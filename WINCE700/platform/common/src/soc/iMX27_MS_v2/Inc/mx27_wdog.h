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
//  Copyright (C) 2004, Motorola Inc. All Rights Reserved
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
//
//------------------------------------------------------------------------------
//
//	Header:	 MX27_wdog.h
//
//	Provides definitions for watchdog module based on Freescale Mx27 .
//
//------------------------------------------------------------------------------

#ifndef __MX27_WDOG_H
#define __MX27_WDOG_H

#if	__cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------
typedef enum {
    wdogResetUnknown,
    wdogResetSoftware,
    wdogResetTimeout,
    wdogResetExternal,
    wdogResetPowerOn,
} wdogReset_c;

//------------------------------------------------------------------------------
// REGISTER	LAYOUT
//------------------------------------------------------------------------------
typedef struct WDOGRegisters{
    REG16    WDOGControl;        /* 00: WDOG Control Register         */
    REG16    WDOGStatus;          /* 02: WDOG Status Register          */
    REG16    WDOGResetStatus; /* 04: WDOG Reset Status Register */
} WDOGRegisters_t, *pWDOGRegisters_t;


//------------------------------------------------------------------------------
// REGISTER	OFFSETS
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
#define WDOG_WCR_WT_LSH        8

#define WDOG_WSR_WSR_LSH       0

#define WDOG_WRSR_SFTW_LSH     0
#define WDOG_WRSR_TOUT_LSH     1
#define WDOG_WRSR_EXT_LSH      3
#define WDOG_WRSR_PWR_LSH      4

//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
#define WDOG_WCR_WDZST_WID     1
#define WDOG_WCR_WDBG_WID      1
#define WDOG_WCR_WDE_WID       1
#define WDOG_WCR_WRE_WID       1
#define WDOG_WCR_SRS_WID       1
#define WDOG_WCR_WDA_WID       1
#define WDOG_WCR_WT_WID        8

#define WDOG_WSR_WSR_WID       16

#define WDOG_WRSR_SFTW_WID     1
#define WDOG_WRSR_TOUT_WID     1
#define WDOG_WRSR_EXT_WID      1
#define WDOG_WRSR_PWR_WID      1

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

#define WDOG_WCR_WDA_ASSERT_TIMEOUT 0
#define WDOG_WCR_WDA_NOEFFECT      1

#define WDOG_WSR_WSR_RELOAD1       0x5555
#define WDOG_WSR_WSR_RELOAD2       0xAAAA

#define WDOG_WRSR_SFTW_NORESET     0
#define WDOG_WRSR_SFTW_RESET       1

#define WDOG_WRSR_TOUT_NORESET     0
#define WDOG_WRSR_TOUT_RESET       1

#define WDOG_WRSR_EXT_NORESET      0
#define WDOG_WRSR_EXT_RESET        1

#define WDOG_WRSR_PWR_NORESET      0
#define WDOG_WRSR_PWR_RESET        1

#define WDOG_WCR_WT_MASK           0xFF

#ifdef __cplusplus
}
#endif

#endif // __MX27_WDOG_H
