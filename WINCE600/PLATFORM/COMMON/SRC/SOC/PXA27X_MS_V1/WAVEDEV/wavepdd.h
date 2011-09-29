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


// -----------------------------------------------------------------------------
//
// @doc     EX_AUDIO_DDSI
//
// @enum    PCM_TYPE | Enumeration of standard PCM data types.
//
// @field   PBYTE | pData | Pointer to just the data buffer which contains
//          the waveform data to be played.
//
// @field   ULONG | nBufferSize |
//          Size, in number of bytes, of the buffer to play.
//
// @field   ULONG | nBufferPosition |
//          Current position, in number of bytes in the user buffer.
//
// @field   BOOL | fLooping |
//          If TRUE, the buffer should be played in an infinite loop.
//
// -----------------------------------------------------------------------------
typedef enum {
     PCM_TYPE_M8,
     PCM_TYPE_M16,
     PCM_TYPE_S8,
     PCM_TYPE_S16
} PCM_TYPE, *PPCM_TYPE;


// -----------------------------------------------------------------------------
//
// @doc     EX_AUDIO_DDSI
//
// @struct  SAMPLE_8_MONO | Single sample from 8-bit mono PCM data stream.
//
// @field   UINT8 | sample | Unsigned 8-bit sample
//
// -----------------------------------------------------------------------------
typedef struct  {
    UINT8 sample;              // Unsigned 8-bit sample
} SAMPLE_8_MONO;


// -----------------------------------------------------------------------------
//
// @doc     EX_AUDIO_DDSI
//
// @struct  SAMPLE_16_MONO | Single sample from 16-bit mono PCM data stream.
//
// @field   INT16 | sample | Unsigned 16-bit sample
//
// -----------------------------------------------------------------------------
typedef struct  {
    INT16 sample;              // Signed 16-bit sample
} SAMPLE_16_MONO;


// -----------------------------------------------------------------------------
//
// @doc     EX_AUDIO_DDSI
//
// @struct  SAMPLE_8_STEREO | Single sample from 8-bit stereo PCM data stream.
//
// @field   UINT8 | sample_left | Unsigned 8-bit sample from left channel.
//
// @field   UINT8 | sample_right | Unsigned 8-bit sample from right channel.
//
// -----------------------------------------------------------------------------
typedef struct  {
    UINT8 sample_left;         // Unsigned 8-bit sample
    UINT8 sample_right;        // Unsigned 8-bit sample
} SAMPLE_8_STEREO;


// -----------------------------------------------------------------------------
//
// @doc     EX_AUDIO_DDSI
//
// @struct  SAMPLE_16_STEREO | Single sample from 16-bit stereo PCM data stream.
//
// @field   UINT16 | sample_left | Unsigned 16-bit sample from left channel.
//
// @field   UINT16 | sample_right | Unsigned 16-bit sample from right channel.
//
// -----------------------------------------------------------------------------
typedef struct  {
    INT16 sample_left;         // Signed 16-bit sample
    INT16 sample_right;        // Signed 16-bit sample
} SAMPLE_16_STEREO;



// -----------------------------------------------------------------------------
//
// @doc     EX_AUDIO_DDSI
//
// @struct  PCM_SAMPLE | Union that allows access to any of the Sample Types
//
// @field   SAMPLE_8_MONO | m8 | <t SAMPLE_8_MONO>
// @field   SAMPLE_16_MONO | m16 | <t SAMPLE_16_MONO>
// @field   SAMPLE_8_STEREO | s8 | <t SAMPLE_8_STEREO>
// @field   SAMPLE_16_STEREO | s16 | <t SAMPLE_16_STEREO>
//
// -----------------------------------------------------------------------------
typedef union {

     SAMPLE_8_MONO m8;
     SAMPLE_16_MONO m16;
     SAMPLE_8_STEREO s8;
     SAMPLE_16_STEREO s16;

} PCM_SAMPLE, *PPCM_SAMPLE;


// Size of the DMA page
#define SAMPLE_MASK                             0xFFF0

#define AUDIO_BUFFER_SIZE  0x1000

/*#define WPDM_PRIVATE 0
#define WPDM_PRIVATE_MUTE_EXTL3 WPDM_PRIVATE+0 // A MUTE or extended l3 write
#define WPDM_PRIVATE_MIXER WPDM_PRIVATE+1
#define WPDM_PRIVATE_DIAG WPDM_PRIVATE+2
//#define WPDM_PRIVATE_EQ WPDM_PRIVATE+3 //TODO: why does this value get booted by MDD?
*/

#define WPDM_PRIVATE WM_USER+10     
#define WPDM_PRIVATE_WRITE_AC97     WPDM_PRIVATE+0 // do a write to the ac 97 register
#define WPDM_PRIVATE_READ_AC97      WPDM_PRIVATE+1 // do a read to the ac 97 register
#define WPDM_PRIVATE_DIAG_MSG       WPDM_PRIVATE+2 // TBD
#define WPDM_PRIVATE_RET_ERROR      0xFFFFFFFF
#define WPDM_PRIVATE_RET_SUCCESS    0x0
#define WPDM_PRIVATE_RET_NOTSUPPORTED    MMSYSERR_NOTSUPPORTED

// DCSRn Register Settings (see sa-1110 manual for DCSRn details)
#define DMA_RUN       1
#define DMA_IE        2 // interrupt enable flag
#define DMA_ERR       4
#define DMA_DONEA     8
#define DMA_STARTA 0x10
#define DMA_DONEB  0x20
#define DMA_STARTB 0x40
#define DMA_BIU    0x80

//CODEC RESET BEHAVIOR
#define RESET_GOTORESET 0 //force into reset mode
#define RESET_LEAVERESET 1 //normal operation
#define RESET_CYCLE 2

// Function Prototypes
//

BOOL AudioMute(BOOL);
BOOL AudioEnable(BOOL);
BOOL AudioWaitForRegIntr(VOID);
BOOL AudioInitializeComplete();


