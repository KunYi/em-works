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
#include <wtypes.h>
#include <oal.h>
#include <resmgr.h>

// The maximum 'range' registry key size (this should include the NIC which will be added in this code)
static const DWORD MAX_RANGE_COUNT = 256;

///////////////////////////////////////////////////////////////////////////////
// A simple number pair
class RangeToken
{
    DWORD m_dwFirst, m_dwSecond;
public:
    ///////////////////////////////////////////////////////////////////////////
    // parses a token in the form "0x100-0x200" and stops after the second number (or NULL)
    RangeToken(
        __in_ecount(cchLen) const TCHAR* const pszRangeStart, 
        DWORD cchLen
        )
    {
        const TCHAR* pszRange = pszRangeStart;

        m_dwFirst = wcstoul(pszRange, const_cast<TCHAR**>(&pszRange), 0);

        // skip over whitespace, usually there is none
        DWORD cchCount = 0;
        while ((*pszRange) && (*pszRange != L'-') && (cchCount++ < cchLen))
        {
            ++pszRange;
        }

        // buffer overflow, pRange is NULL terminated internally, so should never happen
        ASSERT(cchCount < cchLen);

        if (*pszRange)
        {
            ++pszRange;
        }

        m_dwSecond = wcstoul(pszRange, const_cast<TCHAR**>(&pszRange), 0);

        // buffer overflow, pRange is NULL terminated internally, so should never happen
        ASSERT(pszRange <= pszRangeStart + cchLen);
    }

    ///////////////////////////////////////////////////////////////////////////
    // constructor
    RangeToken(
        const DWORD dwFirst, 
        const DWORD dwSecond
        ) : 
        m_dwFirst(dwFirst), 
        m_dwSecond(dwSecond) 
    { 
    };

    ///////////////////////////////////////////////////////////////////////////
    // Default constructor
    RangeToken() : 
        m_dwFirst((DWORD)-1), 
        m_dwSecond((DWORD)-1) 
    { 
    };

    ///////////////////////////////////////////////////////////////////////////
    inline DWORD GetFirst() const 
    { 
        return m_dwFirst; 
    }
    ///////////////////////////////////////////////////////////////////////////
    inline DWORD GetSecond() const 
    { 
        return m_dwSecond; 
    }
};

///////////////////////////////////////////////////////////////////////////////
// Parses the 'Range' registry key into individual ranges.
// The ranges key is expected to look like "0-0x100, 0x200-0x300"
class RangesParser
{
    TCHAR  m_szRanges[MAX_RANGE_COUNT];
    TCHAR* m_pszCurTokenEnd;
public:
    ///////////////////////////////////////////////////////////////////////////
    // Initalises based on the Registry
    BOOL Init()
    {
        HKEY hKey;
        DWORD cbSize = sizeof m_szRanges;

        if (NKRegOpenKeyEx(
            HKEY_LOCAL_MACHINE, 
            L"Drivers\\Resources\\IO", 
            0, 
            0, 
            &hKey) 
            != ERROR_SUCCESS)
        {
            return FALSE;
        }

        // the backup key should always be unmodified, if it doesn't exist this is the first boot
        DWORD RetVal = NKRegQueryValueEx(
            hKey, 
            TEXT("Ranges-backup"), 
            NULL, 
            NULL, 
            (BYTE*)(&m_szRanges), 
            &cbSize);

        if (RetVal == ERROR_FILE_NOT_FOUND)
        {
            RetVal = NKRegQueryValueEx(
                hKey, 
                TEXT("Ranges"), 
                NULL, 
                NULL, 
                (BYTE*)(&m_szRanges), 
                &cbSize);

            if (RetVal == ERROR_SUCCESS)
            {
                NKRegSetValueEx(hKey, 
                    TEXT("Ranges-backup"), 
                    0, 
                    REG_SZ, 
                    (BYTE*)(&m_szRanges), 
                    cbSize);
            }
        }

        if (RetVal != ERROR_SUCCESS)
        {
            // MAX_RANGE_LEN is too small
            ASSERT(RetVal != ERROR_MORE_DATA);

            return FALSE;
        }

        // ensure it is NULL terminated
        m_szRanges[_countof(m_szRanges) - 1] = 0;

        return TRUE;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Default constructor
    RangesParser() : 
        m_pszCurTokenEnd(m_szRanges) 
    {
    };

    ///////////////////////////////////////////////////////////////////////////
    // Gets the next token from the range
    BOOL GetNextToken(__out RangeToken&Token)
    {
        const TCHAR* const pszCurTokenStart = m_pszCurTokenEnd;

        if (!*pszCurTokenStart) 
        {
            return FALSE;
        }

        // find the start of the next token
        while ((*m_pszCurTokenEnd) 
            && (*m_pszCurTokenEnd != L',')
            && (m_pszCurTokenEnd < m_szRanges + _countof(m_szRanges) - 1 ))
        {
            ++m_pszCurTokenEnd;
        }

        // buffer overflow: pCurTokenEnd is NULL terminated internally, so should never happen
        ASSERT(m_pszCurTokenEnd < m_szRanges + _countof(m_szRanges) - 1);

        // construct a RangeToken from the two numbers
        Token = RangeToken(pszCurTokenStart, m_pszCurTokenEnd - pszCurTokenStart);

        // increment past the ',' 
        if (*m_pszCurTokenEnd)
        {
            ++m_pszCurTokenEnd;
        }

        return TRUE;
    }
};

///////////////////////////////////////////////////////////////////////////////
// This class takes Ranges and formats them as text into a buffer. Once finished these are written to the registry
class RangesOutput
{
    TCHAR  m_szRanges[MAX_RANGE_COUNT];
    TCHAR* m_pszCurTokenEnd;
    BOOL   m_fOutputIsFull;
public:
    ///////////////////////////////////////////////////////////////////////////
    // default constructor
    RangesOutput() : 
        m_pszCurTokenEnd(m_szRanges), 
        m_fOutputIsFull(FALSE) 
    { 
        m_szRanges[0] = 0; 
    };

    ///////////////////////////////////////////////////////////////////////////
    // adds a RangeToken to the buffer
    BOOL Writeout(const RangeToken&Token)
    {
        // avoid overruns
        m_szRanges[_countof(m_szRanges)-1] = 0;

        m_pszCurTokenEnd = m_szRanges;
        while (*m_pszCurTokenEnd)
        {
            ++m_pszCurTokenEnd;
        }

        if ((m_pszCurTokenEnd != m_szRanges )
            && (m_pszCurTokenEnd < &m_szRanges[_countof(m_szRanges)]))
        {
            // if this isn't the first token, then start with a ','
            *m_pszCurTokenEnd++ = L',';
            *m_pszCurTokenEnd = 0;
        }
        
        // sprintf the range
        if (StringCchPrintf(
            m_pszCurTokenEnd, 
            _countof(m_szRanges) - (m_pszCurTokenEnd - m_szRanges), 
            L"0x%x-0x%x", 
            Token.GetFirst(), 
            Token.GetSecond()
            ) != S_OK)
        {
            m_fOutputIsFull = TRUE;
        }

        return FALSE == m_fOutputIsFull;
    }

    ///////////////////////////////////////////////////////////////////////////
    // returns true if the buffer is full
    BOOL IsFull() const 
    {   
        return m_fOutputIsFull; 
    }

    ///////////////////////////////////////////////////////////////////////////
    // Writes the buffer to the registry
    BOOL Finialize() const
    {
        ASSERT(m_fOutputIsFull == FALSE);

        HKEY   hKey;
        DWORD  cbSize;
        size_t cchSize;

        StringCchLength(m_szRanges, _countof(m_szRanges), &cchSize);
        cbSize = (1 + cchSize) * sizeof m_szRanges[0];

        // write the modifed buffer back to the registry
        if (NKRegOpenKeyEx(
            HKEY_LOCAL_MACHINE, 
            L"Drivers\\Resources\\IO", 
            0, 
            0, 
            &hKey) != ERROR_SUCCESS)
        {
            return FALSE;
        }

        if (NKRegSetValueEx(
            hKey, 
            TEXT("Ranges"), 
            0, 
            REG_SZ, 
            (BYTE*)(&m_szRanges), 
            cbSize) != ERROR_SUCCESS)
        {
            return FALSE;
        }

        return TRUE;
    }
};

///////////////////////////////////////////////////////////////////////////////
// This function allows devices to request exclusive access to shareable resources.
//
// dwResId - must be RESMGR_IOSPACE
// dwId    - the start of the IO Range to reserve
// dwLen   - the length of the IO Range to reserve
//
// returns: TRUE if this range is not already reserved, and was reserved
//          FALSE on failure, or if the range was previously reserved
//
// NOTE: this will "fail" on hive registry, as the previous boot will reserve this address
//
extern "C"
BOOL 
ResourceRequest(
                DWORD dwResId, 
                DWORD dwId, 
                DWORD dwLen
                )
{
    if ((dwResId != RESMGR_IOSPACE) || (!dwLen) || (dwId + dwLen < dwId))
    {
        // This supports only IO Space 
        // dwLen must be non 0
        // dwId + dwLen must not roll over
        ASSERT(FALSE);
        return FALSE;
    }

    const DWORD dwReqStart = dwId;
    const DWORD dwReqEnd = dwId + dwLen;

    RangesParser Parser;

    if (!Parser.Init())
    {
        return FALSE;
    }

    RangesOutput Output;
    RangeToken Token;

    BOOL fModifiedRange = FALSE;

    while (Parser.GetNextToken(Token))
    {
        const DWORD dwFreeStart = Token.GetFirst();
        const DWORD dwFreeEnd = Token.GetSecond();

        // if the Reqested token is in the middle of the current free token..
        if ((dwFreeStart < dwReqStart) && (dwFreeEnd > dwReqEnd))
        {
            // ..reserve the space
            Output.Writeout(RangeToken(dwFreeStart, dwReqStart));
            Output.Writeout(RangeToken(dwReqEnd, dwFreeEnd));
            fModifiedRange = TRUE;
        }
        // if the Requested token crosses a free token boundry, or is already reserved..
        else if ((dwFreeStart > dwReqStart) && (dwFreeEnd > dwReqEnd))
        {
            // this might happen on HIVE registry, since a previous boot allocated the resource
            RETAILMSG(
                OAL_WARN, 
                (L"ResourceRequest: The IO Space was already reserved: 0x%x-0x%x", 
                dwReqStart, 
                dwReqEnd)
                );
            Output.Writeout(Token);
        }
        else
        {
            // don't modify the current free token
            Output.Writeout(Token);
        }
    }

    if (fModifiedRange && (Output.IsFull() == FALSE))
    {
        RETAILMSG(OAL_INFO, 
            (L"ResourceRequest: Reserving IO Space for KITL: 0x%x-0x%x", 
            dwReqStart, 
            dwReqEnd)
            );
        return Output.Finialize();
    }
    else
        return FALSE;
}


///////////////////////////////////////////////////////////////////////////////
// This routine is a safer version of the C built-in function 'sprintf'.
//
STRSAFEAPI
StringCchPrintfW(
    __out_ecount(cchDest) STRSAFE_LPWSTR pszDest,
    __in size_t cchDest,
    __in __format_string STRSAFE_LPCWSTR pszFormat,
    ...)
{
    HRESULT hr;

    if (cchDest > STRSAFE_MAX_CCH)
    {
        hr = STRSAFE_E_INVALID_PARAMETER;
    }
    else
    {
        va_list argList;

        va_start(argList, pszFormat);

        // pass on the args to NKwvsprintfW() to do the actual work
        if ((size_t)NKwvsprintfW(pszDest, pszFormat, argList, cchDest) < cchDest)
            hr = S_OK;
        else
            hr = STRSAFE_E_INSUFFICIENT_BUFFER;

        va_end(argList);
    }

    return hr;
}

///////////////////////////////////////////////////////////////////////////////
// wcstoul Stubs

// the following functions are declared as __dllimport, but we implement locally
#pragma warning (disable:4273)

///////////////////////////////////////////////////////////////////////////////
// Retrieves character-type information for the characters in the specified source string. 
// For each character in the string, the function sets one or more bits in the 
// corresponding 16-bit element of the output array. Each bit identifies a given character 
// type, such as whether the character is a letter, a digit, or neither. 
//
// used by iswspace/iswctype
//
// only supports limited chars expected in the 'Range' key, ASSERTS() for all others
//
BOOL
WINAPI
GetStringTypeExW(
    LCID     Locale,
    DWORD    dwInfoType,
    __in_ecount(cchSrc) LPCWSTR  lpSrcStr,
    int      cchSrc,
    __out LPWORD  lpCharType) 
{ 
    UNREFERENCED_PARAMETER(Locale);
    UNREFERENCED_PARAMETER(dwInfoType);
    UNREFERENCED_PARAMETER(cchSrc);

    if (lpSrcStr[0] == L' ')
        *lpCharType = _SPACE;

    else if (lpSrcStr[0] >= L'0' && lpSrcStr[0] <= L'9')
        *lpCharType = _DIGIT;

    else if (lpSrcStr[0] == L'-' || lpSrcStr[0] == L',')
        *lpCharType = _PUNCT;

    else if (lpSrcStr[0] >= L'A' && lpSrcStr[0] <= L'F')
        *lpCharType = _UPPER | _HEX | _ALPHA;

    else if (lpSrcStr[0] >= L'a' && lpSrcStr[0] <= L'f')
        *lpCharType = _LOWER | _HEX | _ALPHA;

    else if (lpSrcStr[0] == 0)
        *lpCharType = _CONTROL;

    // This is missing most of the characters not expected in the Range. see _wctype[]
    else 
    {
        ASSERT(FALSE);
        return FALSE;
    }

    return TRUE; 
} 
///////////////////////////////////////////////////////////////////////////////
// This function maps a character string to a wide-character (Unicode) string. 
//
// used by isdigit/_isctype
//
int
WINAPI
MultiByteToWideChar(
    UINT     CodePage,
    DWORD    dwFlags,
    __in LPCSTR   lpMultiByteStr,
    int      cbMultiByte,
    __out_ecount(cchWideChar) LPWSTR  lpWideCharStr,
    int      cchWideChar) 
{
    UNREFERENCED_PARAMETER(CodePage);
    UNREFERENCED_PARAMETER(dwFlags);

    // a) we only support single char conversions
    // b) we don't support DB, so assert it is 7bit clean
    if (cbMultiByte == cchWideChar == 1 &&
       (lpMultiByteStr[0] & 0x80) == 0)
    {
        lpWideCharStr[0] = lpMultiByteStr[0];
        return 1; 
    }

    ASSERT(FALSE);
    return 0;
} 

///////////////////////////////////////////////////////////////////////////////
// Determines if a specified character is potentially a lead byte. 
// A lead byte is the first byte of a two-byte character in a double-byte character set (DBCS). 
//
// used by _isctype
//
BOOL
WINAPI
IsDBCSLeadByte(BYTE TestChar) 
{
    UNREFERENCED_PARAMETER(TestChar);

    // we don't support DB, so assert it is 7bit clean
    ASSERT((TestChar & 0x80) == 0);

    return FALSE; 
} 

///////////////////////////////////////////////////////////////////////////////
// Converts a character string or a single character to lower/upper case. 
// If the operand is a character string, the function converts the characters in place. 
// 
// unused, unimplemented stubs which the linker wants
//
LPWSTR WINAPI CharLowerW(__inout LPWSTR lpsz) {ASSERT(FALSE); return lpsz; } 
LPWSTR WINAPI CharUpperW(__inout LPWSTR lpsz) {ASSERT(FALSE); return lpsz; } 
