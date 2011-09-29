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

		waveinpdd.cpp

	Revision History:

		26th April		1999		Released
		30th June		1999		Modified audio in buffer address
									Added AudioInInterrupt Flag
		26th October	1999		Modified WaveData length thin out
        29-Nov-1999 cea		Changed for SH3 (7709A) DMA control
*/

#include <windows.h>
#include <types.h>
#include <memory.h>
#include <excpt.h>
#include <waveddsi.h>
#include <wavedbg.h>
#include <mmsystem.h>
#include <Excpt.h>
#include "wavepdd.h"
#include "waveInpdd.h"

#include "cc.h"
#include "macros.h"
#include "wavecpud.h"

extern "C" PUCHAR GetVirtualAddressOfUncachedMemory( PVOID base, ULONG sz, char* cID );
extern BOOL g_fInUse;
extern PWAVEFORMATEX g_pwfx[2];
extern PCM_TYPE g_pcmtype[2];

// audio in globals

// pointer to the physical memory addresses

DmaObject * pDmaInObject=NULL;
extern DWORD dwCCCodecRegBase;//CC_CODEC_REGBASE
extern DWORD dwCCSysSMCR;//CC_SYS_SMSCR

//PVBYTE *pAudioRecAddress;

// Double Buffer for Audio Playback
//

static ULONG v_recPage;

static  PBYTE   pCC_CODEC_RegBase;	// Companion chip  SERIAL CODEC register offsets
static  PBYTE   pCC_SYS_RegBase;	// Companion chip  SMSCR
static  PBYTE   pAudioBufferBase;	// Audio buffer


// Companion Chip Registers

static PVUSHORT pCODEC_CR;						// virtual addrs of C.C. registers
static PVULONG  pCODEC_ACR, pCODEC_CAR, pCODEC_CDR;
static PVULONG  pCODEC_ATAGR;
static PVULONG  pCODEC_PCML;
static PVULONG  pSMSCR;
static PVUSHORT pCODEC_FSR;

// statistics

DWORD g_dwOverruns;		// # of overrun errors
DWORD g_dwFrames;		// # of 48 word frames filled

LPWAVEFORMATEX lpFormat2 = 0;

#define DUMPEXCEPTION() \
	DEBUGMSG(ZONE_ERROR,(TEXT("Exception %d @ %s:%d\r\n"), GetExceptionCode(), __FILE__, __LINE__ ))

/*****************************************************************************
*   FUNCTION :      private_AudioIsIn
*   DESCRIPTION : returns true if dma is set for audio in
*   INPUTS :        	None
*   OUTPUTS :       	bool value
*   DESIGN NOTES :
*   CAUTIONS :
*****************************************************************************/
BOOL 
private_AudioIsIn()
{
    if (pDmaInObject->GetDmaSourceReg() == (dwCCCodecRegBase + CC_CODEC_PCML_OFFSET))
        return true;

    return false;
}

/*****************************************************************************
*   FUNCTION :  	private_AudioInGetInterruptType
*   DESCRIPTION :   decodes type of audio interrupt
*   INPUTS :		None
*   OUTPUTS :     	interrupt type
*   DESIGN NOTES :  
*   CAUTIONS :		
*****************************************************************************/
AUDIO_STATE 
private_AudioInGetInterruptType(
   VOID
   )
{
    //if ( pDriverGlobals->aud.inInt == (USHORT)NULL )
    if (pDmaInObject == NULL || !private_AudioIsIn())
    {
        DEBUGMSG(ZONE_TEST, (TEXT("PDD_AudioInGetInterruptType:  ignore")));
        return AUDIO_STATE_IGNORE;
    }

    //Stop the codec. Will be restarted later if needed
    WRITE_REGISTER_ULONG((PULONG)pCODEC_ACR, (ULONG)0x0000000 );   // rec stop
    	
    DEBUGMSG(ZONE_TEST, (TEXT("PDD_AudioInGetInterruptType:  recording")));
    return AUDIO_STATE_IN_RECORDING;
}

/*****************************************************************************
*   FUNCTION :  	private_AudioInInitialize
*   DESCRIPTION :   sets up register access for audio-in
*					sets delay time for audio-in start-up
*   INPUTS :		TRUE iff successful
*   OUTPUTS :     	None
*   DESIGN NOTES :  
*   CAUTIONS :		
*****************************************************************************/
BOOL
private_AudioInInitialize(
   VOID
   )
{
	//ULONG SizeOfBuffer;
	char here[] = "PDD_AudioInInitialize";

	FUNC_WPDD("+PDD_AudioInInitialize");
	RETAILMSG( 1, (TEXT("WaveInpdd.c private_AudioInInitialize: START\r\n")));

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
*/
/*
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
	pCC_CODEC_RegBase =(PBYTE)GetVirtualAddressOfUncachedMemory(
		(PBYTE)dwCCCodecRegBase,//CC_CODEC_REGBASE,
		(DWORD)CC_CODEC_REGSIZE,
		"PDD_AudioInitialize, pCC_CODEC_RegBase");

	if (pCC_CODEC_RegBase == NULL) return false;

	pCODEC_CR    = (PVUSHORT) (pCC_CODEC_RegBase + CC_CODEC_CR_OFFSET     );
	pCODEC_CAR   = (PVULONG ) (pCC_CODEC_RegBase + CC_CODEC_CAR_OFFSET    );
	pCODEC_CDR   = (PVULONG ) (pCC_CODEC_RegBase + CC_CODEC_CDR_OFFSET    );
	pCODEC_ACR   = (PVULONG ) (pCC_CODEC_RegBase + CC_CODEC_ACR_OFFSET    );
	pCODEC_ATAGR = (PVULONG ) (pCC_CODEC_RegBase + CC_CODEC_ATAGR_OFFSET  );
	pCODEC_PCML  = (PVULONG ) (pCC_CODEC_RegBase + CC_CODEC_PCML_OFFSET   );
	pCODEC_FSR   = (PVUSHORT) (pCC_CODEC_RegBase + CC_CODEC_FSR_OFFSET    );

	pCC_SYS_RegBase =(PBYTE)GetVirtualAddressOfUncachedMemory(
		(PBYTE)dwCCSysSMCR,//CC_SYS_SMSCR,
		(DWORD)CC_SYS_REGSIZE,
		"PDD_AudioInitialize, pCC_SYS_RegBase");

	if (pCC_SYS_RegBase == NULL) return FALSE;
	pSMSCR       = (PVULONG ) (pCC_SYS_RegBase );

	// Set pointers to virtual addresses of audio buffers
/*
	SizeOfBuffer = (AUDIO_DMA_PAGE_REC_SIZE * 2);

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
	if (pDmaInObject==NULL) {
		pDmaInObject= CreateDmaObject(TRUE); // True for record dma buffer
		ASSERT(pDmaInObject);
	};
	pAudioBufferBase=(PBYTE)pDmaInObject->GetDmaVirtualAddr();
	if (pAudioBufferBase == NULL)
	{
		RETAILMSG( 1, (TEXT("pAudioBufferBase ---error---\r\n")));
		return FALSE;
	}

	// Setup a Access to AudioPlayingAddress and AudioInInterrupt

/*	pDriverGlobals = VirtualAlloc(
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
// Initialize AudioInInterrupt Flag
//	pAudioRecAddress = pDriverGlobals->aud.play;
/*	pDriverGlobals->aud.rec_address = (ULONG)NULL;
	pDriverGlobals->aud.inInt = (USHORT)NULL;
*/	pDmaInObject->SetGlobalRecAddress(NULL);
	

	RETAILMSG(1, (TEXT("PDD_AudioInInitialize: Successful Init\r\n")));
	RETAILMSG(1, (TEXT("WaveInpdd.c private_AudioInInitialize: END\r\n")));
	FUNC("-PDD_AudioInInitialize");
	return TRUE;
}

/*****************************************************************************
*   FUNCTION :  	private_AudioInPowerHandler
*   DESCRIPTION :   performs power on/off, this also happens at open/close!
*   INPUTS :		bPowerDown 1=power off, 0=power on
*					bInKernel 1=in kernel (don't call OS), 0=in IST
*   OUTPUTS :     	None
*   DESIGN NOTES :  
*   CAUTIONS :		Don't call OS when in kernel mode
*****************************************************************************/
VOID
private_AudioInPowerHandler(
	BOOL bPowerDown,
	BOOL bInKernel
	)
{
}

VOID
AudioOutPowerHandler(
	BOOL bPowerDown
	)
{
	return; // Power Handling Not Yet Implemented
}

/*****************************************************************************
*   FUNCTION :  	private_AudioInDeinitialize
*   DESCRIPTION :   Free memory required by driver
*   INPUTS :		None
*   OUTPUTS :     	None
*   DESIGN NOTES :  
*   CAUTIONS :		
*****************************************************************************/
VOID 
private_AudioInDeinitialize(
	VOID
	)
{
	FUNC("+PDD_AudioInDeinitialize");
	FUNC("-PDD_AudioInDisable");
}

/*****************************************************************************
*   FUNCTION :  	private_AudioInStart
*   DESCRIPTION :   start recording a sound
*   INPUTS :		pwh: wave header to insert sound into
*   OUTPUTS :     	None
*   DESIGN NOTES :  
*   CAUTIONS :		
*****************************************************************************/
VOID
private_WaveInStart(
	PWAVEHDR pwh
	)
{
	FUNC_WPDD("+PDD_WaveInStart");
	v_recPage  = 0;

	//Stop DMA
	
/*	READ_REGISTER_ULONG((PULONG)pCHCR1  );
	WRITE_REGISTER_ULONG((PULONG)pCHCR1, 
		SH3_DMAC_CHCR_RL_ACTIVE_HIGH |
		SH3_DMAC_CHCR_DM_INCREMENTED |
		SH3_DMAC_CHCR_TS_32 |
		SH3_DMAC_CHCR_IE_NOT_GENARATED |
		SH3_DMAC_CHCR_DE_DISABLED );
*/	pDmaInObject->StopDma();
	WRITE_REGISTER_ULONG((PULONG)pCODEC_ACR, (ULONG)0x000effff );	//adjust L and R data
	WRITE_REGISTER_ULONG((PULONG)pCODEC_ACR, (ULONG)0x007fffff );	//adjust L and R data

	// Rig DMA for Transfer of First Buffer
	
/*	WRITE_REGISTER_ULONG((PULONG)pDAR1,    (ULONG)(dma_pagePhysicalAddress[0]));
	WRITE_REGISTER_ULONG((PULONG)pSAR1,    (ULONG)CC_CODEC_PCML );
	WRITE_REGISTER_ULONG((PULONG)pDMATCR1, (ULONG)( AUDIO_DMA_PAGE_REC_SIZE / 4 ));	// set data count
*/	pDmaInObject->ArmDma(TRUE,(ULONG)(dwCCCodecRegBase+CC_CODEC_PCML_OFFSET)/*CC_CODEC_PCML*/);// Arm First Dma Buffer.
	// Debug: DMA, Status Report

	WRITE_REGISTER_ULONG((PULONG)pCODEC_ACR, (ULONG)0x007fffff );
//	WRITE_REGISTER_USHORT((PUSHORT)pDMAOR, SH3_DMAC_DMAOR_DME );
	pDmaInObject->EnableDma();

	// Begin Play of First Buffer
/*	
	READ_REGISTER_ULONG((PULONG)pCHCR1  );
	WRITE_REGISTER_ULONG((PULONG)pCHCR1, 
		SH3_DMAC_CHCR_RL_ACTIVE_HIGH |
		SH3_DMAC_CHCR_DM_INCREMENTED |
		SH3_DMAC_CHCR_TS_32 |
		SH3_DMAC_CHCR_IE_NOT_GENARATED |
		SH3_DMAC_CHCR_DE_ENABLED );

	READ_REGISTER_ULONG((PULONG)pCHCR1  );
	WRITE_REGISTER_ULONG((PULONG)pCHCR1,
		SH3_DMAC_CHCR_RL_ACTIVE_HIGH |
		SH3_DMAC_CHCR_DM_INCREMENTED |
		SH3_DMAC_CHCR_TS_32 |
		SH3_DMAC_CHCR_IE_GENERATED |
		SH3_DMAC_CHCR_DE_ENABLED );
*/	pDmaInObject->StartDma(FALSE);
	pDmaInObject->StartDma();

	WRITE_REGISTER_ULONG((PULONG)pCODEC_ACR, (ULONG)0x007fffff );   // rec start

	v_recPage = 1 - v_recPage;
//	WriteAudioRecAddress(dma_pagePhysicalAddress[v_recPage]);//[macro]
/*	pDriverGlobals->aud.rec_address = dma_pagePhysicalAddress[v_recPage];
	pDriverGlobals->aud.inInt = (USHORT)NULL;
*/	pDmaInObject->SetGlobalAddr(v_recPage==0);
	FUNC_WPDD("-PDD_WaveInStart");
}

/*****************************************************************************
*   FUNCTION :  	private_AudioInContinue
*   DESCRIPTION :   continuerecording a sound - occurs at audio in interrupt
*   INPUTS :		pwh: wave header to insert sound into
*   OUTPUTS :     	None
*   DESIGN NOTES :  
*   CAUTIONS :		pwh can be null for no buffers available
*****************************************************************************/
VOID
private_WaveInContinue(
   PWAVEHDR pwh
   )
{
	int		i;
	long double	SkipCount;
	int 		mic_data_read_count;
	int		wave_data_write_count;
	char		write_flag = 0;

	LONG samplingRate = 0;
	FUNC_VERBOSE("+PDD_WaveInContinue");

	WRITE_REGISTER_ULONG((PULONG)pCODEC_ACR, 0x03fffff);				//adjust L and R data
	pDmaInObject->ContinueDma();
	WRITE_REGISTER_ULONG((PULONG)pCODEC_ACR, (ULONG)0x00600000 );  	 // rec restart

	if (lpFormat2)
	{
		samplingRate = lpFormat2->nSamplesPerSec;
	}
	switch( (ULONG)samplingRate )
	{
	case 11025:
		SkipCount = 8.707482993L;
		break;
	case 22050:
		SkipCount = 4.353741497L;
		break;
	case 44100:
		SkipCount = 2.176870748L;
		break;
	default:
		SkipCount = 8.707482993L;
		break;
	}

	// process raw audio samples 
	// copy processed audio sample into WAVEHDR

	mic_data_read_count = 0;
	wave_data_write_count = 0;
	write_flag = 0;

	for( i=0; i< (int)pDmaInObject->GetAudioDmaPageSize()/*AUDIO_DMA_PAGE_REC_SIZE*/; i=i+4)
	{
		PPCM_SAMPLE pSample;	// where to store the audio record data
		INT32 raw_sample;		// 32 bit signed sample from AC97PCML


		__try
		{
			raw_sample = READ_REGISTER_ULONG(pAudioBufferBase + ((1 - v_recPage) * pDmaInObject->GetAudioDmaPageSize()/*AUDIO_DMA_PAGE_REC_SIZE*/) + i);
		} 
		__except ( 1 ) 
		{
			DUMPEXCEPTION();
		}

		mic_data_read_count = mic_data_read_count + 1;
		if ((( wave_data_write_count + 1 ) * SkipCount ) < mic_data_read_count )
			write_flag = 1;

		if (write_flag == 1 )
		{
			__try 
			{
				if( pwh == 0 ) break;

				if( pwh->dwBytesRecorded >= pwh->dwBufferLength )
				{
					DEBUGMSG(ZONE_TEST,(TEXT("WAVE RECORD: NEXT BUFFER\r\n")));
					pwh = pwh->lpNext;
					
					if( pwh == 0 )
					{
						DEBUGMSG(ZONE_TEST,(TEXT("WAVE RECORD: NO BUFFERS\r\n")));
						break;
					}
				}
			}
			__except ( 1 )
			{
				DUMPEXCEPTION();
			}
			__try 
			{
				// this is safe, since odd alignment should only occur for 8 bit sound
				// however, it is assumed that the WAVEHDR buffer is in multiples
				//  of the sample size (1,2, 2, or 4), otherwise the buffer
				//  could be overrun by a few bytes!
				pSample = (PPCM_SAMPLE) (pwh->lpData + pwh->dwBytesRecorded);
				
				switch( g_pcmtype[WAPI_IN] )
				{
				case PCM_TYPE_M8:	// 8 bit mono
					pSample->m8.sample = (raw_sample >> 12) + 128;
					pwh->dwBytesRecorded += 1;
					break;

				case PCM_TYPE_S8:	// 8 bit stereo
					pSample->s8.sample_left =
					pSample->s8.sample_right = (raw_sample >> 12) + 128;
					pwh->dwBytesRecorded += 2;
					break;

				case PCM_TYPE_M16:	// 16 bit mono
					pSample->m16.sample = raw_sample >> 4;
					pwh->dwBytesRecorded += 2;
					break;

				case PCM_TYPE_S16:	// 16 bit stereo
					pSample->s16.sample_left =
					pSample->s16.sample_right = raw_sample >> 4;
					pwh->dwBytesRecorded += 4;
					break;

				default: // this should absolutely never happen - we should catch this at WIDM_OPEN
					DEBUGMSG(ZONE_ERROR,(TEXT("WAVE RECORD: invalid pcm type %d\r\n"),
						g_pcmtype[WAPI_IN] ));
					break;
				} //end switch
				write_flag = 0;
				wave_data_write_count = wave_data_write_count + 1;
			}
			__except ( 1 ) 
			{
				DUMPEXCEPTION();
			} // end try
		} // end if
	} // end for

	v_recPage = 1 - v_recPage;
//	WriteAudioRecAddress(dma_pagePhysicalAddress[v_recPage]);//[macro]
/*	pDriverGlobals->aud.rec_address = dma_pagePhysicalAddress[v_recPage];
	pDriverGlobals->aud.inInt = (USHORT)NULL;
*/	pDmaInObject->SetGlobalAddr(v_recPage==0);
//	*pDmaInObject->GetGlobalInIntPointer()=NULL
	FUNC_VERBOSE("-PDD_WaveInContinue");
}

/*****************************************************************************
*   FUNCTION :  	private_AudioInStop
*   DESCRIPTION :   stop recording a sound
*   INPUTS :		
*   OUTPUTS :     	None
*   DESIGN NOTES :  
*   CAUTIONS :		
*****************************************************************************/
VOID 
private_WaveInStop()
{
	FUNC_WPDD("+PDD_WaveInStop");
	FUNC_WPDD("-PDD_WaveInStop");
}

/*****************************************************************************
*   FUNCTION :  	private_AudioInStandby
*   DESCRIPTION :   stop recording a sound
*   INPUTS :		
*   OUTPUTS :     	None
*   DESIGN NOTES :  what is the difference between this and Stop?
*   CAUTIONS :		
*****************************************************************************/
VOID 
private_WaveInStandby()
{
	FUNC_WPDD("+PDD_WaveInStandby");
	FUNC_WPDD("-PDD_WaveInStandby");
}

/*****************************************************************************
*   FUNCTION :  	private_AudioInClose
*   DESCRIPTION :   close recording device, powers off
*   INPUTS :		
*   OUTPUTS :     	None
*   DESIGN NOTES :  
*   CAUTIONS :		
*****************************************************************************/
VOID
private_WaveInClose()
{
	FUNC_WPDD("PDD_WaveInClose");
 	lpFormat2=0;

    g_fInUse = FALSE;

    /*	pDriverGlobals->aud.rec_address = (ULONG)NULL;
	pDriverGlobals->aud.inInt = (USHORT)NULL;
*/	pDmaInObject->SetGlobalRecAddress(NULL);
}

/*****************************************************************************
*   FUNCTION :  	private_AudioInOpen
*   DESCRIPTION :   opens recording sound device, powers on
*   INPUTS :		lpFormat = what recording parameters to use
*   OUTPUTS :     	MMRESULT - ok if params ok, else a error code
*   DESIGN NOTES :  
*   CAUTIONS :		
*****************************************************************************/
MMRESULT
private_WaveInOpen(
	LPWAVEFORMATEX lpFormat,
	BOOL fQueryFormatOnly
	)
{
	MMRESULT mmRet = MMSYSERR_NOERROR;
	FUNC_WPDD("+PDD_WaveInOpen");

	// Allow PCM, mono or stereo, 8 or 16 bit at 11, 22, or 44 KHz

	if ((lpFormat->wFormatTag != WAVE_FORMAT_PCM) || 
		(lpFormat->nChannels != Monaural && lpFormat->nChannels != Stereo) ||
		(lpFormat->nSamplesPerSec != Hz11025 && lpFormat->nSamplesPerSec != Hz22050 && lpFormat->nSamplesPerSec != Hz44100) ||
		(lpFormat->wBitsPerSample != SixteenBits && lpFormat->wBitsPerSample != EightBits))
	{
		mmRet = WAVERR_BADFORMAT;
	}

	if (fQueryFormatOnly || mmRet != MMSYSERR_NOERROR)
	{
		goto EXIT;
	}

    // check if input stream already allocated, fail
    if (g_fInUse) {
        mmRet = MMSYSERR_ALLOCATED;
        goto EXIT;
    }
    g_fInUse = TRUE;

	//Stop DMA
/*	READ_REGISTER_ULONG((PULONG)pCHCR1  );
	WRITE_REGISTER_ULONG((PULONG)pCHCR1, 
		SH3_DMAC_CHCR_RL_ACTIVE_HIGH |
		SH3_DMAC_CHCR_DM_INCREMENTED |
		SH3_DMAC_CHCR_TS_32 |
		SH3_DMAC_CHCR_IE_NOT_GENARATED |
		SH3_DMAC_CHCR_DE_DISABLED );
*/	pDmaInObject->StopDma();
	lpFormat2=lpFormat;		// for WaveInContinue


	// Open with the given format. Choose a playback rate from this.
	
	g_pwfx[WAPI_IN] = lpFormat;

	if (g_pwfx[WAPI_IN]->wBitsPerSample == EightBits)
	{
		if (g_pwfx[WAPI_IN]->nChannels == Monaural) 
			g_pcmtype[WAPI_IN] = PCM_TYPE_M8;
		else
			g_pcmtype[WAPI_IN] = PCM_TYPE_S8;
	}
	else
	{
		if (g_pwfx[WAPI_IN]->nChannels == Monaural)
			g_pcmtype[WAPI_IN] = PCM_TYPE_M16;
		else
			g_pcmtype[WAPI_IN] = PCM_TYPE_S16;
	}

	// Power on and initialize the AFE
	
	private_AudioInPowerHandler( FALSE, FALSE );

	// clear stats
	
	g_dwOverruns = 0;
	g_dwFrames = 0;

EXIT:
	FUNC_WPDD("-PDD_WaveInOpen");
	return(mmRet);
}

