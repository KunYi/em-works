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
//     cphysmem.hpp
// 
// Abstract:  
//     Definitions for physical memory manager.
//     
// Notes: 
// 
#ifndef __CPHYSMEM_HPP__
#define __CPHYSMEM_HPP__

typedef unsigned long FAR * LPULONG;

// All the memory that this CPhysMem class returns to callers
// will have its physical address divisible by CPHYSMEM_MEMORY_ALIGNMENT.
// This allows us to satisfy the HCD alignment requirements. For
// instance, in UHCI, TDs and QHs must be aligned on 16 byte boundaries.
//
// This number should be 1) A power of two and 2) a common multiple
// of any required alignment criteria of the HCD.
#define CPHYSMEM_MEMORY_ALIGNMENT DWORD(1)

//
// Hook these if you want to track local memory allocations.
//
#define CPhysMem_Alloc  LocalAlloc
#define CPhysMem_Free   LocalFree

#define MEMLIST_NEXT_PA(pnode)  (pnode->dwPhysAddr + pnode->dwSize)

typedef struct tMEMLIST   MEMLIST;
typedef struct tMEMLIST* PMEMLIST;
struct tMEMLIST
{
    DWORD       dwVirtAddr;
    DWORD       dwPhysAddr;
    DWORD       dwSize;
    PMEMLIST    next;
    PMEMLIST    prev;
};


#ifdef DEBUG
#define VALIDATE_HEAPS(fHighPri)  ValidateHeaps(fHighPri)
#else
#define VALIDATE_HEAPS(fHighPri)
#endif


//
// Flag defines for AllocateMemory / FreeMemory
//
#define CPHYSMEM_FLAG_HIGHPRIORITY  DWORD(1 << 0)  // Allocates from High Priority region first
#define CPHYSMEM_FLAG_NOBLOCK       DWORD(1 << 1)  // Call doesn't block if no memory available.

#ifdef DEBUG
    #define CPHYSMEM_BLOCK_FOR_MEM_INTERVAL    1000
    #define CPHYSMEM_DEBUG_MAXIMUM_BLOCK_TIME  5000
#else
    #define CPHYSMEM_BLOCK_FOR_MEM_INTERVAL INFINITE
#endif // DEBUG

class CPhysMem
{
public:
    CPhysMem(DWORD cbSize, DWORD cbHighPrioritySize, PUCHAR pVirtAddr, PUCHAR pPhysAddr);
    ~CPhysMem();
    void ReInit();

    BOOL
    AllocateMemory(
        IN const DWORD dwPassedInSize,
        OUT PUCHAR* const pVirtAddr,
        IN const DWORD dwFlags,
        IN const DWORD dwAlignSize);

    void
    FreeMemory(
        IN const PUCHAR virtAddr,
        IN const ULONG physAddr,
        IN const DWORD dwFlags );

    inline ULONG
    VaToPa(const PUCHAR virtAddr);

    inline PUCHAR
    PaToVa(ULONG physAddr);

    inline BOOL
    InittedOK()const { return m_fInitted; };

    const DWORD  m_cbTotal;
    const DWORD  m_cbHighPri;
    const PUCHAR m_pVirtBase;
    const PUCHAR m_pPhysBase;

private:
    BOOL AddNodeToInUseList(PMEMLIST pNode, BOOL fHighPri);
    BOOL AddNodeToFreeList(PMEMLIST pNode, BOOL fHighPri);
    
    BOOL DeleteNode(PMEMLIST pNode);
    
    PMEMLIST CreateNewNode(DWORD dwSize, DWORD dwVirtAddr, DWORD dwPhysAddr);
    
    PMEMLIST FindFreeBlock(DWORD dwSize, DWORD dwAlignSize, BOOL fHighPri);
    BOOL RemoveNodeFromList(PMEMLIST pNode, PMEMLIST* pListHead);
    BOOL FreeList(PMEMLIST *ppHead);
#ifdef DEBUG
    BOOL ValidateHeaps(BOOL fHighPri);
#endif
    operator=(const CPhysMem& /*rhs*/) { ASSERT(FALSE); }


    CRITICAL_SECTION m_csLock;
    BOOL        m_fInitted;
    BOOL        m_fPhysFromPlat;
    DWORD       m_PaVaConversion;
    DWORD       m_dwTotalPhysMemSize;
    PUCHAR      m_pPhysicalBufferAddr;

    PMEMLIST    m_pNodeFreeListHead;
    
//    DWORD       m_dwSpecialPA;
//    DWORD       m_dwSpecialVA;
//    BOOL        m_bSpecialTaken;
    
    DWORD       m_dwNormalPA;
    DWORD       m_dwNormalVA;
    DWORD       m_dwNormalSize;
    PMEMLIST    m_pFreeListHead;
    PMEMLIST    m_pInUseListHead;
    
    DWORD       m_dwHighPriorityPA;
    DWORD       m_dwHighPriorityVA;
    DWORD       m_dwHighPrioritySize;
    PMEMLIST    m_pHighPriorityInUseListHead;
    PMEMLIST    m_pHighPriorityFreeListHead;
};


#define FREELIST(fHighPri)  ((PMEMLIST)(fHighPri ? m_pHighPriorityFreeListHead : m_pFreeListHead))
#define INUSELIST(fHighPri) ((PMEMLIST)(fHighPri ? m_pHighPriorityInUseListHead : m_pInUseListHead))

inline ULONG
CPhysMem::VaToPa(const PUCHAR virtAddr)
{
    DEBUGCHK( virtAddr != NULL );
    return ULONG(ULONG(virtAddr) + m_PaVaConversion);
}

inline PUCHAR
CPhysMem::PaToVa(ULONG physAddr)
{
    DEBUGCHK( physAddr != 0 );
    return PUCHAR(physAddr - m_PaVaConversion);
}


//---------------------------------------------------------------------
//
// Linked list helper functions
//
#define IsListEmpty(_ListHead)          ((BOOL)((_ListHead)->next == _ListHead))
#define EndOfList(_ListHead, _Entry)    ((BOOL)(_ListHead == _Entry))

inline void
InitializeListHead(PMEMLIST _ListHead)
{
    _ListHead->next = _ListHead->prev = _ListHead;
}


inline PMEMLIST
FirstNode(
    PMEMLIST _ListHead
    ) 
{ 
    return(_ListHead->next); 
}


inline void
RemoveNode(
    PMEMLIST pNode
    )
{
    pNode->prev->next = pNode->next;
    pNode->next->prev = pNode->prev;
    pNode->next = NULL;
    pNode->prev = NULL;
}


inline void
InsertNodeBefore(
    PMEMLIST pNodeNew,
    PMEMLIST pNodeExisting
    )
{
    pNodeExisting->prev->next = pNodeNew;
    pNodeNew->prev = pNodeExisting->prev;
    pNodeNew->next = pNodeExisting;
    pNodeExisting->prev = pNodeNew;
}

#endif //__CPHYSMEM_HPP__

