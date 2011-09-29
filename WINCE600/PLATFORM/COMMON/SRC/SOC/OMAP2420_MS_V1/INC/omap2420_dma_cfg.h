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
//  File:  omap2420_dma_cfg.h
//
//  This file assigns DMA channels to devices on the chip
//
#ifndef __OMAP2420_DMA_CFG_H
#define __OMAP2420_DMA_CFG_H

//------------------------------------------------------------------------------
//
// DMA Logical Channel assignment
//

#define OMAP2420_DMA_LCH_UNUSED_0        0
#define OMAP2420_DMA_LCH_UNUSED_1        1
#define OMAP2420_DMA_LCH_AUDIO_TX        2
#define OMAP2420_DMA_LCH_AUDIO_RX        3
#define OMAP2420_DMA_LCH_USBC_TX         4
#define OMAP2420_DMA_LCH_USBC_RX         5
#define OMAP2420_DMA_LCH_PRI_2D          6
#define OMAP2420_DMA_LCH_SERIAL_TX       7
#define OMAP2420_DMA_LCH_SERIAL_RX       8
#define OMAP2420_DMA_LCH_MMC_SDIO_TX     9
#define OMAP2420_DMA_LCH_MMC_SDIO_RX     10
#define OMAP2420_DMA_LCH_UNUSED_11       11
#define OMAP2420_DMA_LCH_UNUSED_12       12
#define OMAP2420_DMA_LCH_UNUSED_13       13
#define OMAP2420_DMA_LCH_UNUSED_14       14
#define OMAP2420_DMA_LCH_UNUSED_15       15

/* camera dma */
#define OMAP2420_CAMDMA_LCH_INPUT        0

#endif //_OMAP2420_DMA_CFG_H

