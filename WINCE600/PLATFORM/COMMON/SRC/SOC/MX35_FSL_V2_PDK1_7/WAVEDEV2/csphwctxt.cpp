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
//  Copyright (C) 2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File: csphwctxt.cpp
//
//  The wavedev2 hwctxt SOC-level implementation.
//
//-----------------------------------------------------------------------------
#include <windows.h>
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
//  Function: GetSsiBaseRegAddr
//
//  This function returns the physical base address for the
//  SSI registers based on the device index requested.
//
//  Parameters:
//      index
//          [in] Index of the SSI device requested.
//
//  Returns:
//      Physical base address for SSI registers, or 0 if an
//      invalid index was passed.
//
//-----------------------------------------------------------------------------
UINT32 GetSsiBaseRegAddr(UINT32 index)
{
    switch (index)
    {
    case 1:
        return CSP_BASE_REG_PA_SSI1;

    case 2:
        return CSP_BASE_REG_PA_SSI2;

    default:
        return 0;
    }
}


//-----------------------------------------------------------------------------
//
//  Function: GetAudmuxBaseRegAddr
//
//  This function returns the physical base address for the
//  AUDMUX registers based on the device index requested.
//
//  Parameters:
//      None.
//
//  Returns:
//      Physical base address for AUDMUX registers.
//
//-----------------------------------------------------------------------------
UINT32 GetAudmuxBaseRegAddr()
{
    return CSP_BASE_REG_PA_AUDMUX;
}

//-----------------------------------------------------------------------------
//
//  Function: GetSdmaChannelIRQ
//
//  This function returns the IRQ number of the specified SDMA channel.
//
//  Parameters:
//      chan
//          [in] The SDMA channel.
//
//  Returns:
//      IRQ number.
//
//-----------------------------------------------------------------------------
DWORD GetSdmaChannelIRQ(UINT32 chan)
{
    return (IRQ_SDMA_CH0 + chan);
}

