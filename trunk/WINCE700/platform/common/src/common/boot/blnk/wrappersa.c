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
//
//  Global:  g_szBuffer
//
//  Global static variable used as buffer for debug output functions. As long
//  as we suppose that code is run in single thread enviroment no protection
//  is needed.
//
static 
CHAR 
g_szBuffer[384];

//------------------------------------------------------------------------------

int
NKvsprintfA(
    LPSTR szBuffer,
    LPCSTR szFormat,
    va_list pArgList,
    int maxChars
    );

void 
OEMWriteDebugByte(
    BYTE ch
    );

//------------------------------------------------------------------------------

void
KITLOutputDebugString(
    const char *fmt, ...
    )
{
    va_list pArgList;
    char *p = g_szBuffer;

    va_start(pArgList, fmt);
    NKvsprintfA(g_szBuffer, fmt, pArgList, sizeof(g_szBuffer));
    va_end(pArgList);
    
    while (*p != '\0') OEMWriteDebugByte(*p++);
}

//------------------------------------------------------------------------------

void
NKsprintfA(
    LPSTR szBuffer,
    size_t bufferSize,
    LPCSTR szFormat,
    ...
    )
{
    va_list pArgList;

    va_start(pArgList, szFormat);
    NKvsprintfA(szBuffer, szFormat, pArgList, bufferSize);
    va_end(pArgList);
}

//------------------------------------------------------------------------------

