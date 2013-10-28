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
// Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  profiler.c
//
//  This file contains implementation of profiler module suitable for the
//  i.MX27 with count/compare timer. The routines use GPT3 for the profiling timer.
//
//------------------------------------------------------------------------------
#include <windows.h>
#include <nkintr.h>
#include <oal.h>
#include <intr.h>
#include "csp.h"

//------------------------------------------------------------------------------
// External Functions
extern UINT32 OALTimerGetClkSrc(void);
extern UINT32 OALTimerGetClkPrescalar(void);
extern VOID OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX index, DDK_CLOCK_GATE_MODE mode);

//------------------------------------------------------------------------------
// Local Variables 
static CSP_GPT_REGS *g_pGPT = NULL;

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
//  Function:  ConfigureNextProfilerCount
//
//  Updates the profiler count (prepares the timer for the next timer event).
//
static void ConfigureNextProfilerCount(DWORD dwCountInterval)
{

    DWORD dwTimerTemp;

	
    if (!g_pProfilerISR) return;

    // Change number of timer ticks in the period.
    
    g_pGPT->TCMP = dwCountInterval;
    
    // Clear timer compare interrupt flag (write-1-clear)
    OUTREG32(&g_pGPT->TSTAT, CSP_BITFMASK(GPT_TSTAT_COMP));

    dwTimerTemp = INREG32(&g_pGPT->TCTL) & (~ CSP_BITFVAL(GPT_TCTL_TEN, GPT_TCTL_TEN_ENABLE));

    OUTREG32(&g_pGPT->TCTL, dwTimerTemp); // Stop and Reset the counter 

    OUTREG32(&g_pGPT->TCTL, dwTimerTemp  // Start timer
		|CSP_BITFVAL(GPT_TCTL_TEN, GPT_TCTL_TEN_ENABLE));	
}

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


   // Enable IPG clock input to GPT3 module
    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_GPT3, DDK_CLOCK_GATE_MODE_ENABLE);
   
    // Obtain a pointer to the GPT registers.
    if (!g_pGPT)
    {
        g_pGPT = (CSP_GPT_REGS *)OALPAtoUA(CSP_BASE_REG_PA_GPT3);
    }

   // Init the GPT3
    // Disable GPT
    g_pGPT->TCTL = 0;
   
    // Software reset GPT3
    OUTREG32(&g_pGPT->TCTL, CSP_BITFMASK(GPT_TCTL_SWR));

    // Wait for the software reset to complete
    while (INREG32(&g_pGPT->TCTL) & CSP_BITFMASK(GPT_TCTL_SWR));	

    g_pGPT->TPRER = OALTimerGetClkPrescalar();

    OUTREG32(&g_pGPT->TCTL,
        CSP_BITFVAL(GPT_TCTL_TEN, GPT_TCTL_TEN_DISABLE) |
        CSP_BITFVAL(GPT_TCTL_CLKSOURCE, OALTimerGetClkSrc()) |
        CSP_BITFVAL(GPT_TCTL_COMPEN, GPT_TCTL_COMPEN_ENABLE) |
        CSP_BITFVAL(GPT_TCTL_CAPTEN, GPT_TCTL_CAPTEN_DISABLE) |
        CSP_BITFVAL(GPT_TCTL_CAP, GPT_TCTL_CAP_DISABLE) |
        CSP_BITFVAL(GPT_TCTL_FRR, GPT_TCTL_FRR_FREERUN) |
        CSP_BITFVAL(GPT_TCTL_OM, GPT_TCTL_OM_ACTIVELOW) |
        CSP_BITFVAL(GPT_TCTL_CC, GPT_TCTL_CC_ENABLE));

    // How many hi-res ticks per profiler hit
    g_profiler.countsPerHit = (g_oalTimer.countsPerMSec * interval)/1000;

    // Following code should not be interrupted
    enabled = INTERRUPTS_ENABLE(FALSE);

    // Configure profiling ISR callback function.
    g_pProfilerISR = OALProfileIntrHandler;

    // Update the compare register for the next profile hit.
    ConfigureNextProfilerCount(g_profiler.countsPerHit);

    // Enable interrupts
    INTERRUPTS_ENABLE(enabled);
    irq = IRQ_GPT3;
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

    // Disable IPG clock input to GPT3 module
    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_GPT3, DDK_CLOCK_GATE_MODE_DISABLE);


    // Following code should not be interrupted
    enabled = INTERRUPTS_ENABLE(FALSE);

    // Disable the profile timer interrupt
    irq = IRQ_GPT3;
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
    ConfigureNextProfilerCount(g_profiler.countsPerHit);

    // First call profiler
    ProfilerHit(ra);

    // Enable interrupts
    irq = IRQ_GPT3;
    OALIntrDoneIrqs(1, &irq);

    return(SYSINTR_NOP); 
}

//------------------------------------------------------------------------------

