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

		wavepdd.h

	Revision History:

		26th April 1999		Released

*/

typedef enum {
	PCM_TYPE_M8,
	PCM_TYPE_M16,
	PCM_TYPE_S8,
	PCM_TYPE_S16
} PCM_TYPE, *PPCM_TYPE;

typedef struct {
	UINT8 sample;              // Unsigned 8-bit sample
} SAMPLE_8_MONO;

typedef struct {
	INT16 sample;              // Signed 16-bit sample
} SAMPLE_16_MONO;

typedef struct {
	UINT8 sample_left;         // Unsigned 8-bit sample
	UINT8 sample_right;        // Unsigned 8-bit sample
} SAMPLE_8_STEREO;

typedef struct {
	INT16 sample_left;         // Signed 16-bit sample
	INT16 sample_right;        // Signed 16-bit sample
} SAMPLE_16_STEREO;

typedef union {
	SAMPLE_8_MONO m8;
	SAMPLE_16_MONO m16;
	SAMPLE_8_STEREO s8;
	SAMPLE_16_STEREO s16;
} PCM_SAMPLE, *PPCM_SAMPLE;

#ifdef PAGE_SIZE
#undef PAGE_SIZE
#endif
#define PAGE_SIZE 0x1000

#define	Monaural			1
#define Stereo				2
#define SixteenBits			16
#define EightBits			8
#define MinPlaybackSpeed	5000
#define MaxPlaybackSpeed	50000
#define Hz44100				44100
#define Hz22050				22050
#define Hz11025				11025

// Function Prototypes

MMRESULT private_WaveGetDevCaps(WAPI_INOUT apidir, PVOID pCaps, UINT wSize);

void PrintBuffer(short* pbuf);
VOID FreeAllocatedVirtualMemory( PUCHAR pVirtualAddr );
//PUCHAR GetVirtualAddressOfUncachedMemory( PVOID base, ULONG sz, char* cID );
