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
//  Header: s3c6410_post.h
//
//  Defines the Post Processor CPU register layout and definitions.
//
#ifndef __S3C6410_POST_H
#define __S3C6410_POST_H

#if __cplusplus
    extern "C"
    {
#endif


//------------------------------------------------------------------------------
//  Type: S3C6410_POST_REG
//
//  Post Processor register layout. This register bank is located
//  by the constant CPU_BASE_REG_XX_POST in the configuration file
//  cpu_base_reg_cfg.h.
//

typedef struct
{
    UINT32 MODE;                // 0x00
    UINT32 PreScale_Ratio;        // 0x04
    UINT32 PreScaleImgSize;        // 0x08
    UINT32 SRCImgSize;            // 0x0c

    UINT32 MainScale_H_Ratio;    // 0x10
    UINT32 MainScale_V_Ratio;    // 0x14
    UINT32 DSTImgSize;            // 0x18
    UINT32 PreScale_SHFactor;    // 0x1c

    UINT32 ADDRStart_Y;            // 0x20
    UINT32 ADDRStart_Cb;        // 0x24
    UINT32 ADDRStart_Cr;        // 0x28
    UINT32 ADDRStart_RGB;        // 0x2c

    UINT32 ADDREnd_Y;            // 0x30
    UINT32 ADDREnd_Cb;            // 0x34
    UINT32 ADDREnd_Cr;            // 0x38
    UINT32 ADDREnd_RGB;        // 0x3c

    UINT32 Offset_Y;                // 0x40
    UINT32 Offset_Cb;            // 0x44
    UINT32 Offset_Cr;            // 0x48
    UINT32 Offset_RGB;            // 0x4c

    UINT32 PAD0;                // 0x50
    UINT32 NxtADDRStart_Y;        // 0x54
    UINT32 NxtADDRStart_Cb;        // 0x58
    UINT32 NxtADDRStart_Cr;        // 0x5c

    UINT32 NxtADDRStart_RGB;    // 0x60
    UINT32 NxtADDREnd_Y;        // 0x64
    UINT32 NxtADDREnd_Cb;        // 0x68
    UINT32 NxtADDREnd_Cr;        // 0x6c

    UINT32 NxtADDREnd_RGB;        // 0x70
    UINT32 ADDRStart_oCb;        // 0x74
    UINT32 ADDRStart_oCr;        // 0x78
    UINT32 ADDREnd_oCb;        // 0x7c

    UINT32 ADDREnd_oCr;        // 0x80
    UINT32 Offset_oCb;            // 0x84
    UINT32 Offset_oCr;            // 0x88
    UINT32 NxtADDRStart_oCb;    // 0x8c

    UINT32 NxtADDRStart_oCr;    // 0x90
    UINT32 NxtADDREnd_oCb;        // 0x94
    UINT32 NxtADDREnd_oCr;        // 0x98
    UINT32 POSTENVID;            // 0x9c

    UINT32 MODE_2;                // 0xa0
    UINT32 PAD1[3];                // 0xa4~af
} S3C6410_POST_REG, *PS3C6410_POST_REG;

#if __cplusplus
    }
#endif

#endif    // __S3C6410_POST_H