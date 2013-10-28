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
//  Copyright (C) 2004-2007, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header:  mxarm11_rnga.h
//
//  Provides definitions for RNGA module based on Freescale ARM11 chassis.
//
//------------------------------------------------------------------------------
#ifndef __MXARM11_RNGA_H
#define __MXARM11_RNGA_H

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------

typedef struct 
{
    UINT32 RNG_CR;
    UINT32 RNG_SR;
    UINT32 RNG_ENT;
    UINT32 RNG_OFIFO;
    // The following registers are not available when the RGNA is in secure mode.
    UINT32 RNG_MOD;
    UINT32 RNG_VER;
    UINT32 RNG_OSCCR;
    UINT32 RNG_OSC1CT;
    UINT32 RNG_OSC2CT;
    UINT32 RNG_OSCSTAT;
} CSP_RNGA_REGS, *PCSP_RNGA_REGS;


//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
#define RNG_CR_OFFSET        0x0000
#define RNG_SR_OFFSET        0x0004
#define RNG_ENT_OFFSET       0x0008
#define RNG_OFIFO_OFFSET     0x000C
#define RNG_MOD_OFFSET       0x0010
#define RNG_VER_OFFSET       0x0014
#define RNG_OSCCR_OFFSET     0x0018
#define RNG_OSC1CT_OFFSET    0x001C
#define RNG_OSC2CT_OFFSET    0x0020
#define RNG_OSCSTAT_OFFSET   0x0024

//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
#define RNG_CR_GO_LSH              0
#define RNG_CR_HASSURE_LSH         1
#define RNG_CR_INTMASK_LSH         2
#define RNG_CR_CLRINT_LSH          3
#define RNG_CR_SLEEP_LSH           4


#define RNG_SR_SECVIOLATION_LSH    0
#define RNG_SR_LASTRDSTAT_LSH      1
#define RNG_SR_FIFOUNDER_LSH       2
#define RNG_SR_ERRINT_LSH          3
#define RNG_SR_SLEEP_LSH           4
#define RNG_SR_FIFOLEVEL_LSH       8
#define RNG_SR_FIFOSIZE_LSH       16
#define RNG_SR_OSCDEAD_LSH        31


#define RNG_MOD_VERIFY_LSH         0
#define RNG_MOD_OSCFQTEST_LSH      1


#define RNG_VER_SHFTCLKOFF_LSH     0
#define RNG_VER_FORCSYSCLK_LSH     1
#define RNG_VER_RSTSHFTREG_LSH     2


#define RNG_OSCCR_NUMCLK_LSH       0


#define RNG_OSC1CT_NUMCLKRXD_LSH   0


#define RNG_OSC2CT_NUMCLKRXD_LSH   0


#define RNG_OSCSTAT_OSC1_LSH       0
#define RNG_OSCSTAT_OSC2_LSH       1


//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
#define RNG_CR_GO_WID              1
#define RNG_CR_HASSURE_WID         1
#define RNG_CR_INTMASK_WID         1
#define RNG_CR_CLRINT_WID          1
#define RNG_CR_SLEEP_WID           1


#define RNG_SR_SECVIOLATION_WID    1
#define RNG_SR_LASTRDSTAT_WID      1
#define RNG_SR_FIFOUNDER_WID       1
#define RNG_SR_ERRINT_WID          1
#define RNG_SR_SLEEP_WID           1
#define RNG_SR_FIFOLEVEL_WID       8
#define RNG_SR_FIFOSIZE_WID        8
#define RNG_SR_OSCDEAD_WID         1


#define RNG_MOD_OSCFQTEST_WID      1
#define RNG_MOD_VERIFY_WID         1


#define RNG_VER_RSTSHFTREG_WID     1
#define RNG_VER_FORCSYSCLK_WID     1
#define RNG_VER_SHFTCLKOFF_WID     1


#define RNG_OSCCR_NUMCLKSET_WID    1


#define RNG_OSC1CT_NUMCLKRXD_WID  20


#define RNG_OSC2CT_NUMCLKRXD_WID  20


#define RNG_OSCSTAT_OSC1_WID       1
#define RNG_OSCSTAT_OSC2_WID       1

//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------
#define RNG_CR_GO_SET              1
#define RNG_CR_GO_CLEAR            0
                                  
#define RNG_CR_HASSURE_ENABLE      1
#define RNG_CR_HASSURE_DISABLE     0

#define RNG_CR_INTMASK_MASK        1
#define RNG_CR_INTMASK_ENABLE      0
                                    

#define RNG_CR_CLRINT_CLEAR        1

#define RNG_CR_SLEEP_SET           1
#define RNG_CR_SLEEP_CLEAR         0
#define RNG_MOD_OSCFQTEST_ENABLE   1
#define RNG_MOD_OSCFQTEST_DISABLE  0

#define RNG_MOD_VERIFY_ENABLE      1
#define RNG_MOD_VERIFY_DISABLE     0

#define RNG_VER_RSTSHFTREG_SET     1
 
#define RNG_VER_FORCSYSCLK_SET     1
#define RNG_VER_FORCSYSCLK_CLEAR   0

#define RNG_VER_SHFTCLKOFF_SET     1
#define RNG_VER_SHFTCLKOFF_CLEAR   0




//------------------------------------------------------------------------------
// Functions
//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // __MXARM11_RNGA_H
