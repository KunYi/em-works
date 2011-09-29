//------------------------------------------------------------------------------
//
//  Copyright (C) 2008-2009, Freescale Semiconductor, Inc. All Rights Reserved.
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


//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Global Variables

DWORD FSLFourCC[] = {
    FOURCC_YV12,
    FOURCC_NV12,    
    FOURCC_UYVY
};
#define MAX_FOURCC        sizeof(FSLFourCC)/sizeof(DWORD)

// callbacks from the DIRECTDRAW object
DDHAL_DDCALLBACKS cbDDCallbacks =
{
    sizeof( DDHAL_DDCALLBACKS ),
    DDHAL_CB32_CREATESURFACE        |
    DDHAL_CB32_WAITFORVERTICALBLANK |
    DDHAL_CB32_CANCREATESURFACE     |
    DDHAL_CB32_CREATEPALETTE        |
//    DDHAL_CB32_GETSCANLINE          |
    0,
    HalCreateSurface,
    DDGPEWaitForVerticalBlank,
    DDGPECanCreateSurface,
    DDGPECreatePalette,
    NULL                            // GetScanLine is not supported.
};

// callbacks from the DIRECTDRAWSURFACE object

DDHAL_DDSURFACECALLBACKS cbDDSurfaceCallbacks =
{
    sizeof( DDHAL_DDSURFACECALLBACKS ), // dwSize
    DDHAL_SURFCB32_DESTROYSURFACE     | // dwFlags
    DDHAL_SURFCB32_FLIP               |
    DDHAL_SURFCB32_LOCK               |
    DDHAL_SURFCB32_UNLOCK             |
    DDHAL_SURFCB32_SETCOLORKEY        |
    DDHAL_SURFCB32_GETBLTSTATUS       |
    DDHAL_SURFCB32_GETFLIPSTATUS      |
    DDHAL_SURFCB32_UPDATEOVERLAY      |
    DDHAL_SURFCB32_SETOVERLAYPOSITION |
    // DDHAL_SURFCB32_SETPALETTE         |
    0,
    HalDestroySurface,
    HalFlip,
    HalLock,
    HalUnlock,
    HalSetColorKey,
    HalGetBltStatus,
    HalGetFlipStatus,
    HalUpdateOverlay,
    HalSetOverlayPosition,
    NULL
};

// callbacks from the DIRECTDRAWPALETTE object

DDHAL_DDPALETTECALLBACKS cbDDPaletteCallbacks =
{
    sizeof( DDHAL_DDPALETTECALLBACKS ),   // dwSize
    DDHAL_PALCB32_DESTROYPALETTE |        // dwFlags
    DDHAL_PALCB32_SETENTRIES     |
    0,
    DDGPEDestroyPalette,                  // DestroyPalette
    DDGPESetEntries                       // SetEntries
};


// buildDDHALInfo must set up this information
unsigned long   g_nVideoMemorySize = 0L;
unsigned char * g_pVideoMemory     = NULL; // virtual address of video memory
                                           // from client's side

DWORD g_nTransparentColor = 0L;


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
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(modeidx);

    DEBUGMSG(GPE_ZONE_INIT, (TEXT("+buildDDHALInfo\r\n")));

    // Clear the entire DDHALINFO structure.
    memset( lpddhi, 0, sizeof(DDHALINFO) );

    DDIPU *pGPE = (DDIPU *)GetDDGPE();

    // WINCE600: The DDHALMODEINFO structure is no longer defined in WinCE 6.0
    //           so we cannot simply use it to define our supported display
    //           modes.
    //
    //           Instead, we must properly initialize the supported display
    //           modes in DDIPU::Init().

    // g_pGPE and g_pDDrawPrimarySurface were already set up by HalInit

    pGPE->GetVideoMemoryInfo((PULONG)&g_pVideoMemory, &g_nVideoMemorySize );
    DEBUGMSG( GPE_ZONE_INIT, (TEXT("GetVideoMemoryInfo returned ")
                              TEXT("size=%d\r\n"),
                              g_nVideoMemorySize));

    // Populate the rest of the DDHALINFO structure:
    lpddhi->dwSize               = sizeof(DDHALINFO);
    lpddhi->lpDDCallbacks        = &cbDDCallbacks;
    lpddhi->lpDDSurfaceCallbacks = &cbDDSurfaceCallbacks;
    lpddhi->lpDDPaletteCallbacks = &cbDDPaletteCallbacks;
    lpddhi->GetDriverInfo        = HalGetDriverInfo;

    // hw specific caps:
    lpddhi->ddCaps.dwSize = sizeof(DDCAPS);     // size of the DDDRIVERCAPS structure

    // NOTE: DDCKEYCAPS_SRCBLT must be defined for both the ADC and SDC
    //       displays with WinCE 6.0 in order to use the IPU hardware
    //       accelerated RGB and YUV overlay surface flipping functions
    //       HalUpdateOverlay() and HalFlip().
    //
    //       If the DDCKEYCAPS_SRCBLT flag is not defined, then DirectDraw
    //       will always fall back to using the non-IPU accelerated GDI RGB
    //       bitblt operations when rendering video and this will result in
    //       very poor system and display performance.
    //
    lpddhi->ddCaps.dwCKeyCaps = // Color key capabilities.
#if defined(USE_C2D_ROUTINES)
         DDCKEYCAPS_BOTHBLT            | // Supports transparent blitting
                                           //   with a color key for both source
                                           //   and destination surfaces.
#else
        // DDCKEYCAPS_BOTHBLT            | // Supports transparent blitting
                                           //   with a color key for both source
                                           //   and destination surfaces.
#endif
        // DDCKEYCAPS_DESTBLT            | // Supports transparent blitting
                                        //   with a color key that identifies
                                        //   the replaceable bits of the
                                        //   destination surface for RGB
                                        //   colors.
        // DDCKEYCAPS_DESTBLTCLRSPACE    | // Supports transparent blitting
                                        //   with a color space that
                                        //   identifies the replaceable bits
                                        //   of the destination surface for
                                        //   RGB colors.
        // DDCKEYCAPS_DESTBLTCLRSPACEYUV | // Supports transparent blitting
                                           //   with a color space that
                                           //   identifies the replaceable bits
                                           //   of the destination surface for
                                           //   YUV colors.
        DDCKEYCAPS_SRCBLT             | // Supports transparent blitting
                                           //   using the color key for the
                                           //   source with this surface for
                                           //   RGB colors.
        // DDCKEYCAPS_SRCBLTCLRSPACE     | // Supports transparent blitting
                                           //   using a color space for the
                                           //   source with this surface for
                                           //   RGB colors.
        // DDCKEYCAPS_SRCBLTCLRSPACEYUV  | // Supports transparent blitting
                                           //   using a color space for the
                                           //   source with this surface for
                                           //   YUV colors.
        0;

    lpddhi->ddCaps.dwPalCaps  = // Palette capabilities.
        // DDPCAPS_ALPHA          | // Supports palettes that include an alpha
                                    // component.
        // DDPCAPS_PRIMARYSURFACE | // Indicates that the palette is attached
                                    // to the primary surface.
        0;

    lpddhi->ddCaps.dwBltCaps = // Driver specific blitting capabilities.
        DDBLTCAPS_READSYSMEM  | // Supports blitting from system memory.
        DDBLTCAPS_WRITESYSMEM | // Supports blitting to system memory.
#if defined(USE_C2D_ROUTINES)
        DDBLTCAPS_FOURCCTORGB | // Supports blitting from a surface with a
                                   //   FOURCC pixel format to a surface with
                                   //   an RGB pixel format.
        //DDBLTCAPS_COPYFOURCC  | // Supports blitting from a surface with a
                                //   FOURCC pixel format to another surface
                                //   with the same pixel format, or to the
                                //   same surface.
#else
        //DDBLTCAPS_FOURCCTORGB | // Supports blitting from a surface with a
                                   //   FOURCC pixel format to a surface with
                                   //   an RGB pixel format.
        DDBLTCAPS_COPYFOURCC  | // Supports blitting from a surface with a
                                //   FOURCC pixel format to another surface
                                //   with the same pixel format, or to the
                                //   same surface.
#endif
        // DDBLTCAPS_FILLFOURCC  | // Supports color-fill blitting to a surface
                                   //   with a FOURCC pixel format.
        0;

    lpddhi->ddCaps.dwAlphaCaps =  // Alpha blitting capabilities.
#if defined(USE_C2D_ROUTINES)
         DDALPHACAPS_ALPHAPIXELS   | // Supports per-pixel alpha values
                                     //   specified alongside with the RGB
                                     //   values in the pixel structure.
        // DDALPHACAPS_PREMULT     | // Supports pixel formats with
                                     //   premultiplied alpha values.
         DDALPHACAPS_NONPREMULT    | // Supports pixel formats with non-
                                     //   premultiplied alpha values.
#else
        // DDALPHACAPS_ALPHAPIXELS   | // Supports per-pixel alpha values
                                     //   specified alongside with the RGB
                                     //   values in the pixel structure.
        // DDALPHACAPS_PREMULT     | // Supports pixel formats with
                                     //   premultiplied alpha values.
        // DDALPHACAPS_NONPREMULT    | // Supports pixel formats with non-
                                     //   premultiplied alpha values.
#endif
        // DDALPHACAPS_ALPHAFILL   | // Supports color-fill blitting using an
                                     //   alpha value.
        // DDALPHACAPS_ALPHANEG    | // Supports inverted-alpha pixel formats,
                                     //   where 0 indicates fully opaque and
                                     //   255 indicates fully transparent.
        0;

    lpddhi->ddCaps.dwOverlayCaps = // General overlay capabilities.
        DDOVERLAYCAPS_FLIP                | // Supports surface flipping with
                                            //   overlays.
        DDOVERLAYCAPS_FOURCC              | // Supports FOURCC pixel formats
                                            //   with overlays. Use
                                            //   IDirectDraw::GetFourCCCodes to
                                            //   determine which FOURCC formats
                                            //   are supported.
        DDOVERLAYCAPS_ZORDER              | // Supports changing Z order of
                                               //   overlays.
        // DDOVERLAYCAPS_MIRRORLEFTRIGHT     | // Supports surface mirroring in
                                               //   the left-to-right direction
                                               //   for overlays.
        DDOVERLAYCAPS_MIRRORUPDOWN        | // Supports surface mirroring in
                                            //   the up-to-down direction for
                                            //   overlays.
        DDOVERLAYCAPS_CKEYSRC             | // Supports source color keying for
                                            //   overlays.
        // DDOVERLAYCAPS_CKEYSRCCLRSPACE     | // Supports source color-space
                                               //   keying for overlays.
        // DDOVERLAYCAPS_CKEYSRCCLRSPACEYUV  | // Supports source color-space
                                               //   keying for overlays with
                                               //   FOURCC pixel formats.
        DDOVERLAYCAPS_CKEYDEST            | // Supports destination color
                                            //   keying for overlays.
        // DDOVERLAYCAPS_CKEYDESTCLRSPACE    | // Supports destination color-
                                               //   space keying for overlays.
        // DDOVERLAYCAPS_CKEYDESTCLRSPACEYUV | // Supports destination color-
                                               //   space keying for overlays
                                               //   with FOURCC pixel formats.
        // DDOVERLAYCAPS_CKEYBOTH            | // Supports simultaneous source
                                               //   and destination color keying
                                               //   for overlays.
        // DDOVERLAYCAPS_ALPHADEST           | // Supports destination alpha
                                               //   blending for overlays.
         DDOVERLAYCAPS_ALPHASRC              | // Supports source alpha blending
                                               //   for overlays.
        // DDOVERLAYCAPS_ALPHADESTNEG        | // Supports inverted destination
                                               //   alpha blending for overlays.
        // DDOVERLAYCAPS_ALPHASRCNEG         | // Supports inverted source
                                               //   alpha blending for overlays.
        DDOVERLAYCAPS_ALPHACONSTANT          | // Supports constant alpha
                                               //   blending for overlays
                                               //   (specified in the
                                               //   DDOVERLAYFX structure).
        // DDOVERLAYCAPS_ALPHAPREMULT        | // Supports premultiplied alpha
                                               //   pixel formats for overlay
                                               //   alpha blending.
        DDOVERLAYCAPS_ALPHANONPREMULT        | // Supports non-premultiplied
                                               //   alpha pixel formats for
                                               //   overlay alpha blending.
        DDOVERLAYCAPS_ALPHAANDKEYDEST        | // Supports simultaneous source
                                               //   alpha blending with a
                                               //   destination color key for
                                               //   overlays.
        DDOVERLAYCAPS_OVERLAYSUPPORT      | // Supports overlay surfaces.
        0;

    lpddhi->ddCaps.dwMiscCaps = // Miscellaneous video capabilities.
        // DDMISCCAPS_READSCANLINE        | // Supports reading the current
                                            //   scanline being drawn.
        // DDMISCCAPS_READMONITORFREQ     | // Unsupported.
        DDMISCCAPS_READVBLANKSTATUS    | // Supports reading the current
                                         //   V-Blank status of the hardware.
        // DDMISCCAPS_FLIPINTERVAL        | // Supports interval flipping.
        // DDMISCCAPS_FLIPODDEVEN         | // Supports Even/Odd flipping.
        DDMISCCAPS_FLIPVSYNCWITHVBI       | // Supports V-Sync-coordinated
                                            //   flipping.
        // DDMISCCAPS_COLORCONTROLOVERLAY | // Supports color controls on
                                            //   overlay surfaces.
        // DDMISCCAPS_COLORCONTROLPRIMARY | // Supports color controls on
                                            //   primary surfaces.
        // DDMISCCAPS_GAMMACONTROLOVERLAY | // Supports gamma controls on
                                            //   overlay surfaces.
        // DDMISCCAPS_GAMMACONTROLPRIMARY | // Supports gamma controls on
                                            //   primary surfaces.
        0;

    lpddhi->ddCaps.ddsCaps.dwCaps = // Capabilities of the surface.
        // DDSCAPS_ALPHA          | // Indicates that this surface contains
                                    //   alpha-only information.
        DDSCAPS_BACKBUFFER     | // Indicates that this surface is the back
                                 //   buffer of a surface flipping structure.
        DDSCAPS_FLIP           | // Indicates that this surface is a part of
                                 //   a surface flipping structure.
        DDSCAPS_FRONTBUFFER    | // Indicates that this surface is the front
                                 //   buffer of a surface flipping structure.
        DDSCAPS_OVERLAY        | // Indicates that this surface is an overlay.
        // DDSCAPS_PALETTE        |
        DDSCAPS_PRIMARYSURFACE | // Indicates the surface is the primary
                                 //   surface.
        // DDSCAPS_READONLY       | // Indicates that only read access is
                                    //   permitted to the surface. When locking
                                    //   the surface with
                                    //   IDirectDrawSurface::Lock, the
                                    //   DDLOCK_READONLY flag must be specified.
        DDSCAPS_SYSTEMMEMORY   | // Indicates that this surface memory was
                                 //   allocated in system memory.
        DDSCAPS_VIDEOMEMORY    | // Indicates that this surface exists in
                                 //   display memory.
        DDSCAPS_PRIMARYSURFACE2  | // Indicate that surface is the secondary primary
                                   // surface. 
                                   // == DDSCAPS_PRIMARYSURFACE|DDSCAPS_VIDEOPORT
        // DDSCAPS_WRITEONLY      | // Indicates that only write access is
                                  //   permitted to the surface.
        DDSCAPS_OWNDC          |  // Surfaces that own their own DCs, following 2008.1 QFE 
        0;

    lpddhi->lpdwFourCC              = FSLFourCC;   // fourcc codes supported
    lpddhi->ddCaps.dwNumFourCCCodes = MAX_FOURCC;  // number of four cc codes

    lpddhi->ddCaps.dwVidMemTotal  = g_nVideoMemorySize;
    lpddhi->ddCaps.dwVidMemFree   = g_nVideoMemorySize;

    // Note that lpddhi->ddCaps.dwVidMemStride has already been initialized
    // to zero when the entire structure was initially zeroed out and that
    // zero is the correct value for this field since we are using a planar
    // YV12 for video overlay surfaces.

    // The maximum and current number of visible overlays.
    lpddhi->ddCaps.dwMaxVisibleOverlays  = 1;
    lpddhi->ddCaps.dwCurrVisibleOverlays = 0;

    // The minimum and maximum overlay stretch factors (multiplied by 1000).
    lpddhi->ddCaps.dwMinOverlayStretch = 125;
    lpddhi->ddCaps.dwMaxOverlayStretch = 100000;

    // The overlay source and destination rectangle pixel and byte size
    // alignment requirements.
    lpddhi->ddCaps.dwAlignBoundarySrc  = 1;
    lpddhi->ddCaps.dwAlignSizeSrc      = 8;
    lpddhi->ddCaps.dwAlignBoundaryDest = 1;
    lpddhi->ddCaps.dwAlignSizeDest     = 8;

    // Set the bits indicating which ROPS are supported.
    SETROPBIT(lpddhi->ddCaps.dwRops,SRCCOPY);
    SETROPBIT(lpddhi->ddCaps.dwRops,PATCOPY);
    SETROPBIT(lpddhi->ddCaps.dwRops,BLACKNESS);
    SETROPBIT(lpddhi->ddCaps.dwRops,WHITENESS);

    DEBUGMSG(GPE_ZONE_INIT, (TEXT("-DDIPU buildDDHALInfo\r\n")));
}
