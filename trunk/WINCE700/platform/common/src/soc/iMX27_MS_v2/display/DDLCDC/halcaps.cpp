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
//------------------------------------------------------------------------------
//---------------------------------------------------------------------------
//  Copyright (C) 2005-2006, 2007, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//---------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  File:  halcaps.cpp
//
//  Hardware abstraction layer capabilities.
//
//------------------------------------------------------------------------------

#include "precomp.h"


//------------------------------------------------------------------------------
// External Functions


//------------------------------------------------------------------------------
// External Variables


//------------------------------------------------------------------------------
// Defines

#define SCREEN_WIDTH    (g_pGPE->ScreenWidth())
#define SCREEN_HEIGHT   (g_pGPE->ScreenHeight())


//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Global Variables


// callbacks from the DIRECTDRAW object
DDHAL_DDCALLBACKS cbDDCallbacks =
{
    sizeof( DDHAL_DDCALLBACKS ),          // dwSize
    DDHAL_CB32_CREATESURFACE |
    DDHAL_CB32_WAITFORVERTICALBLANK |
    DDHAL_CB32_CANCREATESURFACE |
    DDHAL_CB32_CREATEPALETTE |
    0,
    DDGPECreateSurface,
    DDGPEWaitForVerticalBlank,
    HalCanCreateSurface,                  // CanCreateSurface,
    DDGPECreatePalette,                   // CreatePalette
    NULL,                                 // GetScanLine
};


// callbacks from the DIRECTDRAWMISCELLANEOUS object
DDHAL_DDMISCELLANEOUSCALLBACKS MiscellaneousCallbacks =
{
    sizeof(DDHAL_DDMISCELLANEOUSCALLBACKS),
    0,
};


// callbacks from the DIRECTDRAWPALETTE object
DDHAL_DDPALETTECALLBACKS cbDDPaletteCallbacks =
{
    sizeof( DDHAL_DDPALETTECALLBACKS ),   // dwSize
    DDHAL_PALCB32_DESTROYPALETTE |        // dwFlags
    DDHAL_PALCB32_SETENTRIES |
    0,
    DDGPEDestroyPalette,                  // DestroyPalette
    DDGPESetEntries                       // SetEntries
};


// callbacks from the DIRECTDRAWSURFACE object
DDHAL_DDSURFACECALLBACKS cbDDSurfaceCallbacks =
{
    sizeof( DDHAL_DDSURFACECALLBACKS ),   // dwSize
    DDHAL_SURFCB32_DESTROYSURFACE |       // dwFlags
    DDHAL_SURFCB32_FLIP |
    DDHAL_SURFCB32_LOCK |
    DDHAL_SURFCB32_UNLOCK |
    DDHAL_SURFCB32_GETBLTSTATUS |
    DDHAL_SURFCB32_GETFLIPSTATUS |
    DDHAL_SURFCB32_UPDATEOVERLAY |
    DDHAL_SURFCB32_SETOVERLAYPOSITION |
    DDHAL_SURFCB32_SETPALETTE |
    NULL,
    DDGPEDestroySurface,                  // DestroySurface
    HalFlip,                              // Flip
    DDGPELock,                            // Lock
    DDGPEUnlock,                          // Unlock
    NULL,                                 // SetColorKey
    HalGetBltStatus,                      // GetBltStatus
    HalGetFlipStatus,                     // GetFlipStatus
    HalUpdateOverlay,                     // UpdateOverlay
    HalSetOverlayPosition,                // SetOverlayPosition
    DDGPESetPalette,                      // SetPalette
};


// callbacks from the DIRECTDRAWVIDEOPORT pseudo object
DDHAL_DDVIDEOPORTCALLBACKS VideoPortCallbacks =
{
    sizeof(DDHAL_DDVIDEOPORTCALLBACKS),
    0,
};


// InitDDHALInfo must set up this information
unsigned long    g_nVideoMemorySize        = 0L;
unsigned char *  g_pVideoMemory            = NULL;    // virtual address of video memory from client's side


//------------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
// Local Functions


//------------------------------------------------------------------------------
//
// Function: buildDDHALInfo
//
// This function sets up the global variables that DDGPE requires.
// The buildDDHALInfo function then populates the specified DDHALINFO
// structure with the appropriate DirectDraw callbacks, caps, and
// other driver-specific parameters. These are filled out both at
// initialization time and during mode changes.
//
// Parameters:
//      lpddhi
//          [out] A pointer to a DDHALINFO structure that should be
//          populated with the appropriate settings.
//
//      modeIndex
//          [in] A DWORD indicating the current mode.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
EXTERN_C void buildDDHALInfo( LPDDHALINFO lpddhi, DWORD modeidx )
{
    DEBUGMSG(GPE_ZONE_INIT, (TEXT("MX27DDLcdc buildDDHALInfo: ==>\r\n")));
    memset( lpddhi, 0, sizeof(DDHALINFO) );     // Clear the DDHALINFO structure

    if( !g_pVideoMemory )    // in case this is called more than once...
    {
        MX27DDLcdc *pGPE = static_cast<MX27DDLcdc *>(GetDDGPE());
        unsigned long physicalVideoMemoryStart;

        pGPE->GetPhysicalVideoMemory( &physicalVideoMemoryStart,
                                      &g_nVideoMemorySize );

        DEBUGMSG( GPE_ZONE_INIT, (TEXT("GetPhysicalVideoMemory returned ")
                                  TEXT("phys=0x%08x size=%d\r\n"),
                                  physicalVideoMemoryStart,
                                  g_nVideoMemorySize));

        g_pVideoMemory = (BYTE*)physicalVideoMemoryStart;

        DEBUGMSG(GPE_ZONE_INIT,(TEXT("gpVidMem=%08x\r\n"), g_pVideoMemory));
    }

    // Populate the rest of the DDHALINFO structure:
    lpddhi->dwSize = sizeof(DDHALINFO);
    lpddhi->lpDDCallbacks = &cbDDCallbacks;
    lpddhi->lpDDSurfaceCallbacks = &cbDDSurfaceCallbacks;
    lpddhi->lpDDPaletteCallbacks = &cbDDPaletteCallbacks;
    lpddhi->GetDriverInfo = HalGetDriverInfo;

    // HW specific capabilities:
    lpddhi->ddCaps.dwSize = sizeof(DDCAPS);             // Size of the DDCAPS structure

    // - Surface capabilities
    lpddhi->ddCaps.dwVidMemTotal = g_nVideoMemorySize;  // Total amount of video memory
    lpddhi->ddCaps.dwVidMemFree = g_nVideoMemorySize;   // Amount of free video memory
    lpddhi->ddCaps.dwVidMemStride = 0;                  // Video memory stride (0 if linear)
    lpddhi->ddCaps.ddsCaps.dwCaps =                     // DDSCAPS structure has all the general capabilities
        DDSCAPS_BACKBUFFER |                            // Can create backbuffer surfaces
        DDSCAPS_FLIP |                                  // Can flip between surfaces
        DDSCAPS_FRONTBUFFER |                           // Can create front-buffer surfaces
        DDSCAPS_OVERLAY |                               // Can create overlay surfaces
        DDSCAPS_PRIMARYSURFACE |                        // Has a primary surface
        DDSCAPS_SYSTEMMEMORY |                          // Surfaces are in system memory
        DDSCAPS_VIDEOMEMORY |                           // Surfaces are in video memory
        DDSCAPS_OWNDC |                      //support the DDSCAPS_OWNDC surfaces
        0;
    lpddhi->ddCaps.dwNumFourCCCodes = 0;                // Number of four cc codes

    // - Palette capabilities
    lpddhi->ddCaps.dwPalCaps  = 0;                      // Palette capabilities.

    // - Hardware blitting capabilities
    lpddhi->ddCaps.dwBltCaps =                          // Driver specific blitting capabilities.
        DDBLTCAPS_READSYSMEM |                          // Supports blitting from system memory.
        DDBLTCAPS_WRITESYSMEM |                         // Supports blitting to system memory.
        DDBLTCAPS_ROTATION90 |                           // Support blit rotation
        0;

    // - Color Key capabilities
    lpddhi->ddCaps.dwCKeyCaps = 0;                      // Color key capabilities

    //- Alpha blitting capabilities
    lpddhi->ddCaps.dwAlphaCaps =   0;                   // Alpha blitting capabilities.

    SETROPBIT(lpddhi->ddCaps.dwRops, SRCCOPY);          // Set bits for ROPS supported
    SETROPBIT(lpddhi->ddCaps.dwRops, PATCOPY);
    SETROPBIT(lpddhi->ddCaps.dwRops, BLACKNESS);
    SETROPBIT(lpddhi->ddCaps.dwRops, WHITENESS);

    // - Overlay capabilities for mx27.

    // - Overlay capabilities
    lpddhi->ddCaps.dwOverlayCaps =                      // General overlay capabilities.
        DDOVERLAYCAPS_FLIP |                            // Supports surface flipping with overlays.
        NULL |                                          // Supports FOURCC pixel formats with overlays. Use IDirectDraw::GetFourCCCodes to determine which FOURCC formats are supported.
        NULL |                                          // Supports surface mirroring in the up-to-down direction for overlays.
        DDOVERLAYCAPS_CKEYSRC |                         // Supports source color keying for overlays.
        NULL |                                          // Supports source alpha blending for overlays.
        NULL |                                          // Supports inverted source alpha blending for overlays.
        NULL |                                          // Supports constant alpha blending for overlays (specified in the DDOVERLAYFX structure).
        DDOVERLAYCAPS_OVERLAYSUPPORT |                  // Supports overlay surfaces.
        0;
    lpddhi->ddCaps.dwMaxVisibleOverlays=1;              // maximum number of visible overlays
    lpddhi->ddCaps.dwCurrVisibleOverlays = 0;           // current number of visible overlays
    lpddhi->ddCaps.dwAlignBoundarySrc = 0;              // overlay source rectangle alignment
    lpddhi->ddCaps.dwAlignSizeSrc = 16;                 // overlay source rectangle byte size
    lpddhi->ddCaps.dwAlignBoundaryDest = 0;             // overlay destination rectangle alignment
    lpddhi->ddCaps.dwAlignSizeDest = 16;                // overlay destination rectangle byte size
    lpddhi->ddCaps.dwMinOverlayStretch = 1000;          // Min Overlay Stretch factor
    lpddhi->ddCaps.dwMaxOverlayStretch = 1000;          // Max Overlay Stretch factor

    // - Miscallenous capabilities
    lpddhi->ddCaps.dwMiscCaps = 
        DDMISCCAPS_FLIPVSYNCWITHVBI |
        0;

    // - Video port capabilities
    lpddhi->ddCaps.dwMinVideoStretch = 0;               // minimum video port stretch factor multiplied by 1000, eg 1000 == 1.0, 1300 == 1.3
    lpddhi->ddCaps.dwMaxVideoStretch = 0;               // maximum video port stretch factor multiplied by 1000, eg 1000 == 1.0, 1300 == 1.3
    lpddhi->ddCaps.dwMaxVideoPorts = 0;                 // maximum number of usable video ports
    lpddhi->ddCaps.dwCurrVideoPorts = 0;                // current number of video ports used

    // Fourcc
    lpddhi->lpdwFourCC = 0;                             // fourcc codes not supported

    DEBUGMSG(GPE_ZONE_INIT, (TEXT("MX27DDLcdc buildDDHALInfo: <==\r\n")));
}
