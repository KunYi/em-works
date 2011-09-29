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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
/* 
** Copyright 2000-2003 Intel Corporation All Rights Reserved.
**
** Portions of the source code contained or described herein and all documents
** related to such source code (Material) are owned by Intel Corporation
** or its suppliers or licensors and is licensed by Microsoft Corporation for distribution.  
** Title to the Material remains with Intel Corporation or its suppliers and licensors. 
** Use of the Materials is subject to the terms of the Microsoft license agreement which accompanied the Materials.  
** No other license under any patent, copyright, trade secret or other intellectual
** property right is granted to or conferred upon you by disclosure or
** delivery of the Materials, either expressly, by implication, inducement,
** estoppel or otherwise 
** Some portion of the Materials may be copyrighted by Microsoft Corporation.
*/

#include "precomp.h"

// callbacks from the DIRECTDRAW object

DDHAL_DDCALLBACKS cbDDCallbacks =
{
    sizeof( DDHAL_DDCALLBACKS ),
    DDHAL_CB32_CREATESURFACE |
    DDHAL_CB32_WAITFORVERTICALBLANK |
    DDHAL_CB32_CANCREATESURFACE |
//    DDHAL_CB32_CREATEPALETTE |
//    DDHAL_CB32_GETSCANLINE |
    0,
    DDGPECreateSurface,
    DDGPEWaitForVerticalBlank,
    DDGPECanCreateSurface,                        
    NULL,                         
    NULL
};

// callbacks from the DIRECTDRAWSURFACE object

DDHAL_DDSURFACECALLBACKS cbDDSurfaceCallbacks =
{
    sizeof( DDHAL_DDSURFACECALLBACKS ), // dwSize
    DDHAL_SURFCB32_DESTROYSURFACE |         // dwFlags
//    DDHAL_SURFCB32_FLIP |
    DDHAL_SURFCB32_LOCK |
    DDHAL_SURFCB32_UNLOCK |
    DDHAL_SURFCB32_SETCOLORKEY |
//    DDHAL_SURFCB32_GETBLTSTATUS |
//    DDHAL_SURFCB32_GETFLIPSTATUS |
//    DDHAL_SURFCB32_UPDATEOVERLAY |
//    DDHAL_SURFCB32_SETOVERLAYPOSITION |
//    DDHAL_SURFCB32_SETPALETTE |
    0,
    DDGPEDestroySurface,
    NULL,
    DDGPELock,
    DDGPEUnlock,
    DDGPESetColorKey,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

// InitDDHALInfo must set up this information
unsigned long           g_nVideoMemorySize      = 0L;
unsigned char *         g_pVideoMemory          = NULL; // virtual address of video memory from client's side
DWORD                   g_nTransparentColor     = 0L;

DWORD WINAPI HalGetDriverInfo(LPDDHAL_GETDRIVERINFODATA lpInput)
{
    DEBUGENTER( HalGetDriverInfo );

    lpInput->ddRVal = DDERR_CURRENTLYNOTAVAIL;

    DEBUGLEAVE( HalGetDriverInfo );

    return (DDHAL_DRIVER_HANDLED);
}

void buildDDHALInfo( LPDDHALINFO lpddhi, DWORD modeidx )
{
    memset( lpddhi, 0, sizeof(DDHALINFO) );

    SA2Video * pSA2Video = (SA2Video *)GetDDGPE();

    if ( !g_pVideoMemory )   // in case this is called more than once...
    {
        unsigned long VideoMemoryStart;
        pSA2Video->GetVirtualVideoMemory( &VideoMemoryStart, &g_nVideoMemorySize );
        DEBUGMSG( GPE_ZONE_INIT,(TEXT("GetVirtualVideoMemory returned addr=0x%08x size=%d\r\n"), VideoMemoryStart, g_nVideoMemorySize));

        g_pVideoMemory = (BYTE*)VideoMemoryStart;
        DEBUGMSG( GPE_ZONE_INIT,(TEXT("gpVidMem=%08x\r\n"), g_pVideoMemory ));
    }

    lpddhi->dwSize = sizeof(DDHALINFO);

    lpddhi->lpDDCallbacks = &cbDDCallbacks;
    lpddhi->lpDDSurfaceCallbacks = &cbDDSurfaceCallbacks;
    
    lpddhi->GetDriverInfo = HalGetDriverInfo;

    lpddhi->ddCaps.dwSize = sizeof(DDCAPS);

    lpddhi->ddCaps.dwVidMemTotal = g_nVideoMemorySize;
    lpddhi->ddCaps.dwVidMemFree = g_nVideoMemorySize;
    lpddhi->ddCaps.ddsCaps.dwCaps=
		DDSCAPS_PRIMARYSURFACE |
		DDSCAPS_SYSTEMMEMORY |
		0;

    lpddhi->ddCaps.dwPalCaps=
		DDPCAPS_PRIMARYSURFACE |
		0;

    lpddhi->ddCaps.dwBltCaps =
        DDBLTCAPS_READSYSMEM |
        DDBLTCAPS_WRITESYSMEM |
        0;

    SETROPBIT(lpddhi->ddCaps.dwRops,SRCCOPY);                   // Set bits for ROPS supported
    SETROPBIT(lpddhi->ddCaps.dwRops,PATCOPY);
    SETROPBIT(lpddhi->ddCaps.dwRops,BLACKNESS);
    SETROPBIT(lpddhi->ddCaps.dwRops,WHITENESS);
}

