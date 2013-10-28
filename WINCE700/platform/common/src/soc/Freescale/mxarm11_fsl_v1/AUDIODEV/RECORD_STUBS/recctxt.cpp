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
//  Copyright (C) 2004-2007 Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  recctxt.cpp
//
//    ChipSpecific driver for Audio device. The primary responsibility of this driver is to
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

static BOOL g_saveInputDMARunning  = FALSE;

//-----------------------------------------------------------------------------
// Defines

//-----------------------------------------------------------------------------
//Externs


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
    m_hAudioDelayDisableEvent  = NULL;

    pm_InputDeviceContext = NULL;
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
    UNREFERENCED_PARAMETER(Index);
    DEBUGMSG(ZONE_FUNCTION,(_T("+RecordContext::Init\n")));

    // If we have already initialized this device, return error
    if (m_RecordInitialized)
    {
        return(FALSE);
    }

    m_RecordInitialized = TRUE;
    m_InputDMARunning = FALSE; //No recording

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
    UNREFERENCED_PARAMETER(pVirtDMABufferAddr);
    m_Input_pbDMA_PAGES[0]  = NULL;
    m_Input_pbDMA_PAGES[1]  = NULL;

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
    return TRUE;
}


//-----------------------------------------------------------------------------
//

BOOL HardwareContext::GetInputMute (void)
{
    return TRUE;
}

MMRESULT HardwareContext::SetInputMute (BOOL fMute)
{
    UNREFERENCED_PARAMETER(fMute);

    return 1;
}

DWORD HardwareContext::GetInputGain (void)
{
    return 1;
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
     UNREFERENCED_PARAMETER(dwGain);
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
    UNREFERENCED_PARAMETER(g_PhysDMABufferAddr);
    return TRUE;
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
    return FALSE;
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
    return;

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
    //Do nothing...Only a STUB
    return;
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
    return 0; // Recording disabled
}


