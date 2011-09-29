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
//  This file defines constants used for LCD display initialization.
//
#ifndef __BSP_LCD_CFG_H
#define __BSP_LCD_CFG_H

#define BSP_LCD_WIDTH           240
#define BSP_LCD_HEIGHT          320

#define BSP_PALETTE_SIZE        0x100

#define BSP_LCD_SIZE            (DISPC_SIZE_LCD_LPP(BSP_LCD_HEIGHT) | DISPC_SIZE_LCD_PPL(BSP_LCD_WIDTH))
#define BSP_DIG_SIZE            (DISPC_SIZE_DIG_LPP(BSP_LCD_HEIGHT) | DISPC_SIZE_DIG_PPL(BSP_LCD_WIDTH))
#define BSP_GFX_SIZE            (DISPC_GFX_SIZE_GFXSIZEY(BSP_LCD_HEIGHT) | DISPC_GFX_SIZE_GFXSIZEX(BSP_LCD_WIDTH))
#define BSP_TV_SIZE             (DISPC_VID_SIZE_VIDSIZEY(BSP_LCD_HEIGHT) | DISPC_VID_SIZE_VIDSIZEX(BSP_LCD_WIDTH))
#define BSP_TV_PICTURE_SIZE     (DISPC_VID_PICTURE_SIZE_VIDORGSIZEY(BSP_LCD_WIDTH) | DISPC_VID_PICTURE_SIZE_VIDORGSIZEX(BSP_LCD_WIDTH))
//#define BSP_TV_PICTURE_SIZE   (DISPC_VID_PICTURE_SIZE_VIDORGSIZEY(BSP_LCD_HEIGHT) | DISPC_VID_PICTURE_SIZE_VIDORGSIZEX(BSP_LCD_WIDTH))

#define BSP_LCD_SYSCONFIG       (DISPC_SYSCONFIG_SIDLEMODE(1) | DISPC_SYSCONFIG_MIDLEMODE(1))
#define BSP_LCD_CONTROL         (DISPC_CONTROL_GPOUT1 | DISPC_CONTROL_GPOUT0 | DISPC_CONTROL_TFTDATALINES_16 | DISPC_CONTROL_STNTFT | DISPC_CONTROL_HT(2))
#define BSP_LCD_CONFIG          DISPC_CONFIG_LOADMODE(2)
#define BSP_LCD_TIMING_H        (DISPC_TIMING_H_HSW(4) | DISPC_TIMING_H_HFP(4) | DISPC_TIMING_H_HBP(65))
#define BSP_LCD_TIMING_V        (DISPC_TIMING_V_HSW(1) | DISPC_TIMING_V_HFP(2) | DISPC_TIMING_V_HBP(8))
#define BSP_LCD_POL_FREQ        0x00000000
#define BSP_LCD_DIVISOR         (DISPC_DIVISOR_PCD(5) | DISPC_DIVISOR_LCD(1))
#define BSP_LCD_DEFAULTCOLOR0   0x000000FF
#define BSP_LCD_DEFAULTCOLOR1   0x00FF0000

#define BSP_GFX_POS             (DISPC_GFX_POS_GFXPOSY(0) | DISPC_GFX_POS_GFXPOSX(0))
#define BSP_GFX_ATTR            (DISPC_GFX_ATTR_GFXENABLE | DISPC_GFX_ATTR_GFXFORMAT(6))
#define BSP_GFX_FIFO_THRESHOLD  (DISPC_GFX_FIFO_THRESHOLD_LOW(192) | DISPC_GFX_FIFO_THRESHOLD_HIGH(252))
#define BSP_GFX_ROW_INC         0x00000001
#define BSP_GFX_PIXEL_INC       0x00000001
#define BSP_GFX_WINDOW_SKIP     0x00000000

#define BSP_TV_POS              (DISPC_VID_POS_VIDPOSY(0) | DISPC_VID_POS_VIDPOSX(0))
#define BSP_TV_ATTR             (DISPC_VID_ATTR_VIDFORMAT(6) | DISPC_VID_ATTR_VIDCHANNELOUT)
#define BSP_TV_FIFO_THRESHOLD   (DISPC_VID_FIFO_THRESHOLD_LOW(192) | DISPC_VID_FIFO_THRESHOLD_HIGH(252))
#define BSP_TV_ROW_INC          0x00000001
#define BSP_TV_PIXEL_INC        0x00000001

#define BSP_LCD_I2CEXPADDR      0x20
#define BSP_LCD_ENBIT           (1 << 7)
#define BSP_LCD_BKENBIT         (1 << 5)

#endif	// __BSP_LCD_CFG_H
