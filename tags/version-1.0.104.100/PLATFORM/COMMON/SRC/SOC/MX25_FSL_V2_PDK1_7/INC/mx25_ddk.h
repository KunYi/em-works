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
//  File:  mx25_ddk.h
//
//  Contains MX25 definitions to assist with driver development.
//
//------------------------------------------------------------------------------
#ifndef __MX25_DDK_H
#define __MX25_DDK_H

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
    DDK_CLOCK_SIGNAL_MCUPLL     = 0,
    DDK_CLOCK_SIGNAL_USBPLL     = 1,
    DDK_CLOCK_SIGNAL_ARM        = 2,
    DDK_CLOCK_SIGNAL_AHB        = 3,
    DDK_CLOCK_SIGNAL_IPG        = 4,
    DDK_CLOCK_SIGNAL_USBCLK     = 5,
    
    DDK_CLOCK_SIGNAL_END        = 6,

    DDK_CLOCK_SIGNAL_START_AHB_CLK      = 9,

    DDK_CLOCK_SIGNAL_AHB_ATA    = 10,
    DDK_CLOCK_SIGNAL_AHB_BROM   = 11,
    DDK_CLOCK_SIGNAL_AHB_CSI    = 12,
    DDK_CLOCK_SIGNAL_AHB_EMI    = 13,
    DDK_CLOCK_SIGNAL_AHB_ESAI   = 14,
    DDK_CLOCK_SIGNAL_AHB_ESDHC1 = 15,
    DDK_CLOCK_SIGNAL_AHB_ESDHC2 = 16,
    DDK_CLOCK_SIGNAL_AHB_FEC    = 17,
    DDK_CLOCK_SIGNAL_AHB_LCDC   = 18,
    DDK_CLOCK_SIGNAL_AHB_RTIC   = 19,
    DDK_CLOCK_SIGNAL_AHB_SDMA   = 20,
    DDK_CLOCK_SIGNAL_AHB_SLCDC  = 21,
    DDK_CLOCK_SIGNAL_AHB_USBOTG = 22,

    DDK_CLOCK_SIGNAL_START_PER_CLK = 29,

    DDK_CLOCK_SIGNAL_PER_CSI    = 30,
    DDK_CLOCK_SIGNAL_PER_EPIT   = 31, 
    DDK_CLOCK_SIGNAL_PER_ESAI   = 32, 
    DDK_CLOCK_SIGNAL_PER_ESDHC1 = 33, 
    DDK_CLOCK_SIGNAL_PER_ESDHC2 = 34, 
    DDK_CLOCK_SIGNAL_PER_GPT    = 35, 
    DDK_CLOCK_SIGNAL_PER_I2C    = 36, 
    DDK_CLOCK_SIGNAL_PER_LCDC   = 37,
    DDK_CLOCK_SIGNAL_PER_NFC    = 38,
    DDK_CLOCK_SIGNAL_PER_OWIRE  = 39,
    DDK_CLOCK_SIGNAL_PER_PWM    = 40,
    DDK_CLOCK_SIGNAL_PER_SIM1   = 41,
    DDK_CLOCK_SIGNAL_PER_SIM2   = 42,
    DDK_CLOCK_SIGNAL_PER_SSI1   = 43,
    DDK_CLOCK_SIGNAL_PER_SSI2   = 44,
    DDK_CLOCK_SIGNAL_PER_UART   = 45,

    DDK_CLOCK_SIGNAL_ENUM_END   = 46,

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
//CGR2
    DDK_CLOCK_GATE_INDEX_WDOG       = 83,   
    DDK_CLOCK_GATE_INDEX_UART5      = 82,
    DDK_CLOCK_GATE_INDEX_UART4      = 81,
    DDK_CLOCK_GATE_INDEX_UART3      = 80,
    DDK_CLOCK_GATE_INDEX_UART2      = 79,
    DDK_CLOCK_GATE_INDEX_UART1      = 78,
    DDK_CLOCK_GATE_INDEX_TCHSCRN    = 77,
    DDK_CLOCK_GATE_INDEX_SSI2       = 76,
    DDK_CLOCK_GATE_INDEX_SSI1       = 75,
    DDK_CLOCK_GATE_INDEX_SPBA       = 74,
    DDK_CLOCK_GATE_INDEX_SLCDC      = 73,
    DDK_CLOCK_GATE_INDEX_SIM2       = 72,
    DDK_CLOCK_GATE_INDEX_SIM1       = 71,
    DDK_CLOCK_GATE_INDEX_SDMA       = 70,
    DDK_CLOCK_GATE_INDEX_SCC        = 69,
    DDK_CLOCK_GATE_INDEX_RTIC       = 68,
    DDK_CLOCK_GATE_INDEX_RNGB       = 67,
    DDK_CLOCK_GATE_INDEX_PWM4       = 66,
    DDK_CLOCK_GATE_INDEX_PWM3       = 65,
    DDK_CLOCK_GATE_INDEX_PWM2       = 64,

//CGR1
    DDK_CLOCK_GATE_INDEX_PWM1       = 63,
    DDK_CLOCK_GATE_INDEX_OWIRE      = 62,
    DDK_CLOCK_GATE_INDEX_LCDC       = 61,
    DDK_CLOCK_GATE_INDEX_KPP        = 60,
    DDK_CLOCK_GATE_INDEX_IOMUXC     = 59,
    DDK_CLOCK_GATE_INDEX_IIM        = 58,
    DDK_CLOCK_GATE_INDEX_I2C3       = 57,
    DDK_CLOCK_GATE_INDEX_I2C2       = 56,
    DDK_CLOCK_GATE_INDEX_I2C1       = 55,
    DDK_CLOCK_GATE_INDEX_GPT4       = 54,
    DDK_CLOCK_GATE_INDEX_GPT3       = 53,
    DDK_CLOCK_GATE_INDEX_GPT2       = 52,
    DDK_CLOCK_GATE_INDEX_GPT1       = 51,
//  No clock gating for GPIOs
//    DDK_CLOCK_GATE_INDEX_GPIO3      = 50,
//    DDK_CLOCK_GATE_INDEX_GPIO2      = 49,
//    DDK_CLOCK_GATE_INDEX_GPIO1      = 48,
    DDK_CLOCK_GATE_INDEX_FEC        = 47,
    DDK_CLOCK_GATE_INDEX_ESDHC2     = 46,
    DDK_CLOCK_GATE_INDEX_ESDHC1     = 45,
    DDK_CLOCK_GATE_INDEX_ESAI       = 44,
    DDK_CLOCK_GATE_INDEX_EPIT2      = 43,
    DDK_CLOCK_GATE_INDEX_EPIT1      = 42,
    DDK_CLOCK_GATE_INDEX_ECT        = 41, 
    DDK_CLOCK_GATE_INDEX_RTC        = 40, 
    DDK_CLOCK_GATE_INDEX_CSPI3      = 39, 
    DDK_CLOCK_GATE_INDEX_CSPI2      = 38, 
    DDK_CLOCK_GATE_INDEX_CSPI1      = 37, 
    DDK_CLOCK_GATE_INDEX_CSI        = 36, 
    DDK_CLOCK_GATE_INDEX_CAN2       = 35, 
    DDK_CLOCK_GATE_INDEX_CAN1       = 34, 
    DDK_CLOCK_GATE_INDEX_ATA        = 33, 
    DDK_CLOCK_GATE_INDEX_AUDMUX     = 32, 
    
// CGR0
    DDK_CLOCK_GATE_INDEX_AHB_USBOTG = 28,
    DDK_CLOCK_GATE_INDEX_AHB_SLCDC  = 27,
    DDK_CLOCK_GATE_INDEX_AHB_SDMA   = 26,
    DDK_CLOCK_GATE_INDEX_AHB_RTIC   = 25,
    DDK_CLOCK_GATE_INDEX_AHB_LCDC   = 24,
    DDK_CLOCK_GATE_INDEX_AHB_FEC    = 23,
    DDK_CLOCK_GATE_INDEX_AHB_ESDHC2 = 22,
    DDK_CLOCK_GATE_INDEX_AHB_ESDHC1 = 21,
    DDK_CLOCK_GATE_INDEX_AHB_ESAI   = 20,
    DDK_CLOCK_GATE_INDEX_AHB_EMI    = 19,
    DDK_CLOCK_GATE_INDEX_AHB_CSI    = 18,
    DDK_CLOCK_GATE_INDEX_AHB_BROM   = 17,
    DDK_CLOCK_GATE_INDEX_AHB_ATA    = 16,
    DDK_CLOCK_GATE_INDEX_PER_UART   = 15,
    DDK_CLOCK_GATE_INDEX_PER_SSI2   = 14,
    DDK_CLOCK_GATE_INDEX_PER_SSI1   = 13,
    DDK_CLOCK_GATE_INDEX_PER_SIM2   = 12,
    DDK_CLOCK_GATE_INDEX_PER_SIM1   = 11,
    DDK_CLOCK_GATE_INDEX_PER_PWM    = 10,
    DDK_CLOCK_GATE_INDEX_PER_OWIRE  = 9,
    DDK_CLOCK_GATE_INDEX_PER_NFC    = 8,
    DDK_CLOCK_GATE_INDEX_PER_LCDC   = 7,
    DDK_CLOCK_GATE_INDEX_PER_I2C    = 6, 
    DDK_CLOCK_GATE_INDEX_PER_GPT    = 5, 
    DDK_CLOCK_GATE_INDEX_PER_ESDHC2 = 4, 
    DDK_CLOCK_GATE_INDEX_PER_ESDHC1 = 3, 
    DDK_CLOCK_GATE_INDEX_PER_ESAI   = 2, 
    DDK_CLOCK_GATE_INDEX_PER_EPIT   = 1, 
    DDK_CLOCK_GATE_INDEX_PER_CSI    = 0,



} DDK_CLOCK_GATE_INDEX;


//-----------------------------------------------------------------------------
//
//  Type: DDK_CLOCK_GATE_MODE
//
//  Clock gating modes supported by CCM clock gating registers.
//
//-----------------------------------------------------------------------------
typedef enum {
    DDK_CLOCK_GATE_MODE_DISABLED    = 0,
    DDK_CLOCK_GATE_MODE_ENABLED     = 1
} DDK_CLOCK_GATE_MODE;


//-----------------------------------------------------------------------------
//
//  Type: DDK_CLOCK_BAUD_SOURCE
//
//  Input source for baud clock generation.
//  Warning: The value set here are index in the clkFreq table in bsp args.
//  So keep the consistency between DDK_CLOCK_BAUD_SOURCE and DDK_CLOCK_SIGNAL.
// 
//-----------------------------------------------------------------------------
typedef enum {
    DDK_CLOCK_BAUD_SOURCE_USBPLL        = 1,
    DDK_CLOCK_BAUD_SOURCE_AHB           = 3
} DDK_CLOCK_BAUD_SOURCE;

/* JJH 
//-----------------------------------------------------------------------------
//
//  Type: DDK_CLOCK_ACM
//
//  Module name of Audio Clock Mux (ACM) configuration.
//
//-----------------------------------------------------------------------------
typedef enum {
    DDK_CLOCK_ACM_SSI2                      = CCM_ACMR_SSI2_AUDIO_CLK_SEL_LSH,
    DDK_CLOCK_ACM_SSI1                      = CCM_ACMR_SSI1_AUDIO_CLK_SEL_LSH,
    DDK_CLOCK_ACM_SPDIF                     = CCM_ACMR_SPDIF_AUDIO_CLK_SEL_LSH,
    DDK_CLOCK_ACM_ESAI                      = CCM_ACMR_ESAI_AUDIO_CLK_SEL_LSH
} DDK_CLOCK_ACM;


//-----------------------------------------------------------------------------
//
//  Type: DDK_CLOCK_ACM_SRC
//
//  Clock source selections of Audio Clock Mux (ACM) configuration.
//
//-----------------------------------------------------------------------------
typedef enum {
    DDK_CLOCK_ACM_SRC_OSC_AUDIO             = 0,
    DDK_CLOCK_ACM_SRC_ESAI_SCKT             = 1,
    DDK_CLOCK_ACM_SRC_ESAI_SCKR             = 2,
    DDK_CLOCK_ACM_SRC_SPDIF_HCKT            = 3,
    DDK_CLOCK_ACM_SRC_SSI1_SRCK             = 4,
    DDK_CLOCK_ACM_SRC_SSI1_STCK             = 5,
    DDK_CLOCK_ACM_SRC_SSI2_SRCK             = 6,
    DDK_CLOCK_ACM_SRC_SSI2_STCK             = 7,
    DDK_CLOCK_ACM_SRC_CCM_OSCAUDIO          = 8,
    DDK_CLOCK_ACM_SRC_SPDIF_OUTCLK          = 9,
    DDK_CLOCK_ACM_SRC_SPDIF_SRCLK           = 10
} DDK_CLOCK_ACM_SRC;
*/

//-----------------------------------------------------------------------------
//
//  Type: DDK_CLOCK_CKO_SRC
//
//  Clock output source (CKO) signal selections.
//
//-----------------------------------------------------------------------------
typedef enum {
DDK_CLOCK_CKO_SRC_32K_CLK               = 0,
DDK_CLOCK_CKO_SRC_24M_CLK               = 1,
DDK_CLOCK_CKO_SRC_UNGATED_ARM_CLK       = 2,
DDK_CLOCK_CKO_SRC_UNGATED_AHB_CLK       = 3,
DDK_CLOCK_CKO_SRC_UNGATED_IPG_CLK       = 4,
DDK_CLOCK_CKO_SRC_PER_CLK_0_SRC         = 5,
DDK_CLOCK_CKO_SRC_USB_CLK_SRC           = 6,
DDK_CLOCK_CKO_SRC_GATED_ARM_CLK         = 7,
DDK_CLOCK_CKO_SRC_GATED_AHB_CLK         = 8,
DDK_CLOCK_CKO_SRC_GATED_IPG_CLK         = 9,
DDK_CLOCK_CKO_SRC_PER_CLK0              = 10,
DDK_CLOCK_CKO_SRC_PER_CLK2              = 11,
DDK_CLOCK_CKO_SRC_PER_CLK13             = 12,
DDK_CLOCK_CKO_SRC_PER_CLK14             = 13,
DDK_CLOCK_CKO_SRC_USB_CLK               = 14
} DDK_CLOCK_CKO_SRC;


//-----------------------------------------------------------------------------
//
//  Type: DDK_IOMUX_PIN
//
//  Specifies the functional pin name used to configure the IOMUX SW_MUX_CTL.
//
//-----------------------------------------------------------------------------
typedef enum {
    DDK_IOMUX_PIN_A10              = (0),
    DDK_IOMUX_PIN_A13              = (1),
    DDK_IOMUX_PIN_A14              = (2),
    DDK_IOMUX_PIN_A15              = (3),
    DDK_IOMUX_PIN_A16              = (4),
    DDK_IOMUX_PIN_A17              = (5),
    DDK_IOMUX_PIN_A18              = (6),
    DDK_IOMUX_PIN_A19              = (7),
    DDK_IOMUX_PIN_A20              = (8),
    DDK_IOMUX_PIN_A21              = (9),
    DDK_IOMUX_PIN_A22              = (10),
    DDK_IOMUX_PIN_A23              = (11),
    DDK_IOMUX_PIN_A24              = (12),
    DDK_IOMUX_PIN_A25              = (13),
    DDK_IOMUX_PIN_EB0              = (14),
    DDK_IOMUX_PIN_EB1              = (15),
    DDK_IOMUX_PIN_OE               = (16),
    DDK_IOMUX_PIN_CS0              = (17),
    DDK_IOMUX_PIN_CS1              = (18),
    DDK_IOMUX_PIN_CS4              = (19),
    DDK_IOMUX_PIN_CS5              = (20),
    DDK_IOMUX_PIN_NF_CE0           = (21),
    DDK_IOMUX_PIN_ECB              = (22),
    DDK_IOMUX_PIN_LBA              = (23),
    DDK_IOMUX_PIN_BCLK             = (24),
    DDK_IOMUX_PIN_RW               = (25),
    DDK_IOMUX_PIN_NFWE_B           = (26),
    DDK_IOMUX_PIN_NFRE_B           = (27),
    DDK_IOMUX_PIN_NFALE            = (28),
    DDK_IOMUX_PIN_NFCLE            = (29),
    DDK_IOMUX_PIN_NFWP_B           = (30),
    DDK_IOMUX_PIN_NFRB             = (31),
    DDK_IOMUX_PIN_D15              = (32),
    DDK_IOMUX_PIN_D14              = (33),
    DDK_IOMUX_PIN_D13              = (34),
    DDK_IOMUX_PIN_D12              = (35),
    DDK_IOMUX_PIN_D11              = (36),
    DDK_IOMUX_PIN_D10              = (37),
    DDK_IOMUX_PIN_D9               = (38),
    DDK_IOMUX_PIN_D8               = (39),
    DDK_IOMUX_PIN_D7               = (40),
    DDK_IOMUX_PIN_D6               = (41),
    DDK_IOMUX_PIN_D5               = (42),
    DDK_IOMUX_PIN_D4               = (43),
    DDK_IOMUX_PIN_D3               = (44),
    DDK_IOMUX_PIN_D2               = (45),
    DDK_IOMUX_PIN_D1               = (46),
    DDK_IOMUX_PIN_D0               = (47),
    DDK_IOMUX_PIN_LD0              = (48),
    DDK_IOMUX_PIN_LD1              = (49),
    DDK_IOMUX_PIN_LD2              = (50),
    DDK_IOMUX_PIN_LD3              = (51),
    DDK_IOMUX_PIN_LD4              = (52),
    DDK_IOMUX_PIN_LD5              = (53),
    DDK_IOMUX_PIN_LD6              = (54),
    DDK_IOMUX_PIN_LD7              = (55),
    DDK_IOMUX_PIN_LD8              = (56),
    DDK_IOMUX_PIN_LD9              = (57),
    DDK_IOMUX_PIN_LD10             = (58),
    DDK_IOMUX_PIN_LD11             = (59),
    DDK_IOMUX_PIN_LD12             = (60),
    DDK_IOMUX_PIN_LD13             = (61),
    DDK_IOMUX_PIN_LD14             = (62),
    DDK_IOMUX_PIN_LD15             = (63),
    DDK_IOMUX_PIN_HSYNC            = (64),
    DDK_IOMUX_PIN_VSYNC            = (65),
    DDK_IOMUX_PIN_LSCLK            = (66),
    DDK_IOMUX_PIN_OE_ACD           = (67),
    DDK_IOMUX_PIN_CONTRAST         = (68),
    DDK_IOMUX_PIN_PWM              = (69),
    DDK_IOMUX_PIN_CSI_D2           = (70),
    DDK_IOMUX_PIN_CSI_D3           = (71),
    DDK_IOMUX_PIN_CSI_D4           = (72),
    DDK_IOMUX_PIN_CSI_D5           = (73),
    DDK_IOMUX_PIN_CSI_D6           = (74),
    DDK_IOMUX_PIN_CSI_D7           = (75),
    DDK_IOMUX_PIN_CSI_D8           = (76),
    DDK_IOMUX_PIN_CSI_D9           = (77),
    DDK_IOMUX_PIN_CSI_MCLK         = (78),
    DDK_IOMUX_PIN_CSI_VSYNC        = (79),
    DDK_IOMUX_PIN_CSI_HSYNC        = (80),
    DDK_IOMUX_PIN_CSI_PIXCLK       = (81),
    DDK_IOMUX_PIN_I2C1_CLK         = (82),
    DDK_IOMUX_PIN_I2C1_DAT         = (83),
    DDK_IOMUX_PIN_CSPI1_MOSI       = (84),
    DDK_IOMUX_PIN_CSPI1_MISO       = (85),
    DDK_IOMUX_PIN_CSPI1_SS0        = (86),
    DDK_IOMUX_PIN_CSPI1_SS1        = (87),
    DDK_IOMUX_PIN_CSPI1_SCLK       = (88),
    DDK_IOMUX_PIN_CSPI1_RDY        = (89),
    DDK_IOMUX_PIN_UART1_RXD        = (90),
    DDK_IOMUX_PIN_UART1_TXD        = (91),
    DDK_IOMUX_PIN_UART1_RTS        = (92),
    DDK_IOMUX_PIN_UART1_CTS        = (93),
    DDK_IOMUX_PIN_UART2_RXD        = (94),
    DDK_IOMUX_PIN_UART2_TXD        = (95),
    DDK_IOMUX_PIN_UART2_RTS        = (96),
    DDK_IOMUX_PIN_UART2_CTS        = (97),
    DDK_IOMUX_PIN_SD1_CMD          = (98),
    DDK_IOMUX_PIN_SD1_CLK          = (99),
    DDK_IOMUX_PIN_SD1_DATA0        = (100),
    DDK_IOMUX_PIN_SD1_DATA1        = (101),
    DDK_IOMUX_PIN_SD1_DATA2        = (102),
    DDK_IOMUX_PIN_SD1_DATA3        = (103),
    DDK_IOMUX_PIN_KPP_ROW0         = (104),
    DDK_IOMUX_PIN_KPP_ROW1         = (105),
    DDK_IOMUX_PIN_KPP_ROW2         = (106),
    DDK_IOMUX_PIN_KPP_ROW3         = (107),
    DDK_IOMUX_PIN_KPP_COL0         = (108),
    DDK_IOMUX_PIN_KPP_COL1         = (109),
    DDK_IOMUX_PIN_KPP_COL2         = (110),
    DDK_IOMUX_PIN_KPP_COL3         = (111),
    DDK_IOMUX_PIN_FEC_MDC          = (112),
    DDK_IOMUX_PIN_FEC_MDIO         = (113),
    DDK_IOMUX_PIN_FEC_TDATA0       = (114),
    DDK_IOMUX_PIN_FEC_TDATA1       = (115),
    DDK_IOMUX_PIN_FEC_TX_EN        = (116),
    DDK_IOMUX_PIN_FEC_RDATA0       = (117),
    DDK_IOMUX_PIN_FEC_RDATA1       = (118),
    DDK_IOMUX_PIN_FEC_RX_DV        = (119),
    DDK_IOMUX_PIN_FEC_TX_CLK       = (120),
    DDK_IOMUX_PIN_RTCK             = (121),
    DDK_IOMUX_PIN_DE_B             = (122),
    DDK_IOMUX_PIN_GPIO_A           = (123),
    DDK_IOMUX_PIN_GPIO_B           = (124),
    DDK_IOMUX_PIN_GPIO_C           = (125),
    DDK_IOMUX_PIN_GPIO_D           = (126),
    DDK_IOMUX_PIN_GPIO_E           = (127),
    DDK_IOMUX_PIN_GPIO_F           = (128),
    DDK_IOMUX_PIN_EXT_ARMCLK       = (129),
    DDK_IOMUX_PIN_UPLL_BYPCLK      = (130),
    DDK_IOMUX_PIN_VSTBY_REQ        = (131),
    DDK_IOMUX_PIN_VSTBY_ACK        = (132),
    DDK_IOMUX_PIN_POWER_FAIL       = (133),
    DDK_IOMUX_PIN_CLKO             = (134),
    DDK_IOMUX_PIN_BOOT_MODE0       = (135),
    DDK_IOMUX_PIN_BOOT_MODE1       = (136),
} DDK_IOMUX_PIN;


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


//-----------------------------------------------------------------------------
//
//  Type: DDK_IOMUX_PAD
//
//  Specifies the functional pad name used to configure the IOMUX SW_PAD_CTL.
//
//-----------------------------------------------------------------------------
typedef enum {
    DDK_IOMUX_PAD_A13                   = (0),
    DDK_IOMUX_PAD_A14                   = (1),
    DDK_IOMUX_PAD_A15                   = (2),
    DDK_IOMUX_PAD_A17                   = (3),
    DDK_IOMUX_PAD_A18                   = (4),
    DDK_IOMUX_PAD_A19                   = (5),
    DDK_IOMUX_PAD_A20                   = (6),
    DDK_IOMUX_PAD_A21                   = (7),
    DDK_IOMUX_PAD_A23                   = (8),
    DDK_IOMUX_PAD_A24                   = (9),
    DDK_IOMUX_PAD_A25                   = (10),
    DDK_IOMUX_PAD_EB0                   = (11),
    DDK_IOMUX_PAD_EB1                   = (12),
    DDK_IOMUX_PAD_OE                    = (13),
    DDK_IOMUX_PAD_CS4                   = (14),
    DDK_IOMUX_PAD_CS5                   = (15),
    DDK_IOMUX_PAD_NF_CE0                = (16),
    DDK_IOMUX_PAD_ECB                   = (17),
    DDK_IOMUX_PAD_LBA                   = (18),
    DDK_IOMUX_PAD_RW                    = (19),
    DDK_IOMUX_PAD_NFRB                  = (20),
    DDK_IOMUX_PAD_D15                   = (21),
    DDK_IOMUX_PAD_D14                   = (22),
    DDK_IOMUX_PAD_D13                   = (23),
    DDK_IOMUX_PAD_D12                   = (24),
    DDK_IOMUX_PAD_D11                   = (25),
    DDK_IOMUX_PAD_D10                   = (26),
    DDK_IOMUX_PAD_D9                    = (27),
    DDK_IOMUX_PAD_D8                    = (28),
    DDK_IOMUX_PAD_D7                    = (29),
    DDK_IOMUX_PAD_D6                    = (30),
    DDK_IOMUX_PAD_D5                    = (31),
    DDK_IOMUX_PAD_D4                    = (32),
    DDK_IOMUX_PAD_D3                    = (33),
    DDK_IOMUX_PAD_D2                    = (34),
    DDK_IOMUX_PAD_D1                    = (35),
    DDK_IOMUX_PAD_D0                    = (36),
    DDK_IOMUX_PAD_LD0                   = (37),
    DDK_IOMUX_PAD_LD1                   = (38),
    DDK_IOMUX_PAD_LD2                   = (39),
    DDK_IOMUX_PAD_LD3                   = (40),
    DDK_IOMUX_PAD_LD4                   = (41),
    DDK_IOMUX_PAD_LD5                   = (42),
    DDK_IOMUX_PAD_LD6                   = (43),
    DDK_IOMUX_PAD_LD7                   = (44),
    DDK_IOMUX_PAD_LD8                   = (45),
    DDK_IOMUX_PAD_LD9                   = (46),
    DDK_IOMUX_PAD_LD10                  = (47),
    DDK_IOMUX_PAD_LD11                  = (48),
    DDK_IOMUX_PAD_LD12                  = (49),
    DDK_IOMUX_PAD_LD13                  = (50),
    DDK_IOMUX_PAD_LD14                  = (51),
    DDK_IOMUX_PAD_LD15                  = (52),
    DDK_IOMUX_PAD_HSYNC                 = (53),
    DDK_IOMUX_PAD_VSYNC                 = (54),
    DDK_IOMUX_PAD_LSCLK                 = (55),
    DDK_IOMUX_PAD_OE_ACD                = (56),
    DDK_IOMUX_PAD_CONTRAST              = (57),
    DDK_IOMUX_PAD_PWM                   = (58),
    DDK_IOMUX_PAD_CSI_D2                = (59),
    DDK_IOMUX_PAD_CSI_D3                = (60),
    DDK_IOMUX_PAD_CSI_D4                = (61),
    DDK_IOMUX_PAD_CSI_D5                = (62),
    DDK_IOMUX_PAD_CSI_D6                = (63),
    DDK_IOMUX_PAD_CSI_D7                = (64),
    DDK_IOMUX_PAD_CSI_D8                = (65),
    DDK_IOMUX_PAD_CSI_D9                = (66),
    DDK_IOMUX_PAD_CSI_MCLK              = (67),
    DDK_IOMUX_PAD_CSI_VSYNC             = (68),
    DDK_IOMUX_PAD_CSI_HSYNC             = (69),
    DDK_IOMUX_PAD_CSI_PIXCLK            = (70),
    DDK_IOMUX_PAD_I2C1_CLK              = (71),
    DDK_IOMUX_PAD_I2C1_DAT              = (72),
    DDK_IOMUX_PAD_CSPI1_MOSI            = (73),
    DDK_IOMUX_PAD_CSPI1_MISO            = (74),
    DDK_IOMUX_PAD_CSPI1_SS0             = (75),
    DDK_IOMUX_PAD_CSPI1_SS1             = (76),
    DDK_IOMUX_PAD_CSPI1_SCLK            = (77),
    DDK_IOMUX_PAD_CSPI1_RDY             = (78),
    DDK_IOMUX_PAD_UART1_RXD             = (79),
    DDK_IOMUX_PAD_UART1_TXD             = (80),
    DDK_IOMUX_PAD_UART1_RTS             = (81),
    DDK_IOMUX_PAD_UART1_CTS             = (82),
    DDK_IOMUX_PAD_UART2_RXD             = (83),
    DDK_IOMUX_PAD_UART2_TXD             = (84),
    DDK_IOMUX_PAD_UART2_RTS             = (85),
    DDK_IOMUX_PAD_UART2_CTS             = (86),
    DDK_IOMUX_PAD_SD1_CMD               = (87),
    DDK_IOMUX_PAD_SD1_CLK               = (88),
    DDK_IOMUX_PAD_SD1_DATA0             = (89),
    DDK_IOMUX_PAD_SD1_DATA1             = (90),
    DDK_IOMUX_PAD_SD1_DATA2             = (91),
    DDK_IOMUX_PAD_SD1_DATA3             = (92),
    DDK_IOMUX_PAD_KPP_ROW0              = (93),
    DDK_IOMUX_PAD_KPP_ROW1              = (94),
    DDK_IOMUX_PAD_KPP_ROW2              = (95),
    DDK_IOMUX_PAD_KPP_ROW3              = (96),
    DDK_IOMUX_PAD_KPP_COL0              = (97),
    DDK_IOMUX_PAD_KPP_COL1              = (98),
    DDK_IOMUX_PAD_KPP_COL2              = (99),
    DDK_IOMUX_PAD_KPP_COL3              = (100),
    DDK_IOMUX_PAD_FEC_MDC               = (101),
    DDK_IOMUX_PAD_FEC_MDIO              = (102),
    DDK_IOMUX_PAD_FEC_TDATA0            = (103),
    DDK_IOMUX_PAD_FEC_TDATA1            = (104),
    DDK_IOMUX_PAD_FEC_TX_EN             = (105),
    DDK_IOMUX_PAD_FEC_RDATA0            = (106),
    DDK_IOMUX_PAD_FEC_RDATA1            = (107),
    DDK_IOMUX_PAD_FEC_RX_DV             = (108),
    DDK_IOMUX_PAD_FEC_TX_CLK            = (109),
    DDK_IOMUX_PAD_RTCK                  = (110),
    DDK_IOMUX_PAD_TDO                   = (111),
    DDK_IOMUX_PAD_DE_B                  = (112),
    DDK_IOMUX_PAD_GPIO_A                = (113),
    DDK_IOMUX_PAD_GPIO_B                = (114),
    DDK_IOMUX_PAD_GPIO_C                = (115),
    DDK_IOMUX_PAD_GPIO_D                = (116),
    DDK_IOMUX_PAD_GPIO_E                = (117),
    DDK_IOMUX_PAD_GPIO_F                = (118),
    DDK_IOMUX_PAD_VSTBY_REQ             = (119),
    DDK_IOMUX_PAD_VSTBY_ACK             = (120),
    DDK_IOMUX_PAD_POWER_FAIL            = (121),
    DDK_IOMUX_PAD_CLKO                  = (122),

	DDK_IOMUX_PAD_NULL                  = (1000),
} DDK_IOMUX_PAD;


//-----------------------------------------------------------------------------
//
//  Type: DDK_IOMUX_PAD_SLEW
//
//  Specifies the slew rate for a pad.
//
//-----------------------------------------------------------------------------
typedef enum 
{
    DDK_IOMUX_PAD_SLEW_SLOW = (IOMUX_SW_PAD_CTL_SRE_SLOW << IOMUX_SW_PAD_CTL_SRE_LSH),
    DDK_IOMUX_PAD_SLEW_FAST = (IOMUX_SW_PAD_CTL_SRE_FAST << IOMUX_SW_PAD_CTL_SRE_LSH),
} DDK_IOMUX_PAD_SLEW;


//-----------------------------------------------------------------------------
//
//  Type: DDK_IOMUX_PAD_DRIVE
//
//  Specifies the drive strength for a pad.
//
//-----------------------------------------------------------------------------
typedef enum 
{
    DDK_IOMUX_PAD_DRIVE_NORMAL  = (IOMUX_SW_PAD_CTL_DSE_NORMAL << IOMUX_SW_PAD_CTL_DSE_LSH),
    DDK_IOMUX_PAD_DRIVE_HIGH    = (IOMUX_SW_PAD_CTL_DSE_HIGH << IOMUX_SW_PAD_CTL_DSE_LSH),
    DDK_IOMUX_PAD_DRIVE_MAX     = (IOMUX_SW_PAD_CTL_DSE_MAX << IOMUX_SW_PAD_CTL_DSE_LSH)
} DDK_IOMUX_PAD_DRIVE;


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


//-----------------------------------------------------------------------------
//
//  Type: DDK_IOMUX_PAD_PULL
//
//  Specifies the pull-up/pull-down/keeper configuration for a pad.
//
//-----------------------------------------------------------------------------
typedef enum 
{
    DDK_IOMUX_PAD_PULL_NONE         =   (IOMUX_SW_PAD_CTL_PKE_DISABLE << IOMUX_SW_PAD_CTL_PKE_LSH),
    DDK_IOMUX_PAD_PULL_KEEPER       =   ((IOMUX_SW_PAD_CTL_PUE_KEEPER_ENABLE << IOMUX_SW_PAD_CTL_PUE_LSH) | 
                                        (IOMUX_SW_PAD_CTL_PKE_ENABLE << IOMUX_SW_PAD_CTL_PKE_LSH)),
    DDK_IOMUX_PAD_PULL_DOWN_100K    =   ((IOMUX_SW_PAD_CTL_PUS_100K_DOWN << IOMUX_SW_PAD_CTL_PUS_LSH) | 
                                        (IOMUX_SW_PAD_CTL_PUE_PULL_ENABLE << IOMUX_SW_PAD_CTL_PUE_LSH) | 
                                        (IOMUX_SW_PAD_CTL_PKE_ENABLE << IOMUX_SW_PAD_CTL_PKE_LSH)),
    DDK_IOMUX_PAD_PULL_UP_47K       =   ((IOMUX_SW_PAD_CTL_PUS_47K_UP << IOMUX_SW_PAD_CTL_PUS_LSH) | 
                                        (IOMUX_SW_PAD_CTL_PUE_PULL_ENABLE << IOMUX_SW_PAD_CTL_PUE_LSH) | 
                                        (IOMUX_SW_PAD_CTL_PKE_ENABLE << IOMUX_SW_PAD_CTL_PKE_LSH)),
    DDK_IOMUX_PAD_PULL_UP_100K      =   ((IOMUX_SW_PAD_CTL_PUS_100K_UP << IOMUX_SW_PAD_CTL_PUS_LSH) | 
                                        (IOMUX_SW_PAD_CTL_PUE_PULL_ENABLE << IOMUX_SW_PAD_CTL_PUE_LSH) | 
                                        (IOMUX_SW_PAD_CTL_PKE_ENABLE << IOMUX_SW_PAD_CTL_PKE_LSH)),
    DDK_IOMUX_PAD_PULL_UP_22K       =   ((IOMUX_SW_PAD_CTL_PUS_22K_UP << IOMUX_SW_PAD_CTL_PUS_LSH) | 
                                        (IOMUX_SW_PAD_CTL_PUE_PULL_ENABLE << IOMUX_SW_PAD_CTL_PUE_LSH) | 
                                        (IOMUX_SW_PAD_CTL_PKE_ENABLE << IOMUX_SW_PAD_CTL_PKE_LSH)),
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


//-----------------------------------------------------------------------------
//
//  Type: DDK_IOMUX_PAD_VOLTAGE
//
//  Specifies the driver voltage for a pad.
//
//-----------------------------------------------------------------------------
typedef enum 
{
    DDK_IOMUX_PAD_VOLTAGE_3V3   = (IOMUX_SW_PAD_CTL_DRIVE_VOLTAGE_3V3 << IOMUX_SW_PAD_CTL_DRIVE_VOLTAGE_LSH),
    DDK_IOMUX_PAD_VOLTAGE_1V8   = (IOMUX_SW_PAD_CTL_DRIVE_VOLTAGE_1V8 << IOMUX_SW_PAD_CTL_DRIVE_VOLTAGE_LSH)
} DDK_IOMUX_PAD_VOLTAGE;


//-----------------------------------------------------------------------------
//
//  Type: DDK_IOMUX_SELEIN
//
//  Specifies the ports that have select input configuration.
//
//-----------------------------------------------------------------------------
typedef enum {
    DDK_IOMUX_SELEIN_AUDMUX_P4_INPUT_DA_AMX             =(0),
    DDK_IOMUX_SELEIN_AUDMUX_P4_INPUT_DB_AMX             =(1),
    DDK_IOMUX_SELEIN_AUDMUX_P4_INPUT_RXCLK_AMX          =(2),
    DDK_IOMUX_SELEIN_AUDMUX_P4_INPUT_RXFS_AMX           =(3),
    DDK_IOMUX_SELEIN_AUDMUX_P4_INPUT_TXCLK_AMX          =(4),
    DDK_IOMUX_SELEIN_AUDMUX_P4_INPUT_TXFS_AMX           =(5),
    DDK_IOMUX_SELEIN_AUDMUX_P7_INPUT_DA_AMX             =(6),
    DDK_IOMUX_SELEIN_AUDMUX_P7_INPUT_TXFS_AMX           =(7),
    DDK_IOMUX_SELEIN_CAN1_IPP_IND_CANRX                 =(8),
    DDK_IOMUX_SELEIN_CAN2_IPP_IND_CANRX                 =(9),
    DDK_IOMUX_SELEIN_CSI_IPP_CSI_D_0                    =(10),
    DDK_IOMUX_SELEIN_CSI_IPP_CSI_D_1                    =(11),
    DDK_IOMUX_SELEIN_CSPI1_IPP_IND_SS3_B                =(12),
    DDK_IOMUX_SELEIN_CSPI2_IPP_CSPI_CLK_IN              =(13),
    DDK_IOMUX_SELEIN_CSPI2_IPP_IND_DATAREADY_B          =(14),
    DDK_IOMUX_SELEIN_CSPI2_IPP_IND_MISO                 =(15),
    DDK_IOMUX_SELEIN_CSPI2_IPP_IND_MOSI                 =(16),
    DDK_IOMUX_SELEIN_CSPI2_IPP_IND_SS0_B                =(17),
    DDK_IOMUX_SELEIN_CSPI2_IPP_IND_SS1_B                =(18),
    DDK_IOMUX_SELEIN_CSPI3_IPP_CSPI_CLK_IN              =(19),
    DDK_IOMUX_SELEIN_CSPI3_IPP_IND_DATAREADY_B          =(20),
    DDK_IOMUX_SELEIN_CSPI3_IPP_IND_MISO                 =(21),
    DDK_IOMUX_SELEIN_CSPI3_IPP_IND_MOSI                 =(22),
    DDK_IOMUX_SELEIN_CSPI3_IPP_IND_SS0_B                =(23),
    DDK_IOMUX_SELEIN_CSPI3_IPP_IND_SS1_B                =(24),
    DDK_IOMUX_SELEIN_CSPI3_IPP_IND_SS2_B                =(25),
    DDK_IOMUX_SELEIN_CSPI3_IPP_IND_SS3_B                =(26),
    DDK_IOMUX_SELEIN_ESDHC1_IPP_DAT4_IN                 =(27),
    DDK_IOMUX_SELEIN_ESDHC1_IPP_DAT5_IN                 =(28),
    DDK_IOMUX_SELEIN_ESDHC1_IPP_DAT6_IN                 =(29),
    DDK_IOMUX_SELEIN_ESDHC1_IPP_DAT7_IN                 =(30),
    DDK_IOMUX_SELEIN_ESDHC2_IPP_CARD_CLK_IN             =(31),
    DDK_IOMUX_SELEIN_ESDHC2_IPP_CMD_IN                  =(32),
    DDK_IOMUX_SELEIN_ESDHC2_IPP_DAT0_IN                 =(33),
    DDK_IOMUX_SELEIN_ESDHC2_IPP_DAT1_IN                 =(34),
    DDK_IOMUX_SELEIN_ESDHC2_IPP_DAT2_IN                 =(35),
    DDK_IOMUX_SELEIN_ESDHC2_IPP_DAT3_IN                 =(36),
    DDK_IOMUX_SELEIN_ESDHC2_IPP_DAT4_IN                 =(37),
    DDK_IOMUX_SELEIN_ESDHC2_IPP_DAT5_IN                 =(38),
    DDK_IOMUX_SELEIN_ESDHC2_IPP_DAT6_IN                 =(39),
    DDK_IOMUX_SELEIN_ESDHC2_IPP_DAT7_IN                 =(40),
    DDK_IOMUX_SELEIN_FEC_FEC_COL                        =(41),
    DDK_IOMUX_SELEIN_FEC_FEC_CRS                        =(42),
    DDK_IOMUX_SELEIN_FEC_FEC_RDATA_2                    =(43),
    DDK_IOMUX_SELEIN_FEC_FEC_RDATA_3                    =(44),
    DDK_IOMUX_SELEIN_FEC_FEC_RX_CLK                     =(45),
    DDK_IOMUX_SELEIN_FEC_FEC_RX_ER                      =(46),
    DDK_IOMUX_SELEIN_I2C2_IPP_SCL_IN                    =(47),
    DDK_IOMUX_SELEIN_I2C2_IPP_SDA_IN                    =(48),
    DDK_IOMUX_SELEIN_I2C3_IPP_SCL_IN                    =(49),
    DDK_IOMUX_SELEIN_I2C3_IPP_SDA_IN                    =(50),
    DDK_IOMUX_SELEIN_KPP_IPP_IND_COL_4                  =(51),
    DDK_IOMUX_SELEIN_KPP_IPP_IND_COL_5                  =(52),
    DDK_IOMUX_SELEIN_KPP_IPP_IND_COL_6                  =(53),
    DDK_IOMUX_SELEIN_KPP_IPP_IND_COL_7                  =(54),
    DDK_IOMUX_SELEIN_KPP_IPP_IND_ROW_4                  =(55),
    DDK_IOMUX_SELEIN_KPP_IPP_IND_ROW_5                  =(56),
    DDK_IOMUX_SELEIN_KPP_IPP_IND_ROW_6                  =(57),
    DDK_IOMUX_SELEIN_KPP_IPP_IND_ROW_7                  =(58),
    DDK_IOMUX_SELEIN_SIM1_PIN_SIM_RCVD1_IN              =(59),
    DDK_IOMUX_SELEIN_SIM1_PIN_SIM_SIMPD1                =(60),
    DDK_IOMUX_SELEIN_SIM1_SIM_RCVD1_IO                  =(61),
    DDK_IOMUX_SELEIN_SIM2_PIN_SIM_RCVD1_IN              =(62),
    DDK_IOMUX_SELEIN_SIM2_PIN_SIM_SIMPD1                =(63),
    DDK_IOMUX_SELEIN_SIM2_SIM_RCVD1_IO                  =(64),
    DDK_IOMUX_SELEIN_UART3_IPP_UART_RTS_B               =(65),
    DDK_IOMUX_SELEIN_UART3_IPP_UART_RXD_MUX             =(66),
    DDK_IOMUX_SELEIN_UART4_IPP_UART_RTS_B               =(67),
    DDK_IOMUX_SELEIN_UART4_IPP_UART_RXD_MUX             =(68),
    DDK_IOMUX_SELEIN_UART5_IPP_UART_RTS_B               =(69),
    DDK_IOMUX_SELEIN_UART5_IPP_UART_RXD_MUX             =(70),
    DDK_IOMUX_SELEIN_USB_TOP_IPP_IND_OTG_USB_OC         =(71),
    DDK_IOMUX_SELEIN_USB_TOP_IPP_IND_UH2_USB_OC         =(72),
} DDK_IOMUX_SELEIN;

//------------------------------------------------------------------------------
// Macros
#define DDK_INDEX_TO_CPU_INDEX(x) ((x)-(DDK_CLOCK_SIGNAL_START_PER_CLK + 1))  //convert the DDK index into the SOC index (PER0 is the CSI per clock)

// Silicon version:
//
//      Define      Silicon Rev
//      -----------------------
//      0x01        TO1.0
//      0x02        TO1.1
//
#define DDK_SI_REV_TO1_0    0x01
#define DDK_SI_REV_TO1_1    0x02


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
    DDK_DVFC_FREQ_AHB               = 1,

    DDK_DVFC_FREQ_ENUM_END          = 2
} DDK_DVFC_SETPOINT_FREQ;

typedef enum
{
    DDK_DVFC_DOMAIN_CPU             = 0,
    DDK_DVFC_DOMAIN_ENUM_END        = 1
} DDK_DVFC_DOMAIN;

typedef struct
{
    UINT32 mV;
    UINT32 freq[DDK_DVFC_FREQ_ENUM_END];
} DDK_DVFC_SETPOINT_INFO, *PDDK_DVFC_SETPOINT_INFO;

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
    // Shared setpoint info
    BOOL bDvfcActive;
    BOOL bSetpointPending;
    DDK_DVFC_SETPOINT setpointCur[DDK_DVFC_DOMAIN_ENUM_END];
    DDK_DVFC_SETPOINT setpointMin[DDK_DVFC_DOMAIN_ENUM_END];
    DDK_DVFC_SETPOINT setpointMax[DDK_DVFC_DOMAIN_ENUM_END];
    DDK_DVFC_SETPOINT setpointLoad[DDK_DVFC_DOMAIN_ENUM_END];
    UINT32 setpointReqCount[DDK_DVFC_DOMAIN_ENUM_END][DDK_DVFC_SETPOINT_ENUM_END];
}  DDK_CLK_CONFIG, *PDDK_CLK_CONFIG;


//------------------------------------------------------------------------------
// Functions
BOOL DDKIomuxSetPinMux(DDK_IOMUX_PIN pin, DDK_IOMUX_PIN_MUXMODE muxmode, DDK_IOMUX_PIN_SION sion);
BOOL DDKIomuxGetPinMux(DDK_IOMUX_PIN pin, DDK_IOMUX_PIN_MUXMODE *pMuxmode, DDK_IOMUX_PIN_SION *pSion);
BOOL DDKIomuxSetPadConfig(DDK_IOMUX_PAD pad, DDK_IOMUX_PAD_SLEW slew, DDK_IOMUX_PAD_DRIVE drive, DDK_IOMUX_PAD_OPENDRAIN opendrain, DDK_IOMUX_PAD_PULL pull, DDK_IOMUX_PAD_HYSTERESIS hysteresis, DDK_IOMUX_PAD_VOLTAGE voltage);
BOOL DDKIomuxGetPadConfig(DDK_IOMUX_PAD pad, DDK_IOMUX_PAD_SLEW *pSlew, DDK_IOMUX_PAD_DRIVE *pDrive, DDK_IOMUX_PAD_OPENDRAIN *pOpendrain, DDK_IOMUX_PAD_PULL *pPull, DDK_IOMUX_PAD_HYSTERESIS *pHysteresis, DDK_IOMUX_PAD_VOLTAGE *pVoltage);
BOOL DDKIomuxSelectInput(DDK_IOMUX_SELEIN port, UINT32 daisy);

BOOL DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX index, DDK_CLOCK_GATE_MODE mode);
BOOL DDKClockGetGatingMode(DDK_CLOCK_GATE_INDEX index, DDK_CLOCK_GATE_MODE *pMode);

BOOL DDKClockGetFreq(DDK_CLOCK_SIGNAL sig, UINT32 *freq);
BOOL DDKClockConfigBaud(DDK_CLOCK_SIGNAL sig, DDK_CLOCK_BAUD_SOURCE src, UINT32 divider);
BOOL DDKClockSetCKO(BOOL bEnable, DDK_CLOCK_CKO_SRC src, UINT32 divider);
BOOL DDKClockSetpointRequest(DDK_DVFC_SETPOINT setpoint, BOOL bBlock);
BOOL DDKClockSetpointRelease(DDK_DVFC_SETPOINT setpoint);

#endif // __MX25_DDK_H
