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
//------------------------------------------------------------------------------
//
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

#pragma once
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.


Module Name:    HWCTXT.H

Abstract:               Platform dependent code for the mixing audio driver.

-*/

#include <ceddk.h>
#include <ddkreg.h>
#include "common_macros.h"
#include "common_ddk.h"
#include "audiotypes.h"
#include "common_esai.h"
#include "asrc_base.h"

#define OUTPUT_SAMPLERATE  (48000)
#define INPUT_SAMPLERATE   (48000)

#define AUDIO_DMA_NUMBER_PAGES  2
#define AUDIO_DELAY_EVENTS      3


class HardwareContext
{
public:
    static BOOL CreateHWContext(DWORD Index);
    HardwareContext();
    ~HardwareContext();

    void Lock()   {EnterCriticalSection(&m_Lock);}
    void Unlock() {LeaveCriticalSection(&m_Lock);}

    DWORD GetNumInputDevices()  {return m_NumInputDevices;}
    DWORD GetNumOutputDevices() {return 1;}
    DWORD GetNumMixerDevices()  {return 1;}

    DeviceContext *GetInputDeviceContext(UINT DeviceId)
    {
        UNREFERENCED_PARAMETER(DeviceId);
        return &m_InputDeviceContext;
    }

    DeviceContext *GetOutputDeviceContext(UINT DeviceId)
    {
        UNREFERENCED_PARAMETER(DeviceId);
        return &m_OutputDeviceContext;
    }

    BOOL Init(DWORD Index);
    BOOL Deinit();

    void PowerUp();
    void PowerDown();

    BOOL StartInputDMA();
    BOOL StartOutputDMA();

    void StopInputDMA();
    void StopOutputDMA();

    void InterruptThread();
    void DisableDelayThread();

    DWORD ForceSpeaker (BOOL bSpeaker);

    BOOL PmControlMessage (
                      DWORD  dwCode,
                      PBYTE  pBufIn,
                      DWORD  dwLenIn,
                      PBYTE  pBufOut,
                      DWORD  dwLenOut,
                      PDWORD pdwActualOut);

    DWORD GetDriverRegValue(LPWSTR ValueName, DWORD DefaultValue);
    void SetDriverRegValue(LPWSTR ValueName, DWORD Value);
    
    BOOL IsSupportedOutputFormat(LPWAVEFORMATEX lpFormat);
    BOOL IsSupportedInputFormat(LPWAVEFORMATEX lpFormat);

    BOOL SelectInputSource(DWORD nIndex);

    BOOL InitOutputWaveFormat(LPWAVEFORMATEX lpFormat);
    BOOL InitInputWaveFormat(LPWAVEFORMATEX lpFormat);

    BOOL DeinitOutputWaveFormat(LPWAVEFORMATEX lpFormat);
    BOOL DeinitInputWaveFormat(LPWAVEFORMATEX lpFormat);

    BOOL IsSupportedWAVEFORMATEX(){
        return m_bSupportWAVEFORMATEX;
        };
    
    //BOOL SetCodecLoopback(BOOL bEnable);
    void AsrcIntrThread(void);
    HANDLE m_hAsrcIntrEvent;
    HANDLE m_hAsrcTrigEvent;

protected:
    BOOL InitInterruptThread();
    BOOL InitDisableDelayThread();

    ULONG TransferInputBuffers(UINT8 checkFirst);
    ULONG TransferOutputBuffers();
    ULONG TransferBuffer(AUDIO_BUS audioBus, UINT8 NumBuf);

    DWORD GetInterruptThreadPriority()
    {
        return GetDriverRegValue(TEXT("Priority256"),150);
    }

    DWORD m_DriverIndex;
    CRITICAL_SECTION m_Lock;

    BOOL m_Initialized;
    BOOL m_InPowerHandler;

    InputDeviceContext  m_InputDeviceContext;
    OutputDeviceContext m_OutputDeviceContext;

    // Track number of valid samples in each DMA output buffer
    ULONG m_OutBytes[AUDIO_DMA_NUMBER_PAGES];

    // Which DMA channels are running
    BOOL m_InputDMARunning;
    BOOL m_OutputDMARunning;

    CEDEVICE_POWER_STATE m_DxState;

    LONG m_NumForcedSpeaker;
    void SetSpeakerEnable(BOOL bEnable);
    void RecalcSpeakerEnable();

// Gain-related APIs
public:
    DWORD       GetOutputGain (void);
    MMRESULT    SetOutputGain (DWORD dwVolume);
    DWORD       GetInputGain (void);
    MMRESULT    SetInputGain (DWORD dwVolume);

    BOOL        GetOutputMute (void);
    MMRESULT    SetOutputMute (BOOL fMute);
    BOOL        GetInputMute (void);
    MMRESULT    SetInputMute (BOOL fMute);

protected:
    void UpdateOutputGain();
    void UpdateInputGain();
    DWORD m_dwOutputGain;
    DWORD m_dwInputGain;
    BOOL  m_fInputMute;
    BOOL  m_fOutputMute;

    WAVEFORMATEXTENSIBLE m_wavInputFmtEx;  
    BOOL m_bWaveInputFormatInitialed;

    WAVEFORMATEXTENSIBLE m_wavOutputFmtEx;  
    BOOL m_bWaveOutputFormatInitialed;

protected:
    DWORD m_AudioSysintr;
    
    HANDLE m_hAudioIntrEvent;
    HANDLE m_hAudioIntrThread;

    // DMA buffers and pages
    PHYSICAL_ADDRESS m_DMAOutBufPhysAddr;
    PHYSICAL_ADDRESS m_DMAInBufPhysAddr;

    PBYTE m_DMAOutBufVirtAddr;
    PBYTE m_DMAInBufVirtAddr;

    PBYTE m_DMAOutPageVirtAddr[AUDIO_DMA_NUMBER_PAGES];
    PBYTE m_DMAInPageVirtAddr[AUDIO_DMA_NUMBER_PAGES];

// FSL audio porting
private:
    UINT8 m_NumInputDevices;
    UINT16 m_OutputDMAPageSize;
    UINT16 m_InputDMAPageSize;
    
    PCSP_ESAI_REG m_pESAI;

    volatile DWORD m_OutputDMAStatus;
    volatile DWORD m_InputDMAStatus;
    
    UINT8 m_OutputDMAChan;
    UINT8 m_InputDMAChan;

    // Set to TRUE if an SSI TX underrun error has occurred 
    // and we must now perform a recovery operation.
    BOOL  m_bESAITXUnderrunError;
    BOOL  m_bESAIRXOverrunError;
    
    // Flag to indicate if we can stop output DMA.
    BOOL  m_bCanStopOutputDMA;
    
    // Flag to bypass delay of disabling operation.
    BOOL m_delayBypass;

    // Flag to indicate the Codec loopback state
    BOOL m_bCodecLoopbackState;

    //Flag to support WAVEFORMATEX besides WAVEFORMATEXTENSIBLE

    BOOL m_bSupportWAVEFORMATEX;
    // Declare timer IDs that will be used to control the timed delay when
    // disabling the audio hardware. This allows better performance when
    // back-to-back audio operations are performed.
    MMRESULT m_DACDelayDisableTimerID;
    MMRESULT m_ADCDelayDisableTimerID;

    HANDLE m_hDACDelayDisableEvent;
    HANDLE m_hADCDelayDisableEvent;
    
    // The event that the audio driver IST will use to signal when it
    // wants to stop the output DMA (at the end of each playback operation).
    // Note that we don't want the audio driver IST thread to directly call
    // StopDMAChannel() since this also involves a call to some MMTIMER APIs
    // where the priority of the calling thread is changed. To be safe, we
    // want to avoid any changes to the IST thread priority.
    HANDLE m_hStopOutputDMAEvent;

    HANDLE h_AudioDelayEvents[AUDIO_DELAY_EVENTS];
    
    // Handle for thread that will process all timeout events
    // from the CODEC delayed disable timer.
    HANDLE m_hAudioDelayedDisableThread;

    BOOL m_bAsrcSupport;
    BOOL m_bAsrcEnable;
    BOOL m_bAsrcOpened; // we have requested one asrc pair
    ASRC_PAIR_INDEX m_asrcPairIndex;
    DWORD m_dwAsrcMaxChnNum;
    HANDLE m_hAsrcIntrThread;
    void StartCodec(AUDIO_BUS audioBus);
    void StopCodec(AUDIO_BUS audioBus);

    void DumpESAIRegs(void);

    void InitESAI(void);
    void StartESAI(AUDIO_BUS audioBus);
    void StopESAI(AUDIO_BUS audioBus);
    void PrepareESAI(AUDIO_BUS audioBus);

    void ConfigESAITx(void);
    BOOL ConfigOutputDMA(void);

    void ConfigESAIRx(void);
    BOOL ConfigInputDMA(void);

    BOOL InitAsrcP2P(void);
    void DeinitAsrcP2P(void);
    BOOL ConfigAsrc(void);
    void AsrcIntrRoutine();
    BOOL InitOutputDMA();
    BOOL InitInputDMA();

    void OutputIntrRoutine();
    void InputIntrRoutine();
};

void CallInterruptThread(HardwareContext *pHWContext);
void CallDisableDelayThread(HardwareContext *pHWContext);

extern HardwareContext *g_pHWContext;


