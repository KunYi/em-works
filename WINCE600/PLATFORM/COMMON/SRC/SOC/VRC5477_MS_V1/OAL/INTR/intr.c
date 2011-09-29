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
//  File: intr.c
//
//  This file implements interrupt module for VRC5477 North Bridge.
//
#include <windows.h>
#include <ceddk.h>
#include <nkintr.h>
#include <oal.h>
#include <oal_intr_mips.h>
#include <vrc5477.h>

//------------------------------------------------------------------------------
//
//  External:  g_oalLastSysIntr
//
//  This global variable should be set by interrupt handler to last SYSINTR
//  when interrupt occurs. It is used by fake (busy loop) idle/power off OAL 
//  implementations.
//
#ifdef OAL_FAKE_IDLE
extern volatile UINT32 g_oalLastSysIntr;
#endif


//------------------------------------------------------------------------------
//
//  Global:  g_pVRC5477Regs
//
static VRC5477_REGS *g_pVRC5477Regs;


//------------------------------------------------------------------------------
//
//  Function:  OALIntrInit
//
//  This function initialize Vr4131 interrupt hardware.
//
BOOL OALIntrInit()
{
    BOOL rc = FALSE;
    
    OALMSG(OAL_INTR&&OAL_FUNC, (L"+OALInterruptInit\r\n"));

    // Initialize interrupt mapping
    OALIntrMapInit();

    // Get and save uncached virtual addresses fir ICU and GIU
    g_pVRC5477Regs = OALPAtoUA(VRC5477_REG_PA);

    // Disable all interrupts and set routing to MIPS CPU interrupts
    OUTREG32(&g_pVRC5477Regs->INTCTRL0, VRC5477_INTCTRL0);
    OUTREG32(&g_pVRC5477Regs->INTCTRL1, VRC5477_INTCTRL1);
    OUTREG32(&g_pVRC5477Regs->INTCTRL2, VRC5477_INTCTRL2);
    OUTREG32(&g_pVRC5477Regs->INTCTRL3, VRC5477_INTCTRL3);

    // Set PCI bus level and edge
    OUTREG32(&g_pVRC5477Regs->INTPPES0, VRC5477_INTPPES0);
    OUTREG32(&g_pVRC5477Regs->INTPPES1, VRC5477_INTPPES1);
    
    // Clear all interrupts
    OUTREG32(&g_pVRC5477Regs->INTCLR32, 0xFFFFFFFF);
    
    // Hook interrupts 0 to 3
    if (!HookInterrupt(0, OALIntr0Handler)) {
        OALMSG(OAL_ERROR, (
            L"ERROR: OALIntrInit: HookInterrupt for MIPS interrupt 0 failed\r\n"
        ));
        goto cleanUp;
    }        
    if (!HookInterrupt(1, OALIntr1Handler)) {
        OALMSG(OAL_ERROR, (
            L"ERROR: OALIntrInit: HookInterrupt for MIPS interrupt 1 failed\r\n"
        ));
        goto cleanUp;
    }        
    if (!HookInterrupt(2, OALIntr2Handler)) {
        OALMSG(OAL_ERROR, (
            L"ERROR: OALIntrInit: HookInterrupt for MIPS interrupt 2 failed\r\n"
        ));
        goto cleanUp;
    }
    if (!HookInterrupt(3, OALIntr3Handler)) {
        OALMSG(OAL_ERROR, (
            L"ERROR: OALIntrInit: HookInterrupt for MIPS interrupt 3 failed\r\n"
        ));
        goto cleanUp;
    }

#ifdef OAL_BSP_CALLBACKS
    // Give BSP change to initialize subordinate controller
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
//  This function returns IRQ for CPU/SoC devices based on their
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

    // This shouldn't happen
    if (*pCount < 1) goto cleanUp;

#ifdef OAL_BSP_CALLBACKS
    rc = BSPIntrRequestIrqs(pDevLoc, pCount, pIrqs);
#endif    

cleanUp:        
    OALMSG(OAL_INTR&&OAL_FUNC, (L"-OALIntrRequestIrqs(rc = %d)\r\n", rc));
    return rc;
}


//------------------------------------------------------------------------------
//
//  Function:  OALIntrEnableIrq
//
BOOL OALIntrEnableIrqs(UINT32 count, const UINT32 *pIrqs)
{
    BOOL rc = TRUE;
    UINT32 irq, i;

    OALMSG(OAL_INTR&&OAL_FUNC, (
        L"+OALntrEnableIrqs(%d, 0x%08x)\r\n", count, pIrqs
    ));

    for (i = 0; i < count; i++) {
#ifndef OAL_BSP_CALLBACKS
        irq = pIrqs[i];
#else
        // Give BSP chance to enable irq on subordinate interrupt controller
        irq = BSPIntrEnableIrq(pIrqs[i]);
#endif
        if (irq == OAL_INTR_IRQ_UNDEFINED) continue;
        // Depending on IRQ number use one from four registers
        if (irq < 8) {
            // Use interrupt mask register 1
            SETREG32(&g_pVRC5477Regs->INTCTRL0, 8 << (irq << 2));
        } else if (irq < 16) {
            SETREG32(&g_pVRC5477Regs->INTCTRL1, 8 << ((irq - 8) << 2));
        } else if (irq < 24) {
            SETREG32(&g_pVRC5477Regs->INTCTRL2, 8 << ((irq - 16) << 2));
        } else if (irq < 32) {
            SETREG32(&g_pVRC5477Regs->INTCTRL3, 8 << ((irq - 24) << 2));
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
    UINT32 irq, i;

    OALMSG(OAL_INTR&&OAL_FUNC, (
        L"+OALIntrDisableIrqs(%d, 0x%08x -> %d)\r\n", count, pIrqs, *pIrqs
    ));

    for (i = 0; i < count; i++) {
#ifndef OAL_BSP_CALLBACKS
        irq = pIrqs[i];
#else
        // Give BSP chance to disable irq on subordinate interrupt controller
        irq = BSPIntrDisableIrq(pIrqs[i]);
#endif
        // Depending on IRQ number use one from four registers
        if (irq < 8) {
            // Use interrupt mask register 1
            CLRREG32(&g_pVRC5477Regs->INTCTRL0, 8 << (irq << 2));
        } else if (irq < 16) {
            CLRREG32(&g_pVRC5477Regs->INTCTRL1, 8 << ((irq - 8) << 2));
        } else if (irq < 24) {
            CLRREG32(&g_pVRC5477Regs->INTCTRL2, 8 << ((irq - 16) << 2));
        } else if (irq < 32) {
            CLRREG32(&g_pVRC5477Regs->INTCTRL3, 8 << ((irq - 24) << 2));
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
    UINT32 irq, i;

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
        // Depending on IRQ number use one from four registers
        if (irq < 8) {
            // Use interrupt mask register 1
            SETREG32(&g_pVRC5477Regs->INTCTRL0, 8 << (irq << 2));
        } else if (irq < 16) {
            SETREG32(&g_pVRC5477Regs->INTCTRL1, 8 << ((irq - 8) << 2));
        } else if (irq < 24) {
            SETREG32(&g_pVRC5477Regs->INTCTRL2, 8 << ((irq - 16) << 2));
        } else if (irq < 32) {
            SETREG32(&g_pVRC5477Regs->INTCTRL3, 8 << ((irq - 24) << 2));
        }        
    }

    OALMSG(OAL_INTR&&OAL_VERBOSE, (L"-OALIntrDoneIrqs\r\n"));
}


//------------------------------------------------------------------------------
//
//  Function:  OALIntrHandler
//
//  This is interrupt handler implementation for MIPS interrupt 0.
//
UINT32 OALIntrHandler (PULONG pStsReg)
{
    UINT32 irq = OAL_INTR_IRQ_UNDEFINED, sysIntr = SYSINTR_NOP;
    UINT32 status, mask, maskbit;
    PULONG pICReg = NULL;
    

#ifdef OAL_ILTIMING
    if (g_oalILT.active) g_oalILT.interrupts++;
#endif

    // Get pending interrupt
    status = INREG32(pStsReg);

    // This should not happer
    if (status == 0) goto cleanUp;

    // Find active interrupt    
    for (mask = 1, irq = 0; mask != 0; mask <<= 1, irq++) {
        if ((status & mask) != 0) break;
    }

    // find out the register and the bit mask
    if (irq < 8) {
        pICReg = &g_pVRC5477Regs->INTCTRL0;
        maskbit = 8 << (irq << 2);
    } else if (irq < 16) {
        pICReg = &g_pVRC5477Regs->INTCTRL1;
        maskbit = 8 << ((irq - 8) << 2);
    } else if (irq < 24) {
        pICReg = &g_pVRC5477Regs->INTCTRL2;
        maskbit = 8 << ((irq - 16) << 2);
    } else if (irq < 32) {
        pICReg = &g_pVRC5477Regs->INTCTRL3;
        maskbit = 8 << ((irq - 24) << 2);
    }        

    // mask the interrupt
    if (pICReg) {
        CLRREG32(pICReg, maskbit);
    }

    // Clear edge
    CLRREG32(&g_pVRC5477Regs->INTCLR32, mask);

#ifdef OAL_BSP_CALLBACKS
    // Give BSP chance to translate IRQ -- if there is subordinate
    // interrupt controller in BSP it give chance to decode its status
    // and change IRQ
    irq = BSPIntrActiveIrq(irq);
#endif

    // If there isn't valid IRQ leave
    if (irq == OAL_INTR_IRQ_UNDEFINED) goto cleanUp;
    
    // First find if IRQ is claimed by chain
    sysIntr = NKCallIntChain((UCHAR)irq);
    if (sysIntr == SYSINTR_CHAIN || !NKIsSysIntrValid(sysIntr)) {
        // IRQ wasn't claimed or SYSINTR isn't valid, use static mapping
        sysIntr = OALIntrTranslateIrq(irq);
    }

    // unmask interrupt if it's invalid or nop
    if (SYSINTR_NOP == sysIntr) {
#ifdef OAL_BSP_CALLBACKS
        BSPIntrEnableIrq (irq);
#endif
        if (pICReg) {
            SETREG32 (pICReg, maskbit);
        }
    }

#ifdef OAL_FAKE_IDLE
    // Set flag for fake wake/idle
    if (sysIntr != SYSINTR_NOP) g_oalLastSysIntr = sysIntr;
#endif
    
cleanUp:    
    return sysIntr;
}

//------------------------------------------------------------------------------
//
//  Function:  OALIntr0Handler
//
//  This is interrupt handler implementation for MIPS interrupt 0.
//
UINT32 OALIntr0Handler()
{
    return OALIntrHandler (&g_pVRC5477Regs->INT0STAT);
}

//------------------------------------------------------------------------------
//
//  Function:  OALIntr1Handler
//
//  This is interrupt handler implementation for MIPS interrupt 1.
//
UINT32 OALIntr1Handler()
{
    return OALIntrHandler (&g_pVRC5477Regs->INT1STAT);
}

//------------------------------------------------------------------------------
//
//  Function:  OALIntr2Handler
//
//  This is interrupt handler implementation for MIPS interrupt 2.
//
UINT32 OALIntr2Handler()
{
    return OALIntrHandler (&g_pVRC5477Regs->INT2STAT);
}

//------------------------------------------------------------------------------
//
//  Function:  OALIntr3Handler
//
//  This is interrupt handler implementation for MIPS interrupt 3.
//
UINT32 OALIntr3Handler()
{
    return OALIntrHandler (&g_pVRC5477Regs->INT3STAT);
}

//------------------------------------------------------------------------------
