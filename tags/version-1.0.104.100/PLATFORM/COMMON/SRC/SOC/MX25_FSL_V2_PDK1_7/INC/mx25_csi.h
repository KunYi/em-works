//------------------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
//  Use of this source code is subject to the terms of the Microsoft end-user
//  license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
//  If you did not accept the terms of the EULA, you are not authorized to use
//  this source code. For a copy of the EULA, please see the LICENSE.RTF on your
//  install media.
//
//------------------------------------------------------------------------------
// Copyright (C) 2006, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  Header:  mx25_csi.h
//
//  Provides definitions for CSI module based on i.MX25 processor.
//
//------------------------------------------------------------------------------
#ifndef __MX25_CSI_H
#define __MX25_CSI_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------
typedef struct {
    REG32 CSICR1;
    REG32 CSICR2;
    REG32 CSICR3;
    REG32 CSISTATFIFO;
    REG32 CSIRXFIFO;
    REG32 CSIRXCNT;
    REG32 CSISR;
    REG32 CSIDBG;
    REG32 CSIDMASASTATFIFO;
    REG32 CSIDMATSSTATFIFO;
    REG32 CSIDMASAFB1;
    REG32 CSIDMASAFB2;
    REG32 CSIFBUFPARA;
    REG32 CSIIMAGPARA;
} CSP_CSI_REGS, *PCSP_CSI_REGS;

//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
#define CSI_CSICR1_OFFSET               0x0000
#define CSI_CSICR2_OFFSET               0x0004
#define CSI_CSICR3_OFFSET               0x0008
#define CSI_CSISTATFIFO_OFFSET          0x000C
#define CSI_CSIRXFIFO_OFFSET            0x0010
#define CSI_CSIRXCNT_OFFSET             0x0014
#define CSI_CSISR_OFFSET                0x0018
#define CSI_CSIDBG_OFFSET               0x001C
#define CSI_CSIDMASASTATFIFO_OFFSET     0x0020
#define CSI_CSIDMATSSTATFIFO_OFFSET     0x0024
#define CSI_CSIDMASAFB1_OFFSET          0x0028
#define CSI_CSIDMASAFB2_OFFSET          0x002C
#define CSI_CSIFBUFPARA_OFFSET          0x0030
#define CSI_CSIIMAGPARA_OFFSET          0x0034
        
//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
// CSICR1 : CSI Control Register 1
#define CSI_CSICR1_PIXEL_BIT_LSH             0
#define CSI_CSICR1_REDGE_LSH                 1
#define CSI_CSICR1_INV_PCLK_LSH              2
#define CSI_CSICR1_INV_DATA_LSH              3
#define CSI_CSICR1_GCLK_MODE_LSH             4
#define CSI_CSICR1_CLR_RXFIFO_LSH            5
#define CSI_CSICR1_CLR_STATFIFO_LSH          6
#define CSI_CSICR1_PACK_DIR_LSH              7
#define CSI_CSICR1_FCC_LSH                   8
#define CSI_CSICR1_MCLKEN_LSH                9
#define CSI_CSICR1_CCIR_EN_LSH              10
#define CSI_CSICR1_HSYNC_POL_LSH            11
#define CSI_CSICR1_MCLKDIV_LSH              12
#define CSI_CSICR1_SOF_INTEN_LSH            16
#define CSI_CSICR1_SOF_POL_LSH              17
#define CSI_CSICR1_RXFF_INTEN_LSH           18
#define CSI_CSICR1_FB1_DMA_DONE_INTEN_LSH   19
#define CSI_CSICR1_FB2_DMA_DONE_INTEN_LSH   20
#define CSI_CSICR1_STAFF_INTEN_LSH          21
#define CSI_CSICR1_SFF_DMA_DONE_INTEN_LSH   22
#define CSI_CSICR1_STATFF_INTEN_LSH         23
#define CSI_CSICR1_RF_OR_INTEN_LSH          24
#define CSI_CSICR1_SF_OR_INTEN_LSH          25
#define CSI_CSICR1_COF_INT_EN_LSH           26
#define CSI_CSICR1_CCIR_MODE_LSH            27
#define CSI_CSICR1_PRP_IF_EN_LSH            28
#define CSI_CSICR1_EOF_INT_EN_LSH           29
#define CSI_CSICR1_EXT_VSYNC_LSH            30
#define CSI_CSICR1_SWAP16_EN_LSH            31

//CSICR2 : CSI Control Register 2
#define CSI_CSICR2_HSC_LSH                   0
#define CSI_CSICR2_VSC_LSH                   8
#define CSI_CSICR2_LVRM_LSH                 16
#define CSI_CSICR2_BTS_LSH                  19
#define CSI_CSICR2_SCE_LSH                  23
#define CSI_CSICR2_AFS_LSH                  24
#define CSI_CSICR2_DRM_LSH                  26
#define CSI_CSICR2_DMA_BURST_TYPE_SFF_LSH   28
#define CSI_CSICR2_DMA_BURST_TYPE_RFF_LSH   30

//CSICR3 : CSI Control Register 3
#define CSI_CSICR3_ECC_AUTO_EN_LSH           0
#define CSI_CSICR3_ECC_INT_EN_LSH            1
#define CSI_CSICR3_ZERO_PACK_EN_LSH          2
#define CSI_CSICR3_TWO_8BIT_SENSOR_LSH       3
#define CSI_CSICR3_RXFF_LEVEL_LSH            4  
#define CSI_CSICR3_HRESP_ERR_EN_LSH          7
#define CSI_CSICR3_STATFF_LEVEL_LSH          8
#define CSI_CSICR3_DMA_REQ_EN_SFF_LSH       11          
#define CSI_CSICR3_DMA_REQ_EN_RFF_LSH       12
#define CSI_CSICR3_DMA_REFLASH_SFF_LSH      13
#define CSI_CSICR3_DMA_REFLASH_RFF_LSH      14
#define CSI_CSICR3_FRMCNT_RST_LSH           15
#define CSI_CSICR3_FRMCNT_LSH               16

//CSISTATFIFO : CSI Statistic FIFO Register
#define CSI_CSISTATFIFO_STAT_LSH             0

//CSIRXFIFO : CSI RxFIFO Register
#define CSI_CSIRXFIFO_IMAGE_LSH              0

//CSIRXCNT :  CSI RxFIFO Count Register
#define CSI_CSIRXCNT_RXCNT_LSH               0

//CSISR :  CSI Status Register
#define CSI_CSISR_DRDY_LSH                   0
#define CSI_CSISR_ECC_INT_LSH                1
#define CSI_CSISR_HRESP_ERR_INT_LSH          7
#define CSI_CSISR_COF_INT_LSH               13
#define CSI_CSISR_F1_INT_LSH                14
#define CSI_CSISR_F2_INT_LSH                15
#define CSI_CSISR_SOF_INT_LSH               16
#define CSI_CSISR_EOF_INT_LSH               17
#define CSI_CSISR_RXFF_INT_LSH              18
#define CSI_CSISR_DMA_TSF_DONE_FB1_LSH      19
#define CSI_CSISR_DMA_TSF_DONE_FB2_LSH      20
#define CSI_CSISR_STAFF_INT_LSH             21
#define CSI_CSISR_DMA_TSF_DONE_SFF_LSH      22
#define CSI_CSISR_RFF_OR_INT_LSH            24
#define CSI_CSISR_SFF_OR_INT_LSH            25

//CSIDBGREG : CSI Debug Register
#define CSI_CSIDBG_DEBUG_EN_LSH              0

//CSIDMASASTATFIFO : CSI DMA Start Address Register - For STATFIFO
#define CSI_CSIDMASASTATFIFO_DMA_START_ADDR_SFF_LSH     0

//CSIDMATSSTATFIFO : CSI DMA Transfer Size Register - For STATFIFO
#define CSI_CSIDMATSSTATFIFO_DMA_TSF_SIZE_SFF_LSH       0

//CSIDMASAFB1 : CSI DMA Start Address Register - For Frame Buffer 1
#define CSI_CSIDMASAFB1_DMA_START_ADDR_FB1_LSH          0

//CSIDMASAFB2 : CSI DMA Transfer Size Register - For Frame Buffer 2
#define CSI_CSIDMASAFB2_DMA_START_ADDR_FB2_LSH          0

//CSIFBUFPARA : CSI Frame Buffer Parameter Register
#define CSI_CSIFBUFPARA_FBUF_STRIDE_LSH     0

//CSIIMAGPARA : CSI Image Parameter Register
#define CSI_CSIIMAGPARA_IMAGE_HEIGHT_LSH    0
#define CSI_CSIIMAGPARA_IMAGE_WIDTH_LSH    16

//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
// CSICR1 : CSI Control Register 1
#define CSI_CSICR1_PIXEL_BIT_WID             1
#define CSI_CSICR1_REDGE_WID                 1
#define CSI_CSICR1_INV_PCLK_WID              1
#define CSI_CSICR1_INV_DATA_WID              1
#define CSI_CSICR1_GCLK_MODE_WID             1
#define CSI_CSICR1_CLR_RXFIFO_WID            1
#define CSI_CSICR1_CLR_STATFIFO_WID          1
#define CSI_CSICR1_PACK_DIR_WID              1
#define CSI_CSICR1_FCC_WID                   1
#define CSI_CSICR1_MCLKEN_WID                1
#define CSI_CSICR1_CCIR_EN_WID               1
#define CSI_CSICR1_HSYNC_POL_WID             1
#define CSI_CSICR1_MCLKDIV_WID               4
#define CSI_CSICR1_SOF_INTEN_WID             1
#define CSI_CSICR1_SOF_POL_WID               1
#define CSI_CSICR1_RXFF_INTEN_WID            1
#define CSI_CSICR1_FB1_DMA_DONE_INTEN_WID    1
#define CSI_CSICR1_FB2_DMA_DONE_INTEN_WID    1
#define CSI_CSICR1_STAFF_INTEN_WID           1
#define CSI_CSICR1_SFF_DMA_DONE_INTEN_WID    1
#define CSI_CSICR1_STATFF_INTEN_WID          1
#define CSI_CSICR1_RF_OR_INTEN_WID           1
#define CSI_CSICR1_SF_OR_INTEN_WID           1
#define CSI_CSICR1_COF_INT_EN_WID            1
#define CSI_CSICR1_CCIR_MODE_WID             1
#define CSI_CSICR1_PRP_IF_EN_WID             1
#define CSI_CSICR1_EOF_INT_EN_WID            1
#define CSI_CSICR1_EXT_VSYNC_WID             1
#define CSI_CSICR1_SWAP16_EN_WID             1

//CSICR2 : CSI Control Register 2
#define CSI_CSICR2_HSC_WID                   8
#define CSI_CSICR2_VSC_WID                   8
#define CSI_CSICR2_LVRM_WID                  3
#define CSI_CSICR2_BTS_WID                   2
#define CSI_CSICR2_SCE_WID                   1
#define CSI_CSICR2_AFS_WID                   2
#define CSI_CSICR2_DRM_WID                   1
#define CSI_CSICR2_DMA_BURST_TYPE_SFF_WID    2
#define CSI_CSICR2_DMA_BURST_TYPE_RFF_WID    2

//CSICR3 : CSI Control Register 3
#define CSI_CSICR3_ECC_AUTO_EN_WID           1
#define CSI_CSICR3_ECC_INT_EN_WID            1
#define CSI_CSICR3_ZERO_PACK_EN_WID          1
#define CSI_CSICR3_TWO_8BIT_SENSOR_WID       1
#define CSI_CSICR3_RXFF_LEVEL_WID            3  
#define CSI_CSICR3_HRESP_ERR_EN_WID          1
#define CSI_CSICR3_STATFF_LEVEL_WID          3
#define CSI_CSICR3_DMA_REQ_EN_SFF_WID        1          
#define CSI_CSICR3_DMA_REQ_EN_RFF_WID        1
#define CSI_CSICR3_DMA_REFLASH_SFF_WID       1
#define CSI_CSICR3_DMA_REFLASH_RFF_WID       1
#define CSI_CSICR3_FRMCNT_RST_WID            1
#define CSI_CSICR3_FRMCNT_WID               16

//CSISTATFIFO : CSI Statistic FIFO Register
#define CSI_CSISTATFIFO_STAT_WID            32

//CSIRXFIFO : CSI RxFIFO Register
#define CSI_CSIRXFIFO_IMAGE_WID             32

//CSIRXCNT :  CSI RxFIFO Count Register
#define CSI_CSIRXCNT_RXCNT_WID              22

//CSISR :  CSI Status Register
#define CSI_CSISR_DRDY_WID                   1
#define CSI_CSISR_ECC_INT_WID                1
#define CSI_CSISR_HRESP_ERR_INT_WID          1
#define CSI_CSISR_COF_INT_WID                1
#define CSI_CSISR_F1_INT_WID                 1
#define CSI_CSISR_F2_INT_WID                 1
#define CSI_CSISR_SOF_INT_WID                1
#define CSI_CSISR_EOF_INT_WID                1
#define CSI_CSISR_RXFF_INT_WID               1
#define CSI_CSISR_DMA_TSF_DONE_FB1_WID       1
#define CSI_CSISR_DMA_TSF_DONE_FB2_WID       1
#define CSI_CSISR_STAFF_INT_WID              1
#define CSI_CSISR_DMA_TSF_DONE_SFF_WID       1
#define CSI_CSISR_RFF_OR_INT_WID             1
#define CSI_CSISR_SFF_OR_INT_WID             1

//CSIDBGREG : CSI Debug Register
#define CSI_CSIDBG_DEBUG_EN_WID              1

//CSIDMASASTATFIFO : CSI DMA Start Address Register - For STATFIFO
#define CSI_CSIDMASASTATFIFO_DMA_START_ADDR_SFF_WID     32

//CSIDMATSSTATFIFO : CSI DMA Transfer Size Register - For STATFIFO
#define CSI_CSIDMATSSTATFIFO_DMA_TSF_SIZE_SFF_WID       32

//CSIDMASAFB1 : CSI DMA Start Address Register - For Frame Buffer 1
#define CSI_CSIDMASAFB1_DMA_START_ADDR_FB1_WID          32

//CSIDMASAFB2 : CSI DMA Transfer Size Register - For Frame Buffer 2
#define CSI_CSIDMASAFB2_DMA_START_ADDR_FB2_WID          32

//CSIFBUFPARA : CSI Frame Buffer Parameter Register
#define CSI_CSIFBUFPARA_FBUF_STRIDE_WID     16

//CSIIMAGPARA : CSI Image Parameter Register
#define CSI_CSIIMAGPARA_IMAGE_HEIGHT_WID   16
#define CSI_CSIIMAGPARA_IMAGE_WIDTH_WID    16

//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------
// CSICR1 : CSI Control Register 1
#define CSI_CSICR1_PIXEL_BIT_8BIT   0
#define CSI_CSICR1_PIXEL_BIT_10BIT  1

#define CSI_CSICR1_REDGE_RISING     1
#define CSI_CSICR1_REDGE_FALLING    0

#define CSI_CSICR1_INV_PCLK_DIRECT  0
#define CSI_CSICR1_INV_PCLK_INVERT  1

#define CSI_CSICR1_INV_DATA_DIRECT  0
#define CSI_CSICR1_INV_DATA_INVERT  1

#define CSI_CSICR1_GCLK_MODE_NONGATED   0
#define CSI_CSICR1_GCLK_MODE_GATED  1

#define CSI_CSICR1_CLR_RXFIFO_CLEAR 1

#define CSI_CSICR1_CLR_STATFIFO_CLEAR   1

#define CSI_CSICR1_PACK_DIR_LSB     0
#define CSI_CSICR1_PACK_DIR_MSB     1

#define CSI_CSICR1_FCC_ASYNC        0
#define CSI_CSICR1_FCC_SYNC     1

#define CSI_CSICR1_MCLKEN_DISABLE   0
#define CSI_CSICR1_MCLKEN_ENABLE    1

#define CSI_CSICR1_CCIR_EN_TRADITIONAL  0
#define CSI_CSICR1_CCIR_EN_CCIR     1

#define CSI_CSICR1_HSYNC_POL_LOW    0
#define CSI_CSICR1_HSYNC_POL_HIGH   1

#define CSI_CSICR1_MCLKDIV_VALUE(X)     ((X)/2)

#define CSI_CSICR1_SOF_INTEN_DISABLE    0
#define CSI_CSICR1_SOF_INTEN_ENABLE 1

#define CSI_CSICR1_SOF_POL_FALLING  0
#define CSI_CSICR1_SOF_POL_RISING   1

#define CSI_CSICR1_RXFF_INTEN_DISABLE   0
#define CSI_CSICR1_RXFF_INTEN_ENABLE    1

#define CSI_CSICR1_FB1_DMA_DONE_INTEN_DISABLE   0
#define CSI_CSICR1_FB1_DMA_DONE_INTEN_ENABLE    1

#define CSI_CSICR1_FB2_DMA_DONE_INTEN_DISABLE   0
#define CSI_CSICR1_FB2_DMA_DONE_INTEN_ENABLE    1

#define CSI_CSICR1_STATFF_INTEN_DISABLE 0
#define CSI_CSICR1_STATFF_INTEN_ENABLE  1

#define CSI_CSICR1_SFF_DMA_DONE_INTEN_DISABLE   0
#define CSI_CSICR1_SFF_DMA_DONE_INTEN_ENABLE    1

#define CSI_CSICR1_RF_OR_INTEN_DISABLE  0
#define CSI_CSICR1_RF_OR_INTEN_ENABLE   1

#define CSI_CSICR1_SF_OR_INTEN_DISABLE  0
#define CSI_CSICR1_SF_OR_INTEN_ENABLE   1

#define CSI_CSICR1_COF_INT_EN_DISABLE   0
#define CSI_CSICR1_COF_INT_EN_ENABLE    1

#define CSI_CSICR1_CCIR_MODE_PROGRESSIVE 0
#define CSI_CSICR1_CCIR_MODE_INTERLACE   1

#define CSI_CSICR1_PRP_IF_EN_DISABLE    0
#define CSI_CSICR1_PRP_IF_EN_ENABLE     1

#define CSI_CSICR1_EOF_INT_EN_DISABLE   0
#define CSI_CSICR1_EOF_INT_EN_ENABLE    1

#define CSI_CSICR1_EXT_VSYNC_INTERNAL   0
#define CSI_CSICR1_EXT_VSYNC_EXTERNAL   1

#define CSI_CSICR1_SWAP16_EN_DISABLE    0
#define CSI_CSICR1_SWAP16_EN_ENABLE     1

//CSICR2 : CSI Control Register 2
#define CSI_CSICR2_HSC_VALUE(X)     (X)
#define CSI_CSICR2_VSC_VALUE(X)     (X)
#define CSI_CSICR2_LVRM_512X384     0
#define CSI_CSICR2_LVRM_448X336     1
#define CSI_CSICR2_LVRM_384X288     2
#define CSI_CSICR2_LVRM_384X256     3
#define CSI_CSICR2_LVRM_320X240     4
#define CSI_CSICR2_LVRM_288X216     5
#define CSI_CSICR2_LVRM_400X300     6

#define CSI_CSICR2_BTS_GR       0
#define CSI_CSICR2_BTS_RG       1
#define CSI_CSICR2_BTS_BG       2
#define CSI_CSICR2_BTS_GB       3

#define CSI_CSICR2_SCE_DISABLE      0
#define CSI_CSICR2_SCE_ENABLE       1

#define CSI_CSICR2_AFS_CONSECUTIVE  0
#define CSI_CSICR2_AFS_EVERYOTHER   1
#define CSI_CSICR2_AFS_EVERYFOUR    2 

#define CSI_CSICR2_DRM_8X6      0
#define CSI_CSICR2_DRM_8X12     1

#define CSI_CSICR2_DMA_BURST_TYPE_SFF_INCR8     0
#define CSI_CSICR2_DMA_BURST_TYPE_SFF_INCR4     1
#define CSI_CSICR2_DMA_BURST_TYPE_SFF_INCR16    3

#define CSI_CSICR2_DMA_BURST_TYPE_RFF_INCR8     0
#define CSI_CSICR2_DMA_BURST_TYPE_RFF_INCR4     1
#define CSI_CSICR2_DMA_BURST_TYPE_RFF_INCR16    3

//CSICR3 : CSI Control Register 3
#define CSI_CSICR3_ECC_AUTO_EN_DISABLE      0
#define CSI_CSICR3_ECC_AUTO_EN_ENABLE       1

#define CSI_CSICR3_ECC_INT_EN_DISABLE       0
#define CSI_CSICR3_ECC_INT_EN_ENABLE        1

#define CSI_CSICR3_ZERO_PACK_EN_DISABLE     0
#define CSI_CSICR3_ZERO_PACK_EN_ENABLE      1

#define CSI_CSICR3_TWO_8BIT_SENSOR_ONE      0
#define CSI_CSICR3_TWO_8BIT_SENSOR_TWO      1

#define CSI_CSICR3_RXFF_LEVEL_4WORDS        0
#define CSI_CSICR3_RXFF_LEVEL_8WORDS        1
#define CSI_CSICR3_RXFF_LEVEL_16WORDS       2
#define CSI_CSICR3_RXFF_LEVEL_24WORDS       3
#define CSI_CSICR3_RXFF_LEVEL_32WORDS       4
#define CSI_CSICR3_RXFF_LEVEL_48WORDS       5
#define CSI_CSICR3_RXFF_LEVEL_64WORDS       6
#define CSI_CSICR3_RXFF_LEVEL_96WORDS       7

#define CSI_CSICR3_HRESP_ERR_EN_DISABLE     0
#define CSI_CSICR3_HRESP_ERR_EN_ENABLE      1

#define CSI_CSICR3_STATFF_LEVEL_4WORDS      0
#define CSI_CSICR3_STATFF_LEVEL_8WORDS      1
#define CSI_CSICR3_STATFF_LEVEL_12WORDS     2
#define CSI_CSICR3_STATFF_LEVEL_16WORDS     3
#define CSI_CSICR3_STATFF_LEVEL_24WORDS     4
#define CSI_CSICR3_STATFF_LEVEL_32WORDS     5
#define CSI_CSICR3_STATFF_LEVEL_48WORDS     6
#define CSI_CSICR3_STATFF_LEVEL_64WORDS     7

#define CSI_CSICR3_DMA_REQ_EN_SFF_DISABLE   0
#define CSI_CSICR3_DMA_REQ_EN_SFF_ENABLE    1

#define CSI_CSICR3_DMA_REQ_EN_RFF_DISABLE   0
#define CSI_CSICR3_DMA_REQ_EN_RFF_ENABLE    1

#define CSI_CSICR3_DMA_REFLASH_SFF_NO_FLASH    0
#define CSI_CSICR3_DMA_REFLASH_SFF_FLASH       1

#define CSI_CSICR3_DMA_REFLASH_RFF_NO_FLASH    0
#define CSI_CSICR3_DMA_REFLASH_RFF_FLASH       1

#define CSI_CSICR3_FRMCNT_RESET_DISABLE        0
#define CSI_CSICR3_FRMCNT_RESET_ENABLE         1

#define CSI_CSICR3_FRMCNT_VALUE(X)     (X)

//CSISTATFIFO : CSI Statistic FIFO Register

//CSIRXFIFO : CSI RxFIFO Register

//CSIRXCNT :  CSI RxFIFO Count Register
#define CSI_CSIRXCNT_RXCNT_VALUE(X)     (X & 0x3FFFFF)

//CSISR :  CSI Status Register
#define CSI_CSISR_ECC_INT_W1L               1
#define CSI_CSISR_HRESP_ERR_INT_W1L         1
#define CSI_CSISR_COF_INT_W1L               1
#define CSI_CSISR_SOF_INT_W1L               1
#define CSI_CSISR_EOF_INT_W1L               1
#define CSI_CSISR_DMA_TSF_DONE_FB1_W1L      1
#define CSI_CSISR_DMA_TSF_DONE_FB2_W1L      1
#define CSI_CSISR_DMA_TSF_DONE_SFF_W1L      1
#define CSI_CSISR_RF_OR_INT_W1L             1
#define CSI_CSISR_SF_OR_INT_W1L             1

//CSIDBGREG : CSI Debug Register
#define CSI_CSIDBGREG_DEBUG_SIGNAL_EN_DISABLE   0
#define CSI_CSIDBGREG_DEBUG_SIGNAL_EN_ENABLE    1

//CSIDMASASTATFIFO : CSI DMA Start Address Register - For STATFIFO

//CSIDMATSSTATFIFO : CSI DMA Transfer Size Register - For STATFIFO

//CSIDMASAFB1 : CSI DMA Start Address Register - For Frame Buffer 1

//CSIDMASAFB2 : CSI DMA Transfer Size Register - For Frame Buffer 2

//CSIFBUFPARA : CSI Frame Buffer Parameter Register

//CSIIMAGPARA : CSI Image Parameter Register

#ifdef __cplusplus
}
#endif

#endif // __MX25_CSI_H
