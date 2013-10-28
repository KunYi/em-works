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
//  Header: mxarm11_base_mem.h
//
//  This header file defines the Physical Addresses (PA) of
//  the SoC chip selects and external memory assignments.
//
//  Provides memory map definitions that are shared for ARM11 chassis.
//
//------------------------------------------------------------------------------
#ifndef __MXARM11_BASE_MEM_H
#define __MXARM11_BASE_MEM_H

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
// ARM11-CHASSIS INTERNAL MEMORY MAP
//-----------------------------------------------------------------------------
#define CSP_BASE_MEM_PA_AIPSA                   (0x40000000)
#define CSP_BASE_MEM_PA_AIPSB                   (0x50000000)
#define CSP_BASE_MEM_PA_MAX_S2                  (0x70000000)
#define CSP_BASE_MEM_PA_MAX_S1                  (0x80000000)
#define CSP_BASE_MEM_PA_MAX_S0                  (0xC0000000)


//-----------------------------------------------------------------------------
// ARM11-CHASSIS EXTERNAL MEMORY MAP
//-----------------------------------------------------------------------------
#define CSP_BASE_MEM_PA_CSD0                    (0x80000000)
#define CSP_BASE_MEM_PA_CSD1                    (0x90000000)

#define CSP_BASE_MEM_PA_CS0                     (0xA0000000)
#define CSP_BASE_MEM_PA_CS1                     (0xA8000000)
#define CSP_BASE_MEM_PA_CS2                     (0xB0000000)
#define CSP_BASE_MEM_PA_CS3                     (0xB2000000)
#define CSP_BASE_MEM_PA_CS4                     (0xB4000000)


#if __cplusplus
}
#endif

#endif

