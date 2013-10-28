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
//
//  File: intr.c
//
//  This file implements the major part of interrupt support for the 
//  MX27 SoC.
//
//------------------------------------------------------------------------------
#include <windows.h>
#include <ceddk.h>
#include <nkintr.h>
#include <cmnintrin.h>
#include <oal.h>
#include "csp.h"
#include <intr.h>



//------------------------------------------------------------------------------
// External Functions


//------------------------------------------------------------------------------
// External Variables

//------------------------------------------------------------------------------
// Defines

//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Global Variables
PCSP_AITC_REGS g_pAITC;
PCSP_GPIO_REGS g_pGPIO;
BOOL g_bBSPIrq = FALSE;

//  Function pointer to profiling timer ISR routine.
//
PFN_PROFILER_ISR g_pProfilerISR = NULL;

UINT32 g_IRQ_RTC = IRQ_RTIC;

//------------------------------------------------------------------------------
//
//  Function:  OALIntrInit
//
//  This function initialize interrupt mapping, hardware and call platform
//  specific initialization.
//
BOOL OALIntrInit()
{
    BOOL rc = FALSE;
    GPIO_PORT port;

    OALMSG( OAL_FUNC&&OAL_INTR, (L"+OALInterruptInit\r\n") );

    // Initialize interrupt mapping
    OALIntrMapInit();

    // Get uncached virtual addresses for AITC
    g_pAITC = (PCSP_AITC_REGS) OALPAtoUA(CSP_BASE_REG_PA_AITC);
    if (g_pAITC == NULL)
    {
        OALMSG(OAL_ERROR, (L"OALIntrInit:  AITC null pointer!\r\n"));
        goto cleanUp;
    }

    // Get uncached virtual addresses for GPIO modules
    g_pGPIO = (PCSP_GPIO_REGS)OALPAtoUA(CSP_BASE_REG_PA_GPIO);
    if (g_pGPIO == NULL)
    {
        OALMSG(OAL_ERROR, (L"OALIntrInit:  GPIO null pointer!\r\n"));
        goto cleanUp;
    }

    // Enable normal interrupt to raise ARM core priority in bus request
    OUTREG32(&g_pAITC->INTCNTL, CSP_BITFMASK(AITC_INTCNTL_NIAD));
     
    // Mask and clear all GPIO IRQs,Unmask GPIO port level interrupt
    for(port = GPIO_PORT_A; port < GPIO_PORT_MAX; port++)
    {
        g_pGPIO->PORT[port].IMR = 0;
        g_pGPIO->PORT[port].ISR = 0xFFFFFFFF;
        g_pGPIO->PMASK |= (1 << port);
    }    
 
#ifdef OAL_BSP_CALLBACKS
    // Give BSP chance to initialize subordinate controller
    rc = BSPIntrInit();
#else
    rc = TRUE;
#endif

cleanUp:
    OALMSG(OAL_INTR&&OAL_FUNC, (L"-OALInterruptInit(rc = %d)\r\n", rc));
    return rc;
}



//------------------------------------------------------------------------------
//
//  Function:  OALIntrRequestIrqs
//
//  This function returns IRQs for CPU/SoC devices based on their
//  physical address.
//
BOOL OALIntrRequestIrqs(DEVICE_LOCATION *pDevLoc, UINT32 *pCount, UINT32 *pIrqs)
{
    BOOL rc = FALSE;

    OALMSG(OAL_INTR&&OAL_FUNC, (
        L"+OALIntrRequestIrqs(0x%08x->%d/%d/0x%08x/%d, 0x%08x, 0x%08x)\r\n",
        pDevLoc, pDevLoc->IfcType, pDevLoc->BusNumber, pDevLoc->LogicalLoc,
        pDevLoc->Pin, pCount, pIrqs
    )); 

    if (pIrqs == NULL || pCount == NULL || *pCount < 1) goto cleanUp;

   switch (pDevLoc->IfcType) 
    {
    case Internal:

        switch (pDevLoc->LogicalLoc)
        {
        case CSP_BASE_REG_PA_I2C2:
            pIrqs[0] = IRQ_I2C2;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_GPT6:
            pIrqs[0] = IRQ_GPT6;
            *pCount = 1;
            rc = TRUE;
            break;	

        case CSP_BASE_REG_PA_GPT5:
            pIrqs[0] = IRQ_GPT5;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_GPT4:
            pIrqs[0] = IRQ_GPT4;
            *pCount = 1;
            rc = TRUE;
            break;
			
        case CSP_BASE_REG_PA_RTIC:
            pIrqs[0] = IRQ_RTIC;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_CSPI3:
            pIrqs[0] = IRQ_CSPI3;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_MSHC:
            pIrqs[0] = IRQ_MSHC;
            *pCount = 1;
            rc = TRUE;
            break;
			
        case CSP_BASE_REG_PA_GPIO:
            pIrqs[0] = IRQ_GPIO;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_SDHC3:
            pIrqs[0] = IRQ_SDHC3;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_SDHC2:
            pIrqs[0] = IRQ_SDHC2;
            *pCount = 1;
            rc = TRUE;
            break;
			
        case CSP_BASE_REG_PA_SDHC1:
            pIrqs[0] = IRQ_SDHC1;
            *pCount = 1;
            rc = TRUE;
            break;
			
        case CSP_BASE_REG_PA_I2C1:
            pIrqs[0] = IRQ_I2C1;
            *pCount = 1;
            rc = TRUE;
            break;
            
        case CSP_BASE_REG_PA_SSI2:
            pIrqs[0] = IRQ_SSI2;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_SSI1:
            pIrqs[0] = IRQ_SSI1;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_CSPI2:
            pIrqs[0] = IRQ_CSPI2;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_CSPI1:
            pIrqs[0] = IRQ_CSPI1;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_UART4:
            pIrqs[0] = IRQ_UART4;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_UART3:
            pIrqs[0] = IRQ_UART3;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_UART2:
            pIrqs[0] = IRQ_UART2;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_UART1:
            pIrqs[0] = IRQ_UART1;
            *pCount = 1;
            rc = TRUE;
            break;
			

        case CSP_BASE_REG_PA_KPP:
            pIrqs[0] = IRQ_KPP;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_RTC:
            pIrqs[0] = IRQ_RTC;
            *pCount = 1;
            rc = TRUE;
            break;
			
        case CSP_BASE_REG_PA_PWM:
            pIrqs[0] = IRQ_PWM;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_GPT3:
            pIrqs[0] = IRQ_GPT3;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_GPT2:
            pIrqs[0] = IRQ_GPT2;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_GPT1:
            pIrqs[0] = IRQ_GPT1;
            *pCount = 1;
            rc = TRUE;
            break;				

        case CSP_BASE_REG_PA_WDOG:
            pIrqs[0] = IRQ_WDOG;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_PCMCIA:
            pIrqs[0] = IRQ_PCMCIA;  // Slammed SIM IRQ
            *pCount = 1;
            rc = TRUE;
            break;
            
        case CSP_BASE_REG_PA_NANDFC:
            pIrqs[0] = IRQ_NFC;
            *pCount = 1;
            rc = TRUE;
            break;
            
        case CSP_BASE_REG_PA_ATA:
            pIrqs[0] = IRQ_ATA;
            *pCount = 1;
            rc = TRUE;
            break;
            
        case CSP_BASE_REG_PA_CSI:
            pIrqs[0] = IRQ_CSI;
            *pCount = 1;
            rc = TRUE;
            break;
            
        case CSP_BASE_REG_PA_UART6:
            pIrqs[0] = IRQ_UART6;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_UART5:
            pIrqs[0] = IRQ_UART5;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_FEC:
            pIrqs[0] = IRQ_FEC;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_EMMA:
           // Check if output array can hold all IRQs
            if (*pCount >= 2)
            {
                pIrqs[0] = IRQ_EMMAPRP;
                pIrqs[1] = IRQ_EMMAPP;
                *pCount = 2;
                rc = TRUE;
            }
            break;

        case CSP_BASE_REG_PA_VCM:
            pIrqs[0] = IRQ_H264;
            *pCount = 1;
            rc = TRUE;
            break;
			
        case CSP_BASE_REG_PA_USB:
            // Check if output array can hold all IRQs
            if (*pCount >= 3)
            {
                pIrqs[0] = IRQ_USBHS1;
                pIrqs[1] = IRQ_USBHS2;
                pIrqs[2] = IRQ_USBOTG;
                *pCount = 3;
                rc = TRUE;
            }
            break;

        case CSP_BASE_REG_PA_SCC:
            // Check if output array can hold all IRQs
            if (*pCount >= 2)
            {
                pIrqs[0] = IRQ_SMN;
                pIrqs[1] = IRQ_SCM;
                *pCount = 2;
                rc = TRUE;
            }
            break;

        case CSP_BASE_REG_PA_SAHARA:
            pIrqs[0] = IRQ_SAHARA;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_SLCDC:
            pIrqs[0] = IRQ_SLCDC;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_LCDC:
            pIrqs[0] = IRQ_LCDC;   
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_IIM:
            pIrqs[0] = IRQ_IIM;
            *pCount = 1;
            rc = TRUE;
            break;
}

        break;

    }

#ifdef OAL_BSP_CALLBACKS
    if (!rc) rc = BSPIntrRequestIrqs(pDevLoc, pCount, pIrqs);
#endif    

cleanUp:        
    OALMSG(OAL_INTR&&OAL_FUNC, (L"-OALIntrRequestIrqs(rc = %d)\r\n", rc));
    return rc;
}


//------------------------------------------------------------------------------
//
//  Function:  OALIntrEnableIrqs
//
BOOL OALIntrEnableIrqs(UINT32 count, const UINT32 *pIrqs)
{
    BOOL rc = TRUE;
    UINT32 i, irq;
    UINT32 pin;
    GPIO_PORT port;

    OALMSG(OAL_INTR&&OAL_FUNC, (
        L"+OALIntrEnableIrqs(%d, 0x%08x)\r\n", count, pIrqs
    ));

    for (i = 0; i < count; i++) {
#ifndef OAL_BSP_CALLBACKS
        irq = pIrqs[i];
#else
        // Give BSP chance to enable irq on subordinate interrupt controller
        irq = BSPIntrEnableIrq(pIrqs[i]);
#endif
        if (irq == OAL_INTR_IRQ_UNDEFINED) continue;

        // If IRQ is valid
        if (irq <= CSP_IRQ_GPIO_MAX)
        {
            if(irq >= CSP_IRQ_GPIO_MIN)
            {
                // Enable GPIO pin interrupt
                pin = irq - CSP_IRQ_GPIO_MIN;
                port = pin / 32;
                pin %= 32;
                SETREG32(&g_pGPIO->PORT[port].IMR,(1 << pin));
                SETREG32(&g_pGPIO->PORT[port].ISR,(1 << pin));
                irq = IRQ_GPIO;
            }            
            // Enable the primary IRQ
            OUTREG32(&g_pAITC->INTENNUM, irq);            
        } else {
            rc = FALSE;
        }
    }

    OALMSG(OAL_INTR&&OAL_FUNC, (L"-OALIntrEnableIrqs(rc = %d)\r\n", rc));
    return rc;    
}


//------------------------------------------------------------------------------
//
//  Function:  OALIntrDisableIrqs
//
VOID OALIntrDisableIrqs(UINT32 count, const UINT32 *pIrqs)
{
    UINT32 i, irq;
    UINT32 pin;
    GPIO_PORT port;
    
    OALMSG(OAL_INTR&&OAL_FUNC, (
        L"+OALIntrDisableIrqs(%d, 0x%08x)\r\n", count, pIrqs
    ));

    for (i = 0; i < count; i++) {
#ifndef OAL_BSP_CALLBACKS
        irq = pIrqs[i];
#else
        // Give BSP chance to disable irq on subordinate interrupt controller
        irq = BSPIntrDisableIrq(pIrqs[i]);
#endif
        if (irq == OAL_INTR_IRQ_UNDEFINED) continue;

        // If IRQ is valid
        if (irq <= CSP_IRQ_GPIO_MAX)
        {
            if(irq >= CSP_IRQ_GPIO_MIN)
            {
                // Disable GPIO pin interrupt
                pin = irq - CSP_IRQ_GPIO_MIN;
                port = pin / 32;
                pin %= 32;
                CLRREG32(&g_pGPIO->PORT[port].IMR,(1 << pin));
            }            
            else
            {
                // Disable the primary IRQ
                OUTREG32(&g_pAITC->INTDISNUM, irq);
            }          
        }
    }

    OALMSG(OAL_INTR&&OAL_FUNC, (L"-OALIntrDisableIrqs\r\n"));
}


//------------------------------------------------------------------------------
//
//  Function:  OALIntrDoneIrqs
//
VOID OALIntrDoneIrqs(UINT32 count, const UINT32 *pIrqs)
{
    UINT32 i, irq;
    UINT32 pin;
    BOOL bEnable;
    GPIO_PORT port;
    
    
    OALMSG(OAL_INTR&&OAL_VERBOSE, (
        L"+OALIntrDoneIrqs(%d, 0x%08x)\r\n", count, pIrqs
    ));

    for (i = 0; i < count; i++) {
#ifndef OAL_BSP_CALLBACKS
        irq = pIrqs[i];
#else
        // Give BSP chance to finish irq on subordinate interrupt controller
        irq = BSPIntrDoneIrq(pIrqs[i]);
#endif    
        if (irq == OAL_INTR_IRQ_UNDEFINED) continue;
		
        // Upper OAL layer is not disabling interrupts, but we
        // need interrupts disabled for safe access to GPIO and
        // AVIC registers
        bEnable = INTERRUPTS_ENABLE(FALSE);
        // If IRQ is valid
        if (irq <= CSP_IRQ_GPIO_MAX)
        {
            if(irq >= CSP_IRQ_GPIO_MIN)
            {
                // Enable GPIO pin interrupt
                pin = irq - CSP_IRQ_GPIO_MIN;
                port = pin / 32;
                pin %= 32;
                SETREG32(&g_pGPIO->PORT[port].ISR,(1 << pin));
                SETREG32(&g_pGPIO->PORT[port].IMR,(1 << pin));
                irq = IRQ_GPIO;
            }            
            // Enable the primary IRQ
            OUTREG32(&g_pAITC->INTENNUM, irq);            
        }
        INTERRUPTS_ENABLE(bEnable);
    }

    OALMSG(OAL_INTR&&OAL_VERBOSE, (L"-OALIntrDoneIrqs\r\n"));
}


//------------------------------------------------------------------------------
//
//  Function:  OEMInterruptHandler
//
ULONG OEMInterruptHandler(ULONG ra)
{
    UINT32 sysIntr = SYSINTR_NOP;
    UINT32 irq;
    UINT32 line;
    GPIO_PORT port;

    irq = EXTREG32BF(&g_pAITC->NIVECSR, AITC_NIVECSR_NIVECTOR);

    // Make sure interrupt is pending
    if (irq >= AITC_IRQ_SOURCES_MAX)
    {
        OALMSGS(OAL_ERROR, 
            (TEXT("OEMInterrupHandler:  No pending interrupt!\r\n")));
    }

    // If system timer interrupt
    else if (irq == IRQ_GPT1) 
    {
        // Call timer interrupt handler
        sysIntr = OALTimerIntrHandler();
    } 

    // If profile timer interrupt
    else if (g_pProfilerISR && (irq == IRQ_GPT3))
    {
	 // Mask the interrupt
	 OUTREG32(&g_pAITC->INTDISNUM, irq);
        // Call profiling interupt handler
        sysIntr = g_pProfilerISR(ra);
    }
    
    // Else not system timer interrupt
    else 
    {

       // GPIO special case:  Interrupts from module are ORed together
        // so we cannot disable the IRQ_GPIO since this would block other 
        // drivers relying on GPIO interrupts.  Instead we clear the interrupt
        // at the GPIO and then disable all interrupts associated with the
        // SYSINTR that owns this GPIO interrupt, including the GPIO line
        // that caused the interrupt to occur.
        if (irq == IRQ_GPIO)
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
            if(irq == IRQ_GPIO)
                irq = OAL_INTR_IRQ_UNDEFINED;
        } // GPIO special case
        
#ifdef OAL_BSP_CALLBACKS
        // Give BSP chance to translate IRQ -- if there is subordinate
        // interrupt controller in BSP it give chance to decode its status
        // and change IRQ
        irq = BSPIntrActiveIrq(irq);
#endif
        if(g_bBSPIrq) // If porcessed by BSP, We should clear the GPIO IRQ status
        {
                 SETREG32(&g_pGPIO->PORT[port].ISR,(1 << line));
        }

        // if IRQ assigned to this GPIO is defined
        if (irq != OAL_INTR_IRQ_UNDEFINED)
        {
            if(!g_bBSPIrq) // Not porcessed by BSP
            {    if((irq >= CSP_IRQ_GPIO_MIN) && (irq <= CSP_IRQ_GPIO_MAX))
                {
                    // Mask GPIO pin interrupt
                    CLRREG32(&g_pGPIO->PORT[port].IMR,(1 << line));
                }
                else
                {
                    // Mask the AITC interrupt
                    OUTREG32(&g_pAITC->INTDISNUM, irq);
                }                
            } 
       
            // First find if IRQ is claimed by chain
            sysIntr = NKCallIntChain((UCHAR)irq);
            if (sysIntr == SYSINTR_CHAIN || !NKIsSysIntrValid(sysIntr)) 
            {
                // IRQ wasn't claimed, use static mapping
                 sysIntr = OALIntrTranslateIrq(irq);		 
            }
            
            // unmask interrupts in case it's NOP or invalid
            if (SYSINTR_NOP == sysIntr) 
            {
#ifdef OAL_BSP_CALLBACKS   
                // Have porcessed by BSP         
                if(g_bBSPIrq) 
                {
                    // BSP specific irq
                    BSPIntrEnableIrq (irq);
                }
                else 
#endif              
                {  
                    if((irq >= CSP_IRQ_GPIO_MIN) && (irq <= CSP_IRQ_GPIO_MAX))
                    {
                        // Enable GPIO pin interrupt
                        SETREG32(&g_pGPIO->PORT[port].ISR,(1 << line));
                        SETREG32(&g_pGPIO->PORT[port].IMR,(1 << line));  
                    }
                    else
                    {
                        // Unmask the AITC interrupt
                        OUTREG32(&g_pAITC->INTENNUM, irq);
                    }
                }
            }
        }
        else
        {
            OALMSGS(OAL_ERROR, 
                (TEXT("OEMInterrupHandler:  undefined IRQ (%d)!\r\n"),
                EXTREG32BF(&g_pAITC->NIVECSR, AITC_NIVECSR_NIVECTOR))); 
        }
    } // Else not system timer interrupt
    return sysIntr;
}

//------------------------------------------------------------------------------
