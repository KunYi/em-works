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
//  File:  SecureKeys.c
//
//  This file implements the IOCTL_HAL_GETREGSECUREKEYS handler.
//
#include <windows.h>
#include <oal.h>

//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalGetRegSecureKeys
//
//  IOCTL_HAL_GETREGSECUREKEYS is depreciated, provided here for backward
//  compatibility.
//
BOOL OALIoCtlHalGetRegSecureKeys( 
    UINT32 dwIoControlCode, VOID *lpInBuf, UINT32 nInBufSize, VOID *lpOutBuf, 
    UINT32 nOutBufSize, UINT32* lpBytesReturned)
{
    UNREFERENCED_PARAMETER(dwIoControlCode);    
    UNREFERENCED_PARAMETER(lpInBuf);    
    UNREFERENCED_PARAMETER(nInBufSize);    
    UNREFERENCED_PARAMETER(lpOutBuf);    
    UNREFERENCED_PARAMETER(nOutBufSize);    
    UNREFERENCED_PARAMETER(lpBytesReturned);    

    OALMSG(1, (L"WARNING: Deprecated  IOCTL \"IOCTL_HAL_GETREGSECUREKEYS\""));
    OALMSG(1, (L"called. Please remove IOCTL and function calls to "));
    OALMSG(1, (L"OALIoCtlHalGetRegSecureKeys from your code base\r\n"));

    NKSetLastError(ERROR_NOT_SUPPORTED);

    return (FALSE);
}

