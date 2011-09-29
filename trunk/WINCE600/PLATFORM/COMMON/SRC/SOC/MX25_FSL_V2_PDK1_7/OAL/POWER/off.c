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
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
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
extern PCSP_GPIO_REGS g_pGPIO4;
extern PCSP_SDMA_REGS g_pSDMA;
extern PCSP_CRM_REGS g_pCRM;
extern PCSP_EPIT_REG g_pEPIT;
extern UINT32 g_oalSysIntr2Irq[SYSINTR_MAXIMUM][IRQ_PER_SYSINTR];

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
VOID OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX index, DDK_CLOCK_GATE_MODE mode);
VOID OALInitializeClockGating(void);
static BOOL OALWakeupInterruptEnable(DWORD sysIntr);

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
    UINT32 regSave[9];
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
    regSave[5] = INREG32(&g_pGPIO4->IMR);

    regSave[6] = INREG32(&g_pCRM->CGR_REGS.CGR[0]);
    regSave[7] = INREG32(&g_pCRM->CGR_REGS.CGR[1]);
    regSave[8] = INREG32(&g_pCRM->CGR_REGS.CGR[2]);

    // Mask all interrupts
    OUTREG32(&g_pAVIC->INTENABLEH, 0);
    OUTREG32(&g_pAVIC->INTENABLEL, 0);
    OUTREG32(&g_pGPIO1->IMR, 0);
    OUTREG32(&g_pGPIO2->IMR, 0);
    OUTREG32(&g_pGPIO3->IMR, 0);
    OUTREG32(&g_pGPIO4->IMR, 0);

    // Mask all CRM STOP mode wakeup interrupts
    OUTREG32(&g_pCRM->LPIMR0, 0xFFFFFFFF);
    OUTREG32(&g_pCRM->LPIMR1, 0xFFFFFFFF);

    // Now enable interrupt if it was enabled as wakeup source
    for (sysIntr = SYSINTR_DEVICES; sysIntr < SYSINTR_MAXIMUM; sysIntr++)
    {
        // Skip if sysIntr isn't allowed as wake source
        if (!OALPowerWakeSource(sysIntr)) continue;
        
        // Enable it as wakeup source interrupt
        OALWakeupInterruptEnable(sysIntr);
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

    // turn off all clocks except the what is required for code execution.
    OALInitializeClockGating();

    // Power off ...
    OALCPUPowerOff();

    // Restore clock gating state
    OUTREG32(&g_pCRM->CGR_REGS.CGR[0], regSave[6]);
    OUTREG32(&g_pCRM->CGR_REGS.CGR[1], regSave[7]);
    OUTREG32(&g_pCRM->CGR_REGS.CGR[2], regSave[8]);

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
    OUTREG32(&g_pGPIO4->IMR, regSave[5]);

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
    INSREG32(&g_pCRM->CGR_REGS.CGR[CRM_CGR_INDEX(index)], CRM_CGR_MASK(index), 
        CRM_CGR_VAL(index, mode));

    INTERRUPTS_ENABLE(fEnable);
}

//-----------------------------------------------------------------------------
//
//  Function: OALInitializeClockGating()
//
//  This function provides the OAL a mechanism to initialize the clock gates
//
//  Parameters:
//      None
//
//  Returns:
//      None
//
//-----------------------------------------------------------------------------

VOID OALInitializeClockGating(void)
{
    //setup the low power mode so that the CPU clock is gated off during WFI.
    INSREG32(&g_pCRM->CCTL,
        CSP_BITFMASK(CRM_CCTL_LP_CTL), 
        CSP_BITFVAL(CRM_CCTL_LP_CTL, CRM_CCTL_LP_CTL_WAIT));

    //Disable alll the clock except the one for the EMI (External Memory interface) because of the the systems runs from the external RAM.
    OUTREG32(&g_pCRM->CGR_REGS.CGR[0],CRM_CGR_VAL(DDK_CLOCK_GATE_INDEX_AHB_EMI, DDK_CLOCK_GATE_MODE_ENABLED));
    OUTREG32(&g_pCRM->CGR_REGS.CGR[1],0);
    OUTREG32(&g_pCRM->CGR_REGS.CGR[2],0);

    // ENGcm07872 : CG_CTL does not work as expected. we need to keep it to 0.    
    SETREG32(&g_pCRM->CCTL,CSP_BITFVAL(CRM_CCTL_CG_CTL,0));

    
}

//-----------------------------------------------------------------------------
//
//  Function: OALWakeInterruptEnable
//
//  This function enables wakeup interrupt on both AVIC and CRM.
//
//  Parameters:
//      sysIntr
//          [in] SYSINTR of wakeup interrupt.
//
//  Returns:
//      TRUE if sysIntr is valid, else FALSE.
//
//-----------------------------------------------------------------------------
static BOOL OALWakeupInterruptEnable(DWORD sysIntr)
{
    BOOL rc = TRUE;
    UINT32 i, irq;
    
    // Validate sysIntr
    if (sysIntr >= SYSINTR_MAXIMUM)
    {
        OALMSG(OAL_ERROR, (L"ERROR: OALWakeupInterruptEnable: "
            L"Invalid sysIntr value %d\r\n", sysIntr
            ));
        rc = FALSE;
        goto cleanUp;
    }

    for (i = 0; i < IRQ_PER_SYSINTR; i++)
    {
        // Give BSP chance to enable irq on subordinate interrupt controller
        irq = BSPIntrEnableIrq(g_oalSysIntr2Irq[sysIntr][i]);

        if (irq == OAL_INTR_IRQ_UNDEFINED) continue;
        
        // Enable IRQ
        switch (irq >> 5)
        {
        case 0:
        case 1:
            // Primary IRQ in AVIC
            OUTREG32(&g_pAVIC->INTENNUM, irq);
            // Enable CRM STOP mode wakeup interrupt
            if (irq > 31)
            {
                CLRREG32(&g_pCRM->LPIMR1, (1 << (irq - 32)));
            }
            else
            {
                CLRREG32(&g_pCRM->LPIMR0, (1 << irq));
            }
            break;

        case 2:
            // Secondary IRQ in GPIO1
            SETREG32(&g_pGPIO1->IMR, 1U << (irq - IRQ_GPIO1_PIN0));
            OUTREG32(&g_pAVIC->INTENNUM, IRQ_GPIO1);
            // Enable GPIO1 interrupt as CRM STOP mode wakeup source
            CLRREG32(&g_pCRM->LPIMR1, (1 << (IRQ_GPIO1 - 32)));
            break;

        case 3:
            // Secondary IRQ in GPIO2
            SETREG32(&g_pGPIO2->IMR, 1U << (irq - IRQ_GPIO2_PIN0));
            OUTREG32(&g_pAVIC->INTENNUM, IRQ_GPIO2);
            // Enable GPIO2 interrupt as CRM STOP mode wakeup source
            CLRREG32(&g_pCRM->LPIMR1, (1 << (IRQ_GPIO2 - 32)));
            break;

        case 4:
            // Secondary IRQ in GPIO3
            SETREG32(&g_pGPIO3->IMR, 1U << (irq - IRQ_GPIO3_PIN0));
            OUTREG32(&g_pAVIC->INTENNUM, IRQ_GPIO3);
            // Enable GPIO3 interrupt as CRM STOP mode wakeup source
            CLRREG32(&g_pCRM->LPIMR0, (1 << IRQ_GPIO3));
            break;
       
        case 5:
            // Secondary IRQ in GPIO4
            SETREG32(&g_pGPIO4->IMR, 1U << (irq - IRQ_GPIO4_PIN0));
            OUTREG32(&g_pAVIC->INTENNUM, IRQ_GPIO4);
            // Enable GPIO4 interrupt as CRM STOP mode wakeup source
            CLRREG32(&g_pCRM->LPIMR0, (1 << IRQ_GPIO4));
            break;

        case 6:
            // Secondary IRQ in SDMA (no secondary mask)
            // so nothing to do here
            // ...
            // Enable SDMA interrupt as CRM STOP mode wakeup source
            CLRREG32(&g_pCRM->LPIMR1, (1 << (IRQ_SDMA - 32)));            
            break;

        default:
            // Invalid IRQ
            rc = FALSE;
        }
    }

cleanUp:
    return rc;
}

