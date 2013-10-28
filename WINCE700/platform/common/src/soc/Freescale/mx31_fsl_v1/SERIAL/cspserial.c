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
// Copyright (C) 2006, 2007 Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
//
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
#include "mxarm11_uart.h"

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
        //Set IrDA transmission to active low for MX31
    INSREG32BF(&pUartReg->UCR3, UART_UCR3_INVT, UART_UCR3_INVT_ACTIVELOW);
}

