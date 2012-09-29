//-----------------------------------------------------------------------------
//
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File: hw_dcp.c
//
//  Brief data co-processor interface
//
//-----------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115)
#include <windows.h>
#pragma warning(pop)

#include "common_macros.h"
#include "common_dcp.h"

extern PVOID pv_HWregDCP;

//------------------------------------------------------------------------------
//
// Function : dcp_hw_RunChannel
//
// Function for run DCP channel
//
//  Parameters:
//      Channel
//          [in] DCP channel number
//
//      Packets
//          [in] DCP work packet address
//
//      NumberOfPackets
//          [in] number of DCP work packet
//
// Returns:
//      ERROR_SUCCESS   successful
//      Other                     failed
//
//------------------------------------------------------------------------------
RtResult dcp_hw_RunChannel(UINT32 Channel,  DCPWorkPacket_t *Packets, UINT32 NumberOfPackets)
{
    RtResult Status;

    if (Channel < DCP_MAX_CHANNELS)
    {
        HW_DCP_CHnCMDPTR_WR(Channel, (reg32_t)Packets);
        HW_DCP_CHnSEMA_WR(Channel, NumberOfPackets);
        Status = ERROR_SUCCESS;
    }
    else
    {
        Status = ERROR_INVALID_DATA;
        ERRORMSG (1, (L"DCP Invalid Channel!\r\n"));
        ASSERT(0);
    }

    return Status;

}

//------------------------------------------------------------------------------
//
// Function : dcp_hw_GatherResidualWrites
//
// Function to enable ragged writes to unaligned buffers to be gathered 
// between multiple write operations.
//
// This improves performance by removing several byte operations
// between write bursts. Trailing byte writes are held in a residual
// write data buffer and combined with a subsequent write to the
// buffer to form a word write.
//
//  Parameters:
//      Enable
//          [in] whether to enable gather residual write
//
// Returns:
//      ERROR_SUCCESS   successful
//
//------------------------------------------------------------------------------
RtResult dcp_hw_GatherResidualWrites(BOOL Enable)
{
    if (Enable == FALSE)
    {
        HW_DCP_CTRL_CLR(BM_DCP_CTRL_GATHER_RESIDUAL_WRITES);
    }
    else
    {
        HW_DCP_CTRL_SET(BM_DCP_CTRL_GATHER_RESIDUAL_WRITES);
    }

    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function : dcp_hw_ContextCaching
//
// Function to enable caching of contexts between operations.
//
// If only a single channel is used for encryption/hashing, 
// enabling caching causes the context to not
// be reloaded if the channel was the last to be used.
//
//  Parameters:
//      Enable
//          [in] Whether to enable caching of contexts between operations
//
// Returns:
//      ERROR_SUCCESS   successful
//
//------------------------------------------------------------------------------
RtResult dcp_hw_ContextCaching(BOOL Enable)
{
    if (Enable == FALSE)
    {
        HW_DCP_CTRL_CLR(BM_DCP_CTRL_ENABLE_CONTEXT_CACHING);
    }
    else
    {
        HW_DCP_CTRL_SET(BM_DCP_CTRL_ENABLE_CONTEXT_CACHING);
    }

    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function : dcp_hw_Reset
//
// Function to reset DCP module.
//
//  Parameters:
//      None.
//
// Returns:
//      ERROR_SUCCESS   successful
//
//------------------------------------------------------------------------------
RtResult dcp_hw_Reset()
{
    // reset the blocks and leave them
    // running.
    HW_DCP_CTRL_SET(BM_DCP_CTRL_SFTRST);
    while(!HW_DCP_CTRL.B.CLKGATE);
    HW_DCP_CTRL_CLR(BM_DCP_CTRL_SFTRST | BM_DCP_CTRL_CLKGATE);

    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function : dcp_hw_ContextSwitching
//
// Function to enable automatic context switching for the channels.
//
// Software should set this bit if more than one channel is doing hashing or
// cipher operations that require context to be saved (for instance,
// when CBC mode is enabled). By disabling context switching,
// software can save the 208 bytes used for the context buffer.
//
//  Parameters:
//      Enable.
//          [in] Whether to enable automatic context switching for the channels
//
//      Buffer
//          [in] Context pointer address.
//
// Returns:
//      ERROR_SUCCESS   successful
//
//------------------------------------------------------------------------------
RtResult dcp_hw_ContextSwitching(BOOL Enable, void *Buffer)
{
    if (Enable == FALSE)
    {
        HW_DCP_CTRL_CLR(BM_DCP_CTRL_ENABLE_CONTEXT_SWITCHING);

        // Give the context buffer pointer an illegal address so if context switching
        // is inadvertantly enabled, the dcp will return an error instead of trashing
        // good memory. The dcp dma cannot access rom, so any rom address will do.
        HW_DCP_CONTEXT_WR(0xFFFF0000);
    }
    else
    {
        HW_DCP_CTRL_SET(BM_DCP_CTRL_ENABLE_CONTEXT_SWITCHING);
        HW_DCP_CONTEXT_WR((reg32_t)Buffer);
    }

    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function : dcp_hw_IsEnabled
//
// Function to check whether DCP is enable
//
//  Parameters:
//      None.
//
// Returns:
//      TRUE   DCP is enabled
//      FALSE   DCP is disabled
//
//------------------------------------------------------------------------------
BOOL dcp_hw_IsEnabled()
{
    UINT32 Value;

    Value = BM_DCP_CTRL_SFTRST | BM_DCP_CTRL_CLKGATE;

    if ((HW_DCP_CTRL_RD() & Value) != 0)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

//------------------------------------------------------------------------------
//
// Function : dcp_hw_Enable
//
// Function to enable DCP module.
//
//  Parameters:
//      Enable.
//          [in] Whether to enable DCP
//
// Returns:
//      ERROR_SUCCESS   successful
//
//------------------------------------------------------------------------------
RtResult dcp_hw_Enable(BOOL Enable)
{
    if (Enable == TRUE)
    {
        HW_DCP_CTRL_CLR(BF_DCP_CTRL_CLKGATE(1));
    }
    else
    {
        HW_DCP_CTRL_SET(BF_DCP_CTRL_CLKGATE(1));
    }

    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function : dcp_hw_ChannelEnable
//
// Function to enable enabled the DMA channel associated with it.
//
// When not enabled, the channel is denied access to the
// central DMA resources.
//
//  Parameters:
//      Channel.
//          [in] Channel to enable
//
//      Enable
//          [in] Whether to enable channek.
//
// Returns:
//      ERROR_SUCCESS   successful
//      Other                     failed
//
//------------------------------------------------------------------------------
RtResult dcp_hw_ChannelEnable(UINT32 Channel, BOOL Enable)
{
    RtResult Status;

    if (Channel < DCP_MAX_CHANNELS)
    {
        if (Enable)
        {
            HW_DCP_CHANNELCTRL_SET(1 << Channel);
        }
        else
        {
            HW_DCP_CHANNELCTRL_CLR(1 << Channel);
        }
        Status = ERROR_SUCCESS;
    }
    else
    {
        Status = ERROR_INVALID_ACCESS;
        ERRORMSG (1, (L"DCP Invalid Channel!\r\n"));
        ASSERT(0);
    }

    return Status;
}

//------------------------------------------------------------------------------
//
// Function : dcp_hw_ChannelInterruptEnable
//
// Function to enable channel interrupt.
//
//  Parameters:
//      Channel.
//          [in] Channel to enable interrupt.
//
//      Enable
//          [in] Whether to enable interrupt
//
// Returns:
//      ERROR_SUCCESS   successful
//      Other                     failed
//
//------------------------------------------------------------------------------
RtResult dcp_hw_ChannelInterruptEnable(UINT32 Channel, BOOL Enable)
{
    RtResult Status;

    if (Channel < DCP_MAX_CHANNELS)
    {
        if (Enable)
        {
            HW_DCP_CTRL_SET(1 << Channel);
        }
        else
        {
            HW_DCP_CTRL_CLR(1 << Channel);
        }
        Status = ERROR_SUCCESS;
    }
    else
    {
        Status = ERROR_INVALID_ACCESS;
        ERRORMSG (1, (L"DCP Invalid Channel!\r\n"));
        ASSERT(0);
    }

    return Status;
}

//------------------------------------------------------------------------------
//
// Function : dcp_hw_ChannelInterruptClear
//
// Function to clear channel interrupt status.
//
//  Parameters:
//      Channel.
//          [in] Channel to clear interrupt status.
//
// Returns:
//      ERROR_SUCCESS   successful
//      Other                     failed
//
//------------------------------------------------------------------------------
RtResult dcp_hw_ChannelInterruptClear(UINT32 Channel)
{
    RtResult Status = ERROR_SUCCESS;;

    if (Channel < DCP_MAX_CHANNELS)
    {
        HW_DCP_STAT_CLR(1 << Channel);
    }
    else
    {
        Status = ERROR_INVALID_DATA;
        ERRORMSG (1, (L"DCP Invalid Channel!\r\n"));
        ASSERT(0);
    }

    return Status;
}

//------------------------------------------------------------------------------
//
// Function : dcp_hw_VMIMergeIRQ
//
// Function to Indicates that the interrupt for channel 0 should be merged with
// the other interrupts on the shared dcp_irq interrupt.
//
//  Parameters:
//      Merge.
//          [in] When set to FALSE, channel 0's interrupt will be routed to the separate
//              dcp_vmi_irq. When set to TRUE, the interrupt will be routed to the 
//              shared DCP interrupt.
//
// Returns:
//      ERROR_SUCCESS   successful
//
//------------------------------------------------------------------------------
RtResult dcp_hw_VMIMergeIRQ(BOOL Merge)
{
    if (Merge)
    {
        HW_DCP_CHANNELCTRL_SET(BM_DCP_CHANNELCTRL_CH0_IRQ_MERGED);
    }
    else
    {
        HW_DCP_CHANNELCTRL_CLR(BM_DCP_CHANNELCTRL_CH0_IRQ_MERGED);
    }

    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function : dcp_hw_SetChannelHighPriority
//
// Function to set  the corresponding channel to have high-priority arbitration
//
// High priority channels will be arbitrated round-robin and will take precedence
// over other channels that are not marked as high priority.
//
//  Parameters:
//      Channel.
//          [in] Channel to set priority
//
//      set
//          [in] Whether to set high priority
//
// Returns:
//      ERROR_SUCCESS   successful
//      Other                     failed
//
//------------------------------------------------------------------------------
RtResult dcp_hw_SetChannelHighPriority(UINT32 Channel, BOOL Set)
{
    UINT32 Bit;
    RtResult Status;

    if(Channel >= DCP_MAX_CHANNELS)
    {
        Status = ERROR_INVALID_DATA;
        ERRORMSG (1, (L"DCP Invalid Channel!\r\n"));
        ASSERT(0);
        return Status;
    }

    Status = ERROR_SUCCESS;

    Bit = 1 << Channel;

    if (Set)
    {
        HW_DCP_CHANNELCTRL_SET(BF_DCP_CHANNELCTRL_HIGH_PRIORITY_CHANNEL(Bit));
    }
    else
    {
        HW_DCP_CHANNELCTRL_CLR(BF_DCP_CHANNELCTRL_HIGH_PRIORITY_CHANNEL(Bit));
    }

    return ERROR_SUCCESS;
}
