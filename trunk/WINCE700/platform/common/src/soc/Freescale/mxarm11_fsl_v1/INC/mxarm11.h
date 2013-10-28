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
//  Header: mxarm11.h
//
//  This header file defines the generic MXARM11 processor.
//
//  This header file is comprised of component header files that define the
//  register layout of each component.
//
//------------------------------------------------------------------------------
#ifndef __MXARM11_H
#define __MXARM11_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------


// Types and Macros
#include "socarm_types.h"
#include "socarm_macros.h"

// Base Definitions
#include "mxarm11_base_regs.h"
#include "mxarm11_base_mem.h"

// IRQ Definitions
#include "mxarm11_irq.h"

// IOCTL Definitions
#include "mxarm11_ioctl.h"

// IOMUX Definitions
#include "mxarm11_iomux.h"

// SoC Components
#include "mxarm11_avic.h"
#include "mxarm11_uart.h"
#include "mxarm11_epit.h"
#include "mxarm11_rtc.h"
#include "mxarm11_gpio.h"
#include "mxarm11_cspi.h"
#include "mxarm11_ipu.h"
#include "mxarm11_nandfc.h"
#include "mxarm11_gpt.h"
#include "mxarm11_kpp.h"
#include "mxarm11_wdog.h"
#include "mxarm11_sdma.h"
#include "mxarm11_pwm.h"
#include "mxarm11_i2c.h"
#include "mxarm11_firi.h"
#include "mxarm11_audmux.h"
#include "mxarm11_ssi.h"
#include "mxarm11_edio.h"
#include "mxarm11_mu.h"
#include "mxarm11_sdhc.h"
#include "mxarm11_rnga.h"

// DDK Definitions
#include "mxarm11_ddk.h"


//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif //__MXARM11_H
