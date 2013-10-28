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
//  Header: s3c6410_usbotg.h
//
//  Defines the USBOTG controller CPU register layout and definitions.
//
#ifndef __S3C6410_USBOTG_H
#define __S3C6410_USBOTG_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  Type: S3C6410_USBOTG_REG
//
//  Defines the USB device control register block. 
//

typedef struct {
    UINT32    OPHYPWR;
    UINT32    OPHYCLK;
    UINT32     ORSTCON;
}OTG_PHY_REG, *PS_OTG_PHY_REG;

//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif

