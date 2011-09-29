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
#include "mx233_timrot.h"

//------------------------------------------------------------------------------
// Defines

//------------------------------------------------------------------------------
// External Variables
PVOID pv_HWregTIMROT;
extern BOOL InitRTC();
static PVOID pv_HWregDIGCTL = NULL;

//-----------------------------------------------------------------------------
//  Global: g_oalTimer
OAL_TIMER_STATE g_oalTimer;

//------------------------------------------------------------------------------
// External Functions
extern void OALCPUEnterWFI();

__inline void RechargeTimer(DWORD reschedTime,BOOL finterrupt);
VOID OALTimerUpdateRescheduleTime(DWORD time);

//-----------------------------------------------------------------------------
// Local Variables
static UINT32 g_totalPartialCounts = 0;
static UINT32 g_lastmicroseconds = 0;
static UINT32 g_curmicroseconds = 0;

//-----------------------------------------------------------------------------
// Local Functions

BOOL InitOSTickTimer(hw_timer_InputSource_t eClock, UINT16 u16Fixedcnt)
{
    BOOL rc = FALSE;
    hw_timer_SetupStruct_t stTimerSetup;

    // Check timer input clock source
    if ( (eClock < 0) || (eClock > TIMER_SELECT_TICK_ALWAYS) )
        goto Cleanup;
    if(pv_HWregDIGCTL == NULL)
        pv_HWregDIGCTL = (PVOID)OALPAtoVA(CSP_BASE_REG_PA_DIGCTL, FALSE);

    // Setup the timer default configuration for OS tick timer 0

    stTimerSetup.bIrqEnable     = TIMER_INTERRUPT_DISABLE;
    stTimerSetup.bPolarity      = TIMER_INPUT_POLARITY_NO_CHANGE;
    stTimerSetup.bDutyCycleMode = TIMER_DUTY_CYCLE_MODE_OFF;
    stTimerSetup.bUpdate            = TIMER_UPDATE_RUNNING_COUNT_ON;
    stTimerSetup.bReload            = TIMER_RELOAD_RUNNING_COUNT_ON;
    stTimerSetup.ePrescale          = DIVIDE_BY_8;

    //Setup the timer hardware registers
    stTimerSetup.eSelect       = eClock;
    stTimerSetup.u16FixedCount = u16Fixedcnt;

    //
    // Enable the timrot block and disbale the clock gate
    //  to make the block configurable.
    //
    BF_CLR(TIMROT_ROTCTRL, CLKGATE);        // Disbale the clock gate
    BF_CLR(TIMROT_ROTCTRL, SFTRST);         // Enable the timrot block

    //
    // Configure the timer control register
    //
    BF_WRn(TIMROT_TIMCTRLn, TIMER0, IRQ_EN,   stTimerSetup.bIrqEnable);
    BF_WRn(TIMROT_TIMCTRLn, TIMER0, POLARITY, stTimerSetup.bPolarity);
    BF_WRn(TIMROT_TIMCTRLn, TIMER0, UPDATE,   stTimerSetup.bUpdate);
    BF_WRn(TIMROT_TIMCTRLn, TIMER0, RELOAD,   stTimerSetup.bReload);
    BF_WRn(TIMROT_TIMCTRLn, TIMER0, PRESCALE, stTimerSetup.ePrescale);
    BF_WRn(TIMROT_TIMCTRLn, TIMER0, SELECT,   stTimerSetup.eSelect);

    //
    // Sets the timer FIXED_COUNT value
    //
    if ((stTimerSetup.bReload) && (stTimerSetup.u16FixedCount))
        BF_WRn(TIMROT_TIMCOUNTn, TIMER0, FIXED_COUNT, stTimerSetup.u16FixedCount-1);
    else
        BF_WRn(TIMROT_TIMCOUNTn, TIMER0, FIXED_COUNT, stTimerSetup.u16FixedCount);

    g_lastmicroseconds = (UINT32)HW_DIGCTL_MICROSECONDS_RD();

    //
    rc = TRUE;

Cleanup:
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  RechargeTimer
//
//
//  Assumes: a timer interrupt cannot occur inside this function (caller must ensure this condition)
//
__inline void  RechargeTimer(DWORD reschedMSec, BOOL finterrupt)
{
    UINT32 baseMSec;
    UINT32 deltaMSec = 0;
    UINT32 deltaCounts;
    UINT32 passed = 0;
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
    deltaCounts = deltaMSec << 5;

    // Recharge timer to start new period
    // if reschedule time has already passed set the timer to generate interrupt
    // as soon as it is possible.
    if(deltaMSec == 0)
    {
        deltaCounts = g_oalTimer.countsMargin;
    }

    g_oalTimer.actualMSecPerSysTick   = deltaMSec;
    g_oalTimer.actualCountsPerSysTick = deltaMSec * 1000;
    
    // How many counts has already left since last tick
    if(finterrupt == FALSE)
    {
        passed = ((((UINT32)OALTimerCountsSinceSysTick() + g_totalPartialCounts)) << 5) / 1000;

        if(deltaCounts > passed)
            deltaCounts = deltaCounts - passed + g_oalTimer.countsMargin;
        else
            deltaCounts = g_oalTimer.countsMargin;
    }

    BF_WRn(TIMROT_TIMCOUNTn, TIMER0, FIXED_COUNT, deltaCounts);
    while( (BF_RDn(TIMROT_TIMCOUNTn, TIMER0, RUNNING_COUNT)) != deltaCounts) ;

}

//-----------------------------------------------------------------------------
//
//  Function: OALTimerInit
//
//  This function is typically called from the OEMInit to initialize
//  Windows CE system timer. This implementation uses the SoC EPIT1
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
BOOL OALTimerInit(UINT32 msecPerSysTick, UINT32 countsPerMSec,
                  UINT32 countsMargin)
{
    BOOL rc = FALSE;
    UINT32 irq, sysIntr;

    OALMSG(OAL_TIMER&&OAL_FUNC, (
               L"+OALTimerInit(%d, %d, %d)\r\n", msecPerSysTick, countsPerMSec,
               countsMargin
               ));

    // Check for specialized support of 1MHz time base
    if (msecPerSysTick != 1 || countsPerMSec != 1000)
    {
        OALMSG(OAL_ERROR, (
                   L"ERROR: OALTimerInit: System tick period out of range(countsPerMSec = %d)...\r\n",
                   countsPerMSec));
        goto cleanUp;
    }

    // Initialize timer state global variables
    g_oalTimer.countsPerMSec                    = countsPerMSec;
    g_oalTimer.countsMargin                     = countsMargin;
    g_oalTimer.msecPerSysTick                   = msecPerSysTick;
    g_oalTimer.actualMSecPerSysTick             = msecPerSysTick;
    g_oalTimer.countsPerSysTick                 = (countsPerMSec * msecPerSysTick);
    g_oalTimer.actualCountsPerSysTick   = g_oalTimer.countsPerSysTick;
    g_oalTimer.curCounts                                = 0;

    // MX233 core has 16bit Free running count
    g_oalTimer.maxPeriodMSec                    = 1000;

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

    // MX233: Timer 0
    if(!InitOSTickTimer(TIMER_SELECT_32KHZ_XTAL,32) )
    {
        OALMSG(OAL_ERROR, (L"+OALTimerInit:  InitOSTickTimer Failed\r\n"));
        goto cleanUp;
    }

    // Enable system tick interrupt
    if (!OEMInterruptEnable(sysIntr, NULL, 0)) {
        OALMSG(OAL_ERROR, (
                   L"ERROR: OALTimerInit: Interrupt enable for system timer failed"
                   ));
        goto cleanUp;
    }

    // clear Timer Interrupt
    BF_CLRn(TIMROT_TIMCTRLn, TIMER0, IRQ);

    // enable TIMER0 interrupt
    BF_WRn(TIMROT_TIMCTRLn, TIMER0, IRQ_EN, TIMER_INTERRUPT_ENABLE);

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
    INT32 count = 0;
    do {
        g_curmicroseconds = (UINT32)HW_DIGCTL_MICROSECONDS_RD();
        count = (INT32)(g_curmicroseconds - g_lastmicroseconds);
    }
    while(g_curmicroseconds !=(UINT32)HW_DIGCTL_MICROSECONDS_RD());
   
    return  count;

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
    UINT32 sysIntr          =  SYSINTR_NOP;
    INT32 msecToNextResched;
    UINT32 delta = 0;

#ifdef OAL_ILTIMING
    if (g_oalILT.active) {
        if(BF_RDn(TIMROT_TIMCOUNTn, TIMER0, RUNNING_COUNT) > 0)
            g_oalILT.isrTime1 = (BF_RDn(TIMROT_TIMCOUNTn, TIMER0, FIXED_COUNT) - BF_RDn(TIMROT_TIMCOUNTn, TIMER0, RUNNING_COUNT)) * 31;
        else
            g_oalILT.isrTime1 = 0; 
    }        
#endif


    delta = (UINT32)OALTimerCountsSinceSysTick();  
    g_lastmicroseconds = (UINT32)HW_DIGCTL_MICROSECONDS_RD();

    // clear Timer Interrupt
    BF_CLRn(TIMROT_TIMCTRLn, 0, IRQ);
    
    // g_lastPartialCounts represents fractional milliseconds which will
    // be added to CurMSec once they add up to 1 MSec
    g_totalPartialCounts += delta;

    CurMSec += g_totalPartialCounts / g_oalTimer.countsPerMSec;
    
    g_totalPartialCounts = g_totalPartialCounts % g_oalTimer.countsPerMSec;
    
    g_oalTimer.curCounts += delta;

    // Calculate the distance to the next reschedule period
    msecToNextResched = dwReschedTime - CurMSec;

    // Reschedule?
    if (msecToNextResched <= 0)
    {
        sysIntr = SYSINTR_RESCHED;

        // Recharge timer with the maximum period possible from now. Kernel will call
        // OALTimerUpdateRescheduleTime to set it to the correct value it wants.
        RechargeTimer(CurMSec + g_oalTimer.maxPeriodMSec, TRUE);
    }
    // Check if we need to change the current tick interval
    else if(msecToNextResched  != (int)g_oalTimer.actualMSecPerSysTick)
    {
        // change the current tick interval
        RechargeTimer(CurMSec+ msecToNextResched,TRUE);
    }

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
    if ((baseMSec != CurMSec) || (counts < (INT32)g_oalTimer.countsPerMSec)) goto cleanUp;

    //Note: We are going to assume that RechargeTimer will not take more than 1 ms and since we have already
    // checked above that there is at least 1 ms before the next timer interrupt, no timer interrupts
    // can occur during RechargeTimer execution (thus we satisfy the condition imposed by RechargeTimer)
    RechargeTimer(time, FALSE);

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
UINT32 OEMGetTickCount()
{
    UINT32 count;
    UINT32 offset,unused;

        // System timer tick period exceeds 1 ms.
        //
        // This code adjusts the accuracy of the returned value to the nearest
        // MSec when the system tick exceeds 1 ms. The following code checks if
        // a system timer interrupt occurred between reading the CurMSec value
        // and the call to fetch the HiResTicksSinceSysTick. If so, the value of
        // CurMSec and Offset is re-read, with the certainty that a system timer
        // interrupt will not occur again.
        do {
            count = CurMSec;
            unused = g_totalPartialCounts;
            offset = OALTimerCountsSinceSysTick();
        }
        while ((unused != g_totalPartialCounts)||(count != CurMSec));

        // Adjust the MSec value with the contribution from HiRes counter.
        count += ((offset + unused) / g_oalTimer.countsPerMSec);

    return count;
}
//------------------------------------------------------------------------------
//
//  Function: OALQueryPerformanceFrequency
//
//  This function returns the frequency of the high-resolution
//  performance counter.
//
BOOL OALTimerQueryPerformanceFrequency(LARGE_INTEGER *pFrequency)
{
    if (!pFrequency) return FALSE;

    pFrequency->HighPart = 0;
    pFrequency->LowPart = 1000000;
    
    return TRUE;
}
//------------------------------------------------------------------------------
//
//  Function: OALQueryPerformanceCounter
//
//  This function returns the current value of the high-resolution
//  performance counter.
//
BOOL OALTimerQueryPerformanceCounter(LARGE_INTEGER *pCounter)
{
    UINT64 base;
    INT32 offset;

    if (!pCounter) return FALSE;

    // Make sure CurTicks is the same before and after read of counter
    // to avoid for possible rollover. Note that this is probably not necessary
    // because TimerTicksSinceBeat will return negative value when it happen.
    // We must be careful about signed/unsigned arithmetic.

    do {
        base = g_oalTimer.curCounts;
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
    if (idleCounts < 0) idleCounts = 0;

    // Update idle counters
    idle.LowPart = curridlelow;
    idle.HighPart = curridlehigh;
    idle.QuadPart += idleCounts;
    curridlelow  = idle.LowPart;
    curridlehigh = idle.HighPart;
}
