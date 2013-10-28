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
#include <windows.h>
#include <strsafe.h>
#include <nkexport.h>

//------------------------------------------------------------------------------

STRSAFEAPI
StringCchVPrintfW(
    __out_ecount(cchDest) STRSAFE_LPWSTR pszDest,
    __in size_t cchDest,
    __in __format_string STRSAFE_LPCWSTR pszFormat,
    __in va_list argList
    )
{
    HRESULT hr;
    
    if ((size_t)NKwvsprintfW(pszDest, pszFormat, argList, cchDest) < cchDest)
        {
        hr = S_OK;
        }
    else
        {
        hr = STRSAFE_E_INSUFFICIENT_BUFFER;
        }
    return hr;        
}

//------------------------------------------------------------------------------

