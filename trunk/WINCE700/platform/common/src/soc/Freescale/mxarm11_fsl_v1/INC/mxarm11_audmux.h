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
//  Header:  mxarm11_audmux.h
//
//  Provides definitions for the AUDMUX (Digital Audio Mux) module based
//  on the Freescale ARM11 chassis.
//
//------------------------------------------------------------------------------
#ifndef __MXARM11_AUDMUX_H
#define __MXARM11_AUDMUX_H

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
    UINT32 PTCR1;
    UINT32 PDCR1;
    UINT32 PTCR2;
    UINT32 PDCR2;
    UINT32 PTCR3;
    UINT32 PDCR3;
    UINT32 PTCR4;
    UINT32 PDCR4;
    UINT32 PTCR5;
    UINT32 PDCR5;
    UINT32 PTCR6;
    UINT32 PDCR6;
    UINT32 PTCR7;
    UINT32 PDCR7;
    UINT32 CNMCR;
} CSP_AUDMUX_REG, *PCSP_AUDMUX_REG;


//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
#define AUDMUX_PTCR1_OFFSET                 0x0000
#define AUDMUX_PDCR1_OFFSET                 0x0004
#define AUDMUX_PTCR2_OFFSET                 0x0008
#define AUDMUX_PDCR2_OFFSET                 0x000C
#define AUDMUX_PTCR3_OFFSET                 0x0010
#define AUDMUX_PDCR3_OFFSET                 0x0014
#define AUDMUX_PTCR4_OFFSET                 0x0018
#define AUDMUX_PDCR4_OFFSET                 0x001C
#define AUDMUX_PTCR5_OFFSET                 0x0020
#define AUDMUX_PDCR5_OFFSET                 0x0024
#define AUDMUX_PTCR6_OFFSET                 0x0028
#define AUDMUX_PDCR6_OFFSET                 0x002C
#define AUDMUX_PTCR7_OFFSET                 0x0030
#define AUDMUX_PDCR7_OFFSET                 0x0034
#define AUDMUX_CNMCR_OFFSET                 0x0038


//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
#define AUDMUX_PTCR_SYN_LSH                 11
#define AUDMUX_PTCR_RCSEL_LSH               12
#define AUDMUX_PTCR_RCLKDIR_LSH             16
#define AUDMUX_PTCR_RFSEL_LSH               17
#define AUDMUX_PTCR_RFSDIR_LSH              21
#define AUDMUX_PTCR_TCSEL_LSH               22
#define AUDMUX_PTCR_TCLKDIR_LSH             26
#define AUDMUX_PTCR_TFSEL_LSH               27
#define AUDMUX_PTCR_TFSDIR_LSH              31

#define AUDMUX_PDCR_INMMASK_LSH             0
#define AUDMUX_PDCR_MODE_LSH                8
#define AUDMUX_PDCR_TXRXEN_LSH              12
#define AUDMUX_PDCR_RXDSEL_LSH              13

#define AUDMUX_CNMCR_CNTLOW_LSH             0
#define AUDMUX_CNMCR_CNTHI_LSH              8
#define AUDMUX_CNMCR_CLKPOL_LSH             16
#define AUDMUX_CNMCR_FSPOL_LSH              17
#define AUDMUX_CNMCR_CEN_LSH                18


//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
#define AUDMUX_PTCR_SYN_WID                 1
#define AUDMUX_PTCR_RCSEL_WID               4
#define AUDMUX_PTCR_RCLKDIR_WID             1
#define AUDMUX_PTCR_RFSEL_WID               4
#define AUDMUX_PTCR_RFSDIR_WID              1
#define AUDMUX_PTCR_TCSEL_WID               4
#define AUDMUX_PTCR_TCLKDIR_WID             1
#define AUDMUX_PTCR_TFSEL_WID               4
#define AUDMUX_PTCR_TFSDIR_WID              1

#define AUDMUX_PDCR_INMMASK_WID             8
#define AUDMUX_PDCR_MODE_WID                2
#define AUDMUX_PDCR_TXRXEN_WID              1
#define AUDMUX_PDCR_RXDSEL_WID              3

#define AUDMUX_CNMCR_CNTLOW_WID             8
#define AUDMUX_CNMCR_CNTHI_WID              8
#define AUDMUX_CNMCR_CLKPOL_WID             1
#define AUDMUX_CNMCR_FSPOL_WID              1
#define AUDMUX_CNMCR_CEN_WID                1


//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------

// PTCR
#define AUDMUX_PTCR_SYN_ASYNC               0   // Asynchronous mode
#define AUDMUX_PTCR_SYN_SYNC                1   // Synchronous mode

#define AUDMUX_PTCR_RCSEL_TXCLK_PORT1       0   // RXCLK is TXCLK from port1
#define AUDMUX_PTCR_RCSEL_TXCLK_PORT2       1   // RXCLK is TXCLK from port2
#define AUDMUX_PTCR_RCSEL_TXCLK_PORT3       2   // RXCLK is TXCLK from port3
#define AUDMUX_PTCR_RCSEL_TXCLK_PORT4       3   // RXCLK is TXCLK from port4
#define AUDMUX_PTCR_RCSEL_TXCLK_PORT5       4   // RXCLK is TXCLK from port5
#define AUDMUX_PTCR_RCSEL_TXCLK_PORT6       5   // RXCLK is TXCLK from port6
#define AUDMUX_PTCR_RCSEL_TXCLK_PORT7       6   // RXCLK is TXCLK from port7
#define AUDMUX_PTCR_RCSEL_RXCLK_PORT1       8   // RXCLK is RXCLK from port1
#define AUDMUX_PTCR_RCSEL_RXCLK_PORT2       9   // RXCLK is RXCLK from port2
#define AUDMUX_PTCR_RCSEL_RXCLK_PORT3       10  // RXCLK is RXCLK from port3
#define AUDMUX_PTCR_RCSEL_RXCLK_PORT4       11  // RXCLK is RXCLK from port4
#define AUDMUX_PTCR_RCSEL_RXCLK_PORT5       12  // RXCLK is RXCLK from port5
#define AUDMUX_PTCR_RCSEL_RXCLK_PORT6       13  // RXCLK is RXCLK from port6
#define AUDMUX_PTCR_RCSEL_RXCLK_PORT7       14  // RXCLK is RXCLK from port7

#define AUDMUX_PTCR_RCLKDIR_INPUT           0   // RXCLK is an input
#define AUDMUX_PTCR_RCLKDIR_OUTPUT          1   // RXCLK is an output

#define AUDMUX_PTCR_RFSEL_TXFS_PORT1        0   // RXFS is TXFS from port1
#define AUDMUX_PTCR_RFSEL_TXFS_PORT2        1   // RXFS is TXFS from port2
#define AUDMUX_PTCR_RFSEL_TXFS_PORT3        2   // RXFS is TXFS from port3
#define AUDMUX_PTCR_RFSEL_TXFS_PORT4        3   // RXFS is TXFS from port4
#define AUDMUX_PTCR_RFSEL_TXFS_PORT5        4   // RXFS is TXFS from port5
#define AUDMUX_PTCR_RFSEL_TXFS_PORT6        5   // RXFS is TXFS from port6
#define AUDMUX_PTCR_RFSEL_TXFS_PORT7        6   // RXFS is TXFS from port7
#define AUDMUX_PTCR_RFSEL_RXFS_PORT1        8   // RXFS is RXFS from port1
#define AUDMUX_PTCR_RFSEL_RXFS_PORT2        9   // RXFS is RXFS from port2
#define AUDMUX_PTCR_RFSEL_RXFS_PORT3        10  // RXFS is RXFS from port3
#define AUDMUX_PTCR_RFSEL_RXFS_PORT4        11  // RXFS is RXFS from port4
#define AUDMUX_PTCR_RFSEL_RXFS_PORT5        12  // RXFS is RXFS from port5
#define AUDMUX_PTCR_RFSEL_RXFS_PORT6        13  // RXFS is RXFS from port6
#define AUDMUX_PTCR_RFSEL_RXFS_PORT7        14  // RXFS is RXFS from port7

#define AUDMUX_PTCR_RFSDIR_INPUT            0   // RXFS is an input
#define AUDMUX_PTCR_RFSDIR_OUTPUT           1   // RXFS is an output

#define AUDMUX_PTCR_TCSEL_TXCLK_PORT1       0   // TXCLK is TXCLK from port1
#define AUDMUX_PTCR_TCSEL_TXCLK_PORT2       1   // TXCLK is TXCLK from port2
#define AUDMUX_PTCR_TCSEL_TXCLK_PORT3       2   // TXCLK is TXCLK from port3
#define AUDMUX_PTCR_TCSEL_TXCLK_PORT4       3   // TXCLK is TXCLK from port4
#define AUDMUX_PTCR_TCSEL_TXCLK_PORT5       4   // TXCLK is TXCLK from port5
#define AUDMUX_PTCR_TCSEL_TXCLK_PORT6       5   // TXCLK is TXCLK from port6
#define AUDMUX_PTCR_TCSEL_TXCLK_PORT7       6   // TXCLK is TXCLK from port7
#define AUDMUX_PTCR_TCSEL_RXCLK_PORT1       8   // TXCLK is RXCLK from port1
#define AUDMUX_PTCR_TCSEL_RXCLK_PORT2       9   // TXCLK is RXCLK from port2
#define AUDMUX_PTCR_TCSEL_RXCLK_PORT3       10  // TXCLK is RXCLK from port3
#define AUDMUX_PTCR_TCSEL_RXCLK_PORT4       11  // TXCLK is RXCLK from port4
#define AUDMUX_PTCR_TCSEL_RXCLK_PORT5       12  // TXCLK is RXCLK from port5
#define AUDMUX_PTCR_TCSEL_RXCLK_PORT6       13  // TXCLK is RXCLK from port6
#define AUDMUX_PTCR_TCSEL_RXCLK_PORT7       14  // TXCLK is RXCLK from port7

#define AUDMUX_PTCR_TCLKDIR_INPUT           0   // TXCLK is an input
#define AUDMUX_PTCR_TCLKDIR_OUTPUT          1   // TXCLK is an output

#define AUDMUX_PTCR_TFSEL_TXFS_PORT1        0   // TXFS is TXFS from port1
#define AUDMUX_PTCR_TFSEL_TXFS_PORT2        1   // TXFS is TXFS from port2
#define AUDMUX_PTCR_TFSEL_TXFS_PORT3        2   // TXFS is TXFS from port3
#define AUDMUX_PTCR_TFSEL_TXFS_PORT4        3   // TXFS is TXFS from port4
#define AUDMUX_PTCR_TFSEL_TXFS_PORT5        4   // TXFS is TXFS from port5
#define AUDMUX_PTCR_TFSEL_TXFS_PORT6        5   // TXFS is TXFS from port6
#define AUDMUX_PTCR_TFSEL_TXFS_PORT7        6   // TXFS is TXFS from port7
#define AUDMUX_PTCR_TFSEL_RXFS_PORT1        8   // TXFS is RXFS from port1
#define AUDMUX_PTCR_TFSEL_RXFS_PORT2        9   // TXFS is RXFS from port2
#define AUDMUX_PTCR_TFSEL_RXFS_PORT3        10  // TXFS is RXFS from port3
#define AUDMUX_PTCR_TFSEL_RXFS_PORT4        11  // TXFS is RXFS from port4
#define AUDMUX_PTCR_TFSEL_RXFS_PORT5        12  // TXFS is RXFS from port5
#define AUDMUX_PTCR_TFSEL_RXFS_PORT6        13  // TXFS is RXFS from port6
#define AUDMUX_PTCR_TFSEL_RXFS_PORT7        14  // TXFS is RXFS from port7

#define AUDMUX_PTCR_TFSDIR_INPUT            0   // TXFS is an input
#define AUDMUX_PTCR_TFSDIR_OUTPUT           1   // TXFS is an output

// PDCR
#define AUDMUX_PDCR_INMMASK_EXC_RXD1     0x01   // Exclude port1 RXD from network
#define AUDMUX_PDCR_INMMASK_EXC_RXD2     0x02   // Exclude port1 RXD from network
#define AUDMUX_PDCR_INMMASK_EXC_RXD3     0x04   // Exclude port1 RXD from network
#define AUDMUX_PDCR_INMMASK_EXC_RXD4     0x08   // Exclude port1 RXD from network
#define AUDMUX_PDCR_INMMASK_EXC_RXD5     0x10   // Exclude port1 RXD from network
#define AUDMUX_PDCR_INMMASK_EXC_RXD6     0x20   // Exclude port1 RXD from network
#define AUDMUX_PDCR_INMMASK_EXC_RXD7     0x40   // Exclude port1 RXD from network

#define AUDMUX_PDCR_MODE_NORMAL             0   // Normal mode
#define AUDMUX_PDCR_MODE_INT_NET            1   // Internal network mode
#define AUDMUX_PDCR_MODE_CEBUS_NET          1   // CE Bus network mode

#define AUDMUX_PDCR_TXRXEN_NO_SWAP          0   // No swapping of RX/TX signals
#define AUDMUX_PDCR_TXRXEN_SWAP             1   // Swap RX/TX signals

#define AUDMUX_PDCR_RXDSEL_PORT1            0   // Port1 is source for RxD data
#define AUDMUX_PDCR_RXDSEL_PORT2            1   // Port2 is source for RxD data
#define AUDMUX_PDCR_RXDSEL_PORT3            2   // Port3 is source for RxD data
#define AUDMUX_PDCR_RXDSEL_PORT4            3   // Port4 is source for RxD data
#define AUDMUX_PDCR_RXDSEL_PORT5            4   // Port5 is source for RxD data
#define AUDMUX_PDCR_RXDSEL_PORT6            5   // Port6 is source for RxD data
#define AUDMUX_PDCR_RXDSEL_PORT7            6   // Port7 is source for RxD data

// CNMCR
#define AUDMUX_CNMCR_CLKPOL_POLARITY0       0   // CE Bus port polarity 0
#define AUDMUX_CNMCR_CLKPOL_POLARITY1       1   // CE Bus port polarity 1


#ifdef __cplusplus
}
#endif

#endif // __MXARM11_AUDMUX_H
