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

//=============================================================================
// File Description:
//
// Defines some helpfule macros for Usb
//
//
//=============================================================================

#ifndef __USBDBGUTILS_H_
#define __USBDBGUTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

// #defines for code readability
#define CONTROL_ENDPT 0

// heplful macros
#define USBFN_IS_OUT_REQUESTTYPE(bmRequestType) ((bmRequestType & 0x80) == 0)
#define USBFN_IS_IN_REQUESTTYPE(bmRequestType) ((bmRequestType & 0x80) != 0)
#define ENDPTNUM(endPtAddr)     (endPtAddr & 0x0F)
#define IS_ENDPT_RX(endPtAddr)  ((endPtAddr & 0x80) == 0)
#define IS_ENDPT_TX(endPtAddr)  ((endPtAddr & 0x80) != 0)

#ifdef __cplusplus
}
#endif

#endif
