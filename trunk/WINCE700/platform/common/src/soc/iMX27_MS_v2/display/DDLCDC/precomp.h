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
//---------------------------------------------------------------------------
//  Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//---------------------------------------------------------------------------

#include <windows.h>
#include <nkintr.h>
#include <types.h>
#include <winddi.h>
#include <emul.h>
#include <ceddk.h>
#include <ddkmacro.h>
#include <ddkreg.h>
#include <Winreg.h>
#include <Pwingdi.h>
#include <pm.h>

#include <ddrawi.h>
#include <ddgpe.h>
#if !(UNDER_CE >= 600)
#include <ddpguids.h>
#endif
#include <ddhfuncs.h>

#include "csp.h"
#include "lcdc.h"

#include "DDLcdc.h"
#include "ddgpeuser.h"
