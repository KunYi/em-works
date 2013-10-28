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
//-----------------------------------------------------------------------------
//
// Copyright (C) 2006, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header: mx27_base_reg.inc
//
//  This header file defines the Physical Addresses (PA) of 
//  the base registers for the System on Chip (SoC) components.
//
//  Provides memory map definitions for MX27 peripherals.
//
//------------------------------------------------------------------------------
//
//  INFORMATION
//
//  The physical addresses for SoC registers are fixed, hence they are defined
//  in the CPU's common directory. The virtual addresses of the SoC registers
//  are defined by the OEM and are configured in the platform's configuration 
//  directory by the file: .../PLATFORM/<NAME>/SRC/CONFIG/CPU_BASE_REG_CFG.H.
//
//-----------------------------------------------------------------------------
//
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
#ifndef __MX27_BASE_REGS_H__
#define __MX27_BASE_REGS_H__

#if __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------------------------
// MX27 REGISTER MEMORY MAP
//-----------------------------------------------------------------------------
//
// AIPI1 periperhals
//
#define CSP_BASE_REG_PA_AIPI1           (0x10000000)
#define CSP_BASE_REG_PA_DMAC            (0x10001000)
#define CSP_BASE_REG_PA_WDOG            (0x10002000)
#define CSP_BASE_REG_PA_GPT1            (0x10003000)
#define CSP_BASE_REG_PA_GPT2            (0x10004000)
#define CSP_BASE_REG_PA_GPT3            (0x10005000)
#define CSP_BASE_REG_PA_PWM             (0x10006000)
#define CSP_BASE_REG_PA_RTC             (0x10007000)
#define CSP_BASE_REG_PA_KPP             (0x10008000)
#define CSP_BASE_REG_PA_OWIRE           (0x10009000)
#define CSP_BASE_REG_PA_UART1           (0x1000A000)
#define CSP_BASE_REG_PA_UART2           (0x1000B000)
#define CSP_BASE_REG_PA_UART3           (0x1000C000)
#define CSP_BASE_REG_PA_UART4           (0x1000D000)
#define CSP_BASE_REG_PA_CSPI1           (0x1000E000)
#define CSP_BASE_REG_PA_CSPI2           (0x1000F000)
#define CSP_BASE_REG_PA_SSI1            (0x10010000)
#define CSP_BASE_REG_PA_SSI2            (0x10011000)
#define CSP_BASE_REG_PA_I2C1            (0x10012000)
#define CSP_BASE_REG_PA_SDHC1           (0x10013000)
#define CSP_BASE_REG_PA_SDHC2           (0x10014000)
#define CSP_BASE_REG_PA_GPIO            (0x10015000)
#define CSP_BASE_REG_PA_AUDMUX          (0x10016000)
#define CSP_BASE_REG_PA_CSPI3           (0x10017000)
#define CSP_BASE_REG_PA_MSHC            (0x10018000)
#define CSP_BASE_REG_PA_GPT4            (0x10019000)
#define CSP_BASE_REG_PA_GPT5            (0x1001A000)
#define CSP_BASE_REG_PA_UART5           (0x1001B000)
#define CSP_BASE_REG_PA_UART6           (0x1001C000)
#define CSP_BASE_REG_PA_I2C2            (0x1001D000)
#define CSP_BASE_REG_PA_SDHC3           (0x1001E000)
#define CSP_BASE_REG_PA_GPT6            (0x1001F000)

//
// AIPI2 Peripherals
//
#define CSP_BASE_REG_PA_AIPI2           (0x10020000)
#define CSP_BASE_REG_PA_LCDC            (0x10021000)
#define CSP_BASE_REG_PA_SLCDC           (0x10022000)
#define CSP_BASE_REG_PA_VCM             (0x10023000)
#define CSP_BASE_REG_PA_USB             (0x10024000)
#define CSP_BASE_REG_PA_SAHARA          (0x10025000)
#define CSP_BASE_REG_PA_EMMA            (0x10026000)
#define CSP_BASE_REG_PA_CRM             (0x10027000)
#define CSP_BASE_REG_PA_SYSCTRL         (0x10027800)    // Share slot 7 with CRM
#define CSP_BASE_REG_PA_IIM             (0x10028000)
// Reserved (Slot 9)
#define CSP_BASE_REG_PA_RTIC            (0x1002A000)
#define CSP_BASE_REG_PA_FEC             (0x1002B000)
#define CSP_BASE_REG_PA_SCC             (0x1002C000)    // Two slots
// Reserved (Slots 14 - 26)
#define CSP_BASE_REG_PA_ETBREG          (0x1003B000)
#define CSP_BASE_REG_PA_ETBRAM          (0x1003C000)    // Two slots
#define CSP_BASE_REG_PA_JAM             (0x1003E000)
#define CSP_BASE_REG_PA_MAX             (0x1003F000)

//
// Non-AIPI Peripherals
//
#define CSP_BASE_REG_PA_AITC            (0x10040000)
#define CSP_BASE_REG_PA_ROMPATCH        (0x10041000)
#define CSP_BASE_REG_PA_CSI             (0x80000000)
#define CSP_BASE_REG_PA_ATA             (0x80001000)
#define CSP_BASE_REG_PA_NANDFC          (0xD8000000)
#define CSP_BASE_REG_PA_ESDRAMC         (0xD8001000)
#define CSP_BASE_REG_PA_WEIM            (0xD8002000)
#define CSP_BASE_REG_PA_M3IF            (0xD8003000)
#define CSP_BASE_REG_PA_PCMCIA          (0xD8004000)

#if __cplusplus
}
#endif

#endif // __MX27_BASE_REGS_H__
    
