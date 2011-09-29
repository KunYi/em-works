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
//  File:  omap5912_ocpi.h
//
#ifndef __OMAP5912_OCPI_H
#define __OMAP5912_OCPI_H

//------------------------------------------------------------------------------

typedef volatile struct {
    UINT32 ADDRFAULT;           // 0000 - Address fault
    UINT32 MCMDFAULT;           // 0004 - Master command fault
    UINT32 SINTERRUPT0;         // 0008 - SINTERRUPT0
    UINT32 ABORTTYPE;           // 000C - Type of abort
    UINT32 SINTERRUPT1;         // 0010 - SINTERRUPT1
    UINT32 PROTECT;             // 0014 - Protection
    UINT32 SECURE;              // 0018 - Secure Mode
} OMAP5912_OCPI_REGS;

//------------------------------------------------------------------------------

#endif
