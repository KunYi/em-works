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

		wavepdd.cpp

	Revision History:

		26th April 1999		Released

*/

//  Functions:
//      PDD_AudioGetInterruptType
//      PDD_AudioMessage
//      PDD_AudioInitialize
//      PDD_AudioDeinitialize
//      PDD_AudioPowerHandler
//      PDD_WaveProc
#include <windows.h>
#include <types.h>
#include <memory.h>
#include <excpt.h>
#include <waveddsi.h>
#include <wavedbg.h>
#include <mmsystem.h>

#include "wavepdd.h"
#include "waveOutpdd.h"
#include "waveInpdd.h"

#include "cc.h"
//#include "oalintr.h"

BOOL g_fInUse; // TRUE when in/out in use
PWAVEFORMATEX g_pwfx[2];
PCM_TYPE g_pcmtype[2];


/*****************************************************************************
*   FUNCTION :  	PDD_AudioGetInterruptType
*   DESCRIPTION :   decodes type of audio interrupt
*   INPUTS :		None
*   OUTPUTS :     	interrupt type
*   DESIGN NOTES :  
*   CAUTIONS :		returned value contains both audio-in and -out states
*****************************************************************************/
AUDIO_STATE 
PDD_AudioGetInterruptType(
   VOID
   )
{
	// An audio interrupt has occured. We need to tell the MDD who owns it
	// and what state that puts us in.
	//
	// Note: return value represents both an audio-in and an audio-out state
	//		 (AUDIO_STATE stores audio-in and audio-out in separate nybbles)

	return	(private_AudioInGetInterruptType()  & AUDIO_STATE_IN_MASK)  |
 			(private_AudioOutGetInterruptType() & AUDIO_STATE_OUT_MASK) ;
}

/*****************************************************************************
*   FUNCTION :  	PDD_AudioInitialize
*   DESCRIPTION :   intializes system for audio-in and audio-out
*   INPUTS :		None
*   OUTPUTS :     	TRUE
*   DESIGN NOTES :  tries to initialize the other, even if one fails
*   CAUTIONS :		
*****************************************************************************/
BOOL 
PDD_AudioInitialize(
   DWORD dwIndex
   )
{
	BOOL inSuccess, outSuccess;
//	BOOL  outSuccess;

	FUNC_WPDD("+PDD_AudioInitialize");

	inSuccess  = private_AudioInInitialize();
	outSuccess = private_AudioOutInitialize(false);

        g_fInUse = false;

	FUNC_WPDD("-PDD_AudioInitialize");

	return inSuccess && outSuccess;
}
/*****************************************************************************
*   FUNCTION :  	PDD_AudioPowerHandler
*   DESCRIPTION :   power-down/power-up (after power-down) for audio-in/out
*   INPUTS :		TRUE means power down, FALSE means power up
*   OUTPUTS :     	None
*   DESIGN NOTES :  
*   CAUTIONS :		
*****************************************************************************/
VOID
PDD_AudioPowerHandler(
	BOOL power_down
   )
{
	private_AudioOutPowerHandler( power_down );
}


/*****************************************************************************
*   FUNCTION :  	PDD_AudioMessage
*   DESCRIPTION :   Handle any custom WAVEIN or WAVEOUT messages here
*   INPUTS :		None
*   OUTPUTS :     	None
*   DESIGN NOTES :  
*   CAUTIONS :		
*****************************************************************************/
DWORD
PDD_AudioMessage(
	UINT uMsg,
	DWORD dwParam1,
	DWORD dwParam2
	)
{
	return(MMSYSERR_NOTSUPPORTED);
}


/*****************************************************************************
*   FUNCTION :  	PDD_AudioDeinitialize
*   DESCRIPTION :   Reset audio devices to original state
*   INPUTS :		None
*   OUTPUTS :     	None
*   DESIGN NOTES :  
*   CAUTIONS :		
*****************************************************************************/
VOID 
PDD_AudioDeinitialize(
	VOID
	)
{
	FUNC("+PDD_AudioDeinitialize");

	private_AudioInDeinitialize();
	private_AudioOutDeinitialize(false);

	FUNC("-PDD_AudioDeinitialize");
}

MMRESULT
private_WaveGetDevCaps(
	WAPI_INOUT apidir,
	PVOID pCaps,
	UINT  wSize
	)
{
	PWAVEOUTCAPS pwc = (PWAVEOUTCAPS)pCaps;

	MMRESULT mmRet = MMSYSERR_NOERROR;

	FUNC_WPDD("+PDD_WaveGetDevCaps");

	if (pCaps == NULL)  {
		return(MMSYSERR_NOERROR);
	}

	pwc->wMid = MM_MICROSOFT;
	pwc->wPid = (apidir == WAPI_OUT ? 24 : 23);  // generic in or out...
	pwc->vDriverVersion = 0x0001;
	wsprintf (pwc->szPname, TEXT("Audio (%hs)"), __DATE__);
	pwc->dwFormats =	WAVE_FORMAT_1M08 | WAVE_FORMAT_1M16 |
						WAVE_FORMAT_1S08 | WAVE_FORMAT_1S16 |
						WAVE_FORMAT_2M08 | WAVE_FORMAT_2M16 |
						WAVE_FORMAT_2S08 | WAVE_FORMAT_2S16 |
						WAVE_FORMAT_4M08 | WAVE_FORMAT_4M16 |
						WAVE_FORMAT_4S08 | WAVE_FORMAT_4S16;
	pwc->wChannels = 2;

	if (apidir == WAPI_OUT) 
	{
		pwc->dwSupport = WAVECAPS_VOLUME;
	}

	FUNC_WPDD("-PDD_WaveGetDevCaps");

	return(mmRet);
}    


MMRESULT
PDD_WaveProc(
	WAPI_INOUT apidir,
	DWORD      dwCode,
	DWORD      dwParam1,
	DWORD      dwParam2
	)
{
	MMRESULT mmRet = MMSYSERR_NOERROR;

	switch (dwCode) 
	{
	case WPDM_CLOSE:
		if (apidir == WAPI_IN)
			private_WaveInClose();
		else
			private_WaveOutClose();
		break;

	case WPDM_CONTINUE:
		if (apidir == WAPI_IN)
			private_WaveInContinue((PWAVEHDR) dwParam1);
		else 
			private_WaveOutContinue((PWAVEHDR) dwParam1);
		break;

	case WPDM_GETDEVCAPS:
		mmRet = private_WaveGetDevCaps(apidir, (PVOID) dwParam1, (UINT) dwParam2);
		break;

	case WPDM_OPEN:
		if (apidir == WAPI_IN)
			mmRet = private_WaveInOpen((LPWAVEFORMATEX) dwParam1, (BOOL) dwParam2);
		else 
			mmRet = private_WaveOutOpen((LPWAVEFORMATEX) dwParam1, (BOOL) dwParam2);
		break;

	case WPDM_STANDBY:
		if (apidir == WAPI_IN)
			private_WaveInStandby();
		else 
			private_WaveOutStandby();
		break;

	case WPDM_START:
		if (apidir == WAPI_IN)
			private_WaveInStart((PWAVEHDR) dwParam1);
		else 
			private_WaveOutStart((PWAVEHDR) dwParam1);
		break;

	case WPDM_STOP:
		if (apidir == WAPI_IN)
			private_WaveInStop();
		else 
			private_WaveOutStop();
		break;

	case WPDM_PAUSE:
		if (apidir == WAPI_OUT)
			private_WaveOutPause();
		else 
			mmRet = MMSYSERR_NOTSUPPORTED;
		break;

	case WPDM_ENDOFDATA:
		if (apidir == WAPI_OUT)
			private_WaveOutEndOfData();
		else 
			mmRet = MMSYSERR_NOTSUPPORTED;
		break;

	case WPDM_RESTART:
		if (apidir == WAPI_OUT)
			private_WaveOutRestart((PWAVEHDR) dwParam1);
		else 
			mmRet = MMSYSERR_NOTSUPPORTED;
		break;

	case WPDM_GETVOLUME:
		*((PULONG) dwParam1) = private_AudioGetVolume();
		break;

	case WPDM_SETVOLUME:
		private_AudioSetVolume( (ULONG) dwParam1 );
		break;

	default :
		mmRet = MMSYSERR_NOTSUPPORTED;
		break;
	}
	return(mmRet);
}

// -----------------------------------------------------------------------------
//  Debugging aid...
// -----------------------------------------------------------------------------
void PrintBuffer(short* pbuf)
{
	PRINTMSG(1, (TEXT("\r\n-------------Print Buffer @0x%08X\r\n"), pbuf));
}   

/*****************************************************************************
*   FUNCTION :  	FreeAllocatedVirtualMemory
*   DESCRIPTION :   Returns allocated virtual memory to operating system
*   INPUTS :		Virtual address returned by VirtualAlloc
*					Size of region to reserved by VirtualAlloc
*   OUTPUTS :     	None
*   DESIGN NOTES :  
*   CAUTIONS :
*****************************************************************************/
VOID
FreeAllocatedVirtualMemory(
	PBYTE virtualAddress
	)
{
	if( virtualAddress )
		VirtualFree((PVOID)virtualAddress, 0, MEM_RELEASE);
}


/*****************************************************************************
*   FUNCTION :  	GetVirtualAddressOfUncachedMemory
*   DESCRIPTION :   Associates virtual addr with physical memory region
*   INPUTS :		Physical adddress of memory region (page aligned)
*					Size of region to be reserved
*					String identifying point of invocation (for error msgs)
*   OUTPUTS :     	Virtual address associated (or NULL, if failure occurs)
*   DESIGN NOTES :  
*   CAUTIONS :
*****************************************************************************/
extern "C"  PBYTE GetVirtualAddressOfUncachedMemory(
	PBYTE physicalAddress,
	DWORD size,
	char* callerID)
{
	PBYTE virtualAddress;

#define here "GetVirtualAddressOfUncachedMemory"

	FUNC(here);
	if ((ULONG)physicalAddress % PAGE_SIZE) 
	{
		ERRMSG2((TEXT(here "%s: Physical Addr (0x%x) Not Page-Aligned\r\n")), callerID, physicalAddress);
	}
	
	virtualAddress = (PBYTE)VirtualAlloc(
		NULL,
		size,
		MEM_RESERVE,
		PAGE_NOACCESS);
	
	if (virtualAddress == NULL) 
	{
		ERRMSG1((TEXT(here "%s: Virtual Alloc Failed\r\n")), callerID);
		return NULL;
	}
	
	if (!VirtualCopy((PVOID) virtualAddress,
		(PVOID) physicalAddress,
		size,
		PAGE_READWRITE | PAGE_NOCACHE))
	{
		ERRMSG1((TEXT(here "%s: Virtual Copy Failed\r\n")), callerID);
		FreeAllocatedVirtualMemory( virtualAddress );
		return NULL;
	}
	return virtualAddress;
}
