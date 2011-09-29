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
#include <ceddk.h>
#include <nkintr.h>
#include <oal.h>
#include <vrc5477.h>

//------------------------------------------------------------------------------
//
//  global:  g_oalLastSysIntr
//
//  This global variable should be set by interrupt handler to last SYSINTR
//  when interrupt occurs. It is used by fake (busy loop) idle/power off OAL 
//  implementations.
//
volatile UINT32 g_oalLastSysIntr;

//------------------------------------------------------------------------------
//
//  Function:  OEMPowerOff
//
//  Called when the system is to transition to it's lowest power mode (off).
//  On Vrc5477 there isn't way how to power off system, so implementation
//  uses fake power off.
//
void OEMPowerOff(void)
{
    VRC5477_REGS *pVRC5477Regs = OALPAtoUA(VRC5477_REG_PA);
    UINT32 sysIntr, mask0, mask1, mask2, mask3;
    
    RETAILMSG(TRUE, (L"*** Power OFF ***\n"));

    // Save the current interrupt masks
    mask0 = INREG32(&pVRC5477Regs->INTCTRL0);
    mask1 = INREG32(&pVRC5477Regs->INTCTRL1);
    mask2 = INREG32(&pVRC5477Regs->INTCTRL2);
    mask3 = INREG32(&pVRC5477Regs->INTCTRL3);

    // Disable all interrupts
    CLRREG32(&pVRC5477Regs->INTCTRL0, 0x88888888);
    CLRREG32(&pVRC5477Regs->INTCTRL1, 0x88888888);
    CLRREG32(&pVRC5477Regs->INTCTRL2, 0x88888888);
    CLRREG32(&pVRC5477Regs->INTCTRL3, 0x88888888);

    // Let BSP do board specific stuff
    BSPPowerOff();

    // Now enable interrupt if it was enabled as wakeup source    
    for (sysIntr = SYSINTR_FIRMWARE; sysIntr < SYSINTR_MAXIMUM; sysIntr++) {
        // Skip if sysIntr isn't allowed as wake source
        if (!OALPowerWakeSource(sysIntr)) continue;
        // Enable it as interrupt
        OEMInterruptEnable(sysIntr, NULL, 0);
    }

    // Then power off KITL
    OALKitlPowerOff();

    // put CPU into low-powered waiting state
    do {
        // clear last SYSINTR global variable
        g_oalLastSysIntr = SYSINTR_NOP;

        INTERRUPTS_ON();
        // wait here untill interrupt is issued
        do {
            __asm (
                ".set noreorder\n"
                "nop\n"
                ".word 0x420007E0\n"
                "nop\n"
            );
        } while (g_oalLastSysIntr == SYSINTR_NOP);
        INTERRUPTS_OFF();
    } while (!OALPowerWakeSource(g_oalLastSysIntr));

    // save wake source
    g_oalWakeSource = g_oalLastSysIntr;

    // Restore KITL
    OALKitlPowerOn();

    // Then let BSP do board specific stuff
    BSPPowerOn();

    // Restore original interrupt masks
    OUTREG32(&pVRC5477Regs->INTCTRL0, mask0);
    OUTREG32(&pVRC5477Regs->INTCTRL1, mask1);
    OUTREG32(&pVRC5477Regs->INTCTRL2, mask2);
    OUTREG32(&pVRC5477Regs->INTCTRL3, mask3);
    
    RETAILMSG(TRUE, (L"*** Power ON  ***\n"));
}

//------------------------------------------------------------------------------

