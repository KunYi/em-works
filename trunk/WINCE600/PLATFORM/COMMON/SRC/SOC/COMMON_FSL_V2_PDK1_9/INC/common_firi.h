//-----------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//-----------------------------------------------------------------------------
//
//  Copyright (C) 2004-2007,2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//  Header: common_firi.h
//
//  Provides definitions for FIRI module.
//
//------------------------------------------------------------------------------

#ifndef __COMMON_FIRI_H
#define __COMMON_FIRI_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------
typedef struct{
    UINT32 FIRI_TCR;                // 00: FIRI Tx Control Register
    UINT32 FIRI_TCTR;             // 04: FIRI Tx Count Register
    UINT32 FIRI_RCR;                // 08: FIRI Rx Control Register
    UINT32 FIRI_TSR;                // 0C: FIRI Tx Status Register
    UINT32 FIRI_RSR;                // 10: FIRI Rx Status Register
    UINT32 FIRI_TXFIFO;         // 14: FIRI Tx FIFO Register
    UINT32 FIRI_RXFIFO;         // 18: FIRI Rx FIFO Register
    UINT32 FIRI_CR;               // 1C: FIRI Control Register
} CSP_FIRI_REG, *PCSP_FIRI_REG;


//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
#define FIRI_TCR_OFFSET        0x0000
#define FIRI_TCTR_OFFSET       0x0004
#define FIRI_RCR_OFFSET        0x0008
#define FIRI_TSR_OFFSET        0x000C
#define FIRI_RSR_OFFSET        0x0010
#define FIRI_TXFIFO_OFFSET     0x0014
#define FIRI_RXFIFO_OFFSET     0x0018
#define FIRI_CR_OFFSET         0x001C

//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------
#define FIRI_TCR_TE_LSH        0
#define FIRI_TCR_TM_LSH        1
#define FIRI_TCR_TPP_LSH       3
#define FIRI_TCR_SIP_LSH       4
#define FIRI_TCR_PC_LSH        5
#define FIRI_TCR_PCF_LSH       6
#define FIRI_TCR_TFUIE_LSH     7
#define FIRI_TCR_TPEIE_LSH     8
#define FIRI_TCR_TCIE_LSH      9
#define FIRI_TCR_TDT_LSH      10
#define FIRI_TCR_SRF_LSH      13
#define FIRI_TCR_TPA_LSH      16
#define FIRI_TCR_HAG_LSH      24

#define FIRI_TCTR_TPL_LSH      0

#define FIRI_RCR_RE_LSH        0
#define FIRI_RCR_RM_LSH        1
#define FIRI_RCR_RPP_LSH       3
#define FIRI_RCR_RFOIE_LSH     4
#define FIRI_RCR_PAIE_LSH      5
#define FIRI_RCR_RPEIE_LSH     6
#define FIRI_RCR_RPA_LSH       7
#define FIRI_RCR_RDT_LSH       8
#define FIRI_RCR_RPEDE_LSH    11
#define FIRI_RCR_RA_LSH       16
#define FIRI_RCR_RAM_LSH      24

#define FIRI_TSR_TFU_LSH       0
#define FIRI_TSR_TPE_LSH       1
#define FIRI_TSR_SIPE_LSH      2
#define FIRI_TSR_TC_LSH        3
#define FIRI_TSR_TFP_LSH       8

#define FIRI_RSR_DDE_LSH       0
#define FIRI_RSR_CRCE_LSH      1
#define FIRI_RSR_BAM_LSH       2
#define FIRI_RSR_RFO_LSH       3
#define FIRI_RSR_RPE_LSH       4
#define FIRI_RSR_PAS_LSH       5
#define FIRI_RSR_RFP_LSH       8

#define FIRI_CR_OSF_LSH        0
#define FIRI_CR_BL_LSH         5

//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------
#define FIRI_TCR_TE_WID        1
#define FIRI_TCR_TM_WID        2
#define FIRI_TCR_TPP_WID       1
#define FIRI_TCR_SIP_WID       1
#define FIRI_TCR_PC_WID        1
#define FIRI_TCR_PCF_WID       1
#define FIRI_TCR_TFUIE_WID     1
#define FIRI_TCR_TPEIE_WID     1
#define FIRI_TCR_TCIE_WID      1
#define FIRI_TCR_TDT_WID       3
#define FIRI_TCR_SRF_WID       2
#define FIRI_TCR_TPA_WID       8
#define FIRI_TCR_HAG_WID       1

#define FIRI_TCTR_TPL_WID     11

#define FIRI_RCR_RE_WID        1
#define FIRI_RCR_RM_WID        2
#define FIRI_RCR_RPP_WID       1
#define FIRI_RCR_RFOIE_WID     1
#define FIRI_RCR_PAIE_WID      1
#define FIRI_RCR_RPEIE_WID     1
#define FIRI_RCR_RPA_WID       1
#define FIRI_RCR_RDT_WID       3
#define FIRI_RCR_RPEDE_WID     1
#define FIRI_RCR_RA_WID        8
#define FIRI_RCR_RAM_WID       2

#define FIRI_TSR_TFU_WID       1
#define FIRI_TSR_TPE_WID       1
#define FIRI_TSR_SIPE_WID      1
#define FIRI_TSR_TC_WID        1
#define FIRI_TSR_TFP_WID       8

#define FIRI_RSR_DDE_WID       1
#define FIRI_RSR_CRCE_WID      1
#define FIRI_RSR_BAM_WID       1
#define FIRI_RSR_RFO_WID       1
#define FIRI_RSR_RPE_WID       1
#define FIRI_RSR_PAS_WID       1
#define FIRI_RSR_RFP_WID       8

#define FIRI_CR_OSF_WID        4
#define FIRI_CR_BL_WID         7

//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------
#define FIRI_TCR_TE_DISABLE         0
#define FIRI_TCR_TE_ENABLE          1

#define FIRI_TCR_TM_FIR             0
#define FIRI_TCR_TM_576_MIR         1
#define FIRI_TCR_TM_1152_MIR        2
#define FIRI_TCR_TM_SOFT_ASSEMBLE   3

#define FIRI_TCR_TPP_NO_INVERT      0
#define FIRI_TCR_TPP_INVERT         1

#define FIRI_TCR_SIP_ENABLE         1

#define FIRI_TCR_PC_CRC_STO         0
#define FIRI_TCR_PC_ABORT           1

#define FIRI_TCR_PCF_CRC_STO        0
#define FIRI_TCR_PCF_ABORT          1

#define FIRI_TCR_TFUIE_DISABLE      0
#define FIRI_TCR_TFUIE_ENABLE       1

#define FIRI_TCR_TPEIE_DISABLE      0
#define FIRI_TCR_TPEIE_ENABLE       1

#define FIRI_TCR_TCIE_DISABLE       0
#define FIRI_TCR_TCIE_ENABLE        1

typedef enum {
    FIRI_TCR_TDT_0,
    FIRI_TCR_TDT_16,
    FIRI_TCR_TDT_32,
    FIRI_TCR_TDT_48,
    FIRI_TCR_TDT_64,
    FIRI_TCR_TDT_80,
    FIRI_TCR_TDT_96,
    FIRI_TCR_TDT_112
} FIRI_TCR_TDT_VAL;

typedef enum {
    FIRI_TCR_SRF_16PA_2STA,
    FIRI_TCR_SRF_32PA_4STA,
    FIRI_TCR_SRF_64PA_8STA,
    FIRI_TCR_SRF_128PA_16STA
} FIRI_TCR_SRF;

#define FIRI_TCR_HAG_READ_ADDR      0
#define FIRI_TCR_HAG_TPA_ADDR       1

#define FIRI_RCR_RE_DISABLE         0
#define FIRI_RCR_RE_ENABLE          1

#define FIRI_RCR_RM_FIR             0
#define FIRI_RCR_RM_576_MIR         1
#define FIRI_RCR_RM_1152_MIR        2
#define FIRI_RCR_RM_SOFT_ASSEMBLE   3

#define FIRI_RCR_RPP_NO_INVERT      0
#define FIRI_RCR_RPP_INVERT         1

#define FIRI_RCR_RFOIE_DISABLE      0
#define FIRI_RCR_RFOIE_ENABLE       1

#define FIRI_RCR_PAIE_DISABLE       0
#define FIRI_RCR_PAIE_ENABLE        1

#define FIRI_RCR_RPEIE_DISABLE      0
#define FIRI_RCR_RPEIE_ENABLE       1

#define FIRI_RCR_RPA_NO_CLEAR       0
#define FIRI_RCR_RPA_CLEAR          1

typedef enum {
    FIRI_RCR_RDT_0,
    FIRI_RCR_RDT_16,
    FIRI_RCR_RDT_32,
    FIRI_RCR_RDT_48,
    FIRI_RCR_RDT_64,
    FIRI_RCR_RDT_80,
    FIRI_RCR_RDT_96,
    FIRI_RCR_RDT_112
} FIRI_RCR_RDT_VAL;

#define FIRI_RCR_RPEDE_DISABLE       0
#define FIRI_RCR_RPEDE_ENABLE        1

#define FIRI_RCR_RAM_NO_MATCH        0
#define FIRI_RCR_RAM_RA              1
#define FIRI_RCR_RAM_BROADCAST       2
#define FIRI_RCR_RAM_RA_BROADCAST    3

#define FIRI_TSR_TFU_NO_UNDERRUN     0
#define FIRI_TSR_TFU_UNDERRUN        1

#define FIRI_TSR_TPE_IN_PROGRESS     0
#define FIRI_TSR_TPE_COMPLETE        1

#define FIRI_TSR_SIPE_IN_PROGRESS    0
#define FIRI_TSR_SIPE_COMPLETE       1

#define FIRI_TSR_TC_IN_PROGRESS      0
#define FIRI_TSR_TC_COMPLETE         1

#define FIRI_RSR_DDE_NO_ILLEGAL      0
#define FIRI_RSR_DDE_ILLEGAL         1

#define FIRI_RSR_CRCE_NO_FAILURE     0
#define FIRI_RSR_CRCE_FAILURE        1

#define FIRI_RSR_BAM_NO_BROADCAST    0
#define FIRI_RSR_BAM_BROADCAST       1

#define FIRI_RSR_RFO_NO_OVERRUN      0
#define FIRI_RSR_RFO_OVERRUN         1

#define FIRI_RSR_RPE_NO_DETECT       0
#define FIRI_RSR_RPE_DETECT          1

#define FIRI_RSR_PAS_NO_SEARCH       0
#define FIRI_RSR_PAS_SEARCH          1

#ifdef __cplusplus
}
#endif

#endif // __COMMON_FIRI_H
