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
// Header: mx25_rngb.h
//
// Provides definitions for RNGB module.
//
//------------------------------------------------------------------------------
#ifndef __MX25_RNGB_H
#define __MX25_RNGB_H

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------
typedef struct 
{
    UINT32 RNG_ID;
    UINT32 RNG_COMMAND;
    UINT32 RNG_CR;
    UINT32 RNG_SR;
    UINT32 RNG_ERROR_SR;
    UINT32 RNG_OFIFO;
    UINT32 RNG_ENTROPY;
    UINT32 RNG_RESERVED1;
    // The following registers are not available when the RGNC is in secure mode.
    UINT32 RNG_VER;
    UINT32 RNG_VKEY;
    UINT32 RNG_OSCCR;
    UINT32 RNG_OSCCT;
    UINT32 RNG_OSCSTAT;
} CSP_RNGB_REGS, *PCSP_RNGB_REGS;


//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
#define RNG_ID_OFFSET               0x0000
#define RNG_COMMAND_OFFSET          0x0004
#define RNG_CR_OFFSET               0x0008
#define RNG_SR_OFFSET               0x000C
#define RNG_ERROR_SR_OFFSET         0x0010
#define RNG_OFIFO_OFFSET            0x0014
#define RNG_ENTROPY_OFFSET          0x0018
#define RNG_VER_OFFSET              0x0020
#define RNG_VKEY_OFFSET             0x0024
#define RNG_OSCCR_OFFSET            0x0028
#define RNG_OSCCT_OFFSET            0x002C
#define RNG_OSCSTAT_OFFSET          0x0030

//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
#define RNG_ID_MINORVER_LSH         0
#define RNG_ID_MAJORVER_LSH         8
#define RNG_ID_CHIPVER_LSH          16
#define RNG_ID_TYPE_LSH             28

#define RNG_COMMAND_SELFTEST_LSH    0
#define RNG_COMMAND_SEED_LSH        1
#define RNG_COMMAND_CLRINT_LSH      4
#define RNG_COMMAND_CLRERR_LSH      5
#define RNG_COMMAND_SWRST_LSH       6

#define RNG_CR_FIFORES_LSH          0
#define RNG_CR_AUTOSEED_LSH         4
#define RNG_CR_MASKDONE_LSH         5
#define RNG_CR_MASKERR_LSH          6
#define RNG_CR_VERIFMODE_LSH        8
#define RNG_CR_CTLAACC_LSH          9

#define RNG_SR_SECSTAT_LSH          0
#define RNG_SR_BUSY_LSH             1
#define RNG_SR_SLEEP_LSH            2
#define RNG_SR_RESEED_LSH           3
#define RNG_SR_STDONE_LSH           4
#define RNG_SR_SEEDDONE_LSH         5
#define RNG_SR_NEWSEEDDONE_LSH      6
#define RNG_SR_FIFOLVL_LSH          8
#define RNG_SR_FIFOSIZE_LSH         12
#define RNG_SR_ERROR_LSH            16
#define RNG_SR_ST_PF_LSH            22
#define RNG_SR_ST_TEST_PF_LSH       31

#define RNG_ERROR_SR_LFSR_LSH       0
#define RNG_ERROR_SR_OSC_LSH        1
#define RNG_ERROR_SR_ST_LSH         2
#define RNG_ERROR_SR_STAT_LSH       3
#define RNG_ERROR_SR_FIFO_LSH       4

#define RNG_VER_SHFTCLKOFF_LSH      0
#define RNG_VER_FORCSYSCLK_LSH      1
#define RNG_VER_OSCTEST_LSH         2
#define RNG_VER_FAKESEED_LSH        3
#define RNG_VER_TESTKEY_LSH         4
#define RNG_VER_RSTSHFTREG_LSH      8
#define RNG_VER_KEYACC_LSH          24

#define RNG_OSCCR_CYCLES_LSH        0

#define RNG_OSCCT_CLKPULSE_LSH      0

#define RNG_OSCSTAT_OSC_LSH         0

//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
#define RNG_ID_MINORVER_WID         8
#define RNG_ID_MAJORVER_WID         8
#define RNG_ID_CHIPVER_WID          8
#define RNG_ID_TYPE_WID             4

#define RNG_COMMAND_SELFTEST_WID    1
#define RNG_COMMAND_SEED_WID        1
#define RNG_COMMAND_CLRINT_WID      1
#define RNG_COMMAND_CLRERR_WID      1
#define RNG_COMMAND_SWRST_WID       1

#define RNG_CR_FIFORES_WID          2
#define RNG_CR_AUTOSEED_WID         1
#define RNG_CR_MASKDONE_WID         1
#define RNG_CR_MASKERR_WID          1
#define RNG_CR_VERIFMODE_WID        1
#define RNG_CR_CTLAACC_WID          1

#define RNG_SR_SECSTAT_WID          1
#define RNG_SR_BUSY_WID             1
#define RNG_SR_SLEEP_WID            1
#define RNG_SR_RESEED_WID           1
#define RNG_SR_STDONE_WID           1
#define RNG_SR_SEEDDONE_WID         1
#define RNG_SR_NEWSEEDDONE_WID      1
#define RNG_SR_FIFOLVL_WID          4
#define RNG_SR_FIFOSIZE_WID         4
#define RNG_SR_ERROR_WID            1
#define RNG_SR_ST_PF_WID            2
#define RNG_SR_ST_TEST_PF_WID       8

#define RNG_ERROR_SR_LFSR_WID       1
#define RNG_ERROR_SR_OSC_WID        1
#define RNG_ERROR_SR_ST_WID         1
#define RNG_ERROR_SR_STAT_WID       1
#define RNG_ERROR_SR_FIFO_WID       1

#define RNG_VER_SHFTCLKOFF_WID      1
#define RNG_VER_FORCSYSCLK_WID      1
#define RNG_VER_OSCTEST_WID         1
#define RNG_VER_FAKESEED_WID        1
#define RNG_VER_TESTKEY_WID         1
#define RNG_VER_RSTSHFTREG_WID      1
#define RNG_VER_KEYACC_WID          8

#define RNG_OSCCR_CYCLES_WID        18

#define RNG_OSCCT_CLKPULSE_WID      20

#define RNG_OSCSTAT_OSC_WID         1

//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------
#define RNG_COMMAND_SELFTEST_SET    1

#define RNG_COMMAND_SEED_SET        1

#define RNG_COMMAND_CLRINT_SET      1

#define RNG_COMMAND_CLRERR_SET      1

#define RNG_COMMAND_SWRST_SET       1

#define RNG_CR_FIFORES_SET          1
#define RNG_CR_FIFORES_CLEAR        0

#define RNG_CR_AUTOSEED_SET         1
#define RNG_CR_AUTOSEED_CLEAR       0

#define RNG_CR_MASKDONE_SET         1
#define RNG_CR_MASKDONE_CLEAR       0

#define RNG_CR_MASKERR_SET          1
#define RNG_CR_MASKERR_CLEAR        0

#define RNG_CR_VERIFMODE_SET        1
#define RNG_CR_VERIFMODE_CLEAR      0

#define RNG_CR_CTLAACC_SET          1
#define RNG_CR_CTLAACC_CLEAR        0

#define RNG_VER_RSTSHFTREG_SET      1
 
#define RNG_VER_FORCSYSCLK_SET      1
#define RNG_VER_FORCSYSCLK_CLEAR    0

#define RNG_VER_SHFTCLKOFF_SET      1
#define RNG_VER_SHFTCLKOFF_CLEAR    0

//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------
#define RNGA                       0x0
#define RNGB                       0x1
#define RNGC                       0x2

//------------------------------------------------------------------------------
// Functions
//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // __MX25_RNGB_H
