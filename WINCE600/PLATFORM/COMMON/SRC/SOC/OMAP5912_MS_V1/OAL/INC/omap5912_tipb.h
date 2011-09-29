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
//  Header:  omap5912_tipb.h
//
#ifndef __OMAP5912_TIPB_H
#define __OMAP5912_TIPB_H

//------------------------------------------------------------------------------

typedef volatile struct {
    UINT32 RHEA_CNTL;           // 0x0000
    UINT32 RHEA_BUS_ALLOC;      // 0x0004
    UINT32 ARM_RHEA_CNTL;       // 0x0008
    UINT32 ENH_RHEA_CNTL;       // 0x000C
    UINT32 DEBUG_ADDRESS;       // 0x0010
    UINT32 DEBUG_DATA_LSB;      // 0x0014
    UINT32 DEBUG_DATA_MSB;      // 0x0018
    UINT32 DEBUG_CNTL_SIG;      // 0x001C
    UINT32 ACCESS_CNTL;         // 0x0020
} OMAP5912_TIPB_REGS;

//------------------------------------------------------------------------------

#endif // __OMAP5912_TIPB_H
