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
#include <windows.h>
#include <pkfuncs.h>
#include <oal.h>

/*
OEMCacheRangeFlush()

  Single OAL entry point for all cache flush operations.
  
    Parameters:
    
      pAddr	 - starting VA on which the cache operation is to be performed
      dwLength - length of flush
      dwFlags  - specifies the cache operation to be performed:
      
        CACHE_SYNC_WRITEBACK: write back DATA cache
        CACHE_SYNC_DISCARD: write back and discard DATA cache
        CACHE_SYNC_INSTRUCTIONS: discard I-Cache
        CACHE_SYNC_FLUSH_I_TLB: flush instruction TLB
        CACHE_SYNC_FLUSH_D_TLB: flush data TLB
        CACHE_SYNC_FLUSH_TLB: flush both I/D TLB
        CACHE_SYNC_L2_WRITEBACK: write back L2 cache
        CACHE_SYNC_L2_DISCARD: write back and discard L2 cache
        CACHE_SYNC_ALL: perform all the above operations.
        
    Return Value
          
        None.
            
    Remarks
              
        If both pAddr and dwLength are 0, the entire cache (or TLB, as directed by dwFlags) will be flushed.
        Only the kernel can set the TLB flush flags when it calls this routine, and when a TLB flush is performed
        with dwLength == PAGE_SIZE, pAddr is guaranteed to be on a page boundary.

        This routine is not called by the kernel until after KernelRelocate() has been invoked, at which
        point global read/write data is available.  If an OEM needs to flush cache or TLB entries during
        early boot phases, they will need to implement a platform-specific API to do so.

        NOTE:  This routine could be optimized for a particular platform by using #define or const data
        to describe the size of the cache(s) or TLB.  This implementation allows dynamic cache sizing 
        in order to keep porting simple.
                
*/
void OEMCacheRangeFlush(LPVOID pAddr, DWORD dwLength, DWORD dwFlags)
{
    // these routines are defined in assembly language
    extern void SH4FlushICache(void);
	extern void SH4FlushDCache(DWORD);
	extern void SH4ClearTLB(void);

    // global variable defined by the platform
	extern const DWORD SH4CacheLines;

	if ((dwFlags & (CACHE_SYNC_DISCARD)) || (dwFlags & (CACHE_SYNC_WRITEBACK))) 
    {
        SH4FlushDCache(SH4CacheLines);
	} 

	if (dwFlags & (CACHE_SYNC_INSTRUCTIONS)) {
        // all we can do is flush the entire cache, it is too expensive
        // to search for individual entries.
		SH4FlushICache();
	}

    if (dwFlags & CACHE_SYNC_FLUSH_TLB) {
        // flushing individual TLB entries is too expensive, so flush the whole
        // thing.
		SH4ClearTLB ();
    }
}


//------------------------------------------------------------------------------
//
//  Function:  OALCacheGlobalsInit
//
//  This function initializes globals variables which holds cache parameters.
//  It must be called before any other cache/TLB function.
//
VOID OALCacheGlobalsInit()
{
    g_oalCacheInfo.L1Flags          = 0;
    g_oalCacheInfo.L1ISetsPerWay    = 256;
    g_oalCacheInfo.L1INumWays       = 2;
    g_oalCacheInfo.L1ILineSize      = 32;
    g_oalCacheInfo.L1ISize          = (16 * 1024);
    g_oalCacheInfo.L1DSetsPerWay    = 512;
    g_oalCacheInfo.L1DNumWays       = 2;
    g_oalCacheInfo.L1DLineSize      = 32;
    g_oalCacheInfo.L1DSize          = (32 * 1024);

    // No L2 Cache
    g_oalCacheInfo.L2Flags          = 0;
    g_oalCacheInfo.L2ISetsPerWay    = 0;
    g_oalCacheInfo.L2INumWays       = 0;
    g_oalCacheInfo.L2ILineSize      = 0;
    g_oalCacheInfo.L2ISize          = 0;
    g_oalCacheInfo.L2DSetsPerWay    = 0;
    g_oalCacheInfo.L2DNumWays       = 0;
    g_oalCacheInfo.L2DLineSize      = 0;
    g_oalCacheInfo.L2DSize          = 0;
}
