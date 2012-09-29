//-----------------------------------------------------------------------------
//
// Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  Header:  uart16552.h
//
//  Provides definitions for external SC16C652 Dual UART module
//
//------------------------------------------------------------------------------
#ifndef __SC16C652_H
#define __SC16C652_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------
typedef union
{
    struct {
        UINT8 RHR_THR;
        UINT8 IER;
        UINT8 FCR_ISR;
        UINT8 LCR;
        UINT8 MCR;
        UINT8 LSR;
        UINT8 MSR;
        UINT8 SPR;
    } General;
    struct {
        UINT8 DLL;
        UINT8 DLM;
    } Special;
    struct {
        UINT8 reserved1;
        UINT8 reserved2;
        UINT8 EFR;
        UINT8 reserved3;
        UINT8 XON_1;
        UINT8 XOFF_1;
        UINT8 XON_2;
        UINT8 XOFF_2;
    } Enhanced;
} BSP_SC16C652_REGS, *PBSP_SC16C652_REGS;

//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------

// General Register Set
#define SC16C652_RHR_THR_OFFSET             0x0000
#define SC16C652_IER_OFFSET                 0x0001
#define SC16C652_FCR_ISR_OFFSET             0x0002
#define SC16C652_LCR_OFFSET                 0x0003
#define SC16C652_MCR_OFFSET                 0x0004
#define SC16C652_LSR_OFFSET                 0x0005
#define SC16C652_MSR_OFFSET                 0x0006
#define SC16C652_SPR_OFFSET                 0x0007

// Special Register Set
#define SC16C652_DLL_OFFSET                 0x0000
#define SC16C652_DLM_OFFSET                 0x0001

// Enhanced Register Set
#define SC16C652_EFR_OFFSET                 0x0002
#define SC16C652_XON_1_OFFSET               0x0004
#define SC16C652_XOFF_1_OFFSET              0x0005
#define SC16C652_XON_2_OFFSET               0x0006
#define SC16C652_XOFF_2_OFFSET              0x0007

//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------

//// General Register Set
// IER
#define SC16C652_IER_RHR_LSH                    0
#define SC16C652_IER_THR_INT_LSH                1
#define SC16C652_IER_RX_LINE_STATUS_INT_LSH     2
#define SC16C652_IER_MODEM_STATUS_INT_RX_LSH    3
#define SC16C652_IER_SLEEP_MODE_LSH             4
#define SC16C652_IER_XOFF_INT_LSH               5
#define SC16C652_IER_RTS_INT_LSH                6
#define SC16C652_IER_CTS_INT_LSH                7

// LCR
#define SC16C652_LCR_WORD_LEN_LSH               0
#define SC16C652_LCR_STOP_BITS_LSH              2
#define SC16C652_LCR_PARITY_EN_LSH              3
#define SC16C652_LCR_EVEN_PARITY_LSH            4
#define SC16C652_LCR_SET_PARITY_LSH             5
#define SC16C652_LCR_SET_BREAK_LSH              6
#define SC16C652_LCR_DIVISOR_LATCH_EN_LSH       7

// MCR
#define SC16C652_MCR_DTR_LSH                    0
#define SC16C652_MCR_RTS_LSH                    1
#define SC16C652_MCR_OP1_LSH                    2
#define SC16C652_MCR_OP2_INT_EN_LSH             3
#define SC16C652_MCR_LOOP_BACK_LSH              4
#define SC16C652_MCR_CLOCK_SELECT_LSH           7

// FCR
#define SC16C652_FCR_FIFO_EN_LSH                0
#define SC16C652_FCR_RCVR_FIFO_RESET_LSH        1
#define SC16C652_FCR_XMIT_FIFO_RESET_LSH        2
#define SC16C652_FCR_DMA_MODE_SELECT_LSH        3
#define SC16C652_FCR_TX_TRIGGER_LSH             4
#define SC16C652_FCR_RCVR_TRIGGER_EN_LSH        6

// FCR
#define SC16C652_LSR_RCVR_DATA_READY_LSH        0
#define SC16C652_LSR_OVERRUN_ERROR_LSH          1
#define SC16C652_LSR_PARITY_ERROR_LSH           2
#define SC16C652_LSR_FRAMING_ERROR_LSH          3
#define SC16C652_LSR_BREAK_INT_LSH              4
#define SC16C652_LSR_THR_EMPTY_LSH              5
#define SC16C652_LSR_THR_TSR_EMPTY_LSH          6
#define SC16C652_LSR_FIFO_DATA_ERROR_LSH        7


//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------

//// General Register Set
// IER
#define SC16C652_IER_RHR_WID                    1
#define SC16C652_IER_THR_INT_WID                1
#define SC16C652_IER_RX_LINE_STATUS_INT_WID     1
#define SC16C652_IER_MODEM_STATUS_INT_RX_WID    1
#define SC16C652_IER_SLEEP_MODE_WID             1
#define SC16C652_IER_XOFF_INT_WID               1
#define SC16C652_IER_RTS_INT_WID                1
#define SC16C652_IER_CTS_INT_WID                1

// LCR
#define SC16C652_LCR_WORD_LEN_WID               2
#define SC16C652_LCR_STOP_BITS_WID              1
#define SC16C652_LCR_PARITY_EN_WID              1
#define SC16C652_LCR_EVEN_PARITY_WID            1
#define SC16C652_LCR_SET_PARITY_WID             1
#define SC16C652_LCR_SET_BREAK_WID              1
#define SC16C652_LCR_DIVISOR_LATCH_EN_WID       1

// MCR
#define SC16C652_MCR_DTR_WID                    1
#define SC16C652_MCR_RTS_WID                    1
#define SC16C652_MCR_OP1_WID                    1
#define SC16C652_MCR_OP2_INT_EN_WID             1
#define SC16C652_MCR_LOOP_BACK_WID              1
#define SC16C652_MCR_CLOCK_SELECT_WID           1

// FCR
#define SC16C652_FCR_FIFO_EN_WID                1
#define SC16C652_FCR_RCVR_FIFO_RESET_WID        1
#define SC16C652_FCR_XMIT_FIFO_RESET_WID        1
#define SC16C652_FCR_DMA_MODE_SELECT_WID        1
#define SC16C652_FCR_TX_TRIGGER_WID             2
#define SC16C652_FCR_RCVR_TRIGGER_EN_WID        2

// FCR
#define SC16C652_LSR_RCVR_DATA_READY_WID        1
#define SC16C652_LSR_OVERRUN_ERROR_WID          1
#define SC16C652_LSR_PARITY_ERROR_WID           1
#define SC16C652_LSR_FRAMING_ERROR_WID          1
#define SC16C652_LSR_BREAK_INT_WID              1
#define SC16C652_LSR_THR_EMPTY_WID              1
#define SC16C652_LSR_THR_TSR_EMPTY_WID          1
#define SC16C652_LSR_FIFO_DATA_ERROR_WID        1


//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------
#define SC16C652_ENABLE                         1
#define SC16C652_DISABLE                        0

#define SC16C652_LCR_WORD_LEN_5                 0
#define SC16C652_LCR_WORD_LEN_6                 1
#define SC16C652_LCR_WORD_LEN_7                 2
#define SC16C652_LCR_WORD_LEN_8                 3

#define SC16C652_LCR_STOP_BITS_1                0
#define SC16C652_LCR_STOP_BITS_2                1

#ifdef __cplusplus
}
#endif

#endif // __SC16C652_H
