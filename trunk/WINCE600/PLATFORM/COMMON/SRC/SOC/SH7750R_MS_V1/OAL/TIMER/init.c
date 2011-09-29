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
//  File: init.c
//  
//  This file contains timer functions implementation for count/compare
//  counters.
//
#include <windows.h>
#include <shx.h>
#include <nkintr.h>
#include <oal_log.h>
#include <oal_timer.h>
#include <oal_memory.h>
#include <oal_io.h>
#include <oal.h>

//------------------------------------------------------------------------------
//
BOOL OALTimerInit(UINT32 msecPerSysTick, UINT32 countsPerMSec, UINT32 countsMargin)
{
    BOOL          retVal   = FALSE;
    SH4_TMU_REGS *pTMURegs = OALPAtoUA(SH4_REG_PA_TMU);
    UINT32        count    = msecPerSysTick * countsPerMSec;

    OALMSG(OAL_FUNC, (L"+OALTimerInit(0x%08x, 0x%08x, 0x%08x)\r\n", 
                      msecPerSysTick, countsPerMSec, countsMargin));

    // Stop Timer
    OUTREG8(&pTMURegs->TSTR, INREG8(&pTMURegs->TSTR) & ~TMU_TSTR_STR0);

    // Set the input frequency to Peripheral / 16
    OUTREG16(&pTMURegs->TCR0, TMU_TCR_D16);

    // Enable underflow interrupts
    OUTREG16(&pTMURegs->TCR0, INREG16(&pTMURegs->TCR0) | TMU_TCR_UNIE);

    // Clear any pending interrupts
    OUTREG16(&pTMURegs->TCR0, INREG16(&pTMURegs->TCR0) & ~TMU_TCR_UNF);

    g_oalTimer.countsPerMSec          = countsPerMSec;
    g_oalTimer.countsMargin           = countsMargin;
    g_oalTimer.maxPeriodMSec          = UINT_MAX / countsPerMSec;
    g_oalTimer.msecPerSysTick         = msecPerSysTick;
    g_oalTimer.countsPerSysTick       = g_oalTimer.countsPerMSec * g_oalTimer.msecPerSysTick; 
    g_oalTimer.actualMSecPerSysTick   = g_oalTimer.msecPerSysTick;
    g_oalTimer.actualCountsPerSysTick = g_oalTimer.countsPerSysTick;

    idleconv = g_oalTimer.countsPerMSec;
    curridlehigh = curridlelow = 0;

   // Initialize high resolution timer function pointers
    pQueryPerformanceFrequency = OALTimerQueryPerformanceFrequency;
    pQueryPerformanceCounter   = OALTimerQueryPerformanceCounter;

    // Update the reload value
    OUTREG32(&pTMURegs->TCOR0, g_oalTimer.countsPerSysTick - 1);

    // Update the current count
    OUTREG32(&pTMURegs->TCNT0, g_oalTimer.countsPerSysTick - 1);

    // Start Timer
    OUTREG8(&pTMURegs->TSTR, INREG8(&pTMURegs->TSTR) | TMU_TSTR_STR0);
    retVal = TRUE;

    OALMSG(OAL_FUNC, (L"-OALTimerInit(rc = %d)\r\n", retVal));

    return retVal;
}
