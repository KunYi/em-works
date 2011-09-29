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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
//------------------------------------------------------------------------------
//
//  File:  i2c.h
//
#ifndef __I2C_H
#define __I2C_H

#include "i2ctrans.h"

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  GUID:  DEVICE_IFC_I2C_GUID
//
DEFINE_GUID(
    DEVICE_IFC_I2C_GUID, 0xecdf6dd3, 0x4405, 0x4ebc, 
    0x87, 0x23, 0xf4, 0x48, 0x1a, 0x7, 0x2c, 0x4c 
);

//------------------------------------------------------------------------------
//
//  Type:  DEVICE_IFC_I2C
//
//  This structure is used to obtain I2C interface funtion pointers used for
//  in-process calls via IOCTL_DDK_GET_DRIVER_IFC.
//
typedef struct {
    DWORD context;
    BOOL  (*pfnSetSlaveAddress)(DWORD context, DWORD size, DWORD address);
    DWORD (*pfnTransact)(DWORD context, I2CTRANS *pTrans);
} DEVICE_IFC_I2C;

//------------------------------------------------------------------------------
//
//  Type:  DEVICE_CONTEXT_I2C
//
//  This structure is used to store I2C device context.
//
typedef struct {
    DEVICE_IFC_I2C ifc;
    HANDLE hDevice;
} DEVICE_CONTEXT_I2C;

//------------------------------------------------------------------------------

#define IOCTL_I2C_SET_SLAVE_ADDRESS     \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0200, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_I2C_TRANSACT     \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0201, METHOD_BUFFERED, FILE_ANY_ACCESS)


//------------------------------------------------------------------------------
//
//  Type:  I2C_SET_SLAVE_ADDRESS
//
//  This structure is used in IOCTL_I2C_SET_SLAVE_ADDRESS to pass input
//  parameters.
//
typedef struct {
    DWORD size;
    DWORD address;
} I2C_SET_SLAVE_ADDRESS;

//------------------------------------------------------------------------------
//
//  Functions: I2Cxxx
//
// example devicename L"I2C1:"
__inline HANDLE I2COpen(LPCWSTR devicename)
{
    HANDLE hDevice;
    DEVICE_CONTEXT_I2C *pContext = NULL;

    hDevice = CreateFile(devicename, 0, 0, NULL, 0, 0, NULL);
    if (hDevice == INVALID_HANDLE_VALUE) goto clean;

    // Allocate memory for our handler...
    if ((pContext = (DEVICE_CONTEXT_I2C *)LocalAlloc(
        LPTR, sizeof(DEVICE_CONTEXT_I2C)
    )) == NULL) {
        CloseHandle(hDevice);
        goto clean;
    }

    // Get function pointers, fail when IOCTL isn't supported...
    if (!DeviceIoControl(
        hDevice, IOCTL_DDK_GET_DRIVER_IFC, (VOID*)&DEVICE_IFC_I2C_GUID,
        sizeof(DEVICE_IFC_I2C_GUID), &pContext->ifc, sizeof(DEVICE_IFC_I2C),
        NULL, NULL
    )) {
        CloseHandle(hDevice);
        LocalFree(pContext);
        pContext = NULL;
        goto clean;
    }

    // Save device handle
    pContext->hDevice = hDevice;

clean:
    return pContext;
}

__inline VOID I2CClose(HANDLE hContext)
{
    DEVICE_CONTEXT_I2C *pContext = (DEVICE_CONTEXT_I2C *)hContext;

    if ( (pContext != NULL) && (pContext->hDevice != INVALID_HANDLE_VALUE) )
        CloseHandle(pContext->hDevice);

    if (pContext != NULL)
        LocalFree(pContext);
}

__inline BOOL I2CSetSlaveAddress(HANDLE hContext, DWORD size, DWORD address)
{
    DEVICE_CONTEXT_I2C *pContext = (DEVICE_CONTEXT_I2C *)hContext;
    return pContext->ifc.pfnSetSlaveAddress(pContext->ifc.context, size, address);
}

__inline DWORD I2CTransact(HANDLE hContext, I2CTRANS *pTrans)
{
    DEVICE_CONTEXT_I2C *pContext = (DEVICE_CONTEXT_I2C *)hContext;
    return pContext->ifc.pfnTransact(pContext->ifc.context, pTrans);
}
    
//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif
