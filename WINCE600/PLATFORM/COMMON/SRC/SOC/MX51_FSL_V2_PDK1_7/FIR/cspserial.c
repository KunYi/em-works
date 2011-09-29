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
//  File:  cspserial.c
//
//  Provides SoC-specific configuration routines for
//  the SERIAL/SIR.
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
// Function:  CspSerialSetMux
//
// This function sets the RXDMUX select bit of CR3 of UARTs.
//
// Parameters:
//           pUartReg [IN] UART Register Address
//
// Returns:
//
//
//-----------------------------------------------------------------------------
VOID CspSerialSetMux(PCSP_UART_REG pUartReg)
{
    INSREG32BF(&pUartReg->UCR3, UART_UCR3_RXDMUXSEL, UART_UCR3_RXDMUXSEL_MUX);
}

//-----------------------------------------------------------------------------
//
// Function:  CspSerialSetTxIvt
//
// This function sets the RXDMUX select bit of CR3 of UARTs.
//
// Parameters:
//            pUartReg [IN] UART Register Address
//
// Returns:
//
//
//-----------------------------------------------------------------------------
VOID CspSerialSetTxIvt(PCSP_UART_REG pUartReg)
{
    //Set IrDA transmission to active low for iMX51
    INSREG32BF(&pUartReg->UCR3, UART_UCR3_INVT, UART_UCR3_INVT_ACTIVELOW);
}

