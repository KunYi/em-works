//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2010, Freescale Semiconductor, Inc. All Rights Reserved.
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
#define BSP_DVFS_LOADTRACK_DN_PCT           70  // >70% idle -> lower setpoint
#define BSP_DVFS_LOADTRACK_DN_MSEC          5*BSP_DVFS_LOADTRACK_MSEC

//------------------------------------------------------------------------------
// DVFS SETPOINT CONFIGURATION
//------------------------------------------------------------------------------

// ----- CPU DOMAIN ----- //

// HIGH SETPOINT
#define BSP_DVFS_CPU_HIGH_REF_CPU_FREQ      454         // 454MHz
#define BSP_DVFS_CPU_HIGH_FRAC_CPUFRAC      19   
#define BSP_DVFS_CPU_HIGH_ARM_CLK_DIV       1           // div by 1 (454MHz)
#define BSP_DVFS_CPU_HIGH_AHB_CLK_DIV       3           // div by 3 (151)
#define BSP_DVFS_CPU_HIGH_mV                1550        // mV
#define BSP_DVFS_CPU_HIGH_BO_mV             1450        // mV

#define BSP_DVFS_CPU_HIGH_REF_EMI_FREQ      392         //392MHz
#define BSP_DVFS_CPU_HIGH_FRAC_EMIFRAC      22   
#define BSP_DVFS_CPU_HIGH_EMI_CLK_DIV       2           // div by 2 (196MHz)


#define BSP_DVFS_CPU_HIGH_ARM_FREQ  \
        ((BSP_DVFS_CPU_HIGH_REF_CPU_FREQ) / (BSP_DVFS_CPU_HIGH_ARM_CLK_DIV))
#define BSP_DVFS_CPU_HIGH_AHB_FREQ \
        ((BSP_DVFS_CPU_HIGH_ARM_FREQ) / (BSP_DVFS_CPU_HIGH_AHB_CLK_DIV))
#define BSP_DVFS_CPU_HIGH_EMI_FREQ \
        ((BSP_DVFS_CPU_HIGH_REF_EMI_FREQ) / (BSP_DVFS_CPU_HIGH_EMI_CLK_DIV))


// MEDIUM SETPOINT
#define BSP_DVFS_CPU_MED_REF_CPU_FREQ       261         //261 Hz
#define BSP_DVFS_CPU_MED_FRAC_CPUFRAC       33       
#define BSP_DVFS_CPU_MED_ARM_CLK_DIV        1           // div by 1 (261MHz)
#define BSP_DVFS_CPU_MED_AHB_CLK_DIV        2           // div by 2 (130MHz)
#define BSP_DVFS_CPU_MED_mV                 1350        // mV
#define BSP_DVFS_CPU_MED_BO_mV              1250         // mV

#define  DVFC_DDR2_FREQ_CHANGE

#ifdef  DVFC_DDR2_FREQ_CHANGE
#define BSP_DVFS_CPU_MED_REF_EMI_FREQ       261         // 261
#define BSP_DVFS_CPU_MED_FRAC_EMIFRAC       33   
#define BSP_DVFS_CPU_MED_EMI_CLK_DIV        2           // div by 2 (130 MHz)
#else
#define BSP_DVFS_CPU_MED_REF_EMI_FREQ       392         // 392
#define BSP_DVFS_CPU_MED_FRAC_EMIFRAC       22   
#define BSP_DVFS_CPU_MED_EMI_CLK_DIV        2           // div by 2 (196 MHz)
#endif
#define BSP_DVFS_CPU_MED_ARM_FREQ  \
        ((BSP_DVFS_CPU_MED_REF_CPU_FREQ) / (BSP_DVFS_CPU_MED_ARM_CLK_DIV))
#define BSP_DVFS_CPU_MED_AHB_FREQ \
        ((BSP_DVFS_CPU_MED_ARM_FREQ) / (BSP_DVFS_CPU_MED_AHB_CLK_DIV))
#define BSP_DVFS_CPU_MED_EMI_FREQ \
        ((BSP_DVFS_CPU_MED_REF_EMI_FREQ) / (BSP_DVFS_CPU_MED_EMI_CLK_DIV))

// LOW SETPOINT
#define BSP_DVFS_CPU_LOW_REF_CPU_FREQ       320         // 320MHz
#define BSP_DVFS_CPU_LOW_FRAC_CPUFRAC       27          
#define BSP_DVFS_CPU_LOW_ARM_CLK_DIV        5           // div by 1 (64MHz)
#define BSP_DVFS_CPU_LOW_AHB_CLK_DIV        1           // div by 2 ( 64 MHz)
#define BSP_DVFS_CPU_LOW_mV                 1350        // mV
#define BSP_DVFS_CPU_LOW_BO_mV              1250         // mV

#ifdef  DVFC_DDR2_FREQ_CHANGE
#define BSP_DVFS_CPU_LOW_REF_EMI_FREQ       261          // 332MHz
#define BSP_DVFS_CPU_LOW_FRAC_EMIFRAC       33    
#define BSP_DVFS_CPU_LOW_EMI_CLK_DIV        2            // div by 2(130.5MHz)
#else
#define BSP_DVFS_CPU_LOW_REF_EMI_FREQ       392         // 480MHz
#define BSP_DVFS_CPU_LOW_FRAC_EMIFRAC       22   
#define BSP_DVFS_CPU_LOW_EMI_CLK_DIV        2           // div by 2 (200 MHz)
#endif

#define BSP_DVFS_CPU_LOW_ARM_FREQ  \
        ((BSP_DVFS_CPU_LOW_REF_CPU_FREQ) / (BSP_DVFS_CPU_LOW_ARM_CLK_DIV))
#define BSP_DVFS_CPU_LOW_AHB_FREQ \
        ((BSP_DVFS_CPU_LOW_ARM_FREQ) / (BSP_DVFS_CPU_LOW_AHB_CLK_DIV))
#define BSP_DVFS_CPU_LOW_EMI_FREQ \
            ((BSP_DVFS_CPU_LOW_REF_EMI_FREQ) / (BSP_DVFS_CPU_LOW_EMI_CLK_DIV))

#endif  // __DVFS_H
