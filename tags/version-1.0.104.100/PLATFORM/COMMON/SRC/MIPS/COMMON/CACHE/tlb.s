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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
#include <kxmips.h>
#include <nkintr.h>

#define KSEG0_BASE          0x80000000

//------------------------------------------------------------------------------

        .text
        .globl  OEMTLBSize

OEMTLBSize:
        .word   31
        
//------------------------------------------------------------------------------
//
//  Function:     OALClearTLB
//
//  Clear all TLB entries. Because MIPS has unified TLB and there isn't simple
//  way how to find TLB entry for address full TLB is cleared each time.
//
LEAF_ENTRY(OALClearTLB)

        .set noreorder
        .text
        
        mfc0    t0, psr                 // Save current psr
        mfc0    t1, entryhi             // Save current asid
        nop
        andi    t2, t0, 0xFFFE          // Mask interrupts
        mtc0    t2, psr                 // Disable interrupts
        nop                             // fill delay slot
        li      t2, KSEG0_BASE          // Unmapped address
        lw      t3, OEMTLBSize          // (t3) = index & loop counter
        mfc0    t4, wired               // (t4) = loop limit

10:     mtc0    zero, entrylo0          // Clear entrylo0 - Invalid entry
        mtc0    zero, entrylo1          // Clear entrylo1 - Invalid entry
        mtc0    t2, entryhi             // Clear entryhi - Ivalid address
        mtc0    t3, index               // Set index
        add     t2, 0x2000
        nop                             // Fill delay slot
        tlbwi                           // Write entry (indexed)
        nop
        bne     t3, t4, 10b             // If not zero do next one
        addu    t3,  -1                 // Decrement index, loop counter

        mtc0    t1, entryhi             // Restore asid
        mtc0    t0, psr                 // Restore psr

        j       ra
        nop
        
        .end OALClearTlb

//------------------------------------------------------------------------------

