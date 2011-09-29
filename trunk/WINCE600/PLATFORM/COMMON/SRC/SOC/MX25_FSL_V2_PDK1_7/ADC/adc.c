//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
// adc.c
//
// SOC layer for ADC driver. 
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <ceddk.h>
#pragma warning(pop)

#include "csp.h"

//-----------------------------------------------------------------------------
//
// Function:  CspADCTouchGetBaseRegAddr
//
// This function returns the physical base address for the
// Touchscreen registers
//
// Parameters:
//      none.
//
// Returns:
//      Physical base address for the Touchscreen.
//
//-----------------------------------------------------------------------------
DWORD CspADCTouchGetBaseRegAddr()
{
    return CSP_BASE_REG_PA_TCHSC;
}
