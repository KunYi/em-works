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
// Copyright (C) 2006, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//      THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//      ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//      THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//      PARTICULAR PURPOSE.
//
// ----------------------------------------------------------------------------
#include "wavemain.h"

//-----------------------------------------------------------------------------
// Global Variables

#ifdef DEBUG
DBGPARAM dpCurSettings =
{
    _T("WAV"),
    {
        _T("Test"), _T("Params"), _T("Verbose"), _T("Interrupt"),
        _T("WODM"), _T("WIDM"), _T("PDD"), _T("MDD"),
        _T("REGS"),_T("Misc"),_T("Init"),_T("IOCTL"),
        _T("Alloc"),_T("Function"),_T("Warnings"),_T("Errors")
    },
    0xC000
};
#endif

static CEDEVICE_POWER_STATE CurDx;

// -----------------------------------------------------------------------------
//
//
//  @func   Driver DLL initialisation routine.
//
//  @parm
//		HANDLE	hDLL	Hanle to DLL
//		DWORD dwReason
//		LPVOID lpvReserved
//
//  @rdesc  Returns TRUE to indicate successfull initialisation.
//
// -----------------------------------------------------------------------------
BOOL CALLBACK DllMain(HANDLE hDLL,
                      DWORD dwReason,
                      LPVOID lpvReserved)
{
    switch (dwReason) {
        case DLL_PROCESS_ATTACH :
            DEBUGREGISTER((HINSTANCE)hDLL);
            DisableThreadLibraryCalls((HMODULE) hDLL);
            break;

        case DLL_PROCESS_DETACH :
            break;

        case DLL_THREAD_DETACH :
            break;

        case DLL_THREAD_ATTACH :
            break;

        default :
            break;
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
//          <f WAV_Read>,         <nl>
//          <f WAV_Write>,        <nl>
//          <f WAV_Seek>,         <nl>
//          <f WAV_PowerUp>,      <nl>
//          <f WAV_PowerDown>,    <nl>
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
//          <f WAV_Open> and <f WAV_Close>.  On system power up and power down
//          calls are made to <f WAV_PowerUp> and <f WAV_PowerDown>. All
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
extern "C" DWORD WAV_Init(DWORD Index)
{
    return((DWORD)HardwareContext::CreateHWContext(Index));
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
extern "C" BOOL WAV_Deinit(DWORD dwData)
{
    return(g_pHWContext->Deinit());
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
extern "C" PDWORD WAV_Open( DWORD dwData,
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
extern "C" BOOL WAV_Close(PDWORD pdwData)
{
    // we trust the device manager to give us a valid context to free.
    delete pdwData;
    return(TRUE);
}

// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @func   DWORD | WAV_Read | Device read routine
//
//  @parm   DWORD | dwOpenData | Value returned from WAV_Open call (ignored)
//
//  @parm   LPVOID | pBuf | Buffer to receive data (ignored)
//
//  @parm   DWORD | len | Maximum length to read (ignored)
//
//  @rdesc  Returns 0 always. WAV_Read should never get called and does
//          nothing. Required DEVICE.EXE function, but all data communication
//          is handled by <f WAV_IOControl>.
//
// -----------------------------------------------------------------------------
extern "C" DWORD WAV_Read(DWORD dwData,
               LPVOID pBuf,
               DWORD Len)
{
    // Return length read
    return(0);
}

// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @func   DWORD | WAV_Write | Device write routine
//
//  @parm   DWORD | dwOpenData | Value returned from WAV_Open call (ignored)
//
//  @parm   LPCVOID | pBuf | Buffer containing data (ignored)
//
//  @parm   DWORD | len | Maximum length to write (ignored)
//
//  @rdesc  Returns 0 always. WAV_Write should never get called and does
//          nothing. Required DEVICE.EXE function, but all data communication
//          is handled by <f WAV_IOControl>.
//
// -----------------------------------------------------------------------------
extern "C" DWORD WAV_Write(DWORD dwData,
                LPCVOID pBuf,
                DWORD Len)
{
    // return number of bytes written (or -1 for error)
    return(0);
}

// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @func   DWORD | WAV_Seek | Device seek routine
//
//  @parm   DWORD | dwOpenData | Value returned from WAV_Open call (ignored)
//
//  @parm   long | pos | Position to seek to (relative to type) (ignored)
//
//  @parm   DWORD | type | FILE_BEGIN, FILE_CURRENT, or FILE_END (ignored)
//
//  @rdesc  Returns -1 always. WAV_Seek should never get called and does
//          nothing. Required DEVICE.EXE function, but all data communication
//          is handled by <f WAV_IOControl>.
//
// -----------------------------------------------------------------------------
extern "C" DWORD WAV_Seek(DWORD dwData,
               long pos,
               DWORD type)
{
    // return an error
    return((DWORD)-1);
}

// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @func   void | WAV_PowerUp | Device powerup routine
//
//  @comm   Called to restore device from suspend mode.  Cannot call any
//          routines aside from those in the dll in this call.
//
// -----------------------------------------------------------------------------
extern "C" VOID WAV_PowerUp(VOID)
{
 //   g_pHWContext->PowerUp();
    return;
}

// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @func   void | WAV_PowerDown | Device powerdown routine
//
//  @comm   Called to suspend device.  Cannot call any routines aside from
//          those in the dll in this call.
//
// -----------------------------------------------------------------------------
extern "C" VOID WAV_PowerDown(VOID)
{
//    g_pHWContext->PowerDown();
    return;
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

    DWORD dwRet;

    g_pHWContext->Lock();
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

    case WODM_GETEXTDEVCAPS:
        {
            DeviceContext *pDeviceContext;

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

    case WODM_OPEN:
        {
            // DEBUGMSG(1, (TEXT("WODM_OPEN\r\n"));
            DeviceContext *pDeviceContext = g_pHWContext->GetOutputDeviceContext(uDeviceId);
            dwRet = pDeviceContext->OpenStream((LPWAVEOPENDESC)dwParam1, dwParam2, (StreamContext **)dwUser);
            break;
        }

#ifdef AUDIO_RECORDING_ENABLED

    case WIDM_OPEN:
        {
            // DEBUGMSG(1, (TEXT("WIDM_OPEN\r\n"));
            DeviceContext *pDeviceContext = g_pHWContext->GetInputDeviceContext(uDeviceId);
            dwRet = pDeviceContext->OpenStream((LPWAVEOPENDESC)dwParam1, dwParam2, (StreamContext **)dwUser);
            break;
        }

    case WIDM_CLOSE:
            //Fall through to WODM_CLOSE to complete closing the stream 
#endif // #ifdef AUDIO_RECORDING_ENABLED

    case WODM_CLOSE:
        {
            // Close the audio I/O stream here (for both input and output
            // streams).
            //
            // The required StopOutputDMA()/StopInputDMA()calls are
            // made in the interrupt handler when we finis transmitting
            // the last pieces of the audio streams.
            dwRet = pStreamContext->Close();

            // Release stream context here, rather than inside
            // StreamContext::Close, so that if someone (like CMidiStream)
            // has subclassed Close there's no chance that the object will
            // get released out from under them.
            if (dwRet==MMSYSERR_NOERROR)
            {
                pStreamContext->Release();
            }
            break;
        }

    case WODM_RESTART:
    case WIDM_START:
        {
            dwRet = pStreamContext->Run();
            break;
        }

    case WODM_PAUSE:
    case WIDM_STOP:
        {
            dwRet = pStreamContext->Stop();
            break;
        }

    case WODM_GETPOS:
    case WIDM_GETPOS:
        {
            dwRet = pStreamContext->GetPos((PMMTIME)dwParam1);
            break;
        }

    case WODM_RESET:
    case WIDM_RESET:
        {
            dwRet = pStreamContext->Reset();
            break;
        }

    case WODM_WRITE:
    case WIDM_ADDBUFFER:
        {
            //DEBUGMSG(1, (TEXT("WODM_WRITE/WIDM_ADDBUFFER, Buffer=0x%x\r\n"),dwParam1);
            dwRet = pStreamContext->QueueBuffer((LPWAVEHDR)dwParam1);
            break;
        }

    case WODM_GETVOLUME:
        {
            PULONG pdwGain = (PULONG)dwParam1;

            if (pStreamContext)
            {
                *pdwGain = pStreamContext->GetGain();
            }
            else
            {
                // Handle device gain in hardware
                *pdwGain = g_pHWContext->GetOutputGain();
                // Handle device gain in software
                //DeviceContext *pDeviceContext = g_pHWContext->GetOutputDeviceContext(uDeviceId);
                //*pdwGain = pDeviceContext->GetGain();
            }
            dwRet = MMSYSERR_NOERROR;
            break;
        }

    case WODM_SETVOLUME:
        {
            LONG dwGain = dwParam1;
            if (pStreamContext)
            {
                dwRet = pStreamContext->SetGain(dwGain);
            }
            else
            {
                // Handle device gain in hardware
                dwRet = g_pHWContext->SetOutputGain(dwGain);
                // Handle device gain in software
                //DeviceContext *pDeviceContext = g_pHWContext->GetOutputDeviceContext(uDeviceId);
                //dwRet = pDeviceContext->SetGain(dwGain);
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
            dwRet = pStreamContext->SetSecondaryGainClass(dwParam1);
            break;
        }

    case MM_WOM_SETSECONDARYGAINLIMIT:
        {
            DeviceContext *pDeviceContext;
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

    case MM_MOM_MIDIMESSAGE:
        {
            CMidiStream *pMidiStream = (CMidiStream *)dwUser;
            dwRet = pMidiStream->MidiMessage(dwParam1);
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
    }
    g_pHWContext->Unlock();

    // Pass the return code back via pBufOut
    if (pdwResult)
    {
        *pdwResult = dwRet;
    }

    return(TRUE);
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
extern "C" BOOL WAV_IOControl(PDWORD  pdwOpenData,
                   DWORD  dwCode,
                   PBYTE  pBufIn,
                   DWORD  dwLenIn,
                   PBYTE  pBufOut,
                   DWORD  dwLenOut,
                   PDWORD pdwActualOut)
{

	  DWORD  dwErr = ERROR_INVALID_PARAMETER;

    // Check caller trust. If context hasn't been initialized, load from
    // CeGetCallerTrust.
    if (*pdwOpenData != OEM_CERTIFY_TRUST) {
        /*if (OEM_CERTIFY_TRUST != (*pdwOpenData = CeGetCallerTrust())) {
            PRINTMSG(ZONE_WARN, (TEXT("WAV_IoControl: untrusted process\n")));
            SetLastError(ERROR_ACCESS_DENIED);
            return FALSE;
        }*/
    }

    _try
    {
        switch (dwCode)
        {
        case IOCTL_MIX_MESSAGE:
            return HandleMixerMessage((PMMDRV_MESSAGE_PARAMS)pBufIn, (DWORD *)pBufOut);

        case IOCTL_WAV_MESSAGE:
            return HandleWaveMessage((PMMDRV_MESSAGE_PARAMS)pBufIn, (DWORD *)pBufOut);


        case IOCTL_POWER_CAPABILITIES:
            // tell the power manager about ourselves.
            if ( pBufOut != NULL &&
                 dwLenOut >= sizeof(POWER_CAPABILITIES) &&
                 pdwActualOut != NULL)
            {
                PPOWER_CAPABILITIES ppc = (PPOWER_CAPABILITIES) pBufOut;
                memset(ppc, 0, sizeof(POWER_CAPABILITIES));
                ppc->DeviceDx = 0x11;   // support D0, D4
                ppc->WakeFromDx = 0x00; // No wake capability
                ppc->InrushDx = 0x00;       // No in rush requirement
                ppc->Power[D0] = 600;                   // 0.6W
                ppc->Power[D1] = PwrDeviceUnspecified;
                ppc->Power[D2] = PwrDeviceUnspecified;
                ppc->Power[D3] = PwrDeviceUnspecified;
                ppc->Power[D4] = 0;
                ppc->Latency[D0] = 0;
                ppc->Latency[D1] = PwrDeviceUnspecified;
                ppc->Latency[D2] = PwrDeviceUnspecified;
                ppc->Latency[D3] = PwrDeviceUnspecified;
                ppc->Latency[D4] = 0;
                ppc->Flags = 0;
                *pdwActualOut = sizeof(POWER_CAPABILITIES);
                dwErr = ERROR_SUCCESS;
            }
            break;

        case IOCTL_POWER_SET:
            if ( pBufOut != NULL &&
                 dwLenOut == sizeof(CEDEVICE_POWER_STATE) &&
                 pdwActualOut != NULL)
            {
                CEDEVICE_POWER_STATE NewDx = *(PCEDEVICE_POWER_STATE)pBufOut;
                if (NewDx != CurDx)
                {
                    if (NewDx == D0)
                    {
						g_pHWContext->PowerUp();
                    }
                    else
                    {
                        // if asked for a state we don't support, go to the next lower one
                        // which is D4
                        NewDx = D4;
						g_pHWContext->PowerDown();
                    }
                    CurDx = NewDx;
                    *(PCEDEVICE_POWER_STATE)pBufOut = CurDx;
                }
                *pdwActualOut = sizeof(CEDEVICE_POWER_STATE);
                dwErr = ERROR_SUCCESS;
            }
            break;

        case IOCTL_POWER_GET:
            if ( pBufOut != NULL &&
                 dwLenOut == sizeof(CEDEVICE_POWER_STATE) &&
                 pdwActualOut != NULL)
            {
                // just return our CurrentDx value
                *(PCEDEVICE_POWER_STATE)pBufOut = CurDx;
                *pdwActualOut = sizeof(CEDEVICE_POWER_STATE);
                dwErr = ERROR_SUCCESS;
            }
            break;


        case IOCTL_POWER_QUERY:
            if ( pBufOut != NULL &&
                 dwLenOut == sizeof(CEDEVICE_POWER_STATE) &&
                 pdwActualOut != NULL)
            {
                CEDEVICE_POWER_STATE NewDx = *(PCEDEVICE_POWER_STATE)pBufOut;

                if (VALID_DX(NewDx))
                {
                    // this is a valid Dx state so return a good status
                    *pdwActualOut = sizeof(CEDEVICE_POWER_STATE);
                    dwErr = ERROR_SUCCESS;
                }
            }
            break;
    }
	}
    _except (GetExceptionCode() == STATUS_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
    {
        RETAILMSG(1, (TEXT("EXCEPTION IN WAV_IOControl!!!!\r\n")));
        SetLastError(E_FAIL);
    }

    if(dwErr == ERROR_SUCCESS)
        return TRUE;
    else
        return FALSE;
}

