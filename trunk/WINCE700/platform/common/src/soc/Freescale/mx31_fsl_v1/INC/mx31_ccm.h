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
//  Header:  MX31_ccm.h
//
//  Provides the public interface for the CCM module.  This module defines
//  the header information for the application processor's clock and reset
//  module.
//
//------------------------------------------------------------------------------

#ifndef __MX31_CCM_H__
#define __MX31_CCM_H__

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
    UINT32 RCSR;
    UINT32 MPCTL;
    UINT32 UPCTL;
    UINT32 SRPCTL;
    UINT32 COSR;
    UINT32 CGR[3];
    UINT32 WIMR;
    UINT32 LDC;
    UINT32 DCVR[4];
    UINT32 LTR0;
    UINT32 LTR1;
    UINT32 LTR2;
    UINT32 LTR3;
    UINT32 LTBR0;
    UINT32 LTBR1;
    UINT32 PMCR0;
    UINT32 PMCR1;
    UINT32 PDR;  
} CSP_CCM_REGS, *PCSP_CCM_REGS;

//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
#define CCM_CCMR_OFFSET                 0x0000
#define CCM_PDR0_OFFSET                 0x0004
#define CCM_PDR1_OFFSET                 0x0008
#define CCM_RCSR_OFFSET                 0x000C
#define CCM_MPCTL_OFFSET                0x0010
#define CCM_UPCTL_OFFSET                0x0014
#define CCM_SRPCTL_OFFSET               0x0018
#define CCM_COSR_OFFSET                 0x001C
#define CCM_CGR_OFFSET                  0x0020
#define CCM_WIMR_OFFSET                 0x002C
#define CCM_LDC_OFFSET                  0x0030
#define CCM_DCVR_OFFSET                 0x0034
#define CCM_LTR0_OFFSET                 0x0044
#define CCM_LTR1_OFFSET                 0x0048
#define CCM_LTR2_OFFSET                 0x004C
#define CCM_LTR3_OFFSET                 0x0050
#define CCM_LTBR0_OFFSET                0x0054
#define CCM_LTBR1_OFFSET                0x0058
#define CCM_PMCR0_OFFSET                0x005C
#define CCM_PMCR1_OFFSET                0x0060
#define CCM_PDR_OFFSET                  0x0064


//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
#define CCM_CCMR_FPME_LSH               0
#define CCM_CCMR_PRCS_LSH               1
#define CCM_CCMR_MPE_LSH                3
#define CCM_CCMR_SBYCS_LSH              4
#define CCM_CCMR_MDS_LSH                7
#define CCM_CCMR_SPE_LSH                8
#define CCM_CCMR_UPE_LSH                9
#define CCM_CCMR_FIRS_LSH               11
#define CCM_CCMR_LPM_LSH                14
#define CCM_CCMR_SSI1S_LSH              18
#define CCM_CCMR_SSI2S_LSH              21
#define CCM_CCMR_PERCS_LSH              24
#define CCM_CCMR_CSCS_LSH               25
#define CCM_CCMR_FPMF_LSH               26
#define CCM_CCMR_WBEN_LSH               27
#define CCM_CCMR_VSTBY_LSH              28
#define CCM_CCMR_L2PG_LSH               29

#define CCM_PDR0_MCU_PODF_LSH           0
#define CCM_PDR0_MAX_PODF_LSH           3
#define CCM_PDR0_IPG_PODF_LSH           6
#define CCM_PDR0_NFC_PODF_LSH           8
#define CCM_PDR0_HSP_PODF_LSH           11
#define CCM_PDR0_PER_PODF_LSH           16
#define CCM_PDR0_CSI_PODF_LSH           23

#define CCM_PDR1_SSI1_PODF_LSH          0
#define CCM_PDR1_SSI1_PRE_PODF_LSH      6
#define CCM_PDR1_SSI2_PODF_LSH          9
#define CCM_PDR1_SSI2_PRE_PODF_LSH      15
#define CCM_PDR1_FIRI_PODF_LSH          18
#define CCM_PDR1_FIRI_PRE_PODF_LSH      24
#define CCM_PDR1_USB_PODF_LSH           27
#define CCM_PDR1_USB_PRDF_LSH           30

#define CCM_RCSR_REST_LSH               0
#define CCM_RCSR_GPF_LSH                4
#define CCM_RCSR_SDM_LSH                12
#define CCM_RCSR_MPRES_LSH              15
#define CCM_RCSR_OSCNT_LSH              16
#define CCM_RCSR_BTP_LSH                23
#define CCM_RCSR_NFMS_LSH               30
#define CCM_RCSR_NF16B_LSH              31

#define CCM_MPCTL_MFN_LSH               0
#define CCM_MPCTL_MFI_LSH               10
#define CCM_MPCTL_MFD_LSH               16
#define CCM_MPCTL_PDF_LSH               26
#define CCM_MPCTL_BRMO_LSH              31

#define CCM_UPCTL_MFN_LSH               0
#define CCM_UPCTL_MFI_LSH               10
#define CCM_UPCTL_MFD_LSH               16
#define CCM_UPCTL_PDF_LSH               26
#define CCM_UPCTL_BRMO_LSH              31

#define CCM_SPCTL_MFN_LSH               0
#define CCM_SPCTL_MFI_LSH               10
#define CCM_SPCTL_MFD_LSH               16
#define CCM_SPCTL_PDF_LSH               26
#define CCM_SPCTL_BRMO_LSH              31

#define CCM_COSR_CLKOSEL_LSH            0
#define CCM_COSR_CLKODIV_LSH            6
#define CCM_COSR_CLKOEN_LSH             9

#define CCM_DCVR_ELV_LSH                2
#define CCM_DCVR_LLV_LSH                12
#define CCM_DCVR_ULV_LSH                22

#define CCM_LTR0_DIV3CK_LSH             1
#define CCM_LTR0_SIGD0_LSH              3
#define CCM_LTR0_DNTHR_LSH              16
#define CCM_LTR0_UPTHR_LSH              22
#define CCM_LTR0_SIGD13_LSH             29

#define CCM_LTR1_PNCTHR_LSH             0
#define CCM_LTR1_UPCNT_LSH              6
#define CCM_LTR1_DNCNT_LSH              14
#define CCM_LTR1_LTBRSR_LSH             22
#define CCM_LTR1_LTBRSH_LSH             23

#define CCM_LTR2_EMAC_LSH               0
#define CCM_LTR2_WSW9_LSH               9

#define CCM_LTR3_WSW0_LSH               5

#define CCM_PMCR0_DPTEN_LSH             0
#define CCM_PMCR0_PTVAI_LSH             1
#define CCM_PMCR0_PTVAIM_LSH            3
#define CCM_PMCR0_DVFEN_LSH             4
#define CCM_PMCR0_DCR_LSH               5
#define CCM_PMCR0_DRCE0_LSH             6
#define CCM_PMCR0_DRCE1_LSH             7
#define CCM_PMCR0_DRCE2_LSH             8
#define CCM_PMCR0_DRCE3_LSH             9
#define CCM_PMCR0_WFIM_LSH              10
#define CCM_PMCR0_DPVV_LSH              11
#define CCM_PMCR0_DPVCR_LSH             12
#define CCM_PMCR0_FSVAI_LSH             13
#define CCM_PMCR0_FSVAIM_LSH            15
#define CCM_PMCR0_UPDTEN_LSH            16
#define CCM_PMCR0_PTVIS_LSH             17
#define CCM_PMCR0_LBCF_LSH              18
#define CCM_PMCR0_LBFL_LSH              20
#define CCM_PMCR0_LBMI_LSH              21
#define CCM_PMCR0_DVFIS_LSH             22
#define CCM_PMCR0_DVFEV_LSH             23
#define CCM_PMCR0_VSCNT_LSH             24
#define CCM_PMCR0_UDSC_LSH              27
#define CCM_PMCR0_DVSUP_LSH             28
#define CCM_PMCR0_DVS1_LSH              28
#define CCM_PMCR0_DVS0_LSH              29
#define CCM_PMCR0_DFSUP0_LSH            30
#define CCM_PMCR0_DFSUP1_LSH            31

#define CCM_PMCR1_DVGP_LSH              0
#define CCM_PMCR1_CPFA_LSH              6
#define CCM_PMCR1_NWTS_LSH              7
#define CCM_PMCR1_PWTS_LSH              8
#define CCM_PMCR1_CPSPA_LSH             9
#define CCM_PMCR1_WBCN_LSH              16

//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
#define CCM_CCMR_FPME_WID               1
#define CCM_CCMR_PRCS_WID               2
#define CCM_CCMR_MPE_WID                1
#define CCM_CCMR_SBYCS_WID              1
#define CCM_CCMR_MDS_WID                1
#define CCM_CCMR_SPE_WID                1
#define CCM_CCMR_UPE_WID                1
#define CCM_CCMR_FIRS_WID               2
#define CCM_CCMR_LPM_WID                2
#define CCM_CCMR_SSI1S_WID              2
#define CCM_CCMR_SSI2S_WID              2
#define CCM_CCMR_PERCS_WID              1
#define CCM_CCMR_CSCS_WID               1
#define CCM_CCMR_FPMF_WID               1
#define CCM_CCMR_WBEN_WID               1
#define CCM_CCMR_VSTBY_WID              1
#define CCM_CCMR_L2PG_WID               1

#define CCM_PDR0_MCU_PODF_WID           3
#define CCM_PDR0_MAX_PODF_WID           3
#define CCM_PDR0_IPG_PODF_WID           2
#define CCM_PDR0_NFC_PODF_WID           3
#define CCM_PDR0_HSP_PODF_WID           3
#define CCM_PDR0_PER_PODF_WID           5
#define CCM_PDR0_CSI_PODF_WID           9

#define CCM_PDR1_SSI1_PODF_WID          6
#define CCM_PDR1_SSI1_PRE_PODF_WID      3
#define CCM_PDR1_SSI2_PODF_WID          6
#define CCM_PDR1_SSI2_PRE_PODF_WID      3
#define CCM_PDR1_FIRI_PODF_WID          6
#define CCM_PDR1_FIRI_PRE_PODF_WID      3
#define CCM_PDR1_USB_PODF_WID           3
#define CCM_PDR1_USB_PRDF_WID           2

#define CCM_RCSR_REST_WID               3
#define CCM_RCSR_GPF_WID                4
#define CCM_RCSR_SDM_WID                2
#define CCM_RCSR_MPRES_WID              1
#define CCM_RCSR_OSCNT_WID              7
#define CCM_RCSR_BTP_WID                5
#define CCM_RCSR_NFMS_WID               1
#define CCM_RCSR_NF16B_WID              1

#define CCM_MPCTL_MFN_WID               10
#define CCM_MPCTL_MFI_WID               4
#define CCM_MPCTL_MFD_WID               10
#define CCM_MPCTL_PDF_WID               4
#define CCM_MPCTL_BRMO_WID              1

#define CCM_UPCTL_MFN_WID               10
#define CCM_UPCTL_MFI_WID               4
#define CCM_UPCTL_MFD_WID               10
#define CCM_UPCTL_PDF_WID               4
#define CCM_UPCTL_BRMO_WID              1

#define CCM_SPCTL_MFN_WID               10
#define CCM_SPCTL_MFI_WID               4
#define CCM_SPCTL_MFD_WID               10
#define CCM_SPCTL_PDF_WID               4
#define CCM_SPCTL_BRMO_WID              1

#define CCM_COSR_CLKOSEL_WID            4
#define CCM_COSR_CLKODIV_WID            3
#define CCM_COSR_CLKOEN_WID             1

#define CCM_CGR_CG_WID                  2

#define CCM_DCVR_ELV_WID                10
#define CCM_DCVR_LLV_WID                10
#define CCM_DCVR_ULV_WID                10

#define CCM_LTR0_DIV3CK_WID             2
#define CCM_LTR0_DNTHR_WID              6
#define CCM_LTR0_UPTHR_WID              6

#define CCM_LTR1_PNCTHR_WID             6
#define CCM_LTR1_UPCNT_WID              8
#define CCM_LTR1_DNCNT_WID              8
#define CCM_LTR1_LTBRSR_WID             1
#define CCM_LTR1_LTBRSH_WID             1

#define CCM_LTR2_EMAC_WID               9

#define CCM_LTR_WSW_WID                 3

#define CCM_PMCR0_DPTEN_WID             1
#define CCM_PMCR0_PTVAI_WID             2
#define CCM_PMCR0_PTVAIM_WID            1
#define CCM_PMCR0_DVFEN_WID             1
#define CCM_PMCR0_DCR_WID               1
#define CCM_PMCR0_DRCE0_WID             1
#define CCM_PMCR0_DRCE1_WID             1
#define CCM_PMCR0_DRCE2_WID             1
#define CCM_PMCR0_DRCE3_WID             1
#define CCM_PMCR0_WFIM_WID              1
#define CCM_PMCR0_DPVV_WID              1
#define CCM_PMCR0_DPVCR_WID             1
#define CCM_PMCR0_FSVAI_WID             2
#define CCM_PMCR0_FSVAIM_WID            1
#define CCM_PMCR0_UPDTEN_WID            1
#define CCM_PMCR0_PTVIS_WID             1
#define CCM_PMCR0_LBCF_WID              2
#define CCM_PMCR0_LBFL_WID              1
#define CCM_PMCR0_LBMI_WID              1
#define CCM_PMCR0_DVFIS_WID             1
#define CCM_PMCR0_DVFEV_WID             1
#define CCM_PMCR0_VSCNT_WID             3
#define CCM_PMCR0_UDSC_WID              1
#define CCM_PMCR0_DVSUP_WID             2
#define CCM_PMCR0_DVS1_WID              1
#define CCM_PMCR0_DVS0_WID              1
#define CCM_PMCR0_DFSUP0_WID            1
#define CCM_PMCR0_DFSUP1_WID            1

#define CCM_PMCR1_DVGP_WID              4
#define CCM_PMCR1_CPFA_WID              1
#define CCM_PMCR1_NWTS_WID              1
#define CCM_PMCR1_PWTS_WID              1
#define CCM_PMCR1_CPSPA_WID             4
#define CCM_PMCR1_WBCN_WID              8


//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------

// CCMR
#define CCM_CCMR_FPME_DISABLED          0
#define CCM_CCMR_FPME_ENABLED           1

#define CCM_CCMR_PRCS_FPM_REF           1
#define CCM_CCMR_PRCS_CKIH_REF          2

#define CCM_CCMR_MPE_DISABLED           0
#define CCM_CCMR_MPE_ENABLED            1

#define CCM_CCMR_SBYCS_DISABLED         0
#define CCM_CCMR_SBYCS_ENABLED          1

#define CCM_CCMR_MDS_MCUPLL             0
#define CCM_CCMR_MDS_REFCLK             1

#define CCM_CCMR_SPE_DISABLED           0
#define CCM_CCMR_SPE_ENABLED            1

#define CCM_CCMR_UPE_DISABLED           0
#define CCM_CCMR_UPE_ENABLED            1

#define CCM_CCMR_FIRS_MCU_CLK           0
#define CCM_CCMR_FIRS_USB_CLK           1
#define CCM_CCMR_FIRS_SERIAL_CLK        2

#define CCM_CCMR_LPM_RUN                0
#define CCM_CCMR_LPM_DOZE               1
#define CCM_CCMR_LPM_STATE_RETENTION    2
#define CCM_CCMR_LPM_DEEP_SLEEP         3

#define CCM_CCMR_SSI1S_MCU_CLK          0
#define CCM_CCMR_SSI1S_USB_CLK          1
#define CCM_CCMR_SSI1S_SERIAL_CLK       2

#define CCM_CCMR_SSI2S_MCU_CLK          0
#define CCM_CCMR_SSI2S_USB_CLK          1
#define CCM_CCMR_SSI2S_SERIAL_CLK       2

#define CCM_CCMR_PERCS_USB_CLK          0
#define CCM_CCMR_PERCS_IPG_CLK          1

#define CCM_CCMR_CSCS_USB_CLK           0
#define CCM_CCMR_CSCS_SERIAL_CLK        1

#define CCM_CCMR_FPMF_512               0
#define CCM_CCMR_FPMF_1024              1

#define CCM_CCMR_WBEN_DISABLE           0
#define CCM_CCMR_WBEN_ENABLE            1

#define CCM_CCMR_VSTBY_DISABLE          0
#define CCM_CCMR_VSTBY_ENABLE           1

#define CCM_CCMR_L2PG_DISABLE           0
#define CCM_CCMR_L2PG_ENABLE            1

// LTR
#define CCM_LTR1_PNCTHR_FORCE_PANIC     0
#define CCM_LTR_THRESHOLD_DISABLE       63

// PMCR0
#define CCM_PMCR0_DPTEN_DPTC_DISABLE    0
#define CCM_PMCR0_DPTEN_DPTC_ENABLE     1

#define CCM_PMCR0_PTVAI_VOLT_NO_CHANGE  0
#define CCM_PMCR0_PTVAI_VOLT_DOWN       1
#define CCM_PMCR0_PTVAI_VOLT_UP         2
#define CCM_PMCR0_PTVAI_VOLT_UP_PANIC   3

#define CCM_PMCR0_PTVAIM_UNMASK         0
#define CCM_PMCR0_PTVAIM_MASK           1

#define CCM_PMCR0_DVFEN_DVFS_DISABLE    0
#define CCM_PMCR0_DVFEN_DVFS_ENABLE     1

#define CCM_PMCR0_DCR_256_CLOCKS        0
#define CCM_PMCR0_DCR_512_CLOCKS        1

#define CCM_PMCR0_DRCE_DISABLE          0
#define CCM_PMCR0_DRCE_ENABLE           1

#define CCM_PMCR0_WFIM_UNMASK           0
#define CCM_PMCR0_WFIM_MASK             1

#define CCM_PMCR0_DPVV_VOLT_INVALID     0
#define CCM_PMCR0_DPVV_VOLT_VALID       1

#define CCM_PMCR0_DPVCR_NO_CHANGE_REQ   0
#define CCM_PMCR0_DPVCR_CHANGE_REQ      1

#define CCM_PMCR0_FSVAI_FREQ_NO_CHANGE  0
#define CCM_PMCR0_FSVAI_FREQ_UP         1
#define CCM_PMCR0_FSVAI_FREQ_DOWN       2
#define CCM_PMCR0_FSVAI_FREQ_UP_PANIC   3

#define CCM_PMCR0_FSVAIM_UNMASK         0
#define CCM_PMCR0_FSVAIM_MASK           1

#define CCM_PMCR0_UPDTEN_DISABLE_UPDATE 0
#define CCM_PMCR0_UPDTEN_ENABLE_UPDATE  1

#define CCM_PMCR0_PTVIS_SDMA_DPTC_REQ   0
#define CCM_PMCR0_PTVIS_MCU_DPTC_REQ    1

#define CCM_PMCR0_LBCF_BUF_SIZE_4       0
#define CCM_PMCR0_LBCF_BUF_SIZE_8       1
#define CCM_PMCR0_LBCF_BUF_SIZE_12      2
#define CCM_PMCR0_LBCF_BUF_SIZE_16      3

#define CCM_PMCR0_LBFL_BUF_NOT_FULL     0
#define CCM_PMCR0_LBFL_BUF_FULL         1

#define CCM_PMCR0_LBMI_UNMASKED         0
#define CCM_PMCR0_LBMI_MASKED           1

#define CCM_PMCR0_DVFIS_SDMA_DVFS_REQ   0
#define CCM_PMCR0_DVFIS_MCU_DVFS_REQ    1

#define CCM_PMCR0_DVFEV_REQ_NOT_ALWAYS  0
#define CCM_PMCR0_DVFEV_REQ_ALWAYS      1

#define CCM_PMCR0_UDSC_FREQ_DOWN        0
#define CCM_PMCR0_UDSC_FREQ_UP          1

#define CCM_PMCR0_DFSUP0_PLL_UPDATE     0
#define CCM_PMCR0_DFSUP0_POSTDIV_UPDATE 1

#define CCM_PMCR0_DFSUP1_SRPLL_UPDATE   0
#define CCM_PMCR0_DFSUP1_MCUPLL_UPDATE  1

//------------------------------------------------------------------------------
// HELPER MACROS
//------------------------------------------------------------------------------
#define CCM_CGR_INDEX(gateIndex)        ((gateIndex) >> 4)
#define CCM_CGR_SHIFT(gateIndex)        (((gateIndex) % 16) << 1)
#define CCM_CGR_MASK(gateIndex)         (CCM_CGR_CG_MASK <<  CCM_CGR_SHIFT(gateIndex))
#define CCM_CGR_VAL(gateIndex, val)     (val << CCM_CGR_SHIFT(gateIndex))

// CGR Masks
#define CCM_CGR0_SDHC1_MASK             CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_SDHC1)
#define CCM_CGR0_SDHC2_MASK             CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_SDHC2)
#define CCM_CGR0_GPT_MASK               CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_GPT)
#define CCM_CGR0_EPIT1_MASK             CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_EPIT1)
#define CCM_CGR0_EPIT2_MASK             CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_EPIT2)
#define CCM_CGR0_IIM_MASK               CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_IIM)
#define CCM_CGR0_ATA_MASK               CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_ATA)
#define CCM_CGR0_SDMA_MASK              CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_SDMA)
#define CCM_CGR0_CSPI3_MASK             CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_CSPI3)
#define CCM_CGR0_RNG_MASK               CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_RNG)
#define CCM_CGR0_UART1_MASK             CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_UART1)
#define CCM_CGR0_UART2_MASK             CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_UART2)
#define CCM_CGR0_SSI1_MASK              CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_SSI1)
#define CCM_CGR0_I2C1_MASK              CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_I2C1)
#define CCM_CGR0_I2C2_MASK              CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_I2C2)
#define CCM_CGR0_I2C3_MASK              CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_I2C3)

#define CCM_CGR1_MPEG4_MASK             CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_MPEG4)
#define CCM_CGR1_MEMSTICK1_MASK         CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_MEMSTICK1)
#define CCM_CGR1_MEMSTICK2_MASK         CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_MEMSTICK2)
#define CCM_CGR1_CSI_MASK               CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_CSI)
#define CCM_CGR1_RTC_MASK               CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_RTC)
#define CCM_CGR1_WDOG_MASK              CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_WDOG)
#define CCM_CGR1_PWM_MASK               CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_PWM)
#define CCM_CGR1_SIM_MASK               CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_SIM)
#define CCM_CGR1_ECT_MASK               CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_ECT)
#define CCM_CGR1_USBOTG_MASK            CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_USBOTG)
#define CCM_CGR1_KPP_MASK               CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_KPP)
#define CCM_CGR1_IPU_MASK               CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_IPU)
#define CCM_CGR1_UART3_MASK             CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_UART3)
#define CCM_CGR1_UART4_MASK             CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_UART4)
#define CCM_CGR1_UART5_MASK             CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_UART5)
#define CCM_CGR1_OWIRE_MASK             CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_OWIRE)

#define CCM_CGR2_SSI2_MASK              CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_SSI2)
#define CCM_CGR2_CSPI1_MASK             CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_CSPI1)
#define CCM_CGR2_CSPI2_MASK             CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_CSPI2)
#define CCM_CGR2_GACC_MASK              CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_GACC)
#define CCM_CGR2_EMI_MASK               CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_EMI)
#define CCM_CGR2_RTIC_MASK              CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_RTIC)
#define CCM_CGR2_FIRI_MASK              CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_FIRI)

// CGR2 "special cases"
#define CCM_CGR2_NFC_MASK               (1U << (DDK_CLOCK_GATE_INDEX_NFC - DDK_CLOCK_GATE_INDEX_SPECIAL))
//#define CCM_CGR2_NFC_MASK               CCM_CGR_MASK(DDK_CLOCK_GATE_INDEX_NFC)


#ifdef __cplusplus
}
#endif

#endif // __MX31_CCM_H__
