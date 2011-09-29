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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
//------------------------------------------------------------------------------
//
//  File:  bulverde_base_regs.h
//
//  Intel Bulverde register and physical address definitions.
//
#ifndef _bulverde_base_regs_h_
#define _bulverde_base_regs_h_

////////////////////////////////////////////////////
/* DEVICE BASE ADDRESSES GROUPED BY FUNCTIONALITY */
////////////////////////////////////////////////////

//
// Internal Memory - Storage (256 KB)
//
#define BULVERDE_BASE_REG_PA_IMSTORAGE  0x5C000000

//
// Internal Memory - Control (12 B)
//
#define BULVERDE_BASE_REG_PA_IMCONTROL  0x58000000

//
// Camera Peripheral
//
#define BULVERDE_BASE_REG_PA_CAMERA     0x50000000

//
// USB Host
//
#define BULVERDE_BASE_REG_PA_USBH       0x4C000000

//
// MEMC
//
#define BULVERDE_BASE_REG_PA_MEMC       0x48000000

//
// LCDC
//
#define BULVERDE_BASE_REG_PA_LCD        0x44000000

//
// Peripheral registers base - DMAC, UART[3:1/SIR], I2S/C, AC97, USBC, FIR,
// RTC, OST, PWM, INTC, GPIO, PWRMAN/RESETC, SSP[3:1], MMC, CLKMAN, BB, KEYPAD,
// USIM, MEMSTICK
//
#define BULVERDE_BASE_REG_PA_PERIPH     0x40000000

//
// PCMCIA Slots 0,1
//
#define BULVERDE_BASE_REG_PA_PCMCIA_S0_IO    0x20000000
#define BULVERDE_BASE_REG_PA_PCMCIA_S0_ATTR  0x28000000
#define BULVERDE_BASE_REG_PA_PCMCIA_S0_CMN   0x2C000000
#define BULVERDE_BASE_REG_PA_PCMCIA_S1_IO    0x30000000
#define BULVERDE_BASE_REG_PA_PCMCIA_S1_ATTR  0x38000000
#define BULVERDE_BASE_REG_PA_PCMCIA_S1_CMN   0x3C000000
 

/////////////////////////////////////////////////////////////////////////////////////////
/* PERIPHERAL OFFSETS */
/////////////////////////////////////////////////////////////////////////////////////////

#define DMAC_OFFSET                     0x0             // DMA CONTROLLER
#define FFUART_OFFSET                   0x00100000      // Full-Feature UART
#define BTUART_OFFSET                   0x00200000      // BlueTooth UART
#define I2C_OFFSET                      0x00300000      // I2C
#define I2S_OFFSET                      0x00400000      // I2S
#define AC97_OFFSET                     0x00500000      // AC97
#define UDC_OFFSET                      0x00600000      // UDC (usb client)
#define STUART_OFFSET                   0x00700000      // Standard UART
#define FIR_OFFSET                      0x00800000      // FIR
#define RTC_OFFSET                      0x00900000      // real time clock
#define OST_OFFSET                      0x00A00000      // OS Timer
#define PWM0_2_OFFSET                   0x00B00000      // PWM 0 (pulse-width mod)
#define PWM1_3_OFFSET                   0x00C00000      // PWM 1 (pulse-width mod)
#define INTC_OFFSET                     0x00D00000      // Interrupt controller
#define GPIO_OFFSET                     0x00E00000      // GPIO
#define PWR_OFFSET                      0x00F00000      // Power Manager and Reset Control
#define SSP1_OFFSET                     0x01000000      // SSP 1
#define MMC_OFFSET                      0x01100000      // MMC
#define CLKMGR_OFFSET                   0x01300000      // Clock Manager
#define BB_OFFSET                       0x01400000      // Baseband Interface
#define KEYPAD_OFFSET                   0x01500000      // Keypad Interface
#define USIM_OFFSET                     0x01600000      // USIM
#define SSP2_OFFSET                     0x01700000      // SSP 2
#define MEMSTK_OFFSET                   0x01800000      // Memory Stick
#define SSP3_OFFSET                     0x01900000      // SSP 3

/////////////////////////////////////////////////////////////////////////////////////////
/* DEVICE-SPECIFIC ADDRESS DEFINITIONS */
/////////////////////////////////////////////////////////////////////////////////////////

#define BULVERDE_BASE_REG_PA_DMAC       (BULVERDE_BASE_REG_PA_PERIPH + DMAC_OFFSET)
#define BULVERDE_BASE_REG_PA_FFUART     (BULVERDE_BASE_REG_PA_PERIPH + FFUART_OFFSET)
#define BULVERDE_BASE_REG_PA_BTUART     (BULVERDE_BASE_REG_PA_PERIPH + BTUART_OFFSET)
#define BULVERDE_BASE_REG_PA_STUART     (BULVERDE_BASE_REG_PA_PERIPH + STUART_OFFSET)
#define BULVERDE_BASE_REG_PA_I2C        (BULVERDE_BASE_REG_PA_PERIPH + I2C_OFFSET)
#define BULVERDE_BASE_REG_PA_I2S        (BULVERDE_BASE_REG_PA_PERIPH + I2S_OFFSET)
#define BULVERDE_BASE_REG_PA_AC97       (BULVERDE_BASE_REG_PA_PERIPH + AC97_OFFSET)
#define BULVERDE_BASE_REG_PA_UDC        (BULVERDE_BASE_REG_PA_PERIPH + UDC_OFFSET)
#define BULVERDE_BASE_REG_PA_FIR        (BULVERDE_BASE_REG_PA_PERIPH + FIR_OFFSET)
#define BULVERDE_BASE_REG_PA_RTC        (BULVERDE_BASE_REG_PA_PERIPH + RTC_OFFSET)
#define BULVERDE_BASE_REG_PA_OST        (BULVERDE_BASE_REG_PA_PERIPH + OST_OFFSET)
#define BULVERDE_BASE_REG_PA_PWM0_2     (BULVERDE_BASE_REG_PA_PERIPH + PWM0_2_OFFSET)
#define BULVERDE_BASE_REG_PA_PWM1_3     (BULVERDE_BASE_REG_PA_PERIPH + PWM1_3_OFFSET)
#define BULVERDE_BASE_REG_PA_INTC       (BULVERDE_BASE_REG_PA_PERIPH + INTC_OFFSET)
#define BULVERDE_BASE_REG_PA_GPIO       (BULVERDE_BASE_REG_PA_PERIPH + GPIO_OFFSET)
#define BULVERDE_BASE_REG_PA_PWR        (BULVERDE_BASE_REG_PA_PERIPH + PWR_OFFSET)
#define BULVERDE_BASE_REG_PA_SSP1       (BULVERDE_BASE_REG_PA_PERIPH + SSP1_OFFSET)
#define BULVERDE_BASE_REG_PA_MMC        (BULVERDE_BASE_REG_PA_PERIPH + MMC_OFFSET)
#define BULVERDE_BASE_REG_PA_CLKMGR     (BULVERDE_BASE_REG_PA_PERIPH + CLKMGR_OFFSET)
#define BULVERDE_BASE_REG_PA_BB         (BULVERDE_BASE_REG_PA_PERIPH + BB_OFFSET)
#define BULVERDE_BASE_REG_PA_KEYPAD     (BULVERDE_BASE_REG_PA_PERIPH + KEYPAD_OFFSET)
#define BULVERDE_BASE_REG_PA_USIM       (BULVERDE_BASE_REG_PA_PERIPH + USIM_OFFSET)
#define BULVERDE_BASE_REG_PA_SSP2       (BULVERDE_BASE_REG_PA_PERIPH + SSP2_OFFSET)
#define BULVERDE_BASE_REG_PA_MEMSTK     (BULVERDE_BASE_REG_PA_PERIPH + MEMSTK_OFFSET)
#define BULVERDE_BASE_REG_PA_SSP3       (BULVERDE_BASE_REG_PA_PERIPH + SSP3_OFFSET)


#endif  // _bulverde_base_regs_h_.

