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
// Copyright (c) 2007, 2008 BSQUARE Corporation. All rights reserved.

/*
================================================================================
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
================================================================================
*/
//
//  Header:  am387x_base_regs.h
//

//------------------------------------------------------------------------------
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
#ifndef __AM387X_BASE_REGS_H
#define __AM387X_BASE_REGS_H

//---------- L4 Slow Peripheral

#define AM387X_L4S_CONFIG_PA                      0x48000000

#define AM387X_L4S_LINK_AGENT_PA                  0x48000800
#define AM387X_L4S_IP0_PA                         0x48001000  // initiator port 0
#define AM387X_L4S_IP1_PA                         0x48001400  // initiator port 1

#define AM387X_UART0_REGS_PA                      0x48020000
#define AM387X_UART1_REGS_PA                      0x48022000
#define AM387X_UART2_REGS_PA                      0x48024000

#define AM387X_I2C0_REGS_PA                       0x48028000
#define AM387X_I2C1_REGS_PA                       0x4802A000
//#define AM387X_GPTIMER0_REGS_PA                   0x4802C000
#define AM387X_GPTIMER1_REGS_PA                   0x4802E000
#define AM387X_MCSPI0_REGS_PA                     0x48030000
#define AM387X_GPIO0_REGS_PA                      0x48032000
#define AM387X_MCASP0_CFG_REGS_PA                 0x48038000
#define AM387X_MCASP1_CFG_REGS_PA                 0x4803C000

#define AM387X_GPTIMER2_REGS_PA                   0x48040000
#define AM387X_GPTIMER3_REGS_PA                   0x48042000
#define AM387X_GPTIMER4_REGS_PA                   0x48044000
#define AM387X_GPTIMER5_REGS_PA                   0x48046000
#define AM387X_GPTIMER6_REGS_PA                   0x48048000
#define AM387X_GPTIMER7_REGS_PA                   0x4804A000
#define AM387X_GPIO1_REGS_PA                      0x4804C000
#define AM387X_MCASP2_CFG_REGS_PA                 0x48050000
#define AM387X_MCASP2_AFIFO_REGS_PA               0x48051000
#define AM387X_MMCHS0_REGS_PA                     0x48060000 
#define AM387X_MMCHS1_REGS_PA                     0x481D8000 
#define AM387X_ELM_REGS_PA                        0x48080000
#define AM387X_RTC_REGS_PA                        0x480C0000
#define AM387X_MBOX_REGS_PA                       0x480C8000
#define AM387X_SPINLOCK_REGS_PA                   0x480CA000

#define AM387X_DSS_REGS_PA                        0x48100000 // HDVPSS

//#define AM387X_SMMU_REGS_PA                       0x48010000
//#define AM387X_SMC0_REGS_PA                       0x48034000
//#define AM387X_SMC1_REGS_PA                       0x48036000

#define AM387X_HDMI_PHY_0_REGS_PA                 0x46C00300
//#define AM387X_HDMI_WP_0_REGS_PA                  0x46C00000
//#define AM387X_HDMI_CORE_0_REGS_PA                0x46C00400

#define AM387X_L4_CNTRL_MODULE_PA                 0x48140000
//#define AM387X_PINCTRL_REGS_PA					  (AM387X_L4_CNTRL_MODULE_PA + 0x0800)

#define AM387X_OCP_CONF_REGS_PA				(AM387X_L4_CNTRL_MODULE_PA + 0x0000)
#define AM387X_DEVICE_BOOT_REGS_PA			(AM387X_L4_CNTRL_MODULE_PA + 0x0040)
#define AM387X_PLL_CONTROL_REGS_PA          AM387X_DEVICE_BOOT_REGS_PA
#define AM387X_SECnFUSE_KEY_REGS_PA			(AM387X_L4_CNTRL_MODULE_PA + 0x0100)
#define AM387X_SECURE_SM_REGS_PA			(AM387X_L4_CNTRL_MODULE_PA + 0x0300)
#define AM387X_DEVICE_CONF_REGS_PA			(AM387X_L4_CNTRL_MODULE_PA + 0x0400)
#define AM387X_STATUS_CONTROL_REGS_PA		AM387X_DEVICE_CONF_REGS_PA
#define AM387X_SYSC_PADCONFS_REGS_PA		(AM387X_L4_CNTRL_MODULE_PA + 0x0800)
#define AM387X_PINCTRL_REGS_PA				AM387X_SYSC_PADCONFS_REGS_PA
#define AM387X_SYSC_MISC_REGS_PA			(AM387X_L4_CNTRL_MODULE_PA + 0x0E00)
#define AM387X_SYSC_INTR_DMA_MUX_REGS_PA	(AM387X_L4_CNTRL_MODULE_PA + 0x0F00)
#define AM387X_SYSC_MISC2_REGS_PA			(AM387X_L4_CNTRL_MODULE_PA + 0x1300)

//------------------------------------------------------------------------------
#define AM387X_PRCM_REGS_PA                       0x48180000

#define AM387X_SMRFLX0_REGS_PA                    0x48188000
#define AM387X_SMRFLX1_REGS_PA                    0x4818A000
#define AM387X_OCP_WATCHPT_REGS_PA                0x4818C000

#define AM387X_I2C2_REGS_PA                       0x4819C000
#define AM387X_I2C3_REGS_PA                       0x4819E000

#define AM387X_MCSPI1_REGS_PA                     0x481A0000
#define AM387X_MCSPI2_REGS_PA                     0x481A2000
#define AM387X_MCSPI3_REGS_PA                     0x481A4000

#define AM387X_UART3_REGS_PA                      0x481A6000
#define AM387X_UART4_REGS_PA                      0x481A8000
#define AM387X_UART5_REGS_PA                      0x481AA000

#define AM387X_GPIO2_REGS_PA                      0x481AC000
#define AM387X_GPIO3_REGS_PA                      0x481AE000

#define AM387X_GPTIMER8_REGS_PA                   0x481C1000
#define AM387X_SYNCTIMER32K_REGS_PA               0x481C3000

#define AM387X_PLLSS_REGS_PA                      0x481C5000
#define AM387X_WDT0_REGS_PA                       0x481C7000
#define AM387X_DCAN0_REGS_PA                      0x481CC000
#define AM387X_DCAN1_REGS_PA                      0x481D0000
#define AM387X_MMC_REGS_PA                        0x481D8000

//------------------------------------------------------------------------------
//  Interrupt Units
#define AM387X_INTC_MPU_REGS_PA                   0x48200000
#define AM387X_MPUSS_CONG_REGS_PA                 0x48240000
#define AM387X_SSM_REGS_PA                        0x48280000

//---------- L4 Fast Peripheral
#define AM387X_L4F_CFG_REGS_PA				      0x4A000000
#define AM387X_L4F_LINK_AGENT_PA                  0x4A000800
#define AM387X_L4F_IP0_PA                         0x4A001000  // initiator port 0
#define AM387X_L4F_IP1_PA                         0x4A001400  // initiator port 1

#define AM387X_EMACSW_REGS_PA					  0x4A100000
#define AM387X_SATA_REGS_PA						  0x4A140000

#define AM387X_MCASP3_CFG_REGS_PA				  0x4A1A2000
#define AM387X_MCASP3_DATA_REGS_PA				  0x4A1A5000
#define AM387X_MCASP4_CFG_REGS_PA				  0x4A1A8000
#define AM387X_MCASP4_DATA_REGS_PA				  0x4A1AB000
#define AM387X_MCASP5_CFG_REGS_PA				  0x4A1AE000
#define AM387X_MCASP5_DATA_REGS_PA				  0x4A1B1000

// L3 memory
#define AM387X_MCASP0_DATA_REGS_PA				  0x46000000
#define AM387X_MCASP1_DATA_REGS_PA				  0x46400000
#define AM387X_MCASP2_DATA_REGS_PA				  0x46800000

#define AM387X_USBSS_PA							  0x47400000

//------------------------------------------------------------------------------
//  GPMC Module Register base address
//  (see file omap_gpmc.h for offset definitions for this base address)
//  VA: Netra has a newer version of the GPMC with more registers vs. omap
//      but I hope the omap works for now !!!! 
//------------------------------------------------------------------------------
#define AM387X_GPMC_REGS_PA                       0x50000000
#define AM387X_GPMC_REGS_SIZE                     0x00001000

//------------------------------------------------------------------------------
//  DMM/DDR-EMIF Base address 
//  See am387x_dmm.h for offsets
//------------------------------------------------------------------------------
#define AM387X_EMIF4_0_CFG_REGS_PA		0x4C000000
#define AM387X_EMIF4_1_CFG_REGS_PA		0x4D000000
#define AM387X_DMM_REGS_PA				0x4E000000

#endif // __AM387X_BASE_REGS_H
