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
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  halsurf.cpp
//
//  DirectDrawSurface callback functions.
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


//------------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
// Local Functions


//------------------------------------------------------------------------------
// DDHAL_DDCALLBACKS

//------------------------------------------------------------------------------
//
// Function: HalCreateSurface
//
// This callback function creates a DirectDrawSurface object 
// for this DirectDraw object.  
//
// Parameters:
//      lpcsd
//          [in, out] Pointer to a DDHAL_CREATESURFACEDATA structure 
//          that contains the information necessary to create the 
//          DirectDrawSurface object. 
//
// Returns:
//      Returns one of the following values: 
//          DDHAL_DRIVER_HANDLED 
//          DDHAL_DRIVER_NOTHANDLED 
//
//------------------------------------------------------------------------------
DWORD WINAPI HalCreateSurface( LPDDHAL_CREATESURFACEDATA pd )
{
    DEBUGENTER( HalCreateSurface );
    /*
    typedef struct _DDHAL_CREATESURFACEDATA
    {
        LPDDRAWI_DIRECTDRAW_GBL     lpDD;           // driver struct
        LPDDSURFACEDESC             lpDDSurfaceDesc;// description of surface being created
        LPDDRAWI_DDRAWSURFACE_LCL   FAR *lplpSList; // list of created surface objects
        DWORD                       dwSCnt;         // number of surfaces in SList
        HRESULT                     ddRVal;         // return value
        LPDDHAL_CREATESURFACE       CreateSurface;  // PRIVATE: ptr to callback
    } DDHAL_CREATESURFACEDATA;
    */

    DWORD result = DDHAL_DRIVER_HANDLED;

    // Implementation
    if(pd->lpDDSurfaceDesc->ddsCaps.dwCaps & (DDSCAPS_FLIP | DDSCAPS_OVERLAY))
    {
        DWORD *dwCapsList = (DWORD *)malloc(sizeof(DWORD) * pd->dwSCnt);
        DWORD iSurf = 0;

        if(NULL == dwCapsList)
        {
            DEBUGMSG(HAL_ZONE_ERROR, (TEXT("HalCreateSurface: NO LOCAL HEAP MEMORY!!!\r\n")));
            pd->ddRVal = DDERR_OUTOFMEMORY;
            return DDHAL_DRIVER_HANDLED;
        }

        // Force all flip or overlay to be in video memory
        for(iSurf = 0; iSurf < pd->dwSCnt; iSurf++)
        {
            dwCapsList[iSurf] = pd->lplpSList[iSurf]->ddsCaps.dwCaps;
            pd->lplpSList[iSurf]->ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
        }

        result = DDGPECreateSurface(pd);

        // Restore back the DDSCAPS parameters
        for(iSurf = 0; iSurf < pd->dwSCnt; iSurf++)
        {
            pd->lplpSList[iSurf]->ddsCaps.dwCaps = dwCapsList[iSurf];
        }

        free(dwCapsList);
    }
    else
    {
        result = DDGPECreateSurface(pd);
    }

    DEBUGMSG(HAL_ZONE_CREATE, (TEXT("HalCreateSurface: surface create %s\r\n"), (pd->ddRVal == DD_OK) ? L"successful" : L"failed"));

    return result;
}



//------------------------------------------------------------------------------
//
// Function: HalCanCreateSurface
//
// This function indicates whether a surface can be created. 
//
// Parameters:
//      lpccsd 
//          [in, out] Pointer to a DDHAL_CANCREATESURFACEDATA structure 
//          that contains the information required for the driver to 
//          determine whether a surface can be created. 
//
// Returns:
//      Returns one of the following values: 
//          DDHAL_DRIVER_HANDLED 
//          DDHAL_DRIVER_NOTHANDLED 
//
//------------------------------------------------------------------------------
DWORD WINAPI HalCanCreateSurface( LPDDHAL_CANCREATESURFACEDATA pd )
{
    DEBUGENTER( HalCanCreateSurface );
    /*
    typedef struct _DDHAL_CANCREATESURFACEDATA
    {
        LPDDRAWI_DIRECTDRAW_GBL    lpDD;                // driver struct
        LPDDSURFACEDESC            lpDDSurfaceDesc;     // description of surface being created
        DWORD                    bIsDifferentPixelFormat;
                                                        // pixel format differs from primary surface
        HRESULT                    ddRVal;              // return value
        LPDDHAL_CANCREATESURFACE    CanCreateSurface;
                                                        // PRIVATE: ptr to callback
    } DDHAL_CANCREATESURFACEDATA;
    */

    // We only have one pixel format due to the LCDC of i.MX27 processor
    if(pd->bIsDifferentPixelFormat)
    {
        DEBUGMSG(HAL_ZONE_ERROR, (TEXT("HalCanCreateSurface: the requested surface want different format!\r\n")));
        goto CannotCreate;
    }

#if 0  // We should NOT enable this check, because we can force video memory later in HalCreateSurface().
    // If the surface is used for page flipping or overlay, it must be in video memory
    DWORD caps = pd->lpDDSurfaceDesc->ddsCaps.dwCaps;

    if(caps & (DDSCAPS_FLIP | DDSCAPS_OVERLAY))
    {
        if(caps & DDSCAPS_VIDEOMEMORY)
        {
            // Flip or overlay surface is video memory, we can create!
        }
        else
        {
            DEBUGMSG(HAL_ZONE_ERROR, (TEXT("HalCanCreateSurface: flip or overlay surface not in video memory\r\n"), caps));
            goto CannotCreate;
        }
    }
#endif

//CanCreate:
    DEBUGMSG(HAL_ZONE_CREATE, (TEXT("HalCanCreateSurface: OK\r\n")));
    pd->ddRVal = DD_OK;
    return DDHAL_DRIVER_HANDLED;

CannotCreate:
    DEBUGMSG(HAL_ZONE_CREATE, (TEXT("HalCanCreateSurface: Unsupported\r\n")));
    pd->ddRVal = DDERR_UNSUPPORTEDFORMAT;
    return DDHAL_DRIVER_HANDLED;
}


//------------------------------------------------------------------------------
// DDHAL_DDSURFACECALLBACKS

//------------------------------------------------------------------------------
//
// Function: HalFlip
//
// This callback function associates the surface memory 
// for the back buffer with the surface memory for the 
// front buffer.
//
// Parameters:
//      pfd 
//          [in, out] Pointer to a DDHAL_FLIPDATA structure that contains 
//          information necessary to perform a flip. 
//
// Returns:
//      Returns one of the following values: 
//          DDHAL_DRIVER_HANDLED 
//          DDHAL_DRIVER_NOTHANDLED 
//
//------------------------------------------------------------------------------
DWORD WINAPI HalFlip( LPDDHAL_FLIPDATA pd )
{
    //DEBUGENTER( HalFlip );
    /*
    typedef struct _DDHAL_FLIPDATA
    {
        LPDDRAWI_DIRECTDRAW_GBL     lpDD;           // driver struct
        LPDDRAWI_DDRAWSURFACE_LCL   lpSurfCurr;     // current surface
        LPDDRAWI_DDRAWSURFACE_LCL   lpSurfTarg;     // target surface (to flip to)
        DWORD                       dwFlags;        // flags
        HRESULT                     ddRVal;         // return value
        LPDDHALSURFCB_FLIP          Flip;           // PRIVATE: ptr to callback
    } DDHAL_FLIPDATA;
    */

    return ((MXDDLcdc *)GetDDGPE())->Flip(pd);
}



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
DWORD WINAPI HalGetBltStatus( LPDDHAL_GETBLTSTATUSDATA pd )
{
    DEBUGENTER( HalGetBltStatus );
    /*
    typedef struct _DDHAL_GETBLTSTATUSDATA
    {
        LPDDRAWI_DIRECTDRAW_GBL     lpDD;           // driver struct
        LPDDRAWI_DDRAWSURFACE_LCL   lpDDSurface;    // surface struct
        DWORD                       dwFlags;        // flags
        HRESULT                     ddRVal;         // return value
        LPDDHALSURFCB_GETBLTSTATUS  GetBltStatus;   // PRIVATE: ptr to callback
    } DDHAL_GETBLTSTATUSDATA;
    */

    // Implementation
    pd->ddRVal = DD_OK;

    if (pd->dwFlags & DDGBS_CANBLT)
    {
        if (((MXDDLcdc *)GetDDGPE())->IsBusy())
        {
            pd->ddRVal = DDERR_WASSTILLDRAWING;
        }
    }
    else if (pd->dwFlags & DDGBS_ISBLTDONE)
    {
        if (((MXDDLcdc *)GetDDGPE())->IsBusy())
        {
            pd->ddRVal = DDERR_WASSTILLDRAWING;
        }
    }

    return DDHAL_DRIVER_HANDLED;
}



//------------------------------------------------------------------------------
//
// Function: HalUpdateOverlay
//
// This function repositions or modifies the visual 
// attributes of an overlay surface.  
//
// Parameters:
//      pd 
//          [in, out] Pointer to a DDHAL_UPDATEOVERLAYDATA structure that 
//          contains the information required to update the overlay surface. 
//
// Returns:
//      Returns one of the following values: 
//          DDHAL_DRIVER_HANDLED 
//          DDHAL_DRIVER_NOTHANDLED 
//
//------------------------------------------------------------------------------
DWORD HalUpdateOverlay( LPDDHAL_UPDATEOVERLAYDATA pd )
{
    DEBUGENTER( HalUpdateOverlay );
    /*
    typedef struct _DDHAL_UPDATEOVERLAYDATA
    {
        LPDDRAWI_DIRECTDRAW_GBL     lpDD;           // driver struct
        LPDDRAWI_DDRAWSURFACE_LCL   lpDDDestSurface;// dest surface
        RECTL                       rDest;          // dest rect
        LPDDRAWI_DDRAWSURFACE_LCL   lpDDSrcSurface; // src surface
        RECTL                       rSrc;           // src rect
        DWORD                       dwFlags;        // flags
        DDOVERLAYFX                 overlayFX;      // overlay FX
        HRESULT                     ddRVal;         // return value
        LPDDHALSURFCB_UPDATEOVERLAY UpdateOverlay;  // PRIVATE: ptr to callback
    } DDHAL_UPDATEOVERLAYDATA;
    */

    // Implementation
    return ((MXDDLcdc *)GetDDGPE())->UpdateOverlay(pd);
}


//------------------------------------------------------------------------------
//
// Function: HalSetOverlayPosition
//
// This function changes the display coordinates of an overlay surface. 
//
// Parameters:
//      pd 
//          [in, out] Pointer to a DDHAL_SETOVERLAYPOSITIONDATA structure 
//          that contains the information required to change the display 
//          coordinates of an overlay surface. 
//
// Returns:
//      Returns one of the following values: 
//          DDHAL_DRIVER_HANDLED 
//          DDHAL_DRIVER_NOTHANDLED 
//
//------------------------------------------------------------------------------
DWORD HalSetOverlayPosition( LPDDHAL_SETOVERLAYPOSITIONDATA pd )
{
    DEBUGENTER( HalSetOverlayPosition );
    /*
    typedef struct _DDHAL_SETOVERLAYPOSITIONDATA
    {
        LPDDRAWI_DIRECTDRAW_GBL     lpDD;           // driver struct
        LPDDRAWI_DDRAWSURFACE_LCL   lpDDSrcSurface; // src surface
        LPDDRAWI_DDRAWSURFACE_LCL   lpDDDestSurface;// dest surface
        LONG                        lXPos;          // x position
        LONG                        lYPos;          // y position
        HRESULT                     ddRVal;         // return value
        LPDDHALSURFCB_SETOVERLAYPOSITION SetOverlayPosition;
                                                    // PRIVATE: ptr to callback
    } DDHAL_SETOVERLAYPOSITIONDATA;
    */

    // Implementation
    return ((MXDDLcdc *)GetDDGPE())->SetOverlayPosition(pd);
}

//------------------------------------------------------------------------------
//
// Function: HalSetColorKey
//
// This function sets the surface's colorkey. We must override the default
// DDGPESetColorKey() implementation in order to support hardware accelerated
// flipping of both RGB and YUV overlay surfaces through our other "Hal"
// functions.
//
// Parameters:
//      pd
//          [in, out] Pointer to a DDHAL_SETCOLORKEYDATA structure
//          that contains the information required to change the colorkey
//          settings of a surface.
//
// Returns:
//      Returns one of the following values:
//          DDHAL_DRIVER_HANDLED
//          DDHAL_DRIVER_NOTHANDLED
//
//------------------------------------------------------------------------------
DWORD WINAPI HalSetColorKey( LPDDHAL_SETCOLORKEYDATA pd )
{
    DDGPESurf* pSurf = DDGPESurf::GetDDGPESurf(pd->lpDDSurface);
    if (pSurf != NULL)
    {
        // The default implementation of DDGPESetColorKey() will only set the
        // surface's new colorkey values if the DDCKEY_SRCBLT flag is set.
        // However, this flag is only set for RGB surfaces and not for YUV
        // surfaces. Therefore, we must skip that check altogether and just
        // do what is needed and then return DD_OK if we want to be able
        // to be able to use our IPU accelerated functions HalUpdateOverlay()
        // and HalFlip().

        pSurf->SetColorKeyLow(pd->ckNew.dwColorSpaceLowValue);
        pSurf->SetColorKeyHigh(pd->ckNew.dwColorSpaceHighValue);

        pd->ddRVal = DD_OK;
    }
    else
    {
        pd->ddRVal = DDERR_INVALIDOBJECT;
    }

    return DDHAL_DRIVER_HANDLED;
}
