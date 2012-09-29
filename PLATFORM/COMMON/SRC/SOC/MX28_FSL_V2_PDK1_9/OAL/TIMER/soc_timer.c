//------------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  soc_timer.c
//
//  This file contains SoC-specific routines to support the OAL timer.
//
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#include <oal.h>
#pragma warning(pop)

#include "csp.h"
#include "mx28_timrot.h"

//------------------------------------------------------------------------------
// Defines
// Limit period to max positive 32-bit offset
#define TIMER0_MAX_PERIOD               0x5FFFFFFF        //134 seconds

// RAW_TICKS_TO_MSEC converts from raw timer ticks to msec.
#define RAW_TICKS_TO_MSEC(ticks)        (ticks) / g_oalTimer.countsPerMSec

// MSEC_TO_RAW_TICKS converts from msec to raw timer ticks.
#define MSEC_TO_RAW_TICKS(msec)         (msec) * g_oalTimer.countsPerMSec

//------------------------------------------------------------------------------
// External Variables
extern BOOL InitRTC();

//-----------------------------------------------------------------------------
//  Global: g_oalTimer
OAL_TIMER_STATE g_oalTimer;
PVOID pv_HWregTIMROT;

//------------------------------------------------------------------------------
// External Functions
extern void OALCPUEnterWFI();

//-----------------------------------------------------------------------------
// Local Functions
VOID OALTimerRecharge(UINT32 period, UINT32 margin);
VOID OALTimerUpdateRescheduleTime(DWORD time);

//-----------------------------------------------------------------------------
// Local Variables
static volatile UINT32 g_BaseTimerCount;
static volatile UINT32 g_UnusedTimerCount;

//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//
//  Function: InitOSTickTimer
//
//  This function is to initial OS tick timer.
//
//  Parameters:
//      eClock
//          [in] hw_timer_InputSource_t input source.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL InitOSTickTimer(hw_timer_InputSource_t eClock)
{
    BOOL rc = FALSE;
    hw_timer_SetupStruct_t stTimerSetup;

    // Check timer input clock source
    if ((eClock < 0) || (eClock > TIMER_SELECT_TICK_ALWAYS))
    {
        goto Cleanup;
    }
    // Setup the timer default configuration for OS tick timer 0
    stTimerSetup.bIrqEnable         = TIMER_INTERRUPT_DISABLE;
    stTimerSetup.bPolarity          = TIMER_INPUT_POLARITY_NO_CHANGE;
    stTimerSetup.bDutyCycleMode     = TIMER_DUTY_CYCLE_MODE_OFF;
    stTimerSetup.bUpdate            = TIMER_UPDATE_RUNNING_COUNT_OFF;
    stTimerSetup.bReload            = TIMER_RELOAD_RUNNING_COUNT_OFF;
    stTimerSetup.ePrescale          = DIVIDE_BY_2;
    stTimerSetup.bMatchMode         = TIMER_MATCH_MODE_ENABLE;

    //Setup the timer hardware registers
    stTimerSetup.eSelect            = eClock;

    // Enable the timrot block and disbale the clock gate to make the block configurable.
    BF_CLR(TIMROT_ROTCTRL, CLKGATE);        // Disbale the clock gate
    BF_CLR(TIMROT_ROTCTRL, SFTRST);         // Enable the timrot block

    // Configure the timer control register
    BF_WRn(TIMROT_TIMCTRLn, TIMER0, IRQ_EN,       stTimerSetup.bIrqEnable);
    BF_WRn(TIMROT_TIMCTRLn, TIMER0, POLARITY,     stTimerSetup.bPolarity);
    BF_WRn(TIMROT_TIMCTRLn, TIMER0, UPDATE,       stTimerSetup.bUpdate);
    BF_WRn(TIMROT_TIMCTRLn, TIMER0, RELOAD,       stTimerSetup.bReload);
    BF_WRn(TIMROT_TIMCTRLn, TIMER0, PRESCALE,     stTimerSetup.ePrescale);
    BF_WRn(TIMROT_TIMCTRLn, TIMER0, SELECT,       stTimerSetup.eSelect);
    BF_WRn(TIMROT_TIMCTRLn, TIMER0, MATCH_MODE,   stTimerSetup.bMatchMode);

    rc = TRUE;

Cleanup:
    return rc;
}

//-----------------------------------------------------------------------------
//
//  Function: OALTimerInit
//
//  This function is typically called from the OEMInit to initialize
//  Windows CE system timer. This implementation uses the SoC timer0
//  for a system timer.
//
//  Parameters:
//      msecPerSysTick
//          [in] Defines the system-tick period in msec.
//
//      countsPerMSec
//          [in] States the timer input clock frequency. The value is equal to
//          the frequency divided by 1000.
//
//      countsMargin
//          [in] Used in the timer manipulation routines
//          to define a safe time range in raw timer ticks where the timer
//          can be modified without errors.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL OALTimerInit(UINT32 msecPerSysTick, UINT32 countsPerMSec,UINT32 countsMargin)
{
    BOOL rc = FALSE;
    UINT32 irq, sysIntr;
    UINT32 countsPerSysTick;

    OALMSG(OAL_TIMER&&OAL_FUNC, (
               L"+OALTimerInit(%d, %d, %d)\r\n", msecPerSysTick, countsPerMSec,
               countsMargin
               ));

    // Validate Input parameters
    countsPerSysTick = countsPerMSec * msecPerSysTick;

    // Initialize timer state global variables
    g_oalTimer.countsPerMSec               = countsPerMSec;
    g_oalTimer.countsMargin                = countsMargin;
    g_oalTimer.msecPerSysTick              = msecPerSysTick;
    g_oalTimer.actualMSecPerSysTick        = msecPerSysTick;
    g_oalTimer.countsPerSysTick            = (countsPerMSec * msecPerSysTick);
    g_oalTimer.actualCountsPerSysTick      = g_oalTimer.countsPerSysTick;
    g_oalTimer.curCounts                   = 0;

    // MX28 core has 32bit Free running count
    g_oalTimer.maxPeriodMSec               = RAW_TICKS_TO_MSEC(TIMER0_MAX_PERIOD);

    // Set idle conversion constant and counters (kernel-exported values)
    idleconv = countsPerMSec;
    curridlehigh = 0;
    curridlelow  = 0;

    // Initialize update reschedule time function pointer
    pOEMUpdateRescheduleTime = OALTimerUpdateRescheduleTime;

    // Initialize high resolution timer function pointers
    pQueryPerformanceFrequency = OALTimerQueryPerformanceFrequency;
    pQueryPerformanceCounter   = OALTimerQueryPerformanceCounter;

    // Request IRQ to SYSINTR mapping
    irq = IRQ_TIMER0;
    sysIntr = OALIntrRequestSysIntr(1, &irq, OAL_INTR_FORCE_STATIC);

    // Make sure we have a valid sysIntr
    if (sysIntr == SYSINTR_UNDEFINED)
    {
        OALMSG(OAL_ERROR, (L"+OALTimerInit:  SYSINTR_UNDEFINED\r\n"));
        goto cleanUp;
    }

    // Get uncached virtual addresses for system timer
    pv_HWregTIMROT = (PVOID) OALPAtoUA(CSP_BASE_REG_PA_TIMROT);
    if (pv_HWregTIMROT == NULL)
    {
        OALMSG(OAL_ERROR, (L"+OALTimerInit:  pv_HWregTIMROT null pointer!\r\n"));
        goto cleanUp;
    }

    // MX28: Timer 0 as system tick timer
    if(!InitOSTickTimer(TIMER_SELECT_TICK_ALWAYS))
    {
        OALMSG(OAL_ERROR, (L"+OALTimerInit:  InitOSTickTimer Failed\r\n"));
        goto cleanUp;
    }

    // Initialize timer global for calculating counts since last system tick
    g_BaseTimerCount = HW_TIMROT_RUNNING_COUNTn_RD(TIMER0);

    // Configure the compare register to generate an interrupt when
    // the timer0 ticks for the desired system tick have expired
    OALTimerSetCompare(g_BaseTimerCount - countsPerSysTick);

    // Enable system tick interrupt
    if (!OEMInterruptEnable(sysIntr, NULL, 0)) 
    {
        OALMSG(OAL_ERROR, (
                   L"ERROR: OALTimerInit: Interrupt enable for system timer failed"
                   ));
        goto cleanUp;
    }

    // Clear Timer Interrupt
    BF_CLRn(TIMROT_TIMCTRLn, TIMER0, IRQ);

    // Enable TIMER0 interrupt
    BF_WRn(TIMROT_TIMCTRLn, TIMER0, IRQ_EN, TIMER_INTERRUPT_ENABLE);

    // Initial RTC
    InitRTC();

    // Done
    rc = TRUE;

cleanUp:
    OALMSG(OAL_TIMER && OAL_FUNC, (L"-OALTimerInit(rc = %d)\r\n", rc));
    return rc;
}
//-----------------------------------------------------------------------------
//
//  Function: OALTimerCountsSinceSysTick
//
//  This function returns the time that has expired since the current
//  system tick.
//
//  Parameters:
//      None.
//
//  Returns:
//      Raw clock ticks since last system tick.
//
//-----------------------------------------------------------------------------
INT32 OALTimerCountsSinceSysTick()
{
    // Time difference since last system tick can be measured by
    // calculating the delta between timer starting point for the 
    // current system tick and the current timer value
    return g_BaseTimerCount - HW_TIMROT_RUNNING_COUNTn_RD(TIMER0);
}
//------------------------------------------------------------------------------
//  Function: OALTimerIntrHandler
//
//  This function implement timer interrupt handler. It is called from common
//  ARM interrupt handler.
//------------------------------------------------------------------------------
UINT32 OALTimerIntrHandler()
{
    UINT32 sysIntr = SYSINTR_NOP;
    UINT32 delta, msec;
    INT32 msecToNextResched;
    
#ifdef OAL_ILTIMING
    if (g_oalILT.active) 
    {
        g_oalILT.isrTime1 = HW_TIMROT_MATCH_COUNTn_RD(TIMER0) - HW_TIMROT_RUNNING_COUNTn_RD(TIMER0);
    }        
#endif

    // Determine how much time has expired 
    delta = OALTimerCountsSinceSysTick();

    // Update timer globals
    g_oalTimer.curCounts += delta;

    // Calculate the elapsed time in msec.  Include the rounding error that 
    // occurred during the last CurMSec calculation.  This prevents drift 
    // beteen CurMsec and g_oalTimer.curCounts.
    msec = RAW_TICKS_TO_MSEC(delta + g_UnusedTimerCount);
    CurMSec += msec;

    // Calculate raw ticks of rounding error to be included in the next
    // CurMSec calculation.
    g_UnusedTimerCount = (delta + g_UnusedTimerCount) - MSEC_TO_RAW_TICKS(msec);    

    // Move the base forward
    g_BaseTimerCount -= delta;
            
    // Calculate the distance to the next reschedule period
    msecToNextResched = dwReschedTime - CurMSec;

    // Reschedule?
    if (msecToNextResched <= 0)
    {
        sysIntr = SYSINTR_RESCHED;

        // Reset to maximum tick interval.  Scheduler will call 
        // OALTimerUpdateRescheduleTime if period needs to be reduced.
        g_oalTimer.actualMSecPerSysTick = g_oalTimer.maxPeriodMSec;
        g_oalTimer.actualCountsPerSysTick = MSEC_TO_RAW_TICKS(g_oalTimer.maxPeriodMSec);
    }

    // Check if we need to reduce the current tick interval
    else if(msecToNextResched < (int)g_oalTimer.actualMSecPerSysTick)
    {
        // If the next reschedule is sooner that the period of the tick, 
        // adjust the tick interval to prevent from over-shooting
        g_oalTimer.actualMSecPerSysTick = (DWORD)msecToNextResched;
        g_oalTimer.actualCountsPerSysTick = MSEC_TO_RAW_TICKS(msecToNextResched);
    }

    // Advance to next system tick
    OALTimerRecharge(g_oalTimer.actualCountsPerSysTick, g_oalTimer.countsMargin);

#ifdef OAL_ILTIMING
    if (g_oalILT.active) 
    {
        g_oalILT.counter--;
        if (g_oalILT.counter == 0) 
        {
            sysIntr = SYSINTR_TIMING;
            g_oalILT.counter = g_oalILT.counterSet;
            g_oalILT.isrTime2 = OALTimerCountsSinceSysTick();
        }
    }
#endif

    return sysIntr;

}
//------------------------------------------------------------------------------
//
//  Function:  OALTimerUpdateRescheduleTime
//
//  This function is called by kernel to set next reschedule time.
//
//------------------------------------------------------------------------------
VOID OALTimerUpdateRescheduleTime(DWORD time)
{
    UINT32 baseMSec, diffMSec, diffCounts;
    INT32 counts;
    BOOL bIntrEnable;
    
    // Get current system timer counter
    baseMSec = CurMSec;

    // Return if we are already setup correctly
    if (time == (baseMSec + g_oalTimer.actualMSecPerSysTick))
        goto cleanUp;

    // Calculate how far we are from next tick
    counts = g_oalTimer.actualCountsPerSysTick - OALTimerCountsSinceSysTick();

    // If timer interrupt occurs, or we are within 1 ms of the scheduled
    // interrupt, just return - timer ISR will take care of it.
    if (baseMSec != CurMSec || counts < (INT32)g_oalTimer.countsPerMSec) 
        goto cleanUp;       

    // Calculate the distance between the new time and the last timer interrupt
    diffMSec = time - baseMSec;

    // Trying to set reschedule time prior or equal to CurMSec - this could
    // happen if a thread is on its way to sleep while preempted before
    // getting into the Sleep Queue
    if ((INT32)diffMSec < 0) 
        diffMSec = 0;

    // Clamp distance to next reschedule time 
    if (diffMSec > g_oalTimer.maxPeriodMSec) 
        diffMSec = g_oalTimer.maxPeriodMSec;      

    // Convert distance to next reschedule time into raw ticks
    diffCounts = MSEC_TO_RAW_TICKS(diffMSec);

    // Save actual values to be used by interrupt handler
    g_oalTimer.actualMSecPerSysTick = diffMSec;
    g_oalTimer.actualCountsPerSysTick = diffCounts;

    // Reduce actual timer period (implementation must shift interrupt time
    // if we are too close to new tick time)
    bIntrEnable = INTERRUPTS_ENABLE(FALSE);
    OALTimerRecharge(diffCounts, g_oalTimer.countsMargin);
    INTERRUPTS_ENABLE(bIntrEnable);

cleanUp:
    return;

}


//------------------------------------------------------------------------------
//
//  Function: OEMGetTickCount
//
//  This returns the number of milliseconds that have elapsed since Windows
//  CE was started. If the system timer period is 1ms the function simply
//  returns the value of CurMSec. If the system timer period is greater then
//  1 ms, the HiRes offset is added to the value of CurMSec.
//
//------------------------------------------------------------------------------
UINT32 OEMGetTickCount()
{
    UINT32 base, unused, count;
    INT32 offset;
    
    // The total elapsed time is calculated as follows:
    //
    //      CurMSec                    (global maintained by OALTimerIntrHandler)
    //    + g_UnusedTimerCount         (rounding error from last CurMSec calc)
    //    + OALTimerCountsSinceSysTick (ticks since last OALTimerIntrHandler)
    //    -----------------------------
    //      count returned
    //
    // Loop until g_BaseTimerCount reads the same to make sure the 
    // OALTimerIntrHandler did not occur during our read of the timer globals.
    // 
    do {
        base   = g_BaseTimerCount;
        count  = CurMSec;
        unused = g_UnusedTimerCount;
        offset = OALTimerCountsSinceSysTick();
    } while ((base != g_BaseTimerCount));

    count += RAW_TICKS_TO_MSEC(unused + offset);

    return count;

}
//------------------------------------------------------------------------------
//
//  Function: OALQueryPerformanceFrequency
//
//  This function returns the frequency of the high-resolution
//  performance counter.
//
//------------------------------------------------------------------------------
BOOL OALTimerQueryPerformanceFrequency(LARGE_INTEGER *pFrequency)
{
    if (!pFrequency) 
    {
        return FALSE;
    }
    
    pFrequency->HighPart = 0;
    pFrequency->LowPart  = 1000 * g_oalTimer.countsPerMSec;
    
    return TRUE;
}
//------------------------------------------------------------------------------
//
//  Function: OALQueryPerformanceCounter
//
//  This function returns the current value of the high-resolution
//  performance counter.
//
//------------------------------------------------------------------------------
BOOL OALTimerQueryPerformanceCounter(LARGE_INTEGER *pCounter)
{
    UINT64 base;
    INT32 offset;

    if (!pCounter) 
    {
        return FALSE;
    }
    // Make sure CurTicks is the same before and after read of counter
    // to avoid for possible rollover. Note that this is probably not necessary
    // because TimerTicksSinceBeat will return negative value when it happen.
    // We must be careful about signed/unsigned arithmetic.
    
    do {
       base   = g_oalTimer.curCounts;
       offset = OALTimerCountsSinceSysTick();
    } while (base != g_oalTimer.curCounts);

    // Update the counter
    pCounter->QuadPart = (ULONGLONG)((INT64)base + offset);

    return TRUE;
}
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
//  This implementation doesn't change system tick. It is intend to be used
//  with variable tick implementation. However it should work with fixed
//  variable tick implementation also (with lower efficiency because maximal
//  idle time is 1 ms).
//
//------------------------------------------------------------------------------
VOID OEMIdle(DWORD idleParam)
{

    INT32 beforeCounts, afterCounts, idleCounts;
    ULARGE_INTEGER idle;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(idleParam);

    // Find how many hi-res ticks was already used
    beforeCounts = OALTimerCountsSinceSysTick();

    // Move SoC/CPU to idle mode
    OALCPUIdle();
    INTERRUPTS_OFF();

    afterCounts = OALTimerCountsSinceSysTick();

    // Get real idle value. If result is negative we didn't idle at all.
    idleCounts = afterCounts - beforeCounts;
    if (idleCounts < 0) 
    {
        idleCounts = 0;
    }
    // Update idle counters
    idle.LowPart = curridlelow;
    idle.HighPart = curridlehigh;
    idle.QuadPart += idleCounts;
    curridlelow  = idle.LowPart;
    curridlehigh = idle.HighPart;
}
//-----------------------------------------------------------------------------
//
//  Function:  OALTimerSetCompare
//
//  This function sets timer compare value. This function should be
//  implemented only for systems with count/compare timer type. It is used
//  for OALTimerRecharge/OALTimerCountsSinceSysTick/OALTimerReduceSysTick and
//  OALTimerExtendSysTick implementation for systems with count/compare timer.
//
//------------------------------------------------------------------------------
VOID OALTimerSetCompare(UINT32 compare)
{
    // Clear Timer Interrupt
    BF_CLRn(TIMROT_TIMCTRLn, TIMER0, IRQ);

    // Set the new compare value
    HW_TIMROT_MATCH_COUNTn_WR(TIMER0, compare);
}

//-----------------------------------------------------------------------------
//
//  Function:  OALTimerRecharge
//
//  This function recharge count/compare timer. In case that we are late more
//  than margin value we will use count as new counter base. Under
//  normal conditions previous compare value stored in global variable is used.
//  Using global variable instead reading compare register allows compensate
//  offset when we reduce system tick to value which can cause hazard.
//
//  Parameters:
//      period
//          [in] System tick period in raw timer ticks.
//
//      margin
//          [in] Safe time range in raw timer ticks where the timer can be 
//          modified without errors.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID OALTimerRecharge(UINT32 period, UINT32 margin)
{
    UINT32 cnt, cmpr;
    
    // Grab current timer counter
    cnt = HW_TIMROT_RUNNING_COUNTn_RD(TIMER0);

    // Set timer compare to end of next system tick
    cmpr = g_BaseTimerCount - period + g_UnusedTimerCount;

    // If requested timer compare value is invalid
    if ((INT32) ((cnt - cmpr) - margin) < 0)
    {
        // Cheat
        cmpr = cnt - margin;
    }

    OALTimerSetCompare(cmpr);
}


