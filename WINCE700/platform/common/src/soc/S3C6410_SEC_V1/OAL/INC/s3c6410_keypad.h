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
//  Header: s3c6410_keypad.h
//
//  Defines the KEYPAD Interface controller CPU register layout and definitions.
//
#ifndef __S3C6410_KEYPAD_H
#define __S3C6410_KEYPAD_H

#if __cplusplus
extern "C" 
{
#endif


//------------------------------------------------------------------------------
//  Type: S3C6410_KEYPAD_REG
//
//  Defines KEYPAD interface control register layout. This register bank is located by
//  the constant CPU_BASE_REG_XX_KEYPAD in the configuration file 
//  cpu_base_reg_cfg.h.
//
// KEYPAD Base Address = 0x7E00A000

typedef struct 
{
    UINT32 KEYIFCON;     // 0x00
    UINT32 KEYIFSTSCLR;    // 0x04
    UINT32 KEYIFCOL;    // 0x08
    UINT32 KEYIFROW;    // 0x0C
    UINT32 KEYIFFC;        // 0x10
    UINT32 KEYIFCLRINT; // 0x14
    
} S3C6410_KEYPAD_REG, *PS3C6410_KEYPAD_REG;        


#if __cplusplus
    }
#endif

#endif 
