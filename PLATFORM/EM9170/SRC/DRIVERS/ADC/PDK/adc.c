//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
// adc.c
//
// Board-level layer for ADC driver. 
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <ceddk.h>
#pragma warning(pop)

#include "bsp.h"

//-----------------------------------------------------------------------------
//
// Function:  BSPADCCIomuxConfig
//
// This function configure the IO used by the ADC controller
//
// Parameters:
//   none
//
// Returns:
//      TRUE indicates success, FALSE indicates failure
//
//-----------------------------------------------------------------------------
BOOL BSPADCCIomuxConfig()
{
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function:  BSPADCEnableClock
//
// This function enable or disable the clocks required by the ADC controller
//
// Parameters:
//      fEnable : 
//          * TRUE turn the clocks on
//          * FALSE turn them off
//
// Returns:
//      TRUE indicates success, FALSE indicates failure
//
//-----------------------------------------------------------------------------

BOOL BSPADCEnableClock(BOOL fEnable)
{
    BOOL fResult;
    
    if (fEnable)
    {
        fResult = DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX_TCHSCRN, 
            DDK_CLOCK_GATE_MODE_ENABLED);
    }
    else
    {
        fResult = DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX_TCHSCRN, 
            DDK_CLOCK_GATE_MODE_DISABLED);
    }
    return fResult;
}

//-----------------------------------------------------------------------------
//
// Function: BSPADCGetIPGClock
//
// Retrieves the clock frequency in Hz used in ADC driver.
//
// Parameters:
//      None.
//
// Returns:
//      The IPG clock frequence.
//
//-----------------------------------------------------------------------------
UINT32 BSPADCGetIPGClock()
{
    UINT32 freq;
    DDKClockGetFreq(DDK_CLOCK_SIGNAL_IPG, &freq);
    return freq;
}

//-----------------------------------------------------------------------------
//
// Function:  BSPADCGetHSYNCPolarity
//
// This function returns the HSYNC polarity that the ADC should use its filter
//
// Parameters:
//      none
//
// Returns:
//      the HSYNC polarity
//
//-----------------------------------------------------------------------------
DWORD BSPADCGetHSYNCPolarity()
{
    return ADC_TGCR_HSYNC_POLARITY_HIGH;
}

//-----------------------------------------------------------------------------
//
// Function:  BSPADCGetHSYNCEnable
//
// This function returns the HSYNC enable flag that the ADC should use
// Sometimes it may be better not to use the HSYNC filter. For example when
// the LCDC clock is stopped. In our case let's consider it's better to use it.
//
// Parameters:
//      none
//
// Returns:
//      the HSYNC enable state
//
//-----------------------------------------------------------------------------
DWORD BSPADCGetHSYNCEnable()
{
    return ADC_TGCR_HSYNC_FILTER_ON;
}

