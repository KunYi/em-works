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
//  Copyright (C) 2005-2006, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//---------------------------------------------------------------------------

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

    DWORD   dwSurfaceDescFlags = pd->lpDDSurfaceDesc->dwFlags;
    DWORD   dwSurfaceCaps = pd->lpDDSurfaceDesc->ddsCaps.dwCaps;
    DWORD   dwPixelFormat;
    DWORD   dwBpp;

    if (dwSurfaceDescFlags & DDSD_PIXELFORMAT)
    {
        dwPixelFormat = pd->lpDDSurfaceDesc->ddpfPixelFormat.dwFlags;
        dwBpp = pd->lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount;
    }
    else
    {
        // get the surface values of the primary
        dwPixelFormat = DDPF_RGB;
        dwBpp  = (GetDDGPE())->DDGPEPrimarySurface()->Bpp();
    }

    // check dwHeight and dwWidth for avoiding restriction of DirectDraw
    if (((dwSurfaceDescFlags & DDSD_HEIGHT) && (pd->lpDDSurfaceDesc->dwHeight >= 0x00010000)) ||
        ((dwSurfaceDescFlags & DDSD_WIDTH ) && (pd->lpDDSurfaceDesc->dwWidth  >= 0x00010000))) 
    {
        DEBUGMSG(HAL_ZONE_ERROR, (TEXT("HalCanCreateSurface: unsupported height/width, check if they are valid!\r\n")));
        goto CannotCreate;
    }

    // primary surface
    if (dwSurfaceCaps & DDSCAPS_PRIMARYSURFACE) 
    {
        if (dwPixelFormat & DDPF_RGB)
        {
                if ( (dwSurfaceDescFlags & DDSD_PIXELFORMAT) && (dwBpp == 16) &&
                     (pd->lpDDSurfaceDesc->ddpfPixelFormat.dwRBitMask == 0xF800 &&
                     pd->lpDDSurfaceDesc->ddpfPixelFormat.dwGBitMask == 0x07E0 &&
                     pd->lpDDSurfaceDesc->ddpfPixelFormat.dwBBitMask == 0x001F) )
                {
                        goto CanCreate;
                }
                else
                {
                    DEBUGMSG(HAL_ZONE_ERROR,(TEXT("HalCanCreateSurface: %dbpp not supported for primary surface.\r\n"), dwBpp));
                    goto CannotCreate;
                }
        }
        else 
        {
            DEBUGMSG(HAL_ZONE_ERROR,(TEXT("HalCanCreateSurface: %dbpp not supported for primary surface.\r\n"), dwBpp));
            goto CannotCreate;
        }
    }
    // overlay surface support.
    else if (dwSurfaceCaps & DDSCAPS_OVERLAY) 
    {
        if (dwPixelFormat & DDPF_RGB)
        {
            goto CanCreate;
        }
        else 
        {
            DEBUGMSG(HAL_ZONE_ERROR,(TEXT("HalCanCreateSurface: %dbpp Overlay surface creation not supported .\r\n"), dwBpp));
            goto CannotCreate;
        }
    }
    // offscreen surface
    else 
    {
        if (dwSurfaceCaps & DDSCAPS_VIDEOMEMORY)
        {
            
            // We only have one pixel format due to the LCDC of i.MX27 processor
            if(pd->bIsDifferentPixelFormat)
            {
                DEBUGMSG(HAL_ZONE_ERROR, (TEXT("HalCanCreateSurface: the requested surface want different format!\r\n")));
                goto CannotCreate;
            }
            else
            {
                goto CanCreate;    
            }
        }
        else
        {
            // for system memory surfaces, see if our emulation can deal with it...
            return DDGPECanCreateSurface(pd);
        }
    }

CanCreate:
    DEBUGMSG(HAL_ZONE_CREATE, (TEXT("HalCanCreateSurface: Unsupported\r\n")));
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

    return ((MX27DDLcdc *)GetDDGPE())->Flip(pd);
}

//------------------------------------------------------------------------------
//
// Function: HalGetFlipStatus
//
// This callback function checks the status of the Flip operation.
//
// Parameters:
//      pfd 
//          [in, out] Pointer to a LPDDHAL_GETFLIPSTATUSDATA structure that 
//          contains information necessary.
//
// Returns:
//      Returns one of the following values: 
//          DDHAL_DRIVER_HANDLED
//
//------------------------------------------------------------------------------
DWORD WINAPI HalGetFlipStatus(LPDDHAL_GETFLIPSTATUSDATA pd)
{
    DEBUGENTER( HalGetFlipStatus );

    return ((MX27DDLcdc *)GetDDGPE())->GetFlipStatus(pd);
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
        if (((MX27DDLcdc *)GetDDGPE())->IsBusy())
        {
            pd->ddRVal = DDERR_WASSTILLDRAWING;
        }
    }
    else if (pd->dwFlags & DDGBS_ISBLTDONE)
    {
        if (((MX27DDLcdc *)GetDDGPE())->IsBusy())
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
    return ((MX27DDLcdc *)GetDDGPE())->UpdateOverlay(pd);
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
    return ((MX27DDLcdc *)GetDDGPE())->SetOverlayPosition(pd);
}
