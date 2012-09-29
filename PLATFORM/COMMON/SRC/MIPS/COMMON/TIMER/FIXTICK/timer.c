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
    timerMatch = OALTimerGetCount();
    OALTimerSetCompare(timerMatch + g_oalTimer.countsPerSysTick);

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
    UINT32 lastTimerMatch = OALTimerGetCompare() - g_oalTimer.countsPerSysTick;
    return (OALTimerGetCount() - lastTimerMatch);
}

//------------------------------------------------------------------------------
//
//  Function: OALTimerIntrHandler
//
//  This function implements timer interrupt handler. The implementation
//  can be compiled to support interrupt latency timing
//  based on system interrupt and fake idle wakeup.
//
UINT32 OALTimerIntrHandler()
{
    UINT32 sysIntr=SYSINTR_NOP;
    UINT32 timerCount, timerMatch;

    timerCount = OALTimerGetCount();
    timerMatch = OALTimerGetCompare();

    if (timerCount < timerMatch) return sysIntr; //it is fake intr, return

#ifdef OAL_ILTIMING
    if (g_oalILT.active) {
            g_oalILT.isrTime1 = timerCount - timerMatch;
    }        
#endif

    // load next match value
    if ((INT32)(timerCount - timerMatch - g_oalTimer.countsMargin)<0)
        OALTimerSetCompare(timerMatch + g_oalTimer.countsPerSysTick);
    else
        OALTimerSetCompare(OALTimerGetCount() + g_oalTimer.countsPerSysTick);

    // Update high resolution counter
    g_oalTimer.curCounts += g_oalTimer.countsPerSysTick;

    // Update the millisecond counter
    CurMSec += g_oalTimer.msecPerSysTick;

    // Reschedule ?
    if ((int)(CurMSec - dwReschedTime) >= 0)
        sysIntr = SYSINTR_RESCHED;

#ifdef OAL_TIMER_RTC
    // Update RTC global variable
    *g_pOALRTCTicks += g_oalTimer.actualMSecPerSysTick;
    // When RTC alarm is active check if it has to be fired
    if (g_oalRTCAlarm > 0) {
        if (g_oalRTCAlarm > g_oalTimer.actualMSecPerSysTick) {
            g_oalRTCAlarm -= g_oalTimer.actualMSecPerSysTick;
        } else {
            g_oalRTCAlarm = 0;
            sysIntr = SYSINTR_RTC_ALARM;
        }            
    }
#endif

#ifdef OAL_ILTIMING
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
#endif

    // Return
    return sysIntr;
}

//------------------------------------------------------------------------------
