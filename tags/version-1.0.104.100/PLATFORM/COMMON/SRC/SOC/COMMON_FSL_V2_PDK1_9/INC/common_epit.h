//------------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//  Header:  common_epit.h
//
//  Provides definitions for the EPIT (Enhanced Periodic Interrupt Timer) 
//  module that are common to Freescale SoCs.
//
//------------------------------------------------------------------------------
#ifndef __COMMON_EPIT_H
#define __COMMON_EPIT_H

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
     UINT32 CR;
     UINT32 SR;
     UINT32 LR;
     UINT32 CMPR;
     UINT32 CNT;
} CSP_EPIT_REG, *PCSP_EPIT_REG;


//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
#define EPIT_CR_OFFSET          0x0000
#define EPIT_SR_OFFSET          0x0004
#define EPIT_LR_OFFSET          0x0008
#define EPIT_CMPR_OFFSET        0x000C
#define EPIT_CNR_OFFSET         0x0010


//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
#define EPIT_CR_EN_LSH          0
#define EPIT_CR_ENMOD_LSH       1
#define EPIT_CR_OCIEN_LSH       2
#define EPIT_CR_RLD_LSH         3
#define EPIT_CR_PRESCALAR_LSH   4
#define EPIT_CR_SWR_LSH         16
#define EPIT_CR_IOVW_LSH        17
#define EPIT_CR_DBGEN_LSH       18
#define EPIT_CR_WAITEN_LSH      19
#define EPIT_CR_DOZEN_LSH       20
#define EPIT_CR_STOPEN_LSH      21
#define EPIT_CR_OM_LSH          22
#define EPIT_CR_CLKSRC_LSH      24

#define EPIT_SR_OCIF_LSH        0

#define EPIT_LR_LOAD_LSH        0

#define EPIT_CMPR_COMPARE_LSH   0

#define EPIT_CNT_COUNT_LSH      0


//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
#define EPIT_CR_EN_WID          1
#define EPIT_CR_ENMOD_WID       1
#define EPIT_CR_OCIEN_WID       2
#define EPIT_CR_RLD_WID         1
#define EPIT_CR_PRESCALAR_WID   12
#define EPIT_CR_SWR_WID         1
#define EPIT_CR_IOVW_WID        1
#define EPIT_CR_DBGEN_WID       1
#define EPIT_CR_WAITEN_WID      1
#define EPIT_CR_DOZEN_WID       1
#define EPIT_CR_STOPEN_WID      1
#define EPIT_CR_OM_WID          2
#define EPIT_CR_CLKSRC_WID      2

#define EPIT_SR_OCIF_WID        1

#define EPIT_LR_LOAD_WID        32

#define EPIT_CMPR_COMPARE_WID   32

#define EPIT_CNT_COUNT_WID      32


//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------

// CR
#define EPIT_CR_EN_DISABLE          0
#define EPIT_CR_EN_ENABLE           1

#define EPIT_CR_ENMOD_RESUME        0
#define EPIT_CR_ENMOD_LOAD          1

#define EPIT_CR_OCIEN_DISABLE       0
#define EPIT_CR_OCIEN_ENABLE        1

#define EPIT_CR_RLD_ROLLOVER        0
#define EPIT_CR_RLD_RELOAD          1

#define EPIT_CR_SWR_NORESET         0
#define EPIT_CR_SWR_RESET           1

#define EPIT_CR_IOVW_NOOVR          0
#define EPIT_CR_IOVW_OVR            1

#define EPIT_CR_DBGEN_INACTIVE      0
#define EPIT_CR_DBGEN_ACTIVE        1

#define EPIT_CR_WAITEN_DISABLE      0
#define EPIT_CR_WAITEN_ENABLE       1

#define EPIT_CR_DOZEN_DISABLE       0
#define EPIT_CR_DOZEN_ENABLE        1

#define EPIT_CR_STOPEN_DISABLE      0
#define EPIT_CR_STOPEN_ENABLE       1

#define EPIT_CR_OM_DICONNECT        0
#define EPIT_CR_OM_TOGGLE           1
#define EPIT_CR_OM_CLEAR            2
#define EPIT_CR_OM_SET              3

#define EPIT_CR_CLKSRC_OFF          0
#define EPIT_CR_CLKSRC_IPGCLK       1
#define EPIT_CR_CLKSRC_HIGHFREQ     2   // High freq is sourcing from PERCLK
#define EPIT_CR_CLKSRC_CKIL         3

// CNT
#define EPIT_CNT_COUNT_MAX          0xFFFFFFFF

#ifdef __cplusplus
}
#endif

#endif // __COMMON_EPIT_H
