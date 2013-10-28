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
    UINT32 hitsPerSysTick;                     // number of times we'll call profiler ISR before calling timer ISR
    UINT32 hitsLeftUntilSysTick;                // when this is 0 we reset it and call the timer ISR
    VOID (*pUpdateRescheduleTime)(DWORD);       // saved function pointer
} g_profiler;

//------------------------------------------------------------------------------
//
//  Function:  OEMProfileTimerEnable
//
//  This function is called by kernel to start kernel profiling timer.
//  It allows the MIPS CP0 count/compare registers to be used both for 
//  the system tick and the profiler.
//
VOID OEMProfileTimerEnable(DWORD interval)
{
    BOOL enabled;
    DWORD originalInterval = interval;
    
    OALMSG(TRUE, (L"+OEMProfileTimerEnable(%d)\r\n", interval));
    
    // We can't enable timer second time
    if (g_profiler.enabled) return;

    // Make sure the interval isn't too small for our timer margin (it should be at least 8 times the margin)
    // countsmargin * 8 * 1000us/msec 
    if (interval < ((8 * 1000 * g_oalTimer.countsMargin) / g_oalTimer.countsPerMSec))
    {
        OALMSG(TRUE, (L"OEMProfileTimerEnable(%d), interval too small for margin\r\n", interval));
        interval = ((8 * 1000 * g_oalTimer.countsMargin) / g_oalTimer.countsPerMSec);
    }

    // Low resolution profiling is not supported as it would disrupt our system tick
    if (interval > g_oalTimer.msecPerSysTick * 1000) return;

    // Find the closest value that divides evenly into the system tick
    // Because we're using the same clock for the profiler and system tick, if we choose
    // a number that is not evenly divisible, we will get jitter in the system tick.
    while((g_oalTimer.msecPerSysTick * 1000) % interval && interval < g_oalTimer.msecPerSysTick * 1000)
    {
        interval++;
    }

    if(originalInterval != interval)
    {
        OALMSG(TRUE, (L"OEMProfileTimerEnable(%d), interval changed to %d\r\n", originalInterval, interval));
    }

    g_profiler.hitsPerSysTick = (g_oalTimer.msecPerSysTick * 1000) / interval;
    g_profiler.countsPerHit = (g_oalTimer.countsPerMSec * interval) / 1000;

    // Following code should not be interrupted
    enabled = INTERRUPTS_ENABLE(FALSE);

    // if system run with variable system tick, we are going to force it to run in fix-tick
    if (pOEMUpdateRescheduleTime != NULL) {

        // save the pOEMUpdateRescheduleTime function pointer and set it to NULL, so that the system will run in fix-tick
        g_profiler.pUpdateRescheduleTime = pOEMUpdateRescheduleTime;
        pOEMUpdateRescheduleTime = NULL;
    }

    // Profiler will start on next timer interrupt, which will be on a system tick
    g_profiler.hitsLeftUntilSysTick = 0;

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
     }

    // Hook original timer interrupt service routine
    UnhookInterrupt(5, OALProfileIntrHandler);
    HookInterrupt(5, OALTimerIntrHandler);

    // If needed, reset the timer interrupt to the next system tick
    if(g_profiler.hitsLeftUntilSysTick != 0)
    {
        UINT32 timerCount = OALTimerGetCount();
        UINT32 timerMatch = OALTimerGetCompare();

        if (((INT32)(timerCount + g_oalTimer.countsMargin - timerMatch -
                (g_profiler.countsPerHit * g_profiler.hitsLeftUntilSysTick))  >  0)) {
            // We exceed the timer margin, just get as close as possible.  This can cause a small amount of
            // drift in the system tick vs. wall-clock time for fixed-tick implementation.
            OALTimerSetCompare( OALTimerGetCount() + g_profiler.countsPerHit );
        } else {
            // We have not exceeded the timer margin
            OALTimerSetCompare( timerMatch + g_profiler.countsPerHit * g_profiler.hitsLeftUntilSysTick );
        }    
    }
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
UINT32 OALProfileIntrHandler(UINT32 ra)
{
    UINT32 sysIntr=SYSINTR_NOP;
    UINT32 timerCount, timerMatch;

    // First call profiler
    ProfilerHit(ra);

    timerCount = OALTimerGetCount();
    timerMatch = OALTimerGetCompare();

    if ((INT32)(timerCount - timerMatch) < 0) return sysIntr; //it is fake intr, return

     // Is it time to call timer ISR?
    if(g_profiler.hitsLeftUntilSysTick == 0)
    {
        sysIntr = OALTimerIntrHandler();
        g_profiler.hitsLeftUntilSysTick = g_profiler.hitsPerSysTick;
    }

    g_profiler.hitsLeftUntilSysTick--;

    if (((INT32)(timerCount + g_oalTimer.countsMargin - timerMatch - g_profiler.countsPerHit)  >  0)) {
        // We exceed the timer margin, just get as close as possible.  This can cause a small amount of
        // drift in the system tick vs. wall-clock time for fixed-tick implementation.
        OALTimerSetCompare( OALTimerGetCount() + g_profiler.countsPerHit );
    } else {
        // We have not exceeded the timer margin
        OALTimerSetCompare( timerMatch + g_profiler.countsPerHit );
    }

    return sysIntr;                                     
}

//------------------------------------------------------------------------------

