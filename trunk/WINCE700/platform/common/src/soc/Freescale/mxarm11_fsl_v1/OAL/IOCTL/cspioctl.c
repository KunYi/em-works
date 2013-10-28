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
//  Copyright (C) 2004-2007, Freescale Semiconductor, Inc. All Rights Reserved.
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

#include "mxarm11.h"

//------------------------------------------------------------------------------
// External Variables
extern UINT32 g_oalSdmaTranslate[SDMA_NUM_CHANNELS];
extern UINT32 g_oalGpioTranslate[DDK_GPIO_PORT3+1][GPIO_INTR_SOURCES_MAX];
extern UINT32 g_oalGpioMask[DDK_GPIO_PORT3+1][OAL_INTR_IRQ_MAXIMUM];
extern PCSP_AVIC_REGS g_pAVIC;


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
//  Function: OALIoCtlHalRequestDmaIrqMap
//
//
BOOL OALIoCtlHalRequestDmaIrqMap(UINT32 code, VOID *pInpBuffer,
    UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize)
{
    BOOL rc = FALSE;
    PDMA_IRQ_MAP_PARMS pDmaParms;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(code);
    UNREFERENCED_PARAMETER(pOutBuffer);
    UNREFERENCED_PARAMETER(outSize);
    UNREFERENCED_PARAMETER(pOutSize);

    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"+OALIoCtlHalRequestDmaIrqMap\r\n"));

    // Check input parameters
    if (pInpBuffer == NULL || inpSize < sizeof(DMA_IRQ_MAP_PARMS))
    {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        OALMSG(OAL_WARN, (
            L"WARN: IOCTL_HAL_REQUEST_DMA_IRQ_MAP invalid parameters\r\n"
        ));
        goto cleanUp;
    }

    pDmaParms = (PDMA_IRQ_MAP_PARMS) pInpBuffer;
    if (pDmaParms->irq >= OAL_INTR_IRQ_MAXIMUM)
    {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        OALMSG(OAL_WARN, (
            L"WARN: IOCTL_HAL_REQUEST_DMA_IRQ_MAP invalid IRQ parameter\r\n"
        ));
        goto cleanUp;
    }

    if (pDmaParms->chan >= SDMA_NUM_CHANNELS)
    {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        OALMSG(OAL_WARN, (
            L"WARN: IOCTL_HAL_REQUEST_DMA_IRQ_MAP invalid DMA channel parameter\r\n"
        ));
        goto cleanUp;
    }

    // Make sure another mapping does not already exist
    if (g_oalSdmaTranslate[pDmaParms->chan] != OAL_INTR_IRQ_UNDEFINED)
    {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        OALMSG(OAL_WARN, (
            L"WARN: IOCTL_HAL_REQUEST_DMA_IRQ_MAP mapping for IRQ not available\r\n"
        ));
        goto cleanUp;
    }

    // Grant the mapping
    g_oalSdmaTranslate[pDmaParms->chan] = pDmaParms->irq;
    rc = TRUE;

cleanUp:
    OALMSG(OAL_INTR&&OAL_FUNC, (
        L"+OALIoCtlHalRequestDmaIrqMap(rc = %d)\r\n", rc
    ));
    return rc;

}


//------------------------------------------------------------------------------
//
//  Function: OALIoCtlHalReleaseDmaIrqMap
//
//
BOOL OALIoCtlHalReleaseDmaIrqMap(UINT32 code, VOID *pInpBuffer,
    UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize)
{
    BOOL rc = FALSE;
    UINT8 chan;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(code);
    UNREFERENCED_PARAMETER(pOutBuffer);
    UNREFERENCED_PARAMETER(outSize);
    UNREFERENCED_PARAMETER(pOutSize);

    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"+OALIoCtlHalReleaseDmaIrqMap\r\n"));

    // Check input parameters
    if (pInpBuffer == NULL || inpSize < sizeof(UINT8))
    {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        OALMSG(OAL_WARN, (
            L"WARN: IOCTL_HAL_RELEASE_DMA_IRQ_MAP invalid parameters\r\n"
        ));
        goto cleanUp;
    }

    chan = *((UINT8 *) pInpBuffer);
    if (chan >= SDMA_NUM_CHANNELS)
    {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        OALMSG(OAL_WARN, (
            L"WARN: IOCTL_HAL_RELEASE_DMA_IRQ_MAP invalid DMA channel parameter\r\n"
        ));
        goto cleanUp;
    }

    // Release the mapping
    g_oalSdmaTranslate[chan] = (UINT32) OAL_INTR_IRQ_UNDEFINED;
    rc = TRUE;

cleanUp:
    OALMSG(OAL_INTR&&OAL_FUNC, (
        L"+OALIoCtlHalReleaseDmaIrqMap(rc = %d)\r\n", rc
    ));
    return rc;

}


//------------------------------------------------------------------------------
//
//  Function: OALIoCtlHalRequestGpioIrqMap
//
//
BOOL OALIoCtlHalRequestGpioIrqMap(UINT32 code, VOID *pInpBuffer,
    UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize)
{
    BOOL rc = FALSE;
    PGPIO_IRQ_MAP_PARMS pParms;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(code);
    UNREFERENCED_PARAMETER(pOutBuffer);
    UNREFERENCED_PARAMETER(outSize);
    UNREFERENCED_PARAMETER(pOutSize);

    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"+OALIoCtlHalRequestGpioIrqMap\r\n"));

    // Check input parameters
    if (pInpBuffer == NULL || inpSize < sizeof(GPIO_IRQ_MAP_PARMS))
    {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        OALMSG(OAL_WARN, (
            L"WARN: IOCTL_HAL_REQUEST_GPIO_IRQ_MAP invalid parameters\r\n"
        ));
        goto cleanUp;
    }

    pParms = (PGPIO_IRQ_MAP_PARMS) pInpBuffer;
    if (pParms->irq >= OAL_INTR_IRQ_MAXIMUM)
    {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        OALMSG(OAL_WARN, (
            L"WARN: IOCTL_HAL_REQUEST_GPIO_IRQ_MAP invalid IRQ parameter\r\n"
        ));
        goto cleanUp;
    }

    if (pParms->port > DDK_GPIO_PORT3)
    {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        OALMSG(OAL_WARN, (
            L"WARN: IOCTL_HAL_REQUEST_GPIO_IRQ_MAP invalid GPIO port parameter\r\n"
        ));
        goto cleanUp;
    }

    if (pParms->pin >= GPIO_PINS_PER_PORT)
    {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        OALMSG(OAL_WARN, (
            L"WARN: IOCTL_HAL_REQUEST_GPIO_IRQ_MAP invalid GPIO pin parameter\r\n"
        ));
        goto cleanUp;
    }

    // Make sure another mapping does not already exist
    if (g_oalGpioTranslate[pParms->port][pParms->pin] != OAL_INTR_IRQ_UNDEFINED)
    {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        OALMSG(OAL_WARN, (
            L"WARN: IOCTL_HAL_REQUEST_GPIO_IRQ_MAP mapping for GPIO not available\r\n"
        ));
        goto cleanUp;
    }

    // Grant the mapping
    g_oalGpioTranslate[pParms->port][pParms->pin] = pParms->irq;
    g_oalGpioMask[pParms->port][pParms->irq] |= (1U << (pParms->pin));
    rc = TRUE;

cleanUp:
    OALMSG(OAL_INTR&&OAL_FUNC, (
        L"+OALIoCtlHalRequestGpioIrqMap(rc = %d)\r\n", rc
    ));
    return rc;

}


//------------------------------------------------------------------------------
//
//  Function: OALIoCtlHalReleaseGpioIrqMap
//
//
BOOL OALIoCtlHalReleaseGpioIrqMap(UINT32 code, VOID *pInpBuffer,
    UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize)
{
    BOOL rc = FALSE;
    PGPIO_IRQ_MAP_PARMS pParms;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(code);
    UNREFERENCED_PARAMETER(pOutBuffer);
    UNREFERENCED_PARAMETER(outSize);
    UNREFERENCED_PARAMETER(pOutSize);

    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"+OALIoCtlHalReleaseGpioIrqMap\r\n"));

    // Check input parameters
    if (pInpBuffer == NULL || inpSize < sizeof(GPIO_IRQ_MAP_PARMS))
    {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        OALMSG(OAL_WARN, (
            L"WARN: IOCTL_HAL_RELEASE_GPIO_IRQ_MAP invalid parameters\r\n"
        ));
        goto cleanUp;
    }

    pParms = (PGPIO_IRQ_MAP_PARMS) pInpBuffer;
    
    if (pParms->port > DDK_GPIO_PORT3)
    {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        OALMSG(OAL_WARN, (
            L"WARN: IOCTL_HAL_RELEASE_GPIO_IRQ_MAP invalid GPIO port parameter\r\n"
        ));
        goto cleanUp;
    }

    if (pParms->pin >= GPIO_PINS_PER_PORT)
    {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        OALMSG(OAL_WARN, (
            L"WARN: IOCTL_HAL_RELEASE_GPIO_IRQ_MAP invalid GPIO pin parameter\r\n"
        ));
        goto cleanUp;
    }

    // Release the mapping
    g_oalGpioTranslate[pParms->port][pParms->pin] = (UINT32) OAL_INTR_IRQ_UNDEFINED;
    g_oalGpioMask[pParms->port][pParms->irq] &= (~(1U << (pParms->pin)));
    
    rc = TRUE;

cleanUp:
    OALMSG(OAL_INTR&&OAL_FUNC, (
        L"+OALIoCtlHalReleaseGpioIrqMap(rc = %d)\r\n", rc
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
        pReg = &g_pAVIC->INTFRCH;
        irq -= 32;
    }
    else
    {
        pReg = &g_pAVIC->INTFRCL;        
    }

    // Safely update AVIC INTFRC register
    do
    {
        oldReg = INREG32(pReg);
        newReg = oldReg | (1U << irq);
    } while ((UINT32) InterlockedTestExchange((LPLONG) pReg, 
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
        pReg = &g_pAVIC->INTFRCH;
        irq -= 32;
    }
    else
    {
        pReg = &g_pAVIC->INTFRCL;        
    }

    // Safely update AVIC INTFRC register
    do
    {
        oldReg = INREG32(pReg);
        newReg = oldReg & (~(1U << irq));
    } while ((UINT32) InterlockedTestExchange((LPLONG) pReg, 
                        oldReg, newReg) != oldReg);        
    
cleanUp:
    OALMSG(OAL_INTR&&OAL_FUNC, (
        L"+OALIoCtlHalUnforceIrq(rc = %d)\r\n", rc
    ));
    return rc;

}




