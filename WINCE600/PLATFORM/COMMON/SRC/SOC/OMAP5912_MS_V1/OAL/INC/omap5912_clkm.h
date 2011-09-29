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
//  File:  omap5912_clkm.h
//
#ifndef __OMAP5912_CLKM_H
#define __OMAP5912_CLKM_H

//------------------------------------------------------------------------------

typedef volatile struct {
    UINT16 CKCTL;               // 0000
    UINT16 RESERVED_0002;       // 0002
    UINT16 IDLECT1;             // 0004
    UINT16 RESERVED_0006;       // 0006
    UINT16 IDLECT2;             // 0008
    UINT16 RESERVED_000A;       // 000A
    UINT16 EWUPCT;              // 000C
    UINT16 RESERVED_000E;       // 000E
    UINT16 RSTCT1;              // 0010
    UINT16 RESERVED_0012;       // 0012
    UINT16 RSTCT2;              // 0014
    UINT16 RESERVED_0016;       // 0016
    UINT16 SYSST;               // 0018
    UINT16 RESERVED_001A;       // 001A
    UINT16 CKOUT1;              // 001C
    UINT16 RESERVED_001E;       // 001E
    UINT16 CKOUT2;              // 0020
    UINT16 RESERVED_0022;       // 0022
    UINT16 IDLECT3;             // 0024
    UINT16 RESERVED_0026[109];  // 0026
    UINT16 DPLL1_CTL;           // 0100
    UINT16 RESERVED_0102;       // 0102
} OMAP5912_CLKM_REGS;

//------------------------------------------------------------------------------

#define IDLWDT_ARM              (1 << 0)
#define IDLXORP_ARM             (1 << 1)
#define IDLPER_ARM              (1 << 2)
#define IDLIF_ARM               (1 << 6)
#define IDLDPLL_ARM             (1 << 7)
#define WKUP_MODE               (1 << 10)
#define IDLCLKOUT_ARM           (1 << 12)

#define EN_WDTCK                (1 << 0)
#define EN_XORPCK               (1 << 1)
#define EN_PERCK                (1 << 2)
#define EN_LCDCK                (1 << 3)
#define EN_APICK                (1 << 6)
#define EN_TIMCK                (1 << 7)
#define DMACK_REQ               (1 << 8)
#define EN_CKOUT_ARM            (1 << 11)

#define EN_OCPI_CK              (1 << 0)
#define IDLOCPI_ARM             (1 << 1)
#define EN_TC1_CK               (1 << 2)
#define IDLTC1_ARM              (1 << 3)
#define EN_TC2_CK               (1 << 4)
#define IDLTC2_ARM              (1 << 5)

#define RSTCT1_SW_RST           (1 << 3)
#define RSTCT1_ARM_RST          (1 << 0)

#define DPLL1_CTL_PLL_ENABLE    (1 << 4)
#define DPLL1_CTL_BYPASS_DIV1   (0 << 2)
#define DPLL1_CTL_BYPASS_DIV2   (1 << 2)
#define DPLL1_CTL_BYPASS_DIV4   (2 << 2)

//------------------------------------------------------------------------------

#endif // __OMAP5912_CLKM_H
