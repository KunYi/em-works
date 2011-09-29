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
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
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

#include "touchpdd.h"


//-----------------------------------------------------------------------------
// External Functions
extern BOOL BSPTouchInit(DWORD dwIntervalMsec);
extern void BSPTouchDeinit(void);
extern TOUCH_PANEL_SAMPLE_FLAGS BSPTouchGetSample(INT *x, INT *y);
extern void BSPTouchSetSampleRate(DWORD dwIntervalMsec);
extern void BSPTouchPowerHandler(BOOL boff);
extern void BSPTouchTimerEnable(void);
extern void BSPTouchInterruptDone(void);
extern void BSPCheckIRQ(void);
extern void BSPTouchADCDelay(void);


//-----------------------------------------------------------------------------
// External Variables
extern HANDLE hTouchPanelEvent;


//-----------------------------------------------------------------------------
// Defines


//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables
DWORD gIntrTouch        = (DWORD) SYSINTR_NOP;
DWORD gIntrTouchChanged = (DWORD) SYSINTR_NOP; // Not used here.

INT gCurSampleRateSetting = 1;         // High sample rate setting.


//-----------------------------------------------------------------------------
// Local Variables
static DWORD gCurSampleRate    = TOUCH_SAMPLE_RATE_HIGH;
static DWORD gTouchPowerStatus = TouchPowerOn;


//-----------------------------------------------------------------------------
// Local Functions
static BOOL TouchDriverCalibrationPointGet(struct TPDC_CALIBRATION_POINT *lpOutput);


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
LONG DdsiTouchPanelAttach(void)
{
    DEBUGMSG(ZONE_FUNCTION, (_T("DdsiTouchPanelAttach+\r\n")));
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
LONG DdsiTouchPanelDetach(void)
{
    DEBUGMSG(ZONE_FUNCTION, (_T("DdsiTouchPanelDetach+\r\n")));
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
BOOL DdsiTouchPanelEnable(void)
{
    BOOL rc = TRUE;

    DEBUGMSG(ZONE_FUNCTION, (_T("DdsiTouchPanelEnable+\r\n")));

    // if we are already powered on, just return
    if (gTouchPowerStatus == TouchPowerOn)
    {
        goto cleanUp;
    }

    // Initialize BSP-specific settings
    if (!BSPTouchInit(1000/gCurSampleRate))
    {
        DEBUGMSG(ZONE_ERROR,
            (_T("DdsiTouchPanelEnable:  BSPTouchInit failed!\r\n")));
        rc = FALSE;
        goto cleanUp;
    }

    // Power on the touch device
    DdsiTouchPanelPowerHandler(FALSE);

cleanUp:
    if (!rc) BSPTouchDeinit();

    DEBUGMSG(ZONE_FUNCTION, (_T("DdsiTouchPanelEnable-\r\n")));

    return rc;
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
void DdsiTouchPanelDisable(void)
{
    DEBUGMSG(ZONE_FUNCTION, (_T("DdsiTouchPanelDisable+\r\n")));

    // If we are already off, just return
    if (gTouchPowerStatus == TouchPowerOff)
        return;

    // Power off
    DdsiTouchPanelPowerHandler(TRUE);

    // Deinitialize BSP-specific settings
    BSPTouchDeinit();

    DEBUGMSG(ZONE_FUNCTION, (_T("DdsiTouchPanelDisable-\r\n")));
    return;
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
        ptrSR->SamplesPerSecondLow = TOUCH_SAMPLE_RATE_LOW;
        ptrSR->SamplesPerSecondHigh = TOUCH_SAMPLE_RATE_HIGH;
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
void DdsiTouchPanelGetPoint(TOUCH_PANEL_SAMPLE_FLAGS *pTipStateFlags,
                            INT *pUncalX, INT *pUncalY)
{
    UINT32 CurrentDown = 0;

    DEBUGMSG(ZONE_FUNCTION, (_T("DdsiTouchPanelGetPoint+\r\n")));

    // Ingore touch samples in power off state
    if (gTouchPowerStatus == TouchPowerOff)
    {
        *pTipStateFlags |= TouchSampleIgnore;
        BSPTouchInterruptDone();
        return;
    }
    // A wrapper for the above block. Done to intergate the SOC drivers
    // across platforms
     
    BSPCheckIRQ();

    CurrentDown = *pTipStateFlags & TouchSamplePreviousDownFlag;

    // Call TouchGetSampleADS7843 to get sample from touch controller
    *pTipStateFlags = BSPTouchGetSample(pUncalX, pUncalY);

    BSPTouchADCDelay();

    // Check for pen down condition
    if ((*pTipStateFlags) & TouchSampleDownFlag)
    {
        // enable touch timer (also enables timer IRQ)
        BSPTouchTimerEnable();

        //SetEvent(hTouchPanelEvent);
        if (CurrentDown)
        {
            // We must restore the TouchSamplePreviousDownFlag here to
            // be able to properly complete the calibration procedure.
            *pTipStateFlags |= TouchSamplePreviousDownFlag;
        }
    }
    // Else this must be pen up condition
    else if (CurrentDown)
    {
        // But we only want to send one pen up indication. All subsequent
        // pen up "noise" should simply be returned with "TouchSampleIgnore"
        // set so that the MDD will silently discard the readings.

        DEBUGMSG(ZONE_TIPSTATE,
            (_T("DdsiTouchPanelGetPoint:  PEN UP\r\n")));

        // Send pen up to mdd (note that we must set the valid flag or the
        // MDD will not update its internal state properly (refer to
        // CurrentDown state in tchmain.c)
        *pTipStateFlags = TouchSampleValidFlag;
    }

    DEBUGMSG(ZONE_FUNCTION, (_T("DdsiTouchPanelGetPoint-\r\n")));

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
void DdsiTouchPanelPowerHandler(BOOL boff)
{
    DEBUGMSG(ZONE_FUNCTION, (_T("DdsiTouchPanelPowerHandler+\r\n")));

    BSPTouchPowerHandler(boff);

    if (boff == TRUE)
    {
        // Set status pen power off
        gTouchPowerStatus = TouchPowerOff;
    }
    else
    {
        // Set status pen power on
        gTouchPowerStatus = TouchPowerOn;
    }

    DEBUGMSG(ZONE_FUNCTION, (_T("DdsiTouchPanelPowerHandler-\r\n")));

    return;
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
BOOL DdsiTouchPanelSetMode(INT iIndex, LPVOID  lpInput)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(lpInput);

    DEBUGMSG(ZONE_FUNCTION, (_T("DdsiTouchPanelSetMode+\r\n")));

    if (iIndex == TPSM_SAMPLERATE_LOW_ID)
    {
        gCurSampleRate = TOUCH_SAMPLE_RATE_LOW;
        BSPTouchSetSampleRate(1000/TOUCH_SAMPLE_RATE_LOW);
        gCurSampleRateSetting = 0;
    }
    else if (iIndex == TPSM_SAMPLERATE_HIGH_ID)
    {
        gCurSampleRate = TOUCH_SAMPLE_RATE_HIGH;
        BSPTouchSetSampleRate(1000/TOUCH_SAMPLE_RATE_HIGH);
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
static BOOL TouchDriverCalibrationPointGet(struct TPDC_CALIBRATION_POINT *lpOutput)
{
    INT32 cDisplayWidth = lpOutput->cDisplayWidth;
    INT32 cDisplayHeight = lpOutput->cDisplayHeight;

    int CalibrationRadiusX = cDisplayWidth / 10;
    int CalibrationRadiusY = cDisplayHeight / 10;

    DEBUGMSG(ZONE_FUNCTION, (_T("TouchDriverCalibrationPointGet+")));

    switch (lpOutput->PointNumber)
    {
    case 0:   // Middle
        lpOutput->CalibrationX = cDisplayWidth / 2;
        lpOutput->CalibrationY = cDisplayHeight / 2;
        break;

    case 1:   // Upper Left
        lpOutput->CalibrationX = CalibrationRadiusX * 2;
        lpOutput->CalibrationY = CalibrationRadiusY * 2;
        break;

    case 2:   // Lower Left
        lpOutput->CalibrationX = CalibrationRadiusX * 2;
        lpOutput->CalibrationY = cDisplayHeight - CalibrationRadiusY * 2;
        break;

    case 3:   // Lower Right
        lpOutput->CalibrationX = cDisplayWidth - CalibrationRadiusX * 2;
        lpOutput->CalibrationY = cDisplayHeight - CalibrationRadiusY * 2;
        break;

    case 4:   // Upper Right
        lpOutput->CalibrationX = cDisplayWidth - CalibrationRadiusX * 2;
        lpOutput->CalibrationY = CalibrationRadiusY * 2;
        break;

    default:
        lpOutput->CalibrationX = cDisplayWidth / 2;
        lpOutput->CalibrationY = cDisplayHeight / 2;
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    DEBUGMSG(ZONE_FUNCTION, (_T("TouchDriverCalibrationPoints-\r\n")));

    return TRUE;
}
