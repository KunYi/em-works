//------------------------------------------------------------------------------
//
//  Copyright (C) 2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  soc_tve.c
//
//  Provides SOC driver implementation for TV encoder chip.
//

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
// Function:  CSPTVEGetBaseAddr
//
// This function returns the base address for the TVE module.
//
// Parameters:
//      None.
//
// Returns:
//      TVE Base address.
//
//-----------------------------------------------------------------------------
DWORD CSPTVEGetBaseAddr()
{
    return CSP_BASE_REG_PA_TVE;
}

//-----------------------------------------------------------------------------
//
// Function:  CSPTVEGetIRQ
//
// This function returns the IRQ number for the TVE Interrupt.
//
// Parameters:
//      None.
//
// Returns:
//      TVE IRQ number.
//
//-----------------------------------------------------------------------------
DWORD CSPTVEGetIRQ(void)
{
    return IRQ_TVE;
}


//-----------------------------------------------------------------------------
//
// Function:  CSPTVESetClockGatingMode
//
// This function sets the TVE clock gating mode.
//
// Parameters:
//      
//    bTVEClockEnable
//        [in] a bool value if TVE clock is enabled/disabled
//
// Returns:
//    TRUE if sucessful, otherwise return FALSE.   
//
//-----------------------------------------------------------------------------
BOOL CSPTVESetClockGatingMode(BOOL bTVEClockEnable)
{
    if (bTVEClockEnable)
        return DDKClockSetGatingMode((DDK_CLOCK_GATE_INDEX)DDK_CLOCK_GATE_INDEX_TVE, DDK_CLOCK_GATE_MODE_ENABLED_ALL);
    else
        return DDKClockSetGatingMode((DDK_CLOCK_GATE_INDEX)DDK_CLOCK_GATE_INDEX_TVE, DDK_CLOCK_GATE_MODE_DISABLED);  
}


//-----------------------------------------------------------------------------
//
// Function:  CSPTVESetClockConfigBaud
//
// This function sets the TVE clock configuration baud.
//
// Parameters:
//      
//    clock_divide_ratio
//        [in] clock divide ratio.
//
// Returns:
//    TRUE if sucessful, otherwise return FALSE.   
//
//-----------------------------------------------------------------------------
BOOL  CSPTVESetClockConfigBaud(DWORD clock_divide_ratio)
{
     return DDKClockConfigBaud(DDK_CLOCK_SIGNAL_TVE_216_54, DDK_CLOCK_BAUD_SOURCE_PLL3,
                               clock_divide_ratio, 0); // set 0 to postdiv for TVE
}


