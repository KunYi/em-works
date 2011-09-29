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
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File: intr.c
//
//  PQOAL interrupt support.
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

#include <intr.h>
#include <csp.h>


//------------------------------------------------------------------------------
// External Functions
extern UINT32 OALProfileGetIrq(void);

//------------------------------------------------------------------------------
// External Variables


//------------------------------------------------------------------------------
// Defines


//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Global Variables
PCSP_AVIC_REGS g_pAVIC;
PCSP_GPIO_REGS g_pGPIO1;
PCSP_GPIO_REGS g_pGPIO2;
PCSP_GPIO_REGS g_pGPIO3;
PCSP_GPIO_REGS g_pGPIO4;
PCSP_SDMA_REGS g_pSDMA;

//  Function pointer to profiling timer ISR routine.
PFN_PROFILER_ISR g_pProfilerISR = NULL;


//------------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
// Local Functions


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
    UINT32 irq;

    OALMSG(OAL_FUNC&&OAL_INTR, (L"+OALIntrInit\r\n"));

    // Initialize interrupt mapping
    OALIntrMapInit();

    // Get uncached virtual addresses for AVIC
    g_pAVIC = (PCSP_AVIC_REGS) OALPAtoUA(CSP_BASE_REG_PA_AVIC);
    if (g_pAVIC == NULL)
    {
        OALMSG(OAL_ERROR, (L"OALIntrInit: AVIC null pointer!\r\n"));
        goto cleanUp;
    }

    // Get uncached virtual addresses for GPIO modules
    g_pGPIO1 = (PCSP_GPIO_REGS) OALPAtoUA(CSP_BASE_REG_PA_GPIO1);
    if (g_pGPIO1 == NULL)
    {
        OALMSG(OAL_ERROR, (L"OALIntrInit: GPIO1 null pointer!\r\n"));
        goto cleanUp;
    }

    g_pGPIO2 = (PCSP_GPIO_REGS) OALPAtoUA(CSP_BASE_REG_PA_GPIO2);
    if (g_pGPIO2 == NULL)
    {
        OALMSG(OAL_ERROR, (L"OALIntrInit: GPIO2 null pointer!\r\n"));
        goto cleanUp;
    }

    g_pGPIO3 = (PCSP_GPIO_REGS) OALPAtoUA(CSP_BASE_REG_PA_GPIO3);
    if (g_pGPIO3 == NULL)
    {
        OALMSG(OAL_ERROR, (L"OALIntrInit: GPIO3 null pointer!\r\n"));
        goto cleanUp;
    }

    g_pGPIO4 = (PCSP_GPIO_REGS) OALPAtoUA(CSP_BASE_REG_PA_GPIO4);
    if (g_pGPIO4 == NULL)
    {
        OALMSG(OAL_ERROR, (L"OALIntrInit: GPIO4 null pointer!\r\n"));
        goto cleanUp;
    }

    // Get uncached virtual addresses for SDMA
    g_pSDMA = (PCSP_SDMA_REGS) OALPAtoUA(CSP_BASE_REG_PA_SDMA);
    if (g_pSDMA == NULL)
    {
        OALMSG(OAL_ERROR, (L"OALIntrInit: SDMA null pointer!\r\n"));
        goto cleanUp;
    }

    // Mask and clear all GPIO IRQs
    OUTREG32(&g_pGPIO1->IMR, 0);
    OUTREG32(&g_pGPIO1->ISR, 0xFFFFFFFF);
    OUTREG32(&g_pGPIO2->IMR, 0);
    OUTREG32(&g_pGPIO2->ISR, 0xFFFFFFFF);
    OUTREG32(&g_pGPIO3->IMR, 0);
    OUTREG32(&g_pGPIO3->ISR, 0xFFFFFFFF);
    OUTREG32(&g_pGPIO4->IMR, 0);
    OUTREG32(&g_pGPIO4->ISR, 0xFFFFFFFF);

    // Clear all SDMA channel interrupt bits (w1c)
    OUTREG32(&g_pSDMA->INTR, 0xFFFFFFFF);

    // Enable the SDMA IRQ
    irq = IRQ_SDMA;
    if (!OALIntrEnableIrqs(1, &irq))
    {
        OALMSG(OAL_WARN, 
            (L"OALIntrInit: can't enable IRQ_SDMA\r\n"));
    }

#ifdef OAL_BSP_CALLBACKS
    // Give BSP chance to initialize subordinate controller
    rc = BSPIntrInit();
#else
    rc = TRUE;
#endif

cleanUp:
    OALMSG(OAL_INTR&&OAL_FUNC, (L"-OALIntrInit(rc = %d)\r\n", rc));
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
        case CSP_BASE_REG_PA_CSPI3:
            pIrqs[0] = IRQ_CSPI3;
            *pCount = 1;
            rc = TRUE;
            break;
        case CSP_BASE_REG_PA_GPT4:
            pIrqs[0] = IRQ_GPT4;
            *pCount = 1;
            rc = TRUE;
            break;
        case CSP_BASE_REG_PA_OWIRE:
            pIrqs[0] = IRQ_OWIRE;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_I2C1:
            pIrqs[0] = IRQ_I2C1;
            *pCount = 1;
            rc = TRUE;
            break;
                
        case CSP_BASE_REG_PA_I2C2:
            pIrqs[0] = IRQ_I2C2;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_UART4:
            pIrqs[0] = IRQ_UART4;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_RTIC:
            pIrqs[0] = IRQ_RTIC;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_ESAI:
            pIrqs[0] = IRQ_ESAI;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_ESDHC2:
            pIrqs[0] = IRQ_ESDHC2;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_ESDHC1:
            pIrqs[0] = IRQ_ESDHC1;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_I2C3:
            pIrqs[0] = IRQ_I2C3;
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

        case CSP_BASE_REG_PA_ATA:
            pIrqs[0] = IRQ_ATA;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_GPIO3:
            pIrqs[0] = IRQ_GPIO3;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_CSI:
            pIrqs[0] = IRQ_CSI;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_UART3:
            pIrqs[0] = IRQ_UART3;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_IIM:
            pIrqs[0] = IRQ_IIM;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_SIM1:
            pIrqs[0] = IRQ_SIM1;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_SIM2:
            pIrqs[0] = IRQ_SIM2;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_RNGB:
            pIrqs[0] = IRQ_RNGB;
            *pCount = 1;
            rc = TRUE;
            break;
            
        case CSP_BASE_REG_PA_KPP:
            pIrqs[0] = IRQ_KPP;
            *pCount = 1;
            rc = TRUE;
            break;
 
        case CSP_BASE_REG_PA_PWM1:
            pIrqs[0] = IRQ_PWM;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_EPIT2:
            pIrqs[0] = IRQ_EPIT2;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_EPIT1:
            pIrqs[0] = IRQ_EPIT1;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_GPT3:
            pIrqs[0] = IRQ_GPT3;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_UART2:
            pIrqs[0] = IRQ_UART2;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_NANDFC:
            pIrqs[0] = IRQ_NANDFC;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_SDMA:
            pIrqs[0] = IRQ_SDMA;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_USB:
            // Check if output array can hold all IRQs
            if (*pCount >= 2)
            {
                *pIrqs = IRQ_USB_HOST;
                *(pIrqs + 1) = IRQ_USB_OTG;
                *pCount = 2;
                rc = TRUE;
            }
            break;

        case CSP_BASE_REG_PA_PWM2:
            pIrqs[0] = IRQ_PWM2;
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

        case CSP_BASE_REG_PA_UART5:
            pIrqs[0] = IRQ_UART5;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_PWM3:
            pIrqs[0] = IRQ_PWM3;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_PWM4:
            pIrqs[0] = IRQ_PWM3;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_CAN1:
            pIrqs[0] = IRQ_CAN1;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_CAN2:
            pIrqs[0] = IRQ_CAN2;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_UART1:
            pIrqs[0] = IRQ_UART1;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_TCHSC:
            pIrqs[0] = IRQ_TCHSC;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_SCC:
            // Check if output array can hold all IRQs
            if (*pCount >= 2)
            {
                *pIrqs = IRQ_SCC_SCM;
                *(pIrqs + 1) = IRQ_SCC_SMN;
                *pCount = 2;
                rc = TRUE;
            }
            break;

        case CSP_BASE_REG_PA_GPIO2:
            pIrqs[0] = IRQ_GPIO2;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_GPIO1:
            pIrqs[0] = IRQ_GPIO1;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_WDOG:
            pIrqs[0] = IRQ_WDOG;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_DRYICE:
            pIrqs[0] = IRQ_DRYICE;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_FEC:
            pIrqs[0] = IRQ_FEC;
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

    OALMSG(OAL_INTR&&OAL_FUNC, (
        L"+OALIntrEnableIrqs(%d, 0x%08x)\r\n", count, pIrqs
    ));

    for (i = 0; i < count; i++)
    {
#ifndef OAL_BSP_CALLBACKS
        irq = pIrqs[i];
#else
        // Give BSP chance to enable irq on subordinate interrupt controller
        irq = BSPIntrEnableIrq(pIrqs[i]);
#endif
        if (irq == OAL_INTR_IRQ_UNDEFINED) continue;
        
        // Enable IRQ
        switch (irq >> 5)
        {
        case 0:
        case 1:
            // Primary IRQ in AVIC
            OUTREG32(&g_pAVIC->INTENNUM, irq);
            break;

        case 2:
            // Secondary IRQ in GPIO1
            SETREG32(&g_pGPIO1->IMR, 1U << (irq - IRQ_GPIO1_PIN0));
            OUTREG32(&g_pAVIC->INTENNUM, IRQ_GPIO1);
            break;

        case 3:
            // Secondary IRQ in GPIO2
            SETREG32(&g_pGPIO2->IMR, 1U << (irq - IRQ_GPIO2_PIN0));
            OUTREG32(&g_pAVIC->INTENNUM, IRQ_GPIO2);
            break;

        case 4:
            // Secondary IRQ in GPIO3
            SETREG32(&g_pGPIO3->IMR, 1U << (irq - IRQ_GPIO3_PIN0));
            OUTREG32(&g_pAVIC->INTENNUM, IRQ_GPIO3);
            break;
       
        case 5:
            // Secondary IRQ in GPIO4
            SETREG32(&g_pGPIO4->IMR, 1U << (irq - IRQ_GPIO4_PIN0));
            OUTREG32(&g_pAVIC->INTENNUM, IRQ_GPIO4);
            break;

        case 6:
            // Secondary IRQ in SDMA (no secondary mask)
            break;

        default:
            // Invalid IRQ
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

    OALMSG(OAL_INTR&&OAL_FUNC, (
        L"+OALIntrDisableIrqs(%d, 0x%08x)\r\n", count, pIrqs
    ));

    for (i = 0; i < count; i++)
    {
#ifndef OAL_BSP_CALLBACKS
        irq = pIrqs[i];
#else
        // Give BSP chance to disable irq on subordinate interrupt controller
        irq = BSPIntrDisableIrq(pIrqs[i]);
#endif
        if (irq == OAL_INTR_IRQ_UNDEFINED) continue;
        
        // Disable IRQ
        switch (irq >> 5)
        {
        case 0:
        case 1:
            // Primary IRQ in AVIC
            OUTREG32(&g_pAVIC->INTDISNUM, irq);
            break;

        case 2:
            // Secondary IRQ in GPIO1
            CLRREG32(&g_pGPIO1->IMR, 1U << (irq - IRQ_GPIO1_PIN0));
            break;

        case 3:
            // Secondary IRQ in GPIO2
            CLRREG32(&g_pGPIO2->IMR, 1U << (irq - IRQ_GPIO2_PIN0));
            break;

        case 4:
            // Secondary IRQ in GPIO3
            CLRREG32(&g_pGPIO3->IMR, 1U << (irq - IRQ_GPIO3_PIN0));
            break;

        case 5:
            // Secondary IRQ in GPIO4
            CLRREG32(&g_pGPIO4->IMR, 1U << (irq - IRQ_GPIO4_PIN0));
            break;

        case 6:
            // Secondary IRQ in SDMA (no secondary mask)
            break;
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
    BOOL bEnable;

    OALMSG(OAL_INTR&&OAL_VERBOSE, (
        L"+OALIntrDoneIrqs(%d, 0x%08x)\r\n", count, pIrqs
    ));

    for (i = 0; i < count; i++)
    {
#ifndef OAL_BSP_CALLBACKS
        irq = pIrqs[i];
#else
        // Give BSP chance to finish irq on subordinate interrupt controller
        irq = BSPIntrDoneIrq(pIrqs[i]);
#endif
        if (irq == OAL_INTR_IRQ_UNDEFINED) continue;

        // GPIO and SDMA primary interrupts always retain enabled, 
        // so we need to disable interrupt to safely access AVIC, 
        // GPIO and SDMA registers.
        bEnable = INTERRUPTS_ENABLE(FALSE);

        // Clear ISR (w1c) and enable IRQ
        switch (irq >> 5)
        {
        case 0:
        case 1:
            // Primary IRQ in AVIC (No ISR)
            OUTREG32(&g_pAVIC->INTENNUM, irq);
            break;

        // We do not clear GPIO secondary ISR here, because we do not want 
        // the GPIO interrupt be cleared by OALIntrDoneIrqs called from 
        // the context of other interrupts, which are associated with the 
        // same one sysintr. Instead, driver needs to call DDKGpioClearIntrPin
        // to clear ISR bit when the interrupt has been done by driver.
        case 2:
            // Secondary IRQ in GPIO1
            SETREG32(&g_pGPIO1->IMR, 1U << (irq - IRQ_GPIO1_PIN0));
            break;

        case 3:
            // Secondary IRQ in GPIO2
            SETREG32(&g_pGPIO2->IMR, 1U << (irq - IRQ_GPIO2_PIN0));
            break;

        case 4:
            // Secondary IRQ in GPIO3
            SETREG32(&g_pGPIO3->IMR, 1U << (irq - IRQ_GPIO3_PIN0));
            break;

        case 5:
            // Secondary IRQ in GPIO4
            SETREG32(&g_pGPIO4->IMR, 1U << (irq - IRQ_GPIO4_PIN0));
            break;

        case 6:
            // Secondary IRQ in SDMA (No secondary mask)
            break;
        }

        // Enable interrupt
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

    irq = EXTREG32(&g_pAVIC->NIVECSR, CSP_BITFMASK(AVIC_NIVECSR_NIVECTOR),
        AVIC_NIVECSR_NIVECTOR_LSH);

    // Make sure interrupt is pending
    if (irq >= AVIC_IRQ_SOURCES_MAX)
    {
        OALMSGS(OAL_ERROR, 
            (TEXT("OEMInterrupHandler:  No pending interrupt!\r\n")));
    }

    // If system timer interrupt
    else if (irq == IRQ_EPIT1) 
    {
        // Call timer interrupt handler
        sysIntr = OALTimerIntrHandler();
    } 

    // If profile timer interrupt
    else if (g_pProfilerISR && (irq == OALProfileGetIrq()))
    {
        // Call profiling interupt handler
        sysIntr = g_pProfilerISR(ra);
    }
    
    // Else not system timer interrupt
    else 
    {
#ifdef OAL_ILTIMING
        if (g_oalILT.active) {
            g_oalILT.isrTime1 = OALTimerCountsSinceSysTick();
            g_oalILT.savedPC = 0;
            g_oalILT.interrupts++;
        }        
#endif

        // GPIO1 special case
        if (irq == IRQ_GPIO1)
        {
            // Detect GPIO line that is asserting interrupt
            line = _CountLeadingZeros(INREG32(&g_pGPIO1->ISR) 
                                      & INREG32(&g_pGPIO1->IMR));

            // If at least one GPIO interrupt line is asserted
            if (line < 32)
            {
                // Translate it to the secondary IRQ
                irq = IRQ_GPIO1_PIN0 + (31 - line);
            }
            else
            {
                irq = (UINT32) OAL_INTR_IRQ_UNDEFINED;
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
                // Translate it to the secondary IRQ
                irq = IRQ_GPIO2_PIN0 + (31 - line);
            }
            else
            {
                irq = (UINT32) OAL_INTR_IRQ_UNDEFINED;
            }
        }

        // GPIO3 special case
        else if (irq == IRQ_GPIO3)
        {
            // Detect GPIO line that is asserting interrupt
            line = _CountLeadingZeros(INREG32(&g_pGPIO3->ISR) 
                                      & INREG32(&g_pGPIO3->IMR));

            // If at least one GPIO interrupt line is asserted
            if (line < 32)
            {
                // Translate it to the secondary IRQ
                irq = IRQ_GPIO3_PIN0 + (31 - line);
            }
            else
            {
                irq = (UINT32) OAL_INTR_IRQ_UNDEFINED;
            }
        }

        // GPIO4 special case
        else if (irq == IRQ_GPIO4)
        {
            // Detect GPIO line that is asserting interrupt
            line = _CountLeadingZeros(INREG32(&g_pGPIO4->ISR) 
                                      & INREG32(&g_pGPIO4->IMR));

            // If at least one GPIO interrupt line is asserted
            if (line < 32)
            {
                // Translate it to the secondary IRQ
                irq = IRQ_GPIO4_PIN0 + (31 - line);
            }
            else
            {
                irq = (UINT32) OAL_INTR_IRQ_UNDEFINED;
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
                line = 31 - line;

                // Need to clear ISR here because SDMA has no secondary mask.
                // Otherwise the pending interrupt will make this handler 
                // block other threads.
                OUTREG32(&g_pSDMA->INTR, 1U << line);
                
                // Translate it to the secondary IRQ
                irq = IRQ_SDMA_CH0 + line;
            }
            else
            {
                irq = (UINT32) OAL_INTR_IRQ_UNDEFINED;
            }
        }
        
#ifdef OAL_BSP_CALLBACKS
        // Give BSP chance to translate IRQ -- if there is subordinate
        // interrupt controller in the BSP give it a chance to decode
        // its status and change the IRQ
        irq = BSPIntrActiveIrq(irq);
#endif

        // If IRQ is defined
        if (irq != OAL_INTR_IRQ_UNDEFINED)
        {
            // First find if IRQ is claimed by chain
            sysIntr = NKCallIntChain((UCHAR)irq);
            if (sysIntr == SYSINTR_CHAIN || !NKIsSysIntrValid(sysIntr))
            {
                // IRQ wasn't claimed, use static mapping
                sysIntr = OALIntrTranslateIrq(irq);
            }
            if (sysIntr < SYSINTR_MAXIMUM)
            // Mask all IRQs associated with the sysintr
            OEMInterruptMask(sysIntr, TRUE);
        }
        else
        {
            OALMSGS(OAL_ERROR, 
                (TEXT("OEMInterruptHandler:  undefined IRQ (%d)!\r\n"), 
                EXTREG32(&g_pAVIC->NIVECSR, CSP_BITFMASK(AVIC_NIVECSR_NIVECTOR),
                AVIC_NIVECSR_NIVECTOR_LSH)));
        }
    }
    return sysIntr;
}

//------------------------------------------------------------------------------
