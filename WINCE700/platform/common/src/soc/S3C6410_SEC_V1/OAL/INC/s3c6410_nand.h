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
//  Header: s3c6410_nand.h
//
//  Defines the NAND controller CPU register layout and definitions.
//
#ifndef __S3C6410_NAND_H
#define __S3C6410_NAND_H

#if __cplusplus
    extern "C"
    {
#endif


//------------------------------------------------------------------------------
//  Type: S3C6410_NAND_REG
//
//  NAND Flash controller register layout. This register bank is located
//  by the constant CPU_BASE_REG_XX_NAND in the configuration file
//  cpu_base_reg_cfg.h.
//

typedef struct
{
    UINT32  NFCONF;         //0x00    // configuration reg
    UINT32  NFCONT;         //0x04
    UINT8   NFCMD;          //0x08    // command set reg
    UINT8   d0[3];
    UINT8   NFADDR;         //0x0C    // address set reg
    UINT8   d1[3];
    UINT8   NFDATA;         //0x10    // data reg
    UINT8   d2[3];
    UINT32  NFMECCD0;       //0x14
    UINT32  NFMECCD1;       //0x18
    UINT32  NFSECCD;        //0x1C
    UINT32  NFSBLK;         //0x20
    UINT32  NFEBLK;         //0x24  // error correction code 2
    UINT32  NFSTAT;         //0x28  // operation status reg
    UINT32  NFECCERR0;      //0x2C
    UINT32  NFECCERR1;      //0x30
    UINT32  NFMECC0;        //0x34  // error correction code 0
    UINT32  NFMECC1;        //0x38  // error correction code 1
    UINT32  NFSECC;         //0x3C
    UINT32  NFMLCBITPT;     //0x40

#ifdef _IROMBOOT_
    UINT32  NF8ECCERR0;     //0x44  // 8bit ECC error status0 register
    UINT32  NF8ECCERR1;     //0x48  // 8bit ECC error status1 register
    UINT32  NF8ECCERR2;     //0x4C  // 8bit ECC error status2 register
    UINT32  NFM8ECC0;       //0x50  // Generated 8-bit ECC status0 register
    UINT32  NFM8ECC1;       //0x54  // Generated 8-bit ECC status1 register
    UINT32  NFM8ECC2;       //0x58  // Generated 8-bit ECC status2 register
    UINT32  NFM8ECC3;       //0x5C  // Generated 8-bit ECC status3 register
    UINT32  NFMLC8BITPT0;   //0x60  // 8-bit ECC error bit pattern 0 register
    UINT32  NFMLC8BITPT1;   //0x60  // 8-bit ECC error bit pattern 1 register

#endif  // !_IROMBOOT_

} S3C6410_NAND_REG, *PS3C6410_NAND_REG;

typedef struct _REG_NFCONF
{
    unsigned int cfgenbnfcon    : 1;
    unsigned int mlcclkctrl     : 1;
    unsigned int    : 4;
    unsigned int msglength      : 1;
    unsigned int ecctype        : 2;
    unsigned int    : 8;
    unsigned int tacls          : 3;
    unsigned int    : 1;
    unsigned int twrph0         : 3;
    unsigned int    : 1;
    unsigned int twrph1         : 3;
    unsigned int pagesize       : 1;
    unsigned int    : 1;
    unsigned int addrcycle      : 1;
    unsigned int    : 1;
} SYSCON_NFCONF;

#define BIT_CfgEnbNFCON             (31)
#define BIT_MLCClkCtrl              (30)
#define BIT_MsgLength               (25)
#define BIT_ECCType                 (23)
#define BIT_TACLS                   (12)
#define BIT_TWRPH0                  (8)
#define BIT_TWRPH1                  (4)
#define BIT_PageSize                (3)
#define BIT_AddrCycle               (1)

typedef struct _REG_NFCONT
{
    unsigned int    :13;   // RESERVED
    unsigned int eccdirection       : 1;
    unsigned int locktight          : 1;
    unsigned int softlock           : 1;
    unsigned int    : 3;
    unsigned int enbeccdeclnt       : 1;
    unsigned int stop8bitecc        : 1;
    unsigned int enbillegalacclnt   : 1;
    unsigned int enbrnbint          : 1;
    unsigned int rnb_transmode      : 1;
    unsigned int mainecclock        : 1;
    unsigned int spareecclock       : 1;
    unsigned int initmecc           : 1;
    unsigned int initsecc           : 1;
    unsigned int    : 1;
    unsigned int reg_nce1           : 1;
    unsigned int reg_nce0           : 1;
    unsigned int mode               : 1;
} SYSCON_REG_NFCONT;

#define NFCONT_DECODE_4_8_ECC_READ  (0<<18)
#define NFCONT_ENCODE_4_8_ECC_PROGRAM   (1<<18)
#define NFCONT_DISABLE_LOCK_TIGHT   (0<<17)
#define NFCONT_ENABLE_LOCK_TIGHT    (1<<17)
#define NFCONT_DISABLE_SOFTLOCK     (0<<16)
#define NFCONT_ENABLE_SOFTLOCK      (1<<16)
#define NFCONT_DISABLE_ECCDECINT    (0<<12)
#define NFCONT_ENABLE_ECCDECINT     (1<<12)
#define NFCONT_8BIT_ECC_STOP        (1<<11)
#define NFCONT_DISABLE_ILLEGAL_ACCESS_INT   (0<<10)
#define NFCONT_ENABLE_ILLEGAL_ACCESS_INT   (1<<10)
#define NFCONT_DISABLE_RNB_INT      (0<<9)
#define NFCONT_ENABLE_RNB_INT       (1<<9)
#define NFCONT_DETECT_RNB_RISING    (0<<8)
#define NFCONT_DETECT_RNB_FALLING   (1<<8)
#define NFCONT_UNLOCK_MECC          (0<<7)
#define NFCONT_LOCK_MECC            (1<<7)
#define NFCONT_UNLOCK_SECC          (0<<6)
#define NFCONT_LOCK_SECC            (1<<6)
#define NFCONT_INIT_MECC            (1<<5)
#define NFCONT_INIT_SECC            (1<<4)
#define NFCONT_REG_NCE1_LOW         (0<<2)
#define NFCONT_REG_NCE1_HIGH        (1<<2)
#define NFCONT_REG_NCE0_LOW         (0<<1)
#define NFCONT_REG_NCE0_HIGH        (1<<1)
#define NFCONT_DISABLE              (1<<0)
#define NFCONT_ENABLE               (1<<0)

#if __cplusplus
    }
#endif

#endif

