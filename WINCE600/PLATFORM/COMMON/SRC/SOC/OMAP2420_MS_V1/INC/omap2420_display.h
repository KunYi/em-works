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
//
// Portions Copyright (c) Texas Instruments.  All rights reserved. 
//
//------------------------------------------------------------------------------
//
//  File:  omap2420_display.h
//
//  This header file is comprised of display module register details defined as
// structures and macros for configuring and controlling display module

#ifndef __OMAP2420_DISPLAY_H
#define __OMAP2420_DISPLAY_H

//------------------------------------------------------------------------------

//
// Display Subsystem Specific (DSS)
//

// Base Address: OMAP2420_DISS1_REGS_PA

typedef volatile struct
{
	UINT32 ulDSS_REVISIONNUMBER;	//offset 0x00, Revision ID
	UINT32 ulRESERVED_1[3];
	UINT32 ulDSS_SYSCONFIG;			//offset 0x10, OCP i/f params
	UINT32 ulDSS_SYSSTATUS;			//offset 0x14, module status info
	UINT32 ulRESERVED_2[10];
	UINT32 ulDSS_CONTROL;			//offset 0x40, DSS control bits
	UINT32 ulRESERVED_3[3];
	UINT32 ulDSS_PSA_LCD_REG1;		//offset 0x50, PSA LCD 32 LSB signature
	UINT32 ulDSS_PSA_LCD_REG2;		//offset 0x54, PSA LCD 32 MSB signature
	UINT32 ulDSS_PSA_VIDEO_REG;		//offset 0x58, PSA video signature and data availability
	UINT32 ulDSS_STATUS;			//offset 0x5C, DSS status
}
OMAP2420_DSS_REGS;

typedef volatile struct
{
	UINT32 ulH;
	UINT32 ulHV;
}
OMAP2420_FIRCOEF_REGS;

typedef volatile struct
{
	UINT32 ulBA0;				//offset 0x00, base addr config of vid buf0 for video win #n
	UINT32 ulBA1;				//offset 0x04, base addr config of vid buf1 for video win #n
	UINT32 ulPOSITION;			//offset 0x08, pos of vid win #n
	UINT32 ulSIZE;				//offset 0x0C, size of vid win #n
	UINT32 ulATTRIBUTES;		//offset 0x10, config of vid win #n
	UINT32 ulFIFO_THRESHOLD;	//offset 0x14, video FIFO associated with video pipeline #n
	UINT32 ulFIFO_SIZE_STATUS;	//offset 0x18, GFX FIFO size status
	UINT32 ulROW_INC;			//offset 0x1C, no of bytes to inc at end of row
	UINT32 ulPIXEL_INC;			//offset 0x20, no of bytes to incr bet 2 pixels
	UINT32 ulFIR;				//offset 0x24, resize factors for horz & vert up/down sampling of video win #n
	UINT32 ulPICTURE_SIZE;		//offset 0x28, video pict size conf 
	UINT32 ulACCU0;				//offset 0x2C, configures resize accumulator init values for horz and vert up/dn-sampling
	UINT32 ulACCU1;				//offset 0x30, configures resize accumulator init values for horz and vert up/dn-sampling
	OMAP2420_FIRCOEF_REGS aFIR_COEF[8];	// offset 0x34, configures up/dn-scaling coeff for vert & horz resize
										// of video pict asso with video win #n for the phase i
	UINT32 ulCONV_COEF0;		//offset 0x74, color space conversion matrix coeff
	UINT32 ulCONV_COEF1;		//offset 0x78, color space conversion matrix coeff
	UINT32 ulCONV_COEF2;		//offset 0x7C, color space conversion matrix coeff
	UINT32 ulCONV_COEF3;		//offset 0x80, color space conversion matrix coeff
	UINT32 ulCONV_COEF4;		//offset 0x84, color space conversion matrix coeff
}
OMAP2420_VID_REGS;

typedef volatile struct
{
	UINT32 ulDISPC_REVISION;			//offset 0x0, revision code
	UINT32 ulRESERVED_1[3];
	UINT32 ulDISPC_SYSCONFIG;			//offset 0x10, OCP i/f params
	UINT32 ulDISPC_SYSSTATUS;			//offset 0x14, module status info
	UINT32 ulDISPC_IRQSTATUS;			//offset 0x18, module internal events status
	UINT32 ulDISPC_IRQENABLE;			//offset 0x1C, module intr mask/unmask
	UINT32 ulRESERVED_2[8];
	UINT32 ulDISPC_CONTROL;				//offset 0x40, module configuration
	UINT32 ulDISPC_CONFIG;				//offset 0x44, shadow reg updated on VFP start period
	UINT32 ulDISPC_CAPABLE;				//offset 0x48, shadow reg updated on VFP start period
	UINT32 ulDISPC_DEFAULT_COLOR0;		//offset 0x4C, def solid bkgrnd color config
	UINT32 ulDISPC_DEFAULT_COLOR1;		//offset 0x50, def solid bkgrnd color config
	UINT32 ulDISPC_TRANS_COLOR0;		//offset 0x54, def trans colorvalue for video/graphics
	UINT32 ulDISPC_TRANS_COLOR1;		//offset 0x58, def trans colorvalue for video/graphics
	UINT32 ulDISPC_LINE_STATUS;			//offset 0x5C, current LCD panel display line number
	UINT32 ulDISPC_LINE_NUMBER;			//offset 0x60, LCD panel display line number for  intr&DMA req
	UINT32 ulDISPC_TIMING_H;			//offset 0x64, timing logic for HSYNC signal
	UINT32 ulDISPC_TIMING_V;			//offset 0x68, timing logic for VSYNC signal
	UINT32 ulDISPC_POL_FREQ;			//offset 0x6C, signal config
	UINT32 ulDISPC_DIVISOR;				//offset 0x70, divisor config
	UINT32 ulRESERVED_3;
	UINT32 ulDISPC_SIZE_DIG;			//offset 0x78, configures frame, size of digital o/p field
	UINT32 ulDISPC_SIZE_LCD;			//offset 0x7C, configure panel size
	UINT32 ulDISPC_GFX_BA0;				//offset 0x80, base addr of GFX buf
	UINT32 ulDISPC_GFX_BA1;				//offset 0x84, base addr of GFX buf
	UINT32 ulDISPC_GFX_POSITION;		//offset 0x88, config posof GFX win
	UINT32 ulDISPC_GFX_SIZE;			//offset 0x8C, size of GFX window
	UINT32 ulRESERVED_4[4];
	UINT32 ulDISPC_GFX_ATTRIBUTES;		//offset 0xA0, config attr of GFX
	UINT32 ulDISPC_GFX_FIFO_THRESHOLD;	//offset 0xA4, GFX FIFO config
	UINT32 ulDISPC_GFX_FIFO_SIZE_STATUS;//offset 0xA8, GFX FIFO size status
	UINT32 ulDISPC_GFX_ROW_INC;			//offset 0xAC, no of bytes to inc at end of row.
	UINT32 ulDISPC_GFX_PIXEL_INC;		//offset 0xB0, no of bytes to incr bet 2 pixels
	UINT32 ulDISPC_GFX_WINDOW_SKIP;		//offset 0xB4, no of bytes to skip during video win#n disp
	UINT32 ulDISPC_GFX_TABLE_BA;		//offset 0xB8, config base addr of palette buffer or the gamma tbl buf
	OMAP2420_VID_REGS tDISPC_VID1;		//offset 0xBC, video1 pipeline registers
	UINT32 ulRESERVED_5[2];
	OMAP2420_VID_REGS tDISPC_VID2;		//offset 0x14C, video2 pipeline registers
	UINT32 ulDISPC_DATA_CYCLE1;			//offset 0x1D4, color space conversion matrix coeff
	UINT32 ulDISPC_DATA_CYCLE2;			//offset 0x1D8, color space conversion matrix coeff
	UINT32 ulDISPC_DATA_CYCLE3;			//offset 0x1DC, color space conversion matrix coeff
}
OMAP2420_DISPC_REGS;

//
// Remote Frame Buffer
//

// Base Address: OMAP2420_RFBI1_REGS_PA (defined as 0x48050800)
   
typedef volatile struct
{
	UINT32 ulRFBI_REVISION;			//offset 0x0, revision ID
	UINT32 ulRESERVED_1[3];
	UINT32 ulRFBI_SYSCONFIG;		//offset 0x10, OCP i/f control
	UINT32 ulRFBI_SYSSTATUS;		//offset 0x14, status information
	UINT32 ulRESERVED_2[10];
	UINT32 ulRFBI_CONTROL;			//offset 0x40, module config
	UINT32 ulRFBI_PIXEL_CNT;		//offset 0x44, pixel cnt value
	UINT32 ulRFBI_LINE_NUMBER;		//offset 0x48, no of lines to sync the beginning of transfer
	UINT32 ulRFBI_CMD;				//offset 0x4C, cmd config
	UINT32 ulRFBI_PARAM;			//offset 0x50, parms config
	UINT32 ulRFBI_DATA;				//offset 0x54, data config
	UINT32 ulRFBI_READ;				//offset 0x58, read config
	UINT32 ulRFBI_STATUS;			//offset 0x5C, status config
	UINT32 ulRFBI_CONFIG0;			//offset 0x60, config0 module
	UINT32 ulRFBI_ONOFF_TIME0;		//offset 0x64, timing config
	UINT32 ulRFBI_CYCLE_TIME0;		//offset 0x68, timing config
	UINT32 ulRFBI_DATA_CYCLE1_0;	//offset 0x6C, data format for 1st cycle
	UINT32 ulRFBI_DATA_CYCLE2_0;	//offset 0x70, data format for 2nd cycle
	UINT32 ulRFBI_DATA_CYCLE3_0;	//offset 0x74, data format for 3rd cycle
	UINT32 ulRFBI_CONFIG1;			//offset 0x78, config1 module
	UINT32 ulRFBI_ONOFF_TIME1;		//offset 0x7C, timing config
	UINT32 ulRFBI_CYCLE_TIME1;		//offset 0x80, timing config
	UINT32 ulRFBI_DATA_CYCLE1_1;	//offset 0x84, data format for 1st cycle
	UINT32 ulRFBI_DATA_CYCLE2_1;	//offset 0x88, data format for 2nd cycle
	UINT32 ulRFBI_DATA_CYCLE3_1;	//offset 0x8C, data format for 3rd cycle
	UINT32 ulRFBI_VSYNC_WIDTH;		//offset 0x90, VSYNC min pulse
	UINT32 ulRFBI_HSYNC_WIDTH;		//offset 0x94, HSYNC max pulse
}
OMAP2420_RFBI_REGS, *pRFBIREGS;

//
// Video Encoder  - TBD
//
// Base Address: OMAP2420_VENC1_REGS_PA (defined as 0x48050C00)
   
typedef volatile struct
{
	UINT32 ulVENC_REV_ID;							//offset 0x00
	UINT32 ulVENC_STATUS;							//offset 0x04
	UINT32 ulVENC_F_CONTROL;						//offset 0x08
	UINT32 ulVENC_ulRESERVED_1;						//offset 0x0C
	UINT32 ulVENC_VIDOUT_CTRL;						//offset 0x10 
	UINT32 ulVENC_SYNC_CTRL;						//offset 0x14
	UINT32 ulVENC_ulRESERVED_2;						//offset 0x18
	UINT32 ulVENC_LLEN;								//offset 0x1C
	UINT32 ulVENC_FLENS;							//offset 0x20
	UINT32 ulVENC_HFLTR_CTRL;						//offset 0x24
	UINT32 ulVENC_CC_CARR_WSS_CARR;					//offset 0x28
	UINT32 ulVENC_C_PHASE;							//offset 0x2C
	UINT32 ulVENC_GAIN_U;							//offset 0x30
	UINT32 ulVENC_GAIN_V;							//offset 0x34
	UINT32 ulVENC_GAIN_Y;							//offset 0x38
	UINT32 ulVENC_BLACK_LEVEL;						//offset 0x3C
	UINT32 ulVENC_BLANK_LEVEL;						//offset 0x40
	UINT32 ulVENC_X_COLOR;							//offset 0x44
	UINT32 ulVENC_M_CONTROL;						//offset 0x48
	UINT32 ulVENC_BSTAMP_WSS_DATA;					//offset 0x4C
	UINT32 ulVENC_S_CARR;							//offset 0x50
	UINT32 ulVENC_LINE21;							//offset 0x54
	UINT32 ulVENC_LN_SEL;							//offset 0x58
	UINT32 ulVENC_L21__WC_CTL;						//offset 0x5C
	UINT32 ulVENC_HTRIGGER_VTRIGGER;				//offset 0x60
	UINT32 ulVENC_SAVID__EAVID;						//offset 0x64
	UINT32 ulVENC_FLEN__FAL;						//offset 0x68
	UINT32 ulVENC_LAL__PHASE_RESET;					//offset 0x6C
	UINT32 ulVENC_HS_INT_START_STOP_X;				//offset 0x70
	UINT32 ulVENC_HS_EXT_START_STOP_X;				//offset 0x74
	UINT32 ulVENC_VS_INT_START_X;					//offset 0x78
	UINT32 ulVENC_VS_INT_STOP_X__VS_INT_START_Y;	//offset 0x7C
	UINT32 ulVENC_VS_INT_STOP_Y__VS_EXT_START_X;	//offset 0x80
	UINT32 ulVENC_VS_EXT_STOP_X__VS_EXT_START_Y;	//offset 0x84
	UINT32 ulVENC_VS_EXT_STOP_Y;					//offset 0x88
	UINT32 ulVENC_ulRESERVED_3;						//offset 0x8C
	UINT32 ulVENC_AVID_START_STOP_X;				//offset 0x90
	UINT32 ulVENC_AVID_START_STOP_Y;				//offset 0x94
	UINT32 ulVENC_ulRESERVED_4[2];					//offset 0x98
	UINT32 ulVENC_FID_INT_START_X__FID_INT_START_Y;	//offset 0xA0
	UINT32 ulVENC_FID_INT_OFFSET_Y__FID_EXT_START_X;//offset 0xA4
	UINT32 ulVENC_FID_EXT_START_Y__FID_EXT_OFFSET_Y;//offset 0xA8
	UINT32 ulVENC_ulRESERVED_5;						//offset 0xAC
	UINT32 ulVENC_TVDETGP_INT_START_STOP_X;			//offset 0xB0
	UINT32 ulVENC_TVDETGP_INT_START_STOP_Y;			//offset 0xB4
	UINT32 ulVENC_GEN_CTRL;							//offset 0xB8
	UINT32 ulVENC_ulRESERVED_6;						//offset 0xBC
	UINT32 ulVENC_ulRESERVED_7;						//offset 0xC0
	UINT32 ulVENC_DAC_TST;							//offset 0xC4
	UINT32 ulVENC_DAC;								//offset 0xC8
}
OMAP2420_VENC_REGS, *pVENCREGS;

// DSS_SYSCONFIG register fields

#define DSS_SYSCONFIG_AUTOIDLE					(1 << 0)
#define DSS_SYSCONFIG_SOFTRESET					(1 << 1)

// DSS_SYSSTATUS register fields

#define DSS_SYSSTATUS_RESETDONE					(1 << 0)

// DSS_CONTROL register fields

#define DSS_CONTROL_DPLL_APLL_CLK				(1 << 0)
#define DSS_CONTROL_RESERVED1					(1 << 1)
#define DSS_CONTROL_VENC_CLOCK_MODE				(1 << 2)
#define DSS_CONTROL_VENC_CLOCK_4X_ENABLE		(1 << 3)
#define DSS_CONTROL_DAC_DEMEN					(1 << 4)
#define DSS_CONTROL_DAC_LCDPSACLR				(1 << 6)
#define DSS_CONTROL_DAC_LCDPSAON				(1 << 7)
#define DSS_CONTROL_DAC_VIDEOPSACLR				(1 << 8)
#define DSS_CONTROL_DAC_VIDEOPSAON				(1 << 9)

// DSS_PSA_LCD_REG_2 register fields

#define DSS_PSA_LCD_2_SIG_MSB(sig)				((sig) << 0)
#define DSS_PSA_LCD_2_DATA_AVAIL				(1 << 31)

// DSS_PSA_VIDEO_REG register fields

#define DSS_PSA_VIDEO_SIG(sig)					((sig) << 0)
#define DSS_PSA_VIDEO_DATA_AVAIL				(1 << 31)

// DSS_STATUS register fields

#define DSS_STATUS_DPLL_ENABLE					(1 << 0)
#define DSS_STATUS_APLL_ENABLE					(1 << 1)

// DISPC_SYSCONFIG register fields

#define DISPC_SYSCONFIG_AUTOIDLE				(1 << 0)
#define DISPC_SYSCONFIG_SOFTRESET				(1 << 1)
#define DISPC_SYSCONFIG_SIDLEMODE(mode)			((mode) << 3)
#define DISPC_SYSCONFIG_MIDLEMODE(mode)			((mode) << 12)

// DISPC_SYSSTATUS register fields

#define DISPC_SYSSTATUS_RESETDONE				(1 << 0)

// DISPC_CONTROL register fields

#define DISPC_CONTROL_LCDENABLE					(1 << 0)
#define DISPC_CONTROL_DIGITALENABLE				(1 << 1)
#define DISPC_CONTROL_MONCOLOR					(1 << 2)
#define DISPC_CONTROL_STNTFT					(1 << 3)
#define DISPC_CONTROL_M8B						(1 << 4)
#define DISPC_CONTROL_GOLCD						(1 << 5)
#define DISPC_CONTROL_GODIGITAL					(1 << 6)
#define DISPC_CONTROL_TFTDITHER_ENABLE			(1 << 7)
#define DISPC_CONTROL_TFTDATALINES_12			(0 << 8)
#define DISPC_CONTROL_TFTDATALINES_16			(1 << 8)
#define DISPC_CONTROL_TFTDATALINES_18			(2 << 8)
#define DISPC_CONTROL_TFTDATALINES_24			(3 << 8)
#define DISPC_CONTROL_SECURE					(1 << 10)
#define DISPC_CONTROL_RFBIMODE					(1 << 11)
#define DISPC_CONTROL_OVERLAY_OPTIMIZATION		(1 << 12)
#define DISPC_CONTROL_GPOUT0					(1 << 15)
#define DISPC_CONTROL_GPOUT1					(1 << 16)
#define DISPC_CONTROL_HT(ht)					((ht) << 17)
#define DISPC_CONTROL_TDMENABLE					(1 << 20)
#define DISPC_CONTROL_TDMPARALLEL_MODE_8		(0 << 21)
#define DISPC_CONTROL_TDMPARALLEL_MODE_9		(1 << 21)
#define DISPC_CONTROL_TDMPARALLEL_MODE_12		(2 << 21)
#define DISPC_CONTROL_TDMPARALLEL_MODE_16		(3 << 21)
#define DISPC_CONTROL_TDMCYCLE_FORMAT_11		(0 << 23)
#define DISPC_CONTROL_TDMCYCLE_FORMAT_21		(1 << 23)
#define DISPC_CONTROL_TDMCYCLE_FORMAT_31		(2 << 23)
#define DISPC_CONTROL_TDMCYCLE_FORMAT_32		(3 << 23)
#define DISPC_CONTROL_TDMUNUSED_BITS_LO			(0 << 25)
#define DISPC_CONTROL_TDMUNUSED_BITS_HI			(1 << 25)
#define DISPC_CONTROL_TDMUNUSED_BITS_SAME		(2 << 25)

// DISPC_CONFIG register fields

#define DISPC_CONFIG_PIXELGATED					(1 << 0)
#define DISPC_CONFIG_LOADMODE(mode)				((mode) << 1)
#define DISPC_CONFIG_PALETTEGAMMATABLE			(1 << 3)
#define DISPC_CONFIG_PIXELDATAGATED				(1 << 4)
#define DISPC_CONFIG_PIXELCLOCKGATED			(1 << 5)
#define DISPC_CONFIG_HSYNCGATED					(1 << 6)
#define DISPC_CONFIG_VSYNCGATED					(1 << 7)
#define DISPC_CONFIG_ACBIASGATED				(1 << 8)
#define DISPC_CONFIG_FUNCGATED					(1 << 9)
#define DISPC_CONFIG_TCKLCDENABLE				(1 << 10)
#define DISPC_CONFIG_TCKLCDSELECTION			(1 << 11)
#define DISPC_CONFIG_TCKDIGENABLE				(1 << 12)
#define DISPC_CONFIG_TCKDIGSELECTION			(1 << 13)

// DISPC_TIMING_H register fields

#define DISPC_TIMING_H_HSW(hsw)					(((hsw) - 1) << 0)
#define DISPC_TIMING_H_HFP(hfp)					(((hfp) - 1) << 8)
#define DISPC_TIMING_H_HBP(hbp)					(((hbp) - 1) << 20)

// DISPC_TIMING_V register fields

#define DISPC_TIMING_V_HSW(vsw)					((vsw) << 0)
#define DISPC_TIMING_V_HFP(vfp)					((vfp) << 8)
#define DISPC_TIMING_V_HBP(vbp)					((vbp) << 20)

// DISPC_POL_FREQ register fields

#define DISPC_POL_FREQ_ACB(acb)					((acb) << 0)
#define DISPC_POL_FREQ_ACBI(acbi)				((acbi) << 8)
#define DISPC_POL_FREQ_IVS						(1 << 12)
#define DISPC_POL_FREQ_IHS						(1 << 13)
#define DISPC_POL_FREQ_IPC						(1 << 14)
#define DISPC_POL_FREQ_IEO						(1 << 15)
#define DISPC_POL_FREQ_RF						(1 << 16)
#define DISPC_POL_FREQ_ONOFF					(1 << 17)

// DISPC_DIVISOR register fields

#define DISPC_DIVISOR_PCD(pcd)					((pcd) << 0)
#define DISPC_DIVISOR_LCD(lcd)					((lcd) << 16)

// DISPC_SIZE_DIG register fields

#define DISPC_SIZE_DIG_PPL(ppl)					(((ppl) - 1) << 0)
#define DISPC_SIZE_DIG_LPP(lpp)					(((lpp) - 1) << 16)

// DISPC_SIZE_LCD register fields

#define DISPC_SIZE_LCD_PPL(ppl)					(((ppl) - 1) << 0)
#define DISPC_SIZE_LCD_LPP(lpp)					(((lpp) - 1) << 16)

// DISPC_GFX_POSITION register fields

#define DISPC_GFX_POS_GFXPOSX(x)				((x) << 0)
#define DISPC_GFX_POS_GFXPOSY(y)				((y) << 16)

// DISPC_GFX_SIZE register fields

#define DISPC_GFX_SIZE_GFXSIZEX(x)				(((x) - 1) << 0)
#define DISPC_GFX_SIZE_GFXSIZEY(y)				(((y) - 1) << 16)

// DISPC_GFX_ATTRIBUTES register fields

#define DISPC_GFX_ATTR_GFXENABLE				(1 << 0)
#define DISPC_GFX_ATTR_GFXFORMAT(fmt)			((fmt) << 1)
#define DISPC_GFX_ATTR_GFXREPLICATIONENABLE		(1 << 5)
#define DISPC_GFX_ATTR_GFXBURSTSIZE(burst)		((burst) << 6)
#define DISPC_GFX_ATTR_GFXCHANNELOUT			(1 << 8)
#define DISPC_GFX_ATTR_GFXNIBBLEMODE			(1 << 9)
#define DISPC_GFX_ATTR_GFXENDIANESS				(1 << 10)

// DISPC_GFX_FIFO_THRESHOLD register fields

#define DISPC_GFX_FIFO_THRESHOLD_LOW(low)		((low) << 0)
#define DISPC_GFX_FIFO_THRESHOLD_HIGH(high)		((high) << 16)

// DISPC_VID1_POSITION and DISPC_VID2_POSITION register fields

#define DISPC_VID_POS_VIDPOSX(x)				((x) << 0)
#define DISPC_VID_POS_VIDPOSY(y)				((y) << 16)

// DISPC_VID1_SIZE and DISPC_VID2_SIZE register fields

#define DISPC_VID_SIZE_VIDSIZEX(x)				(((x) - 1) << 0)
#define DISPC_VID_SIZE_VIDSIZEY(y)				(((y) - 1) << 16)

// DISPC_VID1_ATTRIBUTES and DISPC_VID2_ATTRIBUTES register fields

#define DISPC_VID_ATTR_VIDENABLE				(1 << 0)
#define DISPC_VID_ATTR_VIDFORMAT(fmt)			((fmt) << 1)
#define DISPC_VID_ATTR_VIDRESIZE_NONE			(0 << 5)
#define DISPC_VID_ATTR_VIDRESIZE_HORIZONTAL		(1 << 5)
#define DISPC_VID_ATTR_VIDRESIZE_VERTICAL		(2 << 5)
#define DISPC_VID_ATTR_VIDRESIZE_BOTH			(3 << 5)
#define DISPC_VID_ATTR_VIDHRESIZE_CONF			(1 << 7)
#define DISPC_VID_ATTR_VIDVRESIZE_CONF			(1 << 8)
#define DISPC_VID_ATTR_VIDCOLORCONVENABLE		(1 << 9)
#define DISPC_VID_ATTR_VIDREPLICATIONENABLE		(1 << 10)
#define DISPC_VID_ATTR_VIDFULLRANGE				(1 << 11)
#define DISPC_VID_ATTR_VIDROTATION_0			(1 << 12)
#define DISPC_VID_ATTR_VIDROTATION_90			(1 << 12)
#define DISPC_VID_ATTR_VIDROTATION_180			(1 << 12)
#define DISPC_VID_ATTR_VIDROTATION_270			(1 << 12)
#define DISPC_VID_ATTR_VIDBURSTSIZE(burst)		((burst) << 14)
#define DISPC_VID_ATTR_VIDCHANNELOUT			(1 << 16)
#define DISPC_VID_ATTR_VIDENDIANNESS			(1 << 17)
#define DISPC_VID_ATTR_VIDROWREPEATENABLE		(1 << 18)

// DISPC_VID1_FIFO_THRESHOLD and DISPC_VID2_FIFO_THRESHOLD register fields

#define DISPC_VID_FIFO_THRESHOLD_LOW(low)		((low) << 0)
#define DISPC_VID_FIFO_THRESHOLD_HIGH(high)		((high) << 16)

// DISPC_VID1_PICTURE_SIZE and DISPC_VID2_PICTURE_SIZE register fields

#define DISPC_VID_PICTURE_SIZE_VIDORGSIZEX(x)	(((x) - 1) << 0)
#define DISPC_VID_PICTURE_SIZE_VIDORGSIZEY(y)	(((y) - 1) << 16)

// DISPC_VID1_FIR and DISPC_VID2_FIR register fields

#define DISPC_VID_FIR_VIDFIRHINC(inc)			((inc) << 0)
#define DISPC_VID_FIR_VIDFIRVINC(inc)			((inc) << 16)

// DISPC_VID1_CONV_COEF0 and DISPC_VID2_CONV_COEF0 register fields

#define DISPC_VID_CONV_COEF0_RY(ry)				((ry) << 0)
#define DISPC_VID_CONV_COEF0_RCR(rcr)			((rcr) << 16)

// DISPC_VID1_CONV_COEF1 and DISPC_VID2_CONV_COEF1 register fields

#define DISPC_VID_CONV_COEF1_RCB(rcb)			((rcb) << 0)
#define DISPC_VID_CONV_COEF1_GY(gy)				((gy) << 16)

// DISPC_VID1_CONV_COEF2 and DISPC_VID2_CONV_COEF2 register fields

#define DISPC_VID_CONV_COEF2_GCR(gcr)			((gcr) << 0)
#define DISPC_VID_CONV_COEF2_GCB(gcb)			((gcb) << 16)

// DISPC_VID1_CONV_COEF3 and DISPC_VID2_CONV_COEF3 register fields

#define DISPC_VID_CONV_COEF3_BY(by)				((by) << 0)
#define DISPC_VID_CONV_COEF3_BCR(bcr)			((bcr) << 16)

// DISPC_VID1_CONV_COEF4 and DISPC_VID2_CONV_COEF4 register fields

#define DISPC_VID_CONV_COEF4_BCB(bcb)			((bcb) << 0)

// DISPC_DAC_TST register fields

#define DISPC_DAC_TST_DACX						(1 << 1)
#define DISPC_DAC_TST_DAC_INVT					(1 << 3)
#define DISPC_DAC_TST_DAC_DX					(1 << 4)
#define DISPC_DAC_TST_DAC_SEL					(1 << 6)

//DISPC_IRQSTATUS

#define DISPC_IRQSTATUS_FRAMEDONE               (1 << 0)
#define DISPC_IRQSTATUS_VSYNC                   (1 << 1)
#define DISPC_IRQSTATUS_EVSYNC_EVEN             (1 << 2)
#define DISPC_IRQSTATUS_EVSYNCODD               (1 << 3)
#define DISPC_IRQSTATUS_ACBIASCOUNTSTATUS       (1 << 4)
#define DISPC_IRQSTATUS_PROGRAMMEDLINENUMBER    (1 << 5)
#define DISPC_IRQSTATUS_GFXFIFOUNDERFLOW        (1 << 6)
#define DISPC_IRQSTATUS_GFXENDWINDOW            (1 << 7)
#define DISPC_IRQSTATUS_PALLETEGAMMALOADING     (1 << 8)
#define DISPC_IRQSTATUS_OCPERROR                (1 << 9)
#define DISPC_IRQSTATUS_VID1FIFOUNDERFLOW       (1 << 10)
#define DISPC_IRQSTATUS_VID1ENDWINDOW           (1 << 11)
#define DISPC_IRQSTATUS_VID2FIFOUNDERFLOW       (1 << 12)
#define DISPC_IRQSTATUS_VID2ENDWINDOW           (1 << 13)
#define DISPC_IRQSTATUS_SYNCLOST                (1 << 14)
#define DISPC_IRQSTATUS_SYNCLOSTDIGITAL         (1 << 15)

#endif
