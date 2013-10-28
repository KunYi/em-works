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
//  File:  wdog.c
//
//   This file implements the Watch Dog routines for the Freescale MX27
//   processors.
//
//------------------------------------------------------------------------------
#include <windows.h>
#include <nkintr.h>
#include "oal_log.h"
#include "oal_memory.h"
#include "csp.h"

//------------------------------------------------------------------------------
// External Functions

extern VOID OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX index, 
    DDK_CLOCK_GATE_MODE mode);

UINT32 WdogInit(UINT32 TimeoutMSec);
BOOL WdogService(void);
BOOL WdogDisable(void);
wdogReset_c WdogGetReset(void);


//------------------------------------------------------------------------------
// External Variables

//------------------------------------------------------------------------------
// Defines

#define WDOG_GET_WT(msec)    ((msec/WDOG_TIMEOUT_RES) & WDOG_WCR_WT_MASK)
#define WDOG_TIMEOUT_RES    500    // (4096*4*1000)/32768

//------------------------------------------------------------------------------
// Global Variables

//------------------------------------------------------------------------------
// Local Variables
static pWDOGRegisters_t g_pWdog;

//------------------------------------------------------------------------------
// Local Functions

//-----------------------------------------------------------------------------
//
// Function: WdogInit
//
//  This function asserts a system reset after the desired timeout.
//
// Parameters:
//      timeoutMSec
//          [in] watchdog timeout in msec.
//
// Returns:
//      This function returns the actual timeout period(msec).
//
//-----------------------------------------------------------------------------
UINT32 WdogInit(UINT32 TimeoutMSec)
{
    // Enable the IPG clock input to the WDT module.
    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_WDT, DDK_CLOCK_GATE_MODE_ENABLE);

	// Get uncached virtual addresses for Watchdog
    g_pWdog = (pWDOGRegisters_t) OALPAtoUA(CSP_BASE_REG_PA_WDOG);
    if (g_pWdog == NULL) {
        OALMSG(OAL_ERROR, (L"WdogInit:  Watchdog null pointer!\r\n"));
        return 0;
    }

    // Set timeout period as required. Suspend watchdog timer in powerdown
    // mode. Configure to system reset on timeout.
    g_pWdog->WDOGControl |= CSP_BITFVAL(WDOG_WCR_WT, WDOG_GET_WT(TimeoutMSec)) |
                    CSP_BITFVAL(WDOG_WCR_WDA, WDOG_WCR_WDA_ASSERT_TIMEOUT) |
                    CSP_BITFVAL(WDOG_WCR_WDZST, WDOG_WCR_WDZST_SUSPEND);

    g_pWdog->WDOGControl |= CSP_BITFVAL(WDOG_WCR_WDE, WDOG_WCR_WDE_ENABLE);

    WdogService();

    return (WDOG_GET_WT(TimeoutMSec)*WDOG_TIMEOUT_RES);
}


//-----------------------------------------------------------------------------
//
// Function: WdogService
//
//  This function services the watchdog timer.
//
// Parameters: None
//
// Returns:
//      TRUE on success else FALSE on failure.
//
//-----------------------------------------------------------------------------
BOOL WdogService(void)
{
    if (g_pWdog == NULL) {
        OALMSG(OAL_ERROR, (L"WdogService:  Watchdog not initialized!\r\n"));
        return FALSE;
    }
    // 1. write 0x5555
    g_pWdog->WDOGStatus= WDOG_WSR_WSR_RELOAD1;

    // 2. write 0xAAAA
    g_pWdog->WDOGStatus=  WDOG_WSR_WSR_RELOAD2;

    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: WdogDisable
//
//  This function disables the watchdog timer.
//
// Parameters: None
//
// Returns:
//      TRUE on success else FALSE on failure..
//
//-----------------------------------------------------------------------------
BOOL WdogDisable(void)
{
    if (g_pWdog == NULL) {
        OALMSG(OAL_ERROR, (L"WdogDisable:  Watchdog not initialized!\r\n"));
        return FALSE;
    }

    CSP_BITFCLR(g_pWdog->WDOGControl, WDOG_WCR_WDE);

    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function: WdogGetReset
//
//  This function returns the cause of system reset.
//
// Parameters:
//
// Returns:
//      This function returns the cause of system reset.
//
//-----------------------------------------------------------------------------
wdogReset_c WdogGetReset(void)
{
    wdogReset_c type;
    
    // Get uncached virtual addresses for Watchdog
    g_pWdog = (pWDOGRegisters_t) OALPAtoUA(CSP_BASE_REG_PA_WDOG);
    if (g_pWdog == NULL) {
        OALMSG(OAL_ERROR, (L"WdogGetReset:  Watchdog not initialized!\r\n"));
        return wdogResetUnknown;
    }

    switch(g_pWdog->WDOGResetStatus) {
        case CSP_BITFVAL(WDOG_WRSR_PWR, WDOG_WRSR_PWR_RESET):
            type = wdogResetPowerOn;
            break;

        case CSP_BITFVAL(WDOG_WRSR_EXT, WDOG_WRSR_EXT_RESET):
            type = wdogResetExternal;
            break;

        case CSP_BITFVAL(WDOG_WRSR_TOUT, WDOG_WRSR_TOUT_RESET):
            type = wdogResetTimeout;
            break;

        case CSP_BITFVAL(WDOG_WRSR_SFTW, WDOG_WRSR_SFTW_RESET):
            type = wdogResetSoftware;
            break;

        default:
            type = wdogResetUnknown;
            break;
    }

    return type;
}
