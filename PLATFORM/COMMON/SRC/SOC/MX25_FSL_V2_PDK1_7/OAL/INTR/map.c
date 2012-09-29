//------------------------------------------------------------------------------
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
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File: map.c
//
//  This file implements functionality related to mapping between IRQ and
//  SYSINTR. This is custom implementation for MX25 because there can be
//  additional 32x4 GPIO IRQs and 32 SDMA IRQs. These 160 secondary interrupts 
//  generate hardware interrupts IRQ_GPIO1, IRQ_GPIO2, IRQ_GPIO3, IRQ_GPIO4 and IRQ_SDMA 
//  respectively. OAL needs to demultiplex these primary IRQs into appropriate
//  seconddary IRQs (generic implementation supports maximum of 64 IRQs). 
//  The implementation also supports multiple IRQs per one SYSINTR mapping.
//
//------------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <nkintr.h>
#include <oal.h>
#pragma warning(pop)

#include "csp.h"

//------------------------------------------------------------------------------
//
//  Globals: g_oalSysIntr2Irq/g_oalIrq2SysIntr
//
UINT32 g_oalSysIntr2Irq[SYSINTR_MAXIMUM][IRQ_PER_SYSINTR];
static UINT32 g_oalIrq2SysIntr[IRQ_MAXIMUM];

//------------------------------------------------------------------------------
//
//  Function:  OALIntrMapInit
//
//  This function must be called from OALInterruptInit to initialize mapping
//  between IRQ and SYSINTR. It simply initialize mapping arrays.
//
VOID OALIntrMapInit()
{
    UINT32 i, j;

    OALMSG(OAL_FUNC&&OAL_INTR, (L"+OALIntrMapInit\r\n"));

    // Initialize interrupt maps
    for (i = 0; i < SYSINTR_MAXIMUM; i++)
    {
        for (j = 0; j < IRQ_PER_SYSINTR; j++)
        {
            g_oalSysIntr2Irq[i][j] = (UINT32) OAL_INTR_IRQ_UNDEFINED;
        }
    }
    
    for (i = 0; i < IRQ_MAXIMUM; i++)
    {
        g_oalIrq2SysIntr[i] = (UINT32) SYSINTR_UNDEFINED;
    }

    OALMSG(OAL_FUNC&&OAL_INTR, (L"-OALIntrMapInit\r\n"));
}

//------------------------------------------------------------------------------
//
//  Function:   OALIntrStaticTranslate
//
//  This function sets static translation between IRQ and SYSINTR.
//
VOID OALIntrStaticTranslate(UINT32 sysIntr, UINT32 irq)
{
    UINT32 i;

    OALMSG(OAL_FUNC&&OAL_INTR, (
        L"+OALIntrStaticTranslate(%d, %d)\r\n", sysIntr, irq
        ));
    
    if ((irq >= IRQ_MAXIMUM) || (sysIntr >= SYSINTR_MAXIMUM))
        goto cleanUp;

    for (i = 0; i < IRQ_PER_SYSINTR; i++)
    {
        if (g_oalSysIntr2Irq[sysIntr][i] != OAL_INTR_IRQ_UNDEFINED)
            continue;
        g_oalSysIntr2Irq[sysIntr][i] = irq;
        break;
    }
    
    g_oalIrq2SysIntr[irq] = sysIntr;

cleanUp:
    OALMSG(OAL_FUNC&&OAL_INTR, (L"-OALIntrStaticTranslate\r\n"));
}

//------------------------------------------------------------------------------
//
//  Function:  OALIntrTranslateSysIntr
//
//  This function maps a SYSINTR to its corresponding IRQ. It is typically used
//  in OEMInterruptXXX to obtain IRQs for given SYSINTR.
//
BOOL OALIntrTranslateSysIntr(UINT32 sysIntr, UINT32 *pCount, const UINT32 **ppIrqs)
{
    BOOL rc;

    OALMSG(OAL_INTR&&OAL_VERBOSE, (L"+OALTranslateSysIntr(%d)\r\n", sysIntr));

    // Valid SYSINTR?
    if (sysIntr >= SYSINTR_MAXIMUM)
    {
        rc = FALSE;
        goto cleanUp;
    }
    
    *pCount = IRQ_PER_SYSINTR;
    *ppIrqs = g_oalSysIntr2Irq[sysIntr];

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
    UINT32 sysIntr = (UINT32) SYSINTR_UNDEFINED;

    OALMSG(OAL_FUNC&&OAL_VERBOSE, (L"+OALIntrTranslateIrq(%d)\r\n", irq));

    if (irq < IRQ_MAXIMUM)
        sysIntr = g_oalIrq2SysIntr[irq];

    OALMSG(OAL_FUNC&&OAL_VERBOSE, (
        L"-OALIntrTranslateIrq(sysIntr = %d)\r\n", sysIntr
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
    UINT32 sysIntr, i, j;

    OALMSG(OAL_INTR&&OAL_FUNC, (
        L"+OALIntrRequestSysIntr(%d, 0x%08x, 0x%08x)\r\n", count, pIrqs, flags
        ));

    // Valid IRQ?
    if (count > IRQ_PER_SYSINTR)
    {
        OALMSG(OAL_ERROR, (L"ERROR: OALIntrRequestSysIntr: "
            L"Only %d IRQs for SYSINTR is supported\r\n", IRQ_PER_SYSINTR
            ));
        sysIntr = (UINT32) SYSINTR_UNDEFINED;
        goto cleanUp;
    }

    // If static mapping is requested check if we can obtain one
    if ((flags & OAL_INTR_STATIC) != 0)
    {
        for (i = 0; i < count; i++)
        {
            if (pIrqs[i] >= IRQ_MAXIMUM)
            {
                OALMSG(OAL_ERROR, (L"ERROR: OALIntrRequestSysIntr: "
                    L"IRQ %d out of range\r\n", pIrqs[i]
                    ));
                sysIntr = (UINT32) SYSINTR_UNDEFINED;
                goto cleanUp;
            }

            if (g_oalIrq2SysIntr[pIrqs[i]] != SYSINTR_UNDEFINED)
            {
                OALMSG(OAL_ERROR, (L"ERROR: OALIntrRequestSysIntr: "
                    L"Static mapping for IRQ %d already assigned\r\n", pIrqs[i]
                    ));
                sysIntr = (UINT32) SYSINTR_UNDEFINED;
                goto cleanUp;
            }
        }
    }

    // If translate flag is set all IRQs must be mapped to same SYSINTR
    if (pIrqs[0] >= IRQ_MAXIMUM)
    {
        OALMSG(OAL_ERROR, (L"ERROR: OALIntrRequestSysIntr: "
            L"IRQ %d out of range\r\n", pIrqs[0]
            ));
        sysIntr = (UINT32) SYSINTR_UNDEFINED;
        goto cleanUp;
    }

    if (((flags & OAL_INTR_TRANSLATE) != 0) &&
        (g_oalIrq2SysIntr[pIrqs[0]] != SYSINTR_UNDEFINED))
    {
        sysIntr = g_oalIrq2SysIntr[pIrqs[0]];
        
        // Check remaining IRQs
        for (i = 1; i < count; i++)
        {
            if (pIrqs[i] >= IRQ_MAXIMUM)
            {
                OALMSG(OAL_ERROR, (L"ERROR: OALIntrRequestSysIntr: "
                    L"IRQ %d out of range\r\n", pIrqs[i]
                    ));
                sysIntr = (UINT32) SYSINTR_UNDEFINED;
                goto cleanUp;
            }

            // If it is mapped to same SYSINTR, we are fine
            if (g_oalIrq2SysIntr[pIrqs[i]] == sysIntr) 
                continue;
            
            // It isn't mapped or mapped to some other SYSINTR, error...
            sysIntr = (UINT32) SYSINTR_UNDEFINED;
        }

        goto cleanUp;
    }

    // Find next available SYSINTR value...
    for (sysIntr = SYSINTR_FIRMWARE; sysIntr < SYSINTR_MAXIMUM; sysIntr++)
    {
        if (g_oalSysIntr2Irq[sysIntr][0] == OAL_INTR_IRQ_UNDEFINED) 
            break;
    }

    // Any available SYSINTRs left?
    if (sysIntr >= SYSINTR_MAXIMUM)
    {
        OALMSG(OAL_ERROR, (L"ERROR: OALIntrRequestSysIntr: "
            L"No available SYSINTR found\r\n"
            ));
        sysIntr = (UINT32) SYSINTR_UNDEFINED;
        goto cleanUp;
    }

    // Make SYSINTR -> IRQs association
    for (i = 0; i < count; i++)
    {
        if (pIrqs[i] >= IRQ_MAXIMUM)
        {
            OALMSG(OAL_ERROR, (L"ERROR: OALIntrRequestSysIntr: "
                L"IRQ %d out of range\r\n", pIrqs[i]
                ));

            for (j = 0; j < i; j++)
            {
                g_oalSysIntr2Irq[sysIntr][j] = (UINT32) OAL_INTR_IRQ_UNDEFINED;
            }

            sysIntr = (UINT32) SYSINTR_UNDEFINED;
            goto cleanUp;
        }

        g_oalSysIntr2Irq[sysIntr][i] = pIrqs[i];
    }

    while (i < IRQ_PER_SYSINTR)
    {
        g_oalSysIntr2Irq[sysIntr][i++] = (UINT32) OAL_INTR_IRQ_UNDEFINED;
    }

    // Make IRQ -> SYSINTR association, only if not multiply-mapped
    if ((flags & OAL_INTR_DYNAMIC) != 0) 
        goto cleanUp;
    
    for (i = 0; i < count; i++)
    {
        if ((g_oalIrq2SysIntr[pIrqs[i]] == SYSINTR_UNDEFINED) ||
            ((flags & OAL_INTR_FORCE_STATIC) != 0))
        {
            g_oalIrq2SysIntr[pIrqs[i]] = sysIntr;
        }
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
    BOOL rc = TRUE;
    UINT32 i, irq;

    OALMSG(OAL_INTR&&OAL_FUNC, (L"+OALIntrReleaseSysIntr(%d)\r\n", sysIntr));

    if ((sysIntr < SYSINTR_FIRMWARE) || (sysIntr >= SYSINTR_MAXIMUM))
    {
        OALMSG(OAL_ERROR, (L"ERROR: OALIntrReleaseSysIntr: "
            L"Invalid sysIntr value %d\r\n", sysIntr
            ));
        rc = FALSE;
        goto cleanUp;
    }

    // Release all mapping for given SYSINTR
    for (i = 0; i < IRQ_PER_SYSINTR; i++)
    {
        if (g_oalSysIntr2Irq[sysIntr][i] == OAL_INTR_IRQ_UNDEFINED)
            continue;
        
        // Remove the SYSINTR -> IRQ mapping
        irq = g_oalSysIntr2Irq[sysIntr][i];
        g_oalSysIntr2Irq[sysIntr][i] = (UINT32) OAL_INTR_IRQ_UNDEFINED;

        // If there is IRQ -> SYSINTR for this IRQ release it also
        if (irq >= IRQ_MAXIMUM)
        {
            rc = FALSE;
            continue;
        }

        if (g_oalIrq2SysIntr[irq] == sysIntr)
        {
            g_oalIrq2SysIntr[irq] = (UINT32) SYSINTR_UNDEFINED;
        }
    }

cleanUp:
    OALMSG(OAL_INTR&&OAL_FUNC, (L"-OALIntrReleaseSysIntr(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  OEMInterruptEnable
//
//  This function enables the IRQ given its corresponding SysIntr value.
//  Function returns true if SysIntr is valid, else false.
//
BOOL OEMInterruptEnable(DWORD sysIntr, VOID* pData, DWORD dataSize)
{
    BOOL rc = FALSE;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pData);
    UNREFERENCED_PARAMETER(dataSize);

    OALMSG(OAL_INTR&&OAL_VERBOSE, 
        (L"+OEMInterruptEnable(%d, 0x%x, %d)\r\n", sysIntr, pData, dataSize
        ));
    if (sysIntr >= SYSINTR_MAXIMUM)
    {
        OALMSG(OAL_ERROR, (L"ERROR: OEMInterruptEnable: "
            L"Invalid sysIntr value %d\r\n", sysIntr
            ));
        goto cleanUp;
    }

    // SYSINTR_VMINI & SYSINTR_TIMING are special cases
    if ((sysIntr == SYSINTR_VMINI) || (sysIntr == SYSINTR_TIMING))
    {
        rc = TRUE;
        goto cleanUp;
    }

    // Enable interrupts
    rc = OALIntrEnableIrqs(IRQ_PER_SYSINTR, g_oalSysIntr2Irq[sysIntr]);

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
VOID OEMInterruptDisable(DWORD sysIntr)
{
    OALMSG(OAL_INTR&&OAL_VERBOSE, (L"+OEMInterruptDisable(%d)\r\n", sysIntr));
    if (sysIntr >= SYSINTR_MAXIMUM)
    {
        OALMSG(OAL_ERROR, (L"ERROR: OEMInterruptDisable: "
            L"Invalid sysIntr value %d\r\n", sysIntr
            ));
        goto cleanUp;
    }

    // Disable interrupts
    OALIntrDisableIrqs(IRQ_PER_SYSINTR, g_oalSysIntr2Irq[sysIntr]);
cleanUp:    
    OALMSG(OAL_INTR&&OAL_VERBOSE, (L"-OEMInterruptDisable\r\n"));
}

//------------------------------------------------------------------------------
//
//  Function:  OEMInterruptMask
//
//  This function masks the IRQ given its corresponding SysIntr value.
//
//
VOID OEMInterruptMask(DWORD sysIntr, BOOL mask)
{
    OALMSG(OAL_INTR&&OAL_VERBOSE, (
        L"+OEMInterruptMask(%d, %d)\r\n", sysIntr, mask
        ));
    if (sysIntr >= SYSINTR_MAXIMUM)
    {
        OALMSG(OAL_ERROR, (L"ERROR: OEMInterruptMask: "
            L"Invalid sysIntr value %d\r\n", sysIntr
            ));
        goto cleanUp;
    }

    // Based on mask enable or disable
    if (mask)
    {
        OALIntrDisableIrqs(IRQ_PER_SYSINTR, g_oalSysIntr2Irq[sysIntr]);
    }
    else 
    {
        OALIntrEnableIrqs(IRQ_PER_SYSINTR, g_oalSysIntr2Irq[sysIntr]);
    }
cleanUp:    
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
VOID OEMInterruptDone(DWORD sysIntr)
{
    OALMSG(OAL_INTR&&OAL_VERBOSE, (L"+OEMInterruptDone(%d)\r\n", sysIntr));
    if (sysIntr >= SYSINTR_MAXIMUM)
    {
        OALMSG(OAL_ERROR, (L"ERROR: OEMInterruptDone: "
            L"Invalid sysIntr value %d\r\n", sysIntr
            ));
        goto cleanUp;
    }

    // Re-enable interrupts
    OALIntrDoneIrqs(IRQ_PER_SYSINTR, g_oalSysIntr2Irq[sysIntr]);
cleanUp:    
    OALMSG(OAL_INTR&&OAL_VERBOSE, (L"-OEMInterruptDone\r\n"));
}

//------------------------------------------------------------------------------

