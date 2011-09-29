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
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
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
extern PCSP_TZIC_REGS g_pTZIC;
extern PCSP_GPIO_REGS g_pGPIO1;
extern PCSP_GPIO_REGS g_pGPIO2;
extern PCSP_GPIO_REGS g_pGPIO3;
extern PCSP_GPIO_REGS g_pGPIO4;
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
    UINT32 registerStore[TZIC_IRQ_SET_MAX+DDK_GPIO_PORT4+1];
    UINT32 sysIntr, irq = TZIC_IRQ_SOURCES_MAX;
    UINT32 line;
    UINT32 irqSet;
    BOOL PowerState = TRUE;

    // Reset the wake source global
    g_oalWakeSource = SYSWAKE_UNKNOWN;

    // Call board-level power off function
    BSPPowerOff();

    // Save state of enabled interrupts
    registerStore[0] = INREG32(&g_pGPIO1->IMR);
    registerStore[1] = INREG32(&g_pGPIO2->IMR);
    registerStore[2] = INREG32(&g_pGPIO3->IMR);
    registerStore[3] = INREG32(&g_pGPIO4->IMR);
    registerStore[4] = INREG32(&g_pTZIC->ENSET[0]);
    registerStore[5] = INREG32(&g_pTZIC->ENSET[1]);
    registerStore[6] = INREG32(&g_pTZIC->ENSET[2]);
    registerStore[7] = INREG32(&g_pTZIC->ENSET[3]);

    // Mask all interrupts
    OUTREG32(&g_pGPIO1->IMR, 0);
    OUTREG32(&g_pGPIO2->IMR, 0);
    OUTREG32(&g_pGPIO3->IMR, 0);
    OUTREG32(&g_pGPIO4->IMR, 0);
    OUTREG32(&g_pTZIC->ENCLEAR[0], 0xFFFFFFFF);
    OUTREG32(&g_pTZIC->ENCLEAR[1], 0xFFFFFFFF);
    OUTREG32(&g_pTZIC->ENCLEAR[2], 0xFFFFFFFF);
    OUTREG32(&g_pTZIC->ENCLEAR[3], 0xFFFFFFFF);

    // Now enable interrupt if it was enabled as wakeup source
    for (sysIntr = SYSINTR_DEVICES; sysIntr < SYSINTR_MAXIMUM; sysIntr++)
    {
        // Skip if sysIntr isn't allowed as wake source
        if (!OALPowerWakeSource(sysIntr)) continue;
        
        // Enable it as wakeup source interrupt
        OEMInterruptEnable(sysIntr, NULL, 0);

        // Set flag to indicate we have at least one wake source
        PowerState = FALSE;
    }


    if (!PowerState)
    {
        // Switch off power for KITL device
        KITLIoctl(IOCTL_KITL_POWER_CALL, &PowerState, sizeof(PowerState), NULL, 0, NULL);

        // Place the system in suspend state and wait for an interrupt.
        OALMSG(OAL_POWER, 
            (_T("INFO: OEMPowerOff entering suspend.  ENSET[0] = 0x%x, ENSET[1] = 0x%x, ENSET[2] = 0x%x, ENSET[3] = 0x%x\r\n"), 
             INREG32(&g_pTZIC->ENSET[0]), INREG32(&g_pTZIC->ENSET[1]), INREG32(&g_pTZIC->ENSET[2]), INREG32(&g_pTZIC->ENSET[3])));

        // Disable the OS tick timer (EPIT1) while we are powered off.  This is 
        // necessary since the EPIT can be clocked from CKIL and will continue
        // to run in some low-power modes.
        INSREG32BF(&g_pEPIT->CR, EPIT_CR_EN, EPIT_CR_EN_DISABLE);

        // Power off ...
        OALCPUPowerOff();

        // Firstly enable the OS tick timer (EPIT1)
        INSREG32BF(&g_pEPIT->CR, EPIT_CR_EN, EPIT_CR_EN_ENABLE);

        // Get interrupt source
        for (irqSet = 0; irqSet < TZIC_IRQ_SET_MAX; irqSet++)
        {
            line = _CountLeadingZeros(INREG32(&g_pTZIC->HIPND[irqSet]));
            if (line < 32)
            {
                irq = (irqSet << 5) + (31 - line);
                break;
            }
        }

        OALMSG(OAL_POWER, (_T("INFO: OEMPowerOff leaving suspend.  IRQ = 0x%x\r\n"), irq));

        // If valid wake interrupt is pending
        if (irq < TZIC_IRQ_SOURCES_MAX)
        {
            if ((irq == IRQ_GPIO1_UPPER16) || (irq == IRQ_GPIO1_LOWER16))
            {
                // detect GPIO line that is asserting interrupt
                line = _CountLeadingZeros(INREG32(&g_pGPIO1->ISR) 
                                          & INREG32(&g_pGPIO1->IMR));

                // If at least one GPIO interrupt line is asserted
                if (line < 32)
                { 
                    irq = IRQ_GPIO1_PIN0 + (31 - line);
                }

            } // GPIO1 special case

            else if ((irq == IRQ_GPIO2_UPPER16) || (irq == IRQ_GPIO2_LOWER16))
            {
                // detect GPIO line that is asserting interrupt
                line = _CountLeadingZeros(INREG32(&g_pGPIO2->ISR) 
                                          & INREG32(&g_pGPIO2->IMR));

                // If at least one GPIO interrupt line is asserted
                if (line < 32)
                { 
                    irq = IRQ_GPIO2_PIN0 + (31 - line);
                }

            } // GPIO2 special case

            else if ((irq == IRQ_GPIO3_UPPER16) || (irq == IRQ_GPIO3_LOWER16))
            {
                // detect GPIO line that is asserting interrupt
                line = _CountLeadingZeros(INREG32(&g_pGPIO3->ISR) 
                                          & INREG32(&g_pGPIO3->IMR));

                // If at least one GPIO interrupt line is asserted
                if (line < 32)
                { 
                    irq = IRQ_GPIO3_PIN0 + (31 - line);
                }
            } // GPIO3 special case

            else if ((irq == IRQ_GPIO4_UPPER16) || (irq == IRQ_GPIO4_LOWER16))
            {
                // detect GPIO line that is asserting interrupt
                line = _CountLeadingZeros(INREG32(&g_pGPIO4->ISR) 
                                          & INREG32(&g_pGPIO4->IMR));

                // If at least one GPIO interrupt line is asserted
                if (line < 32)
                { 
                    irq = IRQ_GPIO4_PIN0 + (31 - line);
                }
            } // GPIO4 special case

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
    }
    else
    {
        OALMSG(OAL_INFO, (_T("INFO: No wake sources for OEMPowerOff, system will resume...\r\n")));
    }


    // Restore state of enabled GPIO interrupts
    OUTREG32(&g_pGPIO1->IMR, registerStore[0]);
    OUTREG32(&g_pGPIO2->IMR, registerStore[1]);
    OUTREG32(&g_pGPIO3->IMR, registerStore[2]);
    OUTREG32(&g_pGPIO4->IMR, registerStore[3]);

    // Restore state of enabled interrupts
    OUTREG32(&g_pTZIC->ENCLEAR[0], 0xFFFFFFFF);
    OUTREG32(&g_pTZIC->ENSET[0], registerStore[4]);
    OUTREG32(&g_pTZIC->ENCLEAR[1], 0xFFFFFFFF);
    OUTREG32(&g_pTZIC->ENSET[1], registerStore[5]);
    OUTREG32(&g_pTZIC->ENCLEAR[2], 0xFFFFFFFF);
    OUTREG32(&g_pTZIC->ENSET[2], registerStore[6]);
    OUTREG32(&g_pTZIC->ENCLEAR[3], 0xFFFFFFFF);
    OUTREG32(&g_pTZIC->ENSET[3], registerStore[7]);

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
    INSREG32(&g_pCCM->CCGR[CCM_CGR_INDEX(index)], CCM_CGR_MASK(index),
             CCM_CGR_VAL(index, mode));

    INTERRUPTS_ENABLE(fEnable);
}


//-----------------------------------------------------------------------------
//
//  Function: OALClockEnableIIM
//
//  This function enables/disables module clocks for the IIM module. 
//
//  Parameters:
//      bClockEnable
//           [in] Set TRUE to enable IIM module clocks.  Set FALSE
//           to disable IIM module clocks.
//
//  Returns:
//      None
//
//-----------------------------------------------------------------------------
VOID OALClockEnableIIM(BOOL bClockEnable)
{
    DDK_CLOCK_GATE_MODE mode = bClockEnable ? DDK_CLOCK_GATE_MODE_ENABLED_ALL :
        DDK_CLOCK_GATE_MODE_DISABLED;

    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_IIM, mode);
}

