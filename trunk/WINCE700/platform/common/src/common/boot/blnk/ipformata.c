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

//------------------------------------------------------------------------------

void
NKsprintfA(
    LPSTR szBuffer,
    size_t bufferSize,
    LPCSTR szFormat,
    ...
    );

//------------------------------------------------------------------------------
//
// This routine will take a binary IP address as represent here and
// return a dotted decimal version of it

char*
inet_ntoa(
    DWORD dwIP
    )
{

    static char szDottedD[16];

    NKsprintfA(
        szDottedD, sizeof(szDottedD), "%u.%u.%u.%u",
        (BYTE)dwIP, (BYTE)(dwIP >> 8), (BYTE)(dwIP >> 16), (BYTE)(dwIP >> 24) 
        );

    return szDottedD;

}

//------------------------------------------------------------------------------
//
// This routine will take a dotted decimal IP address as represent here and
// return a binary version of it
//
DWORD
inet_addr(
    char *pszDottedD
    )
{

    DWORD dwIP = 0;
    DWORD cBytes;
    char *pszLastNum;
    int atoi (const char *s);
    
    // Replace the dots with NULL terminators
    pszLastNum = pszDottedD;
    for( cBytes = 0; cBytes < 4; cBytes++ ) {
        while(*pszDottedD != '.' && *pszDottedD != '\0')
            pszDottedD++;
        if (pszDottedD == '\0' && cBytes != 3)
            return 0;
        *pszDottedD = '\0';
        dwIP |= (atoi(pszLastNum) & 0xFF) << (8*cBytes);
        pszLastNum = ++pszDottedD;
    }

    return dwIP;

}

//------------------------------------------------------------------------------

