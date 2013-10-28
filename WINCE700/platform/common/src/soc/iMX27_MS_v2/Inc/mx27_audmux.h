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
// Copyright (C) 2005, MOTOROLA, INC. All Rights Reserved
//
//------------------------------------------------------------------------------
//
// Copyright (C) 2005-2006, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
//
//------------------------------------------------------------------------------
//
//  Header:  mx27_audmux.h
//
//  Provides definitions for AUDMUX module based on i.MX27.
//
//------------------------------------------------------------------------------

#ifndef __MX27_AUDMUX_H
#define __MX27_AUDMUX_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------
typedef enum {
    AUDMUX_PORT_ID_HOST1 = 0,
    AUDMUX_PORT_ID_HOST2,
    AUDMUX_PORT_ID_HOST3,
    AUDMUX_PORT_ID_PERI1,
    AUDMUX_PORT_ID_PERI2,
    AUDMUX_PORT_ID_PERI3,
    AUDMUX_PORT_ID_MAX,
} AUDMUX_PORT_ID;

#define AUDMUX_NUM_HOST_PORTS   3
#define AUDMUX_NUM_PERI_PORTS   3

//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------
typedef struct {
    UINT32 HPCR1;
    UINT32 HPCR2;
    UINT32 HPCR3;
    UINT32 _PAD1;
    UINT32 PPCR1;
    UINT32 PPCR2;
    UINT32 _PAD2;
    UINT32 PPCR3;
} CSP_AUDMUX_REG, *PCSP_AUDMUX_REG;

//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
#define AUDMUX_HPCR1_OFFSET         0x0000
#define AUDMUX_HPCR2_OFFSET         0x0004
#define AUDMUX_HPCR3_OFFSET         0x0008
#define AUDMUX_PPCR1_OFFSET         0x0010
#define AUDMUX_PPCR2_OFFSET         0x0014
#define AUDMUX_PPCR3_OFFSET         0x001C

//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
#define AUDMUX_HPCR_INMMASK_LSH     0
#define AUDMUX_HPCR_INMEN_LSH       8
#define AUDMUX_HPCR_TXRXEN_LSH      10
#define AUDMUX_HPCR_SYN_LSH         12
#define AUDMUX_HPCR_RXDSEL_LSH      13
#define AUDMUX_HPCR_RFCSEL_LSH      20
#define AUDMUX_HPCR_RCLKDIR_LSH     24
#define AUDMUX_HPCR_RFSDIR_LSH      25
#define AUDMUX_HPCR_TFCSEL_LSH      26
#define AUDMUX_HPCR_TCLKDIR_LSH     30
#define AUDMUX_HPCR_TFSDIR_LSH      31

#define AUDMUX_PPCR_TXRXEN_LSH      10
#define AUDMUX_PPCR_SYN_LSH         12
#define AUDMUX_PPCR_RXDSEL_LSH      13
#define AUDMUX_PPCR_RFCSEL_LSH      20
#define AUDMUX_PPCR_RCLKDIR_LSH     24
#define AUDMUX_PPCR_RFSDIR_LSH      25
#define AUDMUX_PPCR_TFCSEL_LSH      26
#define AUDMUX_PPCR_TCLKDIR_LSH     30
#define AUDMUX_PPCR_TFSDIR_LSH      31

//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
#define AUDMUX_HPCR_INMMASK_WID     8
#define AUDMUX_HPCR_INMEN_WID       1
#define AUDMUX_HPCR_TXRXEN_WID      1
#define AUDMUX_HPCR_SYN_WID         1
#define AUDMUX_HPCR_RXDSEL_WID      3
#define AUDMUX_HPCR_RFCSEL_WID      4
#define AUDMUX_HPCR_RCLKDIR_WID     1
#define AUDMUX_HPCR_RFSDIR_WID      1
#define AUDMUX_HPCR_TFCSEL_WID      4
#define AUDMUX_HPCR_TCLKDIR_WID     1
#define AUDMUX_HPCR_TFSDIR_WID      1

#define AUDMUX_PPCR_TXRXEN_WID      1
#define AUDMUX_PPCR_SYN_WID         1
#define AUDMUX_PPCR_RXDSEL_WID      3
#define AUDMUX_PPCR_RFCSEL_WID      4
#define AUDMUX_PPCR_RCLKDIR_WID     1
#define AUDMUX_PPCR_RFSDIR_WID      1
#define AUDMUX_PPCR_TFCSEL_WID      4
#define AUDMUX_PPCR_TCLKDIR_WID     1
#define AUDMUX_PPCR_TFSDIR_WID      1


//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------
// HPCR
#define AUDMUX_HPCR_INMEN_DISABLE           0
#define AUDMUX_HPCR_INMEN_INT_NET_MODE      1

#define AUDMUX_HPCR_TXRXEN_NO_SWITCH        0   // Only supported by port3
#define AUDMUX_HPCR_TXRXEN_SWITCH           1

#define AUDMUX_HPCR_SYN_ASYNC               0
#define AUDMUX_HPCR_SYN_SYNC                1

#define AUDMUX_HPCR_RFCSEL_FROM_TX          0x0 // 0xxx indicatest get tx fs/clk
#define AUDMUX_HPCR_RFCSEL_FROM_RX          0x8 // 1xxx indicates get rx fs/clk

#define AUDMUX_HPCR_RCLKDIR_INPUT           0
#define AUDMUX_HPCR_RCLKDIR_OUTPUT          1

#define AUDMUX_HPCR_RFSDIR_INPUT            0
#define AUDMUX_HPCR_RFSDIR_OUTPUT           1

#define AUDMUX_HPCR_TFCSEL_FROM_TX          0x0 // 0xxx indicatest get tx fs/clk
#define AUDMUX_HPCR_TFCSEL_FROM_RX          0x8 // 1xxx indicates get rx fs/clk

#define AUDMUX_HPCR_TCLKDIR_INPUT           0
#define AUDMUX_HPCR_TCLKDIR_OUTPUT          1

#define AUDMUX_HPCR_TFSDIR_INPUT            0
#define AUDMUX_HPCR_TFSDIR_OUTPUT           1

// PPCR
#define AUDMUX_PPCR_INMEN_DISABLE           0
#define AUDMUX_PPCR_INMEN_INT_NET_MODE      1

#define AUDMUX_PPCR_TXRXEN_NO_SWITCH        0
#define AUDMUX_PPCR_TXRXEN_SWITCH           1

#define AUDMUX_PPCR_SYN_ASYNC               0
#define AUDMUX_PPCR_SYN_SYNC                1

#define AUDMUX_PPCR_RFCSEL_FROM_TX          0x0 // 0xxx indicatest get tx fs/clk
#define AUDMUX_PPCR_RFCSEL_FROM_RX          0x8 // 1xxx indicates get rx fs/clk

#define AUDMUX_PPCR_RCLKDIR_INPUT           0
#define AUDMUX_PPCR_RCLKDIR_OUTPUT          1

#define AUDMUX_PPCR_RFSDIR_INPUT            0
#define AUDMUX_PPCR_RFSDIR_OUTPUT           1

#define AUDMUX_PPCR_TFCSEL_FROM_TX          0x0 // 0xxx indicatest get tx fs/clk
#define AUDMUX_PPCR_TFCSEL_FROM_RX          0x8 // 1xxx indicates get rx fs/clk

#define AUDMUX_PPCR_TCLKDIR_INPUT           0
#define AUDMUX_PPCR_TCLKDIR_OUTPUT          1

#define AUDMUX_PPCR_TFSDIR_INPUT            0
#define AUDMUX_PPCR_TFSDIR_OUTPUT           1


#ifdef __cplusplus
}
#endif

#endif // __MX27_AUDMUX_H

