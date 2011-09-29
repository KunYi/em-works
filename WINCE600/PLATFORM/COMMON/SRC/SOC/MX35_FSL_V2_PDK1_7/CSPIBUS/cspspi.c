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
/*---------------------------------------------------------------------------
* Copyright (C) 2006-2008, Freescale Semiconductor, Inc. All Rights Reserved.
* THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
* AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
*--------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
//
//  File:  cspspi.c
//
//  Provides SoC-specific configuration routines for
//  the CSPIBUS.
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
// Function:  CspCSPIGetBaseRegAddr
//
// This function returns the physical base address for the
// CSPI registers based on the device index requested.
//
// Parameters:
//      index
//          [in] Index of the CSPI device requested.
//
// Returns:
//      Physical base address for CSPI registers, or 0 if an
//      invalid index was passed.
//
//-----------------------------------------------------------------------------
DWORD CspCSPIGetBaseRegAddr(UINT32 index)
{
    switch (index)
    {
        case 1:
            return CSP_BASE_REG_PA_CSPI1;
        case 2:
            return CSP_BASE_REG_PA_CSPI2;
        default:
            return 0;
    }
}

//-----------------------------------------------------------------------------
//
// Function:  CspCSPIGetIRQ
//
// This function returns the IRQ number for the
// CSPI based on the device index requested.
//
// Parameters:
//      index
//          [in] Index of the CSPI device requested.
//
// Returns:
//      IRQ number for CSPI, or 0 if an
//      invalid index was passed.
//
//-----------------------------------------------------------------------------

DWORD CspCSPIGetIRQ(UINT32 index)
{
    switch (index)
    {
        case 1:
            return IRQ_CSPI1;
        case 2:
            return IRQ_CSPI2;
        default:
            return 0;
    }
}

//-----------------------------------------------------------------------------
//
// Function:  CspCSPIGetDmaReqTx
//
// This function returns Tx Dma Req the device index requested.
//
// Parameters:
//      index
//          [in] Index of the CSPI device requested.
//
// Returns:
//      Tx DMA request or 0 if an invalid index was passed.
//
//-----------------------------------------------------------------------------
DDK_DMA_REQ CspCSPIGetDmaReqTx(UINT32 index)
{
    switch (index)
    {
        case 1:
            return DDK_DMA_REQ_CSPI1_TX;
        case 2:
            return DDK_DMA_REQ_CSPI2_TX;
        default:
            return 0;
    }
}

//-----------------------------------------------------------------------------
//
// Function:  CspCSPIGetDmaReqRx
//
// This function returns the Rx Dma Req the device index requested.
//
// Parameters:
//      index
//          [in] Index of the CSPI device requested.
//
// Returns:
//      Rx DMA request or 0 if an invalid index was passed.
//
//-----------------------------------------------------------------------------
DDK_DMA_REQ CspCSPIGetDmaReqRx(UINT32 index)
{
    switch (index)
    {
        case 1:
            return DDK_DMA_REQ_CSPI1_RX;
        case 2:
            return DDK_DMA_REQ_CSPI2_RX;
        default:
            return 0;
    }
}
