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
//  Header:  am389x_base_regs.h
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
#ifndef __AM389X_BASE_REGS_H
#define __AM389X_BASE_REGS_H

//---------- L4 Slow Peripheral

#define AM389X_L4S_CONFIG_PA                      0x48000000


#define AM389X_EFUSE_CNTR_PA                      0x48008000

#define AM389X_SMMU_REGS_PA                       0x48010000

#define AM389X_UART0_REGS_PA                      0x48020000
#define AM389X_UART1_REGS_PA                      0x48022000
#define AM389X_UART2_REGS_PA                      0x48024000

#define AM389X_I2C0_REGS_PA                       0x48028000
#define AM389X_I2C1_REGS_PA                       0x4802A000

#define AM389X_GPTIMER0_REGS_PA                   0x4802C000
#define AM389X_GPTIMER1_REGS_PA                   0x4802E000
#define AM389X_GPTIMER2_REGS_PA                   0x48040000
#define AM389X_GPTIMER3_REGS_PA                   0x48042000
#define AM389X_GPTIMER4_REGS_PA                   0x48044000
#define AM389X_GPTIMER5_REGS_PA                   0x48046000
#define AM389X_GPTIMER6_REGS_PA                   0x48048000
#define AM389X_GPTIMER7_REGS_PA                   0x4804A000

#define AM389X_MCSPIOCP_REGS_PA                   0x48030000

#define AM389X_GPIO0_REGS_PA                      0x48032000
#define AM389X_GPIO1_REGS_PA                      0x4804C000

#define AM389X_SMC0_REGS_PA                       0x48034000
#define AM389X_SMC1_REGS_PA                       0x48036000

#define AM389X_MCASP0_CFG_REGS_PA                 0x48038000
#define AM389X_MCASP1_CFG_REGS_PA                 0x4803C000
#define AM389X_MCASP2_CFG_REGS_PA                 0x48050000

#define AM389X_MMCHS_REGS_PA                      0x48060000

// Error location module
#define AM389X_ELM_REGS_PA                        0x48080000

// What is that?
#define AM389X_SPARE_MODULE_PA                    0x480A0000

#define AM389X_RTC_REGS_PA                        0x480C0000

#define AM389X_WDT1_REGS_PA                       0x480C2000

#define AM389X_MBOX_REGS_PA                       0x480C8000

#define AM389X_SPINLOCK_REGS_PA                   0x480CA000

#define AM389X_DSS_REGS_PA                        0x48100000

#define AM389X_HDMI_WP_0_REGS_PA                  0x46C00000
#define AM389X_HDMI_CORE_0_REGS_PA                0x46C00400
#define AM389X_HDMI_PHY_0_REGS_PA                 0x48122000

#define AM389X_L4_CNTRL_MODULE_PA                 0x48140000
#define AM389X_STATUS_CONTROL_REGS_PA             (AM389X_L4_CNTRL_MODULE_PA + 0x40)
#define AM389X_PLL_CONTROL_REGS_PA                (AM389X_L4_CNTRL_MODULE_PA + 0x0400)
#define AM389X_DEVICE_CONF_REGS_PA				  (AM389X_L4_CNTRL_MODULE_PA + 0x0600)
//------------------------------------------------------------------------------
#define AM389X_SYSC_PADCONFS_REGS_PA			  0x48140800

#define AM389X_PRCM_REGS_PA                       0x48180000

#define AM389X_SMRFLX0_REGS_PA                    0x48188000
#define AM389X_SMRFLX1_REGS_PA                    0x4818A000

#define AM389X_OCP_WATCHPT_REGS_PA                0x4818C000

#define AM389X_P1500_CFG_REGS_PA                  0x4818E000

#define AM389X_DDR0_PHY_CTRL_REGS_PA              0x48198000
#define AM389X_DDR1_PHY_CTRL_REGS_PA              0x4819A000

//---------- L4 Fast Peripheral
#define AM389X_L4_FAST_CFG_REGS_PA				0x4A000000
#define AM389X_TPPSS_REGS_PA					0x4A080000
#define AM389X_CPGMAC0_REGS_PA					0x4A100000
#define AM389X_CPGMAC1_REGS_PA					0x4A120000
#define AM389X_SATA_REGS_PA						0x4A140000
#define AM389X_SPARE_REGS_PA					0x4A180000



//------------------------------------------------------------------------------
//  Interrupt Units
#define AM389X_INTC_MPU_REGS_PA                   0x48200000

#define AM389X_MPUSS_CONG_REGS_PA                 0x48240000

#define AM389X_SSM_REGS_PA                        0x48280000

// L3 memory
#define AM389X_MCASP0_PA							0x46000000
#define AM389X_MCASP0_PA							0x46000000
#define AM389X_MCASP0_PA							0x46000000

#define AM389X_USBSS_PA								0x47400000

//------------------------------------------------------------------------------
//  GPMC Module Register base address
//  (see file omap_gpmc.h for offset definitions for this base address)
//  VA: Netra has a newer version of the GPMC with more registers vs. omap
//      but I hope the omap works for now !!!! 
//------------------------------------------------------------------------------
#define AM389X_GPMC_REGS_PA                       0x50000000
#define AM389X_GPMC_REGS_SIZE                     0x00001000

//------------------------------------------------------------------------------
//  DMM/DDR-EMIF Base address 
//  See am389x_dmm.h for offsets
//------------------------------------------------------------------------------
#define AM389X_EMIF4_0_CFG_REGS_PA		0x4C000000
#define AM389X_EMIF4_1_CFG_REGS_PA		0x4D000000
#define AM389X_DMM_REGS_PA				0x4E000000

#define AM389X_DDRPHY_0_CONFIG_REGS_PA	0x48198000
#define AM389X_DDRPHY_1_CONFIG_REGS_PA	0x4819a000
#define AM389X_DDRPHY_CONFIG_REGS_PA	((emif == 0) ? AM389X_DDRPHY_0_CONFIG_REGS_PA:AM389X_DDRPHY_1_CONFIG_REGS_PA)


//----------------------------------------------------

// do not forget to add mode definitions for L3 domain 

//----------------------------------------------------


#endif // __AM389X_BASE_REGS_H
