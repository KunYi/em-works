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
//  File:  format.c
//
//  This file implements the IOCTL_HAL_QUERY_FORMAT_PARTITION handler.
//
#include <windows.h>
#include <oal.h>
#include <oalex.h>

//------------------------------------------------------------------------------
//
//  Function: OALIoCtlHalQueryFormatPartition
//
BOOL
OALIoCtlHalQueryFormatPartition(
    UINT32 code, 
    VOID *pInpBuffer, 
    UINT32 inpSize, 
    VOID *pOutBuffer, 
    UINT32 outSize,
    UINT32 *pOutSize
    )
{
    BOOL rc = FALSE;
    BOOL *pColdBoot;

    if (pOutSize != NULL) *pOutSize = sizeof(BOOL);

    // Check buffer size
    if (pOutBuffer == NULL || outSize < sizeof(BOOL)) 
        {
        NKSetLastError(ERROR_INSUFFICIENT_BUFFER);
        OALMSG(OAL_WARN, (L"WARN: OALIoCtlHalQueryFormatPartition: "
            L"Buffer too small\r\n"
            ));
        goto cleanUp;
        }

    *(BOOL*)pOutBuffer = FALSE;
    pColdBoot = OALArgsQuery(OAL_ARGS_QUERY_COLDBOOT);
    if ((pColdBoot != NULL) && *pColdBoot)
        {
        RETAILMSG(TRUE, (L"*** Cold Boot - Format TFAT ***\r\n"));
        *(BOOL*)pOutBuffer = TRUE;

        // Clear the flag for next boot time(warm-boot) unless it is set again.
        *pColdBoot = FALSE;
        }

    rc = TRUE;
    
cleanUp:
    return rc;
}

//------------------------------------------------------------------------------
