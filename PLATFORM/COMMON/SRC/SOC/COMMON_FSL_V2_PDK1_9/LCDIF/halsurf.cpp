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
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
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
//#undef DEBUGMSG
//#define DEBUGMSG(cond, msg) RETAILMSG(1,msg)

//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Global Variables


//------------------------------------------------------------------------------
// Local Variables

//------------------------------------------------------------------------------
// External Functions
EXTERN_C SCODE DDGPEGetPixelFormatFromSurfaceDesc(
                    LPDDSURFACEDESC            lpDDSurfaceDesc,
                    EDDGPEPixelFormat*         pPixelFormat,
                    EGPEFormat*                pFormat
                    );

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
//      pd
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
    DEBUGENTER(HalCreateSurface);

    SCODE                     sc = S_OK;
    unsigned int              iSurf;
    LPDDRAWI_DDRAWSURFACE_LCL pSurf;
    EDDGPEPixelFormat         pixelFormat;
    EGPEFormat                format;
    DWORD                     dwCaps;

    unsigned int              nWidth  = pd->lpDDSurfaceDesc->dwWidth; 
    unsigned int              nHeight = pd->lpDDSurfaceDesc->dwHeight;

    unsigned long             offset;

    DDGPE *                   pDDGPE = GetDDGPE();

    if (pd->lpDDSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_OVERLAY)
    {
        sc = DDGPEGetPixelFormatFromSurfaceDesc(pd->lpDDSurfaceDesc,
            &pixelFormat,
            &format);    
        if (FAILED(sc)) {

            DEBUGMSG(GPE_ZONE_WARNING,(TEXT("HalCreateSurface ERROR - DDERR_UNSUPPORTEDFORMAT (0x%08x)\r\n"), sc ));
            pd->ddRVal = DDERR_UNSUPPORTEDFORMAT;
            DEBUGLEAVE(HalCreateSurface);
            return DDHAL_DRIVER_HANDLED;
        }

        for( iSurf=0; iSurf<pd->dwSCnt; iSurf++ ) {

            pSurf = pd->lplpSList[iSurf];
            dwCaps = pSurf->ddsCaps.dwCaps;

            sc = pDDGPE->AllocVideoSurface(
                pSurf,
                nWidth,
                nHeight,
                format,
                pixelFormat,
                &offset
                );

            if( FAILED( sc ) )
            {
                pd->ddRVal = sc;
                DEBUGMSG(GPE_ZONE_ERROR,(TEXT("Failed to create surface, sc=0x%08x\r\n"),sc));
                return DDHAL_DRIVER_HANDLED;
            }
            else{
                DDGPESurf * pOverlay = DDGPESurf::GetDDGPESurf(pSurf);

                pOverlay->SetOverlay();

                //The m_ScreenWidth and m_ScreenHeight won't change after this calling for overlay surface
                pOverlay->SetRotation(pOverlay->Width(),
                    pOverlay->Height(),
                    pOverlay->Rotate());
            }
            pSurf->ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;

        } // end of for loop

        // Reach back into the surface description to fill out the surface stride.
        // (All surfaces in a flipping chain should have the same stride.)

        pSurf = pd->lplpSList[0];
        if (pSurf)
        {
            int nStrideBytes = DDGPESurf::GetDDGPESurf(pSurf)->Stride();
            int BytesPerPixel = DDGPESurf::GetDDGPESurf(pSurf)->BytesPerPixel();
            pd->lpDDSurfaceDesc->lPitch = nStrideBytes;
            pd->lpDDSurfaceDesc->lXPitch = BytesPerPixel;

            pd->lpDDSurfaceDesc->dwSurfaceSize = DDGPESurf::GetDDGPESurf(pSurf)->SurfaceSize();
            pd->lpDDSurfaceDesc->dwFlags |= (DDSD_PITCH | DDSD_XPITCH | DDSD_SURFACESIZE);
            pd->lpDDSurfaceDesc->ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
        }
        pd->ddRVal = DD_OK;
    }
    else
        DDGPECreateSurface(pd);

    DEBUGMSG(GPE_ZONE_CREATE, (TEXT("HalCreateSurface: surface create %s\r\n"), (pd->ddRVal == DD_OK) ? L"successful" : L"failed"));
    DEBUGLEAVE(HalCreateSurface);
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
//      pd 
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
    DEBUGENTER( HalFlip );
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
    return ((DDLcdif *)GetDDGPE())->Flip(pd);
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
    return ((DDLcdif *)GetDDGPE())->UpdateOverlay(pd);
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
    return ((DDLcdif *)GetDDGPE())->SetOverlayPosition(pd);
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
    DEBUGMSG(1,(_T("HalSetColorKey")));

    DDGPESurf* pSurf = DDGPESurf::GetDDGPESurf(pd->lpDDSurface);

    DEBUGMSG(1,(_T("HalSetColorKey offset=0x%x"),pSurf->OffsetInVideoMemory()));

    DEBUGMSG(1,(_T("pd->dwFlags = 0x%x"),pd->dwFlags));
    DEBUGMSG(1,(_T("low = 0x%x, high = 0x%x"),pd->ckNew.dwColorSpaceLowValue,pd->ckNew.dwColorSpaceHighValue));
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

