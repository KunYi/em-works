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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
//------------------------------------------------------------------------------
//
//  Header:  bulverde_uart.h
//
//  Defines the UART controller register layout associated types and constants.
//
#ifndef __BULVERDE_UART_H
#define __BULVERDE_UART_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  Type:  BULVERDE_UART_REG    
//
//  UART control registers.
//
typedef struct {
    UINT32    thr_rbr_dll;    // DLAB = 0  WO  8bit - Transmit Holding Register (THR).
                              // DLAB = 0  RO  8bit - Receive Buffer Register (RBR).
                              // DLAB = 1  RW  8bit - Divisor Latch Low Register (DLL).
    UINT32    ier_dlh;        // DLAB = 0  RW  8bit - Interrupt Enable Register.
    UINT32    iir_fcr;        // DLAB = X  RO  8bit - Interrupt Identification Register.
    UINT32    lcr;            // DLAB = X  RW  8bit - Line Control Register.
    UINT32    mcr;            // DLAB = X  RW  8bit - Modem Control Regiser.
    UINT32    lsr;            // DLAB = X  RO  8bit - Line Status Register.
    UINT32    msr;            // DLAB = X  RO  8bit - Modem Status Register.
    UINT32    scr;            // DLAB = X  RW  8bit - Scratchpad Register.
    UINT32    irdasel;        // DLAB = X  RW  8bit - IrDA Select Register.
    UINT32    fior;           // DLAB = X  RO  FIFO Occupancy Register.
    UINT32    abr;            // DLAB = X  RW  Autobaud Control Register.
    UINT32    acr;            // DLAB = X Autobaud Count Register.

} BULVERDE_UART_REG, *PBULVERDE_UART_REG;


// FFUART
//
typedef BULVERDE_UART_REG BULVERDE_FFUART_REG;
typedef BULVERDE_UART_REG *PBULVERDE_FFUART_REG;

// BTUART
//
typedef BULVERDE_UART_REG BULVERDE_BTUART_REG;
typedef BULVERDE_UART_REG *PBULVERDE_BTUART_REG;

// STUART
//
typedef BULVERDE_UART_REG BULVERDE_STUART_REG;
typedef BULVERDE_UART_REG *PBULVERDE_STUART_REG;

//------------------------------------------------------------------------------

#if __cplusplus
    }
#endif

#endif

