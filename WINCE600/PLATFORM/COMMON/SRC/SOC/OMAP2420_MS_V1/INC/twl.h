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
#ifndef __TWL_H
#define __TWL_H

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#define TWL_INTR_FAKE                         (-1)
#define TWL_INTR_CD1                      0         // SLOT1 INSERT/REMOVAL EVENT
#define TWL_INTR_CD2                      1         // SLOT2 INSERT/REMOVAL EVENT
#define TWL_INTR_DL1                      2         // SLOT1 DATA1 LOW EVENT
#define TWL_INTR_DL2                      3         // SLOT2 DATA1 LOW EVENT
#define TWL_INTR_LOWBAT                   4         // LOW BATTERY EVENT
#define TWL_INTR_HOTDIE                   5         // HOT DIE EVENT
#define TWL_INTR_UVLO                     6         // UVLO EVENT
#define TWL_INTR_TSHUT                    7			// THERMAL SHUTDOWN EVENT
#define TWL_INTR_RTCTMR                   8         // RTC TIMER EVENT
#define TWL_INTR_RTCALM                   9         // RTC ALARM EVENT
#define TWL_INTR_RCERR                    10        // RTC ERROR EVENT
#define TWL_INTR_PSHBTN                   11		// PUSH BUTTON EVE#define TWL_INTR_CHARGE_STOP


#define TWL_PRIMITIVE_SET_I2S_FREQUENCY       1
#define TWL_PRIMITIVE_SET_AUDIOPROFILE        2

#define MAX_TWL_BUFFER_SIZE                   256

//------------------------------------------------------------------------------
//
//  Define:  TWL_DEVICE_NAME
//
#define TWL_DEVICE_NAME         L"TWL1:"

//------------------------------------------------------------------------------
//
//  GUID:  DEVICE_IFC_TWL_GUID
//
DEFINE_GUID(
    DEVICE_IFC_TWL_GUID, 0xa49dd5e9, 0x1996, 0x4fc4,
    0xa1, 0x6c, 0xec, 0x7d, 0x4a, 0xa6, 0x86, 0x9
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
    BOOL (*pfnWriteRegs)(DWORD ctx, DWORD address, const VOID *pBuffer, DWORD size);
    BOOL (*pfnSetIntrEvent)(DWORD ctx, DWORD intrId, HANDLE hEvent);
    BOOL (*pfnIntrEnable)(DWORD ctx, DWORD intrId);
    BOOL (*pfnIntrDisable)(DWORD ctx, DWORD intrId);
    BOOL (*pfnProcessPrimitive)(DWORD ctx, UINT primitive, DWORD dwData, DWORD dwDataEx);
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
    DWORD size;
} IOCTL_TWL_WRITEREGS_IN;

#define IOCTL_TWL_STEREO_OUT        \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0302, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_TWL_MIC_IN_STEREO_OUT \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0303, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_TWL_DISABLE_AUDIO     \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0304, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_TWL_HEADSET    \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0305, METHOD_BUFFERED, FILE_ANY_ACCESS)

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
TWLProcessPrimitive(
    HANDLE  hContext,
    UINT    primitive,
    DWORD   dwData,
    DWORD   dwDataEx
    )
{
    DEVICE_CONTEXT_TWL *pContext = (DEVICE_CONTEXT_TWL*)hContext;
    return pContext->ifc.pfnProcessPrimitive(pContext->ifc.context, primitive,
        dwData, dwDataEx);
}

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif

