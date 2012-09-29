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
//
//  Header:  common_wdog.h
//
//  Provides definitions for WDOG (watchdog) module that are common to 
//  Freescale SoCs.
//
//------------------------------------------------------------------------------

#ifndef __COMMON_WDOG_H
#define __COMMON_WDOG_H

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
    UINT16  WCR;
    UINT16  WSR;
    UINT16  WRSR;
} CSP_WDOG_REGS, *PCSP_WDOG_REGS;


//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
#define WDOG_WCR_OFFSET        0x0000
#define WDOG_WSR_OFFSET        0x0002
#define WDOG_WRSR_OFFSET       0x0004


//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
#define WDOG_WCR_WDZST_LSH     0
#define WDOG_WCR_WDBG_LSH      1
#define WDOG_WCR_WDE_LSH       2
#define WDOG_WCR_WRE_LSH       3
#define WDOG_WCR_SRS_LSH       4
#define WDOG_WCR_WDA_LSH       5
#define WDOG_WCR_WOE_LSH       6
#define WDOG_WCR_WT_LSH        8

#define WDOG_WSR_WSR_LSH       0

#define WDOG_WRSR_SFTW_LSH     0
#define WDOG_WRSR_TOUT_LSH     1
#define WDOG_WRSR_CMON_LSH     2
#define WDOG_WRSR_EXT_LSH      3
#define WDOG_WRSR_PWR_LSH      4
#define WDOG_WRSR_JRST_LSH     5

//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
#define WDOG_WCR_WDZST_WID     1
#define WDOG_WCR_WDBG_WID      1
#define WDOG_WCR_WDE_WID       1
#define WDOG_WCR_WRE_WID       1
#define WDOG_WCR_SRS_WID       1
#define WDOG_WCR_WDA_WID       1
#define WDOG_WCR_WOE_WID       1
#define WDOG_WCR_WT_WID        8

#define WDOG_WSR_WSR_WID       16

#define WDOG_WRSR_SFTW_WID     1
#define WDOG_WRSR_TOUT_WID     1
#define WDOG_WRSR_CMON_WID     1
#define WDOG_WRSR_EXT_WID      1
#define WDOG_WRSR_PWR_WID      1
#define WDOG_WRSR_JRST_WID     1

//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------

#define WDOG_WCR_WDZST_CONTINUE    0
#define WDOG_WCR_WDZST_SUSPEND     1

#define WDOG_WCR_WDBG_CONTINUE     0
#define WDOG_WCR_WDBG_SUSPEND      1

#define WDOG_WCR_WDE_DISABLE       0
#define WDOG_WCR_WDE_ENABLE        1

#define WDOG_WCR_WRE_SIG_RESET     0
#define WDOG_WCR_WRE_SIG_TIMEOUT   1

#define WDOG_WCR_SRS_NOEFFECT      1
#define WDOG_WCR_SRS_ASSERT_RESET  0

#define WDOG_WCR_WDA_ASSET_TIMEOUT 0
#define WDOG_WCR_WDA_NOEFFECT      1

#define WDOG_WCR_WOE_TRISTATE      0
#define WDOG_WCR_WOE_OUTPUT_PIN    1

#define WDOG_WSR_WSR_RELOAD1       0x5555
#define WDOG_WSR_WSR_RELOAD2       0xAAAA

#define WDOG_WRSR_SFTW_NORESET     0
#define WDOG_WRSR_SFTW_RESET       1

#define WDOG_WRSR_TOUT_NORESET     0
#define WDOG_WRSR_TOUT_RESET       1

#define WDOG_WRSR_CMON_NORESET     0
#define WDOG_WRSR_CMON_RESET       1

#define WDOG_WRSR_EXT_NORESET      0
#define WDOG_WRSR_EXT_RESET        1

#define WDOG_WRSR_PWR_NORESET      0
#define WDOG_WRSR_PWR_RESET        1

#define WDOG_WRSR_JRST_NORESET     0
#define WDOG_WRSR_JRST_RESET       1

#define WDOG_WCR_WT_MASK           0xFF

#ifdef __cplusplus
}
#endif

#endif // __COMMON_WDOG_H
