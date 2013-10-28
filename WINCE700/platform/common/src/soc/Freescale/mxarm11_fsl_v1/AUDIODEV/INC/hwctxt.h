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
#pragma once
//
//
//
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.


Module Name:    HWCTXT.H

Abstract:       Platform dependent code for the mixing audio driver.

Environment:    Freescale Power Management ICs with WinCE 5.0 or later.

-*/
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2007, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

#include "mxarm11.h"
#include <ceddk.h> // Needed for typedef of PHYSICAL_ADDRESS

//-----------------------------------------------------------------------------
// Defines for audio hardware capabilities.


#define OUTCHANNELS   (2)    // PMIC Stereo DAC has two output channels.
#define BITSPERSAMPLE (16)    // Voice CODEC and Stereo DAC uses 16 bits/sample

#define INSAMPLERATE  (16000)    // Voice CODEC input at 16 kHz.
#define OUTSAMPLERATE (44100)    // Stereo DAC output at 44.1 kHz.

// Inverse sample rate, in .32 fixed format, with 1 added at bottom to ensure
// round up.
#define INVSAMPLERATE ((UINT32)(((1i64<<32)/OUTSAMPLERATE)+1))

// Define the size of each audio data word. The valid choices are:
//
//     INT8    for 8 bits/word
//     INT16   for 16 bits/word
//     INT32   for 24 bits/word
//
// The correct value to use depends upon the audio CODEC hardware. Note that
// the SSI hardware only supports a maximum of 24 bits/word.
//
// The required definition for the MC13783 and SC5512 PMICs is INT16.
//
// The word size that is defined here is also used to set the corresponding
// SDMA and SSI transfer sizes.
//
typedef INT16 HWSAMPLE;
typedef HWSAMPLE *PHWSAMPLE;

// Set USE_MIX_SATURATE to 1 if you want the mixing code to guard against
// saturation. This costs a couple of extra instructions in the inner
// signal resampling/mixing loop.
#define USE_MIX_SATURATE    (1)

// The code will use the follwing values as saturation points. These values
// should match the word size defined for HWSAMPLE above.
#define AUDIO_SAMPLE_MAX    (32767)
#define AUDIO_SAMPLE_MIN    (-32768)

// Size in bytes of each DMA buffer. We allocate 2 DMA buffers each for audio
// playback and recording.
#define AUDIO_DMA_PAGE_SIZE     4096 // Default 4096 bytes.

//----- Used to track DMA controllers status -----
#define DMA_CLEAR           0x00000000
#define DMA_DONEA           0x00000002
#define DMA_DONEB           0x00000004
#define DMA_BIU             0x00000008 // Determines which buffer is in use:
                                       // A=0, B=1.
#define DMA_STOP            0x00000010 // Stop DMA after buffers exhausted

//----- Used for scheduling DMA transfers -----
#define  OUT_BUFFER_A       0
#define  OUT_BUFFER_B       1

#define AUDIO_REGKEY_PREFIX TEXT("Drivers\\BuiltIn\\Audio\\PMIC\\Config")

//------------Globals---------------------

//------------------- Board Support Package Interface ----------------------


class HardwareContext
{
public:
    static BOOL CreateHWContext(DWORD Index);
    HardwareContext();
    ~HardwareContext();

    void Lock()   {EnterCriticalSection(&m_Lock);}
    void Unlock() {LeaveCriticalSection(&m_Lock);}

    DWORD GetNumOutputDevices() {return 1;}
    DWORD GetNumMixerDevices()  {return 1;}

    DeviceContext *GetOutputDeviceContext(UINT DeviceId)
    {
        // Remove-W4: Warning C4100 workaround
        UNREFERENCED_PARAMETER(DeviceId);
        return &m_OutputDeviceContext;
    }

    BOOL Init(DWORD Index);
    BOOL Deinit();

    void PowerUp();
    void PowerDown();

    BOOL        StartOutputDMA();
    void        StopOutputDMA();
    DWORD       GetOutputGain (void);
    MMRESULT    SetOutputGain (DWORD dwVolume);
    BOOL        GetOutputMute (void);
    MMRESULT    SetOutputMute (BOOL fMute);

    void InterruptThread();

    void DisableDelayThread();

protected:

    typedef enum
    {
      AUDIO_BUS_STEREO_OUT,
      AUDIO_BUS_VOICE_IN
    } AUDIO_BUS;

    typedef enum
    {
      AUDIO_PATH_EARPIECE,
      AUDIO_PATH_SPEAKER,
      AUDIO_PATH_HEADSET,
      AUDIO_PATH_LINEOUT,
      AUDIO_PATH_LINEIN,
      AUDIO_PATH_MIC
    } AUDIO_PATH;


    typedef enum
    {
      AUDIO_PWR_STATE_OFF,
      AUDIO_PWR_STATE_STANDBY,
      AUDIO_PWR_STATE_ON
    } AUDIO_PWR_STATE;


   typedef enum
   {
      PORT1 = 0,
      PORT2 = 1,
      PORT3 = 2,
   } AUDMUX_INTERNAL_PORT;

   typedef enum
   {
      PORT4 = 3,
      PORT5 = 4,
      PORT6 = 5,
      PORT7 = 6,
   } AUDMUX_EXTERNAL_PORT;


    // Define the timed delay interval that will be used between the completion
    // of an audio I/O operation and the disabling of the audio CODEC hardware.
    // On some hardware platforms, enabling the audio CODEC hardware can require
    // a significant ramp-up time (up to 150 ms in the case of the MC13783 PMIC).
    // Therefore, when back-to-back audio I/O operations are performed, it is
    // best to just leave the audio CODEC hardware enabled in between the two
    // audio streams.
    //
    // The delay interval that is defined here will be used to determine how
    // long after an audio stream has been completed before the audio CODEC
    // hardware is actually disabled. If another audio stream operation is
    // started, before the interval has expired, then the timeout is simply
    // cancelled and the audio CODEC will not need to be reenabled for the
    // second audio stream.
    //
    // For the MC13783 PMIC, a delay of 1 sec is recommended since that should
    // give enough time to perform back-to-back audio I/O operations. We don't
    // want the delay interval to be too long, however, or else we end up
    // wasting power by keeping the audio CODEC enabled when it is not being
    // used.
    //
    // Making the delay interval shorter than 1 sec is also not recommended
    // since that can cause the audio CODEC to be disabled just before another
    // audio stream is processed. This would eliminate any benefit of having
    // this delayed CODEC disable feature.
    //
    // Note that for audio CODECs that don't have any noticeable ramp-up or
    // enable delay period, a timer value of 0 is allowed. Setting the timer
    // delay interval to 0 will immediately disable the audio CODEC upon the
    // completion of each audio stream I/O operation.


    DWORD m_DriverIndex;
    CRITICAL_SECTION m_Lock;

    BOOL m_Initialized;

    static const int NUM_DMA_BUFFERS = 2; // For DMA transfers to/from the
                                          // audio hardware we need to allocate
                                          // 2 buffers and we just alternate
                                          // between them. Note that we allocate
                                          // separate pairs of DMA buffers for
                                          // output/playback and for
                                          // input/recording.

    static const int INTR_PRIORITY = 249; // Default interrupt thread priority.
    static const unsigned short INTR_PRIORITY_REGKEY[12];



    OutputDeviceContext m_OutputDeviceContext;

    DWORD m_dwOutputGain;
    BOOL  m_fOutputMute;

    PBYTE m_Output_pbDMA_PAGES[NUM_DMA_BUFFERS];
    BOOL  m_bCanStopOutputDMA;
    BOOL  m_OutputDMARunning;
    WORD  m_nOutputVolume;                   // Current HW Playback Volume.
                                             // Volume.

    BOOL  m_bSSITXUnderrunError;             // Set to TRUE if an SSI TX
                                             // underrun error has occurred
                                             // and we must now perform a
                                             // recovery operation.

    BOOL m_audioPowerdown;  // Flag to indicate if PowerDown() has been called.

    DWORD m_MicrophoneRouting;
    DWORD m_SpeakerRouting;
    DWORD m_InternalRouting;
    DWORD m_MasterOutputGain;

    BOOL InitInterruptThread();

    BOOL InitOutputDMA();
    BOOL DeinitOutputDMA();
    ULONG TransferOutputBuffer(const PBYTE pBufferStart,
                               const PBYTE pBufferEnd);
    ULONG TransferOutputBuffers(DWORD dwDCSR);

    BOOL MapRegisters();
    BOOL UnmapRegisters();
    BOOL MapDMABuffers();
    BOOL UnmapDMABuffers();

    DWORD GetInterruptThreadPriority();

    HANDLE m_hAudioInterrupt;                // Handle to Audio Interrupt event.
    HANDLE m_hAudioInterruptThread;          // Handle to thread which waits on
                                             // an audio interrupt event.

    HANDLE m_hAudioDelayedDisableThread;     // Handle for thread that will
                                             // process all timeout events from
                                             // the CODEC delayed disable timer.

    // Declare a timer ID that will be used to control the timed delay when
    // disabling the audio hardware. This allows better performance when
    // back-to-back audio operations are performed.
    HANDLE   m_hAudioStopOutputDMA;
    HANDLE   m_hAudioDelayDisableEvent;
    MMRESULT m_AudioDelayDisableTimerID;

//-----------Recording related functions---------------------------------------
/*We need to include some recording functions here since they make BSP specific
 *calls*/
public:

    BOOL  InitInputDMA(PHYSICAL_ADDRESS g_PhysDMABufferAddr);
    BOOL  DeinitInputDMA();
    BOOL        StartInputDMA();
    void        StopInputDMA();
    DWORD       GetInputGain (void);
    MMRESULT    SetInputGain (DWORD dwVolume);
    BOOL        GetInputMute (void);
    MMRESULT    SetInputMute (BOOL fMute);
    void InterruptRecordThread();

    //-------------------- Platform specific members ---------------------------

    volatile DWORD  m_OutputDMAStatus;       // Output DMA channel's status
    UINT8  m_OutputDMAChan;                  // Ouput DMA virtual channel index
    UINT32 m_OutputDMALevel;                 // Ouput DMA watermark level

    PCSP_SSI_REG m_pSSI1;
    DWORD m_dwSysintrSSI1;
    PCSP_SSI_REG m_pSSI2;
    DWORD m_dwSysintrSSI2;

    PCSP_AUDMUX_REG m_pAUDMUX;

    AUDIO_PWR_STATE m_CodecPwrState;

    BOOL BSPAudioInit();
    BOOL BSPAudioDeinit();

    PBYTE BSPAudioAllocDMABuffers(PHYSICAL_ADDRESS *pPhysicalAddress);
    void BSPAudioDeallocDMABuffers(PVOID virtualAddress);

    void BSPAudioInitCodec(AUDIO_BUS bus);
    void BSPAudioSetCodecPower(AUDIO_PWR_STATE state);

    void BSPAudioPowerUp(const BOOL fullPowerUp);
    void BSPAudioPowerDown(const BOOL fullPowerOff);

    void BSPAudioInitSsi(AUDIO_BUS bus, PCSP_SSI_REG pSSI);

    void BSPAudioRoute(AUDMUX_INTERNAL_PORT intPort,
                       AUDMUX_EXTERNAL_PORT extPort,
                       BOOL bMaster);

    PCSP_SSI_REG BSPAudioGetStDACSSI();

    BOOL BSPAudioInitOutput(AUDIO_BUS bus);
    BOOL BSPAudioStartOutput(AUDIO_BUS bus, AUDIO_PATH path,
                             const HWSAMPLE *const ssiFifoPrefill,
                             const unsigned nSsiFifoPrefill);
    BOOL BSPAudioStopOutput(AUDIO_BUS bus, AUDIO_PATH path);
    BOOL BSPAudioSetOutputGain(AUDIO_BUS bus, const DWORD dwGain);

    void BSPAudioStartSsiOutput(PCSP_SSI_REG pSSI,
                                const HWSAMPLE *const ssiFifoPrefill,
                                const unsigned nSsiFifoPrefill);
    void BSPAudioStopSsiOutput(PCSP_SSI_REG pSSI);

    void BSPAudioStartCodecOutput(AUDIO_BUS bus, AUDIO_PATH path);
    void BSPAudioStopCodecOutput(AUDIO_BUS bus, AUDIO_PATH path);



    void BSPAudioStartSsiInput(PCSP_SSI_REG pSSI);
    void BSPAudioStopSsiInput(PCSP_SSI_REG pSSI);

    void BSPAudioStartCodecInput(AUDIO_BUS bus, AUDIO_PATH path);
    void BSPAudioStopCodecInput(AUDIO_BUS bus, AUDIO_PATH path);

//--------------Recording Related Definitions--------

    BOOL BSPAudioInitInput(AUDIO_BUS bus);
    BOOL BSPAudioStartInput(AUDIO_BUS bus, AUDIO_PATH path);
    BOOL BSPAudioStopInput(AUDIO_BUS bus, AUDIO_PATH path);
    BOOL BSPAudioSetInputGain(AUDIO_BUS bus, const DWORD dwGain);
    PCSP_SSI_REG BSPAudioGetVCodecSSI();



};

extern DWORD CALLBACK CallInterruptThread(HardwareContext *pHWContext);
extern DWORD CALLBACK BSPAudioDisableDelayHandler(HardwareContext *pHWContext);

//--------------------------------- Externs ------------------------------------

extern HardwareContext *g_pHWContext;
