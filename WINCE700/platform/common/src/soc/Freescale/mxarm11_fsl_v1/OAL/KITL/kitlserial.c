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
//
// FILE:    kitlserial.c
//
// PURPOSE: This module provides the OAL functions for supporting a
//          serial tranport for KITL.
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// INCLUDE FILES
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <nkintr.h>
#include <oal.h>
#pragma warning(pop)

#include "mxarm11.h"

//-----------------------------------------------------------------------------
// EXTERNAL FUNCTIONS
//-----------------------------------------------------------------------------
extern void OEMKitlSerialInit (void);

//-----------------------------------------------------------------------------
// GLOBAL DEFINITIONS
//-----------------------------------------------------------------------------
#define UART_UCR2_CTS_ACTIVE    0x1000
#define UART_RX_FRAMING_ERR 0x1000
#define UART_RX_PARITY_ERR  0x400
#define UART_RX_OVERRUN     0x2000
#define UART_RX_ERRORS      (UART_RX_FRAMING_ERR | UART_RX_PARITY_ERR | UART_RX_OVERRUN)

//-----------------------------------------------------------------------------
// GLOBAL VARIABLES
//-----------------------------------------------------------------------------
static PCSP_UART_REG g_pUART;


//-----------------------------------------------------------------------------
// STATIC FUNCTIONS
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//
// FUNCTION:    SerialInit
//
// DESCRIPTION:
//      Initializes the interal UART with the specified communication settings.
//
// PARAMETERS:
//      pSerInfo
//          [in/out] Pointer to KITL_SERIAL_INFO structure that contains
//          information about how to initialize the serial KITL transport.
//
// RETURNS:
//      If this function succeeds, it returns TRUE, otherwise
//      it returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL SerialInit(KITL_SERIAL_INFO *pSerInfo)
{
    // Get pointer to peripheral base
    g_pUART = (PCSP_UART_REG) pSerInfo->pAddress;

    if (g_pUART == NULL) return FALSE;

    // Tell KITL upper layers that optimal tranfer size is TXFIFO size
    pSerInfo->bestSize = UART_TXFIFO_DEPTH;

    OEMKitlSerialInit ();

    return TRUE;
}


//-----------------------------------------------------------------------------
//
// FUNCTION:    SerialDeinit
//
// DESCRIPTION:
//      Deinitializes the internal UART previously configured with SerialInit.
//
// PARAMETERS:
//      None.
//
// RETURNS:
//      None
//
//-----------------------------------------------------------------------------
VOID SerialDeinit(void)
{
}


//-----------------------------------------------------------------------------
//
// FUNCTION:    SerialRecv
//
// DESCRIPTION:
//      This function is used by KITL protocol to send a packet/frame to the
//      desktop KITL transport.
//
// PARAMETERS:
//      pch
//          [out] Pointer to buffer used store received data.
//
//      cbRead
//          [in] size of the buffer to be read
//
// RETURNS:  
//      Number of characters read
//
//-----------------------------------------------------------------------------
UINT16 SerialRecv(UINT8 *pch, UINT16 cbRead)
{    
    UINT32 urxd;
    UINT16 count;

    // read until buffer size is reached or an error occurs
    for(count = 0; count < cbRead; count++)
    {           
        if (!(INREG32(&g_pUART->USR2) & CSP_BITFMASK(UART_USR2_RDR)))
            break;
        
        // read char from FIFO
        urxd = INREG32(&g_pUART->URXD);

        // If error detected in current character
        if (urxd & UART_RX_ERRORS) {
            KITLOutputDebugString("E %x\n", urxd);
            count = 0;
            break;
        }
        
        // Place read char into buffer
    *(pch + count) = (UINT8) (urxd & 0xff);
    }

    // KITLOutputDebugString("<");

    return count;
}


//-----------------------------------------------------------------------------
//
// FUNCTION:    SerialSend
//
// DESCRIPTION: 
//      This function is used by KITL protocol to send a packet/frame to the 
//      desktop KITL transport.
//
// PARAMETERS:
//      pch 
//          [in] Pointer to buffer holding data to be transmitted.
//
//      cbSend 
//          [in] size of the buffer to be written
//
// RETURNS:  
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
UINT16 SerialSend(UINT8 *pch, UINT16 cbSend)
    {
    UINT16 count;

    for(count = 0; count < cbSend; count++) {
        // Wait until there is room in the FIFO
    while(INREG32(&g_pUART->UTS) & CSP_BITFMASK(UART_UTS_TXFULL));
        // Write character to port
    OUTREG32(&g_pUART->UTXD, *pch); 
        // Point to next character
        ++pch;
    }

    // KITLOutputDebugString(">");
    
    return count;
}

//-----------------------------------------------------------------------------
//
// FUNCTION:    SerialSendComplete
//
// DESCRIPTION: 
//      This function is used by KITL protocol to indicate packet send completion
//
// PARAMETERS:
//      size 
//          [in] Number of charcters that has been transmitted.
//
// RETURNS:  
//      None
//
//-----------------------------------------------------------------------------
VOID SerialSendComplete(UINT16 size)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(size);
}


//-----------------------------------------------------------------------------
//
// FUNCTION:    SerialEnableInts
//
// DESCRIPTION: 
//      This function enables serial interrupts used for the KITL transport.
//
// PARAMETERS:
//      None.
//
// RETURNS:  
//      None.
//
//-----------------------------------------------------------------------------
VOID SerialEnableInts(void)
{
}


//-----------------------------------------------------------------------------
//
// FUNCTION:    SerialDisableInts
//
// DESCRIPTION: 
//      This function disables serial interrupts used for the KITL transport.
//
// PARAMETERS:
//      None.
//
// RETURNS:  
//      None.
//
//-----------------------------------------------------------------------------
VOID SerialDisableInts(void)
{
}

//-----------------------------------------------------------------------------
//
// FUNCTION:    SerialFlowControl
//
// DESCRIPTION: 
//      This function control the serial flow for the KITL transport.
//
// PARAMETERS:
//      fOn
//
// RETURNS:  
//      None.
//
//-----------------------------------------------------------------------------
VOID SerialFlowControl (BOOL fOn)
{
    if (fOn)
    {
        INSREG32BF(&g_pUART->UCR2, UART_UCR2_IRTS, UART_UCR2_IRTS_IGNORERTS);   
        //INSREG32BF(&g_pUART->UCR2, UART_UCR2_CTS, UART_UCR2_CTS_LOW);

    }
    else
    {
        OUTREG32(&g_pUART->UCR2, 
                INREG32(&g_pUART->UCR2)&~CSP_BITFMASK(UART_UCR2_IRTS));
        //OUTREG32(&g_pUART->UCR2, 
          //  INREG32(&g_pUART->UCR2)&~CSP_BITFMASK(UART_UCR2_CTS));
    }
}

//-----------------------------------------------------------------------------
// PRIVATE FUNCTIONS
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// END OF FILE
//-----------------------------------------------------------------------------
