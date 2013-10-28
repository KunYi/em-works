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

//------------------------------------------------------------------------------

STRSAFEAPI 
StringCchCopyW(
    __out_ecount(cchDest) STRSAFE_LPWSTR pszDest,
    __in size_t cchDest,
    __in STRSAFE_LPCWSTR pszSrc
    )
{
    long hr;

    if (cchDest == 0)
        {
        hr = STRSAFE_E_INVALID_PARAMETER;
        }
    else
        {
        while ((cchDest > 0) && (*pszSrc != L'\0'))
            {
            *pszDest++ = *pszSrc++;
            cchDest--;
            }
        if (cchDest == 0)
            {
            pszDest--;
            hr = STRSAFE_E_INSUFFICIENT_BUFFER;
            }
        else
            {
            hr = S_OK;
            }
        *pszDest = L'\0';
        }
    
    return hr;
}

//------------------------------------------------------------------------------

STRSAFEAPI
StringCchCatW(
    __inout_ecount(cchDest) STRSAFE_LPWSTR pszDest,
    __in size_t cchDest,
    __in STRSAFE_LPCWSTR pszSrc
    )
{
    HRESULT hr;

    if (cchDest == 0)
        {
        hr = STRSAFE_E_INVALID_PARAMETER;
        }
    else
        {
        while ((cchDest > 0) && (*pszDest != L'\0'))
            {
            pszDest++;
            cchDest--;
            }
        while ((cchDest > 0) && (*pszSrc != L'\0'))
            {
            *pszDest++ = *pszSrc++;
            cchDest--;
            }
        if (cchDest == 0)
            {
            pszDest--;
            hr = STRSAFE_E_INSUFFICIENT_BUFFER;
            }
        else
            {
            hr = S_OK;
            }
        *pszDest = L'\0';
        }
    
    return hr;
}    

//------------------------------------------------------------------------------

STRSAFEAPI
StringCchCopyNW(
    __out_ecount(cchDest) STRSAFE_LPWSTR pszDest,
    __in size_t cchDest, 
    __in_ecount(cchToCopy) STRSAFE_LPCWSTR pszSrc,
    __in size_t cchToCopy
    )
{
    HRESULT hr;
    
    if (cchDest == 0)
        {
        hr = STRSAFE_E_INVALID_PARAMETER;
        }
    else
        {
        while ((cchDest > 0) && (cchToCopy > 0) && (*pszSrc != L'\0'))
            {
            *pszDest++ = *pszSrc++;
            cchDest--;
            cchToCopy--;
            }
        if (cchDest == 0)
            {
            // we are going to truncate pszDest
            pszDest--;
            hr = STRSAFE_E_INSUFFICIENT_BUFFER;
            }
        else
            {
            hr = S_OK;
            }
        *pszDest= L'\0';
        }
    
    return hr;
}

//------------------------------------------------------------------------------

STRSAFEAPI
StringCchCatNW(
    __inout_ecount(cchDest) STRSAFE_LPWSTR pszDest,
    __in size_t cchDest,
    __in_ecount(cchToAppend) STRSAFE_LPCWSTR pszSrc,
    __in size_t cchToAppend
    )
{
    HRESULT hr;
    
    if (cchDest == 0)
        {
        hr = STRSAFE_E_INVALID_PARAMETER;
        }
    else
        {
        while ((cchDest > 0) && (*pszDest != L'\0'))
            {
            pszDest++;
            cchDest--;
            }
        while ((cchDest > 0) && (cchToAppend > 0) && (*pszSrc != L'\0'))
            {
            *pszDest++ = *pszSrc++;
            cchDest--;
            cchToAppend--;
            }
        if (cchDest == 0)
            {
            // we are going to truncate pszDest
            pszDest--;
            hr = STRSAFE_E_INSUFFICIENT_BUFFER;
            }
        else
            {
            hr = S_OK;
            }
        *pszDest= L'\0';
        }
    
    return hr;
}

//------------------------------------------------------------------------------

STRSAFEAPI 
StringCchLengthW(
    __in STRSAFE_LPCWSTR psz,
    __in size_t cchMax,
    __out_opt size_t *pcchLength
    )
{
    HRESULT hr;

    if (psz == NULL)
        {
        hr = STRSAFE_E_INVALID_PARAMETER;
        }
    else
        {
        size_t cch = cchMax;
        
        while ((cch > 0) && (*psz != L'\0'))
            {
            psz++;
            cch--;
            }
        
        if (cch == 0)
            {
            hr = STRSAFE_E_INVALID_PARAMETER;
            cch = cchMax;
            }
        else
            {
            hr = S_OK;
            }

        if (pcchLength != NULL) *pcchLength = cchMax - cch;
        }

    return hr;
}

//------------------------------------------------------------------------------

STRSAFEAPI 
StringCbCopyW(
    __inout_bcount(cbDest) STRSAFE_LPWSTR pszDest,
    __in size_t cbDest,
    __in STRSAFE_LPCWSTR pszSrc
    )
{
    return StringCchCopyW(pszDest, cbDest >> 1, pszSrc);
}

//------------------------------------------------------------------------------

STRSAFEAPI
StringCbCatW(
    __out_bcount(cbDest) STRSAFE_LPWSTR pszDest,
    __in size_t cbDest,
    __in STRSAFE_LPCWSTR pszSrc
    )
{
    return StringCchCatW(pszDest, cbDest >> 1, pszSrc);
}    

//------------------------------------------------------------------------------

STRSAFEAPI
StringCbCopyNW(
    __out_bcount(cbDest) STRSAFE_LPWSTR pszDest,
    __in size_t cbDest, 
    __in_bcount(cbToCopy) STRSAFE_LPCWSTR pszSrc,
    __in size_t cbToCopy
    )
{
    return StringCchCopyNW(pszDest, cbDest >> 1, pszSrc, cbToCopy >> 1);
}

//------------------------------------------------------------------------------

STRSAFEAPI
StringCbCatNW(
    __inout_bcount(cbDest) STRSAFE_LPWSTR pszDest,
    __in size_t cbDest,
    __in_bcount(cbToAppend) STRSAFE_LPCWSTR pszSrc,
    __in size_t cbToAppend
    )
{
    return StringCchCatNW(pszDest, cbDest >> 1, pszSrc, cbToAppend >> 1);
}

//------------------------------------------------------------------------------

STRSAFEAPI 
StringCbLengthW(
    __in STRSAFE_LPCWSTR psz,
    __in size_t cbMax,
    __out_opt size_t *pcbLength
    )
{
    HRESULT hr = StringCchLengthW(psz, cbMax >> 1, pcbLength);
    if (SUCCEEDED(hr) && (pcbLength != NULL)) *pcbLength <<= 1;
    return hr;
}

//------------------------------------------------------------------------------
