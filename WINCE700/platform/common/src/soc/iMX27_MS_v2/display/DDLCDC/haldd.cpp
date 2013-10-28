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
//-----------------------------------------------------------------------------
//  Copyright (C) 2004-2005, MOTOROLA, INC. All Rights Reserved
//-----------------------------------------------------------------------------
//---------------------------------------------------------------------------
//  Copyright (C) 2005-2006, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//---------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  File:  haldd.cpp
//
//  DirectDraw Display Driver Callback Functions.
//
//------------------------------------------------------------------------------

#include "precomp.h"


//------------------------------------------------------------------------------
// External Functions


//------------------------------------------------------------------------------
// External Variables


//------------------------------------------------------------------------------
// Defines


//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
// Local Functions


//------------------------------------------------------------------------------
// Global Variables


//------------------------------------------------------------------------------
// Global Functions

//------------------------------------------------------------------------------
//
// Function: HalGetDriverInfo
//
// This function is used to get further DirectDraw 
// hardware abstraction layer (DDHAL) information after buildDDHALInfo(). 
//
// Parameters:
//      lpInput
//          [in, out] Pointer to a DDHAL_GETDRIVERINFODATA structure 
//          that contains the driver-specific information.
//
// Returns:
//      DDHAL_DRIVER_HANDLED.
//
//------------------------------------------------------------------------------
DWORD WINAPI HalGetDriverInfo(LPDDHAL_GETDRIVERINFODATA lpInput)
{
    DEBUGMSG(HAL_ZONE_INIT,(TEXT("GetDriverInfo invoked !!\r\n")));

    lpInput->ddRVal = DDERR_CURRENTLYNOTAVAIL;

    return DDHAL_DRIVER_HANDLED;
}


//------------------------------------------------------------------------------
// DDHAL_DDCALLBACKS

//------------------------------------------------------------------------------
//
// Function: HalWaitForVerticalBlank
//
// This callback function helps the application synchronize 
// itself with the vertical blanking interval (VBI).
//
// Parameters:
//      pd
//          [in, out] Pointer to a DDHAL_WAITFORVERTICALBLANKDATA structure
//          that contains the vertical blank information. 
//
// Returns:
//      Returns one of the following values: 
//          DDHAL_DRIVER_HANDLED 
//          DDHAL_DRIVER_NOTHANDLED 
//
//------------------------------------------------------------------------------
DWORD WINAPI HalWaitForVerticalBlank( LPDDHAL_WAITFORVERTICALBLANKDATA pd )
{
    DEBUGENTER( HalWaitForVerticalBlank );

    return ((MX27DDLcdc *)GetDDGPE())->WaitForVerticalBlank(pd);
}
