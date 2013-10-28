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
//  Header: s3c6410_hsmmc.h
//
//  Defines the High speed MMC controller CPU register layout and
//  definitions.
//
#ifndef __S3C6410_HSMMC_H
#define __S3C6410_HSMMC_H

#if __cplusplus
    extern "C" 
    {
#endif


//------------------------------------------------------------------------------
//  Type: S3C6410_HSMMC_REG    
//
//  High speed MMC Control Registers. This register bank is located by the constant
//  CPU_BASE_REG_XX_SDI in the configuration file cpu_base_reg_cfg.h.
//
// HS MMC 0 Base Address = 0x7c200000
// HS MMC 1 Base Address = 0x7c300000
// HS MMC 2 Base Address = 0x7c400000

typedef struct 
{
    UINT32    SYSAD;                 // 0x00
    UINT16    BLKSIZE;               // 0x04
    UINT16    BLKCNT;                // 0x06
    UINT32    ARGUMENT;              // 0x08
    UINT16    TRNMOD;                // 0x0C
    UINT16    CMDREG;                // 0x0E
    UINT32    RSPREG0;               // 0x10
    UINT32    RSPREG1;               // 0x14
    UINT32    RSPREG2;               // 0x18
    UINT32    RSPREG3;               // 0x1C
    UINT32    BDATA;                 // 0x20
    UINT32    PRNSTS;                // 0x24
    UINT8     HOSTCTL;               // 0x28
    UINT8     PWRCON;                // 0x29
    UINT8     BLKGAP;                // 0x2A
    UINT8     WAKCON;                // 0x2B
    UINT16    CLKCON;                // 0x2C
    UINT8     TIMEOUTCON;            // 0x2E
    UINT8     SWRST;                 // 0x2F
    UINT16    NORINTSTS;             // 0x30
    UINT16    ERRINTSTS;             // 0x32
    UINT16    NORINTSTSEN;           // 0x34
    UINT16    ERRINTSTSEN;           // 0x36
    UINT16    NORINTSIGEN;           // 0x38
    UINT16    ERRINTSIGEN;           // 0x3A
    UINT16    ACMD12ERRSTS;          // 0x3C
    UINT16    PAD1;                  // 0x3E
    UINT32    CAPAREG;               // 0x40
    UINT32    PAD2;                  // 0x44
    UINT32    MAXCURR;               // 0x48
    UINT32    PAD3;                     // 0x4C
    UINT16      FEAER;                   // 0x50 (2.0)
    UINT16      FEERR;                    // 0x52 (2.0)
    UINT32      ADMAERR;                   // 0x54 (2.0)
    UINT32      ADMASYSADDR;             // 0x58 (2.0)
    UINT32      PAD4[9];               // 0x5C ~ 0x7C
    UINT32    CONTROL2;              // 0x80
    UINT32    CONTROL3;              // 0x84
    UINT32    PAD5;                 // 0x88 (2.0)
    UINT32      CONTROL4;              // 0x8C (2.0)
    UINT32    PAD6[27];              // 0x90 ~ 0xF8
    UINT16    PAD7;                  // 0xFC
    UINT16    HCVER;                 // 0xFE
} S3C6410_HSMMC_REG, *PS3C6410_HSMMC_REG;


#if __cplusplus
    }
#endif

#endif 
