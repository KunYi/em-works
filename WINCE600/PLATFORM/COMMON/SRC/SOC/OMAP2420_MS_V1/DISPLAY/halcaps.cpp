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
//------------------------------------------------------------------------------
//
//  File:  halcaps.cpp
//
#include "precomp.h"

// system memory is used as video memory, so this number is arbitrary
#define VIDEO_MEMORY_SIZE   0x01000000 // (4 * 1024 * 1024)

//------------------------------------------------------------------------------
//
// Supported FOURCC codes

#define FOURCC_YUV422       MAKEFOURCC('U', 'Y', 'V', 'Y')

DWORD aFourCC[] =
{
    FOURCC_YUV422,
};

#define NUM_FOURCC_CODES    (sizeof(aFourCC) / sizeof(DWORD))

//------------------------------------------------------------------------------
//
// callbacks from the DIRECTDRAW object

static DDHAL_DDCALLBACKS cbDDCallbacks =
{
    sizeof(DDHAL_DDCALLBACKS),
    DDHAL_CB32_CREATESURFACE|DDHAL_CB32_CANCREATESURFACE|DDHAL_CB32_CREATEPALETTE,
    DDGPECreateSurface,
    NULL, // WaitForVerticalBlank
    DDGPECanCreateSurface,
    DDGPECreatePalette,
    NULL // GetScanLine
};

//------------------------------------------------------------------------------
//
// Callbacks from the DIRECTDRAWMISCELLANEOUS object

static DDHAL_DDMISCELLANEOUSCALLBACKS MiscellaneousCallbacks =
{
    sizeof(DDHAL_DDMISCELLANEOUSCALLBACKS),
    0,      // flags
    NULL,   // HalGetAvailDriverMemory,
    NULL    // HalGetDeviceIdentifier
};

//------------------------------------------------------------------------------
//
// Callbacks from the DIRECTDRAWPALETTE object

static DDHAL_DDPALETTECALLBACKS cbDDPaletteCallbacks =
{
    sizeof(DDHAL_DDPALETTECALLBACKS),
    DDHAL_PALCB32_DESTROYPALETTE|DDHAL_PALCB32_SETENTRIES,
    DDGPEDestroyPalette,
    DDGPESetEntries
};

//------------------------------------------------------------------------------
//
// Callbacks from the DIRECTDRAWSURFACE object

static DDHAL_DDSURFACECALLBACKS cbDDSurfaceCallbacks =
{
    sizeof(DDHAL_DDSURFACECALLBACKS),
    DDHAL_SURFCB32_DESTROYSURFACE|DDHAL_SURFCB32_FLIP|DDHAL_SURFCB32_LOCK|DDHAL_SURFCB32_UNLOCK|DDHAL_SURFCB32_SETCOLORKEY|
        DDHAL_SURFCB32_GETBLTSTATUS|DDHAL_SURFCB32_GETFLIPSTATUS|DDHAL_SURFCB32_UPDATEOVERLAY|DDHAL_SURFCB32_SETOVERLAYPOSITION|
        DDHAL_SURFCB32_SETPALETTE,
    DDGPEDestroySurface,
    HalFlip,
    HalLock,
    HalUnlock,
    DDGPESetColorKey,
    HalGetBltStatus,
    HalGetFlipStatus,
    HalUpdateOverlay,
    HalSetOverlayPosition,
    DDGPESetPalette
};

//------------------------------------------------------------------------------
//
// buildDDHALInfo

EXTERN_C VOID buildDDHALInfo(LPDDHALINFO lpddhi, DWORD modeIdx)
{
    OMAP2420GPE *pOMAP2420GPE = static_cast<OMAP2420GPE*>(GetDDGPE());
    pOMAP2420GPE->BuildDDHALInfo(lpddhi, modeIdx);
}

//------------------------------------------------------------------------------
//
// OMAP2420GPE::BuildDDHALInfo

VOID OMAP2420GPE::BuildDDHALInfo(DDHALINFO *pInfo, DWORD modeIdx)
{
    // clear the DDHALINFO structure and set the sizes
    memset(pInfo, 0, sizeof(DDHALINFO));
    pInfo->dwSize = sizeof(DDHALINFO);
    pInfo->ddCaps.dwSize = sizeof(DDCAPS);

    // set the callback pointers
    pInfo->lpDDCallbacks = &cbDDCallbacks;
    pInfo->lpDDSurfaceCallbacks = &cbDDSurfaceCallbacks;
    pInfo->lpDDPaletteCallbacks = &cbDDPaletteCallbacks;
    pInfo->GetDriverInfo = HalGetDriverInfo;

    // set the video memory sizes
    pInfo->ddCaps.dwVidMemTotal = VIDEO_MEMORY_SIZE;
    pInfo->ddCaps.dwVidMemFree = VIDEO_MEMORY_SIZE;

    // set the overlay info
//    pInfo->ddCaps.dwMaxVisibleOverlays = 1;
    pInfo->ddCaps.dwCurrVisibleOverlays = 0;
    //	pInfo->ddCaps.dwMinOverlayStretch = 500;
    //	pInfo->ddCaps.dwMaxOverlayStretch = 8000;
    pInfo->ddCaps.dwMinOverlayStretch = 1000;
    pInfo->ddCaps.dwMaxOverlayStretch = 1000;

    // set some misc info
    pInfo->ddCaps.dwNumFourCCCodes = 0; //NUM_FOURCC_CODES;
    pInfo->ddCaps.dwAlignBoundarySrc = 0;
    pInfo->ddCaps.dwAlignSizeSrc = 0;
//    pInfo->lpdwFourCC = aFourCC;
    pInfo->dwFlags = 0;
    
    // set the general surface capabilities
    pInfo->ddCaps.ddsCaps.dwCaps = (DDSCAPS_BACKBUFFER|
                                    DDSCAPS_FLIP|
                                    DDSCAPS_FRONTBUFFER|
//                                    DDSCAPS_OVERLAY|
                                    DDSCAPS_PRIMARYSURFACE|
                                    DDSCAPS_SYSTEMMEMORY|
                                    DDSCAPS_VIDEOMEMORY|
                                    0);

    // set the color key capabilities of the surface
    pInfo->ddCaps.dwCKeyCaps = DDCKEYCAPS_SRCBLT|
                               //DDCKEYCAPS_DESTBLT|
                               0;

    // set the palette capabilities
    pInfo->ddCaps.dwPalCaps = 0; // DDPCAPS_PRIMARYSURFACE

    // set the BLT capabilities
    pInfo->ddCaps.dwBltCaps = DDBLTCAPS_READSYSMEM | DDBLTCAPS_WRITESYSMEM;

    // set the overlay capabilities
    //pInfo->ddCaps.dwOverlayCaps = DDOVERLAYCAPS_OVERLAYSUPPORT|DDOVERLAYCAPS_FLIP|DDOVERLAYCAPS_CKEYSRC|DDOVERLAYCAPS_CKEYDEST;

    // set the supported ROPS
    SETROPBIT(pInfo->ddCaps.dwRops, SRCCOPY);
    SETROPBIT(pInfo->ddCaps.dwRops, PATCOPY);
    SETROPBIT(pInfo->ddCaps.dwRops, BLACKNESS);
    SETROPBIT(pInfo->ddCaps.dwRops, WHITENESS);
}
