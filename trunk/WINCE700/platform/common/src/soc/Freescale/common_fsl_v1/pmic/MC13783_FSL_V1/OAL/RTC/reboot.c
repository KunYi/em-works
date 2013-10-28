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
//  File: reboot.c
//
//  This file implement Freescale ARM11 SoC family OALIoCtlxxxx functions.
//
//------------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <oal.h>
#pragma warning(pop)

#include "mxarm11.h"
#include "regs.h"

extern BOOL OALPmicRead(UINT32 addr, PUINT32 pData);
extern BOOL OALPmicWrite(UINT32 addr, UINT32 data);
extern BOOL OALPmicWriteMasked(UINT32 addr, UINT32 mask, UINT32 data);

#define REBOOT_DELAY    3
#define ONE_DAY         (60*60*24)
#define ROLLOVER        ONE_DAY - REBOOT_DELAY

// For Watchdog Support
UINT32 WdogInit(UINT32 TimeoutMSec);

//------------------------------------------------------------------------------
//
//  Function: OALIoCtlHalReboot
//
//
BOOL OALIoCtlHalReboot(UINT32 code, VOID *pInpBuffer,
                       UINT32 inpSize, VOID *pOutBuffer,
                       UINT32 outSize, UINT32 *pOutSize)
{
    UINT32 temp_time, temp_day;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(code);
    UNREFERENCED_PARAMETER(pInpBuffer);
    UNREFERENCED_PARAMETER(inpSize);
    UNREFERENCED_PARAMETER(pOutBuffer);
    UNREFERENCED_PARAMETER(outSize);
    UNREFERENCED_PARAMETER(pOutSize);

    //
    // If the board design supports software-controllable hardware reset logic,
    // it should be used.  This routine can be overidden in the specific
    // platform code to control board-level reset logic, should it exist.
    //

    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"+OALIoCtlHalReboot\r\n"));

    // this causes the TOD interrupt to fire so we can wake up the MC13783

    OALPmicRead(MC13783_RTC_TM_ADDR, &temp_time);
    OALPmicRead(MC13783_RTC_DAY_ADDR, &temp_day);

    if ( temp_time >= ROLLOVER)             // if time count is >= 86397
    {
        OALMSG( 1, (L" Rollove condition met\r\n"));

        temp_time += REBOOT_DELAY;          // 86397 + 3 = 86400
        temp_time -= ONE_DAY;               // 86400 - 86400 = 0
        temp_time++;                        // avoid a problem rolling over

        temp_day++;                         // its the next day
    }
    else
        temp_time += REBOOT_DELAY;          // 86396 + 3 = 86399

    OALMSG( 1, (L" Rollove condition NOT met\r\n"));

    OALPmicWrite(MC13783_RTC_DAY_ALM_ADDR, temp_day );
    OALPmicWrite(MC13783_RTC_ALM_ADDR, temp_time);

    // Set watchdog timer to timeout in 0.5 sec
    WdogInit(500);

    // Wait for reset...
    //
    for (;;);

    // Should never get to this point...
    //
#if 0 // Remove-W4: Warning C4702 workaround
    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"-OALIoCtlHalReboot\r\n"));
    return(TRUE);
#endif
}

//------------------------------------------------------------------------------
