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
// Portions Copyright (c) Texas Instruments.  All rights reserved.
//
//------------------------------------------------------------------------------
//
//  File:  timer.c
//
//  The 32KHz timer (32768Hz) requires special procedure for update.
//  After writting new value to TVR and setting TRB bit in CR register
//  code should wait for counter harware to clear TRB bit. Then code has
//  to wait until value readed from TCR register is new value minus one.
//  The reason is that counter will ignore any new value set in this period
//  (when TCR value is equal to value set to TVR). This is true even in case
//  of autoload, so in our situation it means we can't update timer when
//  OEMIdle is entered until counter value is smaller than set/autoload one.
//  Another issue is related to interrupt. There is latency half or one clock
//  before interrupt is seen in interrupt controller. However TCR value can
//  already has been set to recharged value. This is reason why code has to
//  check for pending interrupt two times.
//
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#include <oal.h>
#include <omap5912.h>

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
//  Global: g_pOALEMIFRegs
//
//  This global variable contains EMIF module virtual address. It is used
//  in OALCPUIdle to move DRAM to self-refresh mode.
//
OMAP5912_EMIF_REGS *g_pOALEMIFRegs;

//------------------------------------------------------------------------------
//
//  Define: TIMER_PERIOD/TIMER_MARGIN/SET_COMPENSTATION
//
//  This definition are used in code when timer period, timer margin (if count
//  value is higher we can update timer period) or timer set compensation is
//  used. However some code was optimalized for timer period 32 tick and
//  it will require code modification when different period is used.
//
#define TIMER_PERIOD                32
#define TIMER_MARGIN                3

//------------------------------------------------------------------------------

typedef struct {
    UINT32 maxIdleMSec;                     // maximal idle in MSec
    volatile UINT64 curCounts;              // counts at last system tick
    OMAP5912_TIMER32K_REGS *pTimerRegs;      // 32K timer address
    BOOL updatePeriod;                      // change timer period
} OMAP5912_TIMER_STATE;

//------------------------------------------------------------------------------
//
//  Global: g_timer
//
//  This is global instance of timer control block.
//
OMAP5912_TIMER_STATE g_timer;

//------------------------------------------------------------------------------
// External functions

BOOL OALIntrIsIrqPending(UINT32 irq);

//------------------------------------------------------------------------------
//
//  Function: OALTimerInit
//
//  For OMAP5912 32kHz timer is used to implement system timer. The code is
//  optimized for this timer with period 32 timer clocks per system tick. We
//  ignore all parameters.
//
BOOL OALTimerInit(
    UINT32 sysTickMSec, UINT32 countsPerMSec, UINT32 countsMargin
) {
    BOOL rc = FALSE;
    UINT32 sysIntr;


    OALMSG(OAL_TIMER&&OAL_FUNC, (
        L"+OALTimerInit(%d, %d, %d)\r\n", sysTickMSec, countsPerMSec,
        countsMargin
    ));

    // 32K timer has 24 bits
    g_timer.maxIdleMSec = 0x1000000 >> 5;
    g_timer.updatePeriod = FALSE;

    // Set idle conversion constant and counters
    idleconv = 32768;
    curridlehigh = curridlelow = 0;
    g_timer.curCounts = 0;

    // Set global variable to tell interrupt module about timer used
    g_oalTimerIrq = IRQ_TIMER32K;

    // Request SYSINTR for timer IRQ, it is done to reserve it...
    sysIntr = OALIntrRequestSysIntr(1, &g_oalTimerIrq, OAL_INTR_FORCE_STATIC);

    // Enable System Tick interrupt
    if (!OEMInterruptEnable(sysIntr, NULL, 0)) {
        OALMSG(OAL_ERROR, (
            L"ERROR: OALTimerInit: Interrupt enable for system timer failed"
        ));
        goto cleanUp;
    }

    // Get virtual addresses for hardware
    g_timer.pTimerRegs = OALPAtoUA(OMAP5912_TIMER32K_REGS_PA);
    g_pOALEMIFRegs = OALPAtoUA(OMAP5912_EMIF_REGS_PA);

    // Start timer
    OUTREG32(&g_timer.pTimerRegs->TVR, TIMER_PERIOD - 1);
    SETREG32(&g_timer.pTimerRegs->CR, CR_TRB);
    while ((INREG32(&g_timer.pTimerRegs->CR) & CR_TRB) != 0);
    SETREG32(&g_timer.pTimerRegs->CR, CR_TSS|CR_INT_EN|CR_ARL);

    // Done
    rc = TRUE;

cleanUp:
    OALMSG(OAL_TIMER && OAL_FUNC, (L"-OALTimerInit(rc = %d)\r\n", rc));
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
    UINT32 delta;

    // Should we update timer period?
    if (g_timer.updatePeriod) {
        UINT32 set = INREG32(&g_timer.pTimerRegs->TVR);
        UINT32 cnt = INREG32(&g_timer.pTimerRegs->TCR);
        // Wait for timer value change...
        do {
            while (cnt == INREG32(&g_timer.pTimerRegs->TCR));
            cnt = INREG32(&g_timer.pTimerRegs->TCR);
        } while (cnt == set);
        // Set new value
        OUTREG32(&g_timer.pTimerRegs->TVR, TIMER_PERIOD - 1);
        SETREG32(&g_timer.pTimerRegs->CR, CR_TRB);
        // Wait while it is set
        while ((INREG32(&g_timer.pTimerRegs->CR) & CR_TRB) != 0);
        // Wait until one tick (any timer set in this time is ignored)
        while (INREG32(&g_timer.pTimerRegs->TCR) != (TIMER_PERIOD - 2));
        g_timer.curCounts += (set + 1) - cnt + 1;
        // We are done with period update
        g_timer.updatePeriod = FALSE;
    }
    else {
        g_timer.curCounts += TIMER_PERIOD;
    }

    // Update the millisecond and high resolution counters
    CurMSec = (UINT32)((g_timer.curCounts * 1000) >> 15);

    // Reschedule?
    delta = dwReschedTime - CurMSec;
    if ((INT32)delta <= 0) sysIntr = SYSINTR_RESCHED;

    // Update LEDs.
    // (Right shift by 10 instead of expensive division by 1000. This will 
    // cause the LEDs to update every 1.024 seconds instead every 1 second,
    // which is okay as this is just a notification and not a measurement of any sort)
    OEMWriteDebugLED(0x0f, CurMSec >> 10);

    // Return
    return sysIntr;
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
//  Interrupts are disabled when OEMIdle is called and when it returns.
//
//  Note that system timer must be running when CPU/SoC is moved to reduced
//  power mode.
//
void OEMIdle(DWORD idleParam)
{
    UINT32 baseMSec, idleMSec;
    INT32 usedCounts, idleCounts=0;
    ULARGE_INTEGER idle;
    UINT32 cnt, set;
    OMAP5912_TIMER32K_REGS *pTimerRegs = g_timer.pTimerRegs;

    // Return, if we are waiting for restoring timer period
    if (g_timer.updatePeriod) {
        return;
    }

    // Get current system timer counter
    baseMSec = CurMSec;

    // Compute the remaining idle time
    idleMSec = dwReschedTime - baseMSec;

    // Idle time has expired - we need to return
    if ((INT32)idleMSec <= 0) return;

    // Limit the maximum idle time to what is supported.
    if (idleMSec > g_timer.maxIdleMSec) idleMSec = g_timer.maxIdleMSec;

    // Find where we are and if interrupt is pending
    cnt = INREG32(&pTimerRegs->TCR);
    if (OALIntrIsIrqPending(IRQ_TIMER32K) || cnt < TIMER_MARGIN) 
    {
        // Full period was used, leave OEMIdle...
        return;
    }

    
    // Wait for timer value change...
    do {
        while (cnt == INREG32(&pTimerRegs->TCR));
        // We have check for interrupt one more time...
        cnt = INREG32(&pTimerRegs->TCR);
    } while (cnt == TIMER_PERIOD - 1);

    // If there is pending interrupt now use short idle...
    if (OALIntrIsIrqPending(IRQ_TIMER32K)) {
        // Full period was used, leave OEMIdle...
        return;
    } else {

        // Idle time in hi-res ticks
        idleCounts = idleMSec << 5;

        // We already used
        usedCounts = TIMER_PERIOD - cnt + 1;

        // New timer period without timer setup compensation
        if (idleMSec == 1)
        {
            set = TIMER_PERIOD - 1;
        }
        else
        {
            set = idleCounts - usedCounts - 1;
        }

        // Set new value
        OUTREG32(&pTimerRegs->TVR, set);
        SETREG32(&pTimerRegs->CR, CR_TRB);
        // Wait while it is set
        while ((INREG32(&pTimerRegs->CR) & CR_TRB) != 0);
        // Wait until one tick (any timer set in this time is ignored)
        while (INREG32(&pTimerRegs->TCR) != (set - 1));

        // There must be period update
        g_timer.updatePeriod = TRUE;
    }

    // Move SoC/CPU to idle mode - interrupt are disabled
    OALCPUIdle();

    // Get current counter value
    cnt = INREG32(&pTimerRegs->TCR);



    if (OALIntrIsIrqPending(IRQ_TIMER32K) || cnt < TIMER_MARGIN)
    {
        // It's timer interrupt, or no timer interrupt but close enough
        idleCounts = (set + 1) + usedCounts;
    }
    else
    {

        // Wait for timer value change...
        while (cnt == INREG32(&pTimerRegs->TCR));

        // We have recheck pending IRQ
        cnt = INREG32(&pTimerRegs->TCR);
        if (!OALIntrIsIrqPending(IRQ_TIMER32K)) {
            // no timer interrupt, must be some other interrupts, so just add how many counts elapsed
            idleCounts = usedCounts + (set + 1) - cnt + 1;

            // set the timer back to 1 msec
            set = TIMER_PERIOD - 1;
            // Set new value
            OUTREG32(&pTimerRegs->TVR, set);
            SETREG32(&pTimerRegs->CR, CR_TRB);
            // Wait while it is set
            while ((INREG32(&pTimerRegs->CR) & CR_TRB) != 0);
            // Wait until one tick (any timer set in this time is ignored)
            while (INREG32(&pTimerRegs->TCR) != (set - 1));
            g_timer.updatePeriod = FALSE;
        }
        else 
        {
            // timer interrupted
            idleCounts = (set + 1) + usedCounts;
        }

    } 

    // Align between idle and tick in every 32768
    idle.LowPart   = curridlelow;
    idle.HighPart  = curridlehigh;
    idle.QuadPart &= ~0x7fffull;
    idle.QuadPart |= (g_timer.curCounts * 1000)&0x7fffull;

    // Fix system tick counters & idle counter
    g_timer.curCounts += idleCounts;
    CurMSec = (UINT32)((g_timer.curCounts * 1000) >> 15);

    // Update idle counters
    idle.QuadPart += idleCounts * 1000;
    curridlelow  = idle.LowPart;
    curridlehigh = idle.HighPart;

    OEMWriteDebugLED(0x0f, CurMSec >> 10);
    OEMWriteDebugLED(0xf0, curridlelow >> 21);
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
    return CurMSec;
}

//------------------------------------------------------------------------------

