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
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  debugserial.c
//
//  This module is provides the interface to the serial port.
//
//-----------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115)
#include <windows.h>
#include <nkintr.h>
#pragma warning(pop)

#include "bsp.h"
#include <dbgserial.h>

//------------------------------------------------------------------------------
// Defines
//

//------------------------------------------------------------------------------
// Externs
//
//------------------------------------------------------------------------------
// External Functions
extern BOOL OALBspArgsInit(VOID);
extern VOID OALBspArgsPrint(VOID);

//------------------------------------------------------------------------------
// Global Variables

//------------------------------------------------------------------------------
// Local Variables
//
static PVOID pv_HWregUARTDbg = NULL;

//------------------------------------------------------------------------------
// Local Functions
//
//------------------------------------------------------------------------------
//
//  Function: OEMInitDebugSerial
//
//  Initializes the debug serial port
//

VOID OEMInitDebugSerial()
{
#ifdef DEBUG
    UINT32 logMask = 0;
#endif
    PVOID pv_HWregPINCTRL;
    UINT32 UartReadDummy;

#ifdef DEBUG
    // At this moment we must suppress logging.
    //
    logMask = dpCurSettings.ulZoneMask;
    dpCurSettings.ulZoneMask = 0;
#endif

    // Initialize BSP_ARGS to get early clocking info
    OALBspArgsInit();


    pv_HWregUARTDbg = (PVOID) OALPAtoVA(CSP_BASE_REG_PA_UARTDBG, FALSE);
    pv_HWregPINCTRL = (PVOID) OALPAtoVA(CSP_BASE_REG_PA_PINCTRL, FALSE);

    // Make sure all debug UART interrupts are off
    HW_UARTDBGIMSC_WR(0x0);

    HW_PINCTRL_CTRL_CLR(BM_PINCTRL_CTRL_SFTRST | BM_PINCTRL_CTRL_CLKGATE);

    // Configure the GPIO UART pins.
    HW_PINCTRL_MUXSEL7_SET(0xF);          // Switch both pins to GPIO
    HW_PINCTRL_MUXSEL7_CLR(1 << 2);       // DBG-TX (bank 3 pin 17) muxmode=10
    HW_PINCTRL_MUXSEL7_CLR(1 << 0);       // DBG-RX (bank 3 pin 16) muxmode=10

    // Set the Baud Rate
    HW_UARTDBGIBRD_WR((HW_UARTDBGIBRD_RD() & BM_UARTDBGIBRD_UNAVAILABLE) | GET_UARTDBG_BAUD_DIVINT(DEBUG_BAUD));
    HW_UARTDBGFBRD_WR((HW_UARTDBGFBRD_RD() & BM_UARTDBGFBRD_UNAVAILABLE) | GET_UARTDBG_BAUD_DIVFRAC(DEBUG_BAUD));

    //Setting UART properties to 8N1
    HW_UARTDBGLCR_H_WR(BF_UARTDBGLCR_H_SPS(0)   |
                       BF_UARTDBGLCR_H_WLEN(3)  |
                       BF_UARTDBGLCR_H_FEN(1)   |
                       BF_UARTDBGLCR_H_STP2(0)  |
                       BF_UARTDBGLCR_H_EPS(0)   |
                       BF_UARTDBGLCR_H_PEN(0)   |
                       BF_UARTDBGLCR_H_BRK(0));

    //Clear Tx/Rx FIFO
    for(; (HW_UARTDBGFR_RD() & BM_UARTDBGFR_RXFE) == 0; )
    {
        UartReadDummy = HW_UARTDBGDR_RD();
    }

    // Clear Receive Status
    HW_UARTDBGRSR_ECR_WR((HW_UARTDBGRSR_ECR_RD() & ~BM_UARTDBGRSR_ECR_EC) | \
                         BF_UARTDBGRSR_ECR_EC(0xF));

    HW_UARTDBGIFLS_WR(0x9);

    // Enable the UART.
    HW_UARTDBGCR_WR( BM_UARTDBGCR_UARTEN | BM_UARTDBGCR_RXE | BM_UARTDBGCR_TXE );

#ifdef DEBUG
    // Restore the logging mask.
    //
    dpCurSettings.ulZoneMask = logMask;
#endif

    // Serial debug support is now active.  Print BSP_ARGS info.
    OALBspArgsPrint();

}
//------------------------------------------------------------------------------
//
//  Function: OEMWriteDebugByte
//
//  Transmits a character out the debug serial port.
//
VOID OEMWriteDebugByte(UINT8 ch)
{
    UINT32 loop = 0;
    if (!pv_HWregUARTDbg)
    {
        return;
    }

    // Spin if FIFO has more than half data.
    //
    while ( (BF_RD( UARTDBGFR, TXFF)) && (loop < 0x7FFF))
    {
        loop++;
    }

    // Write a character byte to the FIFO.
    //
    if(!BF_RD( UARTDBGFR, TXFF))
        BF_WR( UARTDBGDR, DATA, ch);
}

//------------------------------------------------------------------------------
//
//  Function: OEMReadDebugByte
//
//  Reads a byte from the debug serial port. Does not wait for a character.
//  If a character is not available function returns "OEM_DEBUG_READ_NODATA".
//------------------------------------------------------------------------------
int OEMReadDebugByte()
{
    int retVal = OEM_DEBUG_READ_NODATA;

    // check if DEBUG UART driver is loaded
    if(HW_UARTDBGILPR_RD()&0x1)    return(retVal);

    if (!pv_HWregUARTDbg)
    {
        return(retVal);
    }

    // Return if no data.
    //
    if((HW_UARTDBGFR_RD() & BM_UARTDBGFR_RXFE) != 0)
    {
        return(retVal);
    }

    // Read data.
    //
    retVal = HW_UARTDBGDR_RD();

    // Signal error if PE or FE was set.
    // Do nothing if BI or OE was set.
    //
    if((retVal & (BM_UARTDBGDR_OE | BM_UARTDBGDR_BE | BM_UARTDBGDR_PE | BM_UARTDBGDR_FE)) != 0)
    {
        retVal = OEM_DEBUG_COM_ERROR;
    }

    return(retVal);
}

//------------------------------------------------------------------------------
//
//  Function: OEMClearDebugCommError
//
//  Clears communications errors (flushes the serial FIFO).
//------------------------------------------------------------------------------
void OEMClearDebugCommError(void)
{
    // check if DEBUG UART driver is loaded
    if(HW_UARTDBGILPR_RD()&0x1)    return;

    while(OEMReadDebugByte() == OEM_DEBUG_COM_ERROR) ;
}

//------------------------------------------------------------------------------
//
//  Function:  OEMWriteDebugString
//
//  Output unicode string to debug serial port
//------------------------------------------------------------------------------
VOID OEMWriteDebugString(UINT16 *string)
{
    // check if DEBUG UART driver is loaded
    if(HW_UARTDBGILPR_RD()&0x1)    return;

    while (*string != L'\0') OEMWriteDebugByte((UINT8)*string++);
}
//------------------------------------------------------------------------------
