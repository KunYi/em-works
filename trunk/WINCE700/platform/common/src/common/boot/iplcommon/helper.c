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

// Stub required for NAND FMD
//
LPVOID MapCallerPtr(LPVOID ptr, DWORD dwLen)
{
    UNREFERENCED_PARAMETER(dwLen);
    return ptr;
}

// Stubs needed for fmdhook
//
HANDLE CreateMutex( 
  LPSECURITY_ATTRIBUTES lpMutexAttributes, 
  BOOL bInitialOwner, 
  LPCTSTR lpName 
)
{
    UNREFERENCED_PARAMETER(lpMutexAttributes);
    UNREFERENCED_PARAMETER(bInitialOwner);
    UNREFERENCED_PARAMETER(lpName);
    return (HANDLE)1;
}

DWORD WaitForSingleObject( 
  HANDLE hHandle, 
  DWORD dwMilliseconds 
)
{
    UNREFERENCED_PARAMETER(hHandle);
    UNREFERENCED_PARAMETER(dwMilliseconds);
    return 0;
}

BOOL ReleaseMutex( 
  HANDLE hMutex 
)
{
    UNREFERENCED_PARAMETER(hMutex);
    return 0;
}
