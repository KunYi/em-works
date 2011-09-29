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
#include <omap2420.h>


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
    BOOL rc = FALSE;

#if 0
    OMAP2420_DEVICE_ID_REGS *pDeviceIDRegs = OALPAtoUA(OMAP2420_DEVICE_ID_REGS_PA);
    UINT32 DieID[2];

    // Check buffer size
    if (lpBytesReturned != NULL) 
    {
        *lpBytesReturned = sizeof(DieID);
    }

    if (lpOutBuf == NULL || nOutBufSize < sizeof(DieID))
    {
        NKSetLastError(ERROR_INSUFFICIENT_BUFFER);
        OALMSG(OAL_WARN, (L"WARN: OALIoCtlHalGetHWEntropy: Buffer too small\r\n"));
		goto cleanup;
    }
    else
    {
        // Get die ids
        DieID[0] = INREG32(&pDeviceIDRegs->OMAP_DIE_ID_0);
        DieID[1] = INREG32(&pDeviceIDRegs->OMAP_DIE_ID_1);

        // Copy pattern to output buffer
        memcpy(lpOutBuf, DieID, sizeof(DieID));

    }
#endif
        // We are done
        rc = TRUE;
//cleanup: change it when OMAP2420 specifics are implemented
    // Indicate status
    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"-OALIoCtlHalGetHWEntropy(rc = %d)\r\n", rc));
    return rc;
}

