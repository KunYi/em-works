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
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2007 Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  recctxt.cpp
//
//  ChipSpecific driver for Audio device. The primary responsibility of this driver is to
//
//    1. Create stubs to support audio recording.
//    
//
//    PMIC Components:
//
//      * Voice CODEC for recording along with associated mixers, input
//        amplifiers, and audio microphone input jacks.
//
//    i.MX or MXC Components:
//
//      * SSI1 for Voice CODEC I/O
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
//      WINCE500\PLATFORM\MX31\SRC\DRIVERS\WAVEDEV\HWCTXT.CPP
//      WINCE500\PUBLIC\COMMON\OAK\CSP\ARM\FREESCALE\PMIC\SDK\PMIC_AUDIO.CPP
//
//-----------------------------------------------------------------------------

#include "wavemain.h"
#include <Mmsystem.h>   // For timer driver interface.



//-----------------------------------------------------------------------------
// Exported Global Variables
RecordContext *g_pRecordContext = NULL;


//-----------------------------------------------------------------------------
// Local Global Variables

//-----------------------------------------------------------------------------
// Defines

//-----------------------------------------------------------------------------
//Externs
extern UINT16 audioDMAPageSize;

//-----------------------------------------------------------------------------
//
//  Function: CreateRecordContext
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
BOOL RecordContext::CreateRecordContext(DWORD Index)
{
    DEBUGMSG(ZONE_FUNCTION,(_T("+RecordContext::CreateRecordContext\n")));

    if (g_pRecordContext)
    {
        return(TRUE);
    }

    g_pRecordContext = new RecordContext;
    if (!g_pRecordContext)
    {
        return(FALSE);
    }

    DEBUGMSG(ZONE_FUNCTION,(_T("-HardwareContext::CreateHWContext\n")));

    return(g_pRecordContext->Init(Index));
}

//-----------------------------------------------------------------------------
//
//  Function: RecordContext
//
//  This function is the constructor for the Record context class.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
RecordContext::RecordContext()
{
    DEBUGMSG(ZONE_FUNCTION,(_T("+RecordContext::RecordContext\n")));
    m_RecordInitialized = FALSE;

    InitializeCriticalSection(&m_Lock);

    m_AudioDelayDisableTimerID = NULL;
    m_hAudioDelayDisableEvent  = CreateEvent(NULL, FALSE, FALSE, NULL);

    pm_InputDeviceContext = new InputDeviceContext();

    DEBUGMSG(ZONE_FUNCTION,(_T("-HardwareContext::HardwareContext\n")));
}


//-----------------------------------------------------------------------------
//
//  Function: ~RecordContext
//
//  This function is the destructor for the Record context class.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
RecordContext::~RecordContext()
{
    DEBUGMSG(ZONE_FUNCTION,(_T("+RecordContext::~RecordContext\n")));

    Lock();

    if (m_AudioDelayDisableTimerID != NULL)
    {
        timeKillEvent(m_AudioDelayDisableTimerID);
        m_AudioDelayDisableTimerID = NULL;
    }

    Unlock();

    // Note: The m_hAudioDelayDisableEvent handle will be closed and
    //       deallocated just before the DisableDelayThread terminates.

    DEBUGMSG(ZONE_FUNCTION,(_T("-HardwareContext::~HardwareContext\n")));
}

//-----------------------------------------------------------------------------
//
//  Function: Init
//
//  This function initializes the recording context for the audio device.
//  This function is only a stub
//
//  Parameters:
//      Index
//          [in] Context parameter passed to the WAV_Init function
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL RecordContext::Init(DWORD Index)
{
    DEBUGMSG(ZONE_FUNCTION,(_T("+RecordContext::Init\n")));
    UNREFERENCED_PARAMETER(Index);

    // If we have already initialized this device, return error
    if (m_RecordInitialized)
    {
        return(FALSE);
    }

    m_dwInputGain   = 0x0000;
    m_fInputMute    = TRUE;

    m_InputDMARunning  = FALSE;
    m_InputDMAStatus   = DMA_CLEAR;

    m_RecordInitialized = TRUE;

    return m_RecordInitialized;
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
BOOL RecordContext::Deinit()
{
    DEBUGMSG(ZONE_FUNCTION,(_T("+RecordContext::Deinit\n")));

    m_RecordInitialized = FALSE;

    DEBUGMSG(ZONE_FUNCTION,(_T("-RecordContext::Deinit\n")));

    return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function:  MapRecordDMABuffers
//
//  This function maps the DMA buffers used for audio input.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL RecordContext::MapRecordDMABuffers(PBYTE pVirtDMABufferAddr)
{
    
    m_Input_pbDMA_PAGES[0]  = pVirtDMABufferAddr + (2 * audioDMAPageSize);
    m_Input_pbDMA_PAGES[1]  = pVirtDMABufferAddr + (3 * audioDMAPageSize);

    return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function:  UnmapRecordDMABuffers
//
//  This function Unmaps the DMA buffers used for audio input.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------

BOOL RecordContext::UnmapRecordDMABuffers()
{
     m_Input_pbDMA_PAGES[0]  = NULL;
     m_Input_pbDMA_PAGES[1]  = NULL;
     return TRUE;
}


//-----------------------------------------------------------------------------
//

BOOL HardwareContext::GetInputMute (void)
{
    return g_pRecordContext->m_fInputMute;
}

MMRESULT HardwareContext::SetInputMute (BOOL fMute)
{
    // TODO: Call down to PMIC driver for codec mute configuration

    // Save input mute configuration
    g_pRecordContext->m_fInputMute = fMute;

    return g_pRecordContext->pm_InputDeviceContext->SetGain(fMute ? 0:
                                                g_pRecordContext->m_dwInputGain);
}

DWORD HardwareContext::GetInputGain (void)
{
    return g_pRecordContext->m_dwInputGain;
}


//-----------------------------------------------------------------------------
//
//  Function:  SetInputGain
//
//
//  Parameters:
//      dwGain.
//
//  Returns:
//      Returns MMSYSERR_NOERROR .
//
//-----------------------------------------------------------------------------
MMRESULT HardwareContext::SetInputGain (DWORD dwGain)
{
    DEBUGMSG(ZONE_FUNCTION,(_T("+HardwareContext::SetInputGain\n")));

    BSPAudioSetInputGain(AUDIO_BUS_VOICE_IN, dwGain);

    g_pRecordContext->m_dwInputGain = dwGain;

    DEBUGMSG(ZONE_FUNCTION,(_T("-HardwareContext::SetInputGain\n")));

    return MMSYSERR_NOERROR;
}

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
//      Returns TRUE .
//
//-----------------------------------------------------------------------------
BOOL HardwareContext::InitInputDMA(PHYSICAL_ADDRESS g_PhysDMABufferAddr)
{
    BOOL rc = FALSE;
    DDK_DMA_ACCESS audioDataRXWordSize = DDK_DMA_ACCESS_8BIT;

#if 0 // Remove-W4: Warning C4127 workaround
    if (sizeof(HWSAMPLE) == sizeof(INT16))
    {
        audioDataRXWordSize = DDK_DMA_ACCESS_16BIT;
    }
    else if (sizeof(HWSAMPLE) == sizeof(INT32))
    {
        audioDataRXWordSize = DDK_DMA_ACCESS_32BIT;
    }
#else
    UINT8 size = sizeof(HWSAMPLE);
    if (size == sizeof(INT16))
    {
        audioDataRXWordSize = DDK_DMA_ACCESS_16BIT;
    }
    else if (size == sizeof(INT32))
    {
        audioDataRXWordSize = DDK_DMA_ACCESS_32BIT;
    }
#endif

    DEBUGMSG(ZONE_FUNCTION,(_T("+RecordContext::InitInputDMA\r\n")));

    // Check if DMA buffer has been allocated
    if (!g_PhysDMABufferAddr.LowPart)
    {
        ERRORMSG(ZONE_ERROR, (_T("Invalid DMA buffer physical address.\r\n")));
        goto cleanUp;
    }

    // Configure the platform-specific input DMA channel
    if (!BSPAudioInitInput(AUDIO_BUS_VOICE_IN))
    {
        ERRORMSG(ZONE_ERROR, (_T("BSPAudioInitInput failed.\r\n")));
        goto cleanUp;
    }

    // Allocate a DMA chain for the virtual channel to handle the DMA transfer
    // requests.
    if (!DDKSdmaAllocChain(g_pRecordContext->m_InputDMAChan, NUM_DMA_BUFFERS))
    {
        ERRORMSG(ZONE_ERROR, (_T("DDKSdmaAllocChain failed.\r\n")));
        goto cleanUp;
    }

    // Configure buffer descriptors
    if (!DDKSdmaSetBufDesc(g_pRecordContext->m_InputDMAChan,
                           0,
                           DDK_DMA_FLAGS_INTR | DDK_DMA_FLAGS_CONT,
                           g_PhysDMABufferAddr.LowPart +
                               (2 * audioDMAPageSize),
                           0,
                           audioDataRXWordSize,
                           audioDMAPageSize))
    {
        ERRORMSG(ZONE_ERROR, (_T("DDKSdmaSetBufDesc failed.\r\n")));
        goto cleanUp;
    }

    if (!DDKSdmaSetBufDesc(g_pRecordContext->m_InputDMAChan,
                           1,
                           DDK_DMA_FLAGS_INTR | DDK_DMA_FLAGS_CONT |
                               DDK_DMA_FLAGS_WRAP,
                           g_PhysDMABufferAddr.LowPart +
                               (3 * audioDMAPageSize),
                           0,
                           audioDataRXWordSize,
                           audioDMAPageSize))
    {
        ERRORMSG(ZONE_ERROR, (_T("DDKSdmaSetBufDesc failed.\r\n")));
        goto cleanUp;
    }

    rc = TRUE;

cleanUp:
    if (!rc)
    {
        DeinitInputDMA();
    }

    DEBUGMSG(ZONE_FUNCTION,(_T("-RecordContext::InitInputDMA\r\n")));

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
//      Returns TRUE .
//
//-----------------------------------------------------------------------------
BOOL HardwareContext::DeinitInputDMA()
{
    DEBUGMSG(ZONE_FUNCTION,(_T("+RecordContext::DeinitInputDMA\n")));

    // Stop the audio input devices
    BSPAudioStopInput(AUDIO_BUS_VOICE_IN, AUDIO_PATH_MIC);

    if (g_pRecordContext->m_InputDMAChan != 0)
    {
        // Kill the input DMA channel
        DDKSdmaStopChan(g_pRecordContext->m_InputDMAChan, TRUE);
        g_pRecordContext->m_InputDMAChan = 0;
    }

    DEBUGMSG(ZONE_FUNCTION,(_T("-RecordContext::DeinitInputDMA\n")));

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
//      Returns TRUE .
//
//-----------------------------------------------------------------------------
BOOL HardwareContext::StartInputDMA()
{

    BOOL rc = FALSE;

    DEBUGMSG(ZONE_FUNCTION,(_T("+RecordContext::StartInputDMA\r\n")));

    // Must acquire critical section Lock() here so that we don't have a race
    // condition with StopInputDMA() and the interrupt handler.
    Lock();

    // Terminate the existing record CODEC disable timer since we're about
    // to start another recording operation.
    if (g_pRecordContext->m_AudioDelayDisableTimerID != NULL)
    {
        timeKillEvent(g_pRecordContext->m_AudioDelayDisableTimerID);
        g_pRecordContext->m_AudioDelayDisableTimerID = NULL;
    }

    // Reinitialize audio input if it is not already active.
    if(!g_pRecordContext->m_InputDMARunning)
    {
        // Initialize the input DMA state variables.
        g_pRecordContext->m_InputDMARunning = TRUE;
        g_pRecordContext->m_InputDMAStatus  = (DWORD)~(DMA_DONEA | DMA_DONEB);

        // Reset the state of the input DMA chain.
        //
        // Note that the second "watermark" argument to DDKSdmaInitChain()
        // is actually in bytes whereas "m_InputDMALevel" is given in
        // SSI FIFO slots or words. Therefore, we have to multiply by the
        // audio data word size to ensure that we are properly emptying
        // the SSI RX FIFO.
        //
        if(!DDKSdmaInitChain(g_pRecordContext->m_InputDMAChan,
                             g_pRecordContext->m_InputDMALevel
                                                                          * sizeof(HWSAMPLE)))
        {
            DEBUGMSG(ZONE_ERROR, (_T("RecordContext::StartInputDMA() - ")
                      _T("Unable to initialize input DMA channel!\r\n")));

            // Must reset the m_InputDMARunning state variable to FALSE
            // if we encounter an error here.
            g_pRecordContext->m_InputDMARunning = FALSE;

            goto START_ERROR;
        }

        // Start the input DMA.
        DDKSdmaStartChan(g_pRecordContext->m_InputDMAChan);

        // Enable audio input devices. This must be done after we activate the
        // input DMA channel because the audio hardware will immediately start
        // recording when this call is made.
        BSPAudioStartInput(AUDIO_BUS_VOICE_IN, AUDIO_PATH_MIC);
    }

    rc = TRUE;

START_ERROR:
    Unlock();

    DEBUGMSG(ZONE_FUNCTION,(TEXT("-RecordContext::StartInputDMA\r\n")));

    return rc;
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
    DEBUGMSG(ZONE_FUNCTION,(_T("+RecordContext::StopInputDMA\n")));

    // If the input DMA is running, stop it
    if (g_pRecordContext->m_InputDMARunning)
    {
        // Reset input DMA state variables
        g_pRecordContext->m_InputDMAStatus  = DMA_CLEAR;
        g_pRecordContext->m_InputDMARunning = FALSE;

        // Disable audio input devices
        BSPAudioStopInput(AUDIO_BUS_VOICE_IN, AUDIO_PATH_MIC);

        // Kill the output DMA channel
        DDKSdmaStopChan(g_pRecordContext->m_InputDMAChan, FALSE);
    }

    DEBUGMSG(ZONE_FUNCTION,(_T("-RecordContext::StopInputDMA\n")));
}


//-----------------------------------------------------------------------------
//
//  Function:  TransferInputBuffer
//
//  This function retrieves the chunk of recorded sound data and inputs
//  it into an audio buffer for potential "mixing".
//
//  Parameters:
//      NumBuf
//          [in] Input DMA page to be filled.
//
//  Returns:
//      Returns number of bytes placed into input buffer.
//
//-----------------------------------------------------------------------------
ULONG RecordContext::TransferInputBuffer(ULONG NumBuf)
{
    ULONG BytesTransferred = 0;

    PBYTE pBufferStart = m_Input_pbDMA_PAGES[NumBuf];
    PBYTE pBufferEnd = pBufferStart + audioDMAPageSize;
    PBYTE pBufferLast;

    __try
    {
        pBufferLast = pm_InputDeviceContext->TransferBuffer(pBufferStart,
                                                          pBufferEnd,NULL);
        BytesTransferred = pBufferLast - pBufferStart;

        // We will mark the input DMA buffer as being in use again regardless
        // of whether or not the data was completely copied out to an
        // application-supplied data buffer. The input DMA buffer must be
        // immediately reused because the audio recording operation is still
        // running.
        if(NumBuf == IN_BUFFER_A)
        {
            m_InputDMAStatus &= ~DMA_DONEA;
        }
        else
        {
            m_InputDMAStatus &= ~DMA_DONEB;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("WAVDEV2.DLL:TransferInputBuffer() - ")
                              TEXT("EXCEPTION: %d"), GetExceptionCode()));
    }

    return BytesTransferred;
}


//-----------------------------------------------------------------------------
//
//  Function:  TransferInputBuffers
//
//  This function determines which DMA input buffer (A and/or B) needs to be
//  transferred to the audio application-supplied data buffers.  The DMA input
//  buffer can be reused once the data has been copied out.
//
//  If there is insufficient space in the application-supplied data buffers to
//  transfer out all of the data that is currently in the DMA input buffers,
//  then all excess data is simply discarded.
//
//  Parameters:
//      dwDCSR
//          [in] Current status of the input DMA.
//      checkFirst
//          [in] Identifies which of the two available DMA input buffers
//               should be transferred first in order to maintain the
//               correct sequencing of the input audio stream.
//
//  Returns:
//      Returns number of bytes placed into the input buffers.
//
//-----------------------------------------------------------------------------
ULONG RecordContext::TransferInputBuffers(DWORD dwDCSR, DWORD checkFirst)
{
    ULONG BytesTransferred=0;

    if (checkFirst == IN_BUFFER_A)
    {
        if (dwDCSR & DMA_DONEA)
        {
            BytesTransferred = TransferInputBuffer(IN_BUFFER_A);
        }

        if (dwDCSR & DMA_DONEB)
        {
            BytesTransferred += TransferInputBuffer(IN_BUFFER_B);
        }
    }
    else    // (checkFirst == IN_BUFFER_B)
    {
        if (dwDCSR & DMA_DONEB)
        {
            BytesTransferred = TransferInputBuffer(IN_BUFFER_B);
        }

        if (dwDCSR & DMA_DONEA)
        {
            BytesTransferred += TransferInputBuffer(IN_BUFFER_A);
        }
    }

    return BytesTransferred;
}


//-----------------------------------------------------------------------------
//
//  Function:  InterruptRecordThread
//
//  This function Contains the interrupt handling required for Recording.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void HardwareContext::InterruptRecordThread()
{
        DWORD recCheckFirst = IN_BUFFER_A;
        UINT32 bufDescStatus[NUM_DMA_BUFFERS];

        // Handle DMA input (i.e., audio recording) if it is active.
        if (g_pRecordContext->m_InputDMARunning)
        {
             PCSP_SSI_REG pVCodecSSI = BSPAudioGetVCodecSSI();
             DWORD ssi_sisr = INREG32(&pVCodecSSI->SISR);
             if (ssi_sisr & CSP_BITFMASK(SSI_SISR_TUE0))
             {
             // Don't expect to see this ever if the SSI is used only
             // for audio recording using the PMIC Voice CODEC.
                    ERRORMSG(ZONE_ERROR, (_T("ERROR: SSI%d TX underrun ")
                                    _T("error\n"),
                                    (1)));
             }

           // Get the transfer status of the DMA input buffers.
           if (!DDKSdmaGetChainStatus(g_pRecordContext->m_InputDMAChan, bufDescStatus))
           {
                 ERRORMSG(ZONE_ERROR,(_T("Could not retrieve input buffer ")
                                _T("status\r\n")));
           }

           if (bufDescStatus[IN_BUFFER_A] & DDK_DMA_FLAGS_ERROR)
          {
                 ERRORMSG(ZONE_ERROR,(_T("ERROR:  Error flag set in ")
                               _T("IN_BUFFER_A\r\n")));
          }

          if (bufDescStatus[IN_BUFFER_B] & DDK_DMA_FLAGS_ERROR)
          {
                 ERRORMSG(ZONE_ERROR,(_T("ERROR:  Error flag set in ")
                                _T("IN_BUFFER_B\r\n")));
          }

          // Set DMA status bits according to retrieved transfer status.
          if (!(bufDescStatus[IN_BUFFER_A] & DDK_DMA_FLAGS_BUSY))
          {
          // Mark input buffer A as having new audio data.
                g_pRecordContext->m_InputDMAStatus |= DMA_DONEA;
                DDKSdmaClearBufDescStatus(g_pRecordContext->m_InputDMAChan,
                                                                                                    IN_BUFFER_A);
          }

          if (!(bufDescStatus[IN_BUFFER_B] & DDK_DMA_FLAGS_BUSY))
          {
          // Mark input buffer B as having new audio data.
                g_pRecordContext->m_InputDMAStatus |= DMA_DONEB;
                DDKSdmaClearBufDescStatus(g_pRecordContext->m_InputDMAChan,
                                                                                                     IN_BUFFER_B);
           }

         // Try to empty out the input DMA buffers by copying the data
         // up to the application-supplied data buffers.
         //
         // Note that we require the higher-level audio application
         // to provide enough data buffers to transfer all of the new
         // data from the input DMA buffers. Otherwise, we will be
         // forced to discard the data and just continue recording.
               g_pRecordContext->TransferInputBuffers(g_pRecordContext->m_InputDMAStatus,
                                                                                                                        recCheckFirst);

        // Update the recCheckFirst flag to mark the first input DMA
        // buffer that should be transferred in order to maintain the
        // correct ABABABAB... input DMA buffer sequencing.
        //
        // Note that except for the 2 cases that we check for below, we
        // do not need to change the current value of the recCheckFirst
        // flag. Here are some pseudocode blocks to explain why:
        //
        //     if ((recCheckFirst == IN_BUFFER_A) &&
           //         (m_InputDMAStatus & DMA_DONEA) &&
           //         (m_InputDMAStatus & DMA_DONEB))
           //     {
           //         // This means that both input DMA buffers A and
           //         // B were full and that the data was copied out
           //         // during the handling of this interrupt. So we
           //         // can just continue the input DMA operation in
           //         // the same state.
           //     }
           //     if ((recCheckFirst == IN_BUFFER_A)  &&
           //         !(m_InputDMAStatus & DMA_DONEA) &&
           //         !(m_InputDMAStatus & DMA_DONEB))
           //     {
           //         // This is an invalid or error condition that
           //         // should never happen because it indicates that
           //         // we got an input DMA interrupt but neither
           //         // input buffer was full.
           //     }
           //     if ((recCheckFirst == IN_BUFFER_A)  &&
           //         !(m_InputDMAStatus & DMA_DONEA) &&
           //         (m_InputDMAStatus & DMA_DONEB))
           //     {
           //         // This is an invalid or error condition that
           //         // should never happen because it indicates that
           //         // we've lost our expected ABABABAB... buffer
           //         // sequencing.
           //     }
           //     if ((recCheckFirst == IN_BUFFER_A) &&
           //         (m_InputDMAStatus & DMA_DONEA) &&
           //         !(m_InputDMAStatus & DMA_DONEB))
           //     {
           //         // This is the only scenario that we need to
           //         // check for (see the code below).
           //     }
           //
           // The same logic can be applied for the cases where the
           // recCheckFirst flag is equal to IN_BUFFER_B.
           if ((recCheckFirst == IN_BUFFER_A) &&
              (g_pRecordContext->m_InputDMAStatus & DMA_DONEA) &&
              !(g_pRecordContext->m_InputDMAStatus & DMA_DONEB))
           {
           // Input DMA buffer A was full and just transferred but
           // input DMA buffer B was not yet full so we must check
           // it first when handling the next input DMA interrupt.
                 recCheckFirst = IN_BUFFER_B;
           }
           else if ((recCheckFirst == IN_BUFFER_B) &&
                    (g_pRecordContext->m_InputDMAStatus & DMA_DONEB) &&
                    !(g_pRecordContext->m_InputDMAStatus & DMA_DONEA))
           {
           // Input DMA buffer B was full and just transferred but
           // input DMA buffer A was not yet full so we must check
           // it first when handling the next input DMA interrupt.
                 recCheckFirst = IN_BUFFER_A;
           }

          // Check for SSI receiver overrun errors. These are
          // bad and can result in unpredictable swapping of the
          // left/right audio channels.
           if (ssi_sisr & CSP_BITFMASK(SSI_SISR_ROE0))
           {
                  ERRORMSG(ZONE_ERROR, (_T("ERROR: SSI%d RX overrun ")
                                   _T("error\n"),
                                   (1)));
           }

           // Force the input DMA channel to be enabled again since we
           // now have at least one input DMA buffer that's ready to be
           // used.
                  DDKSdmaStartChan(g_pRecordContext->m_InputDMAChan);
     }

}



//-----------------------------------------------------------------------------
//
//  Function:  GetNumInputDevices
//
//  This function returns the number of Input device.
//
//  Parameters:
//      None.
//
//  Returns:
//      The number of input devices.
//
//-----------------------------------------------------------------------------

DWORD RecordContext::GetNumInputDevices(void)
{
        return 1; // Recording Enabled
}
