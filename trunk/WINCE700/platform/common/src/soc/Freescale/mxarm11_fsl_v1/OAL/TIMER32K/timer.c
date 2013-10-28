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

// Limit max period to 10 seconds to prevent time conversion macros
// below from overflowing.
#define EPIT_MAX_PERIOD                 (10*32768)

// RAW_TICKS_TO_MSEC converts from raw timer ticks to msec.  Right
// shift by 15 (divide by 32768) avoids divide.
#define RAW_TICKS_TO_MSEC(ticks)        (((ticks) * 1000) >> 15)

// RAW_TICKS_TO_MSEC_RND is the same as RAW_TICKS_TO_MSEC but rounds up.
#define RAW_TICKS_TO_MSEC_RND(ticks)    ((((ticks)+1) * 1000) >> 15)

// MSEC_TO_RAW_TICKS converts from msec to raw timer ticks.  Left
// shift by 15 (multiply by 32768) avoids multiply.
#define MSEC_TO_RAW_TICKS(msec)         (((msec) << 15) / 1000)

// MSEC_TO_RAW_TICKS_RND is the same as MSEC_TO_RAW_TICKS but rounds up.
#define MSEC_TO_RAW_TICKS_RND(msec)     ((((msec) << 15) + 500) / 1000)

// TIMER_ERROR_33_TICKS is the raw timer tick error introduced by using 33
// ticks for the timer interval (instead of the ideal 32.768).
#define TIMER_ERROR_33_TICKS            (33000-32768)

// TIMER_ERROR_32_TICKS is the raw timer tick error introduced by using 32
// ticks for the timer interval (instead of the ideal 32.768).
#define TIMER_ERROR_32_TICKS            (32768-32000)

//------------------------------------------------------------------------------
// Global Variables
PCSP_EPIT_REG g_pEPIT;


//------------------------------------------------------------------------------
// Local Variables
static UINT32 g_BaseTimerCount;
static INT32 g_TimerError;


//------------------------------------------------------------------------------
// Local Functions


//------------------------------------------------------------------------------
//
//  Function: OALTimerQueryPerformanceFrequency32k
//
//  This function returns the frequency of the high-resolution
//  performance counter.
//
BOOL OALTimerQueryPerformanceFrequency32k(LARGE_INTEGER *pFrequency)
{
    if (!pFrequency) return FALSE;

    pFrequency->HighPart = 0;
    pFrequency->LowPart = 32768;
    return TRUE;
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
    UINT32 countsPerSysTick;
    UINT32 irq, sysIntr;

    OALMSG(OAL_TIMER&&OAL_FUNC, (
        L"+OALTimerInit( %d, %d, %d )\r\n",
        msecPerSysTick, countsPerMSec, countsMargin
    ));

    // Validate Input parameters
    countsPerSysTick = countsPerMSec * msecPerSysTick;

    // Check for specialized support of 32.768 kHz time base
    if (msecPerSysTick != 1 || countsPerSysTick != 32)
    {
        OALMSG(OAL_ERROR, (
            L"ERROR: OALTimerInit: System tick period out of range(countsPerSysTick = %d)...\r\n",
        countsPerSysTick));
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
    g_oalTimer.maxPeriodMSec = ((UINT32) EPIT_MAX_PERIOD) / g_oalTimer.countsPerMSec;
    // g_oalTimer.maxPeriodMSec = 1;

    // Set kernel exported globals to initial values
    idleconv = 33;      // GetIdleTime will not be accurate (ideally we would
                        // set idleconv to 32.768) due to integer rounding,
                        // but it is better to use a conversion factor of 33
                        // to prevent 100%+ idle time calculations
    curridlehigh = 0;
    curridlelow = 0;

    // Initialize high resolution timer function pointers
    pQueryPerformanceFrequency = OALTimerQueryPerformanceFrequency32k;
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

    OALMSG(TRUE, (L"OALTimerInit:  32768 Hz CKIL support is enabled.\r\n"));

    // Initialize timer global for calculating counts since last system tick
    g_BaseTimerCount = INREG32(&g_pEPIT->CNT);

    // Initialize timer global for keeping track of offset from ideal 32.768 raw
    // tick interval.
    g_TimerError = 0;

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
    UINT32 period;

    if (g_oalILT.active) {
        if (--g_oalILT.counter == 0) {
            sysIntr = SYSINTR_TIMING;
            g_oalILT.counter = g_oalILT.counterSet;
            g_oalILT.isrTime2 = OALTimerCountsSinceSysTick();
        }
    }

    // Update timer globals
    g_oalTimer.curCounts += g_oalTimer.actualCountsPerSysTick;
    CurMSec += 1;

    // Since 32.768 kHz does not divide evenly to get a 1 msec OS tick
    // interval (32768 / 1000 = 32.768) we modulate between 32 and 33 raw
    // ticks to achieve an average 32.768 raw ticks.  The decision to use
    // 32 or 33 raw ticks is based on a running summation of the error
    // introduced by the raw tick interval chosen.

    // If current timer interval is using 32 raw ticks
    if (g_oalTimer.actualCountsPerSysTick == 32)
    {
        // Update error calculation. Current interval undershot ideal
        // 32.768 interval.
        g_TimerError -= TIMER_ERROR_32_TICKS;
    }
    // Else current timer interval is using 33 raw ticks
    else
    {
        // Update error calculation. Current interval overshot ideal
        // 32.768 interval.
        g_TimerError += TIMER_ERROR_33_TICKS;
    }

    // Use interval of 33 raw ticks (which is closer to 32.768 than
    // 32 raw ticks) unless the overshoot error introduced exceeds
    // undershoot error of using 32 raw ticks.
    if (g_TimerError < TIMER_ERROR_32_TICKS)
    {
        period = 33;
    }
    else
    {
        period = 32;
    }

    // Advance to next system tick
    OALTimerRecharge(period, g_oalTimer.countsMargin);

    // Keep track of the raw ticks used in the current OS tick
    g_oalTimer.actualCountsPerSysTick = period;

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
        g_oalTimer.actualCountsPerSysTick) return;

    // Limit the maximum idle time to what is supported.
    // Counter size is the limiting parameter.  When kernel
    // profiler or interrupt latency timing is active, it is set
    // to one system tick.
    if (idleMSec > g_oalTimer.maxPeriodMSec)
    {
        idleMSec = g_oalTimer.maxPeriodMSec;
    }

    // We can wait only full systick
    idleSysTicks = idleMSec;

    // This is idle time in hi-res ticks
    idleCounts = MSEC_TO_RAW_TICKS_RND(idleSysTicks);

    // Prolong beat period to idle time -- don't do it idle time isn't
    // longer than one system tick. Even if OALTimerExtendSysTick function
    // should accept this value it can cause problems if kernel profiler
    // or interrupt latency timing is active.
    if (idleSysTicks > 1)
    {
        // Extend timer period.
        OALTimerUpdate(idleCounts, g_oalTimer.countsMargin);
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

        // Switch back to 33 raw ticks (which is closer to 32.768 than 32 raw
        // ticks).  Any error introduced by overshooting will be accounted
        // for in OALTimerIntrHandler.
        g_oalTimer.actualCountsPerSysTick = 33;

        // Restore original system tick and expired ticks
        idleSysTicks = OALTimerUpdate(g_oalTimer.actualCountsPerSysTick,
            g_oalTimer.countsMargin);

        // Adjust system timer OAL variables.  Note expired system ticks
        // returned by OALTimerUpdate is rounded down to nearest system tick.
        CurMSec += (idleSysTicks);
        g_oalTimer.curCounts += MSEC_TO_RAW_TICKS_RND(idleSysTicks);
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

    // Set the new compare value
    OUTREG32(&g_pEPIT->CMPR, compare);
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
    UINT32 cnt, cmpr, msecOffset, rawTickOffset;

    // Grab current timer counter
    cnt = INREG32(&g_pEPIT->CNT);

    // Advance base for current system tick to beginning of next system tick.
    // Adjustment must be based on raw ticks of the previous OS tick
    // interval since period is being modulated by OALTimerIntrHandler.
    g_BaseTimerCount -= g_oalTimer.actualCountsPerSysTick;

    // Set timer compare to end of next system tick
    cmpr = g_BaseTimerCount - period;

    // If requested timer compare value is invalid
    if ((INT32) (cnt - cmpr - margin) < 0)
    {
        // Calculate number of full msec we have fallen behind
        msecOffset = RAW_TICKS_TO_MSEC(g_BaseTimerCount - cnt);

        // Calculate raw tick offset from msec offset
        rawTickOffset = MSEC_TO_RAW_TICKS(msecOffset);

        // Advance the timer globals to prevent clock drift
        CurMSec += msecOffset;
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
            rc = RAW_TICKS_TO_MSEC_RND(expired);

            // Advance base for current system tick to beginning of next system tick
            g_BaseTimerCount -= MSEC_TO_RAW_TICKS_RND(rc);

            // Clamp base to prevent overflow
            if ((g_BaseTimerCount - cnt) > EPIT_MAX_PERIOD)
            {
                g_BaseTimerCount = cnt;
            }
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

