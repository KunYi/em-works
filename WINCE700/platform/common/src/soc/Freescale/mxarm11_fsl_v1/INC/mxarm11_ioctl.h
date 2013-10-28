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
//  Copyright (C) 2004-2007, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  mxarm11_ioctl.h
//
//  This header contains common IOCTL definitions that are specific to the
//  Freescale ARM11 chassis.
//
//------------------------------------------------------------------------------
#ifndef __MXARM11_IOCTL_H
#define __MXARM11_IOCTL_H

//------------------------------------------------------------------------------
// ARM11 CHASSIS IOCTL DEFINITIONS
//------------------------------------------------------------------------------
#define IOCTL_HAL_IRQ2SYSINTR  \
    CTL_CODE(FILE_DEVICE_HAL, 2048, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_HAL_REQUEST_DMA_IRQ_MAP  \
    CTL_CODE(FILE_DEVICE_HAL, 2049, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_HAL_RELEASE_DMA_IRQ_MAP  \
    CTL_CODE(FILE_DEVICE_HAL, 2050, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_HAL_REQUEST_GPIO_IRQ_MAP  \
    CTL_CODE(FILE_DEVICE_HAL, 2051, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_HAL_RELEASE_GPIO_IRQ_MAP  \
    CTL_CODE(FILE_DEVICE_HAL, 2052, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_HAL_FORCE_IRQ  \
    CTL_CODE(FILE_DEVICE_HAL, 2053, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_HAL_UNFORCE_IRQ  \
    CTL_CODE(FILE_DEVICE_HAL, 2054, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_HAL_DVFC_BUS_SCALE \
    CTL_CODE(FILE_DEVICE_HAL, 2056, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_HAL_QUERY_SI_VERSION \
    CTL_CODE(FILE_DEVICE_HAL, 2057, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct
{
    UINT32 irq;
    UINT32 chan;
} DMA_IRQ_MAP_PARMS, *PDMA_IRQ_MAP_PARMS;

typedef struct
{
    UINT32 irq;
    UINT32 port;
    UINT32 pin;
} GPIO_IRQ_MAP_PARMS, *PGPIO_IRQ_MAP_PARMS;

//------------------------------------------------------------------------------
// ARM11 CHASSIS IOCTL PROTOTYPES
//------------------------------------------------------------------------------
BOOL OALIoCtlHalIrq2Sysintr(UINT32 code, VOID *pInpBuffer,
    UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize);

BOOL OALIoCtlHalRequestDmaIrqMap(UINT32 code, VOID *pInpBuffer,
    UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize);

BOOL OALIoCtlHalReleaseDmaIrqMap(UINT32 code, VOID *pInpBuffer,
    UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize);

BOOL OALIoCtlHalRequestGpioIrqMap(UINT32 code, VOID *pInpBuffer, 
    UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize);

BOOL OALIoCtlHalReleaseGpioIrqMap(UINT32 code, VOID *pInpBuffer, 
    UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize);

BOOL OALIoCtlHalForceIrq(UINT32 code, VOID *pInpBuffer,
    UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize);

BOOL OALIoCtlHalUnforceIrq(UINT32 code, VOID *pInpBuffer,
    UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize);

BOOL OALIoCtlQueryDispSettings(UINT32 code, VOID *lpInBuf, 
    UINT32 nInBufSize, VOID *lpOutBuf, UINT32 nOutBufSize, UINT32 *lpBytesReturned);

BOOL OALIoCtlHalGetHWEntropy(UINT32, VOID*, UINT32, VOID*, UINT32, UINT32*);
BOOL OALIoCtlHalGetRandomSeed(UINT32, VOID *, UINT32, VOID *, UINT32, UINT32 *);
BOOL OALIoCtlHalGetRegSecureKeys(UINT32, VOID *, UINT32, VOID *, UINT32, UINT32 *);

//------------------------------------------------------------------------------

#endif // __MXARM11_IOCTL_H
