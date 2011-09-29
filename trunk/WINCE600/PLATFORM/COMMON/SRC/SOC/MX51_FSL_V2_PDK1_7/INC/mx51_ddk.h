//------------------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
//  Use of this source code is subject to the terms of the Microsoft end-user
//  license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
//  If you did not accept the terms of the EULA, you are not authorized to use
//  this source code. For a copy of the EULA, please see the LICENSE.RTF on your
//  install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  mx51_ddk.h
//
//  Contains MX51 definitions to assist with driver development.
//
//------------------------------------------------------------------------------
#ifndef __MX51_DDK_H
#define __MX51_DDK_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Types

//-----------------------------------------------------------------------------
//
//  Type: DDK_CLOCK_SIGNAL
//
//  Clock signal name for querying/setting clock configuration.
//
//-----------------------------------------------------------------------------
typedef enum
{
    DDK_CLOCK_SIGNAL_CKIH               = 0,
    DDK_CLOCK_SIGNAL_PLL1               = 1,
    DDK_CLOCK_SIGNAL_PLL2               = 2,        
    DDK_CLOCK_SIGNAL_PLL3               = 3,
    DDK_CLOCK_SIGNAL_LP_APM             = 4,
    DDK_CLOCK_SIGNAL_ARM                = 5,
    DDK_CLOCK_SIGNAL_AXI_A              = 6,
    DDK_CLOCK_SIGNAL_AXI_B              = 7,
    DDK_CLOCK_SIGNAL_EMI_SLOW           = 8,
    DDK_CLOCK_SIGNAL_AHB                = 9,
    DDK_CLOCK_SIGNAL_IPG                = 10,
    DDK_CLOCK_SIGNAL_PERCLK             = 11,
    DDK_CLOCK_SIGNAL_CKIL_SYNC          = 12,
    DDK_CLOCK_SIGNAL_DDR                = 13,
    DDK_CLOCK_SIGNAL_ARM_AXI            = 14,
    DDK_CLOCK_SIGNAL_IPU_HSP            = 15,
    DDK_CLOCK_SIGNAL_GPU                = 16,
    DDK_CLOCK_SIGNAL_VPU_AXI            = 17,
    DDK_CLOCK_SIGNAL_ENFC               = 18,
    DDK_CLOCK_SIGNAL_DEBUG_APB          = 19,
     
    DDK_CLOCK_SIGNAL_USBOH3             = 21,
    DDK_CLOCK_SIGNAL_ESDHC1             = 22,
    DDK_CLOCK_SIGNAL_ESDHC3             = 23,
    DDK_CLOCK_SIGNAL_ESDHC4             = 24,
    DDK_CLOCK_SIGNAL_ESDHC2             = 25,
    DDK_CLOCK_SIGNAL_UART               = 26,
    DDK_CLOCK_SIGNAL_SSI1               = 27,
    DDK_CLOCK_SIGNAL_SSI3               = 28,
    DDK_CLOCK_SIGNAL_SSI2               = 29,
    DDK_CLOCK_SIGNAL_SSI_EXT1           = 30,
    DDK_CLOCK_SIGNAL_SSI_EXT2           = 31,
    DDK_CLOCK_SIGNAL_USB_PHY            = 32,
    DDK_CLOCK_SIGNAL_TVE_216_54         = 33,
    DDK_CLOCK_SIGNAL_DI                 = 34,
    DDK_CLOCK_SIGNAL_DI0                = 34, // DI0 replaces DI for TO2
    DDK_CLOCK_SIGNAL_VPU_RCLK           = 35,
    DDK_CLOCK_SIGNAL_SPDIF0             = 36,
    DDK_CLOCK_SIGNAL_SPDIF1             = 37,
    DDK_CLOCK_SIGNAL_SLIMBUS            = 38,
    DDK_CLOCK_SIGNAL_SIM                = 39,
    DDK_CLOCK_SIGNAL_FIRI               = 40,
    DDK_CLOCK_SIGNAL_HSI2C              = 41,
    DDK_CLOCK_SIGNAL_SSI_LP_APM         = 42,
    DDK_CLOCK_SIGNAL_SPDIF_XTAL         = 43,
    DDK_CLOCK_SIGNAL_HSC1               = 44,
    DDK_CLOCK_SIGNAL_HSC2               = 45,
    DDK_CLOCK_SIGNAL_ESC                = 46,
    DDK_CLOCK_SIGNAL_CSI_MCLK1          = 47,
    DDK_CLOCK_SIGNAL_CSI_MCLK2          = 48,
    DDK_CLOCK_SIGNAL_ECSPI              = 49,
    DDK_CLOCK_SIGNAL_WRCK               = 50,
    DDK_CLOCK_SIGNAL_LPSR               = 51,
    DDK_CLOCK_SIGNAL_PGC                = 52,
    DDK_CLOCK_SIGNAL_OSC                = 53,
    DDK_CLOCK_SIGNAL_CKIH_CAMP1         = 54,
    DDK_CLOCK_SIGNAL_CKIH2_CAMP2        = 55,
    DDK_CLOCK_SIGNAL_CKIH2              = 56,
    DDK_CLOCK_SIGNAL_TVE_DI             = 57,
    DDK_CLOCK_SIGNAL_FPM                = 58,
    DDK_CLOCK_SIGNAL_IPP_DI             = 59,
    DDK_CLOCK_SIGNAL_DI1                = 60, // DI1 new for TO2
    DDK_CLOCK_SIGNAL_GPU2D              = 61,
    DDK_CLOCK_SIGNAL_ENUM_END           = 62
} DDK_CLOCK_SIGNAL;

//-----------------------------------------------------------------------------
//
//  Type: DDK_CLOCK_GATE_INDEX
//
//  Index for referencing the corresponding clock gating control bits within
//  the CCM.
//
//-----------------------------------------------------------------------------
typedef enum 
{
    DDK_CLOCK_GATE_INDEX_ARM_BUS        = 0,    // CGR0[1:0]
    DDK_CLOCK_GATE_INDEX_ARM_AXI        = 1,    // CGR0[3:2]
    DDK_CLOCK_GATE_INDEX_ARM_DEBUG      = 2,    // CGR0[5:4]
    DDK_CLOCK_GATE_INDEX_TZIC           = 3,    // CGR0[7:6]
    DDK_CLOCK_GATE_INDEX_DAP            = 4,    // CGR0[9:8]
    DDK_CLOCK_GATE_INDEX_TPIU           = 5,    // CGR0[11:10]
    DDK_CLOCK_GATE_INDEX_CTI2           = 6,    // CGR0[13:12]
    DDK_CLOCK_GATE_INDEX_CTI3           = 7,    // CGR0[15:14]
    DDK_CLOCK_GATE_INDEX_AHBMUX1        = 8,    // CGR0[17:16]
    DDK_CLOCK_GATE_INDEX_AHBMUX2        = 9,    // CGR0[19:18]
    DDK_CLOCK_GATE_INDEX_ROMCP          = 10,   // CGR0[21:20]
    DDK_CLOCK_GATE_INDEX_ROM            = 11,   // CGR0[23:22]    
    DDK_CLOCK_GATE_INDEX_AIPS_TZ1       = 12,   // CGR0[25:24]  
    DDK_CLOCK_GATE_INDEX_AIPS_TZ2       = 13,   // CGR0[27:26]
    DDK_CLOCK_GATE_INDEX_AHB_MAX        = 14,   // CGR0[29:28]
    DDK_CLOCK_GATE_INDEX_IIM            = 15,   // CGR0[31:30]

    DDK_CLOCK_GATE_INDEX_TMAX1          = 16,   // CGR1[1:0]  
    DDK_CLOCK_GATE_INDEX_TMAX2          = 17,   // CGR1[3:2]  
    DDK_CLOCK_GATE_INDEX_TMAX3          = 18,   // CGR1[5:4]  
    DDK_CLOCK_GATE_INDEX_UART1_IPG      = 19,   // CGR1[7:6]  
    DDK_CLOCK_GATE_INDEX_UART1_PERCLK   = 20,   // CGR1[9:8]  
    DDK_CLOCK_GATE_INDEX_UART2_IPG      = 21,   // CGR1[11:10]
    DDK_CLOCK_GATE_INDEX_UART2_PERCLK   = 22,   // CGR1[13:12]
    DDK_CLOCK_GATE_INDEX_UART3_IPG      = 23,   // CGR1[15:14]
    DDK_CLOCK_GATE_INDEX_UART3_PERCLK   = 24,   // CGR1[17:16]
    DDK_CLOCK_GATE_INDEX_I2C1           = 25,   // CGR1[19:18]
    DDK_CLOCK_GATE_INDEX_I2C2           = 26,   // CGR1[21:20]
    DDK_CLOCK_GATE_INDEX_HSI2C_IPG      = 27,   // CGR1[23:22]
    DDK_CLOCK_GATE_INDEX_HSI2C_SERIAL   = 28,   // CGR1[25:24]    
    DDK_CLOCK_GATE_INDEX_FIRI_IPG       = 29,   // CGR1[27:26]
    DDK_CLOCK_GATE_INDEX_FIRI_SERIAL    = 30,   // CGR1[29:28]     
                                                
    DDK_CLOCK_GATE_INDEX_USB_PHY        = 32,   // CGR2[1:0]  
    DDK_CLOCK_GATE_INDEX_EPIT1_IPG      = 33,   // CGR2[3:2]  
    DDK_CLOCK_GATE_INDEX_EPIT1_HIGHFREQ = 34,   // CGR2[5:4]  
    DDK_CLOCK_GATE_INDEX_EPIT2_IPG      = 35,   // CGR2[7:6]  
    DDK_CLOCK_GATE_INDEX_EPIT2_HIGHFREQ = 36,   // CGR2[9:8]  
    DDK_CLOCK_GATE_INDEX_PWM1_IPG       = 37,   // CGR2[11:10]
    DDK_CLOCK_GATE_INDEX_PWM1_HIGHFREQ  = 38,   // CGR2[13:12]
    DDK_CLOCK_GATE_INDEX_PWM2_IPG       = 39,   // CGR2[15:14]
    DDK_CLOCK_GATE_INDEX_PWM2_HIGHFREQ  = 40,   // CGR2[17:16]
    DDK_CLOCK_GATE_INDEX_GPT_IPG        = 41,   // CGR2[19:18]
    DDK_CLOCK_GATE_INDEX_GPT_HIGHFREQ   = 42,   // CGR2[21:20]
    DDK_CLOCK_GATE_INDEX_OWIRE          = 43,   // CGR2[23:22]
    DDK_CLOCK_GATE_INDEX_FEC            = 44,   // CGR2[25:24]
    DDK_CLOCK_GATE_INDEX_USBOH3_IPG     = 45,   // CGR2[27:26]
    DDK_CLOCK_GATE_INDEX_USBOH3_60M     = 46,   // CGR2[29:28]
    DDK_CLOCK_GATE_INDEX_TVE            = 47,   // CGR2[31:30]

    DDK_CLOCK_GATE_INDEX_ESDHC1_IPG     = 48,   // CGR3[1:0]  
    DDK_CLOCK_GATE_INDEX_ESDHC1_PERCLK  = 49,   // CGR3[3:2]  
    DDK_CLOCK_GATE_INDEX_ESDHC2_IPG     = 50,   // CGR3[5:4]  
    DDK_CLOCK_GATE_INDEX_ESDHC2_PERCLK  = 51,   // CGR3[7:6]  
    DDK_CLOCK_GATE_INDEX_ESDHC3_IPG     = 52,   // CGR3[9:8]  
    DDK_CLOCK_GATE_INDEX_ESDHC3_PERCLK  = 53,   // CGR3[11:10]
    DDK_CLOCK_GATE_INDEX_ESDHC4_IPG     = 54,   // CGR3[13:12]
    DDK_CLOCK_GATE_INDEX_ESDHC4_PERCLK  = 55,   // CGR3[15:14]
    DDK_CLOCK_GATE_INDEX_SSI1_IPG       = 56,   // CGR3[17:16]
    DDK_CLOCK_GATE_INDEX_SSI1_SSI       = 57,   // CGR3[19:18]
    DDK_CLOCK_GATE_INDEX_SSI2_IPG       = 58,   // CGR3[21:20]
    DDK_CLOCK_GATE_INDEX_SSI2_SSI       = 59,   // CGR3[23:22]
    DDK_CLOCK_GATE_INDEX_SSI3_IPG       = 60,   // CGR3[25:24]
    DDK_CLOCK_GATE_INDEX_SSI3_SSI       = 61,   // CGR3[27:26]
    DDK_CLOCK_GATE_INDEX_SSI_EXT1       = 62,   // CGR3[29:28]
    DDK_CLOCK_GATE_INDEX_SSI_EXT2       = 63,   // CGR3[31:30]

    DDK_CLOCK_GATE_INDEX_PATA           = 64,   // CGR4[1:0]  
    DDK_CLOCK_GATE_INDEX_SIM_IPG        = 65,   // CGR4[3:2]  
    DDK_CLOCK_GATE_INDEX_SIM_SERIAL     = 66,   // CGR4[5:4]     
    DDK_CLOCK_GATE_INDEX_HSC_HS1        = 67,   // CGR4[7:6]  
    DDK_CLOCK_GATE_INDEX_HSC_HS2        = 68,   // CGR4[9:8]  
    DDK_CLOCK_GATE_INDEX_HSC_ESC        = 69,   // CGR4[11:10]
    DDK_CLOCK_GATE_INDEX_HSC_HSP        = 70,   // CGR4[13:12]
    DDK_CLOCK_GATE_INDEX_SAHARA         = 71,   // CGR4[15:14]
    DDK_CLOCK_GATE_INDEX_RTIC           = 72,   // CGR4[17:16]
    DDK_CLOCK_GATE_INDEX_ECSPI1_IPG     = 73,   // CGR4[19:18]
    DDK_CLOCK_GATE_INDEX_ECSPI1_PERCLK  = 74,   // CGR4[21:20]
    DDK_CLOCK_GATE_INDEX_ECSPI2_IPG     = 75,   // CGR4[23:22]
    DDK_CLOCK_GATE_INDEX_ECSPI2_PERCLK  = 76,   // CGR4[25:24]
    DDK_CLOCK_GATE_INDEX_CSPI_IPG       = 77,   // CGR4[27:26]
    DDK_CLOCK_GATE_INDEX_SRTC           = 78,   // CGR4[29:28]
    DDK_CLOCK_GATE_INDEX_SDMA           = 79,   // CGR4[31:30]

    DDK_CLOCK_GATE_INDEX_SPBA           = 80,   // CGR5[1:0]  
    DDK_CLOCK_GATE_INDEX_GPU            = 81,   // CGR5[3:2]  
    DDK_CLOCK_GATE_INDEX_GARB           = 82,   // CGR5[5:4]  
    DDK_CLOCK_GATE_INDEX_VPU            = 83,   // CGR5[7:6]
    DDK_CLOCK_GATE_INDEX_VPU_REFERENCE  = 84,   // CGR5[9:8]
    DDK_CLOCK_GATE_INDEX_IPU            = 85,   // CGR5[11:10]
    DDK_CLOCK_GATE_INDEX_IPU_DI         = 86,   // CGR5[13:12]
    DDK_CLOCK_GATE_INDEX_EMI_FAST       = 87,   // CGR5[15:14]
    DDK_CLOCK_GATE_INDEX_EMI_SLOW       = 88,   // CGR5[17:16]
    DDK_CLOCK_GATE_INDEX_EMI_INTR       = 89,   // CGR5[19:18]
    DDK_CLOCK_GATE_INDEX_EMI_ENFC       = 90,   // CGR5[21:20]
    DDK_CLOCK_GATE_INDEX_EMI_WRCK       = 91,   // CGR5[23:22]
    DDK_CLOCK_GATE_INDEX_GPC_IPG        = 92,   // CGR5[25:24]
    DDK_CLOCK_GATE_INDEX_SPDIF0         = 93,   // CGR5[27:26]
    DDK_CLOCK_GATE_INDEX_SPDIF1         = 94,   // CGR5[29:28]
    DDK_CLOCK_GATE_INDEX_SPDIF_IPG      = 95,   // CGR5[31:30]

    DDK_CLOCK_GATE_INDEX_SLIMBUS        = 96,   // CGR6[1:0]
    DDK_CLOCK_GATE_INDEX_SLIMBUS_SERIAL = 97,   // CGR6[3:2]
    DDK_CLOCK_GATE_INDEX_CSI_MCLK1      = 98,   // CGR6[5:4]
    DDK_CLOCK_GATE_INDEX_CSI_MCLK2      = 99,   // CGR6[7:6]    
    DDK_CLOCK_GATE_INDEX_EMI_GARB       = 100,  // CGR6[9:8]
    DDK_CLOCK_GATE_INDEX_IPU_DI0        = 101,  // CGR6[11:10]
    DDK_CLOCK_GATE_INDEX_IPU_DI1        = 102,  // CGR6[13:12]
    DDK_CLOCK_GATE_INDEX_GPU2D          = 103,  // CGR6[15:14]
    DDK_CLOCK_GATE_INDEX_ENUM_END       = 104
} DDK_CLOCK_GATE_INDEX;

//-----------------------------------------------------------------------------
//
//  Type: DDK_CLOCK_OVERRIDE_ENABLE_INDEX
//
//  Index for referencing the corresponding clock enable overrite bits within
//  the CMEOR register.
//
//-----------------------------------------------------------------------------
typedef enum 
{
    DDK_CLOCK_OVERRIDE_ENABLE_SAHARA   = (0),
    DDK_CLOCK_OVERRIDE_ENABLE_OWIRE    = (2),
    DDK_CLOCK_OVERRIDE_ENABLE_IIM      = (3),
    DDK_CLOCK_OVERRIDE_ENABLE_ESDHC    = (4),   
    DDK_CLOCK_OVERRIDE_ENABLE_GPT      = (5),
    DDK_CLOCK_OVERRIDE_ENABLE_EPIT     = (6),
    DDK_CLOCK_OVERRIDE_ENABLE_GPU      = (7),
    DDK_CLOCK_OVERRIDE_ENABLE_DAP      = (8),  
    DDK_CLOCK_OVERRIDE_ENABLE_VPU      = (9),
    DDK_CLOCK_OVERRIDE_ENABLE_EMI_GARB = (12),
    DDK_CLOCK_OVERRIDE_ENABLE_EMI_FAST = (13),
    DDK_CLOCK_OVERRIDE_ENABLE_EMI_SLOW = (14), 
    DDK_CLOCK_OVERRIDE_ENABLE_EMI_INTR = (15),
    DDK_CLOCK_OVERRIDE_ENABLE_EMI_M0   = (16),
    DDK_CLOCK_OVERRIDE_ENABLE_EMI_M1   = (17),
    DDK_CLOCK_OVERRIDE_ENABLE_EMI_M2   = (18),
    DDK_CLOCK_OVERRIDE_ENABLE_EMI_M3   = (19),
    DDK_CLOCK_OVERRIDE_ENABLE_EMI_M4   = (20),
    DDK_CLOCK_OVERRIDE_ENABLE_EMI_M5   = (21),
    DDK_CLOCK_OVERRIDE_ENABLE_HSC_ESC  = (22),
    DDK_CLOCK_OVERRIDE_ENABLE_HSC_PRIM = (23),
    DDK_CLOCK_OVERRIDE_ENABLE_HSC_SEC  = (24),
    DDK_CLOCK_OVERRIDE_ENABLE_ENUM_END = (25)
} DDK_CLOCK_OVERRIDE_ENABLE_INDEX;

//-----------------------------------------------------------------------------
//
//  Type: DDK_CLOCK_GATE_MODE
//
//  Clock gating modes supported by CCM clock gating registers.
//
//-----------------------------------------------------------------------------
typedef enum {
    DDK_CLOCK_GATE_MODE_DISABLED            = 0,
    DDK_CLOCK_GATE_MODE_ENABLED_RUN         = 1,
    DDK_CLOCK_GATE_MODE_ENABLED_ALL         = 3,
    DDK_CLOCK_GATE_MODE_POWER_OFF           = 4
} DDK_CLOCK_GATE_MODE;

//-----------------------------------------------------------------------------
//
//  Type: DDK_CLOCK_OVERRIDE_ENABLE_MODE
//
//  Clock enabling override modes supported by CCM Module Enable Override Rigester(CMERO).
//
//-----------------------------------------------------------------------------
typedef enum {
    DDK_CLOCK_OVERRIDE_MODE_DISABLED = 0,
    DDK_CLOCK_OVERRIDE_MODE_ENABLED  = 1
} DDK_CLOCK_OVERRIDE_MODE;


//-----------------------------------------------------------------------------
//
//  Type: DDK_CLOCK_CKO1_SRC
//
//  Clock output source (CKO1) signal selections.
//
//-----------------------------------------------------------------------------
typedef enum {
    DDK_CLOCK_CKO1_SRC_ARM                   = 0,
    DDK_CLOCK_CKO1_SRC_PLL1                  = 1,
    DDK_CLOCK_CKO1_SRC_PLL2                  = 2,
    DDK_CLOCK_CKO1_SRC_PLL3                  = 3,
    DDK_CLOCK_CKO1_SRC_EMI_CORE              = 4,
    DDK_CLOCK_CKO1_SRC_NFC                   = 6,
    DDK_CLOCK_CKO1_SRC_DI                    = 8,
    DDK_CLOCK_CKO1_SRC_AHB                   = 11,
    DDK_CLOCK_CKO1_SRC_IPG                   = 12,
    DDK_CLOCK_CKO1_SRC_PERCLK                = 13,
    DDK_CLOCK_CKO1_SRC_CKIL_SYNC             = 14
} DDK_CLOCK_CKO1_SRC;


//-----------------------------------------------------------------------------
//
//  Type: DDK_CLOCK_CKO2_SRC
//
//  Clock output source (CKO2) signal selections.
//
//-----------------------------------------------------------------------------
typedef enum {
    DDK_CLOCK_CKO2_SRC_DPTC_CORE             = 0,
    DDK_CLOCK_CKO2_SRC_DPTC_PERIPH           = 1,
    DDK_CLOCK_CKO2_SRC_ESDHC1                = 3,
    DDK_CLOCK_CKO2_SRC_USBOH3                = 4,
    DDK_CLOCK_CKO2_SRC_WRCK                  = 5,
    DDK_CLOCK_CKO2_SRC_CSPI                  = 6,
    DDK_CLOCK_CKO2_SRC_PPL1_REF              = 7,
    DDK_CLOCK_CKO2_SRC_ESDHC3                = 8,
    DDK_CLOCK_CKO2_SRC_DDR                   = 9,
    DDK_CLOCK_CKO2_SRC_ARM_AXI               = 10,
    DDK_CLOCK_CKO2_SRC_USBPHY_PLL_OUT        = 11,
    DDK_CLOCK_CKO2_SRC_VPU_RCLK              = 12,
    DDK_CLOCK_CKO2_SRC_IPU_HSP               = 13,
    DDK_CLOCK_CKO2_SRC_OSC                   = 14,
    DDK_CLOCK_CKO2_SRC_CKIH_CAMP             = 15,
    DDK_CLOCK_CKO2_SRC_FPM                   = 16,
    DDK_CLOCK_CKO2_SRC_ESDHC2                = 17,
    DDK_CLOCK_CKO2_SRC_SSI1                  = 18,
    DDK_CLOCK_CKO2_SRC_SSI2                  = 19,
    DDK_CLOCK_CKO2_SRC_LPSR                  = 22,
    DDK_CLOCK_CKO2_SRC_PGC                   = 23,
    DDK_CLOCK_CKO2_SRC_TVE_EXT               = 24,
    DDK_CLOCK_CKO2_SRC_USB_PHY               = 25,
    DDK_CLOCK_CKO2_SRC_TVE_216_54            = 26,
    DDK_CLOCK_CKO2_SRC_LP_APM                = 27,
    DDK_CLOCK_CKO2_SRC_UART                  = 28,
    DDK_CLOCK_CKO2_SRC_SPDIF0                = 29,
    DDK_CLOCK_CKO2_SRC_SPDIF1                = 30
} DDK_CLOCK_CKO2_SRC;

//-----------------------------------------------------------------------------
//
//  Type: DDK_CLOCK_CKO_DIV
//
//  Clock output source (CKO) divider selections.
//
//-----------------------------------------------------------------------------
typedef enum {
    DDK_CLOCK_CKO_DIV_1                     = 0,
    DDK_CLOCK_CKO_DIV_2                     = 1,
    DDK_CLOCK_CKO_DIV_3                     = 2,
    DDK_CLOCK_CKO_DIV_4                     = 3,
    DDK_CLOCK_CKO_DIV_5                     = 4,
    DDK_CLOCK_CKO_DIV_6                     = 5,
    DDK_CLOCK_CKO_DIV_7                     = 6,
    DDK_CLOCK_CKO_DIV_8                     = 7
} DDK_CLOCK_CKO_DIV;


//-----------------------------------------------------------------------------
//
//  Type: DDK_CLOCK_BAUD_SOURCE
//
//  Input source for baud clock generation.
//
//-----------------------------------------------------------------------------
typedef enum {
    DDK_CLOCK_BAUD_SOURCE_PLL1              = 0,
    DDK_CLOCK_BAUD_SOURCE_PLL2              = 1,
    DDK_CLOCK_BAUD_SOURCE_PLL3              = 2,
    DDK_CLOCK_BAUD_SOURCE_LP_APM            = 3,
    DDK_CLOCK_BAUD_SOURCE_SSI_LP_APM        = 4,
    DDK_CLOCK_BAUD_SOURCE_SPDIF_XTAL        = 5,
    DDK_CLOCK_BAUD_SOURCE_ESDHC1            = 6,
    DDK_CLOCK_BAUD_SOURCE_ESDHC2            = 7,
    DDK_CLOCK_BAUD_SOURCE_SSI1              = 8,
    DDK_CLOCK_BAUD_SOURCE_SSI2              = 9,
    DDK_CLOCK_BAUD_SOURCE_OSC               = 10,
    DDK_CLOCK_BAUD_SOURCE_CKIH_CAMP1        = 11,
    DDK_CLOCK_BAUD_SOURCE_CKIH2_CAMP2       = 12,
    DDK_CLOCK_BAUD_SOURCE_CKIH              = 13,
    DDK_CLOCK_BAUD_SOURCE_CKIH2             = 14,
    DDK_CLOCK_BAUD_SOURCE_TVE_DI            = 15,
    DDK_CLOCK_BAUD_SOURCE_IPG               = 16,
    DDK_CLOCK_BAUD_SOURCE_AHB               = 17,    
    DDK_CLOCK_BAUD_SOURCE_FPM               = 18,
    DDK_CLOCK_BAUD_SOURCE_FPM_DIV2          = 19,
    DDK_CLOCK_BAUD_SOURCE_IPP_DI            = 20,
    DDK_CLOCK_BAUD_SOURCE_GND               = 21,
    DDK_CLOCK_BAUD_SOURCE_ENUM_END          = 22
} DDK_CLOCK_BAUD_SOURCE;


//-----------------------------------------------------------------------------
//
//  Type: DDK_IOMUX_PIN_MUXMODE
//
//  Specifies the MUX_MODE for a signal.
//
//-----------------------------------------------------------------------------
typedef enum 
{
    DDK_IOMUX_PIN_MUXMODE_ALT0  = (IOMUX_SW_MUX_CTL_MUX_MODE_ALT0 << IOMUX_SW_MUX_CTL_MUX_MODE_LSH),
    DDK_IOMUX_PIN_MUXMODE_ALT1  = (IOMUX_SW_MUX_CTL_MUX_MODE_ALT1 << IOMUX_SW_MUX_CTL_MUX_MODE_LSH),
    DDK_IOMUX_PIN_MUXMODE_ALT2  = (IOMUX_SW_MUX_CTL_MUX_MODE_ALT2 << IOMUX_SW_MUX_CTL_MUX_MODE_LSH),
    DDK_IOMUX_PIN_MUXMODE_ALT3  = (IOMUX_SW_MUX_CTL_MUX_MODE_ALT3 << IOMUX_SW_MUX_CTL_MUX_MODE_LSH),
    DDK_IOMUX_PIN_MUXMODE_ALT4  = (IOMUX_SW_MUX_CTL_MUX_MODE_ALT4 << IOMUX_SW_MUX_CTL_MUX_MODE_LSH),
    DDK_IOMUX_PIN_MUXMODE_ALT5  = (IOMUX_SW_MUX_CTL_MUX_MODE_ALT5 << IOMUX_SW_MUX_CTL_MUX_MODE_LSH),
    DDK_IOMUX_PIN_MUXMODE_ALT6  = (IOMUX_SW_MUX_CTL_MUX_MODE_ALT6 << IOMUX_SW_MUX_CTL_MUX_MODE_LSH),
    DDK_IOMUX_PIN_MUXMODE_ALT7  = (IOMUX_SW_MUX_CTL_MUX_MODE_ALT7 << IOMUX_SW_MUX_CTL_MUX_MODE_LSH)
} DDK_IOMUX_PIN_MUXMODE;


//-----------------------------------------------------------------------------
//
//  Type: DDK_IOMUX_PIN_SION
//
//  Specifies the Software Input On Filed for a signal.
//
//-----------------------------------------------------------------------------
typedef enum 
{
    DDK_IOMUX_PIN_SION_REGULAR  = (IOMUX_SW_MUX_CTL_SION_REGULAR << IOMUX_SW_MUX_CTL_SION_LSH),
    DDK_IOMUX_PIN_SION_FORCE    = (IOMUX_SW_MUX_CTL_SION_FORCE << IOMUX_SW_MUX_CTL_SION_LSH)
} DDK_IOMUX_PIN_SION;

#define DDK_IOMUX_PIN_SION_NULL     ((DDK_IOMUX_PIN_SION)0)


//-----------------------------------------------------------------------------
//
//  Type: DDK_IOMUX_PAD_SLEW
//
//  Specifies the slew rate for a pad.
//
// 
//-----------------------------------------------------------------------------
typedef enum 
{
    DDK_IOMUX_PAD_SLEW_SLOW = (IOMUX_SW_PAD_CTL_SRE_SLOW << IOMUX_SW_PAD_CTL_SRE_LSH),
    DDK_IOMUX_PAD_SLEW_FAST = (IOMUX_SW_PAD_CTL_SRE_FAST << IOMUX_SW_PAD_CTL_SRE_LSH),
} DDK_IOMUX_PAD_SLEW;

#define DDK_IOMUX_PAD_SLEW_NULL ((DDK_IOMUX_PAD_SLEW)0)

//-----------------------------------------------------------------------------
//
//  Type: DDK_IOMUX_PAD_DRIVE
//
//  Specifies the drive strength for a pad.
//
// 
//-----------------------------------------------------------------------------
typedef enum 
{
    DDK_IOMUX_PAD_DRIVE_NORMAL  = (IOMUX_SW_PAD_CTL_DSE_NORMAL << IOMUX_SW_PAD_CTL_DSE_LSH),
    DDK_IOMUX_PAD_DRIVE_MEDIUM  = (IOMUX_SW_PAD_CTL_DSE_MEDIUM << IOMUX_SW_PAD_CTL_DSE_LSH),
    DDK_IOMUX_PAD_DRIVE_HIGH    = (IOMUX_SW_PAD_CTL_DSE_HIGH << IOMUX_SW_PAD_CTL_DSE_LSH),
    DDK_IOMUX_PAD_DRIVE_MAX     = (IOMUX_SW_PAD_CTL_DSE_MAX << IOMUX_SW_PAD_CTL_DSE_LSH)
} DDK_IOMUX_PAD_DRIVE;

#define DDK_IOMUX_PAD_DRIVE_NULL ((DDK_IOMUX_PAD_DRIVE)0)

//-----------------------------------------------------------------------------
//
//  Type: DDK_IOMUX_PAD_OPENDRAIN
//
//  Specifies the open drain for a pad.
//
//-----------------------------------------------------------------------------
typedef enum 
{
    DDK_IOMUX_PAD_OPENDRAIN_DISABLE = (IOMUX_SW_PAD_CTL_ODE_DISABLE << IOMUX_SW_PAD_CTL_ODE_LSH),
    DDK_IOMUX_PAD_OPENDRAIN_ENABLE  = (IOMUX_SW_PAD_CTL_ODE_ENABLE << IOMUX_SW_PAD_CTL_ODE_LSH)
} DDK_IOMUX_PAD_OPENDRAIN;

#define DDK_IOMUX_PAD_OPENDRAIN_NULL  ((DDK_IOMUX_PAD_OPENDRAIN)0)

//-----------------------------------------------------------------------------
//
//  Type: DDK_IOMUX_PAD_PULL
//
//  Specifies the pull-up/pull-down/keeper configuration for a pad.
//
// 
//-----------------------------------------------------------------------------
typedef enum 
{
    DDK_IOMUX_PAD_PULL_NONE         = (IOMUX_SW_PAD_CTL_PKE_DISABLE << IOMUX_SW_PAD_CTL_PKE_LSH),
        
    DDK_IOMUX_PAD_PULL_KEEPER       = (IOMUX_SW_PAD_CTL_PUE_KEEPER << IOMUX_SW_PAD_CTL_PUE_LSH) |
                                      (IOMUX_SW_PAD_CTL_PKE_ENABLE << IOMUX_SW_PAD_CTL_PKE_LSH),
                                      
    DDK_IOMUX_PAD_PULL_DOWN_100K    = (IOMUX_SW_PAD_CTL_PUS_100K_DOWN << IOMUX_SW_PAD_CTL_PUS_LSH) |
                                      (IOMUX_SW_PAD_CTL_PKE_ENABLE << IOMUX_SW_PAD_CTL_PKE_LSH) |
                                      (IOMUX_SW_PAD_CTL_PUE_PULL << IOMUX_SW_PAD_CTL_PUE_LSH),
                                      
    DDK_IOMUX_PAD_PULL_UP_100K      = (IOMUX_SW_PAD_CTL_PUS_100K_UP << IOMUX_SW_PAD_CTL_PUS_LSH) |
                                      (IOMUX_SW_PAD_CTL_PKE_ENABLE << IOMUX_SW_PAD_CTL_PKE_LSH) |
                                      (IOMUX_SW_PAD_CTL_PUE_PULL << IOMUX_SW_PAD_CTL_PUE_LSH),
                                      
    DDK_IOMUX_PAD_PULL_UP_47K       = (IOMUX_SW_PAD_CTL_PUS_47K_UP << IOMUX_SW_PAD_CTL_PUS_LSH) |
                                      (IOMUX_SW_PAD_CTL_PKE_ENABLE << IOMUX_SW_PAD_CTL_PKE_LSH) |
                                      (IOMUX_SW_PAD_CTL_PUE_PULL << IOMUX_SW_PAD_CTL_PUE_LSH),
                                      
    DDK_IOMUX_PAD_PULL_UP_22K       = (IOMUX_SW_PAD_CTL_PUS_22K_UP << IOMUX_SW_PAD_CTL_PUS_LSH) |
                                      (IOMUX_SW_PAD_CTL_PKE_ENABLE << IOMUX_SW_PAD_CTL_PKE_LSH) |
                                      (IOMUX_SW_PAD_CTL_PUE_PULL << IOMUX_SW_PAD_CTL_PUE_LSH)
    
} DDK_IOMUX_PAD_PULL;


//-----------------------------------------------------------------------------
//
//  Type: DDK_IOMUX_PAD_HYSTERESIS
//
//  Specifies the hysteresis for a pad.
//
//-----------------------------------------------------------------------------
typedef enum 
{
    DDK_IOMUX_PAD_HYSTERESIS_DISABLE    = (IOMUX_SW_PAD_CTL_HYS_DISABLE << IOMUX_SW_PAD_CTL_HYS_LSH),
    DDK_IOMUX_PAD_HYSTERESIS_ENABLE     = (IOMUX_SW_PAD_CTL_HYS_ENABLE << IOMUX_SW_PAD_CTL_HYS_LSH)
} DDK_IOMUX_PAD_HYSTERESIS;

#define DDK_IOMUX_PAD_HYSTERESIS_NULL   ((DDK_IOMUX_PAD_HYSTERESIS)0)

//-----------------------------------------------------------------------------
//
//  Type: DDK_IOMUX_PAD_INMODE
//
//  Specifies the input mode (DDR/CMOS) for a pad.
//
//-----------------------------------------------------------------------------
typedef enum 
{
    DDK_IOMUX_PAD_INMODE_CMOS       = (IOMUX_SW_PAD_CTL_DDR_INPUT_CMOS << IOMUX_SW_PAD_CTL_DDR_INPUT_LSH),
    DDK_IOMUX_PAD_INMODE_DDR        = (IOMUX_SW_PAD_CTL_DDR_INPUT_DDR << IOMUX_SW_PAD_CTL_DDR_INPUT_LSH)
} DDK_IOMUX_PAD_INMODE;

#define DDK_IOMUX_PAD_INMODE_NULL  ((DDK_IOMUX_PAD_INMODE)0)

//-----------------------------------------------------------------------------
//
//  Type: DDK_IOMUX_PAD_OUTVOLT
//
//  Specifies the output voltage for a pad.
//
//-----------------------------------------------------------------------------
typedef enum 
{
    DDK_IOMUX_PAD_OUTVOLT_LOW       = (IOMUX_SW_PAD_CTL_HVE_LOW << IOMUX_SW_PAD_CTL_HVE_LSH),
    DDK_IOMUX_PAD_OUTVOLT_HIGH      = (IOMUX_SW_PAD_CTL_HVE_HIGH << IOMUX_SW_PAD_CTL_HVE_LSH)
} DDK_IOMUX_PAD_OUTVOLT;

#define DDK_IOMUX_PAD_OUTVOLT_NULL  ((DDK_IOMUX_PAD_OUTVOLT)0)

//-----------------------------------------------------------------------------
//
//  Type: DDK_DVFC_SETPOINT
//
//  Provides abstract names for frequency/voltage setpoints supported by the
//  DVFC driver.
//
// 
//-----------------------------------------------------------------------------
typedef enum
{
    DDK_DVFC_SETPOINT_HIGH          = 0,
    DDK_DVFC_SETPOINT_MEDIUM        = 1,
    DDK_DVFC_SETPOINT_LOW           = 2,
    DDK_DVFC_SETPOINT_ENUM_END      = 3
} DDK_DVFC_SETPOINT;

typedef enum
{
    // CPU setpoint frequencies
    DDK_DVFC_FREQ_CPU               = 0,

    // PERIPHERAL setpoint frequencies
    DDK_DVFC_FREQ_AHB               = 0,
    DDK_DVFC_FREQ_DDR               = 1,

    DDK_DVFC_FREQ_ENUM_END          = 2
} DDK_DVFC_SETPOINT_FREQ;

typedef enum
{
    DDK_DVFC_DOMAIN_CPU             = 0,
    DDK_DVFC_DOMAIN_PERIPH          = 1,
    DDK_DVFC_DOMAIN_ENUM_END        = 2
} DDK_DVFC_DOMAIN;

typedef struct
{
    UINT32 mV;
    UINT32 freq[DDK_DVFC_FREQ_ENUM_END];
} DDK_DVFC_SETPOINT_INFO, *PDDK_DVFC_SETPOINT_INFO;

#define MAX_LEAF_NODES 8

typedef struct
{
    DDK_CLOCK_BAUD_SOURCE root;
    UINT32 numLeaf;
    DDK_CLOCK_GATE_INDEX leaf[MAX_LEAF_NODES];
} DDK_CLK_TREE_NODE;

typedef struct
{
    UINT32 refCount;
    DDK_CLOCK_GATE_MODE enabledMode;
    DDK_CLOCK_GATE_MODE disabledMode;
} DDK_CLK_REF_INFO;


//-----------------------------------------------------------------------------
//
//  Type: DDK_CLK_CONFIG
//
//  Shared global structure used to communicate clock and voltage/frequency
//  setpoint information between the CSPDDK and DVFC drivers.
//
// 
//-----------------------------------------------------------------------------
typedef struct
{
    // Shared clock management info
    UINT32 rootRefCount[DDK_CLOCK_BAUD_SOURCE_ENUM_END];
    DDK_CLK_TREE_NODE clkTreeNode[DDK_CLOCK_GATE_INDEX_ENUM_END];
    DDK_CLK_REF_INFO clkRefInfo[DDK_CLOCK_GATE_INDEX_ENUM_END];

    // Shared setpoint info
    BOOL bDvfcActive;
    BOOL bSetpointPending;
    DDK_DVFC_SETPOINT setpointCur[DDK_DVFC_DOMAIN_ENUM_END];
    DDK_DVFC_SETPOINT setpointMin[DDK_DVFC_DOMAIN_ENUM_END];
    DDK_DVFC_SETPOINT setpointMax[DDK_DVFC_DOMAIN_ENUM_END];
    DDK_DVFC_SETPOINT setpointLoad[DDK_DVFC_DOMAIN_ENUM_END];
    UINT32 setpointReqCount[DDK_DVFC_DOMAIN_ENUM_END][DDK_DVFC_SETPOINT_ENUM_END];
    DDK_DVFC_SETPOINT_INFO periphSetpointReq[DDK_CLOCK_GATE_INDEX_ENUM_END];
}  DDK_CLK_CONFIG, *PDDK_CLK_CONFIG;

//------------------------------------------------------------------------------
// Macros

// ROM ID is used to uniquely identify the silicon version:
//
//      ROM ID      Silicon Rev
//      -----------------------
//      0x01        TO1.0
//      0x02        TO1.1
//      0x10        TO2.0
//
#define DDK_SI_REV_TO1_0    0x01
#define DDK_SI_REV_TO1_1    0x02
#define DDK_SI_REV_TO2_0    0x10


//------------------------------------------------------------------------------
// Functions

BOOL DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX index, DDK_CLOCK_GATE_MODE mode);
BOOL DDKClockGetGatingMode(DDK_CLOCK_GATE_INDEX index, DDK_CLOCK_GATE_MODE *pMode);
BOOL DDKClockGetFreq(DDK_CLOCK_SIGNAL sig, UINT32 *freq);
BOOL DDKClockSetFreq(DDK_CLOCK_SIGNAL sig, UINT32 freq);
BOOL DDKClockConfigBaud(DDK_CLOCK_SIGNAL sig, DDK_CLOCK_BAUD_SOURCE src, UINT32 preDiv, UINT32 postDiv);
BOOL DDKClockSetCKO1(BOOL bEnable, DDK_CLOCK_CKO1_SRC index, DDK_CLOCK_CKO_DIV div);
BOOL DDKClockSetCKO2(BOOL bEnable, DDK_CLOCK_CKO2_SRC index, DDK_CLOCK_CKO_DIV div);
BOOL DDKClockSetpointRequest(DDK_DVFC_SETPOINT setpoint, DDK_DVFC_DOMAIN domain, BOOL bBlock);
BOOL DDKClockSetpointRelease(DDK_DVFC_SETPOINT setpoint, DDK_DVFC_DOMAIN domain);
BOOL DDKClockSetOverride(DDK_CLOCK_OVERRIDE_ENABLE_INDEX index, DDK_CLOCK_OVERRIDE_MODE mode);
BOOL DDKClockGetOverride(DDK_CLOCK_OVERRIDE_ENABLE_INDEX index, DDK_CLOCK_OVERRIDE_MODE *pMode);
 
#if __cplusplus
}
#endif

#endif // __MX51_DDK_H
