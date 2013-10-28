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
// Copyright (C) 2006, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
//------------------------------------------------------------------------------
//
// Header: csp.h
//
// This header file defines the MX27 processor.
//
// The MX27 is a System on Chip (SoC) part consisting of an ARM9 core.
// This header file is comprised of component header files that define the
// register layout of each component.
//
//-----------------------------------------------------------------------------

#ifndef __CSP_H__
#define __CSP_H__

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

// Types and Macros
#include "csparm_types.h"
#include "csparm_macros.h"

// Base Definitions
#include "mx27_base_regs.h"
#include "mx27_base_mem.h"

// IRQ Definitions
#include "mx27_irq.h"

// SoC Components
#include "mx27_aitc.h"
#include "mx27_ata.h"
#include "mx27_weim.h"
#include "mx27_esdramc.h"
#include "mx27_pllcrc.h"
#include "mx27_sysctrl.h"
#include "mx27_gpio.h"
#include "mx27_gpt.h"
#include "mx27_rtc.h"
#include "mx27_uart.h"
#include "mx27_wdog.h"
#include "mx27_uart.h"
#include "mx27_kpp.h"
#include "mx27_ioctl.h"
#include "mx27_owire.h"
#include "mx27_dmac.h"
#include "mx27_cspi.h"
#include "mx27_lcdc.h"
#include "mx27_slcdc.h"
#include "mx27_i2c.h"
#include "mx27_sdhc.h"
#include "mx27_nandfc.h"
#include "mx27_pwm.h"
#include "mx27_audmux.h"
#include "mx27_ssi.h"
#include "mx27_usb.h"
#include "mx27_pp.h"
#include "mx27_prp.h"
#include "mx27_fec.h"

// CSI Definitions
#include "mx27_csi.h"
	
// DDK Definitions
#include "mx27_ddk.h"

//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif //__CSP_H__
