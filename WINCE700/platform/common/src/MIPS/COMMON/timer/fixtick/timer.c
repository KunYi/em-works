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
//  File: timer.c
//  
//  This file contains timer module functions implementation. The code is
//  using coprocessor 0 count/compare counter. It can be used for most
//  MIPS based CPU/SoC. Note that some hardware implementation doesn't update
//  CP0 counter when it is moved to idle mode. If this is a case this 
//  implementation can't be used for such hardware.
//
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#include <oal.h>
#include <oal_intr_mips.h>


//------------------------------------------------------------------------------
//
//  External:  g_oalRTCTicks/g_oalRTCAlarm
//
//  This global variables are used when RTC is implemented by timer interrupt.
//  The g_oalRTCTicks is incremented in each timer interrupt (RTC implementation
//  doesn't uses CurMSec to avoid rollover in approx 47 days). If g_aolRTCAlarm
//  is nonzero it is decremented on each timer interrupt. When it reaches zero 
//  SYSINTR_RTC_ALARM is returned (instead SYSINTR_RESCHED, but reschedule
//  is done in all cases when timer interrupt returns value different from
//  SYSINTR_NOP).
//
#ifdef OAL_TIMER_RTC
extern UINT64 *g_pOALRTCTicks;
extern UINT64 g_oalRTCTicks;
extern UINT64 g_oalRTCAlarm;
#endif

static volatile UINT32 g_lastTimerMatch;
static UINT32 g_totalPartialCounts = 0;

//------------------------------------------------------------------------------
//
//  Function:  OALTimerInit
//
//  Initialize timer oriented constants and start system timer.
//  Fix tick timer triggers each 1 ms.
//
BOOL OALTimerInit(
    UINT32 msecPerSysTick, UINT32 countsPerMSec, UINT32 countsMargin
)
{
    BOOL rc = FALSE;
    UINT32 timerMatch;

    OALMSG(OAL_TIMER&&OAL_FUNC, (
        L"+OALTimerInit(%d, %d, %d)\r\n", msecPerSysTick, countsPerMSec,
        countsMargin
    ));

    // MIPS counter has 32 bits, but we need to avoid too long periods
    g_oalTimer.maxPeriodMSec = 0x3FFFFFFF/countsPerMSec;

    // Initialize timer state global variable.
    g_oalTimer.countsPerMSec = countsPerMSec;
    g_oalTimer.msecPerSysTick = msecPerSysTick;
    g_oalTimer.countsMargin = countsMargin;
    g_oalTimer.countsPerSysTick = countsPerMSec * msecPerSysTick;
    g_oalTimer.actualMSecPerSysTick = msecPerSysTick;
    g_oalTimer.actualCountsPerSysTick = g_oalTimer.countsPerSysTick;
    g_oalTimer.curCounts = 0;

    // Set idle conversion constant and counters (kernel-exported values)
    idleconv = countsPerMSec;
    curridlehigh = curridlelow = 0;

    // Initialize high resolution timer function pointers
    pQueryPerformanceFrequency = OALTimerQueryPerformanceFrequency;
    pQueryPerformanceCounter = OALTimerQueryPerformanceCounter;

#ifdef OAL_TIMER_RTC
    // Get counter from arguments (depends on platform)
    g_pOALRTCTicks = OALArgsQuery(OAL_ARGS_QUERY_RTC);
    // If platform hasn't such location use internal one
    if (g_pOALRTCTicks == NULL) g_pOALRTCTicks = &g_oalRTCTicks;
    // Clear alarm
    g_oalRTCAlarm = 0;
#endif

    // Hook timer ISR
    if (!HookInterrupt(5, OALTimerIntrHandler)) {
        OALMSG(OAL_ERROR, (
            L"ERROR: OALTimerInit: HookInterrupt for MIPS interrupt 5 failed\r\n"
        ));
        goto cleanUp;
    }        

    // configure and arm the timer interrupt to issue intr every specified system tick interval
    g_lastTimerMatch = OALTimerGetCount();
    timerMatch = g_lastTimerMatch + g_oalTimer.countsPerSysTick;
    OALTimerSetCompare(timerMatch);

    // We are ok
    rc = TRUE;

cleanUp:
    OALMSG(OAL_TIMER&&OAL_FUNC, (L"-OALTimerInit(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  OALTimerCountsSinceSysTick
//
INT32 OALTimerCountsSinceSysTick()
{
    return (OALTimerGetCount() - g_lastTimerMatch) + g_totalPartialCounts;
}

//------------------------------------------------------------------------------
//
//  Function: OALTimerIntrHandler
//
//  This function implements timer interrupt handler. The implementation
//  can be compiled to support interrupt latency timing
//  based on system interrupt and fake idle wakeup.
//
//  It allows the MIPS CP0 count/compare registers to be used both for 
//  the system tick and the profiler.
//
UINT32 OALTimerIntrHandler()
{
    UINT32 sysIntr=SYSINTR_NOP;
    UINT32 timerCount, timerMatch;
    UINT32 tickIncrement = 0;

    timerCount = OALTimerGetCount();
    timerMatch = OALTimerGetCompare();

    if ((INT32)(timerCount - timerMatch) < 0)
    {
        return sysIntr; //it is fake intr, return
    }

    // Valid interrupt
    g_lastTimerMatch = timerMatch;

    if (g_oalILT.active) {
            g_oalILT.isrTime1 = timerCount - timerMatch;
    }        

    // load next match value
    if((INT32)((timerCount + g_oalTimer.countsMargin) // Current time + margin
        - (timerMatch + g_oalTimer.countsPerSysTick)) // 1 tick since our last interrupt - where we'd ideally set the next compare
       > 0)
    {
        // We're past the point where it would be safe to update
        // So we need to set the compare register for more than 1 tick since the last compare
        UINT32 partialCounts, currentCount = OALTimerGetCount();
        OALTimerSetCompare(currentCount + g_oalTimer.countsPerSysTick);
        partialCounts = currentCount -timerMatch;
        g_totalPartialCounts += partialCounts;
        if(g_totalPartialCounts > g_oalTimer.countsPerSysTick)
        {
            tickIncrement += g_oalTimer.msecPerSysTick;
            g_oalTimer.curCounts += g_oalTimer.countsPerSysTick;
            g_totalPartialCounts -= g_oalTimer.countsPerSysTick;
        }
    }
    else
    {
        // We're safe to update the timer normally
        OALTimerSetCompare(timerMatch + g_oalTimer.countsPerSysTick);
    }

    // Fixed tick, add one tick for the timer interrupt
    tickIncrement += g_oalTimer.msecPerSysTick;

    // Update high resolution counter
    g_oalTimer.curCounts += g_oalTimer.countsPerSysTick;

    // Update the millisecond counter
    CurMSec += tickIncrement;

    // Reschedule ?
    if ((int)(CurMSec - dwReschedTime) >= 0)
    {
        sysIntr = SYSINTR_RESCHED;
    }

#ifdef OAL_TIMER_RTC
    // Update RTC global variable
    *g_pOALRTCTicks += tickIncrement;
    if (g_oalRTCAlarm > 0)
    {
        if (g_oalRTCAlarm > tickIncrement)
        {
            g_oalRTCAlarm -= tickIncrement;
        }
        else
        {
            g_oalRTCAlarm = 0;
            sysIntr = SYSINTR_RTC_ALARM;
        }            
    }
#endif

    if (g_oalILT.active)
    {
        g_oalILT.counter--;
        if (g_oalILT.counter==0)
        {
            sysIntr = SYSINTR_TIMING;
            g_oalILT.counter = g_oalILT.counterSet;
            g_oalILT.isrTime2 = OALTimerGetCount() - timerMatch;
        }
    }        

    // Return
    return sysIntr;
}

//------------------------------------------------------------------------------
