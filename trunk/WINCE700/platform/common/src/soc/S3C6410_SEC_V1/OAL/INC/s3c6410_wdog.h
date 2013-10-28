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
//  Header:  s3c6410_wdog.h
//
//  Defines the Watchdog Timer register layout and associated types
//  and constants.
//
#ifndef __S3C6410_WDOG_H
#define __S3C6410_WDOG_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  Type:  S3C6410_WATCHDOG_REG
//
//  Watchdog timer control registers. This register bank is located by the
//  constant S3C6410_BASE_REG_XX_WATCHDOG in the configuration file
//  s3c6410_reg_base_cfg.h.
//

typedef struct
{
    UINT32 WTCON;        // 0x00
    UINT32 WTDAT;        // 0x04
    UINT32 WTCNT;        // 0x08
    UINT32 WTCLRINT;    // 0x0c
} S3C6410_WATCHDOG_REG, *PS3C6410_WATCHDOG_REG;

//------------------------------------------------------------------------------

#if __cplusplus
    }
#endif

#endif
