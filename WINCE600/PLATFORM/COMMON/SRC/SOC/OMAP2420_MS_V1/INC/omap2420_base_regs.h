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
//
// Portions Copyright (c) Texas Instruments.  All rights reserved. 
//
//------------------------------------------------------------------------------
//
//  Header:  omap2420_base_regs.h
//
//  This header file defines the addresses of the base registers for
//  the System on Chip (SoC) components.
//
//  The following abbreviations are used for different addressing type:
//
//                PA - physical address
//                UA - uncached virtual address
//                CA - cached virtual address
//                OA - offset address
//
//  The naming convention for CPU base registers is:
//
//                <SoC>_<SUBSYSTEM>_REGS_<ADDRTYPE>
//
#ifndef __OMAP2420_BASE_REGS_H
#define __OMAP2420_BASE_REGS_H

//------------------------------------------------------------------------------
//  Device ID
//------------------------------------------------------------------------------
#define OMAP2420_DEVICE_ID_REGS_PA      0x48014218

//------------------------------------------------------------------------------
//  Configuration Unit
//------------------------------------------------------------------------------

#define OMAP2420_REGS_PA                0x48000000
#define OMAP2420_REGS_SIZE              0x08000000

#define OMAP2420_CONFIG_REGS_PA         0x48000000

//------------------------------------------------------------------------------
//  Power, Reset, Clock Management module
//------------------------------------------------------------------------------

#define OMAP2420_PRCM_REGS_PA           0x48008000

//------------------------------------------------------------------------------
//  UART Units
//------------------------------------------------------------------------------

#define OMAP2420_UART1_REGS_PA          0x4806A000
#define OMAP2420_UART2_REGS_PA          0x4806C000
#define OMAP2420_UART3_REGS_PA          0x4806E000

//------------------------------------------------------------------------------
//  USB 
//------------------------------------------------------------------------------

#define OMAP2420_USBOT1_REG_PA          0x4805E000

//------------------------------------------------------------------------------
//  USB OTG Controller
//------------------------------------------------------------------------------

#define OMAP2420_OTG_REGS_PA            0x4805E300

//------------------------------------------------------------------------------
//  USB Device Controller
//------------------------------------------------------------------------------

#define OMAP2420_USBD_REGS_PA           0x4805E200

//------------------------------------------------------------------------------
//  Camera Controller
//------------------------------------------------------------------------------

#define OMAP2420_CAMSUB_REGS_PA         0x48052000 // Camera top
#define OMAP2420_CAMCORE_REGS_PA        0x48052400 // Camera core
#define OMAP2420_CAMDMA_REGS_PA         0x48052800 // Camera DMA
#define OMAP2420_CAMMMU_REGS_PA         0x48052C00 // Camera MMU

//------------------------------------------------------------------------------
//  IVA MMU base address
//------------------------------------------------------------------------------

#define OMAP2420_IVA_MMU_REGS_PA        0x5D000000

//------------------------------------------------------------------------------
//  HDQ/1Wire Controller
//------------------------------------------------------------------------------

#define OMAP2420_HDQ_1WIRE_REGS_PA      0x480B2000

//------------------------------------------------------------------------------
//  MMC/SDIO Module
//------------------------------------------------------------------------------

#define OMAP2420_MSDI1_REGS_PA          0x4809C000

//------------------------------------------------------------------------------
//  32kHz Timer Units
//------------------------------------------------------------------------------

#define OMAP2420_TIMER32K_REGS_PA       0x48004000

//
// General Purpose Timers.
//

#define OMAP2420_GPTIMER1_REGS_PA       0x48028000
#define OMAP2420_GPTIMER2_REGS_PA       0x4802A000
#define OMAP2420_GPTIMER3_REGS_PA       0x48078000
#define OMAP2420_GPTIMER4_REGS_PA       0x4807A000
#define OMAP2420_GPTIMER5_REGS_PA       0x4807C000
#define OMAP2420_GPTIMER6_REGS_PA       0x4807E000
#define OMAP2420_GPTIMER7_REGS_PA       0x48080000
#define OMAP2420_GPTIMER8_REGS_PA       0x48082000
#define OMAP2420_GPTIMER9_REGS_PA       0x48084000
#define OMAP2420_GPTIMER10_REGS_PA      0x48086000
#define OMAP2420_GPTIMER11_REGS_PA      0x48088000
#define OMAP2420_GPTIMER12_REGS_PA      0x4808A000

//------------------------------------------------------------------------------
//  USB Host Controller
//------------------------------------------------------------------------------

#define OMAP2420_USBH_REGS_PA           0x4805E000

//------------------------------------------------------------------------------
//  EAC Controller
//------------------------------------------------------------------------------

#define OMAP2420_EAC_REGS_PA            0x48090000

//------------------------------------------------------------------------------
//  Mailbox registers
//------------------------------------------------------------------------------

#define OMAP2420_MLB1_REGS_PA           0x48094000
#define OMAP2420_MLB2_REGS_PA           0x48095000

//------------------------------------------------------------------------------
//  DSP MMU Controller
//------------------------------------------------------------------------------

#define OMAP2420_DSP_MMU_REGS_PA        0x5A000000

//------------------------------------------------------------------------------
//  GPIO Controllers
//------------------------------------------------------------------------------

#define OMAP2420_GPIO1_REGS_PA          0x48018000
#define OMAP2420_GPIO2_REGS_PA          0x4801A000
#define OMAP2420_GPIO3_REGS_PA          0x4801C000
#define OMAP2420_GPIO4_REGS_PA          0x4801E000

//------------------------------------------------------------------------------
//  OCP Controller
//------------------------------------------------------------------------------

#define OMAP2420_OCP_REG_PA             0x5C060000

//------------------------------------------------------------------------------
//  Timer Units
//------------------------------------------------------------------------------

#define OMAP2420_TIMER1_REGS_PA         0x48028000
#define OMAP2420_TIMER2_REGS_PA         0x4802A000
#define OMAP2420_TIMER3_REGS_PA         0x48078000
#define OMAP2420_TIMER4_REGS_PA         0x4807A000
#define OMAP2420_TIMER5_REGS_PA         0x4807C000
#define OMAP2420_TIMER6_REGS_PA         0x4807E000
#define OMAP2420_TIMER7_REGS_PA         0x48080000
#define OMAP2420_TIMER8_REGS_PA         0x48082000
#define OMAP2420_TIMER9_REGS_PA         0x48084000
#define OMAP2420_TIMER10_REGS_PA        0x48086000
#define OMAP2420_TIMER11_REGS_PA        0x48088000
#define OMAP2420_TIMER12_REGS_PA        0x4808A000

//------------------------------------------------------------------------------
//  Watchdog Unit
//------------------------------------------------------------------------------

#define OMAP2420_WDOG1_REGS_PA          0x48020000 // secure
#define OMAP2420_WDOG2_REGS_PA          0x48022000 // omap
#define OMAP2420_WDOG3_REGS_PA          0x48024000 // dsp
#define OMAP2420_WDOG4_REGS_PA          0x48026000 // iva

//------------------------------------------------------------------------------
//  Interrupt Units
//------------------------------------------------------------------------------

#define OMAP2420_INTC_MPU_REGS_PA       0x480FE000
#define OMAP2420_INTC_IVA_REGS_PA       0x40000000

//------------------------------------------------------------------------------
//  I2C Controller
//------------------------------------------------------------------------------

#define OMAP2420_I2C1_REGS_PA           0x48070000
#define OMAP2420_I2C2_REGS_PA           0x48072000

//------------------------------------------------------------------------------
//  System and DSP DMA Controllers
//------------------------------------------------------------------------------

// The registers 0ffset 0x00 - 0x7C are common to all the DMA's
// So the logical channel starts at offset 0x80.

#define OMAP2420_SDMA_REGS_PA           0x48056000
//
// SDMA
//
#define OMAP2420_DMA_LCD_REGS_PA        0x48056080

// Two IOMA values are available (0xF80000 & 0xFC0000). 
// We need to identify the IOMA used.
#define OMAP2420_DDMA_REGS_PA           (0xF80000 + 0xC000) 

//------------------------------------------------------------------------------
//  Display Subsystem
//------------------------------------------------------------------------------

#define OMAP2420_DISS1_REGS_PA          0x48050000
#define OMAP2420_DISC1_REGS_PA          0x48050400
#define OMAP2420_RFBI1_REGS_PA          0x48050800
#define OMAP2420_VENC1_REGS_PA          0x48050C00

//------------------------------------------------------------------------------
//  McBSP base addresses
//  (see file omap2420_McBSP.h for offset definitions for these base addresses)
//------------------------------------------------------------------------------

#define OMAP2420_MCBSP1_REGS_PA         0x48074000
#define OMAP2420_MCBSP2_REGS_PA         0x48076000

//------------------------------------------------------------------------------
//  SRAM embedded memory
//------------------------------------------------------------------------------

#define OMAP2420_SRAM_SIZE              (640*1024)
#define OMAP2420_SRAM_PA                0x40206000

//-----------------------------------------------------------------------------
// SDRAM for Display
//-----------------------------------------------------------------------------

#define OMAP2420_SDRAM_LCD_PA           0xA1F00000
#define OMAP2420_SDRAM_LCD_SIZE         0x01000000

//------------------------------------------------------------------------------
//  Control Pad Configuration Module Register base address
//  (see file ctrlpadconf.h for offset definitions for this base address)
//------------------------------------------------------------------------------

#define OMAP2420_SYSC1_REGS_PA          0x48000000

//------------------------------------------------------------------------------
//  GPMC Module Register base address
//  (see file omap2420_gpmc.h for offset definitions for this base address)
//------------------------------------------------------------------------------

#define OMAP2420_GPMC_REGS_PA           0x6800A000

//------------------------------------------------------------------------------
//  SDRAM module register base addresses
//  (see file omap2420_sdram.h for offset definitions for these base addresses)
//------------------------------------------------------------------------------

#define OMAP2420_SMS_REGS_PA            0x68008000
#define OMAP2420_SDRC_REGS_PA           0x68009000

//
// Frame Adjustment Counter Register (FAC)
// (see file omap24240_timer.h for offset definitions)
//

#define OMAP2420_FAC_REGS_PA            0x48092000

//------------------------------------------------------------------------------
//  McSPI base addresses
//  (see file omap2420_McSPI.h for offset definitions for these base addresses)
//------------------------------------------------------------------------------

#define OMAP2420_MCSPI1_REGS_PA         0x48098000 
#define OMAP2420_MCSPI2_REGS_PA         0x4809A000

//------------------------------------------------------------------------------
//  DSP subsystem (IPI module, dealing with OMAP 24xx memory space) base address
//  (see file omap2420_dsp.h for offset definitions for this base address)
//------------------------------------------------------------------------------

#define OMAP2420_DSP_IPI_REGS_PA        0x59000000

//------------------------------------------------------------------------------
//  vlynq module, see file omap2420_vlynq.h for offset definitions for this base address
//------------------------------------------------------------------------------

#define OMAP2420_VLYNQFUNC_REGS_PA      0x67FFFE00

//------------------------------------------------------------------------------
//  SSI Controller, see file omap2420_SSI.h for offset definitions
//------------------------------------------------------------------------------

#define OMAP2420_SSI_REGS_PA            0x48058000  // SSI controller
#define OMAP2420_GDD1_REGS_PA           0x48059000  // Generic distribute DMD port 1
#define OMAP2420_SST1_REGS_PA           0x4805A000  // Synchronized serial tranmitter port 1
#define OMAP2420_SSR1_REGS_PA           0x4805A800  // Synchronized serial receiver port 1
#define OMAP2420_SST2_REGS_PA           0x4805B000  // Synchronized serial transmitter port 2
#define OMAP2420_SSR2_REGS_PA           0x4805B800  // Synchronized serial reciever port 2

#endif // __OMAP2420_BASE_REGS_H
