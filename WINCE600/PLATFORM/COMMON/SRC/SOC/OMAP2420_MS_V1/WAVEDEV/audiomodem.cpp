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
// Wave extent for Smartphone and Pocket PC.
//
/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8

#include <wavemain.h>
#include "xhwctxt.h"
//#include "csmi_api.h" //OMAP730

//#undef DEBUGMSG
//#define DEBUGMSG(x, y) RETAILMSG(1,y)

//------------------------------------------------------------------------------
//
//  some hardcoded modem settings
//

// GSM uplink volume.
#define GSM_UPLINK_VOLUME                   0xff


//------------------------------------------------------------------------------
//
//  Function: InitModem ()
//
//

BOOL
ACAudioHWContext::InitModem ()
{

    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function: OpenModemDevice ()
//
//  Open handle to GTI driver for communication with DSP.
//

BOOL
ACAudioHWContext::OpenModemDevice ()
{
    // assume m_csModemDevice is locked
    // CAutoLock cs(m_csModemDevice);
    BOOL fRet = TRUE;
#ifdef OMAP730
    DEBUGMSG( ZONE_MODEM, (TEXT("AC: OpenModemDevice\r\n")));
    if (m_hGsmDev==NULL)
    {
        // Invoke GSM driver for future uses.
        m_hGsmDev = CreateDevice(GTI_VPORT_CTRL,
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            0, NULL);

        DEBUGMSG( ZONE_MODEM, (TEXT("AC: Open GTI_VPORT_CTRL...0x%x\r\n"), m_hGsmDev));

        if (m_hGsmDev == INVALID_HANDLE_VALUE )
        {
            m_hGsmDev = NULL;
            DEBUGMSG(ZONE_ERROR, (TEXT("WAVE: Failed to open GTI1 driver...no problem, try again later\r\n")));
            fRet = FALSE;
        }
        else
        {
            // set sidetone to defined state
            SetSideTone (TRUE);
        }
    }
#endif
    return fRet;
}

//------------------------------------------------------------------------------
//
//  Function: SetModemDevice (BOOL fOn)
//
//  Set modem   stereo mode
//              headset/speaker mode
//  based on HeadsetOn/CodecOn flasg
//

BOOL
ACAudioHWContext::SetModemDevice (BOOL fOn)
{
    CAutoLock cs(&m_csModemDevice);

    DEBUGMSG( ZONE_MODEM, (TEXT("AC: SetModemDevice\r\n")));
    // Open the modem device in case it is not opened yet.
    if (!OpenModemDevice())
    {
        return FALSE;
    }

#ifdef OMAP730
    DWORD dwRet;
    CSMI_INFO_GC_AUDIO_STEREO_REQ  stereoReq;

    while (TRUE)
    {
        // Igore the command if not needed.
        if (m_fModemCodecOn && fOn)
        {
            break;
        }

        // Igore the command if not needed.
        if (!m_fModemCodecOn && !fOn)
        {
            break;
        }

        // Save the flag.
        m_fModemCodecOn = fOn;

        if (fOn)
            stereoReq.ControlCode = GC_STEREO_ON;
        else
            stereoReq.ControlCode = GC_STEREO_OFF;

        stereoReq.Gain = 0;             // 0db
        stereoReq.SampleFrequency = GC_SAMPLE_FREQ_44100;

        if (m_fHeadsetOn)
            stereoReq.AudioMode = GC_HEADSET_MODE;
        else
            stereoReq.AudioMode = GC_SPEAKER_MODE;

        // Control sidetone
        if (!DeviceIoCtrl(  m_hGsmDev,
                            GC_AUDIO_STEREO_CODEC_REQ,
                            (LPVOID)&stereoReq,
                            sizeof(CSMI_INFO_GC_AUDIO_STEREO_REQ),
                            NULL,
                            0,
                            &dwRet,
                            NULL))
        {
            ERRORMSG(1, (TEXT("AC: GC_AUDIO_STEREO_CODEC_REQ..failed\r\n")));
        }
        else
        {
            DEBUGMSG( ZONE_MODEM,
                (TEXT("AC: GC_AUDIO_STEREO_CODEC_REQ...on=%d, headset=%d\r\n"),
                fOn, m_fHeadsetOn));
        }

        break;
    }
#endif
    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function: SetModemHeadset
//
//  set Modem headset mode.
//

BOOL
ACAudioHWContext::SetModemHeadset (BOOL fHeadset)
{
    CAutoLock cs(&m_csModemDevice);

    DEBUGMSG( ZONE_MODEM, (TEXT("AC: SetModemHeadset\r\n")));
    // Save Headset flag.
    m_fHeadsetOn = fHeadset;

    if (!OpenModemDevice())
    {
        return FALSE;
    }

#ifdef OMAP730
    DWORD dwRet;
    CSMI_INFO_GC_AUDIO_STEREO_REQ  stereoReq;

    stereoReq.ControlCode = GC_AUDIO_MODE_CHANGE;

    if (fHeadset)
        stereoReq.AudioMode = GC_HEADSET_MODE;
    else
        stereoReq.AudioMode = GC_SPEAKER_MODE;

    stereoReq.Gain = 0; // 0db
    stereoReq.SampleFrequency = GC_SAMPLE_FREQ_44100;

    // Control sidetone
    if (!DeviceIoCtrl(  m_hGsmDev,
                        GC_AUDIO_STEREO_CODEC_REQ,
                        (LPVOID)&stereoReq,
                        sizeof(CSMI_INFO_GC_AUDIO_STEREO_REQ),
                        NULL,
                        0,
                        &dwRet,
                        NULL))
    {
        ERRORMSG(1, (TEXT("AC: GC_AUDIO_MODE_CHANGE..failed\r\n")));
    }
    else
    {
        DEBUGMSG( ZONE_MODEM, (TEXT("AC: GC_AUDIO_MODE_CHANGE..%d\r\n"), fHeadset));
    }
#endif
    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function: SetVoiceRxGain
//
//

BOOL
ACAudioHWContext::SetVoiceRxGain( DWORD dwGain )
{
    DEBUGMSG( ZONE_MODEM, (TEXT("AC: SetVoiceRxGain:...0x%x\r\n"),dwGain));
    return ( TRUE );
}

//------------------------------------------------------------------------------
//
//  Function: GetVoiceRxGain
//
//

BOOL
ACAudioHWContext::GetVoiceRxGain(DWORD *pdwGain)
{
    DEBUGMSG( ZONE_MODEM, (TEXT("AC: GetVoiceRxGain:...0x%x\r\n"),*pdwGain));
    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function: SetVoiceTxGain
//
//

BOOL
ACAudioHWContext::SetVoiceTxGain(DWORD dwGain)
{
    DEBUGMSG( ZONE_MODEM, (TEXT("AC: SetVoiceTxGain:...0x%x\r\n"),dwGain));
    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function: GetVoiceTxGain
//
//

BOOL
ACAudioHWContext::GetVoiceTxGain(DWORD *pdwGain)
{
    DEBUGMSG( ZONE_MODEM, (TEXT("AC: GetVoiceTxGain:...0x%x\r\n"),*pdwGain));
    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function: SetCMSIAudioInfo
//
//

BOOL
ACAudioHWContext::SetCMSIAudioInfo(PCMSI_AUDIO_SETTING pInfo)
{
    DEBUGMSG( ZONE_MODEM, (TEXT("AC: SetCMSIAudioInfo\r\n")));
#ifdef OMAP730
#ifndef SHIP_BUILD
    CAutoLock cs(&m_csModemDevice);

    DWORD dwRet;

    // Open the modem device in case it is not opened yet.
    if (!OpenModemDevice())
    {
        return FALSE;
    }

    CSMI_INFO_GC_AUDIO_VOICE_REQ  audioReq;
    audioReq.Uplink = pInfo->fUpLink;
    audioReq.Mute   = pInfo->fMute;
    audioReq.Volume = pInfo->bVolume;

    if (!DeviceIoCtrl(  m_hGsmDev,
                        GC_AUDIO_VOICE_REQ,
                        (LPVOID)&audioReq,
                        sizeof(CSMI_INFO_GC_AUDIO_VOICE_REQ),
                        NULL,
                        0,
                        &dwRet,
                        NULL))
    {
        ERRORMSG(1, (TEXT("AC: GC_AUDIO_VOICE_REQ..failed\r\n")));
        return FALSE;
    }
#endif


    DEBUGMSG( ZONE_MODEM, (TEXT("AC: CSMI_INFO_GC_AUDIO_VOICE_REQ:...OK\r\n")));
#endif
    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function: GetCMSIAudioInfo
//
//

BOOL
ACAudioHWContext::GetCMSIAudioInfo(PCMSI_AUDIO_SETTING pInfo)
{
    DEBUGMSG( ZONE_MODEM, (TEXT("AC: GetCMSIAudioInfo:...OK\r\n")));
    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function: SetEACAudioInfo
//
//  This function allows some tweaking of audio routing and volumes for 
//  debugging purposes in non ship builds
//

BOOL 
ACAudioHWContext::SetEACAudioInfo(PEAC_AUDIO_SETTING pInfo)
{
    DEBUGMSG( ZONE_MODEM, (TEXT("AC: SetEACAudioInfo\r\n")));
#ifdef OMAP730 
#ifndef SHIP_BUILD
    USHORT usVal, usBak;
    int i;

    if (pInfo->S != INREG16(&m_pEACRegisters->ASTCTR))
    {
        OUTREG16(&m_pEACRegisters->ASTCTR, pInfo->S);
    }

    usBak=INREG16(&m_pEACRegisters->AMSCFR);

    usVal=0;
    // switches K1-K12
    for (i=0;i<12;i++)
        if (pInfo->K[i])
            usVal |=(1 << i);

    if (usBak != usVal)
        OUTREG16(&m_pEACRegisters->AMSCFR,usVal);

    // DMA gain
    if (pInfo->DMAVOL!=INREG16(&m_pEACRegisters->AMVCTR))
        OUTREG16(&m_pEACRegisters->AMVCTR,pInfo->DMAVOL);

    if (pInfo->M[0]!=INREG16(&m_pEACRegisters->AM1VCTR))
        OUTREG16(&m_pEACRegisters->AM1VCTR,pInfo->M[0]);

    if (pInfo->M[1]!=INREG16(&m_pEACRegisters->AM2VCTR))
        OUTREG16(&m_pEACRegisters->AM2VCTR,pInfo->M[1]);

    if (pInfo->M[2]!=INREG16(&m_pEACRegisters->AM3VCTR))
        OUTREG16(&m_pEACRegisters->AM3VCTR,pInfo->M[2]);
#endif
    DEBUGMSG( ZONE_MODEM, (TEXT("AC: SetEACAudioInfo:...OK\r\n")));
#endif
    return TRUE;
}


//------------------------------------------------------------------------------
//
//  Function: GetCMSIAudioInfo
//
//

BOOL 
ACAudioHWContext::GetEACAudioInfo(PEAC_AUDIO_SETTING pInfo)
{
    DEBUGMSG( ZONE_MODEM, (TEXT("AC: GetEACAudioInfo\r\n")));
#ifdef OMAP730
#ifndef SHIP_BUILD
    USHORT usVal;
    int i;

    pInfo->S = INREG16(&m_pEACRegisters->ASTCTR);

    // switches K1-K12
    usVal = INREG16(&m_pEACRegisters->AMSCFR);
    for (i=0;i<12;i++)
        pInfo->K[i] = (usVal & (1 << i))!=0;

    // DMA gain
    pInfo->DMAVOL = INREG16(&m_pEACRegisters->AMVCTR);

    pInfo->M[0] = INREG16(&m_pEACRegisters->AM1VCTR);
    pInfo->M[1] = INREG16(&m_pEACRegisters->AM2VCTR);
    pInfo->M[2] = INREG16(&m_pEACRegisters->AM3VCTR);
#endif
#endif

    DEBUGMSG( ZONE_MODEM, (TEXT("AC: GetEACAudioInfo:...OK\r\n")));

    return TRUE;
}



//------------------------------------------------------------------------------
//
//  Function: ModemNetworkEnabled
//
//

BOOL
ACAudioHWContext::ModemNetworkEnabled (BOOL fEnable)
{
    DEBUGMSG( ZONE_MODEM, (TEXT("AC: ModemNetworkEnabled:...%d\r\n"), fEnable));

    SetModemDevice(fEnable);

    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function: InputStreamOpened
//
//

BOOL
ACAudioHWContext::InputStreamOpened()
{
    CAutoLock cs(&m_csModemDevice);

    DEBUGMSG( ZONE_MODEM, (TEXT("AC: InputStreamOpened\r\n")));
    // Open the modem device in case it is not opened yet.
    if (!OpenModemDevice ())
    {
        return FALSE;
    }

#ifdef OMAP730
    CSMI_INFO_GC_AUDIO_VOICE_REQ     audioReq;
    audioReq.Uplink = TRUE;                 // Uplink
    audioReq.Mute   = FALSE;                // no mute
    audioReq.Volume = GSM_UPLINK_VOLUME;    // Use default?
    DWORD dwRet;

    if (!DeviceIoCtrl(  m_hGsmDev,
                        GC_AUDIO_VOICE_REQ,
                        (LPVOID)&audioReq,
                        sizeof(CSMI_INFO_GC_AUDIO_VOICE_REQ),
                        NULL,
                        0,
                        &dwRet,
                        NULL)) {
        ERRORMSG(1, (TEXT("AC: GC_AUDIO_VOICE_REQ..failed\r\n")));
        return FALSE;
    }

    DEBUGMSG( ZONE_MODEM, (TEXT("AC: Input GC_AUDIO_VOICE_REQ:...0x%x\r\n"),
               audioReq.Volume));
#endif
    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function: InputStreamClosed
//
//  called on stop of wave capture.
//

BOOL
ACAudioHWContext::InputStreamClosed()
{
    CAutoLock cs(&m_csModemDevice);

    DEBUGMSG( ZONE_MODEM, (TEXT("AC: InputStreamClosed\r\n")));
#ifdef OMAP730
    // Open the modem device in case it is not opened yet.
    if (!OpenModemDevice())
    {
        return FALSE;
    }

    CSMI_INFO_GC_AUDIO_VOICE_REQ     audioReq;
    audioReq.Uplink = TRUE;                     // Uplink
    audioReq.Mute   = TRUE;                     // Mute
    audioReq.Volume = GSM_UPLINK_VOLUME;        // Use default?
    DWORD dwRet;

    if (!DeviceIoCtrl(  m_hGsmDev,
                        GC_AUDIO_VOICE_REQ,
                        (LPVOID)&audioReq,
                        sizeof(CSMI_INFO_GC_AUDIO_VOICE_REQ),
                        NULL,
                        0,
                        &dwRet,
                        NULL)) {
        ERRORMSG(1, (TEXT("AC: GC_AUDIO_VOICE_REQ..failed\r\n")));
        return FALSE;
    }

    DEBUGMSG( ZONE_MODEM, (TEXT("AC: InputStreamClosed:...\r\n")));
#endif
    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function: OutputStreamOpened
//
//

BOOL
ACAudioHWContext::OutputStreamOpened()
{
    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function: OutputStreamClosed
//
//

BOOL
ACAudioHWContext::OutputStreamClosed()
{
    return TRUE;
}


//------------------------------------------------------------------------------
//
//  Function: SetSideTone
//
//  control side tone
//
//
// Set the side tone and echo cancellation of the mic.
//
/* -------------------------------------------------------------------------- */
/*  aecControl register bits                                                  */
/*                                                                            */
/*      bit 0 : ACK bit : set to 1 in order to warn DSP that a new command  */
/*              is present.                                                   */
/*      bit 1 : enable AEC                                                    */
/*      bit 2 : enable SPENH (= Speech Enhancement = noise reduction)         */
/*      bit 3 : additionnal AEC gain attenuation (lsb)                        */
/*      bit 4 : additionnal AEC gain attenuation (msb)                        */
/*      bit 5 : additionnal SPENH gain attenuation (lsb)                      */
/*      bit 6 : additionnal SPENH gain attenuation (msb)                      */
/*      bit 7 : reset trigger for AEC                                         */
/*      bit 8 : reset trigger for SPENH                                       */
/*      bit 9 : AEC selector 0 : short AEC, 1 : long AEC                      */
/*                                                                            */
/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/*  VALID VALUES                                                              */
/*                                                                            */
/*  for Short AEC        0083                                                 */
/*  for long AEC         0283                                                 */
/*  for long AEC  -6 dB  028B                                                 */
/*  for long AEC  -12 dB 0293                                                 */
/*  for long AEC  -18 dB 029B                                                 */
/*  for SPENH            0105                                                 */
/*  for SPENH -6 dB      0125                                                 */
/*  for SPENH -12 dB     0145                                                 */
/*  for SPENH -18 dB     0165                                                 */
/*  for BOTH             0187                                                 */
/*  for STOP ALL         0001 (all bits reset + ACK to 1 to warn the DSP)     */
/*                                                                            */
/* -------------------------------------------------------------------------- */
//

BOOL
ACAudioHWContext::SetSideTone(BOOL fEnable)
{
    CAutoLock cs(&m_csModemDevice);

    DEBUGMSG( ZONE_MODEM, (TEXT("AC: SetSideTone\r\n")));
    // Open the modem device in case it is not opened yet.
    if (!OpenModemDevice())
    {
        return FALSE;
    }

#ifdef OMAP730
    DWORD dwRet;
    CSMI_INFO_GC_AUDIO_SIDETONE_REQ  sidetoneReq;
    if (fEnable)
    {
        sidetoneReq.Volume = 
        m_bHeadsetActive ? (UINT8)m_nSidetoneHeadset : (UINT8)m_nSidetoneSpeaker;
    }
    else 
    {
        sidetoneReq.Volume = 0;    
    }

    // Control sidetone
    if (!DeviceIoCtrl(  m_hGsmDev,
                        GC_AUDIO_SIDETONE_REQ,
                        (LPVOID)&sidetoneReq,
                        sizeof(CSMI_INFO_GC_AUDIO_SIDETONE_REQ),
                        NULL,
                        0,
                        &dwRet,
                        NULL)) {
        ERRORMSG(1, (TEXT("GC_AUDIO_SIDETONE_REQ..failed\r\n")));
        return FALSE;
    }

    DEBUGMSG( ZONE_MODEM, (TEXT("AC: GC_AUDIO_SIDETONE_REQ:...\r\n")));

    CSMI_INFO_GC_AUDIO_ECHOCANC_REQ  echocancReq;
    if (fEnable)
        echocancReq.Algorithm = 0x0001;   // All bits reset
    else
        echocancReq.Algorithm = 0x029B;   // -18 dB

    // Control echo cancellation
    if (!DeviceIoCtrl(  m_hGsmDev,
                        GC_AUDIO_ECHOCANC_REQ,
                        (LPVOID)&echocancReq,
                        sizeof(CSMI_INFO_GC_AUDIO_ECHOCANC_REQ),
                        NULL,
                        0,
                        &dwRet,
                        NULL)) {
        ERRORMSG(1, (TEXT("GC_AUDIO_ECHOCANC_REQ..failed\r\n")));
        return FALSE;
    }

    DEBUGMSG( ZONE_MODEM, (TEXT("AC: GC_AUDIO_ECHOCANC_REQ:...\r\n")));

#endif

    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function: HandleExtMessage
//
//  handle custom wave driver IOCtl for modem
//

BOOL
ACAudioHWContext::HandleExtMessage (
                        DWORD  dwCode,
                        PBYTE  pBufIn,
                        DWORD  dwLenIn,
                        PBYTE  pBufOut,
                        DWORD  dwLenOut,
                        PDWORD pdwActualOut)
{
    DWORD data;

    switch (dwCode)
    {
        // Receiving volume
        case IOCTL_VOICE_RX_VOLUME_SET:
            if (pdwActualOut) *pdwActualOut = 0;
            if (CeSafeCopyMemory(&data, pBufIn, sizeof(DWORD)) == 0) break;
            return SetVoiceRxGain(data);

        case IOCTL_VOICE_RX_VOLUME_GET:
            if (pdwActualOut) *pdwActualOut = sizeof (DWORD);
            if (CeSafeCopyMemory(&data, pBufIn, sizeof(DWORD)) == 0) break;
            return GetVoiceRxGain(&data);

        // Tramsmitting volume
        case IOCTL_VOICE_TX_VOLUME_SET:
            if (pdwActualOut) *pdwActualOut = sizeof (DWORD);
            if (CeSafeCopyMemory(&data, pBufIn, sizeof(DWORD)) == 0) break;
            return SetVoiceTxGain(data);

        case IOCTL_VOICE_TX_VOLUME_GET:
            if (pdwActualOut) *pdwActualOut = sizeof (DWORD);
            if (CeSafeCopyMemory(&data, pBufIn, sizeof(DWORD)) == 0) break;
            return GetVoiceTxGain(&data);

        case IOCTL_CMSI_AUDIO_INFO_SET:
            {
                CMSI_AUDIO_SETTING audio;
                if (pBufIn == NULL || dwLenIn<sizeof(CMSI_AUDIO_SETTING)) return FALSE;
                if (pdwActualOut) *pdwActualOut = 0;
                if (CeSafeCopyMemory(&audio, pBufIn, sizeof(audio)) == 0) break;
                return SetCMSIAudioInfo(&audio);
            }
        case IOCTL_CMSI_AUDIO_INFO_GET:
            {
                CMSI_AUDIO_SETTING audio;
                if (pBufOut == NULL || dwLenOut<sizeof(CMSI_AUDIO_SETTING)) return FALSE;
                if (pdwActualOut) *pdwActualOut = sizeof(CMSI_AUDIO_SETTING);
                if (GetCMSIAudioInfo(&audio)) 
                {
                    if (CeSafeCopyMemory(pBufOut, &audio, sizeof(audio)) == 0) break;
                    return TRUE;
                }
                break;
            }
        case IOCTL_START_AMR_CAPTURE:
            if (pdwActualOut) *pdwActualOut = 0;
            return SetAMRcapture(TRUE);

        case IOCTL_STOP_AMR_CAPTURE:
            if (pdwActualOut) *pdwActualOut = 0;
            return SetAMRcapture(FALSE);

        case IOCTL_EAC_INFO_SET:
            {
                EAC_AUDIO_SETTING audio;
                if (pBufIn == NULL || dwLenIn<sizeof(EAC_AUDIO_SETTING)) return FALSE;
                if (pdwActualOut) *pdwActualOut = 0;
                if (CeSafeCopyMemory(&audio, pBufIn, sizeof(audio)) == 0) break;
                return SetEACAudioInfo(&audio);
            }

        case IOCTL_EAC_INFO_GET:
            {
                EAC_AUDIO_SETTING audio;
                if (pBufOut == NULL || dwLenOut<sizeof(EAC_AUDIO_SETTING)) return FALSE;
                if (pdwActualOut) *pdwActualOut = sizeof(EAC_AUDIO_SETTING);
                if (GetEACAudioInfo(&audio)) {
                    if (CeSafeCopyMemory(pBufOut, &audio, sizeof(audio)) == 0) break;
                    return TRUE;
                }
            }
            break;

        default:
            ERRORMSG(1, (TEXT("WAVE: Unknown IOCTL_xxx(0x%08X), device = 0x%04X, function = 0x%03X \r\n"),
						dwCode, dwCode >> 16, (dwCode >> 2) & 0xFFF));
            break;
    }
    return FALSE;
}

