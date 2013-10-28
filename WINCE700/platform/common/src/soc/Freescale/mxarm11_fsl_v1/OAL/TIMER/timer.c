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
//  Copyright (C) 2004-2007, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Module: timer.c
//
//  Interface to OAL timer services.
//
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#include <oal.h>
#pragma warning(pop)

#include "mxarm11.h"

//------------------------------------------------------------------------------
// External Functions

extern UINT32 OALTimerGetClkSrc(void);
extern UINT32 OALTimerGetClkPrescalar(void);


//------------------------------------------------------------------------------
// Defines

#define EPIT_MAX_PERIOD         0x10000000


//------------------------------------------------------------------------------
// Local Variables

PCSP_EPIT_REG g_pEPIT;
static UINT32 g_BaseTimerCount;


//------------------------------------------------------------------------------
// Local Functions


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
    UINT32 countsPerSysTick;
    UINT32 irq, sysIntr;

    OALMSG(OAL_TIMER&&OAL_FUNC, (
        L"+OALTimerInit( %d, %d, %d )\r\n",
        msecPerSysTick, countsPerMSec, countsMargin
    ));

    // Validate Input parameters
    countsPerSysTick = countsPerMSec * msecPerSysTick;
    if (msecPerSysTick < 1 || msecPerSysTick > 1000 ||
        countsPerSysTick < 1 || countsPerSysTick > EPIT_CNT_COUNT_MAX)
    {
        OALMSG(OAL_ERROR, (
            L"ERROR: OALTimerInit: System tick period out of range..."
        ));
        goto cleanUp;
    }

    // Initialize timer state global variables
    g_oalTimer.msecPerSysTick = msecPerSysTick;
    g_oalTimer.actualMSecPerSysTick = msecPerSysTick;
    g_oalTimer.countsPerMSec = countsPerMSec;
    g_oalTimer.countsMargin = countsMargin;
    g_oalTimer.countsPerSysTick = countsPerSysTick;
    g_oalTimer.actualCountsPerSysTick = countsPerSysTick;
    g_oalTimer.curCounts = 0;
    g_oalTimer.maxPeriodMSec = ((UINT32) EPIT_MAX_PERIOD)/g_oalTimer.countsPerMSec;
    // g_oalTimer.maxPeriodMSec = 1;

    // Set kernel exported globals to initial values
    idleconv = countsPerMSec;
    curridlehigh = 0;
    curridlelow = 0;

    // Initialize high resolution timer function pointers
    pQueryPerformanceFrequency = OALTimerQueryPerformanceFrequency;
    pQueryPerformanceCounter = OALTimerQueryPerformanceCounter;

    // Request IRQ to SYSINTR mapping
    irq = IRQ_EPIT1;
    sysIntr = OALIntrRequestSysIntr(1, &irq, OAL_INTR_FORCE_STATIC);

    // Make sure we have a valid sysIntr
    if (sysIntr == SYSINTR_UNDEFINED)
    {
        OALMSG(OAL_ERROR, (L"+OALTimerInit:  SYSINTR_UNDEFINED\r\n"));
        goto cleanUp;
    }

    // Get uncached virtual addresses for system timer
    g_pEPIT = (PCSP_EPIT_REG) OALPAtoUA(CSP_BASE_REG_PA_EPIT1);
    if (g_pEPIT == NULL)
    {
        OALMSG(OAL_ERROR, (L"+OALTimerInit:  EPIT1 null pointer!\r\n"));
        goto cleanUp;
    }

    // Disable EPIT and clear all configuration bits
    OUTREG32(&g_pEPIT->CR, 0);

    // Assert software reset for the timer
    OUTREG32(&g_pEPIT->CR, CSP_BITFMASK(EPIT_CR_SWR));

    // Wait for the software reset to complete
    while (INREG32(&g_pEPIT->CR) & CSP_BITFMASK(EPIT_CR_SWR));

    // Enable timer for "free-running" mode where timer rolls
    // over from 0x00000000 to 0xFFFFFFFF
    OUTREG32(&g_pEPIT->CR,
        CSP_BITFVAL(EPIT_CR_EN, EPIT_CR_EN_ENABLE) |
        CSP_BITFVAL(EPIT_CR_ENMOD, EPIT_CR_ENMOD_RESUME) |
        CSP_BITFVAL(EPIT_CR_OCIEN, EPIT_CR_OCIEN_ENABLE) |
        CSP_BITFVAL(EPIT_CR_RLD, EPIT_CR_RLD_ROLLOVER) |
        CSP_BITFVAL(EPIT_CR_PRESCALAR, OALTimerGetClkPrescalar()) |
        CSP_BITFVAL(EPIT_CR_SWR, EPIT_CR_SWR_NORESET) |
        CSP_BITFVAL(EPIT_CR_IOVW, EPIT_CR_IOVW_NOOVR) |
        CSP_BITFVAL(EPIT_CR_DBGEN, EPIT_CR_DBGEN_ACTIVE) |
        CSP_BITFVAL(EPIT_CR_WAITEN, EPIT_CR_WAITEN_ENABLE) |
        CSP_BITFVAL(EPIT_CR_DOZEN, EPIT_CR_DOZEN_ENABLE) |
        CSP_BITFVAL(EPIT_CR_STOPEN, EPIT_CR_STOPEN_ENABLE) |
        CSP_BITFVAL(EPIT_CR_OM, EPIT_CR_OM_DICONNECT) |
        CSP_BITFVAL(EPIT_CR_CLKSRC, OALTimerGetClkSrc()));

    // Initialize timer global for calculating counts since last system tick
    g_BaseTimerCount = INREG32(&g_pEPIT->CNT);

    // Configure the compare register to generate an interrupt when
    // the EPIT ticks for the desired system tick have expired
    OALTimerSetCompare(g_BaseTimerCount - countsPerSysTick);

    // Enable system tick interrupt
    if (!OEMInterruptEnable(sysIntr, NULL, 0)) {
        OALMSG(OAL_ERROR, (
            L"ERROR: OALTimerInit: Interrupt enable for system timer failed"
        ));
        goto cleanUp;
    }

//
// Define ENABLE_WATCH_DOG to enable watchdog timer support.
// NOTE: When watchdog is enabled, the device will reset itself if watchdog timer is not refreshed within ~4.5 second.
//       Therefore it should not be enabled when kernel debugger is connected, as the watchdog timer will not be refreshed.
//
#ifdef ENABLE_WATCH_DOG
    {
        extern void InitWatchDogTimer (void);
        InitWatchDogTimer ();
    }
#endif

    // Done
    rc = TRUE;

cleanUp:
    OALMSG(OAL_TIMER && OAL_FUNC, (L"-OALTimerInit(rc = %d)\r\n", rc));
    return rc;
}


//-----------------------------------------------------------------------------
//
//  Function: OALTimerIntrHandler
//
//  This function implements the timer interrupt handler. It is called from
//  the common ARM interrupt handler.
//
//  Parameters:
//      None.
//
//  Returns:
//      SYSINTR value from the interrupt handler
//
//-----------------------------------------------------------------------------
UINT32 OALTimerIntrHandler()
{
    UINT32 sysIntr = SYSINTR_NOP;

    if (g_oalILT.active) {
        if (--g_oalILT.counter == 0) {
            sysIntr = SYSINTR_TIMING;
            g_oalILT.counter = g_oalILT.counterSet;
            g_oalILT.isrTime2 = OALTimerCountsSinceSysTick();
        }
    }

    // Update timer globals
    g_oalTimer.curCounts += g_oalTimer.countsPerSysTick;
    CurMSec += g_oalTimer.msecPerSysTick;

    // Advance to next system tick
    OALTimerRecharge(g_oalTimer.countsPerSysTick, g_oalTimer.countsMargin);

    // Reschedule?
    if ((int)(CurMSec - dwReschedTime) >= 0) sysIntr = SYSINTR_RESCHED;

    return sysIntr;
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
    return g_BaseTimerCount - INREG32(&g_pEPIT->CNT);
}


//-----------------------------------------------------------------------------
//
//  Function:     OEMIdle
//
//  This function is called by the kernel when there are no threads ready to
//  run. The CPU should be put into a reduced power mode if possible and
//  halted. It is important to be able to resume execution quickly upon
//  receiving an interrupt.
//
//  Interrupts are disabled when OEMIdle is called and when it returns.
//
//  Note that system timer must be running when CPU/SoC is moved to reduced
//  power mode.
//
//  Parameters:
//      dwIdleParam
//          [in] Used by MIPS CPU only. Base value for program status
//          register (PSR) to allow interrupts to be enabled.
//
//  Returns:
//      None.
//
//------------------------------------------------------------------------------
void OEMIdle(DWORD dwIdleParam)
{
    UINT32 idleMSec, idleSysTicks;
    INT32 countBeforeIdle, countAfterIdle, idleCounts;
    ULARGE_INTEGER idle;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(dwIdleParam);

    // Compute the remaining idle time
    idleMSec = dwReschedTime - CurMSec;

    // Idle time has expired - we need to return
    if ((INT32)idleMSec <= 0) return;

    // If we don't have enough margin to update the timer before the current
    // system tick expires, just return
    if ((OALTimerCountsSinceSysTick() + g_oalTimer.countsMargin) >=
        g_oalTimer.countsPerSysTick) return;

    // Limit the maximum idle time to what is supported.
    // Counter size is the limiting parameter.  When kernel
    // profiler or interrupt latency timing is active, it is set
    // to one system tick.
    if (idleMSec > g_oalTimer.maxPeriodMSec)
    {
        idleMSec = g_oalTimer.maxPeriodMSec;
    }

    // We can wait only full systick
    idleSysTicks = idleMSec / g_oalTimer.msecPerSysTick;

    // This is idle time in hi-res ticks
    idleCounts = idleSysTicks * g_oalTimer.countsPerSysTick;

    // Prolong beat period to idle time -- don't do it idle time isn't
    // longer than one system tick. Even if OALTimerExtendSysTick function
    // should accept this value it can cause problems if kernel profiler
    // or interrupt latency timing is active.
    if (idleSysTicks > 1)
    {
        // Extend timer period.
        OALTimerUpdate(idleCounts, g_oalTimer.countsMargin);

        // Update value for timer interrupt which wakeup from idle
        g_oalTimer.actualMSecPerSysTick = idleMSec;
        g_oalTimer.actualCountsPerSysTick = idleCounts;
    }

    // Find how many hi-res ticks were are consumed in the current system tick
    // before idle mode is entered
    countBeforeIdle = OALTimerCountsSinceSysTick();

    // Move SoC/CPU to idle mode
    OALCPUIdle();

    // Find how many hi-res ticks were are consumed in the current system tick
    // after idle mode is exited
    countAfterIdle = OALTimerCountsSinceSysTick();

    // Return system tick period back to original. Don't call when idle
    // time was one system tick. See comment above.
    if (idleSysTicks > 1)
    {

        // Since interrupts were disabled upon entry to idle mode, this code
        // should assume the timer interrupt may be pending, but not yet
        // serviced.  Execution resumes within OEMIdle before the timer interrupt
        // is serviced.  We will clear any pending timer interrupt and prepare
        // for the next available system tick.

        // Restore original system tick and expired ticks
        idleSysTicks = OALTimerUpdate(g_oalTimer.countsPerSysTick,
            g_oalTimer.countsMargin);

        // Restore original values used for advancing timer globals
        g_oalTimer.actualMSecPerSysTick = g_oalTimer.msecPerSysTick;
        g_oalTimer.actualCountsPerSysTick = g_oalTimer.countsPerSysTick;

        // Adjust system timer OAL variables.  Note expired system ticks
        // returned by OALTimerUpdate is rounded down to nearest system tick.
        CurMSec += (idleSysTicks * g_oalTimer.msecPerSysTick);
        g_oalTimer.curCounts += (idleSysTicks * g_oalTimer.countsPerSysTick);

    }

    // Get real idle value. If result is negative we didn't idle at all.
    idleCounts = countAfterIdle - countBeforeIdle;
    if (idleCounts < 0) idleCounts = 0;

    // Update idle counters
    idle.LowPart = curridlelow;
    idle.HighPart = curridlehigh;
    idle.QuadPart += idleCounts;
    curridlelow  = idle.LowPart;
    curridlehigh = idle.HighPart;

}


//------------------------------------------------------------------------------
//
//  Function:  OALTimerSetCompare
//
//  This function sets timer compare value. This function should be
//  implemented only for systems with count/compare timer type. It is used
//  for OALTimerRecharge/OALTimerCountsSinceSysTick/OALTimerReduceSysTick and
//  OALTimerExtendSysTick implementation for systems with count/compare timer.
//
VOID OALTimerSetCompare(UINT32 compare)
{
    // Clear timer compare interrupt flag (write-1-clear)
    OUTREG32(&g_pEPIT->SR, CSP_BITFMASK(EPIT_SR_OCIF));

    OUTREG32(&g_pEPIT->CMPR, compare);


#ifdef VPMX31
    // Keep advancing the compare value until we have a valid timer event.
    // On Virtio, the timer advances while the ARM simulator is inactive
    // resulting in invalid timer compare settings.
    while ((INT32) (INREG32(&g_pEPIT->CNT) - INREG32(&g_pEPIT->CMPR)) <= 0)
    {
        OUTREG32(&g_pEPIT->CMPR, INREG32(&g_pEPIT->CNT) - g_oalTimer.countsMargin);

        // Clear timer compare interrupt flag (write-1-clear)
        OUTREG32(&g_pEPIT->SR, CSP_BITFMASK(EPIT_SR_OCIF));
    }
#endif

}

//------------------------------------------------------------------------------
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
////
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
    UINT32 cnt, cmpr, sysTickOffset, rawTickOffset;

    // Grab current timer counter
    cnt = INREG32(&g_pEPIT->CNT);

    // Advance base for current system tick to beginning of next system tick
    g_BaseTimerCount -= period;

    // Set timer compare to end of next system tick
    cmpr = g_BaseTimerCount - period;

    // If requested timer compare value is invalid
    if ((INT32) (cnt - cmpr - margin) < 0)
    {
        // Calculate number of system ticks we have fallen behind
        sysTickOffset = (g_BaseTimerCount - cnt) / period;

        // Calculate raw tick offset from system tick offset
        rawTickOffset = sysTickOffset * period;

        // Advance the timer globals to prevent clock drift
        CurMSec += (sysTickOffset * g_oalTimer.msecPerSysTick);
        g_oalTimer.curCounts += rawTickOffset;

        // Catch up system tick base
        g_BaseTimerCount -= rawTickOffset;

        // Calculate new timer compare value
        cmpr = g_BaseTimerCount - period;

        // Grab current timer counter
        cnt = INREG32(&g_pEPIT->CNT);

        // If we still don't have enough margin
        if ((INT32) (cnt - cmpr - margin) < 0)
        {
            // Let timer interrupt occur ASAP
            cmpr = cnt - margin;
        }
    }

    OALTimerSetCompare(cmpr);
}


//------------------------------------------------------------------------------
//
//  Function: OALTimerUpdate
//
//  This function is called to change length of actual system timer period.
//  If end of actual period is closer than margin period isn't changed (so
//  original period elapse). Function returns time which already expires
//  in new period length units. If end of new period is closer to actual time
//  than margin period end is shifted by margin (but next period should fix
//  this shift - this is reason why OALTimerRecharge doesn't read back
//  compare register and it uses saved value instead).
//
UINT32 OALTimerUpdate(UINT32 period, UINT32 margin)
{
    UINT32 rc = 0, cmpr, cnt, expired;

    // Grab current timer counter
    cnt = INREG32(&g_pEPIT->CNT);

    // If requested system tick is zero
    if (period == 0)
    {
        // Let interrupt occur ASAP
        cmpr = cnt - margin;

        OALTimerSetCompare(cmpr);
    }

    // Else, requested system tick is non-zero
    else
    {
        // Get number of expired high-res ticks in current system tick
        expired = g_BaseTimerCount - cnt;

        // If we want to reduce system tick (i.e. ticks expired in
        // current system tick meet or exceed requested period)
        if (expired >= period)
        {
            // Calculate expired time in new period length units
            rc = expired / period;

            // Advance base for current system tick to beginning of next system tick
            g_BaseTimerCount -= (rc * period);

        }

        // New compare value can be determined by subtracting
        // requested period current system tick base
        cmpr = g_BaseTimerCount - period;

        // If we have enough margin to update timer compare value
        if ((INT32) (cnt - cmpr - margin) >= 0)
        {
            OALTimerSetCompare(cmpr);
        }
    }

    return rc;
}
