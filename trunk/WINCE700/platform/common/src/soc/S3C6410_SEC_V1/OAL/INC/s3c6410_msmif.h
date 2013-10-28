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
//  Header: s3c6410_msmif.h
//
//  Defines the Modem Interface CPU register layout and definitions.
//
#ifndef __S3C6410_MSMIF_H
#define __S3C6410_MSMIF_H

#if __cplusplus
    extern "C"
    {
#endif


//------------------------------------------------------------------------------
//  Type: S3C6410_MSMIF_REG
//
//  Post Processor register layout. This register bank is located
//  by the constant CPU_BASE_REG_XX_POST in the configuration file
//  cpu_base_reg_cfg.h.
//

typedef struct
{
    UINT32 INT2AP;            // 0x00
    UINT32 INT2MSM;            // 0x04
    UINT32 MIFCON;            // 0x08
    UINT32 MIFPCON;            // 0x0c

    UINT32 MSMINTCLR;        // 0x10
    UINT32 PAD[3];            // 0x14~0x1f
} S3C6410_MSMIF_REG, *PS3C6410_MSMIF_REG;

#if __cplusplus
    }
#endif

#endif    // __S3C6410_MSMIF_H