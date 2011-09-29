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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
//------------------------------------------------------------------------------
//
//  File:  timer.c
//
//  Intel Mainstone board initialization code.
//

#include <windows.h>
#include <nkintr.h>
#include <bulverde.h>
#include <oal.h>

//------------------------------------------------------------------------------
// Defines 
 
//------------------------------------------------------------------------------
// External Variables 

//------------------------------------------------------------------------------
// Global Variables 
 
//------------------------------------------------------------------------------
// Local Variables 
static XLLP_OST_HANDLE_T    g_XllpOSTHandle;

//------------------------------------------------------------------------------
// Local Functions 


BOOL OALTimerInit (UINT32 msecPerSysTick, 
                   UINT32 countsPerMSec, 
                   UINT32 countsMargin)
{
    UINT32 TimerMatch;

    // Initialize timer state global variable.
    //
    g_oalTimer.countsPerMSec          = countsPerMSec;
    g_oalTimer.msecPerSysTick         = msecPerSysTick;
    g_oalTimer.actualMSecPerSysTick   = msecPerSysTick;
    g_oalTimer.countsMargin           = countsMargin;

    g_oalTimer.countsPerSysTick       = (countsPerMSec * msecPerSysTick);
    g_oalTimer.actualCountsPerSysTick = (countsPerMSec * msecPerSysTick);
    g_oalTimer.curCounts              = 0;
    g_oalTimer.maxPeriodMSec          = (UINT32)0x7FFFFFFF/g_oalTimer.countsPerMSec;

    // Initialize kernel-exported values.
    //
    idleconv     = countsPerMSec;
    curridlehigh = 0;
    curridlehigh = 0;

    // Initialize high resolution timer function pointers
    pQueryPerformanceFrequency = OALTimerQueryPerformanceFrequency;
    pQueryPerformanceCounter   = OALTimerQueryPerformanceCounter;

    // Obtain pointers to OST and INTC registers.
    //
    g_XllpOSTHandle.pOSTRegs  = (XLLP_OST_T *) OALPAtoVA(BULVERDE_BASE_REG_PA_OST, FALSE);
    g_XllpOSTHandle.pINTCRegs = (XLLP_INTC_T *)OALPAtoVA(BULVERDE_BASE_REG_PA_INTC, FALSE);

    // XLLI initializes oier and rtsr to zeroes, so no further
    // initialization needs to be done.  Match timers and
    // alarms are disabled.
    //
    // Current usage of Match registers:
    //  M0 - Scheduler
    //  M1 - Touch Panel
    //  M2 - Profiler

    // Configure and arm the timer interrupt  to interrupt every specified system tick interval.
    //
    TimerMatch = (g_XllpOSTHandle.pOSTRegs->oscr0 + g_oalTimer.countsPerSysTick);
    XllpOstConfigureTimer (&g_XllpOSTHandle, MatchReg0, TimerMatch);

    return(TRUE);
}


//------------------------------------------------------------------------------
//
//  Function: OALTimerCountsSinceSysTick
//
//  This function return count of hi res ticks since system tick.
//
//

INT32 OALTimerCountsSinceSysTick()
{
    UINT32 count, match;
    
    // Ensure a timer interrupt doesn't happen between the reads
    do {
        match = g_XllpOSTHandle.pOSTRegs->osmr0;
        count = g_XllpOSTHandle.pOSTRegs->oscr0;
    } while (match != g_XllpOSTHandle.pOSTRegs->osmr0);

    return (INT32) (count - (match - g_oalTimer.countsPerSysTick));
}


//------------------------------------------------------------------------------
//
//  Function: OALTimerIntrHandler
//
//  This function implement timer interrupt handler. It is called from common
//  ARM interrupt handler.
//
UINT32 OALTimerIntrHandler()
{
    UINT32 sysIntr = SYSINTR_NOP;
    UINT32 TimerCount;
    UINT32 TimerMatch;

    // Configure and arm the timer interrupt  to interrupt every specified system tick interval.
    //
    TimerCount = g_XllpOSTHandle.pOSTRegs->oscr0;
    TimerMatch = g_XllpOSTHandle.pOSTRegs->osmr0;

#ifdef OAL_ILTIMING
    if (g_oalILT.active) {
        g_oalILT.isrTime1 = TimerCount - TimerMatch;
    }        
#endif

    if ((INT32)(TimerCount - TimerMatch - g_oalTimer.countsMargin) < 0)
    {
        XllpOstConfigureTimer(&g_XllpOSTHandle, MatchReg0, (TimerMatch + g_oalTimer.countsPerSysTick));
    }
    else
    {
        XllpOstConfigureMatchReg(&g_XllpOSTHandle, MatchReg0, g_oalTimer.countsPerSysTick);
    }

    // Update high resolution counter.
    //
    g_oalTimer.curCounts += g_oalTimer.countsPerSysTick;

    // Update the millisecond counter.
    //
    CurMSec += g_oalTimer.msecPerSysTick;

    // Update LEDs.
    //
    OEMWriteDebugLED(0, (CurMSec/1000));

    // Reschedule?
    //
    if ((int)(CurMSec - dwReschedTime) >= 0) sysIntr = SYSINTR_RESCHED;

#ifdef OAL_ILTIMING
    if (g_oalILT.active) {
        if (--g_oalILT.counter == 0) {
            sysIntr = SYSINTR_TIMING;
            g_oalILT.counter = g_oalILT.counterSet;
            g_oalILT.isrTime2 = g_XllpOSTHandle.pOSTRegs->oscr0 - TimerMatch;
        }
    }
#endif

    return (sysIntr);
}

//------------------------------------------------------------------------------
