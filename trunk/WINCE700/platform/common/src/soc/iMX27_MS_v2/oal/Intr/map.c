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
// Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//
//  File:  map.c
//
//  The file implement modified table/array based mapping between IRQ and SYSINTR
//  which is suitable for most OAL implementations.
//
//-----------------------------------------------------------------------------
#include <windows.h>
#include <nkintr.h>
#include <oal_intr.h>
#include <oal_log.h>
#include "csp.h"

//
// Bypass WinCE 5.0 OAL limitation on number of IRQs
// for SYSINTR/IRQ mapping.
// Rest is all Microsoft codes
//
#ifdef OAL_INTR_IRQ_MAXIMUM
#undef OAL_INTR_IRQ_MAXIMUM
#define OAL_INTR_IRQ_MAXIMUM    CSP_IRQ_MX27_MAX
#endif

//------------------------------------------------------------------------------

static UINT32 g_oalSysIntr2Irq[SYSINTR_MAXIMUM];
static UINT32 g_oalIrq2SysIntr[OAL_INTR_IRQ_MAXIMUM];

//------------------------------------------------------------------------------
//
//  Function:  OALIntrMapInit
//
//  This function must be called from OALInterruptInit to initialize mapping
//  between IRQ and SYSINTR. It simply initialize mapping arrays.
//
VOID OALIntrMapInit()
{
    UINT32 i;
    
    OALMSG(OAL_FUNC&&OAL_INTR, (L"+OALIntrMapInit\r\n"));

    // Initialize interrupt maps
    for (i = 0; i < SYSINTR_MAXIMUM; i++) {
        g_oalSysIntr2Irq[i] = OAL_INTR_IRQ_UNDEFINED;
    }
    for (i = 0; i < OAL_INTR_IRQ_MAXIMUM; i++) {
        g_oalIrq2SysIntr[i] = SYSINTR_UNDEFINED;
    }

    OALMSG(OAL_FUNC&&OAL_INTR, (L"-OALIntrMapInit\r\n"));
}


//------------------------------------------------------------------------------
//
//  Function:   OALIntrStaticTranslate
//
//  This function sets static translation between IRQ and SYSINTR. In most
//  cases it should not be used. Only exception is mapping for 
//  SYSINTR_RTC_ALARM and obsolete device drivers.
//
VOID OALIntrStaticTranslate(UINT32 sysIntr, UINT32 irq)
{
    OALMSG(OAL_FUNC&&OAL_INTR, (
        L"+OALIntrStaticTranslate(%d, %d)\r\n", sysIntr, irq
    ));
    if (irq < OAL_INTR_IRQ_MAXIMUM && sysIntr < SYSINTR_MAXIMUM) {
        g_oalSysIntr2Irq[sysIntr] = irq;
        g_oalIrq2SysIntr[irq] = sysIntr;
    }        
    OALMSG(OAL_FUNC&&OAL_INTR, (L"-OALIntrStaticTranslate\r\n"));
}


//------------------------------------------------------------------------------
//
//  Function:  OALIntrTranslateSysIntr
//
//  This function maps a SYSINTR to its corresponding IRQ. It is typically used
//  in OEMInterruptXXX to obtain IRQs for given SYSINTR.
//
BOOL OALIntrTranslateSysIntr(
    UINT32 sysIntr, UINT32 *pCount, const UINT32 **ppIrqs
) {
    BOOL rc;

    OALMSG(OAL_INTR&&OAL_VERBOSE, (L"+OALTranslateSysIntr(%d)\r\n", sysIntr));

    // Valid SYSINTR?
    if (sysIntr >= SYSINTR_MAXIMUM) {
        rc = FALSE;
        goto cleanUp;
    }
    *pCount = 1;
    *ppIrqs = &g_oalSysIntr2Irq[sysIntr];
    rc = TRUE;

cleanUp:
    OALMSG(OAL_INTR&&OAL_VERBOSE, (L"-OALTranslateSysIntr(rc = %d)\r\n", rc));
    return rc;
}


//------------------------------------------------------------------------------
//
//  Function:  OALIntrTranslateIrq
//
//  This function maps a IRQ to its corresponding SYSINTR.
//
UINT32 OALIntrTranslateIrq(UINT32 irq)
{
    UINT32 sysIntr = SYSINTR_UNDEFINED;
    
    OALMSG(OAL_FUNC&&OAL_VERBOSE, (L"+OALIntrTranslateIrq(%d)\r\n", irq));

    if (irq >= OAL_INTR_IRQ_MAXIMUM) goto cleanUp;
    sysIntr = g_oalIrq2SysIntr[irq];

cleanUp:
    OALMSG(OAL_FUNC&&OAL_VERBOSE, (
        L"-OEMTranslateIrq(sysIntr = %d)\r\n", sysIntr
    ));
    return sysIntr;
}


//------------------------------------------------------------------------------
//
//  Function:  OALIntrRequestSysIntr
//
//  This function allocate new SYSINTR for given IRQ and it there isn't
//  static mapping for this IRQ it will create it.
//
UINT32 OALIntrRequestSysIntr(UINT32 count, const UINT32 *pIrqs, UINT32 flags)
{
    UINT32 irq, sysIntr;

    OALMSG(OAL_INTR&&OAL_FUNC, (
        L"+OALIntrRequestSysIntr(%d, 0x%08x, 0x%08x)\r\n", count, pIrqs, flags
    ));
    
    // Valid IRQ?
    if (count != 1 || pIrqs[0] >= OAL_INTR_IRQ_MAXIMUM) {
        sysIntr = SYSINTR_UNDEFINED;            
        goto cleanUp;
    }
    irq = pIrqs[0];

    // If there is mapping for given irq check for special cases
    if (g_oalIrq2SysIntr[irq] != SYSINTR_UNDEFINED) {
        // If static mapping is requested we fail
        if ((flags & OAL_INTR_STATIC) != 0) {
            OALMSG(OAL_ERROR, (L"ERROR: OALIntrRequestSysIntr: "
               L"Static mapping for IRQ %d already assigned\r\n", irq
            ));
            sysIntr = SYSINTR_UNDEFINED;            
            goto cleanUp;
        }
        // If we should translate, return existing SYSINTR
        if ((flags & OAL_INTR_TRANSLATE) != 0) {
            sysIntr = g_oalIrq2SysIntr[irq];
            goto cleanUp;
        }
    }

    // Find next available SYSINTR value...
    for (sysIntr = SYSINTR_FIRMWARE; sysIntr < SYSINTR_MAXIMUM; sysIntr++) {
        if (g_oalSysIntr2Irq[sysIntr] == OAL_INTR_IRQ_UNDEFINED) break;
    }
    
    // Any available SYSINTRs left?
    if (sysIntr >= SYSINTR_MAXIMUM) {
        OALMSG(OAL_ERROR, (L"ERROR: OALIntrRequestSysIntr: "
            L"No avaiable SYSINTR found\r\n"
        ));            
        sysIntr = SYSINTR_UNDEFINED;
        goto cleanUp;
    }
    
    // Make SYSINTR -> IRQ association.
    g_oalSysIntr2Irq[sysIntr] = irq;
    
    // Make IRQ -> SYSINTR association if required
    if ((flags & OAL_INTR_DYNAMIC) != 0) goto cleanUp;
    if (
        g_oalIrq2SysIntr[irq] == SYSINTR_UNDEFINED ||
        (flags & OAL_INTR_FORCE_STATIC) != 0
    ) {
        g_oalIrq2SysIntr[irq] = sysIntr;
    }

cleanUp:
    OALMSG(OAL_INTR&&OAL_FUNC, (
        L"-OALIntrRequestSysIntr(sysIntr = %d)\r\n", sysIntr
    ));
    return sysIntr;
}


//------------------------------------------------------------------------------
//
//  Function:  OALIntrReleaseSysIntr
//
//  This function release given SYSINTR and remove static mapping if exists.
//
BOOL OALIntrReleaseSysIntr(UINT32 sysIntr)
{
    BOOL rc = FALSE;
    UINT32 irq;

    // is this a valid sysIntr?
    if ((sysIntr < SYSINTR_FIRMWARE) || (sysIntr >= SYSINTR_MAXIMUM)) {
        OALMSG(OAL_ERROR, (L"ERROR: OALIntrReleaseSysIntr: "
            L"Invalid sysIntr value %d\r\n", sysIntr
            ));
        goto cleanUp;
    }

    OALMSG(OAL_INTR&&OAL_FUNC, (L"+OALIntrReleaseSysIntr(%d)\r\n", sysIntr));

    // Is the SYSINTR already released?
    if (g_oalSysIntr2Irq[sysIntr] == OAL_INTR_IRQ_UNDEFINED) goto cleanUp;

    // Remove the SYSINTR -> IRQ mapping
    irq = g_oalSysIntr2Irq[sysIntr];
    g_oalSysIntr2Irq[sysIntr] = OAL_INTR_IRQ_UNDEFINED;

    // If we're releasing the SYSINTR directly mapped in the IRQ mapping, 
    // remove the IRQ mapping also
    if (g_oalIrq2SysIntr[irq] == sysIntr) {
        g_oalIrq2SysIntr[irq] = SYSINTR_UNDEFINED;
    }
    rc = TRUE;
	
cleanUp:
    OALMSG(OAL_INTR&&OAL_FUNC, (L"-OALIntrReleaseSysIntr(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  Function:  OEMInterruptEnable
//
//  This function enables the IRQ given its corresponding SysIntr value.
//  Function returns true if SysIntr is valid, else false.
//
BOOL
OEMInterruptEnable(
    DWORD sysIntr, 
    VOID* pData, 
    DWORD dataSize
    )
{
    BOOL rc = FALSE;

    OALMSG(OAL_INTR&&OAL_VERBOSE, 
        (L"+OEMInterruptEnable(%d, 0x%x, %d)\r\n", sysIntr, pData, dataSize
        ));

    // SYSINTR_VMINI & SYSINTR_TIMING are special cases
    if ((sysIntr == SYSINTR_VMINI) || (sysIntr == SYSINTR_TIMING))
        {
        rc = TRUE;
        goto cleanUp;
        }

    // Enable interrupts
    rc = OALIntrEnableIrqs(1, &g_oalSysIntr2Irq[sysIntr]);

cleanUp:    
    OALMSG(OAL_INTR&&OAL_VERBOSE, (L"-OEMInterruptEnable(rc = 1)\r\n"));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  OEMInterruptDisable
//
//  This function disables the IRQ given its corresponding SysIntr value.
//
VOID
OEMInterruptDisable(
    DWORD sysIntr
    )
{
    OALMSG(OAL_INTR&&OAL_VERBOSE, (L"+OEMInterruptDisable(%d)\r\n", sysIntr));

    // Disable interrupts
    OALIntrDisableIrqs(1, &g_oalSysIntr2Irq[sysIntr]);

    OALMSG(OAL_INTR&&OAL_VERBOSE, (L"-OEMInterruptDisable\r\n"));
}

//------------------------------------------------------------------------------
//
//  Function:  OEMInterruptMask
//
//  This function masks the IRQ given its corresponding SysIntr value.
//
//
VOID
OEMInterruptMask(
    DWORD sysIntr, 
    BOOL mask
    )
{
    OALMSG(OAL_INTR&&OAL_VERBOSE, (
        L"+OEMInterruptMask(%d, %d)\r\n", sysIntr, mask
        ));

    // Based on mask enable or disable
    if (mask)
        {
        OALIntrDisableIrqs(1, &g_oalSysIntr2Irq[sysIntr]);
        }
    else 
        {
        OALIntrEnableIrqs(1, &g_oalSysIntr2Irq[sysIntr]);
        }

    OALMSG(OAL_INTR&&OAL_VERBOSE, (L"-OEMInterruptMask\r\n"));
}

//------------------------------------------------------------------------------
//
//  Function: OEMInterruptDone
//
//  OEMInterruptDone is called by the kernel when a device driver
//  calls InterruptDone(). The system is not preemtible when this
//  function is called.
//
VOID
OEMInterruptDone(
    DWORD sysIntr
    )
{
    OALMSG(OAL_INTR&&OAL_VERBOSE, (L"+OEMInterruptDone(%d)\r\n", sysIntr));

    // Re-enable interrupts
    OALIntrDoneIrqs(1, &g_oalSysIntr2Irq[sysIntr]);
    
    OALMSG(OAL_INTR&&OAL_VERBOSE, (L"-OEMInterruptDone\r\n"));
}

//------------------------------------------------------------------------------

