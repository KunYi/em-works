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
//  File:  watchdog_sw_rst.c
//
//  Samsung SMDK6410 SW_RST using watchdog timer support code.
//
#include <windows.h>
#include <ceddk.h>
#include <oal.h>
#include <s3c6410.h>

// WTCON - control register, bit specifications
#define WTCON_PRESCALE(x)   (((x)&0xff)<<8)    // bit 15:8, prescale value, 0 <= (x) <= 27
#define WTCON_ENABLE        (1<<5)            // bit 5, enable watchdog timer
#define WTCON_CLK_DIV16     (0<<3)
#define WTCON_CLK_DIV32     (1<<3)
#define WTCON_CLK_DIV64     (2<<3)
#define WTCON_CLK_DIV128    (3<<3)
#define WTCON_INT_ENABLE    (1<<2)
#define WTCON_RESET         (1<<0)

// WTCNT - watchdog counter register
#define WTCNT_CNT(x)            ((x)&0xffff)

// WTDAT - watchdog reload value register
#define WTDAT_CNT(x)            ((x)&0xffff)

// WTCLRINT - watchdog interrupt clear register
#define WTCLRINT_CLEAR            (1<<0)

// Watchdog Clock
// PCLK : 66MHz
// PCLK/PRESCALER : 66/66 = 1MHz
// PCLK/PRESCALER/DIVIDER : 1MHz/128 = 7.812 KHz
// MAX Counter = 0xffff = 65535
// Period = 65535/7812 =~ 8.4 sec
#define WD_PRESCALER            (66-1)

//------------------------------------------------------------------------------
//
//  Function:  _OEMSWReset
//
//  This is the function to reset S3C6410 using watchdog timer.
//
void _OEMSWReset(void)
{
    volatile S3C6410_WATCHDOG_REG *pWTDogReg = NULL;      // VA for Watchdog base
    pWTDogReg = (S3C6410_WATCHDOG_REG *)OALPAtoVA(S3C6410_BASE_REG_PA_WATCHDOG, FALSE);
    if (!pWTDogReg)
    {
        OALMSG (OAL_ERROR, (L"Address of Watch Dog Base Not Defined, WatchDog not enabled!\r\n"));
    }
    else
    {
        pWTDogReg->WTCON = WTCON_PRESCALE(WD_PRESCALER) | WTCON_CLK_DIV128 | WTCON_RESET;
        pWTDogReg->WTDAT = WTDAT_CNT(0x1);
        pWTDogReg->WTCNT = WTCNT_CNT(0x1);
        pWTDogReg->WTCON |= WTCON_ENABLE;
    }
}

