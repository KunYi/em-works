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
// Copyright (C) 2004, Motorola Inc. All Rights Reserved
//
//------------------------------------------------------------------------------
//
// Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
// Header:  mx27_sysctrl.h
//
// Provides definitions for system control module based on MX27.
//
//------------------------------------------------------------------------------
#ifndef __MX27_SYSCTRL_H__
#define __MX27_SYSCTRL_H__

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
    REG32 CID;
    REG32 reserved1[4];
    REG32 FMCR;
    REG32 GPCR;
    REG32 WBCR;
    REG32 DSCR1;
    REG32 DSCR2;
    REG32 DSCR3;
    REG32 DSCR4;
    REG32 DSCR5;
    REG32 DSCR6;
    REG32 DSCR7;
    REG32 DSCR8;
    REG32 DSCR9;
    REG32 DSCR10;
    REG32 DSCR11;
    REG32 DSCR12;
    REG32 DSCR13;
    REG32 PSCR;
    REG32 PCSR;
    REG32 reserved2;
    REG32 PMCR;
    REG32 DCVR[4];
} CSP_SYSCTRL_REGS, *PCSP_SYSCTRL_REGS;

//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
#define SYSCTRL_SIDR_OFFSET         0x0000
// Reserved                         0x0004 ~ 0x0010
#define SYSCTRL_FMCR_OFFSET         0x0014
#define SYSCTRL_GPCR_OFFSET         0x0018
#define SYSCTRL_WBCR_OFFSET         0x001C
#define SYSCTRL_DSCR1_OFFSET        0x0020
#define SYSCTRL_DSCR2_OFFSET        0x0024
#define SYSCTRL_DSCR3_OFFSET        0x0028
#define SYSCTRL_DSCR4_OFFSET        0x002C
#define SYSCTRL_DSCR5_OFFSET        0x0030
#define SYSCTRL_DSCR6_OFFSET        0x0034
#define SYSCTRL_DSCR7_OFFSET        0x0038
#define SYSCTRL_DSCR8_OFFSET        0x003C
#define SYSCTRL_DSCR9_OFFSET        0x0040
#define SYSCTRL_DSCR10_OFFSET       0x0044
#define SYSCTRL_DSCR11_OFFSET       0x0048
#define SYSCTRL_DSCR12_OFFSET       0x004C
#define SYSCTRL_DSCR13_OFFSET       0x0050
#define SYSCTRL_PSCR_OFFSET         0x0054
#define SYSCTRL_PCSR_OFFSET         0x0058
// Reserved                         0x005C
#define SYSCTRL_PMCR_OFFSET         0x0060
#define SYSCTRL_DCVR_OFFSET         0x0064


//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
// CID
#define SYSCTRL_CID_MANUFACTURE_ID_LSH          0
#define SYSCTRL_CID_PART_NUMBER_LSH             12
#define SYSCTRL_CID_VERSION_ID_LSH              28

// FMCR
#define SYSCTRL_FMCR_SDCS0_SEL_LSH              0
#define SYSCTRL_FMCR_SDCS1_SEL_LSH              1
#define SYSCTRL_FMCR_SLCDC_SEL_LSH              2
#define SYSCTRL_FMCR_NF_16BIT_SEL_LSH           4
#define SYSCTRL_FMCR_NF_FMS_LSH                 5
#define SYSCTRL_FMCR_IOIS16_CTL_LSH             8
#define SYSCTRL_FMCR_PC_BVD2_CTL_LSH            9
#define SYSCTRL_FMCR_PC_BVD1_CTL_LSH            10
#define SYSCTRL_FMCR_PC_VS2_CTL_LSH             11
#define SYSCTRL_FMCR_PC_VS1_CTL_LSH             12
#define SYSCTRL_FMCR_PC_READY_CTL_LSH           13
#define SYSCTRL_FMCR_PC_WAIT_B_CTL_LSH          14
#define SYSCTRL_FMCR_KP_ROW6_CTL_LSH            16
#define SYSCTRL_FMCR_KP_ROW7_CTL_LSH            17
#define SYSCTRL_FMCR_KP_COL6_CTL_LSH            18
#define SYSCTRL_FMCR_UART4_RTS_CTL_LSH          24
#define SYSCTRL_FMCR_UART4_RXD_CTL_LSH          25

// GPCR
#define SYSCTRL_GPCR_CLK_DDR_MODE_LSH           1
#define SYSCTRL_GPCR_DDR_MODE_LSH               2
#define SYSCTRL_GPCR_CLOCK_GATING_EN_LSH        3
#define SYSCTRL_GPCR_DMA_BURST_OVERRIDE_LSH     8
#define SYSCTRL_GPCR_PP_BURST_OVERRIDE_LSH      9
#define SYSCTRL_GPCR_USB_BURST_OVERRIDE_LSH     10
#define SYSCTRL_GPCR_ETM9_PAD_EN_LSH            11
#define SYSCTRL_GPCR_BOOT_LSH                   16

// WBCR
#define SYSCTRL_WBCR_CRM_WBM_LSH                0
#define SYSCTRL_WBCR_CRM_WBFA_LSH               3
#define SYSCTRL_WBCR_CRM_SPA_BIT01_LSH          8
#define SYSCTRL_WBCR_CRM_SPA_BIT23_LSH          10
#define SYSCTRL_WBCR_CRM_WBM_EMI_LSH            16
#define SYSCTRL_WBCR_CRM_WBFA_EMI_LSH           19
#define SYSCTRL_WBCR_CRM_SPA_EMI_BIT01_LSH      24
#define SYSCTRL_WBCR_CRM_SPA_EMI_BIT23_LSH      26

//DSCR1
#define SYSCTRL_DSCR1_DS_SLOW11_DVS_PMIC_LSH     		20
#define SYSCTRL_DSCR1_DS_SLOW10_SDHC1_CSPI3_LSH     	18
#define SYSCTRL_DSCR1_DS_SLOW9_JTAG_LSH     	16
#define SYSCTRL_DSCR1_DS_SLOW8_PWM_LSH 	 	14
#define SYSCTRL_DSCR1_DS_SLOW7_CSPI1_LSH    	12
#define SYSCTRL_DSCR1_DS_SLOW6_SSI1_LSH       	10
#define SYSCTRL_DSCR1_DS_SLOW5_GPT1_LSH      	8
#define SYSCTRL_DSCR1_DS_SLOW4_USBH1_LSH    	6
#define SYSCTRL_DSCR1_DS_SLOW3_CSI_LSH          	4
#define SYSCTRL_DSCR1_DS_SLOW2_SDHC2_MSHC_LSH     	2
#define SYSCTRL_DSCR1_DS_SLOW1_LCDC_LSH     	0

//PSCR
#define SYSCTRL_PSCR_SD2_D0_PUENCR_LSH	14
#define SYSCTRL_PSCR_SD2_D1_PUENCR_LSH	12
#define SYSCTRL_PSCR_SD2_D2_PUENCR_LSH	10
#define SYSCTRL_PSCR_SD2_D3_PUENCR_LSH	8
#define SYSCTRL_PSCR_SD2_CMD_PUENCR_LSH	6
#define SYSCTRL_PSCR_SD2_CLK_PUENCR_LSH	4
#define SYSCTRL_PSCR_SD1_D3_PUENCR_LSH	2
#define SYSCTRL_PSCR_ATA_DAT3_PUENCR_LSH	0

// PCSR
#define SYSCTRL_PCSR_S0_AMPR_SEL_LSH            16
#define SYSCTRL_PCSR_S1_AMPR_SEL_LSH            17
#define SYSCTRL_PCSR_S2_AMPR_SEL_LSH            18
#define SYSCTRL_PCSR_S3_AMPR_SEL_LSH            19

// PMCR
#define SYSCTRL_PMCR_DPTEN_LSH                  0
#define SYSCTRL_PMCR_DIE_LSH                    1
#define SYSCTRL_PMCR_DIM_LSH                    2
#define SYSCTRL_PMCR_DRCE0_LSH                  4
#define SYSCTRL_PMCR_DRCE1_LSH                  5
#define SYSCTRL_PMCR_DRCE2_LSH                  6
#define SYSCTRL_PMCR_DRCE3_LSH                  7
#define SYSCTRL_PMCR_RCLKON_LSH                 8
#define SYSCTRL_PMCR_DCR_LSH                    9
#define SYSCTRL_PMCR_REFCOUNTER_LSH             16
#define SYSCTRL_PMCR_LO_LSH                     28
#define SYSCTRL_PMCR_UP_LSH                     29
#define SYSCTRL_PMCR_EM_LSH                     30
#define SYSCTRL_PMCR_MC_LSH                     31

// DCVR
#define SYSCTRL_DCVR_ELV_LSH                    0
#define SYSCTRL_DCVR_LLV_LSH                    10
#define SYSCTRL_DCVR_ULV_LSH                    21

//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
// CID
#define SYSCTRL_CID_MANUFACTURE_ID_WID          12
#define SYSCTRL_CID_PART_NUMBER_WID             16
#define SYSCTRL_CID_VERSION_ID_WID              4

// FMCR
#define SYSCTRL_FMCR_SDCS0_SEL_WID              1
#define SYSCTRL_FMCR_SDCS1_SEL_WID              1
#define SYSCTRL_FMCR_SLCDC_SEL_WID              1
#define SYSCTRL_FMCR_NF_16BIT_SEL_WID           1
#define SYSCTRL_FMCR_NF_FMS_WID                 1
#define SYSCTRL_FMCR_IOIS16_CTL_WID             1
#define SYSCTRL_FMCR_PC_BVD2_CTL_WID            1
#define SYSCTRL_FMCR_PC_BVD1_CTL_WID            1
#define SYSCTRL_FMCR_PC_VS2_CTL_WID             1
#define SYSCTRL_FMCR_PC_VS1_CTL_WID             1
#define SYSCTRL_FMCR_PC_READY_CTL_WID           1
#define SYSCTRL_FMCR_PC_WAIT_B_CTL_WID          1
#define SYSCTRL_FMCR_KP_ROW6_CTL_WID            1
#define SYSCTRL_FMCR_KP_ROW7_CTL_WID            1
#define SYSCTRL_FMCR_KP_COL6_CTL_WID            1
#define SYSCTRL_FMCR_UART4_RTS_CTL_WID          1
#define SYSCTRL_FMCR_UART4_RXD_CTL_WID          1

// GPCR
#define SYSCTRL_GPCR_CLK_DDR_MODE_WID           1
#define SYSCTRL_GPCR_DDR_MODE_WID               1
#define SYSCTRL_GPCR_CLOCK_GATING_EN_WID        1
#define SYSCTRL_GPCR_DMA_BURST_OVERRIDE_WID     1
#define SYSCTRL_GPCR_PP_BURST_OVERRIDE_WID      1
#define SYSCTRL_GPCR_USB_BURST_OVERRIDE_WID     1
#define SYSCTRL_GPCR_ETM9_PAD_EN_WID            1
#define SYSCTRL_GPCR_BOOT_WID                   4

// WBCR
#define SYSCTRL_WBCR_CRM_WBM_WID                3
#define SYSCTRL_WBCR_CRM_WBFA_WID               1
#define SYSCTRL_WBCR_CRM_SPA_BIT01_WID          2
#define SYSCTRL_WBCR_CRM_SPA_BIT23_WID          2
#define SYSCTRL_WBCR_CRM_WBM_EMI_WID            3
#define SYSCTRL_WBCR_CRM_WBFA_EMI_WID           1
#define SYSCTRL_WBCR_CRM_SPA_EMI_BIT01_WID      2
#define SYSCTRL_WBCR_CRM_SPA_EMI_BIT23_WID      2

//DSCR1
#define SYSCTRL_DSCR1_DS_SLOW11_DVS_PMIC_WID 		2
#define SYSCTRL_DSCR1_DS_SLOW10_SDHC1_CSPI3_WID 	2
#define SYSCTRL_DSCR1_DS_SLOW9_JTAG_WID 		2
#define SYSCTRL_DSCR1_DS_SLOW8_PWM_WID 		2
#define SYSCTRL_DSCR1_DS_SLOW7_CSPI1_WID 	2
#define SYSCTRL_DSCR1_DS_SLOW6_SSI1_WID 		2
#define SYSCTRL_DSCR1_DS_SLOW5_GPT1_WID 		2
#define SYSCTRL_DSCR1_DS_SLOW4_USBH1_WID 	2
#define SYSCTRL_DSCR1_DS_SLOW3_CSI_WID 		2
#define SYSCTRL_DSCR1_DS_SLOW2_SDHC2_MSHC_WID 	2
#define SYSCTRL_DSCR1_DS_SLOW1_LCDC_WID 		2

//PSCR
#define SYSCTRL_PSCR_SD2_D0_PUENCR_WID	2
#define SYSCTRL_PSCR_SD2_D1_PUENCR_WID	2
#define SYSCTRL_PSCR_SD2_D2_PUENCR_WID	2
#define SYSCTRL_PSCR_SD2_D3_PUENCR_WID	2
#define SYSCTRL_PSCR_SD2_CMD_PUENCR_WID	2
#define SYSCTRL_PSCR_SD2_CLK_PUENCR_WID	2
#define SYSCTRL_PSCR_SD1_D3_PUENCR_WID	2
#define SYSCTRL_PSCR_ATA_DAT3_PUENCR_WID	2

// PCSR
#define SYSCTRL_PCSR_S0_AMPR_SEL_WID            1
#define SYSCTRL_PCSR_S1_AMPR_SEL_WID            1
#define SYSCTRL_PCSR_S2_AMPR_SEL_WID            1
#define SYSCTRL_PCSR_S3_AMPR_SEL_WID            1

// PMCR
#define SYSCTRL_PMCR_DPTEN_WID                  1
#define SYSCTRL_PMCR_DIE_WID                    1
#define SYSCTRL_PMCR_DIM_WID                    2
#define SYSCTRL_PMCR_DRCE0_WID                  1
#define SYSCTRL_PMCR_DRCE1_WID                  1
#define SYSCTRL_PMCR_DRCE2_WID                  1
#define SYSCTRL_PMCR_DRCE3_WID                  1
#define SYSCTRL_PMCR_RCLKON_WID                 1
#define SYSCTRL_PMCR_DCR_WID                    1
#define SYSCTRL_PMCR_REFCOUNTER_WID             11
#define SYSCTRL_PMCR_LO_WID                     1
#define SYSCTRL_PMCR_UP_WID                     1
#define SYSCTRL_PMCR_EM_WID                     1
#define SYSCTRL_PMCR_MC_WID                     1

//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------
// FMCR
#define SYSCTRL_FMCR_SDCS0_SEL_USE_CS2          0
#define SYSCTRL_FMCR_SDCS0_SEL_USE_CSD0         1

#define SYSCTRL_FMCR_SDCS1_SEL_USE_CS3          0
#define SYSCTRL_FMCR_SDCS1_SEL_USE_CSD1         1

#define SYSCTRL_FMCR_SLCDC_SEL_SLCDC            0
#define SYSCTRL_FMCR_SLCDC_SEL_BB               1

#define SYSCTRL_FMCR_NF_16BIT_SEL_8BIT          0
#define SYSCTRL_FMCR_NF_16BIT_SEL_16BIT         1

#define SYSCTRL_FMCR_NF_FMS_512B_PAGE           0
#define SYSCTRL_FMCR_NF_FMS_2KB_PAGE            1

#define SYSCTRL_FMCR_IOIS16_CTL_PC17_BOUT       0
#define SYSCTRL_FMCR_IOIS16_CTL_IOIS16          1

#define SYSCTRL_FMCR_PC_BVD2_CTL_PC18_BOUT      0
#define SYSCTRL_FMCR_PC_BVD2_CTL_PC_BVD2        1

#define SYSCTRL_FMCR_PC_BVD1_CTL_PC19_BOUT      0
#define SYSCTRL_FMCR_PC_BVD1_CTL_PC_BVD1        1

#define SYSCTRL_FMCR_PC_VS2_CTL_PC28_BOUT       0
#define SYSCTRL_FMCR_PC_VS2_CTL_PC_VS2          1

#define SYSCTRL_FMCR_PC_VS1_CTL_PC29_BOUT       0
#define SYSCTRL_FMCR_PC_VS1_CTL_PC_VS1          1

#define SYSCTRL_FMCR_PC_READY_CTL_PC30_BOUT     0
#define SYSCTRL_FMCR_PC_READY_CTL_PC_READY      1

#define SYSCTRL_FMCR_PC_WAIT_B_CTL_PC31_BOUT    0
#define SYSCTRL_FMCR_PC_WAIT_B_CTL_PC_WAIT_B    1

#define SYSCTRL_FMCR_KP_ROW6_CTL_PE1_ALT        0
#define SYSCTRL_FMCR_KP_ROW6_CTL_PE7_ALT        1

#define SYSCTRL_FMCR_KP_ROW7_CTL_PE2_ALT        0
#define SYSCTRL_FMCR_KP_ROW7_CTL_PE4_ALT        1

#define SYSCTRL_FMCR_KP_COL6_CTL_PE0_ALT        0
#define SYSCTRL_FMCR_KP_COL6_CTL_PE6_ALT        1

#define SYSCTRL_FMCR_UART4_RTS_CTL_PB31_AOUT    0
#define SYSCTRL_FMCR_UART4_RTS_CTL_PB26_ALT     1

#define SYSCTRL_FMCR_UART4_RXD_CTL_PB29_AOUT    0
#define SYSCTRL_FMCR_UART4_RXD_CTL_PB31_ALT     1

// GPCR
#define SYSCTRL_GPCR_CLK_DDR_MODE_IPP_DSE01     0
#define SYSCTRL_GPCR_CLK_DDR_MODE_SSTL_18       1

#define SYSCTRL_GPCR_DDR_MODE_IPP_DSE01         0
#define SYSCTRL_GPCR_DDR_MODE_SSTL_18           1

#define SYSCTRL_GPCR_CLOCK_GATING_EN_DISABLE    0
#define SYSCTRL_GPCR_CLOCK_GATING_EN_ENABLE     1

#define SYSCTRL_GPCR_DMA_BURST_OVERRIDE_BYPASS  0
#define SYSCTRL_GPCR_DMA_BURST_OVERRIDE_INCR    1

#define SYSCTRL_GPCR_PP_BURST_OVERRIDE_BYPASS   0
#define SYSCTRL_GPCR_PP_BURST_OVERRIDE_INCR     1

#define SYSCTRL_GPCR_USB_BURST_OVERRIDE_BYPASS  0
#define SYSCTRL_GPCR_USB_BURST_OVERRIDE_INCR    1

#define SYSCTRL_GPCR_ETM9_PAD_EN_DISABLE        0
#define SYSCTRL_GPCR_ETM9_PAD_EN_ENABLE         1

#define SYSCTRL_GPCR_BOOT_UART_USB1             0
#define SYSCTRL_GPCR_BOOT_UART_USB2             1
#define SYSCTRL_GPCR_BOOT_NF_8BIT_2KB_PAGE      2
#define SYSCTRL_GPCR_BOOT_NF_16BIT_2KB_PAGE     3
#define SYSCTRL_GPCR_BOOT_NF_16BIT_512B_PAGE    4
#define SYSCTRL_GPCR_BOOT_16BIT_CS0             5
#define SYSCTRL_GPCR_BOOT_32BIT_CS0             6
#define SYSCTRL_GPCR_BOOT_NF_8BIT_512B_PAGE     7

//DSCR
#define SYSCTRL_DSCR_DRIVING_STRENGTH_NORMAL 		0
#define SYSCTRL_DSCR_DRIVING_STRENGTH_HIGH 		1
#define SYSCTRL_DSCR_DRIVING_STRENGTH_MAX 		3

//PSCR
#define SYSCTRL_PSCR_100K_PD		0
#define SYSCTRL_PSCR_100K_PU		1
#define SYSCTRL_PSCR_47K_PU		2
#define SYSCTRL_PSCR_22K_PU		3

// PMCR
#define SYSCTRL_PMCR_DPTEN_DPTC_DISABLE         0
#define SYSCTRL_PMCR_DPTEN_DPTC_ENABLE          1

#define SYSCTRL_PMCR_DIE_DPTC_DISABLE           0
#define SYSCTRL_PMCR_DIE_DPTC_ENABLE            1

#define SYSCTRL_PMCR_DIM_DPTC_ALL               0
#define SYSCTRL_PMCR_DIM_DPTC_LOWER             1
#define SYSCTRL_PMCR_DIM_DPTC_UPPER             0
#define SYSCTRL_PMCR_DIM_DPTC_EMER              1

#define SYSCTRL_PMCR_DRCE_DISABLE               0
#define SYSCTRL_PMCR_DRCE_ENABLE                1

#define SYSCTRL_PMCR_RCLKON_DPTC_NORMAL         0
#define SYSCTRL_PMCR_RCLKON_DPTC_ALWAYS         1

#define SYSCTRL_PMCR_DCR_128_CLOCKS             0
#define SYSCTRL_PMCR_DCR_256_CLOCKS             1


#ifdef __cplusplus
}
#endif

#endif // __MX27_SYSCTRL_H__
