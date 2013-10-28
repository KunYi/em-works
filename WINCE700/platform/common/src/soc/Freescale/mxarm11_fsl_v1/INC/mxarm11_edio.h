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
//  Header:  mxarm11_edio.h
//
//  Provides definitions for EDIO module based on Freescale ARM11 chassis.
//
//------------------------------------------------------------------------------
#ifndef __MXARM11_EDIO_H
#define __MXARM11_EDIO_H


#if __cplusplus
extern "C" {
#endif


//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------
#define EDIO_PINS_PER_PORT          8


//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------
typedef struct
{
    UINT16 EPPAR;
    UINT16 EPDDR;
    UINT16 EPDR;
    UINT16 EPFR;
} CSP_EDIO_REGS, *PCSP_EDIO_REGS;


//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
#define EDIO_EPPAR_OFFSET               0x0000
#define EDIO_EPDDR_OFFSET               0x0002
#define EDIO_EPDR_OFFSET                0x0004
#define EDIO_EPFR_OFFSET                0x0006


//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
#define EDIO_EPPAR_EPPAR_LSH            0

#define EDIO_EPDDR_EPDD_LSH             0

#define EDIO_EPDR_EPD_LSH               0

#define EDIO_EPFR_EPF_LSH               0


//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
#define EDIO_EPPAR_EPPAR_WID            2

#define EDIO_EPDDR_EPDD_WID             1

#define EDIO_EPDR_EPD_WID               1

#define EDIO_EPFR_EPF_WID               1


//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------
#define EDIO_EPPAR_EPPAR_LOW_LEVEL      0
#define EDIO_EPPAR_EPPAR_RISING_EDGE    1
#define EDIO_EPPAR_EPPAR_FALLING_EDGE   2
#define EDIO_EPPAR_EPPAR_EITHER_EDGE    3

#define EDIO_EPDDR_EPDD_INPUT           0
#define EDIO_EPDDR_EPDD_OUTPUT          1

#define EDIO_EPFR_EPF_EDGE_NOT_DETECTED 0
#define EDIO_EPFR_EPF_EDGE_DETECTED     1

//------------------------------------------------------------------------------
// HELPER MACROS
//------------------------------------------------------------------------------
#define EDIO_PIN_MASK(pin)              (1U << pin)
#define EDIO_EPPAR_MASK(pin)            (0x3 << (pin << 1))
#define EDIO_EPPAR_VAL(val, pin)        (val << (pin << 1))


#ifdef __cplusplus
}
#endif

#endif // __MXARM11_EDIO_H
