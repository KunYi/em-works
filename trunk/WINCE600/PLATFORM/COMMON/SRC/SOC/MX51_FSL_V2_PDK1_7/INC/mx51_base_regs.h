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
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header: mx51_base_reg.h
//
//  This header file defines the Physical Addresses (PA) of
//  the base registers for the System on Chip (SoC) components.
//
//-----------------------------------------------------------------------------

#ifndef __MX51_BASE_REG_H
#define __MX51_BASE_REG_H

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
// PERIPHERAL REGISTER MEMORY MAP
//-----------------------------------------------------------------------------

// On-chip AHB peripherals
#define CSP_BASE_REG_PA_GPU                     (0x30000000)
#define CSP_BASE_REG_PA_IPU                     (0x40000000)
#define CSP_BASE_REG_PA_TZIC                    (0x8FFFC000)
#define CSP_BASE_REG_PA_GPU2D                   (0xD0000000)
#define CSP_BASE_REG_PA_TZIC_TO2                (0xE0000000)


// AIPS #1 SPBA peripherals
#define CSP_BASE_REG_PA_ESDHC1                  (0x70004000)
#define CSP_BASE_REG_PA_ESDHC2                  (0x70008000)
#define CSP_BASE_REG_PA_UART3                   (0x7000C000)
#define CSP_BASE_REG_PA_ECSPI1                  (0x70010000)
#define CSP_BASE_REG_PA_SSI2                    (0x70014000)
#define CSP_BASE_REG_PA_SDMA_INTERNAL           (0x7001C000)
#define CSP_BASE_REG_PA_ESDHC3                  (0x70020000)
#define CSP_BASE_REG_PA_ESDHC4                  (0x70024000)
#define CSP_BASE_REG_PA_SPDIF                   (0x70028000)
#define CSP_BASE_REG_PA_PATA_UDMA               (0x70030000)
#define CSP_BASE_REG_PA_SLIMBUS                 (0x70034000)
#define CSP_BASE_REG_PA_HSI2C                   (0x70038000)
#define CSP_BASE_REG_PA_SPBA                    (0x7003C000)

// AIPS #1 on-platform peripherals
#define CSP_BASE_REG_PA_AIPS1                   (0x73F00000)

// AIPS #1 off-platform peripherals
#define CSP_BASE_REG_PA_USB                     (0x73F80000)
#define CSP_BASE_REG_PA_GPIO1                   (0x73F84000)
#define CSP_BASE_REG_PA_GPIO2                   (0x73F88000)
#define CSP_BASE_REG_PA_GPIO3                   (0x73F8C000)
#define CSP_BASE_REG_PA_GPIO4                   (0x73F90000)
#define CSP_BASE_REG_PA_KPP                     (0x73F94000)
#define CSP_BASE_REG_PA_WDOG1                   (0x73F98000)
#define CSP_BASE_REG_PA_WDOG2                   (0x73F9C000)
#define CSP_BASE_REG_PA_GPT                     (0x73FA0000)
#define CSP_BASE_REG_PA_SRTC                    (0x73FA4000)
#define CSP_BASE_REG_PA_IOMUXC                  (0x73FA8000)
#define CSP_BASE_REG_PA_EPIT1                   (0x73FAC000)
#define CSP_BASE_REG_PA_EPIT2                   (0x73FB0000)
#define CSP_BASE_REG_PA_PWM1                    (0x73FB4000)
#define CSP_BASE_REG_PA_PWM2                    (0x73FB8000)
#define CSP_BASE_REG_PA_UART1                   (0x73FBC000)
#define CSP_BASE_REG_PA_UART2                   (0x73FC0000)
#define CSP_BASE_REG_PA_USB_PL301               (0x73FC4000)
#define CSP_BASE_REG_PA_SRC                     (0x73FD0000)
#define CSP_BASE_REG_PA_CCM                     (0x73FD4000)
#define CSP_BASE_REG_PA_GPC                     (0x73FD8000)


// AIPS #2 on-platform peripherals
#define CSP_BASE_REG_PA_AIPS2                   (0x83F00000)

// AIPS #2 off-platform peripherals
#define CSP_BASE_REG_PA_DPLL1                   (0x83F80000)
#define CSP_BASE_REG_PA_DPLL2                   (0x83F84000)
#define CSP_BASE_REG_PA_DPLL3                   (0x83F88000)
#define CSP_BASE_REG_PA_AHBMAX                  (0x83F94000)
#define CSP_BASE_REG_PA_IIM                     (0x83F98000)
#define CSP_BASE_REG_PA_CSU                     (0x83F9C000)
#define CSP_BASE_REG_PA_ELBOW                   (0x83FA0000)
#define CSP_BASE_REG_PA_OWIRE                   (0x83FA4000)
#define CSP_BASE_REG_PA_FIRI                    (0x83FA8000)
#define CSP_BASE_REG_PA_ECSPI2                  (0x83FAC000)
#define CSP_BASE_REG_PA_SDMA                    (0x83FB0000)
#define CSP_BASE_REG_PA_SCC                     (0x83FB4000)
#define CSP_BASE_REG_PA_ROMCP                   (0x83FB8000)
#define CSP_BASE_REG_PA_RTIC                    (0x83FBC000)
#define CSP_BASE_REG_PA_CSPI                    (0x83FC0000)
#define CSP_BASE_REG_PA_I2C2                    (0x83FC4000)
#define CSP_BASE_REG_PA_I2C1                    (0x83FC8000)
#define CSP_BASE_REG_PA_SSI1                    (0x83FCC000)
#define CSP_BASE_REG_PA_AUDMUX                  (0x83FD0000)
#define CSP_BASE_REG_PA_M4IF                    (0x83FD8000)
#define CSP_BASE_REG_PA_ESDCTL                  (0x83FD9000)
#define CSP_BASE_REG_PA_WEIM                    (0x83FDA000)
#define CSP_BASE_REG_PA_NFC_AXI                 (0xCFFF0000)
#define CSP_BASE_REG_PA_NFC_IP                  (0x83FDB000)
#define CSP_BASE_REG_PA_EMI                     (0x83FDBF00)
#define CSP_BASE_REG_PA_MIPI                    (0x83FDC000)
#define CSP_BASE_REG_PA_PATA_PIO                (0x83FE0000)
#define CSP_BASE_REG_PA_SIM                     (0x83FE4000)
#define CSP_BASE_REG_PA_SSI3                    (0x83FE8000)
#define CSP_BASE_REG_PA_FEC                     (0x83FEC000)
#define CSP_BASE_REG_PA_TVE                     (0x83FF0000)
#define CSP_BASE_REG_PA_VPU                     (0x83FF4000)
#define CSP_BASE_REG_PA_SAHARA                  (0x83FF8000)


#if __cplusplus
}
#endif

#endif


