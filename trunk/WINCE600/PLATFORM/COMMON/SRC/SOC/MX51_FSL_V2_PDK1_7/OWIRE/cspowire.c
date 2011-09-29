//------------------------------------------------------------------------------
//
// Copyright (C) 2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
// File:  cspowire.c
//
// Provides SoC-specific configuration routines for One-Wire.
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
// Function:  OwireGetBaseRegAddr
//
// This function returns the physical base address for the
// One-Wire registers.
//
// Parameters:
//      None
//
// Returns:
//      Physical base address for One-Wire registers.
//
//-----------------------------------------------------------------------------
UINT32 OwireGetBaseRegAddr(void)
{
    return CSP_BASE_REG_PA_OWIRE;
}


//-----------------------------------------------------------------------------
//
// Function:  OwireGetIRQ
//
// This function returns the IRQ number for One-Wire.
//
// Parameters:
//      None
//
// Returns:
//      IRQ number for One-Wire.
//
//-----------------------------------------------------------------------------
UINT32 OwireGetIRQ(void)
{
    return IRQ_OWIRE;
}
