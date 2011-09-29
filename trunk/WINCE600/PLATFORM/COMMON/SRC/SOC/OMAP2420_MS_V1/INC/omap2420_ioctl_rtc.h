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
//  This file contains OMAP specific oal extensions.
//
#ifndef __OALEX_H
#define __OALEX_H

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  Define: IOCTL_HAL_RTC_ALARM
//
//  This code is used to call from RTC driver when alarm occurs.
//
#define IOCTL_HAL_RTC_ALARM    \
    CTL_CODE(FILE_DEVICE_HAL, 1026, METHOD_BUFFERED, FILE_ANY_ACCESS)

BOOL
OALIoCtlHalRtcAlarm(
    UINT32, VOID*, UINT32, VOID*, UINT32, UINT32*
    );

#ifdef __cplusplus
}
#endif

#endif
