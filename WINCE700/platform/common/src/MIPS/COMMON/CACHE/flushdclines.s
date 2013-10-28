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

#include <kxmips.h>
#define __MIPS_ASSEMBLER
#include <oal_cache.h>

//------------------------------------------------------------------------------
//
// Function:     OALFlushDCacheLines(VOID *pAddress, UINT32 size)
//
//  Write back and invalidate range of addresses in DCache.  This routine
//  uses cache operations based on virtual addresses, so TLB misses may
//  occur while this routine is running.
//

LEAF_ENTRY(OALFlushDCacheLines)
        .set noreorder

        // Get cache info address
        la      t7, g_oalCacheInfo

        // Get cache size
        lw      t0, L1DLineSize(t7)

        // Get the last address in range
        addu    t1, a0, a1

        // loop through the cache
10:     cache   HIT_WRITEBACK_INVALIDATE_D, 0(a0)
        addu    a0, t0
        bltu    a0, t1, 10b
        nop

        // return to the caller
        j       ra
        nop

        .end OALFlushDCacheLines

//------------------------------------------------------------------------------

