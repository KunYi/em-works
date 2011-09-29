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
#define __MIPS_ASSEMBLER
#include <oal_cache.h>

#define KSEG0_BASE      0x80000000

//------------------------------------------------------------------------------
//
// Function:     OALFlushICache()
//
// This function flush and invalidate all instruction cache
//

LEAF_ENTRY(OALFlushICache)

        .set noreorder

        // Get cache info address
        la      t7, g_oalCacheInfo

        // Get cache size
        lw      t0, L1ISize(t7)
//        srl	t0, 1
        
        // Get cache line size
        lw      t1, L1ILineSize(t7)

        // Get the VA with which to index the cache -- since
        // it will go through the MMU, use a KSEG0 address
        // to avoid causing TLB faults
        li      t2, KSEG0_BASE

        // Get the last address in this range to process
        addu    t3, t2, t0
        subu    t3, t1

        // loop through the cache
10:     cache   INDEX_INVALIDATE_I, 0(t2)
        cache   INDEX_INVALIDATE_I, 1(t2)
        bne     t3, t2, 10b
        addu    t2, t1                  // increment in delay slot

        // return to the caller
        j       ra
        nop

        .end OALFlushICache

//------------------------------------------------------------------------------
