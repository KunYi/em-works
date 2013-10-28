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
//  Copyright (C) 2004,	MOTOROLA, INC. All Rights Reserved
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
//
//------------------------------------------------------------------------------
//
//  Header:  mx27_owire.h
//
//  Provides definitions for one-wire module based on Freescale ARM9 chassis.
//
//------------------------------------------------------------------------------
#ifndef __MX27_OWIRE_H
#define __MX27_OWIRE_H

#ifdef __cplusplus
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
     UINT16 CR;
     UINT16 TD;
     UINT16 RST;
} CSP_OWIRE_REGS, *PCSP_OWIRE_REGS;


//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
#define OWIRE_CR_OFFSET          0x0000
#define OWIRE_TD_OFFSET          0x0002
#define OWIRE_RST_OFFSET         0x0004


//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
#define OWIRE_CR_RDST_LSH        3
#define OWIRE_CR_WR1RD_LSH       4
#define OWIRE_CR_WR0_LSH         5
#define OWIRE_CR_PST_LSH         6
#define OWIRE_CR_RPP_LSH         7

#define OWIRE_TD_DVDR_LSH        0

#define OWIRE_RST_RST_LSH        0


//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
#define OWIRE_CR_RDST_WID        1
#define OWIRE_CR_WR1RD_WID       1
#define OWIRE_CR_WR0_WID         1
#define OWIRE_CR_PST_WID         1
#define OWIRE_CR_RPP_WID         1

#define OWIRE_TD_DVDR_WID        8

#define OWIRE_RST_RST_WID        1


//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------

// OWIRE Control register
#define OWIRE_CR_WR1RD_WRITE             1 // Write a 1 bit to the interface
#define OWIRE_CR_WR1RD_DONE              0 // Write 1 completed

#define OWIRE_CR_WR0_WRITE               1 // Write a 0 bit to the interface
#define OWIRE_CR_WR0_DONE                0 // Write 0 completed

#define OWIRE_CR_PST_PRESENT             1 // Device is present
#define OWIRE_CR_PST_NOTPRESENT          0 // Device is not present

#define OWIRE_CR_RPP_RESET               1 // Generate Reset Pulse and sample
                                           // for DS2502 presence pulse
#define OWIRE_CR_RPP_DONE                0 // Reset pulse complete

// OWIRE Reset register
#define OWIRE_RST_RST_RESET              1 // One-wire is in reset
#define OWIRE_RST_RST_ENDRESET           0 // One-wire is out of reset

#ifdef __cplusplus
}
#endif

#endif // __MX27_OWIRE_H
