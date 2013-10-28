//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
//-----------------------------------------------------------------------------
//
//  Copyright (C) 2004-2007, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
//  File:  cspi2c.c
//
//  Provides SoC-specific configuration routines for
//  the I2C (Inter IC Communication).
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
            return CSP_BASE_REG_PA_I2C;
        case 2:
            return CSP_BASE_REG_PA_I2C2;
        case 3:
            return CSP_BASE_REG_PA_I2C3;
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
            return IRQ_I2C;
        case 2:
            return IRQ_I2C2;
        case 3:
            return IRQ_I2C3;
        default:
            return 0;
    }
}
