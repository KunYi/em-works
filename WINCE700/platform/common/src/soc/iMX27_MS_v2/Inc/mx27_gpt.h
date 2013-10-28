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
// Copyright (C) 2004, MOTOROLA, INC. All Rights Reserved
//
//------------------------------------------------------------------------------
//
// Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header:  mx27_gpt.h
//
//  Provides definitions for GPT module based on i.MX27.
//
//------------------------------------------------------------------------------
#ifndef __MX27_GPT_H
#define __MX27_GPT_H

#ifdef __cplusplus
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
     REG32 TCTL;
     REG32 TPRER;
     REG32 TCMP;
     REG32 TCR;
     REG32 TCN;
     REG32 TSTAT;
} CSP_GPT_REGS, *PCSP_GPT_REGS;

//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
#define GPT_TCTL_OFFSET         0x0000
#define GPT_TPRER_OFFSET        0x0004
#define GPT_TCMP_OFFSET         0x0008
#define GPT_TCR_OFFSET          0x000C
#define GPT_TCN_OFFSET          0x0010
#define GPT_TSTAT_OFFSET        0x0014

//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
#define GPT_TCTL_TEN_LSH        0
#define GPT_TCTL_CLKSOURCE_LSH  1
#define GPT_TCTL_COMPEN_LSH     4
#define GPT_TCTL_CAPTEN_LSH     5
#define GPT_TCTL_CAP_LSH        6
#define GPT_TCTL_FRR_LSH        8
#define GPT_TCTL_OM_LSH         9
#define GPT_TCTL_CC_LSH         10
#define GPT_TCTL_SWR_LSH        15

#define GPT_TPRER_PRESCALER_LSH 0

#define GPT_TCMP_COMPARE_LSH    0

#define GPT_TCR_CAPTURE_LSH     0

#define GPT_TCN_COUNT_LSH       0

#define GPT_TSTAT_COMP_LSH      0
#define GPT_TSTAT_CAPT_LSH      1

//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
#define GPT_TCTL_TEN_WID        1
#define GPT_TCTL_CLKSOURCE_WID  3
#define GPT_TCTL_COMPEN_WID     1
#define GPT_TCTL_CAPTEN_WID     1
#define GPT_TCTL_CAP_WID        2
#define GPT_TCTL_FRR_WID        1
#define GPT_TCTL_OM_WID         1
#define GPT_TCTL_CC_WID         1
#define GPT_TCTL_SWR_WID        1

#define GPT_TPRER_PRESCALER_WID 10

#define GPT_TCMP_COMPARE_WID    32

#define GPT_TCR_CAPTURE_WID     32

#define GPT_TCN_COUNT_WID       32

#define GPT_TSTAT_COMP_WID      1
#define GPT_TSTAT_CAPT_WID      1

//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------

// GPTCR
#define GPT_TCTL_TEN_ENABLE             1 // GPT enabled
#define GPT_TCTL_TEN_DISABLE            0 // GPT disabled

#define GPT_TCTL_CLKSOURCE_DISABLE      0 // clock disabled
#define GPT_TCTL_CLKSOURCE_PERCLK1      1 // PERCLK1 to prescaler
#define GPT_TCTL_CLKSOURCE_PERCLK1DIV4  2 // PERCLK1 divided by 4 to prescaler
#define GPT_TCTL_CLKSOURCE_TIN          3 // TIN to prescaler
#define GPT_TCTL_CLKSOURCE_32KCLK       7 // 32kHz clock to prescaler

#define GPT_TCTL_COMPEN_ENABLE          1 // Compare interrupt enabled
#define GPT_TCTL_COMPEN_DISABLE         0 // Compare interrupt disabled

#define GPT_TCTL_CAPTEN_ENABLE          1 // Capture interrupt enabled
#define GPT_TCTL_CAPTEN_DISABLE         0 // Capture interrupt disabled

#define GPT_TCTL_CAP_DISABLE            0 // Capture function disabled
#define GPT_TCTL_CAP_RISING             1 // Capture on rising edge
#define GPT_TCTL_CAP_FALLING            1 // Capture on falling edge
#define GPT_TCTL_CAP_BOTH               0 // Capture on both edges

#define GPT_TCTL_FRR_FREERUN            1 // Freerun mode (counter continues after compare)
#define GPT_TCTL_FRR_RESTART            0 // Restart mode (counter set to zero after compare)

#define GPT_TCTL_OM_ACTIVELOW           0 // Output active low pulse for 1 clock
#define GPT_TCTL_OM_TOGGLE              1 // Toggle output

#define GPT_TCTL_CC_ENABLE              1 // Counter reset when TEN = 0
#define GPT_TCTL_CC_DISABLE             0 // Counter halted at current count when TEN = 0

#define GPT_TCTL_SWR_RESET              1 // Self-clearing software reset
#define GPT_TCTL_SWR_NORESET            0 // Do not activate software reset

// TPRER
#define GPT_TPRER_PRESCALER_MAX         0x7FF

// TCN
#define GPT_TCN_COUNT_MAX               0xFFFFFFFF

// TSTAT
#define GPT_TSTAT_COMP_NOEVENT          0 // No compare event
#define GPT_TSTAT_COMP_EVENT            1 // A compare event has occurred
#define GPT_TSTAT_COMP_NORESET          0 // No reset
#define GPT_TSTAT_COMP_RESET            1 // Reset


#define GPT_TSTAT_CAPT_NOEVENT          0 // No capture event
#define GPT_TSTAT_CAPT_EVENT            1 // A capture event has occurred
#define GPT_TSTAT_CAPT_NORESET          0 // No reset
#define GPT_TSTAT_CAPT_RESET            1 // Reset


#ifdef __cplusplus
}
#endif

#endif // __MX27_GPT_H
