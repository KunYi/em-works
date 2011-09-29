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
//  File:  omap5912_mmcsd.h
//
#ifndef __OMAP5912_MMCSD_H
#define __OMAP5912_MMCSD_H

typedef volatile struct _OMAP5912_MMCSD_REGS
{
   UINT16 MMC_CMD;
       UINT16 DUMMY01;
   UINT16 MMC_ARG1;
	   UINT16 DUMMY02;
   UINT16 MMC_ARG2;
	   UINT16 DUMMY03;
   UINT16 MMC_CON;
	   UINT16 DUMMY04;
   UINT16 MMC_STAT;
	   UINT16 DUMMY05;
   UINT16 MMC_IE;
	   UINT16 DUMMY06;
   UINT16 MMC_CTO;
	   UINT16 DUMMY07;
   UINT16 MMC_DTO;
	   UINT16 DUMMY08;
   UINT16 MMC_DATA;
	   UINT16 DUMMY09;
   UINT16 MMC_BLEN;
	   UINT16 DUMMY10;
   UINT16 MMC_NBLK;
	   UINT16 DUMMY11;
   UINT16 MMC_BUF;
	   UINT16 DUMMY12;
   UINT16 MMC_SPI;
	   UINT16 DUMMY13;
   UINT16 MMC_SDIO;
	   UINT16 DUMMY14;
   UINT16 MMC_SYSTEST;
	   UINT16 DUMMY15;
   UINT16 MMC_REV;
	   UINT16 DUMMY16;
   UINT16 MMC_RSP0;
       UINT16 DUMMY17;
   UINT16 MMC_RSP1;
	   UINT16 DUMMY18;
   UINT16 MMC_RSP2;
	   UINT16 DUMMY19;
   UINT16 MMC_RSP3;
	   UINT16 DUMMY20;
   UINT16 MMC_RSP4;
	   UINT16 DUMMY21;
   UINT16 MMC_RSP5;
	   UINT16 DUMMY22;
   UINT16 MMC_RSP6;
	   UINT16 DUMMY23;
   UINT16 MMC_RSP7;
	   UINT16 DUMMY24;
   UINT16 MMC_IOSR;
	   UINT16 DUMMY25;
   UINT16 MMC_SYSC;
	   UINT16 DUMMY26;
   UINT16 MMC_SYSS;
	   UINT16 DUMMY27;
} OMAP5912_MMCSD_REGS;

/////
#define STD_HC_SLOTS				1
#define STD_HC_MAX_SLOTS                2

#define STD_HC_MAX_CLOCK_FREQUENCY  24000000    // 24 MHz

#define STD_HC_MIN_BLOCK_LENGTH         1
#define STD_HC_MAX_BLOCK_LENGTH      2048

#define MMCSD_CLOCK_INPUT		    (48 * 1000 * 1000)
#define MMCSD_CLOCK_INIT			(400 * 1000)

#define SDMMC_DEFAULT_BLOCK_LEN     512
#define SDMMC_DEFAULT_NUM_BLOCKS      1

#define SDMMC_DEFAULT_ALMOST_EMPTY   32
#define SDMMC_DEFAULT_ALMOST_FULL    32

#define MMC_CTO_CONTROL_MAX         0x00FD
#define MMC_CTO_CONTROL_DEFAULT     0x0005      // 5 * 1024 = 5K cycles
#define MMC_DTO_CONTROL_MAX         0xFFFD
#define MMC_DTO_CONTROL_DEFAULT     0x0200      // 512 * 1024 = 512K cycles

#define OMAP5912_OCR_SUPPORTED_VOLTAGES_MMC		0x00FFC000
#define OMAP5912_OCR_SUPPORTED_VOLTAGES_SD		0x00FF8000


// The control register
#define MASK_MMC_CON_DW				(BIT15)
#define MASK_MMC_CON_MODE			(BIT13 | BIT12)
#define MASK_MMC_CON_POWERUP		(BIT11)
#define MASK_MMC_CON_BE				(BIT10)
#define MASK_MMC_CON_CLKD			(BIT9 | BIT8 | BIT7 | BIT6 | BIT5 | BIT4 | BIT3 | BIT2 | BIT1 | BIT0)

#define SHIFT_MMC_CON_DW			(15)
#define SHIFT_MMC_CON_MODE			(12)
#define SHIFT_MMC_CON_POWERUP		(11)
#define SHIFT_MMC_CON_BE			(10)
#define SHIFT_MMC_CON_CLKD			(0)

#define VALUE_MMC_CON_DW_1BIT			(0 << SHIFT_MMC_CON_DW)
#define VALUE_MMC_CON_DW_4BIT			(1 << SHIFT_MMC_CON_DW)
#define VALUE_MMC_CON_MODE_MMCSD		(0 << SHIFT_MMC_CON_MODE)
#define VALUE_MMC_CON_MODE_SPI			(1 << SHIFT_MMC_CON_MODE)
#define VALUE_MMC_CON_MODE_SYSTEST		(2 << SHIFT_MMC_CON_MODE)
#define VALUE_MMC_CON_MODE_RESV			(3 << SHIFT_MMC_CON_MODE)
#define VALUE_MMC_CON_POWERUP_DOWN		(0 << SHIFT_MMC_CON_POWERUP)
#define VALUE_MMC_CON_POWERUP_UP		(1 << SHIFT_MMC_CON_POWERUP)
#define VALUE_MMC_CON_BE_LITTLEENDIAN	(0 << SHIFT_MMC_CON_BE)
#define VALUE_MMC_CON_BE_BIGENDIAN		(1 << SHIFT_MMC_CON_BE)

#define MMC_CON_DW			0x8000
#define MMC_CON_POWER_UP	0x0800


// The status register
#define MMC_STAT_CERR  		0x4000
#define MMC_STAT_CIRQ		0x2000
#define MMC_STAT_OCRB		0x1000
#define MMC_STAT_AE		0x0800
#define MMC_STAT_AF		0x0400
#define MMC_STAT_CRW		0x0200
#define MMC_STAT_CCRC		0x0100
#define MMC_STAT_CTO		0x0080
#define MMC_STAT_DCRC		0x0040
#define MMC_STAT_DTO		0x0020
#define MMC_STAT_EOFB		0x0010
#define MMC_STAT_BRS		0x0008
#define MMC_STAT_CB		0x0004
#define MMC_STAT_CD		0x0002
#define MMC_STAT_EOC		0x0001
#define MMC_STAT_ALLBIT 0x7fff

// The interrupt enable register
#define MMC_IE_EOC      0x0001
#define MMC_IE_CD       0x0002
#define MMC_IE_CB       0x0004
#define MMC_IE_BRS      0x0008
#define MMC_IE_EOFB     0x0010
#define MMC_IE_DTO      0x0020
#define MMC_IE_DCRC     0x0040
#define MMC_IE_CTO      0x0080
#define MMC_IE_CCRC     0x0100
#define MMC_IE_CRW      0x0200
#define MMC_IE_AF       0x0400
#define MMC_IE_AE       0x0800
#define MMC_IE_OCRB     0x1000
#define MMC_IE_CIRQ     0x2000
#define MMC_IE_CERR     0x4000


// BUF config register
#define MMC_BUF_RXDE    0x8000
#define MMC_BUF_TXDE    0x0080


// SDIO config register
#define MASK_MMC_SDIO_CDE		(BIT2)
#define SHIFT_MMC_SDIO_CDE	(2)
#define VALUE_MMC_SDIO_CDE_ENABLE	(1 << SHIFT_MMC_SDIO_CDE)

#define MMC_SDIO_DPE		0x0020


// Soft Reset Register 
#define MMC_SYSC_SRST		0x0002
#define MMC_SYSS_RSTD		0x0001


// DMA
#define SDIO_DMA_OUTPUT_CHANNEL (OMAP5912_DMA_LCH_MMC_SDIO_TX)
#define SDIO_DMA_INPUT_CHANNEL  (OMAP5912_DMA_LCH_MMC_SDIO_RX)

// DMA input channel. 
#define   SDIO_INPUT_DMA_SOURCE            (OMAP5912_MMCSD_REGS_PA+0x10)    
#define   SDIO_INPUT_DMA_REQUEST           OMAP5912_REQ_SDMMC_RX
#define   SDIO_INPUT_PORT_TYPE             DMA_PORT_TYPE_TIPB

// DMA output channel. 
#define   SDIO_OUTPUT_DMA_REQUEST          OMAP5912_REQ_SDMMC_TX
#define   SDIO_OUTPUT_DMA_DEST             (OMAP5912_MMCSD_REGS_PA+0x10)
#define   SDIO_OUTPUT_PORT_TYPE            DMA_PORT_TYPE_TIPB

#endif
