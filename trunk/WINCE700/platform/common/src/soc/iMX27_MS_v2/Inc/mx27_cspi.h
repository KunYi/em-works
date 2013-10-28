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
// Copyright (C) 2004, MOTOROLA, INC. All Rights Reserved
//
//------------------------------------------------------------------------------
//
// Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
// Header: mx27_cspi.h
//
// Provides definitions for CSPI module on MX27.
//
//------------------------------------------------------------------------------
#ifndef __MX27_CSPI_H__
#define __MX27_CSPI_H__

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------
#define CSPI_FIFO_SLOT_MAX      8

//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------
typedef struct
{
    REG32 RXDATA;
    REG32 TXDATA;
    REG32 CONTROLREG;
    REG32 INT;
    REG32 TEST;
    REG32 PERIOD;
    REG32 DMA;
    REG32 RESET;
} CSP_CSPI_REGS, *PCSP_CSPI_REGS;


//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
#define CSPI_RXDATA_OFFSET                  0x0000
#define CSPI_TXDATA_OFFSET                  0x0004
#define CSPI_CONTROLREG_OFFSET              0x0008
#define CSPI_INT_OFFSET                     0x000C
#define CSPI_TEST_OFFSET                    0x0010
#define CSPI_PERIOD_OFFSET                  0x0014
#define CSPI_DMA_OFFSET                     0x0018
#define CSPI_RESET_OFFSET                   0x001C

//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
#define CSPI_RXDATA_LSH                     0

#define CSPI_TXDATA_LSH                     0

#define CSPI_CONTROLREG_BITCOUNT_LSH        0
#define CSPI_CONTROLREG_POL_LSH             5
#define CSPI_CONTROLREG_PHA_LSH             6
#define CSPI_CONTROLREG_SSCTL_LSH           7
#define CSPI_CONTROLREG_SSPOL_LSH           8
#define CSPI_CONTROLREG_XCH_LSH             9
#define CSPI_CONTROLREG_SPIEN_LSH           10
#define CSPI_CONTROLREG_MODE_LSH            11
#define CSPI_CONTROLREG_DRCTL_LSH           12
#define CSPI_CONTROLREG_DATARATE_LSH        14
#define CSPI_CONTROLREG_CS_LSH              19
#define CSPI_CONTROLREG_SWAP_LSH            21
#define CSPI_CONTROLREG_SDHC_SPIEN_LSH      22
#define CSPI_CONTROLREG_BURST_LSH           23

#define CSPI_INT_TE_LSH                     0
#define CSPI_INT_TH_LSH                     1
#define CSPI_INT_TF_LSH                     2
#define CSPI_INT_TSHFE_LSH                  3
#define CSPI_INT_RR_LSH                     4
#define CSPI_INT_RH_LSH                     5
#define CSPI_INT_RF_LSH                     6
#define CSPI_INT_RO_LSH                     7
#define CSPI_INT_BO_LSH                     8
#define CSPI_INT_TEEN_LSH                   9
#define CSPI_INT_THEN_LSH                   10
#define CSPI_INT_TFEN_LSH                   11
#define CSPI_INT_TSHFEEN_LSH                12
#define CSPI_INT_RREN_LSH                   13
#define CSPI_INT_RHEN_LSH                   14
#define CSPI_INT_RFEN_LSH                   15
#define CSPI_INT_ROEN_LSH                   16
#define CSPI_INT_BOEN_LSH                   17

#define CSPI_TEST_TXCNT_LSH                 0
#define CSPI_TEST_RXCNT_LSH                 4
#define CSPI_TEST_SSSTATUS_LSH              8
#define CSPI_TEST_SS_ASSERT_LSH             12
#define CSPI_TEST_INIT_LSH                  13
#define CSPI_TEST_LBC_LSH                   14

#define CSPI_PERIOD_WAIT_LSH                0
#define CSPI_PERIOD_CSRC_LSH                15

#define CSPI_DMA_RHDMA_LSH                  4
#define CSPI_DMA_RFDMA_LSH                  5
#define CSPI_DMA_TEDMA_LSH                  6
#define CSPI_DMA_THDMA_LSH                  7
#define CSPI_DMA_RHDEN_LSH                  12
#define CSPI_DMA_TRDEN_LSH                  13
#define CSPI_DMA_TEDEN_LSH                  14
#define CSPI_DMA_THDEN_LSH                  15

#define CSPI_RESET_START_LSH                0

//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
#define CSPI_RXDATA_WID                     32

#define CSPI_TXDATA_WID                     32

#define CSPI_CONTROLREG_BITCOUNT_WID        5
#define CSPI_CONTROLREG_POL_WID             1
#define CSPI_CONTROLREG_PHA_WID             1
#define CSPI_CONTROLREG_SSCTL_WID           1
#define CSPI_CONTROLREG_SSPOL_WID           1
#define CSPI_CONTROLREG_XCH_WID             1
#define CSPI_CONTROLREG_SPIEN_WID           1
#define CSPI_CONTROLREG_MODE_WID            1
#define CSPI_CONTROLREG_DRCTL_WID           2
#define CSPI_CONTROLREG_DATARATE_WID        5
#define CSPI_CONTROLREG_CS_WID              2
#define CSPI_CONTROLREG_SWAP_WID            1
#define CSPI_CONTROLREG_SDHC_SPIEN_WID      1
#define CSPI_CONTROLREG_BURST_WID           1

#define CSPI_INT_TE_WID                     1
#define CSPI_INT_TH_WID                     1
#define CSPI_INT_TF_WID                     1
#define CSPI_INT_TSHFE_WID                  1
#define CSPI_INT_RR_WID                     1
#define CSPI_INT_RH_WID                     1
#define CSPI_INT_RF_WID                     1
#define CSPI_INT_RO_WID                     1
#define CSPI_INT_BO_WID                     1
#define CSPI_INT_TEEN_WID                   1
#define CSPI_INT_THEN_WID                   1
#define CSPI_INT_TFEN_WID                   1
#define CSPI_INT_TSHFEEN_WID                1
#define CSPI_INT_RREN_WID                   1
#define CSPI_INT_RHEN_WID                   1
#define CSPI_INT_RFEN_WID                   1
#define CSPI_INT_ROEN_WID                   1
#define CSPI_INT_BOEN_WID                   1

#define CSPI_TEST_TXCNT_WID                 4
#define CSPI_TEST_RXCNT_WID                 4
#define CSPI_TEST_SSSTATUS_WID              4
#define CSPI_TEST_SS_ASSERT_WID             2
#define CSPI_TEST_INIT_WID                  1
#define CSPI_TEST_LBC_WID                   1

#define CSPI_PERIOD_WAIT_WID                14
#define CSPI_PERIOD_CSRC_WID                1

#define CSPI_DMA_RHDMA_WID                  1
#define CSPI_DMA_RFDMA_WID                  1
#define CSPI_DMA_TEDMA_WID                  1
#define CSPI_DMA_RHDMA_WID                  1
#define CSPI_DMA_TEDEN_WID                  1
#define CSPI_DMA_THDEN_WID                  1
#define CSPI_DMA_RHDEN_WID                  1
#define CSPI_DMA_RFDEN_WID                  1

#define CSPI_RESET_START_WID                1

//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------
//
// CONTROLREG
// Bitcounts - the CSPI control code decrements these values by 1 because
// the hardware increments by 1.
#define CSPI_CONTROLREG_BITCOUNT_1BIT       0x01    // 1-bit transfer
#define CSPI_CONTROLREG_BITCOUNT_8BIT       0x08    // 8-bit transfer
#define CSPI_CONTROLREG_BITCOUNT_9BIT       0x09    // 9-bit transfer
#define CSPI_CONTROLREG_BITCOUNT_16BIT      0x10    // 16-bit transfer
#define CSPI_CONTROLREG_BITCOUNT_32BIT      0x20    // 32-bit transfer

#define CSPI_CONTROLREG_POL_ACTIVE_HIGH     0x0     // Active high polarity
#define CSPI_CONTROLREG_POL_ACTIVE_LOW      0x1     // Active low polarity

#define CSPI_CONTROLREG_PHA0                0x0     // Phase 0 operation
#define CSPI_CONTROLREG_PHA1                0x1     // Phase 1 operation

#define CSPI_CONTROLREG_SSCTL_ASSERT        0x0     // MS: SS stays aserted
#define CSPI_CONTROLREG_SSCTL_PULSE         0x1     // MS: Insert pulse

#define CSPI_CONTROLREG_SSPOL_ACTIVE_LOW    0x0     // Active low
#define CSPI_CONTROLREG_SSPOL_ACTIVE_HIGH   0x1     // Active high

#define CSPI_CONTROLREG_XCH_IDLE            0x0     // Idle
#define CSPI_CONTROLREG_XCH_EN              0x1     // Initiates exchange

#define CSPI_CONTROLREG_SPIEN_DISABLE       0x0     // SPI disabled
#define CSPI_CONTROLREG_SPIEN_ENABLE        0x1     // SPI enabled

#define CSPI_CONTROLREG_MODE_SLAVE          0x0     // Slave mode
#define CSPI_CONTROLREG_MODE_MASTER         0x1     // Master mode

#define CSPI_CONTROLREG_DRCTL_DONTCARE      0x0     // Ignored
#define CSPI_CONTROLREG_DRCTL_FALLING_EDGE  0x1     // Falling edge trigger
#define CSPI_CONTROLREG_DRCTL_ACTIVE_LOW    0x2     // Active low trigger

// For even states other than 0, div = 2 * (2 ^ (n/2))
// For odd states other than 0, div = 3 * (2 ^ ((n-1)/2))
#define CSPI_CONTROLREG_DATARATE_DIV3       0x01    // Only available if SDHC_SPIEN set
#define CSPI_CONTROLREG_DATARATE_DIV4       0x02
#define CSPI_CONTROLREG_DATARATE_DIV6       0x03
#define CSPI_CONTROLREG_DATARATE_DIV8       0x04
#define CSPI_CONTROLREG_DATARATE_DIV12      0x05
#define CSPI_CONTROLREG_DATARATE_DIV16      0x06
#define CSPI_CONTROLREG_DATARATE_DIV24      0x07
#define CSPI_CONTROLREG_DATARATE_DIV32      0x08
#define CSPI_CONTROLREG_DATARATE_DIV48      0x09
#define CSPI_CONTROLREG_DATARATE_DIV64      0x0A
#define CSPI_CONTROLREG_DATARATE_DIV96      0x0B
#define CSPI_CONTROLREG_DATARATE_DIV128     0x0C
#define CSPI_CONTROLREG_DATARATE_DIV192     0x0D
#define CSPI_CONTROLREG_DATARATE_DIV256     0x0E
#define CSPI_CONTROLREG_DATARATE_DIV384     0x0F
#define CSPI_CONTROLREG_DATARATE_DIV512     0x10
// Min and max divisor values
#define CSPI_CONTROLREG_DATARATE_DIV_MIN    3
#define CSPI_CONTROLREG_DATARATE_DIV_MAX    512

#define CSPI_CONTROLREG_CS_SS0              0x0     // SS0 will be asserted
#define CSPI_CONTROLREG_CS_SS1              0x1     // SS1 will be asserted
#define CSPI_CONTROLREG_CS_SS2              0x2     // SS2 will be asserted
#define CSPI_CONTROLREG_CS_SS3              0x3     // SS3 will be asserted

#define CSPI_CONTROLREG_SWAP_NOSWAP         0x0     // No swapping
#define CSPI_CONTROLREG_SWAP_SWAPDATA       0x1     // Byte swapping for TxFIFO & RxFIFO

#define CSPI_CONTROLREG_SDHC_SPIEN_DISABLE  0x0     // SDHC SPI mode disabled
#define CSPI_CONTROLREG_SDHC_SPIEN_ENABLE   0x1     // SDHC SPI mode enabled

#define CSPI_CONTROLREG_BURST_INSERTIDLE    0x0     // Idle time inserted between transfers
#define CSPI_CONTROLREG_BURST_NOIDLE        0x1     // No idle time between consecutive data transfers

// INTREG
#define CSPI_INT_TH_NOT_HALF_EMPTY          0       // Less than 4 empty slots in TxFIFO
#define CSPI_INT_TH_HALF_EMPTY              1       // 4 or more empty slots in TxFIFO

#define CSPI_INT_TF_NOT_FULL                0       // Less than 9 data words in TxFIFO
#define CSPI_INT_TF_FULL                    1       // 9 data words in buffer

#define CSPI_INT_TSHFE_NOT_EMPTY            0       // At least one data word is in TxFIFO or at least one data bit is in Tx Shift Register.
#define CSPI_INT_TSHFE_EMPTY                1       // TxFIFO and Tx Shift Register are empty

#define CSPI_INT_RR_EMPTY                   0       // RxFIFO is empty
#define CSPI_INT_RR_NOT_EMPTY               1       // At least one data word in RxFIFO

#define CSPI_INT_RH_NOT_HALF_FULL           0       // Less than 4 data words in RxFIFO
#define CSPI_INT_RH_HALF_FULL               1       // More than or equal to 4 data in RxFIFO

#define CSPI_INT_RF_NOT_FULL                0       // Less than 8 data words in RxFIFO
#define CSPI_INT_RF_FULL                    1       // 8 data words in RxFIFO.

#define CSPI_INT_RO_NOT_OVERFLOW            0       // RxFIFO is not overflow
#define CSPI_INT_RO_OVERFLOW                1       // RxFIFO is overflow

#define CSPI_INT_BO_NOT_OVERFLOW            0       // Bit count overflow error
#define CSPI_INT_BO_OVERFLOW                1       // Bit count overflow

#define CSPI_INT_TEEN_DISABLE               0       // TxFIFO empty interrupt
#define CSPI_INT_TEEN_ENABLE                1       // TxFIFO empty interrupt

#define CSPI_INT_THEN_DISABLE               0       // TxFIFO half interrupt
#define CSPI_INT_THEN_ENABLE                1       // TxFIFO half interrupt

#define CSPI_INT_TFEN_DISABLE               0       // TxFIFO full interrupt
#define CSPI_INT_TFEN_ENABLE                1       // TxFIFO full interrupt

#define CSPI_INT_TSHFEEN_DISABLE            0       // Transfer shift register empty interrupt
#define CSPI_INT_TSHFEEN_ENABLE             1       // Transfer shift register empty interrupt

#define CSPI_INT_RREN_DISABLE               0       // RxFIFO data ready interrupt
#define CSPI_INT_RREN_ENABLE                1       // RxFIFO data ready interrupt

#define CSPI_INT_RHEN_DISABLE               0       // RxFIFO half interrupt
#define CSPI_INT_RHEN_ENABLE                1       // RxFIFO half interrupt

#define CSPI_INT_RFEN_DISABLE               0       // RxFIFO full interrupt
#define CSPI_INT_RFEN_ENABLE                1       // RxFIFO full interrupt

#define CSPI_INT_ROEN_DISABLE               0       // RxFIFO overflow interrupt
#define CSPI_INT_ROEN_ENABLE                1       // RxFIFO overflow interrupt

#define CSPI_INT_BOEN_DISABLE               0       // Bit count overflow interrupt
#define CSPI_INT_BOEN_ENABLE                1       // Bit count overflow interrupt

// TEST
#define CSPI_TEST_INIT_NOINIT               0x0     // No initialization
#define CSPI_TEST_INIT_INITSTATE            0x1     // Init state machine

#define CSPI_TEST_LBC_NOCONN                0x0     // Not connected
#define CSPI_TEST_LBC_CONN                  0x1     // Internally connected

#define CSPI_TEST_SS_ASSERT_DISABLE         0x0     // SSn asserted as per control reg
#define CSPI_TEST_SS_ASSERT_ENABLE          0x1     // SSn remains asserted after TsSHFD set

// PERIOD
#define CSPI_PERIOD_CSRC_SPI                0x0     // SPI clock
#define CSPI_PERIOD_CSRC_CKIL               0x1     // 32.768kHz

// DMA
#define CSPI_DMA_RFDMA_NOT_HALF             0       // RxFIFO has less than 4 slots
#define CSPI_DMA_RFDMA_HALF                 1       // RxFIFO has 4 or more slots

#define CSPI_DMA_RHDMA_NOT_FULL             0       // RxFIFO not full
#define CSPI_DMA_RHDMA_FULL                 1       // RxFIFO is full

#define CSPI_DMA_THDMA_NOT_EMPTY            0       // TxFIFO not empty
#define CSPI_DMA_THDMA_EMPTY                1       // TxFIFO is empty

#define CSPI_DMA_TEDMA_NOT_HALF             0       // TxFIFO has less than 4 empty slots
#define CSPI_DMA_TEDMA_HALF                 1       // TxFIFO has 4 or more empty slots

#define CSPI_DMA_RHDEN_DISABLE              0       // RxFIFO half full DMA req
#define CSPI_DMA_RHDEN_ENABLE               1       // RxFIFO half full DMA req

#define CSPI_DMA_RFDEN_DISABLE              0       // RxFIFO full DMA req
#define CSPI_DMA_RFDEN_ENABLE               1       // RxFIFO full DMA req

#define CSPI_DMA_TEDEN_DISABLE              0       // TxFIFO empty DMA req
#define CSPI_DMA_TEDEN_ENABLE               1       // TxFIFO empty DMA req

#define CSPI_DMA_THDEN_DISABLE              0       // TxFIFO half empty DMA req
#define CSPI_DMA_THDEN_ENABLE               1       // TxFIFO half empty DMA req

// RESET
#define CSPI_RESET_START_NORESET            0       // No soft reset
#define CSPI_RESET_START_SWRESET            1       // Generate soft reset

#ifdef __cplusplus
}
#endif

#endif // __MX27_CSPI_H__

