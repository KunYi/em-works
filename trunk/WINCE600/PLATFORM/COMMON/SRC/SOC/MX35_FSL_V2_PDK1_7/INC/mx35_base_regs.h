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
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header: mx35_base_regs.h
//
//  This header file defines the Physical Addresses (PA) of
//  the base registers for the System on Chip (SoC) components.
//
//  Provides memory map definitions that are specific to the MX35
//  architecture.
//
//-----------------------------------------------------------------------------

#ifndef __MX35_BASE_REGS_H
#define __MX35_BASE_REGS_H

#if __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------------------------
//  INFORMATION
//
//  The physical addresses for SoC registers are fixed, hence they are defined
//  in the CPU's common directory. The virtual addresses of the SoC registers
//  are defined by the OEM and are configured in the platform's configuration
//  directory by the file: .../PLATFORM/<NAME>/SRC/CONFIG/CPU_BASE_REG_CFG.H.
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// MX35-SPECIFIC PERIPHERAL REGISTER MEMORY MAP
//-----------------------------------------------------------------------------

// AIPS 1 periperhals
#define CSP_BASE_REG_PA_AIPS1                   (0x43F00000)
#define CSP_BASE_REG_PA_MAX                     (0x43F04000)
#define CSP_BASE_REG_PA_EVTMON                  (0x43F08000)
#define CSP_BASE_REG_PA_CLKCTL                  (0x43F0C000)
#define CSP_BASE_REG_PA_ETBREG                  (0x43F10000)
#define CSP_BASE_REG_PA_ETBMEM                  (0x43F14000)
#define CSP_BASE_REG_PA_ECT                     (0x43F18000)
#define CSP_BASE_REG_PA_I2C1                    (0x43F80000)
#define CSP_BASE_REG_PA_I2C3                    (0x43F84000)
#define CSP_BASE_REG_PA_UART1                   (0x43F90000)
#define CSP_BASE_REG_PA_UART2                   (0x43F94000)
#define CSP_BASE_REG_PA_I2C2                    (0x43F98000)
#define CSP_BASE_REG_PA_OWIRE                   (0x43F9C000)
#define CSP_BASE_REG_PA_SSI1                    (0x43FA0000)
#define CSP_BASE_REG_PA_CSPI1                   (0x43FA4000)
#define CSP_BASE_REG_PA_KPP                     (0x43FA8000)
#define CSP_BASE_REG_PA_IOMUXC                  (0x43FAC000)
#define CSP_BASE_REG_PA_ECT_IPBUS1              (0x43FB8000)
#define CSP_BASE_REG_PA_ECT_IPBUS2              (0x43FBC000)

// AIPS 2 Peripherals
#define CSP_BASE_REG_PA_UART3                   (0x5000C000)
#define CSP_BASE_REG_PA_CSPI2                   (0x50010000)
#define CSP_BASE_REG_PA_SSI2                    (0x50014000)
#define CSP_BASE_REG_PA_ATA                     (0x50020000)
#define CSP_BASE_REG_PA_MSHC                    (0x50024000)
#define CSP_BASE_REG_PA_SPDIF                   (0x50028000)
#define CSP_BASE_REG_PA_ASRC                    (0x5002C000)
#define CSP_BASE_REG_PA_ESAI                    (0x50034000)
#define CSP_BASE_REG_PA_FEC                     (0x50038000)
#define CSP_BASE_REG_PA_SPBA                    (0x5003C000)
#define CSP_BASE_REG_PA_AIPS2                   (0x53F00000)
#define CSP_BASE_REG_PA_CCM                     (0x53F80000)
#define CSP_BASE_REG_PA_GPT                     (0x53F90000)
#define CSP_BASE_REG_PA_EPIT1                   (0x53F94000)
#define CSP_BASE_REG_PA_EPIT2                   (0x53F98000)
#define CSP_BASE_REG_PA_GPIO3                   (0x53FA4000)
#define CSP_BASE_REG_PA_SCC                     (0x53FAC000)
#define CSP_BASE_REG_PA_RNGC                    (0x53FB0000)
#define CSP_BASE_REG_PA_ESDHC1                  (0x53FB4000)
#define CSP_BASE_REG_PA_ESDHC2                  (0x53FB8000)
#define CSP_BASE_REG_PA_ESDHC3                  (0x53FBC000)
#define CSP_BASE_REG_PA_IPU                     (0x53FC0000)
#define CSP_BASE_REG_PA_AUDMUX                  (0x53FC4000)
#define CSP_BASE_REG_PA_GPIO1                   (0x53FCC000)
#define CSP_BASE_REG_PA_GPIO2                   (0x53FD0000)
#define CSP_BASE_REG_PA_SDMA                    (0x53FD4000)
#define CSP_BASE_REG_PA_RTC                     (0x53FD8000)
#define CSP_BASE_REG_PA_WDOG                    (0x53FDC000)
#define CSP_BASE_REG_PA_PWM                     (0x53FE0000)
#define CSP_BASE_REG_PA_CAN1                    (0x53FE4000)
#define CSP_BASE_REG_PA_CAN2                    (0x53FE8000)
#define CSP_BASE_REG_PA_RTIC                    (0x53FEC000)
#define CSP_BASE_REG_PA_IIM                     (0x53FF0000)
#define CSP_BASE_REG_PA_USB                     (0x53FF4000)
#define CSP_BASE_REG_PA_MLB                     (0x53FF8000)

// Non-AIPS Peripherals
#define CSP_BASE_REG_PA_GPU2D                   (0x20000000)
#define CSP_BASE_REG_PA_L2CC                    (0x30000000)
#define CSP_BASE_REG_PA_ROMPATCH                (0x60000000)
#define CSP_BASE_REG_PA_AVIC                    (0x68000000)
#define CSP_BASE_REG_PA_ESDCTL                  (0xB8001000)
#define CSP_BASE_REG_PA_WEIM                    (0xB8002000)
#define CSP_BASE_REG_PA_M3IF                    (0xB8003000)
#define CSP_BASE_REG_PA_EMI                     (0xB8004000)
#define CSP_BASE_REG_PA_NANDFC                  (0xBB000000)

#if __cplusplus
}
#endif

#endif

