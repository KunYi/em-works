//-----------------------------------------------------------------------------
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
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

#include <windows.h>
#include <nkintr.h>
#include <types.h>
#include <pm.h>
#include <ddkreg.h>
#include <Winreg.h>
#include <ceddk.h>
#include <ddkmacro.h>

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)

#include <winddi.h>
#include <emul.h>
#include <Pwingdi.h>
#include <ddrawi.h>
#include <ddgpe.h>

#if !(UNDER_CE >= 600)
#include <ddpguids.h>
#endif
#include <ddhfuncs.h>
#pragma warning(pop)

#include "common_lcdc.h"
#include "common_macros.h"
#include "lcdc_mode.h"
#include "display_vf.h"

#include "DDLcdc.h"
#include "ddgpeuser.h"
