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

		waveInpdd.h

	Revision History:

		26th April 1999		Released

*/

// Function Prototypes

BOOL private_AudioInInitialize(VOID);
VOID private_AudioInDeinitialize(VOID);
AUDIO_STATE private_AudioInGetInterruptType(VOID);
VOID private_AudioInPowerHandler( BOOL bPowerDown, BOOL bInKernel );
VOID private_WaveInStart( PWAVEHDR pwh );
VOID private_WaveInContinue( PWAVEHDR pwh );
VOID private_WaveInStop();
VOID private_WaveInStandby ();
MMRESULT private_WaveGetDevCaps(WAPI_INOUT apidir, PVOID pCaps, UINT wSize);
VOID private_WaveInClose ();
MMRESULT private_WaveInOpen (LPWAVEFORMATEX lpFormat,BOOL fQueryFormatOnly);
VOID Delay_AC( VOID );
