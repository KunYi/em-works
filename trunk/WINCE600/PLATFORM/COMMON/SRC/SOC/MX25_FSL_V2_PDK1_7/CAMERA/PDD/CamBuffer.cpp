//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  CamBuffer.cpp
//
//  Implementation of CSI buffer managing methods
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <Devload.h>
#include <ceddk.h>
#include <NKIntr.h>
#include <winddi.h>
#include <ddgpe.h>
#pragma warning(pop)

#include "common_macros.h"

#include "CamBuffer.h"


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

//------------------------------------------------------------------------------
// Local Variables

//------------------------------------------------------------------------------
// Local Functions

//-----------------------------------------------------------------------------
//
// Function: CamBuffer
//
// CamBuffer class constructor.
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
CamBuffer::CamBuffer(DWORD dwBufferSize)
{
    UINT32 PhysAddr; 
    m_dwBufSize = dwBufferSize;

    // Allocate a buffer based on the size passed in.
    m_pVirtAddr = (UINT32*)AllocPhysMem(dwBufferSize, PAGE_EXECUTE_READWRITE, 0, 0, (unsigned long *)&PhysAddr);
    if (!m_pVirtAddr)
    {
        DEBUGMSG(1, (TEXT("CamBuffer : Failed while trying to allocate physical memory\r\n ")));
        m_pVirtAddr = NULL;
        m_pPhysAddr = NULL;
    }
    else
    {
        m_pPhysAddr = (UINT32*)PhysAddr;
    }
}

//-----------------------------------------------------------------------------
//
// Function: ~CamBuffer
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
CamBuffer::~CamBuffer(void)
{
    if(m_pVirtAddr)
    {
        if (!FreePhysMem(m_pVirtAddr)) 
        {
            DEBUGMSG(1, (TEXT("CamBuffer : Failed while freeing physcal memory\r\n")));
        }
        m_pVirtAddr = NULL;
        m_pPhysAddr = NULL;
    }
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
UINT32* CamBuffer::PhysAddr(void)
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
UINT32* CamBuffer::VirtAddr(void)
{
    return m_pVirtAddr;
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
DWORD CamBuffer::BufSize(void)
{
    return m_dwBufSize;
}



