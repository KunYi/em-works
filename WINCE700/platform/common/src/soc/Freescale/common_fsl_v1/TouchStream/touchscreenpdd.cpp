//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
#pragma warning(push)
#pragma warning(disable: 4127 4201)
//------------------------------------------------------------------------------
// Public
//
#include <windows.h>
#include <types.h>
#include <nkintr.h>
#include <creg.hxx>
#include <tchstream.h>
#include <tchstreamddsi.h>
#include <pm.h>

//------------------------------------------------------------------------------
// Platform
//
#include <ceddk.h>
#include "touchscreenpdd.h"

//------------------------------------------------------------------------------
// Defines
//

//------------------------------------------------------------------------------
// Debug zones
//
#ifndef SHIP_BUILD

#undef ZONE_ERROR
#undef ZONE_WARN
#undef ZONE_FUNCTION
#undef ZONE_INFO
#undef ZONE_TIPSTATE

#define ZONE_ERROR          DEBUGZONE(0)
#define ZONE_WARN           DEBUGZONE(1)
#define ZONE_FUNCTION       DEBUGZONE(2)
#define ZONE_INFO           DEBUGZONE(3)
#define ZONE_TIPSTATE       DEBUGZONE(4)

#endif


//------------------------------------------------------------------------------
// globals
//
static DWORD                          s_mddContext;
static PFN_TCH_MDD_REPORTSAMPLESET    s_pfnMddReportSampleSet;
static HANDLE                         s_hISTThread;

//==============================================================================
// Function Name: TchPdd_Init
//
// Description: PDD should always implement this function. MDD calls it during
//              initialization to fill up the function table with rest of the
//              Helper functions.
//
// Arguments:
//              [IN] pszActiveKey - current active touch driver key
//              [IN] pMddIfc - MDD interface info
//              [OUT]pPddIfc - PDD interface (the function table to be filled)
//              [IN] hMddContext - mdd context (send to MDD in callback fn)
//
// Ret Value:   pddContext.
//==============================================================================
extern "C" DWORD WINAPI TchPdd_Init(LPCTSTR pszActiveKey,
    TCH_MDD_INTERFACE_INFO* pMddIfc,
    TCH_PDD_INTERFACE_INFO* pPddIfc,
    DWORD hMddContext
    )
{
    DWORD pddContext = 0;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("TchPdd_Init+\r\n")));

    // Initialize once only
    if (s_TouchDevice.bInitialized)
    {
        pddContext = (DWORD) &s_TouchDevice;
        goto cleanup;
    }


    // Remember the callback function pointer
    s_pfnMddReportSampleSet = pMddIfc->pfnMddReportSampleSet;

    // Remember the mdd context
    s_mddContext = hMddContext;

    s_TouchDevice.nSampleRate = DEFAULT_SAMPLE_RATE;
    s_TouchDevice.dwPowerState = D0;
    s_TouchDevice.dwSamplingTimeOut = INFINITE;
    s_TouchDevice.bTerminateIST = FALSE;
    s_TouchDevice.hTouchPanelEvent = 0;

    // Initialize HW
    if (!PDDInitializeHardware(pszActiveKey))
    {
        DEBUGMSG(ZONE_ERROR,  (TEXT("ERROR: TOUCH: Failed to initialize touch PDD.\r\n")));
        goto cleanup;
    }

    //Calibrate the screen, if the calibration data is not already preset in the registry
    PDDStartCalibrationThread();

    //Initialization of the h/w is done
    s_TouchDevice.bInitialized = TRUE;

    pddContext = (DWORD) &s_TouchDevice;

    // fill up pddifc table
    pPddIfc->version        = 1;
    pPddIfc->pfnDeinit      = TchPdd_Deinit;
    pPddIfc->pfnIoctl       = TchPdd_Ioctl;
    pPddIfc->pfnPowerDown   = TchPdd_PowerDown;
    pPddIfc->pfnPowerUp     = TchPdd_PowerUp;


cleanup:
    DEBUGMSG(ZONE_FUNCTION, (TEXT("TchPdd_Init-\r\n")));
    return pddContext;
}


//==============================================================================
// Function Name: TchPdd_DeInit
//
// Description: MDD calls it during deinitialization. PDD should deinit hardware
//              and deallocate memory etc.
//
// Arguments:   [IN] hPddContext. pddcontext returned in TchPdd_Init
//
// Ret Value:   None
//
//==============================================================================
void WINAPI TchPdd_Deinit(DWORD hPddContext)
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("TchPdd_Deinit+\r\n")));

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hPddContext);

    // Close the IST and release the resources before
    // de-initializing.
    PDDTouchPanelDisable();

    //  Shutdown HW
    PDDDeinitializeHardware();

    s_TouchDevice.bInitialized = FALSE;


    DEBUGMSG(ZONE_FUNCTION, (TEXT("TchPdd_Deinit-\r\n")));
}


//==============================================================================
// Function Name: TchPdd_Ioctl
//
// Description: The MDD controls the touch PDD through these IOCTLs.
//
// Arguments:   [IN] hPddContext. pddcontext returned in TchPdd_Init
//              DOWRD dwCode. IOCTL code
//              PBYTE pBufIn. Input Buffer pointer
//              DWORD dwLenIn. Input buffer length
//              PBYTE pBufOut. Output buffer pointer
//              DWORD dwLenOut. Output buffer length
//              PWORD pdwAcutalOut. Actual output buffer length.
//
// Ret Value:   TRUE if success else FALSE. SetLastError() if FALSE.
//==============================================================================
BOOL WINAPI TchPdd_Ioctl(DWORD hPddContext,
      DWORD dwCode,
      PBYTE pBufIn,
      DWORD dwLenIn,
      PBYTE pBufOut,
      DWORD dwLenOut,
      PDWORD pdwActualOut
)
{
    DWORD dwResult = ERROR_INVALID_PARAMETER;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hPddContext);

    // Only allow kernel process to call PDD ioctls.
    if (GetCurrentProcessId() != GetDirectCallerProcessId()) {
        DEBUGMSG(ZONE_ERROR, (L"TchPdd_Ioctl: Can be called only from device process (caller process id 0x%08x)\r\n", \
            GetDirectCallerProcessId()));
        SetLastError(ERROR_ACCESS_DENIED);
        return FALSE;
    }

    switch (dwCode)
    {
        //  Enable touch panel
        case IOCTL_TOUCH_ENABLE_TOUCHPANEL:
            DEBUGMSG(ZONE_FUNCTION, (L"TchPdd_Ioctl: IOCTL_TOUCH_ENABLE_TOUCHPANEL\r\n"));
            PDDTouchPanelEnable();
            dwResult = ERROR_SUCCESS;
            break;

        //  Disable touch panel
        case IOCTL_TOUCH_DISABLE_TOUCHPANEL:
            DEBUGMSG(ZONE_FUNCTION, (L"TchPdd_Ioctl: IOCTL_TOUCH_DISABLE_TOUCHPANEL\r\n"));
            PDDTouchPanelDisable();
            dwResult = ERROR_SUCCESS;
            break;


        //  Get current sample rate
        case IOCTL_TOUCH_GET_SAMPLE_RATE:
            DEBUGMSG(ZONE_FUNCTION, (L"TchPdd_Ioctl: IOCTL_TOUCH_GET_SAMPLE_RATE\r\n"));

            //  Check parameter validity
            if ((pBufOut == NULL) || (pdwActualOut == NULL) || (dwLenOut < sizeof(DWORD)))
            {
                dwResult = ERROR_INVALID_PARAMETER;
                break;
            }
            __try
            {
                *pdwActualOut = sizeof(DWORD);
                //  Get the sample rate
                *((DWORD*)pBufOut) = s_TouchDevice.nSampleRate;
                dwResult = ERROR_SUCCESS;
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
                DEBUGMSG(ZONE_ERROR, (L"TchPdd_Ioctl: Exception in IOCTL_TOUCH_SET_SAMPLE_RATE\r\n"));
            }

            break;

        //  Set the current sample rate
        case IOCTL_TOUCH_SET_SAMPLE_RATE:
            DEBUGMSG(ZONE_FUNCTION, (L"TchPdd_Ioctl: IOCTL_TOUCH_SET_SAMPLE_RATE\r\n"));

            //  Check parameter validity
            if ((pBufIn == NULL) || (dwLenIn < sizeof(DWORD)))
            {
                dwResult = ERROR_INVALID_PARAMETER;
                break;
            }
            __try
            {
                //  Set the sample rate
                s_TouchDevice.nSampleRate = *((DWORD*)pBufIn);
                dwResult = ERROR_SUCCESS;
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
                DEBUGMSG(ZONE_ERROR, (L"TchPdd_Ioctl: Exception in IOCTL_TOUCH_GET_SAMPLE_RATE\r\n"));
            }

            break;

        //  Get touch properties
        case IOCTL_TOUCH_GET_TOUCH_PROPS:
            DEBUGMSG(ZONE_FUNCTION, (L"TchPdd_Ioctl: IOCTL_TOUCH_GET_TOUCH_PROPS\r\n"));

            //  Check parameter validity
            if ((pBufOut == NULL) || (pdwActualOut == NULL) || (dwLenOut < sizeof(TCH_PROPS)))
            {
                dwResult = ERROR_INVALID_PARAMETER;
                break;
            }
            __try
            {
                *pdwActualOut = sizeof(TCH_PROPS);

                //  Fill out the touch driver properties
                ((TCH_PROPS*)pBufOut)->minSampleRate            = TOUCHPANEL_SAMPLE_RATE_LOW;
                ((TCH_PROPS*)pBufOut)->maxSampleRate            = TOUCHPANEL_SAMPLE_RATE_HIGH;
                ((TCH_PROPS*)pBufOut)->minCalCount              = 5;
                ((TCH_PROPS*)pBufOut)->maxSimultaneousSamples   = 1;
                ((TCH_PROPS*)pBufOut)->touchType                = TOUCHTYPE_SINGLETOUCH;
                ((TCH_PROPS*)pBufOut)->xRangeMin                = ADC_SAMPLE_MIN_VALUE;
                ((TCH_PROPS*)pBufOut)->xRangeMax                = ADC_SAMPLE_MAX_VALUE;
                ((TCH_PROPS*)pBufOut)->yRangeMin                = ADC_SAMPLE_MIN_VALUE;
                ((TCH_PROPS*)pBufOut)->yRangeMax                = ADC_SAMPLE_MAX_VALUE;
                ((TCH_PROPS*)pBufOut)->calHoldSteadyTime        = CAL_HOLD_STEADY_TIME;
                ((TCH_PROPS*)pBufOut)->calDeltaReset            = CAL_DELTA_RESET;


                dwResult = ERROR_SUCCESS;
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
                DEBUGMSG(ZONE_ERROR, (L"TchPdd_Ioctl: Exception in IOCTL_TOUCH_GET_TOUCH_PROPS\r\n"));
            }
            break;

        //  Power management IOCTLs
        case IOCTL_POWER_CAPABILITIES:
            DEBUGMSG(ZONE_FUNCTION, (L"TchPdd_Ioctl: IOCTL_POWER_CAPABILITIES\r\n"));

            //  Check parameter validity
            if ((pBufOut == NULL) || (pdwActualOut == NULL) || (dwLenOut < sizeof(POWER_CAPABILITIES)))
            {
                dwResult = ERROR_INVALID_PARAMETER;
                break;
            }
            __try
            {
                PPOWER_CAPABILITIES ppc = (PPOWER_CAPABILITIES) pBufOut;
                memset(ppc, 0, sizeof(*ppc));
                ppc->DeviceDx = DX_MASK(D0) | DX_MASK(D1) | DX_MASK(D2) | DX_MASK(D3) | DX_MASK(D4);

                *pdwActualOut = sizeof(POWER_CAPABILITIES);

                dwResult = ERROR_SUCCESS;
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
                DEBUGMSG(ZONE_ERROR, (L"TchPdd_Ioctl: Exception in IOCTL_POWER_CAPABILITIES\r\n"));
            }

            break;

        case IOCTL_POWER_GET:
            DEBUGMSG(ZONE_FUNCTION, (L"TchPdd_Ioctl: IOCTL_POWER_GET\r\n"));

            //  Check parameter validity
            if ((pBufOut == NULL) || (pdwActualOut == NULL) || (dwLenOut < sizeof(CEDEVICE_POWER_STATE)))
            {
                dwResult = ERROR_INVALID_PARAMETER;
                break;
            }
            __try
            {
                *(PCEDEVICE_POWER_STATE) pBufOut = (CEDEVICE_POWER_STATE) s_TouchDevice.dwPowerState;

                *pdwActualOut = sizeof(CEDEVICE_POWER_STATE);

                dwResult = ERROR_SUCCESS;
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
                DEBUGMSG(ZONE_ERROR, (L"TchPdd_Ioctl: Exception in IOCTL_POWER_GET\r\n"));
            }
            break;

        case IOCTL_POWER_SET:
            DEBUGMSG(ZONE_FUNCTION, (L"TchPdd_Ioctl: IOCTL_POWER_SET\r\n"));
            CEDEVICE_POWER_STATE dx;

            //  Check parameter validity
            if ((pBufOut == NULL) || (dwLenOut < sizeof(CEDEVICE_POWER_STATE)))
            {
                dwResult = ERROR_INVALID_PARAMETER;
                break;
            }
            __try
            {
                dx = *(CEDEVICE_POWER_STATE*)pBufOut;
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
                DEBUGMSG(ZONE_ERROR, (L"TchPdd_Ioctl: Exception in IOCTL_POWER_SET\r\n"));
                dwResult = ERROR_INVALID_PARAMETER;
                break;
            }

            if( VALID_DX(dx) )
            {
                DEBUGMSG(ZONE_FUNCTION, (L"TchPdd_Ioctl: IOCTL_POWER_SET = to D%u\r\n", dx));
                s_TouchDevice.dwPowerState = dx;

                if (pdwActualOut)
                    *pdwActualOut = sizeof(CEDEVICE_POWER_STATE);

                //  Enable touchscreen for D0-D2; otherwise disable
                switch( dx )
                {
                    case D0 :
                    case D1 :
                    case D2 :
                    case D3 :
                    case D4 :
                        PDDTouchPanelPowerHandler(dx);
                        break;

                    default:
                        //  Ignore
                        break;
                }

                dwResult = ERROR_SUCCESS;
            }
            break;

        default:
            dwResult = ERROR_NOT_SUPPORTED;
            break;
    }

    if (dwResult != ERROR_SUCCESS)
    {
        SetLastError(dwResult);
        return FALSE;
    }
    return TRUE;
}


//==============================================================================
// Function Name: TchPdd_PowerUp
//
// Description: MDD passes xxx_PowerUp stream interface call to PDD.
//
// Arguments:   [IN] hPddContext. pddcontext returned in TchPdd_Init
//
// Ret Value:   None
//==============================================================================
void WINAPI TchPdd_PowerUp(
    DWORD hPddContext
    )
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("TchPdd_PowerUp+\r\n")));
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hPddContext);
    DEBUGMSG(ZONE_FUNCTION, (TEXT("TchPdd_PowerUp-\r\n")));
}

//==============================================================================
// Function Name: TchPdd_PowerDown
//
// Description: MDD passes xxx_PowerDown stream interface call to PDD.
//
// Arguments:   [IN] hPddContext. pddcontext returned in TchPdd_Init
//
// Ret Value:   None
//==============================================================================
void WINAPI TchPdd_PowerDown(
    DWORD hPddContext
    )
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("TchPdd_PowerDown+\r\n")));
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hPddContext);
    DEBUGMSG(ZONE_FUNCTION, (TEXT("TchPdd_PowerDown-\r\n")));
}


//==============================================================================
//Internal Functions
//==============================================================================

//==============================================================================
// Function Name: PDDCalibrationThread
//
// Description: This function is called from the PDD Init to calibrate the screen.
//              If the calibration data is already present in the registry,
//              this step is skipped, else a call is made into the GWES for calibration.
//
// Arguments:   None.
//
// Ret Value:   Success(1), faliure(0)
//==============================================================================
static HRESULT PDDCalibrationThread()
{
    HKEY hKey;
    DWORD dwType;
    LONG lResult;
    HANDLE hAPIs;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("CalibrationThread+\r\n")));

    // try to open [HKLM\hardware\devicemap\touch] key
    if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, RK_HARDWARE_DEVICEMAP_TOUCH, 0, KEY_ALL_ACCESS, &hKey))
    {
        DEBUGMSG(ZONE_CALIBRATE, (TEXT("CalibrationThread: calibration: Can't find [HKLM/%s]\r\n"), RK_HARDWARE_DEVICEMAP_TOUCH));
        return E_FAIL;
    }

    // check for calibration data (query the type of data only)
    lResult = RegQueryValueEx(hKey, RV_CALIBRATION_DATA, 0, &dwType, NULL, NULL);
    RegCloseKey(hKey);
    if (lResult == ERROR_SUCCESS)
    {
        // registry contains calibration data, return
        return S_OK;
    }

    hAPIs = OpenEvent(EVENT_ALL_ACCESS, FALSE, TEXT("SYSTEM/GweApiSetReady"));
    if (hAPIs)
    {
        WaitForSingleObject(hAPIs, INFINITE);
        CloseHandle(hAPIs);
    }

    // Perform calibration
    TouchCalibrate();

#ifdef DEBUG
    TCHAR szCalibrationData[100];
    DWORD Size = sizeof(szCalibrationData);

    // try to open [HKLM\hardware\devicemap\touch] key
    if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, RK_HARDWARE_DEVICEMAP_TOUCH, 0, KEY_ALL_ACCESS, &hKey))
    {
        DEBUGMSG(ZONE_CALIBRATE, (TEXT("CalibrationThread: calibration: Can't find [HKLM/%s]\r\n"), RK_HARDWARE_DEVICEMAP_TOUCH));
        return E_FAIL;
    }

    // display new calibration data
    lResult = RegQueryValueEx(hKey, RV_CALIBRATION_DATA, 0, &dwType, NULL, NULL);
    if (lResult == ERROR_SUCCESS)
    {
        RegQueryValueEx(hKey, RV_CALIBRATION_DATA, 0, &dwType, (BYTE *) szCalibrationData, (DWORD *) &Size);
    }

    DEBUGMSG(ZONE_CALIBRATE, (TEXT("CalibrationThread: New Calibration data-->%s\r\n"), szCalibrationData));
    RegCloseKey(hKey);
#endif

    DEBUGMSG(ZONE_FUNCTION, (TEXT("CalibrationThread-\r\n")));

    return S_OK;
}


//==============================================================================
// Function Name: PDDStartCalibrationThread
//
// Description: This function is creates the calibration thread with
//              PDDCalibrationThread as entry.
//
// Arguments:   None.
//
// Ret Value:   None
//==============================================================================
void PDDStartCalibrationThread()
{
    HANDLE hThread;

    hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)PDDCalibrationThread, NULL, 0, NULL);
    // We don't need the handle, close it here
    CloseHandle(hThread);
}

//==============================================================================
// Function: PDDTouchPanelPowerHandler
//
// This function indicates to the driver that the system is entering
// or leaving the suspend state.
//
// Parameters:
//      dx
//          [in] TRUE indicates that the system is turning off. FALSE
//          indicates that the system is turning on.
//
// Returns:
//      None.
//==============================================================================
void PDDTouchPanelPowerHandler(CEDEVICE_POWER_STATE dx)
{
    DEBUGMSG(ZONE_FUNCTION, (_T("PDDTouchPanelPowerHandler+\r\n")));

    if (s_TouchDevice.hTouchPanelEvent != NULL)
    {
        switch (dx)
        {
            case D0:
            case D1:
            case D2:
                BSPTouchPowerHandler(FALSE);
                break;
            case D3:
                //Ack. any pending interrupts.
                BSPTouchInterruptDone();
                break;
            case D4:
                BSPTouchPowerHandler(TRUE);
                break;
        }
    }

    DEBUGMSG(ZONE_FUNCTION, (_T("PDDTouchPanelPowerHandler-\r\n")));
    return;
}

//==============================================================================
// Function Name: PDDTouchPanelGetPoint
//
// Description: This function is used to read the touch coordinates from the h/w.
//
// Arguments:
//                  TOUCH_PANEL_SAMPLE_FLAGS * - Pointer to the sample flags. This flag is filled with the
//                                                                      values TouchSampleDownFlag or TouchSampleValidFlag;
//                  INT * - Pointer to the x coordinate of the sample.
//                  INT * - Pointer to the y coordinate of the sample.
//
// Ret Value:   None
//==============================================================================
void PDDTouchPanelGetPoint(
    TOUCH_PANEL_SAMPLE_FLAGS *pTipStateFlags,
    INT *pUncalX,
    INT *pUncalY
    )
{

    DEBUGMSG(ZONE_FUNCTION, (_T("PDDTouchPanelGetPoint+\r\n")));


    // Call BSPTouchGetSample to get sample from touch controller
    *pTipStateFlags = BSPTouchGetSample(pUncalX, pUncalY);


    // Check for pen down condition
    if ((*pTipStateFlags) & TouchSampleDownFlag)
    {
        s_TouchDevice.dwSamplingTimeOut = 1000/s_TouchDevice.nSampleRate;
    }
    // Else this must be pen up condition
    else
    {
        s_TouchDevice.dwSamplingTimeOut = INFINITE;
        // But we only want to send one pen up indication. All subsequent
        // pen up "noise" should simply be returned with "TouchSampleIgnore"
        // set so that the MDD will silently discard the readings.

        DEBUGMSG(ZONE_TIPSTATE, (_T("PDDTouchPanelGetPoint:  PEN UP\r\n")));

        // Send pen up to mdd (note that we must set the valid flag or the
        // MDD will not update its internal state properly (refer to
        // CurrentDown state in tchmain.c)
        *pTipStateFlags = TouchSampleValidFlag;
    }

    DEBUGMSG(ZONE_FUNCTION, (_T("PDDTouchPanelGetPoint-\r\n")));
    return;
}

//==============================================================================
// Function Name: PDDTouchIST
//
// Description: This is the IST which waits on the touch event or Time out value.
//              Normally the IST waits on the touch event infinitely, but once the
//              pen down condition is recognized the time out interval is changed
//              to dwSamplingTimeOut.
//
// Arguments:
//                  PVOID - Reseved not currently used.
//
// Ret Value:   None
//==============================================================================
ULONG PDDTouchIST(PVOID   reserved)
{
    TOUCH_PANEL_SAMPLE_FLAGS    SampleFlags = 0;
    CETOUCHINPUT input;
    INT32                       RawX, RawY;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(reserved);

    DEBUGMSG(ZONE_TOUCH_IST, (L"PDDTouchIST: IST thread started\r\n"));

    //  Loop until told to stop
    while(!s_TouchDevice.bTerminateIST)
    {
        //  Wait for touch IRQ, timeout or terminate flag
       WaitForSingleObject(s_TouchDevice.hTouchPanelEvent, s_TouchDevice.dwSamplingTimeOut);

        //  Check for terminate flag
        if (s_TouchDevice.bTerminateIST)
            break;

        PDDTouchPanelGetPoint( &SampleFlags, &RawX, &RawY);

        if ( SampleFlags & TouchSampleIgnore )
        {
            // do nothing, not a valid sample
            continue;
        }

        //convert x-y coordination to one sample set before sending it to touch MDD.
        if(ConvertTouchPanelToTouchInput(SampleFlags, RawX, RawY, &input))
        {
            //send this 1 sample to mdd
            s_pfnMddReportSampleSet(s_mddContext, 1, &input);
        }

        DEBUGMSG(ZONE_TOUCH_SAMPLES, ( TEXT( "Sample:   (%d, %d)\r\n" ), RawX, RawY ) );
    }

    BSPTouchInterruptDone();
    BSPTouchInterruptDisable();

    CloseHandle(s_TouchDevice.hTouchPanelEvent);
    s_TouchDevice.hTouchPanelEvent = NULL;

    DEBUGMSG(ZONE_TOUCH_IST, (L"PDDTouchIST: IST thread ending\r\n"));
    return ERROR_SUCCESS;
}

//==============================================================================
// Function Name: PDDInitializeHardware
//
// Description: This routine configures the h/w
//
// Arguments:  None
//
// Ret Value:   TRUE - Success
//                   FAIL - Failure
//==============================================================================
BOOL PDDInitializeHardware(LPCTSTR pszActiveKey)
{
    BOOL   rc = FALSE;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("PDDInitializeHardware+\r\n")));

    // Read parameters from registry
    if (GetDeviceRegistryParams(
            pszActiveKey,
            &s_TouchDevice,
            dimof(s_deviceRegParams),
            s_deviceRegParams) != ERROR_SUCCESS)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("ERROR: PDDInitializeHardware: Error reading from Registry.\r\n")));
        goto cleanup;
    }

    // Nothing to do here as of now, can do some BSP initialization here.
    rc = TRUE;


cleanup:

    DEBUGMSG(ZONE_FUNCTION, (TEXT("PDDInitializeHardware-\r\n")));
    if( rc == FALSE )
    {
        PDDDeinitializeHardware();
    }

    return rc;
}


//==============================================================================
// Function Name: PDDDeinitializeHardware
//
// Description: This routine Deinitializes the h/w
//
// Arguments:  None
//
// Ret Value:   None
//==============================================================================
VOID PDDDeinitializeHardware()
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("PDDDeinitializeHardware+\r\n")));

    // Nothing to do here as of now, can do some BSP deinitialization here.
    DEBUGMSG(ZONE_FUNCTION, (TEXT("PDDDeinitializeHardware-\r\n")));
}


//==============================================================================
// Function Name: PDDTouchPanelEnable
//
// Description: This routine creates the touch thread(if it is not already created)
//              initializes the interrupt and unmasks it.
//
// Arguments:  None
//
// Ret Value:   TRUE - Success
//==============================================================================
BOOL  PDDTouchPanelEnable()
{
    BOOL rc = TRUE;
    DEBUGMSG(ZONE_FUNCTION, (TEXT("PDDTouchPanelEnable+\r\n")));

    //  Check if already running
    if( s_TouchDevice.hTouchPanelEvent == NULL )
    {
        //  Clear terminate flag
        s_TouchDevice.bTerminateIST = FALSE;

        //  Create touch event
        s_TouchDevice.hTouchPanelEvent = BSPTouchAttach();
        if( !s_TouchDevice.hTouchPanelEvent )
        {
            DEBUGMSG(ZONE_ERROR,(TEXT("PDDTouchPanelEnable: Failed to initialize touch event.\r\n")));
            return FALSE;
        }

        rc = BSPTouchInterruptEnable();

        s_TouchDevice.dwSamplingTimeOut = INFINITE;

        //  Create IST thread
        s_hISTThread = CreateThread( NULL, 0, PDDTouchIST, 0, 0, NULL );
        if( s_hISTThread == NULL )
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("PDDTouchPanelEnable: Failed to create IST thread\r\n")));
            return FALSE;
        }

        // set IST thread priority
        CeSetThreadPriority (s_hISTThread, s_TouchDevice.dwISTPriority);
    }

    DEBUGMSG(ZONE_FUNCTION, (TEXT("PDDTouchPanelEnable-\r\n")));
    return rc;
}

//==============================================================================
// Function Name: PDDTouchPanelDisable
//
// Description: This routine closes the IST and releases other resources.
//
// Arguments:  None
//
// Ret Value:   TRUE - Success
//==============================================================================
VOID  PDDTouchPanelDisable()
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("PDDTouchPanelDisable+\r\n")));

    //  Disable touch interrupt service thread
    if( s_TouchDevice.hTouchPanelEvent )
    {
        BSPTouchInterruptDone();
        BSPTouchInterruptDisable();

        s_TouchDevice.bTerminateIST = TRUE;
        SetEvent( s_TouchDevice.hTouchPanelEvent );
        //Wait 3sec for the IST to exit and if the thread doesn't exit, forcefully terminate
        if (WaitForSingleObject(s_hISTThread, THREAD_EXIT_TIMEOUT) != WAIT_OBJECT_0)
        {
          // Do this only when the thread is not terminated properly.
          DEBUGCHK(FALSE);
          TerminateThread(s_hISTThread, (DWORD)-1);
        }
        CloseHandle(s_hISTThread);
        s_hISTThread = NULL;
    }

    DEBUGMSG(ZONE_FUNCTION, (TEXT("PDDTouchPanelDisable-\r\n")));
}
