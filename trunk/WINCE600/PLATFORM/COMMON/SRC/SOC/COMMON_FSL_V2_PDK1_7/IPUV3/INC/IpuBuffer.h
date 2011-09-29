//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  IpuBuffer.h
//
//  Class definition for an object to manage multiple buffering
//  in the IDMAC of the IPU.
//
//------------------------------------------------------------------------------

#ifndef __IPUBUFFER_H__
#define __IPUBUFFER_H__

//------------------------------------------------------------------------------
// Defines


//------------------------------------------------------------------------------
// Types


typedef struct ipuBufferDataStruct
{
    UINT32 *pPhysAddr;
    UINT32 *pVirtAddr;
} ipuBufferData, *pIpuBufferData;

typedef enum
{
    MEM_TYPE_VIDEOMEMORY,
    MEM_TYPE_IRAM
}MEM_TYPE, *PMEM_TYPE;

//------------------------------------------------------------------------------
// Functions
//From surface heap in ddgpe.h
// Surface Heap (use instead of Node2D if you're dealing with multiple surface types
class BufferHeap
{
    BufferHeap      *m_pNext;
    BufferHeap      *m_pPrev;
    UINT32           m_pStart;
    DWORD            m_nSize;
    DWORD            m_fUsed;
public:
                    BufferHeap
                    (
                        DWORD size,
                        UINT32 base,
                        BufferHeap *pNext = (BufferHeap *)NULL,
                        BufferHeap *pPrev = (BufferHeap *)NULL
                    );
                    ~BufferHeap();
    BufferHeap *    Alloc( DWORD size );
    UINT32          Address() { return m_pStart; } // offset
    void            Free();
    DWORD           Available();
    DWORD           Size();    // returns size of this & subsequent nodes
    DWORD           NodeSize(){ return m_nSize; }
};


class IpuBuffer
{
    public:
        IpuBuffer(DWORD dwBufferSize, MEM_TYPE MemType);
        ~IpuBuffer();

        UINT32* PhysAddr();
        UINT32* VirtAddr();
        MEM_TYPE MemType();
        DWORD BufSize();
        BufferHeap* IpuBufferHeap();

    private:
        UINT32 *m_pPhysAddr;
        UINT32 *m_pVirtAddr;
        DWORD m_dwBufSize;
        MEM_TYPE m_Memory_type;
        BufferHeap *m_pIpuBufferHeap;

};



#endif  // __IPUBUFFER_H__
