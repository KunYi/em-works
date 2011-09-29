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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
// Portions Copyright (c) Texas Instruments.  All rights reserved.
//
//------------------------------------------------------------------------------
//
#include <wavemain.h>

#if defined( BSP_SMARTPHONE) || defined( BSP_POCKETPC )
#include <audiosys.h>
#include <wfmtmidi.h>
#include <DevNotify.h>            
#include <ril.h>
#endif

#include "xhwctxt.h"

// 
HANDLE g_hKeyPressEvent;      // pm key press event

//------------------------------------------------------------------------------
//
//  Function: PmControlMessage
//
//  Power management routine.
//
/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8

BOOL
ACAudioHWContext::PmControlMessage (
                                                DWORD  dwCode,
                        PBYTE  pBufIn,
                        DWORD  dwLenIn,
                        PBYTE  pBufOut,
                        DWORD  dwLenOut,
                        PDWORD pdwActualOut)
{
    BOOL bRetVal = FALSE;

    switch (dwCode)
    {
    // Return device specific power capabilities.
    case IOCTL_POWER_CAPABILITIES:
    {
        POWER_CAPABILITIES pc;

        // Check arguments.
        if ( pBufOut == NULL || dwLenOut < sizeof(POWER_CAPABILITIES))
        {
            DEBUGMSG(ZONE_ERROR, (L"ACAudioHWContext::PmControlMessage: "
                L"Invalid parameter.\r\n"
            ));
            break;
        }

        // Clear capabilities structure.
        memset(&pc, 0, sizeof(POWER_CAPABILITIES));

        // Set power capabilities. Supports D0 and D4.
        pc.DeviceDx = DX_MASK(D0)|DX_MASK(D4);

        DEBUGMSG(ZONE_POWER, (L"ACAudioHWContext::PmControlMessage: "
            L"IOCTL_POWER_CAPABILITIES = 0x%x\r\n", pc.DeviceDx
        ));

        if (CeSafeCopyMemory(pBufOut, &pc, sizeof(pc)) == 0) break;

        // Update returned data size.
        if (pdwActualOut) *pdwActualOut = sizeof(pc);
        bRetVal = TRUE;
        break;
    }

    // Indicate if the device is ready to enter a new device power state.
    case IOCTL_POWER_QUERY:
    {
        // Check arguments.
        if (pBufOut == NULL || dwLenOut < sizeof(CEDEVICE_POWER_STATE))
        {
            DEBUGMSG(ZONE_ERROR, (L"ACAudioHWContext::PmControlMessage: "
                L"Invalid parameter.\r\n"
            ));
            break;
        }

        if (CeSafeCopyMemory( pBufOut, &m_ExternPowerStateRequired, sizeof(CEDEVICE_POWER_STATE)) == 0)
            break;

        DEBUGMSG(ZONE_POWER, (L"ACAudioHWContext::PmControlMessage: "
            L"IOCTL_POWER_QUERY = %d\r\n", m_ExternPowerStateRequired
        ));

        if (!VALID_DX(m_ExternPowerStateRequired))
        {
            DEBUGMSG(ZONE_ERROR, (L"ACAudioHWContext::PmControlMessage: "
                L"IOCTL_POWER_QUERY invalid power state.\r\n"));
            break;
        }

        if (pdwActualOut) *pdwActualOut = sizeof(CEDEVICE_POWER_STATE);
        bRetVal = TRUE;
        break;
    }

    // Request a change from one device power state to another.
    case IOCTL_POWER_SET:
    {
        CEDEVICE_POWER_STATE dxState;

        // Check arguments.
        if (pBufOut == NULL || dwLenOut < sizeof(CEDEVICE_POWER_STATE))
        {
            DEBUGMSG(ZONE_ERROR, (L"ACAudioHWContext::PmControlMessage: "
                L"Invalid parameter.\r\n"
            ));
            break;
        }

        if (CeSafeCopyMemory(&dxState, pBufOut, sizeof(dxState)) == 0) break;

        DEBUGMSG(ZONE_POWER, (L"ACAudioHWContext::PmControlMessage: "
            L"IOCTL_POWER_SET = %d.\r\n", dxState
        ));

        // Check for any valid power state.
        if (VALID_DX(dxState))
        {
            Lock();
            m_ExternPowerStateRequired = dxState ;
            // Power off.
            if ( dxState == D4 )
            {
                HWAudioPowerTimeout (TRUE);
            }
            // Power on.
            else
            {
                HWAudioPowerTimeout (FALSE);
            }

            Unlock();

            bRetVal = TRUE;
        }
        else
        {
            DEBUGMSG(ZONE_ERROR, (L"ACAudioHWContext::PmControlMessage: "
                L"IOCTL_POWER_SET invalid power state.\r\n"
            ));
        }
        break;
    }

    // Return the current device power state.
    case IOCTL_POWER_GET:
    {
        DEBUGMSG(ZONE_POWER, (L"ACAudioHWContext::PmControlMessage: "
            L"IOCTL_POWER_GET -- not supported!\r\n"
        ));
        break;
    }

    default:
        DEBUGMSG(ZONE_ERROR, (L"ACAudioHWContext::PmControlMessage: "
            L"Unknown IOCTL_xxx(0x%0.8X) \r\n", dwCode
        ));
        break;
    }

    return bRetVal;
}

//------------------------------------------------------------------------------
//
//  Function: RegisterRilCallback
//
//   This function register RIL callbacks.
//


BOOL
ACAudioHWContext::RegisterRilCallback (HANDLE *phRil)
{
    // Try to initialize RIL
    DEBUGMSG(ZONE_FUNCTION, (L">>>>> Calling RIL_Initialize()\r\n"));
    *phRil = NULL;

#if defined( BSP_SMARTPHONE) || defined( BSP_POCKETPC )
    HRESULT hr;

    // RIL port 1
    hr = RIL_Initialize(1, ResultCallback, NotifyCallback,
                    RIL_NCLASS_CALLCTRL, NULL, phRil);

    if (FAILED(hr)) {
        DEBUGMSG(ZONE_ERROR, (L"RIL_Initialize call failed, code = %x", hr));
        return FALSE;
    }
#endif

    // Create key press event;
    g_hKeyPressEvent = CreateEvent( NULL, FALSE, TRUE, TEXT( "KeyPressEvent" ) );

    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function: UnregisterRilCallback
//
 //  This function un-register RIL callbacks.
//

BOOL
ACAudioHWContext::UnregisterRilCallback (HANDLE hRil)
{
    // Try to initialize RIL
    DEBUGMSG(ZONE_FUNCTION, (L">>>>> Calling RIL_Deinitialize()\r\n"));

#if defined( BSP_SMARTPHONE) || defined( BSP_POCKETPC )
    HRESULT hr;

    if (hRil) {
        hr = RIL_Deinitialize(hRil);

        if (FAILED(hr)) {
            DEBUGMSG(ZONE_ERROR, (L"RIL_Deinitialize call failed, code = %x", hr));
            return FALSE;
        }
    }
#endif

    return TRUE;
}

#if defined( BSP_SMARTPHONE) || defined( BSP_POCKETPC )

//------------------------------------------------------------------------------
//
//  Function: NotifyCallback
//
//  This function is RIL callback notification function.
//

void CALLBACK
NotifyCallback(DWORD dwCode, const void* lpData, DWORD cbData, DWORD dwParam)
{
    RILCONNECTINFO *prilConnInfo;

    switch (dwCode)
    {
        case RIL_NOTIFY_RING:
#if 0
            if (sizeof(RILRINGINFO) == cbData) {
                DWORD dwRingType;

                RILRINGINFO* prri = (RILRINGINFO*)lpData;
                dwRingType = prri->dwCallType;
                if (prri->dwParams & RIL_PARAM_RI_SERVICEINFO) {
                } else {
                    RETAILMSG (PM_TRACE,
                        (TEXT("Got RIL_NOTIFY_RING, call type = %d\r\n"),
                        dwRingType));
                }
            } else {
                RETAILMSG (PM_TRACE, (TEXT("Got RIL_NOTIFY_RING\r\n")));
            }
#endif

            DEBUGMSG(ZONE_FUNCTION, (L"Got RIL_NOTIFY_RING\r\n"));

            // In the case where we are waking from the Suspend State during
            // a phone call we need to set the state to ON to prevent the phone
            // from returning to suspend in 15 seconds.
            // set power state from "Resuming" to "On"
            SetSystemPowerState(NULL, POWER_STATE_ON, 0);

            // Set keylight event here in order to activate the display.
            if (g_hKeyPressEvent)
            {
                SetEvent (g_hKeyPressEvent);
            }

#if 0       // The following function does not work somehow.
            // Set power state as normal.
            DWORD dwRet;
            dwRet = SetSystemPowerState(NULL, POWER_STATE_ON, POWER_FORCE);
            if (dwRet == ERROR_SUCCESS )
            {
                RETAILMSG (PM_TRACE, (TEXT("SetSystemPowerState ok = 0x%x\r\n"), dwRet));
            }
            else
            {
                RETAILMSG (PM_TRACE, (TEXT("SetSystemPowerState error = 0x%x\r\n"), dwRet));
            }
#endif
            break;

        case RIL_NOTIFY_DIAL:
        case RIL_NOTIFY_DIALEDID:
            DEBUGMSG(ZONE_FUNCTION, (L"Got RIL_NOTIFY_DIALEDID\r\n"));

            // We are in a call.
            g_pHWContext->PrepareForVoiceCall (TRUE);
            break;

        case RIL_NOTIFY_CONNECT:
            DEBUGMSG(ZONE_FUNCTION, (L"Got RIL_NOTIFY_CONNECT\r\n"));

            // We only consider voice calls only.
            prilConnInfo = (RILCONNECTINFO*)lpData;
            if (prilConnInfo->dwCallType != RIL_CALLTYPE_VOICE)
            {
                DEBUGMSG(ZONE_FUNCTION, (L"not VOICE...\r\n"));
                break;
            }

            // We are in a call.
            g_pHWContext->PrepareForVoiceCall (TRUE);
            break;

        case RIL_NOTIFY_DISCONNECT:
            DEBUGMSG(ZONE_FUNCTION, (L"Got RIL_NOTIFY_DISCONNECT\r\n"));

            // The call is disconnected.
            g_pHWContext->PrepareForVoiceCall (FALSE);
            break;
    }
}

//------------------------------------------------------------------------------
//
//  Function: ResultCallback
//
//  This function is RIL result notification function.
//

void CALLBACK
ResultCallback(DWORD dwCode, HRESULT hrCmdID, const void* lpData,
                             DWORD cbData, DWORD dwParam)
{
    switch (dwCode)
    {
        case RIL_RESULT_OK:
            DEBUGMSG(ZONE_FUNCTION, (L"Result ok = 0x%x\r\n", dwCode));
            break;
    }
}

#endif
