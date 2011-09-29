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
// Copyright (c) Texas Instruments Corporation.  All rights reserved. 
//
//------------------------------------------------------------------------------
//
//  Header:  omap2420_device_id.h
//
#ifndef __OMAP2420_DEVICE_ID_H
#define __OMAP2420_DEVICE_ID_H

//------------------------------------------------------------------------------

typedef volatile struct {
    UINT32 DIE_ID_0;           // 0008
    UINT32 DIE_ID_1;           // 000C
    UINT32 DIE_ID_2;           // 0008
    UINT32 DIE_ID_3;           // 000C
} OMAP2420_DEVICE_ID_REGS;

//------------------------------------------------------------------------------

#define OMAP2420_GP_DEVICE_MASK         (3<<30)
#define OMAP2420_NORMAL_MASK            (3<<2)
#define OMAP2420_EMULATION_MASK         (3<<0)

#define OMAP2420_PROD_ID_MASK           (0xFFFF<<1)

//------------------------------------------------------------------------------
    
#endif
