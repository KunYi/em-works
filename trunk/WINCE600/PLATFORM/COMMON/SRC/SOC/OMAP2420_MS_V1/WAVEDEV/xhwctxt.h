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
#pragma once

// Wave sample rates
#define SAMPLE_8K                   8000
#define SAMPLE_11K                  11025
#define SAMPLE_16K                  16000
#define SAMPLE_22K                  22050 
#define SAMPLE_32K                  32000
#define SAMPLE_44K                  44100
#define SAMPLE_48K                  48000
#define SAMPLE_88K                  88000
#define SAMPLE_96K                  96000

enum PIN_FUNC
{
    CONNECTION_NOT_DEFINED,
    V3U3V2T4_TO_EAC_BT_AUSPI,
    W6R9Y6Y5_TO_EAC_BT_AUSPI
};

class ACAudioHWContext : public OMAP2420DMAContext 
{

public:
    ACAudioHWContext(LPTSTR lpszContext);
    ~ACAudioHWContext();

    BOOL  HWMapControllerRegs();
    void  HWPowerUp( void );
    void  HWPowerDown( void );

    void  HWInitController ();
    BOOL  HWInitCodec( void ){ return TRUE; }

    BOOL  HWInitNetwork( void );
    BOOL  HWEnableNetwork( BOOL bEnable );

    BOOL  HWAudioPowerTimeout( BOOL bOn );
    void  HWUpdateAudioPRC( void );
    BOOL  HWDeinit( void );

    // for modem device notification
    BOOL OutputStreamOpened();
    BOOL OutputStreamClosed();
    BOOL InputStreamOpened();
    BOOL InputStreamClosed();

    // start/stop data transfer of I/O channels
    void HWEnableInputChannel (BOOL fEnable);
    void HWEnableOutputChannel (BOOL fEnable);


    void SetRecordMemoPath (BOOL fOn);

    BOOL HandleExtMessage ( DWORD  dwCode,
                           PBYTE  pBufIn,
                           DWORD  dwLenIn,
                           PBYTE  pBufOut,
                           DWORD  dwLenOut,
                           PDWORD pdwActualOut);

    BOOL PmControlMessage ( 
                        DWORD  dwCode,
                        PBYTE  pBufIn,
                        DWORD  dwLenIn,
                        PBYTE  pBufOut,
                        DWORD  dwLenOut,
                        PDWORD pdwActualOut);

    BOOL SetCMSIAudioInfo(PCMSI_AUDIO_SETTING pInfo);
    BOOL GetCMSIAudioInfo(PCMSI_AUDIO_SETTING pInfo);
    BOOL SetEACAudioInfo(PEAC_AUDIO_SETTING pInfo);
    BOOL GetEACAudioInfo(PEAC_AUDIO_SETTING pInfo);

private:

// Codec functions.
    VOID SetCodecPower( BOOL fPowerOn );
    VOID SetOutputVolume(DWORD dwVolume);

    void InitCodecPort();

// Audio controller.
    void SetControllerClocks (BOOL fOn);
    BOOL SetAMRcapture (BOOL fStart);
    void EnableExtSpeaker(BOOL bEnable);

// Modem device. 
    BOOL InitModem();
    BOOL SetModemDevice (BOOL fOn);
    BOOL SetModemHeadset (BOOL fHeadset);
    BOOL ModemNetworkEnabled(BOOL fEnable);
    BOOL SetSideTone(BOOL fEnable);

    BOOL OpenModemDevice ();
    BOOL SetVoiceRxGain( DWORD dwGain );
    BOOL GetVoiceRxGain(DWORD *pdwGain);
    BOOL SetVoiceTxGain(DWORD dwGain);
    BOOL GetVoiceTxGain(DWORD *pdwGain);

    VOID InitModemPort();
    VOID InitBluetoothPort();
    VOID ConfigEacBTAuSpiPins(PIN_FUNC PinFunction);


//debug helper function    
    VOID DumpMCBSPRegisters();

// Functions to select audio output/input from BT or the voice codec.
    void SelectBtAudio(DWORD dwAudioRouting);
    void SelectVoiceCodec();

    BOOL RegisterRilCallback (HANDLE *phRil);
    BOOL UnregisterRilCallback (HANDLE);    
//------------------------------------------------------------------------------

protected:
    // External Power State Request.
    CEDEVICE_POWER_STATE m_ExternPowerStateRequired;
    // modem specific private
    // GSM driver handle.
    HANDLE m_hGsmDev;
    
    // Critical section for Modem.
    CRITICAL_SECTION m_csModemDevice;

    // Modem codec init flag. 
    BOOL   m_fModemCodecOn;
    BOOL   m_fHeadsetOn;
    
//------------------------------------------------------------------------------

    // Bluetooth current pin connection
    PIN_FUNC m_CurBTEacConnection;

//------------------------------------------------------------------------------
    
    CRITICAL_SECTION m_csAudioPRC;  // Critical section for PRC.

    BOOL m_bEACPowerOn;         // actual codec power state. 

    BOOL m_bModemPortActive;     // Modem port active (on call).
    BOOL m_bPowerTimeout;        // Any power can be off. 

    BOOL m_bBtHeadsetActive;     // BT headset active. 
    BOOL m_bHeadsetActive;       // Headset is active 
    BOOL m_bLoudSpeakerActive;   // Loud speaker is active 
    BOOL m_bChooseBtHeadset;     // Flag indicating that the BT headset is used 
                                 // even if the physically connected headset is 
                                 // plugged in.

//------------------------------------------------------------------------------

	// Pointers to controllers.
//	OMAP730_EAC_REGS		*m_pEACRegisters;
	OMAP2420_McBSP_REGS		*m_pMCBSPRegisters;
	OMAP2420_PRCM_REGS		*m_pPRCMRegs;
	HANDLE					m_hSPI;

//------------------------------------------------------------------------------
//
//  these members can be preset by registry
//

public:
    DWORD m_nSidetoneHeadset;
    DWORD m_nSidetoneSpeaker;
    DWORD m_nDMAReadVol;
    DWORD m_nDMAWriteVol;
    DWORD m_nGSMSpeakerVol;
    DWORD m_nWavSpeakerVol;
    DWORD m_nWavGSMVol;
    DWORD m_nGSMWavVol;

//------------------------------------------------------------------------------
    
};

// Power management APIs. 
#if defined( BSP_SMARTPHONE) || defined( BSP_POCKETPC )

extern "C"
{
    void CALLBACK NotifyCallback(DWORD, const void*, DWORD, DWORD);
    void CALLBACK ResultCallback(DWORD, HRESULT, const void*, DWORD, DWORD);
}
#endif
