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
//  File: ioctl.c
//
//  This file implements Freescale MX27 SoC-specific OALIoCtlxxxx functions.
//
//------------------------------------------------------------------------------
#include <windows.h>
#include <oal.h>
#include "csp.h"

//------------------------------------------------------------------------------
// External Variables
extern PCSP_AITC_REGS g_pAITC;


//------------------------------------------------------------------------------
//
//  Function: OALIoCtlHalIrq2Sysintr
//
//
BOOL OALIoCtlHalIrq2Sysintr(UINT32 code, VOID *pInpBuffer,
    UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize)
{
    BOOL rc = FALSE;

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
    UINT32 irq, oldReg, newReg;
    PUINT32 pReg;

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
    if (irq >= OAL_INTR_IRQ_MAXIMUM)
    {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        OALMSG(OAL_WARN, (
            L"WARN: IOCTL_HAL_FORCE_IRQ invalid IRQ parameter\r\n"
        ));
        goto cleanUp;
    }

    if (irq > 32)
    {
        pReg = (PUINT)&g_pAITC->INTFRCH;
        irq -= 32;
    }
    else
    {
        pReg = (PUINT)&g_pAITC->INTFRCL;        
    }

    // Safely update AITC INTFRC register
    do
    {
        oldReg = INREG32(pReg);
        newReg = oldReg | (1U << irq);
    } while (InterlockedTestExchange(pReg, 
                        oldReg, newReg) != oldReg);        
    
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
    UINT32 irq, oldReg, newReg;
    PUINT32 pReg;

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
    if (irq >= OAL_INTR_IRQ_MAXIMUM)
    {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        OALMSG(OAL_WARN, (
            L"WARN: IOCTL_HAL_UNFORCE_IRQ invalid IRQ parameter\r\n"
        ));
        goto cleanUp;
    }

    if (irq > 32)
    {
        pReg = (PUINT)&g_pAITC->INTFRCH;
        irq -= 32;
    }
    else
    {
        pReg = (PUINT)&g_pAITC->INTFRCL;        
    }

    // Safely update AVIC INTFRC register
    do
    {
        oldReg = INREG32(pReg);
        newReg = oldReg & (~(1U << irq));
    } while (InterlockedTestExchange(pReg, 
                        oldReg, newReg) != oldReg);        
    
cleanUp:
    OALMSG(OAL_INTR&&OAL_FUNC, (
        L"+OALIoCtlHalUnforceIrq(rc = %d)\r\n", rc
    ));
    return rc;

}




