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
//  File:  MX31_ddk.h
//
//  Contains MX31 definitions to assist with driver development.
//
//------------------------------------------------------------------------------
#ifndef __MX31_DDK_H
#define __MX31_DDK_H

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
    DDK_CLOCK_SIGNAL_USBPLL             = 1,        
    DDK_CLOCK_SIGNAL_SERPLL             = 2,
    DDK_CLOCK_SIGNAL_ARM                = 3,
    DDK_CLOCK_SIGNAL_IPU                = 4,
    DDK_CLOCK_SIGNAL_AHB                = 5,
    DDK_CLOCK_SIGNAL_IPG                = 6,
    DDK_CLOCK_SIGNAL_NFC                = 7,
    DDK_CLOCK_SIGNAL_GACC               = 8,
    DDK_CLOCK_SIGNAL_SSI1               = 9,
    DDK_CLOCK_SIGNAL_SSI2               = 10,
    DDK_CLOCK_SIGNAL_FIRI               = 11,
    DDK_CLOCK_SIGNAL_CSI                = 12,
    DDK_CLOCK_SIGNAL_USB                = 13,
    DDK_CLOCK_SIGNAL_SIM                = 14,
    DDK_CLOCK_SIGNAL_PER                = 15,
    DDK_CLOCK_SIGNAL_ENUM_END           = 16
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
    DDK_CLOCK_GATE_INDEX_SDHC1          = 0,    // CGR0[1:0]
    DDK_CLOCK_GATE_INDEX_SDHC2          = 1,    // CGR0[3:2]
    DDK_CLOCK_GATE_INDEX_GPT            = 2,    // CGR0[5:4]
    DDK_CLOCK_GATE_INDEX_EPIT1          = 3,    // CGR0[7:6]
    DDK_CLOCK_GATE_INDEX_EPIT2          = 4,    // CGR0[9:8]
    DDK_CLOCK_GATE_INDEX_IIM            = 5,    // CGR0[11:10]
    DDK_CLOCK_GATE_INDEX_ATA            = 6,    // CGR0[13:12]
    DDK_CLOCK_GATE_INDEX_SDMA           = 7,    // CGR0[15:14]
    DDK_CLOCK_GATE_INDEX_CSPI3          = 8,    // CGR0[17:16]
    DDK_CLOCK_GATE_INDEX_RNG            = 9,    // CGR0[19:18]
    DDK_CLOCK_GATE_INDEX_UART1          = 10,   // CGR0[21:20]
    DDK_CLOCK_GATE_INDEX_UART2          = 11,   // CGR0[23:22]
    DDK_CLOCK_GATE_INDEX_SSI1           = 12,   // CGR0[25:24]
    DDK_CLOCK_GATE_INDEX_I2C1           = 13,   // CGR0[27:26]
    DDK_CLOCK_GATE_INDEX_I2C2           = 14,   // CGR0[29:28]
    DDK_CLOCK_GATE_INDEX_I2C3           = 15,   // CGR0[31:30]

    DDK_CLOCK_GATE_INDEX_MPEG4          = 16,   // CGR1[1:0]
    DDK_CLOCK_GATE_INDEX_MEMSTICK1      = 17,   // CGR1[3:2]
    DDK_CLOCK_GATE_INDEX_MEMSTICK2      = 18,   // CGR1[5:4]
    DDK_CLOCK_GATE_INDEX_CSI            = 19,   // CGR1[7:6]
    DDK_CLOCK_GATE_INDEX_RTC            = 20,   // CGR1[9:8]
    DDK_CLOCK_GATE_INDEX_WDOG           = 21,   // CGR1[11:10]
    DDK_CLOCK_GATE_INDEX_PWM            = 22,   // CGR1[13:12]
    DDK_CLOCK_GATE_INDEX_SIM            = 23,   // CGR1[15:14]
    DDK_CLOCK_GATE_INDEX_ECT            = 24,   // CGR1[17:16]
    DDK_CLOCK_GATE_INDEX_USBOTG         = 25,   // CGR1[19:18]
    DDK_CLOCK_GATE_INDEX_KPP            = 26,   // CGR1[21:20]
    DDK_CLOCK_GATE_INDEX_IPU            = 27,   // CGR1[23:22]
    DDK_CLOCK_GATE_INDEX_UART3          = 28,   // CGR1[25:24]
    DDK_CLOCK_GATE_INDEX_UART4          = 29,   // CGR1[27:26]
    DDK_CLOCK_GATE_INDEX_UART5          = 30,   // CGR1[29:28]
    DDK_CLOCK_GATE_INDEX_OWIRE          = 31,   // CGR1[31:30]

    DDK_CLOCK_GATE_INDEX_SSI2           = 32,   // CGR2[1:0]
    DDK_CLOCK_GATE_INDEX_CSPI1          = 33,   // CGR2[3:2]
    DDK_CLOCK_GATE_INDEX_CSPI2          = 34,   // CGR2[5:4]
    DDK_CLOCK_GATE_INDEX_GACC           = 35,   // CGR2[7:6]
    DDK_CLOCK_GATE_INDEX_EMI            = 36,   // CGR2[9:8]
    DDK_CLOCK_GATE_INDEX_RTIC           = 37,   // CGR2[11:10]
    DDK_CLOCK_GATE_INDEX_FIRI           = 38,   // CGR2[13:12]
    
    //DDK_CLOCK_GATE_INDEX_NFC            = 44   // CGR2[23:24]
    DDK_CLOCK_GATE_INDEX_SPECIAL        = 39,   // Start of CGR2 "special cases"
    DDK_CLOCK_GATE_INDEX_NFC            = (DDK_CLOCK_GATE_INDEX_SPECIAL+28),    // CGR2[28]

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
    DDK_CLOCK_BAUD_SOURCE_MCUPLL            = 0,
    DDK_CLOCK_BAUD_SOURCE_USBPLL            = 1,
    DDK_CLOCK_BAUD_SOURCE_SERPLL            = 2,
    DDK_CLOCK_BAUD_SOURCE_NONE              = 3,
} DDK_CLOCK_BAUD_SOURCE;


//-----------------------------------------------------------------------------
//
//  Type: DDK_CLOCK_CKO_SRC
//
//  Clock output source (CKO) signal selections.
//
//-----------------------------------------------------------------------------
typedef enum {
    DDK_CLOCK_CKO_SRC_MCUPLL                = 0,
    DDK_CLOCK_CKO_SRC_IPG                   = 1,
    DDK_CLOCK_CKO_SRC_USBPLL                = 2,
    DDK_CLOCK_CKO_SRC_PLLREF                = 3,
    DDK_CLOCK_CKO_SRC_FPM                   = 4,
    DDK_CLOCK_CKO_SRC_AHB                   = 5,
    DDK_CLOCK_CKO_SRC_ARM                   = 6,
    DDK_CLOCK_CKO_SRC_SPLL                  = 7,
    DDK_CLOCK_CKO_SRC_CKIH                  = 8,
    DDK_CLOCK_CKO_SRC_EMI                   = 9,
    DDK_CLOCK_CKO_SRC_HSP                   = 10,
    DDK_CLOCK_CKO_SRC_NFC                   = 11,
    DDK_CLOCK_CKO_SRC_PER                   = 12
} DDK_CLOCK_CKO_SRC;


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
    DDK_CLOCK_CKO_DIV_4                     = 2,
    DDK_CLOCK_CKO_DIV_8                     = 3,
    DDK_CLOCK_CKO_DIV_16                    = 4,
} DDK_CLOCK_CKO_DIV;


//-----------------------------------------------------------------------------
//
//  Type: DDK_CLOCK_DVFC_MODE
//
//  Clock output source (CKO) divider selections.
//
//-----------------------------------------------------------------------------
typedef enum {
    DDK_CLOCK_DVFC_MODE_NORMAL              = (0x0 << CCM_PMCR1_DVGP_LSH),
    DDK_CLOCK_DVFC_MODE_PANIC               = (0x1 << CCM_PMCR1_DVGP_LSH),
    DDK_CLOCK_DVFC_MODE_SUSPEND             = (0x2 << CCM_PMCR1_DVGP_LSH),
    DDK_CLOCK_DVFC_MODE_BUS_LOW             = (0x4 << CCM_PMCR1_DVGP_LSH)
} DDK_CLOCK_DVFC_MODE;


//-----------------------------------------------------------------------------
//
//  Type: DDK_IOMUX_PIN
//
//  Specifies the functional pin name used to configure the IOMUX.
//
//-----------------------------------------------------------------------------
#ifdef VPMX31
typedef enum {
    DDK_IOMUX_PIN_PAD2                  = (0),
    DDK_IOMUX_PIN_SD1CLK_B              = (8),
    DDK_IOMUX_PIN_CE_CONTROL            = (16),
    DDK_IOMUX_PIN_ATA_RESET_B           = (24),
    DDK_IOMUX_PIN_ATA_DMACK             = (32),
    DDK_IOMUX_PIN_ATA_DIOW              = (40),
    DDK_IOMUX_PIN_ATA_DIOR              = (48),
    DDK_IOMUX_PIN_ATA_CS1               = (56),
    DDK_IOMUX_PIN_ATA_CS0               = (64),
    DDK_IOMUX_PIN_SD1_DATA3             = (72),
    DDK_IOMUX_PIN_SD1_DATA2             = (80),
    DDK_IOMUX_PIN_SD1_DATA1             = (88),
    DDK_IOMUX_PIN_SD1_DATA0             = (96),
    DDK_IOMUX_PIN_SD1_CLK               = (104),
    DDK_IOMUX_PIN_SD1_CMD               = (112),
    DDK_IOMUX_PIN_D3_SPL                = (120),
    DDK_IOMUX_PIN_D3_CLS                = (128),
    DDK_IOMUX_PIN_D3_REV                = (136),
    DDK_IOMUX_PIN_CONTRAST              = (144),
    DDK_IOMUX_PIN_VSYNC3                = (152),
    DDK_IOMUX_PIN_READ                  = (160),
    DDK_IOMUX_PIN_WRITE                 = (168),
    DDK_IOMUX_PIN_PAR_RS                = (176),
    DDK_IOMUX_PIN_SER_RS                = (184),
    DDK_IOMUX_PIN_LCS1                  = (192),
    DDK_IOMUX_PIN_LCS0                  = (200),
    DDK_IOMUX_PIN_SD_D_CLK              = (208),
    DDK_IOMUX_PIN_SD_D_IO               = (216),
    DDK_IOMUX_PIN_SD_D_O                = (224),
    DDK_IOMUX_PIN_DRDY0                 = (232),
    DDK_IOMUX_PIN_FPSHIFT               = (240),
    DDK_IOMUX_PIN_HSYNC                 = (248),
    DDK_IOMUX_PIN_VSYNC0                = (256),
    DDK_IOMUX_PIN_LD17                  = (264),
    DDK_IOMUX_PIN_LD16                  = (272),
    DDK_IOMUX_PIN_LD15                  = (280),
    DDK_IOMUX_PIN_LD14                  = (288),
    DDK_IOMUX_PIN_LD13                  = (296),
    DDK_IOMUX_PIN_LD12                  = (304),
    DDK_IOMUX_PIN_LD11                  = (312),
    DDK_IOMUX_PIN_LD10                  = (320),
    DDK_IOMUX_PIN_LD9                   = (328),
    DDK_IOMUX_PIN_LD8                   = (336),
    DDK_IOMUX_PIN_LD7                   = (344),
    DDK_IOMUX_PIN_LD6                   = (352),
    DDK_IOMUX_PIN_LD5                   = (360),
    DDK_IOMUX_PIN_LD4                   = (368),
    DDK_IOMUX_PIN_LD3                   = (376),
    DDK_IOMUX_PIN_LD2                   = (384),
    DDK_IOMUX_PIN_LD1                   = (392),
    DDK_IOMUX_PIN_LD0                   = (400),
    DDK_IOMUX_PIN_USBH2_DATA1           = (408),
    DDK_IOMUX_PIN_USBH2_DATA0           = (416),
    DDK_IOMUX_PIN_USBH2_NXT             = (424),
    DDK_IOMUX_PIN_USBH2_STP             = (432),
    DDK_IOMUX_PIN_USBH2_DIR             = (440),
    DDK_IOMUX_PIN_USBH2_CLK             = (448),
    DDK_IOMUX_PIN_USBOTG_DATA7          = (456),
    DDK_IOMUX_PIN_USBOTG_DATA6          = (464),
    DDK_IOMUX_PIN_USBOTG_DATA5          = (472),
    DDK_IOMUX_PIN_USBOTG_DATA4          = (480),
    DDK_IOMUX_PIN_USBOTG_DATA3          = (488),
    DDK_IOMUX_PIN_USBOTG_DATA2          = (496),
    DDK_IOMUX_PIN_USBOTG_DATA1          = (504),
    DDK_IOMUX_PIN_USBOTG_DATA0          = (512),
    DDK_IOMUX_PIN_USBOTG_NXT            = (520),
    DDK_IOMUX_PIN_USBOTG_STP            = (528),
    DDK_IOMUX_PIN_USBOTG_DIR            = (536),
    DDK_IOMUX_PIN_USBOTG_CLK            = (544),
    DDK_IOMUX_PIN_USB_BYP               = (552),
    DDK_IOMUX_PIN_USB_OC                = (560),
    DDK_IOMUX_PIN_USB_PWR               = (568),
    DDK_IOMUX_PIN_SJC_MOD               = (576),
    DDK_IOMUX_PIN_DE_B                  = (584),
    DDK_IOMUX_PIN_TRSTB                 = (592),
    DDK_IOMUX_PIN_TDO                   = (600),
    DDK_IOMUX_PIN_TDI                   = (608),
    DDK_IOMUX_PIN_TMS                   = (616),
    DDK_IOMUX_PIN_TCK                   = (624),
    DDK_IOMUX_PIN_RTCK                  = (632),
    DDK_IOMUX_PIN_KEY_COL7              = (640),
    DDK_IOMUX_PIN_KEY_COL6              = (648),
    DDK_IOMUX_PIN_KEY_COL5              = (656),
    DDK_IOMUX_PIN_KEY_COL4              = (664),
    DDK_IOMUX_PIN_KEY_COL3              = (672),
    DDK_IOMUX_PIN_KEY_COL2              = (680),
    DDK_IOMUX_PIN_KEY_COL1              = (688),
    DDK_IOMUX_PIN_KEY_COL0              = (696),
    DDK_IOMUX_PIN_KEY_ROW7              = (704),
    DDK_IOMUX_PIN_KEY_ROW6              = (712),
    DDK_IOMUX_PIN_KEY_ROW5              = (720),
    DDK_IOMUX_PIN_KEY_ROW4              = (728),
    DDK_IOMUX_PIN_KEY_ROW3              = (736),
    DDK_IOMUX_PIN_KEY_ROW2              = (744),
    DDK_IOMUX_PIN_KEY_ROW1              = (752),
    DDK_IOMUX_PIN_KEY_ROW0              = (760),
    DDK_IOMUX_PIN_BATT_LINE             = (768),
    DDK_IOMUX_PIN_CTS2                  = (776),
    DDK_IOMUX_PIN_RTS2                  = (784),
    DDK_IOMUX_PIN_TXD2                  = (792),
    DDK_IOMUX_PIN_RXD2                  = (800),
    DDK_IOMUX_PIN_DTR_DCE2              = (808),
    DDK_IOMUX_PIN_DCD_DTE1              = (816),
    DDK_IOMUX_PIN_RI_DTE1               = (824),
    DDK_IOMUX_PIN_DSR_DTE1              = (832),
    DDK_IOMUX_PIN_DTR_DTE1              = (840),
    DDK_IOMUX_PIN_DCD_DCE1              = (848),
    DDK_IOMUX_PIN_RI_DCE1               = (856),
    DDK_IOMUX_PIN_DSR_DCE1              = (864),
    DDK_IOMUX_PIN_DTR_DCE1              = (872),
    DDK_IOMUX_PIN_CTS1                  = (880),
    DDK_IOMUX_PIN_RTS1                  = (888),
    DDK_IOMUX_PIN_TXD1                  = (896),
    DDK_IOMUX_PIN_RXD1                  = (904),
    DDK_IOMUX_PIN_CSPI2_SPI_RDY         = (912),
    DDK_IOMUX_PIN_CSPI2_SCLK            = (920),
    DDK_IOMUX_PIN_CSPI2_SS2             = (928),
    DDK_IOMUX_PIN_CSPI2_SS1             = (936),
    DDK_IOMUX_PIN_CSPI2_SS0             = (944),
    DDK_IOMUX_PIN_CSPI2_MISO            = (952),
    DDK_IOMUX_PIN_CSPI2_MOSI            = (960),
    DDK_IOMUX_PIN_CSPI1_SPI_RDY         = (968),
    DDK_IOMUX_PIN_CSPI1_SCLK            = (976),
    DDK_IOMUX_PIN_CSPI1_SS2             = (984),
    DDK_IOMUX_PIN_CSPI1_SS1             = (992),
    DDK_IOMUX_PIN_CSPI1_SS0             = (1000),
    DDK_IOMUX_PIN_CSPI1_MISO            = (1008),
    DDK_IOMUX_PIN_CSPI1_MOSI            = (1016),
    DDK_IOMUX_PIN_SFS6                  = (1024),
    DDK_IOMUX_PIN_SCK6                  = (1032),
    DDK_IOMUX_PIN_SRXD6                 = (1040),
    DDK_IOMUX_PIN_STXD6                 = (1048),
    DDK_IOMUX_PIN_SFS5                  = (1056),
    DDK_IOMUX_PIN_SCK5                  = (1064),
    DDK_IOMUX_PIN_SRXD5                 = (1072),
    DDK_IOMUX_PIN_STXD5                 = (1080),
    DDK_IOMUX_PIN_SFS4                  = (1088),
    DDK_IOMUX_PIN_SCK4                  = (1096),
    DDK_IOMUX_PIN_SRXD4                 = (1104),
    DDK_IOMUX_PIN_STXD4                 = (1112),
    DDK_IOMUX_PIN_SFS3                  = (1120),
    DDK_IOMUX_PIN_SCK3                  = (1128),
    DDK_IOMUX_PIN_SRXD3                 = (1136),
    DDK_IOMUX_PIN_STXD3                 = (1144),
    DDK_IOMUX_PIN_I2C_DAT               = (1152),
    DDK_IOMUX_PIN_I2C_CLK               = (1160),
    DDK_IOMUX_PIN_CSI_PIXCLK            = (1168),
    DDK_IOMUX_PIN_CSI_HSYNC             = (1176),
    DDK_IOMUX_PIN_CSI_VSYNC             = (1184),
    DDK_IOMUX_PIN_CSI_MCLK              = (1192),
    DDK_IOMUX_PIN_CSI_D15               = (1200),
    DDK_IOMUX_PIN_CSI_D14               = (1208),
    DDK_IOMUX_PIN_CSI_D13               = (1216),
    DDK_IOMUX_PIN_CSI_D12               = (1224),
    DDK_IOMUX_PIN_CSI_D11               = (1232),
    DDK_IOMUX_PIN_CSI_D10               = (1240),
    DDK_IOMUX_PIN_CSI_D9                = (1248),
    DDK_IOMUX_PIN_CSI_D8                = (1256),
    DDK_IOMUX_PIN_CSI_D7                = (1264),
    DDK_IOMUX_PIN_CSI_D6                = (1272),
    DDK_IOMUX_PIN_CSI_D5                = (1280),
    DDK_IOMUX_PIN_CSI_D4                = (1288),
    DDK_IOMUX_PIN_M_GRANT               = (1296),
    DDK_IOMUX_PIN_M_REQUEST             = (1304),
    DDK_IOMUX_PIN_PC_POE                = (1312),
    DDK_IOMUX_PIN_PC_RW_B               = (1320),
    DDK_IOMUX_PIN_IOIS16                = (1328),
    DDK_IOMUX_PIN_PC_RST                = (1336),
    DDK_IOMUX_PIN_PC_BVD2               = (1344),
    DDK_IOMUX_PIN_PC_BVD1               = (1352),
    DDK_IOMUX_PIN_PC_VS2                = (1360),
    DDK_IOMUX_PIN_PC_VS1                = (1368),
    DDK_IOMUX_PIN_PC_PWRON              = (1376),
    DDK_IOMUX_PIN_PC_READY              = (1384),
    DDK_IOMUX_PIN_PC_WAIT_B             = (1392),
    DDK_IOMUX_PIN_PC_CD2_B              = (1400),
    DDK_IOMUX_PIN_PC_CD1_B              = (1408),
    DDK_IOMUX_PIN_D0                    = (1416),
    DDK_IOMUX_PIN_D1                    = (1424),
    DDK_IOMUX_PIN_D2                    = (1432),
    DDK_IOMUX_PIN_D3                    = (1440),
    DDK_IOMUX_PIN_D4                    = (1448),
    DDK_IOMUX_PIN_D5                    = (1456),
    DDK_IOMUX_PIN_D6                    = (1464),
    DDK_IOMUX_PIN_D7                    = (1472),
    DDK_IOMUX_PIN_D8                    = (1480),
    DDK_IOMUX_PIN_D9                    = (1488),
    DDK_IOMUX_PIN_D10                   = (1496),
    DDK_IOMUX_PIN_D11                   = (1504),
    DDK_IOMUX_PIN_D12                   = (1512),
    DDK_IOMUX_PIN_D13                   = (1520),
    DDK_IOMUX_PIN_D14                   = (1528),
    DDK_IOMUX_PIN_D15                   = (1536),
    DDK_IOMUX_PIN_NFRB                  = (1544),
    DDK_IOMUX_PIN_NFCE_B                = (1552),
    DDK_IOMUX_PIN_NFWP_B                = (1560),
    DDK_IOMUX_PIN_NFCLE                 = (1568),
    DDK_IOMUX_PIN_NFALE                 = (1576),
    DDK_IOMUX_PIN_NFRE_B                = (1584),
    DDK_IOMUX_PIN_NFWE_B                = (1592),
    DDK_IOMUX_PIN_SDQS3                 = (1600),
    DDK_IOMUX_PIN_SDQS2                 = (1608),
    DDK_IOMUX_PIN_SDQS1                 = (1616),
    DDK_IOMUX_PIN_SDQS0                 = (1624),
    DDK_IOMUX_PIN_SD2CLK_B              = (1632),
    DDK_IOMUX_PIN_SDCLK                 = (1640),
    DDK_IOMUX_PIN_SDCKE1                = (1648),
    DDK_IOMUX_PIN_SDCKE0                = (1656),
    DDK_IOMUX_PIN_SDWE                  = (1664),
    DDK_IOMUX_PIN_CAS                   = (1672),
    DDK_IOMUX_PIN_RAS                   = (1680),
    DDK_IOMUX_PIN_RW                    = (1688),
    DDK_IOMUX_PIN_BCLK                  = (1696),
    DDK_IOMUX_PIN_LBA                   = (1704),
    DDK_IOMUX_PIN_ECB                   = (1712),
    DDK_IOMUX_PIN_CS5                   = (1720),
    DDK_IOMUX_PIN_CS4                   = (1728),
    DDK_IOMUX_PIN_CS3                   = (1736),
    DDK_IOMUX_PIN_CS2                   = (1744),
    DDK_IOMUX_PIN_CS1                   = (1752),
    DDK_IOMUX_PIN_CS0                   = (1760),
    DDK_IOMUX_PIN_OE                    = (1768),
    DDK_IOMUX_PIN_EB1                   = (1776),
    DDK_IOMUX_PIN_EB0                   = (1784),
    DDK_IOMUX_PIN_DQM3                  = (1792),
    DDK_IOMUX_PIN_DQM2                  = (1800),
    DDK_IOMUX_PIN_DQM1                  = (1808),
    DDK_IOMUX_PIN_DQM0                  = (1816),
    DDK_IOMUX_PIN_SD31                  = (1824),
    DDK_IOMUX_PIN_SD30                  = (1832),
    DDK_IOMUX_PIN_SD29                  = (1840),
    DDK_IOMUX_PIN_SD28                  = (1848),
    DDK_IOMUX_PIN_SD27                  = (1856),
    DDK_IOMUX_PIN_SD26                  = (1864),
    DDK_IOMUX_PIN_SD25                  = (1872),
    DDK_IOMUX_PIN_SD24                  = (1880),
    DDK_IOMUX_PIN_SD23                  = (1888),
    DDK_IOMUX_PIN_SD22                  = (1896),
    DDK_IOMUX_PIN_SD21                  = (1904),
    DDK_IOMUX_PIN_SD20                  = (1912),
    DDK_IOMUX_PIN_SD19                  = (1920),
    DDK_IOMUX_PIN_SD18                  = (1928),
    DDK_IOMUX_PIN_SD17                  = (1936),
    DDK_IOMUX_PIN_SD16                  = (1944),
    DDK_IOMUX_PIN_SD15                  = (1952),
    DDK_IOMUX_PIN_SD14                  = (1960),
    DDK_IOMUX_PIN_SD13                  = (1968),
    DDK_IOMUX_PIN_SD12                  = (1976),
    DDK_IOMUX_PIN_SD11                  = (1984),
    DDK_IOMUX_PIN_SD10                  = (1992),
    DDK_IOMUX_PIN_SD9                   = (2000),
    DDK_IOMUX_PIN_SD8                   = (2008),
    DDK_IOMUX_PIN_SD7                   = (2016),
    DDK_IOMUX_PIN_SD6                   = (2024),
    DDK_IOMUX_PIN_SD5                   = (2032),
    DDK_IOMUX_PIN_SD4                   = (2040),
    DDK_IOMUX_PIN_SD3                   = (2048),
    DDK_IOMUX_PIN_SD2                   = (2056),
    DDK_IOMUX_PIN_SD1                   = (2064),
    DDK_IOMUX_PIN_SD0                   = (2072),
    DDK_IOMUX_PIN_SDBA0                 = (2080),
    DDK_IOMUX_PIN_SDBA1                 = (2088),
    DDK_IOMUX_PIN_A25                   = (2096),
    DDK_IOMUX_PIN_A24                   = (2104),
    DDK_IOMUX_PIN_A23                   = (2112),
    DDK_IOMUX_PIN_A22                   = (2120),
    DDK_IOMUX_PIN_A21                   = (2128),
    DDK_IOMUX_PIN_A20                   = (2136),
    DDK_IOMUX_PIN_A19                   = (2144),
    DDK_IOMUX_PIN_A18                   = (2152),
    DDK_IOMUX_PIN_A17                   = (2160),
    DDK_IOMUX_PIN_A16                   = (2168),
    DDK_IOMUX_PIN_A15                   = (2176),
    DDK_IOMUX_PIN_A14                   = (2184),
    DDK_IOMUX_PIN_A13                   = (2192),
    DDK_IOMUX_PIN_A12                   = (2200),
    DDK_IOMUX_PIN_A11                   = (2208),
    DDK_IOMUX_PIN_MA10                  = (2216),
    DDK_IOMUX_PIN_A10                   = (2224),
    DDK_IOMUX_PIN_A9                    = (2232),
    DDK_IOMUX_PIN_A8                    = (2240),
    DDK_IOMUX_PIN_A7                    = (2248),
    DDK_IOMUX_PIN_A6                    = (2256),
    DDK_IOMUX_PIN_A5                    = (2264),
    DDK_IOMUX_PIN_A4                    = (2272),
    DDK_IOMUX_PIN_A3                    = (2280),
    DDK_IOMUX_PIN_A2                    = (2288),
    DDK_IOMUX_PIN_A1                    = (2296),
    DDK_IOMUX_PIN_A0                    = (2304),
    DDK_IOMUX_PIN_VPG1                  = (2312),
    DDK_IOMUX_PIN_VPG0                  = (2320),
    DDK_IOMUX_PIN_DVFS1                 = (2328),
    DDK_IOMUX_PIN_DVFS0                 = (2336),
    DDK_IOMUX_PIN_VSTBY                 = (2344),
    DDK_IOMUX_PIN_POWER_FAIL            = (2352),
    DDK_IOMUX_PIN_CKIL                  = (2360),
    DDK_IOMUX_PIN_BOOT_MODE4            = (2368),
    DDK_IOMUX_PIN_BOOT_MODE3            = (2376),
    DDK_IOMUX_PIN_BOOT_MODE2            = (2384),
    DDK_IOMUX_PIN_BOOT_MODE1            = (2392),
    DDK_IOMUX_PIN_BOOT_MODE0            = (2400),
    DDK_IOMUX_PIN_CLKO                  = (2408),
    DDK_IOMUX_PIN_POR_B                 = (2416),
    DDK_IOMUX_PIN_RESET_IN_B            = (2424),
    DDK_IOMUX_PIN_CKIH                  = (2432),
    DDK_IOMUX_PIN_SIMPD0                = (2440),
    DDK_IOMUX_PIN_SRX0                  = (2448),
    DDK_IOMUX_PIN_STX0                  = (2456),
    DDK_IOMUX_PIN_SVEN0                 = (2464),
    DDK_IOMUX_PIN_SRST0                 = (2472),
    DDK_IOMUX_PIN_SCLK0                 = (2480),
    DDK_IOMUX_PIN_GPIO3_1               = (2488),
    DDK_IOMUX_PIN_GPIO3_0               = (2496),
    DDK_IOMUX_PIN_GPIO1_6               = (2504),
    DDK_IOMUX_PIN_GPIO1_5               = (2512),
    DDK_IOMUX_PIN_GPIO1_4               = (2520),
    DDK_IOMUX_PIN_GPIO1_3               = (2528),
    DDK_IOMUX_PIN_GPIO1_2               = (2536),
    DDK_IOMUX_PIN_GPIO1_1               = (2544),
    DDK_IOMUX_PIN_GPIO1_0               = (2552),
    DDK_IOMUX_PIN_PWMO                  = (2560),
    DDK_IOMUX_PIN_WATCHDOG_RST          = (2568),
    DDK_IOMUX_PIN_COMPARE               = (2576),
    DDK_IOMUX_PIN_CAPTURE               = (2584)
} DDK_IOMUX_PIN;
#else
typedef enum {
    DDK_IOMUX_PIN_TTM                   = (0),
    DDK_IOMUX_PIN_CSPI3_SPI_RDY         = (8),
    DDK_IOMUX_PIN_CSPI3_SCLK            = (16),
    DDK_IOMUX_PIN_CSPI3_MISO            = (24),
    DDK_IOMUX_PIN_CSPI3_MOSI            = (32),
    DDK_IOMUX_PIN_CLKSS                 = (40),
    DDK_IOMUX_PIN_CE_CONTROL            = (48),
    DDK_IOMUX_PIN_ATA_RESET_B           = (56),
    DDK_IOMUX_PIN_ATA_DMACK             = (64),
    DDK_IOMUX_PIN_ATA_DIOW              = (72),
    DDK_IOMUX_PIN_ATA_DIOR              = (80),
    DDK_IOMUX_PIN_ATA_CS1               = (88),
    DDK_IOMUX_PIN_ATA_CS0               = (96),
    DDK_IOMUX_PIN_SD1_DATA3             = (104),
    DDK_IOMUX_PIN_SD1_DATA2             = (112),
    DDK_IOMUX_PIN_SD1_DATA1             = (120),
    DDK_IOMUX_PIN_SD1_DATA0             = (128),
    DDK_IOMUX_PIN_SD1_CLK               = (136),
    DDK_IOMUX_PIN_SD1_CMD               = (144),
    DDK_IOMUX_PIN_D3_SPL                = (152),
    DDK_IOMUX_PIN_D3_CLS                = (160),
    DDK_IOMUX_PIN_D3_REV                = (168),
    DDK_IOMUX_PIN_CONTRAST              = (176),
    DDK_IOMUX_PIN_VSYNC3                = (184),
    DDK_IOMUX_PIN_READ                  = (192),
    DDK_IOMUX_PIN_WRITE                 = (200),
    DDK_IOMUX_PIN_PAR_RS                = (208),
    DDK_IOMUX_PIN_SER_RS                = (216),
    DDK_IOMUX_PIN_LCS1                  = (224),
    DDK_IOMUX_PIN_LCS0                  = (232),
    DDK_IOMUX_PIN_SD_D_CLK              = (240),
    DDK_IOMUX_PIN_SD_D_IO               = (248),
    DDK_IOMUX_PIN_SD_D_I                = (256),
    DDK_IOMUX_PIN_DRDY0                 = (264),
    DDK_IOMUX_PIN_FPSHIFT               = (272),
    DDK_IOMUX_PIN_HSYNC                 = (280),
    DDK_IOMUX_PIN_VSYNC0                = (288),
    DDK_IOMUX_PIN_LD17                  = (296),
    DDK_IOMUX_PIN_LD16                  = (304),
    DDK_IOMUX_PIN_LD15                  = (312),
    DDK_IOMUX_PIN_LD14                  = (320),
    DDK_IOMUX_PIN_LD13                  = (328),
    DDK_IOMUX_PIN_LD12                  = (336),
    DDK_IOMUX_PIN_LD11                  = (344),
    DDK_IOMUX_PIN_LD10                  = (352),
    DDK_IOMUX_PIN_LD9                   = (360),
    DDK_IOMUX_PIN_LD8                   = (368),
    DDK_IOMUX_PIN_LD7                   = (376),
    DDK_IOMUX_PIN_LD6                   = (384),
    DDK_IOMUX_PIN_LD5                   = (392),
    DDK_IOMUX_PIN_LD4                   = (400),
    DDK_IOMUX_PIN_LD3                   = (408),
    DDK_IOMUX_PIN_LD2                   = (416),
    DDK_IOMUX_PIN_LD1                   = (424),
    DDK_IOMUX_PIN_LD0                   = (432),
    DDK_IOMUX_PIN_USBH2_DATA1           = (440),
    DDK_IOMUX_PIN_USBH2_DATA0           = (448),
    DDK_IOMUX_PIN_USBH2_NXT             = (456),
    DDK_IOMUX_PIN_USBH2_STP             = (464),
    DDK_IOMUX_PIN_USBH2_DIR             = (472),
    DDK_IOMUX_PIN_USBH2_CLK             = (480),
    DDK_IOMUX_PIN_USBOTG_DATA7          = (488),
    DDK_IOMUX_PIN_USBOTG_DATA6          = (496),
    DDK_IOMUX_PIN_USBOTG_DATA5          = (504),
    DDK_IOMUX_PIN_USBOTG_DATA4          = (512),
    DDK_IOMUX_PIN_USBOTG_DATA3          = (520),
    DDK_IOMUX_PIN_USBOTG_DATA2          = (528),
    DDK_IOMUX_PIN_USBOTG_DATA1          = (536),
    DDK_IOMUX_PIN_USBOTG_DATA0          = (544),
    DDK_IOMUX_PIN_USBOTG_NXT            = (552),
    DDK_IOMUX_PIN_USBOTG_STP            = (560),
    DDK_IOMUX_PIN_USBOTG_DIR            = (568),
    DDK_IOMUX_PIN_USBOTG_CLK            = (576),
    DDK_IOMUX_PIN_USB_BYP               = (584),
    DDK_IOMUX_PIN_USB_OC                = (592),
    DDK_IOMUX_PIN_USB_PWR               = (600),
    DDK_IOMUX_PIN_SJC_MOD               = (608),
    DDK_IOMUX_PIN_DE_B                  = (616),
    DDK_IOMUX_PIN_TRSTB                 = (624),
    DDK_IOMUX_PIN_TDO                   = (632),
    DDK_IOMUX_PIN_TDI                   = (640),
    DDK_IOMUX_PIN_TMS                   = (648),
    DDK_IOMUX_PIN_TCK                   = (656),
    DDK_IOMUX_PIN_RTCK                  = (664),
    DDK_IOMUX_PIN_KEY_COL7              = (672),
    DDK_IOMUX_PIN_KEY_COL6              = (680),
    DDK_IOMUX_PIN_KEY_COL5              = (688),
    DDK_IOMUX_PIN_KEY_COL4              = (696),
    DDK_IOMUX_PIN_KEY_COL3              = (704),
    DDK_IOMUX_PIN_KEY_COL2              = (712),
    DDK_IOMUX_PIN_KEY_COL1              = (720),
    DDK_IOMUX_PIN_KEY_COL0              = (728),
    DDK_IOMUX_PIN_KEY_ROW7              = (736),
    DDK_IOMUX_PIN_KEY_ROW6              = (744),
    DDK_IOMUX_PIN_KEY_ROW5              = (752),
    DDK_IOMUX_PIN_KEY_ROW4              = (760),
    DDK_IOMUX_PIN_KEY_ROW3              = (768),
    DDK_IOMUX_PIN_KEY_ROW2              = (776),
    DDK_IOMUX_PIN_KEY_ROW1              = (784),
    DDK_IOMUX_PIN_KEY_ROW0              = (792),
    DDK_IOMUX_PIN_BATT_LINE             = (800),
    DDK_IOMUX_PIN_CTS2                  = (808),
    DDK_IOMUX_PIN_RTS2                  = (816),
    DDK_IOMUX_PIN_TXD2                  = (824),
    DDK_IOMUX_PIN_RXD2                  = (832),
    DDK_IOMUX_PIN_DTR_DCE2              = (840),
    DDK_IOMUX_PIN_DCD_DTE1              = (848),
    DDK_IOMUX_PIN_RI_DTE1               = (856),
    DDK_IOMUX_PIN_DSR_DTE1              = (864),
    DDK_IOMUX_PIN_DTR_DTE1              = (872),
    DDK_IOMUX_PIN_DCD_DCE1              = (880),
    DDK_IOMUX_PIN_RI_DCE1               = (888),
    DDK_IOMUX_PIN_DSR_DCE1              = (896),
    DDK_IOMUX_PIN_DTR_DCE1              = (904),
    DDK_IOMUX_PIN_CTS1                  = (912),
    DDK_IOMUX_PIN_RTS1                  = (920),
    DDK_IOMUX_PIN_TXD1                  = (928),
    DDK_IOMUX_PIN_RXD1                  = (936),
    DDK_IOMUX_PIN_CSPI2_SPI_RDY         = (944),
    DDK_IOMUX_PIN_CSPI2_SCLK            = (952),
    DDK_IOMUX_PIN_CSPI2_SS2             = (960),
    DDK_IOMUX_PIN_CSPI2_SS1             = (968),
    DDK_IOMUX_PIN_CSPI2_SS0             = (976),
    DDK_IOMUX_PIN_CSPI2_MISO            = (984),
    DDK_IOMUX_PIN_CSPI2_MOSI            = (992),
    DDK_IOMUX_PIN_CSPI1_SPI_RDY         = (1000),
    DDK_IOMUX_PIN_CSPI1_SCLK            = (1008),
    DDK_IOMUX_PIN_CSPI1_SS2             = (1016),
    DDK_IOMUX_PIN_CSPI1_SS1             = (1024),
    DDK_IOMUX_PIN_CSPI1_SS0             = (1032),
    DDK_IOMUX_PIN_CSPI1_MISO            = (1040),
    DDK_IOMUX_PIN_CSPI1_MOSI            = (1048),
    DDK_IOMUX_PIN_SFS6                  = (1056),
    DDK_IOMUX_PIN_SCK6                  = (1064),
    DDK_IOMUX_PIN_SRXD6                 = (1072),
    DDK_IOMUX_PIN_STXD6                 = (1080),
    DDK_IOMUX_PIN_SFS5                  = (1088),
    DDK_IOMUX_PIN_SCK5                  = (1096),
    DDK_IOMUX_PIN_SRXD5                 = (1104),
    DDK_IOMUX_PIN_STXD5                 = (1112),
    DDK_IOMUX_PIN_SFS4                  = (1120),
    DDK_IOMUX_PIN_SCK4                  = (1128),
    DDK_IOMUX_PIN_SRXD4                 = (1136),
    DDK_IOMUX_PIN_STXD4                 = (1144),
    DDK_IOMUX_PIN_SFS3                  = (1152),
    DDK_IOMUX_PIN_SCK3                  = (1160),
    DDK_IOMUX_PIN_SRXD3                 = (1168),
    DDK_IOMUX_PIN_STXD3                 = (1176),
    DDK_IOMUX_PIN_I2C_DAT               = (1184),
    DDK_IOMUX_PIN_I2C_CLK               = (1192),
    DDK_IOMUX_PIN_CSI_PIXCLK            = (1200),
    DDK_IOMUX_PIN_CSI_HSYNC             = (1208),
    DDK_IOMUX_PIN_CSI_VSYNC             = (1216),
    DDK_IOMUX_PIN_CSI_MCLK              = (1224),
    DDK_IOMUX_PIN_CSI_D15               = (1232),
    DDK_IOMUX_PIN_CSI_D14               = (1240),
    DDK_IOMUX_PIN_CSI_D13               = (1248),
    DDK_IOMUX_PIN_CSI_D12               = (1256),
    DDK_IOMUX_PIN_CSI_D11               = (1264),
    DDK_IOMUX_PIN_CSI_D10               = (1272),
    DDK_IOMUX_PIN_CSI_D9                = (1280),
    DDK_IOMUX_PIN_CSI_D8                = (1288),
    DDK_IOMUX_PIN_CSI_D7                = (1296),
    DDK_IOMUX_PIN_CSI_D6                = (1304),
    DDK_IOMUX_PIN_CSI_D5                = (1312),
    DDK_IOMUX_PIN_CSI_D4                = (1320),
    DDK_IOMUX_PIN_M_GRANT               = (1328),
    DDK_IOMUX_PIN_M_REQUEST             = (1336),
    DDK_IOMUX_PIN_PC_POE                = (1344),
    DDK_IOMUX_PIN_PC_RW_B               = (1352),
    DDK_IOMUX_PIN_IOIS16                = (1360),
    DDK_IOMUX_PIN_PC_RST                = (1368),
    DDK_IOMUX_PIN_PC_BVD2               = (1376),
    DDK_IOMUX_PIN_PC_BVD1               = (1384),
    DDK_IOMUX_PIN_PC_VS2                = (1392),
    DDK_IOMUX_PIN_PC_VS1                = (1400),
    DDK_IOMUX_PIN_PC_PWRON              = (1408),
    DDK_IOMUX_PIN_PC_READY              = (1416),
    DDK_IOMUX_PIN_PC_WAIT_B             = (1424),
    DDK_IOMUX_PIN_PC_CD2_B              = (1432),
    DDK_IOMUX_PIN_PC_CD1_B              = (1440),
    DDK_IOMUX_PIN_D0                    = (1448),
    DDK_IOMUX_PIN_D1                    = (1456),
    DDK_IOMUX_PIN_D2                    = (1464),
    DDK_IOMUX_PIN_D3                    = (1472),
    DDK_IOMUX_PIN_D4                    = (1480),
    DDK_IOMUX_PIN_D5                    = (1488),
    DDK_IOMUX_PIN_D6                    = (1496),
    DDK_IOMUX_PIN_D7                    = (1504),
    DDK_IOMUX_PIN_D8                    = (1512),
    DDK_IOMUX_PIN_D9                    = (1520),
    DDK_IOMUX_PIN_D10                   = (1528),
    DDK_IOMUX_PIN_D11                   = (1536),
    DDK_IOMUX_PIN_D12                   = (1544),
    DDK_IOMUX_PIN_D13                   = (1552),
    DDK_IOMUX_PIN_D14                   = (1560),
    DDK_IOMUX_PIN_D15                   = (1568),
    DDK_IOMUX_PIN_NFRB                  = (1576),
    DDK_IOMUX_PIN_NFCE_B                = (1584),
    DDK_IOMUX_PIN_NFWP_B                = (1592),
    DDK_IOMUX_PIN_NFCLE                 = (1600),
    DDK_IOMUX_PIN_NFALE                 = (1608),
    DDK_IOMUX_PIN_NFRE_B                = (1616),
    DDK_IOMUX_PIN_NFWE_B                = (1624),
    DDK_IOMUX_PIN_SDQS3                 = (1632),
    DDK_IOMUX_PIN_SDQS2                 = (1640),
    DDK_IOMUX_PIN_SDQS1                 = (1648),
    DDK_IOMUX_PIN_SDQS0                 = (1656),
    DDK_IOMUX_PIN_SD2CLK_B              = (1664),
    DDK_IOMUX_PIN_SDCLK                 = (1672),
    DDK_IOMUX_PIN_SDCKE1                = (1680),
    DDK_IOMUX_PIN_SDCKE0                = (1688),
    DDK_IOMUX_PIN_SDWE                  = (1696),
    DDK_IOMUX_PIN_CAS                   = (1704),
    DDK_IOMUX_PIN_RAS                   = (1712),
    DDK_IOMUX_PIN_RW                    = (1720),
    DDK_IOMUX_PIN_BCLK                  = (1728),
    DDK_IOMUX_PIN_LBA                   = (1736),
    DDK_IOMUX_PIN_ECB                   = (1744),
    DDK_IOMUX_PIN_CS5                   = (1752),
    DDK_IOMUX_PIN_CS4                   = (1760),
    DDK_IOMUX_PIN_CS3                   = (1768),
    DDK_IOMUX_PIN_CS2                   = (1776),
    DDK_IOMUX_PIN_CS1                   = (1784),
    DDK_IOMUX_PIN_CS0                   = (1792),
    DDK_IOMUX_PIN_OE                    = (1800),
    DDK_IOMUX_PIN_EB1                   = (1808),
    DDK_IOMUX_PIN_EB0                   = (1816),
    DDK_IOMUX_PIN_DQM3                  = (1824),
    DDK_IOMUX_PIN_DQM2                  = (1832),
    DDK_IOMUX_PIN_DQM1                  = (1840),
    DDK_IOMUX_PIN_DQM0                  = (1848),
    DDK_IOMUX_PIN_SD31                  = (1856),
    DDK_IOMUX_PIN_SD30                  = (1864),
    DDK_IOMUX_PIN_SD29                  = (1872),
    DDK_IOMUX_PIN_SD28                  = (1880),
    DDK_IOMUX_PIN_SD27                  = (1888),
    DDK_IOMUX_PIN_SD26                  = (1896),
    DDK_IOMUX_PIN_SD25                  = (1904),
    DDK_IOMUX_PIN_SD24                  = (1912),
    DDK_IOMUX_PIN_SD23                  = (1920),
    DDK_IOMUX_PIN_SD22                  = (1928),
    DDK_IOMUX_PIN_SD21                  = (1936),
    DDK_IOMUX_PIN_SD20                  = (1944),
    DDK_IOMUX_PIN_SD19                  = (1952),
    DDK_IOMUX_PIN_SD18                  = (1960),
    DDK_IOMUX_PIN_SD17                  = (1968),
    DDK_IOMUX_PIN_SD16                  = (1976),
    DDK_IOMUX_PIN_SD15                  = (1984),
    DDK_IOMUX_PIN_SD14                  = (1992),
    DDK_IOMUX_PIN_SD13                  = (2000),
    DDK_IOMUX_PIN_SD12                  = (2008),
    DDK_IOMUX_PIN_SD11                  = (2016),
    DDK_IOMUX_PIN_SD10                  = (2024),
    DDK_IOMUX_PIN_SD9                   = (2032),
    DDK_IOMUX_PIN_SD8                   = (2040),
    DDK_IOMUX_PIN_SD7                   = (2048),
    DDK_IOMUX_PIN_SD6                   = (2056),
    DDK_IOMUX_PIN_SD5                   = (2064),
    DDK_IOMUX_PIN_SD4                   = (2072),
    DDK_IOMUX_PIN_SD3                   = (2080),
    DDK_IOMUX_PIN_SD2                   = (2088),
    DDK_IOMUX_PIN_SD1                   = (2096),
    DDK_IOMUX_PIN_SD0                   = (2104),
    DDK_IOMUX_PIN_SDBA0                 = (2112),
    DDK_IOMUX_PIN_SDBA1                 = (2120),
    DDK_IOMUX_PIN_A25                   = (2128),
    DDK_IOMUX_PIN_A24                   = (2136),
    DDK_IOMUX_PIN_A23                   = (2144),
    DDK_IOMUX_PIN_A22                   = (2152),
    DDK_IOMUX_PIN_A21                   = (2160),
    DDK_IOMUX_PIN_A20                   = (2168),
    DDK_IOMUX_PIN_A19                   = (2176),
    DDK_IOMUX_PIN_A18                   = (2184),
    DDK_IOMUX_PIN_A17                   = (2192),
    DDK_IOMUX_PIN_A16                   = (2200),
    DDK_IOMUX_PIN_A15                   = (2208),
    DDK_IOMUX_PIN_A14                   = (2216),
    DDK_IOMUX_PIN_A13                   = (2224),
    DDK_IOMUX_PIN_A12                   = (2232),
    DDK_IOMUX_PIN_A11                   = (2240),
    DDK_IOMUX_PIN_MA10                  = (2248),
    DDK_IOMUX_PIN_A10                   = (2256),
    DDK_IOMUX_PIN_A9                    = (2264),
    DDK_IOMUX_PIN_A8                    = (2272),
    DDK_IOMUX_PIN_A7                    = (2280),
    DDK_IOMUX_PIN_A6                    = (2288),
    DDK_IOMUX_PIN_A5                    = (2296),
    DDK_IOMUX_PIN_A4                    = (2304),
    DDK_IOMUX_PIN_A3                    = (2312),
    DDK_IOMUX_PIN_A2                    = (2320),
    DDK_IOMUX_PIN_A1                    = (2328),
    DDK_IOMUX_PIN_A0                    = (2336),
    DDK_IOMUX_PIN_VPG1                  = (2344),
    DDK_IOMUX_PIN_VPG0                  = (2352),
    DDK_IOMUX_PIN_DVFS1                 = (2360),
    DDK_IOMUX_PIN_DVFS0                 = (2368),
    DDK_IOMUX_PIN_VSTBY                 = (2376),
    DDK_IOMUX_PIN_POWER_FAIL            = (2384),
    DDK_IOMUX_PIN_CKIL                  = (2392),
    DDK_IOMUX_PIN_BOOT_MODE4            = (2400),
    DDK_IOMUX_PIN_BOOT_MODE3            = (2408),
    DDK_IOMUX_PIN_BOOT_MODE2            = (2416),
    DDK_IOMUX_PIN_BOOT_MODE1            = (2424),
    DDK_IOMUX_PIN_BOOT_MODE0            = (2432),
    DDK_IOMUX_PIN_CLKO                  = (2440),
    DDK_IOMUX_PIN_POR_B                 = (2448),
    DDK_IOMUX_PIN_RESET_IN_B            = (2456),
    DDK_IOMUX_PIN_CKIH                  = (2464),
    DDK_IOMUX_PIN_SIMPD0                = (2472),
    DDK_IOMUX_PIN_SRX0                  = (2480),
    DDK_IOMUX_PIN_STX0                  = (2488),
    DDK_IOMUX_PIN_SVEN0                 = (2496),
    DDK_IOMUX_PIN_SRST0                 = (2504),
    DDK_IOMUX_PIN_SCLK0                 = (2512),
    DDK_IOMUX_PIN_GPIO3_1               = (2520),
    DDK_IOMUX_PIN_GPIO3_0               = (2528),
    DDK_IOMUX_PIN_GPIO1_6               = (2536),
    DDK_IOMUX_PIN_GPIO1_5               = (2544),
    DDK_IOMUX_PIN_GPIO1_4               = (2552),
    DDK_IOMUX_PIN_GPIO1_3               = (2560),
    DDK_IOMUX_PIN_GPIO1_2               = (2568),
    DDK_IOMUX_PIN_GPIO1_1               = (2576),
    DDK_IOMUX_PIN_GPIO1_0               = (2584),
    DDK_IOMUX_PIN_PWMO                  = (2592),
    DDK_IOMUX_PIN_WATCHDOG_RST          = (2600),
    DDK_IOMUX_PIN_COMPARE               = (2608),
    DDK_IOMUX_PIN_CAPTURE               = (2616)
} DDK_IOMUX_PIN;
#endif // IMGVPMX31


//-----------------------------------------------------------------------------
//
//  Type: DDK_IOMUX_OUT
//
//  Specifies the muxing on the output path for a signal.
//
// 
//-----------------------------------------------------------------------------
typedef enum 
{
    DDK_IOMUX_OUT_GPIO      = (IOMUX_SW_MUX_CTL_OUT_GPIO << IOMUX_SW_MUX_CTL_OUT_LSH),
    DDK_IOMUX_OUT_FUNC      = (IOMUX_SW_MUX_CTL_OUT_FUNC << IOMUX_SW_MUX_CTL_OUT_LSH),
    DDK_IOMUX_OUT_ALT1      = (IOMUX_SW_MUX_CTL_OUT_MUX1 << IOMUX_SW_MUX_CTL_OUT_LSH),
    DDK_IOMUX_OUT_ALT2      = (IOMUX_SW_MUX_CTL_OUT_MUX2 << IOMUX_SW_MUX_CTL_OUT_LSH),
    DDK_IOMUX_OUT_ALT3      = (IOMUX_SW_MUX_CTL_OUT_MUX3 << IOMUX_SW_MUX_CTL_OUT_LSH),
    DDK_IOMUX_OUT_ALT4      = (IOMUX_SW_MUX_CTL_OUT_MUX4 << IOMUX_SW_MUX_CTL_OUT_LSH),
    DDK_IOMUX_OUT_ALT5      = (IOMUX_SW_MUX_CTL_OUT_MUX5 << IOMUX_SW_MUX_CTL_OUT_LSH),
    DDK_IOMUX_OUT_ALT6      = (IOMUX_SW_MUX_CTL_OUT_MUX6 << IOMUX_SW_MUX_CTL_OUT_LSH)
} DDK_IOMUX_OUT;


//-----------------------------------------------------------------------------
//
//  Type: DDK_IOMUX_IN
//
//  Specifies the muxing on the input path for a signal.
//
// 
//-----------------------------------------------------------------------------
typedef enum 
{
    DDK_IOMUX_IN_NONE       = (0),
    DDK_IOMUX_IN_GPIO       = (1U << IOMUX_SW_MUX_CTL_GPIO_IN_LSH),
    DDK_IOMUX_IN_FUNC       = (1U << IOMUX_SW_MUX_CTL_FUNC_IN_LSH),
    DDK_IOMUX_IN_ALT1       = (1U << IOMUX_SW_MUX_CTL_MUX1_IN_LSH),
    DDK_IOMUX_IN_ALT2       = (1U << IOMUX_SW_MUX_CTL_MUX2_IN_LSH)
} DDK_IOMUX_IN;


//-----------------------------------------------------------------------------
//
//  Type: DDK_IOMUX_GPR
//
//  Specifies the general purpose register (GPR) bits within the IOMUX
//  used to control various muxing features within the SoC.
// 
//-----------------------------------------------------------------------------
typedef enum 
{
    DDK_IOMUX_GPR_FIRI_UART2    = (0),
    DDK_IOMUX_GPR_DDR_MODE      = (1),
    DDK_IOMUX_GPR_CSPI_BB       = (2),
    DDK_IOMUX_GPR_ATA1          = (3),
    DDK_IOMUX_GPR_ATA2          = (4),
    DDK_IOMUX_GPR_ATA3          = (5),
    DDK_IOMUX_GPR_ATA4          = (6),
    DDK_IOMUX_GPR_ATA5          = (7),
    DDK_IOMUX_GPR_ATA6          = (8),
    DDK_IOMUX_GPR_ATA7          = (9),
    DDK_IOMUX_GPR_ATA8          = (10),
    DDK_IOMUX_GPR_UH2           = (11),
    DDK_IOMUX_GPR_CSD0          = (12),
    DDK_IOMUX_GPR_CSD1          = (13),
    DDK_IOMUX_GPR_CSPI1_UART3   = (14),
    DDK_IOMUX_GPR_MBX           = (15),
    DDK_IOMUX_GPR_TAMPER_EN     = (16),
    DDK_IOMUX_GPR_USB_4WIRE     = (17),
    DDK_IOMUX_GPR_USB_COMMON    = (18),
    DDK_IOMUX_GPR_SDHC_MS1      = (19),
    DDK_IOMUX_GPR_SDHC_MS2      = (20),
    DDK_IOMUX_GPR_SPLL_BYP      = (21),
    DDK_IOMUX_GPR_UPLL_BYP      = (22),
    DDK_IOMUX_GPR_MSHC1_CLK     = (23),
    DDK_IOMUX_GPR_MSHC2_CLK     = (24),
    DDK_IOMUX_GPR_CSPI3_UART5   = (25),
    DDK_IOMUX_GPR_ATA9          = (26),
    DDK_IOMUX_GPR_USB_SUSPEND   = (27),
    DDK_IOMUX_GPR_USB_OTG_LOOP  = (28),
    DDK_IOMUX_GPR_USB_HS1_LOOP  = (29),
    DDK_IOMUX_GPR_USB_HS2_LOOP  = (30),
    DDK_IOMUX_GPR_CLK0          = (31)
} DDK_IOMUX_GPR;



//-----------------------------------------------------------------------------
//
//  Type: DDK_IOMUX_PAD
//
//  Specifies the functional pad name used to configure the IOMUX.  The enum
//  value corresponds to the bit offset within the SW_PAD_CTL registers.
//
//-----------------------------------------------------------------------------
typedef enum {
    DDK_IOMUX_PAD_PAD2                  = (0),
    DDK_IOMUX_PAD_PAD1                  = (10),
    DDK_IOMUX_PAD_TTM                   = (20),
    DDK_IOMUX_PAD_CSPI3_SPI_RDY         = (32),
    DDK_IOMUX_PAD_CSPI3_SCLK            = (42),
    DDK_IOMUX_PAD_CSPI3_MISO            = (52),
    DDK_IOMUX_PAD_CSPI3_MOSI            = (64),
    DDK_IOMUX_PAD_CLKSS                 = (74),
    DDK_IOMUX_PAD_CE_CONTROL            = (84),
    DDK_IOMUX_PAD_ATA_RESET_B           = (96),
    DDK_IOMUX_PAD_ATA_DMACK             = (106),
    DDK_IOMUX_PAD_ATA_DIOW              = (116),
    DDK_IOMUX_PAD_ATA_DIOR              = (128),
    DDK_IOMUX_PAD_ATA_CS1               = (138),
    DDK_IOMUX_PAD_ATA_CS0               = (148),
    DDK_IOMUX_PAD_SD1_DATA3             = (160),
    DDK_IOMUX_PAD_SD1_DATA2             = (170),
    DDK_IOMUX_PAD_SD1_DATA1             = (180),
    DDK_IOMUX_PAD_SD1_DATA0             = (192),
    DDK_IOMUX_PAD_SD1_CLK               = (202),
    DDK_IOMUX_PAD_SD1_CMD               = (212),
    DDK_IOMUX_PAD_D3_SPL                = (224),
    DDK_IOMUX_PAD_D3_CLS                = (234),
    DDK_IOMUX_PAD_D3_REV                = (244),
    DDK_IOMUX_PAD_CONTRAST              = (256),
    DDK_IOMUX_PAD_VSYNC3                = (266),
    DDK_IOMUX_PAD_READ                  = (276),
    DDK_IOMUX_PAD_WRITE                 = (288),
    DDK_IOMUX_PAD_PAR_RS                = (298),
    DDK_IOMUX_PAD_SER_RS                = (308),
    DDK_IOMUX_PAD_LCS1                  = (320),
    DDK_IOMUX_PAD_LCS0                  = (330),
    DDK_IOMUX_PAD_SD_D_CLK              = (340),
    DDK_IOMUX_PAD_SD_D_IO               = (352),
    DDK_IOMUX_PAD_SD_D_I                = (362),
    DDK_IOMUX_PAD_DRDY0                 = (372),
    DDK_IOMUX_PAD_FPSHIFT               = (384),
    DDK_IOMUX_PAD_HSYNC                 = (394),
    DDK_IOMUX_PAD_VSYNC0                = (404),
    DDK_IOMUX_PAD_LD17                  = (416),
    DDK_IOMUX_PAD_LD16                  = (426),
    DDK_IOMUX_PAD_LD15                  = (436),
    DDK_IOMUX_PAD_LD14                  = (448),
    DDK_IOMUX_PAD_LD13                  = (458),
    DDK_IOMUX_PAD_LD12                  = (468),
    DDK_IOMUX_PAD_LD11                  = (480),
    DDK_IOMUX_PAD_LD10                  = (490),
    DDK_IOMUX_PAD_LD9                   = (500),
    DDK_IOMUX_PAD_LD8                   = (512),
    DDK_IOMUX_PAD_LD7                   = (522),
    DDK_IOMUX_PAD_LD6                   = (532),
    DDK_IOMUX_PAD_LD5                   = (544),
    DDK_IOMUX_PAD_LD4                   = (554),
    DDK_IOMUX_PAD_LD3                   = (564),
    DDK_IOMUX_PAD_LD2                   = (576),
    DDK_IOMUX_PAD_LD1                   = (586),
    DDK_IOMUX_PAD_LD0                   = (596),
    DDK_IOMUX_PAD_USBH2_DATA1           = (608),
    DDK_IOMUX_PAD_USBH2_DATA0           = (618),
    DDK_IOMUX_PAD_USBH2_NXT             = (628),
    DDK_IOMUX_PAD_USBH2_STP             = (640),
    DDK_IOMUX_PAD_USBH2_DIR             = (650),
    DDK_IOMUX_PAD_USBH2_CLK             = (660),
    DDK_IOMUX_PAD_USBOTG_DATA7          = (672),
    DDK_IOMUX_PAD_USBOTG_DATA6          = (682),
    DDK_IOMUX_PAD_USBOTG_DATA5          = (692),
    DDK_IOMUX_PAD_USBOTG_DATA4          = (704),
    DDK_IOMUX_PAD_USBOTG_DATA3          = (714),
    DDK_IOMUX_PAD_USBOTG_DATA2          = (724),
    DDK_IOMUX_PAD_USBOTG_DATA1          = (736),
    DDK_IOMUX_PAD_USBOTG_DATA0          = (746),
    DDK_IOMUX_PAD_USBOTG_NXT            = (756),
    DDK_IOMUX_PAD_USBOTG_STP            = (768),
    DDK_IOMUX_PAD_USBOTG_DIR            = (778),
    DDK_IOMUX_PAD_USBOTG_CLK            = (788),
    DDK_IOMUX_PAD_USB_BYP               = (800),
    DDK_IOMUX_PAD_USB_OC                = (810),
    DDK_IOMUX_PAD_USB_PWR               = (820),
    DDK_IOMUX_PAD_SJC_MOD               = (832),
    DDK_IOMUX_PAD_DE_B                  = (842),
    DDK_IOMUX_PAD_TRSTB                 = (852),
    DDK_IOMUX_PAD_TDO                   = (864),
    DDK_IOMUX_PAD_TDI                   = (874),
    DDK_IOMUX_PAD_TMS                   = (884),
    DDK_IOMUX_PAD_TCK                   = (896),
    DDK_IOMUX_PAD_RTCK                  = (906),
    DDK_IOMUX_PAD_KEY_COL7              = (916),
    DDK_IOMUX_PAD_KEY_COL6              = (928),
    DDK_IOMUX_PAD_KEY_COL5              = (938),
    DDK_IOMUX_PAD_KEY_COL4              = (948),
    DDK_IOMUX_PAD_KEY_COL3              = (960),
    DDK_IOMUX_PAD_KEY_COL2              = (970),
    DDK_IOMUX_PAD_KEY_COL1              = (980),
    DDK_IOMUX_PAD_KEY_COL0              = (992),
    DDK_IOMUX_PAD_KEY_ROW7              = (1002),
    DDK_IOMUX_PAD_KEY_ROW6              = (1012),
    DDK_IOMUX_PAD_KEY_ROW5              = (1024),
    DDK_IOMUX_PAD_KEY_ROW4              = (1034),
    DDK_IOMUX_PAD_KEY_ROW3              = (1044),
    DDK_IOMUX_PAD_KEY_ROW2              = (1056),
    DDK_IOMUX_PAD_KEY_ROW1              = (1066),
    DDK_IOMUX_PAD_KEY_ROW0              = (1076),
    DDK_IOMUX_PAD_BATT_LINE             = (1088),
    DDK_IOMUX_PAD_CTS2                  = (1098),
    DDK_IOMUX_PAD_RTS2                  = (1108),
    DDK_IOMUX_PAD_TXD2                  = (1120),
    DDK_IOMUX_PAD_RXD2                  = (1130),
    DDK_IOMUX_PAD_DTR_DCE2              = (1140),
    DDK_IOMUX_PAD_DCD_DTE1              = (1152),
    DDK_IOMUX_PAD_RI_DTE1               = (1162),
    DDK_IOMUX_PAD_DSR_DTE1              = (1172),
    DDK_IOMUX_PAD_DTR_DTE1              = (1184),
    DDK_IOMUX_PAD_DCD_DCE1              = (1194),
    DDK_IOMUX_PAD_RI_DCE1               = (1204),
    DDK_IOMUX_PAD_DSR_DCE1              = (1216),
    DDK_IOMUX_PAD_DTR_DCE1              = (1226),
    DDK_IOMUX_PAD_CTS1                  = (1236),
    DDK_IOMUX_PAD_RTS1                  = (1248),
    DDK_IOMUX_PAD_TXD1                  = (1258),
    DDK_IOMUX_PAD_RXD1                  = (1268),
    DDK_IOMUX_PAD_CSPI2_SPI_RDY         = (1280),
    DDK_IOMUX_PAD_CSPI2_SCLK            = (1290),
    DDK_IOMUX_PAD_CSPI2_SS2             = (1300),
    DDK_IOMUX_PAD_CSPI2_SS1             = (1312),
    DDK_IOMUX_PAD_CSPI2_SS0             = (1322),
    DDK_IOMUX_PAD_CSPI2_MISO            = (1332),
    DDK_IOMUX_PAD_CSPI2_MOSI            = (1344),
    DDK_IOMUX_PAD_CSPI1_SPI_RDY         = (1354),
    DDK_IOMUX_PAD_CSPI1_SCLK            = (1364),
    DDK_IOMUX_PAD_CSPI1_SS2             = (1376),
    DDK_IOMUX_PAD_CSPI1_SS1             = (1386),
    DDK_IOMUX_PAD_CSPI1_SS0             = (1396),
    DDK_IOMUX_PAD_CSPI1_MISO            = (1408),
    DDK_IOMUX_PAD_CSPI1_MOSI            = (1418),
    DDK_IOMUX_PAD_SFS6                  = (1428),
    DDK_IOMUX_PAD_SCK6                  = (1440),
    DDK_IOMUX_PAD_SRXD6                 = (1450),
    DDK_IOMUX_PAD_STXD6                 = (1460),
    DDK_IOMUX_PAD_SFS5                  = (1472),
    DDK_IOMUX_PAD_SCK5                  = (1482),
    DDK_IOMUX_PAD_SRXD5                 = (1492),
    DDK_IOMUX_PAD_STXD5                 = (1504),
    DDK_IOMUX_PAD_SFS4                  = (1514),
    DDK_IOMUX_PAD_SCK4                  = (1524),
    DDK_IOMUX_PAD_SRXD4                 = (1536),
    DDK_IOMUX_PAD_STXD4                 = (1546),
    DDK_IOMUX_PAD_SFS3                  = (1556),
    DDK_IOMUX_PAD_SCK3                  = (1568),
    DDK_IOMUX_PAD_SRXD3                 = (1578),
    DDK_IOMUX_PAD_STXD3                 = (1588),
    DDK_IOMUX_PAD_I2C_DAT               = (1600),
    DDK_IOMUX_PAD_I2C_CLK               = (1610),
    DDK_IOMUX_PAD_CSI_PIXCLK            = (1620),
    DDK_IOMUX_PAD_CSI_HSYNC             = (1632),
    DDK_IOMUX_PAD_CSI_VSYNC             = (1642),
    DDK_IOMUX_PAD_CSI_MCLK              = (1652),
    DDK_IOMUX_PAD_CSI_D15               = (1664),
    DDK_IOMUX_PAD_CSI_D14               = (1674),
    DDK_IOMUX_PAD_CSI_D13               = (1684),
    DDK_IOMUX_PAD_CSI_D12               = (1696),
    DDK_IOMUX_PAD_CSI_D11               = (1706),
    DDK_IOMUX_PAD_CSI_D10               = (1716),
    DDK_IOMUX_PAD_CSI_D9                = (1728),
    DDK_IOMUX_PAD_CSI_D8                = (1738),
    DDK_IOMUX_PAD_CSI_D7                = (1748),
    DDK_IOMUX_PAD_CSI_D6                = (1760),
    DDK_IOMUX_PAD_CSI_D5                = (1770),
    DDK_IOMUX_PAD_CSI_D4                = (1780),
    DDK_IOMUX_PAD_M_GRANT               = (1792),
    DDK_IOMUX_PAD_M_REQUEST             = (1802),
    DDK_IOMUX_PAD_PC_POE                = (1812),
    DDK_IOMUX_PAD_PC_RW_B               = (1824),
    DDK_IOMUX_PAD_IOIS16                = (1834),
    DDK_IOMUX_PAD_PC_RST                = (1844),
    DDK_IOMUX_PAD_PC_BVD2               = (1856),
    DDK_IOMUX_PAD_PC_BVD1               = (1866),
    DDK_IOMUX_PAD_PC_VS2                = (1876),
    DDK_IOMUX_PAD_PC_VS1                = (1888),
    DDK_IOMUX_PAD_PC_PWRON              = (1898),
    DDK_IOMUX_PAD_PC_READY              = (1908),
    DDK_IOMUX_PAD_PC_WAIT_B             = (1920),
    DDK_IOMUX_PAD_PC_CD2_B              = (1930),
    DDK_IOMUX_PAD_PC_CD1_B              = (1940),
    DDK_IOMUX_PAD_D0                    = (1952),
    DDK_IOMUX_PAD_D1                    = (1962),
    DDK_IOMUX_PAD_D2                    = (1972),
    DDK_IOMUX_PAD_D3                    = (1984),
    DDK_IOMUX_PAD_D4                    = (1994),
    DDK_IOMUX_PAD_D5                    = (2004),
    DDK_IOMUX_PAD_D6                    = (2016),
    DDK_IOMUX_PAD_D7                    = (2026),
    DDK_IOMUX_PAD_D8                    = (2036),
    DDK_IOMUX_PAD_D9                    = (2048),
    DDK_IOMUX_PAD_D10                   = (2058),
    DDK_IOMUX_PAD_D11                   = (2068),
    DDK_IOMUX_PAD_D12                   = (2080),
    DDK_IOMUX_PAD_D13                   = (2090),
    DDK_IOMUX_PAD_D14                   = (2100),
    DDK_IOMUX_PAD_D15                   = (2112),
    DDK_IOMUX_PAD_NFRB                  = (2122),
    DDK_IOMUX_PAD_NFCE_B                = (2132),
    DDK_IOMUX_PAD_NFWP_B                = (2144),
    DDK_IOMUX_PAD_NFCLE                 = (2154),
    DDK_IOMUX_PAD_NFALE                 = (2164),
    DDK_IOMUX_PAD_NFRE_B                = (2176),
    DDK_IOMUX_PAD_NFWE_B                = (2186),
    DDK_IOMUX_PAD_SDQS3                 = (2196),
    DDK_IOMUX_PAD_SDQS2                 = (2208),
    DDK_IOMUX_PAD_SDQS1                 = (2218),
    DDK_IOMUX_PAD_SDQS0                 = (2228),
    DDK_IOMUX_PAD_SD2CLK_B              = (2240),
    DDK_IOMUX_PAD_SDCLK                 = (2250),
    DDK_IOMUX_PAD_SDCKE1                = (2260),
    DDK_IOMUX_PAD_SDCKE0                = (2272),
    DDK_IOMUX_PAD_SDWE                  = (2282),
    DDK_IOMUX_PAD_CAS                   = (2292),
    DDK_IOMUX_PAD_RAS                   = (2304),
    DDK_IOMUX_PAD_RW                    = (2314),
    DDK_IOMUX_PAD_BCLK                  = (2324),
    DDK_IOMUX_PAD_LBA                   = (2336),
    DDK_IOMUX_PAD_ECB                   = (2346),
    DDK_IOMUX_PAD_CS5                   = (2356),
    DDK_IOMUX_PAD_CS4                   = (2368),
    DDK_IOMUX_PAD_CS3                   = (2378),
    DDK_IOMUX_PAD_CS2                   = (2388),
    DDK_IOMUX_PAD_CS1                   = (2400),
    DDK_IOMUX_PAD_CS0                   = (2410),
    DDK_IOMUX_PAD_OE                    = (2420),
    DDK_IOMUX_PAD_EB1                   = (2432),
    DDK_IOMUX_PAD_EB0                   = (2442),
    DDK_IOMUX_PAD_DQM3                  = (2452),
    DDK_IOMUX_PAD_DQM2                  = (2464),
    DDK_IOMUX_PAD_DQM1                  = (2474),
    DDK_IOMUX_PAD_DQM0                  = (2484),
    DDK_IOMUX_PAD_SD31                  = (2496),
    DDK_IOMUX_PAD_SD30                  = (2506),
    DDK_IOMUX_PAD_SD29                  = (2516),
    DDK_IOMUX_PAD_SD28                  = (2528),
    DDK_IOMUX_PAD_SD27                  = (2538),
    DDK_IOMUX_PAD_SD26                  = (2548),
    DDK_IOMUX_PAD_SD25                  = (2560),
    DDK_IOMUX_PAD_SD24                  = (2570),
    DDK_IOMUX_PAD_SD23                  = (2580),
    DDK_IOMUX_PAD_SD22                  = (2592),
    DDK_IOMUX_PAD_SD21                  = (2602),
    DDK_IOMUX_PAD_SD20                  = (2612),
    DDK_IOMUX_PAD_SD19                  = (2624),
    DDK_IOMUX_PAD_SD18                  = (2634),
    DDK_IOMUX_PAD_SD17                  = (2644),
    DDK_IOMUX_PAD_SD16                  = (2656),
    DDK_IOMUX_PAD_SD15                  = (2666),
    DDK_IOMUX_PAD_SD14                  = (2676),
    DDK_IOMUX_PAD_SD13                  = (2688),
    DDK_IOMUX_PAD_SD12                  = (2698),
    DDK_IOMUX_PAD_SD11                  = (2708),
    DDK_IOMUX_PAD_SD10                  = (2720),
    DDK_IOMUX_PAD_SD9                   = (2730),
    DDK_IOMUX_PAD_SD8                   = (2740),
    DDK_IOMUX_PAD_SD7                   = (2752),
    DDK_IOMUX_PAD_SD6                   = (2762),
    DDK_IOMUX_PAD_SD5                   = (2772),
    DDK_IOMUX_PAD_SD4                   = (2784),
    DDK_IOMUX_PAD_SD3                   = (2794),
    DDK_IOMUX_PAD_SD2                   = (2804),
    DDK_IOMUX_PAD_SD1                   = (2816),
    DDK_IOMUX_PAD_SD0                   = (2826),
    DDK_IOMUX_PAD_SDBA0                 = (2836),
    DDK_IOMUX_PAD_SDBA1                 = (2848),
    DDK_IOMUX_PAD_A25                   = (2858),
    DDK_IOMUX_PAD_A24                   = (2868),
    DDK_IOMUX_PAD_A23                   = (2880),
    DDK_IOMUX_PAD_A22                   = (2890),
    DDK_IOMUX_PAD_A21                   = (2900),
    DDK_IOMUX_PAD_A20                   = (2912),
    DDK_IOMUX_PAD_A19                   = (2922),
    DDK_IOMUX_PAD_A18                   = (2932),
    DDK_IOMUX_PAD_A17                   = (2944),
    DDK_IOMUX_PAD_A16                   = (2954),
    DDK_IOMUX_PAD_A15                   = (2964),
    DDK_IOMUX_PAD_A14                   = (2976),
    DDK_IOMUX_PAD_A13                   = (2986),
    DDK_IOMUX_PAD_A12                   = (2996),
    DDK_IOMUX_PAD_A11                   = (3008),
    DDK_IOMUX_PAD_MA10                  = (3018),
    DDK_IOMUX_PAD_A10                   = (3028),
    DDK_IOMUX_PAD_A9                    = (3040),
    DDK_IOMUX_PAD_A8                    = (3050),
    DDK_IOMUX_PAD_A7                    = (3060),
    DDK_IOMUX_PAD_A6                    = (3072),
    DDK_IOMUX_PAD_A5                    = (3082),
    DDK_IOMUX_PAD_A4                    = (3092),
    DDK_IOMUX_PAD_A3                    = (3104),
    DDK_IOMUX_PAD_A2                    = (3114),
    DDK_IOMUX_PAD_A1                    = (3124),
    DDK_IOMUX_PAD_A0                    = (3136),
    DDK_IOMUX_PAD_VPG1                  = (3146),
    DDK_IOMUX_PAD_VPG0                  = (3156),
    DDK_IOMUX_PAD_DVFS1                 = (3168),
    DDK_IOMUX_PAD_DVFS0                 = (3178),
    DDK_IOMUX_PAD_VSTBY                 = (3188),
    DDK_IOMUX_PAD_POWER_FAIL            = (3200),
    DDK_IOMUX_PAD_CKIL                  = (3210),
    DDK_IOMUX_PAD_BOOT_MODE4            = (3220),
    DDK_IOMUX_PAD_BOOT_MODE3            = (3232),
    DDK_IOMUX_PAD_BOOT_MODE2            = (3242),
    DDK_IOMUX_PAD_BOOT_MODE1            = (3252),
    DDK_IOMUX_PAD_BOOT_MODE0            = (3264),
    DDK_IOMUX_PAD_CLKO                  = (3274),
    DDK_IOMUX_PAD_POR_B                 = (3284),
    DDK_IOMUX_PAD_RESET_IN_B            = (3296),
    DDK_IOMUX_PAD_CKIH                  = (3306),
    DDK_IOMUX_PAD_SIMPD0                = (3316),
    DDK_IOMUX_PAD_SRX0                  = (3328),
    DDK_IOMUX_PAD_STX0                  = (3338),
    DDK_IOMUX_PAD_SVEN0                 = (3348),
    DDK_IOMUX_PAD_SRST0                 = (3360),
    DDK_IOMUX_PAD_SCLK0                 = (3370),
    DDK_IOMUX_PAD_GPIO3_1               = (3380),
    DDK_IOMUX_PAD_GPIO3_0               = (3392),
    DDK_IOMUX_PAD_GPIO1_6               = (3402),
    DDK_IOMUX_PAD_GPIO1_5               = (3412),
    DDK_IOMUX_PAD_GPIO1_4               = (3424),
    DDK_IOMUX_PAD_GPIO1_3               = (3434),
    DDK_IOMUX_PAD_GPIO1_2               = (3444),
    DDK_IOMUX_PAD_GPIO1_1               = (3456),
    DDK_IOMUX_PAD_GPIO1_0               = (3466),
    DDK_IOMUX_PAD_PWMO                  = (3476),
    DDK_IOMUX_PAD_WATCHDOG_RST          = (3488),
    DDK_IOMUX_PAD_COMPARE               = (3498),
    DDK_IOMUX_PAD_CAPTURE               = (3508)
} DDK_IOMUX_PAD;


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
    DDK_IOMUX_PAD_DRIVE_HIGH    = (IOMUX_SW_PAD_CTL_DSE_HIGH << IOMUX_SW_PAD_CTL_DSE_LSH),
    DDK_IOMUX_PAD_DRIVE_MAX     = (IOMUX_SW_PAD_CTL_DSE_MAX << IOMUX_SW_PAD_CTL_DSE_LSH)
} DDK_IOMUX_PAD_DRIVE;


//-----------------------------------------------------------------------------
//
//  Type: DDK_IOMUX_PAD_MODE
//
//  Specifies the CMOS/open drain mode for a pad.
//
// 
//-----------------------------------------------------------------------------
typedef enum 
{
    DDK_IOMUX_PAD_MODE_CMOS      = (IOMUX_SW_PAD_CTL_ODE_CMOS << IOMUX_SW_PAD_CTL_ODE_LSH),
    DDK_IOMUX_PAD_MODE_OPENDRAIN = (IOMUX_SW_PAD_CTL_ODE_OPEN_DRAIN << IOMUX_SW_PAD_CTL_ODE_LSH)
} DDK_IOMUX_PAD_MODE;


//-----------------------------------------------------------------------------
//
//  Type: DDK_IOMUX_PAD_TRIG
//
//  Specifies the trigger for a pad.
//
// 
//-----------------------------------------------------------------------------
typedef enum 
{
    DDK_IOMUX_PAD_TRIG_CMOS     = (IOMUX_SW_PAD_CTL_HYS_CMOS << IOMUX_SW_PAD_CTL_HYS_LSH),
    DDK_IOMUX_PAD_TRIG_SCHMITT  = (IOMUX_SW_PAD_CTL_HYS_SCHMITT << IOMUX_SW_PAD_CTL_HYS_LSH)
} DDK_IOMUX_PAD_TRIG;


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
    UINT32 ssi1Cgr0Mask;
    UINT32 csiCgr1Mask;
    UINT32 usbCgr1Mask;
    UINT32 ssi2Cgr2Mask;
    UINT32 firCgr2Mask;
}  DDK_CLK_CONFIG, *PDDK_CLK_CONFIG;

//------------------------------------------------------------------------------
// Macros

// IOMUX_MUX_REG converts from a pin number to an index for the SW_MUX_CTL
// register.  
#define IOMUX_MUX_REG(pin)          ((pin) >> 5)

// IOMUX_MUX_SHIFT converts from a pin number to a shift value used
// to reference the SW_MUX_CTL bits assigned to that pin.  
#define IOMUX_MUX_SHIFT(pin)        ((pin) & 0x1F)

// IOMUX_MUX_MASK converts from a pin number to a mask used to isolate
// the SW_MUX_CTL register bits assigned to that pin.
#define IOMUX_MUX_MASK(pin)  \
    (IOMUX_SW_MUX_CTL_MASK << IOMUX_MUX_SHIFT(pin))

// IOMUX_MUX_BFINS performs a bitfield insert on a SW_MUX_CTL register
#define IOMUX_MUX_BFINS(pin, reg, val)  \
    (((reg) & (~(IOMUX_MUX_MASK(pin)))) | ((val) << IOMUX_MUX_SHIFT(pin)))

// IOMUX_MUX_BFEXT performs a bitfield extract on a SW_MUX_CTL register
#define IOMUX_MUX_BFEXT(pin, reg)  \
    (((reg) >> IOMUX_MUX_SHIFT(pin)) & IOMUX_SW_MUX_CTL_MASK)
        
// IOMUX_PAD_REG converts from a pad number to an index for the SW_PAD_CTL
// register.  
#define IOMUX_PAD_REG(pad)          ((pad) >> 5)

// IOMUX_PAD_SHIFT converts from a pad number to a shift value used
// to reference the SW_PAD_CTL bits assigned to that pad.  
#define IOMUX_PAD_SHIFT(pad)        ((pad) & 0x1F)

// IOMUX_PAD_MASK converts from a pad number to a mask used to isolate
// the SW_PAD_CTL register bits assigned to that pad.
#define IOMUX_PAD_MASK(pad)  \
    (IOMUX_SW_PAD_CTL_MASK << IOMUX_PAD_SHIFT(pad))

// IOMUX_PAD_BFINS performs a bitfield insert on a SW_PAD_CTL register
#define IOMUX_PAD_BFINS(pad, reg, val)  \
    (((reg) & (~(IOMUX_PAD_MASK(pad)))) | ((val) << IOMUX_PAD_SHIFT(pad)))

// IOMUX_PAD_BFINS performs a bitfield extract on a SW_PAD_CTL register
#define IOMUX_PAD_BFEXT(pad, reg)  \
    (((reg) >> IOMUX_PAD_SHIFT(pad)) & IOMUX_SW_PAD_CTL_MASK)



//------------------------------------------------------------------------------
// Functions

BOOL DDKIomuxSetPinMux(DDK_IOMUX_PIN pin, DDK_IOMUX_OUT outMux, DDK_IOMUX_IN inMux);
BOOL DDKIomuxGetPinMux(DDK_IOMUX_PIN pin, DDK_IOMUX_OUT *pOutMux, DDK_IOMUX_IN *pInMux);
BOOL DDKIomuxSetPadConfig(DDK_IOMUX_PAD pad, DDK_IOMUX_PAD_SLEW slew, DDK_IOMUX_PAD_DRIVE drive, DDK_IOMUX_PAD_MODE mode, DDK_IOMUX_PAD_TRIG trig, DDK_IOMUX_PAD_PULL pull);
BOOL DDKIomuxGetPadConfig(DDK_IOMUX_PAD pad, DDK_IOMUX_PAD_SLEW *pSlew, DDK_IOMUX_PAD_DRIVE *pDrive, DDK_IOMUX_PAD_MODE *pMode, DDK_IOMUX_PAD_TRIG *pTrig, DDK_IOMUX_PAD_PULL *pPull);
BOOL DDKIomuxSetGpr(UINT32 mask, UINT32 data);
BOOL DDKIomuxSetGprBit(DDK_IOMUX_GPR bit, UINT32 data);

BOOL DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX index, DDK_CLOCK_GATE_MODE mode);
BOOL DDKClockGetGatingMode(DDK_CLOCK_GATE_INDEX index, DDK_CLOCK_GATE_MODE *pMode);
BOOL DDKClockGetFreq(DDK_CLOCK_SIGNAL sig, UINT32 *freq);
BOOL DDKClockConfigBaud(DDK_CLOCK_SIGNAL sig, DDK_CLOCK_BAUD_SOURCE src, UINT32 preDiv, UINT32 postDiv);
BOOL DDKClockSetCKO(BOOL bEnable, DDK_CLOCK_CKO_SRC src, DDK_CLOCK_CKO_DIV div);
BOOL DDKClockSetpointRequest(DDK_DVFC_SETPOINT setpoint, BOOL bBlock);
BOOL DDKClockSetpointRelease(DDK_DVFC_SETPOINT setpoint);
BOOL DDKClockEnablePanicMode(void);
BOOL DDKClockDisablePanicMode(void);
 
#endif // __MX31_DDK_H
