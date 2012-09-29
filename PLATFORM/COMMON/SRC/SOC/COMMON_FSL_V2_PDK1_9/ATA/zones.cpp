//------------------------------------------------------------------------------
// Copyright (c) Microsoft Corporation.  All rights reserved.
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
// Copyright (C) 2006-2008 Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  zones.cpp
//
//       ATA module Zonemask Macros
//
//------------------------------------------------------------------------------

#include <windows.h>
#include <debug.h>

DBGPARAM dpCurSettings = {
    TEXT("ATA"), {
    TEXT("Init"),TEXT("Deinit"),TEXT("Main"),TEXT("I/O"),
    TEXT("PCMCIA"),TEXT("MX"),TEXT("IOCTL"),TEXT("CDROM"),
    TEXT("DMA"),TEXT("Power"),TEXT("Undefined"),TEXT("Undefined"),
    TEXT("Warning"),TEXT("Error"),TEXT("Helper"), TEXT("CELOG") },
#if 0
    ZONEMASK_IOCTL | 
    ZONEMASK_DMA | 
    ZONEMASK_INIT | 
    ZONEMASK_HELPER | 
    ZONEMASK_DEINIT | 
    ZONEMASK_PCMCIA | 
    ZONEMASK_MX | 
    ZONEMASK_MAIN | 
    ZONEMASK_IO | 
    ZONEMASK_FUNC | 
    ZONEMASK_ERROR |
    ZONEMASK_CELOG 
#else    
    ZONEMASK_ERROR |
    ZONEMASK_WARNING |
    ZONEMASK_INIT
#endif    
};

