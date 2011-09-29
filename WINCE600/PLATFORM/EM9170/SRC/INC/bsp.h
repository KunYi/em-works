//------------------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
//  Use of this source code is subject to the terms of the Microsoft end-user
//  license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
//  If you did not accept the terms of the EULA, you are not authorized to use
//  this source code. For a copy of the EULA, please see the LICENSE.RTF on your
//  install media.
//
//-----------------------------------------------------------------------------
//
// Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File: bsp.h
//
//  This header file is comprised of component header files that defines
//  the standard include hierarchy for the board support packege.
//
//------------------------------------------------------------------------------

#ifndef __BSP_H
#define __BSP_H

//------------------------------------------------------------------------------

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <ceddk.h>
#include <nkintr.h>
#include "oal.h"
#pragma warning(pop)


// Processor Definitions
#include "csp.h"

// Custom OAL Definitions
#include "oal_iomux.h"

// Configuration Files
#include "args.h"
#include "bsp_clocks.h"
#include "bsp_cfg.h"
#include "bsp_base_reg_cfg.h"
#include "ioctl_cfg.h"
#include "image_cfg.h"

// Board Level Definitions
#include "cpld.h"
#include "em9170_cpld.h"			// CS&ZHL JUN-11-2011: for CPLD and ISA in EM9170
#include "bsp_drivers.h"			// CS&ZHL JUN-24-2011: support EM9170 based stream drivers

//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif
