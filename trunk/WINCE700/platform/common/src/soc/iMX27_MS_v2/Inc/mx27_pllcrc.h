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
// Copyright (C) 2004,	MOTOROLA, INC. All Rights Reserved
//
//------------------------------------------------------------------------------
//
// Copyright (C) 2004-2006,2007 Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
// Header:  mx27_pllcrc.h
//
// Provides definitions for PLL, Clock & Reset controller module based on MX27.
//
//------------------------------------------------------------------------------
#ifndef __MX27_PLLCRC_H__
#define __MX27_PLLCRC_H__

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------
#define PLLCRC_PCCR_CG_MASK                     (0x1)

//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------
typedef struct {
    REG32 CSCR;
    REG32 MPCTL0;
    REG32 MPCTL1;
    REG32 SPCTL0;
    REG32 SPCTL1;
    REG32 OSC26MCTL;
    REG32 PCDR0;
    REG32 PCDR1;
    REG32 PCCR0;
    REG32 PCCR1;
    REG32 CCSR;
    REG32 PMCTL;
    REG32 PMCOUNT;
    REG32 WKGDCTL;
} CSP_PLLCRC_REGS, *PCSP_PLLCRC_REGS;

//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
#define PLLCRC_CSCR_OFFSET                      (0x0000)
#define PLLCRC_MPCTL0_OFFSET                    (0x0004)
#define PLLCRC_MPCTL1_OFFSET                    (0x0008)
#define PLLCRC_SPCTL0_OFFSET                    (0x000C)
#define PLLCRC_SPCTL1_OFFSET                    (0x0010)
#define PLLCRC_OSC26MCTL_OFFSET                 (0x0014)
#define PLLCRC_PCDR0_OFFSET                     (0x0018)
#define PLLCRC_PCDR1_OFFSET                     (0x001C)
#define PLLCRC_PCCR0_OFFSET                     (0x0020)
#define PLLCRC_PCCR1_OFFSET                     (0x0024)
#define PLLCRC_CCSR_OFFSET                      (0x0028)
#define PLLCRC_PMCTL_OFFSET                     (0x002C)
#define PLLCRC_PMCOUNT_OFFSET                   (0x0030)
#define PLLCRC_WKGDCTL_OFFSET                   (0x0034)

//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
// CSCR
#define PLLCRC_CSCR_MPEN_LSH                    0
#define PLLCRC_CSCR_SPEN_LSH                    1
#define PLLCRC_CSCR_FPM_EN_LSH                  2
#define PLLCRC_CSCR_OSC26M_DIS_LSH              3
#define PLLCRC_CSCR_OSC26M_DIV1P5_LSH           4
#ifdef MX27TO2
#define PLLCRC_CSCR_AHBDIV_LSH                  8
#define PLLCRC_CSCR_ARMDIV_LSH                  12
#define PLLCRC_CSCR_ARMSRC_LSH                  15
#else
#define PLLCRC_CSCR_IPDIV_LSH                   8
#define PLLCRC_CSCR_BCLKDIV_LSH                 9
#define PLLCRC_CSCR_PRESC_LSH                   13
#endif
#define PLLCRC_CSCR_MCU_SEL_LSH                 16
#define PLLCRC_CSCR_SP_SEL_LSH                  17
#define PLLCRC_CSCR_MPLL_RESTART_LSH            18
#define PLLCRC_CSCR_SPLL_RESTART_LSH            19
#define PLLCRC_CSCR_MSHC_SEL_LSH                20
#define PLLCRC_CSCR_H264_SEL_LSH                21
#define PLLCRC_CSCR_SSI1_SEL_LSH                22
#define PLLCRC_CSCR_SSI2_SEL_LSH                23
#define PLLCRC_CSCR_SD_CNT_LSH                  24
#define PLLCRC_CSCR_USB_DIV_LSH                 28
#define PLLCRC_CSCR_UPDATE_DIS_LSH              31

// MPCTL0
#define PLLCRC_MPCTL0_MFN_LSH                   0
#define PLLCRC_MPCTL0_MFI_LSH                   10
#define PLLCRC_MPCTL0_MFD_LSH                   16
#define PLLCRC_MPCTL0_PD_LSH                    26
#define PLLCRC_MPCTL0_CPLM_LSH                  31

// MPCTL1
#define PLLCRC_MPCTL1_BRMO_LSH                  6
#define PLLCRC_MPCTL1_LF_LSH                    15

// SPCTL0
#define PLLCRC_SPCTL0_MFN_LSH                   0
#define PLLCRC_SPCTL0_MFI_LSH                   10
#define PLLCRC_SPCTL0_MFD_LSH                   16
#define PLLCRC_SPCTL0_PD_LSH                    26
#define PLLCRC_SPCTL0_CPLM_LSH                  31

// SPCTL1
#define PLLCRC_SPCTL1_BRMO_LSH                  6
#define PLLCRC_SPCTL1_LF_LSH                    15

// OSC26MCTL
#define PLLCRC_OSC26MCTL_ANATEST_LSH            0
#define PLLCRC_OSC26MCTL_AGC_LSH                8
#define PLLCRC_OSC26MCTL_OSC26M_PEAK_LSH        16

// PCDR0
#define PLLCRC_PCDR0_MSHCDIV_LSH                0
#ifdef MX27TO2  //alex TO2
#define PLLCRC_PCDR0_NFCDIV_LSH                 6
#define PLLCRC_PCDR0_H264DIV_LSH                10
#else
#define PLLCRC_PCDR0_H264DIV_LSH                8
#define PLLCRC_PCDR0_NFCDIV_LSH                 12
#endif
#define PLLCRC_PCDR0_SSI1DIV_LSH                16
#define PLLCRC_PCDR0_CLKO_DIV_LSH               22
#define PLLCRC_PCDR0_CLKO_EN_LSH                25
#define PLLCRC_PCDR0_SSI2DIV_LSH                26

// PCDR1
#define PLLCRC_PCDR1_PERDIV1_LSH                0
#define PLLCRC_PCDR1_PERDIV2_LSH                8
#define PLLCRC_PCDR1_PERDIV3_LSH                16
#define PLLCRC_PCDR1_PERDIV4_LSH                24

// PCCR0
#define PLLCRC_PCCR0_SSI2_EN_LSH                0
#define PLLCRC_PCCR0_SSI1_EN_LSH                1
#define PLLCRC_PCCR0_SLCDC_EN_LSH               2
#define PLLCRC_PCCR0_SDHC3_EN_LSH               3
#define PLLCRC_PCCR0_SDHC2_EN_LSH               4
#define PLLCRC_PCCR0_SDHC1_EN_LSH               5
#define PLLCRC_PCCR0_SCC_EN_LSH                 6
#define PLLCRC_PCCR0_SAHARA_EN_LSH              7
#define PLLCRC_PCCR0_RTIC_EN_LSH                8
#define PLLCRC_PCCR0_RTC_EN_LSH                 9
#define PLLCRC_PCCR0_PWM_EN_LSH                 11
#define PLLCRC_PCCR0_OWIRE_EN_LSH               12
#define PLLCRC_PCCR0_MSHC_EN_LSH                13
#define PLLCRC_PCCR0_LCDC_EN_LSH                14
#define PLLCRC_PCCR0_KPP_EN_LSH                 15
#define PLLCRC_PCCR0_IIM_EN_LSH                 16
#define PLLCRC_PCCR0_I2C2_EN_LSH                17
#define PLLCRC_PCCR0_I2C1_EN_LSH                18
#define PLLCRC_PCCR0_GPT6_EN_LSH                19
#define PLLCRC_PCCR0_GPT5_EN_LSH                20
#define PLLCRC_PCCR0_GPT4_EN_LSH                21
#define PLLCRC_PCCR0_GPT3_EN_LSH                22
#define PLLCRC_PCCR0_GPT2_EN_LSH                23
#define PLLCRC_PCCR0_GPT1_EN_LSH                24
#define PLLCRC_PCCR0_GPIO_EN_LSH                25
#define PLLCRC_PCCR0_FEC_EN_LSH                 26
#define PLLCRC_PCCR0_EMMA_EN_LSH                27
#define PLLCRC_PCCR0_DMA_EN_LSH                 28
#define PLLCRC_PCCR0_CSPI3_EN_LSH               29
#define PLLCRC_PCCR0_CSPI2_EN_LSH               30
#define PLLCRC_PCCR0_CSPI1_EN_LSH               31

// PCCR1
#define PLLCRC_PCCR1_MSHC_BAUDEN_LSH            2
#define PLLCRC_PCCR1_NFC_BAUDEN_LSH             3
#define PLLCRC_PCCR1_SSI2_BAUDEN_LSH            4
#define PLLCRC_PCCR1_SSI1_BAUDEN_LSH            5
#define PLLCRC_PCCR1_H264_BAUDEN_LSH            6
#define PLLCRC_PCCR1_PERCLK4_EN_LSH             7
#define PLLCRC_PCCR1_PERCLK3_EN_LSH             8
#define PLLCRC_PCCR1_PERCLK2_EN_LSH             9
#define PLLCRC_PCCR1_PERCLK1_EN_LSH             10
#define PLLCRC_PCCR1_HCLK_USB_LSH               11
#define PLLCRC_PCCR1_HCLK_SLCDC_LSH             12
#define PLLCRC_PCCR1_HCLK_SAHARA_LSH            13
#define PLLCRC_PCCR1_HCLK_RTIC_LSH              14
#define PLLCRC_PCCR1_HCLK_LCDC_LSH              15
#define PLLCRC_PCCR1_HCLK_H264_LSH              16
#define PLLCRC_PCCR1_HCLK_FEC_LSH               17
#define PLLCRC_PCCR1_HCLK_EMMA_LSH              18
#define PLLCRC_PCCR1_HCLK_EMI_LSH               19
#define PLLCRC_PCCR1_HCLK_DMA_LSH               20
#define PLLCRC_PCCR1_HCLK_CSI_LSH               21
#define PLLCRC_PCCR1_HCLK_BROM_LSH              22
#define PLLCRC_PCCR1_HCLK_ATA_LSH               23
#define PLLCRC_PCCR1_WDT_EN_LSH                 24
#define PLLCRC_PCCR1_USB_EN_LSH                 25
#define PLLCRC_PCCR1_UART6_EN_LSH               26
#define PLLCRC_PCCR1_UART5_EN_LSH               27
#define PLLCRC_PCCR1_UART4_EN_LSH               28
#define PLLCRC_PCCR1_UART3_EN_LSH               29
#define PLLCRC_PCCR1_UART2_EN_LSH               30
#define PLLCRC_PCCR1_UART1_EN_LSH               31

// CCSR
#define PLLCRC_CCSR_CLKO_SEL_LSH                0
#define PLLCRC_CCSR_CLKMODE_LSH                 8
#define PLLCRC_CCSR_32K_SR_LSH                  15

// WKGDCTL
#define PLLCRC_WKGDCTL_WKGD_EN_LSH              0

//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
// CSCR
#define PLLCRC_CSCR_MPEN_WID                    1
#define PLLCRC_CSCR_SPEN_WID                    1
#define PLLCRC_CSCR_FPM_EN_WID                  1
#define PLLCRC_CSCR_OSC26M_DIS_WID              1
#define PLLCRC_CSCR_OSC26M_DIV1P5_WID           1
#ifdef MX27TO2  //Alex TO2
#define PLLCRC_CSCR_AHBDIV_WID                   2
#define PLLCRC_CSCR_ARMDIV_WID                 2
#define PLLCRC_CSCR_ARMSRC_WID                   1
#else
#define PLLCRC_CSCR_IPDIV_WID                   1
#define PLLCRC_CSCR_BCLKDIV_WID                 4
#define PLLCRC_CSCR_PRESC_WID                   3
#endif
#define PLLCRC_CSCR_MCU_SEL_WID                 1
#define PLLCRC_CSCR_SP_SEL_WID                  1
#define PLLCRC_CSCR_MPLL_RESTART_WID            1
#define PLLCRC_CSCR_SPLL_RESTART_WID            1
#define PLLCRC_CSCR_MSHC_SEL_WID                1
#define PLLCRC_CSCR_H264_SEL_WID                1
#define PLLCRC_CSCR_SSI1_SEL_WID                1
#define PLLCRC_CSCR_SSI2_SEL_WID                1
#define PLLCRC_CSCR_SD_CNT_WID                  2
#define PLLCRC_CSCR_USB_DIV_WID                 3

// MPCTL0
#define PLLCRC_MPCTL0_MFN_WID                   10
#define PLLCRC_MPCTL0_MFI_WID                   4
#define PLLCRC_MPCTL0_MFD_WID                   10
#define PLLCRC_MPCTL0_PD_WID                    4
#define PLLCRC_MPCTL0_CPLM_WID                  1

// MPCTL1
#define PLLCRC_MPCTL1_BRMO_WID                  1
#define PLLCRC_MPCTL1_LF_WID                    1

// SPCTL0
#define PLLCRC_SPCTL0_MFN_WID                   10
#define PLLCRC_SPCTL0_MFI_WID                   4
#define PLLCRC_SPCTL0_MFD_WID                   10
#define PLLCRC_SPCTL0_PD_WID                    4
#define PLLCRC_SPCTL0_CPLM_WID                  1

// SPCTL1
#define PLLCRC_SPCTL1_BRMO_WID                  1
#define PLLCRC_SPCTL1_LF_WID                    1

// OSC26MCTL
#define PLLCRC_OSC26MCTL_ANATEST_WID            6
#define PLLCRC_OSC26MCTL_AGC_WID                6
#define PLLCRC_OSC26MCTL_OSC26M_PEAK_WID        2

// PCDR0
#define PLLCRC_PCDR0_MSHCDIV_WID                5
#ifdef MX27TO2  //alex TO2
#define PLLCRC_PCDR0_NFCDIV_WID                 4
#define PLLCRC_PCDR0_H264DIV_WID                6
#else
#define PLLCRC_PCDR0_H264DIV_WID                4
#define PLLCRC_PCDR0_NFCDIV_WID                 4
#endif
#define PLLCRC_PCDR0_SSI1DIV_WID                6
#define PLLCRC_PCDR0_CLKO_DIV_WID               3
#define PLLCRC_PCDR0_CLKO_EN_WID                1
#define PLLCRC_PCDR0_SSI2DIV_WID                6

// PCDR1
#define PLLCRC_PCDR1_PERDIV1_WID                6
#define PLLCRC_PCDR1_PERDIV2_WID                6
#define PLLCRC_PCDR1_PERDIV3_WID                6
#define PLLCRC_PCDR1_PERDIV4_WID                6

// PCCR0
#define PLLCRC_PCCR0_SSI2_EN_WID                1
#define PLLCRC_PCCR0_SSI1_EN_WID                1
#define PLLCRC_PCCR0_SLCDC_EN_WID               1
#define PLLCRC_PCCR0_SDHC3_EN_WID               1
#define PLLCRC_PCCR0_SDHC2_EN_WID               1
#define PLLCRC_PCCR0_SDHC1_EN_WID               1
#define PLLCRC_PCCR0_SCC_EN_WID                 1
#define PLLCRC_PCCR0_SAHARA_EN_WID              1
#define PLLCRC_PCCR0_RTIC_EN_WID                1
#define PLLCRC_PCCR0_RTC_EN_WID                 1
#define PLLCRC_PCCR0_PWM_EN_WID                 1
#define PLLCRC_PCCR0_OWIRE_EN_WID               1
#define PLLCRC_PCCR0_MSHC_EN_WID                1
#define PLLCRC_PCCR0_LCDC_EN_WID                1
#define PLLCRC_PCCR0_KPP_EN_WID                 1
#define PLLCRC_PCCR0_IIM_EN_WID                 1
#define PLLCRC_PCCR0_I2C2_EN_WID                1
#define PLLCRC_PCCR0_I2C1_EN_WID                1
#define PLLCRC_PCCR0_GPT6_EN_WID                1
#define PLLCRC_PCCR0_GPT5_EN_WID                1
#define PLLCRC_PCCR0_GPT4_EN_WID                1
#define PLLCRC_PCCR0_GPT3_EN_WID                1
#define PLLCRC_PCCR0_GPT2_EN_WID                1
#define PLLCRC_PCCR0_GPT1_EN_WID                1
#define PLLCRC_PCCR0_GPIO_EN_WID                1
#define PLLCRC_PCCR0_FEC_EN_WID                 1
#define PLLCRC_PCCR0_EMMA_EN_WID                1
#define PLLCRC_PCCR0_DMA_EN_WID                 1
#define PLLCRC_PCCR0_CSPI3_EN_WID               1
#define PLLCRC_PCCR0_CSPI2_EN_WID               1
#define PLLCRC_PCCR0_CSPI1_EN_WID               1

// PCCR1
#define PLLCRC_PCCR1_MSHC_BAUDEN_WID            1
#define PLLCRC_PCCR1_NFC_BAUDEN_WID             1
#define PLLCRC_PCCR1_SSI2_BAUDEN_WID            1
#define PLLCRC_PCCR1_SSI1_BAUDEN_WID            1
#define PLLCRC_PCCR1_H264_BAUDEN_WID            1
#define PLLCRC_PCCR1_PERCLK4_EN_WID             1
#define PLLCRC_PCCR1_PERCLK3_EN_WID             1
#define PLLCRC_PCCR1_PERCLK2_EN_WID             1
#define PLLCRC_PCCR1_PERCLK1_EN_WID             1
#define PLLCRC_PCCR1_HCLK_USB_WID               1
#define PLLCRC_PCCR1_HCLK_SLCDC_WID             1
#define PLLCRC_PCCR1_HCLK_SAHARA_WID            1
#define PLLCRC_PCCR1_HCLK_RTIC_WID              1
#define PLLCRC_PCCR1_HCLK_LCDC_WID              1
#define PLLCRC_PCCR1_HCLK_H264_WID              1
#define PLLCRC_PCCR1_HCLK_FEC_WID               1
#define PLLCRC_PCCR1_HCLK_EMMA_WID              1
#define PLLCRC_PCCR1_HCLK_EMI_WID               1
#define PLLCRC_PCCR1_HCLK_DMA_WID               1
#define PLLCRC_PCCR1_HCLK_CSI_WID               1
#define PLLCRC_PCCR1_HCLK_BROM_WID              1
#define PLLCRC_PCCR1_HCLK_ATA_WID               1
#define PLLCRC_PCCR1_WDT_EN_WID                 1
#define PLLCRC_PCCR1_USB_EN_WID                 1
#define PLLCRC_PCCR1_UART6_EN_WID               1
#define PLLCRC_PCCR1_UART5_EN_WID               1
#define PLLCRC_PCCR1_UART4_EN_WID               1
#define PLLCRC_PCCR1_UART3_EN_WID               1
#define PLLCRC_PCCR1_UART2_EN_WID               1
#define PLLCRC_PCCR1_UART1_EN_WID               1

// CCSR
#define PLLCRC_CCSR_CLKO_SEL_WID                5
#define PLLCRC_CCSR_CLKMODE_WID                 2
#define PLLCRC_CCSR_32K_SR_WID                  1

// WKGDCTL
#define PLLCRC_WKGDCTL_WKGD_EN_WID              1

//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------
// CSCR
#define PLLCRC_CSCR_MPEN_DISABLE                0
#define PLLCRC_CSCR_MPEN_ENABLE                 1

#define PLLCRC_CSCR_SPEN_DISABLE                0
#define PLLCRC_CSCR_SPEN_ENABLE                 1

#define PLLCRC_CSCR_FPM_EN_DISABLE              0
#define PLLCRC_CSCR_FPM_EN_ENABLE               1

#define PLLCRC_CSCR_OSC26M_DIS_ENABLE           0
#define PLLCRC_CSCR_OSC26M_DIS_DISABLE          1

#define PLLCRC_CSCR_OSC26M_DIV1P5_DIV1          0
#define PLLCRC_CSCR_OSC26M_DIV1P5_DIVP5         1

#define PLLCRC_CSCR_IPDIV_TESTONLY              0
#define PLLCRC_CSCR_IPDIV_DIV2                  1

#define PLLCRC_CSCR_MCU_SEL_SRC_FPM             0
#define PLLCRC_CSCR_MCU_SEL_SRC_OSC26           1

#define PLLCRC_CSCR_SP_SEL_SRC_FPM              0
#define PLLCRC_CSCR_SP_SEL_SRC_OSC26            1

#define PLLCRC_CSCR_MPLL_RESTART_NOEFFECT       0
#define PLLCRC_CSCR_MPLL_RESTART_EN             1

#define PLLCRC_CSCR_SPLL_RESTART_NOEFFECT       0
#define PLLCRC_CSCR_SPLL_RESTART_EN             1

#define PLLCRC_CSCR_MSHC_SEL_SRC_SPLL           0
#define PLLCRC_CSCR_MSHC_SEL_SRC_MPLL           1

#define PLLCRC_CSCR_H264_SEL_SRC_SPLL           0
#define PLLCRC_CSCR_H264_SEL_SRC_MPLL           1

#define PLLCRC_CSCR_SSI1_SEL_SRC_SPLL           0
#define PLLCRC_CSCR_SSI1_SEL_SRC_MPLL           1

#define PLLCRC_CSCR_SSI2_SEL_SRC_SPLL           0
#define PLLCRC_CSCR_SSI2_SEL_SRC_MPLL           1

#define PLLCRC_CSCR_SD_CNT_AFT_NEXT_EDG         0
#define PLLCRC_CSCR_SD_CNT_AFT_2ND_EDG          1
#define PLLCRC_CSCR_SD_CNT_AFT_3RD_EDG          2
#define PLLCRC_CSCR_SD_CNT_AFT_4TH_EDG          3

// MPCTL0
#define PLLCRC_MPCTL0_CPLM_MODE_FOL             0
#define PLLCRC_MPCTL0_CPLM_MODE_FPL             1

// MPCTL1
#define PLLCRC_MPCTL1_BRMO_1ST_ORDER            0
#define PLLCRC_MPCTL1_BRMO_2ND_ORDER            1

#define PLLCRC_MPCTL1_LF_MPLL_NOT_LOCKED        0
#define PLLCRC_MPCTL1_LF_MPLL_LOCKED            1

// SPCTL0
#define PLLCRC_SPCTL0_CPLM_MODE_FOL             0
#define PLLCRC_SPCTL0_CPLM_MODE_FPL             1

// SPCTL1
#define PLLCRC_SPCTL1_BRMO_1ST_ORDER            0
#define PLLCRC_SPCTL1_BRMO_2ND_ORDER            1

#define PLLCRC_SPCTL1_LF_SPLL_NOT_LOCKED        0
#define PLLCRC_SPCTL1_LF_SPLL_LOCKED            1

// WKGDCTL
#define PLLCRC_WKGDCTL_WKGD_EN_DISABLE          0
#define PLLCRC_WKGDCTL_WKGD_EN_ENABLE           1

//------------------------------------------------------------------------------
// HELPER MACROS
//------------------------------------------------------------------------------
#define CRM_PCCR_INDEX(gateIndex)        ((gateIndex) >> 5)
#define CRM_PCCR_VAL(gateIndex, val)     (val << ((gateIndex) % 32))

#ifdef __cplusplus
}
#endif

#endif // __MX27_PLLCRC_H__
