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
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  soc_timer.c
//
//  This file contains SoC-specific routines to support the OAL timer.
//
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#pragma warning(pop)

#include "csp.h"


//-----------------------------------------------------------------------------
//
//  Function: OALTimerGetIrq
//
//  This function returns to SoC-specific IRQ for the OAL hardware timer.
//
//  Parameters:
//      None.
//
//  Returns:
//      IRQ for OAL hardware timer.
//
//-----------------------------------------------------------------------------
UINT32 OALTimerGetIrq(void)
{
    return IRQ_EPIT1;
}


//-----------------------------------------------------------------------------
//
//  Function: OALTimerGetBaseRegAddr
//
//  This function returns to SoC-specific register base address for the 
//  OAL hardware timer.
//
//  Parameters:
//      None.
//
//  Returns:
//      Register base address for OAL hardware timer.
//
//-----------------------------------------------------------------------------
UINT32 OALTimerGetBaseRegAddr(void)
{
    return CSP_BASE_REG_PA_EPIT1;
}
