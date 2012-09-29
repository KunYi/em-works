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
//  Header:  mx25_wdog.h
//
//  Provides definitions for WDOG (watchdog) module of the MX25 that is slightly different fare common to 
//  Freescale SoCs.
//
//------------------------------------------------------------------------------

#ifndef __MX25_WDOG_H
#define __MX25_WDOG_H

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
    REG16  WCR;
    REG16  WSR;
    REG16  WRSR;
    REG16  WICR;
    REG16  WMCR;
} CSP_WDOG_REGS, *PCSP_WDOG_REGS;


//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
#define WDOG_WCR_WDZST_LSH     0
#define WDOG_WCR_WDBG_LSH      1
#define WDOG_WCR_WDE_LSH       2
#define WDOG_WCR_WDT_LSH       3
#define WDOG_WCR_SRS_LSH       4
#define WDOG_WCR_WDA_LSH       5
#define WDOG_WCR_WDW_LSH       7
#define WDOG_WCR_WT_LSH        8

#define WDOG_WSR_WSR_LSH       0

#define WDOG_WRSR_SFTW_LSH     0
#define WDOG_WRSR_TOUT_LSH     1

//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
#define WDOG_WCR_WDZST_WID     1
#define WDOG_WCR_WDBG_WID      1
#define WDOG_WCR_WDE_WID       1
#define WDOG_WCR_WDT_WID       1
#define WDOG_WCR_SRS_WID       1
#define WDOG_WCR_WDA_WID       1
#define WDOG_WCR_WDW_WID       1
#define WDOG_WCR_WT_WID        8

#define WDOG_WSR_WSR_WID       16

#define WDOG_WRSR_SFTW_WID     1
#define WDOG_WRSR_TOUT_WID     1

//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------

#define WDOG_WCR_WDZST_CONTINUE    0
#define WDOG_WCR_WDZST_SUSPEND     1

#define WDOG_WCR_WDBG_CONTINUE     0
#define WDOG_WCR_WDBG_SUSPEND      1

#define WDOG_WCR_WDE_DISABLE       0
#define WDOG_WCR_WDE_ENABLE        1

#define WDOG_WCR_WDT_NOEFFECT     0
#define WDOG_WCR_WDT_TIMEOUT_ASSERTION  1

#define WDOG_WCR_SRS_NOEFFECT      1
#define WDOG_WCR_SRS_ASSERT_RESET  0

#define WDOG_WCR_WDA_ASSET_TIMEOUT 0
#define WDOG_WCR_WDA_NOEFFECT      1

#define WDOG_WCR_WDW_CONTINUE   0
#define WDOG_WCR_WDW_SUSPEND    1

#define WDOG_WSR_WSR_RELOAD1       0x5555
#define WDOG_WSR_WSR_RELOAD2       0xAAAA

#define WDOG_WRSR_SFTW_NORESET     0
#define WDOG_WRSR_SFTW_RESET       1

#define WDOG_WRSR_TOUT_NORESET     0
#define WDOG_WRSR_TOUT_RESET       1

#define WDOG_WCR_WT_MASK           0xFF

#ifdef __cplusplus
}
#endif

#endif // __MX25_WDOG_H
