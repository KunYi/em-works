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
//  File:  display.h
//
#ifndef __DISPLAY_H
#define __DISPLAY_H

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

//
// DISPC mode related parameters specific to GFX window
//
typedef struct {
	UINT32 width;			// number of pixels in a line
	UINT32 height;			// number of lines in a frame
	FLONG  red;				// red mask
    FLONG  green;			// green mask
    FLONG  blue;			// blue mask
    UINT32 sysconfig;		// sys config
    UINT32 control;			// control
    UINT32 config;			// config
    UINT32 timing_H;		// horizontal pulse timing
	UINT32 timing_V;		// vertical pulse timing
	UINT32 pol_freq;		// polling frequency
	UINT32 divisor;			// clock divider
	UINT32 size_LCD;		// LCD window size
	UINT32 size_DIG;		// TV window size
	UINT32 background_LCD;	// LCD background color
	UINT32 background_DIG;	// TV background color
	UINT32 GFX_pos;			// GFX window position
	UINT32 GFX_size;		// GFX window size
	UINT32 GFX_attr;		// GFX window attributes
	UINT32 GFX_FIFO_thres;	// GFX FIFO Threshold(Low & high)
	UINT32 GFX_row_inc;		// row increment
	UINT32 GFX_pixel_inc;	// pixel increment
	UINT32 GFX_window_skip;	// window skip
} OMAP2420_DISPLAY_MODE_INFO;

//------------------------------------------------------------------------------
// TV-Out command codes and settings
#define DRVESC_GET_TVOUT	6550
#define DRVESC_SET_TVOUT	6551

#define TVOUT_NONE			0
#define TVOUT_NTSC			1
#define TVOUT_PAL			2

//------------------------------------------------------------------------------
//
//  Function:  DisplayPddInit
//
//  This function initializes LCD display hardware. It returns handle which
//  must be used in all DisplayXXX calls.
//
HANDLE DisplayPddInit(LPCWSTR context);

//------------------------------------------------------------------------------
//
//  Function:  DisplayPddDeinit
//
//  This function deinitializes LCD display hardware.
//
VOID DisplayPddDeinit(HANDLE hContext);

//------------------------------------------------------------------------------
//
//  Function:  DisplayPddNumModes
//
//  This function returns number of display modes supported by display hardware.
//
int DisplayPddNumModes(HANDLE hContext);

//------------------------------------------------------------------------------
//
//  Function:  DisplayPddGetModeInfo
//
//  This function returns LCD controller setup info for given mode.
//
BOOL DisplayPddGetModeInfo(
    HANDLE hContext, int modeNumber, OMAP2420_DISPLAY_MODE_INFO *pInfo
);

//------------------------------------------------------------------------------
//
//  Function:  DisplayPddSetMode
//
//  This function sets LCD display hardware to given mode.
//
BOOL DisplayPddSetMode(HANDLE hContext, int modeNumber);

//------------------------------------------------------------------------------
//
//  Function:  DisplayPddSetPower
//
VOID DisplayPddSetPower(HANDLE hContext, CEDEVICE_POWER_STATE dx);

//------------------------------------------------------------------------------
//
//  Function:  DisplayPddPowerHandler
//
VOID DisplayPddPowerHandler(HANDLE hContext, BOOL off);

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif
