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
// Copyright (C) 2005, MOTOROLA, INC. All Rights Reserved
//
//------------------------------------------------------------------------------
//
// Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  watchdog.c
//
//  Interface to OAL watchdog services.
//
#include <windows.h>
#include <ceddk.h>
#include <oal.h>
#include "csp.h"

//------------------------------------------------------------------------------
// External Functions
// For Watchdog Support
UINT32 WdogInit(UINT32 TimeoutMSec);
BOOL WdogService(void);

//------------------------------------------------------------------------------
// Defines

//------------------------------------------------------------------------------
// Local Variables

//------------------------------------------------------------------------------
// Local Functions

//
// kernel exports
//
#define WD_REFRESH_PERIOD               3000    // tell the OS to refresh watchdog every 3 second.
#define WD_RESET_PERIOD                 4500    // tell the wdog to reset the system after 4.5 seconds.

//
// function to refresh watchdog timer
//
void RefreshWatchdogTimer (void)
{
    static BOOL bFirstTime = TRUE;

    OALMSG(OAL_FUNC, (L"+RefreshWatchdogTimer\r\n"));

    if (bFirstTime)
    {
        OALMSG(OAL_FUNC, (L"+RefreshWatchdogTimer: First call, init the WDOG to timeout reset in 4.5 secs\r\n"));
        WdogInit(WD_RESET_PERIOD);
        bFirstTime = FALSE;
    }
    else
    {
        OALMSG(OAL_FUNC, (L"+RefreshWatchdogTimer: Subsequence calls, refresh the WDOG timeout to 4.5 secs again\r\n"));
        WdogService();
    }

    OALMSG(OAL_FUNC, (L"-RefreshWatchdogTimer\r\n"));
}

//------------------------------------------------------------------------------
//
//  Function:  InitWatchDogTimer
//
//  This is the function to enable hardware watchdog timer support by kernel.
//
void InitWatchDogTimer (void)
{
    OALMSG(OAL_FUNC, (L"+InitWatchDogTimer\r\n"));


    pfnOEMRefreshWatchDog = RefreshWatchdogTimer;
    dwOEMWatchDogPeriod   = WD_REFRESH_PERIOD;


    OALMSG(OAL_FUNC, (L"-InitWatchDogTimer\r\n"));
}

//------------------------------------------------------------------------------
