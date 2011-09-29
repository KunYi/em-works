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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
//------------------------------------------------------------------------------
//
//  File:  omap5912_emif.h
//
#ifndef __OMAP5912_EMIF_H
#define __OMAP5912_EMIF_H

//------------------------------------------------------------------------------

typedef volatile struct {

    UINT32 OCPT1_PRIOR;                 // 0000
    UINT32 EMIFS_LRUREG;                // 0004
    UINT32 EMIFF_PRIORITY;              // 0008
    UINT32 EMIFS_CONFIG;                // 000C
    UINT32 EMIFS_CFG_0;                 // 0010
    UINT32 EMIFS_CFG_1;                 // 0014
    UINT32 EMIFS_CFG_2;                 // 0018
    UINT32 EMIFS_CFG_3;                 // 001C
    UINT32 EMIFF_CONFIG;                // 0020
    UINT32 EMIFF_MRS;                   // 0024
    UINT32 EMIFS_TIMEOUT1;              // 0028
    UINT32 EMIFS_TIMEOUT2;              // 002C
    UINT32 EMIFS_TIMEOUT3;              // 0030
    UINT32 ENDIANISM_CONTROL;           // 0034
    UINT32 RESERVED_0038;               // 0038
    UINT32 EMIFF_CONFIG2;               // 003C
    UINT32 EMIFS_FL_CFG_DYN_WAIT;       // 0040
    UINT32 EMIFS_ABORT_ADDR;            // 0044
    UINT32 EMIFS_ABORT_TYPE;            // 0048
    UINT32 EMIFS_ABORT_TOUT;            // 004C
    UINT32 EMIFS_ACFG_0_I;              // 0050
    UINT32 EMIFS_ACFG_1_I;              // 0054
    UINT32 EMIFS_ACFG_2_I;              // 0058
    UINT32 EMIFS_ACFG_3_I;              // 005C
    UINT32 EMIFF_DOUBLER_EN;            // 0060
    UINT32 EMIFF_DLL_WRT_CONTROL;       // 0064
    UINT32 EMIFF_DLL_WRT_STATUS;        // 0068
    UINT32 RESERVED_006C;               // 006C
    UINT32 EMIFF_MRS_NEW;               // 0070
    UINT32 EMIFF_EMRS0;                 // 0074
    UINT32 EMIFF_EMRS1;                 // 0078
    UINT32 RESERVED_007C;               // 007C
    UINT32 EMIFF_OPERATION;             // 0080
    UINT32 EMIFF_MANUAL_CMD;            // 0084
    UINT32 RESERVED_0088;               // 0088
    UINT32 EMIFF_TIMEOUT1;              // 008C
    UINT32 EMIFF_TIMEOUT2;              // 0090
    UINT32 EMIFF_TIMEOUT3;              // 0094
    UINT32 EMIFF_ABORT_ADDRESS;         // 0098
    UINT32 EMIFF_ABORT_TYPE;            // 009C
    UINT32 OCPT1_PTOR1;                 // 00A0
    UINT32 OCPT1_PTOR2;                 // 00A4
    UINT32 OCPT1_PTOR3;                 // 00A8
    UINT32 OCPT1_ATOR;                  // 00AC
    UINT32 OCPT1_AADDR;                 // 00B0
    UINT32 OCPT1_ATYPER;                // 00B4
    UINT32 OCPT_CONFIG;                 // 00B8
    UINT32 EMIFF_DLL_LRD_STATUS;        // 00BC
    UINT32 EMIFF_DLL_URD_CONTROL;       // 00C0
    UINT32 EMIFF_DLL_URD_STATUS;        // 00C4
    UINT32 EMIFF_EMRS2;                 // 00C8
    UINT32 EMIFF_DLL_LRD_CONTROL;       // 00CC
    UINT32 OCPT2_PRIOR;                 // 00D0
    UINT32 OCPT2_PTOR1;                 // 00D4
    UINT32 OCPT2_PTOR2;                 // 00D8
    UINT32 OCPT2_PTOR3;                 // 00DC
    UINT32 OCPT2_ATOR;                  // 00E0
    UINT32 OCPT2_AADDR;                 // 00E4
    UINT32 OCPT2_ATYPER;                // 00E8

} OMAP5912_EMIF_REGS;

//------------------------------------------------------------------------------

#endif // __OMAP5912_EMIF_H
