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
// -----------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
// -----------------------------------------------------------------------------
//
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  halcaps.cpp
//
//  Hardware abstraction layer capabilities.
//
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include "precomp.h"
#include "dispperf.h"
#pragma warning(pop)

//#undef DEBUGMSG
//#define DEBUGMSG(cond, msg) RETAILMSG(1,msg)

// callbacks from the DIRECTDRAW object

//------------------------------------------------------------------------------
// Global Variables

DWORD FSLFourCC[] = {
    FOURCC_YV12,
};
#define MAX_FOURCC        sizeof(FSLFourCC)/sizeof(DWORD)

DDHAL_DDCALLBACKS cbDDCallbacks =
{
    sizeof(DDHAL_DDCALLBACKS),
    DDHAL_CB32_CREATESURFACE        |
    DDHAL_CB32_WAITFORVERTICALBLANK | //DDHAL_CB32_WAITFORVERTICALBLANK |
    DDHAL_CB32_CANCREATESURFACE     |
    DDHAL_CB32_CREATEPALETTE        | //DDHAL_CB32_CREATEPALETTE        |
    //DDHAL_CB32_GETSCANLINE          |
    0,
    HalCreateSurface,
    DDGPEWaitForVerticalBlank, //NULL,
    DDGPECanCreateSurface,
    DDGPECreatePalette, //NULL,
    NULL
};
////////
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
////////
// callbacks from the DIRECTDRAWSURFACE object

DDHAL_DDSURFACECALLBACKS cbDDSurfaceCallbacks =
{
    sizeof(DDHAL_DDSURFACECALLBACKS),
    DDHAL_SURFCB32_DESTROYSURFACE     |
    DDHAL_SURFCB32_FLIP               |
    DDHAL_SURFCB32_LOCK               |
    DDHAL_SURFCB32_UNLOCK             |
    DDHAL_SURFCB32_SETCOLORKEY        |
    DDHAL_SURFCB32_GETBLTSTATUS       |
    //DDHAL_SURFCB32_GETFLIPSTATUS      |
    DDHAL_SURFCB32_UPDATEOVERLAY      |
    DDHAL_SURFCB32_SETOVERLAYPOSITION |
    DDHAL_SURFCB32_SETPALETTE         | //DDHAL_SURFCB32_SETPALETTE         |
    0,
    DDGPEDestroySurface,
    HalFlip,
    DDGPELock,
    DDGPEUnlock,
    HalSetColorKey,
    HalGetBltStatus,
    DDGPEGetFlipStatus, //NULL,
    HalUpdateOverlay,
    HalSetOverlayPosition,
    DDGPESetPalette, //NULL
};

// InitDDHALInfo must set up this information
unsigned long g_nVideoMemorySize = 0L;
unsigned char* g_pVideoMemory = NULL; // virtual address of video memory from client's side
DWORD g_nTransparentColor = 0L;

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
    DEBUGENTER(HalGetDriverInfo);

    // The flat driver does not implement any of the exteneded DirectDraw
    // caps or callbacks.

    if ( lpInput->guidInfo == GUID_GetDriverInfo_VidMemBase )
    {
        // Map all of videomemory into the client app.

        DDGPE * pDdGpe = GetDDGPE();

        unsigned long VideoMemoryBase = 0;
        unsigned long VideoMemorySize = 0;

        pDdGpe->GetPhysicalVideoMemory( &VideoMemoryBase, &VideoMemorySize );

        *reinterpret_cast< DWORD * >(lpInput->lpvData) = VideoMemoryBase;
        lpInput->dwActualSize = sizeof ( DWORD );
        lpInput->ddRVal       = DD_OK;
    }
    else
    {
        lpInput->ddRVal = DDERR_CURRENTLYNOTAVAIL;
    }

    DEBUGLEAVE(HalGetDriverInfo);

    return DDHAL_DRIVER_HANDLED;
} //HalGetDriverInfo

//------------------------------------------------------------------------------
//
// Function: HalGetBltStatus
//
// This function determines whether the blit queue
// has room for more blits and whether the blit is busy.
//
// Parameters:
//      pd
//          [in, out] Pointer to a DDHAL_GETBLTSTATUSDATA structure that
//          returns the blits status information.
//
// Returns:
//      Returns one of the following values:
//          DDHAL_DRIVER_HANDLED
//          DDHAL_DRIVER_NOTHANDLED
//
//------------------------------------------------------------------------------
DWORD WINAPI HalGetBltStatus(LPDDHAL_GETBLTSTATUSDATA pd)
{
    DEBUGENTER(HalGetBltStatus);  
    pd->ddRVal = DD_OK;

    if (pd->dwFlags & (DDGBS_CANBLT | DDGBS_ISBLTDONE)) {
        if (GetDDGPE()->IsBusy())
            pd->ddRVal = DDERR_WASSTILLDRAWING;
    } //if

    DEBUGLEAVE(HalGetBltStatus);

    return DDHAL_DRIVER_HANDLED;
} //HalGetBltStatus

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
EXTERN_C void buildDDHALInfo(LPDDHALINFO lpddhi, DWORD modeidx)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(modeidx);

    DEBUGMSG(1, (L"buildDDHALInfo enter\r\n"));
    if(!g_pVideoMemory) {  // in case this is called more than once...
        unsigned long VideoMemoryStart;

        GetDDGPE()->GetPhysicalVideoMemory(&VideoMemoryStart,
                                           &g_nVideoMemorySize);

        g_pVideoMemory = (BYTE*) VideoMemoryStart;
    } //if

    memset(lpddhi, 0, sizeof(DDHALINFO));

    lpddhi->dwSize = sizeof(DDHALINFO);

    lpddhi->lpDDCallbacks = &cbDDCallbacks;
    lpddhi->lpDDSurfaceCallbacks = &cbDDSurfaceCallbacks;
    lpddhi->lpDDPaletteCallbacks = &cbDDPaletteCallbacks;   //
    lpddhi->GetDriverInfo = HalGetDriverInfo;

    // Capability bits.

    lpddhi->ddCaps.dwSize = sizeof(DDCAPS);

    lpddhi->ddCaps.dwVidMemTotal = g_nVideoMemorySize;
    lpddhi->ddCaps.dwVidMemFree = g_nVideoMemorySize;
    lpddhi->ddCaps.dwVidMemStride = 0;

    lpddhi->ddCaps.ddsCaps.dwCaps = (DDSCAPS_ALPHA |            // Can create alpha-only surfaces
                                     DDSCAPS_FRONTBUFFER    |   // Can create front-buffer surfaces
                                     DDSCAPS_BACKBUFFER     |   // Can create backbuffer surfaces
                                     DDSCAPS_FLIP           |   // Can flip between surfaces
                                     DDSCAPS_PRIMARYSURFACE |   // Has a primary surface
                                     DDSCAPS_OVERLAY |          // Can create overlay surfaces
                                     DDSCAPS_VIDEOMEMORY |      // Surfaces are in video memory
                                     DDSCAPS_SYSTEMMEMORY |
                                     DDSCAPS_OWNDC);

    lpddhi->ddCaps.dwPalCaps =         
        DDPCAPS_ALPHA |                                 // Supports palettes that include an alpha component
        DDPCAPS_PRIMARYSURFACE |                        // Indicates that the palette is attached to the primary surface
        0;


    // DirectDraw Blttting caps refer to hardware blitting support only.

    // WBB: Does this need to change when we do hardware blit?
    lpddhi->ddCaps.dwBltCaps =
        DDBLTCAPS_READSYSMEM |                          // Supports blitting from system memory.
        DDBLTCAPS_WRITESYSMEM |                         // Supports blitting to system memory.
        // DDBLTCAPS_FOURCCTORGB |                      // Supports blitting from a surface with a FOURCC pixel format to a surface with an RGB pixel format.
        // DDBLTCAPS_COPYFOURCC |                       // Supports blitting from a surface with a FOURCC pixel format to another surface with the same pixel format, or to the same surface.
        // DDBLTCAPS_FILLFOURCC |                       // Supports color-fill blitting to a surface with a FOURCC pixel format.
        0;
    lpddhi->ddCaps.dwCKeyCaps =
        // DDCKEYCAPS_BOTHBLT |                         // Supports transparent blitting with for both source and destination surfaces.
        DDCKEYCAPS_DESTBLT |                            // Supports transparent blitting with a color key that identifies the replaceable bits of the destination surface for RGB colors.
        // DDCKEYCAPS_DESTBLTCLRSPACE |                 // Supports transparent blitting with a color space that identifies the replaceable bits of the destination surface for RGB colors.
        // DDCKEYCAPS_DESTBLTCLRSPACEYUV |              // Supports transparent blitting with a color space that identifies the replaceable bits of the destination surface for YUV colors.
        DDCKEYCAPS_SRCBLT |                                // Supports transparent blitting using the color key for the source with this surface for RGB colors.
        // DDCKEYCAPS_SRCBLTCLRSPACE |                  // Supports transparent blitting using a color space for the source with this surface for RGB colors.
        // DDCKEYCAPS_SRCBLTCLRSPACEYUV |               // Supports transparent blitting using a color space for the source with this surface for YUV colors.
        0;
    lpddhi->ddCaps.dwAlphaCaps = 0;

    // Overlay caps.

    lpddhi->ddCaps.dwOverlayCaps = // General overlay capabilities.
                DDOVERLAYCAPS_FLIP |                            // Supports surface flipping with overlays.
                DDOVERLAYCAPS_CKEYSRC |                         // Supports source color keying for overlays.
                DDOVERLAYCAPS_CKEYSRCCLRSPACE |                 // Supports source color-space keying for overlays.
                DDOVERLAYCAPS_CKEYDEST |                        // Supports destination color keying for overlays.
                //DDOVERLAYCAPS_ALPHADEST |                       // Supports destination alpha blending for overlays.
                DDOVERLAYCAPS_ALPHASRC |                        // Supports source alpha blending for overlays.
                //DDOVERLAYCAPS_ALPHASRCNEG |                     // Supports inverted source alpha blending for overlays.
                DDOVERLAYCAPS_ALPHACONSTANT |                   // Supports constant alpha blending for overlays (specified in the DDOVERLAYFX structure).
                DDOVERLAYCAPS_OVERLAYSUPPORT      | // Supports overlay surfaces.
                DDOVERLAYCAPS_CKEYDESTCLRSPACE  |   // Supports destination color-space keying for overlays.
                DDOVERLAYCAPS_FOURCC              | // Supports FOURCC pixel formats with overlays. Use IDirectDraw::GetFourCCCodes to determine which FOURCC formats are supported.
                DDOVERLAYCAPS_MIRRORUPDOWN        | // Supports surface mirroring in the up-to-down direction for overlays.
                DDOVERLAYCAPS_MIRRORLEFTRIGHT     | // Supports surface mirroring in the left-to-right direction for overlays.
                DDOVERLAYCAPS_ALPHANONPREMULT        | // Supports non-premultiplied alpha pixel formats for overlay alpha blending.
                DDOVERLAYCAPS_ALPHAANDKEYDEST        | // Supports simultaneous source alpha blending with a destination color key for overlays.                
                0;
    lpddhi->ddCaps.dwMaxVisibleOverlays = 1;
    lpddhi->ddCaps.dwCurrVisibleOverlays = 0;

    DWORD dwAlign = ((DDLcdif *)GetDDGPE())->GetOverlayAlign();
    DEBUGMSG(1,(_T("dwAlign %d\r\n"),dwAlign));
    lpddhi->ddCaps.dwAlignBoundarySrc = dwAlign;
    lpddhi->ddCaps.dwAlignSizeSrc = dwAlign;
    lpddhi->ddCaps.dwAlignBoundaryDest = dwAlign;
    lpddhi->ddCaps.dwAlignSizeDest = dwAlign;

    DWORD dwDownScaleExp = ((DDLcdif *)GetDDGPE())->GetDownScaleFactorExp();
    lpddhi->ddCaps.dwMinOverlayStretch = 1000 >> dwDownScaleExp;  // Min Overlay Stretch factor
    lpddhi->ddCaps.dwMaxOverlayStretch = 4096000;  // Max Overlay Stretch factor

    // more driver specific capabilities
    lpddhi->ddCaps.dwMiscCaps =
        DDMISCCAPS_READVBLANKSTATUS |                   // Supports reading the current
        //   V-Blank status of the hardware.
        DDMISCCAPS_FLIPVSYNCWITHVBI    ;                // Supports V-Sync-coordinated
    //   flipping.0;


    SETROPBIT(lpddhi->ddCaps.dwRops, SRCCOPY);          // Set bits for ROPS supported
    SETROPBIT(lpddhi->ddCaps.dwRops, PATCOPY);
    SETROPBIT(lpddhi->ddCaps.dwRops, BLACKNESS);
    SETROPBIT(lpddhi->ddCaps.dwRops, WHITENESS);

    // - Video port capabilities
    lpddhi->ddCaps.dwMinVideoStretch = 0;               // minimum video port stretch factor multiplied by 1000, eg 1000 == 1.0, 1300 == 1.3
    lpddhi->ddCaps.dwMaxVideoStretch = 0;               // maximum video port stretch factor multiplied by 1000, eg 1000 == 1.0, 1300 == 1.3
    lpddhi->ddCaps.dwMaxVideoPorts = 0;                 // maximum number of usable video ports
    lpddhi->ddCaps.dwCurrVideoPorts = 0;                // current number of video ports used

    // Fourcc
    lpddhi->lpdwFourCC = FSLFourCC;               // fourcc codes supported
    lpddhi->ddCaps.dwNumFourCCCodes = MAX_FOURCC;  // number of four cc codes

#if 0
    (    // The driver allows the current vertical
        // blank status to be read
        DDMISCCAPS_READVBLANKSTATUS |
        // The driver is able to synchronize flips
        // with the vertical blanking period
        DDMISCCAPS_FLIPVSYNCWITHVBI);
#endif

    DEBUGMSG( 1,(L"buildDDHALInfo exit\r\n"));
} //buildDDHALInfo

