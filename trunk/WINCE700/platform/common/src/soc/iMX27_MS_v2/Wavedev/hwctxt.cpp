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
//-----------------------------------------------------------------------------
//
//  Copyright (C) 2005-2006, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  hwctxt.cpp
//
//  Platform independent code for the WinCE WAV audio driver. This code's primary
//  responsibilities are:
//
//    1. Initialize the platform's audio hardware (e.g., audio CODEC,
//       amplifiers, etc.).
//    2. Initialize and configure all associated hardware components (e.g.,
//       DMA controllers, Synchronous Serial Interfaces (SSIs), audio MUXes,
//       interrupt controllers, etc.).
//    3. Configure DMA operations to move audio data between the memory buffers
//       and the SSIs.
//    4. Handle audio hardware-related interrupt events.
//    5. Configure the audio hardware to perform audio playback and recording
//       functions.
//    6. Provide smart power management to minimize power consumption (e.g.,
//       powering up components only as required and powering them down when
//       they are no longer needed). Note, however, that we don't do any
//       power management for components that are not directly audio-related.
//       For example, we will disable the clock signal for the SSIs when
//       we stop playing audio but we won't powerdown the clock/reset module
//       since that may still be needed by other parts of the system.
//
//  All other tasks (e.g., audio stream mixing, resampling etc.) are currently
//  handled by the "upper" layers of the WinCE audio driver.
//
//  This device driver implementation is based upon the audio features and
//  components provided by the Freescale Power Management ICs (PMICs) in
//  conjunction with either an i.MX or MXC-based platform.
//
//  More specifically, the following key platform-dependent hardware components
//  are used to provide the audio playback and recording functions:
//
//    PMIC Components:
//
//      * Stereo DAC for playback along with associated mixers, output
//        amplifiers, and audio output jacks.
//      * Voice CODEC for recording along with associated mixers, input
//        amplifiers, and audio microphone input jacks.
//
//    i.MX or MXC Components:
//
//      * SSI1 for Voice CODEC I/O
//      * SSI2 for Stereo DAC I/O
//      * DMA to handle audio I/O between memory and SSI1/SSI2.
//      * Audio MUX to route the audio stream between the SSI and the
//        PMIC Voice CODEC or Stereo DAC.
//      * Clock and Reset Module to generate the required clock signals for
//        SSI1/SSI2 (and the PMIC clock input in the case of the MC13783 PMIC).
//
//  See also the corresponding PMIC-specific source files for complete details
//  about the implementation of the BSP interface for audio. For example, in
//  the case of the iMX27 platform, the rest of the audio driver code can be
//  found in the following files:
//
//      WINCE700\PLATFORM\3DS_IMX27\SRC\DRIVERS\WAVEDEV\MC13783\HWCTXT.CPP
//      WINCE700\PLATFORM\COMMON\SRC\SOC\IMX27_MS_v2\PMIC\MC13783\...
//          ...\SDK\PMIC_AUDIO.CPP
//
//-----------------------------------------------------------------------------

#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#include <Mmsystem.h>   // For timer driver interface.
#include "wavemain.h"


//-----------------------------------------------------------------------------
// External Variables

#ifdef DEBUG

extern DBGPARAM dpCurSettings;

#endif // #ifdef DEBUG


//-----------------------------------------------------------------------------
// Defines
// Define the size (in data words) of the SSI TX FIFO. This is normally 8.

#define SSI_TX_FIFO_DEPTH               8 // Default 8

//-----------------------------------------------------------------------------
// Exported Global Variables


//-----------------------------------------------------------------------------
// Local Global Variables

const unsigned short HardwareContext::INTR_PRIORITY_REGKEY[12] =
    TEXT("Priority256");


PHYSICAL_ADDRESS g_PhysDMABufferAddr;
static BOOL g_saveOutputDMARunning = FALSE;

HardwareContext *g_pHWContext = NULL;

#ifdef AUDIO_RECORDING_ENABLED
static BOOL g_saveInputDMARunning  = FALSE;
#endif


//-----------------------------------------------------------------------------
//
//  Function: CreateHWContext
//
//  This function is invoked by the WAV_Init to create a device instance
//  that will be passed to the WAV_Open and WAV_DeInit functions.  If
//  multiple devices are supported, a unique device instance should be
//  returned for each device.
//
//  Parameters:
//      Index
//          [in] Context parameter passed to the WAV_Init function
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL HardwareContext::CreateHWContext(DWORD Index)
{
    DEBUGMSG(ZONE_FUNCTION,(_T("+HardwareContext::CreateHWContext\n")));

    if (g_pHWContext)
    {
        return(TRUE);
    }

    g_pHWContext = new HardwareContext;
    if (!g_pHWContext)
    {
        return(FALSE);
    }

    DEBUGMSG(ZONE_FUNCTION,(_T("-HardwareContext::CreateHWContext\n")));

    return(g_pHWContext->Init(Index));
}


//-----------------------------------------------------------------------------
//
//  Function: HardwareContext
//
//  This function is the constructor for the hardware context class.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
HardwareContext::HardwareContext()
#ifdef AUDIO_RECORDING_ENABLED
    : m_InputDeviceContext(), m_OutputDeviceContext()
#else
    : m_OutputDeviceContext()
#endif
{
    DEBUGMSG(ZONE_FUNCTION,(_T("+HardwareContext::HardwareContext\n")));

    InitializeCriticalSection(&m_Lock);
    m_Initialized = FALSE;

    // This flag is only set to TRUE when PowerDown() is called.
    m_audioPowerdown = FALSE;

    // The actual interrupt handler thread will be created later by calling
    // CreateThread() in InitInterruptThread().
    m_hAudioInterruptThread = NULL;

    // Initialize the timer ID and timeout event for both playback and
    // recording operations.
    //
    // We must create the events before we create the event handling thread
    // because the event handling thread will immediately begin to run once
    // it has been created.
    m_AudioDelayDisableTimerID[0] = NULL;
    m_hAudioDelayDisableEvent[0]  = CreateEvent(NULL, FALSE, FALSE, NULL);
#ifdef AUDIO_RECORDING_ENABLED
    m_AudioDelayDisableTimerID[1] = NULL;
    m_hAudioDelayDisableEvent[1]  = CreateEvent(NULL, FALSE, FALSE, NULL);
#endif

    // Also initialize the delayed disable timer event handling thread.
    // The actual thread will be created later by calling CreateThread()
    // in Init().
    m_hAudioDelayedDisableThread = NULL;

    m_TxBufIndex = 0;

    DEBUGMSG(ZONE_FUNCTION,(_T("-HardwareContext::HardwareContext\n")));
}


//-----------------------------------------------------------------------------
//
//  Function: ~HardwareContext
//
//  This function is the destructor for the hardware context class.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
HardwareContext::~HardwareContext()
{
    DEBUGMSG(ZONE_FUNCTION,(_T("+HardwareContext::~HardwareContext\n")));

    Lock();

    if (m_AudioDelayDisableTimerID[0] != NULL)
    {
        timeKillEvent(m_AudioDelayDisableTimerID[0]);
        m_AudioDelayDisableTimerID[0] = NULL;
    }
#ifdef AUDIO_RECORDING_ENABLED
    if (m_AudioDelayDisableTimerID[1] != NULL)
    {
        timeKillEvent(m_AudioDelayDisableTimerID[1]);
        m_AudioDelayDisableTimerID[1] = NULL;
    }
#endif

    Unlock();

    DeleteCriticalSection(&m_Lock);

    if (m_hAudioDelayDisableEvent[0] != NULL)
    {
        CloseHandle(m_hAudioDelayDisableEvent[0]);
        m_hAudioDelayDisableEvent[0] = NULL;
    }
#ifdef AUDIO_RECORDING_ENABLED
    if (m_hAudioDelayDisableEvent[1] != NULL)
    {
        CloseHandle(m_hAudioDelayDisableEvent[1]);
        m_hAudioDelayDisableEvent[1] = NULL;
    }
#endif

    DEBUGMSG(ZONE_FUNCTION,(_T("-HardwareContext::~HardwareContext\n")));
}


//-----------------------------------------------------------------------------
//
//  Function: Init
//
//  This function initializes the hardware context for the audio device.
//
//  Parameters:
//      Index
//          [in] Context parameter passed to the WAV_Init function
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL HardwareContext::Init(DWORD Index)
{
    DEBUGMSG(ZONE_FUNCTION,(_T("+HardwareContext::Init\n")));

    // If we have already initialized this device, return error
    if (m_Initialized)
    {
        return(FALSE);
    }

    // Initialize hardware context to be muted and zero gain since hardware
    // should currently be in a powered down state. The audio hardware will
    // be automatically powered up when audio playback/recording is actually
    // initiated.
    m_dwOutputGain  = 0;
    m_fOutputMute   = TRUE;
#ifdef AUDIO_RECORDING_ENABLED
    m_dwInputGain   = 0x0000;
    m_fInputMute    = TRUE;
#endif
    m_CodecPwrState = AUDIO_PWR_STATE_OFF;

    // Initialize the driver state/status variables
    m_dwDriverIndex      = Index;
    m_OutputDMARunning = FALSE;
    m_dwOutputDMAStatus  = DMA_CLEAR;
#ifdef AUDIO_RECORDING_ENABLED
    m_InputDMARunning  = FALSE;
    m_dwInputDMAStatus   = DMA_CLEAR;
#endif

    // Initialize BSP-specific configuration.
    if (!BSPAudioInit())
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("WAVEDEV.DLL:HardwareContext::Init() - ")
                              TEXT("BSPInit failed\r\n")));
        goto cleanUp;
    }

    // Map the DMA buffers.
    if (!MapDMABuffers())
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("WAVEDEV.DLL:HardwareContext::Init() - ")
                              TEXT("MapDMABuffers failed\r\n")));
        goto cleanUp;
    }

    // Initialize the output DMA.
    if (!InitOutputDMA())
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("WAVEDEV.DLL:HardwareContext::Init() - ")
                              TEXT("Failed to initialize output DMA.\r\n")));
        goto cleanUp;
    }

#ifdef AUDIO_RECORDING_ENABLED
    // Initialize the input DMA.
    if (!InitInputDMA())
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("WAVEDEV.DLL:HardwareContext::Init() - ")
                              TEXT("Failed to initialize input DMA.\r\n")));
        goto cleanUp;
    }
#endif

    // Initialize and create audio processing IST.
    if (!InitInterruptThread())
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("WAVEDEV.DLL:HardwareContext::Init() - ")
                  TEXT("Failed to initialize interrupt thread.\r\n")));
        goto cleanUp;
    }

    // Initialize and create delayed disable timer event handling thread.
    if ((m_hAudioDelayedDisableThread == NULL) &&
        (PLAYBACK_DISABLE_DELAY_MSEC > 0) || (RECORD_DISABLE_DELAY_MSEC > 0))
    {
        // Create a thread to handle the CODEC disable delay timer events.
        m_hAudioDelayedDisableThread =
            CreateThread((LPSECURITY_ATTRIBUTES)NULL,
                         0,
                         (LPTHREAD_START_ROUTINE)BSPAudioDisableDelayHandler,
                         this,
                         0,
                         NULL);

        if (!m_hAudioDelayedDisableThread)
        {
            ERRMSG("Unable to create audio CODEC delayed disable timer event "
                   "handling thread");
            goto cleanUp;
        }
    }

    m_Initialized = TRUE;

cleanUp:

    DEBUGMSG(ZONE_FUNCTION,(_T("-HardwareContext::Init\n")));

    return m_Initialized;
}


//-----------------------------------------------------------------------------
//
//  Function:  Deinit
//
//  This function deinitializes the hardware context for the audio device.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL HardwareContext::Deinit()
{
    DEBUGMSG(ZONE_FUNCTION,(_T("+HardwareContext::Deinit\n")));

    Lock();

#ifdef AUDIO_RECORDING_ENABLED
    // Terminate input (recording) DMA operation if it is currently running.

    StopInputDMA();

#endif


    StopOutputDMA();


    Unlock();

    // Unmap the audio control registers and DMA buffers.
    BSPAudioDeinit();

    UnmapDMABuffers();

    if (g_pHWContext)
    {
        delete g_pHWContext;
    }

    m_Initialized = FALSE;

    DEBUGMSG(ZONE_FUNCTION,(_T("-HardwareContext::Deinit\n")));

    return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function:  MapDMABuffers
//
//  This function maps the DMA buffers used for audio input/output.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL HardwareContext::MapDMABuffers()
{
    PBYTE pVirtDMABufferAddr = NULL;

    DEBUGMSG(ZONE_FUNCTION,(_T("+HardwareContext::MapDMABuffers\n")));

    // Allocate a block of virtual memory (physically contiguous) for the
    // DMA buffers. Note that a platform-specific API call is used here to
    // allow for the DMA buffers to be allocated from different memory regions
    // (e.g., internal vs. external memory).
    pVirtDMABufferAddr = BSPAudioAllocDMABuffers(&g_PhysDMABufferAddr);

    if (pVirtDMABufferAddr == NULL)
    {
        RETAILMSG(TRUE, (TEXT("WAVEDEV.DLL:HardwareContext::MapDMABuffers() - ")
                         TEXT("Failed to allocate DMA buffer.\r\n")));
        return(FALSE);
    }

    // Setup the DMA page pointers.
    //
    // NOTE: Currently, input and output each have two DMA pages: these pages
    //       are used in a round-robin fashion so that the OS can read/write
    //       one buffer while the audio codec chip read/writes the other buffer.
    //
    m_Output_pbDMA_PAGES[0] = pVirtDMABufferAddr;
    m_Output_pbDMA_PAGES[1] = pVirtDMABufferAddr + AUDIO_DMA_PAGE_SIZE;
#ifdef AUDIO_RECORDING_ENABLED
    m_Input_pbDMA_PAGES[0]  = pVirtDMABufferAddr + (2 * AUDIO_DMA_PAGE_SIZE);
    m_Input_pbDMA_PAGES[1]  = pVirtDMABufferAddr + (3 * AUDIO_DMA_PAGE_SIZE);
#endif

    // Return physical addresses for TX and RX buffer
    m_DmaTxBuffer[0] = (UINT32)g_PhysDMABufferAddr.QuadPart;
    m_DmaTxBuffer[1] = (UINT32)g_PhysDMABufferAddr.QuadPart + AUDIO_DMA_PAGE_SIZE;

    #ifdef AUDIO_RECORDING_ENABLED
    m_DmaRxBuffer[0] = (UINT32)g_PhysDMABufferAddr.QuadPart + (2 * AUDIO_DMA_PAGE_SIZE);
    m_DmaRxBuffer[1] = (UINT32)g_PhysDMABufferAddr.QuadPart + (3 * AUDIO_DMA_PAGE_SIZE);
    #endif

    DEBUGMSG(ZONE_FUNCTION,(_T("-HardwareContext::MapDMABuffers\n")));

    return(TRUE);

}


//-----------------------------------------------------------------------------
//
//  Function:  UnmapDMABuffers
//
//  This function unmaps the DMA buffers previously mapped with the
//  MapDMABuffers function.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL HardwareContext::UnmapDMABuffers()
{
   DEBUGMSG(ZONE_FUNCTION,(_T("+HardwareContext::UnmapDMABuffers\n")));

    if (m_Output_pbDMA_PAGES[0])
    {
        BSPAudioDeallocDMABuffers((PVOID)(m_Output_pbDMA_PAGES[0]));
        m_Output_pbDMA_PAGES[0] = NULL;
        m_Output_pbDMA_PAGES[1] = NULL;
#ifdef AUDIO_RECORDING_ENABLED
        m_Input_pbDMA_PAGES[0]  = NULL;
        m_Input_pbDMA_PAGES[1]  = NULL;
#endif
    }

    DEBUGMSG(ZONE_FUNCTION,(_T("-HardwareContext::UnmapDMABuffers\n")));

    return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function:  SetOutputGain
//
//  Parameters:
//      DWORD dwGain - new gain level
//
//  Returns:
//      Returns MMSYSERR_NOERROR if successful, otherwise returns
//      MMSYSERR_ERROR.
//
//-----------------------------------------------------------------------------
MMRESULT HardwareContext::SetOutputGain (DWORD dwGain)
{
    DEBUGMSG(ZONE_FUNCTION,(_T("+HardwareContext::SetOutputGain\n")));

    BSPAudioSetOutputGain(AUDIO_BUS_STEREO_OUT, dwGain);
    m_dwOutputGain = dwGain;

    DEBUGMSG(ZONE_FUNCTION,(_T("-HardwareContext::SetOutputGain\n")));

    return MMSYSERR_NOERROR;
}


//-----------------------------------------------------------------------------
//
//  Function:  SetOutputMute
//
//  Parameters:
//      BOOL  fmute - TRUE / FALSE to indicate mute or no mute
//
//  Returns:
//      Returns MMSYSERR_NOERROR if successful, otherwise returns
//      MMSYSERR_ERROR.
//
//-----------------------------------------------------------------------------

MMRESULT HardwareContext::SetOutputMute (BOOL fMute)
{
    // TODO: Call down to PMIC driver for codec mute configuration

    // Save output mute configuration
    m_fOutputMute = fMute;

    return MMSYSERR_NOERROR;
}

//-----------------------------------------------------------------------------
//
//  Function:  GetOutputMute
//  This function gets the mute state.
//  Parameters:
//     None.
//
//  Returns:
//      m_fOutputMute   - Indicates the mute state.
//
//-----------------------------------------------------------------------------
BOOL HardwareContext::GetOutputMute (void)
{
    return m_fOutputMute;
}

//-----------------------------------------------------------------------------
//
//  Function:  GetOutputGain
//  This function gets the Output Gain state.
//  Parameters:
//     None.
//
//  Returns:
//      m_dwOutputGain  - Indicates the gain state.
//
//-----------------------------------------------------------------------------

DWORD HardwareContext::GetOutputGain (void)
{
    return m_dwOutputGain;
}

#ifdef AUDIO_RECORDING_ENABLED
//-----------------------------------------------------------------------------
//
//  Function:  GetInputMute
//  This function gets the mute state.
//  Parameters:
//     None.
//
//  Returns:
//      m_fInputMute    - Indicates the mute state.
//
//-----------------------------------------------------------------------------

BOOL HardwareContext::GetInputMute (void)
{
    return m_fInputMute;
}

//-----------------------------------------------------------------------------
//
//  Function:  SetInputMute
//
//  Parameters:
//      DWORD fmute - TRUE / FALSE to indicate mute or no mute
//
//  Returns:
//      Returns MMSYSERR_NOERROR if successful, otherwise returns
//      MMSYSERR_ERROR.
//
//----------------------------------------------------------------------------
MMRESULT HardwareContext::SetInputMute (BOOL fMute)
{
    // TODO: Call down to PMIC driver for codec mute configuration

    // Save input mute configuration
    m_fInputMute = fMute;

    return m_InputDeviceContext.SetGain(fMute ? 0: m_dwInputGain);
}

//-----------------------------------------------------------------------------
//
//  Function:  GetInputGain
//  This function gets the Input Gain state.
//  Parameters:
//     None.
//
//  Returns:
//      m_dwInputGain   - Indicates the gain state.
//
//-----------------------------------------------------------------------------
DWORD HardwareContext::GetInputGain (void)
{
    return m_dwInputGain;
}


//-----------------------------------------------------------------------------
//
//  Function:  SetInputGain
//
//
//  Parameters:
//      DWORD dwGain    - Set the gain values
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
MMRESULT HardwareContext::SetInputGain (DWORD dwGain)
{
    DEBUGMSG(ZONE_FUNCTION,(_T("+HardwareContext::SetInputGain\n")));

    BSPAudioSetInputGain(AUDIO_BUS_VOICE_IN, dwGain);

    m_dwInputGain = dwGain;

    DEBUGMSG(ZONE_FUNCTION,(_T("-HardwareContext::SetInputGain\n")));

    return MMSYSERR_NOERROR;
}

#endif // #ifdef AUDIO_RECORDING_ENABLED

//-----------------------------------------------------------------------------
//
//  Function:  InitOutputDMA
//
//  This function initializes the DMA channel for output.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL HardwareContext::InitOutputDMA()
{
    BOOL rc = FALSE;
    //TODO: Replace the DMAC_TRANSFER_SIZE with appropriate macro name
    //when changed in mx27_ddk.h

    DEBUGMSG(ZONE_FUNCTION,(_T("+HardwareContext::InitOutputDMA\n")));

    // Confirm that DMA buffer has been allocated.
    if (!g_PhysDMABufferAddr.LowPart)
    {
        DEBUGMSG(ZONE_ERROR, (_T("Invalid DMA buffer physical address.\r\n")));
        goto cleanUp;
    }

    // Configure the platform-specific output DMA channel
    if (!BSPAudioInitOutput(AUDIO_BUS_STEREO_OUT, m_DmaTxBuffer[0], AUDIO_DMA_PAGE_SIZE ))
    {
        DEBUGMSG(ZONE_ERROR, (_T("BSPAudioInitOutput failed.\r\n")));
        goto cleanUp;
    }

    rc = TRUE;

cleanUp:
    if (!rc)
    {
        DeinitOutputDMA();
    }

    DEBUGMSG(ZONE_FUNCTION,(_T("-HardwareContext::InitOutputDMA\n")));

    return rc;
}


//-----------------------------------------------------------------------------
//
//  Function:  DeinitOutputDMA
//
//  This function deinitializes the DMA channel for output.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL HardwareContext::DeinitOutputDMA()
{
    DEBUGMSG(ZONE_FUNCTION,(_T("+HardwareContext::DeinitOutputDMA\n")));

    // Stop the audio output devices
    BSPAudioStopOutput(AUDIO_BUS_STEREO_OUT, AUDIO_PATH_HEADSET);

    DDKDmacStopChan(m_OutputDMAChan);
    DDKDmacReleaseChan(m_OutputDMAChan);
    DDKDmacClearChannelIntr(m_OutputDMAChan);

    DEBUGMSG(ZONE_FUNCTION,(_T("-HardwareContext::DeinitOutputDMA\n")));

    return TRUE;
}


//------------------------------------------------------------------------------
//
//  FUNCTION:       InterruptClear
//
//  DESCRIPTION:    Clears the SSI interrupt status by doing a
//                  status register read followed by a erad/write to
//                  the appropriate data fifo register
//
//  PARAMETERS:
//                  dir - transfer dir
//                  ch - SSI channel
//
//  RETURNS:
//                  NONE
//
//------------------------------------------------------------------------------
UINT32 HardwareContext::ClearInterruptStatus(SSI_TRANSFER_DIR dir, SSI_CHANNEL ch)
{
    PCSP_SSI_REG pStDACSSI = BSPAudioGetStDACSSI();
    UINT32 dummy;
    UINT32 sisr;
    UINT32 sfscr;
    UINT32 i;

    DEBUGMSG(1, (TEXT("SsiClass::ClearInterruptStatus+\r\n")));

    sisr = pStDACSSI->SISR;
    sfscr = pStDACSSI->SFCSR;
    DEBUGMSG(1, (TEXT("ClearInterruptStatus: SISR=0x%x\r\n"), sisr));

    if(ch == SSI_CHANNEL0)
    {
        if(dir == SSI_TRANSFER_TX)
        {

            if((sisr & CSP_BITFMASK(SSI_SISR_TFS)) != 0)
                for(i = 0; i < (8 - CSP_BITFEXT(sfscr, SSI_SFCSR_TFWM0)); i++)
                    pStDACSSI->STX0 = 0;
        }
        else
        {
            if((sisr & CSP_BITFMASK(SSI_SISR_RFS)) != 0)
                for(i = 0; i < (8 - CSP_BITFEXT(sfscr, SSI_SFCSR_RFWM0)); i++)
                    dummy = pStDACSSI->SRX0;
        }
    }
    else
    {
        if(dir == SSI_TRANSFER_TX)
        {
            if((sisr & CSP_BITFMASK(SSI_SISR_TFS)) != 0)
                for(i = 0; i < (8 - CSP_BITFEXT(sfscr, SSI_SFCSR_TFWM1)); i++)
                    pStDACSSI->STX1 = 0;
        }
        else
        {
            if((sisr & CSP_BITFMASK(SSI_SISR_RFS)) != 0)
                for(i = 0; i < (8 - CSP_BITFEXT(sfscr, SSI_SFCSR_RFWM1)); i++)
                    dummy = pStDACSSI->SRX1;
        }
    }

    DEBUGMSG(1, (TEXT("SsiClass::ClearInterruptStatus- sisr=0x%x\r\n"), sisr));

    return sisr;

}


//------------------------------------------------------------------------------
//
//  FUNCTION:       StartOutputDMA
//
//  DESCRIPTION:    Start playback DMA
//
//  PARAMETERS:
//                  None
//
//  RETURNS:
//                  TRUE
//
//------------------------------------------------------------------------------

BOOL HardwareContext::StartOutputDMA()
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("+StartOutputDMA\r\n")));

    UINT32 OutputTransferred;

    if(!m_OutputDMARunning)
    {
        //----- 1. Initialize our buffer counters -----
        // For now, pretend output dma is running in case we accidentally get reentered
        m_OutputDMARunning=TRUE;
        m_OutBytes[0] = 0;
        m_OutBytes[1] = 0;
        m_TxBufIndex = 0;

     Lock();
        //----- 2. Prime the output buffer with sound data -----
        OutputTransferred = TransferOutputBuffers();
        OutputTransferred += TransferOutputBuffers();

        Unlock();

        //----- 3. If we did transfer any data to the DMA buffers, go ahead and enable DMA -----
        if(OutputTransferred)
        {
            //----- 4. Configure the channel for playback -----
            DDKDmacSetSrcAddress(m_OutputDMAChan, m_DmaTxBuffer[0]);
            DDKDmacSetRepeatType(m_OutputDMAChan, DMAC_REPEAT_FOREVER);
            DDKDmacEnableChannelIntr(m_OutputDMAChan);

            //----- 5. Make sure the audio isn't muted -----
            //AudioMute(AUDIO_CH_OUT, FALSE);

            //----- 6. Start the DMA controller -----
            DDKDmacClearChannelIntr(m_OutputDMAChan);
            DDKDmacStartChan(m_OutputDMAChan);
            //InterruptDone(m_dwSysintrOutput);

            //UINT32 status;
            //status = ClearInterruptStatus(SSI_TRANSFER_TX, SSI_CHANNEL0);

           // call back BSP function to enable the output
            BSPAudioStartOutput(AUDIO_BUS_STEREO_OUT, AUDIO_PATH_HEADSET);

            // Wait for DMA to start
            while(DDKDmacGetTransSize(m_OutputDMAChan) == 0);
            // Set DMA for next buffer

            DDKDmacSetSrcAddress(m_OutputDMAChan, m_DmaTxBuffer[1]);

        }
        else    // We didn't transfer any data, so DMA wasn't enabled
        {
            m_OutputDMARunning = FALSE;
        }
    }

    DEBUGMSG(ZONE_FUNCTION, (TEXT("-StartOutputDMA\r\n")));
    return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function:  StopOutputDMA
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
    DEBUGMSG(ZONE_FUNCTION ,(_T("+HardwareContext::StopOutputDMA\n")));

    if(m_OutputDMARunning)
    {
       // BSPAudioStartOutput(FALSE);

         // Disable the speaker, Stereo DAC, and the audio MUX.
        BSPAudioStopOutput(AUDIO_BUS_STEREO_OUT, AUDIO_PATH_HEADSET);

        DDKDmacDisableChannelIntr(m_OutputDMAChan);
        DDKDmacStopChan(m_OutputDMAChan);
        DDKDmacClearChannelIntr(m_OutputDMAChan);

        m_OutputDMARunning = FALSE;

        DEBUGMSG(ZONE_MISC, (TEXT("Stopped output DMA\r\n")));
    }


    DEBUGMSG(ZONE_FUNCTION,(_T("-HardwareContext::StopOutputDMA\n")));
}


#ifdef AUDIO_RECORDING_ENABLED

//-----------------------------------------------------------------------------
//
//  Function:  InitInputDMA
//
//  This function initializes the DMA channel for input.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL HardwareContext::InitInputDMA()
{
    BOOL rc = FALSE;

    //TODO: Replace the DMAC_TRANSFER_SIZE with appropriate macro name
    //when changed in mx27_ddk.h

    DEBUGMSG(ZONE_FUNCTION,(_T("+HardwareContext::InitInputDMA\r\n")));

    // Check if DMA buffer has been allocated
    if (!g_PhysDMABufferAddr.LowPart)
    {
        DEBUGMSG(ZONE_ERROR, (_T("Invalid DMA buffer physical address.\r\n")));
        goto cleanUp;
    }

    // Configure the platform-specific output DMA channel
    if (!BSPAudioInitInput(AUDIO_BUS_VOICE_IN, m_DmaRxBuffer[0], AUDIO_DMA_PAGE_SIZE))
    {
        DEBUGMSG(ZONE_ERROR, (_T("BSPAudioInitOutput failed.\r\n")));
        goto cleanUp;
    }

    rc = TRUE;

cleanUp:
    if (!rc)
    {
        DeinitInputDMA();
    }

    DEBUGMSG(ZONE_FUNCTION,(_T("-HardwareContext::InitInputDMA\r\n")));

    return rc;
}


//-----------------------------------------------------------------------------
//
//  Function:  DeinitInputDMA
//
//  This function deinitializes the DMA channel for input.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL HardwareContext::DeinitInputDMA()
{
    DEBUGMSG(ZONE_FUNCTION,(_T("+HardwareContext::DeinitInputDMA\n")));

    // Stop the audio input devices
    BSPAudioStopInput(AUDIO_BUS_VOICE_IN, AUDIO_PATH_MIC);

    DDKDmacStopChan(m_InputDMAChan);
    DDKDmacReleaseChan(m_InputDMAChan);
    DDKDmacClearChannelIntr(m_InputDMAChan);

    DEBUGMSG(ZONE_FUNCTION,(_T("-HardwareContext::DeinitInputDMA\n")));

    return TRUE;
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
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL HardwareContext::StartInputDMA()
{

    BOOL rc = FALSE;

    DEBUGMSG(ZONE_FUNCTION,(_T("+HardwareContext::StartInputDMA\r\n")));

    // Must acquire critical section Lock() here so that we don't have a race
    // condition with StopInputDMA() and the interrupt handler.
    Lock();

    // Terminate the existing record CODEC disable timer since we're about
    // to start another recording operation.
    if (m_AudioDelayDisableTimerID[1] != NULL)
    {
        timeKillEvent(m_AudioDelayDisableTimerID[1]);
        m_AudioDelayDisableTimerID[1] = NULL;
    }

    // Reinitialize audio input if it is not already active.
    if(!m_InputDMARunning)
    {
        //----- 1. Initialize our buffer counters -----
        // For now, pretend input dma is running in case we accidentally get reentered
        m_InputDMARunning = TRUE;
        m_InBytes[0] = 0;
        m_InBytes[1] = 0;
        m_RxBufIndex = 0;

        //----- 2. Configure the channel for record -----
         //TODO: Define the channel number appropriately for m_DmaRxChannel
        DDKDmacSetRepeatType(m_InputDMAChan, DMAC_REPEAT_FOREVER);
        DDKDmacSetDestAddress(m_InputDMAChan, m_DmaRxBuffer[0]);

        //----- 3. Make sure the audio isn't muted -----
        //AudioMute(AUDIO_CH_IN, FALSE);

        //----- 4. Start the input DMA -----
        DDKDmacClearChannelIntr(m_InputDMAChan);
        DDKDmacStartChan(m_InputDMAChan);
        DDKDmacEnableChannelIntr(m_InputDMAChan);

        //InterruptDone(m_dwSysintrInput);

        BSPAudioStartInput(AUDIO_BUS_VOICE_IN, AUDIO_PATH_MIC);

        // Wait for DMA to start
        while(DDKDmacGetTransSize(m_InputDMAChan) == 0);
        // Set DMA for next buffer
        DDKDmacSetDestAddress(m_InputDMAChan, m_DmaRxBuffer[1]);

        DEBUGMSG(ZONE_MISC, (TEXT("Started input DMA\r\n")));
    }

    rc = TRUE;

//START_ERROR:
    Unlock();

    DEBUGMSG(ZONE_FUNCTION,(TEXT("-HardwareContext::StartInputDMA\r\n")));

    return rc;
}


//-----------------------------------------------------------------------------
//
//  Function:  StopInputDMA
//
//  This function stops any DMA activity on the input channel.
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
void HardwareContext::StopInputDMA()
{
    DEBUGMSG(ZONE_FUNCTION,(_T("+HardwareContext::StopInputDMA\n")));

    // If the input DMA is running, stop it
   if(m_InputDMARunning)
    {
        BSPAudioStopInput(AUDIO_BUS_VOICE_IN, AUDIO_PATH_MIC);

        DDKDmacDisableChannelIntr(m_InputDMAChan);
        DDKDmacStopChan(m_InputDMAChan);
        DDKDmacClearChannelIntr(m_InputDMAChan);

        //AudioMute(AUDIO_CH_IN, TRUE);

        m_InputDMARunning = FALSE;

        DEBUGMSG(ZONE_MISC, (TEXT("Stopped input DMA\r\n")));
    }

    DEBUGMSG(ZONE_FUNCTION,(_T("-HardwareContext::StopInputDMA\n")));
}

#endif // #ifdef AUDIO_RECORDING_ENABLED


//-----------------------------------------------------------------------------
//
//  Function:  GetInterruptThreadPriority
//
//  This function attempts to retrieve the thread priority for the audio
//  IST from the registry and supplies a default priority if one is not
//  found.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns thread priority for the audio IST.
//
//-----------------------------------------------------------------------------
DWORD HardwareContext::GetInterruptThreadPriority()
{
    HKEY hDevKey;
    DWORD dwValType;
    DWORD dwValLen;
    DWORD dwPrio = INTR_PRIORITY; // Default IST priority

    hDevKey = OpenDeviceKey((LPWSTR)m_dwDriverIndex);
    if (hDevKey)
    {
        dwValLen = sizeof(DWORD);
        RegQueryValueEx(
            hDevKey,
            INTR_PRIORITY_REGKEY,
            NULL,
            &dwValType,
            (PUCHAR)&dwPrio,
            &dwValLen);
        RegCloseKey(hDevKey);
    }

    return dwPrio;
}


//-----------------------------------------------------------------------------
//
//  Function:  InitInterruptThread
//
//  This function initializes the IST for handling DMA interrupts.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL HardwareContext::InitInterruptThread()
{
    DEBUGMSG(ZONE_FUNCTION,(_T("+HardwareContext::InitInterruptThread\n")));

    // Create an automatic reset, unsignaled, and unnamed event for IST
    // signaling.
    m_hAudioInterrupt = CreateEvent(NULL, FALSE, FALSE, NULL);

    // Check if event creation suceeded
    if (!m_hAudioInterrupt)
    {
        ERRMSG("Unable to create audio interrupt event");
        return(FALSE);
    }

    // Register the audio driver interrupts.
    if (m_pSSI2)
    {
        if (! InterruptInitialize(m_dwSysintrOutput, m_hAudioInterrupt, NULL, 0))
        {
            ERRMSG("Unable to initialize audio input interrupt for SSI1");
            return FALSE;
        }

        // Mask SSI1 interrupts. DMA transfer interrupts are used to signal
        // the Interrupt Service Thread (IST).

        //InterruptMask(m_dwSysintrOutput, TRUE);
    }

    if (m_pSSI1)
    {
        if (! InterruptInitialize(m_dwSysintrInput, m_hAudioInterrupt, NULL, 0))
        {
            ERRMSG("Unable to initialize audio output interrupt for SSI2");
            return FALSE;
        }

        // Mask SSI2 interrupts. DMA transfer interrupts are used to signal
        // the Interrupt Service Thread (IST).
        //InterruptMask(m_dwSysintrInput, TRUE);
    }


    // Create the Interrupt Service Thread (IST).
    m_hAudioInterruptThread =
        CreateThread((LPSECURITY_ATTRIBUTES)NULL,
                     0,
                     (LPTHREAD_START_ROUTINE)CallInterruptThread,
                     this,
                     0,
                     NULL);
    if (!m_hAudioInterruptThread)
    {
        ERRMSG("Unable to create audio interrupt event handling thread");
        return FALSE;
    }

    // Bump up the priority since the interrupt must be serviced immediately.
    CeSetThreadPriority(m_hAudioInterruptThread, GetInterruptThreadPriority());

    DEBUGMSG(ZONE_FUNCTION,(_T("-HardwareContext::InitInterruptThread\n")));

    return(TRUE);
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
//      None
//
//-----------------------------------------------------------------------------
void HardwareContext::PowerUp()
{
    BSPAudioPowerUp(TRUE);

    // Restart the audio I/O operations that were suspended when PowerDown()
    // was called.
    if (g_saveOutputDMARunning)
    {
        StartOutputDMA();
    }
#ifdef AUDIO_RECORDING_ENABLED
    if (g_saveInputDMARunning)
    {
        StartInputDMA();
    }
#endif

    BSPAudioPowerUp(FALSE);

    m_audioPowerdown = FALSE;
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

    // Manually set the timed delay disable events. This will explicitly
    // trigger any in-progress delays to immediately terminate.
    if (m_hAudioDelayDisableEvent[0] != NULL)
    {
        PulseEvent(m_hAudioDelayDisableEvent[0]);
    }
#ifdef AUDIO_RECORDING_ENABLED
    if (m_hAudioDelayDisableEvent[1] != NULL)
    {
        PulseEvent(m_hAudioDelayDisableEvent[1]);
    }
#endif

    // This tells the low-level PMIC driver to immediately terminate any
    // pending timer delays.
    BSPAudioPowerDown(FALSE);

    // Set this flag to allow special handling for the powerdown procedure.
    // The most important thing we want to avoid when powering down is using
    // any sort of timer or other type of delay mechanism. We need to bypass
    // anything like that and simply shutdown the hardware components when
    // performing a powerdown operation.
    m_audioPowerdown = TRUE;

    // Save state for correct powerup handling.
    g_saveOutputDMARunning = m_OutputDMARunning;
#ifdef AUDIO_RECORDING_ENABLED
    g_saveInputDMARunning  = m_InputDMARunning;
#endif

    // Request all active audio-related DMA channels to stop.
    StopOutputDMA();
#ifdef AUDIO_RECORDING_ENABLED
    StopInputDMA();
#endif

    // Request audio devices to power down.
    BSPAudioPowerDown(TRUE);
}


//-----------------------------------------------------------------------------
//----------------------------- Helper Functions ------------------------------
//-----------------------------------------------------------------------------

//############################################ Helper Functions #############################################
//------------------------------------------------------------------------------
//
//  FUNCTION:       TransferOutputBuffer
//
//  DESCRIPTION:    Actual function to transfer playback data from
//                  the indicated buffer to DMA.
//                  Called by TransferOutputBuffers
//
//  PARAMETERS:
//                  bufIndex - index of playback buffer to empty
//
//  RETURNS:
//                  Number of bytes transferred from buffer
//
//------------------------------------------------------------------------------
UINT32 HardwareContext::TransferOutputBuffer(UINT8 bufIndex)
{
    ULONG BytesTransferred = 0;

    UINT8 *pBufferStart = (UINT8 *)m_Output_pbDMA_PAGES[bufIndex];
    UINT8 *pBufferEnd = pBufferStart + AUDIO_DMA_PAGE_SIZE;
    UINT8 *pBufferLast;

    __try
    {
        pBufferLast = m_OutputDeviceContext.TransferBuffer(pBufferStart, pBufferEnd, NULL);
        BytesTransferred = m_OutBytes[bufIndex] = pBufferLast-pBufferStart;

        // Enable if you need to clear the rest of the DMA buffer
        StreamContext::ClearBuffer(pBufferLast,pBufferEnd);

        //if(BytesTransferred == 0)
        //    DmacSetRepeatType(m_DmaTxChannel, DMAC_REPEAT_DISABLED);
        //else
        DDKDmacSetSrcAddress(m_OutputDMAChan, m_DmaTxBuffer[bufIndex]);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("WAVDEV2.DLL:TransferOutputBuffer() - EXCEPTION: %d"), GetExceptionCode()));
    }

    return BytesTransferred;
}
//------------------------------------------------------------------------------
//
//  FUNCTION:       TransferOutputBuffers
//
//  DESCRIPTION:    Calls TransferOutputBuffer to transfer data from
//                  playback buffer to DMA. Also toggles the current
//                  buffer index to acheive double buffering. Stops
//                  output dma if nothing is transferred for 2
//                  buffers consecutively.
//
//  PARAMETERS:
//                  None
//
//  RETURNS:
//                  Number of bytes transferred from buffer
//
//------------------------------------------------------------------------------
UINT32 HardwareContext::TransferOutputBuffers(void)
{
    UINT32 BytesTransferred = 0;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("+TransferOutputBuffers\r\n")));

    BytesTransferred += TransferOutputBuffer(m_TxBufIndex);
    m_TxBufIndex = (m_TxBufIndex == 0)? 1 : 0;

    // If it was our interrupt, but we weren't able to transfer any bytes
    // (e.g. no full buffers ready to be emptied)
    // and all the output DMA buffers are now empty, then stop the output DMA
    if(m_OutBytes[0] + m_OutBytes[1] == 0)
    {
       if (DDKDmacGetTransStatus(m_OutputDMAChan) & DMAC_TRANSFER_STATUS_COMPLETE)
         if(m_OutputDMARunning)
        {
            BSPAudioStopOutput(AUDIO_BUS_STEREO_OUT, AUDIO_PATH_HEADSET);
            DDKDmacStopChan(m_OutputDMAChan);
            m_OutputDMARunning = FALSE;
        }

    }

    DEBUGMSG(ZONE_FUNCTION, (TEXT("-TransferOutputBuffers: BytesTransferred=%d\r\n"), BytesTransferred));

    return BytesTransferred;
}

#ifdef AUDIO_RECORDING_ENABLED
//------------------------------------------------------------------------------
//
//  FUNCTION:       TransferInputBuffer
//
//  DESCRIPTION:    Actual function to transfer received data from
//                  DMA to the indicated receive buffer.
//                  Called by TransferInputBuffers
//
//  PARAMETERS:
//                  bufIndex - index of receive buffer to fill
//
//  RETURNS:
//                  Number of bytes transferred to buffer
//
//------------------------------------------------------------------------------
UINT32 HardwareContext::TransferInputBuffer(UINT8 bufIndex)
{
    ULONG BytesTransferred = 0;

    UINT8 *pBufferStart = (UINT8 *)m_Input_pbDMA_PAGES[bufIndex];
    UINT8 *pBufferEnd = pBufferStart + AUDIO_DMA_PAGE_SIZE;
    UINT8 *pBufferLast;

    __try
    {
        pBufferLast = m_InputDeviceContext.TransferBuffer(pBufferStart, pBufferEnd, NULL);
        BytesTransferred = m_InBytes[bufIndex] = pBufferLast-pBufferStart;
        //DmacSetRepeatType(m_DmaRxChannel, DMAC_REPEAT_ONCE);
        DDKDmacSetDestAddress(m_InputDMAChan, m_DmaRxBuffer[bufIndex]);
        //DmacStartChan(m_DmaRxChannel);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("WAVDEV2.DLL:TransferInputBuffer() - EXCEPTION: %d"), GetExceptionCode()));
    }

    return BytesTransferred;
}
//------------------------------------------------------------------------------
//
//  FUNCTION:       TransferInputBuffers
//
//  DESCRIPTION:    Calls TransferInputBuffer to transfer data from
//                  DMA to receive buffer. Also toggles the current
//                  buffer index to acheive double buffering. Stops
//                  input dma if nothing is transferred.
//
//  PARAMETERS:
//                  None
//
//  RETURNS:
//                  Number of bytes transferred to buffer
//
//------------------------------------------------------------------------------
UINT32 HardwareContext::TransferInputBuffers(void)
{
    UINT32 BytesTransferred = 0;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("+TransferInputBuffers\r\n")));

    BytesTransferred = TransferInputBuffer(m_RxBufIndex);
    m_RxBufIndex = (m_RxBufIndex == 0)? 1 : 0;

    // If it was our interrupt, but we weren't able to transfer any bytes
    // (e.g. no empty buffers ready to be filled)
    // Then stop the input DMA
    if(BytesTransferred == 0)
    {
       if (DDKDmacGetTransStatus(m_OutputDMAChan) & DMAC_TRANSFER_STATUS_COMPLETE)
           if(m_InputDMARunning)
           {
                BSPAudioStopInput(AUDIO_BUS_VOICE_IN, AUDIO_PATH_MIC);
                DDKDmacStopChan(m_InputDMAChan);
                m_InputDMARunning = FALSE;
            }


    }

    DEBUGMSG(ZONE_FUNCTION, (TEXT("-TransferInputBuffers: BytesTransferred=%d\r\n"), BytesTransferred));
    return BytesTransferred;
}

#endif


//------------------------------------------------------------------------------
//
//  FUNCTION:       InterruptThread
//
//  DESCRIPTION:    Interrupt service routine for playback and recording
//                  DMA interrupts
//
//  PARAMETERS:
//                  None
//
//  RETURNS:
//                  None
//
//------------------------------------------------------------------------------

void HardwareContext::InterruptThread()
{
#ifdef AUDIO_RECORDING_ENABLED
    UINT32 InputTransferred;
#endif

    UINT32 OutputTransferred;
    UINT32 intrMask;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("+InterruptThread\r\n")));

    while(TRUE)
    {
        DEBUGMSG(ZONE_INTERRUPT, (TEXT("+WaitForSingleObject\r\n")));
        WaitForSingleObject(m_hAudioInterrupt, INFINITE);
        DEBUGMSG(ZONE_INTERRUPT, (TEXT("-WaitForSingleObject\r\n")));

        //----- 1. Grab the lock -----

        Lock();

        __try
        {
            //----- 2. Determine the interrupt source (input DMA operation or output DMA operation?) -----
            //----- NOTE:   Often, platforms use two separate DMA channels for input/output operations but
            //              have the OAL return SYSINTR_AUDIO as the interrupt source.  If this is the case,
            //              then the interrupt source (input or output DMA channel) must be determined in
            //              this step.

            intrMask = DDKDmacGetTransStatus(m_OutputDMAChan);

            if(intrMask != DMAC_TRANSFER_STATUS_NONE)
            {
                //----- 3. Acknowledge the DMA interrupt -----
                DDKDmacClearChannelIntr(m_OutputDMAChan);
                InterruptDone(m_dwSysintrOutput);
                if ( (intrMask == DMAC_TRANSFER_STATUS_COMPLETE) &&
                                (m_OutputDMARunning == TRUE) )
                {
                    OutputTransferred = TransferOutputBuffers();
                }
            }

#ifdef AUDIO_RECORDING_ENABLED

            intrMask = DDKDmacGetTransStatus(m_InputDMAChan);

            if(intrMask != DMAC_TRANSFER_STATUS_NONE)
            {
                //----- 3. Acknowledge the DMA interrupt -----
                DDKDmacClearChannelIntr(m_InputDMAChan);
                InterruptDone(m_dwSysintrInput);
                if((intrMask == DMAC_TRANSFER_STATUS_COMPLETE) && (m_InputDMARunning == TRUE))
                {

                    InputTransferred = TransferInputBuffers();

                }
            }
#endif

        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("WAVDEV2.DLL:InterruptThread() - EXCEPTION: %d"), GetExceptionCode()));
        }

        //----- 10. Give up the lock -----
        Unlock();

    }  // while(TRUE)

    DEBUGMSG(ZONE_FUNCTION, (TEXT("-InterruptThread\r\n")));
}


//-----------------------------------------------------------------------------
//
//  Function:  CallInterruptThread
//
//  This function serves as a wrapper for the IST called within the hardware
//  context.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void CallInterruptThread(HardwareContext *pHWContext)
{
    if (pHWContext != NULL)
    {
        pHWContext->InterruptThread();
    }
    else
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("CallInterruptThread() called with ")
                              TEXT("pHWContext == NULL\n")));
    }
}











