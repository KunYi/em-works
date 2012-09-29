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
//  Local Variables
static volatile UINT32 g_lastTimerMatch;
static UINT32 g_totalPartialCounts = 0;
static volatile UINT32 g_lastPartialCounts = 0;

//------------------------------------------------------------------------------
//
//  Function:  RechargeTimer
//
// Assumes: a timer interrupt cannot occur inside this function (caller must ensure this condition)
//
__inline VOID RechargeTimer(DWORD reschedMSec)
{
    UINT32 baseMSec;
    UINT32 deltaMSec = 0;
    UINT32 deltaCounts;
    UINT32 TimerCount;
    UINT32 newTimerMatch;

    baseMSec = CurMSec;
    deltaMSec = reschedMSec - baseMSec;

    // reschedule time already passed?
    if ((INT32)deltaMSec < 0) 
    {
            deltaMSec = 0;
    }
    else if (deltaMSec > g_oalTimer.maxPeriodMSec) 
    { 
        // If next tick is longer than maximal tick period,
        // then schedule next tick for max allowed MSec by the timer counter
        deltaMSec = g_oalTimer.maxPeriodMSec;
    }
    
    // at this point we know the deltaMSec from CurMSec when next systick has to be scheduled

    // since timers work off of counts calculate deltaCounts
    deltaCounts = deltaMSec * g_oalTimer.countsPerMSec;

    // Recharge timer to start new period

    // ideal case: increment match value by deltaCounts
    // but can't do simply this because while this routine is being executed counter is incrementing.
    // Hence, for these two conditions need to increment deltaCounts so that new match value will be ahead of counter.
    // 1) if deltaCounts is too small and incrementing match value by deltaCounts puts it before counter
    // 2) if incremeting match value by deltaCounts puts it ahead of counter but so close that by the time match value is set counter 
    //     will be ahead of it
    // For both these conditions it is suffice to use this condition if (match value + deltaCounts) < (counter + margin) then increment deltaCounts

    newTimerMatch = g_lastTimerMatch + deltaCounts;
 
    /*** TIMER SPECIFIC CODE ***/
    TimerCount = OALTimerGetCount();
    /*** TIMER SPECIFIC CODE END***/

    // setting osmr0 to newTimerMatch puts it before oscr0?
    if (((INT32)(TimerCount + g_oalTimer.countsMargin - g_lastTimerMatch - deltaCounts)  >  0)) {

        // set the new match value to the earliest we can set it to
        newTimerMatch =   TimerCount +  g_oalTimer.countsMargin;

        //since timermatch value changed we need to recalculate deltaCounts and deltaMSec.
        //These values are used to set actualMSecPerSysTick and actualCountsPerSysTick.
        deltaCounts = (INT32)(newTimerMatch - g_lastTimerMatch);

        // deltaMSec  denotes the time that CurMSec should be incremented by next time when timer ISR runs.
        // This is calculated by dividing deltaCounts by countsPerMsec. The following code avoids this expensive 
        // division for the most common cases of delta being between 0 and 1ms and between 1 and 2ms
        if (deltaCounts < g_oalTimer.countsPerMSec) { // deltaCounts < 1MSec
            deltaMSec = 0;
            g_lastPartialCounts = deltaCounts;
        }
        else if (deltaCounts < (g_oalTimer.countsPerMSec << 1)) {  // deltaCounts < 2MSec
            deltaMSec = 1;
            g_lastPartialCounts = deltaCounts - g_oalTimer.countsPerMSec;
        }
        else {
            deltaMSec = deltaCounts / g_oalTimer.countsPerMSec;            
            g_lastPartialCounts = deltaCounts - deltaMSec * g_oalTimer.countsPerMSec;
        }
    }


    /*** TIMER SPECIFIC CODE ***/
    OALTimerSetCompare(newTimerMatch);
    /*** TIMER SPECIFIC CODE END***/

    g_oalTimer.actualMSecPerSysTick = deltaMSec;
    g_oalTimer.actualCountsPerSysTick = deltaCounts;

}

//------------------------------------------------------------------------------
//
//  Function:  OALTimerInit
//
//  Initialize timer oriented constants and start system timer. Usually system
//  timer triggers each 1 ms but it can be changed if required to optimize
//  platform and set beat period to multiple of ms.
//
BOOL OALTimerInit(
    UINT32 msecPerSysTick, UINT32 countsPerMSec, UINT32 countsMargin
)
{
    BOOL rc = FALSE;
    UINT32 timerMatch, timerCount;

    OALMSG(OAL_TIMER&&OAL_FUNC, (
        L"+OALTimerInit(%d, %d, %d)\r\n", msecPerSysTick, countsPerMSec,
        countsMargin
    ));

    // MIPS counter has 32 bits, but we need to avoid too long periods
    g_oalTimer.maxPeriodMSec = 0x3FFFFFFF/countsPerMSec;

    // System tick period must be smaller than counter maximal period.
    // This reduction will be usual when variable tick is used.
    if (msecPerSysTick > g_oalTimer.maxPeriodMSec) {
        msecPerSysTick = g_oalTimer.maxPeriodMSec;
    }

    // Initialize timer state structure
    g_oalTimer.countsPerMSec = countsPerMSec;
    g_oalTimer.countsMargin = countsMargin;
    g_oalTimer.msecPerSysTick = msecPerSysTick;
    g_oalTimer.actualMSecPerSysTick = msecPerSysTick;
    g_oalTimer.countsPerSysTick = countsPerMSec * msecPerSysTick;
    g_oalTimer.actualCountsPerSysTick = g_oalTimer.countsPerSysTick;
    g_oalTimer.curCounts = 0;

    // Set idle conversion constant and counters (kernel-exported values)
    idleconv = countsPerMSec;
    curridlehigh = curridlelow = 0;

    // initialize update reschedule time function pointer
    pOEMUpdateRescheduleTime = OALTimerUpdateRescheduleTime;

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

    timerCount = OALTimerGetCount();
    timerMatch = timerCount + g_oalTimer.countsPerSysTick;
    OALTimerSetCompare(timerMatch);

    // set g_lastTimerMatch to TimerCount
    g_lastTimerMatch = timerCount;

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
    return OALTimerGetCount() - g_lastTimerMatch;
}

//------------------------------------------------------------------------------
//
//  Function: OALTimerIntrHandler
//
//  This function implements timer interrupt handler. The implementation
//  can be compiled to support interrupt latency timing, real time clock 
//  based on system interrupt and fake idle wakeup.
//
UINT32 OALTimerIntrHandler()
{
    UINT32 sysIntr=SYSINTR_NOP;

    g_lastTimerMatch = OALTimerGetCompare();

#ifdef OAL_ILTIMING
    if (g_oalILT.active) {
            g_oalILT.isrTime1 = OALTimerCountsSinceSysTick();
    }        
#endif

    // g_lastPartialCounts represents fractional milliseconds which will
    // be added to CurMSec once they add up to 1 MSec
    g_totalPartialCounts += g_lastPartialCounts;
    g_lastPartialCounts = 0;

    if (g_totalPartialCounts > g_oalTimer.countsPerMSec)
    {
        CurMSec++;
        g_totalPartialCounts -= g_oalTimer.countsPerMSec;
    }

    // Update the millisecond and high resolution counters
    CurMSec += g_oalTimer.actualMSecPerSysTick;
    g_oalTimer.curCounts += g_oalTimer.actualCountsPerSysTick;

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

    // reschedule ?
    if ((INT32)(CurMSec - dwReschedTime) >= 0)
    {
        sysIntr = SYSINTR_RESCHED;
        RechargeTimer(CurMSec + g_oalTimer.maxPeriodMSec);
    }
    else
    {
        RechargeTimer(dwReschedTime);
    }

#ifdef OAL_ILTIMING
    if (g_oalILT.active)
    {
        g_oalILT.counter--;
        if (g_oalILT.counter==0)
        {
            sysIntr = SYSINTR_TIMING;
            g_oalILT.counter = g_oalILT.counterSet;
            g_oalILT.isrTime2 = OALTimerCountsSinceSysTick();
        }
    }
#endif

    // Return
    return sysIntr;
}

//------------------------------------------------------------------------------
//
//  Function:  OALTimerUpdateRescheduleTime
//
//  This function is called by kernel to set next reschedule time.
//
VOID OALTimerUpdateRescheduleTime(DWORD time)
{
    UINT32 baseMSec;
    INT32 counts;

    // Get current system timer counter
    baseMSec = CurMSec;

    // Return if we are already setup correctly
    if (time == (baseMSec + g_oalTimer.actualMSecPerSysTick)) goto cleanUp;

    // How far we are from next tick
    counts = g_oalTimer.actualCountsPerSysTick - OALTimerCountsSinceSysTick();

    // If timer interrupts occurs, or we are within 1 ms of the scheduled
    // interrupt, just return - timer ISR will take care of it.
    if ((baseMSec != CurMSec) || (counts < (INT32)g_oalTimer.countsPerMSec)) goto cleanUp;

    // Note: We are going to assume that RechargeTimer will not take more than 1 ms and since we have already
    // checked above that there is at least 1 ms before the next timer intr, no timer intr can occur during RechargeTimer execution
    // (thus we satisfy the condition imposed by RechargeTimer)
    RechargeTimer(time);

cleanUp:
    return;    
}

//------------------------------------------------------------------------------
