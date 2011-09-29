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
/*++

$Workfile: WAVEPDD.C $

$Date: 2/03/04 10:11a $

Abstract:
      The PDD (Platform Dependent Driver) is responsible for
      communicating with the audio circuit to start and stop playback
      and/or recording and initialize and deinitialize the circuits. The
      API between the PDD and the MDD is called the DDSI (see "Key Routines"
      for more info).

Warning:
      This driver is specifically written for the UCB 1400 Audio Codec.  It is
      not a general AC 97 CODEC device driver.

Key Routines:

  PDD_AudioGetInterruptType -- This routine is part of the Interrupt Service Thread managed
    by the wave MDD.  Here interrupts are processed and DMA buffers are returned to the driver.
    That is, the DMA buffers are protected to keep the DMAC from reading or writing the buffers
    while the driver is processing them.  If the DMAC attempts to work on a buffer that is has
    not been posted by the driver an Interrupt will occur indicating write overflow or read
    under flow.  The DMAC engine will stall until the buffer is ready.  The status of reads or
    writes is returned to the MDD.

  private_WaveInStart, private_WaveOutStart -- These routines setup the DMAC for receive or
    transmit.  They copy data from application buffers (sent from or to the OS) to DMA buffers.
    The inital DMA buffers are then "posted" or setup for the DMAC engine to read or write.

  private_WaveInContinue, private_WaveOutContinue -- These routines continue processing
    input or output by moving data from or to application buffers from or to the DMAC buffers.
    Buffers are "posted" (marked for handling) by these routines.  If the DMAC engine attempts
    to read a buffer before it is posted, it will generate an interrupt and stall.  If the DMAC
    has stalled this routine will restart DMA.  (See also PDD_AudioGetInterruptType,
    private_AudioFillBuffer, and private_AudioGetBuffer).

  private_AudioFillBuffer,private_AudioGetBuffer -- These routines perform the actual copy and
    formatting of application sound data.  They copy the data from or to the DMAC buffers from
    or to the application buffers.  The data is also converted from the format of the audio
    Codec (stereo 16 bit) to the format requested by the application (such as monoral 8 bit).

  private_WaveInStop, private_WaveOutStop -- These routines stop the processing of audio buffers
    by indicating that no more data is available and setting the DMAC registers to stop xmit or
    recv of application data.

  PDD_WaveProc -- This routine actually processes the messages of the Device Driver Service
    Interface (DDSI).  The MDD sends these messages to the PDD for handling.

  PDD_AudioMessage -- This routine exposes a private interface to applications.

Key Structures:


Function Hierarchy:

     PDD_AudioGetInterruptType      xx
     PDD_AudioMessage               xx
     PDD_AudioInitialize            xx
         AudioPowerOn()             xx
         AudioOutMute();            xx
     PDD_AudioDeinitialize          xx
         PddpAudioDeallocateVm();   xx
     PDD_AudioPowerHandler          xx
     PDD_WaveProc                   xx
         WPDM_OPEN,                 xx
            private_WaveOpen        xx
         WPDM_CLOSE                 xx
         WPDM_START                 xx
             private_WaveInStart    xx
               private_AudioGetBuffer xx
             private_WaveOutStart   xx
               private_AudioFillBuffer !!
         WPDM_RESTART               xx
             private_WaveOutRestart xx
         WPDM_STOP                  xx
             private_WaveInStop     xx
             private_WaveOutStop    xx
         WPDM_CONTINUE              xx
             private_WaveInContinue xx
             private_WaveOutContinue xx
         WPDM_ENDOFDATA             xx
             private_WaveOutEndOfData xx
         WPDM_STANDBY                xx
             private_WaveStandby     --
         WPDM_PAUSE                  xx
             private_WaveOutPause    xx
         WPDM_GETDEVCAPS             xx
             private_WaveGetDevCaps  xx
         WPDM_GETVOLUME
         WPDM_SETVOLUME

Notes:



--*/

static const char __copyright[]= "Copyright 2000 Intel Corp."; //only for C/CPP files

/*++

** Copyright 2000-2001 Intel Corporation. All Rights Reserved.
**
** Portions of the source code contained or described herein and all documents
** related to such source code (Material) are owned by Intel Corporation
** or its suppliers or licensors and is licensed by Microsoft Corporation for distribution.
** Title to the Material remains with Intel Corporation or its suppliers and licensors.
** Use of the Materials is subject to the terms of the Microsoft license agreement which accompanied the Materials.
** No other license under any patent, copyright, trade secret or other intellectual
** property right is granted to or conferred upon you by disclosure or
** delivery of the Materials, either expressly, by implication, inducement,
** estoppel or otherwise
** Some portion of the Materials may be copyrighted by Microsoft Corporation.

--*/

//-----------------------------------------------------------------
// INCLUDE FILES
//-----------------------------------------------------------------

#include <windows.h>
#include <types.h>
#include <memory.h>
#include <excpt.h>
#include <wavepdd.h>
#include <waveddsi.h>
#include <wavedbg.h>
#include <nkintr.h>
#include <ceddk.h>
#include <devload.h>
#include <bulverde.h>
#include "mmsystem.h"
#include <wavedbg.h>
#include "aclink.h"
#include "dmac.h"
#include "AC97.H"
#include "WAVE162.H"

//-----------------------------------------------------------------
// MACROS
//-----------------------------------------------------------------

#define BITS_8_TO_16(x)      (short) ( ( (LONG) ( (UINT8)(x) - 128) ) << 8 )
#define BITS_16_TO_8(x)       ( (UINT8) (( (x) >> 8 ) +128))

//-----------------------------------------------------------------
// DEFINES
//-----------------------------------------------------------------
#define NUM_DMA_AUDIO_BUFFERS    4

#define LOOPBACK_MASK 0x40
#define RESET_CODEC_MASK 0x04 //bit 2 0 origin

#define  MHZ11_980 11980000
#define  KHZ08_000 8000
#define  KHZ11_025 11025
#define  KHZ12_000 12000
#define  KHZ16_000 16000
#define  KHZ22_050 22050
#define  KHZ24_000 24000
#define  KHZ32_000 32000
#define  KHZ44_100 44100
#define  KHZ48_000 48000

#define   TIMERTICK   4  // 1 microsecond is 3.7 clock ticks

#define BULVERDE_REG_SYSINTR_VAL_NAME TEXT("Sysintr")
#define BULVERDE_REG_SYSINTR_VAL_LEN  sizeof( DWORD )

// TODO
#define DEV_TOUCH               1
#define DEV_AUDIO               2
#define DEV_BATTERY             3


//-----------------------------------------------------------------
// INTERNAL FUNCTIONS
//-----------------------------------------------------------------

// Local
static BOOL        MapDMADescriptors(void);
static BOOL        MapDeviceRegisters(void);
static void        FillDescriptors(void);


BOOL        AC97_SetVolume(ULONG uVolume);
short int   AC97SetSampleRate(unsigned short int SampleRate, WAPI_INOUT apidir );
BOOL        audio_out_buffer(ULONG);
BOOL        AudioInMute(   BOOL mute   );
BOOL        AudioOutMute(   BOOL mute   );
void        AudioPowerOff();
BOOL        AudioPowerOn();
void        ClearDmac( void );
void        dumpInterrupt(void);
BOOL        InitDMAC(int);
BOOL        IsMuted(BYTE Offset);
void        PowerDownUnit(unsigned short int Unit,BOOL ShutDown);
void        private_WaveStandby(WAPI_INOUT apidir);
short int   ShadowReadAC97(BYTE Offset, unsigned short int *Data, BYTE DevId);
short int   ShadowWriteAC97(BYTE Offset, unsigned short int Data, BYTE DevId);
BOOL        StopDmac(int);
BOOL        UnMapVirtual(void * v_pMapSpace);
VOID        private_WaveOutStop();
unsigned short int LastSampleRateIn=KHZ48_000;
unsigned short int LastSampleRateOut=KHZ48_000;

//-----------------------------------------------------------------
// EXTERNAL FUNCTIONS
//-----------------------------------------------------------------

extern PVOID VirtualAllocCopy(unsigned size,char *str,PVOID pVirtualAddress);

//-----------------------------------------------------------------
// CONSTANTS AND VARIABLES
//-----------------------------------------------------------------
unsigned long  Ac97ShadowRegisters[64]; //shadow writes to the AC97 Codec
static  unsigned int ResetCaps; //HACK: If 0x2a0 it's rev 2a, if its 0x2a then its rev 1b
static  unsigned int CodecType=GENERIC_AC97;
static  BOOL PlayWave = 0;
static  unsigned int    i_buffer            = 0;  // Rcv buffer index
static  unsigned int    o_buffer            = 0;  // Xmt buffer index
static  BOOL    Expect_RcvRup_A;  // TRUE ==> Done_A interrupt expected; FALSE ==> Done_B
static  BOOL    Expect_XmtRup_A;  // TRUE ==> Done_A interrupt expected; FALSE ==> Done_B

InputSourceType gInputSrc=Line;  //Track the input source (microphone or line in)

const ULONG g_dma_buffer_size = AUDIO_BUFFER_SIZE;
DWORD g_capture_buffer_size;
const g_capture_latency = 20; // capture buffers will hold this much data (in milliseconds)

// This lets us know if we're in our power handler routine, so we know
// not to make any system calls (debug messages, etc).
BOOL g_fInPowerHandler;

// however, 44.1 will be about 1.1% off
static int sample_rate_out = KHZ44_100;        //define sample rate globally
static int sample_rate_in  = KHZ44_100;        //define sample rate globally


VOID (*g_pfnFillBuffer) (PINT16 pDstBuffer, PBYTE pSrcBuffer, DWORD dwSamples);
VOID (*g_pfnGetBuffer) (PBYTE pDstBuffer, PINT16 pSrcBuffer, DWORD dwSamples);

//point to the AC97 read/write functions based on
//the needs for the power handler
BOOL (*g_pfnReadAc97)  (BYTE Offset, UINT16 * Data, BYTE DevId);
BOOL (*g_pfnWriteAc97) (BYTE Offset, UINT16 Data, BYTE DevId);

VOID FillBufferM08 (PINT16 pDstBuffer, PBYTE pSrcBuffer, DWORD dwSamples);
VOID FillBufferS08 (PINT16 pDstBuffer, PBYTE pSrcBuffer, DWORD dwSamples);
VOID FillBufferM16 (PINT16 pDstBuffer, PBYTE pSrcBuffer, DWORD dwSamples);
VOID FillBufferS16 (PINT16 pDstBuffer, PBYTE pSrcBuffer, DWORD dwSamples);
VOID GetBufferM08 (PBYTE pDstBuffer, PINT16 pSrcBuffer, DWORD dwSamples);
VOID GetBufferS08 (PBYTE pDstBuffer, PINT16 pSrcBuffer, DWORD dwSamples);
VOID GetBufferM16 (PBYTE pDstBuffer, PINT16 pSrcBuffer, DWORD dwSamples);
VOID GetBufferS16 (PBYTE pDstBuffer, PINT16 pSrcBuffer, DWORD dwSamples);

static int in_counter;

static UINT16 g_VolMatrix[0x64];
volatile PBYTE   dma_page[NUM_DMA_AUDIO_BUFFERS];
volatile PBYTE   dma_page_physical[NUM_DMA_AUDIO_BUFFERS];

ULONG         v_nVolume=0xeeeeeeee; //default to volume

static int max_sample;
static int in_counter;

DWORD gIntrAudio = SYSINTR_UNDEFINED;

//
// These variables may be accessed by different processes or the hardware and
// are likely to change in a time critical fashion. Optimization may cause
// errors if not declared volatile.
//
volatile        BOOL    v_fMoreData[2];


// DMA descriptor buffers.
volatile DMADescriptorChannelType *v_pAudioRcvA_Virtual   = NULL;    // VA: Audio receive buffer A      128 bits (16 byte aligned)
volatile DMADescriptorChannelType *v_pAudioRcvA_Physical  = NULL;    // PA: Audio receive buffer A      128 bits (16 byte aligned)
volatile DMADescriptorChannelType *v_pAudioRcvB_Virtual   = NULL;    // VA: Audio receive buffer B      128 bits (16 byte aligned)
volatile DMADescriptorChannelType *v_pAudioRcvB_Physical  = NULL;    // PA: Audio receive buffer B      128 bits (16 byte aligned)
volatile DMADescriptorChannelType *v_pAudioXmitA_Virtual  = NULL;    // VA: Audio transmit buffer A     128 bits (16 byte aligned)
volatile DMADescriptorChannelType *v_pAudioXmitA_Physical = NULL;    // PA: Audio transmit buffer A     128 bits (16 byte aligned)
volatile DMADescriptorChannelType *v_pAudioXmitB_Virtual  = NULL;    // VA: Audio transmit buffer B     128 bits (16 byte aligned)
volatile DMADescriptorChannelType *v_pAudioXmitB_Physical = NULL;    // PA: Audio transmit buffer B     128 bits (16 byte aligned)
volatile DMADescriptorChannelType *v_pAudioMicA_Virtual   = NULL;    // VA: Audio Microphone buffer A   128 bits (16 byte aligned)
volatile DMADescriptorChannelType *v_pAudioMicA_Physical  = NULL;    // PA: Audio Microphone buffer A   128 bits (16 byte aligned)
volatile DMADescriptorChannelType *v_pAudioMicB_Virtual   = NULL;    // VA: Audio Microphone buffer B   128 bits (16 byte aligned)
volatile DMADescriptorChannelType *v_pAudioMicB_Physical  = NULL;    // PA: Audio Microphone buffer B   128 bits (16 byte aligned)

volatile BULVERDE_DMA_REG     *v_pDMARegs  = NULL;
volatile BULVERDE_AC97_REG    *v_pAC97Regs = NULL;

#define NUM_API_DIRS          2
BOOL g_fInUse[NUM_API_DIRS]; // is apidir in use? if so, fail Open calls
DWORD g_sample_size[NUM_API_DIRS];


void ResetAC97Controller(void)
{
    //reset the AC97 to workaround issue in sighting 46676
    ULONG ulTemp;

    // set GCR.ACOFF - turn controller off to flush FIFOs
    ulTemp = v_pAC97Regs->gcr;
    ulTemp |= 8;
    v_pAC97Regs->gcr = ulTemp; // do it.

    // wait for the ACOFFD (acoff "done") bit to assert.
    while(1)
    {
    ulTemp = v_pAC97Regs->gsr;
    if( ulTemp & 0x8 )
    break;
    }

    // clear GCR.ACOFF - turn controller back on
    ulTemp = v_pAC97Regs->gcr;
    ulTemp &= ~8;
    v_pAC97Regs->gcr = ulTemp;
}



//-----------------------------------------------------------------
// IMPLEMENTATIONS
//-----------------------------------------------------------------

void DumpDmacRegs()
{
    int i;

    DEBUGMSG(ZONE_ERROR, (TEXT( "%x, %x, %x, %x, %x\r\n" ),
        v_pAC97Regs->gsr,
        v_pAC97Regs->posr,
        v_pAC97Regs->pocr,
        v_pAC97Regs->pisr,
        v_pAC97Regs->picr
        ) );


    for (i=0; i< 16; i++)
        DEBUGMSG(ZONE_ERROR, (TEXT( "v_pDMARegs->dcsr[%d]       %x \r\n" ),i,v_pDMARegs->dcsr[i] ) );

    //skip rsvd section rsvd0[44];
    DEBUGMSG(ZONE_ERROR, (TEXT( "v_pDMARegs->dint       %x \r\n" ),v_pDMARegs->dint ) );

    //skip rsvd seciton rsvd1[3];

    for (i=0; i< 39; i++)
        DEBUGMSG(ZONE_ERROR, (TEXT( "v_pDMARegs->drcmr[%d]      %x \r\n" ),i,v_pDMARegs->drcmr[i] ) );

    for (i=0; i<16; i++)
    {

        DEBUGMSG(ZONE_ERROR, (TEXT( "v_pDMARegs->ddg[%d].ddadr  %x \r\n" ),i,v_pDMARegs->ddg[i].ddadr ) );
        DEBUGMSG(ZONE_ERROR, (TEXT( "v_pDMARegs->ddg[%d].dsadr  %x \r\n" ),i,v_pDMARegs->ddg[i].dsadr ) );
        DEBUGMSG(ZONE_ERROR, (TEXT( "v_pDMARegs->ddg[%d].dtadr  %x \r\n" ),i,v_pDMARegs->ddg[i].dtadr ) );
        DEBUGMSG(ZONE_ERROR, (TEXT( "v_pDMARegs->ddg[%d].dcmd   %x \r\n" ),i,v_pDMARegs->ddg[i].dcmd ) );


    }

}

void ClearDmacRegs()
{
    int i;
    for (i=0; i< 16; i++)
        v_pDMARegs->dcsr[i]=0;


    //skip rsvd section rsvd0[44];

    //skip rsvd seciton rsvd1[3];

    for (i=0; i< 40; i++)
        v_pDMARegs->drcmr[i]=0x0;

    for (i=0; i<16; i++)
    {
        v_pDMARegs->dcsr[i]=0x00000000;  //no descriptor fetch
        v_pDMARegs->ddg[i].ddadr = 0;  //write an address and see if it comes back
        v_pDMARegs->ddg[i].dsadr = 0;
        v_pDMARegs->ddg[i].dtadr = 0;
        v_pDMARegs->ddg[i].dcmd = 0;

    }

}


void Ac97RegDump()
{
    BYTE i;
    short int retval=FALSE;

    unsigned short int Data;

        for (i=0; i<0x7e; i+=2)
        {
            retval= ShadowReadAC97(i, &Data , DEV_AUDIO );

            DEBUGMSG(ZONE_ERROR,(TEXT("reg %x is %x ---"),i,Data));
            if ((i %4) == 0)
                DEBUGMSG(ZONE_ERROR,(TEXT("\r\n")));

        }
}

//------------------------------------------------------------------------------------------------------------
// Function: PDD_AudioGetInterruptType
//
// Purpose: Process an audio interrupt from the MDD
//
// Returns: AUDIO_STATE, indicates the state of audio playback or record to the MDD
//
//-------------------------------------------------------------------------------------------------------------
AUDIO_STATE
PDD_AudioGetInterruptType(
   VOID
   )
{

    AUDIO_STATE retval=AUDIO_STATE_IGNORE ;

    DEBUGMSG(ZONE_VERBOSE, (TEXT( "+PDD_AudioGetInterruptType\r\n" )) );

    //
    // An audio interrupt has occured. We need to tell the MDD who owns it
    // and what state that puts us in.
    //
    // Note, that you can return both an input and an output state simultaneously
    // by simply OR'ing the two values together (output and input are held
    // in upper and lower nibbles respectively).
    //

    //
    // I N P U T
    //

    //did we get an input error?
    if (v_pDMARegs->dcsr[gInputSrc]  & DCSR_BUSERRINTR)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT( "DCSR_BUSERRINTR ERROR on input\r\n" )) );

        DEBUGCHK(0); // a dma interrupt occured
        retval |= AUDIO_STATE_IGNORE;           //the descriptor was loaded but not ready for use

    }

    //did we get an input interrupt?
    if (v_pDMARegs->dcsr[gInputSrc]  & DCSR_ENDINTR)  //if the input channel stopped at the end of a buffer xfer
    {
        v_pDMARegs->dcsr[gInputSrc]  |= DCSR_ENDINTR;

        if (v_fMoreData[WAPI_IN] == TRUE)
            retval |= AUDIO_STATE_IN_RECORDING;
        else
            retval |= AUDIO_STATE_IN_STOPPED;           //TODO: Does the MDD now send a WPDM_STOP message?

    }

    if (v_pDMARegs->dcsr[gInputSrc]  & DCSR_STARTINTR)
    {
        v_pDMARegs->dcsr[gInputSrc]  |= DCSR_STARTINTR;  // reset the interrupt

        DEBUGMSG(ZONE_VERBOSE, (TEXT( "DCSR_STARTINTER on input\r\n" )) );

        retval |= AUDIO_STATE_IN_OVERFLOW;              //the descriptor was loaded but not ready for use

    }

    //did we get an output error?
    if (v_pDMARegs->dcsr[DMA_CH_OUT] & DCSR_BUSERRINTR)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT( "DCSR_BUSERRINTR ERROR on output\r\n" )) );

        DEBUGCHK(0); // a dma interrupt occured
        retval |= AUDIO_STATE_IGNORE;           //the descriptor was loaded but not ready for use

    }

    //did we get an output interrupt?
    if (v_pDMARegs->dcsr[DMA_CH_OUT] & DCSR_ENDINTR) //if the out channel stopped at the end of a buffer xfer
    {
        v_pDMARegs->dcsr[DMA_CH_OUT] |= DCSR_ENDINTR;

        if (v_fMoreData[WAPI_OUT] == TRUE)
            retval |= AUDIO_STATE_OUT_PLAYING;
        else
            retval |= AUDIO_STATE_OUT_STOPPED;          //TODO: Does the MDD now send a WPDM_STOP message?

    }

    if (v_pDMARegs->dcsr[DMA_CH_OUT] & DCSR_STARTINTR)
    {
        v_pDMARegs->dcsr[DMA_CH_OUT] |= DCSR_STARTINTR;  // reset the interrupt

        DEBUGMSG(ZONE_ERROR, (TEXT( "DCSR_STARTINTER on output\r\n" )) );

        retval |= AUDIO_STATE_OUT_UNDERFLOW;            //the descriptor was loaded but not ready for use

    }

    DEBUGMSG(ZONE_VERBOSE, (TEXT( "-PDD_AudioGetInterruptType\r\n" )) );


    return retval;
}


//------------------------------------------------------------------------------------------------------------
// Function: PddpAudioDeallocateVm
//
// Purpose: Unmap all allocated virtual memory locations
//
// Returns: none
//
//-------------------------------------------------------------------------------------------------------------
BOOL PddpAudioDeallocateVm(void)
{

    if (v_pDMARegs)
    {
        VirtualFree ((void*) v_pDMARegs,  0, MEM_RELEASE);
    }

    if (dma_page[0])
    {
        VirtualFree ((void*) dma_page[0],  0, MEM_RELEASE);
    }

    if (v_pAudioRcvA_Virtual)
    {
        VirtualFree ((void*) v_pAudioRcvA_Virtual,  0, MEM_RELEASE);
    }

    if (v_pAudioRcvB_Virtual)
    {
        VirtualFree ((void*) v_pAudioRcvB_Virtual,  0, MEM_RELEASE);
    }

    if (v_pAudioXmitA_Virtual)
    {
        VirtualFree ((void*) v_pAudioXmitA_Virtual,  0, MEM_RELEASE);
    }

    if (v_pAudioXmitB_Virtual)
    {
        VirtualFree ((void*) v_pAudioXmitB_Virtual,  0, MEM_RELEASE);
    }

    if (v_pAudioMicA_Virtual)
    {
        VirtualFree ((void*) v_pAudioMicA_Virtual,  0, MEM_RELEASE);
    }

    if (v_pAudioMicB_Virtual)
    {
        VirtualFree ((void*) v_pAudioMicB_Virtual,  0, MEM_RELEASE);
    }

    return(TRUE);
}


//------------------------------------------------------------------------------------------------------------
// Function: PDD_AudioInitialize
//
// Purpose: Initialize Audio & DMA Hardware
//
// Returns: TRUE indicates success. FALSE indicates failure
//
//-------------------------------------------------------------------------------------------------------------
BOOL PDD_AudioInitialize(DWORD dwIndex)
{

    int i;
    BOOL retValue = FALSE;
    DMA_ADAPTER_OBJECT Adapter;
    PHYSICAL_ADDRESS   PA;
    HKEY hConfig = NULL;
    LPWSTR lpRegPath = (LPWSTR)dwIndex;
    DWORD dwDataSize = 0;
    LONG regError;


    DEBUGMSG(ZONE_ERROR, (TEXT("+Audio Initialize\r\n")));

    g_fInPowerHandler = FALSE;

    g_pfnReadAc97     = ReadAC97;
    g_pfnWriteAc97    = WriteAC97;


    // Get audio driver parameters from the registry.
    //
    hConfig = OpenDeviceKey(lpRegPath);
    if(hConfig == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("ERROR: AudioInitialize: OpenDeviceKey('%s') failed.\r\n"), lpRegPath));
        return(FALSE);
    }

    // Read the SYSINTR value from the registry.
    //
    dwDataSize = BULVERDE_REG_SYSINTR_VAL_LEN;
    regError = RegQueryValueEx(hConfig, BULVERDE_REG_SYSINTR_VAL_NAME, NULL, NULL, (LPBYTE)&gIntrAudio, &dwDataSize);
    if (regError != ERROR_SUCCESS)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("ERROR: AudioInitialize: Query registry Sysintr value failed.\r\n")));
        RegCloseKey(hConfig);
        return(FALSE);
    }

    // Close the registry handle.
    //
    RegCloseKey(hConfig);


    //
    // Map the DMAC descriptors.
    //
    if (!MapDMADescriptors())
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("ERROR: AudioInitialize: failed to allocate DMA descriptor buffers.\r\n")));
        return(FALSE);
    }

    //
    // Map device registers.
    //
    if (!MapDeviceRegisters())
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("ERROR: AudioInitialize: failed to map device register(s).\r\n")));
        return(FALSE);
    }


    // Map both DMA pages into the local address space.
    //
    Adapter.ObjectSize    = sizeof(DMA_ADAPTER_OBJECT);
    Adapter.InterfaceType = Internal;
    Adapter.BusNumber     = 0;

    dma_page[0] = (BYTE *) HalAllocateCommonBuffer(&Adapter, (AUDIO_BUFFER_SIZE * NUM_DMA_AUDIO_BUFFERS), &PA, FALSE);
    if (!dma_page[0])
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("ERROR: AudioInitialize: failed to allocate DMA buffer(s).\r\n")));
        return(FALSE);
    }
    dma_page[1]     = dma_page[0] + AUDIO_BUFFER_SIZE;
    dma_page[2]     = dma_page[1] + AUDIO_BUFFER_SIZE;
    dma_page[3]     = dma_page[2] + AUDIO_BUFFER_SIZE;

    dma_page_physical[0] = (BYTE *)PA.LowPart;
    dma_page_physical[1] = dma_page_physical[0] + AUDIO_BUFFER_SIZE;
    dma_page_physical[2] = dma_page_physical[1] + AUDIO_BUFFER_SIZE;
    dma_page_physical[3] = dma_page_physical[2] + AUDIO_BUFFER_SIZE;

    DEBUGMSG(ZONE_ALLOC, (TEXT("INFO: AudioInitialize: dmap[0]=0x%x, dmap[1]=0x%x, dmap[2]=0x%x, dmap[3]=0x%x\r\n" ),
             dma_page[0],dma_page[1],dma_page[2],dma_page[3]));


    // Fill out the DMA descriptors.
    // NOTE: this doesn't actually load the descriptors.  We'll do that when asked to start DMA.
    //
    FillDescriptors();

    // Dump the Descriptorx.
    //
    DEBUGMSG(ZONE_VERBOSE, (TEXT( "\r\n")));
    DEBUGMSG(ZONE_VERBOSE, (TEXT( "v_pAudioXmitA->ddadr 0x%x \r\n" ),v_pAudioXmitA_Virtual->ddadr) );
    DEBUGMSG(ZONE_VERBOSE, (TEXT( "v_pAudioXmitA->dsadr 0x%x \r\n" ),v_pAudioXmitA_Virtual->dsadr) );
    DEBUGMSG(ZONE_VERBOSE, (TEXT( "v_pAudioXmitA->dtadr 0x%x \r\n" ),v_pAudioXmitA_Virtual->dtadr) );
    DEBUGMSG(ZONE_VERBOSE, (TEXT( "v_pAudioXmitA->dcmd  0x%x \r\n" ),v_pAudioXmitA_Virtual->dcmd ) );

    DEBUGMSG(ZONE_VERBOSE, (TEXT( "\r\n")));
    DEBUGMSG(ZONE_VERBOSE, (TEXT( "v_pAudioXmitB->ddadr 0x%x \r\n" ),v_pAudioXmitB_Virtual->ddadr) );
    DEBUGMSG(ZONE_VERBOSE, (TEXT( "v_pAudioXmitB->dsadr 0x%x \r\n" ),v_pAudioXmitB_Virtual->dsadr) );
    DEBUGMSG(ZONE_VERBOSE, (TEXT( "v_pAudioXmitB->dtadr 0x%x \r\n" ),v_pAudioXmitB_Virtual->dtadr) );
    DEBUGMSG(ZONE_VERBOSE, (TEXT( "v_pAudioXmitB->dcmd  0x%x \r\n" ),v_pAudioXmitB_Virtual->dcmd ) );

    DEBUGMSG(ZONE_VERBOSE, (TEXT( "\r\n")));
    DEBUGMSG(ZONE_VERBOSE, (TEXT( "v_pAudioRcvA->ddadr 0x%x \r\n" ),v_pAudioRcvA_Virtual->ddadr) );
    DEBUGMSG(ZONE_VERBOSE, (TEXT( "v_pAudioRcvA->dsadr 0x%x \r\n" ),v_pAudioRcvA_Virtual->dsadr) );
    DEBUGMSG(ZONE_VERBOSE, (TEXT( "v_pAudioRcvA->dtadr 0x%x \r\n" ),v_pAudioRcvA_Virtual->dtadr) );
    DEBUGMSG(ZONE_VERBOSE, (TEXT( "v_pAudioRcvA->dcmd  0x%x \r\n" ),v_pAudioRcvA_Virtual->dcmd ) );

    DEBUGMSG(ZONE_VERBOSE, (TEXT( "\r\n")));
    DEBUGMSG(ZONE_VERBOSE, (TEXT( "v_pAudioRcvB->ddadr 0x%x \r\n" ),v_pAudioRcvB_Virtual->ddadr) );
    DEBUGMSG(ZONE_VERBOSE, (TEXT( "v_pAudioRcvB->dsadr 0x%x \r\n" ),v_pAudioRcvB_Virtual->dsadr) );
    DEBUGMSG(ZONE_VERBOSE, (TEXT( "v_pAudioRcvB->dtadr 0x%x \r\n" ),v_pAudioRcvB_Virtual->dtadr) );
    DEBUGMSG(ZONE_VERBOSE, (TEXT( "v_pAudioRcvB->dcmd  0x%x \r\n" ),v_pAudioRcvB_Virtual->dcmd ) );


    // Compress range of 3f - 0 to 1f - 0 this makes volume adjustment seem intuitive.
    //
    for (i=0; i < 64; i+=2)
    {
        g_VolMatrix[i]   = (63 - i) >> 1;
        g_VolMatrix[i+1] = (63 - i) >> 1;
    }
    // Customize lower volume settings to quickly rolloff to neg infinity.
    g_VolMatrix[0] = 0x3f;
    g_VolMatrix[1] = 0x3A;
    g_VolMatrix[2] = 0x35;
    g_VolMatrix[3] = 0x2F;
    g_VolMatrix[4] = 0x2a;
    g_VolMatrix[5] = 0x25;
    g_VolMatrix[6] = 0x1f;


    // Initialize the AClink.
    //
    if (InitializeACLink(FALSE, DEV_AUDIO))
    {
        // Power-on the audio controller.
        if (AudioPowerOn())
        {
            retValue = TRUE;
        }
    }

    //msWait(500);

    DEBUGMSG(ZONE_ERROR, (TEXT("-Audio Initialize\r\n")));

    return (retValue);

}


//------------------------------------------------------------------------------------------------------------
// Function: PDD_AudioPowerHandler
//
// Purpose: This function is responsible for managing the audio hardware during POWER_UP
//           and POWER_DOWN notifications
//-------------------------------------------------------------------------------------------------------------
VOID
PDD_AudioPowerHandler(
    BOOL power_down
   )
{

    //
    // Handle any power up/down issues here.
    //
    g_fInPowerHandler = TRUE;

    //
    // Reset to initial state of SoundCSR inside ASIC, clearing all interrupts.
    // Speaker and CODEC are powered down.

    if (!power_down)
    {
        g_pfnReadAc97  = ReadAC97Raw;
        g_pfnWriteAc97 = WriteAC97Raw;
        AudioPowerOn();
        g_pfnReadAc97  = ReadAC97;
        g_pfnWriteAc97 = WriteAC97;
    }
    else
    {
        g_pfnReadAc97  = ReadAC97Raw;
        g_pfnWriteAc97 = WriteAC97Raw;
        AudioPowerOff();
    }

    g_fInPowerHandler = FALSE;

}

//------------------------------------------------------------------------------------------------------------
// Function: AudioPowerOn
//
// Purpose: Routine for powering on Audio hardware.  Do *all* register initializations (gpio, SSP, clocks, etc)
//          but, do not init DMA.
//
// Warning: This may be called from within power handler, so should not make any system calls.
//
// Note:
//-------------------------------------------------------------------------------------------------------------
BOOL
AudioPowerOn()
{

    //Basic Outline:
    // configue the GPIO registers
    // Set hardcoded values like variable rate audio
    // Set the BCR values (for sandgate)
    // Restore the state of the AC97 shadow registers from the WINCE registry
    //   if no registry values are found, then set the shadow registers to known defaults
    // set key register values to the values from the shadow registers
    //   set volume
    //   set record select
    //   set EQ values (bass, treble, and mode
    //   Clear Audio Mute (output & input)

    static int FirstTime=TRUE;

    unsigned short int Ac97RegisterData=0;
    unsigned short int FeatureCrs1Data=0;

    static DWORD VendorId=0;
    unsigned short int VidTemp=0;


    DEBUGMSG(ZONE_VERBOSE, (TEXT( "+Audio PowerOn\r\n" )) );

    if (!FirstTime)
    {
        ConfigureAC97Control();
    }

    //Force VRA (variable rate audio) to be on
    if( !ShadowWriteAC97( EXTENDED_AUDIO_CTRL, VRA_ENABLED_MASK , DEV_AUDIO)  )  // Enable Variable Rate Audio
        DEBUGMSG(ZONE_ERROR, (TEXT( "-- Bad VRA Value \r\n")   ) );

    if( !ShadowWriteAC97( RECORD_SELECT, 0x0, DEV_AUDIO )  )  // default is AUDIO_MIC_INPUT_MONO
        DEBUGMSG(ZONE_ERROR, (TEXT( "-- Bad RECORD_SELECT \r\n")   ) );

    //Set the output volume from the OS

    AC97_SetVolume(v_nVolume); // this will write to the shadow register

    //Set the record gain value
    if( !ShadowWriteAC97( RECORD_GAIN, 0x0f0f, DEV_AUDIO )  )  //set to a well working value
        DEBUGMSG(3, (TEXT( "-- Bad RECORD_GAIN \r\n")   ) );

    //Set the record gain value
    if( !ShadowWriteAC97( FEATURE_CSR1, 0x5400, DEV_AUDIO )  )  //set bass & treble to a good sounding value
        DEBUGMSG(3, (TEXT( "-- Bad FEATURE_CSR1 \r\n")   ) );

    //
    //compliant codecs such as Crystal require the PCM volume
    //if it's a ucb1400 then don't write the PCM volume (although it shouldn't hurt)
    //

    ShadowReadAC97(VENDOR_ID1, &VidTemp, DEV_AUDIO);  //ucb1400 doesn't use an ascii (that I can tell)
    VendorId= (VidTemp <<16); //ffffffffssssssss // f=first ascii s = second ascii
    ShadowReadAC97(VENDOR_ID2, &VidTemp, DEV_AUDIO);
    VendorId |=VidTemp;       //ttttttttrrrrrrrr //t = third ascii, r=rev #
    VendorId &= 0xfffffff0;//trim of version number


    //vendor specific
    switch (VendorId)
    {
        case 0x50534300: //philips UCB1400
            ShadowWriteAC97( FEATURE_CSR2, PWR_SMART_CODEC, DEV_AUDIO );  //enable smart mode

            g_pfnReadAc97(RESET, (unsigned short *) &ResetCaps, DEV_TOUCH); // if it returns 0x2a it's rev 1b, if 0x2a0 it 2a

            //
            //CAREFULL, writing the FEATURE_CSR1 could mess up the touch screen
            //

            ShadowReadAC97( FEATURE_CSR1,&Ac97RegisterData,DEV_AUDIO);  //get the audio shadow value

            Ac97RegisterData = Ac97RegisterData & EQ_MASK;              //get just EQ data

            ShadowReadAC97( FEATURE_CSR1,&FeatureCrs1Data,DEV_AUDIO);         //get the real codec register value
            //mask off eq data

            FeatureCrs1Data = FeatureCrs1Data &  0x8000;                // lob off reserved bit (must be 0)
            FeatureCrs1Data = FeatureCrs1Data & ~EQ_MASK;               // lob off eq data
            FeatureCrs1Data = FeatureCrs1Data | Ac97RegisterData;   // stored EQ data with actual Feature data

            //comment out headphone enable to save power, use an app to turn it on instead
            if (ResetCaps==REV_2A)
            {
                FeatureCrs1Data = FeatureCrs1Data | (unsigned short) HPEN_MASK; //turn on head phone
                CodecType=UCB14002A;
            }
            else
                CodecType=UCB14001B;

            if( !ShadowWriteAC97( FEATURE_CSR1, FeatureCrs1Data, DEV_AUDIO )  )  //write out the result
                DEBUGMSG(3, (TEXT( "-- Bad FEATURE_CRS1 \r\n")   ) );

            break;



        default:  //vanilla AC97
            ShadowWriteAC97( PCM_OUT_VOL, 0x1f1f, DEV_AUDIO );  //AC97 set the PCM out value (not used on ucb1400)
            break;


    }

    if (FirstTime)
        FirstTime=FALSE;

    AC97SetSampleRate(LastSampleRateOut,WAPI_OUT);
    AC97SetSampleRate(LastSampleRateIn,WAPI_IN);

    return (TRUE);

}

//------------------------------------------------------------------------------------------------------------
// Function: AudioPowerOff
//
// Purpose: Routine for powering down Audio hardware.
//-------------------------------------------------------------------------------------------------------------
VOID
AudioPowerOff()
{
    //This may be called from within powerhandler, so should not make any system calls.

    //Basic Outline
    //  Save the AC97 shadow register to the registry (not implemented)
    //  Mute record and playback to prevent problems.
    //  Set the BCR register values
    //  unconfigure the GPIO's

    private_WaveOutStop();

    AudioOutMute(TRUE);
    AudioInMute(TRUE);

    if (!UnConfigureAC97Control())
    {
        DEBUGCHK(0);
    }

}

//------------------------------------------------------------------------------------------------------------
// Function: PDD_AudioMessage
//
// Purpose: Handle any custom WAVEIN or WAVEOUT messages
//
// Returns: DWORD -- Error Return from the function
//
//-------------------------------------------------------------------------------------------------------------
DWORD
PDD_AudioMessage(
    UINT uMsg,
    DWORD dwParam1,
    DWORD dwParam2
    )
{

    unsigned short int MyData=0;

    switch (uMsg)
    {
/*
        Disabled - these pose a security risk.

        case WPDM_PRIVATE_WRITE_AC97:
            if (!ShadowWriteAC97( (BYTE) dwParam1,(unsigned short int)dwParam2 , DEV_AUDIO ))
                return(MMSYSERR_ERROR);

            DEBUGMSG(ZONE_ERROR, (TEXT( "write %x %x \r\n" ),dwParam1,dwParam2 ) );
            return (MMSYSERR_NOERROR);
            break;

        case WPDM_PRIVATE_READ_AC97:
            if (!ShadowReadAC97( (BYTE) dwParam1, &MyData , DEV_AUDIO ))
                return(MMSYSERR_ERROR);
            DEBUGMSG(ZONE_ERROR, (TEXT( "read %x %x \r\n" ),dwParam1,MyData ));
            if (dwParam2 != (unsigned short int) NULL)
                * (unsigned short int *) dwParam2 =  MyData;

            return (MMSYSERR_NOERROR);
            break;
*/

        case WPDM_PRIVATE_DIAG_MSG:
            switch (dwParam1)
            {
                case 0: // unused
                    return (MMSYSERR_NOERROR);

                case 1: // reset the codec
                    ColdResetAC97Control();
                    AudioPowerOff();
                    AudioPowerOn();
                    return (MMSYSERR_NOERROR);
                    break;

                default:
                    return (MMSYSERR_NOTSUPPORTED);

            }
            break;
    }
    return(MMSYSERR_NOTSUPPORTED);
}


//------------------------------------------------------------------------------------------------------------
// Function: PDD_AudioDeinitialize
//
// Purpose: This function turns off and disconnects the audio device
//-------------------------------------------------------------------------------------------------------------
VOID
PDD_AudioDeinitialize(
    VOID
    )
{

    // Disable DMA input.
    //
    StopDmac(gInputSrc);

    // Disable DMA output.
    //
    StopDmac(DMA_CH_OUT);

    // Disable/clear DMA IN interrupts.
    //
    v_pDMARegs->dcsr[gInputSrc] &= ~DCSR_STOPIRQEN;

    // Disable/clear DMA OUT interrupts.
    //
    v_pDMARegs->dcsr[DMA_CH_OUT] &= ~DCSR_STOPIRQEN;

    // Power-off the audio controller.
    //
    AudioPowerOff();

    // De-allocate the AClink.
    //
    DeInitializeACLink(g_fInPowerHandler, DEV_AUDIO);

    // Free memory and mapped registers.
    //
    PddpAudioDeallocateVm();

}

//------------------------------------------------------------------------------------------------------------
// Function: GetMuteState
//
// Purpose: Get the Mute State from the Codec Shadow Register
//
// Returns: True if MUTE, false if no mute
//
//-------------------------------------------------------------------------------------------------------------
BOOL
IsMuted(BYTE Offset)
{

    unsigned short int Ac97Data=0;

    ShadowReadAC97(Offset,&Ac97Data,DEV_AUDIO); //read the current hardware volume

    if (Ac97Data & MUTE_MASK)
        return (TRUE);
    else
        return (FALSE);

}


//------------------------------------------------------------------------------------------------------------
// Function: AudioOutMute
//
// Purpose: Quick Mute the hardware with out effecting the stored volume value
//
// Returns: TRUE indicates success. FALSE indicates failure
//
//-------------------------------------------------------------------------------------------------------------
BOOL
AudioOutMute(
   BOOL mute
   )
{
    BOOL retval = FALSE;

    unsigned short int Ac97Data=0;

    ShadowReadAC97(MASTER_VOLUME,&Ac97Data,DEV_AUDIO);  //read the current hardware volume

    if( mute )
    {
        Ac97Data=Ac97Data |   MUTE_MASK;    //set the mute bit
    }
    else
        Ac97Data=Ac97Data & ~ MUTE_MASK;    //clear the mute bit


    return( ShadowWriteAC97(MASTER_VOLUME, Ac97Data, DEV_AUDIO) );
}

//------------------------------------------------------------------------------------------------------------
// Function: AudioInMute
//
// Purpose: Quick Mute the hardware input with out effecting the gain
//
// Returns: TRUE indicates success. FALSE indicates failure
//
//-------------------------------------------------------------------------------------------------------------
BOOL
AudioInMute(
   BOOL mute
   )
{
    BOOL retval = FALSE;

    unsigned short int Ac97Data=0;

    ShadowReadAC97(RECORD_GAIN,&Ac97Data, DEV_AUDIO);           //read the current hardware volume

    if( mute )
    {
        Ac97Data=Ac97Data |   MUTE_MASK;    //set the mute bit
    }
    else
        Ac97Data=Ac97Data &  ~ MUTE_MASK;   //clear the mute bit


    return( ShadowWriteAC97(RECORD_GAIN, Ac97Data, DEV_AUDIO) );
}

//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
VOID
private_AudioComplete(PWAVEHDR pwh, ULONG cbBytesCompleted)
{
    // we've just finished playing another cbDstBytes of data.
    // update the queued headers accordingly
    while ((pwh != NULL) && (cbBytesCompleted > 0)) {

        if (pwh->dwBytesRecorded >= pwh->reserved) {
            pwh = pwh->lpNext;
        }
        else {
            ULONG cbBytesLeft = pwh->reserved - pwh->dwBytesRecorded;
            ULONG cbAdvance = min(cbBytesCompleted, cbBytesLeft);
            cbBytesCompleted -= cbAdvance;
            pwh->dwBytesRecorded += cbAdvance;
        }
    }

}

VOID
FillBufferM08 (PINT16 pDstBuffer, PBYTE pSrcBuffer, DWORD dwSamples)
{
    while (dwSamples-- > 0) {
        PPCM_SAMPLE pSample = (PPCM_SAMPLE) pSrcBuffer;
        INT16 sample16 = BITS_8_TO_16(pSample->m8.sample);
        pDstBuffer[0] = sample16;
        pDstBuffer[1] = sample16;
        pDstBuffer += 2;
        pSrcBuffer += 1;
    }
}

VOID
FillBufferS08 (PINT16 pDstBuffer, PBYTE pSrcBuffer, DWORD dwSamples)
{
    while (dwSamples-- > 0) {
        PPCM_SAMPLE pSample = (PPCM_SAMPLE) pSrcBuffer;
        pDstBuffer[0] = BITS_8_TO_16(pSample->s8.sample_left);
        pDstBuffer[1] = BITS_8_TO_16(pSample->s8.sample_right);
        pDstBuffer += 2;
        pSrcBuffer += 2;
    }
}

VOID
FillBufferM16 (PINT16 pDstBuffer, PBYTE pSrcBuffer, DWORD dwSamples)
{
    while (dwSamples-- > 0) {
        INT16 sample = * (PINT16) pSrcBuffer;
        pDstBuffer[0] = sample;
        pDstBuffer[1] = sample;
        pDstBuffer += 2;
        pSrcBuffer += 2;
    }
}

VOID
FillBufferS16 (PINT16 pDstBuffer, PBYTE pSrcBuffer, DWORD dwSamples)
{
    memcpy(pDstBuffer, pSrcBuffer, dwSamples * sizeof(DWORD));
}


//------------------------------------------------------------------------------------------------------------
// Function: private_AudioFillBuffer
//
// Purpose: Copies (plays) audio data to the DMAC buffers from Windows CE application buffers.  The data is
//          reformatted to meet the requirements of the codec.  This function is called in response to the
//          WPDM_START or WPDM_CONTINUE messages where the WAPI_INOUT directin is WAPI_OUT.
//-------------------------------------------------------------------------------------------------------------
VOID
private_AudioFillBuffer (
    PWAVEHDR pwh,
    PDWORD pDstBuffer,
    DWORD dwDstSamples
    )
{

    FUNC_VERBOSE("+PDD_AudioFillBuffer");

    while ((pwh != NULL) && (dwDstSamples > 0)) {
        if (pwh->reserved >= pwh->dwBufferLength) {
            pwh = pwh->lpNext;
        }
        else {
            DWORD dwSrcSamples = (pwh->dwBufferLength - pwh->reserved) / g_sample_size[WAPI_OUT];
            DWORD dwSamples = min(dwDstSamples, dwSrcSamples);

            __try
            {
                g_pfnFillBuffer((PINT16) pDstBuffer, pwh->lpData + pwh->reserved, dwSamples);
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                DEBUGMSG(ZONE_ERROR, (TEXT("wavepdd: Handled buffer fault in private_AudioFillBuffer\r\n")));
            }

            pwh->reserved += dwSamples * g_sample_size[WAPI_OUT];
            pDstBuffer += dwSamples;
            dwDstSamples -= dwSamples;
        }
    }

    //the input buffer is out of data, pad the DMA buffer with zeros
    memset(pDstBuffer, 0, dwDstSamples * sizeof(DWORD));

}

//------------------------------------------------------------------------------------------------------------
// Function: private_WaveOutEndOfData
//
// Purpose: Stops output of audio data.
//-------------------------------------------------------------------------------------------------------------
VOID
private_WaveOutEndOfData()
{
    FUNC_WPDD("+PDD_WaveOutEndOfData");

    //
    // There is no more data coming...
    //
    v_fMoreData[WAPI_OUT] = FALSE;

    StopDmac(DMA_CH_OUT);

    FUNC_WPDD("-PDD_WaveOutEndOfData");
}

//------------------------------------------------------------------------------------------------------------
// Function: private_WaveOutStart
//
// Purpose: Starts (play or) processing of audio data and initiates DMAC output to the codec in response to
//          the WPDM_START message where the WAPI_INOUT directin is WAPI_OUT.
//-------------------------------------------------------------------------------------------------------------
VOID
private_WaveOutStart (
    PWAVEHDR pwh
    )
{
    FUNC_WPDD("+PDD_WaveOutStart");


    //clear the mute
    AudioOutMute(FALSE);

    //
    //fill up both pages but indicate we expect more data
    //

    v_fMoreData[WAPI_OUT] = TRUE;        // we expect to have more data coming...

    private_AudioFillBuffer(pwh, (PDWORD) dma_page[0], g_dma_buffer_size / sizeof(DWORD));
    private_AudioFillBuffer(pwh, (PDWORD) dma_page[1], g_dma_buffer_size / sizeof(DWORD));

    InitDMAC(DMA_CH_OUT);  //Load the Descriptors, Start a DMA

    FUNC_WPDD("-PDD_WaveOutStart");

}

//------------------------------------------------------------------------------------------------------------
// Function: private_WaveOutContinue
//
// Purpose: Continues (play or) processing of audio data and DMA output to the codec in response to
//          the WPDM_CONTINUE message where the WAPI_INOUT directin is WAPI_OUT.
//-------------------------------------------------------------------------------------------------------------
VOID
private_WaveOutContinue (
    PWAVEHDR pwh
    )
{
    ULONG next_page;

    FUNC_VERBOSE("+PDD_WaveOutContinue");

    // figure out which page the DMA engine will go to next & copy there
    next_page = (v_pDMARegs->ddg[DMA_CH_OUT].ddadr == (UINT32)v_pAudioXmitA_Physical) ? 0 : 1;

    private_AudioComplete(pwh, g_dma_buffer_size);

    private_AudioFillBuffer (pwh, (PDWORD) dma_page[next_page], g_dma_buffer_size / sizeof(DWORD));


    FUNC_VERBOSE("-PDD_WaveOutContinue");
}

//------------------------------------------------------------------------------------------------------------
// Function: private_WaveOutRestart
//
// Purpose: continues playback of paused data in response to WPDM_RESET
//-------------------------------------------------------------------------------------------------------------
MMRESULT
private_WaveOutRestart (
    PWAVEHDR pwh
    )
{
    FUNC_VERBOSE("+PDD_WaveOutRestart");

    private_WaveOutStart (pwh);

    FUNC_VERBOSE("-PDD_WaveOutRestart");
    return(MMSYSERR_NOERROR);
}



//------------------------------------------------------------------------------------------------------------
// Function: private_WaveOutPause
//
// Purpose: Pause playback of audio data in response to WPDM_PAUSE
//-------------------------------------------------------------------------------------------------------------
MMRESULT
private_WaveOutPause (
    VOID
    )
{
    FUNC_VERBOSE("+PDD_WaveOutPause");

    private_WaveOutEndOfData();

    FUNC_VERBOSE("-PDD_WaveOutPause");
    return(MMSYSERR_NOERROR);
}


//------------------------------------------------------------------------------------------------------------
// Function: private_WaveOutStop
//
// Purpose: Stop the playback of audio data in response to WPDM_STOP
//-------------------------------------------------------------------------------------------------------------
VOID
private_WaveOutStop()
{
    FUNC_WPDD("+private_WaveOutStop");

    v_fMoreData[WAPI_OUT] = FALSE;

    StopDmac(DMA_CH_OUT);

    AudioOutMute(TRUE);

    FUNC_WPDD("-private_WaveOutStop");
}
//------------------------------------------------------------------------------------------------------------
//  FUNCTION: private_AudioGetBuffer
//
//  PURPOSE: grab incoming data from DMA buffers and write it to application buffers
//------------------------------------------------------------------------------------------------------------

VOID
GetBufferM08 (PBYTE pDstBuffer, PINT16 pSrcBuffer, DWORD dwSamples)
{
    while (dwSamples-- > 0) {
        *pDstBuffer = (BYTE) BITS_16_TO_8(pSrcBuffer[0]);
        pSrcBuffer += 2;
        pDstBuffer += 1;
    }
}

VOID
GetBufferS08 (PBYTE pDstBuffer, PINT16 pSrcBuffer, DWORD dwSamples)
{
    while (dwSamples-- > 0) {
        pDstBuffer[0] = (BYTE) BITS_16_TO_8(pSrcBuffer[0]);
        pDstBuffer[1] = (BYTE) BITS_16_TO_8(pSrcBuffer[1]);
        pSrcBuffer += 2;
        pDstBuffer += 2;
    }
}


VOID
GetBufferM16 (PBYTE pDst, PINT16 pSrcBuffer, DWORD dwSamples)
{
    PINT16 pDstBuffer = (PINT16) pDst;
    while (dwSamples-- > 0) {
        pDstBuffer[0] = pSrcBuffer[0];
        pSrcBuffer += 2;
        pDstBuffer += 1;
    }
}


VOID
GetBufferS16 (PBYTE pDstBuffer, PINT16 pSrcBuffer, DWORD dwSamples)
{
    memcpy(pDstBuffer, pSrcBuffer, dwSamples * sizeof(DWORD));
}


VOID
private_AudioGetBuffer (
    PWAVEHDR pwh,
    PDWORD pSrcBuffer,
    DWORD dwSrcSamples
    )
{
    FUNC_VERBOSE("+PDD_AudioGetBuffer");

    while ((pwh != NULL) && (dwSrcSamples > 0)) {
        if (pwh->dwBytesRecorded >= pwh->dwBufferLength) {
            pwh = pwh->lpNext;
        }
        else {
            DWORD dwDstSamples = (pwh->dwBufferLength - pwh->dwBytesRecorded) / g_sample_size[WAPI_IN];
            DWORD dwSamples = min(dwSrcSamples, dwDstSamples);

            __try
            {
                g_pfnGetBuffer(pwh->lpData + pwh->dwBytesRecorded, (PINT16) pSrcBuffer, dwSamples);
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                DEBUGMSG(ZONE_ERROR, (TEXT("wavepdd: Handled buffer fault in private_AudioGetBuffer\r\n")));
            }

            pwh->dwBytesRecorded += dwSamples * g_sample_size[WAPI_IN];
            pSrcBuffer += dwSamples;
            dwSrcSamples -= dwSamples;
        }
    }

    FUNC_VERBOSE("+PDD_AudioGetBuffer");
}

//------------------------------------------------------------------------------------------------------------
// Function: private_WaveInStart
//
// Purpose: Starts (record or) processing of audio data and initiates DMAC input from the codec in response to
//          the WPDM_START message where the WAPI_INOUT directin is WAPI_IN.
//-------------------------------------------------------------------------------------------------------------
VOID
private_WaveInStart (
    PWAVEHDR pwh
    )
{
    FUNC_WPDD("+PDD_WaveInStart");

    //clear the mute
    AudioInMute(FALSE);

    // enable microphone Boost, but keep the line from mic to speakers muted.
    ShadowWriteAC97( MIC_VOLUME, 0x8040, DEV_AUDIO);

    //
    //fill up both pages but indicate we expect more data
    //

    v_fMoreData[WAPI_IN]  = TRUE;        // we expect to have more data coming...

    InitDMAC(gInputSrc);   //Load the Descriptors, Start a DMA

    FUNC_WPDD("+PDD_WaveInStart");

}

//------------------------------------------------------------------------------------------------------------
// Function: private_WaveInContinue
//
// Purpose:  instruct the audio PDD to continue with the data pointed to by the wave header
//-------------------------------------------------------------------------------------------------------------
VOID
private_WaveInContinue(
   PWAVEHDR pwh
   )
{
    ULONG next_page;
    FUNC_VERBOSE("+PDD_WaveInContinue");

    // figure out which page the DMA engine will go to next & copy from there
    next_page = (v_pDMARegs->ddg[gInputSrc].ddadr == (UINT32) v_pAudioRcvA_Physical) ? 2 : 3;

    //fill up the input buffer (dma_page[]) pointed to by next_page
    private_AudioGetBuffer  (pwh, (PDWORD) dma_page[next_page], g_capture_buffer_size / sizeof(DWORD));

    FUNC_VERBOSE("-PDD_WaveInContinue");
}

//------------------------------------------------------------------------------------------------------------
// Function: private_WaveInStop
//
// Purpose: stop recording immediately
//-------------------------------------------------------------------------------------------------------------
VOID
private_WaveInStop(
    PWAVEHDR pwh
    )
{
    FUNC_WPDD("+PDD_WaveInStop");

    v_fMoreData[WAPI_IN] = FALSE;

    StopDmac(gInputSrc);

    AudioInMute(TRUE);

    FUNC_WPDD("-PDD_WaveInStop");

}

//------------------------------------------------------------------------------------------------------------
// Function: PowerDownUnit
//
// Purpose: power up or power down a dac, adc, or other devices
//-------------------------------------------------------------------------------------------------------------
void PowerDownUnit(unsigned short int Unit,BOOL ShutDown)
{

    //TODO Do we need to wait some time before powering on or off the unit?
    unsigned short int PowerDownCtrlStat=0;

    if( !ShadowReadAC97( POWERDOWN_CTRL_STAT, &PowerDownCtrlStat, DEV_AUDIO)  )  // Enable Variable Rate Audio
            DEBUGMSG(ZONE_ERROR, (TEXT( "-- read of PwrDnCtrl failed \r\n")   ) );

    if (ShutDown)
        PowerDownCtrlStat |= Unit;
    else
        PowerDownCtrlStat &= ~Unit;

    if( !ShadowWriteAC97( POWERDOWN_CTRL_STAT, PowerDownCtrlStat , DEV_AUDIO)  )  // Enable Variable Rate Audio
            DEBUGMSG(ZONE_ERROR, (TEXT( "-- Bad VRA Value \r\n")   ) );

}



//------------------------------------------------------------------------------------------------------------
// Function: private_WaveStandby
//
// Purpose: instruct the audio PDD to put the audio circuitry in standby mode.
//    According to the documentation this function "This message should turn off as much of the audio circuit
//    as possible. When this message is called again, the audio circuitry should be turned on as quickly as
//    possibly [sic]. This message is sent between playback of sounds to minimize audio circuit power drain.
//-------------------------------------------------------------------------------------------------------------
VOID
private_WaveStandby(
    WAPI_INOUT apidir
    )
{
    static BOOL PowerDownIn=TRUE;
    static BOOL PowerDownOut=TRUE;


    //TODO: this function doesn't seem to get called under PB 3.0
    switch(apidir)
    {
        case WAPI_IN:
            DEBUGMSG(ZONE_TEST,(TEXT("wavestandby PowerDownIn: %x WAPI_INOUT: %x\r\n"),PowerDownIn,apidir));
            PowerDownUnit(PWR_PR0_ADC,PowerDownIn);  //turn
            PowerDownIn=!PowerDownIn;
            break;

        case WAPI_OUT:
            DEBUGMSG(ZONE_TEST,(TEXT("wavestandby PowerDownOut: %x WAPI_INOUT: %x\r\n"),PowerDownOut,apidir));
            PowerDownUnit(PWR_PR1_DAC,PowerDownOut);  //turn
            PowerDownOut=!PowerDownOut;
            break;

        default:
            DEBUGMSG(ZONE_TEST,(TEXT("bad param in wavestandby\r\n")));
            DEBUGCHK(FALSE);
    }

}

//------------------------------------------------------------------------------------------------------------
// Function: private_WaveGetDevCaps
//
// Purpose: return the capabilities of the audio hardware
//-------------------------------------------------------------------------------------------------------------
MMRESULT
private_WaveGetDevCaps(
    WAPI_INOUT apidir,
    PVOID pCaps,
    UINT  wSize
    )
{


    PWAVEINCAPS pwic = pCaps;
    PWAVEOUTCAPS pwoc = pCaps;

    MMRESULT mmRet = MMSYSERR_NOERROR;

    FUNC_WPDD("+PDD_WaveGetDevCaps");

   if (pCaps == NULL)  {
        //
        // If pCaps == NULL, we are requesting if the driver PDD is capable of
        // this mode at all. In other words, if APIDIR == WAPI_IN,  we return
        // no error if input is supported, and MMSYSERR_NOTSUPPORTED otherwise.
        // Since UCB1400 has both, we return MMSYSERR_NOERROR regardless.
        //
        if( WAPI_IN == apidir)
            return( MMSYSERR_NOERROR );
        else // ( WAPI_OUT == apidir)
            return( MMSYSERR_NOERROR );
    }
    //
    // Fill in the DevCaps here.
    //
    if (apidir == WAPI_OUT)
    {
        DEBUGMSG(ZONE_VERBOSE, (TEXT("API_OUT\r\n"  )) );

        pwoc->wMid = MM_MICROSOFT;  //TODO: this says microsoft wrote this driver, change to intel
        pwoc->wPid = (apidir == WAPI_OUT ? 24 : 23);  // generic in or out...
        pwoc->vDriverVersion = 0x0001;
        wsprintf (pwoc->szPname, TEXT("AC '97  Stereo (%hs)"), __DATE__);
        pwoc->dwFormats =   WAVE_FORMAT_1M08 |
                            WAVE_FORMAT_1M16 |
                            WAVE_FORMAT_1S08 |
                            WAVE_FORMAT_1S16 |
                            WAVE_FORMAT_2M08 |
                            WAVE_FORMAT_2M16 |
                            WAVE_FORMAT_2S08 |
                            WAVE_FORMAT_2S16 |
                            WAVE_FORMAT_4M08 |
                            WAVE_FORMAT_4M16 |
                            WAVE_FORMAT_4S08 |
                            WAVE_FORMAT_4S16;


        pwoc->wChannels = 2;

        //pwoc->dwSupport = WAVECAPS_VOLUME | WAVECAPS_SYNC  ;

    }
    else
    {
        DEBUGMSG(ZONE_VERBOSE, (TEXT("API_IN\r\n"  )) );
        pwic->wMid = MM_MICROSOFT;  //TODO: this says microsoft wrote this driver, change to intel
        pwic->wPid = (apidir == WAPI_OUT ? 24 : 23);  // generic in or out...
        pwic->vDriverVersion = 0x0001;
        wsprintf (pwoc->szPname, TEXT("AC '97  Stereo (%hs)"), __DATE__);
        pwic->dwFormats =   WAVE_FORMAT_1M08 |
                            WAVE_FORMAT_1M16 |
                            WAVE_FORMAT_1S08 |
                            WAVE_FORMAT_1S16 |
                            WAVE_FORMAT_2M08 |
                            WAVE_FORMAT_2M16 |
                            WAVE_FORMAT_2S08 |
                            WAVE_FORMAT_2S16 |
                            WAVE_FORMAT_4M08 |
                            WAVE_FORMAT_4M16 |
                            WAVE_FORMAT_4S08 |
                            WAVE_FORMAT_4S16;



        pwic->wChannels = 2;

        //pwic->dwSupport = WAVECAPS_VOLUME | WAVECAPS_SYNC  ;

     }

    FUNC_WPDD("-PDD_WaveGetDevCaps");;

    return(mmRet);
}

//------------------------------------------------------------------------------------------------------------
// Function: private_WaveOpen
//
// Purpose:  prepare to either send or receive audio data (based on WAPI_INOUT direction flag)
//-------------------------------------------------------------------------------------------------------------
MMRESULT
private_WaveOpen(
    WAPI_INOUT apidir,
    LPWAVEFORMATEX lpFormat,
    BOOL fQueryFormatOnly
    )
{

    MMRESULT mmRet = MMSYSERR_NOERROR;

    FUNC_VERBOSE("+PDD_WaveOpen");

    switch (CodecType)
    {
        case UCB14002A:
            if ((lpFormat->wFormatTag         != WAVE_FORMAT_PCM)  ||
               (  lpFormat->nChannels         != 1 &&
                  lpFormat->nChannels         != 2)             ||
               (  lpFormat->nSamplesPerSec    != KHZ08_000 &&
                  lpFormat->nSamplesPerSec    != KHZ11_025 &&
                  lpFormat->nSamplesPerSec    != KHZ12_000 &&
                  lpFormat->nSamplesPerSec    != KHZ22_050 &&
                  lpFormat->nSamplesPerSec    != KHZ24_000 &&
                  lpFormat->nSamplesPerSec    != KHZ32_000 &&
                  lpFormat->nSamplesPerSec    != KHZ44_100 &&
                  lpFormat->nSamplesPerSec    != KHZ48_000
                  )         ||
               (  lpFormat->wBitsPerSample    != 16 &&
                  lpFormat->wBitsPerSample    != 8))
            {
                DEBUGMSG(ZONE_ERROR, (TEXT( "return bad format\r\n" )) );
                return WAVERR_BADFORMAT;
            }
            break;

        case UCB14001B:
        default:
            if ((lpFormat->wFormatTag         != WAVE_FORMAT_PCM)  ||
               (  lpFormat->nChannels         != 1 &&
                  lpFormat->nChannels         != 2)             ||
               (  lpFormat->nSamplesPerSec    != KHZ08_000 &&
                  lpFormat->nSamplesPerSec    != KHZ11_025 &&
                  lpFormat->nSamplesPerSec    != KHZ16_000 &&
                  lpFormat->nSamplesPerSec    != KHZ22_050 &&
                  lpFormat->nSamplesPerSec    != KHZ32_000 &&
                  lpFormat->nSamplesPerSec    != KHZ44_100 &&
                  lpFormat->nSamplesPerSec    != KHZ48_000
                  )         ||
               (  lpFormat->wBitsPerSample    != 16 &&
                  lpFormat->wBitsPerSample    != 8))
            {
                DEBUGMSG(ZONE_ERROR, (TEXT( "return bad format\r\n " )) );
                return WAVERR_BADFORMAT;
            }
    } // end switch

    //if they only want a query format, then we can return success.
    if (fQueryFormatOnly) {
        DEBUGMSG( ZONE_VERBOSE, (TEXT( "Format Query\r\n " )) );
        return MMSYSERR_NOERROR;
    }

    if (g_fInUse[apidir]) {
        DEBUGMSG(ZONE_ERROR, (TEXT("\r\n")));
        return MMSYSERR_ALLOCATED;
    }
    if (apidir < NUM_API_DIRS) g_fInUse[apidir] = TRUE;

    //ENABLE VRA
    if( !ShadowWriteAC97( EXTENDED_AUDIO_CTRL, VRA_ENABLED_MASK , DEV_AUDIO)  )  // Enable Variable Rate Audio
        DEBUGMSG(ZONE_ERROR, (TEXT( "-- Bad VRA Value \r\n")   ) );

    //set the sample rate
    if (AC97SetSampleRate((unsigned short int)lpFormat->nSamplesPerSec,apidir) == FALSE)
    {
    DEBUGMSG( 1, (TEXT( "return bad sample rate:\r\n " )) );
    return MMSYSERR_NOTSUPPORTED;
    }



    if (apidir < NUM_API_DIRS) g_sample_size[apidir] = lpFormat->nBlockAlign;

    if (apidir == WAPI_OUT) {
        if (lpFormat->wBitsPerSample == 8) {
            if (lpFormat->nChannels == 1) {
                g_pfnFillBuffer = FillBufferM08;
            }
            else {
                g_pfnFillBuffer = FillBufferS08;
            }
        }
        else {
            if (lpFormat->nChannels == 1) {
                g_pfnFillBuffer = FillBufferM16;
            }
            else {
                g_pfnFillBuffer = FillBufferS16;
            }
        }
    }
    else {
        if (lpFormat->wBitsPerSample == 8) {
            if (lpFormat->nChannels == 1) {
                g_pfnGetBuffer = GetBufferM08;
            }
            else {
                g_pfnGetBuffer = GetBufferS08;
            }
        }
        else {
            if (lpFormat->nChannels == 1) {
                g_pfnGetBuffer = GetBufferM16;
            }
            else {
                g_pfnGetBuffer = GetBufferS16;
            }
        }
        // Support Low-latency Capture
        // Adjust the effective DMA buffer size, based on the requested sample rate.
        // The idea is that we want our DMA capture buffers to represent a fixed amount of time,
        // regardless of the sample rate.
        // This means that if the application buffers are small (as is the case with VoIP or DirectSoundCapture)
        // then we still can return the buffer very soon after we have enough captured data to fill it.
        // The way this used to work (fixed-size capture buffers) meant that at low sample rates
        // we would wait for hundreds of milliseconds, then turn around and discover that the application
        // had only given us 80 milliseconds of wave headers, so we would be forced to drop samples on the floor.
        g_capture_buffer_size = sizeof(DWORD) * g_capture_latency * lpFormat->nSamplesPerSec / 1000;
        if (g_capture_buffer_size > g_dma_buffer_size) {
            g_capture_buffer_size = g_dma_buffer_size;
        }
        v_pAudioRcvA_Virtual->dcmd = (v_pAudioRcvA_Virtual->dcmd & ~0x1fff) | g_capture_buffer_size;
        v_pAudioRcvB_Virtual->dcmd = (v_pAudioRcvB_Virtual->dcmd & ~0x1fff) | g_capture_buffer_size;

    }

    ResetAC97Controller(); // this flushes the fifos and freezes AC-Link - workaround for B-Stepping problem.

    FUNC_VERBOSE("-PDD_WaveOpen");

    return MMSYSERR_NOERROR;
}


//------------------------------------------------------------------------------------------------------------
// Function: PDD_WaveProc
//
// Purpose: process WPDM_xxx messages from the MDD.  This is the main message handler for the PDD.
//-------------------------------------------------------------------------------------------------------------
MMRESULT
PDD_WaveProc(
    WAPI_INOUT apidir,
    DWORD      dwCode,
    DWORD      dwParam1,
    DWORD      dwParam2
    )
{
    MMRESULT mmRet = MMSYSERR_NOERROR;
    DEBUGMSG( ZONE_VERBOSE, (TEXT( "PDD_WaveProc: " )) );

    switch (dwCode) {
        case WPDM_CLOSE:
            if (apidir == WAPI_IN && g_fInUse[apidir]) {
                // if app never send WODM_RESET, the capture DMA is still running. Stop it now.
                private_WaveInStop(NULL);
            }
            if (apidir < NUM_API_DIRS) g_fInUse[apidir] = FALSE;
            DEBUGMSG( ZONE_VERBOSE, (TEXT( "....close %s\r\n" ),(apidir == WAPI_IN)?"i n ":"o u t ") );
            break;

        case WPDM_CONTINUE:
            DEBUGMSG( ZONE_VERBOSE, (TEXT( "....continue %s\r\n" ),(apidir == WAPI_IN)?"i n ":"o u t ") );
            if (apidir == WAPI_IN)
                private_WaveInContinue((PWAVEHDR) dwParam1);
            else
                private_WaveOutContinue((PWAVEHDR) dwParam1);
            break;

        case WPDM_GETDEVCAPS:
        DEBUGMSG( ZONE_VERBOSE, (TEXT( "....getdevcaps\r\n" )) );
            mmRet = private_WaveGetDevCaps(apidir, (PVOID) dwParam1, (UINT) dwParam2);
            break;

        case WPDM_OPEN:
        DEBUGMSG( ZONE_VERBOSE, (TEXT( "....open %s\r\n" ),(apidir == WAPI_IN)?"i n ":"o u t ") );

            mmRet = private_WaveOpen(apidir, (LPWAVEFORMATEX) dwParam1, (BOOL) dwParam2);
            break;

        case WPDM_STANDBY:
        DEBUGMSG( ZONE_VERBOSE, (TEXT( "....standby\r\n" )) );
            private_WaveStandby (apidir);
            break;

        case WPDM_START:
        DEBUGMSG( ZONE_VERBOSE, (TEXT( "....start %s\r\n" ),(apidir == WAPI_IN)?"i n ":"o u t ") );
            if (apidir == WAPI_IN)
                private_WaveInStart((PWAVEHDR) dwParam1);
            else
                private_WaveOutStart((PWAVEHDR) dwParam1);
            break;

        case WPDM_STOP:
        DEBUGMSG( ZONE_VERBOSE, (TEXT( "....stop  %s\r\n" ),(apidir == WAPI_IN)?"i n ":"o u t ") );
            if (apidir == WAPI_IN)
                private_WaveInStop((PWAVEHDR) dwParam1);
            else
                private_WaveOutStop();
            break;

        case WPDM_PAUSE:
        DEBUGMSG( ZONE_VERBOSE, (TEXT( "....pause\r\n" )) );
            if (apidir == WAPI_OUT)
                private_WaveOutPause();
            else
                mmRet = MMSYSERR_NOTSUPPORTED;
            break;

        case WPDM_ENDOFDATA:
        DEBUGMSG( ZONE_VERBOSE, (TEXT( "....endofdata\r\n" )) );
            if (apidir == WAPI_OUT)
                private_WaveOutEndOfData();
            else
                mmRet = MMSYSERR_NOTSUPPORTED;
            break;

        case WPDM_RESTART:
        DEBUGMSG( ZONE_VERBOSE, (TEXT( "....restart\r\n" )) );
            if (apidir == WAPI_OUT)
                private_WaveOutRestart((PWAVEHDR) dwParam1);
            else
                mmRet = MMSYSERR_NOTSUPPORTED;
            break;

        case WPDM_GETVOLUME: //TODO:  use L3 to get volume
        DEBUGMSG( ZONE_VERBOSE, (TEXT( "....getvolume\r\n" )) );
            *((PULONG) dwParam1) = v_nVolume;
            break;

        case WPDM_SETVOLUME: //TODO:  use L3 to set volume
        DEBUGMSG( ZONE_VERBOSE, (TEXT( "....setvolume %d\r\n" ),(ULONG) dwParam1 ) );
            v_nVolume = (ULONG) dwParam1;
            AC97_SetVolume(v_nVolume);
            break;

        default :
        DEBUGMSG( ZONE_VERBOSE, (TEXT( "....unsupported\r\n" )) );
            mmRet = MMSYSERR_NOTSUPPORTED;
            break;
    }

    return(mmRet);
}

//------------------------------------------------------------------------------------------------------------
// Function: UnMapVirtual
//
// Purpose:  Free variables from virtual space
//-------------------------------------------------------------------------------------------------------------
BOOL UnMapVirtual(void * v_pMapSpace)
{
    BOOL ret=FALSE;

    if (v_pMapSpace != NULL )
        return (VirtualFree ((void*) v_pMapSpace,  0, MEM_RELEASE));
    else
        return (TRUE);
}

//------------------------------------------------------------------------------------------------------------
// Function: AC97_SetVolume
//
// Purpose: Convert the Windows Volume to an AC 97 volume
//-------------------------------------------------------------------------------------------------------------

BOOL AC97_SetVolume(ULONG uVolume)
{
    unsigned short int ucAC97VolL;
    unsigned short int ucAC97VolR;
    unsigned short int ucAC97VolBoth;
    unsigned short int ucAC97VolCur;

    // Windows Volume is 16 bits of Left and 16 bits of right (0xRRRRLLLL)

    ucAC97VolR=(unsigned short) (uVolume & 0x0000FFFF) >> 10; // six bits of volume
    ucAC97VolR=g_VolMatrix[ucAC97VolR];

    ucAC97VolL=(unsigned short) ((uVolume & 0xFFFF0000)>>16) >> 10;  //six bits of volume
    ucAC97VolL=g_VolMatrix[ucAC97VolL];

    ucAC97VolBoth=(ucAC97VolL<<8) | ucAC97VolR;

    ShadowReadAC97( MASTER_VOLUME, &ucAC97VolCur, DEV_AUDIO );

    ucAC97VolCur &= 0xC0C0;  //only keep the upper 2 bits of each byte
    ucAC97VolBoth &= ~0xC0C0;  //only keep lower 6 bits of each byte
    ucAC97VolBoth |= ucAC97VolCur;  //add current volume bites


    //NKDbgPrintfW(TEXT("left:%x  right:%x, both:%x ucAC97VolBoth\r\n"),ucAC97VolL,ucAC97VolR);

    return (ShadowWriteAC97(MASTER_VOLUME, ucAC97VolBoth, DEV_AUDIO));
    //return TRUE;

}


//------------------------------------------------------------------------------------------------------------
// Function: MapDeviceRegisters
//
// Purpose:  Map the Bulverde device registers required for the audio driver.
//
// Returns:  TRUE indicates success. FALSE indicates failure
//
//-------------------------------------------------------------------------------------------------------------
static BOOL MapDeviceRegisters(void)
{
    PHYSICAL_ADDRESS PA;

    PA.QuadPart  = BULVERDE_BASE_REG_PA_DMAC;
    v_pDMARegs   = (BULVERDE_DMA_REG *) MmMapIoSpace(PA, sizeof(BULVERDE_DMA_REG), FALSE);


    PA.QuadPart  = BULVERDE_BASE_REG_PA_AC97;
    v_pAC97Regs  = (BULVERDE_AC97_REG *) MmMapIoSpace(PA, sizeof(BULVERDE_AC97_REG), FALSE);

    if (!v_pDMARegs || !v_pAC97Regs)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("ERROR: MapDeviceRegisters: failed to map device register(s).\r\n")));
        return(FALSE);
    }

    return (TRUE);

}


//------------------------------------------------------------------------------------------------------------
// Function: MapDMADescriptors
//
// Purpose:  Map the physical DMA Descriptors into virtual space
//
// Returns:  TRUE indicates success. FALSE indicates failure
//
//-------------------------------------------------------------------------------------------------------------
static BOOL MapDMADescriptors(void)
{
    DMA_ADAPTER_OBJECT Adapter;
    PHYSICAL_ADDRESS   PA;

    Adapter.ObjectSize    = sizeof(DMA_ADAPTER_OBJECT);
    Adapter.InterfaceType = Internal;
    Adapter.BusNumber     = 0;

    v_pAudioRcvA_Virtual = (DMADescriptorChannelType *) HalAllocateCommonBuffer(&Adapter, 0x20, &PA, FALSE);
    if (v_pAudioRcvA_Virtual)
    {
        v_pAudioRcvA_Physical = (DMADescriptorChannelType *) PA.LowPart;
    }

    v_pAudioRcvB_Virtual = (DMADescriptorChannelType *) HalAllocateCommonBuffer(&Adapter, 0x20, &PA, FALSE);
    if (v_pAudioRcvB_Virtual)
    {
        v_pAudioRcvB_Physical = (DMADescriptorChannelType *) PA.LowPart;
    }

    v_pAudioXmitA_Virtual = (DMADescriptorChannelType *) HalAllocateCommonBuffer(&Adapter, 0x20, &PA, FALSE);
    if (v_pAudioXmitA_Virtual)
    {
        v_pAudioXmitA_Physical = (DMADescriptorChannelType *) PA.LowPart;
    }

    v_pAudioXmitB_Virtual = (DMADescriptorChannelType *) HalAllocateCommonBuffer(&Adapter, 0x20, &PA, FALSE);
    if (v_pAudioXmitB_Virtual)
    {
        v_pAudioXmitB_Physical = (DMADescriptorChannelType *) PA.LowPart;
    }

    v_pAudioMicA_Virtual = (DMADescriptorChannelType *) HalAllocateCommonBuffer(&Adapter, 0x20, &PA, FALSE);
    if (v_pAudioMicA_Virtual)
    {
        v_pAudioMicA_Physical = (DMADescriptorChannelType *) PA.LowPart;
    }

    v_pAudioMicB_Virtual = (DMADescriptorChannelType *) HalAllocateCommonBuffer(&Adapter, 0x20, &PA, FALSE);
    if (v_pAudioMicB_Virtual)
    {
        v_pAudioMicB_Physical = (DMADescriptorChannelType *) PA.LowPart;
    }

    if (!v_pAudioRcvA_Virtual  || !v_pAudioRcvB_Virtual  ||
        !v_pAudioXmitA_Virtual || !v_pAudioXmitB_Virtual ||
        !v_pAudioMicA_Virtual  || !v_pAudioMicB_Virtual)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("ERROR: MapDMADescriptors: failed to allocate DMA descriptor buffer(s).\r\n")));
        return(FALSE);
    }

    return (TRUE);

}



#ifdef DEBUG
//------------------------------------------------------------------------------------------------------------
// Function: ClearDMA
//
// Purpose: Make sure all channels are DEAD //this assumes no others use DMA
//
// Note: This code could be cleaner, but I want to be explicit on the programming of descriptors
//
//-------------------------------------------------------------------------------------------------------------
void ClearDmac( void )
{
    DEBUGMSG(ZONE_ERROR, (TEXT( "\r\n\r\n ------------------------- \r\n Clearing all DMA channels \r\n ------------------------- \r\n\r\n" )) );

    //make sure no DMA activity will occure
    v_pDMARegs->dcsr[0]=0x40000007;   //           0100 0000 0000 0000 0000 0000 0000 0111
    v_pDMARegs->dcsr[1]=0x40000007;   //           0100 0000 0000 0000 0000 0000 0000 0111
    v_pDMARegs->dcsr[2]=0x40000007;   //           0100 0000 0000 0000 0000 0000 0000 0111
    v_pDMARegs->dcsr[3]=0x40000007;   //           0100 0000 0000 0000 0000 0000 0000 0111
    v_pDMARegs->dcsr[4]=0x40000007;   //           0100 0000 0000 0000 0000 0000 0000 0111
    v_pDMARegs->dcsr[5]=0x40000007;   //           0100 0000 0000 0000 0000 0000 0000 0111
    v_pDMARegs->dcsr[6]=0x40000007;   //           0100 0000 0000 0000 0000 0000 0000 0111
    v_pDMARegs->dcsr[7]=0x40000007;   //           0100 0000 0000 0000 0000 0000 0000 0111
    v_pDMARegs->dcsr[8]=0x40000007;   //           0100 0000 0000 0000 0000 0000 0000 0111
    v_pDMARegs->dcsr[9]=0x40000007;   //           0100 0000 0000 0000 0000 0000 0000 0111
    v_pDMARegs->dcsr[10]=0x40000007;   //           0100 0000 0000 0000 0000 0000 0000 0111
    v_pDMARegs->dcsr[12]=0x40000007;   //           0100 0000 0000 0000 0000 0000 0000 0111
    v_pDMARegs->dcsr[13]=0x40000007;   //           0100 0000 0000 0000 0000 0000 0000 0111
    v_pDMARegs->dcsr[14]=0x40000007;   //           0100 0000 0000 0000 0000 0000 0000 0111
    v_pDMARegs->dcsr[15]=0x40000007;   //           0100 0000 0000 0000 0000 0000 0000 0111

}
#endif

//------------------------------------------------------------------------------------------------------------
// Function: FillDescriptors
//
// Purpose: Fill the descriptors with appropriate data
//
// Note: This code could be cleaner, but I want to be explicit on the programming of descriptors
//
//-------------------------------------------------------------------------------------------------------------
static void FillDescriptors(void)
{
    union DMACmdReg CmdBuff;


#define USE_BITFIELD
#ifdef  USE_BITFIELD

    // set values with bit fields.
    //
    CmdBuff.DcmdReg.len        = AUDIO_BUFFER_SIZE;  // length of the memory buffer
    CmdBuff.DcmdReg.width      = 0x3;    // binary 11 (see quick Quick Reference sheet to DMA programming in the cotulla EAS)
    CmdBuff.DcmdReg.size       = 0x3;    // binary 11
    CmdBuff.DcmdReg.endian     = 0;      // little endian
    CmdBuff.DcmdReg.flybyt     = 0;      // Flowthrough
    CmdBuff.DcmdReg.flybys     = 0;      // Flowthrough
    CmdBuff.DcmdReg.endirqen   = 1;      // 1 means Interrupt when decrement length = 0;
    CmdBuff.DcmdReg.startirqen = 0;      // 1 means Interrupt when the desc is loaded
    CmdBuff.DcmdReg.flowtrg    = 0;      // 1 means the target is an external peripheral
    CmdBuff.DcmdReg.flowsrc    = 1;      // 1 means the source is an external peripheral (and needs flow control)
    CmdBuff.DcmdReg.inctrgadd  = 1;      // 1 means increment the target address (since it's memory)
    CmdBuff.DcmdReg.incsrcadd  = 0;      // 1 means increment the source address (since it's a peripheral)
#else
    // set value based on masks.
    //
    CmdBuff.DcmdDword = DMAC_AC97_RCVAB_CMD_MASK | AUDIO_BUFFER_SIZE;
#endif

    // fill RcvA Descriptor.
    //
    v_pAudioRcvA_Virtual->ddadr = (UINT32) v_pAudioRcvB_Physical;        // address of the next (RcvB) descriptor
    v_pAudioRcvA_Virtual->dtadr = (UINT32) dma_page_physical[2];         // source address of the AC97 RcvA buffer
    v_pAudioRcvA_Virtual->dsadr = (BULVERDE_BASE_REG_PA_AC97 + 0x40);    // destination address of the RcvA buffer
    v_pAudioRcvA_Virtual->dcmd  = CmdBuff.DcmdDword;                     // size and cmd values of the RcvA buffer

    // fill RcvB Descriptor.
    //
    v_pAudioRcvB_Virtual->ddadr = (UINT32) v_pAudioRcvA_Physical;        // address of the next (RcvA) descriptor
    v_pAudioRcvB_Virtual->dtadr = (UINT32) dma_page_physical[3];         // source address of the AC97 RcvB buffer
    v_pAudioRcvB_Virtual->dsadr = (BULVERDE_BASE_REG_PA_AC97 + 0x40);    // destination address of the RcvB buffer
    v_pAudioRcvB_Virtual->dcmd  = CmdBuff.DcmdDword;                     // size and cmd values of the RcvB buffer


#define USE_BITFIELD
#ifdef  USE_BITFIELD

    // set values with bit fields.
    //
    CmdBuff.DcmdReg.len        = AUDIO_BUFFER_SIZE;  // length of memory buffer
    CmdBuff.DcmdReg.width      = 0x3;    // binary 11 (see quick Quick Reference sheet to DMA programming in the cotulla EAS)
    CmdBuff.DcmdReg.size       = 0x3;    // binary 11
    CmdBuff.DcmdReg.endian     = 0;      // little endian
    CmdBuff.DcmdReg.flybyt     = 0;      // Flowthrough
    CmdBuff.DcmdReg.flybys     = 0;      // Flowthrough
    CmdBuff.DcmdReg.endirqen   = 1;      // 1 means Interrupt when decrement length = 0;
    CmdBuff.DcmdReg.startirqen = 0;      // 1 means Interrupt when the desc is loaded
    CmdBuff.DcmdReg.flowtrg    = 1;      // 1 means the target is an external peripheral
    CmdBuff.DcmdReg.flowsrc    = 0;      // 1 means the source is an external peripheral (and needs flow con
    CmdBuff.DcmdReg.inctrgadd  = 0;      // 1 means increment the target address (since it's memory)
    CmdBuff.DcmdReg.incsrcadd  = 1;      // 1 means increment the source address (since it's a peripheral)
#else
    // set value based on masks.
    //
    CmdBuff.DcmdDword = DMAC_AC97_XMITAB_CMD_MASK | AUDIO_BUFFER_SIZE;
#endif

    // fill XmitA Descriptor.
    //
    v_pAudioXmitA_Virtual->ddadr = (UINT32) v_pAudioXmitB_Physical;       // address of the next (XmitB) descriptor
    v_pAudioXmitA_Virtual->dtadr = (BULVERDE_BASE_REG_PA_AC97 + 0x40);    // source address of the AC97 XmitA buffer
    v_pAudioXmitA_Virtual->dsadr = (UINT32) dma_page_physical[0];         // destination address of the XmitA buffer
    v_pAudioXmitA_Virtual->dcmd  = CmdBuff.DcmdDword;                     // size and cmd values of the XmitA buffer

    // fill XmitB Descriptor.
    //
    v_pAudioXmitB_Virtual->ddadr = (UINT32) v_pAudioXmitA_Physical;       // address of the next (XmitA) descriptor
    v_pAudioXmitB_Virtual->dtadr = (BULVERDE_BASE_REG_PA_AC97 + 0x40);    // source address of the AC97 XmitB buffer
    v_pAudioXmitB_Virtual->dsadr = (UINT32) dma_page_physical[1];         // destination address of the XmitB buffer
    v_pAudioXmitB_Virtual->dcmd  = CmdBuff.DcmdDword;                     // size and cmd values of the XmitB buffer


#define USE_BITFIELD
#ifdef  USE_BITFIELD

    // set values with bit fields.
    //
    CmdBuff.DcmdReg.len        = AUDIO_BUFFER_SIZE;  // length of memory buffer
    CmdBuff.DcmdReg.width      = 0x3;    // binary 11 (see quick Quick Reference sheet to DMA programming in the cotulla EAS)
    CmdBuff.DcmdReg.size       = 0x3;    // binary 11
    CmdBuff.DcmdReg.endian     = 0;      // little endian
    CmdBuff.DcmdReg.flybyt     = 0;      // Flowthrough
    CmdBuff.DcmdReg.flybys     = 0;      // Flowthrough
    CmdBuff.DcmdReg.endirqen   = 1;      // 1 means Interrupt when decrement length = 0;
    CmdBuff.DcmdReg.startirqen = 0;      // 1 means Interrupt when the desc is loaded
    CmdBuff.DcmdReg.flowtrg    = 0;      // 1 means the target is an external peripheral
    CmdBuff.DcmdReg.flowsrc    = 1;      // 1 means the source is an external peripheral (and needs flow control)
    CmdBuff.DcmdReg.inctrgadd  = 1;      // 1 means increment the target address (since it's memory)
    CmdBuff.DcmdReg.incsrcadd  = 0;      // 1 means increment the source address (since it's a peripheral)
#else
    // set value based on masks.
    //
    CmdBuff.DcmdDword = DMAC_AC97_MICAB_CMD_MASK | AUDIO_BUFFER_SIZE;
#endif

    // fill MicA Descriptor.
    //
    v_pAudioMicA_Virtual->ddadr = (UINT32) v_pAudioMicB_Physical & DESC_ADDRESS_MASK;             // address of the next (MicA) descriptor
    v_pAudioMicA_Virtual->dtadr = (UINT32) dma_page_physical[2] & FORCE_64BIT_ALIGNMENT;          // source address of the AC97 Mica buffer
    v_pAudioMicA_Virtual->dsadr = (BULVERDE_BASE_REG_PA_AC97 + 0x40) & FORCE_128BIT_ALIGNMENT;    // destination address of the Mica buffer
    v_pAudioMicA_Virtual->dcmd  = CmdBuff.DcmdDword;                                              // size and cmd values of the Mica buffer

    // fill MicB Descriptor.
    //
    v_pAudioMicB_Virtual->ddadr = (UINT32) v_pAudioMicA_Physical & DESC_ADDRESS_MASK;             // address of the next (MicA) descriptor
    v_pAudioMicB_Virtual->dtadr = (UINT32) dma_page_physical[3] & FORCE_64BIT_ALIGNMENT;          // source address of the AC97 Micb buffer
    v_pAudioMicB_Virtual->dsadr = (BULVERDE_BASE_REG_PA_AC97 + 0x40) & FORCE_128BIT_ALIGNMENT;    // destination address of the Micb buffer
    v_pAudioMicB_Virtual->dcmd  = CmdBuff.DcmdDword;                                              // size and cmd values of the Micb buffer

}


//------------------------------------------------------------------------------------------------------------
// Function: AC97SetSampleRate
//
// Purpose: Write the sample rate value to the ac97 codec
//-------------------------------------------------------------------------------------------------------------
short int AC97SetSampleRate(unsigned short int SampleRate, WAPI_INOUT apidir )
{
    short int RetVal=FALSE;

    switch(SampleRate)
    {
        case 8000:
        case 11025:
        case 16000:
        case 22050:
        case 32000:
        case 44100:
        case 48000:
            RetVal=TRUE;
            break;
        default:
            DEBUGCHK(0);  //we sent a bad rate
            RetVal=FALSE;
            return (RetVal);


    }

    //this write assumes that the
    if (apidir == WAPI_IN)
    {
        RetVal=ShadowWriteAC97((BYTE)AUDIO_ADC_RATE,(unsigned short int)(SampleRate & 0xFFFF) , DEV_AUDIO); //set the input sample rate
        LastSampleRateIn=SampleRate;
    }
    else
    {
        RetVal=ShadowWriteAC97((BYTE)AUDIO_DAC_RATE,(unsigned short int)(SampleRate & 0xFFFF) , DEV_AUDIO); //set the output sample rate
        LastSampleRateOut=SampleRate;
    }

    return (RetVal);
}

//------------------------------------------------------------------------------------------------------------
// Function: ShadowWriteAC97
//
// Purpose: Perform AC97 write and copy the write to a shadow register.  This helps performance by reducing the
//          number of reads over the "slow" ac97 link
//
// Returns: SUCCESS if there was a successful write to the AC97 codec
//
//-------------------------------------------------------------------------------------------------------------
short int ShadowWriteAC97(BYTE Offset, unsigned short int Data, BYTE DevId)
{
    //there are some values which should not be shadowed since the codec is volatile
    //TODO: return failure or warning on attempts to write values that shouldn't be shadowed

    short int RetVal = TRUE;
    int i;

    RetVal = g_pfnWriteAc97(Offset, Data, DevId);

    if (RetVal != TRUE)
    {
        for (i=1; i<3; i++)
        {
            RetVal=g_pfnWriteAc97(Offset, Data, DevId);
            if (RetVal == TRUE)
                i=3;
        }
    }

    return(RetVal);
}


//------------------------------------------------------------------------------------------------------------
// Function: ShadowReadAC97
//
// Purpose: Return a value from a shadowed ac97 codec register.  No actual read of the AC97 codec occurs.
//
// Returns: SUCCESS
//
//-------------------------------------------------------------------------------------------------------------
short int ShadowReadAC97(BYTE Offset, unsigned short int *Data, BYTE DevId)
{

    BOOL RetVal=FALSE;
    //there are some values which should not be shadowed since the codec is volatile
    //TODO: return failure on attempts to read values that shouldn't be shadowed
    //*Data=(unsigned short int) Ac97ShadowRegisters[Offset];
    //*Data=ReadAc97(Offset);

    int i;
    RetVal = g_pfnReadAc97(Offset,Data, DevId);
    if (RetVal != TRUE)
    {
        for (i=1; i<3; i++)
        {
            RetVal=g_pfnReadAc97(Offset,Data, DevId);
            if (RetVal == TRUE)
                i=3;
        }
    }

    return (RetVal);
}


//------------------------------------------------------------------------------------------------------------
// Function: InitDMAC
//
// Purpose: Populate DMAC Descriptors and fill DMA registers with descriptor pointers, Start DMA transmit
//
// Returns:
//
//-------------------------------------------------------------------------------------------------------------
BOOL InitDMAC(int Channel)
{

    // 1. Set the physical address of the frame descriptors

    // 2. setup the request channel map register (with channel and valid)


    // 3. Load the address of the first descriptor into the channel group. The rest of
    //    the registers are loaded when the RUN bit is set in the DCSR.

    DEBUGMSG(ZONE_VERBOSE, (TEXT( "InitDmac %d \r\n" ),Channel) );

    switch(Channel)
    {
        case DMA_CH_OUT:
            //DEBUGMSG(ZONE_ERROR, (TEXT( "Starting DMA XMIT channel %d \r\n" ),Channel) );
            v_pDMARegs->ddg[DMA_CH_OUT].ddadr = (UINT32) v_pAudioXmitA_Physical;
            v_pDMARegs->drcmr[DMA_CHMAP_AC97_OUT] = DMA_MAP_VALID_MASK | DMA_CH_OUT ;
            v_pAC97Regs->posr =  0x10;                          //clear the output Fifo error
            v_pDMARegs->dcsr[DMA_CH_OUT] |=  DCSR_RUN;          // set the RUN bit
            break;

        case DMA_CH_MIC:
            //DEBUGMSG(ZONE_ERROR, (TEXT( "Starting DMA channel %d \r\n" ),Channel) );
            v_pDMARegs->ddg[Channel].ddadr = (UINT32) v_pAudioMicA_Physical;
            v_pDMARegs->drcmr[DMA_CHMAP_AC97_MIC]  = DMA_MAP_VALID_MASK | DMA_CH_MIC;
            v_pAC97Regs->mcsr =  0x10;                          //clear Pcm Input Fifo status
            v_pDMARegs->dcsr[DMA_CH_MIC] |=  DCSR_RUN;          // set the RUN bit
            break;

        case DMA_CH_RCV:
            //DEBUGMSG(ZONE_ERROR, (TEXT( "Starting DMA Receive channel %d \r\n" ),Channel) );
            v_pDMARegs->ddg[DMA_CH_RCV].ddadr = (UINT32) v_pAudioRcvA_Physical;
            v_pDMARegs->drcmr[DMA_CHMAP_AC97_RCV] = DMA_MAP_VALID_MASK | DMA_CH_RCV ;
            v_pAC97Regs->pisr =  0x10;                           //clear Pcm Input Fifo status
            v_pDMARegs->dcsr[DMA_CH_RCV] |=  DCSR_RUN;          // set the RUN bit
            break;

        default:
            DebugBreak();
    }

    // set the RUN bit for the channel.

    return TRUE;
}


//------------------------------------------------------------------------------------------------------------
// Function: StopDmac
//
// Purpose: To stop DMA transmit
//-------------------------------------------------------------------------------------------------------------
BOOL  StopDmac(int Channel)
{
    //TODO: Do I need to set the dma length to 0

    v_pDMARegs->dcsr[Channel] &= ~DCSR_RUN;  //clear the run but

    return(TRUE);
}


// -----------------------------------------------------------------------------
//  Debugging aids...
// -----------------------------------------------------------------------------
void PrintBuffer(INT32 UNALIGNED * pbuf)
{
    UINT i;
    const ULONG dma_num_samples = g_dma_buffer_size/4;

    PRINTMSG(1, (TEXT("\r\n-------------Print Buffer @0x%08X\r\n"), pbuf));

    for (i=0; i< dma_num_samples; i++) {
        if (i%8 == 0)
            PRINTMSG(1, (TEXT("\r\n")));

        PRINTMSG(1, (TEXT(" %08x"), pbuf[i]));
    }

    PRINTMSG(1, (TEXT("\r\n\r\n")));
}

