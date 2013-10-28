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
//  Header: s3c6410_dma.h
//
//  Defines the DMA Controller CPU register layout and
//  definitions.
//
#ifndef __S3C6410_DMA_H
#define __S3C6410_DMA_H

#if __cplusplus
    extern "C"
    {
#endif


//------------------------------------------------------------------------------
//  Type: S3C6410_DMAC_REG
//
//  Defines the SPI register layout. This register bank is located by the
//  constant CPU_REG_BASE_XX_SPIX in the configuration file s3c6410_reg_base_cfg.h.
//

typedef struct
{
    UINT32 DMACIntStatus;            // 0x00
    UINT32 DMACIntTCStatus;            // 0x04
    UINT32 DMACIntTCClear;            // 0x08
    UINT32 DMACIntErrStatus;            // 0x0C

    UINT32 DMACIntErrClear;            // 0x10
    UINT32 DMACRawIntTCStatus;        // 0x14
    UINT32 DMACRawIntErrStatus;        // 0x18
    UINT32 DMACEnbldChns;            // 0x1C

    UINT32 DMACSoftBReq;            // 0x20
    UINT32 DMACSoftSReq;            // 0x24
    UINT32 DMACSoftLBReq;            // 0x28
    UINT32 DMACSoftLSReq;            // 0x2C

    UINT32 DMACConfiguration;        // 0x30
    UINT32 DMACSync;                // 0x34
    UINT32 PAD0[2];                    // 0x38~0x3F

    UINT32 PAD1[48];                    // 0x40~0xFF

    UINT32 DMACC0SrcAddr;            // 0x100
    UINT32 DMACC0DestAddr;            // 0x104
    UINT32 DMACC0LLI;                // 0x108
    UINT32 DMACC0Control0;            // 0x10C

    UINT32 DMACC0Control1;            // 0x110
    UINT32 DMACC0Configuration;        // 0x114
    UINT32 PAD2[2];                    // 0x118~0x11F

    UINT32 DMACC1SrcAddr;            // 0x120
    UINT32 DMACC1DestAddr;            // 0x124
    UINT32 DMACC1LLI;                // 0x128
    UINT32 DMACC1Control0;            // 0x12C

    UINT32 DMACC1Control1;            // 0x130
    UINT32 DMACC1Configuration;        // 0x134
    UINT32 PAD3[2];                    // 0x138~0x13F

    UINT32 DMACC2SrcAddr;            // 0x140
    UINT32 DMACC2DestAddr;            // 0x144
    UINT32 DMACC2LLI;                // 0x148
    UINT32 DMACC2Control0;            // 0x14C

    UINT32 DMACC2Control1;            // 0x150
    UINT32 DMACC2Configuration;        // 0x154
    UINT32 PAD4[2];                    // 0x158~0x15F

    UINT32 DMACC3SrcAddr;            // 0x160
    UINT32 DMACC3DestAddr;            // 0x164
    UINT32 DMACC3LLI;                // 0x168
    UINT32 DMACC3Control0;            // 0x16C

    UINT32 DMACC3Control1;            // 0x170
    UINT32 DMACC3Configuration;        // 0x174
    UINT32 PAD5[2];                    // 0x178~0x17F

    UINT32 DMACC4SrcAddr;            // 0x180
    UINT32 DMACC4DestAddr;            // 0x184
    UINT32 DMACC4LLI;                // 0x188
    UINT32 DMACC4Control0;            // 0x18C

    UINT32 DMACC4Control1;            // 0x190
    UINT32 DMACC4Configuration;        // 0x194
    UINT32 PAD6[2];                    // 0x198~0x19F

    UINT32 DMACC5SrcAddr;            // 0x1A0
    UINT32 DMACC5DestAddr;            // 0x1A4
    UINT32 DMACC5LLI;                // 0x1A8
    UINT32 DMACC5Control0;            // 0x1AC

    UINT32 DMACC5Control1;            // 0x1B0
    UINT32 DMACC5Configuration;        // 0x1B4
    UINT32 PAD7[2];                    // 0x1B8~0x1BF

    UINT32 DMACC6SrcAddr;            // 0x1C0
    UINT32 DMACC6DestAddr;            // 0x1C4
    UINT32 DMACC6LLI;                // 0x1C8
    UINT32 DMACC6Control0;            // 0x1CC

    UINT32 DMACC6Control1;            // 0x1D0
    UINT32 DMACC6Configuration;        // 0x1D4
    UINT32 PAD8[2];                    // 0x1D8~0x1DF

    UINT32 DMACC7SrcAddr;            // 0x1E0
    UINT32 DMACC7DestAddr;            // 0x1E4
    UINT32 DMACC7LLI;                // 0x1E8
    UINT32 DMACC7Control0;            // 0x1EC

    UINT32 DMACC7Control1;            // 0x1F0
    UINT32 DMACC7Configuration;        // 0x1F4
    UINT32 PAD9[2];                    // 0x1F8~0x1FF
} S3C6410_DMAC_REG, *PS3C6410_DMAC_REG;

//------------------------------------------------------------------------------

#if __cplusplus
    }
#endif

#endif    // __S3C6410_DMA_H
