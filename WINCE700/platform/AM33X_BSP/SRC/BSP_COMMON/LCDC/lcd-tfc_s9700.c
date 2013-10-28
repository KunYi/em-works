/*
 *   Copyright (c) Texas Instruments Incorporated 2009. All Rights Reserved.
 *
 *   Use of this software is controlled by the terms and conditions found 
 *   in the license agreement under which this software has been supplied.
 * 
 *   lcdc-tfc_s9700.c
 */
#include <windows.h>
#include <ceddk.h> 
#include <lcdc.h>

int LCDC_panel_TFC_S9700RTWV35TR_init(void) 
{
	return 0;
}


struct lcd_panel panel_tfc_s9700 = {
	LCDC_PANEL_TFT | LCDC_INV_VSYNC | LCDC_INV_HSYNC |
	LCDC_HSVS_CONTROL, // config
	DISPC_PIXELFORMAT_ARGB32,	// bpp
	800,		// x_res 
	480,		// y_res
	30000000,	// pixel_clock
	47,			// hsw 
	39,			// hfp
	39,			// hbp
	2,			// vsw
	13,			// vfb
	29,			// vbp
	0,			// acb (????)	
	0,                  // mono_8bit_mode
	0,                  // tft_alt_mode
	COLOR_ACTIVE,    //panel_shade
	LCDC_panel_TFC_S9700RTWV35TR_init // init
};

struct lcd_panel *get_panel(void){
	return &panel_tfc_s9700;
}
