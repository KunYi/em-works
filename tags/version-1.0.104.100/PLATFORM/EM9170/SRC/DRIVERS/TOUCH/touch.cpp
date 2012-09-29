//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
// File: touch.cpp
//
//  Implementation of the Touchscreen Driver
//------------------------------------------------------------------------------

//--------------------------------------------------------------
// Include Files
//--------------------------------------------------------------
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#include <tchddsi.h>
#include "csp.h"
#include "adc_ioctl.h"
#include "touch.h"

#ifndef _PREFAST_
#pragma warning(disable: 4068) // Disable pragma warnings
#endif

//--------------------------------------------------------------
// Defines
//--------------------------------------------------------------
// Sample rate
#define TOUCHPANEL_SELECT_SAMPLE_RATE_LOW   0
#define TOUCHPANEL_SELECT_SAMPLE_RATE_HIGH  1

// (4-wire touch-screen measurement)
#define TSC_4WIRE_PRECHARGE         0x0000158C
#define TSC_4WIRE_TOUCH_DETECT      0x0000578E
#define TSC_4WIRE_X_MEASUMENT       0x00001C90
#define TSC_4WIRE_Y_MEASUMENT       0x00004604
#define NB_SAMPLES_PER_ACQUISITION  (2+2*MEASUREMENT_REP)

#define MAX_DISTANCE                15
#define MEASUREMENT_REP             7

//The first samples of the four x samples and four y samples should be discarded because their value it too dependent of the pressure applied
#define SKIP_FIRST_SAMPLE_OF_THE_FOUR   

#ifdef SKIP_FIRST_SAMPLE_OF_THE_FOUR
#define NB_REQUIRED_SAMPLES         (12) 
#define MANIMUM_NB_VALID_SAMPLES    (6) //number of samples within range to declare the position valid
#define FIRST_VALID_SAMPLE_INDEX    (1)
#else
#define NB_REQUIRED_SAMPLES         (14) 
#define MANIMUM_NB_VALID_SAMPLES    (6) //number of samples within range to declare the position valid
#define FIRST_VALID_SAMPLE_INDEX    (0)
#endif
//--------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------
// MDD requiered
DWORD gIntrTouch        = (DWORD) SYSINTR_UNDEFINED;
DWORD gIntrTouchChanged = (DWORD) SYSINTR_NOP;

// This allows you to force new gIntrTouch interrupt after this time.
// Note that the extern "C"
// is required so that the variable name doesn't get decorated, and
// since we have an initializer the 'extern' is actually ignored and
// space is allocated.
extern "C" DWORD gdwTouchIstTimeout;

// The MDD requires a minimum of MIN_CAL_COUNT consecutive samples before
// it will return a calibration coordinate to GWE.  This value is defined
// in the PDD so that each OEM can control the behaviour of the touch
// panel and still use the Microsoft supplied MDD.  
extern "C" const int MIN_CAL_COUNT = 6;

static T_IOCTL_CFG_ITEM_PARAM g_Itemcfg[]=
{
    {TCC0,TSC_4WIRE_PRECHARGE | (0x1<<20) /*Ignore the sample*/},
    {TCC1,TSC_4WIRE_TOUCH_DETECT},
    {TCC2,TSC_4WIRE_X_MEASUMENT | ((MEASUREMENT_REP-1)<<16) /*make 4 X measurement*/},
    {TCC3,TSC_4WIRE_Y_MEASUMENT | ((MEASUREMENT_REP-1)<<16) /*make 4  Y measurement*/},
    {TICR,TSC_4WIRE_TOUCH_DETECT}
};

static T_IOCTL_CFG_QUEUE_PARAM g_TouchScreenQueuecfg =
{
    TOUCH_QUEUE,// Configure the queue dedicated to the touchscreen
    0x00103210, // item 0 = 4-wire prechage
    // item 1 = 4-wire detect
    // item 2 = 4-wire x
    // item 3 = 4-wire y
    // item 4 = 4-wire precharge
    // item 5 = 4-wire detect
    0,
    CSP_BITFVAL(ADC_TCQCR_QSM           ,ADC_TCQCR_QSM_PEN) | // start conversion on PEN down
    CSP_BITFVAL(ADC_TCQCR_FQS           ,0) |
    CSP_BITFVAL(ADC_TCQCR_RPT           ,0) |
    CSP_BITFVAL(ADC_TCQCR_LAST_ITEM_ID  ,5) |
    CSP_BITFVAL(ADC_TCQCR_FIFOWATERMARK ,9) |
    CSP_BITFVAL(ADC_TCQCR_REPEATWAIT    ,0) |
    CSP_BITFVAL(ADC_TCQCR_QRST          ,0) |
    CSP_BITFVAL(ADC_TCQCR_FRST          ,0) |
    CSP_BITFVAL(ADC_TCQCR_PDMSK         ,ADC_TCQCR_PDMASK_UNMASK) | // unmask PEN down
    CSP_BITFVAL(ADC_TCQCR_PDCFG         ,ADC_TCQCR_PDCFG_EDGE),      // PEN down interrupt is edge sensitive
    ~(CSP_BITFMASK(ADC_TCQMR_PD_IRQ))   // Unmask Pen Down Interrupt
};

static DWORD gdwCurrentSamplePeriod = TOUCHPANEL_SAMPLE_RATE_LOW;
static HANDLE g_hADC;

//-----------------------------------------------------------------------------
// Function: BOOL TouchDriverCalibrationPointGet(TPDC_CALIBRATION_POINT *pTCP)
//
// This function calculate the position of each calibration points
//
// Parameters:
//    pTCP : pointer to a TPDC_CALIBRATION_POINT structure
//
// Returns:
//          TRUE when all is good
//          FALSE when all is bad
//
//
//-----------------------------------------------------------------------------
BOOL TouchDriverCalibrationPointGet(TPDC_CALIBRATION_POINT *pTCP)
{
    INT32   cDisplayWidth = pTCP->cDisplayWidth;
    INT32   cDisplayHeight = pTCP->cDisplayHeight;

    int CalibrationRadiusX = cDisplayWidth / 10;
    int CalibrationRadiusY = cDisplayHeight / 10;

    switch (pTCP->PointNumber) {
        case    0:  // Middle
            pTCP->CalibrationX = cDisplayWidth / 2;
            pTCP->CalibrationY = cDisplayHeight / 2;
            break;

        case    1:  // Upper Left
            pTCP->CalibrationX = (CalibrationRadiusX * 2);
            pTCP->CalibrationY = (CalibrationRadiusY * 2);
            break;

        case    2:  // Lower Left
            pTCP->CalibrationX = CalibrationRadiusX * 2;
            pTCP->CalibrationY = cDisplayHeight - (CalibrationRadiusY * 2);
            break;

        case    3:  // Lower Right
            pTCP->CalibrationX = cDisplayWidth - (CalibrationRadiusX * 2);
            pTCP->CalibrationY = cDisplayHeight - (CalibrationRadiusY * 2);
            break;

        case    4:  // Upper Right
            pTCP->CalibrationX = cDisplayWidth - (CalibrationRadiusX * 2);
            pTCP->CalibrationY = (CalibrationRadiusY * 2);
            break;

        default:
            pTCP->CalibrationX = cDisplayWidth / 2;
            pTCP->CalibrationY = cDisplayHeight / 2;
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }
    return TRUE;
}


//--------------------------------------------------------------
// PDD Entry points
//--------------------------------------------------------------

//-----------------------------------------------------------------------------
// Function: LONG DdsiTouchPanelAttach(void)
//
// This function executes when the MDD's DLL entry point 
//             gets a DLL_PROCESS_ATTACH message.
//
//
// Returns:
//          0 : success
//          -1 : failure
//-----------------------------------------------------------------------------
LONG DdsiTouchPanelAttach(void)
{
    BOOL bRet = TRUE;
    DWORD dwLogIntr = IRQ_TCHSC;
    
    //DEBUGMSG( ZONE_INIT, (TEXT("+Touch screen Init\r\n")));
    RETAILMSG(1, (TEXT("+Touch screen Init\r\n")));
        
    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &dwLogIntr, sizeof(DWORD), &gIntrTouch, sizeof(DWORD), NULL))
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("ERROR: Failed to request the gIntrTouch sysintr\r\n")));
        gIntrTouch = (DWORD) SYSINTR_UNDEFINED;
        bRet = FALSE;
    }       
    else
    {
        g_hADC = CreateFile(TEXT("ADC1:"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL); 
        if (g_hADC == INVALID_HANDLE_VALUE)
        {
			RETAILMSG(1, (TEXT("DdsiTouchPanelAttach::open ADC1: failed\r\n")));
            bRet = FALSE;
        }
        else
        {
            int i;
            T_IOCTL_WAKEUP_SOURCE_PARAM WakeUpCfg;
            // Configure the items required by the Touchscreen

            for (i=0; bRet && (i<sizeof(g_Itemcfg)/sizeof(g_Itemcfg[0]));i++)
            {
                bRet= bRet && DeviceIoControl(g_hADC,IOCTL_CFG_ITEM,
                    &g_Itemcfg[i],sizeof(g_Itemcfg[i]),
                    NULL,0,
                    NULL,NULL);     
            }

            bRet= bRet && DeviceIoControl(g_hADC,IOCTL_CFG_QUEUE,
                &g_TouchScreenQueuecfg,sizeof(g_TouchScreenQueuecfg),
                NULL,0,
                NULL,NULL);     

            InterruptDone(gIntrTouch);

#ifdef TOUCHSCREEN_IS_A_WAKEUP_SOURCE
            WakeUpCfg.fEnableWakeUp = TRUE;
            KernelIoControl(IOCTL_HAL_ENABLE_WAKE,&gIntrTouch,sizeof(gIntrTouch),NULL,0,NULL);
#else
            WakeUpCfg.fEnableWakeUp = FALSE;
#endif
            DeviceIoControl(g_hADC,IOCTL_WAKEUP_SOURCE,
                &WakeUpCfg,sizeof(WakeUpCfg),
                NULL,0,
                NULL,NULL);


        }
    }
    if (bRet == FALSE)
    {
        DdsiTouchPanelDetach();
        return -1;
    }
    return 0;
}


//-----------------------------------------------------------------------------
// Function: LONG DdsiTouchPanelDetach(void)
//
// This function executes when the MDD's DLL entry point 
//             gets a DLL_PROCESS_DETTACH message.
//
//
// Returns:
//          0 : success
//          -1 : failure
//-----------------------------------------------------------------------------
LONG DdsiTouchPanelDetach(void)
{
     if (g_hADC != INVALID_HANDLE_VALUE)
    {
        CloseHandle(g_hADC);
        g_hADC = INVALID_HANDLE_VALUE;
    }
        
    // Release all Sysintr
    if (gIntrTouch != SYSINTR_UNDEFINED)
    {
#ifdef TOUCHSCREEN_IS_A_WAKEUP_SOURCE
            KernelIoControl(IOCTL_HAL_DISABLE_WAKE,&gIntrTouch,sizeof(gIntrTouch),NULL,0,NULL);
#endif
        if (!KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &gIntrTouch, sizeof(DWORD), NULL, 0, NULL))
        {
            ERRORMSG(1, (TEXT("ERROR: Failed to release the gIntrTouch sysintr\r\n")));
        }   
        gIntrTouch = (DWORD) SYSINTR_UNDEFINED;
    }
    
    return 0;
}

//-----------------------------------------------------------------------------
// Function: DdsiTouchPanelDisable
//
// This function disables the touch screen device.
//
// This function disables the touch screen device. Disabling a touch screen 
// prevents generation of any subsequent touch samples, and removes power from 
// the screen's hardware. The exact steps necessary depend on the specific 
// characteristics of the touch screen hardware.
//-----------------------------------------------------------------------------
VOID DdsiTouchPanelDisable(void)
{
    InterruptMask(gIntrTouch,TRUE);    
}

//-----------------------------------------------------------------------------
// Function: DdsiTouchPanelEnable
//
// This function applies power to the touch screen device.
//
//
//
// Returns:
//          TRUE when all is good
//          FALSE when all is bad
//
// This function applies power to the touch screen device and initializes it 
// for operation.
//-----------------------------------------------------------------------------
BOOL DdsiTouchPanelEnable(void)
{
    InterruptMask(gIntrTouch,FALSE);
    return TRUE;
}


//-----------------------------------------------------------------------------
// Function: DdsiTouchPanelGetDeviceCaps
//
// This function applies power to the touch screen device.
//
// Parameters:
//    iIndex : Capability to query.
//    lpOutput : Pointer to one or more memory locations to 
//                        place the queried information
// Returns:
//      TRUE when all is good
//      FALSE when something is wrong
//
//-----------------------------------------------------------------------------
BOOL DdsiTouchPanelGetDeviceCaps(INT iIndex, LPVOID lpOutput)
{
    BOOL bRet = FALSE;
    
    if (lpOutput == NULL) {
        ERRORMSG(1,(TEXT("TouchPanelGetDeviceCaps: invalid parameter.\r\n")));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    
    switch  (iIndex) {
        case TPDC_SAMPLE_RATE_ID:
            {
            TPDC_SAMPLE_RATE    *pTSR = (TPDC_SAMPLE_RATE*)lpOutput;
            pTSR->SamplesPerSecondLow = TOUCHPANEL_SAMPLE_RATE_LOW;
            pTSR->SamplesPerSecondHigh = TOUCHPANEL_SAMPLE_RATE_HIGH;
            pTSR->CurrentSampleRateSetting = (gdwCurrentSamplePeriod == TOUCHPANEL_SAMPLE_RATE_LOW)?TOUCHPANEL_SELECT_SAMPLE_RATE_LOW:TOUCHPANEL_SELECT_SAMPLE_RATE_HIGH;
            bRet = TRUE;
            }
            break;
            
        case TPDC_CALIBRATION_POINT_COUNT_ID:
            {
            TPDC_CALIBRATION_POINT_COUNT *pTCPC = (TPDC_CALIBRATION_POINT_COUNT*)lpOutput;
            pTCPC->flags = 0;
            pTCPC->cCalibrationPoints = 5;
            bRet = TRUE;
            }
            break;
            
        case TPDC_CALIBRATION_POINT_ID:
            bRet = TouchDriverCalibrationPointGet((TPDC_CALIBRATION_POINT*)lpOutput);
            break;
            
        default:
            ERRORMSG(1,(TEXT("TouchPanelGetDeviceCaps: invalid parameter.\r\n")));
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
    }

    return bRet;
}

//-----------------------------------------------------------------------------
// Function: FilterSamples
//
// This function recursively filter a set of data.
//
// Parameters:
//      data : sample set
//      dwNbSamples : number of samples in this set
//      dwNbValidSamples : number of valid samples in this set
//      pMeanPosition [OUT] : mean position (sum of valid samples / divided by number of valid samples). 
//                            Valid only when the function returns TRUE.
//
// Returns:
//      FALSE when the amount of not-yet rejected data is too low (filtering failed)
//      TRUE when the filtering succeds (total samples' deviation is below thresold)
//-----------------------------------------------------------------------------

BOOL FilterSamples(T_POSITION* data,DWORD dwNbSamples,DWORD dwNbValidSamples,T_POSITION* pMeanPosition)
{
    DWORD i;
    int CandidateForDiscard = -1;
    DWORD nbValidSamples = 0;
    int meanx=0,meany=0;
    int dist,maxDist=0;
    
    DEBUGCHK(data && pMeanPosition);

    if (dwNbValidSamples < MANIMUM_NB_VALID_SAMPLES)
    {
        return FALSE;
    }

    // compute mean value
    for (i=0;i<dwNbSamples;i++)
    {
        if (data[i].valid)
        {
            nbValidSamples++;
            meanx += data[i].x;
            meany += data[i].y;
        }
    }    
    meanx = meanx / nbValidSamples;
    meany = meany / nbValidSamples;

    // look for the farthest position
    for (i=0;i<dwNbSamples;i++)
    {
        if (data[i].valid)
        {
            dist = ((meanx - data[i].x) * (meanx - data[i].x)+
                        (meany - data[i].y) * (meany - data[i].y));
            if (dist > maxDist)
            {
                maxDist = dist;
                CandidateForDiscard = i;
            }
        }
    }
    if (maxDist > (MAX_DISTANCE*MAX_DISTANCE))
    {
        data[CandidateForDiscard].valid = FALSE;
        return FilterSamples(data,dwNbSamples,dwNbValidSamples-1,pMeanPosition);

    }
    else
    {
        pMeanPosition->x = meanx;
        pMeanPosition->y = meany;
        pMeanPosition->valid = TRUE;
        return TRUE;
    }
}

//-----------------------------------------------------------------------------
// Function: DdsiTouchPanelGetPoint
//
// This function returns the most recently acquired point.
//
// Parameters:
//    pTipState : Pointer to where to return the tip state information
//    pUnCalX : Pointer to where to return the x-coordinate
//    pUnCalY : Pointer to where to return the y-coordinate
//
//
// This function returns the most recently acquired point and its associated tip state information.
//-----------------------------------------------------------------------------
void DdsiTouchPanelGetPoint(TOUCH_PANEL_SAMPLE_FLAGS *pTipState, INT *pUnCalX, INT *pUnCalY)
{
    static INT iPrevValidSampleX    = 0;
    static INT iPrevValidSampleY    = 0;
    static DWORD dwNbSamples = 0;
    static T_POSITION data[NB_REQUIRED_SAMPLES];
    BOOL bIsPenDown = FALSE;
    BOOL bIsAcquisitionOK = FALSE;
    T_POSITION meanPosition = {0,0,FALSE};
    DWORD dwOutBytes;
    UINT16 buffer[NB_SAMPLES_PER_ACQUISITION];
   
    T_IOCTL_GET_DATA_AND_CLEAR_QUEUE_STATUS_PARAM param;

    param.eQueueID = TOUCH_QUEUE;
    param.dwTimeout = 10; // The bigger is the value, the longer is the penup detection time
    

#pragma prefast(disable: 6001, "buffer is a OUT buffer, it doesn't need to be initialized")
    DeviceIoControl(g_hADC,IOCTL_GET_DATA_AND_CLEAR_QUEUE_STATUS,
                &param,sizeof(param),
                buffer,sizeof(buffer),
                &dwOutBytes,NULL);
#pragma prefast(pop)

    switch (dwOutBytes/sizeof(UINT16))
    {
    case NB_SAMPLES_PER_ACQUISITION:
        bIsAcquisitionOK = TRUE;
        bIsPenDown = TRUE;
        break;
    case 0:
        bIsAcquisitionOK = TRUE;
        bIsPenDown = FALSE;
        break;
    default:     
        bIsAcquisitionOK = FALSE;
        bIsPenDown = FALSE;
        break;      
    }

    if (bIsAcquisitionOK && bIsPenDown)
    {   
        int i;
        for (i=FIRST_VALID_SAMPLE_INDEX;i<MEASUREMENT_REP;i++)
        {
            data[dwNbSamples].x = buffer[1+i] >> 4;
            data[dwNbSamples].y = buffer[1+MEASUREMENT_REP+i] >> 4;
            data[dwNbSamples].valid = TRUE;
            dwNbSamples++;

            if (dwNbSamples == NB_REQUIRED_SAMPLES)
            {
                if (FilterSamples(data,dwNbSamples,dwNbSamples,&meanPosition) == FALSE)
                {
                    
                }
                else
                {
                    
                }
                dwNbSamples = 0;
                break;
            }
        }
    }


    if (meanPosition.valid)
    {
        *pTipState =  TouchSampleValidFlag | TouchSampleDownFlag;
        *pUnCalX = meanPosition.x;
        *pUnCalY = meanPosition.y;

        // Store the sample for futur use
        iPrevValidSampleX = *pUnCalX;
        iPrevValidSampleY = *pUnCalY;
        gdwTouchIstTimeout = gdwCurrentSamplePeriod;
    }
    else if (!bIsPenDown) // Pen Up
    {
        //discard all samples
        dwNbSamples = 0;

        // Set flags
        *pTipState = TouchSampleValidFlag;
        // return the last valid sample
        *pUnCalX = iPrevValidSampleX;
        *pUnCalY = iPrevValidSampleY;

        gdwTouchIstTimeout = INFINITE;
        InterruptDone(gIntrTouch);
    }   
    
    else
    {
        // return the last valid sample
        *pTipState =  TouchSampleIgnore;
        *pUnCalX = iPrevValidSampleX;
        *pUnCalY = iPrevValidSampleY;
        gdwTouchIstTimeout = 0;
    }
}


//-----------------------------------------------------------------------------
// Function: VOID DdsiTouchPanelPowerHandler
//
//
// Parameters:
//    bOff : TRUE indicates that the system is turning off. FALSE indicates that the system is turning on.
//
//
// This function indicates to the driver that the system is entering or leaving the suspend state.
//-----------------------------------------------------------------------------
VOID DdsiTouchPanelPowerHandler(BOOL bOff)
{
    UNREFERENCED_PARAMETER(bOff);
#ifdef TOUCHSCREEN_IS_A_WAKEUP_SOURCE
    if (bOff)
    {
        gdwTouchIstTimeout = gdwCurrentSamplePeriod;
        InterruptDone(gIntrTouch);
    }
#else
    if (bOff)
    {
        InterruptMask(gIntrTouch,TRUE);
        gdwTouchIstTimeout = INFINITE;
    }
    else
    {
        InterruptMask(gIntrTouch,FALSE);
        gdwTouchIstTimeout = INFINITE;
    }
#endif
}


//-----------------------------------------------------------------------------
// Function: DdsiTouchPanelSetMode
//
// This function sets information about the touch screen device.
//
// Parameters:
//    iIndex : Mode to set.
//    lpInput : Pointer to one or more memory locations where the update 
//              information resides. Points to one or more memory locations 
//              to place the queried information.
//
// Returns:
//      TRUE when all is good
//      FALSE when all is bad
//-----------------------------------------------------------------------------
BOOL DdsiTouchPanelSetMode(INT iIndex, LPVOID lpInput)
{
    BOOL bRet = FALSE;
    UNREFERENCED_PARAMETER(lpInput);
    switch (iIndex)
    {
        case TPSM_SAMPLERATE_HIGH_ID:
            gdwCurrentSamplePeriod = TOUCHPANEL_SAMPLE_RATE_HIGH;
            DEBUGMSG(1,(TEXT("DdsiTouchPanelSetMode : Set SampleRate to High.\r\n")));
            bRet = TRUE;
        break;
        
        case TPSM_SAMPLERATE_LOW_ID:
            gdwCurrentSamplePeriod = TOUCHPANEL_SAMPLE_RATE_LOW;
            DEBUGMSG(1,(TEXT("DdsiTouchPanelSetMode : Set SampleRate to Low.\r\n")));
            bRet = TRUE;
        break;
        
        default:
            bRet = FALSE;
            ERRORMSG(1,(TEXT("DdsiTouchPanelSetMode : Unsupported Mode.\r\n")));
        break;
    }
    
    return bRet;    
}
