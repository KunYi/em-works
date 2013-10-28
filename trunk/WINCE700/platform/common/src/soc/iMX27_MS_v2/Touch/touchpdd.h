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
//  Copyright (C) 2004, Motorola Inc. All Rights Reserved
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  Touchpdd.h
//
//   Header file for touch panel driver.
//
//------------------------------------------------------------------------------
#ifndef __TOUCHPDD_H__
#define __TOUCHPDD_H__

//------------------------------------------------------------------------------
// Defines

// These macros define the touch panel sample rate and sample number.
#define TOUCH_SAMPLE_RATE_LOW           100    // Low sample rate
#define TOUCH_SAMPLE_RATE_HIGH          200    // High sample rate
#define TOUCH_PRIORITY                  109    // Touch IST Priority

//------------------------------------------------------------------------------
// Types

typedef enum TouchPowerStatus
{
    TouchPowerOff,
    TouchPowerOn
} TouchPowerStatus_c;

extern "C"
DWORD
WINAPI
TchPdd_Init(
    LPCTSTR pszActiveKey,
    TCH_MDD_INTERFACE_INFO* pMddIfc,
    TCH_PDD_INTERFACE_INFO* pPddIfc,
    DWORD hMddContext
    );

void
WINAPI
TchPdd_Deinit(
    DWORD hPddContext
    );

void
WINAPI
TchPdd_PowerUp(
    DWORD hPddContext
    );

void
WINAPI
TchPdd_PowerDown(
    DWORD hPddContext
    );

BOOL
WINAPI
TchPdd_Ioctl(
    DWORD hPddContext,
    DWORD dwCode,
    PBYTE pBufIn,
    DWORD dwLenIn,
    PBYTE pBufOut,
    DWORD dwLenOut,
    PDWORD pdwActualOut
    );

 extern "C"
 DWORD
 WINAPI
 TchPdd_Init(
     LPCTSTR pszActiveKey,
     TCH_MDD_INTERFACE_INFO* pMddIfc,
     TCH_PDD_INTERFACE_INFO* pPddIfc,
     DWORD hMddContext
     );

 void
 WINAPI
 TchPdd_Deinit(
     DWORD hPddContext
     );

 void
 WINAPI
 TchPdd_PowerUp(
     DWORD hPddContext
     );

 void
 WINAPI
 TchPdd_PowerDown(
     DWORD hPddContext
     );

 BOOL
 WINAPI
 TchPdd_Ioctl(
     DWORD hPddContext,
     DWORD dwCode,
     PBYTE pBufIn,
     DWORD dwLenIn,
     PBYTE pBufOut,
     DWORD dwLenOut,
     PDWORD pdwActualOut
     );

//------------------------------------------------------------------------------
// Functions


#endif //__TOUCHPDD_H__