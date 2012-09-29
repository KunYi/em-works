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
//  This header file defines the MX28 processor.
//
//  The MX28 is a System on Chip (SoC) part consisting of an ARM9 core.
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
#include "mx28_base_mem.h"
#include "mx28_base_regs.h"

// IRQ Definitions
#include "mx28_irq.h"

// DMA Definitions

// IOCTL Definitions


// SoC Components
#include "common_uartdbg.h"

#include "regsssp.h"
#include "regsusbphy.h"
#include "regsapbh.h"
#include "regsapbx.h"
#include "regspinctrl.h"
#include "regsicoll.h"
#include "regstimrot.h"
#include "regsclkctrl.h"
#include "regsdram.h"
#include "regsusbphy.h"
#include "regsdigctl.h"
#include "regspower.h"
#include "mx28_regslcdif.h"
#include "regspwm.h"
#include "regsi2c.h"
#include "regsgpmi.h"
#include "regsocotp.h"
#include "regsbch.h"
#include "regslradc.h"
#include "regsrtc.h"
#include "regssaif.h"
#include "mx28_usb.h"
#include "mx28_rtc.h"
#include "mx28_ssp.h"

// DDK Definitions
#include "mx28_ddk.h"
#include "common_lcdif.h"
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// PMU Configuration Settings
//------------------------------------------------------------------------------
#define VDDDVolt2Reg(x)  (x - 800)/25
#define VDDIOVolt2Reg(x) (x - 2800)/50
#define VDDAVolt2Reg(x)  (x - 1500)/25

#if __cplusplus
}
#endif

#endif //__CSP_H
