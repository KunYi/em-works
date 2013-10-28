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
#include <flashOs.h>
#include <bootMemory.h>
#include <bootLog.h>
#include <bootString.h>

//------------------------------------------------------------------------------

VOID
Flash_LogMessage(
    const WCHAR * format,
    ...
    )
{
    UNREFERENCED_PARAMETER(format);
#if 0
    va_list pArgList;
    
    va_start(pArgList, format);
    BootLogV(format, pArgList);
    OEMBootLogWrite(L"\r\n");
#endif    
}

//------------------------------------------------------------------------------

VOID
Flash_LogData(
    DWORD id, 
    VOID *pData, 
    DWORD length
    )
{
    UNREFERENCED_PARAMETER(id);
    UNREFERENCED_PARAMETER(pData);
    UNREFERENCED_PARAMETER(length);
}

//------------------------------------------------------------------------------

LPVOID
Flash_VirtualAlloc(
    LPVOID pAddress,
    DWORD size,
    DWORD allocationTypeFlags,
    DWORD protectFlags
    )
{
    VOID *pMemory;
    
    UNREFERENCED_PARAMETER(protectFlags);
    
    if ((allocationTypeFlags & MEM_RESERVE) != 0)
        {
        pMemory = BootAlloc(size);
        }
    else
        {
        pMemory = pAddress;
        }

    return pMemory;
}

//------------------------------------------------------------------------------

BOOL
Flash_VirtualFree(
    LPVOID pAddress,
    DWORD size,
    DWORD freeType
    )
{
    UNREFERENCED_PARAMETER(size);
    if ((freeType & MEM_RELEASE) != 0) BootFree(pAddress);
    return TRUE;
}

//------------------------------------------------------------------------------

LRESULT
Flash_GetRegistryValue(
    DWORD Context,
    PCWSTR ValueName, 
    PDWORD pValue
    )
{
    UNREFERENCED_PARAMETER(Context);
    
    if(BootStringEqual((wstring_t)ValueName, L"UpdateReadOnly"))
    {
        *pValue = 1;
        return NO_ERROR;
    }

    return ERROR_NOT_FOUND;
}

//------------------------------------------------------------------------------

LRESULT
Flash_GetRegistryString(
    DWORD Context,
    PCWSTR ValueName, 
    PWSTR ValueString,
    DWORD StringLength
    )
{
    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(ValueName);
    UNREFERENCED_PARAMETER(ValueString);
    UNREFERENCED_PARAMETER(StringLength);
    return ERROR_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------

BOOL 
CeSafeCopyMemory(
    LPVOID pDst, LPCVOID pSrc, DWORD cbSize
    )
{
    memcpy(pDst, pSrc, cbSize);
    return TRUE;
}

//------------------------------------------------------------------------------
// This function is required and used by compaction functionality in 
// flashmddcore.lib.  Compaction rarely runs in the bootloader, so it is acceptable, 
// and not an error, to always return 0 as opposed to truly random values.
DWORD 
Random(
    )
{
    return 0;
}    

//------------------------------------------------------------------------------

HRESULT
StringCchCopyW(
    wstring_t dest,
    size_t cchDest,
    wcstring_t src
    )
{
    HRESULT hr = S_OK;

    if ((cchDest == 0)||(cchDest > STRSAFE_MAX_CCH)) 
        {
        hr = STRSAFE_E_INVALID_PARAMETER;
        goto cleanUp;
        }

    while (cchDest && (*src != L'\0'))
    {
        *dest++ = *src++;
        cchDest--;
    }

    if (cchDest == 0)
    {
        // we are going to truncate dest
        dest--;
        hr = STRSAFE_E_INSUFFICIENT_BUFFER;
    }

    *dest= L'\0';

cleanUp:
    return hr;
}

//------------------------------------------------------------------------------

