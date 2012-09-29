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
// Copyright (C) 2006-2010, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//------------------------------------------------------------------------------
//
//  File:  flush.c
//
//  This file implements OEMCacheRangeFlush function suitable for ARM CPU
//  with separate data and instruction caches.
//
#pragma warning(push)
#pragma warning(disable: 4115 4214 4201)
#include <windows.h>
#include <ceddk.h>
#include <oal.h>
#pragma warning(pop)

//------------------------------------------------------------------------------
// External Functions
extern VOID OALCleanL2CacheLines(VOID* pAddress, UINT32 size);
extern VOID OALCleanL2Cache();
extern VOID OALFlushL2Cache();

//------------------------------------------------------------------------------
// Global Variables

// KB975133 eliminates the need of forced L2 cache maintenance after L1 
// maintenance.  By default, the OAL now assumes the kernel changes implemented 
// with KB957133 are available and will strictly comply with the cache operation 
// indicated by flags passed to OEMCacheRangeFlush.  If the kernel updates 
// associated with KB975133 have not been integrated, L2 cache maintenance 
// can be forced after L1 cache maintenance requests by setting 
// g_bOalL2FlagsForced to TRUE.  This can be done by referencing this global
// as an extern and setting it TRUE in platform-specific OEMInit code.
BOOL g_bOalL2FlagsForced = FALSE;

//------------------------------------------------------------------------------
//
//  Function:  OEMCacheRangeFlush
//
//  Implements entry point for all cache and TLB flush operations. It takes
//  following parameters:
//
//      pAddress - starting VA on which the cache operation is to be performed
//      length - length of flush
//      flags  - specifies the cache operation to be performed:
//
//          CACHE_SYNC_WRITEBACK: write back DATA cache
//          CACHE_SYNC_DISCARD: write back and discard DATA cache
//          CACHE_SYNC_INSTRUCTIONS: discard I-Cache
//          CACHE_SYNC_FLUSH_I_TLB: flush instruction TLB
//          CACHE_SYNC_FLUSH_D_TLB: flush data TLB
//          CACHE_SYNC_FLUSH_TLB: flush both I/D TLB
//          CACHE_SYNC_L2_WRITEBACK: write back L2 cache
//          CACHE_SYNC_L2_DISCARD: write back and discard L2 cache
//          CACHE_SYNC_ALL: perform all the above operations.
//
//  If both pAddress and length are 0, the entire cache (or TLB, as directed by
//  flags) will be flushed. Only the kernel can set the TLB flush flags when
//  it calls this routine, and when a TLB flush is performed with 
//  length == PAGE_SIZE, pAddress is guaranteed to be on a page boundary.
//

void OEMCacheRangeFlush(VOID *pAddress, DWORD length, DWORD flags)
{
    DWORD range;

    OALMSG(OAL_CACHE&&OAL_VERBOSE, (
        L"+OEMCacheRangeFlush(0x%08x, %d, 0x%08x)\r\n", pAddress, length, flags
    ));

    // Clear L2 flags when L2 is disabled
    if (g_oalCacheInfo.L2DSize == 0)
    {
        flags &= (~(CACHE_SYNC_L2_DISCARD | CACHE_SYNC_L2_WRITEBACK));
    }
    // Check for forced L2 maintenance
    else if (g_bOalL2FlagsForced)
    {    
        if ((flags & CACHE_SYNC_DISCARD) != 0) flags |= CACHE_SYNC_L2_DISCARD;
        else if ((flags & CACHE_SYNC_WRITEBACK) != 0) flags |= CACHE_SYNC_L2_WRITEBACK;
    }

    if ((flags & CACHE_SYNC_DISCARD) != 0) {
        // Write back and invalidate the selected portions of the data cache
        if (length == 0) {
            if (pAddress == NULL) OALFlushDCache();
        } else {
            // Normalize address to cache line alignment
            UINT32 mask = g_oalCacheInfo.L1DLineSize - 1;
            UINT32 address = (UINT32)pAddress & ~mask;
            // Adjust size to reflect cache line alignment
            range = (UINT32)pAddress - address + length;
            // If range is bigger than cache size flush all
            if (range >= g_oalCacheInfo.L1DSize) {
                OALFlushDCache();
            } else {
                // Flush all the indicated cache entries
                OALFlushDCacheLines((VOID*)address, range);

                // Cortex-A8 clean and invalidate by MVA occurs to PoC (L3). 
                // Clear CACHE_SYNC_L2_DISCARD flag since indicated lines
                // will be flushed to L3 during OALFlushDCacheLines.
                flags &= (~(CACHE_SYNC_L2_DISCARD));
            }
        }
    } else if ((flags & CACHE_SYNC_WRITEBACK) != 0) {
        // Write back the selected portion of the data cache
        if (length == 0) {
            if (pAddress == NULL) OALCleanDCache();
        } else {
            // Normalize address to cache line alignment
            UINT32 mask = g_oalCacheInfo.L1DLineSize - 1;
            UINT32 address = (UINT32)pAddress & ~mask;
            // Adjust size to reflect cache line alignment
            range = (UINT32)pAddress - address + length;
            // If range is bigger than cache size clean all
            if (range >= g_oalCacheInfo.L1DSize) {
                OALCleanDCache();
            } else {
                // Cortex-A8 clean by MVA can occur to PoU (L2) or PoC (L3).
                // If CACHE_SYNC_L2_WRITEBACK is set, skip the clean to PoU 
                // used by OALCleanDCacheLines since clean to PoC will be
                // used later by OALCleanL2CacheLines.
                if (!(flags & CACHE_SYNC_L2_WRITEBACK))
                {
                    // Flush all the indicated cache entries
                    OALCleanDCacheLines((VOID*)address, range);
                }
            }
        }
    }
    if ((flags & CACHE_SYNC_INSTRUCTIONS) != 0) {
        if (length == 0) {
            if (pAddress == NULL) OALFlushICache();
        } else {
            // Normalize address to cache line alignment
            UINT32 mask = g_oalCacheInfo.L1ILineSize - 1;
            UINT32 address = (UINT32)pAddress & ~mask;
            range = (UINT32)pAddress - address + length;
            if (range >= g_oalCacheInfo.L1ISize) {
                OALFlushICache();
            } else {
                OALFlushICacheLines((VOID*)address, range);
            }
        }
    }

    if ((flags & CACHE_SYNC_L2_DISCARD) != 0) {
        // Write back and invalidate the selected portions of the data cache
        if (length == 0) {
            if (pAddress == NULL) OALFlushL2Cache();
        } else {
            // Normalize address to cache line alignment
            UINT32 mask = g_oalCacheInfo.L2DLineSize - 1;
            UINT32 address = (UINT32)pAddress & ~mask;
            // Adjust size to reflect cache line alignment
            range = (UINT32)pAddress - address + length;
            // If range is bigger than cache size flush all
            if (range >= g_oalCacheInfo.L2DSize) {
                OALFlushL2Cache();
            } else {
                // Flush all the indicated cache entries
                OALFlushDCacheLines((VOID*)address, range);
            }
        }
    } else if ((flags & CACHE_SYNC_L2_WRITEBACK) != 0) {
        // Write back the selected portion of the data cache
        if (length == 0) {
            if (pAddress == NULL) OALCleanL2Cache();
        } else {
            // Normalize address to cache line alignment
            UINT32 mask = g_oalCacheInfo.L2DLineSize - 1;
            UINT32 address = (UINT32)pAddress & ~mask;
            // Adjust size to reflect cache line alignment
            range = (UINT32)pAddress - address + length;
            // If range is bigger than cache size clean all
            if (range >= g_oalCacheInfo.L2DSize) {
                OALCleanL2Cache();
            } else {
                // Write back all the indicated cache entries
                OALCleanL2CacheLines((VOID*)address, range);
            }
        }
    }

    if ((flags & CACHE_SYNC_FLUSH_I_TLB) != 0) {
        if (length == (DWORD)PAGE_SIZE) {
            // flush one TLB entry
            OALClearITLBEntry(pAddress);
        } else {
            // flush the whole TLB
            OALClearITLB();
        }
    }

    if ((flags & CACHE_SYNC_FLUSH_D_TLB) != 0) {
        // check first for TLB updates forced by paging
        if (length == (DWORD)PAGE_SIZE) {
            // flush one TLB entry
            OALClearDTLBEntry(pAddress);
        } else {
            // flush the whole TLB
            OALClearDTLB();
        }
    }

    OALMSG(OAL_CACHE&&OAL_VERBOSE, (L"-OEMCacheRangeFlush\r\n"));
}

//------------------------------------------------------------------------------
