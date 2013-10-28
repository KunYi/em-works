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
// Copyright (c) 2007, 2008 BSQUARE Corporation. All rights reserved.

/*
================================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
================================================================================
*/
//
//  Header:  omap35xx_gptimer.h
//
#ifndef __OMAP35XX_32KSYNCNT_H
#define __OMAP35XX_32KSYNCNT_H


//------------------------------------------------------------------------------

typedef volatile struct {
    UINT32 REV;                 // 0000
    UINT32 SYSCONFIG;           // 0004
    UINT32 Reserved1;           // 0008
    UINT32 Reserved2;           // 000c
    UINT32 CR;                  // 0010
} OMAP_32KSYNCNT_REGS;

#define C32KSYNCNT_SYSCONFIG_IDLEMODE_FORCEIDLE      (0 << 3)
#define C32KSYNCNT_SYSCONFIG_IDLEMODE_NOIDLE         (1 << 3)

//------------------------------------------------------------------------------

#endif // __OMAP35XX_32KSYNCNT_H

