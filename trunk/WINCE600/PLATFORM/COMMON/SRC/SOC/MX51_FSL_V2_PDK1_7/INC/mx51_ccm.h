//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header:  mx51_ccm.h
//
//  Provides definitions for the CCM (Clock Controller Module).
//
//------------------------------------------------------------------------------

#ifndef __MX51_CCM_H
#define __MX51_CCM_H

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------
#define CCM_CGR_CG_MASK                 (0x3)
#define CCM_NUM_CCGR                    7

//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------
typedef struct
{
    UINT32 CCR;         // 0x0000
    UINT32 CCDR;        // 0x0004
    UINT32 CSR;         // 0x0008
    UINT32 CCSR;        // 0x000C
    UINT32 CACRR;       // 0x0010
    UINT32 CBCDR;       // 0x0014
    UINT32 CBCMR;       // 0x0018
    UINT32 CSCMR1;      // 0x001C
    UINT32 CSCMR2;      // 0x0020
    UINT32 CSCDR1;      // 0x0024
    UINT32 CS1CDR;      // 0x0028
    UINT32 CS2CDR;      // 0x002C
    UINT32 CDCDR;       // 0x0030
    UINT32 CHSCCDR;     // 0x0034
    UINT32 CSCDR2;      // 0x0038
    UINT32 CSCDR3;      // 0x003C
    UINT32 CSCDR4;      // 0x0040
    UINT32 CWDR;        // 0x0044
    UINT32 CDHIPR;      // 0x0048
    UINT32 CDCR;        // 0x004C
    UINT32 CTOR;        // 0x0050
    UINT32 CLPCR;       // 0x0054
    UINT32 CISR;        // 0x0058
    UINT32 CIMR;        // 0x005C
    UINT32 CCOSR;       // 0x0060
    UINT32 CGPR;        // 0x0064
    UINT32 CCGR[CCM_NUM_CCGR]; // 0x0068..0x0080
    UINT32 CMEOR;       // 0x0084
} CSP_CCM_REGS, *PCSP_CCM_REGS;

//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
#define CCM_CCR_OFFSET      0x0000
#define CCM_CCDR_OFFSET     0x0004
#define CCM_CSR_OFFSET      0x0008
#define CCM_CCSR_OFFSET     0x000C
#define CCM_CACRR_OFFSET    0x0010
#define CCM_CBCDR_OFFSET    0x0014
#define CCM_CBCMR_OFFSET    0x0018
#define CCM_CSCMR1_OFFSET   0x001C
#define CCM_CSCMR2_OFFSET   0x0020
#define CCM_CSCDR1_OFFSET   0x0024
#define CCM_CS1CDR_OFFSET   0x0028
#define CCM_CS2CDR_OFFSET   0x002C
#define CCM_CDCDR_OFFSET    0x0030
#define CCM_CHSCCDR_OFFSET  0x0034
#define CCM_CSCDR2_OFFSET   0x0038
#define CCM_CSCDR3_OFFSET   0x003C
#define CCM_CSCDR4_OFFSET   0x0040
#define CCM_CR2_OFFSET      0x0044
#define CCM_CDHIPR_OFFSET   0x0048
#define CCM_CDCR_OFFSET     0x004C
#define CCM_CTOR_OFFSET     0x0050
#define CCM_CLPCR_OFFSET    0x0054
#define CCM_CISR_OFFSET     0x0058
#define CCM_CIMR_OFFSET     0x005C
#define CCM_CCOSR_OFFSET    0x0060
#define CCM_CGPR_OFFSET     0x0064
#define CCM_CCGR_OFFSET     0x0068
#define CCM_CMEOR_OFFSET    0x0084

//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
#define CCM_CCR_OSCNT_LSH                       0
#define CCM_CCR_FPM_EN_LSH                      8
#define CCM_CCR_CAMP1_EN_LSH                    9
#define CCM_CCR_CAMP2_EN_LSH                    10
#define CCM_CCR_FPM_MULT_LSH                    11
#define CCM_CCR_COSC_EN_LSH                     12

#define CCM_CCDR_EMI_HS_MASK_LSH                16
#define CCM_CCDR_IPU_HS_MASK_LSH                17
#define CCM_CCDR_HSC_HS_MASK_LSH                18

#define CCM_CSR_REF_EN_B_LSH                    0
#define CCM_CSR_FPM_READY_LSH                   1
#define CCM_CSR_CAMP1_READY_LSH                 2
#define CCM_CSR_CAMP2_READY_LSH                 3
#define CCM_CSR_LVS_VALUE_LSH                   4
#define CCM_CSR_COSC_READY_LSH                  5

#define CCM_CCSR_PLL3_SW_CLK_SEL_LSH            0
#define CCM_CCSR_PLL2_SW_CLK_SEL_LSH            1
#define CCM_CCSR_PLL1_SW_CLK_SEL_LSH            2
#define CCM_CCSR_PLL3_DIV_PODF_LSH              3
#define CCM_CCSR_PLL2_DIV_PODF_LSH              5
#define CCM_CCSR_STEP_SEL_LSH                   7
#define CCM_CCSR_LP_APM_LSH                     9

#define CCM_CACRR_ARM_PODF_LSH                  0

#define CCM_CBCDR_PERCLK_PODF_LSH               0
#define CCM_CBCDR_PERCLK_PRED2_LSH              3
#define CCM_CBCDR_PERCLK_PRED1_LSH              6
#define CCM_CBCDR_IPG_PODF_LSH                  8
#define CCM_CBCDR_AHB_PODF_LSH                  10
#define CCM_CBCDR_NFC_PODF_LSH                  13
#define CCM_CBCDR_AXI_A_PODF_LSH                16
#define CCM_CBCDR_AXI_B_PODF_LSH                19
#define CCM_CBCDR_EMI_SLOW_PODF_LSH             22
#define CCM_CBCDR_PERIPH_CLK_SEL_LSH            25
#define CCM_CBCDR_EMI_CLK_SEL_LSH               26
#define CCM_CBCDR_DDR_CLK_PODF_LSH              27
#define CCM_CBCDR_DDR_HIGH_FREQ_CLK_SEL_LSH     30

#define CCM_CBCMR_PERCLK_IPG_SEL_LSH            0
#define CCM_CBCMR_PERCLK_LP_APM_SEL_LSH         1
#define CCM_CBCMR_DEBUG_APB_CLK_SEL_LSH         2
#define CCM_CBCMR_GPU_CLK_SEL_LSH               4
#define CCM_CBCMR_IPU_HSP_CLK_SEL_LSH           6
#define CCM_CBCMR_ARM_AXI_CLK_SEL_LSH           8
#define CCM_CBCMR_DDR_CLK_SEL_LSH               10
#define CCM_CBCMR_PERIPH_APM_SEL_LSH            12
#define CCM_CBCMR_VPU_AXI_SEL_LSH               14
#define CCM_CBCMR_GPU2D_CLK_SEL_LSH             16

#define CCM_CSCMR1_SSI_EXT1_COM_LSH             0
#define CCM_CSCMR1_SSI_EXT2_COM_LSH             1
#define CCM_CSCMR1_SPDIF_XTAL_CLK_SEL_LSH       2
#define CCM_CSCMR1_ECSPI_CLK_SEL_LSH            4
#define CCM_CSCMR1_TVE_EXT_CLK_SEL_LSH          6
#define CCM_CSCMR1_TVE_CLK_SEL_LSH              7
#define CCM_CSCMR1_SSI_APM_CLK_SEL_LSH          8
#define CCM_CSCMR1_VPU_RCLK_ROOT_LSH            10
#define CCM_CSCMR1_SSI3_CLK_SEL_LSH             11
#define CCM_CSCMR1_SSI2_CLK_SEL_LSH             12
#define CCM_CSCMR1_SSI1_CLK_SEL_LSH             14
#define CCM_CSCMR1_ESDHC2_CLK_SEL_LSH           16
#define CCM_CSCMR1_ESDHC4_CLK_SEL_LSH           18
#define CCM_CSCMR1_ESDHC3_CLK_SEL_LSH           19
#define CCM_CSCMR1_ESDHC1_CLK_SEL_LSH           20
#define CCM_CSCMR1_USBOH3_CLK_SEL_LSH           22
#define CCM_CSCMR1_UART_CLK_SEL_LSH             24
#define CCM_CSCMR1_USB_PHY_CLK_SEL_LSH          26
#define CCM_CSCMR1_SSI_EXT1_CLK_SEL_LSH         28
#define CCM_CSCMR1_SSI_EXT2_CLK_SEL_LSH         30

#define CCM_CSCMR2_SPDIF0_CLK_SEL_LSH           0
#define CCM_CSCMR2_SPDIF1_CLK_SEL_LSH           2
#define CCM_CSCMR2_SPDIF0_COM_LSH               4
#define CCM_CSCMR2_SPDIF1_COM_LSH               5
#define CCM_CSCMR2_SLIMBUS_CLK_SEL_LSH          6
#define CCM_CSCMR2_SLIMBUS_COM_LSH              9
#define CCM_CSCMR2_SIM_CLK_SEL_LSH              10
#define CCM_CSCMR2_FIRI_CLK_SEL_LSH             12
#define CCM_CSCMR2_HSI2C_CLK_SEL_LSH            14
#define CCM_CSCMR2_HSC1_CLK_SEL_LSH             16
#define CCM_CSCMR2_HSC2_CLK_SEL_LSH             18
#define CCM_CSCMR2_ESC_CLK_SEL_LSH              20
#define CCM_CSCMR2_CSI_MCLK1_CLK_SEL_LSH        22
#define CCM_CSCMR2_CSI_MCLK2_CLK_SEL_LSH        24 
#define CCM_CSCMR2_DI_CLK_SEL_LSH               26
#define CCM_CSCMR2_DI0_CLK_SEL_LSH              26
#define CCM_CSCMR2_DI1_CLK_SEL_LSH              29

#define CCM_CSCDR1_UART_CLK_PODF_LSH            0
#define CCM_CSCDR1_UART_CLK_PRED_LSH            3
#define CCM_CSCDR1_USBOH3_CLK_PODF_LSH          6
#define CCM_CSCDR1_USBOH3_CLK_PRED_LSH          8
#define CCM_CSCDR1_ESDHC1_CLK_PODF_LSH          11
#define CCM_CSCDR1_PGC_CLK_PODF_LSH             14
#define CCM_CSCDR1_ESDHC1_CLK_PRED_LSH          16
#define CCM_CSCDR1_ESDHC2_CLK_PODF_LSH          19
#define CCM_CSCDR1_ESDHC2_CLK_PRED_LSH          22

#define CCM_CS1CDR_SSI1_CLK_PODF_LSH            0
#define CCM_CS1CDR_SSI1_CLK_PRED_LSH            6
#define CCM_CS1CDR_SSI_EXT1_CLK_PODF_LSH        16
#define CCM_CS1CDR_SSI_EXT1_CLK_PRED_LSH        22

#define CCM_CS2CDR_SSI2_CLK_PODF_LSH            0
#define CCM_CS2CDR_SSI2_CLK_PRED_LSH            6
#define CCM_CS2CDR_SSI_EXT2_CLK_PODF_LSH        16
#define CCM_CS2CDR_SSI_EXT2_CLK_PRED_LSH        22

#define CCM_CDCDR_USB_PHY_PODF_LSH              0
#define CCM_CDCDR_USB_PHY_PRED_LSH              3
#define CCM_CDCDR_DI_CLK_PRED_LSH               6
#define CCM_CDCDR_SPDIF1_CLK_PODF_LSH           9
#define CCM_CDCDR_SPDIF1_CLK_PRED_LSH           16
#define CCM_CDCDR_SPDIF0_CLK_PODF_LSH           19
#define CCM_CDCDR_SPDIF0_CLK_PRED_LSH           25
#define CCM_CDCDR_TVE_CLK_PRED_LSH              28

#define CCM_CHSCCDR_HSC1_CLK_PODF_LSH           0
#define CCM_CHSCCDR_HSC2_CLK_PODF_LSH           3
#define CCM_CHSCCDR_ESC_CLK_PODF_LSH            6
#define CCM_CHSCCDR_ESC_CLK_PRED_LSH            12

#define CCM_CSCDR2_SLIMBUS_CLK_PODF_LSH         0
#define CCM_CSCDR2_SLIMBUS_CLK_PRED_LSH         6
#define CCM_CSCDR2_SIM_CLK_PODF_LSH             9
#define CCM_CSCDR2_SIM_CLK_PRED_LSH             16
#define CCM_CSCDR2_ECSPI_CLK_PODF_LSH           19
#define CCM_CSCDR2_ECSPI_CLK_PRED_LSH           25

#define CCM_CSCDR3_FIRI_CLK_PODF_LSH            0
#define CCM_CSCDR3_FIRI_CLK_PRED_LSH            6
#define CCM_CSCDR3_HSI2C_CLK_PODF_LSH           9
#define CCM_CSCDR3_HSI2C_CLK_PRED_LSH           16

#define CCM_CSCDR4_CSI_MCLK1_CLK_PODF_LSH       0
#define CCM_CSCDR4_CSI_MCLK1_CLK_PRED_LSH       6
#define CCM_CSCDR4_CSI_MCLK2_CLK_PODF_LSH       9
#define CCM_CSCDR4_CSI_MCLK2_CLK_PRED_LSH       16

#define CCM_CWDR_GPIO1_4_ICR_LSH                0
#define CCM_CWDR_GPIO1_5_ICR_LSH                2
#define CCM_CWDR_GPIO1_6_ICR_LSH                4
#define CCM_CWDR_GPIO1_7_ICR_LSH                6
#define CCM_CWDR_GPIO1_8_ICR_LSH                8
#define CCM_CWDR_GPIO1_9_ICR_LSH                10
#define CCM_CWDR_GPIO1_4_DIR_LSH                16
#define CCM_CWDR_GPIO1_5_DIR_LSH                17
#define CCM_CWDR_GPIO1_6_DIR_LSH                18
#define CCM_CWDR_GPIO1_7_DIR_LSH                19
#define CCM_CWDR_GPIO1_8_DIR_LSH                20
#define CCM_CWDR_GPIO1_9_DIR_LSH                21

#define CCM_CDHIPR_AXI_A_PODF_BUSY_LSH          0
#define CCM_CDHIPR_AXI_B_PODF_BUSY_LSH          1
#define CCM_CDHIPR_EMI_SLOW_PODF_BUSY_LSH       2
#define CCM_CDHIPR_AHB_PODF_BUSY_LSH            3
#define CCM_CDHIPR_NFC_PODF_BUSY_LSH            4
#define CCM_CDHIPR_PERIPH_CLK_SEL_BUSY_LSH      5
#define CCM_CDHIPR_EMI_CLK_SEL_BUSY_LSH         6
#define CCM_CDHIPR_DDR_PODF_BUSY_LSH            7
#define CCM_CDHIPR_DDR_HF_CLK_SEL_BUSY_LSH      8
#define CCM_CDHIPR_ARM_PODF_BUSY_LSH            16

#define CCM_CDCR_PERIPH_CLK_DVFS_PODF_LSH       0
#define CCM_CDCR_ARM_FREQ_SHIFT_DIVIDER_LSH     2
#define CCM_CDCR_HSC1_DVFS_EN_LSH               3
#define CCM_CDCR_HSC2_DVFS_EN_LSH               4
#define CCM_CDCR_SOFTWARE_DVFS_EN_LSH           5
#define CCM_CDCR_SW_PERIPH_CLK_DIV_REQ_LSH      6
#define CCM_CDCR_SW_PERIPH_CLK_DIV_REQ_STATUS_LSH  7 

#define CCM_CTOR_OBS_SPARE_OUTPUT_2_SEL_LSH     0
#define CCM_CTOR_OBS_SPARE_OUTPUT_1_SLE_LSH     4
#define CCM_CTOR_OBS_SPARE_OUTPUT_0_SLE_LSH     8
#define CCM_CTOR_OBS_EN_LSH                     13

#define CCM_CLPCR_LPM_LSH                       0
#define CCM_CLPCR_BYPASS_PMIC_READY_LSH         2
#define CCM_CLPCR_LPSR_CLK_SEL_LSH              3
#define CCM_CLPCR_ARM_CLK_DIS_ON_LPM_LSH        5
#define CCM_CLPCR_SBYOS_LSH                     6
#define CCM_CLPCR_DIS_REF_OSC_LSH               7
#define CCM_CLPCR_VSTBY_LSH                     8
#define CCM_CLPCR_STBY_COUNT_LSH                9
#define CCM_CLPCR_COSC_PWRDOWN_LSH              11
#define CCM_CLPCR_BYPASS_SAHARA_LPM_HS_LSH      16
#define CCM_CLPCR_BYPASS_RTIC_LPM_HS_LSH        17
#define CCM_CLPCR_BYPASS_IPU_LPM_HS_LSH         18
#define CCM_CLPCR_BYPASS_EMI_LPM_HS_LSH         19
#define CCM_CLPCR_BYPASS_SDMA_LPM_HS_LSH        20
#define CCM_CLPCR_BYPASS_MAX_LPM_HS_LSH         21
#define CCM_CLPCR_BYPASS_SCC_LPM_HS_LSH         22
#define CCM_CLPCR_BYPASS_HSC_LPM_HS_LSH         23

#define CCM_CISR_LRF_PLL1_LSH                   0
#define CCM_CISR_LRF_PLL2_LSH                   1
#define CCM_CISR_LRF_PLL3_LSH                   2
#define CCM_CISR_FPM_READY_LSH                  3
#define CCM_CISR_CAMP1_READY_LSH                4
#define CCM_CISR_CAMP2_READY_LSH                5
#define CCM_CISR_COSC_READY_LSH                 6
#define CCM_CISR_KPP_WAKEUP_DET_LSH             7
#define CCM_CISR_SDHC1_WAKEUP_DET_LSH           8
#define CCM_CISR_SDHC2_WAKEUP_DET_LSH           9
#define CCM_CISR_GPIO1_4_WAKEUP_DET_LSH         10
#define CCM_CISR_GPIO1_5_WAKEUP_DET_LSH         11
#define CCM_CISR_GPIO1_6_WAKEUP_DET_LSH         12
#define CCM_CISR_GPIO1_7_WAKEUP_DET_LSH         13
#define CCM_CISR_GPIO1_8_WAKEUP_DET_LSH         14
#define CCM_CISR_GPIO1_9_WAKEUP_DET_LSH         15
#define CCM_CISR_DIVIDER_LOADED_LSH             16
#define CCM_CISR_AXI_A_PODF_LOADED_LSH          17
#define CCM_CISR_AXI_B_PODF_LOADED_LSH          18
#define CCM_CISR_EMI_SLOW_PODF_LOADED_LSH       19
#define CCM_CISR_AHB_PODF_LOADED_LSH            20
#define CCM_CISR_NFC_PODF_LOADED_LSH            21
#define CCM_CISR_PERIPH_CLK_SEL_LOADED_LSH      22
#define CCM_CISR_EMI_CLK_SEL_LOADED_LSH         23
#define CCM_CISR_ARM_PODF_LOADED_LSH            25

#define CCM_CIMR_MASK_LRF_PLL1_LSH              0
#define CCM_CIMR_MASK_LRF_PLL2_LSH              1
#define CCM_CIMR_MASK_LRF_PLL3_LSH              2
#define CCM_CIMR_MASK_FPM_READY_LSH             3
#define CCM_CIMR_MASK_CAMP1_READY_LSH           4
#define CCM_CIMR_MASK_CAMP2_READY_LSH           5
#define CCM_CIMR_MASK_COSC_READY_LSH            6
#define CCM_CIMR_MASK_KPP_WAKEUP_DET_LSH        7
#define CCM_CIMR_MASK_SDHC1_WAKEUP_DET_LSH      8
#define CCM_CIMR_MASK_SDHC2_WAKEUP_DET_LSH      9
#define CCM_CIMR_MASK_GPIO1_4_WAKEUP_DET_LSH    10
#define CCM_CIMR_MASK_GPIO1_5_WAKEUP_DET_LSH    11
#define CCM_CIMR_MASK_GPIO1_6_WAKEUP_DET_LSH    12
#define CCM_CIMR_MASK_GPIO1_7_WAKEUP_DET_LSH    13
#define CCM_CIMR_MASK_GPIO1_8_WAKEUP_DET_LSH    14
#define CCM_CIMR_MASK_GPIO1_9_WAKEUP_DET_LSH    15
#define CCM_CIMR_MASK_DIVIDER_LOADED_LSH        16
#define CCM_CIMR_MASK_AXI_A_PODF_LOADED_LSH     17
#define CCM_CIMR_MASK_AXI_B_PODF_LOADED_LSH     18
#define CCM_CIMR_MASK_EMI_SLOW_PODF_LOADED_LSH  19
#define CCM_CIMR_MASK_AHB_PODF_LOADED_LSH       20
#define CCM_CIMR_MASK_NFC_PODF_LOADED_LSH       21
#define CCM_CIMR_MASK_PERIPH_CLK_SEL_LOADED_LSH 22
#define CCM_CIMR_MASK_EMI_CLK_SEL_LOADED_LSH    23
#define CCM_CIMR_MASK_ARM_PODF_LOADED_LSH       25


#define CCM_CCOSR_CKO1_SEL_LSH                  0
#define CCM_CCOSR_CKO1_DIV_LSH                  4
#define CCM_CCOSR_CKO1_EN_LSH                   7
#define CCM_CCOSR_CKO2_SEL_LSH                  16
#define CCM_CCOSR_CKO2_DIV_LSH                  21
#define CCM_CCOSR_CKO2_EN_LSH                   24

//#define CCM_CGPR_V1_L2_BIST_CLKDIV_LSH          0
#define CCM_CGPR_FPM_MUX_SELECT_LSH             3
#define CCM_CGPR_EFUSE_PROG_SUPPLY_GATE_LSH     4
#define CCM_CGPR_OVERIDE_APM_EMI_INTR_CLOCK_GATING_LSH   5
#define CCM_CGPR_ARM_ASYNC_REF_SEL5_LSH         15
#define CCM_CGPR_ARM_ASYNC_REF_SEL0_LSH         16
#define CCM_CGPR_ARM_ASYNC_REF_SEL1_LSH         17
#define CCM_CGPR_ARM_ASYNC_REF_SEL2_LSH         18
#define CCM_CGPR_ARM_ASYNC_REF_SEL3_LSH         19
#define CCM_CGPR_ARM_ASYNC_REF_SEL4_LSH         20
#define CCM_CGPR_ARM_ASYNC_REF_SEL6_LSH         21
#define CCM_CGPR_ARM_ASYNC_REF_SEL7_LSH         22
#define CCM_CGPR_ARM_ASYNC_REF_EN_LSH           23
#define CCM_CGPR_ARM_CLK_INPUT_SEL_LSH          24


//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
#define CCM_CCR_OSCNT_WID                       8
#define CCM_CCR_FPM_EN_WID                      1
#define CCM_CCR_CAMP1_EN_WID                    1
#define CCM_CCR_CAMP2_EN_WID                    1
#define CCM_CCR_FPM_MULT_WID                    1
#define CCM_CCR_COSC_EN_WID                     1

#define CCM_CCDR_EMI_HS_MASK_WID                1
#define CCM_CCDR_IPU_HS_MASK_WID                1
#define CCM_CCDR_HSC_HS_MASK_WID                1

#define CCM_CSR_REF_EN_B_WID                    1
#define CCM_CSR_FPM_READY_WID                   1
#define CCM_CSR_CAMP1_READY_WID                 1
#define CCM_CSR_CAMP2_READY_WID                 1
#define CCM_CSR_LVS_VALUE_WID                   1
#define CCM_CSR_COSC_READY_WID                  1

#define CCM_CCSR_PLL3_SW_CLK_SEL_WID            1
#define CCM_CCSR_PLL2_SW_CLK_SEL_WID            1
#define CCM_CCSR_PLL1_SW_CLK_SEL_WID            1
#define CCM_CCSR_PLL3_DIV_PODF_WID              2
#define CCM_CCSR_PLL2_DIV_PODF_WID              2
#define CCM_CCSR_STEP_SEL_WID                   2
#define CCM_CCSR_LP_APM_WID                     1

#define CCM_CACRR_ARM_PODF_WID                  3

#define CCM_CBCDR_PERCLK_PODF_WID               3
#define CCM_CBCDR_PERCLK_PRED2_WID              3
#define CCM_CBCDR_PERCLK_PRED1_WID              2
#define CCM_CBCDR_IPG_PODF_WID                  2
#define CCM_CBCDR_AHB_PODF_WID                  3
#define CCM_CBCDR_NFC_PODF_WID                  3
#define CCM_CBCDR_AXI_A_PODF_WID                3
#define CCM_CBCDR_AXI_B_PODF_WID                3
#define CCM_CBCDR_EMI_SLOW_PODF_WID             3
#define CCM_CBCDR_PERIPH_CLK_SEL_WID            1
#define CCM_CBCDR_EMI_CLK_SEL_WID               1
#define CCM_CBCDR_DDR_CLK_PODF_WID              3
#define CCM_CBCDR_DDR_HIGH_FREQ_CLK_SEL_WID     1

#define CCM_CBCMR_PERCLK_IPG_SEL_WID            1
#define CCM_CBCMR_PERCLK_LP_APM_SEL_WID         1
#define CCM_CBCMR_DEBUG_APB_CLK_SEL_WID         2
#define CCM_CBCMR_GPU_CLK_SEL_WID               2
#define CCM_CBCMR_IPU_HSP_CLK_SEL_WID           2
#define CCM_CBCMR_ARM_AXI_CLK_SEL_WID           2
#define CCM_CBCMR_DDR_CLK_SEL_WID               2
#define CCM_CBCMR_PERIPH_APM_SEL_WID            2
#define CCM_CBCMR_VPU_AXI_SEL_WID               2
#define CCM_CBCMR_GPU2D_CLK_SEL_WID             2

#define CCM_CSCMR1_SSI_EXT1_COM_WID             1
#define CCM_CSCMR1_SSI_EXT2_COM_WID             1
#define CCM_CSCMR1_SPDIF_XTAL_CLK_SEL_WID       2
#define CCM_CSCMR1_ECSPI_CLK_SEL_WID            2
#define CCM_CSCMR1_TVE_EXT_CLK_SEL_WID          1
#define CCM_CSCMR1_TVE_CLK_SEL_WID              1
#define CCM_CSCMR1_SSI_APM_CLK_SEL_WID          2
#define CCM_CSCMR1_VPU_RCLK_ROOT_WID            1
#define CCM_CSCMR1_SSI3_CLK_SEL_WID             1
#define CCM_CSCMR1_SSI2_CLK_SEL_WID             2
#define CCM_CSCMR1_SSI1_CLK_SEL_WID             2
#define CCM_CSCMR1_ESDHC2_CLK_SEL_WID           2
#define CCM_CSCMR1_ESDHC4_CLK_SEL_WID           1
#define CCM_CSCMR1_ESDHC3_CLK_SEL_WID           1
#define CCM_CSCMR1_ESDHC1_CLK_SEL_WID           2
#define CCM_CSCMR1_USBOH3_CLK_SEL_WID           2
#define CCM_CSCMR1_UART_CLK_SEL_WID             2
#define CCM_CSCMR1_USB_PHY_CLK_SEL_WID          1
#define CCM_CSCMR1_SSI_EXT1_CLK_SEL_WID         2
#define CCM_CSCMR1_SSI_EXT2_CLK_SEL_WID         2


#define CCM_CSCMR2_SPDIF0_CLK_SEL_WID           2
#define CCM_CSCMR2_SPDIF1_CLK_SEL_WID           2
#define CCM_CSCMR2_SPDIF0_COM_WID               1
#define CCM_CSCMR2_SPDIF1_COM_WID               1
#define CCM_CSCMR2_SLIMBUS_CLK_SEL_WID          3
#define CCM_CSCMR2_SLIMBUS_COM_WID              1
#define CCM_CSCMR2_SIM_CLK_SEL_WID              2
#define CCM_CSCMR2_FIRI_CLK_SEL_WID             2
#define CCM_CSCMR2_HSI2C_CLK_SEL_WID            2
#define CCM_CSCMR2_HSC1_CLK_SEL_WID             2
#define CCM_CSCMR2_HSC2_CLK_SEL_WID             2
#define CCM_CSCMR2_ESC_CLK_SEL_WID              2
#define CCM_CSCMR2_CSI_MCLK1_CLK_SEL_WID        2
#define CCM_CSCMR2_CSI_MCLK2_CLK_SEL_WID        2 
#define CCM_CSCMR2_DI_CLK_SEL_WID               3
#define CCM_CSCMR2_DI0_CLK_SEL_WID              3
#define CCM_CSCMR2_DI1_CLK_SEL_WID              3


#define CCM_CSCDR1_UART_CLK_PODF_WID            3
#define CCM_CSCDR1_UART_CLK_PRED_WID            3
#define CCM_CSCDR1_USBOH3_CLK_PODF_WID          2
#define CCM_CSCDR1_USBOH3_CLK_PRED_WID          3
#define CCM_CSCDR1_ESDHC1_CLK_PODF_WID          3
#define CCM_CSCDR1_PGC_CLK_PODF_WID             2
#define CCM_CSCDR1_ESDHC1_CLK_PRED_WID          3
#define CCM_CSCDR1_ESDHC2_CLK_PODF_WID          3
#define CCM_CSCDR1_ESDHC2_CLK_PRED_WID          3

#define CCM_CS1CDR_SSI1_CLK_PODF_WID            6
#define CCM_CS1CDR_SSI1_CLK_PRED_WID            3
#define CCM_CS1CDR_SSI_EXT1_CLK_PODF_WID        6
#define CCM_CS1CDR_SSI_EXT1_CLK_PRED_WID        3

#define CCM_CS2CDR_SSI2_CLK_PODF_WID            6
#define CCM_CS2CDR_SSI2_CLK_PRED_WID            3
#define CCM_CS2CDR_SSI_EXT2_CLK_PODF_WID        6
#define CCM_CS2CDR_SSI_EXT2_CLK_PRED_WID        3

#define CCM_CDCDR_USB_PHY_PODF_WID              3
#define CCM_CDCDR_USB_PHY_PRED_WID              3
#define CCM_CDCDR_DI_CLK_PRED_WID               3
#define CCM_CDCDR_SPDIF1_CLK_PODF_WID           6
#define CCM_CDCDR_SPDIF1_CLK_PRED_WID           3
#define CCM_CDCDR_SPDIF0_CLK_PODF_WID           6
#define CCM_CDCDR_SPDIF0_CLK_PRED_WID           3
#define CCM_CDCDR_TVE_CLK_PRED_WID              3

#define CCM_CHSCCDR_HSC1_CLK_PODF_WID           3
#define CCM_CHSCCDR_HSC2_CLK_PODF_WID           3
#define CCM_CHSCCDR_ESC_CLK_PODF_WID            6
#define CCM_CHSCCDR_ESC_CLK_PRED_WID            3

#define CCM_CSCDR2_SLIMBUS_CLK_PODF_WID         6
#define CCM_CSCDR2_SLIMBUS_CLK_PRED_WID         3
#define CCM_CSCDR2_SIM_CLK_PODF_WID             6
#define CCM_CSCDR2_SIM_CLK_PRED_WID             3
#define CCM_CSCDR2_ECSPI_CLK_PODF_WID           6
#define CCM_CSCDR2_ECSPI_CLK_PRED_WID           3

#define CCM_CSCDR3_FIRI_CLK_PODF_WID            6
#define CCM_CSCDR3_FIRI_CLK_PRED_WID            3
#define CCM_CSCDR3_HSI2C_CLK_PODF_WID           6
#define CCM_CSCDR3_HSI2C_CLK_PRED_WID           3

#define CCM_CSCDR4_CSI_MCLK1_CLK_PODF_WID       6
#define CCM_CSCDR4_CSI_MCLK1_CLK_PRED_WID       3
#define CCM_CSCDR4_CSI_MCLK2_CLK_PODF_WID       6
#define CCM_CSCDR4_CSI_MCLK2_CLK_PRED_WID       3

#define CCM_CWDR_GPIO1_4_ICR_WID                2
#define CCM_CWDR_GPIO1_5_ICR_WID                2
#define CCM_CWDR_GPIO1_6_ICR_WID                2
#define CCM_CWDR_GPIO1_7_ICR_WID                2
#define CCM_CWDR_GPIO1_8_ICR_WID                2
#define CCM_CWDR_GPIO1_9_ICR_WID                2
#define CCM_CWDR_GPIO1_4_DIR_WID                1
#define CCM_CWDR_GPIO1_5_DIR_WID                1
#define CCM_CWDR_GPIO1_6_DIR_WID                1
#define CCM_CWDR_GPIO1_7_DIR_WID                1
#define CCM_CWDR_GPIO1_8_DIR_WID                1
#define CCM_CWDR_GPIO1_9_DIR_WID                1

#define CCM_CDHIPR_AXI_A_PODF_BUSY_WID          1
#define CCM_CDHIPR_AXI_B_PODF_BUSY_WID          1
#define CCM_CDHIPR_EMI_SLOW_PODF_BUSY_WID       1
#define CCM_CDHIPR_AHB_PODF_BUSY_WID            1
#define CCM_CDHIPR_NFC_PODF_BUSY_WID            1
#define CCM_CDHIPR_PERIPH_CLK_SEL_BUSY_WID      1
#define CCM_CDHIPR_EMI_CLK_SEL_BUSY_WID         1
#define CCM_CDHIPR_DDR_PODF_BUSY_WID            1
#define CCM_CDHIPR_DDR_HF_CLK_SEL_BUSY_WID      1
#define CCM_CDHIPR_ARM_PODF_BUSY_WID            1

#define CCM_CDCR_PERIPH_CLK_DVFS_PODF_WID       2
#define CCM_CDCR_ARM_FREQ_SHIFT_DIVIDER_WID     1
#define CCM_CDCR_HSC1_DVFS_EN_WID               1
#define CCM_CDCR_HSC2_DVFS_EN_WID               1
#define CCM_CDCR_SOFTWARE_DVFS_EN_WID           1
#define CCM_CDCR_SW_PERIPH_CLK_DIV_REQ_WID      1
#define CCM_CDCR_SW_PERIPH_CLK_DIV_REQ_STATUS_WID  1 


#define CCM_CTOR_OBS_SPARE_OUTPUT_2_SEL_WID     4
#define CCM_CTOR_OBS_SPARE_OUTPUT_1_SLE_WID     4
#define CCM_CTOR_OBS_SPARE_OUTPUT_0_SLE_WID     5
#define CCM_CTOR_OBS_EN_WID                     1

#define CCM_CLPCR_LPM_WID                       2
#define CCM_CLPCR_BYPASS_PMIC_READY_WID         1
#define CCM_CLPCR_LPSR_CLK_SEL_WID              2
#define CCM_CLPCR_ARM_CLK_DIS_ON_LPM_WID        1
#define CCM_CLPCR_SBYOS_WID                     1
#define CCM_CLPCR_DIS_REF_OSC_WID               1
#define CCM_CLPCR_VSTBY_WID                     1
#define CCM_CLPCR_STBY_COUNT_WID                2
#define CCM_CLPCR_COSC_PWRDOWN_WID              1
#define CCM_CLPCR_BYPASS_SAHARA_LPM_HS_WID      1
#define CCM_CLPCR_BYPASS_RTIC_LPM_HS_WID        1
#define CCM_CLPCR_BYPASS_IPU_LPM_HS_WID         1
#define CCM_CLPCR_BYPASS_EMI_LPM_HS_WID         1
#define CCM_CLPCR_BYPASS_SDMA_LPM_HS_WID        1
#define CCM_CLPCR_BYPASS_MAX_LPM_HS_WID         1
#define CCM_CLPCR_BYPASS_SCC_LPM_HS_WID         1
#define CCM_CLPCR_BYPASS_HSC_LPM_HS_WID         1

#define CCM_CISR_LRF_PLL1_WID                   1
#define CCM_CISR_LRF_PLL2_WID                   1
#define CCM_CISR_LRF_PLL3_WID                   1
#define CCM_CISR_FPM_READY_WID                  1
#define CCM_CISR_CAMP1_READY_WID                1
#define CCM_CISR_CAMP2_READY_WID                1
#define CCM_CISR_COSC_READY_WID                 1
#define CCM_CISR_KPP_WAKEUP_DET_WID             1
#define CCM_CISR_SDHC1_WAKEUP_DET_WID           1
#define CCM_CISR_SDHC2_WAKEUP_DET_WID           1
#define CCM_CISR_GPIO1_4_WAKEUP_DET_WID         1
#define CCM_CISR_GPIO1_5_WAKEUP_DET_WID         1
#define CCM_CISR_GPIO1_6_WAKEUP_DET_WID         1
#define CCM_CISR_GPIO1_7_WAKEUP_DET_WID         1
#define CCM_CISR_GPIO1_8_WAKEUP_DET_WID         1
#define CCM_CISR_GPIO1_9_WAKEUP_DET_WID         1
#define CCM_CISR_DIVIDER_LOADED_WID             1
#define CCM_CISR_AXI_A_PODF_LOADED_WID          1
#define CCM_CISR_AXI_B_PODF_LOADED_WID          1
#define CCM_CISR_EMI_SLOW_PODF_LOADED_WID       1
#define CCM_CISR_AHB_PODF_LOADED_WID            1
#define CCM_CISR_NFC_PODF_LOADED_WID            1
#define CCM_CISR_PERIPH_CLK_SEL_LOADED_WID      1
#define CCM_CISR_EMI_CLK_SEL_LOADED_WID         1
#define CCM_CISR_ARM_PODF_LOADED_WID            1


#define CCM_CIMR_MASK_LRF_PLL1_WID              1
#define CCM_CIMR_MASK_LRF_PLL2_WID              1
#define CCM_CIMR_MASK_LRF_PLL3_WID              1
#define CCM_CIMR_MASK_FPM_READY_WID             1
#define CCM_CIMR_MASK_CAMP1_READY_WID           1
#define CCM_CIMR_MASK_CAMP2_READY_WID           1
#define CCM_CIMR_MASK_COSC_READY_WID            1
#define CCM_CIMR_MASK_KPP_WAKEUP_DET_WID        1
#define CCM_CIMR_MASK_SDHC1_WAKEUP_DET_WID      1
#define CCM_CIMR_MASK_SDHC2_WAKEUP_DET_WID      1
#define CCM_CIMR_MASK_GPIO1_4_WAKEUP_DET_WID    1
#define CCM_CIMR_MASK_GPIO1_5_WAKEUP_DET_WID    1
#define CCM_CIMR_MASK_GPIO1_6_WAKEUP_DET_WID    1
#define CCM_CIMR_MASK_GPIO1_7_WAKEUP_DET_WID    1
#define CCM_CIMR_MASK_GPIO1_8_WAKEUP_DET_WID    1
#define CCM_CIMR_MASK_GPIO1_9_WAKEUP_DET_WID    1
#define CCM_CIMR_MASK_DIVIDER_LOADED_WID        1
#define CCM_CIMR_MASK_AXI_A_PODF_LOADED_WID     1
#define CCM_CIMR_MASK_AXI_B_PODF_LOADED_WID     1
#define CCM_CIMR_MASK_EMI_SLOW_PODF_LOADED_WID  1
#define CCM_CIMR_MASK_AHB_PODF_LOADED_WID       1
#define CCM_CIMR_MASK_NFC_PODF_LOADED_WID       1
#define CCM_CIMR_MASK_PERIPH_CLK_SEL_LOADED_WID 1
#define CCM_CIMR_MASK_EMI_CLK_SEL_LOADED_WID    1
#define CCM_CIMR_MASK_ARM_PODF_LOADED_WID       1


#define CCM_CCOSR_CKO1_SEL_WID                  4
#define CCM_CCOSR_CKO1_DIV_WID                  3
#define CCM_CCOSR_CKO1_EN_WID                   1
#define CCM_CCOSR_CKO2_SEL_WID                  5
#define CCM_CCOSR_CKO2_DIV_WID                  3
#define CCM_CCOSR_CKO2_EN_WID                   1

//#define CCM_CGPR_V1_L2_BIST_CLKDIV_WID          3
#define CCM_CGPR_FPM_MUX_SELECT_WID             1
#define CCM_CGPR_EFUSE_PROG_SUPPLY_GATE_WID     1
#define CCM_CGPR_OVERIDE_APM_EMI_INTR_CLOCK_GATING_WID   1
#define CCM_CGPR_ARM_ASYNC_REF_SEL5_WID         1
#define CCM_CGPR_ARM_ASYNC_REF_SEL0_WID         1
#define CCM_CGPR_ARM_ASYNC_REF_SEL1_WID         1
#define CCM_CGPR_ARM_ASYNC_REF_SEL2_WID         1
#define CCM_CGPR_ARM_ASYNC_REF_SEL3_WID         1
#define CCM_CGPR_ARM_ASYNC_REF_SEL4_WID         1
#define CCM_CGPR_ARM_ASYNC_REF_SEL6_WID         1
#define CCM_CGPR_ARM_ASYNC_REF_SEL7_WID         1
#define CCM_CGPR_ARM_ASYNC_REF_EN_WID           1
#define CCM_CGPR_ARM_CLK_INPUT_SEL_WID          1


//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------
//CCSR
#define CCM_CCSR_PLL3_SW_CLK_SEL_MAIN           0
#define CCM_CCSR_PLL3_SW_CLK_SEL_BYPASS         1

#define CCM_CCSR_PLL2_SW_CLK_SEL_MAIN           0
#define CCM_CCSR_PLL2_SW_CLK_SEL_BYPASS         1

#define CCM_CCSR_PLL1_SW_CLK_SEL_MAIN           0
#define CCM_CCSR_PLL1_SW_CLK_SEL_STEP           1

#define CCM_CCSR_STEP_SEL_LP_APM                0 
#define CCM_CCSR_STEP_SEL_PLL1_BYPASS           1
#define CCM_CCSR_STEP_SEL_PLL2                  2
#define CCM_CCSR_STEP_SEL_PLL3                  3

#define CCM_CCSR_LP_APM_OSC                     0
#define CCM_CCSR_LP_APM_FPM                     1

// CLPCR
#define CCM_CLPCR_LPM_RUN                       0
#define CCM_CLPCR_LPM_WAIT                      1
#define CCM_CLPCR_LPM_STOP                      2
#define CCM_CLPCR_LPM_LPSR                      3

#define CCM_CLPCR_LPSR_CLK_SEL_CKIL             0
#define CCM_CLPCR_LPSR_CLK_SEL_FPM              1
#define CCM_CLPCR_LPSR_CLK_SEL_FPM_HALF         2
#define CCM_CLPCR_LPSR_CLK_SEL_GND              3

#define CCM_CLPCR_ARM_CLK_DIS_ON_LPM_ENABLE     0
#define CCM_CLPCR_ARM_CLK_DIS_ON_LPM_DISABLE    1

#define CCM_CLPCR_VSTBY_DISABLE                 0
#define CCM_CLPCR_VSTBY_ENABLE                  1

#define CCM_CLPCR_COSC_PWRDOWN_DISABLE          0
#define CCM_CLPCR_COSC_PWRDOWN_ENABLE           1

//CWDR
#define CCM_CWDR_GPIO_DIR_INPUT                 0
#define CCM_CWDR_GPIO_DIR_OUTPUT                1

#define CCM_CWDR_GPIO_ICR_LOW_LEVEL             0
#define CCM_CWDR_GPIO_ICR_HIGH_LEVEL            1
#define CCM_CWDR_GPIO_ICR_RISE_EDGE             2
#define CCM_CWDR_GPIO_ICR_FALL_EDGE             3


//------------------------------------------------------------------------------
// HELPER MACROS
//------------------------------------------------------------------------------
#define CCM_CGR_INDEX(gateIndex)        ((gateIndex) >> 4)
#define CCM_CGR_SHIFT(gateIndex)        (((gateIndex) % 16) << 1)
#define CCM_CGR_MASK(gateIndex)         (CCM_CGR_CG_MASK <<  CCM_CGR_SHIFT(gateIndex))
#define CCM_CGR_VAL(gateIndex, val)     (val << CCM_CGR_SHIFT(gateIndex))
#define CCM_CMEOR_SHIFT(Index)          (Index)
#define CCM_CMEOR_MASK(Index)           (1 << (Index))
#define CCM_CMEOR_VAL(Index, val)       (val << CCM_CMEOR_SHIFT(Index))

// CGR Masks
#define CCM_CGR0_ARM_BUS_MASK            CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_ARM_BUS)
#define CCM_CGR0_ARM_AXI_MASK            CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_ARM_AXI)
#define CCM_CGR0_ARM_DEBUG_MASK          CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_ARM_DEBUG)
#define CCM_CGR0_TZIC_MASK               CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_TZIC)
#define CCM_CGR0_DAP_MASK                CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_DAP)
#define CCM_CGR0_TPIU_MASK               CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_TPIU)
#define CCM_CGR0_CTI2_MASK               CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_CTI2)
#define CCM_CGR0_CTI3_MASK               CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_CTI3)
#define CCM_CGR0_AHBMUX1_MASK            CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_AHBMUX1)
#define CCM_CGR0_AHBMUX2_MASK            CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_AHBMUX2)
#define CCM_CGR0_ROMCP_MASK              CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_ROMCP)
#define CCM_CGR0_ROM_MASK                CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_ROM)  
#define CCM_CGR0_AIPS_TZ1_MASK           CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_AIPS_TZ1)
#define CCM_CGR0_AIPS_TZ2_MASK           CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_AIPS_TZ2)
#define CCM_CGR0_AHB_MAX_MASK            CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_AHB_MAX)
#define CCM_CGR0_IIM_MASK                CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_IIM)

#define CCM_CGR1_TMAX1_MASK              CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_TMAX1)
#define CCM_CGR1_TMAX2_MASK              CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_TMAX2)
#define CCM_CGR1_TMAX3_MASK              CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_TMAX3)
#define CCM_CGR1_UART1_IPG_MASK          CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_UART1_IPG)
#define CCM_CGR1_UART1_PERCLK_MASK       CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_UART1_PERCLK)
#define CCM_CGR1_UART2_IPG_MASK          CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_UART2_IPG)
#define CCM_CGR1_UART2_PERCLK_MASK       CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_UART2_PERCLK)
#define CCM_CGR1_UART3_IPG_MASK          CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_UART3_IPG)
#define CCM_CGR1_UART3_PERCLK_MASK       CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_UART3_PERCLK)
#define CCM_CGR1_I2C1_MASK               CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_I2C1)
#define CCM_CGR1_I2C2_MASK               CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_I2C2)
#define CCM_CGR1_HSI2C_IPG_MASK          CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_HSI2C_IPG)
#define CCM_CGR1_HSI2C_SERIAL_MASK       CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_HSI2C_SERIAL)  
#define CCM_CGR1_FIRI_IPG_MASK           CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_FIRI_IPG)
#define CCM_CGR1_FIRI_SERIAL_MASK        CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_FIRI_SERIAL)   

#define CCM_CGR2_USB_PHY_MASK            CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_USB_PHY)
#define CCM_CGR2_EPIT1_IPG_MASK          CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_EPIT1_IPG)
#define CCM_CGR2_EPIT1_HIGHFREQ_MASK     CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_EPIT1_HIGHFREQ)
#define CCM_CGR2_EPIT2_IPG_MASK          CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_EPIT2_IPG)
#define CCM_CGR2_EPIT2_HIGHFREQ_MASK     CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_EPIT2_HIGHFREQ)
#define CCM_CGR2_PWM1_IPG_MASK           CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_PWM1_IPG)
#define CCM_CGR2_PWM1_HIGHFREQ_MASK      CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_PWM1_HIGHFREQ)
#define CCM_CGR2_PWM2_IPG_MASK           CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_PWM2_IPG)
#define CCM_CGR2_PWM2_HIGHFREQ_MASK      CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_PWM2_HIGHFREQ)
#define CCM_CGR2_GPT_IPG_MASK            CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_GPT_IPG)
#define CCM_CGR2_GPT_HIGHFREQ_MASK       CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_GPT_HIGHFREQ)
#define CCM_CGR2_OWIRE_MASK              CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_OWIRE)
#define CCM_CGR2_FEC_MASK                CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_FEC)
#define CCM_CGR2_USBOH3_IPG_MASK         CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_USBOH3_IPG)
#define CCM_CGR2_USBOH3_60M_MASK         CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_USBOH3_60M)
#define CCM_CGR2_TVE_MASK                CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_TVE)
                            
#define CCM_CGR3_ESDHC1_IPG_MASK         CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_ESDHC1_IPG)
#define CCM_CGR3_ESDHC1_PERCLK_MASK      CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_ESDHC1_PERCLK)
#define CCM_CGR3_ESDHC2_IPG_MASK         CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_ESDHC2_IPG)
#define CCM_CGR3_ESDHC2_PERCLK_MASK      CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_ESDHC2_PERCLK)
#define CCM_CGR3_ESDHC3_IPG_MASK         CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_ESDHC3_IPG)
#define CCM_CGR3_ESDHC3_PERCLK_MASK      CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_ESDHC3_PERCLK)
#define CCM_CGR3_ESDHC4_IPG_MASK         CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_ESDHC4_IPG)
#define CCM_CGR3_ESDHC4_PERCLK_MASK      CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_ESDHC4_PERCLK)
#define CCM_CGR3_SSI1_IPG_MASK           CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_SSI1_IPG)
#define CCM_CGR3_SSI1_SSI_MASK           CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_SSI1_SSI)
#define CCM_CGR3_SSI2_IPG_MASK           CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_SSI2_IPG)
#define CCM_CGR3_SSI2_SSI_MASK           CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_SSI2_SSI)
#define CCM_CGR3_SSI3_IPG_MASK           CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_SSI3_IPG)
#define CCM_CGR3_SSI3_SSI_MASK           CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_SSI3_SSI)
#define CCM_CGR3_SSI_EXT1_MASK           CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_SSI_EXT1)
#define CCM_CGR3_SSI_EXT2_MASK           CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_SSI_EXT2)

#define CCM_CGR4_PATA_MASK               CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_PATA)
#define CCM_CGR4_SIM_IPG_MASK            CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_SIM_IPG)
#define CCM_CGR4_SIM_SERIAL_MASK         CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_SIM_SERIAL) 
#define CCM_CGR4_HSC_HS1_MASK            CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_HSC_HS1)
#define CCM_CGR4_HSC_HS2_MASK            CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_HSC_HS2)
#define CCM_CGR4_HSC_ESC_MASK            CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_HSC_ESC)
#define CCM_CGR4_HSC_HSP_MASK            CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_HSC_HSP)
#define CCM_CGR4_SAHARA_MASK             CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_SAHARA)
#define CCM_CGR4_RTIC_MASK               CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_RTIC)
#define CCM_CGR4_ECSPI1_IPG_MASK         CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_ECSPI1_IPG)
#define CCM_CGR4_ECSPI1_PERCLK_MASK      CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_ECSPI1_PERCLK)
#define CCM_CGR4_ECSPI2_IPG_MASK         CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_ECSPI2_IPG)
#define CCM_CGR4_ECSPI2_PERCLK_MASK      CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_ECSPI2_PERCLK)
#define CCM_CGR4_CSPI_IPG_MASK           CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_CSPI_IPG)
#define CCM_CGR4_SRTC_MASK               CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_SRTC)
#define CCM_CGR4_SDMA_MASK               CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_SDMA)

#define CCM_CGR5_SPBA_MASK               CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_SPBA)
#define CCM_CGR5_GPU_MASK                CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_GPU)
#define CCM_CGR5_GARB_MASK               CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_GARB)
#define CCM_CGR5_VPU_MASK                CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_VPU)
#define CCM_CGR5_VPU_REFERENCE_MASK      CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_VPU_REFERENCE)
#define CCM_CGR5_IPU_MASK                CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_IPU)
#define CCM_CGR5_IPU_DI_MASK             CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_IPU_DI)
#define CCM_CGR5_EMI_FAST_MASK           CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_EMI_FAST)
#define CCM_CGR5_EMI_SLOW_MASK           CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_EMI_SLOW)
#define CCM_CGR5_EMI_INTR_MASK           CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_EMI_INTR)
#define CCM_CGR5_EMI_ENFC_MASK           CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_EMI_ENFC)
#define CCM_CGR5_EMI_WRCK_MASK           CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_EMI_WRCK)
#define CCM_CGR5_GPC_IPG_MASK            CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_GPC_IPG)
#define CCM_CGR5_SPDIF0_MASK             CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_SPDIF0)
#define CCM_CGR5_SPDIF1_MASK             CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_SPDIF1)
#define CCM_CGR5_SPDIF_IPG_MASK          CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_SPDIF_IPG)

#define CCM_CGR6_SLIMBUS_MASK            CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_SLIMBUS)
#define CCM_CGR6_SLIMBUS_SERIAL_MASK     CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_SLIMBUS_SERIAL)
#define CCM_CGR6_CSI_MCLK1_MASK          CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_CSI_MCLK1)
#define CCM_CGR6_CSI_MCLK2_MASK          CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_CSI_MCLK2)
#define CCM_CGR6_EMI_GARB_MASK           CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_EMI_GARB)
#define CCM_CGR6_IPU_DI0_MASK            CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_IPU_DI0)
#define CCM_CGR6_IPU_DI1_MASK            CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_IPU_DI1)
#define CCM_CGR6_GPU2D_MASK              CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_GPU2D)

#ifdef __cplusplus
}
#endif

#endif // __MX51_CCM_H
