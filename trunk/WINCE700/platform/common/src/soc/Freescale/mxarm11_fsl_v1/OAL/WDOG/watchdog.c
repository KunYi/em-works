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
//  File:  watchdog.c
//
//  Freescale ARM11 family watchdog timer support code.
//
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <ceddk.h>
#include <oal.h>
#pragma warning(pop)

#include "mxarm11.h"

#if (_WINCEOSVER<700)
//
// kernel exports
//
extern DWORD   dwNKWatchDogThreadPriority;      // watchdog thread priority, default is 100, set by kernel. OEM can adjust as desired
#endif

// For Watchdog Support
UINT32 WdogInit(UINT32 TimeoutMSec);
BOOL WdogService(void);

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
        OALMSG(OAL_FUNC, (L"+RefreshWatchdogTimer: First call, init the Wdog to timeout reset in 4.5 secs\r\n"));
        WdogInit(WD_RESET_PERIOD);
        bFirstTime = FALSE;
    }
    else
    {
        OALMSG(OAL_FUNC, (L"+RefreshWatchdogTimer: Subsequence calls, refresh the Wdog timeout to 4.5 secs again\r\n"));
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
