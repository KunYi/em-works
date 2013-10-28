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
//  Header:  s3c6410_uart.h
//
//  Defines the UART controller register layout associated types and constants.
//
#ifndef __S3C6410_UART_H
#define __S3C6410_UART_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  Type:  S3C6410_UART_REG
//
//  UART control registers. This register bank is located by the constant
//  S3C6410_BASE_REG_XX_UARTn in the configuration file
//  s3c6410_base_reg_cfg.h.
//

// This definition is for each channel registers
typedef struct
{
    UINT32 ULCON;        // 0x000
    UINT32 UCON;        // 0x004
    UINT32 UFCON;        // 0x008
    UINT32 UMCON;        // 0x00c

    UINT32 UTRSTAT;        // 0x010
    UINT32 UERSTAT;        // 0x014
    UINT32 UFSTAT;        // 0x018
    UINT32 UMSTAT;        // 0x01c

    UINT32 UTXH;        // 0x020
    UINT32 URXH;        // 0x024
    UINT32 UBRDIV;        // 0x028
    UINT32 UDIVSLOT;    // 0x02c

    UINT32 UINTP;        // 0x030
    UINT32 UINTSP;        // 0x034
    UINT32 UINTM;        // 0x038
    UINT32 PAD0;        // 0x03c
} S3C6410_UART_REG, *PS3C6410_UART_REG;

typedef struct
{
    UINT32 ULCON0;        // 0x000
    UINT32 UCON0;        // 0x004
    UINT32 UFCON0;        // 0x008
    UINT32 UMCON0;        // 0x00c

    UINT32 UTRSTAT0;    // 0x010
    UINT32 UERSTAT0;    // 0x014
    UINT32 UFSTAT0;        // 0x018
    UINT32 UMSTAT0;        // 0x01c

    UINT32 UTXH0;        // 0x020
    UINT32 URXH0;        // 0x024
    UINT32 UBRDIV0;        // 0x028
    UINT32 UDIVSLOT0;    // 0x02c

    UINT32 UINTP0;        // 0x030
    UINT32 UINTSP0;        // 0x034
    UINT32 UINTM0;        // 0x038
    UINT32 PAD0;        // 0x03c

    UINT32 PAD1[240];    // 0x040~0x3ff

    UINT32 ULCON1;        // 0x400
    UINT32 UCON1;        // 0x404
    UINT32 UFCON1;        // 0x408
    UINT32 UMCON1;        // 0x40c

    UINT32 UTRSTAT1;    // 0x410
    UINT32 UERSTAT1;    // 0x414
    UINT32 UFSTAT1;        // 0x418
    UINT32 UMSTAT1;        // 0x41c

    UINT32 UTXH1;        // 0x420
    UINT32 URXH1;        // 0x424
    UINT32 UBRDIV1;        // 0x428
    UINT32 UDIVSLOT1;    // 0x42c

    UINT32 UINTP1;        // 0x430
    UINT32 UINTSP1;        // 0x434
    UINT32 UINTM1;        // 0x438
    UINT32 PAD2;        // 0x43c

    UINT32 PAD3[240];    // 0x440~0x7ff

    UINT32 ULCON2;        // 0x800
    UINT32 UCON2;        // 0x804
    UINT32 UFCON2;        // 0x808
    UINT32 UMCON2;        // 0x80c

    UINT32 UTRSTAT2;    // 0x810
    UINT32 UERSTAT2;    // 0x814
    UINT32 UFSTAT2;        // 0x818
    UINT32 UMSTAT2;        // 0x81c

    UINT32 UTXH2;        // 0x820
    UINT32 URXH2;        // 0x824
    UINT32 UBRDIV2;        // 0x828
    UINT32 UDIVSLOT2;    // 0x82c

    UINT32 UINTP2;        // 0x830
    UINT32 UINTSP2;        // 0x834
    UINT32 UINTM2;        // 0x838
    UINT32 PAD4;        // 0x83c

    UINT32 PAD5[240];    // 0x840~0xbff

    UINT32 ULCON3;        // 0xc00
    UINT32 UCON3;        // 0xc04
    UINT32 UFCON3;        // 0xc08
    UINT32 UMCON3;        // 0xc0c

    UINT32 UTRSTAT3;    // 0xc10
    UINT32 UERSTAT3;    // 0xc14
    UINT32 UFSTAT3;        // 0xc18
    UINT32 UMSTAT3;        // 0xc1c

    UINT32 UTXH3;        // 0xc20
    UINT32 URXH3;        // 0xc24
    UINT32 UBRDIV3;        // 0xc28
    UINT32 UDIVSLOT3;    // 0xc2c

    UINT32 UINTP3;        // 0xc30
    UINT32 UINTSP3;        // 0xc34
    UINT32 UINTM3;        // 0xc38
    UINT32 PAD6;        // 0xc3c
} S3C6410_UART_FULL_BLOCK_REG, *PS3C6410_UART_FULL_BLOCK_REG;

//------------------------------------------------------------------------------

#if __cplusplus
    }
#endif

#endif    // __S3C6410_UART_H
