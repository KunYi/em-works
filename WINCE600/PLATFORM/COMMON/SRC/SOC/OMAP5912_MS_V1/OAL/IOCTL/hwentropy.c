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
//  File:  hwentropy.c
//
//  This file implements the IOCTL_HAL_GET_HWENTROPY handler.
//
#include <windows.h>
#include <oal.h>
#include <omap5912.h>


//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalGetHWEntropy
//
//  Implements the IOCTL_HAL_GET_HWENTROPY handler. This function creates a
//  64-bit value which is unique to the hardware.  This value never changes.
//
BOOL OALIoCtlHalGetHWEntropy( 
    UINT32 dwIoControlCode, VOID *lpInBuf, UINT32 nInBufSize, VOID *lpOutBuf, 
    UINT32 nOutBufSize, UINT32* lpBytesReturned)
{
    OMAP5912_DEVICE_ID_REGS *pDevRegs = OALPAtoUA(OMAP5912_DEVICE_ID_REGS_PA);
    UINT32 DieID[2];
    BOOL rc = FALSE;

    // Check buffer size
    if (lpBytesReturned != NULL) 
    {
        *lpBytesReturned = sizeof(DieID);
    }

    if (!lpOutBuf && nOutBufSize > 0)
    {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        OALMSG(OAL_WARN, (L"WARN: OALIoCtlHalGetHWEntropy: Invalid parameters\r\n"));
    }
    else if (!lpOutBuf || nOutBufSize < sizeof(DieID))
    {
        NKSetLastError(ERROR_INSUFFICIENT_BUFFER);
        OALMSG(OAL_WARN, (L"WARN: OALIoCtlHalGetHWEntropy: Buffer too small\r\n"));
    }
    else
    {
        // Get die ids
        DieID[0] = INREG32(&pDevRegs->DIE_ID_LSB);
        DieID[1] = INREG32(&pDevRegs->DIE_ID_MSB);

        // Copy pattern to output buffer
        memcpy(lpOutBuf, DieID, sizeof(DieID));

        // We are done
        rc = TRUE;
    }

    // Indicate status
    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"-OALIoCtlHalGetHWEntropy(rc = %d)\r\n", rc));
    return rc;
}

