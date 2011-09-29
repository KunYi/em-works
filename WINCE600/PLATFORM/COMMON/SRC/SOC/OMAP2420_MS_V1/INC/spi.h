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
//  File:  spi.h
//
#ifndef __SPI_H
#define __SPI_H

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  Define:  SPI_DEVICE_NAME
//
#define SPI_DEVICE_NAME         L"SPI1:"

//------------------------------------------------------------------------------
//
//  GUID:  DEVICE_IFC_SPI_GUID
//
// {2E559225-C95E-4300-86E9-6A5CBC07328F}
DEFINE_GUID(
	DEVICE_IFC_SPI_GUID, 0x2e559225, 0xc95e, 0x4300, 
	0x86, 0xe9, 0x6a, 0x5c, 0xbc, 0x7, 0x32, 0x8f
	);


//------------------------------------------------------------------------------
//
//  Type:  DEVICE_IFC_SPI
//
//  This structure is used to obtain I2C interface funtion pointers used for
//  in-process calls via IOCTL_DDK_GET_DRIVER_IFC.
//
typedef struct {
    DWORD context;
    BOOL  (*pfnSetSlaveAddress)(DWORD context, DWORD address);
    DWORD (*pfnTransfer)(DWORD context, VOID *pBuffer);
} DEVICE_IFC_SPI;

//------------------------------------------------------------------------------
//
//  Type:  DEVICE_CONTEXT_SPI
//
//  This structure is used to store I2C device context.
//
typedef struct {
    DEVICE_IFC_SPI ifc;
    HANDLE hDevice;
} DEVICE_CONTEXT_SPI;

//------------------------------------------------------------------------------

#define IOCTL_SPI_SET_SLAVE_ADDRESS     \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0200, METHOD_BUFFERED, FILE_ANY_ACCESS)

//------------------------------------------------------------------------------
//
//  Functions: SPIxxx
//
__inline HANDLE SPIOpen()
{
    HANDLE hDevice;
    DEVICE_CONTEXT_SPI *pContext = NULL;

    hDevice = CreateFile(SPI_DEVICE_NAME, 0, 0, NULL, 0, 0, NULL);
    if (hDevice == INVALID_HANDLE_VALUE) goto clean;

    // Allocate memory for our handler...
    if ((pContext = (DEVICE_CONTEXT_SPI *)LocalAlloc(
        LPTR, sizeof(DEVICE_CONTEXT_SPI)
    )) == NULL) {
        CloseHandle(hDevice);
        goto clean;
    }

    // Get function pointers, fail when IOCTL isn't supported...
    if (!DeviceIoControl(
        hDevice, IOCTL_DDK_GET_DRIVER_IFC, (VOID*)&DEVICE_IFC_SPI_GUID,
        sizeof(DEVICE_IFC_SPI_GUID), &pContext->ifc, sizeof(DEVICE_IFC_SPI),
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

__inline VOID SPIClose(HANDLE hContext)
{
    DEVICE_CONTEXT_SPI *pContext = (DEVICE_CONTEXT_SPI *)hContext;
    CloseHandle(pContext->hDevice);
    LocalFree(pContext);
}

__inline BOOL SPISetSlaveAddress(HANDLE hContext, DWORD address)
{
    DEVICE_CONTEXT_SPI *pContext = (DEVICE_CONTEXT_SPI *)hContext;
    return pContext->ifc.pfnSetSlaveAddress(pContext->ifc.context, address);
}

__inline DWORD SPITransfer(HANDLE hContext, VOID *pBuffer)
{
    DEVICE_CONTEXT_SPI *pContext = (DEVICE_CONTEXT_SPI *)hContext;
    return pContext->ifc.pfnTransfer(pContext->ifc.context, pBuffer);
}
    
//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif
