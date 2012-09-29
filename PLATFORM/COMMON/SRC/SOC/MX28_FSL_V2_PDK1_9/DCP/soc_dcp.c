//------------------------------------------------------------------------------
//
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
//  File:  soc_dcp.c
//
//  Provides SoC-specific configuration routines for
//  the dcp.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115)
#include <windows.h>
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
// Function:  CSPDCPGetBaseAddr
//
// This function returns the base address for the DCP module.
//
// Parameters:
//      None.
//
// Returns:
//      DCP Base address.
//
//-----------------------------------------------------------------------------
DWORD CSPDCPGetBaseAddr()
{
    return CSP_BASE_REG_PA_DCP;
}

//-----------------------------------------------------------------------------
//
// Function:  CSPDCPGetIRQ
//
// This function returns the IRQ number for the DCP Interrupt.
//
// Parameters:
//      None.
//
// Returns:
//      DCP IRQ number.
//
//-----------------------------------------------------------------------------
DWORD CSPDCPGetIRQ(void)
{
    return IRQ_DCP;
}

//-----------------------------------------------------------------------------
//
// Function:  DCPNoCSCOrCSCAvailable
//
// This function checks whether DCP has CSC module or whether CSC module in DCP
// is available.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE
//      FALSE
//
//-----------------------------------------------------------------------------
BOOL DCPNoCSCOrCSCAvailable(void)
{
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function:  DCPSetCSCInterrupt
//
// This function set interrupt for CSC module in DCP.
// MX28 DCP doesn't have CSC module, hence do nothing here.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void DCPSetCSCInterrupt()
{
}

//-----------------------------------------------------------------------------
//
// Function:  DCPInitCSCSysVar
//
// This function initialize OS variable for CSC module in DCP.
// MX28 DCP doesn't have CSC module, hence do nothing here.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void DCPInitCSCSysVar()
{
}

//-----------------------------------------------------------------------------
//
// Function:  DCPLordCSCCoef
//
// This function loads CSC coefficient for DCP.
// MX28 DCP doesn't have CSC module, hence do nothing here.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void DCPLordCSCCoef()
{
}

//-----------------------------------------------------------------------------
//
// Function:  DCPCSCInterruptHandler
//
// This function handles interrupt process for CSC module in DCP.
// MX28 DCP doesn't have CSC module, hence do nothing here.
//
// Parameters:
//      None.
//
// Returns:
//      None
//
//-----------------------------------------------------------------------------
void DCPCSCInterruptHandler()
{
}