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
//  Header: s3c6410_iis.h
//
//  Defines the IIS bus control register layout and associated constants 
//  and types.
//
//------------------------------------------------------------------------------

#ifndef __S3C6410_IIS_H
#define __S3C6410_IIS_H

#if __cplusplus
    extern "C" 
    {
#endif


//------------------------------------------------------------------------------
//  Type: name    
//
//  Defines IIS bus control register layout. This register bank is located by
//  the constant CPU_BASE_REG_XX_IIS in the configuration file 
//  cpu_base_reg_cfg.h.
//

typedef struct 
{
    UINT32    IISCON;     // 0x00 - I2S interface control register
    UINT32    IISMOD;    // 0x04 - I2S interface mode register
    UINT32    IISFIC;    // 0x08 - I2S interface FIFO control register
    UINT32    IISPSR;    // 0x0c - I2S interface clock divider control register
    UINT32    IISTXD;    // 0x10 - I2S interface transmit data register
    UINT32    IISRXD;    // 0x14 - I2S interface receive data register

} S3C6410_IIS_REG, *PS3C6410_IIS_REG;


#if __cplusplus
    }
#endif

#endif 
