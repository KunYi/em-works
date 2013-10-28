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

BOOL x86IoCtlQueryDispSettings(
                               UINT32 code, 
                               __in_bcount(nInBufSize) void *lpInBuf, 
                               UINT32 nInBufSize, 
                               __out_bcount(nOutBufSize) void *lpOutBuf, 
                               UINT32 nOutBufSize, 
                               __out UINT32 *lpBytesReturned
                               ) 
{
    UNREFERENCED_PARAMETER(nInBufSize);
    UNREFERENCED_PARAMETER(lpInBuf);
    UNREFERENCED_PARAMETER(code);

    if (lpBytesReturned) 
    {
        *lpBytesReturned = sizeof (DWORD) * 3;
    }

    if (!lpOutBuf && nOutBufSize > 0) 
    {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    if (!lpOutBuf || nOutBufSize < sizeof(DWORD)*3) 
    {
        NKSetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

    // Check the boot arg structure for the default display settings.
    __try 
    {
        ((PDWORD)lpOutBuf)[0] = g_pX86Info->cxDisplayScreen;
        ((PDWORD)lpOutBuf)[1] = g_pX86Info->cyDisplayScreen;
        ((PDWORD)lpOutBuf)[2] = g_pX86Info->bppScreen;
    } 
    __except (GetExceptionCode() == STATUS_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
    {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        if (lpBytesReturned) 
        {
            *lpBytesReturned = 0;
        }
        return FALSE;
    }
     
    return TRUE;
}
