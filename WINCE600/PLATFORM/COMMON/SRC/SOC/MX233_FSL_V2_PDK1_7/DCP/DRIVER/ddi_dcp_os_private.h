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

#include "ddi_dcp.h"

#pragma warning(push)
#pragma warning(disable: 4214)
#include <ceddk.h>
#pragma warning(pop)

#define virt2phys(addr) (void*) (((UINT32) (addr) - (UINT32) g_DCPChannel) + (UINT32) dcp_phys_address.LowPart)

    extern PHYSICAL_ADDRESS dcp_phys_address;

    void dcp_isr_Init(void);
    HRESULT dcp_os_Init(DCPChannel_t* Channels, int ChannelCount);

    HRESULT dcp_csc_GetSemaphore(UINT32 TimeOut);
    HRESULT dcp_csc_PutSemaphore(void);
    int dcp_csc_GetSemaphoreCount(void);

    HRESULT dcp_PutSemaphore(void);
    HRESULT dcp_GetSemaphore(void);
    HRESULT dcp_GetChannelSemaphore(DCPChannel_t* Channel, UINT32 TimeOut);
    HRESULT dcp_PutChannelSemaphore(DCPChannel_t* Channel);
    int dcp_GetSemaphoreCount(void);
    
    HRESULT dcp_AcquireMutex(void);
    HRESULT dcp_ReleaseMutex(void);

#ifdef __cplusplus
} //extern "C"
#endif

#endif /* _DDI_DCP_OS_H */
