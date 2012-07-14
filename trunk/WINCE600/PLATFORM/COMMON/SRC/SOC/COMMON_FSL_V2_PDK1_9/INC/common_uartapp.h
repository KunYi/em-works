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
//  Header:  COMMON_UARTAPP.h
//
//  Provides definitions for UARTAPP module.
//
//------------------------------------------------------------------------------

#ifndef __COMMON_UARTAPP_H
#define __COMMON_UARTAPP_H

#if __cplusplus
extern "C" {
#endif

#ifndef REGS_UARTAPP_BASE
#define REGS_UARTAPP0_BASE         (DWORD)pHWHead->pv_HWregUARTApp0
#define REGS_UARTAPP1_BASE         (DWORD)pHWHead->pv_HWregUARTApp1
#define REGS_UARTAPP2_BASE         (DWORD)pHWHead->pv_HWregUARTApp2
#define REGS_UARTAPP3_BASE         (DWORD)pHWHead->pv_HWregUARTApp3
#define REGS_UARTAPP4_BASE         (DWORD)pHWHead->pv_HWregUARTApp4

#define REGS_UARTAPP_BASE(x) ( x == 0 ? REGS_UARTAPP0_BASE : x == 1 ? REGS_UARTAPP1_BASE : x == 2 ? REGS_UARTAPP2_BASE : \
							   x == 3 ? REGS_UARTAPP3_BASE : x == 4 ? REGS_UARTAPP4_BASE : 0xffff0000)
#endif

//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------
#define UARTAPP_RXFIFO_DEPTH       32
#define UARTAPP_TXFIFO_DEPTH       32

#define UARTAPP_CLOCK_FREQUENCY    (24000000)

#define BUILD_UARTAPP_BAUDRATE_DIVIDER(b)       ((UARTAPP_CLOCK_FREQUENCY * 32) / (b))
#define GET_UARTAPP_BAUD_DIVINT(b)              ((BUILD_UARTAPP_BAUDRATE_DIVIDER(b)) >> 6)
#define GET_UARTAPP_BAUD_DIVFRAC(b)             ((BUILD_UARTAPP_BAUDRATE_DIVIDER(b)) & 0x3F)

//divisor must be between 0xEC and 0x3FFFC0, inclusive.
#define MIN_UARTAPP_DIVISOR        0xEC       // Minimum UART XCLK divisor constant.
#define MAX_UARTAPP_DIVISOR        0x3FFFC0   // Maximum UART XCLK divisor constant.

// error status in DATA FIFO
#define FIFO_FRAME_ERROR           0x100
#define FIFO_PARITY_ERROR          0x200
#define FIFO_BREAK_ERROR           0x400
#define FIFO_OVERRUN_ERROR         0x800
#define FIFO_ERROR                 0xF00

// invalid received data
#define INVALID_DATA               0x100000

#define ALL_RX_ERROR_MASK        ( BM_UARTAPPSTAT_OERR | \
                                   BM_UARTAPPSTAT_BERR | \
                                   BM_UARTAPPSTAT_PERR | \
                                   BM_UARTAPPSTAT_FERR )


#define ALL_INT_STATUS_MASK      ( BM_UARTAPPINTR_OEIS | \
                                   BM_UARTAPPINTR_BEIS | \
                                   BM_UARTAPPINTR_PEIS | \
                                   BM_UARTAPPINTR_FEIS | \
                                   BM_UARTAPPINTR_RTIS | \
                                   BM_UARTAPPINTR_TXIS | \
                                   BM_UARTAPPINTR_RXIS | \
                                   BM_UARTAPPINTR_CTSMIS )

#define ALL_INT_ENABLE_MASK      ( BM_UARTAPPINTR_OEIEN | \
                                   BM_UARTAPPINTR_BEIEN | \
                                   BM_UARTAPPINTR_PEIEN | \
                                   BM_UARTAPPINTR_FEIEN | \
                                   BM_UARTAPPINTR_RTIEN | \
                                   BM_UARTAPPINTR_TXIEN | \
                                   BM_UARTAPPINTR_RXIEN | \
                                   BM_UARTAPPINTR_CTSMIEN )

#define UARTAPP_LINECTRL_WLEN_5    0
#define UARTAPP_LINECTRL_WLEN_6    1
#define UARTAPP_LINECTRL_WLEN_7    2
#define UARTAPP_LINECTRL_WLEN_8    3

#define UART_APP_FIFO_ONE_EIGHT    0
#define UART_APP_FIFO_ONE_QUARTER  1
#define UART_APP_FIFO_ONE_HALF     2
#define UART_APP_FIFO_THREE_QUARTERS 3


//------------------------------------------------------------------------------
//   HW_UARTAPPCTRL0 - UART Receive DMA Register
//------------------------------------------------------------------------------
#ifndef __LANGUAGE_ASM__
typedef union
{
    reg32_t  U;
    struct
    {
        unsigned XFER_COUNT      :  16;
        unsigned RXTIMEOUT       :  11;
        unsigned RXTO_ENABLE     :  1;
        unsigned RX_SOURCE       :  1;
        unsigned RUN             :  1;
        unsigned CLKGATE         :  1;
        unsigned SFTRST          :  1;
    } B;
} hw_uartappctrl0_t;
#endif

//constants & macros for entire HW_UARTAPPCTRL0 register

#define HW_UARTAPPCTRL0_ADDR(x)        (REGS_UARTAPP_BASE(x) + 0x0)
#define HW_UARTAPPCTRL0_SET_ADDR(x)    (HW_UARTAPPCTRL0_ADDR(x) + 0x4)
#define HW_UARTAPPCTRL0_CLR_ADDR(x)    (HW_UARTAPPCTRL0_ADDR(x) + 0x8)
#define HW_UARTAPPCTRL0_TOG_ADDR(x)    (HW_UARTAPPCTRL0_ADDR(x) + 0xC)

#ifndef __LANGUAGE_ASM__
#define HW_UARTAPPCTRL0(x)             (*(volatile hw_uartappctrl0_t *) HW_UARTAPPCTRL0_ADDR(x))
#define HW_UARTAPPCTRL0_RD(x)          (HW_UARTAPPCTRL0(x).U)
#define HW_UARTAPPCTRL0_WR(x, v)       (HW_UARTAPPCTRL0(x).U = (v))
#define HW_UARTAPPCTRL0_SET(x, v)      ((*(volatile reg32_t *) HW_UARTAPPCTRL0_SET_ADDR(x)) = (v))
#define HW_UARTAPPCTRL0_CLR(x, v)      ((*(volatile reg32_t *) HW_UARTAPPCTRL0_CLR_ADDR(x)) = (v))
#define HW_UARTAPPCTRL0_TOG(x, v)      ((*(volatile reg32_t *) HW_UARTAPPCTRL0_TOG_ADDR(x)) = (v))
#endif

//constants & macros for individual HW_UARTAPPCTRL0 bitfields

// Register HW_UARTAPPCTRL0, field SFTRST
#define BP_UARTAPPCTRL0_SFTRST         31
#define BM_UARTAPPCTRL0_SFTRST         0x80000000

//  Register HW_UARTAPPCTRL0, field CLKGATE 
#define BP_UARTAPPCTRL0_CLKGATE        30
#define BM_UARTAPPCTRL0_CLKGATE        0x40000000

//  Register HW_UARTAPPCTRL0, field RUN
#define BP_UARTAPPCTRL0_RUN            29
#define BM_UARTAPPCTRL0_RUN            0x20000000

//  Register HW_UARTAPPCTRL0, field RX_SOURCE
#define BP_UARTAPPCTRL0_RX_SOURCE      28
#define BM_UARTAPPCTRL0_RX_SOURCE      0x10000000

//  Register HW_UARTAPPCTRL0, field RXTO_ENABLE
#define BP_UARTAPPCTRL0_RXTO_ENABLE    27
#define BM_UARTAPPCTRL0_RXTO_ENABLE    0x08000000

//  Register HW_UARTAPPCTRL0, field RXTIMEOUT 
#define BP_UARTAPPCTRL0_RXTIMEOUT      16
#define BM_UARTAPPCTRL0_RXTIMEOUT      0x07FF0000
#define BF_UARTAPPCTRL0_RXTIMEOUT(v)   (((v) << 16) & BM_UARTAPPCTRL0_RXTIMEOUT)

//  Register HW_UARTAPPCTRL0, field DATA
#define BP_UARTAPPCTRL0_XFER_COUNT     0
#define BM_UARTAPPCTRL0_XFER_COUNT     0x0000FFFF
#define BF_UARTAPPCTRL0_XFER_COUNT(v)  (((v) << 0) & BM_UARTAPPCTRL0_XFER_COUNT)


//------------------------------------------------------------------------------
//   HW_UARTAPPCTRL1 - UART Transmit DMA Register
//------------------------------------------------------------------------------
#ifndef __LANGUAGE_ASM__
typedef union
{
    reg32_t  U;
    struct
    {
        unsigned XFER_COUNT      :  16;
        unsigned RSVD1           :  12;
        unsigned RUN             :  1;
        unsigned RSVD2           :  3;
    } B;
} hw_uartappctrl1_t;
#endif

//constants & macros for entire HW_UARTAPPCTRL1 register

#define HW_UARTAPPCTRL1_ADDR(x)        (REGS_UARTAPP_BASE(x) + 0x10)
#define HW_UARTAPPCTRL1_SET_ADDR(x)    (HW_UARTAPPCTRL1_ADDR(x) + 0x4)
#define HW_UARTAPPCTRL1_CLR_ADDR(x)    (HW_UARTAPPCTRL1_ADDR(x) + 0x8)
#define HW_UARTAPPCTRL1_TOG_ADDR(x)    (HW_UARTAPPCTRL1_ADDR(x) + 0xC)


#ifndef __LANGUAGE_ASM__
#define HW_UARTAPPCTRL1(x)             (*(volatile hw_uartappctrl1_t *) HW_UARTAPPCTRL1_ADDR(x))
#define HW_UARTAPPCTRL1_RD(x)          (HW_UARTAPPCTRL1(x).U)
#define HW_UARTAPPCTRL1_WR(x, v)       (HW_UARTAPPCTRL1(x).U = (v))
#define HW_UARTAPPCTRL1_SET(x, v)      ((*(volatile reg32_t *) HW_UARTAPPCTRL1_SET_ADDR(x)) = (v))
#define HW_UARTAPPCTRL1_CLR(x, v)      ((*(volatile reg32_t *) HW_UARTAPPCTRL1_CLR_ADDR(x)) = (v))
#define HW_UARTAPPCTRL1_TOG(x, v)      ((*(volatile reg32_t *) HW_UARTAPPCTRL1_TOG_ADDR(x)) = (v))
#endif

//constants & macros for individual HW_UARTAPPCTRL1 bitfields

// Register HW_UARTAPPCTRL1, field RUN
#define BP_UARTAPPCTRL1_RSVD2          29
#define BM_UARTAPPCTRL1_RSVD2          0xE0000000

// Register HW_UARTAPPCTRL1, field RUN
#define BP_UARTAPPCTRL1_RUN            28
#define BM_UARTAPPCTRL1_RUN            0x10000000

//  Register HW_UARTAPPCTRL1, field RSVD1 
#define BP_UARTAPPCTRL1_RSVD1          16
#define BM_UARTAPPCTRL1_RSVD1          0x0FFF0000

//  Register HW_UARTAPPCTRL1, field XFER_COUNT
#define BP_UARTAPPCTRL1_XFER_COUNT     0
#define BM_UARTAPPCTRL1_XFER_COUNT     0x0000FFFF
#define BF_UARTAPPCTRL1_XFER_COUNT(v)  (((v) << 0) & BM_UARTAPPCTRL1_XFER_COUNT)


//------------------------------------------------------------------------------
//   HW_UARTAPPCTRL2 - UART Control Register
//------------------------------------------------------------------------------
#ifndef __LANGUAGE_ASM__
typedef union
{
    reg32_t  U;
    struct
    {
        unsigned UARTEN        :  1;
        unsigned SIREN         :  1;
        unsigned SIRLP         :  1;
        unsigned RSVD4         :  3;
        unsigned USE_LCR2      :  1;
        unsigned LBE           :  1;
        unsigned TXE           :  1;
        unsigned RXE           :  1;
        unsigned DTR           :  1;
        unsigned RTS           :  1;
        unsigned OUT1          :  1;
        unsigned OUT2          :  1;
        unsigned RTSEN         :  1;
        unsigned CTSEN         :  1;
        unsigned TXIFLSEL      :  3;
        unsigned RSVD3         :  1;
        unsigned RXIFLSEL      :  3;
        unsigned RSVD2         :  1;
        unsigned RXDMAE        :  1;
        unsigned TXDMAE        :  1;
        unsigned DMAONERR      :  1;
        unsigned RTS_SEMAPHONE :  1;
        unsigned INVERT_RX     :  1;
        unsigned INVERT_TX     :  1;
        unsigned INVERT_CTS    :  1;
        unsigned INVERT_RTS    :  1;
    } B;
} hw_uartappctrl2_t;
#endif

//constants & macros for entire HW_UARTAPPCTRL2 register

#define HW_UARTAPPCTRL2_ADDR(x)        (REGS_UARTAPP_BASE(x) + 0x20)
#define HW_UARTAPPCTRL2_SET_ADDR(x)    (HW_UARTAPPCTRL2_ADDR(x) + 0x4)
#define HW_UARTAPPCTRL2_CLR_ADDR(x)    (HW_UARTAPPCTRL2_ADDR(x) + 0x8)
#define HW_UARTAPPCTRL2_TOG_ADDR(x)    (HW_UARTAPPCTRL2_ADDR(x) + 0xC)

#ifndef __LANGUAGE_ASM__
#define HW_UARTAPPCTRL2(x)             (*(volatile hw_uartappctrl2_t *) HW_UARTAPPCTRL2_ADDR(x))
#define HW_UARTAPPCTRL2_RD(x)          (HW_UARTAPPCTRL2(x).U)
#define HW_UARTAPPCTRL2_WR(x, v)       (HW_UARTAPPCTRL2(x).U = (v))
#define HW_UARTAPPCTRL2_SET(x, v)      ((*(volatile reg32_t *) HW_UARTAPPCTRL2_SET_ADDR(x)) = (v))
#define HW_UARTAPPCTRL2_CLR(x, v)      ((*(volatile reg32_t *) HW_UARTAPPCTRL2_CLR_ADDR(x)) = (v))
#define HW_UARTAPPCTRL2_TOG(x, v)      ((*(volatile reg32_t *) HW_UARTAPPCTRL2_TOG_ADDR(x)) = (v))
#endif

//constants & macros for individual HW_UARTAPPCTRL2 bitfields

// Register HW_UARTAPPCTRL2, field RUN
#define BP_UARTAPPCTRL2_INVERT_RTS      31
#define BM_UARTAPPCTRL2_INVERT_RTS      0x80000000

// Register HW_UARTAPPCTRL2, field INVERT_CTS
#define BP_UARTAPPCTRL2_INVERT_CTS      30
#define BM_UARTAPPCTRL2_INVERT_CTS      0x40000000

//  Register HW_UARTAPPCTRL2, field INVERT_TX
#define BP_UARTAPPCTRL2_INVERT_TX       29
#define BM_UARTAPPCTRL2_INVERT_TX       0x20000000

//  Register HW_UARTAPPCTRL2, field INVERT_RX
#define BP_UARTAPPCTRL2_INVERT_RX       28
#define BM_UARTAPPCTRL2_INVERT_RX       0x10000000

//  Register HW_UARTAPPCTRL2, field RTS_SEMAPHONE
#define BP_UARTAPPCTRL2_RTS_SEMAPHONE   27
#define BM_UARTAPPCTRL2_RTS_SEMAPHONE   0x08000000

//  Register HW_UARTAPPCTRL2, field DMAONERR
#define BP_UARTAPPCTRL2_DMAONERR        26
#define BM_UARTAPPCTRL2_DMAONERR        0x04000000

//  Register HW_UARTAPPCTRL2, field RTS_SEMAPHONE
#define BP_UARTAPPCTRL2_TXDMAE          25
#define BM_UARTAPPCTRL2_TXDMAE          0x02000000

//  Register HW_UARTAPPCTRL2, field RXDMAE
#define BP_UARTAPPCTRL2_RXDMAE          24
#define BM_UARTAPPCTRL2_RXDMAE          0x01000000

//  Register HW_UARTAPPCTRL2, field RSVD2
#define BP_UARTAPPCTRL2_RSVD2           23
#define BM_UARTAPPCTRL2_RSVD2           0x00800000

//  Register HW_UARTAPPCTRL2, field RXIFLSEL
#define BP_UARTAPPCTRL2_RXIFLSEL        20
#define BM_UARTAPPCTRL2_RXIFLSEL        0x00700000
#define BF_UARTAPPCTRL2_RXIFLSEL(v)     (((v) << 20) & BM_UARTAPPCTRL2_RXIFLSEL)

//  Register HW_UARTAPPCTRL2, field RSVD3
#define BP_UARTAPPCTRL2_RSVD3           19
#define BM_UARTAPPCTRL2_RSVD3           0x00080000

//  Register HW_UARTAPPCTRL2, field TXIFLSEL
#define BP_UARTAPPCTRL2_TXIFLSEL        16
#define BM_UARTAPPCTRL2_TXIFLSEL        0x00070000
#define BF_UARTAPPCTRL2_TXIFLSEL(v)     (((v) << 16) & BM_UARTAPPCTRL2_TXIFLSEL)

//  Register HW_UARTAPPCTRL2, field CTSEN
#define BP_UARTAPPCTRL2_CTSEN           15
#define BM_UARTAPPCTRL2_CTSEN           0x00008000

//  Register HW_UARTAPPCTRL2, field RTSEN
#define BP_UARTAPPCTRL2_RTSEN           14
#define BM_UARTAPPCTRL2_RTSEN           0x00004000

//  Register HW_UARTAPPCTRL2, field OUT2
#define BP_UARTAPPCTRL2_OUT2            13
#define BM_UARTAPPCTRL2_OUT2            0x00002000

//  Register HW_UARTAPPCTRL2, field OUT1
#define BP_UARTAPPCTRL2_OUT1            12
#define BM_UARTAPPCTRL2_OUT1            0x00001000

//  Register HW_UARTAPPCTRL2, field RTS
#define BP_UARTAPPCTRL2_RTS             11
#define BM_UARTAPPCTRL2_RTS             0x00000800

//  Register HW_UARTAPPCTRL2, field DTR
#define BP_UARTAPPCTRL2_DTR             10
#define BM_UARTAPPCTRL2_DTR             0x00000400

//  Register HW_UARTAPPCTRL2, field RXE
#define BP_UARTAPPCTRL2_RXE             9
#define BM_UARTAPPCTRL2_RXE             0x00000100

//  Register HW_UARTAPPCTRL2, field TXE
#define BP_UARTAPPCTRL2_TXE             8
#define BM_UARTAPPCTRL2_TXE             0x00000100

//  Register HW_UARTAPPCTRL2, field LBE
#define BP_UARTAPPCTRL2_LBE             7
#define BM_UARTAPPCTRL2_LBE             0x00000080

//  Register HW_UARTAPPCTRL2, field USE_LCR2
#define BP_UARTAPPCTRL2_USE_LCR2        6
#define BM_UARTAPPCTRL2_USE_LCR2        0x00000040

//  Register HW_UARTAPPCTRL2, field RSVD4
#define BP_UARTAPPCTRL2_RSVD4           3
#define BM_UARTAPPCTRL2_RSVD4           0x00000038

//  Register HW_UARTAPPCTRL2, field SIRLP
#define BP_UARTAPPCTRL2_SIRLP           2
#define BM_UARTAPPCTRL2_SIRLP           0x00000004

//  Register HW_UARTAPPCTRL2, field SIREN
#define BP_UARTAPPCTRL2_SIREN           1
#define BM_UARTAPPCTRL2_SIREN           0x00000002

//  Register HW_UARTAPPCTRL2, field UARTEN
#define BP_UARTAPPCTRL2_UARTEN          0
#define BM_UARTAPPCTRL2_UARTEN          0x000000001


//------------------------------------------------------------------------------
//   HW_UARTAPPLINECTRL - UART Line Control Register
//------------------------------------------------------------------------------
#ifndef __LANGUAGE_ASM__
typedef union
{
    reg32_t  U;
    struct
    {
        unsigned BRK           :  1;
        unsigned PEN           :  1;
        unsigned EPS           :  1;
        unsigned STP2          :  1;
        unsigned FEN           :  1;
        unsigned WLEN          :  2;
        unsigned SPS           :  1;
        unsigned BAUD_DIVFRAC  :  6;
        unsigned RSVD          :  2;
        unsigned BAUD_DIVINT   :  16;
    } B;
} hw_uartapplinectrl_t;
#endif

//constants & macros for entire HW_UARTAPPLINECTRL register

#define HW_UARTAPPLINECTRL_ADDR(x)        (REGS_UARTAPP_BASE(x) + 0x30)
#define HW_UARTAPPLINECTRL_SET_ADDR(x)    (HW_UARTAPPLINECTRL_ADDR(x) + 0x4)
#define HW_UARTAPPLINECTRL_CLR_ADDR(x)    (HW_UARTAPPLINECTRL_ADDR(x) + 0x8)
#define HW_UARTAPPLINECTRL_TOG_ADDR(x)    (HW_UARTAPPLINECTRL_ADDR(x) + 0xC)

#ifndef __LANGUAGE_ASM__
#define HW_UARTAPPLINECTRL(x)             (*(volatile hw_uartapplinectrl_t *) HW_UARTAPPLINECTRL_ADDR(x))
#define HW_UARTAPPLINECTRL_RD(x)          (HW_UARTAPPLINECTRL(x).U)
#define HW_UARTAPPLINECTRL_WR(x, v)       (HW_UARTAPPLINECTRL(x).U = (v))
#define HW_UARTAPPLINECTRL_SET(x, v)      ((*(volatile reg32_t *) HW_UARTAPPLINECTRL_SET_ADDR(x)) = (v))
#define HW_UARTAPPLINECTRL_CLR(x, v)      ((*(volatile reg32_t *) HW_UARTAPPLINECTRL_CLR_ADDR(x)) = (v))
#define HW_UARTAPPLINECTRL_TOG(x, v)      ((*(volatile reg32_t *) HW_UARTAPPLINECTRL_TOG_ADDR(x)) = (v))
#endif

//constants & macros for individual HW_UARTAPPLINECTRL bitfields

// Register HW_UARTAPPLINECTRL, field BAUD_DIVINT
#define BP_UARTAPPLINECTRL_BAUD_DIVINT    16
#define BM_UARTAPPLINECTRL_BAUD_DIVINT    0xFFFF0000
#define BF_UARTAPPLINECTRL_BAUD_DIVINT(v) (((v) << 16) & BM_UARTAPPLINECTRL_BAUD_DIVINT)

// Register HW_UARTAPPCTRL2, field RSVD
#define BP_UARTAPPLINECTRL_RSVD           14
#define BM_UARTAPPLINECTRL_RSVD           0x0000C000

//  Register HW_UARTAPPLINECTRL, field BAUD_DIVFRAC
#define BP_UARTAPPLINECTRL_BAUD_DIVFRAC   8
#define BM_UARTAPPLINECTRL_BAUD_DIVFRAC   0x00003F00
#define BF_UARTAPPLINECTRL_BAUD_DIVFRAC(v) (((v) << 8) & BM_UARTAPPLINECTRL_BAUD_DIVFRAC)

//  Register HW_UARTAPPLINECTRL, field SPS
#define BP_UARTAPPLINECTRL_SPS            7
#define BM_UARTAPPLINECTRL_SPS            0x00000080

//  Register HW_UARTAPPLINECTRL, field WLEN
#define BP_UARTAPPLINECTRL_WLEN           5
#define BM_UARTAPPLINECTRL_WLEN           0x00000060
#define BF_UARTAPPLINECTRL_WLEN(v)        (((v) << 5) & BM_UARTAPPLINECTRL_WLEN)

//  Register HW_UARTAPPLINECTRL, field FEN
#define BP_UARTAPPLINECTRL_FEN            4
#define BM_UARTAPPLINECTRL_FEN            0x00000010

//  Register HW_UARTAPPLINECTRL, field STP2
#define BP_UARTAPPLINECTRL_STP2           3
#define BM_UARTAPPLINECTRL_STP2           0x00000008

//  Register HW_UARTAPPLINECTRL, field EPS
#define BP_UARTAPPLINECTRL_EPS            2
#define BM_UARTAPPLINECTRL_EPS            0x00000004

//  Register HW_UARTAPPLINECTRL, field PEN
#define BP_UARTAPPLINECTRL_PEN            1
#define BM_UARTAPPLINECTRL_PEN            0x00000002

//  Register HW_UARTAPPLINECTRL, field BRK
#define BP_UARTAPPLINECTRL_BRK            0
#define BM_UARTAPPLINECTRL_BRK            0x00000001


//------------------------------------------------------------------------------
//   HW_UARTAPPLINECTRL2 - UART Line Control2 Register
//------------------------------------------------------------------------------
#ifndef __LANGUAGE_ASM__
typedef union
{
    reg32_t  U;
    struct
    {
        unsigned RSVD1         :  1;
        unsigned PEN           :  1;
        unsigned EPS           :  1;
        unsigned STP2          :  1;
        unsigned FEN           :  1;
        unsigned WLEN          :  2;
        unsigned SPS           :  1;
        unsigned BAUD_DIVFRAC  :  6;
        unsigned RSVD          :  2;
        unsigned BAUD_DIVINT   :  16;
    } B;
} hw_uartapplinectrl2_t;
#endif

//constants & macros for entire HW_UARTAPPLINECTRL2 register

#define HW_UARTAPPLINECTRL2_ADDR(x)        (REGS_UARTAPP_BASE(x) + 0x40)
#define HW_UARTAPPLINECTRL2_SET_ADDR(x)    (HW_UARTAPPLINECTRL2_ADDR(x) + 0x4)
#define HW_UARTAPPLINECTRL2_CLR_ADDR(x)    (HW_UARTAPPLINECTRL2_ADDR(x) + 0x8)
#define HW_UARTAPPLINECTRL2_TOG_ADDR(x)    (HW_UARTAPPLINECTRL2_ADDR(x) + 0xC)

#ifndef __LANGUAGE_ASM__
#define HW_UARTAPPLINECTRL2(x)             (*(volatile hw_uartapplinectrl2_t *) HW_UARTAPPLINECTRL2_ADDR(x))
#define HW_UARTAPPLINECTRL2_RD(x)          (HW_UARTAPPLINECTRL2(x).U)
#define HW_UARTAPPLINECTRL2_WR(x, v)       (HW_UARTAPPLINECTRL2(x).U = (v))
#define HW_UARTAPPLINECTRL2_SET(x, v)      ((*(volatile reg32_t *) HW_UARTAPPLINECTRL2_SET_ADDR(x)) = (v))
#define HW_UARTAPPLINECTRL2_CLR(x, v)      ((*(volatile reg32_t *) HW_UARTAPPLINECTRL2_CLR_ADDR(x)) = (v))
#define HW_UARTAPPLINECTRL2_TOG(x, v)      ((*(volatile reg32_t *) HW_UARTAPPLINECTRL2_TOG_ADDR(x)) = (v))
#endif

//constants & macros for individual HW_UARTAPPLINECTRL2 bitfields

// Register HW_UARTAPPLINECTRL2, field BAUD_DIVINT
#define BP_UARTAPPLINECTRL2_BAUD_DIVINT    16
#define BM_UARTAPPLINECTRL2_BAUD_DIVINT    0xFFFF0000
#define BF_UARTAPPLINECTRL2_BAUD_DIVINT(v) (((v) << 16) & BM_UARTAPPLINECTRL2_BAUD_DIVINT)

// Register HW_UARTAPPCTRL2, field RSVD
#define BP_UARTAPPLINECTRL2_RSVD           14
#define BM_UARTAPPLINECTRL2_RSVD           0x0000C000

//  Register HW_UARTAPPLINECTRL2, field BAUD_DIVFRAC
#define BP_UARTAPPLINECTRL2_BAUD_DIVFRAC   8
#define BM_UARTAPPLINECTRL2_BAUD_DIVFRAC   0x00003F00
#define BF_UARTAPPLINECTRL2_BAUD_DIVFRAC(v) (((v) << 8) & BM_UARTAPPLINECTRL2_BAUD_DIVFRAC)

//  Register HW_UARTAPPLINECTRL2, field SPS
#define BP_UARTAPPLINECTRL2_SPS            7
#define BM_UARTAPPLINECTRL2_SPS            0x00000080

//  Register HW_UARTAPPLINECTRL2, field WLEN
#define BP_UARTAPPLINECTRL2_WLEN           5
#define BM_UARTAPPLINECTRL2_WLEN           0x00000060
#define BF_UARTAPPLINECTRL2_WLEN(v)        (((v) << 5) & BM_UARTAPPLINECTRL2_WLEN)

//  Register HW_UARTAPPLINECTRL2, field FEN
#define BP_UARTAPPLINECTRL2_FEN            4
#define BM_UARTAPPLINECTRL2_FEN            0x00000010

//  Register HW_UARTAPPLINECTRL2, field STP2
#define BP_UARTAPPLINECTRL2_STP2           3
#define BM_UARTAPPLINECTRL2_STP2           0x00000008

//  Register HW_UARTAPPLINECTRL2, field EPS
#define BP_UARTAPPLINECTRL2_EPS            2
#define BM_UARTAPPLINECTRL2_EPS            0x00000004

//  Register HW_UARTAPPLINECTRL2, field PEN
#define BP_UARTAPPLINECTRL2_PEN            1
#define BM_UARTAPPLINECTRL2_PEN            0x00000002

//  Register HW_UARTAPPLINECTRL2, field RSVD1
#define BP_UARTAPPLINECTRL2_RSVD1          0
#define BM_UARTAPPLINECTRL2_RSVD1          0x00000001


//------------------------------------------------------------------------------
//   HW_UARTAPPINTR - UART Interrupt Register
//------------------------------------------------------------------------------
#ifndef __LANGUAGE_ASM__
typedef union
{
    reg32_t  U;
    struct
    {
        unsigned RIMIS         :  1;
        unsigned CTSMIS        :  1;
        unsigned DCDMIS        :  1;
        unsigned DSRMIS        :  1;
        unsigned RXIS          :  1;
        unsigned TXIS          :  1;
        unsigned RTIS          :  1;
        unsigned FEIS          :  1;
        unsigned PEIS          :  1;
        unsigned BEIS          :  1;
        unsigned OEIS          :  1;
        unsigned ABDIS         :  1;
        unsigned RSVD2         :  4;
        unsigned RIMIEN        :  1;
        unsigned CTSMIEN       :  1;
        unsigned DCDMIEN       :  1;
        unsigned DSRMIEN       :  1;
        unsigned RXIEN         :  1;
        unsigned TXIEN         :  1;
        unsigned RTIEN         :  1;
        unsigned FEIEN         :  1;
        unsigned PEIEN         :  1;
        unsigned BEIEN         :  1;
        unsigned OEIEN         :  1;
        unsigned ABDIEN        :  1;
        unsigned RSVD1         :  4;
    } B;
} hw_uartappintr_t;
#endif

//constants & macros for entire HW_UARTAPPINTR register

#define HW_UARTAPPINTR_ADDR(x)       (REGS_UARTAPP_BASE(x) + 0x50)
#define HW_UARTAPPINTR_SET_ADDR(x)   (HW_UARTAPPINTR_ADDR(x) + 0x4)
#define HW_UARTAPPINTR_CLR_ADDR(x)   (HW_UARTAPPINTR_ADDR(x) + 0x8)
#define HW_UARTAPPINTR_TOG_ADDR(x)   (HW_UARTAPPINTR_ADDR(x) + 0xC)

#ifndef __LANGUAGE_ASM__
#define HW_UARTAPPINTR(x)            (*(volatile hw_uartappintr_t *) HW_UARTAPPINTR_ADDR(x))
#define HW_UARTAPPINTR_RD(x)         (HW_UARTAPPINTR(x).U)
#define HW_UARTAPPINTR_WR(x, v)      (HW_UARTAPPINTR(x).U = (v))
#define HW_UARTAPPINTR_SET(x, v)     ((*(volatile reg32_t *) HW_UARTAPPINTR_SET_ADDR(x)) = (v))
#define HW_UARTAPPINTR_CLR(x, v)     ((*(volatile reg32_t *) HW_UARTAPPINTR_CLR_ADDR(x)) = (v))
#define HW_UARTAPPINTR_TOG(x, v)     ((*(volatile reg32_t *) HW_UARTAPPINTR_TOG_ADDR(x)) = (v))
#endif

//constants & macros for individual HW_UARTAPPINTR bitfields

// Register HW_UARTAPPINTR, field RSVD1
#define BP_UARTAPPINTR_RSVD1         28
#define BM_UARTAPPINTR_RSVD1         0xF0000000

// Register HW_UARTAPPINTR, field ABDIEN
#define BP_UARTAPPINTR_ABDIEN        27
#define BM_UARTAPPINTR_ABDIEN        0x08000000

//  Register HW_UARTAPPINTR, field OEIEN
#define BP_UARTAPPINTR_OEIEN         26
#define BM_UARTAPPINTR_OEIEN         0x04000000

//  Register HW_UARTAPPINTR, field BEIEN
#define BP_UARTAPPINTR_BEIEN         25
#define BM_UARTAPPINTR_BEIEN         0x02000000

// Register HW_UARTAPPINTR, field PEIEN
#define BP_UARTAPPINTR_PEIEN         24
#define BM_UARTAPPINTR_PEIEN         0x01000000

// Register HW_UARTAPPINTR, field FEIEN
#define BP_UARTAPPINTR_FEIEN         23
#define BM_UARTAPPINTR_FEIEN         0x00800000

//  Register HW_UARTAPPINTR, field RTIEN
#define BP_UARTAPPINTR_RTIEN         22
#define BM_UARTAPPINTR_RTIEN         0x00400000

//  Register HW_UARTAPPINTR, field TXIEN
#define BP_UARTAPPINTR_TXIEN         21
#define BM_UARTAPPINTR_TXIEN         0x00200000

// Register HW_UARTAPPINTR, field RXIEN
#define BP_UARTAPPINTR_RXIEN         20
#define BM_UARTAPPINTR_RXIEN         0x00100000

// Register HW_UARTAPPINTR, field DSRMIEN
#define BP_UARTAPPINTR_DSRMIEN       19
#define BM_UARTAPPINTRL_DSRMIEN      0x00080000

//  Register HW_UARTAPPINTR, field DCDMIEN
#define BP_UARTAPPINTR_DCDMIEN       18
#define BM_UARTAPPINTR_DCDMIEN       0x00040000

// Register HW_UARTAPPINTR, field CTSMIEN
#define BP_UARTAPPINTR_CTSMIEN       17
#define BM_UARTAPPINTR_CTSMIEN       0x00020000

//  Register HW_UARTAPPINTR, field RIMIEN
#define BP_UARTAPPINTR_RIMIEN        16
#define BM_UARTAPPINTR_RIMIEN        0x00010000

//  Register HW_UARTAPPINTR, field RSVD2
#define BP_UARTAPPINTR_RSVD2         12
#define BM_UARTAPPINTR_RSVD2         0x0000F000

//  Register HW_UARTAPPINTR, field ABDIS
#define BP_UARTAPPINTR_ABDIS         11
#define BM_UARTAPPINTR_ABDIS         0x00000800

// Register HW_UARTAPPINTR, field OEIS
#define BP_UARTAPPINTR_OEIS          10
#define BM_UARTAPPINTR_OEIS          0x00000400

//  Register HW_UARTAPPINTR, field BEIS
#define BP_UARTAPPINTR_BEIS          9
#define BM_UARTAPPINTR_BEIS          0x00000200

//  Register HW_UARTAPPINTR, field PEIS
#define BP_UARTAPPINTR_PEIS          8
#define BM_UARTAPPINTR_PEIS          0x00000100

//  Register HW_UARTAPPINTR, field FEIS
#define BP_UARTAPPINTR_FEIS          7
#define BM_UARTAPPINTR_FEIS          0x00000080

// Register HW_UARTAPPINTR, field RTIS
#define BP_UARTAPPINTR_RTIS          6
#define BM_UARTAPPINTR_RTIS          0x00000040

//  Register HW_UARTAPPINTR, field TXIS
#define BP_UARTAPPINTR_TXIS          5
#define BM_UARTAPPINTR_TXIS          0x00000020

//  Register HW_UARTAPPINTR, field RXIS
#define BP_UARTAPPINTR_RXIS          4
#define BM_UARTAPPINTR_RXIS          0x00000010

//  Register HW_UARTAPPINTR, field DSRMIS
#define BP_UARTAPPINTR_DSRMIS        3
#define BM_UARTAPPINTR_DSRMIS        0x00000008

// Register HW_UARTAPPINTR, field DCDMIS
#define BP_UARTAPPINTR_DCDMIS        2
#define BM_UARTAPPINTR_DCDMIS        0x00000004

//  Register HW_UARTAPPINTR, field CTSMIS
#define BP_UARTAPPINTR_CTSMIS        1
#define BM_UARTAPPINTR_CTSMIS        0x00000002

//  Register HW_UARTAPPINTR, field RIMIS
#define BP_UARTAPPINTR_RIMIS         0
#define BM_UARTAPPINTR_RIMIS         0x00000001


//------------------------------------------------------------------------------
//   HW_UARTAPPDATA - UART DATA Register
//------------------------------------------------------------------------------
#ifndef __LANGUAGE_ASM__
typedef union
{
    reg32_t  U;
    struct
    {
        unsigned DATA         :  32;
    } B;
} hw_uartappdata_t;
#endif

//constants & macros for entire HW_UARTAPPDATA register

#define HW_UARTAPPDATA_ADDR(x)    (REGS_UARTAPP_BASE(x) + 0x60)

#ifndef __LANGUAGE_ASM__
#define HW_UARTAPPDATA(x)         (*(volatile hw_uartappdata_t *) HW_UARTAPPDATA_ADDR(x))
#define HW_UARTAPPDATA_RD(x)      (HW_UARTAPPDATA(x).U)
#define HW_UARTAPPDATA_WR(x, v)   (HW_UARTAPPDATA(x).U = (v))
#endif

//constants & macros for individual HW_UARTAPPDATA bitfields

// Register HW_UARTAPPDATA, field DATA
#define BP_UARTAPPDATA_DATA       0
#define BM_UARTAPPDATA_DATA       0xFFFFFFFF
#define BF_UARTAPPDATA_DATA(v)    (((v) << 0) & BM_UARTAPPDATA_DATA)


//------------------------------------------------------------------------------
//   HW_UARTAPPSTAT - UART Status Register
//------------------------------------------------------------------------------
#ifndef __LANGUAGE_ASM__
typedef union
{
    reg32_t  U;
    struct
    {
        unsigned RXCOUNT         :  16;
        unsigned FERR            :  1;
        unsigned PERR            :  1;
        unsigned BERR            :  1;
        unsigned OERR            :  1;
        unsigned RXBYTE_INVALID  :  4;
        unsigned RXFE            :  1;
        unsigned TXFF            :  1;
        unsigned RXFF            :  1;
        unsigned TXFE            :  1;
        unsigned CTS             :  1;
        unsigned BUSY            :  1;
        unsigned HISPEED         :  1;
        unsigned PRESENT         :  1;
    } B;
} hw_uartappstat_t;
#endif

//constants & macros for entire HW_UARTAPPSTAT register

#define HW_UARTAPPSTAT_ADDR(x)        (REGS_UARTAPP_BASE(x) + 0x70)

#ifndef __LANGUAGE_ASM__
#define HW_UARTAPPSTAT(x)             (*(volatile hw_uartappstat_t *) HW_UARTAPPSTAT_ADDR(x))
#define HW_UARTAPPSTAT_RD(x)          (HW_UARTAPPSTAT(x).U)
#define HW_UARTAPPSTAT_WR(x, v)       (HW_UARTAPPSTAT(x).U = (v))
#endif

//constants & macros for individual HW_UARTAPPSTAT bitfields

// Register HW_UARTAPPSTAT, field PRESENT
#define BP_UARTAPPSTAT_PRESENT        31
#define BM_UARTAPPSTAT_PRESENT        0x80000000

// Register HW_UARTAPPSTAT, field HISPEED
#define BP_UARTAPPSTAT_HISPEED        30
#define BM_UARTAPPSTAT_HISPEED        0x40000000

//  Register HW_UARTAPPSTAT, field BUSY
#define BP_UARTAPPSTAT_BUSY           29
#define BM_UARTAPPSTAT_BUSY           0x20000000

//  Register HW_UARTAPPSTAT, field CTS
#define BP_UARTAPPSTAT_CTS            28
#define BM_UARTAPPSTAT_CTS            0x10000000

// Register HW_UARTAPPSTAT, field TXFE
#define BP_UARTAPPSTAT_TXFE           27
#define BM_UARTAPPSTAT_TXFE           0x08000000

// Register HW_UARTAPPSTAT, field RXFF
#define BP_UARTAPPSTAT_RXFF           26
#define BM_UARTAPPSTAT_RXFF           0x04000000

//  Register HW_UARTAPPSTAT, field TXFF
#define BP_UARTAPPSTAT_TXFF           25
#define BM_UARTAPPSTAT_TXFF           0x02000000

//  Register HW_UARTAPPSTAT, field RXFE
#define BP_UARTAPPSTAT_RXFE           24
#define BM_UARTAPPSTAT_RXFE           0x01000000

// Register HW_UARTAPPSTAT, field RXBYTE_INVALID
#define BP_UARTAPPSTAT_RXBYTE_INVALID 20
#define BM_UARTAPPSTAT_RXBYTE_INVALID 0x00F00000

// Register HW_UARTAPPSTAT, field OERR
#define BP_UARTAPPSTAT_OERR           19
#define BM_UARTAPPSTAT_OERR           0x00080000

//  Register HW_UARTAPPSTAT, field BERR
#define BP_UARTAPPSTAT_BERR           18
#define BM_UARTAPPSTAT_BERR           0x00040000

//  Register HW_UARTAPPSTAT, field PERR
#define BP_UARTAPPSTAT_PERR           17
#define BM_UARTAPPSTAT_PERR           0x00020000

//  Register HW_UARTAPPSTAT, field FERR
#define BP_UARTAPPSTAT_FERR           16
#define BM_UARTAPPSTAT_FERR           0x00010000

//  Register HW_UARTAPPSTAT, field RXCOUNT
#define BP_UARTAPPSTAT_RXCOUNT        0
#define BM_UARTAPPSTAT_RXCOUNT        0x0000FFFF
#define BF_UARTAPPSTAT_RXCOUNT(v)     (((v) << 0) & BM_UARTAPPSTAT_RXCOUNT)


//------------------------------------------------------------------------------
//   HW_UARTAPPDEBUG - UART DEBUG Register
//------------------------------------------------------------------------------
#ifndef __LANGUAGE_ASM__
typedef union
{
    reg32_t  U;
    struct
    {
        unsigned RXDMARQ       :  1;
        unsigned TXDMARQ       :  1;
        unsigned RXCMDEND      :  1;
        unsigned TXCMDEND      :  1;
        unsigned RXDMARUN      :  1;
        unsigned TXDMARUN      :  1;
        unsigned RSVD1         :  4;
        unsigned RXFBAUD_DIV   :  6;
        unsigned RXIBAUD_DIV   :  16;
    } B;
} hw_uartappdebug_t;
#endif

//constants & macros for entire HW_UARTAPPDEBUG register

#define HW_UARTAPPDEBUG_ADDR(x)          (REGS_UARTAPP_BASE(x) + 0x80)

#ifndef __LANGUAGE_ASM__
#define HW_UARTAPPDEBUG(x)               (*(volatile hw_uartappdebug_t *) HW_UARTAPPDEBUG_ADDR(x))
#define HW_UARTAPPDEBUG_RD(x)            (HW_UARTAPPDEBUG(x).U)
#define HW_UARTAPPDEBUG_WR(x, v)         (HW_UARTAPPDEBUG(x).U = (v))
#endif

//constants & macros for individual HW_UARTAPPDEBUG bitfields

// Register HW_UARTAPPDEBUG, field RXIBAUD_DIV
#define BP_UARTAPPDEBUG_RXIBAUD_DIV       16
#define BM_UARTAPPDEBUG_RXIBAUD_DIV       0xFFFF0000
#define BF_UARTAPPDEBUG_RXIBAUD_DIV(v)    (((v) << 16) & BM_UARTAPPDEBUG_RXIBAUD_DIV)

// Register HW_UARTAPPDEBUG, field RXFBAUD_DIV
#define BP_UARTAPPDEBUG_RXFBAUD_DIV       10
#define BM_UARTAPPDEBUG_RXFBAUD_DIV       0x0000FC00
#define BF_UARTAPPDEBUG_RXFBAUD_DIV(v)    (((v) << 10) & BM_UARTAPPDEBUG_RXFBAUD_DIV)

//  Register HW_UARTAPPDEBUG, field RSVD1
#define BP_UARTAPPDEBUG_RSVD1             6
#define BM_UARTAPPDEBUG_RSVD1             0x000003C0

//  Register HW_UARTAPPDEBUG, field TXDMARUN
#define BP_UARTAPPDEBUG_TXDMARUN          5
#define BM_UARTAPPDEBUG_TXDMARUN          0x00000020

// Register HW_UARTAPPDEBUG, field RXDMARUN
#define BP_UARTAPPDEBUG_RXDMARUN          4
#define BM_UARTAPPDEBUG_RXDMARUN          0x00000010

//  Register HW_UARTAPPDEBUG, field TXCMDEND
#define BP_UARTAPPDEBUG_TXCMDEND          3
#define BM_UARTAPPDEBUG_TXCMDEND          0x00000008

// Register HW_UARTAPPDEBUG, field RXCMDEND
#define BP_UARTAPPDEBUG_RXCMDEND          2
#define BM_UARTAPPDEBUG_RXCMDEND          0x00000004

//  Register HW_UARTAPPDEBUG, field TXDMARQ
#define BP_UARTAPPDEBUG_TXDMARQ           1
#define BM_UARTAPPDEBUG_TXDMARQ           0x00000002

// Register HW_UARTAPPDEBUG, field RXDMARQ
#define BP_UARTAPPDEBUG_RXDMARQ           0
#define BM_UARTAPPDEBUG_RXDMARQD          0x00000001


//------------------------------------------------------------------------------
//   HW_UARTAPPVERSION - UART Version Register
//------------------------------------------------------------------------------
#ifndef __LANGUAGE_ASM__
typedef union
{
    reg32_t  U;
    struct
    {
        unsigned STEP        :  16;
        unsigned MINOR       :  8;
        unsigned MAJOR       :  8;
    } B;
} hw_uartappversion_t;
#endif

//constants & macros for entire HW_UARTAPPVERSION register

#define HW_UARTAPPVERSION_ADDR(x)      (REGS_UARTAPP_BASE(x) + 0x90)

#ifndef __LANGUAGE_ASM__
#define HW_UARTAPPVERSION(x)           (*(volatile hw_uartappversion_t *) HW_UARTAPPVERSION_ADDR(x))
#define HW_UARTAPPVERSION_RD(x)        (HW_UARTAPPVERSION(x).U)
#define HW_UARTAPPVERSION_WR(x, v)     (HW_UARTAPPVERSION(x).U = (v))
#endif

//constants & macros for individual HW_UARTAPPVERSION bitfields

// Register HW_UARTAPPVERSION, field MAJOR
#define BP_UARTAPPVERSION_MAJOR        24
#define BM_UARTAPPVERSION_MAJOR        0xFF000000
#define BF_UARTAPPVERSION_MAJOR(v)     (((v) << 24) & BM_UARTAPPVERSION_MAJOR)

// Register HW_UARTAPPVERSION, field MINOR
#define BP_UARTAPPVERSION_MINOR        16
#define BM_UARTAPPVERSION_MINOR        0x00FF0000
#define BF_UARTAPPVERSION_MINOR(v)     (((v) << 16) & BM_UARTAPPVERSION_MINOR)

//  Register HW_UARTAPPVERSION, field STEP
#define BP_UARTAPPVERSION_STEP         0
#define BM_UARTAPPVERSION_STEP         0x0000FFFF


//------------------------------------------------------------------------------
//   HW_UARTAPPAUTOBAUD - UART AutoBaud Register
//------------------------------------------------------------------------------
#ifndef __LANGUAGE_ASM__
typedef union
{
    reg32_t  U;
    struct
    {
        unsigned BAUD_DETECT_ENABLE    :  1;
        unsigned START_BAUD_DETECT     :  1;
        unsigned START_WITH_RUNBIT     :  1;
        unsigned TWO_REF_CHARS         :  1;
        unsigned UPDATE_TX             :  1;
        unsigned RSVD1                 :  11;
        unsigned REFCHAR0              :  8;
        unsigned REFCHAR1              :  8;
    } B;
} hw_uartappautobaud_t;
#endif

//constants & macros for entire HW_UARTAPPAUTOBAUD register

#define HW_UARTAPPAUTOBAUD_ADDR(x)        (REGS_UARTAPP_BASE(x) + 0x80)

#ifndef __LANGUAGE_ASM__
#define HW_UARTAPPAUTOBAUD(x)             (*(volatile hw_uartappautobaud_t *) HW_UARTAPPAUTOBAUD_ADDR(x))
#define HW_UARTAPPAUTOBAUD_RD(x)          (HW_UARTAPPAUTOBAUD(x).U)
#define HW_UARTAPPAUTOBAUD_WR(x, v)       (HW_UARTAPPAUTOBAUD(x).U = (v))
#define HW_UARTAPPAUTOBAUD_SET(x, v)      (HW_UARTAPPAUTOBAUD_WR(HW_UARTAPPAUTOBAUD_RD(x) |  (v)))
#define HW_UARTAPPAUTOBAUD_CLR(x, v)      (HW_UARTAPPAUTOBAUD_WR(HW_UARTAPPAUTOBAUD_RD(x) & ~(v)))
#define HW_UARTAPPAUTOBAUDTOG(x, v)       (HW_UARTAPPAUTOBAUD_WR(HW_UARTAPPAUTOBAUD_RD(x) ^  (v)))
#endif

//constants & macros for individual HW_UARTAPPAUTOBAUD bitfields

// Register HW_UARTAPPAUTOBAUD, field REFCHAR1
#define BP_UARTAPPAUTOBAUD_REFCHAR1             24
#define BM_UARTAPPAUTOBAUD_REFCHAR1             0xFF000000
#define BF_UARTAPPAUTOBAUD_REFCHAR1(v)          (((v) << 24) & BM_UARTAPPAUTOBAUD_REFCHAR1)

// Register HW_UARTAPPAUTOBAUD, field REFCHAR0
#define BP_UARTAPPAUTOBAUD_REFCHAR0             16
#define BM_UARTAPPAUTOBAUD_REFCHAR0             0x00FF0000
#define BF_UARTAPPAUTOBAUD_REFCHAR0(v)          (((v) << 16) & BM_UARTAPPAUTOBAUD_REFCHAR0)

//  Register HW_UARTAPPAUTOBAUD, field RSVD1
#define BP_UARTAPPAUTOBAUD_RSVD1                5
#define BM_UARTAPPAUTOBAUD_RSVD1                0x0000FFE0

// Register HW_UARTAPPAUTOBAUD, field UPDATE_TX
#define BP_UARTAPPAUTOBAUD_UPDATE_TX            4
#define BM_UARTAPPAUTOBAUD_UPDATE_TX            0x00000010

// Register HW_UARTAPPAUTOBAUD, field TWO_REF_CHARS
#define BP_UARTAPPAUTOBAUD_TWO_REF_CHARS        3
#define BM_UARTAPPAUTOBAUD_TWO_REF_CHARS        0x00000008

//  Register HW_UARTAPPAUTOBAUD, field START_WITH_RUNBIT
#define BP_UARTAPPAUTOBAUD_START_WITH_RUNBIT    2
#define BM_UARTAPPAUTOBAUD_START_WITH_RUNBIT    0x00000004

//  Register HW_UARTAPPAUTOBAUD, field START_BAUD_DETECT
#define BP_UARTAPPAUTOBAUD_START_BAUD_DETECT    1
#define BM_UARTAPPAUTOBAUD_START_BAUD_DETECT    0x00000002

//  Register HW_UARTAPPAUTOBAUD, field BAUD_DETECT_ENABLE
#define BP_UARTAPPAUTOBAUD_BAUD_DETECT_ENABLE   0
#define BM_UARTAPPAUTOBAUD_BAUD_DETECT_ENABLE   0x00000001


#endif // __COMMON_UARTAPP_H
