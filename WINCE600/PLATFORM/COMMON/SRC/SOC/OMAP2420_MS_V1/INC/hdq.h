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
//  File:  hdq_1wire.h
//
//  This header defines interface for the HDQ bus interface.
//
#ifndef __HDQ_H
#define __HDQ_H
#include "omap2420_prcm.h"
#include "omap2420_hdq.h"
#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  Define:  HDQ_DEVICE_NAME
//
#define HDQ_DEVICE_NAME        L"HDQ1:"

//------------------------------------------------------------------------------
//
//  GUID:  DEVICE_IFC_HDQ_GUID
//
DEFINE_GUID(
    DEVICE_IFC_HDQ_GUID, 0x1b981662, 0x7631, 0x4879,
    0x96, 0x78, 0x62, 0x9e, 0xd2, 0x5b, 0xc1, 0x7b
);

//------------------------------------------------------------------------------
//
//  Type:  DEVICE_IFC_HDQ
//
//  This structure is used to obtain HDQ interface funtion pointers
//  used for in-process calls via IOCTL_DDK_GET_DRIVER_IFC.
//
typedef struct {
    DWORD context;
    BOOL  (*pfnWrite)(DWORD context, UCHAR address, USHORT data);
    DWORD (*pfnRead)(DWORD context, UCHAR address, USHORT *pData);
    BOOL  (*pfnSetMode)(DWORD context, DWORD mode);
} DEVICE_IFC_HDQ;

//------------------------------------------------------------------------------
//
//  Type:  DEVICE_CONTEXT_HDQ
//
//  This structure is used to store HDQ device context.
//
typedef struct {
    DEVICE_IFC_HDQ ifc;
    HANDLE hDevice;
} DEVICE_CONTEXT_HDQ;

//------------------------------------------------------------------------------
//
//  Define:  HDQ_MODE_xxx
//
//  Following defines the different modes of the module.
//
#define HDQ_MODE_HDQ8           0
#define HDQ_MODE_HDQ16          1

/* //------------------------------------------------------------------------------

BOOL HDQ_Deinit(DWORD context);
BOOL HDQ_Write(DWORD context, UCHAR address, USHORT data);
BOOL HDQ_Read(DWORD context, UCHAR address, USHORT *pData);
BOOL HDQ_SetMode(DWORD context, DWORD mode);
DWORD HDQ_Init(LPCTSTR szContext, LPCVOID pBusContext);
DWORD HDQ_Open(DWORD context, DWORD accessCode, DWORD shareMode);
BOOL HDQ_Close(DWORD context);
BOOL HDQ_IOControl(DWORD context, DWORD code, BYTE *pInBuffer, DWORD inSize, BYTE *pOutBuffer, DWORD outSize, DWORD *pOutSize);
VOID HDQ_PowerUp(DWORD context);
void HDQ_PowerDown(DWORD context);

//------------------------------------------------------------------------------
VOID* HdqOpen(void);
VOID HdqClose(HANDLE hContext);
BOOL HdqWrite8(HANDLE hContext, UCHAR address, UCHAR data);
BOOL HdqRead8(HANDLE hContext, UCHAR address, UCHAR *pData);
BOOL HdqWrite16(HANDLE hContext, UCHAR address, USHORT data);
BOOL HdqRead16(HANDLE hContext, UCHAR address, USHORT *pData);
VOID HdqSetMode(HANDLE hContext, DWORD mode);
 
//------------------------------------------------------------------------------
//  Local Structures

typedef struct {
	DWORD cookie;
	DWORD memBase;
	DWORD memLen;
	DWORD irq;
	DWORD breakTimeout;
	DWORD txTimeout;
	DWORD rxTimeout;
	OMAP2420_PRCM_REGS* pPRCMRegs;
	OMAP2420_HDQ_REGS* pHDQRegs; // uses OMAP2420_HDQ_1WIRE_REGS_PA
	HANDLE hParentBus;
	LONG instances;
	CRITICAL_SECTION cs;
	DWORD sysIntr;
	HANDLE hIntrEvent;
	DWORD mode;
	CEDEVICE_POWER_STATE powerState;
} HDQ_DEVICE;

BOOL bDumpOmapHdqRegs(HDQ_DEVICE* const pDevice);
*/
//------------------------------------------------------------------------------

__inline VOID* HdqOpen()
{
    HANDLE hDevice;
    DEVICE_CONTEXT_HDQ *pContext = NULL;

    hDevice = CreateFile(HDQ_DEVICE_NAME, 0, 0, NULL, 0, 0, NULL);
    if (hDevice == INVALID_HANDLE_VALUE) goto clean;

    // Allocate memory for our handler...
    if ((pContext = (DEVICE_CONTEXT_HDQ *)LocalAlloc(
        LPTR, sizeof(DEVICE_CONTEXT_HDQ)
    )) == NULL) {
        CloseHandle(hDevice);
        goto clean;
    }

    // Get function pointers, fail when IOCTL isn't supported...
    if (!DeviceIoControl(
        hDevice, IOCTL_DDK_GET_DRIVER_IFC, (VOID*)&DEVICE_IFC_HDQ_GUID,
        sizeof(DEVICE_IFC_HDQ_GUID), &pContext->ifc, sizeof(DEVICE_IFC_HDQ), 
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

__inline VOID HdqClose(HANDLE hContext)
{
    DEVICE_CONTEXT_HDQ *pContext = (DEVICE_CONTEXT_HDQ *)hContext;
    CloseHandle(pContext->hDevice);
    LocalFree(pContext);
}

__inline BOOL HdqWrite8(HANDLE hContext, UCHAR address, UCHAR data)
{
    DEVICE_CONTEXT_HDQ *pContext = (DEVICE_CONTEXT_HDQ *)hContext;
    return pContext->ifc.pfnWrite(
        pContext->ifc.context, address, (USHORT)data
    );
}

__inline BOOL HdqRead8(HANDLE hContext, UCHAR address, UCHAR *pData)
{
    DEVICE_CONTEXT_HDQ *pContext = (DEVICE_CONTEXT_HDQ*)hContext;
    BOOL rc;
    USHORT data;

    if ((rc = pContext->ifc.pfnRead(
        pContext->ifc.context, address, &data
    ))) *pData = (UCHAR)data;
    return rc;
}

__inline BOOL HdqWrite16(HANDLE hContext, UCHAR address, USHORT data)
{
    DEVICE_CONTEXT_HDQ *pContext = (DEVICE_CONTEXT_HDQ *)hContext;
    return pContext->ifc.pfnWrite(
        pContext->ifc.context, address, data
    );
}

__inline BOOL HdqRead16(HANDLE hContext, UCHAR address, USHORT *pData)
{
    DEVICE_CONTEXT_HDQ *pContext = (DEVICE_CONTEXT_HDQ*)hContext;
    
    return pContext->ifc.pfnRead(pContext->ifc.context, address, pData);
}

__inline VOID HdqSetMode(HANDLE hContext, DWORD mode)
{
    DEVICE_CONTEXT_HDQ *pContext = (DEVICE_CONTEXT_HDQ*)hContext;
    pContext->ifc.pfnSetMode(pContext->ifc.context, mode);
}

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#define HDQ_DEVICE_COOKIE       'hdqD'

#endif
