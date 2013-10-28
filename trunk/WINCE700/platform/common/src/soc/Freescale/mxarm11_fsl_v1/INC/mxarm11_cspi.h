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
// Copyright (C) 2004-2007, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//  Header:  mxarm11_cspi.h
//
//  Provides definitions for CSPI module based on Freescale ARM11 chassis.
//
//------------------------------------------------------------------------------

#ifndef __MXARM11_CSPI_H
#define __MXARM11_CSPI_H

#if __cplusplus
extern "C" {
#endif


//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------
typedef struct
{
    UINT32 RXDATA;
    UINT32 TXDATA;
    UINT32 CONREG;
    UINT32 INTREG;
    UINT32 DMAREG;
    UINT32 STATREG;
    UINT32 PERIODREG;
    UINT32 TESTREG;
} CSP_CSPI_REG, *PCSP_CSPI_REG;


//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
#define CSPI_RXDATA_OFFSET          0x0000
#define CSPI_TXDATA_OFFSET          0x0004
#define CSPI_CONREG_OFFSET          0x0008
#define CSPI_INTREG_OFFSET          0x000C
#define CSPI_DMAREG_OFFSET          0x0010
#define CSPI_STATREG_OFFSET         0x0014
#define CSPI_PERIODREG_OFFSET       0x0018
#define CSPI_TESTREG_OFFSET         0x001C


//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
#define CSPI_CONREG_EN_LSH          0
#define CSPI_CONREG_MODE_LSH        1
#define CSPI_CONREG_XCH_LSH         2
#define CSPI_CONREG_SMC_LSH         3
#define CSPI_CONREG_POL_LSH         4
#define CSPI_CONREG_PHA_LSH         5
#define CSPI_CONREG_SSCTL_LSH       6
#define CSPI_CONREG_SSPOL_LSH       7
#define CSPI_CONREG_BITCOUNT_LSH    8
#define CSPI_CONREG_DATARATE_LSH    16
#define CSPI_CONREG_DRCTL_LSH       20
#define CSPI_CONREG_CHIPSELECT_LSH  24

#define CSPI_INTREG_TEEN_LSH        0
#define CSPI_INTREG_THEN_LSH        1
#define CSPI_INTREG_TFEN_LSH        2
#define CSPI_INTREG_RREN_LSH        3
#define CSPI_INTREG_RHEN_LSH        4
#define CSPI_INTREG_RFEN_LSH        5
#define CSPI_INTREG_ROEN_LSH        6
#define CSPI_INTREG_BOEN_LSH        7
#define CSPI_INTREG_TCEN_LSH        8

#define CSPI_DMAREG_TEDEN_LSH       0
#define CSPI_DMAREG_THDEN_LSH       1
#define CSPI_DMAREG_RHDEN_LSH       4
#define CSPI_DMAREG_RFDEN_LSH       5

#define CSPI_STATREG_TE_LSH         0
#define CSPI_STATREG_TH_LSH         1
#define CSPI_STATREG_TF_LSH         2
#define CSPI_STATREG_RR_LSH         3
#define CSPI_STATREG_RH_LSH         4
#define CSPI_STATREG_RF_LSH         5
#define CSPI_STATREG_RO_LSH         6
#define CSPI_STATREG_BO_LSH         7
#define CSPI_STATREG_TC_LSH         8

#define CSPI_PERIODREG_SAMPLEPERIOD_LSH     0
#define CSPI_PERIODREG_CSRC_LSH             15

#define CSPI_TESTREG_TXCNT_LSH      0
#define CSPI_TESTREG_RXCNT_LSH      4
#define CSPI_TESTREG_SMSTATUS_LSH   8
#define CSPI_TESTREG_LBC_LSH        14
#define CSPI_TESTREG_SWAP_LSH       15


//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
#define CSPI_CONREG_EN_WID          1
#define CSPI_CONREG_MODE_WID        1
#define CSPI_CONREG_XCH_WID         1
#define CSPI_CONREG_SMC_WID         1
#define CSPI_CONREG_POL_WID         1
#define CSPI_CONREG_PHA_WID         1
#define CSPI_CONREG_SSCTL_WID       1
#define CSPI_CONREG_SSPOL_WID       1
#define CSPI_CONREG_BITCOUNT_WID    5
#define CSPI_CONREG_DATARATE_WID    3
#define CSPI_CONREG_DRCTL_WID       2
#define CSPI_CONREG_CHIPSELECT_WID  2

#define CSPI_INTREG_TEEN_WID        1
#define CSPI_INTREG_THEN_WID        1
#define CSPI_INTREG_TFEN_WID        1
#define CSPI_INTREG_RREN_WID        1
#define CSPI_INTREG_RHEN_WID        1
#define CSPI_INTREG_RFEN_WID        1
#define CSPI_INTREG_ROEN_WID        1
#define CSPI_INTREG_BOEN_WID        1
#define CSPI_INTREG_TCEN_WID        1

#define CSPI_DMAREG_TEDEN_WID       1
#define CSPI_DMAREG_THDEN_WID       1
#define CSPI_DMAREG_RHDEN_WID       1
#define CSPI_DMAREG_RFDEN_WID       1

#define CSPI_STATREG_TE_WID         1
#define CSPI_STATREG_TH_WID         1
#define CSPI_STATREG_TF_WID         1
#define CSPI_STATREG_RR_WID         1
#define CSPI_STATREG_RH_WID         1
#define CSPI_STATREG_RF_WID         1
#define CSPI_STATREG_RO_WID         1
#define CSPI_STATREG_BO_WID         1
#define CSPI_STATREG_TC_WID         1

#define CSPI_PERIODREG_SAMPLEPERIOD_WID     14
#define CSPI_PERIODREG_CSRC_WID             1

#define CSPI_TESTREG_TXCNT_WID      4
#define CSPI_TESTREG_RXCNT_WID      4
#define CSPI_TESTREG_SMSTATUS_WID   4
#define CSPI_TESTREG_LBC_WID        1
#define CSPI_TESTREG_SWAP_WID       1

//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------

// CONREG
#define CSPI_CONREG_SS0                 0x0     // SS0 will be asserted
#define CSPI_CONREG_SS1                 0x1     // SS1 will be asserted
#define CSPI_CONREG_SS2                 0x2     // SS2 will be asserted
#define CSPI_CONREG_SS3                 0x3     // SS3 will be asserted

#define CSPI_CONREG_DRCTL_DONTCARE      0x0
#define CSPI_CONREG_DRCTL_FALLING_EDGE  0x1
#define CSPI_CONREG_DRCTL_ACTIVE_LOW    0x2

#define CSPI_CONREG_DIV4                0x0     // Divide ratio is 2^(0+2)
#define CSPI_CONREG_DIV8                0x1     // Divide ratio is 2^(1+2)
#define CSPI_CONREG_DIV16               0x2     // Divide ratio is 2^(2+2)
#define CSPI_CONREG_DIV32               0x3     // Divide ratio is 2^(3+2)
#define CSPI_CONREG_DIV64               0x4     // Divide ratio is 2^(4+2)
#define CSPI_CONREG_DIV128              0x5     // Divide ratio is 2^(5+2)
#define CSPI_CONREG_DIV256              0x6     // Divide ratio is 2^(6+2)
#define CSPI_CONREG_DIV512              0x7     // Divide ratio is 2^(7+2)

#define CSPI_CONREG_1BIT                0x00    // 1-bit transfer
#define CSPI_CONREG_BITCOUNT_8BIT       0x07    // 8-bit transfer
#define CSPI_CONREG_BITCOUNT_16BIT      0x0F    // 16-bit transfer
#define CSPI_CONREG_BITCOUNT_32BIT      0x1F    // 32-bit transfer

#define CSPI_CONREG_SSPOL_ACTIVE_LOW    0x0     // Active low
#define CSPI_CONREG_SSPOL_ACTIVE_HIGH   0x1     // Active high

#define CSPI_CONREG_SSCTL_ASSERT        0x0     // SS stays aserted
#define CSPI_CONREG_SSCTL_PULSE         0x1     // Insert pulse

#define CSPI_CONREG_PHA0                0x0     // Phase 0 operation
#define CSPI_CONREG_PHA1                0x1     // Phase 1 operation

#define CSPI_CONREG_POL_ACTIVE_LOW      0x1     // Active low polarity
#define CSPI_CONREG_POL_ACTIVE_HIGH     0x0     // Active high polarity

#define CSPI_CONREG_SMC_XCH             0x0     // XCH bit control SPI burst
#define CSPI_CONREG_SMC_IMM             0x1     // Immediately start SPI burst

#define CSPI_CONREG_XCH_IDLE            0x0     // Idle
#define CSPI_CONREG_XCH_EN              0x1     // Initiates exchange

#define CSPI_CONREG_MODE_SLAVE          0x0     // Slave mode
#define CSPI_CONREG_MODE_MASTER         0x1     // Master mode

#define CSPI_CONREG_EN_DISABLE          0x0     // SPI disabled
#define CSPI_CONREG_EN_ENABLE           0x1     // SPI enabled

// INTREG
#define CSPI_INTREG_TCEN_DISABLE        0       // Transfer completed interrupt
#define CSPI_INTREG_TCEN_ENABLE         1       // Transfer completed interrupt

#define CSPI_INTREG_BOEN_DISABLE        0       // Bit count overflow interrupt
#define CSPI_INTREG_BOEN_ENABLE         1       // Bit count overflow interrupt

#define CSPI_INTREG_ROEN_DISABLE        0       // RxFIFO overflow interrupt
#define CSPI_INTREG_ROEN_ENABLE         1       // RxFIFO overflow interrupt

#define CSPI_INTREG_RFEN_DISABLE        0       // RxFIFO full interrupt
#define CSPI_INTREG_RFEN_ENABLE         1       // RxFIFO full interrupt

#define CSPI_INTREG_RHEN_DISABLE        0       // RxFIFO half interrupt
#define CSPI_INTREG_RHEN_ENABLE         1       // RxFIFO half interrupt

#define CSPI_INTREG_RREN_DISABLE        0       // RxFIFO data ready interrupt
#define CSPI_INTREG_RREN_ENABLE         1       // RxFIFO data ready interrupt

#define CSPI_INTREG_TFEN_DISABLE        0       // TxFIFO full interrupt
#define CSPI_INTREG_TFEN_ENABLE         1       // TxFIFO full interrupt

#define CSPI_INTREG_THEN_DISABLE        0       // TxFIFO half interrupt
#define CSPI_INTREG_THEN_ENABLE         1       // TxFIFO half interrupt

#define CSPI_INTREG_TEEN_DISABLE        0       // TxFIFO empty interrupt
#define CSPI_INTREG_TEEN_ENABLE         1       // TxFIFO empty interrupt

// DMAREG
#define CSPI_DMAREG_RFDEN_DISABLE       0       // RxFIFO full DMA req
#define CSPI_DMAREG_RFDEN_ENABLE        1       // RxFIFO full DMA req

#define CSPI_DMAREG_RHDEN_DISABLE       0       // RxFIFO half full DMA req
#define CSPI_DMAREG_RHDEN_ENABLE        1       // RxFIFO half full DMA req

#define CSPI_DMAREG_THDEN_DISABLE       0       // TxFIFO half empty DMA req
#define CSPI_DMAREG_THDEN_ENABLE        1       // TxFIFO half empty DMA req

#define CSPI_DMAREG_TEDEN_DISABLE       0       // TxFIFO empty DMA req
#define CSPI_DMAREG_TEDEN_ENABLE        1       // TxFIFO empty DMA req

// PERIODREG
#define CSPI_PERIODREG_CSRC_SPI         0x0     // SPI clock
#define CSPI_PERIODREG_CSRC_CKIL        0x1     // 32.768kHz

// TESTREG
#define CSPI_TESTREG_LBC_NOCONN         0x0     // Not connected
#define CSPI_TESTREG_LBC_CONN           0x1     // Internally connected
#define CSPI_TESTREG_SWAP_DISABLE       0x0     // Do not Swap
#define CSPI_TESTREG_SWAP_ENABLE        0x1     // Swap bits bits in the RXFIFO

#ifdef __cplusplus
}
#endif

#endif // __MXARM11_CSPI_H
