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
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Module Name:
//     CPhysMem.cpp
//
// Abstract:
//     physical memory manager
//
// Notes:
//


/*---------------------------------------------------------------------------
* Copyright (C) 2006,2007 Freescale Semiconductor, Inc. All Rights Reserved.
* THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
* AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
*--------------------------------------------------------------------------*/


#include "cphysmem.hpp"

#ifdef IRAM_PATCH
extern "C" {
extern BOOL BSPCheckIfAddrInIRam(DWORD pPhysAddr);
}
#endif

#define RETAIL_LOG 0
#define CRITICAL_LOG 0
typedef LPVOID * LPLPVOID;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
CPhysMem::CPhysMem(
    DWORD   cbSize,
    DWORD   cbHighPrioritySize,
    PUCHAR  pVirtAddr,
    PUCHAR  pPhysAddr
    )
    : m_cbTotal(cbSize),
      m_cbHighPri(cbHighPrioritySize),
      m_pVirtBase(pVirtAddr),
      m_pPhysBase(pPhysAddr)
{

    ASSERT(cbSize > 0 && cbHighPrioritySize > 0); // must be so or the driver cannot work.

    //
    // The PDD can pass in a physical buffer, or we'll try to allocate one from
    // system RAM.
    //
    if (pVirtAddr && pPhysAddr) {
        DEBUGMSG(ZONE_INIT,(TEXT("DMA buffer passed in from PDD\r\n")));
        m_pPhysicalBufferAddr = pVirtAddr;
        m_dwNormalVA = (DWORD) pVirtAddr;
        m_dwNormalPA = (DWORD) pPhysAddr;
        m_fPhysFromPlat = TRUE;
    }
    else {
        DEBUGMSG(ZONE_INIT,(TEXT("Allocating DMA buffer from system RAM\r\n")));

        m_pPhysicalBufferAddr = (PUCHAR)AllocPhysMem(m_cbTotal,
                                             PAGE_READWRITE|PAGE_NOCACHE,
                                             0,    // Default alignment
                                             0,    // Reserved
                                             &m_dwNormalPA);

        m_dwNormalVA = (DWORD) m_pPhysicalBufferAddr;
        m_fPhysFromPlat = FALSE;
    }
    {   // we want all blocks to have their Phys Addr divisible by
        // CPHYSMEM_MEMORY_ALIGNMENT. To achieve this, we start off
        // having the physical memory block aligned properly, and
        // then allocate memory only in blocks divisible by
        // CPHYSMEM_MEMORY_ALIGNMENT
        const DWORD dwOffset = m_dwNormalPA & (CPHYSMEM_MEMORY_ALIGNMENT - 1);
        DEBUGCHK( dwOffset == m_dwNormalPA % CPHYSMEM_MEMORY_ALIGNMENT );
        DEBUGCHK( cbSize > CPHYSMEM_MEMORY_ALIGNMENT );
        if ( dwOffset != 0 ) {
            // skip over the first few bytes of memory, as it is not
            // aligned properly. This shouldn't happen though, because
            // the new memory should have been aligned on a page boundary
            DEBUGCHK( 0 );
            // we can't code -= dwOffset because then we'll enter
            // memory that we don't own.
            m_dwNormalVA += CPHYSMEM_MEMORY_ALIGNMENT - dwOffset;
            m_dwNormalPA += CPHYSMEM_MEMORY_ALIGNMENT - dwOffset;
            cbSize -= CPHYSMEM_MEMORY_ALIGNMENT - dwOffset;
        }
    }

    m_dwTotalPhysMemSize = cbSize;
    m_PaVaConversion = m_dwNormalPA - m_dwNormalVA;

    DEBUGMSG(ZONE_INIT,
        (TEXT("CPhysMem   Total Alloc Region PhysAddr = 0x%08X, VirtAddr = 0x%08X, size = %d\r\n"),
        m_dwNormalPA, m_dwNormalVA, m_dwTotalPhysMemSize));

    //
    // Set aside a page for the special request.
    //
#ifdef IRAM_PATCH
    if (BSPCheckIfAddrInIRam((DWORD)pPhysAddr))
    {
        //do not allocate special mem
        RETAILMSG(RETAIL_LOG, (L"We will not allocate special memory\r\n"));
        m_dwSpecialVA = (DWORD) m_dwNormalVA;
        m_dwSpecialPA = (DWORD) m_dwNormalPA;
        m_bSpecialTaken = FALSE;
    }
    else
    {
        //
        m_dwSpecialVA = (DWORD) m_dwNormalVA;
        m_dwSpecialPA = (DWORD) m_dwNormalPA;
        m_dwNormalVA += USBPAGESIZE;
        m_dwNormalPA += USBPAGESIZE;
        cbSize -= USBPAGESIZE;
        m_bSpecialTaken = FALSE;
        memset((PVOID) m_dwSpecialVA, 0x00, USBPAGESIZE);
    }
#else
    m_dwSpecialVA = (DWORD) m_dwNormalVA;
    m_dwSpecialPA = (DWORD) m_dwNormalPA;
    m_dwNormalVA += USBPAGESIZE;
    m_dwNormalPA += USBPAGESIZE;
    cbSize -= USBPAGESIZE;
    m_bSpecialTaken = FALSE;
    memset((PVOID) m_dwSpecialVA, 0x00, USBPAGESIZE);
#endif

    DEBUGMSG(ZONE_INIT,
        (TEXT("CPhysMem Special Alloc Region PhysAddr = 0x%08X, VirtAddr = 0x%08X, size = %d\r\n"),
        m_dwSpecialPA, m_dwSpecialVA, USBPAGESIZE));

    //
    // Set aside the High Priority region.
    //
    m_dwHighPriorityVA = (DWORD) m_dwNormalVA;
    m_dwHighPriorityPA = (DWORD) m_dwNormalPA;
    m_dwNormalVA += cbHighPrioritySize;
    m_dwNormalPA += cbHighPrioritySize;
    cbSize -= cbHighPrioritySize;
    m_dwHighPrioritySize = cbHighPrioritySize;
#ifdef EHCI_PROBE
    RETAILMSG(CRITICAL_LOG, (L"VA %x, Size %x\r\n", m_dwHighPriorityVA, m_dwHighPrioritySize));
#endif
    memset((PVOID) m_dwHighPriorityVA, 0x00, m_dwHighPrioritySize);

    DEBUGMSG(ZONE_INIT,
        (TEXT("CPhysMem HighPri Alloc Region PhysAddr = 0x%08X, VirtAddr = 0x%08X, size = %d\r\n"),
        m_dwHighPriorityPA, m_dwHighPriorityVA, m_dwHighPrioritySize));

    //
    // And the rest is for normal allocations.
    //
    m_dwNormalSize = cbSize;
#ifdef DETAIL_MSG
    RETAILMSG(RETAIL_LOG, (L"memset %d bytes\r\n", m_dwNormalSize));
#endif
#ifdef EHCI_PROBE
    RETAILMSG(CRITICAL_LOG, (L"VA %x, Size %x\r\n", m_dwNormalVA, m_dwNormalSize));
#endif
    memset((PVOID) m_dwNormalVA, 0x00, m_dwNormalSize);

    DEBUGMSG(ZONE_INIT,
        (TEXT("CPhysMem  Normal Alloc Region PhysAddr = 0x%08X, VirtAddr = 0x%08X, size = %d\r\n"),
        m_dwNormalPA, m_dwNormalVA, m_dwNormalSize));



    m_hFreeMemEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    m_fHasBlocked = FALSE;
    InitializeCriticalSection(&m_csLock);
#ifdef IRAM_PATCH
    m_pNodeFreeListHead = NULL;
    m_pFreeListHead = NULL ;
    m_pInUseListHead = NULL ;
    m_pHighPriorityFreeListHead = NULL ;
    m_pHighPriorityInUseListHead = NULL ;
#endif

    ReInit();

}
void CPhysMem::ReInit()
{
    PMEMLIST pNode;
    DWORD StringSize;
    StringSize = 0;
    EnterCriticalSection(&m_csLock);
    //
    // Create dummy entries for the list head (simpler linked list code)
    //
#ifdef IRAM_PATCH
    if (m_pNodeFreeListHead)
    {
        FreeList(&m_pNodeFreeListHead);
    }
    if (m_pFreeListHead)
    {
        FreeList(&m_pFreeListHead);
    }
    if (m_pInUseListHead)
    {
        FreeList(&m_pInUseListHead);
    }
    if (m_pHighPriorityFreeListHead)
    {
        FreeList(&m_pHighPriorityFreeListHead);
    }
    if (m_pHighPriorityInUseListHead)
    {
        FreeList(&m_pHighPriorityInUseListHead);
    }
#endif
    m_pNodeFreeListHead = CreateNewNode(0, 0, 0);
    if (m_pNodeFreeListHead) InitializeListHead(m_pNodeFreeListHead);

    DEBUGMSG(ZONE_CPHYSMEM && ZONE_VERBOSE, (TEXT("CPhysMem Init : NodeFreeListHead = 0x%08X\r\n"),
                          m_pNodeFreeListHead));

    m_pFreeListHead = CreateNewNode(0, 0, 0);
    if (m_pFreeListHead) InitializeListHead(m_pFreeListHead);

    m_pInUseListHead = CreateNewNode(0, 0, 0);
    if (m_pInUseListHead) InitializeListHead(m_pInUseListHead);


    DEBUGMSG(ZONE_CPHYSMEM && ZONE_VERBOSE, (TEXT("CPhysMem Init : FreeListHead = 0x%08X, InUseListHead = 0x%08X\r\n"), m_pFreeListHead, m_pInUseListHead));

    m_pHighPriorityFreeListHead = CreateNewNode(0, 0, 0);
    if (m_pHighPriorityFreeListHead) InitializeListHead(m_pHighPriorityFreeListHead);

    m_pHighPriorityInUseListHead = CreateNewNode(0, 0, 0);
    if (m_pHighPriorityInUseListHead) InitializeListHead(m_pHighPriorityInUseListHead);

    DEBUGMSG(ZONE_CPHYSMEM && ZONE_VERBOSE, (TEXT("CPhysMem Init : HighPriFreeListHead = 0x%08X, HighPriInUseListHead = 0x%08X\r\n"),
                          m_pHighPriorityFreeListHead, m_pHighPriorityInUseListHead));

    // Send an alert if we're being constructed under OOM conditions.
    m_fInitted =
        (m_pNodeFreeListHead && m_pFreeListHead && m_pInUseListHead &&
         m_pHighPriorityFreeListHead && m_pHighPriorityInUseListHead);

#ifdef DETAIL_MSG
    RETAILMSG(RETAIL_LOG, (L"%x Null head created success? %d\r\n", this, m_fInitted));
#endif
    //
    // One big chunk on the free list to start things off.
    //
    pNode = CreateNewNode(m_dwNormalSize, m_dwNormalVA, m_dwNormalPA);
#ifdef DETAIL_MSG
    RETAILMSG(RETAIL_LOG, (L"normal node @ %x, with start %x (%x), size %d\r\n", pNode, pNode->dwVirtAddr, pNode->dwPhysAddr, pNode->dwSize));
#endif
    if (pNode) {
#ifdef DEBUG
       StringSize = sizeof(pNode->szDescription)/sizeof(TCHAR);
       StringCchCopy(pNode->szDescription,StringSize,TEXT("Free Low Pri Mem"));
       // _tcscpy( pNode->szDescription, TEXT("Free Low Pri Mem") );
#endif // DEBUG

        DEBUGMSG(ZONE_CPHYSMEM && ZONE_VERBOSE, (TEXT("CPhysMem Init : Main Free Heap Node = 0x%08X\r\n"), pNode));

        InsertNodeBefore(pNode, FirstNode(m_pFreeListHead));

        VALIDATE_HEAPS(FALSE);
    } else
        m_fInitted = FALSE;

    //
    // Same thing for High Priority Region
    //
    pNode = CreateNewNode(m_dwHighPrioritySize, m_dwHighPriorityVA, m_dwHighPriorityPA);
#ifdef DETAIL_MSG
    RETAILMSG(RETAIL_LOG, (L"hp node @ %x\r\n", pNode));
#endif
    if (pNode) {
#ifdef DEBUG
        StringSize = sizeof(pNode->szDescription)/sizeof(TCHAR);
        StringCchCopy(pNode->szDescription,StringSize,TEXT("Free Low Pri Mem"));
       // _tcscpy( pNode->szDescription, TEXT("Free High Pri Mem") );
#endif // DEBUG

        DEBUGMSG(ZONE_CPHYSMEM && ZONE_VERBOSE, (TEXT("CPhysMem Init : HighPri Free Heap Node = 0x%08X\r\n"),
                                                 pNode));

        InsertNodeBefore(pNode, FirstNode(m_pHighPriorityFreeListHead));

        VALIDATE_HEAPS(TRUE);
    } else
        m_fInitted = FALSE;

    LeaveCriticalSection(&m_csLock);
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#ifdef DEBUG
BOOL
CPhysMem::ValidateHeaps(BOOL fHighPri)
{
    PMEMLIST pNode = FirstNode(FREELIST(fHighPri));
    DWORD dwSizeTotal = 0;
    DWORD dwSizePrev = 0;
    DWORD dwSizeFree = 0;
    DWORD dwNodesFree = 0;

    while (!EndOfList(FREELIST(fHighPri), pNode)) {

        DEBUGMSG(ZONE_VERBOSE && ZONE_CPHYSMEM, (TEXT("FreeList(pri = %d) : pNode size = %4d, PA = 0x%08x, VA = 0x%08X, Desc = %s\r\n"),
                              fHighPri, pNode->dwSize, pNode->dwPhysAddr, pNode->dwVirtAddr, pNode->szDescription ));

        if ( pNode->dwSize == 0 ) {
            DEBUGCHK( pNode->dwVirtAddr == 0 &&
                      pNode->dwPhysAddr == 0 );
        } else {
            DEBUGCHK( pNode->dwSize % CPHYSMEM_MEMORY_ALIGNMENT == 0 &&
                      pNode->dwPhysAddr % CPHYSMEM_MEMORY_ALIGNMENT == 0 &&
                      PUCHAR(pNode->dwVirtAddr) == PaToVa( pNode->dwPhysAddr ) );
        }

        dwSizeTotal += pNode->dwSize;
        dwSizeFree  += pNode->dwSize;

        if (dwSizePrev > pNode->dwSize) {
            DEBUGMSG(ZONE_ERROR, (TEXT("CPhysMem ValidateHeaps : Free List not sorted (%d > %d)\r\n"),
                         dwSizePrev, pNode->dwSize));
            DEBUGCHK(0);
            return(FALSE);
        }

        if ((pNode->next->prev != pNode) || (pNode->prev->next != pNode)) {
            DEBUGMSG(ZONE_ERROR, (TEXT("CPhysMem ValidateHeaps : Invalid linked list (free)\r\n")));
            DEBUGCHK(0);
            return(FALSE);
        }

        dwSizePrev = pNode->dwSize;
        pNode = pNode->next;
    }

    pNode = FirstNode(INUSELIST(fHighPri));

    while (!EndOfList(INUSELIST(fHighPri), pNode)) {

        DEBUGMSG(ZONE_VERBOSE && ZONE_CPHYSMEM, (TEXT("InUseList(pri = %d) : pNode size = %4d, PA = 0x%08x, VA = 0x%08X, Desc = %s\r\n"),
                              fHighPri, pNode->dwSize, pNode->dwPhysAddr, pNode->dwVirtAddr, pNode->szDescription ));

        if ( pNode->dwSize == 0 ) {
            DEBUGCHK( pNode->dwVirtAddr == 0 &&
                      pNode->dwPhysAddr == 0 );
        } else {
            DEBUGCHK( pNode->dwSize % CPHYSMEM_MEMORY_ALIGNMENT == 0 &&
                      pNode->dwPhysAddr % CPHYSMEM_MEMORY_ALIGNMENT == 0 &&
                      PUCHAR(pNode->dwVirtAddr) == PaToVa( pNode->dwPhysAddr ) );
        }

        dwSizeTotal += pNode->dwSize;

        if ((pNode->next->prev != pNode) || (pNode->prev->next != pNode)) {
            DEBUGMSG(ZONE_ERROR, (TEXT("CPhysMem ValidateHeaps : Invalid linked list (inuse)\r\n")));
            DEBUGCHK(0);
            return(FALSE);
        }

        pNode = pNode->next;
    }

    DEBUGMSG(ZONE_CPHYSMEM, (TEXT("CPhysMem ValidateHeaps : FreeSize = %d bytes; TotalSize = %d bytes\r\n"),
                          dwSizeFree, dwSizeTotal));

    if (dwSizeTotal != (fHighPri ? m_dwHighPrioritySize : m_dwNormalSize)) {
        DEBUGMSG(ZONE_ERROR, (TEXT("CPhysMem ValidateHeaps : We've lost some memory somewhere\r\n")));
        DEBUGCHK(0);
        return(FALSE);
    }

    //
    // Validate the NODES free list.
    //
    pNode = FirstNode(m_pNodeFreeListHead);
    while (!EndOfList(m_pNodeFreeListHead, pNode)) {

        dwNodesFree++;
        // these are set in DeleteNode
        DEBUGCHK( pNode->dwSize == 0xdeadbeef &&
                  pNode->dwPhysAddr == 0xdeadbeef &&
                  pNode->dwVirtAddr == 0xdeadbeef );

        if ((pNode->next->prev != pNode) || (pNode->prev->next != pNode)) {
            DEBUGMSG(ZONE_ERROR, (TEXT("CPhysMem ValidateHeaps : Invalid linked list (nodefree)\r\n")));
            DEBUGCHK(0);
            return(FALSE);
        }
        pNode = pNode->next;
    }
    DEBUGMSG(ZONE_CPHYSMEM && ZONE_VERBOSE, (TEXT("CPhysMem ValidateHeaps : Nodes Free = %d\r\n"),dwNodesFree));

    return (TRUE);
}
#endif


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
PMEMLIST
CPhysMem::CreateNewNode(
    DWORD   dwSize,
    DWORD   dwVirtAddr,
    DWORD   dwPhysAddr
    )
{
#ifdef DEBUG
    if ( dwSize == 0 ) {
        DEBUGCHK( dwVirtAddr == 0 &&
                  dwPhysAddr == 0 );
    } else {
        DEBUGCHK( dwSize % CPHYSMEM_MEMORY_ALIGNMENT == 0 &&
                  dwPhysAddr % CPHYSMEM_MEMORY_ALIGNMENT == 0 &&
                  PUCHAR(dwVirtAddr) == PaToVa( dwPhysAddr ) );
    }
#endif // DEBUG

    PMEMLIST pNode;
    //
    // If we already have a node allocated and sitting around, use it.
    //
    if ((dwSize == 0) || IsListEmpty(m_pNodeFreeListHead)) {
        pNode = (PMEMLIST) CPhysMem_Alloc(LPTR, sizeof(MEMLIST));
    } else {
        pNode = FirstNode(m_pNodeFreeListHead);
        RemoveNode(pNode);
    }

    if (pNode != NULL) {
        pNode->dwVirtAddr = dwVirtAddr;
        pNode->dwPhysAddr = dwPhysAddr;
        pNode->dwSize = dwSize;
        pNode->next = NULL;
        pNode->prev = NULL;
    #ifdef DEBUG
        DWORD StringSize = sizeof(pNode->szDescription)/sizeof(TCHAR);
        StringCchCopy(pNode->szDescription,StringSize,TEXT("Default Desc"));
        //_tcscpy( pNode->szDescription, TEXT("Default Desc") );
    #endif // DEBUG
    }

    return (pNode);
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
BOOL
CPhysMem::DeleteNode(
    PMEMLIST pNode
    )
{
    //
    // We don't actually delete any of the nodes. We just keep them on our
    // free list to use later. Keeps us from thrashing on the heap.
    //
#ifdef DEBUG
    pNode->dwSize = 0xdeadbeef;
    pNode->dwPhysAddr = 0xdeadbeef;
    pNode->dwVirtAddr = 0xdeadbeef;
    DWORD StringSize = sizeof(pNode->szDescription)/sizeof(TCHAR);
    StringCchCopy(pNode->szDescription,StringSize,TEXT("Deleted Node"));

    //_tcscpy( pNode->szDescription, TEXT("Deleted Node") );
#endif // DEBUG
    InsertNodeBefore(pNode, FirstNode(m_pNodeFreeListHead));
    return(TRUE);
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
PMEMLIST
CPhysMem::FindFreeBlock(
    DWORD   dwSize,
    BOOL    fHighPri
    )
{
    DEBUGCHK( dwSize >= CPHYSMEM_MEMORY_ALIGNMENT &&
              dwSize % CPHYSMEM_MEMORY_ALIGNMENT == 0 );
    //
    // The free list is sorted by increasing block sizes, so just find the
    // first block that's at least "dwSize" big.
    //
    PMEMLIST pNode = FirstNode(FREELIST(fHighPri));
#ifdef DETAIL_MSG
    RETAILMSG(RETAIL_LOG, (L"FFN(%d) %x\r\n", fHighPri, pNode));
#endif

    while (!EndOfList(FREELIST(fHighPri), pNode)) {
#ifdef EHCI_PROBE
        RETAILMSG(RETAIL_LOG, (L"pN %x\r\n", pNode));
#endif
        if (dwSize <= pNode->dwSize) {
            RemoveNode(pNode);
            return (pNode);
        }
        pNode = pNode->next;
    }

    return (NULL);
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
BOOL
CPhysMem::AddNodeToFreeList(
    PMEMLIST pNode,
    BOOL     fHighPri
    )
{
    //
    // The free list is sorted by increasing block sizes, not by increasing
    // address so we must scan the list for any possible connecting free blocks,
    // and then coalesce them into a single free block. There will be at most
    // two blocks to find (one on either end) so scan for both of them.
    //
    PMEMLIST pNodeTraverse = FirstNode(FREELIST(fHighPri));

    PMEMLIST pNodePrevious = NULL; // Points to the previous connecting free block
    PMEMLIST pNodeNext = NULL;     // Points to the next connecting free block
    //
    // The endpoints that we are trying to match up to.
    //
    DWORD dwThisPA = pNode->dwPhysAddr;
    DWORD dwNextPA = MEMLIST_NEXT_PA(pNode);

    //
    // Walk the list looking for blocks that are next to this one.
    //
    while (!EndOfList(FREELIST(fHighPri), pNodeTraverse)) {

        if (dwThisPA == MEMLIST_NEXT_PA(pNodeTraverse)) {
            //
            // We've found the block just ahead of this one. Remember it.
            //
            pNodePrevious = pNodeTraverse;

        } else if (dwNextPA == pNodeTraverse->dwPhysAddr) {
            //
            // We've found the block just after of this one.
            //
            pNodeNext = pNodeTraverse;
        }

        if ((pNodePrevious == NULL) || (pNodeNext == NULL)) {
            //
            // We haven't connected both ends, so keep on looking...
            //
            pNodeTraverse = pNodeTraverse->next;
        } else {
            //
            // We've found blocks to connect on both ends, let's get on with it.
            //
            break;
        }
    }


    if (pNodePrevious != NULL) {
        //
        // Combine with the previous block.
        //
        RemoveNode(pNodePrevious);
        //
        // Grow pNode to hold both.
        //
        pNode->dwSize = pNode->dwSize + pNodePrevious->dwSize;
        pNode->dwVirtAddr = pNodePrevious->dwVirtAddr;
        pNode->dwPhysAddr = pNodePrevious->dwPhysAddr;
        DeleteNode(pNodePrevious);
    }

    if (pNodeNext != NULL) {
        //
        // Combine with the next block.
        //
        RemoveNode(pNodeNext);
        //
        // Grow pNode to hold both.
        //
        pNode->dwSize = pNode->dwSize + pNodeNext->dwSize;
    #ifdef DEBUG
        // take description of the largest block
        DWORD StringSize = sizeof(pNode->szDescription)/sizeof(TCHAR);
        StringCchCopy(pNode->szDescription,StringSize,pNodeNext->szDescription);


       // _tcscpy( pNode->szDescription, pNodeNext->szDescription );
    #endif // DEBUG
        DeleteNode(pNodeNext);
    }

    //
    // Add pNode to the free list in sorted size order.
    //
    pNodeTraverse = FirstNode(FREELIST(fHighPri));

    while (!EndOfList(FREELIST(fHighPri), pNodeTraverse)) {

        if (pNode->dwSize <= pNodeTraverse->dwSize) {
            break;
        }
        pNodeTraverse = pNodeTraverse->next;
    }

    //
    // Insert this node before the traverse node.
    //
    InsertNodeBefore(pNode, pNodeTraverse);

    return TRUE;
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
CPhysMem::~CPhysMem()
{
    DeleteCriticalSection(&m_csLock);
    CloseHandle(m_hFreeMemEvent);

    FreeList(&m_pInUseListHead);
    FreeList(&m_pFreeListHead);
    FreeList(&m_pHighPriorityInUseListHead);
    FreeList(&m_pHighPriorityFreeListHead);

    FreeList(&m_pNodeFreeListHead);

    if (!m_fPhysFromPlat)
        FreePhysMem(m_pPhysicalBufferAddr);
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
BOOL CPhysMem::FreeList(PMEMLIST *ppHead)
{
    PMEMLIST pCurrent;
    PMEMLIST pNext;

    if ( *ppHead != NULL ) {
        pCurrent = (*ppHead)->next;
        while ( pCurrent != *ppHead ) {
            DEBUGCHK( pCurrent != NULL );
            pNext = pCurrent->next;
            CPhysMem_Free( pCurrent );
            pCurrent = pNext;
        }
        CPhysMem_Free( *ppHead );
        *ppHead = NULL;
    }

    return(TRUE);
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

BOOL
CPhysMem::AllocateSpecialMemory(
    IN const DWORD DEBUG_ONLY( dwSize ), // dwSize ignored in retail
    OUT PUCHAR* const pVirtAddr )
{
    DEBUGCHK( dwSize <= USBPAGESIZE );
    PREFAST_DEBUGCHK( pVirtAddr != NULL );

    // during suspend/resume this routine will be called again; we can safely
    // leave the special memory set aside since we will always need the same amount.
    if(!m_bSpecialTaken) {
        m_bSpecialTaken = TRUE;
        DEBUGCHK( m_dwSpecialPA == VaToPa( PUCHAR(m_dwSpecialVA) ) );
        DEBUGCHK( m_dwSpecialPA % CPHYSMEM_MEMORY_ALIGNMENT == 0 );
    }

    *pVirtAddr = (PUCHAR) m_dwSpecialVA;

    DEBUGMSG(ZONE_CPHYSMEM && ZONE_VERBOSE,(TEXT("CPhysMem AllocateMemory : bSpecial allocated\r\n")));
    return(TRUE);
}

void
CPhysMem::FreeSpecialMemory(
    IN const PUCHAR DEBUG_ONLY( virtAddr ) ) // virtAddr ignored in retail
{

    DEBUGCHK( m_dwSpecialVA == (DWORD) virtAddr );
    DEBUGCHK( m_bSpecialTaken );

    m_bSpecialTaken = FALSE;
}

BOOL
CPhysMem::AllocateMemory(
    DEBUG_PARAM( IN const TCHAR* pszMemDescription ) // description of memory being alloc'd
    IN const DWORD dwPassedInSize,
    OUT PUCHAR* const pVirtAddr,
    IN const DWORD dwFlags,
    IN BOOL* pfRequestingAbort // = NULL
    )
{
#ifdef DEBUG
    PREFAST_DEBUGCHK( pszMemDescription != NULL );
    DEBUGCHK( dwPassedInSize > 0 );
    // for now, only the following sets of flags should be passed in
    DEBUGCHK( dwFlags == 0 || // low priority, allow blocking
              dwFlags == CPHYSMEM_FLAG_NOBLOCK || // low priority, no blocking
              dwFlags == (CPHYSMEM_FLAG_HIGHPRIORITY | CPHYSMEM_FLAG_NOBLOCK) ); // high pri, no blocking

    if ( dwFlags & CPHYSMEM_FLAG_NOBLOCK ) {
        // pfRequestingAbort will be ignored for NO_BLOCK transfers,
        // so why is caller passing it in? Note that nothing
        // bad will happen if pfRequestingAbort != NULL, so
        // this check can be removed in the future if need be.
        DEBUGCHK( pfRequestingAbort == NULL );
    } else {
        // blocking transfers must pass in a pointer
        // for allowing the transfer to abort, and
        // the original state of this abort request
        // should be FALSE. If not, the blocking
        // request is ignored.
        DEBUGCHK( pfRequestingAbort != NULL &&
                  *pfRequestingAbort == FALSE );
    }
#endif // DEBUG

    PMEMLIST    pNode = NULL;
    const BOOL  fHighPri = !!(dwFlags & CPHYSMEM_FLAG_HIGHPRIORITY);
    const BOOL  fNoBlock = !!(dwFlags & CPHYSMEM_FLAG_NOBLOCK);
    // We keep our block sizes in multiples of CPHYSMEM_MEMORY_ALIGNMENT
    DWORD       dwSize = ( (dwPassedInSize - 1) & ~(CPHYSMEM_MEMORY_ALIGNMENT - 1) )
                                 + CPHYSMEM_MEMORY_ALIGNMENT;

    PREFAST_DEBUGCHK( pVirtAddr != NULL );
    DEBUGCHK( dwSize % CPHYSMEM_MEMORY_ALIGNMENT == 0 );
    DEBUGCHK( dwSize - dwPassedInSize < CPHYSMEM_MEMORY_ALIGNMENT );
    DEBUGMSG(ZONE_CPHYSMEM && ZONE_VERBOSE && (dwSize != dwPassedInSize),
             (TEXT("AllocateMemory Desc = %s: (roundup %d->%d)\r\n"), pszMemDescription, dwPassedInSize, dwSize ));

    EnterCriticalSection( &m_csLock );

    DEBUGMSG( ZONE_CPHYSMEM && ZONE_VERBOSE, (TEXT("CPhysMem: Heap pri = %d before allocation of %d bytes:\n"), fHighPri, dwSize ) );
    VALIDATE_HEAPS(fHighPri);

    //
    // Scan the free list for the first chunk that's just big enough to satisfy
    // this request. Remove from the free list. Chop it up (unless the result
    // is less than CPHYSMEM_MEMORY_ALIGNMENT bytes). Then re-sort the remaining
    // free chunk back into the free list and place the newly allocated chunk on
    // the IN USE list.
    //
#ifdef EHCI_PROBE
    RETAILMSG(RETAIL_LOG, (L"alloc %x(%d) @ %x\r\n", dwSize, fHighPri, this));
#endif
    pNode = FindFreeBlock(dwSize, fHighPri);
#ifdef DETAIL_MSG
    RETAILMSG(RETAIL_LOG, (L"find node %x", pNode));
#endif
    if ( pNode == NULL ) {
#ifdef EHCI_PROBE
        RETAILMSG(RETAIL_LOG, (L"no free block, need to check\r\n"));
#endif
        if ( fHighPri ) {
            //
            // Not available from High Priority region, try allocating from Normal region.
            //
            LeaveCriticalSection(&m_csLock);

            DEBUGCHK( dwFlags == (CPHYSMEM_FLAG_HIGHPRIORITY | CPHYSMEM_FLAG_NOBLOCK) );
            return AllocateMemory( DEBUG_PARAM( pszMemDescription )
                                   dwPassedInSize,
                                   pVirtAddr,
                                   CPHYSMEM_FLAG_NOBLOCK, // dwFlags & ~CPHYSMEM_FLAG_HIGHPRIORITY,
                                   pfRequestingAbort );

        } else if ( !fNoBlock &&
                    pfRequestingAbort != NULL ) {
            //
            // Caller requested block for memory
            //
        #ifdef DEBUG
            DWORD dwStartBlockTickCount = GetTickCount();
        #endif // DEBUG
            do {
                LeaveCriticalSection(&m_csLock);

                if ( *pfRequestingAbort == FALSE ) {
                    m_fHasBlocked = TRUE;
#ifdef EHCI_PROBE
                    RETAILMSG(RETAIL_LOG, (L"wait free mem\r\n"));
#endif
                    WaitForSingleObject(m_hFreeMemEvent, CPHYSMEM_BLOCK_FOR_MEM_INTERVAL );

                    if ( *pfRequestingAbort ) {
                        *pVirtAddr = NULL;
                        return FALSE;
                    }

                    // if this fails, we've been waiting for memory too long
                    DEBUGCHK( GetTickCount() - dwStartBlockTickCount < CPHYSMEM_DEBUG_MAXIMUM_BLOCK_TIME );
                }

                EnterCriticalSection(&m_csLock);

                pNode = FindFreeBlock(dwSize, fHighPri);
            } while ( pNode == NULL );
            // rest of processing done below
        } else {
            DEBUGMSG( ZONE_WARNING, (TEXT("CPhysMem AllocateMemory : No memory available") ));
            LeaveCriticalSection(&m_csLock);
            *pVirtAddr = NULL;
            return FALSE;
        }
    }

    // case pNode == NULL should have been handled above

    if ( pNode->dwSize - dwSize >= CPHYSMEM_MEMORY_ALIGNMENT) {
        // There's enough left over to create a new block.
        PMEMLIST pNodeNew = CreateNewNode(pNode->dwSize - dwSize,
                                          pNode->dwVirtAddr + dwSize,
                                          pNode->dwPhysAddr + dwSize);
#ifdef EHCI_PROBE
        RETAILMSG(RETAIL_LOG, (L"Create New Node %x, size %x\r\n", pNodeNew, pNode->dwSize - dwSize));
#endif
    #ifdef DEBUG
        DWORD StringSize = sizeof(pNodeNew->szDescription)/sizeof(TCHAR);
        StringCchCopy(pNodeNew->szDescription,StringSize,pNode->szDescription);
        //_tcscpy( pNodeNew->szDescription, pNode->szDescription );
    #endif // DEBUG
        AddNodeToFreeList(pNodeNew, fHighPri);

        pNode->dwSize = dwSize; // remember to resize old block
    }

#ifdef DEBUG
    // add description to block
    DEBUGCHK( _tcslen( pszMemDescription ) < CPHYSMEM_MAX_DEBUG_NODE_DESCRIPTION_LENGTH );
    DWORD StringSize = sizeof(pNode->szDescription)/sizeof(TCHAR);
    StringCchCopy(pNode->szDescription,StringSize,pszMemDescription);
    //_tcscpy( pNode->szDescription, pszMemDescription );
    // trash the memory before we return it to caller
    memset( PUCHAR( pNode->dwVirtAddr ), GARBAGE, pNode->dwSize );
#endif // DEBUG

    DEBUGMSG(ZONE_CPHYSMEM, (TEXT("CPhysMem AllocateMemory : PA = 0x%08X, VA = 0x%08X, Size = %d, Desc = %s\r\n"),
                          pNode->dwPhysAddr, pNode->dwVirtAddr, pNode->dwSize, pNode->szDescription ) );

    // mark this node used
    InsertNodeBefore(pNode, FirstNode(INUSELIST(fHighPri)));

    VALIDATE_HEAPS(fHighPri);

    LeaveCriticalSection(&m_csLock);

    DEBUGCHK( pNode->dwPhysAddr % CPHYSMEM_MEMORY_ALIGNMENT == 0 );
    *pVirtAddr = PUCHAR( pNode->dwVirtAddr );
    return TRUE;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void
CPhysMem::FreeMemory(
    IN const PUCHAR virtAddr,
    IN const ULONG physAddr,
    IN const DWORD dwFlags
    )
{
    // for now, only the following sets of flags should be passed in
    DEBUGCHK( dwFlags == 0 || // low priority, allow blocking
              dwFlags == CPHYSMEM_FLAG_NOBLOCK || // low priority, no blocking
              dwFlags == (CPHYSMEM_FLAG_HIGHPRIORITY | CPHYSMEM_FLAG_NOBLOCK) ); // high pri, no blocking

    BOOL fRemoved = FALSE;
    BOOL fHighPri = !!(dwFlags & CPHYSMEM_FLAG_HIGHPRIORITY);

    // caller of FreeMemory is capable of calling
    // PaToVa or VaToPa if they need to. Also,
    // we shouldn't be called to free NULL memory.
    DEBUGCHK( virtAddr != NULL && physAddr != 0 );
    DEBUGCHK( virtAddr == PaToVa( physAddr ) );
    DEBUGCHK( physAddr % CPHYSMEM_MEMORY_ALIGNMENT == 0 );

    EnterCriticalSection(&m_csLock);

#ifdef EHCI_PROBE
    RETAILMSG(RETAIL_LOG, (L"FM: find first node\r\n"));
#endif
    PMEMLIST pNode = FirstNode(INUSELIST(fHighPri));
#ifdef EHCI_PROBE
    RETAILMSG(RETAIL_LOG, (L"%x\r\n", pNode));
#endif

    DEBUGMSG( ZONE_CPHYSMEM && ZONE_VERBOSE, (TEXT("CPhysMem: Heap pri = %d before free VA = 0x%08x:\n"), fHighPri, virtAddr ) );
    VALIDATE_HEAPS(fHighPri);

    //
    // Walk the list looking for this block
    //
    while (!EndOfList(INUSELIST(fHighPri), pNode)) {

        if ((pNode->dwVirtAddr == (DWORD) virtAddr) &&
            (pNode->dwPhysAddr == (DWORD) physAddr)) {

        #ifdef DEBUG
            // trash this memory
            DEBUGCHK( pNode->dwSize > 0 ); // otherwise, why are we calling FreeMemory??
            memset( PUCHAR( pNode->dwVirtAddr ), GARBAGE, pNode->dwSize );

            DEBUGMSG(ZONE_CPHYSMEM,
                     (TEXT("CPhysMem FreeMemory : PA = 0x%08X, VA = 0x%08X, Size = %d, Desc = %s\r\n"),
                     pNode->dwPhysAddr, pNode->dwVirtAddr, pNode->dwSize, pNode->szDescription ));

            // change description
        DWORD StringSize = sizeof(pNode->szDescription)/sizeof(TCHAR);
        StringCchCopy(pNode->szDescription,StringSize,TEXT("Freed Memory"));


        //    _tcscpy( pNode->szDescription, TEXT("Freed Memory") );
        #endif // DEBUG
#ifdef EHCI_PROBE
            RETAILMSG(RETAIL_LOG, (L"Hit! Remove\r\n"));
#endif
            RemoveNode(pNode);
            AddNodeToFreeList(pNode, fHighPri);
            fRemoved = TRUE;
            break;
        }
        pNode = pNode->next;
    }

    if (fHighPri && !fRemoved) {
#ifdef EHCI_PROBE
        RETAILMSG(RETAIL_LOG, (L"H/N, need check\r\n"));
#endif
        LeaveCriticalSection(&m_csLock);

        //
        // Try removing from normal region.
        //
        DEBUGCHK( dwFlags == ( CPHYSMEM_FLAG_HIGHPRIORITY | CPHYSMEM_FLAG_NOBLOCK ) );
        FreeMemory( virtAddr,
                    physAddr,
                    CPHYSMEM_FLAG_NOBLOCK ); // dwFlags & ~CPHYSMEM_FLAG_HIGHPRIORITY
        return;
    }
    DEBUGCHK( fRemoved );

    DEBUGMSG( ZONE_CPHYSMEM && ZONE_VERBOSE, (TEXT("CPhysMem: Heap pri = %d after free VA = 0x%08x:\n"), fHighPri, virtAddr ) );
    VALIDATE_HEAPS(fHighPri);

    LeaveCriticalSection(&m_csLock);

    //
    // Signal everyone waiting for memory that some just became available.
    //
    if (m_fHasBlocked)
        PulseEvent(m_hFreeMemEvent);
}

#ifdef IRAM_PATCH
void CPhysMem::ShowMemInfo(void)
{
    PMEMLIST curEle;
    DWORD eleCnt, useSize, freeSize;

    RETAILMSG(1, (L"IRam object %x\r\n", this));
    //1. m_pNodeFreeListHead, number of element
    eleCnt = 0;
    curEle = FirstNode(m_pNodeFreeListHead);
    for (;;)
    {
        if (EndOfList(m_pNodeFreeListHead, curEle))
        {
            break;
        }
        curEle = curEle->next;
        eleCnt++;
    }
    RETAILMSG(1, (L"%d Free Ele\r\n", eleCnt));

    //2. Normal
    eleCnt = 0;
    useSize = 0;
    freeSize = m_dwNormalSize;
    curEle = FirstNode(m_pInUseListHead);
    for (;;)
    {
        if (EndOfList(m_pInUseListHead, curEle))
        {
            break;
        }
        useSize += curEle->dwSize;
        freeSize -= curEle->dwSize;
        curEle = curEle->next;
        eleCnt++;
    }
    RETAILMSG(1, (L"%d Normal node, used %x free %x\r\n", eleCnt, useSize, freeSize));

    //3. Special
    eleCnt = 0;
    useSize = 0;
    freeSize = m_dwHighPrioritySize;
    curEle = FirstNode(m_pHighPriorityInUseListHead);
    for (;;)
    {
        if (EndOfList(m_pHighPriorityInUseListHead, curEle))
        {
            break;
        }
        useSize += curEle->dwSize;
        freeSize -= curEle->dwSize;
        curEle = curEle->next;
        eleCnt++;
    }
    RETAILMSG(1, (L"%d HP Node, used %x free %x\r\n", eleCnt, useSize, freeSize));
}
#endif

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
BOOL
CPhysMem::ReleaseBlockedCalls()
{
    //
    // Signal everyone waiting for memory to check if they have been aborted.
    //
    if (m_fHasBlocked)
        PulseEvent(m_hFreeMemEvent);

    return(TRUE);
}

