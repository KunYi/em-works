//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
//  File:  soc_display.c
//
//  Provides SoC-specific configuration routines for
//  the IPUV3 display interface driver.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#pragma warning(pop)

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
// Function:  CSPIPUGetClk
//
// This function returns the clock frequency for the IPUV3 module.
//
// Parameters:
//      None.
//
// Returns:
//      IPU HSP clock frequency in Hz.
//
//-----------------------------------------------------------------------------
DWORD CSPIPUGetClk()
{
    UINT32 ipuClk;

    // Retrieve HSP clock frequency value
    DDKClockGetFreq(DDK_CLOCK_SIGNAL_IPU_HSP, &ipuClk);

    return ipuClk;
}
