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
// Portions Copyright (c) Texas Instruments.  All rights reserved. 
//
//------------------------------------------------------------------------------
//
//  File:  omap2420.h
//
//  This header file is comprised of component header files that define 
//  the register layout of each System on Chip (SoC) component.
#include <ti_constants.h>

#ifndef __OMAP2420_SDIO_H
#define __OMAP2420_SDIO_H
//------------------------------------------------------------------------------


/////
// Soft Reset Register 
#define SOFT_RESET_ALL                  0x0002

/////
#define STD_HC_SLOTS                1

#define MMC_CTO_CONTROL_MAX         0x00FD  
#define MMC_DTO_CONTROL_MAX         0xFFFD
#define MMC_DTO_CONTROL             40000

#define STD_HC_MAX_CLOCK_FREQUENCY  24000000    // 24 MHz

#define MMCSD_CLOCK_INPUT           (96 * 1000 * 1000)
#define MMCSD_CLOCK_INIT            (400 * 1000)

#define SDMMC_DEFAULT_BLOCK_LEN     512
#define SDMMC_DEFAULT_NUM_BLOCKS      1
#define SDMMC_DEFAULT_ALMOST_EMPTY   32
#define SDMMC_DEFAULT_ALMOST_FULL    32

// The control register
#define MASK_MMC_CON_DW             (BIT15)
#define MASK_MMC_CON_MODE           (BIT13 | BIT12)
#define MASK_MMC_CON_POWERUP        (BIT11)
#define MASK_MMC_CON_BE             (BIT10)
#define MASK_MMC_CON_CLKD           (BIT9 | BIT8 | BIT7 | BIT6 | BIT5 | BIT4 | BIT3 | BIT2 | BIT1 | BIT0)

#define SHIFT_MMC_CON_DW            (15)
#define SHIFT_MMC_CON_MODE          (12)
#define SHIFT_MMC_CON_POWERUP       (11)
#define SHIFT_MMC_CON_BE            (10)
#define SHIFT_MMC_CON_CLKD          (0)

#define VALUE_MMC_CON_DW_1BIT           (0 << SHIFT_MMC_CON_DW)
#define VALUE_MMC_CON_DW_4BIT           (1 << SHIFT_MMC_CON_DW)
#define VALUE_MMC_CON_MODE_MMCSD        (0 << SHIFT_MMC_CON_MODE)
#define VALUE_MMC_CON_MODE_SPI          (1 << SHIFT_MMC_CON_MODE)
#define VALUE_MMC_CON_MODE_SYSTEST      (2 << SHIFT_MMC_CON_MODE)
#define VALUE_MMC_CON_MODE_RESV         (3 << SHIFT_MMC_CON_MODE)
#define VALUE_MMC_CON_POWERUP_DOWN      (0 << SHIFT_MMC_CON_POWERUP)
#define VALUE_MMC_CON_POWERUP_UP        (1 << SHIFT_MMC_CON_POWERUP)
#define VALUE_MMC_CON_BE_LITTLEENDIAN   (0 << SHIFT_MMC_CON_BE)
#define VALUE_MMC_CON_BE_BIGENDIAN      (1 << SHIFT_MMC_CON_BE)

// The status register
#define MMC_STAT_CERR   (1 << 14)
#define MMC_STAT_CIRQ   (1 << 13)
#define MMC_STAT_OCRB   (1 << 12)
#define MMC_STAT_AE     (1 << 11)
#define MMC_STAT_AF     (1 << 10)
#define MMC_STAT_CRW    (1 << 9)
#define MMC_STAT_CCRC   (1 << 8)
#define MMC_STAT_CTO    (1 << 7)
#define MMC_STAT_DCRC   (1 << 6)
#define MMC_STAT_DTO    (1 << 5)
#define MMC_STAT_EOFB   (1 << 4)
#define MMC_STAT_BRS    (1 << 3)
#define MMC_STAT_CB     (1 << 2)
#define MMC_STAT_CD     (1 << 1)
#define MMC_STAT_EOC    (1 << 0)

// The interrupt enable register
#define MMC_IE_EOC      (1 << 0)
#define MMC_IE_CD       (1 << 1)
#define MMC_IE_CB       (1 << 2)
#define MMC_IE_BRS      (1 << 3)
#define MMC_IE_EOFB     (1 << 4)
#define MMC_IE_DTO      (1 << 5)
#define MMC_IE_DCRC     (1 << 6)
#define MMC_IE_CTO      (1 << 7)
#define MMC_IE_CCRC     (1 << 8)
#define MMC_IE_CRW      (1 << 9)
#define MMC_IE_AF       (1 << 10)
#define MMC_IE_AE       (1 << 11)
#define MMC_IE_OCRB     (1 << 12)
#define MMC_IE_CIRQ     (1 << 13)
#define MMC_IE_CERR     (1 << 14)

#define MMC_BUF_RXDE    0x8000
#define MMC_BUF_TXDE    0x0080

// SDIO config register
#define MASK_MMC_SDIO_CDE       (BIT2)
#define SHIFT_MMC_SDIO_CDE  (2)
#define VALUE_MMC_SDIO_CDE_ENABLE   (1 << SHIFT_MMC_SDIO_CDE)


//
//  SD/MMC Registers
//

typedef volatile struct
{
  UINT16 MMC_CMD;               //offset 0x0
  UINT16 ulRESERVED_0x2;
  UINT16 MMC_ARG1;              //offset 0x4
  UINT16 ulRESERVED_0x6;
  UINT16 MMC_ARG2;              //offset 0x8
  UINT16 ulRESERVED_0xA;
  UINT16 MMC_CON;               //offset 0xC
  UINT16 ulRESERVED_0xE;
  UINT16 MMC_STAT;              //offset 0x10
  UINT16 ulRESERVED_0x12;
  UINT16 MMC_IE;                //offset 0x14
  UINT16 ulRESERVED_0x16;
  UINT16 MMC_CTO;               //offset 0x18
  UINT16 ulRESERVED_0x1A;
  UINT16 MMC_DTO;               //offset 0x1C
  UINT16 ulRESERVED_0x1E;
  UINT16 MMC_DATA;              //offset 0x20
  UINT16 ulRESERVED_0x22;
  UINT16 MMC_BLEN;              //offset 0x24
  UINT16 ulRESERVED_0x26;
  UINT16 MMC_NBLK;              //offset 0x28
  UINT16 ulRESERVED_0x2A;
  UINT16 MMC_BUF;               //offset 0x2C
  UINT16 ulRESERVED_0x2E[3];
  UINT16 MMC_SDIO;              //offset 0x34
  UINT16 ulRESERVED_0x36;
  UINT16 MMC_SYSTEST;           //offset 0x38
  UINT16 ulRESERVED_0x3A;
  UINT16 MMC_REV;               //offset 0x3C
  UINT16 ulRESERVED_0x3E;
  UINT16 MMC_RSP0;              //offset 0x40
  UINT16 ulRESERVED_0x42;
  UINT16 MMC_RSP1;              //offset 0x44
  UINT16 ulRESERVED_0x46;
  UINT16 MMC_RSP2;              //offset 0x48
  UINT16 ulRESERVED_0x4A;
  UINT16 MMC_RSP3;              //offset 0x4C
  UINT16 ulRESERVED_0x4E;
  UINT16 MMC_RSP4;              //offset 0x50
  UINT16 ulRESERVED_0x52;
  UINT16 MMC_RSP5;              //offset 0x54
  UINT16 ulRESERVED_0x56;
  UINT16 MMC_RSP6;              //offset 0x58
  UINT16 ulRESERVED_0x5A;
  UINT16 MMC_RSP7;              //offset 0x5C
  UINT16 ulRESERVED_0x5E;
  UINT16 MMC_IOSR;              //offset 0x60
  UINT16 ulRESERVED_0x62;
  UINT16 MMC_SYSC;              //offset 0x64
  UINT16 ulRESERVED_0x66;
  UINT16 MMC_SISS;              //offset 0x68
} OMAP2420_SDIO_REGS;

#define STD_HC_MAX_SLOTS                2

#define STD_HC_MIN_BLOCK_LENGTH         1
#define STD_HC_MAX_BLOCK_LENGTH         2048


#endif
