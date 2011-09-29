//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  mx25esdhc.cpp
//
//  Provides MX25-specific configuration routines for
//  the ESDHC base driver.
//
//-----------------------------------------------------------------------------

#include "esdhc.h"
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
// Function:  CspESDHCGetBaseAddr
//
// This function returns the base address for the ESDHC module.
//
// Parameters:
//      dwIndex
//          [in] ESDHC index.
//
// Returns:
//      ESDHC 1 or 2 Base address.
//
//-----------------------------------------------------------------------------
DWORD CESDHCBase::CspESDHCGetBaseAddr(DWORD dwIndex)
{
    DWORD dwBaseAddr = 0;
    
    switch(dwIndex)
    {

        case 1:
            dwBaseAddr = CSP_BASE_REG_PA_ESDHC1;
            break;
            
        case 2:
            dwBaseAddr = CSP_BASE_REG_PA_ESDHC2;
            break;
            
        default:
            break;
            
    }
    return dwBaseAddr;
}

//-----------------------------------------------------------------------------
//
// Function:  CspESDHCGetIRQ
//
// This function returns the IRQ number for the ESDHC Interrupt.
//
// Parameters:
//      dwIndex
//          [in] ESDHC index.
//
// Returns:
//      ESDHC 1 or 2 IRQ number.
//
//-----------------------------------------------------------------------------
DWORD CESDHCBase::CspESDHCGetIRQ(DWORD dwIndex)
{
    DWORD dwSysIntr = (DWORD) -1;
    
    switch(dwIndex)
    {

        case 1:
            dwSysIntr = IRQ_ESDHC1;
            break;
            
        case 2:
            dwSysIntr = IRQ_ESDHC2;
            break;
                       
        default:
            break;
            
    }
    return dwSysIntr;
}
