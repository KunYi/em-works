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

#ifndef __TRITON_H__
#define __TRITON_H__


//------------------------------------------------------------------------------
//
//  Global:  dpCurSettings
//
//  Set debug zones names and initial setting for driver
//
#ifndef SHIP_BUILD

#define ZONE_ERROR          DEBUGZONE(0)
#define ZONE_WARN           DEBUGZONE(1)
#define ZONE_FUNCTION       DEBUGZONE(2)
#define ZONE_INIT           DEBUGZONE(3)
#define ZONE_INFO           DEBUGZONE(4)
#define ZONE_IST            DEBUGZONE(5)
#define ZONE_RTC            DEBUGZONE(15)

#endif

//------------------------------------------------------------------------------
//  Local Definitions

#define TWL_DEVICE_COOKIE       'twlD'

#define TWL_PAGE_REGISTER       0xFF

//------------------------------------------------------------------------------
//  Local Structures

typedef struct {
    DWORD cookie;
    DWORD irq;
    DWORD i2cAddress;
    DWORD priority256;
    HANDLE hI2C;
    HANDLE hICX;
    CRITICAL_SECTION cs;
    DWORD sysIntr;
    HANDLE hIntrEvent;
    HANDLE hIntrThread;
    BOOL intrThreadExit;
    HANDLE hSetIntrEvent[17];
} Device_t;

//------------------------------------------------------------------------------
//  Local Functions

BOOL
TWL_Deinit(
    DWORD context
    );

static 
DWORD 
TWL_IntrThread(
    VOID *pContext
    );

static
BOOL
TWL_ReadRegs(
    DWORD context, 
    DWORD address,
    UCHAR *pBuffer,
    DWORD size
    );

static
BOOL
TWL_WriteRegs(
    DWORD context, 
    DWORD address,
    const UCHAR *pBuffer,
    DWORD size
    );

static
BOOL
TWL_SetIntrEvent(
    DWORD context,
    DWORD intrId,
    HANDLE hEvent
    );

static
BOOL 
TWL_IntrEnable(
    DWORD context, 
    DWORD intrId
    );

static
BOOL 
TWL_IntrDisable(
    DWORD context, 
    DWORD intrId
    );

BOOL
ReadRegs(
    Device_t *pDevice,
    DWORD address,
    VOID *pBuffer,
    DWORD size
    );

BOOL
WriteRegs(
    Device_t *pDevice,
    DWORD address,
    const VOID *pBuffer,
    DWORD size
    );

#endif 

