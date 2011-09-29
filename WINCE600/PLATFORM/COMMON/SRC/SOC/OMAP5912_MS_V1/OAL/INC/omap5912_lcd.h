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
//  File:  omap5912_lcd.h
//
#ifndef __OMAP5912_LCD_H
#define __OMAP5912_LCD_H

//------------------------------------------------------------------------------

typedef volatile struct {
    UINT32 CTRL;            // 0000
    UINT32 TIMING0;         // 0004
    UINT32 TIMING1;         // 0008
    UINT32 TIMING2;         // 000C
    UINT32 STATUS;          // 0010
    UINT32 SUBPANEL;        // 0014
    UINT32 LINEINT;         // 0018
    UINT32 DISPSTAT;        // 001C
} OMAP5912_LCD_REGS;

//------------------------------------------------------------------------------

#define LCD_TIMING0_PPL_MASK    (0x3FF << 0)
#define LCD_TIMING1_LPP_MASK    (0x3FF << 0)

#define LCD_CTRL_EN             (1 << 0)
#define LCD_CTRL_BW             (1 << 1)
#define LCD_CTRL_VSYNC          (1 << 2)
#define LCD_CTRL_DONE           (1 << 3)
#define LCD_CTRL_LOAD           (1 << 4)
#define LCD_CTRL_LINE           (1 << 5)
#define LCD_CTRL_LINE_DED       (1 << 6)
#define LCD_CTRL_TFT            (1 << 7)
#define LCD_CTRL_BE             (1 << 8)
#define LCD_CTRL_M8B            (1 << 9)
#define LCD_CTRL_LINE_CLR       (1 << 10)
#define LCD_CTRL_PXL_GATED      (1 << 11)

#define LCD_STATUS_DONE         (1 << 0)

//------------------------------------------------------------------------------

#endif // __OMAP5912_LCD_H
