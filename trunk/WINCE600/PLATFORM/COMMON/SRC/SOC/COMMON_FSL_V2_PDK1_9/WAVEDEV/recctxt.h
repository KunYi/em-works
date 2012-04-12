//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
#pragma once
//
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.


Module Name:    RECCTXT.H

Abstract:       Chipspecific header file to support recording.

Environment:    Freescale Power Management ICs with WinCE 5.0 or later.

--*/
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2008 Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

#include <ceddk.h> // Needed for typedef of PHYSICAL_ADDRESS

//-----------------------------------------------------------------------------
// Defines for audio hardware capabilities.

#define INCHANNELS    (1)    // The PMIC Voice CODEC has one input channel.
                             //
                             // Note that the MC13783 PMIC actually has 2
                             // different input/recording paths. One path
                             // supports a mono microphone (audio jack J3 on
                             // the PMIC daughtercard) while the second path
                             // supports a stereo microphone (audio jack J4 on
                             // the daughtercard).
                             //
                             // However, since WinCE does not have any built-in
                             // capability to select different audio recording
                             // paths and devices with different capabilities,
                             // the current audio driver implementation simply
                             // uses the lowest common denominator and only
                             // supports mono recording. You may use the [HKLM\
                             // Drivers\BuiltIn\Audio\PMIC\Config\Recording]
                             // registry key to select which audio input jack
                             // is to be used. But only the left input channel
                             // will be recorded in the current device driver
                             // implementation if the stereo input jack is
                             // selected.

#define INSAMPLERATE  (16000)    // Voice CODEC input at 16 kHz.


//----- Used for scheduling DMA transfers -----
#define  IN_BUFFER_A        0
#define  IN_BUFFER_B        1

//-----------------Globals---------------------

class RecordContext
{
public:
    static BOOL CreateRecordContext(DWORD Index);
    RecordContext();
    ~RecordContext();


    void Lock()   {EnterCriticalSection(&m_Lock);}
    void Unlock() {LeaveCriticalSection(&m_Lock);}

    DWORD GetNumInputDevices(void);


    DeviceContext *GetInputDeviceContext(UINT DeviceId)
    {
        // Remove-W4: Warning C4100 workaround
        UNREFERENCED_PARAMETER(DeviceId);
        return pm_InputDeviceContext;
    }

    BOOL Init(DWORD Index);
    BOOL Deinit();


    BOOL m_RecordInitialized;

    CRITICAL_SECTION m_Lock;

    static const int NUM_DMA_BUFFERS = 2; // For DMA transfers to/from the
                                          // audio hardware we need to allocate
                                          // 2 buffers and we just alternate
                                          // between them. Note that we allocate
                                          // separate pairs of DMA buffers for
                                          // output/playback and for
                                          // input/recording.


    InputDeviceContext *pm_InputDeviceContext;

    BOOL m_audioPowerdown;  // Flag to indicate if PowerDown() has been called.

    DWORD m_dwInputGain;
    BOOL  m_fInputMute;

    PBYTE m_Input_pbDMA_PAGES[NUM_DMA_BUFFERS];
    BOOL  m_InputDMARunning;
    WORD  m_nInputVolume;                    // Current HW Input (Microphone)


    volatile DWORD  m_InputDMAStatus;        // Input DMA channel's status
    UINT8  m_InputDMAChan;                   // Input DMA virtual channel index
    UINT32 m_InputDMALevel;                  // Input DMA watermark level


//Declare a timer ID that will be used to control the timed delay when
//disabling the audio hardware. This allows better performance when
//back-to-back audio operations are performed
    MMRESULT m_AudioDelayDisableTimerID;
    HANDLE   m_hAudioDelayDisableEvent;

    /*Map/Unmap the DMA buffers when recording is enabled*/
    BOOL MapRecordDMABuffers(PBYTE pVirtDMABufferAddr);
    BOOL UnmapRecordDMABuffers();

    ULONG TransferInputBuffer(ULONG NumBuf);
    ULONG TransferInputBuffers(DWORD dwDCSR, DWORD checkFirst);


};


//--------------------------------- Externs ------------------------------------

extern RecordContext *g_pRecordContext;
