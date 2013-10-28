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
//
//------------------------------------------------------------------------------
//
//  File: interrupt.c
//
//  This file implement major part of interrupt module for S3C6410X SoC.
//
#include <windows.h>
#include <ceddk.h>
#include <Nkintr.h>
#include <oal.h>

// Base Definitions
#include "s3c6410_base_regs.h"

// SoC Components
#include "s3c6410_dma.h"
#include "s3c6410_gpio.h"
#include "s3c6410_intr.h"
#include "s3c6410_vintr.h"
#include "s3c6410_system.h"

#include <intr.h>

#define VIC_MASK
#define HEART_BEAT_DURATION     (1000)   // Ticks

static const UINT32 g_VirIrq2PhyIrq[IRQ_MAX_S3C6410] =
{
    PHYIRQ_EINT0,       // 0    // 0
    PHYIRQ_EINT0,       // 1    // 0
    PHYIRQ_EINT0,       // 2    // 0
    PHYIRQ_EINT0,       // 3    // 0
    PHYIRQ_EINT1,       // 4    // 1
    PHYIRQ_EINT1,       // 5    // 1
    PHYIRQ_EINT1,       // 6    // 1
    PHYIRQ_EINT1,       // 7    // 1
    PHYIRQ_EINT1,       // 8    // 1
    PHYIRQ_EINT1,       // 9    // 1

    PHYIRQ_EINT1,       // 10 // 1
    PHYIRQ_EINT1,       // 11 // 1
    PHYIRQ_RTC_TIC,     // 12 // 2
    PHYIRQ_CAMIF_C,     // 13 // 3
    PHYIRQ_CAMIF_P,     // 14 // 4

    PHYIRQ_I2C1,        // 15 // 5
    PHYIRQ_I2S_V40,     // 16 // 6
    PHYIRQ_SSS,         // 17 // 7
    PHYIRQ_3D,          // 18 // 8
    
    PHYIRQ_POST,        // 19 // 9

    PHYIRQ_ROTATOR,     // 20 // 10
    PHYIRQ_2D,          // 21 // 11
    PHYIRQ_TVENC,       // 22 // 12
    PHYIRQ_TVSCALER,    // 23 // 13
    PHYIRQ_BATF,        // 24 // 14
    PHYIRQ_JPEG,        // 25 // 15
    PHYIRQ_MFC,         // 26 // 16
    PHYIRQ_SDMA0,       // 27 // 17
    PHYIRQ_SDMA1,       // 28 // 18
    PHYIRQ_ARM_DMAERR,  // 29 // 19

    PHYIRQ_ARM_DMA,     // 30 // 20
    PHYIRQ_ARM_DMAS,    // 31 // 21
    PHYIRQ_KEYPAD,      // 32 // 22
    PHYIRQ_TIMER0,      // 33 // 23
    PHYIRQ_TIMER1,      // 34 // 24
    PHYIRQ_TIMER2,      // 35 // 25
    PHYIRQ_WDT,         // 36 // 26
    PHYIRQ_TIMER3,      // 37 // 27
    PHYIRQ_TIMER4,      // 38 // 28
    PHYIRQ_LCD0_FIFO,   // 39 // 29

    PHYIRQ_LCD1_FRAME,  // 40 // 30
    PHYIRQ_LCD2_SYSIF,  // 41 // 31
    PHYIRQ_EINT2,       // 42 // 32
    PHYIRQ_EINT2,       // 43 // 32
    PHYIRQ_EINT2,       // 44 // 32
    PHYIRQ_EINT2,       // 45 // 32
    PHYIRQ_EINT2,       // 46 // 32
    PHYIRQ_EINT2,       // 47 // 32
    PHYIRQ_EINT2,       // 48 // 32
    PHYIRQ_EINT2,       // 49 // 32

    PHYIRQ_EINT3,       // 50 // 33
    PHYIRQ_EINT3,       // 51 // 33
    PHYIRQ_EINT3,       // 52 // 33
    PHYIRQ_EINT3,       // 53 // 33
    PHYIRQ_EINT3,       // 54 // 33
    PHYIRQ_EINT3,       // 55 // 33
    PHYIRQ_EINT3,       // 56 // 33
    PHYIRQ_EINT3,       // 57 // 33
    PHYIRQ_PCM0,        // 58 // 34
    PHYIRQ_PCM1,        // 59 // 35

    PHYIRQ_AC97,        // 60 // 36
    PHYIRQ_UART0,       // 61 // 37
    PHYIRQ_UART1,       // 62 // 38
    PHYIRQ_UART2,       // 63 // 39
    PHYIRQ_UART3,       // 64 // 40
    PHYIRQ_DMA0,        // 65 // 41
    PHYIRQ_DMA0,        // 66 // 41
    PHYIRQ_DMA0,        // 67 // 41
    PHYIRQ_DMA0,        // 68 // 41
    PHYIRQ_DMA0,        // 69 // 41

    PHYIRQ_DMA0,        // 70 // 41
    PHYIRQ_DMA0,        // 71 // 41
    PHYIRQ_DMA0,        // 72 // 41
    PHYIRQ_DMA1,        // 73 // 42
    PHYIRQ_DMA1,        // 74 // 42
    PHYIRQ_DMA1,        // 75 // 42
    PHYIRQ_DMA1,        // 76 // 42
    PHYIRQ_DMA1,        // 77 // 42
    PHYIRQ_DMA1,        // 78 // 42
    PHYIRQ_DMA1,        // 79 // 42

    PHYIRQ_DMA1,        // 80 // 42
    PHYIRQ_ONENAND0,    // 81 // 43
    PHYIRQ_ONENAND1,    // 82 // 44
    PHYIRQ_NFC,         // 83 // 45
    PHYIRQ_CFC,         // 84 // 46
    PHYIRQ_UHOST,       // 85 // 47
    PHYIRQ_SPI0,        // 86 // 48
    PHYIRQ_SPI1,        // 87 // 49
    PHYIRQ_I2C,         // 88 // 50
    PHYIRQ_HSITX,       // 89 // 51

    PHYIRQ_HSIRX,       // 90 // 52
    PHYIRQ_RESERVED,    // 91 // 53
    PHYIRQ_MSM,         // 92 // 54
    PHYIRQ_HOSTIF,      // 93 // 55
    PHYIRQ_HSMMC0,      // 94 // 56
    PHYIRQ_HSMMC1,      // 95 // 57
    PHYIRQ_OTG,         // 96 // 58
    PHYIRQ_IRDA,        // 97 // 59
    PHYIRQ_RTC_ALARM,   // 98 // 60
    PHYIRQ_SEC,         // 99 // 61

    PHYIRQ_PENDN,       // 100 // 62
    PHYIRQ_ADC          // 101 // 63
};

static const UINT32 g_PhyIrq2VirIrq[PHYIRQ_MAX_S3C6410] =
{
    // VIC0
    IRQ_EINT0,          // 0 (IRQ_EINT0~IRQ_EINT3)
    IRQ_EINT4,          // 1 (IRQ_EINT4~IRQ_EINT11)
    IRQ_RTC_TIC,        // 2
    IRQ_CAMIF_C,        // 3
    IRQ_CAMIF_P,        // 4

    IRQ_I2C1,           // 5
    IRQ_I2S_V40,        // 6
    IRQ_SSS,            // 7
    IRQ_3D,             // 8
    
    IRQ_POST,           // 9
    IRQ_ROTATOR,        // 10
    IRQ_2D,             // 11
    IRQ_TVENC,          // 12
    IRQ_TVSCALER,       // 13
    IRQ_BATF,           // 14
    IRQ_JPEG,           // 15
    IRQ_MFC,            // 16
    IRQ_SDMA0,          // 17
    IRQ_SDMA1,          // 18
    IRQ_ARM_DMAERR,     // 19
    IRQ_ARM_DMA,        // 20
    IRQ_ARM_DMAS,       // 21
    IRQ_KEYPAD,         // 22
    IRQ_TIMER0,         // 23
    IRQ_TIMER1,         // 24
    IRQ_TIMER2,         // 25
    IRQ_WDT,            // 26
    IRQ_TIMER3,         // 27
    IRQ_TIMER4,         // 28
    IRQ_LCD0_FIFO,      // 29
    IRQ_LCD1_FRAME,     // 30
    IRQ_LCD2_SYSIF,     // 31

    // VIC1
    IRQ_EINT12,         // 32 (IRQ_EINT12~IRQ_EINT19)
    IRQ_EINT20,         // 33 (IRQ_EINT20~IRQ_EINT27)
    IRQ_PCM0,           // 34
    IRQ_PCM1,           // 35
    IRQ_AC97,           // 36
    IRQ_UART0,          // 37
    IRQ_UART1,          // 38
    IRQ_UART2,          // 39
    IRQ_UART3,          // 40
    IRQ_DMA0_CH0,       // 41 (IRQ_DMA0_CH0~IRQ_DMA0_CH7)
    IRQ_DMA1_CH0,       // 42 (IRQ_DMA1_CH0~IRQ_DMA1_CH7)
    IRQ_ONENAND0,       // 43
    IRQ_ONENAND1,       // 44
    IRQ_NFC,            // 45
    IRQ_CFC,            // 46
    IRQ_UHOST,          // 47
    IRQ_SPI0,           // 48
    IRQ_SPI1,           // 49
    IRQ_I2C,            // 50
    IRQ_HSITX,          // 51
    IRQ_HSIRX,          // 52
    IRQ_RESERVED,       // 53
    IRQ_MSM    ,        // 54
    IRQ_HOSTIF,         // 55
    IRQ_HSMMC0,         // 56
    IRQ_HSMMC1,         // 57
    IRQ_OTG,            // 58
    IRQ_IRDA,           // 59
    IRQ_RTC_ALARM,      // 60
    IRQ_SEC,            // 61
    IRQ_PENDN,          // 62
    IRQ_ADC             // 63
};

//------------------------------------------------------------------------------
//
//  Globals:  g_pIntrRegs/g_pPortRegs
//
//  The global variables are storing virual address for interrupt and port
//  registers for use in interrupt handling to avoid possible time consumig
//  call to OALPAtoVA function.
//
static volatile S3C6410_VIC_REG *g_pVIC0Reg;
static volatile S3C6410_VIC_REG *g_pVIC1Reg;
static volatile S3C6410_GPIO_REG *g_pGPIOReg;
static volatile S3C6410_DMAC_REG *g_pDMAC0Reg;
static volatile S3C6410_DMAC_REG *g_pDMAC1Reg;

//  Function pointer to profiling timer ISR routine.
//
PFN_PROFILER_ISR g_pProfilerISR = NULL;

static void PrepareEINTIntr(void);
static void PrepareDMACIntr(void);
static void InitializeVIC(void);
void VICTableInit(void);                    // Reference by OEMPowerOff() in "Off.c"

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

    OALMSG( OAL_FUNC&&OAL_INTR, (L"+OALInterruptInit\r\n") );

    // First get uncached virtual addresses
    g_pVIC0Reg = (S3C6410_VIC_REG *)OALPAtoVA(S3C6410_BASE_REG_PA_VIC0, FALSE);
    g_pVIC1Reg = (S3C6410_VIC_REG *)OALPAtoVA(S3C6410_BASE_REG_PA_VIC1, FALSE);
    g_pGPIOReg = (S3C6410_GPIO_REG*)OALPAtoVA(S3C6410_BASE_REG_PA_GPIO, FALSE);
    g_pDMAC0Reg = (S3C6410_DMAC_REG*)OALPAtoVA(S3C6410_BASE_REG_PA_DMA0, FALSE);
    g_pDMAC1Reg = (S3C6410_DMAC_REG*)OALPAtoVA(S3C6410_BASE_REG_PA_DMA1, FALSE);

    // Initialize interrupt mapping
    OALIntrMapInit();

    PrepareDMACIntr();

    // Initialize VIC
    InitializeVIC();

    PrepareEINTIntr();

#ifdef OAL_BSP_CALLBACKS
    // Give BSP change to initialize subordinate controller
    rc = BSPIntrInit();
#else
    rc = TRUE;
#endif

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

    OALMSG(OAL_INTR&&OAL_FUNC, (L"+OALIntrRequestIrqs(0x%08x->%d/%d/0x%08x/%d, 0x%08x, 0x%08x)\r\n",
                pDevLoc, pDevLoc->IfcType, pDevLoc->BusNumber, pDevLoc->LogicalLoc,
                pDevLoc->Pin, pCount, pIrqs));

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
//  Function:  OALIntrEnableIrqs
//
BOOL OALIntrEnableIrqs(UINT32 count, const UINT32 *pIrqs)
{
    BOOL bRet = TRUE;
    UINT32 VirtualIRQ;
    UINT32 PhysicalIRQ;
    UINT32 i;
            
    OALMSG(OAL_INTR&&OAL_FUNC, (L"+OALIntrEnableIrqs(%d, 0x%08x)\r\n", count, pIrqs));

    for (i = 0; i < count; i++)
    {
#ifndef OAL_BSP_CALLBACKS
        VirtualIRQ = pIrqs[i];
#else
        // Give BSP chance to enable irq on subordinate interrupt controller
        VirtualIRQ = BSPIntrEnableIrq(pIrqs[i]);
#endif

        if (VirtualIRQ == OAL_INTR_IRQ_UNDEFINED) continue;

        // Translate to Physical IRQ
        PhysicalIRQ = g_VirIrq2PhyIrq[VirtualIRQ];

        if (PhysicalIRQ == PHYIRQ_EINT0 ||  // IRQ_EINT0 ~ IRQ_EINT3
            PhysicalIRQ == PHYIRQ_EINT1 )   // IRQ_EINT4 ~ IRQ_EINT11
        {
            g_pGPIOReg->EINT0MASK &= ~(1<<VirtualIRQ);   // Enable Sub Interrupt
            g_pGPIOReg->EINT0PEND = (1<<VirtualIRQ);     // Clear Sub Pending
        }
        else if (PhysicalIRQ == PHYIRQ_EINT2 || // IRQ_EINT12 ~ IRQ_EINT19
                 PhysicalIRQ == PHYIRQ_EINT3 )  // IRQ_EINT20 ~ IRQ_EINT27
        {
            g_pGPIOReg->EINT0MASK &= ~(1<<(VirtualIRQ-30));   // Enable Sub Interrupt
            g_pGPIOReg->EINT0PEND = (1<<(VirtualIRQ-30));     // Clear Sub Pending
        }
        else if (PhysicalIRQ < VIC1_BIT_OFFSET)
        {
            g_pVIC0Reg->VICINTENABLE = (0x1<<PhysicalIRQ);
        }
        else if (PhysicalIRQ < PHYIRQ_MAX_S3C6410)
        {
            g_pVIC1Reg->VICINTENABLE = (0x1<<(PhysicalIRQ-VIC1_BIT_OFFSET));
        }
        else
        {
            bRet = FALSE;
        }
    }

    OALMSG(OAL_INTR&&OAL_FUNC, (L"-OALIntrEnableIrqs(rc = %d)\r\n", bRet));

    return bRet;
}

//------------------------------------------------------------------------------
//
//  Function:  OALIntrDisableIrqs
//
VOID OALIntrDisableIrqs(UINT32 count, const UINT32 *pIrqs)
{
    UINT32 VirtualIRQ;
    UINT32 PhysicalIRQ;
    UINT32 i;

    OALMSG(OAL_INTR&&OAL_FUNC, (L"+OALIntrDisableIrqs(%d, 0x%08x)\r\n", count, pIrqs));

    for (i = 0; i < count; i++)
    {
#ifndef OAL_BSP_CALLBACKS
        VirtualIRQ = pIrqs[i];
#else
        // Give BSP chance to disable irq on subordinate interrupt controller
        VirtualIRQ = BSPIntrDisableIrq(pIrqs[i]);
#endif
        if (VirtualIRQ == OAL_INTR_IRQ_UNDEFINED) continue;

        // Translate to Physical IRQ
        PhysicalIRQ = g_VirIrq2PhyIrq[VirtualIRQ];

        if (PhysicalIRQ == PHYIRQ_EINT0 ||  // IRQ_EINT0 ~ IRQ_EINT3
            PhysicalIRQ == PHYIRQ_EINT1 )   // IRQ_EINT4 ~ IRQ_EINT11
        {
            g_pGPIOReg->EINT0MASK |= (1<<VirtualIRQ);    // Mask Sub Interrupt
            g_pGPIOReg->EINT0PEND = (1<<VirtualIRQ);     // Clear Sub Pending

            // Do not Mask PHYIRQ_EINT0/1 Interrupt !!!
        }
        else if (PhysicalIRQ == PHYIRQ_EINT2 || // IRQ_EINT12 ~ IRQ_EINT19
                 PhysicalIRQ == PHYIRQ_EINT3 )  // IRQ_EINT20 ~ IRQ_EINT27
        {
            
            g_pGPIOReg->EINT0MASK |= (1<<(VirtualIRQ-30));    // Mask Sub Interrupt
            g_pGPIOReg->EINT0PEND = (1<<(VirtualIRQ-30));     // Clear Sub Pending

            // Do not Mask PHYIRQ_EINT2/3 Interrupt !!!
        }
        else if (PhysicalIRQ == PHYIRQ_DMA0)
        {
            // Do not Mask PHYIRQ_DMA0 Interrupt !!!
        }
        else if (PhysicalIRQ == PHYIRQ_DMA1)
        {
            // Do not Mask PHYIRQ_DMA1 Interrupt !!!
        }
        else if (PhysicalIRQ < VIC1_BIT_OFFSET)
        {
            g_pVIC0Reg->VICINTENCLEAR = (0x1<<PhysicalIRQ);
        }
        else if (PhysicalIRQ < PHYIRQ_MAX_S3C6410)
        {
            g_pVIC1Reg->VICINTENCLEAR = (0x1<<(PhysicalIRQ-VIC1_BIT_OFFSET));
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
    UINT32 VirtualIRQ;
    UINT32 PhysicalIRQ;
    UINT32 i;

    OALMSG(OAL_INTR&&OAL_VERBOSE, (L"+OALIntrDoneIrqs(%d, 0x%08x)\r\n", count, pIrqs));

    for (i = 0; i < count; i++)
    {
#ifndef OAL_BSP_CALLBACKS
        VirtualIRQ = pIrqs[i];
#else
        // Give BSP chance to finish irq on subordinate interrupt controller
        VirtualIRQ = BSPIntrDoneIrq(pIrqs[i]);
#endif

        // Translate to Physical IRQ
        PhysicalIRQ = g_VirIrq2PhyIrq[VirtualIRQ];

        if (PhysicalIRQ == PHYIRQ_EINT0 ||  // IRQ_EINT0 ~ IRQ_EINT3
            PhysicalIRQ == PHYIRQ_EINT1)    // IRQ_EINT4 ~ IRQ_EINT11
        {
            g_pGPIOReg->EINT0MASK &= ~(1<<VirtualIRQ);   // Enable Sub Interrupt
        }
        else if (PhysicalIRQ == PHYIRQ_EINT2 || // IRQ_EINT12 ~ IRQ_EINT19
                 PhysicalIRQ == PHYIRQ_EINT3 )  // IRQ_EINT20 ~ IRQ_EINT27
        {
            g_pGPIOReg->EINT0MASK &= ~(1<<(VirtualIRQ-30));   // Enable Sub Interrupt
        }
        else if (PhysicalIRQ < VIC1_BIT_OFFSET)
        {
            g_pVIC0Reg->VICINTENABLE = (0x1<<PhysicalIRQ);
        }
        else if (PhysicalIRQ < PHYIRQ_MAX_S3C6410)
        {
            g_pVIC1Reg->VICINTENABLE = (0x1<<(PhysicalIRQ-VIC1_BIT_OFFSET));
        }
    }

    OALMSG(OAL_INTR&&OAL_VERBOSE, (L"-OALIntrDoneIrqs\r\n"));
}

//------------------------------------------------------------------------------
//
//  Function:  OEMInterruptHandler
//
ULONG OEMInterruptHandler(ULONG ra)
{
    UINT32 SysIntr = SYSINTR_NOP;
    UINT32 PhysicalIRQ;
    UINT32 VirtualIRQ;
    UINT32 uiPending;
    UINT32 uiMask;
    int nNumber;
#ifdef    VIC_MASK
    UINT32 IntEnVIC0, IntEnVIC1;
#endif

    static DWORD HeartBeatCnt, HeartBeatStat;  //LED4 is used for heart beat

    // Get Pending Interrupt Number
    PhysicalIRQ = g_pVIC1Reg->VICADDRESS;    // Ready Dummy from VIC1
    PhysicalIRQ = g_pVIC0Reg->VICADDRESS;

#ifdef    VIC_MASK
    // To Avoid low proirity interrupt lost
    IntEnVIC0 = g_pVIC0Reg->VICINTENABLE;
    IntEnVIC1 = g_pVIC1Reg->VICINTENABLE;
    g_pVIC0Reg->VICINTENCLEAR = 0xffffffff;
    g_pVIC1Reg->VICINTENCLEAR = 0xffffffff;
#endif

    // Translate Physical IRQ to Virtual IRQ (Except Flatten IRQ)
    VirtualIRQ = g_PhyIrq2VirIrq[PhysicalIRQ];

    if (PhysicalIRQ == PHYIRQ_TIMER4)
    {
        //-------------
        // System Timer
        //-------------

        // Heart Beat LED
        if (++HeartBeatCnt >= HEART_BEAT_DURATION)
        {
            HeartBeatCnt = 0;
            HeartBeatStat ^= 1;

            if (HeartBeatStat)
            {
                OEMWriteDebugLED(-1, MAKELONG(0x1, 0x1));
            }
            else
            {
                OEMWriteDebugLED(-1, MAKELONG(0x0, 0x1));
            }
        }

        if (g_pVIC0Reg->VICRAWINTR & (1<<PHYIRQ_TIMER4))
        {
            // Handling System Timer Interrupt
            SysIntr = OALTimerIntrHandler();
        }
        else
        {
            OALMSG(TRUE, (L"[OAL:ERR] PHYIRQ_TIMER4 Interrupt But There is No Pending !!!!\r\n"));
        }

    }
    else if (PhysicalIRQ == PHYIRQ_TIMER2)
    {
        //-------------
        // Profiling Timer
        //-------------

        // Masking Interrupt
#ifdef    VIC_MASK
        IntEnVIC0 &= ~(1<<PHYIRQ_TIMER2);
#else
        g_pVIC0Reg->VICINTENCLEAR = (1<<PHYIRQ_TIMER2);
#endif

        // Handling Profiling Timer Interrupt
        if (g_pProfilerISR)
        {
            SysIntr = g_pProfilerISR(ra);
        }
    }
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

        if (PhysicalIRQ == PHYIRQ_EINT0)
        {
            // Do not mask PHYIRQ_EINT0 interrupt for other sub interrupt
            // Mask EINTMASK and EINTPEND which occured this time
            // So each IST of EINT should unmask their EINT for Next Interrupt

            // Check Sub Source and Mask
            uiPending = g_pGPIOReg->EINT0PEND;
            uiMask = g_pGPIOReg->EINT0MASK;

            for (nNumber=0; nNumber<4; nNumber++)    // EINT0~EINT3
            {
                if ((uiPending & (1<<nNumber)) && !(uiMask & (1<<nNumber)))
                {
                    g_pGPIOReg->EINT0MASK |= (1<<nNumber);    // Mask Sub Interrupt
                    g_pGPIOReg->EINT0PEND = (1<<nNumber);        // Clear Sub Pending
                    VirtualIRQ = (IRQ_EINT0+nNumber);                // Set Virtual IRQ
                    break;
                }
            }
        }
        else if (PhysicalIRQ == PHYIRQ_EINT1)
        {
            // Do not mask PHYIRQ_EINT1 interrupt for other sub interrupt
            // Mask EINTMASK and EINTPEND which occured this time
            // So each IST of EINT should unmask their EINT for Next Interrupt

            // Check Sub Source
            uiPending = g_pGPIOReg->EINT0PEND;
            uiMask = g_pGPIOReg->EINT0MASK;

            for (nNumber=4; nNumber<12; nNumber++)    // EINT4~EINT11
            {
                if ((uiPending & (1<<nNumber)) && !(uiMask & (1<<nNumber)))
                {
                    g_pGPIOReg->EINT0MASK |= (1<<nNumber);    // Mask Sub Interrupt
                    g_pGPIOReg->EINT0PEND = (1<<nNumber);        // Clear Sub Pending
                    VirtualIRQ = (IRQ_EINT4+(nNumber-4));            // Set Virtual IRQ
                    break;
                }
            }
        }
        else if (PhysicalIRQ == PHYIRQ_EINT2)
        {
            // Do not mask PHYIRQ_EINT2 interrupt for other sub interrupt
            // Mask EINTMASK and EINTPEND which occured this time
            // So each IST of EINT should unmask their EINT for Next Interrupt

            // Check Sub Source
            uiPending = g_pGPIOReg->EINT0PEND;
            uiMask = g_pGPIOReg->EINT0MASK;

            for (nNumber=12; nNumber<20; nNumber++)    // EINT12~EINT19
            {
                if ((uiPending & (1<<nNumber)) && !(uiMask & (1<<nNumber)))
                {
                    g_pGPIOReg->EINT0MASK |= (1<<nNumber);    // Mask Sub Interrupt
                    g_pGPIOReg->EINT0PEND = (1<<nNumber);        // Clear Sub Pending
                    VirtualIRQ = (IRQ_EINT12+(nNumber-12));        // Set Virtual IRQ
                    break;
                }
            }
        }
        else if (PhysicalIRQ == PHYIRQ_EINT3)
        {
            // Do not mask PHYIRQ_EINT3 interrupt for other sub interrupt
            // Mask EINTMASK and EINTPEND which occured this time
            // So each IST of EINT should unmask their EINT for Next Interrupt

            // Check Sub Source
            uiPending = g_pGPIOReg->EINT0PEND;
            uiMask = g_pGPIOReg->EINT0MASK;

            for (nNumber=20; nNumber<28; nNumber++)    // EINT20~EINT27
            {
                if ((uiPending & (1<<nNumber)) && !(uiMask & (1<<nNumber)))
                {
                    g_pGPIOReg->EINT0MASK |= (1<<nNumber);    // Mask Sub Interrupt
                    g_pGPIOReg->EINT0PEND = (1<<nNumber);        // Clear Sub Pending
                    VirtualIRQ = (IRQ_EINT20+(nNumber-20));        // Set Virtual IRQ
                    break;
                }
            }
        }
        else if (PhysicalIRQ == PHYIRQ_DMA0)
        {
            DWORD dwIntStatus;

#ifdef    VIC_MASK
            IntEnVIC1 &= ~(1<<(PHYIRQ_DMA0-VIC1_BIT_OFFSET));
#else
            g_pVIC1Reg->VICINTENCLEAR = (1<<(PHYIRQ_DMA0-VIC1_BIT_OFFSET));
#endif

            dwIntStatus = g_pDMAC0Reg->DMACIntStatus;

            if (dwIntStatus & 0x01) VirtualIRQ = IRQ_DMA0_CH0;        // channel 0
            else if (dwIntStatus & 0x02) VirtualIRQ = IRQ_DMA0_CH1;    // channel 1
            else if (dwIntStatus & 0x04) VirtualIRQ = IRQ_DMA0_CH2;    // channel 2
            else if (dwIntStatus & 0x08) VirtualIRQ = IRQ_DMA0_CH3;    // channel 3
            else if (dwIntStatus & 0x10) VirtualIRQ = IRQ_DMA0_CH4;    // channel 4
            else if (dwIntStatus & 0x20) VirtualIRQ = IRQ_DMA0_CH5;    // channel 5
            else if (dwIntStatus & 0x40) VirtualIRQ = IRQ_DMA0_CH6;    // channel 6
            else if (dwIntStatus & 0x80) VirtualIRQ = IRQ_DMA0_CH7;    // channel 7
        }
        else if (PhysicalIRQ == PHYIRQ_DMA1)
        {
            DWORD dwIntStatus;

#ifdef    VIC_MASK
            IntEnVIC1 &= ~(1<<(PHYIRQ_DMA1-VIC1_BIT_OFFSET));
#else
            g_pVIC1Reg->VICINTENCLEAR = (1<<(PHYIRQ_DMA1-VIC1_BIT_OFFSET));
#endif

            dwIntStatus = g_pDMAC1Reg->DMACIntStatus;

            if (dwIntStatus & 0x01) VirtualIRQ = IRQ_DMA1_CH0;        // channel 0
            else if (dwIntStatus & 0x02) VirtualIRQ = IRQ_DMA1_CH1;    // channel 1
            else if (dwIntStatus & 0x04) VirtualIRQ = IRQ_DMA1_CH2;    // channel 2
            else if (dwIntStatus & 0x08) VirtualIRQ = IRQ_DMA1_CH3;    // channel 3
            else if (dwIntStatus & 0x10) VirtualIRQ = IRQ_DMA1_CH4;    // channel 4
            else if (dwIntStatus & 0x20) VirtualIRQ = IRQ_DMA1_CH5;    // channel 5
            else if (dwIntStatus & 0x40) VirtualIRQ = IRQ_DMA1_CH6;    // channel 6
            else if (dwIntStatus & 0x80) VirtualIRQ = IRQ_DMA1_CH7;    // channel 7
        }
        else if (PhysicalIRQ < VIC1_BIT_OFFSET)
        {
#ifdef    VIC_MASK
            IntEnVIC0 &= ~(1<<PhysicalIRQ);
#else
            g_pVIC0Reg->VICINTENCLEAR = (1<<PhysicalIRQ);
#endif
        }
        else if (PhysicalIRQ < PHYIRQ_MAX_S3C6410)
        {
#ifdef    VIC_MASK
            IntEnVIC1 &= ~(1<<(PhysicalIRQ-VIC1_BIT_OFFSET));
#else
            g_pVIC1Reg->VICINTENCLEAR = (1<<(PhysicalIRQ-VIC1_BIT_OFFSET));
#endif
        }

        // We don't recommend using IRQ sharing and IISR.
        // To use static mapping from design time is recommended for device driver developer.
        // The IISR can affect whole system's performance
        // and it makes interrupt handling and debugging harder.
        
        // First find if IRQ is claimed by chain
        SysIntr = NKCallIntChain((BYTE)VirtualIRQ);

        if (SysIntr == SYSINTR_CHAIN || !NKIsSysIntrValid(SysIntr))
        {
            // IRQ wasn't claimed, use static mapping
            SysIntr = OALIntrTranslateIrq(VirtualIRQ);
        }
    }

    // Clear Vector Address Register
    g_pVIC0Reg->VICADDRESS = 0x0;
    g_pVIC1Reg->VICADDRESS = 0x0;

#ifdef    VIC_MASK
    // To Avoid low proirity interrupt lost
    g_pVIC0Reg->VICINTENABLE = IntEnVIC0;
    g_pVIC1Reg->VICINTENABLE = IntEnVIC1;
#endif

    return SysIntr;
}

static void PrepareEINTIntr(void)
{
    g_pGPIOReg->EINT0MASK = 0xFFFFFFFF;    // Mask All Sub Interrupts
    g_pGPIOReg->EINT0PEND = 0xFFFFFFFF;    // Clear All Sub Pending Interrupts

    g_pVIC0Reg->VICINTENABLE |= (0x1<<PHYIRQ_EINT0); // Enable IRQ_EINT0 ~ IRQ_EINT3
    g_pVIC0Reg->VICINTENABLE |= (0x1<<PHYIRQ_EINT1); // Enable // IRQ_EINT4 ~ IRQ_EINT11
    
    g_pVIC1Reg->VICINTENABLE |= (0x1<<(PHYIRQ_EINT2 - VIC1_BIT_OFFSET)); // Enable IRQ_EINT12 ~ IRQ_EINT19
    g_pVIC1Reg->VICINTENABLE |= (0x1<<(PHYIRQ_EINT3 - VIC1_BIT_OFFSET)); // Enable IRQ_EINT20 ~ IRQ_EINT27
}

static void PrepareDMACIntr(void)
{
    // Disable Interrupt of All Channel
    // Mask TC and Error Interrupt
    // Clear Interrupt Pending

    g_pDMAC0Reg->DMACC0Control0 &= ~(1<<31);        // TCINT Disable
    g_pDMAC0Reg->DMACC0Configuration &= (3<<14);    // TCINT & ErrINT Mask
    g_pDMAC0Reg->DMACC1Control0 &= ~(1<<31);        // TCINT Disable
    g_pDMAC0Reg->DMACC1Configuration &= (3<<14);    // TCINT & ErrINT Mask
    g_pDMAC0Reg->DMACC2Control0 &= ~(1<<31);        // TCINT Disable
    g_pDMAC0Reg->DMACC2Configuration &= (3<<14);    // TCINT & ErrINT Mask
    g_pDMAC0Reg->DMACC3Control0 &= ~(1<<31);        // TCINT Disable
    g_pDMAC0Reg->DMACC3Configuration &= (3<<14);    // TCINT & ErrINT Mask
    g_pDMAC0Reg->DMACC4Control0 &= ~(1<<31);        // TCINT Disable
    g_pDMAC0Reg->DMACC4Configuration &= (3<<14);    // TCINT & ErrINT Mask
    g_pDMAC0Reg->DMACC5Control0 &= ~(1<<31);        // TCINT Disable
    g_pDMAC0Reg->DMACC5Configuration &= (3<<14);    // TCINT & ErrINT Mask
    g_pDMAC0Reg->DMACC6Control0 &= ~(1<<31);        // TCINT Disable
    g_pDMAC0Reg->DMACC6Configuration &= (3<<14);    // TCINT & ErrINT Mask
    g_pDMAC0Reg->DMACC7Control0 &= ~(1<<31);        // TCINT Disable
    g_pDMAC0Reg->DMACC7Configuration &= (3<<14);    // TCINT & ErrINT Mask

    g_pDMAC0Reg->DMACIntTCClear = 0xff;            // TC Int Pending Clear
    g_pDMAC0Reg->DMACIntErrClear = 0xff;            // Err Int Pending Clear

    g_pDMAC1Reg->DMACC0Control0 &= ~(1<<31);        // TCINT Disable
    g_pDMAC1Reg->DMACC0Configuration &= (3<<14);    // TCINT & ErrINT Mask
    g_pDMAC1Reg->DMACC1Control0 &= ~(1<<31);        // TCINT Disable
    g_pDMAC1Reg->DMACC1Configuration &= (3<<14);    // TCINT & ErrINT Mask
    g_pDMAC1Reg->DMACC2Control0 &= ~(1<<31);        // TCINT Disable
    g_pDMAC1Reg->DMACC2Configuration &= (3<<14);    // TCINT & ErrINT Mask
    g_pDMAC1Reg->DMACC3Control0 &= ~(1<<31);        // TCINT Disable
    g_pDMAC1Reg->DMACC3Configuration &= (3<<14);    // TCINT & ErrINT Mask
    g_pDMAC1Reg->DMACC4Control0 &= ~(1<<31);        // TCINT Disable
    g_pDMAC1Reg->DMACC4Configuration &= (3<<14);    // TCINT & ErrINT Mask
    g_pDMAC1Reg->DMACC5Control0 &= ~(1<<31);        // TCINT Disable
    g_pDMAC1Reg->DMACC5Configuration &= (3<<14);    // TCINT & ErrINT Mask
    g_pDMAC1Reg->DMACC6Control0 &= ~(1<<31);        // TCINT Disable
    g_pDMAC1Reg->DMACC6Configuration &= (3<<14);    // TCINT & ErrINT Mask
    g_pDMAC1Reg->DMACC7Control0 &= ~(1<<31);        // TCINT Disable
    g_pDMAC1Reg->DMACC7Configuration &= (3<<14);    // TCINT & ErrINT Mask

    g_pDMAC1Reg->DMACIntTCClear = 0xff;            // TC Int Pending Clear
    g_pDMAC1Reg->DMACIntErrClear = 0xff;            // Err Int Pending Clear
}

static void InitializeVIC(void)
{
    // Disable All Interrupts
    g_pVIC0Reg->VICINTENCLEAR = 0xFFFFFFFF;
    g_pVIC1Reg->VICINTENCLEAR = 0xFFFFFFFF;
    g_pVIC0Reg->VICSOFTINTCLEAR = 0xFFFFFFFF;
    g_pVIC1Reg->VICSOFTINTCLEAR = 0xFFFFFFFF;

    // Clear Current Active Vector Address
    g_pVIC0Reg->VICADDRESS = 0x0;
    g_pVIC1Reg->VICADDRESS = 0x0;

    // Initialize Vector Address Table
    VICTableInit();

    // Disable Vectored Interrupt Mode on CP15
    System_DisableVIC();

    // Enable IRQ Interrupt on CP15
    System_EnableIRQ();

    // Enable FIQ Interrupt on CP15
    System_EnableFIQ();
}

void VICTableInit(void)
{
    // This Function is reference by OEMPowerOff() in "Off.c"
    // Make Sure that Caller and This function is in Same Address Space

    // Fill Vector Address of VIC0
    // Actually, Filled with Physical IRQ Numbers.
    // Because We do not use vectored interrupt feature

    g_pVIC0Reg->VICVECTADDR0 = PHYIRQ_EINT0;
    g_pVIC0Reg->VICVECTADDR1 = PHYIRQ_EINT1;
    g_pVIC0Reg->VICVECTADDR2 = PHYIRQ_RTC_TIC;
    g_pVIC0Reg->VICVECTADDR3 = PHYIRQ_CAMIF_C;
    g_pVIC0Reg->VICVECTADDR4 = PHYIRQ_CAMIF_P;

    g_pVIC0Reg->VICVECTADDR5 = PHYIRQ_I2C1;
    g_pVIC0Reg->VICVECTADDR6 = PHYIRQ_I2S_V40;
    g_pVIC0Reg->VICVECTADDR7 = PHYIRQ_SSS;
    g_pVIC0Reg->VICVECTADDR8 = PHYIRQ_3D;
    
    g_pVIC0Reg->VICVECTADDR9 = PHYIRQ_POST;
    g_pVIC0Reg->VICVECTADDR10 = PHYIRQ_ROTATOR;
    g_pVIC0Reg->VICVECTADDR11 = PHYIRQ_2D;
    g_pVIC0Reg->VICVECTADDR12 = PHYIRQ_TVENC;
    g_pVIC0Reg->VICVECTADDR13 = PHYIRQ_TVSCALER;
    g_pVIC0Reg->VICVECTADDR14 = PHYIRQ_BATF;
    g_pVIC0Reg->VICVECTADDR15 = PHYIRQ_JPEG;
    g_pVIC0Reg->VICVECTADDR16 = PHYIRQ_MFC;
    g_pVIC0Reg->VICVECTADDR17 = PHYIRQ_SDMA0;
    g_pVIC0Reg->VICVECTADDR18 = PHYIRQ_SDMA1;
    g_pVIC0Reg->VICVECTADDR19 = PHYIRQ_ARM_DMAERR;
    g_pVIC0Reg->VICVECTADDR20 = PHYIRQ_ARM_DMA;
    g_pVIC0Reg->VICVECTADDR21 = PHYIRQ_ARM_DMAS;
    g_pVIC0Reg->VICVECTADDR22 = PHYIRQ_KEYPAD;
    g_pVIC0Reg->VICVECTADDR23 = PHYIRQ_TIMER0;
    g_pVIC0Reg->VICVECTADDR24 = PHYIRQ_TIMER1;
    g_pVIC0Reg->VICVECTADDR25 = PHYIRQ_TIMER2;
    g_pVIC0Reg->VICVECTADDR26 = PHYIRQ_WDT;
    g_pVIC0Reg->VICVECTADDR27 = PHYIRQ_TIMER3;
    g_pVIC0Reg->VICVECTADDR28 = PHYIRQ_TIMER4;
    g_pVIC0Reg->VICVECTADDR29 = PHYIRQ_LCD0_FIFO;
    g_pVIC0Reg->VICVECTADDR30 = PHYIRQ_LCD1_FRAME;
    g_pVIC0Reg->VICVECTADDR31 = PHYIRQ_LCD2_SYSIF;

    // Fill Vector Address of VIC1
    g_pVIC1Reg->VICVECTADDR0 = PHYIRQ_EINT2;
    g_pVIC1Reg->VICVECTADDR1 = PHYIRQ_EINT3;
    g_pVIC1Reg->VICVECTADDR2 = PHYIRQ_PCM0;
    g_pVIC1Reg->VICVECTADDR3 = PHYIRQ_PCM1;
    g_pVIC1Reg->VICVECTADDR4 = PHYIRQ_AC97;
    g_pVIC1Reg->VICVECTADDR5 = PHYIRQ_UART0;
    g_pVIC1Reg->VICVECTADDR6 = PHYIRQ_UART1;
    g_pVIC1Reg->VICVECTADDR7 = PHYIRQ_UART2;
    g_pVIC1Reg->VICVECTADDR8 = PHYIRQ_UART3;
    g_pVIC1Reg->VICVECTADDR9 = PHYIRQ_DMA0;
    g_pVIC1Reg->VICVECTADDR10 = PHYIRQ_DMA1;
    g_pVIC1Reg->VICVECTADDR11 = PHYIRQ_ONENAND0;
    g_pVIC1Reg->VICVECTADDR12 = PHYIRQ_ONENAND1;
    g_pVIC1Reg->VICVECTADDR13 = PHYIRQ_NFC;
    g_pVIC1Reg->VICVECTADDR14 = PHYIRQ_CFC;
    g_pVIC1Reg->VICVECTADDR15 = PHYIRQ_UHOST;
    g_pVIC1Reg->VICVECTADDR16 = PHYIRQ_SPI0;
    g_pVIC1Reg->VICVECTADDR17 = PHYIRQ_SPI1;
    g_pVIC1Reg->VICVECTADDR18 = PHYIRQ_I2C;
    g_pVIC1Reg->VICVECTADDR19 = PHYIRQ_HSITX;
    g_pVIC1Reg->VICVECTADDR20 = PHYIRQ_HSIRX;
    g_pVIC1Reg->VICVECTADDR21 = PHYIRQ_RESERVED;
    g_pVIC1Reg->VICVECTADDR22 = PHYIRQ_MSM;
    g_pVIC1Reg->VICVECTADDR23 = PHYIRQ_HOSTIF;
    g_pVIC1Reg->VICVECTADDR24 = PHYIRQ_HSMMC0;
    g_pVIC1Reg->VICVECTADDR25 = PHYIRQ_HSMMC1;
    g_pVIC1Reg->VICVECTADDR26 = PHYIRQ_OTG;
    g_pVIC1Reg->VICVECTADDR27 = PHYIRQ_IRDA;
    g_pVIC1Reg->VICVECTADDR28 = PHYIRQ_RTC_ALARM;
    g_pVIC1Reg->VICVECTADDR29 = PHYIRQ_SEC;
    g_pVIC1Reg->VICVECTADDR30 = PHYIRQ_PENDN;
    g_pVIC1Reg->VICVECTADDR31 = PHYIRQ_ADC;
}

//------------------------------------------------------------------------------

