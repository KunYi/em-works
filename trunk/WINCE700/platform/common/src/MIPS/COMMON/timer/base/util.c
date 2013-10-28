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
//  File: util.c
//
//  This function contain implementation of OALStall and OALGetTickCount 
//  functions from timer module suitable for most MIPS based CPU/SoC. It is
//  using CP0 count register to obtain actual
//
#include <windows.h>
#include <oal_timer.h>

//------------------------------------------------------------------------------
//
//  External function:  OALTimerGetCount
//
//  This function is implemented in MIPS CP0 library. It returns actual CP0
//  count register value.
//
UINT32 OALTimerGetCount();

//------------------------------------------------------------------------------
//
//  Function:  OALStall
//
//  Wait for time specified in parameter in microseconds (busy wait). This
//  function can be called in hardware/kernel initialization process.
//
VOID OALStall(UINT32 microSec)
{
    UINT32 base, counts;

    while (microSec > 0) {
        if (microSec > 1000) {
            counts = g_oalTimer.countsPerMSec;
            microSec -= 1000;
        } else {
            counts = (microSec * g_oalTimer.countsPerMSec)/1000;
            microSec = 0;
        }            
        base = OALTimerGetCount();
        while ((OALTimerGetCount() - base) < counts);
    }
}

//------------------------------------------------------------------------------
//
//  Function:  OALGetTickCount
//
//  Function returns number of 1ms ticks since system reboot/reset.
//
UINT32 OALGetTickCount()
{
    static UINT32 base = 0, offset = 0;
    UINT32 plus, delta;

    delta = OALTimerGetCount() - base;
    plus = delta/g_oalTimer.countsPerMSec;
    offset += plus;
    base += plus * g_oalTimer.countsPerMSec;
    return offset;
}

//------------------------------------------------------------------------------
