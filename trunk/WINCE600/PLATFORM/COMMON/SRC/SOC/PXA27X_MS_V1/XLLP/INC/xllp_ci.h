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
/****************************************************************************** 
** Copyright 2000-2003 Intel Corporation All Rights Reserved.
**
** Portions of the source code contained or described herein and all documents
** related to such source code (Material) are owned by Intel Corporation
** or its suppliers or licensors and is licensed by Microsoft Corporation for distribution.  
** Title to the Material remains with Intel Corporation or its suppliers and licensors. 
** Use of the Materials is subject to the terms of the Microsoft license agreement which accompanied the Materials.  
** No other license under any patent, copyright, trade secret or other intellectual
** property right is granted to or conferred upon you by disclosure or
** delivery of the Materials, either expressly, by implication, inducement,
** estoppel or otherwise 
** Some portion of the Materials may be copyrighted by Microsoft Corporation.
**
********************************************************************************/
#ifndef __XLLP_CI_H__
#define __XLLP_CI_H__

//---------------------------------------------------------------------------
// Register definitions
//---------------------------------------------------------------------------
#define XLLP_CI_REGBASE_PHY 0x50000000

enum XLLP_CI_REGS {
        XLLP_CI_REG_SIZE = 0x04U,    //Size of each CI register (below) = 4 bytes 
                
        XLLP_CICR0 = 0x00,           //Control
        XLLP_CICR1 = 0x04,
        XLLP_CICR2 = 0x08,
        XLLP_CICR3 = 0x0C,
        XLLP_CICR4 = 0x10,
        
        XLLP_CISR  = 0x14,           //Status
        XLLP_CIFR  = 0x18,           //FIFO Control
        XLLP_CITOR = 0x1C,           //Time-out
        
        XLLP_CIBR0 = 0x28,           //Receive Buffer
        XLLP_CIBR1 = 0x30,
        XLLP_CIBR2 = 0x38
};

enum XLLP_CI_REGBITS_CICR0 {
        XLLP_CI_CICR0_FOM       = 0x00000001,
        XLLP_CI_CICR0_EOFM      = 0x00000002,
        XLLP_CI_CICR0_SOFM      = 0x00000004,
        XLLP_CI_CICR0_CDM       = 0x00000008,
        XLLP_CI_CICR0_QDM       = 0x00000010,
        XLLP_CI_CICR0_PERRM     = 0x00000020,
        XLLP_CI_CICR0_EOLM      = 0x00000040,
        XLLP_CI_CICR0_FEM       = 0x00000080,
        XLLP_CI_CICR0_RDAVM     = 0x00000100,
        XLLP_CI_CICR0_TOM       = 0x00000200,
        XLLP_CI_CICR0_RESERVED  = 0x03FFFC00,
        XLLP_CI_CICR0_SIM_SHIFT = 24,
        XLLP_CI_CICR0_SIM_SMASK = 0x7,
        XLLP_CI_CICR0_DIS       = 0x08000000,
        XLLP_CI_CICR0_ENB       = 0x10000000,
        XLLP_CI_CICR0_SL_CAP_EN = 0x20000000,
        XLLP_CI_CICR0_PAR_EN    = 0x40000000,
        XLLP_CI_CICR0_DMA_EN    = 0x80000000,
        XLLP_CI_CICR0_INTERRUPT_MASK = 0x3FF
};

enum XLLP_CI_REGBITS_CICR1 {
        XLLP_CI_CICR1_DW_SHIFT       = 0,
        XLLP_CI_CICR1_DW_SMASK       = 0x7,
        XLLP_CI_CICR1_COLOR_SP_SHIFT = 3,
        XLLP_CI_CICR1_COLOR_SP_SMASK = 0x3,
        XLLP_CI_CICR1_RAW_BPP_SHIFT  = 5,
        XLLP_CI_CICR1_RAW_BPP_SMASK  = 0x3,
        XLLP_CI_CICR1_RGB_BPP_SHIFT  = 7,
        XLLP_CI_CICR1_RGB_BPP_SMASK  = 0x7,
        XLLP_CI_CICR1_YCBCR_F        = 0x00000400,
        XLLP_CI_CICR1_RBG_F          = 0x00000800,
        XLLP_CI_CICR1_RGB_CONV_SHIFT = 12,
        XLLP_CI_CICR1_RGB_CONV_SMASK = 0x7,
        XLLP_CI_CICR1_PPL_SHIFT      = 15,
        XLLP_CI_CICR1_PPL_SMASK      = 0x7FF,
        XLLP_CI_CICR1_RESERVED       = 0x1C000000,
        XLLP_CI_CICR1_RGBT_CONV_SHIFT= 29,
        XLLP_CI_CICR1_RGBT_CONV_SMASK= 0x3,
        XLLP_CI_CICR1_TBIT           = 0x80000000
};

enum XLLP_CI_REGBITS_CICR2 {
        XLLP_CI_CICR2_FSW_SHIFT = 0,
        XLLP_CI_CICR2_FSW_SMASK = 0x3,
        XLLP_CI_CICR2_BFPW_SHIFT= 3,
        XLLP_CI_CICR2_BFPW_SMASK= 0x3F,
        XLLP_CI_CICR2_RESERVED  = 0x00000200,
        XLLP_CI_CICR2_HSW_SHIFT = 10,
        XLLP_CI_CICR2_HSW_SMASK = 0x3F,
        XLLP_CI_CICR2_ELW_SHIFT = 16,
        XLLP_CI_CICR2_ELW_SMASK = 0xFF,
        XLLP_CI_CICR2_BLW_SHIFT = 24,     
        XLLP_CI_CICR2_BLW_SMASK = 0xFF    
};

enum XLLP_CI_REGBITS_CICR3 {
    XLLP_CI_CICR3_LPF_SHIFT = 0,
    XLLP_CI_CICR3_LPF_SMASK = 0x7FF,
    XLLP_CI_CICR3_VSW_SHIFT = 11,
    XLLP_CI_CICR3_VSW_SMASK = 0x1F,
    XLLP_CI_CICR3_EFW_SHIFT = 16,
    XLLP_CI_CICR3_EFW_SMASK = 0xFF,
    XLLP_CI_CICR3_BFW_SHIFT = 24,
    XLLP_CI_CICR3_BFW_SMASK = 0xFF
};

enum XLLP_CI_REGBITS_CICR4 {
    XLLP_CI_CICR4_DIV_SHIFT = 0,
    XLLP_CI_CICR4_DIV_SMASK = 0xFF,
    XLLP_CI_CICR4_FR_RATE_SHIFT = 8,
    XLLP_CI_CICR4_FR_RATE_SMASK = 0x7,
    XLLP_CI_CICR4_RESERVED1 = 0x0007F800,
    XLLP_CI_CICR4_MCLK_EN   = 0x00080000,
    XLLP_CI_CICR4_VSP       = 0x00100000,
    XLLP_CI_CICR4_HSP       = 0x00200000,
    XLLP_CI_CICR4_PCP       = 0x00400000,
    XLLP_CI_CICR4_PCLK_EN   = 0x00800000,
    XLLP_CI_CICR4_RESERVED2 = 0xFF000000,
    XLLP_CI_CICR4_RESERVED  = XLLP_CI_CICR4_RESERVED1 | XLLP_CI_CICR4_RESERVED2
};

enum XLLP_CI_REGBITS_CISR {
    XLLP_CI_CISR_IFO_0      = 0x00000001,
    XLLP_CI_CISR_IFO_1      = 0x00000002,
    XLLP_CI_CISR_IFO_2      = 0x00000004,
    XLLP_CI_CISR_EOF        = 0x00000008,
    XLLP_CI_CISR_SOF        = 0x00000010,
    XLLP_CI_CISR_CDD        = 0x00000020,
    XLLP_CI_CISR_CQD        = 0x00000040,
    XLLP_CI_CISR_PAR_ERR    = 0x00000080,
    XLLP_CI_CISR_EOL        = 0x00000100,
    XLLP_CI_CISR_FEMPTY_0   = 0x00000200,
    XLLP_CI_CISR_FEMPTY_1   = 0x00000400,
    XLLP_CI_CISR_FEMPTY_2   = 0x00000800,
    XLLP_CI_CISR_RDAV_0     = 0x00001000,
    XLLP_CI_CISR_RDAV_1     = 0x00002000,
    XLLP_CI_CISR_RDAV_2     = 0x00004000, 
    XLLP_CI_CISR_FTO        = 0x00008000,
    XLLP_CI_CISR_RESERVED   = 0xFFFF0000
};

enum XLLP_CI_REGBITS_CIFR {
    XLLP_CI_CIFR_FEN0       = 0x00000001,
    XLLP_CI_CIFR_FEN1       = 0x00000002,
    XLLP_CI_CIFR_FEN2       = 0x00000004,
    XLLP_CI_CIFR_RESETF     = 0x00000008,
    XLLP_CI_CIFR_THL_0_SHIFT= 4,
    XLLP_CI_CIFR_THL_0_SMASK= 0x3,
    XLLP_CI_CIFR_RESERVED1  = 0x000000C0,
    XLLP_CI_CIFR_FLVL0_SHIFT= 8,
    XLLP_CI_CIFR_FLVL0_SMASK= 0xFF,
    XLLP_CI_CIFR_FLVL1_SHIFT= 16,
    XLLP_CI_CIFR_FLVL1_SMASK= 0x7F,
    XLLP_CI_CIFR_FLVL2_SHIFT= 23,
    XLLP_CI_CIFR_FLVL2_SMASK= 0x7F,
    XLLP_CI_CIFR_RESERVED2  = 0xC0000000,
    XLLP_CI_CIFR_RESERVED   = XLLP_CI_CIFR_RESERVED1 | XLLP_CI_CIFR_RESERVED2 
};

//---------------------------------------------------------------------------
//     Parameter Type definitions
//---------------------------------------------------------------------------
typedef enum  {
        XLLP_CI_RAW8 = 0,                   //RAW
        XLLP_CI_RAW9,
        XLLP_CI_RAW10,
        XLLP_CI_YCBCR422,               //YCBCR
        XLLP_CI_YCBCR422_PLANAR,        //YCBCR Planaried
        XLLP_CI_RGB444,                 //RGB
        XLLP_CI_RGB555,
        XLLP_CI_RGB565,
        XLLP_CI_RGB666,
        XLLP_CI_RGB888,
        XLLP_CI_RGBT555_0,              //RGB+Transparent bit 0
        XLLP_CI_RGBT888_0,
        XLLP_CI_RGBT555_1,              //RGB+Transparent bit 1  
        XLLP_CI_RGBT888_1,
        XLLP_CI_RGB666_PACKED,          //RGB Packed 
        XLLP_CI_RGB888_PACKED,
        XLLP_CI_INVALID_FORMAT = 0xFF
} XLLP_CI_IMAGE_FORMAT;

typedef enum {
    XLLP_CI_INTSTATUS_IFO_0      = 0x00000001,
    XLLP_CI_INTSTATUS_IFO_1      = 0x00000002,
    XLLP_CI_INTSTATUS_IFO_2      = 0x00000004,
    XLLP_CI_INTSTATUS_EOF        = 0x00000008,
    XLLP_CI_INTSTATUS_SOF        = 0x00000010,
    XLLP_CI_INTSTATUS_CDD        = 0x00000020,
    XLLP_CI_INTSTATUS_CQD        = 0x00000040,
    XLLP_CI_INTSTATUS_PAR_ERR    = 0x00000080,
    XLLP_CI_INTSTATUS_EOL        = 0x00000100,
    XLLP_CI_INTSTATUS_FEMPTY_0   = 0x00000200,
    XLLP_CI_INTSTATUS_FEMPTY_1   = 0x00000400,
    XLLP_CI_INTSTATUS_FEMPTY_2   = 0x00000800,
    XLLP_CI_INTSTATUS_RDAV_0     = 0x00001000,
    XLLP_CI_INTSTATUS_RDAV_1     = 0x00002000,
    XLLP_CI_INTSTATUS_RDAV_2     = 0x00004000, 
    XLLP_CI_INTSTATUS_FTO        = 0x00008000,
    XLLP_CI_INTSTATUS_ALL       = 0x0000FFFF
} XLLP_CI_INTERRUPT_STATUS;

typedef enum {
    XLLP_CI_INT_IFO      = 0x00000001,
    XLLP_CI_INT_EOF      = 0x00000002,
    XLLP_CI_INT_SOF      = 0x00000004,
    XLLP_CI_INT_CDD      = 0x00000008,
    XLLP_CI_INT_CQD      = 0x00000010,
    XLLP_CI_INT_PAR_ERR  = 0x00000020,
    XLLP_CI_INT_EOL      = 0x00000040,
    XLLP_CI_INT_FEMPTY   = 0x00000080,
    XLLP_CI_INT_RDAV     = 0x00000100,
    XLLP_CI_INT_FTO      = 0x00000200,
    XLLP_CI_INT_ALL      = 0x000003FF
} XLLP_CI_INTERRUPT_MASK;
#define XLLP_CI_INT_MAX 10

typedef enum XLLP_CI_MODE {
        XLLP_CI_MODE_MP,             // Master-Parallel
        XLLP_CI_MODE_SP,             // Slave-Parallel
        XLLP_CI_MODE_MS,             // Master-Serial
        XLLP_CI_MODE_EP,             // Embedded-Parallel
        XLLP_CI_MODE_ES              // Embedded-Serial
} XLLP_CI_MODE;


typedef enum  {
        XLLP_CI_FR_ALL = 0,          // Capture all incoming frames
        XLLP_CI_FR_1_2,              // Capture 1 out of every 2 frames
        XLLP_CI_FR_1_3,              // Capture 1 out of every 3 frames
        XLLP_CI_FR_1_4,
        XLLP_CI_FR_1_5,
        XLLP_CI_FR_1_6,
        XLLP_CI_FR_1_7,
        XLLP_CI_FR_1_8
} XLLP_CI_FRAME_CAPTURE_RATE;


typedef enum  {
        XLLP_CI_FIFO_THL_32 = 0,
        XLLP_CI_FIFO_THL_64,
        XLLP_CI_FIFO_THL_96
} XLLP_CI_FIFO_THRESHOLD;

typedef struct {
    unsigned int BFW;
    unsigned int BLW;
} XLLP_CI_MP_TIMING, XLLP_CI_MS_TIMING;

typedef struct {
    unsigned int BLW;
    unsigned int ELW; 
    unsigned int HSW;
    unsigned int BFPW;
    unsigned int FSW; 
    unsigned int BFW;
    unsigned int EFW;
    unsigned int VSW; 
} XLLP_CI_SP_TIMING;

typedef enum {
    XLLP_CI_DATA_WIDTH4 = 0x0,
    XLLP_CI_DATA_WIDTH5 = 0x1,
    XLLP_CI_DATA_WIDTH8 = 0x2,  
    XLLP_CI_DATA_WIDTH9 = 0x3,  
    XLLP_CI_DATA_WIDTH10= 0x4   
} XLLP_CI_DATA_WIDTH;

//-------------------------------------------------------------------------------------------------------
//      Configuration APIs
//-------------------------------------------------------------------------------------------------------
void XllpCISetFrameRate(unsigned int ci_reg_base, XLLP_CI_FRAME_CAPTURE_RATE frate);
XLLP_CI_FRAME_CAPTURE_RATE XllpCIGetFrameRate(unsigned int ci_reg_base);
void XllpCISetImageFormat(unsigned int ci_reg_base, XLLP_CI_IMAGE_FORMAT input_format, XLLP_CI_IMAGE_FORMAT output_format); 
void XllpCISetMode(unsigned int ci_reg_base, XLLP_CI_MODE mode, XLLP_CI_DATA_WIDTH data_width);
void XllpCIConfigureMP(unsigned int ci_reg_base, unsigned int PPL, unsigned int LPF, XLLP_CI_MP_TIMING* timing);
void XllpCIConfigureSP(unsigned int ci_reg_base, unsigned int PPL, unsigned int LPF, XLLP_CI_SP_TIMING* timing);
void XllpCIConfigureMS(unsigned int ci_reg_base, unsigned int PPL, unsigned int LPF, XLLP_CI_MS_TIMING* timing);
void XllpCIConfigureEP(unsigned int ci_reg_base, int parity_check);
void XllpCIConfigureES(unsigned int ci_reg_base, int parity_check);
void XllpCISetClock(unsigned int ci_reg_base, unsigned int clk_regs_base, int pclk_enable, int mclk_enable, unsigned int mclk_mhz);
void XllpCISetPolarity(unsigned int ci_reg_base, int pclk_sample_falling, int hsync_active_low, int vsync_active_low);
void XllpCISetFIFO(unsigned int ci_reg_base, unsigned int timeout, XLLP_CI_FIFO_THRESHOLD threshold, int fifo1_enable, 
                   int fifo2_enable);
void XllpCIResetFIFO(unsigned int ci_reg_base);
void XllpCISetInterruptMask(unsigned int ci_reg_base, unsigned int mask);
unsigned int XllpCIGetInterruptMask(unsigned int ci_reg_base);
void XllpCIClearInterruptStatus(unsigned int ci_reg_base, unsigned int status);
unsigned int XllpCIGetInterruptStatus(unsigned int ci_reg_base);
void XllpCISetRegisterValue(unsigned int ci_reg_base, unsigned int reg_offset, unsigned int value);
void XllpCIDisableComplete(unsigned int ci_reg_base);

//-------------------------------------------------------------------------------------------------------
//      Control APIs
//-------------------------------------------------------------------------------------------------------
void XllpCIInit(unsigned int ci_reg_base, unsigned int clk_regs_base);
void XllpCIDeInit(unsigned int ci_reg_base, unsigned int clk_regs_base);
void XllpCIEnable(unsigned int ci_reg_base, int dma_en);
int  XllpCIDisable(unsigned int ci_reg_base, unsigned int ost_reg_base, int quick, int wait_for_disable_complete );
void XllpCISlaveCaptureEnable(unsigned int ci_reg_base);
void XllpCISlaveCaptureDisable(unsigned int ci_reg_base);


#endif