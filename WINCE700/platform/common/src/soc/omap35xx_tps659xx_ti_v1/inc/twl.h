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
/*
================================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
================================================================================
*/
//
//  File: twl.h
//
#ifndef __TWL_H
#define __TWL_H

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  Define:  TWL_DEVICE_NAME
//
#define TWL_DEVICE_NAME         L"TWL1:"

//------------------------------------------------------------------------------
//
//  GUID:  DEVICE_IFC_TWL_GUID
//
// {DEF0A04B-B967-43db-959E-D9FC6225CDEB}
DEFINE_GUID(
    DEVICE_IFC_TWL_GUID, 0xdef0a04b, 0xb967, 0x43db, 
    0x95, 0x9e, 0xd9, 0xfc, 0x62, 0x25, 0xcd, 0xeb
    );


//------------------------------------------------------------------------------
//
//  Type:  DEVICE_IFC_TWL
//
//  This structure is used to obtain TWL interface funtion pointers used for
//  in-process calls via IOCTL_DDK_GET_DRIVER_IFC.
//
typedef struct {
    DWORD context;
    BOOL (*pfnReadRegs)(DWORD ctx, DWORD address, VOID *pBuffer, DWORD size);
    BOOL (*pfnWriteRegs)(DWORD ctx, DWORD address, const void *pBuffer, DWORD size);
    BOOL (*pfnSetIntrEvent)(DWORD ctx, DWORD intrId, HANDLE hEvent);
    BOOL (*pfnIntrEnable)(DWORD ctx, DWORD intrId);
    BOOL (*pfnIntrDisable)(DWORD ctx, DWORD intrId);
    BOOL (*pfnEnableWakeup)(DWORD ctx, DWORD intrId, BOOL bEnable);
} DEVICE_IFC_TWL;

//------------------------------------------------------------------------------
//
//  Type:  DEVICE_CONTEXT_TWL
//
//  This structure is used to store TWL device context
//
typedef struct {
    DEVICE_IFC_TWL ifc;
    HANDLE hDevice;
} DEVICE_CONTEXT_TWL;

//------------------------------------------------------------------------------

#define IOCTL_TWL_READREGS          \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0300, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct {
    DWORD address;
    DWORD size;
} IOCTL_TWL_READREGS_IN;

#define IOCTL_TWL_WRITEREGS         \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0301, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct {
    DWORD address;
    const void* pBuffer;
    DWORD size;
} IOCTL_TWL_WRITEREGS_IN;


//------------------------------------------------------------------------------
//
//  Functions: TWLxxx
//
__inline
HANDLE 
TWLOpen(
    )
{
    HANDLE hDevice;
    DEVICE_CONTEXT_TWL *pContext = NULL;

    hDevice = CreateFile(TWL_DEVICE_NAME, 0, 0, NULL, 0, 0, NULL);
    if (hDevice == INVALID_HANDLE_VALUE) goto cleanUp;

    // Allocate memory for our handler...
    pContext = (DEVICE_CONTEXT_TWL*)LocalAlloc(
        LPTR, sizeof(DEVICE_CONTEXT_TWL)
        );
    if (pContext == NULL)
        {
        CloseHandle(hDevice);
        goto cleanUp;
        }

    // Get function pointers, fail when IOCTL isn't supported...
    if (!DeviceIoControl(
            hDevice, IOCTL_DDK_GET_DRIVER_IFC, (VOID*)&DEVICE_IFC_TWL_GUID,
            sizeof(DEVICE_IFC_TWL_GUID), &pContext->ifc, sizeof(DEVICE_IFC_TWL),
            NULL, NULL))
        {
        CloseHandle(hDevice);
        LocalFree(pContext);
        pContext = NULL;
        goto cleanUp;
        }

    // Save device handle
    pContext->hDevice = hDevice;

cleanUp:
    return pContext;
}

__inline
VOID
TWLClose(
    HANDLE hContext
    )
{
    DEVICE_CONTEXT_TWL *pContext = (DEVICE_CONTEXT_TWL*)hContext;
    CloseHandle(pContext->hDevice);
    LocalFree(pContext);
}

__inline
BOOL
TWLReadRegs(
    HANDLE hContext, 
    DWORD address,
    VOID *pBuffer,
    DWORD size
    )
{
    DEVICE_CONTEXT_TWL *pContext = (DEVICE_CONTEXT_TWL*)hContext;
    return pContext->ifc.pfnReadRegs(
        pContext->ifc.context, address, pBuffer, size
        );
}
    
__inline
BOOL 
TWLWriteRegs(
    HANDLE hContext, 
    DWORD address,
    const VOID *pBuffer,
    DWORD size
    )
{
    DEVICE_CONTEXT_TWL *pContext = (DEVICE_CONTEXT_TWL*)hContext;
    return pContext->ifc.pfnWriteRegs(
        pContext->ifc.context, address, pBuffer, size
        );
}

__inline
BOOL 
TWLSetIntrEvent(
    HANDLE hContext, 
    DWORD intrId,
    HANDLE hEvent
    )
{
    DEVICE_CONTEXT_TWL *pContext = (DEVICE_CONTEXT_TWL*)hContext;
    return pContext->ifc.pfnSetIntrEvent(
        pContext->ifc.context, intrId, hEvent
        );
}

__inline
BOOL 
TWLIntrEnable(
    HANDLE hContext, 
    DWORD intrId
    )
{
    DEVICE_CONTEXT_TWL *pContext = (DEVICE_CONTEXT_TWL*)hContext;
    return pContext->ifc.pfnIntrEnable(pContext->ifc.context, intrId);
}

__inline
BOOL 
TWLIntrDisable(
    HANDLE hContext, 
    DWORD intrId
    )
{
    DEVICE_CONTEXT_TWL *pContext = (DEVICE_CONTEXT_TWL*)hContext;
    return pContext->ifc.pfnIntrDisable(pContext->ifc.context, intrId);
}

__inline
BOOL 
TWLWakeEnable(
    HANDLE hContext, 
    DWORD intrId,
    BOOL bEnable
    )
{
    DEVICE_CONTEXT_TWL *pContext = (DEVICE_CONTEXT_TWL*)hContext;
    return pContext->ifc.pfnEnableWakeup(pContext->ifc.context, intrId, bEnable);
}


//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif
