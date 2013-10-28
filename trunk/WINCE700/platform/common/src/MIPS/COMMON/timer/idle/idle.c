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
#include <windows.h>
#include <nkintr.h>
#include <pkfuncs.h>
#include <oal.h>

//------------------------------------------------------------------------------
//
//  Function:     OEMIdle
//
//  This function is called by the kernel when there are no threads ready to 
//  run. The CPU should be put into a reduced power mode if possible and halted. 
//  It is important to be able to resume execution quickly upon receiving an 
//  interrupt.
//
//  Interrupts are disabled when OEMIdle is called and when it returns.
//
//  This implementation doesn't change system tick. It may be used with variable
//  or fixed tick timers.  It does not rely on the timer ISR telling us ahead of time how
//  long we will idle.  Thus, it will work even if a variable tick timer is used with a
//  profiler running on the same clock.
//
VOID OEMIdle(DWORD idleParam)
{
    UINT32 baseMSec;
    INT32 usedCounts, idleCounts;
    ULARGE_INTEGER idle;

    // Get current system timer counter
    baseMSec = CurMSec;
    
    // Find how many hi-res ticks was already used
    usedCounts = OALTimerCountsSinceSysTick();

    // Move SoC/CPU to idle mode
    OALCPUIdle();

    // Find out how long we waited
    idleCounts = (CurMSec -baseMSec) * g_oalTimer.countsPerMSec + OALTimerCountsSinceSysTick();

    // Get real idle value. If result is negative we didn't idle at all.
    idleCounts -= usedCounts;
    if (idleCounts < 0) idleCounts = 0;

    // Update idle counters
    idle.LowPart = curridlelow;
    idle.HighPart = curridlehigh;
    idle.QuadPart += idleCounts;
    curridlelow  = idle.LowPart;
    curridlehigh = idle.HighPart;
}

//------------------------------------------------------------------------------
