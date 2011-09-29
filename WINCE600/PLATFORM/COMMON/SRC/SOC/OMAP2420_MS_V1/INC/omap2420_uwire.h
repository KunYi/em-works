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
//  Header:  omap2420_uwire.h
//
#ifndef __OMAP2420_UWIRE_H
#define __OMAP2420_UWIRE_H

//------------------------------------------------------------------------------

typedef volatile struct {
    union {                 // 0x0000
        UINT16 TDR;
        UINT16 RDR;
    };
    UINT16 UNUSED_0002;
    UINT16 CSR;             // 0x0004
    UINT16 UNUSED_0006;
    UINT16 SR1;             // 0x0008
    UINT16 UNUSED_000A;
    UINT16 SR2;             // 0x000C
    UINT16 UNUSED_000E;
    UINT16 SR3;             // 0x0010
    UINT16 UNUSED_0012;
    UINT16 SR4;             // 0x0014
    UINT16 UNUSED_0016;
    UINT16 SR5;             // 0x0018
} OMAP2420_UWIRE_REGS;

//------------------------------------------------------------------------------

#define UWIRE_CSR_RDRB          (1 << 15)
#define UWIRE_CSR_CSRB          (1 << 14)
#define UWIRE_CSR_START         (1 << 13)
#define UWIRE_CSR_CSCMD         (1 << 12)

#define UWIRE_SR_CHK            (1 << 5)
#define UWIRE_SR_CLK_FREQ2      (0 << 3)
#define UWIRE_SR_CLK_FREQ4      (1 << 3)
#define UWIRE_SR_CLK_FREQ8      (2 << 3)
#define UWIRE_SR_CS_LVL         (1 << 2)
#define UWIRE_SR_EDGE_WR        (1 << 1)
#define UWIRE_SR_EDGE_RD        (1 << 0)

#define UWIRE_SR3_CLK_FREQ2     (0 << 1)
#define UWIRE_SR3_CLK_FREQ4     (1 << 1)
#define UWIRE_SR3_CLK_FREQ8     (2 << 1)
#define UWIRE_SR3_CLK_FREQUNDEF (3 << 1)
#define UWIRE_SR3_CLK_EN        (1 << 0)

#define UWIRE_SR4_CLK_INV       (1 << 0)

//------------------------------------------------------------------------------

#endif
