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
//------------------------------------------------------------------------------
//
// Copyright (C) 2004, MOTOROLA, INC. All Rights Reserved
//
//------------------------------------------------------------------------------
//
// Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  ddk_dmac.c
//
//  Provides the DDK (driver development kit) for the DMA controller (DMAC).
//
//  NOTES:
//  To use a DMA channel:
//  =====================
//  1. In INC/bsp_cfg.h, add the following defines:
//      - desired channel identifier
//  2. Allocate DMA buffers using HalAllocateCommonBuffer(), request and bind
//     the desired channel. Example, look at SDHC or audio drivers.
//  3. If using DMA interrupts, add handling in driver for the 
//     SYSINTR assigned to the channel.
//
//  WARNING:
//  ========
//  The DMAC status registers MUST BE CLEARED after every DMA transfer
//  in order for the channel to be restarted. The user driver must call 
//  ClearChannelInterrupt() before calling TransStart() even if 
//  interrupts are not enabled/used.
//
//------------------------------------------------------------------------------
#include <windows.h>
#include <ceddk.h>
#include <csp.h>

#ifdef DEBUG

DBGPARAM dpCurSettings = {
    L"MX27DMAC", {
        L"Info",	L"Function",    L"Warnings",    L"Errors",
        L"Init",	L"Undefined",	L"Undefined",   L"Undefined",
        L"Undefined",   L"Undefined",   L"Undefined",   L"Undefined",
        L"Undefined",	L"Undefined",	L"Undefined",	L"Undefined"
    },
    0x0008
};

#define ZONE_INFO           DEBUGZONE(0)
#define ZONE_FUNCTION       DEBUGZONE(1)
#define ZONE_WARN           DEBUGZONE(2)
#define ZONE_ERROR          DEBUGZONE(3)
#define ZONE_INIT			DEBUGZONE(4) 
#endif
//-----------------------------------------------------------------------------
// External Functions

//-----------------------------------------------------------------------------
// External Variables


//-----------------------------------------------------------------------------
// Defines

#define CHAN_MASK(chan) (1 << (chan))
//-----------------------------------------------------------------------------
// Types
typedef struct {
    BOOL    Register2DFlagA;      // flag for using status of 2D registers set A
    BOOL    Register2DFlagB;      // flag for using status of 2D registers set B
    UINT16  BurstTimeout;         // burst timeout count down data
    UINT16  ChannelAllocated;     // flags for channel allocation status
} DMAC_DLL_SHARED, *PDMAC_DLL_SHARED;

//DMAC channel data structure
typedef struct {
   UINT32 SAR;
   UINT32 DAR;
   UINT32 CNTR;
   UINT32 CCR;
   UINT32 RSSR;
   UINT32 BLR;   
   UINT32 RTOR_BUCR;
} DMAC_CHANNEL_DATA, *PDMAC_CHANNEL_DATA;

//-----------------------------------------------------------------------------
// Global Variables


//-----------------------------------------------------------------------------
// Local Variables
static HANDLE g_hFileMap = NULL;
static PDMAC_DLL_SHARED g_pDllShared = NULL;
static CSP_DMAC_REGS *g_pDMAC = NULL;



//-----------------------------------------------------------------------------
// Local Functions
BOOL DmacAlloc(void);
BOOL DmacDealloc(void);

BOOL DmacInit(void);
BOOL DmacDeinit(void);

static BOOL DmacTranslateCfg(DMAC_CHANNEL_CFG *pChannelCfg, DMAC_CHANNEL_DATA *pChannelData);
static void DmacShowRegister(UINT32 channelMask);


//-----------------------------------------------------------------------------
//
// Function:  DmacDeinit
//
// This function deinitializes the DMAC.  Called by the Device Manager to
// de-initialize a device.
//
// Parameters:
//      None.
//
// Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
BOOL DmacDeinit(void)
{
    BOOL rc = TRUE;
    UINT8 chan;

    DEBUGMSG(ZONE_INIT, (_T("DmacDeinit+\r\n")));

    // Disable all channels
    for (chan = 1; chan < DMAC_NUM_CHANNELS; chan++)
    {
        g_pDMAC->CHANNEL[chan].CCR = 0;
    }

    // Deallocate DMAC driver data structures
    DmacDealloc();

    DEBUGMSG(ZONE_INIT, (_T("DmacDeinit- (rc = %d)\r\n"), rc));

    return rc;
}

//-----------------------------------------------------------------------------
//
// Function:  DmacInit
//
// This function initializes the DMAC.  Called by the CSPDDK Init() function to
// initialize the DMA controller.
//
// Parameters:
//      None.
//
// Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DmacInit(void)
{
    BOOL rc = FALSE;
    UINT32 i;

    DEBUGMSG(ZONE_INIT, (_T("DmacInit+\r\n")));

    // Set 2D memory registers use status flag as free
    g_pDllShared->Register2DFlagA = DMAC_CHANNEL_INVALID;
    g_pDllShared->Register2DFlagB = DMAC_CHANNEL_INVALID;

    // reset DMAC
    INSREG32BF(&g_pDMAC->DCR, DMAC_DCR_DRST, DMAC_DCR_DRST_RESET);

    // Default to burst time out disabled
    DEBUGMSG(ZONE_INIT, (TEXT("DMAC: Burst time out disabled!\r\n")));
    INSREG32BF(&g_pDMAC->DBTOCR, DMAC_DBTOCR_EN, DMAC_DBTOCR_EN_DISABLE);

    // Clear 2D memory registers
    g_pDMAC->XSRA = 0;
    g_pDMAC->YSRA = 0;
    g_pDMAC->WSRA = 0;
    g_pDMAC->XSRB = 0;
    g_pDMAC->YSRB = 0;
    g_pDMAC->WSRB = 0;

    // Default to disable all DMA channels
    for(i = 0; i < DMAC_NUM_CHANNELS; i++)
    {
        g_pDMAC->CHANNEL[i].CCR = 0;
    }

    // Enable DMAC for priviledged access only
    g_pDMAC->DCR = (CSP_BITFVAL(DMAC_DCR_DEN, DMAC_DCR_DEN_ENABLE) |
                    CSP_BITFVAL(DMAC_DCR_DRST, DMAC_DCR_DRST_NOEFFECT) |
                    CSP_BITFVAL(DMAC_DCR_DAM, DMAC_DCR_DAM_PRIVILEGED));

    rc = TRUE;
    
    if(!rc)
        DmacDeinit();
    DEBUGMSG(ZONE_INIT, (_T("DmacInit- (rc = %d)\r\n"), rc));
    return rc;
}

//-----------------------------------------------------------------------------
//
// Function:  DDKDmacRequestChan
//
// This function checks that the desired channel is currently not in use and is 
// available. If not, the next highest free channel is returned. The channel is
// allocated if successful.
//
// Parameters:
//      chan
//            [in] - the desired channel number
//
// Returns:
//      Returns channel ID if available channel,
//      otherwise returns DMAC_CHANNEL_INVALID.
//
//-----------------------------------------------------------------------------
UINT8 DDKDmacRequestChan(UINT8 chan)
{
    INT8 i = 0;
    UINT32 mask;

    DEBUGMSG(ZONE_FUNCTION, (_T("DDKDmacRequestChan+:(chan = %d)\r\n"), chan));

    if(chan >= DMAC_NUM_CHANNELS)
        return DMAC_CHANNEL_INVALID;
        
    // Check channel allocation status for availability
    mask = CHAN_MASK(chan);
    if((g_pDllShared->ChannelAllocated & mask) != 0)
    {
        // Walk the channel allocation status to find available channel
        for(i = DMAC_NUM_CHANNELS - 1 , mask = (1 << (DMAC_NUM_CHANNELS - 1)); 
            i >= 0; i--, mask >>= 1)
        {
            if((g_pDllShared->ChannelAllocated & mask) == 0)
            {
                chan = i;
                break;
            }
        }
    }
    
    if(i < 0)
    {
        DEBUGMSG(ZONE_ERROR,
            (_T("DDKDmacRequestChan:  No channel available\r\n")));
        chan = DMAC_CHANNEL_INVALID;
    }
    else
    {
        g_pDllShared->ChannelAllocated |= CHAN_MASK(chan);
    }

    DEBUGMSG(ZONE_FUNCTION, (_T("DDKDmacRequestChan-:(chan = %d)\r\n"), chan));

    return chan;
}

//-----------------------------------------------------------------------------
//
// Function:  DDKDmacReleaseChan
//
// This function releases a channel previously requested by the DDKDmacRequestChan
// routine.
//
// Parameters:
//      chan
//          [in] - channel returned by DDKDmacRequestChan function.
//
// Returns:
//      Returns TRUE if the release operation was successful, otherwise
//      returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKDmacReleaseChan(UINT8 chan)
{
    BOOL rc = FALSE;
    UINT32 mask;

    DEBUGMSG(ZONE_FUNCTION, (_T("DDKDmacReleaseChan+\r\n")));

    // Validate channel parameter
    if (chan >= DMAC_NUM_CHANNELS)
    {
        DEBUGMSG(ZONE_ERROR, (_T("DDKDmacReleaseChan: Invalid channel %d\r\n"), chan));
        goto cleanUp;
    }

    // Make sure this channel is really in use
    mask = CHAN_MASK(chan);
    if((g_pDllShared->ChannelAllocated & mask) == 0)
    {
        DEBUGMSG(ZONE_WARN, (_T("DDKDmacReleaseChan: Channel%d not allocated.\r\n"), chan));
        goto cleanUp;
    }

    // Disable the channel first
    INSREG32BF(&g_pDMAC->CHANNEL[chan].CCR, DMAC_CCR_CEN, DMAC_CCR_CEN_DISABLE);       

    // Mask channel interrupt & clear all interrupt status
    g_pDMAC->DIMR |= mask;
    g_pDMAC->DISR = mask;
    g_pDMAC->DBTOSR = mask;
    g_pDMAC->DRTOSR = mask;
    g_pDMAC->DSESR = mask;
    g_pDMAC->DBOSR = mask;

    // Clear global allocation flag
    g_pDllShared->ChannelAllocated  &=  ~mask;

    // Clear 2D register set usage flag if used
    if(g_pDllShared->Register2DFlagA == chan)
        g_pDllShared->Register2DFlagA = DMAC_CHANNEL_INVALID;
    if(g_pDllShared->Register2DFlagB == chan)
        g_pDllShared->Register2DFlagB = DMAC_CHANNEL_INVALID;

    DEBUGMSG(ZONE_INFO, (TEXT("DDKDmacReleaseChan: Channel%d is released.\r\n"), chan));   

    rc = TRUE;

cleanUp:
    DEBUGMSG(ZONE_FUNCTION, (_T("DDKDmacReleaseChan- (rc = %d)\r\n"), rc));
    return rc;
}

//-----------------------------------------------------------------------------
//
// Function:  DDKDmacConfigureChan
//
// This function configures a channel after a request is successful.
//
// Parameters:
//      chan
//          [in] - channel to bind.
//
//      pChannelCfg
//          [in] - Configuration for the desired channel.
//
// Returns:
//      Returns channel ID if the open operation was successful,
//      otherwise returns DMAC_CHANNEL_INVALID.
//
//-----------------------------------------------------------------------------
UINT8 DDKDmacConfigureChan(UINT8 chan, DMAC_CHANNEL_CFG *pChannelCfg)
{
    DMAC_CHANNEL_DATA ChannelData;
    DMAC_CHANNEL_DATA *pChannelData = &ChannelData;
    BOOL bSetB2DinUse = FALSE;
    
    DEBUGMSG(ZONE_FUNCTION, (_T("DDKDmacConfigureChan+\r\n")));

    // Check if the channel is already allocated
    if((g_pDllShared->ChannelAllocated & CHAN_MASK(chan)) == 0)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("DDKDmacConfigureChan: Channel%d is not allocated!!\r\n"), chan));
        chan = DMAC_CHANNEL_INVALID;
        goto cleanup;
    }

    // Check if 2D memory is needed
    if ((pChannelCfg->DstMode== DMAC_TRANSFER_MODE_2D_MEMORY) || 
        (pChannelCfg->SrcMode == DMAC_TRANSFER_MODE_2D_MEMORY))
    {
        // use 2D memory registers
        if(g_pDllShared->Register2DFlagA == DMAC_CHANNEL_INVALID)
        {
            // Set A is free
            // keep track track of usage
            g_pDllShared->Register2DFlagA = chan;

            // Configure A set registers
            INSREG32BF(&g_pDMAC->XSRA, DMAC_XSR_XS, pChannelCfg->XSR);
            INSREG32BF(&g_pDMAC->YSRA, DMAC_YSR_YS, pChannelCfg->YSR);
            INSREG32BF(&g_pDMAC->WSRA, DMAC_WSR_WS, pChannelCfg->WSR);
        }
        else if(g_pDllShared->Register2DFlagB == DMAC_CHANNEL_INVALID)
        {
            // Set B is free
            // keep track track of usage
            g_pDllShared->Register2DFlagB = chan;

            // Configure B set registers
            INSREG32BF(&g_pDMAC->XSRB, DMAC_XSR_XS, pChannelCfg->XSR);
            INSREG32BF(&g_pDMAC->YSRB, DMAC_YSR_YS, pChannelCfg->YSR);
            INSREG32BF(&g_pDMAC->WSRB, DMAC_WSR_WS, pChannelCfg->WSR);
            
            bSetB2DinUse = TRUE;
        }
        else   // No free 2D memory register set
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("DDKDmacConfigureChan: All 2D memory registers are used!!\r\n")));
            chan = DMAC_CHANNEL_INVALID;
            goto cleanup;
        }
    }

    // clear channel data
    memset(pChannelData, 0x00, sizeof(DMAC_CHANNEL_DATA));

    // Translate the input configuration into register values
    if(DmacTranslateCfg(pChannelCfg, pChannelData) == FALSE)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("DDKDmacConfigureChan: Error in channel configuration!\r\n")));
        chan = DMAC_CHANNEL_INVALID;
        goto cleanup;
    }

    // Disable DMAC channel
    INSREG32BF(&g_pDMAC->CHANNEL[chan].CCR, DMAC_CCR_CEN, DMAC_CCR_CEN_DISABLE);       

    // Set source, destination addresses and data size
    g_pDMAC->CHANNEL[chan].SAR = pChannelData->SAR;
    g_pDMAC->CHANNEL[chan].DAR = pChannelData->DAR;
    g_pDMAC->CHANNEL[chan].CNTR = pChannelData->CNTR;

    // set other channel registers
    g_pDMAC->CHANNEL[chan].RSSR = pChannelData->RSSR;
    g_pDMAC->CHANNEL[chan].BLR = pChannelData->BLR;
    g_pDMAC->CHANNEL[chan].RTOR_BUCR = pChannelData->RTOR_BUCR;      

    if(bSetB2DinUse)
        CSP_BITFINS(pChannelData->CCR, DMAC_CCR_MSEL, DMAC_CCR_MSEL_2DMEM_SETB);

    // Set channel control register
    CSP_BITFINS(pChannelData->CCR, DMAC_CCR_CEN, DMAC_CCR_CEN_DISABLE);
    g_pDMAC->CHANNEL[chan].CCR = pChannelData->CCR; 

    DEBUGMSG(ZONE_INFO, (TEXT("DDKDmacConfigureChan: channel %d is configured! CCR(0x%08x)\r\n"), 
                                chan, &g_pDMAC->CHANNEL[chan].CCR));

cleanup:
    DEBUGMSG(ZONE_FUNCTION, (_T("DDKDmacConfigureChan- (chan = %d)\r\n"), chan));
    return chan;
}

//-----------------------------------------------------------------------------
//
// Function:  DDKDmacStartChan
//
// This function starts the specified channel.
//
// Parameters:
//      chan
//          [in] - DMAC channel to start.
//
// Returns:
//      Returns TRUE if the start request is successful, else returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKDmacStartChan(UINT8 chan)
{
    BOOL rc = FALSE;

    DEBUGMSG(ZONE_FUNCTION, (_T("DDKDmacStartChan+ %d\r\n"), chan));

    // Validate channel parameter
    if(chan >= DMAC_NUM_CHANNELS)
    {
        DEBUGMSG(ZONE_ERROR,
            (_T("DDKDmacStartChan: Invalid channel %d\r\n"), chan));
        goto cleanUp;
    }

    // Check if channel is in use
    if((g_pDllShared->ChannelAllocated & CHAN_MASK(chan)) == 0)
    {
        DEBUGMSG(ZONE_WARN,
            (_T("DDKDmacStartChan:  Chan %d start ignored, not allocated"), chan));
        goto cleanUp;
    }

    INSREG32BF(&g_pDMAC->CHANNEL[chan].CCR, DMAC_CCR_CEN, DMAC_CCR_CEN_DISABLE);
    INSREG32BF(&g_pDMAC->CHANNEL[chan].CCR, DMAC_CCR_CEN, DMAC_CCR_CEN_ENABLE);

    rc = TRUE;

cleanUp:
    DEBUGMSG(ZONE_FUNCTION, (_T("DDKDmacStartChan- (rc = %d)\r\n"), rc));
    return rc;
}


//-----------------------------------------------------------------------------
//
// Function:  DDKDmacStopChan
//
// This function stops the specified channel.
//
// Parameters:
//      chan
//          [in] - DMAC channel to start.
//
// Returns:
//      Returns TRUE if the stop request is successful, else returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKDmacStopChan(UINT8 chan)
{
    BOOL rc = FALSE;

    DEBUGMSG(ZONE_FUNCTION, (_T("DDKDmacStopChan+\r\n")));

    // Validate channel parameter
    if(chan >= DMAC_NUM_CHANNELS)
    {
        DEBUGMSG(ZONE_ERROR,
            (_T("DDKDmacStopChan: Invalid channel %d\r\n"), chan));
        goto cleanUp;
    }

    INSREG32BF(&g_pDMAC->CHANNEL[chan].CCR, DMAC_CCR_CEN, DMAC_CCR_CEN_DISABLE);

    rc = TRUE;

cleanUp:
    DEBUGMSG(ZONE_FUNCTION, (_T("DDKDmacStopChan- (rc = %d)\r\n"), rc));
    return rc;
}

//-----------------------------------------------------------------------------
//
// Function:  DmacGetTranStatus
//
// This function queries the status of a channel transfer.
//
// Parameters:
//      chan
//          [in] - channel returned by DDKDmacRequestChan function.
//
// Returns:
// Bitmap indicating type of interrupt status. DMAC_TRANSFER_STATUS_COMPLETE  
// indicates that the DMA cycle completed with out error.
// DMAC_TRANSFER_STATUS_NONE indicates that the DMA cycle is ongoing.
// Any other return value indicates error in the transfer.
//
// In interrupt mode, the return value indicate 
// the type of pending interrupt
//
//-----------------------------------------------------------------------------
UINT32 DDKDmacGetTransStatus(UINT8 chan)
{
    UINT32 rc = DMAC_TRANSFER_STATUS_NONE;
    UINT32 mask;

    DEBUGMSG(ZONE_FUNCTION, (_T("DDKDmacGetTransStatus+\r\n")));

    // Validate channel parameter
    if(chan >= DMAC_NUM_CHANNELS)
    {
        DEBUGMSG(ZONE_ERROR, (_T("DDKDmacGetTransStatus: Invalid channel\r\n")));
        goto cleanUp;
    }

    // Check if channel in use
    mask = 1 << chan;
    if((g_pDllShared->ChannelAllocated & mask) == 0)
    {
        DEBUGMSG(ZONE_ERROR, (_T("DDKDmacGetTransStatus: Channel%d not allocated\r\n"), chan));
        goto cleanUp;
    }

    // Get transfer status
    if((g_pDMAC->DSESR & mask) != 0)
        rc |= DMAC_TRANSFER_STATUS_ERROR;

    if((g_pDMAC->DBOSR & mask) != 0)
       rc |= DMAC_TRANSFER_STATUS_BUFFER_OVERFLOW;

    if((g_pDMAC->DRTOSR & mask) != 0)
       rc |= DMAC_TRANSFER_STATUS_REQ_TIMEOUT;

    if((g_pDMAC->DBTOSR & mask) != 0)
        rc |= DMAC_TRANSFER_STATUS_BURST_TIMEOUT;

    // Interrupt pending with no error
    if( (g_pDMAC->DISR & mask) == mask && rc == DMAC_TRANSFER_STATUS_NONE)
        rc |= DMAC_TRANSFER_STATUS_COMPLETE;

cleanUp:
    DEBUGMSG(ZONE_FUNCTION, (_T("DDKDmacGetTransStatus- (rc = 0x%X)\r\n"), rc));
    return rc;

}

//------------------------------------------------------------------------------
//
// Function:  DmacGetTranSize
//
// This function returns the number of bytes transferred by a channel.
//
// Parameters:
//      chan
//          [in] - channel returned by DDKDmacRequestChan function.
//
// Returns:
//       number of bytes transferred by a channel.
//
//------------------------------------------------------------------------------
UINT32 DDKDmacGetTransSize(UINT8 chan)
{
    UINT32 rc;
    
    DEBUGMSG(ZONE_FUNCTION, (_T("DmacGetTranSize+\r\n")));
    
    rc = g_pDMAC->CHANNEL[chan].CCNR;

    DEBUGMSG(ZONE_FUNCTION, (_T("DmacGetTranSize- (rc = 0x%X)\r\n"), rc));
    return rc;
}

//------------------------------------------------------------------------------
//
// Function:  DDKDmacSetSrcAddress
//
// This function sets the source address for the next DMA cycle.
//
// Parameters:
//      chan
//          [in] - channel returned by DDKDmacRequestChan function.
//
//      src
//          [in] - source address for next DMA cycle
// Returns:
//       None
//
//------------------------------------------------------------------------------
void DDKDmacSetSrcAddress(UINT8 chan, UINT32 src)
{
    DEBUGMSG(ZONE_FUNCTION, (_T("DDKDmacSetSrcAddress+\r\n")));
    
    g_pDMAC->CHANNEL[chan].SAR = src;

    DEBUGMSG(ZONE_FUNCTION, (_T("DDKDmacSetSrcAddress- (chan = %d src = 0x%X)\r\n"), 
                            chan, src));
}

//------------------------------------------------------------------------------
//
// Function:  DDKDmacSetDestAddress
//
// This function sets the destination address for the next DMA cycle.
//
// Parameters:
//      chan
//          [in] - channel returned by DDKDmacRequestChan function.
//
//      dest
//          [in] - destination address for next DMA cycle
// Returns:
//       None
//
//------------------------------------------------------------------------------
void DDKDmacSetDestAddress(UINT8 chan, UINT32 dest)
{
    DEBUGMSG(ZONE_FUNCTION, (_T("DDKDmacSetDestAddress+\r\n")));
    
    g_pDMAC->CHANNEL[chan].DAR = dest;

    DEBUGMSG(ZONE_FUNCTION, (_T("DDKDmacSetDestAddress- (chan = %d dest = 0x%X)\r\n"), 
                        chan, dest));
}

//------------------------------------------------------------------------------
//
// Function:  DDKDmacSetTransCount
//
// This function sets the transfer byte count for the next DMA cycle.
//
// Parameters:
//      chan
//          [in] - channel returned by DDKDmacRequestChan function.
//
//      count
//          [in] - transfer byte count for next DMA cycle
// Returns:
//       None
//
//------------------------------------------------------------------------------
void DDKDmacSetTransCount(UINT8 chan, UINT32 count)
{
    DEBUGMSG(ZONE_FUNCTION, (_T("DDKDmacSetTransCount+\r\n")));
    
    g_pDMAC->CHANNEL[chan].CNTR = count;

    DEBUGMSG(ZONE_FUNCTION, (_T("DDKDmacSetTransCount- (chan = %d count = 0x%X)\r\n"), 
                        chan, count));
}

//------------------------------------------------------------------------------
//
// Function:  DDKDmacSetBurstLength
//
// This function sets the transfer burst length for the next DMA cycle.
//
// Parameters:
//      chan
//          [in] - channel returned by DDKDmacRequestChan function.
//
//      burstLen
//          [in] - Burst length for next DMA cycle
// Returns:
//       None
//
//------------------------------------------------------------------------------
void DDKDmacSetBurstLength(UINT8 chan, UINT32 burstLen)
{
    DEBUGMSG(ZONE_FUNCTION, (_T("DDKDmacSetTransCount+\r\n")));
    
    INSREG32BF(&g_pDMAC->CHANNEL[chan].BLR, DMAC_BLR_BL, burstLen);

    DEBUGMSG(ZONE_FUNCTION, (_T("DDKDmacSetTransCount- (chan = %d burstLen = 0x%X)\r\n"), 
                        chan, burstLen));
}

//------------------------------------------------------------------------------
//
// Function:  DDKDmacSetRepeatType
//
// This function sets the transfer repeat type for the next DMA cycle.
//
// Parameters:
//      chan
//          [in] - channel returned by DDKDmacRequestChan function.
//
//      repeatType
//          [in] - Repeat type for next DMA cycle
// Returns:
//       None
//
//------------------------------------------------------------------------------
void DDKDmacSetRepeatType(UINT8 chan, DMAC_REPEAT_TYPE repeatType)
{
    DEBUGMSG(ZONE_FUNCTION, (_T("DDKDmacSetRepeatType+\r\n")));
    
    switch(repeatType)
    {
        case DMAC_REPEAT_DISABLED:
            INSREG32BF(&g_pDMAC->CHANNEL[chan].CCR, DMAC_CCR_RPT, DMAC_CCR_RPT_DISABLE);
            INSREG32BF(&g_pDMAC->CHANNEL[chan].CCR, DMAC_CCR_ACRPT, DMAC_CCR_ACRPT_DISABLE);
            DEBUGMSG(ZONE_INFO, (_T("DDKDmacSetRepeatType: No repeat.\r\n")));
            break;

        case DMAC_REPEAT_ONCE:
            INSREG32BF(&g_pDMAC->CHANNEL[chan].CCR, DMAC_CCR_RPT, DMAC_CCR_RPT_ENABLE);
            INSREG32BF(&g_pDMAC->CHANNEL[chan].CCR, DMAC_CCR_ACRPT, DMAC_CCR_ACRPT_AUTOCLEAR_RPT);
            DEBUGMSG(ZONE_INFO, (_T("DDKDmacSetRepeatType: Repeat once.\r\n")));
            break;

        case DMAC_REPEAT_FOREVER:
            INSREG32BF(&g_pDMAC->CHANNEL[chan].CCR, DMAC_CCR_RPT, DMAC_CCR_RPT_ENABLE);
            INSREG32BF(&g_pDMAC->CHANNEL[chan].CCR, DMAC_CCR_ACRPT, DMAC_CCR_ACRPT_DISABLE);
            DEBUGMSG(ZONE_INFO, (_T("DDKDmacSetRepeatType: Repeat forever.\r\n")));
            break;

        default:
            DEBUGMSG(ZONE_ERROR, (_T("DDKDmacSetRepeatType: Invalid repeat type.\r\n")));
    }
    DEBUGMSG(ZONE_FUNCTION, (_T("DDKDmacSetRepeatType- (chan = %d repeatType = 0x%X)\r\n"), 
                        chan, repeatType));
}    

//------------------------------------------------------------------------------
//
// Function:  DDKDmacIsChannelIntr
//
// This function returns true if the channel has interrupted.
//
// Parameters:
//      chan
//          [in] - channel returned by DDKDmacRequestChan function.
//
// Returns:
//      Returns TRUE if channel interrupt pending, otherwise returns FALSE.
//
//------------------------------------------------------------------------------
BOOL DDKDmacIsChannelIntr(UINT8 chan)
{
    UINT32 mask = CHAN_MASK(chan);
    return ((g_pDMAC->DISR & mask) == mask);
}

//------------------------------------------------------------------------------
//
// Function:  DDKDmacClearChannelIntr
//
// This function returns true if the channel has interrupted.
//
// Parameters:
//      chan
//          [in] - channel returned by DDKDmacRequestChan function.
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
void DDKDmacClearChannelIntr(UINT8 chan)
{
    UINT32 mask;
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("+DDKDmacClearChannelIntr\r\n")));

    mask = CHAN_MASK(chan);
    g_pDMAC->DSESR = mask;
    g_pDMAC->DBOSR = mask;
    g_pDMAC->DRTOSR = mask;
    g_pDMAC->DBTOSR = mask;
    g_pDMAC->DISR = mask;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("-DDKDmacClearChannelIntr\r\n")));
}

//------------------------------------------------------------------------------
//
// Function:  DDKDmacEnableChannelIntr
//
// This function enables interrupts for the desired channel.
//
// Parameters:
//      chan
//          [in] - channel returned by DDKDmacRequestChan function.
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
void DDKDmacEnableChannelIntr(UINT8 chan)
{
    UINT32 mask;
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("+DDKDmacEnableChannelIntr\r\n")));

    mask = CHAN_MASK(chan);
    // Clear any pending interrupt status
    g_pDMAC->DSESR = mask;
    g_pDMAC->DBOSR = mask;
    g_pDMAC->DRTOSR = mask;
    g_pDMAC->DBTOSR = mask;
    g_pDMAC->DISR = mask;
    // Enable channel interrupt
    g_pDMAC->DIMR &= ~mask;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("-DDKDmacEnableChannelIntr: DIMR=0x%x\r\n"), g_pDMAC->DIMR));
}

//------------------------------------------------------------------------------
//
// Function:  DDKDmacDisableChannelIntr
//
// This function disables interrupts for the desired channel.
//
// Parameters:
//      chan
//          [in] - channel returned by DDKDmacRequestChan function.
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
void DDKDmacDisableChannelIntr(UINT8 chan)
{
    UINT32 mask;
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("+DDKDmacDisableChannelIntr\r\n")));

    mask = CHAN_MASK(chan);
    // Disable channel interrupt
    g_pDMAC->DIMR |= mask;
    // Clear any pending interrupt status
    g_pDMAC->DSESR = mask;
    g_pDMAC->DBOSR = mask;
    g_pDMAC->DRTOSR = mask;
    g_pDMAC->DBTOSR = mask;
    g_pDMAC->DISR = mask;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("-DDKDmacDisableChannelIntr: DIMR=0x%x\r\n"), g_pDMAC->DIMR));
}

//-----------------------------------------------------------------------------
//
// Function:  DmacAlloc
//
// This function allocates the data structures required for interaction
// with the DMAC hardware.
//
// Parameters:
//      None.
//
// Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DmacAlloc(void)
{
    BOOL rc = FALSE;
    PHYSICAL_ADDRESS phyAddr;

    DEBUGMSG(ZONE_INIT, (_T("DmacAlloc+\r\n")));

    if (g_hFileMap == NULL)
    {
        g_hFileMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL,
            PAGE_READWRITE, 0, sizeof(DMAC_DLL_SHARED), _T("DMAC_DLL_SHARED"));

        if (g_hFileMap == NULL)
        {
            DEBUGMSG(ZONE_ERROR, (_T("DmacAlloc: CreateFileMapping failed!\r\n")));
            goto cleanUp;
        }
    }

    if (g_pDllShared == NULL)
    {
        g_pDllShared = (DMAC_DLL_SHARED *)MapViewOfFile(g_hFileMap,
            FILE_MAP_ALL_ACCESS, 0, 0, 0);

        if (g_pDllShared == NULL)
        {
            DEBUGMSG(ZONE_ERROR, (_T("DmacAlloc: MapViewOfFile failed!\r\n")));
            goto cleanUp;
        }
    }

    if (g_pDMAC == NULL)
    {
        phyAddr.QuadPart = CSP_BASE_REG_PA_DMAC;

        // Map peripheral physical address to virtual address
        g_pDMAC = (CSP_DMAC_REGS *)MmMapIoSpace(phyAddr, sizeof(CSP_DMAC_REGS), FALSE);

        // Check if virtual mapping failed
        if (g_pDMAC == NULL)
        {
            DEBUGMSG(ZONE_ERROR, (_T("DmacAlloc:  MmMapIoSpace failed!\r\n")));
            goto cleanUp;
        }
    }

    rc = TRUE;

cleanUp:
    // If initialization failed, be sure to clean up
    if (!rc) DmacDealloc();
    DEBUGMSG(ZONE_INIT, (_T("DmacAlloc- (rc = %d)\r\n"), rc));
    return rc;
}


//-----------------------------------------------------------------------------
//
// Function:  DmacDealloc
//
// This function deallocates the data structures required for interaction
// with the DMAC hardware.  Note that data structures for individual channels
// are freed in DmacDeinit.
//
// Parameters:
//      None.
//
// Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
BOOL DmacDealloc(void)
{
    DEBUGMSG(ZONE_INIT, (_T("DmacDealloc+\r\n")));

    // Unmap peripheral registers
    if (g_pDMAC)
    {
        MmUnmapIoSpace(g_pDMAC, sizeof(CSP_DMAC_REGS));
        g_pDMAC = NULL;
    }

    // Unmap view of shared DLL memory
    if (g_pDllShared)
    {
        UnmapViewOfFile(g_pDllShared);
        g_pDllShared = NULL;
    }

    // Close handle to shared DLL memory
    if (g_hFileMap)
    {
        CloseHandle(g_hFileMap);
        g_hFileMap = NULL;
    }

    DEBUGMSG(ZONE_INIT, (_T("DmacDealloc- (rc = %d)\r\n"), TRUE));

    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function:  DmacTranslateCfg
//
// This function is used to translate the input channel configuration into 
// DMAC register values.
//
// Parameters:
//      pChannelCfg
//          [in] - channel configuration.
//
//      pChannelData
//          [out] - DMAC channel register values
//
// Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//------------------------------------------------------------------------------
static BOOL DmacTranslateCfg(DMAC_CHANNEL_CFG *pChannelCfg, DMAC_CHANNEL_DATA *pChannelData)
{
    BOOL rc = FALSE;
    
    DEBUGMSG(ZONE_FUNCTION, (_T("DmacTranslateCfg+\r\n")));
    
    // Translate configuration for CCR
    switch(pChannelCfg->RepeatType)
    {
        case DMAC_REPEAT_DISABLED:
            pChannelData->CCR = CSP_BITFVAL(DMAC_CCR_ACRPT, DMAC_CCR_ACRPT_DISABLE) |
                                CSP_BITFVAL(DMAC_CCR_RPT, DMAC_CCR_RPT_DISABLE);
            break;

        case DMAC_REPEAT_ONCE:
            pChannelData->CCR = CSP_BITFVAL(DMAC_CCR_ACRPT, DMAC_CCR_ACRPT_AUTOCLEAR_RPT) |
                                CSP_BITFVAL(DMAC_CCR_RPT, DMAC_CCR_RPT_ENABLE);
            break;

        case DMAC_REPEAT_FOREVER:
            pChannelData->CCR =  CSP_BITFVAL(DMAC_CCR_ACRPT, DMAC_CCR_ACRPT_DISABLE) |
                                 CSP_BITFVAL(DMAC_CCR_RPT, DMAC_CCR_RPT_ENABLE);
            break;
            
        default:
            DEBUGMSG(ZONE_ERROR, (_T("DmacTranslateCfg: Invalid repeat type\r\n")));
            goto cleanUp;
    }
    
    if((pChannelCfg->DstMode >= DMAC_TRANSFER_MODE_LINEAR_MEMORY) && (pChannelCfg->DstMode <= DMAC_TRANSFER_MODE_EOBE)  &&
       (pChannelCfg->SrcMode >= DMAC_TRANSFER_MODE_LINEAR_MEMORY) && (pChannelCfg->SrcMode <= DMAC_TRANSFER_MODE_EOBE) &&
       (pChannelCfg->DstSize >= DMAC_TRANSFER_SIZE_32BIT) && (pChannelCfg->DstSize <= DMAC_TRANSFER_SIZE_16BIT) &&
       (pChannelCfg->SrcSize >= DMAC_TRANSFER_SIZE_32BIT) && (pChannelCfg->SrcSize <= DMAC_TRANSFER_SIZE_16BIT) )
            pChannelData->CCR |= CSP_BITFVAL(DMAC_CCR_DMOD, pChannelCfg->DstMode) |
                                 CSP_BITFVAL(DMAC_CCR_SMOD, pChannelCfg->SrcMode) |
                                 CSP_BITFVAL(DMAC_CCR_DSIZ, pChannelCfg->DstSize) |
                                 CSP_BITFVAL(DMAC_CCR_SSIZ, pChannelCfg->SrcSize) ;
    else
    {
        DEBUGMSG(ZONE_ERROR, (_T("DmacTranslateCfg: Invalid dest/src configuration\r\n")));
        goto cleanUp;
    }
    
    if (pChannelCfg->MemDirIncrease)
        pChannelData->CCR |= CSP_BITFVAL(DMAC_CCR_MDIR, DMAC_CCR_MDIR_INCREMENT);
    else
        pChannelData->CCR |= CSP_BITFVAL(DMAC_CCR_MDIR, DMAC_CCR_MDIR_DECREMENT);

    
    // BUCR & RTOR share the same address. Which register is written depends on 
    // whether ext request enable (REN) is set in CCR.
    
    if (pChannelCfg->ExtReqEnable)
    {
        pChannelData->CCR |= CSP_BITFVAL(DMAC_CCR_REN, DMAC_CCR_REN_ENABLE) |
                             CSP_BITFVAL(DMAC_CCR_FRC, DMAC_CCR_FRC_NOEFFECT);

        pChannelData->RSSR = CSP_BITFVAL(DMAC_RSSR_RSS, pChannelCfg->ReqSrc);

        // Set source clock of request time out counter is HCLK
        if (pChannelCfg->ReqTimeout)
            pChannelData->RTOR_BUCR = CSP_BITFVAL(DMAC_RTOR_EN, DMAC_RTOR_EN_ENABLE) |
                             CSP_BITFVAL(DMAC_RTOR_CLK, DMAC_RTOR_CLK_HCLK) |
                             CSP_BITFVAL(DMAC_RTOR_PSC, 0) |
                             CSP_BITFVAL(DMAC_RTOR_CNT, pChannelCfg->ReqTOCounter);
        else
            pChannelData->RTOR_BUCR = 0;
    }
    else
    {
        pChannelData->CCR |= CSP_BITFVAL(DMAC_CCR_REN, DMAC_CCR_REN_DISABLE) |
                             CSP_BITFVAL(DMAC_CCR_FRC, DMAC_CCR_FRC_FORCE_DMA_CYCLE);

        pChannelData->RSSR = 0; // default

        pChannelData->RTOR_BUCR = CSP_BITFVAL(DMAC_BUCR_BU_CNT, pChannelCfg->BusClkCounter);
    }

    // Translate other registers data
    pChannelData->SAR = pChannelCfg->SrcAddr;
    pChannelData->DAR = pChannelCfg->DstAddr;
    pChannelData->CNTR = pChannelCfg->DataSize;
    pChannelData->BLR = CSP_BITFVAL(DMAC_BLR_BL, pChannelCfg->BurstLength);

    rc = TRUE;

cleanUp:
    DEBUGMSG(ZONE_FUNCTION, (_T("DmacTranslateCfg- (rc = %d)\r\n"), rc));
    return rc;
}

//-----------------------------------------------------------------------------
//
// Function:  DmacShowRegister
//
// This function prints out the values in the DMAC registers.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
static void DmacShowRegister(UINT32 channelMask)
{   
    UINT32 mask;
    UINT8 i;

    DEBUGMSG(1, (TEXT("DMAC General Registers:\r\n")));
    DEBUGMSG(1, (TEXT("DCR(0x%08x)\r\n"), g_pDMAC->DCR));
    DEBUGMSG(1, (TEXT("DISR(0x%08x)\r\n"), g_pDMAC->DISR));
    DEBUGMSG(1, (TEXT("DIMR(0x%08x)\r\n"), g_pDMAC->DIMR));
    DEBUGMSG(1, (TEXT("DRTOSR(0x%08x)\r\n"), g_pDMAC->DRTOSR));
    DEBUGMSG(1, (TEXT("DBOSR(0x%08x)\r\n"), g_pDMAC->DBOSR));
    DEBUGMSG(1, (TEXT("DBTOCR(0x%08x)\r\n"), g_pDMAC->DBTOCR));
    DEBUGMSG(1, (TEXT("2D memory registers:\r\n")));
    DEBUGMSG(1, (TEXT("WSRA(0x%08x)\r\n"), g_pDMAC->WSRA));
    DEBUGMSG(1, (TEXT("XSRA(0x%08x)\r\n"), g_pDMAC->XSRA));
    DEBUGMSG(1, (TEXT("YSRA(0x%08x)\r\n"), g_pDMAC->YSRA));
    DEBUGMSG(1, (TEXT("WSRB(0x%08x)\r\n"), g_pDMAC->WSRB));
    DEBUGMSG(1, (TEXT("XSRB(0x%08x)\r\n"), g_pDMAC->XSRB));
    DEBUGMSG(1, (TEXT("YSRB(0x%08x)\r\n"), g_pDMAC->YSRB));

    for(i = 0, mask = 1; i <= DMAC_NUM_CHANNELS; i++, mask <<= 1)
    {
        if(mask & channelMask)
        {
            DEBUGMSG(1, (TEXT("Channel %d:\r\n"), i));
            DEBUGMSG(1, (TEXT("SAR(0x%08x)\r\n"), g_pDMAC->CHANNEL[i].SAR));
            DEBUGMSG(1, (TEXT("DAR(0x%08x)\r\n"), g_pDMAC->CHANNEL[i].DAR));
            DEBUGMSG(1, (TEXT("CNTR(0x%08x)\r\n"), g_pDMAC->CHANNEL[i].CNTR));
            DEBUGMSG(1, (TEXT("CCR(0x%08x)\r\n"), g_pDMAC->CHANNEL[i].CCR));
            DEBUGMSG(1, (TEXT("RSSR(0x%08x)\r\n"), g_pDMAC->CHANNEL[i].RSSR));
            DEBUGMSG(1, (TEXT("BLR(0x%08x)\r\n"), g_pDMAC->CHANNEL[i].BLR));
            DEBUGMSG(1, (TEXT("RTOR_BUCR(0x%08x)\r\n"), g_pDMAC->CHANNEL[i].RTOR_BUCR));
            DEBUGMSG(1, (TEXT("CCNR(0x%08x)\r\n"), g_pDMAC->CHANNEL[i].CCNR));
        }
    }
}


