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
//  Copyright (C) 2004-2007, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  hwctxt.cpp
//
//  Platform dependent code for the WinCE WAV audio driver. This code's primary
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
//      * SDMA to handle audio I/O between memory and SSI1/SSI2.
//      * Audio MUX to route the audio stream between the SSI and the
//        PMIC Voice CODEC or Stereo DAC.
//      * Clock and Reset Module to generate the required clock signals for
//        SSI1/SSI2 (and the PMIC clock input in the case of the MC13783 PMIC).
//
//  See also the corresponding PMIC-specific source files for complete details
//  about the implementation of the BSP interface for audio. For example, in
//  the case of the MX31 platform, the rest of the audio driver code can be
//  found in the following files:
//
//      WINCE500\PLATFORM\MX31ADS\SRC\DRIVERS\WAVEDEV\MC13783\HWCTXT.CPP
//      WINCE500\PUBLIC\COMMON\OAK\CSP\ARM\FREESCALE\PMIC\SDK\PMIC_AUDIO.CPP
//
//-----------------------------------------------------------------------------

#include "wavemain.h"
#include <Mmsystem.h>   // For timer driver interface.

// Define the size (in data words) of the SSI TX FIFO. This is normally 8.

#define SSI_TX_FIFO_DEPTH               8 // Default 8
#define PLAYBACK_DISABLE_DELAY_MSEC     1000; // Default 1000

//-----------------------------------------------------------------------------
// External Variables

extern UINT32 PlaybackDisableDelayMsec;
extern UINT32 RecordDisableDelayMsec;
extern UINT16 audioDMAPageSize;

//-----------------------------------------------------------------------------
// Defines

const unsigned short HardwareContext::INTR_PRIORITY_REGKEY[12] =
    TEXT("Priority256");


//-----------------------------------------------------------------------------
// Exported Global Variables

HardwareContext *g_pHWContext = NULL;


//-----------------------------------------------------------------------------
// Local Global Variables

static PHYSICAL_ADDRESS g_PhysDMABufferAddr;

static BOOL g_saveOutputDMARunning = FALSE;
static BOOL g_saveInputDMARunning  = FALSE;


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

    RecordContext::CreateRecordContext(Index);

    if(!g_pHWContext || !g_pRecordContext)
    {
         return FALSE;
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
            : m_OutputDeviceContext()
{
    DEBUGMSG(ZONE_FUNCTION,(_T("+HardwareContext::HardwareContext\n")));

    InitializeCriticalSection(&m_Lock);
    m_Initialized = FALSE;

    // This flag is only set to TRUE when PowerDown() is called.
    m_audioPowerdown = FALSE;

    // The actual interrupt handler thread will be created later by calling
    // CreateThread() in InitInterruptThread().
    m_hAudioInterruptThread = NULL;

    // Create event that the audio driver IST will use to signal when it
    // wants to stop the output DMA (at the end of each playback operation).
    // Note that we don't want the audio driver IST thread to directly call
    // StopOutputDMA() since this also involves a call to some MMTIMER APIs
    // where the priority of the calling thread is changed. To be safe, we
    // want to avoid any changes to the IST thread priority.
    m_hAudioStopOutputDMA = CreateEvent(NULL, FALSE, FALSE, NULL);

    // Initialize the timer ID and timeout event for playback operations.
    //
    // We must create the events before we create the event handling thread
    // because the event handling thread will immediately begin to run once
    // it has been created.
    m_AudioDelayDisableTimerID = NULL;
    m_hAudioDelayDisableEvent  = CreateEvent(NULL, FALSE, FALSE, NULL);

    // Also initialize the delayed disable timer event handling thread.
    // The actual thread will be created later by calling CreateThread()
    // in Init().
    m_hAudioDelayedDisableThread = NULL;

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

    if (m_AudioDelayDisableTimerID != NULL)
    {
        timeKillEvent(m_AudioDelayDisableTimerID);
        m_AudioDelayDisableTimerID = NULL;
    }

    Unlock();

    DeleteCriticalSection(&m_Lock);

    // Note: The m_hAudioDelayDisableEvent and m_hAudioStopOutputDMA event
    //       handles will be closed and deallocated just before the
    //       DisableDelayThread terminates.
    //
    //       The m_hAudioInterrupt event handle will be closed and deallocated
    //       just before the InterruptThread terminates.

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
    m_CodecPwrState = AUDIO_PWR_STATE_OFF;

    // Initialize the driver state/status variables
    m_DriverIndex       = Index;
    m_bCanStopOutputDMA = TRUE;
    m_OutputDMARunning  = FALSE;
    m_OutputDMAStatus   = DMA_CLEAR;
    m_bSSITXUnderrunError = FALSE;

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

    // Initialize the input DMA.
    if (!InitInputDMA(g_PhysDMABufferAddr))
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("WAVEDEV.DLL:HardwareContext::Init() - ")
                              TEXT("Failed to initialize input DMA.\r\n")));
        goto cleanUp;
    }

    // Note that the audio driver interrupt handler will only run if
    // m_Initialized is TRUE so we must initialize the state variable
    // here before trying to create the IST thread.
    m_Initialized = TRUE;

    // Initialize and create audio processing IST.
    if ((m_hAudioInterruptThread == NULL) && (!InitInterruptThread()))
    {
        ERRMSG("WAVEDEV.DLL:HardwareContext::Init() - "
               "Failed to initialize interrupt thread.\r\n");
        m_Initialized = FALSE;
        goto cleanUp;
    }

    // Initialize and create delayed disable timer event handling thread.
    if ((m_hAudioDelayedDisableThread == NULL) &&
        ((PlaybackDisableDelayMsec > 0) || (RecordDisableDelayMsec > 0)))
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
            m_Initialized = FALSE;
            goto cleanUp;
        }
    }

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

    if (m_OutputDMARunning)
    {
        StopOutputDMA();
    }

    Unlock();

    // Unmap the audio control registers and DMA buffers.
    BSPAudioDeinit();
    UnmapDMABuffers();

    m_Initialized = FALSE;

    // Terminate input (recording) DMA operation if it is currently running.
    if (g_pRecordContext->m_InputDMARunning)
    {
        StopInputDMA();
    }

    // Wake up the Delayed Disable thread so that it can clean up after
    // itself and then terminate.
    // (now that m_Initialize is FALSE).
    if (m_hAudioStopOutputDMA != NULL)
    {
        SetEvent(m_hAudioStopOutputDMA);
    }

    // Wake up the audio driver IST thread so that it can also clean up
    // after itself and then terminate.
    if (m_hAudioInterrupt != NULL)
    {
        SetEvent(m_hAudioInterrupt);
    }

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
    BOOL rc = TRUE;

    DEBUGMSG(ZONE_FUNCTION,(_T("+HardwareContext::MapDMABuffers\n")));

    // Allocate a block of virtual memory (physically contiguous) for the
    // DMA buffers. Note that a platform-specific API call is used here to
    // allow for the DMA buffers to be allocated from different memory regions
    // (e.g., internal vs. external memory).
    pVirtDMABufferAddr = BSPAudioAllocDMABuffers(&g_PhysDMABufferAddr);

    if (pVirtDMABufferAddr == NULL)
    {
        ERRORMSG(ZONE_ERROR, (TEXT("WAVEDEV.DLL:HardwareContext::")
                              TEXT("MapDMABuffers() - ")
                              TEXT("Failed to allocate DMA buffer.\r\n")));
        rc = FALSE;
    }

    // Setup the DMA page pointers.
    //
    // NOTE: Currently, input and output each have two DMA pages: these pages
    //       are used in a round-robin fashion so that the OS can read/write
    //       one buffer while the audio codec chip read/writes the other buffer.
    //
    m_Output_pbDMA_PAGES[0] = pVirtDMABufferAddr;
    m_Output_pbDMA_PAGES[1] = pVirtDMABufferAddr + audioDMAPageSize;

    /*map the DMA buffers for recording*/
    if(TRUE != g_pRecordContext->MapRecordDMABuffers(pVirtDMABufferAddr))
    {
        rc = FALSE;
    }

    DEBUGMSG(ZONE_FUNCTION,(_T("-HardwareContext::MapDMABuffers\n")));

    return rc;

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
   BOOL rc = TRUE;
   DEBUGMSG(ZONE_FUNCTION,(_T("+HardwareContext::UnmapDMABuffers\n")));

    if (m_Output_pbDMA_PAGES[0])
    {
        BSPAudioDeallocDMABuffers((PVOID)(m_Output_pbDMA_PAGES[0]));
        m_Output_pbDMA_PAGES[0] = NULL;
        m_Output_pbDMA_PAGES[1] = NULL;

        if(TRUE != g_pRecordContext->UnmapRecordDMABuffers())
        {
             rc = FALSE;
        } 
    }

    DEBUGMSG(ZONE_FUNCTION,(_T("-HardwareContext::UnmapDMABuffers\n")));

    return rc;
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

MMRESULT HardwareContext::SetOutputMute (BOOL fMute)
{
    // TODO: Call down to PMIC driver for codec mute configuration

    // Save output mute configuration
    m_fOutputMute = fMute;

    return MMSYSERR_NOERROR;
}

BOOL HardwareContext::GetOutputMute (void)
{
    return m_fOutputMute;
}

DWORD HardwareContext::GetOutputGain (void)
{
    return m_dwOutputGain;
}


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
    DDK_DMA_ACCESS audioDataTXWordSize = DDK_DMA_ACCESS_8BIT;

    UINT8 size = sizeof(HWSAMPLE);
    if (size == sizeof(INT16))
    {
        audioDataTXWordSize = DDK_DMA_ACCESS_16BIT;
    }
    else if (size == sizeof(INT32))
    {
        audioDataTXWordSize = DDK_DMA_ACCESS_32BIT;
    }

    DEBUGMSG(ZONE_FUNCTION,(_T("+HardwareContext::InitOutputDMA\n")));

    // Confirm that DMA buffer has been allocated.
    if (!g_PhysDMABufferAddr.LowPart)
    {
        ERRORMSG(ZONE_ERROR, (_T("Invalid DMA buffer physical address.\r\n")));
        goto cleanUp;
    }

    // Configure the platform-specific output DMA channel
    if (!BSPAudioInitOutput(AUDIO_BUS_STEREO_OUT))
    {
        ERRORMSG(ZONE_ERROR, (_T("BSPAudioInitOutput failed.\r\n")));
        goto cleanUp;
    }

    // Allocate a DMA chain for the virtual channel to handle the DMA transfer
    // requests.
    if (!DDKSdmaAllocChain(m_OutputDMAChan, NUM_DMA_BUFFERS))
    {
        ERRORMSG(ZONE_ERROR, (_T("DDKSdmaAllocChain failed.\r\n")));
        goto cleanUp;
    }

    // Configure the SDMA buffer descriptors. We currently use 2 contiguous DMA
    // pages as a circular queue. Each DMA buffer is audioDMAPageSize bytes
    // in size.
    //
    // If the audio data stream is not big enough to fill a DMA buffer (i.e., we
    // are performing the last transfer), then we will resize the data transfer
    // at that time.
    if (!DDKSdmaSetBufDesc(m_OutputDMAChan,
                           0,
                           DDK_DMA_FLAGS_INTR | DDK_DMA_FLAGS_CONT,
                           g_PhysDMABufferAddr.LowPart,
                           0,
                           audioDataTXWordSize,
                           audioDMAPageSize))
    {
        ERRORMSG(ZONE_ERROR, (_T("DDKSdmaSetBufDesc failed.\r\n")));
        goto cleanUp;
    }

    if (!DDKSdmaSetBufDesc(m_OutputDMAChan,
                           1,
                           DDK_DMA_FLAGS_INTR | DDK_DMA_FLAGS_CONT |
                                                DDK_DMA_FLAGS_WRAP,
                           g_PhysDMABufferAddr.LowPart + audioDMAPageSize,
                           0,
                           audioDataTXWordSize,
                           audioDMAPageSize))
    {
        ERRORMSG(ZONE_ERROR, (_T("DDKSdmaSetBufDesc failed.\r\n")));
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

    if (m_OutputDMAChan != 0)
    {
        DDKSdmaCloseChan(m_OutputDMAChan);
        m_OutputDMAChan = 0;
    }

    DEBUGMSG(ZONE_FUNCTION,(_T("-HardwareContext::DeinitOutputDMA\n")));

    return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function:  StartOutputDMA
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
    HWSAMPLE ssiFifoPrefill[SSI_TX_FIFO_DEPTH]; // For SSI TX FIFO prefill.
    unsigned nSsiFifoPrefill = 0; // Number of words used in ssiFifoPrefill[].
    BOOL rc = FALSE;
    ULONG nOutputTransferred = 0;

    DEBUGMSG(ZONE_FUNCTION,(_T("+HardwareContext::StartOutputDMA\r\n")));

    // Must acquire critical section Lock() here so that we don't have a race
    // condition with StopOutputDMA() and the interrupt handler.
    Lock();

    // Terminate the existing playback CODEC disable timer since we're about
    // to start another playback operation.
    if (m_AudioDelayDisableTimerID != NULL)
    {
        timeKillEvent(m_AudioDelayDisableTimerID);
        m_AudioDelayDisableTimerID = NULL;
    }

    // Reinitialize audio output if it is not already active.
    if (!m_OutputDMARunning || m_bSSITXUnderrunError)
    {
        // Initialize output DMA state variables. Note that if we get here
        // because m_bSSITXUnderrunError is still TRUE, then we must have
        // ran out of additional audio data in the InterruptThread() handler.
        // Therefore, it is correct here to reinitialize everything just as
        // if the DMA was not running.
        m_OutputDMAStatus  = DMA_DONEA | DMA_DONEB;
        m_OutputDMARunning = TRUE;

        // Pre-fill with silence
        nSsiFifoPrefill = sizeof(ssiFifoPrefill);
        memset( ssiFifoPrefill, 0, sizeof(ssiFifoPrefill) );

        // Convert from bytes to number of words (i.e., the number of elements
        // in the ssiFifoPrefill[] buffer that are currently in use).
        nSsiFifoPrefill /= sizeof(HWSAMPLE);

        // Also prime the output DMA buffers.
        nOutputTransferred = TransferOutputBuffers(m_OutputDMAStatus);

        // Check if any audio output data was actually available and
        // transferred to the SSI FIFO prefill and/or DMA output buffers.
        //
        // Note that we don't really care that nSsiFifoPrefill is in "words"
        // while nOutputTransferred is in "bytes" since all that we care about
        // here is whether the sum of the two is non-zero.
        if(nSsiFifoPrefill + nOutputTransferred > 0)
        {
            // Check if any of the DMA buffers have been filled with new audio
            // data. Do not enable the DMA controller if there is no data in
            // any of the DMA buffers (this is normally an unusual condition
            // but it may occur so let's handle it properly).
            if (nOutputTransferred > 0)
            {
                // Allocate a set of DMA buffer descriptors and reset the
                // state of the output DMA transfer list. This call must be
                // matched by a later call to DDKSdmaStopChan(m_OutputDMAChan,
                // FALSE) in StopOutputDMA() to free up all of the allocated
                // resources.
            //
                // Note that the second "watermark" argument to the function
                // DDKSdmaInitChain() is actually in bytes whereas the variable
                // "m_OutputDMALevel" is given in SSI FIFO slots or words.
                // Therefore, we have to multiply by the audio data word size
                // to ensure that we are properly refilling the SSI TX FIFO.
            //
                if(!m_bSSITXUnderrunError &&
                   !DDKSdmaInitChain(m_OutputDMAChan,
                                 m_OutputDMALevel * sizeof(HWSAMPLE)))
            {
                    DEBUGMSG(ZONE_ERROR,
                             (_T("HardwareContext::StartOutputDMA(): Unable ")
                              _T("to initialize output DMA channel!\r\n")));

                    m_OutputDMARunning  = FALSE;
                    m_bCanStopOutputDMA = TRUE;

                    goto START_ERROR;
            }

            DEBUGMSG(ZONE_VERBOSE, (_T("  Starting first DMA operation\r\n")));

            // Start the output DMA channel. This must be done before we call
            // BSPAudioStartOutput() because we have to make sure that audio
            // data is already available before we activate the audio output
            // hardware.
            DDKSdmaStartChan(m_OutputDMAChan);

                
            }

            // Check to see if we need to complete the handling of the last
            // SSI TX underrun error which was flagged in InterruptThread().
            //
            // This takes care of the unusual situation whereby the last
            // playback operation ended with an SSI TX underrun error but
            // we are now about to resume playback without having called
            // StopOutputDMA() in between.
            if (m_bSSITXUnderrunError)
            {
                // In this case, the SSI has just been disabled and it does
                // not need to be completely reinitialized.
                //
                // Reenable the SSI here if we disabled it during the last
                // InterruptThread() call where we also had to handle a TX
                // underrun error and there was no more audio data available
                // to continue.
                BSPAudioStartSsiOutput(BSPAudioGetStDACSSI(),
                                       ssiFifoPrefill, nSsiFifoPrefill);

                // We have now completed the recovery from the most recent
                // SSI TX underrun error.
                m_bSSITXUnderrunError = FALSE;

            }
            else
            {
                // Enable the audio output hardware. We also fill the SSI
                // transmit FIFO at this point with the initial words from
                // the audio stream so that there is data immediately ready
                // to be transmitted when we enable the SSI transmitter.
                // This call must be done after we have already activated the
                // output DMA channel (by calling DDKSdmaStartChan()).
                // Otherwise, there is a race condition between how fast the
                // initial transmit FIFO will be emptied and whether the DMA
                // controller will be ready yet to handle the first interrupt
                // from the FIFO.
                BSPAudioStartOutput(AUDIO_BUS_STEREO_OUT,
                                    AUDIO_PATH_HEADSET,
                                    ssiFifoPrefill,
                                    nSsiFifoPrefill);
            }
        }
    }
    else
    {
        // Try to transfer in case we've got a buffer spare and waiting
        nOutputTransferred = TransferOutputBuffers(m_OutputDMAStatus);
  
     }
        

    // This will be set to TRUE only by the audio driver IST at the
    // end of the playback operation.
    m_bCanStopOutputDMA = FALSE;

    rc = TRUE;

START_ERROR:
    Unlock();

    DEBUGMSG(ZONE_FUNCTION,(_T("-HardwareContext::StartOutputDMA\r\n")));

    return rc;
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

    // If the output DMA is running, stop it
    if (m_OutputDMARunning)
    {
        // Reset output DMA state variables
        m_OutputDMAStatus  = DMA_CLEAR;
        m_OutputDMARunning = FALSE;

        // Also reset the SSI TX underrun error flag here since we are about
        // to shutdown the current audio setup. The audio hardware will be
        // fully reinitialized when StartOutputDMA() is next called so we no
        // longer care if an SSI TX underrun error was previously flagged for
        // the current audio playback operation.
        m_bSSITXUnderrunError = FALSE;

        // Disable the speaker, Stereo DAC, and the audio MUX.
        BSPAudioStopOutput(AUDIO_BUS_STEREO_OUT, AUDIO_PATH_HEADSET);

        // Kill the output DMA channel and frees up the DMA buffer descriptors.
        DDKSdmaStopChan(m_OutputDMAChan, FALSE);
    }

    DEBUGMSG(ZONE_FUNCTION,(_T("-HardwareContext::StopOutputDMA\n")));
}


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

    hDevKey = OpenDeviceKey((LPWSTR)m_DriverIndex);
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
    if (m_pSSI1)
    {
        if (! InterruptInitialize(m_dwSysintrSSI1, m_hAudioInterrupt, NULL, 0))
        {
            ERRMSG("Unable to initialize audio input interrupt for SSI1");
            return FALSE;
        }

        // Mask SSI1 interrupts. SDMA transfer interrupts are used to signal
        // the Interrupt Service Thread (IST).
        InterruptMask(m_dwSysintrSSI1, TRUE);
    }

    if (m_pSSI2)
    {
        if (! InterruptInitialize(m_dwSysintrSSI2, m_hAudioInterrupt, NULL, 0))
        {
            ERRMSG("Unable to initialize audio output interrupt for SSI2");
            return FALSE;
        }

        // Mask SSI2 interrupts. SDMA transfer interrupts are used to signal
        // the Interrupt Service Thread (IST).
        InterruptMask(m_dwSysintrSSI2, TRUE);
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
//      Returns TRUE if successful, otherwise returns FALSE.
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

    if (g_saveInputDMARunning)
    {
        StartInputDMA();
    }

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
    if (m_hAudioDelayDisableEvent != NULL)
    {
        PulseEvent(m_hAudioDelayDisableEvent);
    }

    if (g_pRecordContext->m_hAudioDelayDisableEvent != NULL)
    {
        PulseEvent(g_pRecordContext->m_hAudioDelayDisableEvent);
    }

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

    g_saveInputDMARunning  = g_pRecordContext->m_InputDMARunning;

    // Request all active audio-related DMA channels to stop.
    StopOutputDMA();

    StopInputDMA();

    // Request audio devices to power down.
    BSPAudioPowerDown(TRUE);
}


//-----------------------------------------------------------------------------
//----------------------------- Helper Functions ------------------------------
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//
//  Function:  TransferOutputBuffer
//
//  This function retrieves the next "mixed" audio buffer of data to DMA into
//  the output channel.
//
//  Parameters:
//      NumBuf
//          [in] Output DMA page to be filled.
//
//  Returns:
//      Returns number of bytes placed into output buffer.
//
//-----------------------------------------------------------------------------
ULONG HardwareContext::TransferOutputBuffer(const PBYTE pBufferStart,
                                            const PBYTE pBufferEnd)
{
    ULONG BytesTransferred = 0;

    PBYTE pBufferLast;

    __try
    {
        pBufferLast = m_OutputDeviceContext.TransferBuffer(pBufferStart,
                                                           pBufferEnd,
                                                           NULL);
        BytesTransferred = pBufferLast - pBufferStart;

        if(pBufferLast != pBufferEnd)

        // Only mark the DMA buffer as being in use if some data was actually
        // transferred.
            // Enable if you need to clear the rest of the DMA buffer
            // TODO:
            // Probably want to do something better than clearing out remaining
            // buffer samples.  DC output by replicating last sample or
            // some type of fade out would be better.
            StreamContext::ClearBuffer(pBufferLast, pBufferEnd);

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("WAVDEV2.DLL:TransferOutputBuffer() - ")
                              TEXT("EXCEPTION: %d"), GetExceptionCode()));
    }

    return BytesTransferred;
}


//-----------------------------------------------------------------------------
//
//  Function:  TransferOutputBuffers
//
//  This function determines which output buffer (A or B) needs to be filled
//  with sound data.  The correct buffer is then populated with data and ready
//  to DMA to the output channel.
//
//  Parameters:
//      dwDCSR
//          [in] Current status of the output DMA.
//
//  Returns:
//      Returns number of bytes placed into the output buffers.
//
//-----------------------------------------------------------------------------
ULONG HardwareContext::TransferOutputBuffers(DWORD dwDCSR)
{
    ULONG BytesTransferred_A = 0;
    ULONG BytesTransferred_B = 0;

    if (dwDCSR & DMA_DONEA)
    {
        // DMA buffer A is free, try to refill it.
        BytesTransferred_A =
            TransferOutputBuffer(m_Output_pbDMA_PAGES[OUT_BUFFER_A],
                                 m_Output_pbDMA_PAGES[OUT_BUFFER_A] +
                                 audioDMAPageSize);
        if (BytesTransferred_A > 0)
        {
            m_OutputDMAStatus &= ~DMA_DONEA;
  
        }
    }

    if (dwDCSR & DMA_DONEB)
    {
        // DMA buffer B is free, try to refill it.
        //
        // Note that we can refill both DMA buffers at once if both DMA
        // buffers are free and there is sufficient data.
        //
        BytesTransferred_B =
            TransferOutputBuffer(m_Output_pbDMA_PAGES[OUT_BUFFER_B],
                                 m_Output_pbDMA_PAGES[OUT_BUFFER_B] +
                                 audioDMAPageSize);
        if (BytesTransferred_B > 0)
        {
            m_OutputDMAStatus &= ~DMA_DONEB;
 
        }
    }

    return BytesTransferred_A + BytesTransferred_B;
}

//-----------------------------------------------------------------------------
//
//  Function:  InterruptThread
//
//  This function is the IST for handling audio input and output DMA interrupts.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void HardwareContext::InterruptThread()
{
    HWSAMPLE ssiFifoPrefill[SSI_TX_FIFO_DEPTH]; // For SSI TX FIFO prefill.
    unsigned nSsiFifoPrefill = 0; // Number of words used in ssiFifoPrefill[].
    UINT32 bufDescStatus[NUM_DMA_BUFFERS];

    DEBUGMSG(ZONE_FUNCTION,(_T("+HardwareContext::InterruptThread\n")));

    // Fast way to access embedded pointers in wave headers in other processes.
   // SetProcPermissions((DWORD)-1);

    // Loop repeatedly to handle each audio interrupt event until we
    // deinitialize the audio driver.
    while (m_Initialized)
    {
        // Wait for the IST to be signaled by the OAL interrupt handler.
        WaitForSingleObject(m_hAudioInterrupt, INFINITE);

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

        __try
        {
            // Handle DMA output (i.e., audio playback) if it is active.
            if (m_OutputDMARunning)
            {
                PCSP_SSI_REG pStDACSSI = BSPAudioGetStDACSSI();
                DWORD ssi_sisr = INREG32(&pStDACSSI->SISR);

                // Check for SSI transmitter underrun errors. These are
                // bad and can result in unpredictable swapping of the
                // left/right audio channels if we do not recover from
                // it properly.
                if (ssi_sisr & CSP_BITFMASK(SSI_SISR_TUE0))
                {
                    DEBUGMSG(ZONE_ERROR, (_T("ERROR: SSI%d TX underrun ")
                                          _T("error\n"),
                                         (pStDACSSI == m_pSSI1 ? 1 : 2)));

                    // Flag the SSI TX underrun error so that we can recover
                    // from it later.
                    m_bSSITXUnderrunError = TRUE;
                }

                if (ssi_sisr & CSP_BITFMASK(SSI_SISR_ROE0))
                {
                    // Don't expect to see this ever if the SSI is used only
                    // for audio playback to the PMIC Stereo DAC.
                    ERRORMSG(ZONE_ERROR, (_T("ERROR: SSI%d RX overrun ")
                                          _T("error\n"),
                                          (pStDACSSI == m_pSSI1 ? 1 : 2)));
                }

                // Get the transfer status of the DMA output buffers.
                if (!DDKSdmaGetChainStatus(m_OutputDMAChan, bufDescStatus))
                {
                    ERRORMSG(ZONE_ERROR,(_T("Could not retrieve output buffer ")
                                         _T("status\r\n")));
                }

                if (bufDescStatus[OUT_BUFFER_A] & DDK_DMA_FLAGS_ERROR)
                {
                    ERRORMSG(ZONE_ERROR,(_T("ERROR:  Error flag set in ")
                                         _T("OUT_BUFFER_A descriptor\r\n")));
                }

                if (bufDescStatus[OUT_BUFFER_B] & DDK_DMA_FLAGS_ERROR)
                {
                    ERRORMSG(ZONE_ERROR,(_T("ERROR:  Error flag set in ")
                                         _T("OUT_BUFFER_B descriptor\r\n")));
                }

                // Set DMA status bits according to retrieved transfer status.
                if (!(bufDescStatus[OUT_BUFFER_A] & DDK_DMA_FLAGS_BUSY))
                {
                    // The data in DMA buffer A has been transmitted and the
                    // buffer may now be reused.
                    m_OutputDMAStatus |= DMA_DONEA;
                    DDKSdmaClearBufDescStatus(m_OutputDMAChan, OUT_BUFFER_A);
                }

                if (!(bufDescStatus[OUT_BUFFER_B] & DDK_DMA_FLAGS_BUSY))
                {
                    // The data in DMA buffer B has been transmitted and the
                    // buffer may now be reused.
                    m_OutputDMAStatus |= DMA_DONEB;
                    DDKSdmaClearBufDescStatus(m_OutputDMAChan, OUT_BUFFER_B);
                }

                // Check to see if an SSI TX underrun error has just occurred
                // and, if necessary, recover from it.
                if (m_bSSITXUnderrunError)
                {
                    // Start the SSI TX underrun error recovery by disabling
                    // the SSI. This will also flush out any stray data that
                    // may still be in the TX FIFO. Note that this is possible
                    // because an earlier DMA operation may have refilled the
                    // TX FIFO (we only get interrupts here when a DMA buffer
                    // has been emptied).
                    //
                    BSPAudioStopSsiOutput(BSPAudioGetStDACSSI());

                    // Next stop all output DMA channel activity and flush out
                    // both DMA buffers.
                    DDKSdmaStopChan(m_OutputDMAChan, TRUE);
                    m_OutputDMAStatus = DMA_DONEA | DMA_DONEB;

                    // Pre-fill with silence
                    nSsiFifoPrefill = sizeof(ssiFifoPrefill);
                    memset( ssiFifoPrefill, 0, sizeof(ssiFifoPrefill) );
                    }

                // Try to refill any empty output DMA buffers with new data.
                TransferOutputBuffers(m_OutputDMAStatus);

                // Don't restart the DMA unless we actually have some data
                // in at least one of the DMA buffers.
                if (!(m_OutputDMAStatus & DMA_DONEA) ||
                    !(m_OutputDMAStatus & DMA_DONEB))
                {
                    // SDMA will disable itself on buffer underrun.
                    // Force channel to be enabled since we now have
                    // at least one buffer to be processed.
                    DDKSdmaStartChan(m_OutputDMAChan);

                    // Also reenable the SSI here if we disabled it above when
                    // we began our handling of the TX underrun error.
                    if (m_bSSITXUnderrunError)
                    {
                        BSPAudioStartSsiOutput(BSPAudioGetStDACSSI(),
                                               ssiFifoPrefill, nSsiFifoPrefill);

                        // We have now completed the recovery from the most
                        // recent SSI TX underrun error.
                        m_bSSITXUnderrunError = FALSE;
                    }
                }
                else
                {
                    // The upper audio driver layer has no more data, so we can
                    // terminate the current DMA operation because all DMA
                    // buffers are empty.
                    //
                    // Note that we signal the DisableDelayThread() in the
                    // BSP portion of the audio driver to actually make the
                    // call to StopOutputDMA() so that we can ensure that
                    // this IST thread will never have it's priority changed.
                    //
                    // Currently the MMTIMER library can temporarily change
                    // the priority of a calling thread and StopOutputDMA()
                    // does make calls to the MMTIMER library. Therefore,
                    // we do not want this IST thread making any direct
                    // MMTIMER calls.
                    //
                    m_bCanStopOutputDMA = TRUE;
                    m_OutputDMARunning = FALSE; 
                    SetEvent(m_hAudioStopOutputDMA);
                }
            }

            /*Check for the recording part of interrupt handling*/
            InterruptRecordThread();
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("WAVDEV2.DLL:InterruptThread() - ")
                                  TEXT("EXCEPTION: %d"),
                 GetExceptionCode()));
        }

        // No need to call InterruptDone since we are using SDMA interrupts
        Unlock();
    }

    CloseHandle(m_hAudioInterrupt);
    m_hAudioInterrupt = NULL;

    DEBUGMSG(ZONE_FUNCTION,(_T("-HardwareContext::InterruptThread\n")));
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
DWORD CALLBACK CallInterruptThread(HardwareContext *pHWContext)
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
    return 1;
}
