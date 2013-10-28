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
// -----------------------------------------------------------------------------
//
//      THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//      ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//      THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//      PARTICULAR PURPOSE.
//  
// -----------------------------------------------------------------------------

// Abstract:  
//   This file implements the NK kernel interfaces for the real time clock.


#include <windows.h>
#include <nkintr.h>
#include <pc.h>
#include <oal.h>

// NOTE: A problem has been found with some chipsets such that
// setting the time to 23:59:59 on the 29th or 30th day of a month which
// has less than 31 days causes the clock to roll over incorrectly.
// Uncomment the following line to fix this problem.  However, be aware 
// that the fix consists of responding to calls that set the time to 
// HH:MM:59 by instead setting the time to HH:MM:58.
//#define HARDWARE_TIME_SET_PROBLEM 1

// Global CS, which is initialized in oeminit
static CRITICAL_SECTION RTC_critsect;

BOOL Bare_SetAlarmTime(LPSYSTEMTIME lpst);
BOOL Bare_SetRealTime(LPSYSTEMTIME lpst);
BOOL Bare_GetRealTime(LPSYSTEMTIME lpst);

void RTCPostInit()
{
    InitializeCriticalSection(&RTC_critsect);
}

BOOL OEMGetRealTime(
                    __out LPSYSTEMTIME lpst
                    )
{
    BOOL RetVal;

    EnterCriticalSection(&RTC_critsect);
    RetVal = Bare_GetRealTime(lpst);
    LeaveCriticalSection(&RTC_critsect);

    return RetVal;
}

BOOL OEMSetRealTime(
                    __in LPSYSTEMTIME lpst
                    )
{
    BOOL RetVal;

    EnterCriticalSection(&RTC_critsect);
    RetVal = Bare_SetRealTime(lpst);
    LeaveCriticalSection(&RTC_critsect);
    
    return RetVal;
}

BOOL OEMSetAlarmTime(
                     __in LPSYSTEMTIME lpst
                     )
{
    BOOL RetVal;

    EnterCriticalSection(&RTC_critsect);
    RetVal = Bare_SetAlarmTime(lpst);
    LeaveCriticalSection(&RTC_critsect);
    
    return RetVal;
}

BOOL x86IoCtlHalInitRTC(
                        UINT32 code, 
                        __in_bcount(nInBufSize) void *lpInBuf, 
                        UINT32 nInBufSize, 
                        __out_bcount(nOutBufSize) void *lpOutBuf, 
                        UINT32 nOutBufSize, 
                        __out UINT32 *lpBytesReturned
                        ) 
{
    UNREFERENCED_PARAMETER(code);
    UNREFERENCED_PARAMETER(lpInBuf);
    UNREFERENCED_PARAMETER(nInBufSize);
    UNREFERENCED_PARAMETER(lpOutBuf);
    UNREFERENCED_PARAMETER(nOutBufSize);

    // We have a battery backed CMOS based clock.  Don't actually do anything
    // to the RTC unless it appears to have been corrupted.  I guess the best
    // way to detect corruption is by checksumming CMOS

    if (lpBytesReturned) 
    {
        *lpBytesReturned = 0;
    }

    return TRUE;
}

