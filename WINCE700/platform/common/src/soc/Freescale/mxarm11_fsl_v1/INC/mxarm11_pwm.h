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
//  Header: mxarm11_pwm.h
//
//  Provides definitions for PWM module based on Freescale ARM11 chassis.
//
//------------------------------------------------------------------------------

#ifndef __MXARM11_PWM_H
#define __MXARM11_PWM_H

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
     REG32 PWM_CR;
     REG32 PWM_SR;
     REG32 PWM_IR;
     REG32 PWM_SAR;
     REG32 PWM_PR;
     REG32 PWM_CNR;
} CSP_PWM_REG, *PCSP_PWM_REG;

//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------

#define   PWM_CR_OFFSET         (0x0000)
#define   PWM_SR_OFFSET         (0x0004)
#define   PWM_IR_OFFSET         (0x0008)
#define   PWM_SAR_OFFSET        (0x000C)
#define   PWM_PR_OFFSET         (0x0010)
#define   PWM_CNR_OFFSET        (0x0014)

//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
#define   PWM_CR_EN_LSH             0
#define   PWM_CR_REPEAT_LSH         1
#define   PWM_CR_SWR_LSH            3
#define   PWM_CR_PRESCALER_LSH      4
#define   PWM_CR_CLKSRC_LSH         16
#define   PWM_CR_POUTC_LSH          18
#define   PWM_CR_HCTR_LSH           20
#define   PWM_CR_BCTR_LSH           21
#define   PWM_CR_DBGEN_LSH          22
#define   PWM_CR_WAITEN_LSH         23
#define   PWM_CR_DOZEN_LSH          24
#define   PWM_CR_STOPEN_LSH         25
#define   PWM_CR_FWM_LSH            26

#define   PWM_SR_FIFOAV_LSH         0
#define   PWM_SR_FE_LSH             3
#define   PWM_SR_ROV_LSH            4
#define   PWM_SR_CMP_LSH            5

#define  PWM_IR_FIE_LSH             0
#define  PWM_IR_RIE_LSH             1
#define  PWM_IR_CIE_LSH             2

//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------

#define   PWM_CR_EN_WID             1
#define   PWM_CR_REPEAT_WID         2
#define   PWM_CR_SWR_WID            1
#define   PWM_CR_PRESCALER_WID      12
#define   PWM_CR_CLKSRC_WID         2
#define   PWM_CR_POUTC_WID          2
#define   PWM_CR_HCTR_WID           1
#define   PWM_CR_BCTR_WID           1
#define   PWM_CR_DBGEN_WID          1
#define   PWM_CR_WAITEN_WID         1
#define   PWM_CR_DOZEN_WID          1
#define   PWM_CR_STOPEN_WID         1
#define   PWM_CR_FWM_WID            2

#define   PWM_SR_FIFOAV_WID         3
#define   PWM_SR_FE_WID             1
#define   PWM_SR_ROV_WID            1
#define   PWM_SR_CMP_WID            1

#define   PWM_IR_FIE_WID            1
#define   PWM_IR_RIE_WID            1
#define   PWM_IR_CIE_WID            1

//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------
#define   PWM_CR_EN_ENABLE          1
#define   PWM_CR_EN_DISABLE         0

#define   PWM_CR_SWR_RESET          1
#define   PWM_CR_SWR_NORESET        0

#define  PWM_CR_HCTR_ENABLE         1
#define  PWM_CR_HCTR_DISABLE        0

#define   PWM_CR_BCTR_ENABLE        1
#define   PWM_CR_BCTR_DISABLE       0

#define  PWM_CR_DBGEN_ACTIVE        1
#define  PWM_CR_DBGEN_INACTIVE      0

#define  PWM_CR_WAITEN_ACTIVE       1
#define  PWM_CR_WAITEN_INACTIVE     0

#define  PWM_CR_DOZEN_ACTIVE        1
#define  PWM_CR_DOZEN_INACTIVE      0

#define  PWM_CR_STOPEN_ACTIVE       1
#define  PWM_CR_STOPEN_INACTIVE     0

#define PWM_IR_CIE_ENABLE           1
#define PWM_IR_CIE_DISABLE          0
#define PWM_IR_RIE_ENABLE           1
#define PWM_IR_RIE_DISABLE          0
#define PWM_IR_FIE_ENABLE           1
#define PWM_IR_FIE_DISABLE          0

#define PWM_SR_CMP_STATUS_CLEAR     1  //  status clear
#define PWM_SR_ROV_STATUS_CLEAR     1  //  status clear
#define PWM_SR_FE_STATUS_CLEAR      1  //  status clear

//------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------
#define PWM_PWMCR_CLKSRC_OFF                   0
#define PWM_PWMCR_CLKSRC_IPG_CLK               1
#define PWM_PWMCR_CLKSRC_IPG_CLK_HIGHFREQ      2
#define PWM_PWMCR_CLKSRC_IPG_CLK_32K           3

//------------------------------------------------------------------------------
// FUNCTION PROTOTYPES
//------------------------------------------------------------------------------



#ifdef __cplusplus
}
#endif

#endif // __MXARM11_PWM_H
