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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
//------------------------------------------------------------------------------
//
//  File: intr.c
//
//  This file implement major part of interrupt module for OMAP5912 SoC.
//
#include <windows.h>
#include <ceddk.h>
#include <nkintr.h>
#include <oal.h>
#include <omap5912.h>


//------------------------------------------------------------------------------
//
//  Global:  g_oalProfilerIrq
//
//  IRQ of the timer used for profiling. Set by OEMProfileTimerEnable.
//
extern UINT32 g_oalProfilerIrq;

//------------------------------------------------------------------------------
//
//  Global:  g_oalProfilerEnabled
//
//  Indicates that profiling is enabled. Set by OEMProfileTimerEnable.
//
extern BOOL g_oalProfilerEnabled;

//------------------------------------------------------------------------------
//
//  Global:  g_oalTimerIrq
//
//  This variable contains IRQ of timer used for system clock. It is 
//  set in timer initialization function OALTimerInit.
//
UINT32 g_oalTimerIrq = OAL_INTR_IRQ_UNDEFINED;


//------------------------------------------------------------------------------
//
//  Static: g_pIntcRegs
//
//  This value contains virtual uncached address of interrupt controller
//  unit registers.
//
static OMAP5912_INTC_REGS *g_pIntcL1Regs;
static OMAP5912_INTC_REGS *g_pIntcL2ARegs;
static OMAP5912_INTC_REGS *g_pIntcL2BRegs;
static OMAP5912_INTC_REGS *g_pIntcL2CRegs;
static OMAP5912_INTC_REGS *g_pIntcL2DRegs;
static OMAP5912_GPIO_REGS *g_pGPIO1Regs;
static OMAP5912_GPIO_REGS *g_pGPIO2Regs;
static OMAP5912_GPIO_REGS *g_pGPIO3Regs;
static OMAP5912_GPIO_REGS *g_pGPIO4Regs;
static OMAP5912_ARMIO_REGS *g_pMPUIORegs;

//------------------------------------------------------------------------------
//
//  Static: g_intcLxLevel
//
//  Following arrays contain interrupt routing, level and priority
//  initialization values for ILR interrupt controller registers.
//
static UINT32 g_intcL1Level[] = {
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,   // 0
    ILR_FIQ|ILR_LEVEL|ILR_PRI16, ILR_FIQ|ILR_LEVEL|ILR_PRI16,   // 2
    ILR_IRQ|ILR_EDGE|ILR_PRI16, ILR_IRQ|ILR_EDGE|ILR_PRI16,   // 4
    ILR_IRQ|ILR_EDGE|ILR_PRI16, ILR_IRQ|ILR_EDGE|ILR_PRI16,    // 6
    ILR_IRQ|ILR_EDGE|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,   // 8
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,    // 10
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_EDGE|ILR_PRI16,   // 12
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,   // 14
    ILR_IRQ|ILR_EDGE|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,    // 16
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,   // 18
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,   // 20
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,   // 22
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,   // 24
    ILR_IRQ|ILR_EDGE|ILR_PRI16, ILR_IRQ|ILR_EDGE|ILR_PRI16,    // 26
    ILR_IRQ|ILR_EDGE|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,   // 28
    ILR_IRQ|ILR_EDGE|ILR_PRI16, ILR_IRQ|ILR_EDGE|ILR_PRI16    // 30
};

static UINT32 g_intcL2ALevel[] = {
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_EDGE|ILR_PRI16,   // 32/0
    ILR_IRQ|ILR_EDGE|ILR_PRI16, ILR_IRQ|ILR_EDGE|ILR_PRI16,    // 34/2
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,   // 36/4
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,   // 38/6
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_EDGE|ILR_PRI16,    // 40/8
    ILR_IRQ|ILR_EDGE|ILR_PRI16, ILR_IRQ|ILR_EDGE|ILR_PRI16,    // 42/10
    ILR_IRQ|ILR_EDGE|ILR_PRI16, ILR_IRQ|ILR_EDGE|ILR_PRI16,    // 44/12
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,   // 46/14
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,    // 48/16
    ILR_IRQ|ILR_EDGE|ILR_PRI16, ILR_IRQ|ILR_EDGE|ILR_PRI16,   // 50/18
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,   // 52/20
    ILR_IRQ|ILR_EDGE|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,   // 54/22
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_EDGE|ILR_PRI16,    // 56/24
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,   // 58/26
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,   // 60/28
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16    // 62/30
};

static UINT32 g_intcL2BLevel[] = {
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,   // 64/32
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,    // 66/34
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,   // 68/36
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,   // 70/38
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,    // 72/40
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_EDGE|ILR_PRI16,    // 74/42
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,   // 76/44
    ILR_IRQ|ILR_EDGE|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,   // 78/46
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,   // 80/48
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,   // 82/50
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,    // 84/52
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,    // 86/54
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,    // 88/56
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,    // 90/58
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,    // 92/60
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,   // 94/62
};

static UINT32 g_intcL2CLevel[] = {
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,   // 96/32
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,    // 98/34
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,   // 100/36
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,   // 102/38
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,    // 104/40
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,    // 106/42
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,   // 108/44
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,   // 110/46
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,   // 112/48
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,   // 114/50
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,    // 116/52
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,    // 118/54
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,    // 120/56
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,    // 122/58
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,    // 124/60
    ILR_IRQ|ILR_EDGE|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,   // 126/62
};

static UINT32 g_intcL2DLevel[] = {
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,   // 128/32
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,    // 130/34
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,   // 132/36
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,   // 134/38
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,    // 136/40
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,    // 138/42
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,   // 140/44
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,   // 142/46
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,   // 144/48
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,   // 146/50
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,    // 148/52
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,    // 150/54
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,    // 152/56
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,    // 154/58
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,    // 156/60
    ILR_IRQ|ILR_LEVEL|ILR_PRI16, ILR_IRQ|ILR_LEVEL|ILR_PRI16,   // 158/62
};

//------------------------------------------------------------------------------
//
//  Function:  OALIntrInit
//
//  This function initialize OMAP5912 interrupt subsystem. Implementation must
//  use its own mapping structure because general implementation limits
//  number of IRQ to 64 but OMAP5912 has 159 IRQs.
//
BOOL OALIntrInit()
{
    BOOL rc = FALSE;
    UINT32 irq, i;

    OALMSG(OAL_INTR&&OAL_FUNC, (L"+OALInterruptInit\r\n"));

    // Initialize interrupt mapping
    OALIntrMapInit();

    // Get interrupt controller registers virtual uncached addresses
    g_pIntcL1Regs = OALPAtoUA(OMAP5912_INTC_L1_REGS_PA);
    g_pIntcL2ARegs = OALPAtoUA(OMAP5912_INTC_L2A_REGS_PA);
    g_pIntcL2BRegs = OALPAtoUA(OMAP5912_INTC_L2B_REGS_PA);
    g_pIntcL2CRegs = OALPAtoUA(OMAP5912_INTC_L2C_REGS_PA);
    g_pIntcL2DRegs = OALPAtoUA(OMAP5912_INTC_L2D_REGS_PA);
    g_pGPIO1Regs = OALPAtoUA(OMAP5912_GPIO1_REGS_PA);
    g_pGPIO2Regs = OALPAtoUA(OMAP5912_GPIO2_REGS_PA);
    g_pGPIO3Regs = OALPAtoUA(OMAP5912_GPIO3_REGS_PA);
    g_pGPIO4Regs = OALPAtoUA(OMAP5912_GPIO4_REGS_PA);
    g_pMPUIORegs = OALPAtoUA(OMAP5912_ARMIO_REGS_PA);

    // Disable all interrupts
    OUTREG32(&g_pIntcL1Regs->MIR, 0xFFFFFFFF);
    OUTREG32(&g_pIntcL2ARegs->MIR, 0xFFFFFFFF);
    OUTREG32(&g_pIntcL2BRegs->MIR, 0xFFFFFFFF);
    OUTREG32(&g_pIntcL2CRegs->MIR, 0xFFFFFFFF);
    OUTREG32(&g_pIntcL2DRegs->MIR, 0xFFFFFFFF);
    OUTREG32(&g_pGPIO1Regs->IRQENABLE1, 0);
    OUTREG32(&g_pGPIO2Regs->IRQENABLE1, 0);
    OUTREG32(&g_pGPIO3Regs->IRQENABLE1, 0);
    OUTREG32(&g_pGPIO4Regs->IRQENABLE1, 0);
    OUTREG16(&g_pMPUIORegs->IO_INT_MASK, 0xFFFF);

    // Initialize interrupt routing, level and priority
    for (i = 0; i < 32; i++) {
        OUTREG32(&g_pIntcL1Regs->ILR[i], g_intcL1Level[i]);
        OUTREG32(&g_pIntcL2ARegs->ILR[i], g_intcL2ALevel[i]);
        OUTREG32(&g_pIntcL2BRegs->ILR[i], g_intcL2BLevel[i]);
        OUTREG32(&g_pIntcL2CRegs->ILR[i], g_intcL2CLevel[i]);
        OUTREG32(&g_pIntcL2DRegs->ILR[i], g_intcL2DLevel[i]);
    }

    // Enable interrupts from L2 controllers
    irq = IRQ_L2FIQ; OALIntrEnableIrqs(1, &irq);
    irq = IRQ_L2IRQ; OALIntrEnableIrqs(1, &irq);

    // Add static mapping for RTC alarm
    OALIntrStaticTranslate(SYSINTR_RTC_ALARM, IRQ_RTC_ALARM);

    // And enable it (it will not occur until it is set in OEMSetAlarmTime)
    OEMInterruptEnable(SYSINTR_RTC_ALARM, NULL, 0);

    // Call board specific initialization
    rc = BSPIntrInit();

    OALMSG(OAL_INTR&&OAL_FUNC, (L"-OALInterruptInit(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  OALIntrRequestIrq
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

    // This shouldn't happen
    if (*pCount < 1) goto cleanUp;

    switch (pDevLoc->IfcType) {
    case Internal:
        switch ((ULONG)pDevLoc->LogicalLoc) {
        case OMAP5912_USBD_REGS_PA:
            *pCount = 1;
            pIrqs[0] = IRQ_USB;
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
    UINT32 irq, i;

    OALMSG(OAL_INTR&&OAL_VERBOSE, (
        L"+OALntrEnableIrqs(%d, 0x%08x)\r\n", count, pIrqs
    ));

    for (i = 0; i < count; i++) {
        irq = pIrqs[i];
        if (irq < 32) {
            CLRREG32(&g_pIntcL1Regs->MIR, 1 << irq);
        } else if (irq < 64) {
            CLRREG32(&g_pIntcL2ARegs->MIR, 1 << (irq - 32));
        } else if (irq < 96) {
            CLRREG32(&g_pIntcL2BRegs->MIR, 1 << (irq - 64));
        } else if (irq < 128) {
            CLRREG32(&g_pIntcL2CRegs->MIR, 1 << (irq - 96));
        } else if (irq < 160) {
            CLRREG32(&g_pIntcL2DRegs->MIR, 1 << (irq - 128));
        } else if (irq < IRQ_GPIO_16) {
            CLRREG32(&g_pIntcL1Regs->MIR, 1 << IRQ_GPIO1);
            SETREG32(&g_pGPIO1Regs->IRQENABLE1, 1 << (irq - IRQ_GPIO_0));
        } else if (irq < IRQ_GPIO_32) {
            CLRREG32(&g_pIntcL2BRegs->MIR, 1 << (IRQ_GPIO2-64));
            SETREG32(&g_pGPIO2Regs->IRQENABLE1, 1 << (irq - IRQ_GPIO_16));
        } else if (irq < IRQ_GPIO_48) {
            CLRREG32(&g_pIntcL2BRegs->MIR, 1 << (IRQ_GPIO3-64));
            SETREG32(&g_pGPIO3Regs->IRQENABLE1, 1 << (irq - IRQ_GPIO_32));
        } else if (irq < IRQ_MPUIO_0) {
            CLRREG32(&g_pIntcL2BRegs->MIR, 1 << (IRQ_GPIO4-64));
            SETREG32(&g_pGPIO4Regs->IRQENABLE1, 1 << (irq - IRQ_GPIO_48));
        } else if (irq < OMAP5912_IRQ_MAXIMUM) {
            CLRREG32(&g_pIntcL2ARegs->MIR, 1 << (IRQ_MPUIO-32));
            CLRREG16(&g_pMPUIORegs->IO_INT_MASK, 1 << (irq - IRQ_MPUIO_0));
        } else if (irq != OAL_INTR_IRQ_UNDEFINED) {
            rc = FALSE;
        }
    }

    OALMSG(OAL_INTR&&OAL_VERBOSE, (L"-OALIntrEnableIrqs(rc = %d)\r\n", rc));
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
        L"+OALIntrDisableIrqs(%d, 0x%08x)\r\n", count, pIrqs
    ));

    for (i = 0; i < count; i++) {
        irq = pIrqs[i];
        if (irq < 32) {
            SETREG32(&g_pIntcL1Regs->MIR, 1 << irq);
        } else if (irq < 64) {
            SETREG32(&g_pIntcL2ARegs->MIR, 1 << (irq - 32));
        } else if (irq < 96) {
            SETREG32(&g_pIntcL2BRegs->MIR, 1 << (irq - 64));
        } else if (irq < 128) {
            SETREG32(&g_pIntcL2CRegs->MIR, 1 << (irq - 96));
        } else if (irq < 160) {
            SETREG32(&g_pIntcL2DRegs->MIR, 1 << (irq - 128));
        } else if (irq < IRQ_GPIO_16) {
            CLRREG32(&g_pGPIO1Regs->IRQENABLE1, 1 << (irq - IRQ_GPIO_0));
        } else if (irq < IRQ_GPIO_32) {
            CLRREG32(&g_pGPIO2Regs->IRQENABLE1, 1 << (irq - IRQ_GPIO_16));
        } else if (irq < IRQ_GPIO_48) {
            CLRREG32(&g_pGPIO3Regs->IRQENABLE1, 1 << (irq - IRQ_GPIO_32));
        } else if (irq < IRQ_MPUIO_0) {
            CLRREG32(&g_pGPIO4Regs->IRQENABLE1, 1 << (irq - IRQ_GPIO_48));
        } else if (irq < OMAP5912_IRQ_MAXIMUM) {
            SETREG16(&g_pMPUIORegs->IO_INT_MASK, 1 << (irq - IRQ_MPUIO_0));
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
        irq = pIrqs[i];
        if (irq < 32) {
            CLRREG32(&g_pIntcL1Regs->MIR, 1 << irq);
        } else if (irq < 64) {
            CLRREG32(&g_pIntcL2ARegs->MIR, 1 << (irq - 32));
        } else if (irq < 96) {
            CLRREG32(&g_pIntcL2BRegs->MIR, 1 << (irq - 64));
        } else if (irq < 128) {
            CLRREG32(&g_pIntcL2CRegs->MIR, 1 << (irq - 96));
        } else if (irq < 160) {
            CLRREG32(&g_pIntcL2DRegs->MIR, 1 << (irq - 128));
        } else if (irq < IRQ_GPIO_16) {
            SETREG32(&g_pGPIO1Regs->IRQENABLE1, 1 << (irq - IRQ_GPIO_0));
        } else if (irq < IRQ_GPIO_32) {
            SETREG32(&g_pGPIO2Regs->IRQENABLE1, 1 << (irq - IRQ_GPIO_16));
        } else if (irq < IRQ_GPIO_48) {
            SETREG32(&g_pGPIO3Regs->IRQENABLE1, 1 << (irq - IRQ_GPIO_32));
        } else if (irq < IRQ_MPUIO_0) {
            SETREG32(&g_pGPIO4Regs->IRQENABLE1, 1 << (irq - IRQ_GPIO_48));
        } else if (irq < OMAP5912_IRQ_MAXIMUM) {
            CLRREG16(&g_pMPUIORegs->IO_INT_MASK, 1 << (irq - IRQ_MPUIO_0));
        }
    }
}


//------------------------------------------------------------------------------
//
//  Function:  OEMInterruptHandler
//
//  This is interrupt handler implementation.
//
UINT32 OEMInterruptHandler(DWORD ra)
{
    UINT32 irq = OAL_INTR_IRQ_UNDEFINED;
    UINT32 sysIntr = SYSINTR_NOP;
    UINT32 data, mask;
    OMAP5912_GPIO_REGS *pGPIORegs = NULL;
    OMAP5912_ARMIO_REGS *pMPUIORegs = NULL;

    // Get pending interrupt
    irq = INREG32(&g_pIntcL1Regs->SIR_IRQ);

    // First check if it is interrupt from cascade controller
    if (irq != IRQ_L2IRQ) {
        // It isn't cascade interrupt, check for GPIO interrupt
        if (irq == IRQ_GPIO1 && (data = INREG32(&g_pGPIO1Regs->IRQSTATUS1)) != 0) {
            // It is interrupt from GPIO1, find which one, mask and clear it.
            for (irq = IRQ_GPIO_0, mask = 1; mask != 0; mask <<= 1, irq++) {
                if ((mask & data) != 0) break;
            }
            CLRPORT32(&g_pGPIO1Regs->IRQENABLE1, mask);
            OUTPORT32(&g_pGPIO4Regs->IRQSTATUS1, mask);
        } else {
            SETREG32(&g_pIntcL1Regs->MIR, 1 << irq);
        }
    } else {

        // It is interrupt from L2, get number from there
        irq = INREG32(&g_pIntcL2ARegs->SIR_IRQ) + 32;

        // check for GPIO interrupts
        switch (irq) {
        case IRQ_GPIO2:
            irq = IRQ_GPIO_16;
            pGPIORegs=g_pGPIO2Regs;
            break;
        case IRQ_GPIO3:
            irq = IRQ_GPIO_32;
            pGPIORegs=g_pGPIO3Regs;
            break;
        case IRQ_GPIO4:
            irq = IRQ_GPIO_48;
            pGPIORegs=g_pGPIO4Regs;
            break;
        case IRQ_MPUIO:
            irq = IRQ_MPUIO_0;
            pMPUIORegs = g_pMPUIORegs;
            break;
        }

        // If it is interrupt from GPIOx, find which one, mask and clear it..
        if (pGPIORegs != NULL && (data = INREG32(&pGPIORegs->IRQSTATUS1)) != 0) {
            for (mask = 1; mask != 0; mask <<= 1, irq++) {
                if ((mask & data) != 0) break;
            }
            CLRPORT32(&pGPIORegs->IRQENABLE1, mask);
            OUTPORT32(&pGPIORegs->IRQSTATUS1, mask);
        }
        else if (pMPUIORegs != NULL && (data = INREG16(&pMPUIORegs->IO_INT_STAT)) != 0) {
            for (mask = 1; mask != 0; mask <<= 1, irq++) {
                if ((mask & data) != 0) break;
            }
            SETPORT16(&pMPUIORegs->IO_INT_MASK, mask);
        }
        else if (irq < 64) {
            // Mask interrupt on L2A
            SETPORT32(&g_pIntcL2ARegs->MIR, 1 << (irq - 32));
        } else if (irq < 96) {
            // Mask interrupt on L2B
            SETPORT32(&g_pIntcL2BRegs->MIR, 1 << (irq - 64));
        } else if (irq < 128) {
            // Mask interrupt on L2C
            SETPORT32(&g_pIntcL2CRegs->MIR, 1 << (irq - 96));
        } else {
            // Mask interrupt on L2D
            SETPORT32(&g_pIntcL2DRegs->MIR, 1 << (irq - 128));
        }

        // Acknowledge interrupt on L2
        OUTREG32(&g_pIntcL2ARegs->CNTL, CNTL_NEW_IRQ);
    }

    // Acknowledge interrupt on L1
    OUTREG32(&g_pIntcL1Regs->CNTL, CNTL_NEW_IRQ);

    // Check if this is timer IRQ
    if (irq == g_oalTimerIrq) {
         // Call timer interrupt handler

        sysIntr = OALTimerIntrHandler(ra);
        // We are done with interrupt
        OALIntrDoneIrqs(1, &irq);
    } else if (irq == g_oalProfilerIrq && g_oalProfilerEnabled == TRUE) {
        // Call profiler and reenable interrupt
        ProfilerHit(ra);
        CLRREG32(&g_pIntcL1Regs->MIR, 1 << irq);
    } else if (irq != OAL_INTR_IRQ_UNDEFINED) {
        // We don't assume IRQ sharing, use static mapping
        sysIntr = OALIntrTranslateIrq(irq);
    }
    
    return sysIntr;
}


//------------------------------------------------------------------------------
//
//  Function:  OALIntrIsIrqPending
//
//  This function checks if the given interrupt is pending.
//
BOOL OALIntrIsIrqPending(UINT32 irq)
{
    BOOL rc = FALSE;

    if (irq < 32) {
        rc = INREG32(&g_pIntcL1Regs->ITR) & (1 << irq);
    } else if (irq < 64) {
        rc = INREG32(&g_pIntcL2ARegs->ITR) & (1 << (irq - 32));
    } else if (irq < 96) {
        rc = INREG32(&g_pIntcL2BRegs->ITR) & (1 << (irq - 64));
    } else if (irq < 128) {
        rc = INREG32(&g_pIntcL2CRegs->ITR) & (1 << (irq - 96));
    } else if(irq < 160){
        rc = INREG32(&g_pIntcL2DRegs->ITR) & (1 << (irq - 128));
    } else if (irq < IRQ_GPIO_16) {
        rc = INREG32(&g_pGPIO1Regs->IRQSTATUS1) & (1 << (irq - IRQ_GPIO_0));
    } else if (irq < IRQ_GPIO_32) {
        rc = INREG32(&g_pGPIO2Regs->IRQSTATUS1) & (1 << (irq - IRQ_GPIO_16));
    } else if (irq < IRQ_GPIO_48) {
        rc = INREG32(&g_pGPIO3Regs->IRQSTATUS1) & (1 << (irq - IRQ_GPIO_32));
    } else if (irq < IRQ_MPUIO_0) {
        rc = INREG32(&g_pGPIO4Regs->IRQSTATUS1) & (1 << (irq - IRQ_GPIO_48));
    } else if (irq < OMAP5912_IRQ_MAXIMUM) {
        rc = INREG16(&g_pMPUIORegs->IO_INT_STAT) & (1 << (irq - IRQ_MPUIO_0));
    }

    return (rc != 0);
}

//------------------------------------------------------------------------------
