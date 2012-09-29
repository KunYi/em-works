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
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  lcdc_base.c
//
//  Provides SoC-specific configuration routines for
//  the LCDC common driver.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <winddi.h>
#include <gpe.h>
#pragma warning(pop)

#include "common_lcdc.h"
#include "lcdc_mode.h"

#include "csp.h"


//-----------------------------------------------------------------------------
// External Functions

//-----------------------------------------------------------------------------
// External Variables


//-----------------------------------------------------------------------------
// Defines


//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables


//-----------------------------------------------------------------------------
// Local Variables


//-----------------------------------------------------------------------------
// Local Functions

//-----------------------------------------------------------------------------
//
// Function:  LCDCGetBaseRegAddr
//
// This function returns the physical base address for the
// LCDC registers.
//
// Parameters:
//      None.
//
// Returns:
//      Physical base address for LCDC registers.
//
//-----------------------------------------------------------------------------
UINT32 LCDCGetBaseRegAddr(void)
{
    return CSP_BASE_REG_PA_LCDC;
}

//-----------------------------------------------------------------------------
//
// Function:  LCDCGetIRQ
//
// This function returns the IRQ number for the LCDC.
//
// Parameters:
//      None.
//
// Returns:
//      LCDC IRQ number.
//
//-----------------------------------------------------------------------------
DWORD LCDCGetIRQ(void)
{
    return IRQ_LCDC;
}



//------------------------------------------------------------------------------
//
// Function: LCDCEnableClock
//
// This function enable or disable Lcdc clock for LCD panel mode.
//
// Parameters:
//      bEnable
//          [in] This argument is enable or disable Lcdc clock for LCD panel mode.
//
// Returns:
//      Return operation status.
//      TRUE: Success
//      FALSE: Error
//
//------------------------------------------------------------------------------
BOOL LCDCEnableClock(BOOL bEnable)
{
    BOOL    bState;
    if(bEnable)
    {
        bState = DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX_LCDC, DDK_CLOCK_GATE_MODE_ENABLED);
        if (bState == FALSE) return bState;
        // Enable LCDC
        bState = DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX_AHB_LCDC, DDK_CLOCK_GATE_MODE_ENABLED);
        if (bState == FALSE) return bState;
        bState = DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX_PER_LCDC, DDK_CLOCK_GATE_MODE_ENABLED);
        if (bState == FALSE) return bState;
    }
    else
    {
        // Disable LCDC
        bState = DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX_LCDC, DDK_CLOCK_GATE_MODE_DISABLED);
        if (bState == FALSE) return bState;
        // Enable LCDC
        bState = DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX_AHB_LCDC, DDK_CLOCK_GATE_MODE_DISABLED);
        if (bState == FALSE) return bState;
        bState = DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX_PER_LCDC, DDK_CLOCK_GATE_MODE_DISABLED);
        if (bState == FALSE) return bState;
     }
    return bState;
}



//------------------------------------------------------------------------------
//
// Function: LCDCGetRefClk
//
// This function get the LCDC reference clock. This value is MPLL value divide
// by the PCDR3 divider register.
//
// Parameters:
//      pModeDesc
//          [in] This argument contains the display mode descriptor
//
// Returns:
//      Return the lcdc reference clock
//
//------------------------------------------------------------------------------
ULONG LCDCGetRefClk(PLCDC_MODE_DESCRIPTOR pModeDesc)
{
    UINT32                freq;
    UINT32                uLCDRefClk;

    UNREFERENCED_PARAMETER(pModeDesc);

    DDKClockGetFreq(DDK_CLOCK_SIGNAL_PER_LCDC, &freq);
    uLCDRefClk = freq;

    return uLCDRefClk;
}
