//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
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
extern "C" VOID BSPNAND_UnmapRegister(PVOID VirtAddr, DWORD size);
//-----------------------------------------------------------------------------
// External Variables

//-----------------------------------------------------------------------------
// Defines
#define MAX_GPMI_CLK_FREQUENCY_kHZ (120000)


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
//     bEnabled
//          [in] - enable/disable clock.  
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
BOOL BSPNAND_SetClock(BOOL bEnabled)
{
    BOOL rc = TRUE;
    UINT32 frequency , rootfreq, u32Div;
    static BOOL bInit = FALSE;

    if(!bInit){
        // Bump GPMI_CLK frequency up to the maximum.
        frequency = MAX_GPMI_CLK_FREQUENCY_kHZ;
        //status = DDKClockSetGpmiClk(&frequency, TRUE);

        DDKClockGetFreq(DDK_CLOCK_SIGNAL_REF_GPMI, &rootfreq);

        u32Div = rootfreq / (frequency*1000);
        if(u32Div != 0)
            rc = DDKClockConfigBaud(DDK_CLOCK_SIGNAL_GPMI, DDK_CLOCK_BAUD_SOURCE_REF_GPMI, u32Div );
        if (rc != TRUE)
        {
            return rc;
        }    
        bInit = TRUE;
    }
    
    if (bEnabled)
    {
        rc = DDKClockSetGatingMode(DDK_CLOCK_GATE_GPMI_CLK, FALSE);
    }
    else
    {
        rc = DDKClockSetGatingMode(DDK_CLOCK_GATE_GPMI_CLK, TRUE);
    }

    
    return rc;
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


VOID BSPNAND_UnmapRegister(PVOID VirtAddr, DWORD size)
{
    MmUnmapIoSpace(VirtAddr, size);
}
