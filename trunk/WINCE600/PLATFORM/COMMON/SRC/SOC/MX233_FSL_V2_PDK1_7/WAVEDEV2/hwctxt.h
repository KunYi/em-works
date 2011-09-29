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
// -----------------------------------------------------------------------------
//
//  Copyright (C) 2009 Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
// -----------------------------------------------------------------------------

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
#include "soc_types.h"
#include "csp.h"

#define OUTPUT_SAMPLERATE  (44100)
#define INPUT_SAMPLERATE   (44100)

#define AUDIO_DMA_PAGE_SIZE     4096
#define AUDIO_DMA_NUMBER_PAGES  2
#define AUDIO_DELAY_EVENTS      3

// Types
typedef enum
{
    AUDIO_BUS_DAC,
    AUDIO_BUS_ADC
} AUDIO_BUS;

class HardwareContext
{
public:
    static BOOL CreateHWContext(DWORD Index);
    HardwareContext();
    ~HardwareContext();

    void Lock()   {
        EnterCriticalSection(&m_Lock);
    }
    void Unlock() {
        LeaveCriticalSection(&m_Lock);
    }

    DWORD GetNumInputDevices()  {
        return m_NumInputDevices;
    }
    DWORD GetNumOutputDevices() {
        return 1;
    }
    DWORD GetNumMixerDevices()  {
        return 1;
    }

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
    void InterruptThread2();
    void DisableDelayThread();

    DWORD ForceSpeaker (BOOL bSpeaker);

    BOOL PmControlMessage (
        DWORD dwCode,
        PBYTE pBufIn,
        DWORD dwLenIn,
        PBYTE pBufOut,
        DWORD dwLenOut,
        PDWORD pdwActualOut);

    DWORD GetDriverRegValue(LPWSTR ValueName, DWORD DefaultValue);
    void SetDriverRegValue(LPWSTR ValueName, DWORD Value);

    BOOL IsSupportedOutputFormat(LPWAVEFORMATEX lpFormat);

    BOOL SelectInputSource(DWORD nIndex);
    BOOL SetCodecLoopback(BOOL bEnable);

protected:
    BOOL InitInterruptThread();
    BOOL InitDisableDelayThread();
    BOOL InitHPDetectInterruptThread();

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

    InputDeviceContext m_InputDeviceContext;
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
    BOOL m_fInputMute;
    BOOL m_fOutputMute;

protected:
    DWORD m_AudioSysintr;

    HANDLE m_hAudioIntrEvent;
    HANDLE m_hAudioIntrThread;

    DWORD m_AudioSysintr2;
    HANDLE m_hAudioIntrEvent2;
    HANDLE m_hAudioIntrThread2;

    // DMA buffers and pages
    PHYSICAL_ADDRESS m_DMAOutBufPhysAddr;
    PHYSICAL_ADDRESS m_DMAInBufPhysAddr;

    PBYTE m_DMAOutBufVirtAddr;
    PBYTE m_DMAInBufVirtAddr;

    PBYTE m_DMAOutPageVirtAddr[AUDIO_DMA_NUMBER_PAGES];
    PBYTE m_DMAInPageVirtAddr[AUDIO_DMA_NUMBER_PAGES];

    // DMA Descriptor

    PHYSICAL_ADDRESS m_DMAOutDesPhysAddr;
    PHYSICAL_ADDRESS m_DMAInDesPhysAddr;

    PAUDIO_DMA_DESCRIPTOR m_DMAOutDescriptorPhysAddr;
    PAUDIO_DMA_DESCRIPTOR m_DMAInDescriptorPhysAddr;

    PAUDIO_DMA_DESCRIPTOR m_DMAINDescriptorVirtAddr;
    PAUDIO_DMA_DESCRIPTOR m_DMAOutDescriptorVirtAddr;


// FSL audio porting
private:
    BOOL m_PowerUpState;
    UINT8 m_NumInputDevices;
    UINT16 m_OutputDMAPageSize;
    UINT16 m_InputDMAPageSize;

    PVOID pv_HWregAUDIOOUT;
    PVOID pv_HWregAUDIOIN;

    volatile DWORD m_OutputDMAStatus;
    volatile DWORD m_InputDMAStatus;

    UINT8 m_OutputDMAChan;
    UINT8 m_InputDMAChan;


    // Flag to indicate if we can stop output DMA.
    BOOL m_bCanStopOutputDMA;

    // Flag to bypass delay of disabling operation.
    BOOL m_delayBypass;

    // Flag to indicate the Codec loopback state
    BOOL m_bCodecLoopbackState;
    DWORD m_EmptyBufer;
    BOOL m_bFirstTime;
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

    void StartCodec(AUDIO_BUS audioBus);
    void StopCodec(AUDIO_BUS audioBus);

    BOOL InitOutputDMA();
    BOOL InitInputDMA();

    void OutputIntrRoutine();
    void InputIntrRoutine();

public:

    UINT8 BSPAudioGetInputDeviceNum();
    UINT16 BSPAllocOutputDMABuffer(PBYTE *pVirtAddr, PHYSICAL_ADDRESS *pPhysAddr);
    UINT16 BSPAllocInputDMABuffer(PBYTE *pVirtAddr, PHYSICAL_ADDRESS *pPhysAddr);
    void BSPDeallocOutputDMABuffer(PBYTE VirtAddr);
    void BSPDeallocInputDMABuffer(PBYTE VirtAddr);

    UINT16 BSPAllocOutputDMADescriptor(PAUDIO_DMA_DESCRIPTOR *pVirtAddr,PHYSICAL_ADDRESS *pPhysAddr );
    UINT16 BSPAllocInputDMADescriptor(PAUDIO_DMA_DESCRIPTOR  *pVirtAddr,PHYSICAL_ADDRESS *pPhysAddr);
    VOID BSPDeallocOutputDMADescriptor(PBYTE VirtAddr);
    VOID BSPDeallocInputDMADescriptor(PBYTE VirtAddr);
    BOOL BSPSetupOutputDMADescriptor();
    BOOL BSPSetupInputDMADescriptor();

    BOOL BSPAudioInitCodec();
    void BSPAudioDeinitCodec();
    void BSPAudioStartDAC();
    void BSPAudioStopDAC();
    void BSPAudioStartADC();
    void BSPAudioStopADC();
    void BSPAudioSelectADCSource(DWORD nIndex);
    BOOL BSPAudioSetCodecLoopback(BOOL bEnable);
    void BSPAudioSetOutputGain(DWORD dwGain);
    void BSPAudioSetInputGain(DWORD dwGain);
    void BSPAudioPowerUp(const BOOL fullPowerUp);
    void BSPAudioPowerDown(const BOOL fullPowerOff);
    void BSPSetSpeakerEnable(BOOL bEnable);
    void SpeakerHeadphoneSelection();
    void BSPAudioHandleHPDetectIRQ();
    BOOL BSPIsHeadphoneDetectSupported();
    DWORD BSPGetHPDetectIRQ();
};

void CallInterruptThread(HardwareContext *pHWContext);
void CallInterruptThread2(HardwareContext *pHWContext);
void CallDisableDelayThread(HardwareContext *pHWContext);

extern HardwareContext *g_pHWContext;


