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

		waveOutpdd.h

	Revision History:

		26th April     1999		Released
		21th September 1999		Added ClearFifoBuffer process

*/

// Function Prototypes

BOOL		private_AudioOutInitialize(BOOL bPowerOn);
VOID		private_AudioOutDeinitialize(BOOL bPowerOff);
MMRESULT	private_WaveOutOpen(LPWAVEFORMATEX lpFormat,BOOL fQueryFormatOnly);
AUDIO_STATE	private_AudioOutGetInterruptType(VOID);
VOID		private_WaveOutStart(PWAVEHDR pwh);
MMRESULT	private_WaveOutRestart ( PWAVEHDR pwh );
MMRESULT	private_WaveOutPause ( VOID );
VOID		private_WaveOutStop();
VOID		private_WaveOutStandby();
VOID		private_WaveOutClose();
VOID		private_WaveOutEndOfData();
ULONG		private_AudioGetVolume(VOID);
VOID		private_AudioSetVolume(ULONG);
VOID		private_AudioOutPowerHandler( BOOL bPowerDown );
VOID		private_WaveOutContinue ( PWAVEHDR pwh );
VOID		private_AudioPlayFirstBuffer( VOID );
VOID		Delay_AC( VOID );
VOID		ClearFifoBuffer( VOID );

extern VOID		(*private_AudioFillBuffer) ( PWAVEHDR pwh, PBYTE pDstBuffer, DWORD cbDstBytes );
VOID		private_AudioFillBuffer_Fast ( PWAVEHDR pwh, PBYTE pDstBuffer, DWORD cbDstBytes );
VOID		private_AudioFillBuffer_Generic ( PWAVEHDR pwh, PBYTE pDstBuffer, DWORD cbDstBytes );
VOID		private_AudioComplete(PWAVEHDR pwh, DWORD cbBytesCompleted);
