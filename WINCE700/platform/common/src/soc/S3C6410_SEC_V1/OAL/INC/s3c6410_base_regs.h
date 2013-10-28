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
//  Header: cpu_base_reg.h
//
//  This header file defines the Physical Addresses (PA) of
//  the base registers for the System on Chip (SoC) components.
//
#ifndef __S3C6410_BASE_REG_H
#define __S3C6410_BASE_REG_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//  INFORMATION
//
//  The physical addresses for SoC registers are fixed, hence they are defined
//  in the CPU's common directory. The virtual addresses of the SoC registers
//  are defined by the OEM and are configured in the platform's configuration
//  directory by the file: .../PLATFORM/<NAME>/SRC/CONFIG/CPU_BASE_REG_CFG.H.
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//  NAMING CONVENTIONS
//
//  CPU_BASE_REG_ is the standard prefix for CPU base registers.
//
//  Memory ranges are accessed using physical, uncached, or cached addresses,
//  depending on the system state. The following abbreviations are used for
//  each addressing type:
//
//      PA - physical address
//      UA - uncached virtual address
//      CA - cached virtual address
//
//  The naming convention for CPU base registers is:
//
//      CPU_BASE_REG_<ADDRTYPE>_<SUBSYSTEM>
//
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_SROMCON
//
// Locates the SROM Controller register block.
//
#define S3C6410_BASE_REG_PA_SROMCON        0x70000000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_ONENANDC0
//
// Locates the OneNAND Controller 0 register block.
//
#define S3C6410_BASE_REG_PA_ONENANDC0       0x70100000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_ONENANDC1
//
// Locates the OneNAND Controller 1 register block.
//
#define S3C6410_BASE_REG_PA_ONENANDC1       0x70180000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_NFCON
//
// Locates the NAND Flash Controller register block.
//
#define S3C6410_BASE_REG_PA_NFCON        0x70200000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_CFCON
//
// Locates the Compact Flash Controller register block.
//
#define S3C6410_BASE_REG_PA_CFCON        0x70300000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_TZIC0
//
// Locates the Trust Zone Interrupt Controller 0 register block.
//
#define S3C6410_BASE_REG_PA_TZIC0            0x71000000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_TZIC1
//
// Locates the Trust Zone Interrupt Controller 1 register block.
//
#define S3C6410_BASE_REG_PA_TZIC1            0x71100000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_VIC0
//
// Locates the Vectored Interrupt Controller 0 register block.
//
#define S3C6410_BASE_REG_PA_VIC0            0x71200000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_VIC1
//
// Locates the Vectored Interrupt Controller 1 register block.
//
#define S3C6410_BASE_REG_PA_VIC1            0x71300000

// ETB to be defined...
////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_FIMG_3DSE
//
// Locates the FIMG 3DSE register block.
//
#define S3C6410_BASE_REG_PA_FIMG_3DSE            0x72000000
////////////////////////////////////////////////////////////
// Define:  S3C6410_BASE_REG_PA_HOSTIF
//
// Locates the Indirect HOST Interface register block.
//
#define S3C6410_BASE_REG_PA_HOSTIF        0x74000000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_MSMIFBM
//
// Locates the Modem Interface SRAM Buffer Memory block.
//
#define S3C6410_BASE_REG_PA_MSMIF_BM        0x74100000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_MSMIF
//
// Locates the Modem Interface register block.
//
#define S3C6410_BASE_REG_PA_MSMIF_SFR    0x74108000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_USBHOST
//
// Locates the USB Host Controller register block.
//
#define S3C6410_BASE_REG_PA_USBHOST        0x74300000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_MDPIF
//
// Locates the MDP Interface register block.
//
#define S3C6410_BASE_REG_PA_MDPIF        0x74400000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_DMA0
//
// Locates the DMA Controller 0 register block.
//
#define S3C6410_BASE_REG_PA_DMA0            0x75000000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_DMA1
//
// Locates the DMA Controller 1 register block.
//
#define S3C6410_BASE_REG_PA_DMA1            0x75100000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_2DGRAPHICS
//
// Locates the 2D Graphics register block.
//
#define S3C6410_BASE_REG_PA_2DGRAPHICS    0x76100000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_TVENC
//
// Locates the TV Encoder register block.
//
#define S3C6410_BASE_REG_PA_TVENC        0x76200000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_TVSC
//
// Locates the TV Scaler register block.
//
#define S3C6410_BASE_REG_PA_TVSC            0x76300000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_POST
//
// Locates the Post Processor register block.
//
#define S3C6410_BASE_REG_PA_POST            0x77000000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_DISPLAY
//
// Locates the Display Controller register block.
//
#define S3C6410_BASE_REG_PA_DISPLAY        0x77100000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_ROTATOR
//
// Locates the Rotator register block.
//
#define S3C6410_BASE_REG_PA_ROTATOR        0x77200000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_CAMIF
//
// Locates the Camera Interface register block.
//
#define S3C6410_BASE_REG_PA_CAMIF        0x78000000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_JPEG
//
// Locates the JPEG Codec register block.
//
#define S3C6410_BASE_REG_PA_JPEG            0x78800000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_USBOTG_LINK
//
// Locates the USB OTG Link Core register block.
//
#define S3C6410_BASE_REG_PA_USBOTG_LINK    0x7C000000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_USBOTG_PHY
//
// Locates the USB OTG Phy Control register block.
//
#define S3C6410_BASE_REG_PA_USBOTG_PHY    0x7C100000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_HSMMC0
//
// Locates the High Speed MMC Controller 0 register block.
//
#define S3C6410_BASE_REG_PA_HSMMC0        0x7C200000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_HSMMC1
//
// Locates the High Speed MMC Controller 1 register block.
//
#define S3C6410_BASE_REG_PA_HSMMC1        0x7C300000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_HSMMC2
//
// Locates the High Speed MMC Controller 2 register block.
//
#define S3C6410_BASE_REG_PA_HSMMC2        0x7C400000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_SECURITY
//
// Locates the Security Subsystem Config register block.
//
#define S3C6410_BASE_REG_PA_SECURITY        0x7D000000

// AEX, DES, HASH, FIFO, SDMA... to be defined

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_DMA0
//
// Locates the DMA Controller 0 register block.
//
#define S3C6410_BASE_REG_PA_SDMA0           0x7DB00000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_DMA1
//
// Locates the DMA Controller 1 register block.
//
#define S3C6410_BASE_REG_PA_SDMA1           0x7DC00000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_DMC0
//
// Locates the DRAM Controller 0 register block.
//
#define S3C6410_BASE_REG_PA_DMC0            0x7E000000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_DMC1
//
// Locates the DRAM Controller 1 register block.
//
#define S3C6410_BASE_REG_PA_DMC1            0x7E001000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_MFC
//
// Locates the Multi Format Video Codec register block.
//
#define S3C6410_BASE_REG_PA_MFC            0x7E002000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_WATCHDOG
//
// Locates the Watch-Dog Timer register block.
//
#define S3C6410_BASE_REG_PA_WATCHDOG    0x7E004000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_RTC
//
// Locates the Real Time Clock register block.
//
#define S3C6410_BASE_REG_PA_RTC            0x7E005000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_MIPITX
//
// Locates the MIPI HSI Interface TX register block.
//
#define S3C6410_BASE_REG_PA_MIPITX        0x7E006000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_MIPIRX
//
// Locates the MIPI HSI Interface RX register block.
//
#define S3C6410_BASE_REG_PA_MIPIRX        0x7E007000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_KEYPAD
//
// Locates the Keypad Interface register block.
//
#define S3C6410_BASE_REG_PA_KEYPAD        0x7E00A000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_ADC
//
// Locates the ADC / Touch Screen Interface register block.
//
#define S3C6410_BASE_REG_PA_ADC            0x7E00B000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_ETM
//
// Locates the ETM register block.
//
#define S3C6410_BASE_REG_PA_ETM            0x7E00C000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_KEY
//
// Locates the ADC / Touch Screen Interface register block.
//
#define S3C6410_BASE_REG_PA_KEY            0x7E00D000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_CHIPID
//
// Locates the Chip ID register block.
//
#define S3C6410_BASE_REG_PA_CHIPID        0x7E00E000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_SYSCON
//
// Locates the System Controller register block.
//
#define S3C6410_BASE_REG_PA_SYSCON        0x7E00F000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_TZPC
//
// Locates the TZPC register block.
//
#define S3C6410_BASE_REG_PA_TZPC            0x7F000000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_AC97
//
// Locates the AC97 Controller register block.
//
#define S3C6410_BASE_REG_PA_AC97            0x7F001000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_IIS0
//
// Locates the IIS-Bus Interface Ch0 register block.
//
#define S3C6410_BASE_REG_PA_IIS0            0x7F002000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_IIS1
//
// Locates the IIS-Bus Interface Ch1 register block.
//
#define S3C6410_BASE_REG_PA_IIS1            0x7F003000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_IICBUS
//
// Locates the IIC-Bus Interface register block.
//
#define S3C6410_BASE_REG_PA_IICBUS        0x7F004000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_UART
//
// Locates the UART register block.
//
#define S3C6410_BASE_REG_PA_UART0        0x7F005000
#define S3C6410_BASE_REG_PA_UART1        0x7F005400
#define S3C6410_BASE_REG_PA_UART2        0x7F005800
#define S3C6410_BASE_REG_PA_UART3        0x7F005C00

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_PWM
//
// Locates the PWM Timer register block.
//
#define S3C6410_BASE_REG_PA_PWM            0x7F006000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_IRDA
//
// Locates the IrDA Controller register block.
//
#define S3C6410_BASE_REG_PA_IRDA            0x7F007000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_GPIO
//
// Locates the GPIO register block.
//
#define S3C6410_BASE_REG_PA_GPIO            0x7F008000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_PCM0
//
// Locates the PCM Audio Interface Ch 0 register block.
//
#define S3C6410_BASE_REG_PA_PCM0            0x7F009000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_PCM1
//
// Locates the PCM Audio Interface Ch 1 register block.
//
#define S3C6410_BASE_REG_PA_PCM1            0x7F00A000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_SPI0
//
// Locates the Serial Periphral Interface Ch 0 register block.
//
#define S3C6410_BASE_REG_PA_SPI0            0x7F00B000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_SPI1
//
// Locates the Serial Periphral Interface Ch 1 register block.
//
#define S3C6410_BASE_REG_PA_SPI1            0x7F00C000
////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_I2S_40
//
// Locates the I2S version 4.0 Bus Interface register block.
//
#define S3C6410_BASE_REG_PA_I2S_40            0x7F00D000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_GPS
//
// Locates the GPS register block.
//
#define S3C6410_BASE_REG_PA_SSS            0x7F00E000

////////////////////////////////////////////////////////////
//
// Define:  S3C6410_BASE_REG_PA_IIC1
//
// Locates the IIC Bus Interface Ch 1 register block.
//
#define S3C6410_BASE_REG_PA_IIC1            0x7F00F000
#if __cplusplus
}
#endif

#endif

