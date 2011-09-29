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
//  Copyright (C) 2007-2008 Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header: csp.h
//
//  This header file defines the MX51 processor.
//
//-----------------------------------------------------------------------------

#ifndef __CSP_H
#define __CSP_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

// Types and Macros
#include "common_types.h"
#include "common_macros.h"

// Base Definitions
#include "mx51_base_regs.h"
#include "mx51_base_mem.h"

// IRQ Definitions
#include "mx51_irq.h"

// DMA Definitions
#include "mx51_dma.h"

// IOCTL Definitions
#include "common_ioctl.h"

// SoC Components
#include "common_ata.h"
#include "common_dpll.h"
#include "common_ecspi.h"
#include "common_esdhc.h"
#include "common_epit.h"
#include "common_gpio.h"
#include "common_gpt.h"
#include "common_i2c.h"
#include "common_hsi2c.h"
#include "common_tve.h"
#include "common_ipuv3.h"
#include "common_kpp.h"
#include "common_mshc.h"
#include "common_rngc.h"
#include "common_sdmav2.h"
#include "common_spdif.h"
#include "common_srtc.h"
#include "common_ssi.h"
#include "common_tzic.h"
#include "common_uart.h"
#include "common_wdog.h"
#include "common_firi.h"
#include "mx51_ccm.h"
#include "mx51_iomux.h"
#include "mx51_gpc.h"
#include "mx51_usb.h"
#include "mx51_ata.h"
#include "mx51_sim.h"

// DDK Definitions
#include "common_ddk.h"
#include "mx51_ddk.h"

// Select DDK definitions for specific silicon version.
// Note that BSP_SI_VER_TOx is not defined for SOC code, but
// this header will be included by PLATFORM code that does
// have access to this definition.
#ifdef BSP_SI_VER_TO1
#include "mx51_to1_ddk.h"
#include "mx51_nandfc_to1.h"
#endif
#ifdef BSP_SI_VER_TO2
#include "mx51_to2_ddk.h"
#include "mx51_nandfc_to2.h"
#endif

//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif //__CSP_H
