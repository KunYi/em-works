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
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004, Motorola Inc. All Rights Reserved
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  uart.h
//
//   Header file for zeus serial device.
//
//------------------------------------------------------------------------------

#ifndef __UART_H__
#define __UART_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines
#define UART_REF_FREQ    33250000
#define IRSC_BAUDRATE    38400
#define UART_BAUDRATE_DEFAULT    115200

#define SER_FIFO_TXTL    0x2
#define SER_FIFO_RXTL    0x1
#define UART_URXD_RX_DATA_MSK    0xFF

// UBIR & UBMR settings using a std denominator value of 10000
#define UART_UBIR_INC(baud, ref) \
        ((UINT32)((((UINT64)(baud) * 16 * 10000 / (ref)))-1))
#define UART_UBMR_MOD			(10000 - 1)


typedef enum uartType {
    DCE,
    DTE
}uartType_c;

typedef enum irMode {
    SIR_MODE,
    MIR_MODE,
    FIR_MODE
}irMode_c;

//------------------------------------------------------------------------------
// Types

//------------------------------------------------------------------------------
// Functions

#ifdef __cplusplus
}
#endif
#endif // __UART_H__

