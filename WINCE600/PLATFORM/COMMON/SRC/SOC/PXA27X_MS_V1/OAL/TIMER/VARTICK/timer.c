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
#include <oal_ilt.h>

//------------------------------------------------------------------------------
// Defines 
 
//------------------------------------------------------------------------------
// External Variables 

//------------------------------------------------------------------------------
// Global Variables 
 
//------------------------------------------------------------------------------
// Local Variables 
static XLLP_OST_HANDLE_T    g_XllpOSTHandle;
static volatile UINT32      LastTimerMatch;
static UINT32 g_TotalPartialCounts = 0;
static volatile UINT32 g_LastPartialCounts = 0;

//------------------------------------------------------------------------------
// Local Functions 

__inline void RechargeTimer(DWORD reschedTime);
VOID OALTimerUpdateRescheduleTime(DWORD time);


BOOL OALTimerInit (UINT32 msecPerSysTick, 
                   UINT32 countsPerMSec, 
                   UINT32 countsMargin)
{
    UINT32 TimerMatch;
    UINT32 TimerCount;

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

    // Initialize update reschedule time function pointer
    pOEMUpdateRescheduleTime = OALTimerUpdateRescheduleTime;
    
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

    // Configure and arm the timer interrupt  to interrupt starting at current count + 1 system tick interval.
    //
    TimerCount = g_XllpOSTHandle.pOSTRegs->oscr0;
    TimerMatch = TimerCount + g_oalTimer.countsPerSysTick;
    XllpOstConfigureTimer (&g_XllpOSTHandle, MatchReg0, TimerMatch);

    // set LastTimerMatch to TimerCount
    //
    LastTimerMatch = TimerCount;
    
    return(TRUE);
}


//------------------------------------------------------------------------------
//
//  Function: OALTimerCountsSinceSysTick
//
//  This function return count of hi res ticks since system tick.
//   (How many ticks since the last timer interrupt?)
//

INT32 OALTimerCountsSinceSysTick()
{
    UINT32 ltm, count;
    
    // The previous version of this read oscr0 first, causing LastTimerMatch to 
    // end up ahead of oscr0 in rare cases where a timer interrupt would occur 
    // between reading oscr0 and LastTimerMatch.
    // Reversing the read order would have caused a similar problem where the counts
    // returned would have been from two ticks ago, rather than the last tick.
    // To address both of these, we read LastTimerMatch twice, and compare to make sure
    // that no timer interrupts have occured between the reads. 
    do {
        ltm = LastTimerMatch;
        count = g_XllpOSTHandle.pOSTRegs->oscr0;
    } while (ltm != LastTimerMatch);

    return (INT32) (count - ltm);
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

    LastTimerMatch = g_XllpOSTHandle.pOSTRegs->osmr0;


#ifdef OAL_ILTIMING
    if (g_oalILT.active) {
        g_oalILT.isrTime1 = OALTimerCountsSinceSysTick();
    }        
#endif

    // g_LastPartialCounts represents fractional milliseconds which will
    // be added to CurMSec once they add up to one MSec.
    g_TotalPartialCounts += g_LastPartialCounts;
    g_LastPartialCounts = 0;

    if ((g_TotalPartialCounts > g_oalTimer.countsPerMSec)) 
    {
        CurMSec++;
        g_TotalPartialCounts -= g_oalTimer.countsPerMSec;
    }
    
    // Update the millisecond and high resolution counters
    CurMSec += g_oalTimer.actualMSecPerSysTick;
    g_oalTimer.curCounts += g_oalTimer.actualCountsPerSysTick;

    //Re-schedule?
    if ((INT32)(CurMSec - dwReschedTime) >= 0)  {
        sysIntr = SYSINTR_RESCHED;

        // Recharge timer with the maximum period possible from now. Kernel will call
        // OALTimerUpdateRescheduleTime to set it to the correct value it wants.
        RechargeTimer(CurMSec + g_oalTimer.maxPeriodMSec);
    }
    else {
        RechargeTimer(dwReschedTime);
    }
    
    // Update LEDs.
    // (Right shift by 10 instead of expensive division by 1000. This will 
    // cause the LEDs to update every 1.024 seconds instead every 1 second,
    // which is okay as this is just a notification and not a measurement of any sort)
    OEMWriteDebugLED(0, CurMSec >> 10);

#ifdef OAL_ILTIMING
    if (g_oalILT.active) {
        g_oalILT.counter--;
        if (g_oalILT.counter == 0) {
            sysIntr = SYSINTR_TIMING;
            g_oalILT.counter = g_oalILT.counterSet;
            g_oalILT.isrTime2 = OALTimerCountsSinceSysTick();
        }
    }
#endif

    return (sysIntr);
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
    if (baseMSec != CurMSec || counts < (INT32)g_oalTimer.countsPerMSec) {
        goto cleanUp;
    }        

    //Note: We are going to assume that RechargeTimer will not take more than 1 ms and since we have already
    // checked above that there is at least 1 ms before the next timer interrupt, no timer interrupts
    // can occur during RechargeTimer execution (thus we satisfy the condition imposed by RechargeTimer)
    RechargeTimer(time);

cleanUp:
    return;    
}

//------------------------------------------------------------------------------
//
//  Function:  RechargeTimer
//
//
//  Assumes: a timer interrupt cannot occur inside this function (caller must ensure this condition)
//
__inline void  RechargeTimer(DWORD reschedMSec)
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

    // ideal case: increment matchreg (osmr0) by deltaCounts
    // but can't do simply this because while this routine is being executed oscr0 is incrementing.
    // Hence, for these two conditions need to increment deltaCounts so that new osmr0 will be ahead of oscr0.
    // 1) if deltaCounts is too small and incrementing osmr0 by deltaCounts puts it before oscr0
    // 2) if incremeting osmr0 by deltaCounts puts it ahead of oscr0 but so close that by the time osmr0 is set oscr0 
    //     will be ahead of it
    // For both these conditions it is suffice to use this condition if (osmr0 + deltaCounts) < (oscr0 + margin) then increment deltaCounts

    newTimerMatch = LastTimerMatch + deltaCounts;
 
    /*** TIMER SPECIFIC CODE ***/
    TimerCount = g_XllpOSTHandle.pOSTRegs->oscr0;
    /*** TIMER SPECIFIC CODE END***/

    // setting osmr0 to newTimerMatch puts it before oscr0?
    if (((INT32)(TimerCount + g_oalTimer.countsMargin - LastTimerMatch - deltaCounts)  >  0)) {

        // set the new match value to the earliest we can set it to
        newTimerMatch =   TimerCount +  g_oalTimer.countsMargin;

        //since timermatch value changed we need to recalculate deltaCounts and deltaMSec.
        //These values are used to set actualMSecPerSysTick and actualCountsPerSysTick.
        deltaCounts = (INT32)(newTimerMatch - LastTimerMatch);

        // deltaMSec  denotes the time that CurMSec should be incremented by next time when timer ISR runs.
        // This is calculated by dividing deltaCounts by countsPerMsec. The following code avoids this expensive 
        // division for the most common cases of delta being between 0 and 1ms and between 1 and 2ms
        if (deltaCounts < g_oalTimer.countsPerMSec) { // deltaCounts < 1MSec
            deltaMSec = 0;
            g_LastPartialCounts = deltaCounts;
        }
        else if (deltaCounts < (g_oalTimer.countsPerMSec << 1)) {  // deltaCounts < 2MSec
            deltaMSec = 1;
            g_LastPartialCounts = deltaCounts - g_oalTimer.countsPerMSec;
        }
        else {
            deltaMSec = deltaCounts / g_oalTimer.countsPerMSec;            
            g_LastPartialCounts = deltaCounts - deltaMSec * g_oalTimer.countsPerMSec;
        }
    }


    /*** TIMER SPECIFIC CODE ***/
     XllpOstConfigureTimer(&g_XllpOSTHandle, MatchReg0, newTimerMatch);
    /*** TIMER SPECIFIC CODE END***/

    g_oalTimer.actualMSecPerSysTick = deltaMSec;
    g_oalTimer.actualCountsPerSysTick = deltaCounts;

#ifdef DEBUG
    //code to check assumption that timer interrupt should not occur during this routine
    //
    if (baseMSec != CurMSec)
        {
            OEMWriteDebugLED(0, (0xEbbb | 0x1));     //code for error 
            while(1) {};
        }
    
#endif

}

//------------------------------------------------------------------------------
