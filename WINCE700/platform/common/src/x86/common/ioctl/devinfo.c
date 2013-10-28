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
// -----------------------------------------------------------------------------
//
//      THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//      ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//      THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//      PARTICULAR PURPOSE.
//  
// -----------------------------------------------------------------------------
#include <windows.h>
#include <x86boot.h>
#include <kitl.h>
#include <oal.h>


extern LPCWSTR g_oalIoCtlPlatformType;
extern UINT8 g_x86uuid[16];

VOID* OALArgsQuery(UINT32 type)
{
    switch(type)
    {
        case OAL_ARGS_QUERY_DEVID:
            return (VOID*)g_pX86Info->szDeviceName;
            break;
        case OAL_ARGS_QUERY_UUID:
            RETAILMSG(1, (TEXT("You are requesting a UUID from a platform which does not guarantee universal uniqueness.\r\n")));
            return (VOID*)(&(g_x86uuid));
            break;
        default:
            return NULL;
    }
}

BOOL x86IoCtlHalGetUUID (
    UINT32 code, VOID *lpInBuf, UINT32 nInBufSize, VOID *lpOutBuf, 
    UINT32 nOutBufSize, UINT32 *lpBytesReturned
) {
    DWORD dwErr = ERROR_INVALID_PARAMETER;
    UNREFERENCED_PARAMETER(nInBufSize);
    UNREFERENCED_PARAMETER(lpInBuf);
    UNREFERENCED_PARAMETER(code);

    if (lpBytesReturned) {
        *lpBytesReturned = sizeof(GUID);
    }
    if (lpOutBuf && (nOutBufSize >= sizeof(GUID))) {
        USHORT *wMac = g_pX86Info->wMac;
        // OEM's with unique ID hardware can return the value here.

        // The CEPC platform does not have any unique ID settings.
        // We'll use the Kernel Ethernet Debug address if non-zero
        if (wMac[0] | wMac[1] | wMac[2]) {
            memset(lpOutBuf, 0, sizeof(GUID));
            memcpy(lpOutBuf, (char *)wMac, sizeof(g_pX86Info->wMac));
            return TRUE;
        }
        dwErr = ERROR_NOT_SUPPORTED;
    }
    NKSetLastError (dwErr);
    return FALSE;

}

