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
//  File:  hwentropy.c
//
//  This file implements the IOCTL_HAL_GET_HWENTROPY handler.
//
#include <windows.h>
#include <oal.h>
#include <oalex.h>
#include <omap35xx.h>


//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalGetHWEntropy
//
//  Implements the IOCTL_HAL_GET_HWENTROPY handler. This function creates a
//  64-bit value which is unique to the hardware.  This value never changes.
//
BOOL
OALIoCtlHalGetHWEntropy( 
    UINT32 code, 
    VOID *pInpBuffer, 
    UINT32 inpSize, 
    VOID *pOutBuffer,
    UINT32 outSize, 
    UINT32 *pOutSize
    )
{
    BOOL    rc = FALSE;
    UINT32  size = 2*sizeof(DWORD);
    UINT32  *pID = NULL;


    // Check buffer size
    if (pOutSize != NULL) *pOutSize = size;

    if (pOutBuffer == NULL || outSize < size)
        {
        OALMSG(OAL_WARN, (L"WARN: OALIoCtlHalGetHWEntropy: "
            L"Buffer too small\r\n"
            ));
        NKSetLastError(ERROR_INSUFFICIENT_BUFFER);
        goto cleanUp;
    }

    // Get a 64 bit (2 DWORDs) HW generated random number that never changes
    pID = OALArgsQuery(OAL_ARGS_QUERY_HWENTROPY);
    if( pID )
    {
        // Copy pattern to output buffer
        memcpy(pOutBuffer, pID, size);

        // We are done
        rc = TRUE;
    }

cleanUp:
    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"-OALIoCtlHalGetHWEntropy(rc = %d)\r\n", rc));

    return rc;
}

//------------------------------------------------------------------------------

