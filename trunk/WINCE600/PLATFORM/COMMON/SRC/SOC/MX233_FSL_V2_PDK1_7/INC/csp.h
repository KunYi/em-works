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
//  Copyright (C) 2007-2009 Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header: csp.h
//
//  This header file defines the MX233 processor.
//
//  The MX233 is a System on Chip (SoC) part consisting of an ARM9 core.
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

// Base Definitions
#include "mx233_base_mem.h"
#include "mx233_base_regs.h"
#include "soc_macros.h"
#include "soc_types.h"
#include "soc_display.h"

// IRQ Definitions
#include "mx233_irq.h"

// DMA Definitions
#include "hw_dma.h"

// IOCTL Definitions


// SoC Components
#include "regsusbphy.h"
#include "regsuartdbg.h"
#include "regsapbh.h"
#include "regsapbx.h"
#include "regspinctrl.h"
#include "regsicoll.h"
#include "regstimrot.h"
#include "mx233_rtc.h"
#include "regsclkctrl.h"
#include "regsdram.h"
#include "mx233_usb.h"
#include "regsusbphy.h"
#include "regsdigctl.h"
#include "regspower.h"
#include "regsaudioout.h"
#include "regsaudioin.h"
#include "regslcdif.h"
#include "regspwm.h"
#include "regsssp.h"
#include "regsi2c.h"
#include "regstvenc.h"
#include "regsgpmi.h"
#include "regsemi.h"
#include "regspxp.h"
#include "mx233_ssp.h"
#include "regsspdif.h"
#include "regsocotp.h"
#include "regsbch.h"
#include "regslradc.h"
#include "regsdcp.h"

// DDK Definitions
#include "mx233_ddk.h"
#include "power.h"
#include "LCDIF.h"
//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif //__CSP_H
