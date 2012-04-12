//------------------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
//  Use of this source code is subject to the terms of the Microsoft end-user
//  license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
//  If you did not accept the terms of the EULA, you are not authorized to use
//  this source code. For a copy of the EULA, please see the LICENSE.RTF on your
//  install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  common_ioctl.h
//
//  This header contains IOCTL definitions that are common to Freescale SoCs.
//
//------------------------------------------------------------------------------
#ifndef __COMMON_IOCTL_H
#define __COMMON_IOCTL_H

//------------------------------------------------------------------------------
// ARM11 CHASSIS IOCTL DEFINITIONS
//------------------------------------------------------------------------------
#define IOCTL_HAL_IRQ2SYSINTR  \
    CTL_CODE(FILE_DEVICE_HAL, 2048, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_HAL_FORCE_IRQ  \
    CTL_CODE(FILE_DEVICE_HAL, 2053, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_HAL_UNFORCE_IRQ  \
    CTL_CODE(FILE_DEVICE_HAL, 2054, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_HAL_DVFC_BUS_SCALE \
    CTL_CODE(FILE_DEVICE_HAL, 2056, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_HAL_QUERY_SI_VERSION \
    CTL_CODE(FILE_DEVICE_HAL, 2057, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_HAL_QUERY_BOARD_ID \
    CTL_CODE(FILE_DEVICE_HAL, 2058, METHOD_BUFFERED, FILE_ANY_ACCESS)

//------------------------------------------------------------------------------
// ARM11 CHASSIS IOCTL PROTOTYPES
//------------------------------------------------------------------------------
BOOL OALIoCtlHalIrq2Sysintr(UINT32 code, VOID *pInpBuffer,
    UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize);

BOOL OALIoCtlHalForceIrq(UINT32 code, VOID *pInpBuffer,
    UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize);

BOOL OALIoCtlHalUnforceIrq(UINT32 code, VOID *pInpBuffer,
    UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize);

BOOL OALIoCtlQueryDispSettings(UINT32 code, VOID *lpInBuf, 
    UINT32 nInBufSize, VOID *lpOutBuf, UINT32 nOutBufSize, UINT32 *lpBytesReturned);

BOOL OALIoCtlQuerySiVersion(UINT32 code, VOID *lpInBuf, 
    UINT32 nInBufSize, VOID *lpOutBuf, UINT32 nOutBufSize, UINT32 *lpBytesReturned);

BOOL OALIoCtlHalGetHWEntropy(UINT32, VOID*, UINT32, VOID*, UINT32, UINT32*);
BOOL OALIoCtlHalGetRandomSeed(UINT32, VOID *, UINT32, VOID *, UINT32, UINT32 *);
BOOL OALIoCtlHalGetRegSecureKeys(UINT32, VOID *, UINT32, VOID *, UINT32, UINT32 *);

//------------------------------------------------------------------------------

#endif // __COMMON_IOCTL_H
