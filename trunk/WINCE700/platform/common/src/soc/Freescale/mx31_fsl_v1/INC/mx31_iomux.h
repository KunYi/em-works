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
//-----------------------------------------------------------------------------
//
//  Copyright (C) 2004, Motorola Inc. All Rights Reserved
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2007, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header:  MX31_iomux.h
//
//  Provides definitions for the IOMUX module of the MX31 SoC.
//
//------------------------------------------------------------------------------

#ifndef __MX31_IOMUX_H__
#define __MX31_IOMUX_H__


//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------

// Mask for isolating mux control bits of a single pin
#define IOMUX_SW_MUX_CTL_MASK       0x7FU

// Mask for isolating mux control bits of a single pad
#define IOMUX_SW_PAD_CTL_MASK       0x3FFU


//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------
typedef struct
{
    UINT32 INT_OBS[2];      // 0x00..0x07
    UINT32 GPR;             // 0x08..0xB
    UINT32 SW_MUX_CTL[82];  // 0x0C..0x153
    UINT32 SW_PAD_CTL[110]; // 0x154..0x30B
} CSP_IOMUX_REGS, *PCSP_IOMUX_REGS;


//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
#define IOMUX_INT_OBS_OFFSET                0x0000
#define IOMUX_GPR_OFFSET                    0x0008
#define IOMUX_SW_MUX_CTL_OFFSET             0x000C
#define IOMUX_SW_PAD_CTL_OFFSET             0x0150


//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
#define IOMUX_SW_MUX_CTL_IN_LSH             0
#define IOMUX_SW_MUX_CTL_GPIO_IN_LSH        0
#define IOMUX_SW_MUX_CTL_FUNC_IN_LSH        1
#define IOMUX_SW_MUX_CTL_MUX1_IN_LSH        2
#define IOMUX_SW_MUX_CTL_MUX2_IN_LSH        3
#define IOMUX_SW_MUX_CTL_OUT_LSH            4

#define IOMUX_SW_PAD_CTL_SRE_LSH            0
#define IOMUX_SW_PAD_CTL_DSE_LSH            1
#define IOMUX_SW_PAD_CTL_ODE_LSH            3
#define IOMUX_SW_PAD_CTL_HYS_LSH            4
#define IOMUX_SW_PAD_CTL_PUS_LSH            5
#define IOMUX_SW_PAD_CTL_PUE_LSH            7
#define IOMUX_SW_PAD_CTL_PKE_LSH            8
#define IOMUX_SW_PAD_CTL_LOOP_LSH           9


//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
#define IOMUX_SW_MUX_CTL_IN_WID             4
#define IOMUX_SW_MUX_CTL_GPIO_IN_WID        1
#define IOMUX_SW_MUX_CTL_FUNC_IN_WID        1
#define IOMUX_SW_MUX_CTL_MUX1_IN_WID        1
#define IOMUX_SW_MUX_CTL_MUX2_IN_WID        1
#define IOMUX_SW_MUX_CTL_OUT_WID            3

#define IOMUX_SW_PAD_CTL_SRE_WID            1
#define IOMUX_SW_PAD_CTL_DSE_WID            2
#define IOMUX_SW_PAD_CTL_ODE_WID            1
#define IOMUX_SW_PAD_CTL_HYS_WID            1
#define IOMUX_SW_PAD_CTL_PUS_WID            2
#define IOMUX_SW_PAD_CTL_PUE_WID            1
#define IOMUX_SW_PAD_CTL_PKE_WID            1
#define IOMUX_SW_PAD_CTL_LOOP_WID           1


//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------

// SW_MUX_CTL
#define IOMUX_SW_MUX_CTL_GPIO_IN_DISABLE    0   // Disable GPIO input
#define IOMUX_SW_MUX_CTL_GPIO_IN_ENABLE     1   // Enable GPIO input
#define IOMUX_SW_MUX_CTL_FUNC_IN_DISABLE    0   // Disable functional input
#define IOMUX_SW_MUX_CTL_FUNC_IN_ENABLE     1   // Enable functional input
#define IOMUX_SW_MUX_CTL_MUX1_IN_DISABLE    0   // Disable MUX1 input
#define IOMUX_SW_MUX_CTL_MUX1_IN_ENABLE     1   // Enable MUX1 input
#define IOMUX_SW_MUX_CTL_MUX2_IN_DISABLE    0   // Disable MUX2 input
#define IOMUX_SW_MUX_CTL_MUX2_IN_ENABLE     1   // Enable MUX2 input

#define IOMUX_SW_MUX_CTL_OUT_GPIO           0   // Ouput from GPIO
#define IOMUX_SW_MUX_CTL_OUT_FUNC           1   // Functional ouput
#define IOMUX_SW_MUX_CTL_OUT_MUX1           2   // Ouput from MUX1
#define IOMUX_SW_MUX_CTL_OUT_MUX2           3   // Ouput from MUX2
#define IOMUX_SW_MUX_CTL_OUT_MUX3           4   // Ouput from MUX3
#define IOMUX_SW_MUX_CTL_OUT_MUX4           5   // Ouput from MUX4
#define IOMUX_SW_MUX_CTL_OUT_MUX5           6   // Ouput from MUX5
#define IOMUX_SW_MUX_CTL_OUT_MUX6           7   // Ouput from MUX6


// SW_PAD_CTL
#define IOMUX_SW_PAD_CTL_SRE_SLOW           0   // Slow slew rate
#define IOMUX_SW_PAD_CTL_SRE_FAST           1   // Fast slew rate

#define IOMUX_SW_PAD_CTL_DSE_NORMAL         0   // Normal drive strength
#define IOMUX_SW_PAD_CTL_DSE_HIGH           1   // High drive strength
#define IOMUX_SW_PAD_CTL_DSE_MAX            2   // Maximum drive strength

#define IOMUX_SW_PAD_CTL_ODE_CMOS           0   // Output is CMOS
#define IOMUX_SW_PAD_CTL_ODE_OPEN_DRAIN     1   // Output is open drain

#define IOMUX_SW_PAD_CTL_HYS_CMOS           0   // CMOS input
#define IOMUX_SW_PAD_CTL_HYS_SCHMITT        1   // Schmitt trigger input

#define IOMUX_SW_PAD_CTL_PUS_100K_DOWN      0   // 100K Ohm pull down
#define IOMUX_SW_PAD_CTL_PUS_100K_UP        1   // 100K Ohm pull up
#define IOMUX_SW_PAD_CTL_PUS_47K_UP         2   // 47K Ohm pull up
#define IOMUX_SW_PAD_CTL_PUS_22K_UP         3   // 22K Ohm pull up

#define IOMUX_SW_PAD_CTL_PUE_KEEPER         0  // Keeper enable
#define IOMUX_SW_PAD_CTL_PUE_PULL           1  // Pull up/down enable

#define IOMUX_SW_PAD_CTL_PKE_DISABLE        0   // Pull up/down/keeper disabled
#define IOMUX_SW_PAD_CTL_PKE_ENABLE         1   // Pull up/down/keeper enabled

#endif // __MX31_IOMUX_H__
