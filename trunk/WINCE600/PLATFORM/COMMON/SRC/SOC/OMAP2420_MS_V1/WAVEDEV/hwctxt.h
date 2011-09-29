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

#include <pm.h>

// Audio output devices 
typedef enum __AUDIO_DEVICE__
{
    DEVICE_NORMAL,
    DEVICE_HEADSET,
    DEVICE_CARKIT, 
    DEVICE_SPEAKER,
    DEVICE_MAX_NUM
} AUDIO_DEVICE;

// Number of output channels, 1=mono, 2=stereo
#define OUTCHANNELS     (2)

// Number of significant bits per sample (e.g. 16=16-bit codec, 24=24-bit codec)
#define BITSPERSAMPLE   (16)

// Native sample rate
#define SAMPLERATE      (44100)

// Inverse sample rate, in 0.32 fixed format, with 1 added at bottom to ensure round up.
#define INVSAMPLERATE ((UINT32)(((1i64<<32)/SAMPLERATE)+1))

// Native sample size, INT16 for 16-bit samples, INT32 for anything larger.
typedef INT16 HWSAMPLE;
typedef HWSAMPLE *PHWSAMPLE;

// Set USE_MIX_SATURATE to 1 if you want the mixing code to guard against saturation
#define USE_MIX_SATURATE (1)

// Set USE_HW_SATURATE to 1 if you want the hwctxt code to guard against saturation
// This only works if the DMA sample size is greater than 16 bits.
#define USE_HW_SATURATE (0)

// The code will then use the follwing values as saturation points
// Minimum and maximum sample values for saturation calculations
#define AUDIO_SAMPLE_MAX ((HWSAMPLE)32767)
#define AUDIO_SAMPLE_MIN ((HWSAMPLE)-32768)

// Size of 1 dma page. There are 2 pages for input & 2 pages for output,
// so the total size of the DMA buffer will be 
// AUDIO_DMA_PAGES * AUDIO_DMA_PAGE_SIZE * 2
#define AUDIO_DMA_PAGE_SIZE     0x2000
#define AUDIO_DMA_PAGES         2

// defaults for software mixer volumes in db
#define STREAM_ATTEN_MAX    100
#define DEVICE_ATTEN_MAX    35
#define SECOND_ATTEN_MAX    35


//
// Control register bit definitions
//

class HardwareContext
{
public:
    static BOOL CreateHWContext(DWORD Index);
    HardwareContext();
    virtual ~HardwareContext();

    void Lock()   {EnterCriticalSection(&m_Lock);}
    void Unlock() {LeaveCriticalSection(&m_Lock);}

    DWORD GetNumInputDevices()  {return 1;}
    DWORD GetNumOutputDevices() {return 1;}

    DeviceContext *GetInputDeviceContext(UINT DeviceId)
    {
        return &m_InputDeviceContext;
    }

    DeviceContext *GetOutputDeviceContext(UINT DeviceId)
    {
        return &m_OutputDeviceContext;
    }

    DWORD GetStreamAttenMax()
    {
        return m_dwStreamAttenMax;
    }
    DWORD SetStreamAttenMax(DWORD dwSetAtten)
    {
        m_dwStreamAttenMax=dwSetAtten;
    }
    DWORD GetDeviceAttenMax()
    {
        return m_dwDeviceAttenMax;
    }
    DWORD SetDeviceAttenMax(DWORD dwSetAtten)
    {
        m_dwDeviceAttenMax=dwSetAtten;
    }

    DWORD GetSecondAttenMax()
    {
        return m_dwSecondAttenMax;
    }
    DWORD SetSecondAttenMax(DWORD dwSetAtten)
    {
        m_dwSecondAttenMax=dwSetAtten;
    }


    BOOL Init(DWORD Index);
    BOOL Deinit();

    void PowerUp();
    void PowerDown();

    void StartInputDMA();
    void StartOutputDMA();

    void StopInputDMA();
    void StopOutputDMA();

    void InterruptThreadTx();
    void InterruptThreadRx();
    void TimeoutThread();

    DWORD ForceSpeaker (BOOL bSpeaker);
    void RecalcSpeakerEnable();

    ULONG TransferInputBuffer(ULONG NumBuf);
    ULONG TransferOutputBuffer(ULONG NumBuf);

    BOOL PrepareForVoiceCall( BOOL bInVoiceCall );
    BOOL SetExtSpeakerPower( BOOL fOn );

    void NotifyHeadsetOn (BOOL);
    void ToggleExtSpeaker ();
    VOID NotifyBtHeadsetOn(DWORD dwBtAudioRouting);

    // Hardware DMA specific functions
    virtual BOOL  HWMapDMAMemory( DWORD dwSize )=0;
    virtual void  HWInitInputDMA( void )=0;
    virtual void  HWInitOutputDMA( void )=0;
    virtual void  HWStartOutputDMA( void )=0;
    virtual void  HWStopOutputDMA( void )=0;
    virtual void  HWStartInputDMA( void )=0;
    virtual void  HWStopInputDMA( void )=0;
    virtual ULONG HWTransferInputBuffers( void )=0;
    virtual ULONG HWTransferOutputBuffers( void )=0;
    virtual PBYTE HWDMAMemoryIn( void )=0;
    virtual PBYTE HWDMAMemoryOut( void )=0;

    // Platform specific part of EAC configuration
    virtual BOOL HWMapControllerRegs()=0;
    virtual void HWInitController ()=0;

    virtual BOOL HWInitCodec( void )=0;
    virtual BOOL HWInitNetwork( void )=0;
    virtual BOOL HWEnableNetwork( BOOL bEnable )=0;
    virtual BOOL HWAudioPowerTimeout( BOOL bOn )=0;
    virtual void HWUpdateAudioPRC( void )=0;
    virtual void HWPowerUp( void )=0;
    virtual void HWPowerDown( void )=0;
    virtual BOOL HWDeinit( void )=0;

    // for modem device notification
    virtual BOOL OutputStreamOpened()=0;
    virtual BOOL OutputStreamClosed()=0;
    virtual BOOL InputStreamOpened()=0;
    virtual BOOL InputStreamClosed()=0;

    virtual void SetRecordMemoPath (BOOL fOn)=0;

    virtual BOOL HandleExtMessage ( DWORD  dwCode,
                           PBYTE  pBufIn,
                           DWORD  dwLenIn,
                           PBYTE  pBufOut,
                           DWORD  dwLenOut,
                           PDWORD pdwActualOut)=0;
    virtual BOOL PmControlMessage ( 
                        DWORD  dwCode,
                        PBYTE  pBufIn,
                        DWORD  dwLenIn,
                        PBYTE  pBufOut,
                        DWORD  dwLenOut,
                        PDWORD pdwActualOut)=0;
    virtual BOOL RegisterRilCallback (HANDLE *phRil)=0;
protected:
    void  OutputBufferCacheFlush (PBYTE pbMem, DWORD dwLen);
    void  InputBufferCacheDiscard(PBYTE pbMem, DWORD dwLen);
    virtual void  SetupDelayUpdate();
    virtual void DelayedUpdate();
    BOOL  InitMemories();
    void  InitDMA();
    void  InitController();
    void  InitCodec();
    BOOL  InitInterruptThread();
    DWORD MapIrqToSysIntr(DWORD irq);
      
    void GetRegistryValue();
    void TerminateCloseThread(HANDLE hThread) ;

    CRITICAL_SECTION m_Lock;

    DWORD m_dwPriority256;
    DWORD m_dwTimeoutTicks ;
    BOOL  m_fRequestedSysIntr;
    DWORD m_IntrAudioTx;
    DWORD m_IntrAudioRx;
    BOOL  m_fTerminating;

    BOOL  m_fRxInterruptIntialized ;
    BOOL  m_fTxInterruptIntialized ;
    HANDLE m_hAudioInterruptTx;           // Handle to Audio Interrupt event.
    HANDLE m_hAudioInterruptRx;           // Handle to Audio Interrupt event.
    HANDLE m_hAudioInterruptThreadTx;     // Handle to thread which waits on an 
                                          // audio transmit interrupt event.
    HANDLE m_hAudioInterruptThreadRx;     // Handle to thread which waits on an 
                                          // audio receive interrupt event.
    HANDLE m_hTimeoutEvent;
    HANDLE m_hTimeoutThread;

    ULONG TransferInputBuffers();
    ULONG TransferOutputBuffers();

#if USE_HW_SATURATE
    void HandleSaturation(PBYTE pBuffer, PBYTE pBufferEnd);
#endif

    // PM Required.
    CEDEVICE_POWER_STATE    m_CurPowerState;
    HANDLE                  m_hParent;

    DWORD m_DriverIndex;

    BOOL m_Initialized;
    BOOL m_InPowerHandler;
    
    LONG m_NumForcedSpeaker;
    
    BOOL m_InputDMARunning;
    BOOL m_OutputDMARunning;

    InputDeviceContext m_InputDeviceContext;
    OutputDeviceContext m_OutputDeviceContext;

    PBYTE   m_Input_pbDMA_PAGES [AUDIO_DMA_PAGES];
    PBYTE   m_Output_pbDMA_PAGES[AUDIO_DMA_PAGES];

    // audio state helper variables
    BOOL m_bDMARunning;          // DMA is running. 
    BOOL m_bHeadsetPluggedIn;    // Headset is plugged.
    BOOL m_bToggleLoadSpeaker;   // Load speaker is toggle.
    HANDLE m_hRil;               // RIL handle.
    BOOL  m_bBtHeadsetSelected;  // BT headset has been selected.
    DWORD m_dwBtAudioRouting;    // BT audio routing.

public:
    DWORD m_dwStreamAttenMax;
    DWORD m_dwDeviceAttenMax;
    DWORD m_dwSecondAttenMax;
    
};




extern HardwareContext *g_pHWContext;

//------------------------------------------------------------------------------
//
//  Function: OutputBufferCacheFlush ()
//  
//  utility function to flush cached mixer DMA memory 
//

inline void
HardwareContext::OutputBufferCacheFlush(PBYTE pbMem, DWORD dwLen)
{
#ifdef MIXER_CACHEDMEM
    CacheRangeFlush(pbMem, dwLen, CACHE_SYNC_WRITEBACK);
#endif
}


//------------------------------------------------------------------------------
//
//  Function: InputBufferCacheDiscard ()
//  
//  utility function to discard cached audio capture DMA memory 
//

inline void
HardwareContext::InputBufferCacheDiscard(PBYTE pbMem, DWORD dwLen)
{
#ifdef INPUT_CACHEDMEM
    CacheRangeFlush(pbMem, dwLen, CACHE_SYNC_DISCARD);
#endif
}
