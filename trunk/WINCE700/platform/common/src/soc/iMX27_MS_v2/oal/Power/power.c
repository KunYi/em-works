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
// NOTE: stubs are being used - this isn't done
//
//------------------------------------------------------------------------------
//
// Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  off.c
//
//  This file provides the capabilities to suspend the system and controlling
//  wake sources.
//
//------------------------------------------------------------------------------

#include <windows.h>
#include <ceddk.h>
#include <nkintr.h>
#include <cmnintrin.h>
#include <oal.h>
#include "csp.h"

//------------------------------------------------------------------------------
// External Functions
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// External Variables
//------------------------------------------------------------------------------
extern PCSP_AITC_REGS g_pAITC;
extern PCSP_PLLCRC_REGS g_pPLLCRC;
extern PCSP_GPIO_REGS g_pGPIO;
//extern BOOL g_bBSPIrq;

//------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------
VOID OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX index, 
    DDK_CLOCK_GATE_MODE mode);
//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Global Variables
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Local Variables
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Local Functions
//------------------------------------------------------------------------------


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
VOID OEMPowerOff()
{
    UINT32 intEnableH;
    UINT32 intEnableL;
    UINT32 gpioimrstate[GPIO_PORT_MAX];
    UINT32 sysIntr;
    UINT32 irq;
    UINT32 line;
    GPIO_PORT port;

    // Let BSP do board specific stuff
    BSPPowerOff();    

    // Save state of enabled interrupts
    intEnableH = INREG32(&g_pAITC->INTENABLEH);
    intEnableL = INREG32(&g_pAITC->INTENABLEL);
    gpioimrstate[GPIO_PORT_A] = INREG32(&g_pGPIO->PORT[GPIO_PORT_A].IMR);
    gpioimrstate[GPIO_PORT_B] = INREG32(&g_pGPIO->PORT[GPIO_PORT_B].IMR);
    gpioimrstate[GPIO_PORT_C] = INREG32(&g_pGPIO->PORT[GPIO_PORT_C].IMR);
    gpioimrstate[GPIO_PORT_D] = INREG32(&g_pGPIO->PORT[GPIO_PORT_D].IMR);
    gpioimrstate[GPIO_PORT_E] = INREG32(&g_pGPIO->PORT[GPIO_PORT_E].IMR);
    gpioimrstate[GPIO_PORT_F] = INREG32(&g_pGPIO->PORT[GPIO_PORT_F].IMR);    
 
    // Disable all GPIO interrups 
    OUTREG32(&g_pGPIO->PORT[GPIO_PORT_A].IMR,0);
    OUTREG32(&g_pGPIO->PORT[GPIO_PORT_B].IMR,0);
    OUTREG32(&g_pGPIO->PORT[GPIO_PORT_C].IMR,0);
    OUTREG32(&g_pGPIO->PORT[GPIO_PORT_D].IMR,0);
    OUTREG32(&g_pGPIO->PORT[GPIO_PORT_E].IMR,0);	
    OUTREG32(&g_pGPIO->PORT[GPIO_PORT_F].IMR,0);	
	
    // Disable all 
    OUTREG32(&g_pAITC->INTENABLEH, 0);
    OUTREG32(&g_pAITC->INTENABLEL, 0);
  
    // Now enable interrupt if it was enabled as wakeup source    
    for(sysIntr = SYSINTR_DEVICES; sysIntr < SYSINTR_MAXIMUM; sysIntr++)
    {
        // Skip if sysIntr isn't allowed as wake source
        if (!OALPowerWakeSource(sysIntr)) continue;
        // Enable it as interrupt
        OEMInterruptEnable(sysIntr, NULL, 0);
    }
	
    // Power off KITL
    OALKitlPowerOff();
   
    // Disable MPLL. When the conditions are satisfied, 
    // the MPLL will be turned off, and this action also
    // automatically turns off the SPLL. We don't need
    // to disable clocks to all modules by clearing PCCR0 
    // and PCCR1 in CRM module.     
    g_pPLLCRC->CSCR &= ~(CSP_BITFMASK(PLLCRC_CSCR_MPEN));

    // Finally power off CPU
    OALCPUPowerOff();

    // Find wakeup source
    irq = (UINT32)EXTREG32BF(&g_pAITC->NIVECSR, AITC_NIVECSR_NIVECTOR);
        
    if(irq == IRQ_GPIO)
    {        
        for(port = GPIO_PORT_A; port < GPIO_PORT_MAX; port++)
        {
            // detect GPIO line that is asserting interrupt
            line = _CountLeadingZeros(INREG32(&g_pGPIO->PORT[port].ISR) 
                                  & INREG32(&g_pGPIO->PORT[port].IMR)); 
            // If at least one GPIO interrupt line is asserted   
            if (line < 32)
            {
                line = 31 - line;
                irq = CSP_IRQ_GPIO_MIN + (port * 32 + line);
                break;
            }                          
        }            
        // Invalid GPIO interrupt
      //  if(irq == IRQ_GPIO)
       //     irq = OAL_INTR_IRQ_UNDEFINED;
    }
    
#if 0//#ifdef OAL_BSP_CALLBACKS
    // Give BSP chance to translate IRQ 
    irq = BSPIntrActiveIrq(irq);

    if(g_bBSPIrq) // If porcessed by BSP, We should clear the GPIO IRQ status
    {
         SETREG32(&g_pGPIO->PORT[port].ISR,(1 << line));
    }
#endif

    // When we find wakup source set global variable
    g_oalWakeSource = OALIntrTranslateIrq(irq);

    // Power on CPU
    OALCPUPowerOn();    

    // Restore KITL
    OALKitlPowerOn();    

    // Then let BSP do board specific stuff
    BSPPowerOn();

    // Restore original interrupt masks
    OUTREG32(&g_pGPIO->PORT[GPIO_PORT_A].IMR, gpioimrstate[GPIO_PORT_A]);
    OUTREG32(&g_pGPIO->PORT[GPIO_PORT_B].IMR, gpioimrstate[GPIO_PORT_B]);
    OUTREG32(&g_pGPIO->PORT[GPIO_PORT_C].IMR, gpioimrstate[GPIO_PORT_C]);
    OUTREG32(&g_pGPIO->PORT[GPIO_PORT_D].IMR, gpioimrstate[GPIO_PORT_D]);
    OUTREG32(&g_pGPIO->PORT[GPIO_PORT_E].IMR, gpioimrstate[GPIO_PORT_E]);	
    OUTREG32(&g_pGPIO->PORT[GPIO_PORT_F].IMR, gpioimrstate[GPIO_PORT_F]);
    OUTREG32(&g_pAITC->INTENABLEH, intEnableH);
    OUTREG32(&g_pAITC->INTENABLEL, intEnableL);

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
    REG32* pPCCR = &g_pPLLCRC->PCCR0;
    
    // CGR is a shared register so we must disable interrupts temporarily for
    // safe access
    if (mode)
    {
		fEnable = INTERRUPTS_ENABLE(FALSE);
	  	SETREG32(&pPCCR[CRM_PCCR_INDEX(index)], CRM_PCCR_VAL(index, 1));
		INTERRUPTS_ENABLE(fEnable);
    }
      else 
    {
		fEnable = INTERRUPTS_ENABLE(FALSE);
	  	CLRREG32(&pPCCR[CRM_PCCR_INDEX(index)], CRM_PCCR_VAL(index, 1));
		INTERRUPTS_ENABLE(fEnable);
    }	    
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
    DDK_CLOCK_GATE_MODE mode = bClockEnable ? DDK_CLOCK_GATE_MODE_ENABLE :
        DDK_CLOCK_GATE_MODE_DISABLE;
    
    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_IIM, mode);
}

