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
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004, Motorola Inc. All Rights Reserved
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  touchpdd.cpp
//
//  Provides the PDD code for the helper touch driver routines.
//
//-----------------------------------------------------------------------------

#include <windows.h>
#include <nkintr.h>
#include <tchstream.h>
#include <tchstreamddsi.h>
#include <pm.h>

#include "touchpdd.h"
#include "csp.h"

//-----------------------------------------------------------------------------
// External Functions
extern BOOL BSPTouchInit();
extern void BSPTouchDeinit(void);
extern TOUCH_PANEL_SAMPLE_FLAGS BSPTouchGetSample(USHORT *x, USHORT *y);
extern void BSPTouchPowerHandler(BOOL boff);
extern BOOL BSPTouchInterruptDisable();
extern BOOL BSPTouchInterruptEnable(TCHAR* strTouchEventName);


//-----------------------------------------------------------------------------
// External Variables

//-----------------------------------------------------------------------------
// Defines

//-----------------------------------------------------------------------------
// Types

//-----------------------------------------------------------------------------
// Global Variables

//-----------------------------------------------------------------------------
// Local Variables
static DWORD gTouchPowerStatus = TouchPowerOff;
static TCHAR* gTouchEventName = _T("MX27_TOUCH_EVENT");

//-----------------------------------------------------------------------------
// Local Functions
ULONG PddTouchIST(PVOID reserved );
BOOL PddEnableTouchIST( BOOL bEnable );

BOOL TouchPanelEnable(void);
void TouchPanelDisable(void);
void TouchPanelPowerHandler(BOOL boff);

//  Touch MDD info
static DWORD                    m_mddContext;
//static PFN_TCH_MDD_REPORTSAMPLE m_pfnMddReportSample;
static PFN_TCH_MDD_REPORTSAMPLESET m_pfnMddReportSampleSet;


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
// local data structures
//

typedef struct
{
    BOOL    bInitialized;
    LONG    nSampleRate;
    LONG    nPenPriority256;
    DWORD   dwPowerState;

    BOOL    bTerminateIST;
    HANDLE  hTouchEvent;
    HANDLE  hTouchIST;
}
TOUCH_DEVICE;

static TOUCH_DEVICE s_TouchDevice =
{
    FALSE
};

//==============================================================================
// Description: PDD should always implement this function. MDD calls it during
//              initialization to fill up the function table with rest of the
//              Helper functions.
//
// Arguments:   [IN] pszActiveKey - current active touch driver key
//              [IN] pMddIfc - MDD interface info
//              [OUT]pPddIfc - PDD interface (the function table to be filled)
//              [IN] hMddContext - mdd context (send to MDD in callback fn)
//
// Ret Value:   pddContext.
//
extern "C"
DWORD
WINAPI
TchPdd_Init(
    LPCTSTR pszActiveKey,
    TCH_MDD_INTERFACE_INFO* pMddIfc,
    TCH_PDD_INTERFACE_INFO* pPddIfc,
    DWORD hMddContext
    )
{
    DWORD pddContext = 0;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("TchPdd_Init+\r\n")));


    // fill up pddifc table
    pPddIfc->version        = 1;
    pPddIfc->pfnDeinit      = TchPdd_Deinit;
    pPddIfc->pfnIoctl       = TchPdd_Ioctl;
    pPddIfc->pfnPowerDown   = TchPdd_PowerDown;
    pPddIfc->pfnPowerUp     = TchPdd_PowerUp;

    // Remember the callback function pointer
    //m_pfnMddReportSample = pMddIfc->pfnMddReportSample;
    m_pfnMddReportSampleSet = pMddIfc->pfnMddReportSampleSet;

    // Remember the mdd context
    m_mddContext = hMddContext;

    // Initialize once only
    if (s_TouchDevice.bInitialized)
    {
        pddContext = (DWORD) &s_TouchDevice;
        goto cleanup;
    }

    // Initialize HW
    TouchPanelEnable();
    // Get the ADC up
    TouchPanelPowerHandler(FALSE);


    //  Done
    s_TouchDevice.bInitialized = TRUE;
    s_TouchDevice.nSampleRate = TOUCH_SAMPLE_RATE_HIGH;
    s_TouchDevice.nPenPriority256 = TOUCH_PRIORITY;
    pddContext = (DWORD) &s_TouchDevice;

cleanup:
    DEBUGMSG(ZONE_FUNCTION, (TEXT("TchPdd_Init-\r\n")));
    return pddContext;
}


//==============================================================================
// Description: MDD calls it during deinitialization. PDD should deinit hardware
//              and deallocate memory etc.
//
// Arguments:   [IN] hPddContext. pddcontext returned in TchPdd_Init
//
// Ret Value:   None
//
void
WINAPI
TchPdd_Deinit(
    DWORD hPddContext
    )
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("TchPdd_Deinit+\r\n")));

    //  Shutdown HW
    TouchPanelDisable();


    s_TouchDevice.bInitialized = FALSE;


    DEBUGMSG(ZONE_FUNCTION, (TEXT("TchPdd_Deinit-\r\n")));
}

//==============================================================================
// Description: MDD passes xxx_PowerUp stream interface call to PDD.
//
// Arguments:   [IN] hPddContext. pddcontext returned in TchPdd_Init
//
// Ret Value:   None
//
void
WINAPI
TchPdd_PowerUp(
    DWORD hPddContext
    )
{
}

//==============================================================================
// Description: MDD passes xxx_PowerDown stream interface call to PDD.
//
// Arguments:   [IN] hPddContext. pddcontext returned in TchPdd_Init
//
// Ret Value:   None
//
void
WINAPI
TchPdd_PowerDown(
    DWORD hPddContext
    )
{
}

//==============================================================================
// Description: IOCTLs passed to PDD.
//
// Arguments:   [IN] hPddContext. pddcontext returned in TchPdd_Init
//
// Ret Value:   TRUE if success else FALSE. SetLastError() if FALSE.
//
BOOL
WINAPI
TchPdd_Ioctl(
  DWORD hPddContext,
  DWORD dwCode,
  PBYTE pBufIn,
  DWORD dwLenIn,
  PBYTE pBufOut,
  DWORD dwLenOut,
  PDWORD pdwActualOut
)
{
    DWORD dwResult = ERROR_INVALID_PARAMETER;

    switch (dwCode)
    {
        //  Enable touch panel
        case IOCTL_TOUCH_ENABLE_TOUCHPANEL:
            DEBUGMSG(ZONE_FUNCTION, (L"TchPdd_Ioctl: IOCTL_TOUCH_ENABLE_TOUCHPANEL\r\n"));
            PddEnableTouchIST( TRUE );
            dwResult = ERROR_SUCCESS;
            break;

        //  Disable touch panel
        case IOCTL_TOUCH_DISABLE_TOUCHPANEL:
            DEBUGMSG(ZONE_FUNCTION, (L"TchPdd_Ioctl: IOCTL_TOUCH_DISABLE_TOUCHPANEL\r\n"));
            PddEnableTouchIST( FALSE );
            dwResult = ERROR_SUCCESS;
            break;


        //  Get current sample rate
        case IOCTL_TOUCH_GET_SAMPLE_RATE:
            DEBUGMSG(ZONE_FUNCTION, (L"TchPdd_Ioctl: IOCTL_TOUCH_GET_SAMPLE_RATE\r\n"));

            //  Check parameter validity
            if ((pBufOut != NULL) || (dwLenOut == sizeof(DWORD)))
            {
                if (pdwActualOut)
                    *pdwActualOut = sizeof(DWORD);

                //  Get the sample rate
                *((DWORD*)pBufOut) = s_TouchDevice.nSampleRate;
                dwResult = ERROR_SUCCESS;
            }
            break;

        //  Set the current sample rate
        case IOCTL_TOUCH_SET_SAMPLE_RATE:
            DEBUGMSG(ZONE_FUNCTION, (L"TchPdd_Ioctl: IOCTL_TOUCH_SET_SAMPLE_RATE\r\n"));

            //  Check parameter validity
            if ((pBufIn != NULL) || (dwLenIn == sizeof(DWORD)))
            {
                //  Set the sample rate
                s_TouchDevice.nSampleRate = *((DWORD*)pBufIn);
                dwResult = ERROR_SUCCESS;

            }
            break;

        //  Get touch properties
        case IOCTL_TOUCH_GET_TOUCH_PROPS:
            //  Check parameter validity
            if (pBufOut != NULL || dwLenOut == sizeof(TCH_PROPS))
            {
                if (pdwActualOut)
                    *pdwActualOut = sizeof(TCH_PROPS);\

                //  Fill out the touch driver properties
                ((TCH_PROPS*)pBufOut)->minSampleRate        = TOUCH_SAMPLE_RATE_LOW;
                ((TCH_PROPS*)pBufOut)->maxSampleRate        = TOUCH_SAMPLE_RATE_HIGH;
                ((TCH_PROPS*)pBufOut)->minCalCount          = 5;
                dwResult = ERROR_SUCCESS;
            }
            break;

        //  Power management IOCTLs
        case IOCTL_POWER_CAPABILITIES:
            DEBUGMSG(ZONE_FUNCTION, (L"TchPdd_Ioctl: IOCTL_POWER_CAPABILITIES\r\n"));
            if (pBufOut != NULL && dwLenOut == sizeof(POWER_CAPABILITIES))
            {
                PPOWER_CAPABILITIES ppc = (PPOWER_CAPABILITIES) pBufOut;
                memset(ppc, 0, sizeof(*ppc));
                ppc->DeviceDx = DX_MASK(D0) | DX_MASK(D1) | DX_MASK(D2) | DX_MASK(D3) | DX_MASK(D4);

                if (pdwActualOut)
                    *pdwActualOut = sizeof(POWER_CAPABILITIES);

                dwResult = ERROR_SUCCESS;
            }
            break;

        case IOCTL_POWER_GET:
            DEBUGMSG(ZONE_FUNCTION, (L"TchPdd_Ioctl: IOCTL_POWER_GET\r\n"));
            if(pBufOut != NULL && dwLenOut == sizeof(CEDEVICE_POWER_STATE))
            {
                *(PCEDEVICE_POWER_STATE) pBufOut = (CEDEVICE_POWER_STATE) s_TouchDevice.dwPowerState;

                if (pdwActualOut)
                    *pdwActualOut = sizeof(CEDEVICE_POWER_STATE);

                dwResult = ERROR_SUCCESS;
            }
            break;

        case IOCTL_POWER_SET:
            DEBUGMSG(ZONE_FUNCTION, (L"TchPdd_Ioctl: IOCTL_POWER_SET\r\n"));
            if(pBufOut != NULL && dwLenOut == sizeof(CEDEVICE_POWER_STATE))
            {
                CEDEVICE_POWER_STATE dx = *(CEDEVICE_POWER_STATE*)pBufOut;
                if( VALID_DX(dx) )
                {
                    DEBUGMSG(ZONE_FUNCTION, (L"TchPdd_Ioctl: IOCTL_POWER_SET = to D%u\r\n", dx));
                    s_TouchDevice.dwPowerState = dx;

                    if (pdwActualOut)
                        *pdwActualOut = sizeof(CEDEVICE_POWER_STATE);

                    //  Enable touchscreen for D0-D2; otherwise disable
                    switch( dx )
                    {
                        case D0:
                        case D1:
                        case D2:
                            //  Enable touchscreen
                            TouchPanelPowerHandler(FALSE);    //Enable Touch ADC
                            break;

                        case D3:
                        case D4:
                            //  Disable touchscreen
                            TouchPanelPowerHandler(TRUE); //Disable Touch ADC
                            break;

                        default:
                            //  Ignore
                            break;
                    }

                    dwResult = ERROR_SUCCESS;
                }
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
// PddTouchIST()
//  Description: IST routine for the Touch.
//
// Arguments:   None
//
// Ret Value:   None
//

ULONG
PddTouchIST(
    PVOID   reserved  // not used.
    )
{
    TOUCH_PANEL_SAMPLE_FLAGS    SampleFlags = 0;
    UINT32                      CurrentDown = 0;
    UINT32  samplingTimeOut = INFINITE;
    BOOL    bDown = FALSE;
    USHORT  x, y;
    USHORT  usFilteredX;
    USHORT  usFilteredY;
    CETOUCHINPUT input;


    DEBUGMSG(ZONE_TOUCH_IST, (L"PddTouchIST: IST thread started\r\n"));


    //  Loop until told to stop
    while(!s_TouchDevice.bTerminateIST)
    {
        //  Wait for touch IRQ, timeout or terminate flag
        DWORD dwRet = WaitForSingleObject(s_TouchDevice.hTouchEvent, samplingTimeOut);

        if(dwRet == WAIT_TIMEOUT)
        {
            DEBUGMSG(ZONE_TOUCH_IST, (L"PddTouchIST: Wait Timed out \r\n"));
        }else
        {
            DEBUGMSG(ZONE_TOUCH_IST, (L"PddTouchIST: Wait Signalled \r\n"));
        }

        //  Check for terminate flag
        if (s_TouchDevice.bTerminateIST)
            break;

        SampleFlags = BSPTouchGetSample(&x, &y);
        if ( SampleFlags & TouchSampleIgnore )
        {
            // do nothing, not a valid sample
            samplingTimeOut = 1000 / (2*s_TouchDevice.nSampleRate);
            continue;
        }

        if ( SampleFlags & TouchSampleValidFlag )
        {
            CurrentDown = SampleFlags & TouchSampleDownFlag;
            if (CurrentDown)
            {
                bDown = TRUE;
                usFilteredX = x;
                usFilteredY = y;

                //  Report the Down sample
                //convert x-y coordination to one sample set before sending it to touch MDD.
                if(ConvertTouchPanelToTouchInput((TouchSampleDownFlag|TouchSampleValidFlag), usFilteredX, usFilteredY, &input))
                {
                    //send this 1 sample to mdd
                    m_pfnMddReportSampleSet(m_mddContext, 1, &input); 
                }
               
                DEBUGMSG(ZONE_TOUCH_SAMPLES, ( TEXT( "Down: (%d, %d)\r\n" ), usFilteredX, usFilteredY ) );

                //  Change timeout to poll pen down samples
                samplingTimeOut = 1000 / (2*s_TouchDevice.nSampleRate);

            }else
            {
                //prevent multiple up events
                if( bDown )
                {
                    bDown = FALSE;
                    //  Change sampling rate to wait for next pen down interrupt
                    samplingTimeOut = INFINITE;

                    //  Report the Up sample
                    //convert x-y coordination to one sample set before sending it to touch MDD.
                    if(ConvertTouchPanelToTouchInput(TouchSampleValidFlag, usFilteredX, usFilteredY, &input))
                    {
                        //send this 1 sample to mdd
                        m_pfnMddReportSampleSet(m_mddContext, 1, &input); 
                    }

                    DEBUGMSG(ZONE_TOUCH_SAMPLES, ( TEXT( "Up:   (%d, %d)\r\n" ), usFilteredX, usFilteredY ) );
                }
            }
        }
    }

    CloseHandle(s_TouchDevice.hTouchEvent);
    s_TouchDevice.hTouchEvent = NULL;

    DEBUGMSG(ZONE_TOUCH_IST, (L"PddTouchIST: IST thread ending\r\n"));

    return 1;
}


//==============================================================================
// PddEnableTouchIST()
//  Description: Enable/Disable the touch Interrupt
//
// Arguments:   Boolean indicating enable/disable
//
// Ret Value:   Success/Failure
//

BOOL
PddEnableTouchIST( BOOL bEnable )
{
    BOOL    bStatus = FALSE;

    //  Control touch interrupt service thread
    if( bEnable )
    {
        //  Check if already running
        if(s_TouchDevice.hTouchEvent == NULL)
        {
            //  Clear terminate flag
            s_TouchDevice.bTerminateIST = FALSE;

            //  Create touch event
            s_TouchDevice.hTouchEvent = CreateEvent( NULL, FALSE, FALSE, gTouchEventName );
            if( !s_TouchDevice.hTouchEvent )
            {
                DEBUGMSG(
                    ZONE_ERROR,
                    (TEXT("PddEnableTouchIST: Failed to initialize touch event.\r\n"))
                    );
                goto cleanUp;
            }

            BSPTouchInterruptEnable(gTouchEventName);

            //  Create IST thread
            s_TouchDevice.hTouchIST= CreateThread( NULL, 0, PddTouchIST, 0, 0, NULL );
            if(s_TouchDevice.hTouchIST == NULL)
            {
                DEBUGMSG(
                    ZONE_ERROR,
                    (TEXT("PddEnableTouchIST: Failed to create IST thread\r\n"))
                    );
                goto cleanUp;
            }

            // set IST thread priority
            CeSetThreadPriority (s_TouchDevice.hTouchIST, s_TouchDevice.nPenPriority256);
        }

        //  Running
        bStatus = TRUE;
    }
    else
    {
        //  Disable touch interrupt service thread
        if( s_TouchDevice.hTouchEvent )
        {
            BSPTouchInterruptDisable();

            s_TouchDevice.bTerminateIST = TRUE;
            SetEvent( s_TouchDevice.hTouchEvent );
            WaitForSingleObject(s_TouchDevice.hTouchIST, INFINITE);
            CloseHandle(s_TouchDevice.hTouchIST);
            s_TouchDevice.hTouchIST=NULL;
        }
        //  Stopped
        bStatus = TRUE;
    }


cleanUp:
    //  Return status
    return bStatus;
}





//-----------------------------------------------------------------------------
//
// Function: TouchPanelEnable
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
BOOL TouchPanelEnable(void)
{
    BOOL rc = TRUE;

    DEBUGMSG(ZONE_FUNCTION, (_T("TouchPanelEnable+\r\n")));

    // if we are already powered on, just return
    if (gTouchPowerStatus == TouchPowerOn)
    {
        goto cleanUp;
    }

    // Initialize BSP-specific settings
    if (!BSPTouchInit())
    {
        DEBUGMSG(ZONE_ERROR,
            (_T("TouchPanelEnable:  BSPTouchInit failed!\r\n")));
        rc = FALSE;
        goto cleanUp;
    }

    //Enable the IST
    PddEnableTouchIST( TRUE );

cleanUp:
    if (!rc) BSPTouchDeinit();

    DEBUGMSG(ZONE_FUNCTION, (_T("TouchPanelEnable-\r\n")));

    return rc;
}


//-----------------------------------------------------------------------------
//
// Function: TouchPanelDisable
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
void TouchPanelDisable(void)
{
    DEBUGMSG(ZONE_FUNCTION, (_T("TouchPanelDisable+\r\n")));

    // If we are already off, just return
    if (gTouchPowerStatus == TouchPowerOff)
        return;

    //Disable the IST
    PddEnableTouchIST( FALSE );

    // Deinitialize BSP-specific settings
    BSPTouchDeinit();

    DEBUGMSG(ZONE_FUNCTION, (_T("TouchPanelDisable-\r\n")));
    return;
}


//-----------------------------------------------------------------------------
//
// Function: TouchPanelPowerHandler
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
void TouchPanelPowerHandler(BOOL boff)
{
    DEBUGMSG(ZONE_FUNCTION, (_T("TouchPanelPowerHandler+\r\n")));

    //Make sure that there is a toggle
    if( (boff == TRUE)&&(gTouchPowerStatus != TouchPowerOn) ||
        (boff == FALSE)&&(gTouchPowerStatus != TouchPowerOff) )
        return;


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

    DEBUGMSG(ZONE_FUNCTION, (_T("TouchPanelPowerHandler-\r\n")));

    return;
}
