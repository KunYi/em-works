//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------
//
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  Module: timer.c
//
//  Interface to OAL timer services.
//
//-----------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#include <oal.h>
#pragma warning(pop)

#include "common_macros.h"
#include "common_epit.h"

//-----------------------------------------------------------------------------
// External Functions
extern UINT32 OALTimerGetIrq(void);
extern UINT32 OALTimerGetBaseRegAddr(void);
extern UINT32 OALTimerGetClkSrc(void);
extern UINT32 OALTimerGetClkFreq(void);
extern UINT32 OALTimerGetClkPrescalar(void);
#ifdef ENABLE_WATCH_DOG
    extern void InitWatchDogTimer (void);  
#endif


//-----------------------------------------------------------------------------
// Defines

// Limit period to max positive 32-bit offset
#define EPIT_MAX_PERIOD                 (g_bTimer32K ?  (0x7FFFFFFF / 1000) : (0x7FFFFFFF))

// RAW_TICKS_TO_MSEC converts from raw timer ticks to msec.
#define RAW_TICKS_TO_MSEC(ticks)        (g_bTimer32K ? (((ticks) * 1000) >> 15) : ((ticks) / g_oalTimer.countsPerMSec))

// MSEC_TO_RAW_TICKS converts from msec to raw timer ticks.
#define MSEC_TO_RAW_TICKS(msec)         (g_bTimer32K ? (((msec) << 15) / 1000) : ((msec) * g_oalTimer.countsPerMSec))


//-----------------------------------------------------------------------------
// Global Variables
PCSP_EPIT_REG g_pEPIT;

//-----------------------------------------------------------------------------
//
//  Global: g_oalTimer    
//
//  This is global instance of timer control block.
//
OAL_TIMER_STATE g_oalTimer;

//-----------------------------------------------------------------------------
// Local Variables
static volatile UINT32 g_BaseTimerCount;
static volatile UINT32 g_UnusedTimerCount;
static BOOL g_bTimer32K = FALSE;


//-----------------------------------------------------------------------------
// Local Functions

//-----------------------------------------------------------------------------
//
//  Function: OEMGetTickCount
//
//  This returns the number of milliseconds that have elapsed since Windows 
//  CE was started. If the system timer period is 1ms the function simply 
//  returns the value of CurMSec. If the system timer period is greater then
//  1 ms, the HiRes offset is added to the value of CurMSec.
//
//  Parameters:
//      None.
//
//  Returns:
//      The number of milliseconds that have elapsed since the system was 
//      started indicates success.
//
//-----------------------------------------------------------------------------
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
       base = g_BaseTimerCount;
       count = CurMSec;
       unused = g_UnusedTimerCount;
       offset = OALTimerCountsSinceSysTick();
    } while ((base != g_BaseTimerCount));

    count += RAW_TICKS_TO_MSEC(unused+offset);

    return count;
}


//-----------------------------------------------------------------------------
//
//  Function: OALQueryPerformanceCounter
//
//  This function returns the current value of the high-resolution 
//  performance counter.
//  
//  Parameters:
//      pCounter
//          [out] High-resolution performance counter value.
//
//  Returns:
//      TRUE indicates success.  FALSE indicates failure.
//
//-----------------------------------------------------------------------------
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


//-----------------------------------------------------------------------------
//
//  Function: OALTimerQueryPerformanceFrequency
//
//  This function returns the frequency of the high-resolution 
//  performance counter.
//
//  Parameters:
//      pFrequency
//          [out] High-resolution performance frequency value.
//
//  Returns:
//      TRUE indicates success.  FALSE indicates failure.
//
//-----------------------------------------------------------------------------
BOOL OALTimerQueryPerformanceFrequency(LARGE_INTEGER *pFrequency)
{
    if (!pFrequency) return FALSE;

    pFrequency->HighPart = 0;
    if (g_bTimer32K)
    { 
        pFrequency->LowPart = 32768;
    }
    else
    {
        pFrequency->LowPart = 1000 * g_oalTimer.countsPerMSec;
    }
    return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function:  OALTimerUpdateRescheduleTime
//
//  This function is called by kernel to set next reschedule time.
//
//  Parameters:
//      time
//          [in] Time that the system timer needs to be programmed to.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID OALTimerUpdateRescheduleTime(DWORD time)
{
    UINT32 baseMSec, diffMSec, diffCounts;
    INT32 counts;
    BOOL bIntrEnable;
    
    // Get current system timer counter
    baseMSec = CurMSec;

    // Return if we are already setup correctly
    if (time == (baseMSec + g_oalTimer.actualMSecPerSysTick)) goto cleanUp;

    // Calculate how far we are from next tick
    counts = g_oalTimer.actualCountsPerSysTick - OALTimerCountsSinceSysTick();

    // If timer interrupt occurs, or we are within 1 ms of the scheduled
    // interrupt, just return - timer ISR will take care of it.
    if (baseMSec != CurMSec || counts < (INT32)g_oalTimer.countsPerMSec) {
        goto cleanUp;
    }        

    // Calculate the distance between the new time and the last timer interrupt
    diffMSec = time - baseMSec;

    // Trying to set reschedule time prior or equal to CurMSec - this could
    // happen if a thread is on its way to sleep while preempted before
    // getting into the Sleep Queue
    if ((INT32)diffMSec < 0) diffMSec = 0;

    // Clamp distance to next reschedule time 
    if (diffMSec > g_oalTimer.maxPeriodMSec) {
        diffMSec = g_oalTimer.maxPeriodMSec;
    }        

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
    UINT32 clkSrc, clkFreq, prescalar;

    OALMSG(OAL_TIMER&&OAL_FUNC, (
        L"+OALTimerInit( %d, %d, %d )\r\n",
        msecPerSysTick, countsPerMSec, countsMargin
    ));

    pOEMUpdateRescheduleTime = OALTimerUpdateRescheduleTime;


    // Validate Input parameters
    countsPerSysTick = countsPerMSec * msecPerSysTick;

    // Check if 32K timer support is needed
    clkFreq = OALTimerGetClkFreq();
    clkSrc = OALTimerGetClkSrc();
    if (clkFreq == 32768)
    {
        g_bTimer32K = TRUE;
        prescalar = 0;

        OALMSG(TRUE, (L"OALTimerInit:  32768 Hz CKIL support is enabled.\r\n"));

        // Check for specialized support of 32.768 kHz time base 
        if (msecPerSysTick < 1 || countsPerSysTick != 32)
        {
            OALMSG(OAL_ERROR, (
                L"ERROR: OALTimerInit: System tick period out of range(countsPerSysTick = %d)...\r\n",
            countsPerSysTick));
            goto cleanUp;
        }
    }
    else
    {        
        prescalar = OALTimerGetClkPrescalar();
        
        if (msecPerSysTick < 1 ||
            countsPerSysTick < 1 || 
            countsPerSysTick > EPIT_CNT_COUNT_MAX)
        {
            OALMSG(OAL_ERROR, (
                L"ERROR: OALTimerInit: System tick period out of range..."
            ));
            goto cleanUp;
        }
    }

    // Initialize timer state global variables
    g_oalTimer.msecPerSysTick = msecPerSysTick;
    g_oalTimer.actualMSecPerSysTick = msecPerSysTick;
    g_oalTimer.countsPerMSec = countsPerMSec;
    g_oalTimer.countsMargin = countsMargin;
    g_oalTimer.countsPerSysTick = countsPerSysTick;
    g_oalTimer.actualCountsPerSysTick = countsPerSysTick;
    g_oalTimer.curCounts = 0;
    g_oalTimer.maxPeriodMSec = RAW_TICKS_TO_MSEC(EPIT_MAX_PERIOD);

    // Set kernel exported globals to initial values
    if (g_bTimer32K)
    {
        idleconv = 32768;   // Use a conversion factor of 32768 to avoid rounding 
                            // error in GetIdleTime.  During OEMIdle, we will
                            // scale the raw idle counts by 1000 to compensate
                            // for this conversion to seconds (normally idleconv
                            // specifies a conversion to msec)
    }
    else
    {
        idleconv = countsPerMSec;
    }
    curridlehigh = 0;
    curridlelow = 0;

    // Initialize high resolution timer function pointers
    pQueryPerformanceFrequency = OALTimerQueryPerformanceFrequency;
    pQueryPerformanceCounter = OALTimerQueryPerformanceCounter;

    // Request IRQ to SYSINTR mapping
    irq = OALTimerGetIrq();
    sysIntr = OALIntrRequestSysIntr(1, &irq, OAL_INTR_FORCE_STATIC);

    // Make sure we have a valid sysIntr
    if (sysIntr == SYSINTR_UNDEFINED)
    {
        OALMSG(OAL_ERROR, (L"+OALTimerInit:  SYSINTR_UNDEFINED\r\n"));
        goto cleanUp;
    }

    // Get uncached virtual addresses for system timer
    g_pEPIT = (PCSP_EPIT_REG) OALPAtoUA(OALTimerGetBaseRegAddr());
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
        CSP_BITFVAL(EPIT_CR_PRESCALAR, prescalar) |
        CSP_BITFVAL(EPIT_CR_SWR, EPIT_CR_SWR_NORESET) |
        CSP_BITFVAL(EPIT_CR_IOVW, EPIT_CR_IOVW_NOOVR) |
        CSP_BITFVAL(EPIT_CR_DBGEN, EPIT_CR_DBGEN_ACTIVE) |
        CSP_BITFVAL(EPIT_CR_WAITEN, EPIT_CR_WAITEN_ENABLE) |
        CSP_BITFVAL(EPIT_CR_DOZEN, EPIT_CR_DOZEN_ENABLE) |
        CSP_BITFVAL(EPIT_CR_STOPEN, EPIT_CR_STOPEN_ENABLE) |
        CSP_BITFVAL(EPIT_CR_OM, EPIT_CR_OM_DICONNECT) |
        CSP_BITFVAL(EPIT_CR_CLKSRC, clkSrc));

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
        InitWatchDogTimer ();
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
    UINT32 sysIntr = SYSINTR_NOP, delta, msec;
    INT32 msecToNextResched;
    
#ifdef OAL_ILTIMING
    if (g_oalILT.active) {
        g_oalILT.isrTime1 = INREG32(&g_pEPIT->CMPR) - INREG32(&g_pEPIT->CNT);
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
    if (g_oalILT.active) {
        g_oalILT.counter--;
        if (g_oalILT.counter == 0) {
            sysIntr = SYSINTR_TIMING;
            g_oalILT.counter = g_oalILT.counterSet;
            g_oalILT.isrTime2 = OALTimerCountsSinceSysTick();
        }
    }
#endif

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
//-----------------------------------------------------------------------------
void OEMIdle(DWORD dwIdleParam)
{
    INT32 countBeforeIdle, countAfterIdle, idleCounts;
    ULARGE_INTEGER idle;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(dwIdleParam);

    // Find how many hi-res ticks were are consumed in the current system tick
    // before idle mode is entered
    countBeforeIdle = OALTimerCountsSinceSysTick();
    
    // Move SoC/CPU to idle mode
    OALCPUIdle();

    // Find how many hi-res ticks were are consumed in the current system tick
    // after idle mode is exited
    countAfterIdle = OALTimerCountsSinceSysTick();
            
    // Get real idle value. If result is negative we didn't idle at all.
    idleCounts = countAfterIdle - countBeforeIdle;
    if (idleCounts < 0) 
    {
        idleCounts = 0;
    }
    else
    {
        if (g_bTimer32K)
        {
            // Scale the idle counts by 1000 since we set the idleconv
            // global to 32768 to avoid rounding error during GetIdleTime. 
            idleCounts = idleCounts * 1000;
        }
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
VOID OALTimerSetCompare(UINT32 compare)
{
    // Clear timer compare interrupt flag (write-1-clear)
    OUTREG32(&g_pEPIT->SR, CSP_BITFMASK(EPIT_SR_OCIF));

    // Set the new compare value
    OUTREG32(&g_pEPIT->CMPR, compare);
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
    cnt = INREG32(&g_pEPIT->CNT);

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
