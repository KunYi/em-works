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
#include <windows.h>
#include <nkglobal.h>

//-----------------------------------------------------------------------------
//
//  OEMCacheRangeFlush
//
void OEMCacheRangeFlush(
                        LPVOID pAddr, 
                        DWORD dwLength, 
                        DWORD dwFlags
                        )
{
    UNREFERENCED_PARAMETER(pAddr);
    UNREFERENCED_PARAMETER(dwLength);

    // cache flush?
    if (dwFlags & (CACHE_SYNC_WRITEBACK | CACHE_SYNC_DISCARD)) 
    {
        DWORD dwUncacheAddress = g_pNKGlobal->dwKnownUncachedAddress;
        
        // just perform an uncached access
        _asm {  mov eax, dword ptr ds:[dwUncacheAddress] }
    }

    if (dwFlags & CACHE_SYNC_FLUSH_TLB) 
    {
        // flush TLB
        _asm {
            mov     eax, cr3
            mov     cr3, eax
        }
    }
}

//------------------------------------------------------------------------------

