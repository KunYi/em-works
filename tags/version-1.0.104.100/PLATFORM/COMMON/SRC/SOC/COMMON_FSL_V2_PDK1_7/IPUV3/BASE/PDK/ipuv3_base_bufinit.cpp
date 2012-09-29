//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  IpuBuffermanager.cpp
//
//  Implementation of IPU buffer managing methods
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <Devload.h>
#include <ceddk.h>
#pragma warning(pop)

#include "common_macros.h"
#include "ipuv3_base_include.h"

#include "IpuBuffer.h"
#include "Ipu_base.h"
#include "ipuv3_base_priv.h"

//------------------------------------------------------------------------------
// External Functions
extern "C" DWORD BSPGetVMemSizeFromRegistry(VOID);
extern "C" DWORD BSPGetIPUIRAMStartAddr(VOID);
extern "C" DWORD BSPGetIPUIRAMSize(VOID);
extern "C" BOOL  BSPGetReservedVMem(ULONG * pPhysicalMemAddr, ULONG * pVMemSize);


//------------------------------------------------------------------------------
// External Variables
extern  LPVOID          g_pVideoMemory;         // Virtual Address of the video memory
extern  ULONG           g_nVideoMemoryPhysical; // Physical Linear Access Window (LAW) address
extern  BufferHeap    *g_pVideoMemoryHeap;     // Base entry representing the main video memory
 
extern  LPVOID          g_pIRAM;         // Virtual Address of the iRAM
extern  ULONG           g_nIRAMPhysical; // Physical Linear Access Window (LAW) address of iRAM
extern  BufferHeap    *g_pIRAMHeap;     // Base entry representing the internal RAM

//------------------------------------------------------------------------------
// Defines

//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Global Variables

ULONG           g_nVideoMemorySize;     // Size in bytes of actual video RAM total
ULONG           g_nIRAMSize;     // Size in bytes of actual internal RAM total

//------------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
// Local Functions


//-----------------------------------------------------------------------------
//
// Function: IpuBufferManagerInit
//
// Initalize all memory heap which will be used in IPU module.
//
// Parameters:
//      bCachedWT
//          [in] Determine the cache mode of the memory
//
// Returns:
//      TRUE if succeed.
//
//-----------------------------------------------------------------------------
BOOL IpuBufferManagerInit(BOOL bCachedWT)
{
    PHYSICAL_ADDRESS phyAddr;

    // bCachedWT = BSP_VID_MEM_CACHE_WRITETHROUGH;
    // Allocate video memory
    /*
     * IMPORTANT: There is no dedicated video memory for i.MX series
     *            processor, therefore, we have to share from
     *            system memory.
     */
    // We no longer retrieve video memory size from the registry.
    // Size comes from constant in image_cfg.h
    if(BSPGetReservedVMem(&g_nVideoMemoryPhysical, &g_nVideoMemorySize)== TRUE)
    {
        phyAddr.QuadPart = g_nVideoMemoryPhysical;

        // Map reserved contiguous physical memory to set Linear
        // Address Window Physical Address
        g_pVideoMemory = MmMapIoSpace(phyAddr, g_nVideoMemorySize, FALSE);
        // Check if virtual mapping failed
        if (!g_pVideoMemory)
        {
            DEBUGMSG(ZONE_ERROR,
                     (TEXT("%s(): Videomemory MmMapIoSpace failed!\r\n"), __WFUNCTION__));
            return FALSE;
        }
        RETAILMSG(1,
                 (TEXT("%s(): Detected reserved video memory(%d bytes), will ignore registry setting!\r\n"), __WFUNCTION__, g_nVideoMemorySize));
            

    }
    else
    {
        g_nVideoMemorySize = BSPGetVMemSizeFromRegistry();

        // Map reserved contiguous physical memory to set Linear
        // Address Window Physical Address
        g_pVideoMemory = AllocPhysMem(g_nVideoMemorySize, PAGE_READWRITE, 0, 0, (ULONG *) &g_nVideoMemoryPhysical);

        // Check if virtual mapping failed
        if (!g_pVideoMemory)
        {
            DEBUGMSG(ZONE_ERROR,
                     (TEXT("%s(): AllocPhysMem failed!\r\n"), __WFUNCTION__));
            return FALSE;
        }
    }

    if (bCachedWT)
    {
        // Update the Framebuffer to be cached, write-through
        VirtualSetAttributes((PUCHAR)g_pVideoMemory, g_nVideoMemorySize, 0x8, 0xC, NULL);
    }
    else
    {
        // Update the Framebuffer to be non-cached, bufferable
        VirtualSetAttributes((PUCHAR)g_pVideoMemory, g_nVideoMemorySize, 0x4, 0xC, NULL);
    }
    
    // Clear the virtual memory
    memset((PUCHAR)g_pVideoMemory, 0, g_nVideoMemorySize);

    // Create video memory heap
    g_pVideoMemoryHeap = new BufferHeap(g_nVideoMemorySize, NULL, NULL, NULL);

    //============================ALLOCATE IRAM=================================
    phyAddr.QuadPart = BSPGetIPUIRAMStartAddr();

    g_nIRAMSize = BSPGetIPUIRAMSize();

    // Map reserved contiguous physical memory to set Linear
    // Address Window Physical Address
    g_pIRAM = MmMapIoSpace(phyAddr, g_nIRAMSize, FALSE);
    // Check if virtual mapping failed
    if (!g_pIRAM)
    {
        DEBUGMSG(ZONE_ERROR,
                 (TEXT("%s(): MmMapIoSpace failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }

    g_nIRAMPhysical = BSPGetIPUIRAMStartAddr();


    if (bCachedWT)
    {
        // Update the Framebuffer to be cached, write-through
        VirtualSetAttributes((PUCHAR)g_pIRAM, g_nIRAMSize, 0x8, 0xC, NULL);
    }
    else
    {
        // Update the Framebuffer to be non-cached, bufferable
        VirtualSetAttributes((PUCHAR)g_pIRAM, g_nIRAMSize, 0x4, 0xC, NULL);
    }

    // Clear the virtual memory
    memset((PUCHAR)g_pIRAM, 0, g_nIRAMSize);

    // Create video memory heap
    g_pIRAMHeap = new BufferHeap(g_nIRAMSize, NULL, NULL, NULL);
    return TRUE;
}

//-----------------------------------------------------------------------------
//
// Function: IpuBufferManagerDeinit
//
// Free all memory heap.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void IpuBufferManagerDeinit(void)
{
    if (g_pVideoMemoryHeap)
    {
        delete g_pVideoMemoryHeap;
        g_pVideoMemoryHeap = NULL;
    }

    if (g_pVideoMemory)
    {
        FreePhysMem(g_pVideoMemory);
        g_pVideoMemory=NULL;
        g_nVideoMemoryPhysical = 0;
    }

    if (g_pIRAMHeap)
    {
        delete g_pIRAMHeap;
        g_pIRAMHeap = NULL;
    }

    if (g_pIRAM)
    {
        MmUnmapIoSpace(g_pIRAM,g_nIRAMSize);
        g_pIRAM=NULL;
        g_nIRAMPhysical = 0;
    }
    return;
}


//-----------------------------------------------------------------------------
//
// Function: IPUGetVideoMemorySize
//
// Get the total video memory size.
//
// Parameters:
//      None.
//
// Returns:
//      Video memory size.
//
//-----------------------------------------------------------------------------
DWORD IPUGetVideoMemorySize()
{
    return g_nVideoMemorySize;
}


//-----------------------------------------------------------------------------
//
// Function: IPUGetVideoMemoryBase
//
// Get the video memory base.
//
// Parameters:
//      None.
//
// Returns:
//      Video memory base.
//
//-----------------------------------------------------------------------------
DWORD IPUGetVideoMemoryBase()
{
    return (DWORD)g_pVideoMemory;
}
