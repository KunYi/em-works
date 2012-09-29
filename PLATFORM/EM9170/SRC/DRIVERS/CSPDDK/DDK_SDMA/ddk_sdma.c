//-----------------------------------------------------------------------------
//
// Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  ddk_sdma.c
//
//  This file contains the platform-dependent CSPDDK SDMA support.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <ceddk.h>
#pragma warning(pop)

#include "bsp.h"

//-----------------------------------------------------------------------------
// External Functions


//-----------------------------------------------------------------------------
// External Variables
extern DMA_ADAPTER_OBJECT g_DmaAdapter;
extern PSDMA_HOST_SHARED_REGION g_pHostSharedUA;
extern PSDMA_HOST_SHARED_REGION g_pHostSharedPA;
extern UINT32 g_HostSharedSize;


//-----------------------------------------------------------------------------
// Defines


//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables


//-----------------------------------------------------------------------------
// Local Variables


//-----------------------------------------------------------------------------
// Local Functions


//-----------------------------------------------------------------------------
//
//  Function:  BSPSdmaAllocShared
//
//  This function allocates and maps the host shared region.
//
//  Parameters:
//      phyAddr
//          [in] Physical address for host shared region.
//
//  Returns:
//      If successful, returns pointer to uncached virtual region where
//      host shared memory is allocated, otherwise returns NULL.
//
//-----------------------------------------------------------------------------
PSDMA_HOST_SHARED_REGION BSPSdmaAllocShared(PHYSICAL_ADDRESS phyAddr)
{
    PSDMA_HOST_SHARED_REGION pHostSharedUA = NULL;
    
    // If host shared region is allocated from IRAM
    if ((phyAddr.LowPart >= IMAGE_WINCE_IRAM_PA_START) && 
        (phyAddr.LowPart < (IMAGE_WINCE_IRAM_PA_START+IMAGE_WINCE_IRAM_SIZE)))
    {

        //RETAILMSG(TRUE, (_T("SDMA Memory Map (IRAM):  PA = 0x%x, size = %d bytes"), 
        //    phyAddr.LowPart, IMAGE_WINCE_DDKSDMA_IRAM_SIZE));

        // Map host shared physical address space to virtual address space
        pHostSharedUA = (PSDMA_HOST_SHARED_REGION) MmMapIoSpace(phyAddr,
            IMAGE_WINCE_DDKSDMA_IRAM_SIZE, FALSE);

        if (pHostSharedUA == NULL)
        {
            ERRORMSG(TRUE, (_T("MmMapIoSpace failed!\r\n")));
            goto cleanUp;
        }

        g_HostSharedSize = IMAGE_WINCE_DDKSDMA_IRAM_SIZE;

        // Hardware constraints require IRAM to be accessed with PAHB or
        // we can incur errors on subsequent accesses to DRAM.  Update
        // the MMU attributes to use backwards-compatible extended
        // small page format.  This format allows us to set the TEX bits
        // to mark the IRAM region for non-shared device (directs
        // access to PAHB)
        //
        // Extended Small Page Format
        // --------------------------
        // BITS[31:12] = ADDRESS (preserved)
        // BITS[11:9] = SBZ (write as zero)
        // BITS[8:6] = TEX  (010b used for non-shared device)
        // BITS[5:4] = AP (preserved)
        // BITS[3:2] = CB (00b used for non-shared device)
        // BITS[1:0] = must be 11b for extended small page
        //
        // VAL = (0x0 << 9) | (0x2 << 6) | (0x0 << 2) | (0x3 << 0) = 0x083
        // MASK = (0x7 << 9) | (0x7 << 6) | (0x3 << 2) | (0x3 << 0) = 0xFCF
        
        if (!VirtualSetAttributes(pHostSharedUA, IMAGE_WINCE_DDKSDMA_IRAM_SIZE, 
            0x083, 0xFCF, NULL))
        {
            ERRORMSG(TRUE, (_T("VirtualSetAttributes failed!\r\n")));
            goto cleanUp;
        }
    
        // Flush the TLB since we directly updated the page tables
        CacheRangeFlush(0, 0, CACHE_SYNC_FLUSH_TLB);
    }

    // Else the host shared region must be allocated from external RAM
    else
    {
        //RETAILMSG(TRUE, (_T("SDMA Memory Map (RAM):  PA = 0x%x, size = %d bytes"), 
        //    phyAddr.LowPart, IMAGE_WINCE_DDKSDMA_IRAM_SIZE));

        // Map host shared physical address space to virtual address space
        pHostSharedUA = (PSDMA_HOST_SHARED_REGION) MmMapIoSpace(phyAddr,
            IMAGE_WINCE_DDKSDMA_RAM_SIZE, FALSE);

        if (pHostSharedUA == NULL)
        {
            ERRORMSG(TRUE, (_T("MmMapIoSpace failed!\r\n")));
            goto cleanUp;
        }

        g_HostSharedSize = IMAGE_WINCE_DDKSDMA_RAM_SIZE;

    }


cleanUp:
    return pHostSharedUA;

}


//-----------------------------------------------------------------------------
//
//  Function:  BSPSdmaAllocChain
//
//  This function allocates a chain of buffer descriptors for
//  a virtual DMA channel from platform-specific storage.
//
//  Parameters:
//      chan
//          [in] Virtual channel returned by DDKSdmaOpenChan function.
//
//      numBufDesc
//          [in] Number of buffer descriptors to allocate for the chain.
//
//      pPhyAddr
//          [out] Points to physical address where chain is allocated.
//
//  Returns:
//      If successful, returns pointer to uncached virtual region 
//      where chain is allocated, otherwise returns NULL.
//
//-----------------------------------------------------------------------------
PVOID BSPSdmaAllocChain(UINT8 chan, UINT32 numBufDesc, PPHYSICAL_ADDRESS pPhyAddr)
{
    BOOL bStaticBufDesc = TRUE;
    PVOID pBufDesc = NULL;

    // Make sure the region is mapped
    if (g_pHostSharedPA == NULL)
        return NULL;
        
    // Check if number of buffer descriptors requested exceeds number
    // supported with static buffer descriptor storage
    if (numBufDesc > SDMA_STATIC_BUF_DESC)
    {
        bStaticBufDesc = FALSE;
    }
    
    // Make sure we can support this channel since limited static buffer
    // descriptor storage may not be sufficient to support all channels
    if (((UINT32) g_pHostSharedPA < IMAGE_BOOT_RAMDEV_RAM_PA_START) &&
        (((UINT32) &(g_pHostSharedPA->chanBufDesc[chan][0])) >
                (IMAGE_WINCE_DDKSDMA_IRAM_PA_START+IMAGE_WINCE_DDKSDMA_IRAM_SIZE)))
    {
            bStaticBufDesc = FALSE;
    }
    else if (((UINT32) &(g_pHostSharedPA->chanBufDesc[chan][0])) > 
            (IMAGE_WINCE_DDKSDMA_RAM_PA_START+IMAGE_WINCE_DDKSDMA_RAM_SIZE))
    {
            bStaticBufDesc = FALSE;
    }
    
    // If buffer descriptors can be allocated from static storage
    if (bStaticBufDesc)
    {
        pBufDesc = &(g_pHostSharedUA->chanBufDesc[chan-1][0]);
        pPhyAddr->HighPart = 0;
        pPhyAddr->LowPart = (UINT32) &(g_pHostSharedPA->chanBufDesc[chan-1][0]);
    }

    // Else attempt dynamic allocation
    else
    {
        if (g_pHostSharedUA->chanDesc[chan].bExtended)
        {
            pBufDesc = HalAllocateCommonBuffer(&g_DmaAdapter,
                numBufDesc*sizeof(SDMA_BUF_DESC_EXT), pPhyAddr, FALSE);
        }
        else
        {
            pBufDesc = HalAllocateCommonBuffer(&g_DmaAdapter,
                numBufDesc*sizeof(SDMA_BUF_DESC), pPhyAddr, FALSE);
        }
    }

    // Keep track of the buffer descriptor storage method
    g_pHostSharedUA->chanDesc[chan].bStaticBufDesc = bStaticBufDesc;
    
    return pBufDesc;

}


//-----------------------------------------------------------------------------
//
//  Function:  BSPSdmaFreeChain
//
//  This function frees a chain of buffer descriptors for
//  a virtual DMA channel from platform-specific storage.
//
//  Parameters:
//      chan
//          [in] Virtual channel returned by DDKSdmaOpenChan function.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID BSPSdmaFreeChain(UINT8 chan)
{
    PHYSICAL_ADDRESS phyAddr;
    
    // If chain does not reside in static buffer descriptor storage
    if (!(g_pHostSharedUA->chanDesc[chan].bStaticBufDesc))
    {
        phyAddr.QuadPart = g_pHostSharedUA->chanCtrlBlk[chan].baseBufDescPA;
    
        HalFreeCommonBuffer(&g_DmaAdapter, 0, phyAddr,
            g_pHostSharedUA->chanDesc[chan].pBaseBufDescUA, FALSE);
    }
}


//-----------------------------------------------------------------------------
//
//  Function:  BSPSdmaBufIntMem
//
//  This function determines if the DMA buffers associated with the specified
//  DMA request line will be located in internal memory.  This allows for
//  adjustments in the SDMA script used for transferring data and effects
//  the BSP power mangement techniques.
//
//  Parameters:
//      dmaReq
//          [in] Specifies the DMA request line.
//
//
//  Returns:  
//      Returns TRUE if the DMA buffer will be located in internal memory.
//      Otherwise, returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL BSPSdmaBufIntMem(DDK_DMA_REQ dmaReq)
{
    BOOL rc = FALSE;

    switch(dmaReq)
    {
    case DDK_DMA_REQ_SSI1_TX0:
    case DDK_DMA_REQ_SSI2_TX0:
    case DDK_DMA_REQ_SSI1_RX0:
    case DDK_DMA_REQ_SSI2_RX0:
#ifdef BSP_AUDIO_DMA_BUF_ADDR
#if (BSP_AUDIO_DMA_BUF_ADDR == IMAGE_WINCE_AUDIO_IRAM_PA_START)
        rc = TRUE;
#endif
#endif
        break;
    }

    return rc;
}

//-----------------------------------------------------------------------------
//
//  Function:  BSPSdmaGetM3BaseAddr
//
//  This function returns M3 base address which is needed by some scripts.
//
//  Parameters:
//      None.
//
//
//  Returns:  
//      M3 base address.
//
//-----------------------------------------------------------------------------
UINT32 BSPSdmaGetM3BaseAddr(void)
{
    return IMAGE_BOOT_RAMDEV_RAM_PA_START;
}

