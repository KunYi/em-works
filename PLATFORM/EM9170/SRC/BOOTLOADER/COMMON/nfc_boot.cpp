//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
#include <windows.h>
#include <ceddk.h>
#include <oal_memory.h>

//-----------------------------------------------------------------------------
// External Functions
extern "C" BOOL BSPNAND_SetClock(BOOL bEnabled);
extern "C" VOID BSPNAND_ConfigIOMUX(DWORD CsNum);
extern "C" VOID* BSPNAND_RemapRegister(DWORD PhyAddr, DWORD size);
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
//      None.
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
BOOL BSPNAND_SetClock(BOOL bEnabled)
{
    UNREFERENCED_PARAMETER(bEnabled);
    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function:  NFCConfigIOMUX
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
    UNREFERENCED_PARAMETER(size);
    return OALPAtoUA(PhyAddr);
}
