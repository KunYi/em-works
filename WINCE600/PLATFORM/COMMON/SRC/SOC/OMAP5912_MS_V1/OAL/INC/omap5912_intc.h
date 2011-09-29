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
//  Header:  omap5912_intc.h
//
#ifndef __OMAP5912_INTC_H
#define __OMAP5912_INTC_H

//------------------------------------------------------------------------------

typedef volatile struct {
    UINT32 ITR;                 // 0000
    UINT32 MIR;                 // 0004
    UINT32 UNUSED0008[2];       // 0008
    UINT32 SIR_IRQ;             // 0010
    UINT32 SIR_FIQ;             // 0014
    UINT32 CNTL;                // 0018
    UINT32 ILR[32];             // 001C
    UINT32 SIR;                 // 009C
    union {                     // 00A0
        UINT32 ENH_CNTL;
        UINT32 STATUS;
    };
    UINT32 OCP_CFG;             // 00A4
    UINT32 INTH_REV;            // 00A8
} OMAP5912_INTC_REGS;

//------------------------------------------------------------------------------

#define ILR_IRQ                 (0 << 0)
#define ILR_FIQ                 (1 << 0)
#define ILR_EDGE                (0 << 1)
#define ILR_LEVEL               (1 << 1)
#define ILR_PRI0                (0 << 2)
#define ILR_PRI1                (1 << 2)
#define ILR_PRI2                (2 << 2)
#define ILR_PRI3                (3 << 2)
#define ILR_PRI4                (4 << 2)
#define ILR_PRI5                (5 << 2)
#define ILR_PRI6                (6 << 2)
#define ILR_PRI7                (7 << 2)
#define ILR_PRI8                (8 << 2)
#define ILR_PRI9                (9 << 2)
#define ILR_PRI10               (10 << 2)
#define ILR_PRI11               (11 << 2)
#define ILR_PRI12               (12 << 2)
#define ILR_PRI13               (13 << 2)
#define ILR_PRI14               (14 << 2)
#define ILR_PRI15               (15 << 2)
#define ILR_PRI16               (16 << 2)
#define ILR_PRI17               (17 << 2)
#define ILR_PRI18               (18 << 2)
#define ILR_PRI19               (19 << 2)
#define ILR_PRI20               (20 << 2)
#define ILR_PRI21               (21 << 2)
#define ILR_PRI22               (22 << 2)
#define ILR_PRI23               (23 << 2)
#define ILR_PRI24               (24 << 2)
#define ILR_PRI25               (25 << 2)
#define ILR_PRI26               (26 << 2)
#define ILR_PRI27               (27 << 2)
#define ILR_PRI28               (28 << 2)
#define ILR_PRI29               (29 << 2)
#define ILR_PRI30               (30 << 2)
#define ILR_PRI31               (31 << 2)
 
//------------------------------------------------------------------------------

#define CNTL_NEW_IRQ            (1 << 0)
#define CNTL_NEW_FIQ            (1 << 1)

//------------------------------------------------------------------------------

#endif // __OMA5912_INTC_H
