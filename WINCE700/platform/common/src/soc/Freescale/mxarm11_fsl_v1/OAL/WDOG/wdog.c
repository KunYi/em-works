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
//  Copyright (C) 2004-2007, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  wdog.c
//
//  PQOAL watchdog timer support.
//
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <nkintr.h>
#pragma warning(pop)

#include "oal_log.h"
#include "oal_memory.h"
#include "mxarm11.h"

//------------------------------------------------------------------------------
// External Functions
UINT32 WdogInit(UINT32 TimeoutMSec);
BOOL WdogService(void);
BOOL WdogDisable(void);
wdogReset_c WdogGetReset(void);
UINT16 OALWdogGetConfig(VOID);


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
    UINT16 wcr;
    
    // Get uncached virtual addresses for Watchdog
    g_pWdog = (pWDOGRegisters_t) OALPAtoUA(CSP_BASE_REG_PA_WDOG);
    if (g_pWdog == NULL) {
        OALMSG(OAL_ERROR, (L"WdogInit:  Watchdog null pointer!\r\n"));
        return 0;
    }

    // Call down to platform-specific code to get WDOG configuration
    wcr = OALWdogGetConfig();
    wcr |= CSP_BITFVAL(WDOG_WCR_WT, WDOG_GET_WT(TimeoutMSec));    

    // Configure and then enable the watchdog
    g_pWdog->WDOGControl = wcr;    
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
// Parameters:
//
// Returns:
//      None.
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
// Parameters:
//
// Returns:
//      None.
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