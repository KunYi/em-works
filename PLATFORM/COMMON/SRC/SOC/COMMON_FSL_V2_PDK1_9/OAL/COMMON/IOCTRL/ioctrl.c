//-----------------------------------------------------------------------------
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
// FILE:    ioctrl.c
//
// PURPOSE: This module provide get current kitl information. 
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// INCLUDE FILES
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <nkintr.h>
#include <oal.h>
#pragma warning(pop)

#include "common_macros.h"

BOOL OALIoCtlKitlGetInfo(
    UINT32 dwIoControlCode, VOID *lpInBuf, UINT32 nInBufSize, VOID *lpOutBuf,
    UINT32 nOutBufSize, UINT32* lpBytesReturned)
{
    BOOL rc = FALSE;

    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"+OALIoCtlHalGetHWEntropy(rc = %d)\r\n", rc));

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(dwIoControlCode);
    UNREFERENCED_PARAMETER(lpInBuf);
    UNREFERENCED_PARAMETER(nInBufSize);

    // Check buffer size
    if (lpBytesReturned != NULL)
    {
        *lpBytesReturned = sizeof(void *);
    }

    if (lpOutBuf == NULL || nOutBufSize < sizeof(void *) )
    {
        NKSetLastError(ERROR_INSUFFICIENT_BUFFER);
        OALMSG(OAL_WARN, (L"WARN: OALIoCtlHalGetHWEntropy: Buffer too small\r\n"));
    }
    else
    {
        // Copy the unique part ID that was stored during OEMInit
        //memcpy(lpOutBuf, g_UPID, sizeof(UINT8)*8);
        OAL_KITL_ARGS *pArgs;
        pArgs=OALArgsQuery(OAL_ARGS_QUERY_KITL);
        if(pArgs == 0)
            return FALSE;
       
        if( !(pArgs->flags & OAL_KITL_FLAGS_ENABLED) )
        {
            return FALSE;
        }

        *(DWORD*)lpOutBuf = pArgs->devLoc.LogicalLoc;

       rc = TRUE;
    }

    // Indicate status
    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"-OALIoCtlHalGetHWEntropy(rc = %d)\r\n", rc));
    return rc;
}
