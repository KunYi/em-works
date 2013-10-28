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
//  Copyright (C) 2004-2007 Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File: platformreboot.c
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



#define REBOOT_DELAY    3
#define ONE_DAY         (60*60*24)
#define ROLLOVER        ONE_DAY - REBOOT_DELAY

//Externs
extern BOOL OEMGetRealTime(SYSTEMTIME *pTime);
extern BOOL OEMSetRealTime(LPSYSTEMTIME pTime);

//Local function

UINT32 CalculateTime(SYSTEMTIME Time);

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
    SYSTEMTIME SystemTimeVal, *lpSystemTimeVal;
    UINT32 TimeOfTheDay = 0;

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

    if(OEMGetRealTime(&SystemTimeVal) != TRUE)
    {
        OALMSG(OAL_IOCTL&&OAL_FUNC, (L"Couldnot Get System Time\r\n"));
        return FALSE;
    }

    /* Get the time of the day in seconds */

    TimeOfTheDay = CalculateTime(SystemTimeVal);


    if ( TimeOfTheDay >= ROLLOVER)             // if time count is >= 86397
    {
        OALMSG( 1, (L" Rollover condition met\r\n"));

        SystemTimeVal.wSecond = 0;
        SystemTimeVal.wSecond++;                        // avoid a problem rolling over

        SystemTimeVal.wDay++;                         // its the next day
    }
    else
        SystemTimeVal.wSecond += REBOOT_DELAY;          // 86396 + 3 = 86399

    OALMSG( 1, (L" Rollover condition NOT met\r\n"));

    lpSystemTimeVal = &SystemTimeVal;

    if(OEMSetRealTime(lpSystemTimeVal) != TRUE)
    {
        OALMSG( 1, (L"Failed to set system time\r\n"));
    }


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
//
// Function CalculateTime

UINT32 CalculateTime(SYSTEMTIME Time)
{
    return (UINT32)(Time.wHour*60*60 +
            Time.wMinute*60 + Time.wSecond);
}
