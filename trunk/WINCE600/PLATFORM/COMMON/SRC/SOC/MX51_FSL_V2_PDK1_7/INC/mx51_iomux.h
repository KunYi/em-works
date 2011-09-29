//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header:  mx51_iomux.h
//
//  Provides definitions for the IOMUX module.
//
//------------------------------------------------------------------------------

#ifndef __MX51_IOMUX_H__
#define __MX51_IOMUX_H__


//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------
typedef struct
{
    UINT32 GPR0;                // 0x0000
    UINT32 GPR1;                // 0x0004
    UINT32 OBSERVE[5];          // 0x0008~0x0018
    UINT32 SW_MUX_CTL[247];     // 0x001C~0x03F4
    UINT32 SW_PAD_CTL[332];     // 0x03F8~0x0924
    UINT32 SELECT_INPUT[87];    // 0x0928~0x0A80
} CSP_IOMUX_TO1_REGS, *PCSP_IOMUX_TO1_REGS;

typedef struct
{
    UINT32 GPR0;                // 0x0000
    UINT32 GPR1;                // 0x0004
    UINT32 OBSERVE[5];          // 0x0008~0x0018
    UINT32 SW_MUX_CTL[245];     // 0x001C~0x03EC
    UINT32 SW_PAD_CTL[309];     // 0x03F0~0x08C0
    UINT32 SELECT_INPUT[89];    // 0x08C4~0x0A24
} CSP_IOMUX_TO2_REGS, *PCSP_IOMUX_TO2_REGS;

//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
#define IOMUX_GPR0_OFFSET                   0x0000
#define IOMUX_GPR1_OFFSET                   0x0004
#define IOMUX_OBSERVE_OFFSET                0x0008
#define IOMUX_SW_MUX_CTL_OFFSET             0x001C
#define IOMUX_TO1_SW_PAD_CTL_OFFSET         0x03F8
#define IOMUX_TO1_SELECT_INPUT_OFFSET       0x0928
#define IOMUX_TO2_SW_PAD_CTL_OFFSET         0x03F0
#define IOMUX_TO2_SELECT_INPUT_OFFSET       0x08C4

//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
#define IOMUX_GPR0_SSI3_TX0_SLM_TX0_LSH     0
#define IOMUX_GPR0_SSI3_TX1_SLM_TX1_LSH     1     
#define IOMUX_GPR0_SSI3_RX0_SLM_RX0_LSH     2
#define IOMUX_GPR0_SSI3_RX1_SLM_RX1_LSH     3
#define IOMUX_GPR0_SSI2_TX0_SLM_TX2_LSH     4
#define IOMUX_GPR0_SSI2_RX0_SLM_RX2_LSH     5
#define IOMUX_GPR0_SSI2_TX1_SLM_TX3_LSH     6
#define IOMUX_GPR0_ESDHC2_I2C2_LSH          7
#define IOMUX_GPR0_ESDHC1_I2C1_LSH          8
#define IOMUX_GPR0_GPU_GPIO1_0_LSH          9

#define IOMUX_SW_MUX_CTL_MUX_MODE_LSH       0
#define IOMUX_SW_MUX_CTL_SION_LSH           4

#define IOMUX_SW_PAD_CTL_SRE_LSH            0
#define IOMUX_SW_PAD_CTL_DSE_LSH            1
#define IOMUX_SW_PAD_CTL_ODE_LSH            3
#define IOMUX_SW_PAD_CTL_PUS_LSH            4
#define IOMUX_SW_PAD_CTL_PUE_LSH            6
#define IOMUX_SW_PAD_CTL_PKE_LSH            7
#define IOMUX_SW_PAD_CTL_HYS_LSH            8
#define IOMUX_SW_PAD_CTL_DDR_INPUT_LSH      9
#define IOMUX_SW_PAD_CTL_HVE_LSH            13

#define IOMUX_SELECT_INPUT_DAISY_LSH        0

//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
#define IOMUX_GPR0_SSI3_TX0_SLM_TX0_WID     1
#define IOMUX_GPR0_SSI3_TX1_SLM_TX1_WID     1     
#define IOMUX_GPR0_SSI3_RX0_SLM_RX0_WID     1
#define IOMUX_GPR0_SSI3_RX1_SLM_RX1_WID     1
#define IOMUX_GPR0_SSI2_TX0_SLM_TX2_WID     1
#define IOMUX_GPR0_SSI2_RX0_SLM_RX2_WID     1
#define IOMUX_GPR0_SSI2_TX1_SLM_TX3_WID     1
#define IOMUX_GPR0_ESDHC2_I2C2_WID          1
#define IOMUX_GPR0_ESDHC1_I2C1_WID          1
#define IOMUX_GPR0_GPU_GPIO1_0_WID          1

#define IOMUX_SW_MUX_CTL_MUX_MODE_WID       3
#define IOMUX_SW_MUX_CTL_SION_WID           1

#define IOMUX_SW_PAD_CTL_SRE_WID            1
#define IOMUX_SW_PAD_CTL_DSE_WID            2
#define IOMUX_SW_PAD_CTL_ODE_WID            1
#define IOMUX_SW_PAD_CTL_PUS_WID            2
#define IOMUX_SW_PAD_CTL_PUE_WID            1
#define IOMUX_SW_PAD_CTL_PKE_WID            1
#define IOMUX_SW_PAD_CTL_HYS_WID            1
#define IOMUX_SW_PAD_CTL_DDR_INPUT_WID      1
#define IOMUX_SW_PAD_CTL_HVE_WID            1

#define IOMUX_SELECT_INPUT_DAISY_WID        4

//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------

// SW_MUX_CTL
#define IOMUX_SW_MUX_CTL_MUX_MODE_ALT0      0   // Select ALT0 mux mode
#define IOMUX_SW_MUX_CTL_MUX_MODE_ALT1      1   // Select ALT1 mux mode
#define IOMUX_SW_MUX_CTL_MUX_MODE_ALT2      2   // Select ALT2 mux mode
#define IOMUX_SW_MUX_CTL_MUX_MODE_ALT3      3   // Select ALT3 mux mode
#define IOMUX_SW_MUX_CTL_MUX_MODE_ALT4      4   // Select ALT4 mux mode
#define IOMUX_SW_MUX_CTL_MUX_MODE_ALT5      5   // Select ALT5 mux mode
#define IOMUX_SW_MUX_CTL_MUX_MODE_ALT6      6   // Select ALT6 mux mode
#define IOMUX_SW_MUX_CTL_MUX_MODE_ALT7      7   // Select ALT7 mux mode

#define IOMUX_SW_MUX_CTL_SION_REGULAR       0   // Input is determined by mux mode
#define IOMUX_SW_MUX_CTL_SION_FORCE         1   // Force input of some pad

// SW_PAD_CTL
#define IOMUX_SW_PAD_CTL_SRE_SLOW           0   // Slow slew rate
#define IOMUX_SW_PAD_CTL_SRE_FAST           1   // Fast slew rate

#define IOMUX_SW_PAD_CTL_DSE_NORMAL         0   // Normal drive strength
#define IOMUX_SW_PAD_CTL_DSE_MEDIUM         1   // Medium drive strength
#define IOMUX_SW_PAD_CTL_DSE_HIGH           2   // High drive strength
#define IOMUX_SW_PAD_CTL_DSE_MAX            3   // Maximum drive strength

#define IOMUX_SW_PAD_CTL_ODE_DISABLE        0   // Disable open drain
#define IOMUX_SW_PAD_CTL_ODE_ENABLE         1   // Enable open drain

#define IOMUX_SW_PAD_CTL_PUS_100K_DOWN      0   // 100K Ohm pull down
#define IOMUX_SW_PAD_CTL_PUS_47K_UP         1   // 47K Ohm pull up
#define IOMUX_SW_PAD_CTL_PUS_100K_UP        2   // 100K Ohm pull up
#define IOMUX_SW_PAD_CTL_PUS_22K_UP         3   // 22K Ohm pull up

#define IOMUX_SW_PAD_CTL_PUE_KEEPER         0  // Keeper enable
#define IOMUX_SW_PAD_CTL_PUE_PULL           1  // Pull up/down enable

#define IOMUX_SW_PAD_CTL_PKE_DISABLE        0   // Pull up/down/keeper disabled
#define IOMUX_SW_PAD_CTL_PKE_ENABLE         1   // Pull up/down/keeper enabled

#define IOMUX_SW_PAD_CTL_HYS_DISABLE        0   // Disable hysteresis
#define IOMUX_SW_PAD_CTL_HYS_ENABLE         1   // Enable hysteresis

#define IOMUX_SW_PAD_CTL_DDR_INPUT_CMOS     0   // CMOS input
#define IOMUX_SW_PAD_CTL_DDR_INPUT_DDR      1   // DDR input

#define IOMUX_SW_PAD_CTL_HVE_LOW            0   // Low output voltage
#define IOMUX_SW_PAD_CTL_HVE_HIGH           1   // High output voltage


#endif // __MX51_IOMUX_H__
