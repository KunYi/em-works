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
//  Header: mx233_base_mem.h
//
//  This header file defines the Physical Addresses (PA) of
//  the SoC chip selects and external memory assignments.
//
//  Provides memory map definitions specific to the MX233 SoC.
//
//------------------------------------------------------------------------------
#ifndef __MX233_BASE_MEM_H
#define __MX233_BASE_MEM_H

#if __cplusplus
extern "C" {
#endif

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
// MX233-SPECIFIC INTERNAL MEMORY MAP
//-----------------------------------------------------------------------------
#define CSP_BASE_MEM_PA_SDRAM                   (0x40000000)
#define CSP_BASE_MEM_PA_IRAM                    (0x00000000)

//-----------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif

