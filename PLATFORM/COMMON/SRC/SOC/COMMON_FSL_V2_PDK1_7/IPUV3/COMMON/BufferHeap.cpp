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
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  BufferHeap.cpp
//
//  Implementation of heap class for buffer allocation
//
//-----------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#pragma warning(pop)

#include "IpuBuffer.h"
//------------------------------------------------------------------------------
// External Functions

//------------------------------------------------------------------------------
// External Variables

//------------------------------------------------------------------------------
// Defines
#define ZONE_BUFFERHEAP    0
//------------------------------------------------------------------------------
// Types

//------------------------------------------------------------------------------
// Global Variables

//------------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
// Local Functions

//-----------------------------------------------------------------------------
//
// Function: Available
//
// Return the avaliable space in the heap.
//
// Parameters:
//      None.
//
// Returns:
//      The available space.
//
//-----------------------------------------------------------------------------

DWORD BufferHeap::Available()
{
    BufferHeap *pNode;
    DWORD available = 0;

    for( pNode = this; pNode; pNode = pNode->m_pNext )
        if( !pNode->m_fUsed )
            available += pNode->m_nSize;

    return available;
}

//-----------------------------------------------------------------------------
//
// Function: BufferHeap
//
// Buffer heap class constructor.
//
// Parameters:
//      size
//          [in] The size of buffer want to be created, in bytes
//
//      base
//          [in] The heap start address.
//
//      pNext
//          [in] The next heap pointer
//
//      pPrev
//          [in] The previous heap pointer
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
BufferHeap::BufferHeap( DWORD size, UINT32 base, BufferHeap *pNext, BufferHeap *pPrev )
{
    DEBUGMSG(ZONE_BUFFERHEAP,(TEXT("BufferHeap::BufferHeap size %d address 0x%08x pPrev %08x pNext %08x this %08x\r\n"),
        size, base, pPrev, pNext, this ));
    m_pNext = pNext;
    if( pNext != NULL )
    {
        m_pNext->m_pPrev = this;
    }
    m_pPrev = pPrev; 
    if( pPrev != NULL )
    {
        m_pPrev->m_pNext = this;
    }
    m_pStart = base;
    m_nSize = size & 0xfffff000; //aligned with 4k
    m_fUsed = 0;
}
//-----------------------------------------------------------------------------
//
// Function: ~BufferHeap
//
// The destructor for Class.  Deletes heap.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------

BufferHeap::~BufferHeap()
{
    DEBUGMSG(ZONE_BUFFERHEAP,(TEXT("BufferHeap::~BufferHeap 0x%08x (size=%d)\r\n"),this,m_nSize));
    if( !m_pPrev )
    {
        DEBUGMSG(ZONE_BUFFERHEAP,(TEXT("Deleting heap!\r\n")));
        // if this is the base, then we are deleting the heap,
        BufferHeap *pLast = this;
        while( pLast->m_pNext )
            pLast = pLast->m_pNext;
        while( pLast != this )
        {
            pLast = pLast->m_pPrev;
            //Avoid prefast warning 6011            
            if(pLast->m_pNext)
            {
                pLast->m_pNext->m_pPrev = (BufferHeap *)NULL;    // prevent BufferHeap::~BufferHeap from doing anything
                delete pLast->m_pNext;
                pLast->m_pNext = (BufferHeap *)NULL;
            }
        }
    }
    else
    {
        DEBUGMSG(ZONE_BUFFERHEAP,(TEXT("Deleting node only\r\n")));
        // otherwise, we are just freeing this node
        m_pPrev->m_nSize += m_nSize;
        m_pPrev->m_pNext = m_pNext;
        if( m_pNext )
            m_pNext->m_pPrev = m_pPrev;
    }
}

//-----------------------------------------------------------------------------
//
// Function: Alloc
//
// Allocate a small heap from the heap pool.
//
// Parameters:
//      size
//          [in] The size of buffer want to be created, in bytes, 4k aligned
//
// Returns:
//      Return the small heap.
//
//-----------------------------------------------------------------------------

BufferHeap *BufferHeap::Alloc( DWORD size )
{
    BufferHeap *pNode = this;
    size = (size+0x00000fff)&0xfffff000; //aligned with 4k
    DEBUGMSG(ZONE_BUFFERHEAP,(TEXT("BufferHeap::Alloc(%d)\r\n"),size));
    while( pNode && ( ( pNode->m_fUsed ) || ( pNode->m_nSize < size ) ) )
        pNode = pNode->m_pNext;
    if( pNode && ( pNode->m_nSize > size ) )
    {
        DEBUGMSG(ZONE_BUFFERHEAP,(TEXT("Splitting %d byte node at 0x%08x\r\n"), pNode->m_nSize, pNode ));
        // Ok, have to split into used & unused section
        BufferHeap *pFreeNode = new
            BufferHeap(
                pNode->m_nSize - size,                                // size
                (UINT32)(size + (unsigned long)pNode->m_pStart),    // start
                pNode->m_pNext,                                        // next
                pNode                                                // prev
                );
        if( !pFreeNode )
        {
            pNode = (BufferHeap *)NULL;    // out of memory for new node
            DEBUGMSG(ZONE_BUFFERHEAP,(TEXT("Failed to allocate new node\r\n")));
        }
        else
        {
            pNode->m_nSize = size;
        }
    }
    if( pNode )
    {
        pNode->m_fUsed = 1;
        DEBUGMSG(ZONE_BUFFERHEAP,(TEXT("Marking node at 0x%08x as used (offset = 0x08x)\r\n"),pNode, pNode->Address()));
    }
    return pNode;
}

//-----------------------------------------------------------------------------
//
// Function: Free
//
// Free the small heap back to the heap pool.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------

void BufferHeap::Free()
{
    BufferHeap *pMerge;
    BufferHeap *pNode = this;

    pNode->m_fUsed = 0;
    DEBUGMSG(ZONE_BUFFERHEAP,(TEXT("BufferHeap::Free 0x%08x (size=%d)\r\n"),pNode,pNode->m_nSize));
    pMerge = pNode->m_pPrev;
    if( pMerge && !pMerge->m_fUsed )
    {
        DEBUGMSG(ZONE_BUFFERHEAP,(TEXT("BufferHeap::Free - merging node %08x with prior node (%08x)\r\n"),
            pNode, pMerge ));
        delete pNode;            // Merge pNode with prior node
        pNode = pMerge;
    }
    pMerge = pNode->m_pNext;
    if( pMerge && !pMerge->m_fUsed )
    {
        DEBUGMSG(ZONE_BUFFERHEAP,(TEXT("BufferHeap::Free - merging %08x with subsequent node (%08x)\r\n"),
            pNode, pMerge ));
        delete pMerge;            // Merge pNode with subsequent node
    }
}

//-----------------------------------------------------------------------------
//
// Function: Size
//
// Accumlate the total size of heap.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------

DWORD BufferHeap::Size()
{
    // this shouldn't be called too often, - just walk through accumulating total size;
    BufferHeap *pNode;
    DWORD size = 0;

    for( pNode=this; pNode; pNode = pNode->m_pNext )
        size += pNode->m_nSize;

    return size;
}
