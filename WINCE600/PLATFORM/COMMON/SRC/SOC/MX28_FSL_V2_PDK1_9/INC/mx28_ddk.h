//------------------------------------------------------------------------------
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
//  Copyright (C) 2007-2009 Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header: mx28_ddk.h
//------------------------------------------------------------------------------
#ifndef __MX28_DDK_H
#define __MX28_DDK_H

#include "common_ddkvs.h"

#if __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------------------------
//
//  Type: DDK_IOMUX_PIN
//
//  Specifies the functional pin name used to configure the HW_PINCTRL_MUXSELn.
//  Function will use
//-----------------------------------------------------------------------------

#define DDK_IOMUX_GET_BANK(x)  (x >> 5)
#define DDK_IOMUX_GET_PIN(x)   (x & 0x1F)
#define DDK_IOMUX_GET_BIT(x)   (1 << ( x & 0x1F ))

//-----------------------------------------------------------------------------
//
//  Type: DDK_GPIO_PORT
//
//  Specifies the GPIO module instance.  Note that not all instances will
//  be available on all platforms.
//
//-----------------------------------------------------------------------------
typedef enum
{
    DDK_GPIO_BANK0      = 0,
    DDK_GPIO_BANK1      = 1,
    DDK_GPIO_BANK2      = 2,
    DDK_GPIO_BANK3      = 3,
    DDK_GPIO_BANK4      = 4,    
    DDK_GPIO_BANK5      = 5, 
    DDK_GPIO_BANK6      = 6, 
} DDK_GPIO_BANK;


// DDK_GPIO_CFG(struct) bits
//Set pin to INPUT
#define DDK_GPIO_INPUT           (0)
//Set pin to be an OUTPUT
#define DDK_GPIO_OUTPUT          (1)
//Enable pin for interrupts
#define DDK_GPIO_IRQ_CAPABLE     (1)
//Pin does not generate interrupts
#define DDK_GPIO_IRQ_INCAPABLE   (0)
// Allow pin to generate interrupts
#define DDK_GPIO_IRQ_ENABLED     (1)
// Prevent pin from generating interrupts
#define DDK_GPIO_IRQ_DISABLED    (0)
// Trigger on level
#define DDK_GPIO_IRQ_LEVEL       (1)
// Trigger on edge
#define DDK_GPIO_IRQ_EDGE        (0)
// Trigger on high/rising edge
#define DDK_GPIO_IRQ_POLARITY_HI (1)
// Trigger on low/falling edge
#define DDK_GPIO_IRQ_POLARITY_LO (0)


//-----------------------------------------------------------------------------
//
//  Type: DDK_IOMUX_PIN
//
//  Specifies the functional pin name used to configure the HW_PINCTRL_MUXSELn.
//  Function will use DDK_IOMUX_GET_BANK(x),DDK_IOMUX_GET_PIN(x),DDK_IOMUX_GET_BIT(x)
//  macro for determine BANK , PIN and BIT number
//-----------------------------------------------------------------------------
typedef enum _DDK_IOMUX_PIN
{
    // BANK 0 MUXSEL0(pins 0-15)
    DDK_IOMUX_GPMI_D00                 =(0x0),
    DDK_IOMUX_SSP1_D0_0                =(0x0),
    DDK_IOMUX_GPMI_D01                 =(0x1),
    DDK_IOMUX_SSP1_D1                  =(0x1),
    DDK_IOMUX_GPMI_D02                 =(0x2),
    DDK_IOMUX_SSP1_D2                  =(0x2),
    DDK_IOMUX_GPMI_D03                 =(0x3),
    DDK_IOMUX_SSP1_D3_0                =(0x3),
    DDK_IOMUX_GPMI_D04                 =(0x4),
    DDK_IOMUX_SSP1_D4                  =(0x4),
    DDK_IOMUX_GPMI_D05                 =(0x5),
    DDK_IOMUX_SSP1_D5                  =(0x5),
    DDK_IOMUX_GPMI_D06                 =(0x6),
    DDK_IOMUX_SSP1_D6                  =(0x6),
    DDK_IOMUX_GPMI_D07                 =(0x7),
    DDK_IOMUX_SSP1_D7                  =(0x7),
    
    // BANK 0 MUXSEL0(pins 16-31)
    DDK_IOMUX_GPMI_CE0N                =(0x10),
    DDK_IOMUX_SSP3_D0_0                =(0x10),
    DDK_IOMUX_GPMI_CE1N                =(0x11),
    DDK_IOMUX_SSP3_D3_0                =(0x11),
    DDK_IOMUX_GPIO0_17                 =(0x11),
    DDK_IOMUX_GPMI_CE2N                =(0x12),
    DDK_IOMUX_CAN1_TX_0                =(0x12),
    DDK_IOMUX_ENET0_RX_ER_0            =(0x12),
    DDK_IOMUX_GPMI_CE3N                =(0x13),
    DDK_IOMUX_CAN1_RX_0                =(0x13),
    DDK_IOMUX_SAIF1_MCLK_0             =(0x13),
    DDK_IOMUX_GPMI_READY0              =(0x14),
    DDK_IOMUX_SSP1_CARD_DETECT         =(0x14),
    DDK_IOMUX_USB0_ID_0                =(0x14),
    DDK_IOMUX_GPMI_READY1              =(0x15),
    DDK_IOMUX_SSP1_CMD_0               =(0x15),
    DDK_IOMUX_GPIO0_21                 =(0x15),
    DDK_IOMUX_GPMI_READY2              =(0x16),
    DDK_IOMUX_CAN0_TX_0                =(0x16),
    DDK_IOMUX_ENENT0_TX_ER             =(0x16),
    DDK_IOMUX_GPMI_READY3              =(0x17),
    DDK_IOMUX_CAN0_RX_0                =(0x17),
    DDK_IOMUX_HSADC_TRIGGER_0          =(0x17),
    DDK_IOMUX_GPMI_RDN                 =(0x18),
    DDK_IOMUX_SSP3_SCK_0               =(0x18),
    DDK_IOMUX_GPMI_WRN                 =(0x19),
    DDK_IOMUX_SSP1_SCK_0               =(0x19),
    DDK_IOMUX_GPMI_ALE                 =(0x1A),
    DDK_IOMUX_SSP3_D1_0                =(0x1A),
    DDK_IOMUX_SSP3_D4_0                =(0x1A),
    DDK_IOMUX_GPMI_CLE                 =(0x1B),
    DDK_IOMUX_SSP3_D2_0                =(0x1B),
    DDK_IOMUX_SSP3_D5_0                =(0x1B),
    DDK_IOMUX_GPMI_RESETN              =(0x1C),
    DDK_IOMUX_SSP3_CMD_0               =(0x1C),
    DDK_IOMUX_GPIO0_28                 =(0x1C),

    // BANK 1 MUXSEL0(pins 0-15)
    DDK_IOMUX_LCD_D0                   =(0x20),
    DDK_IOMUX_ETM_DA0_0                =(0x20),
    DDK_IOMUX_LCD_D1                   =(0x21),
    DDK_IOMUX_ETM_DA1_0                =(0x21),
    DDK_IOMUX_LCD_D2                   =(0x22),
    DDK_IOMUX_ETM_DA2_0                =(0x22),
    DDK_IOMUX_LCD_D3                   =(0x23),
    DDK_IOMUX_ETM_DA8_0                =(0x23),
    DDK_IOMUX_ETM_DA3_0                =(0x23),
    DDK_IOMUX_LCD_D4                   =(0x24),
    DDK_IOMUX_ETM_DA9_0                =(0x24),
    DDK_IOMUX_ETM_DA4_0                =(0x24),
    DDK_IOMUX_LCD_D5                   =(0x25),
    DDK_IOMUX_ETM_DA5_0                =(0x25),
    DDK_IOMUX_LCD_D6                   =(0x26),
    DDK_IOMUX_ETM_DA6_0                =(0x26),
    DDK_IOMUX_LCD_D7                   =(0x27),
    DDK_IOMUX_ETM_DA7_0                =(0x27),
    DDK_IOMUX_LCD_D8                   =(0x28),
    DDK_IOMUX_ETM_DA3_1                =(0x28),
    DDK_IOMUX_ETM_DA8_1                =(0x28),
    DDK_IOMUX_GPIO1_8	               =(0x28),
    DDK_IOMUX_LCD_D9                   =(0x29),
    DDK_IOMUX_ETM_DA4_1                =(0x29),
    DDK_IOMUX_ETM_DA9_1                =(0x29),
    DDK_IOMUX_GPIO1_9	               =(0x29),
    DDK_IOMUX_LCD_D10                  =(0x2A),
    DDK_IOMUX_ETM_DA10                 =(0x2A),
    DDK_IOMUX_LCD_D11                  =(0x2B),
    DDK_IOMUX_ETM_DA11                 =(0x2B),
    DDK_IOMUX_LCD_D12                  =(0x2C),
    DDK_IOMUX_ETM_DA12                 =(0x2C),
    DDK_IOMUX_LCD_D13                  =(0x2D),
    DDK_IOMUX_ETM_DA13                 =(0x2D),
    DDK_IOMUX_LCD_D14                  =(0x2E),
    DDK_IOMUX_ETM_DA14                 =(0x2E),
    DDK_IOMUX_LCD_D15                  =(0x2F),
    DDK_IOMUX_ETM_DA15                 =(0x2F),
    
    // BANK 1 MUXSEL0(pins 16-31)
    DDK_IOMUX_LCD_D16                  =(0x30),
    DDK_IOMUX_ETM_DA7_1                =(0x30),
    DDK_IOMUX_GPIO1_16                 =(0x30),
    DDK_IOMUX_LCD_D17                  =(0x31),
    DDK_IOMUX_ETM_DA6_1                =(0x31),
    DDK_IOMUX_GPIO1_17                 =(0x31),
    DDK_IOMUX_LCD_D18                  =(0x32),
    DDK_IOMUX_ETM_DA5_1                =(0x32),
    DDK_IOMUX_LCD_D19                  =(0x33),
    DDK_IOMUX_ETM_DA4_2                =(0x33),
    DDK_IOMUX_LCD_D20                  =(0x34),
    DDK_IOMUX_ENET1_1588_EVENT2_OUT    =(0x34),
    DDK_IOMUX_ETM_DA3_2                =(0x34),
    DDK_IOMUX_LCD_D21                  =(0x35),
    DDK_IOMUX_ENET1_1588_EVENT2_IN     =(0x35),
    DDK_IOMUX_ETM_DA2_1                =(0x35),
    DDK_IOMUX_LCD_D22                  =(0x36),
    DDK_IOMUX_ENET1_1588_EVENT3_OUT    =(0x36),
    DDK_IOMUX_ETM_DA1_1                =(0x36),
    DDK_IOMUX_LCD_D23                  =(0x37),
    DDK_IOMUX_ENET1_1588_EVENT3_IN     =(0x37),
    DDK_IOMUX_ETM_DA0_1                =(0x37),
    DDK_IOMUX_LCD_RD_E                 =(0x38),
    DDK_IOMUX_LCD_VSYNC_0              =(0x38),
    DDK_IOMUX_ETM_TCTL_0               =(0x38),
    DDK_IOMUX_LCD_WR_RWN               =(0x39),
    DDK_IOMUX_LCD_HSYNC_0              =(0x39),
    DDK_IOMUX_ETM_TCLK_0               =(0x39),
    DDK_IOMUX_LCD_RS                   =(0x3A),
    DDK_IOMUX_LCD_DOTCLK_0             =(0x3A),
    DDK_IOMUX_LCD_CS                   =(0x3B),
    DDK_IOMUX_LCD_ENABLE_0             =(0x3B),
    DDK_IOMUX_LCD_VSYNC_1              =(0x3C),
    DDK_IOMUX_SAIF1_SDATA0_0           =(0x3C),
    DDK_IOMUX_LCD_HSYNC_1              =(0x3D),
    DDK_IOMUX_SAIF1_SDATA1_0           =(0x3D),
    DDK_IOMUX_ETM_TCTL_1               =(0x3D),
    DDK_IOMUX_LCD_DOTCLK_1             =(0x3E),
    DDK_IOMUX_SAIF1_MCLK_1             =(0x3E),
    DDK_IOMUX_ETM_TCLK_1               =(0x3E),
    DDK_IOMUX_LCD_ENABLE_1             =(0x3F),

    // BANK 2 MUXSEL0(pins 0-15)
    DDK_IOMUX_SSP0_D0                  =(0x40),
    DDK_IOMUX_GPIO2_0                  =(0x40),
    DDK_IOMUX_SSP0_D1                  =(0x41),
    DDK_IOMUX_GPIO2_1                  =(0x41),
    DDK_IOMUX_SSP0_D2                  =(0x42),
    DDK_IOMUX_GPIO2_2                  =(0x42),
    DDK_IOMUX_SSP0_D3                  =(0x43),
    DDK_IOMUX_GPIO2_3                  =(0x43),
    DDK_IOMUX_SSP0_D4                  =(0x44),
    DDK_IOMUX_SSP2_D0_0                =(0x44),
    DDK_IOMUX_GPIO2_4                  =(0x44),
    DDK_IOMUX_SSP0_D5                  =(0x45),
    DDK_IOMUX_SSP2_D3_0                =(0x45),
    DDK_IOMUX_GPIO2_5                  =(0x45),
    DDK_IOMUX_SSP0_D6                  =(0x46),
    DDK_IOMUX_SSP2_CMD_0               =(0x46),
    DDK_IOMUX_GPIO2_6                  =(0x46),
    DDK_IOMUX_SSP0_D7                  =(0x47),
    DDK_IOMUX_SSP2_SCK_0               =(0x47),
    DDK_IOMUX_GPIO2_7                  =(0x47),
    DDK_IOMUX_SSP0_CMD                 =(0x48),
    DDK_IOMUX_GPIO2_8                  =(0x48),
    DDK_IOMUX_SSP0_CARD_DETECT         =(0x49),
    DDK_IOMUX_GPIO2_9                  =(0x49),
    DDK_IOMUX_SSP0_SCK                 =(0x4A),
    DDK_IOMUX_GPIO2_10                 =(0x4A),
    DDK_IOMUX_SSP1_SCK_1               =(0x4C),
    DDK_IOMUX_SSP2_D1_0                =(0x4C),
    DDK_IOMUX_ENET0_1588_EVENT2_OUT_0  =(0x4C),
    DDK_IOMUX_SSP1_CMD_1               =(0x4D),
    DDK_IOMUX_SSP2_D2_0                =(0x4D),
    DDK_IOMUX_ENET0_1588_EVENT2_IN_0   =(0x4D),
    DDK_IOMUX_SSP1_D0_1                =(0x4E),
    DDK_IOMUX_SSP2_D6                  =(0x4E),
    DDK_IOMUX_ENET0_1588_EVENT3_OUT_0  =(0x4E),
    DDK_IOMUX_SSP1_D3_1                =(0x4F),
    DDK_IOMUX_SSP2_D7                  =(0x4F),
    DDK_IOMUX_ENET0_1588_EVENT3_IN_0   =(0x4F),

    // BANK 2 MUXSEL0(pins 16-31)
    DDK_IOMUX_SSP2_SCK_1               =(0x50),
    DDK_IOMUX_AUART2_RX_0              =(0x50),
    DDK_IOMUX_SAIF0_SDATA1_0           =(0x50),
	DDK_IOMUX_GPIO2_16				   =(0x50),
    DDK_IOMUX_SSP2_CMD_1               =(0x51),
    DDK_IOMUX_AUART2_TX_0              =(0x51),
    DDK_IOMUX_SAIF0_SDATA2_0           =(0x51),
	DDK_IOMUX_GPIO2_17				   =(0x51),
    DDK_IOMUX_SSP2_D0_1                =(0x52),
    DDK_IOMUX_AUART3_RX_0              =(0x52),
    DDK_IOMUX_SAIF1_SDATA1_1           =(0x52),
	DDK_IOMUX_GPIO2_18				   =(0x52),
    DDK_IOMUX_SSP2_D3_1                =(0x53),
    DDK_IOMUX_AUART3_TX_0              =(0x53),
    DDK_IOMUX_SAIF1_SDATA2_0           =(0x53),
	DDK_IOMUX_GPIO2_19				   =(0x53),
    DDK_IOMUX_SSP2_D4                  =(0x54),
    DDK_IOMUX_SSP2_D1_1                =(0x54),
    DDK_IOMUX_USB1_OVERCURRENT_0       =(0x54),
	DDK_IOMUX_GPIO2_20				   =(0x54),
    DDK_IOMUX_SSP2_D5                  =(0x55),
    DDK_IOMUX_SSP2_D2_1                =(0x55),
    DDK_IOMUX_USB0_OVERCURRENT_0       =(0x55),
    DDK_IOMUX_SSP3_SCK_1               =(0x58),
    DDK_IOMUX_AUART4_TX_0              =(0x58),
    DDK_IOMUX_ENET1_1588_EVENT0_OUT    =(0x58),
    DDK_IOMUX_SSP3_CMD_1               =(0x59),
    DDK_IOMUX_AUART4_RX_0              =(0x59),
    DDK_IOMUX_ENET1_1588_EVENT0_IN     =(0x59),
    DDK_IOMUX_SSP3_D0_1                =(0x5A),
    DDK_IOMUX_AUART4_RTS_0             =(0x5A),
    DDK_IOMUX_ENET1_1588_EVENT1_OUT    =(0x5A),
    DDK_IOMUX_SSP3_D3_1                =(0x5B),
    DDK_IOMUX_AUART4_CTS_0             =(0x5B),
    DDK_IOMUX_ENET1_1588_EVENT1_IN     =(0x5B),

    // BANK 3 MUXSEL0(pins 0-15)
    DDK_IOMUX_AUART0_RX                =(0x60),
    DDK_IOMUX_I2C0_SCL_0               =(0x60),
    DDK_IOMUX_DUART_CTS                =(0x60),
    DDK_IOMUX_AUART0_TX                =(0x61),
    DDK_IOMUX_I2C0_SDA_0               =(0x61),
    DDK_IOMUX_DUART_RTS                =(0x61),
    DDK_IOMUX_AUART0_CTS               =(0x62),
    DDK_IOMUX_AUART4_RX_1              =(0x62),
    DDK_IOMUX_DUART_RX_0               =(0x62),
    DDK_IOMUX_AUART0_RTS               =(0x63),
    DDK_IOMUX_AUART4_TX_1              =(0x63),
    DDK_IOMUX_DUART_TX_0               =(0x63),
    DDK_IOMUX_AUART1_RX                =(0x64),
    DDK_IOMUX_SSP2_CARD_DETECT         =(0x64),
    DDK_IOMUX_PWM0_0                   =(0x64),
    DDK_IOMUX_GPIO3_4                  =(0x64),
    DDK_IOMUX_AUART1_TX                =(0x65),
    DDK_IOMUX_SSP3_CARD_DETECT         =(0x65),
    DDK_IOMUX_PWM1_0                   =(0x65),
    DDK_IOMUX_GPIO3_5                  =(0x65),
    DDK_IOMUX_AUART1_CTS               =(0x66),
    DDK_IOMUX_USB0_OVERCURRENT_1       =(0x66),
    DDK_IOMUX_TIMROT_ROTARYA_0         =(0x66),
    DDK_IOMUX_AUART1_RTS               =(0x67),
    DDK_IOMUX_USB0_ID_1                =(0x67),
    DDK_IOMUX_TIMROT_ROTARYB_0         =(0x67),
    DDK_IOMUX_AUART2_RX_1              =(0x68),
    DDK_IOMUX_SSP3_D1_1                =(0x68),
    DDK_IOMUX_SSP3_D4_1                =(0x68),
    DDK_IOMUX_AUART2_TX_1              =(0x69),
    DDK_IOMUX_SSP3_D2_1                =(0x69),
    DDK_IOMUX_SSP3_D5_1                =(0x69),
    DDK_IOMUX_AUART2_CTS               =(0x6A),
    DDK_IOMUX_I2C1_SCL_0               =(0x6A),
    DDK_IOMUX_SAIF1_BITCLK             =(0x6A),
    DDK_IOMUX_AUART2_RTS               =(0x6B),
    DDK_IOMUX_I2C1_SDA_0               =(0x6B),
    DDK_IOMUX_SAIF1_LRCLK              =(0x6B),
    DDK_IOMUX_AUART3_RX_1              =(0x6C),
    DDK_IOMUX_CAN0_TX_1                =(0x6C),
    DDK_IOMUX_ENET0_1588_EVENT0_OUT_0  =(0x6C),
    DDK_IOMUX_AUART3_TX_1              =(0x6D),
    DDK_IOMUX_CAN0_RX_1                =(0x6D),
    DDK_IOMUX_ENET0_1588_EVENT0_IN_0   =(0x6D),
    DDK_IOMUX_AUART3_CTS               =(0x6E),
    DDK_IOMUX_CAN1_TX_1                =(0x6E),
    DDK_IOMUX_ENET0_1588_EVENT1_OUT_0  =(0x6E),
    DDK_IOMUX_AUART3_RTS               =(0x6F),
    DDK_IOMUX_CAN1_RX_1                =(0x6F),
    DDK_IOMUX_ENET0_1588_EVENT1_IN_0   =(0x6F),

    // BANK 3 MUXSEL0(pins 16-31)
    DDK_IOMUX_PWM0_1                   =(0x70),
    DDK_IOMUX_I2C1_SCL_1               =(0x70),
    DDK_IOMUX_DUART_RX_1               =(0x70),
    DDK_IOMUX_GPIO3_16                 =(0x70),
    DDK_IOMUX_PWM1_1                   =(0x71),
    DDK_IOMUX_I2C1_SDA_1               =(0x71),
    DDK_IOMUX_DUART_TX_1               =(0x71),
    DDK_IOMUX_GPIO3_17                 =(0x71),
    DDK_IOMUX_PWM2                     =(0x72),
    DDK_IOMUX_USB0_ID_2                =(0x72),
    DDK_IOMUX_USB1_OVERCURRENT_1       =(0x72),
    DDK_IOMUX_SAIF0_MCLK               =(0x74),
    DDK_IOMUX_PWM3_0                   =(0x74),
    DDK_IOMUX_AUART4_CTS_1             =(0x74),
    DDK_IOMUX_SAIF0_LRCLK              =(0x75),
    DDK_IOMUX_PWM4_0                   =(0x75),
    DDK_IOMUX_AUART4_RTS_1             =(0x75),
    DDK_IOMUX_SAIF0_BITCLK             =(0x76),
    DDK_IOMUX_PWM5                     =(0x76),
    DDK_IOMUX_AUART4_RX_2              =(0x76),
    DDK_IOMUX_SAIF0_SDATA0             =(0x77),
    DDK_IOMUX_PWM6                     =(0x77),
    DDK_IOMUX_AUART4_TX_2              =(0x77),
    DDK_IOMUX_I2C0_SCL_1               =(0x78),
    DDK_IOMUX_TIMROT_ROTARYA_1         =(0x78),
    DDK_IOMUX_DUART_RX_2               =(0x78),
    DDK_IOMUX_GPIO3_24                 =(0x78),
    DDK_IOMUX_I2C0_SDA_1               =(0x79),
    DDK_IOMUX_TIMROT_ROTARYB_1         =(0x79),
    DDK_IOMUX_DUART_TX_2               =(0x79),
    DDK_IOMUX_GPIO3_25                 =(0x79),
    DDK_IOMUX_SAIF1_SDATA0_1           =(0x7A),
    DDK_IOMUX_PWM7                     =(0x7A),
    DDK_IOMUX_SAIF0_SDATA1_1           =(0x7A),
    DDK_IOMUX_SPDIF_TX                 =(0x7B),
    DDK_IOMUX_ENET1_RX_ER              =(0x7B),
    DDK_IOMUX_PWM3_1                   =(0x7C),
    DDK_IOMUX_GPIO3_28                 =(0x7C),
    DDK_IOMUX_PWM4_1                   =(0x7D),
    DDK_IOMUX_GPIO3_29                 =(0x7D),
    DDK_IOMUX_LCD_RESET                =(0x7E),
    DDK_IOMUX_LCD_VSYNC                =(0x7E),
    DDK_IOMUX_GPIO3_30                 =(0x7E),

    // BANK 4 MUXSEL0(pins 0-15)
    DDK_IOMUX_ENET0_MDC                =(0x80),
    DDK_IOMUX_GPMI_CE4N                =(0x80),
    DDK_IOMUX_SAIF0_SDATA1_2           =(0x80),
    DDK_IOMUX_ENET0_MDIO               =(0x81),
    DDK_IOMUX_GPMI_CE5N                =(0x81),
    DDK_IOMUX_SAIF0_SDATA2_1           =(0x81),
    DDK_IOMUX_ENET0_RX_EN              =(0x82),
    DDK_IOMUX_GPMI_CE6N                =(0x82),
    DDK_IOMUX_SAIF1_SDATA1_2           =(0x82),
    DDK_IOMUX_ENET0_RXD0               =(0x83),
    DDK_IOMUX_GPMI_CE7N                =(0x83),
    DDK_IOMUX_SAIF1_SDATA2_1           =(0x83),
    DDK_IOMUX_ENET0_RXD1               =(0x84),
    DDK_IOMUX_GPMI_READY4              =(0x84),
    DDK_IOMUX_ENET0_TX_CLK             =(0x85),
    DDK_IOMUX_HSADC_TRIGGER_1          =(0x85),
    DDK_IOMUX_ENET0_1588_EVENT2_OUT_1  =(0x85),
    DDK_IOMUX_ENET0_TX_EN              =(0x86),
    DDK_IOMUX_GPMI_READY5              =(0x86),
    DDK_IOMUX_ENET0_TXD0               =(0x87),
    DDK_IOMUX_GPMI_READY6              =(0x87),
    DDK_IOMUX_ENET0_TXD1               =(0x88),
    DDK_IOMUX_GPMI_READY7              =(0x88),
    DDK_IOMUX_ENET0_RXD2               =(0x89),
    DDK_IOMUX_ENET1_RXD0               =(0x89),
    DDK_IOMUX_ENET0_1588_EVENT0_OUT_1  =(0x89),
    DDK_IOMUX_ENET0_RXD3               =(0x8A),
    DDK_IOMUX_ENET1_RXD1               =(0x8A),
    DDK_IOMUX_ENET0_1588_EVENT0_IN_1   =(0x8A),
    DDK_IOMUX_ENET0_TXD2               =(0x8B),
    DDK_IOMUX_ENET1_TXD0               =(0x8B),
    DDK_IOMUX_ENET0_1588_EVENT1_OUT_1  =(0x8B),
    DDK_IOMUX_ENET0_TXD3               =(0x8C),
    DDK_IOMUX_ENET1_TXD1               =(0x8C),
    DDK_IOMUX_ENET0_1588_EVENT1_IN_1   =(0x8C),
    DDK_IOMUX_ENET0_RX_CLK             =(0x8D),
    DDK_IOMUX_ENET0_RX_ER_1            =(0x8D),
    DDK_IOMUX_ENET0_1588_EVENT2_IN_1   =(0x8D),
    DDK_IOMUX_ENET0_COL                =(0x8E),
    DDK_IOMUX_ENET1_TX_EN              =(0x8E),
    DDK_IOMUX_ENET0_1588_EVENT3_OUT_1  =(0x8E),
    DDK_IOMUX_ENET0_CRS                =(0x8F),
    DDK_IOMUX_ENET1_RX_EN              =(0x8F),
    DDK_IOMUX_ENET0_1588_EVENT3_IN_1   =(0x8F),

    // BANK 4 MUXSEL0(pins 16-31)
    DDK_IOMUX_CLKCTRL_ENET             =(0x90),
    DDK_IOMUX_JTAG_RTCK                =(0x94),
    DDK_IOMUX_GPIO4_20                 =(0x94),
    
    // BANK 5 MUXSEL0(pins 0-15)
    DDK_IOMUX_EMI_DATA0                =(0xA0),
    DDK_IOMUX_EMI_DATA1                =(0xA1),
    DDK_IOMUX_EMI_DATA2                =(0xA2),
    DDK_IOMUX_EMI_DATA3                =(0xA3),
    DDK_IOMUX_EMI_DATA4                =(0xA4),
    DDK_IOMUX_EMI_DATA5                =(0xA5),
    DDK_IOMUX_EMI_DATA6                =(0xA6),
    DDK_IOMUX_EMI_DATA7                =(0xA7),
    DDK_IOMUX_EMI_DATA8                =(0xA8),
    DDK_IOMUX_EMI_DATA9                =(0xA9),
    DDK_IOMUX_EMI_DATA10               =(0xAA),
    DDK_IOMUX_EMI_DATA11               =(0xAB),
    DDK_IOMUX_EMI_DATA12               =(0xAC),
    DDK_IOMUX_EMI_DATA13               =(0xAD),
    DDK_IOMUX_EMI_DATA14               =(0xAE),
    DDK_IOMUX_EMI_DATA15               =(0xAF),

    // BANK 5 MUXSEL0(pins 16-31)
    DDK_IOMUX_EMI_ODT0                 =(0xB0),
    DDK_IOMUX_EMI_DQM0                 =(0xB1),
    DDK_IOMUX_EMI_ODT1                 =(0xB2),
    DDK_IOMUX_EMI_DQM1                 =(0xB3),
    DDK_IOMUX_EMI_DDR_OPEN_FEEDBACK    =(0xB4),
    DDK_IOMUX_EMI_CLK                  =(0xB5),
    DDK_IOMUX_EMI_DQS0                 =(0xB6),
    DDK_IOMUX_EMI_DQS1                 =(0xB7),
    DDK_IOMUX_EMI_DDR_OPEN             =(0xBA),

    // BANK 6 MUXSEL0(pins 0-15)
    DDK_IOMUX_EMI_ADDR0                =(0xC0),
    DDK_IOMUX_EMI_ADDR1                =(0xC1),
    DDK_IOMUX_EMI_ADDR2                =(0xC2),
    DDK_IOMUX_EMI_ADDR3                =(0xC3),
    DDK_IOMUX_EMI_ADDR4                =(0xC4),
    DDK_IOMUX_EMI_ADDR5                =(0xC5),
    DDK_IOMUX_EMI_ADDR6                =(0xC6),
    DDK_IOMUX_EMI_ADDR7                =(0xC7),
    DDK_IOMUX_EMI_ADDR8                =(0xC8),
    DDK_IOMUX_EMI_ADDR9                =(0xC9),
    DDK_IOMUX_EMI_ADDR10               =(0xCA),
    DDK_IOMUX_EMI_ADDR11               =(0xCB),
    DDK_IOMUX_EMI_ADDR12               =(0xCC),
    DDK_IOMUX_EMI_ADDR13               =(0xCD),
    DDK_IOMUX_EMI_ADDR14               =(0xCE),

    // BANK 6 MUXSEL0(pins 16-31)
    DDK_IOMUX_EMI_BA0                  =(0xD0),
    DDK_IOMUX_EMI_BA1                  =(0xD1),
    DDK_IOMUX_EMI_BA2                  =(0xD2),
    DDK_IOMUX_EMI_CASN                 =(0xD3),
    DDK_IOMUX_EMI_RASN                 =(0xD4),
    DDK_IOMUX_EMI_WEN                  =(0xD5),
    DDK_IOMUX_EMI_CE0N                 =(0xD6),
    DDK_IOMUX_EMI_CE1N                 =(0xD7),
    DDK_IOMUX_EMI_CKE                  =(0xD8),

    DDK_IOMUX_GPIO_MAX_ID              =(224),
    DDK_IOMUX_GPIO_END_TABLE           =(255)
} DDK_IOMUX_PIN;

//-----------------------------------------------------------------------------
//
//  Type: DDK_IOMUX_PIN_MUXMODE
//
//  Specifies the mode to configure the Pin.
//
//-----------------------------------------------------------------------------
typedef enum _DDK_IOMUX_PIN_MUXMODE
{
    DDK_IOMUX_MODE_00               = 0,
    DDK_IOMUX_MODE_01               = 1,
    DDK_IOMUX_MODE_02               = 2,
    DDK_IOMUX_MODE_GPIO             = 3
} DDK_IOMUX_PIN_MUXMODE;

//-----------------------------------------------------------------------------
typedef struct _DDK_GPIO_CFG
{
    //pin direction, 1=Output or 0=Input
    unsigned DDK_PIN_IO : 1 ;
    // pin capable to trigger interrupt request; must be zero if pin_IO==1
    unsigned DDK_PIN_IRQ_CAPABLE : 1 ;
    // pin enabled as interrupt src; valid only if _pin_irq_capable=1
    unsigned DDK_PIN_IRQ_ENABLE : 1 ;
    //pin trigger mode 1=level or 0=edge; valid only if pin_irq_capable=1
    unsigned DDK_PIN_IRQ_LEVEL : 1 ;
    // pin trigger polarity 1=hi/rising or 0=low/falling; valid only if pin_irq_capable=1
    unsigned DDK_PIN_IRQ_POLARITY : 1 ;
    // unused bits
    unsigned DDK_PIN_RESERVED : 3 ;
} DDK_GPIO_CFG;   //GPIO block configuration structure, one per GPIO pin

//-----------------------------------------------------------------------------
//
//  Type: DDK_IOMUX_PAD_DRIVE
//
//  Specifies the drive strength for a pad.
//
//-----------------------------------------------------------------------------
typedef enum
{
    DDK_IOMUX_PAD_DRIVE_4MA         = 0,
    DDK_IOMUX_PAD_DRIVE_8MA         = 1,
    DDK_IOMUX_PAD_DRIVE_12MA        = 2,
    DDK_IOMUX_PAD_DRIVE_RESERVED    = 3
} DDK_IOMUX_PAD_DRIVE;

//-----------------------------------------------------------------------------
//
//  Type: DDK_IOMUX_PAD_PULL
//
//  Specifies the pull-up/pull-down/keeper configuration for a pad.
//
//-----------------------------------------------------------------------------
typedef enum
{
    DDK_IOMUX_PAD_PULL_ENABLE       = 0,
    DDK_IOMUX_PAD_PULL_DISABLE      = 1
} DDK_IOMUX_PAD_PULL;

//-----------------------------------------------------------------------------
//
//  Type: DDK_IOMUX_PAD_VOLTAGE
//
//  Specifies the driver voltage for a pad.
//
//-----------------------------------------------------------------------------
typedef enum
{
    DDK_IOMUX_PAD_VOLTAGE_RESERVED      = 0,
    DDK_IOMUX_PAD_VOLTAGE_1V8           = 0,
    DDK_IOMUX_PAD_VOLTAGE_3V3           = 1
} DDK_IOMUX_PAD_VOLTAGE;

//------------------------------------------------------------------------------

// DDK GPIO
BOOL DDKGpioConfig(DDK_IOMUX_PIN gpio_pin,DDK_GPIO_CFG gpio_cfg);
BOOL DDKGpioWriteDataPin(DDK_IOMUX_PIN pin,UINT32 Data);
BOOL DDKGpioReadData(DDK_GPIO_BANK bank, UINT32 *pData);
BOOL DDKGpioReadDataPin(DDK_IOMUX_PIN pin, UINT32 *pData);
BOOL DDKGpioReadIntrPin(DDK_IOMUX_PIN pin,UINT32 *pData);
BOOL DDKGpioEnableDataPin(DDK_IOMUX_PIN pin,UINT32 Data);
VOID DDKGpioClearIntrPin(DDK_IOMUX_PIN gpio_pin);

//
BOOL DDKIomuxSetPinMux(DDK_IOMUX_PIN pin, DDK_IOMUX_PIN_MUXMODE muxmode);
BOOL DDKIomuxGetPinMux(DDK_IOMUX_PIN pin, DDK_IOMUX_PIN_MUXMODE *pMuxmode);
BOOL DDKIomuxSetPadConfig(DDK_IOMUX_PIN pin,DDK_IOMUX_PAD_DRIVE drive,DDK_IOMUX_PAD_PULL pull,DDK_IOMUX_PAD_VOLTAGE voltage);
BOOL DDKIomuxGetPadConfig(DDK_IOMUX_PIN pin,DDK_IOMUX_PAD_DRIVE *pDrive,DDK_IOMUX_PAD_PULL *pPull,DDK_IOMUX_PAD_VOLTAGE *pVoltage);

//------------------------------------------------------------------------------
// DMA Module

//-----------------------------------------------------------------------------
//
//  Type: DDK_APBH_BURST_SIZE
//
//  Specifies the burst size for APBH.
//
//-----------------------------------------------------------------------------
typedef enum
{
    DDK_APBH_BURST_SIZE_BURST0      = 0,
    DDK_APBH_BURST_SIZE_BURST4      = 1,
    DDK_APBH_BURST_SIZE_BURST8      = 2
} DDK_APBH_BURST_SIZE;


//------------------------------------------------------------------------------
// APBH Channel number

#define DDK_MAX_APBH_CHANNEL             16

#define DDK_APBH_CHANNEL_SSP0            0x00
#define DDK_APBH_CHANNEL_SSP1            0x01
#define DDK_APBH_CHANNEL_SSP2            0x02
#define DDK_APBH_CHANNEL_SSP3            0x03
#define DDK_APBH_CHANNEL_GPMI0           0x04
#define DDK_APBH_CHANNEL_GPMI1           0x05
#define DDK_APBH_CHANNEL_GPMI2           0x06
#define DDK_APBH_CHANNEL_GPMI3           0x07
#define DDK_APBH_CHANNEL_GPMI4           0x08
#define DDK_APBH_CHANNEL_GPMI5           0x09
#define DDK_APBH_CHANNEL_GPMI6           0x0A
#define DDK_APBH_CHANNEL_GPMI7           0x0B
#define DDK_APBH_CHANNEL_HSADC           0x0C
#define DDK_APBH_CHANNEL_LCDIF           0x0D
#define DDK_APBH_CHANNEL_EMPTY0          0x0E
#define DDK_APBH_CHANNEL_EMPTY1          0x0F

// APBX Channel number
#define DDK_MAX_APBX_CHANNEL             16

#define APBX_CHANNEL_AUART4_RX           0x00
#define APBX_CHANNEL_AUART4_TX           0x01
#define APBX_CHANNEL_SPDIF_TX            0x02
#define APBX_CHANNEL_EMPTY0              0x03
#define APBX_CHANNEL_SAIF0               0x04
#define APBX_CHANNEL_SAIF1               0x05
#define APBX_CHANNEL_I2C0                0x06
#define APBX_CHANNEL_I2C1                0x07
#define APBX_CHANNEL_AUART0_RX           0x08
#define APBX_CHANNEL_AUART0_TX           0x09
#define APBX_CHANNEL_AUART1_RX           0x0A
#define APBX_CHANNEL_AUART1_TX           0x0B
#define APBX_CHANNEL_AUART2_RX           0x0C
#define APBX_CHANNEL_AUART2_TX           0x0D
#define APBX_CHANNEL_AUART3_RX           0x0E
#define APBX_CHANNEL_AUART3_TX           0x0F

// DDK APBH Function
VOID DDKApbhDmaSetBurstSize(UINT32 Channel, DDK_APBH_BURST_SIZE bstsize);


//------------------------------------------------------------------------------
// Clock Control module

//------------------------------------------------------------------------------
// Types

//------------------------------------------------------------------------------
// Defines

//! \brief Value of 24MHz as kHz.
#define XTAL_24MHZ_IN_KHZ 24000
//! \brief Value of 24Mhz in Hz.
#define XTAL_24MHZ 24000000
//! \brief Value of 480MHz as kHz.
#define PLL_480MHZ_IN_KHZ 480000
//! \brief Value of 480Mhz in Hz.
#define PLL_480MHZ 480000000
//! \brief Maximum PLL frequeny in kHz.
#define MAX_PLL_KHZ 480000
//! \brief Minimum PLL frequncy in Khz allowed by SDK.
#define MIN_PLL_KHZ 24000
//! \brief The constant value in the PFD calculation.
#define PFD_DIV_CONSTANT 18
//! \brief Minimum divider value for Phase Fractional Dividers (PFD).
#define MIN_PFD_VALUE 18
//! \brief Maximum divider value for Phase Fractional Dividers (PFD).
#define MAX_PFD_VALUE 35
//! \brief Minimum frequency that will be generated by PLL.
#define MIN_PFD_FREQ_KHZ 246857
//! \brief Maximum frequency that will be generated by PLL.
#define MAX_PFD_FREQ_KHZ 480000
//! \brief Product of maximum PLL and minimum PFD value used in calculations
#define PFD_CONSTANT (MAX_PLL_KHZ * MIN_PFD_VALUE)
//! \brief Crystal divider to achieve 24MHz
#define DIVIDER_24MHZ 1
//! \brief TBD
#define HCLK_THRESHOLD 100000000
//! \brief TBD
#define HCLK_DIV_1 1
//! \brief TBD
#define HCLK_DIV_2 2
//! \brief Maximum CPU clock frequency allowed.
#define MAX_PCLK 480000
//! \brief Minimum CPU clock frequency allowed.
#define MIN_PCLK 1000
//! \brief Maximum HBUS clock divider allowed.
#define MAX_HCLK_DIV 31
//! \brief Minimum HBUS clock divider allowed.
#define MIN_HCLK_DIV 1
//! \brief Maximum EMI clock freqquency allowed.
#define MAX_EMICLK 480000
//! \brief Minimum EMI clock frequency allowed.
#define MIN_EMICLK 1500
//! \brief Maximum XBUS clock frequency allowed.
#define MAX_XCLK 24000
//! \brief Minumum XBUS clock frequency allowed. 
#define MIN_XCLK 23
//! \brief Maximum display clock frequency allowed.
#define MAX_PIXCLK 480000
//! \brief Minimum display clock frequency allowed.
#define MIN_PIXCLK 1000
//! \brief Minimum integer divider for default cases.  
#define MIN_INT_DIV 1


#define PWM_24MHZ          24000000
#define DRI_24MHZ          24000000
#define DIGCTL_CLK1MHZ     1000000
#define TIMROT_CLK32KHZ    32000
#define ADC_CLK2KHZ        2000
#define SPDIF_120MHZ       120000000


//Enumerates the reference signals from the PLL
typedef enum _CSP_clocks_pll_ref_clks_t
{
    //ref_cpu signal for cpu clock
    PLL_REF_CPU,
    //ref_emi signal for emi clock
    PLL_REF_EMI,
    //ref_io0 signal for ssp0, ssp1 clocks
    PLL_REF_IO0,
    //ref_io1 signal for ssp2, ssp3 clocks
    PLL_REF_IO1,    
    //ref_pix signal for display clock
    PLL_REF_PIX,
    //ref_hsadc signal for hsadc clocks
    PLL_REF_HSADC,
    //ref_gpmi signal for gmpi clocks
    PLL_REF_GPMI,
    //ref_enet signal for enet clocks
    PLL_REF_ENET
} CSP_clocks_pll_ref_clks_t;

//-----------------------------------------------------------------------------
//
//  Type: DDK_CLOCK_SIGNAL
//
//  Clock signal name for querying/setting clock configuration.
//
//-----------------------------------------------------------------------------
typedef enum
{
    DDK_CLOCK_SIGNAL_PLL0                 = 0,
    DDK_CLOCK_SIGNAL_PLL1                 = 1,
    DDK_CLOCK_SIGNAL_PLL2                 = 2,
    DDK_CLOCK_SIGNAL_REF_CPU              = 3,
    DDK_CLOCK_SIGNAL_REF_EMI              = 4,
    DDK_CLOCK_SIGNAL_REF_IO0              = 5,
    DDK_CLOCK_SIGNAL_REF_IO1              = 6, 
    DDK_CLOCK_SIGNAL_REF_PIX              = 7,
    DDK_CLOCK_SIGNAL_REF_HSADC            = 8,
    DDK_CLOCK_SIGNAL_REF_GPMI             = 9,
    DDK_CLOCK_SIGNAL_REF_PLL              = 10,
    DDK_CLOCK_SIGNAL_REF_XTAL             = 11,    
    DDK_CLOCK_SIGNAL_REF_ENET_PLL         = 12,
    DDK_CLOCK_SIGNAL_P_CLK                = 13,  
    DDK_CLOCK_SIGNAL_H_CLK                = 14,
    DDK_CLOCK_SIGNAL_H_CLK_FLEXCAN0_IPG   = 15,
    DDK_CLOCK_SIGNAL_H_CLK_FLEXCAN1_IPG   = 16,
    DDK_CLOCK_SIGNAL_H_CLK_FLEXCAN0       = 17,
    DDK_CLOCK_SIGNAL_H_CLK_FLEXCAN1       = 18,
    DDK_CLOCK_SIGNAL_H_CLK_ENET_SWI       = 19,
    DDK_CLOCK_SIGNAL_H_CLK_MAC0           = 20,
    DDK_CLOCK_SIGNAL_H_CLK_MAC1           = 21,
    DDK_CLOCK_SIGNAL_OCROM                = 22,
    DDK_CLOCK_SIGNAL_ETM                  = 23,
    DDK_CLOCK_SIGNAL_EMI                  = 24,
    DDK_CLOCK_SIGNAL_SSP0                 = 25,
    DDK_CLOCK_SIGNAL_SSP1                 = 26,
    DDK_CLOCK_SIGNAL_SSP2                 = 27,
    DDK_CLOCK_SIGNAL_SSP3                 = 28,
    DDK_CLOCK_SIGNAL_GPMI                 = 29,
    DDK_CLOCK_SIGNAL_SPDIF                = 30,
    DDK_CLOCK_SIGNAL_SAIF0                = 31,
    DDK_CLOCK_SIGNAL_SAIF1                = 32,
    DDK_CLOCK_SIGNAL_DIS_LCDIF            = 33,
    DDK_CLOCK_SIGNAL_HSADC                = 34,
    DDK_CLOCK_SIGNAL_ENET_TIME            = 35,
    DDK_CLOCK_SIGNAL_X_CLK                = 36,
    DDK_CLOCK_SIGNAL_UART                 = 37,
    DDK_CLOCK_SIGNAL_XTAL24M              = 38,
    DDK_CLOCK_SIGNAL_32K                  = 39,
    DDK_CLOCK_SIGNAL_FLEXCAN0             = 40,
    DDK_CLOCK_SIGNAL_FLEXCAN1             = 41,
    DDK_CLOCK_SIGNAL_LRADC2K              = 42,
    DDK_CLOCK_SIGNAL_UTMI0                = 43,
    DDK_CLOCK_SIGNAL_UTMI1                = 44,
    DDK_CLOCK_SIGNAL_PWM24M               = 45,
    DDK_CLOCK_SIGNAL_TIMROT32K            = 46,     
    DDK_CLOCK_SIGNAL_ENUM_END             = 47
} DDK_CLOCK_SIGNAL;                    



//-----------------------------------------------------------------------------
//
//  Type: DDK_CLOCK_GATE_INDEX
//
//  Clock GATE name for querying/setting clock configuration.
//
//-----------------------------------------------------------------------------
typedef enum
{  
    DDK_CLOCK_GATE_UART_CLK          = 0,   
    DDK_CLOCK_GATE_PWM24M_CLK        = 1,  
    DDK_CLOCK_GATE_TIMROT32K_CLK     = 2, 
    DDK_CLOCK_GATE_SSP0_CLK          = 3,   
    DDK_CLOCK_GATE_SSP1_CLK          = 4,
    DDK_CLOCK_GATE_SSP2_CLK          = 5,  
    DDK_CLOCK_GATE_SSP3_CLK          = 6,    
    DDK_CLOCK_GATE_GPMI_CLK          = 7,
    DDK_CLOCK_GATE_SPDIF_CLK         = 8,
    DDK_CLOCK_GATE_EMI_CLK           = 9,
    DDK_CLOCK_GATE_SAIF0_CLK         = 10,
    DDK_CLOCK_GATE_SAIF1_CLK         = 11,
    DDK_CLOCK_GATE_DIS_LCDIF_CLK     = 12,
    DDK_CLOCK_GATE_ETM_CLK           = 13,
    DDK_CLOCK_GATE_ENET_CLK          = 14,
    DDK_CLOCK_GATE_FLEXCAN0_CLK      = 15,
    DDK_CLOCK_GATE_FLEXCAN1_CLK      = 16,
    DDK_CLOCK_GATE_UTMI0_CLK480M_CLK = 17,
    DDK_CLOCK_GATE_UTMI1_CLK480M_CLK = 18,
    DDK_CLOCK_GATE_HSADC_CLK         = 19,
    DDK_CLOCK_GATE_ENUM_END          = 20
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
    DDK_CLOCK_GATE_MODE_ENABLEDD            = 1
} DDK_CLOCK_GATE_MODE;


//-----------------------------------------------------------------------------
//
//  Type: DDK_CLOCK_BAUD_SOURCE
//
//  Input source for baud clock generation.
//
//-----------------------------------------------------------------------------
typedef enum {
    DDK_CLOCK_BAUD_SOURCE_PLL0              = 0,
    DDK_CLOCK_BAUD_SOURCE_PLL1              = 1,
    DDK_CLOCK_BAUD_SOURCE_PLL2              = 2,    
    DDK_CLOCK_BAUD_SOURCE_REF_CPU           = 3,
    DDK_CLOCK_BAUD_SOURCE_REF_EMI           = 4,
    DDK_CLOCK_BAUD_SOURCE_REF_IO0           = 5,
    DDK_CLOCK_BAUD_SOURCE_REF_IO1           = 6,    
    DDK_CLOCK_BAUD_SOURCE_REF_PIX           = 7,
    DDK_CLOCK_BAUD_SOURCE_REF_HSADC         = 8,
    DDK_CLOCK_BAUD_SOURCE_REF_GPMI          = 9,
    DDK_CLOCK_BAUD_SOURCE_REF_PLL           = 10,
    DDK_CLOCK_BAUD_SOURCE_REF_ENET_PLL      = 11,
    DDK_CLOCK_BAUD_SOURCE_REF_XTAL          = 12,
    DDK_CLOCK_BAUD_SOURCE_ENUM_END          = 13
} DDK_CLOCK_BAUD_SOURCE;

//-----------------------------------------------------------------------------
//
//  Type: PLL0 and PLL1 LFR_SEL
//
//
//-----------------------------------------------------------------------------
typedef enum 
{
    LFR_DEFAULT     = 0x0,
    LFR_TIMES_2     = 0x1,
    LFR_TIMES_05    = 0x2,
    LFR_UNDEFINED   = 0x3
} DDK_CLOCK_PLLCTRL0_1_LFR_SEL;
//-----------------------------------------------------------------------------
//
//  Type: PLL0 and PLL1 CP_SEL
//
//
//-----------------------------------------------------------------------------
typedef enum 
{
    CP_DEFAULT      = 0x0,
    CP_TIMES_2      = 0x1,
    CP_TIMES_05     = 0x2,
    CP_UNDEFINED    = 0x3
} DDK_CLOCK_PLLCTRL0_1_CP_SEL;
//-----------------------------------------------------------------------------
//
//  Type: PLL0 and PLL1 DIV_SEL
//
//
//-----------------------------------------------------------------------------
typedef enum
{
    DIV_DEFAULT     = 0x0,
    DIV_LOWER       = 0x1,
    DIV_LOWEST      = 0x2,
    DIV_UNDEFINED   = 0x3
} DDK_CLOCK_PLLCTRL0_1_DIV_SEL;

//-----------------------------------------------------------------------------
//
//  Type: CLOCK SELECT BYPASS
//
//
//-----------------------------------------------------------------------------

typedef enum _hw_clkctrl_bypass_clk_t
{
    BYPASS_CPU        = 0x40000,
    BYPASS_DIS_LCDIF  = 0x4000,
    BYPASS_ETM        = 0x100,
    BYPASS_EMI        = 0x80,
    BYPASS_SSP3       = 0x40,
    BYPASS_SSP2       = 0x20,
    BYPASS_SSP1       = 0x10,
    BYPASS_SSP0       = 0x8,
    BYPASS_GPMI       = 0x4,
    BYPASS_SAIF1      = 0x2,
    BYPASS_SAIF0      = 0x1
    // Change all the clocks to use the same bypass.  For PMI to switch PLL off.
} DDK_CLOCK_BYPASS_CLK;
//-----------------------------------------------------------------------------
//
//  Type: HBUS SLOW DIV
//
//
//-----------------------------------------------------------------------------
typedef enum
{
    SLOW_DIV_BY1  = 0x0,
    SLOW_DIV_BY2  = 0x1,
    SLOW_DIV_BY4  = 0x2,
    SLOW_DIV_BY8  = 0x3,
    SLOW_DIV_BY16 = 0x4,
    SLOW_DIV_BY32 = 0x5
} DDK_CLOCK_HBUS_SLOW_DIV;
//-----------------------------------------------------------------------------
//
//  Type: HBUS ENABLE BIT
//
//-----------------------------------------------------------------------------
typedef enum
{
    APBHDMA_AS_ENABLE       = 0x04000000,
    APBXDMA_AS_ENABLE       = 0x02000000,
    TRAFFIC_JAM_AS_ENABLE   = 0x01000000,
    TRAFFIC_AS_ENABLE       = 0x00800000,
    CPU_DATA_AS_ENABLE      = 0x00400000,
    CPU_INSTR_AS_ENABLE     = 0x00200000
} DDK_CLOCK_HBUS_AS_ENABLE;

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
//-----------------------------------------------------------------------------
//
//  Type: CPU SET POINT FREQ
//
//-----------------------------------------------------------------------------
typedef enum
{
    // CPU setpoint frequencies
    DDK_DVFC_FREQ_CPU               = 0,
    DDK_DVFC_FREQ_AHB               = 1,
    DDK_DVFC_FREQ_EMI               = 2,
    DDK_DVFC_FREQ_ENUM_END          = 3
} DDK_DVFC_SETPOINT_FREQ;
//-----------------------------------------------------------------------------
//
//  Type: CPU DOMAIN
//
//
//-----------------------------------------------------------------------------
typedef enum
{
    DDK_DVFC_DOMAIN_CPU             = 0,
    DDK_DVFC_DOMAIN_ENUM_END        = 1
} DDK_DVFC_DOMAIN;
//-----------------------------------------------------------------------------
//
//  Type: DVFC SET POINT AND VOLTAGE
//
//
//-----------------------------------------------------------------------------
typedef struct
{
    UINT32 mV;
    UINT32 mV_BO;  
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
    // Shared clock management info
    UINT32 clockFreq[DDK_CLOCK_SIGNAL_ENUM_END];    
    UINT32 clockRefCount[DDK_CLOCK_GATE_ENUM_END];    
    DDK_CLOCK_BAUD_SOURCE root[DDK_CLOCK_GATE_ENUM_END];
    UINT32 rootRefCount[DDK_CLOCK_BAUD_SOURCE_ENUM_END];    

    // Shared setpoint info
    BOOL bDvfcActive;
    BOOL bSetpointPending;
    BOOL bEMIClockChange;
    DDK_DVFC_SETPOINT setpointCur[DDK_DVFC_DOMAIN_ENUM_END];
    DDK_DVFC_SETPOINT setpointMin[DDK_DVFC_DOMAIN_ENUM_END];
    DDK_DVFC_SETPOINT setpointMax[DDK_DVFC_DOMAIN_ENUM_END];
    DDK_DVFC_SETPOINT setpointLoad[DDK_DVFC_DOMAIN_ENUM_END];
    UINT32 setpointReqCount[DDK_DVFC_DOMAIN_ENUM_END][DDK_DVFC_SETPOINT_ENUM_END];
}  DDK_CLK_CONFIG, *PDDK_CLK_CONFIG;

//------------------------------------------------------------------------------
// DDK Functions for Clock module


//------------------------------------------------------------------------------
// Functions

BOOL DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX index, BOOL bClkGate);
BOOL DDKClockGetGatingMode(DDK_CLOCK_GATE_INDEX index);
BOOL DDKClockGetFreq(DDK_CLOCK_SIGNAL sig, UINT32 *freq);
BOOL DDKClockConfigBaud(DDK_CLOCK_SIGNAL sig, DDK_CLOCK_BAUD_SOURCE src, UINT32 u32Div);
BOOL DDKClockSetpointRequest(DDK_DVFC_SETPOINT setpoint, BOOL bBlock);
BOOL DDKClockSetpointRelease(DDK_DVFC_SETPOINT setpoint);
VOID DDKClockLock(VOID);
VOID DDKClockUnlock(VOID);
UINT32 DDKClockReadENetConfig(VOID);
VOID DDKClockWriteENetConfig(UINT32 RegData);

//------------------------------------------------------------------------------
#if __cplusplus
}
#endif

#endif //__MX28_DDK_H
