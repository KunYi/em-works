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
//  Copyright (C) 2004-2007, Freescale Semiconductor, Inc. All Rights Reserved.
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


//------------------------------------------------------------------------------
// External Variables


//------------------------------------------------------------------------------
// Defines

#ifdef VERBOSE_INTENABLE
#define TRACE_INTENABLE \
    OALMSG(OAL_INFO, (L"g_oalIrqMask.HighPart = 0x%x\r\n", g_oalIrqMask[irq].HighPart)); \
    OALMSG(OAL_INFO, (L"g_oalIrqMask.LowPart = 0x%x\r\n", g_oalIrqMask[irq].LowPart)); \
    OALMSG(OAL_INFO, (L"g_oalGpioMask[0] = 0x%x\r\n", g_oalGpioMask[0][irq])); \
    OALMSG(OAL_INFO, (L"g_oalGpioMask[1] = 0x%x\r\n", g_oalGpioMask[1][irq])); \
    OALMSG(OAL_INFO, (L"g_oalGpioMask[2] = 0x%x\r\n", g_oalGpioMask[2][irq])); \
    OALMSG(OAL_INFO, (L"INTENABLEH = 0x%x\r\n", INREG32(&g_pAVIC->INTENABLEH))); \
    OALMSG(OAL_INFO, (L"INTENABLEL = 0x%x\r\n", INREG32(&g_pAVIC->INTENABLEL))); \
    OALMSG(OAL_INFO, (L"GPIO1_IMR = 0x%x\r\n", INREG32(&g_pGPIO1->IMR))); \
    OALMSG(OAL_INFO, (L"GPIO2_IMR = 0x%x\r\n", INREG32(&g_pGPIO2->IMR))); \
    OALMSG(OAL_INFO, (L"GPIO3_IMR = 0x%x\r\n", INREG32(&g_pGPIO3->IMR)));
#else
#define TRACE_INTENABLE
#endif // VERBOSE_INTENABLE


#define OAL_IRQ_ENABLE(irq) \
    if (g_oalGpioMask[0][irq]) SETREG32(&g_pGPIO1->IMR, g_oalGpioMask[0][irq]); \
    if (g_oalGpioMask[1][irq]) SETREG32(&g_pGPIO2->IMR, g_oalGpioMask[1][irq]); \
    if (g_oalGpioMask[2][irq]) SETREG32(&g_pGPIO3->IMR, g_oalGpioMask[2][irq]); \
    if (g_oalIrqMask[irq].HighPart) SETREG32(&g_pAVIC->INTENABLEH, g_oalIrqMask[irq].HighPart); \
    if (g_oalIrqMask[irq].LowPart) SETREG32(&g_pAVIC->INTENABLEL, g_oalIrqMask[irq].LowPart); \
    TRACE_INTENABLE;

#define OAL_IRQ_DISABLE(irq) \
    if (g_oalIrqMask[irq].HighPart) CLRREG32(&g_pAVIC->INTENABLEH, g_oalIrqMask[irq].HighPart); \
    if (g_oalIrqMask[irq].LowPart) CLRREG32(&g_pAVIC->INTENABLEL, g_oalIrqMask[irq].LowPart); \
    if (g_oalGpioMask[0][irq]) CLRREG32(&g_pGPIO1->IMR, g_oalGpioMask[0][irq]); \
    if (g_oalGpioMask[1][irq]) CLRREG32(&g_pGPIO2->IMR, g_oalGpioMask[1][irq]); \
    if (g_oalGpioMask[2][irq]) CLRREG32(&g_pGPIO3->IMR, g_oalGpioMask[2][irq]); \
    TRACE_INTENABLE;


//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Global Variables

UINT32 g_oalIrqTranslate[OAL_INTR_IRQ_MAXIMUM];
ULARGE_INTEGER g_oalIrqMask[OAL_INTR_IRQ_MAXIMUM];
UINT32 g_oalGpioTranslate[DDK_GPIO_PORT3+1][GPIO_INTR_SOURCES_MAX];
UINT32 g_oalSdmaTranslate[SDMA_NUM_CHANNELS];
UINT32 g_oalGpioMask[DDK_GPIO_PORT3+1][OAL_INTR_IRQ_MAXIMUM];
PCSP_AVIC_REGS g_pAVIC;

const UINT32 g_IRQ_RTC = IRQ_RTC; // Create a global variable for the
                                  // SOC-specific IRQ that is used by
                                  // the RTC.

//  Function pointer to profiling timer ISR routine.
//
PFN_PROFILER_ISR g_pProfilerISR = NULL;


//------------------------------------------------------------------------------
// Local Variables

PCSP_GPIO_REGS g_pGPIO1;
PCSP_GPIO_REGS g_pGPIO2;
PCSP_GPIO_REGS g_pGPIO3;
PCSP_SDMA_REGS g_pSDMA;


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
    UCHAR i;
    UINT32 irq;

    OALMSG( OAL_FUNC&&OAL_INTR, (L"+OALInterruptInit\r\n") );

    // Initialize interrupt mapping
    OALIntrMapInit();

    // Get uncached virtual addresses for AVIC
    g_pAVIC = (PCSP_AVIC_REGS) OALPAtoUA(CSP_BASE_REG_PA_AVIC);
    if (g_pAVIC == NULL)
    {
        OALMSG(OAL_ERROR, (L"OALIntrInit:  AVIC null pointer!\r\n"));
        goto cleanUp;
    }

    // Get uncached virtual addresses for GPIO modules
    g_pGPIO1 = (PCSP_GPIO_REGS) OALPAtoUA(CSP_BASE_REG_PA_GPIO1);
    if (g_pGPIO1 == NULL)
    {
        OALMSG(OAL_ERROR, (L"OALIntrInit:  GPIO1 null pointer!\r\n"));
        goto cleanUp;
    }

    g_pGPIO2 = (PCSP_GPIO_REGS) OALPAtoUA(CSP_BASE_REG_PA_GPIO2);
    if (g_pGPIO2 == NULL)
    {
        OALMSG(OAL_ERROR, (L"OALIntrInit:  GPIO2 null pointer!\r\n"));
        goto cleanUp;
    }

    g_pGPIO3 = (PCSP_GPIO_REGS) OALPAtoUA(CSP_BASE_REG_PA_GPIO3);
    if (g_pGPIO3 == NULL)
    {
        OALMSG(OAL_ERROR, (L"OALIntrInit:  GPIO3 null pointer!\r\n"));
        goto cleanUp;
    }

    // Get uncached virtual addresses for SDMA
    g_pSDMA = (PCSP_SDMA_REGS) OALPAtoUA(CSP_BASE_REG_PA_SDMA);
    if (g_pSDMA == NULL)
    {
        OALMSG(OAL_ERROR, (L"OALIntrInit:  SDMA null pointer!\r\n"));
        goto cleanUp;
    }

    // Initialize SoC-specific translation tables
    for (i = 0; i < OAL_INTR_IRQ_MAXIMUM; i++)
    {
        g_oalIrqTranslate[i] = i;
        g_oalIrqMask[i].QuadPart = CSP_IRQMASK(i);
    }

    memset(g_oalGpioTranslate, OAL_INTR_IRQ_UNDEFINED,
        sizeof(g_oalGpioTranslate));
    memset(g_oalSdmaTranslate, OAL_INTR_IRQ_UNDEFINED,
        sizeof(g_oalSdmaTranslate));
    memset(g_oalGpioMask, 0, sizeof(g_oalGpioMask));

    // Mask and clear all GPIO IRQs
    OUTREG32(&g_pGPIO1->IMR, 0);
    OUTREG32(&g_pGPIO1->ISR, 0xFFFFFFFF);
    OUTREG32(&g_pGPIO2->IMR, 0);
    OUTREG32(&g_pGPIO2->ISR, 0xFFFFFFFF);
    OUTREG32(&g_pGPIO3->IMR, 0);
    OUTREG32(&g_pGPIO3->ISR, 0xFFFFFFFF);

    // Translation table above maps provides a one-to-one mapping
    // between AVIC IRQs and the IRQ table maintained for SYSINTR
    // mapping.  For peripherals that have multiple IRQs that we want
    // to be reported as a single SYSINTR, we can "slam" them by
    // overriding the corresponding table values.  This translation
    // is required since PQOAL assumes one-to-one mapping between
    // SYSINTR and IRQ.
    //
    // NOTE:  Maintain consistency with slammed IRQs and the
    //        IRQs returned from OALIntrRequestIrqs

    // Slam SIM IRQs to IRQ_SIM_COMMON
    g_oalIrqMask[IRQ_SIM_COMMON].QuadPart =
        CSP_IRQMASK(IRQ_SIM_COMMON) | CSP_IRQMASK(IRQ_SIM_DATA);
    g_oalIrqTranslate[IRQ_SIM_DATA] = IRQ_SIM_COMMON;

    // Slam IPU IRQs to IRQ_IPU_GENERAL
    g_oalIrqMask[IRQ_IPU_GENERAL].QuadPart =
        CSP_IRQMASK(IRQ_IPU_ERROR) | CSP_IRQMASK(IRQ_IPU_GENERAL);
    g_oalIrqTranslate[IRQ_IPU_ERROR] = IRQ_IPU_GENERAL;

    // Slam CCM DVFS interrupt to CCM DPTC interrupt
    g_oalIrqMask[IRQ_CCM].QuadPart =
        CSP_IRQMASK(IRQ_CCM) | CSP_IRQMASK(IRQ_DVFS);
    g_oalIrqTranslate[IRQ_DVFS] = IRQ_CCM;


#ifdef OAL_BSP_CALLBACKS
    // Give BSP chance to initialize subordinate controller
    rc = BSPIntrInit();
#else
    rc = TRUE;
#endif

    // Enable the SDMA IRQ
    irq = IRQ_SDMA;
    if (!OALIntrEnableIrqs(1, &irq))
    {
        OALMSG(OAL_WARN,
            (L"OALIntrInit: can't enable IRQ_SDMA\r\n"));
    }

    // Enable GPIO1 IRQ
    irq = IRQ_GPIO1;
    if (!OALIntrEnableIrqs(1, &irq))
    {
        OALMSG(OAL_WARN,
            (L"OALIntrInit: can't enable IRQ_GPIO1\r\n"));
    }

    // Enable GPIO2 IRQ
    irq = IRQ_GPIO2;
    if (!OALIntrEnableIrqs(1, &irq))
    {
        OALMSG(OAL_WARN,
            (L"OALIntrInit: can't enable IRQ_GPIO2\r\n"));
    }

    // Enable GPIO3 IRQ
    irq = IRQ_GPIO3;
    if (!OALIntrEnableIrqs(1, &irq))
    {
        OALMSG(OAL_WARN,
            (L"OALIntrInit: can't enable IRQ_GPIO3\r\n"));
    }

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
        case CSP_BASE_REG_PA_I2C3:
            pIrqs[0] = IRQ_I2C3;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_I2C2:
            pIrqs[0] = IRQ_I2C2;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_MPEG4_ENCODER:
            pIrqs[0] = IRQ_MPEG4_ENCODE;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_RTIC:
            pIrqs[0] = IRQ_RTIC;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_FIRI:
            pIrqs[0] = IRQ_FIRI;
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

        case CSP_BASE_REG_PA_I2C:
            pIrqs[0] = IRQ_I2C;
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

        case CSP_BASE_REG_PA_ATA_CTRL:
            pIrqs[0] = IRQ_ATA_CTRL;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_MEM_PA_GACC:
            pIrqs[0] = IRQ_GACC;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_CSPI3:
            pIrqs[0] = IRQ_CSPI3;
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

        case CSP_BASE_REG_PA_SIM:
            pIrqs[0] = IRQ_SIM_COMMON;  // Slammed SIM IRQ
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_RNGA:
            pIrqs[0] = IRQ_RNGA;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_EVTMON:
            pIrqs[0] = IRQ_EVTMON;
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

        case CSP_BASE_REG_PA_GPT:
            pIrqs[0] = IRQ_GPT;
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

        case CSP_BASE_REG_PA_USBOTG:
            // Check if output array can hold all IRQs
            if (*pCount >= 3)
            {
                pIrqs[0] = IRQ_USB_HOST1;
                pIrqs[1] = IRQ_USB_HOST2;
                pIrqs[2] = IRQ_USB_OTG;
                *pCount = 3;
                rc = TRUE;
            }
            break;

        case CSP_BASE_REG_PA_MSHC1:
            pIrqs[0] = IRQ_MSHC1;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_MSHC2:
            pIrqs[0] = IRQ_MSHC2;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_UART1:
            pIrqs[0] = IRQ_UART1;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_IPU:
            pIrqs[0] = IRQ_IPU_GENERAL;   // Slammed IPU IRQ
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_UART4:
            pIrqs[0] = IRQ_UART4;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_UART5:
            pIrqs[0] = IRQ_UART5;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_ECT:
            pIrqs[0] = IRQ_ECT;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_SCC:
            // Check if output array can hold all IRQs
            if (*pCount >= 2)
            {
                pIrqs[0] = IRQ_SCC_SCM;
                pIrqs[1] = IRQ_SCC_SMN;
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

        case CSP_BASE_REG_PA_CCM:
            pIrqs[0] = IRQ_CCM;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_PCMCIA:
            pIrqs[0] = IRQ_PCMCIA;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_WDOG:
            pIrqs[0] = IRQ_WDOG;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_GPIO3:
            pIrqs[0] = IRQ_GPIO3;
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

    for (i = 0; i < count; i++) {
#ifndef OAL_BSP_CALLBACKS
        irq = pIrqs[i];
#else
        // Give BSP chance to enable irq on subordinate interrupt controller
        irq = BSPIntrEnableIrq(pIrqs[i]);
#endif
        if (irq == OAL_INTR_IRQ_UNDEFINED) continue;

        // If IRQ is valid
        if (irq < OAL_INTR_IRQ_MAXIMUM)
        {
            // Enable the IRQ using entries in translation tables
            OAL_IRQ_ENABLE(irq);

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

        OAL_IRQ_DISABLE(irq);
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
        OAL_IRQ_ENABLE(irq);
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
    else if (g_pProfilerISR && (irq == IRQ_GPT))
    {
        // Call profiling interupt handler
        sysIntr = g_pProfilerISR(ra);
    }

    // Else not system timer interrupt
    else
    {
        if (g_oalILT.active) {
            g_oalILT.isrTime1 = OALTimerCountsSinceSysTick();
            g_oalILT.savedPC = 0;
            g_oalILT.interrupts++;
        }

        // GPIO1 special case:  Interrupts from module are ORed together
        // so we cannot disable the IRQ_GPIO1 since this would block other
        // drivers relying on GPIO interrupts.  Instead we clear the interrupt
        // at the GPIO and then disable all interrupts associated with the
        // SYSINTR that owns this GPIO interrupt, including the GPIO line
        // that caused the interrupt to occur.
        if (irq == IRQ_GPIO1)
        {
            // detect GPIO line that is asserting interrupt
            line = _CountLeadingZeros(INREG32(&g_pGPIO1->ISR)
                                      & INREG32(&g_pGPIO1->IMR));

            // If at least one GPIO interrupt line is asserted
            if (line < 32)
            {
                irq = g_oalGpioTranslate[0][31 - line];
            }
            else
            {
                irq = (UINT32) OAL_INTR_IRQ_UNDEFINED;
            }

        } // GPIO1 special case

        // GPIO2 special case:  Interrupts from module are ORed together
        // so we cannot disable the IRQ_GPIO2 since this would block other
        // drivers relying on GPIO interrupts.  Instead we clear the interrupt
        // at the GPIO and then disable all interrupts associated with the
        // SYSINTR that owns this GPIO interrupt, including the GPIO line
        // that caused the interrupt to occur.
        else if (irq == IRQ_GPIO2)
        {
            // detect GPIO line that is asserting interrupt
            line = _CountLeadingZeros(INREG32(&g_pGPIO2->ISR)
                                      & INREG32(&g_pGPIO2->IMR));

            // If at least one GPIO interrupt line is asserted
            if (line < 32)
            {
                irq = g_oalGpioTranslate[1][31 - line];
            }
            else
            {
                irq = (UINT32) OAL_INTR_IRQ_UNDEFINED;
            }

        } // GPIO2 special case

        // GPIO3 special case:  Interrupts from module are ORed together
        // so we cannot disable the IRQ_GPIO3 since this would block other
        // drivers relying on GPIO interrupts.  Instead we clear the interrupt
        // at the GPIO and then disable all interrupts associated with the
        // SYSINTR that owns this GPIO interrupt, including the GPIO line
        // that caused the interrupt to occur.
        else if (irq == IRQ_GPIO3)
        {
            // detect GPIO line that is asserting interrupt
            line = _CountLeadingZeros(INREG32(&g_pGPIO3->ISR)
                                      & INREG32(&g_pGPIO3->IMR));

            // If at least one GPIO interrupt line is asserted
            if (line < 32)
            {
                irq = g_oalGpioTranslate[2][31 - line];
            }
            else
            {
                irq = (UINT32) OAL_INTR_IRQ_UNDEFINED;
            }

        } // GPIO3 special case

        // SDMA special case:  Interrupts from SDMA are ORed together
        // so we cannot disable the SDMA_IRQ since this would block other
        // drivers relying on SDMA interrupts.  Instead we just clear the
        // interrupt at the SDMA
        else if (irq == IRQ_SDMA)
        {
            // Detect SDMA channel that is asserting interrupt
            line = _CountLeadingZeros(INREG32(&g_pSDMA->INTR));

            // If at least one SDMA channel interrupt is asserted
            if (line < 32)
            {
                line = 31 - line;

                // Clear the pending interrupt at the SDMA (ISR is w1c)
                OUTREG32(&g_pSDMA->INTR, 1 << line);

                irq = g_oalSdmaTranslate[line];
            }
            else
            {
                irq = (UINT32) OAL_INTR_IRQ_UNDEFINED;
            }

        } // SDMA special case

        // Normal case:  An interrupt other than the special cases handled above
        // has occurred.  We use our tables to determine the corresponding
        // SYSINTR and disable all associated interrupt sources.
        else
        {
            irq = g_oalIrqTranslate[irq];
        } // normal case

#ifdef OAL_BSP_CALLBACKS
        // Give BSP chance to translate IRQ -- if there is subordinate
        // interrupt controller in the BSP give it a chance to decode
        // its status and change the IRQ
        irq = BSPIntrActiveIrq(irq);
#endif

        // if IRQ assigned to this GPIO is defined
        if (irq != OAL_INTR_IRQ_UNDEFINED)
        {
            OAL_IRQ_DISABLE(irq);

            // First find if IRQ is claimed by chain
            sysIntr = NKCallIntChain((UCHAR)irq);
            if (sysIntr == SYSINTR_CHAIN || !NKIsSysIntrValid(sysIntr))
            {
                // IRQ wasn't claimed, use static mapping
                sysIntr = OALIntrTranslateIrq(irq);
            }
        }
        else
        {
            OALMSGS(OAL_ERROR,
                (TEXT("OEMInterruptHandler:  undefined IRQ (%d)!\r\n"),
                EXTREG32(&g_pAVIC->NIVECSR, CSP_BITFMASK(AVIC_NIVECSR_NIVECTOR),
                AVIC_NIVECSR_NIVECTOR_LSH)));
        }
    } // Else not system timer interrupt

    return sysIntr;
}

//------------------------------------------------------------------------------
