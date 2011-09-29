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
#include <windows.h>
#include <nkintr.h> 
#include <shx.h>
#include <oal.h>

//------------------------------------------------------------------------------
// Array of interrupt priorities exported by the kernel
#ifndef IntrPrio
#define IntrPrio g_pNKGlobal->IntrPrio
#endif

//------------------------------------------------------------------------------
//
//  Function:  HookAndSetPriority
//
BOOL HookAndSetPriority(int hwIntNumber, FARPROC pfnHandler, BYTE intrPrio)
{
    BOOL retVal = FALSE;
    
    OALMSG(OAL_INTR&&OAL_FUNC, (L"+HookAndSetPriority(%d, 0x%08x, %d)\r\n",
                      hwIntNumber, pfnHandler, intrPrio));
                      
    if(!HookInterrupt(hwIntNumber, pfnHandler))
    {
        OALMSG(OAL_ERROR, (L"ERROR: Unable to hook interrupt\r\n"));
        goto cleanUp;
    }
    
    IntrPrio[hwIntNumber] = intrPrio;
    retVal = TRUE;

cleanUp:
    OALMSG(OAL_INTR&&OAL_FUNC, (L"-HookAndSetPriority(rc = %d)\r\n", retVal));
    return retVal;
}

//------------------------------------------------------------------------------
//
//  Function:  OALIntrInit/BSPIntrInit
//
//  This function initialize interrupt hardware. It is usally called at system
//  initialization. If implementation uses platform callback it will call
//  BPSIntrInit.
//
BOOL OALIntrInit()
{
    BOOL            retVal      = FALSE;
    SH4_INTC_REGS  *pINTCRegs   = OALPAtoUA(SH4_REG_PA_INTC);

    OALMSG(OAL_INTR&&OAL_FUNC, (L"+OALIntrInit()\r\n"));
    
    // Setup physical to logical mapping tables
    OALIntrMapInit();

#ifdef OAL_BSP_CALLBACKS
    retVal = BSPIntrInit();
#else
    retVal = TRUE;
#endif

    OALMSG(OAL_INTR&&OAL_FUNC, (L"-OALIntrInit(rc = %d)\r\n", retVal));
    return retVal;
}

//------------------------------------------------------------------------------
//
//  Function:  OALIntrEnableIrq/BSPIntrEnableIrq
//
//  This function enable interrupt identified by IRQ. If implementation uses
//  platform callbacks it will call BSPIntrEnableIrq before IRQ is enabled in
//  hardware. The BSPIntrEnableIrq returns IRQ used for interrupt controller
//  chaining.
//
BOOL OALIntrEnableIrqs(UINT32 count, const UINT32 *pIrqs)
{
    UINT32           irq, i;
    BOOL             retVal     = TRUE;
    SH4_RTC_REGS    *pRTCRegs   = OALPAtoUA(SH4_REG_PA_RTC);
    OALMSG(OAL_INTR&&OAL_VERBOSE, (L"+OALIntrEnableIrqs(%d, 0x%08x)\r\n", count, pIrqs));

    for(i = 0; i < count; i++) 
    {
#ifndef OAL_BSP_CALLBACKS
        irq = pIrqs[i];
#else 
        // Give the BSP a chance to enable the irq
        irq = BSPIntrEnableIrq(pIrqs[i]);
#endif
        if(irq == OAL_INTR_IRQ_UNDEFINED) continue;

        switch(irq)
        {
        case IRQ_RTC_ATI:
            OUTREG8(&pRTCRegs->RCR1, INREG8(&pRTCRegs->RCR1) | RTC_RCR1_AIE);
            OALMSG(OAL_INTR&&OAL_VERBOSE, (L"INFO: IRQ_RTC_ATI Enabled\r\n"));
            break;

        default:
            OALMSG(OAL_ERROR, (L"ERROR: Unable to enable IRQ %d\r\n", irq));
            retVal = FALSE;
        }
    }

    OALMSG(OAL_INTR&&OAL_VERBOSE, (L"-OALIntrEnableIrqs(rc = %d)\r\n", retVal));
    return retVal;
}

//------------------------------------------------------------------------------
//
//  Function:  OALIntrDisableIrq/BSPIntrDisableIrq
//
//  This function disable interrupt identified by IRQ. If implementation uses
//  platform callbacks it will call BSPIntrDisableIrq before IRQ is disabled in
//  hardware. The BSPIntrEnableIrq returns IRQ used for interrupt controller
//  chaining if it is suitable to disable it.
//
VOID OALIntrDisableIrqs(UINT32 count, const UINT32 *pIrqs)
{
    UINT32 irq, i;
    SH4_RTC_REGS *pRTCRegs = OALPAtoUA(SH4_REG_PA_RTC);
    OALMSG(OAL_INTR&&OAL_VERBOSE, (L"+OALIntrDisableIrqs(%d, 0x%08x)\r\n", count, pIrqs));

    for(i = 0; i < count; i++)
    {
#ifndef OAL_BSP_CALLBACKS
        irq = pIrqs[i];
#else
        irq = BSPIntrDisableIrq(pIrqs[i]);
#endif
        if(irq == OAL_INTR_IRQ_UNDEFINED) continue;

        switch(irq)
        {
        case IRQ_RTC_ATI:
            OUTREG8(&pRTCRegs->RCR1, INREG8(&pRTCRegs->RCR1) & ~RTC_RCR1_AIE);
            OALMSG(OAL_INTR&&OAL_VERBOSE, (L"INFO: IRQ_RTC_ATI Disabled\r\n"));
            break;

        default:
            OALMSG(OAL_ERROR, (L"ERROR: Unable to disable IRQ %d\r\n", irq));
        }
    }

    OALMSG(OAL_INTR&&OAL_VERBOSE, (L"-OALIntrDisableIrqs()\r\n"));
}

//------------------------------------------------------------------------------
//
//  Function:  OALIntrRequestIrq/BSPIntrRequestIrq
//
//  This function return list of IRQs for device identified by DEVICE_LOCATION 
//  parameter. If implementation uses platform callbacks it should call
//  BSPIntrRequestIrq  in case that it doesn't recognise device.
//  On input *pCount contains maximal number of IRQs allowed in list. On return
//  it returns number of IRQ in list.
//
BOOL OALIntrRequestIrqs(DEVICE_LOCATION *pDevLoc, UINT32 *pCount, UINT32 *pIrqs)
{
    BOOL retVal = FALSE;

    OALMSG(OAL_INTR&&OAL_FUNC, (
        L"+OALIntrRequestIrqs(0x%08x->%d/%d/0x%08x/%d, 0x%08x)\r\n",
        pDevLoc, pDevLoc->IfcType, pDevLoc->BusNumber, pDevLoc->LogicalLoc,
        pDevLoc->Pin, pIrqs
    ));

    if(*pCount < 1) goto cleanUp;

#ifdef OAL_BSP_CALLBACKS
    if (!retVal) retVal = BSPIntrRequestIrqs(pDevLoc, pCount, pIrqs);
#endif    
        
cleanUp:
    OALMSG(OAL_INTR&&OAL_FUNC, (L"-OALIntrRequestIrqs(rc = %d)\r\n", retVal));
    return retVal;
}
//------------------------------------------------------------------------------
//
//  Function:  OALIntrDoneIrq/BSPIntrDoneIrq
//
//  This function finish interrupt identified by IRQ. If implementation uses
//  platform callbacks it will call BSPIntrDoneIrq before IRQ is enabled in
//  hardware. The BSPIntrDoneIrq returns IRQ used for interrupt controller
//  chaining if it is suitable to finish it. In most cases implementation will
//  for both function will be similar to OALIntrEnableIrq/BSPIntrEnableIrq.
//
VOID OALIntrDoneIrqs(UINT32 count, const UINT32 *pIrqs)
{
    UINT32 irq, i;
    SH4_RTC_REGS *pRTCRegs = OALPAtoUA(SH4_REG_PA_RTC);

    OALMSG(OAL_VERBOSE&&OAL_FUNC, (L"+OALIntrDoneIrqs(%d, 0x%08x)\r\n", count, pIrqs));

    for(i = 0; i < count; i++)
    {
#ifndef OAL_BSP_CALLBACKS
        irq = pIrqs[i];
#else
        irq = BSPIntrDoneIrq(pIrqs[i]);
#endif
        if(irq == OAL_INTR_IRQ_UNDEFINED) continue;

        switch(irq)
        {
        case IRQ_RTC_ATI:
            // Clear alarm flag but do not reenable interrupt for the RTC Alarm
            // since that will happen the next time OEMSetAlarmTime is called
            OUTREG8(&pRTCRegs->RCR1, (INREG8(&pRTCRegs->RCR1) & ~(RTC_RCR1_AF)));
            break;

        default:
            OALMSG(OAL_ERROR, (L"ERROR: Unable to disable IRQ %d\r\n", irq));
        }
    }

    OALMSG(OAL_VERBOSE&&OAL_FUNC, (L"-OALIntrDoneIrqs()\r\n"));
}
