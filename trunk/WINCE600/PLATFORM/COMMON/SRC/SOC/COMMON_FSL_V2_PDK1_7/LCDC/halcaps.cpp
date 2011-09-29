//-----------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
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

DWORD FSLFourCC[] = {
  MAKEFOURCC('Y', 'V', '1', '2'),
  MAKEFOURCC('U', 'Y', 'V', 'Y')
};
#define MAX_FOURCC        sizeof(FSLFourCC)/sizeof(DWORD)


// callbacks from the DIRECTDRAW object
DDHAL_DDCALLBACKS cbDDCallbacks =
{
    sizeof( DDHAL_DDCALLBACKS ),            // dwSize
    DDHAL_CB32_CREATESURFACE |
    DDHAL_CB32_WAITFORVERTICALBLANK |
    DDHAL_CB32_CANCREATESURFACE |
    DDHAL_CB32_CREATEPALETTE |
    // DDHAL_CB32_GETSCANLINE |
    // DDHAL_CB32_SETEXCLUSIVEMODE |
    0,
    HalCreateSurface,
    DDGPEWaitForVerticalBlank,
    DDGPECanCreateSurface,
    DDGPECreatePalette,                     // CreatePalette
    NULL // HalGetScanLine,                 // GetScanLine
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
    // DDHAL_SURFCB32_SETCLIPLIST |
    DDHAL_SURFCB32_LOCK |
    DDHAL_SURFCB32_UNLOCK |
    DDHAL_SURFCB32_SETCOLORKEY |
    DDHAL_SURFCB32_GETBLTSTATUS |
    // DDHAL_SURFCB32_GETFLIPSTATUS |
    DDHAL_SURFCB32_UPDATEOVERLAY |
    DDHAL_SURFCB32_SETOVERLAYPOSITION |
    DDHAL_SURFCB32_SETPALETTE |
    0,
    DDGPEDestroySurface,                  // DestroySurface
    HalFlip,                              // Flip
    DDGPELock,                            // Lock
    DDGPEUnlock,                          // Unlock
    HalSetColorKey,                       // SetColorKey
    HalGetBltStatus,                      // GetBltStatus
    DDGPEGetFlipStatus,                   // GetFlipStatus
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
    UNREFERENCED_PARAMETER(modeidx);

    DEBUGMSG(GPE_ZONE_INIT, (TEXT("MXDDLcdc buildDDHALInfo: ==>\r\n")));
    memset( lpddhi, 0, sizeof(DDHALINFO) );     // Clear the DDHALINFO structure

    if( !g_pVideoMemory )    // in case this is called more than once...
    {
        MXDDLcdc *pGPE = static_cast<MXDDLcdc *>(GetDDGPE());
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
        DDSCAPS_ALPHA |                                 // Can create alpha-only surfaces
        DDSCAPS_BACKBUFFER |                            // Can create backbuffer surfaces
        DDSCAPS_FLIP |                                  // Can flip between surfaces
        DDSCAPS_FRONTBUFFER |                           // Can create front-buffer surfaces
        DDSCAPS_OVERLAY |                               // Can create overlay surfaces
        // DDSCAPS_PALETTE |                            // Not supported
        DDSCAPS_PRIMARYSURFACE |                        // Has a primary surface
        DDSCAPS_SYSTEMMEMORY |                          // Surfaces are in system memory
        DDSCAPS_VIDEOMEMORY |                           // Surfaces are in video memory
        // DDSCAPS_WRITEONLY |                          // Indicates that only write access is permitted to the surface
        // DDSCAPS_VIDEOPORT |                          // Not supported
        // DDSCAPS_READONLY |                           // Indicates that only read access is permitted to the surface
        // DDSCAPS_HARDWAREDEINTERLACE |                // Not supported
        // DDSCAPS_NOTUSERLOCKABLE |                    // Not supported
        // DDSCAPS_DYNAMIC |                            // Not supported
        DDSCAPS_OWNDC |                                 // Surfaces that own their own DCs, following 2008.1 QFE 
        0;
    lpddhi->ddCaps.dwNumFourCCCodes = 0; // MAX_FOURCC; // Number of four cc codes // LCDC does not support CSC

    // - Palette capabilities
    lpddhi->ddCaps.dwPalCaps  =                         // Palette capabilities.
        DDPCAPS_ALPHA |                                 // Supports palettes that include an alpha component
        DDPCAPS_PRIMARYSURFACE |                        // Indicates that the palette is attached to the primary surface
        0;

    // - Hardware blitting capabilities
    lpddhi->ddCaps.dwBltCaps =                          // Driver specific blitting capabilities.
        DDBLTCAPS_READSYSMEM |                          // Supports blitting from system memory.
        DDBLTCAPS_WRITESYSMEM |                         // Supports blitting to system memory.
        // DDBLTCAPS_FOURCCTORGB |                      // Supports blitting from a surface with a FOURCC pixel format to a surface with an RGB pixel format.
        // DDBLTCAPS_COPYFOURCC |                       // Supports blitting from a surface with a FOURCC pixel format to another surface with the same pixel format, or to the same surface.
        // DDBLTCAPS_FILLFOURCC |                       // Supports color-fill blitting to a surface with a FOURCC pixel format.
        0;
    lpddhi->ddCaps.dwCKeyCaps =                         // Color key capabilities
        // DDCKEYCAPS_BOTHBLT |                         // Supports transparent blitting with for both source and destination surfaces.
        DDCKEYCAPS_DESTBLT |                            // Supports transparent blitting with a color key that identifies the replaceable bits of the destination surface for RGB colors.
        // DDCKEYCAPS_DESTBLTCLRSPACE |                 // Supports transparent blitting with a color space that identifies the replaceable bits of the destination surface for RGB colors.
        // DDCKEYCAPS_DESTBLTCLRSPACEYUV |              // Supports transparent blitting with a color space that identifies the replaceable bits of the destination surface for YUV colors.
     DDCKEYCAPS_SRCBLT |                                // Supports transparent blitting using the color key for the source with this surface for RGB colors.
        // DDCKEYCAPS_SRCBLTCLRSPACE |                  // Supports transparent blitting using a color space for the source with this surface for RGB colors.
        // DDCKEYCAPS_SRCBLTCLRSPACEYUV |               // Supports transparent blitting using a color space for the source with this surface for YUV colors.
        0;
    lpddhi->ddCaps.dwAlphaCaps =                        // Alpha blitting capabilities.
        // DDALPHACAPS_ALPHAPIXELS |                    // Supports per-pixel alpha values specified alongside with the RGB values in the pixel structure.
        // DDALPHACAPS_PREMULT |                        // Supports pixel formats with premultiplied alpha values.
        // DDALPHACAPS_NONPREMULT |                     // Supports pixel formats with non-premultiplied alpha values.
        // DDALPHACAPS_ALPHAFILL |                      // Supports color-fill blitting using an alpha value.
        // DDALPHACAPS_ALPHANEG |                       // Supports inverted-alpha pixel formats, where 0 indicates fully opaque and 255 indicates fully transparent.
        0;
    SETROPBIT(lpddhi->ddCaps.dwRops, SRCCOPY);          // Set bits for ROPS supported
    SETROPBIT(lpddhi->ddCaps.dwRops, PATCOPY);
    SETROPBIT(lpddhi->ddCaps.dwRops, BLACKNESS);
    SETROPBIT(lpddhi->ddCaps.dwRops, WHITENESS);

    // - Overlay capabilities
    lpddhi->ddCaps.dwOverlayCaps =                      // General overlay capabilities.
        DDOVERLAYCAPS_FLIP |                            // Supports surface flipping with overlays.
        // DDOVERLAYCAPS_AUTOFLIP |                     // Supports auto surface flipping with overlays.
        // DDOVERLAYCAPS_FOURCC |                       // Supports FOURCC pixel formats with overlays. Use IDirectDraw::GetFourCCCodes to determine which FOURCC formats are supported. // LCDC does not support CSC
        // DDOVERLAYCAPS_ZORDER |                       // Supports changing Z order of overlays.
        // DDOVERLAYCAPS_MIRRORLEFTRIGHT |              // Supports surface mirroring in the left-to-right direction for overlays.
        DDOVERLAYCAPS_MIRRORUPDOWN |                    // Supports surface mirroring in the up-to-down direction for overlays.
        DDOVERLAYCAPS_CKEYSRC |                         // Supports source color keying for overlays.
        DDOVERLAYCAPS_CKEYSRCCLRSPACE |                 // Supports source color-space keying for overlays.
        // DDOVERLAYCAPS_CKEYSRCCLRSPACEYUV |           // Supports source color-space keying for overlays with FOURCC pixel formats.
        DDOVERLAYCAPS_CKEYDEST |                        // Supports destination color keying for overlays. // Although GW does not support DEST CK, need set here then ceplayer will use overlay, otherwise use gdi
        // DDOVERLAYCAPS_CKEYDESTCLRSPACE |             // Supports destination colo-space keying for overlays.
        // DDOVERLAYCAPS_CKEYDESTCLRSPACEYUV |          // Supports destination color-space keying for overlays with FOURCC pixel formats.
        // DDOVERLAYCAPS_CKEYBOTH |                     // Supports simultaneous source and destination color keying for overlays.
        DDOVERLAYCAPS_ALPHADEST |                       // Supports destination alpha blending for overlays.
        DDOVERLAYCAPS_ALPHASRC |                        // Supports source alpha blending for overlays.
        // DDOVERLAYCAPS_ALPHADESTNEG |                 // Supports inverted destination alpha blending for overlays.
        DDOVERLAYCAPS_ALPHASRCNEG |                     // Supports inverted source alpha blending for overlays.
        DDOVERLAYCAPS_ALPHACONSTANT |                   // Supports constant alpha blending for overlays (specified in the DDOVERLAYFX structure).
        // DDOVERLAYCAPS_ALPHAPREMULT |                 // Supports premultiplied alpha pixel formats for overlay alpha blending.
        // DDOVERLAYCAPS_ALPHANONPREMULT |              // Supports non-premultiplied alpha pixel formats for overlay alpha blending.
        // DDOVERLAYCAPS_ALPHAANDKEYDEST |              // Supports simultaneous source alpha blending with a destination color key for overlays.
        DDOVERLAYCAPS_OVERLAYSUPPORT |                  // Supports overlay surfaces.
        0;
    lpddhi->ddCaps.dwMaxVisibleOverlays=1;              // maximum number of visible overlays
    lpddhi->ddCaps.dwCurrVisibleOverlays = 0;           // current number of visible overlays
    lpddhi->ddCaps.dwAlignBoundarySrc = 0;              // overlay source rectangle alignment
    // CAUTION: This align constraint is verify in MXDDLcdc::UpdateOverlay
    lpddhi->ddCaps.dwAlignSizeSrc = 16;                 // overlay source rectangle byte size
    lpddhi->ddCaps.dwAlignBoundaryDest = 0;             // overlay destination rectangle alignment
    // CAUTION: This align constraint is verify in MXDDLcdc::UpdateOverlay
    lpddhi->ddCaps.dwAlignSizeDest = 16;                // overlay destination rectangle byte size
    lpddhi->ddCaps.dwMinOverlayStretch = 1000;          // Min Overlay Stretch factor
    lpddhi->ddCaps.dwMaxOverlayStretch = 1000;          // Max Overlay Stretch factor

    // - Miscallenous capabilities
    lpddhi->ddCaps.dwMiscCaps = 
        DDMISCCAPS_READVBLANKSTATUS |                   // Supports reading the current
                                                        //   V-Blank status of the hardware.
        DDMISCCAPS_FLIPVSYNCWITHVBI    ;                // Supports V-Sync-coordinated
                                                        //   flipping.0;

    // - Video port capabilities
    lpddhi->ddCaps.dwMinVideoStretch = 0;               // minimum video port stretch factor multiplied by 1000, eg 1000 == 1.0, 1300 == 1.3
    lpddhi->ddCaps.dwMaxVideoStretch = 0;               // maximum video port stretch factor multiplied by 1000, eg 1000 == 1.0, 1300 == 1.3
    lpddhi->ddCaps.dwMaxVideoPorts = 0;                 // maximum number of usable video ports
    lpddhi->ddCaps.dwCurrVideoPorts = 0;                // current number of video ports used

    // Fourcc
    lpddhi->lpdwFourCC = 0; // FSLFourCC;               // fourcc codes supported // LCDC does not support CSC
    
    DEBUGMSG(GPE_ZONE_INIT, (TEXT("MXDDLcdc buildDDHALInfo: <==\r\n")));
}
