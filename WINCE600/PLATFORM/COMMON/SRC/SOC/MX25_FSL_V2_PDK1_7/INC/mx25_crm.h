//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
// Header:  mx25_crm.h
//
// Provides definitions for PLL, Clock & Reset controller module based on MX25.
//
//------------------------------------------------------------------------------
#ifndef __MX25_CRM_H__
#define __MX25_CRM_H__

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------
#define CRM_CGR_CG_MASK                 (0x1)
#define CRM_PCDR_DIV_MASK               (0x3F)
#define CRM_MCR_PER_LCK_MUX_MASK        (0xFF)

//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------
typedef struct {
    REG32 MPCTL;    //MCU PLL Control Register
    REG32 UPCTL;    //USB PLL Control Register
    REG32 CCTL;     //Clock Control Register
    union {
        struct
        {
            REG32 CGCR0;    //Clock Gating Control Register 0
            REG32 CGCR1;    //Clock Gating Control Register 1
            REG32 CGCR2;    //Clock Gating Control Register 2
        } s;
        REG32 CGR[3];
    }CGR_REGS;
    union {
        struct
        {
            REG32 PCDR0;    //Per Clock Divider Register 0
            REG32 PCDR1;    //Per Clock Divider Register 1
            REG32 PCDR2;    //Per Clock Divider Register 2
            REG32 PCDR3;    //Per Clock Divider Register 3
        } s;
        REG32 PCDR[4];
    }PCDR_REGS;
    REG32 RCSR;     //CRM Status Register
    REG32 CRDR;     //CRM Debug Register
    union {
        struct
        {
            REG32 DCVR0;    //DPTC Comparator Value Registers
            REG32 DCVR1;    //DPTC Comparator Value Registers
            REG32 DCVR2;    //DPTC Comparator Value Registers
            REG32 DCVR3;    //DPTC Comparator Value Registers   
        } s;
        REG32 DCVR[4];
    }DCVR_REGS;
    REG32 LTR0;     //Load Tracking Register 0
    REG32 LTR1;     //Load Tracking Register 1
    REG32 LTR2;     //Load Tracking Register 2
    REG32 LTR3;     //Load Tracking Register 3
    REG32 LTBR0;    //Load Tracking Buffer Register 0
    REG32 LTBR1;    //Load Tracking Buffer Register 1
    REG32 PMCR0;    //Power Management Control Register 0
    REG32 PMCR1;    //Power Management Control Register 1
    REG32 PMCR2;    //Power Management Control Register 2
    REG32 MCR;      //Misc Control Register
    REG32 LPIMR0;   //Low Power Interrupt Mask Registers
    REG32 LPIMR1;   //Low Power Interrupt Mask Registers
} CSP_CRM_REGS, *PCSP_CRM_REGS;

//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
#define CRM_MPCTL_MFN_LSH                       0
#define CRM_MPCTL_MFI_LSH                       10
#define CRM_MPCTL_LOCK_LSH                      15
#define CRM_MPCTL_MFD_LSH                       16
#define CRM_MPCTL_PDF_LSH                       26
#define CRM_MPCTL_BRMO_LSH                      31

#define CRM_UPCTL_MFN_LSH                       0
#define CRM_UPCTL_MFI_LSH                       10
#define CRM_UPCTL_LOCK_LSH                      15
#define CRM_UPCTL_MFD_LSH                       16
#define CRM_UPCTL_PDF_LSH                       26
#define CRM_UPCTL_BRMO_LSH                      31

#define CRM_CCTL_ARMCLK_DIV_LSH                 30
#define CRM_CCTL_AHBCLK_DIV_LSH                 28
#define CRM_CCTL_MPLL_RST_LSH                   27
#define CRM_CCTL_UPLL_RST_LSH                   26
#define CRM_CCTL_LP_CTL_LSH                     24
#define CRM_CCTL_UPLL_DIS_LSH                   23
#define CRM_CCTL_MPLL_BYPASS_LSH                22
#define CRM_CCTL_USB_DIV_LSH                    16
#define CRM_CCTL_CG_CTL_LSH                     15
#define CRM_CCTL_ARM_SRC_LSH                    14

#define CRM_SCSR_CLK_SEL_LSH                    12

#define CRM_DCVR_ELV_LSH                        2
#define CRM_DCVR_LLV_LSH                        12
#define CRM_DCVR_ULV_LSH                        22


#define CRM_PMCR0_DPTEN_LSH                     0
#define CRM_PMCR0_PTVAI_LSH                     1
#define CRM_PMCR0_PTVAIM_LSH                    3
#define CRM_PMCR0_DVFEN_LSH                     4
#define CRM_PMCR0_SCR_LSH                       5
#define CRM_PMCR0_DRCE0_LSH                     6
#define CRM_PMCR0_DRCE1_LSH                     7
#define CRM_PMCR0_DRCE2_LSH                     8
#define CRM_PMCR0_DRCE3_LSH                     9
#define CRM_PMCR0_WFIM_LSH                      10
#define CRM_PMCR0_DPVV_LSH                      11
#define CRM_PMCR0_DPVCR_LSH                     12
#define CRM_PMCR0_FSVAI_LSH                     13
#define CRM_PMCR0_FSVAIM_LSH                    15
#define CRM_PMCR0_DVFS_START_LSH                16
#define CRM_PMCR0_PTVIS_LSH                     17
#define CRM_PMCR0_LBCF_LSH                      18
#define CRM_PMCR0_LBFL_LSH                      20
#define CRM_PMCR0_LBMI_LSH                      21
#define CRM_PMCR0_DVFIS_LSH                     22
#define CRM_PMCR0_DVFEV_LSH                     23
#define CRM_PMCR0_DVFS_UPD_FINISH_LSH           24
#define CRM_PMCR0_DVSUP_LSH                     28
#define CRM_PMCR0_DVS1_LSH                      28
#define CRM_PMCR0_DVS0_LSH                      29

#define CRM_PMCR1_DVGP_LSH                      0
#define CRM_PMCR1_CPEN_LSH                      13
#define CRM_PMCR1_CPEN_EMI_LSH                  29

#define CRM_PMCR2_OSC24M_DOWN_LSH               16

#define CRM_RCSR_REST_LSH                       0
#define CRM_RCSR_NFC_FMS_LSH                    8
#define CRM_RCSR_NFC_4K_LSH                     9
#define CRM_RCSR_MEM_CTRL_LSH                   30
#define CRM_RCSR_MEM_TYPE_LSH                   28

#define CRM_LTR0_DIV3CK_LSH                     1
#define CRM_LTR0_SIGD0_LSH                      3
#define CRM_LTR0_SIGD12_LSH                     15
#define CRM_LTR0_DNTHR_LSH                      16
#define CRM_LTR0_UPTHR_LSH                      22
#define CRM_LTR0_SIGD13_LSH                     29
#define CRM_LTR0_SIGD14_LSH                     30
#define CRM_LTR0_SIGD15_LSH                     31

#define CRM_LTR1_PNCTHR_LSH                     0
#define CRM_LTR1_UPCNT_LSH                      6
#define CRM_LTR1_DNCNT_LSH                      14
#define CRM_LTR1_LTBRSR_LSH                     22
#define CRM_LTR1_LTBRSH_LSH                     23

#define CRM_LTR2_EMAC_LSH                       0
#define CRM_LTR2_WSW9_LSH                       11
#define CRM_LTR3_WSW0_LSH                       5

// MCR Misc Control Register
#define CRM_MCR_USB_XTAL_MUX_LSH                31
#define CRM_MCR_CLKO_EN_LSH                     30
#define CRM_MCR_CLKO_DIV_LSH                    24
#define CRM_MCR_CLKO_SEL_LSH                    20
#define CRM_MCR_ESAI_CLK_MUX_LSH                19
#define CRM_MCR_SSI1_CLK_MUX_LSH                18
#define CRM_MCR_SSI2_CLK_MUX_LSH                17
#define CRM_MCR_USB_CLK_MUX_LSH                 16
#define CRM_MCR_PER_CLK_MUX_LSH                 0

//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
#define CRM_MPCTL_MFN_WID                       10
#define CRM_MPCTL_MFI_WID                       4
#define CRM_MPCTL_LOCK_WID                      1
#define CRM_MPCTL_MFD_WID                       10
#define CRM_MPCTL_PDF_WID                       4
#define CRM_MPCTL_BRMO_WID                      1

#define CRM_UPCTL_MFN_WID                       10
#define CRM_UPCTL_MFI_WID                       4
#define CRM_UPCTL_LOCK_WID                      1
#define CRM_UPCTL_MFD_WID                       10
#define CRM_UPCTL_PDF_WID                       4
#define CRM_UPCTL_BRMO_WID                      1

#define CRM_CCTL_ARMCLK_DIV_WID                 2
#define CRM_CCTL_AHBCLK_DIV_WID                 2
#define CRM_CCTL_MPLL_RST_WID                   1
#define CRM_CCTL_UPLL_RST_WID                   1
#define CRM_CCTL_LP_CTL_WID                     2
#define CRM_CCTL_UPLL_DIS_WID                   1
#define CRM_CCTL_MPLL_BYPASS_WID                1
#define CRM_CCTL_USB_DIV_WID                    6
#define CRM_CCTL_CG_CTL_WID                     1
#define CRM_CCTL_ARM_SRC_WID                    1

#define CRM_SCSR_CLK_SEL_WID                    1

#define CRM_DCVR_ELV_WID                        10
#define CRM_DCVR_LLV_WID                        10
#define CRM_DCVR_ULV_WID                        10

#define CRM_LTR0_SIGD_WID                       16
#define CRM_LTR0_DNTHR_WID                      6
#define CRM_LTR0_UPTHR_WID                      6
#define CRM_LTR0_DIV3CK_WID                     2

#define CRM_LTR1_PNCTHR_WID                     6
#define CRM_LTR1_UPCNT_WID                      8
#define CRM_LTR1_DNCNT_WID                      8
#define CRM_LTR1_LTBRSR_WID                     1
#define CRM_LTR1_LTBRSH_WID                     1

#define CRM_LTR2_EMAC_WID                       9

#define CRM_LTR_WSW_WID                         3

#define CRM_PMCR0_DPTEN_WID                     1
#define CRM_PMCR0_PTVAI_WID                     2
#define CRM_PMCR0_PTVAIM_WID                    1
#define CRM_PMCR0_DVFEN_WID                     1
#define CRM_PMCR0_SCR_WID                       1
#define CRM_PMCR0_DRCE0_WID                     1
#define CRM_PMCR0_DRCE1_WID                     1
#define CRM_PMCR0_DRCE2_WID                     1
#define CRM_PMCR0_DRCE3_WID                     1
#define CRM_PMCR0_WFIM_WID                      1
#define CRM_PMCR0_DPVV_WID                      1
#define CRM_PMCR0_DPVCR_WID                     1
#define CRM_PMCR0_FSVAI_WID                     2
#define CRM_PMCR0_FSVAIM_WID                    1
#define CRM_PMCR0_DVFS_START_WID                1
#define CRM_PMCR0_PTVIS_WID                     1
#define CRM_PMCR0_LBCF_WID                      2
#define CRM_PMCR0_LBFL_WID                      1
#define CRM_PMCR0_LBMI_WID                      1
#define CRM_PMCR0_DVFIS_WID                     1
#define CRM_PMCR0_DVFEV_WID                     1
#define CRM_PMCR0_DVFS_UPD_FINISH_WID           1
#define CRM_PMCR0_DVSUP_WID                     2
#define CRM_PMCR0_DVS1_WID                      1
#define CRM_PMCR0_DVS0_WID                      1

#define CRM_PMCR1_DVGP_WID                      4
#define CRM_PMCR1_CPEN_WID                      1
#define CRM_PMCR1_CPEN_EMI_WID                  1

#define CRM_PMCR2_OSC24M_DOWN_WID               1

#define CRM_RCSR_REST_WID                       4
#define CRM_RCSR_NFC_FMS_WID                    1
#define CRM_RCSR_NFC_4K_WID                     1
#define CRM_RCSR_MEM_CTRL_WID                   2
#define CRM_RCSR_MEM_TYPE_WID                   2

// MCR Misc Control Register
#define CRM_MCR_USB_XTAL_MUX_WID                1
#define CRM_MCR_CLKO_EN_WID                     1
#define CRM_MCR_CLKO_DIV_WID                    6
#define CRM_MCR_CLKO_SEL_WID                    4
#define CRM_MCR_ESAI_CLK_MUX_WID                1
#define CRM_MCR_SSI1_CLK_MUX_WID                1
#define CRM_MCR_SSI2_CLK_MUX_WID                1
#define CRM_MCR_USB_CLK_MUX_WID                 1
#define CRM_MCR_PER_CLK_MUX_WID                 16

//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------
//RSCR
#define CRM_RCSR_MEM_CTRL_WEIM                  0
#define CRM_RCSR_MEM_CTRL_NAND                  1
#define CRM_RCSR_MEM_CTRL_EXPANSION_DEV         3
#define CRM_RCSR_MEM_TYPE_EXPANSION_SD          0
#define CRM_RCSR_MEM_TYPE_EXPANSION_I2C_ROM     2
#define CRM_RCSR_MEM_TYPE_EXPANSION_SPI_FLASH   3
#define CRM_RCSR_NFC_FMS_NOT2K                  0
#define CRM_RCSR_NFC_FMS_2K                     1
#define CRM_RCSR_NFC_4K_NOT4K                   0
#define CRM_RCSR_NFC_4K_4K                      1

#define CRM_RCSR_REST_POR                   0
#define CRM_RCSR_REST_RIR                   1
#define CRM_RCSR_REST_WDOG                  2
#define CRM_RCSR_REST_SOFT                  4
#define CRM_RCSR_REST_JTAG                  8

// PMCR0
#define CRM_PMCR0_DPTEN_DPTC_DISABLE            0
#define CRM_PMCR0_DPTEN_DPTC_ENABLE             1

#define CRM_PMCR0_PTVAI_VOLT_NO_CHANGE          0
#define CRM_PMCR0_PTVAI_VOLT_DOWN               1
#define CRM_PMCR0_PTVAI_VOLT_UP                 2
#define CRM_PMCR0_PTVAI_VOLT_UP_PANIC           3

#define CRM_PMCR0_PTVAIM_UNMASK                 0
#define CRM_PMCR0_PTVAIM_MASK                   1

#define CRM_PMCR0_DVFEN_DVFS_DISABLE            0
#define CRM_PMCR0_DVFEN_DVFS_ENABLE             1

#define CRM_PMCR0_SCR_256_CLOCKS                0
#define CRM_PMCR0_SCR_512_CLOCKS                1

#define CRM_PMCR0_DRCE_DISABLE                  0
#define CRM_PMCR0_DRCE_ENABLE                   1

#define CRM_PMCR0_WFIM_UNMASK                   0
#define CRM_PMCR0_WFIM_MASK                     1

#define CRM_PMCR0_DPVV_VOLT_INVALID             0
#define CRM_PMCR0_DPVV_VOLT_VALID               1

#define CRM_PMCR0_DPVCR_NO_CHANGE_REQ           0
#define CRM_PMCR0_DPVCR_CHANGE_REQ              1

#define CRM_PMCR0_FSVAI_FREQ_NO_CHANGE          0
#define CRM_PMCR0_FSVAI_FREQ_UP                 1
#define CRM_PMCR0_FSVAI_FREQ_DOWN               2
#define CRM_PMCR0_FSVAI_FREQ_UP_PANIC           3

#define CRM_PMCR0_FSVAIM_UNMASK                 0
#define CRM_PMCR0_FSVAIM_MASK                   1

#define CRM_PMCR0_DVFS_START_NOT                0
#define CRM_PMCR0_DVFS_START_UPDATING           1

#define CRM_PMCR0_PTVIS_SDMA_DPTC_REQ           0
#define CRM_PMCR0_PTVIS_MCU_DPTC_REQ            1

#define CRM_PMCR0_LBCF_BUF_SIZE_4               0
#define CRM_PMCR0_LBCF_BUF_SIZE_8               1
#define CRM_PMCR0_LBCF_BUF_SIZE_12              2
#define CRM_PMCR0_LBCF_BUF_SIZE_16              3

#define CRM_PMCR0_LBFL_BUF_NOT_FULL             0
#define CRM_PMCR0_LBFL_BUF_FULL                 1

#define CRM_PMCR0_LBMI_UNMASKED                 0
#define CRM_PMCR0_LBMI_MASKED                   1

#define CRM_PMCR0_DVFIS_SDMA_DVFS_REQ           0
#define CRM_PMCR0_DVFIS_MCU_DVFS_REQ            1

#define CRM_PMCR0_DVFEV_REQ_NOT_ALWAYS          0
#define CRM_PMCR0_DVFEV_REQ_ALWAYS              1

#define CRM_PMCR0_DVFS_UPD_FINISH_NOT           0
#define CRM_PMCR0_DVFS_UPD_FINISH_FINISHED      1

// PMCR1
#define CRM_PMCR1_CPEN_ENABLE                   1
#define CRM_PMCR1_CPEN_EMI_ENABLE               1

// PMCR2
#define CRM_PMCR2_OSC24M_DOWN_DOWN              1

// CCTL
#define CRM_CCTL_LP_CTL_RUN                     0
#define CRM_CCTL_LP_CTL_WAIT                    1
#define CRM_CCTL_LP_CTL_DOZE                    2
#define CRM_CCTL_LP_CTL_SLEEP                   3

// MCR Misc Control Register
#define CRM_MCR_USB_XTAL_EXT24M                 0
#define CRM_MCR_USB_XTAL_FROM_UPLL              1

#define CRM_MCR_CLKO_DISABLE                    0
#define CRM_MCR_CLKO_ENABLE                     1

#define CRM_MCR_ESAI_CLK_MUX_NORMAL             0
#define CRM_MCR_ESAI_CLK_MUX_DEDICATED          1

#define CRM_MCR_SSI1_CLK_MUX_NORMAL             0
#define CRM_MCR_SSI1_CLK_MUX_DEDICATED          1

#define CRM_MCR_SSI2_CLK_MUX_NORMAL             0
#define CRM_MCR_SSI2_CLK_MUX_DEDICATED          1

#define CRM_MCR_USB_CLK_MUX_UPLL                0
#define CRM_MCR_USB_CLK_MUX_HCLK                1

#define CRM_MCR_PER_CLK_MUX_HCLK                0
#define CRM_MCR_PER_CLK_MUX_UPLL                1



//------------------------------------------------------------------------------
// HELPER MACROS
//------------------------------------------------------------------------------
#define CRM_CGR_INDEX(gateIndex)        ((gateIndex) >> 5)
#define CRM_CGR_SHIFT(gateIndex)        ((gateIndex) & (31))
#define CRM_CGR_MASK(gateIndex)         (CRM_CGR_CG_MASK <<  CRM_CGR_SHIFT(gateIndex))
#define CRM_CGR_VAL(gateIndex, val)     (val << CRM_CGR_SHIFT(gateIndex))

#define CRM_PCDR_INDEX(ClkIndex)        ((ClkIndex) >> 2)
#define CRM_PCDR_SHIFT(ClkIndex)        (((ClkIndex) & (3))*8)
#define CRM_PCDR_MASK(ClkIndex)         (CRM_PCDR_DIV_MASK << (CRM_PCDR_SHIFT(ClkIndex)))
#define CRM_PCDR_VAL(ClkIndex)          (val << (CRM_PCDR_SHIFT(ClkIndex)))
#define CRM_PCDR_VAL_EXTRACT(val,ClkIndex)  ((val >> (CRM_PCDR_SHIFT(ClkIndex)))& CRM_PCDR_DIV_MASK)


#ifdef __cplusplus
}
#endif

#endif // __MX25_CRM_H__
