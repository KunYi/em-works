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
// Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
// File:  cspi2c.c
//
// Provides SoC-specific configuration routines for
// the I2C (Inter IC Communication).
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
// Function:  I2CGetBaseRegAddr
//
// This function returns the physical base address for the
// I2C registers based on the device index requested.
//
// Parameters:
//      index
//          [in] Index of the I2C device requested.
//
// Returns:
//      Physical base address for I2C registers, or 0 if an
//      invalid index was passed.
//
//-----------------------------------------------------------------------------
UINT32 I2CGetBaseRegAddr(UINT32 index)
{
    switch (index)
    {
        case 1:
            return CSP_BASE_REG_PA_I2C1;
        case 2:
            return CSP_BASE_REG_PA_I2C2;
        default:
            return 0;
    }
}


//-----------------------------------------------------------------------------
//
// Function:  I2CGetIRQ
//
// This function returns the IRQ number for the
// I2C based on the device index requested.
//
// Parameters:
//      index
//          [in] Index of the I2C device requested.
//
// Returns:
//      IRQ number for I2C, or 0 if an
//      invalid index was passed.
//
//-----------------------------------------------------------------------------
UINT32 I2CGetIRQ(UINT32 index)
{
    switch (index)
    {
        case 1:
            return IRQ_I2C1;
        case 2:
            return IRQ_I2C2;
        default:
            return 0;
    }
}
