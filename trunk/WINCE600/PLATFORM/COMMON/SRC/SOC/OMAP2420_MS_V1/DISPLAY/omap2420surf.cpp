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
//------------------------------------------------------------------------------
//
//  File:  omap2420surf.cpp
//
#include "precomp.h"

//-----------------------------------------------------------------------------

OMAP2420Surf::
OMAP2420Surf(
    int width,
    int height,
    DWORD pa,
    VOID *pBits,
    int stride,
    EGPEFormat format
    ) : DDGPESurf(width, height, pBits, stride, format)
{
    BOOL rc = FALSE;
    UCHAR *pSurface = NULL;
    m_nOffsetInVideoMemory = pa;
    m_fInVideoMemory = (pa != 0);
    m_hSurface = NULL;
    m_pSurface = NULL;
    m_inCache = FALSE;
    m_pVidRegs = NULL;

    // Create external mapping when surface is in video memory. We get to
    // preblems with MapViewOfFile/VirtualAlloc. For whichever reason
    // somebody allocates first page in mapped view. To workaround this
    // issue we reserve one more page and ignore first one in virtual alloc.
    if (m_fInVideoMemory)
        {
        // Get surface size
        ULONG size = stride * height;

        // We have to align on page boundary
        DWORD offset = pa  & (PAGE_SIZE - 1);
        pa &= ~(PAGE_SIZE - 1);
        size += offset;

        // Create file mapping
        m_hSurface = CreateFileMapping(
            INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, size + PAGE_SIZE,
            NULL
            );
        if (m_hSurface == NULL)
            {
            DEBUGMSG(GPE_ZONE_ERROR, (L"ERROR: OMAP2420Surf::OMAP2420Surf: "
                L"CreateFileMapping failed!\r\n"
                ));
            goto cleanUp;
            }

        pSurface = (UCHAR*)MapViewOfFile(m_hSurface, FILE_MAP_WRITE, 0, 0, 0);
        if (pSurface == NULL)
            {
            DEBUGMSG(GPE_ZONE_ERROR, (L"ERROR: OMAP2420Surf::OMAP730Surf: "
                L"MapViewOfFile failed!\r\n"
                ));
            goto cleanUp;
            }

        if (!VirtualCopy(
                pSurface + PAGE_SIZE, (VOID*)(pa >> 8), size, 
                PAGE_READWRITE|PAGE_PHYSICAL|PAGE_NOCACHE
                ))
            {
            DEBUGMSG(GPE_ZONE_ERROR, (L"ERROR: OMAPV1030GPE::MapFrameBuffer: "
                L"VirtualCopy failed (0x%08x, 0x%08x, 0x%08x, 0x%08x)\r\n",
                m_pSurface, pa >> 8, size, PAGE_READWRITE | PAGE_PHYSICAL
                ));
            goto cleanUp;
            }

        // Adjust frame offset
        pSurface += offset + PAGE_SIZE;
        
        // Save original surface address
        m_pSurface = (UCHAR*)m_pVirtAddr;

        // And replace it with mapped one...
        m_pVirtAddr = (ADDRESS)pSurface;
        }

    rc = TRUE;

cleanUp:
    if (!rc && (pSurface != NULL))
        {
        ADDRESS address = (ADDRESS)pSurface;
        address &= ~(PAGE_SIZE - 1);
        UnmapViewOfFile((VOID*)address);
        }
}

//-----------------------------------------------------------------------------

OMAP2420Surf::
~OMAP2420Surf(
    )
{
    // Delete surface mapping if exists
    if (m_pSurface != NULL)
        {
        ADDRESS address = m_pVirtAddr;
        address -= PAGE_SIZE;
        address &= ~(PAGE_SIZE - 1);
        UnmapViewOfFile((VOID*)address);
        m_pVirtAddr = (ADDRESS)m_pSurface;
        }

    if (m_hSurface != NULL) CloseHandle(m_hSurface);
}


//------------------------------------------------------------------------------
//
//  Method:  SurfaceOk
//
//  Return true when surface was created succesfully.
//
BOOL
OMAP2420Surf::
SurfaceOk(
    )
{
    return (m_pSurface != NULL); 
}

//------------------------------------------------------------------------------
//
//  Method:  InCache
//
//  Set flag indicating surface requiring discard on next DMA bitblt
//
VOID
OMAP2420Surf::
InCache(
    ) {
    m_inCache = TRUE; 
    }

//------------------------------------------------------------------------------
//
//  Method:  WriteBack
//
//  Flush surface memory in cache.
//
VOID
OMAP2420Surf::
WriteBack(
    )
{
    ASSERT(m_pSurface != NULL);
    if (m_pSurface != NULL)
        {
        CacheRangeFlush(
            (VOID*)m_pSurface, m_nStrideBytes * m_nHeight, CACHE_SYNC_WRITEBACK
            );
        }
}

//------------------------------------------------------------------------------
//
//  Method:  Discard
//
//  Flush and invalidate surface memory in cache.
//
VOID
OMAP2420Surf::
Discard(
    )
{
    ASSERT(m_pSurface != NULL);
    if ((m_inCache) && (m_pSurface != NULL))
        {
        CacheRangeFlush(
            (VOID*)m_pSurface, m_nStrideBytes * m_nHeight, CACHE_SYNC_DISCARD
            );
        }
    m_inCache = FALSE;
}

//------------------------------------------------------------------------------
