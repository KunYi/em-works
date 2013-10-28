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
//------------------------------------------------------------------------------
//
//  Copyright (C) 2005-2006, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

#if !(UNDER_CE >= 600)
#include "precomp.h"

// Misc function prototypes
EXTERN_C BOOL mmHALInit( LPDDHALINFO lpddhi, BOOL reset, DWORD modeidx );


EXTERN_C BOOL WINAPI HALInit(
    LPDDHALINFO lpddhi,
    BOOL reset,
    DWORD modeidx )
{
    //SCODE sc = S_OK;

    DEBUGENTER(mmHALInit);

    if( !g_pGPE )   // in case this is called more than once...
    {
        DEBUGMSG( GPE_ZONE_INIT,(TEXT("DDHAL_IF calling GetGPE()\r\n")));
        g_pGPE = (DDGPE*)GetGPE();
        // If GetGPE allocation failed, return.
        if (!g_pGPE)
        {
            RETAILMSG(1, (TEXT("HALInit: GPE allocation failed.\r\n")));
            return(FALSE);
        }

        DEBUGMSG( GPE_ZONE_INIT,(TEXT("GetGPE() returned g_pGPE=0x%08x\r\n"), g_pGPE));

        // Video memory info:
        g_pDDrawPrimarySurface = (DDGPESurf*)(g_pGPE->PrimarySurface());
        DEBUGMSG( GPE_ZONE_INIT,(TEXT("g_pDDrawPrimarySurface: 0x%08x\r\n"), g_pDDrawPrimarySurface));
    }
/*
    // If GPE is currently rotated, don't allow DDraw to use the driver.
    if (g_pGPE->IsRotate())
    {
        RETAILMSG(1, (TEXT("HALInit: GPE is rotated.\r\n")));
        return(FALSE);
    }
*/
    // Initialize the hardware and the callbacks
    buildDDHALInfo(lpddhi, modeidx);

    DEBUGLEAVE(mmHALInit);

    return TRUE;
}


EXTERN_C BOOL UpdateHALInit(
    LPDDRAWI_DIRECTDRAW_GBL lpDD,           // driver struct
    DWORD modeidx )
{
    DDHALINFO   ddhalinfo;

    DEBUGENTER(UpdateHALInit);

    memset(&ddhalinfo, 0, sizeof(ddhalinfo));
    ddhalinfo.dwSize = sizeof(ddhalinfo);

    buildDDHALInfo(&ddhalinfo, modeidx);

    lpDD->vmiData = ddhalinfo.vmiData;
    lpDD->dwMonitorFrequency = ddhalinfo.dwMonitorFrequency;
    lpDD->ddCaps = ddhalinfo.ddCaps;

    DEBUGLEAVE(UpdateHALInit);
    return TRUE;
}

#endif // !(UNDER_CE >= 600)
