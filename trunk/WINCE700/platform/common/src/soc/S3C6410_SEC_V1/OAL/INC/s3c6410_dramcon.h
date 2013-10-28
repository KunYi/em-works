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
//  Header: s3c6410_dramcon.h
//
//  Defines the DRAM controller CPU register layout and definitions.
//
#ifndef __S3C6410_DRAMCON_H
#define __S3C6410_DRAMCON_H

#if __cplusplus
    extern "C"
    {
#endif

//------------------------------------------------------------------------------
//  Type: S3C6410_DRAMCON_REG
//
//  DRAM controller register layout. This register bank is located
//  by the constant CPU_BASE_REG_XX_DMCX in the configuration file
//  cpu_base_reg_cfg.h.
//

typedef struct
{
    UINT32 MEMSTAT;    // 0x00
    UINT32 MEMCCMD;    // 0x04
    UINT32 DIRECTCMD;    // 0x08
    UINT32 MEMCFG;        // 0x0c

    UINT32 REFRESH;        // 0x10
    UINT32 CASLAT;        // 0x14
    UINT32 DQSS;        // 0x18
    UINT32 T_MRD;        // 0x1c

    UINT32 T_RAS;        // 0x20
    UINT32 T_RC;            // 0x24
    UINT32 T_RCD;        // 0x28
    UINT32 T_RFC;        // 0x2c

    UINT32 T_RP;            // 0x30
    UINT32 T_RRD;        // 0x34
    UINT32 T_WR;        // 0x38
    UINT32 T_WTR;        // 0x3c

    UINT32 T_XP;            // 0x40
    UINT32 T_XSR;        // 0x44
    UINT32 T_ESR;        // 0x48
    UINT32 MEMCFG2;    // 0x4c

    UINT32 PAD0[44];        // 0x50~0xff

    UINT32 id_0_cfg;        // 0x100
    UINT32 id_1_cfg;        // 0x104
    UINT32 id_2_cfg;        // 0x108
    UINT32 id_3_cfg;        // 0x10c

    UINT32 id_4_cfg;        // 0x110
    UINT32 id_5_cfg;        // 0x114
    UINT32 id_6_cfg;        // 0x118
    UINT32 id_7_cfg;        // 0x11c

    UINT32 id_8_cfg;        // 0x120
    UINT32 id_9_cfg;        // 0x124
    UINT32 id_10_cfg;    // 0x128
    UINT32 id_11_cfg;    // 0x12c

    UINT32 id_12_cfg;    // 0x130
    UINT32 id_13_cfg;    // 0x134
    UINT32 id_14_cfg;    // 0x138
    UINT32 id_15_cfg;    // 0x13c

    UINT32 PAD1[48];        // 0x140~0x1ff

    UINT32 chip_0_cfg;    // 0x200
    UINT32 chip_1_cfg;    // 0x204
    UINT32 PAD2[2];        // 0x208~0x20f

    UINT32 PAD3[60];        // 0x210~0x2ff

    UINT32 user_stat;    // 0x300
    UINT32 user_cfg;        // 0x304
    UINT32 PAD4[2];        // 0x308~0x30f
} S3C6410_DRAMCON_REG, *PS3C6410_DRAMCON_REG;

#if __cplusplus
    }
#endif

#endif    // __S3C6410_DRAMCON_H
