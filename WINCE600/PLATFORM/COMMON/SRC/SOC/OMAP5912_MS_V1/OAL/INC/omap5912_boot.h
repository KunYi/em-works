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
//  File:  omap5912_boot.h
//
#ifndef __OMAP5912_BOOT_H
#define __OMAP5912_BOOT_H

//------------------------------------------------------------------------------

typedef struct {
    UINT32 start;
    UINT32 size;
    UINT32 flags;
    UINT32 align;
    UINT32 spare;
    UINT8  name[12];
} OMAP5912_BOOT_TOC;

//------------------------------------------------------------------------------

#endif

