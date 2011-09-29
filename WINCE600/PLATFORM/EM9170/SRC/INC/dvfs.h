//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  dvfs.h
//
//  This file contains macros for the supporting DVFS (Dynamic Voltage and
//  Frequency scaling).
//
//------------------------------------------------------------------------------
#ifndef __DVFS_H
#define __DVFS_H

//------------------------------------------------------------------------------
// LOAD TRACKING CONFIGURATION
//------------------------------------------------------------------------------
#define BSP_DVFS_LOADTRACK_MSEC             50  // 50 msec tracking window
#define BSP_DVFS_LOADTRACK_UP_PCT           25  // <25% idle -> raise setpoint
#define BSP_DVFS_LOADTRACK_UP_MSEC          2*BSP_DVFS_LOADTRACK_MSEC
#define BSP_DVFS_LOADTRACK_DN_PCT           85  // >85% idle -> lower setpoint
#define BSP_DVFS_LOADTRACK_DN_MSEC          5*BSP_DVFS_LOADTRACK_MSEC


//------------------------------------------------------------------------------
// DVFS SETPOINT CONFIGURATION
//------------------------------------------------------------------------------

// ----- CPU DOMAIN ----- //

// HIGH SETPOINT
#define BSP_DVFS_CPU_HIGH_PLL_FREQ          532000000   // Hz
#define BSP_DVFS_CPU_HIGH_ARM_SRC           1           // 0.75 * 532 MHz
#define BSP_DVFS_CPU_HIGH_ARM_CLK_DIV       0           // div by 1 (399 MHz)
#define BSP_DVFS_CPU_HIGH_AHB_CLK_DIV       2           // div by 3 (133 MHz)
#define BSP_DVFS_CPU_HIGH_mV                1450        // mV
#if (BSP_DVFS_CPU_HIGH_ARM_SRC == 1)
#define BSP_DVFS_CPU_HIGH_ARM_FREQ  \
        ((BSP_DVFS_CPU_HIGH_PLL_FREQ * 3) / (4*(BSP_DVFS_CPU_HIGH_ARM_CLK_DIV+1)))
#else
#define BSP_DVFS_CPU_HIGH_ARM_FREQ  \
        ((BSP_DVFS_CPU_HIGH_PLL_FREQ) / (BSP_DVFS_CPU_HIGH_ARM_CLK_DIV+1))
#endif
#define BSP_DVFS_CPU_HIGH_AHB_FREQ \
        (BSP_DVFS_CPU_HIGH_ARM_FREQ / (BSP_DVFS_CPU_HIGH_AHB_CLK_DIV+1))

// MEDIUM SETPOINT
#define BSP_DVFS_CPU_MED_PLL_FREQ           532000000   // Hz
#define BSP_DVFS_CPU_MED_ARM_SRC            0           // 1 * 532 MHz
#define BSP_DVFS_CPU_MED_ARM_CLK_DIV        1           // div by 2 (266 MHz)
#define BSP_DVFS_CPU_MED_AHB_CLK_DIV        1           // div by 2 (133 MHz)
#define BSP_DVFS_CPU_MED_mV                 1196        // mV
#if (BSP_DVFS_CPU_MED_ARM_SRC == 1)
#define BSP_DVFS_CPU_MED_ARM_FREQ  \
        ((BSP_DVFS_CPU_MED_PLL_FREQ * 3) / (4*(BSP_DVFS_CPU_MED_ARM_CLK_DIV+1)))
#else
#define BSP_DVFS_CPU_MED_ARM_FREQ  \
        ((BSP_DVFS_CPU_MED_PLL_FREQ) / (BSP_DVFS_CPU_MED_ARM_CLK_DIV+1))
#endif
#define BSP_DVFS_CPU_MED_AHB_FREQ \
        (BSP_DVFS_CPU_MED_ARM_FREQ / (BSP_DVFS_CPU_MED_AHB_CLK_DIV+1))

// LOW SETPOINT
#define BSP_DVFS_CPU_LOW_PLL_FREQ           532000000   // Hz
#define BSP_DVFS_CPU_LOW_ARM_SRC            1           // 0.75 * 532 MHz
#define BSP_DVFS_CPU_LOW_ARM_CLK_DIV        2           // div by 3 (133 MHz)
#define BSP_DVFS_CPU_LOW_AHB_CLK_DIV        0           // div by 1 (133 MHz)
#define BSP_DVFS_CPU_LOW_mV                 1196        // mV
#if (BSP_DVFS_CPU_LOW_ARM_SRC == 1)
#define BSP_DVFS_CPU_LOW_ARM_FREQ  \
        ((BSP_DVFS_CPU_LOW_PLL_FREQ * 3) / (4*(BSP_DVFS_CPU_LOW_ARM_CLK_DIV+1)))
#else
#define BSP_DVFS_CPU_LOW_ARM_FREQ  \
        ((BSP_DVFS_CPU_LOW_PLL_FREQ) / (BSP_DVFS_CPU_LOW_ARM_CLK_DIV+1))
#endif
#define BSP_DVFS_CPU_LOW_AHB_FREQ \
        (BSP_DVFS_CPU_LOW_ARM_FREQ / (BSP_DVFS_CPU_LOW_AHB_CLK_DIV+1))

#endif  // __DVFS_H
