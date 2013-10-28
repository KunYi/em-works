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
//  File:  rndismac.c
//
//  This file This file implements the IOCTL_HAL_GET_RNDIS_MACADDR handler.
//
#include <windows.h>
#include <oal.h>
#include <oalex.h>
#include <omap35xx.h>

//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalGetRNdisMacAddr
//
//  Implements the IOCTL_HAL_GET_RNDIS_MACADDR handler.
//
BOOL
OALIoCtlHalGetRNdisMacAddr(
    UINT32 code, 
    VOID *pInpBuffer, 
    UINT32 inpSize, 
    VOID *pOutBuffer,
    UINT32 outSize, 
    UINT32 *pOutSize
    )
{
    BOOL    rc = FALSE;
    UINT32  size = 6*sizeof(UINT8);
    UINT8   *pMAC = NULL;


    // Check buffer size
    if (pOutSize != NULL) *pOutSize = size;

    if (pOutBuffer == NULL || outSize < size)
        {
        OALMSG(OAL_WARN, (L"WARN: OALIoCtlHalGetRNdisMacAddr: "
            L"Buffer too small\r\n"
            ));
        NKSetLastError(ERROR_INSUFFICIENT_BUFFER);
        goto cleanUp;
    }

    // Get a MAC address for RNDIS
    pMAC = OALArgsQuery(OAL_ARGS_QUERY_RNDISMAC);
    if( pMAC )
    {
        // Copy pattern to output buffer
        memcpy(pOutBuffer, pMAC, size);

        // We are done
        rc = TRUE;
    }

cleanUp:
    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"-OALIoCtlHalGetRNdisMacAddr(rc = %d)\r\n", rc));

    return rc;
}

//------------------------------------------------------------------------------

