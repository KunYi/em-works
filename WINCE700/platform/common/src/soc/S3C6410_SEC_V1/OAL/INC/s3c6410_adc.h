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
//  Header: s3c6410_adc.h
//
//  Defines the analog to digital (ADC) controller CPU register layout and
//  definitions.
//
#ifndef __S3C6410_ADC_H
#define __S3C6410_ADC_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  Type: S3C6410_ADC_REG
//
//  Defines the A/D converter's control register block. This register bank is
//  located by the constant S3C6410_BASE_REG_ADC in configuration file
//  s3c6410_base_reg_cfg.h.
//

typedef struct
{
    UINT32 ADCCON;        // 0x00
    UINT32 ADCTSC;        // 0x04
    UINT32 ADCDLY;        // 0x08
    UINT32 ADCDAT0;        // 0x0c

    UINT32 ADCDAT1;        // 0x10
    UINT32 ADCUPDN;    // 0x14
    UINT32 ADCCLRINT;    // 0x18
    UINT32 PAD0;        // 0x1c

    UINT32 ADCCLRWK;    // 0x20
    UINT32 PAD1[3];        // 0x24~0x2f
} S3C6410_ADC_REG, *PS3C6410_ADC_REG;

//------------------------------------------------------------------------------

#if __cplusplus
    }
#endif

#endif
