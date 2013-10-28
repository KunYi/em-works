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
/*
================================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
================================================================================
*/
//
//  File:  power.c
//
#include <windows.h>
#include <nkintr.h>
#include <oal.h>
#include <oalex.h>
#include <omap35xx.h>
#include <interrupt_struct.h>
#include <bus.h>
#include <oal_prcm.h>

//------------------------------------------------------------------------------
//
//  External:  g_pIntr
//
//  contains gpio and interrupt information.  Initialized in OALIntrInit()
//
extern OMAP_INTR_CONTEXT const *g_pIntr;

extern VOID PrcmDumpSavedRefCounts();
extern void DumpPrcmRegsSnapshot();
extern BOOL g_PrcmDebugSuspendResume;

//------------------------------------------------------------------------------
//
//  External:  OALIntrIsIrqPending
//
//  Checks if the given interrupt is pending.
//
BOOL OALIntrIsIrqPending(UINT32 irq);

//------------------------------------------------------------------------------
//
//  External:  _gpioClkId
//
//  device id's of all GPIO lines.
//
static 
OMAP_DEVICE _gpioClkId[] = {
    OMAP_DEVICE_GPIO1, 
    OMAP_DEVICE_GPIO2,
    OMAP_DEVICE_GPIO3,
    OMAP_DEVICE_GPIO4,
    OMAP_DEVICE_GPIO5,
    OMAP_DEVICE_GPIO6
};

//------------------------------------------------------------------------------
//
//  Function:  OEMInterruptPending
//
//  This function returns true when given sysIntr interrupt is pending.
//
BOOL OEMInterruptPending(DWORD sysIntr)
{
    BOOL pending = FALSE;
    const UINT32 *pIrqs;
    UINT32 ix, count;

    OALMSG(OAL_INTR&&OAL_VERBOSE, (
        L"+OEMInterruptPending(%d)\r\n", sysIntr
        ));

    if (OALIntrTranslateSysIntr(sysIntr, &count, &pIrqs))
        {
        for (ix = 0; ix < count && !pending; ix++)
            {
            pending = OALIntrIsIrqPending(pIrqs[ix]);
            }            
        }
        
    OALMSG(OAL_INTR&&OAL_VERBOSE, (
        L"-OEMInterruptPending(rc = %d)\r\n", pending
        ));
    return pending;
}

//------------------------------------------------------------------------------
//
//  Function:  OEMPowerOff
//
//  Called when the system is to transition to it's lowest power mode (off)
//
VOID
OEMPowerOff(
    )
{
    int i = 0;
    UINT irq = 0;
    UINT32 sysIntr;
    UINT intr[3];
    UINT gpio[OMAP_GPIO_BANK_COUNT];
    UINT wkup[OMAP_GPIO_BANK_COUNT];
    UINT32 mask;
    BOOL bPowerOn;

    // UNDONE: verify if this is still necessary
    // Disable hardware watchdog
    OALWatchdogEnable(FALSE);
    
    // Make sure that KITL is powered off
    bPowerOn = FALSE;
    KITLIoctl(IOCTL_KITL_POWER_CALL, &bPowerOn, sizeof(bPowerOn), NULL, 0, NULL);    

    // Disable GPTimer2 (used for high perf/monte carlo profiling)
    PrcmDeviceEnableClocks(OMAP_DEVICE_GPTIMER2, FALSE);

    // Give chance to do board specific stuff
    BSPPowerOff();

    //----------------------------------------------
    // capture all enabled interrupts and disable interrupts
    intr[0] = INREG32(&g_pIntr->pICLRegs->INTC_MIR0);
    intr[1] = INREG32(&g_pIntr->pICLRegs->INTC_MIR1);
    intr[2] = INREG32(&g_pIntr->pICLRegs->INTC_MIR2);

    OUTREG32(&g_pIntr->pICLRegs->INTC_MIR_SET0, OMAP_MPUIC_MASKALL);
    OUTREG32(&g_pIntr->pICLRegs->INTC_MIR_SET1, OMAP_MPUIC_MASKALL);
    OUTREG32(&g_pIntr->pICLRegs->INTC_MIR_SET2, OMAP_MPUIC_MASKALL);

    //----------------------------------------------
    // Context Save/Restore
    //  Mask all GPIO interrupts
    for (i = 0; i < OMAP_GPIO_BANK_COUNT; ++i)
        {
        PrcmDeviceEnableClocksKernel(_gpioClkId[i], TRUE);
        gpio[i] = INREG32(&g_pIntr->pGPIORegs[i]->IRQENABLE1);
        wkup[i] = INREG32(&g_pIntr->pGPIORegs[i]->WAKEUPENABLE);
        OUTREG32(&g_pIntr->pGPIORegs[i]->IRQENABLE1, 0);
        OUTREG32(&g_pIntr->pGPIORegs[i]->WAKEUPENABLE, 0);
        PrcmDeviceEnableClocksKernel(_gpioClkId[i], FALSE);
        }

    //----------------------------------------------
    // Clear all enabled IO PAD wakeups for GPIOs
    for (i = 0; i < OMAP_GPIO_BANK_COUNT; ++i) 
        {
        irq = IRQ_GPIO_0 + (i * 32);
        mask = wkup[i];
        while (mask != 0)
            {
            // If a GPIO was wakeup enabled, then clear the wakeup
            if (mask & 0x1)
                {
                OEMEnableIOPadWakeup((irq - IRQ_GPIO_0), FALSE);
                }
            
            irq++;
            mask >>= 1;    
            }
        }

    //----------------------------------------------
    // Enable wake sources interrupts
    for (sysIntr = SYSINTR_FIRMWARE; sysIntr < SYSINTR_MAXIMUM; sysIntr++)
        {
        // Skip if sysIntr isn't allowed as wake source
        if (!OALPowerWakeSource(sysIntr)) 
            continue;

        // Enable it as interrupt
        OEMInterruptEnable(sysIntr, NULL, 0);
        }

    // enter full retention
    PrcmSuspend();
    
    //----------------------------------------------
    // Find wakeup source
    for (sysIntr = SYSINTR_FIRMWARE; sysIntr < SYSINTR_MAXIMUM; sysIntr++)
        {            
        // Skip if sysIntr isn't allowed as wake source
        if (!OALPowerWakeSource(sysIntr)) 
            continue;

        // When this sysIntr is pending we find wake source
        if (OEMInterruptPending(sysIntr))
            {
            g_oalWakeSource = sysIntr;
            break;
            }
        }

    // UNDONE: 
    // Need to find-out this is necessary
    // From a suspend we need to manually wakeup the peripheral power domain
    PrcmDomainSetClockStateKernel(POWERDOMAIN_PERIPHERAL, CLOCKDOMAIN_PERIPHERAL, CLKSTCTRL_WAKEUP);
   
    //----------------------------------------------
    // Context Save/Restore
    //  Mask all GPIO interrupts
    for (i = 0; i < OMAP_GPIO_BANK_COUNT; ++i)
        {
        PrcmDeviceEnableClocksKernel(_gpioClkId[i], TRUE);
        OUTREG32(&g_pIntr->pGPIORegs[i]->IRQENABLE1, gpio[i]);
        OUTREG32(&g_pIntr->pGPIORegs[i]->WAKEUPENABLE, wkup[i]);
        PrcmDeviceEnableClocksKernel(_gpioClkId[i], FALSE);
        }

    //-------------------------------------------------------
    // Enable all previously enabled IO PAD wakeups for GPIOs
    for (i = 0; i < OMAP_GPIO_BANK_COUNT; ++i) 
        {
        irq = IRQ_GPIO_0 + (i * 32);
        mask = wkup[i];
        while (mask != 0)
            {
            // If a GPIO was wakeup enabled, then clear the wakeup
            if (mask & 0x1)
                {
                OEMEnableIOPadWakeup((irq - IRQ_GPIO_0), TRUE);
                }
            
            irq++;
            mask >>= 1;    
            }
        }
    
    //----------------------------------------------
    // Re-enable interrupts    
    OUTREG32(&g_pIntr->pICLRegs->INTC_MIR_CLEAR0, ~intr[0]);
    OUTREG32(&g_pIntr->pICLRegs->INTC_MIR_CLEAR1, ~intr[1]);
    OUTREG32(&g_pIntr->pICLRegs->INTC_MIR_CLEAR2, ~intr[2]);  
    
    //----------------------------------------------
    // Do board specific stuff    
    BSPPowerOn();    

    //Sync to Hardware RTC after suspend\resume
    OALIoCtlHalRtcTime( 0,  NULL, 0, NULL, 0, NULL);
    
    // Enable GPTimer2 (used for high perf/monte carlo profiling)
    PrcmDeviceEnableClocks(OMAP_DEVICE_GPTIMER2, TRUE);

    // Reinitialize KITL
    bPowerOn = TRUE;
    KITLIoctl(IOCTL_KITL_POWER_CALL, &bPowerOn, sizeof(bPowerOn), NULL, 0, NULL);    
    
    // Enable hardware watchdog
    OALWatchdogEnable(TRUE);
    
#ifndef SHIP_BUILD
    if (g_PrcmDebugSuspendResume)
    {
        OALMSG(1, (L"Enable wake sources:\r\n"));
        for (sysIntr = SYSINTR_FIRMWARE; sysIntr < SYSINTR_MAXIMUM; sysIntr++)
        {
            if (OALPowerWakeSource(sysIntr)) 
                OALMSG(1, (L"  SYSINTR %d\r\n", sysIntr));
        }

        OALMSG(1, (L"\r\nWake due to SYSINTR %d\r\n", g_oalWakeSource));
        OALWakeupLatency_DumpSnapshot();
        PrcmDumpSavedRefCounts();
        DumpPrcmRegsSnapshot();
    }
#endif
}

//------------------------------------------------------------------------------
