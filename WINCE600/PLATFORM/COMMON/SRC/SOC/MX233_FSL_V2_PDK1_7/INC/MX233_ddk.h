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
//  Header: MX233_ddk.h
//------------------------------------------------------------------------------
#ifndef __MX233_DDK_H
#define __MX233_DDK_H

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

#define DDK_IOMUX_GET_BANK(x)  (x>>5)
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
    DDK_GPIO_BANK1      = 0,
    DDK_GPIO_BANK2      = 1,
    DDK_GPIO_BANK3      = 2,
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
    DDK_IOMUX_GPMI_D00                  =(0x0),           // BANK 0 MUXSEL0(pins 0-15)
    DDK_IOMUX_SSP2_D0                   =(0x0),
    DDK_IOMUX_GPMI_LCD_D8               =(0x0),
    DDK_IOMUX_GPMI_D01                  =(0x1),
    DDK_IOMUX_SSP2_D1                   =(0x1),
    DDK_IOMUX_GPMI_LCD_D9               =(0x1),
    DDK_IOMUX_GPMI_D02                  =(0x2),
    DDK_IOMUX_SSP2_D2                   =(0x2),
    DDK_IOMUX_GPMI_LCD_D10              =(0x2),
    DDK_IOMUX_GPMI_D03                  =(0x3),
    DDK_IOMUX_SSP2_D3                   =(0x3),
    DDK_IOMUX_GPMI_LCD_D11              =(0x3),
    DDK_IOMUX_GPMI_D04                  =(0x4),
    DDK_IOMUX_SSP2_D4                   =(0x4),
    DDK_IOMUX_GPMI_LCD_D12              =(0x4),
    DDK_IOMUX_GPMI_D05                  =(0x5),
    DDK_IOMUX_SSP2_D5                   =(0x5),
    DDK_IOMUX_GPMI_LCD_D13              =(0x5),
    DDK_IOMUX_GPMI_D06                  =(0x6),
    DDK_IOMUX_SSP2_D6                   =(0x6),
    DDK_IOMUX_GPMI_LCD_D14              =(0x6),
    DDK_IOMUX_GPMI_D07                  =(0x7),
    DDK_IOMUX_SSP2_D7                   =(0x7),
    DDK_IOMUX_GPMI_LCD_D15              =(0x7),
    DDK_IOMUX_GPMI_D08                  =(0x8),
    DDK_IOMUX_SSP1_D4                   =(0x8),
    DDK_IOMUX_GPMI_LCD_D18              =(0x8),
    DDK_IOMUX_GPMI_D09                  =(0x9),
    DDK_IOMUX_SSP1_D5                   =(0x9),
    DDK_IOMUX_GPMI_LCD_D19              =(0x9),
    DDK_IOMUX_GPMI_D10                  =(0xA),
    DDK_IOMUX_SSP1_D6                   =(0xA),
    DDK_IOMUX_GPMI_LCD_D20              =(0xA),
    DDK_IOMUX_GPMI_D11                  =(0xB),
    DDK_IOMUX_SSP1_D7                   =(0xB),
    DDK_IOMUX_GPMI_LCD_D21              =(0xB),
    DDK_IOMUX_GPMI_D12                  =(0xC),
    DDK_IOMUX_GPMI_LCD_D22              =(0xC),
    DDK_IOMUX_GPMI_D13                  =(0xD),
    DDK_IOMUX_GPMI_LCD_D23              =(0xD),
    DDK_IOMUX_GPMI_D14                  =(0xE),
    DDK_IOMUX_AUART2_RX                 =(0xE),
    DDK_IOMUX_GPMI_D15                  =(0xF),
    DDK_IOMUX_AUART2_TX                 =(0xF),
    DDK_IOMUX_GPMI_CE3N                 =(0xF),

    DDK_IOMUX_GPMI_CLE                  =(0x10),           // BANK 0 MUXSEL1(pins 16-31)
    DDK_IOMUX_GPMI_LCD_D16              =(0x10),
    DDK_IOMUX_GPMI_ALE                  =(0x11),
    DDK_IOMUX_GPMI_LCD_D17              =(0x11),
    DDK_IOMUX_GPMI_CE2N                 =(0x12),
    DDK_IOMUX_GPMI_A2                   =(0x12),
    DDK_IOMUX_GPMI_RB0                  =(0x13),
    DDK_IOMUX_SSP2_DETECT               =(0x13),
    DDK_IOMUX_GPMI_RB1                  =(0x14),
    DDK_IOMUX_SSP2_CMD                  =(0x14),
    DDK_IOMUX_GPMI_RB2                  =(0x15),
    DDK_IOMUX_GPMI_RDY3                 =(0x16),
    DDK_IOMUX_GPMI_WPN                  =(0x17),
    DDK_IOMUX_GPMI_WRN                  =(0x18),
    DDK_IOMUX_SSP2_SCK                  =(0x18),
    DDK_IOMUX_GPMI_RDN                  =(0x19),
    DDK_IOMUX_AUART1_CTS                =(0x1A),
    DDK_IOMUX_SSP1_D4_DUP               =(0x1A),
    DDK_IOMUX_AUART1_RTS                =(0x1B),
    DDK_IOMUX_IR_CLK                    =(0x1B),
    DDK_IOMUX_SSP1_D5_DUP               =(0x1B),
    DDK_IOMUX_AUART1_RX                 =(0x1C),
    DDK_IOMUX_IR_DATA_IN                =(0x1C),
    DDK_IOMUX_SSP1_D6_DUP               =(0x1C),
    DDK_IOMUX_AUART1_TX                 =(0x1D),
    DDK_IOMUX_IR_DATA_OUT               =(0x1D),
    DDK_IOMUX_SSP1_D7_DUP               =(0x1D),
    DDK_IOMUX_I2C_CLK                   =(0x1E),
    DDK_IOMUX_I2C_CLK_GPMI_RDY2         =(0x1E),
    DDK_IOMUX_I2C_SCL_AUART1_TX         =(0x1E),
    DDK_IOMUX_I2C_SDA                   =(0x1F),
    DDK_IOMUX_I2C_SDA_GPMI_CE2N         =(0x1F),
    DDK_IOMUX_I2C_SDA_AUART1_RX         =(0x1F),

    DDK_IOMUX_LCD_D00                   =(0x20),            // BANK 1 MUXSEL2(pins 0-15)
    DDK_IOMUX_ETM_DA8                   =(0x20),
    DDK_IOMUX_LCD_D01                   =(0x21),
    DDK_IOMUX_ETM_DA9                   =(0x21),
    DDK_IOMUX_LCD_D02                   =(0x22),
    DDK_IOMUX_ETM_DA10                  =(0x22),
    DDK_IOMUX_LCD_D03                   =(0x23),
    DDK_IOMUX_ETM_DA11                  =(0x23),
    DDK_IOMUX_LCD_D04                   =(0x24),
    DDK_IOMUX_ETM_DA12                  =(0x24),
    DDK_IOMUX_LCD_D05                   =(0x25),
    DDK_IOMUX_ETM_DA13                  =(0x25),
    DDK_IOMUX_LCD_D06                   =(0x26),
    DDK_IOMUX_ETM_DA14                  =(0x26),
    DDK_IOMUX_LCD_D07                   =(0x27),
    DDK_IOMUX_ETM_DA15                  =(0x27),
    DDK_IOMUX_LCD_D08                   =(0x28),
    DDK_IOMUX_ETM_DA0                   =(0x28),
    DDK_IOMUX_SAIF2_SDATA0              =(0x28),
    DDK_IOMUX_LCD_D09                   =(0x29),
    DDK_IOMUX_ETM_DA1                   =(0x29),
    DDK_IOMUX_SAIF1_SDATA0              =(0x29),
    DDK_IOMUX_LCD_D10                   =(0x2A),
    DDK_IOMUX_ETM_DA2                   =(0x2A),
    DDK_IOMUX_SAIF_BITCLK               =(0x2A),
    DDK_IOMUX_LCD_D11                   =(0x2B),
    DDK_IOMUX_ETM_DA3                   =(0x2B),
    DDK_IOMUX_SAIF_LRCLK                =(0x2B),
    DDK_IOMUX_LCD_D12                   =(0x2C),
    DDK_IOMUX_ETM_DA4                   =(0x2C),
    DDK_IOMUX_SAIF2_SDATA1              =(0x2C),
    DDK_IOMUX_LCD_D13                   =(0x2D),
    DDK_IOMUX_ETM_DA6                   =(0x2D),
    DDK_IOMUX_SAIF2_SDATA2              =(0x2D),
    DDK_IOMUX_LCD_D14                   =(0x2E),
    DDK_IOMUX_ETM_DA5                   =(0x2E),
    DDK_IOMUX_SAIF1_SDATA2              =(0x2E),
    DDK_IOMUX_LCD_D15                   =(0x2F),
    DDK_IOMUX_ETM_DA7                   =(0x2F),
    DDK_IOMUX_SAIF1_SDATA1              =(0x2F),

    DDK_IOMUX_LCD_D16                   =(0x30),          // BANK 1 MUXSEL3(pins 16-31)
    DDK_IOMUX_SAIF1_ALT_BITCLK          =(0x30),
    DDK_IOMUX_LCD_D17                   =(0x31),
    DDK_IOMUX_LCD_RESET                 =(0x32),
    DDK_IOMUX_ETM_TCTL                  =(0x32),
    DDK_IOMUX_GPMI_CE3N_DUP             =(0x32),
    DDK_IOMUX_LCD_RS                    =(0x33),
    DDK_IOMUX_ETM_TCTLK                 =(0x33),
    DDK_IOMUX_LCD_WR                    =(0x34),
    DDK_IOMUX_LCD_CS                    =(0x35),
    DDK_IOMUX_LCD_DOTCLK                =(0x36),
    DDK_IOMUX_GPMI_CE2N_DUP1            =(0x36),
    DDK_IOMUX_LCD_ENABLE                =(0x37),
    DDK_IOMUX_I2C_CLK_DUP1              =(0x37),
    DDK_IOMUX_LCD_HSYNC                 =(0x38),
    DDK_IOMUX_I2C_SD_DUP1               =(0x38),
    DDK_IOMUX_LCD_VSYNC                 =(0x39),
    DDK_IOMUX_LCD_BUSY                  =(0x39),
    DDK_IOMUX_PWM0                      =(0x3A),
    DDK_IOMUX_TIMROT1                   =(0x3A),
    DDK_IOMUX_DUART1_RX                 =(0x3A),
    DDK_IOMUX_PWM1                      =(0x3B),
    DDK_IOMUX_TIMROT2                   =(0x3B),
    DDK_IOMUX_DUART1_TX                 =(0x3B),
    DDK_IOMUX_PWM2                      =(0x3C),
    DDK_IOMUX_GPMI_RB3                  =(0x3C),
    DDK_IOMUX_PWM3                      =(0x3D),
    DDK_IOMUX_ETM_TCTL_DUP1             =(0x3D),
    DDK_IOMUX_AUART1_CTS_DUP1           =(0x3D),
    DDK_IOMUX_PWM4                      =(0x3E),
    DDK_IOMUX_ETM_TCLK                  =(0x3E),
    DDK_IOMUX_AUART1_RTS_DUP1           =(0x3E),
    DDK_IOMUX_GPIO_BANK1_PIN31          =(0x3F),            // Reserved

    DDK_IOMUX_SSP1_CMD                  =(0x40),            // BANK 2 MUXSEL4(pins 0-15)
    DDK_IOMUX_JTAG_TDO                  =(0x40),
    DDK_IOMUX_SSP1_DET                  =(0x41),
    DDK_IOMUX_GPMI_CE3N_DUP2            =(0x41),
    DDK_IOMUX_USB_OTG_ID                =(0x41),
    DDK_IOMUX_SSP1_D0                   =(0x42),
    DDK_IOMUX_JTAG_TDI                  =(0x42),
    DDK_IOMUX_SSP1_D1                   =(0x43),
    DDK_IOMUX_I2C_CLK_DUP2              =(0x43),
    DDK_IOMUX_JTAG_TCK                  =(0x43),
    DDK_IOMUX_SSP1_D2                   =(0x44),
    DDK_IOMUX_I2C_SD_DUP2               =(0x44),
    DDK_IOMUX_JTAG_RTCK                 =(0x44),
    DDK_IOMUX_SSP1_D3                   =(0x45),
    DDK_IOMUX_JTAG_TMS                  =(0x45),
    DDK_IOMUX_SSP1_SCK                  =(0x46),
    DDK_IOMUX_JTAG_TRST_N               =(0x46),
    DDK_IOMUX_TIMROT1_DUP1              =(0x47),
    DDK_IOMUX_AUART1_RTS_DUP2           =(0x47),
    DDK_IOMUX_SPDIF                     =(0x47),
    DDK_IOMUX_TIMROT2_DUP               =(0x48),
    DDK_IOMUX_AUART1_CTS_DUP2           =(0x48),
    DDK_IOMUX_GPMI_CE3N_DUP22           =(0x48),
    DDK_IOMUX_EMI_A00                   =(0x49),
    DDK_IOMUX_EMI_A01                   =(0x4A),
    DDK_IOMUX_EMI_A02                   =(0x4B),
    DDK_IOMUX_EMI_A03                   =(0x4C),
    DDK_IOMUX_EMI_A04                   =(0x4D),
    DDK_IOMUX_EMI_A05                   =(0x4E),
    DDK_IOMUX_EMI_A06                   =(0x4F),

    DDK_IOMUX_EMI_A07                   =(0x50),       // BANK 2 MUXSEL5(pins 16-31)
    DDK_IOMUX_EMI_A08                   =(0x51),
    DDK_IOMUX_EMI_A09                   =(0x52),
    DDK_IOMUX_EMI_A10                   =(0x53),
    DDK_IOMUX_EMI_A11                   =(0x54),
    DDK_IOMUX_EMI_A12                   =(0x55),
    DDK_IOMUX_EMI_BA0                   =(0x56),
    DDK_IOMUX_EMI_BA1                   =(0x57),
    DDK_IOMUX_EMI_CASN                  =(0x58),
    DDK_IOMUX_EMI_CE0N                  =(0x59),
    DDK_IOMUX_EMI_CE1N                  =(0x5A),
    DDK_IOMUX_GEMI_CE1N                 =(0x5B),
    DDK_IOMUX_GEMI_CE0N                 =(0x5C),
    DDK_IOMUX_EMI_CKE                   =(0x5D),
    DDK_IOMUX_EMI_RASN                  =(0x5E),
    DDK_IOMUX_EMI_WEN                   =(0x5F),

    DDK_IOMUX_EMI_D00                   =(0x60),             // BANK 3 MUXSEL6(pins 0-15)
    DDK_IOMUX_EMI_D01                   =(0x61),
    DDK_IOMUX_EMI_D02                   =(0x62),
    DDK_IOMUX_EMI_D03                   =(0x63),
    DDK_IOMUX_EMI_D04                   =(0x64),
    DDK_IOMUX_EMI_D05                   =(0x65),
    DDK_IOMUX_EMI_D06                   =(0x66),
    DDK_IOMUX_EMI_D07                   =(0x67),
    DDK_IOMUX_EMI_D08                   =(0x68),
    DDK_IOMUX_EMI_D09                   =(0x69),
    DDK_IOMUX_EMI_D10                   =(0x6A),
    DDK_IOMUX_EMI_D11                   =(0x6B),
    DDK_IOMUX_EMI_D12                   =(0x6C),
    DDK_IOMUX_EMI_D13                   =(0x6D),
    DDK_IOMUX_EMI_D14                   =(0x6E),
    DDK_IOMUX_EMI_D15                   =(0x6F),

    DDK_IOMUX_EMI_DQM0                  =(0x70),           // BANK 3 MUXSEL7(pins 16-31)
    DDK_IOMUX_EMI_DQM1                  =(0x71),
    DDK_IOMUX_EMI_DQS0                  =(0x72),
    DDK_IOMUX_EMI_DQS1                  =(0x73),
    DDK_IOMUX_EMI_CLK                   =(0x74),
    DDK_IOMUX_EMI_CLKN                  =(0x75),
    DDK_IOMUX_GPIO_BANK3_PIN22          =(0x76),   // Reserved
    DDK_IOMUX_GPIO_BANK3_PIN23          =(0x77),   // Reserved
    DDK_IOMUX_GPIO_BANK3_PIN24          =(0x78),   // Reserved
    DDK_IOMUX_GPIO_BANK3_PIN25          =(0x79),   // Reserved
    DDK_IOMUX_GPIO_BANK3_PIN26          =(0x7A),   // Reserved
    DDK_IOMUX_GPIO_BANK3_PIN27          =(0x7B),   // Reserved
    DDK_IOMUX_GPIO_BANK3_PIN28          =(0x7C),   // Reserved
    DDK_IOMUX_GPIO_BANK3_PIN29          =(0x7D),   // Reserved
    DDK_IOMUX_GPIO_BANK3_PIN30          =(0x7E),   // Reserved
    DDK_IOMUX_GPIO_BANK3_PIN31          =(0x7F),   // Reserved

    DDK_IOMUX_GPIO_MAX_ID               =(128),
    DDK_IOMUX_GPIO_END_TABLE            =(255)
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
// APBH Channel number

#define DDK_MAX_APBH_CHANNEL             8

#define DDK_APBH_CHANNEL_LCDIF           0x00
#define DDK_APBH_CHANNEL_SSP1            0x01
#define DDK_APBH_CHANNEL_SSP2            0x02
#define DDK_APBH_CHANNEL_RESERVED        0x03
#define DDK_APBH_CHANNEL_NAND0           0x04
#define DDK_APBH_CHANNEL_NAND1           0x05
#define DDK_APBH_CHANNEL_NAND2           0x06
#define DDK_APBH_CHANNEL_NAND3           0x07

// APBX
#define DDK_MAX_APBX_CHANNEL     10

#define APBX_CHANNEL_AUDIO_ADC  0x00
#define APBX_CHANNEL_AUDIO_DAC  0x01
#define APBX_CHANNEL_SPDIFTX    0x02
#define APBX_CHANNEL_I2C                0x03
#define APBX_CHANNEL_SAIF1              0x04
#define APBX_CHANNEL_DRI                0x05
#define APBX_CHANNEL_UART1RX    0x06
#define APBX_CHANNEL_UART1TX    0x07
#define APBX_CHANNEL_IRDARX             0x06
#define APBX_CHANNEL_IRDATX             0x07
#define APBX_CHANNEL_UART2RX    0x08
#define APBX_CHANNEL_UART2TX    0x09
#define APBX_CHANNEL_SAIF2              0x0A

#define DDK_DMA_COMMAND_NO_DMA_XFER  0x0
#define DDK_DMA_COMMAND_DMA_WRITE    0x1
#define DDK_DMA_COMMAND_DMA_READ     0x2

#define DDK_DMA_ERROR_NONE           0
#define DDK_DMA_ERROR_EARLYTERM      1
#define DDK_DMA_ERROR_BUS            2
#define DDK_DMA_ERROR_INCOMPLETE     3
#define DDK_DMA_ERROR_TIMEOUT        4
#define DDK_DMA_ERROR_UNKNOWN        0x7fffffff
//------------------------------------------------------------------------------

// DDK APBH Definitions
BOOL DDKApbhStartDma(UINT8 Channel,PVOID memAddrPA, UINT8 semaphore);
BOOL DDKApbhStopDma(UINT8 Channel);
BOOL DDKApbhDmaInitChan(UINT8 Channel,BOOL bEnableIrq);
BOOL DDKApbhDmaChanCLKGATE(UINT8 Channel,BOOL bClockGate);
BOOL DDKApbhDmaClearCommandCmpltIrq(UINT8 Channel);
BOOL DDKApbhDmaEnableCommandCmpltIrq(UINT8 Channel,BOOL bEnable);
BOOL DDKApbhDmaResetChan(UINT8 Channel,BOOL bReset);
BOOL DDKApbhDmaFreezeChan(UINT8 Channel,BOOL bFreeze);
UINT32 DDKApbhDmaGetPhore(UINT32 Channel);

// DDK APBX Definitions
BOOL DDKApbxStartDma(UINT8 Channel,PVOID memAddrPA, UINT8 semaphore);
BOOL DDKApbxStopDma(UINT8 Channel);
BOOL DDKApbxDmaInitChan(UINT8 Channel,BOOL bEnableIrq);
BOOL DDKApbxDmaGetActiveIrq(UINT8 Channel);
BOOL DDKApbxDmaGetErrorIrq(UINT8 Channel);
BOOL DDKApbxDmaClearCommandCmpltIrq(UINT8 Channel);
BOOL DDKApbxDmaClearErrorIrq(UINT8 Channel);
BOOL DDKApbxDmaEnableCommandCmpltIrq(UINT8 Channel,BOOL bEnable);
UINT32 DDKApbxDmaGetErrorStatus(UINT8 Channel);
BOOL DDKApbxDmaResetChan(UINT8 Channel, BOOL bReset);
BOOL DDKApbxDmaFreezeChan(UINT8 Channel, BOOL bFreeze);
UINT32 DDKApbxGetNextCMDAR(UINT8 Channel);
BOOL DDKApbxDmaClearInterrupts(UINT8 Channel);
BOOL DDKApbhDmaClearErrorIrq(UINT8 Channel);

// DDK GPIO
BOOL DDKGpioConfig(DDK_IOMUX_PIN gpio_pin,DDK_GPIO_CFG gpio_cfg,DDK_IOMUX_PAD_DRIVE drive,DDK_IOMUX_PAD_VOLTAGE voltage,BOOL bPull_Enable);
BOOL DDKGpioWriteDataPin(DDK_IOMUX_PIN pin,UINT32 Data);
BOOL DDKGpioReadData(DDK_GPIO_BANK bank, UINT32 *pData);
BOOL DDKGpioReadDataPin(DDK_IOMUX_PIN pin, UINT32 *pData);
BOOL DDKGpioReadIntr(DDK_IOMUX_PIN pin,UINT32 *pData);
BOOL DDKGpioEnableDataPin(DDK_IOMUX_PIN pin,UINT32 Data);
VOID DDKGpioIRQSTATCLR(DDK_IOMUX_PIN gpio_pin);
BOOL DDKGpioIRQPolarityConfig(DDK_IOMUX_PIN gpio_pin,BOOL bEnable);

//
BOOL DDKIomuxSetPinMux(DDK_IOMUX_PIN pin, DDK_IOMUX_PIN_MUXMODE muxmode);
BOOL DDKIomuxGetPinMux(DDK_IOMUX_PIN pin, DDK_IOMUX_PIN_MUXMODE *pMuxmode);
BOOL DDKIomuxSetPadConfig(DDK_IOMUX_PIN pin,DDK_IOMUX_PAD_DRIVE drive,DDK_IOMUX_PAD_PULL pull,DDK_IOMUX_PAD_VOLTAGE voltage);
BOOL DDKIomuxGetPadConfig(DDK_IOMUX_PIN pin,DDK_IOMUX_PAD_DRIVE *pDrive,DDK_IOMUX_PAD_PULL *pPull,DDK_IOMUX_PAD_VOLTAGE *pVoltage);
BOOL DDKIomuxEnablePullup(DDK_IOMUX_PIN pin, BOOL bEnable);

//------------------------------------------------------------------------------


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
//! \brief Maximum IR clock divider allowed.
#define MAX_IR_DIV 768
//! \brief Minimum IR clock divider allowed.
#define MIN_IR_DIV 5
//! \brief Maximum IR oversample clock divider allowed.
#define MAX_IROV_DIV 260
//! \brief Minimum IR oversample clock divider allowed. 
#define MIN_IROV_DIV 4
//! \brief Maximum display clock frequency allowed.
#define MAX_PIXCLK 480000
//! \brief Minimum display clock frequency allowed.
#define MIN_PIXCLK 1000
//! \brief Minimum integer divider for default cases.  
#define MIN_INT_DIV 1

#define REF_VID_432MHZ   432000000
#define TV_108MHZ        108000000
#define TV_54MHZ         54000000
#define TV_27MHZ         27000000

#define FILT_24MHZ       24000000
#define PWM_24MHZ        24000000
#define DRI_24MHZ        24000000
#define DIGCTL_CLK1MHZ     1000000
#define TIMROT_CLK32KHZ    32000
#define ADC_CLK2KHZ        2000
#define SPDIF_120MHZ     120000000


//Enumerates the reference signals from the PLL
typedef enum _CSP_clocks_pll_ref_clks_t
{
    //ref_cpu signal for CPU clock
    PLL_REF_CPU,
    //ref_emi signal for EMI clock
    PLL_REF_EMI,
    //ref_io signal for SSP, GPMI, and IR/IROV clocks
    PLL_REF_IO,
    //ref_pix signal for display clock
    PLL_REF_PIX
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
    DDK_CLOCK_SIGNAL_PLL              = 0,
    DDK_CLOCK_SIGNAL_REF_CPU          = 1,
    DDK_CLOCK_SIGNAL_REF_EMI          = 2,
    DDK_CLOCK_SIGNAL_REF_IO           = 3,
    DDK_CLOCK_SIGNAL_REF_PIX          = 4,
    DDK_CLOCK_SIGNAL_REF_VID          = 5,
    DDK_CLOCK_SIGNAL_REF_PLL          = 6,
    DDK_CLOCK_SIGNAL_REF_XTAL         = 7,                                      
    DDK_CLOCK_SIGNAL_P_CLK            = 8,  
    DDK_CLOCK_SIGNAL_H_CLK            = 9,
    DDK_CLOCK_SIGNAL_X_CLK            = 10,                              
    DDK_CLOCK_SIGNAL_OCROM_CLK        = 11,
    DDK_CLOCK_SIGNAL_EMI_CLK          = 12,

    DDK_CLOCK_SIGNAL_ETM_CLK          = 13,    
    DDK_CLOCK_SIGNAL_SSP_CLK          = 14,   
    DDK_CLOCK_SIGNAL_GPMI_CLK         = 15,
    DDK_CLOCK_SIGNAL_IROV_CLK         = 16,
    DDK_CLOCK_SIGNAL_IR_CLK           = 17,
    DDK_CLOCK_SIGNAL_PIX_CLK          = 18,
                                        
    DDK_CLOCK_SIGNAL_SAIF_CLK         = 19,
    DDK_CLOCK_SIGNAL_PCMSPDIF_CLK     = 20,  
    DDK_CLOCK_SIGNAL_UTMI_CLK480M     = 21,
    DDK_CLOCK_SIGNAL_VDAC_CLK         = 22,
                                        
    DDK_CLOCK_SIGNAL_TV108M_CLK       = 23,
    DDK_CLOCK_SIGNAL_TV54M_CLK        = 24,
    DDK_CLOCK_SIGNAL_TV27M_CLK        = 25,
    DDK_CLOCK_SIGNAL_TVENC_FIFO       = 26,
                                        
    DDK_CLOCK_SIGNAL_UART_CLK         = 27,    
    DDK_CLOCK_SIGNAL_FILT24M_CLK      = 28, 
    DDK_CLOCK_SIGNAL_PWM24M_CLK       = 29,
    DDK_CLOCK_SIGNAL_DRI24M_CLK       = 30, 
    DDK_CLOCK_SIGNAL_DIGCTL_CLK1M_CLK = 31,
    DDK_CLOCK_SIGNAL_TIMROT_32K_CLK   = 32,
                                        
    DDK_CLOCK_SIGNAL_ADC_CLK          = 33,
    DDK_CLOCK_SIGNAL_ANA24M_CLK       = 34,  
    DDK_CLOCK_SIGNAL_ENUM_END         = 35
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
    DDK_CLOCK_GATE_ETM_CLK          = 0,
    DDK_CLOCK_GATE_SSP_CLK          = 1,   
    DDK_CLOCK_GATE_GPMI_CLK         = 2,
    DDK_CLOCK_GATE_IROV_CLK         = 3,
    DDK_CLOCK_GATE_IR_CLK           = 4,
    DDK_CLOCK_GATE_PIX_CLK          = 5,
    DDK_CLOCK_GATE_SAIF_CLK         = 6,
    DDK_CLOCK_GATE_PCMSPDIF_CLK     = 7,  
    DDK_CLOCK_GATE_UTMI_CLK480M     = 8,
    DDK_CLOCK_GATE_TV108M_CLK       = 9,
    DDK_CLOCK_GATE_TV54M_CLK        = 10,
    DDK_CLOCK_GATE_TV27M_CLK        = 11,
    DDK_CLOCK_GATE_TVENC_FIFO       = 12,
    DDK_CLOCK_GATE_UART_CLK         = 13,    
    DDK_CLOCK_GATE_FILT24M_CLK      = 14, 
    DDK_CLOCK_GATE_PWM24M_CLK       = 15,
    DDK_CLOCK_GATE_DRI24M_CLK       = 16, 
    DDK_CLOCK_GATE_DIGCTL_CLK1M_CLK = 17,
    DDK_CLOCK_GATE_TIMROT_32K_CLK   = 18,
    DDK_CLOCK_GATE_ENUM_END         = 19
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
    DDK_CLOCK_BAUD_SOURCE_PLL              = 0,
    DDK_CLOCK_BAUD_SOURCE_REF_CPU          = 1,
    DDK_CLOCK_BAUD_SOURCE_REF_EMI          = 2,
    DDK_CLOCK_BAUD_SOURCE_REF_IO           = 3,
    DDK_CLOCK_BAUD_SOURCE_REF_PIX          = 4,
    DDK_CLOCK_BAUD_SOURCE_REF_VID          = 5,
    DDK_CLOCK_BAUD_SOURCE_REF_PLL          = 6,
    DDK_CLOCK_BAUD_SOURCE_REF_XTAL         = 7,
    DDK_CLOCK_BAUD_SOURCE_ENUM_END         = 8
} DDK_CLOCK_BAUD_SOURCE;


typedef enum 
{
    LFR_DEFAULT     = 0x0,
    LFR_TIMES_2     = 0x1,
    LFR_TIMES_05    = 0x2,
    LFR_UNDEFINED   = 0x3
} DDK_CLOCK_PLLCTRL0_LFR_SEL;

typedef enum 
{
    CP_DEFAULT      = 0x0,
    CP_TIMES_2      = 0x1,
    CP_TIMES_05     = 0x2,
    CP_UNDEFINED    = 0x3
} DDK_CLOCK_PLLCTRL0_CP_SEL;

typedef enum
{
    DIV_DEFAULT     = 0x0,
    DIV_LOWER       = 0x1,
    DIV_LOWEST      = 0x2,
    DIV_UNDEFINED   = 0x3
} DDK_CLOCK_PLLCTRL0_DIV_SEL;

typedef enum _hw_clkctrl_bypass_clk_t
{
    BYPASS_CPU  = 0x80,
    BYPASS_EMI  = 0x40,
    BYPASS_SSP  = 0x20,
    BYPASS_GPMI = 0x10,
    BYPASS_IR   = 0x08,
    BYPASS_PIX  = 0x02,
    BYPASS_SAIF = 0x01

    // Change all the clocks to use the same bypass.  For PMI to
    // switch PLL off.
    //BYPASS_ALL =
} DDK_CLOCK_BYPASS_CLK;

typedef enum
{
    SLOW_DIV_BY1  = 0x0,
    SLOW_DIV_BY2  = 0x1,
    SLOW_DIV_BY4  = 0x2,
    SLOW_DIV_BY8  = 0x3,
    SLOW_DIV_BY16 = 0x4,
    SLOW_DIV_BY32 = 0x5
} DDK_CLOCK_HBUS_SLOW_DIV;

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

typedef enum
{
    // CPU setpoint frequencies
    DDK_DVFC_FREQ_CPU               = 0,
    DDK_DVFC_FREQ_AHB               = 1,
    DDK_DVFC_FREQ_EMI               = 2,
    DDK_DVFC_FREQ_ENUM_END          = 3
} DDK_DVFC_SETPOINT_FREQ;

typedef enum
{
    DDK_DVFC_DOMAIN_CPU             = 0,
    DDK_DVFC_DOMAIN_ENUM_END        = 1
} DDK_DVFC_DOMAIN;

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

//------------------------------------------------------------------------------
#if __cplusplus
}
#endif

#endif //__MX233_DDK_H
