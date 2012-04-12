//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  bsputils.c
//
//  This module is provides the common routine for BSP.
//
//-----------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115)
#include <windows.h>
#include <nkintr.h>
#pragma warning(pop)

#include "bsp.h"

static PVOID pv_HWregRTC = NULL;
static PVOID pv_HWregDIGCTL = NULL;

//------------------------------------------------------------------------------
//
// Function: OALStall
//
// This function stalls CPU for the time defined in parameters. The unit
// is 1 microsecond.
//
// Parameters:
//      uSecs
//          [in] The microseconds to stall.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
VOID OALStall(UINT32 microSec)
{
    UINT32 expireTime, currentTime;
    if(pv_HWregDIGCTL == NULL)
    {
        pv_HWregDIGCTL = (PVOID)OALPAtoVA(CSP_BASE_REG_PA_DIGCTL, FALSE);

        //enable the digital Control Microseconds counter
        HW_DIGCTL_CTRL.B.XTAL24M_GATE = 0;
    }

    currentTime = HW_DIGCTL_MICROSECONDS_RD();
    expireTime = currentTime + microSec;

    //
    // Check if we wrapped on the expireTime
    // and delay first part until wrap
    //
    if (expireTime < currentTime)
    {
        while (currentTime < HW_DIGCTL_MICROSECONDS_RD()) ;
    }
    while (HW_DIGCTL_MICROSECONDS_RD() <= expireTime) ;
}

//-----------------------------------------------------------------------------
//
// Function: OEMKitlGetSecs
//
// This function returns a free-running seconds count, which is mostly used
// by debug Ethernet routines (e.g. OEMEthGetFrame) to implement timeouts.
//
// Parameters:
//      None.
//
// Returns:
//      The seconds since RTC runs.
//
//-----------------------------------------------------------------------------
DWORD OEMKitlGetSecs(void)
{
    // MAP the Hardware registers
    if(pv_HWregRTC == NULL)
    {
        pv_HWregRTC = (PVOID)OALPAtoVA(CSP_BASE_REG_PA_RTC, FALSE);
    }
    return HW_RTC_SECONDS_RD();
}
//------------------------------------------------------------------------------
//
//  Function:  OALGetTickCount
//
//  This function is called by some KITL libraries to obtain relative time
//  since device boot. It is mostly used to implement timeout in network
//  protocol.
//------------------------------------------------------------------------------
UINT32 OALGetTickCount()
{
    // MAP the Hardware registers
    if(pv_HWregRTC == NULL)
    {
        pv_HWregRTC = (PVOID)OALPAtoVA(CSP_BASE_REG_PA_RTC, FALSE);
    }
    return OEMKitlGetSecs () * 1000;
}

//------------------------------------------------------------------------------
