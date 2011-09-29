//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File: intr.c
//
//  PQOAL interrupt support.
//
//-----------------------------------------------------------------------------

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


//-----------------------------------------------------------------------------
// External Functions


//-----------------------------------------------------------------------------
// External Variables


//-----------------------------------------------------------------------------
// Defines


// Types


//-----------------------------------------------------------------------------
// Global Variables
PCSP_TZIC_REGS g_pTZIC;
PCSP_GPIO_REGS g_pGPIO1;
PCSP_GPIO_REGS g_pGPIO2;
PCSP_GPIO_REGS g_pGPIO3;
PCSP_GPIO_REGS g_pGPIO4;
PCSP_SDMA_REGS g_pSDMA;

const UINT32 g_IRQ_RTC = IRQ_SRTC_TZ;   // Create a global variable for the
                                        // SOC-specific IRQ that is used by
                                        // the RTC.

//  Function pointer to profiling timer ISR routine.
PFN_PROFILER_ISR g_pProfilerISR = NULL;


//-----------------------------------------------------------------------------
// Local Variables


// Local Functions


//-----------------------------------------------------------------------------
//
//  Function:  OALIntrInit
//
//  This function initialize interrupt mapping, hardware and call platform
//  specific initialization.
//
BOOL OALIntrInit()
{
    BOOL rc = FALSE;

    OALMSG( OAL_FUNC&&OAL_INTR, (L"+OALInterruptInit\r\n") );

    // Initialize interrupt mapping
    OALIntrMapInit();

    // Get uncached virtual addresses for TZIC.  TO1/TO2 have different
    // base addresses for TZIC.  Only one will be mapped in g_oalAddressTable,
    // so try both before bailing out.
    g_pTZIC = (PCSP_TZIC_REGS) OALPAtoUA(CSP_BASE_REG_PA_TZIC_TO2);
    if (g_pTZIC == NULL)
    {
        g_pTZIC = (PCSP_TZIC_REGS) OALPAtoUA(CSP_BASE_REG_PA_TZIC);
        if (g_pTZIC == NULL)
        {
            OALMSG(OAL_ERROR, (L"OALIntrInit:  TZIC null pointer!\r\n"));
            goto cleanUp;
        }
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

    g_pGPIO4 = (PCSP_GPIO_REGS) OALPAtoUA(CSP_BASE_REG_PA_GPIO4);
    if (g_pGPIO4 == NULL)
    {
        OALMSG(OAL_ERROR, (L"OALIntrInit:  GPIO4 null pointer!\r\n"));
        goto cleanUp;
    }

    // Get uncached virtual addresses for SDMA
    g_pSDMA = (PCSP_SDMA_REGS) OALPAtoUA(CSP_BASE_REG_PA_SDMA);
    if (g_pSDMA == NULL)
    {
        OALMSG(OAL_ERROR, (L"OALIntrInit:  SDMA null pointer!\r\n"));
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


//-----------------------------------------------------------------------------
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
        case CSP_BASE_REG_PA_GPU:
            if (*pCount >= 2)
            {
                *pIrqs = IRQ_GPU;
                *(pIrqs + 1) = IRQ_GPU_IDLE;
                *pCount = 2;
                rc = TRUE;
            }
            break;

        case CSP_BASE_REG_PA_IPU:
            if (*pCount >= 2)
            {
                *pIrqs = IRQ_IPU_SYNC;
                *(pIrqs + 1) = IRQ_IPU_ERROR;
                *pCount = 2;
                rc = TRUE;
            }
            break;

        case CSP_BASE_REG_PA_ESDHC1:
            *pIrqs = IRQ_ESDHC1;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_ESDHC2:
            *pIrqs = IRQ_ESDHC2;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_UART3:
            *pIrqs = IRQ_UART3;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_ECSPI2:
            *pIrqs = IRQ_ECSPI2;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_SSI2:
            *pIrqs = IRQ_SSI2;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_ESDHC3:
            *pIrqs = IRQ_ESDHC3;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_ESDHC4:
            *pIrqs = IRQ_ESDHC4;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_SPDIF:
            *pIrqs = IRQ_SPDIF;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_PATA_UDMA:
            *pIrqs = IRQ_PATA;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_SLIMBUS:
            if (*pCount >= 3)
            {
                *pIrqs = IRQ_SLIMBUS_INT;
                *(pIrqs + 1) = IRQ_SLIMBUS_EXCEP;
                *(pIrqs + 2) = IRQ_SLIMBUS_RX;
                *pCount = 3;
                rc = TRUE;
            }
            break;

        case CSP_BASE_REG_PA_HSI2C:
            *pIrqs = IRQ_HSI2C;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_USB:
            if (*pCount >= 4)
            {
                *pIrqs = IRQ_USB_HOST1;
                *(pIrqs + 1) = IRQ_USB_HOST2;
                *(pIrqs + 2) = IRQ_USB_HOST3;
                *(pIrqs + 3) = IRQ_USB_OTG;
                *pCount = 4;
                rc = TRUE;
            }
            break;

        case CSP_BASE_REG_PA_GPIO1:
            if (*pCount >= 2)
            {
                *pIrqs = IRQ_GPIO1_LOWER16;
                *(pIrqs + 1) = IRQ_GPIO1_UPPER16;
                *pCount = 2;
                rc = TRUE;
            }
            break;

        case CSP_BASE_REG_PA_GPIO2:
            if (*pCount >= 2)
            {
                *pIrqs = IRQ_GPIO2_LOWER16;
                *(pIrqs + 1) = IRQ_GPIO2_UPPER16;
                *pCount = 2;
                rc = TRUE;
            }
            break;

        case CSP_BASE_REG_PA_GPIO3:
            if (*pCount >= 2)
            {
                *pIrqs = IRQ_GPIO3_LOWER16;
                *(pIrqs + 1) = IRQ_GPIO3_UPPER16;
                *pCount = 2;
                rc = TRUE;
            }
            break;

        case CSP_BASE_REG_PA_GPIO4:
            if (*pCount >= 2)
            {
                *pIrqs = IRQ_GPIO4_LOWER16;
                *(pIrqs + 1) = IRQ_GPIO4_UPPER16;
                *pCount = 2;
                rc = TRUE;
            }
            break;

        case CSP_BASE_REG_PA_KPP:
            *pIrqs = IRQ_KPP;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_WDOG1:
            *pIrqs = IRQ_WDOG1;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_WDOG2:
            *pIrqs = IRQ_WDOG2;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_GPT:
            *pIrqs = IRQ_GPT;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_SRTC:
            // Check if output array can hold all IRQs
            if (*pCount >= 2)
            {
                *pIrqs = IRQ_SRTC_NTZ;
                *(pIrqs + 1) = IRQ_SRTC_TZ;
                *pCount = 2;
                rc = TRUE;
            }
            break;

        case CSP_BASE_REG_PA_IOMUXC:
            *pIrqs = IRQ_IOMUX;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_EPIT1:
            *pIrqs = IRQ_EPIT1;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_EPIT2:
            *pIrqs = IRQ_EPIT2;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_PWM1:
            *pIrqs = IRQ_PWM1;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_PWM2:
            *pIrqs = IRQ_PWM2;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_UART1:
            *pIrqs = IRQ_UART1;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_UART2:
            *pIrqs = IRQ_UART2;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_SRC:
            *pIrqs = IRQ_SRC;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_CCM:
            // Check if output array can hold all IRQs
            if (*pCount >= 2)
            {
                *pIrqs = IRQ_CCM1;
                *(pIrqs + 1) = IRQ_CCM2;
                *pCount = 2;
                rc = TRUE;
            }
            break;

        case CSP_BASE_REG_PA_GPC:
            // Check if output array can hold all IRQs
            if (*pCount >= 2)
            {
                *pIrqs = IRQ_GPC1;
                *(pIrqs + 1) = IRQ_GPC2;
                *pCount = 2;
                rc = TRUE;
            }
            break;

        case CSP_BASE_REG_PA_IIM:
            *pIrqs = IRQ_IIM;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_CSU:
            *pIrqs = IRQ_CSU;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_OWIRE:
            *pIrqs = IRQ_OWIRE;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_FIRI:
            *pIrqs = IRQ_FIRI;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_ECSPI1:
            *pIrqs = IRQ_ECSPI1;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_SDMA:
            *pIrqs = IRQ_SDMA;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_SCC:
            // Check if output array can hold all IRQs
            if (*pCount >= 3)
            {
                *pIrqs = IRQ_SCC_HIGH;
                *(pIrqs + 1) = IRQ_SCC_TZ;
                *(pIrqs + 2) = IRQ_SCC_NTZ;
                *pCount = 3;
                rc = TRUE;
            }
            break;

        case CSP_BASE_REG_PA_RTIC:
            *pIrqs = IRQ_RTIC;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_CSPI:
            *pIrqs = IRQ_CSPI;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_I2C2:
            *pIrqs = IRQ_I2C2;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_I2C1:
            *pIrqs = IRQ_I2C1;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_SSI1:
            *pIrqs = IRQ_SSI1;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_EMI:
            *pIrqs = IRQ_EMI;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_MIPI:
            // Check if output array can hold all IRQs
            if (*pCount >= 3)
            {
                *pIrqs = IRQ_MIPI_ERROR;
                *(pIrqs + 1) = IRQ_MIPI_TIMER;
                *(pIrqs + 2) = IRQ_MIPI_FUNC;
                *pCount = 3;
                rc = TRUE;
            }
            break;
            
        case CSP_BASE_REG_PA_PATA_PIO:
            *pIrqs = IRQ_PATA;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_SIM:
            // Check if output array can hold all IRQs
            if (*pCount >= 2)
            {
                *pIrqs = IRQ_SIM_IPB;
                *(pIrqs + 1) = IRQ_SIM_DATA;
                *pCount = 2;
                rc = TRUE;
            }
            break;

        case CSP_BASE_REG_PA_SSI3:
            *pIrqs = IRQ_SSI3;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_FEC:
            *pIrqs = IRQ_FEC;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_TVE:
            *pIrqs = IRQ_TVE;
            *pCount = 1;
            rc = TRUE;
            break;

        case CSP_BASE_REG_PA_VPU:
            // Check if output array can hold all IRQs
            if (*pCount >= 2)
            {
                *pIrqs = IRQ_VPU;
                *(pIrqs + 1) = IRQ_VPU_IDLE;
                *pCount = 2;
                rc = TRUE;
            }
            break;

        case CSP_BASE_REG_PA_SAHARA:
            // Check if output array can hold all IRQs
            if (*pCount >= 2)
            {
                *pIrqs = IRQ_SAHARAH_TZ;
                *(pIrqs + 1) = IRQ_SAHARAH_NTZ;
                *pCount = 2;
                rc = TRUE;
            }
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


//-----------------------------------------------------------------------------
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
            // Primary IRQ in TZIC
            OUTREG32(&g_pTZIC->ENSET[0], 1U << irq);
            break;

        case 1:
            // Primary IRQ in TZIC
            OUTREG32(&g_pTZIC->ENSET[1], 1U << (irq - 32));
            break;

        case 2:
            // Primary IRQ in TZIC
            OUTREG32(&g_pTZIC->ENSET[2], 1U << (irq - 64));
            break;

        case 3:
            // Primary IRQ in TZIC
            OUTREG32(&g_pTZIC->ENSET[3], 1U << (irq - 96));
            break;

        case 4:
            // Secondary IRQ in GPIO1
            SETREG32(&g_pGPIO1->IMR, 1U << (irq - IRQ_GPIO1_PIN0));
            // Primary IRQ in TZIC
            if (irq < IRQ_GPIO1_PIN16)
            {
                OUTREG32(&g_pTZIC->ENSET[1], 1U << (IRQ_GPIO1_LOWER16 - 32));
            }
            else
            {
                OUTREG32(&g_pTZIC->ENSET[1], 1U << (IRQ_GPIO1_UPPER16 - 32));
            }
            break;

        case 5:
            // Secondary IRQ in GPIO2
            SETREG32(&g_pGPIO2->IMR, 1U << (irq - IRQ_GPIO2_PIN0));
            // Primary IRQ in TZIC
            if (irq < IRQ_GPIO2_PIN16)
            {
                OUTREG32(&g_pTZIC->ENSET[1], 1U << (IRQ_GPIO2_LOWER16 - 32));
            }
            else
            {
                OUTREG32(&g_pTZIC->ENSET[1], 1U << (IRQ_GPIO2_UPPER16 - 32));
            }
            break;

        case 6:
            // Secondary IRQ in GPIO3
            SETREG32(&g_pGPIO3->IMR, 1U << (irq - IRQ_GPIO3_PIN0));
            // Primary IRQ in TZIC
            if (irq < IRQ_GPIO3_PIN16)
            {
                OUTREG32(&g_pTZIC->ENSET[1], 1U << (IRQ_GPIO3_LOWER16 - 32));
            }
            else
            {
                OUTREG32(&g_pTZIC->ENSET[1], 1U << (IRQ_GPIO3_UPPER16 - 32));
            }
            break;

        case 7:
            // Secondary IRQ in GPIO4
            SETREG32(&g_pGPIO4->IMR, 1U << (irq - IRQ_GPIO4_PIN0));
            // Primary IRQ in TZIC
            if (irq < IRQ_GPIO4_PIN16)
            {
                OUTREG32(&g_pTZIC->ENSET[1], 1U << (IRQ_GPIO4_LOWER16 - 32));
            }
            else
            {
                OUTREG32(&g_pTZIC->ENSET[1], 1U << (IRQ_GPIO4_UPPER16 - 32));
            }
            break;

        case 8:
            // Secondary IRQ in SDMA (no secondary mask)
            
            // Primary IRQ in TZIC
            OUTREG32(&g_pTZIC->ENSET[0], 1U << IRQ_SDMA);            
            break;

        default:
            // Invalid IRQ
            rc = FALSE;
        }
    }

    OALMSG(OAL_INTR&&OAL_FUNC, (L"-OALIntrEnableIrqs(rc = %d)\r\n", rc));
    return rc;
}


//-----------------------------------------------------------------------------
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
            // Primary IRQ in TZIC
            OUTREG32(&g_pTZIC->ENCLEAR[0], 1U << irq);
            break;

        case 1:
            // Primary IRQ in TZIC
            OUTREG32(&g_pTZIC->ENCLEAR[1], 1U << (irq - 32));
            break;

        case 2:
            // Primary IRQ in TZIC
            OUTREG32(&g_pTZIC->ENCLEAR[2], 1U << (irq - 64));
            break;

        case 3:
            // Primary IRQ in TZIC
            OUTREG32(&g_pTZIC->ENCLEAR[3], 1U << (irq - 96));
            break;

        case 4:
            // Secondary IRQ in GPIO1
            CLRREG32(&g_pGPIO1->IMR, 1U << (irq - IRQ_GPIO1_PIN0));
            break;

        case 5:
            // Secondary IRQ in GPIO2
            CLRREG32(&g_pGPIO2->IMR, 1U << (irq - IRQ_GPIO2_PIN0));
            break;

        case 6:
            // Secondary IRQ in GPIO3
            CLRREG32(&g_pGPIO3->IMR, 1U << (irq - IRQ_GPIO3_PIN0));
            break;

        case 7:
            // Secondary IRQ in GPIO4
            CLRREG32(&g_pGPIO4->IMR, 1U << (irq - IRQ_GPIO4_PIN0));
            break;

        case 8:
            // Secondary IRQ in SDMA (no secondary mask)
            break;
        }
    }

    OALMSG(OAL_INTR&&OAL_FUNC, (L"-OALIntrDisableIrqs\r\n"));
}


//-----------------------------------------------------------------------------
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

        // Upper OAL layer is not disabling interrupts, but we
        // need interrupts disabled for safe access to GPIO and
        // TZIC registers
        bEnable = INTERRUPTS_ENABLE(FALSE);

        // Enable IRQ
        switch (irq >> 5)
        {
        case 0:
            // Primary IRQ in TZIC
            OUTREG32(&g_pTZIC->ENSET[0], 1U << irq);
            break;

        case 1:
            // Primary IRQ in TZIC
            OUTREG32(&g_pTZIC->ENSET[1], 1U << (irq - 32));
            break;

        case 2:
            // Primary IRQ in TZIC
            OUTREG32(&g_pTZIC->ENSET[2], 1U << (irq - 64));
            break;

        case 3:
            // Primary IRQ in TZIC
            OUTREG32(&g_pTZIC->ENSET[3], 1U << (irq - 96));
            break;

        case 4:
            // Secondary IRQ in GPIO1
            SETREG32(&g_pGPIO1->IMR, 1U << (irq - IRQ_GPIO1_PIN0));
            break;

        case 5:
            // Secondary IRQ in GPIO2
            SETREG32(&g_pGPIO2->IMR, 1U << (irq - IRQ_GPIO2_PIN0));
            break;

        case 6:
            // Secondary IRQ in GPIO3
            SETREG32(&g_pGPIO3->IMR, 1U << (irq - IRQ_GPIO3_PIN0));
            break;

        case 7:
            // Secondary IRQ in GPIO4
            SETREG32(&g_pGPIO4->IMR, 1U << (irq - IRQ_GPIO4_PIN0));
            break;

        case 8:
            // Secondary IRQ in SDMA (no secondary mask)
            break;
        }

        // Enable interrupt
        INTERRUPTS_ENABLE(bEnable);
    }

    OALMSG(OAL_INTR&&OAL_VERBOSE, (L"-OALIntrDoneIrqs\r\n"));
}


//-----------------------------------------------------------------------------
//
//  Function:  OEMInterruptHandler
//
ULONG OEMInterruptHandler(ULONG ra)
{
    UINT32 sysIntr = SYSINTR_NOP;
    UINT32 irq = TZIC_IRQ_SOURCES_MAX;
    UINT32 line;
    UINT32 irqSet;

    for (irqSet = 0; irqSet < TZIC_IRQ_SET_MAX; irqSet++)
    {
        line = _CountLeadingZeros(INREG32(&g_pTZIC->HIPND[irqSet]));
        if (line < 32)
        {
            irq = (irqSet << 5) + (31 - line);
            break;
        }
    }
        
    // Make sure interrupt is pending
    if (irq >= TZIC_IRQ_SOURCES_MAX)
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
#ifdef OAL_ILTIMING
        if (g_oalILT.active) {
            g_oalILT.interrupts++;
        }        
#endif

        // GPIO1 special case:  Interrupts from module are ORed together
        // so we cannot disable the IRQ_GPIO1 since this would block other
        // drivers relying on GPIO interrupts.  Instead we clear the interrupt
        // at the GPIO and then disable all interrupts associated with the
        // SYSINTR that owns this GPIO interrupt, including the GPIO line
        // that caused the interrupt to occur.
        if ((irq == IRQ_GPIO1_UPPER16) || (irq == IRQ_GPIO1_LOWER16))
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

        } // GPIO1 special case

        // GPIO2 special case:  Interrupts from module are ORed together
        // so we cannot disable the IRQ_GPIO2 since this would block other 
        // drivers relying on GPIO interrupts.  Instead we clear the interrupt
        // at the GPIO and then disable all interrupts associated with the
        // SYSINTR that owns this GPIO interrupt, including the GPIO line
        // that caused the interrupt to occur.
        else if ((irq == IRQ_GPIO2_UPPER16) || (irq == IRQ_GPIO2_LOWER16))
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

        } // GPIO2 special case

        // GPIO3 special case:  Interrupts from module are ORed together
        // so we cannot disable the IRQ_GPIO3 since this would block other
        // drivers relying on GPIO interrupts.  Instead we clear the interrupt
        // at the GPIO and then disable all interrupts associated with the
        // SYSINTR that owns this GPIO interrupt, including the GPIO line
        // that caused the interrupt to occur.
        else if ((irq == IRQ_GPIO3_UPPER16) || (irq == IRQ_GPIO3_LOWER16))
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

        } // GPIO3 special case

        // GPIO4 special case:  Interrupts from module are ORed together
        // so we cannot disable the IRQ_GPIO4 since this would block other
        // drivers relying on GPIO interrupts.  Instead we clear the interrupt
        // at the GPIO and then disable all interrupts associated with the
        // SYSINTR that owns this GPIO interrupt, including the GPIO line
        // that caused the interrupt to occur.
        else if ((irq == IRQ_GPIO4_UPPER16) || (irq == IRQ_GPIO4_LOWER16))
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

        } // GPIO4 special case

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

                // Translate it to the secondary IRQ (No secondary mask)
                irq = IRQ_SDMA_CH0 + line;
            }
            else
            {
                irq = (UINT32) OAL_INTR_IRQ_UNDEFINED;
            }

        } // SDMA special case

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
            OALMSGS(OAL_ERROR,
                (TEXT("OEMInterruptHandler:  undefined IRQ!\r\n")));
        }
    } // Else not system timer interrupt

    return sysIntr;
}
