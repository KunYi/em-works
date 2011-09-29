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
//  File:  profiler.c
//
//  This file contains implementation of profiler module suitable for MIPS
//  CPU/SoC with count/compare CP0 timer. It probably can be used on most 
//  if not all MIPS platforms for kernel profiling.
//
#include <windows.h>
#include <nkintr.h>
#include <oal.h>
#include <oal_intr_mips.h>

//------------------------------------------------------------------------------
// Local Variables 

static struct {
    BOOL enabled;                               // is profiler active?
    UINT32 countsPerHit;                        // counts per profiler interrupt
    UINT32 counts;                              // counts in system tick
    UINT32 partialCounts;                              // partial counts in CurMSec
    UINT32 totalPartialCounts;                              // total partial counts for CurMSec accumulation
    UINT32 maxPeriodMSec;                       // saved maximal timer period
    VOID (*pUpdateRescheduleTime)(DWORD);       // saved function pointer
} g_profiler;

//------------------------------------------------------------------------------
//
//  Function:  OEMProfileTimerEnable
//
//  This function is called by kernel to start kernel profiling timer.
//
VOID OEMProfileTimerEnable(DWORD interval)
{
    BOOL enabled;
    
    OALMSG(TRUE, (L"+OEMProfileTimerEnable(%d)\r\n", interval));
    
    // We can't enable timer second time
    if (g_profiler.enabled) return;

    // How many hi-res ticks per profiler hit
    g_profiler.countsPerHit = (g_oalTimer.countsPerMSec * interval)/1000;

    // Make sure that value isn't too small
    if (g_profiler.countsPerHit < 8 * g_oalTimer.countsMargin) {
        g_profiler.countsPerHit = 8 * g_oalTimer.countsMargin;
    }

    // Following code should not be interrupted
    enabled = INTERRUPTS_ENABLE(FALSE);

    // if system run with variable system tick, we are going to force it to run in fix-tick
    if (pOEMUpdateRescheduleTime != NULL) {

        // save the pOEMUpdateRescheduleTime function pointer and set it to NULL, so that the system will run in fix-tick
        g_profiler.pUpdateRescheduleTime = pOEMUpdateRescheduleTime;
        pOEMUpdateRescheduleTime = NULL;

        // Avoid idle longer than system tick, so set value to one system tick
        g_profiler.maxPeriodMSec = g_oalTimer.maxPeriodMSec;
        g_oalTimer.maxPeriodMSec = g_oalTimer.msecPerSysTick;
    }

    // Update the system timer variables
    g_oalTimer.actualCountsPerSysTick = g_profiler.countsPerHit;
    g_oalTimer.actualMSecPerSysTick = g_profiler.countsPerHit / g_oalTimer.countsPerMSec;
    g_profiler.partialCounts = g_profiler.countsPerHit % g_oalTimer.countsPerMSec;
    g_profiler.totalPartialCounts = 0;

    // start profiler counter
    g_profiler.counts = 0;

    // Hook profiler interrupt service routing
    UnhookInterrupt(5, OALTimerIntrHandler);
    HookInterrupt(5, OALProfileIntrHandler);

    // Enable interrupts
    INTERRUPTS_ENABLE(enabled);

    // Set flag
    g_profiler.enabled = TRUE;

    OALMSG(TRUE, (L"-OEMProfileTimerEnable\r\n"));
}

//------------------------------------------------------------------------------
//
//  Function:  OEMProfileTimerEnable
//
//  This function is called by kernel to stop kernel profiling timer.
//

VOID OEMProfileTimerDisable() 
{
    BOOL enabled;

    OALMSG(TRUE, (L"+OEMProfileTimerDisable()\r\n"));

    // No disable without enable
    if (!g_profiler.enabled) goto cleanUp;

    // Following code should not be interrupted
    enabled = INTERRUPTS_ENABLE(FALSE);

    // since we convert var-tick system to fix-tick, we'll change it back once we quit profiling
    if (g_profiler.pUpdateRescheduleTime != NULL) {
        pOEMUpdateRescheduleTime = g_profiler.pUpdateRescheduleTime;
        g_profiler.pUpdateRescheduleTime = NULL;
        // Restore maximal idle
        g_oalTimer.maxPeriodMSec = g_profiler.maxPeriodMSec;
    }

    // Hook original timer interrupt service routine
    UnhookInterrupt(5, OALProfileIntrHandler);
    HookInterrupt(5, OALTimerIntrHandler);

    // Enable interrupts
    INTERRUPTS_ENABLE(enabled);

    // Reset flag
    g_profiler.enabled = FALSE;

cleanUp:
    OALMSG(TRUE, (L"-OEMProfileTimerDisable\r\n"));
}

//------------------------------------------------------------------------------
//
//  Function:  OALProfileIntrHandler
//
//  This is timer interrupt handler which replace default handler in time when
//  kernel profiling is active. It calls original interrupt handler in
//  appropriate times.
//
UINT32 OALProfileIntrHandler()
{
    UINT32 sysIntr=SYSINTR_NOP;
    UINT32 timerCount, timerMatch;

    // First call profiler
    ProfilerHit(GetEPC());

    timerCount = OALTimerGetCount();
    timerMatch = OALTimerGetCompare();

    if (timerCount < timerMatch) return sysIntr; //it is fake intr, return

    // Get new counts value
    g_profiler.counts += g_profiler.countsPerHit;

    OALTimerSetCompare( timerMatch + g_profiler.countsPerHit );

    // Update high resolution counter
    g_oalTimer.curCounts += g_profiler.countsPerHit;
    CurMSec += g_oalTimer.actualMSecPerSysTick;
    g_profiler.totalPartialCounts += g_profiler.partialCounts;
    if (g_profiler.totalPartialCounts > g_oalTimer.countsPerMSec)
    {
        CurMSec++;
        g_profiler.totalPartialCounts -= g_oalTimer.countsPerMSec;
    }

    // Reschedule ?
    if ((int)(CurMSec - dwReschedTime) >= 0)
        sysIntr = SYSINTR_RESCHED;

    return sysIntr;                                     
}

//------------------------------------------------------------------------------

