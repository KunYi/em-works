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
//  Header: s3c6410_tvenc.h
//
//  Defines the TV Encoder CPU register layout and definitions.
//
#ifndef __S3C6410_TVENC_H
#define __S3C6410_TVENC_H

#if __cplusplus
    extern "C"
    {
#endif


//------------------------------------------------------------------------------
//  Type: S3C6410_TVENC_REG
//
//  Post Processor register layout. This register bank is located
//  by the constant CPU_BASE_REG_XX_POST in the configuration file
//  cpu_base_reg_cfg.h.
//

typedef struct
{
    // TV Controller
    UINT32 TVCTRL;            // 0x00
    UINT32 VBPORCH;            // 0x04
    UINT32 HBPORCH;            // 0x08
    UINT32 HENHOFFSET;        // 0x0c

    UINT32 VDEMOWSIZE;        // 0x10
    UINT32 HDEMOWSIZE;        // 0x14
    UINT32 INIMAGESIZE;        // 0x18
    // Encoder
    UINT32 PEDCTRL;            // 0x1c

    UINT32 YCFILTERBW;        // 0x20
    UINT32 HUECTRL;            // 0x24
    UINT32 FSCCTRL;            // 0x28
    UINT32 FSCDTOMANCTRL;    // 0x2c

    UINT32 PAD0;            // 0x30
    UINT32 BGCTRL;            // 0x34
    UINT32 BGHVAVCTRL;        // 0x38
    UINT32 PAD1;            // 0x3c

    // Image Enhancer
    UINT32 PAD2;            // 0x40
    UINT32 CONTRABRIGHT;    // 0x44
    UINT32 CBCRGAINCTRL;    // 0x48
    UINT32 DEMOWINCTRL;    // 0x4c

    UINT32 PAD3[2];            // 0x50~0x57
    UINT32 BWGAIN;            // 0x58
    UINT32 PAD4;            // 0x5c

    UINT32 SHARPCTRL;        // 0x60
    UINT32 GAMMACTRL;        // 0x64
    UINT32 FSCAUXCTRL;        // 0x68
    UINT32 SYNCSIZECTRL;    // 0x6c

    UINT32 BURSTCTRL;        // 0x70
    UINT32 MACROBURSTCTRL;    // 0x74
    UINT32 ACTVIDPOSCTRL;    // 0x78
    UINT32 ENCCTRL;            // 0x7c

    UINT32 MUTECTRL;        // 0x80
    UINT32 MACROVISION0;    // 0x84
    UINT32 MACROVISION1;    // 0x88
    UINT32 MACROVISION2;    // 0x8c

    UINT32 MACROVISION3;    // 0x90
    UINT32 MACROVISION4;    // 0x94
    UINT32 MACROVISION5;    // 0x98
    UINT32 MACROVISION6;    // 0x9c
} S3C6410_TVENC_REG, *PS3C6410_TVENC_REG;

#if __cplusplus
    }
#endif

#endif    // __S3C6410_TVSC_H