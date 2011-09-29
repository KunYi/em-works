//------------------------------------------------------------------------------
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
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File: ioctl.c
//
//  This file implements Freescale SoC-specific OALIoCtlxxxx functions.
//
//------------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <oal.h>
#pragma warning(pop)

#include "csp.h"

//------------------------------------------------------------------------------
// External Variables
extern PCSP_TZIC_REGS g_pTZIC;


//------------------------------------------------------------------------------
//
//  Function: OALIoCtlHalIrq2Sysintr
//
//
BOOL OALIoCtlHalIrq2Sysintr(UINT32 code, VOID *pInpBuffer,
    UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize)
{
    BOOL rc = FALSE;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(code);

    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"+OALIoCtlHalIrq2Sysintr\r\n"));

    // Check input parameters
    if (
        pInpBuffer == NULL || inpSize < sizeof(UINT32) ||
        pOutBuffer == NULL || outSize < sizeof(UINT32)
    ) {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        OALMSG(OAL_WARN, (
            L"WARN: IOCTL_HAL_IRQ2SYSINTR invalid parameters\r\n"
        ));
        goto cleanUp;
    }

    // Call function itself
    *(UINT32*)pOutBuffer = OALIntrTranslateIrq(*(UINT32*)pInpBuffer);
    if (pOutSize != NULL) *pOutSize = sizeof(UINT32);
    rc = TRUE;

cleanUp:
    OALMSG(OAL_INTR&&OAL_FUNC, (
        L"+OALIoCtlHalIrq2Sysintr(rc = %d)\r\n", rc
    ));
    return rc;

}


//------------------------------------------------------------------------------
//
//  Function: OALIoCtlHalForceIrq
//
//
BOOL OALIoCtlHalForceIrq(UINT32 code, VOID *pInpBuffer,
    UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize)
{
    BOOL rc = FALSE;
    UINT32 irq;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(code);
    UNREFERENCED_PARAMETER(pOutBuffer);
    UNREFERENCED_PARAMETER(outSize);
    UNREFERENCED_PARAMETER(pOutSize);

    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"+OALIoCtlHalForceIrq\r\n"));

    // Check input parameters
    if (pInpBuffer == NULL || inpSize < sizeof(UINT32))
    {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        OALMSG(OAL_WARN, (
            L"WARN: IOCTL_HAL_FORCE_IRQ invalid parameters\r\n"
        ));
        goto cleanUp;
    }

    irq = *((PUINT32) pInpBuffer);
    if (irq >= IRQ_RESERVED127)
    {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        OALMSG(OAL_WARN, (
            L"WARN: IOCTL_HAL_FORCE_IRQ invalid IRQ parameter\r\n"
        ));
        goto cleanUp;
    }

    OUTREG32(&g_pTZIC->SWINT, 
        CSP_BITFVAL(TZIC_SWINT_INTNEG, TZIC_SWINT_INTNEG_ASSERTED) | irq);
    
cleanUp:
    OALMSG(OAL_INTR&&OAL_FUNC, (
        L"+OALIoCtlHalForceIrq(rc = %d)\r\n", rc
    ));
    return rc;

}


//------------------------------------------------------------------------------
//
//  Function: OALIoCtlHalUnforceIrq
//
//
BOOL OALIoCtlHalUnforceIrq(UINT32 code, VOID *pInpBuffer,
    UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize)
{
    BOOL rc = FALSE;
    UINT32 irq;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(code);
    UNREFERENCED_PARAMETER(pOutBuffer);
    UNREFERENCED_PARAMETER(outSize);
    UNREFERENCED_PARAMETER(pOutSize);

    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"+OALIoCtlHalUnforceIrq\r\n"));

    // Check input parameters
    if (pInpBuffer == NULL || inpSize < sizeof(UINT32))
    {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        OALMSG(OAL_WARN, (
            L"WARN: IOCTL_HAL_UNFORCE_IRQ invalid parameters\r\n"
        ));
        goto cleanUp;
    }

    irq = *((PUINT32) pInpBuffer);
    if (irq > IRQ_RESERVED127)
    {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        OALMSG(OAL_WARN, (
            L"WARN: IOCTL_HAL_UNFORCE_IRQ invalid IRQ parameter\r\n"
        ));
        goto cleanUp;
    }

    OUTREG32(&g_pTZIC->SWINT, 
        CSP_BITFVAL(TZIC_SWINT_INTNEG, TZIC_SWINT_INTNEG_NEGATED) | irq);
    
    
cleanUp:
    OALMSG(OAL_INTR&&OAL_FUNC, (
        L"+OALIoCtlHalUnforceIrq(rc = %d)\r\n", rc
    ));
    return rc;

}




