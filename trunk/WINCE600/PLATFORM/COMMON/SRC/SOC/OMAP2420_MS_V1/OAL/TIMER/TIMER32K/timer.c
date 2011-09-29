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
//  File:  timer.c
//
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#include <oal.h>
#include <omap2420.h>
#include <omap2420_led.h>

//------------------------------------------------------------------------------
//
//  External: g_oalTimerIrq
//
//  This variable is defined in interrupt module and it is used in interrupt
//  handler to distinguish system timer interrupt.
//
extern UINT32 g_oalTimerIrq;   

//------------------------------------------------------------------------------
//
//  Global: g_oalTimer    
//
//  This is global instance of timer control block.
//
OAL_TIMER_STATE g_oalTimer;

//------------------------------------------------------------------------------
//
//  Local: g_pTimerRegs;
//
static OMAP2420_GPTIMER_REGS *g_pTimerRegs;
static OMAP2420_PRCM_REGS *g_pPRCMRegs;

//------------------------------------------------------------------------------
//
//  Function: OALTimerInit
//
//  This function is typically called from the OEMInit to initialize Windows CE
//  system timer. The tickMSec parameter determine timer period in milliseconds.
//  On most platform timer period will be 1 ms, but it can be usefull to use
//  higher value for some specific devices.
//
//  For OMAP2420 32kHz timer is used to implement system timer. Because of
//  fixed timer frequency countsPerMSec and countsMargin parameters are
//  ignored.
//
BOOL OALTimerInit(
    UINT32 msecPerSysTick, UINT32 countsPerMSec, UINT32 countsMargin
) {
    BOOL rc = FALSE;
    UINT32 sysIntr;

    OALMSG(OAL_TIMER&&OAL_FUNC, (
        L"+OALTimerInit(%d, %d, %d)\r\n", msecPerSysTick, countsPerMSec,
        countsMargin
    ));

    // Initialize high resolution timer function pointers
    pQueryPerformanceFrequency = OALTimerQueryPerformanceFrequency;
    pQueryPerformanceCounter   = OALTimerQueryPerformanceCounter;

    // Ignore two last passed arguments 
    countsPerMSec = OMAP2420_GPTIMER1_COUNTS_PER_1MS;
    countsMargin = 2;
    
    // 32 bit counter
    g_oalTimer.maxPeriodMSec = 0xFFFFFFFF/countsPerMSec;

    // System tick period must be smaller than counter maximal period.
    // This reduction will be usual when variable tick is used.
    if (msecPerSysTick > g_oalTimer.maxPeriodMSec) {
        msecPerSysTick = g_oalTimer.maxPeriodMSec;
    }

    // Initialize timer state structure
    g_oalTimer.countsPerMSec = countsPerMSec;
    g_oalTimer.countsMargin = countsMargin;
    g_oalTimer.msecPerSysTick = msecPerSysTick;
    g_oalTimer.countsPerSysTick = g_oalTimer.countsPerMSec * msecPerSysTick;
    g_oalTimer.actualMSecPerSysTick = g_oalTimer.msecPerSysTick;
    g_oalTimer.actualCountsPerSysTick = g_oalTimer.countsPerSysTick;

    // Set kernel exported globals to initial values
    idleconv = 2;
    curridlelow = 0;
    curridlehigh = 0;

    // Set global variable to tell interrupt module about timer used
    g_oalTimerIrq = IRQ_GPT1; 

    // Request SYSINTR for timer IRQ, it is done to reserve it...
    sysIntr = OALIntrRequestSysIntr(1, &g_oalTimerIrq, OAL_INTR_FORCE_STATIC);
    
    // Hardware Setup
    g_pPRCMRegs  = OALPAtoUA(OMAP2420_PRCM_REGS_PA);
    g_pTimerRegs = OALPAtoUA(OMAP2420_GPTIMER1_REGS_PA);
    
    OUTREG32(&g_pTimerRegs->TCLR, 0x00000000);
    CurMSec = 0;

    // Select non posted mode
    CLRREG32(&g_pTimerRegs->TSICR, 0x4);
    
    // Enable global wakeup feature and smart idle 
    //  Set clock activity - FCLK can be  switched off, L4 interface clock is maintained during wkup.
    OUTREG32(&g_pTimerRegs->TIOCP_CFG, 0x00000214);
    
    // Enable overflow wakeup
    OUTREG32(&g_pTimerRegs->TWER, 0x00000002);
    
    //Clear Interrupt
    SETREG32(&g_pTimerRegs->TISR, 0x2);

    // Enabled overflow interrupt
    SETREG32(&g_pTimerRegs->TIER, 0x2);
    
    //OALMSG(TRUE, (L"Timer Load Value is %x\r\n",g_oalTimer.countsPerSysTick));
    //  Set the load register value.
    OUTREG32(&g_pTimerRegs->TLDR, (0xFFFFFFFF - g_oalTimer.countsPerSysTick+1));
    
    //  Trigger a counter reload by writing    
    OUTREG32(&g_pTimerRegs->TTGR, 0xFFFFFFFF);
   
    //  Start the timer.  Also set for auto reload
    SETREG32(&g_pTimerRegs->TCLR, 0x00000003);

    // Setup Timer clock
    CLRREG32(&g_pPRCMRegs->ulCM_CLKSEL_WKUP, 0x3);
    
    // Enable the functional and interface  clocks for the GPT 1. 
    SETREG32(&g_pPRCMRegs->ulCM_ICLKEN_WKUP, 0x1);
    SETREG32(&g_pPRCMRegs->ulCM_FCLKEN_WKUP, 0x1);

    // Enable System Tick interrupt
    if (!OEMInterruptEnable(sysIntr, NULL, 0)) {
        OALMSG(OAL_ERROR, (
            L"ERROR: OALTimerInit: Interrupt enable for system timer failed"
        ));
        goto cleanUp;
    }

    rc = TRUE;
    
cleanUp:
    OALMSG(OAL_TIMER && OAL_FUNC, (L"-OALTimerInit..(rc = %d)\r\n", rc));
    return rc;
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
    UINT32 sysIntr = SYSINTR_NOP;

#ifdef OAL_ILTIMING
    if (g_oalILT.active) {
            g_oalILT.isrTime1 = OALTimerCountsSinceSysTick();
    }        
#endif

    //Acknowledge the current timer interrupt and 
    //clear the status for next timer interrupt
    SETREG32(&g_pTimerRegs->TISR,0x2);

    // Update the millisecond and high resolution counters
    g_oalTimer.curCounts += g_oalTimer.actualCountsPerSysTick;
    CurMSec = (UINT32)((g_oalTimer.curCounts*1000)>>15);

    // Update LED display (bits 0&1)
    OALLED(LED_IDX_TIMERSPIN, CurMSec >> 10);
      
    // Reschedule?
    if ((int)(CurMSec - dwReschedTime) >= 0) sysIntr = SYSINTR_RESCHED;

#ifdef OAL_ILTIMING
    if (g_oalILT.active) {
        if (--g_oalILT.counter == 0) {
            sysIntr = SYSINTR_TIMING;
            g_oalILT.counter = g_oalILT.counterSet;
            g_oalILT.isrTime2 = OALTimerCountsSinceSysTick();
        }
    }
#endif
    // Done    
    return sysIntr;
}

//------------------------------------------------------------------------------
//
//  Function: OALTimerCountsSinceSysTick
//
//  This function return count of hi res ticks since system tick.
//
INT32 OALTimerCountsSinceSysTick()
{
    // Calc value.
    return (INREG32(&g_pTimerRegs->TCRR) - INREG32(&g_pTimerRegs->TLDR));
}

//------------------------------------------------------------------------------
//
//  Function:  OEMIdle
//
//  This is 1ms fixed tick implementation. No need to implement the OEMIdle function.
//
void OEMIdle(DWORD idleParam)
{
}

//------------------------------------------------------------------------------
//
//  Function: OEMGetTickCount
//
//  This returns the number of milliseconds that have elapsed since Windows 
//  CE was started. If the system timer period is 1ms the function simply 
//  returns the value of CurMSec. If the system timer period is greater then
//  1 ms, the HiRes offset is added to the value of CurMSec.
//
UINT32 OEMGetTickCount()
{
    UINT32 count;
    INT32 offset;

    if (g_oalTimer.actualMSecPerSysTick == 1) {
        // Return CurMSec if the system tick is 1 ms.
        count = CurMSec;
    }  else {
        // System timer tick period exceeds 1 ms. 
        //
        // This code adjusts the accuracy of the returned value to the nearest
        // MSec when the system tick exceeds 1 ms. The following code checks if 
        // a system timer interrupt occurred between reading the CurMSec value 
        // and the call to fetch the HiResTicksSinceSysTick. If so, the value of
        // CurMSec and Offset is re-read, with the certainty that a system timer
        // interrupt will not occur again.
        do {
            count = CurMSec;
            offset = OALTimerCountsSinceSysTick();
        } 
        while (count != CurMSec);

        // Adjust the MSec value with the contribution from HiRes counter.
        count += offset/g_oalTimer.countsPerMSec;
    }

    return count;
}



//------------------------------------------------------------------------------
//
//  Function: OALQueryPerformanceFrequency
//
//  This function returns the frequency of the high-resolution 
//  performance counter.
//
BOOL OALTimerQueryPerformanceFrequency(LARGE_INTEGER *pFrequency)
{
    if (!pFrequency) return FALSE;

    pFrequency->HighPart = 0;
    pFrequency->LowPart = 32768; // 32k timer frequency
    return TRUE;
}


//------------------------------------------------------------------------------
//
//  Function: OALQueryPerformanceCounter
//
//  This function returns the current value of the high-resolution 
//  performance counter.
//  
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

//------------------------------------------------------------------------------

