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
//  File:  soc_timer.c
//
//  This file contains SoC-specific routines to support the OAL timer.
//
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#include <oal.h>
#pragma warning(pop)

#include "csp.h"
#include "mx233_timrot.h"

//------------------------------------------------------------------------------
// Defines
// Macros used to conver raw ticks to msec and raw msec to ticks for 32768 HZ
// Input clock
#define MSEC_TO_TICK(msec)     (((msec) << 12)/125 + 1)   // msec * 32.768
#define TICK_TO_MSEC(tick)     (((tick) * 1000) >> 15)    // msec / 32.768

//------------------------------------------------------------------------------
// External Variables
PVOID pv_HWregTIMROT;

//-----------------------------------------------------------------------------
//  Global: g_oalTimer
OAL_TIMER_STATE g_oalTimer;

//------------------------------------------------------------------------------
// External Functions

//-----------------------------------------------------------------------------
// Local Variables

//-----------------------------------------------------------------------------
// Local Functions

BOOL InitOSTickTimer(hw_timer_InputSource_t eClock, UINT16 u16Fixedcnt)
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
    BF_WRn(TIMROT_TIMCTRLn, TIMER0, IRQ_EN,   stTimerSetup.bIrqEnable);
    BF_WRn(TIMROT_TIMCTRLn, TIMER0, POLARITY, stTimerSetup.bPolarity);
    BF_WRn(TIMROT_TIMCTRLn, TIMER0, UPDATE,   stTimerSetup.bUpdate);
    BF_WRn(TIMROT_TIMCTRLn, TIMER0, RELOAD,   stTimerSetup.bReload);
    BF_WRn(TIMROT_TIMCTRLn, TIMER0, PRESCALE, stTimerSetup.ePrescale);
    BF_WRn(TIMROT_TIMCTRLn, TIMER0, SELECT,   stTimerSetup.eSelect);

    //
    // Sets the timer FIXED_COUNT value
    //
    if ((stTimerSetup.bReload) && (stTimerSetup.u16FixedCount))
        BF_WRn(TIMROT_TIMCOUNTn, TIMER0, FIXED_COUNT, stTimerSetup.u16FixedCount-1);
    else
        BF_WRn(TIMROT_TIMCOUNTn, TIMER0, FIXED_COUNT, stTimerSetup.u16FixedCount);

    //
    rc = TRUE;

Cleanup:
    return rc;
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
    UINT32 irq, sysIntr;

    OALMSG(OAL_TIMER&&OAL_FUNC, (
               L"+OALTimerInit( %d, %d, %d )\r\n",
               msecPerSysTick, countsPerMSec, countsMargin
               ));

    // Check for specialized support of 32.768 kHz time base
    if (msecPerSysTick != 1 || countsPerMSec != 32)
    {
        OALMSG(OAL_ERROR, (
                   L"ERROR: OALTimerInit: System tick period out of range(countsPerMSec = %d)...\r\n",
                   countsPerMSec));
        goto cleanUp;
    }

    // Initialize timer state global variables
    g_oalTimer.countsPerMSec            = countsPerMSec;
    g_oalTimer.msecPerSysTick           = msecPerSysTick;
    g_oalTimer.actualMSecPerSysTick = msecPerSysTick;
    g_oalTimer.countsMargin                     = countsMargin;

    g_oalTimer.countsPerSysTick       = (countsPerMSec * msecPerSysTick);
    g_oalTimer.actualCountsPerSysTick = (countsPerMSec * msecPerSysTick);
    g_oalTimer.curCounts              = 0;
    g_oalTimer.maxPeriodMSec          = 1000; // Maximum period the timer will interrupt on, in mSec;

    // Initialize kernel-exported values.
    //
    idleconv     = MSEC_TO_TICK(1);
    curridlehigh = 0;
    curridlehigh = 0;

    // Initialize high resolution timer function pointers
    pQueryPerformanceFrequency = OALTimerQueryPerformanceFrequency;
    pQueryPerformanceCounter   = OALTimerQueryPerformanceCounter;

    // Request IRQ to SYSINTR mapping
    irq = IRQ_TIMER0;
    sysIntr = OALIntrRequestSysIntr(1, &irq, OAL_INTR_FORCE_STATIC);

    // Make sure we have a valid sysIntr
    if (sysIntr == SYSINTR_UNDEFINED)
    {
        OALMSG(OAL_ERROR, (L"+OALTimerInit:  SYSINTR_UNDEFINED\r\n"));
        goto cleanUp;
    }

    // Get uncached virtual addresses for system timer
    pv_HWregTIMROT = (PVOID) OALPAtoUA(CSP_BASE_REG_PA_TIMROT);
    if (pv_HWregTIMROT == NULL)
    {
        OALMSG(OAL_ERROR, (L"+OALTimerInit:  pv_HWregTIMROT null pointer!\r\n"));
        goto cleanUp;
    }

    // MX233: Timer 0
    if(!InitOSTickTimer(TIMER_SELECT_32KHZ_XTAL,(UINT16)countsPerMSec) )
    {
        OALMSG(OAL_ERROR, (L"+OALTimerInit:  InitOSTickTimer Failed\r\n"));
        goto cleanUp;
    }

    // Enable system tick interrupt
    if (!OEMInterruptEnable(sysIntr, NULL, 0)) {
        OALMSG(OAL_ERROR, (
                   L"ERROR: OALTimerInit: Interrupt enable for system timer failed"
                   ));
        goto cleanUp;
    }

    // clear Timer Interrupt
    BF_CLRn(TIMROT_TIMCTRLn, TIMER0, IRQ);

    // enable TIMER0 interrupt
    BF_WRn(TIMROT_TIMCTRLn, TIMER0, IRQ_EN, TIMER_INTERRUPT_ENABLE);

    // Done
    rc = TRUE;

cleanUp:
    OALMSG(OAL_TIMER && OAL_FUNC, (L"-OALTimerInit(rc = %d)\r\n", rc));
    return rc;
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
    hw_timrot_timcountn_t dwTimerCountReg;

    dwTimerCountReg.U = HW_TIMROT_TIMCOUNTn_RD(0);
    return (dwTimerCountReg.B.FIXED_COUNT - dwTimerCountReg.B.RUNNING_COUNT);
}
//------------------------------------------------------------------------------
//
//  Function: OALTimerIntrHandler
//
//  This function implement timer interrupt handler. It is called from common
//  ARM interrupt handler.
//
UINT32 OALTimerIntrHandler()
{
    UINT32 sysIntr          = SYSINTR_NOP;

    // clear Timer Interrupt
    BF_CLRn(TIMROT_TIMCTRLn, 0, IRQ);

    // Update the millisecond and high resolution counters
    CurMSec += g_oalTimer.msecPerSysTick;
    g_oalTimer.curCounts += g_oalTimer.countsPerSysTick;

    //Re-schedule?
    if ((INT32)(CurMSec - dwReschedTime) >= 0)  {
        sysIntr = SYSINTR_RESCHED;
    }

    return (sysIntr);
}

//------------------------------------------------------------------------------
//
//  Function:     OEMIdle
//
//  This function is called by the kernel when there are no threads ready to
//  run. The CPU should be put into a reduced power mode if possible and halted.
//  It is important to be able to resume execution quickly upon receiving an
//  interrupt.
//
//  It is assumed that interrupts are off when OEMIdle is called. Interrrupts
//  are also turned off when OEMIdle returns.
//
VOID OEMIdle(DWORD idleParam)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(idleParam);
}
