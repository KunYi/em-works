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
// Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  File: reboot.c
//
//  This file implement Freescale MX27 SoC specific OALIoCtlxxxx functions.
//
//------------------------------------------------------------------------------
#include <windows.h>
#include <oal.h>
#include "csp.h"
#include "regs.h"


extern BOOL OALPmicRead(UINT32 addr, PUINT32 pData);
extern BOOL OALPmicWrite(UINT32 addr, UINT32 data);

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

    //
    // If the board design supports software-controllable hardware reset logic,
    // it should be used.  This routine can be overidden in the specific
    // platform code to control board-level reset logic, should it exist.
    //

    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"+OALIoCtlHalReboot\r\n"));

    // this causes the TOD interrupt to fire so we can wake up the MC13783

#if 0
	// this for testing only
//	temp_time = ROLLOVER + 2;				// 86499
//	temp_time = ROLLOVER;					// 86397
//	temp_time = ROLLOVER - 1;				// 86396

//    OALPmicWrite(MC13783_RTC_TM_ADDR, temp_time);
#endif

    OALPmicRead(MC13783_RTC_TM_ADDR, &temp_time);
    OALPmicRead(MC13783_RTC_DAY_ADDR, &temp_day);

    if ( temp_time >= ROLLOVER)             // if time count is >= 86397
    {
        OALMSG( 1, (L" Rollove condition met\r\n"));

        temp_time += REBOOT_DELAY;          // 86397 + 3 = 86400
        temp_time -= ONE_DAY;               // 86400 - 86400 = 0
        temp_time++;						// avoid a problem rolling over

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
    while(TRUE);

    // Should never get to this point...
    //
    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"-OALIoCtlHalReboot\r\n"));

    return(TRUE);

}


//------------------------------------------------------------------------------
