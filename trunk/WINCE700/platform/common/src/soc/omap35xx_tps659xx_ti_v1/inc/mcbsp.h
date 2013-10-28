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
// Portions Copyright (c) Texas Instruments.  All rights reserved.
//
//------------------------------------------------------------------------------
//
//  defines platform independent data structures and api's.
//
#ifndef __MCBSP_H
#define __MCBSP_H

#include <memtxapi.h>

//------------------------------------------------------------------------------
//
//  GUID:  DEVICE_IFC_MCBSP_GUID
//
DEFINE_GUID(
    DEVICE_IFC_MCBSP_GUID, 0x172504cf, 0x7352, 0x4b2d,
    0xbd, 0xc3, 0x44, 0x93, 0xdd, 0x67, 0xc1, 0x35
);


//------------------------------------------------------------------------------
//
//  Type:  DEVICE_IFC_BSP
//
//  This structure is used to obtain I2C interface funtion pointers used for
//  in-process calls via IOCTL_DDK_GET_DRIVER_IFC.
//
typedef struct {
    DWORD context;
    BOOL (*pfnIOctl)(DWORD context, DWORD code, UCHAR *pInBuffer, DWORD inSize, 
                        UCHAR *pOutBuffer, DWORD outSize, DWORD *pOutSize);
} DEVICE_IFC_MCBSP;


//------------------------------------------------------------------------------
//
//  Type:  DEVICE_CONTEXT_I2C
//
//  This structure is used to store I2C device context.
//
typedef struct {
    DEVICE_IFC_MCBSP ifc;
    HANDLE hDevice;
} DEVICE_CONTEXT_MCBSP;


//------------------------------------------------------------------------------
//
//  Functions: BSPxxx
//
__inline HANDLE 
MCBSPOpen(
    LPCTSTR szDev
    )
{
    HANDLE hDevice;
    DEVICE_CONTEXT_MCBSP *pContext = NULL;

    hDevice = CreateFile(szDev, 0, 0, NULL, 0, 0, NULL);
    if (hDevice == INVALID_HANDLE_VALUE) goto cleanUp;

    // Allocate memory for our handler...
    pContext = (DEVICE_CONTEXT_MCBSP*)LocalAlloc(
        LPTR, sizeof(DEVICE_CONTEXT_MCBSP)
        );
    if (pContext == NULL)
        {
        CloseHandle(hDevice);
        goto cleanUp;
        }

    // Get function pointers, fail when IOCTL isn't supported...
    if (!DeviceIoControl(
            hDevice, IOCTL_DDK_GET_DRIVER_IFC, (VOID*)&DEVICE_IFC_MCBSP_GUID,
            sizeof(DEVICE_IFC_MCBSP_GUID), &pContext->ifc, 
            sizeof(DEVICE_IFC_MCBSP),NULL, NULL)
            )
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

__inline BOOL
MCBSPRegisterDirectDataTransfer(
    HANDLE hContext,
    EXTERNAL_DRVR_DATA_TRANSFER_IN *pData,
    EXTERNAL_DRVR_DATA_TRANSFER_OUT *pDataOut
    )
{
    DEVICE_CONTEXT_MCBSP *pContext = (DEVICE_CONTEXT_MCBSP *)hContext;
    return pContext->ifc.pfnIOctl(pContext->ifc.context, 
        IOCTL_EXTERNAL_DRVR_REGISTER_TRANSFERCALLBACKS, (UCHAR*)pData, sizeof(EXTERNAL_DRVR_DATA_TRANSFER_IN),
        (UCHAR*)pDataOut, sizeof(EXTERNAL_DRVR_DATA_TRANSFER_OUT), NULL);
}

__inline BOOL
MCBSPUnregisterDirectDataTransfer(
    HANDLE hContext,
    EXTERNAL_DRVR_DATA_TRANSFER_IN *pData,
    EXTERNAL_DRVR_DATA_TRANSFER_IN *pDataOut
    )
{
    DEVICE_CONTEXT_MCBSP *pContext = (DEVICE_CONTEXT_MCBSP *)hContext;
    return pContext->ifc.pfnIOctl(pContext->ifc.context, 
        IOCTL_EXTERNAL_DRVR_UNREGISTER_TRANSFERCALLBACKS, (UCHAR*)pData, sizeof(EXTERNAL_DRVR_DATA_TRANSFER_IN),
        (UCHAR*)pDataOut, sizeof(EXTERNAL_DRVR_DATA_TRANSFER_OUT), NULL);
}

//------------------------------------------------------------------------------

#endif
