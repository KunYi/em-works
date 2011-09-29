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
#ifndef __I2CTRANS_H
#define __I2CTRANS_H

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  STRUCT: I2CTRANS
//
#define I2C_OPCODE_READ           1
#define I2C_OPCODE_WRITE          2
#define I2C_OPCODE_MASK           3
#define I2C_OPCODE_REPEATSTART    0x80000000

/* input clock is 12Mhz.  these values are high,low times for clock.  2*value = value to use as
    the divider */
#define I2C_CLOCK_100Khz          0x3C      /* 12Mhz/(60*2) = 100Khz */
#define I2C_CLOCK_400Khz          0x0F      /* 12Mhz/(15*2) = 400Khz */
#define I2C_CLOCK_DEFAULT         I2C_CLOCK_400Khz

#define I2CTRANS_MAX_STREAMED_TRANSACTIONS  8
#define I2CTRANS_BUFFER_BYTES               256
typedef struct _i2cTransaction
{
    /* reserved - do not use */
    DWORD mReserved1;
    DWORD mReserved2;
    /* input: */
    DWORD mClk_HL_Divisor;
    DWORD mOpCode[I2CTRANS_MAX_STREAMED_TRANSACTIONS];
    DWORD mBufferOffset[I2CTRANS_MAX_STREAMED_TRANSACTIONS];
    DWORD mTransLen[I2CTRANS_MAX_STREAMED_TRANSACTIONS];
    /* output: */
    DWORD mErrorCode;
    UCHAR mBuffer[I2CTRANS_BUFFER_BYTES];
} I2CTRANS;


//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif
