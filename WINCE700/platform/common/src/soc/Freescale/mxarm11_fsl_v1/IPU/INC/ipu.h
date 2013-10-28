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
//------------------------------------------------------------------------------
//
//  Copyright (C) 2005-2007, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  Ipu.h
//
//  Common IPU definitions
//
//-----------------------------------------------------------------------------

#ifndef __IPU_H__
#define __IPU_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines
#define IPU_IOCTL_ENABLE_IC                        0
#define IPU_IOCTL_DISABLE_IC                       1
#define IPU_IOCTL_ENABLE_CSI                       2
#define IPU_IOCTL_DISABLE_CSI                      3
#define IPU_IOCTL_ENABLE_SDC                       4
#define IPU_IOCTL_DISABLE_SDC                      5
#define IPU_IOCTL_ENABLE_ADC                       6
#define IPU_IOCTL_DISABLE_ADC                      7
#define IPU_IOCTL_ENABLE_DI                        8
#define IPU_IOCTL_DISABLE_DI                       9
#define IPU_IOCTL_ENABLE_PF                        10
#define IPU_IOCTL_DISABLE_PF                       11

#define IPU_PRP_INTR_EVENT                         L"PRP Interrupt"
#define IPU_PP_INTR_EVENT                          L"PP Interrupt"
#define IPU_PF_INTR_EVENT                          L"PF Interrupt"
#define IPU_SDC_BG_INTR_EVENT                      L"SDC BG Interrupt"
#define IPU_SDC_FG_INTR_EVENT                      L"SDC FG Interrupt"
#define IPU_ADC_INTR_EVENT                         L"ADC Interrupt"

#define IDMAC_INTERLEAVED_FORMAT_CODE_RGB          4
#define IDMAC_INTERLEAVED_FORMAT_CODE_YUV444       4
#define IDMAC_INTERLEAVED_FORMAT_CODE_YUV422       6
#define IDMAC_INTERLEAVED_FORMAT_CODE_GENERIC      7

#define IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV444   1
#define IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV422   2
#define IDMAC_NON_INTERLEAVED_FORMAT_CODE_YUV420   3

#define IDMAC_BPP_CODE_32                          0
#define IDMAC_BPP_CODE_24                          1
#define IDMAC_BPP_CODE_16                          2
#define IDMAC_BPP_CODE_8                           3
#define IDMAC_BPP_CODE_4                           4

#define IDMAC_PIXEL_BURST_CODE_4                   3
#define IDMAC_PIXEL_BURST_CODE_8                   7
#define IDMAC_PIXEL_BURST_CODE_16                  15
#define IDMAC_PIXEL_BURST_CODE_32                  31

#define SCREEN_PIX_WIDTH_VGA        640
#define SCREEN_PIX_HEIGHT_VGA       480

#define SCREEN_PIX_WIDTH_D1         720
#define SCREEN_PIX_HEIGHT_D1_NTSC   480
#define SCREEN_PIX_HEIGHT_D1_PAL    576

#define DISP_MIN_WIDTH                             8
#define DISP_MIN_HEIGHT                            8
#define DISP_BPP                                   16
#define DISP_BYTES_PP                              (DISP_BPP / 8)

//
// Color conversion coefficient table
//
// RGB to YUV
static const UINT16 rgb2yuv_tbl[4][13] =
{
    // C00 C01   C02    C10     C11    C12    C20   C21     C22     A0       A1       A2    Scale
    {0x4D, 0x96, 0x1D, 0x1D5, 0x1AB, 0x80, 0x80, 0x195, 0x1EB, 0x00, 0x200, 0x200, 1},  // A1
    {0x42, 0x81, 0x19, 0x1DA, 0x1B6, 0x70, 0x70, 0x1A2, 0x1EE, 0x40, 0x200, 0x200, 1},  // A0
    {0x36, 0xB7, 0x12, 0x1E3, 0x19D, 0x80, 0x80, 0x18C, 0x1F4, 0x00, 0x200, 0x200, 1},  // B1
    {0x2F, 0x9D, 0x10, 0x1E6, 0x1A9, 0x70, 0x70, 0x19A, 0x1F6, 0x40, 0x200, 0x200, 1}   // B0
};



// YUV to RGB
static const UINT16 yuv2rgb_tbl[4][13] =
{
    // C00 C01   C02   C10    C11     C12     C20   C21   C22    A0         A1       A2      Scale
    {0x95, 0x00, 0xCC, 0x95, 0x1CE, 0x198, 0x95, 0xFF, 0x00, 0x1E42, 0x10A, 0x1DD6, 2}, //A1
//    {0x80, 0x00, 0xB4, 0x80, 0x1D4, 0x1A5, 0x80, 0xE3, 0x00, 0x1E99, 0x10F, 0x1E3A, 2}, //A1
    {0x4A, 0x66, 0x00, 0x4A, 0x1CC, 0x1E7, 0x4A, 0x00, 0x81, 0x1E42, 0x10F, 0x1DD6, 3}, //A0
    {0x80, 0x00, 0xCA, 0x80, 0x1E8, 0x1C4, 0x80, 0xED, 0x00, 0x1E6D, 0xA8,  0x1E25, 0}, //B1
    {0x4A, 0x73, 0x00, 0x4A, 0x1DE, 0x1F2, 0x4A, 0x00, 0x87, 0x1E10, 0x9A,  0x1DBE, 3}  //B0
};

//------------------------------------------------------------------------------
// Types
typedef enum IPU_INT_ID_ENUM {
    IPU_INT_DMAIC_0 = 0,
    IPU_INT_DMAIC_1 = 1,
    IPU_INT_DMAIC_2 = 2,
    IPU_INT_DMAIC_3 = 3,
    IPU_INT_DMAIC_4 = 4,
    IPU_INT_DMAIC_5 = 5,
    IPU_INT_DMAIC_6 = 6,
    IPU_INT_DMAIC_7 = 7,
    IPU_INT_DMAIC_8 = 8,
    IPU_INT_DMAIC_9 = 9,
    IPU_INT_DMAIC_10 = 10,
    IPU_INT_DMAIC_11 = 11,
    IPU_INT_DMAIC_12 = 12,
    IPU_INT_DMAIC_13 = 13,
    IPU_INT_DMASDC_0 = 14,
    IPU_INT_DMASDC_1 = 15,
    IPU_INT_DMASDC_2 = 16,
    IPU_INT_DMASDC_3 = 17,
    IPU_INT_DMAADC_2 = 18,
    IPU_INT_DMAADC_3 = 19,
    IPU_INT_DMAADC_4 = 20,
    IPU_INT_DMAADC_5 = 21,
    IPU_INT_DMAADC_6 = 22,
    IPU_INT_DMAADC_7 = 23,
    IPU_INT_DMAPF_0 = 24,
    IPU_INT_DMAPF_1 = 25,
    IPU_INT_DMAPF_2 = 26,
    IPU_INT_DMAPF_3 = 27,
    IPU_INT_DMAPF_4 = 28,
    IPU_INT_DMAPF_5 = 29,
    IPU_INT_DMAPF_6 = 30,
    IPU_INT_DMAPF_7 = 31,
    IPU_INT_MAX_ID
} IPU_INT_ID;

typedef enum IPU_DRIVER_ENUM
{
    IPU_DRIVER_PP,     // Post-Processor driver
    IPU_DRIVER_PRP,    // Pre-Processor driver
    IPU_DRIVER_ADC,    // ADC driver
    IPU_DRIVER_SDC,    // SDC driver
} IPU_DRIVER;

typedef enum IC_CHANNEL_ENUM
{
    IC_CHANNEL_ENC,     // IC encoding channel
    IC_CHANNEL_VF       // IC viewfinding channel
} IC_CHANNEL;

//  RGB Formats
// Additional RGB formats has to be addede here
typedef enum {
  IPU_PIX_FMT_RGB666,
  IPU_PIX_FMT_RGB565,
  IPU_PIX_FMT_RGB24,
  IPU_PIX_FMT_YUV422,
}IPU_PIXEL_FORMAT;

// Bitfield of SDC Display Interface signal polarities.
typedef struct {
    UINT32 DATAMASK_EN:1;
    UINT32 CLKIDLE_EN :1;
    UINT32 CLKSEL_EN  :1;
    UINT32 VSYNC_POL  :1;
    UINT32 ENABLE_POL :1;
    UINT32 DATA_POL   :1;       // true = inverted
    UINT32 CLK_POL    :1;       // true = rising edge
    UINT32 HSYNC_POL  :1;       // true = active high
    UINT32 Dummy      :24;      // Dummy variable for alignment.
} SDC_IPU_DI_SIGNAL_CFG;

// Bitfield of ADC Display Interface signal polarities.
typedef struct {
    UINT32 DISP_NUM  :2;
    UINT32 DISP_IF_MODE:2;
    UINT32 DISP_PAR_BURST_MODE:2;
    UINT32 DATA_POL :1;            // true = inverted
    UINT32 CS_POL  :1;                // true = active high
    UINT32 PAR_RS_POL   :1;       // true = inverse
    UINT32 WR_POL    :1;             // true = active high
    UINT32 RD_POL  :1;               // true = active high
    UINT32 VSYNC_POL  :1;          // true = active high
    UINT32 SD_D_POL :1;            // true = inverse
    UINT32 SD_CLK_POL :1;         // true = inverse
    UINT32 SER_RS_POL :1;         // true = inverse
    UINT32 BCLK_POL :1;             // true = inverted
    UINT32 Dummy      :16;          // Dummy variable for alignment.
} ADC_IPU_DI_SIGNAL_CFG;

//   Enumeration of Synchronous (Memory-less) panel types
//   add name of a new panel in the following format
//   IPU_PANEL_<Name>_TFT

typedef enum {
    IPU_PANEL_SHARP_TFT,        // Registry value is 0
    IPU_PANEL_NEC_TFT,          // Registry value is 1
    IPU_TV_NTSC,                // Registry value is 2
    IPU_TV_PAL,                 // Registry value is 3
    ADCPanelOffset,
    IPU_PANEL_TOSHIBA,          // Registry value is 5
    IPU_PANEL_EPSON,            // Registry value is 6
    // New panel goes here ,    // Registry value is 4
    // New panel goes here ,    // Registry value is 5
    numPanel,
} IPU_PANEL_TYPE;


/*
* Panel Information can be filled as a struct variable to extend the driver
*/

struct PANEL_INFO_ST {
    PUCHAR NAME;
    IPU_PANEL_TYPE TYPE;
    IPU_PIXEL_FORMAT PIXEL_FMT;
    INT MODEID;
    INT WIDTH;
    INT HEIGHT;
    INT FREQUENCY;
    INT VSYNCWIDTH;
    INT VSTARTWIDTH;
    INT VENDWIDTH;
    INT HSYNCWIDTH;
    INT HSTARTWIDTH;
    INT HENDWIDTH;
    INT RD_CYCLE_PER; // in ns
    INT RD_UP_POS; // in ns
    INT RD_DOWN_POS; // in ns
    INT WR_CYCLE_PER; // in ns
    INT WR_UP_POS; // in ns
    INT WR_DOWN_POS; // in ns
    INT PIX_CLK_FREQ; // in Hz
    INT PIX_DATA_POS; // in ns
    ADC_IPU_DI_SIGNAL_CFG ADC_SIG_POL;
    SDC_IPU_DI_SIGNAL_CFG SDC_SIG_POL;
};

typedef struct PANEL_INFO_ST PANEL_INFO;

//------------------------------------------------------------------------------
// Functions

#ifdef __cplusplus
}
#endif

#endif //__IPU_H__
