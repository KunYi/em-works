//-----------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  Header:  common_tzic.h
//
//  Provides definitions for the TZIC (TrustZone Interrupt Controller) 
//  module that are common to Freescale SoCs.
//
//-----------------------------------------------------------------------------
#ifndef __COMMON_TZIC_H
#define __COMMON_TZIC_H

#if __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------------------------
// GENERAL MODULE CONSTANTS
//-----------------------------------------------------------------------------
#define TZIC_IRQ_SOURCES_MAX    128
#define TZIC_IRQ_SET_MAX        (TZIC_IRQ_SOURCES_MAX / 32)


//-----------------------------------------------------------------------------
// REGISTER LAYOUT
//-----------------------------------------------------------------------------
typedef struct
{
    UINT32 INTCTRL;         // 0x000
    UINT32 INTTYPE;         // 0x004
    UINT32 reserved1;       // 0x008
    UINT32 PRIOMASK;        // 0x00C
    UINT32 SYNCCTRL;        // 0x010
    UINT32 DSMINT;          // 0x014
    UINT32 reserved2[26];   // 0x018-0x080
    UINT32 INTSEC[32];      // 0x080-0x100
    UINT32 ENSET[32];       // 0x100-0x180
    UINT32 ENCLEAR[32];     // 0x180-0x200
    UINT32 SRCSET[32];      // 0x200-0x280
    UINT32 SRCCLEAR[32];    // 0x280-0x300
    UINT32 reserved3[64];   // 0x300-0x400
    UINT32 PRIORITY[256];   // 0x400-0x800
    UINT32 reserved4[320];  // 0x800-0xD00
    UINT32 PND[32];         // 0xD00-0xD80
    UINT32 HIPND[32];       // 0xD80-0xE00
    UINT32 WAKEUP[32];      // 0xE00-0xE80
    UINT32 reserved5[32];   // 0xE80-0xF00
    UINT32 SWINT;           // 0xF00
} CSP_TZIC_REGS, *PCSP_TZIC_REGS;

//-----------------------------------------------------------------------------
// REGISTER OFFSETS
//-----------------------------------------------------------------------------
#define TZIC_INTCTRL_OFFSET             0x0000
#define TZIC_INTTYPE_OFFSET             0x0004
#define TZIC_PRIOMASK_OFFSET            0x000C
#define TZIC_SYNCCTRL_OFFSET            0x0010
#define TZIC_DSMINT_OFFSET              0x0014
#define TZIC_INTSEC_OFFSET              0x0080
#define TZIC_ENSET_OFFSET               0x0100
#define TZIC_ENCLEAR_OFFSET             0x0180
#define TZIC_SRCSET_OFFSET              0x0200
#define TZIC_SRCCLR_OFFSET              0x0280
#define TZIC_PRIORITY_OFFSET            0x0400
#define TZIC_PND_OFFSET                 0x0D00
#define TZIC_HIPND_OFFSET               0x0D80
#define TZIC_WAKEUP_OFFSET              0x0E00
#define TZIC_SWINT_OFFSET               0x0F00
 

//-----------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//-----------------------------------------------------------------------------
#define TZIC_INTCTRL_EN_LSH                 0
#define TZIC_INTCTRL_NSEN_LSH               16
#define TZIC_INTCTRL_NSENMASK_LSH           31

#define TZIC_INTTYPE_ITLINES_LSH            0
#define TZIC_INTTYPE_CPUS_LSH               5
#define TZIC_INTTYPE_DOM_LSH                10

#define TZIC_PRIOMASK_MASK_LSH              0

#define TZIC_SYNCCTRL_SYNCMODE_LSH          0

#define TZIC_DSMINT_DSM_LSH                 0

#define TZIC_SWINT_INTID_LSH                0
#define TZIC_SWINT_INTNEG_LSH               31


//-----------------------------------------------------------------------------
// REGISTER BIT FIELD WIDTHS
//-----------------------------------------------------------------------------
#define TZIC_INTCTRL_EN_WID                 1
#define TZIC_INTCTRL_NSEN_WID               1
#define TZIC_INTCTRL_NSENMASK_WID           1

#define TZIC_INTTYPE_ITLINES_WID            5
#define TZIC_INTTYPE_CPUS_WID               3
#define TZIC_INTTYPE_DOM_WID                1

#define TZIC_PRIOMASK_MASK_WID              6

#define TZIC_SYNCCTRL_SYNCMODE_WID          2

#define TZIC_DSMINT_DSM_WID                 1

#define TZIC_SWINT_INTID_WID                10
#define TZIC_SWINT_INTNEG_WID               1


//-----------------------------------------------------------------------------
// REGISTER BIT WRITE VALUES
//-----------------------------------------------------------------------------

// INTCTRL
#define TZIC_INTCTRL_EN_DISABLE             0
#define TZIC_INTCTRL_EN_ENABLE              1

#define TZIC_INTCTRL_NSEN_DISABLE           0
#define TZIC_INTCTRL_NSEN_ENABLE            1

#define TZIC_INTCTRL_NSENMASK_IGNORE        0
#define TZIC_INTCTRL_NSENMASK_UPDATE        1

// SYNCCTRL
#define TZIC_SYNCCTRL_SYNCMODE_LOWLATENCY   0
#define TZIC_SYNCCTRL_SYNCMODE_LOWPOWER     1

// DSMINT
#define TZIC_DSMINT_DSM_UPDATE              0
#define TZIC_DSMINT_DSM_HOLDOFF             1

// SWINT
#define TZIC_SWINT_INTNEG_ASSERTED          0
#define TZIC_SWINT_INTNEG_NEGATED           1


//-----------------------------------------------------------------------------
// HELPER MACROS
//-----------------------------------------------------------------------------
#define TZIC_IRQ_SET_INDEX(irq)             ((irq) >> 5)
#define TZIC_IRQ_SET_MASK(irq)              (1U << (irq & 0x1F))

#ifdef __cplusplus
}
#endif

#endif // __COMMON_TZIC_H
