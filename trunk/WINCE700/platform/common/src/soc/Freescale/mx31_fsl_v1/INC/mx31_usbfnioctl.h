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
//  Header:  MX31_usbfnioctl.h
//
//  Provides definitions for operation of function & mdd in Freescale USB implementation
//
//------------------------------------------------------------------------------
#ifndef __MX31_USBFNIOCTL_H
#define __MX31_USBFNIOCTL_H

#include <usbfnioctl.h>

#define USB_FEATURE_TEST_MODE          2   /* defined in USB2.0 section 9.4 (table 9-6) */

// According to usbfnioctl, values 0x200 to 0x2FF are reserved for OEM
#define IOCTL_UFN_SET_TEST_MODE                     _UFN_ACCESS_CTL_CODE(0x201)

#define USB_TEST_MODE_DISABLE           0
#define USB_TEST_MODE_J_STATE           1
#define USB_TEST_MODE_K_STATE           2
#define USB_TEST_MODE_SE0_NAK           3
#define USB_TEST_MODE_PACKET            4
#define USB_TEST_MODE_FORCE_ENABLE_HS   5
#define USB_TEST_MODE_FORCE_ENABLE_FS   6
#define USB_TEST_MODE_FORCE_ENABLE_LS   7
#define USB_TEST_MODE_MAX               USB_TEST_MODE_FORCE_ENABLE_LS  /* highest supported current test mode */

#endif // __MX31_USBFNIOCTL_H
