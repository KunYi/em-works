//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  lut.c
//
//  IPU Look-Up Table memory access functions
//
//------------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <Devload.h>
#include <ceddk.h>
#include <cmnintrin.h>
#pragma warning(pop)

#include "common_macros.h"
#include "common_ipuv3.h"

#include "lut.h"
#include "IpuBuffer.h"
#include "Ipu_base.h"
#include "Ipu_common.h"


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
PCSP_IPU_MEM_LUT   g_pIPUV3_LUT;


//------------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
// Local Functions


//------------------------------------------------------------------------------
//
// Function: LUTRegsInit
//
// This function initializes the structures needed to access
// the IPUv3 Look-Up Table memory.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if successful, FALSE if failure.
//------------------------------------------------------------------------------
BOOL LUTRegsInit()
{
    BOOL rc = FALSE;
    DWORD dwIPUBaseAddr;
    HANDLE hIPUBase = NULL;
    PHYSICAL_ADDRESS phyAddr;

    IPU_FUNCTION_ENTRY();

    if (g_pIPUV3_LUT == NULL)
    {
        //  *** Use IPU_BASE driver to retrieve IPU Base Address ***

        // First, create handle to IPU_BASE driver
        hIPUBase = IPUV3BaseOpenHandle();
        if (hIPUBase == INVALID_HANDLE_VALUE)
        {
            RETAILMSG(1,
                (TEXT("%s: Opening IPU_BASE handle failed!\r\n"), __WFUNCTION__));
            goto cleanUp;
        }

        dwIPUBaseAddr = IPUV3BaseGetBaseAddr(hIPUBase);
        if (dwIPUBaseAddr == -1)
        {
            RETAILMSG (1,
                (TEXT("%s: Failed to retrieve IPU Base addr!\r\n"), __WFUNCTION__));
            goto cleanUp;
        }

        // Map LUT memory region entries
        phyAddr.QuadPart = dwIPUBaseAddr + CSP_IPUV3_LUT_REGS_OFFSET;

        // Map peripheral physical address to virtual address
        g_pIPUV3_LUT = (PCSP_IPU_MEM_LUT) MmMapIoSpace(phyAddr, sizeof(CSP_IPU_MEM_LUT),
            FALSE);

        // Check if virtual mapping failed
        if (g_pIPUV3_LUT == NULL)
        {
            RETAILMSG(1,
                (_T("Init:  MmMapIoSpace failed!\r\n")));
            goto cleanUp;
        }
    }

    rc = TRUE;

cleanUp:
    if (!IPUV3BaseCloseHandle(hIPUBase))
    {
        RETAILMSG(1,
            (_T("LUT Cleanup: Failed closing IPU Base handle!\r\n")));
    }

    // If initialization failed, be sure to clean up
    if (!rc) LUTRegsCleanup();

    IPU_FUNCTION_EXIT();
    return rc;
}

//-----------------------------------------------------------------------------
//
// Function:  LUTRegsCleanup
//
// This function deallocates the data structures required for interaction
// with the IPUv3 LUT.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void LUTRegsCleanup(void)
{
    IPU_FUNCTION_ENTRY();

    // Unmap peripheral registers
    if (g_pIPUV3_LUT)
    {
        MmUnmapIoSpace(g_pIPUV3_LUT, sizeof(PCSP_IPU_MEM_LUT));
        g_pIPUV3_LUT = NULL;
    }

    IPU_FUNCTION_EXIT();
}

//------------------------------------------------------------------------------
//
// Function: LUTWriteSingleEntry
//
// This function writes one 32-bit entry into the
// IPUv3 Look-Up Table memory.
//
// Parameters:
//      index
//          [in] The LUT look-up entry index (between 0-255).
//
//      dwLUTEntry
//          [in] Data to write into LUT.
//
// Returns:
//      FALSE if dwIndex does not fall into a valid range (0-255).
//------------------------------------------------------------------------------
BOOL LUTWriteSingleEntry(DWORD dwIndex, DWORD dwLUTEntry)
{
    // Check validity of dwIndex
    if ((dwIndex < 0) || (dwIndex > 255))
    {
        return FALSE;
    }

    OUTREG32(&g_pIPUV3_LUT->LUTEntries[dwIndex], dwLUTEntry);

    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function: LUTWriteRangeOfEntries
//
// This function writes a range of 32-bit entries into the
// IPUv3 Look-Up Table memory.
//
// Parameters:
//      dwStartIndex
//          [in] The LUT look-up entry index (between 0-255).
//
//      dwEndIndex
//          [in] The LUT look-up entry index (between 0-255).
//
//      pdwLUTEntries
//          [in] Pointer to an array of 32-bit LUT entries.
//
// Returns:
//      FALSE if any of the following conditions are met:
//          - dwStartIndex is less than 0 or greater than 255
//          - dwEndIndex is less than 0 or greater than 255
//          - dwEndIndex is less than dwStartIndex
//      TRUE if success.
//------------------------------------------------------------------------------
BOOL LUTWriteRangeOfEntries(DWORD dwStartIndex, DWORD dwEndIndex, DWORD *pdwLUTEntries)
{
    DWORD i;

    // Check validity of dwStartIndex
    if ((dwStartIndex < 0) || (dwStartIndex > 255))
    {
        return FALSE;
    }

    // Check validity of dwEndIndex
    if ((dwEndIndex < 0) || (dwEndIndex > 255))
    {
        return FALSE;
    }

    // Check relation of dwEndIndex and dwStartIndex
    if (dwEndIndex < dwStartIndex)
    {
        return FALSE;
    }

    for (i = dwStartIndex; i <= dwEndIndex; i++)
    {
        LUTWriteSingleEntry(i, pdwLUTEntries[i - dwStartIndex]);
    }

    return TRUE;
}
