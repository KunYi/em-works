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
//  Copyright (C) 2004, MOTOROLA, INC. All Rights Reserved
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header:  mx27_lcdc.h
//
//  Provides definitions for LCDC module based on i.MX27.
//
//------------------------------------------------------------------------------
#ifndef __MX27_LCDC_H
#define __MX27_LCDC_H

#if __cplusplus
extern "C" {
#endif


//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------
#define LCDC_PCD_VALUE(refclk, pixelclk)    \
    ( ((refclk) % (pixelclk) == 0)? (((refclk) / (pixelclk)) - 1) : ((refclk) / (pixelclk)) )
#define LCDC_PIXEL_SIZE_BYTES(bpp)    \
    ( ((bpp) < 8)? 1 : ( ((bpp) > 16)? 4 : ( (bpp) >> 3 ) ))
#define LCDC_GW_SIZE_X(x)               (x >> 4)
#define LCDC_GW_SIZE_STRIDE(x)          (x >> 2)
#define LCDC_GW_TRANSPARENCY(x)         (0xFF - x)

//This structure defines panel information.
struct DisplayPanel {
    UINT32 width;
    UINT32 height;
    UINT32 hsync_width;
    UINT32 hwait1;
    UINT32 hwait2;
    UINT32 vsync_width;
    UINT32 vwait1;
    UINT32 vwait2;
};

// LCDC Content Struct
typedef struct {
  VOID* pGPEModeInfo;
  UINT16  VideoFrameWidth;
  UINT16  VideoFrameHeight;
  UINT32  VideoMemorySize;
  ULONG VideoMemoryPhyAdd;  
} LCDC_CTX, *PLCDCCTX;

typedef struct lcdcGraphicWindowOP{
  UINT8  Result;

  // Buffer of Graphic Window Description
  HANDLE BufHandle;
  UINT32* BufPhysicalAddr;

  // Graphic Window Description, size and position
  UINT16 Width;
  UINT16 Height;
  UINT16 LineStride;
  UINT16 XOffset;
  UINT16 YOffset;
  UINT8  Transparency;  // 255 indicate totally transparent, ie. not displayed on LCD screen
                        // 0 indicate totally opaque, ie. overlay on LCD screen

  BOOL isFlipWindow;
}lcdcGraphicWindowOP_t, *pLcdcGraphicWindowOP_t;

//Escape codes
#define LCDC_ESC_REQUEST_WINDOW      0x10001 // Request hardware resources
#define LCDC_ESC_RELEASE_WINDOW      0x10002 // Release hardware resources
#define LCDC_ESC_ENABLE_WINDOW       0x10003 // Configure and enable graphic window
#define LCDC_ESC_DISABLE_WINDOW      0x10004 // Disable graphic window
#define LCDC_ESC_FLIP_WINDOW         0x10005 // Vertical flip graphic window
#define LCDC_ESC_GET_TRANSPARENCY    0x10006 // Get current transparency
#define LCDC_ESC_SET_TRANSPARENCY    0x10007 // Set transparency of the window

//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------
typedef struct {
    REG32 SSAR;
    REG32 SR;
    REG32 VPWR;
    REG32 CPR;
    REG32 CWHBR;
    REG32 CCMR;
    REG32 PCR;
    REG32 HCR;
    REG32 VCR;
    REG32 POR;
    REG32 SCR;
    REG32 PCCR;
    REG32 DCR;
    REG32 RMCR;
    REG32 ICR;
    REG32 IER;
    REG32 ISR;
    REG32 _pad1[3];
    REG32 GWSAR;
    REG32 GWSR;
    REG32 GWVPWR;
    REG32 GWPOR;
    REG32 GWPR;
    REG32 GWCR;
    REG32 GWDCR;
    REG32 _pad2[5];
    REG32 AUSCR;
    REG32 AUSCCR;
} CSP_LCDC_REGS, *PCSP_LCDC_REGS;


//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
#define LCDC_SSAR_OFFSET        0x0000
#define LCDC_SR_OFFSET          0x0004
#define LCDC_VPWR_OFFSET        0x0008
#define LCDC_CPR_OFFSET         0x000C
#define LCDC_CWHBR_OFFSET       0x0010
#define LCDC_CCMR_OFFSET        0x0014
#define LCDC_PCR_OFFSET         0x0018
#define LCDC_HCR_OFFSET         0x001C
#define LCDC_VCR_OFFSET         0x0020
#define LCDC_POR_OFFSET         0x0024
#define LCDC_SCR_OFFSET         0x0028
#define LCDC_PCCR_OFFSET        0x002C
#define LCDC_DCR_OFFSET         0x0030
#define LCDC_RMCR_OFFSET        0x0034
#define LCDC_ICR_OFFSET         0x0038
#define LCDC_IER_OFFSET         0x003C
#define LCDC_ISR_OFFSET         0x0040
#define LCDC_GWSAR_OFFSET       0x0050
#define LCDC_GWSR_OFFSET        0x0054
#define LCDC_GWVPWR_OFFSET      0x0058
#define LCDC_GWPOR_OFFSET       0x005C
#define LCDC_GWPR_OFFSET        0x0060
#define LCDC_GWCR_OFFSET        0x0064
#define LCDC_GWDCR_OFFSET       0x0068
#define LCDC_AUSCR_OFFSET       0x0080
#define LCDC_AUSCCR_OFFSET      0x0084

//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
// SSAR : Screen Start Address Register
#define LCDC_SSAR_SSA_LSH                   0

// SR : Size Register
#define LCDC_SR_YMAX_LSH                    0
#define LCDC_SR_XMAX_LSH                    20
#define LCDC_SR_BUSSIZE_LSH                 28

// VPWR : Virtual Page Width Register
#define LCDC_VPWR_VPW_LSH                   0


// CPR : LCD Cursor Position Register
#define LCDC_CPR_CYP_LSH                    0
#define LCDC_CPR_CXP_LSH                    16
#define LCDC_CPR_OP_LSH                     28
#define LCDC_CPR_CC_LSH                     30

// CWHBR : LCD Cursor Width Height and Blink Register
#define LCDC_CWHBR_BD_LSH                   0
#define LCDC_CWHBR_CH_LSH                   16
#define LCDC_CWHBR_CW_LSH                   24
#define LCDC_CWHBR_BK_EN_LSH                31

// CCMR : LCD Color Cursor Mapping Register
#define LCDC_CCMR_CUR_COL_B_LSH             0
#define LCDC_CCMR_CUR_COL_G_LSH             6
#define LCDC_CCMR_CUR_COL_R_LSH             12

// PCR : Panel Configuration Register
#define LCDC_PCR_PCD_LSH                    0
#define LCDC_PCR_SHARP_LSH                  6
#define LCDC_PCR_SCLKSEL_LSH                7
#define LCDC_PCR_ACD_LSH                    8
#define LCDC_PCR_ACDSEL_LSH                 15
#define LCDC_PCR_REV_VS_LSH                 16
#define LCDC_PCR_SWAP_SEL_LSH               17
#define LCDC_PCR_END_SEL_LSH                18
#define LCDC_PCR_SCLKIDLE_LSH               19
#define LCDC_PCR_OEPOL_LSH                  20
#define LCDC_PCR_CLKPOL_LSH                 21
#define LCDC_PCR_LPPOL_LSH                  22
#define LCDC_PCR_FLMPOL_LSH                 23
#define LCDC_PCR_PIXPOL_LSH                 24
#define LCDC_PCR_BPIX_LSH                   25
#define LCDC_PCR_PBSIZ_LSH                  28
#define LCDC_PCR_COLOR_LSH                  30
#define LCDC_PCR_TFT_LSH                    31

// HCR : Horizontal Configuration Register
#define LCDC_HCR_H_WAIT_2_LSH               0
#define LCDC_HCR_H_WAIT_1_LSH               8
#define LCDC_HCR_H_WIDTH_LSH                26

// VCR : Vertical Configuration Register
#define LCDC_VCR_V_WAIT_2_LSH               0
#define LCDC_VCR_V_WAIT_1_LSH               8
#define LCDC_VCR_V_WIDTH_LSH                26

// POR : Panning Offset Register
#define LCDC_POR_POR_LSH                    0

// SCR : Sharp Configuration Register
#define LCDC_SCR_GRAY1_LSH                  0
#define LCDC_SCR_GRAY2_LSH                  4
#define LCDC_SCR_REV_TOGGLE_DELAY_LSH       8
#define LCDC_SCR_CLS_RISE_DELAY_LSH         16
#define LCDC_SCR_PS_RISE_DELAY_LSH          26

// PCCR : PWM Contrast Control Register
#define LCDC_PCCR_PW_LSH                    0
#define LCDC_PCCR_CC_EN_LSH                 8
#define LCDC_PCCR_SCR_LSH                   9
#define LCDC_PCCR_LDMSK_LSH                 15
#define LCDC_PCCR_CLS_HI_WIDTH_LSH          16

// RMCR : Refresh Mode Control Register
#define LCDC_RMCR_SELF_REF_LSH              0

// DCR : DMA Control Register
#define LCDC_DCR_TM_LSH                     0
#define LCDC_DCR_HM_LSH                     16
#define LCDC_DCR_BURST_LSH                  31

// ICR : Interrupt Configuration Register
#define LCDC_ICR_INTCON_LSH                 0
#define LCDC_ICR_INTSYN_LSH                 2
#define LCDC_ICR_GW_INT_CON_LSH             4

// IER: Interrupt Enable Register
#define LCDC_IER_BOF_EN_LSH                 0
#define LCDC_IER_EOF_EN_LSH                 1
#define LCDC_IER_ERR_RES_EN_LSH             2
#define LCDC_IER_UDR_ERR_EN_LSH             3
#define LCDC_IER_GW_BOF_EN_LSH              4
#define LCDC_IER_GW_EOF_EN_LSH              5
#define LCDC_IER_GW_ERR_RES_EN_LSH          6
#define LCDC_IER_GW_UDR_ERR_EN_LSH          7

// ISR : Interrupt Status Register
#define LCDC_ISR_BOF_LSH                    0
#define LCDC_ISR_EOF_LSH                    1
#define LCDC_ISR_ERR_RES_LSH                2
#define LCDC_ISR_UDR_ERR_LSH                3
#define LCDC_ISR_GW_BOF_LSH                 4
#define LCDC_ISR_GW_EOF_LSH                 5
#define LCDC_ISR_GW_ERR_RES_LSH             6
#define LCDC_ISR_GW_UDR_ERR_LSH             7

// GWSAR : Graphic window Start Address Register
#define LCDC_GWSAR_GWSA_LSH                 0

// GWSR : Graphic Window Size Register
#define LCDC_GWSR_GWH_LSH                   0
#define LCDC_GWSR_GWW_LSH                   20

// GWVPWR : Graphic Windos Virtual Page Width Register
#define LCDC_GWVPWR_GWVPW_LSH               0

// GWPOR : Graphic Window Panning Offset Register
#define LCDC_GWPOR_WPOR_LSH                 0

// GWPR : Graphic Window Position Register
#define LCDC_GWPR_GWYP_LSH                  0
#define LCDC_GWPR_GWXP_LSH                  16

// GWCR : Graphic Window Control Register
#define LCDC_GWCR_GWCKB_LSH                 0
#define LCDC_GWCR_GWCKG_LSH                 6
#define LCDC_GWCR_GWCKR_LSH                 12
#define LCDC_GWCR_GW_RVS_LSH                21
#define LCDC_GWCR_GWE_LSH                   22
#define LCDC_GWCR_GWCKE_LSH                 23
#define LCDC_GWCR_GWAV_LSH                  24

// GWDCR : Graphic Window DMA Control Register
#define LCDC_GWDCR_GWTM_LSH                 0
#define LCDC_GWDCR_GWHM_LSH                 16
#define LCDC_GWDCR_GWBT_LSH                 31

// AUSCR : AUS Mode Control Register
#define LCDC_AUSCR_AGWCKB_LSH               0
#define LCDC_AUSCR_AGWCKG_LSH               8
#define LCDC_AUSCR_AGWCKR_LSH               16
#define LCDC_AUSCR_AUSMODE_LSH              31

// AUSCCR : AUS Mode Cursor Control Register
#define LCDC_AUSCCR_ACUR_COL_B_LSH          0
#define LCDC_AUSCCR_ACUR_COL_G_LSH          8
#define LCDC_AUSCCR_ACUR_COL_R_LSH          16

//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
// SSAR : Screen Start Address Register
#define LCDC_SSAR_SSA_WID                   30

// SR : Size RegISTER
#define LCDC_SR_YMAX_WID                    10
#define LCDC_SR_XMAX_WID                    6
#define LCDC_SR_BUSSIZE_WID                 1

// VPWR : Virtual Page Width Register
#define LCDC_VPWR_VPW_WID                   10

// CPR : LCD Cursor Position Register
#define LCDC_CPR_CYP_WID                    10
#define LCDC_CPR_CXP_WID                    10
#define LCDC_CPR_OP_WID                     1
#define LCDC_CPR_CC_WID                     2

// CWHBR : LCD Cursor Width Height and Blink Register
#define LCDC_CWHBR_BD_WID                   8
#define LCDC_CWHBR_CH_WID                   5
#define LCDC_CWHBR_CW_WID                   5
#define LCDC_CWHBR_BK_EN_WID                1

// CCMR : LCD Color Cursor Mapping Register
#define LCDC_CCMR_CUR_COL_B_WID             6
#define LCDC_CCMR_CUR_COL_G_WID             6
#define LCDC_CCMR_CUR_COL_R_WID             6

// PCR : Panel Configuration Register
#define LCDC_PCR_PCD_WID                    6
#define LCDC_PCR_SHARP_WID                  1
#define LCDC_PCR_SCLKSEL_WID                1
#define LCDC_PCR_ACD_WID                    7
#define LCDC_PCR_ACDSEL_WID                 1
#define LCDC_PCR_REV_VS_WID                 1
#define LCDC_PCR_SWAP_SEL_WID               1
#define LCDC_PCR_END_SEL_WID                1
#define LCDC_PCR_SCLKIDLE_WID               1
#define LCDC_PCR_OEPOL_WID                  1
#define LCDC_PCR_CLKPOL_WID                 1
#define LCDC_PCR_LPPOL_WID                  1
#define LCDC_PCR_FLMPOL_WID                 1
#define LCDC_PCR_PIXPOL_WID                 1
#define LCDC_PCR_BPIX_WID                   3
#define LCDC_PCR_PBSIZ_WID                  2
#define LCDC_PCR_COLOR_WID                  1
#define LCDC_PCR_TFT_WID                    1

// HCR : Horizontal Configuration Register
#define LCDC_HCR_H_WAIT_2_WID               8
#define LCDC_HCR_H_WAIT_1_WID               8
#define LCDC_HCR_H_WIDTH_WID                6

// VCR : Vertical Configuration Register
#define LCDC_VCR_V_WAIT_2_WID               8
#define LCDC_VCR_V_WAIT_1_WID               8
#define LCDC_VCR_V_WIDTH_WID                6

// POR : Panning Offset Register
#define LCDC_POR_POR_WID                    5

// SCR : Sharp Configuration Register
#define LCDC_SCR_GRAY1_WID                  4
#define LCDC_SCR_GRAY2_WID                  4
#define LCDC_SCR_REV_TOGGLE_DELAY_WID       4
#define LCDC_SCR_CLS_RISE_DELAY_WID         8
#define LCDC_SCR_PS_RISE_DELAY_WID          6

// PCCR : PWM Contrast Control Register
#define LCDC_PCCR_PW_WID                    8
#define LCDC_PCCR_CC_EN_WID                 1
#define LCDC_PCCR_SCR_WID                   2
#define LCDC_PCCR_LDMSK_WID                 1
#define LCDC_PCCR_CLS_HI_WIDTH_WID          9

// RMCR : Refresh Mode Control Register
#define LCDC_RMCR_SELF_REF_WID              1

// DCR : DMA Control Register
#define LCDC_DCR_TM_WID                     4
#define LCDC_DCR_HM_WID                     4
#define LCDC_DCR_BURST_WID                  1

// ICR : Interrupt Configuration Register
#define LCDC_ICR_INTCON_WID                 1
#define LCDC_ICR_INTSYN_WID                 1
#define LCDC_ICR_GW_INT_CON_WID             1

// IER: Interrupt Enable Register
#define LCDC_IER_BOF_EN_WID                 1
#define LCDC_IER_EOF_EN_WID                 1
#define LCDC_IER_ERR_RES_EN_WID             1
#define LCDC_IER_UDR_ERR_EN_WID             1
#define LCDC_IER_GW_BOF_EN_WID              1
#define LCDC_IER_GW_EOF_EN_WID              1
#define LCDC_IER_GW_ERR_RES_EN_WID          1
#define LCDC_IER_GW_UDR_ERR_EN_WID          1

// ISR : Interrupt Status Register
#define LCDC_ISR_BOF_WID                    1
#define LCDC_ISR_EOF_WID                    1
#define LCDC_ISR_ERR_RES_WID                1
#define LCDC_ISR_UDR_ERR_WID                1
#define LCDC_ISR_GW_BOF_WID                 1
#define LCDC_ISR_GW_EOF_WID                 1
#define LCDC_ISR_GW_ERR_RES_WID             1
#define LCDC_ISR_GW_UDR_ERR_WID             1

// GWSAR : Graphic window Start Address Register
#define LCDC_GWSAR_GWSA_WID                 32

// GWSR : Graphic Window Size Register
#define LCDC_GWSR_GWH_WID                   10
#define LCDC_GWSR_GWW_WID                   6

// GWVPWR : Graphic Windos Virtual Page Width Register
#define LCDC_GWVPWR_GWVPW_WID               10

// GWPOR : Graphic Window Panning Offset Register
#define LCDC_GWPOR_WPOR_WID                 5

// GWPR : Graphic Window Position Register
#define LCDC_GWPR_GWYP_WID                  10
#define LCDC_GWPR_GWXP_WID                  10

// GWCR : Graphic Window Control Register
#define LCDC_GWCR_GWCKB_WID                 6
#define LCDC_GWCR_GWCKG_WID                 6
#define LCDC_GWCR_GWCKR_WID                 6
#define LCDC_GWCR_GW_RVS_WID                1
#define LCDC_GWCR_GWE_WID                   1
#define LCDC_GWCR_GWCKE_WID                 1
#define LCDC_GWCR_GWAV_WID                  8

// GWDCR : Graphic Window DMA Control Register
#define LCDC_GWDCR_GWTM_WID                 5
#define LCDC_GWDCR_GWHM_WID                 5
#define LCDC_GWDCR_GWBT_WID                 1

// AUSCR : AUS Mode Control Register
#define LCDC_AUSCR_AGWCKB_WID               8
#define LCDC_AUSCR_AGWCKG_WID               8
#define LCDC_AUSCR_AGWCKR_WID               8
#define LCDC_AUSCR_AUSMODE_WID              1

// AUSCCR : AUS Mode Cursor Control Register
#define LCDC_AUSCCR_ACUR_COL_B_WID          8
#define LCDC_AUSCCR_ACUR_COL_G_WID          8
#define LCDC_AUSCCR_ACUR_COL_R_WID          8

//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------
// SR : Size Register
#define LCDC_SR_BUSSIZE_32BIT               1   // The AHB 32Bit Width
#define LCDC_SR_BUSSIZE_64BIT               0   // The AHB 64Bit Width

// CPR : LCD Cursor Position Register
#define LCDC_CPR_OP_DISABLE                 0   // Arithmetric operation disable
#define LCDC_CPR_OP_ENABLE                  1   // Arithmetric operation enable

#define LCDC_CPR_CC_DISABLED                0   // Cursor disabled
#define LCDC_CPR_CC_OR                      1   // OR between background and cursor
#define LCDC_CPR_CC_EOR                     2   // EOR between background and cursor
#define LCDC_CPR_CC_AND                     3   // And between background and cursor

// CWHBR : LCD Cursor Width Height and Blink Register
#define LCDC_CWHBR_BK_EN_DISABLE            0   // Cursor blink disable
#define LCDC_CWHBR_BK_EN_ENABLE             1   // Cursor blink enable

#define LCDC_CWHBR_CW_CURSOR_DISABLED       0

#define LCDC_CWHBR_CH_CURSOR_DISABLED       0

#define LCDC_CWHBR_BD_MAX_DIV               255

// PCR : Panel Configuration Register
#define LCDC_PCR_SHARP_DISABLE              0   // Disable signal for Sharp HR-TFT panels.
#define LCDC_PCR_SHARP_ENABLE               1   // Enable signal for Sharp HR-TFT panels.

#define LCDC_PCR_SCLKSEL_DISABLE            0   // Disable OE and LSCLK in TFT mode when no data
#define LCDC_PCR_SCLKSEL_ENABLE             1   // Enable OE and LSCLK in TFT mode when no data

#define LCDC_PCR_ACDSEL_USE_FRM             0   // Use FRM as clock source for ACD count
#define LCDC_PCR_ACDSEL_USE_LPHSYNC         1   // Use LP/HSYN as clock source for ACD count

#define LCDC_PCR_REV_VS_NORMAL              0   // Vertical scan in normal direction
#define LCDC_PCR_REV_VS_REVERSE             1   // Vertical scan in reverse direction

#define LCDC_PCR_SWAP_SEL_16BPP             0   // Swap data for 16bpp mode
#define LCDC_PCR_SWAP_SEL_12BPP             0   // Swap data for 12bpp mode
#define LCDC_PCR_SWAP_SEL_8BPP              1   // Swap data for 8bpp mode
#define LCDC_PCR_SWAP_SEL_4BPP              1   // Swap data for 4bpp mode
#define LCDC_PCR_SWAP_SEL_2BPP              1   // Swap data for 2bpp mode
#define LCDC_PCR_SWAP_SEL_1BPP              1   // Swap data for 1bpp mode

#define LCDC_PCR_END_SEL_LITTLE_ENDIAN      0   // Image download into memory as little endian format
#define LCDC_PCR_END_SEL_BIG_ENDIAN         1   // Image download into memory as big endian format

#define LCDC_PCR_SCLKIDLE_DISABLE           0   // Disable LSCLK when VSYNC idle
#define LCDC_PCR_SCLKIDLE_ENABLE            1   // Enable LSCLK when VSYNC idle

#define LCDC_PCR_OEPOL_ACTIVE_HIGH          0   // Output polarity active high
#define LCDC_PCR_OEPOL_ACTIVE_LOW           1   // Output polarity active low

#define LCDC_PCR_CLKPOL_NEG_EDGE            0   // Active negative edge of LSCLK
#define LCDC_PCR_CLKPOL_POS_EDGE            1   // Active positive edge of LSCLK

#define LCDC_PCR_LPPOL_ACTIVE_HIGH          0   // Active high
#define LCDC_PCR_LPPOL_ACTIVE_LOW           1   // Active low

#define LCDC_PCR_FLMPOL_ACTIVE_HIGH         0   // Active high
#define LCDC_PCR_FLMPOL_ACTIVE_LOW          1   // Active low

#define LCDC_PCR_PIXPOL_ACTIVE_HIGH         0   // Active high
#define LCDC_PCR_PIXPOL_ACTIVE_LOW          1   // Active low

// Note: 
// To set normal 18bpp mode:
//  BPIX = 110
//  END_SEL = 0
//  SWAP_SEL = X (don’t care)
// To set Microsoft PAL_BGR 18bpp mode:
//  BPIX = 110
//  END_SEL = 1
//  SWAP_SEL = 1
#define LCDC_PCR_BPIX_1BPP                  0   // 1bpp
#define LCDC_PCR_BPIX_2BPP                  1   // 2bpp
#define LCDC_PCR_BPIX_4BPP                  2   // 4bpp
#define LCDC_PCR_BPIX_8BPP                  3   // 8bpp
#define LCDC_PCR_BPIX_12BPP                 4   // 12bpp
#define LCDC_PCR_BPIX_16BPP                 5   // 16bpp
#define LCDC_PCR_BPIX_18BPP                 6   // 18bpp

#define LCDC_PCR_PBSIZ_1BIT                 0   //  1 bit panel bus width
#define LCDC_PCR_PBSIZ_4BIT                 2   //  4 bit panel bus width
#define LCDC_PCR_PBSIZ_8BIT                 3   //  8 bit panel bus width

#define LCDC_PCR_COLOR_MONOCHROME           0   // Monochrome LCD panel
#define LCDC_PCR_COLOR_COLOR                1   // Color LCD panel

#define LCDC_PCR_TFT_PASSIVE                0   // Passive display
#define LCDC_PCR_TFT_ACTIVE                 1   // Active display

// PCCR : PWM Contrast Control Register
#define LCDC_PCCR_PW_MAX                    255 // PWM max

#define LCDC_PCCR_CC_EN_DISABLE             0   // Contrast control off
#define LCDC_PCCR_CC_EN_ENABLE              1   // Contrast control on

#define LCDC_PCCR_SCR_LINEPULSE             0   // Clock source to PWM counter is line pulse
#define LCDC_PCCR_SCR_PIXELCLK              1   // Clock source to PWM counter is pixel clock
#define LCDC_PCCR_SCR_LCDCLK                2   // Clock source to PWM counter is LCD clock

#define LCDC_PCCR_LDMSK_DISABLE             0   // LD[15:0] is always normal
#define LCDC_PCCR_LDMSK_ENABLE              1   // LD[15:0] is always 0

// RMCR : Refresh Mode Control Register
#define LCDC_RMCR_SELF_REF_DISABLE          0   // Self refresh disable
#define LCDC_RMCR_SELF_REF_ENABLE           1   // Self refresh enable

// DCR : DMA Control Register
#define LCDC_DCR_BURST_DYNAMIC              0   // Burst length is dynamic
#define LCDC_DCR_BURST_FIXED                1   // Burst length is fixed

// ICR : Interrupt Configuration Register
#define LCDC_ICR_INTCON_EOF                 0   // Interrupt flag is set when EOF reached
#define LCDC_ICR_INTCON_BOF                 1   // Interrupt flag is set when BOF reached

#define LCDC_ICR_INTSYN_MEMORY              0   // Interrupt flag is set on loading last/first frame data from memory
#define LCDC_ICR_INTSYN_PANEL               1   // Interrupt flag is set on loading last/first frame data to LCD panel

#define LCDC_ICR_GW_INT_CON_END             0   // Interrupt flag is set when end of graphic window is reached
#define LCDC_ICR_GW_INT_CON_BEG             1   // Interrupt flag is set when beginning of graphic window is reached

// IER: Interrupt Enable Register
#define LCDC_IER_BOF_EN_DISABLE             0   // BOF interrupt disable
#define LCDC_IER_BOF_EN_ENABLE              1   // BOF interrupt enable

#define LCDC_IER_EOF_EN_DISABLE             0   // EOF interrupt disable
#define LCDC_IER_EOF_EN_ENABLE              1   // EOF interrupt enable

#define LCDC_IER_ERR_RES_DISABLE            0   // Error reponse interrupt disable
#define LCDC_IER_ERR_RES_ENABLE             1   // Error reponse interrupt enable

#define LCDC_IER_UDR_ERR_EN_DISABLE         0   // Underrun error reponse interrupt disable
#define LCDC_IER_UDR_ERR_EN_ENABLE          1   // Underrun error reponse interrupt enable

#define LCDC_IER_GW_BOF_EN_DISABLE          0   // Graphic window BOF interrupt disable
#define LCDC_IER_GW_BOF_EN_ENABLE           1   // Graphic window BOF interrupt enable

#define LCDC_IER_GW_EOF_EN_DISABLE          0   // Graphic window EOF interrupt disable
#define LCDC_IER_GW_EOF_EN_ENABLE           1   // Graphic window EOF interrupt enable

#define LCDC_IER_GW_ERR_RES_EN_DISABLE      0   // Graphic window error reponse interrupt disable
#define LCDC_IER_GW_ERR_RES_EN_ENABLE       1   // Graphic window error reponse interrupt enable

#define LCDC_IER_GW_UDR_ERR_EN_DISABLE      0   // Graphic window underrun error reponse interrupt disable
#define LCDC_IER_GW_UDR_ERR_EN_ENABLE       1   // Graphic window underrun error reponse interrupt enable

// ISR : Interrupt Status Register
#define LCDC_ISR_BOF_NO_INTERRUPT           0   // No BOF interrupt
#define LCDC_ISR_BOF_INTERRUPT              1   // BOF interrupt

#define LCDC_ISR_EOF_NO_INTERRUPT           0   // No EOF interrupt
#define LCDC_ISR_EOF_INTERRUPT              1   // EOF interrupt

#define LCDC_ISR_ERR_RES_DISABLE            0   // No error reponse interrupt
#define LCDC_ISR_ERR_RES_ENABLE             1   // Error reponse interrupt

#define LCDC_ISR_UDR_ERR_NO_INTERRUPT       0   // No underrun error reponse interrupt
#define LCDC_ISR_UDR_ERR_INTERRUPT          1   // Underrun error reponse interrupt

#define LCDC_ISR_GW_BOF_NO_INTERRUPT        0   // No graphic window BOF interrupt
#define LCDC_ISR_GW_BOF_INTERRUPT           1   // Graphic window BOF interrupt

#define LCDC_ISR_GW_EOF_NO_INTERRUPT        0   // No graphic window EOF interrupt
#define LCDC_ISR_GW_EOF_INTERRUPT           1   // Graphic window EOF interrupt

#define LCDC_ISR_GW_ERR_RES_NO_INTERRUPT    0   // No graphic window error reponse interrupt
#define LCDC_ISR_GW_ERR_RES_INTERRUPT       1   // Graphic window error reponse interrupt

#define LCDC_ISR_GW_UDR_ERR_NO_INTERRUPT    0   // No graphic window underrun error reponse interrupt
#define LCDC_ISR_GW_UDR_ERR_INTERRUPT       1   // Graphic window underrun error reponse interrupt

// GWCR : Graphic Window Control Register
#define LCDC_GWCR_GW_RVS_NORMAL             0   // Vertical scan in normal direction
#define LCDC_GWCR_GW_RVS_REVERSE            1   // Vertical scan in reverse direction

#define LCDC_GWCR_GWE_DISABLE               0   // Graphic window disable
#define LCDC_GWCR_GWE_ENABLE                1   // Graphic window enable

#define LCDC_GWCR_GWCKE_DISABLE             0   // Graphic window color keying disable
#define LCDC_GWCR_GWCKE_ENABLE              1   // Graphic window color keying enable

#define LCDC_GWCR_GWAV_TRANSPARENT          0   // Graphic window totally transparent
#define LCDC_GWCR_GWAV_OPAQUE               1   // Graphic window totally opaque

// GWDCR : Graphic Window DMA Control Register
#define LCDC_GWDCR_GWBT_DYNAMIC             0   // Burst length is dynamic
#define LCDC_GWDCR_GWBT_FIXED               1   // Burst length is fixed

// AUSCR : AUS Mode Control Register
#define LCDC_AUSCR_AUSMODE_NORMAL           0   // Normal Mode 
#define LCDC_AUSCR_AUSMODE_AUS              1   // AUS Mode

// mode
#ifndef DISPLAY_MODE_SHPQVGA
#define DISPLAY_MODE_SHPQVGA    0
#endif
#ifndef DISPLAY_MODE_NECVGA
#define DISPLAY_MODE_NECVGA     1
#endif
#ifndef DISPLAY_MODE_NTSC
#define DISPLAY_MODE_NTSC       2
#endif
#ifndef DISPLAY_MODE_PAL
#define DISPLAY_MODE_PAL        3
#endif
#ifndef DISPLAY_MODE_NONE
#define DISPLAY_MODE_NONE       4
#endif


#ifdef __cplusplus
}
#endif

#endif // __MX27_LCDC_H
