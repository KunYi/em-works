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
//  File:  power.c
//
#include <windows.h>
#include <nkintr.h>
#include <oal.h>
#include <omap5912.h>

//------------------------------------------------------------------------------
//  External functions

BOOL OALIntrIsIrqPending(UINT32 irq);

BOOL OALIntrTranslateSysIntr(
    UINT32 sysIntr, UINT32 *pCount, const UINT32 **ppIrqs
);


//------------------------------------------------------------------------------
//
//  Function:  OEMInterruptPending
//
//  This function returns true when given sysIntr interrupt is pending.
//
BOOL OEMInterruptPending(DWORD sysIntr)
{
    BOOL pending = FALSE;
    const UINT32 *pIrqs;
    UINT32 ix, count;

    OALMSG(OAL_INTR&&OAL_VERBOSE, (
        L"+OEMInterruptPending(%d)\r\n", sysIntr
    ));

    if (OALIntrTranslateSysIntr(sysIntr, &count, &pIrqs)) {
        for (ix = 0; ix < count; ix++ && !pending) {
            pending = OALIntrIsIrqPending(pIrqs[ix]);
        }            
    }

    OALMSG(OAL_INTR&&OAL_VERBOSE, (
        L"-OEMInterruptPending(rc = %d)\r\n", pending
    ));
    return pending;
}

//------------------------------------------------------------------------------
//
//  Function:  OEMPowerOff
//
//  Called when the system is to transition to it's lowest power mode (off).
//
void OEMPowerOff(void)
{
    OMAP5912_INTC_REGS *pIntcL1Regs = OALPAtoUA(OMAP5912_INTC_L1_REGS_PA);
    OMAP5912_INTC_REGS *pIntcL2ARegs = OALPAtoUA(OMAP5912_INTC_L2A_REGS_PA);
    OMAP5912_INTC_REGS *pIntcL2BRegs = OALPAtoUA(OMAP5912_INTC_L2B_REGS_PA);
    OMAP5912_INTC_REGS *pIntcL2CRegs = OALPAtoUA(OMAP5912_INTC_L2C_REGS_PA);
    OMAP5912_INTC_REGS *pIntcL2DRegs = OALPAtoUA(OMAP5912_INTC_L2D_REGS_PA);
    OMAP5912_GPIO_REGS *pGPIO1Regs = OALPAtoUA(OMAP5912_GPIO1_REGS_PA);
    OMAP5912_GPIO_REGS *pGPIO2Regs = OALPAtoUA(OMAP5912_GPIO2_REGS_PA);
    OMAP5912_GPIO_REGS *pGPIO3Regs = OALPAtoUA(OMAP5912_GPIO3_REGS_PA);
    OMAP5912_GPIO_REGS *pGPIO4Regs = OALPAtoUA(OMAP5912_GPIO4_REGS_PA);
    OMAP5912_ULPD_REGS *pULPDRegs = OALPAtoUA(OMAP5912_ULPD_REGS_PA);
    OMAP5912_RTC_REGS *pRTCRegs = OALPAtoUA(OMAP5912_RTC_REGS_PA);
    UINT32 intcL1, intcL2A, intcL2B, intcL2C, intcL2D;
    UINT32 gpio1, gpio2, gpio3, gpio4;
    UINT32 sysIntr;
    UINT8 rtcIntrReg;

    // Give chance to do board specific stuff
    BSPPowerOff();

    OALStall(1000000);

    // Save existing interrupt masks and peripheral states
    intcL1 = INREG32(&pIntcL1Regs->MIR);
    intcL2A = INREG32(&pIntcL2ARegs->MIR); 
    intcL2B = INREG32(&pIntcL2BRegs->MIR); 
    intcL2C = INREG32(&pIntcL2CRegs->MIR); 
    intcL2D = INREG32(&pIntcL2DRegs->MIR); 
    gpio1 = INREG32(&pGPIO1Regs->IRQENABLE1);
    gpio2 = INREG32(&pGPIO2Regs->IRQENABLE1);
    gpio3 = INREG32(&pGPIO3Regs->IRQENABLE1);
    gpio4 = INREG32(&pGPIO4Regs->IRQENABLE1);
    rtcIntrReg = INREG8(&pRTCRegs->INTR);

    // Disable most interrupts including RTC (left IRQ_L2FIQ/IRQ_L2IRQ enabled) 
    OUTREG32(&pIntcL1Regs->MIR, 0xFFFFFFFC);
    OUTREG32(&pIntcL2ARegs->MIR, 0xFFFFFFFF);
    OUTREG32(&pIntcL2BRegs->MIR, 0xFFFFFFFF);
    OUTREG32(&pIntcL2CRegs->MIR, 0xFFFFFFFF);
    OUTREG32(&pIntcL2DRegs->MIR, 0xFFFFFFFF);
    OUTREG32(&pGPIO1Regs->IRQENABLE1, 0);
    OUTREG32(&pGPIO2Regs->IRQENABLE1, 0);
    OUTREG32(&pGPIO3Regs->IRQENABLE1, 0);
    OUTREG32(&pGPIO4Regs->IRQENABLE1, 0);
    OUTREG8(&pRTCRegs->INTR, rtcIntrReg & ~RTC_INTR_ALARM);

    OALStall(1000000);

    // Enable wake sources interrupts
    for (sysIntr = SYSINTR_FIRMWARE; sysIntr < SYSINTR_MAXIMUM; sysIntr++) {
        // Skip if sysIntr isn't allowed as wake source
        if (!OALPowerWakeSource(sysIntr)) continue;
        // Enable it as interrupt
        OEMInterruptEnable(sysIntr, NULL, 0);

        if( (SYSINTR_RTC_ALARM == sysIntr) && (rtcIntrReg & RTC_INTR_ALARM) )
        {
            // In the case where we want the RTC to act as a wakeup source we will restore the RTC device's original interrupt
            // control state.  We do not force the RTC to be generating interrupts here because we assume the RTC module has already
            // been properly setup.

            OUTREG8(&pRTCRegs->INTR, rtcIntrReg);
        }
    }

    // Enable deep sleep
    SETREG16(&pULPDRegs->POWER_CTRL, POWER_CTRL_DEEP_SLEEP_EN);

    OALStall(1000000);

    // Move SoC/CPU to idle mode, there is no special power-off
    // mode on OMAP5912 (but deep sleep is very similar to it)
    OALCPUIdle();

    // Find wakeup source
    for (sysIntr = SYSINTR_FIRMWARE; sysIntr < SYSINTR_MAXIMUM; sysIntr++) {
        // Skip if sysIntr isn't allowed as wake source
        if (!OALPowerWakeSource(sysIntr)) continue;
        // When this sysIntr is pending we find wake source
        if (OEMInterruptPending(sysIntr)) {
            g_oalWakeSource = sysIntr;
            break;
        }
    }

    // Do board specific stuff    
    BSPPowerOn();

    // Restore interrupt masks and peripheral states
    OUTREG8(&pRTCRegs->INTR, rtcIntrReg);
    OUTREG32(&pIntcL1Regs->MIR, intcL1);
    OUTREG32(&pIntcL2ARegs->MIR, intcL2A);
    OUTREG32(&pIntcL2BRegs->MIR, intcL2B);
    OUTREG32(&pIntcL2CRegs->MIR, intcL2C);
    OUTREG32(&pIntcL2DRegs->MIR, intcL2D);
    OUTREG32(&pGPIO1Regs->IRQENABLE1, gpio1);
    OUTREG32(&pGPIO2Regs->IRQENABLE1, gpio2);
    OUTREG32(&pGPIO3Regs->IRQENABLE1, gpio3);
    OUTREG32(&pGPIO4Regs->IRQENABLE1, gpio4);
}

//------------------------------------------------------------------------------

