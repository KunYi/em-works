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
#ifndef __DMADRV_H
#define __DMADRV_H

#include "dma_arbdefs.h"

#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------------------------------*/
/*                                  driver ioctl codes                                    */
/*----------------------------------------------------------------------------------------*/

typedef struct _dmaIOCTL
{
    uint    mParam[4];  /* 16 bytes */
} DMAIOCTL;

#define DMA_IOCTL_ENUM_GETSIZE          1
#define DMA_IOCTL_ENUM_GETDATA          2
#define DMA_IOCTL_OPEN                  3
#define DMA_IOCTL_ACQ                   4
#define DMA_IOCTL_FREE                  5
#define DMA_IOCTL_SET                   6
#define DMA_IOCTL_GET                   7
#define DMA_IOCTL_CLOSE                 10


/*----------------------------------------------------------------------------------------*/
/*                                  error codes                                           */
/*----------------------------------------------------------------------------------------*/

#define DMADRV_FACILITY            0x48
#define DMADRV_ERROR               0x80000000
#define DMADRV_WARNING             0x40000000
#define DMADRV_MAKEERR(x)          ((DMADRV_ERROR | (DMADRV_FACILITY<<16)) | (x))
#define DMADRV_MAKEWARN(x)         ((DMADRV_WARNING | (DMADRV_FACILITY<<16)) | (x))
#define DMADRV_FAILED(x)           (((x)&DMADRV_ERROR)!=0)

#define DMADRVERR_NOINIT                DMADRV_MAKEERR(0x0001)
    /* driver Init() not called successfully. */
#define DMADRVERR_ALREADYINIT           DMADRV_MAKEERR(0x0002)
    /* driver Init() already called successfully */
#define DMADRVERR_EXCEPTION             DMADRV_MAKEERR(0x0003)
    /* the function call resulted in an exception - critical internal error */
#define DMADRVERR_NOTSUPPORTED          DMADRV_MAKEERR(0x0004)
    /* the specified function is not supported (read/write) */
#define DMADRVERR_NOTOPEN               DMADRV_MAKEERR(0x0005)
    /* driver is not open */
#define DMADRVERR_INVALIDHANDLE         DMADRV_MAKEERR(0x0006)
    /* driver thinks the handle used for an operation is not valid */
#define DMADRVERR_ALREADYOPEN           DMADRV_MAKEERR(0x0007)
    /* driver is already open */
#define DMADRVERR_BADPOINTER            DMADRV_MAKEERR(0x0008)
    /* bad pointer passed to function */
#define DMADRVERR_UNKNOWNIOCTL          DMADRV_MAKEERR(0x0009)
    /* unknown IOCTL number passed */
#define DMADRVERR_INIT_FAILURE          DMADRV_MAKEERR(0x000A)
    /* failed to initialize dma controllers managed by this driver */
#define DMADRVERR_PARAM_INVALID         DMADRV_MAKEERR(0x000B)
    /* invalid parameter */
#define DMADRVERR_PARAM_SIZE_INVALID    DMADRV_MAKEERR(0x000C)
    /* parameter size is invalid */
#define DMADRVERR_RESOURCES             DMADRV_MAKEERR(0x000D)
    /* insufficient resources to complete request */
#define DMADRVERR_UNKNOWNCONTROLLER     DMADRV_MAKEERR(0x000E)
    /* unknown controller id specified */
#define DMADRVERR_UNKNOWNPROPERTY       DMADRV_MAKEERR(0x000F)
    /* property specified is not known to the driver */
#define DMADRVERR_PROP_READONLY         DMADRV_MAKEERR(0x0010)
    /* property is read-only */
#define DMADRVERR_PROP_WRITEONLY        DMADRV_MAKEERR(0x0010)
    /* property is write-only */

/*----------------------------------------------------------------------------------------*/
/*                                  driver C interface                                    */
/*----------------------------------------------------------------------------------------*/

DWORD DMA_Init(LPCTSTR szContext, LPCVOID pBusContext);
BOOL  DMA_Deinit(DWORD context);
DWORD DMA_Open(DWORD context, DWORD accessCode, DWORD shareMode);
BOOL  DMA_Close(DWORD context);
DWORD DMA_Read(DWORD context, PVOID pBuffer, DWORD size);
DWORD DMA_Write(DWORD context, PVOID pBuffer, DWORD size);
BOOL  DMA_IOControl(
            DWORD context, DWORD code, BYTE *pInBuffer, DWORD inSize, BYTE *pOutBuffer,
            DWORD outSize, DWORD *pOutSize);
VOID  DMA_PowerUp(DWORD context);
void  DMA_PowerDown(DWORD context);

#ifdef __cplusplus
};
#endif

#endif // __DMADRV_H