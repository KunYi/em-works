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
/**************************************************************************
** Copyright 2000-2003 Intel Corporation. All Rights Reserved.
**
** Portions of the source code contained or described herein and all documents
** related to such source code (Material) are owned by Intel Corporation
** or its suppliers or licensors and is licensed by Microsoft Corporation for distribution.  
** Title to the Material remains with Intel Corporation or its suppliers and licensors. 
** Use of the Materials is subject to the terms of the Microsoft license agreement which accompanied the Materials.  
** No other license under any patent, copyright, trade secret or other intellectual
** property right is granted to or conferred upon you by disclosure or
** delivery of the Materials, either expressly, by implication, inducement,
** estoppel or otherwise 
** Some portion of the Materials may be copyrighted by Microsoft Corporation.

Module Name:  pmuioctl.h

Abstract:
Contains  IOCTL PMU Functions and Interfaces definition

**************************************************************************/


#ifndef __PMUIOCTL_H__
#define __PMUIOCTL_H__

// PMU IOCTLs
//
#define IOCTL_PMU_CONFIG     CTL_CODE(FILE_DEVICE_HAL, 4000, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_GET_CPU_ID     CTL_CODE(FILE_DEVICE_HAL, 4001, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PMU_CCF        CTL_CODE(FILE_DEVICE_HAL, 4002, METHOD_BUFFERED, FILE_ANY_ACCESS)

// Subcodes to IOCTL_PMU_CONFIG
//
#define PMU_ENABLE_IRQ  1
#define PMU_ENABLE_FIQ  2
#define PMU_DISABLE_IRQ 3
#define PMU_DISABLE_FIQ 4
#define PMU_READ_REG    5
#define PMU_WRITE_REG   6
#define PMU_ALLOCATE    7
#define PMU_RELEASE     8
#define PMU_OEM_INFO    9

// Subcodes to IOCTL_PMU_CCF
//
#define PMU_CCF_GETCURRENT      1
#define PMU_CCF_SETLOCK         2
#define PMU_CCF_UNLOCK          3

// Subcodes to IOCTL_TURBO_MODE
//
#define PWR_ENTER_TURBO     1
#define PWR_EXIT_TURBO      2

// Struct for OEM config info
//
typedef struct
{
    unsigned long sysintrID;
    unsigned long PMUglobals;
} OEMInfo, *POEMInfo;

// Struct for PMU Register access and
// other PMU Config calls
//
typedef struct
{
    unsigned long subcode;
    unsigned long PMUReg;
    unsigned long PMUValue;
    LPVOID        pCallback;
    OEMInfo       OEMData;
} PMURegInfo, *PPMURegInfo;

// Struct for PMU CCF access
//
typedef struct
{
    unsigned long subcode;
    unsigned long newFreq;
    unsigned long curFreq;
    LPVOID        pCallback;
} PMUCCFInfo, *PPMUCCFInfo;

// Struct for CPU ID info
//
typedef struct
{
    unsigned long CPUId;
} CPUIdInfo, *PCPUIdInfo;

// PMU IOCTL functions
//
BOOL OALIoCtlPMUCCFCall(
    UINT32 code, VOID *pInpBuffer, UINT32 inpSize, VOID *pOutBuffer,
    UINT32 outSize, UINT32 *pOutSize);
    
BOOL OALIoCtlPMUConfigCall(
    UINT32 code, VOID *pInpBuffer, UINT32 inpSize, VOID *pOutBuffer,
    UINT32 outSize, UINT32 *pOutSize);
    
BOOL OALIoCtlGetCPUIdCall(
    UINT32 code, VOID *pInpBuffer, UINT32 inpSize, VOID *pOutBuffer,
    UINT32 outSize, UINT32 *pOutSize);

#endif
