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
// Portions Copyright (c) Texas Instruments.  All rights reserved. 
//
//------------------------------------------------------------------------------
//
//  Header:  omap5912_base_regs.h
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
#ifndef __OMAP5912_BASE_REGS_H
#define __OMAP5912_BASE_REGS_H

//------------------------------------------------------------------------------
//  Configuration Unit
//------------------------------------------------------------------------------

#define OMAP5912_CONFIG_REGS_PA         0xFFFE1000

//------------------------------------------------------------------------------
//  TIPB Bridges
//------------------------------------------------------------------------------

#define OMAP5912_TIPBPRIV_REGS_PA       0xFFFECA00
#define OMAP5912_TIPBPUBL_REGS_PA       0xFFFED300

//------------------------------------------------------------------------------
//  ULPD/Clock/DPLL1 Units
//------------------------------------------------------------------------------

#define OMAP5912_ULPD_REGS_PA           0xFFFE0800
#define OMAP5912_CLKM_REGS_PA           0xFFFECE00

//------------------------------------------------------------------------------
//  UART Units
//------------------------------------------------------------------------------

#define OMAP5912_UART1_REGS_PA          0xFFFB0000
#define OMAP5912_UART2_REGS_PA          0xFFFB0800
#define OMAP5912_UART3_REGS_PA          0xFFFB9800

//------------------------------------------------------------------------------
//  USB OTG Controller
//------------------------------------------------------------------------------

#define OMAP5912_OTG_REGS_PA            0xFFFB0400

//------------------------------------------------------------------------------
//  USB Device Controller
//------------------------------------------------------------------------------

#define OMAP5912_USBD_REGS_PA           0xFFFB4000

//------------------------------------------------------------------------------
//  RTC Unit
//------------------------------------------------------------------------------

#define OMAP5912_RTC_REGS_PA            0xFFFB4800

//------------------------------------------------------------------------------
//  HDQ/1Wire Controller
//------------------------------------------------------------------------------

#define OMAP5912_HDQ_1WIRE_REGS_PA      0xFFFBC000

//------------------------------------------------------------------------------
//  SD Controller
//------------------------------------------------------------------------------

#define OMAP5912_MMCSD_REGS_PA           0xFFFB7800
#define OMAP5912_MMCSD2_REGS_PA          0xFFFB7C00

//------------------------------------------------------------------------------
//  32kHz Timer Units
//------------------------------------------------------------------------------

#define OMAP5912_TIMER32K_REGS_PA       0xFFFB9000

//------------------------------------------------------------------------------
//  USB Host Controller
//------------------------------------------------------------------------------

#define OMAP5912_USBH_REGS_PA           0xFFFBA000

//------------------------------------------------------------------------------
//  DSP MMU Controller
//------------------------------------------------------------------------------

#define OMAP5912_DSP_MMU_REGS_PA        0xFFFED200

//------------------------------------------------------------------------------
//  GPIO Controllers
//------------------------------------------------------------------------------

#define OMAP5912_GPIO1_REGS_PA          0xFFFBE400             
#define OMAP5912_GPIO2_REGS_PA          0xFFFBEC00              
#define OMAP5912_GPIO3_REGS_PA          0xFFFBB400              
#define OMAP5912_GPIO4_REGS_PA          0xFFFBBC00              

//------------------------------------------------------------------------------
//  OCPx Controllers
//------------------------------------------------------------------------------

#define OMAP5912_OCPI_REGS_PA           0xFFFEC320
#define OMAP5912_OCPT_REGS_PA           0xFFFECC00

//------------------------------------------------------------------------------
//  Timer Units
//------------------------------------------------------------------------------

#define OMAP5912_TIMER1_REGS_PA         0xFFFEC500
#define OMAP5912_TIMER2_REGS_PA         0xFFFEC600
#define OMAP5912_TIMER3_REGS_PA         0xFFFEC700

//------------------------------------------------------------------------------
//  Watchdog Unit
//------------------------------------------------------------------------------

#define OMAP5912_WDOG_REGS_PA           0xFFFEC800

//------------------------------------------------------------------------------
//  Interrupt Units
//------------------------------------------------------------------------------

#define OMAP5912_INTC_L1_REGS_PA        0xFFFECB00
#define OMAP5912_INTC_L2A_REGS_PA       0xFFFE0000
#define OMAP5912_INTC_L2B_REGS_PA       0xFFFE0100
#define OMAP5912_INTC_L2C_REGS_PA       0xFFFE0200
#define OMAP5912_INTC_L2D_REGS_PA       0xFFFE0300

//------------------------------------------------------------------------------
//  MicroWire Controller
//------------------------------------------------------------------------------

#define OMAP5912_UWIRE_REGS_PA          0xFFFB3000

//------------------------------------------------------------------------------
//  I2C Controller
//------------------------------------------------------------------------------

#define OMAP5912_I2C_REGS_PA            0xFFFB3800

//------------------------------------------------------------------------------
//  ARM/KBD I/O Controller (ARMIO)
//------------------------------------------------------------------------------

#define OMAP5912_ARMIO_REGS_PA          0xFFFB5000            

//------------------------------------------------------------------------------
//  DMA Controllers
//------------------------------------------------------------------------------

#define OMAP5912_DMA_REGS_PA            0xFFFEDC00

#define OMAP5912_DMA_LC1_REGS_PA        0xFFFED800
#define OMAP5912_DMA_LC2_REGS_PA        0xFFFED840
#define OMAP5912_DMA_LC3_REGS_PA        0xFFFED880
#define OMAP5912_DMA_LC4_REGS_PA        0xFFFED8C0
#define OMAP5912_DMA_LC5_REGS_PA        0xFFFED900
#define OMAP5912_DMA_LC6_REGS_PA        0xFFFED940
#define OMAP5912_DMA_LC7_REGS_PA        0xFFFED980
#define OMAP5912_DMA_LC8_REGS_PA        0xFFFED9C0
#define OMAP5912_DMA_LC9_REGS_PA        0xFFFEDA00
#define OMAP5912_DMA_LC10_REGS_PA       0xFFFEDA40
#define OMAP5912_DMA_LC11_REGS_PA       0xFFFEDA80
#define OMAP5912_DMA_LC12_REGS_PA       0xFFFEDAC0
#define OMAP5912_DMA_LC13_REGS_PA       0xFFFEDB00
#define OMAP5912_DMA_LC14_REGS_PA       0xFFFEDB40
#define OMAP5912_DMA_LC15_REGS_PA       0xFFFEDB80
#define OMAP5912_DMA_LC16_REGS_PA       0xFFFEDBC0

#define OMAP5912_DMA_LCD_REGS_PA        0xFFFEEC00

//------------------------------------------------------------------------------
//  LCD & LCDCONV Controllers
//------------------------------------------------------------------------------

#define OMAP5912_LCD_REGS_PA            0xFFFEC000
#define OMAP5912_LCDCONV_REGS_PA        0xFFFE3000

//------------------------------------------------------------------------------
//  EMIF Unit
//------------------------------------------------------------------------------

#define OMAP5912_EMIF_REGS_PA           0xFFFECC00

//------------------------------------------------------------------------------
//  McBSP Units
//------------------------------------------------------------------------------

#define OMAP5912_MCBSP1_REGS_PA         0xE1011800
#define OMAP5912_MCBSP2_REGS_PA         0xFFFB1000
#define OMAP5912_MCBSP3_REGS_PA         0xE1017000

//------------------------------------------------------------------------------
//  MCSI Units
//------------------------------------------------------------------------------

#define OMAP5912_MCSI1_REGS_PA          0xE1012800
#define OMAP5912_MCSI2_REGS_PA          0xE1012000

//------------------------------------------------------------------------------
//  SPI Units
//------------------------------------------------------------------------------

#define OMAP5912_SPI1_REGS_PA           0xFFFB0C00

//------------------------------------------------------------------------------
//  General-Purpose Timer Units
//------------------------------------------------------------------------------

#define OMAP5912_GP_TIMER1_REGS_PA      0xFFFB1400
#define OMAP5912_GP_TIMER2_REGS_PA      0xFFFB1C00
#define OMAP5912_GP_TIMER3_REGS_PA      0xFFFB2400
#define OMAP5912_GP_TIMER4_REGS_PA      0xFFFB2C00
#define OMAP5912_GP_TIMER5_REGS_PA      0xFFFB3400
#define OMAP5912_GP_TIMER6_REGS_PA      0xFFFB3C00
#define OMAP5912_GP_TIMER7_REGS_PA      0xFFFB7400
#define OMAP5912_GP_TIMER8_REGS_PA      0xFFFBD400

//------------------------------------------------------------------------------
//  PWL Controller
//------------------------------------------------------------------------------

#define OMAP5912_PWL_REGS_PA            0xFFFB5800

//------------------------------------------------------------------------------
//  PWT Controller
//------------------------------------------------------------------------------

#define OMAP5912_PWT_REGS_PA            0xFFFB6000

//------------------------------------------------------------------------------
//  FAC Unit
//------------------------------------------------------------------------------

#define OMAP5912_FAC_REGS_PA            0xFFFBA800

//------------------------------------------------------------------------------
//  MPU TIPB Bus Switch
//------------------------------------------------------------------------------

#define OMAP5912_TIPB_SWITCH_REGS_PA    0xFFFBC800

//------------------------------------------------------------------------------
//  LPG Controller
//------------------------------------------------------------------------------

#define OMAP5912_LPG1_REGS_PA           0xFFFBD000
#define OMAP5912_LPG2_REGS_PA           0xFFFBD800

//------------------------------------------------------------------------------
//  Shared Mailbox
//------------------------------------------------------------------------------

#define OMAP5912_MAILBOX_REGS_PA        0xFFFCF000

//------------------------------------------------------------------------------
//  Device Die Identification
//------------------------------------------------------------------------------

#define OMAP5912_DEVICE_ID_REGS_PA      0xFFFE1800

//------------------------------------------------------------------------------
//  Production Identification
//------------------------------------------------------------------------------

#define OMAP5912_PRODUCT_ID_REGS_PA     0xFFFE2000

//------------------------------------------------------------------------------
//  19s WATCHDOG UNIT
//------------------------------------------------------------------------------

#define OMAP5912_19SWATCHDOG_REGS_PA    0xFFFEB000

//------------------------------------------------------------------------------
//  MPU Interface
//------------------------------------------------------------------------------

#define OMAP5912_MPUI_REGS_PA           0xFFFEC900

//------------------------------------------------------------------------------
//  DPLL1
//------------------------------------------------------------------------------

#define OMAP5912_DPLL1_REGS_PA          0xFFFECF00

//------------------------------------------------------------------------------
//  32-kHz Synchro Count
//------------------------------------------------------------------------------

#define OMAP5912_SYNC_TIMER_REGS_PA     0xFFFED400

//------------------------------------------------------------------------------
//  SRAM embedded memory
//------------------------------------------------------------------------------

#define OMAP5912_SRAM_SIZE              0x0003E800
#define OMAP5912_SRAM_BASE_PA           0x20000000

//------------------------------------------------------------------------------

#endif // __OMAP5912_BASE_REGS_H
