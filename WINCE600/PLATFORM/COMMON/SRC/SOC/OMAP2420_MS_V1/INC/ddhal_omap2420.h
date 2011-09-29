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
//------------------------------------------------------------------------------
//
//  Header:  ddhal_omap2420.h
//
//  This header defines KernelIO extensions for Omap2420 based display drivers
//

#ifndef __DDHAL_OMAP2420_H
#define __DDHAL_OMAP2420_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  Define:  DDHAL_xxx
//
//  Define function required by omap2420 display driver to be provided by HAL.
//

//  just get size for flat memory allocation
#define DDHAL_OMAP2420_FLIP_SURF_SIZE          10

//  In:  modeId
//  Out: Size of memory needed + extra memory ( for e.g. palette) 

//  prepare surface to be used as flipping surface
#define DDHAL_OMAP2420_FLIP_SURF_PREPARE       11

//------------------------------------------------------------------------------
//
//  Define:  DDHAL_xxx
//
//  This values define flags passed in mode info
//
//  extra bits defined for OMAP2420 display driver
// 

//  heap of primary surface can flip    
#define DDHAL_OMAP2420_FLAG_CAN_FLIP     (1 << 16)
//  program B1 on flip
#define DDHAL_OMAP2420_FLAG_SETUP_B1     (1 << 17)
//  program B2 on flip
#define DDHAL_OMAP2420_FLAG_SETUP_B2     (1 << 18)
//  to flip on primary heap set base address using B1 offset
#define DDHAL_OMAP2420_FLAG_FLIP_B1      (1 << 19)
//  to flip on primary heap set base address using B2 offset
#define DDHAL_OMAP2420_FLAG_FLIP_B2      (1 << 20)
//  surface based in SRAM
#define DDHAL_OMAP2420_FLAG_SRAM         (1 << 21)
//  video memory allocation policy: reserve space for 2nd flip buffer
#define DDHAL_OMAP2420_FLAG_VIDMEM_REQUIRE_ONLY (1<<22)

//------------------------------------------------------------------------------
//
//  DDHAL_OMAP2420_DMA_LCD_INFO
//
//  In:  NULL
//  Out:


//------------------------------------------------------------------------------
//
//  Type: DDHAL_OMAP2420_DMA_LCD
//
typedef struct 
{
    ULONG  command    ;     // must contain DDHAL_OMAP2420_FLIP_SURF_PREPARE

    UINT32 modeId     ;     // reference flip surface for mode X

    UINT32 virtBase   ;     // virtual base address

    UINT32 physBase   ;     // physical base

    UINT32 size       ;     // memory size    

    UINT32 flags      ;     // instructions for flip code (see flags)

    UINT32 offset     ;     // offset of frame buffer in memory

    // LCD DMA channel control
    UINT16 CTRL       ;
    UINT16 CCR        ;
    UINT16 CSDP       ;

    // LCD DMA B1 
    UINT16 SRC_EN_B1  ;     // DMA setup to flip surface
    UINT16 SRC_FN_B1  ;
    UINT16 SRC_EI_B1  ;
    UINT16 SRC_FI_B1_L;
    UINT16 SRC_FI_B1_U;
    UINT16 TOP_B1_L   ;
    UINT16 TOP_B1_U   ;

    // LCD DMA B2 
    UINT16 SRC_EN_B2  ;
    UINT16 SRC_FN_B2  ;
    UINT16 SRC_EI_B2  ;
    UINT16 SRC_FI_B2_L;
    UINT16 SRC_FI_B2_U;
    UINT16 TOP_B2_L   ;
    UINT16 TOP_B2_U   ;

} DDHAL_OMAP2420_DMA_LCD;


//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif
