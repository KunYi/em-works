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
//  This file contains implementation of profiler module suitable for the
//  Intel PXA27x "Bulverde" CPU/SoC with count/compare timer.  The routines
//  use match register 2 (M2) for the profiling timer.
//
#include <windows.h>
#include <nkintr.h>
#include <oal.h>
#include <intr.h>
#include <bulverde.h>

//------------------------------------------------------------------------------
// Local Variables 

static XLLP_OST_HANDLE_T g_XllpOSTHandle;

static struct
{
    BOOL enabled;                               // is profiler active?
    UINT32 countsPerHit;                        // counts per profiler interrupt
} g_profiler;

//------------------------------------------------------------------------------
// External Variables 

extern PFN_PROFILER_ISR g_pProfilerISR;

//------------------------------------------------------------------------------
// Local Functions 
UINT32 OALProfileIntrHandler(UINT32 ra);
    
//------------------------------------------------------------------------------
//
//  Function:  OEMProfileTimerEnable
//
//  This function is called by kernel to start kernel profiling timer.
//
VOID OEMProfileTimerEnable(DWORD interval)
{
    BOOL enabled;
    UINT32 irq;
    
    OALMSG(TRUE, (L"+OEMProfileTimerEnable(%d)\r\n", interval));
    
    // We can't enable timer second time
    if (g_profiler.enabled) return;
   
    // How many hi-res ticks per profiler hit
    g_profiler.countsPerHit = (g_oalTimer.countsPerMSec * interval)/1000;

    // Following code should not be interrupted
    enabled = INTERRUPTS_ENABLE(FALSE);

    // Configure profiling ISR callback function.
    g_pProfilerISR = OALProfileIntrHandler;

    // Obtain pointers to OST and INTC registers.
    //
    g_XllpOSTHandle.pOSTRegs  = OALPAtoVA(BULVERDE_BASE_REG_PA_OST, FALSE);
    g_XllpOSTHandle.pINTCRegs = OALPAtoVA(BULVERDE_BASE_REG_PA_INTC, FALSE);

    // XLLI initializes oier and rtsr to zeroes, so no further
    // initialization needs to be done.  Match timers and
    // alarms are disabled.
    //
    // Current usage of Match registers:
    //  M0 - Scheduler
    //  M1 - Touch Panel
    //  M2 - Profiler

    // Update the compare register for the next profile hit.
    //
    XllpOstConfigureMatchReg(
        &g_XllpOSTHandle, MatchReg2, g_profiler.countsPerHit
    );

    // Enable interrupts
    INTERRUPTS_ENABLE(enabled);
    irq = IRQ_OSMR2;
    OALIntrDoneIrqs(1, &irq);

    // Set flag
    g_profiler.enabled = TRUE;

    OALMSG(TRUE, (L"-OEMProfileTimerEnable\r\n"));
}


//------------------------------------------------------------------------------
//
//  Function:  OEMProfileTimerDisable
//
//  This function is called by kernel to stop kernel profiling timer.
//

VOID OEMProfileTimerDisable() 
{
    BOOL enabled;
    UINT32 irq;

    OALMSG(TRUE, (L"+OEMProfileTimerDisable()\r\n"));

    // No disable without enable
    if (!g_profiler.enabled) goto cleanUp;

    // Following code should not be interrupted
    enabled = INTERRUPTS_ENABLE(FALSE);

    // Disable the profile timer interrupt
    irq = IRQ_OSMR2;
    OALIntrDisableIrqs(1, &irq);

    // Deconfigure profiling ISR callback function.
    g_pProfilerISR = NULL;

    // Reset flag
    g_profiler.enabled = FALSE;

    // Enable interrupts
    INTERRUPTS_ENABLE(enabled);

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
    UINT32 irq;

    // Update the compare register for the next profile hit.
    XllpOstConfigureMatchReg(
        &g_XllpOSTHandle, MatchReg2, g_profiler.countsPerHit
    );

    // First call profiler
    ProfilerHit(ra);

    // Enable interrupts
    irq = IRQ_OSMR2;
    OALIntrDoneIrqs(1, &irq);

    return SYSINTR_NOP; 
}

//------------------------------------------------------------------------------

