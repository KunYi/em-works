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
//  This file implement major part of interrupt module for OMAP2420 SoC.
//
#include <windows.h>
#include <ceddk.h>
#include <nkintr.h>
#include <oal.h>
#include <omap2420.h>
#include <bsp_menelaus.h>

//------------------------------------------------------------------------------
//
//  Extern:  g_oalIrq2SysIntr
//
//  IRQ to SYSINTR mapping table
//
extern UINT32 g_oalIrq2SysIntr[];

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
static OMAP2420_MPUINTC_REGS *g_pIntcRegs;

//------------------------------------------------------------------------------
//
//  Static: g_pIntcRegs
//
//  This value contains virtual uncached address of interrupt controller
//  unit registers.
//
static OMAP2420_GPIO_REGS *g_pGPIORegs[4];


//------------------------------------------------------------------------------
//
//  Static: g_intcLxLevel
//
//  Following arrays contain interrupt routing and priority
//  initialization values for ILR interrupt controller registers.
//  Level initialization is not performed here for the OMAP2420.
//
static UINT32 g_intcPriorityLevel[] = {
    ILR_FIQ|ILR_PRI16, ILR_IRQ|ILR_PRI16,   // 0
    ILR_IRQ|ILR_PRI16, ILR_IRQ|ILR_PRI16,   // 2
    ILR_IRQ|ILR_PRI16, ILR_IRQ|ILR_PRI16,   // 4
    ILR_IRQ|ILR_PRI16, ILR_IRQ|ILR_PRI16,   // 6
    ILR_IRQ|ILR_PRI16, ILR_IRQ|ILR_PRI16,   // 8
    ILR_IRQ|ILR_PRI16, ILR_IRQ|ILR_PRI16,   // 10
    ILR_IRQ|ILR_PRI16, ILR_IRQ|ILR_PRI16,   // 12
    ILR_IRQ|ILR_PRI16, ILR_IRQ|ILR_PRI16,   // 14
    ILR_IRQ|ILR_PRI16, ILR_IRQ|ILR_PRI16,   // 16
    ILR_IRQ|ILR_PRI16, ILR_IRQ|ILR_PRI16,   // 18
    ILR_IRQ|ILR_PRI16, ILR_IRQ|ILR_PRI16,   // 20
    ILR_IRQ|ILR_PRI16, ILR_IRQ|ILR_PRI16,   // 22
    ILR_IRQ|ILR_PRI16, ILR_IRQ|ILR_PRI16,   // 24
    ILR_IRQ|ILR_PRI16, ILR_IRQ|ILR_PRI16,   // 26
    ILR_IRQ|ILR_PRI16, ILR_IRQ|ILR_PRI16,   // 28
    ILR_IRQ|ILR_PRI16, ILR_IRQ|ILR_PRI16,   // 30

    ILR_IRQ|ILR_PRI16, ILR_IRQ|ILR_PRI16,   // 32/0
    ILR_IRQ|ILR_PRI16, ILR_IRQ|ILR_PRI16,   // 34/2
    ILR_IRQ|ILR_PRI16, ILR_IRQ|ILR_PRI16,   // 36/4
    ILR_IRQ|ILR_PRI16, ILR_IRQ|ILR_PRI16,   // 38/6
    ILR_IRQ|ILR_PRI16, ILR_IRQ|ILR_PRI16,   // 40/8
    ILR_IRQ|ILR_PRI16, ILR_IRQ|ILR_PRI16,   // 42/10
    ILR_IRQ|ILR_PRI16, ILR_IRQ|ILR_PRI16,   // 44/12
    ILR_IRQ|ILR_PRI16, ILR_IRQ|ILR_PRI16,   // 46/14
    ILR_IRQ|ILR_PRI16, ILR_IRQ|ILR_PRI16,   // 48/16
    ILR_IRQ|ILR_PRI16, ILR_IRQ|ILR_PRI16,   // 50/18
    ILR_IRQ|ILR_PRI16, ILR_IRQ|ILR_PRI16,   // 52/20
    ILR_IRQ|ILR_PRI16, ILR_IRQ|ILR_PRI16,   // 54/22
    ILR_IRQ|ILR_PRI16, ILR_IRQ|ILR_PRI16,   // 56/24
    ILR_IRQ|ILR_PRI16, ILR_IRQ|ILR_PRI16,   // 58/26
    ILR_IRQ|ILR_PRI16, ILR_IRQ|ILR_PRI16,   // 60/28
    ILR_IRQ|ILR_PRI16, ILR_IRQ|ILR_PRI16,   // 62/30

    ILR_IRQ|ILR_PRI16, ILR_IRQ|ILR_PRI16,   // 64/32
    ILR_IRQ|ILR_PRI16, ILR_IRQ|ILR_PRI16,   // 66/34
    ILR_IRQ|ILR_PRI16, ILR_IRQ|ILR_PRI16,   // 68/36
    ILR_IRQ|ILR_PRI16, ILR_IRQ|ILR_PRI16,   // 70/38
    ILR_IRQ|ILR_PRI16, ILR_IRQ|ILR_PRI16,   // 72/40
    ILR_IRQ|ILR_PRI16, ILR_IRQ|ILR_PRI16,   // 74/42
    ILR_IRQ|ILR_PRI16, ILR_IRQ|ILR_PRI16,   // 76/44
    ILR_IRQ|ILR_PRI16, ILR_IRQ|ILR_PRI16,   // 78/46
    ILR_IRQ|ILR_PRI16, ILR_IRQ|ILR_PRI16,   // 80/48
    ILR_IRQ|ILR_PRI16, ILR_IRQ|ILR_PRI16,   // 82/50
    ILR_IRQ|ILR_PRI16, ILR_IRQ|ILR_PRI16,   // 84/52
    ILR_IRQ|ILR_PRI16, ILR_IRQ|ILR_PRI16,   // 86/54
    ILR_IRQ|ILR_PRI16, ILR_IRQ|ILR_PRI16,   // 88/56
    ILR_IRQ|ILR_PRI16, ILR_IRQ|ILR_PRI16,   // 90/58
    ILR_IRQ|ILR_PRI16, ILR_IRQ|ILR_PRI16,   // 92/60
    ILR_IRQ|ILR_PRI16, ILR_IRQ|ILR_PRI16    // 94/62
};

static UINT32 g_GPIO_LEVELDETECT0[]  = {
	0x0000, //GPIO0-31 
	0x0000, //GPIO32-63
	1<<28 , //GPIO64-95, GPIO92(kitl)
	0x0000  //GPIO96-127
};
static UINT32 g_GPIO_LEVELDETECT1[]  = {
	0x0000, //GPIO0-31 
	0x0000, //GPIO32-63
	0x0000, //GPIO64-95 
	0x0000  //GPIO96-127
};
static UINT32 g_GPIO_RISINGDETECT[]  = {
	0x0000, //GPIO0-31 
	0x0000, //GPIO32-63
	0x0000, //GPIO64-95 
	0x0000  //GPIO96-127
};
static UINT32 g_GPIO_FALLINGDETECT[]  = {

//  GPIO11, GPIO6,
	1<<11 | 1<<6,	//GPIO0-31 

	0x0000,			//GPIO32-63

//  GPIO93(keyboard), GPIO89(keyboard), GPIO88(keyboard),
	1<<29 | 1<<25 | 1<<24,	//GPIO64-95
	
//  GPIO124,
	1<<28 ,			//GPIO96-127
};

//------------------------------------------------------------------------------
//
//  Function:  OALIntrInit
//
//  This function initialize OMAP2420 interrupt subsystem. Implementation must
//  use its own mapping structure because general implementation limits
//  number of IRQ to 64 but OMAP2420 has 96 IRQs.
//
BOOL OALIntrInit()
{
    BOOL rc = FALSE;
    UINT32 i;

    OALMSG(OAL_INTR&&OAL_FUNC, (L"+OALInterruptInit\r\n"));

    // Initialize interrupt mapping
    OALIntrMapInit();

    // Get interrupt controller and GPIO registers' virtual uncached addresses
    g_pIntcRegs = OALPAtoUA(OMAP2420_INTC_MPU_REGS_PA);
    g_pGPIORegs[0] = OALPAtoUA(OMAP2420_GPIO1_REGS_PA);
    g_pGPIORegs[1] = OALPAtoUA(OMAP2420_GPIO2_REGS_PA);
    g_pGPIORegs[2] = OALPAtoUA(OMAP2420_GPIO3_REGS_PA);
    g_pGPIORegs[3] = OALPAtoUA(OMAP2420_GPIO4_REGS_PA);

    //Reset the MPU INTC and wait until reset is complete
    SETREG32(&g_pIntcRegs->ulINTC_SYSCONFIG, OMAP2420_MPUINTC_RESETBIT);
    while ((INREG32(&g_pIntcRegs->ulINTC_SYSSTATUS)& OMAP2420_MPUINTC_RESETSTATUS) == 0);

    // Disable all interrupts and clear the ISR - for all for GPIO banks, too.
    OUTREG32(&g_pIntcRegs->ulINTC_MIR_SET0, OMAP2420_MPUINTC_MASKALL);
    OUTREG32(&g_pIntcRegs->ulINTC_MIR_SET1, OMAP2420_MPUINTC_MASKALL);
    OUTREG32(&g_pIntcRegs->ulINTC_MIR_SET2, OMAP2420_MPUINTC_MASKALL);

    for (i = 0; i < 4; i++) {
        OUTREG32(&g_pGPIORegs[i]->ulGPIO_CLEARIRQENABLE1, 0xFFFFFFFF);
        OUTREG32(&g_pGPIORegs[i]->ulGPIO_CLEARIRQENABLE2, 0xFFFFFFFF);
		OUTREG32(&g_pGPIORegs[i]->ulGPIO_IRQSTATUS1, 0xFFFFFFFF);     
		OUTREG32(&g_pGPIORegs[i]->ulGPIO_IRQSTATUS2, 0xFFFFFFFF);    
        OUTREG32(&g_pGPIORegs[i]->ulGPIO_LEVELDETECT0, g_GPIO_LEVELDETECT0[i]);
        OUTREG32(&g_pGPIORegs[i]->ulGPIO_LEVELDETECT1, g_GPIO_LEVELDETECT1[i]);
		OUTREG32(&g_pGPIORegs[i]->ulGPIO_RISINGDETECT, g_GPIO_RISINGDETECT[i]);     
		OUTREG32(&g_pGPIORegs[i]->ulGPIO_FALLINGDETECT, g_GPIO_FALLINGDETECT[i]);    
    }

    // Initialize interrupt routing and priority
    for (i = 0; i < 96; i++) {
        OUTREG32(&g_pIntcRegs->ulINTC_ILR[i], g_intcPriorityLevel[i]);
    }

    // Allow interrupts 
    INTERRUPTS_ON();

    //  Allocate SYSINTR_RESCHED to a timer for the system tick
    OALIntrStaticTranslate( SYSINTR_RESCHED, IRQ_GPT1 );
    OEMInterruptEnable( SYSINTR_RESCHED, NULL, 0 );

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
#if 0
    switch (pDevLoc->IfcType) {
    case Internal:
        switch ((ULONG)pDevLoc->LogicalLoc) {
        case OMAP2420_USBD_REGS_PA:
            *pCount = 1;
            pIrqs[0] = IRQ_USB_GENI;
            rc = TRUE;
            break;
        }
        break;
    }
#endif            
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

    OALMSG(OAL_INTR, (L"+OALIntrEnableIrqs(%d, 0x%08x)\r\n", count, pIrqs));

    for (i = 0; i < count; i++) {
        irq = pIrqs[i];
        if (irq < 32) {
            OUTREG32(&g_pIntcRegs->ulINTC_MIR_CLEAR0, 1 << irq);
        } else if (irq < 64) {
            OUTREG32(&g_pIntcRegs->ulINTC_MIR_CLEAR1, 1 << (irq - 32));
        } else if (irq < 96) {
            OUTREG32(&g_pIntcRegs->ulINTC_MIR_CLEAR2, 1 << (irq - 64));
		} else if (irq < IRQ_GPIO_32) {
			OALMSG(OAL_INTR, (L"OALIntrEnableIrqs(Irq GPIO1 %d)\r\n", irq - IRQ_GPIO_0));
            OUTREG32(&g_pIntcRegs->ulINTC_MIR_CLEAR0, 1 << IRQ_GPIO1_MPU);
			OUTREG32(&g_pGPIORegs[0]->ulGPIO_IRQSTATUS1, 1<<(irq - IRQ_GPIO_0));  // clear IRQ
			SETREG32(&g_pGPIORegs[0]->ulGPIO_IRQENABLE1, 1<<(irq - IRQ_GPIO_0));  // unmask IRQ
        } else if (irq < IRQ_GPIO_64) {
			OALMSG(OAL_INTR, (L"OALIntrEnableIrqs(Irq GPIO2 %d)\r\n", irq - IRQ_GPIO_32));
            OUTREG32(&g_pIntcRegs->ulINTC_MIR_CLEAR0, 1 << IRQ_GPIO2_MPU);
			OUTREG32(&g_pGPIORegs[1]->ulGPIO_IRQSTATUS1, 1<<(irq - IRQ_GPIO_32));  // clear IRQ
			SETREG32(&g_pGPIORegs[1]->ulGPIO_IRQENABLE1, 1<<(irq - IRQ_GPIO_32));  // unmask IRQ
        } else if (irq < IRQ_GPIO_96) {
			OALMSG(OAL_INTR, (L"OALIntrEnableIrqs(Irq GPIO3 %d)\r\n", irq - IRQ_GPIO_64));
            OUTREG32(&g_pIntcRegs->ulINTC_MIR_CLEAR0, 1 << IRQ_GPIO3_MPU);
			OUTREG32(&g_pGPIORegs[2]->ulGPIO_IRQSTATUS1, 1<<(irq - IRQ_GPIO_64));  // clear IRQ
			SETREG32(&g_pGPIORegs[2]->ulGPIO_IRQENABLE1, 1<<(irq - IRQ_GPIO_64));  // unmask IRQ
        } else if (irq < IRQ_GPIO_128) {
			OALMSG(OAL_INTR, (L"OALIntrEnableIrqs(Irq GPIO4 %d)\r\n", irq - IRQ_GPIO_96));
            OUTREG32(&g_pIntcRegs->ulINTC_MIR_CLEAR1, 1<<(IRQ_GPIO4_MPU-32));
			OUTREG32(&g_pGPIORegs[3]->ulGPIO_IRQSTATUS1, 1<<(irq - IRQ_GPIO_96));  // clear IRQ
			SETREG32(&g_pGPIORegs[3]->ulGPIO_IRQENABLE1, 1<<(irq - IRQ_GPIO_96));  // unmask IRQ
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
            OUTREG32(&g_pIntcRegs->ulINTC_MIR_SET0, 1 << irq);
        } else if (irq < 64) {
            OUTREG32(&g_pIntcRegs->ulINTC_MIR_SET1, 1 << (irq - 32));
        } else if (irq < 96) {
            OUTREG32(&g_pIntcRegs->ulINTC_MIR_SET2, 1 << (irq - 64));
        } else if (irq <= IRQ_MENELAUS_PSHBTN) {
		    OALMSG(OAL_INTR, (L"OALIntrDisableIrqs(Irq MENELAUS %d)\r\n", irq - IRQ_MENELAUS_CD1));
			OUTREG32(&g_pIntcRegs->ulINTC_MIR_SET0, 1 << IRQ_SYS_NIRQ);

        } else if (irq < IRQ_GPIO_32) {
			OALMSG(OAL_INTR, (
				L"OALIntrDisableIrqs(Irq GPIO1 %d)\r\n", irq - IRQ_GPIO_0));
            OUTREG32(&g_pIntcRegs->ulINTC_MIR_SET0, 1 << IRQ_GPIO1_MPU);
			CLRREG32(&g_pGPIORegs[0]->ulGPIO_IRQENABLE1, 1<<(irq - IRQ_GPIO_0));  // mask IRQ
			OUTREG32(&g_pGPIORegs[0]->ulGPIO_IRQSTATUS1, 1<<(irq - IRQ_GPIO_0));  // clear IRQ
        } else if (irq < IRQ_GPIO_64) {
			OALMSG(OAL_INTR, (
				L"OALIntrDisableIrqs(Irq GPIO2 %d)\r\n", irq - IRQ_GPIO_32));
            OUTREG32(&g_pIntcRegs->ulINTC_MIR_SET0, 1 << IRQ_GPIO2_MPU);
			CLRREG32(&g_pGPIORegs[1]->ulGPIO_IRQENABLE1, 1<<(irq - IRQ_GPIO_32));  // mask IRQ
			OUTREG32(&g_pGPIORegs[1]->ulGPIO_IRQSTATUS1, 1<<(irq - IRQ_GPIO_32));  // clear IRQ
        } else if (irq < IRQ_GPIO_96) {
			OALMSG(OAL_INTR, (
				L"OALIntrDisableIrqs(Irq GPIO3 %d)\r\n", irq - IRQ_GPIO_64));
            OUTREG32(&g_pIntcRegs->ulINTC_MIR_SET0, 1 << IRQ_GPIO3_MPU);
			CLRREG32(&g_pGPIORegs[2]->ulGPIO_IRQENABLE1, 1<<(irq - IRQ_GPIO_64));  // mask IRQ
			OUTREG32(&g_pGPIORegs[2]->ulGPIO_IRQSTATUS1, 1<<(irq - IRQ_GPIO_64));  // clear IRQ
        } else if (irq < IRQ_GPIO_128) {
			OALMSG(OAL_INTR, (
				L"OALIntrDisableIrqs(Irq GPIO4 %d)\r\n", irq - IRQ_GPIO_96));
            OUTREG32(&g_pIntcRegs->ulINTC_MIR_SET1, 1 << (IRQ_GPIO4_MPU-32));
			CLRREG32(&g_pGPIORegs[3]->ulGPIO_IRQENABLE1, 1<<(irq - IRQ_GPIO_96));  // mask IRQ
			OUTREG32(&g_pGPIORegs[3]->ulGPIO_IRQSTATUS1, 1<<(irq - IRQ_GPIO_96));  // clear IRQ
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

    for (i = 0; i < count; i++) {
        irq = pIrqs[i];
        if (irq < 32) {
            OUTREG32(&g_pIntcRegs->ulINTC_MIR_CLEAR0, 1 << irq);
        } else if (irq < 64) {
            OUTREG32(&g_pIntcRegs->ulINTC_MIR_CLEAR1, 1 << (irq - 32));
        } else if (irq < 96) {
            OUTREG32(&g_pIntcRegs->ulINTC_MIR_CLEAR2, 1 << (irq - 64));
        } else if (irq < IRQ_GPIO_32) {
			OALMSG(OAL_INTR, (L"OALIntrDoneIrqs(Irq GPIO1 %d)\r\n", irq - IRQ_GPIO_0));
            OUTREG32(&g_pIntcRegs->ulINTC_MIR_CLEAR0, 1 << IRQ_GPIO1_MPU);
			OUTREG32(&g_pGPIORegs[0]->ulGPIO_IRQSTATUS1, 1<<(irq - IRQ_GPIO_0));  // clear IRQ
			SETREG32(&g_pGPIORegs[0]->ulGPIO_IRQENABLE1, 1<<(irq - IRQ_GPIO_0));  // unmask IRQ
        } else if (irq < IRQ_GPIO_64) {
			OALMSG(OAL_INTR, (L"OALIntrDoneIrqs(Irq GPIO2 %d)\r\n", irq - IRQ_GPIO_32));
            OUTREG32(&g_pIntcRegs->ulINTC_MIR_CLEAR0, 1 << IRQ_GPIO2_MPU);
			OUTREG32(&g_pGPIORegs[1]->ulGPIO_IRQSTATUS1, 1<<(irq - IRQ_GPIO_32));  // clear IRQ
			SETREG32(&g_pGPIORegs[1]->ulGPIO_IRQENABLE1, 1<<(irq - IRQ_GPIO_32));  // unmask IRQ
        } else if (irq < IRQ_GPIO_96) {
			OALMSG(OAL_INTR, (L"OALIntrDoneIrqs(Irq GPIO3 %d)\r\n", irq - IRQ_GPIO_64));
            OUTREG32(&g_pIntcRegs->ulINTC_MIR_CLEAR0, 1 << IRQ_GPIO3_MPU);
			OUTREG32(&g_pGPIORegs[2]->ulGPIO_IRQSTATUS1, 1<<(irq - IRQ_GPIO_64));  // clear IRQ
			SETREG32(&g_pGPIORegs[2]->ulGPIO_IRQENABLE1, 1<<(irq - IRQ_GPIO_64));  // unmask IRQ
        } else if (irq < IRQ_GPIO_128) {
			OALMSG(OAL_INTR, (L"OALIntrDoneIrqs(Irq GPIO4 %d)\r\n", irq - IRQ_GPIO_96));
            OUTREG32(&g_pIntcRegs->ulINTC_MIR_CLEAR1, 1 << (IRQ_GPIO4_MPU-32));
			OUTREG32(&g_pGPIORegs[3]->ulGPIO_IRQSTATUS1, 1<<(irq - IRQ_GPIO_96));  // clear IRQ
			SETREG32(&g_pGPIORegs[3]->ulGPIO_IRQENABLE1, 1<<(irq - IRQ_GPIO_96));  // unmask IRQ
        } 
    }
    OALMSG(OAL_INTR&&OAL_VERBOSE, (L"-OALIntrDoneIrq\r\n"));
}

//------------------------------------------------------------------------------
//
//  Function:  OEMInterruptHandler
//
//  This is interrupt handler implementation.
//
UINT32 OEMInterruptHandler()
{
    UINT32 irq = OAL_INTR_IRQ_UNDEFINED;
    UINT32 sysIntr = SYSINTR_NOP;
    UINT32 data, mask;
    
    // Get pending interrupt
    irq = INREG32(&g_pIntcRegs->ulINTC_SIR_IRQ);

    // Check if this is timer IRQ
    if (irq == g_oalTimerIrq) {
        // Call timer interrupt handler
        sysIntr = OALTimerIntrHandler();
        // We are done with interrupt
        OALIntrDoneIrqs(1, &irq);

    } 
	else if (irq != OAL_INTR_IRQ_UNDEFINED) 
	{
#ifdef OAL_ILTIMING
    if (g_oalILT.active) g_oalILT.interrupts++;
#endif

        if (irq == IRQ_GPIO1_MPU) {
			OALMSG(OAL_INTR, (L"OEMInterruptHandler(Irq GPIO1)\r\n"));
			irq = IRQ_GPIO_0;
            OUTREG32(&g_pIntcRegs->ulINTC_MIR_SET0, 1 << IRQ_GPIO1_MPU);
			// If it is interrupt from GPIOx, find which one, mask and clear it..
			if ((data = INREG32(&g_pGPIORegs[0]->ulGPIO_IRQSTATUS1)) != 0) {
				for (mask = 1; mask != 0; mask <<= 1, irq++) {
					if ((mask & data) != 0) break;
				}
				OUTREG32(&g_pGPIORegs[0]->ulGPIO_CLEARIRQENABLE1, mask);  // mask IRQ
				OALMSG(OAL_INTR, (L"GPIO Interrupt irq - %d\r\n",irq ));
			}
        } else if (irq == IRQ_GPIO2_MPU) {
			OALMSG(OAL_INTR, (L"OEMInterruptHandler(Irq GPIO2)\r\n"));
			irq = IRQ_GPIO_32;
            OUTREG32(&g_pIntcRegs->ulINTC_MIR_SET0, 1 << IRQ_GPIO2_MPU);
			// If it is interrupt from GPIOx, find which one, mask  it..
			if ((data = INREG32(&g_pGPIORegs[1]->ulGPIO_IRQSTATUS1)) != 0) {
				for (mask = 1; mask != 0; mask <<= 1, irq++) {
					if ((mask & data) != 0) break;
				}
				OUTREG32(&g_pGPIORegs[1]->ulGPIO_CLEARIRQENABLE1, mask);  // mask IRQ
				OALMSG(OAL_INTR, (L"GPIO Interrupt irq - %d\r\n",irq ));
			}
        } else if (irq == IRQ_GPIO3_MPU) {
			OALMSG(OAL_INTR, (L"OEMInterruptHandler(Irq GPIO3)\r\n"));
			irq = IRQ_GPIO_64;
            OUTREG32(&g_pIntcRegs->ulINTC_MIR_SET0, 1 << IRQ_GPIO3_MPU);
			// If it is interrupt from GPIOx, find which one, mask  it..
			if ((data = INREG32(&g_pGPIORegs[2]->ulGPIO_IRQSTATUS1)) != 0) {
				for (mask = 1; mask != 0; mask <<= 1, irq++) {
					if ((mask & data) != 0) break;
				}
				OUTREG32(&g_pGPIORegs[2]->ulGPIO_CLEARIRQENABLE1, mask);  // mask IRQ
				OALMSG(OAL_INTR, (L"GPIO Interrupt irq - %d\r\n",irq ));
			}
        } else if (irq == IRQ_GPIO4_MPU) {
			OALMSG(OAL_INTR, (L"OEMInterruptHandler(Irq GPIO4)\r\n"));
			irq = IRQ_GPIO_96;
            OUTREG32(&g_pIntcRegs->ulINTC_MIR_SET1, 1 << (IRQ_GPIO4_MPU - 32));
			// If it is interrupt from GPIOx, find which one, mask it..
			if ((data = INREG32(&g_pGPIORegs[3]->ulGPIO_IRQSTATUS1)) != 0) {
				for (mask = 1; mask != 0; mask <<= 1, irq++) {
					if ((mask & data) != 0) break;
				}
				OUTREG32(&g_pGPIORegs[3]->ulGPIO_CLEARIRQENABLE1, mask);  // mask IRQ
				OALMSG(OAL_INTR, (L"GPIO Interrupt irq - %d\r\n",irq ));
			}
        } else if (irq < 32) {
			OALMSG(OAL_INTR, (L"OEMInterruptHandler(Irq %d)\r\n", irq));
			SETPORT32(&g_pIntcRegs->ulINTC_MIR0, 1 << irq);
		} else if (irq < 64) {
			OALMSG(OAL_INTR, (L"OEMInterruptHandler(Irq %d)\r\n", irq));
			SETPORT32(&g_pIntcRegs->ulINTC_MIR1, 1 << (irq - 32));
		} else if (irq < 96) {
			OALMSG(OAL_INTR, (L"OEMInterruptHandler(Irq %d)\r\n", irq));
			SETPORT32(&g_pIntcRegs->ulINTC_MIR2, 1 << (irq - 64));
		} else {
			OALMSG(OAL_INTR, (L"OEMInterruptHandler(Irq UNKNOWNNON %d)\r\n", irq));
		}
	
        // We don't assume IRQ sharing, use static mapping
        sysIntr = OALIntrTranslateIrq(irq);
        OALMSG(OAL_INTR, (L"OALIntrTranslateIrq(%d)=%d\r\n",irq,sysIntr));
    }
	else  
	{
        OALMSG( 1, (L"undefined IRQ interrupt - %d\r\n",irq));
    }
    //Acknowledge the existing IRQ and set the INTC for receiving new IRQ interrupts
    SETREG32(&g_pIntcRegs->ulINTC_CONTROL,1);

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
        rc = INREG32(&g_pIntcRegs->ulINTC_ITR0) & (1 << irq);
    } else if (irq < 64) {
        rc = INREG32(&g_pIntcRegs->ulINTC_ITR1) & (1 << (irq - 32));
    } else if (irq < 96) {
        rc = INREG32(&g_pIntcRegs->ulINTC_ITR2) & (1 << (irq - 64));
    } 
    return (rc != 0);
}

//------------------------------------------------------------------------------

