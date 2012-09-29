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
// Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  bsp_base_reg_cfg.h
//
//  This header file defines location for BSP on-board devices. It usually
//  should contain only physical addresses. Virtual addresses should be obtain
//  via OALPAtoVA function call.
//
//------------------------------------------------------------------------------
#ifndef __BSP_BASE_REG_CFG_H
#define __BSP_BASE_REG_CFG_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  NAMING CONVENTIONS
//
//  BSP_BASE_REG_ is the standard prefix for BSP device base registers.
//
//  Memory ranges are accessed using physical, uncached, or cached addresses,
//  depending on the system state. The following abbreviations are used for
//  each addressing type:
//
//      PA - physical address
//      CA - cached virtual address
//      UA - uncached virtual address
//
//  The naming convention for base registers is:
//
//      xxx_BASE_REG_<ADDRTYPE>_<SUBSYSTEM>
//
//------------------------------------------------------------------------------

    
//------------------------------------------------------------------------------
//
//  Define:  BSP_BASE_REG_PA_PBC_BASE
//
//  Locates the PBC CPLD (PBC) module. WEIM CS4 is used
//  to connect this device. Note that there must exist memory mapping in
//  oemaddrtab_cfg.h for this memory area.
//
// Note :  we keep that here even if the CPLD is on CSPI bus because we need 
//         a valid logical address for the LAN911x
//
#define BSP_BASE_REG_PA_PBC_BASE            CSP_BASE_MEM_PA_CS4


//------------------------------------------------------------------------------
//
//  Define:  BSP_BASE_REG_PA_LAN911x
//
#define BSP_BASE_REG_PA_LAN911x_IOBASE      (BSP_BASE_REG_PA_PBC_BASE + CPLD_LAN9217_OFFSET)

#if __cplusplus
}
#endif

#endif
