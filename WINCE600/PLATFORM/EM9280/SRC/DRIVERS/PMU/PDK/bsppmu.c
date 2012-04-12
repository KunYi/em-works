//---------------------------------------------------------------------------
//
//  Copyright (C) 2007-2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  bsppmu.c
//
//  Provides USB device driver present or not.
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#pragma warning(pop)

#include "bsp.h"

//-----------------------------------------------------------------------------
// External Functions


//-----------------------------------------------------------------------------
// External Variables

//-----------------------------------------------------------------------------
// Types
//-----------------------------------------------------------------------------
// Global Variables

//-----------------------------------------------------------------------------
//
// Function: IsUSBDeviceDriverEnable
//
// This function returns whether USB device driver enable.
//
// Parameters:
//        
//
// Returns:
//      Whether USB device driver ia enable.
//
//-----------------------------------------------------------------------------
BOOL IsUSBDeviceDriverEnable()
{
#ifdef BSP_USB_DEVICE
    return TRUE;
#else
    return FALSE;
#endif
}
//-----------------------------------------------------------------------------
//
// Function: Is5VFromVbus
//
// This function returns whether 5V is sourced from VBUS.
//
// Parameters:
//        
//
// Returns:
//
//-----------------------------------------------------------------------------
BOOL Is5VFromVbus()
{
#ifdef BSP_5V_FROM_VBUS
    return TRUE;
#else
    return FALSE;
#endif
}

