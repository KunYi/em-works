//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header:  common_tve.h
//
//  Provides definitions for TVE module.
//
//------------------------------------------------------------------------------
#ifndef __COMMON_TVE_H
#define __COMMON_TVE_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------


#define TVE_BG_RDY_TIME_1MS                 1    // 1 ms for TVDAC bandgap reference ready time 
#define TVE_CD_PER_TIME_200MS               200  // 200 ms
   
//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------

#define TVE_COM_CONF_REG_OFFSET                0x0000
#define TVE_FR_POS_CONT_REG_OFFSET             0x0004
#define TVE_TVDAC_0_CONT_REG_OFFSET            0x0008
#define TVE_TVDAC_1_CONT_REG_OFFSET            0x000C
#define TVE_TVDAC_2_CONT_REG_OFFSET            0x0010
#define TVE_CD_CONT_REG_FFSET                  0x0014
#define TVE_CC_CONT_REG_0_OFFSET               0x0018
#define TVE_CC_CONT_REG_1_OFFSET               0x001C
#define TVE_CGMS_WSS_CONT_REG_0_OFFSET         0x0020
#define TVE_CGMS_WSS_CONT_REG_1_OFFSET         0x0024
#define TVE_INT_CONT_REG_OFFSET                0x0028
#define TVE_STAT_REG_OFFSET                    0x002C
#define TVE_TST_MODE_REG_OFFSET                0x0030


//------------------------------------------------------------------------------
// Macro to access the registers
//------------------------------------------------------------------------------

#define  REG32SETBIT(addr,bitpos) \
         OUTREG32((addr),(INREG32((addr)) | (1<<(bitpos))))


//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------

// TVE Registers
typedef struct
{
    UINT32 COM_CONF_REG;        // Common Configuration Register
    UINT32 FR_POS_CONT_REG;     // Frame Position Control Register
    UINT32 TVDAC_0_CONT_REG;    // TVDAC 0 Control Register
    UINT32 TVDAC_1_CONT_REG;    // TVDAC 1 Control Register
    UINT32 TVDAC_2_CONT_REG;    // TVDAC 2 Control Register
    UINT32 CD_CONT_REG;         // Cable Detection Control Register
    UINT32 CC_CONT_REG_0;       // Closed Caption Control Register 0
    UINT32 CC_CONT_REG_1;       // Closed Caption Control Register 1
    UINT32 CGMS_WSS_CONT_REG_0; // Copy Generations Management System and Widescreen Signaling Control Register 0
    UINT32 CGMS_CONT_REG_1;     // Copy Generations Management System Control Register 1
    UINT32 INT_CONT_REG;        // Interrupt Control Register
    UINT32 STAT_REG;            // Status Register
    UINT32 TST_MODE_REG;        // Test Mode Register
    UINT32 RESERVED_REG_0; 
    UINT32 RESERVED_REG_1;
    UINT32 RESERVED_REG_2;
    UINT32 RESERVED_REG_3; 
    UINT32 RESERVED_REG_4;
    UINT32 RESERVED_REG_5;
} TVE_REGS, *PTVE_REGS;

//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------

// COM_CONF_REG
#define TVE_TVE_EN_LSH                          0
#define TVE_TVDAC_SAMP_RATE_LSH                 1
#define TVE_IPU_CLK_EN_LSH                      3
#define TVE_TV_OUT_MODE_LSH                     4
#define TVE_TV_STAND_LSH                        8
#define TVE_R_SYNC_EN_LSH                       12
#define TVE_G_SYNC_EN_LSH                       13
#define TVE_B_SYNC_EN_LSH                       14
#define TVE_YC_DELAY_LSH                        16
#define TVE_PED_EN_LSH                          17
#define TVE_SECAM_FID_EN_LSH                    18
#define TVE_CHROMA_BW_LSH                       20
#define TVE_VBI_CHROMA_EN_LSH                   22
#define TVE_PHASE_RST_EN_LSH                    23
#define TVE_SCH_PHASE_LSH                       24

// FR_POS_CONT_REG
#define TVE_HORIZ_POS_LSH                       0
#define TVE_VERT_POS_LSH                        8

// TVDAC_0_CONT_REG
#define TVE_TVDAC_0_GAIN_LSH                    0
#define TVE_TVDAC_0_OFFSET_LSH                  8
#define TVE_TVDAC_0_DROP_COMP_LSH               20

// TVDAC_1_CONT_REG
#define TVE_TVDAC_1_GAIN_LSH                    0
#define TVE_TVDAC_1_OFFSET_LSH                  8
#define TVE_TVDAC_1_DROP_COMP_LSH               20

// TVDAC_2_CONT_REG
#define TVE_TVDAC_2_GAIN_LSH                    0
#define TVE_TVDAC_2_OFFSET_LSH                  8
#define TVE_TVDAC_2_DROP_COMP_LSH               20

// CD_CONT_REG
#define TVE_CD_EN_LSH                           0
#define TVE_CD_TRIG_MODE_LSH                    1
#define TVE_CD_MON_PER_LSH                      4
#define TVE_CD_CH_0_LM_EN_LSH                   8
#define TVE_CD_CH_1_LM_EN_LSH                   9
#define TVE_CD_CH_2_LM_EN_LSH                   10
#define TVE_CD_CH_0_SM_EN_LSH                   12
#define TVE_CD_CH_1_SM_EN_LSH                   13
#define TVE_CD_CH_2_SM_EN_LSH                   14
#define TVE_CD_CH_0_REF_LVL_LSH                 16
#define TVE_CD_CH_1_REF_LVL_LSH                 17
#define TVE_CD_CH_2_REF_LVL_LSH                 18
#define TVE_CD_REF_MODE_LSH                     19
#define TVE_BG_RDY_TIME_LSH                     24

// CC_CONT_REG_0
#define TVE_CC_F1_EN_LSH                        0
#define TVE_CC_BOOST_EN_LSH                     1
#define TVE_CC_F1_DATA1_LSH                     16
#define TVE_CC_F1_DATA2_LSH                     24

// CC_CONT_REG_1
#define TVE_CC_F2_EN_LSH                        0
#define TVE_CC_F2_DATA1_LSH                     16
#define TVE_CC_F2_DATA2_LSH                     24

// CGMS_CONT_REG_0
#define TVE_CGMS_F1_WSS_EN_LSH                  0
#define TVE_CGMS_SW_CRC_EN_LSH                  1
#define TVE_CGMS_F1_WSS_DATA_LSH                8

// CGMS_CONT_REG_1
#define TVE_CGMS_F2_EN_LSH                      0
#define TVE_CGMS_F2_DATA_LSH                    8

// INT_CONT_REG
#define TVE_CD_LM_IEN_LSH                       0
#define TVE_CD_SM_IEN_LSH                       1
#define TVE_CD_MON_END_IEN_LSH                  2
#define TVE_CC_F1_DONE_IEN_LSH                  4
#define TVE_CC_F2_DONE_IEN_LSH                  5
#define TVE_CGMS_F1_WSS_DONE_IEN_LSH            6
#define TVE_CGMS_F2_DONE_IEN_LSH                7
#define TVE_FRAME_END_IEN_LSH                   24
#define TVE_FIELD_END_IEN_LSH                   25

// STAT_REG
#define TVE_CD_LM_INT_LSH                       0
#define TVE_CD_SM_INT_LSH                       1
#define TVE_CD_MON_END_INT_LSH                  2
#define TVE_CC_F1_DONE_INT_LSH                  4
#define TVE_CC_F2_DONE_INT_LSH                  5
#define TVE_CGMS_F1_WSS_DONE_INT_LSH            6
#define TVE_CGMS_F2_DONE_INT_LSH                7
#define TVE_CD_CH_0_LM_ST_LSH                   8
#define TVE_CD_CH_1_LM_ST_LSH                   9
#define TVE_CD_CH_2_LM_ST_LSH                   10
#define TVE_CD_CH_0_SM_ST_LSH                   12
#define TVE_CD_CH_1_SM_ST_LSH                   13
#define TVE_CD_CH_2_SM_ST_LSH                   14
#define TVE_CD_MAN_TRIG_LSH                     16
#define TVE_BG_READY_LSH                        20
#define TVE_FRAME_END_INT_LSH                   24
#define TVE_FIELD_END_INT_LSH                   25


// TST_MODE_REG
#define TVE_TEST_PAT_EN_LSH                     0
#define TVE_TVDAC_TEST_MODE_LSH                 4
#define TVE_TEST_DATA_SEL_LSH                   8


//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------

// COM_CONF_REG
#define TVE_TVE_EN_WID                          1
#define TVE_TVDAC_SAMP_RATE_WID                 2
#define TVE_IPU_CLK_EN_WID                      1
#define TVE_TV_OUT_MODE_WID                     3
#define TVE_TV_STAND_WID                        3
#define TVE_R_SYNC_EN_WID                       1
#define TVE_G_SYNC_EN_WID                       1
#define TVE_B_SYNC_EN_WID                       1
#define TVE_YC_DELAY_WID                        1
#define TVE_PED_EN_WID                          1
#define TVE_SECAM_FID_EN_WID                    1
#define TVE_CHROMA_BW_WID                       2
#define TVE_VBI_CHROMA_EN_WID                   1
#define TVE_PHASE_RST_EN_WID                    1
#define TVE_SCH_PHASE_WID                       8

// FR_POS_CONT_REG
#define TVE_HORIZ_POS_WID                       8
#define TVE_VERT_POS_WID                        8

// TVDAC_0_CONT_REG
#define TVE_TVDAC_0_GAIN_WID                    6
#define TVE_TVDAC_0_OFFSET_WID                  10
#define TVE_TVDAC_0_DROP_COMP_WID               4

// TVDAC_1_CONT_REG
#define TVE_TVDAC_1_GAIN_WID                    6
#define TVE_TVDAC_1_OFFSET_WID                  10
#define TVE_TVDAC_1_DROP_COMP_WID               4

// TVDAC_2_CONT_REG
#define TVE_TVDAC_2_GAIN_WID                    6
#define TVE_TVDAC_2_OFFSET_WID                  10
#define TVE_TVDAC_2_DROP_COMP_WID               4

// CD_CONT_REG
#define TVE_CD_EN_WID                           1
#define TVE_CD_TRIG_MODE_WID                    1
#define TVE_CD_MON_PER_WID                      4
#define TVE_CD_CH_0_LM_EN_WID                   1
#define TVE_CD_CH_1_LM_EN_WID                   1
#define TVE_CD_CH_2_LM_EN_WID                   1
#define TVE_CD_CH_0_SM_EN_WID                   1
#define TVE_CD_CH_1_SM_EN_WID                   1
#define TVE_CD_CH_2_SM_EN_WID                   1
#define TVE_CD_CH_0_REF_LVL_WID                 1
#define TVE_CD_CH_1_REF_LVL_WID                 1
#define TVE_CD_CH_2_REF_LVL_WID                 1
#define TVE_CD_REF_MODE_WID                     1
#define TVE_BG_RDY_TIME_WID                     8

// CC_CONT_REG_0
#define TVE_CC_F1_EN_WID                        1
#define TVE_CC_BOOST_EN_WID                     1
#define TVE_CC_F1_DATA1_WID                     7
#define TVE_CC_F1_DATA2_WID                     7

// CC_CONT_REG_1
#define TVE_CC_F2_EN_WID                        1
#define TVE_CC_F2_DATA1_WID                     7
#define TVE_CC_F2_DATA2_WID                     7

// CGMS_CONT_REG_0
#define TVE_CGMS_F1_WSS_EN_WID                  1
#define TVE_CMGS_SW_CRC_EN_WID                  1
#define TVE_CGMS_F1_WSS_DATA_WID                22

// CGMS_CONT_REG_1
#define TVE_CGMS_F2_EN_WID                      1
#define TVE_CGMS_F2_DATA_WID                    22

// INT_CONT_REG
#define TVE_CD_LM_IEN_WID                       1
#define TVE_CD_SM_IEN_WID                       1
#define TVE_CD_MON_END_IEN_WID                  1
#define TVE_CC_F1_DONE_IEN_WID                  1
#define TVE_CC_F2_DONE_IEN_WID                  1
#define TVE_CGMS_F1_WSS_DONE_IEN_WID            1
#define TVE_CGMS_F2_DONE_IEN_WID                1
#define TVE_FRAME_END_IEN_WID                   1
#define TVE_FIELD_END_IEN_WID                   1

// STAT_REG
#define TVE_CD_LM_INT_WID                       1
#define TVE_CD_SM_INT_WID                       1
#define TVE_CD_MON_END_INT_WID                  1
#define TVE_CC_F1_DONE_INT_WID                  1
#define TVE_CC_F2_DONE_INT_WID                  1
#define TVE_CGMS_F1_WSS_DONE_WID                1
#define TVE_CGMS_F2_DONE_WID                    1
#define TVE_CD_CH_0_LM_ST_WID                   1
#define TVE_CD_CH_1_LM_ST_WID                   1
#define TVE_CD_CH_2_LM_ST_WID                   1
#define TVE_CD_CH_0_SM_ST_WID                   1
#define TVE_CD_CH_1_SM_ST_WID                   1
#define TVE_CD_CH_2_SM_ST_WID                   1
#define TVE_CD_MAN_TRIG_WID                     1
#define TVE_BG_READY_WID                        1
#define TVE_FRAME_END_INT_WID                   1
#define TVE_FIELD_END_INT_WID                   1


// TST_MODE_REG
#define TVE_TEST_PAT_EN_WID                     1
#define TVE_TVDAC_TEST_MODE_WID                 3
#define TVE_TEST_DATA_SEL_WID                   1


//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------
#define TVE_ENABLE                              1
#define TVE_DISABLE                             0

// TVE Common Configuration Register (COM_CONF_REG)

#define TVE_TVE_EN_ENABLE                       1
#define TVE_TVE_EN_DISABLE                      0

#define TVE_TVDAC_SAMPRATE_216MHZ               0   // 00  216MHz
#define TVE_TVDAC_SAMPRATE_108MHZ               1   // 01  108MHZ
#define TVE_TVDAC_SAMPRATE_54MHZ                2   // 10  54MHz
#define TVE_TVDAC_SAMPRATE_27MHZ                3   // 11  27MHz
                       
#define TVE_IPU_CLK_EN_ENABLE                   1
#define TVE_IPU_CLK_EN_DISABLE                  0

//#define TVE_TV_OUT_MODE_DISABLE                 0    // 000 Disable output 
//#define TVE_TV_OUT_MODE_CVBS_CH0                1    // 001 CVBS on channel #0 
//#define TVE_TV_OUT_MODE_CVBS_CH2                2    // 002 CVBS on channel #2 
//#define TVE_TV_OUT_MODE_CVBS_CH0_CH2            3    // 011 CVBS on both channels #0 and #2 
//#define TVE_TV_OUT_MODE_S_CH0_CH1               4    // 100 S-video on channels #0 and #1 
//#define TVE_TV_OUT_MODE_S_CH0_CH1_CVBS_CH2      5    // 101 S-video on channels #0 and #1 and CVBS on channel #2 
//#define TVE_TV_OUT_MODE_YPRPB                   6    // 110 YPrPb component 
//#define TVE_TV_OUT_MODE_RGB                     7    // 111 RGB component 

//#define TVE_TV_STAND_NTSC                       0    // 000 NTSC
//#define TVE_TV_STAND_PALM                       1    // 001 PALM
//#define TVE_TV_STAND_PALN                       2    // 010 Combination PALN
//#define TVE_TV_STAND_PAL                        3    // 011 PAL (B,D,G,H,I)
//#define TVE_TV_STAND_SECAM                      4    // 1xx SECAM


#define TVE_R_SYNC_EN_ENABLE                    1
#define TVE_R_SYNC_EN_DISABLE                   0

#define TVE_G_SYNC_EN_ENABLE                    1
#define TVE_G_SYNC_EN_DISABLE                   0

#define TVE_B_SYNC_EN_ENABLE                    1
#define TVE_B_SYNC_EN_DISABLE                   0

#define TVE_YC_DELAY_ENABLE                     1
#define TVE_YC_DELAY_DISABLE                    0

#define TVE_PED_EN_ENABLE                       1
#define TVE_PED_EN_DISABLE                      0

#define TVE_SECAM_FID_EN_ENABLE                 1
#define TVE_SECAM_FID_EN_DISABLE                0

#define TVE_CHROMA_BW_1P3MHZ                    0 // 00  1.3 MHz (default)
#define TVE_CHROMA_BW_0P78MHZ                   1 // 01  0.78 MHz
#define TVE_CHROMA_BW_3P4MHZ                    2 // 10  3.4 MHz


#define TVE_VBI_CHROMA_EN_ENABLE                1
#define TVE_VBI_CHROMA_EN_DISABLE               0

#define TVE_PHASE_RST_EN_ENABLE                 1
#define TVE_PHASE_RST_EN_DISABLE                0

// Cable Detection Control Register (CD_CONT_REG)

#define TVE_CD_EN_ENABLE                        1
#define TVE_CD_EN_DISABLE                       0

#define TVE_CD_TRIG_MODE_MANUAL                 1
#define TVE_CD_TRIG_MODE_AUTO                   0

#define TVE_CD_CH_0_LM_EN_ENABLE                1
#define TVE_CD_CH_0_LM_EN_DISABLE               0

#define TVE_CD_CH_1_LM_EN_ENABLE                1
#define TVE_CD_CH_1_LM_EN_DISABLE               0

#define TVE_CD_CH_2_LM_EN_ENABLE                1
#define TVE_CD_CH_2_LM_EN_DISABLE               0

#define TVE_CD_CH_0_SM_EN_ENABLE                1
#define TVE_CD_CH_0_SM_EN_DISABLE               0

#define TVE_CD_CH_1_SM_EN_ENABLE                1
#define TVE_CD_CH_1_SM_EN_DISABLE               0

#define TVE_CD_CH_2_SM_EN_ENABLE                1
#define TVE_CD_CH_2_SM_EN_DISABLE               0

#define TVE_CD_CH_0_REF_LVL_CHROMA              1
#define TVE_CD_CH_0_REF_LVL_LUMA                0

#define TVE_CD_CH_1_REF_LVL_CHROMA              1
#define TVE_CD_CH_1_REF_LVL_LUMA                0

#define TVE_CD_CH_2_REF_LVL_CHROMA              1
#define TVE_CD_CH_2_REF_LVL_LUMA                0

#define TVE_CD_REF_MODE_MANUAL                  1
#define TVE_CD_REF_MODE_AUTO                    0


// Closed Caption Control Register 0 (CC_CONT_REG_0)

#define TVE_CC_F1_EN_ENABLE                     1
#define TVE_CC_F1_EN_DISABLE                    0

#define TVE_CC_BOOST_EN_ENABLE                  1
#define TVE_CC_BOOST_EN_DISABLE                 0

// Closed Caption Control Register 1 (CC_CONT_REG_1)

#define TVE_CC_F2_EN_ENABLE                     1
#define TVE_CC_F2_EN_DISABLE                    0

// Copy Generation Management System and Widescreen Signaling Control Register 0
// ( CGMS_WSS_CONT_REG_0 )

#define TVE_CGMS_F1_WSS_EN_ENABLE               1
#define TVE_CGMS_F1_WSS_EN_DISABLE              0

#define TVE_CGMS_SW_CRC_EN_ENABLE               1
#define TVE_CGMS_SW_CRC_EN_DISABLE              0

// Copy Generation Management System and Widescreen Signaling Control Register 1
// ( CGMS_WSS_CONT_REG_1 )

#define TVE_CGMS_F2_EN_ENABLE                   1
#define TVE_CGMS_F2_EN_DISABLE                  0

// Interrupt Control Register (INT_CONT_REG)

#define TVE_CD_LM_IEN_ENABLE                    1
#define TVE_CD_LM_IEN_DISABLE                   0

#define TVE_CD_SM_IEN_ENABLE                    1
#define TVE_CD_SM_IEN_DISABLE                   0

#define TVE_CD_MON_END_IEN_ENABLE               1
#define TVE_CD_MON_END_IEN_DISABLE              0

#define TVE_CC_F1_DONE_IEN_ENABLE               1
#define TVE_CC_F1_DONE_IEN_DISABLE              0

#define TVE_CC_F2_DONE_IEN_ENABLE               1
#define TVE_CC_F2_DONE_IEN_DISABLE              0

#define TVE_CGMS_F1_WSS_DONE_IEN_ENABLE         1
#define TVE_CGMS_F1_WSS_DONE_IEN_DISABLE        0

#define TVE_CGMS_F2_DONE_IEN_ENABLE             1
#define TVE_CGMS_F2_DONE_IEN_DISABLE            0

#define TVE_FRAME_END_IEN_ENABLE                1
#define TVE_FRAME_END_IEN_DISABLE               0

#define TVE_FIELD_END_IEN_ENABLE                1
#define TVE_FIELD_END_IEN_DISABLE               0


// Status Register (STAT_REG)

#define TVE_CD_LM_INT_YES                       1
#define TVE_CD_LM_INT_NO                        0

#define TVE_CD_SM_INT_YES                       1
#define TVE_CD_SM_INT_NO                        0

#define TVE_CD_MON_END_INT_YES                  1
#define TVE_CD_MON_END_INT_NO                   0

#define TVE_CC_F1_DONE_INT_YES                  1
#define TVE_CC_F1_DONE_INT_NO                   0

#define TVE_CC_F2_DONE_INT_YES                  1
#define TVE_CC_F2_DONE_INT_NO                   0

#define TVE_CGMS_F1_WSS_DONE_INT_YES            1
#define TVE_CGMS_F1_DONE_INT_NO                 0

#define TVE_CGMS_F2_WSS_DONE_INT_YES            1
#define TVE_CGMS_F2_WSS_DONE_INT_NO             0

#define TVE_CD_CH_0_LM_ST_YES                   1  
#define TVE_CD_CH_0_LM_ST_NO                    0

#define TVE_CD_CH_1_LM_ST_YES                   1
#define TVE_CD_CH_1_LM_ST_NO                    0

#define TVE_CD_CH_2_LM_ST_YES                   1
#define TVE_CD_CH_2_LM_ST_NO                    0

#define TVE_CD_CH_0_SM_ST_YES                   1
#define TVE_CD_CH_0_SM_ST_NO                    0

#define TVE_CD_CH_1_SM_ST_YES                   1
#define TVE_CD_CH_1_SM_ST_NO                    0

#define TVE_CD_CH_2_SM_ST_YES                   1
#define TVE_CD_CH_2_SM_ST_NO                    0 

#define TVE_CD_MAN_TRIG_YES                     1
#define TVE_CD_MAN_TRIG_NO                      0

#define TVE_BG_READY_YES                        1
#define TVE_BG_READY_NO                         0

#define TVE_FRAME_END_INT_YES                   1
#define TVE_FRAME_END_INT_NO                    0

#define TVE_FIELD_END_INT_YES                   1
#define TVE_FIELD_END_INT_NO                    0

//------------------------------------------------------------------------------
// Interrupt Bitmask    
//------------------------------------------------------------------------------
#define INT_CD_LM               (1<<0)
#define INT_CD_SM               (1<<1)
#define INT_CD_MON_END          (1<<2)
#define INT_CC_F1_DONE          (1<<4)
#define INT_CC_F2_DONE          (1<<5)
#define INT_CGMS_F1_WSS_DONE    (1<<6)
#define INT_CGMS_F2_DONE        (1<<7)
#define INT_CD_CH_0_LM_ST       (1<<8)
#define INT_CD_CH_1_LM_ST       (1<<9)
#define INT_CD_CH_2_LM_ST       (1<<10)
#define INT_CD_CH_0_SM_ST       (1<<12)
#define INT_CD_CH_1_SM_ST       (1<<13)
#define INT_CD_CH_2_SM_ST       (1<<14)
#define INT_CD_MAN_TRIG         (1<<16)
#define INT_TVE_FRAME_END       (1<<24)
#define INT_TVE_FIELD_END       (1<<25)



// Test Mode Register (TST_MODE_REG)

#define TVE_TEST_PAT_EN_ENABLE                  1
#define TVE_TEST_PAT_EN_DISABLE                 0

#define TVE_TVDAC_TEST_MODE_DISABLE             0  // 000 Disable test mode (functional mode)
//#define TVE_TVDAC_TEST_MODE_1                   1  // 001 TVDAC test mode 1 - different data on TVDAC channels and UF is in data path
//#define TVE_TVDAC_TEST_MODE_2                   2  // 010 TVDAC test mode 2 - equal data on TVDAC channels and UF is in data path
//#define TVE_TVDAC_TEST_MODE_3                   3  // 011 TVDAC test mode 3 - different data on TVDAC channels and UF is bypassed 
//#define TVE_TVDAC_TEST_MODE_4                   4  // 100 TVDAC test mode 4 - equal data on TVDAC channels and UF is bypassed

#define TVE_TEST_DATA_SEL_ENABLE                1
#define TVE_TEST_DATA_SEL_DISABLE               0


#ifdef __cplusplus
}
#endif

#endif // __COMMON_TVE_H
