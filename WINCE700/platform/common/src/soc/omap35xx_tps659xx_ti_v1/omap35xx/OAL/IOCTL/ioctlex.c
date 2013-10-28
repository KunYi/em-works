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
//  File:  ioctlex.c
//
//  This file This file implements additional OAL calls.
//
#include <windows.h>
#include <nkintr.h>
#include <oal.h>
#include <oalex.h>
#include <omap35xx.h>

//------------------------------------------------------------------------------
extern UINT32 OALGetSiliconIdCode();

//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHALPostInit
//
//  
BOOL OALIoCtlHALPostInit(
    UINT32 code, 
    VOID *pInBuffer,
    UINT32 inSize, 
    VOID *ppOutBuffer, 
    UINT32 outSize, 
    UINT32 *pOutSize
    )
{
    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"+OALIoCtlHALPostInit\r\n"));
    
    OALPowerPostInit();

    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"-OALIoCtlHALPostInit(rc = %d)\r\n", TRUE));
    return TRUE;
}

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
    OALMSG(OAL_IOCTL&&OAL_FUNC, (
        L"-OALIoCtlHalIrq2Sysintr(rc = %d)\r\n", rc
    ));
    return rc;

}

//------------------------------------------------------------------------------
//
//  Function: OALIoCtlHalGetCpuID
//
//
BOOL OALIoCtlHalGetCpuID(UINT32 code, VOID *pInpBuffer,
    UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize)
{
    BOOL  rc = FALSE;

    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"+OALIoCtlHalGetCpuID\r\n"));

    // Check input parameters
    if (
        pOutBuffer == NULL || outSize < sizeof(UINT32)
    ) {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        OALMSG(OAL_WARN, (
            L"WARN: IOCTL_HAL_GET_CPUID invalid parameters\r\n"
        ));
        goto cleanUp;
    }

    //  Get CPU Silicon ID value
    ((DWORD*)pOutBuffer)[0] = OALGetSiliconIdCode();

    rc = TRUE;

cleanUp:
    OALMSG(OAL_IOCTL&&OAL_FUNC, (
        L"-OALIoCtlHalGetCpuID(rc = %d)\r\n", rc
    ));
    return rc;

}

//------------------------------------------------------------------------------
//
//  Function: OALIoCtlHalGetDieID
//
//
BOOL OALIoCtlHalGetDieID(UINT32 code, VOID *pInpBuffer,
    UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize)
{
    BOOL                        rc = FALSE;
    OMAP_IDCORE_REGS*           pIdRegs;    
    IOCTL_HAL_GET_DIEID_OUT*    pDieID;
    
    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"+OALIoCtlHalGetDieID\r\n"));

    // Check input parameters
    if (
        pOutBuffer == NULL || outSize < sizeof(IOCTL_HAL_GET_DIEID_OUT)
    ) {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        OALMSG(OAL_WARN, (
            L"WARN: IOCTL_HAL_GET_DIEID invalid parameters\r\n"
        ));
        goto cleanUp;
    }

    //  Get CPU DIE ID values
    pIdRegs = OALPAtoUA(OMAP_IDCORE_REGS_PA);

    //  Copy to passed in struct
    pDieID = (IOCTL_HAL_GET_DIEID_OUT*)pOutBuffer;
    
    pDieID->dwIdCode    = INREG32(&pIdRegs->IDCODE);
    pDieID->dwProdID_0  = INREG32(&pIdRegs->PRODUCTION_ID_0);
    pDieID->dwProdID_1  = INREG32(&pIdRegs->PRODUCTION_ID_1);
    pDieID->dwProdID_2  = INREG32(&pIdRegs->PRODUCTION_ID_2);
    pDieID->dwProdID_3  = INREG32(&pIdRegs->PRODUCTION_ID_3);
    pDieID->dwDieID_0   = INREG32(&pIdRegs->DIE_ID_0);
    pDieID->dwDieID_1   = INREG32(&pIdRegs->DIE_ID_1);
    pDieID->dwDieID_2   = INREG32(&pIdRegs->DIE_ID_2);
    pDieID->dwDieID_3   = INREG32(&pIdRegs->DIE_ID_3);
    
    rc = TRUE;

cleanUp:
    OALMSG(OAL_IOCTL&&OAL_FUNC, (
        L"-OALIoCtlHalGetDieID(rc = %d)\r\n", rc
    ));
    return rc;

}


//------------------------------------------------------------------------------
//
//  Function: OALIoCtlHalI2CMode
//  
//  *pInpBuffer is a pointer to DWORD which has the following values per byte
//  0 = ignore
//  1 = SS mode
//  2 = FS mode
//  3 = HS mode
//  
//
//  | 31-25(i2c4) | 24-16(i2c3) | 15-8(i2c2) | 7-0(i2c1) |
//
BOOL OALIoCtlHalI2CMode(UINT32 code, VOID *pInpBuffer,
    UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize)
{
    BOOL  rc = FALSE;

    return rc;
}


//------------------------------------------------------------------------------
//
//  Function: OALIoCtlIgnore
//
BOOL OALIoCtlIgnore(UINT32 code, VOID *pInpBuffer,
    UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize)
{
    BOOL  rc = FALSE;

    return rc;
}
//------------------------------------------------------------------------------
