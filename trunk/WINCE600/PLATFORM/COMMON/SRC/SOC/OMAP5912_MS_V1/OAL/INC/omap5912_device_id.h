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
//  Header:  omap5912_device_id.h
//
#ifndef __OMAP5912_DEVICE_ID_H
#define __OMAP5912_DEVICE_ID_H

//------------------------------------------------------------------------------

typedef volatile struct {

    UINT32 DIE_ID_LSB;
    UINT32 DIE_ID_MSB;

} OMAP5912_DEVICE_ID_REGS;

//------------------------------------------------------------------------------

#endif
