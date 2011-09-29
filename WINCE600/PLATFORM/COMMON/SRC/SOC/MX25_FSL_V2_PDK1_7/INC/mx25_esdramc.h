//------------------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
//  Use of this source code is subject to the terms of the Microsoft end-user
//  license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
//  If you did not accept the terms of the EULA, you are not authorized to use
//  this source code. For a copy of the EULA, please see the LICENSE.RTF on your
//  install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
// Header: mx25_esdramc.h
//
// Provides definitions for ESDRAMC module on MX25.
//
//------------------------------------------------------------------------------
#ifndef __MX25_ESDRAMC_H__
#define __MX25_ESDRAMC_H__

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
    REG32 ESDCTL0;
    REG32 ESDCFG0;
    REG32 ESDCTL1;
    REG32 ESDCFG1;
    REG32 ESDMISC;
    REG32 ESDCDLY1;
    REG32 ESDCDLY2;
    REG32 ESDCDLY3;
    REG32 ESDCDLY4;
    REG32 ESDCDLY5;
    REG32 ESDCDLYL;
} CSP_ESDRAMC_REGS, *PCSP_ESDRAMC_REGS;

//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
#define ESDRAMC_ESDCTL0_OFFSET                  0x0000
#define ESDRAMC_ESDCFG0_OFFSET                  0x0004
#define ESDRAMC_ESDCTL1_OFFSET                  0x0008
#define ESDRAMC_ESDCFG1_OFFSET                  0x000C
#define ESDRAMC_ESDMISC_OFFSET                  0x0010
#define ESDRAMC_ESDCDLY1_OFFSET                 0x0020
#define ESDRAMC_ESDCDLY2_OFFSET                 0x0024
#define ESDRAMC_ESDCDLY3_OFFSET                 0x0028
#define ESDRAMC_ESDCDLY4_OFFSET                 0x002C
#define ESDRAMC_ESDCDLY5_OFFSET                 0x0030
#define ESDRAMC_ESDCDLYL_OFFSET                 0x0034

//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // __MX25_ESDRAMC_H__

