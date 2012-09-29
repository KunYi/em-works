//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header:  mx25_iomux.h
//
//  Provides definitions for the IOMUX module of the MX25 SoC.
//
//------------------------------------------------------------------------------

#ifndef __MX25_IOMUX_H__
#define __MX25_IOMUX_H__


//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------
typedef struct
{
    REG32 GPR;
    REG32 INT_OBSRV;
    REG32 SW_MUX_CTL[137];
    REG32 SW_PAD_CTL[123];

    REG32 PAD_CTL_GRP_DVS_MISC;
    REG32 PAD_CTL_GRP_DSE_FEC;
    REG32 PAD_CTL_GRP_DVS_JTAG;
    REG32 PAD_CTL_GRP_DSE_NFC;
    REG32 PAD_CTL_GRP_DSE_CSI;
    REG32 PAD_CTL_GRP_DSE_WEIM; 
    REG32 PAD_CTL_GRP_DSE_DDR;
    REG32 PAD_CTL_GRP_DVS_CRM;
    REG32 PAD_CTL_GRP_DSE_KPP;
    REG32 PAD_CTL_GRP_DSE_SDHC1;
    REG32 PAD_CTL_GRP_DSE_LCD;
    REG32 PAD_CTL_GRP_DSE_UART; 
    REG32 PAD_CTL_GRP_DVS_NFC;
    REG32 PAD_CTL_GRP_DVS_CSI;
    REG32 PAD_CTL_GRP_DSE_CSPI1;
    REG32 PAD_CTL_GRP_DDRTYPE;
    REG32 PAD_CTL_GRP_DVS_SDHC1;
    REG32 PAD_CTL_GRP_DVS_LCD;

    REG32 SW_SELECT_INPUT[73];

} CSP_IOMUX_REGS, *PCSP_IOMUX_REGS;


//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
#define IOMUX_GPR_OFFSET                            0x0000
#define IOMUX_INT_OBSRV_OFFSET                      0x0004
#define IOMUX_SW_MUX_CTL_OFFSET                     0x0008
#define IOMUX_SW_PAD_CTL_OFFSET                     0x022C
#define IOMUX_SW_SELECT_INPUT_OFFSET                0x0460

//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
#define IOMUX_SW_MUX_CTL_MUX_MODE_LSH               0
#define IOMUX_SW_MUX_CTL_SION_LSH                   4

#define IOMUX_SW_PAD_CTL_SRE_LSH                    0
#define IOMUX_SW_PAD_CTL_DSE_LSH                    1
#define IOMUX_SW_PAD_CTL_ODE_LSH                    3
#define IOMUX_SW_PAD_CTL_PUS_LSH                    4
#define IOMUX_SW_PAD_CTL_PUE_LSH                    6
#define IOMUX_SW_PAD_CTL_PKE_LSH                    7
#define IOMUX_SW_PAD_CTL_HYS_LSH                    8
#define IOMUX_SW_PAD_CTL_DRIVE_VOLTAGE_LSH          13

#define IOMUX_SW_SELECT_INPUT_LSH                   0

//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
#define IOMUX_SW_MUX_CTL_MUX_MODE_WID               3
#define IOMUX_SW_MUX_CTL_SION_WID                   1

#define IOMUX_SW_PAD_CTL_SRE_WID                    1
#define IOMUX_SW_PAD_CTL_DSE_WID                    2
#define IOMUX_SW_PAD_CTL_ODE_WID                    1
#define IOMUX_SW_PAD_CTL_PUS_WID                    4
#define IOMUX_SW_PAD_CTL_PUE_WID                    1
#define IOMUX_SW_PAD_CTL_OKE_WID                    1
#define IOMUX_SW_PAD_CTL_HYS_WID                    1
#define IOMUX_SW_PAD_CTL_DRIVE_VOLTAGE_WID          1

#define IOMUX_SW_SELECT_INPUT_WID                   3

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

#define IOMUX_SW_PAD_CTL_PUS_100K_DOWN              0  // 100K Ohm pull down
#define IOMUX_SW_PAD_CTL_PUS_47K_UP                 1  // 47K Ohm pull up
#define IOMUX_SW_PAD_CTL_PUS_100K_UP                2  // 100K Ohm pull up
#define IOMUX_SW_PAD_CTL_PUS_22K_UP                 3  // 22K Ohm pull up

#define IOMUX_SW_PAD_CTL_PKE_DISABLE                0   // Disable keeper 
#define IOMUX_SW_PAD_CTL_PKE_ENABLE                 1   // Enable keeper

#define IOMUX_SW_PAD_CTL_PUE_KEEPER_ENABLE          0   // Enable keeper 
#define IOMUX_SW_PAD_CTL_PUE_PULL_ENABLE            1   // Enable pull up or down

#define IOMUX_SW_PAD_CTL_HYS_DISABLE                0   // Disable hysteresis
#define IOMUX_SW_PAD_CTL_HYS_ENABLE                 1   // Enable hysteresis

#define IOMUX_SW_PAD_CTL_DRIVE_VOLTAGE_3V3          0  // 3.3v Driver
#define IOMUX_SW_PAD_CTL_DRIVE_VOLTAGE_1V8          1  // 1.8v Driver


#endif // __MX25_IOMUX_H__
