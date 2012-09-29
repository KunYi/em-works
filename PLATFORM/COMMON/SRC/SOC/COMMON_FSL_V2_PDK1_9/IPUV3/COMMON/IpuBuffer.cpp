//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  IpuBuffer.cpp
//
//  Implementation of IPU buffer managing methods
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

//------------------------------------------------------------------------------
// Types

//------------------------------------------------------------------------------
// Global Variables

LPVOID          g_pVideoMemory = NULL;         // Virtual Address of the video memory
ULONG           g_nVideoMemoryPhysical = NULL; // Physical Linear Access Window (LAW) address
BufferHeap    *g_pVideoMemoryHeap = NULL;     // Base entry representing the main video memory

LPVOID          g_pIRAM = NULL;         // Virtual Address of the iRAM
ULONG           g_nIRAMPhysical = NULL; // Physical Linear Access Window (LAW) address of iRAM
BufferHeap    *g_pIRAMHeap = NULL;     // Base entry representing the internal RAM

//------------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
// Local Functions

//-----------------------------------------------------------------------------
//
// Function: IpuBuffer
//
// Ipu Buffer class constructor.
//
// Parameters:
//      dwBufferSize
//          [in] The size of buffer want to be created, in bytes
//
//      MemType
//          [in] The memory type of buffer.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
IpuBuffer::IpuBuffer(DWORD dwBufferSize, MEM_TYPE MemType)
{
    m_Memory_type = MemType;
    m_dwBufSize = dwBufferSize;
    if(!g_pIRAMHeap)
    {
        DEBUGMSG(0, (TEXT("No glbal RAM Heap, won't continue intializing!\r\n ")));
        return;
    }
    if(m_Memory_type == MEM_TYPE_IRAM)
    {
        m_pIpuBufferHeap = g_pIRAMHeap->Alloc(dwBufferSize);
        if (m_pIpuBufferHeap)
        {
            m_pVirtAddr = (UINT32 *)((UINT32)g_pIRAM + m_pIpuBufferHeap->Address());
            m_pPhysAddr = (UINT32 *)(g_nIRAMPhysical + m_pIpuBufferHeap->Address());
        }
        else
        {
             DEBUGMSG(1, (TEXT("IRAM is used up. \
                    Try to allocate memory from video memory instead\r\n ")));
             m_Memory_type = MEM_TYPE_VIDEOMEMORY;
        }
    }
    if(m_Memory_type == MEM_TYPE_VIDEOMEMORY)
    {
        m_pIpuBufferHeap = g_pVideoMemoryHeap->Alloc(dwBufferSize);
        if (m_pIpuBufferHeap)
        {
            m_pVirtAddr = (UINT32 *)((UINT32)g_pVideoMemory + m_pIpuBufferHeap->Address());
            m_pPhysAddr = (UINT32 *)(g_nVideoMemoryPhysical + m_pIpuBufferHeap->Address());
        }
        else
        {
            m_pVirtAddr = 0;
            m_pPhysAddr = 0;
            ERRORMSG(1, (TEXT("VideoMemory is used up.\r\n ")));
        }
    }
}

//-----------------------------------------------------------------------------
//
// Function: ~IpuBuffer
//
// The destructor for Class.  Deletes buffes.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
IpuBuffer::~IpuBuffer(void)
{
    if(m_pIpuBufferHeap != NULL)
        m_pIpuBufferHeap->Free();
}

//-----------------------------------------------------------------------------
//
// Function: PhysAddr
//
// Get the physical address of buffer.
//
// Parameters:
//      None.
//
// Returns:
//      Physical address.
//
//-----------------------------------------------------------------------------
UINT32* IpuBuffer::PhysAddr(void)
{
    return m_pPhysAddr;
}

//-----------------------------------------------------------------------------
//
// Function: VirtAddr
//
// Get the virtual address of buffer.
//
// Parameters:
//      None.
//
// Returns:
//      Virtual address.
//
//-----------------------------------------------------------------------------
UINT32* IpuBuffer::VirtAddr(void)
{
    return m_pVirtAddr;
}

//-----------------------------------------------------------------------------
//
// Function: MemType
//
// Get the memory type of buffer.
//
// Parameters:
//      None.
//
// Returns:
//      The memory type of buffer.
//
//-----------------------------------------------------------------------------
MEM_TYPE IpuBuffer::MemType(void)
{
    return m_Memory_type;
}

//-----------------------------------------------------------------------------
//
// Function: BufferHeap
//
// Get the Heap of buffer.
//
// Parameters:
//      None.
//
// Returns:
//      The heap of buffer.
//
//-----------------------------------------------------------------------------
BufferHeap *IpuBuffer::IpuBufferHeap(void)
{
    return m_pIpuBufferHeap;
}
//-----------------------------------------------------------------------------
//
// Function: BufSize
//
// Get the size of buffer.
//
// Parameters:
//      None.
//
// Returns:
//      Buffer size.
//
//-----------------------------------------------------------------------------
DWORD IpuBuffer::BufSize(void)
{
    return m_dwBufSize;
}



