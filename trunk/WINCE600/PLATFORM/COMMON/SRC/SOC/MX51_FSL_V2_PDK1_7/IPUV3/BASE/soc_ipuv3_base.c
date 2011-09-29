//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
//  File:  soc_ipuv3_base.c
//
//  Provides SoC-specific configuration routines for
//  the IPUV3 base driver.
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
// Function:  CSPIPUGetBaseAddr
//
// This function returns the base address for the IPUV3 module.
//
// Parameters:
//      None.
//
// Returns:
//      IPU Base address.
//
//-----------------------------------------------------------------------------
DWORD CSPIPUGetBaseAddr()
{
    return CSP_BASE_REG_PA_IPU;
}

//-----------------------------------------------------------------------------
//
// Function:  IPUGetIRQ
//
// This function returns the IRQ number for the IPU Interrupt.
//
// Parameters:
//      None.
//
// Returns:
//      IPU IRQ number.
//
//-----------------------------------------------------------------------------
DWORD IPUGetIRQ(void)
{
    return IRQ_IPU_SYNC;
}
