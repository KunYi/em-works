//------------------------------------------------------------------------------
//
//  Copyright (C) 2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header: MX35_mlb.h
//
//  Provides definitions for the MLB module that
//  is available on MX35 Freescale SOCs.
//
//------------------------------------------------------------------------------
#ifndef __MX35_MLB_H
#define __MX35_MLB_H

#if    __cplusplus
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
    // General Registers
    UINT32 DCCR;
    UINT32 SSCR;
    UINT32 SDCR;
    UINT32 SMCR;
    UINT32 _reserved0[(0x01C - 0x010) / sizeof(UINT32)];
    UINT32 VCCR;
    UINT32 SBCR;
    UINT32 ABCR;
    UINT32 CBCR;
    UINT32 IBCR;
    UINT32 CICR;
    UINT32 _reserved1[(0x040 - 0x034) / sizeof(UINT32)];

    // Channel 0 Registers
    UINT32 CECR0;
    UINT32 CSCR0;
    UINT32 CCBCR0;
    UINT32 CNBCR0;

    // Channel 1 Registers
    UINT32 CECR1;
    UINT32 CSCR1;
    UINT32 CCBCR1;
    UINT32 CNBCR1;

    // Channel 2 Registers
    UINT32 CECR2;
    UINT32 CSCR2;
    UINT32 CCBCR2;
    UINT32 CNBCR2;

    // Channel 3 Registers
    UINT32 CECR3;
    UINT32 CSCR3;
    UINT32 CCBCR3;
    UINT32 CNBCR3;

    // Channel 4 Registers
    UINT32 CECR4;
    UINT32 CSCR4;
    UINT32 CCBCR4;
    UINT32 CNBCR4;

    // Channel 5 Registers
    UINT32 CECR5;
    UINT32 CSCR5;
    UINT32 CCBCR5;
    UINT32 CNBCR5;

    // Channel 6 Registers
    UINT32 CECR6;
    UINT32 CSCR6;
    UINT32 CCBCR6;
    UINT32 CNBCR6;

    // Channel 7 Registers
    UINT32 CECR7;
    UINT32 CSCR7;
    UINT32 CCBCR7;
    UINT32 CNBCR7;

    // Channel 8 Registers
    UINT32 CECR8;
    UINT32 CSCR8;
    UINT32 CCBCR8;
    UINT32 CNBCR8;

    // Channel 9 Registers
    UINT32 CECR9;
    UINT32 CSCR9;
    UINT32 CCBCR9;
    UINT32 CNBCR9;

    // Channel 10 Registers
    UINT32 CECR10;
    UINT32 CSCR10;
    UINT32 CCBCR10;
    UINT32 CNBCR10;

    // Channel 11 Registers
    UINT32 CECR11;
    UINT32 CSCR11;
    UINT32 CCBCR11;
    UINT32 CNBCR11;

    // Channel 12 Registers
    UINT32 CECR12;
    UINT32 CSCR12;
    UINT32 CCBCR12;
    UINT32 CNBCR12;

    // Channel 13 Registers
    UINT32 CECR13;
    UINT32 CSCR13;
    UINT32 CCBCR13;
    UINT32 CNBCR13;

    // Channel 14 Registers
    UINT32 CECR14;
    UINT32 CSCR14;
    UINT32 CCBCR14;
    UINT32 CNBCR14;

    // Channel 15 Registers
    UINT32 CECR15;
    UINT32 CSCR15;
    UINT32 CCBCR15;
    UINT32 CNBCR15;

    // Reserved Area
    UINT32 _reserved2[(0x280 - 0x140) / sizeof(UINT32)];
    
    // Local Channel Buffers
    UINT32 LCBCR0;
    UINT32 LCBCR1;
    UINT32 LCBCR2;
    UINT32 LCBCR3;
    UINT32 LCBCR4;
    UINT32 LCBCR5;
    UINT32 LCBCR6;
    UINT32 LCBCR7;
    UINT32 LCBCR8;
    UINT32 LCBCR9;
    UINT32 LCBCR10;
    UINT32 LCBCR11;
    UINT32 LCBCR12;
    UINT32 LCBCR13;
    UINT32 LCBCR14;
    UINT32 LCBCR15;
} CSP_MLB_REGS, *PCSP_MLB_REGS;

//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------
#define MLB_CSCR_CBS    (0x1 << 3)      // Current Buffer Start
#define MLB_CSCR_CBD    (0x1 << 2)      // Current Buffer Done

#ifdef __cplusplus
}
#endif

#endif // __MX35_MLB_H

