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
// Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  intr.h
//
//  This file contains board-specific interrupt code.
//
//-----------------------------------------------------------------------------
#include <cmnintrin.h>
#include "bsp.h"

#define GPIO_ICR2_MASK(x)       GPIO_ICR_MASK((x)-16)
#define GPIO_ICR2_VAL(x,y)      GPIO_ICR_VAL(x,(y)-16)

//------------------------------------------------------------------------------
// External Variables
extern PCSP_GPIO_REGS g_pGPIO1;
extern PCSP_IOMUX_REGS g_pIOMUX;

//------------------------------------------------------------------------------
//
//  Function:  BSPIntrInit
//
BOOL BSPIntrInit()
{
	//
	// CS&ZHL JUN-2-2011: for iMX257PDK only
	//
#ifdef		IMX257PDK_CPLD			
    // Mask all CPLD interrupts
    CPLDWrite16(CPLD_INT_MASK_OFFSET, 0xFFFF);

    // Clear all CPLD interrupt status bits
    CPLDWrite16(CPLD_INT_RESET_OFFSET, 0xFFFF);
    CPLDWrite16(CPLD_INT_RESET_OFFSET, 0x0);

    // Configure CPLD interrupt line
    OAL_IOMUX_SET_MUX(g_pIOMUX, DDK_IOMUX_PIN_PWM, DDK_IOMUX_PIN_MUXMODE_ALT5, DDK_IOMUX_PIN_SION_REGULAR);
    
    INSREG32(&g_pGPIO1->GDIR, GPIO_PIN_MASK(CPLD_IRQ_GPIO_PIN), 
             GPIO_PIN_VAL(GPIO_GDIR_INPUT, CPLD_IRQ_GPIO_PIN));
    INSREG32(&g_pGPIO1->EDGE_SEL, GPIO_PIN_MASK(CPLD_IRQ_GPIO_PIN), 
             GPIO_PIN_VAL(GPIO_EDGE_SEL_DISABLE, CPLD_IRQ_GPIO_PIN));

    INSREG32(&g_pGPIO1->ICR2, GPIO_ICR2_MASK(CPLD_IRQ_GPIO_PIN), 
             GPIO_ICR2_VAL(GPIO_ICR_LOW_LEVEL, CPLD_IRQ_GPIO_PIN));
    INSREG32(&g_pGPIO1->IMR, GPIO_PIN_MASK(CPLD_IRQ_GPIO_PIN), 
             GPIO_PIN_VAL(GPIO_IMR_UNMASKED, CPLD_IRQ_GPIO_PIN));


    // Unmask Ethernet CPLD interrupt
    CPLDWrite16(CPLD_INT_MASK_OFFSET, ~(CSP_BITFMASK(CPLD_INT_MASK_ETHER)));    
#endif		//IMX257PDK_CPLD

    return TRUE;
}


//------------------------------------------------------------------------------

BOOL BSPIntrRequestIrqs(DEVICE_LOCATION *pDevLoc, UINT32 *pCount, UINT32 *pIrqs)
{
    BOOL rc = FALSE;

    OALMSGS(OAL_INTR&&OAL_FUNC, (
        L"+BSPIntrRequestIrq(0x%08x->%d/%d/0x%08x/%d, 0x%08x, 0x%08x)\r\n",
        pDevLoc, pDevLoc->IfcType, pDevLoc->BusNumber, pDevLoc->LogicalLoc,
        pDevLoc->Pin, pCount, pIrqs
    ));

    if (pIrqs == NULL || pCount == NULL || *pCount < 1) goto cleanUp;

    switch (pDevLoc->IfcType)
    {
    case Internal:
        switch ((ULONG)pDevLoc->LogicalLoc)
        {
        case BSP_BASE_REG_PA_LAN911x_IOBASE:
            pIrqs[0] = IRQ_CPLD_ETHER;
            *pCount = 1;
            rc = TRUE;
            break;
        }
        break;
    }

cleanUp:
    OALMSGS(OAL_INTR&&OAL_FUNC, (L"-BSPIntrRequestIrq(rc = %d)\r\n", rc));
    return rc;
}


//------------------------------------------------------------------------------
//
//  Function:  BSPIntrEnableIrq
//
//  This function is called from OALIntrEnableIrq to enable interrupt on
//  board-level interrupt controller.
//
UINT32 BSPIntrEnableIrq(UINT32 irq)
{    
    OALMSGS(OAL_INTR&&OAL_VERBOSE, (L"+BSPIntrEnableIrq(%d)\r\n", irq));

    // Check if it is a valid board-level interrupt
    if ((irq >= IRQ_CPLD_MIN) && (irq <= IRQ_CPLD_MAX))
    {
        // Return SoC level IRQ line
        irq = CPLD_IRQ_GPIO_LINE;
    }

    OALMSGS(OAL_INTR&&OAL_VERBOSE, (L"-BSPIntrEnableIrq(irq = %d)\r\n", irq));
    return irq;
}


//------------------------------------------------------------------------------
//
//  Function:  BSPIntrDisableIrq
//
//  This function is called from OALIntrDisableIrq to disable interrupt on
//  board-level interrupt controller.
//
UINT32 BSPIntrDisableIrq(UINT32 irq)
{
    OALMSGS(OAL_INTR&&OAL_VERBOSE, (L"+BSPIntrDisableIrq(%d)\r\n", irq));

    // Check if it is a valid board-level interrupt
    if ((irq >= IRQ_CPLD_MIN) && (irq <= IRQ_CPLD_MAX))
    {
        // Return SoC level IRQ line
        irq = CPLD_IRQ_GPIO_LINE;
    }

    OALMSGS(OAL_INTR&&OAL_VERBOSE, (L"-BSPIntrDisableIrq(irq = %d)\r\n", irq));

    return irq;
}


//------------------------------------------------------------------------------
//
//  Function:  BSPIntrDoneIrq
//
//  This function is called from OALIntrDoneIrq to finish interrupt on
//  board-level interrupt controller.
//
UINT32 BSPIntrDoneIrq(UINT32 irq)
{
    OALMSGS(OAL_INTR&&OAL_VERBOSE, (L"+BSPIntrDoneIrq(%d)\r\n", irq));

    // Check if it is a valid board-level interrupt
    if ((irq >= IRQ_CPLD_MIN) && (irq <= IRQ_CPLD_MAX))
    {
        // Return SoC level IRQ line
        irq = CPLD_IRQ_GPIO_LINE;        
    }

    OALMSGS(OAL_INTR&&OAL_VERBOSE, (L"-BSPIntrDoneIrq(irq = %d)\r\n", irq));
    return irq;
}


//------------------------------------------------------------------------------
//
//  Function:  BSPIntrActiveIrq
//
//  This function is called from interrupt handler to give BSP chance to
//  translate IRQ in case of board-level interrupt controller.
//
UINT32 BSPIntrActiveIrq(UINT32 irq)
{
    OALMSGS(OAL_INTR&&OAL_VERBOSE, (L"+BSPIntrActiveIrq(%d)\r\n", irq));

    // Check if it is a valid board-level interrupt
    if (irq == CPLD_IRQ_GPIO_LINE)
    {
        // Return SoC level IRQ line
        irq = IRQ_CPLD_ETHER;
    }
    return irq;
}
