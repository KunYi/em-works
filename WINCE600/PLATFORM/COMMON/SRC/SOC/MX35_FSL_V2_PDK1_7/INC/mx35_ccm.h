//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header:  mx35_ccm.h
//
//  Provides the public interface for the CCM module.  This module defines
//  the header information for the application processor's clock and reset
//  module.
//
//------------------------------------------------------------------------------

#ifndef __MX35_CCM_H__
#define __MX35_CCM_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------
#define CCM_CGR_CG_MASK                 (0x3)

//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------
typedef struct
{
    UINT32 CCMR;
    UINT32 PDR0;
    UINT32 PDR1;
    UINT32 PDR2;
    UINT32 PDR3;
    UINT32 PDR4;
    UINT32 RCSR;
    UINT32 MPCTL;
    UINT32 PPCTL;
    UINT32 ACMR;
    UINT32 COSR;
    UINT32 CGR[4];
    UINT32 _reserved;
    UINT32 DCVR[4];
    UINT32 LTR0;
    UINT32 LTR1;
    UINT32 LTR2;
    UINT32 LTR3;
    UINT32 LTBR0;
    UINT32 LTBR1;
    UINT32 PMCR0;
    UINT32 PMCR1;
    UINT32 PMCR2;
} CSP_CCM_REGS, *PCSP_CCM_REGS;

//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
#define CCM_CCMR_OFFSET                         0x0000
#define CCM_PDR0_OFFSET                         0x0004
#define CCM_PDR1_OFFSET                         0x0008
#define CCM_PDR2_OFFSET                         0x000C
#define CCM_PDR3_OFFSET                         0x0010
#define CCM_PDR4_OFFSET                         0x0014
#define CCM_RCSR_OFFSET                         0x0018
#define CCM_MPCTL_OFFSET                        0x001C
#define CCM_PPCTL_OFFSET                        0x0020
#define CCM_ACMR_OFFSET                         0x0024
#define CCM_COSR_OFFSET                         0x0028
#define CCM_CGR_OFFSET                          0x002C
#define CCM_DCVR_OFFSET                         0x0040
#define CCM_LTR0_OFFSET                         0x0050
#define CCM_LTR1_OFFSET                         0x0054
#define CCM_LTR2_OFFSET                         0x0058
#define CCM_LTR3_OFFSET                         0x005C
#define CCM_LTBR0_OFFSET                        0x0060
#define CCM_LTBR1_OFFSET                        0x0064
#define CCM_PMCR0_OFFSET                        0x0068
#define CCM_PMCR1_OFFSET                        0x006C
#define CCM_PMCR2_OFFSET                        0x0070


//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
#define CCM_CCMR_MPE_LSH                        3
#define CCM_CCMR_UPE_LSH                        9
#define CCM_CCMR_LPM_LSH                        14
#define CCM_CCMR_RAMW_LSH                       16
#define CCM_CCMR_ROMW_LSH                       18
#define CCM_CCMR_VOL_RDY_CNT_LSH                20
#define CCM_CCMR_WBEN_LSH                       27
#define CCM_CCMR_VSTBY_LSH                      28
#define CCM_CCMR_STBY_EXIT_SRC_LSH              29
#define CCM_CCMR_WFI_LSH                        30

#define CCM_PDR0_AUTO_CON_LSH                   0
#define CCM_PDR0_AUTO_MUX_DIV_LSH               9
#define CCM_PDR0_CCM_PER_AHB_LSH                12
#define CCM_PDR0_CKIL_SEL_LSH                   15
#define CCM_PDR0_CON_MUX_DIV_LSH                16
#define CCM_PDR0_HSP_PODF_LSH                   20
#define CCM_PDR0_IPU_HND_BYP_LSH                23
#define CCM_PDR0_PER_SEL_LSH                    26

#define CCM_PDR1_MSHC_M_U_LSH                   7
#define CCM_PDR1_MSHC_DIV_LSH                   22
#define CCM_PDR1_MSHC_DIV_PRE_LSH               28

#define CCM_PDR2_SSI1_DIV_LSH                   0
#define CCM_PDR2_SSI_M_U_LSH                    6
#define CCM_PDR2_CSI_M_U_LSH                    7
#define CCM_PDR2_SSI2_DIV_LSH                   8
#define CCM_PDR2_CSI_DIV_LSH                    16
#define CCM_PDR2_CSI_DIV_PST_LSH                16
#define CCM_PDR2_CSI_DIV_PRE_LSH                19
#define CCM_PDR2_SSI1_DIV_PRE_LSH               24
#define CCM_PDR2_SSI2_DIV_PRE_LSH               27

#define CCM_PDR3_ESDHC1_DIV_LSH                 0
#define CCM_PDR3_ESDHC1_DIV_PST_LSH             0
#define CCM_PDR3_ESDHC1_DIV_PRE_LSH             3
#define CCM_PDR3_ESDHC_M_U_LSH                  6
#define CCM_PDR3_ESDHC2_DIV_LSH                 8
#define CCM_PDR3_ESDHC2_DIV_PST_LSH             8
#define CCM_PDR3_ESDHC2_DIV_PRE_LSH             11
#define CCM_PDR3_UART_M_U_LSH                   14
#define CCM_PDR3_ESDHC3_DIV_LSH                 16
#define CCM_PDR3_ESDHC3_DIV_PST_LSH             16
#define CCM_PDR3_ESDHC3_DIV_PRE_LSH             19
#define CCM_PDR3_SPDIF_M_U_LSH                  22
#define CCM_PDR3_SPDIF_DIV_LSH                  23
#define CCM_PDR3_SPDIF_DIV_PRE_LSH              29

#define CCM_PDR4_USB_M_U_LSH                    9
#define CCM_PDR4_UART_DIV_LSH                   10
#define CCM_PDR4_UART_DIV_PST_LSH               10
#define CCM_PDR4_UART_DIV_PRE_LSH               13
#define CCM_PDR4_PER0_DIV_LSH                   16
#define CCM_PDR4_PER0_DIV_PST_LSH               16
#define CCM_PDR4_PER0_DIV_PRE_LSH               19
#define CCM_PDR4_USB_DIV_LSH                    22
#define CCM_PDR4_USB_DIV_PST_LSH                22
#define CCM_PDR4_USB_DIV_PRE_LSH                25
#define CCM_PDR4_NFC_DIV_LSH                    28

#define CCM_RCSR_REST_LSH                       0
#define CCM_RCSR_GPF_LSH                        4
#define CCM_RCSR_NFC_FMS_LSH                    8
#define CCM_RCSR_NFC_4K_LSH                     9
#define CCM_RCSR_BOOT_REG_LSH                   10
#define CCM_RCSR_NFC_16BIT_SEL0_LSH             14
#define CCM_RCSR_PERES_LSH                      15
#define CCM_RCSR_BT_ECC_LSH                     22
#define CCM_RCSR_MEM_TYPE_LSH                   23
#define CCM_RCSR_MEM_CTRL_LSH                   25
#define CCM_RCSR_PAGE_SIZE_LSH                  27
#define CCM_RCSR_BUS_WIDTH_LSH                  29
#define CCM_RCSR_BT_USB_SRC_LSH                 30

#define CCM_MPCTL_MFN_LSH                       0
#define CCM_MPCTL_MFI_LSH                       10
#define CCM_MPCTL_MFD_LSH                       16
#define CCM_MPCTL_PDF_LSH                       26
#define CCM_MPCTL_BRMO_LSH                      31

#define CCM_PPCTL_MFN_LSH                       0
#define CCM_PPCTL_MFI_LSH                       10
#define CCM_PPCTL_MFD_LSH                       16
#define CCM_PPCTL_PDF_LSH                       26
#define CCM_PPCTL_BRMO_LSH                      31

#define CCM_ACMR_SSI2_AUDIO_CLK_SEL_LSH         0
#define CCM_ACMR_SSI1_AUDIO_CLK_SEL_LSH         4
#define CCM_ACMR_SPDIF_AUDIO_CLK_SEL_LSH        8
#define CCM_ACMR_ESAI_AUDIO_CLK_SEL_LSH         12

#define CCM_COSR_CLKOSEL_LSH                    0
#define CCM_COSR_CLKOEN_LSH                     5
#define CCM_COSR_CLKO_DIV1_LSH                  6
#define CCM_COSR_CLKO_DIV_LSH                   10
#define CCM_COSR_CLKO_DIV_PRE_LSH               13
#define CCM_COSR_SSI1_RX_SRC_SEL_LSH            16
#define CCM_COSR_SSI1_TX_SRC_SEL_LSH            18
#define CCM_COSR_SSI2_RX_SRC_SEL_LSH            20
#define CCM_COSR_SSI2_TX_SRC_SEL_LSH            22
#define CCM_COSR_ASRC_AUDIO_EN_LSH              24
#define CCM_COSR_ASRC_AUDIO_PODF_LSH            26

#define CCM_DCVR_ELV_LSH                        2
#define CCM_DCVR_LLV_LSH                        12
#define CCM_DCVR_ULV_LSH                        22

#define CCM_LTR0_DIV3CK_LSH                     1
#define CCM_LTR0_SIGD0_LSH                      3
#define CCM_LTR0_SIGD12_LSH                     15
#define CCM_LTR0_DNTHR_LSH                      16
#define CCM_LTR0_UPTHR_LSH                      22
#define CCM_LTR0_SIGD13_LSH                     29
#define CCM_LTR0_SIGD14_LSH                     30
#define CCM_LTR0_SIGD15_LSH                     31

#define CCM_LTR1_PNCTHR_LSH                     0
#define CCM_LTR1_UPCNT_LSH                      6
#define CCM_LTR1_DNCNT_LSH                      14
#define CCM_LTR1_LTBRSR_LSH                     22
#define CCM_LTR1_LTBRSH_LSH                     23

#define CCM_LTR2_EMAC_LSH                       0
#define CCM_LTR2_WSW9_LSH                       11

#define CCM_LTR3_WSW0_LSH                       5

#define CCM_PMCR0_DPTEN_LSH                     0
#define CCM_PMCR0_PTVAI_LSH                     1
#define CCM_PMCR0_PTVAIM_LSH                    3
#define CCM_PMCR0_DVFEN_LSH                     4
#define CCM_PMCR0_SCR_LSH                       5
#define CCM_PMCR0_DRCE0_LSH                     6
#define CCM_PMCR0_DRCE1_LSH                     7
#define CCM_PMCR0_DRCE2_LSH                     8
#define CCM_PMCR0_DRCE3_LSH                     9
#define CCM_PMCR0_WFIM_LSH                      10
#define CCM_PMCR0_DPVV_LSH                      11
#define CCM_PMCR0_DPVCR_LSH                     12
#define CCM_PMCR0_FSVAI_LSH                     13
#define CCM_PMCR0_FSVAIM_LSH                    15
#define CCM_PMCR0_DVFS_START_LSH                16
#define CCM_PMCR0_PTVIS_LSH                     17
#define CCM_PMCR0_LBCF_LSH                      18
#define CCM_PMCR0_LBFL_LSH                      20
#define CCM_PMCR0_LBMI_LSH                      21
#define CCM_PMCR0_DVFIS_LSH                     22
#define CCM_PMCR0_DVFEV_LSH                     23
#define CCM_PMCR0_DVFS_UPD_FINISH_LSH           24
#define CCM_PMCR0_DVSUP_LSH                     28
#define CCM_PMCR0_DVS1_LSH                      28
#define CCM_PMCR0_DVS0_LSH                      29

#define CCM_PMCR1_DVGP_LSH                      0
#define CCM_PMCR1_CPFA_LSH                      6
#define CCM_PMCR1_CPSPA_LSH                     9
#define CCM_PMCR1_WBCN_LSH                      16
#define CCM_PMCR1_CPSPA_EMI_LSH                 24
#define CCM_PMCR1_CPFA_EMI_LSH                  28

#define CCM_PMCR2_DVFS_ACK_LSH                  0
#define CCM_PMCR2_DVFS_REQ_LSH                  1
#define CCM_PMCR2_OSC24M_DOWN_LSH               16
#define CCM_PMCR2_OSC_AUDIO_DOWN_LSH            17
#define CCM_PMCR2_IPU_GAS_LSH                   18
#define CCM_PMCR2_OSC_RDY_CNT_LSH               23


//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
#define CCM_CCMR_MPE_WID                        1
#define CCM_CCMR_UPE_WID                        1
#define CCM_CCMR_LPM_WID                        2
#define CCM_CCMR_RAMW_WID                       2
#define CCM_CCMR_ROMW_WID                       2
#define CCM_CCMR_VOL_RDY_CNT_WID                4
#define CCM_CCMR_WBEN_WID                       1
#define CCM_CCMR_VSTBY_WID                      1
#define CCM_CCMR_STBY_EXIT_SRC_WID              1
#define CCM_CCMR_WFI_WID                        1

#define CCM_PDR0_AUTO_CON_WID                   1
#define CCM_PDR0_AUTO_MUX_DIV_WID               3
#define CCM_PDR0_CCM_PER_AHB_WID                3
#define CCM_PDR0_CKIL_SEL_WID                   1
#define CCM_PDR0_CON_MUX_DIV_WID                4
#define CCM_PDR0_HSP_PODF_WID                   2
#define CCM_PDR0_IPU_HND_BYP_WID                1
#define CCM_PDR0_PER_SEL_WID                    1

#define CCM_PDR1_MSHC_M_U_WID                   1
#define CCM_PDR1_MSHC_DIV_WID                   6
#define CCM_PDR1_MSHC_DIV_PRE_WID               3

#define CCM_PDR2_SSI1_DIV_WID                   6
#define CCM_PDR2_SSI_M_U_WID                    1
#define CCM_PDR2_CSI_M_U_WID                    1
#define CCM_PDR2_SSI2_DIV_WID                   6
#define CCM_PDR2_CSI_DIV_WID                    6
#define CCM_PDR2_CSI_DIV_PST_WID                3
#define CCM_PDR2_CSI_DIV_PRE_WID                3
#define CCM_PDR2_SSI1_DIV_PRE_WID               3
#define CCM_PDR2_SSI2_DIV_PRE_WID               3

#define CCM_PDR3_ESDHC1_DIV_WID                 6
#define CCM_PDR3_ESDHC1_DIV_PST_WID             3
#define CCM_PDR3_ESDHC1_DIV_PRE_WID             3
#define CCM_PDR3_ESDHC_M_U_WID                  1
#define CCM_PDR3_ESDHC2_DIV_WID                 6
#define CCM_PDR3_ESDHC2_DIV_PST_WID             3
#define CCM_PDR3_ESDHC2_DIV_PRE_WID             3
#define CCM_PDR3_UART_M_U_WID                   1
#define CCM_PDR3_ESDHC3_DIV_WID                 6
#define CCM_PDR3_ESDHC3_DIV_PST_WID             3
#define CCM_PDR3_ESDHC3_DIV_PRE_WID             3
#define CCM_PDR3_SPDIF_M_U_WID                  1
#define CCM_PDR3_SPDIF_DIV_WID                  6
#define CCM_PDR3_SPDIF_DIV_PRE_WID              3

#define CCM_PDR4_USB_M_U_WID                    1
#define CCM_PDR4_UART_DIV_WID                   6
#define CCM_PDR4_UART_DIV_PST_WID               3
#define CCM_PDR4_UART_DIV_PRE_WID               3
#define CCM_PDR4_PER0_DIV_WID                   6
#define CCM_PDR4_PER0_DIV_PST_WID               3
#define CCM_PDR4_PER0_DIV_PRE_WID               3
#define CCM_PDR4_USB_DIV_WID                    6
#define CCM_PDR4_USB_DIV_PST_WID                3
#define CCM_PDR4_USB_DIV_PRE_WID                3
#define CCM_PDR4_NFC_DIV_WID                    4

#define CCM_RCSR_REST_WID                       4
#define CCM_RCSR_GPF_WID                        4
#define CCM_RCSR_NFC_FMS_WID                    1
#define CCM_RCSR_NFC_4K_WID                     1
#define CCM_RCSR_BOOT_REG_WID                   2
#define CCM_RCSR_NFC_16BIT_SEL0_WID             1
#define CCM_RCSR_PERES_WID                      1
#define CCM_RCSR_BT_ECC_WID                     1
#define CCM_RCSR_MEM_TYPE_WID                   2
#define CCM_RCSR_MEM_CTRL_WID                   2
#define CCM_RCSR_PAGE_SIZE_WID                  2
#define CCM_RCSR_BUS_WIDTH_WID                  1
#define CCM_RCSR_BT_USB_SRC_WID                 1

#define CCM_MPCTL_MFN_WID                       10
#define CCM_MPCTL_MFI_WID                       4
#define CCM_MPCTL_MFD_WID                       10
#define CCM_MPCTL_PDF_WID                       4
#define CCM_MPCTL_BRMO_WID                      1

#define CCM_PPCTL_MFN_WID                       10
#define CCM_PPCTL_MFI_WID                       4
#define CCM_PPCTL_MFD_WID                       10
#define CCM_PPCTL_PDF_WID                       4
#define CCM_PPCTL_BRMO_WID                      1

#define CCM_ACMR_SSI2_AUDIO_CLK_SEL_WID         4
#define CCM_ACMR_SSI1_AUDIO_CLK_SEL_WID         4
#define CCM_ACMR_SPDIF_AUDIO_CLK_SEL_WID        4
#define CCM_ACMR_ESAI_AUDIO_CLK_SEL_WID         4

#define CCM_COSR_CLKOSEL_WID                    5
#define CCM_COSR_CLKOEN_WID                     1
#define CCM_COSR_CLKO_DIV1_WID                  1
#define CCM_COSR_CLKO_DIV_WID                   3
#define CCM_COSR_CLKO_DIV_PRE_WID               3
#define CCM_COSR_SSI1_RX_SRC_SEL_WID            2
#define CCM_COSR_SSI1_TX_SRC_SEL_WID            2
#define CCM_COSR_SSI2_RX_SRC_SEL_WID            2
#define CCM_COSR_SSI2_TX_SRC_SEL_WID            2
#define CCM_COSR_ASRC_AUDIO_EN_WID              1
#define CCM_COSR_ASRC_AUDIO_PODF_WID            6

#define CCM_CGR_CG_WID                          2

#define CCM_DCVR_ELV_WID                        10
#define CCM_DCVR_LLV_WID                        10
#define CCM_DCVR_ULV_WID                        10

#define CCM_LTR0_SIGD_WID                       16
#define CCM_LTR0_DNTHR_WID                      6
#define CCM_LTR0_UPTHR_WID                      6
#define CCM_LTR0_DIV3CK_WID                     2

#define CCM_LTR1_PNCTHR_WID                     6
#define CCM_LTR1_UPCNT_WID                      8
#define CCM_LTR1_DNCNT_WID                      8
#define CCM_LTR1_LTBRSR_WID                     1
#define CCM_LTR1_LTBRSH_WID                     1

#define CCM_LTR2_EMAC_WID                       9

#define CCM_LTR_WSW_WID                         3

#define CCM_PMCR0_DPTEN_WID                     1
#define CCM_PMCR0_PTVAI_WID                     2
#define CCM_PMCR0_PTVAIM_WID                    1
#define CCM_PMCR0_DVFEN_WID                     1
#define CCM_PMCR0_SCR_WID                       1
#define CCM_PMCR0_DRCE0_WID                     1
#define CCM_PMCR0_DRCE1_WID                     1
#define CCM_PMCR0_DRCE2_WID                     1
#define CCM_PMCR0_DRCE3_WID                     1
#define CCM_PMCR0_WFIM_WID                      1
#define CCM_PMCR0_DPVV_WID                      1
#define CCM_PMCR0_DPVCR_WID                     1
#define CCM_PMCR0_FSVAI_WID                     2
#define CCM_PMCR0_FSVAIM_WID                    1
#define CCM_PMCR0_DVFS_START_WID                1
#define CCM_PMCR0_PTVIS_WID                     1
#define CCM_PMCR0_LBCF_WID                      2
#define CCM_PMCR0_LBFL_WID                      1
#define CCM_PMCR0_LBMI_WID                      1
#define CCM_PMCR0_DVFIS_WID                     1
#define CCM_PMCR0_DVFEV_WID                     1
#define CCM_PMCR0_DVFS_UPD_FINISH_WID           1
#define CCM_PMCR0_DVSUP_WID                     2
#define CCM_PMCR0_DVS1_WID                      1
#define CCM_PMCR0_DVS0_WID                      1

#define CCM_PMCR1_DVGP_WID                      4
#define CCM_PMCR1_CPFA_WID                      1
#define CCM_PMCR1_CPSPA_WID                     4
#define CCM_PMCR1_WBCN_WID                      8
#define CCM_PMCR1_CPSPA_EMI_WID                 4
#define CCM_PMCR1_CPFA_EMI_WID                  1

#define CCM_PMCR2_DVFS_ACK_WID                  1
#define CCM_PMCR2_DVFS_REQ_WID                  1
#define CCM_PMCR2_OSC24M_DOWN_WID               1
#define CCM_PMCR2_OSC_AUDIO_DOWN_WID            1
#define CCM_PMCR2_IPU_GAS_WID                   1
#define CCM_PMCR2_OSC_RDY_CNT_WID               9


//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------

// CCMR
#define CCM_CCMR_MPE_DISABLED                   0
#define CCM_CCMR_MPE_ENABLED                    1

#define CCM_CCMR_UPE_DISABLED                   0
#define CCM_CCMR_UPE_ENABLED                    1

#define CCM_CCMR_LPM_RUN                        0
#define CCM_CCMR_LPM_WAIT                       1
#define CCM_CCMR_LPM_DOZE                       2
#define CCM_CCMR_LPM_SLEEP                      3

#define CCM_CCMR_WBEN_DISABLE                   0
#define CCM_CCMR_WBEN_ENABLE                    1

#define CCM_CCMR_VSTBY_DISABLE                  0
#define CCM_CCMR_VSTBY_ENABLE                   1

#define CCM_CCMR_STBY_EXIT_SRC_SIGNAL           0
#define CCM_CCMR_STBY_EXIT_SRC_COUNTER          1

// PDR0
#define CCM_PDR0_AUTO_MUX_DIV_1_3_ARMSEL_0      0
#define CCM_PDR0_AUTO_MUX_DIV_1_2_ARMSEL_1      1
#define CCM_PDR0_AUTO_MUX_DIV_2_1_ARMSEL_1      2
#define CCM_PDR0_AUTO_MUX_DIV_1_6_ARMSEL_0      4
#define CCM_PDR0_AUTO_MUX_DIV_1_4_ARMSEL_1      5
#define CCM_PDR0_AUTO_MUX_DIV_2_2_ARMSEL_1      6

#define CCM_PDR0_CKIL_SEL_24MDIV                0
#define CCM_PDR0_CKIL_SEL_MUXED                 1

#define CCM_PDR0_CON_MUX_DIV_1_4_ARMSEL_0       0
#define CCM_PDR0_CON_MUX_DIV_1_3_ARMSEL_1       1
#define CCM_PDR0_CON_MUX_DIV_2_2_ARMSEL_0       2
#define CCM_PDR0_CON_MUX_DIV_4_1_ARMSEL_0       6
#define CCM_PDR0_CON_MUX_DIV_1_5_ARMSEL_0       7
#define CCM_PDR0_CON_MUX_DIV_1_8_ARMSEL_0       8
#define CCM_PDR0_CON_MUX_DIV_1_6_ARMSEL_1       9
#define CCM_PDR0_CON_MUX_DIV_2_4_ARMSEL_0       10
#define CCM_PDR0_CON_MUX_DIV_4_2_ARMSEL_0       14

#define CCM_PDR0_HSP_PODF_133                   0
#define CCM_PDR0_HSP_PODF_66P5                  1
#define CCM_PDR0_HSP_PODF_178                   2

#define CCM_PDR0_IPU_HND_BYP_ENABLE             0
#define CCM_PDR0_IPU_HND_BYP_DISABLE            1
#define CCM_PDR0_IPU_HND_BYP_ENABLE_TO1         1
#define CCM_PDR0_IPU_HND_BYP_DISABLE_TO1        0

#define CCM_PDR0_PER_SEL_AHBDIV                 0
#define CCM_PDR0_PER_SEL_ARMAHBSYNC             1

// RCSR
#define CCM_RCSR_NFC_FMS_NOT2K                  0
#define CCM_RCSR_NFC_FMS_2K                     1

#define CCM_RCSR_NFC_4K_NOT4K                   0
#define CCM_RCSR_NFC_4K_4K                      1

#define CCM_RCSR_MEM_CTRL_WEIM                  0
#define CCM_RCSR_MEM_CTRL_NAND                  1
#define CCM_RCSR_MEM_CTRL_SDMMC                 3

// PMCR0
#define CCM_PMCR0_DPTEN_DPTC_DISABLE            0
#define CCM_PMCR0_DPTEN_DPTC_ENABLE             1

#define CCM_PMCR0_PTVAI_VOLT_NO_CHANGE          0
#define CCM_PMCR0_PTVAI_VOLT_DOWN               1
#define CCM_PMCR0_PTVAI_VOLT_UP                 2
#define CCM_PMCR0_PTVAI_VOLT_UP_PANIC           3

#define CCM_PMCR0_PTVAIM_UNMASK                 0
#define CCM_PMCR0_PTVAIM_MASK                   1

#define CCM_PMCR0_DVFEN_DVFS_DISABLE            0
#define CCM_PMCR0_DVFEN_DVFS_ENABLE             1

#define CCM_PMCR0_SCR_256_CLOCKS                0
#define CCM_PMCR0_SCR_512_CLOCKS                1

#define CCM_PMCR0_DRCE_DISABLE                  0
#define CCM_PMCR0_DRCE_ENABLE                   1

#define CCM_PMCR0_WFIM_UNMASK                   0
#define CCM_PMCR0_WFIM_MASK                     1

#define CCM_PMCR0_DPVV_VOLT_INVALID             0
#define CCM_PMCR0_DPVV_VOLT_VALID               1

#define CCM_PMCR0_DPVCR_NO_CHANGE_REQ           0
#define CCM_PMCR0_DPVCR_CHANGE_REQ              1

#define CCM_PMCR0_FSVAI_FREQ_NO_CHANGE          0
#define CCM_PMCR0_FSVAI_FREQ_UP                 1
#define CCM_PMCR0_FSVAI_FREQ_DOWN               2
#define CCM_PMCR0_FSVAI_FREQ_UP_PANIC           3

#define CCM_PMCR0_FSVAIM_UNMASK                 0
#define CCM_PMCR0_FSVAIM_MASK                   1

#define CCM_PMCR0_DVFS_START_NOT                0
#define CCM_PMCR0_DVFS_START_UPDATING           1

#define CCM_PMCR0_PTVIS_SDMA_DPTC_REQ           0
#define CCM_PMCR0_PTVIS_MCU_DPTC_REQ            1

#define CCM_PMCR0_LBCF_BUF_SIZE_4               0
#define CCM_PMCR0_LBCF_BUF_SIZE_8               1
#define CCM_PMCR0_LBCF_BUF_SIZE_12              2
#define CCM_PMCR0_LBCF_BUF_SIZE_16              3

#define CCM_PMCR0_LBFL_BUF_NOT_FULL             0
#define CCM_PMCR0_LBFL_BUF_FULL                 1

#define CCM_PMCR0_LBMI_UNMASKED                 0
#define CCM_PMCR0_LBMI_MASKED                   1

#define CCM_PMCR0_DVFIS_SDMA_DVFS_REQ           0
#define CCM_PMCR0_DVFIS_MCU_DVFS_REQ            1

#define CCM_PMCR0_DVFEV_REQ_NOT_ALWAYS          0
#define CCM_PMCR0_DVFEV_REQ_ALWAYS              1

#define CCM_PMCR0_DVFS_UPD_FINISH_NOT           0
#define CCM_PMCR0_DVFS_UPD_FINISH_FINISHED      1


//------------------------------------------------------------------------------
// HELPER MACROS
//------------------------------------------------------------------------------
#define CCM_CGR_INDEX(gateIndex)        ((gateIndex) >> 4)
#define CCM_CGR_SHIFT(gateIndex)        (((gateIndex) % 16) << 1)
#define CCM_CGR_MASK(gateIndex)         (CCM_CGR_CG_MASK <<  CCM_CGR_SHIFT(gateIndex))
#define CCM_CGR_VAL(gateIndex, val)     (val << CCM_CGR_SHIFT(gateIndex))

// CGR Masks
#define CCM_CGR0_ASRC_MASK              CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_ASRC)
#define CCM_CGR0_ATA_MASK               CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_ATA)
#define CCM_CGR0_CAN1_MASK              CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_CAN1)
#define CCM_CGR0_CAN2_MASK              CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_CAN2)
#define CCM_CGR0_CSPI1_MASK             CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_CSPI1)
#define CCM_CGR0_CSPI2_MASK             CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_CSPI2)
#define CCM_CGR0_ECT_MASK               CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_ECT)
#define CCM_CGR0_EMI_MASK               CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_EMI)
#define CCM_CGR0_EPIT1_MASK             CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_EPIT1)
#define CCM_CGR0_EPIT2_MASK             CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_EPIT2)
#define CCM_CGR0_ESAI_MASK              CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_ESAI)
#define CCM_CGR0_ESDHC1_MASK            CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_ESDHC1)
#define CCM_CGR0_ESDHC2_MASK            CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_ESDHC2)
#define CCM_CGR0_ESDHC3_MASK            CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_ESDHC3)

#define CCM_CGR1_FEC_MASK               CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_FEC)
#define CCM_CGR1_GPIO1_MASK             CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_GPIO1)
#define CCM_CGR1_GPIO2_MASK             CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_GPIO2)
#define CCM_CGR1_GPIO3_MASK             CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_GPIO3)
#define CCM_CGR1_GPT_MASK               CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_GPT)
#define CCM_CGR1_I2C1_MASK              CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_I2C1)
#define CCM_CGR1_I2C2_MASK              CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_I2C2)
#define CCM_CGR1_I2C3_MASK              CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_I2C3)
#define CCM_CGR1_IOMUXC_MASK            CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_IOMUXC)
#define CCM_CGR1_IPU_MASK               CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_IPU)
#define CCM_CGR1_KPP_MASK               CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_KPP)
#define CCM_CGR1_MLB_MASK               CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_MLB)
#define CCM_CGR1_MSHC_MASK              CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_MSHC)
#define CCM_CGR1_OWIRE_MASK             CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_OWIRE)
#define CCM_CGR1_PWM_MASK               CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_PWM)
#define CCM_CGR1_RNGC_MASK              CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_RNGC)

#define CCM_CGR2_RTC_MASK               CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_RTC)
#define CCM_CGR2_RTIC_MASK              CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_RTIC)
#define CCM_CGR2_SCC_MASK               CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_SCC)
#define CCM_CGR2_SDMA_MASK              CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_SDMA)
#define CCM_CGR2_SPBA_MASK              CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_SPBA)
#define CCM_CGR2_SPDIF_MASK             CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_SPDIF)
#define CCM_CGR2_SSI1_MASK              CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_SSI1)
#define CCM_CGR2_SSI2_MASK              CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_SSI2)
#define CCM_CGR2_UART1_MASK             CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_UART1)
#define CCM_CGR2_UART2_MASK             CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_UART2)
#define CCM_CGR2_UART3_MASK             CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_UART3)
#define CCM_CGR2_USBOTG_MASK            CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_USBOTG)
#define CCM_CGR2_WDOG_MASK              CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_WDOG)
#define CCM_CGR2_MAX_MASK               CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_MAX)
#define CCM_CGR2_AUDMUX_MASK            CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_AUDMUX)

#define CCM_CGR3_CSI_MASK               CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_CSI)
#define CCM_CGR3_IIM_MASK               CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_IIM)
#define CCM_CGR3_GPU2D_MASK             CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_GPU2D)


#ifdef __cplusplus
}
#endif

#endif // __MX35_CCM_H__
