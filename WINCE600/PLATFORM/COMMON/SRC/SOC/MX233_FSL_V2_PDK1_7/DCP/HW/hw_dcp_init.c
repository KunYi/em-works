//-----------------------------------------------------------------------------
//
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File: hw_dcp_init.c
//  Brief data co-processor interface
//
//
/////////////////////////////////////////////////////////////////////////////////
#pragma warning(push)
#pragma warning(disable: 4115)
#include <windows.h>
#pragma warning(pop)
#pragma warning(push)
#pragma warning(disable: 4214)
#include <ceddk.h>
#pragma warning(pop)
#include "hw_dcp.h"

HRESULT dcp_hw_Reset(void);

//-----------------------------------------------------------------------------
// Global Variables
PVOID pv_HWregDCP = NULL;

HRESULT dcp_hw_Initialize()
{
    INT32 i;
    
    if (pv_HWregDCP == NULL)
    {
        PHYSICAL_ADDRESS phyAddr;

        //Mem maps the LCDIF module space for access
        phyAddr.QuadPart = CSP_BASE_REG_PA_DCP;
        pv_HWregDCP = (PVOID)MmMapIoSpace(phyAddr, 0x1000, FALSE);
    }

    dcp_hw_Reset();
    dcp_hw_GatherResidualWrites(TRUE);
    dcp_hw_ContextCaching(TRUE);

    // Make sure all of the interrupts are clear
    for (i = 0; i < DCP_MAX_CHANNELS; i++)
    {
        dcp_hw_ChannelInterruptClear(i);
    }
    dcp_hw_CSCInterruptClear();
    dcp_hw_CSCInterruptEnable(TRUE);

    return ERROR_SUCCESS;
}

