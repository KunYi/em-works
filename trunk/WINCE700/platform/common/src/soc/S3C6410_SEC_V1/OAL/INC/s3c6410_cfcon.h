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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
//------------------------------------------------------------------------------
//
//  Header: s3c6410_cfcard.h
//
//  Defines the CF card controller CPU register layout and definitions.
//
#ifndef __S3C6410_CFCON_H_
#define __S3C6410_CFCON_H_

#if __cplusplus
    extern "C"
    {
#endif

//------------------------------------------------------------------------------
//  Type: S3C6410_CFCON_REG
//
//  SSMC control registers. This register bank is located by the constant
//  CPU_BASE_REG_XX_CFCARD in the configuration file cpu_base_reg_cfg.h.
//
// CF I/F Base Address = 0x70300000

typedef struct {
    UINT8    PAD1[0x1800];
    UINT32 MUX_REG;
    UINT8   PAD2[0x20-0x4];

    // PCCARD_BASE = 0x70301820
    UINT32    PCCARD_CNFG_STATUS;                // 0x70301820
    UINT32    PCCARD_INTMSK_SRC;                // 0x70301824
    UINT32    PCCARD_ATTR;                    // 0x70301828
    UINT32    PCCARD_IO;                        // 0x7030182C
    UINT32    PCCARD_COMM;                    // 0x70301830
    UINT8     PAD3[0x100-0x34];

    // ATAPI_BASE = 0x70301900
    UINT32 ATA_CONTROL;                        // 0x70301900
    UINT32 ATA_STATUS;                        // 0x70301904
    UINT32 ATA_COMMAND;                     // 0x70301908
    UINT32 ATA_SWRST;                         // 0x7030190c
    UINT32 ATA_IRQ;                            // 0x70301910
    UINT32 ATA_IRQ_MASK;                    // 0x70301914
    UINT32 ATA_CFG;                            // 0x70301918
    UINT32 ATA_RESERVED1;                    // 0x7030191c
    UINT32 ATA_RESERVED2;                    // 0x70301920
    UINT32 ATA_RESERVED3;                    // 0x70301924
    UINT32 ATA_RESERVED4;                    // 0x70301928
    UINT32 ATA_PIO_TIME;                    // 0x7030192c
    UINT32 ATA_UDMA_TIME;                    // 0x70301930
    UINT32 ATA_XFR_NUM;                        // 0x70301934
    UINT32 ATA_XFR_CNT;                        // 0x70301938
    UINT32 ATA_TBUF_START;                  // 0x7030193c
    UINT32 ATA_TBUF_SIZE;                    // 0x70301940
    UINT32 ATA_SBUF_START;                    // 0x70301944
    UINT32 ATA_SBUF_SIZE;                    // 0x70301948
    UINT32 ATA_CADDR_TBUR;                    // 0x7030194c
    UINT32 ATA_CADDR_SBUF;                    // 0x70301950
    UINT32 ATA_PIO_DTR;                        // 0x70301954
    UINT32 ATA_PIO_FED;                        // 0x70301958
    UINT32 ATA_PIO_SCR;                        // 0x7030195c
    UINT32 ATA_PIO_LLR;                        // 0x70301960
    UINT32 ATA_PIO_LMR;                        // 0x70301964
    UINT32 ATA_PIO_LHR;                        // 0x70301968
    UINT32 ATA_PIO_DVR;                        // 0x7030196c
    UINT32 ATA_PIO_CSD;                        // 0x70301970
    UINT32 ATA_PIO_DAD;                        // 0x70301974
    UINT32 ATA_PIO_READY;                    // 0x70301978
    UINT32 ATA_PIO_RDATA;                    // 0x7030197c
    UINT32 ATA_RESERVED6;                    // 0x70301980
    UINT32 ATA_RESERVED7;                    // 0x70301984
    UINT32 ATA_RESERVED8;                    // 0x70301988
    UINT32 ATA_RESERVED9;                    // 0x7030198c
    UINT32 BUS_FIFO_STATUS;                    // 0x70301990
    UINT32 ATA_FIFO_STATUS;                    // 0x70301994
} S3C6410_CFCON_REG, *PS3C6410_CFCON_REG;

#if __cplusplus
    }
#endif

#endif     // __S3C6410_CFATAPI_H_