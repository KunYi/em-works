//------------------------------------------------------------------------------
//
//  Copyright (C) 2008-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  halsurf.cpp
//
//  HAL-specific DirectDrawSurface callback functions. These callback functions
//  must be implemented in order to override the default DDGPE functions so
//  that we can make use of our display hardware acceleration capabilities.
//
//------------------------------------------------------------------------------

#include "precomp.h"


//------------------------------------------------------------------------------
// External Functions
EXTERN_C SCODE DDGPEGetPixelFormatFromSurfaceDesc(
                    LPDDSURFACEDESC            lpDDSurfaceDesc,
                    EDDGPEPixelFormat*         pPixelFormat,
                    EGPEFormat*                pFormat
                    );

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


////////////////////////////////////////////////////////////////////////////////
///////////////////////////// DDHAL_DDCALLBACKS ////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
/////////////////////////// DDHAL_DDSURFACECALLBACKS ///////////////////////////
////////////////////////////////////////////////////////////////////////////////


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
    /*
    typedef struct _DDHAL_FLIPDATA
    {
        LPDDRAWI_DIRECTDRAW_GBL     lpDD;        // driver struct
        LPDDRAWI_DDRAWSURFACE_LCL   lpSurfCurr;  // current surface
        LPDDRAWI_DDRAWSURFACE_LCL   lpSurfTarg;  // target surface (to flip to)
        DWORD                       dwFlags;     // flags
        HRESULT                     ddRVal;      // return value
        LPDDHALSURFCB_FLIP          Flip;        // PRIVATE: ptr to callback
    } DDHAL_FLIPDATA;
    */

    return ((DDIPU *)GetDDGPE())->Flip(pd);
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

    pd->ddRVal = DD_OK;

    if (pd->dwFlags & DDGBS_CANBLT)
    {
        if (((DDIPU *)GetDDGPE())->IsBusy())
        {
            pd->ddRVal = DDERR_WASSTILLDRAWING;
        }
    }
    else if (pd->dwFlags & DDGBS_ISBLTDONE)
    {
        if (((DDIPU *)GetDDGPE())->IsBusy())
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
DWORD WINAPI HalUpdateOverlay( LPDDHAL_UPDATEOVERLAYDATA pd )
{
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

    return ((DDIPU *)GetDDGPE())->UpdateOverlay(pd);
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
DWORD WINAPI HalSetOverlayPosition( LPDDHAL_SETOVERLAYPOSITIONDATA pd )
{
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

    return ((DDIPU *)GetDDGPE())->SetOverlayPosition(pd);
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
//------------------------------------------------------------------------------
//
// Function: HalDestroySurface
//
// This callback function destroy a DirectDrawSurface object
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

DWORD WINAPI HalDestroySurface(LPDDHAL_DESTROYSURFACEDATA pd)
{
    DEBUGENTER(HalDestroySurface);

    DEBUGMSG(GPE_ZONE_CREATE,(TEXT("Destroy GPESurf *:0x%08x\r\n"), 
                              DDGPESurf::GetDDGPESurf(pd->lpDDSurface)));

    // Make sure we're not destroying the primary.

    if (((DDGPESurf*)(GetDDGPE()->PrimarySurface()) != DDGPESurf::GetDDGPESurf(pd->lpDDSurface))
    &&((DDGPESurf*)(((DDIPU *)GetDDGPE())->PrimarySurface2()) != DDGPESurf::GetDDGPESurf(pd->lpDDSurface)))
    {
        DDGPESurf::DeleteSurface(pd->lpDDSurface);
    }

    DEBUGLEAVE(HalDestroySurface);

    pd->ddRVal = DD_OK;
    return DDHAL_DRIVER_HANDLED;
}


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

    DDIPU *                   pDDGPE = (DDIPU *)GetDDGPE();

    if((pd->lpDDSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE2) == DDSCAPS_PRIMARYSURFACE2)
    {
        //For creating second primary surface
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
            
            //All primary surface must be located at video memory.
            pSurf->ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;  
            
            dwCaps = pSurf->ddsCaps.dwCaps;

            if ((dwCaps & DDSCAPS_PRIMARYSURFACE2)==DDSCAPS_PRIMARYSURFACE2) 
            {
                // The GDI second primary surface should already exist, so we don't need to
                // do any allocation for it.
                if(pDDGPE->PrimarySurface2())
                {
                    pDDGPE->PrimarySurface2()->SetDDGPESurf(pSurf);
                    sc = S_OK;
                }
                else
                {
                    sc = DDERR_OUTOFMEMORY;
                }
            }
            else 
            {
                DWORD dwCapsOptions[2];
                int i, nRetries;
                
                // if neither video or system 
                if (!(dwCaps & (DDSCAPS_VIDEOMEMORY | DDSCAPS_SYSTEMMEMORY)))
                {
                    nRetries = 2;
                    dwCapsOptions[0] = dwCaps | DDSCAPS_VIDEOMEMORY;       // first option is video memory
                    dwCapsOptions[1] = dwCaps | DDSCAPS_SYSTEMMEMORY;
                }
                else
                {
                    nRetries = 1;
                    dwCapsOptions[0] = dwCaps;
                }
                
                for (i = 0; i < nRetries; i++)
                {
                    sc = pDDGPE->AllocBackBuffer(pSurf);
                    
                    if (SUCCEEDED(sc))
                    {
                        break;      // we allocated the first option for the surface, break from the Retries loop
                    }
                }
            }

            if( FAILED( sc ) )
            {
                pd->ddRVal = sc;
                DEBUGMSG(GPE_ZONE_ERROR,(TEXT("Failed to create surface, sc=0x%08x\r\n"),sc));
                return DDHAL_DRIVER_HANDLED;
            }

            if (DDGPESurf::GetDDGPESurf(pSurf)->InVideoMemory()) {
                pSurf->ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
            }
            else {
                pSurf->ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
            }
        } // end of for loop

        // Reach back into the surface description to fill out the surface stride.
        // (All surfaces in a flipping chain should have the same stride.)

        pSurf = pd->lplpSList[0];
        if (pSurf)
        {
            int nStrideBytes = DDGPESurf::GetDDGPESurf(pSurf)->Stride();
            int BytesPerPixel = DDGPESurf::GetDDGPESurf(pSurf)->BytesPerPixel();
            int Rotate = DDGPESurf::GetDDGPESurf(pSurf)->Rotate();

            switch (Rotate)
            {
                case DMDO_0:
                    pd->lpDDSurfaceDesc->lPitch = nStrideBytes;
                    pd->lpDDSurfaceDesc->lXPitch = BytesPerPixel;
                    break;

                case DMDO_90:
                    pd->lpDDSurfaceDesc->lPitch = BytesPerPixel;
                    pd->lpDDSurfaceDesc->lXPitch = -nStrideBytes;
                    break;

                case DMDO_180:
                    pd->lpDDSurfaceDesc->lPitch = -nStrideBytes;
                    pd->lpDDSurfaceDesc->lXPitch = -BytesPerPixel;
                    break;

                case DMDO_270:
                    pd->lpDDSurfaceDesc->lPitch = -BytesPerPixel;
                    pd->lpDDSurfaceDesc->lXPitch = nStrideBytes;
                    break;
            }

            pd->lpDDSurfaceDesc->dwWidth= DDGPESurf::GetDDGPESurf(pSurf)->Width();
            pd->lpDDSurfaceDesc->dwHeight= DDGPESurf::GetDDGPESurf(pSurf)->Height();
            pd->lpDDSurfaceDesc->dwSurfaceSize = DDGPESurf::GetDDGPESurf(pSurf)->SurfaceSize();
            pd->lpDDSurfaceDesc->dwFlags |= (DDSD_PITCH | DDSD_XPITCH | DDSD_SURFACESIZE);
            if (DDGPESurf::GetDDGPESurf(pSurf)->InVideoMemory()) {
                pd->lpDDSurfaceDesc->ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
            }
            else {
                pd->lpDDSurfaceDesc->ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
            }
            pd->ddRVal = DD_OK;
        }
    }
    else if (pd->lpDDSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_OVERLAY)
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
        //We only support create YUV surface on video memory
        if((pixelFormat == ddgpePixelFormat_YUYV)||
            (pixelFormat == ddgpePixelFormat_UYVY)||
            (pixelFormat == ddgpePixelFormat_YUY2)||
            (pixelFormat == ddgpePixelFormat_YV12)||
            (pixelFormat == ddgpePixelFormat_CustomFormat))
        {
            for( iSurf=0; iSurf<pd->dwSCnt; iSurf++ ) 
            {
                pSurf = pd->lplpSList[iSurf];
                pSurf->ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;    
            }
        }
        DDGPECreateSurface(pd);
    }
    DEBUGMSG(GPE_ZONE_CREATE, (TEXT("HalCreateSurface: surface create %s\r\n"), (pd->ddRVal == DD_OK) ? L"successful" : L"failed"));
    DEBUGLEAVE(HalCreateSurface);
    return DDHAL_DRIVER_HANDLED;
}

//------------------------------------------------------------------------------
//
// Function: HalGetFlipStatus
//
// This function tells us the status of a flip.
//
// Parameters:
//      pd
//          [in, out] Pointer to a DDHAL_GETFLIPSTATUSDATA structure that
//          returns the flip status information.
//
// Returns:
//      Returns one of the following values:
//          DDHAL_DRIVER_HANDLED
//
//------------------------------------------------------------------------------
DWORD WINAPI HalGetFlipStatus( LPDDHAL_GETFLIPSTATUSDATA pd )
{

    /*    typedef struct _DDHAL_GETFLIPSTATUSDATA
     {
         LPDDRAWI_DIRECTDRAW_GBL     lpDD;           // driver struct
         LPDDRAWI_DDRAWSURFACE_LCL   lpDDSurface;    // surface struct
          DWORD                       dwFlags;        // flags
         HRESULT                     ddRVal;         // return value
     } DDHAL_GETFLIPSTATUSDATA;
    */

    // Implementation
    pd->ddRVal = DD_OK;

    if (pd->dwFlags & DDGFS_CANFLIP)
    {
        // default pd->ddRval is DD_OK, no more action here.
    }
    else if (pd->dwFlags & DDGFS_ISFLIPDONE)
    {
        if (((DDIPU *)GetDDGPE())->IsFlipBusy())
        {
            pd->ddRVal = DDERR_WASSTILLDRAWING;
        }
    }

    return DDHAL_DRIVER_HANDLED;
}
//------------------------------------------------------------------------------
//
// Function: HalLock
//
// This function locks certain area of surface. We must override the default
// DDGPELock() implementation in order to support software cursor.
//
// Parameters:
//      pd
//          [in, out] Pointer to a LPDDHAL_LOCKDATA structure
//          that contains the information required to lock surface.
//
// Returns:
//      Returns one of the following values:
//          DDHAL_DRIVER_HANDLED
//          DDHAL_DRIVER_NOTHANDLED
//
//------------------------------------------------------------------------------
DWORD WINAPI HalLock( LPDDHAL_LOCKDATA pd )
{
    DEBUGENTER(DDGPELock);

    DDIPU * pDDGPE = (DDIPU *)GetDDGPE();

    DDGPESurf * pSurf = DDGPESurf::GetDDGPESurf(pd->lpDDSurface);

    if (pd->dwFlags & DDLOCK_WAITNOTBUSY) {

        GetDDGPE()->WaitForNotBusy();
        if (GetDDGPE()->SurfaceBusyFlipping(pSurf)) {
        
            GetDDGPE()->WaitForVBlank();
            while (GetDDGPE()->SurfaceBusyFlipping(pSurf));
        }
    }
    else if (GetDDGPE()->SurfaceBusyFlipping(pSurf) || GetDDGPE()->IsBusy()) {

        pd->ddRVal = DDERR_WASSTILLDRAWING;
        return DDHAL_DRIVER_HANDLED;
    }

    DWORD ulAddress;

    int x = 0;
    int y = 0;
    if (pd->bHasRect) {

        x = pd->rArea.left;
        y = pd->rArea.top;
    }

    ulAddress = (ULONG) pSurf->GetPtr(x,y);

    pd->lpSurfData = reinterpret_cast<LPVOID>(ulAddress);

    if (pd->bHasRect)
        pDDGPE->AreaLock((DDIPUSurf *)pSurf, &pd->rArea);
    else
        pDDGPE->AreaLock((DDIPUSurf *)pSurf, NULL);

    DEBUGLEAVE(DDGPELock);

    pd->ddRVal = DD_OK;
    return DDHAL_DRIVER_HANDLED;
}
//------------------------------------------------------------------------------
//
// Function: HalUnlock
//
// This function unlocks certain area of surface. We must override the default
// DDGPEUnlock() implementation in order to support software cursor.
//
// Parameters:
//      pd
//          [in, out] Pointer to a LPDDHAL_UNLOCKDATA structure
//          that contains the information required to unlock surface.
//
// Returns:
//      Returns one of the following values:
//          DDHAL_DRIVER_HANDLED
//          DDHAL_DRIVER_NOTHANDLED
//
//------------------------------------------------------------------------------
DWORD WINAPI HalUnlock( LPDDHAL_UNLOCKDATA pd )
{
    DDIPU * pDDGPE = (DDIPU *)GetDDGPE();
    DDIPUSurf * pSurf = (DDIPUSurf *)DDGPESurf::GetDDGPESurf(pd->lpDDSurface);
    
    DEBUGENTER(DDGPEUnlock);

    pDDGPE->AreaUnLock(pSurf);

    DEBUGLEAVE(DDGPEUnlock);

    pd->ddRVal = DD_OK;
    return DDHAL_DRIVER_HANDLED;
}


