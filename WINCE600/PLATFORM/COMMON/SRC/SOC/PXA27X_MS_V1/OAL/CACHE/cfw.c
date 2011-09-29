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
#include <nkintr.h>
#include <pkfuncs.h>


// Cache Information.
//
const CacheInfo ARMCacheInfo =
{
    0,               // flags for L1 cache
    (32 * 1024),     // L1 IC total size in bytes
    32,              // L1 IC line size in bytes
    32,              // L1 IC number of ways, 1 for direct mapped
    (32 * 1024),     // L1 DC total size in bytes
    32,              // L1 DC line size in bytes
    32,              // L1 DC number of ways, 1 for direct mapped
    0,               // flags for L2 cache
    0,               // L2 IC total size in bytes, 0 means no L2 ICache
    0,               // L2 IC line size in bytes
    0,               // L2 IC number of ways, 1 for direct mapped
    0,               // L2 DC total size in bytes, 0 means no L2 DCache
    0,               // L2 DC line size in bytes
    0                // L2 DC number of ways, 1 for direct mapped
};
const DWORD DCACHE_LINES = 1024;    // Number of DC lines (DC length / DC line length).

/*
OEMCacheRangeFlush()

  Single OAL entry point for all cache flush operations.
  
    Parameters:
    
      pAddr  - starting VA on which the cache operation is to be performed
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
                
*/

void OEMCacheRangeFlush(LPVOID pAddr, DWORD dwLength, DWORD dwFlags)
{

    // cache maintenance constants
    const DWORD PAGE_SIZE = 4096;       // 4KB pages used by winCE on XScale

    // these are defined in assembly language
    extern void OALFlushICache(void);
    extern void XScaleFlushICacheLines(LPVOID, DWORD, DWORD);
    extern void XScaleFlushDCache(DWORD, DWORD, DWORD);
    extern void XScaleFlushDCacheLines(PVOID, DWORD, DWORD);
    extern void XScaleCleanDCacheLines(LPVOID, DWORD, DWORD);
    extern void OALClearITLB(void);
    extern void OALClearITLBEntry(LPVOID);
    extern void OALClearDTLB(void);
    extern void OALClearDTLBEntry(LPVOID);
    extern void OALClearUTLB(void);
    extern const PVOID gpvCacheFlushBaseMemoryAddress;


    ///////////////////////////////////////////////////////////////////////////
    //  Flush DCACHE 
    ///////////////////////////////////////////////////////////////////////////
    
    if(dwFlags & CACHE_SYNC_DISCARD)
    {
        // Write-back and invalidate either all or a range
        //
        if (!(dwLength || (DWORD)pAddr))    // If length and address are both zero, flush all.
        {
            XScaleFlushDCache(DCACHE_LINES, ARMCacheInfo.dwL1DCacheLineSize, (DWORD) gpvCacheFlushBaseMemoryAddress);
        }
        else if (dwLength >= (DWORD) ARMCacheInfo.dwL1DCacheSize)
        {
            XScaleFlushDCache(DCACHE_LINES, ARMCacheInfo.dwL1DCacheLineSize, (DWORD) gpvCacheFlushBaseMemoryAddress);
        }
        else
        {
            // normalize address and length
            DWORD dwNormalizedAddress, dwNormalizedLength;
            dwNormalizedAddress = (DWORD) pAddr & ~(ARMCacheInfo.dwL1DCacheLineSize - 1);
            dwNormalizedLength = dwLength + ((DWORD) pAddr - dwNormalizedAddress);

            // invalidate all the indicated cache entries
            XScaleFlushDCacheLines((LPVOID) dwNormalizedAddress, dwNormalizedLength, ARMCacheInfo.dwL1DCacheLineSize);
        }
    }
    else if(dwFlags & CACHE_SYNC_WRITEBACK)
    {
        // write back the address range
        //
        if (!(dwLength || (DWORD)pAddr))    // If length and address are both zero, flush all.
        {
            XScaleFlushDCache(DCACHE_LINES, ARMCacheInfo.dwL1DCacheLineSize, (DWORD) gpvCacheFlushBaseMemoryAddress);
        }
        else if (dwLength >= (DWORD) ARMCacheInfo.dwL1DCacheSize)
        {
            XScaleFlushDCache(DCACHE_LINES, ARMCacheInfo.dwL1DCacheLineSize, (DWORD) gpvCacheFlushBaseMemoryAddress);
        }
        else
        {
            // normalize address to cache line alignment and adjust the length accordingly
            DWORD dwNormalizedAddress = (DWORD) pAddr & ~(ARMCacheInfo.dwL1DCacheLineSize - 1);
            DWORD dwNormalizedLength = dwLength + ((DWORD) pAddr - dwNormalizedAddress);

            // write back all the indicated cache entries
            XScaleCleanDCacheLines((LPVOID) dwNormalizedAddress, dwNormalizedLength, ARMCacheInfo.dwL1DCacheLineSize);
        }
    }


    ///////////////////////////////////////////////////////////////////////////
    //  Flush ICACHE
    ///////////////////////////////////////////////////////////////////////////

    if(dwFlags & CACHE_SYNC_INSTRUCTIONS)
    {

        if (!(dwLength || (DWORD)pAddr))    // If length and address are both zero, flush all.
        {
            OALFlushICache();
        }
        else if (dwLength >= (DWORD) ARMCacheInfo.dwL1ICacheSize)
        {
            OALFlushICache();
        }
        else
        {
            // normalize address and length
            DWORD dwNormalizedAddress, dwNormalizedLength;
            dwNormalizedAddress = (DWORD) pAddr & ~(ARMCacheInfo.dwL1ICacheLineSize - 1);
            dwNormalizedLength = dwLength + ((DWORD) pAddr - dwNormalizedAddress);
            
            // invalidate all the indicated cache entries
            XScaleFlushICacheLines((LPVOID) dwNormalizedAddress, dwNormalizedLength, ARMCacheInfo.dwL1ICacheLineSize);
        }
    }


    ///////////////////////////////////////////////////////////////////////////
    //  Flush I-TLB
    ///////////////////////////////////////////////////////////////////////////

    if(dwFlags & CACHE_SYNC_FLUSH_I_TLB)
    {

        if(dwLength == PAGE_SIZE)
        {
            // flush a single entry from the I-TLB
            //
            OALClearITLBEntry(pAddr);         
        }
        else
        {
            // flush the entire TLB
            //
            OALClearITLB();
        }
    }


    ///////////////////////////////////////////////////////////////////////////
    //  Flush D-TLB
    ///////////////////////////////////////////////////////////////////////////

    if(dwFlags & CACHE_SYNC_FLUSH_D_TLB)
    {

        if(dwLength == PAGE_SIZE)
        {
            // flush a single entry from the I-TLB
            //
            OALClearDTLBEntry(pAddr);
        }
        else
        {
            // flush the entire TLB
            //
            OALClearDTLB();
        }
    }

}

//------------------------------------------------------------------------------
//
//  OEMSetMemoryAttributes
//
//  OEM function to change memory attributes that isn't supported by kernel.
//  Current implementaion only supports PAGE_WRITECOMBINE.
//
//  This function first try to use PAT, and then try MTRR if PAT isn't available.
//
//------------------------------------------------------------------------------
BOOL OEMSetMemoryAttributes (
    LPVOID pVirtAddr,       // Virtual address of region
    LPVOID pPhysAddrShifted,// PhysicalAddress >> 8 (to support up to 40 bit address)
    DWORD  cbSize,          // Size of the region
    DWORD  dwAttributes     // attributes to be set
    )
{
    if (PAGE_WRITECOMBINE != dwAttributes) {
        DEBUGMSG (1, (L"OEMSetMemoryAttributes: Only PAGE_WRITECOMBINE is supported\r\n"));
        return FALSE;
    }

    return NKVirtualSetAttributes (pVirtAddr, cbSize,
                                  0x4,                  // not cacheable, but bufferable
                                  0xC,                  // Mask of all cache related bits
                                  &dwAttributes);
}
