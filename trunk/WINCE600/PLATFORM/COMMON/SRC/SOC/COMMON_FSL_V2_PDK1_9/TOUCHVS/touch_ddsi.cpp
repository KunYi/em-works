//------------------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
//  Use of this source code is subject to the terms of the Microsoft end-user
//  license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
//  If you did not accept the terms of the EULA, you are not authorized to use
//  this source code. For a copy of the EULA, please see the LICENSE.RTF on
//  your install media.
//
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  touchpdd.c
//
//  Provides the PDD code for the DDSI touch driver routines.
//
//-----------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <nkintr.h>
#include <tchddsi.h>
#pragma warning(pop)

#include "common_macros.h"
#include "touch_ddsi.h"
#include "touch_hw.h"
#include "hw_lradc.h"

//-----------------------------------------------------------------------------
// External Functions

//-----------------------------------------------------------------------------
// External Variables
extern "C" const int MIN_CAL_COUNT = 20;
DWORD BSPTouchGetDetIRQ();
DWORD BSPTouchGetChangedIRQ();
#define ABS(x)  ((x) >= 0 ? (x) : (-(x)))

BOOL IsPowerOff=FALSE;

#define TSP_SAMPLE_RATE_LOW             100
#define TSP_SAMPLE_RATE_HIGH            200

static INT TSP_CurRate = TSP_SAMPLE_RATE_HIGH;

//-----------------------------------------------------------------------------
// Defines

//-----------------------------------------------------------------------------
// Types

//-----------------------------------------------------------------------------
// Global Variables
INT gCurSampleRateSetting = 1;         // High sample rate setting.

DWORD gtouchint;
DWORD gtouchCangedIrq;

DWORD gIntrTouch;
DWORD gIntrTouchChanged;


//-----------------------------------------------------------------------------
// Local Variables
HANDLE g_hLRADCTch = NULL;  // handle for LRADC driver
static _EN_TP_INTERRUPT g_ExpectedInterrupt =_TP_INTERRUPT_PEN_DOWN_;

//-----------------------------------------------------------------------------
// Local Functions
static BOOL TouchDriverCalibrationPointGet(TPDC_CALIBRATION_POINT *pTCP);


//-----------------------------------------------------------------------------
//
// Function: DdsiTouchPanelAttach
//
// This function executes when the MDD's DLL entry point gets a
// DLL_PROCESS_ATTACH message.
//
// Parameters:
//      None.
//
// Returns:
//      Returns zero (0).
//
//-----------------------------------------------------------------------------
extern "C" LONG DdsiTouchPanelAttach()
{
   

    gtouchint=BSPTouchGetDetIRQ();
    gtouchCangedIrq=BSPTouchGetChangedIRQ();
   

    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &gtouchint, sizeof(DWORD), &gIntrTouch, sizeof(DWORD), NULL))
    {
        ERRORMSG(TRUE, (TEXT("DdsiTouchPanelAttach(): IOCTL_HAL_REQUEST_SYSINTR failed.\r\n")));
        return -1;
    }

    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &gtouchCangedIrq, sizeof(DWORD), &gIntrTouchChanged, sizeof(DWORD), NULL))
    {
        ERRORMSG(TRUE, (TEXT("DdsiTouchPanelAttach(): IOCTL_HAL_REQUEST_SYSINTR failed.\r\n")));
        return -1;
    }

/*
    DEBUGMSG(ZONE_FUNCTION, (_T("%s: About to enable the wake source %d %d\r\n"),
                    __WFUNCTION__, gIntrTouch, gtouchint));
    // Ask the OAL to enable our interrupt to wake the system from suspend.
    KernelIoControl(IOCTL_HAL_ENABLE_WAKE, &gIntrTouch,
                    sizeof(gIntrTouch), NULL, 0, NULL);

    DEBUGMSG(ZONE_FUNCTION, (_T("%s: Done enabling the wake source\r\n"),
                     __WFUNCTION__));
*/
    return 0;
}

//-----------------------------------------------------------------------------
//
// Function: DdsiTouchPanelDetach
//
// This function executes when the MDD's DLL entry point gets a
// DLL_PROCESS_DETACH message.
//
// Parameters:
//      None.
//
// Returns:
//      Returns zero (0).
//
//-----------------------------------------------------------------------------
extern "C" LONG DdsiTouchPanelDetach()
{
    DEBUGMSG(ZONE_FUNCTION, (_T("DdsiTouchPanelDetach+\r\n")));

    KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &gIntrTouch,
                    sizeof(gIntrTouch), NULL, 0, NULL);

    KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &gIntrTouchChanged,
                    sizeof(gIntrTouchChanged), NULL, 0, NULL);
    
    DEBUGMSG(ZONE_FUNCTION, (_T("DdsiTouchPanelDetach-\r\n")));

    return 0;
}

//-----------------------------------------------------------------------------
//
// Function: DdsiTouchPanelEnable
//
// This function applies power to the touch screen device and initializes
// it for operation.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//-----------------------------------------------------------------------------
extern "C" BOOL DdsiTouchPanelEnable(void)
{
    TCHAR lpDevName[] = TEXT("LDC1:");

    DEBUGMSG(ZONE_FUNCTION, (TEXT("+DdsiTouchPanelEnable()\r\n")));

    //Opening a handle to the LRADC driver
    g_hLRADCTch = LRADCOpenHandle((LPCWSTR)lpDevName);
    if(g_hLRADCTch == INVALID_HANDLE_VALUE)
    {
        ERRORMSG(TRUE, (TEXT("DdsiTouchPanelEnable(): g_hLRADCTch INVALID_HANDLE_VALUE\r\n")));
        return FALSE;
    }

    // Initializing the hardware LRADC
    TouchConfig();

    g_ExpectedInterrupt = _TP_INTERRUPT_PEN_DOWN_;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("-DdsiTouchPanelEnable()\r\n")));

    return(TRUE);
}

//-----------------------------------------------------------------------------
//
// Function: DdsiTouchPanelDisable
//
// This function disables the touch screen device. Disabling a touch
// screen prevents generation of any subsequent touch samples, and
// removes power from the screen's hardware. The exact steps necessary
// depend on the specific characteristics of the touch screen hardware.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
extern "C" void DdsiTouchPanelDisable(void)
{
    DEBUGMSG(ZONE_FUNCTION, (_T("DdsiTouchPanelDisable+\r\n")));

    DEBUGMSG(ZONE_FUNCTION, (_T("DdsiTouchPanelDisable-\r\n")));
    
    return;
}

//-----------------------------------------------------------------------------
//
// Function: DdsiTouchPanelPowerHandler
//
// This function indicates to the driver that the system is entering
// or leaving the suspend state.
//
// Parameters:
//      bOff
//          [in] TRUE indicates that the system is turning off. FALSE
//          indicates that the system is turning on.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
extern "C" void DdsiTouchPanelPowerHandler(BOOL boff)
{

    UNREFERENCED_PARAMETER(boff);
    DEBUGMSG(ZONE_FUNCTION, (_T("DdsiTouchPanelPowerHandler+:%d\r\n"),boff));

    IsPowerOff=boff;

    DEBUGMSG(ZONE_FUNCTION, (_T("DdsiTouchPanelPowerHandler-\r\n")));

    return;
}

//-----------------------------------------------------------------------------
//
// Function: DdsiTouchPanelGetPoint
//
// This function returns the most recently acquired point and its
// associated tip state information.
//
// Parameters:
//      pTipState
//          [out] Pointer to where to return the tip state information in
//          TOUCH_PANEL_SAMPLE_FLAGS.
//
//      pUnCalX
//          [out] Pointer to where to return the x-coordinate.
//
//      pUnCalY
//          [out] Pointer to where to return the y-coordinate.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
extern "C" void DdsiTouchPanelGetPoint(TOUCH_PANEL_SAMPLE_FLAGS *pTipStateFlags, INT *pUncalX, INT *pUncalY)
{
    _EN_TP_INTERRUPT Expect;

    DEBUGMSG(ZONE_FUNCTION, (_T("DdsiTouchPanelGetPoint+\r\n")));

    if(!pTipStateFlags || !pUncalX || !pUncalY)
    {
        ERRORMSG(TRUE, (TEXT("DdsiTouchPanelGetPoint(): Invalid parameter(s)\r\n")));
        return;
    }

    *pTipStateFlags = TouchSampleValidFlag;

    //Clears Interrupts and captures the samples
    TouchInterruptClear();
    Expect = TouchInterruptCapture(g_ExpectedInterrupt);

    if((Expect == _TP_INTERRUPT_PEN_UP_OR_TIMER_SAMPLE_X_) ||
       (Expect == _TP_INTERRUPT_PEN_UP_OR_TIMER_SAMPLE_Y_))
    {
        *pTipStateFlags = TouchGetSample(pUncalX, pUncalY);
        g_ExpectedInterrupt=Expect;
    }

    if(*pTipStateFlags == TouchSampleValidFlag) 
    {
        // Enable Touch detect
        LRADCEnableTouchDetect(g_hLRADCTch,TRUE);

        // Enable Touch detect irq
        LRADCEnableTouchDetectInterrupt(g_hLRADCTch,TRUE);

        // reEnable interrupt
        InterruptDone(gIntrTouch);

        g_ExpectedInterrupt = _TP_INTERRUPT_PEN_DOWN_;
    }

    DEBUGMSG(ZONE_FUNCTION, (_T("DdsiTouchPanelGetPoint-\r\n")));
}



//-----------------------------------------------------------------------------
//
// Function: DdsiTouchPanelGetDeviceCaps
//
// This function queries capabilities of the touch screen device.
//
// Parameters:
//      iIndex
//          [in] Capability to query.  The following capabilities can
//          be specifed:
//
//              TPDC_SAMPLE_RATE_ID - Returns the sample rate.
//
//              TPDC_CALIBRATION_POINT_ID - Returns the x and y
//              coordinates of the required calibration point. The index
//              of the calibration point is set in the PointNumber member
//              of lpOutput.
//
//              TPDC_CALIBRATION_POINT_COUNT_ID Returns the number of
//              calibration points used to calibrate the touch screen.
//
//      lpOutput
//          [out] Pointer to one or more memory locations to place the
//          queried information. The format of the memory referenced
//          depends on the setting of iIndex.
//
//  Returns:
//      TRUE if successful, FALSE otherwise.
//
//-----------------------------------------------------------------------------

BOOL DdsiTouchPanelGetDeviceCaps(INT iIndex, LPVOID lpOutput)
{
    struct TPDC_SAMPLE_RATE *ptrSR;
    struct TPDC_CALIBRATION_POINT *ptrCP;
    struct TPDC_CALIBRATION_POINT_COUNT *ptrCPC;

    DEBUGMSG(ZONE_FUNCTION, (_T("DdsiTouchPanelGetDeviceCaps+\r\n")));

    // Check for invalid parameter
    if (lpOutput == NULL)
        return FALSE;

    switch (iIndex)
    {
    // Get the sample rate
    case TPDC_SAMPLE_RATE_ID:
        ptrSR = (struct TPDC_SAMPLE_RATE*)lpOutput;
        ptrSR->SamplesPerSecondLow = TSP_SAMPLE_RATE_LOW;
        ptrSR->SamplesPerSecondHigh = TSP_SAMPLE_RATE_HIGH;
        ptrSR->CurrentSampleRateSetting = gCurSampleRateSetting;
        break;

    // Get the x and y coordinates of the required calibration point.
    case TPDC_CALIBRATION_POINT_ID:
        ptrCP = (struct TPDC_CALIBRATION_POINT*)lpOutput;
        return (TouchDriverCalibrationPointGet(ptrCP));

    // Get the number of calibration points used to calibrate the touch screen.
    case TPDC_CALIBRATION_POINT_COUNT_ID:
        ptrCPC = (struct TPDC_CALIBRATION_POINT_COUNT*)lpOutput;
        ptrCPC->flags = 0;
        ptrCPC->cCalibrationPoints = 5;
        break;

    default:
        return FALSE;
    }

    DEBUGMSG(ZONE_FUNCTION, (_T("DdsiTouchPanelGetDeviceCaps-\r\n")));
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: DdsiTouchPanelSetMode
//
// This function sets information about the touch screen device.
//
// Parameters:
//      iIndex
//          [in] Mode to set. The following modes can set:
//
//              TPSM_SAMPLERATE_HIGH_ID Sets the sample rate to the high rate.
//
//              TPSM_SAMPLERATE_LOW_ID Sets the sample rate to the low rate.
//
//      lpInput
//          [out] Pointer to one or more memory locations where the update
//          information resides. Points to one or more memory locations to
//          place the queried information.
//
// Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//-----------------------------------------------------------------------------

BOOL DdsiTouchPanelSetMode(INT iIndex, LPVOID lpInput)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(lpInput);

    DEBUGMSG(ZONE_FUNCTION, (_T("DdsiTouchPanelSetMode+\r\n")));

    if (iIndex == TPSM_SAMPLERATE_LOW_ID)
    {
        TSP_CurRate = TSP_SAMPLE_RATE_LOW;
        DEBUGMSG(ZONE_FUNCTION, (TEXT("TouchPanelGetDeviceCaps::DdsiTouchPanelSetMode TSP_CurRate:%d\r\n"),TSP_CurRate));

        gCurSampleRateSetting = 0;
    }
    else if (iIndex == TPSM_SAMPLERATE_HIGH_ID)
    {
        TSP_CurRate = TSP_SAMPLE_RATE_HIGH;

        gCurSampleRateSetting = 1;
    }
    else
        return FALSE;

    DEBUGMSG(ZONE_FUNCTION, (_T("DdsiTouchPanelSetMode-\r\n")));

    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: TouchDriverCalibrationPointGet
//
// This function is used to get a single calibration point, in screen
// coordinates, when the input system is calibration the touch driver.
// The input system will then draw a target on the screen for the user
// to press on.
//
// Parameters:
//      lpOutput
//          [in/out] Pointer to return the calibration point.
//
// Returns:
//      TRUE indicates success, FALSE if the requested point number is
//      not available.
//
//-----------------------------------------------------------------------------
static BOOL TouchDriverCalibrationPointGet(TPDC_CALIBRATION_POINT *pTCP)
{
    INT32 cDisplayWidth  = pTCP->cDisplayWidth;
    INT32 cDisplayHeight = pTCP->cDisplayHeight;

    int CalibrationRadiusX = cDisplayWidth  / 10;
    int CalibrationRadiusY = cDisplayHeight / 10;

    switch (pTCP->PointNumber)
    {
    case    0:
        pTCP->CalibrationX = cDisplayWidth  / 2;
        pTCP->CalibrationY = cDisplayHeight / 2;
        break;

    case    1:
        pTCP->CalibrationX = CalibrationRadiusX * 2;
        pTCP->CalibrationY = CalibrationRadiusY * 2;
        break;

    case    2:
        pTCP->CalibrationX = CalibrationRadiusX * 2;
        pTCP->CalibrationY = cDisplayHeight - CalibrationRadiusY * 2;
        break;

    case    3:
        pTCP->CalibrationX = cDisplayWidth  - CalibrationRadiusX * 2;
        pTCP->CalibrationY = cDisplayHeight - CalibrationRadiusY * 2;
        break;

    case    4:
        pTCP->CalibrationX = cDisplayWidth - CalibrationRadiusX * 2;
        pTCP->CalibrationY = CalibrationRadiusY * 2;
        break;

    default:
        pTCP->CalibrationX = cDisplayWidth  / 2;
        pTCP->CalibrationY = cDisplayHeight / 2;

        SetLastError(ERROR_INVALID_PARAMETER);
        return (FALSE);
    }

    RETAILMSG(0, (TEXT("::: TSP_CalibrationPointGet()\r\n")));
    RETAILMSG(0, (TEXT("cDisplayWidth        : %4X\r\n"), cDisplayWidth     ));
    RETAILMSG(0, (TEXT("cDisplayHeight       : %4X\r\n"), cDisplayHeight    ));
    RETAILMSG(0, (TEXT("CalibrationRadiusX   : %4d\r\n"), CalibrationRadiusX));
    RETAILMSG(0, (TEXT("CalibrationRadiusY   : %4d\r\n"), CalibrationRadiusY));
    RETAILMSG(0, (TEXT("pTCP -> PointNumber  : %4d\r\n"), pTCP->PointNumber));
    RETAILMSG(0, (TEXT("pTCP -> CalibrationX : %4d\r\n"), pTCP->CalibrationX));
    RETAILMSG(0, (TEXT("pTCP -> CalibrationY : %4d\r\n"), pTCP->CalibrationY));

    return (TRUE);
}
