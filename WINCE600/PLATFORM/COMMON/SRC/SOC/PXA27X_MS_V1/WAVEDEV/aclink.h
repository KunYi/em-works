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
/* Copyright 2001 Intel Corp.  */
/*
 *++
 *  FACILITY:
 *      Hudson (Mass)
 *
 *  AUTHORS:
 *
 *
 *
 *  MODIFICATIONS:

 *--
*/
/* Copyright 1999,2000,2001 Intel Corp.  */
/*++

Module Name:  aclinkcontrol.h

Abstract:
 Intel XSC1 StrongARM  platform header file definitions for the AC 97 Control routines.

Notes:

$Date: 4/29/03 2:30p $

--*/
#ifndef __ACLINKCONTORL_H__
#define __ACLINKCONTORL_H__

#ifdef __cplusplus
extern "C"{
#endif

// DEV_... constants are used as bit identifiers in a uchar, so must be <=7
#define DEV_TOUCH               1
#define DEV_AUDIO               2
#define DEV_BATTERY  3

#define ACLINK_MUTEX_NAME       TEXT("ACLINK_CONTROL")  
#define COUNT_MAX               10000000
static HANDLE hACLinkControlMutex = NULL;


// mask definitions for the CodecAccessRegister
#define AC97CAR_CAIP            0x00000001

// Name of mutex for accessing touch/audio registers
#define CLEAR(reg, bit_mask) (reg &= ~bit_mask)
#define TEST(reg, bit_mask)  (reg & bit_mask)


BOOL ReleaseAC97Lock(void);
BOOL GetAC97Lock(void);
BOOL UnConfigureAC97Control(void);
BOOL ConfigureAC97Control(void);
BOOL WriteAC97(UINT8 Offset, UINT16 Data, UINT8 DevId);
BOOL ReadAC97(UINT8 Offset, UINT16 *pData, UINT8 DevId);
BOOL WriteAC97Raw(UINT8 Offset, UINT16 Data, UINT8 DevId);
BOOL ReadAC97Raw(UINT8 Offset, UINT16 *pData, UINT8 DevId);
BOOL AllocateACLinkResources(UINT8);
BOOL InitializeACLink(BOOL, UINT8);
BOOL DeInitializeACLink(BOOL, UINT8);
BOOL ColdResetAC97Control(void);
BOOL DeAllocateACLinkResources(UINT8 DevId);

#ifdef __cplusplus
}
#endif

#endif //__ACLINKCONTORL_H__
