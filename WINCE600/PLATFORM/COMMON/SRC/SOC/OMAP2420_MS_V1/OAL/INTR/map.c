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
//  File: map.c
//
//  This file implements functionality related to mapping between IRQ and
//  SYSINTR. There is custom implementation for OMAP2420 because hardware
//  supports 96 different IRQs compared to maximal 64 IRQs allowed in generic
//  implementation. The implementation also supports multiple IRQ per one
//  SYSINTR mapping.
//
#include <windows.h>
#include <nkintr.h>
#include <oal.h>
#include <omap2420.h>


//------------------------------------------------------------------------------
//
//  Globals: g_oalSysIntr2Irq/g_oalIrq2SysIntr
//
static UINT32 g_oalSysIntr2Irq[SYSINTR_MAXIMUM][OMAP2420_IRQ_PER_SYSINTR];
static UINT32 g_oalIrq2SysIntr[OMAP2420_IRQ_MAXIMUM];

//------------------------------------------------------------------------------
//
//  Globals: g_oalSysIntr2Irq/g_oalIrq2SysIntr
//
static UINT32 g_oalSysIntr2Irq[SYSINTR_MAXIMUM][OMAP2420_IRQ_PER_SYSINTR];
static UINT32 g_oalIrq2SysIntr[OMAP2420_IRQ_MAXIMUM];

//------------------------------------------------------------------------------
//
//  Function:  OALIntrMapInit
//
//  This function must be called from OALInterruptInit to initialize mapping
//  between IRQ and SYSINTR. It simply initialize mapping arrays.
//
VOID
OALIntrMapInit(
    )
{
    UINT32 i, j;

    OALMSG(OAL_FUNC&&OAL_INTR, (L"+OEMInterruptMapInit\r\n"));

    // Initialize interrupt maps
    for (i = 0; i < SYSINTR_MAXIMUM; i++)
        {
        for (j = 0; j < OMAP2420_IRQ_PER_SYSINTR; j++)
            {
            g_oalSysIntr2Irq[i][j] = OAL_INTR_IRQ_UNDEFINED;
            }
        }
    for (i = 0; i < OMAP2420_IRQ_MAXIMUM; i++)
        {
        g_oalIrq2SysIntr[i] = SYSINTR_UNDEFINED;
        }

    OALMSG(OAL_FUNC&&OAL_INTR, (L"-OEMInterruptMapInit\r\n"));
}

//------------------------------------------------------------------------------
//
//  Function:   OALIntrStaticTranslate
//
//  This function sets static translation between IRQ and SYSINTR.
//
VOID
OALIntrStaticTranslate(
    UINT32 sysIntr, 
    UINT32 irq
    )
{
    UINT32 i;

    OALMSG(OAL_FUNC&&OAL_INTR, (
        L"+OALIntrStaticTranslate(%d, %d)\r\n", sysIntr, irq
        ));
    if ((irq >= OMAP2420_IRQ_MAXIMUM) || (sysIntr >= SYSINTR_MAXIMUM))
        goto cleanUp;

    for (i = 0; i < OMAP2420_IRQ_PER_SYSINTR; i++)
        {
        if (g_oalSysIntr2Irq[sysIntr][i] != OAL_INTR_IRQ_UNDEFINED) continue;
        g_oalSysIntr2Irq[sysIntr][i] = irq;
        g_oalIrq2SysIntr[irq] = sysIntr;
        break;
        }

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
BOOL
OALIntrTranslateSysIntr(
    UINT32 sysIntr, 
    UINT32 *pCount, 
    const UINT32 **ppIrqs
    )
{
    BOOL rc;

    OALMSG(OAL_INTR&&OAL_VERBOSE, (L"+OALTranslateSysIntr(%d)\r\n", sysIntr));

    // Valid SYSINTR?
    if (sysIntr >= SYSINTR_MAXIMUM)
        {
        rc = FALSE;
        goto cleanUp;
        }
    *pCount = OMAP2420_IRQ_PER_SYSINTR;
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
UINT32
OALIntrTranslateIrq(
    UINT32 irq
    )
{
    UINT32 sysIntr = SYSINTR_UNDEFINED;

    OALMSG(OAL_FUNC&&OAL_VERBOSE, (L"+OALIntrTranslateIrq(%d)\r\n", irq));

    if (irq < OMAP2420_IRQ_MAXIMUM) sysIntr = g_oalIrq2SysIntr[irq];

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
UINT32
OALIntrRequestSysIntr(
    UINT32 count, 
    const UINT32 *pIrqs, 
    UINT32 flags
    )
{
    UINT32 sysIntr, i, j;


    OALMSG(OAL_INTR&&OAL_FUNC, (
        L"+OALIntrRequestSysIntr(%d, 0x%08x, 0x%08x)\r\n", count, pIrqs, flags
        ));

    // Valid IRQ?
    if (count > OMAP2420_IRQ_PER_SYSINTR)
        {
        OALMSG(OAL_ERROR, (L"ERROR: OALIntrRequestSysIntr: "
            L"Only %d IRQs for SYSINTR is supported\r\n",
            OMAP2420_IRQ_PER_SYSINTR
            ));
        sysIntr = SYSINTR_UNDEFINED;
        goto cleanUp;
        }

    // If static mapping is requested check if we can obtain one
    if ((flags & OAL_INTR_STATIC) != 0)
        {
        for (i = 0; i < count; i++)
            {
            if (pIrqs[i] >= OMAP2420_IRQ_MAXIMUM)
                {
                OALMSG(OAL_ERROR, (L"ERROR: OALIntrRequestSysIntr: "
                    L"IRQ %d out of range\r\n", pIrqs[i]
                    ));
                sysIntr = SYSINTR_UNDEFINED;
                goto cleanUp;
                }

            if (g_oalIrq2SysIntr[pIrqs[i]] != SYSINTR_UNDEFINED)
                {
                OALMSG(OAL_ERROR, (L"ERROR: OALIntrRequestSysIntr: "
                    L"Static mapping for IRQ %d already assigned\r\n", pIrqs[i]
                    ));
                sysIntr = SYSINTR_UNDEFINED;
                goto cleanUp;
                }
            }
        }

    // If translate flag is set all IRQs must be mapped to same SYSINTR
    if (pIrqs[0] >= OMAP2420_IRQ_MAXIMUM)
        {
        OALMSG(OAL_ERROR, (L"ERROR: OALIntrRequestSysIntr: "
            L"IRQ %d out of range\r\n", pIrqs[0]
            ));
        sysIntr = SYSINTR_UNDEFINED;
        goto cleanUp;
        }

    if (((flags & OAL_INTR_TRANSLATE) != 0) &&
        (g_oalIrq2SysIntr[pIrqs[0]] != SYSINTR_UNDEFINED))
        {
        sysIntr = g_oalIrq2SysIntr[pIrqs[0]];
        // Check remaining IRQs
        for (i = 1; i < count; i++)
            {
            if (pIrqs[i] >= OMAP2420_IRQ_MAXIMUM)
                {
                OALMSG(OAL_ERROR, (L"ERROR: OALIntrRequestSysIntr: "
                    L"IRQ %d out of range\r\n", pIrqs[i]
                    ));
                sysIntr = SYSINTR_UNDEFINED;
                goto cleanUp;
                }
            // If it is mapped to same SYSINTR, we are fine
            if (g_oalIrq2SysIntr[pIrqs[i]] == sysIntr) continue;
            // It isn't mapped or mapped to some other SYSINTR, error...
            sysIntr = SYSINTR_UNDEFINED;
            }
        goto cleanUp;
        }

    // Find next available SYSINTR value...
    for (sysIntr = SYSINTR_FIRMWARE; sysIntr < SYSINTR_MAXIMUM; sysIntr++)
        {
        if (g_oalSysIntr2Irq[sysIntr][0] == OAL_INTR_IRQ_UNDEFINED) break;
        }

    // Any available SYSINTRs left?
    if (sysIntr >= SYSINTR_MAXIMUM)
        {
        OALMSG(OAL_ERROR, (L"ERROR: OALIntrRequestSysIntr: "
            L"No available SYSINTR found\r\n"
            ));
        sysIntr = SYSINTR_UNDEFINED;
        goto cleanUp;
        }

    // Make SYSINTR -> IRQs association
    for (i = 0; i < count; i++)
        {
        if (pIrqs[i] >= OMAP2420_IRQ_MAXIMUM)
            {
            OALMSG(OAL_ERROR, (L"ERROR: OALIntrRequestSysIntr: "
                L"IRQ %d out of range\r\n", pIrqs[i]
                ));
            for (j = 0; j < i; j++)
                {
                g_oalSysIntr2Irq[sysIntr][j] = OAL_INTR_IRQ_UNDEFINED;
                }
            sysIntr = SYSINTR_UNDEFINED;
            goto cleanUp;
            }
        g_oalSysIntr2Irq[sysIntr][i] = pIrqs[i];
        }
    while (i < OMAP2420_IRQ_PER_SYSINTR)
        {
        g_oalSysIntr2Irq[sysIntr][i++] = OAL_INTR_IRQ_UNDEFINED;
        }

    // Make IRQ -> SYSINTR association, only if not multiply-mapped
    if ((flags & OAL_INTR_DYNAMIC) != 0) goto cleanUp;
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
BOOL 
OALIntrReleaseSysIntr(
    UINT32 sysIntr
    )
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
    for (i = 0; i < OMAP2420_IRQ_PER_SYSINTR; i++)
        {
        if (g_oalSysIntr2Irq[sysIntr][i] == OAL_INTR_IRQ_UNDEFINED) continue;
        // Remove the SYSINTR -> IRQ mapping
        irq = g_oalSysIntr2Irq[sysIntr][i];
        g_oalSysIntr2Irq[sysIntr][i] = OAL_INTR_IRQ_UNDEFINED;
        // If there is IRQ -> SYSINTR for this IRQ release it also
        if (irq >= OMAP2420_IRQ_MAXIMUM)
            {
            rc = FALSE;
            continue;
            }
        if (g_oalIrq2SysIntr[irq] == sysIntr)
            {
            g_oalIrq2SysIntr[irq] = SYSINTR_UNDEFINED;
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
    rc = OALIntrEnableIrqs(OMAP2420_IRQ_PER_SYSINTR, g_oalSysIntr2Irq[sysIntr]);

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
    OALIntrDisableIrqs(OMAP2420_IRQ_PER_SYSINTR, g_oalSysIntr2Irq[sysIntr]);

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
        OALIntrDisableIrqs(OMAP2420_IRQ_PER_SYSINTR, g_oalSysIntr2Irq[sysIntr]);
        }
    else 
        {
        OALIntrEnableIrqs(OMAP2420_IRQ_PER_SYSINTR, g_oalSysIntr2Irq[sysIntr]);
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
    OALIntrDoneIrqs(OMAP2420_IRQ_PER_SYSINTR, g_oalSysIntr2Irq[sysIntr]); 
    
    OALMSG(OAL_INTR&&OAL_VERBOSE, (L"-OEMInterruptDone\r\n"));
}

//------------------------------------------------------------------------------

