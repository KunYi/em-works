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
//  Header: s3c6410_iic.h
//
//  Defines the IIC Bus controller CPU register layout and definitions.
//
#ifndef __S3C6410_IIC_H
#define __S3C6410_IIC_H

#if __cplusplus
extern "C"
{
#endif


//------------------------------------------------------------------------------
//  Type: S3C6410_IIC_REG
//
//  Defines IIC bus control register layout. This register bank is located by
//  the constant CPU_BASE_REG_XX_IIC in the configuration file
//  cpu_base_reg_cfg.h.
//
// IIC Base Address = 0x7F004000

typedef struct
{
    UINT32    IICCON;
    UINT32    IICSTAT;
    UINT32    IICADD;
    UINT32    IICDS;
    UINT32    IICLC;

} S3C6410_IIC_REG, *PS3C6410_IIC_REG;


#if __cplusplus
    }
#endif

#endif
