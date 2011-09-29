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
//  File:  clk.h
//
//  This file specifies clock module interface. It controls CLKM and UPLD
//  hardware modules.
//
#ifndef __CLK_H
#define __CLK_H

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#define CLK_DEVICE_NAME         L"CLK1:"

//------------------------------------------------------------------------------
//
//  GUID:  DEVICE_IFC_CLK_GUID
//
DEFINE_GUID(
    DEVICE_IFC_CLK_GUID, 0x618daace, 0xca73, 0x4069,
    0xb4, 0x54, 0x9f, 0xa7, 0x48, 0x35, 0xbb, 0x77
);

//------------------------------------------------------------------------------
//
//  Type:  DEVICE_IFC_CLK
//
//  This structure is used to obtain GPIO interface funtion pointers used for
//  in-process calls via IOCTL_DDK_GET_DRIVER_IFC.
//
typedef struct {
    DWORD context;
    BOOL  (*pfnAttach)(DWORD context, DEVICE_LOCATION *pDevLoc);
    BOOL  (*pfnDetach)(DWORD context);
    BOOL  (*pfnRequest)(DWORD context, DWORD id);
    BOOL  (*pfnRelease)(DWORD context, DWORD id);
    DWORD (*pfnFrequency)(DWORD context, DWORD id);
} DEVICE_IFC_CLK;

//------------------------------------------------------------------------------
//
//  Type:  DEVICE_CONTEXT_CLK
//
//  This structure is used to store CLK device context.
//
typedef struct {
    DEVICE_IFC_CLK ifc;
    HANDLE hDevice;
} DEVICE_CONTEXT_CLK;

//------------------------------------------------------------------------------

#define IOCTL_CLK_ATTACH        \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0300, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_CLK_DETACH        \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0301, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_CLK_REQUEST       \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0302, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_CLK_RELEASE       \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0303, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_CLK_FREQUENCY     \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0304, METHOD_BUFFERED, FILE_ANY_ACCESS)

//------------------------------------------------------------------------------
//
//  Function:  ClkOpen
//
//  This function opens clock module context for given device. Device is
//  specified by its location.
//
__inline HANDLE ClkOpen(DEVICE_LOCATION *pDevLoc)
{
    HANDLE hDevice;
    DEVICE_CONTEXT_CLK *pContext = NULL;

    // Open device
    hDevice = CreateFile(CLK_DEVICE_NAME, 0, 0, NULL, 0, 0, NULL);
    if (hDevice == INVALID_HANDLE_VALUE) goto clean;

    // Allocate memory for our handler...
    if ((pContext = (DEVICE_CONTEXT_CLK *)LocalAlloc(
        LPTR, sizeof(DEVICE_CONTEXT_CLK))) == NULL) {
        CloseHandle(hDevice);
        goto clean;
    }

    // Save device handle
    pContext->hDevice = hDevice;

    // Get function pointers, fail when IOCTL isn't supported...
    if (!DeviceIoControl(
        hDevice, IOCTL_DDK_GET_DRIVER_IFC, (VOID*)&DEVICE_IFC_CLK_GUID,
        sizeof(DEVICE_IFC_CLK_GUID), &pContext->ifc,
        sizeof(DEVICE_IFC_CLK), NULL, NULL
    )) {
        CloseHandle(hDevice);
        LocalFree(pContext);
        pContext = NULL;
        goto clean;
    }

    // Attach open context to device
    if (!pContext->ifc.pfnAttach(pContext->ifc.context, pDevLoc)) {
        CloseHandle(hDevice);
        LocalFree(pContext);
        pContext = NULL;
        goto clean;
    }

clean:
    return pContext;
}

//------------------------------------------------------------------------------
//
//  Function:  ClkClose
//
__inline VOID ClkClose(HANDLE hContext)
{
    DEVICE_CONTEXT_CLK *pContext = (DEVICE_CONTEXT_CLK *)hContext;

    // Deattach device from context
    pContext->ifc.pfnDetach(pContext->ifc.context);
    // Close open instance
    CloseHandle(pContext->hDevice);
    // Deallocate wrapper context
    LocalFree(pContext);
}

//------------------------------------------------------------------------------
//
//  Function:  ClkRequest
//
//  This function is called by device driver to request/enable clock for
//  driven hardware. In case that hardware has multiple clock sources, this
//  function should be called multiple times with diffent clock id. Id starts
//  from 0.
//
__inline BOOL ClkRequest(HANDLE hContext, DWORD id)
{
    DEVICE_CONTEXT_CLK *pContext = (DEVICE_CONTEXT_CLK *)hContext;
    return pContext->ifc.pfnRequest(pContext->ifc.context, id);
}

//------------------------------------------------------------------------------
//
//  Function:  ClkRelease
//
//  This function is called by device driver to release clock request for
//  driven hardware. In case that hardware has multiple clock sources, this
//  function should be called multiple times with diffent clock id's.
//
__inline BOOL ClkRelease(HANDLE hContext, DWORD id)
{
    DEVICE_CONTEXT_CLK *pContext = (DEVICE_CONTEXT_CLK *)hContext;
    return pContext->ifc.pfnRelease(pContext->ifc.context, id);
}

//------------------------------------------------------------------------------
//
//  Function:  ClkGetFrequency
//
//  This function returns clock frequency delivered to device.
//
__inline DWORD ClkGetFrequency(HANDLE hContext, DWORD id)
{
    DEVICE_CONTEXT_CLK *pContext = (DEVICE_CONTEXT_CLK *)hContext;
    return pContext->ifc.pfnFrequency(pContext->ifc.context, id);
}

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif
