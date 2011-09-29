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
//  Header:  omap5912_timer32k.h
//
#ifndef __OMAP5912_TIMER32K_H
#define __OMAP5912_TIMER32K_H

//------------------------------------------------------------------------------

#define OMAP5912_TIMER_FREQ                  32768
#define OMAP5912_TIMER32K_COUNTS_PER_1MS     33

//------------------------------------------------------------------------------

typedef volatile struct {
    UINT32 TVR;
    UINT32 TCR;
    UINT32 CR;
} OMAP5912_TIMER32K_REGS;

//------------------------------------------------------------------------------

#define CR_TSS                          (1 << 0)
#define CR_TRB                          (1 << 1)
#define CR_INT_EN                       (1 << 2)
#define CR_ARL                          (1 << 3)

//------------------------------------------------------------------------------

#endif // __OMAP5912_TIMER32K_H
