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


//------------------------------------------------------------------------------
//
//  Module: timer.c
//
//  Interface to OAL timer services.
//
//------------------------------------------------------------------------------
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#include <oal.h>
#include "csp.h"

//------------------------------------------------------------------------------
// External Functions

extern UINT32 OALTimerGetClkSrc(void);
extern UINT32 OALTimerGetClkPrescalar(void);
extern VOID OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX index, 
    DDK_CLOCK_GATE_MODE mode);



//------------------------------------------------------------------------------
// Defines

//------------------------------------------------------------------------------
// Local Variables

static CSP_GPT_REGS *g_pGPT;


//------------------------------------------------------------------------------
// Local Functions

//-----------------------------------------------------------------------------
//
//  Function: OALTimerInit
//
//  This function is typically called from the OEMInit to initialize
//  Windows CE system timer. 
//  On most platform timer period will be 1 ms, but it can be useful to use 
//  higher value for some specific (low-power) devices.
//  The BONO(MX27) implementation uses the GPT1 for a system timer.
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
    if (msecPerSysTick < 1 || msecPerSysTick > 1000 ||
        countsPerSysTick < 1 || countsPerSysTick > GPT_TCN_COUNT_MAX)
    {
        OALMSG(OAL_ERROR, (TEXT("ERROR: OALTimerInit: System tick period out of range. (%d max %d)"),
            msecPerSysTick, 1000 ));
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
    g_oalTimer.maxPeriodMSec = GPT_TCN_COUNT_MAX / countsPerMSec;

    // Set kernel exported globals to initial values
    idleconv = countsPerMSec;
    curridlehigh = 0;
    curridlelow = 0;
    
    // Initialize high resolution timer function pointers
    pQueryPerformanceFrequency = OALTimerQueryPerformanceFrequency;
    pQueryPerformanceCounter = OALTimerQueryPerformanceCounter;

    // Request IRQ to SYSINTR mapping
    irq = IRQ_GPT1;
    sysIntr = OALIntrRequestSysIntr(1, &irq, OAL_INTR_FORCE_STATIC);

    // Make sure we have a valid sysIntr
    if (sysIntr == SYSINTR_UNDEFINED)
    {
        OALMSG(OAL_ERROR, (L"+OALTimerInit:  SYSINTR_UNDEFINED\r\n"));
        goto cleanUp;
    }

	// Enable the IPG Clock input to the GPT1 module
	OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_GPT1, DDK_CLOCK_GATE_MODE_ENABLE);

    // Get uncached virtual addresses for system timer
    g_pGPT = (CSP_GPT_REGS *) OALPAtoUA(CSP_BASE_REG_PA_GPT1);
    if (g_pGPT == NULL)
    {
        OALMSG(OAL_ERROR, (L"+OALTimerInit:  GPT1 null pointer!\r\n"));
        goto cleanUp;
    }

    // Disable GPT
    OUTREG32(&g_pGPT->TCTL, 0);	

    // Software reset GPT1
    OUTREG32(&g_pGPT->TCTL, CSP_BITFMASK(GPT_TCTL_SWR));

    // Wait for the software reset to complete
    while (INREG32(&g_pGPT->TCTL) & CSP_BITFMASK(GPT_TCTL_SWR));

    g_pGPT->TPRER = OALTimerGetClkPrescalar();

    // Initialize counter/compare registers
    OALTimerInitCount(g_oalTimer.countsPerSysTick);
    
    // Enable timer for "freerun" mode:
    //  COMPEN = 1 (Enable compare interrupt)
    //  FRR = 1 (Free running counter)
    //  CC = 0  (counter clear on TEN = 0)
    OUTREG32(&g_pGPT->TCTL,
        CSP_BITFVAL(GPT_TCTL_TEN, GPT_TCTL_TEN_ENABLE) |
        CSP_BITFVAL(GPT_TCTL_CLKSOURCE, OALTimerGetClkSrc()) |
        CSP_BITFVAL(GPT_TCTL_COMPEN, GPT_TCTL_COMPEN_ENABLE) |
        CSP_BITFVAL(GPT_TCTL_CAPTEN, GPT_TCTL_CAPTEN_DISABLE) |
        CSP_BITFVAL(GPT_TCTL_CAP, GPT_TCTL_CAP_DISABLE) |
        CSP_BITFVAL(GPT_TCTL_FRR, GPT_TCTL_FRR_FREERUN) |
        CSP_BITFVAL(GPT_TCTL_OM, GPT_TCTL_OM_ACTIVELOW) |
        CSP_BITFVAL(GPT_TCTL_CC, GPT_TCTL_CC_ENABLE));
    
    // Enable System Tick interrupt
    if(!OEMInterruptEnable(sysIntr, NULL, 0))
    {
        OALMSG(OAL_ERROR, (TEXT("ERROR: OALTimerInit: Interrupt enable for system timer failed")));
        goto cleanUp;

    }

   OALMSG(OAL_TIMER&&OAL_VERBOSE, (TEXT("TCMP(0x%x) TPRER(0x%x) TCTL(0x%x) TCN(0x%x)\r\n"), 
                                    g_pGPT->TCMP, 
                                    g_pGPT->TPRER, 
                                    g_pGPT->TCTL,
                                    g_pGPT->TCN));

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

    // Update the millisecond and high resolution counters
    g_oalTimer.curCounts += g_oalTimer.actualCountsPerSysTick;
    CurMSec += g_oalTimer.actualMSecPerSysTick;

    // Advance to next system tick	
    OALTimerRecharge(g_oalTimer.countsPerSysTick, g_oalTimer.countsMargin);

    // Reschedule?
    if ((INT32)(CurMSec - dwReschedTime) >= 0) sysIntr = SYSINTR_RESCHED;

	// Take care of ILTiming
	if (g_oalILT.active) {
		g_oalILT.isrTime1 = OALTimerCountsSinceSysTick();
		g_oalILT.savedPC = 0;
		g_oalILT.interrupts++;

        if (--g_oalILT.counter == 0) {
            sysIntr = SYSINTR_TIMING;
            g_oalILT.counter = g_oalILT.counterSet;
            g_oalILT.isrTime2 = OALTimerCountsSinceSysTick();
			g_oalILT.interrupts = 0;
        }
    }        

    return sysIntr;
}

//-----------------------------------------------------------------------------
//
//  Function: OALTimerGetCount
//
//  This function returns the actual timer value.
//
//  Parameters:
//      None.
//
//  Returns:
//      actual timer counter value.
//
//-----------------------------------------------------------------------------
UINT32 OALTimerGetCount()
{
    return g_pGPT->TCN;   
}    

//-----------------------------------------------------------------------------
//
//  Function: OALTimerGetCompare
//
//  This function returns the the timer compare value.
//
//  Parameters:
//      None.
//
//  Returns:
//      actual timer compare value.
//
//-----------------------------------------------------------------------------
UINT32 OALTimerGetCompare()
{
    return g_pGPT->TCMP;  
}    

//-----------------------------------------------------------------------------
//
//  Function: OALTimerSetCompare
//
//  This function set the timer compare value.
//
//  Parameters:
//      compare[in] compare value to set
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID OALTimerSetCompare(UINT32 compare)
{
    g_pGPT->TCMP = compare;
    g_pGPT->TSTAT |= CSP_BITFMASK(GPT_TSTAT_COMP);
}    
//-----------------------------------------------------------------------------

