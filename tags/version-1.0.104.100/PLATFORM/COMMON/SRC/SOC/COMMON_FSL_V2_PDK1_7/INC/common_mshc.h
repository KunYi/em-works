//------------------------------------------------------------------------------
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//  Header: common_mshc.h
//
//  Provides definitions for the Freescale MSHC module.
//
//------------------------------------------------------------------------------

#ifndef __COMMON_MSHC_H
#define __COMMON_MSHC_H

#if __cplusplus
extern "C" {
#endif

#include "common_types.h"

//------------------------------------------------------------------------------
// REGISTER LAYOUT
//------------------------------------------------------------------------------

typedef struct {

    REG32 CMDR;                   // 0x00 Command register
    REG32 reserved0;              // 0x04 Reserved
    REG32 DATR;                   // 0x08 Data register
    REG32 reserved1;              // 0x0C Reserved
    REG32 STAR;                   // 0x10 Status register
    REG32 reserved2;              // 0x14 Reserved
    REG32 SYSR;                   // 0x18 System register
    REG32 reserved3;              // 0x1C Reserved
    REG32 DSAR;                   // 0x20 Embeded DMA start address register
     
    } CSP_MSHC_REG, *PCSP_MSHC_REG;

//------------------------------------------------------------------------------
// REGISTER OFFSETS
//------------------------------------------------------------------------------
#define MSHC_CMDR_OFFSET        0x00
#define MSHC_DATR_OFFSET        0x08
#define MSHC_STAR_OFFSET        0x10
#define MSHC_SYSR_OFFSET        0x18
#define MSHC_DSAR_OFFSET        0x20

#ifdef __cplusplus
}
#endif

#endif      // #ifndef __COMMON_MSHC_H


