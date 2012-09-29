//------------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header:  mx25_dryice.h
//
//  Provides definitions for DryIce module of the MX25
//
//------------------------------------------------------------------------------

#ifndef __MX25_DRYICE_H
#define __MX25_DRYICE_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------
typedef struct {
    REG32 DTCMR;        //Time Counter MSB Register
    REG32 DTCLR;        //Time Counter LSB Register
    REG32 DCAMR;        //Clock Alarm MSB Register
    REG32 DCALR;        //Clock Alarm LSB Register
    REG32 DCR;          //Control Register
    REG32 DSR;          //Status Register
    REG32 DIER;         //Interrupt Enable Register
    REG32 DMCR;         //Monotonic Counter Register
    REG32 DKSR;         //Key Select Register
    REG32 DKCR;         //Key Control Register
    REG32 DTCR;         //Tamper Configuration Register
    REG32 DACR;         //Analog Configuration Register
    REG32 reserved[3];
    REG32 DGPR;         //General Purpose Register
    REG32 DPKR0;        //Programmed Key Register 0
    REG32 DPKR1;        //Programmed Key Register 1
    REG32 DPKR2;        //Programmed Key Register 2
    REG32 DPKR3;        //Programmed Key Register 3
    REG32 DPKR4;        //Programmed Key Register 4
    REG32 DPKR5;        //Programmed Key Register 5
    REG32 DPKR6;        //Programmed Key Register 6
    REG32 DPKR7;        //Programmed Key Register 7
    REG32 DRKR0;        //Random Key Register 0
    REG32 DRKR1;        //Random Key Register 1
    REG32 DRKR2;        //Random Key Register 2
    REG32 DRKR3;        //Random Key Register 3
    REG32 DRKR4;        //Random Key Register 4
    REG32 DRKR5;        //Random Key Register 5
    REG32 DRKR6;        //Random Key Register 6
    REG32 DRKR7;        //Random Key Register 7
} CSP_DRYICE_REGS, *PCSP_DRYICE_REGS;

//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
#define DRYICE_TIME_COUNTER_MSB_OFFSET                  0x00
#define DRYICE_TIME_COUNTER_LSB_OFFSET                  0x04
#define DRYICE_CLOCK_ALARM_MSB_OFFSET                   0x08
#define DRYICE_CLOCK_ALARM_LSB_OFFSET                   0x0C
#define DRYICE_CONTROL_OFFSET                           0x10
#define DRYICE_STATUS_OFFSET                            0x14
#define DRYICE_INTERRUPT_ENABLE_OFFSET                  0x18
#define DRYICE_MONOTONIC_COUNTER_OFFSET                 0x1C
#define DRYICE_KEY_SELECT_OFFSET                        0x20
#define DRYICE_KEY_CONTROL_OFFSET                       0x24
#define DRYICE_TAMPER_CONFIG_OFFSET                     0x28
#define DRYICE_ANALOG_CONFIG_OFFSET                     0x2C
#define DRYICE_GENERAL_PURPOSE_OFFSET                   0x3C
#define DRYICE_PROGRAMED_KEY_0_OFFSET                   0x40
#define DRYICE_PROGRAMED_KEY_1_OFFSET                   0x44
#define DRYICE_PROGRAMED_KEY_2_OFFSET                   0x48
#define DRYICE_PROGRAMED_KEY_3_OFFSET                   0x4C
#define DRYICE_PROGRAMED_KEY_4_OFFSET                   0x50
#define DRYICE_PROGRAMED_KEY_5_OFFSET                   0x54
#define DRYICE_PROGRAMED_KEY_6_OFFSET                   0x58
#define DRYICE_PROGRAMED_KEY_7_OFFSET                   0x5C
#define DRYICE_RANDOM_KEY_0_OFFSET                      0x60
#define DRYICE_RANDOM_KEY_1_OFFSET                      0x64
#define DRYICE_RANDOM_KEY_2_OFFSET                      0x68
#define DRYICE_RANDOM_KEY_3_OFFSET                      0x6C
#define DRYICE_RANDOM_KEY_4_OFFSET                      0x70
#define DRYICE_RANDOM_KEY_5_OFFSET                      0x74
#define DRYICE_RANDOM_KEY_6_OFFSET                      0x78
#define DRYICE_RANDOM_KEY_7_OFFSET                      0x7C

//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
#define DRYICE_DCR_TCE_LSH                              3
#define DRYICE_DCR_APE_LSH                              4
#define DRYICE_DCR_OSCB_LSH                             14
#define DRYICE_DCR_NSA_LSH                              15


#define DRYICE_DSR_CAF_LSH                              4
#define DRYICE_DIER_CAIE_LSH                            4

//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
#define DRYICE_DCR_TCE_WID                              1
#define DRYICE_DCR_APE_WID                              1
#define DRYICE_DCR_OSCB_WID                             1
#define DRYICE_DCR_NSA_WID                              1

#define DRYICE_DSR_CAF_WID                              1

#define DRYICE_DIER_CAIE_WID                            1
//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------
#define DRYICE_DCR_APE_ENABLE                           1
#define DRYICE_DCR_APE_DISABLE                          0


#define DRYICE_DIER_CAIE_ENABLE                           1
#define DRYICE_DIER_CAIE_DISABLE                          0



#ifdef __cplusplus
}
#endif

#endif // __MX25_DRYICE_H
