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
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header:  common_dpll.h
//
//  Provides definitions for the DPLL (Digital Phase Lock Loop) 
//  module that are common to Freescale SoCs.
//
//------------------------------------------------------------------------------
#ifndef __COMMON_DPLL_H
#define __COMMON_DPLL_H

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
    UINT32 DP_CTL;
    UINT32 DP_CONFIG;
    UINT32 DP_OP;
    UINT32 DP_MFD;
    UINT32 DP_MFN;
    UINT32 DP_MFN_MINUS;
    UINT32 DP_MFN_PLUS;
    UINT32 DP_HFS_OP;
    UINT32 DP_HFS_MFD;
    UINT32 DP_HFS_MFN;
    UINT32 DP_MFN_TOGC;
    UINT32 DP_DESTAT;
} CSP_DPLL_REGS, *PCSP_DPLL_REGS;


//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
#define DPLL_DP_CTL                     0x0000
#define DPLL_DP_CONFIG                  0x0004
#define DPLL_DP_OP                      0x0008
#define DPLL_DP_MFD                     0x000C
#define DPLL_DP_MFN                     0x0010
#define DPLL_DP_MFN_MINUS               0x0014
#define DPLL_DP_MFN_PLUS                0x0018
#define DPLL_DP_HFS_OP                  0x001C
#define DPLL_DP_HFS_MFD                 0x0020
#define DPLL_DP_HFS_MFN                 0x0024
#define DPLL_DP_MFN_TOGC                0x0028
#define DPLL_DP_DESTAT                  0x002C


//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
#define DPLL_DP_CTL_LRF_LSH             0
#define DPLL_DP_CTL_BRMO_LSH            1
#define DPLL_DP_CTL_PLM_LSH             2
#define DPLL_DP_CTL_RCP_LSH             3
#define DPLL_DP_CTL_RST_LSH             4
#define DPLL_DP_CTL_UPEN_LSH            5
#define DPLL_DP_CTL_PRE_LSH             6
#define DPLL_DP_CTL_HFSM_LSH            7
#define DPLL_DP_CTL_REF_CLK_SEL_LSH     8
#define DPLL_DP_CTL_REF_CLK_DIV_LSH     10
#define DPLL_DP_CTL_ADE_LSH             11
#define DPLL_DP_CTL_DPDCK0_2_EN_LSH     12
#define DPLL_DP_CTL_MUL_CTL_LSH         13

#define DPLL_DP_CONFIG_LDREQ_LSH        0
#define DPLL_DP_CONFIG_AREN_LSH         1
#define DPLL_DP_OP_PDF_LSH              0
#define DPLL_DP_OP_MFI_LSH              4

//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
#define DPLL_DP_CTL_LRF_WID             1
#define DPLL_DP_CTL_BRMO_WID            1
#define DPLL_DP_CTL_PLM_WID             1
#define DPLL_DP_CTL_RCP_WID             1
#define DPLL_DP_CTL_RST_WID             1
#define DPLL_DP_CTL_UPEN_WID            1
#define DPLL_DP_CTL_PRE_WID             1
#define DPLL_DP_CTL_HFSM_WID            1
#define DPLL_DP_CTL_REF_CLK_SEL_WID     2
#define DPLL_DP_CTL_REF_CLK_DIV_WID     1
#define DPLL_DP_CTL_ADE_WID             1
#define DPLL_DP_CTL_DPDCK0_2_EN_WID     1
#define DPLL_DP_CTL_MUL_CTL_WID         1

#define DPLL_DP_CONFIG_LDREQ_WID        1
#define DPLL_DP_CONFIG_AREN_WID         1
#define DPLL_DP_OP_PDF_WID              4
#define DPLL_DP_OP_MFI_WID              4


//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------
#define DPLL_DP_CTL_LRF_DPLL_NO_LOCKED  0
#define DPLL_DP_CTL_LRF_DPLL_LOCKED     1

#define DPLL_DP_CTL_RST_REST_CLEARED    0
#define DPLL_DP_CTL_RST_RESET_ASSERT    1

#define DPLL_DP_CTL_HFSM_HFS_ACTIVE     1
#define DPLL_DP_CTL_HFSM_LFS_ACTIVE     0

#ifdef __cplusplus
}
#endif

#endif // __COMMON_DPLL_H
