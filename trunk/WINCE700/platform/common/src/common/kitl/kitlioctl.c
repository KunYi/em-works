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
//  File:  kitlioctl.c
//
#include <windows.h>
#include <oal.h>

BOOL OALIoCtlVBridge(
    UINT32 code, VOID *pInBuffer, UINT32 inSize, VOID *pOutBuffer, 
    UINT32 outSize, UINT32 *pOutSize);

void OALKitlInitRegistry(void);

//------------------------------------------------------------------------------
//
//  Function:  OEMKitlIoctl
//
//  This function handles KITL IOCTL codes.
//
//
BOOL OEMKitlIoctl (DWORD code, VOID * pInBuffer, DWORD inSize, VOID * pOutBuffer, DWORD outSize, DWORD * pOutSize)
{
    BOOL fRet = FALSE;
    switch (code) {
    case IOCTL_HAL_INITREGISTRY:
        OALKitlInitRegistry();
        NKSetLastError (ERROR_NOT_SUPPORTED);
        // return FALSE, and set last error to Not-supported so the IOCTL also gets routed to OEMIoControl
        break;
    default:
        fRet = OALIoCtlVBridge (code, pInBuffer, inSize, pOutBuffer, outSize, (UINT32 *)pOutSize);
    }

    return fRet;
}
