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
//  File:  soc_profiler.c
//
//  This file contains SoC-specific routines to support the OAL profiler.
//
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#pragma warning(pop)

#include "csp.h"

//------------------------------------------------------------------------------
// External Functions
extern VOID OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX index, 
    DDK_CLOCK_GATE_MODE mode);


//-----------------------------------------------------------------------------
//
//  Function: OALProfileGetIrq
//
//  This function returns to SoC-specific IRQ for the profiler hardware timer.
//
//  Parameters:
//      None.
//
//  Returns:
//      IRQ for profiler hardware timer.
//
//-----------------------------------------------------------------------------
UINT32 OALProfileGetIrq(void)
{
    return IRQ_GPT;
}


//-----------------------------------------------------------------------------
//
//  Function: OALProfileGetBaseRegAddr
//
//  This function returns to SoC-specific register base address for the 
//  profiler hardware timer.
//
//  Parameters:
//      None.
//
//  Returns:
//      Register base address for profiler hardware timer.
//
//-----------------------------------------------------------------------------
UINT32 OALProfileGetBaseRegAddr(void)
{
    return CSP_BASE_REG_PA_GPT;
}


//-----------------------------------------------------------------------------
//
//  Function: OALProfileTimerEnable
//
//  This function performs platform-specific configuration to enable the
//  profile timer.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID OALProfileTimerEnable(void)
{
    // Turn on GPT clocks
    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_GPT, 
        DDK_CLOCK_GATE_MODE_ENABLED_ALL);
}


//-----------------------------------------------------------------------------
//
//  Function: OALProfileTimerDisable
//
//  This function performs platform-specific configuration to disable the
//  profile timer.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID OALProfileTimerDisable(void)
{
    // Turn off GPT clocks
    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_GPT, 
        DDK_CLOCK_GATE_MODE_DISABLED);
}



