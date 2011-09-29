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
//  Header:  omap5912_timer.h
//
#ifndef __OMAP5912_TIMER_H
#define __OMAP5912_TIMER_H

//------------------------------------------------------------------------------

typedef volatile struct {
    UINT32 CNTL;
    UINT32 LOAD;
    UINT32 READ;
} OMAP5912_TIMER_REGS;

//------------------------------------------------------------------------------

#define TIMER_CNTL_ST           (1 << 0)
#define TIMER_CNTL_AR           (1 << 1)
#define TIMER_CNTL_PVT_2        (0 << 2)
#define TIMER_CNTL_PVT_4        (1 << 2)
#define TIMER_CNTL_PVT_8        (2 << 2)
#define TIMER_CNTL_PVT_16       (3 << 2)
#define TIMER_CNTL_PVT_32       (4 << 2)
#define TIMER_CNTL_PVT_64       (5 << 2)
#define TIMER_CNTL_PVT_128      (6 << 2)
#define TIMER_CNTL_PVT_256      (7 << 2)
#define TIMER_CNTL_CLK_EN       (1 << 5)
#define TIMER_CNTL_FREE         (1 << 6)

//------------------------------------------------------------------------------

#endif // __OMAP5912_TIMER_H
