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
//  Copyright (C) 2004-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//  File:  sir.h
//
//   Header file for iMX51 FIR device.
//
//------------------------------------------------------------------------------
#ifndef __SIR_H__
#define __SIR_H__

#include "common_uart.h"

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines

#define SIR_RCV_ERROR \
    CSP_BITFVAL(UART_URXD_ERR, UART_URXD_ERR_ERROR)

#define SIR_INTR_MASK \
    CSP_BITFVAL(UART_USR2_TXDC, UART_USR2_TXDC_SET) |CSP_BITFVAL(UART_USR2_RDR, UART_USR2_RDR_SET)


// These are fine-tuning parameters for the COM port ISR.
// Number of times we poll a COM port register waiting 
// for a value which may/must appear.
    
#define REG_POLL_LOOPS      50000  
#define REG_TIMEOUT_LOOPS   1000000


//------------------------------------------------------------------------------
// Types

typedef enum {
    STATE_INIT,
    STATE_GOT_BOF,
    STATE_ACCEPTING,
    STATE_ESC_SEQUENCE,
    STATE_SAW_EOF,
    STATE_CLEANUP
} portRcvState;

// SIR port information object
typedef struct SirPort{
    UCHAR rawBuf[SER_FIFO_DEPTH];
    PUCHAR readBuf;
    portRcvState rcvState;
    UINT rcvBufPos;
    UINT sendBufPos;
}SirPort_t, *pSirPort_t;


//------------------------------------------------------------------------------
// Functions

#ifdef __cplusplus
}
#endif

#endif // __SIR_H__
