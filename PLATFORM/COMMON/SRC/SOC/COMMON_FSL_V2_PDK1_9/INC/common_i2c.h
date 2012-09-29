//------------------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
//  Use of this source code is subject to the terms of the Microsoft end-user
//  license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
//  If you did not accept the terms of the EULA, you are not authorized to use
//  this source code. For a copy of the EULA, please see the LICENSE.RTF on your
//  install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  common_i2c.h
//
//  Provides definitions for the I2C (Inter IC Communication) module 
//  that are common to Freescale SoCs.
//------------------------------------------------------------------------------

#ifndef __COMMON_I2C_H
#define __COMMON_I2C_H

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
    volatile UINT16 IADR;
    volatile UINT16 _reserved1;
    volatile UINT16 IFDR;
    volatile UINT16 _reserved2;
    volatile UINT16 I2CR;
    volatile UINT16 _reserved3;
    volatile UINT16 I2SR;
    volatile UINT16 _reserved4;
    volatile UINT16 I2DR;
    volatile UINT16 _reserved5;
} CSP_I2C_REG, *PCSP_I2C_REG;

//------------------------------------------------------------------------------
// Functions
//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // __COMMON_I2C_H
