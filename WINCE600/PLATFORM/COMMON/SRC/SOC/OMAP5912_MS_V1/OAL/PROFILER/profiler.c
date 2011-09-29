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
//  This file contains implementation of the functions required for Monte-Carlo
//  profiling.
//
#include <windows.h>
#include <nkintr.h>
#include <oal.h>
#include <omap5912.h>

//------------------------------------------------------------------------------
//
//  Global:  g_oalProfilerIrq
//
//  IRQ of the timer used for profiling. Set by OEMProfileTimerEnable.
//
UINT32 g_oalProfilerIrq = OAL_INTR_IRQ_UNDEFINED;

//------------------------------------------------------------------------------
//
//  Global:  g_oalProfilerEnabled
//
//  Indicates that profiling is enabled. Set by OEMProfileTimerEnable.
//
BOOL g_oalProfilerEnabled = FALSE;

//------------------------------------------------------------------------------
//
//  Function:  OEMProfileTimerEnable
//
//  This function is called by kernel to start kernel profiling timer.
//
VOID OEMProfileTimerEnable(DWORD interval)
{
    BOOL enabled;
    OMAP5912_TIMER_REGS *pTimer;
    
    OALMSG(TRUE, (L"+OEMProfileTimerEnable(%d)\r\n", interval));
    
    // We can't enable timer second time
    if (g_oalProfilerEnabled)
        {
        return;
        }

    // Inform interrupt module
    g_oalProfilerIrq = IRQ_TIMER1;

    // Following code should not be interrupted
    enabled = INTERRUPTS_ENABLE(FALSE);

    // Setup timer 1 for profiling
    pTimer = OALPAtoVA(OMAP5912_TIMER1_REGS_PA, FALSE);
    OUTREG32(&pTimer->LOAD, (65U * interval)/10U);
    OUTREG32(&pTimer->CNTL, (TIMER_CNTL_FREE | TIMER_CNTL_PVT_2 | TIMER_CNTL_CLK_EN |
                    TIMER_CNTL_AR | TIMER_CNTL_ST));
    
    // Set flag
    g_oalProfilerEnabled = TRUE;

    // Enable interrupts
    INTERRUPTS_ENABLE(enabled);

    // Enable profiling interrupt
    OALIntrEnableIrqs(1, &g_oalProfilerIrq);

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
    OMAP5912_TIMER_REGS *pTimer;

    OALMSG(TRUE, (L"+OEMProfileTimerDisable()\r\n"));

    // No disable without enable
    if (!g_oalProfilerEnabled)
        {
        goto cleanUp;
        }

    // Stop timer 1
    pTimer = OALPAtoVA(OMAP5912_TIMER1_REGS_PA, FALSE);
    CLRREG32(&pTimer->CNTL, TIMER_CNTL_ST);
    
    // Following code should not be interrupted
    enabled = INTERRUPTS_ENABLE(FALSE);

    // Inform interrupt module
    g_oalProfilerIrq = OAL_INTR_IRQ_UNDEFINED;

    // Disable the profile timer interrupt
    OALIntrDisableIrqs(1, &g_oalProfilerIrq);

    // Reset flag
    g_oalProfilerEnabled = FALSE;

    // Enable interrupts
    INTERRUPTS_ENABLE(enabled);

cleanUp:
    OALMSG(TRUE, (L"-OEMProfileTimerDisable\r\n"));
}

//------------------------------------------------------------------------------

