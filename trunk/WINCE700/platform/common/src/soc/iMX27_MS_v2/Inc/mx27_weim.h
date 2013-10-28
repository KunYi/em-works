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
// Copyright (C) 2004, MOTOROLA, INC. All Rights Reserved
//
//------------------------------------------------------------------------------
//
// Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
// Header: mx27_weim.h
//
// Provides definitions for WEIM module on MX27.
//
//------------------------------------------------------------------------------
#ifndef __MX27_WEIM_H__
#define __MX27_WEIM_H__

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------
typedef struct
{
    REG32 CSCR0U;
    REG32 CSCR0L;
    REG32 CSCR0A;
    REG32 _pad0;
    REG32 CSCR1U;
    REG32 CSCR1L;
    REG32 CSCR1A;
    REG32 _pad1;
    REG32 CSCR2U;
    REG32 CSCR2L;
    REG32 CSCR2A;
    REG32 _pad2;
    REG32 CSCR3U;
    REG32 CSCR3L;
    REG32 CSCR3A;
    REG32 _pad3;
    REG32 CSCR4U;
    REG32 CSCR4L;
    REG32 CSCR4A;
    REG32 _pad4;
    REG32 CSCR5U;
    REG32 CSCR5L;
    REG32 CSCR5A;
    REG32 _pad5;
    REG32 WCR;
} CSP_WEIM_REGS, *PCSP_WEIM_REGS;

//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
#define WEIM_CSCR0U_OFFSET                  0x0000
#define WEIM_CSCR0L_OFFSET                  0x0004
#define WEIM_CSCR0A_OFFSET                  0x0008
#define WEIM_CSCR1U_OFFSET                  0x0010
#define WEIM_CSCR1L_OFFSET                  0x0014
#define WEIM_CSCR1A_OFFSET                  0x0018
#define WEIM_CSCR2U_OFFSET                  0x0020
#define WEIM_CSCR2L_OFFSET                  0x0024
#define WEIM_CSCR2A_OFFSET                  0x0028
#define WEIM_CSCR3U_OFFSET                  0x0030
#define WEIM_CSCR3L_OFFSET                  0x0034
#define WEIM_CSCR3A_OFFSET                  0x0038
#define WEIM_CSCR4U_OFFSET                  0x0040
#define WEIM_CSCR4L_OFFSET                  0x0044
#define WEIM_CSCR4A_OFFSET                  0x0048
#define WEIM_CSCR5U_OFFSET                  0x0050
#define WEIM_CSCR5L_OFFSET                  0x0054
#define WEIM_CSCR5A_OFFSET                  0x0058
#define WEIM_WCR_OFFSET                     0x0060

//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // __MX27_WEIM_H__

