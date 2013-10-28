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
//------------------------------------------------------------------------------
//
//  File: registry.c
//
#include <windows.h>
#include <devload.h>
#include <ceddk.h>
#include <csp.h>
//------------------------------------------------------------------------------

static DWORD GetStringParam(
    HKEY hKey, 
    void *pBase, 
    const DEVICE_REGISTRY_PARAM *pParam
) 
{
    DWORD status, size, type;
    WCHAR *pName;
    UCHAR *pBuffer, *pValue;

    pName = pParam->name;
    pValue = (UCHAR*)pBase + pParam->offset;
    size = pParam->size;

    // If there is parameter size we simply try to read value or used default
    if (size > 0) {

        status = RegQueryValueEx(hKey, pName, NULL, &type, pValue, &size);
        if (status == ERROR_SUCCESS || pParam->required) goto cleanUp;
        size = (wcslen((WCHAR*)pParam->pDefault) + 1) * sizeof(WCHAR);
        if (size > pParam->size) {
            status = ERROR_OUTOFMEMORY;
        } else {
            memcpy(pValue, pParam->pDefault, size);
            status = ERROR_SUCCESS;
        }            

    } else {

        // First find if value is there
        status = RegQueryValueEx(hKey, pName, NULL, &type, NULL, &size);
        // Value isn't in registry, break or use default
        if (status != ERROR_SUCCESS) {
            if (pParam->required) goto cleanUp;
            size = (wcslen((WCHAR*)pParam->pDefault) + 1) * sizeof(WCHAR);
            pBuffer = (UCHAR*)LocalAlloc(LMEM_FIXED, size);
            if (pBuffer == NULL) {
                status = ERROR_OUTOFMEMORY;
                goto cleanUp;
            }                        
            memcpy(pBuffer, pParam->pDefault, size);
            *(VOID**)pValue = pBuffer;
            status = ERROR_SUCCESS;
        } else {
            pBuffer = (UCHAR*)LocalAlloc(LMEM_FIXED, size);
            if (pBuffer == NULL) {
                status = ERROR_OUTOFMEMORY;
                goto cleanUp;
            }                        
            status = RegQueryValueEx(hKey, pName, NULL, &type, pBuffer, &size);
            *(VOID**)pValue = pBuffer;
        }
    }        

cleanUp:
    return status;
}

//------------------------------------------------------------------------------

static GetDWordParam(
    HKEY hKey, VOID *pBase, const DEVICE_REGISTRY_PARAM *pParam
) {
    DWORD status, size, type;
    WCHAR *pName;
    UCHAR *pValue;

    pName = pParam->name;
    pValue = (UCHAR*)pBase + pParam->offset;
    size = pParam->size;
    
    status = RegQueryValueEx(hKey, pName, NULL, &type, pValue, &size);
    if (status == ERROR_SUCCESS || pParam->required) goto cleanUp;

    *(DWORD*)pValue = (DWORD)pParam->pDefault;
    status = ERROR_SUCCESS;

cleanUp:
    return status;
}

//------------------------------------------------------------------------------

static GetMultiDWordParam(
    HKEY hKey, VOID *pBase, const DEVICE_REGISTRY_PARAM *pParam
) {
    DWORD status, size, type;
    WCHAR *pName, *pBuffer = NULL, *pPos;
    UCHAR *pValue, *pDefault;


    pName = pParam->name;
    pValue = (UCHAR*)pBase + pParam->offset;
    pDefault = (UCHAR*)pParam->pDefault;

    // Get registry value type and size
    status = RegQueryValueEx(hKey, pName, NULL, &type, NULL, &size);
    if (status != ERROR_SUCCESS) {
        // If value doesn't exists use default value if optional
        if (pParam->required) goto cleanUp;
        if (pDefault != NULL) memcpy(pValue, pDefault, pParam->size);
        status = ERROR_SUCCESS;
        goto cleanUp;
    }        

    // If type is DWORD and we expect it, simply read it
    if (type == REG_DWORD) {
        if (size == sizeof(DWORD)) {
            status = RegQueryValueEx(hKey, pName, NULL, NULL, pValue, NULL);
        } else {
            status = ERROR_BAD_LENGTH;
        }
    } else if (type == REG_SZ || type == REG_MULTI_SZ) {
        // Allocate buffer for key
        pBuffer = LocalAlloc(LPTR, size);
        if (pBuffer == NULL) {
            status = ERROR_OUTOFMEMORY;
            goto cleanUp;
        }
        // Read registry value (in most cases it should not fail)
        status = RegQueryValueEx(
            hKey, pName, NULL, NULL, (UCHAR*)pBuffer, &size
        );
        if (status != ERROR_SUCCESS) goto cleanUp;
        pPos = pBuffer;
        size = pParam->size;
        while (size >= sizeof(DWORD) && *pPos != L'\0') {
            while (*pPos == L' ' || (type == REG_SZ && *pPos == L',')) pPos++;
            *(DWORD*)pValue = wcstoul(pPos, &pPos, 16);
            pValue += sizeof(DWORD);
            size -= sizeof(DWORD);
            if (type == REG_MULTI_SZ && *pPos == L'\0') pPos++;
        }
        status = size == 0 ? ERROR_SUCCESS : ERROR_BAD_FORMAT;
    } else {
        status = ERROR_BAD_FORMAT;
    }        

cleanUp:
    if (pBuffer != NULL) LocalFree(pBuffer);
    return status;
}

//------------------------------------------------------------------------------

static DWORD GetBinParam(
    HKEY hKey, VOID *pBase, const DEVICE_REGISTRY_PARAM *pParam
) {
    DWORD status, size, type;
    WCHAR *pName;
    UCHAR *pBuffer, *pValue;

    pName = pParam->name;
    pValue = (UCHAR*)pBase + pParam->offset;
    size = pParam->size;

    // If there is parameter size we simply try to read value or used default
    if (size > 0) {

        status = RegQueryValueEx(hKey, pName, NULL, &type, pValue, &size);
        if (status == ERROR_SUCCESS || pParam->required) goto cleanUp;
        memcpy(pValue, pParam->pDefault, pParam->size);
        status = ERROR_SUCCESS;

    } else {

        // First find if value is there
        status = RegQueryValueEx(hKey, pName, NULL, &type, NULL, &size);
        // Value isn't in registry, break or use default
        if (status != ERROR_SUCCESS) {
            if (pParam->required) goto cleanUp;
            *(VOID**)pValue = NULL;
            status = ERROR_SUCCESS;
        } else {
            pBuffer = (UCHAR*)LocalAlloc(LMEM_FIXED, size);
            if (pBuffer == NULL) {
                status = ERROR_OUTOFMEMORY;
                goto cleanUp;
            }                        
            status = RegQueryValueEx(hKey, pName, NULL, &type, pBuffer, &size);
            *(VOID**)pValue = pBuffer;
        }
    }        

cleanUp:
    return status;
}

//------------------------------------------------------------------------------

DWORD GetDeviceRegistryParams(
    LPCWSTR context, VOID *pBase, DWORD count, 
    const DEVICE_REGISTRY_PARAM params[]
) {
    DWORD status = ERROR_SUCCESS;
    HKEY hKey;
    DWORD i;

    // Open registry context to read parameters
    if ((hKey = OpenDeviceKey(context)) == NULL) {
        // It looks like we didn't get active registry key,
        // try open key directly
        if ((status = RegOpenKeyEx(
            HKEY_LOCAL_MACHINE, context, 0, 0, &hKey
        )) != ERROR_SUCCESS) goto cleanUp;

    }

    // For all members of array
    for (i = 0; i < count && status == ERROR_SUCCESS; i++) {
        switch (params[i].type) {
        case PARAM_DWORD:
            status = GetDWordParam(hKey, pBase, &params[i]);
            break;
        case PARAM_STRING:
            status = GetStringParam(hKey, pBase, &params[i]);
            break;
        case PARAM_MULTIDWORD:
            status = GetMultiDWordParam(hKey, pBase, &params[i]);
            break;
        case PARAM_BIN:
            status = GetBinParam(hKey, pBase, &params[i]);
            break;
        default:
            status = STATUS_FAIL_CHECK;
            break;
        }
    }

    // Close key
    RegCloseKey(hKey);

    // Release allocated memory in case of failure
    if (status != ERROR_SUCCESS) {
        UCHAR *pBuffer, *pValue;

        while (i-- > 0) {
            pValue = (UCHAR*)pBase + params[i].offset;
            switch (params[i].type) {
            case PARAM_STRING:
            case PARAM_BIN:                
                if (params[i].size == 0) {
                    pBuffer = (UCHAR*)(*(VOID**)pValue);
                    *(VOID**)pValue = NULL;
                    LocalFree(pBuffer);
                }                   
                break;                    
            }            
        }
    }

cleanUp:
    return status;
}

//------------------------------------------------------------------------------
