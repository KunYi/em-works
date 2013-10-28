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
//  Header: csp.h
//
//  This header file defines the MX31 processor.
//
//  The MX31 is a System on Chip (SoC) part consisting of an ARM11 core.
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

// Platform Definitions
#include "mxarm11.h"

// Base Definitions
#include "MX31_base_regs.h"
#include "MX31_base_mem.h"

// IRQ Definitions
#include "MX31_irq.h"

// DMA Definitions
#include "MX31_dma.h"

// SoC Components
#include "MX31_ccm.h"
#include "MX31_iomux.h"
#include "MX31_ata.h"
#include "MX31_usb.h"
#include "MX31_nandfc.h"

// DDK Definitions
#include "MX31_ddk.h"

//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif //__CSP_H
