//-----------------------------------------------------------------------------
//
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File: ddi_dcp_os_private.h
//  Brief data co-processor interface for dcp channel acess and synchronization
//
//
/////////////////////////////////////////////////////////////////////////////////
#ifndef _DDI_DCP_OS_H
#define _DDI_DCP_OS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common_macros.h"
#include "common_dcp.h"

#pragma warning(push)
#pragma warning(disable: 4214)
#include <ceddk.h>
#pragma warning(pop)

#define virt2phys(addr) (void*) (((UINT32) (addr) - (UINT32) g_DCPChannel) + (UINT32) dcp_phys_address.LowPart)

    extern PHYSICAL_ADDRESS dcp_phys_address;

    void dcp_isr_Init(void);
    RtResult dcp_os_Init(DCPChannel_t* Channels, int ChannelCount);

    RtResult dcp_PutSemaphore(void);
    RtResult dcp_GetSemaphore(void);
    RtResult dcp_GetChannelSemaphore(DCPChannel_t* Channel, UINT32 TimeOut);
    RtResult dcp_PutChannelSemaphore(DCPChannel_t* Channel);
    int dcp_GetSemaphoreCount(void);
    
    RtResult dcp_AcquireMutex(void);
    RtResult dcp_ReleaseMutex(void);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* _DDI_DCP_OS_H */
