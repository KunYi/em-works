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
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT.
//
//------------------------------------------------------------------------------
//
//  File: idle.c
//
//  This file contains OEMIdle function implementation. This implementation
//  is suitable for most hardware. It depends on timer which allows extend
//  and reduce actual tick period and it has input clock freq >= 1 kHz.
//  The OALCPUIdle is called to move CPU/SoC to low power mode. Separate file
//  contains busy loop implementation which can be used in developer process.
//  When hardware doesn't support low power mode it is preffered to stub
//  OEMIdle function compared to busy loop OALCPUIdle.
//
#include <windows.h>
#include <nkintr.h>
#include <oal.h>
 
// External Variables 
 
// Global Variables 

// Defines 

// Types
 
// Local Variables 
 
// Local Functions 

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
//  Note that system timer must be running when CPU/SoC is moved to reduced
//  power mode.
//
void OEMIdle(DWORD idleParam)
{
    UINT32 baseMSec, idleMSec, idleSysTicks;
    INT32 usedCounts, idleCounts;

	ULARGE_INTEGER idle = {
		curridlelow,
		curridlehigh
	};

    // Get current system timer counter
    baseMSec = CurMSec;

    // Compute the remaining idle time
    idleMSec = dwReschedTime - baseMSec;
    
    // Idle time has expired - we need to return
    if ((INT32)idleMSec <= 0) return;


    // If we don't have enough margin to update the timer before the current
    // system tick expires, just return
    if ((OALTimerCountsSinceSysTick() + g_oalTimer.countsMargin) >= 
        g_oalTimer.countsPerSysTick) return;

	
	// Idle till tick if iltiming is active
	if (g_oalILT.active) {
		// Idle till end of 'tick'
		OALCPUIdle();

		// Update global idle time and return
		idle.QuadPart += g_oalTimer.countsPerSysTick;
		curridlelow = idle.LowPart;
		curridlehigh = idle.HighPart;
		
		return;
	}

    // Limit the maximum idle time to what is supported.  
    // Counter size is the limiting parameter.  When kernel 
    // profiler or interrupt latency timing is active it is set
    // to one system tick.
    if (idleMSec > g_oalTimer.maxPeriodMSec) {
        idleMSec = g_oalTimer.maxPeriodMSec;
    }
    
    // We can wait only full systick
    idleSysTicks = idleMSec/g_oalTimer.msecPerSysTick;
    
    // This is idle time in hi-res ticks
    idleCounts = idleSysTicks * g_oalTimer.countsPerSysTick;


    // Prolong beat period to idle time -- don't do it idle time isn't
    // longer than one system tick. Even if OALTimerExtendSysTick function
    // should accept this value it can cause problems if kernel profiler
    // or interrupt latency timing is active.
    if (idleSysTicks > 1) {
        // Extend timer period
        OALTimerUpdate(idleCounts, g_oalTimer.countsMargin);
        // Update value for timer interrupt which wakeup from idle
        g_oalTimer.actualMSecPerSysTick = idleMSec;
        g_oalTimer.actualCountsPerSysTick = idleCounts;
    }
	
    // Find how many hi-res ticks were are consumed in the current system tick
    // before idle mode is entered
    usedCounts = OALTimerCountsSinceSysTick();
    
    // Move SoC/CPU to idle mode
    OALCPUIdle();

    // Find how many hi-res ticks were are consumed in the current system tick
    // after idle mode is exited
    idleCounts = OALTimerCountsSinceSysTick();

    // Return system tick period back to original. Don't call when idle
    // time was one system tick. See comment above.
    if (idleSysTicks > 1) {
        // Return system tick period back to original
        idleSysTicks = OALTimerUpdate(
            g_oalTimer.countsPerSysTick, g_oalTimer.countsMargin
        );
        // Restore original values
        g_oalTimer.actualMSecPerSysTick = g_oalTimer.msecPerSysTick;
        g_oalTimer.actualCountsPerSysTick = g_oalTimer.countsPerSysTick;
        // Fix system tick counters & idle counter
        CurMSec += idleSysTicks * g_oalTimer.actualMSecPerSysTick;
        g_oalTimer.curCounts += idleSysTicks * g_oalTimer.actualCountsPerSysTick;

    }

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

