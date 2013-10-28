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
// File: mx27_pp.h
//
// Definitions for Post-Processor of eMMA module.
//
//------------------------------------------------------------------------------
#ifndef __MX27_PP_H__
#define __MX27_PP_H__

#ifdef __cplusplus
extern "C" {
#endif


//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------
#define CSP_BASE_REG_PA_EMMA_PP         CSP_BASE_REG_PA_EMMA


//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------
typedef struct {
    REG32 PP_CNTL;
    REG32 PP_INTRCNTL;
    REG32 PP_INTRSTATUS;
    REG32 PP_SOURCE_Y_PTR;
    REG32 PP_SOURCE_CB_PTR;
    REG32 PP_SOURCE_CR_PTR;
    REG32 PP_DEST_RGB_PTR;
    REG32 PP_QUANTIZER_PTR;
    REG32 PP_PROCESS_FRAME_PARA;
    REG32 PP_SOURCE_FRAME_WIDTH;
    REG32 PP_DEST_DISPLAY_WIDTH;
    REG32 PP_DEST_IMAGE_SIZE;
    REG32 PP_DEST_FRAME_FORMAT_CNTL;
    REG32 PP_RESIZE_TABLE_INDEX;
    REG32 PP_CSC_COEF_0123;
    REG32 PP_CSC_COEF_4;    
    REG32 _reserved[48];
    REG32 PP_RESIZE_COEF_TBL[32];
} CSP_PP_REGS, *PCSP_PP_REGS;


//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
#define PP_CNTL_OFFSET                      0x0000
#define PP_INTRCNTL_OFFSET                  0x0004
#define PP_INTRSTATUS_OFFSET                0x0008
#define PP_SOURCE_Y_PTR_OFFSET              0x000C
#define PP_SOURCE_CB_PTR_OFFSET             0x0010
#define PP_SOURCE_CR_PTR_OFFSET             0x0014
#define PP_DEST_RGB_PTR_OFFSET              0x0018
#define PP_QUANTIZER_PTR_OFFSET             0x001C
#define PP_PROCESS_FRAME_PARA_OFFSET        0x0020
#define PP_SOURCE_FRAME_WIDTH_OFFSET        0x0024
#define PP_DEST_DISPLAY_WIDTH_OFFSET        0x0028
#define PP_DEST_IMAGE_SIZE_OFFSET           0x002C
#define PP_DEST_FRAME_FORMAT_CNTL_OFFSET    0x0030
#define PP_RESIZE_TABLE_INDEX_OFFSET        0x0034
#define PP_CSC_COEF_0123_OFFSET             0x0038
#define PP_CSC_COEF_4_OFFSET                0x003C
#define PP_RESIZE_COEF_TBL_OFFSET           0x0100


//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
//
// PP_CNTL
#define PP_CNTL_PP_EN_LSH                                   0
#define PP_CNTL_DEBLOCKEN_LSH                               1
#define PP_CNTL_DERINGEN_LSH                                2
#define PP_CNTL_CSCEN_LSH                                   4
#define PP_CNTL_CSC_TABLE_SEL_LSH                           5
#define PP_CNTL_SWRST_LSH                                   8
#define PP_CNTL_MB_MODE_LSH                                 9
#define PP_CNTL_CSC_OUT_LSH                                 10
#define PP_CNTL_BSDI_LSH                                    12
#define PP_CNTL_BSDO_LSH                                    13
#define PP_CNTL_BSQP_LSH                                    14
#define PP_CNTL_MB_COUNT_LSH                                16

// PP_INTRCNTL
#define PP_INTRCNTL_FRAME_COMP_INTR_EN_LSH                  0
#define PP_INTRCNTL_MB_COMP_INTR_EN_LSH                     1
#define PP_INTRCNTL_ERR_INTR_EN_LSH                         2

// PP_INTRSTATUS
#define PP_INTRSTATUS_FRAME_COMP_INTR_LSH                   0
#define PP_INTRSTATUS_MB_COMP_INTR_LSH                      1
#define PP_INTRSTATUS_ERR_INTR_LSH                          2

// PP_SOURCE_Y_PTR
#define PP_SOURCE_Y_PTR_PP_Y_SOURCE_LSH                     0

// PP_SOURCE_CB_PTR
#define PP_SOURCE_CB_PTR_PP_CB_SOURCE_LSH                   0

// PP_SOURCE_CR_PTR
#define PP_SOURCE_CR_PTR_PP_CR_SOURCE_LSH                   0

// PP_DEST_RGB_PTR
#define PP_DEST_RGB_PTR_RGB_START_ADDR_LSH                  0

// PP_QUANTIZER_PTR
#define PP_QUANTIZER_PTR_QUANTIZER_PTR_LSH                  0

// PP_PROCESS_FRAME_PARA
#define PP_PROCESS_FRAME_PARA_PROCESS_FRAME_HEIGHT_LSH      0
#define PP_PROCESS_FRAME_PARA_PROCESS_FRAME_WIDTH_LSH       16

// PP_SOURCE_FRAME_WIDTH
#define PP_SOURCE_FRAME_WIDTH_Y_INPUT_LINE_STRIDE_LSH       0
#define PP_SOURCE_FRAME_WIDTH_QUANTIZER_FRAME_WIDTH_LSH     16

// PP_DEST_DISPLAY_WIDTH
#define PP_DEST_DISPLAY_WIDTH_OUPUT_LINE_STRIDE_LSH         0

// PP_DEST_IMAGE_SIZE
#define PP_DEST_IMAGE_SIZE_OUT_IMAGE_HEIGHT_LSH             0
#define PP_DEST_IMAGE_SIZE_OUT_IMAGE_WIDTH_LSH              16

// PP_DEST_FRAME_FORMAT_CNTL
#define PP_DEST_FRAME_FORMAT_CNTL_BLUE_WIDTH_LSH            0
#define PP_DEST_FRAME_FORMAT_CNTL_GREEN_WIDTH_LSH           4
#define PP_DEST_FRAME_FORMAT_CNTL_RED_WIDTH_LSH             8
#define PP_DEST_FRAME_FORMAT_CNTL_BLUE_OFFSET_LSH           16
#define PP_DEST_FRAME_FORMAT_CNTL_GREEN_OFFSET_LSH          21
#define PP_DEST_FRAME_FORMAT_CNTL_RED_OFFSET_LSH            26

// PP_RESIZE_TABLE_INDEX
#define PP_RESIZE_TABLE_INDEX_VERT_TBL_END_INDEX_LSH        0
#define PP_RESIZE_TABLE_INDEX_VERT_TBL_START_INDEX_LSH      8
#define PP_RESIZE_TABLE_INDEX_HORI_TBL_END_INDEX_LSH        16
#define PP_RESIZE_TABLE_INDEX_HORI_TBL_START_INDEX_LSH      24

// PP_CSC_COEF_0123
#define PP_CSC_COEF_0123_C3_LSH                             0
#define PP_CSC_COEF_0123_C2_LSH                             8
#define PP_CSC_COEF_0123_C1_LSH                             16
#define PP_CSC_COEF_0123_C0_LSH                             24

// PP_CSC_COEF_4
#define PP_CSC_COEF_4_C4_LSH                                0
#define PP_CSC_COEF_4_X0_LSH                                9

// PP_RESIZE_COEF_TBL
#define PP_RESIZE_COEF_TBL_OP_LSH                           0
#define PP_RESIZE_COEF_TBL_N_LSH                            1
#define PP_RESIZE_COEF_TBL_W_LSH                            3


//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
//
// PP_CNTL
#define PP_CNTL_PP_EN_WID                                   1
#define PP_CNTL_DEBLOCKEN_WID                               1
#define PP_CNTL_DERINGEN_WID                                1
#define PP_CNTL_CSCEN_WID                                   1
#define PP_CNTL_CSC_TABLE_SEL_WID                           2
#define PP_CNTL_SWRST_WID                                   1
#define PP_CNTL_MB_MODE_WID                                 1
#define PP_CNTL_CSC_OUT_WID                                 2
#define PP_CNTL_BSDI_WID                                    1
#define PP_CNTL_BSDO_WID                                    1
#define PP_CNTL_BSQP_WID                                    1
#define PP_CNTL_MB_COUNT_WID                                8

// PP_INTRCNTL
#define PP_INTRCNTL_FRAME_COMP_INTR_EN_WID                  1
#define PP_INTRCNTL_MB_COMP_INTR_EN_WID                     1
#define PP_INTRCNTL_ERR_INTR_EN_WID                         1

// PP_INTRSTATUS
#define PP_INTRSTATUS_FRAME_COMP_INTR_WID                   1
#define PP_INTRSTATUS_MB_COMP_INTR_WID                      1
#define PP_INTRSTATUS_ERR_INTR_WID                          1

// PP_SOURCE_Y_PTR
#define PP_SOURCE_Y_PTR_PP_Y_SOURCE_WID                     32

// PP_SOURCE_CB_PTR
#define PP_SOURCE_CB_PTR_PP_CB_SOURCE_WID                   32

// PP_SOURCE_CR_PTR
#define PP_SOURCE_CR_PTR_PP_CR_SOURCE_WID                   32

// PP_DEST_RGB_PTR
#define PP_DEST_RGB_PTR_RGB_START_ADDR_WID                  32

// PP_QUANTIZER_PTR
#define PP_QUANTIZER_PTR_QUANTIZER_PTR_WID                  32

// PP_PROCESS_FRAME_PARA
#define PP_PROCESS_FRAME_PARA_PROCESS_FRAME_HEIGHT_WID      10
#define PP_PROCESS_FRAME_PARA_PROCESS_FRAME_WIDTH_WID       10

// PP_SOURCE_FRAME_WIDTH
#define PP_SOURCE_FRAME_WIDTH_Y_INPUT_LINE_STRIDE_WID       12
#define PP_SOURCE_FRAME_WIDTH_QUANTIZER_FRAME_WIDTH_WID     8

// PP_DEST_DISPLAY_WIDTH
#define PP_DEST_DISPLAY_WIDTH_OUPUT_LINE_STRIDE_WID         13

// PP_DEST_IMAGE_SIZE
#define PP_DEST_IMAGE_SIZE_OUT_IMAGE_HEIGHT_WID             12
#define PP_DEST_IMAGE_SIZE_OUT_IMAGE_WIDTH_WID              12

// PP_DEST_FRAME_FORMAT_CNTL
#define PP_DEST_FRAME_FORMAT_CNTL_BLUE_WIDTH_WID            4
#define PP_DEST_FRAME_FORMAT_CNTL_GREEN_WIDTH_WID           4
#define PP_DEST_FRAME_FORMAT_CNTL_RED_WIDTH_WID             4
#define PP_DEST_FRAME_FORMAT_CNTL_BLUE_OFFSET_WID           5
#define PP_DEST_FRAME_FORMAT_CNTL_GREEN_OFFSET_WID          5
#define PP_DEST_FRAME_FORMAT_CNTL_RED_OFFSET_WID            5

// PP_RESIZE_TABLE_INDEX
#define PP_RESIZE_TABLE_INDEX_VERT_TBL_END_INDEX_WID        6
#define PP_RESIZE_TABLE_INDEX_VERT_TBL_START_INDEX_WID      6
#define PP_RESIZE_TABLE_INDEX_HORI_TBL_END_INDEX_WID        6
#define PP_RESIZE_TABLE_INDEX_HORI_TBL_START_INDEX_WID      6

// PP_CSC_COEF_0123
#define PP_CSC_COEF_0123_C3_WID                             8
#define PP_CSC_COEF_0123_C2_WID                             8
#define PP_CSC_COEF_0123_C1_WID                             8
#define PP_CSC_COEF_0123_C0_WID                             8

// PP_CSC_COEF_4
#define PP_CSC_COEF_4_C4_WID                                9
#define PP_CSC_COEF_4_X0_WID                                1

// PP_RESIZE_COEF_TBL
#define PP_RESIZE_COEF_TBL_OP_WID                           1
#define PP_RESIZE_COEF_TBL_N_WID                            2
#define PP_RESIZE_COEF_TBL_W_WID                            5


//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------
//
// PP_CNTL
#define PP_CNTL_PP_EN_DISABLE                   0
#define PP_CNTL_PP_EN_ENABLE                    1

#define PP_CNTL_DEBLOCKEN_DISABLE               0
#define PP_CNTL_DEBLOCKEN_ENABLE                1

#define PP_CNTL_DERINGEN_DISABLE                0
#define PP_CNTL_DERINGEN_ENABLE                 1

#define PP_CNTL_CSCEN_YUV                       0
#define PP_CNTL_CSCEN_RGB                       1

#define PP_CNTL_CSC_TABLE_SEL_A1                0
#define PP_CNTL_CSC_TABLE_SEL_A0                1
#define PP_CNTL_CSC_TABLE_SEL_B1                2
#define PP_CNTL_CSC_TABLE_SEL_B0                3

#define PP_CNTL_SWRST_NORESET                   0
#define PP_CNTL_SWRST_RESET                     1

#define PP_CNTL_MB_MODE_FRAME                   0
#define PP_CNTL_MB_MODE_MACROBLOCK              1

#define PP_CNTL_CSC_OUT_32BIT                   0
#define PP_CNTL_CSC_OUT_8BIT                    1
#define PP_CNTL_CSC_OUT_16BIT                   2

#define PP_CNTL_BSDI_DISABLE                    0
#define PP_CNTL_BSDI_ENABLE                     1

#define PP_CNTL_BSDO_DISABLE                    0
#define PP_CNTL_BSDO_ENABLE                     1

#define PP_CNTL_BSQP_DISABLE                    0
#define PP_CNTL_BSQP_ENABLE                     1


// PP_INTRCNTL
#define PP_INTRCNTL_FRAME_COMP_INTR_EN_DISABLE  0
#define PP_INTRCNTL_FRAME_COMP_INTR_EN_ENABLE   1

#define PP_INTRCNTL_MB_COMP_INTR_EN_DISABLE     0
#define PP_INTRCNTL_MB_COMP_INTR_EN_ENABLE      1

#define PP_INTRCNTL_ERR_INTR_EN_DISABLE         0
#define PP_INTRCNTL_ERR_INTR_EN_ENABLE          1


// PP_CSC_COEF_4
#define PP_CSC_COEF_4_X0_OFFSET0                0
#define PP_CSC_COEF_4_X0_OFFSET16               1


// PP_RESIZE_COEF_TBL
#define PP_RESIZE_COEF_TBL_OP_NOPIXELS          0
#define PP_RESIZE_COEF_TBL_OP_PIXELS            1

#define PP_RESIZE_COEF_TBL_N_NOPIXELS           0
#define PP_RESIZE_COEF_TBL_N_1PIXELS            1
#define PP_RESIZE_COEF_TBL_N_2PIXELS            2
#define PP_RESIZE_COEF_TBL_N_3PIXELS            3


#ifdef __cplusplus
}
#endif

#endif // __MX27_PP_H__

