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
// This file is the stub implementation necessary for CRT secure APIs. 
//

#include <windows.h>

void
__cdecl
__crt_unrecoverable_error(
    const wchar_t *pszExpression,
    const wchar_t *pszFunction,
    const wchar_t *pszFile,
    unsigned int nLine,
    uintptr_t pReserved
    )
{
    UNREFERENCED_PARAMETER(pszExpression);
    UNREFERENCED_PARAMETER(pszFunction);
    UNREFERENCED_PARAMETER(pszFile);
    UNREFERENCED_PARAMETER(nLine);
    UNREFERENCED_PARAMETER(pReserved);
    for(;;);
}
