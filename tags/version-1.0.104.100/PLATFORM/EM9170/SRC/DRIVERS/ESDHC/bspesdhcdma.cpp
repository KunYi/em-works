//-----------------------------------------------------------------------------
//
// Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  bspesdhcdma.cpp
//
//  Provides BSP-specific SDMA routines for use by ESDHC driver.
//
//------------------------------------------------------------------------------

#include "bsp.h"
#include "esdhc.h"
#include "esdhcdma.hpp"


//------------------------------------------------------------------------------
// External Functions


//------------------------------------------------------------------------------
// External Variables


//------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Global Variables

//------------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
// Local Functions
CESDHCBaseEDMA::CESDHCBaseEDMA(CESDHCBase & SDHCSlotBase) : CESDHCBaseDMA(SDHCSlotBase)
{
}

CESDHCBaseEDMA::~CESDHCBaseEDMA()
{
}

BOOL CESDHCBaseEDMA::BspEDMAInit()
{
    return FALSE;
}


BOOL CESDHCBaseEDMA::BspEDMAArm(SD_BUS_REQUEST& Request, BOOL fToDevice)
{
    UNREFERENCED_PARAMETER(Request);
    UNREFERENCED_PARAMETER(fToDevice);
    return FALSE;
}

BOOL CESDHCBaseEDMA::BspEDMANotifyEvent(SD_BUS_REQUEST & Request, DMAEVENT dmaEvent)
{
    UNREFERENCED_PARAMETER(Request);
    UNREFERENCED_PARAMETER(dmaEvent);
    return FALSE;
}

DMAEVENT CESDHCBaseEDMA::BspEDMACheckCompletion(BOOL fToDevice)
{
    DMAEVENT dmaEvent = NO_DMA;

    UNREFERENCED_PARAMETER(fToDevice);

    return dmaEvent;
}
