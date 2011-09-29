//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header:  mx35_iomux.h
//
//  Provides definitions for the IOMUX module of the MX35 SoC.
//
//------------------------------------------------------------------------------

#ifndef __MX35_IOMUX_H__
#define __MX35_IOMUX_H__


//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------
typedef struct
{
    UINT32 GPR;
    UINT32 SW_MUX_CTL[201];
    UINT32 SW_PAD_CTL[283];
    UINT32 SW_PAD_CTL_DDRTYPE_GRP[5];
    UINT32 SW_SELECT_INPUT[148];
} CSP_IOMUX_REGS, *PCSP_IOMUX_REGS;


//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
#define IOMUX_GPR_OFFSET                            0x0000
#define IOMUX_SW_MUX_CTL_OFFSET                     0x0004
#define IOMUX_SW_PAD_CTL_OFFSET                     0x0328
#define IOMUX_SW_PAD_CTL_DDRTYPE_GRP_OFFSET         0x0794
#define IOMUX_SW_SELECT_INPUT_OFFSET                0x07A8


//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
#define IOMUX_SW_MUX_CTL_MUX_MODE_LSH               0
#define IOMUX_SW_MUX_CTL_SION_LSH                   4

#define IOMUX_SW_PAD_CTL_SRE_LSH                    0
#define IOMUX_SW_PAD_CTL_DSE_LSH                    1
#define IOMUX_SW_PAD_CTL_ODE_LSH                    3
#define IOMUX_SW_PAD_CTL_PULL_KEEP_CTL_LSH          4
#define IOMUX_SW_PAD_CTL_HYS_LSH                    8
#define IOMUX_SW_PAD_CTL_DRIVE_VOLTAGE_LSH          13


//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
#define IOMUX_SW_MUX_CTL_MUX_MODE_WID               3
#define IOMUX_SW_MUX_CTL_SION_WID                   1

#define IOMUX_SW_PAD_CTL_SRE_WID                    1
#define IOMUX_SW_PAD_CTL_DSE_WID                    2
#define IOMUX_SW_PAD_CTL_ODE_WID                    1
#define IOMUX_SW_PAD_CTL_PULL_KEEP_CTL_WID          4
#define IOMUX_SW_PAD_CTL_HYS_WID                    1
#define IOMUX_SW_PAD_CTL_DRIVE_VOLTAGE_WID          1


//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------

// SW_MUX_CTL
#define IOMUX_SW_MUX_CTL_MUX_MODE_ALT0              0   // Select ALT0 mux mode
#define IOMUX_SW_MUX_CTL_MUX_MODE_ALT1              1   // Select ALT1 mux mode
#define IOMUX_SW_MUX_CTL_MUX_MODE_ALT2              2   // Select ALT2 mux mode
#define IOMUX_SW_MUX_CTL_MUX_MODE_ALT3              3   // Select ALT3 mux mode
#define IOMUX_SW_MUX_CTL_MUX_MODE_ALT4              4   // Select ALT4 mux mode
#define IOMUX_SW_MUX_CTL_MUX_MODE_ALT5              5   // Select ALT5 mux mode
#define IOMUX_SW_MUX_CTL_MUX_MODE_ALT6              6   // Select ALT6 mux mode
#define IOMUX_SW_MUX_CTL_MUX_MODE_ALT7              7   // Select ALT7 mux mode

#define IOMUX_SW_MUX_CTL_SION_REGULAR               0   // Input is determined by mux mode
#define IOMUX_SW_MUX_CTL_SION_FORCE                 1   // Force input of some pad


// SW_PAD_CTL
#define IOMUX_SW_PAD_CTL_SRE_SLOW                   0   // Slow slew rate
#define IOMUX_SW_PAD_CTL_SRE_FAST                   1   // Fast slew rate

#define IOMUX_SW_PAD_CTL_DSE_NORMAL                 0   // Normal drive strength
#define IOMUX_SW_PAD_CTL_DSE_HIGH                   1   // High drive strength
#define IOMUX_SW_PAD_CTL_DSE_MAX                    2   // Maximum drive strength

#define IOMUX_SW_PAD_CTL_ODE_DISABLE                0   // Disable open drain
#define IOMUX_SW_PAD_CTL_ODE_ENABLE                 1   // Enable open drain

#define IOMUX_SW_PAD_CTL_PULL_KEEP_CTL_DISABLE      0   // Disable keeper 
#define IOMUX_SW_PAD_CTL_PULL_KEEP_CTL_ENABLE       8   // Enable keeper
#define IOMUX_SW_PAD_CTL_PULL_KEEP_CTL_100K_DOWN    12  // 100K Ohm pull down
#define IOMUX_SW_PAD_CTL_PULL_KEEP_CTL_47K_UP       13  // 47K Ohm pull up
#define IOMUX_SW_PAD_CTL_PULL_KEEP_CTL_100K_UP      14  // 100K Ohm pull up
#define IOMUX_SW_PAD_CTL_PULL_KEEP_CTL_22K_UP       15  // 22K Ohm pull up

#define IOMUX_SW_PAD_CTL_HYS_DISABLE                0   // Disable hysteresis
#define IOMUX_SW_PAD_CTL_HYS_ENABLE                 1   // Enable hysteresis

#define IOMUX_SW_PAD_CTL_DRIVE_VOLTAGE_3V3          0  // 3.3v Driver
#define IOMUX_SW_PAD_CTL_DRIVE_VOLTAGE_1V8          1  // 1.8v Driver


#endif // __MX35_IOMUX_H__
