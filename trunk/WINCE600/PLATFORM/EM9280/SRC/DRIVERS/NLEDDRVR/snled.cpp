//------------------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
//  Use of this source code is subject to the terms of the Microsoft end-user
//  license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
//  If you did not accept the terms of the EULA, you are not authorized to use
//  this source code. For a copy of the EULA, please see the LICENSE.RTF on your
//  install media.
//
//------------------------------------------------------------------------------
//
//  File:  Snled.cpp
//
//  Implementation of PDD layer of Notification LED(s)
//
//------------------------------------------------------------------------------
//
// Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

#include <windows.h>
#include <nled.h>
#include <led_drvr.h>
#include <types.h>
#include <bsp.h>
#include <bsp_cfg.h>
#include <nkintr.h>
#include <Winbase.h>
 
//------------------------------------------------------------------------------
// Defines 
#define NLED_MAX_LED                1    // Total number of notification LEDs supported

//------------------------------------------------------------------------------
// Types
struct NLedPddContext
{
    BOOL         bLEDState;
    BOOL         bLEDBlinkState;
    NLED_SETTINGS_INFO NLedSettingsInfo;
    NLED_SUPPORTS_INFO NLedSupportsInfo;
};

//------------------------------------------------------------------------------
// Local Variables 

static NLedPddContext  g_NLedPddContext[NLED_MAX_LED];
//DWORD g_dwBoardID = BOARDID_EVKBOARD;

//------------------------------------------------------------------------------
// Local Functions
DWORD WINAPI LEDBlinkThread(LPVOID lParam);
BOOL LEDGpioSetIOMux();
BOOL TurnOnLED(UINT32 u32Channel);
BOOL TurnOffLED(UINT32 u32Channel);

//-----------------------------------------------------------------------------
//
// Function: NLedDriverInitialize
//
// The NLED MDD calls this routine to initialize the underlying NLED hardware.
// This routine should return TRUE if successful.  If there's a problem
// it should return FALSE and call SetLastError() to pass back the reason
// for the failure.
//
//  Returns: 
//        This routine returns TRUE if successful, or FALSE if there's a problem
//
//-----------------------------------------------------------------------------

BOOL WINAPI NLedDriverInitialize(VOID)
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("+NLedDriverInitialize: invoked\r\n")));

    // Get board ID, used to setup the RGB mapping
    //if (!KernelIoControl(IOCTL_HAL_QUERY_BOARD_ID, NULL, 0, &g_dwBoardID, sizeof(g_dwBoardID), NULL))
    //{
    //    ERRORMSG(1, (_T("Cannot obtain the board ID!\r\n")));
    //}

    LEDGpioSetIOMux();

    // initialization NLedPddContext struct
    memset(&g_NLedPddContext, 0, NLED_MAX_LED * sizeof(NLedPddContext));

    //Setting NLED support Blink mode
    for(int i = 0;i < NLED_MAX_LED;i++)
    {            
        g_NLedPddContext[i].NLedSupportsInfo.LedNum = 0;
        g_NLedPddContext[i].NLedSupportsInfo.fAdjustOffTime = TRUE;
        g_NLedPddContext[i].NLedSupportsInfo.fAdjustOnTime = TRUE;
        g_NLedPddContext[i].NLedSupportsInfo.fAdjustTotalCycleTime = FALSE;
        g_NLedPddContext[i].NLedSupportsInfo.lCycleAdjust = 1000;
        g_NLedPddContext[i].NLedSupportsInfo.fMetaCycleOff = TRUE;
        g_NLedPddContext[i].NLedSupportsInfo.fMetaCycleOn = TRUE;
    }
        
    for(int i = 0;i < NLED_MAX_LED;i++)    
    {
        TurnOffLED(i);    //Turn off all LED
    }  
    DEBUGMSG(ZONE_FUNCTION, (TEXT("-NLedDriverInitialize\r\n")));
         
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: NLedDriverDeInitialize
//
// The NLED MDD calls this routine to deinitialize the underlying NLED
// hardware as the NLED driver is unloaded.  It should return TRUE if 
// successful.  If there's a problem this routine should return FALSE 
// and call SetLastError() to pass back the reason for the failure.
//
//  Returns: 
//        This routine returns TRUE if successful, or FALSE if there's a problem
//
//-----------------------------------------------------------------------------
BOOL WINAPI NLedDriverDeInitialize(VOID)
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("+NLedDriverDeInitialize: invoked\r\n")));
   
    DEBUGMSG(ZONE_FUNCTION, (TEXT("-NLedDriverDeInitialize\r\n")));
    
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: NLedDriverGetDeviceInfo
//
// This routine retrieves information about the NLED device(s) that
// this driver supports.  The nInfoId parameter indicates what specific
// information is being queried and pOutput is a buffer to be filled in.
// The size of pOutput depends on the type of data being requested.  This
// routine returns TRUE if successful, or FALSE if there's a problem -- in
// which case it also calls SetLastError() to pass back more complete
// error information.  The NLED MDD invokes this routine when an application
// calls NLedGetDeviceInfo().
//
// Parameters:
//      InputParm 
//          [in] INT     nInfoId
//
//      OutputParm 
//          [out] PVOID   pOutput
//
// Returns:  
//      This routine returns TRUE if successful, or FALSE if there's a problem
//
//-----------------------------------------------------------------------------
BOOL WINAPI NLedDriverGetDeviceInfo(INT nInfoId,PVOID pOutput)
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("+NLedDriverGetDeviceInfo\r\n")));
    
    BOOL fOk = TRUE;    
    SETFNAME(_T("NLedDriverGetDeviceInfo"));

    if(nInfoId == NLED_COUNT_INFO_ID)
    {
        struct NLED_COUNT_INFO  *p = (struct NLED_COUNT_INFO*)pOutput;
        PREFAST_SUPPRESS(6320, "Generic exception handler");
        __try 
        {
            p->cLeds = NLED_MAX_LED;
        }
        __except(EXCEPTION_EXECUTE_HANDLER) 
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            fOk = FALSE;
        }
    }            
    else if (nInfoId == NLED_SUPPORTS_INFO_ID)
    {
        struct NLED_SUPPORTS_INFO  *p = (struct NLED_SUPPORTS_INFO*)pOutput;
        PREFAST_SUPPRESS(6320, "Generic exception handler");
        __try 
        {
            if(p->LedNum >= NLED_MAX_LED)            
            {    
                fOk = FALSE;
            }
            else
            {        
                p->fAdjustOffTime           = g_NLedPddContext[p->LedNum].NLedSupportsInfo.fAdjustOffTime;
                p->fAdjustOnTime            = g_NLedPddContext[p->LedNum].NLedSupportsInfo.fAdjustOnTime;
                p->lCycleAdjust             = g_NLedPddContext[p->LedNum].NLedSupportsInfo.lCycleAdjust;
                p->fMetaCycleOff            = g_NLedPddContext[p->LedNum].NLedSupportsInfo.fMetaCycleOff;
                p->fMetaCycleOn             = g_NLedPddContext[p->LedNum].NLedSupportsInfo.fMetaCycleOn;
                p->fAdjustTotalCycleTime    = g_NLedPddContext[p->LedNum].NLedSupportsInfo.fAdjustTotalCycleTime;
            }
        }
        __except(EXCEPTION_EXECUTE_HANDLER) 
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            fOk = FALSE;
        }
    }    
    else if (nInfoId == NLED_SETTINGS_INFO_ID)
    {
        struct NLED_SETTINGS_INFO  *p = (struct NLED_SETTINGS_INFO*)pOutput;
        PREFAST_SUPPRESS(6320, "Generic exception handler");
        __try 
        {
            if(p->LedNum >= NLED_MAX_LED)            
            {
                fOk = FALSE;
            }
            else
            {        
                p->MetaCycleOff            = g_NLedPddContext[p->LedNum].NLedSettingsInfo.MetaCycleOff;
                p->MetaCycleOn             = g_NLedPddContext[p->LedNum].NLedSettingsInfo.MetaCycleOn;
                p->OffOnBlink              = g_NLedPddContext[p->LedNum].NLedSettingsInfo.OffOnBlink;
                p->OffTime                 = g_NLedPddContext[p->LedNum].NLedSettingsInfo.OffTime;
                p->OnTime                  = g_NLedPddContext[p->LedNum].NLedSettingsInfo.OnTime;
                p->TotalCycleTime          = g_NLedPddContext[p->LedNum].NLedSettingsInfo.TotalCycleTime;
            }    
        }
        __except(EXCEPTION_EXECUTE_HANDLER) 
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            fOk = FALSE;
        }
    }    
    else
    {
        fOk = FALSE;
        SetLastError(ERROR_INVALID_PARAMETER);
    }
    DEBUGMSG(ZONE_FUNCTION, (TEXT("%s\r\n"),pszFname));    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("-NLedDriverGetDeviceInfo\r\n")));
        
    return (fOk);
}

//-----------------------------------------------------------------------------
//
// Function: NLedDriverSetDevice
//
// This routine changes the configuration of an LED.  The nInfoId parameter
// indicates what kind of configuration information is being changed.  
// Currently only the NLED_SETTINGS_INFO_ID value is supported.  The pInput
// parameter points to a buffer containing the data to be updated.  The size
// of the buffer depends on the value of nInfoId.  This routine returns TRUE
// if successful or FALSE if there's a problem -- in which case it also calls
// SetLastError().  The NLED MDD invokes this routine when an application 
// calls NLedSetDevice().
//
// Parameters:
//      InputParm 
//          [in] INT         nInfoId
//          [in] PVOID       pInput
//
// Returns:  
//      This routine returns TRUE if successful, or FALSE if there's a problem
//
//-----------------------------------------------------------------------------
BOOL WINAPI NLedDriverSetDevice(INT nInfoId, PVOID pInput)
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("+NLedDriverSetDevice\r\n")));

    //HANDLE hThread = NULL;
    UINT LedIndex;

    struct NLED_SETTINGS_INFO *pNledSettingsInfo = (struct NLED_SETTINGS_INFO*)pInput;
    PREFAST_SUPPRESS(6320, "Generic exception handler");
    __try 
    {
        if((nInfoId != NLED_SETTINGS_INFO_ID) || (pNledSettingsInfo->LedNum >= NLED_MAX_LED))
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("NLedDriverSetDevice: Invalid Parameters\r\n")));    
            return FALSE;
        }
   
        LedIndex = pNledSettingsInfo->LedNum;
        g_NLedPddContext[LedIndex].NLedSettingsInfo.LedNum = pNledSettingsInfo->LedNum;    
        g_NLedPddContext[LedIndex].NLedSettingsInfo.OffOnBlink = pNledSettingsInfo->OffOnBlink;
    
    
        if(pNledSettingsInfo->OffOnBlink == 0)    // OffOnBlink setting - OFF
        {    
            TurnOffLED(LedIndex);  
            g_NLedPddContext[LedIndex].bLEDState = FALSE;
        }    
        else if(pNledSettingsInfo->OffOnBlink == 1)  // OffOnBlink setting - ON
        {          
            TurnOnLED( LedIndex);
            g_NLedPddContext[LedIndex].bLEDState = TRUE;
        }
        else if(pNledSettingsInfo->OffOnBlink == 2) // OffOnBlink setting - BLINK
        {      
            g_NLedPddContext[LedIndex].NLedSettingsInfo = *(pNledSettingsInfo);
    
            // Round up OnTime and OffTime values 1 millisecond
            if(g_NLedPddContext[LedIndex].NLedSettingsInfo.OnTime  < 0 || 
                g_NLedPddContext[LedIndex].NLedSettingsInfo.OffTime < 0)
            {
                return 0;
            }
            if(g_NLedPddContext[LedIndex].NLedSettingsInfo.OnTime < 1000)
            {
                g_NLedPddContext[LedIndex].NLedSettingsInfo.OnTime = 1000;
            }
            if(g_NLedPddContext[LedIndex].NLedSettingsInfo.OffTime < 1000)
            {
                g_NLedPddContext[LedIndex].NLedSettingsInfo.OffTime = 1000;    
            }
            g_NLedPddContext[LedIndex].NLedSettingsInfo.TotalCycleTime = 
                g_NLedPddContext[LedIndex].NLedSettingsInfo.OnTime + g_NLedPddContext[LedIndex].NLedSettingsInfo.OffTime;
    
            if(g_NLedPddContext[LedIndex].NLedSettingsInfo.MetaCycleOn < 0 ||
                g_NLedPddContext[LedIndex].NLedSettingsInfo.MetaCycleOff < 0)
            {
                return FALSE;
            }
            UINT MetaCycleOn = g_NLedPddContext[LedIndex].NLedSettingsInfo.MetaCycleOn;
            UINT MetaCycleOff = g_NLedPddContext[LedIndex].NLedSettingsInfo.MetaCycleOff;
            for(;;)
            {    
                if(MetaCycleOn-- <= 0)   
                {
                    break;
                }
                TurnOnLED(LedIndex);
                g_NLedPddContext[LedIndex].bLEDState = TRUE;
                Sleep(g_NLedPddContext[LedIndex].NLedSettingsInfo.OnTime/1000);
    
                if(MetaCycleOff-- <= 0)
                {
                    break;    
                }
                TurnOffLED(LedIndex);
                g_NLedPddContext[LedIndex].bLEDState = FALSE;
                Sleep(g_NLedPddContext[LedIndex].NLedSettingsInfo.OffTime/1000);       
            }
    
        }       
        else    
        {       
            DEBUGMSG(ZONE_ERROR, (TEXT("NLedDriverSetDevice: Invalid OffOnBlink\r\n")));
            return FALSE;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) 
    {
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
    }        
    DEBUGMSG(ZONE_FUNCTION, (TEXT("-NLedDriverSetDevice\r\n")));

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: NLedDriverPowerDown
//
// This routine is invoked by the driver MDD when the system suspends or
// resumes.  The power_down flag indicates whether the system is powering 
// up or powering down.
//
// Parameters:
//      InputParm 
//          [in] BOOL power_down
//
// Returns:  
//          None
//
//-----------------------------------------------------------------------------
VOID WINAPI NLedDriverPowerDown(BOOL power_down)
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("+NLedDriverPowerDown\r\n")));

    for(int i = 0;i < NLED_MAX_LED;i++)  
    {
        if(power_down)  
        {
            if(g_NLedPddContext[i].bLEDState == TRUE) 
            {
                TurnOffLED(i);  //Trun Off LED when LED is on
            }
        }
        else        
        {
            if(g_NLedPddContext[i].bLEDState == TRUE) 
            {
                TurnOnLED(i);  //restore LED to on
            }
        }
    }

    DEBUGMSG(ZONE_FUNCTION, (TEXT("-NLedDriverPowerDown\r\n")));
        
    return;
}

//-----------------------------------------------------------------------------
//
// Function: LEDGpioSetIOMux
//
// This routine is to set the IOMUX for LED.
//
// Parameters:
//     
//          
//
// Returns:  
//          TRUE
//
//-----------------------------------------------------------------------------
BOOL   LEDGpioSetIOMux()
{
    //if (g_dwBoardID == BOARDID_DEVBOARD)
    //{
    DDKIomuxSetPinMux(DDK_IOMUX_PWM0_0,DDK_IOMUX_MODE_GPIO);
    DDKGpioEnableDataPin(DDK_IOMUX_PWM0_0,1);
    DDKGpioWriteDataPin(DDK_IOMUX_PWM0_0, 1);
    //}
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: TurnOnLED
//
// This routine is to turn on LED.
//
// Parameters:
//     
//          
//
// Returns:  
//          TRUE
//
//-----------------------------------------------------------------------------
BOOL TurnOnLED(UINT32 u32Channel)
{
    UNREFERENCED_PARAMETER(u32Channel);
    //if (g_dwBoardID == BOARDID_DEVBOARD)
    //{
    DDKGpioWriteDataPin(DDK_IOMUX_PWM0_0, 1);
    //}
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: TurnOffLED
//
// This routine is to turn off LED.
//
// Parameters:
//     
//          
//
// Returns:  
//          TRUE
//
//-----------------------------------------------------------------------------
BOOL TurnOffLED(UINT32 u32Channel)
{
    UNREFERENCED_PARAMETER(u32Channel);
    //if (g_dwBoardID == BOARDID_DEVBOARD)
    //{
    DDKGpioWriteDataPin(DDK_IOMUX_PWM0_0, 0);
    //}
    return TRUE;
}
