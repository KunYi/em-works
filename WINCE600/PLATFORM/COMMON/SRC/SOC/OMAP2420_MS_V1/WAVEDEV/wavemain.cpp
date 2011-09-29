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

#include "wavemain.h"

//------------------------------------------------------------------------------
//
//
//

#ifdef DEBUG
DBGPARAM dpCurSettings =
{
    TEXT("WaveDriver"), {
    TEXT("EAC"),TEXT("Params"),TEXT("Verbose"),TEXT("Irq"),
    TEXT("WODM"),TEXT("WIDM"),TEXT("PDD"),TEXT("MDD"),
    TEXT("DMA"),TEXT("Misc"),TEXT("Midi"),TEXT("Modem"),
    TEXT("Alloc"),TEXT("Function"),TEXT("Warning"),TEXT("Error") },
    0xC000
};
#endif


//------------------------------------------------------------------------------
//
//  defined in bt_ddi.h
//

// BT audio routing define
#if !defined(WODM_BT_SCO_AUDIO_CONTROL)
#define WODM_BT_SCO_AUDIO_CONTROL   500
#endif


//------------------------------------------------------------------------------
//
//  Function: DllMain
//
//

BOOL CALLBACK DllMain(HANDLE hDLL,
                      DWORD dwReason,
                      LPVOID lpvReserved)
{
    if ( dwReason==DLL_PROCESS_ATTACH )
    {
        DEBUGREGISTER((HMODULE)hDLL);
    }

    return TRUE;
}


// -----------------------------------------------------------------------------
//
// @doc     WDEV_EXT
//
// @topic   WAV Device Interface | Implements the WAVEDEV.DLL device
//          interface. These functions are required for the device to
//          be loaded by DEVICE.EXE.
//
// @xref                          <nl>
//          <f WAV_Init>,         <nl>
//          <f WAV_Deinit>,       <nl>
//          <f WAV_Open>,         <nl>
//          <f WAV_Close>,        <nl>
//          <f WAV_IOControl>     <nl>
//
// -----------------------------------------------------------------------------
//
// @doc     WDEV_EXT
//
// @topic   Designing a Waveform Audio Driver |
//          A waveform audio driver is responsible for processing messages
//          from the Wave API Manager (WAVEAPI.DLL) to playback and record
//          waveform audio. Waveform audio drivers are implemented as
//          dynamic link libraries that are loaded by DEVICE.EXE The
//          default waveform audio driver is named WAVEDEV.DLL (see figure).
//          The messages passed to the audio driver are similar to those
//          passed to a user-mode Windows NT audio driver (such as mmdrv.dll).
//
//          <bmp blk1_bmp>
//
//          Like all device drivers loaded by DEVICE.EXE, the waveform
//          audio driver must export the standard device functions,
//          XXX_Init, XXX_Deinit, XXX_IoControl, etc (see
//          <t WAV Device Interface>). The Waveform Audio Drivers
//          have a device prefix of "WAV".
//
//          Driver loading and unloading is handled by DEVICE.EXE and
//          WAVEAPI.DLL. Calls are made to <f WAV_Init> and <f WAV_Deinit>.
//          When the driver is opened by WAVEAPI.DLL calls are made to
//          <f WAV_Open> and <f WAV_Close>. All
//          other communication between WAVEAPI.DLL and WAVEDEV.DLL is
//          done by calls to <f WAV_IOControl>. The other WAV_xxx functions
//          are not used.
//
// @xref                                          <nl>
//          <t Designing a Waveform Audio PDD>    <nl>
//          <t WAV Device Interface>              <nl>
//          <t Wave Input Driver Messages>        <nl>
//          <t Wave Output Driver Messages>       <nl>
//
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @func   PVOID | WAV_Init | Device initialization routine
//
//  @parm   DWORD | dwInfo | info passed to RegisterDevice
//
//  @rdesc  Returns a DWORD which will be passed to Open & Deinit or NULL if
//          unable to initialize the device.
//
// -----------------------------------------------------------------------------
extern "C" 
DWORD WAV_Init(DWORD Index)
{
    return (DWORD)HardwareContext::CreateHWContext(Index);
}

// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @func   PVOID | WAV_Deinit | Device deinitialization routine
//
//  @parm   DWORD | dwData | value returned from WAV_Init call
//
//  @rdesc  Returns TRUE for success, FALSE for failure.
//
// -----------------------------------------------------------------------------
extern "C" BOOL 
WAV_Deinit(DWORD dwData)
{
    ASSERT(g_pHWContext!=NULL);
    return g_pHWContext->Deinit();
}

// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @func   PVOID | WAV_Open    | Device open routine
//
//  @parm   DWORD | dwData      | Value returned from WAV_Init call (ignored)
//
//  @parm   DWORD | dwAccess    | Requested access (combination of GENERIC_READ
//                                and GENERIC_WRITE) (ignored)
//
//  @parm   DWORD | dwShareMode | Requested share mode (combination of
//                                FILE_SHARE_READ and FILE_SHARE_WRITE) (ignored)
//
//  @rdesc  Returns a DWORD which will be passed to Read, Write, etc or NULL if
//          unable to open device.
//
// -----------------------------------------------------------------------------
extern "C" PDWORD 
WAV_Open( DWORD dwData,
          DWORD dwAccess,
          DWORD dwShareMode)
{
    // allocate and return handle context to efficiently verify caller trust level
    return new DWORD(NULL); // assume untrusted. Can't tell for sure until WAV_IoControl
}

// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @func   BOOL | WAV_Close | Device close routine
//
//  @parm   DWORD | dwOpenData | Value returned from WAV_Open call
//
//  @rdesc  Returns TRUE for success, FALSE for failure
//
// -----------------------------------------------------------------------------
extern "C" BOOL 
WAV_Close(PDWORD pdwData)
{
    // we trust the device manager to give us a valid context to free.
    delete pdwData;
    return(TRUE);
}



BOOL HandleWaveMessage(PMMDRV_MESSAGE_PARAMS pParams, DWORD *pdwResult)
{
    //  set the error code to be no error first
    SetLastError(MMSYSERR_NOERROR);

    UINT uMsg = pParams->uMsg;
    UINT uDeviceId = pParams->uDeviceId;
    DWORD dwParam1 = pParams->dwParam1;
    DWORD dwParam2 = pParams->dwParam2;
    DWORD dwUser   = pParams->dwUser;
    StreamContext *pStreamContext = (StreamContext *)dwUser;
    DWORD dwRet=MMSYSERR_NOTSUPPORTED;

    g_pHWContext->Lock();

    // catch exceptions inside device lock, otherwise device will remain locked!
    _try
    {

    switch (uMsg)
    {
    case WODM_GETNUMDEVS:
        {
            dwRet = g_pHWContext->GetNumOutputDevices();
            break;
        }

    case WIDM_GETNUMDEVS:
        {
            dwRet = g_pHWContext->GetNumInputDevices();
            break;
        }

    case WODM_GETDEVCAPS:
        {
            DeviceContext *pDeviceContext;
            UINT NumDevs = g_pHWContext->GetNumOutputDevices();

            if (pStreamContext)
            {
                pDeviceContext=pStreamContext->GetDeviceContext();
            }
            else
            {
                pDeviceContext = g_pHWContext->GetOutputDeviceContext(uDeviceId);
            }

            dwRet = pDeviceContext->GetDevCaps((PVOID)dwParam1,dwParam2);
            break;
        }


    case WIDM_GETDEVCAPS:
        {
            DeviceContext *pDeviceContext;
            UINT NumDevs = g_pHWContext->GetNumInputDevices();

            if (pStreamContext)
            {
                pDeviceContext=pStreamContext->GetDeviceContext();
            }
            else
            {
                pDeviceContext = g_pHWContext->GetInputDeviceContext(uDeviceId);
            }

            dwRet = pDeviceContext->GetDevCaps((PVOID)dwParam1,dwParam2);
            break;
        }

    case WODM_OPEN:
        {
            DEBUGMSG(ZONE_WODM, (TEXT("WODM_OPEN\r\n")));
            DeviceContext *pDeviceContext = g_pHWContext->GetOutputDeviceContext(uDeviceId);
            dwRet = pDeviceContext->OpenStream((LPWAVEOPENDESC)dwParam1, dwParam2, (StreamContext **)dwUser);
            break;
        }

    case WODM_GETEXTDEVCAPS:
        {
            DeviceContext *pDeviceContext;
            UINT NumDevs = g_pHWContext->GetNumOutputDevices();

            if (pStreamContext)
            {
                pDeviceContext=pStreamContext->GetDeviceContext();
            }
            else
            {
                pDeviceContext = g_pHWContext->GetOutputDeviceContext(uDeviceId);
            }

            dwRet = pDeviceContext->GetExtDevCaps((PVOID)dwParam1,dwParam2);
            break;
        }

    case WIDM_OPEN:
        {
            DEBUGMSG(ZONE_WIDM, (TEXT("WIDM_OPEN\r\n")));
            DeviceContext *pDeviceContext = g_pHWContext->GetInputDeviceContext(uDeviceId);
            dwRet = pDeviceContext->OpenStream((LPWAVEOPENDESC)dwParam1, dwParam2, (StreamContext **)dwUser);
            break;
        }

    case WODM_CLOSE:
    case WIDM_CLOSE:
        {
            DEBUGMSG(ZONE_WODM|ZONE_WIDM, (TEXT("WIDM_CLOSE/WODM_CLOSE\r\n")));
            dwRet = pStreamContext->Close();

            // Release stream context here, rather than inside StreamContext::Close, so that if someone
            // (like CMidiStream) has subclassed Close there's no chance that the object will get released
            // out from under them.
            if (dwRet==MMSYSERR_NOERROR)
            {
                pStreamContext->Release();
            }
            break;
        }

    case WODM_RESTART:
    case WIDM_START:
        {
            DEBUGMSG(ZONE_WODM|ZONE_WIDM, (TEXT("WODM_RESTART/WIDM_START\r\n")));
            dwRet = pStreamContext->Run();
            break;
        }

    case WODM_PAUSE:
    case WIDM_STOP:
        {
            DEBUGMSG(ZONE_WODM|ZONE_WIDM, (TEXT("WODM_PAUSE/WIDM_STOP\r\n")));
            dwRet = pStreamContext->Stop();
            break;
        }

    case WODM_GETPOS:
    case WIDM_GETPOS:
        {
            DEBUGMSG(ZONE_WODM|ZONE_WIDM, (TEXT("WODM_GETPOS/WIDM_GETPOS\r\n")));
            dwRet = pStreamContext->GetPos((PMMTIME)dwParam1);
            break;
        }

    case WODM_RESET:
    case WIDM_RESET:
        {
            DEBUGMSG(ZONE_WODM|ZONE_WIDM, (TEXT("WODM_RESET/WIDM_RESET\r\n")));
            dwRet = pStreamContext->Reset();
            break;
        }

    case WODM_WRITE:
    case WIDM_ADDBUFFER:
        {
            DEBUGMSG(ZONE_WODM|ZONE_WIDM, (TEXT("WODM_WRITE/WIDM_ADDBUFFER, Buffer=0x%x\r\n"),dwParam1));
            dwRet = pStreamContext->QueueBuffer((LPWAVEHDR)dwParam1);
            break;
        }

    case WODM_GETVOLUME:
        {
            PDWORD pdwGain = (PDWORD)dwParam1;
            UINT NumDevs = g_pHWContext->GetNumOutputDevices();

            if (pStreamContext)
            {
                *pdwGain = pStreamContext->GetGain();
            }
            else
            {
                DeviceContext *pDeviceContext = g_pHWContext->GetOutputDeviceContext(uDeviceId);
                *pdwGain = pDeviceContext->GetGain();
            }
            dwRet = MMSYSERR_NOERROR;
            break;
        }

    case WODM_SETVOLUME:
        {
            UINT NumDevs = g_pHWContext->GetNumOutputDevices();
            LONG dwGain = dwParam1 & 0xffff;
            LONG ulRight = (dwParam1>>16);

            // Left and rights volume are the same.
            if (ulRight>dwGain)
            {
                dwGain = (ulRight<<16) + ulRight;
            }
            else
            {
                dwGain = (dwGain<<16) + dwGain;
            }

            if (pStreamContext)
            {
                dwRet = pStreamContext->SetGain(dwGain);
                DEBUGMSG(ZONE_WODM, (TEXT("WODM_SETVOLUME, stream=0x%x\r\n"),dwGain));
            }
            else
            {
                DeviceContext *pDeviceContext = g_pHWContext->GetOutputDeviceContext(uDeviceId);
                dwRet = pDeviceContext->SetGain(dwGain);
                DEBUGMSG(ZONE_WODM, (TEXT("WODM_SETVOLUME, device=0x%x\r\n"),dwGain));
            }
            break;
        }

    case WODM_BREAKLOOP:
        {
            dwRet = pStreamContext->BreakLoop();
            break;
        }

    case WODM_SETPLAYBACKRATE:
        {
            WaveStreamContext *pWaveStream = (WaveStreamContext *)dwUser;
            dwRet = pWaveStream->SetRate(dwParam1);
            break;
        }

    case WODM_GETPLAYBACKRATE:
        {
            WaveStreamContext *pWaveStream = (WaveStreamContext *)dwUser;
            dwRet = pWaveStream->GetRate((DWORD *)dwParam1);
            break;
        }

    case MM_WOM_SETSECONDARYGAINCLASS:
        {
            DEBUGMSG(ZONE_WODM, (TEXT("WAVE: MM_WOM_SETSECONDARYGAINCLASS \r\n")));

            dwRet = pStreamContext->SetSecondaryGainClass(dwParam1);
            break;
        }

    case MM_WOM_SETSECONDARYGAINLIMIT:
        {
            DeviceContext *pDeviceContext;
            DEBUGMSG(ZONE_WODM, (TEXT("WAVE: MM_WOM_SETSECONDARYGAINLIMIT \r\n")));

            if (pStreamContext)
            {
                pDeviceContext = pStreamContext->GetDeviceContext();
            }
            else
            {
                pDeviceContext = g_pHWContext->GetOutputDeviceContext(uDeviceId);
            }
            dwRet = pDeviceContext->SetSecondaryGainLimit(dwParam1,dwParam2);
            break;
        }

    case MM_WOM_FORCESPEAKER:
        {
            DEBUGMSG(ZONE_WODM, (TEXT("WAVE: MM_WOM_FORCESPEAKER \r\n")));
            if (pStreamContext)
            {
                dwRet = pStreamContext->ForceSpeaker((BOOL)dwParam1);
            }
            else
            {
                dwRet = g_pHWContext->ForceSpeaker((BOOL)dwParam1);
            }
            break;
        }

    case MM_MOM_MIDIMESSAGE:
        {
            DEBUGMSG(ZONE_WODM, (TEXT("WAVE: MM_MOM_MIDIMESSAGE \r\n")));
            CMidiStream *pMidiStream = (CMidiStream *)dwUser;
            dwRet = pMidiStream->MidiMessage(dwParam1);
            break;
        }

    case WODM_BT_SCO_AUDIO_CONTROL:
        {
            DEBUGMSG(ZONE_WODM,(TEXT("WODM_BT_SCO_AUDIO_CONTROL\r\n")));
            if (pParams->dwParam2)
            {
                g_pHWContext->NotifyBtHeadsetOn(BT_AUDIO_SYSTEM|BT_AUDIO_MODEM);
            }
            else
            {
                g_pHWContext->NotifyBtHeadsetOn(BT_AUDIO_NONE);
            }
            dwRet = MMSYSERR_NOERROR;
            break;
        }

// unsupported messages
    case WODM_GETPITCH:
    case WODM_SETPITCH:
    case WODM_PREPARE:
    case WODM_UNPREPARE:
    case WIDM_PREPARE:
    case WIDM_UNPREPARE:
    default:
        dwRet  = MMSYSERR_NOTSUPPORTED;
        break;
    }

    }
    _except (GetExceptionCode() == STATUS_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
    {
        ERRORMSG(1, (TEXT("Access violation in HandleWaveMessage!!!!\r\n")));
        SetLastError(E_FAIL);
    }

    g_pHWContext->Unlock();

    // Pass the return code back via pBufOut
    if (pdwResult)
    {
        *pdwResult = dwRet;
    }

    return TRUE;
}

// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @func   BOOL | WAV_IOControl | Device IO control routine
//
//  @parm   DWORD | dwOpenData | Value returned from WAV_Open call
//
//  @parm   DWORD | dwCode |
//          IO control code for the function to be performed. WAV_IOControl only
//          supports one IOCTL value (IOCTL_WAV_MESSAGE)
//
//  @parm   PBYTE | pBufIn |
//          Pointer to the input parameter structure (<t MMDRV_MESSAGE_PARAMS>).
//
//  @parm   DWORD | dwLenIn |
//          Size in bytes of input parameter structure (sizeof(<t MMDRV_MESSAGE_PARAMS>)).
//
//  @parm   PBYTE | pBufOut | Pointer to the return value (DWORD).
//
//  @parm   DWORD | dwLenOut | Size of the return value variable (sizeof(DWORD)).
//
//  @parm   PDWORD | pdwActualOut | Unused
//
//  @rdesc  Returns TRUE for success, FALSE for failure
//
//  @xref   <t Wave Input Driver Messages> (WIDM_XXX) <nl>
//          <t Wave Output Driver Messages> (WODM_XXX)
//
// -----------------------------------------------------------------------------
BOOL 
WAV_IOControl(     DWORD dwOpenData,
                   DWORD  dwCode,
                   PBYTE  pBufIn,
                   DWORD  dwLenIn,
                   PBYTE  pBufOut,
                   DWORD  dwLenOut,
                   PDWORD pdwActualOut)
{
    __try
    {
        switch (dwCode)
        {
        case IOCTL_WAV_MESSAGE:
            return HandleWaveMessage((PMMDRV_MESSAGE_PARAMS)pBufIn, (DWORD *)pBufOut);

        case IOCTL_SET_EXTSPEAKER_POWER:
            if(dwLenIn<sizeof(BOOL))
                return FALSE;
            g_pHWContext->SetExtSpeakerPower((BOOL)*(DWORD*)pBufIn);
            if(pdwActualOut)
                *pdwActualOut = 0;
            return TRUE;

        case IOCTL_NOTIFY_HEADSET:
            if(dwLenIn<sizeof(BOOL))
                return FALSE;
            if(pdwActualOut)
                *pdwActualOut = 0;
            g_pHWContext->NotifyHeadsetOn((BOOL)*(DWORD *)pBufIn);
            return TRUE;

        case IOCTL_NOTIFY_BT_HEADSET:
            if(dwLenIn<sizeof(DWORD))
                return FALSE;
            if(pdwActualOut)
                *pdwActualOut = 0;
            g_pHWContext->NotifyBtHeadsetOn(*(DWORD *)pBufIn);
            return TRUE;

        case IOCTL_TOGGLE_EXT_SPEAKER:
            if(pdwActualOut)
                *pdwActualOut = 0;
            g_pHWContext->ToggleExtSpeaker();
            return TRUE;

        case IOCTL_GSM_CALL_ACTIVE:
            g_pHWContext->PrepareForVoiceCall(TRUE);
            if(pdwActualOut)
                *pdwActualOut = 0;
            return TRUE;

        case IOCTL_GSM_CALL_INACTIVE:
            g_pHWContext->PrepareForVoiceCall(FALSE);
            dwLenOut = 0;
            if(pdwActualOut)
                *pdwActualOut = 0;
            return TRUE;

        // Power management functions.
        case IOCTL_POWER_CAPABILITIES:
        case IOCTL_POWER_QUERY:
        case IOCTL_POWER_SET:
        case IOCTL_POWER_GET:
            return g_pHWContext->PmControlMessage
                                (dwCode, pBufIn, dwLenIn, pBufOut, dwLenOut, pdwActualOut);
            break;

        case IOCTL_DDK_GET_DRIVER_IFC:
            {
            // We can give interface only to our peer in device process
            if (GetCurrentProcessId() != (DWORD)GetCallerProcess()) 
            {
                DEBUGMSG(ZONE_ERROR, (L"ERROR: WAV_IOControl: "
                    L"IOCTL_DDK_GET_DRIVER_IFC can be called only from "
                    L"device process (caller process id 0x%08x)\r\n", 
                    GetCallerProcess()
                ));
                SetLastError(ERROR_ACCESS_DENIED);
                break;
            }
            if (pBufIn != NULL || 
                pBufOut == NULL ||
                dwLenOut < sizeof(CEDDK_DRIVER_PFN)) 
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                break;
            }

            CEDDK_DRIVER_PFN *pFN;
            pFN = (CEDDK_DRIVER_PFN*)pBufOut;
            pFN->context = dwOpenData;
            pFN->pfnIOControl = WAV_IOControl;
            }
            return TRUE;


        default:
            return g_pHWContext->HandleExtMessage (dwCode, pBufIn, dwLenIn, pBufOut, dwLenOut, pdwActualOut);
            break;
        }
    }
    __except (GetExceptionCode() == STATUS_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
    {
        ERRORMSG(1, (TEXT("EXCEPTION IN WAV_IOControl!!!!\r\n")));
        SetLastError(E_FAIL);
    }

    return FALSE;
}

