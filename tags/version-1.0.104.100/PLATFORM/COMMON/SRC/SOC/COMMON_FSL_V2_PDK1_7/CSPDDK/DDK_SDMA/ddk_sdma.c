//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  ddk_sdma.c
//
//  This file contains a DDK interface for the SDMA module.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#pragma warning(pop)

#include "common_macros.h"
#include "common_sdma.h"
#include "common_ddk.h"
#include "common_ioctl.h"

//-----------------------------------------------------------------------------
// External Functions
extern UINT16 SdmaLoadRamScripts(void * pBuf);
extern BOOL SdmaSetChanDesc(DDK_DMA_REQ dmaReq, PSDMA_CHAN_DESC pChanDesc);
extern BOOL SdmaUpdateChanDesc(DDK_DMA_REQ dmaReq, PSDMA_CHAN_DESC pChanDesc);
extern BOOL SdmaSetChanContext(UINT8 chan, PSDMA_CHAN_DESC pChanDesc, 
    UINT32 waterMark, PSDMA_CHANNEL_CONTEXT pChanCtxt);

extern PSDMA_HOST_SHARED_REGION BSPSdmaAllocShared(PHYSICAL_ADDRESS phyAddr);
extern PVOID BSPSdmaAllocChain(UINT8 chan, UINT32 numBufDesc, 
    PPHYSICAL_ADDRESS phyAddr);
extern VOID BSPSdmaFreeChain(UINT8 chan);

//-----------------------------------------------------------------------------
// External Variables
extern const UINT32 g_pSdmaBaseRegPA;

//-----------------------------------------------------------------------------
// Defines
#define CHAN_MASK(chan) (1 << (chan))
#define CMD_SETCTX(chan) (((chan) << 3) | SDMA_CMD_C0_SETCTX)
// #define DDK_SDMA_VERBOSE


//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables
DMA_ADAPTER_OBJECT g_DmaAdapter = { sizeof(DMA_ADAPTER_OBJECT), 
                                    Internal, 0 };
PSDMA_HOST_SHARED_REGION g_pHostSharedUA = NULL;
PSDMA_HOST_SHARED_REGION g_pHostSharedPA = NULL;
UINT32 g_HostSharedSize = 0;

//-----------------------------------------------------------------------------
// Local Variables
static PCSP_SDMA_REGS g_pSDMA = NULL;
static HANDLE g_hSdmaMutex;
static HANDLE g_hAcrMutex;

//-----------------------------------------------------------------------------
// Local Functions
BOOL SdmaInit(void);
BOOL SdmaDeinit(void);
BOOL SdmaAlloc(void);
BOOL SdmaDealloc(void);
static BOOL SdmaWritePM(UINT32 srcAddrPA, UINT32 dstAddrPM, UINT16 count);
static BOOL SdmaWriteChanContext(UINT8 chan, UINT32 srcAddrPA);
static void SdmaSetChanOverride(UINT8 chan, BOOL bHostOvr,
    BOOL bEventOvr, BOOL bDspOvr);
static void SdmaSetChanPriority(UINT8 chan, UINT8 priority);
static BOOL SdmaGetChanStatus(UINT8 chan);
static UINT8 SdmaFindUnusedChan(UINT8 priority);
static void SdmaLockCtrlChan(void);
static void SdmaUnlockCtrlChan(void);
static BOOL SdmaValidChan(UINT8 chan);


//-----------------------------------------------------------------------------
//
//  Function:  DDKSdmaOpenChan
//
//  This function tries to find an available virtual SDMA channel that can
//  be used to support a memory-to-memory, peripheral-to-memory, or
//  memory-to-peripheral transfer.
//
//  Parameters:
//      dmaReq
//          [in] - Specifies the DMA request that will be bound to
//          a virtual channel.
//
//      priority
//          [in] - Priority assigned to the opened channel.
//
//  Returns:
//      Returns the virtual channel index if successful,
//      otherwise returns NULL.
//
//-----------------------------------------------------------------------------
UINT8 DDKSdmaOpenChan(DDK_DMA_REQ dmaReq, UINT8 priority)
{
    UINT8 chan;
    UINT32 i, dmaMask;

    // Try to find an available channel
    chan = SdmaFindUnusedChan(priority);    

    // If available channel was found
    if (chan == 0)
    {
        ERRORMSG(1, (_T("No available channel!\r\n")));
        goto cleanUp;
    }
    
    // Reset CCB
    g_pHostSharedUA->chanCtrlBlk[chan].curBufDescPA = 0;
    g_pHostSharedUA->chanCtrlBlk[chan].baseBufDescPA = 0;
    g_pHostSharedUA->chanCtrlBlk[chan].chanDesc = 0;
    g_pHostSharedUA->chanCtrlBlk[chan].chanStatus = 0;    

    // Reset channel descriptor fields for buffer descriptors
    g_pHostSharedUA->chanDesc[chan].pBaseBufDescUA = NULL;
    g_pHostSharedUA->chanDesc[chan].numBufDesc = 0;

    // Attempt to configure remaining channel descriptor fields based DMA request
    if (!SdmaSetChanDesc(dmaReq, &g_pHostSharedUA->chanDesc[chan]))
    {
        ERRORMSG(1, (_T("SdmaSetChanDesc failed!\r\n")));
        goto cleanUp;
    }
        
    // If the channel has an associated DMA request
    if (g_pHostSharedUA->chanDesc[chan].dmaMask)
    {
        // Search through the DMA requests specified for this channel
        for (i = 0, dmaMask = 0x1; i < SDMA_NUM_CHANNELS; i++, dmaMask <<= 1)
        {
            if (dmaMask & g_pHostSharedUA->chanDesc[chan].dmaMask)
            {
                // Bind DMA requests to virtual channel
                SETREG32(&g_pSDMA->CHNENBL[i], CHAN_MASK(chan));
            }
        }

        // Clear event override
        SdmaSetChanOverride(chan, FALSE, FALSE, TRUE);
    }
    else
    {
        // No DMAREQ for memory-to-memory, set event override
        SdmaSetChanOverride(chan, FALSE, TRUE, TRUE);
    }

    return chan;

cleanUp:
    DDKSdmaCloseChan(chan);
    return 0;
}


//-----------------------------------------------------------------------------
//
//  Function:  DDKSdmaUpdateSharedChan
//
//  This function allows a channel that has multiple DMA requests combined into
//  a shared DMA event to be reconfigured for one of the alternate DMA requests.
//
//  Parameters:
//      dmaReq
//          [in] - Specifies one of the possible DMA requests valid for
//                 a channel with a shared DMA event.  The DMA request
//                 must be one of the set of requests valid for the opened
//                 channel.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKSdmaUpdateSharedChan(UINT8 chan, DDK_DMA_REQ dmaReq)
{
    BOOL rc = FALSE;

    // Validate channel
    if (!SdmaValidChan(chan))
    {
        ERRORMSG(1, (_T("Channel validation failed!\r\n")));
        goto cleanUp;
    }

    rc = SdmaUpdateChanDesc(dmaReq, &g_pHostSharedUA->chanDesc[chan]);

cleanUp:
    return rc;
}


//-----------------------------------------------------------------------------
//
// Function:  DDKSdmaCloseChan
//
// This function closes a channel previously opened by the DDKSdmaOpenChan
// routine.
//
// Parameters:
//      chan
//          [in] - Virtual channel returned by DDKSdmaOpenChan function.
//
// Returns:
//      Returns TRUE if the close operation was successful, otherwise
//      returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKSdmaCloseChan(UINT8 chan)
{
    BOOL rc = FALSE;
    UINT32 i, dmaMask;

    // Validate channel
    if (!SdmaValidChan(chan))
    {
        ERRORMSG(1, (_T("Channel validation failed!\r\n")));
        goto cleanUp;
    }

    // If the channel has an associated DMA request
    if (g_pHostSharedUA->chanDesc[chan].dmaMask)
    {
        // Search through the DMA requests specified for this channel
        for (i = 0, dmaMask = 0x1; i < SDMA_NUM_CHANNELS; i++, dmaMask <<= 1)
        {
            if (dmaMask & g_pHostSharedUA->chanDesc[chan].dmaMask)
            {
                // Unbind DMA request from virtual channel
                CLRREG32(&g_pSDMA->CHNENBL[i], CHAN_MASK(chan));
            }
        }

        // Clear event override
        SdmaSetChanOverride(chan, FALSE, FALSE, TRUE);
    }

    // Make sure event override is cleared
    SdmaSetChanOverride(chan, FALSE, FALSE, TRUE);

    // Unmap the buffer descriptors
    DDKSdmaFreeChain(chan);
        
    // Reset CCB
    g_pHostSharedUA->chanCtrlBlk[chan].curBufDescPA = 0;
    g_pHostSharedUA->chanCtrlBlk[chan].baseBufDescPA = 0;
    g_pHostSharedUA->chanCtrlBlk[chan].chanDesc = 0;
    g_pHostSharedUA->chanCtrlBlk[chan].chanStatus = 0;    

    // Return channel back to pool of unused channels
    SdmaSetChanPriority(chan, SDMA_CHNPRI_CHNPRI_DISABLE);

    rc = TRUE;

cleanUp:
    return rc;
}


//-----------------------------------------------------------------------------
//
//  Function:  DDKSdmaAllocChain
//
//  This function allocates a chain of buffer descriptors for
//  a virtual DMA channel.  
//
//  Parameters:
//      chan
//          [in] - Virtual channel returned by DDKSdmaOpenChan function.
//
//      numBufDesc
//          [in] - Number of buffer descriptors to allocate for the chain.
//
//  Returns:
//      Returns TRUE if the chain allocation succeeded, otherwise returns 
//      FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKSdmaAllocChain(UINT8 chan, UINT32 numBufDesc)
{
    BOOL rc = FALSE;
    PVOID pBufDesc;
    PHYSICAL_ADDRESS phyAddr;
    
    // Validate channel
    if (!SdmaValidChan(chan))
    {
        ERRORMSG(1, (_T("Channel validation failed!\r\n")));
        goto cleanUp;
    }

    // Allocate storage for buffer descriptor chain
    pBufDesc = BSPSdmaAllocChain(chan, numBufDesc, &phyAddr);

    if (pBufDesc == NULL)
    {
        ERRORMSG(1, (_T("BSPSdmaAllocChain failed!\r\n")));
        goto cleanUp;
    }

    // Save chain information in CCB and channel descriptor
    g_pHostSharedUA->chanCtrlBlk[chan].baseBufDescPA = phyAddr.LowPart;
    g_pHostSharedUA->chanCtrlBlk[chan].curBufDescPA = phyAddr.LowPart;
    g_pHostSharedUA->chanDesc[chan].pBaseBufDescUA = pBufDesc;
    g_pHostSharedUA->chanDesc[chan].numBufDesc = numBufDesc;

    rc = TRUE;

cleanUp:
    return rc;
}


//-----------------------------------------------------------------------------
//
//  Function:  DDKSdmaFreeChain
//
//  This function frees a chain of buffer descriptors previously
//  allocated with DDKSdmaAllocChain.
//
//  Parameters:
//      chan
//          [in] - Virtual channel returned by DDKSdmaOpenChan function.
//
//  Returns:
//      Returns TRUE if the chain could be deallocated, otherwise
//      returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKSdmaFreeChain(UINT8 chan)
{
    BOOL rc = FALSE;

    // Validate channel parameter
    if (chan <= 0 || chan >= SDMA_NUM_CHANNELS)
    {
        ERRORMSG(1, (_T("Invalid channel!\r\n")));
        goto cleanUp;
    }

    // Check if descriptors have been allocated for this channel
    if (g_pHostSharedUA->chanDesc[chan].numBufDesc == 0)
    {
        // Nothing to unmap, return success
        rc = TRUE;
        goto cleanUp;
    }

    // Kill the channel
    if (!DDKSdmaStopChan(chan, TRUE))
    {
        ERRORMSG(1, (_T("Cannot kill channel!\r\n")));
        goto cleanUp;
    }

    // Deallocate storage for buffer descriptor chain
    if (g_pHostSharedUA->chanDesc[chan].pBaseBufDescUA)
    {
        BSPSdmaFreeChain(chan);
    }

    // Reset chain information in CCB
    g_pHostSharedUA->chanCtrlBlk[chan].baseBufDescPA = 0;
    g_pHostSharedUA->chanCtrlBlk[chan].curBufDescPA = 0;
    g_pHostSharedUA->chanDesc[chan].pBaseBufDescUA = NULL;
    g_pHostSharedUA->chanDesc[chan].numBufDesc = 0;

    rc = TRUE;

cleanUp:
    return rc;

}


//-----------------------------------------------------------------------------
//
//  Function:  DDKSdmaSetBufDesc
//
//  This function configures a buffer descriptor for a DMA transfer.
//
//  Parameters:
//      chan
//          [in] - Virtual channel returned by DDKSdmaOpenChan function.
//
//      index
//          [in] - Index of buffer descriptor within the chain to
//          be configured.
//
//      modeFlags
//          [in] - Specifies the buffer desciptor mode word flags that
//          control the "continue", "wrap", and "interrupt" settings.          
//
//      memAddr1PA
//          [in] - For memory-to-memory transfers, this parameter specifies 
//          the physical memory source address for the transfer.  For 
//          memory-to-peripheral transfers, this parameter specifies the 
//          physical memory source address for the transfer.  For 
//          peripheral-to-memory transfers, this parameter specifies the 
//          physical memory destination address for the transfer.  
//
//      memAddr2PA
//          [in] - Used only for memory-to-memory transfers to specify 
//          the physical memory destination address for the transfer.  
//          Ignored for memory-to-peripheral and peripheral-to-memory 
//          transfers.
//
//      dataWidth
//          [in] - Used only for memory-to-peripheral and peripheral-to-memory
//          transfers to specify the width of the data for the peripheral
//          transfer.  Ignored for memory-to-memory transfers.
//
//      numBytes
//          [in] - Specifies the size of the transfer in bytes.
//      
//  Returns:
//      Returns TRUE if the buffer descriptor was configured, otherwise
//      returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKSdmaSetBufDesc(UINT8 chan, UINT32 index, 
    UINT32 modeFlags, UINT32 memAddr1PA, UINT32 memAddr2PA, 
    DDK_DMA_ACCESS dataWidth, UINT16 numBytes)
{
    BOOL rc = FALSE;
    PSDMA_BUF_DESC pBufDesc;
    PSDMA_BUF_DESC_EXT pBufDescExt;

    // Validate channel
    if (!SdmaValidChan(chan))
    {
        ERRORMSG(1, (_T("Channel validation failed!\r\n")));
        goto cleanUp;
    }

    // Validate buffer descriptor index
    if (index >= g_pHostSharedUA->chanDesc[chan].numBufDesc)
    {
        ERRORMSG(1, (_T("Invalid BD index!\r\n")));
        goto cleanUp;
    }
    
    // If channel uses extended buffer descriptors
    if (g_pHostSharedUA->chanDesc[chan].bExtended)
    {
        pBufDescExt = (PSDMA_BUF_DESC_EXT) 
            g_pHostSharedUA->chanDesc[chan].pBaseBufDescUA;
    
        pBufDescExt[index].mode =
            CSP_BITFVAL(SDMA_MODE_COMMAND, dataWidth) |
            CSP_BITFVAL(SDMA_MODE_EXT, SDMA_MODE_EXT_USED) |
            CSP_BITFVAL(SDMA_MODE_ERROR, SDMA_MODE_ERROR_NOTFOUND) |
            modeFlags |
            CSP_BITFVAL(SDMA_MODE_DONE, SDMA_MODE_DONE_NOTREADY) |
            CSP_BITFVAL(SDMA_MODE_COUNT, numBytes);
     
        pBufDescExt[index].srcAddrPA = memAddr1PA;
        pBufDescExt[index].destAddrPA = memAddr2PA;
    }

    // Else channel uses non-extended buffer descriptors
    else
    {
        pBufDesc = (PSDMA_BUF_DESC) 
            g_pHostSharedUA->chanDesc[chan].pBaseBufDescUA;
    
        pBufDesc[index].mode =
            CSP_BITFVAL(SDMA_MODE_COMMAND, dataWidth) |
            CSP_BITFVAL(SDMA_MODE_EXT, SDMA_MODE_EXT_UNUSED) |
            CSP_BITFVAL(SDMA_MODE_ERROR, SDMA_MODE_ERROR_NOTFOUND) |
            modeFlags |
            CSP_BITFVAL(SDMA_MODE_DONE, SDMA_MODE_DONE_NOTREADY) |
            CSP_BITFVAL(SDMA_MODE_COUNT, numBytes);
     
        pBufDesc[index].memAddrPA = memAddr1PA;
    }
    
    rc = TRUE;

cleanUp:
    return rc;

}


//-----------------------------------------------------------------------------
//
//  Function:  DDKSdmaGetBufDescStatus
//
//  This function retrieves the status of the "done" and "error" bits from
//  all of the buffer descriptors of a chain.
//
//  Parameters:
//      chan
//          [in] - Virtual channel returned by DDKSdmaOpenChan function.
//
//      index
//          [in] - Index of buffer descriptor.
//
//      pStatus
//          [out] - Points to an array that will be filled with the status
//          of each buffer descriptor in the chain.
//
//  Returns:
//      Returns TRUE if the buffer descriptor status was retrieved 
//      successfully, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKSdmaGetBufDescStatus(UINT8 chan, UINT32 index, 
    UINT32 *pStatus)
{
    BOOL rc = FALSE;
    PSDMA_BUF_DESC pBufDesc;
    PSDMA_BUF_DESC_EXT pBufDescExt;

    // Validate channel
    if (!SdmaValidChan(chan))
    {
        ERRORMSG(1, (_T("Channel validation failed!\r\n")));
        goto cleanUp;
    }

    // Validate buffer descriptor index
    if (index >= g_pHostSharedUA->chanDesc[chan].numBufDesc)
    {
        ERRORMSG(1, (_T("Invalid BD index!\r\n")));
        goto cleanUp;
    }

    // If channel uses extended buffer descriptors
    if (g_pHostSharedUA->chanDesc[chan].bExtended)
    {
        // Get reference to buffer descriptor
        pBufDescExt = (PSDMA_BUF_DESC_EXT) 
            g_pHostSharedUA->chanDesc[chan].pBaseBufDescUA;
    
        // Get mode word from buffer descriptor
        *pStatus = pBufDescExt[index].mode;
    
    }

    // Else channel uses non-extended buffer descriptors
    else
    {
        // Get reference to buffer descriptor
        pBufDesc = (PSDMA_BUF_DESC) 
            g_pHostSharedUA->chanDesc[chan].pBaseBufDescUA;
    
        // Get mode word from buffer descriptor
        *pStatus = pBufDesc[index].mode;
    
    }

#if 0
    // Isolate status bits
    *pStatus &= (CSP_BITFMASK(SDMA_MODE_DONE) | CSP_BITFMASK(SDMA_MODE_ERROR));
#endif

    rc = TRUE;

cleanUp:
    return rc;

}


//-----------------------------------------------------------------------------
//
//  Function:  DDKSdmaGetChainStatus
//
//  This function retrieves the status of the "done" and "error" bits from
//  the specified buffer descriptor.
//
//  Parameters:
//      chan
//          [in] - Virtual channel returned by DDKSdmaOpenChan function.
//
//      index
//          [in] - Index of buffer descriptor.
//
//      pStatus
//          [out] - Retrieved status of the buffer descriptor.
//
//  Returns:
//      Returns TRUE if the buffer descriptor status was retrieved 
//      successfully, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKSdmaGetChainStatus(UINT8 chan, UINT32 *pStatus)
{
    BOOL rc = FALSE;
    UINT32 index;
    PSDMA_BUF_DESC pBufDesc;
    PSDMA_BUF_DESC_EXT pBufDescExt;

    // Validate channel
    if (!SdmaValidChan(chan))
    {
        ERRORMSG(1, (_T("Channel validation failed!\r\n")));
        goto cleanUp;
    }

    // If channel uses extended buffer descriptors
    if (g_pHostSharedUA->chanDesc[chan].bExtended)
    {
        // Get reference to buffer descriptor
        pBufDescExt = (PSDMA_BUF_DESC_EXT) 
            g_pHostSharedUA->chanDesc[chan].pBaseBufDescUA;
    
        // For each buffer descriptor in the chain
        for (index = 0; index < g_pHostSharedUA->chanDesc[chan].numBufDesc; 
            index++)
        {        
#if 0            
            // Get mode word from buffer descriptor and isolate status bits
            pStatus[index] = pBufDescExt[index].mode & 
                (CSP_BITFMASK(SDMA_MODE_DONE) | CSP_BITFMASK(SDMA_MODE_ERROR));
#endif
            
            // Get mode word from buffer descriptor and isolate status bits
            pStatus[index] = pBufDescExt[index].mode;
        }
    
    }

    // Else channel uses non-extended buffer descriptors
    else
    {
        // Get reference to buffer descriptor
        pBufDesc = (PSDMA_BUF_DESC) 
            g_pHostSharedUA->chanDesc[chan].pBaseBufDescUA;
    
        // For each buffer descriptor in the chain
        for (index = 0; index < g_pHostSharedUA->chanDesc[chan].numBufDesc; 
            index++)
        {        
#if 0            
            // Get mode word from buffer descriptor and isolate status bits
            pStatus[index] = pBufDesc[index].mode & 
                (CSP_BITFMASK(SDMA_MODE_DONE) | CSP_BITFMASK(SDMA_MODE_ERROR));
#endif            
            // Get mode word from buffer descriptor and isolate status bits
            pStatus[index] = pBufDesc[index].mode;
        }    
    }

    rc = TRUE;

cleanUp:
    return rc;

}


//-----------------------------------------------------------------------------
//
//  Function:  DDKSdmaClearBufDescStatus
//
//  This function clears the status of the "done" and "error" bits within
//  the specified buffer descriptor.
//
//  Parameters:
//      chan
//          [in] - Virtual channel returned by DDKSdmaOpenChan function.
//
//      index
//          [in] - Index of buffer descriptor.
//
//  Returns:
//      Returns TRUE if the buffer descriptor status was cleared 
//      successfully, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKSdmaClearBufDescStatus(UINT8 chan, UINT32 index)
{
    BOOL rc = FALSE;
    PSDMA_BUF_DESC pBufDesc;
    PSDMA_BUF_DESC_EXT pBufDescExt;

    // Validate channel
    if (!SdmaValidChan(chan))
    {
        ERRORMSG(1, (_T("Channel validation failed!\r\n")));
        goto cleanUp;
    }

    // Validate buffer descriptor index
    if (index >= g_pHostSharedUA->chanDesc[chan].numBufDesc)
    {
        ERRORMSG(1, (_T("Invalid BD index!\r\n")));
        goto cleanUp;
    }

    // If channel uses extended buffer descriptors
    if (g_pHostSharedUA->chanDesc[chan].bExtended)
    {
        // Get reference to buffer descriptor
        pBufDescExt = (PSDMA_BUF_DESC_EXT) 
            g_pHostSharedUA->chanDesc[chan].pBaseBufDescUA;
        
        // Clear status bits
        CSP_BITFINS(pBufDescExt[index].mode, SDMA_MODE_DONE, 
            SDMA_MODE_DONE_NOTREADY);
        CSP_BITFINS(pBufDescExt[index].mode, SDMA_MODE_ERROR, 
            SDMA_MODE_ERROR_NOTFOUND);
    }

    // Else channel uses non-extended buffer descriptors
    else
    {
        // Get reference to buffer descriptor
        pBufDesc = (PSDMA_BUF_DESC) 
            g_pHostSharedUA->chanDesc[chan].pBaseBufDescUA;
        
        // Clear status bits
        CSP_BITFINS(pBufDesc[index].mode, SDMA_MODE_DONE, 
            SDMA_MODE_DONE_NOTREADY);
        CSP_BITFINS(pBufDesc[index].mode, SDMA_MODE_ERROR, 
            SDMA_MODE_ERROR_NOTFOUND);
    }   

    rc = TRUE;

cleanUp:
    return rc;

}


//-----------------------------------------------------------------------------
//
//  Function:  DDKSdmaClearChainStatus
//
//  This function clears the status of the "done" and "error" bits within
//  all of the buffer descriptors of a chain.
//
//  Parameters:
//      chan
//          [in] - Virtual channel returned by DDKSdmaOpenChan function.
//
//  Returns:
//      Returns TRUE if the buffer descriptor status for the chain was cleared 
//      successfully, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKSdmaClearChainStatus(UINT8 chan)
{
    BOOL rc = FALSE;
    UINT32 index;
    PSDMA_BUF_DESC pBufDesc;
    PSDMA_BUF_DESC_EXT pBufDescExt;

    // Validate channel
    if (!SdmaValidChan(chan))
    {
        ERRORMSG(1, (_T("Channel validation failed!\r\n")));
        goto cleanUp;
    }

    // If channel uses extended buffer descriptors
    if (g_pHostSharedUA->chanDesc[chan].bExtended)
    {
        // Get reference to buffer descriptor
        pBufDescExt = (PSDMA_BUF_DESC_EXT) 
            g_pHostSharedUA->chanDesc[chan].pBaseBufDescUA;
        
        for (index = 0; index < g_pHostSharedUA->chanDesc[chan].numBufDesc; 
            index++)
        {        
            // Clear status bits
            CSP_BITFINS(pBufDescExt[index].mode, SDMA_MODE_DONE, 
                SDMA_MODE_DONE_NOTREADY);
            CSP_BITFINS(pBufDescExt[index].mode, SDMA_MODE_ERROR, 
                SDMA_MODE_ERROR_NOTFOUND);
        }
    }

    // Else channel uses non-extended buffer descriptors
    else
    {
        // Get reference to buffer descriptor
        pBufDesc = (PSDMA_BUF_DESC) 
            g_pHostSharedUA->chanDesc[chan].pBaseBufDescUA;
        
        for (index = 0; index < g_pHostSharedUA->chanDesc[chan].numBufDesc; 
            index++)
        {        
            // Clear status bits
            CSP_BITFINS(pBufDesc[index].mode, SDMA_MODE_DONE, 
                SDMA_MODE_DONE_NOTREADY);
            CSP_BITFINS(pBufDesc[index].mode, SDMA_MODE_ERROR, 
                SDMA_MODE_ERROR_NOTFOUND);
        }
    }   

    rc = TRUE;

cleanUp:
    return rc;

}


//-----------------------------------------------------------------------------
//
//  Function:  DDKSdmaInitChain
//
//  This function initializes a buffer descriptor chain.
//
//  Parameters:
//      chan
//          [in] - Virtual channel returned by DDKSdmaOpenChan function.
//
//      waterMark
//          [in] - Specifies the watermark level used by the peripheral
//          to generate a DMA request.  This parameter tells the
//          DMA how many transfers to complete for each assertion of 
//          the DMA request.  Ignored for memory-to-memory transfers.
//
//  Returns:
//      Returns TRUE if the chain could be initialized, otherwise
//      returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKSdmaInitChain(UINT8 chan, UINT32 waterMark)
{
    BOOL rc = FALSE;
    
    // Clear the stats for all buffer descriptors
    if (!DDKSdmaClearChainStatus(chan))
    {
        ERRORMSG(1, (_T("Chain status could not be cleared!\r\n")));
        goto cleanUp;
    }

    // Request lock of control channel
    SdmaLockCtrlChan();
    
    // Make sure channel is not running before we update it
    if (!DDKSdmaStopChan(chan, TRUE))
    {
        ERRORMSG(1, (_T("Cannot kill channel!\r\n")));
        goto cleanUp;
    }

    // Initialize context for the channel
    if (!SdmaSetChanContext(chan, &g_pHostSharedUA->chanDesc[chan], waterMark,
        &g_pHostSharedUA->chanCtxt))
    {
        ERRORMSG(1, (_T("Cannot initialize channel context!\r\n")));
        goto cleanUp;
    }

    // Write out the channel context
    if (!SdmaWriteChanContext(chan, (UINT32) &g_pHostSharedPA->chanCtxt))
    {
        ERRORMSG(1, (_T("Cannot load channel context!\r\n")));
        goto cleanUp;
    }

    // Set physical address of current buffer descriptor
    g_pHostSharedUA->chanCtrlBlk[chan].curBufDescPA =
        g_pHostSharedUA->chanCtrlBlk[chan].baseBufDescPA;

    rc = TRUE;

cleanUp:
    // Unlock control channel
    SdmaUnlockCtrlChan();
    return rc;

}


//-----------------------------------------------------------------------------
//
// Function:  DDKSdmaStartChan
//
// This function starts the specified channel.
//
// Parameters:
//      chan
//          [in] - Virtual channel to start.
//
// Returns:
//      Returns TRUE if the start request is successful, else returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKSdmaStartChan(UINT8 chan)
{
    BOOL rc = FALSE;

    // Validate channel parameter
    if (chan >= SDMA_NUM_CHANNELS)
    {
        ERRORMSG(1, (_T("Invalid channel!\r\n")));
        goto cleanUp;
    }

    // Check if channel has been previously opened
    if (INREG32(&g_pSDMA->CHNPRI[chan]) == SDMA_CHNPRI_CHNPRI_DISABLE)
    {
        ERRORMSG(1, (_T("Channel not opened!\r\n")));
        goto cleanUp;
    }

    // Synchronize with DVFC since bus scaling may be in
    // progress.  During bus scaling, the DVFC driver
    // must wait for all channels to idle before
    // updating the SDMA ACR bit
    WaitForSingleObject(g_hAcrMutex, INFINITE);

    // If HSTART bit is not already set for this channel
    if (!(INREG32(&g_pSDMA->HSTART) & CHAN_MASK(chan)))
    {
#ifdef DDK_SDMA_VERBOSE
        // Dump out CCB for channel being started
        RETAILMSG(TRUE, (_T("CCB[%d]:\r\n"), chan));
        RETAILMSG(TRUE, (_T("curBufDescPA = 0x%x\r\n"), g_pHostSharedUA->chanCtrlBlk[chan].curBufDescPA));
        RETAILMSG(TRUE, (_T("baseBufDescPA = 0x%x\r\n"), g_pHostSharedUA->chanCtrlBlk[chan].baseBufDescPA));
        RETAILMSG(TRUE, (_T("baseBufDescUA = 0x%x\r\n"), g_pHostSharedUA->chanDesc[chan].pBaseBufDescUA));
        
        // Dump out snapshot of important SDMA registers
        RETAILMSG(TRUE, (_T("MC0PTR = 0x%x\r\n"), INREG32(&g_pSDMA->MC0PTR)));
        RETAILMSG(TRUE, (_T("INTR = 0x%x\r\n"), INREG32(&g_pSDMA->INTR)));
        RETAILMSG(TRUE, (_T("STOP_STAT = 0x%x\r\n"), INREG32(&g_pSDMA->STOP_STAT)));
        RETAILMSG(TRUE, (_T("HSTART = 0x%x\r\n"), INREG32(&g_pSDMA->HSTART)));
        RETAILMSG(TRUE, (_T("EVTOVR = 0x%x\r\n"), INREG32(&g_pSDMA->EVTOVR)));
        RETAILMSG(TRUE, (_T("HOSTOVR = 0x%x\r\n"), INREG32(&g_pSDMA->HOSTOVR)));
        RETAILMSG(TRUE, (_T("EVTPEND = 0x%x\r\n"), INREG32(&g_pSDMA->EVTPEND)));
        RETAILMSG(TRUE, (_T("EVTERR = 0x%x\r\n"), INREG32(&g_pSDMA->EVTERR)));
        RETAILMSG(TRUE, (_T("PSW = 0x%x\r\n"), INREG32(&g_pSDMA->PSW)));
        RETAILMSG(TRUE, (_T("CHN0ADDR = 0x%x\r\n"), INREG32(&g_pSDMA->CHN0ADDR)));
        RETAILMSG(TRUE, (_T("CHNENBL[0] = 0x%x\r\n"), INREG32(&g_pSDMA->CHNENBL[0])));
        RETAILMSG(TRUE, (_T("CHNENBL[1] = 0x%x\r\n"), INREG32(&g_pSDMA->CHNENBL[1])));
        RETAILMSG(TRUE, (_T("CHNENBL[2] = 0x%x\r\n"), INREG32(&g_pSDMA->CHNENBL[2])));
        RETAILMSG(TRUE, (_T("CHNENBL[3] = 0x%x\r\n"), INREG32(&g_pSDMA->CHNENBL[3])));
        RETAILMSG(TRUE, (_T("CHNPRI[0] = 0x%x\r\n"), INREG32(&g_pSDMA->CHNPRI[0])));
        RETAILMSG(TRUE, (_T("CHNPRI[1] = 0x%x\r\n"), INREG32(&g_pSDMA->CHNPRI[1])));
        RETAILMSG(TRUE, (_T("CHNPRI[2] = 0x%x\r\n"), INREG32(&g_pSDMA->CHNPRI[2])));
        RETAILMSG(TRUE, (_T("CHNPRI[3] = 0x%x\r\n"), INREG32(&g_pSDMA->CHNPRI[3])));
#endif

        // Start request can be granted (HSTART ignores write of zero into bits)
        OUTREG32(&g_pSDMA->HSTART, CHAN_MASK(chan));
    }

    ReleaseMutex(g_hAcrMutex);

    rc = TRUE;

cleanUp:
    return rc;
}


//-----------------------------------------------------------------------------
//
//  Function:  DDKSdmaStopChan
//
//  This function stops the specified channel.
//
//  Parameters:
//      chan
//          [in] - Virtual channel to stop.
//
//      bKill
//          [in] - Set TRUE to terminate the channel if it is actively
//          running.
//
//  Returns:
//      Returns TRUE if the stop request is successful, else returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKSdmaStopChan(UINT8 chan, BOOL bKill)
{
    BOOL rc = FALSE;

    // Validate channel parameter
    if (chan >= SDMA_NUM_CHANNELS)
    {
        ERRORMSG(1, (_T("SdmaStopChan:  Invalid channel\r\n")));
        goto cleanUp;
    }

    // Synchronize with DVFC since bus scaling may be in
    // progress.  During bus scaling, the DVFC driver
    // must wait for all channels to idle before
    // updating the SDMA ACR bit
    WaitForSingleObject(g_hAcrMutex, INFINITE);

    // Clear HE and HSTART bits using STOP_STAT register
    OUTREG32(&g_pSDMA->STOP_STAT, CHAN_MASK(chan));

    ReleaseMutex(g_hAcrMutex);

    // Kill running channel if requested
    if (bKill)
    {
        // Forcing a reschedule to kill the channel is
        // not recommended.  For now we just ignore the
        // kill request and let the channel terminate
        // normally.
    }

    rc = TRUE;

cleanUp:
    return rc;
}


//-----------------------------------------------------------------------------
//
//  Function:  SdmaInit
//
//  This function initializes the SDMA.  Called by the Device Manager to
//  initialize a device.
//
//  Parameters:
//      None.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL SdmaInit(void)
{
    BOOL rc = FALSE;
    UINT8 dmaEvent;
    UINT16 count;
    DMA_ADAPTER_OBJECT Adapter;
    PBYTE pRamImageUA;
    PHYSICAL_ADDRESS pRamImagePA;
        
    // Zero out shared region
    memset(g_pHostSharedUA, 0, g_HostSharedSize);

    // Configure chan 0 control block (single buffer descriptor reused for all
    // chan 0 commands)
    g_pHostSharedUA->chanCtrlBlk[0].baseBufDescPA =
        (UINT32) &g_pHostSharedPA->chan0BufDesc;
    g_pHostSharedUA->chanCtrlBlk[0].curBufDescPA =
        (UINT32) &g_pHostSharedPA->chan0BufDesc;

#ifdef SDMA_CONTEXT_WITH_SCRATCH
    // Configure context size to include scratch memory
    INSREG32BF(&g_pSDMA->CHN0ADDR, SDMA_CHN0ADDR_SMSZ,
        SDMA_CHN0ADDR_SMSZ_32WORDCNTX);
#endif

    // Clear channel enable RAM
    for (dmaEvent = 0; dmaEvent < SDMA_NUM_CHANNELS; dmaEvent++)
    {
        OUTREG32(&g_pSDMA->CHNENBL[dmaEvent], 0);
    }

    // Set event override
    SdmaSetChanOverride(0, FALSE, TRUE, TRUE);
        
    // Channel 0 operations will use interlocked updates of the priority
    // register to safely share channel 0 resources.  Signify channel 0
    // availability with SDMA_CHNPRI_CHNPRI_DISABLE priority setting
    SdmaSetChanPriority(0, SDMA_CHNPRI_CHNPRI_DISABLE);

    // Allocate a block of virtual memory (physically contiguous) for the
    // DMA RAM image.
    //
    memset(&Adapter, 0, sizeof(DMA_ADAPTER_OBJECT));
    Adapter.InterfaceType = Internal;
    Adapter.ObjectSize    = sizeof(DMA_ADAPTER_OBJECT);

    pRamImageUA = (PBYTE) HalAllocateCommonBuffer(&Adapter, SDMA_RAM_SIZE,
        &pRamImagePA, FALSE);

    // Copy SDMA RAM image into shared buffer
    count = SdmaLoadRamScripts(pRamImageUA);
    
    // Load DMA scripts into RAM
    rc = SdmaWritePM(pRamImagePA.LowPart, SDMA_PM_CODE_START, count);

    // Free the block allocated for the DMA image
    HalFreeCommonBuffer(&Adapter, SDMA_RAM_SIZE, pRamImagePA, (PVOID)(pRamImageUA), FALSE);

    return rc;
}


//-----------------------------------------------------------------------------
//
// Function:  SdmaDeinit
//
// This function deinitializes the SDMA.  Called by the Device Manager to
// de-initialize a device.
//
// Parameters:
//      None.
//
// Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
BOOL SdmaDeinit(void)
{
    BOOL rc = TRUE;
    UINT8 chan;

    // Close all channels
    for (chan = 1; chan < SDMA_NUM_CHANNELS; chan++)
    {
        DDKSdmaCloseChan(chan);
    }
        
    // Deallocate SDMA driver data structures
    SdmaDealloc();

    return rc;
}


//-----------------------------------------------------------------------------
//
// Function:  SdmaAlloc
//
// This function allocates the data structures required for interaction
// with the SDMA hardware.
//
// Parameters:
//      None.
//
// Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL SdmaAlloc(void)
{
    BOOL rc = FALSE;
    PHYSICAL_ADDRESS phyAddr;

    if (g_pSDMA == NULL)
    {
        phyAddr.QuadPart = g_pSdmaBaseRegPA;

        // Map peripheral physical address to virtual address
        g_pSDMA = (PCSP_SDMA_REGS) MmMapIoSpace(phyAddr, sizeof(CSP_SDMA_REGS),
            FALSE);

        // Check if virtual mapping failed
        if (g_pSDMA == NULL)
        {
            ERRORMSG(1, (_T("MmMapIoSpace failed!\r\n")));
            goto cleanUp;
        }
    }

    if (g_pHostSharedUA == NULL)
    {
        phyAddr.QuadPart = INREG32(&g_pSDMA->MC0PTR);
        
        // If we have not yet allocated host shared region
        if (phyAddr.LowPart == 0)
        {
            ERRORMSG(1, (_T("SDMA MC0PTR not initialized!\r\n")));
            goto cleanUp;
        }

        // Allocate and map host shared region
        g_pHostSharedUA = BSPSdmaAllocShared(phyAddr);

        // Check if virtual mapping failed
        if (g_pHostSharedUA == NULL)
        {
            ERRORMSG(1, (_T("MmMapIoSpace failed!\r\n")));
            goto cleanUp;
        }

        // Save physical address for use by other DLL functions
        g_pHostSharedPA = (PSDMA_HOST_SHARED_REGION) phyAddr.LowPart;
    }

    if (g_hSdmaMutex == NULL)
    {
        g_hSdmaMutex = CreateMutex(NULL, FALSE, L"MUTEX_SDMA");

        if (g_hSdmaMutex == NULL)
        {
            ERRORMSG(1, (_T("CreateMutex failed!\r\n")));
            goto cleanUp;
        }
    }

    // Create mutex for supporting AHB bus scaling
    if (g_hAcrMutex == NULL)
    {
        g_hAcrMutex = CreateMutex(NULL, FALSE, L"MUTEX_ACR");

        if (g_hAcrMutex == NULL)
        {
            ERRORMSG(1, (_T("CreateMutex failed!\r\n")));
            goto cleanUp;
        }
    }

    rc = TRUE;

cleanUp:
    if (!rc)  SdmaDealloc();
    return rc;
}


//-----------------------------------------------------------------------------
//
// Function:  SdmaDealloc
//
// This function deallocates the data structures required for interaction
// with the SDMA hardware.  Note that data structures for individual channels
// are freed in SdmaDeinit.
//
// Parameters:
//      None.
//
// Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
BOOL SdmaDealloc(void)
{
    // Unmap host shared region
    if (g_pHostSharedUA)
    {
        MmUnmapIoSpace(g_pHostSharedUA, g_HostSharedSize);
        g_pHostSharedUA = NULL;
        g_pHostSharedPA = NULL;
    }

    // Unmap peripheral registers
    if (g_pSDMA)
    {
        MmUnmapIoSpace(g_pSDMA, sizeof(CSP_SDMA_REGS));
        g_pSDMA = NULL;
    }

    if (g_hSdmaMutex)
    {
        CloseHandle(g_hSdmaMutex);
        g_hSdmaMutex = NULL;
    }

    if (g_hAcrMutex)
    {
        CloseHandle(g_hAcrMutex);
        g_hAcrMutex = NULL;
    }

    return TRUE;
}


//-----------------------------------------------------------------------------
//
// Function:  SdmaWritePM
//
// This function uses the channel 0 boot code to write code to the
// SDMA's internal memory.  This function can be used to load SDMA scripts
// from the host memory into the SDMA's RAM.
//
// Parameters:
//      srcAddrPA
//          [in] - Physical source address for data to be written into SDMA.
//
//      dstAddrPM
//          [in] - Physical destination address within SDMA program memory
//          into which the data will be written.
//
//      count
//          [in] - Number of data bytes to be written into the SDAM.
//
// Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
static BOOL SdmaWritePM(UINT32 srcAddrPA, UINT32 dstAddrPM, UINT16 count)
{
    BOOL rc = FALSE;

    // Request lock
    SdmaLockCtrlChan();

    // Configure chan 0 buffer descriptor
    g_pHostSharedUA->chan0BufDesc.mode =
        CSP_BITFVAL(SDMA_MODE_COMMAND, SDMA_CMD_C0_SET_PM) |
        CSP_BITFVAL(SDMA_MODE_EXT, SDMA_MODE_EXT_USED) |
        CSP_BITFVAL(SDMA_MODE_ERROR, SDMA_MODE_ERROR_NOTFOUND) |
        CSP_BITFVAL(SDMA_MODE_INTR, SDMA_MODE_INTR_NOSEND) |
        CSP_BITFVAL(SDMA_MODE_CONT, SDMA_MODE_CONT_END) |
        CSP_BITFVAL(SDMA_MODE_WRAP, SDMA_MODE_WRAP_WRAP2FIRST) |
        CSP_BITFVAL(SDMA_MODE_DONE, SDMA_MODE_DONE_NOTREADY) |
        CSP_BITFVAL(SDMA_MODE_COUNT, count / 2);

    g_pHostSharedUA->chan0BufDesc.srcAddrPA = srcAddrPA;
    g_pHostSharedUA->chan0BufDesc.destAddrPA = dstAddrPM;

    // Start the channel 0 script
    if (!DDKSdmaStartChan(0))
    {
        ERRORMSG(1, (_T("Chan 0 cannot be started!\r\n")));
        goto cleanUp;
    }

    // Wait for script to complete
    while (SdmaGetChanStatus(0))
    {
        // Give up the CPU
        Sleep(0);
    }

    // Make sure chan 0 buffer descriptor was processed
    if (CSP_BITFEXT(g_pHostSharedUA->chan0BufDesc.mode, SDMA_MODE_DONE)
        == SDMA_MODE_DONE_NOTREADY)
    {
        ERRORMSG(1, (_T("Chan 0 buffer descriptor not processed!\r\n")));
        goto cleanUp;
    }

    // Make sure chan 0 buffer descriptor did not incur error
    if (CSP_BITFEXT(g_pHostSharedUA->chan0BufDesc.mode, SDMA_MODE_ERROR)
        == SDMA_MODE_ERROR_FOUND)
    {
        ERRORMSG(1, (_T("Chan 0 buffer descriptor incurred error!\r\n")));
        goto cleanUp;
    }

    rc = TRUE;

cleanUp:
    SdmaUnlockCtrlChan();
    return rc;
}


//-----------------------------------------------------------------------------
//
// Function:  SdmaWriteChanContext
//
// This function uses the channel 0 boot code to load a channel context into
// the SDMA's internal memory reserved for the context page area.
//
// Parameters:
//      chan
//          [in] - Virtual channel whose context will be loaded.
//
//      srcAddrPA
//          [in] - Physical source address for context to be written into SDMA.
//
// Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
static BOOL SdmaWriteChanContext(UINT8 chan, UINT32 srcAddrPA)
{
    BOOL rc = FALSE;

    // Configure chan 0 buffer descriptor
    g_pHostSharedUA->chan0BufDesc.mode =
        CSP_BITFVAL(SDMA_MODE_COMMAND, CMD_SETCTX(chan)) |
        CSP_BITFVAL(SDMA_MODE_EXT, SDMA_MODE_EXT_UNUSED) |
        CSP_BITFVAL(SDMA_MODE_ERROR, SDMA_MODE_ERROR_NOTFOUND) |
        CSP_BITFVAL(SDMA_MODE_INTR, SDMA_MODE_INTR_NOSEND) |
        CSP_BITFVAL(SDMA_MODE_CONT, SDMA_MODE_CONT_END) |
        CSP_BITFVAL(SDMA_MODE_WRAP, SDMA_MODE_WRAP_WRAP2FIRST) |
        CSP_BITFVAL(SDMA_MODE_DONE, SDMA_MODE_DONE_NOTREADY) |
        CSP_BITFVAL(SDMA_MODE_COUNT, sizeof(SDMA_CHANNEL_CONTEXT) / 4);

    g_pHostSharedUA->chan0BufDesc.srcAddrPA = srcAddrPA;

    // Start the channel 0 script
    if (!DDKSdmaStartChan(0))
    {
        ERRORMSG(1, (_T("Chan 0 cannot be started!\r\n")));
        goto cleanUp;
    }

    // Wait for script to complete
    while (SdmaGetChanStatus(0))
    {
        // Give up the CPU
        Sleep(0);
    }

    // Make sure chan 0 buffer descriptor was processed
    if (CSP_BITFEXT(g_pHostSharedUA->chan0BufDesc.mode, SDMA_MODE_DONE)
        == SDMA_MODE_DONE_NOTREADY)
    {
        ERRORMSG(1, (_T("Chan 0 buffer descriptor not processed!\r\n")));
        goto cleanUp;
    }

    // Make sure chan 0 buffer descriptor did not incur error
    if (CSP_BITFEXT(g_pHostSharedUA->chan0BufDesc.mode, SDMA_MODE_ERROR)
        == SDMA_MODE_ERROR_FOUND)
    {
        ERRORMSG(1, (_T("Chan 0 buffer descriptor incurred error!\r\n")));
        goto cleanUp;
    }

    rc = TRUE;

cleanUp:
    return rc;
}


//-----------------------------------------------------------------------------
//
// Function:  SdmaSetChanOverride
//
// This function configures the override bits for a specified channel.
//
// Parameters:
//      chan
//          [in] - Channel for which override bits will be set.
//
//      bHostOvr
//          [in] - Specifies if the HO (Host Override) bit is set.
//
//      bEventOvr
//          [in] - Specifies if the EO (Event Override) bit is set.
//
//      bDspOvr
//          [in] - Specifies if the DO (DSP Override) bit is set.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
static void SdmaSetChanOverride(UINT8 chan, BOOL bHostOvr,
    BOOL bEventOvr, BOOL bDspOvr)
{
    UINT32 oldOvr, newOvr, *pReg;
    
    // Use interlocked function to update override registers
    pReg = &g_pSDMA->HOSTOVR;
    do
    {
        oldOvr = INREG32((LPLONG) pReg);
        newOvr = (oldOvr & (~CHAN_MASK(chan))) | (bHostOvr << chan);
    } while ((UINT32) InterlockedTestExchange((LPLONG) pReg, 
                        oldOvr, newOvr) != oldOvr);        

    pReg = &g_pSDMA->EVTOVR;
    do
    {
        oldOvr = INREG32((LPLONG) pReg);
        newOvr = (oldOvr & (~CHAN_MASK(chan))) | (bEventOvr << chan);
    } while ((UINT32) InterlockedTestExchange((LPLONG) pReg, 
                        oldOvr, newOvr) != oldOvr);        

    pReg = &g_pSDMA->DSPOVR;
    do
    {
        oldOvr = INREG32((LPLONG) pReg);
        newOvr = (oldOvr & (~CHAN_MASK(chan))) | (bDspOvr << chan);
    } while ((UINT32) InterlockedTestExchange((LPLONG) pReg, 
                        oldOvr, newOvr) != oldOvr);      

}


//-----------------------------------------------------------------------------
//
// Function:  SdmaSetChanPriority
//
// This function configures the priority for a specified channel.
//
// Parameters:
//      chan
//          [in] - Channel for which the priority will be set.
//
//      priority
//          [in] - Priority assigned to the channel.  Possible priority
//          values are 0-7 with 7 being the highest possible priority.
//          A channel assigned priority 0 will be prevented from running.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
static void SdmaSetChanPriority(UINT8 chan, UINT8 priority)
{
    OUTREG32(&g_pSDMA->CHNPRI[chan], priority);
}


//-----------------------------------------------------------------------------
//
// Function:  SdmaGetChanStatus
//
// This function reads the status the specified channel.
//
// Parameters:
//      chan
//          [in] - Virtual channel to for which status will be read.
//
// Returns:
//      Returns TRUE if the specified channel is enabled (HE bit set),
//      else returns FALSE (HE bit clear).
//
//-----------------------------------------------------------------------------
static BOOL SdmaGetChanStatus(UINT8 chan)
{
    return (INREG32(&g_pSDMA->STOP_STAT) & CHAN_MASK(chan)) ? TRUE : FALSE;
}


//-----------------------------------------------------------------------------
//
// Function:  SdmaFindUnusedChan
//
// This function searches the SDMA channel control blocks to find a virtual
// channel that is currently not in use and is available for a handling
// a peripheral-to-memory, memory-to-peripheral, or memory-to-memory
// trasfer.
//
// Parameters:
//      priority
//          [in] - Priority that will be assigned to unused channel to
//          claim it as used.
//
// Returns:
//      Returns virtual channel ID if an available virtual channel is found,
//      otherwise returns 0.
//
//-----------------------------------------------------------------------------
static UINT8 SdmaFindUnusedChan(UINT8 priority)
{
    UINT8 chan;

    // Walk the CCB to find available virtual channel (channel 0 is reserved)
    for (chan = 1; chan < SDMA_NUM_CHANNELS; chan++)
    {
        // Check channel status for availability
        if (INREG32(&g_pSDMA->CHNPRI[chan]) == SDMA_CHNPRI_CHNPRI_DISABLE)
        {
            // Use interlocked call to safely update CHNPRI register which
            // is a shared resource
            if (InterlockedTestExchange((LPLONG) &g_pSDMA->CHNPRI[chan], 
                SDMA_CHNPRI_CHNPRI_DISABLE, priority) 
                == SDMA_CHNPRI_CHNPRI_DISABLE)
            {
                break;
            }
        }
    }

    if (chan >= SDMA_NUM_CHANNELS)
    {
        ERRORMSG(1, (_T("No unused channel found!\r\n")));
        chan = 0;
    }

    return chan;

}


//-----------------------------------------------------------------------------
//
//  Function:  SdmaLockCtrlChan
//
//  This function locks the SDMA control channel (channel 0) to provide
//  safe access to the control channel shared resources.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
static void SdmaLockCtrlChan(void)
{
    WaitForSingleObject(g_hSdmaMutex, INFINITE);
    OUTREG32(&g_pSDMA->CHNPRI[0], SDMA_CHNPRI_CHNPRI_HIGHEST);
}


//-----------------------------------------------------------------------------
//
//  Function:  SdmaUnlockCtrlChan
//
//  This function unlocks the SDMA control channel (channel 0).
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
static void SdmaUnlockCtrlChan(void)
{
    OUTREG32(&g_pSDMA->CHNPRI[0], SDMA_CHNPRI_CHNPRI_DISABLE); 
    ReleaseMutex(g_hSdmaMutex);
}


//-----------------------------------------------------------------------------
//
//  Function:  SdmaValidChan
//
//  This function determines if a channel is valid and configured properly
//  for allocation and configuration of buffer descriptors.
//
//  Parameters:
//      chan
//          [in] - Virtual channel to for which status will be read.
//
//  Returns:
//      Returns TRUE if the specified channel is valid, otherwise
//      returns FALSE.
//
//-----------------------------------------------------------------------------
static BOOL SdmaValidChan(UINT8 chan)
{
    BOOL rc = FALSE;
    
    // Validate channel parameter
    if (chan <= 0 || chan >= SDMA_NUM_CHANNELS)
    {
        ERRORMSG(1, (_T("Invalid channel!\r\n")));
        goto cleanUp;
    }

    // Check if channel has been previously opened
    if (INREG32(&g_pSDMA->CHNPRI[chan]) == SDMA_CHNPRI_CHNPRI_DISABLE)
    {
        ERRORMSG(1, (_T("Channel not opened!\r\n")));
        goto cleanUp;
    }

    rc = TRUE;
    
cleanUp:

    return rc;
}
