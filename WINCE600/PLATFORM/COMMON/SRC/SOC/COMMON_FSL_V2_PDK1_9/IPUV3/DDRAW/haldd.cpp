//-----------------------------------------------------------------------------
//
//  Copyright (C) 2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
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
extern unsigned char * g_pVideoMemory;

//------------------------------------------------------------------------------
// Defines
#ifdef DEBUG
#ifndef HAL_ZONE_INIT
#define HAL_ZONE_INIT     GPE_ZONE_INIT
#endif
#endif


//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Global Variables


//------------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
// Local Functions


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
    
    DWORD dwSize;
 
    DEBUGMSG(HAL_ZONE_INIT,(TEXT("GetDriverInfo invoked !!\r\n")));
 
    lpInput->ddRVal = DDERR_CURRENTLYNOTAVAIL;
 
    if (IsEqualIID(lpInput->guidInfo, GUID_GetDriverInfo_VidMemBase) )
    {
        DEBUGMSG(HAL_ZONE_INIT,(TEXT("GUID_GetDriverInfo_VidMemBase\r\n")));
        dwSize = min(lpInput->dwExpectedSize, sizeof(PUINT8));
        lpInput->dwActualSize = sizeof(PUINT8);
        memcpy(lpInput->lpvData, &g_pVideoMemory, dwSize);
        lpInput->ddRVal = DD_OK;
    }
 
    if( lpInput->ddRVal != DD_OK )
    {
        DEBUGMSG(HAL_ZONE_INIT, (TEXT("HalGetDriverInfo: Currently not ")
                                 TEXT("available\r\n")));
    }
 
    return DDHAL_DRIVER_HANDLED;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////// DDHAL_DDCALLBACKS ///////////////////////////////
////////////////////////////////////////////////////////////////////////////////

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

    return ((DDIPU *)GetDDGPE())->WaitForVerticalBlank(pd);
}
