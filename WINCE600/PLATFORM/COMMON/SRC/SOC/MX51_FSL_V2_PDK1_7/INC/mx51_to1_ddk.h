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
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  mx51_to1_ddk.h
//
//  Contains MX51 TO1 definitions to assist with driver development.
//
//------------------------------------------------------------------------------
#ifndef __MX51_TO1_DDK_H
#define __MX51_TO1_DDK_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Types
typedef CSP_IOMUX_TO1_REGS  CSP_IOMUX_REGS;
typedef PCSP_IOMUX_TO1_REGS PCSP_IOMUX_REGS;


//-----------------------------------------------------------------------------
//
//  Type: DDK_IOMUX_PIN
//
//  Specifies the functional pin name used to configure the IOMUX SW_MUX_CTL.
//
//-----------------------------------------------------------------------------
typedef enum {
    DDK_IOMUX_PIN_EIM_DA0           = (0),
    DDK_IOMUX_PIN_EIM_DA1           = (1),
    DDK_IOMUX_PIN_EIM_DA2           = (2),
    DDK_IOMUX_PIN_EIM_DA3           = (3),
    DDK_IOMUX_PIN_EIM_DA4           = (4),
    DDK_IOMUX_PIN_EIM_DA5           = (5),
    DDK_IOMUX_PIN_EIM_DA6           = (6),
    DDK_IOMUX_PIN_EIM_DA7           = (7),
    DDK_IOMUX_PIN_EIM_DA8           = (8),
    DDK_IOMUX_PIN_EIM_DA9           = (9),
    DDK_IOMUX_PIN_EIM_DA10          = (10),
    DDK_IOMUX_PIN_EIM_DA11          = (11),
    DDK_IOMUX_PIN_EIM_DA12          = (12),
    DDK_IOMUX_PIN_EIM_DA13          = (13),
    DDK_IOMUX_PIN_EIM_DA14          = (14),
    DDK_IOMUX_PIN_EIM_DA15          = (15),
    DDK_IOMUX_PIN_EIM_D16           = (16),
    DDK_IOMUX_PIN_EIM_D17           = (17),
    DDK_IOMUX_PIN_EIM_D18           = (18),
    DDK_IOMUX_PIN_EIM_D19           = (19),
    DDK_IOMUX_PIN_EIM_D20           = (20),
    DDK_IOMUX_PIN_EIM_D21           = (21),
    DDK_IOMUX_PIN_EIM_D22           = (22),
    DDK_IOMUX_PIN_EIM_D23           = (23),
    DDK_IOMUX_PIN_EIM_D24           = (24),
    DDK_IOMUX_PIN_EIM_D25           = (25),
    DDK_IOMUX_PIN_EIM_D26           = (26),
    DDK_IOMUX_PIN_EIM_D27           = (27),
    DDK_IOMUX_PIN_EIM_D28           = (28),
    DDK_IOMUX_PIN_EIM_D29           = (29),
    DDK_IOMUX_PIN_EIM_D30           = (30),
    DDK_IOMUX_PIN_EIM_D31           = (31),
    DDK_IOMUX_PIN_EIM_A16           = (32),
    DDK_IOMUX_PIN_EIM_A17           = (33),
    DDK_IOMUX_PIN_EIM_A18           = (34),
    DDK_IOMUX_PIN_EIM_A19           = (35),
    DDK_IOMUX_PIN_EIM_A20           = (36),
    DDK_IOMUX_PIN_EIM_A21           = (37),
    DDK_IOMUX_PIN_EIM_A22           = (38),
    DDK_IOMUX_PIN_EIM_A23           = (39),
    DDK_IOMUX_PIN_EIM_A24           = (40),
    DDK_IOMUX_PIN_EIM_A25           = (41),
    DDK_IOMUX_PIN_EIM_A26           = (42),
    DDK_IOMUX_PIN_EIM_A27           = (43),
    DDK_IOMUX_PIN_EIM_EB0           = (44),
    DDK_IOMUX_PIN_EIM_EB1           = (45),
    DDK_IOMUX_PIN_EIM_EB2           = (46),
    DDK_IOMUX_PIN_EIM_EB3           = (47),
    DDK_IOMUX_PIN_EIM_OE            = (48),
    DDK_IOMUX_PIN_EIM_CS0           = (49),
    DDK_IOMUX_PIN_EIM_CS1           = (50),
    DDK_IOMUX_PIN_EIM_CS2           = (51),
    DDK_IOMUX_PIN_EIM_CS3           = (52),
    DDK_IOMUX_PIN_EIM_CS4           = (53),
    DDK_IOMUX_PIN_EIM_CS5           = (54),
    DDK_IOMUX_PIN_EIM_DTACK         = (55),
    DDK_IOMUX_PIN_EIM_LBA           = (56),
    DDK_IOMUX_PIN_EIM_CRE           = (57),
    DDK_IOMUX_PIN_DRAM_CS1          = (58),
    DDK_IOMUX_PIN_NANDF_WE_B        = (59),
    DDK_IOMUX_PIN_NANDF_RE_B        = (60),
    DDK_IOMUX_PIN_NANDF_ALE         = (61),
    DDK_IOMUX_PIN_NANDF_CLE         = (62),
    DDK_IOMUX_PIN_NANDF_WP_B        = (63),
    DDK_IOMUX_PIN_NANDF_RB0         = (64),
    DDK_IOMUX_PIN_NANDF_RB1         = (65),
    DDK_IOMUX_PIN_NANDF_RB2         = (66),
    DDK_IOMUX_PIN_NANDF_RB3         = (67),
    DDK_IOMUX_PIN_NANDF_RB4         = (68),
    DDK_IOMUX_PIN_NANDF_RB5         = (69),
    DDK_IOMUX_PIN_NANDF_RB6         = (70),
    DDK_IOMUX_PIN_NANDF_RB7         = (71),
    DDK_IOMUX_PIN_NANDF_CS0         = (72),
    DDK_IOMUX_PIN_NANDF_CS1         = (73),
    DDK_IOMUX_PIN_NANDF_CS2         = (74),
    DDK_IOMUX_PIN_NANDF_CS3         = (75),
    DDK_IOMUX_PIN_NANDF_CS4         = (76),
    DDK_IOMUX_PIN_NANDF_CS5         = (77),
    DDK_IOMUX_PIN_NANDF_CS6         = (78),
    DDK_IOMUX_PIN_NANDF_CS7         = (79),
    DDK_IOMUX_PIN_NANDF_RDY_INT     = (80),
    DDK_IOMUX_PIN_NANDF_D15         = (81),
    DDK_IOMUX_PIN_NANDF_D14         = (82),
    DDK_IOMUX_PIN_NANDF_D13         = (83),
    DDK_IOMUX_PIN_NANDF_D12         = (84),
    DDK_IOMUX_PIN_NANDF_D11         = (85),
    DDK_IOMUX_PIN_NANDF_D10         = (86),
    DDK_IOMUX_PIN_NANDF_D9          = (87),
    DDK_IOMUX_PIN_NANDF_D8          = (88),
    DDK_IOMUX_PIN_NANDF_D7          = (89),
    DDK_IOMUX_PIN_NANDF_D6          = (90),
    DDK_IOMUX_PIN_NANDF_D5          = (91),
    DDK_IOMUX_PIN_NANDF_D4          = (92),
    DDK_IOMUX_PIN_NANDF_D3          = (93),
    DDK_IOMUX_PIN_NANDF_D2          = (94),
    DDK_IOMUX_PIN_NANDF_D1          = (95),
    DDK_IOMUX_PIN_NANDF_D0          = (96),
    DDK_IOMUX_PIN_CSI1_D8           = (97),
    DDK_IOMUX_PIN_CSI1_D9           = (98),
    DDK_IOMUX_PIN_CSI1_D10          = (99),
    DDK_IOMUX_PIN_CSI1_D11          = (100),
    DDK_IOMUX_PIN_CSI1_D12          = (101),
    DDK_IOMUX_PIN_CSI1_D13          = (102),
    DDK_IOMUX_PIN_CSI1_D14          = (103),
    DDK_IOMUX_PIN_CSI1_D15          = (104),
    DDK_IOMUX_PIN_CSI1_D16          = (105),
    DDK_IOMUX_PIN_CSI1_D17          = (106),
    DDK_IOMUX_PIN_CSI1_D18          = (107),
    DDK_IOMUX_PIN_CSI1_D19          = (108),
    DDK_IOMUX_PIN_CSI1_VSYNC        = (109),
    DDK_IOMUX_PIN_CSI1_HSYNC        = (110),
    DDK_IOMUX_PIN_CSI2_D12          = (111),
    DDK_IOMUX_PIN_CSI2_D13          = (112),
    DDK_IOMUX_PIN_CSI2_D14          = (113),
    DDK_IOMUX_PIN_CSI2_D15          = (114),
    DDK_IOMUX_PIN_CSI2_D16          = (115),
    DDK_IOMUX_PIN_CSI2_D17          = (116),
    DDK_IOMUX_PIN_CSI2_D18          = (117),
    DDK_IOMUX_PIN_CSI2_D19          = (118),
    DDK_IOMUX_PIN_CSI2_VSYNC        = (119),
    DDK_IOMUX_PIN_CSI2_HSYNC        = (120),
    DDK_IOMUX_PIN_CSI2_PIXCLK       = (121),
    DDK_IOMUX_PIN_I2C1_CLK          = (122),
    DDK_IOMUX_PIN_I2C1_DAT          = (123),
    DDK_IOMUX_PIN_AUD3_BB_TXD       = (124),
    DDK_IOMUX_PIN_AUD3_BB_RXD       = (125),
    DDK_IOMUX_PIN_AUD3_BB_CK        = (126),
    DDK_IOMUX_PIN_AUD3_BB_FS        = (127),
    DDK_IOMUX_PIN_CSPI1_MOSI        = (128),
    DDK_IOMUX_PIN_CSPI1_MISO        = (129),
    DDK_IOMUX_PIN_CSPI1_SS0         = (130),
    DDK_IOMUX_PIN_CSPI1_SS1         = (131),
    DDK_IOMUX_PIN_CSPI1_RDY         = (132),
    DDK_IOMUX_PIN_CSPI1_SCLK        = (133),
    DDK_IOMUX_PIN_UART1_RXD         = (134),
    DDK_IOMUX_PIN_UART1_TXD         = (135),
    DDK_IOMUX_PIN_UART1_RTS         = (136),
    DDK_IOMUX_PIN_UART1_CTS         = (137),
    DDK_IOMUX_PIN_UART2_RXD         = (138),
    DDK_IOMUX_PIN_UART2_TXD         = (139),
    DDK_IOMUX_PIN_UART3_RXD         = (140),
    DDK_IOMUX_PIN_UART3_TXD         = (141),
    DDK_IOMUX_PIN_OWIRE_LINE        = (142),
    DDK_IOMUX_PIN_KEY_ROW0          = (143),
    DDK_IOMUX_PIN_KEY_ROW1          = (144),
    DDK_IOMUX_PIN_KEY_ROW2          = (145),
    DDK_IOMUX_PIN_KEY_ROW3          = (146),
    DDK_IOMUX_PIN_KEY_COL0          = (147),
    DDK_IOMUX_PIN_KEY_COL1          = (148),
    DDK_IOMUX_PIN_KEY_COL2          = (149),
    DDK_IOMUX_PIN_KEY_COL3          = (150),
    DDK_IOMUX_PIN_KEY_COL4          = (151),
    DDK_IOMUX_PIN_KEY_COL5          = (152),
    DDK_IOMUX_PIN_JTAG_DE_B         = (153),
    DDK_IOMUX_PIN_USBH1_CLK         = (154),
    DDK_IOMUX_PIN_USBH1_DIR         = (155),
    DDK_IOMUX_PIN_USBH1_STP         = (156),
    DDK_IOMUX_PIN_USBH1_NXT         = (157),
    DDK_IOMUX_PIN_USBH1_DATA0       = (158),
    DDK_IOMUX_PIN_USBH1_DATA1       = (159),
    DDK_IOMUX_PIN_USBH1_DATA2       = (160),
    DDK_IOMUX_PIN_USBH1_DATA3       = (161),
    DDK_IOMUX_PIN_USBH1_DATA4       = (162),
    DDK_IOMUX_PIN_USBH1_DATA5       = (163),
    DDK_IOMUX_PIN_USBH1_DATA6       = (164),
    DDK_IOMUX_PIN_USBH1_DATA7       = (165),
    DDK_IOMUX_PIN_DI1_PIN11         = (166),
    DDK_IOMUX_PIN_DI1_PIN12         = (167),
    DDK_IOMUX_PIN_DI1_PIN13         = (168),
    DDK_IOMUX_PIN_DI1_D0_CS         = (169),
    DDK_IOMUX_PIN_DI1_D1_CS         = (170),
    DDK_IOMUX_PIN_DISPB2_SER_DIN    = (171),
    DDK_IOMUX_PIN_DISPB2_SER_DIO    = (172),
    DDK_IOMUX_PIN_DISPB2_SER_CLK    = (173),
    DDK_IOMUX_PIN_DISPB2_SER_RS     = (174),
    DDK_IOMUX_PIN_DISP1_DAT0        = (175),
    DDK_IOMUX_PIN_DISP1_DAT1        = (176),
    DDK_IOMUX_PIN_DISP1_DAT2        = (177),
    DDK_IOMUX_PIN_DISP1_DAT3        = (178),
    DDK_IOMUX_PIN_DISP1_DAT4        = (179),
    DDK_IOMUX_PIN_DISP1_DAT5        = (180),
    DDK_IOMUX_PIN_DISP1_DAT6        = (181),
    DDK_IOMUX_PIN_DISP1_DAT7        = (182),
    DDK_IOMUX_PIN_DISP1_DAT8        = (183),
    DDK_IOMUX_PIN_DISP1_DAT9        = (184),
    DDK_IOMUX_PIN_DISP1_DAT10       = (185),
    DDK_IOMUX_PIN_DISP1_DAT12       = (186),
    DDK_IOMUX_PIN_DISP1_DAT13       = (187),
    DDK_IOMUX_PIN_DISP1_DAT14       = (188),
    DDK_IOMUX_PIN_DISP1_DAT15       = (189),
    DDK_IOMUX_PIN_DISP1_DAT16       = (190),
    DDK_IOMUX_PIN_DISP1_DAT17       = (191),
    DDK_IOMUX_PIN_DISP1_DAT18       = (192),
    DDK_IOMUX_PIN_DISP1_DAT19       = (193),
    DDK_IOMUX_PIN_DISP1_DAT20       = (194),
    DDK_IOMUX_PIN_DISP1_DAT21       = (195),
    DDK_IOMUX_PIN_DISP1_DAT22       = (196),
    DDK_IOMUX_PIN_DISP1_DAT23       = (197),
    DDK_IOMUX_PIN_DI1_PIN3          = (198),
    DDK_IOMUX_PIN_DI1_PIN2          = (199),
    DDK_IOMUX_PIN_DI_GP1            = (200),
    DDK_IOMUX_PIN_DI_GP2            = (201),
    DDK_IOMUX_PIN_DI_GP3            = (202),
    DDK_IOMUX_PIN_DI2_PIN4          = (203),
    DDK_IOMUX_PIN_DI2_PIN2          = (204),
    DDK_IOMUX_PIN_DI2_PIN3          = (205),
    DDK_IOMUX_PIN_DI2_DISP_CLK      = (206),
    DDK_IOMUX_PIN_DI_GP4            = (207),
    DDK_IOMUX_PIN_DISP2_DAT0        = (208),
    DDK_IOMUX_PIN_DISP2_DAT1        = (209),
    DDK_IOMUX_PIN_DISP2_DAT2        = (210),
    DDK_IOMUX_PIN_DISP2_DAT3        = (211),
    DDK_IOMUX_PIN_DISP2_DAT4        = (212),
    DDK_IOMUX_PIN_DISP2_DAT5        = (213),
    DDK_IOMUX_PIN_DISP2_DAT6        = (214),
    DDK_IOMUX_PIN_DISP2_DAT7        = (215),
    DDK_IOMUX_PIN_DISP2_DAT8        = (216),
    DDK_IOMUX_PIN_DISP2_DAT9        = (217),
    DDK_IOMUX_PIN_DISP2_DAT10       = (218),
    DDK_IOMUX_PIN_DISP2_DAT11       = (219),
    DDK_IOMUX_PIN_DISP2_DAT12       = (220),
    DDK_IOMUX_PIN_DISP2_DAT13       = (221),
    DDK_IOMUX_PIN_DISP2_DAT14       = (222),
    DDK_IOMUX_PIN_DISP2_DAT15       = (223),
    DDK_IOMUX_PIN_SD1_CMD           = (224),
    DDK_IOMUX_PIN_SD1_CLK           = (225),
    DDK_IOMUX_PIN_SD1_DATA0         = (226),
    DDK_IOMUX_PIN_SD1_DATA1         = (227),
    DDK_IOMUX_PIN_SD1_DATA2         = (228),
    DDK_IOMUX_PIN_SD1_DATA3         = (229),
    DDK_IOMUX_PIN_GPIO1_0           = (230),
    DDK_IOMUX_PIN_GPIO1_1           = (231),
    DDK_IOMUX_PIN_SD2_CMD           = (232),
    DDK_IOMUX_PIN_SD2_CLK           = (233),
    DDK_IOMUX_PIN_SD2_DATA0         = (234),
    DDK_IOMUX_PIN_SD2_DATA1         = (235),
    DDK_IOMUX_PIN_SD2_DATA2         = (236),
    DDK_IOMUX_PIN_SD2_DATA3         = (237),
    DDK_IOMUX_PIN_GPIO1_2           = (238),
    DDK_IOMUX_PIN_GPIO1_3           = (239),
    DDK_IOMUX_PIN_PMIC_INT_REQ      = (240),
    DDK_IOMUX_PIN_GPIO1_4           = (241),
    DDK_IOMUX_PIN_GPIO1_5           = (242),
    DDK_IOMUX_PIN_GPIO1_6           = (243),
    DDK_IOMUX_PIN_GPIO1_7           = (244),
    DDK_IOMUX_PIN_GPIO1_8           = (245),
    DDK_IOMUX_PIN_GPIO1_9           = (246)
} DDK_IOMUX_PIN;


//-----------------------------------------------------------------------------
//
//  Type: DDK_IOMUX_SELECT_INPUT
//
//  Specifies the ports that have select input configuration.
//
//-----------------------------------------------------------------------------
typedef enum {
    DDK_IOMUX_SELECT_INPUT_AUDMUX_P4_INPUT_DA_AMX              = (0),
    DDK_IOMUX_SELECT_INPUT_AUDMUX_P4_INPUT_DB_AMX              = (1),  
    DDK_IOMUX_SELECT_INPUT_AUDMUX_P4_INPUT_TXCLK_AMX           = (2),
    DDK_IOMUX_SELECT_INPUT_AUDMUX_P4_INPUT_TXFS_AMX            = (3),
    DDK_IOMUX_SELECT_INPUT_AUDMUX_P5_INPUT_DA_AMX              = (4),
    DDK_IOMUX_SELECT_INPUT_AUDMUX_P5_INPUT_DB_AMX              = (5),
    DDK_IOMUX_SELECT_INPUT_AUDMUX_P5_INPUT_RXCLK_AMX           = (6),
    DDK_IOMUX_SELECT_INPUT_AUDMUX_P5_INPUT_RXFS_AMX            = (7),
    DDK_IOMUX_SELECT_INPUT_AUDMUX_P5_INPUT_TXCLK_AMX           = (8),
    DDK_IOMUX_SELECT_INPUT_AUDMUX_P5_INPUT_TXFS_AMX            = (9),
    DDK_IOMUX_SELECT_INPUT_AUDMUX_P6_INPUT_DA_AMX              = (10),
    DDK_IOMUX_SELECT_INPUT_AUDMUX_P6_INPUT_DB_AMX              = (11),
    DDK_IOMUX_SELECT_INPUT_AUDMUX_P6_INPUT_RXCLK_AMX           = (12),
    DDK_IOMUX_SELECT_INPUT_AUDMUX_P6_INPUT_RXFS_AMX            = (13),
    DDK_IOMUX_SELECT_INPUT_AUDMUX_P6_INPUT_TXCLK_AMX           = (14),
    DDK_IOMUX_SELECT_INPUT_AUDMUX_P6_INPUT_TXFS_AMX            = (15),
    DDK_IOMUX_SELECT_INPUT_CCM_IPP_DI_CLK                      = (16),
    DDK_IOMUX_SELECT_INPUT_CCM_PLL1_BYPASS_CLK                 = (17),
    DDK_IOMUX_SELECT_INPUT_CCM_PLL2_BYPASS_CLK                 = (18),
    DDK_IOMUX_SELECT_INPUT_CSPI_IPP_CSPI_CLK_IN                = (19),
    DDK_IOMUX_SELECT_INPUT_CSPI_IPP_IND_MISO                   = (20),
    DDK_IOMUX_SELECT_INPUT_CSPI_IPP_IND_MOSI                   = (21),
    DDK_IOMUX_SELECT_INPUT_CSPI_IPP_IND_SS1_B                  = (22),
    DDK_IOMUX_SELECT_INPUT_CSPI_IPP_IND_SS2_B                  = (23),
    DDK_IOMUX_SELECT_INPUT_CSPI_IPP_IND_SS3_B                  = (24),
    DDK_IOMUX_SELECT_INPUT_DPLLIP1_L1T_TOG_EN                  = (25),
    DDK_IOMUX_SELECT_INPUT_ECSPI2_IPP_IND_SS_B_3               = (26),
    DDK_IOMUX_SELECT_INPUT_EMI_IPP_IND_RDY_INT                 = (27),
    DDK_IOMUX_SELECT_INPUT_ESDHC3_IPP_DAT0_IN                  = (28),
    DDK_IOMUX_SELECT_INPUT_ESDHC3_IPP_DAT1_IN                  = (29),
    DDK_IOMUX_SELECT_INPUT_ESDHC3_IPP_DAT2_IN                  = (30),
    DDK_IOMUX_SELECT_INPUT_ESDHC3_IPP_DAT3_IN                  = (31),
    DDK_IOMUX_SELECT_INPUT_FEC_FEC_COL                         = (32),
    DDK_IOMUX_SELECT_INPUT_FEC_FEC_CRS                         = (33),
    DDK_IOMUX_SELECT_INPUT_FEC_FEC_MDI                         = (34),
    DDK_IOMUX_SELECT_INPUT_FEC_FEC_RDATA_0                     = (35),
    DDK_IOMUX_SELECT_INPUT_FEC_FEC_RDATA_1                     = (36),
    DDK_IOMUX_SELECT_INPUT_FEC_FEC_RDATA_2                     = (37),
    DDK_IOMUX_SELECT_INPUT_FEC_FEC_RDATA_3                     = (38),
    DDK_IOMUX_SELECT_INPUT_FEC_FEC_RX_CLK                      = (39),
    DDK_IOMUX_SELECT_INPUT_FEC_FEC_RX_DV                       = (40),
    DDK_IOMUX_SELECT_INPUT_FEC_FEC_RX_ER                       = (41),
    DDK_IOMUX_SELECT_INPUT_FEC_FEC_TX_CLK                      = (42),
    DDK_IOMUX_SELECT_INPUT_GPIO3_IPP_IND_G_IN_1                = (43),
    DDK_IOMUX_SELECT_INPUT_GPIO3_IPP_IND_G_IN_2                = (44),
    DDK_IOMUX_SELECT_INPUT_GPIO3_IPP_IND_G_IN_3                = (45),
    DDK_IOMUX_SELECT_INPUT_GPIO3_IPP_IND_G_IN_4                = (46),
    DDK_IOMUX_SELECT_INPUT_GPIO3_IPP_IND_G_IN_5                = (47),
    DDK_IOMUX_SELECT_INPUT_GPIO3_IPP_IND_G_IN_6                = (48),
    DDK_IOMUX_SELECT_INPUT_GPIO3_IPP_IND_G_IN_7                = (49),
    DDK_IOMUX_SELECT_INPUT_GPIO3_IPP_IND_G_IN_8                = (50),
    DDK_IOMUX_SELECT_INPUT_HSC_MIPI_MIX_IPP_IND_SENS1_DATA     = (51),
    DDK_IOMUX_SELECT_INPUT_HSC_MIPI_MIX_IPP_IND_SENS2_DATA     = (52),
    DDK_IOMUX_SELECT_INPUT_HSC_MIPI_MIX_PAR_SISG_TRIG          = (53),
    DDK_IOMUX_SELECT_INPUT_I2C1_IPP_SCL_IN                     = (54),
    DDK_IOMUX_SELECT_INPUT_I2C1_IPP_SDA_IN                     = (55),
    DDK_IOMUX_SELECT_INPUT_I2C2_IPP_SCL_IN                     = (56),
    DDK_IOMUX_SELECT_INPUT_I2C2_IPP_SDA_IN                     = (57),
    DDK_IOMUX_SELECT_INPUT_IPU_IPP_DI_0_IND_DISPB_D0_VSYNC     = (58),
    DDK_IOMUX_SELECT_INPUT_IPU_IPP_DI_0_IND_DISPB_SD_D         = (59),
    DDK_IOMUX_SELECT_INPUT_IPU_IPP_DI_1_EXT_CLK                = (60),
    DDK_IOMUX_SELECT_INPUT_IPU_IPP_DI_1_IND_DISPB_SD_D         = (61),
    DDK_IOMUX_SELECT_INPUT_IPU_IPP_DI_1_WAIT                   = (62),
    DDK_IOMUX_SELECT_INPUT_KPP_IPP_IND_COL_6                   = (63),
    DDK_IOMUX_SELECT_INPUT_KPP_IPP_IND_COL_7                   = (64),
    DDK_IOMUX_SELECT_INPUT_KPP_IPP_IND_ROW_4                   = (65),
    DDK_IOMUX_SELECT_INPUT_KPP_IPP_IND_ROW_5                   = (66),
    DDK_IOMUX_SELECT_INPUT_KPP_IPP_IND_ROW_6                   = (67),
    DDK_IOMUX_SELECT_INPUT_KPP_IPP_IND_ROW_7                   = (68),
    DDK_IOMUX_SELECT_INPUT_UART1_IPP_UART_RTS_B                = (69),
    DDK_IOMUX_SELECT_INPUT_UART1_IPP_UART_RXD_MUX              = (70),
    DDK_IOMUX_SELECT_INPUT_UART2_IPP_UART_RTS_B                = (71),
    DDK_IOMUX_SELECT_INPUT_UART2_IPP_UART_RXD_MUX              = (72),
    DDK_IOMUX_SELECT_INPUT_UART3_IPP_UART_RTS_B                = (73),
    DDK_IOMUX_SELECT_INPUT_UART3_IPP_UART_RXD_MUX              = (74),
    DDK_IOMUX_SELECT_INPUT_USBOH3_IPP_IND_UH3_CLK              = (75),
    DDK_IOMUX_SELECT_INPUT_USBOH3_IPP_IND_UH3_DATA_0           = (76),
    DDK_IOMUX_SELECT_INPUT_USBOH3_IPP_IND_UH3_DATA_1           = (77),
    DDK_IOMUX_SELECT_INPUT_USBOH3_IPP_IND_UH3_DATA_2           = (78),
    DDK_IOMUX_SELECT_INPUT_USBOH3_IPP_IND_UH3_DATA_3           = (79),
    DDK_IOMUX_SELECT_INPUT_USBOH3_IPP_IND_UH3_DATA_4           = (80),
    DDK_IOMUX_SELECT_INPUT_USBOH3_IPP_IND_UH3_DATA_5           = (81),
    DDK_IOMUX_SELECT_INPUT_USBOH3_IPP_IND_UH3_DATA_6           = (82),
    DDK_IOMUX_SELECT_INPUT_USBOH3_IPP_IND_UH3_DATA_7           = (83),
    DDK_IOMUX_SELECT_INPUT_USBOH3_IPP_IND_UH3_DIR              = (84),
    DDK_IOMUX_SELECT_INPUT_USBOH3_IPP_IND_UH3_NXT              = (85),
    DDK_IOMUX_SELECT_INPUT_USBOH3_IPP_IND_UH3_STP              = (86)
} DDK_IOMUX_SELECT_INPUT;


//-----------------------------------------------------------------------------
//
//  Type: DDK_IOMUX_PAD
//
//  Specifies the functional pad name used to configure the IOMUX SW_PAD_CTL.
//
//-----------------------------------------------------------------------------
typedef enum {
    DDK_IOMUX_PAD_EIM_D16            = (0),
    DDK_IOMUX_PAD_EIM_D17            = (1),
    DDK_IOMUX_PAD_EIM_D18            = (2),
    DDK_IOMUX_PAD_EIM_D19            = (3),
    DDK_IOMUX_PAD_EIM_D20            = (4),
    DDK_IOMUX_PAD_EIM_D21            = (5),
    DDK_IOMUX_PAD_EIM_D22            = (6),
    DDK_IOMUX_PAD_EIM_D23            = (7),
    DDK_IOMUX_PAD_EIM_D24            = (8),
    DDK_IOMUX_PAD_EIM_D25            = (9),
    DDK_IOMUX_PAD_EIM_D26            = (10),
    DDK_IOMUX_PAD_EIM_D27            = (11),
    DDK_IOMUX_PAD_EIM_D28            = (12),
    DDK_IOMUX_PAD_EIM_D29            = (13),
    DDK_IOMUX_PAD_EIM_D30            = (14),
    DDK_IOMUX_PAD_EIM_D31            = (15),
    DDK_IOMUX_PAD_EIM_A16            = (16),
    DDK_IOMUX_PAD_EIM_A17            = (17),
    DDK_IOMUX_PAD_EIM_A18            = (18),
    DDK_IOMUX_PAD_EIM_A19            = (19),
    DDK_IOMUX_PAD_EIM_A20            = (20),
    DDK_IOMUX_PAD_EIM_A21            = (21),
    DDK_IOMUX_PAD_EIM_A22            = (22),
    DDK_IOMUX_PAD_EIM_A23            = (23),
    DDK_IOMUX_PAD_EIM_A24            = (24),
    DDK_IOMUX_PAD_EIM_A25            = (25),
    DDK_IOMUX_PAD_EIM_A26            = (26),
    DDK_IOMUX_PAD_EIM_A27            = (27),
    DDK_IOMUX_PAD_EIM_EB0            = (28),
    DDK_IOMUX_PAD_EIM_EB1            = (29),
    DDK_IOMUX_PAD_EIM_EB2            = (30),
    DDK_IOMUX_PAD_EIM_EB3            = (31),
    DDK_IOMUX_PAD_EIM_OE             = (32),
    DDK_IOMUX_PAD_EIM_CS0            = (33),
    DDK_IOMUX_PAD_EIM_CS1            = (34),
    DDK_IOMUX_PAD_EIM_CS2            = (35),
    DDK_IOMUX_PAD_EIM_CS3            = (36),
    DDK_IOMUX_PAD_EIM_CS4            = (37),
    DDK_IOMUX_PAD_EIM_CS5            = (38),
    DDK_IOMUX_PAD_EIM_DTACK          = (39),
    DDK_IOMUX_PAD_EIM_WAIT           = (40),
    DDK_IOMUX_PAD_EIM_LBA            = (41),
    DDK_IOMUX_PAD_EIM_BCLK           = (42),
    DDK_IOMUX_PAD_EIM_RW             = (43),
    DDK_IOMUX_PAD_EIM_CRE            = (44),
    DDK_IOMUX_PAD_DRAM_A0            = (45),
    DDK_IOMUX_PAD_DRAM_A1            = (46),
    DDK_IOMUX_PAD_DRAM_A2            = (47),
    DDK_IOMUX_PAD_DRAM_A3            = (48),
    DDK_IOMUX_PAD_DRAM_A4            = (49),
    DDK_IOMUX_PAD_DRAM_A5            = (50),
    DDK_IOMUX_PAD_DRAM_A6            = (51),
    DDK_IOMUX_PAD_DRAM_A7            = (52),
    DDK_IOMUX_PAD_DRAM_A8            = (53),
    DDK_IOMUX_PAD_DRAM_A9            = (54),
    DDK_IOMUX_PAD_DRAM_A10           = (55),
    DDK_IOMUX_PAD_DRAM_A11           = (56),
    DDK_IOMUX_PAD_DRAM_A12           = (57),
    DDK_IOMUX_PAD_DRAM_A13           = (58),
    DDK_IOMUX_PAD_DRAM_A14           = (59),
    DDK_IOMUX_PAD_EIM_SDBA1          = (60),
    DDK_IOMUX_PAD_EIM_SDBA0          = (61),
    DDK_IOMUX_PAD_DRAM_RAS           = (62),
    DDK_IOMUX_PAD_DRAM_CAS           = (63),
    DDK_IOMUX_PAD_DRAM_SDWE          = (64),
    DDK_IOMUX_PAD_DRAM_SDCKE0        = (65),
    DDK_IOMUX_PAD_DRAM_SDCKE1        = (66),
    DDK_IOMUX_PAD_DRAM_SDCLK         = (67),
    DDK_IOMUX_PAD_DRAM_SDQS0         = (68),
    DDK_IOMUX_PAD_DRAM_SDQS1         = (69),
    DDK_IOMUX_PAD_DRAM_SDQS2         = (70),
    DDK_IOMUX_PAD_DRAM_SDQS3         = (71),
    DDK_IOMUX_PAD_DRAM_CS0           = (72),
    DDK_IOMUX_PAD_DRAM_CS1           = (73),
    DDK_IOMUX_PAD_DRAM_D0            = (74), 
    DDK_IOMUX_PAD_DRAM_D1            = (75),
    DDK_IOMUX_PAD_DRAM_D2            = (76),
    DDK_IOMUX_PAD_DRAM_D3            = (77),
    DDK_IOMUX_PAD_DRAM_D4            = (78),
    DDK_IOMUX_PAD_DRAM_D5            = (79),
    DDK_IOMUX_PAD_DRAM_D6            = (80),
    DDK_IOMUX_PAD_DRAM_D7            = (81),
    DDK_IOMUX_PAD_DRAM_D8            = (82),
    DDK_IOMUX_PAD_DRAM_D9            = (83),
    DDK_IOMUX_PAD_DRAM_D10           = (84),
    DDK_IOMUX_PAD_DRAM_D11           = (85),
    DDK_IOMUX_PAD_DRAM_D12           = (86),
    DDK_IOMUX_PAD_DRAM_D13           = (87),
    DDK_IOMUX_PAD_DRAM_D14           = (88),
    DDK_IOMUX_PAD_DRAM_D15           = (89),
    DDK_IOMUX_PAD_DRAM_D16           = (90),
    DDK_IOMUX_PAD_DRAM_D17           = (91),
    DDK_IOMUX_PAD_DRAM_D18           = (92),
    DDK_IOMUX_PAD_DRAM_D19           = (93),
    DDK_IOMUX_PAD_DRAM_D20           = (94),
    DDK_IOMUX_PAD_DRAM_D21           = (95),
    DDK_IOMUX_PAD_DRAM_D22           = (96),
    DDK_IOMUX_PAD_DRAM_D23           = (97),
    DDK_IOMUX_PAD_DRAM_D24           = (98),
    DDK_IOMUX_PAD_DRAM_D25           = (99),
    DDK_IOMUX_PAD_DRAM_D26           = (100),
    DDK_IOMUX_PAD_DRAM_D27           = (101),
    DDK_IOMUX_PAD_DRAM_D28           = (102),
    DDK_IOMUX_PAD_DRAM_D29           = (103), 
    DDK_IOMUX_PAD_DRAM_D30           = (104),
    DDK_IOMUX_PAD_DRAM_D31           = (105), 
    DDK_IOMUX_PAD_DRAM_DQM0          = (106), 
    DDK_IOMUX_PAD_DRAM_DQM1          = (107), 
    DDK_IOMUX_PAD_DRAM_DQM2          = (108), 
    DDK_IOMUX_PAD_DRAM_DQM3          = (109), 
    DDK_IOMUX_PAD_NANDF_WE_B         = (110),
    DDK_IOMUX_PAD_NANDF_RE_B         = (111),
    DDK_IOMUX_PAD_NANDF_ALE          = (112),
    DDK_IOMUX_PAD_NANDF_CLE          = (113),
    DDK_IOMUX_PAD_NANDF_WP_B         = (114),
    DDK_IOMUX_PAD_NANDF_RB0          = (115),
    DDK_IOMUX_PAD_NANDF_RB1          = (116),
    DDK_IOMUX_PAD_NANDF_RB2          = (117),
    DDK_IOMUX_PAD_NANDF_RB3          = (118),
    DDK_IOMUX_PAD_NANDF_RB4          = (119),
    DDK_IOMUX_PAD_NANDF_RB5          = (120),
    DDK_IOMUX_PAD_NANDF_RB6          = (121),
    DDK_IOMUX_PAD_NANDF_RB7          = (122),
    DDK_IOMUX_PAD_NANDF_CS0          = (123),
    DDK_IOMUX_PAD_NANDF_CS1          = (124),
    DDK_IOMUX_PAD_NANDF_CS2          = (125),
    DDK_IOMUX_PAD_NANDF_CS3          = (126),
    DDK_IOMUX_PAD_NANDF_CS4          = (127),
    DDK_IOMUX_PAD_NANDF_CS5          = (128),
    DDK_IOMUX_PAD_NANDF_CS6          = (129),
    DDK_IOMUX_PAD_NANDF_CS7          = (130),
    DDK_IOMUX_PAD_NANDF_RDY_INT      = (131),
    DDK_IOMUX_PAD_NANDF_D15          = (132),
    DDK_IOMUX_PAD_NANDF_D14          = (133),
    DDK_IOMUX_PAD_NANDF_D13          = (134),
    DDK_IOMUX_PAD_NANDF_D12          = (135),
    DDK_IOMUX_PAD_NANDF_D11          = (136),
    DDK_IOMUX_PAD_NANDF_D10          = (137),
    DDK_IOMUX_PAD_NANDF_D9           = (138),
    DDK_IOMUX_PAD_NANDF_D8           = (139),
    DDK_IOMUX_PAD_NANDF_D7           = (140),
    DDK_IOMUX_PAD_NANDF_D6           = (141),
    DDK_IOMUX_PAD_NANDF_D5           = (142),
    DDK_IOMUX_PAD_NANDF_D4           = (143),
    DDK_IOMUX_PAD_NANDF_D3           = (144),
    DDK_IOMUX_PAD_NANDF_D2           = (145),
    DDK_IOMUX_PAD_NANDF_D1           = (146),
    DDK_IOMUX_PAD_NANDF_D0           = (147),
    DDK_IOMUX_PAD_CSI1_D8            = (148),
    DDK_IOMUX_PAD_CSI1_D9            = (149),
    DDK_IOMUX_PAD_CSI1_D10           = (150),
    DDK_IOMUX_PAD_CSI1_D11           = (151),
    DDK_IOMUX_PAD_CSI1_D12           = (152),
    DDK_IOMUX_PAD_CSI1_D13           = (153),
    DDK_IOMUX_PAD_CSI1_D14           = (154),
    DDK_IOMUX_PAD_CSI1_D15           = (155),
    DDK_IOMUX_PAD_CSI1_D16           = (156),
    DDK_IOMUX_PAD_CSI1_D17           = (157),
    DDK_IOMUX_PAD_CSI1_D18           = (158),
    DDK_IOMUX_PAD_CSI1_D19           = (159),
    DDK_IOMUX_PAD_CSI1_VSYNC         = (160),
    DDK_IOMUX_PAD_CSI1_HSYNC         = (161),
    DDK_IOMUX_PAD_CSI1_PIXCLK        = (162),
    DDK_IOMUX_PAD_CSI1_MCLK          = (163),
    DDK_IOMUX_PAD_CSI2_D12           = (164),
    DDK_IOMUX_PAD_CSI2_D13           = (165),
    DDK_IOMUX_PAD_CSI2_D14           = (166),
    DDK_IOMUX_PAD_CSI2_D15           = (167),
    DDK_IOMUX_PAD_CSI2_D16           = (168),
    DDK_IOMUX_PAD_CSI2_D17           = (169),
    DDK_IOMUX_PAD_CSI2_D18           = (170),
    DDK_IOMUX_PAD_CSI2_D19           = (171),
    DDK_IOMUX_PAD_CSI2_VSYNC         = (172),
    DDK_IOMUX_PAD_CSI2_HSYNC         = (173),
    DDK_IOMUX_PAD_CSI2_PIXCLK        = (174),
    DDK_IOMUX_PAD_I2C1_CLK           = (175),
    DDK_IOMUX_PAD_I2C1_DAT           = (176),
    DDK_IOMUX_PAD_AUD3_BB_TXD        = (177),
    DDK_IOMUX_PAD_AUD3_BB_RXD        = (178),
    DDK_IOMUX_PAD_AUD3_BB_CK         = (179),
    DDK_IOMUX_PAD_AUD3_BB_FS         = (180),
    DDK_IOMUX_PAD_CSPI1_MOSI         = (181),
    DDK_IOMUX_PAD_CSPI1_MISO         = (182),
    DDK_IOMUX_PAD_CSPI1_SS0          = (183),
    DDK_IOMUX_PAD_CSPI1_SS1          = (184),
    DDK_IOMUX_PAD_CSPI1_RDY          = (185),
    DDK_IOMUX_PAD_CSPI1_SCLK         = (186),
    DDK_IOMUX_PAD_UART1_RXD          = (187),
    DDK_IOMUX_PAD_UART1_TXD          = (188),
    DDK_IOMUX_PAD_UART1_RTS          = (189),
    DDK_IOMUX_PAD_UART1_CTS          = (190),
    DDK_IOMUX_PAD_UART2_RXD          = (191),
    DDK_IOMUX_PAD_UART2_TXD          = (192),
    DDK_IOMUX_PAD_UART3_RXD          = (193),
    DDK_IOMUX_PAD_UART3_TXD          = (194),
    DDK_IOMUX_PAD_OWIRE_LINE         = (195),
    DDK_IOMUX_PAD_KEY_ROW0           = (196),
    DDK_IOMUX_PAD_KEY_ROW1           = (197),
    DDK_IOMUX_PAD_KEY_ROW2           = (198),
    DDK_IOMUX_PAD_KEY_ROW3           = (199),
    DDK_IOMUX_PAD_KEY_COL0           = (200),
    DDK_IOMUX_PAD_KEY_COL1           = (201),
    DDK_IOMUX_PAD_KEY_COL2           = (202),
    DDK_IOMUX_PAD_KEY_COL3           = (203),
    DDK_IOMUX_PAD_KEY_COL4           = (204),
    DDK_IOMUX_PAD_KEY_COL5           = (205),
    DDK_IOMUX_PAD_JTAG_TCK           = (206),
    DDK_IOMUX_PAD_JTAG_TMS           = (207),
    DDK_IOMUX_PAD_JTAG_TDI           = (208),
    DDK_IOMUX_PAD_JTAG_TRSTB         = (209),
    DDK_IOMUX_PAD_JTAG_MOD           = (210),
    DDK_IOMUX_PAD_USBH1_CLK          = (211),
    DDK_IOMUX_PAD_USBH1_DIR          = (212),
    DDK_IOMUX_PAD_USBH1_STP          = (213),
    DDK_IOMUX_PAD_USBH1_NXT          = (214),
    DDK_IOMUX_PAD_USBH1_DATA0        = (215),
    DDK_IOMUX_PAD_USBH1_DATA1        = (216),
    DDK_IOMUX_PAD_USBH1_DATA2        = (217),
    DDK_IOMUX_PAD_USBH1_DATA3        = (218),
    DDK_IOMUX_PAD_USBH1_DATA4        = (219),
    DDK_IOMUX_PAD_USBH1_DATA5        = (220),
    DDK_IOMUX_PAD_USBH1_DATA6        = (221),
    DDK_IOMUX_PAD_USBH1_DATA7        = (222),
    DDK_IOMUX_PAD_DI1_PIN11          = (223),
    DDK_IOMUX_PAD_DI1_PIN12          = (224),
    DDK_IOMUX_PAD_DI1_PIN13          = (225),
    DDK_IOMUX_PAD_DI1_D0_CS          = (226),
    DDK_IOMUX_PAD_DI1_D1_CS          = (227),
    DDK_IOMUX_PAD_DISPB2_SER_DIN     = (228),
    DDK_IOMUX_PAD_DISPB2_SER_DIO     = (229),
    DDK_IOMUX_PAD_DISPB2_SER_CLK     = (230),
    DDK_IOMUX_PAD_DISPB2_SER_RS      = (231),
    DDK_IOMUX_PAD_DISP1_DAT0         = (232),
    DDK_IOMUX_PAD_DISP1_DAT1         = (233),
    DDK_IOMUX_PAD_DISP1_DAT2         = (234),
    DDK_IOMUX_PAD_DISP1_DAT3         = (235),
    DDK_IOMUX_PAD_DISP1_DAT4         = (236),
    DDK_IOMUX_PAD_DISP1_DAT5         = (237),
    DDK_IOMUX_PAD_DISP1_DAT6         = (238),
    DDK_IOMUX_PAD_DISP1_DAT7         = (239),
    DDK_IOMUX_PAD_DISP1_DAT8         = (240),
    DDK_IOMUX_PAD_DISP1_DAT9         = (241),
    DDK_IOMUX_PAD_DISP1_DAT10        = (242),
    DDK_IOMUX_PAD_DISP1_DAT11        = (243),
    DDK_IOMUX_PAD_DISP1_DAT12        = (244),
    DDK_IOMUX_PAD_DISP1_DAT13        = (245),
    DDK_IOMUX_PAD_DISP1_DAT14        = (246),
    DDK_IOMUX_PAD_DISP1_DAT15        = (247),
    DDK_IOMUX_PAD_DISP1_DAT16        = (248),
    DDK_IOMUX_PAD_DISP1_DAT17        = (249),
    DDK_IOMUX_PAD_DISP1_DAT18        = (250),
    DDK_IOMUX_PAD_DISP1_DAT19        = (251),
    DDK_IOMUX_PAD_DISP1_DAT20        = (252),
    DDK_IOMUX_PAD_DISP1_DAT21        = (253),
    DDK_IOMUX_PAD_DISP1_DAT22        = (254),
    DDK_IOMUX_PAD_DISP1_DAT23        = (255),
    DDK_IOMUX_PAD_DI1_PIN3           = (256),
    DDK_IOMUX_PAD_DI1_DISP_CLK       = (257),
    DDK_IOMUX_PAD_DI1_PIN2           = (258),
    DDK_IOMUX_PAD_DI1_PIN15          = (259),
    DDK_IOMUX_PAD_DI_GP1             = (260),
    DDK_IOMUX_PAD_DI_GP2             = (261),
    DDK_IOMUX_PAD_DI_GP3             = (262),
    DDK_IOMUX_PAD_DI2_PIN4           = (263),
    DDK_IOMUX_PAD_DI2_PIN2           = (264),
    DDK_IOMUX_PAD_DI2_PIN3           = (265),
    DDK_IOMUX_PAD_DI2_DISP_CLK       = (266),
    DDK_IOMUX_PAD_DI_GP4             = (267),
    DDK_IOMUX_PAD_DISP2_DAT0         = (268),
    DDK_IOMUX_PAD_DISP2_DAT1         = (269),
    DDK_IOMUX_PAD_DISP2_DAT2         = (270),
    DDK_IOMUX_PAD_DISP2_DAT3         = (271),
    DDK_IOMUX_PAD_DISP2_DAT4         = (272),
    DDK_IOMUX_PAD_DISP2_DAT5         = (273),
    DDK_IOMUX_PAD_DISP2_DAT6         = (274),
    DDK_IOMUX_PAD_DISP2_DAT7         = (275),
    DDK_IOMUX_PAD_DISP2_DAT8         = (276),
    DDK_IOMUX_PAD_DISP2_DAT9         = (277),
    DDK_IOMUX_PAD_DISP2_DAT10        = (278),
    DDK_IOMUX_PAD_DISP2_DAT11        = (279),
    DDK_IOMUX_PAD_DISP2_DAT12        = (280),
    DDK_IOMUX_PAD_DISP2_DAT13        = (281),
    DDK_IOMUX_PAD_DISP2_DAT14        = (282),
    DDK_IOMUX_PAD_DISP2_DAT15        = (283),
    DDK_IOMUX_PAD_SD1_CMD            = (284),
    DDK_IOMUX_PAD_SD1_CLK            = (285),
    DDK_IOMUX_PAD_SD1_DATA0          = (286),
    DDK_IOMUX_PAD_SD1_DATA1          = (287),
    DDK_IOMUX_PAD_SD1_DATA2          = (288),
    DDK_IOMUX_PAD_SD1_DATA3          = (289),
    DDK_IOMUX_PAD_GPIO1_0            = (290),
    DDK_IOMUX_PAD_GPIO1_1            = (291),
    DDK_IOMUX_PAD_SD2_CMD            = (292),
    DDK_IOMUX_PAD_SD2_CLK            = (293),
    DDK_IOMUX_PAD_SD2_DATA0          = (294),
    DDK_IOMUX_PAD_SD2_DATA1          = (295),
    DDK_IOMUX_PAD_SD2_DATA2          = (296),
    DDK_IOMUX_PAD_SD2_DATA3          = (297),
    DDK_IOMUX_PAD_GPIO1_2            = (298),
    DDK_IOMUX_PAD_GPIO1_3            = (299),
    DDK_IOMUX_PAD_PMIC_STBY_REQ      = (300),
    DDK_IOMUX_PAD_PMIC_ON_REQ        = (301),
    DDK_IOMUX_PAD_PMIC_INT_REQ       = (302),
    DDK_IOMUX_PAD_GPIO1_4            = (303),
    DDK_IOMUX_PAD_GPIO1_5            = (304),
    DDK_IOMUX_PAD_GPIO1_6            = (305),
    DDK_IOMUX_PAD_GPIO1_7            = (306),
    DDK_IOMUX_PAD_GPIO1_8            = (307),
    DDK_IOMUX_PAD_GPIO1_9            = (308), 
    DDK_IOMUX_PAD_GRP_CSI2_PKE0      = (309),
    DDK_IOMUX_PAD_GRP_EIM_SR1        = (310),
    DDK_IOMUX_PAD_GRP_DISP2_PKE0     = (311),  
    DDK_IOMUX_PAD_GRP_DRAM_B4        = (312),
    DDK_IOMUX_PAD_GRP_EIM_SR2        = (313),
    DDK_IOMUX_PAD_GRP_DDR_A0         = (314),
    DDK_IOMUX_PAD_GRP_EMI_PKE0       = (315),
    DDK_IOMUX_PAD_GRP_EIM_SR3        = (316),
    DDK_IOMUX_PAD_GRP_DDR_A1         = (317),
    DDK_IOMUX_PAD_GRP_EIM_SR4        = (318),
    DDK_IOMUX_PAD_GRP_EIM_SR5        = (319),
    DDK_IOMUX_PAD_GRP_EIM_SR6        = (320),
    DDK_IOMUX_PAD_GRP_CSI1_PKE0      = (321),
    DDK_IOMUX_PAD_GRP_DISP1_PKE0     = (322),
    DDK_IOMUX_PAD_GRP_EIM_DS1        = (323),
    DDK_IOMUX_PAD_GRP_EIM_DS2        = (324),
    DDK_IOMUX_PAD_GRP_EIM_DS3        = (325),
    DDK_IOMUX_PAD_GRP_DRAM_B0        = (326),
    DDK_IOMUX_PAD_GRP_EIM_DS4        = (327),
    DDK_IOMUX_PAD_GRP_DRAM_B1        = (328),
    DDK_IOMUX_PAD_GRP_EMI_DS5        = (329), 
    DDK_IOMUX_PAD_GRP_DRAM_B2        = (330), 
    DDK_IOMUX_PAD_GRP_EMI_DS6        = (331)     
} DDK_IOMUX_PAD;
          
          
//------------------------------------------------------------------------------
// Functions

BOOL DDKIomuxSetPinMux(DDK_IOMUX_PIN pin, DDK_IOMUX_PIN_MUXMODE muxmode, 
    DDK_IOMUX_PIN_SION sion);
BOOL DDKIomuxGetPinMux(DDK_IOMUX_PIN pin, DDK_IOMUX_PIN_MUXMODE *pMuxmode, 
    DDK_IOMUX_PIN_SION *pSion);
BOOL DDKIomuxSetPadConfig(DDK_IOMUX_PAD pad, DDK_IOMUX_PAD_SLEW slew, 
    DDK_IOMUX_PAD_DRIVE drive, DDK_IOMUX_PAD_OPENDRAIN openDrain, 
    DDK_IOMUX_PAD_PULL pull, DDK_IOMUX_PAD_HYSTERESIS hysteresis, 
    DDK_IOMUX_PAD_INMODE inputMode, DDK_IOMUX_PAD_OUTVOLT outputVolt);
BOOL DDKIomuxGetPadConfig(DDK_IOMUX_PAD pad, DDK_IOMUX_PAD_SLEW *pSlew, 
    DDK_IOMUX_PAD_DRIVE *pDrive, DDK_IOMUX_PAD_OPENDRAIN *pOpenDrain, 
    DDK_IOMUX_PAD_PULL *pPull, DDK_IOMUX_PAD_HYSTERESIS *pHysteresis, 
    DDK_IOMUX_PAD_INMODE *pInputMode, DDK_IOMUX_PAD_OUTVOLT *pOutputVolt);
BOOL DDKIomuxSetGpr0(UINT32 data);
BOOL DDKIomuxSetGpr1(UINT32 data);
UINT32 DDKIomuxGetGpr0(VOID);
UINT32 DDKIomuxGetGpr1(VOID);
BOOL DDKIomuxSelectInput(DDK_IOMUX_SELECT_INPUT port, UINT32 daisy);


#if __cplusplus
}
#endif

#endif // __MX51_TO1_DDK_H
