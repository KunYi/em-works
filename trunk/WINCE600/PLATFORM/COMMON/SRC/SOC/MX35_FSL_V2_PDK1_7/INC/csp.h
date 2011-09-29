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
//  Copyright (C) 2004-2008 Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header: csp.h
//
//  This header file defines the MX35 processor.
//
//  The MX35 is a System on Chip (SoC) part consisting of an ARM11 core.
//  This header file is comprised of component header files that define the
//  register layout of each component.
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
#include "mx35_base_regs.h"
#include "mx35_base_mem.h"

// IRQ Definitions
#include "mx35_irq.h"

// DMA Definitions
#include "mx35_dma.h"

// IOCTL Definitions
#include "common_ioctl.h"

// SoC Components
#include "common_avic.h"
#include "common_cspi.h"
#include "common_epit.h"
#include "common_gpio.h"
#include "common_gpt.h"
#include "common_i2c.h"
#include "common_rngc.h"
#include "common_rtc.h"
#include "common_sdmav2.h"
#include "common_ssi.h"
#include "common_uart.h"
#include "common_wdog.h"
#include "common_ipu.h"
#include "common_ata.h"
#include "common_spdif.h"
#include "common_asrc.h"
#include "common_esai.h"
#include "common_esdhc.h"

#include "mx35_ccm.h"
#include "mx35_iomux.h"
#include "mx35_nandfc.h"
#include "mx35_usb.h"
#include "mx35_ata.h"
#include "mx35_mlb.h"

// DDK Definitions
#include "common_ddk.h"
#include "mx35_ddk.h"


//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif //__CSP_H
