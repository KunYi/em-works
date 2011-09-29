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
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  mx35_ddk.h
//
//  Contains MX35 definitions to assist with driver development.
//
//------------------------------------------------------------------------------
#ifndef __MX35_DDK_H
#define __MX35_DDK_H

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
    DDK_CLOCK_SIGNAL_MCUPLL             = 0,
    DDK_CLOCK_SIGNAL_PERPLL             = 1,
    DDK_CLOCK_SIGNAL_ARM                = 2,
    DDK_CLOCK_SIGNAL_AHB                = 3,
    DDK_CLOCK_SIGNAL_IPU                = 4,
    DDK_CLOCK_SIGNAL_MLB                = 5,
    DDK_CLOCK_SIGNAL_IPG                = 6,
    DDK_CLOCK_SIGNAL_PER                = 7,
    DDK_CLOCK_SIGNAL_SSI1               = 8,
    DDK_CLOCK_SIGNAL_SSI2               = 9,
    DDK_CLOCK_SIGNAL_CSI                = 10,
    DDK_CLOCK_SIGNAL_ESDHC1             = 11,
    DDK_CLOCK_SIGNAL_ESDHC2             = 12,
    DDK_CLOCK_SIGNAL_ESDHC3             = 13,
    DDK_CLOCK_SIGNAL_SPDIF              = 14,
    DDK_CLOCK_SIGNAL_USB                = 15,
    DDK_CLOCK_SIGNAL_UART               = 16,
    DDK_CLOCK_SIGNAL_NFC                = 17,
    DDK_CLOCK_SIGNAL_ENUM_END           = 18
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
    DDK_CLOCK_GATE_INDEX_ASRC           = 0,
    DDK_CLOCK_GATE_INDEX_ATA            = 1,
    DDK_CLOCK_GATE_INDEX_CAN1           = 3,
    DDK_CLOCK_GATE_INDEX_CAN2           = 4,
    DDK_CLOCK_GATE_INDEX_CSPI1          = 5,
    DDK_CLOCK_GATE_INDEX_CSPI2          = 6,
    DDK_CLOCK_GATE_INDEX_ECT            = 7,
    DDK_CLOCK_GATE_INDEX_EMI            = 9,
    DDK_CLOCK_GATE_INDEX_EPIT1          = 10,
    DDK_CLOCK_GATE_INDEX_EPIT2          = 11,
    DDK_CLOCK_GATE_INDEX_ESAI           = 12,
    DDK_CLOCK_GATE_INDEX_ESDHC1         = 13,
    DDK_CLOCK_GATE_INDEX_ESDHC2         = 14,
    DDK_CLOCK_GATE_INDEX_ESDHC3         = 15,
    DDK_CLOCK_GATE_INDEX_FEC            = 16,
    DDK_CLOCK_GATE_INDEX_GPIO1          = 17,
    DDK_CLOCK_GATE_INDEX_GPIO2          = 18,
    DDK_CLOCK_GATE_INDEX_GPIO3          = 19,
    DDK_CLOCK_GATE_INDEX_GPT            = 20,
    DDK_CLOCK_GATE_INDEX_I2C1           = 21,
    DDK_CLOCK_GATE_INDEX_I2C2           = 22,
    DDK_CLOCK_GATE_INDEX_I2C3           = 23,
    DDK_CLOCK_GATE_INDEX_IOMUXC         = 24,
    DDK_CLOCK_GATE_INDEX_IPU            = 25,
    DDK_CLOCK_GATE_INDEX_KPP            = 26,
    DDK_CLOCK_GATE_INDEX_MLB            = 27,
    DDK_CLOCK_GATE_INDEX_MSHC           = 28,
    DDK_CLOCK_GATE_INDEX_OWIRE          = 29,
    DDK_CLOCK_GATE_INDEX_PWM            = 30,
    DDK_CLOCK_GATE_INDEX_RNGC           = 31,
    DDK_CLOCK_GATE_INDEX_RTC            = 32,
    DDK_CLOCK_GATE_INDEX_RTIC           = 33,
    DDK_CLOCK_GATE_INDEX_SCC            = 34,
    DDK_CLOCK_GATE_INDEX_SDMA           = 35,
    DDK_CLOCK_GATE_INDEX_SPBA           = 36,
    DDK_CLOCK_GATE_INDEX_SPDIF          = 37,
    DDK_CLOCK_GATE_INDEX_SSI1           = 38,
    DDK_CLOCK_GATE_INDEX_SSI2           = 39,
    DDK_CLOCK_GATE_INDEX_UART1          = 40,
    DDK_CLOCK_GATE_INDEX_UART2          = 41,
    DDK_CLOCK_GATE_INDEX_UART3          = 42,
    DDK_CLOCK_GATE_INDEX_USBOTG         = 43,
    DDK_CLOCK_GATE_INDEX_WDOG           = 44,
    DDK_CLOCK_GATE_INDEX_MAX            = 45,
    DDK_CLOCK_GATE_INDEX_AUDMUX         = 47,
    DDK_CLOCK_GATE_INDEX_CSI            = 48,
    DDK_CLOCK_GATE_INDEX_IIM            = 49,
    DDK_CLOCK_GATE_INDEX_GPU2D          = 50,
    DDK_CLOCK_GATE_INDEX_NFC            = 63    // "Fake" for EMI gating purpose
} DDK_CLOCK_GATE_INDEX;


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
    DDK_CLOCK_GATE_MODE_ENABLED_RUN_WAIT    = 2,
    DDK_CLOCK_GATE_MODE_ENABLED_ALL         = 3
} DDK_CLOCK_GATE_MODE;


//-----------------------------------------------------------------------------
//
//  Type: DDK_CLOCK_BAUD_SOURCE
//
//  Input source for baud clock generation.
//
//-----------------------------------------------------------------------------
typedef enum {
    DDK_CLOCK_BAUD_SOURCE_PERPLL            = 0,
    DDK_CLOCK_BAUD_SOURCE_MCUPLL            = 1
} DDK_CLOCK_BAUD_SOURCE;


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


//-----------------------------------------------------------------------------
//
//  Type: DDK_CLOCK_CKO_SRC
//
//  Clock output source (CKO) signal selections.
//
//-----------------------------------------------------------------------------
typedef enum {
    DDK_CLOCK_CKO_SRC_CKIL                  = 0,
    DDK_CLOCK_CKO_SRC_CKIH                  = 1,
    DDK_CLOCK_CKO_SRC_OSCAUDIO              = 2,
    DDK_CLOCK_CKO_SRC_MPLL2X                = 3,
    DDK_CLOCK_CKO_SRC_MPLLP75               = 4,
    DDK_CLOCK_CKO_SRC_MPLL                  = 5,
    DDK_CLOCK_CKO_SRC_PPLL                  = 6,
    DDK_CLOCK_CKO_SRC_ARM                   = 7,
    DDK_CLOCK_CKO_SRC_AHB                   = 8,
    DDK_CLOCK_CKO_SRC_IPG                   = 9,
    DDK_CLOCK_CKO_SRC_PER                   = 10,
    DDK_CLOCK_CKO_SRC_USB                   = 11,
    DDK_CLOCK_CKO_SRC_ESDHC1                = 12,
    DDK_CLOCK_CKO_SRC_SSI1                  = 13,
    DDK_CLOCK_CKO_SRC_MLB                   = 14,
    DDK_CLOCK_CKO_SRC_MSHC                  = 15,
    DDK_CLOCK_CKO_SRC_MPLLLRF               = 16,
    DDK_CLOCK_CKO_SRC_CSI                   = 17,
    DDK_CLOCK_CKO_SRC_SPDIF                 = 18,
    DDK_CLOCK_CKO_SRC_UART1                 = 19,
    DDK_CLOCK_CKO_SRC_ASRC                  = 20,
    DDK_CLOCK_CKO_SRC_DPTC                  = 21
} DDK_CLOCK_CKO_SRC;


//-----------------------------------------------------------------------------
//
//  Type: DDK_IOMUX_PIN
//
//  Specifies the functional pin name used to configure the IOMUX SW_MUX_CTL.
//
//-----------------------------------------------------------------------------
typedef enum {
    DDK_IOMUX_PIN_CAPTURE               = (0),
    DDK_IOMUX_PIN_COMPARE               = (1),
    DDK_IOMUX_PIN_WDOG_RST              = (2),
    DDK_IOMUX_PIN_GPIO1_0               = (3),
    DDK_IOMUX_PIN_GPIO1_1               = (4),
    DDK_IOMUX_PIN_GPIO2_0               = (5),
    DDK_IOMUX_PIN_GPIO3_0               = (6),
    DDK_IOMUX_PIN_CLKO                  = (7),
    DDK_IOMUX_PIN_VSTBY                 = (8),
    DDK_IOMUX_PIN_A0                    = (9),
    DDK_IOMUX_PIN_A1                    = (10),
    DDK_IOMUX_PIN_A2                    = (11),
    DDK_IOMUX_PIN_A3                    = (12),
    DDK_IOMUX_PIN_A4                    = (13),
    DDK_IOMUX_PIN_A5                    = (14),
    DDK_IOMUX_PIN_A6                    = (15),
    DDK_IOMUX_PIN_A7                    = (16),
    DDK_IOMUX_PIN_A8                    = (17),
    DDK_IOMUX_PIN_A9                    = (18),
    DDK_IOMUX_PIN_A10                   = (19),
    DDK_IOMUX_PIN_MA10                  = (20),
    DDK_IOMUX_PIN_A11                   = (21),
    DDK_IOMUX_PIN_A12                   = (22),
    DDK_IOMUX_PIN_A13                   = (23),
    DDK_IOMUX_PIN_A14                   = (24),
    DDK_IOMUX_PIN_A15                   = (25),
    DDK_IOMUX_PIN_A16                   = (26),
    DDK_IOMUX_PIN_A17                   = (27),
    DDK_IOMUX_PIN_A18                   = (28),
    DDK_IOMUX_PIN_A19                   = (29),
    DDK_IOMUX_PIN_A20                   = (30),
    DDK_IOMUX_PIN_A21                   = (31),
    DDK_IOMUX_PIN_A22                   = (32),
    DDK_IOMUX_PIN_A23                   = (33),
    DDK_IOMUX_PIN_A24                   = (34),
    DDK_IOMUX_PIN_A25                   = (35),
    DDK_IOMUX_PIN_EB0                   = (36),
    DDK_IOMUX_PIN_EB1                   = (37),
    DDK_IOMUX_PIN_OE                    = (38),
    DDK_IOMUX_PIN_CS0                   = (39),
    DDK_IOMUX_PIN_CS1                   = (40),
    DDK_IOMUX_PIN_CS2                   = (41),
    DDK_IOMUX_PIN_CS3                   = (42),
    DDK_IOMUX_PIN_CS4                   = (43),
    DDK_IOMUX_PIN_CS5                   = (44),
    DDK_IOMUX_PIN_NF_CE0                = (45),
    DDK_IOMUX_PIN_LBA                   = (46),
    DDK_IOMUX_PIN_BCLK                  = (47),
    DDK_IOMUX_PIN_RW                    = (48),
    DDK_IOMUX_PIN_NFWE_B                = (49),
    DDK_IOMUX_PIN_NFRE_B                = (50),
    DDK_IOMUX_PIN_NFALE                 = (51),
    DDK_IOMUX_PIN_NFCLE                 = (52),
    DDK_IOMUX_PIN_NFWP_B                = (53),
    DDK_IOMUX_PIN_NFRB                  = (54),
    DDK_IOMUX_PIN_CSI_D8                = (55),
    DDK_IOMUX_PIN_CSI_D9                = (56),
    DDK_IOMUX_PIN_CSI_D10               = (57),
    DDK_IOMUX_PIN_CSI_D11               = (58),
    DDK_IOMUX_PIN_CSI_D12               = (59),
    DDK_IOMUX_PIN_CSI_D13               = (60),
    DDK_IOMUX_PIN_CSI_D14               = (61),
    DDK_IOMUX_PIN_CSI_D15               = (62),
    DDK_IOMUX_PIN_CSI_MCLK              = (63),
    DDK_IOMUX_PIN_CSI_VSYNC             = (64),
    DDK_IOMUX_PIN_CSI_HSYNC             = (65),
    DDK_IOMUX_PIN_CSI_PIXCLK            = (66),
    DDK_IOMUX_PIN_I2C1_CLK              = (67),
    DDK_IOMUX_PIN_I2C1_DAT              = (68),
    DDK_IOMUX_PIN_I2C2_CLK              = (69),
    DDK_IOMUX_PIN_I2C2_DAT              = (70),
    DDK_IOMUX_PIN_STXD4                 = (71),
    DDK_IOMUX_PIN_SRXD4                 = (72),
    DDK_IOMUX_PIN_SCK4                  = (73),
    DDK_IOMUX_PIN_STXFS4                = (74),
    DDK_IOMUX_PIN_STXD5                 = (75),
    DDK_IOMUX_PIN_SRXD5                 = (76),
    DDK_IOMUX_PIN_SCK5                  = (77),
    DDK_IOMUX_PIN_STXFS5                = (78),
    DDK_IOMUX_PIN_SCKR                  = (79),
    DDK_IOMUX_PIN_FSR                   = (80),
    DDK_IOMUX_PIN_HCKR                  = (81),
    DDK_IOMUX_PIN_SCKT                  = (82),
    DDK_IOMUX_PIN_FST                   = (83),
    DDK_IOMUX_PIN_HCKT                  = (84),
    DDK_IOMUX_PIN_TX5_RX0               = (85),
    DDK_IOMUX_PIN_TX4_RX1               = (86),
    DDK_IOMUX_PIN_TX3_RX2               = (87),
    DDK_IOMUX_PIN_TX2_RX3               = (88),
    DDK_IOMUX_PIN_TX1                   = (89),
    DDK_IOMUX_PIN_TX0                   = (90),
    DDK_IOMUX_PIN_CSPI1_MOSI            = (91),
    DDK_IOMUX_PIN_CSPI1_MISO            = (92),
    DDK_IOMUX_PIN_CSPI1_SS0             = (93),
    DDK_IOMUX_PIN_CSPI1_SS1             = (94),
    DDK_IOMUX_PIN_CSPI1_SCLK            = (95),
    DDK_IOMUX_PIN_CSPI1_SPI_RDY         = (96),
    DDK_IOMUX_PIN_RXD1                  = (97),
    DDK_IOMUX_PIN_TXD1                  = (98),
    DDK_IOMUX_PIN_RTS1                  = (99),
    DDK_IOMUX_PIN_CTS1                  = (100),
    DDK_IOMUX_PIN_RXD2                  = (101),
    DDK_IOMUX_PIN_TXD2                  = (102),
    DDK_IOMUX_PIN_RTS2                  = (103),
    DDK_IOMUX_PIN_CTS2                  = (104),
    DDK_IOMUX_PIN_USBOTG_PWR            = (105),
    DDK_IOMUX_PIN_USBOTG_OC             = (106),
    DDK_IOMUX_PIN_LD0                   = (107),
    DDK_IOMUX_PIN_LD1                   = (108),
    DDK_IOMUX_PIN_LD2                   = (109),
    DDK_IOMUX_PIN_LD3                   = (110),
    DDK_IOMUX_PIN_LD4                   = (111),
    DDK_IOMUX_PIN_LD5                   = (112),
    DDK_IOMUX_PIN_LD6                   = (113),
    DDK_IOMUX_PIN_LD7                   = (114),
    DDK_IOMUX_PIN_LD8                   = (115),
    DDK_IOMUX_PIN_LD9                   = (116),
    DDK_IOMUX_PIN_LD10                  = (117),
    DDK_IOMUX_PIN_LD11                  = (118),
    DDK_IOMUX_PIN_LD12                  = (119),
    DDK_IOMUX_PIN_LD13                  = (120),
    DDK_IOMUX_PIN_LD14                  = (121),
    DDK_IOMUX_PIN_LD15                  = (122),
    DDK_IOMUX_PIN_LD16                  = (123),
    DDK_IOMUX_PIN_LD17                  = (124),
    DDK_IOMUX_PIN_LD18                  = (125),
    DDK_IOMUX_PIN_LD19                  = (126),
    DDK_IOMUX_PIN_LD20                  = (127),
    DDK_IOMUX_PIN_LD21                  = (128),
    DDK_IOMUX_PIN_LD22                  = (129),
    DDK_IOMUX_PIN_LD23                  = (130),
    DDK_IOMUX_PIN_D3_HSYNC              = (131),
    DDK_IOMUX_PIN_D3_FPSHIFT            = (132),
    DDK_IOMUX_PIN_D3_DRDY               = (133),
    DDK_IOMUX_PIN_CONTRAST              = (134),
    DDK_IOMUX_PIN_D3_VSYNC              = (135),
    DDK_IOMUX_PIN_D3_REV                = (136),
    DDK_IOMUX_PIN_D3_CLS                = (137),
    DDK_IOMUX_PIN_D3_SPL                = (138),
    DDK_IOMUX_PIN_SD1_CMD               = (139),
    DDK_IOMUX_PIN_SD1_CLK               = (140),
    DDK_IOMUX_PIN_SD1_DATA0             = (141),
    DDK_IOMUX_PIN_SD1_DATA1             = (142),
    DDK_IOMUX_PIN_SD1_DATA2             = (143),
    DDK_IOMUX_PIN_SD1_DATA3             = (144),
    DDK_IOMUX_PIN_SD2_CMD               = (145),
    DDK_IOMUX_PIN_SD2_CLK               = (146),
    DDK_IOMUX_PIN_SD2_DATA0             = (147),
    DDK_IOMUX_PIN_SD2_DATA1             = (148),
    DDK_IOMUX_PIN_SD2_DATA2             = (149),
    DDK_IOMUX_PIN_SD2_DATA3             = (150),
    DDK_IOMUX_PIN_ATA_CS0               = (151),
    DDK_IOMUX_PIN_ATA_CS1               = (152),
    DDK_IOMUX_PIN_ATA_DIOR              = (153),
    DDK_IOMUX_PIN_ATA_DIOW              = (154),
    DDK_IOMUX_PIN_ATA_DMACK             = (155),
    DDK_IOMUX_PIN_ATA_RESET_B           = (156),
    DDK_IOMUX_PIN_ATA_IORDY             = (157),
    DDK_IOMUX_PIN_ATA_DATA0             = (158),
    DDK_IOMUX_PIN_ATA_DATA1             = (159),
    DDK_IOMUX_PIN_ATA_DATA2             = (160),
    DDK_IOMUX_PIN_ATA_DATA3             = (161),
    DDK_IOMUX_PIN_ATA_DATA4             = (162),
    DDK_IOMUX_PIN_ATA_DATA5             = (163),
    DDK_IOMUX_PIN_ATA_DATA6             = (164),
    DDK_IOMUX_PIN_ATA_DATA7             = (165),
    DDK_IOMUX_PIN_ATA_DATA8             = (166),
    DDK_IOMUX_PIN_ATA_DATA9             = (167),
    DDK_IOMUX_PIN_ATA_DATA10            = (168),
    DDK_IOMUX_PIN_ATA_DATA11            = (169),
    DDK_IOMUX_PIN_ATA_DATA12            = (170),
    DDK_IOMUX_PIN_ATA_DATA13            = (171),
    DDK_IOMUX_PIN_ATA_DATA14            = (172),
    DDK_IOMUX_PIN_ATA_DATA15            = (173),
    DDK_IOMUX_PIN_ATA_INTRQ             = (174),
    DDK_IOMUX_PIN_ATA_BUFF_EN           = (175),
    DDK_IOMUX_PIN_ATA_DMARQ             = (176),
    DDK_IOMUX_PIN_ATA_DA0               = (177),
    DDK_IOMUX_PIN_ATA_DA1               = (178),
    DDK_IOMUX_PIN_ATA_DA2               = (179),
    DDK_IOMUX_PIN_MLB_CLK               = (180),
    DDK_IOMUX_PIN_MLB_DAT               = (181),
    DDK_IOMUX_PIN_MLB_SIG               = (182),
    DDK_IOMUX_PIN_FEC_TX_CLK            = (183),
    DDK_IOMUX_PIN_FEC_RX_CLK            = (184),
    DDK_IOMUX_PIN_FEC_RX_DV             = (185),
    DDK_IOMUX_PIN_FEC_COL               = (186),
    DDK_IOMUX_PIN_FEC_RDATA0            = (187),
    DDK_IOMUX_PIN_FEC_TDATA0            = (188),
    DDK_IOMUX_PIN_FEC_TX_EN             = (189),
    DDK_IOMUX_PIN_FEC_MDC               = (190),
    DDK_IOMUX_PIN_FEC_MDIO              = (191),
    DDK_IOMUX_PIN_FEC_TX_ERR            = (192),
    DDK_IOMUX_PIN_FEC_RX_ERR            = (193),
    DDK_IOMUX_PIN_FEC_CRS               = (194),
    DDK_IOMUX_PIN_FEC_RDATA1            = (195),
    DDK_IOMUX_PIN_FEC_TDATA1            = (196),
    DDK_IOMUX_PIN_FEC_RDATA2            = (197),
    DDK_IOMUX_PIN_FEC_TDATA2            = (198),
    DDK_IOMUX_PIN_FEC_RDATA3            = (199),
    DDK_IOMUX_PIN_FEC_TDATA3            = (200)
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
    DDK_IOMUX_PAD_CAPTURE               = (0),
    DDK_IOMUX_PAD_COMPARE               = (1),
    DDK_IOMUX_PAD_WDOG_RST              = (2),
    DDK_IOMUX_PAD_GPIO1_0               = (3),
    DDK_IOMUX_PAD_GPIO1_1               = (4),
    DDK_IOMUX_PAD_GPIO2_0               = (5),
    DDK_IOMUX_PAD_GPIO3_0               = (6),
    DDK_IOMUX_PAD_RESET_IN_B            = (7),
    DDK_IOMUX_PAD_POR_B                 = (8),
    DDK_IOMUX_PAD_CLKO                  = (9),
    DDK_IOMUX_PAD_BOOT_MODE0            = (10),
    DDK_IOMUX_PAD_BOOT_MODE1            = (11),
    DDK_IOMUX_PAD_CLK_MODE0             = (12),
    DDK_IOMUX_PAD_CLK_MODE1             = (13),
    DDK_IOMUX_PAD_POWER_FAIL            = (14),
    DDK_IOMUX_PAD_VSTBY                 = (15),
    DDK_IOMUX_PAD_A0                    = (16),
    DDK_IOMUX_PAD_A1                    = (17),
    DDK_IOMUX_PAD_A2                    = (18),
    DDK_IOMUX_PAD_A3                    = (19),
    DDK_IOMUX_PAD_A4                    = (20),
    DDK_IOMUX_PAD_A5                    = (21),
    DDK_IOMUX_PAD_A6                    = (22),
    DDK_IOMUX_PAD_A7                    = (23),
    DDK_IOMUX_PAD_A8                    = (24),
    DDK_IOMUX_PAD_A9                    = (25),
    DDK_IOMUX_PAD_A10                   = (26),
    DDK_IOMUX_PAD_MA10                  = (27),
    DDK_IOMUX_PAD_A11                   = (28),
    DDK_IOMUX_PAD_A12                   = (29),
    DDK_IOMUX_PAD_A13                   = (30),
    DDK_IOMUX_PAD_A14                   = (31),
    DDK_IOMUX_PAD_A15                   = (32),
    DDK_IOMUX_PAD_A16                   = (33),
    DDK_IOMUX_PAD_A17                   = (34),
    DDK_IOMUX_PAD_A18                   = (35),
    DDK_IOMUX_PAD_A19                   = (36),
    DDK_IOMUX_PAD_A20                   = (37),
    DDK_IOMUX_PAD_A21                   = (38),
    DDK_IOMUX_PAD_A22                   = (39),
    DDK_IOMUX_PAD_A23                   = (40),
    DDK_IOMUX_PAD_A24                   = (41),
    DDK_IOMUX_PAD_A25                   = (42),
    DDK_IOMUX_PAD_SDBA1                 = (43),
    DDK_IOMUX_PAD_SDBA0                 = (44),
    DDK_IOMUX_PAD_SD0                   = (45),
    DDK_IOMUX_PAD_SD1                   = (46),
    DDK_IOMUX_PAD_SD2                   = (47),
    DDK_IOMUX_PAD_SD3                   = (48),
    DDK_IOMUX_PAD_SD4                   = (49),
    DDK_IOMUX_PAD_SD5                   = (50),
    DDK_IOMUX_PAD_SD6                   = (51),
    DDK_IOMUX_PAD_SD7                   = (52),
    DDK_IOMUX_PAD_SD8                   = (53),
    DDK_IOMUX_PAD_SD9                   = (54),
    DDK_IOMUX_PAD_SD10                  = (55),
    DDK_IOMUX_PAD_SD11                  = (56),
    DDK_IOMUX_PAD_SD12                  = (57),
    DDK_IOMUX_PAD_SD13                  = (58),
    DDK_IOMUX_PAD_SD14                  = (59),
    DDK_IOMUX_PAD_SD15                  = (60),
    DDK_IOMUX_PAD_SD16                  = (61),
    DDK_IOMUX_PAD_SD17                  = (62),
    DDK_IOMUX_PAD_SD18                  = (63),
    DDK_IOMUX_PAD_SD19                  = (64),
    DDK_IOMUX_PAD_SD20                  = (65),
    DDK_IOMUX_PAD_SD21                  = (66),
    DDK_IOMUX_PAD_SD22                  = (67),
    DDK_IOMUX_PAD_SD23                  = (68),
    DDK_IOMUX_PAD_SD24                  = (69),
    DDK_IOMUX_PAD_SD25                  = (70),
    DDK_IOMUX_PAD_SD26                  = (71),
    DDK_IOMUX_PAD_SD27                  = (72),
    DDK_IOMUX_PAD_SD28                  = (73),
    DDK_IOMUX_PAD_SD29                  = (74),
    DDK_IOMUX_PAD_SD30                  = (75),
    DDK_IOMUX_PAD_SD31                  = (76),
    DDK_IOMUX_PAD_DQM0                  = (77),
    DDK_IOMUX_PAD_DQM1                  = (78),
    DDK_IOMUX_PAD_DQM2                  = (79),
    DDK_IOMUX_PAD_DQM3                  = (80),
    DDK_IOMUX_PAD_EB0                   = (81),
    DDK_IOMUX_PAD_EB1                   = (82),
    DDK_IOMUX_PAD_OE                    = (83),
    DDK_IOMUX_PAD_CS0                   = (84),
    DDK_IOMUX_PAD_CS1                   = (85),
    DDK_IOMUX_PAD_CS2                   = (86),
    DDK_IOMUX_PAD_CS3                   = (87),
    DDK_IOMUX_PAD_CS4                   = (88),
    DDK_IOMUX_PAD_CS5                   = (89),
    DDK_IOMUX_PAD_NF_CE0                = (90),
    DDK_IOMUX_PAD_ECB                   = (91),
    DDK_IOMUX_PAD_LBA                   = (92),
    DDK_IOMUX_PAD_BCLK                  = (93),
    DDK_IOMUX_PAD_RW                    = (94),
    DDK_IOMUX_PAD_RAS                   = (95),
    DDK_IOMUX_PAD_CAS                   = (96),
    DDK_IOMUX_PAD_SDWE                  = (97),
    DDK_IOMUX_PAD_SDCKE0                = (98),
    DDK_IOMUX_PAD_SDCKE1                = (99),
    DDK_IOMUX_PAD_SDCLK                 = (100),
    DDK_IOMUX_PAD_SDQS0                 = (101),
    DDK_IOMUX_PAD_SDQS1                 = (102),
    DDK_IOMUX_PAD_SDQS2                 = (103),
    DDK_IOMUX_PAD_SDQS3                 = (104),
    DDK_IOMUX_PAD_NFWE_B                = (105),
    DDK_IOMUX_PAD_NFRE_B                = (106),
    DDK_IOMUX_PAD_NFALE                 = (107),
    DDK_IOMUX_PAD_NFCLE                 = (108),
    DDK_IOMUX_PAD_NFWP_B                = (109),
    DDK_IOMUX_PAD_NFRB                  = (110),
    DDK_IOMUX_PAD_D15                   = (111),
    DDK_IOMUX_PAD_D14                   = (112),
    DDK_IOMUX_PAD_D13                   = (113),
    DDK_IOMUX_PAD_D12                   = (114),
    DDK_IOMUX_PAD_D11                   = (115),
    DDK_IOMUX_PAD_D10                   = (116),
    DDK_IOMUX_PAD_D9                    = (117),
    DDK_IOMUX_PAD_D8                    = (118),
    DDK_IOMUX_PAD_D7                    = (119),
    DDK_IOMUX_PAD_D6                    = (120),
    DDK_IOMUX_PAD_D5                    = (121),
    DDK_IOMUX_PAD_D4                    = (122),
    DDK_IOMUX_PAD_D3                    = (123),
    DDK_IOMUX_PAD_D2                    = (124),
    DDK_IOMUX_PAD_D1                    = (125),
    DDK_IOMUX_PAD_D0                    = (126),
    DDK_IOMUX_PAD_CSI_D8                = (127),
    DDK_IOMUX_PAD_CSI_D9                = (128),
    DDK_IOMUX_PAD_CSI_D10               = (129),
    DDK_IOMUX_PAD_CSI_D11               = (130),
    DDK_IOMUX_PAD_CSI_D12               = (131),
    DDK_IOMUX_PAD_CSI_D13               = (132),
    DDK_IOMUX_PAD_CSI_D14               = (133),
    DDK_IOMUX_PAD_CSI_D15               = (134),
    DDK_IOMUX_PAD_CSI_MCLK              = (135),
    DDK_IOMUX_PAD_CSI_VSYNC             = (136),
    DDK_IOMUX_PAD_CSI_HSYNC             = (137),
    DDK_IOMUX_PAD_CSI_PIXCLK            = (138),
    DDK_IOMUX_PAD_I2C1_CLK              = (139),
    DDK_IOMUX_PAD_I2C1_DAT              = (140),
    DDK_IOMUX_PAD_I2C2_CLK              = (141),
    DDK_IOMUX_PAD_I2C2_DAT              = (142),
    DDK_IOMUX_PAD_STXD4                 = (143),
    DDK_IOMUX_PAD_SRXD4                 = (144),
    DDK_IOMUX_PAD_SCK4                  = (145),
    DDK_IOMUX_PAD_STXFS4                = (146),
    DDK_IOMUX_PAD_STXD5                 = (147),
    DDK_IOMUX_PAD_SRXD5                 = (148),
    DDK_IOMUX_PAD_SCK5                  = (149),
    DDK_IOMUX_PAD_STXFS5                = (150),
    DDK_IOMUX_PAD_SCKR                  = (151),
    DDK_IOMUX_PAD_FSR                   = (152),
    DDK_IOMUX_PAD_HCKR                  = (153),
    DDK_IOMUX_PAD_SCKT                  = (154),
    DDK_IOMUX_PAD_FST                   = (155),
    DDK_IOMUX_PAD_HCKT                  = (156),
    DDK_IOMUX_PAD_TX5_RX0               = (157),
    DDK_IOMUX_PAD_TX4_RX1               = (158),
    DDK_IOMUX_PAD_TX3_RX2               = (159),
    DDK_IOMUX_PAD_TX2_RX3               = (160),
    DDK_IOMUX_PAD_TX1                   = (161),
    DDK_IOMUX_PAD_TX0                   = (162),
    DDK_IOMUX_PAD_CSPI1_MOSI            = (163),
    DDK_IOMUX_PAD_CSPI1_MISO            = (164),
    DDK_IOMUX_PAD_CSPI1_SS0             = (165),
    DDK_IOMUX_PAD_CSPI1_SS1             = (166),
    DDK_IOMUX_PAD_CSPI1_SCLK            = (167),
    DDK_IOMUX_PAD_CSPI1_SPI_RDY         = (168),
    DDK_IOMUX_PAD_RXD1                  = (169),
    DDK_IOMUX_PAD_TXD1                  = (170),
    DDK_IOMUX_PAD_RTS1                  = (171),
    DDK_IOMUX_PAD_CTS1                  = (172),
    DDK_IOMUX_PAD_RXD2                  = (173),
    DDK_IOMUX_PAD_TXD2                  = (174),
    DDK_IOMUX_PAD_RTS2                  = (175),
    DDK_IOMUX_PAD_CTS2                  = (176),
    DDK_IOMUX_PAD_RTCK                  = (177),
    DDK_IOMUX_PAD_TCK                   = (178),
    DDK_IOMUX_PAD_TMS                   = (179),
    DDK_IOMUX_PAD_TDI                   = (180),
    DDK_IOMUX_PAD_TDO                   = (181),
    DDK_IOMUX_PAD_TRSTB                 = (182),
    DDK_IOMUX_PAD_DE_B                  = (183),
    DDK_IOMUX_PAD_SJC_MOD               = (184),
    DDK_IOMUX_PAD_USBOTG_PWR            = (185),
    DDK_IOMUX_PAD_USBOTG_OC             = (186),
    DDK_IOMUX_PAD_LD0                   = (187),
    DDK_IOMUX_PAD_LD1                   = (188),
    DDK_IOMUX_PAD_LD2                   = (189),
    DDK_IOMUX_PAD_LD3                   = (190),
    DDK_IOMUX_PAD_LD4                   = (191),
    DDK_IOMUX_PAD_LD5                   = (192),
    DDK_IOMUX_PAD_LD6                   = (193),
    DDK_IOMUX_PAD_LD7                   = (194),
    DDK_IOMUX_PAD_LD8                   = (195),
    DDK_IOMUX_PAD_LD9                   = (196),
    DDK_IOMUX_PAD_LD10                  = (197),
    DDK_IOMUX_PAD_LD11                  = (198),
    DDK_IOMUX_PAD_LD12                  = (199),
    DDK_IOMUX_PAD_LD13                  = (200),
    DDK_IOMUX_PAD_LD14                  = (201),
    DDK_IOMUX_PAD_LD15                  = (202),
    DDK_IOMUX_PAD_LD16                  = (203),
    DDK_IOMUX_PAD_LD17                  = (204),
    DDK_IOMUX_PAD_LD18                  = (205),
    DDK_IOMUX_PAD_LD19                  = (206),
    DDK_IOMUX_PAD_LD20                  = (207),
    DDK_IOMUX_PAD_LD21                  = (208),
    DDK_IOMUX_PAD_LD22                  = (209),
    DDK_IOMUX_PAD_LD23                  = (210),
    DDK_IOMUX_PAD_D3_HSYNC              = (211),
    DDK_IOMUX_PAD_D3_FPSHIFT            = (212),
    DDK_IOMUX_PAD_D3_DRDY               = (213),
    DDK_IOMUX_PAD_CONTRAST              = (214),
    DDK_IOMUX_PAD_D3_VSYNC              = (215),
    DDK_IOMUX_PAD_D3_REV                = (216),
    DDK_IOMUX_PAD_D3_CLS                = (217),
    DDK_IOMUX_PAD_D3_SPL                = (218),
    DDK_IOMUX_PAD_SD1_CMD               = (219),
    DDK_IOMUX_PAD_SD1_CLK               = (220),
    DDK_IOMUX_PAD_SD1_DATA0             = (221),
    DDK_IOMUX_PAD_SD1_DATA1             = (222),
    DDK_IOMUX_PAD_SD1_DATA2             = (223),
    DDK_IOMUX_PAD_SD1_DATA3             = (224),
    DDK_IOMUX_PAD_SD2_CMD               = (225),
    DDK_IOMUX_PAD_SD2_CLK               = (226),
    DDK_IOMUX_PAD_SD2_DATA0             = (227),
    DDK_IOMUX_PAD_SD2_DATA1             = (228),
    DDK_IOMUX_PAD_SD2_DATA2             = (229),
    DDK_IOMUX_PAD_SD2_DATA3             = (230),
    DDK_IOMUX_PAD_ATA_CS0               = (231),
    DDK_IOMUX_PAD_ATA_CS1               = (232),
    DDK_IOMUX_PAD_ATA_DIOR              = (233),
    DDK_IOMUX_PAD_ATA_DIOW              = (234),
    DDK_IOMUX_PAD_ATA_DMACK             = (235),
    DDK_IOMUX_PAD_ATA_RESET_B           = (236),
    DDK_IOMUX_PAD_ATA_IORDY             = (237),
    DDK_IOMUX_PAD_ATA_DATA0             = (238),
    DDK_IOMUX_PAD_ATA_DATA1             = (239),
    DDK_IOMUX_PAD_ATA_DATA2             = (240),
    DDK_IOMUX_PAD_ATA_DATA3             = (241),
    DDK_IOMUX_PAD_ATA_DATA4             = (242),
    DDK_IOMUX_PAD_ATA_DATA5             = (243),
    DDK_IOMUX_PAD_ATA_DATA6             = (244),
    DDK_IOMUX_PAD_ATA_DATA7             = (245),
    DDK_IOMUX_PAD_ATA_DATA8             = (246),
    DDK_IOMUX_PAD_ATA_DATA9             = (247),
    DDK_IOMUX_PAD_ATA_DATA10            = (248),
    DDK_IOMUX_PAD_ATA_DATA11            = (249),
    DDK_IOMUX_PAD_ATA_DATA12            = (250),
    DDK_IOMUX_PAD_ATA_DATA13            = (251),
    DDK_IOMUX_PAD_ATA_DATA14            = (252),
    DDK_IOMUX_PAD_ATA_DATA15            = (253),
    DDK_IOMUX_PAD_ATA_INTRQ             = (254),
    DDK_IOMUX_PAD_ATA_BUFF_EN           = (255),
    DDK_IOMUX_PAD_ATA_DMARQ             = (256),
    DDK_IOMUX_PAD_ATA_DA0               = (257),
    DDK_IOMUX_PAD_ATA_DA1               = (258),
    DDK_IOMUX_PAD_ATA_DA2               = (259),
    DDK_IOMUX_PAD_MLB_CLK               = (260),
    DDK_IOMUX_PAD_MLB_DAT               = (261),
    DDK_IOMUX_PAD_MLB_SIG               = (262),
    DDK_IOMUX_PAD_FEC_TX_CLK            = (263),
    DDK_IOMUX_PAD_FEC_RX_CLK            = (264),
    DDK_IOMUX_PAD_FEC_RX_DV             = (265),
    DDK_IOMUX_PAD_FEC_COL               = (266),
    DDK_IOMUX_PAD_FEC_RDATA0            = (267),
    DDK_IOMUX_PAD_FEC_TDATA0            = (268),
    DDK_IOMUX_PAD_FEC_TX_EN             = (269),
    DDK_IOMUX_PAD_FEC_MDC               = (270),
    DDK_IOMUX_PAD_FEC_MDIO              = (271),
    DDK_IOMUX_PAD_FEC_TX_ERR            = (272),
    DDK_IOMUX_PAD_FEC_RX_ERR            = (273),
    DDK_IOMUX_PAD_FEC_CRS               = (274),
    DDK_IOMUX_PAD_FEC_RDATA1            = (275),
    DDK_IOMUX_PAD_FEC_TDATA1            = (276),
    DDK_IOMUX_PAD_FEC_RDATA2            = (277),
    DDK_IOMUX_PAD_FEC_TDATA2            = (278),
    DDK_IOMUX_PAD_FEC_RDATA3            = (279),
    DDK_IOMUX_PAD_FEC_TDATA3            = (280),
    DDK_IOMUX_PAD_FEC_ARMCLK            = (281),
    DDK_IOMUX_PAD_FEC_MODE              = (282)
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
    DDK_IOMUX_PAD_PULL_NONE         = (IOMUX_SW_PAD_CTL_PULL_KEEP_CTL_DISABLE << IOMUX_SW_PAD_CTL_PULL_KEEP_CTL_LSH),
    DDK_IOMUX_PAD_PULL_KEEPER       = (IOMUX_SW_PAD_CTL_PULL_KEEP_CTL_ENABLE << IOMUX_SW_PAD_CTL_PULL_KEEP_CTL_LSH),
    DDK_IOMUX_PAD_PULL_DOWN_100K    = (IOMUX_SW_PAD_CTL_PULL_KEEP_CTL_100K_DOWN << IOMUX_SW_PAD_CTL_PULL_KEEP_CTL_LSH),
    DDK_IOMUX_PAD_PULL_UP_47K       = (IOMUX_SW_PAD_CTL_PULL_KEEP_CTL_47K_UP << IOMUX_SW_PAD_CTL_PULL_KEEP_CTL_LSH),
    DDK_IOMUX_PAD_PULL_UP_100K      = (IOMUX_SW_PAD_CTL_PULL_KEEP_CTL_100K_UP << IOMUX_SW_PAD_CTL_PULL_KEEP_CTL_LSH),
    DDK_IOMUX_PAD_PULL_UP_22K       = (IOMUX_SW_PAD_CTL_PULL_KEEP_CTL_22K_UP << IOMUX_SW_PAD_CTL_PULL_KEEP_CTL_LSH)    
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
    DDK_IOMUX_SELEIN_AUDMUX_P5_INPUT_RXCLK_AMX      = (0),
    DDK_IOMUX_SELEIN_AUDMUX_P5_INPUT_RXFS_AMX       = (1),
    DDK_IOMUX_SELEIN_AUDMUX_P6_INPUT_DA_AMX         = (2),
    DDK_IOMUX_SELEIN_AUDMUX_P6_INPUT_DB_AMX         = (3),
    DDK_IOMUX_SELEIN_AUDMUX_P6_INPUT_RXCLK_AMX      = (4),
    DDK_IOMUX_SELEIN_AUDMUX_P6_INPUT_RXFS_AMX       = (5),
    DDK_IOMUX_SELEIN_AUDMUX_P6_INPUT_TXCLK_AMX      = (6),
    DDK_IOMUX_SELEIN_AUDMUX_P6_INPUT_TXFS_AMX       = (7),
    DDK_IOMUX_SELEIN_CAN1_IPP_IND_CANRX             = (8),
    DDK_IOMUX_SELEIN_CAN2_IPP_IND_CANRX             = (9),
    DDK_IOMUX_SELEIN_CCM_IPP_32K_MUXED_IN           = (10),
    DDK_IOMUX_SELEIN_CCM_IPP_PMIC_RDY               = (11),
    DDK_IOMUX_SELEIN_CSPI1_IPP_IND_SS2_B            = (12),
    DDK_IOMUX_SELEIN_CSPI1_IPP_IND_SS3_B            = (13),
    DDK_IOMUX_SELEIN_CSPI2_IPP_CSPI_CLK_IN          = (14),
    DDK_IOMUX_SELEIN_CSPI2_IPP_IND_DATAREADY_B      = (15),
    DDK_IOMUX_SELEIN_CSPI2_IPP_IND_MISO             = (16),
    DDK_IOMUX_SELEIN_CSPI2_IPP_IND_MOSI             = (17),
    DDK_IOMUX_SELEIN_CSPI2_IPP_IND_SS0_B            = (18),
    DDK_IOMUX_SELEIN_CSPI2_IPP_IND_SS1_B            = (19),
    DDK_IOMUX_SELEIN_CSPI2_IPP_IND_SS2_B            = (20),
    DDK_IOMUX_SELEIN_CSPI2_IPP_IND_SS3_B            = (21),
    DDK_IOMUX_SELEIN_EMI_IPP_IND_WEIM_DTACK_B       = (22),
    DDK_IOMUX_SELEIN_ESDHC1_IPP_DAT4_IN             = (23),
    DDK_IOMUX_SELEIN_ESDHC1_IPP_DAT5_IN             = (24),
    DDK_IOMUX_SELEIN_ESDHC1_IPP_DAT6_IN             = (25),
    DDK_IOMUX_SELEIN_ESDHC1_IPP_DAT7_IN             = (26),
    DDK_IOMUX_SELEIN_ESDHC3_IPP_CARD_CLK_IN         = (27),
    DDK_IOMUX_SELEIN_ESDHC3_IPP_CMD_IN              = (28),
    DDK_IOMUX_SELEIN_ESDHC3_IPP_DAT0_IN             = (29),
    DDK_IOMUX_SELEIN_ESDHC3_IPP_DAT1_IN             = (30),
    DDK_IOMUX_SELEIN_ESDHC3_IPP_DAT2_IN             = (31),
    DDK_IOMUX_SELEIN_ESDHC3_IPP_DAT3_IN             = (32),
    DDK_IOMUX_SELEIN_GPIO1_IPP_IND_G_IN_0           = (33),
    DDK_IOMUX_SELEIN_GPIO1_IPP_IND_G_IN_10          = (34),
    DDK_IOMUX_SELEIN_GPIO1_IPP_IND_G_IN_11          = (35),
    DDK_IOMUX_SELEIN_GPIO1_IPP_IND_G_IN_1           = (36),
    DDK_IOMUX_SELEIN_GPIO1_IPP_IND_G_IN_20          = (37),
    DDK_IOMUX_SELEIN_GPIO1_IPP_IND_G_IN_21          = (38),
    DDK_IOMUX_SELEIN_GPIO1_IPP_IND_G_IN_22          = (39),
    DDK_IOMUX_SELEIN_GPIO1_IPP_IND_G_IN_2           = (40),
    DDK_IOMUX_SELEIN_GPIO1_IPP_IND_G_IN_3           = (41),
    DDK_IOMUX_SELEIN_GPIO1_IPP_IND_G_IN_4           = (42),
    DDK_IOMUX_SELEIN_GPIO1_IPP_IND_G_IN_5           = (43),
    DDK_IOMUX_SELEIN_GPIO1_IPP_IND_G_IN_6           = (44),
    DDK_IOMUX_SELEIN_GPIO1_IPP_IND_G_IN_7           = (45),
    DDK_IOMUX_SELEIN_GPIO1_IPP_IND_G_IN_8           = (46),
    DDK_IOMUX_SELEIN_GPIO1_IPP_IND_G_IN_9           = (47),
    DDK_IOMUX_SELEIN_GPIO2_IPP_IND_G_IN_0           = (48),
    DDK_IOMUX_SELEIN_GPIO2_IPP_IND_G_IN_10          = (49),
    DDK_IOMUX_SELEIN_GPIO2_IPP_IND_G_IN_11          = (50),
    DDK_IOMUX_SELEIN_GPIO2_IPP_IND_G_IN_12          = (51),
    DDK_IOMUX_SELEIN_GPIO2_IPP_IND_G_IN_13          = (52),
    DDK_IOMUX_SELEIN_GPIO2_IPP_IND_G_IN_14          = (53),
    DDK_IOMUX_SELEIN_GPIO2_IPP_IND_G_IN_15          = (54),
    DDK_IOMUX_SELEIN_GPIO2_IPP_IND_G_IN_16          = (55),
    DDK_IOMUX_SELEIN_GPIO2_IPP_IND_G_IN_17          = (56),
    DDK_IOMUX_SELEIN_GPIO2_IPP_IND_G_IN_18          = (57),
    DDK_IOMUX_SELEIN_GPIO2_IPP_IND_G_IN_19          = (58),
    DDK_IOMUX_SELEIN_GPIO2_IPP_IND_G_IN_1           = (59),
    DDK_IOMUX_SELEIN_GPIO2_IPP_IND_G_IN_20          = (60),
    DDK_IOMUX_SELEIN_GPIO2_IPP_IND_G_IN_21          = (61),
    DDK_IOMUX_SELEIN_GPIO2_IPP_IND_G_IN_22          = (62),
    DDK_IOMUX_SELEIN_GPIO2_IPP_IND_G_IN_23          = (63),
    DDK_IOMUX_SELEIN_GPIO2_IPP_IND_G_IN_24          = (64),
    DDK_IOMUX_SELEIN_GPIO2_IPP_IND_G_IN_25          = (65),
    DDK_IOMUX_SELEIN_GPIO2_IPP_IND_G_IN_26          = (66),
    DDK_IOMUX_SELEIN_GPIO2_IPP_IND_G_IN_27          = (67),
    DDK_IOMUX_SELEIN_GPIO2_IPP_IND_G_IN_28          = (68),
    DDK_IOMUX_SELEIN_GPIO2_IPP_IND_G_IN_29          = (69),
    DDK_IOMUX_SELEIN_GPIO2_IPP_IND_G_IN_2           = (70),
    DDK_IOMUX_SELEIN_GPIO2_IPP_IND_G_IN_30          = (71),
    DDK_IOMUX_SELEIN_GPIO2_IPP_IND_G_IN_31          = (72),
    DDK_IOMUX_SELEIN_GPIO2_IPP_IND_G_IN_3           = (73),
    DDK_IOMUX_SELEIN_GPIO2_IPP_IND_G_IN_4           = (74),
    DDK_IOMUX_SELEIN_GPIO2_IPP_IND_G_IN_5           = (75),
    DDK_IOMUX_SELEIN_GPIO2_IPP_IND_G_IN_6           = (76),
    DDK_IOMUX_SELEIN_GPIO2_IPP_IND_G_IN_7           = (77),
    DDK_IOMUX_SELEIN_GPIO2_IPP_IND_G_IN_8           = (78),
    DDK_IOMUX_SELEIN_GPIO2_IPP_IND_G_IN_9           = (79),
    DDK_IOMUX_SELEIN_GPIO3_IPP_IND_G_IN_0           = (80),
    DDK_IOMUX_SELEIN_GPIO3_IPP_IND_G_IN_10          = (81),
    DDK_IOMUX_SELEIN_GPIO3_IPP_IND_G_IN_11          = (82),
    DDK_IOMUX_SELEIN_GPIO3_IPP_IND_G_IN_12          = (83),
    DDK_IOMUX_SELEIN_GPIO3_IPP_IND_G_IN_13          = (84),
    DDK_IOMUX_SELEIN_GPIO3_IPP_IND_G_IN_14          = (85),
    DDK_IOMUX_SELEIN_GPIO3_IPP_IND_G_IN_15          = (86),
    DDK_IOMUX_SELEIN_GPIO3_IPP_IND_G_IN_4           = (87),
    DDK_IOMUX_SELEIN_GPIO3_IPP_IND_G_IN_5           = (88),
    DDK_IOMUX_SELEIN_GPIO3_IPP_IND_G_IN_6           = (89),
    DDK_IOMUX_SELEIN_GPIO3_IPP_IND_G_IN_7           = (90),
    DDK_IOMUX_SELEIN_GPIO3_IPP_IND_G_IN_8           = (91),
    DDK_IOMUX_SELEIN_GPIO3_IPP_IND_G_IN_9           = (92),
    DDK_IOMUX_SELEIN_I2C3_IPP_SCL_IN                = (93),
    DDK_IOMUX_SELEIN_I2C3_IPP_SDA_IN                = (94),
    DDK_IOMUX_SELEIN_IPU_IPP_IND_DISPB_D0_VSYNC     = (95),
    DDK_IOMUX_SELEIN_IPU_IPP_IND_DISPB_D12_VSYNC    = (96),
    DDK_IOMUX_SELEIN_IPU_IPP_IND_DISPB_SD_D         = (97),
    DDK_IOMUX_SELEIN_IPU_IPP_IND_SENSB_DATA_0       = (98),
    DDK_IOMUX_SELEIN_IPU_IPP_IND_SENSB_DATA_1       = (99),
    DDK_IOMUX_SELEIN_IPU_IPP_IND_SENSB_DATA_2       = (100),
    DDK_IOMUX_SELEIN_IPU_IPP_IND_SENSB_DATA_3       = (101),
    DDK_IOMUX_SELEIN_IPU_IPP_IND_SENSB_DATA_4       = (102),
    DDK_IOMUX_SELEIN_IPU_IPP_IND_SENSB_DATA_5       = (103),
    DDK_IOMUX_SELEIN_IPU_IPP_IND_SENSB_DATA_6       = (104),
    DDK_IOMUX_SELEIN_IPU_IPP_IND_SENSB_DATA_7       = (105),
    DDK_IOMUX_SELEIN_KPP_IPP_IND_COL_0              = (106),
    DDK_IOMUX_SELEIN_KPP_IPP_IND_COL_1              = (107),
    DDK_IOMUX_SELEIN_KPP_IPP_IND_COL_2              = (108),
    DDK_IOMUX_SELEIN_KPP_IPP_IND_COL_3              = (109),
    DDK_IOMUX_SELEIN_KPP_IPP_IND_COL_4              = (110),
    DDK_IOMUX_SELEIN_KPP_IPP_IND_COL_5              = (111),
    DDK_IOMUX_SELEIN_KPP_IPP_IND_COL_6              = (112),
    DDK_IOMUX_SELEIN_KPP_IPP_IND_COL_7              = (113),
    DDK_IOMUX_SELEIN_KPP_IPP_IND_ROW_0              = (114),
    DDK_IOMUX_SELEIN_KPP_IPP_IND_ROW_1              = (115),
    DDK_IOMUX_SELEIN_KPP_IPP_IND_ROW_2              = (116),
    DDK_IOMUX_SELEIN_KPP_IPP_IND_ROW_3              = (117),
    DDK_IOMUX_SELEIN_KPP_IPP_IND_ROW_4              = (118),
    DDK_IOMUX_SELEIN_KPP_IPP_IND_ROW_5              = (119),
    DDK_IOMUX_SELEIN_KPP_IPP_IND_ROW_6              = (120),
    DDK_IOMUX_SELEIN_KPP_IPP_IND_ROW_7              = (121),
    DDK_IOMUX_SELEIN_OWIRE_BATTERY_LINE_IN          = (122),
    DDK_IOMUX_SELEIN_SPDIF_HCKT_CLK2                = (123),
    DDK_IOMUX_SELEIN_SPDIF_SPDIF_IN1                = (124),
    DDK_IOMUX_SELEIN_UART3_IPP_UART_RTS_B           = (125),
    DDK_IOMUX_SELEIN_UART3_IPP_UART_RXD_MUX         = (126),
    DDK_IOMUX_SELEIN_USB_TOP_IPP_IND_OTG_DATA_0     = (127),
    DDK_IOMUX_SELEIN_USB_TOP_IPP_IND_OTG_DATA_1     = (128),
    DDK_IOMUX_SELEIN_USB_TOP_IPP_IND_OTG_DATA_2     = (129),
    DDK_IOMUX_SELEIN_USB_TOP_IPP_IND_OTG_DATA_3     = (130),
    DDK_IOMUX_SELEIN_USB_TOP_IPP_IND_OTG_DATA_4     = (131),
    DDK_IOMUX_SELEIN_USB_TOP_IPP_IND_OTG_DATA_5     = (132),
    DDK_IOMUX_SELEIN_USB_TOP_IPP_IND_OTG_DATA_6     = (133),
    DDK_IOMUX_SELEIN_USB_TOP_IPP_IND_OTG_DATA_7     = (134),
    DDK_IOMUX_SELEIN_USB_TOP_IPP_IND_OTG_DIR        = (135),
    DDK_IOMUX_SELEIN_USB_TOP_IPP_IND_OTG_NXT        = (136),
    DDK_IOMUX_SELEIN_USB_TOP_IPP_IND_UH2_DATA_0     = (137),
    DDK_IOMUX_SELEIN_USB_TOP_IPP_IND_UH2_DATA_1     = (138),
    DDK_IOMUX_SELEIN_USB_TOP_IPP_IND_UH2_DATA_2     = (139),
    DDK_IOMUX_SELEIN_USB_TOP_IPP_IND_UH2_DATA_3     = (140),
    DDK_IOMUX_SELEIN_USB_TOP_IPP_IND_UH2_DATA_4     = (141),
    DDK_IOMUX_SELEIN_USB_TOP_IPP_IND_UH2_DATA_5     = (142),
    DDK_IOMUX_SELEIN_USB_TOP_IPP_IND_UH2_DATA_6     = (143),
    DDK_IOMUX_SELEIN_USB_TOP_IPP_IND_UH2_DATA_7     = (144),
    DDK_IOMUX_SELEIN_USB_TOP_IPP_IND_UH2_DIR        = (145),
    DDK_IOMUX_SELEIN_USB_TOP_IPP_IND_UH2_NXT        = (146),
    DDK_IOMUX_SELEIN_USB_TOP_IPP_IND_UH2_USB_OC     = (147)
} DDK_IOMUX_SELEIN;

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
    DDK_DVFC_SETPOINT_TURBO         = 0,
    DDK_DVFC_SETPOINT_HIGH          = 1,
    DDK_DVFC_SETPOINT_MEDIUM        = 2,
    DDK_DVFC_SETPOINT_LOW           = 3,
    DDK_DVFC_NUM_SETPOINTS          = 4
} DDK_DVFC_SETPOINT;


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
    BOOL bDvfcActive;
    DDK_DVFC_SETPOINT setpointCur;
    BOOL bSetpointPending;
    BOOL bAllowPerClkScale;
    BOOL bReqPerClkScale;
    BOOL bAllowBusClkScale;
    BOOL bReqBusClkScale;
    BOOL bEmiDcgEnable;
    UINT32 setpointReqCount[DDK_DVFC_NUM_SETPOINTS];
    UINT32 dmaEmiMask;
    DDK_CLOCK_GATE_MODE nfcClockGateMode;
}  DDK_CLK_CONFIG, *PDDK_CLK_CONFIG;

//------------------------------------------------------------------------------
// Macros

// ROM ID is used to uniquely identify the silicon version:
//
//      ROM ID      Silicon Rev
//      -----------------------
//      0x1         TO1
//      0x2         TO2
//
#define DDK_SI_REV_TO1      0x1
#define DDK_SI_REV_TO2      0x2


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
BOOL DDKClockConfigBaud(DDK_CLOCK_SIGNAL sig, DDK_CLOCK_BAUD_SOURCE src, UINT32 preDiv, UINT32 postDiv);
BOOL DDKClockSetCKO(BOOL bEnable, DDK_CLOCK_CKO_SRC src, UINT32 div1, UINT32 preDiv, UINT32 postDiv);
BOOL DDKClockSetpointRequest(DDK_DVFC_SETPOINT setpoint, BOOL bBlock);
BOOL DDKClockSetpointRelease(DDK_DVFC_SETPOINT setpoint);
 
#endif // __MX35_DDK_H
