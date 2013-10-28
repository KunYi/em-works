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
//  Copyright (C) 2005-2006, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header:  mx27_usbname.h
//
//  Provides definitions for usb module based on Freescale MX27 SoC.
//
//------------------------------------------------------------------------------
#ifndef __MX27_USBNAME_H
#define __MX27_USBNAME_H


#if __cplusplus
extern "C" {
#endif

#define USBFunctionObjectName                     TEXT("USBFunc")
#define USBXcvrObjectName                         TEXT("USBXCVR")
#define USBHostObjectName                         TEXT("USBHost")
#define USBHostDetachName                         TEXT("USBDetach")
#define USBClockGatingName                        TEXT("USB_CLOCK_GATING")
#define USBTransferEventName                      TEXT("USBTransferEvent")

#ifdef __cplusplus
}
#endif

#endif // __MX27_USBNAME_H
