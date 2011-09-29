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
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  soc_profiler.c
//
//  This file contains SoC-specific routines to support the OAL profiler.
//  Timer1 is used for the profiling timer
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <nkintr.h>
#include <oal.h>
#include <intr.h>
#pragma warning(pop)

#include "csp.h"
#include "mx233_timrot.h"

//------------------------------------------------------------------------------
// High performance timer resoluton
// Use the 24MHz/8=3MHz clock.

#define HIGHPERF_FREQ                                   (24000000/8)    // 3 MHZ
#define HIGHPERF_TICKS_PER_1MS                  (HIGHPERF_FREQ/1000)

//------------------------------------------------------------------------------
// External Functions

//------------------------------------------------------------------------------
// External Variables
extern PVOID pv_HWregTIMROT;

//------------------------------------------------------------------------------
// Local Variables


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

BOOL OALProfileTimerEnable(hw_timer_InputSource_t eClock, UINT16 u16Fixedcnt)
{
    BOOL rc = FALSE;
    hw_timer_SetupStruct_t stTimerSetup;

    // Check timer input clock source
    if ( (eClock < 0) || (eClock > TIMER_SELECT_TICK_ALWAYS) )
        goto Cleanup;

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
    BF_WRn(TIMROT_TIMCTRLn, TIMER1, IRQ_EN,   stTimerSetup.bIrqEnable);
    BF_WRn(TIMROT_TIMCTRLn, TIMER1, POLARITY, stTimerSetup.bPolarity);
    BF_WRn(TIMROT_TIMCTRLn, TIMER1, UPDATE,   stTimerSetup.bUpdate);
    BF_WRn(TIMROT_TIMCTRLn, TIMER1, RELOAD,   stTimerSetup.bReload);
    BF_WRn(TIMROT_TIMCTRLn, TIMER1, PRESCALE, stTimerSetup.ePrescale);
    BF_WRn(TIMROT_TIMCTRLn, TIMER1, SELECT,   stTimerSetup.eSelect);

    //
    // Sets the timer FIXED_COUNT value
    //
    if ((stTimerSetup.bReload) && (stTimerSetup.u16FixedCount))
        BF_WRn(TIMROT_TIMCOUNTn, TIMER1, FIXED_COUNT, stTimerSetup.u16FixedCount-1);
    else
        BF_WRn(TIMROT_TIMCOUNTn, TIMER1, FIXED_COUNT, stTimerSetup.u16FixedCount);

    //
    rc = TRUE;

Cleanup:
    return rc;
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

    if( interval == 0)
    {
        // Use default rate of 1 ms
        interval = 1000;
    }
    else if( interval > 20000)
    {
        // Use maximum possible 20 ms
        interval = 20000;
    }

    // How many hi-res ticks per profiler hit
    g_profiler.countsPerHit = (HIGHPERF_TICKS_PER_1MS * interval) / 1000;

    OALMSG(TRUE, (L"OEMProfileTimerEnable: countsPerHit = %d\r\n", g_profiler.countsPerHit));

    // Following code should not be interrupted
    enabled = INTERRUPTS_ENABLE(FALSE);

    // Configure profiling ISR callback function.
    g_pProfilerISR = OALProfileIntrHandler;
/*
    // Obtain a pointer to the TIMROT registers.
    if (!pv_HWregTIMROT)
    {
        pv_HWregTIMROT = (PVOID) OALPAtoVA(CSP_BASE_REG_PA_TIMROT, FALSE);
    }
*/
    // Allow platform-specific configuration

    // MX233: Timer 1
    OALProfileTimerEnable(TIMER_SELECT_TICK_ALWAYS, (UINT16)g_profiler.countsPerHit);

    // clear Timer Interrupt
    BF_CLRn(TIMROT_TIMCTRLn, TIMER1, IRQ);

    // enable TIMER0 interrupt
    BF_WRn(TIMROT_TIMCTRLn, TIMER1, IRQ_EN, TIMER_INTERRUPT_ENABLE);

    // Enable interrupts
    INTERRUPTS_ENABLE(enabled);

    irq = IRQ_TIMER1;
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
    irq = IRQ_TIMER1;
    OALIntrDisableIrqs(1, &irq);

    // clear Timer Interrupt
    BF_CLRn(TIMROT_TIMCTRLn, TIMER1, IRQ);

    // disable TIMER1 interrupt
    BF_WRn(TIMROT_TIMCTRLn, TIMER1, IRQ_EN, TIMER_INTERRUPT_DISABLE);

    // Stop the timer
    BF_WRn(TIMROT_TIMCTRLn, TIMER1, SELECT,   TIMER_SELECT_NEVER_TICK);

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
    // Clear timer interrupt flag for the
    // next profile hit.
    BF_CLRn(TIMROT_TIMCTRLn, TIMER1, IRQ);

    // First call profiler
    ProfilerHit(ra);

    return(SYSINTR_NOP);
}

//------------------------------------------------------------------------------


