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
/*
===============================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
===============================================================================
*/
//
//  File:  devloadex.h
//
//  This file contains OMAP specific ceddk extensions.
//
#ifndef _DEVLOADEX_H_
#define _DEVLOADEX_H_

#ifdef __cplusplus
extern "C" {
#endif

//
// indicates device driver is a omap specific device driver
//
#define DEVICEID_VALNAME                    L"DriverId"     // driver id (optional)


//
// These are the optional values under a device key.
//
#define DEVLOADEX_POWERFLAGS_VALNAME        L"PowerFlags"   // power mask

//
// Flag values.
//
#define POWERFLAGS_NONE                     0x00000000      // No flags defined
#define POWERFLAGS_PRESTATECHANGENOTIFY     0x00000001      // send notification on pre-device state change
#define POWERFLAGS_POSTSTATECHANGENOTIFY    0x00000002      // send notification on post-device state change
#define POWERFLAGS_CONTEXTRESTORE           0x00000100      // device recieves context restore notifiation


//------------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif

//-----------------------------------------------------------------------------
#endif //_DEVLOADEX_H_
