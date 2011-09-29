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
// NOTE: stubs are being used - this isn't done
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2006-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  off.c
//
//  This file provides the capabilities to suspend the system and controlling
//  wake sources.
//
//------------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <ceddk.h>
#include <nkintr.h>
#include <cmnintrin.h>
#include <oal.h>
#pragma warning(pop)
#include "csp.h"


//------------------------------------------------------------------------------
// External Functions


//------------------------------------------------------------------------------
// External Variables
extern PCSP_AVIC_REGS g_pAVIC;
extern PCSP_GPIO_REGS g_pGPIO1;
extern PCSP_GPIO_REGS g_pGPIO2;
extern PCSP_GPIO_REGS g_pGPIO3;
extern PCSP_SDMA_REGS g_pSDMA;
extern PCSP_CCM_REGS g_pCCM;
extern PCSP_EPIT_REG g_pEPIT;


//------------------------------------------------------------------------------
// Defines


//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Global Variables


//------------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
// Local Functions


//-----------------------------------------------------------------------------
//
// Function: OEMPowerOff
//
// Called when the system is to transition to it's lowest power mode.  This
// function stores off some of the important registers (not really needed for
// DSM since registers will not lose state in DSM).
//
// Parameters:
//      None
//
// Returns:
//      None
//
//-----------------------------------------------------------------------------
void OEMPowerOff()
{
    UINT32 regSave[5];
    UINT32 sysIntr, irq;
    UINT32 line;
    BOOL PowerState;

    // Reset the wake source global
    g_oalWakeSource = SYSWAKE_UNKNOWN;

    // Call board-level power off function
    BSPPowerOff();
    
    // Save state of enabled interrupts
    regSave[0] = INREG32(&g_pAVIC->INTENABLEH);
    regSave[1] = INREG32(&g_pAVIC->INTENABLEL);
    regSave[2] = INREG32(&g_pGPIO1->IMR);
    regSave[3] = INREG32(&g_pGPIO2->IMR);
    regSave[4] = INREG32(&g_pGPIO3->IMR);

    // Mask all interrupts
    OUTREG32(&g_pAVIC->INTENABLEH, 0);
    OUTREG32(&g_pAVIC->INTENABLEL, 0);
    OUTREG32(&g_pGPIO1->IMR, 0);
    OUTREG32(&g_pGPIO2->IMR, 0);
    OUTREG32(&g_pGPIO3->IMR, 0);

    // Now enable interrupt if it was enabled as wakeup source
    for (sysIntr = SYSINTR_DEVICES; sysIntr < SYSINTR_MAXIMUM; sysIntr++)
    {
        // Skip if sysIntr isn't allowed as wake source
        if (!OALPowerWakeSource(sysIntr)) continue;
        
        // Enable it as wakeup source interrupt
        OEMInterruptEnable(sysIntr, NULL, 0);
    }

    // Switch off power for KITL device
    PowerState = FALSE;
    KITLIoctl(IOCTL_KITL_POWER_CALL, &PowerState, sizeof(PowerState), NULL, 0, NULL);

    // Place the system in suspend state and wait for an interrupt.
    OALMSG(OAL_POWER, 
        (_T("INFO: OEMPowerOff entering suspend. ") 
         _T("INTENABLEH = 0x%x, INTENABLEL = 0x%x\r\n"), 
         INREG32(&g_pAVIC->INTENABLEH), INREG32(&g_pAVIC->INTENABLEL)));

    // Disable the OS tick timer (EPIT1) while we are powered off.  This is 
    // necessary since the EPIT can be clocked from CKIL and will continue
    // to run in some low-power modes.
    INSREG32BF(&g_pEPIT->CR, EPIT_CR_EN, EPIT_CR_EN_DISABLE);

    // Power off ...
    OALCPUPowerOff();

    // Firstly enable the OS tick timer (EPIT1)
    INSREG32BF(&g_pEPIT->CR, EPIT_CR_EN, EPIT_CR_EN_ENABLE);

    // Get wakeup interrupt source
    irq = EXTREG32BF(&g_pAVIC->NIVECSR, AVIC_NIVECSR_NIVECTOR);

    OALMSG(OAL_POWER, 
        (_T("INFO: OEMPowerOff leaving suspend.  NIVECSR = 0x%x\r\n"), irq));
        
    // If valid wake interrupt is pending
    if (irq < AVIC_IRQ_SOURCES_MAX)
    {
        // GPIO1 special case
        if (irq == IRQ_GPIO1)
        {
            // Detect GPIO line that is asserting interrupt
            line = _CountLeadingZeros(INREG32(&g_pGPIO1->ISR) 
                                      & INREG32(&g_pGPIO1->IMR));

            // If at least one GPIO interrupt line is asserted
            if (line < 32)
            { 
                irq = IRQ_GPIO1_PIN0 + (31 - line);
            }
        }
        
        // GPIO2 special case
        else if (irq == IRQ_GPIO2)
        {
            // Detect GPIO line that is asserting interrupt
            line = _CountLeadingZeros(INREG32(&g_pGPIO2->ISR) 
                                      & INREG32(&g_pGPIO2->IMR));

            // If at least one GPIO interrupt line is asserted
            if (line < 32)
            { 
                irq = IRQ_GPIO2_PIN0 + (31 - line);
            }
        }

        // GPIO3 special case
        else if (irq == IRQ_GPIO3)
        {
            // detect GPIO line that is asserting interrupt
            line = _CountLeadingZeros(INREG32(&g_pGPIO3->ISR) 
                                      & INREG32(&g_pGPIO3->IMR));

            // If at least one GPIO interrupt line is asserted
            if (line < 32)
            { 
                irq = IRQ_GPIO3_PIN0 + (31 - line);
            }
        }

        // SDMA special case
        else if (irq == IRQ_SDMA)
        {
            // Detect SDMA channel that is asserting interrupt
            line = _CountLeadingZeros(INREG32(&g_pSDMA->INTR));

            // If at least one SDMA channel interrupt is asserted
            if (line < 32)
            {
                // Translate it to the secondary IRQ
                irq = IRQ_SDMA_CH0 + (31 - line);
            }
        }

        // Chance of board-level subordinate interrupt controller
        irq = BSPIntrActiveIrq(irq);

        // Map the irq to a SYSINTR
        g_oalWakeSource = OALIntrTranslateIrq(irq);
    }

    // Restore state of enabled interrupts
    OUTREG32(&g_pAVIC->INTENABLEH, regSave[0]);
    OUTREG32(&g_pAVIC->INTENABLEL, regSave[1]);
    OUTREG32(&g_pGPIO1->IMR, regSave[2]);
    OUTREG32(&g_pGPIO2->IMR, regSave[3]);
    OUTREG32(&g_pGPIO3->IMR, regSave[4]);

    // Switch on power for KITL device
    PowerState = TRUE;
    KITLIoctl (IOCTL_KITL_POWER_CALL, &PowerState, sizeof(PowerState), NULL, 0, NULL);

    // Do platform dependent power on actions
    BSPPowerOn();
}


//-----------------------------------------------------------------------------
//
//  Function: OALClockSetGatingMode
//
//  This function provides the OAL a safe mechanism for setting the clock 
//  gating mode of peripherals.
//
//  Parameters:
//      index
//           [in] Index for referencing the peripheral clock gating control 
//           bits.
//
//      mode
//           [in] Requested clock gating mode for the peripheral.
//
//  Returns:
//      None
//
//-----------------------------------------------------------------------------
VOID OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX index, DDK_CLOCK_GATE_MODE mode)
{
    BOOL fEnable;
    
    // CGR is a shared register so we must disable interrupts temporarily
    // for safe access
    fEnable = INTERRUPTS_ENABLE(FALSE);
    
    // Update the clock gating mode
    INSREG32(&g_pCCM->CGR[CCM_CGR_INDEX(index)], CCM_CGR_MASK(index), 
        CCM_CGR_VAL(index, mode));

    INTERRUPTS_ENABLE(fEnable);
}
