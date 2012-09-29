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
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Module Name:    HWCTXT.CPP
//
//  Abstract:               Platform dependent code for the mixing audio driver.
//
//  Notes:                  The following file contains all the hardware specific code
//  for the mixing audio driver.  This code's primary responsibilities are:
//
//  Initialize audio hardware (including codec chip)
//  Schedule DMA operations (move data from/to buffers)
//  Handle audio interrupts
//
//  All other tasks (mixing, volume control, etc.) are handled by the "upper"
//  layers of this driver.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2009 - 2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

#include <windows.h>
#include <types.h>
#include <memory.h>
#include <excpt.h>
#include <wavepdd.h>
#include <waveddsi.h>
#include <wavedbg.h>
#include <nkintr.h>
#include <ceddk.h>
#include "wavemain.h"


//-----------------------------------------------------------------------------
// External Functions

//-----------------------------------------------------------------------------
// External Variables


//-----------------------------------------------------------------------------
// Defines

//----- Used to track DMA controllers status -----
#define DMA_CLEAR           0x00000000
#define DMA_DONEA           0x00000002
#define DMA_DONEB           0x00000004

//----- Used for scheduling DMA transfers -----
#define BUFFER_A            0
#define BUFFER_B            1


//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables
HardwareContext *g_pHWContext=NULL;


//-----------------------------------------------------------------------------
// Local Variables

static UINT32 PlaybackDisableDelayMsec = 100;
static UINT32 RecordDisableDelayMsec = 2000;

static BOOL g_saveOutputDMARunning = FALSE;
static BOOL g_saveInputDMARunning  = FALSE;


BOOL HardwareContext::CreateHWContext(DWORD Index)
{
    if (g_pHWContext)
    {
        return TRUE;
    }

    g_pHWContext = new HardwareContext;
    if (!g_pHWContext)
    {
        return FALSE;
    }

    return g_pHWContext->Init(Index);
}

HardwareContext::HardwareContext() :
    m_InputDeviceContext(),
    m_OutputDeviceContext()
{
    InitializeCriticalSection(&m_Lock);
    m_Initialized = FALSE;

    m_NumInputDevices = BSPAudioGetInputDeviceNum();
}

HardwareContext::~HardwareContext()
{
    DeleteCriticalSection(&m_Lock);
}


BOOL HardwareContext::Init(DWORD Index)
{
    if (m_Initialized)
    {
        return FALSE;
    }

    //
    // Initialize the state/status variables
    //
    m_PowerUpState      = TRUE;
    m_DriverIndex       = Index;
    m_InPowerHandler    = FALSE;
    m_InputDMARunning   = FALSE;
    m_OutputDMARunning  = FALSE;
    m_NumForcedSpeaker  = 0;

    m_dwOutputGain = 0xFFFFFFFF;
    m_dwInputGain  = 0xFFFFFFFF;
    m_fInputMute  = FALSE;
    m_fOutputMute = FALSE;

    m_OutputDMAStatus = DMA_CLEAR;
    m_InputDMAStatus  = DMA_CLEAR;

    m_bCanStopOutputDMA   = TRUE;
    m_delayBypass         = FALSE;
    m_bCodecLoopbackState = FALSE;
    m_bFirstTime                  = TRUE;
    m_InputDeviceContext.SetBaseSampleRate(INPUT_SAMPLERATE);
    m_OutputDeviceContext.SetBaseSampleRate(OUTPUT_SAMPLERATE);

    //
    // Initialize hardware
    //
    PHYSICAL_ADDRESS phyAddr;

    // Map audio peripherals
    phyAddr.QuadPart = CSP_BASE_REG_PA_SAIF0;
    pv_HWregSAIF0 = (PVOID) MmMapIoSpace(phyAddr, 0x1000, FALSE);
    if (pv_HWregSAIF0 == NULL)
    {
        ERRMSG("MmMapIoSpace failed for pv_HWregSAIF0");
        goto exit;
    }

    phyAddr.QuadPart = CSP_BASE_REG_PA_SAIF1;
    pv_HWregSAIF1= (PVOID) MmMapIoSpace(phyAddr, 0x1000, FALSE);
    if (pv_HWregSAIF1 == NULL)
    {
        ERRMSG("MmMapIoSpace failed for pv_HWregSAIF1");
        goto exit;
    }

    // Initialize the Ouput DMA chanel
    InitOutputDMA();

    // Initialize the Input DMA chanel
    InitInputDMA();

    // Initialize audio Codec
    if (!BSPAudioInitCodec())
    {
        ERRMSG("Initialize Audio Codec failed");
        goto exit;
    }

    // Setup dma_pages
    m_DMAOutPageVirtAddr[BUFFER_A] = m_DMAOutBufVirtAddr;
    m_DMAOutPageVirtAddr[BUFFER_B] = m_DMAOutBufVirtAddr + m_OutputDMAPageSize;

    m_DMAInPageVirtAddr[BUFFER_A] = m_DMAInBufVirtAddr;
    m_DMAInPageVirtAddr[BUFFER_B] = m_DMAInBufVirtAddr + m_InputDMAPageSize;

    // Initialize the mixer controls in mixerdrv.cpp
    InitMixerControls();

    // Note that the audio driver interrupt handler will only run if
    // m_Initialized is TRUE so we must initialize the state variable
    // here before trying to create the IST thread.
    m_Initialized = TRUE;

    // Initialize interrupt thread
    if (!InitInterruptThread())
    {
        ERRMSG("Failed to initialize output interrupt thread");
        goto exit;
    }


    if (!InitDisableDelayThread())
    {
        ERRMSG("Failed to initialize disable delay thread");
        goto exit;
    }

    // Initialize interrupt thread for Headphone detect if supported
    if(BSPIsHeadphoneDetectSupported())
    {
        if (!InitHPDetectInterruptThread())
        {
            ERRMSG("Failed to initialize headphone detect interrupt thread");
            goto exit;
        }
    }
    
exit:

    return m_Initialized;
}


//-----------------------------------------------------------------------------
//
//  Function: Deinit
//
//  Deinitializest the hardware: disables DMA channel(s), clears any pending interrupts, 
//  powers down the audio codec chip, etc.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//----------------------------------------------------------------------------
BOOL HardwareContext::Deinit()
{
    m_Initialized = FALSE;

    // Wake up the threads so that they can clean up
    // and then terminate (now that m_Initialize is FALSE).
    if (m_hStopOutputDMAEvent != NULL)
    {
        SetEvent(m_hStopOutputDMAEvent);
    }
    if (m_hAudioIntrEvent != NULL)
    {
        SetEvent(m_hAudioIntrEvent);
    }

    // Free DMA buffers
    BSPDeallocOutputDMABuffer(m_DMAOutBufVirtAddr);
    BSPDeallocInputDMABuffer(m_DMAInBufVirtAddr);
    // Free DMA descriptor buffers
    BSPDeallocOutputDMADescriptor((PBYTE)m_DMAOutDescriptorVirtAddr);
    BSPDeallocInputDMADescriptor((PBYTE)m_DMAINDescriptorVirtAddr);

    // Deinit audio codec
    BSPAudioDeinitCodec();

    // Unmap audio peripherals
    if (pv_HWregSAIF0)
    {
        MmUnmapIoSpace(pv_HWregSAIF0, 0x1000);
        pv_HWregSAIF0 = NULL;
    }

    if (pv_HWregSAIF1)
    {
        MmUnmapIoSpace(pv_HWregSAIF1,0x1000);
        pv_HWregSAIF1 = NULL;
    }

    return TRUE;
}
//--------------------------------------------------------------------------
//
//  Name: InitDMAChannel
//
//  Description: Intialize all the things about a DMA channel
//
//  Parameters: 
//      None.
//
//  Returns: 
//      None.
//
//--------------------------------------------------------------------------
BOOL HardwareContext::InitOutputDMA()
{
    //UINT8 sampleSize = BITSPERSAMPLE;
    UINT16 dmaBufferSize;

    // Allocate a block of virtual memory (physically contiguous) for the
    // DMA buffers. Note that a platform-specific API call is used here to
    // allow for the DMA buffers to be allocated from different memory regions
    // (e.g., internal vs. external memory).
    dmaBufferSize = BSPAllocOutputDMABuffer(&m_DMAOutBufVirtAddr, &m_DMAOutBufPhysAddr);

    if ((m_DMAOutBufVirtAddr == NULL)||(dmaBufferSize == 0))
    {
        ERRMSG("InitOutputDMA: DMA buffer page allocation failed");
        return FALSE;
    }

    m_OutputDMAPageSize = dmaBufferSize / AUDIO_DMA_NUMBER_PAGES;

    // Allocate a block of virtual memory (physically contiguos) for the
    // DMA OUTPut Desriptor.
    if(0 == BSPAllocOutputDMADescriptor(&m_DMAOutDescriptorVirtAddr, &m_DMAOutDesPhysAddr))
    {
        ERRMSG("InitOutputDMA: DMA Descriptor allocation failed");
        return FALSE;
    }
    if(m_DMAOutDescriptorVirtAddr == NULL)
    {
        ERRMSG("InitOutputDMA: DMA Descriptor allocation failed");
        return FALSE;
    }

    // Setup the DMA Descriptor
    m_DMAOutDescriptorPhysAddr = (PAUDIO_DMA_DESCRIPTOR)m_DMAOutDesPhysAddr.LowPart;
    if(!BSPSetupOutputDMADescriptor())
    {
        ERRMSG("InitOutputDMA: Setup DMA Descriptor failed");
    }
    // disable the Audio interrupts
    DDKApbxDmaEnableCommandCmpltIrq(APBX_CHANNEL_SAIF0,FALSE);

    // Put the DMA in RESET State
    DDKApbxDmaResetChan(APBX_CHANNEL_SAIF0,TRUE);

    // If no record device, request systintr here, otherwise
    // it should be done in InitInputDMA(), combining both
    // output and input DMA channel IRQs to request one systintr.
    // Request sysintr
    UINT32 dmaIrq = IRQ_SAIF0_DMA;
    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR,
                             &dmaIrq,
                             sizeof(UINT32),
                             &m_AudioSysintr,
                             sizeof(DWORD),
                             NULL))
    {
        ERRMSG("InitOutputDMA: Sysintr request failed");
        return FALSE;
    }
    
    return TRUE;
}

//--------------------------------------------------------------------------
//
//  Name: InitDMAChannel
//
//  Description: Intialize all the things about a DMA channel
//
//  Parameters: 
//      None.
//
//  Returns:
//      None.
//
//--------------------------------------------------------------------------
BOOL HardwareContext::InitInputDMA()
{
    // Stub out if no record device
    if (m_NumInputDevices == 0)
    {
        return TRUE;
    }

    //UINT8 sampleSize = BITSPERSAMPLE;
    UINT16 dmaBufferSize;

    // Allocate a block of virtual memory (physically contiguous) for the
    // DMA buffers. Note that a platform-specific API call is used here to
    // allow for the DMA buffers to be allocated from different memory regions
    // (e.g., internal vs. external memory).
    dmaBufferSize = BSPAllocInputDMABuffer(&m_DMAInBufVirtAddr, &m_DMAInBufPhysAddr);

    if (m_DMAInBufVirtAddr == NULL)
    {
        ERRMSG("InitInputDMA: DMA buffer page allocation failed");
        return FALSE;
    }

    m_InputDMAPageSize = dmaBufferSize / AUDIO_DMA_NUMBER_PAGES;

    // Allocate a block of virtual memory (physically contiguos) for the
    // DMA OUTPut Desriptor.
    BSPAllocInputDMADescriptor(&m_DMAINDescriptorVirtAddr, &m_DMAInDesPhysAddr);

    if(m_DMAINDescriptorVirtAddr == NULL)
    {
        ERRMSG("InitInputDMA: DMA Descriptor allocation failed");
        return FALSE;
    }

    // Setup the DMA Descriptor
    m_DMAInDescriptorPhysAddr = (PAUDIO_DMA_DESCRIPTOR)m_DMAInDesPhysAddr.LowPart;
    if(!BSPSetupInputDMADescriptor())
    {
        ERRMSG("InitInputDMA: Setup DMA Descriptor failed");
    }

    // disable the Audio interrupts
    DDKApbxDmaEnableCommandCmpltIrq(APBX_CHANNEL_SAIF1,FALSE);

    // Put the DMA in RESET State
    DDKApbxDmaResetChan(APBX_CHANNEL_SAIF1,TRUE);

    // Combine input and output DMA channel IRQs together
    // to request one systintr, so we can handle interrupts
    // from both channels in one thread.
    // Request sysintr
    UINT32 dmaIrq = IRQ_SAIF1_DMA;
    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR,
                             &dmaIrq,
                             sizeof(UINT32),
                             &m_AudioSysintr2,
                             sizeof(DWORD),
                             NULL))
    {
        ERRMSG("InitInputDMA: Sysintr request failed");
        return FALSE;
    }

    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function: InitInterruptThread
//
//  Initializes the IST for handling DMA interrupts.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//----------------------------------------------------------------------------
BOOL HardwareContext::InitInterruptThread()
{
    m_hAudioIntrEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!m_hAudioIntrEvent)
    {
        return FALSE;
    }

    if (!InterruptInitialize(m_AudioSysintr, m_hAudioIntrEvent, NULL, 0))
    {
        return FALSE;
    }

    m_hAudioIntrThread = CreateThread((LPSECURITY_ATTRIBUTES)NULL,
                                      0,
                                      (LPTHREAD_START_ROUTINE)CallInterruptThread,
                                      this,
                                      0,
                                      NULL);
    if (!m_hAudioIntrThread)
    {
        return FALSE;
    }

    // Bump up the priority since the interrupt must be serviced immediately.
    CeSetThreadPriority(m_hAudioIntrThread, GetInterruptThreadPriority());

    m_hAudioIntrEvent2 = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!m_hAudioIntrEvent2)
    {
        return FALSE;
    }

    if (!InterruptInitialize(m_AudioSysintr2, m_hAudioIntrEvent2, NULL, 0))
    {
        return FALSE;
    }

    m_hAudioIntrThread2 = CreateThread((LPSECURITY_ATTRIBUTES)NULL,
                                      0,
                                      (LPTHREAD_START_ROUTINE)CallInterruptThread2,
                                      this,
                                      0,
                                      NULL);
    if (!m_hAudioIntrThread2)
    {
        return FALSE;
    }

    // Bump up the priority since the interrupt must be serviced immediately.
    CeSetThreadPriority(m_hAudioIntrThread2, GetInterruptThreadPriority());

    return TRUE;
}


BOOL HardwareContext::InitDisableDelayThread()
{
    m_DACDelayDisableTimerID = NULL;
    m_ADCDelayDisableTimerID = NULL;
    m_hAudioDelayedDisableThread = NULL;

    // Create delayed disable event and thread
    if ((PlaybackDisableDelayMsec > 0) || (RecordDisableDelayMsec > 0))
    {
        m_hDACDelayDisableEvent  = CreateEvent(NULL, FALSE, FALSE, NULL);
        m_hADCDelayDisableEvent  = CreateEvent(NULL, FALSE, FALSE, NULL);
        m_hStopOutputDMAEvent    = CreateEvent(NULL, FALSE, FALSE, NULL);

        if ((m_hDACDelayDisableEvent == NULL) ||
            (m_hADCDelayDisableEvent == NULL) ||
            (m_hStopOutputDMAEvent == NULL))
        {
            ERRMSG("Unable to create delay events");
            return FALSE;
        }

        h_AudioDelayEvents[0] = m_hStopOutputDMAEvent;
        h_AudioDelayEvents[1] = m_hDACDelayDisableEvent;
        h_AudioDelayEvents[2] = m_hADCDelayDisableEvent;

        m_hAudioDelayedDisableThread =
            CreateThread((LPSECURITY_ATTRIBUTES)NULL,
                         0,
                         (LPTHREAD_START_ROUTINE)CallDisableDelayThread,
                         this,
                         0,
                         NULL);

        if (!m_hAudioDelayedDisableThread)
        {
            ERRMSG("Unable to create audio CODEC delayed disable timer event handling thread");
            return FALSE;
        }
    }

    return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function: StartCodec
//
//  This function configures the audio chip to start audio output.
//
//  Parameters:
//      bus
//          [in] Specifies the audio bus to be used for audio output.
//
//      path
//          [in] Specifies the audio path to be used for audio output.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void HardwareContext::StartCodec(AUDIO_BUS audioBus)
{
    if (audioBus == AUDIO_BUS_DAC)
    {
        BSPAudioStartDAC();
    }
    else // (audioBus == AUDIO_BUS_ADC)
    {
        BSPAudioStartADC();
    }
}


//-----------------------------------------------------------------------------
//
//  Function: BSPAudioStopCodecOutput
//
//  This function configures the audio chip to stop audio output.
//
//  Parameters:
//      bus
//          [in] Specifies the audio bus being used for audio output.
//
//      path
//          [in] Specifies the audio path being used for audio output.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void HardwareContext::StopCodec(AUDIO_BUS audioBus)
{
    UINT32 uDelayMsec;
    HANDLE *phDelayEvent;
    MMRESULT *phDelayTimerID;

    if (audioBus == AUDIO_BUS_DAC)
    {
        uDelayMsec     = PlaybackDisableDelayMsec;
        phDelayEvent   = &m_hDACDelayDisableEvent;
        phDelayTimerID = &m_DACDelayDisableTimerID;
    }
    else // (audioBus == AUDIO_BUS_ADC)
    {
        uDelayMsec     = RecordDisableDelayMsec;
        phDelayEvent   = &m_hADCDelayDisableEvent;
        phDelayTimerID = &m_ADCDelayDisableTimerID;
    }

    // Note that we want to immediately disable the Codec
    // if we're in the process of powering down.
    if ((uDelayMsec > 0) &&
        (m_hAudioDelayedDisableThread != NULL) &&
        (*phDelayEvent != NULL) &&
        !m_delayBypass)
    {
        if (*phDelayTimerID != NULL)
        {
            // Cancel previous timer so that we can restart
            // it later from the beginning.
            timeKillEvent(*phDelayTimerID);
            *phDelayTimerID = NULL;
        }

        // Start a timed delay. If the timer expires without us
        // starting another audio playback or record operation,
        // then we should really disable the audio CODEC hardware.
        *phDelayTimerID = timeSetEvent(uDelayMsec,
                                       1,
                                       (LPTIMECALLBACK)*phDelayEvent,
                                       NULL,
                                       TIME_ONESHOT | TIME_CALLBACK_EVENT_SET);
    }
    else
    {
        // We want to immediately disable the Codec
        if (audioBus == AUDIO_BUS_DAC)
        {
            BSPAudioStopDAC();
        }
        else // (audioBus == AUDIO_BUS_ADC)
        {
            BSPAudioStopADC();
        }
    }
}
//-----------------------------------------------------------------------------
//
//  Function: StartOutputDMA
//
//  This function starts outputting the sound data to the audio codec
//  chip via the DMA.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL HardwareContext::StartOutputDMA()
{
    ULONG nTransferred = 0;

    // Can not start output if loopback is working
    if (m_bCodecLoopbackState)
    {
        return FALSE;
    }

    if (!m_PowerUpState)
    {
        return FALSE;
    }

    // Must acquire critical section Lock() here so that we don't have
    // a race condition with StopOutputDMA() and the interrupt handler.
    Lock();

    // Terminate the existing DAC disable timer since we're about
    // to start another playback operation.
    if (m_DACDelayDisableTimerID != NULL)
    {
        timeKillEvent(m_DACDelayDisableTimerID);
        m_DACDelayDisableTimerID = NULL;
    }

    // Reinitialize audio output if it is not already active.
    if (!m_OutputDMARunning)
    {
        if(m_bFirstTime)
        {
            // Initialize output DMA state variables.
            m_OutputDMAStatus  = DMA_DONEA | DMA_DONEB;

            // Prime the output DMA buffers.
            nTransferred = TransferOutputBuffers();

            if(!DDKApbxDmaInitChan(APBX_CHANNEL_SAIF0,TRUE))
            {
                ERRMSG("StartOutputDMA: Unable to initialize output DMA channel\r\n");

                m_OutputDMARunning  = FALSE;
                m_bCanStopOutputDMA = TRUE;

                return FALSE;
            }

            // clear the Audio OUT interrupt
            DDKApbxDmaClearCommandCmpltIrq(APBX_CHANNEL_SAIF0);

            //enable the interrupt
            DDKApbxDmaEnableCommandCmpltIrq(APBX_CHANNEL_SAIF0, TRUE);

            // Start the output DMA channel. This must be done before we
            // start Codec because we have to make sure that audio
            // data is already available before that.
            DDKApbxStartDma(APBX_CHANNEL_SAIF0,(PVOID)m_DMAOutDescriptorPhysAddr, 2);
            m_EmptyBufer=0;
        }
        else
        {
            // clear the Audio OUT interrupt
            DDKApbxDmaClearCommandCmpltIrq(APBX_CHANNEL_SAIF0);

            // Enable the Audio OUT interrupt
            DDKApbxDmaEnableCommandCmpltIrq(APBX_CHANNEL_SAIF0, TRUE);

            m_EmptyBufer=0;
        }

    
        m_OutputDMARunning = TRUE;
        m_bFirstTime = FALSE;    

        StartCodec(AUDIO_BUS_DAC);

    }

    // This will be set to TRUE only by the audio driver IST at the
    // end of the playback operation.
    m_bCanStopOutputDMA = FALSE;

    Unlock();

    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function: StopOutputDMA
//
//  This function stops any DMA activity on the output channel.
//
//  This function can only be called after Lock() has already been called. A
//  matching Unlock() call must also be made after this function has finished.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
void HardwareContext::StopOutputDMA()
{
    Lock();

    // If the output DMA is running, stop it
    if (m_OutputDMARunning)
    {
        // Reset output DMA state variables
        m_OutputDMAStatus  = DMA_CLEAR;
        m_OutputDMARunning = FALSE;

        // Disable the interrupt
        DDKApbxDmaEnableCommandCmpltIrq(APBX_CHANNEL_SAIF0, FALSE);

        // clear the Audio OUT interrupt
        DDKApbxDmaClearCommandCmpltIrq(APBX_CHANNEL_SAIF0);

        // Disable audio output hardware
        StopCodec(AUDIO_BUS_DAC);

        // Kill the output DMA channel
        if (!m_PowerUpState) DDKApbxStopDma(APBX_CHANNEL_SAIF0);
    }

    Unlock();
}

//-----------------------------------------------------------------------------
//
//  Function:  StartInputDMA
//
//  This function starts inputting the sound data from the audio codec
//  chip via the DMA.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE .
//
//-----------------------------------------------------------------------------
BOOL HardwareContext::StartInputDMA()
{
    // Can not start output if loopback is working
    if (m_bCodecLoopbackState)
    {
        RETAILMSG(TRUE, (L"StartInputDMA: Recording can not work "
                         L"when loopback is working\r\n"));
        return FALSE;
    }

    if (!m_PowerUpState)
    {
        return FALSE;
    }

    // Must acquire critical section Lock() here so that we don't have a race
    // condition with StopInputDMA() and the interrupt handler.
    Lock();

    // Terminate the existing ADC disable timer since we're about
    // to start another recording operation.
    if (m_ADCDelayDisableTimerID != NULL)
    {
        timeKillEvent(m_ADCDelayDisableTimerID);
        m_ADCDelayDisableTimerID = NULL;
    }

    // Reinitialize audio input if it is not already active
    if(!m_InputDMARunning)
    {
        // Initialize the input DMA state variables
        m_InputDMARunning = TRUE;
        m_InputDMAStatus  &= (DWORD) ~(DMA_DONEA | DMA_DONEB);

        // Reset the state of the input DMA chain.
        //
        if(!DDKApbxDmaInitChan(APBX_CHANNEL_SAIF1,TRUE))
        {
            ERRMSG("StartInputDMA: Unable to initialize input DMA channel!\r\n");

            // Must reset the m_InputDMARunning state variable to FALSE
            // if we encounter an error here.
            m_InputDMARunning = FALSE;

            return FALSE;
        }

        DDKApbxDmaClearCommandCmpltIrq(APBX_CHANNEL_SAIF1);
        StartCodec(AUDIO_BUS_ADC);
        // Start the input DMA
        DDKApbxStartDma(APBX_CHANNEL_SAIF1,(PVOID)m_DMAInDescriptorPhysAddr, 2);
    }

    Unlock();

    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function:  StopInputDMA
//
//  This function stops any DMA activity on the input channel.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
void HardwareContext::StopInputDMA()
{
    Lock();

    // If the input DMA is running, stop it
    if (m_InputDMARunning)
    {
        // Reset input DMA state variables
        m_InputDMAStatus  = DMA_CLEAR;
        m_InputDMARunning = FALSE;

        // Disable audio input hardware
        StopCodec(AUDIO_BUS_ADC);

        DDKApbxDmaEnableCommandCmpltIrq(APBX_CHANNEL_SAIF1, FALSE);
        DDKApbxDmaClearCommandCmpltIrq(APBX_CHANNEL_SAIF1);
        // Kill the input DMA channel
        DDKApbxStopDma(APBX_CHANNEL_SAIF1);
        //BSPAudioStopADC();
    }

    Unlock();
}


//-----------------------------------------------------------------------------
//
//  Function: TransferOutputBuffer
//
//  Retrieves the next "mixed" audio buffer of data to DMA into the output channel.
//
//  Parameters:
//      None.
//
//  Returns:
//      Number of bytes needing to be transferred.
//
//----------------------------------------------------------------------------
ULONG HardwareContext::TransferOutputBuffers()
{
    ULONG BytesTransferred_A = 0;
    ULONG BytesTransferred_B = 0;

    if (m_OutputDMAStatus & DMA_DONEA)
    {
        // DMA buffer A is free, try to refill it.
        BytesTransferred_A = TransferBuffer(AUDIO_BUS_DAC, BUFFER_A);

        // Only mark the DMA buffer as being in use if some data was actually
        // transferred.
        if (BytesTransferred_A > 0)
        {
            m_OutputDMAStatus &= ~DMA_DONEA;
        }
    }

    if (m_OutputDMAStatus & DMA_DONEB)
    {
        // DMA buffer B is free, try to refill it.
        //
        // Note that we can refill both DMA buffers at once if both DMA
        // buffers are free and there is sufficient data.
        //
        BytesTransferred_B = TransferBuffer(AUDIO_BUS_DAC, BUFFER_B);

        if (BytesTransferred_B > 0)
        {
            m_OutputDMAStatus &= ~DMA_DONEB;
        }
    }

    return BytesTransferred_A + BytesTransferred_B;
}

ULONG HardwareContext::TransferInputBuffers(UINT8 checkFirst)
{
    ULONG BytesTransferred = 0;

    // We will mark the input DMA buffer as being in use again regardless
    // of whether or not the data was completely copied out to an
    // application-supplied data buffer. The input DMA buffer must be
    // immediately reused because the audio recording operation is still
    // running.
    //
    if (checkFirst == BUFFER_A)
    {
        if (m_InputDMAStatus & DMA_DONEA)
        {
            BytesTransferred = TransferBuffer(AUDIO_BUS_ADC, BUFFER_A);
            m_InputDMAStatus &= ~DMA_DONEA;
        }

        if (m_InputDMAStatus & DMA_DONEB)
        {
            BytesTransferred += TransferBuffer(AUDIO_BUS_ADC, BUFFER_B);
            m_InputDMAStatus &= ~DMA_DONEB;
        }
    }
    else // (checkFirst == IN_BUFFER_B)
    {
        if (m_InputDMAStatus & DMA_DONEB)
        {
            BytesTransferred = TransferBuffer(AUDIO_BUS_ADC, BUFFER_B);
            m_InputDMAStatus &= ~DMA_DONEB;
        }

        if (m_InputDMAStatus & DMA_DONEA)
        {
            BytesTransferred += TransferBuffer(AUDIO_BUS_ADC, BUFFER_A);
            m_InputDMAStatus &= ~DMA_DONEA;
        }
    }

    return BytesTransferred;
}

ULONG HardwareContext::TransferBuffer(AUDIO_BUS audioBus, UINT8 NumBuf)
{
    ULONG BytesTransferred = 0;

    PBYTE pBufferStart;
    PBYTE pBufferEnd;
    PBYTE pBufferLast;

    if (audioBus == AUDIO_BUS_DAC)
    {
        pBufferStart = m_DMAOutPageVirtAddr[NumBuf];
        pBufferEnd = pBufferStart + m_OutputDMAPageSize;

        TRANSFER_STATUS TransferStatus = {0};
        pBufferLast = m_OutputDeviceContext.TransferBuffer(pBufferStart,
                                                           pBufferEnd,
                                                           &TransferStatus);

        BytesTransferred = pBufferLast - pBufferStart;
        if(pBufferLast != pBufferEnd)
        {
            // Enable if you need to clear the rest of the DMA buffer
            //
            // Probably want to do something better than clearing out remaining
            // buffer samples. DC output by replicating last sample or
            // some type of fade out would be better.
            StreamContext::ClearBuffer(pBufferLast, pBufferEnd);
        }
    }
    else // (audioBus == AUDIO_BUS_ADC)
    {
        pBufferStart = m_DMAInPageVirtAddr[NumBuf];
        pBufferEnd = pBufferStart + m_InputDMAPageSize;

        pBufferLast = m_InputDeviceContext.TransferBuffer(pBufferStart, pBufferEnd, NULL);

        BytesTransferred = pBufferLast - pBufferStart;
    }

    return BytesTransferred;
}


void HardwareContext::InterruptThread()
{

    // Loop repeatedly to handle each audio interrupt event until we
    // deinitialize the audio driver.
    while (m_Initialized)
    {
        // Wait for the IST to be signaled by the OAL interrupt handler.
        WaitForSingleObject(m_hAudioIntrEvent, INFINITE);

        // Immediately terminate this thread if Deinit() has been called to
        // unload the audio driver. Otherwise, handle the audio interrupt
        // event.
        if (!m_Initialized)
        {
            break;
        }

        // Grab the audio driver lock to avoid race conditions with the
        // StartOutputDMA() and StopOutputDMA() functions.
        Lock();

        // Handle DMA output (i.e., audio playback) if it is active.
        if (m_OutputDMARunning)
        {
            OutputIntrRoutine();
        }

        // Handle DMA input (i.e., audio recording) if it is active.
        if (m_InputDMARunning)
        {
            InputIntrRoutine();

            // clear the Audio IN interrupt
            DDKApbxDmaClearCommandCmpltIrq(APBX_CHANNEL_SAIF1);
        }


        InterruptDone(m_AudioSysintr);

        Unlock();
    }

    // Close the event handle
    if (m_hAudioIntrEvent != NULL)
    {
        CloseHandle(m_hAudioIntrEvent);
        m_hAudioIntrEvent = NULL;
    }
}

void HardwareContext::InterruptThread2()
{
    // Loop repeatedly to handle each audio interrupt event until we
    // deinitialize the audio driver.
    while (m_Initialized)
    {
        // Wait for the IST to be signaled by the OAL interrupt handler.
        WaitForSingleObject(m_hAudioIntrEvent2, INFINITE);

        // Immediately terminate this thread if Deinit() has been called to
        // unload the audio driver. Otherwise, handle the audio interrupt
        // event.
        if (!m_Initialized)
        {
            break;
        }

        // Grab the audio driver lock to avoid race conditions with the
        // StartOutputDMA() and StopOutputDMA() functions.
        Lock();

         // Handle DMA input (i.e., audio recording) if it is active.
        if (m_InputDMARunning)
        {
            InputIntrRoutine();

            // clear the Audio IN interrupt
            DDKApbxDmaClearCommandCmpltIrq(APBX_CHANNEL_SAIF1);
        }
        InterruptDone(m_AudioSysintr2);

        Unlock();
    }

    // Close the event handle
    if (m_hAudioIntrEvent2 != NULL)
    {
        CloseHandle(m_hAudioIntrEvent2);
        m_hAudioIntrEvent2 = NULL;
    }
}

void HardwareContext::HPDetectInterruptThread()
{
    // While the audio driver is initialized, wait for the HP detect
    // event to occur
    while (m_Initialized)
    {
        // Wait for the IST to be signaled by the OAL interrupt handler.
        WaitForSingleObject(m_hAudioHPDetectIntrEvent, INFINITE);

        // Immediately terminate this thread if Deinit() has been called to
        // unload the audio driver. Otherwise, handle the audio interrupt
        // event.
        if (!m_Initialized)
        {
            // Signal interrupt done just in case an interrupt actually
            // happened in the mean time
            InterruptDone(m_AudioHPDetectSysIntr);
            break;
        }

        // Resolve the HP detect event.  The HP may be plugged/unplugged.
        // This needs to be resolved and appropriate action needs to be taken.
        BSPAudioHandleHPDetectIRQ();

        // Signal the kernel that interrupt processing is done and re-enable
        // for next interrupt
        InterruptDone(m_AudioHPDetectSysIntr);
    }

    // Close the event handle
    if (m_hAudioHPDetectIntrEvent != NULL)
    {
        CloseHandle(m_hAudioHPDetectIntrEvent);
        m_hAudioHPDetectIntrEvent = NULL;
    }
}

void HardwareContext::OutputIntrRoutine()
{
    UINT32 nTransferred;
    if(DDKApbxDmaGetActiveIrq(APBX_CHANNEL_SAIF0)){
    }else{
        return;
    }

    // Set DMA status bits according to retrieved transfer status
    if(DDKApbxGetNextCMDAR(APBX_CHANNEL_SAIF0) == (DWORD)&m_DMAOutDescriptorPhysAddr[BUFFER_A])
    {
        // The data in DMA buffer A has been transmitted and the
        // buffer may now be reused.
        m_OutputDMAStatus |= DMA_DONEA;
    }
    if(DDKApbxGetNextCMDAR(APBX_CHANNEL_SAIF0) == (DWORD)&m_DMAOutDescriptorPhysAddr[BUFFER_B])
    {
        // The data in DMA buffer B has been transmitted and the
        // buffer may now be reused.
        m_OutputDMAStatus |= DMA_DONEB;
    }

    // Try to refill any empty output DMA buffers with new data.
    nTransferred = TransferOutputBuffers();

    // Don't restart the DMA unless we actually have some data
    // in at least one of the DMA buffers.
    if (nTransferred == 0)
    {
        m_EmptyBufer++;
    }
    else
    {
        m_EmptyBufer = 0;
    }
    // clear the Audio OUT interrupt
    DDKApbxDmaClearCommandCmpltIrq(APBX_CHANNEL_SAIF0);
    DDKApbxDmaClearErrorIrq(APBX_CHANNEL_SAIF0);

    if(m_EmptyBufer >= 5)
    {
        // The upper audio driver layer has no more data, so we can
        // terminate the current DMA operation because all DMA
        // buffers are empty.
        //
        // Note that we signal the DisableDelayThread() to actually
        // make the call to StopOutputDMA() so that we can ensure that
        // this IST thread will never have it's priority changed.
        //
        // Currently the MMTIMER library can temporarily change the
        // priority of a calling thread and StopOutputDMA() does
        // make calls to the MMTIMER library. Therefore, we do not
        // want this IST thread making any direct MMTIMER calls.
        //
        m_bCanStopOutputDMA = TRUE;
        SetEvent(m_hStopOutputDMAEvent);
    }
}


void HardwareContext::InputIntrRoutine()
{
    static UINT8 checkFirst = BUFFER_A;
    UINT32 nTransferred;

    if(DDKApbxDmaGetActiveIrq(APBX_CHANNEL_SAIF1)){
    }else{
        return;
    }
    // Set DMA status bits according to retrieved transfer status
    if(DDKApbxGetNextCMDAR(APBX_CHANNEL_SAIF1) == (DWORD)&m_DMAInDescriptorPhysAddr[BUFFER_A])
    {
        // Mark input buffer A as having new audio data.
        m_InputDMAStatus |= DMA_DONEA;
    }
    if(DDKApbxGetNextCMDAR(APBX_CHANNEL_SAIF1) == (DWORD)&m_DMAInDescriptorPhysAddr[BUFFER_B])
    {
        //  Mark input buffer B as having new audio data.
        m_InputDMAStatus |= DMA_DONEB;
    }

    if ((m_InputDMAStatus & (DMA_DONEA | DMA_DONEB)) == 0)
    {
        // If running here, we must encounter a dummy interrupt,
        // of which the data has been transferred within the previous
        // interrupt handling that services another  input buffer.
        // So we want to skip the data tranfer below.
        return;
    }

    // Try to empty out the input DMA buffers by copying the data
    // up to the application-supplied data buffers.
    //
    // Note that we require the higher-level audio application
    // to provide enough data buffers to transfer all of the new
    // data from the input DMA buffers. Otherwise, we will be
    // forced to discard the data and just continue recording.
    //
    nTransferred = TransferInputBuffers(checkFirst);

    if (nTransferred > 0)
    {
        // The same logic can be applied for the cases where the
        // checkFirst flag is equal to BUFFER_B.
        //
        if ((checkFirst == BUFFER_A) &&
            (m_InputDMAStatus & DMA_DONEA) &&
            !(m_InputDMAStatus & DMA_DONEB))
        {
            // Input DMA buffer A was full and just transferred but
            // input DMA buffer B was not yet full so we must check
            // it first when handling the next input DMA interrupt.
            checkFirst = BUFFER_B;
        }
        else if ((checkFirst == BUFFER_B) &&
                 (m_InputDMAStatus & DMA_DONEB) &&
                 !(m_InputDMAStatus & DMA_DONEA))
        {
            // Input DMA buffer B was full and just transferred but
            // input DMA buffer A was not yet full so we must check
            // it first when handling the next input DMA interrupt.
            checkFirst = BUFFER_A;
        }
    }
    else
    {
        StopInputDMA();
    }
}


void HardwareContext::DisableDelayThread()
{
    // We want to loop endlessly here just waiting to process timer timeout
    // events that were created by calling timeSetEvent() in StopCodec().
    while (m_Initialized)
    {
        DWORD index = WaitForMultipleObjects(AUDIO_DELAY_EVENTS,
                                             h_AudioDelayEvents,
                                             FALSE,
                                             INFINITE);

        // Immediately terminate this thread if Deinit() has been called to
        // unload the audio driver. Otherwise, handle the appropriate event.
        if (!m_Initialized)
        {
            break;
        }
        else if (index == WAIT_OBJECT_0)
        {
            // The audio IST has signalled us to terminate the current
            // playback operation.
            Lock();

            // Don't call StopOutputDMA() if a new playback operation has
            // been started in the meantime.
            if (m_bCanStopOutputDMA)
            {
                StopOutputDMA();
                m_bCanStopOutputDMA = FALSE;
            }

            Unlock();
        }
        else if (index == (WAIT_OBJECT_0 + 1))
        {
            // Must first acquire the critical section to avoid race conditions
            // with StartCodec() and the interrupt handler.
            Lock();

            if (!m_OutputDMARunning)
            {
                // The timer has expired without another output audio stream
                // being activated so we can really disable the DAC now.
                BSPAudioStopDAC();
            }
            else
            {
                // Another audio output operation has been started,
                // so we should skip disabling the DAC.
                DEBUGMSG(ZONE_TEST, (_T("Skip disabling the DAC\r\n")));
            }

            m_DACDelayDisableTimerID = NULL;

            Unlock();
        }
        else if (index == (WAIT_OBJECT_0 + 2))
        {
            // Must first acquire the critical section to avoid race conditions
            // with StartCodec() and the interrupt handler.
            Lock();

            if (!m_InputDMARunning)
            {
                // The timer has expired without another input audio stream
                // being activated so we can really disable the ADC now.
                BSPAudioStopADC();
            }
            else
            {
                // Another audio input operation has been started,
                // so we should skip disabling the ADC.
                DEBUGMSG(ZONE_TEST, (_T("Skip disabling the ADC\r\n")));
            }

            m_ADCDelayDisableTimerID = NULL;

            Unlock();
        }
    }

    // Close all delayed event handles
    if (m_hStopOutputDMAEvent != NULL)
    {
        CloseHandle(m_hStopOutputDMAEvent);
        m_hStopOutputDMAEvent = NULL;
    }

    if (m_hDACDelayDisableEvent!= NULL)
    {
        CloseHandle(m_hDACDelayDisableEvent);
        m_hDACDelayDisableEvent = NULL;
    }

    if (m_hADCDelayDisableEvent!= NULL)
    {
        CloseHandle(m_hADCDelayDisableEvent);
        m_hADCDelayDisableEvent = NULL;
    }
}

//-----------------------------------------------------------------------------
//
//  Function: InitHPDetectInterruptThread
//
//  Initializes the IST for handling Headphone detect interrupt (via GPIO)
//  detect
//
//  Parameters:
//      None
//
//  Returns:
//      Boolean indicating success
//
//-----------------------------------------------------------------------------
BOOL HardwareContext::InitHPDetectInterruptThread()
{
    // Request sysintr
     UINT32 HPDetectIrq = BSPGetHPDetectIRQ();
     if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR,
                          &HPDetectIrq,
                          sizeof(UINT32),
                          &m_AudioHPDetectSysIntr,
                          sizeof(DWORD),
                          NULL))
     {
         ERRMSG("InitHPDetectInterruptThread: Sysintr request failed");
         return FALSE;
     } 
    
     // Create a synchronization event object
     m_hAudioHPDetectIntrEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
     if (!m_hAudioHPDetectIntrEvent)
     {
         return FALSE;
     }
    
     // Associate the event to the system interrupt
     if (!InterruptInitialize(m_AudioHPDetectSysIntr, m_hAudioHPDetectIntrEvent, NULL, 0))
     {
         return FALSE;
     }
    
     // Create a separate thread which will be signalled on HP detect/remove event
     m_hAudioHPDetectIntrThread = CreateThread((LPSECURITY_ATTRIBUTES)NULL,
                                        0,
                                        (LPTHREAD_START_ROUTINE)CallHPDetectInterruptThread,
                                        this,
                                        0,
                                        NULL);
     if (!m_hAudioHPDetectIntrThread)
     {
         return FALSE;
     }

    return TRUE;
}

void CallInterruptThread(HardwareContext *pHWContext)
{
    pHWContext->InterruptThread();
}

void CallInterruptThread2(HardwareContext *pHWContext)
{
    pHWContext->InterruptThread2();
}


void CallDisableDelayThread(HardwareContext *pHWContext)
{
    pHWContext->DisableDelayThread();
}

void CallHPDetectInterruptThread(HardwareContext *pHWContext)
{
    pHWContext->HPDetectInterruptThread();
}


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
//
// Gain control functions
//
/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
#define SW_GAIN     0
inline void HardwareContext::UpdateOutputGain()
{
#if (SW_GAIN)
    m_OutputDeviceContext.SetGain(m_fOutputMute ? 0 : m_dwOutputGain);
#else
    BSPAudioSetOutputGain(m_fOutputMute ? 0 : m_dwOutputGain);
#endif
}

inline void HardwareContext::UpdateInputGain()
{
#if (SW_GAIN)
    m_InputDeviceContext.SetGain(m_fInputMute ? 0 : m_dwInputGain);
#else
    BSPAudioSetInputGain(m_fInputMute ? 0 : m_dwInputGain);
#endif
}

MMRESULT HardwareContext::SetOutputGain (DWORD dwGain)
{
    m_dwOutputGain = dwGain;
    UpdateOutputGain();
    return MMSYSERR_NOERROR;
}

MMRESULT HardwareContext::SetOutputMute (BOOL fMute)
{
    m_fOutputMute = fMute;
    UpdateOutputGain();
    return MMSYSERR_NOERROR;
}

DWORD HardwareContext::GetOutputGain (void)
{
    return m_dwOutputGain;
}

BOOL HardwareContext::GetOutputMute (void)
{
    return m_fOutputMute;
}

BOOL HardwareContext::GetInputMute (void)
{
    return m_fInputMute;
}

MMRESULT HardwareContext::SetInputMute (BOOL fMute)
{
    m_fInputMute = fMute;
    UpdateInputGain();
    return MMSYSERR_NOERROR;
}

DWORD HardwareContext::GetInputGain (void)
{
    return m_dwInputGain;
}

MMRESULT HardwareContext::SetInputGain (DWORD dwGain)
{
    m_dwInputGain = dwGain;
    UpdateInputGain();
    return MMSYSERR_NOERROR;
}

// Control the hardware speaker enable
void HardwareContext::SetSpeakerEnable(BOOL bEnable)
{
    BSPSetSpeakerEnable(bEnable);

    return;
}

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// RecalcSpeakerEnable decides whether to enable the speaker or not.
// For now, it only looks at the m_bForceSpeaker variable, but it could
// also look at whether the headset is plugged in
// and/or whether we're in a voice call. Some mechanism would
// need to be implemented to inform the wave driver of changes in the state of
// these variables however.
/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8

void HardwareContext::RecalcSpeakerEnable()
{
    SetSpeakerEnable(m_NumForcedSpeaker);
}

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// SetForceSpeaker is called from the device context to update the state of the
// m_bForceSpeaker variable.
/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8

DWORD HardwareContext::ForceSpeaker( BOOL bForceSpeaker )
{
    // If m_NumForcedSpeaker is non-zero, audio should be routed to an
    // external speaker (if hw permits).
    if (bForceSpeaker)
    {
        m_NumForcedSpeaker++;
        if (m_NumForcedSpeaker==1)
        {
            RecalcSpeakerEnable();
        }else   m_NumForcedSpeaker--;
    }
    else
    {
        m_NumForcedSpeaker--;
        if (m_NumForcedSpeaker==0)
        {
            RecalcSpeakerEnable();
        }else   m_NumForcedSpeaker++;
    }

    return MMSYSERR_NOERROR;
}

//------------------------------------------------------------------------------
//
//  Function: PmControlMessage
//
//  Power management routine.
//
/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8

BOOL
HardwareContext::PmControlMessage (
    DWORD dwCode,
    PBYTE pBufIn,
    DWORD dwLenIn,
    PBYTE pBufOut,
    DWORD dwLenOut,
    PDWORD pdwActualOut)
{
    UNREFERENCED_PARAMETER(pBufIn);
    UNREFERENCED_PARAMETER(dwLenIn);

    BOOL bRetVal = FALSE;

    switch (dwCode)
    {
    // Return device specific power capabilities.
    case IOCTL_POWER_CAPABILITIES:
    {
        PPOWER_CAPABILITIES ppc = (PPOWER_CAPABILITIES) pBufOut;

        // Check arguments.
        if ( ppc && (dwLenOut >= sizeof(POWER_CAPABILITIES)) && pdwActualOut )
        {
            // Clear capabilities structure.
            memset( ppc, 0, sizeof(POWER_CAPABILITIES) );

            // Set power capabilities. Supports D0 and D4.
            ppc->DeviceDx = DX_MASK(D0)|DX_MASK(D4);

            DEBUGMSG(ZONE_FUNCTION, (TEXT("WAVE: IOCTL_POWER_CAPABILITIES = 0x%x\r\n"), ppc->DeviceDx));

            // Update returned data size.
            *pdwActualOut = sizeof(*ppc);
            bRetVal = TRUE;
        }
        else
        {
            DEBUGMSG(ZONE_ERROR, ( TEXT( "WAVE: Invalid parameter.\r\n" ) ) );
        }
        break;
    }

    // Indicate if the device is ready to enter a new device power state.
    case IOCTL_POWER_QUERY:
    {
        PCEDEVICE_POWER_STATE pDxState = (PCEDEVICE_POWER_STATE)pBufOut;

        // Check arguments.
        if (pDxState && (dwLenOut >= sizeof(CEDEVICE_POWER_STATE)) && pdwActualOut)
        {
            DEBUGMSG(ZONE_FUNCTION, (TEXT("WAVE: IOCTL_POWER_QUERY = %d\r\n"), *pDxState));

            // Check for any valid power state.
            if (VALID_DX (*pDxState))
            {
                *pdwActualOut = sizeof(CEDEVICE_POWER_STATE);
                bRetVal = TRUE;
            }
            else
            {
                DEBUGMSG(ZONE_ERROR, ( TEXT( "WAVE: IOCTL_POWER_QUERY invalid power state.\r\n" ) ) );
            }
        }
        else
        {
            DEBUGMSG(ZONE_ERROR, ( TEXT( "WAVE: IOCTL_POWER_QUERY invalid parameter.\r\n" ) ) );
        }
        break;
    }

    // Request a change from one device power state to another.
    case IOCTL_POWER_SET:
    {
        PCEDEVICE_POWER_STATE pDxState = (PCEDEVICE_POWER_STATE)pBufOut;

        // Check arguments.
        if (pDxState && (dwLenOut >= sizeof(CEDEVICE_POWER_STATE)))
        {
            DEBUGMSG(ZONE_FUNCTION, ( TEXT( "WAVE: IOCTL_POWER_SET = %d.\r\n"), *pDxState));

            // Check for any valid power state.
            if (VALID_DX(*pDxState))
            {
                Lock();

                // Power off.
                if ( *pDxState == D4 )
                {
                    PowerDown();
                }
                // Power on.
                else
                {
                    PowerUp();
                }

                m_DxState = *pDxState;

                Unlock();

                bRetVal = TRUE;
            }
            else
            {
                DEBUGMSG(ZONE_ERROR, ( TEXT( "WAVE: IOCTL_POWER_SET invalid power state.\r\n" ) ) );
            }
        }
        else
        {
            DEBUGMSG(ZONE_ERROR, ( TEXT( "WAVE: IOCTL_POWER_SET invalid parameter.\r\n" ) ) );
        }

        break;
    }

    // Return the current device power state.
    case IOCTL_POWER_GET:
    {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("WAVE: IOCTL_POWER_GET -- not supported!\r\n")));
        break;
    }

    default:
        DEBUGMSG(ZONE_WARN, (TEXT("WAVE: Unknown IOCTL_xxx(0x%0.8X) \r\n"), dwCode));
        break;
    }

    return bRetVal;
}

//-----------------------------------------------------------------------------
//
//  Function:  PowerUp
//
//  This function powers up the audio codec chip.  Note that the audio CODEC
//  chip is ONLY powered up when the user wishes to play or record.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
void HardwareContext::PowerUp()
{
    BSPAudioPowerUp(TRUE);

    m_bFirstTime = TRUE;
    m_PowerUpState = TRUE;

    // Restart the audio I/O operations that were suspended when PowerDown()
    // was called.
    if (g_saveOutputDMARunning)
    {
        StartOutputDMA();
    }

    if (g_saveInputDMARunning)
    {
        StartInputDMA();
    }

    m_delayBypass = FALSE;
}


//-----------------------------------------------------------------------------
//
//  Function:  PowerDown
//
//  This function powers down the audio codec chip.  If the input/output
//  channels are muted, then this function powers down the appropriate
//  components of the audio codec chip in order to conserve battery power.
//
//  The PowerDown() method will power down the entire audio chip while the
//  PowerDown(const DWORD channel) allows for the independent powerdown of
//  the input and/or output components to allow a finer degree of control.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void HardwareContext::PowerDown()
{
    // Note that for this special case, we do not need to grab the critical
    // section via Lock() and Unlock() before calling StopOutputDMA() and
    // StopInputDMA(). The powerdown thread runs at a high enough priority
    // that all other threads will be blocked while it runs. Having the
    // Lock() and Unlock() calls here may actually cause potential race
    // condition problems during the shutdown process.
    //
    // Furthermore, we will not resume operation until the hardware is powered
    // up again. At that point the hardware and audio driver state will be
    // reinitialized again. So at this point, we can just go ahead and shut
    // everything down.

    // We need cancel the existing DAC/ADC disable timer and  immidiately 
    // stop DAC/ADC when power down. 
    if (m_DACDelayDisableTimerID != NULL)
    {
        timeKillEvent(m_DACDelayDisableTimerID);
        m_DACDelayDisableTimerID = NULL;
        BSPAudioStopDAC();
    }

    if (m_ADCDelayDisableTimerID != NULL)
    {
        timeKillEvent(m_ADCDelayDisableTimerID);
        m_ADCDelayDisableTimerID = NULL;
        BSPAudioStopADC();
    }

    // Set this flag to allow special handling for the powerdown procedure.
    // The most important thing we want to avoid when powering down is using
    // any sort of timer or other type of delay mechanism. We need to bypass
    // anything like that and simply shutdown the hardware components when
    // performing a powerdown operation.
    m_delayBypass = TRUE;

    // Save state for correct powerup handling.
    g_saveOutputDMARunning = m_OutputDMARunning;
    g_saveInputDMARunning  = m_InputDMARunning;
    m_PowerUpState = FALSE;

    // Request all active audio-related DMA channels to stop.
    StopOutputDMA();
    StopInputDMA();

    // Request audio devices to power down.
    BSPAudioPowerDown(TRUE);

}

DWORD HardwareContext::GetDriverRegValue(LPWSTR ValueName, DWORD DefaultValue)
{
    HKEY DevKey = OpenDeviceKey((LPWSTR)m_DriverIndex);

    if (DevKey)
    {
        DWORD ValueLength = sizeof(DWORD);
        RegQueryValueEx(
            DevKey,
            ValueName,
            NULL,
            NULL,
            (PUCHAR)&DefaultValue,
            &ValueLength);

        RegCloseKey(DevKey);
    }

    return DefaultValue;
}

void HardwareContext::SetDriverRegValue(LPWSTR ValueName, DWORD Value)
{
    HKEY DevKey = OpenDeviceKey((LPWSTR)m_DriverIndex);

    if (DevKey)
    {
        RegSetValueEx(
            DevKey,
            ValueName,
            NULL,
            REG_DWORD,
            (PUCHAR)&Value,
            sizeof(DWORD)
            );

        RegCloseKey(DevKey);
    }

    return;
}

BOOL HardwareContext::IsSupportedOutputFormat(LPWAVEFORMATEX lpFormat)
{
    BOOL bRet = FALSE;

    // Stub out here
    UNREFERENCED_PARAMETER(lpFormat);

    return bRet;
}


/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
//
// Mixer support funcions
//
/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
BOOL HardwareContext::SelectInputSource(DWORD nIndex)
{
    Lock();

    // We do not want to change input source when record path is working
    if (m_InputDMARunning)
    {
        RETAILMSG(TRUE, (L"Can not change input source when "
                         L"recording is running\r\n"));
        return FALSE;
    }

    BSPAudioSelectADCSource(nIndex);

    Unlock();

    return TRUE;
}

BOOL HardwareContext::SetCodecLoopback(BOOL bEnable)
{
    BOOL bRet;

    Lock();

    // We need to stop output and input if they are running
    if (m_OutputDMARunning)
    {
        RETAILMSG(TRUE, (L"SetCodecLoopback: Playback is running\r\n"));

        // Stop output without delay
        m_delayBypass = TRUE;
        StopOutputDMA();
        m_delayBypass = FALSE;
    }

    if (m_InputDMARunning)
    {
        RETAILMSG(TRUE, (L"SetCodecLoopback: Recording is running\r\n"));

        // Stop input without delay
        m_delayBypass = TRUE;
        StopInputDMA();
        m_delayBypass = FALSE;
    }

    bRet = BSPAudioSetCodecLoopback(bEnable);

    // Save state if loopback was successfully set, so that StartOuputDMA()
    // and StartInputDMA() can check the state of loopback.
    if (bRet)
    {
        m_bCodecLoopbackState = bEnable;
    }

    Unlock();

    return bRet;
}

