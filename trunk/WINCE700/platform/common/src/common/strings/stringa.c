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
StringCchCopyA(
    __out_ecount(cchDest) STRSAFE_LPSTR pszDest,
    __in size_t cchDest,
    __in STRSAFE_LPCSTR pszSrc
    )
{
    long hr;

    if (cchDest == 0)
        {
        hr = STRSAFE_E_INVALID_PARAMETER;
        }
    else
        {
        while ((cchDest > 0) && (*pszSrc != '\0'))
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
        *pszDest = '\0';
        }
    
    return hr;
}

//------------------------------------------------------------------------------

STRSAFEAPI
StringCchCatA(
    __inout_ecount(cchDest) STRSAFE_LPSTR pszDest,
    __in size_t cchDest,
    __in STRSAFE_LPCSTR pszSrc
    )
{
    HRESULT hr;

    if (cchDest == 0)
        {
        hr = STRSAFE_E_INVALID_PARAMETER;
        }
    else
        {
        while ((cchDest > 0) && (*pszDest != '\0'))
            {
            pszDest++;
            cchDest--;
            }
        while ((cchDest > 0) && (*pszSrc != '\0'))
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
        *pszDest = '\0';
        }
    
    return hr;
}    

//------------------------------------------------------------------------------

STRSAFEAPI
StringCchCopyNA(
    __out_ecount(cchDest) STRSAFE_LPSTR pszDest,
    __in size_t cchDest,
    __in_ecount(cchToCopy) STRSAFE_LPCSTR pszSrc,
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
        while ((cchDest > 0) && (cchToCopy > 0) && (*pszSrc != '\0'))
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
        *pszDest= '\0';
        }
    
    return hr;
}

//------------------------------------------------------------------------------

STRSAFEAPI
StringCchCatNA(
    __inout_ecount(cchDest) STRSAFE_LPSTR pszDest,
    __in size_t cchDest,
    __in_ecount(cchToAppend) STRSAFE_LPCSTR pszSrc,
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
        while ((cchDest > 0) && (*pszDest != '\0'))
            {
            pszDest++;
            cchDest--;
            }
        while ((cchDest > 0) && (cchToAppend > 0) && (*pszSrc != '\0'))
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
        *pszDest= '\0';
        }
    
    return hr;
}

//------------------------------------------------------------------------------

STRSAFEAPI 
StringCchLengthA(
    __in STRSAFE_LPCSTR psz,
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
        
        while ((cch > 0) && (*psz != '\0'))
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
StringCbCopyA(
    __out_bcount(cbDest) STRSAFE_LPSTR pszDest,
    __in size_t cbDest,
    __in STRSAFE_LPCSTR pszSrc
    )
{
    return StringCchCopyA(pszDest, cbDest, pszSrc);
}

//------------------------------------------------------------------------------

STRSAFEAPI
StringCbCatA(
    __inout_bcount(cbDest) STRSAFE_LPSTR pszDest,
    __in size_t cbDest,
    __in STRSAFE_LPCSTR pszSrc
    )
{
    return StringCchCatA(pszDest, cbDest, pszSrc);
}    

//------------------------------------------------------------------------------

STRSAFEAPI
StringCbCopyNA(
    __out_bcount(cbDest) STRSAFE_LPSTR pszDest,
    __in size_t cbDest, 
    __in_bcount(cbToCopy) STRSAFE_LPCSTR pszSrc,
    __in size_t cbToCopy
    )
{
    return StringCchCopyNA(pszDest, cbDest, pszSrc, cbToCopy);
}

//------------------------------------------------------------------------------

STRSAFEAPI
StringCbCatNA(
    __inout_bcount(cbDest) STRSAFE_LPSTR pszDest,
    __in size_t cbDest,
    __in_bcount(cbToAppend) STRSAFE_LPCSTR pszSrc,
    __in size_t cbToAppend
    )
{
    return StringCchCatNA(pszDest, cbDest, pszSrc, cbToAppend);
}

//------------------------------------------------------------------------------

STRSAFEAPI 
StringCbLengthA(
    __in STRSAFE_LPCSTR psz,
    __in size_t cbMax,
    __out_opt size_t *pcbLength
    )
{
    return StringCchLengthA(psz, cbMax, pcbLength);
}

//------------------------------------------------------------------------------

