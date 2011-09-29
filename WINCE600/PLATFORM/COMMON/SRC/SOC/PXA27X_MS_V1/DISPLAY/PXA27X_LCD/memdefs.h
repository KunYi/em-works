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
//
//  File:       memdefs.h
//
//  Contents:   Bulverde display driver memory address definitions file.
//
//  ** NOTE: This file is only temporary - display buffer allocations (DMA and
//     otherwise) should be dynamically allocated during initialization. **
//
// ----------------------------------------------------------------------------
#ifndef __MEMDEFS_H__
#define __MEMDEFS_H__

#ifdef __cplusplus
extern "C" {
#endif

#define DMA_DESC_SIZE                    0x20
#define PALETTE_BUFFER_SIZE              0x400
#define FRAME_BUFFER_SIZE                0x96000
#define DISPLAY_BUFFER_SIZE              0x2b0000

extern DWORD g_DisplayBasePhysical;
extern DWORD g_DisplayBaseVirtual;

#define RESERVED_DISPLAY_BASE_PHYSICAL   g_DisplayBasePhysical
#define RESERVED_DISPLAY_BASE_VIRTUAL    g_DisplayBaseVirtual

#define DMA_CHANNEL_0_FRAME_DESCRIPTOR_BASE_PHYSICAL      (RESERVED_DISPLAY_BASE_PHYSICAL + 0)
#define DMA_CHANNEL_0_FRAME_DESCRIPTOR_BASE_VIRTUAL       (RESERVED_DISPLAY_BASE_VIRTUAL + 0)

#define DMA_CHANNEL_1_FRAME_DESCRIPTOR_BASE_PHYSICAL      (DMA_CHANNEL_0_FRAME_DESCRIPTOR_BASE_PHYSICAL + DMA_DESC_SIZE)
#define DMA_CHANNEL_1_FRAME_DESCRIPTOR_BASE_VIRTUAL       (DMA_CHANNEL_0_FRAME_DESCRIPTOR_BASE_VIRTUAL + DMA_DESC_SIZE)

#define DMA_CHANNEL_0_ALT_FRAME_DESCRIPTOR_BASE_PHYSICAL  (DMA_CHANNEL_1_FRAME_DESCRIPTOR_BASE_PHYSICAL + DMA_DESC_SIZE)
#define DMA_CHANNEL_0_ALT_FRAME_DESCRIPTOR_BASE_VIRTUAL   (DMA_CHANNEL_1_FRAME_DESCRIPTOR_BASE_VIRTUAL + DMA_DESC_SIZE)

#define PALETTE_FRAME_DESCRIPTOR_BASE_PHYSICAL            (DMA_CHANNEL_0_ALT_FRAME_DESCRIPTOR_BASE_PHYSICAL + DMA_DESC_SIZE)
#define PALETTE_FRAME_DESCRIPTOR_BASE_VIRTUAL             (DMA_CHANNEL_0_ALT_FRAME_DESCRIPTOR_BASE_VIRTUAL + DMA_DESC_SIZE)

#define PALETTE_BUFFER_BASE_PHYSICAL                      (PALETTE_FRAME_DESCRIPTOR_BASE_PHYSICAL + DMA_DESC_SIZE)
#define PALETTE_BUFFER_BASE_VIRTUAL                       (PALETTE_FRAME_DESCRIPTOR_BASE_VIRTUAL + DMA_DESC_SIZE)

#define FRAME_BUFFER_0_BASE_PHYSICAL                      (PALETTE_BUFFER_BASE_PHYSICAL + PALETTE_BUFFER_SIZE)
#define FRAME_BUFFER_0_BASE_VIRTUAL                       (PALETTE_BUFFER_BASE_VIRTUAL + PALETTE_BUFFER_SIZE)

#define FRAME_BUFFER_1_BASE_PHYSICAL                      (FRAME_BUFFER_0_BASE_PHYSICAL + FRAME_BUFFER_SIZE)
#define FRAME_BUFFER_1_BASE_VIRTUAL                       (FRAME_BUFFER_0_BASE_VIRTUAL + FRAME_BUFFER_SIZE)


#define DMA_CHANNEL_2_Y_FRAME_DESCRIPTOR_BASE_PHYSICAL    (FRAME_BUFFER_1_BASE_PHYSICAL + FRAME_BUFFER_SIZE)
#define DMA_CHANNEL_2_Y_FRAME_DESCRIPTOR_BASE_VIRTUAL     (FRAME_BUFFER_1_BASE_VIRTUAL + FRAME_BUFFER_SIZE)

#define DMA_CHANNEL_3_Cb_FRAME_DESCRIPTOR_BASE_PHYSICAL   (DMA_CHANNEL_2_Y_FRAME_DESCRIPTOR_BASE_PHYSICAL + DMA_DESC_SIZE)
#define DMA_CHANNEL_3_Cb_FRAME_DESCRIPTOR_BASE_VIRTUAL    (DMA_CHANNEL_2_Y_FRAME_DESCRIPTOR_BASE_VIRTUAL + DMA_DESC_SIZE)

#define DMA_CHANNEL_4_Cr_FRAME_DESCRIPTOR_BASE_PHYSICAL   (DMA_CHANNEL_3_Cb_FRAME_DESCRIPTOR_BASE_PHYSICAL + DMA_DESC_SIZE)
#define DMA_CHANNEL_4_Cr_FRAME_DESCRIPTOR_BASE_VIRTUAL    (DMA_CHANNEL_3_Cb_FRAME_DESCRIPTOR_BASE_VIRTUAL + DMA_DESC_SIZE)


#define OVERLAY2_PHYSICAL_BASE_ADDRESS                     (DMA_CHANNEL_4_Cr_FRAME_DESCRIPTOR_BASE_PHYSICAL + DMA_DESC_SIZE)
#define OVERLAY2_VIRTUAL_BASE_ADDRESS                      (DMA_CHANNEL_4_Cr_FRAME_DESCRIPTOR_BASE_VIRTUAL + DMA_DESC_SIZE)


//DISPLAY_INACTIVITY_TO_STR


#ifdef __cplusplus
}
#endif

#endif	// __MEMDEFS_H__.
