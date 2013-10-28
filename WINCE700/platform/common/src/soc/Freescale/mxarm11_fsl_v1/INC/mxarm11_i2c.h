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
//  Copyright (C) 2004-2007, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  mxarm11_i2c.h
//
//  Provides definitions for I2C module based on Freescale ARM11 chassis.
//------------------------------------------------------------------------------

#ifndef __MXARM11_I2C_H
#define __MXARM11_I2C_H

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------

#define I2C_IADR_OFFSET     0x0000
#define I2C_IFDR_OFFSET     0x0004
#define I2C_I2CR_OFFSET     0x0008
#define I2C_I2SR_OFFSET     0x000C
#define I2C_I2DR_OFFSET     0x0010

//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------

#define I2C_IADR_ADR_LSH        1

#define I2C_IFDR_IC_LSH         0

#define I2C_I2CR_RSTA_LSH       2
#define I2C_I2CR_TXAK_LSH       3
#define I2C_I2CR_MTX_LSH        4
#define I2C_I2CR_MSTA_LSH       5
#define I2C_I2CR_IIEN_LSH       6
#define I2C_I2CR_IEN_LSH        7

#define I2C_I2SR_RXAK_LSH       0
#define I2C_I2SR_IIF_LSH        1
#define I2C_I2SR_SRW_LSH        2
#define I2C_I2SR_IAL_LSH        4
#define I2C_I2SR_IBB_LSH        5
#define I2C_I2SR_IAAS_LSH       6
#define I2C_I2SR_ICF_LSH        7

#define I2C_I2DR_D_LSH          0

//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------

#define I2C_IADR_ADR_WID        7

#define I2C_IFDR_IC_WID         6

#define I2C_I2CR_RSTA_WID       1
#define I2C_I2CR_TXAK_WID       1
#define I2C_I2CR_MTX_WID        1
#define I2C_I2CR_MSTA_WID       1
#define I2C_I2CR_IIEN_WID       1
#define I2C_I2CR_IEN_WID        1

#define I2C_I2SR_RXAK_WID       1
#define I2C_I2SR_IIF_WID        1
#define I2C_I2SR_SRW_WID        1
#define I2C_I2SR_IAL_WID        1
#define I2C_I2SR_IBB_WID        1
#define I2C_I2SR_IAAS_WID       1
#define I2C_I2SR_ICF_WID        1

#define I2C_I2DR_D_WID          8

//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------

// I2CR
#define I2C_I2CR_RSTA_GENERATE      1   // Repeat Start will be generated
#define I2C_I2CR_RSTA_WITHHOLD      0   // Otherwise

#define I2C_I2CR_TXAK_NO_ACK_SEND   1   // will not send an acknowledgement
#define I2C_I2CR_TXAK_ACK_SEND      0   // otherwise

#define I2C_I2CR_MTX_TRANSMIT       1   // set to transmit mode
#define I2C_I2CR_MTX_RECEIVE        0   // set to receive mode

#define I2C_I2CR_MSTA_MASTER        1   // set to master mode
#define I2C_I2CR_MSTA_SLAVE         0   // set to slave mode

#define I2C_I2CR_IIEN_ENABLE        1   // enable interrupt
#define I2C_I2CR_IIEN_DISABLE       0   // otherwise

#define I2C_I2CR_IEN_ENABLE         1   // enable I2C module
#define I2C_I2CR_IEN_DISABLE        0   // otherwise

// I2SR
#define I2C_I2SR_RXAK_NO_ACK_DETECT 1   // did not detect an ack during bus cycle
#define I2C_I2SR_RXAK_ACK_DETECT    0   // otherwise

#define I2C_I2SR_IIF_PENDING        1   // interrupt pending
#define I2C_I2SR_IIF_NOT_PENDING    0   // otherwise

#define I2C_I2SR_SRW_TRANSMIT       1   // slave transmitting
#define I2C_I2SR_SRW_RECEIVE        0   // slave receiving

#define I2C_I2SR_IAL_LOST           1   // Arbitration loss
#define I2C_I2SR_IAL_NOT_LOST       0   // otherwise

#define I2C_I2SR_IBB_BUSY           1   // bus busy
#define I2C_I2SR_IBB_IDLE           0   // otherwise

#define I2C_I2SR_IAAS_ADDRESSED     1   // addressed as slave
#define I2C_I2SR_IAAS_NOT_ADDRESSED 0   // otherwise

#define I2C_I2SR_ICF_COMPLETE       1   // transfer complete
#define I2C_I2SR_ICF_IN_PROGRESS    0   // transfer in progress

//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------

typedef struct
{
    UINT16 IADR;
    UINT16 _reserved1;
    UINT16 IFDR;
    UINT16 _reserved2;
    UINT16 I2CR;
    UINT16 _reserved3;
    UINT16 I2SR;
    UINT16 _reserved4;
    UINT16 I2DR;
    UINT16 _reserved5;
} CSP_I2C_REG, *PCSP_I2C_REG;

//------------------------------------------------------------------------------
// Functions
//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // __MXARM11_I2C_H