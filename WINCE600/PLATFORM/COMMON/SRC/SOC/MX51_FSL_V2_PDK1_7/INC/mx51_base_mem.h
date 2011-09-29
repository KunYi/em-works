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
//  Header: mx51_base_mem.h
//
//  This header file defines the Physical Addresses (PA) of
//  the SoC chip selects and external memory assignments.
//
//------------------------------------------------------------------------------
#ifndef __MX51_BASE_MEM_H
#define __MX51_BASE_MEM_H

#if __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------------------------
//  INFORMATION
//
//  The physical addresses for SoC chip selects are fixed, hence they are 
//  defined  in the CPU's common directory. The virtual addresses of the SoC 
//  chip selects are defined by the OEM and are configured in the 
//  platform's configuration directory by the file: 
//  .../PLATFORM/<NAME>/SRC/CONFIG/CPU_BASE_REG_CFG.H.
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//  NAMING CONVENTIONS
//
//  CPU_BASE_MEM_ is the standard prefix for CPU chip selects.
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
//      CPU_BASE_MEM_<ADDRTYPE>_<SUBSYSTEM>
//
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// ON-CHIP MEMORY MAP
//-----------------------------------------------------------------------------
#define CSP_BASE_MEM_PA_ROM                     (0x00000000)
#define CSP_BASE_MEM_PA_IRAM                    (0x1FFE8000)
#define CSP_BASE_MEM_PA_IRAM_TO2                (0x1FFE0000)
#define CSP_BASE_MEM_PA_GMEM                    (0x20000000)

#define CSP_BASE_MEM_PA_AIPS1                   (0x70000000)
#define CSP_BASE_MEM_PA_AIPS2                   (0x80000000)


//-----------------------------------------------------------------------------
// OFF-CHIP MEMORY MAP
//-----------------------------------------------------------------------------
#define CSP_BASE_MEM_PA_CSD0                    (0x90000000)
#define CSP_BASE_MEM_PA_CSD1                    (0xA0000000)

#define CSP_BASE_MEM_PA_CS0                     (0xB0000000)
#define CSP_BASE_MEM_PA_CS1                     (0xB8000000)
#define CSP_BASE_MEM_PA_CS2                     (0xC0000000)
#define CSP_BASE_MEM_PA_CS3                     (0xC8000000)
#define CSP_BASE_MEM_PA_CS4                     (0xCC000000)
#define CSP_BASE_MEM_PA_CS5                     (0xCE000000)
#define CSP_BASE_MEM_PA_NFC                     (0xCFFF0000)


#if __cplusplus
}
#endif

#endif

