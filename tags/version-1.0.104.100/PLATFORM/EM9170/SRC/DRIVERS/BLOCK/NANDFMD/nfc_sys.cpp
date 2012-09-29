//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
#include <windows.h>
#include <ceddk.h>
#include <nkintr.h>
#include "csp.h"

//-----------------------------------------------------------------------------
// External Functions
extern "C" BOOL BSPNAND_SetClock(BOOL bEnabled);
extern "C" VOID BSPNAND_ConfigIOMUX(DWORD CsNum);
extern "C" VOID* BSPNAND_RemapRegister(DWORD PhyAddr, DWORD size);
extern "C" BOOL BSP_OEMIoControl(DWORD dwIoControlCode, PBYTE pInBuf, DWORD nInBufSize, 
    PBYTE pOutBuf, DWORD nOutBufSize, PDWORD pBytesReturned);
//-----------------------------------------------------------------------------
// External Variables

//-----------------------------------------------------------------------------
// Defines


//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables


//-----------------------------------------------------------------------------
// Local Variables

//-----------------------------------------------------------------------------
// Local Functions


//-----------------------------------------------------------------------------
//
//  Function:  BSPNAND_SetClock
//
//  This enables/disable clocks for the NANDFC.
//
//  Parameters:
//     CsNum
//          [in] - enable/disable clock.  
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
BOOL BSPNAND_SetClock(BOOL bEnabled)
{
    BOOL rc = TRUE;
    
    if (bEnabled)
    {
        rc = DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX_PER_NFC, DDK_CLOCK_GATE_MODE_ENABLED);
    }
    else
    {
        rc = DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX_PER_NFC, DDK_CLOCK_GATE_MODE_DISABLED);
    }

    return rc;
}

//-----------------------------------------------------------------------------
//
//  Function:  BSPNAND_ConfigIOMUX
//
//  This functions config certain pin for nfc use.  
//
//  Parameters:
//      CsNum
//          [in] - how many cs are used defines how this function works.  
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
VOID BSPNAND_ConfigIOMUX(DWORD CsNum)
{
    UNREFERENCED_PARAMETER(CsNum);
}

//-----------------------------------------------------------------------------
//
//  Function:  BSPNAND_RemapRegister
//
//  This functions remaps certain registers for high level use.  
//
//  Parameters:
//      PhyAddr
//          [in] - physical address that needs to be remapped.  
//
//      size
//          [in] - mapping size
//  Returns:
//      Pointer to the remapped address.
//
//-----------------------------------------------------------------------------
VOID* BSPNAND_RemapRegister(DWORD PhyAddr, DWORD size)
{
    PHYSICAL_ADDRESS phyAddr;
    
    phyAddr.QuadPart = PhyAddr;
    return MmMapIoSpace(phyAddr, size, FALSE);
}