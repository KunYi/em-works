//-----------------------------------------------------------------------------
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
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header:  MX233_UARTAPP.h
//
//  Provides definitions for UARTAPP module based on Freescale ARM11 chassis.
//
//------------------------------------------------------------------------------

#ifndef __MX233_UARTAPP_H
#define __MX233_UARTAPP_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------
#define UARTAPP_RXFIFO_DEPTH       16
#define UARTAPP_TXFIFO_DEPTH       16

#define UARTAPP_CLOCK_FREQUENCY    (24000000)

#define BUILD_UARTAPP_BAUDRATE_DIVIDER(b)       ((UARTAPP_CLOCK_FREQUENCY * 32) / (b))
#define GET_UARTAPP_BAUD_DIVINT(b)                      ((BUILD_UARTAPP_BAUDRATE_DIVIDER(b)) >> 6)
#define GET_UARTAPP_BAUD_DIVFRAC(b)                     ((BUILD_UARTAPP_BAUDRATE_DIVIDER(b)) & 0x3F)

//divisor must be between 0xEC and 0x3FFFC0, inclusive.
#define MIN_UARTAPP_DIVISOR          0xEC   // Minimum UART XCLK divisor constant.
#define MAX_UARTAPP_DIVISOR      0x3FFFC0   // Maximum UART XCLK divisor constant.

// error status in DATA FIFO
#define FIFO_FRAME_ERROR        0x100
#define FIFO_PARITY_ERROR       0x200
#define FIFO_BREAK_ERROR        0x400
#define FIFO_OVERRUN_ERROR      0x800
#define FIFO_ERROR              0xF00

// invalid received data
#define INVALID_DATA            0x100000

//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------
typedef struct
{
    UINT32 CTRL0[4];
    UINT32 CTRL1[4];
    UINT32 CTRL2[4];
    UINT32 LINECTRL[4];
    UINT32 LINECTRL2[4];
    UINT32 INTR[4];
    UINT32 DATA;
    UINT32 RESERVED1[3];
    UINT32 STAT;
    UINT32 RESERVED2[3];
    UINT32 UARTAPP_DEBUG;
    UINT32 RESERVED3[3];
    UINT32 VERSION;
    UINT32 RESERVED4[3];

} CSP_UARTAPP_REG, *PCSP_UARTAPP_REG;

//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
#define UARTAPP_CTRL0_OFFSET              0x0000
#define UARTAPP_CTRL1_OFFSET              0x0010
#define UARTAPP_CTRL2_OFFSET              0x0020
#define UARTAPP_LINECTRL_OFFSET           0x0030
#define UARTAPP_LINECTRL2_OFFSET          0x0040
#define UARTAPP_INTR_OFFSET               0x0050
#define UARTAPP_DATA_OFFSET               0x0060
#define UARTAPP_STAT_OFFSET               0x0070
#define UARTAPP_DEBUG_OFFSET              0x0080
#define UARTAPP_VERSION_OFFSET            0x0090


//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
#define UARTAPP_CTRL0_XFER_COUNT_LSH         0
#define UARTAPP_CTRL0_RXTIMEOUT_LSH          16
#define UARTAPP_CTRL0_RXTO_ENABLE_LSH        27
#define UARTAPP_CTRL0_RX_SOURCE_LSH          28
#define UARTAPP_CTRL0_RUN_LSH                29
#define UARTAPP_CTRL0_CLKGATE_LSH            30
#define UARTAPP_CTRL0_SFTRST_LSH             31

#define UARTAPP_CTRL1_XFER_COUNT_LSH         0
#define UARTAPP_CTRL1_RUN_LSH                28

#define UARTAPP_CTRL2_UARTAPPEN_LSH          0
#define UARTAPP_CTRL2_SIREN_LSH              1
#define UARTAPP_CTRL2_SIRLP_LSH              2
#define UARTAPP_CTRL2_USE_LCR2_LSH           6
#define UARTAPP_CTRL2_LBE_LSH                7
#define UARTAPP_CTRL2_TXE_LSH                8
#define UARTAPP_CTRL2_RXE_LSH                9
#define UARTAPP_CTRL2_DTR_LSH                10
#define UARTAPP_CTRL2_RTS_LSH                11
#define UARTAPP_CTRL2_OUT1_LSH               12
#define UARTAPP_CTRL2_OUT2_LSH               13
#define UARTAPP_CTRL2_RTSEN_LSH              14
#define UARTAPP_CTRL2_CTSEN_LSH              15
#define UARTAPP_CTRL2_TXIFLSEL_LSH           16
#define UARTAPP_CTRL2_RXIFLSEL_LSH           20
#define UARTAPP_CTRL2_RXDMAE_LSH             24
#define UARTAPP_CTRL2_TXDMAE_LSH             25
#define UARTAPP_CTRL2_DMAONERR_LSH           26
#define UARTAPP_CTRL2_RTS_SEMAPHORE_LSH      27
#define UARTAPP_CTRL2_INVERT_RX_LSH          28
#define UARTAPP_CTRL2_INVERT_TX_LSH          29
#define UARTAPP_CTRL2_INVERT_CTS_LSH         30
#define UARTAPP_CTRL2_INVERT_RTS_LSH         31

#define UARTAPP_LINECTRL_BRK_LSH             0
#define UARTAPP_LINECTRL_PEN_LSH             1
#define UARTAPP_LINECTRL_EPS_LSH             2
#define UARTAPP_LINECTRL_STP2_LSH            3
#define UARTAPP_LINECTRL_FEN_LSH             4
#define UARTAPP_LINECTRL_WLEN_LSH            5
#define UARTAPP_LINECTRL_SPS_LSH             7
#define UARTAPP_LINECTRL_BAUD_DIVFRAC_LSH    8
#define UARTAPP_LINECTRL_BAUD_DIVINT_LSH     16

#define UARTAPP_LINECTRL2_PEN_LSH            1
#define UARTAPP_LINECTRL2_EPS_LSH            2
#define UARTAPP_LINECTRL2_STP2_LSH           3
#define UARTAPP_LINECTRL2_FEN_LSH            4
#define UARTAPP_LINECTRL2_WLEN_LSH           5
#define UARTAPP_LINECTRL2_SPS_LSH            7
#define UARTAPP_LINECTRL2_BAUD_DIVFRAC_LSH   8
#define UARTAPP_LINECTRL2_BAUD_DIVINT_LSH    16

#define UARTAPP_INTR_RIMIS_LSH               0
#define UARTAPP_INTR_CTSMIS_LSH              1
#define UARTAPP_INTR_DCDMIS_LSH              2
#define UARTAPP_INTR_DSRMIS_LSH              3
#define UARTAPP_INTR_RXIS_LSH                4
#define UARTAPP_INTR_TXIS_LSH                5
#define UARTAPP_INTR_RTIS_LSH                6
#define UARTAPP_INTR_FEIS_LSH                7
#define UARTAPP_INTR_PEIS_LSH                8
#define UARTAPP_INTR_BEIS_LSH                9
#define UARTAPP_INTR_OEIS_LSH                10
#define UARTAPP_INTR_RIMIEN_LSH              16
#define UARTAPP_INTR_CTSMIEN_LSH             17
#define UARTAPP_INTR_DCDMIEN_LSH             18
#define UARTAPP_INTR_DSRMIEN_LSH             19
#define UARTAPP_INTR_RXIEN_LSH               20
#define UARTAPP_INTR_TXIEN_LSH               21
#define UARTAPP_INTR_RTIEN_LSH               22
#define UARTAPP_INTR_FEIEN_LSH               23
#define UARTAPP_INTR_PEIEN_LSH               24
#define UARTAPP_INTR_BEIEN_LSH               25
#define UARTAPP_INTR_OEIEN_LSH               26

#define UARTAPP_DATA_DATA_LSH                0

#define UARTAPP_STAT_RXCOUNT_LSH             0
#define UARTAPP_STAT_FERR_LSH                16
#define UARTAPP_STAT_PERR_LSH                17
#define UARTAPP_STAT_BERR_LSH                18
#define UARTAPP_STAT_OERR_LSH                19
#define UARTAPP_STAT_RXBYTE_INVALID_LSH      20
#define UARTAPP_STAT_RXFE_LSH                24
#define UARTAPP_STAT_TXFF_LSH                25
#define UARTAPP_STAT_RXFF_LSH                26
#define UARTAPP_STAT_TXFE_LSH                27
#define UARTAPP_STAT_CTS_LSH                 28
#define UARTAPP_STAT_BUSY_LSH                29
#define UARTAPP_STAT_HISPEED_LSH             30
#define UARTAPP_STAT_PRESENT_LSH             31

#define UARTAPP_DEBUG_RXDMARQ_LSH            0
#define UARTAPP_DEBUG_TXDMARQ_LSH            1
#define UARTAPP_DEBUG_RXCMDEND_LSH           2
#define UARTAPP_DEBUG_TXCMDEND_LSH           3
#define UARTAPP_DEBUG_RXDMARUN_LSH           4
#define UARTAPP_DEBUG_TXDMARUN_LSH           5
#define UARTAPP_DEBUG_RXFBAUD_DIV_LSH        10
#define UARTAPP_DEBUG_RXIBAUD_DIV_LSH        16

#define UARTAPP_VERSION_STEP_LSH             0
#define UARTAPP_VERSION_MINOR_LSH            16
#define UARTAPP_VERSION_MAJOR_LSH            24

//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
#define UARTAPP_CTRL0_XFER_COUNT_WID         16
#define UARTAPP_CTRL0_RXTIMEOUT_WID          11
#define UARTAPP_CTRL0_RXTO_ENABLE_WID        1
#define UARTAPP_CTRL0_RX_SOURCE_WID          1
#define UARTAPP_CTRL0_RUN_WID                1
#define UARTAPP_CTRL0_CLKGATE_WID            1
#define UARTAPP_CTRL0_SFTRST_WID             1

#define UARTAPP_CTRL1_XFER_COUNT_WID         16
#define UARTAPP_CTRL1_RUN_WID                1

#define UARTAPP_CTRL2_UARTAPPEN_WID          1
#define UARTAPP_CTRL2_SIREN_WID              1
#define UARTAPP_CTRL2_SIRLP_WID              1
#define UARTAPP_CTRL2_USE_LCR2_WID           1
#define UARTAPP_CTRL2_LBE_WID                1
#define UARTAPP_CTRL2_TXE_WID                1
#define UARTAPP_CTRL2_RXE_WID                1
#define UARTAPP_CTRL2_DTR_WID                1
#define UARTAPP_CTRL2_RTS_WID                1
#define UARTAPP_CTRL2_OUT1_WID               1
#define UARTAPP_CTRL2_OUT2_WID               1
#define UARTAPP_CTRL2_RTSEN_WID              1
#define UARTAPP_CTRL2_CTSEN_WID              1
#define UARTAPP_CTRL2_TXIFLSEL_WID           3
#define UARTAPP_CTRL2_RXIFLSEL_WID           3
#define UARTAPP_CTRL2_RXDMAE_WID             1
#define UARTAPP_CTRL2_TXDMAE_WID             1
#define UARTAPP_CTRL2_DMAONERR_WID           1
#define UARTAPP_CTRL2_RTS_SEMAPHORE_WID      1
#define UARTAPP_CTRL2_INVERT_RX_WID          1
#define UARTAPP_CTRL2_INVERT_TX_WID          1
#define UARTAPP_CTRL2_INVERT_CTS_WID         1
#define UARTAPP_CTRL2_INVERT_RTS_WID         1

#define UARTAPP_LINECTRL_BRK_WID             1
#define UARTAPP_LINECTRL_PEN_WID             1
#define UARTAPP_LINECTRL_EPS_WID             1
#define UARTAPP_LINECTRL_STP2_WID            1
#define UARTAPP_LINECTRL_FEN_WID             1
#define UARTAPP_LINECTRL_WLEN_WID            2
#define UARTAPP_LINECTRL_SPS_WID             1
#define UARTAPP_LINECTRL_BAUD_DIVFRAC_WID    6
#define UARTAPP_LINECTRL_BAUD_DIVINT_WID     16

#define UARTAPP_LINECTRL2_PEN_WID            1
#define UARTAPP_LINECTRL2_EPS_WID            1
#define UARTAPP_LINECTRL2_STP2_WID           1
#define UARTAPP_LINECTRL2_FEN_WID            1
#define UARTAPP_LINECTRL2_WLEN_WID           2
#define UARTAPP_LINECTRL2_SPS_WID            1
#define UARTAPP_LINECTRL2_BAUD_DIVFRAC_WID   6
#define UARTAPP_LINECTRL2_BAUD_DIVINT_WID    16

#define UARTAPP_INTR_RIMIS_WID               1
#define UARTAPP_INTR_CTSMIS_WID              1
#define UARTAPP_INTR_DCDMIS_WID              1
#define UARTAPP_INTR_DSRMIS_WID              1
#define UARTAPP_INTR_RXIS_WID                1
#define UARTAPP_INTR_TXIS_WID                1
#define UARTAPP_INTR_RTIS_WID                1
#define UARTAPP_INTR_FEIS_WID                1
#define UARTAPP_INTR_PEIS_WID                1
#define UARTAPP_INTR_BEIS_WID                1
#define UARTAPP_INTR_OEIS_WID                1
#define UARTAPP_INTR_RIMIEN_WID              1
#define UARTAPP_INTR_CTSMIEN_WID             1
#define UARTAPP_INTR_DCDMIEN_WID             1
#define UARTAPP_INTR_DSRMIEN_WID             1
#define UARTAPP_INTR_RXIEN_WID               1
#define UARTAPP_INTR_TXIEN_WID               1
#define UARTAPP_INTR_RTIEN_WID               1
#define UARTAPP_INTR_FEIEN_WID               1
#define UARTAPP_INTR_PEIEN_WID               1
#define UARTAPP_INTR_BEIEN_WID               1
#define UARTAPP_INTR_OEIEN_WID               1

#define UARTAPP_DATA_DATA_WID                32

#define UARTAPP_STAT_RXCOUNT_WID             16
#define UARTAPP_STAT_FERR_WID                1
#define UARTAPP_STAT_PERR_WID                1
#define UARTAPP_STAT_BERR_WID                1
#define UARTAPP_STAT_OERR_WID                1
#define UARTAPP_STAT_RXBYTE_INVALID_WID      4
#define UARTAPP_STAT_RXFE_WID                1
#define UARTAPP_STAT_TXFF_WID                1
#define UARTAPP_STAT_RXFF_WID                1
#define UARTAPP_STAT_TXFE_WID                1
#define UARTAPP_STAT_CTS_WID                 1
#define UARTAPP_STAT_BUSY_WID                1
#define UARTAPP_STAT_HISPEED_WID             1
#define UARTAPP_STAT_PRESENT_WID             1

#define UARTAPP_DEBUG_RXDMARQ_WID            1
#define UARTAPP_DEBUG_TXDMARQ_WID            1
#define UARTAPP_DEBUG_RXCMDEND_WID           1
#define UARTAPP_DEBUG_TXCMDEND_WID           1
#define UARTAPP_DEBUG_RXDMARUN_WID           1
#define UARTAPP_DEBUG_TXDMARUN_WID           1
#define UARTAPP_DEBUG_RXFBAUD_DIV_WID        6
#define UARTAPP_DEBUG_RXIBAUD_DIV_WID        16

#define UARTAPP_VERSION_STEP_WID             16
#define UARTAPP_VERSION_MINOR_WID            8
#define UARTAPP_VERSION_MAJOR_WID            8

//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------
//CTRL2
#define UARTAPP_CTRL2_UARTEN_ENABLE          1
#define UARTAPP_CTRL2_UARTEN_DISABLE         0

#define UARTAPP_CTRL2_DTREN                              1
#define UARTAPP_CTRL2_DTRDEN                 0

//------------------------------------------------------------------------------
// REGISTER BIT MASK VALUES
//------------------------------------------------------------------------------
//CTRL0
#define BM_UARTAPP_CTRL0_SFTRST              0x80000000
#define BM_UARTAPP_CTRL0_CLKGATE             0x40000000
#define BM_UARTAPP_CTRL0_RUN                 0x20000000
#define BM_UARTAPP_CTRL0_RX_SOURCE           0x10000000
#define BM_UARTAPP_CTRL0_RXTO_ENABLE         0x08000000
#define BM_UARTAPP_CTRL0_RXTIMEOUT           0x07FF0000
#define BM_UARTAPP_CTRL0_XFER_COUNT          0x0000FFFF

//CTRL1
#define BM_UARTAPP_CTRL1_RUN                 0x10000000
#define BM_UARTAPP_CTRL1_XFER_COUNT          0x0000FFFF

//CTRL2
#define BM_UARTAPP_CTRL2_INVERT_RTS          0x80000000
#define BM_UARTAPP_CTRL2_INVERT_CTS          0x40000000
#define BM_UARTAPP_CTRL2_INVERT_TX           0x20000000
#define BM_UARTAPP_CTRL2_INVERT_RX           0x10000000
#define BM_UARTAPP_CTRL2_RTS_SEMAPHORE       0x08000000
#define BM_UARTAPP_CTRL2_DMAONERR            0x04000000
#define BM_UARTAPP_CTRL2_TXDMAE              0x02000000
#define BM_UARTAPP_CTRL2_RXDMAE              0x01000000
#define BM_UARTAPP_CTRL2_RXIFLSEL            0x00700000
#define BM_UARTAPP_CTRL2_TXIFLSEL            0x00070000
#define BM_UARTAPP_CTRL2_CTSEN               0x00008000
#define BM_UARTAPP_CTRL2_RTSEN               0x00004000
#define BM_UARTAPP_CTRL2_OUT2                0x00002000
#define BM_UARTAPP_CTRL2_OUT1                0x00001000
#define BM_UARTAPP_CTRL2_RTS                 0x00000800
#define BM_UARTAPP_CTRL2_DTR                 0x00000400
#define BM_UARTAPP_CTRL2_RXE                 0x00000200
#define BM_UARTAPP_CTRL2_TXE                 0x00000100
#define BM_UARTAPP_CTRL2_LBE                 0x00000080
#define BM_UARTAPP_CTRL2_USE_LCR2            0x00000040
#define BM_UARTAPP_CTRL2_SIRLP               0x00000004
#define BM_UARTAPP_CTRL2_SIREN               0x00000002
#define BM_UARTAPP_CTRL2_UARTEN              0x00000001

//LINECTRL
#define BM_UARTAPP_LINECTRL_BAUD_DIVINT      0xFFFF0000
#define BM_UARTAPP_LINECTRL_BAUD_DIVFRAC     0x00003F00
#define BM_UARTAPP_LINECTRL_SPS              0x00000080
#define BM_UARTAPP_LINECTRL_WLEN             0x00000060
#define BM_UARTAPP_LINECTRL_FEN              0x00000010
#define BM_UARTAPP_LINECTRL_STP2             0x00000008
#define BM_UARTAPP_LINECTRL_EPS              0x00000004
#define BM_UARTAPP_LINECTRL_PEN              0x00000002
#define BM_UARTAPP_LINECTRL_BRK              0x00000001

#define UARTAPP_LINECTRL_WLEN_5                                          0x00
#define UARTAPP_LINECTRL_WLEN_6                                          0x01
#define UARTAPP_LINECTRL_WLEN_7                                          0x02
#define UARTAPP_LINECTRL_WLEN_8                                          0x03


//LINECTRL2
#define BM_UARTAPP_LINECTRL2_BAUD_DIVINT     0xFFFF0000
#define BM_UARTAPP_LINECTRL2_BAUD_DIVFRAC    0x00003F00
#define BM_UARTAPP_LINECTRL2_SPS             0x00000080
#define BM_UARTAPP_LINECTRL2_WLEN            0x00000060
#define BM_UARTAPP_LINECTRL2_FEN             0x00000010
#define BM_UARTAPP_LINECTRL2_STP2            0x00000008
#define BM_UARTAPP_LINECTRL2_EPS             0x00000004
#define BM_UARTAPP_LINECTRL2_PEN             0x00000002

//INTR
#define BM_UARTAPP_INTR_OEIEN                0x04000000
#define BM_UARTAPP_INTR_BEIEN                0x02000000
#define BM_UARTAPP_INTR_PEIEN                0x01000000
#define BM_UARTAPP_INTR_FEIEN                0x00800000
#define BM_UARTAPP_INTR_RTIEN                0x00400000
#define BM_UARTAPP_INTR_TXIEN                0x00200000
#define BM_UARTAPP_INTR_RXIEN                0x00100000
#define BM_UARTAPP_INTR_DSRMIEN              0x00080000
#define BM_UARTAPP_INTR_DCDMIEN              0x00040000
#define BM_UARTAPP_INTR_CTSMIEN              0x00020000
#define BM_UARTAPP_INTR_RIMIEN               0x00010000
#define BM_UARTAPP_INTR_OEIS                 0x00000400
#define BM_UARTAPP_INTR_BEIS                 0x00000200
#define BM_UARTAPP_INTR_PEIS                 0x00000100
#define BM_UARTAPP_INTR_FEIS                 0x00000080
#define BM_UARTAPP_INTR_RTIS                 0x00000040
#define BM_UARTAPP_INTR_TXIS                 0x00000020
#define BM_UARTAPP_INTR_RXIS                 0x00000010
#define BM_UARTAPP_INTR_DSRMIS               0x00000008
#define BM_UARTAPP_INTR_DCDMIS               0x00000004
#define BM_UARTAPP_INTR_CTSMIS               0x00000002
#define BM_UARTAPP_INTR_RIMIS                0x00000001

//DATA
#define BM_UARTAPP_DATA_DATA                 0xFFFFFFFF

//STAT
#define BM_UARTAPP_STAT_PRESENT              0x80000000
#define BM_UARTAPP_STAT_HISPEED              0x40000000
#define BM_UARTAPP_STAT_BUSY                 0x20000000
#define BM_UARTAPP_STAT_CTS                  0x10000000
#define BM_UARTAPP_STAT_TXFE                 0x08000000
#define BM_UARTAPP_STAT_RXFF                 0x04000000
#define BM_UARTAPP_STAT_TXFF                 0x02000000
#define BM_UARTAPP_STAT_RXFE                 0x01000000
#define BM_UARTAPP_STAT_RXBYTE_INVALID       0x00F00000
#define BM_UARTAPP_STAT_OERR                 0x00080000
#define BM_UARTAPP_STAT_BERR                 0x00040000
#define BM_UARTAPP_STAT_PERR                 0x00020000
#define BM_UARTAPP_STAT_FERR                 0x00010000
#define BM_UARTAPP_STAT_RXCOUNT              0x0000FFFF

//DEBUG
#define BM_UARTAPP_DEBUG_TXDMARUN            0x00000020
#define BM_UARTAPP_DEBUG_RXDMARUN            0x00000010
#define BM_UARTAPP_DEBUG_TXCMDEND            0x00000008
#define BM_UARTAPP_DEBUG_RXCMDEND            0x00000004
#define BM_UARTAPP_DEBUG_TXDMARQ             0x00000002
#define BM_UARTAPP_DEBUG_RXDMARQ             0x00000001
#define BM_UARTAPP_DEBUG_RXFBAUD_DIV         0x0000FC00
#define BM_UARTAPP_DEBUG_RXIBAUD_DIV         0xFFFF0000

//VERSION
#define BM_UARTAPP_VERSION_MAJOR             0xFF000000
#define BM_UARTAPP_VERSION_MINOR             0x00FF0000
#define BM_UARTAPP_VERSION_STEP              0x0000FFFF

////////////////////////////////////////////////////////////////////////////////
//! \brief  Combines all the Rx error status bits into a convenient mask.
////////////////////////////////////////////////////////////////////////////////
#define ALL_RX_ERROR_MASK \
    ( BM_UARTAPP_STAT_OERR | \
      BM_UARTAPP_STAT_BERR | \
      BM_UARTAPP_STAT_PERR | \
      BM_UARTAPP_STAT_FERR )

////////////////////////////////////////////////////////////////////////////////
//! \brief  Combines all the interupt status bits into a convenient mask.
//!
//!   The following bits are not yet supported in hardware,
//!   so they are not included in the mask.
//!
//!     BM_UARTAPP_INTR_DSRMIS
//!     BM_UARTAPP_INTR_DCDMIS
//!     BM_UARTAPP_INTR_RIMIS
//!
//!   The following modem bit is not used at this time,
//!   so it is not added to the mask.
//!
//!     BM_UARTAPP_INTR_CTSMIS
////////////////////////////////////////////////////////////////////////////////
#define ALL_INT_STATUS_MASK \
    ( BM_UARTAPP_INTR_OEIS | \
      BM_UARTAPP_INTR_BEIS | \
      BM_UARTAPP_INTR_PEIS | \
      BM_UARTAPP_INTR_FEIS | \
      BM_UARTAPP_INTR_RTIS | \
      BM_UARTAPP_INTR_TXIS | \
      BM_UARTAPP_INTR_RXIS | \
      BM_UARTAPP_INTR_CTSMIS )

#define ALL_INT_ENABLE_MASK \
    ( BM_UARTAPP_INTR_OEIEN | \
      BM_UARTAPP_INTR_BEIEN | \
      BM_UARTAPP_INTR_PEIEN | \
      BM_UARTAPP_INTR_FEIEN | \
      BM_UARTAPP_INTR_RTIEN | \
      BM_UARTAPP_INTR_TXIEN | \
      BM_UARTAPP_INTR_RXIEN | \
      BM_UARTAPP_INTR_CTSMIEN )
////////////////////////////////////////////////////////////////////////////////
//! \brief  Enumeration of supported FIFO notification levels.
////////////////////////////////////////////////////////////////////////////////
typedef enum _SERHW_UART_APP_FILLLINE
{
    UART_APP_FIFO_EMPTY = 0,  //!< Defines a symbol for the 1/8 watermark hardware value.
    UART_APP_FIFO_1_4th = 1,  //!< Defines a symbol for the 1/4 watermark hardware value.
    UART_APP_FIFO_half  = 2,  //!< Defines a symbol for the 1/2 watermark hardware value.
    UART_APP_FIFO_3_4th = 3,  //!< Defines a symbol for the 3/4 watermark hardware value.
    UART_APP_FIFO_7_8th = 4   //!< Defines a symbol for the 7/8 watermark hardware value.
} SERHW_UART_APP_FILLLINE;

#ifdef __cplusplus
}
#endif

#endif // __STMPARM9_UARTAPP0_H


