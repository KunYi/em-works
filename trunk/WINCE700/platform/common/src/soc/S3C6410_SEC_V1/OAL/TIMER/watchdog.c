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
//  File:  watchdog.c
//
//  Samsung SMDK6410 watchdog timer support code.
//
#include <windows.h>
#include <ceddk.h>
#include <oal.h>

// Base Definitions
#include "s3c6410_base_regs.h"

// SoC Components
#include "s3c6410_wdog.h"


// WTCON - control register, bit specifications
#define WTCON_PRESCALE(x)        (((x)&0xff)<<8)    // bit 15:8, prescale value, 0 <= (x) <= 27
#define WTCON_ENABLE            (1<<5)            // bit 5, enable watchdog timer
#define WTCON_CLK_DIV16        (0<<3)
#define WTCON_CLK_DIV32        (1<<3)
#define WTCON_CLK_DIV64        (2<<3)
#define WTCON_CLK_DIV128        (3<<3)
#define WTCON_INT_ENABLE        (1<<2)
#define WTCON_RESET            (1<<0)

// WTCNT - watchdog counter register
#define WTCNT_CNT(x)            ((x)&0xffff)

// WTDAT - watchdog reload value register
#define WTDAT_CNT(x)            ((x)&0xffff)

// WTCLRINT - watchdog interrupt clear register
#define WTCLRINT_CLEAR            (1<<0)

// Watchdog Clock
// PCLK : 25MHz
// PCLK/PRESCALER : 25/25 = 1MHz
// PCLK/PRESCALER/DIVIDER : 1MHz/128 = 7.812 KHz
// MAX Counter = 0xffff = 65535
// Period = 65535/7812 =~ 8.4 sec
#define WD_PRESCALER            (25-1)

#define WD_REFRESH_PERIOD        3000    // tell the OS to refresh watchdog every 3 second.


//
// function to refresh watchdog timer
//
void RefreshWatchdogTimer (void)
{
    static volatile S3C6410_WATCHDOG_REG *pWTDogReg = 0;      // VA for Watchdog base

    if (!pWTDogReg)
    {
        // called the 1st time, setup the watchdog timer
        pWTDogReg = (S3C6410_WATCHDOG_REG *)OALPAtoVA(S3C6410_BASE_REG_PA_WATCHDOG, FALSE);
        if (!pWTDogReg)
        {
            OALMSG (OAL_ERROR, (L"Address of Watch Dog Base Not Defined, WatchDog not enabled!\r\n"));
        }
        else
        {
            pWTDogReg->WTCNT = WTCNT_CNT(0xFFFF);
            pWTDogReg->WTCON = WTCON_PRESCALE(WD_PRESCALER) | WTCON_ENABLE | WTCON_CLK_DIV128 | WTCON_RESET;
        }
    }
    else
    {
        // subsequent refresh calls, just reset the counter register to max value
        pWTDogReg->WTCNT = WTCNT_CNT(0xFFFF);
    }
}

//------------------------------------------------------------------------------
//
//  Function:  OALInitWatchDogTimer
//
//  This is the function to enable hardware watchdog timer support by kernel.
//
void OALInitWatchDogTimer (void)
{
    OALMSG(OAL_FUNC, (L"+SMDKInitWatchDogTimer\r\n"));

    pfnOEMRefreshWatchDog    = RefreshWatchdogTimer;
    dwOEMWatchDogPeriod    = WD_REFRESH_PERIOD;

    OALMSG(OAL_FUNC, (L"-SMDKInitWatchDogTimer\r\n"));
}

//------------------------------------------------------------------------------
