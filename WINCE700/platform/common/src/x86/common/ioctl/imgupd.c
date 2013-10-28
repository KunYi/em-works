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
//  File:  imgupd.c
//
//  This file implements the IOCTL_HAL_UPDATE_MODE handler.
//
#include <windows.h>
#include <oal.h>
#include <bootArg.h>

//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalUpdateMode
//
//  Implements the IOCTL_HAL_UPDATE_MODE handler. 
//  This function gets/sets the RAM-based update mode flag.
//
BOOL
x86IoCtlHalUpdateMode( 
    UINT32 code,
    VOID *pInpBuffer, 
    UINT32 inpSize, 
    VOID *pOutBuffer, 
    UINT32 outSize, 
    UINT32 *pOutSize
    )
{
    BOOL rc = FALSE;
    BOOT_ARGS **ppBootArgs = (BOOT_ARGS **)BOOT_ARG_PTR_LOCATION;
    BOOT_ARGS *pBootArgs = OALPAtoCA((UINT32)*ppBootArgs);

    UNREFERENCED_PARAMETER(outSize);
    UNREFERENCED_PARAMETER(pOutBuffer);
    UNREFERENCED_PARAMETER(code);

    if (pOutSize != NULL) *pOutSize = 0;

    // Verify the input
    if ((pInpBuffer == NULL) || (inpSize != sizeof(BOOL)))
        {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        OALMSG(OAL_WARN, (
            L"ERROR: OALIoCtlHalUpdateMode: Invalid input buffer\r\n"
            ));
        goto cleanUp;
        }

    // Update reused boot argument parameter
    pBootArgs->NANDBootFlags = *((BOOL*)pInpBuffer) ? 0xFF : 0x00;

    RETAILMSG(TRUE, (
        L"pBootArgs 0x%08x -> 0x%02x\r\n", &pBootArgs->NANDBootFlags, 
        pBootArgs->NANDBootFlags
        ));

    // Done
    rc = TRUE;

cleanUp:    
    return rc;
}

//------------------------------------------------------------------------------
