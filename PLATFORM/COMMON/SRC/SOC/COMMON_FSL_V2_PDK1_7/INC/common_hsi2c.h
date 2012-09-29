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
//  File:  common_hsi2c.h
//
//  Provides definitions for the HSI2C (High Speed Inter IC Communication) module 
//------------------------------------------------------------------------------

#ifndef __COMMON_HSI2C_H
#define __COMMON_HSI2C_H

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

#define I2C_HISADR_OFFSET     0x0000
#define I2C_HIMADR_OFFSET     0x0004
#define I2C_HICR_OFFSET       0x0008
#define I2C_HISR_OFFSET       0x000C
#define I2C_HIIMR_OFFSET      0x0010
#define I2C_HITDR_OFFSET      0x0014
#define I2C_HIRDR_OFFSET      0x0018
#define I2C_HIFSFDR_OFFSET    0x001C
#define I2C_HIHSFDR_OFFSET    0x0020
#define I2C_HITFR_OFFSET      0x0024
#define I2C_HIRFR_OFFSET      0x0028
#define I2C_HITDCR_OFFSET     0x002C
#define I2C_HIRDCR_OFFSET     0x0030

//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------

#define I2C_SADR_ADR_LSH         1      //slave address register
#define I2C_MADR_ADR_LSH         1      //master address register

#define I2C_FSFDR_IC_LSH         0
#define I2C_HSFDR_IC_LSH         0

#define I2C_HICR_IEN_LSH        0
#define I2C_HICR_DMARX_LSH      1
#define I2C_HICR_DMATX_LSH      2
#define I2C_HICR_RSTA_LSH       3
#define I2C_HICR_TXAK_LSH       4
#define I2C_HICR_MTX_LSH        5
#define I2C_HICR_MSTA_LSH       6
#define I2C_HICR_IIEN_LSH       7
#define I2C_HICR_ADDR_LSH       8
#define I2C_HICR_MST_LSH        9
#define I2C_HICR_HSM_LSH        12

#define I2C_HISR_TDE_LSH        1
#define I2C_HISR_IAAS_LSH       2
#define I2C_HISR_IAL_LSH        3
#define I2C_HISR_BTD_LSH        4
#define I2C_HISR_RDC_LSH        5
#define I2C_HISR_TDC_LSH        6
#define I2C_HISR_RXAK_LSH       7
#define I2C_HISR_IBB_LSH        8
#define I2C_HISR_SRW_LSH        9

#define I2C_HIIMR_RDF_LSH       0
#define I2C_HIIMR_TDE_LSH       1
#define I2C_HIIMR_AAS_LSH       2
#define I2C_HIIMR_AL_LSH        3
#define I2C_HIIMR_BTD_LSH       4
#define I2C_HIIMR_RDC_LSH       5
#define I2C_HIIMR_TDC_LSH       6
#define I2C_HIIMR_RXAK_LSH      7

#define I2C_HITFR_TFEN_LSH      0
#define I2C_HITFR_TFLSH_LSH     1
#define I2C_HITFR_TFWM_LSH      2
#define I2C_HITFR_TFC_LSH       8

#define I2C_HIRFR_RFEN_LSH      0
#define I2C_HIRFR_RFLSH_LSH     1
#define I2C_HIRFR_RFWM_LSH      2
#define I2C_HIRFR_RFC_LSH       8

#define I2C_HITDCR_TDCC_LSH     0
#define I2C_HITDCR_TDCE_LSH     8
#define I2C_HITDCR_TDCR_LSH     9

#define I2C_HIRDCR_RDCC_LSH     0
#define I2C_HIRDCR_RDCE_LSH     8
#define I2C_HIRDCR_RDCR_LSH     9

//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
#define I2C_MADR_ADR_WID        7      //master address register
#define I2C_SADR_ADR_WID        7

#define I2C_FSFDR_IC_WID        6
#define I2C_HSFDR_IC_WID        6

#define I2C_HICR_RSTA_WID       1
#define I2C_HICR_TXAK_WID       1
#define I2C_HICR_MTX_WID        1
#define I2C_HICR_MSTA_WID       1
#define I2C_HICR_IIEN_WID       1
#define I2C_HICR_IEN_WID        1
#define I2C_HICR_ADDR_WID       1
#define I2C_HICR_DMATX_WID      1
#define I2C_HICR_DMARX_WID      1
#define I2C_HICR_HSM_WID        1
#define I2C_HICR_MST_WID        3

#define I2C_HISR_TDE_WID        1
#define I2C_HISR_RXAK_WID       1
#define I2C_HISR_SRW_WID        1
#define I2C_HISR_IAL_WID        1
#define I2C_HISR_IBB_WID        1
#define I2C_HISR_IAAS_WID       1
#define I2C_HISR_BTD_WID        1
#define I2C_HISR_RDC_WID        1
#define I2C_HISR_TDC_WID        1

#define I2C_HIIMR_RDF_WID        1
#define I2C_HIIMR_TDE_WID        1
#define I2C_HIIMR_AAS_WID        1
#define I2C_HIIMR_AL_WID         1
#define I2C_HIIMR_BTD_WID        1
#define I2C_HIIMR_RDC_WID        1
#define I2C_HIIMR_TDC_WID        1
#define I2C_HIIMR_RXAK_WID       1

#define I2C_HITFR_TFEN_WID      1
#define I2C_HITFR_TFLSH_WID     1
#define I2C_HITFR_TFWM_WID      3
#define I2C_HITFR_TFC_WID       4

#define I2C_HIRFR_RFEN_WID      1
#define I2C_HIRFR_RFLSH_WID     1
#define I2C_HIRFR_RFWM_WID      3
#define I2C_HIRFR_RFC_WID       4

#define I2C_HIRDCR_RDCC_WID     8
#define I2C_HIRDCR_RDCE_WID     1
#define I2C_HIRDCR_RDCR_WID     1
//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------

// HICR
#define I2C_HICR_HSM_ENABLE         1
#define I2C_HICR_HSM_DISABLE        0

#define I2C_HICR_DMATX_ENABLE       1
#define I2C_HICR_DMATX_DISABLE      0

#define I2C_HICR_DMARX_ENABLE       1
#define I2C_HICR_DMARX_DISABLE      0

#define I2C_HICR_RSTA_GENERATE      1   // Repeat Start will be generated
#define I2C_HICR_RSTA_WITHHOLD      0   // Otherwise

#define I2C_HICR_TXAK_NO_ACK_SEND   1   // will not send an acknowledgement
#define I2C_HICR_TXAK_ACK_SEND      0   // otherwise

#define I2C_HICR_MTX_TRANSMIT       1   // set to transmit mode
#define I2C_HICR_MTX_RECEIVE        0   // set to receive mode

#define I2C_HICR_MSTA_MASTER        1   // set to master mode
#define I2C_HICR_MSTA_SLAVE         0   // set to slave mode

#define I2C_HICR_IIEN_ENABLE        1   // enable interrupt
#define I2C_HICR_IIEN_DISABLE       0   // otherwise

#define I2C_HICR_IEN_ENABLE         1   // enable I2C module
#define I2C_HICR_IEN_DISABLE        0   // otherwise

#define I2C_HICR_ADDR_10BIT         1
#define I2C_HICR_ADDR_7BIT          0

// HISR
#define I2C_HISR_RXAK_NO_ACK_DETECT 1   // did not detect an ack during bus cycle
#define I2C_HISR_RXAK_ACK_DETECT    0   // otherwise

#define I2C_HISR_SRW_TRANSMIT       1   // slave transmitting
#define I2C_HISR_SRW_RECEIVE        0   // slave receiving

#define I2C_HISR_IAL_LOST           1   // Arbitration loss
#define I2C_HISR_IAL_NOT_LOST       0   // otherwise

#define I2C_HISR_IBB_BUSY           1   // bus busy
#define I2C_HISR_IBB_IDLE           0   // otherwise

#define I2C_HISR_IAAS_ADDRESSED     1   // addressed as slave
#define I2C_HISR_IAAS_NOT_ADDRESSED 0   // otherwise

#define I2C_HISR_BTD_COMPLETE       1   // transfer complete
#define I2C_HISR_BTD_IN_PROGRESS    0   // transfer in progress

#define I2C_HISR_TDE_EMPTY          1
#define I2C_HISR_TDE_FULL           0

#define I2C_HISR_TDC_ZERO           1
#define I2C_HISR_TDC_UNZERO         0

#define I2C_HISR_RDC_ZERO           1
#define I2C_HISR_RDC_UNZERO         0

// HIIMR
#define I2C_HIIMR_RDF_UNMASK        0
#define I2C_HIIMR_RDF_MASK          1

#define I2C_HIIMR_TDE_UNMASK        0
#define I2C_HIIMR_TDE_MASK          1

#define I2C_HIIMR_AAS_UNMASK        0
#define I2C_HIIMR_AAS_MASK          1

#define I2C_HIIMR_AL_UNMASK         0
#define I2C_HIIMR_AL_MASK           1

#define I2C_HIIMR_BTD_UNMASK        0
#define I2C_HIIMR_BTD_MASK          1

#define I2C_HIIMR_RDC_UNMASK        0
#define I2C_HIIMR_RDC_MASK          1

#define I2C_HIIMR_TDC_UNMASK        0
#define I2C_HIIMR_TDC_MASK          1

#define I2C_HIIMR_RXAK_UNMASK       0
#define I2C_HIIMR_RXAK_MASK         1

//HITFR
#define I2C_HITFR_TFEN_ENABLE       1
#define I2C_HITFR_TFEN_DISABLE      0

#define I2C_HITFR_TFLSH_RESET       1
#define I2C_HITFR_TFLSH_NONE        0

//HIRFR
#define I2C_HIRFR_RFEN_ENABLE       1
#define I2C_HIRFR_RFEN_DISABLE      0

#define I2C_HIRFR_RFLSH_RESET       1
#define I2C_HIRFR_RFLSH_NONE        0

//HITDCR
#define I2C_HITDCR_TDCE_ENABLE      1
#define I2C_HITDCR_TDCE_DISABLE     0

#define I2C_HITDCR_TDCR_RESTART     1
#define I2C_HITDCR_TDCR_STOT        0

//HIRDCR
#define I2C_HIRDCR_RDCE_ENABLE      1
#define I2C_HIRDCR_RDCE_DISABLE     0

#define I2C_HIRDCR_RDCR_RESTART     1
#define I2C_HIRDCR_RDCR_STOT        0
//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------

typedef struct
{
    UINT16 HISADR;
    UINT16 _reserved1;
    UINT16 HIMADR;
    UINT16 _reserved2;
    UINT16 HICR;
    UINT16 _reserved3;
    UINT16 HISR;
    UINT16 _reserved4;
    UINT16 HIIMR;
    UINT16 _reserved5;
    UINT16 HITDR;
    UINT16 _reserved6;
    UINT16 HIRDR;
    UINT16 _reserved7;
    UINT16 HIFSFDR;
    UINT16 _reserved8;
    UINT16 HIHSFDR;
    UINT16 _reserved9;
    UINT16 HITFR;
    UINT16 _reserved10;
    UINT16 HIRFR;
    UINT16 _reserved11;
    UINT16 HITDCR;
    UINT16 _reserved12;
    UINT16 HIRDCR;
    UINT16 _reserved13;
} CSP_HSI2C_REG, *PCSP_HSI2C_REG;


//------------------------------------------------------------------------------
// Functions
//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // __COMMON_HSI2C_H
