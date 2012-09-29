//------------------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
//  Use of this source code is subject to the terms of the Microsoft end-user
//  license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
//  If you did not accept the terms of the EULA, you are not authorized to use
//  this source code. For a copy of the EULA, please see the LICENSE.RTF on your
//  install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header:  common_owire.h
//
//  Provides definitions for one-wire module based on Freescale i.MX chassis.
//
//------------------------------------------------------------------------------
#ifndef __COMMON_OWIRE_H
#define __COMMON_OWIRE_H

#ifdef __cplusplus
extern "C" {
#endif


//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------
#define OWIRE_TIMEOUT            10000 // 10s for timeout
#define OWIRE_READ               0x00FF

//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------
typedef struct
{
     UINT16 CR;
     UINT16 TD;
     UINT16 RST;
     UINT16 CMD;
     UINT16 TRX;
     UINT16 INTR;
     UINT16 IER;
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
                                           // for owire device presence pulse
#define OWIRE_CR_RPP_DONE                0 // Reset pulse complete

// OWIRE Reset register
#define OWIRE_RST_RST_RESET              1 // One-wire is in reset
#define OWIRE_RST_RST_ENDRESET           0 // One-wire is out of reset

// OWIRE Interrupt flag
#define OWIRE_PDR                        0x02
#define OWIRE_TSRE                       0x08
#define OWIRE_RBF                        0x10

#define OWIRE_INT_CLEAR                  0x0000
#define OWIRE_EPD_INT_EN                 0x0001
#define OWIRE_ETSE_INT_EN                0x0008
#define OWIRE_ERBF_INT_EN                0x0010
#ifdef __cplusplus
}
#endif

#endif // __COMMON_OWIRE_H
