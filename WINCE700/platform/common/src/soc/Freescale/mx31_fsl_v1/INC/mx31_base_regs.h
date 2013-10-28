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
//  Header: MX31_base_reg.h
//
//  This header file defines the Physical Addresses (PA) of
//  the base registers for the System on Chip (SoC) components.
//
//  Provides memory map definitions that are specific to the MX31
//  architecture.
//
//-----------------------------------------------------------------------------

#ifndef __MX31_BASE_REG_H
#define __MX31_BASE_REG_H

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
// MX31-SPECIFIC PERIPHERAL REGISTER MEMORY MAP
//-----------------------------------------------------------------------------

// AIPS A periperhals
#define CSP_BASE_REG_PA_I2C3                    (0x43F84000)
#define CSP_BASE_REG_PA_USBOTG                  (0x43F88000)
#define CSP_BASE_REG_PA_ATA_CTRL                (0x43F8C000)
#define CSP_BASE_REG_PA_I2C2                    (0x43F98000)
#define CSP_BASE_REG_PA_IOMUXC                  (0x43FAC000)
#define CSP_BASE_REG_PA_UART4                   (0x43FB0000)
#define CSP_BASE_REG_PA_UART5                   (0x43FB4000)
#define CSP_BASE_REG_PA_ECT_IPBUS1              (0x43FB8000)
#define CSP_BASE_REG_PA_ECT_IPBUS2              (0x43FBC000)

// AIPS B Peripherals
#define CSP_BASE_REG_PA_SIM                     (0x50018000)
#define CSP_BASE_REG_PA_ATA_DMA                 (0x50020000)
#define CSP_BASE_REG_PA_MSHC1                   (0x50024000)
#define CSP_BASE_REG_PA_MSHC2                   (0x50028000)
#define CSP_BASE_REG_PA_CCM                     (0x53F80000)
#define CSP_BASE_REG_PA_CSPI3                   (0x53F84000)
#define CSP_BASE_REG_PA_GPIO3                   (0x53FA4000)
#define CSP_BASE_REG_PA_MPEG4_ENCODER           (0x53FC8000)
#define CSP_BASE_REG_PA_GPIO2                   (0x53FD0000)
#define CSP_BASE_REG_PA_RTIC                    (0x53FEC000)

// Non-AIPS Peripherals
#define CSP_BASE_REG_PA_PCMCIA                  (0xB8004000)


#if __cplusplus
}
#endif

#endif


