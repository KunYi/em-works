//------------------------------------------------------------------------------
//
//  Copyright (C) 2008-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header:  common_tvev2.h
//
//  Provides definitions for TVEV2 module.
//
//------------------------------------------------------------------------------
#ifndef __COMMON_TVEV2_H
#define __COMMON_TVEV2_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------
   
//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------

#define TVEV2_COM_CONF_REG_OFFSET                    0x0000
#define TVEV2_LUMA_FILT_CONT_REG_0_OFFSET            0x0004
#define TVEV2_LUMA_FILT_CONT_REG_1_OFFSET            0x0008
#define TVEV2_LUMA_FILT_CONT_REG_2_OFFSET            0x000C
#define TVEV2_LUMA_FILT_CONT_REG_3_OFFSET            0x0010
#define TVEV2_LUMA_SA_CONT_REG_0_OFFSET              0x0014
#define TVEV2_LUMA_SA_CONT_REG_1_OFFSET              0x0018
#define TVEV2_LUMA_SA_STAT_REG_0_OFFSET              0x001C
#define TVEV2_LUMA_SA_STAT_REG_1_OFFSET              0x0020
#define TVEV2_CHROMA_CONT_REG_OFFSET                 0x0024
#define TVEV2_TVDAC_0_CONT_REG_OFFSET                0x0028
#define TVEV2_TVDAC_1_CONT_REG_OFFSET                0x002C
#define TVEV2_TVDAC_2_CONT_REG_OFFSET                0x0030
#define TVEV2_CD_CONT_REG_OFFSET                     0x0034
#define TVEV2_VBI_DATA_CONT_REG_OFFSET               0x0038
#define TVEV2_VBI_DATA_REG_0_OFFSET                  0x003C
#define TVEV2_VBI_DATA_REG_1_OFFSET                  0x0040
#define TVEV2_VBI_DATA_REG_2_OFFSET                  0x0044
#define TVEV2_VBI_DATA_REG_3_OFFSET                  0x0048
#define TVEV2_VBI_DATA_REG_4_OFFSET                  0x004C
#define TVEV2_VBI_DATA_REG_5_OFFSET                  0x0050
#define TVEV2_VBI_DATA_REG_6_OFFSET                  0x0054
#define TVEV2_VBI_DATA_REG_7_OFFSET                  0x0058
#define TVEV2_VBI_DATA_REG_8_OFFSET                  0x005C
#define TVEV2_VBI_DATA_REG_9_OFFSET                  0x0060
#define TVEV2_INT_CONT_REG_OFFSET                    0x0064
#define TVEV2_STAT_REG_OFFSET                        0x0068
#define TVEV2_TST_MODE_REG_OFFSET                    0x006C
#define TVEV2_USER_MODE_CONT_REG_OFFSET              0x0070
#define TVEV2_SD_TIMING_USR_CONT_REG_0_OFFSET        0x0074
#define TVEV2_SD_TIMING_USR_CONT_REG_1_OFFSET        0x0078
#define TVEV2_SD_TIMING_USR_CONT_REG_2_OFFSET        0x007C
#define TVEV2_HD_TIMING_USR_CONT_REG_0_OFFSET        0x0080
#define TVEV2_HD_TIMING_USR_CONT_REG_1_OFFSET        0x0084
#define TVEV2_HD_TIMING_USR_CONT_REG_2_OFFSET        0x0088
#define TVEV2_LUMA_USR_CONT_REG_0_OFFSET             0x008C
#define TVEV2_LUMA_USR_CONT_REG_1_OFFSET             0x0090
#define TVEV2_LUMA_USR_CONT_REG_2_OFFSET             0x0094
#define TVEV2_LUMA_USR_CONT_REG_3_OFFSET             0x0098
#define TVEV2_CSC_USR_CONT_REG_0_OFFSET              0x009C
#define TVEV2_CSC_USR_CONT_REG_1_OFFSET              0x00A0
#define TVEV2_CSC_USR_CONT_REG_2_OFFSET              0x00A4
#define TVEV2_BLANK_USR_CONT_REG_OFFSET              0x00A8
#define TVEV2_SD_MOD_USR_CONT_REG_OFFSET             0x00AC
#define TVEV2_VBI_DATA_USR_CONT_REG_0_OFFSET         0x00B0
#define TVEV2_VBI_DATA_USR_CONT_REG_1_OFFSET         0x00B4
#define TVEV2_VBI_DATA_USR_CONT_REG_2_OFFSET         0x00B8
#define TVEV2_VBI_DATA_USR_CONT_REG_3_OFFSET         0x00BC
#define TVEV2_VBI_DATA_USR_CONT_REG_4_OFFSET         0x00C0
#define TVEV2_DROP_COMP_USR_CONT_REG_OFFSET          0x00C4


//------------------------------------------------------------------------------
// Macro to access the registers
//------------------------------------------------------------------------------

#define  REG32SETBIT(addr,bitpos) \
         OUTREG32((addr),(INREG32((addr)) | (1<<(bitpos))))


//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------

// TVEV2 Registers
typedef struct
{
    UINT32 COM_CONF_REG;                   // Common Configuration Register
    UINT32 LUMA_FILT_CONT_REG_0;           // Luma Filter Control Register 0
    UINT32 LUMA_FILT_CONT_REG_1;           // Luma Filter Control Register 1
    UINT32 LUMA_FILT_CONT_REG_2;           // Luma Filter Control Register 2
    UINT32 LUMA_FILT_CONT_REG_3;           // Luma Filter Control Register 3
    UINT32 LUMA_SA_CONT_REG_0;             // Luma Statistic Analysis Control Register 0
    UINT32 LUMA_SA_CONT_REG_1;             // Luma Statistic Analysis Control Register 1
    UINT32 LUMA_SA_STAT_REG_0;             // Luma Statistic Analysis Status Register 0
    UINT32 LUMA_SA_STAT_REG_1;             // Luma Statistic Analysis Status Register 1
    UINT32 CHROMA_CONT_REG;                // Chroma Control Register
    UINT32 TVDAC_0_CONT_REG;               // TVDAC 0 Control Register
    UINT32 TVDAC_1_CONT_REG;               // TVDAC 1 Control Register
    UINT32 TVDAC_2_CONT_REG;               // TVDAC 2 Control Register
    UINT32 CD_CONT_REG;                    // Cable Detection Control Register
    UINT32 VBI_DATA_CONT_REG;              // VBI Data Control Register
    UINT32 VBI_DATA_REG_0;                 // VBI Data Register 0
    UINT32 VBI_DATA_REG_1;                 // VBI Data Register 1
    UINT32 VBI_DATA_REG_2;                 // VBI Data Register 2
    UINT32 VBI_DATA_REG_3;                 // VBI Data Register 3
    UINT32 VBI_DATA_REG_4;                 // VBI Data Register 4
    UINT32 VBI_DATA_REG_5;                 // VBI Data Register 5
    UINT32 VBI_DATA_REG_6;                 // VBI Data Register 6
    UINT32 VBI_DATA_REG_7;                 // VBI Data Register 7
    UINT32 VBI_DATA_REG_8;                 // VBI Data Register 8
    UINT32 VBI_DATA_REG_9;                 // VBI Data Register 9
    UINT32 INT_CONT_REG;                   // Interrupt Control Register
    UINT32 STAT_REG;                       // Status Register
    UINT32 TST_MODE_REG;                   // Test Mode Register
    UINT32 USER_MODE_CONT_REG;             // User Mode Control Register
    UINT32 SD_TIMING_USR_CONT_REG_0;       // SD Timing User Control Register 0
    UINT32 SD_TIMING_USR_CONT_REG_1;       // SD Timing User Control Register 1
    UINT32 SD_TIMING_USR_CONT_REG_2;       // SD Timing User Control Register 2
    UINT32 HD_TIMING_USR_CONT_REG_0;       // HD Timing User Control Register 0
    UINT32 HD_TIMING_USR_CONT_REG_1;       // HD Timing User Control Register 1
    UINT32 HD_TIMING_USR_CONT_REG_2;       // HD Timing User Control Register 2
    UINT32 LUMA_USR_CONT_REG_0;            // Luma User Control Register 0
    UINT32 LUMA_USR_CONT_REG_1;            // Luma User Control Register 1
    UINT32 LUMA_USR_CONT_REG_2;            // Luma User Control Register 2
    UINT32 LUMA_USR_CONT_REG_3;            // Luma User Control Register 3
    UINT32 CSC_USR_CONT_REG_0;             // Color Space Conversion User Control Register 0
    UINT32 CSC_USR_CONT_REG_1;             // Color Space Conversion User Control Register 1
    UINT32 CSC_USR_CONT_REG_2;             // Color Space Conversion User Control Register 2
    UINT32 BLANK_USR_CONT_REG;             // Blanking Level User Control Register
    UINT32 SD_MOD_USR_CONT_REG;            // SD Modulation User Control Register
    UINT32 VBI_DATA_USR_CONT_REG_0;        // VBI Data User Control Register 0
    UINT32 VBI_DATA_USR_CONT_REG_1;        // VBI Data User Control Register 1
    UINT32 VBI_DATA_USR_CONT_REG_2;        // VBI Data User Control Register 2
    UINT32 VBI_DATA_USR_CONT_REG_3;        // VBI Data User Control Register 3
    UINT32 VBI_DATA_USR_CONT_REG_4;        // VBI Data User Control Register 4
    UINT32 DROP_COMP_USR_CONT_REG;         // DROP Compensation User Control Register
    UINT32 RESERVED_REG_0; 
    UINT32 RESERVED_REG_1;
    UINT32 RESERVED_REG_2;
    UINT32 RESERVED_REG_3; 
    UINT32 RESERVED_REG_4;
    UINT32 RESERVED_REG_5;
   
} TVEV2_REGS, *PTVEV2_REGS;

//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------

// COM_CONF_REG
#define TVEV2_TVE_EN_LSH                        0
#define TVEV2_TVDAC_SAMP_RATE_LSH               1
#define TVEV2_IPU_CLK_EN_LSH                    3
#define TVEV2_DATA_SOURCE_SEL_LSH               4
#define TVEV2_INP_VIDEO_FORM_LSH                6
#define TVEV2_P2I_CONV_EN_LSH                   7
#define TVEV2_TV_STAND_LSH                      8
#define TVEV2_TV_OUT_MODE_LSH                   12
#define TVEV2_SD_PED_AMP_CONT_LSH               16
#define TVEV2_SYNC_CH_0_EN_LSH                  20
#define TVEV2_SYNC_CH_1_EN_LSH                  21
#define TVEV2_SYNC_CH_2_EN_LSH                  22
#define TVEV2_ACT_LINE_OFFSET_LSH               24

// LUMA_FILT_CONT_REG_0
#define TVEV2_DEFLICK_EN_LSH                    0
#define TVEV2_DEFLICK_MEAS_WIN_LSH              1
#define TVEV2_DEFLICK_COEF_LSH                  4
#define TVEV2_DEFLICK_LOW_THRESH_LSH            8
#define TVEV2_DEFLICK_MID_THRESH_LSH            16
#define TVEV2_DEFLICK_HIGH_THRESH_LSH           24

// LUMA_FILT_CONT_REG_1
#define TVEV2_V_SHARP_EN_LSH                    0
#define TVEV2_V_SHARP_COEF_LSH                  4
#define TVEV2_V_SHARP_LOW_THRESH_LSH            8
#define TVEV2_V_SHARO_HIGH_THRESH_LSH           24

// LUMA_FILT_CONT_REG_2
#define TVEV2_H_SHARP_EN_LSH                    0
#define TVEV2_H_SHARP_COEF_LSH                  4
#define TVEV2_H_SHARP_LOW_THRESH_LSH            8
#define TVEV2_H_SHARP_HIGH_THRESH_LSH           24

// LUMA_FILT_CONT_REG_3
#define TVEV2_DERING_EN_LSH                     0
#define TVEV2_SUPP_FILTER_TYPE_LSH              1
#define TVEV2_DERING_COEF_LSH                   4
#define TVEV2_DERING_LOW_THRESH_LSH             8
#define TVEV2_DERING_MID_THRESH_LSH             16
#define TVEV2_DERING_HIGH_THRESH_LSH            24

// LUMA_SA_CONT_REG_0
#define TVEV2_LUMA_SA_EN_LSH                    0
#define TVEV2_SA_H_POINTS_NUM_LSH               4
#define TVEV2_SA_V_POINTS_NUM_LSH               8

// LUMA_SA_CONT_REG_1
#define TVEV2_SA_WIN_WIDTH_LSH                  0
#define TVEV2_SA_WIN_HEIGHT_LSH                 8
#define TVEV2_SA_WIN_H_OFFSET_LSH               16
#define TVEV2_SA_WIN_V_OFFSET_LSH               24

// LUMA_SA_STAT_REG_0
#define TVEV2_DEFLICK_MEAS_MEAN_LSH             0
#define TVEV2_V_SHARP_MEAS_MEAN_LSH             8
#define TVEV2_H_SHARP_MEAS_MEAN_LSH             16
#define TVEV2_DERING_MEAS_MEAN_LSH              24

// LUMA_SA_STAT_REG_1
#define TVEV2_LUMA_MEAN_LSH                     0

// CHROMA_CONT_REG
#define TVEV2_CHROMA_V_FILT_EN_LSH              0
#define TVEV2_CHROMA_BW_LSH                     4
#define TVEV2_SCH_PHASE_LSH                     8

// TVDAC_0_CONT_REG
#define TVEV2_TVDAC_0_GAIN_LSH                  0
#define TVEV2_TVDAC_0_OFFSET_LSH                8
#define TVEV2_TVDAC_0_BG_RDY_TIME_LSH           16

// TVDAC_1_CONT_REG
#define TVEV2_TVDAC_1_GAIN_LSH                  0
#define TVEV2_TVDAC_1_OFFSET_LSH                8

// TVDAC_2_CONT_REG
#define TVEV2_TVDAC_2_GAIN_LSH                  0
#define TVEV2_TVDAC_2_OFFSET_LSH                8

// CD_CONT_REG
#define TVEV2_CD_EN_LSH                         0
#define TVEV2_CD_TRIG_MODE_LSH                  1
#define TVEV2_CD_MON_PER_LSH                    4
#define TVEV2_CD_CH_0_REF_LVL_LSH               8
#define TVEV2_CD_CH_1_REF_LVL_LSH               9
#define TVEV2_CD_CH_2_REF_LVL_LSH               10
#define TVEV2_CD_REF_MODE_LSH                   11
#define TVEV2_CD_CH_0_LM_EN_LSH                 16
#define TVEV2_CD_CH_1_LM_EN_LSH                 17
#define TVEV2_CD_CH_2_LM_EN_LSH                 18
#define TVEV2_CD_CH_0_SM_EN_LSH                 20
#define TVEV2_CD_CH_1_SM_EN_LSH                 21
#define TVEV2_CD_CH_2_SM_EN_LSH                 22

// VBI_DATA_CONT_REG
#define TVEV2_CC_SD_F1_EN_LSH                   0
#define TVEV2_CC_SD_F2_EN_LSH                   1
#define TVEV2_CC_SD_BOOST_EN_LSH                2
#define TVEV2_CGMS_SD_F1_EN_LSH                 4
#define TVEV2_CGMS_SD_F2_EN_LSH                 5
#define TVEV2_CGMS_SD_SW_CRC_EN_LSH             6
#define TVEV2_WSS_SD_EN_LSH                     7
#define TVEV2_CGMS_HD_A_F1_EN_LSH               8
#define TVEV2_CGMS_HD_A_F2_EN_LSH               9
#define TVEV2_CGMS_HD_A_SW_CRC_EN_LSH           10
#define TVEV2_CGMS_HD_B_F1_EN_LSH               12
#define TVEV2_CGMS_HD_B_F2_EN_LSH               13
#define TVEV2_CGMS_HD_B_SW_CRC_EN_LSH           14
#define TVEV2_CGMS_HD_B_F1_HEADER_LSH           16
#define TVEV2_CGMS_HD_B_F2_HEADER_LSH           24

// VBI_DATA_REG_0
#define TVEV2_CGMS_SD_HD_A_F1_DATA_LSH          0

// VBI_DATA_REG_1
#define TVEV2_CGMS_SD_HD_A_F2_DATA_LSH          0

// VBI_DATA_REG_2
#define TVEV2_CC_SD_CGMS_HD_B_F1_DATA_0_LSH     0   // This VBI register is shared with CC SD and CGMS HD B
#define TVEV2_CC_SD_F1_DATA1_LSH                0   // define SD CC F1 data 1
#define TVEV2_CC_SD_F1_DATA2_LSH                8   // define SD CC F1 data 2

// VBI_DATA_REG_3
#define TVEV2_WSS_SD_CGMS_HD_B_F1_DATA_1_LSH    0   // This VBI register is shared with WSS SD and CGMS HD B
#define TVEV2_WSS_SD_DATA_LSH                   0   // define SD WSS data

// VBI_DATA_REG_4
#define TVEV2_CGMS_HD_B_F1_DATA_2_LSH           0

// VBI_DATA_REG_5
#define TVEV2_CGMS_HD_B_F1_DATA_3_LSH           0

// VBI_DATA_REG_6
#define TVEV2_CC_SD_CGMS_HD_B_F2_DATA_0_LSH     0   // This VBI register is shared with CC SD and CGMS HD B
#define TVEV2_CC_SD_F2_DATA1_LSH                0   // define SD CC F2 data 1
#define TVEV2_CC_SD_F2_DATA2_LSH                8   // define SD CC F2 data 2


// VBI_DATA_REG_7
#define TVEV2_CGMS_HD_B_F2_DATA_1_LSH           0

// VBI_DATA_REG_8
#define TVEV2_CGMS_HD_B_F2_DATA_2_LSH           0

// VBI_DATA_REG_9
#define TVEV2_CGMS_HD_B_F2_DATA_3_LSH           0

// INT_CONT_REG
#define TVEV2_CD_LM_IEN_LSH                     0
#define TVEV2_CD_SM_IEN_LSH                     1
#define TVEV2_CD_MON_END_IEN_LSH                2
#define TVEV2_CC_SD_F1_DONE_IEN_LSH             3
#define TVEV2_CC_SD_F2_DONE_IEN_LSH             4
#define TVEV2_CGMS_SD_F1_DONE_IEN_LSH           5
#define TVEV2_CGMS_SD_F2_DONE_IEN_LSH           6
#define TVEV2_WSS_SD_DONE_IEN_LSH               7
#define TVEV2_CGMS_HD_A_F1_DONE_IEN_LSH         8
#define TVEV2_CGMS_HD_A_F2_DONE_IEN_LSH         9
#define TVEV2_CGMS_HD_B_F1_DONE_IEN_LSH         10
#define TVEV2_CGMS_HD_B_F2_DONE_IEN_LSH         11
#define TVEV2_TVE_FIELD_END_IEN_LSH             12
#define TVEV2_TVE_FRAME_END_IEN_LSH             13
#define TVEV2_SA_MEAS_END_IEN_LSH               14

// STAT_REG
#define TVEV2_CD_LM_INT_LSH                     0
#define TVEV2_CD_SM_INT_LSH                     1
#define TVEV2_CD_MON_END_INT_LSH                2
#define TVEV2_CC_SD_F1_DONE_INT_LSH             3
#define TVEV2_CC_SD_F2_DONE_INT_LSH             4
#define TVEV2_CGMS_SD_F1_DONE_INT_LSH           5
#define TVEV2_CGMS_SD_F2_DONE_INT_LSH           6
#define TVEV2_WSS_SD_DONE_INT_LSH               7
#define TVEV2_CGMS_HD_A_F1_DONE_INT_LSH         8
#define TVEV2_CGMS_HD_A_F2_DONE_INT_LSH         9
#define TVEV2_CGMS_HD_B_F1_DONE_INT_LSH         10
#define TVEV2_CGMS_HD_B_F2_DONE_INT_LSH         11
#define TVEV2_FIELD_END_INT_LSH                 12
#define TVEV2_FRAME_END_INT_LSH                 13
#define TVEV2_SA_MEAS_END_INT_LSH               14
#define TVEV2_CD_CH_0_LM_ST_LSH                 16
#define TVEV2_CD_CH_1_LM_ST_LSH                 17
#define TVEV2_CD_CH_2_LM_ST_LSH                 18
#define TVEV2_CD_CH_0_SM_ST_LSH                 20
#define TVEV2_CD_CH_1_SM_ST_LSH                 21
#define TVEV2_CD_CH_2_SM_ST_LSH                 22
#define TVEV2_CD_MAN_TRIG_LSH                   24
#define TVEV2_BG_READY_LSH                      25

// TST_MODE_REG
#define TVEV2_TVDAC_TEST_MODE_LSH               0
#define TVEV2_TVDAC_0_DATA_FORCE_LSH            4
#define TVEV2_TVDAC_1_DATA_FORCE_LSH            5
#define TVEV2_TVDAC_2_DATA_FORCE_LSH            6
#define TVEV2_TVDAC_TEST_SINE_FREQ_LSH          8
#define TVEV2_TVDAC_TEST_SINE_LEVEL_LSH         12
#define TVEV2_COLORBAR_TYPE_LSH                 16

// USER_MODE_CONT_REG
#define TVEV2_H_TIMING_USR_MODE_EN_LSH          0
#define TVEV2_LUMA_FILT_USR_MODE_EN_LSH         1
#define TVEV2_SC_FREQ_USR_MODE_EN_LSH           2
#define TVEV2_CSCM_COEF_USR_MODE_EN_LSH         3
#define TVEV2_BLANK_LEVEL_USR_MODE_EN_LSH       4
#define TVEV2_VBI_DATA_USR_MODE_EN_LSH          5
#define TVEV2_TVDAC_DROP_COMP_USR_MODE_EN_LSH   6

// SD_TIMING_USR_CONT_REG_0
#define TVEV2_SD_VBI_T0_USR_LSH                 0
#define TVEV2_SD_VBI_T1_USR_LSH                 8
#define TVEV2_SD_VBI_T2_USR_LSH                 20

// SD_TIMING_USR_CONT_REG_1
#define TVEV2_SD_ACT_T0_USR_LSH                 0
#define TVEV2_SD_ACT_T1_USR_LSH                 8
#define TVEV2_SD_ACT_T2_USR_LSH                 16
#define TVEV2_SD_ACT_T3_USR_LSH                 24

// SD_TIMING_USR_CONT_REG_2
#define TVEV2_SD_ACT_T4_USR_LSH                 0
#define TVEV2_SD_ACT_T5_USR_LSH                 12
#define TVEV2_SD_ACT_T6_USR_LSH                 24

// HD_TIMING_USR_CONT_REG_0
#define TVEV2_HD_VBI_ACT_T0_USR_LSH             0
#define TVEV2_HD_VBI_T1_USR_LSH                 8
#define TVEV2_HD_VBI_T2_USR_LSH                 20

// HD_TIMING_USR_CONT_REG_1
#define TVEV2_HD_VBI_T3_USR_LSH                 0
#define TVEV2_HD_ACT_T1_USR_LSH                 16

// HD_TIMING_USR_CONT_REG_2
#define TVEV2_HD_ACT_T2_USR_LSH                 0
#define TVEV2_HD_ACT_T3_USR_LSH                 16

// LUMA_USR_CONT_REG_0
#define TVEV2_DEFLICK_MASK_MATRIX_USR_LSH       0

// LUMA_USR_CONT_REG_1
#define TVEV2_V_SHARP_MASK_MATRIX_USR_LSH       0

// LUMA_USR_CONT_REG_2
#define TVEV2_H_SHARP_MASK_MATRIX_USR_LSH       0

// LUMA_USR_CONT_REG_3
#define TVEV2_DERING_MASK_MATRIX_USR_LSH        0

// CSC_USR_CONT_REG_0
#define TVEV2_DATA_CLIP_USR_LSH                 0
#define TVEV2_BRIGHT_CORR_USR_LSH               8
#define TVEV2_CSCM_A_COEF_USR_LSH               16

// CSC_USR_CONT_REG_1
#define TVEV2_CSCM_B_COEF_USR_LSH               0
#define TVEV2_CSCM_C_COEF_USR_LSH               16

// CSC_USR_CONT_REG_2
#define TVEV2_CSCM_D_COEF_USR_LSH               0
#define TVEV2_CSCM_E_COEF_USR_LSH               16

// BLANK_USR_CONT_REG
#define TVEV2_BLANKING_CH_0_USR_LSH             0
#define TVEV2_BLANKING_CH_1_USR_LSH             10
#define TVEV2_BLANKING_CH_2_USR_LSH             20

// SD_MOD_USR_CONT_REG
#define TVEV2_SC_FREQ_USR_LSH                   0

// VBI_DATA_USR_CONT_REG_0
#define TVEV2_VBI_DATA_START_TIME_USR_LSH       0
#define TVEV2_VBI_DATA_STOP_TIME_USR_LSH        16

// VBI_DATA_USR_CONT_REG_1
#define TVEV2_VBI_PACKET_START_TIME_USR_LSH     0

// VBI_DATA_USR_CONT_REG_2
#define TVEV2_CC_SD_RUNIN_START_TIME_USR_LSH    0
#define TVEV2_CC_SD_RUNIN_DIV_NUM_USR_LSH       16

// VBI_DATA_USR_CONT_REG_3
#define TVEV2_CC_SD_CGMS_HD_B_DIV_USR_LSH       0
#define TVEV2_CC_SD_CGMS_HD_B_DIV_DENOM_USR_LSH 16

// VBI_DATA_USR_CONT_REG_4
#define TVEV2_CC_SD_CGMS_HD_A_DIV_USR_LSH       0
#define TVEV2_CC_SD_CGMS_HD_A_DIV_DENOM_USR_LSH 16

// DROP_COMP_USR_CONT_REG
#define TVEV2_TVDAC_0_DROP_COMP_LSH             0
#define TVEV2_TVDAC_1_DROP_COMP_LSH             4
#define TVEV2_TVDAC_2_DROP_COMP_LSH             8


//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------

// COM_CONF_REG
#define TVEV2_TVE_EN_WID                        1
#define TVEV2_TVDAC_SAMP_RATE_WID               2
#define TVEV2_IPU_CLK_EN_WID                    1
#define TVEV2_DATA_SOURCE_SEL_WID               2
#define TVEV2_INP_VIDEO_FORM_WID                1
#define TVEV2_P2I_CONV_EN_WID                   1
#define TVEV2_TV_STAND_WID                      4
#define TVEV2_TV_OUT_MODE_WID                   3
#define TVEV2_SD_PED_AMP_CONT_WID               2
#define TVEV2_SYNC_CH_0_EN_WID                  1
#define TVEV2_SYNC_CH_1_EN_WID                  1
#define TVEV2_SYNC_CH_2_EN_WID                  1
#define TVEV2_ACT_LINE_OFFSET_WID               3

// LUMA_FILT_CONT_REG_0
#define TVEV2_DEFLICK_EN_WID                    1
#define TVEV2_DEFLICK_MEAS_WIN_WID              1
#define TVEV2_DEFLICK_COEF_WID                  3
#define TVEV2_DEFLICK_LOW_THRESH_WID            8
#define TVEV2_DEFLICK_MID_THRESH_WID            8
#define TVEV2_DEFLICK_HIGH_THRESH_WID           8

// LUMA_FILT_CONT_REG_1
#define TVEV2_V_SHARP_EN_WID                    1
#define TVEV2_V_SHARP_COEF_WID                  3
#define TVEV2_V_SHARP_LOW_THRESH_WID            8
#define TVEV2_V_SHARO_HIGH_THRESH_WID           8

// LUMA_FILT_CONT_REG_2
#define TVEV2_H_SHARP_EN_WID                    1
#define TVEV2_H_SHARP_COEF_WID                  3
#define TVEV2_H_SHARP_LOW_THRESH_WID            8
#define TVEV2_H_SHARP_HIGH_THRESH_WID           8

// LUMA_FILT_CONT_REG_3
#define TVEV2_DERING_EN_WID                     1
#define TVEV2_SUPP_FILTER_TYPE_WID              2
#define TVEV2_DERING_COEF_WID                   3
#define TVEV2_DERING_LOW_THRESH_WID             8
#define TVEV2_DERING_MID_THRESH_WID             8
#define TVEV2_DERING_HIGH_THRESH_WID            8

// LUMA_SA_CONT_REG_0
#define TVEV2_LUMA_SA_EN_WID                    1
#define TVEV2_SA_H_POINTS_NUM_WID               2
#define TVEV2_SA_V_POINTS_NUM_WID               2

// LUMA_SA_CONT_REG_1
#define TVEV2_SA_WIN_WIDTH_WID                  8
#define TVEV2_SA_WIN_HEIGHT_WID                 8
#define TVEV2_SA_WIN_H_OFFSET_WID               8
#define TVEV2_SA_WIN_V_OFFSET_WID               8

// LUMA_SA_STAT_REG_0
#define TVEV2_DEFLICK_MEAS_MEAN_WID             8
#define TVEV2_V_SHARP_MEAS_MEAN_WID             8
#define TVEV2_H_SHARP_MEAS_MEAN_WID             8
#define TVEV2_DERING_MEAS_MEAN_WID              8

// LUMA_SA_STAT_REG_1
#define TVEV2_LUMA_MEAN_WID                     8

// CHROMA_CONT_REG
#define TVEV2_CHROMA_V_FILT_EN_WID              1
#define TVEV2_CHROMA_BW_WID                     3
#define TVEV2_SCH_PHASE_WID                     8

// TVDAC_0_CONT_REG
#define TVEV2_TVDAC_0_GAIN_WID                  6
#define TVEV2_TVDAC_0_OFFSET_WID                8
#define TVEV2_TVDAC_0_BG_RDY_TIME_WID           8

// TVDAC_1_CONT_REG
#define TVEV2_TVDAC_1_GAIN_WID                  6
#define TVEV2_TVDAC_1_OFFSET_WID                8

// TVDAC_2_CONT_REG
#define TVEV2_TVDAC_2_GAIN_WID                  6
#define TVEV2_TVDAC_2_OFFSET_WID                8

// CD_CONT_REG
#define TVEV2_CD_EN_WID                         1
#define TVEV2_CD_TRIG_MODE_WID                  1
#define TVEV2_CD_MON_PER_WID                    4
#define TVEV2_CD_CH_0_REF_LVL_WID               1
#define TVEV2_CD_CH_1_REF_LVL_WID               1
#define TVEV2_CD_CH_2_REF_LVL_WID               1
#define TVEV2_CD_REF_MODE_WID                   1
#define TVEV2_CD_CH_0_LM_EN_WID                 1
#define TVEV2_CD_CH_1_LM_EN_WID                 1
#define TVEV2_CD_CH_2_LM_EN_WID                 1
#define TVEV2_CD_CH_0_SM_EN_WID                 1
#define TVEV2_CD_CH_1_SM_EN_WID                 1
#define TVEV2_CD_CH_2_SM_EN_WID                 1

// VBI_DATA_CONT_REG
#define TVEV2_CC_SD_F1_EN_WID                   1
#define TVEV2_CC_SD_F2_EN_WID                   1
#define TVEV2_CC_SD_BOOST_EN_WID                1
#define TVEV2_CGMS_SD_F1_EN_WID                 1
#define TVEV2_CGMS_SD_F2_EN_WID                 1
#define TVEV2_CGMS_SD_SW_CRC_EN_WID             1
#define TVEV2_WSS_SD_EN_WID                     1
#define TVEV2_CGMS_HD_A_F1_EN_WID               1
#define TVEV2_CGMS_HD_A_F2_EN_WID               1
#define TVEV2_CGMS_HD_A_SW_CRC_EN_WID           1
#define TVEV2_CGMS_HD_B_F1_EN_WID               1
#define TVEV2_CGMS_HD_B_F2_EN_WID               1
#define TVEV2_CGMS_HD_B_SW_CRC_EN_WID           1
#define TVEV2_CGMS_HD_B_F1_HEADER_WID           6
#define TVEV2_CGMS_HD_B_F2_HEADER_WID           6

// VBI_DATA_REG_0
#define TVEV2_CGMS_SD_HD_A_F1_DATA_WID          20

// VBI_DATA_REG_1
#define TVEV2_CGMS_SD_HD_A_F2_DATA_WID          20

// VBI_DATA_REG_2
#define TVEV2_CC_SD_CGMS_HD_B_F1_DATA_0_WID     32
#define TVEV2_CC_SD_F1_DATA1_WID                7    // CC_SD_CGMS_HD_B_F1_DATA_0[6:0]
#define TVEV2_CC_SD_F1_DATA2_WID                7    // CC_SD_CGMS_HD_B_F1_DATA_0[14:8]

// VBI_DATA_REG_3
#define TVEV2_WSS_SD_CGMS_HD_B_F1_DATA_1_WID    32
#define TVEV2_WSS_SD_DATA_WID                   20   // WSS_SD_CGMS_HD_B_F1_DATA_1[19:0]

// VBI_DATA_REG_4
#define TVEV2_CGMS_HD_B_F1_DATA_2_WID           32

// VBI_DATA_REG_5
#define TVEV2_CGMS_HD_B_F1_DATA_3_WID           32

// VBI_DATA_REG_6
#define TVEV2_CC_SD_CGMS_HD_B_F2_DATA_0_WID     32
#define TVEV2_CC_SD_F2_DATA1_WID                7
#define TVEV2_CC_SD_F2_DATA2_WID                7

// VBI_DATA_REG_7
#define TVEV2_CGMS_HD_B_F2_DATA_1_WID           32

// VBI_DATA_REG_8
#define TVEV2_CGMS_HD_B_F2_DATA_2_WID           32

// VBI_DATA_REG_9
#define TVEV2_CGMS_HD_B_F2_DATA_3_WID           32

// INT_CONT_REG
#define TVEV2_CD_LM_IEN_WID                     1
#define TVEV2_CD_SM_IEN_WID                     1
#define TVEV2_CD_MON_END_IEN_WID                1
#define TVEV2_CC_SD_F1_DONE_IEN_WID             1
#define TVEV2_CC_SD_F2_DONE_IEN_WID             1
#define TVEV2_CGMS_SD_F1_DONE_IEN_WID           1
#define TVEV2_CGMS_SD_F2_DONE_IEN_WID           1
#define TVEV2_WSS_SD_DONE_IEN_WID               1
#define TVEV2_CGMS_HD_A_F1_DONE_IEN_WID         1
#define TVEV2_CGMS_HD_A_F2_DONE_IEN_WID         1
#define TVEV2_CGMS_HD_B_F1_DONE_IEN_WID         1
#define TVEV2_CGMS_HD_B_F2_DONE_IEN_WID         1
#define TVEV2_TVE_FIELD_END_IEN_WID             1
#define TVEV2_TVE_FRAME_END_IEN_WID             1
#define TVEV2_SA_MEAS_END_IEN_WID               1

// STAT_REG
#define TVEV2_CD_LM_INT_WID                     1
#define TVEV2_CD_SM_INT_WID                     1
#define TVEV2_CD_MON_END_INT_WID                1
#define TVEV2_CC_SD_F1_DONE_INT_WID             1
#define TVEV2_CC_SD_F2_DONE_INT_WID             1
#define TVEV2_CGMS_SD_F1_DONE_INT_WID           1
#define TVEV2_CGMS_SD_F2_DONE_INT_WID           1
#define TVEV2_WSS_SD_DONE_INT_WID               1
#define TVEV2_CGMS_HD_A_F1_DONE_INT_WID         1
#define TVEV2_CGMS_HD_A_F2_DONE_INT_WID         1
#define TVEV2_CGMS_HD_B_F1_DONE_INT_WID         1
#define TVEV2_CGMS_HD_B_F2_DONE_INT_WID         1
#define TVEV2_FIELD_END_INT_WID                 1
#define TVEV2_FRAME_END_INT_WID                 1
#define TVEV2_SA_MEAS_END_INT_WID               1
#define TVEV2_CD_CH_0_LM_ST_WID                 1
#define TVEV2_CD_CH_1_LM_ST_WID                 1
#define TVEV2_CD_CH_2_LM_ST_WID                 1
#define TVEV2_CD_CH_0_SM_ST_WID                 1
#define TVEV2_CD_CH_1_SM_ST_WID                 1
#define TVEV2_CD_CH_2_SM_ST_WID                 1
#define TVEV2_CD_MAN_TRIG_WID                   1
#define TVEV2_BG_READY_WID                      1

// TST_MODE_REG
#define TVEV2_TVDAC_TEST_MODE_WID               3
#define TVEV2_TVDAC_0_DATA_FORCE_WID            1
#define TVEV2_TVDAC_1_DATA_FORCE_WID            1
#define TVEV2_TVDAC_2_DATA_FORCE_WID            1
#define TVEV2_TVDAC_TEST_SINE_FREQ_WID          3
#define TVEV2_TVDAC_TEST_SINE_LEVEL_WID         2
#define TVEV2_COLORBAR_TYPE_WID                 1

// USER_MODE_CONT_REG
#define TVEV2_H_TIMING_USR_MODE_EN_WID          1
#define TVEV2_LUMA_FILT_USR_MODE_EN_WID         1
#define TVEV2_SC_FREQ_USR_MODE_EN_WID           1
#define TVEV2_CSCM_COEF_USR_MODE_EN_WID         1
#define TVEV2_BLANK_LEVEL_USR_MODE_EN_WID       1
#define TVEV2_VBI_DATA_USR_MODE_EN_WID          1
#define TVEV2_TVDAC_DROP_COMP_USR_MODE_EN_WID   1

// SD_TIMING_USR_CONT_REG_0
#define TVEV2_SD_VBI_T0_USR_WID                 6
#define TVEV2_SD_VBI_T1_USR_WID                 10
#define TVEV2_SD_VBI_T2_USR_WID                 10

// SD_TIMING_USR_CONT_REG_1
#define TVEV2_SD_ACT_T0_USR_WID                 7
#define TVEV2_SD_ACT_T1_USR_WID                 5
#define TVEV2_SD_ACT_T2_USR_WID                 7
#define TVEV2_SD_ACT_T3_USR_WID                 7

// SD_TIMING_USR_CONT_REG_2
#define TVEV2_SD_ACT_T4_USR_WID                 11
#define TVEV2_SD_ACT_T5_USR_WID                 10
#define TVEV2_SD_ACT_T6_USR_WID                 6

// HD_TIMING_USR_CONT_REG_0
#define TVEV2_HD_VBI_ACT_T0_USR_WID             7
#define TVEV2_HD_VBI_T1_USR_WID                 9
#define TVEV2_HD_VBI_T2_USR_WID                 11

// HD_TIMING_USR_CONT_REG_1
#define TVEV2_HD_VBI_T3_USR_WID                 13
#define TVEV2_HD_ACT_T1_USR_WID                 9

// HD_TIMING_USR_CONT_REG_2
#define TVEV2_HD_ACT_T2_USR_WID                 12
#define TVEV2_HD_ACT_T3_USR_WID                 13

// LUMA_USR_CONT_REG_0
#define TVEV2_DEFLICK_MASK_MATRIX_USR_WID       24

// LUMA_USR_CONT_REG_1
#define TVEV2_V_SHARP_MASK_MATRIX_USR_WID       24

// LUMA_USR_CONT_REG_2
#define TVEV2_H_SHARP_MASK_MATRIX_USR_WID       24

// LUMA_USR_CONT_REG_3
#define TVEV2_DERING_MASK_MATRIX_USR_WID        24

// CSC_USR_CONT_REG_0
#define TVEV2_DATA_CLIP_USR_WID                 1
#define TVEV2_BRIGHT_CORR_USR_WID               6
#define TVEV2_CSCM_A_COEF_USR_WID               11

// CSC_USR_CONT_REG_1
#define TVEV2_CSCM_B_COEF_USR_WID               12
#define TVEV2_CSCM_C_COEF_USR_WID               11

// CSC_USR_CONT_REG_2
#define TVEV2_CSCM_D_COEF_USR_WID               12
#define TVEV2_CSCM_E_COEF_USR_WID               13

// BLANK_USR_CONT_REG
#define TVEV2_BLANKING_CH_0_USR_WID             10
#define TVEV2_BLANKING_CH_1_USR_WID             10
#define TVEV2_BLANKING_CH_2_USR_WID             10

// SD_MOD_USR_CONT_REG
#define TVEV2_SC_FREQ_USR_WID                   30

// VBI_DATA_USR_CONT_REG_0
#define TVEV2_VBI_DATA_START_TIME_USR_WID       12
#define TVEV2_VBI_DATA_STOP_TIME_USR_WID        12

// VBI_DATA_USR_CONT_REG_1
#define TVEV2_VBI_PACKET_START_TIME_USR_WID     12

// VBI_DATA_USR_CONT_REG_2
#define TVEV2_CC_SD_RUNIN_START_TIME_USR_WID    12
#define TVEV2_CC_SD_RUNIN_DIV_NUM_USR_WID       11

// VBI_DATA_USR_CONT_REG_3
#define TVEV2_CC_SD_CGMS_HD_B_DIV_USR_WID       7
#define TVEV2_CC_SD_CGMS_HD_B_DIV_DENOM_USR_WID 13

// VBI_DATA_USR_CONT_REG_4
#define TVEV2_CC_SD_CGMS_HD_A_DIV_USR_WID       7
#define TVEV2_CC_SD_CGMS_HD_A_DIV_DENOM_USR_WID 13

// DROP_COMP_USR_CONT_REG
#define TVEV2_TVDAC_0_DROP_COMP_WID             4
#define TVEV2_TVDAC_1_DROP_COMP_WID             4
#define TVEV2_TVDAC_2_DROP_COMP_WID             4



//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------
#define TVEV2_ENABLE                            1
#define TVEV2_DISABLE                           0

// TVEV2 Common Configuration Register (COM_CONF_REG)

#define TVEV2_TVE_EN_ENABLE                     1
#define TVEV2_TVE_EN_DISABLE                    0

#define TVEV2_TVDAC_SD_SAMPRATE_216MHZ          0   // 00  216MHz for SD
#define TVEV2_TVDAC_SD_SAMPRATE_108MHZ          1   // 01  108MHZ for SD
#define TVEV2_TVDAC_SD_SAMPRATE_54MHZ           2   // 10  54MHz for SD
#define TVEV2_TVDAC_HD_SAMPRATE_297MHZ          0   // 00  297MHz for HD
#define TVEV2_TVDAC_HD_SAMPRATE_148MHZ          1   // 01  148.5MHz for HD
                       
#define TVEV2_IPU_CLK_EN_ENABLE                 1
#define TVEV2_IPU_CLK_EN_DISABLE                0

#define TVEV2_DATA_SOURCE_SEL_IPU_BUS_1         0   // 00 Video data bus 1 from the IPU
#define TVEV2_DATA_SOURCE_SEL_IPU_BUS_2         1   // 01 Video data bus 2 from the IPU
#define TVEV2_DATA_SOURCE_SEL_EXTL_TEST_BUS     2   // 10 External test data bus
#define TVEV2_DATA_SOURCE_SEL_INTL_COLOR_BAR    3   // 11 Interlnal Color Bar Generator

#define TVEV2_INP_VIDEO_FORM_YCBCR422           0
#define TVEV2_INP_VIDEO_FORM_YCBCR444           1   

#define TVEV2_P2I_CONV_EN_ENABLE                1   // Enable progressive to interlaced conversion.
                                                    // Valid only in for interlaced output mode
#define TVEV2_P2I_CONV_EN_DISABLE               0   // Disable. Must be 0 for progressive output mode

#define TVEV2_TV_STAND_SD_NTSC                  0   // 0000 SD NTSC
#define TVEV2_TV_STAND_SD_PALM                  1   // 0001 SD PALM
#define TVEV2_TV_STAND_SD_PALN                  2   // 0010 SD Combination PALN                                                   
#define TVEV2_TV_STAND_SD_PAL                   3   // 0011 SD PAL (B,D,G,G,I), which is a normal PAL mode 
#define TVEV2_TV_STAND_HD_720P60                4   // 0100 HD 720P60
#define TVEV2_TV_STAND_HD_720P50                5   // 0101 HD 720P50 
#define TVEV2_TV_STAND_HD_720P30                6   // 0110 HD 720P30
#define TVEV2_TV_STAND_HD_720P25                7   // 0111 HD 720P25
#define TVEV2_TV_STAND_HD_720P24                8   // 1000 HD 720P24
#define TVEV2_TV_STAND_HD_1080I60               9   // 1001 HD 1080I60
#define TVEV2_TV_STAND_HD_1080I50               10  // 1010 HD 1080I50 
#define TVEV2_TV_STAND_HD_1035I60               11  // 1011 HD 1035I60 (1920x1035)
#define TVEV2_TV_STAND_HD_1080P30               12  // 1100 HD 1080P30
#define TVEV2_TV_STAND_HD_1080P25               13  // 1101 HD 1080P25
#define TVEV2_TV_STAND_HD_1080P24               14  // 1110 HD 1080P24

#define TVEV2_TV_OUT_MODE_DISABLE               0   // 000 Disable output 
#define TVEV2_TV_OUT_MODE_CVBS_CH0              1   // 001 CVBS on channel #0 
#define TVEV2_TV_OUT_MODE_CVBS_CH2              2   // 002 CVBS on channel #2 
#define TVEV2_TV_OUT_MODE_CVBS_CH0_CH2          3   // 011 CVBS on both channels #0 and #2 
#define TVEV2_TV_OUT_MODE_S_CH0_CH1             4   // 100 S-video on channels #0 and #1 
#define TVEV2_TV_OUT_MODE_S_CH0_CH1_CVBS_CH2    5   // 101 S-video on channels #0 and #1 and CVBS on channel #2 
#define TVEV2_TV_OUT_MODE_YPRPB                 6   // 110 YPrPb component 
#define TVEV2_TV_OUT_MODE_RGB                   7   // 111 RGB component 

#define TVEV2_SD_PED_AMP_CONT_714MV_NO_PED      0   // 714mV (black-to-white)/286mV (synch), no pedestal
#define TVEV2_SD_PED_AMP_CONT_714MV_WITH_PED    1   // 714mV (black-to-white)/286mV (synch), with pedestal
#define TVEV2_SD_PED_AMP_CONT_700MV_NO_PED      2   // 700mV (black-to-white)/300mV (synch), no pedestal

#define TVEV2_SYNC_CH_0_EN_ENABLE               1   // enable sync on output channel 0
#define TVEV2_SYNC_CH_0_EN_DISABLE              0   // disable sync on output channel 0

#define TVEV2_SYNC_CH_1_EN_ENABLE               1   // enable sync on output channel 1
#define TVEV2_SYNC_CH_1_EN_DISABLE              0   // disable sync on output channel 1

#define TVEV2_SYNC_CH_2_EN_ENABLE               1   // enable sync on output channel 2
#define TVEV2_SYNC_CH_2_EN_DISABLE              0   // disable sync on output channel 2

#define TVEV2_ACT_LINE_OFFSET_NO_OFFSET         0   // No offset
#define TVEV2_ACT_LINE_OFFSET_1_LINE_OFFSET     1   // 1-line offset
#define TVEV2_ACT_LINE_OFFSET_2_LINE_OFFSET     2   // 2-line offset
#define TVEV2_ACT_LINE_OFFSET_3_LINE_OFFSET     3   // 3-line offset
#define TVEV2_ACT_LINE_OFFSET_4_LINE_OFFSET     4   // 4-line offset
#define TVEV2_ACT_LINE_OFFSET_5_LINE_OFFSET     5   // 5-line offset
#define TVEV2_ACT_LINE_OFFSET_6_LINE_OFFSET     6   // 6-line offset
#define TVEV2_ACT_LINE_OFFSET_7_LINE_OFFSET     7   // 7-line offset

// TVEV2 Luma Filter Control Register 0 (LUMA_FILT_CONT_REG_0)

#define TVEV2_DEFLICK_EN_ENABLE                 1
#define TVEV2_DEFLICK_EN_DISABLE                0

#define TVEV2_DEFLICK_MEAS_WIN_3_PIX_WIN        1
#define TVEV2_DEFLICK_MEAS_WIN_1_PIX_WIN        0

#define TVEV2_DEFLICK_COEF_0_125                0
#define TVEV2_DEFLICK_COEF_0_25                 1
#define TVEV2_DEFLICK_COEF_0_375                2
#define TVEV2_DEFLICK_COEF_0_5                  3
#define TVEV2_DEFLICK_COEF_0_625                4
#define TVEV2_DEFLICK_COEF_0_75                 5
#define TVEV2_DEFLICK_COEF_0_875                6
#define TVEV2_DEFLICK_COEF_1_0                  7

// TVEV2 Luma Filter Control Register 1 (LUMA_FILT_CONT_REG_1)

#define TVEV2_V_SHARP_EN_ENABLE                 1
#define TVEV2_V_SHARP_EN_DISABLE                0

#define TVEV2_V_SHARP_COEF_0_125                0
#define TVEV2_V_SHARP_COEF_0_25                 1
#define TVEV2_V_SHARP_COEF_0_375                2
#define TVEV2_V_SHARP_COEF_0_5                  3
#define TVEV2_V_SHARP_COEF_0_625                4
#define TVEV2_V_SHARP_COEF_0_75                 5
#define TVEV2_V_SHARP_COEF_0_875                6
#define TVEV2_V_SHARP_COEF_1_0                  7

// TVEV2 Luma Filter Control Register 2 (LUMA_FILT_CONT_REG_2)

#define TVEV2_H_SHARP_EN_ENABLE                 1
#define TVEV2_H_SHARP_EN_DISABLE                0

#define TVEV2_H_SHARP_COEF_0_125                0
#define TVEV2_H_SHARP_COEF_0_25                 1
#define TVEV2_H_SHARP_COEF_0_375                2
#define TVEV2_H_SHARP_COEF_0_5                  3
#define TVEV2_H_SHARP_COEF_0_625                4
#define TVEV2_H_SHARP_COEF_0_75                 5
#define TVEV2_H_SHARP_COEF_0_875                6
#define TVEV2_H_SHARP_COEF_1_0                  7

// TVEV2 Luma Filter Control Register 3 (LUMA_FILT_CONT_REG_3)

#define TVEV2_DERING_EN_ENABLE                  1
#define TVEV2_DERING_EN_DISABLE                 0

#define TVEV2_SUPP_FILTER_TYPE_NO_SUPP_FILT     0    // No supplement filter
#define TVEV2_SUPP_FILTER_TYPE_LOWPASS_FILT     1    // Lowpass filter
#define TVEV2_SUPP_FILTER_TYPE_PAL_NOTCH_FILT   2    // PAL notch filter
#define TVEV2_SUPP_FILTER_TYPE_NTSC_NOTCH_FILT  3    // NTSC notch filter

#define TVEV2_DERING_COEF_0_125                 0
#define TVEV2_DERING_COEF_0_25                  1
#define TVEV2_DERING_COEF_0_375                 2
#define TVEV2_DERING_COEF_0_5                   3
#define TVEV2_DERING_COEF_0_625                 4
#define TVEV2_DERING_COEF_0_75                  5
#define TVEV2_DERING_COEF_0_875                 6
#define TVEV2_DERING_COEF_1_0                   7

// Chroma Control Register (CHROMA_CONT_REG)
#define TVEV2_CHROMA_V_FILT_EN_ENABLE           1
#define TVEV2_CHROMA_V_FILT_EN_DISABLE          0

#define TVEV2_CHROMA_BW_0                       0
#define TVEV2_CHROMA_BW_1                       1
#define TVEV2_CHROMA_BW_2                       2
#define TVEV2_CHROMA_BW_3                       3
#define TVEV2_CHROMA_BW_4                       4


// Cable Detection Control Register (CD_CONT_REG)

#define TVEV2_CD_EN_ENABLE                      1
#define TVEV2_CD_EN_DISABLE                     0

#define TVEV2_CD_TRIG_MODE_MANUAL               1
#define TVEV2_CD_TRIG_MODE_AUTO                 0

#define TVEV2_CD_MON_PER_0                      0
#define TVEV2_CD_MON_PER_1                      1
#define TVEV2_CD_MON_PER_2                      2
#define TVEV2_CD_MON_PER_3                      3
#define TVEV2_CD_MON_PER_4                      4
#define TVEV2_CD_MON_PER_5                      5
#define TVEV2_CD_MON_PER_6                      6
#define TVEV2_CD_MON_PER_7                      7
#define TVEV2_CD_MON_PER_8                      8
#define TVEV2_CD_MON_PER_9                      9
#define TVEV2_CD_MON_PER_10                    10
#define TVEV2_CD_MON_PER_11                    11
#define TVEV2_CD_MON_PER_12                    12
#define TVEV2_CD_MON_PER_13                    13
#define TVEV2_CD_MON_PER_14                    14
#define TVEV2_CD_MON_PER_15                    15

#define TVEV2_CD_CH_0_REF_LVL_CHROMA            1
#define TVEV2_CD_CH_0_REF_LVL_LUMA              0

#define TVEV2_CD_CH_1_REF_LVL_CHROMA            1
#define TVEV2_CD_CH_1_REF_LVL_LUMA              0

#define TVEV2_CD_CH_2_REF_LVL_CHROMA            1
#define TVEV2_CD_CH_2_REF_LVL_LUMA              0

#define TVEV2_CD_REF_MODE_MANUAL                1
#define TVEV2_CD_REF_MODE_AUTO                  0

#define TVEV2_CD_CH_0_LM_EN_ENABLE              1
#define TVEV2_CD_CH_0_LM_EN_DISABLE             0

#define TVEV2_CD_CH_1_LM_EN_ENABLE              1
#define TVEV2_CD_CH_1_LM_EN_DISABLE             0

#define TVEV2_CD_CH_2_LM_EN_ENABLE              1
#define TVEV2_CD_CH_2_LM_EN_DISABLE             0

#define TVEV2_CD_CH_0_SM_EN_ENABLE              1
#define TVEV2_CD_CH_0_SM_EN_DISABLE             0

#define TVEV2_CD_CH_1_SM_EN_ENABLE              1
#define TVEV2_CD_CH_1_SM_EN_DISABLE             0

#define TVEV2_CD_CH_2_SM_EN_ENABLE              1
#define TVEV2_CD_CH_2_SM_EN_DISABLE             0

// VBI Data Control Register (VBI_DATA_CONT_REG)

#define TVEV2_CC_SD_F1_EN_ENABLE                1
#define TVEV2_CC_SD_F1_EN_DISABLE               0

#define TVEV2_CC_SD_F2_EN_ENABLE                1
#define TVEV2_CC_SD_F2_EN_DISABLE               0

#define TVEV2_CC_SD_BOOST_EN_ENABLE             1
#define TVEV2_CC_SD_BOOST_EN_DISABLE            0

#define TVEV2_CGMS_SD_F1_EN_ENABLE              1
#define TVEV2_CGMS_SD_F1_EN_DISABLE             0

#define TVEV2_CGMS_SD_F2_EN_ENABLE              1
#define TVEV2_CGMS_SD_F2_EN_DISABLE             0

#define TVEV2_CGMS_SD_SW_CRC_EN_ENABLE          1
#define TVEV2_CGMS_SD_SW_CRC_EN_DISABLE         0

#define TVEV2_WSS_SD_EN_ENABLE                  1
#define TVEV2_WSS_SD_EN_DISABLE                 0

#define TVEV2_CGMS_HD_A_F1_EN_ENABLE            1
#define TVEV2_CGMS_HD_A_F1_EN_DISABLE           0

#define TVEV2_CGMS_HD_A_F2_EN_ENABLE            1
#define TVEV2_CGMS_HD_A_F2_EN_DISABLE           0

#define TVEV2_CGMS_HD_A_SW_CRC_EN_ENABLE        1
#define TVEV2_CGMS_HD_A_SW_CRC_EN_DISABLE       0

#define TVEV2_CGMS_HD_B_F1_EN_ENABLE            1
#define TVEV2_CGMS_HD_B_F1_EN_DISABLE           0

#define TVEV2_CGMS_HD_B_F2_EN_ENABLE            1
#define TVEV2_CGMS_HD_B_F2_EN_DISABLE           0

#define TVEV2_CGMS_HD_B_SW_CRC_EN_ENABLE        1
#define TVEV2_CGMS_HD_B_SW_CRC_EN_DISABLE       0

// Interrupt Control Register (INT_CONT_REG)

#define TVEV2_CD_LM_IEN_ENABLE                  1
#define TVEV2_CD_LM_IEN_DISABLE                 0

#define TVEV2_CD_SM_IEN_ENABLE                  1
#define TVEV2_CD_SM_IEN_DISABLE                 0

#define TVEV2_CD_MON_END_IEN_ENABLE             1
#define TVEV2_CD_MON_END_IEN_DISABLE            0

#define TVEV2_CC_SD_F1_DONE_IEN_ENABLE          1
#define TVEV2_CC_SD_F1_DONE_IEN_DISABLE         0

#define TVEV2_CC_SD_F2_DONE_IEN_ENABLE          1
#define TVEV2_CC_SD_F2_DONE_IEN_DISABLE         0

#define TVEV2_CGMS_SD_F1_DONE_IEN_ENABLE        1
#define TVEV2_CGMS_SD_F1_DONE_IEN_DISABLE       0

#define TVEV2_CGMS_SD_F2_DONE_IEN_ENABLE        1
#define TVEV2_CGMS_SD_F2_DONE_IEN_DISABLE       0

#define TVEV2_WSS_SD_DONE_IEN_ENABLE            1
#define TVEV2_WSS_SD_DONE_IEN_DISABLE           0

#define TVEV2_CGMS_HD_A_F1_DONE_IEN_ENABLE      1
#define TVEV2_CGMS_HD_A_F1_DONE_IEN_DISABLE     0

#define TVEV2_CGMS_HD_A_F2_DONE_IEN_ENABLE      1
#define TVEV2_CGMS_HD_A_F2_DONE_IEN_DISABLE     0

#define TVEV2_CGMS_HD_B_F1_DONE_IEN_ENABLE      1
#define TVEV2_CGMS_HD_B_F1_DONE_IEN_DISABLE     0

#define TVEV2_CGMS_HD_B_F2_DONE_IEN_ENABLE      1
#define TVEV2_CGMS_HD_B_F2_DONE_IEN_DISABLE     0

#define TVEV2_TVE_FIELD_END_IEN_ENABLE          1
#define TVEV2_TVE_FIELD_END_IEN_DISABLE         0

#define TVEV2_TVE_FRAME_END_IEN_ENABLE          1
#define TVEV2_TVE_FRAME_END_IEN_DISABLE         0

#define TVEV2_SA_MEAS_END_IEN_ENABLE            1
#define TVEV2_SA_MEAS_END_IEN_DISABLE           0


// Status Register (STAT_REG)

#define TVEV2_CD_LM_INT_DETECTED                1
#define TVEV2_CD_LM_INT_NODETECTED              0

#define TVEV2_CD_SM_INT_DETECTED                1
#define TVEV2_CD_SM_INT_NODETECTED              0

#define TVEV2_CD_MON_END_INT_YES                1
#define TVEV2_CD_MON_END_INT_NO                 0

#define TVEV2_CC_F1_DONE_IEN_YES                1
#define TVEV2_CC_F1_DONE_IEN_NO                 0

#define TVEV2_CC_F2_DONE_IEN_YES                1
#define TVEV2_CC_F2_DONE_IEN_NO                 0

#define TVEV2_CGMS_F1_WSS_DONE_IEN_YES          1
#define TVEV2_CGMS_F1_DONE_IEN_NO               0

#define TVEV2_CGMS_F2_DONE_IEN_YES              1
#define TVEV2_CGMS_F2_DONE_IEN_NO               0

#define TVEV2_WSS_SD_DONE_IEN_YES               1
#define TVEV2_WSS_SD_DONE_IEN_NO                0

#define TVEV2_CGMS_HD_A_F1_DONE_IEN_YES         1
#define TVEV2_CGMS_HD_A_F1_DONE_IEN_NO          0

#define TVEV2_CGMS_HD_A_F2_DONE_IEN_YES         1
#define TVEV2_CGMS_HD_A_F2_DONE_IEN_NO          0

#define TVEV2_CGMS_HD_B_F1_DONE_IEN_YES         1
#define TVEV2_CGMS_HD_B_F1_DONE_IEN_NO          0

#define TVEV2_CGMS_HD_B_F2_DONE_IEN_YES         1
#define TVEV2_CGMS_HD_B_F2_DONE_IEN_NO          0

#define TVEV2_FIELD_END_INT_YES                 1
#define TVEV2_FIELD_END_INT_NO                  0

#define TVEV2_FRAME_END_INT_YES                 1
#define TVEV2_FRAME_END_INT_NO                  0

#define TVEV2_SA_MEAS_END_INT_YES               1
#define TVEV2_SA_MEAS_END_INT_NO                0

#define TVEV2_CD_CH_0_LM_ST_DETECTED            1  
#define TVEV2_CD_CH_0_LM_ST_NODETECTED          0

#define TVEV2_CD_CH_1_LM_ST_DETECTED            1
#define TVEV2_CD_CH_1_LM_ST_NODETECTED          0

#define TVEV2_CD_CH_2_LM_ST_DETECTED            1
#define TVEV2_CD_CH_2_LM_ST_NODETECTED          0

#define TVEV2_CD_CH_0_SM_ST_DETECTED            1
#define TVEV2_CD_CH_0_SM_ST_NODETECTED          0

#define TVEV2_CD_CH_1_SM_ST_DETECTED            1
#define TVEV2_CD_CH_1_SM_ST_NODETECTED          0

#define TVEV2_CD_CH_2_SM_ST_DETECTED            1
#define TVEV2_CD_CH_2_SM_ST_NODETECTED          0 

#define TVEV2_CD_MAN_TRIG_YES                   1
#define TVEV2_CD_MAN_TRIG_NO                    0

#define TVEV2_BG_READY_YES                      1
#define TVEV2_BG_READY_NO                       0


// Test Mode Register (TST_MODE_REG)

#define TVEV2_TVDAC_TEST_MODE_DISABLE           0  // 000 Disable test mode (functional mode)
#define TVEV2_TVDAC_TEST_MODE_1                 1  // 001 TVDAC test mode 1 - different data on TVDAC channels and UF is in data path
#define TVEV2_TVDAC_TEST_MODE_2                 2  // 010 TVDAC test mode 2 - equal data on TVDAC channels and UF is in data path
#define TVEV2_TVDAC_TEST_MODE_3                 3  // 011 TVDAC test mode 3 - different data on TVDAC channels and UF is bypassed 
#define TVEV2_TVDAC_TEST_MODE_4                 4  // 100 TVDAC test mode 4 - equal data on TVDAC channels and UF is bypassed
#define TVEV2_TVDAC_TEST_MODE_5                 5  // 101 TVDAC test mode 5 - sine wave data from the internal generator directly to all TVDAC channels

#define TVEV2_TVDAC_0_DATA_FORCE_ENABLE         1
#define TVEV2_TVDAC_0_DATA_FORCE_DISABLE        0

#define TVEV2_TVDAC_1_DATA_FORCE_ENABLE         1
#define TVEV2_TVDAC_1_DATA_FORCE_DISABLE        0

#define TVEV2_TVDAC_2_DATA_FORCE_ENABLE         1
#define TVEV2_TVDAC_2_DATA_FORCE_DISABLE        0

#define TVEV2_TVDAC_TEST_SINE_FREQ_DIV_8        0
#define TVEV2_TVDAC_TEST_SINE_FREQ_DIV_16       1
#define TVEV2_TVDAC_TEST_SINE_FREQ_DIV_32       2
#define TVEV2_TVDAC_TEST_SINE_FREQ_DIV_64       3
#define TVEV2_TVDAC_TEST_SINE_FREQ_DIV_128      4
#define TVEV2_TVDAC_TEST_SINE_FREQ_DIV_256      5

#define TVEV2_TVDAC_TEST_SINE_LEVEL_1           0   // 100%
#define TVEV2_TVDAC_TEST_SINE_LEVEL_0_75        1   // 75%
#define TVEV2_TVDAC_TEST_SINE_LEVEL_0_50        2   // 50%
#define TVEV2_TVDAC_TEST_SINE_LEVEL_0_25        3   // 25%

#define TVEV2_COLORBAR_TYPE_1                   0  // 100% 
#define TVEV2_COLORBAR_TYPE_0_75                1  // 75% 


// User Mode Control Register (USER_MODE_CONT_REG)

#define TVEV2_H_TIMING_USR_MODE_EN_ENABLE       1
#define TVEV2_H_TIMING_USR_MODE_EN_DISABLE      0 

#define TVEV2_LUMA_FILT_USR_MODE_EN_ENABLE      1
#define TVEV2_LUMA_FILT_USR_MODE_EN_DISABLE     0 

#define TVEV2_SC_FREQ_USR_MODE_EN_ENABLE        1
#define TVEV2_SC_FREQ_USR_MODE_EN_DISABLE       0 

#define TVEV2_CSCM_COEF_USR_MODE_EN_ENABLE      1
#define TVEV2_CSCM_COEF_USR_MODE_EN_DISABLE     0 

#define TVEV2_BLANK_LEVEL_USR_MODE_EN_ENABLE    1
#define TVEV2_BLANK_LEVEL_USR_MODE_EN_DISABLE   0 

#define TVEV2_VBI_DATA_USR_MODE_EN_ENABLE       1
#define TVEV2_VBI_DATA_USR_MODE_EN_DISABLE      0 

#define TVEV2_TVDAC_DROP_COMP_USR_MODE_EN_ENABLE      1
#define TVEV2_TVDAC_DROP_COMP_USR_MODE_EN_DISABLE     0 


#ifdef __cplusplus
}
#endif

#endif // __COMMON_TVEV2_H
