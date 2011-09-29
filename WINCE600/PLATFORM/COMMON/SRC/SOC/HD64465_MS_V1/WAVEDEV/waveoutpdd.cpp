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
/*

  Copyright(c) 1998,1999 Renesas Technology Corp.

    Module Name:

        waveoutpdd.cpp

    Revision History:

        26th April      1999        Released
        30th June       1999        Modified audio out buffer address
                                    Added AudioOutInterrupt Flag
                                    suppress playback recording
        6th  July       1999        Fixed minor bug
        21th September  1999        Modified audio volume
                                    Added audio pause and Audio restart process
                                    Fixed L-R turn bug
                                    Added PCML(R) FIFO buffer clear in WaveOutStop & WaveOutEndOfData & WaveOutPause process
        8th  October    1999        Modified WaveData length extend
        29-Nov-1999 cea     Changed for SH3 (7709A) DMA control
*/

#include <windows.h>
#include <types.h>
#include <memory.h>
#include <excpt.h>
#include <waveddsi.h>
#include <wavedbg.h>
#include <mmsystem.h>
#include <ceddk.h>
#include "wavepdd.h"
#include "waveOutpdd.h"

#include "cc.h"
#include "macros.h"
#include "wavecpud.h"
DmaObject * pDmaOutObject=NULL;
extern DWORD dwCCCodecRegBase;//CC_CODEC_REGBASE
extern DWORD dwCCSysSMCR;//CC_SYS_SMSCR
static PVUSHORT pCODEC_CR;                      // virtual addrs of C.C. registers
static PVULONG  pCODEC_ACR, pCODEC_CAR, pCODEC_CDR;
static PVULONG  pCODEC_ATAGR;
static PVULONG  pCODEC_PCML;
static PVULONG  pSMSCR;
static PVUSHORT pCODEC_FSR;

extern "C" PUCHAR GetVirtualAddressOfUncachedMemory( PVOID base, ULONG sz, char* cID );
extern BOOL g_fInUse;
extern PWAVEFORMATEX g_pwfx[2];
extern PCM_TYPE g_pcmtype[2];
VOID (*private_AudioFillBuffer) ( PWAVEHDR pwh, PBYTE pDstBuffer, DWORD cbDstBytes );

// Software Volume Control

#define scaleForVolume(sample) ((sample * multiplier) >> log2ofDivisor)

const struct {              // table facilitates multiplying by fraction:
   UINT16 multiplier;       //     multiply by "multiplier"
   UINT16 divisor;          //     then shift right by "log2ofDivisor"
} VOL[] = { {0, 0},         //  0/1
            {3, 5},         //  3/32
            {7, 6},         //  7/64
            {1, 3},         //  1/8
            {5, 5},         //  5/32
            {3, 4},         //  3/16
            {7, 5},         //  7/32
            {1, 2},         //  1/4
            {5, 4},         //   etc.
            {3, 3},
            {7, 4},
            {1, 1},
            {5, 3},
            {3, 2},
            {7, 3},
            {1, 0}
          };

// Double Buffer for Audio Playback

static ULONG v_nNextPage;

static  PBYTE   pCC_CODEC_RegBase;  // Companion chip  SERIAL CODEC register offsets
static  PBYTE   pCC_SYS_RegBase;    // Companion chip  SMSCR
static  PBYTE   pAudioBufferBase;   // Audio buffer

//PDRIVER_GLOBALS   pDriverGlobals;     // Drivers Globals

static ULONG v_nVolume;             // current volume setting
static volatile BOOL v_fMoreData[2];// TRUE iff MDD layer has called StopPlay
                                    //   v_fMoreData was volatile on D9000
                                    //   It could be involved in controlling
                                    //   multiple threads.


/*****************************************************************************
*   FUNCTION :      private_AudioIsOut
*   DESCRIPTION : returns true if dma is set for audio out
*   INPUTS :        	None
*   OUTPUTS :       	bool value
*   DESIGN NOTES :
*   CAUTIONS :
*****************************************************************************/
BOOL 
private_AudioIsOut()
{
    if (pDmaOutObject->GetDmaSourceReg() == (dwCCCodecRegBase + CC_CODEC_PCML_OFFSET))
        return false;

    return true;
}
									
/*****************************************************************************
*   FUNCTION :      private_AudioOutGetInterruptType
*   DESCRIPTION :   decodes type of audio interrupt
*   INPUTS :        None
*   OUTPUTS :       interrupt type
*   DESIGN NOTES :
*   CAUTIONS :
*****************************************************************************/
AUDIO_STATE
private_AudioOutGetInterruptType(
   VOID
   )
{
    FUNC_WPDD("+PDD_AudioOutGetInterruptType");

    // An audio interrupt has occured. We need to tell the MDD who owns it
    // and what state that puts us in.

    if (pDmaOutObject == NULL || !private_AudioIsOut())
    {
        DEBUGMSG(ZONE_TEST, (TEXT("PDD_AudioOutGetInterruptType: ignore")));
        FUNC_WPDD("-PDD_AudioOutGetInterruptType");
        return AUDIO_STATE_IGNORE;  // assume audio-in generated interrupt
    }

    //Stop the codec. Will be restarted later if needed
    WRITE_REGISTER_ULONG((PULONG)pCODEC_ACR, (ULONG)0x0000000 );   // play stop

    if (!v_fMoreData[WAPI_OUT]) 
    {
        DEBUGMSG(ZONE_TEST, (TEXT("PDD_AudioOutGetInterruptType: stopped")));
        FUNC_WPDD("-PDD_AudioOutGetInterruptType");
        return AUDIO_STATE_OUT_STOPPED;
    }
    else if (pDmaOutObject->GetGlobalPlayAddress()!=NULL)//(pDriverGlobals->aud.play_address != (ULONG)NULL)
    {
        DEBUGMSG(ZONE_TEST, (TEXT("PDD_AudioOutGetInterruptType: playing")));
        FUNC_WPDD("-PDD_AudioOutGetInterruptType");
        return AUDIO_STATE_OUT_PLAYING;
    }
    else
    {
        DEBUGMSG(ZONE_TEST, (TEXT("PDD_AudioOutGetInterruptType:underflow")));
        FUNC_WPDD("-PDD_AudioOutGetInterruptType");
        return AUDIO_STATE_OUT_UNDERFLOW;
    }
}



/*****************************************************************************
*   FUNCTION :      private_AudioOutInitialize
*   DESCRIPTION :   sets up register access for audio-out
*                   sets DA, DMA, and CMT registers that remain constant
*                   sets volume to maximum (default)
*   INPUTS :        None
*   OUTPUTS :       None
*   DESIGN NOTES :
*   CAUTIONS :
*****************************************************************************/
BOOL
private_AudioOutInitialize(BOOL bPowerOn)
{
    ULONG   ACR_VALUE;
    USHORT  CR_VALUE;
    volatile DWORD dwIndex;

    CalibrateStallCounter();
	
    //ULONG SizeOfBuffer;
    char here[] = "PDD_AudioOutInitialize";

    FUNC_WPDD("+PDD_AudioOutInitialize");
    if (!bPowerOn)
        RETAILMSG( 1, (TEXT("WaveOutpdd.c private_AudioOutInitialize: START\r\n")));

    // Set pointers to virtual addresses of playback registers
/*  
    pDMAC_RegBase =(PBYTE)GetVirtualAddressOfUncachedMemory(
        (PBYTE)DMAC_REGBASE,
        (DWORD)DMAC_REGSIZE,
        "PDD_AudioInitialize, pDMAC_RegBase");

    if (pDMAC_RegBase == NULL) return FALSE;

    pDMAOR   = (PVUSHORT)(pDMAC_RegBase + DMAC_DMAOR_OFFSET );
    pSAR1    = (PVULONG)(pDMAC_RegBase + DMAC_SAR1_OFFSET   );
    pDAR1    = (PVULONG)(pDMAC_RegBase + DMAC_DAR1_OFFSET   );
    pDMATCR1 = (PVULONG)(pDMAC_RegBase + DMAC_DMATCR1_OFFSET);
    pCHCR1   = (PVULONG)(pDMAC_RegBase + DMAC_CHCR1_OFFSET  );

    pCHCR0   = (PVULONG)(pDMAC_RegBase + DMAC_CHCR0_OFFSET  );
    pCHCR2   = (PVULONG)(pDMAC_RegBase + DMAC_CHCR2_OFFSET  );
    pCHCR3   = (PVULONG)(pDMAC_RegBase + DMAC_CHCR3_OFFSET  );

    pINTC_Area_1_RegBase =(PBYTE)GetVirtualAddressOfUncachedMemory(
        (PBYTE)INTC_AREA_1_REGBASE,
        (DWORD)INTC_AREA_1_REGSIZE,
        "PDD_AudioInitialize, pINTC_Area_1_RegBase");

    if (pINTC_Area_1_RegBase == NULL) return FALSE;

    pINTC_Area_7_RegBase =(PBYTE)GetVirtualAddressOfUncachedMemory(
        (PBYTE)INTC_AREA_7_REGBASE,
        (DWORD)INTC_AREA_7_REGSIZE,
        "PDD_AudioInitialize, pINTC_Area_7_RegBase");

    if (pINTC_Area_7_RegBase == NULL) return FALSE;
*/
    if (!bPowerOn)
    pCC_CODEC_RegBase =(PBYTE)GetVirtualAddressOfUncachedMemory(
        (PBYTE)dwCCCodecRegBase,//CC_CODEC_REGBASE,
        (DWORD)CC_CODEC_REGSIZE,
        "PDD_AudioInitialize, pCC_CODEC_RegBase");

    if (pCC_CODEC_RegBase == NULL) return FALSE;
    pCODEC_CR    = (PVUSHORT) (pCC_CODEC_RegBase + CC_CODEC_CR_OFFSET     );
    pCODEC_CAR   = (PVULONG ) (pCC_CODEC_RegBase + CC_CODEC_CAR_OFFSET    );
    pCODEC_CDR   = (PVULONG ) (pCC_CODEC_RegBase + CC_CODEC_CDR_OFFSET    );
    pCODEC_ACR   = (PVULONG ) (pCC_CODEC_RegBase + CC_CODEC_ACR_OFFSET    );
    pCODEC_ATAGR = (PVULONG ) (pCC_CODEC_RegBase + CC_CODEC_ATAGR_OFFSET  );
    pCODEC_PCML  = (PVULONG ) (pCC_CODEC_RegBase + CC_CODEC_PCML_OFFSET   );
    pCODEC_FSR   = (PVUSHORT) (pCC_CODEC_RegBase + CC_CODEC_FSR_OFFSET    );

    if (!bPowerOn)
    pCC_SYS_RegBase =(PBYTE)GetVirtualAddressOfUncachedMemory(
        (PBYTE)dwCCSysSMCR,//CC_SYS_SMSCR,
        (DWORD)CC_SYS_REGSIZE,
        "PDD_AudioInitialize, pCC_SYS_RegBase");

    if (pCC_SYS_RegBase == NULL) return FALSE;
    pSMSCR       = (PVULONG ) (pCC_SYS_RegBase );

    // Set pointers to virtual addresses of audio buffers

/*  SizeOfBuffer = (AUDIO_DMA_PAGE_SIZE * 2);

    pAudioBufferBase = VirtualAlloc(
        NULL,
        SizeOfBuffer,
        MEM_RESERVE,
        PAGE_NOACCESS);

    if (pAudioBufferBase == NULL) return FALSE;

    if (!VirtualCopy((PVOID)pAudioBufferBase,
        (PVOID)dma_pagePhysicalAddress[0],
        (DWORD)SizeOfBuffer,
        PAGE_READWRITE | PAGE_NOCACHE))
    {
        VirtualFree((PVOID)pAudioBufferBase, 0, MEM_RELEASE);
        return FALSE;
    }
*/
    if (pDmaOutObject==NULL) {
        pDmaOutObject= CreateDmaObject(FALSE); // false for play dma buffer
        ASSERT(pDmaOutObject);
    };
    pAudioBufferBase=(PBYTE)pDmaOutObject->GetDmaVirtualAddr();
    if (pAudioBufferBase == NULL)
    {
        if (!bPowerOn)
            RETAILMSG( 1, (TEXT("pAudioBufferBase ---error---\r\n")));
        return FALSE;
    }

    // Setup a Access to AudioPlayingAddress and AudioOutInterrupt
/*
    pDriverGlobals = VirtualAlloc(
        NULL,
        DRIVER_GLOBALS_PHYSICAL_MEMORY_SIZE,
        MEM_RESERVE,
        PAGE_NOACCESS);

    if (pDriverGlobals == NULL) return FALSE;

    if (!VirtualCopy((PVOID)pDriverGlobals,
        (PVOID)DRIVER_GLOBALS_PHYSICAL_MEMORY_START,
        (DWORD)DRIVER_GLOBALS_PHYSICAL_MEMORY_SIZE,
        PAGE_READWRITE | PAGE_NOCACHE))
    {
        VirtualFree((PVOID)pDriverGlobals, 0, MEM_RELEASE);
        return FALSE;
    }
*/
    //
    //  AC97 Setting
    //

    //Reset AC97
    WRITE_REGISTER_USHORT((PUSHORT)pCODEC_CR, 0x0a00 );
    Delay_AC();
    WRITE_REGISTER_USHORT((PUSHORT)pCODEC_FSR, 0x0000 );
    Delay_AC();
    WRITE_REGISTER_USHORT((PUSHORT)pCODEC_CR, 0x0235 );
    Delay_AC();

    //Init_Codec
    ACR_VALUE = (ULONG)READ_REGISTER_ULONG((PULONG)pCODEC_ACR );
    ACR_VALUE &= 0x7fffffff;
    ACR_VALUE |= 0x001fffff;
    WRITE_REGISTER_USHORT((PUSHORT)pCODEC_FSR, 0x0000 );
    Delay_AC();
    WRITE_REGISTER_ULONG((PULONG)pCODEC_ACR, ACR_VALUE );
    Delay_AC();

    //AC_Init
    CR_VALUE = (USHORT)READ_REGISTER_USHORT((PUSHORT)pCODEC_CR );
    CR_VALUE &= 0xf7ff;
    CR_VALUE |= 0x0035;
    WRITE_REGISTER_USHORT((PUSHORT)pCODEC_FSR, 0x0000 );
    Delay_AC();
    WRITE_REGISTER_USHORT((PUSHORT)pCODEC_CR, CR_VALUE );
    Delay_AC();

    if (!bPowerOn) {
        RETAILMSG( 1, (TEXT("Control register  =%08x\r\n"), (USHORT)READ_REGISTER_USHORT((PUSHORT)pCODEC_CR )));

        //Codec Ready?
        RETAILMSG( 1, (TEXT("ATAGR   =%08x\r\n"), (ULONG)READ_REGISTER_ULONG((PULONG)pCODEC_ATAGR )));
        RETAILMSG( 1, (TEXT("WAIT Codec Ready..............\r\n")));
    }

    for (dwIndex=0;dwIndex<0x100;dwIndex++) {
        if (( (ULONG)READ_REGISTER_ULONG((PULONG)pCODEC_ATAGR ) & 0x80000000 )) 
            break;
        Delay_AC();
    };

    if (!bPowerOn)
        RETAILMSG( 1, (TEXT("codec ready with cound 0x%lx !! \r\n"),dwIndex));
    
    //RX TX Valid Slot
    WRITE_REGISTER_ULONG((PULONG)pCODEC_ATAGR, 0x00001ff8 );
    Delay_AC();

    WRITE_REGISTER_ULONG((PULONG)pCODEC_CAR, (ULONG)0x000a6000 );
    Delay_AC();

    if (!bPowerOn)
        RETAILMSG( 1, (TEXT("WAIT ADC DAC ANL Ready?..............\r\n")));

    while(( (ULONG)READ_REGISTER_ULONG((PULONG)pCODEC_CDR ) & 0x000000f0 ) != 0x000000f0)
    {
        WRITE_REGISTER_ULONG((PULONG)pCODEC_CAR, (ULONG)0x000a6000 );
        Delay_AC();
    }

    if (!bPowerOn)
        RETAILMSG( 1, (TEXT("111accept Data!!\r\n")));

    WRITE_REGISTER_USHORT((PUSHORT)pCODEC_FSR, 0x0000 );
    Delay_AC();
    WRITE_REGISTER_USHORT((PUSHORT)pCODEC_CR, 0x0200 );
    Delay_AC();

    WRITE_REGISTER_ULONG((PULONG)pCODEC_CAR, (ULONG)0x00002000 );  // Set Master Volume      address
    Delay_AC();
    WRITE_REGISTER_ULONG((PULONG)pCODEC_CAR, (ULONG)0x00018000 );  // Set PCM Out Volume     address
    Delay_AC();
    WRITE_REGISTER_ULONG((PULONG)pCODEC_CAR, (ULONG)0x00078000 );  // Set Sample rate        address
    Delay_AC();

    WRITE_REGISTER_ULONG((PULONG)pCODEC_CDR, (ULONG)0x00000000 );// Set Master Volume        data (zero dB attenuation)
    Delay_AC();
    WRITE_REGISTER_ULONG((PULONG)pCODEC_CDR, (ULONG)0x00000000 );// Set PCM Out Volume       data (zero dB attenuation)
    Delay_AC();
    WRITE_REGISTER_ULONG((PULONG)pCODEC_CDR,  48000 << 4 );      // Set Sample rate          data
    Delay_AC();

    WRITE_REGISTER_USHORT((PUSHORT)pCODEC_FSR, 0x0000 );
    Delay_AC();
    WRITE_REGISTER_USHORT((PUSHORT)pCODEC_CR, 0x0235 );
    Delay_AC();

    //Audio in set up  start

    WRITE_REGISTER_USHORT((PUSHORT)pCODEC_FSR, 0x0000 );
    Delay_AC();
    WRITE_REGISTER_USHORT((PUSHORT)pCODEC_CR, 0x0200 );
    Delay_AC();
    WRITE_REGISTER_ULONG((PULONG)pCODEC_CAR, (ULONG)0x0000E000 );// Set Mic Volume            address
    Delay_AC();
    WRITE_REGISTER_ULONG((PULONG)pCODEC_CAR, (ULONG)0x0001C000 );// Set Record Gain           address
    Delay_AC();
    WRITE_REGISTER_ULONG((PULONG)pCODEC_CAR, (ULONG)0x0001A000 );// Set Record Select Control address
    Delay_AC();
    WRITE_REGISTER_ULONG((PULONG)pCODEC_CDR, (ULONG)0x000c0000 );// Set Mic Volume           data (mute+boost)
    Delay_AC();
    WRITE_REGISTER_ULONG((PULONG)pCODEC_CDR, (ULONG)0x00004040 );// Set Record Gain           data
    Delay_AC();
    WRITE_REGISTER_ULONG((PULONG)pCODEC_CDR, (ULONG)0x00000000 );// Set Record Select Control data
    Delay_AC();
    WRITE_REGISTER_USHORT((PUSHORT)pCODEC_FSR, 0x0000 );
    Delay_AC();
    WRITE_REGISTER_USHORT((PUSHORT)pCODEC_CR, 0x0235 );
    Delay_AC();

    //Audio in set up end

    Delay_AC();
    WRITE_REGISTER_ULONG((PULONG)pCODEC_CAR, (ULONG)0x00082000 );
    WRITE_REGISTER_ULONG((PULONG)pCODEC_CAR, (ULONG)0x00098000 );
    WRITE_REGISTER_ULONG((PULONG)pCODEC_CAR, (ULONG)0x000F8000 );
    WRITE_REGISTER_ULONG((PULONG)pCODEC_CAR, (ULONG)0x0008E000 );
    Delay_AC();
    Delay_AC();
    if (!bPowerOn) {
        RETAILMSG( 1, (TEXT("Master Vol =%x\r\n"), (ULONG)READ_REGISTER_ULONG((PULONG)pCODEC_CDR )));
        RETAILMSG( 1, (TEXT("PCM    Vol =%x\r\n"), (ULONG)READ_REGISTER_ULONG((PULONG)pCODEC_CDR )));
        RETAILMSG( 1, (TEXT("Sample rate=%x\r\n"), (ULONG)READ_REGISTER_ULONG((PULONG)pCODEC_CDR )));
        RETAILMSG( 1, (TEXT("Mic    Vol =%x\r\n"), (ULONG)READ_REGISTER_ULONG((PULONG)pCODEC_CDR )));
    }

    //DQR_En
    
    WRITE_REGISTER_USHORT((PUSHORT)pCODEC_FSR, 0x0000 );
    Delay_AC();
    WRITE_REGISTER_USHORT((PUSHORT)pCODEC_CR, 0x0231 );
    Delay_AC();
    WRITE_REGISTER_ULONG((PULONG)pCODEC_CDR, (ULONG)0x00007800 );
    Delay_AC();
    WRITE_REGISTER_ULONG((PULONG)pCODEC_CAR, (ULONG)0x00007400 );  // DRQ Enable
    Delay_AC();

    WRITE_REGISTER_ULONG((PULONG)pCODEC_CDR, (ULONG)0x00007800 );
    Delay_AC();
    WRITE_REGISTER_ULONG((PULONG)pCODEC_CAR, (ULONG)0x00007400 );  // DRQ Enable
    Delay_AC();

    WRITE_REGISTER_USHORT((PUSHORT)pCODEC_FSR, 0x0000 );
    Delay_AC();
    WRITE_REGISTER_USHORT((PUSHORT)pCODEC_CR, 0x0235 );
    Delay_AC();
    Delay_AC();
    WRITE_REGISTER_ULONG((PULONG)pCODEC_CAR, (ULONG)0x000f4000 );  // Serial Configration
    Delay_AC();
    if (!bPowerOn)
        RETAILMSG( 1, (TEXT("Serial Configration =%08x\r\n"), (ULONG)READ_REGISTER_ULONG((PULONG)pCODEC_CDR )));

    // Set default volume to maximum

    v_nVolume         = 0xFFFFFFFF;

    // Initialize AudioOutInterrupt Flag
/*  pDriverGlobals->aud.play_address = (ULONG)NULL;
    pDriverGlobals->aud.outInt = (USHORT)NULL;
*/  pDmaOutObject->SetGlobalPlayAddress(NULL);
    if (!bPowerOn) {
        RETAILMSG(1, (TEXT("PDD_AudioOutInitialize: Successful Init\r\n")));
        RETAILMSG(1, (TEXT("WaveOutpdd.c private_AudioOutInitialize: END\r\n")));
    }
    FUNC("-PDD_AudioOutInitialize");
    return TRUE;
}

/*****************************************************************************
*   FUNCTION :      private_AudioOutPowerHandler
*   DESCRIPTION :   performs power on/off
*   INPUTS :        bPowerDown 1=power off, 0=power on
*   OUTPUTS :       None
*   DESIGN NOTES :
*   CAUTIONS :
*****************************************************************************/
VOID
private_AudioOutPowerHandler(
    BOOL bPowerDown
    )
{
    BOOL Success;

    FUNC("private_AudioOutPowerHandler");
    DEBUGMSG(ZONE_TEST, (TEXT("private_AudioOutPowerHandler ---- bPowerDown=%d\r\n"),bPowerDown));
    if( bPowerDown )
    {
        private_AudioOutDeinitialize(TRUE);
    }
    else
    {       // power up
        Success=private_AudioOutInitialize(TRUE);
    }
    return; // (see also private_WaveOutClose)
}

/*****************************************************************************
*   FUNCTION :      private_AudioOutDeinitialize
*   DESCRIPTION :   Reset playback device to initial state
*   INPUTS :        None
*   OUTPUTS :       None
*   DESIGN NOTES :
*   CAUTIONS :
*****************************************************************************/
VOID
private_AudioOutDeinitialize(BOOL bPowerOff )
{
    FUNC("+PDD_AudioOutDeinitialize");

/*  if( pDMAC_RegBase )
        VirtualFree((PVOID)pDMAC_RegBase, 0, MEM_RELEASE);
    
    if( pINTC_Area_1_RegBase )
        VirtualFree((PVOID)pINTC_Area_1_RegBase, 0, MEM_RELEASE);
    
    if( pINTC_Area_7_RegBase )
        VirtualFree((PVOID)pINTC_Area_7_RegBase, 0, MEM_RELEASE);
*/  
    if( pCC_CODEC_RegBase  && !bPowerOff)
        VirtualFree((PVOID)pCC_CODEC_RegBase, 0, MEM_RELEASE);

    
    if( pCC_SYS_RegBase && !bPowerOff )
        VirtualFree((PVOID)pCC_SYS_RegBase, 0, MEM_RELEASE);
    
//  if( pAudioBufferBase && !bPowerOff) 
//      VirtualFree((PVOID)pAudioBufferBase, 0, MEM_RELEASE); // This is buffer will be free by pDmaOutObject

    if (pDmaOutObject && !bPowerOff) {
        delete pDmaOutObject;
        pDmaOutObject=NULL;
    }
/*  if( pDriverGlobals )
        VirtualFree((PVOID)pDriverGlobals, 0, MEM_RELEASE);
*/
    FUNC("-PDD_AudioOutDeinitialize");

}

/*****************************************************************************
*   FUNCTION :      private_WaveOutStart
*   DESCRIPTION :   Initiates playback of wave
*   INPUTS :        wave
*   OUTPUTS :       None
*   DESIGN NOTES :  puts double buffer in initial state, starts play
*   CAUTIONS :
*****************************************************************************/
VOID
private_WaveOutStart (
    PWAVEHDR pwh
    )
{
    const PPCMWAVEFORMAT pFormat = (PPCMWAVEFORMAT) g_pwfx[WAPI_OUT];

    FUNC_WPDD("+PDD_WaveOutStart");

    v_fMoreData[WAPI_OUT] = TRUE;                       // more data expected

    v_nNextPage = 0;

    private_AudioFillBuffer (pwh, pAudioBufferBase, 2 * pDmaOutObject->GetAudioDmaPageSize());

    pDmaOutObject->SetGlobalAddr(FALSE);
    v_nNextPage = 0;

    //RX TX Valid Slot
    
    WRITE_REGISTER_ULONG((PULONG)pCODEC_ATAGR, 0x00001ff8 );
    private_AudioPlayFirstBuffer();                     // start playback

    FUNC_WPDD("-PDD_WaveOutStart");
}

/*****************************************************************************
*   FUNCTION :      private_AudioFillBuffer
*   DESCRIPTION :   fills next buffer from given wave
*                   toggles ready/empty pointers to double buffer
*   INPUTS :        wave
*   OUTPUTS :       None
*   DESIGN NOTES :
*   CAUTIONS :
*****************************************************************************/
VOID
private_AudioFillBuffer_Generic (
    PWAVEHDR pwh,
	PBYTE pDstBuffer,
	DWORD cbDstBytes
    )
{
    const PPCMWAVEFORMAT pFormat = (PPCMWAVEFORMAT) g_pwfx[WAPI_OUT];

    ULONG       l_sample = 0;
    ULONG       r_sample = 0;
    UCHAR       l_wksample = 0;
    UCHAR       r_wksample = 0;
    PCM_SAMPLE  pcmsample;
    PPCM_SAMPLE pSampleSource;
    PULONG      pDstPtr;
    ULONG       ulDstSampleCount;
    LONG        dda_minor;
    LONG        dda_major;
    LONG        dda_accum;

    FUNC_VERBOSE("+PDD_AudioFillBuffer");

    //Set terminal condition for double-buffer and exit if no data remains

    if (pwh == NULL) 
    {
        //pDriverGlobals->aud.play_address = (ULONG)NULL;
        pDmaOutObject->SetGlobalPlayAddress(NULL);
        return;
    }

    // perform sample-rate-conversion using a DDA:
    // for each output sample {
    //    Accumulator += MajorTerm;
    //    if Accumulator <= 0 {
    //        fetch next source sample;
    //    }
    //    Accumulator -= MinorTerm;
    //    write current source sample to destination
    //  }
    // This approach allows conversion from arbitray input sample rates
    // The implementation here has one deficiency - the state of the dda_accum
    // is not preserved across calls to private_waveOutFillBuffer
    // This may lead to minor artifacts at certain sample rates (especially low ones)
    // but requires some extra overhead of saving and restoring the state between calls.

    dda_minor = pFormat->wf.nSamplesPerSec;
    dda_major = 48000; // Codec is set up to run at 48KHz
    dda_accum = 0;

    pDstPtr = (ULONG *)pDstBuffer;

    //
    // For each sample in the buffer...
    //

    ulDstSampleCount = cbDstBytes / 8; // 64-bit samples: 2x32-bit

    while (ulDstSampleCount > 0) 
    {
        if (dda_accum <= 0) {
            // if accumulator reaches zero, time to fetch another source sample
            dda_accum += dda_major;

            if (pwh != NULL && pwh->reserved >= pwh->dwBufferLength) {
                pwh = pwh->lpNext;
            }
            if (pwh == NULL) {
                l_sample = 0;
                r_sample = l_sample;
            }
            else {
                pSampleSource = (PPCM_SAMPLE) (pwh->lpData + pwh->reserved);

                switch (g_pcmtype[WAPI_OUT])
                {
                case PCM_TYPE_M8:
                    l_sample = (INT16) pSampleSource->m8.sample;
                    pwh->reserved++;
                    l_sample = l_sample -128;
                    l_sample = l_sample << 12;
                    r_sample = l_sample;
                    break;

                case PCM_TYPE_S8:
                    pcmsample.s8.sample_left  = pSampleSource->s8.sample_left;
                    pcmsample.s8.sample_right = pSampleSource->s8.sample_right;
                    pwh->reserved += 2;

                    l_sample = pcmsample.s8.sample_left;
                    r_sample = pcmsample.s8.sample_right;

                    l_sample = l_sample -128;
                    l_sample = l_sample << 12;
                    r_sample = r_sample -128;
                    r_sample = r_sample << 12;
                    break;

                case PCM_TYPE_M16:
                    l_sample = pSampleSource->m16.sample;
                    pwh->reserved += 2;
                    l_sample = l_sample << 4;
                    r_sample = l_sample;
                    break;

                case PCM_TYPE_S16:
                    pcmsample.s16.sample_left  = pSampleSource->s16.sample_left;
                    pcmsample.s16.sample_right = pSampleSource->s16.sample_right;
                    pwh->reserved += 4;
                    l_sample = pcmsample.s16.sample_left;
                    r_sample = pcmsample.s16.sample_right;
                    l_sample = l_sample << 4;
                    r_sample = r_sample << 4;
                    break;
                }  // end switch
                l_sample   = ( l_sample & 0x000fffff );
                r_sample   = ( r_sample & 0x000fffff );
            }  // endif pwh == NULL
        } // endif dda_accum <= 0

        *pDstPtr++ = l_sample;
        *pDstPtr++ = r_sample;
        ulDstSampleCount --;
        dda_accum -= dda_minor;
    } // while (ulDstSampleCount)

    FUNC_VERBOSE("-PDD_AudioFillBuffer");
}

VOID
private_AudioFillBuffer_Fast (
    PWAVEHDR pwh,
	PBYTE pDstBuffer,
	DWORD cbDstBytes
    )
{
    BYTE bSilence;
    PULONG      pDstPtr, pSrcPtr;
    ULONG       ulDstSampleCount, ulSrcSampleCount;
    ULONG       ulSample;

    FUNC_VERBOSE("+PDD_AudioFillBufferDirect");

    //Set terminal condition for double-buffer and exit if no data remains

    if (pwh == NULL)  {
        //pDriverGlobals->aud.play_address = (ULONG)NULL;
        pDmaOutObject->SetGlobalPlayAddress(NULL);
        return;
    }

    bSilence = (g_pwfx[WAPI_OUT]->wBitsPerSample == 8) ? 0x80 : 0;
    
    pDstPtr = (ULONG *) pDstBuffer;
    ulDstSampleCount = cbDstBytes / 8; // 64-bit samples: 2x32-bit

    while (ulDstSampleCount > 0 && pwh != NULL) {
        ulSrcSampleCount = (pwh->dwBufferLength - pwh->reserved) / 4; // source is 4-byte samples
        pSrcPtr = (PULONG) (pwh->lpData + pwh->reserved);

        ULONG ulSampleCount = min(ulSrcSampleCount, ulDstSampleCount);
        DWORD dwBytesRecorded = ulSampleCount * 4;

        ulDstSampleCount -= ulSampleCount;

        while (ulSampleCount -- > 0) {
            ulSample = *pSrcPtr++;
            pDstPtr[0] = (ulSample <<  4) & 0xffff0;
            pDstPtr[1] = (ulSample >> 12) & 0xffff0;
            pDstPtr += 2;
        }

        pwh->reserved += dwBytesRecorded;
        if (pwh->reserved >= pwh->dwBufferLength) {
            pwh = pwh->lpNext;
        }

    } // while (ulDstSampleCount)

    // pad with silence if necessary
    if (ulDstSampleCount > 0) {
        memset(pDstPtr, bSilence, ulDstSampleCount * 8);
    }

    FUNC_VERBOSE("-PDD_AudioFillBufferDirect");
}

/*****************************************************************************
*   FUNCTION :      private_AudioPlayFirstBuffer
*   DESCRIPTION :   Starts playback of first buffer
*   INPUTS :        None
*   OUTPUTS :       None
*   DESIGN NOTES :
*   CAUTIONS :
*****************************************************************************/
VOID
private_AudioPlayFirstBuffer(
   VOID
   )
{
    FUNC_WPDD("+PDD_AudioPlayFirstBuffer");
    WRITE_REGISTER_ULONG((PULONG)pCODEC_ACR, (ULONG)0x000effff );   //adjust L and R data
    WRITE_REGISTER_ULONG((PULONG)pCODEC_ACR, (ULONG)0x002fffff );   //adjust L and R data
/*
    WRITE_REGISTER_ULONG((PULONG)pSAR1,    (ULONG)(dma_pagePhysicalAddress[0]));    // Setting DMA controler
    WRITE_REGISTER_ULONG((PULONG)pDAR1,    (ULONG)CC_CODEC_PCML );                  // Prepare DMA Channel 2 for audio playback
    WRITE_REGISTER_ULONG((PULONG)pDMATCR1, (ULONG)( AUDIO_DMA_PAGE_SIZE / 4 ));// Rig DMA for Transfer of First Buffer
*/  pDmaOutObject->ArmDma(TRUE,(ULONG)(dwCCCodecRegBase+CC_CODEC_PCML_OFFSET)/*CC_CODEC_PCML*/);
    // ClearDMA2Interrupt();                    // (see macros)

    WRITE_REGISTER_ULONG((PULONG)pCODEC_ACR, (ULONG)0x000effff );
//  WRITE_REGISTER_ULONG((PUSHORT)pDMAOR, SH3_DMAC_DMAOR_DME );
    pDmaOutObject->EnableDma();

//  pDriverGlobals->aud.outInt = (USHORT)NULL;
  
   // Begin Play of First Buffer

/*  READ_REGISTER_ULONG((PULONG)pCHCR1  );
    WRITE_REGISTER_ULONG((PULONG)pCHCR1, 
        SH3_DMAC_CHCR_RL_ACTIVE_HIGH |
        SH3_DMAC_CHCR_SM_INCREMENTED |
        SH3_DMAC_CHCR_TS_32 |
        SH3_DMAC_CHCR_IE_GENERATED |
        SH3_DMAC_CHCR_DE_ENABLED );
*/  pDmaOutObject->StartDma();

    WRITE_REGISTER_ULONG((PULONG)pCODEC_ACR, (ULONG)0x002fffff );   // play start

    FUNC_WPDD("-PDD_AudioPlayFirstBuffer");
}

/*****************************************************************************
*   FUNCTION :      private_WaveOutContinue
*   DESCRIPTION :   Fill next buffer from wave
*   INPUTS :        wave
*   OUTPUTS :       None
*   DESIGN NOTES :
*   CAUTIONS :
*****************************************************************************/
VOID
private_WaveOutContinue (
    PWAVEHDR pwh
    )
{
    FUNC_VERBOSE("+PDD_WaveOutContinue");

    WRITE_REGISTER_ULONG((PULONG)pCODEC_ACR, 0x03fffff);	//adjust L and R data
    pDmaOutObject->ContinueDma();
    WRITE_REGISTER_ULONG((PULONG)pCODEC_ACR, (ULONG)0x00200000 );  	 // play restart
    
    v_fMoreData[WAPI_OUT] = TRUE;                 // more data expected
	//
	DWORD dwPageSize = pDmaOutObject->GetAudioDmaPageSize();

    private_AudioComplete (pwh, dwPageSize);
    private_AudioFillBuffer (pwh, pAudioBufferBase + v_nNextPage * dwPageSize, dwPageSize);

    // Point AudioPlayingAddress to buffer just filled; toggle v_nNextPage

    pDmaOutObject->SetGlobalAddr(v_nNextPage==0);
    v_nNextPage = 1 - v_nNextPage;


//  pDriverGlobals->aud.outInt = (USHORT)NULL;
//  *(pDmaOutObject->GetGlobalOutIntPointer())=NULL;
    FUNC_VERBOSE("-PDD_WaveOutContinue");
}

//
// -----------------------------------------------------------------------------
//

MMRESULT
private_WaveOutRestart (
    PWAVEHDR pwh
    )
{
    FUNC_VERBOSE("+PDD_WaveOutRestart");
//DMA Restart
/*  WRITE_REGISTER_ULONG((PULONG)pCHCR1, 
        SH3_DMAC_CHCR_RL_ACTIVE_HIGH |
        SH3_DMAC_CHCR_SM_INCREMENTED |
        SH3_DMAC_CHCR_TS_32 |
        SH3_DMAC_CHCR_IE_GENERATED |
        SH3_DMAC_CHCR_DE_ENABLED );
*/  pDmaOutObject->ReStartDma();
    FUNC_VERBOSE("-PDD_WaveOutRestart");
    return(MMSYSERR_NOERROR);
}

//
// -----------------------------------------------------------------------------
//

MMRESULT
private_WaveOutPause (
    VOID
    )
{
    FUNC_VERBOSE("+PDD_WaveOutPause");
//DMA STOP
/*  WRITE_REGISTER_ULONG((PULONG)pCHCR1, 
        SH3_DMAC_CHCR_RL_ACTIVE_HIGH |
        SH3_DMAC_CHCR_SM_INCREMENTED |
        SH3_DMAC_CHCR_TS_32 |
        SH3_DMAC_CHCR_IE_GENERATED |
        SH3_DMAC_CHCR_DE_DISABLED );
*/  pDmaOutObject->StopDma();
//FIFO buffer clear
    ClearFifoBuffer();
    FUNC_VERBOSE("-PDD_WaveOutPause");
    return(MMSYSERR_NOERROR);
}

/*****************************************************************************
*   FUNCTION :      private_WaveOutStop
*   DESCRIPTION :   Shut down audio playback
*   INPUTS :        None
*   OUTPUTS :       None
*   DESIGN NOTES :
*   CAUTIONS :
*****************************************************************************/
VOID
private_WaveOutStop()
{
    FUNC_WPDD("+private_WaveOutStop");

    v_fMoreData[WAPI_OUT] = FALSE;              // probably done already, just making
    //pDriverGlobals->aud.play_address = (ULONG)NULL;       //    sure no more buffers are played
    pDmaOutObject->SetGlobalPlayAddress(NULL);
//DMA STOP
/*  WRITE_REGISTER_ULONG((PULONG)pCHCR1, 
        SH3_DMAC_CHCR_RL_ACTIVE_HIGH |
        SH3_DMAC_CHCR_SM_INCREMENTED |
        SH3_DMAC_CHCR_TS_32 |
        SH3_DMAC_CHCR_IE_GENERATED |
        SH3_DMAC_CHCR_DE_DISABLED );
*/
    pDmaOutObject->StopDma();
    ClearFifoBuffer();		//FIFO buffer clear	
    WRITE_REGISTER_ULONG((PULONG)pCODEC_ATAGR, 0x0);	//stop noise

    FUNC_WPDD("-private_WaveOutStop");
}

/*****************************************************************************
*   FUNCTION :      private_WaveOutEndOfData
*   DESCRIPTION :   Rig to play no more buffers
*   INPUTS :        None
*   OUTPUTS :       None
*   DESIGN NOTES :
*   CAUTIONS :
*****************************************************************************/
VOID
private_WaveOutEndOfData()
{
    FUNC_WPDD("+PDD_WaveOutEndOfData");

//  pDriverGlobals->aud.play_address = (ULONG)NULL;                     // make sure no more buffers are played
    pDmaOutObject->SetGlobalPlayAddress(NULL);
    v_fMoreData[WAPI_OUT] = FALSE;                              // record invocation by MDD of StopPlay
//FIFO buffer clear
    ClearFifoBuffer();
    FUNC_WPDD("-PDD_WaveOutEndOfData");
}

/*****************************************************************************
*   FUNCTION :      private_WaveOutStandby
*   DESCRIPTION :
*   INPUTS :        None
*   OUTPUTS :       None
*   DESIGN NOTES :
*   CAUTIONS :
*****************************************************************************/
VOID
private_WaveOutStandby()
{
    FUNC_WPDD("+PDD_WaveOutStandby");
    FUNC_WPDD("-PDD_WaveOutStandby");
}

/*****************************************************************************
*   FUNCTION :      private_WaveOutOpen
*   DESCRIPTION :   Extract and Decipher Wave Format Parameters
*   INPUTS :        None
*   OUTPUTS :       None
*   DESIGN NOTES :
*   CAUTIONS :
*****************************************************************************/
MMRESULT
private_WaveOutOpen(
    LPWAVEFORMATEX lpFormat,
    BOOL fQueryFormatOnly
    )
{
    MMRESULT mmRet;
    BOOL formatOK;

    FUNC_WPDD("+PDD_WaveOutOpen");

    // Allow PCM, mono, 8 or 16 bit within allow playback speed range

    formatOK = (
        (lpFormat->wFormatTag == WAVE_FORMAT_PCM) &&
        (lpFormat->nChannels == Monaural || lpFormat->nChannels == Stereo) &&
        (lpFormat->wBitsPerSample == SixteenBits || lpFormat->wBitsPerSample == EightBits) &&
        (lpFormat->nSamplesPerSec >= MinPlaybackSpeed) &&
        (lpFormat->nSamplesPerSec <= MaxPlaybackSpeed));

    if ( !formatOK )
    {
        DEBUGMSG(ZONE_TEST, (TEXT("PDD_AudioQueryFormat:\n %d; %d; %d; %d,\r\n"),
            lpFormat->wFormatTag,
            lpFormat->nChannels,
            lpFormat->wBitsPerSample,
            lpFormat->nSamplesPerSec));
        mmRet = WAVERR_BADFORMAT;
    }
    else
    {
        mmRet = MMSYSERR_NOERROR;
        if (!fQueryFormatOnly)
        {
            //
            // NOTE : If the hardware supports input/output one at a time,
            //        the driver should return MMSYSERR_ALLOCATED here.
            //
            if (g_fInUse) 
            {
                mmRet = MMSYSERR_ALLOCATED;
            }
            else 
            {
                // mark the output channel in use
                g_fInUse = TRUE;

                // Open with the given format.
                g_pwfx[WAPI_OUT] = lpFormat;
                private_AudioFillBuffer = private_AudioFillBuffer_Generic; // default to generic case
                if (g_pwfx[WAPI_OUT]->wBitsPerSample == EightBits)
                {
                    if (g_pwfx[WAPI_OUT]->nChannels == Monaural)
                        g_pcmtype[WAPI_OUT] = PCM_TYPE_M8;
                    else
                        g_pcmtype[WAPI_OUT] = PCM_TYPE_S8;
                }
                else
                {
                    if (g_pwfx[WAPI_OUT]->nChannels == Monaural)
                        g_pcmtype[WAPI_OUT] = PCM_TYPE_M16;
                    else {
                        g_pcmtype[WAPI_OUT] = PCM_TYPE_S16;
                        if (g_pwfx[WAPI_OUT]->nSamplesPerSec == 48000) { 
                            // normal case under Software Mixer is 16bit Stereo48K. Optimize for it
                            private_AudioFillBuffer = private_AudioFillBuffer_Fast;
                        }
                    }
                }
            }
        }
    }

#if 0
	// disabling this test. it comes up false-positive on some devices that otherwise work.
	//if Recording  not Play
    ULONG temp_SAR1    = pDmaOutObject->GetDmaSourceReg();//READ_REGISTER_ULONG((PULONG)pSAR1  );
    ULONG temp_DMATCR1 = pDmaOutObject->GetDmaCountReg();//READ_REGISTER_ULONG((PULONG)pDMATCR1  );
    if  ( ( temp_SAR1 == (ULONG)(dwCCCodecRegBase/*CC_CODEC_REGBASE*/ + CC_CODEC_PCML_OFFSET) ) &&
          ( temp_DMATCR1 != 0x00000000 )                        )
    {
		// this looks like an attempt to prevent full-duplex open, which is covered by the 
		// g_fInUse flag.
		// In any case, clear the in-use flag or we'll never open again
		RETAILMSG(1, (TEXT("HD65564 Weirdness: SAR1=%08x DMACTR=%08x\r\n"), temp_SAR1, temp_DMATCR1));
		g_fInUse = FALSE;
        mmRet = MMSYSERR_NOMEM;
    }
#endif


    FUNC_WPDD("-PDD_WaveOutOpen");
    return(mmRet);
}

/*****************************************************************************
*   FUNCTION :      private_WaveOutClose
*   DESCRIPTION :
*   INPUTS :        None
*   OUTPUTS :       None
*   DESIGN NOTES :
*   CAUTIONS :
*****************************************************************************/
VOID
private_WaveOutClose()
{
    FUNC_WPDD("+PDD_WaveOutClose");

    g_fInUse = FALSE;

    // No Actions Needed
    //   unless D/A enable/disable strategy changes to on/off for each wave

    FUNC_WPDD("+PDD_WaveOutClose");
}

/*****************************************************************************
*   FUNCTION :      private_AudioGetVolume
*   DESCRIPTION :   retrieves current volume setting
*   INPUTS :        None
*   OUTPUTS :       current volume setting
*   DESIGN NOTES :
*   CAUTIONS :
*****************************************************************************/
ULONG private_AudioGetVolume(
    VOID
    )
{
    FUNC_WPDD("+PDD_AudioGetVolume");
    return v_nVolume;
    FUNC_WPDD("-PDD_AudioGetVolume");
}

/*****************************************************************************
*   FUNCTION :      private_AudioSetVolume
*   DESCRIPTION :   sets volume
*   INPUTS :        volume setting
*   OUTPUTS :       None
*   DESIGN NOTES :
*   CAUTIONS :
*****************************************************************************/


VOID private_AudioSetVolume(
    ULONG ulVolume
    )
{
    FUNC_WPDD("+PDD_AudioSetVolume");
    v_nVolume = ulVolume; // keep track of what the nominal volume is

    // Device-level SetVolume semantics: 
    // Set the "global" volume that affects all other streams uniformly
	// In the case of this device, that means the AC97_PCMOUT_VOL register.

	ULONG ulLeft  = (ulVolume & 0x0000F800) >> 11;
	ULONG ulRight = (ulVolume & 0xF8000000) >> 27;
	
	ULONG ulRegVal;
	if (ulVolume == 0) {
		// if zero, jult mute the output
		ulRegVal = 0x8000;
	}
	else {
		ulLeft  = 31 - ulLeft;
		ulRight = 31 - ulRight;
		ulRegVal = (ulRight << 8) | ulLeft;
	}


    WRITE_REGISTER_USHORT((PUSHORT)pCODEC_FSR, 0x0000 );
    Delay_AC();
    WRITE_REGISTER_USHORT((PUSHORT)pCODEC_CR, 0x0200 );
    Delay_AC();

    WRITE_REGISTER_ULONG((PULONG)pCODEC_CAR, (ULONG)0x00018000 );  // Set PCM Out Volume     address
    Delay_AC();

    WRITE_REGISTER_ULONG((PULONG)pCODEC_CDR, ulRegVal << 4 ); // bits 4-20
    Delay_AC();

    WRITE_REGISTER_USHORT((PUSHORT)pCODEC_FSR, 0x0000 );
    Delay_AC();
    WRITE_REGISTER_USHORT((PUSHORT)pCODEC_CR, 0x0235 );
    Delay_AC();

    FUNC_WPDD("-PDD_AudioSetVolume");
    return;
}

/*****************************************************************************
*   FUNCTION :      Delay_AC
*   DESCRIPTION :   wait
*   INPUTS :        None
*   OUTPUTS :       None
*   DESIGN NOTES :
*   CAUTIONS :
*****************************************************************************/
void Delay_AC(void)
{
    // in a platform-independent way, stall for *at least* one millisecond,
    // which seems to be plenty long enough for codec registers to stabilize
/*  Does not work during power down/up cycle. The TickCount stopped during the cycle.
    const DWORD delay = 1;
    DWORD deadline = GetTickCount() + delay;
    while (GetTickCount() <= deadline);
*/
/*    volatile unsigned long i;
    for (i=0;i<100000;i++);
    return;
*/
	DDKDriverSleep(1);
}
/*****************************************************************************
*   FUNCTION :      ClearFifoBuffer
*   DESCRIPTION :   companion chip FIFO buffer clear
*   INPUTS :        None
*   OUTPUTS :       None
*   DESIGN NOTES :  PCML * 4  PCMR * 4
*   CAUTIONS :
*****************************************************************************/
void ClearFifoBuffer(void)
{
    int BufferCount;
    for( BufferCount = 0 ; BufferCount < 8 ; BufferCount++ ){
        WRITE_REGISTER_ULONG((PULONG)pCODEC_PCML, (ULONG)0x00000000 );
    }
    return;
}

// -------------------------------------------------------------
// private_AudioComplete
// 
// Update queued wave headers to reflect the fact that we just
// finished playing some data.
// Advance the dwBytesRecorded field in each successive header
// until we've accounted for all the data played or run out
// of queued data, which can happen at the end of playback.
// 
// Note that we can only mark as completed that which has
// actually been transferred into the DMA buffer for playback.
// Therefore we advance the .dwBytesCompleted as far as the .reserved
// field and ignore the actual buffer size.
// -------------------------------------------------------------
VOID
private_AudioComplete(PWAVEHDR pwh, DWORD cbBytesCompleted)
{
	while ((pwh != NULL) && (cbBytesCompleted > 0)) {
		if (pwh->dwBytesRecorded >= pwh->reserved) {
			pwh = pwh->lpNext;
		}
		else {
			ULONG cbBytesLeft = pwh->reserved - pwh->dwBytesRecorded;
			ULONG cbComplete = min(cbBytesCompleted, cbBytesLeft);
			cbBytesCompleted -= cbComplete;
			pwh->dwBytesRecorded += cbComplete;
		}
	}
}
