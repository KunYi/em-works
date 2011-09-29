//-----------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//-----------------------------------------------------------------------------
//
//  Copyright (C) 2004-2007,2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
//  File:  cspfir.c
//
//  Provides SoC-specific configuration routines for
//  the FIR.
//
//-----------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115)
#pragma warning(disable: 4201)
#pragma warning(disable: 4204)
#pragma warning(disable: 4214)
#include <windows.h>
#pragma warning(pop)
#include <nkintr.h>
#pragma warning(push)
#pragma warning(disable: 4214)
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
// Function:  CspFirGetSIRBaseRegAddr
//
// This function returns the physical base address of UART which 
// is used as SIR.
//
// Parameters:
//      None
//
// Returns:
//      Physical base address for UART register.
//
//-----------------------------------------------------------------------------
ULONG CspFirGetSIRBaseRegAddr()
{
    return CSP_BASE_REG_PA_UART2;
}

//-----------------------------------------------------------------------------
//
// Function:  CspFirGetFIRBaseRegAddr
//
// This function returns the physical base address of FIR.
//
// Parameters:
//      None
//
// Returns:
//      Physical base address for FIR register.
//
//-----------------------------------------------------------------------------
ULONG CspFirGetFIRBaseRegAddr()
{
    return CSP_BASE_REG_PA_FIRI;
}

//-----------------------------------------------------------------------------
//
// Function:  CspFirSetRegConfig
//
// This function sets the iMX51 specific configurations for the registers of FIR.
//
// Parameters:
//           pFirReg [IN] FIR Register Address
//
// Returns:
//
//
//-----------------------------------------------------------------------------
VOID CspFirConfig (PCSP_FIRI_REG pFirReg)
{
    INSREG32BF(&pFirReg->FIRI_TCR, FIRI_TCR_TPP, FIRI_TCR_TPP_NO_INVERT);
}


