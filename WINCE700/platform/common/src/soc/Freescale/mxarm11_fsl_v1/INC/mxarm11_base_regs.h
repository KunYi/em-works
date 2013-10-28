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
//  Header: mxarm11_base_regs.h
//
//  This header file defines the Physical Addresses (PA) of
//  the base registers for the System on Chip (SoC) components.
//
//  Provides memory map definitions that are shared for ARM11 chassis.
//
//------------------------------------------------------------------------------
#ifndef __MXARM11_BASE_REG_H
#define __MXARM11_BASE_REG_H

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
// ARM11-CHASSIS PERIPHERAL REGISTER MEMORY MAP
//-----------------------------------------------------------------------------

// AIPS A periperhals
#define CSP_BASE_REG_PA_AIPSAREG                (0x43F00000)
#define CSP_BASE_REG_PA_MAX                     (0x43F04000)
#define CSP_BASE_REG_PA_EVTMON                  (0x43F08000)
#define CSP_BASE_REG_PA_CLKCTL                  (0x43F0C000)
#define CSP_BASE_REG_PA_ETBREG                  (0x43F10000)
#define CSP_BASE_REG_PA_ETBMEM                  (0x43F14000)
#define CSP_BASE_REG_PA_ECT                     (0x43F18000)
#define CSP_BASE_REG_PA_I2C                     (0x43F80000)
#define CSP_BASE_REG_PA_UART1                   (0x43F90000)
#define CSP_BASE_REG_PA_UART2                   (0x43F94000)
#define CSP_BASE_REG_PA_OWIRE                   (0x43F9C000)
#define CSP_BASE_REG_PA_SSI1                    (0x43FA0000)
#define CSP_BASE_REG_PA_CSPI1                   (0x43FA4000)
#define CSP_BASE_REG_PA_KPP                     (0x43FA8000)

// AIPS B Peripherals
#define CSP_BASE_REG_PA_SDHC1                   (0x50004000)
#define CSP_BASE_REG_PA_SDHC2                   (0x50008000)
#define CSP_BASE_REG_PA_UART3                   (0x5000C000)
#define CSP_BASE_REG_PA_CSPI2                   (0x50010000)
#define CSP_BASE_REG_PA_SSI2                    (0x50014000)
#define CSP_BASE_REG_PA_IIM                     (0x5001C000)
#define CSP_BASE_REG_PA_SPBA                    (0x5003C000)
#define CSP_BASE_REG_PA_AIPSBREG                (0x53F00000)
#define CSP_BASE_REG_PA_FIRI                    (0x53F8C000)
#define CSP_BASE_REG_PA_GPT                     (0x53F90000)
#define CSP_BASE_REG_PA_EPIT1                   (0x53F94000)
#define CSP_BASE_REG_PA_EPIT2                   (0x53F98000)
#define CSP_BASE_REG_PA_SCC                     (0x53FAC000)
#define CSP_BASE_REG_PA_RNGA                    (0x53FB0000)
#define CSP_BASE_REG_PA_IPU                     (0x53FC0000)
#define CSP_BASE_REG_PA_AUDMUX                  (0x53FC4000)
#define CSP_BASE_REG_PA_GPIO1                   (0x53FCC000)
#define CSP_BASE_REG_PA_SDMA                    (0x53FD4000)
#define CSP_BASE_REG_PA_RTC                     (0x53FD8000)
#define CSP_BASE_REG_PA_WDOG                    (0x53FDC000)
#define CSP_BASE_REG_PA_PWM                     (0x53FE0000)

// Non-AIPS Peripherals
#define CSP_BASE_REG_PA_L2CC                    (0x30000000)
#define CSP_BASE_REG_PA_ROMPATCH                (0x60000000)
#define CSP_BASE_REG_PA_AVIC                    (0x68000000)
#define CSP_BASE_REG_PA_NANDFC                  (0xB8000000)
#define CSP_BASE_REG_PA_ESDCTL                  (0xB8001000)
#define CSP_BASE_REG_PA_WEIM                    (0xB8002000)
#define CSP_BASE_REG_PA_M3IF                    (0xB8003000)


#if __cplusplus
}
#endif

#endif

