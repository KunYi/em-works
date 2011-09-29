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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
#include <windows.h>
#include <winuser.h>
#include <winuserm.h>
#include <pm.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <omap2420.h>
#include "dmasys.h"

#define DMASYSCONT_NUMCHANNELS 32

SysStandardDMAController::SysStandardDMAController(void) : StandardDMAController(DMASYSCONT_NUMCHANNELS)
{
    mpDMARegs = NULL;
}

SysStandardDMAController::~SysStandardDMAController(void)
{
    if (mpDMARegs)
        ShutDown();
}

uint SysStandardDMAController::Init(void)
{
    PHYSICAL_ADDRESS pa;

    // map standard controller DMA registers into driver address space
    pa.HighPart= 0;
    pa.LowPart = OMAP2420_SDMA_REGS_PA;
    mpDMARegs = (OMAP2420_SDMA_REGS *)MmMapIoSpace(pa, sizeof(OMAP2420_SDMA_REGS), FALSE);
    if (!mpDMARegs)
    {
        ERRORMSG(TRUE, (L"DMA\\SysStandardDMAController::Init(): ERROR mapping DMA registers.\r\n"));
        return DMADRVERR_INIT_FAILURE;
    }

    /* try to retrieve the DMA controller revision here */
    uint major, minor;
    if (!GetRevision(mpDMARegs,major,minor))
    {
        MmUnmapIoSpace((PVOID)mpDMARegs,sizeof(OMAP2420_SDMA_REGS));
        mpDMARegs = NULL;
        return DMADRVERR_INIT_FAILURE;
    }
    DEBUGMSG(ZONE_INIT, (L"DMA\\SysStandardDMAController::Init() "
        L"Revision %d.%d\r\n", major, minor
    ));

    Reset(mpDMARegs);
    return 0;
}

void SysStandardDMAController::ShutDown(void)
{
    if (mpDMARegs)
    {
        Reset(mpDMARegs);
        /* unmap the controller */
        MmUnmapIoSpace((PVOID)mpDMARegs,sizeof(OMAP2420_SDMA_REGS));
        mpDMARegs = NULL;
    }
}

uint SysStandardDMAController::Set(uint aChanMask, DMA_CONT_PROPERTY aProp, uint aValue)
{
    switch (aProp)
    {
        case DMACP_L0IntEnb:
            aValue &= aChanMask;
            if (!aValue)
                return DMADRVERR_PARAM_INVALID;
            SETREG32(&mpDMARegs->DMA4_IRQENABLE_L0, aValue);
            break;
        case DMACP_L0IntDis:
            aValue &= aChanMask;
            if (!aValue)
                return DMADRVERR_PARAM_INVALID;
            CLRREG32(&mpDMARegs->DMA4_IRQENABLE_L0, aValue);
            break;
        case DMACP_L0IntAck:
            aValue &= aChanMask;
            if (!aValue)
                return DMADRVERR_PARAM_INVALID;
            SETREG32(&mpDMARegs->DMA4_IRQSTATUS_L0, aValue);
            break; 
            
        case DMACP_L3IntEnb:
            aValue &= aChanMask;
            if (!aValue)
                return DMADRVERR_PARAM_INVALID;
            SETREG32(&mpDMARegs->DMA4_IRQENABLE_L3, aValue);
            break;
        case DMACP_L3IntDis:
            aValue &= aChanMask;
            if (!aValue)
                return DMADRVERR_PARAM_INVALID;
            CLRREG32(&mpDMARegs->DMA4_IRQENABLE_L3, aValue);
            break;
        case DMACP_L3IntAck:
            aValue &= aChanMask;
            if (!aValue)
                return DMADRVERR_PARAM_INVALID;
            SETREG32(&mpDMARegs->DMA4_IRQSTATUS_L3, aValue);
            break;

        case DMACP_L2IntEnb:
            aValue &= aChanMask;
            if (!aValue)
                return DMADRVERR_PARAM_INVALID;
            SETREG32(&mpDMARegs->DMA4_IRQENABLE_L2, aValue);
            break;
        case DMACP_L2IntDis:
            aValue &= aChanMask;
            if (!aValue)
                return DMADRVERR_PARAM_INVALID;
            CLRREG32(&mpDMARegs->DMA4_IRQENABLE_L2, aValue);
            break;
        case DMACP_L2IntAck:
            aValue &= aChanMask;
            if (!aValue)
                return DMADRVERR_PARAM_INVALID;
            SETREG32(&mpDMARegs->DMA4_IRQSTATUS_L2, aValue);
            break;
            
        default:
            return DMADRVERR_UNKNOWNPROPERTY;
    }

    return 0;
}

uint SysStandardDMAController::Get(uint aChanMask, DMA_CONT_PROPERTY aProp, uint *apRetValue)
{
    switch (aProp)
    {
        case DMACP_L0IntEnb:
        case DMACP_L0IntDis:
        case DMACP_L0IntAck:
        case DMACP_L3IntEnb:
        case DMACP_L3IntDis:
        case DMACP_L3IntAck:
        case DMACP_L2IntEnb:
        case DMACP_L2IntDis:
        case DMACP_L2IntAck:
            return DMADRVERR_PROP_WRITEONLY;
    }
    return DMADRVERR_UNKNOWNPROPERTY;
}
