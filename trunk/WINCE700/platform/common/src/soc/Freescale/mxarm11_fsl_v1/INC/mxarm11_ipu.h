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
//  Copyright (C) 2004-2007, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header:  mxarm11_ipu.h
//
//  Provides definitions for IPU module based on Freescale ARM11 chassis.
//
//------------------------------------------------------------------------------
#ifndef __MXARM11_IPU_H
#define __MXARM11_IPU_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------

// Mutex to protect the integrity of the IPU common registers
#define IPU_COMMON_MUTEX                        TEXT("IPU_COMMON_REGISTERS")

//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------
typedef struct
{
    // IPU Common Registers
    UINT32 IPU_CONF;
    UINT32 IPU_CHA_BUF0_RDY;
    UINT32 IPU_CHA_BUF1_RDY;
    UINT32 IPU_CHA_DB_MODE_SEL;
    UINT32 IPU_CHA_CUR_BUF;
    UINT32 IPU_FS_PROC_FLOW;
    UINT32 IPU_FS_DISP_FLOW;
    UINT32 IPU_TASKS_STAT;
    UINT32 IPU_IMA_ADDR;
    UINT32 IPU_IMA_DATA;
    UINT32 IPU_INT_CTRL_1;
    UINT32 IPU_INT_CTRL_2;
    UINT32 IPU_INT_CTRL_3;
    UINT32 IPU_INT_CTRL_4;
    UINT32 IPU_INT_CTRL_5;
    UINT32 IPU_INT_STAT_1;
    UINT32 IPU_INT_STAT_2;
    UINT32 IPU_INT_STAT_3;
    UINT32 IPU_INT_STAT_4;
    UINT32 IPU_INT_STAT_5;
    UINT32 IPU_BRK_CTRL_1;
    UINT32 IPU_BRK_CTRL_2;
    UINT32 IPU_BRK_STAT;
    UINT32 IPU_DIAGB_CTRL;

    // IPU CSI Registers
    UINT32 CSI_SENS_CONF;
    UINT32 CSI_SENS_FRM_SIZE;
    UINT32 CSI_ACT_FRM_SIZE;
    UINT32 CSI_OUT_FRM_CTRL;
    UINT32 CSI_TST_CTRL;
    UINT32 CSI_CCIR_CODE_1;
    UINT32 CSI_CCIR_CODE_2;
    UINT32 CSI_CCIR_CODE_3;
    UINT32 CSI_FLASH_STROBE_1;
    UINT32 CSI_FLASH_STROBE_2;

    // IPU IC Registers
    UINT32 IC_CONF;
    UINT32 IC_PRP_ENC_RSC;
    UINT32 IC_PRP_VF_RSC;
    UINT32 IC_PP_RSC;
    UINT32 IC_CMBP_1;
    UINT32 IC_CMBP_2;

    // IPU PF Registers
    UINT32 PF_CONF;

    // IPU IDMAC Registers
    UINT32 IDMAC_CONF;
    UINT32 IDMAC_CHA_EN;
    UINT32 IDMAC_CHA_PRI;
    UINT32 IDMAC_CHA_BUSY;

    // IPU SDC (Synchronous Display Controller) Registers
    UINT32 SDC_COM_CONF;
    UINT32 SDC_GRAPH_WIND_CTRL;
    UINT32 SDC_FG_POS;
    UINT32 SDC_BG_POS;
    UINT32 SDC_CUR_POS;
    UINT32 SDC_CUR_BLINK_PWM_CTRL;
    UINT32 SDC_CUR_MAP;
    UINT32 SDC_HOR_CONF;
    UINT32 SDC_VER_CONF;
    UINT32 SDC_SHARP_CONF_1;
    UINT32 SDC_SHARP_CONF_2;

    // IPU ADC (Asynchronous Display Controller) Registers
    UINT32 ADC_CONF;
    UINT32 ADC_SYSCHA1_SA;
    UINT32 ADC_SYSCHA2_SA;
    UINT32 ADC_PRPCHAN_SA;
    UINT32 ADC_PPCHAN_SA;
    UINT32 ADC_DISP0_CONF;
    UINT32 ADC_DISP0_RD_AP;
    UINT32 ADC_DISP0_RDM;
    UINT32 ADC_DISP0_SS;
    UINT32 ADC_DISP1_CONF;
    UINT32 ADC_DISP1_RD_AP;
    UINT32 ADC_DISP1_RDM;
    UINT32 ADC_DISP12_SS;
    UINT32 ADC_DISP2_CONF;
    UINT32 ADC_DISP2_RD_AP;
    UINT32 ADC_DISP2_RDM;
    UINT32 ADC_DISP_VSYNC;

    // IPU DI Registers
    UINT32 DI_DISP_IF_CONF;
    UINT32 DI_DISP_SIG_POL;
    UINT32 DI_SER_DISP1_CONF;
    UINT32 DI_SER_DISP2_CONF;
    UINT32 DI_HSP_CLK_PER;
    UINT32 DI_DISP0_TIME_CONF_1;
    UINT32 DI_DISP0_TIME_CONF_2;
    UINT32 DI_DISP0_TIME_CONF_3;
    UINT32 DI_DISP1_TIME_CONF_1;
    UINT32 DI_DISP1_TIME_CONF_2;
    UINT32 DI_DISP1_TIME_CONF_3;
    UINT32 DI_DISP2_TIME_CONF_1;
    UINT32 DI_DISP2_TIME_CONF_2;
    UINT32 DI_DISP2_TIME_CONF_3;
    UINT32 DI_DISP3_TIME_CONF;
    UINT32 DI_DISP0_DB0_MAP;
    UINT32 DI_DISP0_DB1_MAP;
    UINT32 DI_DISP0_DB2_MAP;
    UINT32 DI_DISP0_CB0_MAP;
    UINT32 DI_DISP0_CB1_MAP;
    UINT32 DI_DISP0_CB2_MAP;
    UINT32 DI_DISP1_DB0_MAP;
    UINT32 DI_DISP1_DB1_MAP;
    UINT32 DI_DISP1_DB2_MAP;
    UINT32 DI_DISP1_CB0_MAP;
    UINT32 DI_DISP1_CB1_MAP;
    UINT32 DI_DISP1_CB2_MAP;
    UINT32 DI_DISP2_DB0_MAP;
    UINT32 DI_DISP2_DB1_MAP;
    UINT32 DI_DISP2_DB2_MAP;
    UINT32 DI_DISP2_CB0_MAP;
    UINT32 DI_DISP2_CB1_MAP;
    UINT32 DI_DISP2_CB2_MAP;
    UINT32 DI_DISP3_B0_MAP;
    UINT32 DI_DISP3_B1_MAP;
    UINT32 DI_DISP3_B2_MAP;
    UINT32 DI_DISP_ACC_CC;
    UINT32 DI_DISP_LLA_CONF;
    UINT32 DI_DISP_LLA_DATA;
} CSP_IPU_REGS, *PCSP_IPU_REGS;

//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------

// IPU Common Registers
#define IPU_IPU_CONF_OFFSET                 0x0000
#define IPU_IPU_CHA_BUF0_RDY_OFFSET         0x0004
#define IPU_IPU_CHA_BUF1_RDY_OFFSET         0x0008
#define IPU_IPU_CHA_DB_MODE_SEL_OFFSET      0x000C
#define IPU_IPU_CHA_CUR_BUF_OFFSET          0x0010
#define IPU_IPU_FS_PROC_FLOW_OFFSET         0x0014
#define IPU_IPU_FS_DISP_FLOW_OFFSET         0x0018
#define IPU_IPU_TASKS_STAT_OFFSET           0x001C
#define IPU_IPU_IMA_ADDR_OFFSET             0x0020
#define IPU_IPU_IMA_DATA_OFFSET             0x0024
#define IPU_IPU_INT_CTRL_1_OFFSET           0x0028
#define IPU_IPU_INT_CTRL_2_OFFSET           0x002C
#define IPU_IPU_INT_CTRL_3_OFFSET           0x0030
#define IPU_IPU_INT_CTRL_4_OFFSET           0x0034
#define IPU_IPU_INT_CTRL_5_OFFSET           0x0038
#define IPU_IPU_INT_STAT_1_OFFSET           0x003C
#define IPU_IPU_INT_STAT_2_OFFSET           0x0040
#define IPU_IPU_INT_STAT_3_OFFSET           0x0044
#define IPU_IPU_INT_STAT_4_OFFSET           0x0048
#define IPU_IPU_INT_STAT_5_OFFSET           0x004C
#define IPU_IPU_BRK_CTRL_1_OFFSET           0x0050
#define IPU_IPU_BRK_CTRL_2_OFFSET           0x0054
#define IPU_IPU_BRK_STAT_OFFSET             0x0058
#define IPU_IPU_DIAGB_CTRL_OFFSET           0x005C

// IPU CSI Registers
#define IPU_CSI_SENS_CONF_OFFSET            0x0060
#define IPU_CSI_SENS_FRM_SIZE_OFFSET        0x0064
#define IPU_CSI_ACT_FRM_SIZE_OFFSET         0x0068
#define IPU_CSI_OUT_FRM_CTRL_OFFSET         0x006C
#define IPU_CSI_TST_CTRL_OFFSET             0x0070
#define IPU_CSI_CCIR_CODE_1_OFFSET          0x0074
#define IPU_CSI_CCIR_CODE_2_OFFSET          0x0078
#define IPU_CSI_CCIR_CODE_3_OFFSET          0x007C
#define CSI_FLASH_STROBE_1_OFFSET           0x0080
#define CSI_FLASH_STROBE_2_OFFSET           0x0084


// IPU IC Registers
#define IPU_IC_CONF_OFFSET                  0x0088
#define IPU_IC_PRP_ENC_RSC_OFFSET           0x008C
#define IPU_IC_PRP_VF_RSC_OFFSET            0x0090
#define IPU_IC_PP_RSC_OFFSET                0x0094
#define IPU_IC_CMBP_1_OFFSET                0x0098
#define IPU_IC_CMBP_2_OFFSET                0x009C

// IPU PF Registers
#define IPU_PF_CONF_OFFSET                  0x00A0

// IPU IDMAC Registers
#define IPU_IDMAC_CONF_OFFSET               0x00A4
#define IPU_IDMAC_CHA_EN_OFFSET             0x00A8
#define IPU_IDMAC_CHA_PRI_OFFSET            0x00AC
#define IPU_IDMAC_CHA_BUSY_OFFSET           0x00B0

// IPU SDC Synchronous Display Controller) Registers
#define IPU_SDC_COM_CONF_OFFSET             0x00B4
#define IPU_SDC_GRAPH_WIND_CTRL_OFFSET      0x00B8
#define IPU_SDC_FG_POS_OFFSET               0x00BC
#define IPU_SDC_BG_POS_OFFSET               0x00C0
#define IPU_SDC_CUR_POS_OFFSET              0x00C4
#define IPU_SDC_CUR_BLINK_PWM_CTRL_OFFSET   0x00C8
#define IPU_SDC_CUR_MAP_OFFSET              0x00CC
#define IPU_SDC_HOR_CONF_OFFSET             0x00D0
#define IPU_SDC_VER_CONF_OFFSET             0x00D4
#define IPU_SDC_SHARP_CONF_1_OFFSET         0x00D8
#define IPU_SDC_SHARP_CONF_2_OFFSET         0x00DC

// IPU ADC (Asynchronous Display Controller) Registers
#define IPU_ADC_CONF_OFFSET                 0x00E0
#define IPU_ADC_SYSCHA1_SA_OFFSET           0x00E4
#define IPU_ADC_SYSCHA2_SA_OFFSET           0x00E8
#define IPU_ADC_PRPCHAN_SA_OFFSET           0x00EC
#define IPU_ADC_PPCHAN_SA_OFFSET            0x00F0
#define IPU_ADC_DISP0_CONF_OFFSET           0x00F4
#define IPU_ADC_DISP0_RD_AP_OFFSET          0x00F8
#define IPU_ADC_DISP0_RDM_OFFSET            0x00FC
#define IPU_ADC_DISP0_SS_OFFSET             0x0100
#define IPU_ADC_DISP1_CONF_OFFSET           0x0104
#define IPU_ADC_DISP1_RD_AP_OFFSET          0x0108
#define IPU_ADC_DISP1_RDM_OFFSET            0x010C
#define IPU_ADC_DISP12_SS_OFFSET            0x0110
#define IPU_ADC_DISP2_CONF_OFFSET           0x0114
#define IPU_ADC_DISP2_RD_AP_OFFSET          0x0118
#define IPU_ADC_DISP2_RDM_OFFSET            0x011C
#define IPU_ADC_DISP2_SS_OFFSET             0x0120

// IPU DI Registers
#define IPU_DI_DISP_IF_CONF_OFFSET          0x0124
#define IPU_DI_DISP_SIG_POL_OFFSET          0x0128
#define IPU_DI_SER_DISP1_CONF_OFFSET        0x012C
#define IPU_DI_SER_DISP2_CONF_OFFSET        0x0130
#define IPU_DI_HSP_CLK_PER_OFFSET           0x0134
#define IPU_DI_DISP0_TIME_CONF_1_OFFSET     0x0138
#define IPU_DI_DISP0_TIME_CONF_2_OFFSET     0x013C
#define IPU_DI_DISP0_TIME_CONF_3_OFFSET     0x0140
#define IPU_DI_DISP1_TIME_CONF_1_OFFSET     0x0144
#define IPU_DI_DISP1_TIME_CONF_2_OFFSET     0x0148
#define IPU_DI_DISP1_TIME_CONF_3_OFFSET     0x014C
#define IPU_DI_DISP2_TIME_CONF_1_OFFSET     0x0150
#define IPU_DI_DISP2_TIME_CONF_2_OFFSET     0x0154
#define IPU_DI_DISP2_TIME_CONF_3_OFFSET     0x0158
#define IPU_DI_DISP3_TIME_CONF_OFFSET       0x015C
#define IPU_DI_DISP0_DB0_MAP_OFFSET         0x0160
#define IPU_DI_DISP0_DB1_MAP_OFFSET         0x0164
#define IPU_DI_DISP0_DB2_MAP_OFFSET         0x0168
#define IPU_DI_DISP0_CB0_MAP_OFFSET         0x016C
#define IPU_DI_DISP0_CB1_MAP_OFFSET         0x0170
#define IPU_DI_DISP0_CB2_MAP_OFFSET         0x0174
#define IPU_DI_DISP1_DB0_MAP_OFFSET         0x0178
#define IPU_DI_DISP1_DB1_MAP_OFFSET         0x017C
#define IPU_DI_DISP1_DB2_MAP_OFFSET         0x0180
#define IPU_DI_DISP1_CB0_MAP_OFFSET         0x0184
#define IPU_DI_DISP1_CB1_MAP_OFFSET         0x0188
#define IPU_DI_DISP1_CB2_MAP_OFFSET         0x018C
#define IPU_DI_DISP2_DB0_MAP_OFFSET         0x0190
#define IPU_DI_DISP2_DB1_MAP_OFFSET         0x0194
#define IPU_DI_DISP2_DB2_MAP_OFFSET         0x0198
#define IPU_DI_DISP2_CB0_MAP_OFFSET         0x019C
#define IPU_DI_DISP2_CB1_MAP_OFFSET         0x01A0
#define IPU_DI_DISP2_CB2_MAP_OFFSET         0x01A4
#define IPU_DI_DISP3_B0_MAP_OFFSET          0x01A8
#define IPU_DI_DISP3_B1_MAP_OFFSET          0x01AC
#define IPU_DI_DISP3_B2_MAP_OFFSET          0x01B0
#define IPU_DI_DISP_ACC_CC_OFFSET           0x01B4
#define IPU_DI_DISP_LLA_CONF_OFFSET         0x01B8
#define IPU_DI_DISP_LLA_DATA_OFFSET         0x01BC

//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------

// IPU_CONF
#define IPU_IPU_CONF_CSI_EN_LSH                 0
#define IPU_IPU_CONF_IC_EN_LSH                  1
#define IPU_IPU_CONF_ROT_EN_LSH                 2
#define IPU_IPU_CONF_PF_EN_LSH                  3
#define IPU_IPU_CONF_SDC_EN_LSH                 4
#define IPU_IPU_CONF_ADC_EN_LSH                 5
#define IPU_IPU_CONF_DI_EN_LSH                  6
#define IPU_IPU_CONF_DU_EN_LSH                  7
#define IPU_IPU_CONF_PXL_ENDIAN_LSH             8

// IPU DMA Channel ID used in the various IPU registers
#define IPU_DMA_CHA_DMAIC_0_LSH                 0
#define IPU_DMA_CHA_DMAIC_1_LSH                 1
#define IPU_DMA_CHA_DMAADC_0_LSH                1
#define IPU_DMA_CHA_DMAIC_2_LSH                 2
#define IPU_DMA_CHA_DMAADC_1_LSH                2
#define IPU_DMA_CHA_DMAIC_3_LSH                 3
#define IPU_DMA_CHA_DMAIC_4_LSH                 4
#define IPU_DMA_CHA_DMAIC_5_LSH                 5
#define IPU_DMA_CHA_DMAIC_6_LSH                 6
#define IPU_DMA_CHA_DMAIC_7_LSH                 7
#define IPU_DMA_CHA_DMAIC_8_LSH                 8
#define IPU_DMA_CHA_DMAIC_9_LSH                 9
#define IPU_DMA_CHA_DMAIC_10_LSH                10
#define IPU_DMA_CHA_DMAIC_11_LSH                11
#define IPU_DMA_CHA_DMAIC_12_LSH                12
#define IPU_DMA_CHA_DMAIC_13_LSH                13
#define IPU_DMA_CHA_DMASDC_0_LSH                14
#define IPU_DMA_CHA_DMASDC_1_LSH                15
#define IPU_DMA_CHA_DMASDC_2_LSH                16
#define IPU_DMA_CHA_DMASDC_3_LSH                17
#define IPU_DMA_CHA_DMAADC_2_LSH                18
#define IPU_DMA_CHA_DMAADC_3_LSH                19
#define IPU_DMA_CHA_DMAADC_4_LSH                20
#define IPU_DMA_CHA_DMAADC_5_LSH                21
#define IPU_DMA_CHA_DMAADC_6_LSH                22
#define IPU_DMA_CHA_DMAADC_7_LSH                23
#define IPU_DMA_CHA_DMAPF_0_LSH                 24
#define IPU_DMA_CHA_DMAPF_1_LSH                 25
#define IPU_DMA_CHA_DMAPF_2_LSH                 26
#define IPU_DMA_CHA_DMAPF_3_LSH                 27
#define IPU_DMA_CHA_DMAPF_4_LSH                 28
#define IPU_DMA_CHA_DMAPF_5_LSH                 29
#define IPU_DMA_CHA_DMAPF_6_LSH                 30
#define IPU_DMA_CHA_DMAPF_7_LSH                 31

// IPU_FS_PROC_FLOW
#define IPU_IPU_FS_PROC_FLOW_ENC_IN_VALID_LSH       0
#define IPU_IPU_FS_PROC_FLOW_VF_IN_VALID_LSH        1
#define IPU_IPU_FS_PROC_FLOW_PRPENC_DEST_SEL_LSH    4
#define IPU_IPU_FS_PROC_FLOW_PRPENC_ROT_SRC_SEL_LSH 5
#define IPU_IPU_FS_PROC_FLOW_PRPVF_ROT_SRC_SEL_LSH  6
#define IPU_IPU_FS_PROC_FLOW_PP_SRC_SEL_LSH         8
#define IPU_IPU_FS_PROC_FLOW_PP_ROT_SRC_SEL_LSH     10
#define IPU_IPU_FS_PROC_FLOW_PF_DEST_SEL_LSH        12
#define IPU_IPU_FS_PROC_FLOW_PRPVF_DEST_SEL_LSH     16
#define IPU_IPU_FS_PROC_FLOW_PRPVF_ROT_DEST_SEL_LSH 20
#define IPU_IPU_FS_PROC_FLOW_PP_DEST_SEL_LSH        24
#define IPU_IPU_FS_PROC_FLOW_PP_ROT_DEST_SEL_LSH    28

// IPU_FS_DISP_FLOW
#define IPU_IPU_FS_DISP_FLOW_SDC0_SRC_SEL_LSH   0
#define IPU_IPU_FS_DISP_FLOW_SDC1_SRC_SEL_LSH   4
#define IPU_IPU_FS_DISP_FLOW_ADC2_SRC_SEL_LSH   8
#define IPU_IPU_FS_DISP_FLOW_ADC3_SRC_SEL_LSH   12
#define IPU_IPU_FS_DISP_FLOW_AUTO_REF_PER_LSH   16

// IPU_TASKS_STAT
#define IPU_IPU_TASKS_STAT_ENC_TSTAT_LSH        7
#define IPU_IPU_TASKS_STAT_VF_TSTAT_LSH         9
#define IPU_IPU_TASKS_STAT_PP_TSTAT_LSH         11
#define IPU_IPU_TASKS_STAT_ENC_ROT_TSTAT_LSH    16
#define IPU_IPU_TASKS_STAT_VF_ROT_TSTAT_LSH     18
#define IPU_IPU_TASKS_STAT_PP_ROT_TSTAT_LSH     20
#define IPU_IPU_TASKS_STAT_ADCSYS1_TSTAT_LSH    24
#define IPU_IPU_TASKS_STAT_ADCSYS2_TSTAT_LSH    26
#define IPU_IPU_TASKS_STAT_ADC_PRPCHAN_LOCK_LSH 28
#define IPU_IPU_TASKS_STAT_ADC_PPCHAN_LOCK_LSH  29
#define IPU_IPU_TASKS_STAT_ADC_SYS1CHAN_LOCK_LSH 30
#define IPU_IPU_TASKS_STAT_ADC_SYS2CHAN_LOCK_LSH 31

// IPU_IMA_ADDR
#define IPU_IPU_IMA_ADDR_WORD_NU_LSH            0
#define IPU_IPU_IMA_ADDR_ROW_NU_LSH             3
#define IPU_IPU_IMA_ADDR_MEM_NU_LSH             16


// IPU_IMA_DATA 

//...parameters for YUV/RGB  interleaved - 1st 132 bit word

//...0th 32 bit word
#define IPU_IPU_IMA_DATA_PARAM_XV_LSH       0
#define IPU_IPU_IMA_DATA_PARAM_YV_LSH       10
#define IPU_IPU_IMA_DATA_PARAM_XB_LSH       20

//...1st 32 bit word
#define IPU_IPU_IMA_DATA_PARAM_YB_LSH       (32 - 32)
#define IPU_IPU_IMA_DATA_PARAM_SCE_LSH      (44 - 32)
// 1 - reserved bit                   
#define IPU_IPU_IMA_DATA_PARAM_NSB_LSH      (46 - 32)
#define IPU_IPU_IMA_DATA_PARAM_LNPB_LSH     (47 - 32)
#define IPU_IPU_IMA_DATA_PARAM_SX_LSH       (53 - 32)
#define IPU_IPU_IMA_DATA_PARAM_LOW_SY_LSH   (63 - 32)

//...2nd 32 bit word
#define IPU_IPU_IMA_DATA_PARAM_HIGH_SY_LSH  (64 - 64)
#define IPU_IPU_IMA_DATA_PARAM_NS_LSH       (73 - 64)
#define IPU_IPU_IMA_DATA_PARAM_SM_LSH       (83 - 64)
#define IPU_IPU_IMA_DATA_PARAM_LOW_SDX_LSH  (93 - 64)

//...3rd 32 bit word
#define IPU_IPU_IMA_DATA_PARAM_HIGH_SDX_LSH (96 - 96)
#define IPU_IPU_IMA_DATA_PARAM_SDY_LSH      (98 - 96)
#define IPU_IPU_IMA_DATA_PARAM_SDRX_LSH     (103 - 96)
#define IPU_IPU_IMA_DATA_PARAM_SDRY_LSH     (104 - 96)
#define IPU_IPU_IMA_DATA_PARAM_SCRQ_LSH     (105 - 96)
// 2 - reserved bits
#define IPU_IPU_IMA_DATA_PARAM_FW_LSH       (108 - 96)
#define IPU_IPU_IMA_DATA_PARAM_LOW_FH_LSH   (120 - 96)

//...4th 32 bit word
#define IPU_IPU_IMA_DATA_PARAM_HIGH_FH_LSH  (128 - 128)


//...parameters for YUV/RGB interleaved - 2nd 132 bit word

//...0th 32 bit word
#define IPU_IPU_IMA_DATA_PARAM_EBA0_LSH     0

//...1st 32 bit word
#define IPU_IPU_IMA_DATA_PARAM_EBA1_LSH     (32 - 32)

//...2nd 32 bit word
#define IPU_IPU_IMA_DATA_PARAM_BPP_LSH      (64 - 64)
#define IPU_IPU_IMA_DATA_PARAM_SL_LSH       (67 - 64)
#define IPU_IPU_IMA_DATA_PARAM_PFS_LSH      (81 - 64)
#define IPU_IPU_IMA_DATA_PARAM_BAM_LSH      (84 - 64)
//  2 - reserved bits
#define IPU_IPU_IMA_DATA_PARAM_NPB_LSH      (89 - 64)
//  1 - reserved bit

//...3rd 32 bit word
#define IPU_IPU_IMA_DATA_PARAM_SAT_LSH      (96 - 96)
#define IPU_IPU_IMA_DATA_PARAM_SCC_LSH      (98 - 96)
#define IPU_IPU_IMA_DATA_PARAM_OFS0_LSH     (99 - 96)
#define IPU_IPU_IMA_DATA_PARAM_OFS1_LSH     (104 - 96)
#define IPU_IPU_IMA_DATA_PARAM_OFS2_LSH     (109 - 96)
#define IPU_IPU_IMA_DATA_PARAM_OFS3_LSH     (114 - 96)
#define IPU_IPU_IMA_DATA_PARAM_WID0_LSH     (119 - 96)
#define IPU_IPU_IMA_DATA_PARAM_WID1_LSH     (122 - 96)
#define IPU_IPU_IMA_DATA_PARAM_WID2_LSH     (125 - 96)

//...4th 32 bit word
#define IPU_IPU_IMA_DATA_PARAM_WID3_LSH     (128 - 128)
#define IPU_IPU_IMA_DATA_PARAM_DEC_SEL_LSH  (131 - 128)

// IPU_INT_CTRL_1
#define IPU_IPU_INT_CTRL_1_DMAIC_0_EOF_EN_LSH   0
#define IPU_IPU_INT_CTRL_1_DMAIC_1_EOF_EN_LSH   1
#define IPU_IPU_INT_CTRL_1_DMAIC_2_EOF_EN_LSH   2
#define IPU_IPU_INT_CTRL_1_DMAIC_3_EOF_EN_LSH   3
#define IPU_IPU_INT_CTRL_1_DMAIC_4_EOF_EN_LSH   4
#define IPU_IPU_INT_CTRL_1_DMAIC_5_EOF_EN_LSH   5
#define IPU_IPU_INT_CTRL_1_DMAIC_6_EOF_EN_LSH   6
#define IPU_IPU_INT_CTRL_1_DMAIC_7_EOF_EN_LSH   7
#define IPU_IPU_INT_CTRL_1_DMAIC_8_EOF_EN_LSH   8
#define IPU_IPU_INT_CTRL_1_DMAIC_9_EOF_EN_LSH   9
#define IPU_IPU_INT_CTRL_1_DMAIC_10_EOF_EN_LSH  10
#define IPU_IPU_INT_CTRL_1_DMAIC_11_EOF_EN_LSH  11
#define IPU_IPU_INT_CTRL_1_DMAIC_12_EOF_EN_LSH  12
#define IPU_IPU_INT_CTRL_1_DMAIC_13_EOF_EN_LSH  13
#define IPU_IPU_INT_CTRL_1_DMASDC_0_EOF_EN_LSH  14
#define IPU_IPU_INT_CTRL_1_DMASDC_1_EOF_EN_LSH  15
#define IPU_IPU_INT_CTRL_1_DMASDC_2_EOF_EN_LSH  16
#define IPU_IPU_INT_CTRL_1_DMASDC_3_EOF_EN_LSH  17
#define IPU_IPU_INT_CTRL_1_DMAADC_2_EOF_EN_LSH  18
#define IPU_IPU_INT_CTRL_1_DMAADC_3_EOF_EN_LSH  19
#define IPU_IPU_INT_CTRL_1_DMAADC_4_EOF_EN_LSH  20
#define IPU_IPU_INT_CTRL_1_DMAADC_5_EOF_EN_LSH  21
#define IPU_IPU_INT_CTRL_1_DMAADC_6_EOF_EN_LSH  22
#define IPU_IPU_INT_CTRL_1_DMAADC_7_EOF_EN_LSH  23
#define IPU_IPU_INT_CTRL_1_DMAPF_0_EOF_EN_LSH   24
#define IPU_IPU_INT_CTRL_1_DMAPF_1_EOF_EN_LSH   25
#define IPU_IPU_INT_CTRL_1_DMAPF_2_EOF_EN_LSH   26
#define IPU_IPU_INT_CTRL_1_DMAPF_3_EOF_EN_LSH   27
#define IPU_IPU_INT_CTRL_1_DMAPF_4_EOF_EN_LSH   28
#define IPU_IPU_INT_CTRL_1_DMAPF_5_EOF_EN_LSH   29
#define IPU_IPU_INT_CTRL_1_DMAPF_6_EOF_EN_LSH   30
#define IPU_IPU_INT_CTRL_1_DMAPF_7_EOF_EN_LSH   31

// IPU_INT_CTRL_3
#define IPU_IPU_INT_CTRL_3_ADC_DISP0_VSYNC_LSH  17
#define IPU_IPU_INT_CTRL_3_ADC_PRP_EOF_EN_LSH   19
#define IPU_IPU_INT_CTRL_3_ADC_PP_EOF_EN_LSH    20
#define IPU_IPU_INT_CTRL_3_ADC_SYS1_EOF_EN_LSH  21
#define IPU_IPU_INT_CTRL_3_ADC_SYS2_EOF_EN_LSH  22

// IPU CSI Registers
#define IPU_CSI_SENS_CONF_VSYNC_POL_LSH         0
#define IPU_CSI_SENS_CONF_HSYNC_POL_LSH         1
#define IPU_CSI_SENS_CONF_DATA_POL_LSH          2
#define IPU_CSI_SENS_CONF_DISP_PIX_CLK_POL_LSH  3
#define IPU_CSI_SENS_CONF_SENS_PRTCL_LSH        4
#define IPU_CSI_SENS_CONF_SENS_CLK_SRC_LSH      7
#define IPU_CSI_SENS_CONF_SENS_DATA_FORMAT_LSH  8
#define IPU_CSI_SENS_CONF_DATA_WIDTH_LSH        10
#define IPU_CSI_SENS_CONF_EXT_VSYNC_LSH         15
#define IPU_CSI_SENS_CONF_DIV_RATIO_LSH         16
#define IPU_CSI_SENS_CONF_MCLK_PIXCLK_RATIO_LSH 24

#define IPU_CSI_SENS_FRM_SIZE_SENS_FRM_WIDTH_LSH  0
#define IPU_CSI_SENS_FRM_SIZE_SENS_FRM_HEIGHT_LSH 16

#define IPU_CSI_ACT_FRM_SIZE_ACT_FRM_WIDTH_LSH  0
#define IPU_CSI_ACT_FRM_SIZE_ACT_FRM_HEIGHT_LSH 16

#define IPU_CSI_OUT_FRM_CTRL_VSC_LSH            0
#define IPU_CSI_OUT_FRM_CTRL_HSC_LSH            8
#define IPU_CSI_OUT_FRM_CTRL_SKIP_ENC_LSH       16
#define IPU_CSI_OUT_FRM_CTRL_SKIP_VF_LSH        21
#define IPU_CSI_OUT_FRM_CTRL_IC_TV_MODE_LSH     26
#define IPU_CSI_OUT_FRM_CTRL_VERT_DWNS_LSH      28
#define IPU_CSI_OUT_FRM_CTRL_HORZ_DWNS_LSH      29

#define IPU_CSI_TST_CTRL_PG_R_VALUE_LSH         0
#define IPU_CSI_TST_CTRL_PG_G_VALUE_LSH         8
#define IPU_CSI_TST_CTRL_PG_B_VALUE_LSH         16
#define IPU_CSI_TST_CTRL_TEST_GEN_MODE_LSH      24

#define IPU_CSI_CCIR_CODE_1_END_FLD0_BLNK_1ST_LSH   0
#define IPU_CSI_CCIR_CODE_1_STRT_FLD0_BLNK_1ST_LSH  3
#define IPU_CSI_CCIR_CODE_1_END_FLD0_BLNK_2ND_LSH   6
#define IPU_CSI_CCIR_CODE_1_STRT_FLD0_BLNK_2ND_LSH  9
#define IPU_CSI_CCIR_CODE_1_END_FLD0_ACTV_LSH       16
#define IPU_CSI_CCIR_CODE_1_STRT_FLD0_ACTV_LSH      19
#define IPU_CSI_CCIR_CODE_1_CCIR_ERR_DET_EN_LSH     24

#define IPU_CSI_CCIR_CODE_2_END_FLD1_BLNK_1ST_LSH   0
#define IPU_CSI_CCIR_CODE_2_STRT_FLD1_BLNK_1ST_LSH  3
#define IPU_CSI_CCIR_CODE_2_END_FLD1_BLNK_2ND_LSH   6
#define IPU_CSI_CCIR_CODE_2_STRT_FLD1_BLNK_2ND_LSH  9
#define IPU_CSI_CCIR_CODE_2_END_FLD1_ACTV_LSH       16
#define IPU_CSI_CCIR_CODE_2_STRT_FLD1_ACTV_LSH      19

#define IPU_CSI_CCIR_CODE_3_CCIR_PRECOM_LSH         0

#define IPU_CSI_FLASH_STROBE_1_CLOCK_SEL_LSH           0
#define IPU_CSI_FLASH_STROBE_1_SENSE_ROW_DURATION_LSH  16

#define IPU_CSI_FLASH_STROBE_2_STROBE_EN_LSH           0
#define IPU_CSI_FLASH_STROBE_2_STROBE_POL_LSH          1
#define IPU_CSI_FLASH_STROBE_2_STROBE_START_TIME_LSH   3
#define IPU_CSI_FLASH_STROBE_2_STROBE_DURATION_LSH     16


// IPU IC Registers
#define IPU_IC_CONF_PRPENC_EN_LSH               0
#define IPU_IC_CONF_PRPENC_CSC1_LSH             1
#define IPU_IC_CONF_PRPENC_ROT_EN_LSH           2
#define IPU_IC_CONF_PRPVF_EN_LSH                8
#define IPU_IC_CONF_PRPVF_CSC1_LSH              9
#define IPU_IC_CONF_PRPVF_CSC2_LSH              10
#define IPU_IC_CONF_PRPVF_CMB_LSH               11
#define IPU_IC_CONF_PRPVF_ROT_EN_LSH            12
#define IPU_IC_CONF_PP_EN_LSH                   16
#define IPU_IC_CONF_PP_CSC1_LSH                 17
#define IPU_IC_CONF_PP_CSC2_LSH                 18
#define IPU_IC_CONF_PP_CMB_LSH                  19
#define IPU_IC_CONF_PP_ROT_EN_LSH               20
#define IPU_IC_CONF_IC_GLB_LOC_A_LSH            28
#define IPU_IC_CONF_IC_KEY_COLOR_EN_LSH         29
#define IPU_IC_CONF_RWS_EN_LSH                  30
#define IPU_IC_CONF_CSI_MEM_WR_EN_LSH           31

#define IPU_IC_PRP_ENC_RSC_PRPENC_RS_R_H_LSH    0
#define IPU_IC_PRP_ENC_RSC_PRPENC_DS_R_H_LSH    14
#define IPU_IC_PRP_ENC_RSC_PRPENC_RS_R_V_LSH    16
#define IPU_IC_PRP_ENC_RSC_PRPENC_DS_R_V_LSH    30

#define IPU_IC_PRP_VF_RSC_PRPVF_RS_R_H_LSH      0
#define IPU_IC_PRP_VF_RSC_PRPVF_DS_R_H_LSH      14
#define IPU_IC_PRP_VF_RSC_PRPVF_RS_R_V_LSH      16
#define IPU_IC_PRP_VF_RSC_PRPVF_DS_R_V_LSH      30

#define IPU_IC_PP_RSC_PP_RS_R_H_LSH             0
#define IPU_IC_PP_RSC_PP_DS_R_H_LSH             14
#define IPU_IC_PP_RSC_PP_RS_R_V_LSH             16
#define IPU_IC_PP_RSC_PP_DS_R_V_LSH             30

#define IPU_IC_CMBP_1_IC_PRPVF_ALPHA_V_LSH      0
#define IPU_IC_CMBP_1_IC_PP_ALPHA_V_LSH         8

#define IPU_IC_CMBP_2_IC_KEY_COLOR_B_LSH        0
#define IPU_IC_CMBP_2_IC_KEY_COLOR_G_LSH        8
#define IPU_IC_CMBP_2_IC_KEY_COLOR_R_LSH        16

// PF_CONF
#define IPU_PF_CONF_PF_TYPE_LSH                 0
#define IPU_PF_CONF_H264_Y_PAUSE_EN_LSH         4
#define IPU_PF_CONF_H264_Y_PAUSE_ROW_LSH        16

// IDMAC_CONF
#define IPU_IDMAC_CONF_PRYM_LSH                 0
#define IPU_IDMAC_CONF_SRCNT_LSH                4
#define IPU_IDMAC_CONF_SINGLE_AHB_M_EN_LSH      8

// SDC_COM_CONF
#define IPU_SDC_COM_CONF_SDC_MODE_LSH           0
#define IPU_SDC_COM_CONF_BG_MCP_FROM_LSH        2
#define IPU_SDC_COM_CONF_FG_MCP_FROM_LSH        3
#define IPU_SDC_COM_CONF_FG_EN_LSH              4
#define IPU_SDC_COM_CONF_GWSEL_LSH              5
#define IPU_SDC_COM_CONF_SDC_GLB_LOC_A_LSH      6
#define IPU_SDC_COM_CONF_SDC_KEY_COLOR_EN_LSH   7
#define IPU_SDC_COM_CONF_MASK_EN_LSH            8
#define IPU_SDC_COM_CONF_BG_EN_LSH              9
#define IPU_SDC_COM_CONF_SHARP_LSH              12
#define IPU_SDC_COM_CONF_DUAL_MODE_LSH          15
#define IPU_SDC_COM_CONF_COC_LSH                16

// SDC_GRAPH_WIND_CTRL
#define IPU_SDC_GRAPH_WIND_CTRL_SDC_KEY_COLOR_B_LSH    0
#define IPU_SDC_GRAPH_WIND_CTRL_SDC_KEY_COLOR_G_LSH    8
#define IPU_SDC_GRAPH_WIND_CTRL_SDC_KEY_COLOR_R_LSH    16
#define IPU_SDC_GRAPH_WIND_CTRL_SDC_ALPHA_V_LSH        24

// SDC_FG_POS
#define IPU_SDC_FG_POS_FGYP_LSH                    0
#define IPU_SDC_FG_POS_FGXP_LSH                    16

// SDC_BG_POS
#define IPU_SDC_BG_POS_BGYP_LSH                    0
#define IPU_SDC_BG_POS_BGXP_LSH                    16

// SDC_CUR_POS
#define IPU_SDC_CUR_POS_CYP_LSH                    0
#define IPU_SDC_CUR_POS_CYH_LSH                    10
#define IPU_SDC_CUR_POS_CXP_LSH                    16
#define IPU_SDC_CUR_POS_CXW_LSH                    26

// SDC_CUR_BLINK_PWM_CTRL
#define IPU_SDC_CUR_BLINK_PWM_CTRL_BKDIV_LSH    0
#define IPU_SDC_CUR_BLINK_PWM_CTRL_BK_EN_LSH    15
#define IPU_SDC_CUR_BLINK_PWM_CTRL_PWM_LSH      16
#define IPU_SDC_CUR_BLINK_PWM_CTRL_CC_EN_LSH    24
#define IPU_SDC_CUR_BLINK_PWM_CTRL_SCR_LSH      25

// SDC_CUR_MAP
#define IPU_SDC_CUR_MAP_CUR_COL_B_LSH           0
#define IPU_SDC_CUR_MAP_CUR_COL_G_LSH           8
#define IPU_SDC_CUR_MAP_CUR_COL_R_LSH           16

// SDC_HOR_CONF
#define IPU_SDC_HOR_CONF_SCREEN_WIDTH_LSH       16
#define IPU_SDC_HOR_CONF_H_SYNC_WIDTH_LSH       26

// SDC_VER_CONF
#define IPU_SDC_VER_CONF_V_SYNC_WIDTH_L_LSH     0
#define IPU_SDC_VER_CONF_SCREEN_HEIGHT_LSH      16
#define IPU_SDC_VER_CONF_V_SYNC_WIDTH_LSH       26

// SDC_SHARP_CONF_1
#define IPU_SDC_SHARP_CONF_1_CLS_RISE_DELAY_LSH     0
#define IPU_SDC_SHARP_CONF_1_PS_FALL_DELAY_LSH      8
#define IPU_SDC_SHARP_CONF_1_REV_TOGGLE_DELAY_LSH   16

// SDC_SHARP_CONF_2
#define IPU_SDC_SHARP_CONF_2_CLS_FALL_DELAY_LSH 0
#define IPU_SDC_SHARP_CONF_2_PS_RISE_DELAY_LSH  16

// ADC_CONF
#define IPU_ADC_CONF_PRP_CHAN_EN_LSH            0
#define IPU_ADC_CONF_PP_CHAN_EN_LSH             1
#define IPU_ADC_CONF_MCU_CHAN_EN_LSH            2
#define IPU_ADC_CONF_PRP_DISP_NUM_LSH           3
#define IPU_ADC_CONF_PRP_ADDR_INC_LSH           5
#define IPU_ADC_CONF_PRP_DATA_MAP_LSH           7
#define IPU_ADC_CONF_PP_DISP_NUM_LSH            8
#define IPU_ADC_CONF_PP_ADDR_INC_LSH            10
#define IPU_ADC_CONF_PP_DATA_MAP_LSH            12
#define IPU_ADC_CONF_PP_NO_TEARING_LSH          13
#define IPU_ADC_CONF_SYS1_NO_TEARING_LSH        14
#define IPU_ADC_CONF_SYS2_NO_TEARING_LSH        15
#define IPU_ADC_CONF_SYS1_MODE_LSH              16
#define IPU_ADC_CONF_SYS1_DISP_NUM_LSH          19
#define IPU_ADC_CONF_SYS1_ADDR_INC_LSH          21
#define IPU_ADC_CONF_SYS1_DATA_MAP_LSH          23
#define IPU_ADC_CONF_SYS2_MODE_LSH              24
#define IPU_ADC_CONF_SYS2_DISP_NUM_LSH          27
#define IPU_ADC_CONF_SYS2_ADDR_INC_LSH          29
#define IPU_ADC_CONF_SYS2_DATA_MAP_LSH          31

// DI_HSP_CLK_PER
#define IPU_DI_HSP_CLK_PER_HSP_CLK_PERIOD_1_LSH 0
#define IPU_DI_HSP_CLK_PER_HSP_CLK_PERIOD_2_LSH 16

// DI_DISP0_TIME_CONF_1
#define IPU_DISP0_TIME_CONF_1_DISP0_IF_CLK_PER_WR_LSH   0
#define IPU_DISP0_TIME_CONF_1_DISP0_IF_CLK_UP_WR_LSH    12
#define IPU_DISP0_TIME_CONF_1_DISP0_IF_CLK_DOWN_WR_LSH  22

// DI_DISP0_TIME_CONF_2
#define IPU_DISP0_TIME_CONF_2_DISP0_IF_CLK_PER_RD_LSH   0
#define IPU_DISP0_TIME_CONF_2_DISP0_IF_CLK_UP_RD_LSH    12
#define IPU_DISP0_TIME_CONF_2_DISP0_IF_CLK_DOWN_RD_LSH  22

// DI_DISP0_TIME_CONF_3
#define IPU_DISP0_TIME_CONF_3_DISP0_PIX_CLK_PER_LSH 0
#define IPU_DISP0_TIME_CONF_3_DISP0_READ_EN_LSH     16
#define IPU_DISP0_TIME_CONF_3_DISP0_RD_WAIT_ST_LSH  28

// DI_DISP3_TIME_CONF
#define IPU_DI_DISP3_TIME_CONF_DISP3_IF_CLK_PER_WR_LSH  0
#define IPU_DI_DISP3_TIME_CONF_DISP3_IF_CLK_UP_WR_LSH   12
#define IPU_DI_DISP3_TIME_CONF_DISP3_IF_CLK_DOWN_WR_LSH 22


// ADC_SYSCHA1_SA
// ADC_SYSCHA2_SA
// ADC_PRPCHAN_SA
// ADC_PPCHAN_SA
#define IPU_ADC_CHA_CHAN_SA_LSH                 0
#define IPU_ADC_CHA_START_TIME_LSH              23

// ADC_DISP0_CONF
// ADC_DISP1_CONF
// ADC_DISP2_CONF
#define IPU_ADC_DISP_CONF_DISP_SL_LSH           0
#define IPU_ADC_DISP_CONF_DISP_TYPE_LSH         12
#define IPU_ADC_DISP_CONF_DISP_DATA_WIDTH_LSH   14
#define IPU_ADC_DISP_CONF_DISP_DATA_MAP_LSH     15

// ADC_DISP0_RD_AP
#define IPU_ADC_DISP0_RD_AP_DISP0_ACK_PTRN_LSH  0

// ADC_DISP0_RDM
#define IPU_ADC_DISP0_RDM_DISP0_MASK_ACK_DATA_LSH   0

// ADC_DISP0_SS
// ADC_DISP1_SS
// ADC_DISP2_SS
#define IPU_ADC_DISP_SS_SCREEN_WIDTH_LSH        0
#define IPU_ADC_DISP_SS_SCREEN_HEIGHT_LSH       16

// ADC_DISP_VSYNC
#define IPU_ADC_DISP_VSYNC_DISP0_VSYNC_MODE_LSH     0
#define IPU_ADC_DISP_VSYNC_DISP12_VSYNC_MODE_LSH    2
#define IPU_ADC_DISP_VSYNC_DISP12_VSYNC_SEL_LSH     4
#define IPU_ADC_DISP_VSYNC_DISP_LN_WT_LSH           6
#define IPU_ADC_DISP_VSYNC_DISP0_VSYNC_WIDTH_LSH    16
#define IPU_ADC_DISP_VSYNC_DISP0_VSYNC_WIDTH_L_LSH  22
#define IPU_ADC_DISP_VSYNC_DISP12_VSYNC_WIDTH_LSH   24
#define IPU_ADC_DISP_VSYNC_DISP12_VSYNC_WIDTH_L_LSH 30


// DI_DISP_IF_CONF
#define IPU_DI_DISP_IF_CONF_DISP0_EN_LSH                0
#define IPU_DI_DISP_IF_CONF_DISP0_IF_MODE_LSH           1
#define IPU_DI_DISP_IF_CONF_DISP0_PAR_BURST_MODE_LSH    3
#define IPU_DI_DISP_IF_CONF_DISP1_EN_LSH                8
#define IPU_DI_DISP_IF_CONF_DISP1_IF_MODE_LSH           9
#define IPU_DI_DISP_IF_CONF_DISP1_PAR_BURST_MODE_LSH    12
#define IPU_DI_DISP_IF_CONF_DISP2_EN_LSH                16
#define IPU_DI_DISP_IF_CONF_DISP2_IF_MODE_LSH           17
#define IPU_DI_DISP_IF_CONF_DISP2_PAR_BURST_MODE_LSH    20
#define IPU_DI_DISP_IF_CONF_DISP3_DATAMASK_LSH          24
#define IPU_DI_DISP_IF_CONF_DISP3_CLK_SEL_LSH           25
#define IPU_DI_DISP_IF_CONF_DISP3_CLK_IDLE_LSH          26
#define IPU_DI_DISP_IF_CONF_DISP012_DEAD_CLK_NUM_LSH    27

// DI_DISP_SIG_POL
#define IPU_DI_DISP_SIG_POL_D0_DATA_POL_LSH             0
#define IPU_DI_DISP_SIG_POL_D0_CS_POL_LSH               1
#define IPU_DI_DISP_SIG_POL_D0_PAR_RS_POL_LSH           2
#define IPU_DI_DISP_SIG_POL_D0_WR_POL_LSH               3
#define IPU_DI_DISP_SIG_POL_D0_RD_POL_LSH               4
#define IPU_DI_DISP_SIG_POL_D0_VSYNC_POL_LSH            5
#define IPU_DI_DISP_SIG_POL_D12_VSYNC_POL_LSH           6
//#define IPU_DI_DISP_SIG_POL_RESERVED            
#define IPU_DI_DISP_SIG_POL_D1_DATA_POL_LSH             8
#define IPU_DI_DISP_SIG_POL_D1_CS_POL_LSH               9
#define IPU_DI_DISP_SIG_POL_D1_PAR_RS_POL_LSH           10
#define IPU_DI_DISP_SIG_POL_D1_WR_POL_LSH               11
#define IPU_DI_DISP_SIG_POL_D1_RD_POL_LSH               12
#define IPU_DI_DISP_SIG_POL_D1_SD_D_POL_LSH             13
#define IPU_DI_DISP_SIG_POL_D1_SD_CLK_POL_LSH           14
#define IPU_DI_DISP_SIG_POL_D1_SER_RS_POL_LSH           15
#define IPU_DI_DISP_SIG_POL_D2_DATA_POL_LSH             16
#define IPU_DI_DISP_SIG_POL_D2_CS_POL_LSH               17
#define IPU_DI_DISP_SIG_POL_D2_PAR_RS_POL_LSH           18
#define IPU_DI_DISP_SIG_POL_D2_WR_POL_LSH               19
#define IPU_DI_DISP_SIG_POL_D2_RD_POL_LSH               20
#define IPU_DI_DISP_SIG_POL_D2_SD_D_POL_LSH             21
#define IPU_DI_DISP_SIG_POL_D2_SD_CLK_POL_LSH           22
#define IPU_DI_DISP_SIG_POL_D2_SER_RS_POL_LSH           23
#define IPU_DI_DISP_SIG_POL_D3_DATA_POL_LSH             24
#define IPU_DI_DISP_SIG_POL_D3_CLK_POL_LSH              25
#define IPU_DI_DISP_SIG_POL_D3_DRDY_SHARP_POL_LSH       26
#define IPU_DI_DISP_SIG_POL_D3_HSYNC_POL_LSH            27
#define IPU_DI_DISP_SIG_POL_D3_VSYNC_POL_LSH            28
#define IPU_DI_DISP_SIG_POL_D0_BCLK_POL_LSH             29
#define IPU_DI_DISP_SIG_POL_D1_BCLK_POL_LSH             30
#define IPU_DI_DISP_SIG_POL_D2_BCLK_POL_LSH             31


// DI_DISP0_DB0_MAP
#define IPU_DI_DISP0_DB0_MAP_MD00_M0_LSH        0
#define IPU_DI_DISP0_DB0_MAP_MD00_M1_LSH        2
#define IPU_DI_DISP0_DB0_MAP_MD00_M2_LSH        4
#define IPU_DI_DISP0_DB0_MAP_MD00_M3_LSH        6
#define IPU_DI_DISP0_DB0_MAP_MD00_M4_LSH        8
#define IPU_DI_DISP0_DB0_MAP_MD00_M5_LSH        10
#define IPU_DI_DISP0_DB0_MAP_MD00_M6_LSH        12
#define IPU_DI_DISP0_DB0_MAP_MD00_M7_LSH        14
#define IPU_DI_DISP0_DB0_MAP_MD00_OFFS0_LSH     16
#define IPU_DI_DISP0_DB0_MAP_MD00_OFFS1_LSH     21
#define IPU_DI_DISP0_DB0_MAP_MD00_OFFS2_LSH     26

// DI_DISP0_DB1_MAP
#define IPU_DI_DISP0_DB1_MAP_MD01_M0_LSH        0
#define IPU_DI_DISP0_DB1_MAP_MD01_M1_LSH        2
#define IPU_DI_DISP0_DB1_MAP_MD01_M2_LSH        4
#define IPU_DI_DISP0_DB1_MAP_MD01_M3_LSH        6
#define IPU_DI_DISP0_DB1_MAP_MD01_M4_LSH        8
#define IPU_DI_DISP0_DB1_MAP_MD01_M5_LSH        10
#define IPU_DI_DISP0_DB1_MAP_MD01_M6_LSH        12
#define IPU_DI_DISP0_DB1_MAP_MD01_M7_LSH        14
#define IPU_DI_DISP0_DB1_MAP_MD01_OFFS0_LSH     16
#define IPU_DI_DISP0_DB1_MAP_MD01_OFFS1_LSH     21
#define IPU_DI_DISP0_DB1_MAP_MD01_OFFS2_LSH     26

// DI_DISP0_DB2_MAP
#define IPU_DI_DISP0_DB2_MAP_MD02_M0_LSH        0
#define IPU_DI_DISP0_DB2_MAP_MD02_M1_LSH        2
#define IPU_DI_DISP0_DB2_MAP_MD02_M2_LSH        4
#define IPU_DI_DISP0_DB2_MAP_MD02_M3_LSH        6
#define IPU_DI_DISP0_DB2_MAP_MD02_M4_LSH        8
#define IPU_DI_DISP0_DB2_MAP_MD02_M5_LSH        10
#define IPU_DI_DISP0_DB2_MAP_MD02_M6_LSH        12
#define IPU_DI_DISP0_DB2_MAP_MD02_M7_LSH        14
#define IPU_DI_DISP0_DB2_MAP_MD02_OFFS0_LSH     16
#define IPU_DI_DISP0_DB2_MAP_MD02_OFFS1_LSH     21
#define IPU_DI_DISP0_DB2_MAP_MD02_OFFS2_LSH     26

// DI_DISP0_CB0_MAP
#define IPU_DI_DISP0_CB0_MAP_MC00_M0_LSH        0
#define IPU_DI_DISP0_CB0_MAP_MC00_M1_LSH        2
#define IPU_DI_DISP0_CB0_MAP_MC00_M2_LSH        4
#define IPU_DI_DISP0_CB0_MAP_MC00_M3_LSH        6
#define IPU_DI_DISP0_CB0_MAP_MC00_M4_LSH        8
#define IPU_DI_DISP0_CB0_MAP_MC00_M5_LSH        10
#define IPU_DI_DISP0_CB0_MAP_MC00_M6_LSH        12
#define IPU_DI_DISP0_CB0_MAP_MC00_M7_LSH        14
#define IPU_DI_DISP0_CB0_MAP_MC00_OFFS0_LSH     16
#define IPU_DI_DISP0_CB0_MAP_MC00_OFFS1_LSH     21
#define IPU_DI_DISP0_CB0_MAP_MC00_OFFS2_LSH     26

// DI_DISP0_CB1_MAP
#define IPU_DI_DISP0_CB1_MAP_MC01_M0_LSH        0
#define IPU_DI_DISP0_CB1_MAP_MC01_M1_LSH        2
#define IPU_DI_DISP0_CB1_MAP_MC01_M2_LSH        4
#define IPU_DI_DISP0_CB1_MAP_MC01_M3_LSH        6
#define IPU_DI_DISP0_CB1_MAP_MC01_M4_LSH        8
#define IPU_DI_DISP0_CB1_MAP_MC01_M5_LSH        10
#define IPU_DI_DISP0_CB1_MAP_MC01_M6_LSH        12
#define IPU_DI_DISP0_CB1_MAP_MC01_M7_LSH        14
#define IPU_DI_DISP0_CB1_MAP_MC01_OFFS0_LSH     16
#define IPU_DI_DISP0_CB1_MAP_MC01_OFFS1_LSH     21
#define IPU_DI_DISP0_CB1_MAP_MC01_OFFS2_LSH     26

// DI_DISP0_CB2_MAP
#define IPU_DI_DISP0_CB2_MAP_MC02_M0_LSH        0
#define IPU_DI_DISP0_CB2_MAP_MC02_M1_LSH        2
#define IPU_DI_DISP0_CB2_MAP_MC02_M2_LSH        4
#define IPU_DI_DISP0_CB2_MAP_MC02_M3_LSH        6
#define IPU_DI_DISP0_CB2_MAP_MC02_M4_LSH        8
#define IPU_DI_DISP0_CB2_MAP_MC02_M5_LSH        10
#define IPU_DI_DISP0_CB2_MAP_MC02_M6_LSH        12
#define IPU_DI_DISP0_CB2_MAP_MC02_M7_LSH        14
#define IPU_DI_DISP0_CB2_MAP_MC02_OFFS0_LSH     16
#define IPU_DI_DISP0_CB2_MAP_MC02_OFFS1_LSH     21
#define IPU_DI_DISP0_CB2_MAP_MC02_OFFS2_LSH     26

// DI_DISP1_DB0_MAP
#define IPU_DI_DISP1_DB0_MAP_MD10_M0_LSH        0
#define IPU_DI_DISP1_DB0_MAP_MD10_M1_LSH        2
#define IPU_DI_DISP1_DB0_MAP_MD10_M2_LSH        4
#define IPU_DI_DISP1_DB0_MAP_MD10_M3_LSH        6
#define IPU_DI_DISP1_DB0_MAP_MD10_M4_LSH        8
#define IPU_DI_DISP1_DB0_MAP_MD10_M5_LSH        10
#define IPU_DI_DISP1_DB0_MAP_MD10_M6_LSH        12
#define IPU_DI_DISP1_DB0_MAP_MD10_M7_LSH        14
#define IPU_DI_DISP1_DB0_MAP_MD10_OFFS0_LSH     16
#define IPU_DI_DISP1_DB0_MAP_MD10_OFFS1_LSH     21
#define IPU_DI_DISP1_DB0_MAP_MD10_OFFS2_LSH     26

// DI_DISP1_DB1_MAP
#define IPU_DI_DISP1_DB1_MAP_MD11_M0_LSH        0
#define IPU_DI_DISP1_DB1_MAP_MD11_M1_LSH        2
#define IPU_DI_DISP1_DB1_MAP_MD11_M2_LSH        4
#define IPU_DI_DISP1_DB1_MAP_MD11_M3_LSH        6
#define IPU_DI_DISP1_DB1_MAP_MD11_M4_LSH        8
#define IPU_DI_DISP1_DB1_MAP_MD11_M5_LSH        10
#define IPU_DI_DISP1_DB1_MAP_MD11_M6_LSH        12
#define IPU_DI_DISP1_DB1_MAP_MD11_M7_LSH        14
#define IPU_DI_DISP1_DB1_MAP_MD11_OFFS0_LSH     16
#define IPU_DI_DISP1_DB1_MAP_MD11_OFFS1_LSH     21
#define IPU_DI_DISP1_DB1_MAP_MD11_OFFS2_LSH     26

// DI_DISP1_DB2_MAP
#define IPU_DI_DISP1_DB2_MAP_MD12_M0_LSH        0
#define IPU_DI_DISP1_DB2_MAP_MD12_M1_LSH        2
#define IPU_DI_DISP1_DB2_MAP_MD12_M2_LSH        4
#define IPU_DI_DISP1_DB2_MAP_MD12_M3_LSH        6
#define IPU_DI_DISP1_DB2_MAP_MD12_M4_LSH        8
#define IPU_DI_DISP1_DB2_MAP_MD12_M5_LSH        10
#define IPU_DI_DISP1_DB2_MAP_MD12_M6_LSH        12
#define IPU_DI_DISP1_DB2_MAP_MD12_M7_LSH        14
#define IPU_DI_DISP1_DB2_MAP_MD12_OFFS0_LSH     16
#define IPU_DI_DISP1_DB2_MAP_MD12_OFFS1_LSH     21
#define IPU_DI_DISP1_DB2_MAP_MD12_OFFS2_LSH     26

// DI_DISP1_CB0_MAP
#define IPU_DI_DISP1_CB0_MAP_MC10_M0_LSH        0
#define IPU_DI_DISP1_CB0_MAP_MC10_M1_LSH        2
#define IPU_DI_DISP1_CB0_MAP_MC10_M2_LSH        4
#define IPU_DI_DISP1_CB0_MAP_MC10_M3_LSH        6
#define IPU_DI_DISP1_CB0_MAP_MC10_M4_LSH        8
#define IPU_DI_DISP1_CB0_MAP_MC10_M5_LSH        10
#define IPU_DI_DISP1_CB0_MAP_MC10_M6_LSH        12
#define IPU_DI_DISP1_CB0_MAP_MC10_M7_LSH        14
#define IPU_DI_DISP1_CB0_MAP_MC10_OFFS0_LSH     16
#define IPU_DI_DISP1_CB0_MAP_MC10_OFFS1_LSH     21
#define IPU_DI_DISP1_CB0_MAP_MC10_OFFS2_LSH     26

// DI_DISP1_CB1_MAP
#define IPU_DI_DISP1_CB1_MAP_MC11_M0_LSH        0
#define IPU_DI_DISP1_CB1_MAP_MC11_M1_LSH        2
#define IPU_DI_DISP1_CB1_MAP_MC11_M2_LSH        4
#define IPU_DI_DISP1_CB1_MAP_MC11_M3_LSH        6
#define IPU_DI_DISP1_CB1_MAP_MC11_M4_LSH        8
#define IPU_DI_DISP1_CB1_MAP_MC11_M5_LSH        10
#define IPU_DI_DISP1_CB1_MAP_MC11_M6_LSH        12
#define IPU_DI_DISP1_CB1_MAP_MC11_M7_LSH        14
#define IPU_DI_DISP1_CB1_MAP_MC11_OFFS0_LSH     16
#define IPU_DI_DISP1_CB1_MAP_MC11_OFFS1_LSH     21
#define IPU_DI_DISP1_CB1_MAP_MC11_OFFS2_LSH     26

// DI_DISP1_CB2_MAP
#define IPU_DI_DISP1_CB2_MAP_MC12_M0_LSH        0
#define IPU_DI_DISP1_CB2_MAP_MC12_M1_LSH        2
#define IPU_DI_DISP1_CB2_MAP_MC12_M2_LSH        4
#define IPU_DI_DISP1_CB2_MAP_MC12_M3_LSH        6
#define IPU_DI_DISP1_CB2_MAP_MC12_M4_LSH        8
#define IPU_DI_DISP1_CB2_MAP_MC12_M5_LSH        10
#define IPU_DI_DISP1_CB2_MAP_MC12_M6_LSH        12
#define IPU_DI_DISP1_CB2_MAP_MC12_M7_LSH        14
#define IPU_DI_DISP1_CB2_MAP_MC12_OFFS0_LSH     16
#define IPU_DI_DISP1_CB2_MAP_MC12_OFFS1_LSH     21
#define IPU_DI_DISP1_CB2_MAP_MC12_OFFS2_LSH     26

// DI_DISP2_DB0_MAP
#define IPU_DI_DISP2_DB0_MAP_MD20_M0_LSH        0
#define IPU_DI_DISP2_DB0_MAP_MD20_M1_LSH        2
#define IPU_DI_DISP2_DB0_MAP_MD20_M2_LSH        4
#define IPU_DI_DISP2_DB0_MAP_MD20_M3_LSH        6
#define IPU_DI_DISP2_DB0_MAP_MD20_M4_LSH        8
#define IPU_DI_DISP2_DB0_MAP_MD20_M5_LSH        10
#define IPU_DI_DISP2_DB0_MAP_MD20_M6_LSH        12
#define IPU_DI_DISP2_DB0_MAP_MD20_M7_LSH        14
#define IPU_DI_DISP2_DB0_MAP_MD20_OFFS0_LSH     16
#define IPU_DI_DISP2_DB0_MAP_MD20_OFFS1_LSH     21
#define IPU_DI_DISP2_DB0_MAP_MD20_OFFS2_LSH     26

// DI_DISP2_DB1_MAP
#define IPU_DI_DISP2_DB1_MAP_MD21_M0_LSH        0
#define IPU_DI_DISP2_DB1_MAP_MD21_M1_LSH        2
#define IPU_DI_DISP2_DB1_MAP_MD21_M2_LSH        4
#define IPU_DI_DISP2_DB1_MAP_MD21_M3_LSH        6
#define IPU_DI_DISP2_DB1_MAP_MD21_M4_LSH        8
#define IPU_DI_DISP2_DB1_MAP_MD21_M5_LSH        10
#define IPU_DI_DISP2_DB1_MAP_MD21_M6_LSH        12
#define IPU_DI_DISP2_DB1_MAP_MD21_M7_LSH        14
#define IPU_DI_DISP2_DB1_MAP_MD21_OFFS0_LSH     16
#define IPU_DI_DISP2_DB1_MAP_MD21_OFFS1_LSH     21
#define IPU_DI_DISP2_DB1_MAP_MD21_OFFS2_LSH     26

// DI_DISP2_DB2_MAP
#define IPU_DI_DISP2_DB2_MAP_MD22_M0_LSH        0
#define IPU_DI_DISP2_DB2_MAP_MD22_M1_LSH        2
#define IPU_DI_DISP2_DB2_MAP_MD22_M2_LSH        4
#define IPU_DI_DISP2_DB2_MAP_MD22_M3_LSH        6
#define IPU_DI_DISP2_DB2_MAP_MD22_M4_LSH        8
#define IPU_DI_DISP2_DB2_MAP_MD22_M5_LSH        10
#define IPU_DI_DISP2_DB2_MAP_MD22_M6_LSH        12
#define IPU_DI_DISP2_DB2_MAP_MD22_M7_LSH        14
#define IPU_DI_DISP2_DB2_MAP_MD22_OFFS0_LSH     16
#define IPU_DI_DISP2_DB2_MAP_MD22_OFFS1_LSH     21
#define IPU_DI_DISP2_DB2_MAP_MD22_OFFS2_LSH     26

// DI_DISP2_CB0_MAP
#define IPU_DI_DISP2_CB0_MAP_MC20_M0_LSH        0
#define IPU_DI_DISP2_CB0_MAP_MC20_M1_LSH        2
#define IPU_DI_DISP2_CB0_MAP_MC20_M2_LSH        4
#define IPU_DI_DISP2_CB0_MAP_MC20_M3_LSH        6
#define IPU_DI_DISP2_CB0_MAP_MC20_M4_LSH        8
#define IPU_DI_DISP2_CB0_MAP_MC20_M5_LSH        10
#define IPU_DI_DISP2_CB0_MAP_MC20_M6_LSH        12
#define IPU_DI_DISP2_CB0_MAP_MC20_M7_LSH        14
#define IPU_DI_DISP2_CB0_MAP_MC20_OFFS0_LSH     16
#define IPU_DI_DISP2_CB0_MAP_MC20_OFFS1_LSH     21
#define IPU_DI_DISP2_CB0_MAP_MC20_OFFS2_LSH     26

// DI_DISP2_CB1_MAP
#define IPU_DI_DISP2_CB1_MAP_MC21_M0_LSH        0
#define IPU_DI_DISP2_CB1_MAP_MC21_M1_LSH        2
#define IPU_DI_DISP2_CB1_MAP_MC21_M2_LSH        4
#define IPU_DI_DISP2_CB1_MAP_MC21_M3_LSH        6
#define IPU_DI_DISP2_CB1_MAP_MC21_M4_LSH        8
#define IPU_DI_DISP2_CB1_MAP_MC21_M5_LSH        10
#define IPU_DI_DISP2_CB1_MAP_MC21_M6_LSH        12
#define IPU_DI_DISP2_CB1_MAP_MC21_M7_LSH        14
#define IPU_DI_DISP2_CB1_MAP_MC21_OFFS0_LSH     16
#define IPU_DI_DISP2_CB1_MAP_MC21_OFFS1_LSH     21
#define IPU_DI_DISP2_CB1_MAP_MC21_OFFS2_LSH     26

// DI_DISP2_CB2_MAP
#define IPU_DI_DISP2_CB2_MAP_MC22_M0_LSH        0
#define IPU_DI_DISP2_CB2_MAP_MC22_M1_LSH        2
#define IPU_DI_DISP2_CB2_MAP_MC22_M2_LSH        4
#define IPU_DI_DISP2_CB2_MAP_MC22_M3_LSH        6
#define IPU_DI_DISP2_CB2_MAP_MC22_M4_LSH        8
#define IPU_DI_DISP2_CB2_MAP_MC22_M5_LSH        10
#define IPU_DI_DISP2_CB2_MAP_MC22_M6_LSH        12
#define IPU_DI_DISP2_CB2_MAP_MC22_M7_LSH        14
#define IPU_DI_DISP2_CB2_MAP_MC22_OFFS0_LSH     16
#define IPU_DI_DISP2_CB2_MAP_MC22_OFFS1_LSH     21
#define IPU_DI_DISP2_CB2_MAP_MC22_OFFS2_LSH     26

// DI_DISP3_B0_MAP
#define IPU_DI_DISP3_B0_MAP_M30_M0_LSH          0
#define IPU_DI_DISP3_B0_MAP_M30_M1_LSH          2
#define IPU_DI_DISP3_B0_MAP_M30_M2_LSH          4
#define IPU_DI_DISP3_B0_MAP_M30_M3_LSH          6
#define IPU_DI_DISP3_B0_MAP_M30_M4_LSH          8
#define IPU_DI_DISP3_B0_MAP_M30_M5_LSH          10
#define IPU_DI_DISP3_B0_MAP_M30_M6_LSH          12
#define IPU_DI_DISP3_B0_MAP_M30_M7_LSH          14
#define IPU_DI_DISP3_B0_MAP_M30_OFFS0_LSH       16
#define IPU_DI_DISP3_B0_MAP_M30_OFFS1_LSH       21
#define IPU_DI_DISP3_B0_MAP_M30_OFFS2_LSH       26

// DI_DISP3_B1_MAP
#define IPU_DI_DISP3_B1_MAP_M31_M0_LSH          0
#define IPU_DI_DISP3_B1_MAP_M31_M1_LSH          2
#define IPU_DI_DISP3_B1_MAP_M31_M2_LSH          4
#define IPU_DI_DISP3_B1_MAP_M31_M3_LSH          6
#define IPU_DI_DISP3_B1_MAP_M31_M4_LSH          8
#define IPU_DI_DISP3_B1_MAP_M31_M5_LSH          10
#define IPU_DI_DISP3_B1_MAP_M31_M6_LSH          12
#define IPU_DI_DISP3_B1_MAP_M31_M7_LSH          14
#define IPU_DI_DISP3_B1_MAP_M31_OFFS0_LSH       16
#define IPU_DI_DISP3_B1_MAP_M31_OFFS1_LSH       21
#define IPU_DI_DISP3_B1_MAP_M31_OFFS2_LSH       26

// DI_DISP3_B2_MAP
#define IPU_DI_DISP3_B2_MAP_M32_M0_LSH          0
#define IPU_DI_DISP3_B2_MAP_M32_M1_LSH          2
#define IPU_DI_DISP3_B2_MAP_M32_M2_LSH          4
#define IPU_DI_DISP3_B2_MAP_M32_M3_LSH          6
#define IPU_DI_DISP3_B2_MAP_M32_M4_LSH          8
#define IPU_DI_DISP3_B2_MAP_M32_M5_LSH          10
#define IPU_DI_DISP3_B2_MAP_M32_M6_LSH          12
#define IPU_DI_DISP3_B2_MAP_M32_M7_LSH          14
#define IPU_DI_DISP3_B2_MAP_M32_OFFS0_LSH       16
#define IPU_DI_DISP3_B2_MAP_M32_OFFS1_LSH       21
#define IPU_DI_DISP3_B2_MAP_M32_OFFS2_LSH       26

// DI_DISP_ACC_CC
#define IPU_DI_DISP_ACC_CC_DISP0_IF_CLK_CNT_D_LSH   0
#define IPU_DI_DISP_ACC_CC_DISP0_IF_CLK_CNT_C_LSH   2
#define IPU_DI_DISP_ACC_CC_DISP1_IF_CLK_CNT_D_LSH   4
#define IPU_DI_DISP_ACC_CC_DISP1_IF_CLK_CNT_C_LSH   6
#define IPU_DI_DISP_ACC_CC_DISP2_IF_CLK_CNT_D_LSH   8
#define IPU_DI_DISP_ACC_CC_DISP2_IF_CLK_CNT_C_LSH   10
#define IPU_DI_DISP_ACC_CC_DISP3_IF_CLK_CNT_D_LSH   12

// DI_DISP_LLA_CONF
#define IPU_DI_DISP_LLA_CONF_DRCT_RS_LSH        0
#define IPU_DI_DISP_LLA_CONF_DRCT_DISP_NUM_LSH  1
#define IPU_DI_DISP_LLA_CONF_DRCT_LOCK_LSH      3
#define IPU_DI_DISP_LLA_CONF_DRCT_MAP_DC_LSH    4
#define IPU_DI_DISP_LLA_CONF_DRCT_BE_MODE_LSH   5

// DI_DISP_LLA_DATA
#define IPU_DI_DISP_LLA_DATA_LLA_DATA_LSH       0


//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------

// IPU_CONF
#define IPU_IPU_CONF_CSI_EN_WID                 1
#define IPU_IPU_CONF_IC_EN_WID                  1
#define IPU_IPU_CONF_ROT_EN_WID                 1
#define IPU_IPU_CONF_PF_EN_WID                  1
#define IPU_IPU_CONF_SDC_EN_WID                 1
#define IPU_IPU_CONF_ADC_EN_WID                 1
#define IPU_IPU_CONF_DI_EN_WID                  1
#define IPU_IPU_CONF_DU_EN_WID                  1
#define IPU_IPU_CONF_PXL_ENDIAN_WID             1

// IPU DMA Channel ID used in the various IPU registers
#define IPU_DMA_CHA_DMAIC_0_WID                 1
#define IPU_DMA_CHA_DMAIC_1_WID                 1
#define IPU_DMA_CHA_DMAADC_0_WID                1
#define IPU_DMA_CHA_DMAIC_2_WID                 1
#define IPU_DMA_CHA_DMAADC_1_WID                1
#define IPU_DMA_CHA_DMAIC_3_WID                 1
#define IPU_DMA_CHA_DMAIC_4_WID                 1
#define IPU_DMA_CHA_DMAIC_5_WID                 1
#define IPU_DMA_CHA_DMAIC_6_WID                 1
#define IPU_DMA_CHA_DMAIC_7_WID                 1
#define IPU_DMA_CHA_DMAIC_8_WID                 1
#define IPU_DMA_CHA_DMAIC_9_WID                 1
#define IPU_DMA_CHA_DMAIC_10_WID                1
#define IPU_DMA_CHA_DMAIC_11_WID                1
#define IPU_DMA_CHA_DMAIC_12_WID                1
#define IPU_DMA_CHA_DMAIC_13_WID                1
#define IPU_DMA_CHA_DMASDC_0_WID                1
#define IPU_DMA_CHA_DMASDC_1_WID                1
#define IPU_DMA_CHA_DMASDC_2_WID                1
#define IPU_DMA_CHA_DMASDC_3_WID                1
#define IPU_DMA_CHA_DMAADC_2_WID                1
#define IPU_DMA_CHA_DMAADC_3_WID                1
#define IPU_DMA_CHA_DMAADC_4_WID                1
#define IPU_DMA_CHA_DMAADC_5_WID                1
#define IPU_DMA_CHA_DMAADC_6_WID                1
#define IPU_DMA_CHA_DMAADC_7_WID                1
#define IPU_DMA_CHA_DMAPF_0_WID                 1
#define IPU_DMA_CHA_DMAPF_1_WID                 1
#define IPU_DMA_CHA_DMAPF_2_WID                 1
#define IPU_DMA_CHA_DMAPF_3_WID                 1
#define IPU_DMA_CHA_DMAPF_4_WID                 1
#define IPU_DMA_CHA_DMAPF_5_WID                 1
#define IPU_DMA_CHA_DMAPF_6_WID                 1
#define IPU_DMA_CHA_DMAPF_7_WID                 1

// IPU_FS_PROC_FLOW
#define IPU_IPU_FS_PROC_FLOW_ENC_IN_VALID_WID       1
#define IPU_IPU_FS_PROC_FLOW_VF_IN_VALID_WID        1
#define IPU_IPU_FS_PROC_FLOW_PRPENC_DEST_SEL_WID    1
#define IPU_IPU_FS_PROC_FLOW_PRPENC_ROT_SRC_SEL_WID 1
#define IPU_IPU_FS_PROC_FLOW_PRPVF_ROT_SRC_SEL_WID  1
#define IPU_IPU_FS_PROC_FLOW_PP_SRC_SEL_WID         2
#define IPU_IPU_FS_PROC_FLOW_PP_ROT_SRC_SEL_WID     2
#define IPU_IPU_FS_PROC_FLOW_PF_DEST_SEL_WID        2
#define IPU_IPU_FS_PROC_FLOW_PRPVF_DEST_SEL_WID     3
#define IPU_IPU_FS_PROC_FLOW_PRPVF_ROT_DEST_SEL_WID 3
#define IPU_IPU_FS_PROC_FLOW_PP_DEST_SEL_WID        3
#define IPU_IPU_FS_PROC_FLOW_PP_ROT_DEST_SEL_WID    3

// IPU_FS_DISP_FLOW
#define IPU_IPU_FS_DISP_FLOW_SDC0_SRC_SEL_WID   3
#define IPU_IPU_FS_DISP_FLOW_SDC1_SRC_SEL_WID   3
#define IPU_IPU_FS_DISP_FLOW_ADC2_SRC_SEL_WID   3
#define IPU_IPU_FS_DISP_FLOW_ADC3_SRC_SEL_WID   3
#define IPU_IPU_FS_DISP_FLOW_AUTO_REF_PER_WID   10

// IPU_TASKS_STAT
#define IPU_IPU_TASKS_STAT_ENC_TSTAT_WID        2
#define IPU_IPU_TASKS_STAT_VF_TSTAT_WID         2
#define IPU_IPU_TASKS_STAT_PP_TSTAT_WID         2
#define IPU_IPU_TASKS_STAT_ENC_ROT_TSTAT_WID    2
#define IPU_IPU_TASKS_STAT_VF_ROT_TSTAT_WID     2
#define IPU_IPU_TASKS_STAT_PP_ROT_TSTAT_WID     2
#define IPU_IPU_TASKS_STAT_ADCSYS1_TSTAT_WID    2
#define IPU_IPU_TASKS_STAT_ADCSYS2_TSTAT_WID    2
#define IPU_IPU_TASKS_STAT_ADC_PRPCHAN_LOCK_WID 1
#define IPU_IPU_TASKS_STAT_ADC_PPCHAN_LOCK_WID  1
#define IPU_IPU_TASKS_STAT_ADC_SYS1CHAN_LOCK_WID 1
#define IPU_IPU_TASKS_STAT_ADC_SYS2CHAN_LOCK_WID 1

// IPU_IMA_ADDR
#define IPU_IPU_IMA_ADDR_WORD_NU_WID            3
#define IPU_IPU_IMA_ADDR_ROW_NU_WID             13
#define IPU_IPU_IMA_ADDR_MEM_NU_WID             4

// IPU_IMA_DATA

//...parameters for YUV/RGB  interleaved - 1st 132 bit word

//...0th 32 bit word
#define IPU_IPU_IMA_DATA_PARAM_XV_WID       10
#define IPU_IPU_IMA_DATA_PARAM_YV_WID       10
#define IPU_IPU_IMA_DATA_PARAM_XB_WID       12

//...1st 32 bit word
#define IPU_IPU_IMA_DATA_PARAM_YB_WID       12
#define IPU_IPU_IMA_DATA_PARAM_SCE_WID      1
#define IPU_IPU_IMA_DATA_PARAM_NSB_WID      1
//  1 - reserved bit                     
#define IPU_IPU_IMA_DATA_PARAM_LNPB_WID     6
#define IPU_IPU_IMA_DATA_PARAM_SX_WID       10
#define IPU_IPU_IMA_DATA_PARAM_LOW_SY_WID   1

//...2nd 32 bit word
#define IPU_IPU_IMA_DATA_PARAM_HIGH_SY_WID  9
#define IPU_IPU_IMA_DATA_PARAM_NS_WID       10
#define IPU_IPU_IMA_DATA_PARAM_SM_WID       10
#define IPU_IPU_IMA_DATA_PARAM_LOW_SDX_WID  3

//...3rd 32 bit word
#define IPU_IPU_IMA_DATA_PARAM_HIGH_SDX_WID 2
#define IPU_IPU_IMA_DATA_PARAM_SDY_WID      5
#define IPU_IPU_IMA_DATA_PARAM_SDRX_WID     1
#define IPU_IPU_IMA_DATA_PARAM_SDRY_WID     1
#define IPU_IPU_IMA_DATA_PARAM_SCRQ_WID     1
//  2 - reserved bits
#define IPU_IPU_IMA_DATA_PARAM_FW_WID       12
#define IPU_IPU_IMA_DATA_PARAM_LOW_FH_WID   8

//...4th 32 bit word
#define IPU_IPU_IMA_DATA_PARAM_HIGH_FH_WID  4

//...parameters for YUV/RGB interleaved - 2nd 132 bit word

//...0th 32 bit word
#define IPU_IPU_IMA_DATA_PARAM_EBA0_WID     32

//...1st 32 bit word
#define IPU_IPU_IMA_DATA_PARAM_EBA1_WID     32

//...2nd 32 bit word
#define IPU_IPU_IMA_DATA_PARAM_BPP_WID      3
#define IPU_IPU_IMA_DATA_PARAM_SL_WID       14
#define IPU_IPU_IMA_DATA_PARAM_PFS_WID      3
#define IPU_IPU_IMA_DATA_PARAM_BAM_WID      3
//  2 - reserved bits
#define IPU_IPU_IMA_DATA_PARAM_NPB_WID      6
//  1 - reserved bit                     

//...3rd 32 bit word
#define IPU_IPU_IMA_DATA_PARAM_SAT_WID      2
#define IPU_IPU_IMA_DATA_PARAM_SCC_WID      1
#define IPU_IPU_IMA_DATA_PARAM_OFS0_WID     5
#define IPU_IPU_IMA_DATA_PARAM_OFS1_WID     5
#define IPU_IPU_IMA_DATA_PARAM_OFS2_WID     5
#define IPU_IPU_IMA_DATA_PARAM_OFS3_WID     5
#define IPU_IPU_IMA_DATA_PARAM_WID0_WID     3
#define IPU_IPU_IMA_DATA_PARAM_WID1_WID     3
#define IPU_IPU_IMA_DATA_PARAM_WID2_WID     3

//...4th 32 bit word
#define IPU_IPU_IMA_DATA_PARAM_WID3_WID     3
#define IPU_IPU_IMA_DATA_PARAM_DEC_SEL_WID  1


// IPU_INT_CTRL_1
#define IPU_IPU_INT_CTRL_1_DMAIC_0_EOF_EN_WID   1
#define IPU_IPU_INT_CTRL_1_DMAIC_1_EOF_EN_WID   1
#define IPU_IPU_INT_CTRL_1_DMAIC_2_EOF_EN_WID   1
#define IPU_IPU_INT_CTRL_1_DMAIC_3_EOF_EN_WID   1
#define IPU_IPU_INT_CTRL_1_DMAIC_4_EOF_EN_WID   1
#define IPU_IPU_INT_CTRL_1_DMAIC_5_EOF_EN_WID   1
#define IPU_IPU_INT_CTRL_1_DMAIC_6_EOF_EN_WID   1
#define IPU_IPU_INT_CTRL_1_DMAIC_7_EOF_EN_WID   1
#define IPU_IPU_INT_CTRL_1_DMAIC_8_EOF_EN_WID   1
#define IPU_IPU_INT_CTRL_1_DMAIC_9_EOF_EN_WID   1
#define IPU_IPU_INT_CTRL_1_DMAIC_10_EOF_EN_WID  1
#define IPU_IPU_INT_CTRL_1_DMAIC_11_EOF_EN_WID  1
#define IPU_IPU_INT_CTRL_1_DMAIC_12_EOF_EN_WID  1
#define IPU_IPU_INT_CTRL_1_DMAIC_13_EOF_EN_WID  1
#define IPU_IPU_INT_CTRL_1_DMASDC_0_EOF_EN_WID  1
#define IPU_IPU_INT_CTRL_1_DMASDC_1_EOF_EN_WID  1
#define IPU_IPU_INT_CTRL_1_DMASDC_2_EOF_EN_WID  1
#define IPU_IPU_INT_CTRL_1_DMASDC_3_EOF_EN_WID  1
#define IPU_IPU_INT_CTRL_1_DMAADC_2_EOF_EN_WID  1
#define IPU_IPU_INT_CTRL_1_DMAADC_3_EOF_EN_WID  1
#define IPU_IPU_INT_CTRL_1_DMAADC_4_EOF_EN_WID  1
#define IPU_IPU_INT_CTRL_1_DMAADC_5_EOF_EN_WID  1
#define IPU_IPU_INT_CTRL_1_DMAADC_6_EOF_EN_WID  1
#define IPU_IPU_INT_CTRL_1_DMAADC_7_EOF_EN_WID  1
#define IPU_IPU_INT_CTRL_1_DMAPF_0_EOF_EN_WID   1
#define IPU_IPU_INT_CTRL_1_DMAPF_1_EOF_EN_WID   1
#define IPU_IPU_INT_CTRL_1_DMAPF_2_EOF_EN_WID   1
#define IPU_IPU_INT_CTRL_1_DMAPF_3_EOF_EN_WID   1
#define IPU_IPU_INT_CTRL_1_DMAPF_4_EOF_EN_WID   1
#define IPU_IPU_INT_CTRL_1_DMAPF_5_EOF_EN_WID   1
#define IPU_IPU_INT_CTRL_1_DMAPF_6_EOF_EN_WID   1
#define IPU_IPU_INT_CTRL_1_DMAPF_7_EOF_EN_WID   1

// IPU_INT_CTRL_3
#define IPU_IPU_INT_CTRL_3_ADC_DISP0_VSYNC_WID  1
#define IPU_IPU_INT_CTRL_3_ADC_PRP_EOF_EN_WID   1
#define IPU_IPU_INT_CTRL_3_ADC_PP_EOF_EN_WID    1
#define IPU_IPU_INT_CTRL_3_ADC_SYS1_EOF_EN_WID  1
#define IPU_IPU_INT_CTRL_3_ADC_SYS2_EOF_EN_WID  1

// IPU CSI Registers
#define IPU_CSI_SENS_CONF_VSYNC_POL_WID         1
#define IPU_CSI_SENS_CONF_HSYNC_POL_WID         1
#define IPU_CSI_SENS_CONF_DATA_POL_WID          1
#define IPU_CSI_SENS_CONF_DISP_PIX_CLK_POL_WID  1
#define IPU_CSI_SENS_CONF_SENS_PRTCL_WID        3
#define IPU_CSI_SENS_CONF_SENS_CLK_SRC_WID      1
#define IPU_CSI_SENS_CONF_SENS_DATA_FORMAT_WID  2
#define IPU_CSI_SENS_CONF_DATA_WIDTH_WID        2
#define IPU_CSI_SENS_CONF_EXT_VSYNC_WID         1
#define IPU_CSI_SENS_CONF_DIV_RATIO_WID         8
#define IPU_CSI_SENS_CONF_MCLK_PIXCLK_RATIO_WID 4

#define IPU_CSI_SENS_FRM_SIZE_SENS_FRM_WIDTH_WID  12
#define IPU_CSI_SENS_FRM_SIZE_SENS_FRM_HEIGHT_WID 12

#define IPU_CSI_ACT_FRM_SIZE_ACT_FRM_WIDTH_WID  12
#define IPU_CSI_ACT_FRM_SIZE_ACT_FRM_HEIGHT_WID 12

#define IPU_CSI_OUT_FRM_CTRL_VSC_WID            8
#define IPU_CSI_OUT_FRM_CTRL_HSC_WID            8
#define IPU_CSI_OUT_FRM_CTRL_SKIP_ENC_WID       5
#define IPU_CSI_OUT_FRM_CTRL_SKIP_VF_WID        5
#define IPU_CSI_OUT_FRM_CTRL_IC_TV_MODE_WID     1
#define IPU_CSI_OUT_FRM_CTRL_VERT_DWNS_WID      1
#define IPU_CSI_OUT_FRM_CTRL_HORZ_DWNS_WID      1

#define IPU_CSI_TST_CTRL_PG_R_VALUE_WID         8
#define IPU_CSI_TST_CTRL_PG_G_VALUE_WID         8
#define IPU_CSI_TST_CTRL_PG_B_VALUE_WID         8
#define IPU_CSI_TST_CTRL_TEST_GEN_MODE_WID      1

#define IPU_CSI_CCIR_CODE_1_END_FLD0_BLNK_1ST_WID   3
#define IPU_CSI_CCIR_CODE_1_STRT_FLD0_BLNK_1ST_WID  3
#define IPU_CSI_CCIR_CODE_1_END_FLD0_BLNK_2ND_WID   3
#define IPU_CSI_CCIR_CODE_1_STRT_FLD0_BLNK_2ND_WID  3
#define IPU_CSI_CCIR_CODE_1_END_FLD0_ACTV_WID       3
#define IPU_CSI_CCIR_CODE_1_STRT_FLD0_ACTV_WID      3
#define IPU_CSI_CCIR_CODE_1_CCIR_ERR_DET_EN_WID     1

#define IPU_CSI_CCIR_CODE_2_END_FLD1_BLNK_1ST_WID   3
#define IPU_CSI_CCIR_CODE_2_STRT_FLD1_BLNK_1ST_WID  3
#define IPU_CSI_CCIR_CODE_2_END_FLD1_BLNK_2ND_WID   3
#define IPU_CSI_CCIR_CODE_2_STRT_FLD1_BLNK_2ND_WID  3
#define IPU_CSI_CCIR_CODE_2_END_FLD1_ACTV_WID       3
#define IPU_CSI_CCIR_CODE_2_STRT_FLD1_ACTV_WID      3

#define IPU_CSI_CCIR_CODE_3_CCIR_PRECOM_WID         24

#define IPU_CSI_FLASH_STROBE_1_CLOCK_SEL_WID           1
#define IPU_CSI_FLASH_STROBE_1_SENSE_ROW_DURATION_WID  16

#define IPU_CSI_FLASH_STROBE_2_STROBE_EN_WID           1
#define IPU_CSI_FLASH_STROBE_2_STROBE_POL_WID          1
#define IPU_CSI_FLASH_STROBE_2_STROBE_START_TIME_WID   13
#define IPU_CSI_FLASH_STROBE_2_STROBE_DURATION_WID     16


// IPU IC Registers
#define IPU_IC_CONF_PRPENC_EN_WID               1
#define IPU_IC_CONF_PRPENC_CSC1_WID             1
#define IPU_IC_CONF_PRPENC_ROT_EN_WID           1
#define IPU_IC_CONF_PRPVF_EN_WID                1
#define IPU_IC_CONF_PRPVF_CSC1_WID              1
#define IPU_IC_CONF_PRPVF_CSC2_WID              1
#define IPU_IC_CONF_PRPVF_CMB_WID               1
#define IPU_IC_CONF_PRPVF_ROT_EN_WID            1
#define IPU_IC_CONF_PP_EN_WID                   1
#define IPU_IC_CONF_PP_CSC1_WID                 1
#define IPU_IC_CONF_PP_CSC2_WID                 1
#define IPU_IC_CONF_PP_CMB_WID                  1
#define IPU_IC_CONF_PP_ROT_EN_WID               1
#define IPU_IC_CONF_IC_GLB_LOC_A_WID            1
#define IPU_IC_CONF_IC_KEY_COLOR_EN_WID         1
#define IPU_IC_CONF_RWS_EN_WID                  1
#define IPU_IC_CONF_CSI_MEM_WR_EN_WID           1

#define IPU_IC_PRP_ENC_RSC_PRPENC_RS_R_H_WID    14
#define IPU_IC_PRP_ENC_RSC_PRPENC_DS_R_H_WID    2
#define IPU_IC_PRP_ENC_RSC_PRPENC_RS_R_V_WID    14
#define IPU_IC_PRP_ENC_RSC_PRPENC_DS_R_V_WID    2

#define IPU_IC_PRP_VF_RSC_PRPVF_RS_R_H_WID      14
#define IPU_IC_PRP_VF_RSC_PRPVF_DS_R_H_WID      2
#define IPU_IC_PRP_VF_RSC_PRPVF_RS_R_V_WID      14
#define IPU_IC_PRP_VF_RSC_PRPVF_DS_R_V_WID      2

#define IPU_IC_PP_RSC_PP_RS_R_H_WID             14
#define IPU_IC_PP_RSC_PP_DS_R_H_WID             2
#define IPU_IC_PP_RSC_PP_RS_R_V_WID             14
#define IPU_IC_PP_RSC_PP_DS_R_V_WID             2

#define IPU_IC_CMBP_1_IC_PRPVF_ALPHA_V_WID      8
#define IPU_IC_CMBP_1_IC_PP_ALPHA_V_WID         8

#define IPU_IC_CMBP_2_IC_KEY_COLOR_B_WID        8
#define IPU_IC_CMBP_2_IC_KEY_COLOR_G_WID        8
#define IPU_IC_CMBP_2_IC_KEY_COLOR_R_WID        8


// PF_CONF
#define IPU_PF_CONF_PF_TYPE_WID                 3
#define IPU_PF_CONF_H264_Y_PAUSE_EN_WID         1
#define IPU_PF_CONF_H264_Y_PAUSE_ROW_WID        6


// IDMAC_CONF
#define IPU_IDMAC_CONF_PRYM_WID                 2
#define IPU_IDMAC_CONF_SRCNT_WID                3
#define IPU_IDMAC_CONF_SINGLE_AHB_M_EN_WID      1


// SDC_COM_CONF
#define IPU_SDC_COM_CONF_SDC_MODE_WID           2
#define IPU_SDC_COM_CONF_BG_MCP_FROM_WID        1
#define IPU_SDC_COM_CONF_FG_MCP_FROM_WID        1
#define IPU_SDC_COM_CONF_FG_EN_WID              1
#define IPU_SDC_COM_CONF_GWSEL_WID              1
#define IPU_SDC_COM_CONF_SDC_GLB_LOC_A_WID      1
#define IPU_SDC_COM_CONF_SDC_KEY_COLOR_EN_WID   1
#define IPU_SDC_COM_CONF_MASK_EN_WID            1
#define IPU_SDC_COM_CONF_BG_EN_WID              1
#define IPU_SDC_COM_CONF_SHARP_WID              1
#define IPU_SDC_COM_CONF_DUAL_MODE_WID          1
#define IPU_SDC_COM_CONF_COC_WID                3

// SDC_GRAPH_WIND_CTRL
#define IPU_SDC_GRAPH_WIND_CTRL_SDC_KEY_COLOR_B_WID    8
#define IPU_SDC_GRAPH_WIND_CTRL_SDC_KEY_COLOR_G_WID    8
#define IPU_SDC_GRAPH_WIND_CTRL_SDC_KEY_COLOR_R_WID    8
#define IPU_SDC_GRAPH_WIND_CTRL_SDC_ALPHA_V_WID        8

// SDC_FG_POS
#define IPU_SDC_FG_POS_FGYP_WID                    10
#define IPU_SDC_FG_POS_FGXP_WID                    10

// SDC_BG_POS
#define IPU_SDC_BG_POS_BGYP_WID                    10
#define IPU_SDC_BG_POS_BGXP_WID                    10

// SDC_CUR_POS
#define IPU_SDC_CUR_POS_CYP_WID                    10
#define IPU_SDC_CUR_POS_CYH_WID                    5
#define IPU_SDC_CUR_POS_CXP_WID                    10
#define IPU_SDC_CUR_POS_CXW_WID                    5

// SDC_CUR_BLINK_PWM_CTRL
#define IPU_SDC_CUR_BLINK_PWM_CTRL_BKDIV_WID    8
#define IPU_SDC_CUR_BLINK_PWM_CTRL_PWM_WID      8
#define IPU_SDC_CUR_BLINK_PWM_CTRL_BK_EN_WID    1
#define IPU_SDC_CUR_BLINK_PWM_CTRL_CC_EN_WID    1
#define IPU_SDC_CUR_BLINK_PWM_CTRL_SCR_WID      2

// SDC_CUR_MAP
#define IPU_SDC_CUR_MAP_CUR_COL_B_WID           8
#define IPU_SDC_CUR_MAP_CUR_COL_G_WID           8
#define IPU_SDC_CUR_MAP_CUR_COL_R_WID           8

// SDC_HOR_CONF
#define IPU_SDC_HOR_CONF_SCREEN_WIDTH_WID       10
#define IPU_SDC_HOR_CONF_H_SYNC_WIDTH_WID       6

// SDC_VER_CONF
#define IPU_SDC_VER_CONF_V_SYNC_WIDTH_L_WID     1
#define IPU_SDC_VER_CONF_SCREEN_HEIGHT_WID      10
#define IPU_SDC_VER_CONF_V_SYNC_WIDTH_WID       6

// SDC_SHARP_CONF_1
#define IPU_SDC_SHARP_CONF_1_CLS_RISE_DELAY_WID     8
#define IPU_SDC_SHARP_CONF_1_PS_FALL_DELAY_WID      8
#define IPU_SDC_SHARP_CONF_1_REV_TOGGLE_DELAY_WID   10

// SDC_SHARP_CONF_2
#define IPU_SDC_SHARP_CONF_2_CLS_FALL_DELAY_WID 10
#define IPU_SDC_SHARP_CONF_2_PS_RISE_DELAY_WID  10

// ADC_CONF
#define IPU_ADC_CONF_PRP_CHAN_EN_WID            1
#define IPU_ADC_CONF_PP_CHAN_EN_WID             1
#define IPU_ADC_CONF_MCU_CHAN_EN_WID            1
#define IPU_ADC_CONF_PRP_DISP_NUM_WID           2
#define IPU_ADC_CONF_PRP_ADDR_INC_WID           2
#define IPU_ADC_CONF_PRP_DATA_MAP_WID           1
#define IPU_ADC_CONF_PP_DISP_NUM_WID            2
#define IPU_ADC_CONF_PP_ADDR_INC_WID            2
#define IPU_ADC_CONF_PP_DATA_MAP_WID            1
#define IPU_ADC_CONF_PP_NO_TEARING_WID          1
#define IPU_ADC_CONF_SYS1_NO_TEARING_WID        1
#define IPU_ADC_CONF_SYS2_NO_TEARING_WID        1
#define IPU_ADC_CONF_SYS1_MODE_WID              3
#define IPU_ADC_CONF_SYS1_DISP_NUM_WID          2
#define IPU_ADC_CONF_SYS1_ADDR_INC_WID          2
#define IPU_ADC_CONF_SYS1_DATA_MAP_WID          1
#define IPU_ADC_CONF_SYS2_MODE_WID              2
#define IPU_ADC_CONF_SYS2_DISP_NUM_WID          2
#define IPU_ADC_CONF_SYS2_ADDR_INC_WID          2
#define IPU_ADC_CONF_SYS2_DATA_MAP_WID          1

// DI_HSP_CLK_PER
#define IPU_DI_HSP_CLK_PER_HSP_CLK_PERIOD_1_WID 7
#define IPU_DI_HSP_CLK_PER_HSP_CLK_PERIOD_2_WID 7

// DI_DISP0_TIME_CONF_1
#define IPU_DISP0_TIME_CONF_1_DISP0_IF_CLK_PER_WR_WID   12
#define IPU_DISP0_TIME_CONF_1_DISP0_IF_CLK_UP_WR_WID    10
#define IPU_DISP0_TIME_CONF_1_DISP0_IF_CLK_DOWN_WR_WID  10

// DI_DISP0_TIME_CONF_2
#define IPU_DISP0_TIME_CONF_2_DISP0_IF_CLK_PER_RD_WID   12
#define IPU_DISP0_TIME_CONF_2_DISP0_IF_CLK_UP_RD_WID    10
#define IPU_DISP0_TIME_CONF_2_DISP0_IF_CLK_DOWN_RD_WID  10

// DI_DISP0_TIME_CONF_3
#define IPU_DISP0_TIME_CONF_3_DISP0_PIX_CLK_PER_WID   12
#define IPU_DISP0_TIME_CONF_3_DISP0_READ_EN_WID       10
#define IPU_DISP0_TIME_CONF_3_DISP0_RD_WAIT_ST_WID    2


// DI_DISP3_TIME_CONF
#define IPU_DI_DISP3_TIME_CONF_DISP3_IF_CLK_PER_WR_WID  12
#define IPU_DI_DISP3_TIME_CONF_DISP3_IF_CLK_UP_WR_WID   10
#define IPU_DI_DISP3_TIME_CONF_DISP3_IF_CLK_DOWN_WR_WID 10



//ADC_SYSCHA1_SA
//ADC_SYSCHA2_SA
//ADC_PRPCHAN_SA
//ADC_PPCHAN_SA
#define IPU_ADC_CHA_CHAN_SA_WID                 23
#define IPU_ADC_CHA_START_TIME_WID              9

// ADC_DISP0_CONF
// ADC_DISP1_CONF
// ADC_DISP2_CONF
#define IPU_ADC_DISP_CONF_DISP_SL_WID           12
#define IPU_ADC_DISP_CONF_DISP_TYPE_WID         2
#define IPU_ADC_DISP_CONF_DISP_DATA_WIDTH_WID   1
#define IPU_ADC_DISP_CONF_DISP_DATA_MAP_WID     1

// ADC_DISP0_RD_AP
#define IPU_ADC_DISP0_RD_AP_DISP0_ACK_PTRN_WID  24

// ADC_DISP0_RDM
#define IPU_ADC_DISP0_RDM_DISP0_MASK_ACK_DATA_WID   24


// ADC_DISP0_SS
// ADC_DISP1_SS
// ADC_DISP2_SS
#define IPU_ADC_DISP_SS_SCREEN_WIDTH_WID        10
#define IPU_ADC_DISP_SS_SCREEN_HEIGHT_WID       10

// ADC_DISP_VSYNC
#define IPU_ADC_DISP_VSYNC_DISP0_VSYNC_MODE_WID     2
#define IPU_ADC_DISP_VSYNC_DISP12_VSYNC_MODE_WID    2
#define IPU_ADC_DISP_VSYNC_DISP12_VSYNC_SEL_WID     1
#define IPU_ADC_DISP_VSYNC_DISP_LN_WT_WID           10
#define IPU_ADC_DISP_VSYNC_DISP0_VSYNC_WIDTH_WID    6
#define IPU_ADC_DISP_VSYNC_DISP0_VSYNC_WIDTH_L_WID  1
#define IPU_ADC_DISP_VSYNC_DISP12_VSYNC_WIDTH_WID   6
#define IPU_ADC_DISP_VSYNC_DISP12_VSYNC_WIDTH_L_WID 1


// DI_DISP_IF_CONF
#define IPU_DI_DISP_IF_CONF_DISP0_EN_WID                1
#define IPU_DI_DISP_IF_CONF_DISP0_IF_MODE_WID           2
#define IPU_DI_DISP_IF_CONF_DISP0_PAR_BURST_MODE_WID    2
#define IPU_DI_DISP_IF_CONF_DISP1_EN_WID                1
#define IPU_DI_DISP_IF_CONF_DISP1_IF_MODE_WID           3
#define IPU_DI_DISP_IF_CONF_DISP1_PAR_BURST_MODE_WID    2
#define IPU_DI_DISP_IF_CONF_DISP2_EN_WID                1
#define IPU_DI_DISP_IF_CONF_DISP2_IF_MODE_WID           3
#define IPU_DI_DISP_IF_CONF_DISP2_PAR_BURST_MODE_WID    2
#define IPU_DI_DISP_IF_CONF_DISP3_DATAMASK_WID          1
#define IPU_DI_DISP_IF_CONF_DISP3_CLK_SEL_WID           1
#define IPU_DI_DISP_IF_CONF_DISP3_CLK_IDLE_WID          1
#define IPU_DI_DISP_IF_CONF_DISP012_DEAD_CLK_NUM_WID    4


// DI_DISP_SIG_POL
#define IPU_DI_DISP_SIG_POL_D0_DATA_POL_WID             1
#define IPU_DI_DISP_SIG_POL_D0_CS_POL_WID               1
#define IPU_DI_DISP_SIG_POL_D0_PAR_RS_POL_WID           1
#define IPU_DI_DISP_SIG_POL_D0_WR_POL_WID               1
#define IPU_DI_DISP_SIG_POL_D0_RD_POL_WID               1
#define IPU_DI_DISP_SIG_POL_D0_VSYNC_POL_WID            1
#define IPU_DI_DISP_SIG_POL_D12_VSYNC_POL_WID           1
//#define IPU_DI_DISP_SIG_POL_RESERVED                  1
#define IPU_DI_DISP_SIG_POL_D1_DATA_POL_WID             1
#define IPU_DI_DISP_SIG_POL_D1_CS_POL_WID               1
#define IPU_DI_DISP_SIG_POL_D1_PAR_RS_POL_WID           1
#define IPU_DI_DISP_SIG_POL_D1_WR_POL_WID               1
#define IPU_DI_DISP_SIG_POL_D1_RD_POL_WID               1
#define IPU_DI_DISP_SIG_POL_D1_SD_D_POL_WID             1
#define IPU_DI_DISP_SIG_POL_D1_SD_CLK_POL_WID           1
#define IPU_DI_DISP_SIG_POL_D1_SER_RS_POL_WID           1
#define IPU_DI_DISP_SIG_POL_D2_DATA_POL_WID             1
#define IPU_DI_DISP_SIG_POL_D2_CS_POL_WID               1
#define IPU_DI_DISP_SIG_POL_D2_PAR_RS_POL_WID           1
#define IPU_DI_DISP_SIG_POL_D2_WR_POL_WID               1
#define IPU_DI_DISP_SIG_POL_D2_RD_POL_WID               1
#define IPU_DI_DISP_SIG_POL_D2_SD_D_POL_WID             1
#define IPU_DI_DISP_SIG_POL_D2_SD_CLK_POL_WID           1
#define IPU_DI_DISP_SIG_POL_D2_SER_RS_POL_WID           1
#define IPU_DI_DISP_SIG_POL_D3_DATA_POL_WID             1
#define IPU_DI_DISP_SIG_POL_D3_CLK_POL_WID              1
#define IPU_DI_DISP_SIG_POL_D3_DRDY_SHARP_POL_WID       1
#define IPU_DI_DISP_SIG_POL_D3_HSYNC_POL_WID            1
#define IPU_DI_DISP_SIG_POL_D3_VSYNC_POL_WID            1
#define IPU_DI_DISP_SIG_POL_D0_BCLK_POL_WID             1
#define IPU_DI_DISP_SIG_POL_D1_BCLK_POL_WID             1
#define IPU_DI_DISP_SIG_POL_D2_BCLK_POL_WID             1




// DI_DISP0_DB0_MAP
#define IPU_DI_DISP0_DB0_MAP_MD00_M0_WID        2
#define IPU_DI_DISP0_DB0_MAP_MD00_M1_WID        2
#define IPU_DI_DISP0_DB0_MAP_MD00_M2_WID        2
#define IPU_DI_DISP0_DB0_MAP_MD00_M3_WID        2
#define IPU_DI_DISP0_DB0_MAP_MD00_M4_WID        2
#define IPU_DI_DISP0_DB0_MAP_MD00_M5_WID        2
#define IPU_DI_DISP0_DB0_MAP_MD00_M6_WID        2
#define IPU_DI_DISP0_DB0_MAP_MD00_M7_WID        2
#define IPU_DI_DISP0_DB0_MAP_MD00_OFFS0_WID     5
#define IPU_DI_DISP0_DB0_MAP_MD00_OFFS1_WID     5
#define IPU_DI_DISP0_DB0_MAP_MD00_OFFS2_WID     5

// DI_DISP0_DB1_MAP
#define IPU_DI_DISP0_DB1_MAP_MD01_M0_WID        2
#define IPU_DI_DISP0_DB1_MAP_MD01_M1_WID        2
#define IPU_DI_DISP0_DB1_MAP_MD01_M2_WID        2
#define IPU_DI_DISP0_DB1_MAP_MD01_M3_WID        2
#define IPU_DI_DISP0_DB1_MAP_MD01_M4_WID        2
#define IPU_DI_DISP0_DB1_MAP_MD01_M5_WID        2
#define IPU_DI_DISP0_DB1_MAP_MD01_M6_WID        2
#define IPU_DI_DISP0_DB1_MAP_MD01_M7_WID        2
#define IPU_DI_DISP0_DB1_MAP_MD01_OFFS0_WID     5
#define IPU_DI_DISP0_DB1_MAP_MD01_OFFS1_WID     5
#define IPU_DI_DISP0_DB1_MAP_MD01_OFFS2_WID     5

// DI_DISP0_DB2_MAP
#define IPU_DI_DISP0_DB2_MAP_MD02_M0_WID        2
#define IPU_DI_DISP0_DB2_MAP_MD02_M1_WID        2
#define IPU_DI_DISP0_DB2_MAP_MD02_M2_WID        2
#define IPU_DI_DISP0_DB2_MAP_MD02_M3_WID        2
#define IPU_DI_DISP0_DB2_MAP_MD02_M4_WID        2
#define IPU_DI_DISP0_DB2_MAP_MD02_M5_WID        2
#define IPU_DI_DISP0_DB2_MAP_MD02_M6_WID        2
#define IPU_DI_DISP0_DB2_MAP_MD02_M7_WID        2
#define IPU_DI_DISP0_DB2_MAP_MD02_OFFS0_WID     5
#define IPU_DI_DISP0_DB2_MAP_MD02_OFFS1_WID     5
#define IPU_DI_DISP0_DB2_MAP_MD02_OFFS2_WID     5

// DI_DISP0_CB0_MAP
#define IPU_DI_DISP0_CB0_MAP_MC00_M0_WID        2
#define IPU_DI_DISP0_CB0_MAP_MC00_M1_WID        2
#define IPU_DI_DISP0_CB0_MAP_MC00_M2_WID        2
#define IPU_DI_DISP0_CB0_MAP_MC00_M3_WID        2
#define IPU_DI_DISP0_CB0_MAP_MC00_M4_WID        2
#define IPU_DI_DISP0_CB0_MAP_MC00_M5_WID        2
#define IPU_DI_DISP0_CB0_MAP_MC00_M6_WID        2
#define IPU_DI_DISP0_CB0_MAP_MC00_M7_WID        2
#define IPU_DI_DISP0_CB0_MAP_MC00_OFFS0_WID     5
#define IPU_DI_DISP0_CB0_MAP_MC00_OFFS1_WID     5
#define IPU_DI_DISP0_CB0_MAP_MC00_OFFS2_WID     5

// DI_DISP0_CB1_MAP
#define IPU_DI_DISP0_CB1_MAP_MC01_M0_WID        2
#define IPU_DI_DISP0_CB1_MAP_MC01_M1_WID        2
#define IPU_DI_DISP0_CB1_MAP_MC01_M2_WID        2
#define IPU_DI_DISP0_CB1_MAP_MC01_M3_WID        2
#define IPU_DI_DISP0_CB1_MAP_MC01_M4_WID        2
#define IPU_DI_DISP0_CB1_MAP_MC01_M5_WID        2
#define IPU_DI_DISP0_CB1_MAP_MC01_M6_WID        2
#define IPU_DI_DISP0_CB1_MAP_MC01_M7_WID        2
#define IPU_DI_DISP0_CB1_MAP_MC01_OFFS0_WID     5
#define IPU_DI_DISP0_CB1_MAP_MC01_OFFS1_WID     5
#define IPU_DI_DISP0_CB1_MAP_MC01_OFFS2_WID     5

// DI_DISP0_CB2_MAP
#define IPU_DI_DISP0_CB2_MAP_MC02_M0_WID        2
#define IPU_DI_DISP0_CB2_MAP_MC02_M1_WID        2
#define IPU_DI_DISP0_CB2_MAP_MC02_M2_WID        2
#define IPU_DI_DISP0_CB2_MAP_MC02_M3_WID        2
#define IPU_DI_DISP0_CB2_MAP_MC02_M4_WID        2
#define IPU_DI_DISP0_CB2_MAP_MC02_M5_WID        2
#define IPU_DI_DISP0_CB2_MAP_MC02_M6_WID        2
#define IPU_DI_DISP0_CB2_MAP_MC02_M7_WID        2
#define IPU_DI_DISP0_CB2_MAP_MC02_OFFS0_WID     5
#define IPU_DI_DISP0_CB2_MAP_MC02_OFFS1_WID     5
#define IPU_DI_DISP0_CB2_MAP_MC02_OFFS2_WID     5

// DI_DISP1_DB0_MAP
#define IPU_DI_DISP1_DB0_MAP_MD10_M0_WID        2
#define IPU_DI_DISP1_DB0_MAP_MD10_M1_WID        2
#define IPU_DI_DISP1_DB0_MAP_MD10_M2_WID        2
#define IPU_DI_DISP1_DB0_MAP_MD10_M3_WID        2
#define IPU_DI_DISP1_DB0_MAP_MD10_M4_WID        2
#define IPU_DI_DISP1_DB0_MAP_MD10_M5_WID        2
#define IPU_DI_DISP1_DB0_MAP_MD10_M6_WID        2
#define IPU_DI_DISP1_DB0_MAP_MD10_M7_WID        2
#define IPU_DI_DISP1_DB0_MAP_MD10_OFFS0_WID     5
#define IPU_DI_DISP1_DB0_MAP_MD10_OFFS1_WID     5
#define IPU_DI_DISP1_DB0_MAP_MD10_OFFS2_WID     5

// DI_DISP1_DB1_MAP
#define IPU_DI_DISP1_DB1_MAP_MD11_M0_WID        2
#define IPU_DI_DISP1_DB1_MAP_MD11_M1_WID        2
#define IPU_DI_DISP1_DB1_MAP_MD11_M2_WID        2
#define IPU_DI_DISP1_DB1_MAP_MD11_M3_WID        2
#define IPU_DI_DISP1_DB1_MAP_MD11_M4_WID        2
#define IPU_DI_DISP1_DB1_MAP_MD11_M5_WID        2
#define IPU_DI_DISP1_DB1_MAP_MD11_M6_WID        2
#define IPU_DI_DISP1_DB1_MAP_MD11_M7_WID        2
#define IPU_DI_DISP1_DB1_MAP_MD11_OFFS0_WID     5
#define IPU_DI_DISP1_DB1_MAP_MD11_OFFS1_WID     5
#define IPU_DI_DISP1_DB1_MAP_MD11_OFFS2_WID     5

// DI_DISP1_DB2_MAP
#define IPU_DI_DISP1_DB2_MAP_MD12_M0_WID        2
#define IPU_DI_DISP1_DB2_MAP_MD12_M1_WID        2
#define IPU_DI_DISP1_DB2_MAP_MD12_M2_WID        2
#define IPU_DI_DISP1_DB2_MAP_MD12_M3_WID        2
#define IPU_DI_DISP1_DB2_MAP_MD12_M4_WID        2
#define IPU_DI_DISP1_DB2_MAP_MD12_M5_WID        2
#define IPU_DI_DISP1_DB2_MAP_MD12_M6_WID        2
#define IPU_DI_DISP1_DB2_MAP_MD12_M7_WID        2
#define IPU_DI_DISP1_DB2_MAP_MD12_OFFS0_WID     5
#define IPU_DI_DISP1_DB2_MAP_MD12_OFFS1_WID     5
#define IPU_DI_DISP1_DB2_MAP_MD12_OFFS2_WID     5

// DI_DISP1_CB0_MAP
#define IPU_DI_DISP1_CB0_MAP_MC10_M0_WID        2
#define IPU_DI_DISP1_CB0_MAP_MC10_M1_WID        2
#define IPU_DI_DISP1_CB0_MAP_MC10_M2_WID        2
#define IPU_DI_DISP1_CB0_MAP_MC10_M3_WID        2
#define IPU_DI_DISP1_CB0_MAP_MC10_M4_WID        2
#define IPU_DI_DISP1_CB0_MAP_MC10_M5_WID        2
#define IPU_DI_DISP1_CB0_MAP_MC10_M6_WID        2
#define IPU_DI_DISP1_CB0_MAP_MC10_M7_WID        2
#define IPU_DI_DISP1_CB0_MAP_MC10_OFFS0_WID     5
#define IPU_DI_DISP1_CB0_MAP_MC10_OFFS1_WID     5
#define IPU_DI_DISP1_CB0_MAP_MC10_OFFS2_WID     5

// DI_DISP1_CB1_MAP
#define IPU_DI_DISP1_CB1_MAP_MC11_M0_WID        2
#define IPU_DI_DISP1_CB1_MAP_MC11_M1_WID        2
#define IPU_DI_DISP1_CB1_MAP_MC11_M2_WID        2
#define IPU_DI_DISP1_CB1_MAP_MC11_M3_WID        2
#define IPU_DI_DISP1_CB1_MAP_MC11_M4_WID        2
#define IPU_DI_DISP1_CB1_MAP_MC11_M5_WID        2
#define IPU_DI_DISP1_CB1_MAP_MC11_M6_WID        2
#define IPU_DI_DISP1_CB1_MAP_MC11_M7_WID        2
#define IPU_DI_DISP1_CB1_MAP_MC11_OFFS0_WID     5
#define IPU_DI_DISP1_CB1_MAP_MC11_OFFS1_WID     5
#define IPU_DI_DISP1_CB1_MAP_MC11_OFFS2_WID     5

// DI_DISP1_CB2_MAP
#define IPU_DI_DISP1_CB2_MAP_MC12_M0_WID        2
#define IPU_DI_DISP1_CB2_MAP_MC12_M1_WID        2
#define IPU_DI_DISP1_CB2_MAP_MC12_M2_WID        2
#define IPU_DI_DISP1_CB2_MAP_MC12_M3_WID        2
#define IPU_DI_DISP1_CB2_MAP_MC12_M4_WID        2
#define IPU_DI_DISP1_CB2_MAP_MC12_M5_WID        2
#define IPU_DI_DISP1_CB2_MAP_MC12_M6_WID        2
#define IPU_DI_DISP1_CB2_MAP_MC12_M7_WID        2
#define IPU_DI_DISP1_CB2_MAP_MC12_OFFS0_WID     5
#define IPU_DI_DISP1_CB2_MAP_MC12_OFFS1_WID     5
#define IPU_DI_DISP1_CB2_MAP_MC12_OFFS2_WID     5

// DI_DISP2_DB0_MAP
#define IPU_DI_DISP2_DB0_MAP_MD20_M0_WID        2
#define IPU_DI_DISP2_DB0_MAP_MD20_M1_WID        2
#define IPU_DI_DISP2_DB0_MAP_MD20_M2_WID        2
#define IPU_DI_DISP2_DB0_MAP_MD20_M3_WID        2
#define IPU_DI_DISP2_DB0_MAP_MD20_M4_WID        2
#define IPU_DI_DISP2_DB0_MAP_MD20_M5_WID        2
#define IPU_DI_DISP2_DB0_MAP_MD20_M6_WID        2
#define IPU_DI_DISP2_DB0_MAP_MD20_M7_WID        2
#define IPU_DI_DISP2_DB0_MAP_MD20_OFFS0_WID     5
#define IPU_DI_DISP2_DB0_MAP_MD20_OFFS1_WID     5
#define IPU_DI_DISP2_DB0_MAP_MD20_OFFS2_WID     5

// DI_DISP2_DB1_MAP
#define IPU_DI_DISP2_DB1_MAP_MD21_M0_WID        2
#define IPU_DI_DISP2_DB1_MAP_MD21_M1_WID        2
#define IPU_DI_DISP2_DB1_MAP_MD21_M2_WID        2
#define IPU_DI_DISP2_DB1_MAP_MD21_M3_WID        2
#define IPU_DI_DISP2_DB1_MAP_MD21_M4_WID        2
#define IPU_DI_DISP2_DB1_MAP_MD21_M5_WID        2
#define IPU_DI_DISP2_DB1_MAP_MD21_M6_WID        2
#define IPU_DI_DISP2_DB1_MAP_MD21_M7_WID        2
#define IPU_DI_DISP2_DB1_MAP_MD21_OFFS0_WID     5
#define IPU_DI_DISP2_DB1_MAP_MD21_OFFS1_WID     5
#define IPU_DI_DISP2_DB1_MAP_MD21_OFFS2_WID     5

// DI_DISP2_DB2_MAP
#define IPU_DI_DISP2_DB2_MAP_MD22_M0_WID        2
#define IPU_DI_DISP2_DB2_MAP_MD22_M1_WID        2
#define IPU_DI_DISP2_DB2_MAP_MD22_M2_WID        2
#define IPU_DI_DISP2_DB2_MAP_MD22_M3_WID        2
#define IPU_DI_DISP2_DB2_MAP_MD22_M4_WID        2
#define IPU_DI_DISP2_DB2_MAP_MD22_M5_WID        2
#define IPU_DI_DISP2_DB2_MAP_MD22_M6_WID        2
#define IPU_DI_DISP2_DB2_MAP_MD22_M7_WID        2
#define IPU_DI_DISP2_DB2_MAP_MD22_OFFS0_WID     5
#define IPU_DI_DISP2_DB2_MAP_MD22_OFFS1_WID     5
#define IPU_DI_DISP2_DB2_MAP_MD22_OFFS2_WID     5

// DI_DISP2_CB0_MAP
#define IPU_DI_DISP2_CB0_MAP_MC20_M0_WID        2
#define IPU_DI_DISP2_CB0_MAP_MC20_M1_WID        2
#define IPU_DI_DISP2_CB0_MAP_MC20_M2_WID        2
#define IPU_DI_DISP2_CB0_MAP_MC20_M3_WID        2
#define IPU_DI_DISP2_CB0_MAP_MC20_M4_WID        2
#define IPU_DI_DISP2_CB0_MAP_MC20_M5_WID        2
#define IPU_DI_DISP2_CB0_MAP_MC20_M6_WID        2
#define IPU_DI_DISP2_CB0_MAP_MC20_M7_WID        2
#define IPU_DI_DISP2_CB0_MAP_MC20_OFFS0_WID     5
#define IPU_DI_DISP2_CB0_MAP_MC20_OFFS1_WID     5
#define IPU_DI_DISP2_CB0_MAP_MC20_OFFS2_WID     5

// DI_DISP2_CB1_MAP
#define IPU_DI_DISP2_CB1_MAP_MC21_M0_WID        2
#define IPU_DI_DISP2_CB1_MAP_MC21_M1_WID        2
#define IPU_DI_DISP2_CB1_MAP_MC21_M2_WID        2
#define IPU_DI_DISP2_CB1_MAP_MC21_M3_WID        2
#define IPU_DI_DISP2_CB1_MAP_MC21_M4_WID        2
#define IPU_DI_DISP2_CB1_MAP_MC21_M5_WID        2
#define IPU_DI_DISP2_CB1_MAP_MC21_M6_WID        2
#define IPU_DI_DISP2_CB1_MAP_MC21_M7_WID        2
#define IPU_DI_DISP2_CB1_MAP_MC21_OFFS0_WID     5
#define IPU_DI_DISP2_CB1_MAP_MC21_OFFS1_WID     5
#define IPU_DI_DISP2_CB1_MAP_MC21_OFFS2_WID     5

// DI_DISP2_CB2_MAP
#define IPU_DI_DISP2_CB2_MAP_MC22_M0_WID        2
#define IPU_DI_DISP2_CB2_MAP_MC22_M1_WID        2
#define IPU_DI_DISP2_CB2_MAP_MC22_M2_WID        2
#define IPU_DI_DISP2_CB2_MAP_MC22_M3_WID        2
#define IPU_DI_DISP2_CB2_MAP_MC22_M4_WID        2
#define IPU_DI_DISP2_CB2_MAP_MC22_M5_WID        2
#define IPU_DI_DISP2_CB2_MAP_MC22_M6_WID        2
#define IPU_DI_DISP2_CB2_MAP_MC22_M7_WID        2
#define IPU_DI_DISP2_CB2_MAP_MC22_OFFS0_WID     5
#define IPU_DI_DISP2_CB2_MAP_MC22_OFFS1_WID     5
#define IPU_DI_DISP2_CB2_MAP_MC22_OFFS2_WID     5

// DI_DISP3_B0_MAP
#define IPU_DI_DISP3_B0_MAP_M30_M0_WID          2
#define IPU_DI_DISP3_B0_MAP_M30_M1_WID          2
#define IPU_DI_DISP3_B0_MAP_M30_M2_WID          2
#define IPU_DI_DISP3_B0_MAP_M30_M3_WID          2
#define IPU_DI_DISP3_B0_MAP_M30_M4_WID          2
#define IPU_DI_DISP3_B0_MAP_M30_M5_WID          2
#define IPU_DI_DISP3_B0_MAP_M30_M6_WID          2
#define IPU_DI_DISP3_B0_MAP_M30_M7_WID          2
#define IPU_DI_DISP3_B0_MAP_M30_OFFS0_WID       5
#define IPU_DI_DISP3_B0_MAP_M30_OFFS1_WID       5
#define IPU_DI_DISP3_B0_MAP_M30_OFFS2_WID       5

// DI_DISP3_B1_MAP
#define IPU_DI_DISP3_B1_MAP_M31_M0_WID          2
#define IPU_DI_DISP3_B1_MAP_M31_M1_WID          2
#define IPU_DI_DISP3_B1_MAP_M31_M2_WID          2
#define IPU_DI_DISP3_B1_MAP_M31_M3_WID          2
#define IPU_DI_DISP3_B1_MAP_M31_M4_WID          2
#define IPU_DI_DISP3_B1_MAP_M31_M5_WID          2
#define IPU_DI_DISP3_B1_MAP_M31_M6_WID          2
#define IPU_DI_DISP3_B1_MAP_M31_M7_WID          2
#define IPU_DI_DISP3_B1_MAP_M31_OFFS0_WID       5
#define IPU_DI_DISP3_B1_MAP_M31_OFFS1_WID       5
#define IPU_DI_DISP3_B1_MAP_M31_OFFS2_WID       5

// DI_DISP3_B2_MAP
#define IPU_DI_DISP3_B2_MAP_M32_M0_WID          2
#define IPU_DI_DISP3_B2_MAP_M32_M1_WID          2
#define IPU_DI_DISP3_B2_MAP_M32_M2_WID          2
#define IPU_DI_DISP3_B2_MAP_M32_M3_WID          2
#define IPU_DI_DISP3_B2_MAP_M32_M4_WID          2
#define IPU_DI_DISP3_B2_MAP_M32_M5_WID          2
#define IPU_DI_DISP3_B2_MAP_M32_M6_WID          2
#define IPU_DI_DISP3_B2_MAP_M32_M7_WID          2
#define IPU_DI_DISP3_B2_MAP_M32_OFFS0_WID       5
#define IPU_DI_DISP3_B2_MAP_M32_OFFS1_WID       5
#define IPU_DI_DISP3_B2_MAP_M32_OFFS2_WID       5

// DI_DISP_ACC_CC
#define IPU_DI_DISP_ACC_CC_DISP0_IF_CLK_CNT_D_WID   2
#define IPU_DI_DISP_ACC_CC_DISP0_IF_CLK_CNT_C_WID   2
#define IPU_DI_DISP_ACC_CC_DISP1_IF_CLK_CNT_D_WID   2
#define IPU_DI_DISP_ACC_CC_DISP1_IF_CLK_CNT_C_WID   2
#define IPU_DI_DISP_ACC_CC_DISP2_IF_CLK_CNT_D_WID   2
#define IPU_DI_DISP_ACC_CC_DISP2_IF_CLK_CNT_C_WID   2
#define IPU_DI_DISP_ACC_CC_DISP3_IF_CLK_CNT_D_WID   2

// DI_DISP_LLA_CONF
#define IPU_DI_DISP_LLA_CONF_DRCT_RS_WID        1
#define IPU_DI_DISP_LLA_CONF_DRCT_DISP_NUM_WID  2
#define IPU_DI_DISP_LLA_CONF_DRCT_LOCK_WID      1
#define IPU_DI_DISP_LLA_CONF_DRCT_MAP_DC_WID    1
#define IPU_DI_DISP_LLA_CONF_DRCT_BE_MODE_WID   1

// DI_DISP_LLA_DATA
#define IPU_DI_DISP_LLA_DATA_LLA_DATA_WID       24


//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------
#define IPU_ENABLE                              1
#define IPU_DISABLE                             0

#define IPU_BIG_ENDIAN                          1
#define IPU_LITTLE_ENDIAN                       0

// IPU Common Registers
#define IPU_IPU_CONF_CSI_EN_ENABLE              1
#define IPU_IPU_CONF_CSI_EN_DISABLE             0

#define IPU_IPU_CONF_IC_EN_ENABLE               1
#define IPU_IPU_CONF_IC_EN_DISABLE              0

#define IPU_IPU_CONF_ROT_EN_ENABLE              1
#define IPU_IPU_CONF_ROT_EN_DISABLE             0

#define IPU_IPU_CONF_PF_EN_ENABLE               1
#define IPU_IPU_CONF_PF_EN_DISABLE              0

#define IPU_IPU_CONF_SDC_EN_ENABLE              1
#define IPU_IPU_CONF_SDC_EN_DISABLE             0

#define IPU_IPU_CONF_ADC_EN_ENABLE              1
#define IPU_IPU_CONF_ADC_EN_DISABLE             0

#define IPU_IPU_CONF_DI_EN_ENABLE               1
#define IPU_IPU_CONF_DI_EN_DISABLE              0

#define IPU_IPU_CONF_DU_EN_ENABLE               1
#define IPU_IPU_CONF_DU_EN_DISABLE              0

#define IPU_IPU_CONF_PXL_ENDIAN_LITTLE          1
#define IPU_IPU_CONF_PXL_ENDIAN_BIG             0

// IPU_FS_PROC_FLOW
#define IPU_IPU_FS_PROC_FLOW_ENC_IN_VALID_VALID     1
#define IPU_IPU_FS_PROC_FLOW_ENC_IN_VALID_INVALID   0

#define IPU_IPU_FS_PROC_FLOW_VF_IN_VALID_VALID      1
#define IPU_IPU_FS_PROC_FLOW_VF_IN_VALID_INVALID    0

#define IPU_IPU_FS_PROC_FLOW_PRPENC_DEST_SEL_ROT    1
#define IPU_IPU_FS_PROC_FLOW_PRPENC_DEST_SEL_ARM    0

#define IPU_IPU_FS_PROC_FLOW_PRPENC_ROT_SRC_SEL_ENC 1
#define IPU_IPU_FS_PROC_FLOW_PRPENC_ROT_SRC_SEL_ARM 0

#define IPU_IPU_FS_PROC_FLOW_PRPVF_ROT_SRC_SEL_VF   1
#define IPU_IPU_FS_PROC_FLOW_PRPVF_ROT_SRC_SEL_ARM  0

#define IPU_IPU_FS_PROC_FLOW_PP_SRC_SEL_ARM         0
#define IPU_IPU_FS_PROC_FLOW_PP_SRC_SEL_PF          1
#define IPU_IPU_FS_PROC_FLOW_PP_SRC_SEL_ROT         2

#define IPU_IPU_FS_PROC_FLOW_PP_ROT_SRC_SEL_ARM     0
#define IPU_IPU_FS_PROC_FLOW_PP_ROT_SRC_SEL_PP      1
#define IPU_IPU_FS_PROC_FLOW_PP_ROT_SRC_SEL_PF      2

#define IPU_IPU_FS_PROC_FLOW_PF_DEST_SEL_ARM        0
#define IPU_IPU_FS_PROC_FLOW_PF_DEST_SEL_PP         1
#define IPU_IPU_FS_PROC_FLOW_PF_DEST_SEL_ROT        2

#define IPU_IPU_FS_PROC_FLOW_PRPVF_DEST_SEL_ARM         0
#define IPU_IPU_FS_PROC_FLOW_PRPVF_DEST_SEL_ROT         1
#define IPU_IPU_FS_PROC_FLOW_PRPVF_DEST_SEL_ADC_1       2
#define IPU_IPU_FS_PROC_FLOW_PRPVF_DEST_SEL_ADC_2       3
#define IPU_IPU_FS_PROC_FLOW_PRPVF_DEST_SEL_SDC_BG      4
#define IPU_IPU_FS_PROC_FLOW_PRPVF_DEST_SEL_SDC_FG      5
#define IPU_IPU_FS_PROC_FLOW_PRPVF_DEST_SEL_ADC_DIRECT  6

#define IPU_IPU_FS_PROC_FLOW_PRPVF_ROT_DEST_SEL_ARM     0
#define IPU_IPU_FS_PROC_FLOW_PRPVF_ROT_DEST_SEL_ADC_1   2
#define IPU_IPU_FS_PROC_FLOW_PRPVF_ROT_DEST_SEL_ADC_2   3
#define IPU_IPU_FS_PROC_FLOW_PRPVF_ROT_DEST_SEL_SDC_BG  4
#define IPU_IPU_FS_PROC_FLOW_PRPVF_ROT_DEST_SEL_SDC_FG  5

#define IPU_IPU_FS_PROC_FLOW_PP_DEST_SEL_ARM        0
#define IPU_IPU_FS_PROC_FLOW_PP_DEST_SEL_ROT        1
#define IPU_IPU_FS_PROC_FLOW_PP_DEST_SEL_ADC_1      2
#define IPU_IPU_FS_PROC_FLOW_PP_DEST_SEL_ADC_2      3
#define IPU_IPU_FS_PROC_FLOW_PP_DEST_SEL_SDC_BG     4
#define IPU_IPU_FS_PROC_FLOW_PP_DEST_SEL_SDC_FG     5
#define IPU_IPU_FS_PROC_FLOW_PP_DEST_SEL_ADC_DIRECT 6

#define IPU_IPU_FS_PROC_FLOW_PP_ROT_DEST_SEL_ARM    0
#define IPU_IPU_FS_PROC_FLOW_PP_ROT_DEST_SEL_PP     1
#define IPU_IPU_FS_PROC_FLOW_PP_ROT_DEST_SEL_ADC_1  2
#define IPU_IPU_FS_PROC_FLOW_PP_ROT_DEST_SEL_ADC_2  3
#define IPU_IPU_FS_PROC_FLOW_PP_ROT_DEST_SEL_SDC_BG 4
#define IPU_IPU_FS_PROC_FLOW_PP_ROT_DEST_SEL_SDC_FG 5


// IPU_FS_DISP_FLOW

#define IPU_IPU_FS_DISP_FLOW_SDC0_SRC_SEL_ARM         0
#define IPU_IPU_FS_DISP_FLOW_SDC0_SRC_SEL_PRPVF_ROT   1
#define IPU_IPU_FS_DISP_FLOW_SDC0_SRC_SEL_PP_ROT      2
#define IPU_IPU_FS_DISP_FLOW_SDC0_SRC_SEL_PRPVF       3
#define IPU_IPU_FS_DISP_FLOW_SDC0_SRC_SEL_PP          4


#define IPU_IPU_FS_DISP_FLOW_SDC1_SRC_SEL_ARM         0
#define IPU_IPU_FS_DISP_FLOW_SDC1_SRC_SEL_PRPVF_ROT   1
#define IPU_IPU_FS_DISP_FLOW_SDC1_SRC_SEL_PP_ROT      2
#define IPU_IPU_FS_DISP_FLOW_SDC1_SRC_SEL_PRPVF       3
#define IPU_IPU_FS_DISP_FLOW_SDC1_SRC_SEL_PP          4


#define IPU_IPU_FS_DISP_FLOW_ADC2_SRC_SEL_ARM         0
#define IPU_IPU_FS_DISP_FLOW_ADC2_SRC_SEL_PRPVF_ROT   1
#define IPU_IPU_FS_DISP_FLOW_ADC2_SRC_SEL_PP_ROT      2
#define IPU_IPU_FS_DISP_FLOW_ADC2_SRC_SEL_PRPVF       3
#define IPU_IPU_FS_DISP_FLOW_ADC2_SRC_SEL_PP          4
#define IPU_IPU_FS_DISP_FLOW_ADC2_SRC_SEL_SNOOP       5
#define IPU_IPU_FS_DISP_FLOW_ADC2_SRC_SEL_AUTO        6
#define IPU_IPU_FS_DISP_FLOW_ADC2_SRC_SEL_AUTO_SNOOP  7

#define IPU_IPU_FS_DISP_FLOW_ADC3_SRC_SEL_ARM         0
#define IPU_IPU_FS_DISP_FLOW_ADC3_SRC_SEL_PRPVF_ROT   1
#define IPU_IPU_FS_DISP_FLOW_ADC3_SRC_SEL_PP_ROT      2
#define IPU_IPU_FS_DISP_FLOW_ADC3_SRC_SEL_PRPVF       3
#define IPU_IPU_FS_DISP_FLOW_ADC3_SRC_SEL_PP          4
#define IPU_IPU_FS_DISP_FLOW_ADC3_SRC_SEL_SNOOP       5
#define IPU_IPU_FS_DISP_FLOW_ADC3_SRC_SEL_AUTO        6
#define IPU_IPU_FS_DISP_FLOW_ADC3_SRC_SEL_AUTO_SNOOP  7


// IPU_TASKS_STAT
#define IPU_IPU_TASKS_STAT_IDLE                 0
#define IPU_IPU_TASKS_STAT_ACTIVE               1
#define IPU_IPU_TASKS_STAT_WAIT4RDY             2

#define IPU_IPU_TASKS_ADC_CHAN_LOCKED           1
#define IPU_IPU_TASKS_ADC_CHAN_UNLOCKED         0

#define IPU_IMA_ADDR_MEM_NU_TPM                 0  // 0000 - Task Parameter Memory
#define IPU_IMA_ADDR_MEM_NU_CPM                 1  // 0001 - Channel Parameter Memory
#define IPU_IMA_ADDR_MEM_NU_LUTM                2  // 0010 - Look-Up Table Memory
#define IPU_IMA_ADDR_MEM_NU_TM                  3  // 0011 - Template Memory
#define IPU_IMA_ADDR_MEM_NU_ADBM                4  // 0100 - Asynchronous Display Buffer Memory
#define IPU_IMA_ADDR_MEM_NU_DOM                 5  // 0101 - Downsizing Output Memory
#define IPU_IMA_ADDR_MEM_NU_DSTM                6  // 0110 - Down Sizing Temporary Memory
#define IPU_IMA_ADDR_MEM_NU_IBM                 7  // 0111 - Input Buffer Memory
#define IPU_IMA_ADDR_MEM_NU_MPM                 8  // 1000 - Main Processing Memory
#define IPU_IMA_ADDR_MEM_NU_RM                  9  // 1001 - Rotation Memory
#define IPU_IMA_ADDR_MEM_NU_PFM                 10 // 1010 - Post Filter Memory
#define IPU_IMA_ADDR_MEM_NU_SDM                 11 // 1011 - Synchronous Display Buffer Memory

#define IPU_DUB_BUF                             1
#define IPU_SIG_BUF                             0

#define IPU_DMA_CHA_READY                       1
#define IPU_DMA_CHA_NOT_READY                   0

#define IPU_IPU_CHA_CUR_BUF_BUF1                1
#define IPU_IPU_CHA_CUR_BUF_BUF0                0
#define IPU_IPU_CHA_CUR_BUF_CLEAR               1 // write-1-to-clear

// IPU CSI Registers
#define IPU_CSI_SENS_CONF_VSYNC_POL_INVERT      1
#define IPU_CSI_SENS_CONF_VSYNC_POL_NO_INVERT   0

#define IPU_CSI_SENS_CONF_HSYNC_POL_INVERT      1
#define IPU_CSI_SENS_CONF_HSYNC_POL_NO_INVERT   0

#define IPU_CSI_SENS_CONF_DATA_POL_INVERT       1
#define IPU_CSI_SENS_CONF_DATA_POL_NO_INVERT    0

#define IPU_CSI_SENS_CONF_DISP_PIX_CLK_INVERT     1
#define IPU_CSI_SENS_CONF_DISP_PIX_CLK_NO_INVERT  0

#define IPU_CSI_SENS_CONF_SENS_PRTCL_GATED_CLOCK_MODE       0
#define IPU_CSI_SENS_CONF_SENS_PRTCL_NONGATED_CLOCK_MODE    1
#define IPU_CSI_SENS_CONF_SENS_PRTCL_CCIR_PROGRESSIVE_MODE  2
#define IPU_CSI_SENS_CONF_SENS_PRTCL_CCIR_INTERLACE_MODE    3
#define IPU_CSI_SENS_CONF_SENS_PRTCL_COMPRESSED_MODE        4

#define IPU_CSI_SENS_CONF_SENS_CLK_SRC_HSP_CLK                 1
#define IPU_CSI_SENS_CONF_SENS_CLK_SRC_IPP_IND_SENSB_SENS_CLK  0

#define IPU_CSI_SENS_CONF_SENS_DATA_FORMAT_RGB     0
#define IPU_CSI_SENS_CONF_SENS_DATA_FORMAT_YUV444  1
#define IPU_CSI_SENS_CONF_SENS_DATA_FORMAT_YUV422  2
#define IPU_CSI_SENS_CONF_SENS_DATA_FORMAT_BAYER   3

#define IPU_CSI_SENS_CONF_DATA_WIDTH_4BIT       0
#define IPU_CSI_SENS_CONF_DATA_WIDTH_8BIT       1
#define IPU_CSI_SENS_CONF_DATA_WIDTH_10BIT      2

#define IPU_CSI_SENS_CONF_EXT_VSYNC_EXTERNAL    1
#define IPU_CSI_SENS_CONF_EXT_VSYNC_INTERNAL    0


#define IPU_CSI_OUT_FRM_CTRL_IC_TV_MODE_ENABLE   1
#define IPU_CSI_OUT_FRM_CTRL_IC_TV_MODE_DISABLE  0

#define IPU_CSI_OUT_FRM_CTRL_VERT_DWNS_ENABLE    1
#define IPU_CSI_OUT_FRM_CTRL_VERT_DWNS_DISABLE   0

#define IPU_CSI_OUT_FRM_CTRL_HORZ_DWNS_ENABLE    1
#define IPU_CSI_OUT_FRM_CTRL_HORZ_DWNS_DISABLE   0


#define IPU_CSI_TST_CTRL_TEST_GEN_MODE_ACTIVE    1
#define IPU_CSI_TST_CTRL_TEST_GEN_MODE_INACTIVE  0


#define IPU_CSI_CCIR_CODE_1_CCIR_ERR_DET_EN_ENABLE     1
#define IPU_CSI_CCIR_CODE_1_CCIR_ERR_DET_EN_DISABLE    0


#define IPU_CSI_FLASH_STROBE_1_CLOCK_SENSB_PIX_CLK     1
#define IPU_CSI_FLASH_STROBE_1_CLOCK_SENSB_MCLK        0


#define IPU_CSI_FLASH_STROBE_2_STROBE_EN_ENABLE        1
#define IPU_CSI_FLASH_STROBE_2_STROBE_EN_DISABLE       0

#define IPU_CSI_FLASH_STROBE_2_STROBE_POL_ACTIVE_HIGH  1
#define IPU_CSI_FLASH_STROBE_2_STROBE_POL_ACTIVE_LOW   0


// IPU IC Registers
#define IPU_IC_CONF_PRPENC_EN_ENABLE            1
#define IPU_IC_CONF_PRPENC_EN_DISABLE           0

#define IPU_IC_CONF_PRPENC_CSC1_ENABLE          1
#define IPU_IC_CONF_PRPENC_CSC1_DISABLE         0

#define IPU_IC_CONF_PRPENC_ROT_EN_ENABLE        1
#define IPU_IC_CONF_PRPENC_ROT_EN_DISABLE       0

#define IPU_IC_CONF_PRPVF_EN_ENABLE             1
#define IPU_IC_CONF_PRPVF_EN_DISABLE            0

#define IPU_IC_CONF_PRPVF_CSC1_ENABLE           1
#define IPU_IC_CONF_PRPVF_CSC1_DISABLE          0

#define IPU_IC_CONF_PRPVF_CSC2_ENABLE           1
#define IPU_IC_CONF_PRPVF_CSC2_DISABLE          0

#define IPU_IC_CONF_PRPVF_CMB_ENABLE            1
#define IPU_IC_CONF_PRPVF_CMB_DISABLE           0

#define IPU_IC_CONF_PRPVF_ROT_EN_ENABLE         1
#define IPU_IC_CONF_PRPVF_ROT_EN_DISABLE        0

#define IPU_IC_CONF_PP_EN_ENABLE                1
#define IPU_IC_CONF_PP_EN_DISABLE               0

#define IPU_IC_CONF_PP_CSC1_ENABLE              1
#define IPU_IC_CONF_PP_CSC1_DISABLE             0

#define IPU_IC_CONF_PP_CSC2_ENABLE              1
#define IPU_IC_CONF_PP_CSC2_DISABLE             0

#define IPU_IC_CONF_PP_CMB_ENABLE               1
#define IPU_IC_CONF_PP_CMB_DISABLE              0

#define IPU_IC_CONF_PP_ROT_EN_ENABLE            1
#define IPU_IC_CONF_PP_ROT_EN_DISABLE           0

#define IPU_IC_CONF_IC_GLB_LOC_A_GLOBAL         1
#define IPU_IC_CONF_IC_GLB_LOC_A_LOCAL          0

#define IPU_IC_CONF_IC_KEY_COLOR_EN_ENABLE      1
#define IPU_IC_CONF_IC_KEY_COLOR_EN_DISABLE     0

#define IPU_IC_CONF_RWS_EN_ENABLE               1
#define IPU_IC_CONF_RWS_EN_DISABLE              0

#define IPU_IC_CONF_CSI_MEM_WR_EN_ENABLE        1
#define IPU_IC_CONF_CSI_MEM_WR_EN_DISABLE       0


#define IPU_IC_PRP_ENC_RSC_PRPENC_DS_R_H_1      0
#define IPU_IC_PRP_ENC_RSC_PRPENC_DS_R_H_2      1
#define IPU_IC_PRP_ENC_RSC_PRPENC_DS_R_H_4      2

#define IPU_IC_PRP_ENC_RSC_PRPENC_DS_R_V_1      0
#define IPU_IC_PRP_ENC_RSC_PRPENC_DS_R_V_2      1
#define IPU_IC_PRP_ENC_RSC_PRPENC_DS_R_V_4      2


#define IPU_IC_PRP_VF_RSC_PRPVF_DS_R_H_1        0
#define IPU_IC_PRP_VF_RSC_PRPVF_DS_R_H_2        1
#define IPU_IC_PRP_VF_RSC_PRPVF_DS_R_H_4        2

#define IPU_IC_PRP_VF_RSC_PRPVF_DS_R_V_1        0
#define IPU_IC_PRP_VF_RSC_PRPVF_DS_R_V_2        1
#define IPU_IC_PRP_VF_RSC_PRPVF_DS_R_V_4        2


#define IPU_IC_PP_RSC_PP_DS_R_H_1               0
#define IPU_IC_PP_RSC_PP_DS_R_H_2               1
#define IPU_IC_PP_RSC_PP_DS_R_H_4               2

#define IPU_IC_PP_RSC_PP_DS_R_V_1               0
#define IPU_IC_PP_RSC_PP_DS_R_V_2               1
#define IPU_IC_PP_RSC_PP_DS_R_V_4               2

// IPU PF REGISTERS
#define IPU_PF_CONF_DISABLED                    0
#define IPU_PF_CONF_MPEG4_DEBLOCK               1
#define IPU_PF_CONF_MPEG4_DERING                2
#define IPU_PF_CONF_MPEG4_DEBLOCK_DERING        3
#define IPU_PF_CONF_H264_DEBLOCK                4

#define IPU_PF_CONF_H264_Y_PAUSE_EN_ENABLE      1
#define IPU_PF_CONF_H264_Y_PAUSE_EN_DISABLE     0

// IPU SDC REGISTERS
#define IPU_SDC_MODE_TFT_MONO                   0
#define IPU_SDC_MODE_TFT_COLOR                  1
#define IPU_SDC_MODE_YUV_PROGRESSIVE            2
#define IPU_SDC_MODE_YUV_INTERLACED             3

#define IPU_SDC_COC_DISABLED                    0
#define IPU_SDC_COC_FULL                        1
#define IPU_SDC_COC_RESERVED                    2
#define IPU_SDC_COC_AND                         3
#define IPU_SDC_COC_OR                          5
#define IPU_SDC_COC_XOR                         6

#define IPU_SDC_COM_CONF_GWSEL_FG               1
#define IPU_SDC_COM_CONF_GWSEL_BG               0

// IPU ADC REGISTERS
#define IPU_ADC_DISPLAY_0                       0
#define IPU_ADC_DISPLAY_1                       1
#define IPU_ADC_DISPLAY_2                       2
#define IPU_ADC_DISPLAY_3                       3

#define IPU_ADC_ADDR_INC_1                      0
#define IPU_ADC_ADDR_INC_2                      1
#define IPU_ADC_ADDR_INC_4                      3

#define IPU_ADC_CONF_DATA_MAP_DATA          0
#define IPU_ADC_CONF_DATA_MAP_COMMAND   1

#define IPU_ADC_CONF_ADDR_INC_1             0
#define IPU_ADC_CONF_ADDR_INC_2             1
#define IPU_ADC_CONF_ADDR_INC_4             3

#define IPU_ADC_CONF_DISP_NUM_DISP0     0 
#define IPU_ADC_CONF_DISP_NUM_DISP1     1
#define IPU_ADC_CONF_DISP_NUM_DISP2     2

#define IPU_ADC_CONF_MODE_DISABLE                                                 0 
#define IPU_ADC_CONF_MODE_WRITE_TEMPLATE_NON_SEQUENTIAL   1 
#define IPU_ADC_CONF_MODE_READ_TEMPLATE_NON_SEQUENTIAL     2 
#define IPU_ADC_CONF_MODE_WRITE_TEMPLATE_UNCONDITIONAL     3  
#define IPU_ADC_CONF_MODE_READ_TEMPLATE_UNCONDITIONAL       4 
#define IPU_ADC_CONF_MODE_WRITE_DATA_RS0                                 5 
#define IPU_ADC_CONF_MODE_WRITE_DATA_RS1                                 6 
#define IPU_ADC_CONF_MODE_WRITE_COMMAND                                  7

#define IPU_ADC_CONF_NO_TEARING_DISABLE    0
#define IPU_ADC_CONF_NO_TEARING_ENABLE     1

#define IPU_ADC_CONF_CHAN_EN_DISABLE     0
#define IPU_ADC_CONF_CHAN_EN_ENABLE      1

#define IPU_ADC_DISP_CONF_MCU_DATA_MAP_DATA         0
#define IPU_ADC_DISP_CONF_MCU_DATA_MAP_COMMAND  1

#define IPU_ADC_DISP_CONF_MCU_DATA_WIDTH_16BITS  0
#define IPU_ADC_DISP_CONF_MCU_DATA_WIDTH_24BITS  1

#define IPU_ADC_DISP_CONF_TYPE_FULL_ADDR_NO_BYTE_EN 0
#define IPU_ADC_DISP_CONF_TYPE_FULL_ADDR_BYTE_EN        1
#define IPU_ADC_DISP_CONF_TYPE_XY_ADDR                            2

#define IPU_ADC_DISP_VSYNC_DISP12_SEL_DISP1                    0
#define IPU_ADC_DISP_VSYNC_DISP12_SEL_DISP2                    1

#define IPU_ADC_DISP_VSYNC_WIDTH_L_PIXELS                       0
#define IPU_ADC_DISP_VSYNC_WIDTH_L_LINES                        1

#define IPU_ADC_DISP_VSYNC_MODE_DISABLE                                            0
#define IPU_ADC_DISP_VSYNC_MODE_INTERNAL_VSYNC                              1
#define IPU_ADC_DISP_VSYNC_MODE_INTERNAL_VSYNC_SYNC_SENSOR     2
#define IPU_ADC_DISP_VSYNC_MODE_EXTERNAL_VSYNC                              3



// IPU DI REGISTERS
#define IPU_DI_DISP_ACC_CC_CLOCK_1_CYCLE        0
#define IPU_DI_DISP_ACC_CC_CLOCK_2_CYCLE        1
#define IPU_DI_DISP_ACC_CC_CLOCK_3_CYCLE        2
#define IPU_DI_DISP_ACC_CC_CLOCK_4_CYCLE        3

#define IPU_DI_DISP_IF_CONF_EN_DISABLE        0
#define IPU_DI_DISP_IF_CONF_EN_ENABLE         1

#define IPU_DI_DISP_IF_CONF_IF_MODE_SYSTEM80_TYPE1                   0
#define IPU_DI_DISP_IF_CONF_IF_MODE_SYSTEM80_TYPE2                   1
#define IPU_DI_DISP_IF_CONF_IF_MODE_SYSTEM68K_TYPE1                 2
#define IPU_DI_DISP_IF_CONF_IF_MODE_SYSTEM68K_TYPE2                 3
#define IPU_DI_DISP_IF_CONF_IF_MODE_SERIAL_3WIRE                       4
#define IPU_DI_DISP_IF_CONF_IF_MODE_SERIAL_4WIRE                       5
#define IPU_DI_DISP_IF_CONF_IF_MODE_SERIAL_5WIRE_RS_SERIAL    6
#define IPU_DI_DISP_IF_CONF_IF_MODE_SERIAL_5WIRE_RS_CS            7

#define IPU_DI_DISP_IF_CONF_PAR_BURST_MODE_BURST_CS        0
#define IPU_DI_DISP_IF_CONF_PAR_BURST_MODE_BURST_BLCK    1
#define IPU_DI_DISP_IF_CONF_PAR_BURST_MODE_SINGLE            2

#define IPU_DI_DISP_SIG_POL_DATA_POL_STRAIGHT                     0
#define IPU_DI_DISP_SIG_POL_DATA_POL_INVERSE                       1

#define IPU_DI_DISP_SIG_POL_CS_POL_ACTIVE_LOW                     0
#define IPU_DI_DISP_SIG_POL_CS_POL_ACTIVE_HIGH                    1

#define IPU_DI_DISP_SIG_POL_PAR_RS_POL_STRAIGHT                 0
#define IPU_DI_DISP_SIG_POL_PAR_RS_POL_INVERSE                    1

#define IPU_DI_DISP_SIG_POL_WR_POL_ACTIVE_LOW                     0
#define IPU_DI_DISP_SIG_POL_WR_POL_ACTIVE_HIGH                    1

#define IPU_DI_DISP_SIG_POL_RD_POL_ACTIVE_LOW                     0
#define IPU_DI_DISP_SIG_POL_RD_POL_ACTIVE_HIGH                    1

#define IPU_DI_DISP_SIG_POL_VSYNC_POL_ACTIVE_LOW               0
#define IPU_DI_DISP_SIG_POL_VSYNC_POL_ACTIVE_HIGH              1

#define IPU_DI_DISP_SIG_POL_SD_D_POL_STRAIGHT                     0
#define IPU_DI_DISP_SIG_POL_SD_D_POL_INVERSE                       1

#define IPU_DI_DISP_SIG_POL_SD_CLK_POL_STRAIGHT                 0
#define IPU_DI_DISP_SIG_POL_SD_CLK_POL_INVERSE                    1

#define IPU_DI_DISP_SIG_POL_SER_RS_POL_STRAIGHT                 0
#define IPU_DI_DISP_SIG_POL_SER_RS_POL_INVERSE                    1

#define IPU_DI_DISP_SIG_POL_BCLK_POL_STRAIGHT                     0
#define IPU_DI_DISP_SIG_POL_BCLK_POL_INVERSE                       1

#ifdef __cplusplus
}
#endif

#endif // __MXARM11_IPU_H