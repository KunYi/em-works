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
//  This file implement major part of interrupt module for the Intel PXA27x SoC.
//
#include <windows.h>
#include <ceddk.h>
#include <nkintr.h>
#include <nkexport.h>
#include <bulverde.h>
#include "oal_log.h"
#include "oal_memory.h"
#include "oal_io.h"
#include "oal_timer.h"
#include "oal_intr.h"
#include <intr.h>
#include <oal_ilt.h>

//------------------------------------------------------------------------------

// External Functions

// External Variables 
 
// Defines 
#define NUM_INT_PRIOS    32

// Types
 
// Global Variables 
PFN_PROFILER_ISR g_pProfilerISR = NULL;

static volatile BULVERDE_GPIO_REG *g_pGpioRegs  = NULL;
static volatile BULVERDE_INTR_REG *g_pICReg = NULL;
static UINT32   g_IntPriorities[NUM_INT_PRIOS] =
{
    IRQ_OSMR0,         // M0 - Scheduler - ID 26
    IRQ_PMU,           // PMU - ID 12
    IRQ_OSMR2,         // M2 - Profiler - ID 28
    IRQ_OSMR3,         // M3 - DVM - ID 29
    IRQ_GPIO1,         // GPIO1 edge detect - Suspend/Resume - ID 9
    IRQ_RTCALARM,      // RTC Alarm - ID 31
    IRQ_BASEBAND,      // Baseband - ID 1
    IRQ_DMAC,          // DMA Controller - ID 25
    0x00000020,        // Trusted Platform Module (Caddo) - ID 32
    0x00000021,        // Camera Capture - ID 33
    IRQ_USBFN,         // UDC - ID 11
    IRQ_USBOHCI,       // USB Host (OHCI) - ID 3
    IRQ_USBNONOHCI,    // USB Host Non-OHCI - ID 2
    IRQ_OSMRXX_4,      // M4-M11 - ID 7
    IRQ_FFUART,        // FFUART - ID 22
    IRQ_STUART,        // STDUART - ID 20
    IRQ_BTUART,        // BTUART - ID 21
    IRQ_AC97,          // AC97 - UCB1400 - ID 14
    IRQ_OSMR1,         // M1 - Touch Timer (Draw) - ID 27
    IRQ_GPIO0,         // GPIO0 - FPGA - ID 8
    IRQ_MMC,           // MMC - ID 23
    IRQ_MEMSTICK,      // Memstick - ID 5
    IRQ_USIM,          // USIM (Smart Card) - ID 15
    IRQ_GPIOXX_2,      // single interrupt for GPIO Pins 2:120
    0x00000000, 
    0x00000000,
    0x00000000, 
    0x00000000, 
    0x00000000,
    0x00000000, 
    0x00000000, 
    0x00000000
};

static UINT32   g_IntPriorities2[2] =
{
    0x00000000,        // Priorities configured in priorit 2 register.
    IRQ_KEYPAD         //    "
};

// Local Variables 

// Local Functions 
void DisableGPIOIrq(UINT32 irq);
void ClearGPIOIrq(UINT32 irq);
UINT32 FindIRQ_GPIOXX_2();
UINT32 FirstSetBitPos(UINT32 val);


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
    UINT8 nIntPrio;
    
    OALMSG( OAL_FUNC&&OAL_INTR, (L"+OALInterruptInit\r\n") );

    // Initialize interrupt mapping
    OALIntrMapInit();

    // Map a pointer to the GPIO regs
    //
    g_pGpioRegs  = (volatile BULVERDE_GPIO_REG *) OALPAtoUA(BULVERDE_BASE_REG_PA_GPIO);

    // Map a pointer to the interrupt controller.
    //
    g_pICReg = (volatile BULVERDE_INTR_REG *) OALPAtoVA(BULVERDE_BASE_REG_PA_INTC, FALSE);

    // Configure interrupt priorities.
    //
    for (nIntPrio = 0 ; nIntPrio < NUM_INT_PRIOS ; nIntPrio++)
    {
        g_pICReg->ipr[nIntPrio] = ((1 << 31) | g_IntPriorities[nIntPrio]);
    }
    g_pICReg->ipr2[0] = ((1 << 31) | g_IntPriorities2[0]);
    g_pICReg->ipr2[1] = ((1 << 31) | g_IntPriorities2[1]);

    //
    // Set DIM, the only bit in the ICCR.  
    // The effect is that only enabled and unmasked
    // interrupts bring the processor out of IDLE mode.
    //
    g_pICReg->iccr = 0x1;

#ifdef OAL_BSP_CALLBACKS
    // Give BSP change to initialize subordinate controller
    rc = BSPIntrInit();
#else
    rc = TRUE;
#endif

    // Setup static interrupt mappings (first one isn't really needed)
    OALIntrStaticTranslate(SYSINTR_RESCHED, IRQ_OSMR0);
    OALIntrStaticTranslate(SYSINTR_RTC_ALARM, IRQ_RTCALARM);

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
//  Function:  OALIntrEnableIrqs
//
BOOL OALIntrEnableIrqs(UINT32 count, const UINT32 *pIrqs)
{
    BOOL rc = TRUE;
    UINT32 irq, i;


    OALMSG(OAL_INTR&&OAL_FUNC, (
        L"+OALIntrEnableIrqs(%d, 0x%08x)\r\n", count, pIrqs
    ));
    
    for (i = 0; i < count; i++) {
#ifndef OAL_BSP_CALLBACKS
        irq = pIrqs[i];
#else
        // Give BSP chance to enable irq on subordinate interrupt controller
        irq = BSPIntrEnableIrq(pIrqs[i]);
        if (irq == OAL_INTR_IRQ_UNDEFINED) continue;
#endif
        if (irq <= IRQ_RTCALARM) {
            // Enable the primary IRQ
            SETREG32(&g_pICReg->icmr, (1 << irq));
        }
        else if ((irq >= IRQ_WTM) && (irq <= IRQ_CAMQCKCAP))
        {
            SETREG32(&g_pICReg->icmr2, (1 << (irq - IRQ_WTM)));
        }
        else if (irq >= IRQ_GPIOXX_2_GPIOMIN && irq <= IRQ_GPIOXX_2_GPIOMAX) {
            //Enable IRQ_GPIOXX_2
            SETREG32(&g_pICReg->icmr, (1 << IRQ_GPIOXX_2));
            //Note: To actually enable this GPIO irq generation make sure
            //the corresponding GRERx/GFERx bits are also set
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
        L"+OALIntrDisableIrqs(%d, 0x%08x)\r\n", count, pIrqs
    ));
    
    for (i = 0; i < count; i++) {
#ifndef OAL_BSP_CALLBACKS
        irq = pIrqs[i];
#else
        // Give BSP chance to disable irq on subordinate interrupt controller
        irq = BSPIntrDisableIrq(pIrqs[i]);
#endif
        if (irq <= IRQ_RTCALARM) {
            // Disable the primary IRQ
            CLRREG32(&g_pICReg->icmr, (1 << irq));
        }
        else if ((irq >= IRQ_WTM) && (irq <= IRQ_CAMQCKCAP)) {
            CLRREG32(&g_pICReg->icmr2, (1 << (irq - IRQ_WTM)));
        }
        else if (irq >= IRQ_GPIOXX_2_GPIOMIN && irq <= IRQ_GPIOXX_2_GPIOMAX) {
            //Clear GRERx and GFERx bit disabling interrupt generation.
            //Note: Since the next line clears the GRERx and GFERx bits,
            //to reenable this GPIO irq generation calling OALIntrEnableIrqs
            //is not enough - make sure corresponding GRERx/GFERx bits are also set.
            DisableGPIOIrq(irq);
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
        if (irq == OAL_INTR_IRQ_UNDEFINED) continue;
#endif
        if (irq <= IRQ_RTCALARM) {
            // Enable the primary IRQ
            SETREG32(&g_pICReg->icmr, (1 << irq));
        }            
        else if ((irq >= IRQ_WTM) && (irq <= IRQ_CAMQCKCAP))
        {
            SETREG32(&g_pICReg->icmr2, (1 << (irq - IRQ_WTM)));
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
    UINT32 irq = OAL_INTR_IRQ_UNDEFINED;
    UINT32 sysIntr = SYSINTR_NOP;

    if (!g_pICReg) {
        return(SYSINTR_NOP);
    }

    // Determine the IRQ of the highest priority pending interrupt
    irq = (UINT16)((g_pICReg->ichp >> 16) & 0x3F);

    if (irq == 0)
    {
        return(SYSINTR_NOP);
    }

    // System timer interrupt?
    if (irq == IRQ_OSMR0)
    {
        // The rest is up to the timer interrupt handler.
        //
        sysIntr = OALTimerIntrHandler();

    }
    // Profiling timer interrupt?
    else if (irq == IRQ_OSMR2)
    {
        // Mask the interrupt
        CLRREG32(&g_pICReg->icmr, (1 << irq));

        // The rest is up to the profiling interrupt handler (if profiling
        // is enabled).
        //
        if (g_pProfilerISR) {
            sysIntr = g_pProfilerISR(ra);
        }
    }
    // Board-level interrupts
    else
    {
#ifdef OAL_BSP_CALLBACKS
        UINT32 origIrq = irq;  // save the original so we can tell if it's BSP specific irq
        
        if (irq == IRQ_GPIO0 || irq == IRQ_GPIO1 || irq == IRQ_GPIOXX_2)
        {
            // Give BSP chance to translate IRQ -- if there is subordinate
            // interrupt controller in BSP it give chance to decode its status
            // and change IRQ
            irq = BSPIntrActiveIrq(irq);

            // if irq equals IRQ_GPIOXX_2 demultiplex it to a particular emulated GPIO IRQ if it's not a BSP specific IRQ
            if (irq == IRQ_GPIOXX_2 && origIrq== irq) {
                
                // find the GPIO IRQ
                irq = FindIRQ_GPIOXX_2();

                if (irq != OAL_INTR_IRQ_UNDEFINED)
                {
                    // Mask the interrupt
                    CLRREG32(&g_pICReg->icmr, (1 << IRQ_GPIOXX_2));
                    
                    //clear the GEDRx bit otherwise this ISR will be called again
                    ClearGPIOIrq(irq);

                    // Unmask the interrupt
                    SETREG32(&g_pICReg->icmr, (1 << IRQ_GPIOXX_2));
                }
            }
            else if (irq == IRQ_GPIO1 && origIrq== irq) {
                //clear the GEDRx bit otherwise this ISR will be called again
                g_pGpioRegs->GEDR0 = 1u << 1;
            }
        }
        else
#endif
        {
            // Mask the interrupt
            if ((irq >= IRQ_WTM) && (irq <= IRQ_CAMQCKCAP))
            {
                CLRREG32(&g_pICReg->icmr2, (1 << (irq - IRQ_WTM)));
            }
            else
            {
                CLRREG32(&g_pICReg->icmr, (1 << irq));
            }
        }

#ifdef OAL_ILTIMING
        if (g_oalILT.active) {
            g_oalILT.interrupts++;
        }        
#endif
        // First find if IRQ is claimed by chain
        sysIntr = (UINT16)NKCallIntChain((UCHAR)irq);
        
        //installable ISR returned SYSINTR_NOP?
        if (SYSINTR_NOP == sysIntr)
        {
#ifdef OAL_BSP_CALLBACKS
            if (origIrq != irq) {
                // BSP specific irq
                BSPIntrEnableIrq (irq);
            } else
#endif
            {
                //no additional processing is required.
                //Unmask the interrupt
                if (irq <= IRQ_RTCALARM) {
                    SETREG32(&g_pICReg->icmr, (1 << irq));
                }
                else if ((irq >= IRQ_WTM) && (irq <= IRQ_CAMQCKCAP)) {
                    SETREG32(&g_pICReg->icmr2, (1 << (irq - IRQ_WTM)));
                }
            }
            
            return SYSINTR_NOP;
        }
        
        if (sysIntr == (UINT16)SYSINTR_CHAIN || !NKIsSysIntrValid(sysIntr))
        {
            // IRQ wasn't claimed, use static mapping
            sysIntr = OALIntrTranslateIrq(irq);
        }

        // unmask interrupts in case it's NOP or invalid
        if (SYSINTR_NOP == sysIntr) {
#ifdef OAL_BSP_CALLBACKS
            if (origIrq != irq) {
                // BSP specific irq
                BSPIntrEnableIrq (irq);
            } else
#endif
            {
                // Unmask the interrupt
                if (irq <= IRQ_RTCALARM) {
                    SETREG32(&g_pICReg->icmr, (1 << irq));
                }
                else if ((irq >= IRQ_WTM) && (irq <= IRQ_CAMQCKCAP)) {
                    SETREG32(&g_pICReg->icmr2, (1 << (irq - IRQ_WTM)));
                }
            }
        }
    }

    return (sysIntr);
}


//------------------------------------------------------------------------------
//
//  Function:  OEMInterruptHandlerFIQ
//
void OEMInterruptHandlerFIQ()
{
}

//------------------------------------------------------------------------------
//
//  Function:  disables a GPIO Irq by clearing the corresponding GFERx and GRERx bits
//
void DisableGPIOIrq(UINT32 irq)
{
    if (IRQ_GPIOXX_2_GPIO2 <= irq && irq <= IRQ_GPIOXX_2_GPIO31) {
        g_pGpioRegs->GFER0 &= ~(1 << (irq +2 - IRQ_GPIOXX_2_GPIO2));
        g_pGpioRegs->GRER0 &= ~(1 << (irq +2 - IRQ_GPIOXX_2_GPIO2));
    }
    else if (IRQ_GPIOXX_2_GPIO32 <= irq && irq <= IRQ_GPIOXX_2_GPIO63) {
        g_pGpioRegs->GFER1 &= ~(1 << (irq - IRQ_GPIOXX_2_GPIO32));
        g_pGpioRegs->GRER1 &= ~(1 << (irq - IRQ_GPIOXX_2_GPIO32));
    }
    else if (IRQ_GPIOXX_2_GPIO64 <= irq && irq <= IRQ_GPIOXX_2_GPIO95)  {
        g_pGpioRegs->GFER2 &= ~(1 << (irq - IRQ_GPIOXX_2_GPIO64));
        g_pGpioRegs->GRER2 &= ~(1 << (irq - IRQ_GPIOXX_2_GPIO64));
    }
    else if (IRQ_GPIOXX_2_GPIO96 <= irq && irq <= IRQ_GPIOXX_2_GPIOMAX)  {
        g_pGpioRegs->GFER3 &= ~(1 << (irq - IRQ_GPIOXX_2_GPIO96)) & GPIO_GFER3_VLD_MSK;
        g_pGpioRegs->GRER3 &= ~(1 << (irq - IRQ_GPIOXX_2_GPIO96)) & GPIO_GRER3_VLD_MSK;
    }
}

//------------------------------------------------------------------------------
//
//  Function:  clears a GPIO Irq by clearing the GEDRx bit
//
void ClearGPIOIrq(UINT32 irq)
{

    if (IRQ_GPIOXX_2_GPIO2 <= irq && irq <= IRQ_GPIOXX_2_GPIO31)
        g_pGpioRegs->GEDR0 = 1 << (irq +2 - IRQ_GPIOXX_2_GPIO2);
    else if (IRQ_GPIOXX_2_GPIO32 <= irq && irq <= IRQ_GPIOXX_2_GPIO63)
        g_pGpioRegs->GEDR1 = 1 << (irq - IRQ_GPIOXX_2_GPIO32);
    else if (IRQ_GPIOXX_2_GPIO64 <= irq && irq <= IRQ_GPIOXX_2_GPIO95)
        g_pGpioRegs->GEDR2 = 1 << (irq - IRQ_GPIOXX_2_GPIO64);
    else if (IRQ_GPIOXX_2_GPIO96 <= irq && irq <= IRQ_GPIOXX_2_GPIOMAX)
        g_pGpioRegs->GEDR3 = (1 << (irq - IRQ_GPIOXX_2_GPIO96));
}

//------------------------------------------------------------------------------
//
//  Function:  returns the position of the first least significant bit set to 1.
//                 ex: for 0b000010  returns 1
//                           0b010100  returns 2
//                           0b011111  returns 0
//
UINT32 FirstSetBitPos(UINT32 val)
{
    UINT32 pos = 0;

    if (!val) return -1;
    
    //zero out all high order bits that are 1 except the lowest one
    val &= (INT32)(0-val);
    for (;;)
        {
            switch (val)
                {
                case 1: return pos;
                case 2: return pos+1;
                case 4: return pos+2;
                case 8: return pos+3;
                case 16: return pos+4;
                case 32: return pos+5;
                case 64: return pos+6;
                case 128: return pos+7;
                default: 
                    val >>= 8;
                    pos += 8;
                }
        }
}


//------------------------------------------------------------------------------
//
//  Function:  Find_GPIOXX_2_IRQ
//
//  looks up GEDRx registers to find the GPIO Pin which caused interrupt
//  Note: The algo below gives GPIOx higher priority over GPIOy where x < y.
//
UINT32 FindIRQ_GPIOXX_2()
{
    //look up GEDR to find the GPIO Pin that caused interrupt
    UINT32 retIrq = OAL_INTR_IRQ_UNDEFINED;
    UINT32 regVal;

    if (regVal = (g_pGpioRegs->GEDR0 & ~0x03 & GPIO_GEDR0_VLD_MSK)) {     //~0x03 masks GPIO0 and GPIO1 bit positions
        retIrq = FirstSetBitPos(regVal >> 2) + IRQ_GPIOXX_2_GPIO2;
    }
    else if (regVal = (g_pGpioRegs->GEDR1 & GPIO_GEDR1_VLD_MSK)) {
        retIrq = FirstSetBitPos(regVal) + IRQ_GPIOXX_2_GPIO32;
    }
    else if (regVal = (g_pGpioRegs->GEDR2 & GPIO_GEDR2_VLD_MSK)) {
        retIrq = FirstSetBitPos(regVal) + IRQ_GPIOXX_2_GPIO64;
    }
    else if (regVal = (g_pGpioRegs->GEDR3 & GPIO_GEDR3_VLD_MSK)) {
        retIrq = FirstSetBitPos(regVal) + IRQ_GPIOXX_2_GPIO96;
    }
    else { 
        return OAL_INTR_IRQ_UNDEFINED;
    }
    
    if (retIrq < IRQ_GPIOXX_2_GPIOMIN|| retIrq > IRQ_GPIOXX_2_GPIOMAX)
        return OAL_INTR_IRQ_UNDEFINED;

    return retIrq;
}

//------------------------------------------------------------------------------
