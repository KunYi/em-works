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
//  Header: s3c6410_rotator.h
//
//  Defines the Image Rotator CPU register layout and definitions.
//
#ifndef __S3C6410_ROTATOR_H
#define __S3C6410_ROTATOR_H

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
    UINT32 CTRLCFG;                // 0x00
    UINT32 SrcAddrReg0;            // 0x04
    UINT32 SrcAddrReg1;            // 0x08
    UINT32 SrcAddrReg2;            // 0x0c    

    UINT32 SrcSizeReg;            // 0x10
    UINT32 PAD0;                // 0x14
    UINT32 DstAddrReg0;            // 0x18
    UINT32 DstAddrReg1;            // 0x1c

    UINT32 DstAddrReg2;            // 0x20
    UINT32 PAD1;                // 0x24
    UINT32 PAD2;                // 0x28
    UINT32 STATCFG;                // 0x2c    
} S3C6410_ROTATOR_REG, *PS3C6410_ROTATOR_REG;

#if __cplusplus
    }
#endif

#endif    // __S3C6410_ROTATOR_H