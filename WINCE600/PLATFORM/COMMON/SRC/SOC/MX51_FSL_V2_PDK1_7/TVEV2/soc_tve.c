//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  soc_tve.c
//
//  Provides SOC driver implementation for TV encoder V2 chip.
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
#define  PLL3_HDTV_297MHZ     297000000
#define  PLL3_SDTV_216MHZ     216000000
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
// Function:  CSPTVESetClock
//
// This function sets the TVE clock configuration baud.
//
// Parameters:
//      
//    bHD
//        [in] TRUE for HD, FALSE for SD.
//
// Returns:
//    TRUE if sucessful, otherwise return FALSE.   
//
//-----------------------------------------------------------------------------
BOOL  CSPTVESetClock(BOOL bHD)
{
    if (bHD)
    {
        // Set PLL3 to 297MHz for HDTV
        return DDKClockSetFreq(DDK_CLOCK_SIGNAL_PLL3, PLL3_HDTV_297MHZ);
    }
    else
    {
        // Set PLL3 to 216MHz for SDTV
        return DDKClockSetFreq(DDK_CLOCK_SIGNAL_PLL3,  PLL3_SDTV_216MHZ);
    }
}


