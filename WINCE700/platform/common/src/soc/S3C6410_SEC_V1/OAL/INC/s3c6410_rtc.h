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
//  Header: s3c6410_rtc.h
//
//  Defines the Real Time Clock (RTC) register layout and associated
//  types and constants.
//
#ifndef __S3C6410_RTC_H__
#define __S3C6410_RTC_H__

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  Type: S3C6410_RTC_REG
//
//  RTC control registers. This register bank is located by the constant
//  S3C6410_BASE_REG_XX_RTC in the configuration file s3c6410_base_reg_cfg.h.
//

typedef struct
{
    UINT32 PAD0[12];        // 0x00~0x2f

    UINT32 INTP;            // 0x30
    UINT32 PAD1[3];        // 0x34~0x3f

    UINT32 RTCCON;        // 0x40
    UINT32 TICCNT;        // 0x44
    UINT32 PAD2[2];        // 0x48~0x4f

    UINT32 RTCALM;        // 0x50
    UINT32 ALMSEC;        // 0x54
    UINT32 ALMMIN;        // 0x58
    UINT32 ALMHOUR;    // 0x5c

    UINT32 ALMDATE;        // 0x60
    UINT32 ALMMON;        // 0x64
    UINT32 ALMYEAR;        // 0x68
    UINT32 PAD3;        // 0x6c

    UINT32 BCDSEC;        // 0x70
    UINT32 BCDMIN;        // 0x74
    UINT32 BCDHOUR;        // 0x78
    UINT32 BCDDATE;        // 0x7c

    UINT32 BCDDAY;        // 0x80
    UINT32 BCDMON;        // 0x84
    UINT32 BCDYEAR;        // 0x88
    UINT32 PAD4;        // 0x8c

    UINT32 CURTICCNT;    // 0x90
    UINT32 RTCLVD;        // 0x94
    UINT32 PAD5[2];        // 0x98~0x9f
} S3C6410_RTC_REG, *PS3C6410_RTC_REG;

//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif
