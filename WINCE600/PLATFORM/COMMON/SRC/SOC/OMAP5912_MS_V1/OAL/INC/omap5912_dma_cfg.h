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
//  File:  omap5912_dma_cfg.h
//
//  This file assigns DMA channels to devices on the chip
//
#ifndef __OMAP5912_DMA_CFG_H
#define __OMAP5912_DMA_CFG_H

//------------------------------------------------------------------------------
//
// DMA Logical Channel assignment
//

#define OMAP5912_DMA_LCH_CAMERA_RX       0
#define OMAP5912_DMA_LCH_UNUSED_1        1
#define OMAP5912_DMA_LCH_AUDIO_TX        2
#define OMAP5912_DMA_LCH_AUDIO_RX        3
#define OMAP5912_DMA_LCH_USBC_TX         4
#define OMAP5912_DMA_LCH_USBC_RX         5
#define OMAP5912_DMA_LCH_PRI_2D          6
#define OMAP5912_DMA_LCH_SERIAL_TX       7
#define OMAP5912_DMA_LCH_SERIAL_RX       8
#define OMAP5912_DMA_LCH_MMC_SDIO_TX     9
#define OMAP5912_DMA_LCH_MMC_SDIO_RX     10
#define OMAP5912_DMA_LCH_NAND_TX         11
#define OMAP5912_DMA_LCH_NAND_RX         12
#define OMAP5912_DMA_LCH_UNUSED_13       13
#define OMAP5912_DMA_LCH_UNUSED_14       14
#define OMAP5912_DMA_LCH_UNUSED_15       15
#define OMAP5912_DMA_LCH_MAX             16

#endif //_OMAP5912_DMA_CFG_H

