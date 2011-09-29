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
extern UINT32 OALRTCAlarmIntrHandler(ULONG ra);

//------------------------------------------------------------------------------
// External Variables


//------------------------------------------------------------------------------
// Defines


//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Global Variables
PVOID pv_HWregICOLL   = NULL;
PVOID pv_HWregPINCTRL = NULL;

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

    OALMSG(OAL_FUNC&&OAL_INTR, (L"+OALIntrInit\r\n"));

    // Initialize interrupt mapping
    OALIntrMapInit();

    // Get uncached virtual addresses for ICOLL
    pv_HWregICOLL = (PVOID) OALPAtoUA(CSP_BASE_REG_PA_ICOLL);
    if (pv_HWregICOLL == NULL)
    {
        OALMSG(OAL_ERROR, (L"OALIntrInit: ICOLL null pointer!\r\n"));
        goto cleanUp;
    }

    // Get uncached virtual addresses for PINCTRL
    pv_HWregPINCTRL = (PVOID) OALPAtoUA(CSP_BASE_REG_PA_PINCTRL);
    if (pv_HWregPINCTRL == NULL)
    {
        OALMSG(OAL_ERROR, (L"OALIntrInit: PINCTRL null pointer!\r\n"));
        goto cleanUp;
    }

    // Reset Interrupt Collector, the reset state is all interrups are disabled
    // priority 0
    HW_ICOLL_CTRL_SET(BM_ICOLL_CTRL_SFTRST);
    HW_ICOLL_CTRL_RD();

    HW_ICOLL_CTRL_CLR(BM_ICOLL_CTRL_SFTRST);
    HW_ICOLL_CTRL_RD();

    HW_ICOLL_CTRL_CLR(BM_ICOLL_CTRL_CLKGATE);
    HW_ICOLL_CTRL_RD();

    // Enable the Final IRQ and Final FIQ by default.
    HW_ICOLL_CTRL_SET(BM_ICOLL_CTRL_FIQ_FINAL_ENABLE |
                      BM_ICOLL_CTRL_IRQ_FINAL_ENABLE |
                      BM_ICOLL_CTRL_NO_NESTING       |
                      BM_ICOLL_CTRL_ARM_RSE_MODE);

    // bump up the Timer interrupt priority
    HW_ICOLL_INTERRUPTn_SET(IRQ_TIMER0,BV_ICOLL_INTERRUPTn_PRIORITY__LEVEL3);


    //mask and clear PINCTRL IRQs
    HW_PINCTRL_IRQEN0_CLR(0xFFFFFFFF);
    HW_PINCTRL_IRQSTAT0_CLR(0xFFFFFFFF);

    HW_PINCTRL_IRQEN1_CLR(0xFFFFFFFF);
    HW_PINCTRL_IRQSTAT1_CLR(0xFFFFFFFF);

    HW_PINCTRL_IRQEN2_CLR(0xFFFFFFFF);
    HW_PINCTRL_IRQSTAT2_CLR(0xFFFFFFFF);

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
               pDevLoc->Pin, pCount, pIrqs));

    if (pIrqs == NULL || pCount == NULL || *pCount < 1) goto cleanUp;

    switch (pDevLoc->IfcType)
    {
    case Internal:

        switch (pDevLoc->LogicalLoc)
        {
        case CSP_BASE_REG_PA_USB:
            *pIrqs = IRQ_USB_CTRL;
            *pCount = 1;
            rc = TRUE;
            break;
            
        case CSP_BASE_REG_PA_UARTAPP0:
            *pIrqs = IRQ_UARTAPP_INTERNAL;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_UARTAPP1:
            *pIrqs = IRQ_UARTAPP2_INTERNAL;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_SSP1:
            *pIrqs = IRQ_GPIO2_PIN3;
            *pCount = 1;
            rc = TRUE;
            break;

        default:
            OALMSGS(1, (TEXT("default : %d\r\n"), *pCount));
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
               L"+OALIntrEnableIrqs(%d, 0x%08x)\r\n", count, (UINT32)pIrqs
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
        case 2:    
            // Primary IRQ in AVIC
            HW_ICOLL_INTERRUPTn_SET(irq, BM_ICOLL_INTERRUPTn_ENABLE);
            break;
    
        case 3:
            // Secondary IRQ in GPIO0
            HW_PINCTRL_IRQEN0_SET(1U << (irq - IRQ_GPIO0_PIN0));
            HW_ICOLL_INTERRUPTn_SET(IRQ_GPIO0, BM_ICOLL_INTERRUPTn_ENABLE);
            break;
    
        case 4:
            // Secondary IRQ in GPIO1
            HW_PINCTRL_IRQEN1_SET(1U << (irq - IRQ_GPIO1_PIN0));
            HW_ICOLL_INTERRUPTn_SET(IRQ_GPIO1, BM_ICOLL_INTERRUPTn_ENABLE);
            break;
    
        case 5:
            // Secondary IRQ in GPIO2
            HW_PINCTRL_IRQEN2_SET(1U << (irq - IRQ_GPIO2_PIN0));
            HW_ICOLL_INTERRUPTn_SET(IRQ_GPIO2, BM_ICOLL_INTERRUPTn_ENABLE);
            break;
    
        default:
            // Invalid IRQ
            rc = FALSE;
            break;
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
               L"+OALIntrDisableIrqs(%d, 0x%08x)\r\n", count, (UINT32)pIrqs
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


        //  Disable IRQ
        switch (irq >> 5)
        {
        case 0:
        case 1:
        case 2:    
            // Primary IRQ in AVIC
            // Disable this interrupt by clearing it's icoll enable bit. The interrupt
            // source will be cleared and re-enabled in the handler. The soft trigger
            // is not gated by the enable bit, so clear it here to keep it from
            // triggering again when we exit the ISR.
            HW_ICOLL_INTERRUPTn_CLR(irq, BM_ICOLL_INTERRUPTn_ENABLE | BM_ICOLL_INTERRUPTn_SOFTIRQ);
            break;

        case 3:
            // Secondary IRQ in GPIO0
            HW_PINCTRL_IRQEN0_CLR(1U << (irq - IRQ_GPIO0_PIN0));
            break;
    
        case 4:
            // Secondary IRQ in GPIO1
            HW_PINCTRL_IRQEN1_CLR(1U << (irq - IRQ_GPIO1_PIN0));
            break;
    
        case 5:
            // Secondary IRQ in GPIO2
            HW_PINCTRL_IRQEN2_CLR(1U << (irq - IRQ_GPIO2_PIN0));
            break;

        default:
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
               L"+OALIntrDoneIrqs(%d, 0x%08x)\r\n", count, (UINT32)pIrqs
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
        case 2:    
            // Primary IRQ in AVIC
            HW_ICOLL_INTERRUPTn_SET(irq, BM_ICOLL_INTERRUPTn_ENABLE);
            break;
    
        case 3:
            // Secondary IRQ in GPIO0
            HW_PINCTRL_IRQEN0_SET(1U << (irq - IRQ_GPIO0_PIN0));
            break;
    
        case 4:
            // Secondary IRQ in GPIO1
            HW_PINCTRL_IRQEN1_SET(1U << (irq - IRQ_GPIO1_PIN0));
            break;
    
        case 5:
            // Secondary IRQ in GPIO2
            HW_PINCTRL_IRQEN2_SET(1U << (irq - IRQ_GPIO2_PIN0));

            break;

        default:
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
    UINT32 dwLevel;
    UINT32 line;

    // Read the vector address and compute the vector number. This also signals
    // inservice to the icoll hardware because the ARM read side effect (RSE)
    // is enabled. We are using the default one word vector pitch.
    irq = (HW_ICOLL_VECTOR_RD() / 4) & 0x7F;

    // Make sure interrupt is pending
    if (irq >= ICOLL_IRQ_SOURCES_MAX)
    {
        OALMSGS(OAL_ERROR,(TEXT("OEMInterrupHandler:  No pending interrupt!\r\n")));
    }

    // If system timer interrupt
    else if (irq == IRQ_TIMER0)
    {
        // Call timer interrupt handler
        sysIntr = OALTimerIntrHandler();
    }

    // If profile timer interrupt
    else if (g_pProfilerISR && (irq == IRQ_TIMER1))
    {
        //OALMSGS(1,(TEXT("ProfilerISR!\r\n")));

        // Call profiling interupt handler
        sysIntr = g_pProfilerISR(ra);
    }
    else if (irq == IRQ_RTC_ALARM)
    {
        sysIntr = OALRTCAlarmIntrHandler(ra);
    }
    // Else not system timer interrupt
    else
    {
#ifdef OAL_ILTIMING
        if (g_oalILT.active) 
        {
            g_oalILT.isrTime1 = OALTimerCountsSinceSysTick();
            g_oalILT.savedPC = 0;
            g_oalILT.interrupts++;
        }
#endif
        // GPIO0 special case
        if(irq == IRQ_GPIO0)
        {
            // Detect GPIO line that is asserting interrupt
            line = _CountLeadingZeros(HW_PINCTRL_IRQSTAT0_RD() & HW_PINCTRL_IRQEN0_RD());

            // If at least one GPIO interrupt line is asserted
            if (line < 32)
            {
                // Translate it to the secondary IRQ
                irq = IRQ_GPIO0_PIN0 + (31 - line);
            }
            else
            {
                irq = (UINT32) OAL_INTR_IRQ_UNDEFINED;
            }
        }

        // GPIO1 special case
        else if(irq == IRQ_GPIO1)
        {
            // Detect GPIO line that is asserting interrupt
            line = _CountLeadingZeros(HW_PINCTRL_IRQSTAT1_RD() & HW_PINCTRL_IRQEN1_RD());

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
        else if(irq == IRQ_GPIO2)
        { 
            // Detect GPIO line that is asserting interrupt
            line = _CountLeadingZeros(HW_PINCTRL_IRQSTAT2_RD() & HW_PINCTRL_IRQEN2_RD());

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

            // Mask all IRQs associated with the sysintr
            OEMInterruptMask(sysIntr, TRUE);
        }
        else
        {
            OALMSGS(OAL_ERROR,(TEXT("OEMInterruptHandler:  undefined IRQ (%d)!\r\n"),((HW_ICOLL_VECTOR_RD() / 4) & 0x3F)));
        }
    } // Else not system timer interrupt

    dwLevel = HW_ICOLL_INTERRUPTn_RD(irq) & BM_ICOLL_INTERRUPTn_PRIORITY;

    // Clear Interrupt
    HW_ICOLL_LEVELACK_SET(1 << dwLevel);

    return sysIntr;
}


