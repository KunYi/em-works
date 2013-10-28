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
//  Copyright (C) 2004, MOTOROLA, INC. All Rights Reserved
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  mx27_ioctl.h
//
//  This header contains common IOCTL definitions that are specific to the
//  Freescale ARM27.
//
//------------------------------------------------------------------------------
#ifndef __MX27_IOCTL_H
#define __MX27_IOCTL_H

//------------------------------------------------------------------------------
// MX27 IOCTL DEFINITIONS
// Kernel IOCTLs based on FILE_DEVICE_HAL
// Note that function codes 0-2047 are reserved for Microsoft Corporation, and
// 2048-4095 are reserved for OEM.
//------------------------------------------------------------------------------
#define IOCTL_HAL_IRQ2SYSINTR  \
    CTL_CODE(FILE_DEVICE_HAL, 2048, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_HAL_FORCE_IRQ  \
    CTL_CODE(FILE_DEVICE_HAL, 2049, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_HAL_UNFORCE_IRQ  \
    CTL_CODE(FILE_DEVICE_HAL, 2050, METHOD_BUFFERED, FILE_ANY_ACCESS)
    
// IOCTL for NANDFC configuration
#define IOCTL_HAL_SET_NANDFC_CFG   \
    CTL_CODE(FILE_DEVICE_HAL, 2051, METHOD_BUFFERED, FILE_ANY_ACCESS)
    
#define IOCTL_HAL_GET_NANDFC_CFG   \
    CTL_CODE(FILE_DEVICE_HAL, 2052, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_HAL_SET_ESDMISC_LH   \
    CTL_CODE(FILE_DEVICE_HAL, 2053, METHOD_BUFFERED, FILE_ANY_ACCESS)

// IOCTL for Clock Freq configuration
#define IOCTL_HAL_SET_CLK    \
    CTL_CODE(FILE_DEVICE_HAL, 2054, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_HAL_GET_CLK    \
    CTL_CODE(FILE_DEVICE_HAL, 2055, METHOD_BUFFERED, FILE_ANY_ACCESS)


//------------------------------------------------------------------------------
// NANDFC related IOCTL defines
//------------------------------------------------------------------------------
// Struct used by IOCTL_HAL_SET_NANDFC_CFG and IOCTL_HAL_GET_NANDFC_CFG
typedef struct  {
    UINT32 clock;       // desired NAND flash clock
    BOOL page2048;      // NAND flash page size. TRUE for 2048b, FALSE for 512b
    BOOL access16Bit;   // NAND flash access size. TRUE for 16bit, FALSE for 8bit.
} NANDFC_IOCTL_CFG, *PNANDFC_IOCTL_CFG;


//------------------------------------------------------------------------------
// MX27 IOCTL PROTOTYPES
//------------------------------------------------------------------------------
BOOL OALIoCtlHalIrq2Sysintr(UINT32 code, VOID *pInpBuffer,
    UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize);

BOOL OALIoCtlHalRequestDmaIrqMap(UINT32 code, VOID *pInpBuffer,
    UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize);

BOOL OALIoCtlHalForceIrq(UINT32 code, VOID *pInpBuffer,
    UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize);

BOOL OALIoCtlHalUnforceIrq(UINT32 code, VOID *pInpBuffer,
    UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize);

BOOL OALIoCtlQueryDispSettings(UINT32 code, VOID *lpInBuf, 
    UINT32 nInBufSize, VOID *lpOutBuf, UINT32 nOutBufSize, UINT32 *lpBytesReturned);

BOOL OALIoCtlHalSetNANDFCCfg(UINT32 code, VOID *pInpBuffer,
    UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize);
    
BOOL OALIoCtlHalGetNANDFCCfg(UINT32 code, VOID *pInpBuffer,
    UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize);


BOOL OALIoCtlHalGetHWEntropy(UINT32, VOID*, UINT32, VOID*, UINT32, UINT32*);
BOOL OALIoCtlHalGetRandomSeed(UINT32, VOID *, UINT32, VOID *, UINT32, UINT32 *);
BOOL OALIoCtlHalGetRegSecureKeys(UINT32, VOID *, UINT32, VOID *, UINT32, UINT32 *);

//------------------------------------------------------------------------------

#endif // __MX27_IOCTL_H
