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
/////////////////////////////////////////////////////////////////////////////////
#pragma warning(push)
#pragma warning(disable: 4115)
#include <windows.h>
#pragma warning(pop)

#include "csp.h"
#include "hw_dcp.h"

extern PVOID pv_HWregDCP;

HRESULT dcp_hw_RunChannel(UINT32 Channel,  DCPWorkPacket_t *Packets, UINT32 NumberOfPackets)
{
    HRESULT Status;

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

HRESULT dcp_hw_GatherResidualWrites(BOOL Enable)
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

HRESULT dcp_hw_ContextCaching(BOOL Enable)
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

HRESULT dcp_hw_Reset()
{
    // reset the blocks and leave them
    // running.
    HW_DCP_CTRL_SET(BM_DCP_CTRL_SFTRST);
    while(!HW_DCP_CTRL.B.CLKGATE);
    HW_DCP_CTRL_CLR(BM_DCP_CTRL_SFTRST | BM_DCP_CTRL_CLKGATE);

    return ERROR_SUCCESS;
}

HRESULT dcp_hw_ContextSwitching(BOOL Enable, void *Buffer)
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

HRESULT dcp_hw_Enable(BOOL Enable)
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

HRESULT dcp_hw_ChannelEnable(UINT32 Channel, BOOL Enable)
{
    HRESULT Status;

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

HRESULT dcp_hw_ChannelInterruptEnable(UINT32 Channel, BOOL Enable)
{
    HRESULT Status;

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

HRESULT dcp_hw_ChannelInterruptClear(UINT32 Channel)
{
    HRESULT Status = ERROR_SUCCESS;;

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

HRESULT dcp_hw_VMIMergeIRQ(BOOL Merge)
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

HRESULT dcp_hw_SetChannelHighPriority(UINT32 Channel, BOOL Set)
{
    UINT32 Bit;
    HRESULT Status;

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
