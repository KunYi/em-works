//-----------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2005-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  cspipu_base.c
//
//  Provides SoC-specific configuration routines for
//  the IPU common driver.
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
// Function:  IPUGetBaseRegAddr
//
// This function returns the physical base address for the
// IPU registers.
//
// Parameters:
//      None.
//
// Returns:
//      Physical base address for IPU registers.
//
//-----------------------------------------------------------------------------
UINT32 IPUGetBaseRegAddr(void)
{
    return CSP_BASE_REG_PA_IPU;
}

//-----------------------------------------------------------------------------
//
// Function:  IPUGetIRQ
//
// This function returns the IRQ number for the IPU.
//
// Parameters:
//      None.
//
// Returns:
//      IUP IRQ number.
//
//-----------------------------------------------------------------------------
DWORD IPUGetIRQ(void)
{
    return IRQ_IPU_GENERAL;
}

