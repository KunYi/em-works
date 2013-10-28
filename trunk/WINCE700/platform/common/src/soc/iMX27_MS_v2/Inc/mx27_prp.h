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
// Copyright (C) 2004,  MOTOROLA, INC. All Rights Reserved
//
//------------------------------------------------------------------------------
//
// Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
// File: mx27_prp.h
//
// Definitions for Pre-Processor of eMMA module
//
//------------------------------------------------------------------------------
#ifndef __MX27_PRP_H__
#define __MX27_PRP_H__

#ifdef __cplusplus
extern "C" {
#endif


//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------
#define PRP_REG_OFFSET                  0x400
#define CSP_BASE_REG_PA_EMMA_PRP        (CSP_BASE_REG_PA_EMMA + PRP_REG_OFFSET)

//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------
//
// Pre-Processing resize registers
typedef struct {
    REG32 PRP_RZ_COEF1;
    REG32 PRP_RZ_COEF2;
    REG32 PRP_RZ_VALID;
} CSP_PRP_RESIZE_REGS, *PCSP_PRP_RESIZE_REGS;

// Pre-Processing registers
typedef struct {
    REG32 PRP_CNTL;                             // 0x00
    REG32 PRP_INTRCNTL;                         // 0x04
    REG32 PRP_INTRSTATUS;                       // 0x08
    REG32 PRP_SOURCE_Y_PTR;                     // 0x0C
    REG32 PRP_SOURCE_CB_PTR;                    // 0x10
    REG32 PRP_SOURCE_CR_PTR;                    // 0x14
    REG32 PRP_DEST_RGB1_PTR;                    // 0x18
    REG32 PRP_DEST_RGB2_PTR;                    // 0x1C
    REG32 PRP_DEST_Y_PTR;                       // 0x20
    REG32 PRP_DEST_CB_PTR;                      // 0x24
    REG32 PRP_DEST_CR_PTR;                      // 0x28
    REG32 PRP_SOURCE_FRAME_SIZE;                // 0x2C
    REG32 PRP_CH1_LINE_STRIDE;                  // 0x30
    REG32 PRP_SOURCE_PIXEL_FORMAT_CNTL;         // 0x34
    REG32 PRP_CH1_PIXEL_FORMAT_CNTL;            // 0x38
    REG32 PRP_CH1_OUT_IMAGE_SIZE;               // 0x3C
    REG32 PRP_CH2_OUT_IMAGE_SIZE;               // 0x40
    REG32 PRP_SOURCE_LINE_STRIDE;               // 0x44
    REG32 PRP_CSC_COEF_012;                     // 0x48
    REG32 PRP_CSC_COEF_345;                     // 0x4C
    REG32 PRP_CSC_COEF_678;                     // 0x50
    /*-------------------------------------------
       PrP has 4 groups of resize registers

       Group 0 ([prpResize_Ch1 + prpResize_Horizontal])
       REG32 CH1_RZ_HORI_COEF1;                 // 0x54
       REG32 CH1_RZ_HORI_COEF2;                 // 0x58
       REG32 CH1_RZ_HORI_VALID;                 // 0x5C 

       Group 1 ([prpResize_Ch1 + prpResize_Vertical])
       REG32 CH1_RZ_VERT_COEF1;                 // 0x60
       REG32 CH1_RZ_VERT_COEF2;                 // 0x64
       REG32 CH1_RZ_VERT_VALID;                 // 0x68 

       Group 2 ([prpResize_Ch2 + prpResize_Horizontal])
       REG32 CH2_RZ_HORI_COEF1;                 // 0x6C
       REG32 CH2_RZ_HORI_COEF2;                 // 0x70
       REG32 CH2_RZ_HORI_VALID;                 // 0x74

       Group 3 ([prpResize_Ch2 + prpResize_Vertical])
       REG32 CH2_RZ_VERT_COEF1;                 // 0x78
       REG32 CH2_RZ_VERT_COEF2;                 // 0x7C
       REG32 CH2_RZ_VERT_VALID;                 // 0x80 
       
       Therefore we can reference the register as
       PRP_RESIZE[prpChannel*2 + dimension]
    *-------------------------------------------*/
    CSP_PRP_RESIZE_REGS PRP_RESIZE[4];               
} CSP_PRP_REGS, *PCSP_PRP_REGS;


//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
#define PRP_CNTL_OFFSET                         0x0000
#define PRP_INTRCNTL_OFFSET                     0x0004
#define PRP_INTRSTATUS_OFFSET                   0x0008
#define PRP_SOURCE_Y_PTR_OFFSET                 0x000C
#define PRP_SOURCE_CB_PTR_OFFSET                0x0010
#define PRP_SOURCE_CR_PTR_OFFSET                0x0014
#define PRP_DEST_RGB1_PTR_OFFSET                0x0018
#define PRP_DEST_RGB2_PTR_OFFSET                0x001C
#define PRP_DEST_Y_PTR_OFFSET                   0x0020
#define PRP_DEST_CB_PTR_OFFSET                  0x0024
#define PRP_DEST_CR_PTR_OFFSET                  0x0028
#define PRP_SOURCE_FRAME_SIZE_OFFSET            0x002C
#define PRP_CH1_LINE_STRIDE_OFFSET              0x0030
#define PRP_SOURCE_PIXEL_FORMAT_CNTL_OFFSET     0x0034
#define PRP_CH1_PIXEL_FORMAT_CNTL_OFFSET        0x0038
#define PRP_CH1_OUT_IMAGE_SIZE_OFFSET           0x003C
#define PRP_CH2_OUT_IMAGE_SIZE_OFFSET           0x0040
#define PRP_SOURCE_LINE_STRIDE_OFFSET           0x0044
#define PRP_CSC_COEF_012_OFFSET                 0x0048
#define PRP_CSC_COEF_345_OFFSET                 0x004C
#define PRP_CSC_COEF_678_OFFSET                 0x0050


//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
//
// PRP_CNTL
#define PRP_CNTL_CH1EN_LSH                                  0
#define PRP_CNTL_CH2EN_LSH                                  1
#define PRP_CNTL_CSIEN_LSH                                  2
#define PRP_CNTL_DATA_IN_MODE_LSH                           3
#define PRP_CNTL_CH1_OUT_MODE_LSH                           5
#define PRP_CNTL_CH2_OUT_MODE_LSH                           7
#define PRP_CNTL_CH1_LEN_LSH                                9
#define PRP_CNTL_CH2_LEN_LSH                                10
#define PRP_CNTL_SKIP_FRAME_LSH                             11
#define PRP_CNTL_SWRST_LSH                                  12
#define PRP_CNTL_CLKEN_LSH                                  13
#define PRP_CNTL_WEN_LSH                                    14
#define PRP_CNTL_CH1BYP_LSH                                 15
#define PRP_CNTL_IN_TSKIP_LSH                               16
#define PRP_CNTL_CH1_TSKIP_LSH                              19
#define PRP_CNTL_CH2_TSKIP_LSH                              22
#define PRP_CNTL_INPUT_FIFO_LEVEL_LSH                       25
#define PRP_CNTL_RZ_FIFO_LEVEL_LSH                          27
#define PRP_CNTL_CH2B1EN_LSH                                29
#define PRP_CNTL_CH2B2EN_LSH                                30
#define PRP_CNTL_CH2FEN_LSH                                 31

// PRP_INTRCNTL
#define PRP_INTRCNTL_RDERRIE_LSH                            0
#define PRP_INTRCNTL_CH1WERRIE_LSH                          1
#define PRP_INTRCNTL_CH2WERRIE_LSH                          2
#define PRP_INTRCNTL_CH1FCIE_LSH                            3
#define PRP_INTRCNTL_CH2FCIE_LSH                            5
#define PRP_INTRCNTL_LBOVFIE_LSH                            7
#define PRP_INTRCNTL_CH2OVFIE_LSH                           8

// PRP_INTRSTATUS
#define PRP_INTRSTATUS_READERR_LSH                          0
#define PRP_INTRSTATUS_CH1WRERR_LSH                         1
#define PRP_INTRSTATUS_CH2WRERR_LSH                         2
#define PRP_INTRSTATUS_CH2B2CI_LSH                          3
#define PRP_INTRSTATUS_CH2B1CI_LSH                          4
#define PRP_INTRSTATUS_CH1B2CI_LSH                          5
#define PRP_INTRSTATUS_CH1B1CI_LSH                          6
#define PRP_INTRSTATUS_LBOVF_LSH                            7
#define PRP_INTRSTATUS_CH2OVF_LSH                           8

// PRP_SOURCE_Y_PTR
#define PRP_SOURCE_Y_PTR_PRP_SOURCE_Y_PTR_LSH               0

// PRP_SOURCE_CB_PTR
#define PRP_SOURCE_CB_PTR_PRP_SOURCE_CB_PTR_LSH             0

// PRP_SOURCE_CR_PTR
#define PRP_SOURCE_CR_PTR_PRP_SOURCE_CR_PTR_LSH             0

// PRP_DEST_RGB1_PTR
#define PRP_DEST_RGB1_PTR_PRP_DEST_RGB1_PTR_LSH             0

// PRP_DEST_RGB2_PTR
#define PRP_DEST_RGB2_PTR_PRP_DEST_RGB2_PTR_LSH             0

// PRP_DEST_Y_PTR
#define PRP_DEST_Y_PTR_PRP_Y_DEST_LSH                       0

// PRP_DEST_CB_PTR
#define PRP_DEST_CB_PTR_PRP_CB_DEST_LSH                     0

// PRP_DEST_CR_PTR
#define PRP_DEST_CR_PTR_PRP_CR_DEST_LSH                     0

// PRP_SOURCE_FRAME_SIZE
#define PRP_SOURCE_FRAME_SIZE_PICTURE_Y_SIZE_LSH            0
#define PRP_SOURCE_FRAME_SIZE_PICTURE_X_SIZE_LSH            16

// PRP_CH1_LINE_STRIDE
#define PRP_CH1_LINE_STRIDE_CH1_OUT_LINE_STRIDE_LSH         0

// PRP_SOURCE_PIXEL_FORMAT_CNTL
#define PRP_SOURCE_PIXEL_FORMAT_CNTL_BLUE_WIDTH_LSH         0
#define PRP_SOURCE_PIXEL_FORMAT_CNTL_GREEN_WIDTH_LSH        4
#define PRP_SOURCE_PIXEL_FORMAT_CNTL_RED_WIDTH_LSH          8
#define PRP_SOURCE_PIXEL_FORMAT_CNTL_BLUE_V_CR_OFFSET_LSH   16
#define PRP_SOURCE_PIXEL_FORMAT_CNTL_GREEN_U_CB_OFFSET_LSH  21
#define PRP_SOURCE_PIXEL_FORMAT_CNTL_RED_Y_OFFSET_LSH       26

// PRP_CH1_PIXEL_FORMAT_CNTL
#define PRP_CH1_PIXEL_FORMAT_CNTL_BLUE_WIDTH_LSH            0
#define PRP_CH1_PIXEL_FORMAT_CNTL_GREEN_WIDTH_LSH           4
#define PRP_CH1_PIXEL_FORMAT_CNTL_RED_WIDTH_LSH             8
#define PRP_CH1_PIXEL_FORMAT_CNTL_BLUE_OFFSET_LSH           16
#define PRP_CH1_PIXEL_FORMAT_CNTL_GREEN_OFFSET_LSH          21
#define PRP_CH1_PIXEL_FORMAT_CNTL_RED_OFFSET_LSH            26

// PRP_CH1_OUT_IMAGE_SIZE
#define PRP_CH1_OUT_IMAGE_SIZE_CH1_OUT_IMAGE_HEIGHT_LSH     0
#define PRP_CH1_OUT_IMAGE_SIZE_CH1_OUT_IMAGE_WIDTH_LSH      16

// PRP_CH2_OUT_IMAGE_SIZE
#define PRP_CH2_OUT_IMAGE_SIZE_CH2_OUT_IMAGE_HEIGHT_LSH     0
#define PRP_CH2_OUT_IMAGE_SIZE_CH2_OUT_IMAGE_WIDTH_LSH      16

// PRP_SOURCE_LINE_STRIDE
#define PRP_SOURCE_LINE_STRIDE_SOURCE_LINE_STRIDE_LSH       0
#define PRP_SOURCE_LINE_STRIDE_CSI_LINE_SKIP_LSH            16

// PRP_CSC_COEF_012
#define PRP_CSC_COEF_012_C2_LSH                             0
#define PRP_CSC_COEF_012_C1_LSH                             11
#define PRP_CSC_COEF_012_C0_LSH                             21

// PRP_CSC_COEF_345
#define PRP_CSC_COEF_345_C5_LSH                             0
#define PRP_CSC_COEF_345_C4_LSH                             11
#define PRP_CSC_COEF_345_C3_LSH                             21

// PRP_CSC_COEF_678
#define PRP_CSC_COEF_678_C8_LSH                             0
#define PRP_CSC_COEF_678_C7_LSH                             11
#define PRP_CSC_COEF_678_C6_LSH                             21
#define PRP_CSC_COEF_678_X0_LSH                             31

// PRP_XXX_RZ_XXXX_VALID
#define PRP_RZ_VALID_OV_LSH                                 0
#define PRP_RZ_VALID_TBL_LEN_LSH                            24
#define PRP_RZ_VALID_AVG_BIL_LSH                            31


//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
//
// PRP_CNTL
#define PRP_CNTL_CH1EN_WID                                  1
#define PRP_CNTL_CH2EN_WID                                  1
#define PRP_CNTL_CSIEN_WID                                  1
#define PRP_CNTL_DATA_IN_MODE_WID                           2
#define PRP_CNTL_CH1_OUT_MODE_WID                           2
#define PRP_CNTL_CH2_OUT_MODE_WID                           2
#define PRP_CNTL_CH1_LEN_WID                                1
#define PRP_CNTL_CH2_LEN_WID                                1
#define PRP_CNTL_SKIP_FRAME_WID                             1
#define PRP_CNTL_SWRST_WID                                  1
#define PRP_CNTL_CLKEN_WID                                  1
#define PRP_CNTL_WEN_WID                                    1
#define PRP_CNTL_CH1BYP_WID                                 1
#define PRP_CNTL_IN_TSKIP_WID                               3
#define PRP_CNTL_CH1_TSKIP_WID                              3
#define PRP_CNTL_CH2_TSKIP_WID                              3
#define PRP_CNTL_INPUT_FIFO_LEVEL_WID                       2
#define PRP_CNTL_RZ_FIFO_LEVEL_WID                          2
#define PRP_CNTL_CH2B1EN_WID                                1
#define PRP_CNTL_CH2B2EN_WID                                1
#define PRP_CNTL_CH2FEN_WID                                 1

// PRP_INTRCNTL
#define PRP_INTRCNTL_RDERRIE_WID                            1
#define PRP_INTRCNTL_CH1WERRIE_WID                          1
#define PRP_INTRCNTL_CH2WERRIE_WID                          1
#define PRP_INTRCNTL_CH1FCIE_WID                            1
#define PRP_INTRCNTL_CH2FCIE_WID                            1
#define PRP_INTRCNTL_LBOVFIE_WID                            1
#define PRP_INTRCNTL_CH2OVFIE_WID                           1

// PRP_INTRSTATUS
#define PRP_INTRSTATUS_READERR_WID                          1
#define PRP_INTRSTATUS_CH1WRERR_WID                         1
#define PRP_INTRSTATUS_CH2WRERR_WID                         1
#define PRP_INTRSTATUS_CH2B2CI_WID                          1
#define PRP_INTRSTATUS_CH2B1CI_WID                          1
#define PRP_INTRSTATUS_CH1B2CI_WID                          1
#define PRP_INTRSTATUS_CH1B1CI_WID                          1
#define PRP_INTRSTATUS_LBOVF_WID                            1
#define PRP_INTRSTATUS_CH2OVF_WID                           1

// PRP_SOURCE_Y_PTR
#define PRP_SOURCE_Y_PTR_PRP_SOURCE_Y_PTR_WID               32

// PRP_SOURCE_CB_PTR
#define PRP_SOURCE_CB_PTR_PRP_SOURCE_CB_PTR_WID             32

// PRP_SOURCE_CR_PTR
#define PRP_SOURCE_CR_PTR_PRP_SOURCE_CR_PTR_WID             32

// PRP_DEST_RGB1_PTR
#define PRP_DEST_RGB1_PTR_PRP_DEST_RGB1_PTR_WID             32

// PRP_DEST_RGB2_PTR
#define PRP_DEST_RGB2_PTR_PRP_DEST_RGB2_PTR_WID             32

// PRP_DEST_Y_PTR
#define PRP_DEST_Y_PTR_PRP_Y_DEST_WID                       32

// PRP_DEST_CB_PTR
#define PRP_DEST_CB_PTR_PRP_CB_DEST_CB_WID                  32

// PRP_DEST_CR_PTR
#define PRP_DEST_CR_PTR_PRP_CR_DEST_WID                     32

// PRP_SOURCE_FRAME_SIZE
#define PRP_SOURCE_FRAME_SIZE_PICTURE_Y_SIZE_WID            11
#define PRP_SOURCE_FRAME_SIZE_PICTURE_X_SIZE_WID            11

// PRP_CH1_LINE_STRIDE
#define PRP_CH1_LINE_STRIDE_CH1_OUT_LINE_STRIDE_WID         12

// PRP_SOURCE_PIXEL_FORMAT_CNTL
#define PRP_SOURCE_PIXEL_FORMAT_CNTL_BLUE_WIDTH_WID         4
#define PRP_SOURCE_PIXEL_FORMAT_CNTL_GREEN_WIDTH_WID        4
#define PRP_SOURCE_PIXEL_FORMAT_CNTL_RED_WIDTH_WID          4
#define PRP_SOURCE_PIXEL_FORMAT_CNTL_BLUE_V_CR_OFFSET_WID   5
#define PRP_SOURCE_PIXEL_FORMAT_CNTL_GREEN_U_CB_OFFSET_WID  5
#define PRP_SOURCE_PIXEL_FORMAT_CNTL_RED_Y_OFFSET_WID       5

// PRP_CH1_PIXEL_FORMAT_CNTL
#define PRP_CH1_PIXEL_FORMAT_CNTL_BLUE_WIDTH_WID            4
#define PRP_CH1_PIXEL_FORMAT_CNTL_GREEN_WIDTH_WID           4
#define PRP_CH1_PIXEL_FORMAT_CNTL_RED_WIDTH_WID             4
#define PRP_CH1_PIXEL_FORMAT_CNTL_BLUE_OFFSET_WID           5
#define PRP_CH1_PIXEL_FORMAT_CNTL_GREEN_OFFSET_WID          5
#define PRP_CH1_PIXEL_FORMAT_CNTL_RED_OFFSET_WID            5

// PRP_CH1_OUT_IMAGE_SIZE
#define PRP_CH1_OUT_IMAGE_SIZE_CH1_OUT_IMAGE_HEIGHT_WID     11
#define PRP_CH1_OUT_IMAGE_SIZE_CH1_OUT_IMAGE_WIDTH_WID      11

// PRP_CH2_OUT_IMAGE_SIZE
#define PRP_CH2_OUT_IMAGE_SIZE_CH2_OUT_IMAGE_HEIGHT_WID     11
#define PRP_CH2_OUT_IMAGE_SIZE_CH2_OUT_IMAGE_WIDTH_WID      11

// PRP_SOURCE_LINE_STRIDE
#define PRP_SOURCE_LINE_STRIDE_SOURCE_LINE_STRIDE_WID       13
#define PRP_SOURCE_LINE_STRIDE_CSI_LINE_SKIP_WID            13

// PRP_CSC_COEF_012
#define PRP_CSC_COEF_012_C2_WID                             8
#define PRP_CSC_COEF_012_C1_WID                             8
#define PRP_CSC_COEF_012_C0_WID                             8

// PRP_CSC_COEF_345
#define PRP_CSC_COEF_345_C5_WID                             7
#define PRP_CSC_COEF_345_C4_WID                             9
#define PRP_CSC_COEF_345_C3_WID                             8

// PRP_CSC_COEF_678
#define PRP_CSC_COEF_678_C8_WID                             7
#define PRP_CSC_COEF_678_C7_WID                             7
#define PRP_CSC_COEF_678_C6_WID                             7
#define PRP_CSC_COEF_678_X0_WID                             1

// PRP_RZ_VALID
#define PRP_RZ_VALID_OV_WID                                 20
#define PRP_RZ_VALID_TBL_LEN_WID                            5
#define PRP_RZ_VALID_AVG_BIL_WID                            1


//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------
//
// PRP_CNTL
#define PRP_CNTL_CH1EN_DISABLE                              0
#define PRP_CNTL_CH1EN_ENABLE                               1

#define PRP_CNTL_CH2EN_DISABLE                              0
#define PRP_CNTL_CH2EN_ENABLE                               1

#define PRP_CNTL_CSIEN_MEM                                  0
#define PRP_CNTL_CSIEN_CSI                                  1

#define PRP_CNTL_DATA_IN_MODE_YUV420                        0
#define PRP_CNTL_DATA_IN_MODE_YUV422                        1
#define PRP_CNTL_DATA_IN_MODE_YUV444                        3
#define PRP_CNTL_DATA_IN_MODE_RGB16                         2
#define PRP_CNTL_DATA_IN_MODE_RGB32                         3

#define PRP_CNTL_CH1_OUT_MODE_RGB8                          0
#define PRP_CNTL_CH1_OUT_MODE_RGB16                         1
#define PRP_CNTL_CH1_OUT_MODE_RGB32                         2
#define PRP_CNTL_CH1_OUT_MODE_YUV422                        3

#define PRP_CNTL_CH2_OUT_MODE_YUV420                        0
#define PRP_CNTL_CH2_OUT_MODE_YUV422                        1
#define PRP_CNTL_CH2_OUT_MODE_YUV444                        2

#define PRP_CNTL_CH1_LEN_DISABLE                            0
#define PRP_CNTL_CH1_LEN_ENABLE                             1

#define PRP_CNTL_CH2_LEN_DISABLE                            0
#define PRP_CNTL_CH2_LEN_ENABLE                             1

#define PRP_CNTL_SKIP_FRAME_CONTINUE                        0
#define PRP_CNTL_SKIP_FRAME_STOP                            1

#define PRP_CNTL_SWRST_RESET                                1

#define PRP_CNTL_CLKEN_ON                                   0
#define PRP_CNTL_CLKEN_OFF                                  1

#define PRP_CNTL_WEN_DISABLE                                0
#define PRP_CNTL_WEN_ENABLE                                 1

#define PRP_CNTL_CH1BYP_CASCADE                             0
#define PRP_CNTL_CH1BYP_DISCASCADE                          1

#define PRP_CNTL_IN_TSKIP_NOSKIP                            0
#define PRP_CNTL_IN_TSKIP_1OF2                              1
#define PRP_CNTL_IN_TSKIP_1OF3                              2
#define PRP_CNTL_IN_TSKIP_2OF3                              3
#define PRP_CNTL_IN_TSKIP_1OF4                              4
#define PRP_CNTL_IN_TSKIP_3OF4                              5
#define PRP_CNTL_IN_TSKIP_2OF5                              6
#define PRP_CNTL_IN_TSKIP_4OF5                              7

#define PRP_CNTL_CH1_TSKIP_NOSKIP                           0
#define PRP_CNTL_CH1_TSKIP_1OF2                             1
#define PRP_CNTL_CH1_TSKIP_1OF3                             2
#define PRP_CNTL_CH1_TSKIP_2OF3                             3
#define PRP_CNTL_CH1_TSKIP_1OF4                             4
#define PRP_CNTL_CH1_TSKIP_3OF4                             5
#define PRP_CNTL_CH1_TSKIP_2OF5                             6
#define PRP_CNTL_CH1_TSKIP_4OF5                             7

#define PRP_CNTL_CH2_TSKIP_NOSKIP                           0
#define PRP_CNTL_CH2_TSKIP_1OF2                             1
#define PRP_CNTL_CH2_TSKIP_1OF3                             2
#define PRP_CNTL_CH2_TSKIP_2OF3                             3
#define PRP_CNTL_CH2_TSKIP_1OF4                             4
#define PRP_CNTL_CH2_TSKIP_3OF4                             5
#define PRP_CNTL_CH2_TSKIP_2OF5                             6
#define PRP_CNTL_CH2_TSKIP_4OF5                             7

#define PRP_CNTL_INPUT_FIFO_LEVEL_128W                      0
#define PRP_CNTL_INPUT_FIFO_LEVEL_96W                       1
#define PRP_CNTL_INPUT_FIFO_LEVEL_64W                       2
#define PRP_CNTL_INPUT_FIFO_LEVEL_32W                       3

#define PRP_CNTL_RZ_FIFO_LEVEL_64W                          0
#define PRP_CNTL_RZ_FIFO_LEVEL_48W                          1
#define PRP_CNTL_RZ_FIFO_LEVEL_32W                          2
#define PRP_CNTL_RZ_FIFO_LEVEL_16W                          3

#define PRP_CNTL_CH2B1EN_DISABLE                            0
#define PRP_CNTL_CH2B1EN_ENABLE                             1

#define PRP_CNTL_CH2B2EN_DISABLE                            0
#define PRP_CNTL_CH2B2EN_ENABLE                             1

#define PRP_CNTL_CH2FEN_DISABLE                             0
#define PRP_CNTL_CH2FEN_ENABLE                              1

// PRP_INTRCNTL
#define PRP_INTRCNTL_RDERRIE_DISABLE                        0
#define PRP_INTRCNTL_RDERRIE_ENABLE                         1

#define PRP_INTRCNTL_CH1WERRIE_DISABLE                      0
#define PRP_INTRCNTL_CH1WERRIE_ENABLE                       1

#define PRP_INTRCNTL_CH2WERRIE_DISABLE                      0
#define PRP_INTRCNTL_CH2WERRIE_ENABLE                       1

#define PRP_INTRCNTL_CH1FCIE_DISABLE                        0
#define PRP_INTRCNTL_CH1FCIE_ENABLE                         1

#define PRP_INTRCNTL_CH2FCIE_DISABLE                        0
#define PRP_INTRCNTL_CH2FCIE_ENABLE                         1

#define PRP_INTRCNTL_LBOVFIE_DISABLE                        0
#define PRP_INTRCNTL_LBOVFIE_ENABLE                         1

#define PRP_INTRCNTL_CH2OVFIE_DISABLE                       0
#define PRP_INTRCNTL_CH2OVFIE_ENABLE                        1

// PRP_CSC_COEF_678
#define PRP_CSC_COEF_678_X0_0                               0
#define PRP_CSC_COEF_678_X0_16                              1


#ifdef __cplusplus
}
#endif

#endif // __MX27_PRP_H__

