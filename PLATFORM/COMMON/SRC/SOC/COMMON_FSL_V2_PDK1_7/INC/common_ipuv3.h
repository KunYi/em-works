//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header:  common_ipuv3.h
//
//  Provides definitions for IPUv3 module.
//
//------------------------------------------------------------------------------
#ifndef __COMMON_IPUV3_H
#define __COMMON_IPUV3_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// REGISTER SUBMODULE OFFSETS FROM IPU BASE ADDR
//------------------------------------------------------------------------------
#define CSP_IPUV3_IPU_COMMON_REGS_OFFSET    0x1E000000
#define CSP_IPUV3_IDMAC_REGS_OFFSET         0x1E008000
#define CSP_IPUV3_ISP_REGS_OFFSET           0x1E010000
#define CSP_IPUV3_DP_REGS_OFFSET            0x1E018000
#define CSP_IPUV3_IC_REGS_OFFSET            0x1E020000
#define CSP_IPUV3_IRT_REGS_OFFSET           0x1E028000
#define CSP_IPUV3_CSI0_REGS_OFFSET          0x1E030000
#define CSP_IPUV3_CSI1_REGS_OFFSET          0x1E038000
#define CSP_IPUV3_DI0_REGS_OFFSET           0x1E040000
#define CSP_IPUV3_DI1_REGS_OFFSET           0x1E048000
#define CSP_IPUV3_SMFC_REGS_OFFSET          0x1E050000
#define CSP_IPUV3_DC_REGS_OFFSET            0x1E058000
#define CSP_IPUV3_DMFC_REGS_OFFSET          0x1E060000
#define CSP_IPUV3_VDI_REGS_OFFSET           0x1E068000
#define CSP_IPUV3_CPMEM_REGS_OFFSET         0x1F000000
#define CSP_IPUV3_LUT_REGS_OFFSET           0x1F020000
#define CSP_IPUV3_SRM_REGS_OFFSET           0x1F040000
#define CSP_IPUV3_TPM_REGS_OFFSET           0x1F060000
#define CSP_IPUV3_DC_TEMPLATE_REGS_OFFSET   0x1F080000
#define CSP_IPUV3_ISP_TBPR_REGS_OFFSET      0x1F0C0000

//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------
typedef struct
{
    // IPUv3 Common Registers
    UINT32 IPU_CONF;
    UINT32 IPU_SISG_CTRL0;
    UINT32 IPU_SISG_CTRL1;
    UINT32 IPU_SISG_SET[6];
    UINT32 IPU_SISG_CLR[6];
    UINT32 IPU_INT_CTRL_1;
    UINT32 IPU_INT_CTRL_2;
    UINT32 IPU_INT_CTRL_3;
    UINT32 IPU_INT_CTRL_4;
    UINT32 IPU_INT_CTRL_5;
    UINT32 IPU_INT_CTRL_6;
    UINT32 IPU_INT_CTRL_7;
    UINT32 IPU_INT_CTRL_8;
    UINT32 IPU_INT_CTRL_9;
    UINT32 IPU_INT_CTRL_10;
    UINT32 IPU_INT_CTRL_11;
    UINT32 IPU_INT_CTRL_12;
    UINT32 IPU_INT_CTRL_13;
    UINT32 IPU_INT_CTRL_14;
    UINT32 IPU_INT_CTRL_15;
    UINT32 IPU_SDMA_EVENT_1;
    UINT32 IPU_SDMA_EVENT_2;
    UINT32 IPU_SDMA_EVENT_3;
    UINT32 IPU_SDMA_EVENT_4;
    UINT32 IPU_SDMA_EVENT_7;
    UINT32 IPU_SDMA_EVENT_8;
    UINT32 IPU_SDMA_EVENT_11;
    UINT32 IPU_SDMA_EVENT_12;
    UINT32 IPU_SDMA_EVENT_13;
    UINT32 IPU_SDMA_EVENT_14;
    UINT32 IPU_SRM_PRI1;
    UINT32 IPU_SRM_PRI2;
    UINT32 IPU_FS_PROC_FLOW1;
    UINT32 IPU_FS_PROC_FLOW2;
    UINT32 IPU_FS_PROC_FLOW3;
    UINT32 IPU_FS_DISP_FLOW1;
    UINT32 IPU_FS_DISP_FLOW2;
    UINT32 IPU_SKIP;
    UINT32 IPU_DISP_ALT_CONF;
    UINT32 IPU_DISP_GEN;
    UINT32 IPU_DISP_ALT1;
    UINT32 IPU_DISP_ALT2;
    UINT32 IPU_DISP_ALT3;
    UINT32 IPU_DISP_ALT4;
    UINT32 IPU_SNOOP;
    UINT32 IPU_MEM_RST;
    UINT32 IPU_PM;
    UINT32 IPU_GPR;
    UINT32 IPU_INT_STAT_1;
    UINT32 IPU_INT_STAT_2;
    UINT32 IPU_INT_STAT_3;
    UINT32 IPU_INT_STAT_4;
    UINT32 IPU_INT_STAT_5;
    UINT32 IPU_INT_STAT_6;
    UINT32 IPU_INT_STAT_7;
    UINT32 IPU_INT_STAT_8;
    UINT32 IPU_INT_STAT_9;
    UINT32 IPU_INT_STAT_10;
    UINT32 IPU_INT_STAT_11;
    UINT32 IPU_INT_STAT_12;
    UINT32 IPU_INT_STAT_13;
    UINT32 IPU_INT_STAT_14;
    UINT32 IPU_INT_STAT_15;
    UINT32 IPU_CUR_BUF_0;
    UINT32 IPU_CUR_BUF_1;
    UINT32 IPU_ALT_CUR_BUF_0;
    UINT32 IPU_ALT_CUR_BUF_1;
    UINT32 IPU_SRM_STAT;
    UINT32 IPU_PROC_TASKS_STAT;
    UINT32 IPU_DISP_TASKS_STAT;
    UINT32 IPU_CH_BUF0_RDY0;
    UINT32 IPU_CH_BUF0_RDY1;
    UINT32 IPU_CH_BUF1_RDY0;
    UINT32 IPU_CH_BUF1_RDY1;
    UINT32 IPU_CH_DB_MODE_SEL0;
    UINT32 IPU_CH_DB_MODE_SEL1;
    UINT32 IPU_ALT_CH_BUF0_RDY0;
    UINT32 IPU_ALT_CH_BUF0_RDY1;
    UINT32 IPU_ALT_CH_BUF1_RDY0;
    UINT32 IPU_ALT_CH_BUF1_RDY1;
    UINT32 IPU_ALT_CH_DB_MODE_SEL0;
    UINT32 IPU_ALT_CH_DB_MODE_SEL1;
} CSP_IPU_COMMON_REGS, *PCSP_IPU_COMMON_REGS;


typedef struct
{
    // IPU IDMAC Registers
    UINT32 IDMAC_CONF;
    UINT32 IDMAC_CH_EN_1;
    UINT32 IDMAC_CH_EN_2;
    UINT32 IDMAC_SEP_ALPHA;
    UINT32 IDMAC_ALT_SEP_ALPHA;
    UINT32 IDMAC_CH_PRI_1;
    UINT32 IDMAC_CH_PRI_2;
    UINT32 IDMAC_WM_EN_1;
    UINT32 IDMAC_WM_EN_2;
    UINT32 IDMAC_LOCK_EN_2;
    UINT32 IDMAC_DUMMY_0;
    UINT32 IDMAC_SUB_ADDR_1;
    UINT32 IDMAC_SUB_ADDR_2;
    UINT32 IDMAC_BNDM_EN_1;
    UINT32 IDMAC_BNDM_EN_2;
    UINT32 IDMAC_SC_CORD;
    UINT32 IDMAC_CH_BUSY_1;
    UINT32 IDMAC_CH_BUSY_2;

} CSP_IPU_IDMAC_REGS, *PCSP_IPU_IDMAC_REGS;

typedef struct
{
    // IPU DP Registers
    UINT32 DP_COM_CONF_SYNC;
    UINT32 DP_GRAPH_WIND_CTRL_SYNC;
    UINT32 DP_FG_POS_SYNC;
    UINT32 DP_CUR_POS_SYNC;
    UINT32 DP_CUR_MAP_SYNC;
    UINT32 DP_GAMMA_C_SYNC[8];
    UINT32 DP_GAMMA_S_SYNC[4];
    UINT32 DP_CSCA_SYNC[4];
    UINT32 DP_CSC_SYNC_0;
    UINT32 DP_CSC_SYNC_1;
    UINT32 DP_CUR_POS_ALT;
    UINT32 DP_COM_CONF_ASYNC0;
    UINT32 DP_GRAPH_WIND_CTRL_ASYNC0;
    UINT32 DP_FG_POS_ASYNC0;
    UINT32 DP_CUR_POS_ASYNC0;
    UINT32 DP_CUR_MAP_ASYNC0;
    UINT32 DP_GAMMA_C_ASYNC0[8];
    UINT32 DP_GAMMA_S_ASYNC0[4];
    UINT32 DP_CSCA_ASYNC0[4];
    UINT32 DP_CSC_ASYNC0_0;
    UINT32 DP_CSC_ASYNC0_1;
    UINT32 DP_COM_CONF_ASYNC1;
    UINT32 DP_GRAPH_WIND_CTRL_ASYNC1;
    UINT32 DP_FG_POS_ASYNC1;
    UINT32 DP_CUR_POS_ASYNC1;
    UINT32 DP_CUR_MAP_ASYNC1;
    UINT32 DP_GAMMA_C_ASYNC1[8];
    UINT32 DP_GAMMA_S_ASYNC1[4];
    UINT32 DP_CSCA_ASYNC1[4];
    UINT32 DP_CSC_ASYNC1_0;
    UINT32 DP_CSC_ASYNC1_1;
//    UINT32 DP_DEBUG_CNT;
//    UINT32 DP_DEBUG_STAT;

} CSP_IPU_DP_REGS, *PCSP_IPU_DP_REGS;

typedef struct
{
    // IPU IC Registers
    UINT32 IC_CONF;
    UINT32 IC_PRP_ENC_RSC;
    UINT32 IC_PRP_VF_RSC;
    UINT32 IC_PP_RSC;
    UINT32 IC_CMBP_1;
    UINT32 IC_CMBP_2;
    UINT32 IC_IDMAC_1;
    UINT32 IC_IDMAC_2;
    UINT32 IC_IDMAC_3;
    UINT32 IC_IDMAC_4;

} CSP_IPU_IC_REGS, *PCSP_IPU_IC_REGS;

// CSI0 same with CSI1
typedef struct
{
    // IPU CSI Registers
    UINT32 CSI_SENS_CONF;
    UINT32 CSI_SENS_FRM_SIZE;
    UINT32 CSI_ACT_FRM_SIZE;
    UINT32 CSI_OUT_FRM_CTRL;
    UINT32 CSI_TST_CTRL;
    UINT32 CSI_CCIR_CODE_1;
    UINT32 CSI_CCIR_CODE_2;
    UINT32 CSI_CCIR_CODE_3;
    UINT32 CSI_DI;
    UINT32 CSI_SKIP;
    UINT32 CSI_CPD_CTRL;
    UINT32 CSI_CPD_RC;
    UINT32 CSI_CPD_RS;
    UINT32 CSI_CPD_GRC;
    UINT32 CSI_CPD_GRS;
    UINT32 CSI_CPD_GBC;
    UINT32 CSI_CPD_GBS;
    UINT32 CSI_CPD_BC;
    UINT32 CSI_CPD_BS;
    UINT32 CSI_CPD_OFFSET1;
    UINT32 CSI_CPD_OFFSET2;
} CSP_IPU_CSI_REGS, *PCSP_IPU_CSI_REGS;
typedef struct
{
    // IPU SMFC Registers
    UINT32 SMFC_MAP;
    UINT32 SMFC_WMC;
    UINT32 SMFC_BS;
} CSP_IPU_SMFC_REGS,*PCSP_IPU_SMFC_REGS;
typedef struct
{
    // IPU DI0 Registers
    UINT32 DI0_GENERAL;
    UINT32 DI0_BS_CLKGEN0;
    UINT32 DI0_BS_CLKGEN1;
    UINT32 DI0_SW_GEN0_1;
    UINT32 DI0_SW_GEN0_2;
    UINT32 DI0_SW_GEN0_3;
    UINT32 DI0_SW_GEN0_4;
    UINT32 DI0_SW_GEN0_5;
    UINT32 DI0_SW_GEN0_6;
    UINT32 DI0_SW_GEN0_7;
    UINT32 DI0_SW_GEN0_8;
    UINT32 DI0_SW_GEN0_9;
    UINT32 DI0_SW_GEN1_1;
    UINT32 DI0_SW_GEN1_2;
    UINT32 DI0_SW_GEN1_3;
    UINT32 DI0_SW_GEN1_4;
    UINT32 DI0_SW_GEN1_5;
    UINT32 DI0_SW_GEN1_6;
    UINT32 DI0_SW_GEN1_7;
    UINT32 DI0_SW_GEN1_8;
    UINT32 DI0_SW_GEN1_9;
    UINT32 DI0_SYNC_AS_GEN;
    UINT32 DI0_DW_GEN[12];
    UINT32 DI0_DW_SET0[12];
    UINT32 DI0_DW_SET1[12];
    UINT32 DI0_DW_SET2[12];
    UINT32 DI0_DW_SET3[12];
    UINT32 DI0_STP_REP[4];
    UINT32 DI0_STP_REP_9;
    UINT32 DI0_SER_CONF;
    UINT32 DI0_SSC;
    UINT32 DI0_POL;
    UINT32 DI0_AW0;
    UINT32 DI0_AW1;
    UINT32 DI0_SCR_CONF;
    UINT32 DI0_STAT;

} CSP_IPU_DI0_REGS, *PCSP_IPU_DI0_REGS;

typedef struct
{
    // IPU DI1 Registers
    UINT32 DI1_GENERAL;
    UINT32 DI1_BS_CLKGEN0;
    UINT32 DI1_BS_CLKGEN1;
    UINT32 DI1_SW_GEN0_1;
    UINT32 DI1_SW_GEN0_2;
    UINT32 DI1_SW_GEN0_3;
    UINT32 DI1_SW_GEN0_4;
    UINT32 DI1_SW_GEN0_5;
    UINT32 DI1_SW_GEN0_6;
    UINT32 DI1_SW_GEN0_7;
    UINT32 DI1_SW_GEN0_8;
    UINT32 DI1_SW_GEN0_9;
    UINT32 DI1_SW_GEN1_1;
    UINT32 DI1_SW_GEN1_2;
    UINT32 DI1_SW_GEN1_3;
    UINT32 DI1_SW_GEN1_4;
    UINT32 DI1_SW_GEN1_5;
    UINT32 DI1_SW_GEN1_6;
    UINT32 DI1_SW_GEN1_7;
    UINT32 DI1_SW_GEN1_8;
    UINT32 DI1_SW_GEN1_9;
    UINT32 DI1_SYNC_AS_GEN;
    UINT32 DI1_DW_GEN[12];
    UINT32 DI1_DW_SET0[12];
    UINT32 DI1_DW_SET1[12];
    UINT32 DI1_DW_SET2[12];
    UINT32 DI1_DW_SET3[12];
    UINT32 DI1_STP_REP[4];
    UINT32 DI1_STP_REP_9;
    UINT32 DI1_SER_CONF;
    UINT32 DI1_SSC;
    UINT32 DI1_POL;
    UINT32 DI1_AW0;
    UINT32 DI1_AW1;
    UINT32 DI1_SCR_CONF;
    UINT32 DI1_STAT;
} CSP_IPU_DI1_REGS, *PCSP_IPU_DI1_REGS;

typedef struct
{
    // IPU DC Registers
    UINT32 DC_READ_CH_CONF;
    UINT32 DC_READ_CH_ADDR;
    UINT32 DC_RL0_CH_0;
    UINT32 DC_RL1_CH_0;
    UINT32 DC_RL2_CH_0;
    UINT32 DC_RL3_CH_0;
    UINT32 DC_RL4_CH_0;
    UINT32 DC_WR_CH_CONF_1;
    UINT32 DC_WR_CH_ADDR_1;
    UINT32 DC_RL0_CH_1;
    UINT32 DC_RL1_CH_1;
    UINT32 DC_RL2_CH_1;
    UINT32 DC_RL3_CH_1;
    UINT32 DC_RL4_CH_1;
    UINT32 DC_WR_CH_CONF_2;
    UINT32 DC_WR_CH_ADDR_2;
    UINT32 DC_RL0_CH_2;
    UINT32 DC_RL1_CH_2;
    UINT32 DC_RL2_CH_2;
    UINT32 DC_RL3_CH_2;
    UINT32 DC_RL4_CH_2;
    UINT32 DC_CMD_CH_CONF_3;
    UINT32 DC_CMD_CH_CONF_4;
    UINT32 DC_WR_CH_CONF_5;
    UINT32 DC_WR_CH_ADDR_5;
    UINT32 DC_RL0_CH_5;
    UINT32 DC_RL1_CH_5;
    UINT32 DC_RL2_CH_5;
    UINT32 DC_RL3_CH_5;
    UINT32 DC_RL4_CH_5;
    UINT32 DC_WR_CH_CONF_6;
    UINT32 DC_WR_CH_ADDR_6;
    UINT32 DC_RL0_CH_6;
    UINT32 DC_RL1_CH_6;
    UINT32 DC_RL2_CH_6;
    UINT32 DC_RL3_CH_6;
    UINT32 DC_RL4_CH_6;
    UINT32 DC_WR_CH_CONF1_8;
    UINT32 DC_WR_CH_CONF2_8;
    UINT32 DC_RL1_CH_8;
    UINT32 DC_RL2_CH_8;
    UINT32 DC_RL3_CH_8;
    UINT32 DC_RL4_CH_8;
    UINT32 DC_RL5_CH_8;
    UINT32 DC_RL6_CH_8;
    UINT32 DC_WR_CH_CONF1_9;
    UINT32 DC_WR_CH_CONF2_9;
    UINT32 DC_RL1_CH_9;
    UINT32 DC_RL2_CH_9;
    UINT32 DC_RL3_CH_9;
    UINT32 DC_RL4_CH_9;
    UINT32 DC_RL5_CH_9;
    UINT32 DC_RL6_CH_9;
    UINT32 DC_GEN;
    UINT32 DC_DISP_CONF1_0;
    UINT32 DC_DISP_CONF1_1;
    UINT32 DC_DISP_CONF1_2;
    UINT32 DC_DISP_CONF1_3;
    UINT32 DC_DISP_CONF2_0;
    UINT32 DC_DISP_CONF2_1;
    UINT32 DC_DISP_CONF2_2;
    UINT32 DC_DISP_CONF2_3;
    UINT32 DC_DI0_CONF1;
    UINT32 DC_DI0_CONF2;
    UINT32 DC_DI1_CONF1;
    UINT32 DC_DI1_CONF2;
    UINT32 DC_MAP_CONF_0;
    UINT32 DC_MAP_CONF_1;
    UINT32 DC_MAP_CONF_2;
    UINT32 DC_MAP_CONF_3;
    UINT32 DC_MAP_CONF_4;
    UINT32 DC_MAP_CONF_5;
    UINT32 DC_MAP_CONF_6;
    UINT32 DC_MAP_CONF_7;
    UINT32 DC_MAP_CONF_8;
    UINT32 DC_MAP_CONF_9;
    UINT32 DC_MAP_CONF_10;
    UINT32 DC_MAP_CONF_11;
    UINT32 DC_MAP_CONF_12;
    UINT32 DC_MAP_CONF_13;
    UINT32 DC_MAP_CONF_14;
    UINT32 DC_MAP_CONF_15;
    UINT32 DC_MAP_CONF_16;
    UINT32 DC_MAP_CONF_17;
    UINT32 DC_MAP_CONF_18;
    UINT32 DC_MAP_CONF_19;
    UINT32 DC_MAP_CONF_20;
    UINT32 DC_MAP_CONF_21;
    UINT32 DC_MAP_CONF_22;
    UINT32 DC_MAP_CONF_23;
    UINT32 DC_MAP_CONF_24;
    UINT32 DC_MAP_CONF_25;
    UINT32 DC_MAP_CONF_26;
    UINT32 DC_UGDE0_0;
    UINT32 DC_UGDE0_1;
    UINT32 DC_UGDE0_2;
    UINT32 DC_UGDE0_3;
    UINT32 DC_UGDE1_0;
    UINT32 DC_UGDE1_1;
    UINT32 DC_UGDE1_2;
    UINT32 DC_UGDE1_3;
    UINT32 DC_UGDE2_0;
    UINT32 DC_UGDE2_1;
    UINT32 DC_UGDE2_2;
    UINT32 DC_UGDE2_3;
    UINT32 DC_UGDE3_0;
    UINT32 DC_UGDE3_1;
    UINT32 DC_UGDE3_2;
    UINT32 DC_UGDE3_3;
    UINT32 DC_LLA0;
    UINT32 DC_LLA1;
    UINT32 DC_R_LLA0;
    UINT32 DC_R_LLA1;
    UINT32 DC_WR_CH_ADDR_5_ALT;
    UINT32 DC_STAT;
} CSP_IPU_DC_REGS, *PCSP_IPU_DC_REGS;

typedef struct
{
    // IPU DMFC Registers
    UINT32 DMFC_RD_CHAN;
    UINT32 DMFC_WR_CHAN;
    UINT32 DMFC_WR_CHAN_DEF;
    UINT32 DMFC_DP_CHAN;
    UINT32 DMFC_DP_CHAN_DEF;
    UINT32 DMFC_GENERAL1;
    UINT32 DMFC_GENERAL2;
    UINT32 DMFC_IC_CTRL;
    UINT32 DMFC_STAT;
} CSP_IPU_DMFC_REGS, *PCSP_IPU_DMFC_REGS;

typedef struct
{
    // IPU VDI Registers
    UINT32 VDI_FSIZE;
    UINT32 VDI_C;
} CSP_IPU_VDI_REGS, *PCSP_IPU_VDI_REGS;

// Channel Parameter Memory Structures
typedef struct
{
    DWORD mword0_dword0;
    DWORD mword0_dword1;
    DWORD mword0_dword2;
    DWORD mword0_dword3;
    DWORD mword0_dword4;
    DWORD reserved0[3];
    DWORD mword1_dword0;
    DWORD mword1_dword1;
    DWORD mword1_dword2;
    DWORD mword1_dword3;
    DWORD mword1_dword4;
    DWORD reserved1[3];
} CPMEM_ENTRY;

typedef struct
{
    CPMEM_ENTRY CPMEMEntries[80];
} CSP_IPU_MEM_CPMEM, *PCSP_IPU_MEM_CPMEM;


// Task Parameter Memory Structures
typedef struct
{
    DWORD coeffs_word1;
    DWORD coeffs_word2;
} TPM_WORD;

typedef struct
{
    TPM_WORD tpm_word1;
    TPM_WORD tpm_word2;
    TPM_WORD tpm_word3;
} TPM_ENTRY;

typedef struct
{
    BYTE dummy1[0x2008]; 
    TPM_ENTRY enc_csc1_matrix1;  // Unused memory up to the 0x2008 address offset for enc_csc1
    BYTE dummy2[0x4028 - 0x2008 - 24]; 
    TPM_ENTRY vf_csc1_matrix1;// Unused memory up to the 0x4028 address offset for vf_csc1
    TPM_ENTRY vf_csc1_matrix2;// Unused memory up to the 0x4040 address offset for vf_csc2
    BYTE dummy4[0x6060 - 0x4028 - 24*2]; // Unused memory up to the 0x6060 address offset for pp_csc1
    TPM_ENTRY pp_csc1_matrix1;
    TPM_ENTRY pp_csc1_matrix2;
} CSP_IPU_MEM_TPM, *PCSP_IPU_MEM_TPM;


// Look-Up Table Memory Structures
typedef struct
{
    UINT32 LUTEntries[256];
} CSP_IPU_MEM_LUT, *PCSP_IPU_MEM_LUT;


// DC Template Memory Structures
typedef struct
{
    DWORD DCTemplateEntries[256][2];
} CSP_IPU_MEM_DC_TEMPLATE, *PCSP_IPU_MEM_DC_TEMPLATE;

//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------

// IPUv3 Common Registers
#define IPU_IPU_CONF_OFFSET                 0x0000
#define IPU_SISG_CTRL0_OFFSET               0x0004
#define IPU_SISG_CTRL1_OFFSET               0x0008
#define IPU_SISG_SET_OFFSET                 0x000C
#define IPU_SISG_CLR_OFFSET                 0x0024
#define IPU_IPU_INT_CTRL_1_OFFSET           0x003C
#define IPU_IPU_INT_CTRL_2_OFFSET           0x0040
#define IPU_IPU_INT_CTRL_3_OFFSET           0x0044
#define IPU_IPU_INT_CTRL_4_OFFSET           0x0048
#define IPU_IPU_INT_CTRL_5_OFFSET           0x004C
#define IPU_IPU_INT_CTRL_6_OFFSET           0x0050
#define IPU_IPU_INT_CTRL_7_OFFSET           0x0054
#define IPU_IPU_INT_CTRL_8_OFFSET           0x0058
#define IPU_IPU_INT_CTRL_9_OFFSET           0x005C
#define IPU_IPU_INT_CTRL_10_OFFSET          0x0060
#define IPU_IPU_INT_CTRL_11_OFFSET          0x0064
#define IPU_IPU_INT_CTRL_12_OFFSET          0x0068
#define IPU_IPU_INT_CTRL_13_OFFSET          0x006C
#define IPU_IPU_INT_CTRL_14_OFFSET          0x0070
#define IPU_IPU_INT_CTRL_15_OFFSET          0x0074
#define IPU_IPU_SDMA_EVENT_1_OFFSET         0x0078
#define IPU_IPU_SDMA_EVENT_2_OFFSET         0x007C
#define IPU_IPU_SDMA_EVENT_3_OFFSET         0x0080
#define IPU_IPU_SDMA_EVENT_4_OFFSET         0x0084
#define IPU_IPU_SDMA_EVENT_7_OFFSET         0x0088
#define IPU_IPU_SDMA_EVENT_8_OFFSET         0x008C
#define IPU_IPU_SDMA_EVENT_11_OFFSET        0x0090
#define IPU_IPU_SDMA_EVENT_12_OFFSET        0x0094
#define IPU_IPU_SDMA_EVENT_13_OFFSET        0x0098
#define IPU_IPU_SDMA_EVENT_14_OFFSET        0x009C
#define IPU_IPU_SRM_PRI1_OFFSET             0x00A0
#define IPU_IPU_SRM_PRI2_OFFSET             0x00A4
#define IPU_IPU_FS_PROC_FLOW1_OFFSET        0x00A8
#define IPU_IPU_FS_PROC_FLOW2_OFFSET        0x00AC
#define IPU_IPU_FS_PROC_FLOW3_OFFSET        0x00B0
#define IPU_IPU_FS_DISP_FLOW1_OFFSET        0x00B4
#define IPU_IPU_FS_DISP_FLOW2_OFFSET        0x00B8
#define IPU_IPU_SKIP_OFFSET                 0x00BC
#define IPU_IPU_DISP_ALT_CONF_OFFSET        0x00C0
#define IPU_IPU_DISP_GEN_OFFSET             0x00C4
#define IPU_IPU_DISP_ALT1_OFFSET            0x00C8
#define IPU_IPU_DISP_ALT2_OFFSET            0x00CC
#define IPU_IPU_DISP_ALT3_OFFSET            0x00D0
#define IPU_IPU_DISP_ALT4_OFFSET            0x00D4
#define IPU_IPU_SNOOP_OFFSET                0x00D8
#define IPU_IPU_MEM_RST_OFFSET              0x00DC
#define IPU_IPU_PM_OFFSET                   0x00E0
#define IPU_IPU_GPR_OFFSET                  0x00E4
#define IPU_IPU_INT_STAT_1_OFFSET           0x00E8
#define IPU_IPU_INT_STAT_2_OFFSET           0x00EC
#define IPU_IPU_INT_STAT_3_OFFSET           0x00F0
#define IPU_IPU_INT_STAT_4_OFFSET           0x00F4
#define IPU_IPU_INT_STAT_5_OFFSET           0x00F8
#define IPU_IPU_INT_STAT_6_OFFSET           0x00FC
#define IPU_IPU_INT_STAT_7_OFFSET           0x0100
#define IPU_IPU_INT_STAT_8_OFFSET           0x0104
#define IPU_IPU_INT_STAT_9_OFFSET           0x0108
#define IPU_IPU_INT_STAT_10_OFFSET          0x010C
#define IPU_IPU_INT_STAT_11_OFFSET          0x0110
#define IPU_IPU_INT_STAT_12_OFFSET          0x0114
#define IPU_IPU_INT_STAT_13_OFFSET          0x0118
#define IPU_IPU_INT_STAT_14_OFFSET          0x011C
#define IPU_IPU_INT_STAT_15_OFFSET          0x0120
#define IPU_IPU_CUR_BUF_0_OFFSET            0x0124
#define IPU_IPU_CUR_BUF_1_OFFSET            0x0128
#define IPU_IPU_ALT_CUR_BUF_0_OFFSET        0x012C
#define IPU_IPU_ALT_CUR_BUF_1_OFFSET        0x0130
#define IPU_IPU_SRM_STAT_OFFSET             0x0134
#define IPU_IPU_PROC_TASKS_STAT_OFFSET      0x0138
#define IPU_IPU_DISP_TASKS_STAT_OFFSET      0x013C
#define IPU_IPU_CH_BUF0_RDY0_OFFSET         0x0140
#define IPU_IPU_CH_BUF0_RDY1_OFFSET         0x0144
#define IPU_IPU_CH_BUF1_RDY0_OFFSET         0x0148
#define IPU_IPU_CH_BUF1_RDY1_OFFSET         0x014C
#define IPU_IPU_CH_DB_MODE_SEL0_OFFSET      0x0150
#define IPU_IPU_CH_DB_MODE_SEL1_OFFSET      0x0154
#define IPU_IPU_ALT_CH_BUF0_RDY0_OFFSET     0x0158
#define IPU_IPU_ALT_CH_BUF0_RDY1_OFFSET     0x015C
#define IPU_IPU_ALT_CH_BUF1_RDY0_OFFSET     0x0160
#define IPU_IPU_ALT_CH_BUF1_RDY1_OFFSET     0x0164
#define IPU_IPU_ALT_CH_DB_MODE_SEL0_OFFSET  0x0168
#define IPU_IPU_ALT_CH_DB_MODE_SEL1_OFFSET  0x016C

// IPU IDMAC Registers
#define IPU_IDMAC_CONF_OFFSET               0x0000
#define IPU_IDMAC_CH_EN_1_OFFSET            0x0004
#define IPU_IDMAC_CH_EN_2_OFFSET            0x0008
#define IPU_IDMAC_SEP_ALPHA_OFFSET          0x000C
#define IPU_IDMAC_ALT_SEP_ALPHA_OFFSET      0x0010
#define IPU_IDMAC_CH_PRI_1_OFFSET           0x0014
#define IPU_IDMAC_CH_PRI_2_OFFSET           0x0018
#define IPU_IDMAC_WM_EN_1_OFFSET            0x001C
#define IPU_IDMAC_WM_EN_2_OFFSET            0x0020
#define IPU_IDMAC_LOCK_EN_2_OFFSET          0x0024
#define IPU_IDMAC_SUB_ADDR_1_OFFSET         0x002C
#define IPU_IDMAC_SUB_ADDR_2_OFFSET         0x0030
#define IPU_IDMAC_BNDM_EN_1_OFFSET          0x0034
#define IPU_IDMAC_BNDM_EN_2_OFFSET          0x0038
#define IPU_IDMAC_SC_CORD_OFFSET            0x003C
#define IPU_IDMAC_CH_BUSY_1_OFFSET          0x0040
#define IPU_IDMAC_CH_BUSY_2_OFFSET          0x0044

// IPU DP Registers
#define IPU_DP_COM_CONF_SYNC_OFFSET         0x0000
#define IPU_DP_GRAPH_WIND_CTRL_SYNC_OFFSET  0x0004
#define IPU_DP_FG_POS_SYNC_OFFSET           0x0008
#define IPU_DP_CUR_POS_SYNC_OFFSET          0x000C
#define IPU_DP_CUR_MAP_SYNC_OFFSET          0x0010
#define IPU_DP_GAMMA_C_SYNC_OFFSET          0x0014
#define IPU_DP_GAMMA_S_SYNC_OFFSET          0x0034
#define IPU_DP_CSCA_SYNC_OFFSET             0x0044
#define IPU_DP_CSC_SYNC_0_OFFSET            0x0054
#define IPU_DP_CSC_SYNC_1_OFFSET            0x0058
#define IPU_DP_CUR_POS_ALT_OFFSET           0x005C
#define IPU_DP_COM_CONF_ASYNC0_OFFSET       0x0060
#define IPU_DP_GRAPH_WIND_CTRL_ASYNC0_OFFSET 0x0064
#define IPU_DP_FG_POS_ASYNC0_OFFSET         0x0068
#define IPU_DP_CUR_POS_ASYNC0_OFFSET        0x006C
#define IPU_DP_CUR_MAP_ASYNC0_OFFSET        0x0070
#define IPU_DP_GAMMA_C_ASYNC0_OFFSET        0x0074
#define IPU_DP_GAMMA_S_ASYNC0_OFFSET        0x0094
#define IPU_DP_CSCA_ASYNC0_OFFSET           0x00A4
#define IPU_DP_CSC_ASYNC0_0_OFFSET          0x00B4
#define IPU_DP_CSC_ASYNC0_1_OFFSET          0x00B8
#define IPU_DP_COM_CONF_ASYNC1_OFFSET       0x00BC
#define IPU_DP_GRAPH_WIND_CTRL_ASYNC1_OFFSET 0x00C0
#define IPU_DP_FG_POS_ASYNC1_OFFSET         0x00C4
#define IPU_DP_CUR_POS_ASYNC1_OFFSET        0x00C8
#define IPU_DP_CUR_MAP_ASYNC1_OFFSET        0x00CC
#define IPU_DP_GAMMA_C_ASYNC1_OFFSET        0x00D0
#define IPU_DP_GAMMA_S_ASYNC1_OFFSET        0x00F0
#define IPU_DP_CSCA_ASYNC1_OFFSET           0x0100
#define IPU_DP_CSC_ASYNC1_0_OFFSET          0x0110
#define IPU_DP_CSC_ASYNC1_1_OFFSET          0x0114
#define IPU_DP_DEBUG_CNT_OFFSET             0x0118
#define IPU_DP_DEBUG_STAT_OFFSET            0x011C

// IPU IC Registers
#define IPU_IC_CONF_OFFSET                  0x0000
#define IPU_IC_PRP_ENC_RSC_OFFSET           0x0004
#define IPU_IC_PRP_VF_RSC_OFFSET            0x0008
#define IPU_IC_PP_RSC_OFFSET                0x000C
#define IPU_IC_CMBP_1_OFFSET                0x0010
#define IPU_IC_CMBP_2_OFFSET                0x0014
#define IPU_IC_IDMAC_1_OFFSET               0x0018
#define IPU_IC_IDMAC_2_OFFSET               0x001C
#define IPU_IC_IDMAC_3_OFFSET               0x0020
#define IPU_IC_IDMAC_4_OFFSET               0x0024

// IPU DI0 Registers
#define IPU_DI0_GENERAL_OFFSET              0x0000
#define IPU_DI0_BS_CLKGEN0_OFFSET           0x0004
#define IPU_DI0_BS_CLKGEN1_OFFSET           0x0008
#define IPU_DI0_SW_GEN0_1_OFFSET            0x000C
#define IPU_DI0_SW_GEN0_2_OFFSET            0x0010
#define IPU_DI0_SW_GEN0_3_OFFSET            0x0014
#define IPU_DI0_SW_GEN0_4_OFFSET            0x0018
#define IPU_DI0_SW_GEN0_5_OFFSET            0x001C
#define IPU_DI0_SW_GEN0_6_OFFSET            0x0020
#define IPU_DI0_SW_GEN0_7_OFFSET            0x0024
#define IPU_DI0_SW_GEN0_8_OFFSET            0x0028
#define IPU_DI0_SW_GEN0_9_OFFSET            0x002C
#define IPU_DI0_SW_GEN1_1_OFFSET            0x0030
#define IPU_DI0_SW_GEN1_2_OFFSET            0x0034
#define IPU_DI0_SW_GEN1_3_OFFSET            0x0038
#define IPU_DI0_SW_GEN1_4_OFFSET            0x003C
#define IPU_DI0_SW_GEN1_5_OFFSET            0x0040
#define IPU_DI0_SW_GEN1_6_OFFSET            0x0044
#define IPU_DI0_SW_GEN1_7_OFFSET            0x0048
#define IPU_DI0_SW_GEN1_8_OFFSET            0x004C
#define IPU_DI0_SW_GEN1_9_OFFSET            0x0050
#define IPU_DI0_SYNC_AS_GEN_OFFSET          0x0054
#define IPU_DI0_DW_GEN_OFFSET               0x0058
#define IPU_DI0_DW_SET0_OFFSET              0x0088
#define IPU_DI0_DW_SET1_OFFSET              0x00B8
#define IPU_DI0_DW_SET2_OFFSET              0x00E8
#define IPU_DI0_DW_SET3_OFFSET              0x0118
#define IPU_DI0_STP_REP_OFFSET              0x0148
#define IPU_DI0_STP_REP_9_OFFSET            0x0158
#define IPU_DI0_SER_CONF_OFFSET             0x015C
#define IPU_DI0_SSC_OFFSET                  0x0160
#define IPU_DI0_POL_OFFSET                  0x0164
#define IPU_DI0_AW0_OFFSET                  0x0168
#define IPU_DI0_AW1_OFFSET                  0x016C
#define IPU_DI0_SCR_CONF_OFFSET             0x0170
#define IPU_DI0_STAT_OFFSET                 0x0174

// IPU DI1 Registers
#define IPU_DI1_GENERAL_OFFSET              0x0000
#define IPU_DI1_BS_CLKGEN0_OFFSET           0x0004
#define IPU_DI1_BS_CLKGEN1_OFFSET           0x0008
#define IPU_DI1_SW_GEN0_1_OFFSET            0x000C
#define IPU_DI1_SW_GEN0_2_OFFSET            0x0010
#define IPU_DI1_SW_GEN0_3_OFFSET            0x0014
#define IPU_DI1_SW_GEN0_4_OFFSET            0x0018
#define IPU_DI1_SW_GEN0_5_OFFSET            0x001C
#define IPU_DI1_SW_GEN0_6_OFFSET            0x0020
#define IPU_DI1_SW_GEN0_7_OFFSET            0x0024
#define IPU_DI1_SW_GEN0_8_OFFSET            0x0028
#define IPU_DI1_SW_GEN0_9_OFFSET            0x002C
#define IPU_DI1_SW_GEN1_1_OFFSET            0x0030
#define IPU_DI1_SW_GEN1_2_OFFSET            0x0034
#define IPU_DI1_SW_GEN1_3_OFFSET            0x0038
#define IPU_DI1_SW_GEN1_4_OFFSET            0x003C
#define IPU_DI1_SW_GEN1_5_OFFSET            0x0040
#define IPU_DI1_SW_GEN1_6_OFFSET            0x0044
#define IPU_DI1_SW_GEN1_7_OFFSET            0x0048
#define IPU_DI1_SW_GEN1_8_OFFSET            0x004C
#define IPU_DI1_SW_GEN1_9_OFFSET            0x0050
#define IPU_DI1_SYNC_AS_GEN_OFFSET          0x0054
#define IPU_DI1_DW_GEN_OFFSET               0x0058
#define IPU_DI1_DW_SET0_OFFSET              0x0088
#define IPU_DI1_DW_SET1_OFFSET              0x00B8
#define IPU_DI1_DW_SET2_OFFSET              0x00E8
#define IPU_DI1_DW_SET3_OFFSET              0x0118
#define IPU_DI1_STP_REP_OFFSET              0x0148
#define IPU_DI1_STP_REP_9_OFFSET            0x0158
#define IPU_DI1_SER_CONF_OFFSET             0x015C
#define IPU_DI1_SSC_OFFSET                  0x0160
#define IPU_DI1_POL_OFFSET                  0x0164
#define IPU_DI1_AW0_OFFSET                  0x0168
#define IPU_DI1_AW1_OFFSET                  0x016C
#define IPU_DI1_SCR_CONF_OFFSET             0x0170
#define IPU_DI1_STAT_OFFSET                 0x0174

    // IPU DC Registers
#define IPU_DC_READ_CH_CONF_OFFSET          0x0000
#define IPU_DC_READ_CH_ADDR_OFFSET          0x0004
#define IPU_DC_RL0_CH_0_OFFSET              0x0008
#define IPU_DC_RL1_CH_0_OFFSET              0x000C
#define IPU_DC_RL2_CH_0_OFFSET              0x0010
#define IPU_DC_RL3_CH_0_OFFSET              0x0014
#define IPU_DC_RL4_CH_0_OFFSET              0x0018
#define IPU_DC_WR_CH_CONF_1_OFFSET          0x001C
#define IPU_DC_WR_CH_ADDR_1_OFFSET          0x0020
#define IPU_DC_RL0_CH_1_OFFSET              0x0024
#define IPU_DC_RL1_CH_1_OFFSET              0x0028
#define IPU_DC_RL2_CH_1_OFFSET              0x002C
#define IPU_DC_RL3_CH_1_OFFSET              0x0030
#define IPU_DC_RL4_CH_1_OFFSET              0x0034
#define IPU_DC_WR_CH_CONF_2_OFFSET          0x0038
#define IPU_DC_WR_CH_ADDR_2_OFFSET          0x003C
#define IPU_DC_RL0_CH_2_OFFSET              0x0040
#define IPU_DC_RL1_CH_2_OFFSET              0x0044
#define IPU_DC_RL2_CH_2_OFFSET              0x0048
#define IPU_DC_RL3_CH_2_OFFSET              0x004C
#define IPU_DC_RL4_CH_2_OFFSET              0x0050
#define IPU_DC_CMD_CH_CONF_3_OFFSET         0x0054
#define IPU_DC_CMD_CH_CONF_4_OFFSET         0x0058
#define IPU_DC_WR_CH_CONF_5_OFFSET          0x005C
#define IPU_DC_WR_CH_ADDR_5_OFFSET          0x0060
#define IPU_DC_RL0_CH_5_OFFSET              0x0064
#define IPU_DC_RL1_CH_5_OFFSET              0x0068
#define IPU_DC_RL2_CH_5_OFFSET              0x006C
#define IPU_DC_RL3_CH_5_OFFSET              0x0070
#define IPU_DC_RL4_CH_5_OFFSET              0x0074
#define IPU_DC_WR_CH_CONF_6_OFFSET          0x0078
#define IPU_DC_WR_CH_ADDR_6_OFFSET          0x007C
#define IPU_DC_RL0_CH_6_OFFSET              0x0080
#define IPU_DC_RL1_CH_6_OFFSET              0x0084
#define IPU_DC_RL2_CH_6_OFFSET              0x0088
#define IPU_DC_RL3_CH_6_OFFSET              0x008C
#define IPU_DC_RL4_CH_6_OFFSET              0x0090
#define IPU_DC_WR_CH_CONF1_8_OFFSET         0x0094
#define IPU_DC_WR_CH_CONF2_8_OFFSET         0x0098
#define IPU_DC_RL1_CH_8_OFFSET              0x009C
#define IPU_DC_RL2_CH_8_OFFSET              0x00A0
#define IPU_DC_RL3_CH_8_OFFSET              0x00A4
#define IPU_DC_RL4_CH_8_OFFSET              0x00A8
#define IPU_DC_RL5_CH_8_OFFSET              0x00AC
#define IPU_DC_RL6_CH_8_OFFSET              0x00B0
#define IPU_DC_WR_CH_CONF1_9_OFFSET         0x00B4
#define IPU_DC_WR_CH_CONF2_9_OFFSET         0x00B8
#define IPU_DC_RL1_CH_9_OFFSET              0x00BC
#define IPU_DC_RL2_CH_9_OFFSET              0x00C0
#define IPU_DC_RL3_CH_9_OFFSET              0x00C4
#define IPU_DC_RL4_CH_9_OFFSET              0x00C8
#define IPU_DC_RL5_CH_9_OFFSET              0x00CC
#define IPU_DC_RL6_CH_9_OFFSET              0x00D0
#define IPU_DC_GEN_OFFSET                   0x00D4
#define IPU_DC_DISP_CONF1_0_OFFSET          0x00D8
#define IPU_DC_DISP_CONF1_1_OFFSET          0x00DC
#define IPU_DC_DISP_CONF1_2_OFFSET          0x00E0
#define IPU_DC_DISP_CONF1_3_OFFSET          0x00E4
#define IPU_DC_DISP_CONF2_0_OFFSET          0x00E8
#define IPU_DC_DISP_CONF2_1_OFFSET          0x00EC
#define IPU_DC_DISP_CONF2_2_OFFSET          0x00F0
#define IPU_DC_DISP_CONF2_3_OFFSET          0x00F4
#define IPU_DC_DI0_CONF1_OFFSET             0x00F8
#define IPU_DC_DI0_CONF2_OFFSET             0x00FC
#define IPU_DC_DI1_CONF1_OFFSET             0x0100
#define IPU_DC_DI1_CONF2_OFFSET             0x0104
#define IPU_DC_MAP_CONF_0_OFFSET            0x0108
#define IPU_DC_MAP_CONF_1_OFFSET            0x010C
#define IPU_DC_MAP_CONF_2_OFFSET            0x0110
#define IPU_DC_MAP_CONF_3_OFFSET            0x0114
#define IPU_DC_MAP_CONF_4_OFFSET            0x0118
#define IPU_DC_MAP_CONF_5_OFFSET            0x011C
#define IPU_DC_MAP_CONF_6_OFFSET            0x0120
#define IPU_DC_MAP_CONF_7_OFFSET            0x0124
#define IPU_DC_MAP_CONF_8_OFFSET            0x0128
#define IPU_DC_MAP_CONF_9_OFFSET            0x012C
#define IPU_DC_MAP_CONF_10_OFFSET           0x0130
#define IPU_DC_MAP_CONF_11_OFFSET           0x0134
#define IPU_DC_MAP_CONF_12_OFFSET           0x0138
#define IPU_DC_MAP_CONF_13_OFFSET           0x013C
#define IPU_DC_MAP_CONF_14_OFFSET           0x0140
#define IPU_DC_MAP_CONF_15_OFFSET           0x0144
#define IPU_DC_MAP_CONF_16_OFFSET           0x0148
#define IPU_DC_MAP_CONF_17_OFFSET           0x014C
#define IPU_DC_MAP_CONF_18_OFFSET           0x0150
#define IPU_DC_MAP_CONF_19_OFFSET           0x0154
#define IPU_DC_MAP_CONF_20_OFFSET           0x0158
#define IPU_DC_MAP_CONF_21_OFFSET           0x015C
#define IPU_DC_MAP_CONF_22_OFFSET           0x0160
#define IPU_DC_MAP_CONF_23_OFFSET           0x0164
#define IPU_DC_MAP_CONF_24_OFFSET           0x0168
#define IPU_DC_MAP_CONF_25_OFFSET           0x016C
#define IPU_DC_MAP_CONF_26_OFFSET           0x0170
#define IPU_DC_UGDE0_0_OFFSET               0x0174
#define IPU_DC_UGDE0_1_OFFSET               0x0178
#define IPU_DC_UGDE0_2_OFFSET               0x017C
#define IPU_DC_UGDE0_3_OFFSET               0x0180
#define IPU_DC_UGDE1_0_OFFSET               0x0184
#define IPU_DC_UGDE1_1_OFFSET               0x0188
#define IPU_DC_UGDE1_2_OFFSET               0x018C
#define IPU_DC_UGDE1_3_OFFSET               0x0190
#define IPU_DC_UGDE2_0_OFFSET               0x0194
#define IPU_DC_UGDE2_1_OFFSET               0x0198
#define IPU_DC_UGDE2_2_OFFSET               0x019C
#define IPU_DC_UGDE2_3_OFFSET               0x01A0
#define IPU_DC_UGDE3_0_OFFSET               0x01A4
#define IPU_DC_UGDE3_1_OFFSET               0x01A8
#define IPU_DC_UGDE3_2_OFFSET               0x01AC
#define IPU_DC_UGDE3_3_OFFSET               0x01B0
#define IPU_DC_LLA0_OFFSET                  0x01B4
#define IPU_DC_LLA1_OFFSET                  0x01B8
#define IPU_DC_R_LLA0_OFFSET                0x01BC
#define IPU_DC_R_LLA1_OFFSET                0x01C0
#define IPU_DC_WR_CH_ADDR_5_ALT_OFFSET      0x01C4
#define IPU_DC_STAT_OFFSET                  0x01C8

    // IPU DMFC Registers
#define IPU_DMFC_RD_CHAN_OFFSET             0x0000
#define IPU_DMFC_WR_CHAN_OFFSET             0x0004
#define IPU_DMFC_WR_CHAN_DEF_OFFSET         0x0008
#define IPU_DMFC_DP_CHAN_OFFSET             0x000C
#define IPU_DMFC_DP_CHAN_DEF_OFFSET         0x0010
#define IPU_DMFC_GENERAL1_OFFSET            0x0014
#define IPU_DMFC_GENERAL2_OFFSET            0x0018
#define IPU_DMFC_IC_CTRL_OFFSET             0x001C
#define IPU_DMFC_STAT_OFFSET                0x0020

// IPU VDI Registers
#define IPU_VDI_FSIZE_OFFSET                0x0000
#define IPU_VDI_C_OFFSET                    0x0004

//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------

// IPU_CONF
#define IPU_IPU_CONF_CSI0_EN_LSH                      0
#define IPU_IPU_CONF_CSI1_EN_LSH                      1
#define IPU_IPU_CONF_IC_EN_LSH                        2
#define IPU_IPU_CONF_IRT_EN_LSH                       3
#define IPU_IPU_CONF_ISP_EN_LSH                       4
#define IPU_IPU_CONF_DP_EN_LSH                        5
#define IPU_IPU_CONF_DI0_EN_LSH                       6
#define IPU_IPU_CONF_DI1_EN_LSH                       7
#define IPU_IPU_CONF_SMFC_EN_LSH                      8
#define IPU_IPU_CONF_DC_EN_LSH                        9
#define IPU_IPU_CONF_DMFC_EN_LSH                      10
#define IPU_IPU_CONF_SISG_EN_LSH                      11
#define IPU_IPU_CONF_VDI_EN_LSH                       12
#define IPU_IPU_CONF_DIAGBUS_MODE_LSH                 16
#define IPU_IPU_CONF_DIAGBUS_ON_LSH                   21
#define IPU_IPU_CONF_IDMAC_DISABLE_LSH                22
#define IPU_IPU_CONF_ISP_DOUBLE_FLOW_LSH              24
#define IPU_IPU_CONF_IC_DMFC_SEL_LSH                  25
#define IPU_IPU_CONF_IC_DMFC_SYNC_LSH                 26
#define IPU_IPU_CONF_VDI_DMFC_SYNC_LSH                27
#define IPU_IPU_CONF_CSI0_DATA_SOURCE_LSH             28
#define IPU_IPU_CONF_CSI1_DATA_SOURCE_LSH             29
#define IPU_IPU_CONF_IC_INPUT_LSH                     30
#define IPU_IPU_CONF_CSI_SEL_LSH                      31

// IPU_SISG_CTRL0
#define IPU_SISG_CTRL0_VSYNC_RESET_COUNTER_LSH        0
#define IPU_SISG_CTRL0_NO_OF_VSYNC_LSH                1
#define IPU_SISG_CTRL0_VAL_STOP_SISG_COUNTER_LSH      4
#define IPU_SISG_CTRL0_MCU_ACTV_TRIG_LSH              29
#define IPU_SISG_CTRL0_EXT_ACTV_LSH                   30

// IPU_SISG_CTRL1
#define IPU_SISG_CTRL1_SISG_STROBE_CNT_LSH            0
#define IPU_SISG_CTRL1_SISG_OUT_POL_LSH               8

// IPU DMA Channel ID used in the various IPU registers
#define IPU_DMA_CHA_0_LSH                             0
#define IPU_DMA_CHA_1_LSH                             1
#define IPU_DMA_CHA_2_LSH                             2
#define IPU_DMA_CHA_3_LSH                             3
#define IPU_DMA_CHA_4_LSH                             4
#define IPU_DMA_CHA_5_LSH                             5
#define IPU_DMA_CHA_6_LSH                             6
#define IPU_DMA_CHA_7_LSH                             7
#define IPU_DMA_CHA_8_LSH                             8
#define IPU_DMA_CHA_9_LSH                             9
#define IPU_DMA_CHA_10_LSH                            10
#define IPU_DMA_CHA_11_LSH                            11
#define IPU_DMA_CHA_12_LSH                            12
#define IPU_DMA_CHA_13_LSH                            13
#define IPU_DMA_CHA_14_LSH                            14
#define IPU_DMA_CHA_15_LSH                            15
#define IPU_DMA_CHA_16_LSH                            16
#define IPU_DMA_CHA_17_LSH                            17
#define IPU_DMA_CHA_18_LSH                            18
#define IPU_DMA_CHA_19_LSH                            19
#define IPU_DMA_CHA_20_LSH                            20
#define IPU_DMA_CHA_21_LSH                            21
#define IPU_DMA_CHA_22_LSH                            22
#define IPU_DMA_CHA_23_LSH                            23
#define IPU_DMA_CHA_24_LSH                            24
#define IPU_DMA_CHA_25_LSH                            25
#define IPU_DMA_CHA_26_LSH                            26
#define IPU_DMA_CHA_27_LSH                            27
#define IPU_DMA_CHA_28_LSH                            28
#define IPU_DMA_CHA_29_LSH                            29
#define IPU_DMA_CHA_30_LSH                            30
#define IPU_DMA_CHA_31_LSH                            31
#define IPU_DMA_CHA_32_LSH                            32
#define IPU_DMA_CHA_33_LSH                            0
#define IPU_DMA_CHA_34_LSH                            1
#define IPU_DMA_CHA_35_LSH                            2
#define IPU_DMA_CHA_36_LSH                            3
#define IPU_DMA_CHA_37_LSH                            4
#define IPU_DMA_CHA_38_LSH                            5
#define IPU_DMA_CHA_39_LSH                            6
#define IPU_DMA_CHA_40_LSH                            7
#define IPU_DMA_CHA_41_LSH                            8
#define IPU_DMA_CHA_42_LSH                            9
#define IPU_DMA_CHA_43_LSH                            10
#define IPU_DMA_CHA_44_LSH                            11
#define IPU_DMA_CHA_45_LSH                            12
#define IPU_DMA_CHA_46_LSH                            13
#define IPU_DMA_CHA_47_LSH                            14
#define IPU_DMA_CHA_48_LSH                            15
#define IPU_DMA_CHA_49_LSH                            16
#define IPU_DMA_CHA_50_LSH                            17
#define IPU_DMA_CHA_51_LSH                            18
#define IPU_DMA_CHA_52_LSH                            19

// IPU_INT_CTRL_9 and IPU_INT_STAT_9
#define IPU_IC_BAYER_BUF_OVF_LSH                  26
#define IPU_IC_ENC_BUF_OVF_LSH                    27
#define IPU_IC_VF_BUF_OVF_LSH                     28
#define IPU_ISP_PUPE_LSH                          29
#define IPU_CSI0_PUPE_LSH                         30
#define IPU_CSI1_PUPE_LSH                         31

// IPU_INT_CTRL_15 & IPU_INT_STAT_15
#define IPU_SNOOPING1_INT_LSH                     0
#define IPU_SNOOPING2_INT_LSH                     1
#define IPU_DP_SF_START_LSH                       2
#define IPU_DP_SF_END_LSH                         3
#define IPU_DP_ASF_START_LSH                      4
#define IPU_DP_ASF_END_LSH                        5
#define IPU_DP_SF_BRAKE_LSH                       6
#define IPU_DP_ASF_BRAKE_LSH                      7
#define IPU_DC_FC_0_LSH                           8
#define IPU_DC_FC_1_LSH                           9
#define IPU_DC_FC_2_LSH                           10
#define IPU_DC_FC_3_LSH                           11
#define IPU_DC_FC_4_LSH                           12
#define IPU_DC_FC_6_LSH                           13
#define IPU_DI_VSYNC_PRE_0_LSH                    14
#define IPU_DI_VSYNC_PRE_1_LSH                    15
#define IPU_DC_DP_START_LSH                       16
#define IPU_DC_ASYNC_STOP_LSH                     17
#define IPU_DI0_CNT_EN_PRE_0_LSH                  18
#define IPU_DI0_CNT_EN_PRE_1_LSH                  19
#define IPU_DI0_CNT_EN_PRE_2_LSH                  20
#define IPU_DI0_CNT_EN_PRE_3_LSH                  21
#define IPU_DI0_CNT_EN_PRE_4_LSH                  22
#define IPU_DI0_CNT_EN_PRE_5_LSH                  23
#define IPU_DI0_CNT_EN_PRE_6_LSH                  24
#define IPU_DI0_CNT_EN_PRE_7_LSH                  25
#define IPU_DI0_CNT_EN_PRE_8_LSH                  26
#define IPU_DI0_CNT_EN_PRE_9_LSH                  27
#define IPU_DI0_CNT_EN_PRE_10_LSH                 28
#define IPU_DI1_DISP_CLK_LSH                      29
#define IPU_DI1_CNT_EN_PRE_3_LSH                  30
#define IPU_DI1_CNT_EN_PRE_8_LSH                  31

// IPU_SRM_PRI1
#define IPU_IPU_SRM_PRI1_CSI1_SRM_PRI_LSH         0
#define IPU_IPU_SRM_PRI1_CSI1_SRM_MODE_LSH        3
#define IPU_IPU_SRM_PRI1_CSI0_SRM_PRI_LSH         8
#define IPU_IPU_SRM_PRI1_CSI0_SRM_MODE_LSH        11
#define IPU_IPU_SRM_PRI1_ISP_SRM_PRI_LSH          16
#define IPU_IPU_SRM_PRI1_ISP_SRM_MODE_LSH         19

// IPU_SRM_PRI2
#define IPU_IPU_SRM_PRI2_DP_SRM_PRI_LSH           0
#define IPU_IPU_SRM_PRI2_DP_S_SRM_MODE_LSH        3
#define IPU_IPU_SRM_PRI2_DP_A0_SRM_MODE_LSH       5
#define IPU_IPU_SRM_PRI2_DP_A1_SRM_MODE_LSH       7
#define IPU_IPU_SRM_PRI2_DC_SRM_PRI_LSH           9
#define IPU_IPU_SRM_PRI2_DC_2_SRM_MODE_LSH        12
#define IPU_IPU_SRM_PRI2_DC_6_SRM_MODE_LSH        14
#define IPU_IPU_SRM_PRI2_DI0_SRM_PRI_LSH          16
#define IPU_IPU_SRM_PRI2_DI0_SRM_MCU_USE_LSH      19
#define IPU_IPU_SRM_PRI2_DI1_SRM_PRI_LSH          24
#define IPU_IPU_SRM_PRI2_DI1_SRM_MODE_LSH         27

// IPU_FS_PROC_FLOW1
#define IPU_IPU_FS_PROC_FLOW1_PRPENC_ROT_SRC_SEL_LSH  0
#define IPU_IPU_FS_PROC_FLOW1_ALT_ISP_SRC_SEL_LSH     4
#define IPU_IPU_FS_PROC_FLOW1_PRPVF_ROT_SRC_SEL_LSH   8
#define IPU_IPU_FS_PROC_FLOW1_PP_SRC_SEL_LSH          12
#define IPU_IPU_FS_PROC_FLOW1_PP_ROT_SRC_SEL_LSH      16
#define IPU_IPU_FS_PROC_FLOW1_ISP_SRC_SEL_LSH         20
#define IPU_IPU_FS_PROC_FLOW1_PRP_SRC_SEL_LSH         24
#define IPU_IPU_FS_PROC_FLOW1_VDI_SRC_SEL_LSH         28
#define IPU_IPU_FS_PROC_FLOW1_ENC_IN_VALID_LSH        30
#define IPU_IPU_FS_PROC_FLOW1_VF_IN_VALID_LSH         31

// IPU_FS_PROC_FLOW2
#define IPU_IPU_FS_PROC_FLOW2_PRP_ENC_DEST_SEL_LSH    0
#define IPU_IPU_FS_PROC_FLOW2_PRPVF_DEST_SEL_LSH      4
#define IPU_IPU_FS_PROC_FLOW2_PRPVF_ROT_DEST_SEL_LSH  8
#define IPU_IPU_FS_PROC_FLOW2_PP_DEST_SEL_LSH         12
#define IPU_IPU_FS_PROC_FLOW2_PP_ROT_DEST_SEL_LSH     16
#define IPU_IPU_FS_PROC_FLOW2_PRPENC_ROT_DEST_SEL_LSH 20
#define IPU_IPU_FS_PROC_FLOW2_PRP_DEST_SEL_LSH        24
#define IPU_IPU_FS_PROC_FLOW2_PRP_ALT_DEST_SEL_LSH    26

// IPU_FS_PROC_FLOW3
#define IPU_IPU_FS_PROC_FLOW3_SMFC0_DEST_SEL_LSH      0
#define IPU_IPU_FS_PROC_FLOW3_SMFC1_DEST_SEL_LSH      4
#define IPU_IPU_FS_PROC_FLOW3_SMFC2_DEST_SEL_LSH      7
#define IPU_IPU_FS_PROC_FLOW3_SMFC3_DEST_SEL_LSH      11

// IPU_FS_DISP_FLOW1
#define IPU_IPU_FS_DISP_FLOW1_DP_SYNC0_SRC_SEL_LSH    0
#define IPU_IPU_FS_DISP_FLOW1_DP_SYNC1_SRC_SEL_LSH    4
#define IPU_IPU_FS_DISP_FLOW1_DP_ASYNC0_SRC_SEL_LSH   8
#define IPU_IPU_FS_DISP_FLOW1_DP_ASYNC1_SRC_SEL_LSH   12
#define IPU_IPU_FS_DISP_FLOW1_DC2_SRC_SEL_LSH         16
#define IPU_IPU_FS_DISP_FLOW1_DC1_SRC_SEL_LSH         20

// IPU_FS_DISP_FLOW2
#define IPU_IPU_FS_DISP_FLOW2_DP_ASYNC1_ALT_SRC_SEL_LSH 0
#define IPU_IPU_FS_DISP_FLOW2_DP_ASYNC0_ALT_SRC_SEL_LSH 4
#define IPU_IPU_FS_DISP_FLOW2_DC2_ALT_SRC_SEL_LSH       16

// IPU_SKIP
#define IPU_IPU_SKIP_CSI_MAX_RATIO_SKIP_IC_ENC_LSH    0
#define IPU_IPU_SKIP_CSI_SKIP_IC_ENC_LSH              3
#define IPU_IPU_SKIP_CSI_MAX_RATIO_SKIP_IC_VF_LSH     8
#define IPU_IPU_SKIP_CSI_SKIP_IC_VF_LSH               11
#define IPU_IPU_SKIP_VDI_MAX_RATIO_SKIP_LSH           16
#define IPU_IPU_SKIP_VDI_SKIP_LSH                     20

// IPU_DISP_GEN
#define IPU_IPU_DISP_GEN_DI0_DUAL_MODE_LSH            0
#define IPU_IPU_DISP_GEN_DI1_DUAL_MODE_LSH            1
#define IPU_IPU_DISP_GEN_DC2_DOUBLE_FLOW_LSH          2
#define IPU_IPU_DISP_GEN_DP_ASYNC_DOUBLE_FLOW_LSH     3
#define IPU_IPU_DISP_GEN_DP_FG_EN_ASYNC0_LSH          4
#define IPU_IPU_DISP_GEN_DP_FG_EN_ASYNC1_LSH          5
#define IPU_IPU_DISP_GEN_DP_PIPE_CLR_LSH              6
#define IPU_IPU_DISP_GEN_MCU_DI_ID_8_LSH              16
#define IPU_IPU_DISP_GEN_MCU_DI_ID_9_LSH              17
#define IPU_IPU_DISP_GEN_MCU_T_LSH                    18
#define IPU_IPU_DISP_GEN_MCU_MAX_BURST_STOP_LSH       22
#define IPU_IPU_DISP_GEN_DI0_COUNTER_RELEASE_LSH      24
#define IPU_IPU_DISP_GEN_DI1_COUNTER_RELEASE_LSH      25

// IPU_DISP_ALT1
#define IPU_IPU_DISP_ALT1_RUN_VALUE_M1_ALT_0_LSH      0
#define IPU_IPU_DISP_ALT1_CNT_CLR_SEL_ALT_0_LSH       12
#define IPU_IPU_DISP_ALT1_CNT_AUTO_RELOAD_ALT_0_LSH   15
#define IPU_IPU_DISP_ALT1_STEP_REPEAT_ALT_0_LSH       16
#define IPU_IPU_DISP_ALT1_SEL_ALT_0_LSH               28

// IPU_DISP_ALT2
#define IPU_IPU_DISP_ALT2_OFFSET_VALUE_ALT_0_LSH      0
#define IPU_IPU_DISP_ALT2_OFFSET_RESOLUTION_ALT_0_LSH 12
#define IPU_IPU_DISP_ALT2_RUN_RESOLUTION_ALT_0_LSH    16

// IPU_DISP_ALT3
#define IPU_IPU_DISP_ALT3_RUN_VALUE_M1_ALT_1_LSH      0
#define IPU_IPU_DISP_ALT3_CNT_CLR_SEL_ALT_1_LSH       12
#define IPU_IPU_DISP_ALT3_CNT_AUTO_RELOAD_ALT_1_LSH   15
#define IPU_IPU_DISP_ALT3_STEP_REPEAT_ALT_1_LSH       16
#define IPU_IPU_DISP_ALT3_SEL_ALT_1_LSH               28

// IPU_DISP_ALT4
#define IPU_IPU_DISP_ALT4_OFFSET_VALUE_ALT_1_LSH      0
#define IPU_IPU_DISP_ALT4_OFFSET_RESOLUTION_ALT_1_LSH 12
#define IPU_IPU_DISP_ALT4_RUN_RESOLUTION_ALT_1_LSH    16

// IPU_SNOOP
#define IPU_IPU_SNOOP_AUTOREF_PER_LSH                 0
#define IPU_IPU_SNOOP_SNOOP2_SYNC_BYP_LSH             16

// IPU_MEM_RST
#define IPU_IPU_MEM_RST_RST_MEM_EN_LSH                0
#define IPU_IPU_MEM_RST_RST_MEM_START_LSH             31

// IPU_PM
#define IPU_IPU_PM_DI0_CLK_PERIOD_0_LSH               0
#define IPU_IPU_PM_DI0_CLK_PERIOD_1_LSH               7
#define IPU_IPU_PM_DI0_SRM_CLOCK_CHANGE_MODE_LSH      14
#define IPU_IPU_PM_CLOCK_MODE_STAT_LSH                15
#define IPU_IPU_PM_DI1_CLK_PERIOD_0_LSH               16
#define IPU_IPU_PM_DI1_CLK_PERIOD_1_LSH               23
#define IPU_IPU_PM_DI1_SRM_CLOCK_CHANGE_MODE_LSH      30
#define IPU_IPU_PM_LPSR_MODE_LSH                      31

// IPU_GPR
#define IPU_IPU_GPR_IPU_DI0_CLK_CHANGE_ACK_DIS_LSH    22
#define IPU_IPU_GPR_IPU_DI1_CLK_CHANGE_ACK_DIS_LSH    23
#define IPU_IPU_GPR_IPU_ALT_CH_BUF0_RDY0_CLR_LSH      24
#define IPU_IPU_GPR_IPU_ALT_CH_BUF0_RDY1_CLR_LSH      25
#define IPU_IPU_GPR_IPU_ALT_CH_BUF1_RDY0_CLR_LSH      26
#define IPU_IPU_GPR_IPU_ALT_CH_BUF1_RDY1_CLR_LSH      27
#define IPU_IPU_GPR_IPU_CH_BUF0_RDY0_CLR_LSH          28
#define IPU_IPU_GPR_IPU_CH_BUF0_RDY1_CLR_LSH          29
#define IPU_IPU_GPR_IPU_CH_BUF1_RDY0_CLR_LSH          30
#define IPU_IPU_GPR_IPU_CH_BUF1_RDY1_CLR_LSH          31

// IPU_SRM_STAT
#define IPU_IPU_SRM_STAT_DP_S_SRM_STAT_LSH            0
#define IPU_IPU_SRM_STAT_DP_A0_SRM_STAT_LSH           1
#define IPU_IPU_SRM_STAT_DP_A1_SRM_STAT_LSH           2
#define IPU_IPU_SRM_STAT_DC_2_SRM_STAT_LSH            4
#define IPU_IPU_SRM_STAT_DC_6_SRM_STAT_LSH            5
#define IPU_IPU_SRM_STAT_DI0_SRM_STAT_LSH             8
#define IPU_IPU_SRM_STAT_DI1_SRM_STAT_LSH             9

// IPU_PROC_TASKS_STAT
#define IPU_IPU_PROC_TASKS_STAT_ENC_TSTAT_LSH         0
#define IPU_IPU_PROC_TASKS_STAT_VF_TSTAT_LSH          2
#define IPU_IPU_PROC_TASKS_STAT_PP_TSTAT_LSH          4
#define IPU_IPU_PROC_TASKS_STAT_ENC_ROT_TSTAT_LSH     6
#define IPU_IPU_PROC_TASKS_STAT_VF_ROT_TSTAT_LSH      8
#define IPU_IPU_PROC_TASKS_STAT_PP_ROT_TSTAT_LSH      10
#define IPU_IPU_PROC_TASKS_STAT_MEM2PRP_TSTAT_LSH     12
#define IPU_IPU_PROC_TASKS_STAT_CSI2MEM_SMFC0_TSTAT_LSH  16
#define IPU_IPU_PROC_TASKS_STAT_CSI2MEM_SMFC1_TSTAT_LSH  18
#define IPU_IPU_PROC_TASKS_STAT_CSI2MEM_SMFC2_TSTAT_LSH  20
#define IPU_IPU_PROC_TASKS_STAT_CSI2MEM_SMFC3_TSTAT_LSH  22

// IPU_DISP_TASKS_STAT
#define IPU_IPU_DISP_TASKS_STAT_DP_ASYNC_TSTAT_LSH    0
#define IPU_IPU_DISP_TASKS_STAT_DP_ASYNC_CUR_FLOW_LSH 3
#define IPU_IPU_DISP_TASKS_STAT_DC_ASYNC1_TSTAT_LSH   4
#define IPU_IPU_DISP_TASKS_STAT_DC_ASYNC2_TSTAT_LSH   8
#define IPU_IPU_DISP_TASKS_STAT_DC_ASYNC2_CUR_FLOW_LSH 11


// IDMAC_CONF
#define IPU_IDMAC_CONF_MAX_REQ_READ_LSH               0
#define IPU_IDMAC_CONF_WIDPT_LSH                      3
#define IPU_IDMAC_CONF_P_ENDIAN_LSH                   16

// IDMAC_SUB_ADDR_1
#define IPU_IDMAC_SUB_ADDR_1_IDMAC_SUB_ADDR_23_LSH    0
#define IPU_IDMAC_SUB_ADDR_1_IDMAC_SUB_ADDR_24_LSH    8
#define IPU_IDMAC_SUB_ADDR_1_IDMAC_SUB_ADDR_29_LSH    16
#define IPU_IDMAC_SUB_ADDR_1_IDMAC_SUB_ADDR_33_LSH    24

// IDMAC_SUB_ADDR_2
#define IPU_IDMAC_SUB_ADDR_2_IDMAC_SUB_ADDR_41_LSH    0
#define IPU_IDMAC_SUB_ADDR_2_IDMAC_SUB_ADDR_51_LSH    8
#define IPU_IDMAC_SUB_ADDR_2_IDMAC_SUB_ADDR_52_LSH    16

// IDMAC_SC_CORD
#define IPU_IDMAC_SC_CORD_SY0_LSH                     0
#define IPU_IDMAC_SC_CORD_SX0_LSH                     16

// Note: The register sets are identical for the SYNC,
// ASYNC0, and ASYNC1 display ports, so only one set of
// register LSH values is needed.

// DP_COM_CONF
#define IPU_DP_COM_CONF_DP_FG_EN_LSH             0
#define IPU_DP_COM_CONF_DP_GWSEL_LSH             1
#define IPU_DP_COM_CONF_DP_GWAM_LSH              2
#define IPU_DP_COM_CONF_DP_GWCKE_LSH             3
#define IPU_DP_COM_CONF_DP_COC_LSH               4
#define IPU_DP_COM_CONF_DP_CSC_DEF_LSH           8
#define IPU_DP_COM_CONF_DP_CSC_GAMUT_SAT_EN_LSH  10
#define IPU_DP_COM_CONF_DP_CSC_YUV_SAT_MODE_LSH  11
#define IPU_DP_COM_CONF_DP_GAMMA_EN_LSH          12
#define IPU_DP_COM_CONF_DP_GAMMA_YUV_EN_LSH      13

// DP_GRAPH_WIND_CTRL
#define IPU_DP_GRAPH_WIND_CTRL_DP_GWCKB_LSH           0
#define IPU_DP_GRAPH_WIND_CTRL_DP_GWCKG_LSH           8
#define IPU_DP_GRAPH_WIND_CTRL_DP_GWCKR_LSH           16
#define IPU_DP_GRAPH_WIND_CTRL_DP_GWAV_LSH            24

// DP_FG_POS
#define IPU_DP_FG_POS_DP_FGYP_LSH                     0
#define IPU_DP_FG_POS_DP_FGXP_LSH                     16

// DP_CUR_POS
#define IPU_DP_CUR_POS_DP_CYP_LSH                     0
#define IPU_DP_CUR_POS_DP_CYH_LSH                     11
#define IPU_DP_CUR_POS_DP_CXP_LSH                     16
#define IPU_DP_CUR_POS_DP_CXW_LSH                     27

// DP_CUR_MAP
#define IPU_DP_CUR_MAP_DP_CUR_COL_R_LSH               0
#define IPU_DP_CUR_MAP_DP_CUR_COL_G_LSH               8
#define IPU_DP_CUR_MAP_DP_CUR_COL_B_LSH               16

// DP_CSC_0
#define IPU_DP_CSC_0_DP_CSC_A8_LSH                    0
#define IPU_DP_CSC_0_DP_CSC_B0_LSH                    16
#define IPU_DP_CSC_0_DP_CSC_S0_LSH                    30

// DP_CSC_1
#define IPU_DP_CSC_1_DP_CSC_B1_LSH                    0
#define IPU_DP_CSC_1_DP_CSC_S1_LSH                    14
#define IPU_DP_CSC_1_DP_CSC_B2_LSH                    16
#define IPU_DP_CSC_1_DP_CSC_S2_LSH                    30

// DP_DEBUG_CNT
#define IPU_DP_DEBUG_CNT_BRAKE_STATUS_EN_0_LSH        0
#define IPU_DP_DEBUG_CNT_BRAKE_CNT_0_LSH              1
#define IPU_DP_DEBUG_CNT_BRAKE_STATUS_EN_1_LSH        4
#define IPU_DP_DEBUG_CNT_BRAKE_CNT_1_LSH              5

// DP_DEBUG_STAT
#define IPU_DP_DEBUG_STAT_V_CNT_OLD_0_LSH             0
#define IPU_DP_DEBUG_STAT_FG_ACTIVE_0_LSH             11
#define IPU_DP_DEBUG_STAT_COMBYP_EN_OLD_0_LSH         12
#define IPU_DP_DEBUG_STAT_CYP_EN_OLD_0_LSH            13
#define IPU_DP_DEBUG_STAT_V_CNT_OLD_1_LSH             16
#define IPU_DP_DEBUG_STAT_FG_ACTIVE_1_LSH             27
#define IPU_DP_DEBUG_STAT_COMBYP_EN_OLD_1_LSH         28
#define IPU_DP_DEBUG_STAT_CYP_EN_OLD_1_LSH            29


// IPU IC Registers
#define IPU_IC_CONF_PRPENC_EN_LSH                     0
#define IPU_IC_CONF_PRPENC_CSC1_LSH                   1
#define IPU_IC_CONF_PRPENC_ROT_EN_LSH                 2
#define IPU_IC_CONF_PRPVF_EN_LSH                      8
#define IPU_IC_CONF_PRPVF_CSC1_LSH                    9
#define IPU_IC_CONF_PRPVF_CSC2_LSH                    10
#define IPU_IC_CONF_PRPVF_CMB_LSH                     11
#define IPU_IC_CONF_PRPVF_ROT_EN_LSH                  12
#define IPU_IC_CONF_PP_EN_LSH                         16
#define IPU_IC_CONF_PP_CSC1_LSH                       17
#define IPU_IC_CONF_PP_CSC2_LSH                       18
#define IPU_IC_CONF_PP_CMB_LSH                        19
#define IPU_IC_CONF_PP_ROT_EN_LSH                     20
#define IPU_IC_CONF_IC_GLB_LOC_A_LSH                  28
#define IPU_IC_CONF_IC_KEY_COLOR_EN_LSH               29
#define IPU_IC_CONF_RWS_EN_LSH                        30
#define IPU_IC_CONF_CSI_MEM_WR_EN_LSH                 31

#define IPU_IC_PRP_ENC_RSC_PRPENC_RS_R_H_LSH          0
#define IPU_IC_PRP_ENC_RSC_PRPENC_DS_R_H_LSH          14
#define IPU_IC_PRP_ENC_RSC_PRPENC_RS_R_V_LSH          16
#define IPU_IC_PRP_ENC_RSC_PRPENC_DS_R_V_LSH          30

#define IPU_IC_PRP_VF_RSC_PRPVF_RS_R_H_LSH            0
#define IPU_IC_PRP_VF_RSC_PRPVF_DS_R_H_LSH            14
#define IPU_IC_PRP_VF_RSC_PRPVF_RS_R_V_LSH            16
#define IPU_IC_PRP_VF_RSC_PRPVF_DS_R_V_LSH            30

#define IPU_IC_PP_RSC_PP_RS_R_H_LSH                   0
#define IPU_IC_PP_RSC_PP_DS_R_H_LSH                   14
#define IPU_IC_PP_RSC_PP_RS_R_V_LSH                   16
#define IPU_IC_PP_RSC_PP_DS_R_V_LSH                   30

#define IPU_IC_CMBP_1_IC_PRPVF_ALPHA_V_LSH            0
#define IPU_IC_CMBP_1_IC_PP_ALPHA_V_LSH               8

#define IPU_IC_CMBP_2_IC_KEY_COLOR_B_LSH              0
#define IPU_IC_CMBP_2_IC_KEY_COLOR_G_LSH              8
#define IPU_IC_CMBP_2_IC_KEY_COLOR_R_LSH              16

#define IPU_IC_IDMAC_1_CB0_BURST_16_LSH               0
#define IPU_IC_IDMAC_1_CB1_BURST_16_LSH               1
#define IPU_IC_IDMAC_1_CB2_BURST_16_LSH               2
#define IPU_IC_IDMAC_1_CB3_BURST_16_LSH               3
#define IPU_IC_IDMAC_1_CB4_BURST_16_LSH               4
#define IPU_IC_IDMAC_1_CB5_BURST_16_LSH               5
#define IPU_IC_IDMAC_1_CB6_BURST_16_LSH               6
#define IPU_IC_IDMAC_1_CB7_BURST_16_LSH               7
#define IPU_IC_IDMAC_1_T1_ROT_LSH                     11
#define IPU_IC_IDMAC_1_T1_FLIP_LR_LSH                 12
#define IPU_IC_IDMAC_1_T1_FLIP_UD_LSH                 13
#define IPU_IC_IDMAC_1_T2_ROT_LSH                     14
#define IPU_IC_IDMAC_1_T2_FLIP_LR_LSH                 15
#define IPU_IC_IDMAC_1_T2_FLIP_UD_LSH                 16
#define IPU_IC_IDMAC_1_T3_ROT_LSH                     17
#define IPU_IC_IDMAC_1_T3_FLIP_LR_LSH                 18
#define IPU_IC_IDMAC_1_T3_FLIP_UD_LSH                 19
#define IPU_IC_IDMAC_1_ALT_CB6_BURST_16_LSH           24
#define IPU_IC_IDMAC_1_ALT_CB7_BURST_16_LSH           25

#define IPU_IC_IDMAC_2_T1_FR_HEIGHT_LSH               0
#define IPU_IC_IDMAC_2_T2_FR_HEIGHT_LSH               10
#define IPU_IC_IDMAC_2_T3_FR_HEIGHT_LSH               20

#define IPU_IC_IDMAC_3_T1_FR_WIDTH_LSH                0
#define IPU_IC_IDMAC_3_T2_FR_WIDTH_LSH                10
#define IPU_IC_IDMAC_3_T3_FR_WIDTH_LSH                20

#define IPU_IC_IDMAC_4_MPM_RW_BRDG_MAX_RQ_LSH         0
#define IPU_IC_IDMAC_4_MPM_DMFC_BRDG_MAX_RQ_LSH       4
#define IPU_IC_IDMAC_4_IBM_BRDG_MAX_RQ_LSH            8
#define IPU_IC_IDMAC_4_RM_BRDG_MAX_RQ_LSH             12

// IPU DI0/DI1 Registers
#define IPU_DI_GENERAL_DI_POLARITY_1_LSH              0
#define IPU_DI_GENERAL_DI_POLARITY_2_LSH              1
#define IPU_DI_GENERAL_DI_POLARITY_3_LSH              2
#define IPU_DI_GENERAL_DI_POLARITY_4_LSH              3
#define IPU_DI_GENERAL_DI_POLARITY_5_LSH              4
#define IPU_DI_GENERAL_DI_POLARITY_6_LSH              5
#define IPU_DI_GENERAL_DI_POLARITY_7_LSH              6
#define IPU_DI_GENERAL_DI_POLARITY_8_LSH              7
#define IPU_DI_GENERAL_DI_POLARITY_CS0_LSH            8
#define IPU_DI_GENERAL_DI_POLARITY_CS1_LSH            9
#define IPU_DI_GENERAL_DI_ERM_VSYNC_SEL_LSH           10
#define IPU_DI_GENERAL_DI_ERR_TREATMENT_LSH           11
#define IPU_DI_GENERAL_DI_SYNC_COUNT_SEL_LSH          12
#define IPU_DI_GENERAL_DI_POLARITY_DISP_CLK_LSH       17
#define IPU_DI_GENERAL_DI_WATCHDOG_MODE_LSH           18
#define IPU_DI_GENERAL_DI_CLK_EXT_LSH                 20
#define IPU_DI_GENERAL_DI_VSYNC_EXT_LSH               21
#define IPU_DI_GENERAL_DI_MASK_SEL_LSH                22
#define IPU_DI_GENERAL_DI_DISP_CLOCK_INIT_LSH         23
#define IPU_DI_GENERAL_DI_CLOCK_STOP_MODE_LSH         24
#define IPU_DI_GENERAL_DI_DISP_Y_SEL_LSH              28

#define IPU_DI_BS_CLKGEN0_DI_DISP_CLK_PERIOD_LSH      0
#define IPU_DI_BS_CLKGEN0_DI_DISP_CLK_OFFSET_LSH      16

#define IPU_DI_BS_CLKGEN1_DI_DISP_CLK_UP_LSH          0
#define IPU_DI_BS_CLKGEN1_DI_DISP_CLK_DOWN_LSH        16

#define IPU_DI_SW_GEN0_DI_OFFSET_RESOLUTION_LSH       0
#define IPU_DI_SW_GEN0_DI_OFFSET_VALUE_LSH            3
#define IPU_DI_SW_GEN0_DI_RUN_RESOLUTION_LSH          16
#define IPU_DI_SW_GEN0_DI_RUN_VALUE_M1_LSH            19

#define IPU_DI_SW_GEN1_DI_CNT_UP_LSH                  0
#define IPU_DI_SW_GEN1_DI_CNT_POLARITY_CLR_SEL_LSH    9
#define IPU_DI_SW_GEN1_DI_CNT_POLARITY_TRIGGER_SEL_LSH 12
#define IPU_DI_SW_GEN1_DI_CNT_DOWN_LSH                16
#define IPU_DI_SW_GEN1_DI_CNT_CLR_SEL_LSH             25
#define IPU_DI_SW_GEN1_DI_CNT_AUTO_RELOAD_LSH         28
#define IPU_DI_SW_GEN1_DI_CNT_POLARITY_GEN_EN_LSH     29

// Unique values to DI0_SW_GEN1_9
#define IPU_DI_SW_GEN1_DI_TAG_SEL_LSH                 15
#define IPU_DI_SW_GEN1_DI_GENTIME_SEL_LSH             29

#define IPU_DI_SYNC_AS_GEN_DI_SYNC_START_LSH          0
#define IPU_DI_SYNC_AS_GEN_DI_VSYNC_SEL_LSH           13
#define IPU_DI_SYNC_AS_GEN_DI_SYNC_START_EN_LSH       28

// For parallel interface
#define IPU_DI_DW_GEN_DI_PT_0_LSH                     0
#define IPU_DI_DW_GEN_DI_PT_1_LSH                     2
#define IPU_DI_DW_GEN_DI_PT_2_LSH                     4
#define IPU_DI_DW_GEN_DI_PT_3_LSH                     6
#define IPU_DI_DW_GEN_DI_PT_4_LSH                     8
#define IPU_DI_DW_GEN_DI_PT_5_LSH                     10
#define IPU_DI_DW_GEN_DI_PT_6_LSH                     12
#define IPU_DI_DW_GEN_DI_CST_LSH                      14
#define IPU_DI_DW_GEN_DI_COMPONENT_SIZE_LSH           16
#define IPU_DI_DW_GEN_DI_ACCESS_SIZE_LSH              24

// For serial interface
#define IPU_DI_DW_GEN_DI_SERIAL_CLK_LSH               0
#define IPU_DI_DW_GEN_DI_SERIAL_RS_LSH                2
#define IPU_DI_DW_GEN_DI_SERIAL_VALID_BITS_LSH        4
// Serial CST same as for parallel, so we don't redefine
#define IPU_DI_DW_GEN_DI_START_PERIOD_LSH             16
#define IPU_DI_DW_GEN_DI_SERIAL_PERIOD_LSH            24

#define IPU_DI_DW_GEN_DI_DATA_CNT_UP_LSH              0
#define IPU_DI_DW_GEN_DI_DATA_CNT_DOWN_LSH            16

#define IPU_DI_DW_SET_DI_DATA_CNT_UP_LSH              0
#define IPU_DI_DW_SET_DI_DATA_CNT_DOWN_LSH            16

#define IPU_DI_STP_REP_STEP_REPEAT_L_LSH              0
#define IPU_DI_STP_REP_STEP_REPEAT_H_LSH              16

#define IPU_DI_SER_CONF_WAIT4SERIAL_LSH               0
#define IPU_DI_SER_CONF_SERIAL_CS_POLARITY_LSH        1
#define IPU_DI_SER_CONF_SERIAL_RS_POLARITY_LSH        2
#define IPU_DI_SER_CONF_SERIAL_DATA_POLARITY_LSH      3
#define IPU_DI_SER_CONF_SER_CLK_POLARITY_LSH          4
#define IPU_DI_SER_CONF_LLA_SER_ACCESS_LSH            5
#define IPU_DI_SER_CONF_SERIAL_LATCH_LSH              8
#define IPU_DI_SER_CONF_SERIAL_LLA_PNTR_RS_W_0_LSH    16
#define IPU_DI_SER_CONF_SERIAL_LLA_PNTR_RS_W_1_LSH    20
#define IPU_DI_SER_CONF_SERIAL_LLA_PNTR_RS_R_0_LSH    24
#define IPU_DI_SER_CONF_SERIAL_LLA_PNTR_RS_R_1_LSH    28

#define IPU_DI_SSC_BYTE_EN_PNTR_LSH                   0
#define IPU_DI_SSC_BYTE_EN_READ_IN_LSH                3
#define IPU_DI_SSC_WAIT_ON_LSH                        5
#define IPU_DI_SSC_CS_ERM_LSH                         16
#define IPU_DI_SSC_PIN11_ERM_LSH                      17
#define IPU_DI_SSC_PIN12_ERM_LSH                      18
#define IPU_DI_SSC_PIN13_ERM_LSH                      19
#define IPU_DI_SSC_PIN14_ERM_LSH                      20
#define IPU_DI_SSC_PIN15_ERM_LSH                      21
#define IPU_DI_SSC_PIN16_ERM_LSH                      22
#define IPU_DI_SSC_PIN17_ERM_LSH                      23

#define IPU_DI_POL_DRDY_POLARITY_11_LSH               0
#define IPU_DI_POL_DRDY_POLARITY_12_LSH               1
#define IPU_DI_POL_DRDY_POLARITY_13_LSH               2
#define IPU_DI_POL_DRDY_POLARITY_14_LSH               3
#define IPU_DI_POL_DRDY_POLARITY_15_LSH               4
#define IPU_DI_POL_DRDY_POLARITY_16_LSH               5
#define IPU_DI_POL_DRDY_POLARITY_17_LSH               6
#define IPU_DI_POL_DRDY_DATA_POLARITY_LSH             7
#define IPU_DI_POL_CS0_POLARITY_11_LSH                8
#define IPU_DI_POL_CS0_POLARITY_12_LSH                9
#define IPU_DI_POL_CS0_POLARITY_13_LSH                10
#define IPU_DI_POL_CS0_POLARITY_14_LSH                11
#define IPU_DI_POL_CS0_POLARITY_15_LSH                12
#define IPU_DI_POL_CS0_POLARITY_16_LSH                13
#define IPU_DI_POL_CS0_POLARITY_17_LSH                14
#define IPU_DI_POL_CS0_DATA_POLARITY_LSH              15
#define IPU_DI_POL_CS1_POLARITY_11_LSH                16
#define IPU_DI_POL_CS1_POLARITY_12_LSH                17
#define IPU_DI_POL_CS1_POLARITY_13_LSH                18
#define IPU_DI_POL_CS1_POLARITY_14_LSH                19
#define IPU_DI_POL_CS1_POLARITY_15_LSH                20
#define IPU_DI_POL_CS1_POLARITY_16_LSH                21
#define IPU_DI_POL_CS1_POLARITY_17_LSH                22
#define IPU_DI_POL_CS1_DATA_POLARITY_LSH              23
#define IPU_DI_POL_CS0_BYTE_EN_POLARITY_LSH           24
#define IPU_DI_POL_CS1_BYTE_EN_POLARITY_LSH           25
#define IPU_DI_POL_WAIT_POLARITY_LSH                  26

#define IPU_DI_AW0_AW_HSTART_LSH                      0
#define IPU_DI_AW0_AW_HCOUNT_SEL_LSH                  12
#define IPU_DI_AW0_AW_HEND_LSH                        16
#define IPU_DI_AW0_AW_TRIG_SEL_LSH                    28

#define IPU_DI_AW1_AW_VSTART_LSH                      0
#define IPU_DI_AW1_AW_VCOUNT_SEL_LSH                  12
#define IPU_DI_AW1_AW_VEND_LSH                        16

#define IPU_DI_SCR_CONF_SCREEN_HEIGHT_LSH             0

#define IPU_DI_STAT_READ_FIFO_EMPTY_LSH               0
#define IPU_DI_STAT_READ_FIFO_FULL_LSH                1
#define IPU_DI_STAT_CNTR_FIFO_EMPTY_LSH               2
#define IPU_DI_STAT_CNTR_FIFO_FULL_LSH                3

// IPU DC Registers
#define IPU_DC_READ_CH_CONF_RD_CHANNEL_EN_LSH         0
#define IPU_DC_READ_CH_CONF_PROG_DI_ID_0_LSH          1
#define IPU_DC_READ_CH_CONF_PROG_DISP_ID_0_LSH        2
#define IPU_DC_READ_CH_CONF_W_SIZE_0_LSH              4
#define IPU_DC_READ_CH_CONF_CHAN_MASK_DEFAULT_0_LSH   6
#define IPU_DC_READ_CH_CONF_CS_ID_0_LSH               8
#define IPU_DC_READ_CH_CONF_CS_ID_1_LSH               9
#define IPU_DC_READ_CH_CONF_CS_ID_2_LSH               10
#define IPU_DC_READ_CH_CONF_CS_ID_3_LSH               11
#define IPU_DC_READ_CH_CONF_TIME_OUT_VALUE_LSH        16

#define IPU_DC_RL0_CH_COD_NF_PRIORITY_CHAN_LSH        0
#define IPU_DC_RL0_CH_COD_NF_START_CHAN_LSH           8
#define IPU_DC_RL0_CH_COD_NL_PRIORITY_CHAN_LSH        16
#define IPU_DC_RL0_CH_COD_NL_START_CHAN_LSH           24

#define IPU_DC_RL1_CH_COD_EOF_PRIORITY_CHAN_LSH       0
#define IPU_DC_RL1_CH_COD_EOF_START_CHAN_LSH          8
#define IPU_DC_RL1_CH_COD_NFIELD_PRIORITY_CHAN_LSH    16
#define IPU_DC_RL1_CH_COD_NFIELD_START_CHAN_LSH       24

#define IPU_DC_RL2_CH_COD_EOL_PRIORITY_CHAN_LSH       0
#define IPU_DC_RL2_CH_COD_EOL_START_CHAN_LSH          8
#define IPU_DC_RL2_CH_COD_EOFIELD_PRIORITY_CHAN_LSH   16
#define IPU_DC_RL2_CH_COD_EOFIELD_START_CHAN_LSH      24

#define IPU_DC_RL3_CH_COD_NEW_ADDR_PRIORITY_CHAN_LSH  0
#define IPU_DC_RL3_CH_COD_NEW_ADDR_START_CHAN_LSH     8
#define IPU_DC_RL3_CH_COD_NEW_CHAN_PRIORITY_CHAN_LSH  16
#define IPU_DC_RL3_CH_COD_NEW_CHAN_START_CHAN_LSH     24

#define IPU_DC_RL4_CH_COD_NEW_DATA_PRIORITY_CHAN_LSH   0
#define IPU_DC_RL4_CH_COD_NEW_DATA_START_CHAN_LSH      8

#define IPU_DC_RL1_CH_COD_NEW_ADDR_PRIORITY_CHAN_8_9_LSH  0
#define IPU_DC_RL1_CH_COD_NEW_ADDR_START_CHAN_W_0_LSH     8
#define IPU_DC_RL1_CH_COD_NEW_ADDR_START_CHAN_W_1_LSH     24

#define IPU_DC_RL2_CH_COD_NEW_CHAN_PRIORITY_CHAN_8_9_LSH  0
#define IPU_DC_RL2_CH_COD_NEW_CHAN_START_CHAN_W_0_LSH     8
#define IPU_DC_RL2_CH_COD_NEW_CHAN_START_CHAN_W_1_LSH     24

#define IPU_DC_RL3_CH_COD_NEW_DATA_PRIORITY_CHAN_8_9_LSH  0
#define IPU_DC_RL3_CH_COD_NEW_DATA_START_CHAN_W_0_LSH     8
#define IPU_DC_RL3_CH_COD_NEW_DATA_START_CHAN_W_1_LSH     24

#define IPU_DC_RL4_CH_COD_NEW_ADDR_START_CHAN_R_0_LSH  8
#define IPU_DC_RL4_CH_COD_NEW_ADDR_START_CHAN_R_1_LSH  24

#define IPU_DC_RL5_CH_COD_NEW_CHAN_START_CHAN_R_0_LSH  8
#define IPU_DC_RL5_CH_COD_NEW_CHAN_START_CHAN_R_1_LSH  24

#define IPU_DC_RL6_CH_COD_NEW_DATA_START_CHAN_R_0_LSH  8
#define IPU_DC_RL6_CH_COD_NEW_DATA_START_CHAN_R_1_LSH  24

#define IPU_DC_WR_CH_CONF_1_W_SIZE_1_LSH              0
#define IPU_DC_WR_CH_CONF_1_PROG_DI_ID_1_LSH          2
#define IPU_DC_WR_CH_CONF_1_PROG_DISP_ID_1_LSH        3
#define IPU_DC_WR_CH_CONF_1_PROG_CHAN_TYP_1_LSH       5
#define IPU_DC_WR_CH_CONF_1_CHAN_MASK_DEFAULT_1_LSH   8
#define IPU_DC_WR_CH_CONF_1_FIELD_MODE_1_LSH          9
#define IPU_DC_WR_CH_CONF_1_PROG_START_TIME_1_LSH     16

#define IPU_DC_WR_CH_CONF_2_W_SIZE_2_LSH              0
#define IPU_DC_WR_CH_CONF_2_PROG_DI_ID_2_LSH          2
#define IPU_DC_WR_CH_CONF_2_PROG_DISP_ID_2_LSH        3
#define IPU_DC_WR_CH_CONF_2_PROG_CHAN_TYP_2_LSH       5
#define IPU_DC_WR_CH_CONF_2_CHAN_MASK_DEFAULT_2_LSH   8
#define IPU_DC_WR_CH_CONF_2_PROG_START_TIME_2_LSH     16

#define IPU_DC_CMD_CH_CONF_3_W_SIZE_3_LSH             0
#define IPU_DC_CMD_CH_CONF_3_COD_CMND_START_CHAN_RS0_3_LSH  8
#define IPU_DC_CMD_CH_CONF_3_COD_CMND_START_CHAN_RS1_3_LSH  24

#define IPU_DC_CMD_CH_CONF_4_W_SIZE_4_LSH             0
#define IPU_DC_CMD_CH_CONF_4_COD_CMND_START_CHAN_RS0_4_LSH  8
#define IPU_DC_CMD_CH_CONF_4_COD_CMND_START_CHAN_RS1_4_LSH  24

#define IPU_DC_WR_CH_CONF_5_W_SIZE_5_LSH              0
#define IPU_DC_WR_CH_CONF_5_PROG_DI_ID_5_LSH          2
#define IPU_DC_WR_CH_CONF_5_PROG_DISP_ID_5_LSH        3
#define IPU_DC_WR_CH_CONF_5_PROG_CHAN_TYP_5_LSH       5
#define IPU_DC_WR_CH_CONF_5_CHAN_MASK_DEFAULT_5_LSH   8
#define IPU_DC_WR_CH_CONF_5_FIELD_MODE_5_LSH          9
#define IPU_DC_WR_CH_CONF_5_PROG_START_TIME_5_LSH     16

#define IPU_DC_WR_CH_CONF_6_W_SIZE_6_LSH              0
#define IPU_DC_WR_CH_CONF_6_PROG_DI_ID_6_LSH          2
#define IPU_DC_WR_CH_CONF_6_PROG_DISP_ID_6_LSH        3
#define IPU_DC_WR_CH_CONF_6_PROG_CHAN_TYP_6_LSH       5
#define IPU_DC_WR_CH_CONF_6_CHAN_MASK_DEFAULT_6_LSH   8
#define IPU_DC_WR_CH_CONF_6_PROG_START_TIME_6_LSH     16

#define IPU_DC_WR_CH_CONF1_8_W_SIZE_8_LSH             0
#define IPU_DC_WR_CH_CONF1_8_CHAN_MASK_DEFAULT_8_LSH  2
#define IPU_DC_WR_CH_CONF1_8_MCU_DISP_ID_8_LSH        3

#define IPU_DC_WR_CH_CONF1_9_W_SIZE_9_LSH             0
#define IPU_DC_WR_CH_CONF1_9_CHAN_MASK_DEFAULT_9_LSH  2
#define IPU_DC_WR_CH_CONF1_9_MCU_DISP_ID_9_LSH        3

#define IPU_DC_GEN_SYNC_1_6_LSH                       1
#define IPU_DC_GEN_MASK_EN_LSH                        4
#define IPU_DC_GEN_MASK4CHAN_5_LSH                    5
#define IPU_DC_GEN_SYNC_PRIORITY_5_LSH                6
#define IPU_DC_GEN_SYNC_PRIORITY_1_LSH                7
#define IPU_DC_GEN_DC_CH5_TYPE_LSH                    8
#define IPU_DC_GEN_DC_BKDIV_LSH                       16
#define IPU_DC_GEN_DC_BK_EN_LSH                       24

#define IPU_DC_DISP_CONF1_DISP_TYP_LSH                0
#define IPU_DC_DISP_CONF1_ADDR_INCREMENT_LSH          2
#define IPU_DC_DISP_CONF1_ADDR_BE_L_INC_LSH           4
#define IPU_DC_DISP_CONF1_MCU_ACC_LB_MASK_LSH         6
#define IPU_DC_DISP_CONF1_DISP_RD_VALUE_PTR_LSH       7

#define IPU_DC_MAP_CONF_MAPPING_PNTR_BYTE0_0_LSH      0
#define IPU_DC_MAP_CONF_MAPPING_PNTR_BYTE1_0_LSH      5
#define IPU_DC_MAP_CONF_MAPPING_PNTR_BYTE2_0_LSH      10
#define IPU_DC_MAP_CONF_MAPPING_PNTR_BYTE0_1_LSH      16
#define IPU_DC_MAP_CONF_MAPPING_PNTR_BYTE1_1_LSH      21
#define IPU_DC_MAP_CONF_MAPPING_PNTR_BYTE2_1_LSH      26

#define IPU_DC_MAP_CONF_MD_MASK_0_LSH                 0
#define IPU_DC_MAP_CONF_MD_OFFSET_0_LSH               8
#define IPU_DC_MAP_CONF_MD_MASK_1_LSH                 16
#define IPU_DC_MAP_CONF_MD_OFFSET_1_LSH               24

#define IPU_DC_UGDE_0_ID_CODED_LSH                    0
#define IPU_DC_UGDE_0_COD_EV_PRIORITY_LSH             3
#define IPU_DC_UGDE_0_COD_EV_START_LSH                8
#define IPU_DC_UGDE_0_COD_ODD_START_LSH               16
#define IPU_DC_UGDE_0_ODD_EN_LSH                      25
#define IPU_DC_UGDE_0_AUTO_RESTART_LSH                26
#define IPU_DC_UGDE_0_NF_NL_LSH                       27

#define IPU_DC_LLA0_MCU_RS_0_0_LSH                    0
#define IPU_DC_LLA0_MCU_RS_1_0_LSH                    8
#define IPU_DC_LLA0_MCU_RS_2_0_LSH                    16
#define IPU_DC_LLA0_MCU_RS_3_0_LSH                    24

#define IPU_DC_LLA1_MCU_RS_0_1_LSH                    0
#define IPU_DC_LLA1_MCU_RS_1_1_LSH                    8
#define IPU_DC_LLA1_MCU_RS_2_1_LSH                    16
#define IPU_DC_LLA1_MCU_RS_3_1_LSH                    24

#define IPU_DC_R_LLA0_MCU_RS_R_0_0_LSH                0
#define IPU_DC_R_LLA0_MCU_RS_R_1_0_LSH                8
#define IPU_DC_R_LLA0_MCU_RS_R_2_0_LSH                16
#define IPU_DC_R_LLA0_MCU_RS_R_3_0_LSH                24

#define IPU_DC_R_LLA1_MCU_RS_R_0_1_LSH                0
#define IPU_DC_R_LLA1_MCU_RS_R_1_1_LSH                8
#define IPU_DC_R_LLA1_MCU_RS_R_2_1_LSH                16
#define IPU_DC_R_LLA1_MCU_RS_R_3_1_LSH                24

#define IPU_DC_STAT_DC_TRIPLE_BUF_CNT_FULL_0_LSH      0
#define IPU_DC_STAT_DC_TRIPLE_BUF_CNT_EMPTY_0_LSH     1
#define IPU_DC_STAT_DC_TRIPLE_BUF_DATA_FULL_0_LSH     2
#define IPU_DC_STAT_DC_TRIPLE_BUF_DATA_EMPTY_0_LSH    3
#define IPU_DC_STAT_DC_TRIPLE_BUF_CNT_FULL_1_LSH      4
#define IPU_DC_STAT_DC_TRIPLE_BUF_CNT_EMPTY_1_LSH     5
#define IPU_DC_STAT_DC_TRIPLE_BUF_DATA_FULL_1_LSH     6
#define IPU_DC_STAT_DC_TRIPLE_BUF_DATA_EMPTY_1_LSH    7

// DMFC registers
#define IPU_DMFC_RD_CHAN_DMFC_BURST_SIZE_0_LSH        6
#define IPU_DMFC_RD_CHAN_DMFC_WM_EN_0_LSH             17
#define IPU_DMFC_RD_CHAN_DMFC_WM_SET_0_LSH            18
#define IPU_DMFC_RD_CHAN_DMFC_WM_CLR_0_LSH            21
#define IPU_DMFC_RD_CHAN_DMFC_PPW_C_LSH               24

#define IPU_DMFC_WR_CHAN_DMFC_ST_ADDR_1_LSH           0
#define IPU_DMFC_WR_CHAN_DMFC_FIFO_SIZE_1_LSH         3
#define IPU_DMFC_WR_CHAN_DMFC_BURST_SIZE_1_LSH        6
#define IPU_DMFC_WR_CHAN_DMFC_ST_ADDR_2_LSH           8
#define IPU_DMFC_WR_CHAN_DMFC_FIFO_SIZE_2_LSH         11
#define IPU_DMFC_WR_CHAN_DMFC_BURST_SIZE_2_LSH        14
#define IPU_DMFC_WR_CHAN_DMFC_ST_ADDR_1c_LSH          16
#define IPU_DMFC_WR_CHAN_DMFC_FIFO_SIZE_1c_LSH        19
#define IPU_DMFC_WR_CHAN_DMFC_BURST_SIZE_1c_LSH       22
#define IPU_DMFC_WR_CHAN_DMFC_ST_ADDR_2c_LSH          24
#define IPU_DMFC_WR_CHAN_DMFC_FIFO_SIZE_2c_LSH        27
#define IPU_DMFC_WR_CHAN_DMFC_BURST_SIZE_2c_LSH       30

#define IPU_DMFC_WR_CHAN_DEF_WM_EN_1_LSH              1
#define IPU_DMFC_WR_CHAN_DEF_WM_SET_1_LSH             2
#define IPU_DMFC_WR_CHAN_DEF_WM_CLR_1_LSH             5
#define IPU_DMFC_WR_CHAN_DEF_WM_EN_2_LSH              9
#define IPU_DMFC_WR_CHAN_DEF_WM_SET_2_LSH             10
#define IPU_DMFC_WR_CHAN_DEF_WM_CLR_2_LSH             13
#define IPU_DMFC_WR_CHAN_DEF_WM_EN_1c_LSH             17
#define IPU_DMFC_WR_CHAN_DEF_WM_SET_1c_LSH            18
#define IPU_DMFC_WR_CHAN_DEF_WM_CLR_1c_LSH            21
#define IPU_DMFC_WR_CHAN_DEF_WM_EN_2c_LSH             25
#define IPU_DMFC_WR_CHAN_DEF_WM_SET_2c_LSH            26
#define IPU_DMFC_WR_CHAN_DEF_WM_CLR_2c_LSH            29

#define IPU_DMFC_DP_CHAN_DMFC_START_ADDR_5b_LSH       0
#define IPU_DMFC_DP_CHAN_DMFC_FIFO_SIZE_5b_LSH        3
#define IPU_DMFC_DP_CHAN_DMFC_BURST_SIZE_5b_LSH       6
#define IPU_DMFC_DP_CHAN_DMFC_START_ADDR_5f_LSH       8
#define IPU_DMFC_DP_CHAN_DMFC_FIFO_SIZE_5f_LSH        11
#define IPU_DMFC_DP_CHAN_DMFC_BURST_SIZE_5f_LSH       14
#define IPU_DMFC_DP_CHAN_DMFC_START_ADDR_6b_LSH       16
#define IPU_DMFC_DP_CHAN_DMFC_FIFO_SIZE_6b_LSH        19
#define IPU_DMFC_DP_CHAN_DMFC_BURST_SIZE_6b_LSH       22
#define IPU_DMFC_DP_CHAN_DMFC_START_ADDR_6f_LSH       24
#define IPU_DMFC_DP_CHAN_DMFC_FIFO_SIZE_6f_LSH        27
#define IPU_DMFC_DP_CHAN_DMFC_BURST_SIZE_6f_LSH       30

#define IPU_DMFC_DP_CHAN_DEF_DMFC_WM_EN_5b_LSH        1
#define IPU_DMFC_DP_CHAN_DEF_DMFC_WM_SET_5b_LSH       2
#define IPU_DMFC_DP_CHAN_DEF_DMFC_WM_CLR_5b_LSH       5
#define IPU_DMFC_DP_CHAN_DEF_DMFC_WM_EN_5f_LSH        9
#define IPU_DMFC_DP_CHAN_DEF_DMFC_WM_SET_5f_LSH       10
#define IPU_DMFC_DP_CHAN_DEF_DMFC_WM_CLR_5f_LSH       13
#define IPU_DMFC_DP_CHAN_DEF_DMFC_WM_EN_6b_LSH        17
#define IPU_DMFC_DP_CHAN_DEF_DMFC_WM_SET_6b_LSH       18
#define IPU_DMFC_DP_CHAN_DEF_DMFC_WM_CLR_6b_LSH       21
#define IPU_DMFC_DP_CHAN_DEF_DMFC_WM_EN_6f_LSH        25
#define IPU_DMFC_DP_CHAN_DEF_DMFC_WM_SET_6f_LSH       26
#define IPU_DMFC_DP_CHAN_DEF_DMFC_WM_CLR_6f_LSH       29

#define IPU_DMFC_GENERAL1_DMFC_DCDP_SYNC_PR_LSH       0
#define IPU_DMFC_GENERAL1_DMFC_BURST_SIZE_9_LSH       5
#define IPU_DMFC_GENERAL1_DMFC_WM_EN_9_LSH            9
#define IPU_DMFC_GENERAL1_DMFC_WM_SET_9_LSH           10
#define IPU_DMFC_GENERAL1_DMFC_WM_CLR_9_LSH           13
#define IPU_DMFC_GENERAL1_WAIT4EOT_1_LSH              16
#define IPU_DMFC_GENERAL1_WAIT4EOT_2_LSH              17
#define IPU_DMFC_GENERAL1_WAIT4EOT_3_LSH              18
#define IPU_DMFC_GENERAL1_WAIT4EOT_4_LSH              19
#define IPU_DMFC_GENERAL1_WAIT4EOT_5B_LSH             20
#define IPU_DMFC_GENERAL1_WAIT4EOT_5F_LSH             21
#define IPU_DMFC_GENERAL1_WAIT4EOT_6B_LSH             22
#define IPU_DMFC_GENERAL1_WAIT4EOT_6F_LSH             23
#define IPU_DMFC_GENERAL1_WAIT4EOT_9_LSH              24

#define IPU_DMFC_GENERAL2_DMFC_FRAME_WIDTH_RD_LSH     0
#define IPU_DMFC_GENERAL2_DMFC_FRAME_HEIGHT_RD_LSH    16

#define IPU_DMFC_IC_CTRL_DMFC_IC_IN_PORT_LSH          0
#define IPU_DMFC_IC_CTRL_DMFC_IC_PPW_C_LSH            4
#define IPU_DMFC_IC_CTRL_DMFC_IC_FRAME_WIDTH_RD_LSH   6
#define IPU_DMFC_IC_CTRL_DMFC_IC_FRAME_HEIGHT_RD_LSH  19

#define IPU_DMFC_STAT_DMFC_FIFO_FULL_0_LSH            0
#define IPU_DMFC_STAT_DMFC_FIFO_FULL_1_LSH            1
#define IPU_DMFC_STAT_DMFC_FIFO_FULL_2_LSH            2
#define IPU_DMFC_STAT_DMFC_FIFO_FULL_3_LSH            3
#define IPU_DMFC_STAT_DMFC_FIFO_FULL_4_LSH            4
#define IPU_DMFC_STAT_DMFC_FIFO_FULL_5_LSH            5
#define IPU_DMFC_STAT_DMFC_FIFO_FULL_6_LSH            6
#define IPU_DMFC_STAT_DMFC_FIFO_FULL_7_LSH            7
#define IPU_DMFC_STAT_DMFC_FIFO_FULL_8_LSH            8
#define IPU_DMFC_STAT_DMFC_FIFO_FULL_9_LSH            9
#define IPU_DMFC_STAT_DMFC_FIFO_FULL_10_LSH           10
#define IPU_DMFC_STAT_DMFC_FIFO_FULL_11_LSH           11
#define IPU_DMFC_STAT_DMFC_FIFO_EMPTY_0_LSH           12
#define IPU_DMFC_STAT_DMFC_FIFO_EMPTY_1_LSH           13
#define IPU_DMFC_STAT_DMFC_FIFO_EMPTY_2_LSH           14
#define IPU_DMFC_STAT_DMFC_FIFO_EMPTY_3_LSH           15
#define IPU_DMFC_STAT_DMFC_FIFO_EMPTY_4_LSH           16
#define IPU_DMFC_STAT_DMFC_FIFO_EMPTY_5_LSH           17
#define IPU_DMFC_STAT_DMFC_FIFO_EMPTY_6_LSH           18
#define IPU_DMFC_STAT_DMFC_FIFO_EMPTY_7_LSH           19
#define IPU_DMFC_STAT_DMFC_FIFO_EMPTY_8_LSH           20
#define IPU_DMFC_STAT_DMFC_FIFO_EMPTY_9_LSH           21
#define IPU_DMFC_STAT_DMFC_FIFO_EMPTY_10_LSH          22
#define IPU_DMFC_STAT_DMFC_FIFO_EMPTY_11_LSH          23
#define IPU_DMFC_STAT_DMFC_IC_BUFFER_FULL_LSH         24
#define IPU_DMFC_STAT_DMFC_IC_BUFFER_EMPTY_LSH        25


// VDI Registers
#define IPU_VDI_FSIZE_VDI_FWIDTH_LSH                  0
#define IPU_VDI_FSIZE_VDI_FHEIGHT_LSH                 16

#define IPU_VDI_C_VDI_CH_422_LSH                      1
#define IPU_VDI_C_VDI_MOT_SEL_LSH                     2
#define IPU_VDI_C_VDI_BURST_SIZE_1_LSH                4
#define IPU_VDI_C_VDI_BURST_SIZE_2_LSH                8
#define IPU_VDI_C_VDI_BURST_SIZE_3_LSH                12
#define IPU_VDI_C_VDI_VWMI1_SET_LSH                   16
#define IPU_VDI_C_VDI_VWMI1_CLR_LSH                   19
#define IPU_VDI_C_VDI_VWMI3_SET_LSH                   22
#define IPU_VDI_C_VDI_VWMI3_CLR_LSH                   25
#define IPU_VDI_C_VDI_TOP_FIELD_MAN_LSH               30
#define IPU_VDI_C_VDI_TOP_FIELD_AUTO_LSH              31

//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------

// IPU_CONF
#define IPU_IPU_CONF_CSI0_EN_WID                      1
#define IPU_IPU_CONF_CSI1_EN_WID                      1
#define IPU_IPU_CONF_IC_EN_WID                        1
#define IPU_IPU_CONF_IRT_EN_WID                       1
#define IPU_IPU_CONF_ISP_EN_WID                       1
#define IPU_IPU_CONF_DP_EN_WID                        1
#define IPU_IPU_CONF_DI0_EN_WID                       1
#define IPU_IPU_CONF_DI1_EN_WID                       1
#define IPU_IPU_CONF_SMFC_EN_WID                      1
#define IPU_IPU_CONF_CSI0_EN_WID                      1
#define IPU_IPU_CONF_CSI1_EN_WID                      1
#define IPU_IPU_CONF_DC_EN_WID                        1
#define IPU_IPU_CONF_DMFC_EN_WID                      1
#define IPU_IPU_CONF_SISG_EN_WID                      1
#define IPU_IPU_CONF_VDI_EN_WID                       1
#define IPU_IPU_CONF_DIAGBUS_MODE_WID                 5
#define IPU_IPU_CONF_DIAGBUS_ON_WID                   1
#define IPU_IPU_CONF_IDMAC_DISABLE_WID                1
#define IPU_IPU_CONF_ISP_DOUBLE_FLOW_WID              1
#define IPU_IPU_CONF_IC_DMFC_SEL_WID                  1
#define IPU_IPU_CONF_IC_DMFC_SYNC_WID                 1
#define IPU_IPU_CONF_VDI_DMFC_SYNC_WID                1
#define IPU_IPU_CONF_CSI0_DATA_SOURCE_WID             1
#define IPU_IPU_CONF_CSI1_DATA_SOURCE_WID             1
#define IPU_IPU_CONF_CSI_SEL_WID                      1
#define IPU_IPU_CONF_IC_INPUT_WID                     1

// IPU_SISG_CTRL0
#define IPU_SISG_CTRL0_VSYNC_RESET_COUNTER_WID        1
#define IPU_SISG_CTRL0_NO_OF_VSYNC_WID                2
#define IPU_SISG_CTRL0_VAL_STOP_SISG_COUNTER_WID      25
#define IPU_SISG_CTRL0_MCU_ACTV_TRIG_WID              1
#define IPU_SISG_CTRL0_EXT_ACTV_WID                   1

// IPU_SISG_CTRL1
#define IPU_SISG_CTRL1_SISG_STROBE_CNT_WID            5
#define IPU_SISG_CTRL1_SISG_OUT_POL_WID               6

// IPU DMA Channel ID used in the various IPU registers
#define IPU_DMA_CHA_0_WID                             1
#define IPU_DMA_CHA_1_WID                             1
#define IPU_DMA_CHA_2_WID                             1
#define IPU_DMA_CHA_3_WID                             1
#define IPU_DMA_CHA_4_WID                             1
#define IPU_DMA_CHA_5_WID                             1
#define IPU_DMA_CHA_6_WID                             1
#define IPU_DMA_CHA_7_WID                             1
#define IPU_DMA_CHA_8_WID                             1
#define IPU_DMA_CHA_9_WID                             1
#define IPU_DMA_CHA_10_WID                            1
#define IPU_DMA_CHA_11_WID                            1
#define IPU_DMA_CHA_12_WID                            1
#define IPU_DMA_CHA_13_WID                            1
#define IPU_DMA_CHA_14_WID                            1
#define IPU_DMA_CHA_15_WID                            1
#define IPU_DMA_CHA_16_WID                            1
#define IPU_DMA_CHA_17_WID                            1
#define IPU_DMA_CHA_18_WID                            1
#define IPU_DMA_CHA_19_WID                            1
#define IPU_DMA_CHA_20_WID                            1
#define IPU_DMA_CHA_21_WID                            1
#define IPU_DMA_CHA_22_WID                            1
#define IPU_DMA_CHA_23_WID                            1
#define IPU_DMA_CHA_24_WID                            1
#define IPU_DMA_CHA_25_WID                            1
#define IPU_DMA_CHA_26_WID                            1
#define IPU_DMA_CHA_27_WID                            1
#define IPU_DMA_CHA_28_WID                            1
#define IPU_DMA_CHA_29_WID                            1
#define IPU_DMA_CHA_30_WID                            1
#define IPU_DMA_CHA_31_WID                            1
#define IPU_DMA_CHA_32_WID                            1
#define IPU_DMA_CHA_33_WID                            1
#define IPU_DMA_CHA_34_WID                            1
#define IPU_DMA_CHA_35_WID                            1
#define IPU_DMA_CHA_36_WID                            1
#define IPU_DMA_CHA_37_WID                            1
#define IPU_DMA_CHA_38_WID                            1
#define IPU_DMA_CHA_39_WID                            1
#define IPU_DMA_CHA_40_WID                            1
#define IPU_DMA_CHA_41_WID                            1
#define IPU_DMA_CHA_42_WID                            1
#define IPU_DMA_CHA_43_WID                            1
#define IPU_DMA_CHA_44_WID                            1
#define IPU_DMA_CHA_45_WID                            1
#define IPU_DMA_CHA_46_WID                            1
#define IPU_DMA_CHA_47_WID                            1
#define IPU_DMA_CHA_48_WID                            1
#define IPU_DMA_CHA_49_WID                            1
#define IPU_DMA_CHA_50_WID                            1
#define IPU_DMA_CHA_51_WID                            1
#define IPU_DMA_CHA_52_WID                            1

// IPU_INT_CTRL_9 and IPU_INT_STAT_9
#define IPU_IC_BAYER_BUF_OVF_WID                  1
#define IPU_IC_ENC_BUF_OVF_WID                    1
#define IPU_IC_VF_BUF_OVF_WID                     1
#define IPU_ISP_PUPE_WID                          1
#define IPU_CSI0_PUPE_WID                         1
#define IPU_CSI1_PUPE_WID                         1

// IPU_INT_CTRL_15 & IPU_INT_STAT_15
#define IPU_SNOOPING1_INT_WID                     1
#define IPU_SNOOPING2_INT_WID                     1
#define IPU_DP_SF_START_WID                       1
#define IPU_DP_SF_END_WID                         1
#define IPU_DP_ASF_START_WID                      1
#define IPU_DP_ASF_END_WID                        1
#define IPU_DP_SF_BRAKE_WID                       1
#define IPU_DP_ASF_BRAKE_WID                      1
#define IPU_DC_FC_0_WID                           1
#define IPU_DC_FC_1_WID                           1
#define IPU_DC_FC_2_WID                           1
#define IPU_DC_FC_3_WID                           1
#define IPU_DC_FC_4_WID                           1
#define IPU_DC_FC_6_WID                           1
#define IPU_DI_VSYNC_PRE_0_WID                    1
#define IPU_DI_VSYNC_PRE_1_WID                    1
#define IPU_DC_DP_START_WID                       1
#define IPU_DC_ASYNC_STOP_WID                     1
#define IPU_DI0_CNT_EN_PRE_0_WID                  1
#define IPU_DI0_CNT_EN_PRE_1_WID                  1
#define IPU_DI0_CNT_EN_PRE_2_WID                  1
#define IPU_DI0_CNT_EN_PRE_3_WID                  1
#define IPU_DI0_CNT_EN_PRE_4_WID                  1
#define IPU_DI0_CNT_EN_PRE_5_WID                  1
#define IPU_DI0_CNT_EN_PRE_6_WID                  1
#define IPU_DI0_CNT_EN_PRE_7_WID                  1
#define IPU_DI0_CNT_EN_PRE_8_WID                  1
#define IPU_DI0_CNT_EN_PRE_9_WID                  1
#define IPU_DI0_CNT_EN_PRE_10_WID                 1
#define IPU_DI1_DISP_CLK_WID                      1
#define IPU_DI1_CNT_EN_PRE_3_WID                  1
#define IPU_DI1_CNT_EN_PRE_8_WID                  1

// IPU_SRM_PRI1
#define IPU_IPU_SRM_PRI1_CSI1_SRM_PRI_WID         3
#define IPU_IPU_SRM_PRI1_CSI1_SRM_MODE_WID        2
#define IPU_IPU_SRM_PRI1_CSI0_SRM_PRI_WID         3
#define IPU_IPU_SRM_PRI1_CSI0_SRM_MODE_WID        2
#define IPU_IPU_SRM_PRI1_ISP_SRM_PRI_WID          3
#define IPU_IPU_SRM_PRI1_ISP_SRM_MODE_WID         2

// IPU_SRM_PRI2
#define IPU_IPU_SRM_PRI2_DP_SRM_PRI_WID           3
#define IPU_IPU_SRM_PRI2_DP_S_SRM_MODE_WID        2
#define IPU_IPU_SRM_PRI2_DP_A0_SRM_MODE_WID       2
#define IPU_IPU_SRM_PRI2_DP_A1_SRM_MODE_WID       2
#define IPU_IPU_SRM_PRI2_DC_SRM_PRI_WID           3
#define IPU_IPU_SRM_PRI2_DC_2_SRM_MODE_WID        2
#define IPU_IPU_SRM_PRI2_DC_6_SRM_MODE_WID        2
#define IPU_IPU_SRM_PRI2_DI0_SRM_PRI_WID          3
#define IPU_IPU_SRM_PRI2_DI0_SRM_MCU_USE_WID      2
#define IPU_IPU_SRM_PRI2_DI1_SRM_PRI_WID          3
#define IPU_IPU_SRM_PRI2_DI1_SRM_MODE_WID         2

// IPU_FS_PROC_FLOW1
#define IPU_IPU_FS_PROC_FLOW1_PRPENC_ROT_SRC_SEL_WID  4
#define IPU_IPU_FS_PROC_FLOW1_ALT_ISP_SRC_SEL_WID     4
#define IPU_IPU_FS_PROC_FLOW1_PRPVF_ROT_SRC_SEL_WID   4
#define IPU_IPU_FS_PROC_FLOW1_PP_SRC_SEL_WID          4
#define IPU_IPU_FS_PROC_FLOW1_PP_ROT_SRC_SEL_WID      4
#define IPU_IPU_FS_PROC_FLOW1_ISP_SRC_SEL_WID         4
#define IPU_IPU_FS_PROC_FLOW1_PRP_SRC_SEL_WID         4
#define IPU_IPU_FS_PROC_FLOW1_VDI_SRC_SEL_WID         2
#define IPU_IPU_FS_PROC_FLOW1_ENC_IN_VALID_WID        1
#define IPU_IPU_FS_PROC_FLOW1_VF_IN_VALID_WID         1

// IPU_FS_PROC_FLOW2
#define IPU_IPU_FS_PROC_FLOW2_PRP_ENC_DEST_SEL_WID    4
#define IPU_IPU_FS_PROC_FLOW2_PRPVF_DEST_SEL_WID      4
#define IPU_IPU_FS_PROC_FLOW2_PRPVF_ROT_DEST_SEL_WID  4
#define IPU_IPU_FS_PROC_FLOW2_PP_DEST_SEL_WID         4
#define IPU_IPU_FS_PROC_FLOW2_PP_ROT_DEST_SEL_WID     4
#define IPU_IPU_FS_PROC_FLOW2_PRPENC_ROT_DEST_SEL_WID 4
#define IPU_IPU_FS_PROC_FLOW2_PRP_DEST_SEL_WID        2
#define IPU_IPU_FS_PROC_FLOW2_PRP_ALT_DEST_SEL_WID    2

// IPU_FS_PROC_FLOW3
#define IPU_IPU_FS_PROC_FLOW3_SMFC0_DEST_SEL_WID      4
#define IPU_IPU_FS_PROC_FLOW3_SMFC1_DEST_SEL_WID      3
#define IPU_IPU_FS_PROC_FLOW3_SMFC2_DEST_SEL_WID      4
#define IPU_IPU_FS_PROC_FLOW3_SMFC3_DEST_SEL_WID      3

// IPU_FS_DISP_FLOW1
#define IPU_IPU_FS_DISP_FLOW1_DP_SYNC0_SRC_SEL_WID    4
#define IPU_IPU_FS_DISP_FLOW1_DP_SYNC1_SRC_SEL_WID    4
#define IPU_IPU_FS_DISP_FLOW1_DP_ASYNC0_SRC_SEL_WID   4
#define IPU_IPU_FS_DISP_FLOW1_DP_ASYNC1_SRC_SEL_WID   4
#define IPU_IPU_FS_DISP_FLOW1_DC2_SRC_SEL_WID         4
#define IPU_IPU_FS_DISP_FLOW1_DC1_SRC_SEL_WID         4

// IPU_FS_DISP_FLOW2
#define IPU_IPU_FS_DISP_FLOW2_DP_ASYNC1_ALT_SRC_SEL_WID 4
#define IPU_IPU_FS_DISP_FLOW2_DP_ASYNC0_ALT_SRC_SEL_WID 4
#define IPU_IPU_FS_DISP_FLOW2_DC2_ALT_SRC_SEL_WID       4

// IPU_SKIP
#define IPU_IPU_SKIP_CSI_MAX_RATIO_SKIP_IC_ENC_WID    3
#define IPU_IPU_SKIP_CSI_SKIP_IC_ENC_WID              5
#define IPU_IPU_SKIP_CSI_MAX_RATIO_SKIP_IC_VF_WID     3
#define IPU_IPU_SKIP_CSI_SKIP_IC_VF_WID               5
#define IPU_IPU_SKIP_VDI_MAX_RATIO_SKIP_WID           4
#define IPU_IPU_SKIP_VDI_SKIP_WID                     12

// IPU_DISP_GEN
#define IPU_IPU_DISP_GEN_DI0_DUAL_MODE_WID            1
#define IPU_IPU_DISP_GEN_DI1_DUAL_MODE_WID            1
#define IPU_IPU_DISP_GEN_DC2_DOUBLE_FLOW_WID          1
#define IPU_IPU_DISP_GEN_DP_ASYNC_DOUBLE_FLOW_WID     1
#define IPU_IPU_DISP_GEN_DP_FG_EN_ASYNC0_WID          1
#define IPU_IPU_DISP_GEN_DP_FG_EN_ASYNC1_WID          1
#define IPU_IPU_DISP_GEN_DP_PIPE_CLR_WID              1
#define IPU_IPU_DISP_GEN_MCU_DI_ID_8_WID              1
#define IPU_IPU_DISP_GEN_MCU_DI_ID_9_WID              1
#define IPU_IPU_DISP_GEN_MCU_T_WID                    4
#define IPU_IPU_DISP_GEN_MCU_MAX_BURST_STOP_WID       1
#define IPU_IPU_DISP_GEN_DI0_COUNTER_RELEASE_WID      1
#define IPU_IPU_DISP_GEN_DI1_COUNTER_RELEASE_WID      1

// IPU_DISP_ALT1
#define IPU_IPU_DISP_ALT1_RUN_VALUE_M1_ALT_0_WID      12
#define IPU_IPU_DISP_ALT1_CNT_CLR_SEL_ALT_0_WID       3
#define IPU_IPU_DISP_ALT1_CNT_AUTO_RELOAD_ALT_0_WID   1
#define IPU_IPU_DISP_ALT1_STEP_REPEAT_ALT_0_WID       12
#define IPU_IPU_DISP_ALT1_SEL_ALT_0_WID               4

// IPU_DISP_ALT2
#define IPU_IPU_DISP_ALT2_OFFSET_VALUE_ALT_0_WID      12
#define IPU_IPU_DISP_ALT2_OFFSET_RESOLUTION_ALT_0_WID 3
#define IPU_IPU_DISP_ALT2_RUN_RESOLUTION_ALT_0_WID    3

// IPU_DISP_ALT3
#define IPU_IPU_DISP_ALT3_RUN_VALUE_M1_ALT_1_WID      12
#define IPU_IPU_DISP_ALT3_CNT_CLR_SEL_ALT_1_WID       3
#define IPU_IPU_DISP_ALT3_CNT_AUTO_RELOAD_ALT_1_WID   1
#define IPU_IPU_DISP_ALT3_STEP_REPEAT_ALT_1_WID       12
#define IPU_IPU_DISP_ALT3_SEL_ALT_1_WID               4

// IPU_DISP_ALT4
#define IPU_IPU_DISP_ALT4_OFFSET_VALUE_ALT_1_WID      12
#define IPU_IPU_DISP_ALT4_OFFSET_RESOLUTION_ALT_1_WID 3
#define IPU_IPU_DISP_ALT4_RUN_RESOLUTION_ALT_1_WID    3

// IPU_SNOOP
#define IPU_IPU_SNOOP_AUTOREF_PER_WID                 10
#define IPU_IPU_SNOOP_SNOOP2_SYNC_BYP_WID             1

// IPU_MEM_RST
#define IPU_IPU_MEM_RST_RST_MEM_EN_WID                23
#define IPU_IPU_MEM_RST_RST_MEM_START_WID             1

// IPU_PM
#define IPU_IPU_PM_DI0_CLK_PERIOD_0_WID               7
#define IPU_IPU_PM_DI0_CLK_PERIOD_1_WID               7
#define IPU_IPU_PM_DI0_SRM_CLOCK_CHANGE_MODE_WID      1
#define IPU_IPU_PM_CLOCK_MODE_STAT_WID                1
#define IPU_IPU_PM_DI1_CLK_PERIOD_0_WID               7
#define IPU_IPU_PM_DI1_CLK_PERIOD_1_WID               7
#define IPU_IPU_PM_DI1_SRM_CLOCK_CHANGE_MODE_WID      1
#define IPU_IPU_PM_LPSR_MODE_WID                      1

// IPU_GPR
#define IPU_IPU_GPR_IPU_DI0_CLK_CHANGE_ACK_DIS_WID    1
#define IPU_IPU_GPR_IPU_DI1_CLK_CHANGE_ACK_DIS_WID    1
#define IPU_IPU_GPR_IPU_ALT_CH_BUF0_RDY0_CLR_WID      1
#define IPU_IPU_GPR_IPU_ALT_CH_BUF0_RDY1_CLR_WID      1
#define IPU_IPU_GPR_IPU_ALT_CH_BUF1_RDY0_CLR_WID      1
#define IPU_IPU_GPR_IPU_ALT_CH_BUF1_RDY1_CLR_WID      1
#define IPU_IPU_GPR_IPU_CH_BUF0_RDY0_CLR_WID          1
#define IPU_IPU_GPR_IPU_CH_BUF0_RDY1_CLR_WID          1
#define IPU_IPU_GPR_IPU_CH_BUF1_RDY0_CLR_WID          1
#define IPU_IPU_GPR_IPU_CH_BUF1_RDY1_CLR_WID          1

// IPU_SRM_STAT
#define IPU_IPU_SRM_STAT_DP_S_SRM_STAT_WID            1
#define IPU_IPU_SRM_STAT_DP_A0_SRM_STAT_WID           1
#define IPU_IPU_SRM_STAT_DP_A1_SRM_STAT_WID           1
#define IPU_IPU_SRM_STAT_DC_2_SRM_STAT_WID            1
#define IPU_IPU_SRM_STAT_DC_6_SRM_STAT_WID            1
#define IPU_IPU_SRM_STAT_DI0_SRM_STAT_WID             1
#define IPU_IPU_SRM_STAT_DI1_SRM_STAT_WID             1

// IPU_PROC_TASKS_STAT
#define IPU_IPU_PROC_TASKS_STAT_ENC_TSTAT_WID         2
#define IPU_IPU_PROC_TASKS_STAT_VF_TSTAT_WID          2
#define IPU_IPU_PROC_TASKS_STAT_PP_TSTAT_WID          2
#define IPU_IPU_PROC_TASKS_STAT_ENC_ROT_TSTAT_WID     2
#define IPU_IPU_PROC_TASKS_STAT_VF_ROT_TSTAT_WID      2
#define IPU_IPU_PROC_TASKS_STAT_PP_ROT_TSTAT_WID      2
#define IPU_IPU_PROC_TASKS_STAT_MEM2PRP_TSTAT_WID     3
#define IPU_IPU_PROC_TASKS_STAT_CSI2MEM_SMFC0_TSTAT_WID  2
#define IPU_IPU_PROC_TASKS_STAT_CSI2MEM_SMFC1_TSTAT_WID  2
#define IPU_IPU_PROC_TASKS_STAT_CSI2MEM_SMFC2_TSTAT_WID  2
#define IPU_IPU_PROC_TASKS_STAT_CSI2MEM_SMFC3_TSTAT_WID  2

// IPU_DISP_TASKS_STAT
#define IPU_IPU_DISP_TASKS_STAT_DP_ASYNC_TSTAT_WID    3
#define IPU_IPU_DISP_TASKS_STAT_DP_ASYNC_CUR_FLOW_WID 1
#define IPU_IPU_DISP_TASKS_STAT_DC_ASYNC1_TSTAT_WID   2
#define IPU_IPU_DISP_TASKS_STAT_DC_ASYNC2_TSTAT_WID   3
#define IPU_IPU_DISP_TASKS_STAT_DC_ASYNC2_CUR_FLOW_WID 1



// IDMAC_CONF
#define IPU_IDMAC_CONF_MAX_REQ_READ_WID               3
#define IPU_IDMAC_CONF_WIDPT_WID                      2
#define IPU_IDMAC_CONF_P_ENDIAN_WID                   1

// IDMAC_SUB_ADDR_1
#define IPU_IDMAC_SUB_ADDR_1_IDMAC_SUB_ADDR_23_WID    7
#define IPU_IDMAC_SUB_ADDR_1_IDMAC_SUB_ADDR_24_WID    7
#define IPU_IDMAC_SUB_ADDR_1_IDMAC_SUB_ADDR_29_WID    7
#define IPU_IDMAC_SUB_ADDR_1_IDMAC_SUB_ADDR_33_WID    7

// IDMAC_SUB_ADDR_2
#define IPU_IDMAC_SUB_ADDR_2_IDMAC_SUB_ADDR_41_WID    7
#define IPU_IDMAC_SUB_ADDR_2_IDMAC_SUB_ADDR_51_WID    7
#define IPU_IDMAC_SUB_ADDR_2_IDMAC_SUB_ADDR_52_WID    7

// IDMAC_SC_CORD
#define IPU_IDMAC_SC_CORD_SY0_WID                     11
#define IPU_IDMAC_SC_CORD_SX0_WID                     12


// Note: The register sets are identical for the SYNC,
// ASYNC0, and ASYNC1 display ports, so only one set of
// register WID values is needed.

// DP_COM_CONF
#define IPU_DP_COM_CONF_DP_FG_EN_WID                  1
#define IPU_DP_COM_CONF_DP_GWSEL_WID                  1
#define IPU_DP_COM_CONF_DP_GWAM_WID                   1
#define IPU_DP_COM_CONF_DP_GWCKE_WID                  1
#define IPU_DP_COM_CONF_DP_COC_WID                    3
#define IPU_DP_COM_CONF_DP_CSC_DEF_WID                2
#define IPU_DP_COM_CONF_DP_CSC_GAMUT_SAT_EN_WID       1
#define IPU_DP_COM_CONF_DP_CSC_YUV_SAT_MODE_WID       1
#define IPU_DP_COM_CONF_DP_GAMMA_EN_WID               1
#define IPU_DP_COM_CONF_DP_GAMMA_YUV_EN_WID           1

// DP_GRAPH_WIND_CTRL
#define IPU_DP_GRAPH_WIND_CTRL_DP_GWCKB_WID           8
#define IPU_DP_GRAPH_WIND_CTRL_DP_GWCKG_WID           8
#define IPU_DP_GRAPH_WIND_CTRL_DP_GWCKR_WID           8
#define IPU_DP_GRAPH_WIND_CTRL_DP_GWAV_WID            8

// DP_FG_POS
#define IPU_DP_FG_POS_DP_FGYP_WID                     11
#define IPU_DP_FG_POS_DP_FGXP_WID                     11

// DP_CUR_POS
#define IPU_DP_CUR_POS_DP_CYP_WID                     11
#define IPU_DP_CUR_POS_DP_CYH_WID                     5
#define IPU_DP_CUR_POS_DP_CXP_WID                     11
#define IPU_DP_CUR_POS_DP_CXW_WID                     5

// DP_CUR_MAP
#define IPU_DP_CUR_MAP_DP_CUR_COL_R_WID               8
#define IPU_DP_CUR_MAP_DP_CUR_COL_G_WID               8
#define IPU_DP_CUR_MAP_DP_CUR_COL_B_WID               8

// DP_CSC_0
#define IPU_DP_CSC_0_DP_CSC_A8_WID                    10
#define IPU_DP_CSC_0_DP_CSC_B0_WID                    14
#define IPU_DP_CSC_0_DP_CSC_S0_WID                    2

// DP_CSC_1
#define IPU_DP_CSC_1_DP_CSC_B1_WID                    14
#define IPU_DP_CSC_1_DP_CSC_S1_WID                    2
#define IPU_DP_CSC_1_DP_CSC_B2_WID                    14
#define IPU_DP_CSC_1_DP_CSC_S2_WID                    2

// DP_DEBUG_CNT
#define IPU_DP_DEBUG_CNT_BRAKE_STATUS_EN_0_WID        1
#define IPU_DP_DEBUG_CNT_BRAKE_CNT_0_WID              3
#define IPU_DP_DEBUG_CNT_BRAKE_STATUS_EN_1_WID        1
#define IPU_DP_DEBUG_CNT_BRAKE_CNT_1_WID              3

// DP_DEBUG_STAT
#define IPU_DP_DEBUG_STAT_V_CNT_OLD_0_WID             11
#define IPU_DP_DEBUG_STAT_FG_ACTIVE_0_WID             1
#define IPU_DP_DEBUG_STAT_COMBYP_EN_OLD_0_WID         1
#define IPU_DP_DEBUG_STAT_CYP_EN_OLD_0_WID            1
#define IPU_DP_DEBUG_STAT_V_CNT_OLD_1_WID             11
#define IPU_DP_DEBUG_STAT_FG_ACTIVE_1_WID             1
#define IPU_DP_DEBUG_STAT_COMBYP_EN_OLD_1_WID         1
#define IPU_DP_DEBUG_STAT_CYP_EN_OLD_1_WID            1


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

#define IPU_IC_IDMAC_1_CB0_BURST_16_WID         1
#define IPU_IC_IDMAC_1_CB1_BURST_16_WID         1
#define IPU_IC_IDMAC_1_CB2_BURST_16_WID         1
#define IPU_IC_IDMAC_1_CB3_BURST_16_WID         1
#define IPU_IC_IDMAC_1_CB4_BURST_16_WID         1
#define IPU_IC_IDMAC_1_CB5_BURST_16_WID         1
#define IPU_IC_IDMAC_1_CB6_BURST_16_WID         1
#define IPU_IC_IDMAC_1_CB7_BURST_16_WID         1
#define IPU_IC_IDMAC_1_T1_ROT_WID               1
#define IPU_IC_IDMAC_1_T1_FLIP_LR_WID           1
#define IPU_IC_IDMAC_1_T1_FLIP_UD_WID           1
#define IPU_IC_IDMAC_1_T2_ROT_WID               1
#define IPU_IC_IDMAC_1_T2_FLIP_LR_WID           1
#define IPU_IC_IDMAC_1_T2_FLIP_UD_WID           1
#define IPU_IC_IDMAC_1_T3_ROT_WID               1
#define IPU_IC_IDMAC_1_T3_FLIP_LR_WID           1
#define IPU_IC_IDMAC_1_T3_FLIP_UD_WID           1
#define IPU_IC_IDMAC_1_ALT_CB6_BURST_16_WID     1
#define IPU_IC_IDMAC_1_ALT_CB7_BURST_16_WID     1

#define IPU_IC_IDMAC_2_T1_FR_HEIGHT_WID         10
#define IPU_IC_IDMAC_2_T2_FR_HEIGHT_WID         10
#define IPU_IC_IDMAC_2_T3_FR_HEIGHT_WID         10

#define IPU_IC_IDMAC_3_T1_FR_WIDTH_WID          10
#define IPU_IC_IDMAC_3_T2_FR_WIDTH_WID          10
#define IPU_IC_IDMAC_3_T3_FR_WIDTH_WID          10

#define IPU_IC_IDMAC_4_MPM_RW_BRDG_MAX_RQ_WID   4
#define IPU_IC_IDMAC_4_MPM_DMFC_BRDG_MAX_RQ_WID 4
#define IPU_IC_IDMAC_4_IBM_BRDG_MAX_RQ_WID      4
#define IPU_IC_IDMAC_4_RM_BRDG_MAX_RQ_WID       4


// IPU DI0 Registers
#define IPU_DI_GENERAL_DI_POLARITY_1_WID              1
#define IPU_DI_GENERAL_DI_POLARITY_2_WID              1
#define IPU_DI_GENERAL_DI_POLARITY_3_WID              1
#define IPU_DI_GENERAL_DI_POLARITY_4_WID              1
#define IPU_DI_GENERAL_DI_POLARITY_5_WID              1
#define IPU_DI_GENERAL_DI_POLARITY_6_WID              1
#define IPU_DI_GENERAL_DI_POLARITY_7_WID              1
#define IPU_DI_GENERAL_DI_POLARITY_8_WID              1
#define IPU_DI_GENERAL_DI_POLARITY_CS0_WID            1
#define IPU_DI_GENERAL_DI_POLARITY_CS1_WID            1
#define IPU_DI_GENERAL_DI_ERM_VSYNC_SEL_WID           1
#define IPU_DI_GENERAL_DI_ERR_TREATMENT_WID           1
#define IPU_DI_GENERAL_DI_SYNC_COUNT_SEL_WID          4
#define IPU_DI_GENERAL_DI_POLARITY_DISP_CLK_WID       1
#define IPU_DI_GENERAL_DI_WATCHDOG_MODE_WID           2
#define IPU_DI_GENERAL_DI_CLK_EXT_WID                 1
#define IPU_DI_GENERAL_DI_VSYNC_EXT_WID               1
#define IPU_DI_GENERAL_DI_MASK_SEL_WID                1
#define IPU_DI_GENERAL_DI_DISP_CLOCK_INIT_WID         1
#define IPU_DI_GENERAL_DI_CLOCK_STOP_MODE_WID         4
#define IPU_DI_GENERAL_DI_DISP_Y_SEL_WID              3

#define IPU_DI_BS_CLKGEN0_DI_DISP_CLK_PERIOD_WID      12
#define IPU_DI_BS_CLKGEN0_DI_DISP_CLK_OFFSET_WID      9

#define IPU_DI_BS_CLKGEN1_DI_DISP_CLK_UP_WID          9
#define IPU_DI_BS_CLKGEN1_DI_DISP_CLK_DOWN_WID        9

#define IPU_DI_SW_GEN0_DI_OFFSET_RESOLUTION_WID       3
#define IPU_DI_SW_GEN0_DI_OFFSET_VALUE_WID            12
#define IPU_DI_SW_GEN0_DI_RUN_RESOLUTION_WID          3
#define IPU_DI_SW_GEN0_DI_RUN_VALUE_M1_WID            12

#define IPU_DI_SW_GEN1_DI_CNT_UP_WID                  9
#define IPU_DI_SW_GEN1_DI_CNT_POLARITY_CLR_SEL_WID    3
#define IPU_DI_SW_GEN1_DI_CNT_POLARITY_TRIGGER_SEL_WID 3
#define IPU_DI_SW_GEN1_DI_CNT_DOWN_WID                9
#define IPU_DI_SW_GEN1_DI_CNT_CLR_SEL_WID             3
#define IPU_DI_SW_GEN1_DI_CNT_AUTO_RELOAD_WID         1
#define IPU_DI_SW_GEN1_DI_CNT_POLARITY_GEN_EN_WID     2

// Unique values to DI0_SW_GEN1_9
#define IPU_DI_SW_GEN1_DI_TAG_SEL_WID                 1
#define IPU_DI_SW_GEN1_DI_GENTIME_SEL_WID             3

#define IPU_DI_SYNC_AS_GEN_DI_SYNC_START_WID          12
#define IPU_DI_SYNC_AS_GEN_DI_VSYNC_SEL_WID           3
#define IPU_DI_SYNC_AS_GEN_DI_SYNC_START_EN_WID       1

// For parallel interface
#define IPU_DI_DW_GEN_DI_PT_0_WID                     2
#define IPU_DI_DW_GEN_DI_PT_1_WID                     2
#define IPU_DI_DW_GEN_DI_PT_2_WID                     2
#define IPU_DI_DW_GEN_DI_PT_3_WID                     2
#define IPU_DI_DW_GEN_DI_PT_4_WID                     2
#define IPU_DI_DW_GEN_DI_PT_5_WID                     2
#define IPU_DI_DW_GEN_DI_PT_6_WID                     2
#define IPU_DI_DW_GEN_DI_CST_WID                      2
#define IPU_DI_DW_GEN_DI_COMPONENT_SIZE_WID           8
#define IPU_DI_DW_GEN_DI_ACCESS_SIZE_WID              8

// For serial interface
#define IPU_DI_DW_GEN_DI_SERIAL_CLK_WID               2
#define IPU_DI_DW_GEN_DI_SERIAL_RS_WID                2
#define IPU_DI_DW_GEN_DI_SERIAL_VALID_BITS_WID        5
// Serial CST same as for parallel, so we don't redefine
#define IPU_DI_DW_GEN_DI_START_PERIOD_WID             8
#define IPU_DI_DW_GEN_DI_SERIAL_PERIOD_WID            8

#define IPU_DI_DW_SET_DI_DATA_CNT_UP_WID              9
#define IPU_DI_DW_SET_DI_DATA_CNT_DOWN_WID            9

#define IPU_DI_STP_REP_STEP_REPEAT_L_WID              12
#define IPU_DI_STP_REP_STEP_REPEAT_H_WID              12

#define IPU_DI_SER_CONF_WAIT4SERIAL_WID               1
#define IPU_DI_SER_CONF_SERIAL_CS_POLARITY_WID        1
#define IPU_DI_SER_CONF_SERIAL_RS_POLARITY_WID        1
#define IPU_DI_SER_CONF_SERIAL_DATA_POLARITY_WID      1
#define IPU_DI_SER_CONF_SER_CLK_POLARITY_WID          1
#define IPU_DI_SER_CONF_LLA_SER_ACCESS_WID            1
#define IPU_DI_SER_CONF_SERIAL_LATCH_WID              8
#define IPU_DI_SER_CONF_SERIAL_LLA_PNTR_RS_W_0_WID    4
#define IPU_DI_SER_CONF_SERIAL_LLA_PNTR_RS_W_1_WID    4
#define IPU_DI_SER_CONF_SERIAL_LLA_PNTR_RS_R_0_WID    4
#define IPU_DI_SER_CONF_SERIAL_LLA_PNTR_RS_R_1_WID    4

#define IPU_DI_SSC_BYTE_EN_PNTR_WID                   3
#define IPU_DI_SSC_BYTE_EN_READ_IN_WID                1
#define IPU_DI_SSC_WAIT_ON_WID                        1
#define IPU_DI_SSC_CS_ERM_WID                         1
#define IPU_DI_SSC_PIN11_ERM_WID                      1
#define IPU_DI_SSC_PIN12_ERM_WID                      1
#define IPU_DI_SSC_PIN13_ERM_WID                      1
#define IPU_DI_SSC_PIN14_ERM_WID                      1
#define IPU_DI_SSC_PIN15_ERM_WID                      1
#define IPU_DI_SSC_PIN16_ERM_WID                      1
#define IPU_DI_SSC_PIN17_ERM_WID                      1

#define IPU_DI_POL_DRDY_POLARITY_11_WID               1
#define IPU_DI_POL_DRDY_POLARITY_12_WID               1
#define IPU_DI_POL_DRDY_POLARITY_13_WID               1
#define IPU_DI_POL_DRDY_POLARITY_14_WID               1
#define IPU_DI_POL_DRDY_POLARITY_15_WID               1
#define IPU_DI_POL_DRDY_POLARITY_16_WID               1
#define IPU_DI_POL_DRDY_POLARITY_17_WID               1
#define IPU_DI_POL_DRDY_DATA_POLARITY_WID             1
#define IPU_DI_POL_CS0_POLARITY_11_WID                1
#define IPU_DI_POL_CS0_POLARITY_12_WID                1
#define IPU_DI_POL_CS0_POLARITY_13_WID                1
#define IPU_DI_POL_CS0_POLARITY_14_WID                1
#define IPU_DI_POL_CS0_POLARITY_15_WID                1
#define IPU_DI_POL_CS0_POLARITY_16_WID                1
#define IPU_DI_POL_CS0_POLARITY_17_WID                1
#define IPU_DI_POL_CS0_DATA_POLARITY_WID              1
#define IPU_DI_POL_CS1_POLARITY_11_WID                1
#define IPU_DI_POL_CS1_POLARITY_12_WID                1
#define IPU_DI_POL_CS1_POLARITY_13_WID                1
#define IPU_DI_POL_CS1_POLARITY_14_WID                1
#define IPU_DI_POL_CS1_POLARITY_15_WID                1
#define IPU_DI_POL_CS1_POLARITY_16_WID                1
#define IPU_DI_POL_CS1_POLARITY_17_WID                1
#define IPU_DI_POL_CS1_DATA_POLARITY_WID              1
#define IPU_DI_POL_CS0_BYTE_EN_POLARITY_WID           1
#define IPU_DI_POL_CS1_BYTE_EN_POLARITY_WID           1
#define IPU_DI_POL_WAIT_POLARITY_WID                  1

#define IPU_DI_AW0_AW_HSTART_WID                      12
#define IPU_DI_AW0_AW_HCOUNT_SEL_WID                  4
#define IPU_DI_AW0_AW_HEND_WID                        12
#define IPU_DI_AW0_AW_TRIG_SEL_WID                    4

#define IPU_DI_AW1_AW_VSTART_WID                      12
#define IPU_DI_AW1_AW_VCOUNT_SEL_WID                  4
#define IPU_DI_AW1_AW_VEND_WID                        12

#define IPU_DI_SCR_CONF_SCREEN_HEIGHT_WID             12

#define IPU_DI_STAT_READ_FIFO_EMPTY_WID               1
#define IPU_DI_STAT_READ_FIFO_FULL_WID                1
#define IPU_DI_STAT_CNTR_FIFO_EMPTY_WID               1
#define IPU_DI_STAT_CNTR_FIFO_FULL_WID                1

// IPU DC Registers
#define IPU_DC_READ_CH_CONF_RD_CHANNEL_EN_WID         1
#define IPU_DC_READ_CH_CONF_PROG_DI_ID_0_WID          1
#define IPU_DC_READ_CH_CONF_PROG_DISP_ID_0_WID        2
#define IPU_DC_READ_CH_CONF_W_SIZE_0_WID              2
#define IPU_DC_READ_CH_CONF_CHAN_MASK_DEFAULT_0_WID   1
#define IPU_DC_READ_CH_CONF_CS_ID_0_WID               1
#define IPU_DC_READ_CH_CONF_CS_ID_1_WID               1
#define IPU_DC_READ_CH_CONF_CS_ID_2_WID               1
#define IPU_DC_READ_CH_CONF_CS_ID_3_WID               1
#define IPU_DC_READ_CH_CONF_TIME_OUT_VALUE_WID        16


#define IPU_DC_RL0_CH_COD_NF_PRIORITY_CHAN_WID        4
#define IPU_DC_RL0_CH_COD_NF_START_CHAN_WID           8
#define IPU_DC_RL0_CH_COD_NL_PRIORITY_CHAN_WID        4
#define IPU_DC_RL0_CH_COD_NL_START_CHAN_WID           8

#define IPU_DC_RL1_CH_COD_EOF_PRIORITY_CHAN_WID       4
#define IPU_DC_RL1_CH_COD_EOF_START_CHAN_WID          8
#define IPU_DC_RL1_CH_COD_NFIELD_PRIORITY_CHAN_WID    4
#define IPU_DC_RL1_CH_COD_NFIELD_START_CHAN_WID       8

#define IPU_DC_RL2_CH_COD_EOL_PRIORITY_CHAN_WID       4
#define IPU_DC_RL2_CH_COD_EOL_START_CHAN_WID          8
#define IPU_DC_RL2_CH_COD_EOFIELD_PRIORITY_CHAN_WID   4
#define IPU_DC_RL2_CH_COD_EOFIELD_START_CHAN_WID      8

#define IPU_DC_RL3_CH_COD_NEW_ADDR_PRIORITY_CHAN_WID  4
#define IPU_DC_RL3_CH_COD_NEW_ADDR_START_CHAN_WID     8
#define IPU_DC_RL3_CH_COD_NEW_CHAN_PRIORITY_CHAN_WID  4
#define IPU_DC_RL3_CH_COD_NEW_CHAN_START_CHAN_WID     8

#define IPU_DC_RL4_CH_COD_NEW_DATA_PRIORITY_CHAN_WID   4
#define IPU_DC_RL4_CH_COD_NEW_DATA_START_CHAN_WID      8

#define IPU_DC_RL1_CH_COD_NEW_ADDR_PRIORITY_CHAN_8_9_WID  4
#define IPU_DC_RL1_CH_COD_NEW_ADDR_START_CHAN_W_0_WID     8
#define IPU_DC_RL1_CH_COD_NEW_ADDR_START_CHAN_W_1_WID     8

#define IPU_DC_RL2_CH_COD_NEW_CHAN_PRIORITY_CHAN_8_9_WID  4
#define IPU_DC_RL2_CH_COD_NEW_CHAN_START_CHAN_W_0_WID     8
#define IPU_DC_RL2_CH_COD_NEW_CHAN_START_CHAN_W_1_WID     8

#define IPU_DC_RL3_CH_COD_NEW_DATA_PRIORITY_CHAN_8_9_WID  4
#define IPU_DC_RL3_CH_COD_NEW_DATA_START_CHAN_W_0_WID     8
#define IPU_DC_RL3_CH_COD_NEW_DATA_START_CHAN_W_1_WID     8

#define IPU_DC_RL4_CH_COD_NEW_ADDR_START_CHAN_R_0_WID  8
#define IPU_DC_RL4_CH_COD_NEW_ADDR_START_CHAN_R_1_WID  8

#define IPU_DC_RL5_CH_COD_NEW_CHAN_START_CHAN_R_0_WID  8
#define IPU_DC_RL5_CH_COD_NEW_CHAN_START_CHAN_R_1_WID  8

#define IPU_DC_RL6_CH_COD_NEW_DATA_START_CHAN_R_0_WID  8
#define IPU_DC_RL6_CH_COD_NEW_DATA_START_CHAN_R_1_WID  8

#define IPU_DC_WR_CH_CONF_1_W_SIZE_1_WID              2
#define IPU_DC_WR_CH_CONF_1_PROG_DI_ID_1_WID          1
#define IPU_DC_WR_CH_CONF_1_PROG_DISP_ID_1_WID        2
#define IPU_DC_WR_CH_CONF_1_PROG_CHAN_TYP_1_WID       3
#define IPU_DC_WR_CH_CONF_1_CHAN_MASK_DEFAULT_1_WID   1
#define IPU_DC_WR_CH_CONF_1_FIELD_MODE_1_WID          1
#define IPU_DC_WR_CH_CONF_1_PROG_START_TIME_1_WID     11

#define IPU_DC_WR_CH_CONF_2_W_SIZE_2_WID              2
#define IPU_DC_WR_CH_CONF_2_PROG_DI_ID_2_WID          1
#define IPU_DC_WR_CH_CONF_2_PROG_DISP_ID_2_WID        2
#define IPU_DC_WR_CH_CONF_2_PROG_CHAN_TYP_2_WID       3
#define IPU_DC_WR_CH_CONF_2_CHAN_MASK_DEFAULT_2_WID   1
#define IPU_DC_WR_CH_CONF_2_PROG_START_TIME_2_WID     11

#define IPU_DC_CMD_CH_CONF_3_W_SIZE_3_WID             2
#define IPU_DC_CMD_CH_CONF_3_COD_CMND_START_CHAN_RS0_3_WID  8
#define IPU_DC_CMD_CH_CONF_3_COD_CMND_START_CHAN_RS1_3_WID  8

#define IPU_DC_CMD_CH_CONF_4_W_SIZE_4_WID             2
#define IPU_DC_CMD_CH_CONF_4_COD_CMND_START_CHAN_RS0_4_WID  8
#define IPU_DC_CMD_CH_CONF_4_COD_CMND_START_CHAN_RS1_4_WID  8

#define IPU_DC_WR_CH_CONF_5_W_SIZE_5_WID              2
#define IPU_DC_WR_CH_CONF_5_PROG_DI_ID_5_WID          1
#define IPU_DC_WR_CH_CONF_5_PROG_DISP_ID_5_WID        2
#define IPU_DC_WR_CH_CONF_5_PROG_CHAN_TYP_5_WID       3
#define IPU_DC_WR_CH_CONF_5_CHAN_MASK_DEFAULT_5_WID   1
#define IPU_DC_WR_CH_CONF_5_FIELD_MODE_5_WID          1
#define IPU_DC_WR_CH_CONF_5_PROG_START_TIME_5_WID     11

#define IPU_DC_WR_CH_CONF_6_W_SIZE_6_WID              2
#define IPU_DC_WR_CH_CONF_6_PROG_DI_ID_6_WID          1
#define IPU_DC_WR_CH_CONF_6_PROG_DISP_ID_6_WID        2
#define IPU_DC_WR_CH_CONF_6_PROG_CHAN_TYP_6_WID       3
#define IPU_DC_WR_CH_CONF_6_CHAN_MASK_DEFAULT_6_WID   1
#define IPU_DC_WR_CH_CONF_6_PROG_START_TIME_6_WID     11

#define IPU_DC_WR_CH_CONF1_8_W_SIZE_8_WID             2
#define IPU_DC_WR_CH_CONF1_8_CHAN_MASK_DEFAULT_8_WID  1
#define IPU_DC_WR_CH_CONF1_8_MCU_DISP_ID_8_WID        2

#define IPU_DC_WR_CH_CONF1_9_W_SIZE_9_WID             2
#define IPU_DC_WR_CH_CONF1_9_CHAN_MASK_DEFAULT_9_WID  1
#define IPU_DC_WR_CH_CONF1_9_MCU_DISP_ID_9_WID        2

#define IPU_DC_GEN_SYNC_1_6_WID                       2
#define IPU_DC_GEN_MASK_EN_WID                        1
#define IPU_DC_GEN_MASK4CHAN_5_WID                    1
#define IPU_DC_GEN_SYNC_PRIORITY_5_WID                1
#define IPU_DC_GEN_SYNC_PRIORITY_1_WID                1
#define IPU_DC_GEN_DC_CH5_TYPE_WID                    1
#define IPU_DC_GEN_DC_BKDIV_WID                       8
#define IPU_DC_GEN_DC_BK_EN_WID                       1

#define IPU_DC_DISP_CONF1_DISP_TYP_WID                2
#define IPU_DC_DISP_CONF1_ADDR_INCREMENT_WID          2
#define IPU_DC_DISP_CONF1_ADDR_BE_L_INC_WID           2
#define IPU_DC_DISP_CONF1_MCU_ACC_LB_MASK_WID         1
#define IPU_DC_DISP_CONF1_DISP_RD_VALUE_PTR_WID       1

#define IPU_DC_MAP_CONF_MAPPING_PNTR_BYTE0_0_WID      5
#define IPU_DC_MAP_CONF_MAPPING_PNTR_BYTE1_0_WID      5
#define IPU_DC_MAP_CONF_MAPPING_PNTR_BYTE2_0_WID      5
#define IPU_DC_MAP_CONF_MAPPING_PNTR_BYTE0_1_WID      5
#define IPU_DC_MAP_CONF_MAPPING_PNTR_BYTE1_1_WID      5
#define IPU_DC_MAP_CONF_MAPPING_PNTR_BYTE2_1_WID      5

#define IPU_DC_MAP_CONF_MD_MASK_0_WID                 8
#define IPU_DC_MAP_CONF_MD_OFFSET_0_WID               5
#define IPU_DC_MAP_CONF_MD_MASK_1_WID                 8
#define IPU_DC_MAP_CONF_MD_OFFSET_1_WID               5

#define IPU_DC_UGDE_0_ID_CODED_WID                    3
#define IPU_DC_UGDE_0_COD_EV_PRIORITY_WID             4
#define IPU_DC_UGDE_0_COD_EV_START_WID                8
#define IPU_DC_UGDE_0_COD_ODD_START_WID               8
#define IPU_DC_UGDE_0_ODD_EN_WID                      1
#define IPU_DC_UGDE_0_AUTO_RESTART_WID                1
#define IPU_DC_UGDE_0_NF_NL_WID                       2

#define IPU_DC_LLA0_MCU_RS_0_0_WID                    8
#define IPU_DC_LLA0_MCU_RS_1_0_WID                    8
#define IPU_DC_LLA0_MCU_RS_2_0_WID                    8
#define IPU_DC_LLA0_MCU_RS_3_0_WID                    8

#define IPU_DC_LLA1_MCU_RS_0_1_WID                    8
#define IPU_DC_LLA1_MCU_RS_1_1_WID                    8
#define IPU_DC_LLA1_MCU_RS_2_1_WID                    8
#define IPU_DC_LLA1_MCU_RS_3_1_WID                    8

#define IPU_DC_R_LLA0_MCU_RS_R_0_0_WID                8
#define IPU_DC_R_LLA0_MCU_RS_R_1_0_WID                8
#define IPU_DC_R_LLA0_MCU_RS_R_2_0_WID                8
#define IPU_DC_R_LLA0_MCU_RS_R_3_0_WID                8

#define IPU_DC_R_LLA1_MCU_RS_R_0_1_WID                8
#define IPU_DC_R_LLA1_MCU_RS_R_1_1_WID                8
#define IPU_DC_R_LLA1_MCU_RS_R_2_1_WID                8
#define IPU_DC_R_LLA1_MCU_RS_R_3_1_WID                8

#define IPU_DC_STAT_DC_TRIPLE_BUF_CNT_FULL_0_WID      1
#define IPU_DC_STAT_DC_TRIPLE_BUF_CNT_EMPTY_0_WID     1
#define IPU_DC_STAT_DC_TRIPLE_BUF_DATA_FULL_0_WID     1
#define IPU_DC_STAT_DC_TRIPLE_BUF_DATA_EMPTY_0_WID    1
#define IPU_DC_STAT_DC_TRIPLE_BUF_CNT_FULL_1_WID      1
#define IPU_DC_STAT_DC_TRIPLE_BUF_CNT_EMPTY_1_WID     1
#define IPU_DC_STAT_DC_TRIPLE_BUF_DATA_FULL_1_WID     1
#define IPU_DC_STAT_DC_TRIPLE_BUF_DATA_EMPTY_1_WID    1

// DMFC registers
#define IPU_DMFC_RD_CHAN_DMFC_BURST_SIZE_0_WID        2
#define IPU_DMFC_RD_CHAN_DMFC_WM_EN_0_WID             1
#define IPU_DMFC_RD_CHAN_DMFC_WM_SET_0_WID            3
#define IPU_DMFC_RD_CHAN_DMFC_WM_CLR_0_WID            3
#define IPU_DMFC_RD_CHAN_DMFC_PPW_C_WID               2

#define IPU_DMFC_WR_CHAN_DMFC_ST_ADDR_1_WID           3
#define IPU_DMFC_WR_CHAN_DMFC_FIFO_SIZE_1_WID         3
#define IPU_DMFC_WR_CHAN_DMFC_BURST_SIZE_1_WID        2
#define IPU_DMFC_WR_CHAN_DMFC_ST_ADDR_2_WID           3
#define IPU_DMFC_WR_CHAN_DMFC_FIFO_SIZE_2_WID         3
#define IPU_DMFC_WR_CHAN_DMFC_BURST_SIZE_2_WID        2
#define IPU_DMFC_WR_CHAN_DMFC_ST_ADDR_1c_WID          3
#define IPU_DMFC_WR_CHAN_DMFC_FIFO_SIZE_1c_WID        3
#define IPU_DMFC_WR_CHAN_DMFC_BURST_SIZE_1c_WID       2
#define IPU_DMFC_WR_CHAN_DMFC_ST_ADDR_2c_WID          3
#define IPU_DMFC_WR_CHAN_DMFC_FIFO_SIZE_2c_WID        3
#define IPU_DMFC_WR_CHAN_DMFC_BURST_SIZE_2c_WID       2

#define IPU_DMFC_WR_CHAN_DEF_WM_EN_1_WID              1
#define IPU_DMFC_WR_CHAN_DEF_WM_SET_1_WID             3
#define IPU_DMFC_WR_CHAN_DEF_WM_CLR_1_WID             3
#define IPU_DMFC_WR_CHAN_DEF_WM_EN_2_WID              1
#define IPU_DMFC_WR_CHAN_DEF_WM_SET_2_WID             3
#define IPU_DMFC_WR_CHAN_DEF_WM_CLR_2_WID             3
#define IPU_DMFC_WR_CHAN_DEF_WM_EN_1c_WID             1
#define IPU_DMFC_WR_CHAN_DEF_WM_SET_1c_WID            3
#define IPU_DMFC_WR_CHAN_DEF_WM_CLR_1c_WID            3
#define IPU_DMFC_WR_CHAN_DEF_WM_EN_2c_WID             1
#define IPU_DMFC_WR_CHAN_DEF_WM_SET_2c_WID            3
#define IPU_DMFC_WR_CHAN_DEF_WM_CLR_2c_WID            3

#define IPU_DMFC_DP_CHAN_DMFC_START_ADDR_5b_WID       3
#define IPU_DMFC_DP_CHAN_DMFC_FIFO_SIZE_5b_WID        3
#define IPU_DMFC_DP_CHAN_DMFC_BURST_SIZE_5b_WID       2
#define IPU_DMFC_DP_CHAN_DMFC_START_ADDR_5f_WID       3
#define IPU_DMFC_DP_CHAN_DMFC_FIFO_SIZE_5f_WID        3
#define IPU_DMFC_DP_CHAN_DMFC_BURST_SIZE_5f_WID       2
#define IPU_DMFC_DP_CHAN_DMFC_START_ADDR_6b_WID       3
#define IPU_DMFC_DP_CHAN_DMFC_FIFO_SIZE_6b_WID        3
#define IPU_DMFC_DP_CHAN_DMFC_BURST_SIZE_6b_WID       2
#define IPU_DMFC_DP_CHAN_DMFC_START_ADDR_6f_WID       3
#define IPU_DMFC_DP_CHAN_DMFC_FIFO_SIZE_6f_WID        3
#define IPU_DMFC_DP_CHAN_DMFC_BURST_SIZE_6f_WID       2

#define IPU_DMFC_DP_CHAN_DEF_DMFC_WM_EN_5b_WID        1
#define IPU_DMFC_DP_CHAN_DEF_DMFC_WM_SET_5b_WID       3
#define IPU_DMFC_DP_CHAN_DEF_DMFC_WM_CLR_5b_WID       3
#define IPU_DMFC_DP_CHAN_DEF_DMFC_WM_EN_5f_WID        1
#define IPU_DMFC_DP_CHAN_DEF_DMFC_WM_SET_5f_WID       3
#define IPU_DMFC_DP_CHAN_DEF_DMFC_WM_CLR_5f_WID       3
#define IPU_DMFC_DP_CHAN_DEF_DMFC_WM_EN_6b_WID        1
#define IPU_DMFC_DP_CHAN_DEF_DMFC_WM_SET_6b_WID       3
#define IPU_DMFC_DP_CHAN_DEF_DMFC_WM_CLR_6b_WID       3
#define IPU_DMFC_DP_CHAN_DEF_DMFC_WM_EN_6f_WID        1
#define IPU_DMFC_DP_CHAN_DEF_DMFC_WM_SET_6f_WID       3
#define IPU_DMFC_DP_CHAN_DEF_DMFC_WM_CLR_6f_WID       3

#define IPU_DMFC_GENERAL1_DMFC_DCDP_SYNC_PR_WID       2
#define IPU_DMFC_GENERAL1_DMFC_BURST_SIZE_9_WID       2
#define IPU_DMFC_GENERAL1_DMFC_WM_EN_9_WID            1
#define IPU_DMFC_GENERAL1_DMFC_WM_SET_9_WID           3
#define IPU_DMFC_GENERAL1_DMFC_WM_CLR_9_WID           3
#define IPU_DMFC_GENERAL1_WAIT4EOT_1_WID              9
#define IPU_DMFC_GENERAL1_WAIT4EOT_2_WID              9
#define IPU_DMFC_GENERAL1_WAIT4EOT_3_WID              9
#define IPU_DMFC_GENERAL1_WAIT4EOT_4_WID              9
#define IPU_DMFC_GENERAL1_WAIT4EOT_5B_WID             9
#define IPU_DMFC_GENERAL1_WAIT4EOT_5F_WID             9
#define IPU_DMFC_GENERAL1_WAIT4EOT_6B_WID             9
#define IPU_DMFC_GENERAL1_WAIT4EOT_6F_WID             9
#define IPU_DMFC_GENERAL1_WAIT4EOT_9_WID              9

#define IPU_DMFC_GENERAL2_DMFC_FRAME_WIDTH_RD_WID     13
#define IPU_DMFC_GENERAL2_DMFC_FRAME_HEIGHT_RD_WID    13

#define IPU_DMFC_IC_CTRL_DMFC_IC_IN_PORT_WID          3
#define IPU_DMFC_IC_CTRL_DMFC_IC_PPW_C_WID            2
#define IPU_DMFC_IC_CTRL_DMFC_IC_FRAME_WIDTH_RD_WID   13
#define IPU_DMFC_IC_CTRL_DMFC_IC_FRAME_HEIGHT_RD_WID  13

#define IPU_DMFC_STAT_DMFC_FIFO_FULL_0_WID            1
#define IPU_DMFC_STAT_DMFC_FIFO_FULL_1_WID            1
#define IPU_DMFC_STAT_DMFC_FIFO_FULL_2_WID            1
#define IPU_DMFC_STAT_DMFC_FIFO_FULL_3_WID            1
#define IPU_DMFC_STAT_DMFC_FIFO_FULL_4_WID            1
#define IPU_DMFC_STAT_DMFC_FIFO_FULL_5_WID            1
#define IPU_DMFC_STAT_DMFC_FIFO_FULL_6_WID            1
#define IPU_DMFC_STAT_DMFC_FIFO_FULL_7_WID            1
#define IPU_DMFC_STAT_DMFC_FIFO_FULL_8_WID            1
#define IPU_DMFC_STAT_DMFC_FIFO_FULL_9_WID            1
#define IPU_DMFC_STAT_DMFC_FIFO_FULL_10_WID           1
#define IPU_DMFC_STAT_DMFC_FIFO_FULL_11_WID           1
#define IPU_DMFC_STAT_DMFC_FIFO_EMPTY_0_WID           1
#define IPU_DMFC_STAT_DMFC_FIFO_EMPTY_1_WID           1
#define IPU_DMFC_STAT_DMFC_FIFO_EMPTY_2_WID           1
#define IPU_DMFC_STAT_DMFC_FIFO_EMPTY_3_WID           1
#define IPU_DMFC_STAT_DMFC_FIFO_EMPTY_4_WID           1
#define IPU_DMFC_STAT_DMFC_FIFO_EMPTY_5_WID           1
#define IPU_DMFC_STAT_DMFC_FIFO_EMPTY_6_WID           1
#define IPU_DMFC_STAT_DMFC_FIFO_EMPTY_7_WID           1
#define IPU_DMFC_STAT_DMFC_FIFO_EMPTY_8_WID           1
#define IPU_DMFC_STAT_DMFC_FIFO_EMPTY_9_WID           1
#define IPU_DMFC_STAT_DMFC_FIFO_EMPTY_10_WID          1
#define IPU_DMFC_STAT_DMFC_FIFO_EMPTY_11_WID          1
#define IPU_DMFC_STAT_DMFC_IC_BUFFER_FULL_WID         1
#define IPU_DMFC_STAT_DMFC_IC_BUFFER_EMPTY_WID        1

// VDI Registers
#define IPU_VDI_FSIZE_VDI_FWIDTH_WID                  10
#define IPU_VDI_FSIZE_VDI_FHEIGHT_WID                 10

#define IPU_VDI_C_VDI_CH_422_WID                      1
#define IPU_VDI_C_VDI_MOT_SEL_WID                     2
#define IPU_VDI_C_VDI_BURST_SIZE_1_WID                4
#define IPU_VDI_C_VDI_BURST_SIZE_2_WID                4
#define IPU_VDI_C_VDI_BURST_SIZE_3_WID                4
#define IPU_VDI_C_VDI_VWMI1_SET_WID                   3
#define IPU_VDI_C_VDI_VWMI1_CLR_WID                   3
#define IPU_VDI_C_VDI_VWMI3_SET_WID                   3
#define IPU_VDI_C_VDI_VWMI3_CLR_WID                   3
#define IPU_VDI_C_VDI_TOP_FIELD_MAN_WID               1
#define IPU_VDI_C_VDI_TOP_FIELD_AUTO_WID              1

//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------
#define IPU_ENABLE                              1
#define IPU_DISABLE                             0

// IPU Common Registers

#define IPU_IPU_CONF_CSI0_EN_ENABLE             1
#define IPU_IPU_CONF_CSI0_EN_DISABLE            0

#define IPU_IPU_CONF_CSI1_EN_ENABLE             1
#define IPU_IPU_CONF_CSI1_EN_DISABLE            0

#define IPU_IPU_CONF_IC_EN_ENABLE               1
#define IPU_IPU_CONF_IC_EN_DISABLE              0

#define IPU_IPU_CONF_IRT_EN_ENABLE              1
#define IPU_IPU_CONF_IRT_EN_DISABLE             0

#define IPU_IPU_CONF_ISP_EN_ENABLE              1
#define IPU_IPU_CONF_ISP_EN_DISABLE             0

#define IPU_IPU_CONF_DP_EN_ENABLE               1
#define IPU_IPU_CONF_DP_EN_DISABLE              0

#define IPU_IPU_CONF_DI0_EN_ENABLE              1
#define IPU_IPU_CONF_DI0_EN_DISABLE             0

#define IPU_IPU_CONF_DI1_EN_ENABLE              1
#define IPU_IPU_CONF_DI1_EN_DISABLE             0

#define IPU_IPU_CONF_SMFC_EN_ENABLE             1
#define IPU_IPU_CONF_SMFC_EN_DISABLE            0

#define IPU_IPU_CONF_DC_EN_ENABLE               1
#define IPU_IPU_CONF_DC_EN_DISABLE              0

#define IPU_IPU_CONF_DMFC_EN_ENABLE             1
#define IPU_IPU_CONF_DMFC_EN_DISABLE            0

#define IPU_IPU_CONF_SISG_EN_ENABLE             1
#define IPU_IPU_CONF_SISG_EN_DISABLE            0

#define IPU_IPU_CONF_VDI_EN_ENABLE              1
#define IPU_IPU_CONF_VDI_EN_DISABLE             0

#define IPU_IPU_CONF_IPU_DIAGBUS_ON             1
#define IPU_IPU_CONF_IPU_DIAGBUS_OFF            0

#define IPU_IPU_CONF_IDMAC_DISABLE_ON           1
#define IPU_IPU_CONF_IDMAC_DISABLE_OFF          0

#define IPU_IPU_CONF_ISP_DOUBLE_FLOW            1
#define IPU_IPU_CONF_ISP_SINGLE_FLOW            0

#define IPU_IPU_CONF_IC_DMFC_SEL_DMAIC_1_TO_DMFC  1
#define IPU_IPU_CONF_IC_DMFC_SEL_DMAIC_1_TO_IDMAC 0

#define IPU_IPU_CONF_IC_DMFC_SYNC_SYNC          1
#define IPU_IPU_CONF_IC_DMFC_SYNC_ASYNC         0

#define IPU_IPU_CONF_VDI_DMFC_SYNC_DIRECT_PATH_ENABLE  1
#define IPU_IPU_CONF_VDI_DMFC_SYNC_DIRECT_PATH_DISABLE 0

#define IPU_IPU_CONF_CSI0_DATA_SOURCE_PARALLEL  0
#define IPU_IPU_CONF_CSI0_DATA_SOURCE_MIPI      1 

#define IPU_IPU_CONF_CSI1_DATA_SOURCE_PARALLEL  0
#define IPU_IPU_CONF_CSI1_DATA_SOURCE_MIPI      1

#define IPU_IPU_CONF_IC_INPUT_ISP               1
#define IPU_IPU_CONF_IC_INPUT_CSI               0


// IPU DMA Interrupt Control Registers

#define IPU_EOF_INTERRUPT_ENABLE                1
#define IPU_EOF_INTERRUPT_DISABLE               0

#define IPU_NFACK_INTERRUPT_ENABLE              1
#define IPU_NFACK_INTERRUPT_DISABLE             0

#define IPU_NFB4EOF_INTERRUPT_ENABLE            1
#define IPU_NFB4EOF_INTERRUPT_DISABLE           0

#define IPU_EOS_INTERRUPT_ENABLE                1
#define IPU_EOS_INTERRUPT_DISABLE               0

#define IPU_EOBND_INTERRUPT_ENABLE              1
#define IPU_EOBND_INTERRUPT_DISABLE             0

#define IPU_EOS_INTERRUPT_ENABLE                1
#define IPU_EOS_INTERRUPT_DISABLE               0

#define IPU_TH_INTERRUPT_ENABLE                 1
#define IPU_TH_INTERRUPT_DISABLE                0

// IPU_INT_CTRL_9 and IPU_INT_STAT_9

#define IPU_IC_BAYER_BUF_OVF_INT_ENABLE         1
#define IPU_IC_BAYER_BUF_OVF_INT_DISABLE        0

#define IPU_IC_ENC_BUF_OVF_INT_ENABLE           1
#define IPU_IC_ENC_BUF_OVF_INT_DISABLE          0

#define IPU_IC_VF_BUF_OVF_INT_ENABLE            1
#define IPU_IC_VF_BUF_OVF_INT_DISABLE           0

#define IPU_ISP_PUPE_INT_ENABLE                 1
#define IPU_ISP_PUPE_INT_DISABLE                0

#define IPU_CSI0_PUPE_INT_ENABLE                1
#define IPU_CSI0_PUPE_INT_DISABLE               0

#define IPU_CSI1_PUPE_INT_ENABLE                1
#define IPU_CSI1_PUPE_INT_DISABLE               0

// IPU_INT_CTRL_15 & IPU_INT_STAT_15

#define IPU_SNOOPING1_INT_ENABLE                1
#define IPU_SNOOPING1_INT_DISABLE               0

#define IPU_SNOOPING2_INT_ENABLE                1
#define IPU_SNOOPING2_INT_DISABLE               0

#define IPU_DP_SF_START_INT_ENABLE              1
#define IPU_DP_SF_START_INT_DISABLE             0

#define IPU_DP_SF_END_INT_ENABLE                1
#define IPU_DP_SF_END_INT_DISABLE               0

#define IPU_DP_ASF_START_INT_ENABLE             1
#define IPU_DP_ASF_START_INT_DISABLE            0

#define IPU_DP_ASF_END_INT_ENABLE               1
#define IPU_DP_ASF_END_INT_DISABLE              0

#define IPU_DP_SF_BRAKE_INT_ENABLE              1
#define IPU_DP_SF_BRAKE_INT_DISABLE             0

#define IPU_DP_ASF_BRAKE_INT_ENABLE             1
#define IPU_DP_ASF_BRAKE_INT_DISABLE            0

#define IPU_DP_FC_INT_ENABLE                    1
#define IPU_DP_FC_INT_DISABLE                   0

#define IPU_DI_VSYNC_PRE_INT_ENABLE             1
#define IPU_DI_VSYNC_PRE_INT_DISABLE            0

#define IPU_DC_DP_START_INT_ENABLE              1
#define IPU_DC_DP_START_INT_DISABLE             0

// IPU SDMA Interrupt Control Registers

#define IPU_EOF_SDMA_INTERRUPT_ENABLE           1
#define IPU_EOF_SDMA_INTERRUPT_DISABLE          0

#define IPU_NFACK_SDMA_INTERRUPT_ENABLE         1
#define IPU_NFACK_SDMA_INTERRUPT_DISABLE        0

#define IPU_NFB4EOF_SDMA_INTERRUPT_ENABLE       1
#define IPU_NFB4EOF_SDMA_INTERRUPT_DISABLE      0

#define IPU_EOS_SDMA_INTERRUPT_ENABLE           1
#define IPU_EOS_SDMA_INTERRUPT_DISABLE          0

#define IPU_EOBND_SDMA_INTERRUPT_ENABLE         1
#define IPU_EOBND_SDMA_INTERRUPT_DISABLE        0

#define IPU_EOS_SDMA_INTERRUPT_ENABLE           1
#define IPU_EOS_SDMA_INTERRUPT_DISABLE          0

#define IPU_TH_SDMA_INTERRUPT_ENABLE            1
#define IPU_TH_SDMA_INTERRUPT_DISABLE           0

// IPU_SRM_PRI1

#define IPU_IPU_SRM_PRI1_SRM_MODE_MCU_ACCESS_RAM                 0
#define IPU_IPU_SRM_PRI1_SRM_MODE_FSU_CONTROL_UPDATE_NEXT_FRAME  1
#define IPU_IPU_SRM_PRI1_SRM_MODE_FSU_CONTROL_SWAP_CONTINUOUSLY  2
#define IPU_IPU_SRM_PRI1_SRM_MODE_MCU_CONTROL_UPDATE_NOW         3

// IPU_SRM_PRI2

#define IPU_IPU_SRM_PRI2_SRM_MODE_MCU_ACCESS_RAM                 0
#define IPU_IPU_SRM_PRI2_SRM_MODE_FSU_CONTROL_UPDATE_NEXT_FRAME  1
#define IPU_IPU_SRM_PRI2_SRM_MODE_FSU_CONTROL_SWAP_CONTINUOUSLY  2
#define IPU_IPU_SRM_PRI2_SRM_MODE_MCU_CONTROL_UPDATE_NOW         3

#define IPU_IPU_SRM_PRI2_DI0_SRM_MCU_USE_MCU_UPDATING            1
#define IPU_IPU_SRM_PRI2_DI0_SRM_MCU_USE_MCU_NOT_UPDATING        0

// IPU_FS_PROC_FLOW1
#define IPU_IPU_FS_PROC_FLOW1_SRC_SEL_MCU             0
#define IPU_IPU_FS_PROC_FLOW1_SRC_SEL_SMFC0           1
#define IPU_IPU_FS_PROC_FLOW1_SRC_SEL_SMFC1           2
#define IPU_IPU_FS_PROC_FLOW1_SRC_SEL_SMFC2           3
#define IPU_IPU_FS_PROC_FLOW1_SRC_SEL_SMFC3           4
#define IPU_IPU_FS_PROC_FLOW1_SRC_SEL_IC_DIRECT       5
#define IPU_IPU_FS_PROC_FLOW1_SRC_SEL_PP              5
#define IPU_IPU_FS_PROC_FLOW1_SRC_SEL_ROT_FOR_PP      6
#define IPU_IPU_FS_PROC_FLOW1_SRC_SEL_IRT_ENCODING    6
#define IPU_IPU_FS_PROC_FLOW1_SRC_SEL_ENCODING        7
#define IPU_IPU_FS_PROC_FLOW1_SRC_SEL_IRT_VIEWFINDER  7
#define IPU_IPU_FS_PROC_FLOW1_SRC_SEL_ISP_MAIN        8
#define IPU_IPU_FS_PROC_FLOW1_SRC_SEL_VIEWFINDER      8
#define IPU_IPU_FS_PROC_FLOW1_SRC_SEL_ISP_ALT         9
#define IPU_IPU_FS_PROC_FLOW1_SRC_SEL_AUTOREF         11
#define IPU_IPU_FS_PROC_FLOW1_SRC_SEL_SNOOP1          12
#define IPU_IPU_FS_PROC_FLOW1_SRC_SEL_SNOOP2          13
#define IPU_IPU_FS_PROC_FLOW1_SRC_SEL_AUTOREF_SNOOP1  14
#define IPU_IPU_FS_PROC_FLOW1_SRC_SEL_AUTOREF_SNOOP2  15

#define IPU_IPU_FS_PROC_FLOW1_VDI_SRC_SEL_MCU         0
#define IPU_IPU_FS_PROC_FLOW1_VDI_SRC_SEL_CSI         1

// IPU_FS_PROC_FLOW2
#define IPU_IPU_FS_PROC_FLOW2_DEST_SEL_MCU               0
#define IPU_IPU_FS_PROC_FLOW2_DEST_SEL_IRT_VIEWFINDER    1
#define IPU_IPU_FS_PROC_FLOW2_DEST_SEL_IRT_ENCODING      1
#define IPU_IPU_FS_PROC_FLOW2_DEST_SEL_IRT_PLAYBACK      3
#define IPU_IPU_FS_PROC_FLOW2_DEST_SEL_IC_PLAYBACK_PP    4
#define IPU_IPU_FS_PROC_FLOW2_DEST_SEL_IC_PRP            5
#define IPU_IPU_FS_PROC_FLOW2_DEST_SEL_DC1               7
#define IPU_IPU_FS_PROC_FLOW2_DEST_SEL_DC2               8
#define IPU_IPU_FS_PROC_FLOW2_DEST_SEL_DP_SYNC0          9
#define IPU_IPU_FS_PROC_FLOW2_DEST_SEL_DP_SYNC1          10
#define IPU_IPU_FS_PROC_FLOW2_DEST_SEL_DP_ASYNC1         11
#define IPU_IPU_FS_PROC_FLOW2_DEST_SEL_DP_ASYNC0         12
#define IPU_IPU_FS_PROC_FLOW2_DEST_SEL_ALT_DC2           13
#define IPU_IPU_FS_PROC_FLOW2_DEST_SEL_ALT_DP_ASYNC1     14
#define IPU_IPU_FS_PROC_FLOW2_DEST_SEL_ALT_DP_ASYNC0     15

#define IPU_IPU_FS_PROC_FLOW2_PRP_DEST_SEL_MCU           0
#define IPU_IPU_FS_PROC_FLOW2_PRP_DEST_SEL_IC_INPUT      1
#define IPU_IPU_FS_PROC_FLOW2_PRP_DEST_SEL_PP            2

#define IPU_IPU_FS_PROC_FLOW2_PRP_ALT_DEST_SEL_MCU       0
#define IPU_IPU_FS_PROC_FLOW2_PRP_ALT_DEST_SEL_IC_INPUT  1
#define IPU_IPU_FS_PROC_FLOW2_PRP_ALT_DEST_SEL_PP        2

// IPU_FS_PROC_FLOW3
#define IPU_IPU_FS_PROC_FLOW3_DEST_SEL_MCU               0
#define IPU_IPU_FS_PROC_FLOW3_DEST_SEL_IRT_ENCODING      1
#define IPU_IPU_FS_PROC_FLOW3_DEST_SEL_IRT_VIEWFINDER    2
#define IPU_IPU_FS_PROC_FLOW3_DEST_SEL_IRT_PLAYBACK      3
#define IPU_IPU_FS_PROC_FLOW3_DEST_SEL_IC_PLAYBACK_PP    4
#define IPU_IPU_FS_PROC_FLOW3_DEST_SEL_IC_PRP            5
#define IPU_IPU_FS_PROC_FLOW3_DEST_SEL_ISP               6
#define IPU_IPU_FS_PROC_FLOW3_DEST_SEL_DC1               7
#define IPU_IPU_FS_PROC_FLOW3_DEST_SEL_DC2               8
#define IPU_IPU_FS_PROC_FLOW3_DEST_SEL_DP_SYNC0          9
#define IPU_IPU_FS_PROC_FLOW3_DEST_SEL_DP_SYNC1          10
#define IPU_IPU_FS_PROC_FLOW3_DEST_SEL_DP_ASYNC0         11
#define IPU_IPU_FS_PROC_FLOW3_DEST_SEL_DP_ASYNC1         12
#define IPU_IPU_FS_PROC_FLOW3_DEST_SEL_ALT_DC2           13
#define IPU_IPU_FS_PROC_FLOW3_DEST_SEL_ALT_DP_ASYNC1     14
#define IPU_IPU_FS_PROC_FLOW3_DEST_SEL_ALT_DP_ASYNC0     15

// IPU_FS_DISP_FLOW1
#define IPU_IPU_FS_DISP_FLOW1_SRC_SEL_IC_MCU             0
#define IPU_IPU_FS_DISP_FLOW1_SRC_SEL_IC_ENCODING        3
#define IPU_IPU_FS_DISP_FLOW1_SRC_SEL_IC_VIEWFINDER      4
#define IPU_IPU_FS_DISP_FLOW1_SRC_SEL_IC_PLAYBACK        5
#define IPU_IPU_FS_DISP_FLOW1_SRC_SEL_IRT_ENCODING       6
#define IPU_IPU_FS_DISP_FLOW1_SRC_SEL_IRT_VIEWFINDER     7
#define IPU_IPU_FS_DISP_FLOW1_SRC_SEL_IRT_PLAYBACK       8
#define IPU_IPU_FS_DISP_FLOW1_SRC_SEL_AUTOREF            11
#define IPU_IPU_FS_DISP_FLOW1_SRC_SEL_SNOOP1             12
#define IPU_IPU_FS_DISP_FLOW1_SRC_SEL_SNOOP2             13
#define IPU_IPU_FS_DISP_FLOW1_SRC_SEL_AUTOREF_SNOOP1     14
#define IPU_IPU_FS_DISP_FLOW1_SRC_SEL_AUTOREF_SNOOP2     15

// IPU_FS_DISP_FLOW2
#define IPU_IPU_FS_DISP_FLOW2_SRC_SEL_IC_MCU             0
#define IPU_IPU_FS_DISP_FLOW2_SRC_SEL_IC_ENCODING        3
#define IPU_IPU_FS_DISP_FLOW2_SRC_SEL_IC_VIEWFINDER      4
#define IPU_IPU_FS_DISP_FLOW2_SRC_SEL_IC_PLAYBACK        5
#define IPU_IPU_FS_DISP_FLOW2_SRC_SEL_IRT_ENCODING       6
#define IPU_IPU_FS_DISP_FLOW2_SRC_SEL_IRT_VIEWFINDER     7
#define IPU_IPU_FS_DISP_FLOW2_SRC_SEL_IRT_PLAYBACK       8
#define IPU_IPU_FS_DISP_FLOW2_SRC_SEL_AUTOREF            11
#define IPU_IPU_FS_DISP_FLOW2_SRC_SEL_SNOOP1             12
#define IPU_IPU_FS_DISP_FLOW2_SRC_SEL_SNOOP2             13
#define IPU_IPU_FS_DISP_FLOW2_SRC_SEL_AUTOREF_SNOOP1     14
#define IPU_IPU_FS_DISP_FLOW2_SRC_SEL_AUTOREF_SNOOP2     15

// IPU_SKIP
#define IPU_IPU_SKIP_MAX_RATIO_SKIP_DISABLE              0

// IPU_DISP_GEN
#define IPU_IPU_DISP_GEN_DI0_DUAL_MODE_ENABLE            1
#define IPU_IPU_DISP_GEN_DI0_DUAL_MODE_DISABLE           0

#define IPU_IPU_DISP_GEN_DI1_DUAL_MODE_ENABLE            1
#define IPU_IPU_DISP_GEN_DI1_DUAL_MODE_DISABLE           0

#define IPU_IPU_DISP_GEN_DC2_DOUBLE_FLOW                 1
#define IPU_IPU_DISP_GEN_DC2_SINGLE_FLOW                 0

#define IPU_IPU_DISP_GEN_DP_ASYNC_DOUBLE_FLOW            1
#define IPU_IPU_DISP_GEN_DP_ASYNC_SINGLE_FLOW            0

#define IPU_IPU_DISP_GEN_DP_FG_EN_ASYNC0_ENABLE          1
#define IPU_IPU_DISP_GEN_DP_FG_EN_ASYNC0_DISABLE         0

#define IPU_IPU_DISP_GEN_DP_FG_EN_ASYNC1_ENABLE          1
#define IPU_IPU_DISP_GEN_DP_FG_EN_ASYNC1_DISABLE         0

#define IPU_IPU_DISP_GEN_DP_PIPE_CLR_CLEAR               1

#define IPU_IPU_DISP_GEN_MCU_DI_ID_8_DI1                 1
#define IPU_IPU_DISP_GEN_MCU_DI_ID_8_DI0                 0

#define IPU_IPU_DISP_GEN_MCU_DI_ID_9_DI1                 1
#define IPU_IPU_DISP_GEN_MCU_DI_ID_9_DI0                 0

#define IPU_IPU_DISP_GEN_MCU_MAX_BURST_STOP_8BEAT        1
#define IPU_IPU_DISP_GEN_MCU_MAX_BURST_STOP_UNLIMITED    0

#define IPU_IPU_DISP_GEN_DI0_COUNTER_RELEASE_START       1
#define IPU_IPU_DISP_GEN_DI0_COUNTER_RELEASE_STOP        0

#define IPU_IPU_DISP_GEN_DI1_COUNTER_RELEASE_START       1
#define IPU_IPU_DISP_GEN_DI1_COUNTER_RELEASE_STOP        0

#define IPU_IPU_MEM_RST_RST_MEM_START_RESET_MEMORY       1

#define IPU_IPU_PM_DI0_SRM_CLOCK_CHANGE_MODE_ENABLE      1
#define IPU_IPU_PM_DI0_SRM_CLOCK_CHANGE_MODE_DISABLE     0

#define IPU_IPU_PM_DI1_SRM_CLOCK_CHANGE_MODE_ENABLE      1
#define IPU_IPU_PM_DI1_SRM_CLOCK_CHANGE_MODE_DISABLE     0

#define IPU_IPU_PM_CLOCK_MODE_STAT_1                     1
#define IPU_IPU_PM_CLOCK_MODE_STAT_0                     0

#define IPU_IPU_PM_LPSR_MODE_NEXT_MODE_LPSR              1
#define IPU_IPU_PM_LPSR_MODE_NEXT_MODE_NOT_LPSR          0

#define IPU_IPU_CUR_BUF_0                                0
#define IPU_IPU_CUR_BUF_1                                1

#define IPU_IPU_CH_BUF_READY                             1
#define IPU_IPU_CH_BUF_NOT_READY                         0

#define IPU_IPU_CH_DB_MODE_ACTIVE                        1
#define IPU_IPU_CH_DB_MODE_NOT_ACTIVE                    0

// IDMAC register values
#define IPU_IDMAC_CONF_WIDPT_1                        0
#define IPU_IDMAC_CONF_WIDPT_2                        1
#define IPU_IDMAC_CONF_WIDPT_3                        2
#define IPU_IDMAC_CONF_WIDPT_4                        3

#define IPU_IDMAC_CONF_P_ENDIAN_LITTLE                0
#define IPU_IDMAC_CONF_P_ENDIAN_BIG                   1

#define IPU_IDMAC_CH_ENABLE                           1
#define IPU_IDMAC_CH_DISABLE                          0

#define IPU_IDMAC_SEP_AL_SEPARATE                     1
#define IPU_IDMAC_SEP_AL_NOT_SEPARATE                 0

#define IPU_IDMAC_CH_PRI_HIGH                         1
#define IPU_IDMAC_CH_PRI_LOW                          0

#define IPU_IDMAC_WM_ENABLE                           1
#define IPU_IDMAC_WM_DISABLE                          0

#define IPU_IDMAC_LOCK_ENABLE                         1
#define IPU_IDMAC_LOCK_DISABLE                        0

#define IPU_IDMAC_BNDM_ENABLE                         1
#define IPU_IDMAC_BNDM_DISABLE                        0

// DP register values
#define IPU_DP_COM_CONF_DP_FG_EN_DISABLE              0
#define IPU_DP_COM_CONF_DP_FG_EN_ENABLE               1

#define IPU_DP_COM_CONF_DP_GWSEL_FULL_PLANE           0
#define IPU_DP_COM_CONF_DP_GWSEL_PARTIAL_PLANE        1

#define IPU_DP_COM_CONF_DP_GWAM_GLOBAL                1
#define IPU_DP_COM_CONF_DP_GWAM_LOCAL                 0

#define IPU_DP_COM_CONF_DP_GWCKE_ENABLE               1
#define IPU_DP_COM_CONF_DP_GWCKE_DISABLE              0

#define IPU_DP_COM_CONF_DP_COC_DISABLE_CURSOR         0
#define IPU_DP_COM_CONF_DP_COC_FULL_CURSOR            1
#define IPU_DP_COM_CONF_DP_COC_REVERSED_CURSOR        2
#define IPU_DP_COM_CONF_DP_COC_AND_WITH_FULL_PLANE    3
#define IPU_DP_COM_CONF_DP_COC_OR_WITH_FULL_PLANE     5
#define IPU_DP_COM_CONF_DP_COC_XOR_WITH_FULL_PLANE    6

#define IPU_DP_COM_CONF_DP_CSC_DEF_SYNC_CSC_DISABLE   0
#define IPU_DP_COM_CONF_DP_CSC_DEF_SYNC_CSC_ENABLE_AFTER_COMBINING     1
#define IPU_DP_COM_CONF_DP_CSC_DEF_SYNC_CSC_ENABLE_BEFORE_COMBINING_BG 2
#define IPU_DP_COM_CONF_DP_CSC_DEF_SYNC_CSC_ENABLE_BEFORE_COMBINING_FG 3

#define IPU_DP_COM_CONF_DP_CSC_GAMUT_SAT_EN_ENABLE    1
#define IPU_DP_COM_CONF_DP_CSC_GAMUT_SAT_EN_DISABLE   0

#define IPU_DP_COM_CONF_DP_CSC_YUV_SAT_MODE_FULL_RANGE 0
#define IPU_DP_COM_CONF_DP_CSC_YUV_SAT_MODE_SATURATED  1

#define IPU_DP_COM_CONF_DP_GAMMA_EN_ENABLE            1
#define IPU_DP_COM_CONF_DP_GAMMA_EN_DISABLE           0

#define IPU_DP_COM_CONF_DP_GAMMA_YUV_EN_ENABLE        1
#define IPU_DP_COM_CONF_DP_GAMMA_YUV_EN_DISABLE       0

#define IPU_DP_CSC_DP_CSC_SCALE_FACTOR_2              0
#define IPU_DP_CSC_DP_CSC_SCALE_FACTOR_1              1
#define IPU_DP_CSC_DP_CSC_SCALE_FACTOR_0              2
#define IPU_DP_CSC_DP_CSC_SCALE_FACTOR_NEG_1          3


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

#define IPU_IC_IDMAC_1_CB_BURST_16              1
#define IPU_IC_IDMAC_1_CB_BURST_8               0

#define IPU_IC_IDMAC_1_T1_ROT_90                1
#define IPU_IC_IDMAC_1_T1_ROT_0                 0

#define IPU_IC_IDMAC_1_T1_FLIP_LR_FLIP_ENABLE   1
#define IPU_IC_IDMAC_1_T1_FLIP_LR_FLIP_DISABLE  0

#define IPU_IC_IDMAC_1_T1_FLIP_UD_FLIP_ENABLE   1
#define IPU_IC_IDMAC_1_T1_FLIP_UD_FLIP_DISABLE  0

#define IPU_IC_IDMAC_1_T2_ROT_90                1
#define IPU_IC_IDMAC_1_T2_ROT_0                 0

#define IPU_IC_IDMAC_1_T2_FLIP_LR_FLIP_ENABLE   1
#define IPU_IC_IDMAC_1_T2_FLIP_LR_FLIP_DISABLE  0

#define IPU_IC_IDMAC_1_T2_FLIP_UD_FLIP_ENABLE   1
#define IPU_IC_IDMAC_1_T2_FLIP_UD_FLIP_DISABLE  0

#define IPU_IC_IDMAC_1_T3_ROT_90                1
#define IPU_IC_IDMAC_1_T3_ROT_0                 0

#define IPU_IC_IDMAC_1_T3_FLIP_LR_FLIP_ENABLE   1
#define IPU_IC_IDMAC_1_T3_FLIP_LR_FLIP_DISABLE  0

#define IPU_IC_IDMAC_1_T3_FLIP_UD_FLIP_ENABLE   1
#define IPU_IC_IDMAC_1_T3_FLIP_UD_FLIP_DISABLE  0


// IPU CSI register

// CSI0_SENS_CONF,CSI1_SENS_CONF
#define IPU_CSI_SENS_CONF_VSYNC_POL_INVERT      1
#define IPU_CSI_SENS_CONF_VSYNC_POL_NO_INVERT   0

#define IPU_CSI_SENS_CONF_HSYNC_POL_INVERT      1
#define IPU_CSI_SENS_CONF_HSYNC_POL_NO_INVERT   0

#define IPU_CSI_SENS_CONF_DATA_POL_INVERT       1
#define IPU_CSI_SENS_CONF_DATA_POL_NO_INVERT    0

#define IPU_CSI_SENS_CONF_DISP_PIX_CLK_INVERT     1
#define IPU_CSI_SENS_CONF_DISP_PIX_CLK_NO_INVERT  0

#define IPU_CSI_SENS_CONF_DATA_EN_POL_INVERT       1
#define IPU_CSI_SENS_CONF_DATA_EN_POL_NO_INVERT    0

#define IPU_CSI_SENS_CONF_SENS_PRTCL_GATED_CLOCK_MODE                 0
#define IPU_CSI_SENS_CONF_SENS_PRTCL_NONGATED_CLOCK_MODE              1
#define IPU_CSI_SENS_CONF_SENS_PRTCL_CCIR_PROGRESSIVE_BT656_MODE      2
#define IPU_CSI_SENS_CONF_SENS_PRTCL_CCIR_INTERLACE_BT656_MODE        3
#define IPU_CSI_SENS_CONF_SENS_PRTCL_CCIR_PROGRESSIVE_BT1120DDR_MODE  4
#define IPU_CSI_SENS_CONF_SENS_PRTCL_CCIR_PROGRESSIVE_BT1120SDR_MODE  5
#define IPU_CSI_SENS_CONF_SENS_PRTCL_CCIR_INTERLACE_BT1120DDR_MODE    6
#define IPU_CSI_SENS_CONF_SENS_PRTCL_CCIR_INTERLACE_BT1120SDR_MODE    7

#define IPU_CSI_SENS_CONF_PACK_TIGHT_32bit_WORD    1
#define IPU_CSI_SENS_CONF_PACK_TIGHT_16bit_WORD    0

#define IPU_CSI_SENS_CONF_DATA_FORMAT_RGB_YUV444    0
#define IPU_CSI_SENS_CONF_DATA_FORMAT_YUV422_YUYV   1
#define IPU_CSI_SENS_CONF_DATA_FORMAT_YUV422_UYVY   2 
#define IPU_CSI_SENS_CONF_DATA_FORMAT_BAYER         3 
#define IPU_CSI_SENS_CONF_DATA_FORMAT_RGB565        4
#define IPU_CSI_SENS_CONF_DATA_FORMAT_RGB555        5  
#define IPU_CSI_SENS_CONF_DATA_FORMAT_RGB444        6 
#define IPU_CSI_SENS_CONF_DATA_FORMAT_JPEG          7

#define IPU_CSI_SENS_CONF_DATA_WIDTH_4BIT       0
#define IPU_CSI_SENS_CONF_DATA_WIDTH_8BIT       1
#define IPU_CSI_SENS_CONF_DATA_WIDTH_9BIT       2
#define IPU_CSI_SENS_CONF_DATA_WIDTH_10BIT      3
#define IPU_CSI_SENS_CONF_DATA_WIDTH_11BIT      4
#define IPU_CSI_SENS_CONF_DATA_WIDTH_12BIT      5
#define IPU_CSI_SENS_CONF_DATA_WIDTH_13BIT      6
#define IPU_CSI_SENS_CONF_DATA_WIDTH_14BIT      7
#define IPU_CSI_SENS_CONF_DATA_WIDTH_15BIT      8
#define IPU_CSI_SENS_CONF_DATA_WIDTH_16BIT      9

#define IPU_CSI_SENS_CONF_EXT_VSYNC_EXTERNAL    1
#define IPU_CSI_SENS_CONF_EXT_VSYNC_INTERNAL    0

#define IPU_CSI_SENS_CONF_DATA_DEST_ISP         0
#define IPU_CSI_SENS_CONF_DATA_DEST_IC          2
#define IPU_CSI_SENS_CONF_DATA_DEST_SMFC        4

#define IPU_CSI_SENS_CONF_JPEG8_EN_ENABLE       1  
#define IPU_CSI_SENS_CONF_JPEG8_EN_DISABLE      0

#define IPU_CSI_SENS_CONF_JPEG_HSYNC_MODE       1
#define IPU_CSI_SENS_CONF_JPEG_VSYNC_MODE       0

#define IPU_CSI_SENS_CONF_FORCE_EOF_DISABLE     0             
#define IPU_CSI_SENS_CONF_FORCE_EOF_ENABLE      1

//CSI0_OUT_FRM_CTRL,CSI1_OUT_FRM_CTRL
#define IPU_CSI_OUT_FRM_CTRL_VERT_DWNS_ENABLE    1
#define IPU_CSI_OUT_FRM_CTRL_VERT_DWNS_DISABLE   0

#define IPU_CSI_OUT_FRM_CTRL_HORZ_DWNS_ENABLE    1
#define IPU_CSI_OUT_FRM_CTRL_HORZ_DWNS_DISABLE   0

//CSI0_TST_CTRL,CSI1_TST_CTRL
#define IPU_CSI_TST_CTRL_TEST_GEN_MODE_ACTIVE    1
#define IPU_CSI_TST_CTRL_TEST_GEN_MODE_INACTIVE  0

//CSI0_CCIR_CODE_1,CSI1_CCIR_CODE_1
#define IPU_CSI_CCIR_CODE_1_CCIR_ERR_DET_EN_ENABLE     1
#define IPU_CSI_CCIR_CODE_1_CCIR_ERR_DET_EN_DISABLE    0

//CSI0_SKIP
#define IPU_CSI_SKIP_ID_00    0
#define IPU_CSI_SKIP_ID_01    1
#define IPU_CSI_SKIP_ID_10    2
#define IPU_CSI_SKIP_ID_11    3

//CSI0_CPD_CTRL
#define IPU_CSI_CPD_CTRL_COMPANDER_ISP     0
#define IPU_CSI_CPD_CTRL_COMPANDER_IC      1
#define IPU_CSI_CPD_CTRL_COMPANDER_SMFC    2

#define IPU_CSI_CPD_CTRL_RED_ROW_BEGIN_GBGB    0
#define IPU_CSI_CPD_CTRL_RED_ROW_BEGIN_GRGR    1

#define IPU_CSI_CPD_CTRL_GREEN_P_BEGIN_BLUE_RED    0
#define IPU_CSI_CPD_CTRL_GREEN_P_BEGIN_GREEN       1

// IPU CSI Registers:width
#define IPU_CSI_SENS_CONF_VSYNC_POL_WID        1
#define IPU_CSI_SENS_CONF_HSYNC_POL_WID         1         
#define IPU_CSI_SENS_CONF_DATA_POL_WID  1
#define IPU_CSI_SENS_CONF_DISP_PIX_CLK_POL_WID  1
#define IPU_CSI_SENS_CONF_SENS_PRTCL_WID 3
#define IPU_CSI_SENS_CONF_PACK_TIGHT_WID 1
#define IPU_CSI_SENS_CONF_DATA_FORMAT_WID 3
#define IPU_CSI_SENS_CONF_DATA_WIDTH_WID 4
#define IPU_CSI_SENS_CONF_EXT_VSYNC_WID 1
#define IPU_CSI_SENS_CONF_DIV_RATIO_WID 8
#define IPU_CSI_SENS_CONF_DATA_DEST_WID 3
#define IPU_CSI_SENS_CONF_JPEG8_EN_WID   1
#define IPU_CSI_SENS_CONF_JPEG_MODE_WID  1
#define IPU_CSI_SENS_CONF_FORCE_EOF_WID  1
#define IPU_CSI_SENS_CONF_DATA_EN_POL_WID      1

#define IPU_CSI_SENS_FRM_SIZE_SENS_FRM_WIDTH_WID  13
#define IPU_CSI_SENS_FRM_SIZE_SENS_FRM_HEIGHT_WID 12

#define IPU_CSI_ACT_FRM_SIZE_ACT_FRM_WIDTH_WID  13
#define IPU_CSI_ACT_FRM_SIZE_ACT_FRM_HEIGHT_WID 12

#define IPU_CSI_OUT_FRM_CTRL_VSC_WID            12
#define IPU_CSI_OUT_FRM_CTRL_HSC_WID            13
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

#define IPU_CSI_CCIR_CODE_3_CCIR_PRECOM_WID         30

#define IPU_CSI_DI_MIPI_DI0_WID 8
#define IPU_CSI_DI_MIPI_DI1_WID 8
#define IPU_CSI_DI_MIPI_DI2_WID 8
#define IPU_CSI_DI_MIPI_DI3_WID 8

#define IPU_CSI_SKIP_MAX_RATIO_SKIP_SMFC_WID 3
#define IPU_CSI_SKIP_SKIP_SMFC_WID 5
#define IPU_CSI_SKIP_ID_2_SKIP_WID 2
#define IPU_CSI_SKIP_RATIO_SKIP_ISP_WID 3
#define IPU_CSI_SKIP_SKIP_ISP_WID   5

#define IPU_CPD_CTRL_GREEN_P_BEGIN_WID  1
#define IPU_CPD_CTRL_RED_ROW_BEGIN_WID  1
#define IPU_CPD_CTRL_CPD_WID            3

#define IPU_CSI_CPD_OFFSET1_CPD_GR_OFFSET_WID 10
#define IPU_CSI_CPD_OFFSET1_CPD_GB_OFFSET_WID 10
#define IPU_CSI_CPD_OFFSET1_CPD_B_OFFSET_WID      10

#define IPU_CSI_CPD_OFFSET2_CPD_R_OFFSET_WID      10

// IPU CSI Registers:left shit
#define IPU_CSI_SENS_CONF_VSYNC_POL_LSH        0
#define IPU_CSI_SENS_CONF_HSYNC_POL_LSH        1         
#define IPU_CSI_SENS_CONF_DATA_POL_LSH  2
#define IPU_CSI_SENS_CONF_DISP_PIX_CLK_POL_LSH  3
#define IPU_CSI_SENS_CONF_SENS_PRTCL_LSH 4
#define IPU_CSI_SENS_CONF_PACK_TIGHT_LSH 7
#define IPU_CSI_SENS_CONF_DATA_FORMAT_LSH 8
#define IPU_CSI_SENS_CONF_DATA_WIDTH_LSH 11
#define IPU_CSI_SENS_CONF_EXT_VSYNC_LSH 15
#define IPU_CSI_SENS_CONF_DIV_RATIO_LSH 16
#define IPU_CSI_SENS_CONF_DATA_DEST_LSH 24
#define IPU_CSI_SENS_CONF_JPEG8_EN_LSH  27
#define IPU_CSI_SENS_CONF_JPEG_MODE_LSH 28
#define IPU_CSI_SENS_CONF_FORCE_EOF_LSH 29
#define IPU_CSI_SENS_CONF_DATA_EN_POL_LSH 31

#define IPU_CSI_SENS_FRM_SIZE_SENS_FRM_WIDTH_LSH  0
#define IPU_CSI_SENS_FRM_SIZE_SENS_FRM_HEIGHT_LSH 16

#define IPU_CSI_ACT_FRM_SIZE_ACT_FRM_WIDTH_LSH  0
#define IPU_CSI_ACT_FRM_SIZE_ACT_FRM_HEIGHT_LSH 16

#define IPU_CSI_OUT_FRM_CTRL_VSC_LSH            0
#define IPU_CSI_OUT_FRM_CTRL_HSC_LSH            16
#define IPU_CSI_OUT_FRM_CTRL_VERT_DWNS_LSH      30
#define IPU_CSI_OUT_FRM_CTRL_HORZ_DWNS_LSH      31

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

#define IPU_CSI_DI_MIPI_DI0_LSH 0
#define IPU_CSI_DI_MIPI_DI1_LSH 8
#define IPU_CSI_DI_MIPI_DI2_LSH 16
#define IPU_CSI_DI_MIPI_DI3_LSH 24

#define IPU_CSI_SKIP_MAX_RATIO_SKIP_SMFC_LSH 0
#define IPU_CSI_SKIP_SKIP_SMFC_LSH 3
#define IPU_CSI_SKIP_ID_2_SKIP_LSH 8
#define IPU_CSI_SKIP_RATIO_SKIP_ISP_LSH 16
#define IPU_CSI_SKIP_SKIP_ISP_LSH   19

#define IPU_CPD_CTRL_GREEN_P_BEGIN_LSH  0
#define IPU_CPD_CTRL_RED_ROW_BEGIN_LSH  1
#define IPU_CPD_CTRL_CPD_LSH            2

#define IPU_CSI_CPD_OFFSET1_CPD_GR_OFFSET_LSH 0
#define IPU_CSI_CPD_OFFSET1_CPD_GB_OFFSET_LSH 10
#define IPU_CSI_CPD_OFFSET1_CPD_B_OFFSET_LSH      20

#define IPU_CSI_CPD_OFFSET2_CPD_R_OFFSET_LSH      0


// IPU DI0 Registers
#define IPU_DI_GENERAL_DI_POLARITY_ACTIVE_HIGH        1
#define IPU_DI_GENERAL_DI_POLARITY_ACTIVE_LOW         0

#define IPU_DI_GENERAL_DI_ERR_TREATMENT_WAIT          1
#define IPU_DI_GENERAL_DI_ERR_TREATMENT_DRIVE_LAST_COMPONENT 0

#define IPU_DI_GENERAL_DI_WATCHDOG_MODE_4_CYCLES      0
#define IPU_DI_GENERAL_DI_WATCHDOG_MODE_16_CYCLES     1
#define IPU_DI_GENERAL_DI_WATCHDOG_MODE_64_CYCLES     2
#define IPU_DI_GENERAL_DI_WATCHDOG_MODE_128_CYCLES    3

#define IPU_DI_GENERAL_DI_CLK_EXT_EXTERNAL            1
#define IPU_DI_GENERAL_DI_CLK_EXT_INTERNAL            0

#define IPU_DI_GENERAL_DI_VSYNC_EXT_EXTERNAL          1
#define IPU_DI_GENERAL_DI_VSYNC_EXT_INTERNAL          0

#define IPU_DI_GENERAL_DI_MASK_SEL_MASK               1
#define IPU_DI_GENERAL_DI_MASK_SEL_COUNTER_2          0

#define IPU_DI_GENERAL_DI_DISP_CLOCK_INIT_RUNNING     1
#define IPU_DI_GENERAL_DI_DISP_CLOCK_INIT_STOPPED     0

#define IPU_DI_GENERAL_DI_CLOCK_STOP_MODE_NEXT_EDGE   0
#define IPU_DI_GENERAL_DI_CLOCK_STOP_MODE_COUNTER1    1
#define IPU_DI_GENERAL_DI_CLOCK_STOP_MODE_COUNTER2    2
#define IPU_DI_GENERAL_DI_CLOCK_STOP_MODE_COUNTER3    3
#define IPU_DI_GENERAL_DI_CLOCK_STOP_MODE_COUNTER4    4
#define IPU_DI_GENERAL_DI_CLOCK_STOP_MODE_COUNTER5    5
#define IPU_DI_GENERAL_DI_CLOCK_STOP_MODE_COUNTER6    6
#define IPU_DI_GENERAL_DI_CLOCK_STOP_MODE_COUNTER7    7
#define IPU_DI_GENERAL_DI_CLOCK_STOP_MODE_COUNTER8    8
#define IPU_DI_GENERAL_DI_CLOCK_STOP_MODE_COUNTER9    9
#define IPU_DI_GENERAL_DI_CLOCK_STOP_MODE_EOL_NOW     12
#define IPU_DI_GENERAL_DI_CLOCK_STOP_MODE_EOF_NOW     13
#define IPU_DI_GENERAL_DI_CLOCK_STOP_MODE_EOL_NEXT_FRAME 14
#define IPU_DI_GENERAL_DI_CLOCK_STOP_MODE_EOF_NEXT_FRAME 15

#define IPU_DI_SW_GEN0_DI_OFFSET_RESOLUTION_DISABLE   0
#define IPU_DI_SW_GEN0_DI_OFFSET_RESOLUTION_DISP_CLK  1
#define IPU_DI_SW_GEN0_DI_OFFSET_RESOLUTION_COUNTER_1 2
#define IPU_DI_SW_GEN0_DI_OFFSET_RESOLUTION_COUNTER_2 3
#define IPU_DI_SW_GEN0_DI_OFFSET_RESOLUTION_COUNTER_3 4
#define IPU_DI_SW_GEN0_DI_OFFSET_RESOLUTION_COUNTER_4 5
#define IPU_DI_SW_GEN0_DI_OFFSET_RESOLUTION_COUNTER_5 6
#define IPU_DI_SW_GEN0_DI_OFFSET_RESOLUTION_EXT_VSYNC 6
#define IPU_DI_SW_GEN0_DI_OFFSET_RESOLUTION_ON        7

#define IPU_DI_SW_GEN0_DI_RUN_RESOLUTION_DISABLE      0
#define IPU_DI_SW_GEN0_DI_RUN_RESOLUTION_DISP_CLK     1
#define IPU_DI_SW_GEN0_DI_RUN_RESOLUTION_COUNTER_1    2
#define IPU_DI_SW_GEN0_DI_RUN_RESOLUTION_COUNTER_2    3
#define IPU_DI_SW_GEN0_DI_RUN_RESOLUTION_COUNTER_3    4
#define IPU_DI_SW_GEN0_DI_RUN_RESOLUTION_COUNTER_4    5
#define IPU_DI_SW_GEN0_DI_RUN_RESOLUTION_COUNTER_5    6
#define IPU_DI_SW_GEN0_DI_RUN_RESOLUTION_EXT_VSYNC    6
#define IPU_DI_SW_GEN0_DI_RUN_RESOLUTION_ON           7


#define IPU_DI_SW_GEN1_DI_CNT_POLARITY_CLR_SEL_INVERTED                   0
#define IPU_DI_SW_GEN1_DI_CNT_POLARITY_CLR_SEL_NOT_INVERTED               1
#define IPU_DI_SW_GEN1_DI_CNT_POLARITY_CLR_SEL_INVERTED_IF_COUNTER_1_SET  2
#define IPU_DI_SW_GEN1_DI_CNT_POLARITY_CLR_SEL_INVERTED_IF_COUNTER_2_SET  3
#define IPU_DI_SW_GEN1_DI_CNT_POLARITY_CLR_SEL_INVERTED_IF_COUNTER_3_SET  4
#define IPU_DI_SW_GEN1_DI_CNT_POLARITY_CLR_SEL_INVERTED_IF_COUNTER_4_SET  5
#define IPU_DI_SW_GEN1_DI_CNT_POLARITY_CLR_SEL_INVERTED_IF_COUNTER_5_SET  6
#define IPU_DI_SW_GEN1_DI_CNT_POLARITY_CLR_SEL_INVERTED_IF_COUNTER_6_SET  7

#define IPU_DI_SW_GEN1_DI_CNT_POLARITY_TRIGGER_SEL_DISABLE     0
#define IPU_DI_SW_GEN1_DI_CNT_POLARITY_TRIGGER_SEL_DISP_CLK    1
#define IPU_DI_SW_GEN1_DI_CNT_POLARITY_TRIGGER_SEL_COUNTER_1   2
#define IPU_DI_SW_GEN1_DI_CNT_POLARITY_TRIGGER_SEL_COUNTER_2   3
#define IPU_DI_SW_GEN1_DI_CNT_POLARITY_TRIGGER_SEL_COUNTER_3   4
#define IPU_DI_SW_GEN1_DI_CNT_POLARITY_TRIGGER_SEL_COUNTER_4   5
#define IPU_DI_SW_GEN1_DI_CNT_POLARITY_TRIGGER_SEL_COUNTER_5   6
#define IPU_DI_SW_GEN1_DI_CNT_POLARITY_TRIGGER_SEL_EXT_VSYNC   6
#define IPU_DI_SW_GEN1_DI_CNT_POLARITY_TRIGGER_SEL_ON          7

#define IPU_DI_SW_GEN1_DI_CNT_CLR_SEL_DISABLE         0
#define IPU_DI_SW_GEN1_DI_CNT_CLR_SEL_DISP_CLK        1
#define IPU_DI_SW_GEN1_DI_CNT_CLR_SEL_COUNTER_1       2
#define IPU_DI_SW_GEN1_DI_CNT_CLR_SEL_COUNTER_2       3
#define IPU_DI_SW_GEN1_DI_CNT_CLR_SEL_COUNTER_3       4
#define IPU_DI_SW_GEN1_DI_CNT_CLR_SEL_COUNTER_4       5
#define IPU_DI_SW_GEN1_DI_CNT_CLR_SEL_COUNTER_5       6
#define IPU_DI_SW_GEN1_DI_CNT_CLR_SEL_EXT_VSYNC       6
#define IPU_DI_SW_GEN1_DI_CNT_CLR_SEL_ON              7

#define IPU_DI_SW_GEN1_DI_CNT_AUTO_RELOAD_FOREVER     1
#define IPU_DI_SW_GEN1_DI_CNT_AUTO_RELOAD_STEP_REPEAT 0

#define IPU_DI_SW_GEN1_DI_CNT_POLARITY_GEN_EN_DISABLE 0
#define IPU_DI_SW_GEN1_DI_CNT_POLARITY_GEN_EN_TRIGGER 1
#define IPU_DI_SW_GEN1_DI_CNT_POLARITY_GEN_EN_ENABLE  2
#define IPU_DI_SW_GEN1_DI_CNT_POLARITY_GEN_EN_COUNTER 3

#define IPU_DI_SW_GEN1_DI_TAG_SEL_COUNTER_9           1
#define IPU_DI_SW_GEN1_DI_TAG_SEL_TRIGGERING_COUNTER  0

#define IPU_DI_DW_GEN_DI_PT_DW_SET0                   0
#define IPU_DI_DW_GEN_DI_PT_DW_SET1                   1
#define IPU_DI_DW_GEN_DI_PT_DW_SET2                   2
#define IPU_DI_DW_GEN_DI_PT_DW_SET3                   3

#define IPU_DI_POL_ACTIVE_HIGH                        1
#define IPU_DI_POL_ACTIVE_LOW                         0

#define IPU_DI_AW0_AW_HCOUNT_SEL_DISABLE              0
#define IPU_DI_AW0_AW_HCOUNT_SEL_COUNTER_1            2
#define IPU_DI_AW0_AW_HCOUNT_SEL_COUNTER_2            3
#define IPU_DI_AW0_AW_HCOUNT_SEL_COUNTER_3            4
#define IPU_DI_AW0_AW_HCOUNT_SEL_COUNTER_4            5
#define IPU_DI_AW0_AW_HCOUNT_SEL_COUNTER_5            6

#define IPU_DI_AW0_AW_TRIG_SEL_DISABLE                0
#define IPU_DI_AW0_AW_TRIG_SEL_DISP_CLK_TRIGGER       1
#define IPU_DI_AW0_AW_TRIG_SEL_COUNTER_1              2
#define IPU_DI_AW0_AW_TRIG_SEL_COUNTER_2              3
#define IPU_DI_AW0_AW_TRIG_SEL_COUNTER_3              4
#define IPU_DI_AW0_AW_TRIG_SEL_COUNTER_4              5
#define IPU_DI_AW0_AW_TRIG_SEL_COUNTER_5              6

#define IPU_DI_AW1_AW_VCOUNT_SEL_DISABLE              0
#define IPU_DI_AW1_AW_VCOUNT_SEL_COUNTER_1            2
#define IPU_DI_AW1_AW_VCOUNT_SEL_COUNTER_2            3
#define IPU_DI_AW1_AW_VCOUNT_SEL_COUNTER_3            4
#define IPU_DI_AW1_AW_VCOUNT_SEL_COUNTER_4            5
#define IPU_DI_AW1_AW_VCOUNT_SEL_COUNTER_5            6


// IPU SMFC register
// SMFC_MAP LSH
#define IPU_SMFC_MAP_CH0_LSH                   0 
#define IPU_SMFC_MAP_CH1_LSH                   3
#define IPU_SMFC_MAP_CH2_LSH                   6
#define IPU_SMFC_MAP_CH3_LSH                   9

// SMFC_MAP wid
#define IPU_SMFC_MAP_CH0_WID                   3
#define IPU_SMFC_MAP_CH1_WID                   3
#define IPU_SMFC_MAP_CH2_WID                   3
#define IPU_SMFC_MAP_CH3_WID                   3

// SMFC_BS LSH
#define IPU_SMFC_BS_CH0_LSH                      0
#define IPU_SMFC_BS_CH1_LSH                      4
#define IPU_SMFC_BS_CH2_LSH                      8
#define IPU_SMFC_BS_CH3_LSH                      12

// SMFC_BS wid
#define IPU_SMFC_BS_CH0_WID                   4
#define IPU_SMFC_BS_CH1_WID                   4
#define IPU_SMFC_BS_CH2_WID                   4
#define IPU_SMFC_BS_CH3_WID                   4

// SMFC_WMC_LSH
#define IPU_SMFC_WMC_SET_CH0_LSH            0
#define IPU_SMFC_WMC_CLR_CH0_LSH            3
#define IPU_SMFC_WMC_SET_CH1_LSH            6
#define IPU_SMFC_WMC_CLR_CH1_LSH            9
#define IPU_SMFC_WMC_SET_CH2_LSH            16
#define IPU_SMFC_WMC_CLR_CH2_LSH            19
#define IPU_SMFC_WMC_SET_CH3_LSH            22
#define IPU_SMFC_WMC_CLR_CH3_LSH            25

// SMFC_WMC_Wid
#define IPU_SMFC_WMC_SET_CH0_WID            3
#define IPU_SMFC_WMC_CLR_CH0_WID            3
#define IPU_SMFC_WMC_SET_CH1_WID            3
#define IPU_SMFC_WMC_CLR_CH1_WID            3
#define IPU_SMFC_WMC_SET_CH2_WID            3
#define IPU_SMFC_WMC_CLR_CH2_WID            3
#define IPU_SMFC_WMC_SET_CH3_WID            3
#define IPU_SMFC_WMC_CLR_CH3_WID            3


// IPU DC Registers
#define IPU_DC_READ_CH_CONF_RD_CHANNEL_EN_ENABLE      1
#define IPU_DC_READ_CH_CONF_RD_CHANNEL_EN_DISABLE     0

#define IPU_DC_PROG_DI_ID_DI1                         1
#define IPU_DC_PROG_DI_ID_DI0                         0

#define IPU_DC_PROG_DISP_ID_DISP0                     0
#define IPU_DC_PROG_DISP_ID_DISP1                     1
#define IPU_DC_PROG_DISP_ID_DISP2                     2
#define IPU_DC_PROG_DISP_ID_DISP3                     3

#define IPU_DC_W_SIZE_8BITS                           0
#define IPU_DC_W_SIZE_16BITS                          1
#define IPU_DC_W_SIZE_24BITS                          2
#define IPU_DC_W_SIZE_32BITS                          3

#define IPU_DC_CHAN_MASK_DEFAULT_NO_MASK              1
#define IPU_DC_CHAN_MASK_DEFAULT_HIGHEST_PRI          0

#define IPU_DC_READ_CH_CONF_CS_ID_CS1                 1
#define IPU_DC_READ_CH_CONF_CS_ID_CS0                 0

#define IPU_DC_RL_CH_PRIORITY_DISABLE                 0
#define IPU_DC_RL_CH_PRIORITY_PRI_1                   1
#define IPU_DC_RL_CH_PRIORITY_PRI_2                   2
#define IPU_DC_RL_CH_PRIORITY_PRI_3                   3
#define IPU_DC_RL_CH_PRIORITY_PRI_4                   4
#define IPU_DC_RL_CH_PRIORITY_PRI_5                   5
#define IPU_DC_RL_CH_PRIORITY_PRI_6                   6
#define IPU_DC_RL_CH_PRIORITY_PRI_7                   7
#define IPU_DC_RL_CH_PRIORITY_PRI_8                   8
#define IPU_DC_RL_CH_PRIORITY_PRI_9                   9
#define IPU_DC_RL_CH_PRIORITY_PRI_10                  10
#define IPU_DC_RL_CH_PRIORITY_PRI_11                  11
#define IPU_DC_RL_CH_PRIORITY_PRI_12                  12
#define IPU_DC_RL_CH_PRIORITY_PRI_13                  13

#define IPU_DC_WR_CH_CONF_PROG_CHAN_TYP_DISABLE                 0
#define IPU_DC_WR_CH_CONF_PROG_CHAN_TYP_NORMAL_NO_ANTI_TEARING  4
#define IPU_DC_WR_CH_CONF_PROG_CHAN_TYP_NORMAL_ANTI_TEARING     5
#define IPU_DC_WR_CH_CONF_PROG_CHAN_TYP_WITH_COMMAND_CHAN       7

#define IPU_DC_WR_CH_CONF_FIELD_MODE_FIELD_MODE       1
#define IPU_DC_WR_CH_CONF_FIELD_MODE_FRAME_MODE       0

#define IPU_DC_GEN_SYNC_1_6_CH1_HANDLES_ASYNC         0
#define IPU_DC_GEN_SYNC_1_6_CH1_HANDLES_SYNC          2

#define IPU_DC_GEN_MASK_EN_ENABLE                     1
#define IPU_DC_GEN_MASK_EN_DISABLE                    0

#define IPU_DC_GEN_MASK4CHAN_5_DP                     1
#define IPU_DC_GEN_MASK4CHAN_5_DC                     0

#define IPU_DC_GEN_SYNC_PRIORITY_5_HIGH               1
#define IPU_DC_GEN_SYNC_PRIORITY_5_LOW                0

#define IPU_DC_GEN_SYNC_PRIORITY_1_HIGH               1
#define IPU_DC_GEN_SYNC_PRIORITY_1_LOW                0

#define IPU_DC_GEN_DC_CH5_TYPE_ASYNC                  1
#define IPU_DC_GEN_DC_CH5_TYPE_SYNC                   0

#define IPU_DC_GEN_DC_BK_EN_ENABLE                    1
#define IPU_DC_GEN_DC_BK_EN_DISABLE                   0

#define IPU_DC_DISP_CONF1_DISP_TYP_SERIAL             0
#define IPU_DC_DISP_CONF1_DISP_TYP_PARALLEL_NO_BYTE_EN 2
#define IPU_DC_DISP_CONF1_DISP_TYP_PARALLEL_BYTE_EN   3

#define IPU_DC_DISP_CONF1_ADDR_INCREMENT_1            0
#define IPU_DC_DISP_CONF1_ADDR_INCREMENT_2            1
#define IPU_DC_DISP_CONF1_ADDR_INCREMENT_3            2
#define IPU_DC_DISP_CONF1_ADDR_INCREMENT_4            3

#define IPU_DC_DISP_CONF1_ADDR_BE_L_INC_0             0
#define IPU_DC_DISP_CONF1_ADDR_BE_L_INC_1             1
#define IPU_DC_DISP_CONF1_ADDR_BE_L_INC_2             2
#define IPU_DC_DISP_CONF1_ADDR_BE_L_INC_3             3

#define IPU_DC_UGDE_0_ODD_EN_ENABLE                   1
#define IPU_DC_UGDE_0_ODD_EN_DISABLE                  0

#define IPU_DC_UGDE_0_AUTO_RESTART_ENABLE             1
#define IPU_DC_UGDE_0_AUTO_RESTART_DISABLE            0

#define IPU_DC_UGDE_0_NF_NL_NEW_LINE                  0
#define IPU_DC_UGDE_0_NF_NL_NEW_FRAME                 1
#define IPU_DC_UGDE_0_NF_NL_NEW_FIELD                 2


// DMFC registers
#define IPU_DMFC_BURST_SIZE_0_32_WORDS                0
#define IPU_DMFC_BURST_SIZE_0_16_WORDS                1
#define IPU_DMFC_BURST_SIZE_0_8_WORDS                 2
#define IPU_DMFC_BURST_SIZE_0_4_WORDS                 3

#define IPU_DMFC_WM_EN_ENABLE                         1
#define IPU_DMFC_WM_EN_DISABLE                        0

#define IPU_DMFC_PPW_C_8BPP                           0
#define IPU_DMFC_PPW_C_16BPP                          1
#define IPU_DMFC_PPW_C_24BPP                          2

#define IPU_DMFC_CHAN_DMFC_ST_ADDR_SEGMENT_0          0
#define IPU_DMFC_CHAN_DMFC_ST_ADDR_SEGMENT_1          1
#define IPU_DMFC_CHAN_DMFC_ST_ADDR_SEGMENT_2          2
#define IPU_DMFC_CHAN_DMFC_ST_ADDR_SEGMENT_3          3
#define IPU_DMFC_CHAN_DMFC_ST_ADDR_SEGMENT_4          4
#define IPU_DMFC_CHAN_DMFC_ST_ADDR_SEGMENT_5          5
#define IPU_DMFC_CHAN_DMFC_ST_ADDR_SEGMENT_6          6
#define IPU_DMFC_CHAN_DMFC_ST_ADDR_SEGMENT_7          7

#define IPU_DMFC_CHAN_DMFC_FIFO_SIZE_512x128          0
#define IPU_DMFC_CHAN_DMFC_FIFO_SIZE_256x128          1
#define IPU_DMFC_CHAN_DMFC_FIFO_SIZE_128x128          2
#define IPU_DMFC_CHAN_DMFC_FIFO_SIZE_64x128           3
#define IPU_DMFC_CHAN_DMFC_FIFO_SIZE_32x128           4
#define IPU_DMFC_CHAN_DMFC_FIFO_SIZE_16x128           5
#define IPU_DMFC_CHAN_DMFC_FIFO_SIZE_8x128            6
#define IPU_DMFC_CHAN_DMFC_FIFO_SIZE_4x128            7


#define IPU_DMFC_GENERAL1_DMFC_DCDP_SYNC_PR_DC        1
#define IPU_DMFC_GENERAL1_DMFC_DCDP_SYNC_PR_DP        2
#define IPU_DMFC_GENERAL1_DMFC_DCDP_SYNC_PR_ROUND_ROBIN 3

#define IPU_DMFC_GENERAL1_WAIT4EOT_WAIT4EOT_MODE      1
#define IPU_DMFC_GENERAL1_WAIT4EOT_NORMAL_MODE        0

#define IPU_DMFC_IC_CTRL_DMFC_IC_IN_PORT_CH28         0
#define IPU_DMFC_IC_CTRL_DMFC_IC_IN_PORT_CH41         1
#define IPU_DMFC_IC_CTRL_DMFC_IC_IN_PORT_CH23         4
#define IPU_DMFC_IC_CTRL_DMFC_IC_IN_PORT_CH27         5
#define IPU_DMFC_IC_CTRL_DMFC_IC_IN_PORT_CH24         6
#define IPU_DMFC_IC_CTRL_DMFC_IC_IN_PORT_CH29         7


#define IPU_DMFC_STAT_DMFC_FIFO_FULL_IS_FULL          1
#define IPU_DMFC_STAT_DMFC_FIFO_FULL_IS_NOT_FULL      0

#define IPU_DMFC_STAT_DMFC_FIFO_EMPTY_IS_EMPTY        1
#define IPU_DMFC_STAT_DMFC_FIFO_EMPTY_IS_NOT_EMPTY    0


// VDI Registers
#define IPU_VDI_C_VDI_CH_422_FORMAT_422               1
#define IPU_VDI_C_VDI_CH_422_FORMAT_420               0

#define IPU_VDI_C_VDI_MOT_SEL_ROM1                    0
#define IPU_VDI_C_VDI_MOT_SEL_ROM2                    1
#define IPU_VDI_C_VDI_MOT_SEL_FULL_MOTION             2

#define IPU_VDI_C_VDI_TOP_FIELD_FIELD0                0
#define IPU_VDI_C_VDI_TOP_FIELD_FIELD1                1


//------------------------------------------------------------------------------
// IDMAC Interrupts
//------------------------------------------------------------------------------

typedef enum IPU_INT_ID_ENUM {
    IPU_INT_IDMAC_0 = 0,
    IPU_INT_IDMAC_1 = 1,
    IPU_INT_IDMAC_2 = 2,
    IPU_INT_IDMAC_3 = 3,
    IPU_INT_IDMAC_4 = 4,
    IPU_INT_IDMAC_5 = 5,
    IPU_INT_IDMAC_6 = 6,
    IPU_INT_IDMAC_7 = 7,
    IPU_INT_IDMAC_8 = 8,
    IPU_INT_IDMAC_9 = 9,
    IPU_INT_IDMAC_10 = 10,
    IPU_INT_IDMAC_11 = 11,
    IPU_INT_IDMAC_12 = 12,
    IPU_INT_IDMAC_13 = 13,
    IPU_INT_IDMAC_14 = 14,
    IPU_INT_IDMAC_15 = 15,
    IPU_INT_IDMAC_16 = 16,
    IPU_INT_IDMAC_17 = 17,
    IPU_INT_IDMAC_18 = 18,
    IPU_INT_IDMAC_19 = 19,
    IPU_INT_IDMAC_20 = 20,
    IPU_INT_IDMAC_21 = 21,
    IPU_INT_IDMAC_22 = 22,
    IPU_INT_IDMAC_23 = 23,
    IPU_INT_IDMAC_24 = 24,
    IPU_INT_IDMAC_25 = 25,
    IPU_INT_IDMAC_26 = 26,
    IPU_INT_IDMAC_27 = 27,
    IPU_INT_IDMAC_28 = 28,
    IPU_INT_IDMAC_29 = 29,
    IPU_INT_IDMAC_30 = 30,
    IPU_INT_IDMAC_31 = 31,
    IPU_INT_MAX_ID,
    IPU_INT_IDMAC_32 = 0,
    IPU_INT_IDMAC_33 = 1,
    IPU_INT_IDMAC_34 = 2,
    IPU_INT_IDMAC_35 = 3,
    IPU_INT_IDMAC_36 = 4,
    IPU_INT_IDMAC_37 = 5,
    IPU_INT_IDMAC_38 = 6,
    IPU_INT_IDMAC_39 = 7,
    IPU_INT_IDMAC_40 = 8,
    IPU_INT_IDMAC_41 = 9,
    IPU_INT_IDMAC_42 = 10,
    IPU_INT_IDMAC_43 = 11,
    IPU_INT_IDMAC_44 = 12,
    IPU_INT_IDMAC_45 = 13,
    IPU_INT_IDMAC_46 = 14,
    IPU_INT_IDMAC_47 = 15,
    IPU_INT_IDMAC_48 = 16,
    IPU_INT_IDMAC_49 = 17,
    IPU_INT_IDMAC_50 = 18,
    IPU_INT_IDMAC_51 = 19,
    IPU_INT_IDMAC_52 = 20
} IPU_INT_ID;


//------------------------------------------------------------------------------
// CPMEM Left-shift and Width Values
//------------------------------------------------------------------------------

// Planar mode
#define CPMEM_PLNR_XV_LSH                             0
#define CPMEM_PLNR_YV_LSH                             10
#define CPMEM_PLNR_XB_LSH                             19
#define CPMEM_PLNR_YB_LSH                             0
#define CPMEM_PLNR_NSB_B_LSH                          12
#define CPMEM_PLNR_CF_LSH                             13
#define CPMEM_PLNR_UBO_LOW_LSH                        14
#define CPMEM_PLNR_UBO_HIGH_LSH                       0
#define CPMEM_PLNR_VBO_LSH                            4
#define CPMEM_PLNR_SO_LSH                             17
#define CPMEM_PLNR_BNDM_LSH                           18
#define CPMEM_PLNR_BM_LSH                             21
#define CPMEM_PLNR_ROT_LSH                            23
#define CPMEM_PLNR_HF_LSH                             24
#define CPMEM_PLNR_VF_LSH                             25
#define CPMEM_PLNR_THE_LSH                            26
#define CPMEM_PLNR_CAP_LSH                            27
#define CPMEM_PLNR_CAE_LSH                            28
#define CPMEM_PLNR_FW_LOW_LSH                         29
#define CPMEM_PLNR_FW_HIGH_LSH                        0
#define CPMEM_PLNR_FH_LSH                             10
#define CPMEM_PLNR_EBA0_LSH                           0
#define CPMEM_PLNR_EBA1_LOW_LSH                       29
#define CPMEM_PLNR_EBA1_HIGH_LSH                      0
#define CPMEM_PLNR_ILO_LOW_LSH                        26
#define CPMEM_PLNR_ILO_HIGH_LSH                       0
#define CPMEM_PLNR_NPB_LSH                            14
#define CPMEM_PLNR_PFS_LSH                            21
#define CPMEM_PLNR_ALU_LSH                            25
#define CPMEM_PLNR_ALBM_LSH                           26
#define CPMEM_PLNR_ID_LSH                             29
#define CPMEM_PLNR_TH_LOW_LSH                         31
#define CPMEM_PLNR_TH_HIGH_LSH                        0
#define CPMEM_PLNR_SLY_LSH                            6
#define CPMEM_PLNR_WID3_LSH                           29
#define CPMEM_PLNR_SLUV_LSH                           0

// Interleaved mode
#define CPMEM_ILVD_XV_LSH                             0
#define CPMEM_ILVD_YV_LSH                             10
#define CPMEM_ILVD_XB_LSH                             19
#define CPMEM_ILVD_YB_LSH                             0
#define CPMEM_ILVD_NSB_B_LSH                          12
#define CPMEM_ILVD_CF_LSH                             13
#define CPMEM_ILVD_SX_LSH                             14
#define CPMEM_ILVD_SY_LOW_LSH                         26
#define CPMEM_ILVD_SY_HIGH_LSH                        0
#define CPMEM_ILVD_NS_LSH                             5
#define CPMEM_ILVD_SDX_LSH                            15
#define CPMEM_ILVD_SM_LSH                             22
#define CPMEM_ILVD_SCC_LSH                            0
#define CPMEM_ILVD_SCE_LSH                            1
#define CPMEM_ILVD_SDY_LSH                            2
#define CPMEM_ILVD_SDRX_LSH                           9
#define CPMEM_ILVD_SDRY_LSH                           10
#define CPMEM_ILVD_BPP_LSH                            11
#define CPMEM_ILVD_DEC_SEL_LSH                        14
#define CPMEM_ILVD_DIM_LSH                            16
#define CPMEM_ILVD_SO_LSH                             17
#define CPMEM_ILVD_BNDM_LSH                           18
#define CPMEM_ILVD_BM_LSH                             21
#define CPMEM_ILVD_ROT_LSH                            23
#define CPMEM_ILVD_HF_LSH                             24
#define CPMEM_ILVD_VF_LSH                             25
#define CPMEM_ILVD_THE_LSH                            26
#define CPMEM_ILVD_CAP_LSH                            27
#define CPMEM_ILVD_CAE_LSH                            28
#define CPMEM_ILVD_FW_LOW_LSH                         29
#define CPMEM_ILVD_FW_HIGH_LSH                        0
#define CPMEM_ILVD_FH_LSH                             10
#define CPMEM_ILVD_EBA0_LSH                           0
#define CPMEM_ILVD_EBA1_LOW_LSH                       29
#define CPMEM_ILVD_EBA1_HIGH_LSH                      0
#define CPMEM_ILVD_ILO_LOW_LSH                        26
#define CPMEM_ILVD_ILO_HIGH_LSH                       0
#define CPMEM_ILVD_NPB_LSH                            14
#define CPMEM_ILVD_PFS_LSH                            21
#define CPMEM_ILVD_ALU_LSH                            25
#define CPMEM_ILVD_ALBM_LSH                           26
#define CPMEM_ILVD_ID_LSH                             29
#define CPMEM_ILVD_TH_LOW_LSH                         31
#define CPMEM_ILVD_TH_HIGH_LSH                        0
#define CPMEM_ILVD_SL_LSH                             6
#define CPMEM_ILVD_WID0_LSH                           20
#define CPMEM_ILVD_WID1_LSH                           23
#define CPMEM_ILVD_WID2_LSH                           26
#define CPMEM_ILVD_WID3_LSH                           29
#define CPMEM_ILVD_OFS0_LSH                           0
#define CPMEM_ILVD_OFS1_LSH                           5
#define CPMEM_ILVD_OFS2_LSH                           10
#define CPMEM_ILVD_OFS3_LSH                           15

// Planar mode
#define CPMEM_PLNR_XV_WID                             10
#define CPMEM_PLNR_YV_WID                             9
#define CPMEM_PLNR_XB_WID                             13
#define CPMEM_PLNR_YB_WID                             12
#define CPMEM_PLNR_NSB_B_WID                          1
#define CPMEM_PLNR_CF_WID                             1
#define CPMEM_PLNR_UBO_LOW_WID                        18
#define CPMEM_PLNR_UBO_HIGH_WID                       4
#define CPMEM_PLNR_VBO_WID                            22
#define CPMEM_PLNR_SO_WID                             1
#define CPMEM_PLNR_BNDM_WID                           3
#define CPMEM_PLNR_BM_WID                             2
#define CPMEM_PLNR_ROT_WID                            1
#define CPMEM_PLNR_HF_WID                             1
#define CPMEM_PLNR_VF_WID                             1
#define CPMEM_PLNR_THE_WID                            1
#define CPMEM_PLNR_CAP_WID                            1
#define CPMEM_PLNR_CAE_WID                            1
#define CPMEM_PLNR_FW_LOW_WID                         3
#define CPMEM_PLNR_FW_HIGH_WID                        10
#define CPMEM_PLNR_FH_WID                             12
#define CPMEM_PLNR_EBA0_WID                           29
#define CPMEM_PLNR_EBA1_LOW_WID                       3
#define CPMEM_PLNR_EBA1_HIGH_WID                      26
#define CPMEM_PLNR_ILO_LOW_WID                        6
#define CPMEM_PLNR_ILO_HIGH_WID                       14
#define CPMEM_PLNR_NPB_WID                            7
#define CPMEM_PLNR_PFS_WID                            4
#define CPMEM_PLNR_ALU_WID                            1
#define CPMEM_PLNR_ALBM_WID                           3
#define CPMEM_PLNR_ID_WID                             2
#define CPMEM_PLNR_TH_LOW_WID                         1
#define CPMEM_PLNR_TH_HIGH_WID                        6
#define CPMEM_PLNR_SLY_WID                            14
#define CPMEM_PLNR_WID3_WID                           3
#define CPMEM_PLNR_SLUV_WID                           14

// Interleaved mode
#define CPMEM_ILVD_XV_WID                             10
#define CPMEM_ILVD_YV_WID                             9
#define CPMEM_ILVD_XB_WID                             13
#define CPMEM_ILVD_YB_WID                             12
#define CPMEM_ILVD_NSB_B_WID                          1
#define CPMEM_ILVD_CF_WID                             1
#define CPMEM_ILVD_SX_WID                             12
#define CPMEM_ILVD_SY_LOW_WID                         6
#define CPMEM_ILVD_SY_HIGH_WID                        5
#define CPMEM_ILVD_NS_WID                             10
#define CPMEM_ILVD_SDX_WID                            7
#define CPMEM_ILVD_SM_WID                             10
#define CPMEM_ILVD_SCC_WID                            1
#define CPMEM_ILVD_SCE_WID                            1
#define CPMEM_ILVD_SDY_WID                            7
#define CPMEM_ILVD_SDRX_WID                           1
#define CPMEM_ILVD_SDRY_WID                           1
#define CPMEM_ILVD_BPP_WID                            3
#define CPMEM_ILVD_DEC_SEL_WID                        2
#define CPMEM_ILVD_DIM_WID                            1
#define CPMEM_ILVD_SO_WID                             1
#define CPMEM_ILVD_BNDM_WID                           3
#define CPMEM_ILVD_BM_WID                             2
#define CPMEM_ILVD_ROT_WID                            1
#define CPMEM_ILVD_HF_WID                             1
#define CPMEM_ILVD_VF_WID                             1
#define CPMEM_ILVD_THE_WID                            1
#define CPMEM_ILVD_CAP_WID                            1
#define CPMEM_ILVD_CAE_WID                            1
#define CPMEM_ILVD_FW_LOW_WID                         3
#define CPMEM_ILVD_FW_HIGH_WID                        10
#define CPMEM_ILVD_FH_WID                             12
#define CPMEM_ILVD_EBA0_WID                           29
#define CPMEM_ILVD_EBA1_LOW_WID                       3
#define CPMEM_ILVD_EBA1_HIGH_WID                      26
#define CPMEM_ILVD_ILO_LOW_WID                        6
#define CPMEM_ILVD_ILO_HIGH_WID                       14
#define CPMEM_ILVD_NPB_WID                            7
#define CPMEM_ILVD_PFS_WID                            4
#define CPMEM_ILVD_ALU_WID                            1
#define CPMEM_ILVD_ALBM_WID                           3
#define CPMEM_ILVD_ID_WID                             2
#define CPMEM_ILVD_TH_LOW_WID                         1
#define CPMEM_ILVD_TH_HIGH_WID                        6
#define CPMEM_ILVD_SL_WID                             14
#define CPMEM_ILVD_WID0_WID                           3
#define CPMEM_ILVD_WID1_WID                           3
#define CPMEM_ILVD_WID2_WID                           3
#define CPMEM_ILVD_WID3_WID                           3
#define CPMEM_ILVD_OFS0_WID                           5
#define CPMEM_ILVD_OFS1_WID                           5
#define CPMEM_ILVD_OFS2_WID                           5
#define CPMEM_ILVD_OFS3_WID                           5

//------------------------------------------------------------------------------
// TPM Left-shift and Width Values
//------------------------------------------------------------------------------
#define TPM_C22_LSH                                   0
#define TPM_C11_LSH                                   9
#define TPM_C00_LSH                                   18
#define TPM_A0_LOW_LSH                                27
#define TPM_A0_HIGH_LSH                               0
#define TPM_SCALE_LSH                                 8
#define TPM_SAT_MODE_LSH                              10
#define TPM_C20_LSH                                   0
#define TPM_C10_LSH                                   9
#define TPM_C01_LSH                                   18
#define TPM_A1_LOW_LSH                                27
#define TPM_A1_HIGH_LSH                               0
#define TPM_C21_LSH                                   0
#define TPM_C12_LSH                                   9
#define TPM_C02_LSH                                   18
#define TPM_A2_LOW_LSH                                27
#define TPM_A2_HIGH_LSH                               0

#define TPM_C22_WID                                   9
#define TPM_C11_WID                                   9
#define TPM_C00_WID                                   9
#define TPM_A0_LOW_WID                                5
#define TPM_A0_HIGH_WID                               8
#define TPM_SCALE_WID                                 2
#define TPM_SAT_MODE_WID                              1
#define TPM_C20_WID                                   9
#define TPM_C10_WID                                   9
#define TPM_C01_WID                                   9
#define TPM_A1_LOW_WID                                5
#define TPM_A1_HIGH_WID                               8
#define TPM_C21_WID                                   9
#define TPM_C12_WID                                   9
#define TPM_C02_WID                                   9
#define TPM_A2_LOW_WID                                5
#define TPM_A2_HIGH_WID                               8

//------------------------------------------------------------------------------
// DC Template opcodes and command fields
//------------------------------------------------------------------------------

// Template Opcodes

#define HLG_OPCODE                                    0x0
#define WRG_OPCODE                                    0x1
#define HLOA_OPCODE                                   0xA
#define WROA_OPCODE                                   0xE
#define HLOD_OPCODE                                   0x10
#define WROD_OPCODE                                   0x18
#define HLOAR_OPCODE                                  0x47
#define WROAR_OPCODE                                  0x67
#define HLODR_OPCODE                                  0x23
#define WRODR_OPCODE                                  0x33
#define WRBC_OPCODE                                   0x19B
#define WCLK_OPCODE                                   0xC9
#define WSTSI_OPCODE                                  0x89
#define WSTSII_OPCODE                                 0x8A
#define WSTSIII_OPCODE                                0x8B
#define RD_OPCODE                                     0x88
#define WACK_OPCODE                                   0x11A
#define MSK_OPCODE                                    0x190
#define HMA_OPCODE                                    0x040
#define HMA1_OPCODE                                   0x020
#define BMA_OPCODE                                    0x3

// Left-shift

#define DC_TEMPLATE_SYNC_LSH                          0
#define DC_TEMPLATE_GLUELOGIC_LSH                     4
#define DC_TEMPLATE_WAVEFORM_LSH                      11
#define DC_TEMPLATE_MAPPING_LSH                       15
#define DC_TEMPLATE_OPERAND_LSH                       20
#define DC_TEMPLATE_OPCODE_LSH                        0
#define DC_TEMPLATE_STOP_LSH                          9

#define DC_TEMPLATE_HLG_DATA_LOW_LSH                  5
#define DC_TEMPLATE_HLG_DATA_HIGH_LSH                 0
#define DC_TEMPLATE_HLG_OPCODE_LSH                    5

#define DC_TEMPLATE_WRG_DATA_LOW_LSH                  15
#define DC_TEMPLATE_WRG_DATA_HIGH_LSH                 0
#define DC_TEMPLATE_WRG_OPCODE_LSH                    7

#define DC_TEMPLATE_HLOA_DATA_LOW_LSH                 20
#define DC_TEMPLATE_HLOA_DATA_HIGH_LSH                0
#define DC_TEMPLATE_HLOA_AF_LSH                       4
#define DC_TEMPLATE_HLOA_OPCODE_LSH                   5

#define DC_TEMPLATE_WROA_DATA_LOW_LSH                 20
#define DC_TEMPLATE_WROA_DATA_HIGH_LSH                0
#define DC_TEMPLATE_WROA_AF_LSH                       4
#define DC_TEMPLATE_WROA_OPCODE_LSH                   5

#define DC_TEMPLATE_HLOD_DATA_LOW_LSH                 20
#define DC_TEMPLATE_HLOD_DATA_HIGH_LSH                0
#define DC_TEMPLATE_HLOD_OPCODE_LSH                   4

#define DC_TEMPLATE_WROD_DATA_LOW_LSH                 20
#define DC_TEMPLATE_WROD_DATA_HIGH_LSH                0
#define DC_TEMPLATE_WROD_OPCODE_LSH                   4

#define DC_TEMPLATE_HLOAR_AF_LSH                      1
#define DC_TEMPLATE_HLOAR_OPCODE_LSH                  2

#define DC_TEMPLATE_WROAR_AF_LSH                      1
#define DC_TEMPLATE_WROAR_OPCODE_LSH                  2

#define DC_TEMPLATE_HLODR_OPCODE_LSH                  3

#define DC_TEMPLATE_WRODR_M0_LSH                      30
#define DC_TEMPLATE_WRODR_M1_LSH                      31
#define DC_TEMPLATE_WRODR_M2_LSH                      0
#define DC_TEMPLATE_WRODR_OPCODE_LSH                  3

#define DC_TEMPLATE_WRBC_OPCODE_LSH                   0

#define DC_TEMPLATE_WCLK_N_CLK_OPERAND_LOW_LSH        20
#define DC_TEMPLATE_WCLK_N_CLK_OPERAND_HIGH_LSH       0
#define DC_TEMPLATE_WCLK_OPCODE_LSH                   1

#define DC_TEMPLATE_WSTSI_N_CLK_OPERAND_LOW_LSH       20
#define DC_TEMPLATE_WSTSI_N_CLK_OPERAND_HIGH_LSH      0
#define DC_TEMPLATE_WSTSI_OPCODE_LSH                  1

#define DC_TEMPLATE_WSTSII_N_CLK_OPERAND_LOW_LSH      20
#define DC_TEMPLATE_WSTSII_N_CLK_OPERAND_HIGH_LSH     0
#define DC_TEMPLATE_WSTSII_OPCODE_LSH                 1

#define DC_TEMPLATE_WSTSIII_N_CLK_OPERAND_LOW_LSH     20
#define DC_TEMPLATE_WSTSIII_N_CLK_OPERAND_HIGH_LSH    0
#define DC_TEMPLATE_WSTSIII_OPCODE_LSH                1

#define DC_TEMPLATE_RD_N_CLK_OPERAND_LOW_LSH          20
#define DC_TEMPLATE_RD_N_CLK_OPERAND_HIGH_LSH         0
#define DC_TEMPLATE_RD_OPCODE_LSH                     1

#define DC_TEMPLATE_WACK_N_CLK_OPERAND_LSH            19
#define DC_TEMPLATE_WACK_OPCODE_LSH                   0

#define DC_TEMPLATE_MSK_DM_LSH                        15
#define DC_TEMPLATE_MSK_NCM_LSH                       16
#define DC_TEMPLATE_MSK_NADM_LSH                      17
#define DC_TEMPLATE_MSK_EOFLDM_LSH                    18
#define DC_TEMPLATE_MSK_EOLM_LSH                      19
#define DC_TEMPLATE_MSK_EOFM_LSH                      20
#define DC_TEMPLATE_MSK_NFLDM_LSH                     21
#define DC_TEMPLATE_MSK_NLM_LSH                       22
#define DC_TEMPLATE_MSK_NFM_LSH                       23
#define DC_TEMPLATE_MSK_E3M_LSH                       24
#define DC_TEMPLATE_MSK_E2M_LSH                       25
#define DC_TEMPLATE_MSK_E1M_LSH                       26
#define DC_TEMPLATE_MSK_E0M_LSH                       27
#define DC_TEMPLATE_MSK_OPCODE_LSH                    0

#define DC_TEMPLATE_HMA_ADDRESS_LSH                   5
#define DC_TEMPLATE_HMA_OPCODE_LSH                    0

#define DC_TEMPLATE_HMA1_ADDRESS_LSH                  5
#define DC_TEMPLATE_HMA1_OPCODE_LSH                   0

#define DC_TEMPLATE_BMA_N_LSH                         5
#define DC_TEMPLATE_BMA_AF_LSH                        3
#define DC_TEMPLATE_BMA_LF_LSH                        4
#define DC_TEMPLATE_BMA_OPCODE_LSH                    5

// Width

#define DC_TEMPLATE_SYNC_WID                          4
#define DC_TEMPLATE_GLUELOGIC_WID                     7
#define DC_TEMPLATE_WAVEFORM_WID                      4
#define DC_TEMPLATE_MAPPING_WID                       5
#define DC_TEMPLATE_OPERAND_WID                       12
#define DC_TEMPLATE_OPCODE_WID                        9
#define DC_TEMPLATE_STOP_WID                          1

#define DC_TEMPLATE_HLG_DATA_LOW_WID                  27
#define DC_TEMPLATE_HLG_DATA_HIGH_WID                 5
#define DC_TEMPLATE_HLG_OPCODE_WID                    4

#define DC_TEMPLATE_WRG_DATA_LOW_WID                  17
#define DC_TEMPLATE_WRG_DATA_HIGH_WID                 7
#define DC_TEMPLATE_WRG_OPCODE_WID                    2

#define DC_TEMPLATE_HLOA_DATA_LOW_WID                 12
#define DC_TEMPLATE_HLOA_DATA_HIGH_WID                4
#define DC_TEMPLATE_HLOA_AF_WID                       1
#define DC_TEMPLATE_HLOA_OPCODE_WID                   4

#define DC_TEMPLATE_WROA_DATA_LOW_WID                 12
#define DC_TEMPLATE_WROA_DATA_HIGH_WID                4
#define DC_TEMPLATE_WROA_AF_WID                       1
#define DC_TEMPLATE_WROA_OPCODE_WID                   4

#define DC_TEMPLATE_HLOD_DATA_LOW_WID                 12
#define DC_TEMPLATE_HLOD_DATA_HIGH_WID                4
#define DC_TEMPLATE_HLOD_OPCODE_WID                   5

#define DC_TEMPLATE_WROD_DATA_LOW_WID                 12
#define DC_TEMPLATE_WROD_DATA_HIGH_WID                4
#define DC_TEMPLATE_WROD_OPCODE_WID                   5

#define DC_TEMPLATE_HLOAR_AF_WID                      1
#define DC_TEMPLATE_HLOAR_OPCODE_WID                  7

#define DC_TEMPLATE_WROAR_AF_WID                      1
#define DC_TEMPLATE_WROAR_OPCODE_WID                  7

#define DC_TEMPLATE_HLODR_OPCODE_WID                  6

#define DC_TEMPLATE_WRODR_M0_WID                      1
#define DC_TEMPLATE_WRODR_M1_WID                      1
#define DC_TEMPLATE_WRODR_M2_WID                      1
#define DC_TEMPLATE_WRODR_OPCODE_WID                  6

#define DC_TEMPLATE_WRBC_OPCODE_WID                   9

#define DC_TEMPLATE_WCLK_N_CLK_OPERAND_LOW_WID        12
#define DC_TEMPLATE_WCLK_N_CLK_OPERAND_HIGH_WID       1
#define DC_TEMPLATE_WCLK_OPCODE_WID                   8

#define DC_TEMPLATE_WSTSI_N_CLK_OPERAND_LOW_WID       12
#define DC_TEMPLATE_WSTSI_N_CLK_OPERAND_HIGH_WID      1
#define DC_TEMPLATE_WSTSI_OPCODE_WID                  8

#define DC_TEMPLATE_WSTSII_N_CLK_OPERAND_LOW_WID      12
#define DC_TEMPLATE_WSTSII_N_CLK_OPERAND_HIGH_WID     1
#define DC_TEMPLATE_WSTSII_OPCODE_WID                 8

#define DC_TEMPLATE_WSTSIII_N_CLK_OPERAND_LOW_WID     12
#define DC_TEMPLATE_WSTSIII_N_CLK_OPERAND_HIGH_WID    1
#define DC_TEMPLATE_WSTSIII_OPCODE_WID                8

#define DC_TEMPLATE_RD_N_CLK_OPERAND_LOW_WID          12
#define DC_TEMPLATE_RD_N_CLK_OPERAND_HIGH_WID         1
#define DC_TEMPLATE_RD_OPCODE_WID                     8

#define DC_TEMPLATE_WACK_N_CLK_OPERAND_WID            13
#define DC_TEMPLATE_WACK_OPCODE_WID                   9

#define DC_TEMPLATE_MSK_DM_WID                        1
#define DC_TEMPLATE_MSK_NCM_WID                       1
#define DC_TEMPLATE_MSK_NADM_WID                      1
#define DC_TEMPLATE_MSK_EOFLDM_WID                    1
#define DC_TEMPLATE_MSK_EOLM_WID                      1
#define DC_TEMPLATE_MSK_EOFM_WID                      1
#define DC_TEMPLATE_MSK_NFLDM_WID                     1
#define DC_TEMPLATE_MSK_NLM_WID                       1
#define DC_TEMPLATE_MSK_NFM_WID                       1
#define DC_TEMPLATE_MSK_E3M_WID                       1
#define DC_TEMPLATE_MSK_E2M_WID                       1
#define DC_TEMPLATE_MSK_E1M_WID                       1
#define DC_TEMPLATE_MSK_E0M_WID                       1
#define DC_TEMPLATE_MSK_OPCODE_WID                    9

#define DC_TEMPLATE_HMA_ADDRESS_WID                   8
#define DC_TEMPLATE_HMA_OPCODE_WID                    9

#define DC_TEMPLATE_HMA1_ADDRESS_WID                  8
#define DC_TEMPLATE_HMA1_OPCODE_WID                   9

#define DC_TEMPLATE_BMA_N_WID                         8
#define DC_TEMPLATE_BMA_AF_WID                        1
#define DC_TEMPLATE_BMA_LF_WID                        1
#define DC_TEMPLATE_BMA_OPCODE_WID                    4

#ifdef __cplusplus
}
#endif

#endif // __COMMON_IPUV3_H
