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
#include <ceddk.h>
#include <hal.h>
#include <x86boot.h>
#include <PCIReg.h>
#include <oal.h>
#include <bootarg.h>

enum { DRIVER_REGPATH_OVERRIDE_LENGTH = 128 };

//------------------------------------------------------------------------------
//
//  Function:  SaveVGAArgs
//
static BOOL SaveVGAArgs()
{
    const BOOT_ARGS** const ppArgs = (BOOT_ARGS**)(BOOT_ARG_PTR_LOCATION);
    const BOOT_ARGS* const pArgs = (const BOOT_ARGS*const )(*(const DWORD*const)ppArgs | 0x80000000);

    BOOL ArgsSet = FALSE;

    if (pArgs != NULL)
    {
        __try
        {
            if (BOOTARG_SIG == pArgs->dwSig)
            {
                HKEY key;
                DWORD Disposition;

                if (NKRegCreateKeyEx(HKEY_LOCAL_MACHINE, TEXT("System\\GDI\\Drivers\\"), 0, NULL, 0, 0, NULL, &key, &Disposition) == ERROR_SUCCESS)
                {
                    DWORD Value = pArgs->vesaMode;
                    BYTE* pValue = (BYTE*)&Value;

                    if (NKRegSetValueEx(key, L"vesaMode", 0, REG_DWORD, pValue, sizeof(DWORD)) != ERROR_SUCCESS)
                        goto RegWriteFailed;

                    Value = pArgs->cxDisplayScreen;
                    if (NKRegSetValueEx(key, L"cxDisplayScreen", 0, REG_DWORD, pValue, sizeof(DWORD)) != ERROR_SUCCESS)
                        goto RegWriteFailed;

                    Value = pArgs->cyDisplayScreen;
                    if (NKRegSetValueEx(key, L"cyDisplayScreen", 0, REG_DWORD, pValue, sizeof(DWORD)) != ERROR_SUCCESS)
                        goto RegWriteFailed;

                    Value = pArgs->bppScreen;
                    if (NKRegSetValueEx(key, L"bppScreen", 0, REG_DWORD, pValue, sizeof(DWORD)) != ERROR_SUCCESS)
                        goto RegWriteFailed;

                    Value = pArgs->pvFlatFrameBuffer;
                    if (NKRegSetValueEx(key, L"pvFlatFrameBuffer", 0, REG_DWORD, pValue, sizeof(DWORD)) != ERROR_SUCCESS)
                        goto RegWriteFailed;

                    Value = pArgs->cbScanLineLength;
                    if (NKRegSetValueEx(key, L"cbScanLineLength", 0, REG_DWORD, pValue, sizeof(DWORD)) != ERROR_SUCCESS)
                        goto RegWriteFailed;

                    Value = pArgs->cxPhysicalScreen;
                    if (NKRegSetValueEx(key, L"cxPhysicalScreen", 0, REG_DWORD, pValue, sizeof(DWORD)) != ERROR_SUCCESS)
                        goto RegWriteFailed;

                    Value = pArgs->cyPhysicalScreen;
                    if (NKRegSetValueEx(key, L"cyPhysicalScreen", 0, REG_DWORD, pValue, sizeof(DWORD)) != ERROR_SUCCESS)
                        goto RegWriteFailed;

                    Value = pArgs->RedMaskSize;
                    if (NKRegSetValueEx(key, L"RedMaskSize", 0, REG_DWORD, pValue, sizeof(DWORD)) != ERROR_SUCCESS)
                        goto RegWriteFailed;

                    Value = pArgs->RedMaskPosition;
                    if (NKRegSetValueEx(key, L"RedMaskPosition", 0, REG_DWORD, pValue, sizeof(DWORD)) != ERROR_SUCCESS)
                        goto RegWriteFailed;

                    Value = pArgs->GreenMaskSize;
                    if (NKRegSetValueEx(key, L"GreenMaskSize", 0, REG_DWORD, pValue, sizeof(DWORD)) != ERROR_SUCCESS)
                        goto RegWriteFailed;

                    Value = pArgs->GreenMaskPosition;
                    if (NKRegSetValueEx(key, L"GreenMaskPosition", 0, REG_DWORD, pValue, sizeof(DWORD)) != ERROR_SUCCESS)
                        goto RegWriteFailed;

                    Value = pArgs->BlueMaskSize;
                    if (NKRegSetValueEx(key, L"BlueMaskSize", 0, REG_DWORD, pValue, sizeof(DWORD)) != ERROR_SUCCESS)
                        goto RegWriteFailed;

                    Value = pArgs->BlueMaskPosition;
                    if (NKRegSetValueEx(key, L"BlueMaskPosition", 0, REG_DWORD, pValue, sizeof(DWORD)) != ERROR_SUCCESS)
                        goto RegWriteFailed;

                    if ((pArgs->MajorVersion > 1 || pArgs->MinorVersion >= 3) &&  pArgs->VideoRam.QuadPart) {
                        ASSERT(pArgs->VideoRam.HighPart == 0); // only support 4096MB
                        Value = pArgs->VideoRam.LowPart;
                        if (NKRegSetValueEx(key, L"VideoRam", 0, REG_DWORD, pValue, sizeof(DWORD)) != ERROR_SUCCESS)
                            goto RegWriteFailed;
                    }

                    ArgsSet = TRUE;

RegWriteFailed:
                    NKRegCloseKey(key);
                }
            }
        }
        __except (GetExceptionCode() == STATUS_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
        {
            RETAILMSG(ZONE_ERROR, (L"Failed to read VGA settings from bootloader"));
        }
    }

    return ArgsSet;
}

//------------------------------------------------------------------------------
//
//  Function:  SetDeviceDriverFlags
//
static BOOL SetDeviceDriverFlags(
                                 __in LPCWSTR szKeyPath, 
                                 DWORD flags
                                 )
{
    BOOL rc = FALSE;
    HKEY hKey;
    DWORD value;

    // Open/create key
    if (NKRegCreateKeyEx(HKEY_LOCAL_MACHINE, szKeyPath, 0, NULL, 0, 0, NULL,&hKey, &value) != ERROR_SUCCESS) 
        goto cleanUp;

    // Set value
    rc = NKRegSetValueEx(hKey, L"Flags", 0, REG_DWORD, (UCHAR*)(&flags), sizeof(DWORD)) == ERROR_SUCCESS;

    // Close key
    NKRegCloseKey(hKey);

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  GetDeviceDriverPath
//
static BOOL GetDeviceDriverPath(
                                 __in LPCWSTR szKeyName,
                                 DWORD cbDriverKeyPath,
                                 __out LPCWSTR szDriverKeyPath
                                 )
{
    BOOL rc = FALSE;
    HKEY hKey;
    DWORD value;
    WCHAR szSerialKeyName[] = L"Drivers\\BootArg\\";

    // Open/create key
    if (NKRegCreateKeyEx(HKEY_LOCAL_MACHINE, szSerialKeyName, 0, NULL, 0, 0, NULL,&hKey, &value) != ERROR_SUCCESS) 
        goto cleanUp;

    // Set value
    rc = NKRegQueryValueEx(hKey, szKeyName, NULL, NULL, (UCHAR*)(szDriverKeyPath), &cbDriverKeyPath) == ERROR_SUCCESS;

    // Close key
    NKRegCloseKey(hKey);

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  DisableSerialDriverForDebugPort
//
static BOOL DisableSerialDriverForDebugPort()
{
    if (g_pX86Info->ucComPort) 
    {
        // "SerialDbg " - the final blank character will be filled in later in this function
        WCHAR szSerialSubKeyName[] = L"SerialDbg ";
        WCHAR szSerialDriverKey[MAX_PATH];

        // convert the COM port to a character
        szSerialSubKeyName[_countof(szSerialSubKeyName)-2] = g_pX86Info->ucComPort + '0';

        if (GetDeviceDriverPath(szSerialSubKeyName, sizeof(szSerialDriverKey), szSerialDriverKey) == FALSE
            || SetDeviceDriverFlags(szSerialDriverKey, DEVFLAGS_NOLOAD) == FALSE)
        {
            RETAILMSG(ZONE_ERROR, (L"ERROR: Failed to reserve Debug Serial Port. This could cause unexpected behavior including crashes"));
            return FALSE;
        }
        else
        {
            RETAILMSG(ZONE_WARNING, (L"WARNING: COM%d: has been reserved exclusively for Debug Messages.", g_pX86Info->ucComPort));
        }
    }

    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  x86IoCtlHalInitRegistry
//
BOOL x86IoCtlHalInitRegistry(
                              UINT32 code, 
                              __in_bcount(nInBufSize) void *lpInBuf, 
                              UINT32 nInBufSize, 
                              __out_bcount(nOutBufSize) void *lpOutBuf, 
                              UINT32 nOutBufSize, 
                              __out UINT32 *lpBytesReturned
                              ) 
{
    UNREFERENCED_PARAMETER(code);
    UNREFERENCED_PARAMETER(lpInBuf);
    UNREFERENCED_PARAMETER(nInBufSize);
    UNREFERENCED_PARAMETER(lpOutBuf);
    UNREFERENCED_PARAMETER(nOutBufSize);

    DisableSerialDriverForDebugPort();

    SaveVGAArgs();

    
    if (lpBytesReturned)
        *lpBytesReturned = 0;

    return TRUE;
}
