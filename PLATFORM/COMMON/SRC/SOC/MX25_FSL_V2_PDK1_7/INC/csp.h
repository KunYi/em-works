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
//  This header file defines the MX25 processor.
//
//  The MX25 is a System on Chip (SoC) part consisting of an ARM9 core.
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
#include "mx25_base_regs.h"
#include "mx25_base_mem.h"

// IRQ Definitions
#include "mx25_irq.h"

// DMA Definitions
#include "mx25_dma.h"

// IOCTL Definitions
#include "common_ioctl.h"
#include "mx25_ioctl.h"

// SoC Components
#include "common_avic.h"
#include "common_epit.h"
#include "common_gpio.h"
#include "common_gpt.h"
#include "common_i2c.h"
#include "common_cspiv2.h"
#include "common_adc.h"
#include "common_sdmav2.h"
#include "common_ssi.h"
#include "common_esai.h"
#include "common_uart.h"
#include "common_lcdc.h"
#include "common_sim.h"
#include "common_ata.h"
#include "common_spdif.h"
#include "common_srtc.h"
#include "common_esdhc.h"

#include "mx25_weim.h"				// CS&ZHL JUN-10-2011: supporting access CPLD in EM9170
#include "mx25_crm.h"
#include "mx25_iomux.h"
#include "mx25_wdog.h"
#include "mx25_kpp.h"
#include "mx25_nandfc.h"
#include "mx25_csi.h"
#include "mx25_fec.h"
#include "mx25_pwm.h"

#include "mx25_usb.h"

#include "mx25_dryice.h"
#include "mx25_rngb.h"

// DDK Definitions
#include "common_ddk.h"
#include "mx25_ddk.h"

//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif //__CSP_H
