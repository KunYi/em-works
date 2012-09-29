//------------------------------------------------------------------------------
//
//  Copyright (C) 2008-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  ddipu_overlay.cpp
//
//  Implementation of Overlay-related functions for DDIPU class, the
//  DirectDraw display driver.
//
//-----------------------------------------------------------------------------
#include "precomp.h"

//-----------------------------------------------------------------------------
// External Functions


//-----------------------------------------------------------------------------
// External Variables


//-----------------------------------------------------------------------------
// Defines

#define IsOverlayMirrorUpDown(pd) (pd->dwFlags & DDOVER_MIRRORUPDOWN)

#define DDOVER_KEY \
    (DDOVER_KEYSRC | \
     DDOVER_KEYSRCOVERRIDE |\
     DDOVER_KEYDEST | \
     DDOVER_KEYDESTOVERRIDE)

#define ERROR_NO_OVERLAY  (MAX_VISIBLE_OVERLAYS + 10)

//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables


//-----------------------------------------------------------------------------
// Local Variables


//-----------------------------------------------------------------------------
// Local Functions

BOOL IsOverlaySurfEqual(pOverlaySurf_t pSurf1, pOverlaySurf_t pSurf2);
INT  AddOverlaySurfaceToTopOfStack(pOverlaySurf_t *ppHeader, UINT32 iSurfAddr, BOOL bShown);
BOOL RemoveOverlaySurfaceFromStack(pOverlaySurf_t pTop, UINT32 iSurfAddr);
void DeleteOverlayStack(pOverlaySurf_t *ppTop);
void SwapSurfaceInStack(pOverlaySurf_t pTop, UINT32 iSurfAddrSwapOut, UINT32 iSurfAddrSwapIn);
pOverlaySurf_t GetOverlaySurfaceInfo(pOverlaySurf_t pTop, UINT32 iSurfAddr);
BOOL IsOverlaySurfaceShown(pOverlaySurf_t pTop, UINT32 iSurfAddr);
void DumpOverlayStack(pOverlaySurf_t pHeader);

BOOL IsOverlayStackEqual(pOverlaySurf_t pTop, LPDDRAWI_DDRAWSURFACE_LCL* pOverlays);
void DumpOverlayInfo(pOverlaySurf_t pSurfInfo);

//------------------------------------------------------------------------------
//
// Function: UpdateOverlay
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
//       DDHAL_DRIVER_HANDLED
//       DDHAL_DRIVER_NOTHANDLED
//
//------------------------------------------------------------------------------
DWORD DDIPU::UpdateOverlay(LPDDHAL_UPDATEOVERLAYDATA pd)
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

    overlaySurf_t overlaySurfInfo;
    pOverlaySurf_t pOverlaySurfPtr;
    UINT16 srcWidth, srcHeight, dstWidth, dstHeight;
    BOOL bZOrderChanged = FALSE;
    BOOL bChangeToShow = FALSE;

    DDGPESurf* pSrcSurf = DDGPESurf::GetDDGPESurf(pd->lpDDSrcSurface);
    DDGPESurf* pDstSurf = DDGPESurf::GetDDGPESurf(pd->lpDDDestSurface);
    if((!pSrcSurf)||(!pDstSurf))
    {
        pd->ddRVal = DD_OK;
        return DDHAL_DRIVER_HANDLED;
    }
    if(pDstSurf == m_pPrimarySurface2)
    {
        //************************************************
        // Handle requests to HIDE an overlay surface
        //************************************************
        if (pd->dwFlags & DDOVER_HIDE)
        {
            DEBUGMSG(GPE_ZONE_HW, (TEXT("DDIPU::OVERLAY HIDE REQUEST\r\n")));

            // Are any more surfaces being shown?
            if (pSrcSurf->OffsetInVideoMemory() == m_Overlay2SurfaceInfo.nBufPhysicalAddr)
            {
                // Shut down overlays now that all overlays have been hidden

                // If it was running, stop the pre-processor.
                EnterCriticalSection(&m_csOverlayShutdown);

                Display_Middle_DisableOverlay();

                LeaveCriticalSection(&m_csOverlayShutdown);

                // Reset the visible overlay.
                m_Overlay2SurfaceInfo.nBufPhysicalAddr = 0xffffffff;

            }
            else
            {
                // the overlay is not currently visible
                // nothing we need to do here
            }
                
            pd->ddRVal = DD_OK;
            return DDHAL_DRIVER_HANDLED;
        }

        if (pSrcSurf->OffsetInVideoMemory() != m_Overlay2SurfaceInfo.nBufPhysicalAddr)
        {
            if (pd->dwFlags & DDOVER_SHOW)
            {
                DEBUGMSG(GPE_ZONE_HW, (TEXT("DDIPU::OVERLAY SHOW REQUEST\r\n")));

                if (m_Overlay2SurfaceInfo.nBufPhysicalAddr != 0xffffffff)
                {
                    // Some other overlay is already visible.

                    DEBUGMSG(GPE_ZONE_ERROR, (TEXT("Error: Other overlay already ")
                                              TEXT("visible!\r\n")));

                    pd->ddRVal = DDERR_OUTOFCAPS;
                    return DDHAL_DRIVER_HANDLED;
                }
                else
                {
                    // We are going to make the overlay visible so mark it as such.
                    m_Overlay2SurfaceInfo.nBufPhysicalAddr = pSrcSurf->OffsetInVideoMemory();

                    DEBUGMSG(GPE_ZONE_HW, (TEXT("DDIPU::OVERLAY SHOW\r\n")));
                }
            }
            else // DDOVER_SHOW not requested
            {
                // The overlay isn't visible, and we haven't been asked to make
                // it visible, so there's nothing that we need to do except just
                // return with a successful status.

                pd->ddRVal = DD_OK;
                return DDHAL_DRIVER_HANDLED;
            }
        }

        // Check here for the valid overlay surface formats that we can handle in
        // hardware. Return immediately if the source overlay surface is not one
        // of the hardware-supported pixel formats.
        switch (pSrcSurf->PixelFormat())
        {
            case ddgpePixelFormat_YV12: // All hardware-supported YUV pixel formats.
            case ddgpePixelFormat_UYVY:
            case ddgpePixelFormat_NV12:
                break;

            case ddgpePixelFormat_565:  // All hardware-supported RGB pixel formats.
            case ddgpePixelFormat_8880:
            case ddgpePixelFormat_8888:
                break;

            default:
                DEBUGMSG(GPE_ZONE_ERROR, (TEXT("DDIPU::UpdateOverlay: unexpected ")
                                          TEXT("pixel format %d\r\n"),
                                          pSrcSurf->PixelFormat()));

                pd->ddRVal = DDERR_UNSUPPORTED;
                return DDHAL_DRIVER_HANDLED;
        }

        //align the source and destination rectangles according to hardware limitation.
        if(!Display_OverlayParamAlign(&pd->rSrc, &pd->rDest, 
                        m_iRotate, pSrcSurf->PixelFormat(),pSrcSurf->Width(), pd->dwFlags & (DDOVER_ALPHASRC | DDOVER_ALPHADEST)))
        {
            DEBUGMSG(GPE_ZONE_ERROR, (TEXT("%s: wrong parameters!\r\n"),
                                      __WFUNCTION__));

            //Display_DisableOverlay2();

            // We are aborting, so set the overlay to NULL
            //m_pVisibleOverlay = NULL;

            pd->ddRVal = DDERR_INVALIDPARAMS;
            return DDHAL_DRIVER_HANDLED;
        }   

        if(m_iNumVisibleOverlays > 1)
        {
            ERRORMSG(1, (TEXT("Can't create overlay surface on 2nd panel when current overlay surface number is larger than 2.\r\n")));
            pd->ddRVal = DDERR_INVALIDPARAMS;
            return DDHAL_DRIVER_HANDLED;           
        }
        overlaySurfInfo.next = NULL;
        overlaySurfInfo.bIsShown = TRUE;

        srcWidth  = (UINT16) (pd->rSrc.right - pd->rSrc.left);
        srcHeight = (UINT16) (pd->rSrc.bottom - pd->rSrc.top);
        dstWidth  = (UINT16) (pd->rDest.right - pd->rDest.left);
        dstHeight = (UINT16) (pd->rDest.bottom - pd->rDest.top);

        overlaySurfInfo.DstWidth_Orig  = dstWidth;
        overlaySurfInfo.DstHeight_Orig = dstHeight;

        overlaySurfInfo.DstWidth  = overlaySurfInfo.DstWidth_Orig;
        overlaySurfInfo.DstHeight = overlaySurfInfo.DstHeight_Orig;


        overlaySurfInfo.SrcRect.left   = pd->rSrc.left;
        overlaySurfInfo.SrcRect.top    = pd->rSrc.top;
        overlaySurfInfo.SrcRect.right  = pd->rSrc.left + srcWidth;
        overlaySurfInfo.SrcRect.bottom = pd->rSrc.top + srcHeight;
        
        // Parameter validity check
        if((pd->rDest.right  > m_pMode->width)        ||
           (pd->rDest.bottom > m_pMode->height)       ||
           (overlaySurfInfo.DstWidth < DISP_MIN_WIDTH) ||
           (overlaySurfInfo.DstHeight ==0) ||
           (srcWidth == 0) ||
           (srcHeight == 0) ||
           (m_iRotate != DMDO_0)
            )
        {
            DEBUGMSG(GPE_ZONE_ERROR, (TEXT("%s: wrong parameters!\r\n"),
                                      __WFUNCTION__));
        
            Display_Middle_DisableOverlay();
        
            // We are aborting, so set the overlay to NULL
            m_Overlay2SurfaceInfo.nBufPhysicalAddr = 0xffffffff;
        
            pd->ddRVal = DDERR_INVALIDPARAMS;
            return DDHAL_DRIVER_HANDLED;
        }

        overlaySurfInfo.SrcWidth  = (UINT16)pSrcSurf->Width();
        overlaySurfInfo.SrcHeight = (UINT16)pSrcSurf->Height();
        overlaySurfInfo.SrcLineStride= pSrcSurf->Stride();
        overlaySurfInfo.SrcPixelFormat = pSrcSurf->PixelFormat();
        overlaySurfInfo.SrcBpp = (UINT16) pSrcSurf->Bpp();
        //Only YUV data support deinterlacing.
        if(overlaySurfInfo.SrcPixelFormat > ddgpePixelFormat_8888)
            overlaySurfInfo.isInterlaced = m_bVideoIsInterlaced;
        else
            overlaySurfInfo.isInterlaced = FALSE;
        overlaySurfInfo.TopField = m_TopField;
        
        overlaySurfInfo.DstBpp = (UINT16) m_nScreenBpp;
        overlaySurfInfo.DstLineStride = (UINT16)(overlaySurfInfo.DstWidth * m_nScreenBpp / 8);
        
        SetupOverlayPosition(&overlaySurfInfo, pd->rDest.left, pd->rDest.top);
        
        overlaySurfInfo.iRotate = m_iRotate;
        
        overlaySurfInfo.nBufPhysicalAddr = pSrcSurf->OffsetInVideoMemory();
        // In TV mode, screen should remain unrotated.
        
        // Setup if overlay is flipped as upside-down
        if(IsOverlayMirrorUpDown(pd))
        {
            DEBUGMSG(GPE_ZONE_HW, (TEXT("DDIPU UpdateOverlay: upside-down overlay ")
                                   TEXT("is requested!\r\n")));
        
            overlaySurfInfo.isUpsideDown = TRUE;
        }
        else
        {
            overlaySurfInfo.isUpsideDown = FALSE;
        }
        
        // Setup color key.
        // IPU only supports one color key value and does not
        // support a range.  So, we take the low value
        // from the color key mask to use as the color key.
        
        // Initialize color key mask to 0xFFFFFFFF.  If no color keying
        // is specified, this 0xFFFFFFFF will indicate no color key.
        overlaySurfInfo.ColorKeyMask = 0xFFFFFFFF;
        
        // Default Graphics Window as Foreground.
        // GW should only be Background in case of Destination Color Keying.
        overlaySurfInfo.ColorKeyPlane = DisplayPlane_1;
        
        if(pd->dwFlags & DDOVER_KEYSRC)
        {
            overlaySurfInfo.ColorKeyMask  =
                pd->lpDDDestSurface->ddckCKSrcOverlay.dwColorSpaceLowValue;
            overlaySurfInfo.ColorKeyPlane = DisplayPlane_1;
        }
        else if(pd->dwFlags & DDOVER_KEYSRCOVERRIDE)
        {
            overlaySurfInfo.ColorKeyMask  =
                pd->overlayFX.dckSrcColorkey.dwColorSpaceLowValue;
            overlaySurfInfo.ColorKeyPlane = DisplayPlane_1;
        }
        
        if(pd->dwFlags & DDOVER_KEYDEST)
        {
            overlaySurfInfo.ColorKeyMask  =
                pd->lpDDDestSurface->ddckCKDestOverlay.dwColorSpaceLowValue;
            overlaySurfInfo.ColorKeyPlane = DisplayPlane_0;
        }
        else if(pd->dwFlags & DDOVER_KEYDESTOVERRIDE)
        {
            overlaySurfInfo.ColorKeyMask  =
                pd->overlayFX.dckDestColorkey.dwColorSpaceLowValue;
            overlaySurfInfo.ColorKeyPlane = DisplayPlane_0;
        }
        
        // Setup alpha blending
        overlaySurfInfo.Transparency = 0xFF;
        overlaySurfInfo.bGlobalAlpha = TRUE;
        
        if(pd->dwFlags & DDOVER_ALPHACONSTOVERRIDE)
        {
            // For SDC, invert transparency if we are using the overlay as the
            // graphics plane
            if (overlaySurfInfo.ColorKeyPlane == DisplayPlane_1)
            {
                overlaySurfInfo.Transparency = 0xFFFFFFFF -
                                               pd->overlayFX.dwAlphaConst;
            }
            else
            {
                overlaySurfInfo.Transparency = pd->overlayFX.dwAlphaConst;
            }
        }
        else if (pd->dwFlags & (DDOVER_ALPHASRC | DDOVER_ALPHADEST))
        {
            overlaySurfInfo.bGlobalAlpha = FALSE;
        }
        
        // If overlay surface data has not changed, skip the step to set up
        // overlay and return.
        EnterCriticalSection(&m_csOverlayData);
        memcpy(&m_Overlay2SurfaceInfo, &overlaySurfInfo, sizeof(overlaySurf_t));

        // Update the graphic window of the IPU display
        Display_Middle_StopOverlay();
        Display_Middle_SetupOverlay(&m_Overlay2SurfaceInfo);
        Display_Middle_StartOverlay(m_Overlay2SurfaceInfo.nBufPhysicalAddr,TRUE);

        LeaveCriticalSection(&m_csOverlayData);
        pd->ddRVal = DD_OK;

        
    }
    else
    {

        //************************************************
        // First thing we do is reorder our internal list
        // of overlays (if needed) and get a pointer to
        // the correct overlaySurfInfo structure
        //************************************************

        // Update overlay ordering stack for all overlays (not just those being shown).
        // This allows us to detect if the Z-ordering has changed.
        if (!IsOverlayStackEqual(m_pOverlaySurfaceInfo, pd->lpDD->pOverlays))
        {
            // Create a temporary new list of overlay info.
            pOverlaySurf_t pNewOverlayInfo = NULL;

            // Create a counter for the number of visible overlays
            UINT32 numVisible = 0;

            // We can't delete old list until we have created a new one because
            // we would lose info about which overlays are currently being shown.

            // Now iterate through pOverlays to create a new stack
            DDGPESurf* pTempOverlay = DDGPESurf::GetDDGPESurf(pd->lpDD->pOverlays[0]);
            int i = 0;
            BOOL bShown;
            // NOTE: There is currently a bug in the DDRAW code, such that there may be NULL
            // elements in the pOverlays array that are non-terminating.  Therefore, we can't
            // stop creating our list when we hit a NULL.  Instead, we just go part-way
            // through the array (32 elems, but we only look through 10, since junk was being
            // written into array elements 28+ at one point), ignoring NULL elements.
    //        while (pTempOverlay != NULL)
            while (i < 10)
            {
                // Only add element to our list if it is non-NULL
                if (pTempOverlay != NULL)
                {
                    pOverlaySurf_t pTempInfo = GetOverlaySurfaceInfo(m_pOverlaySurfaceInfo, pTempOverlay->OffsetInVideoMemory());
                    if (pTempInfo == NULL)
                    {
                        // This is a new overlay...it should not be set as shown.
                        // If DDOVER_SHOW is set, we will update it later in this function
                        bShown = FALSE;
                    }
                    else
                    {
                        // Copy surface info from old stack into 
                        bShown = pTempInfo->bIsShown;
                    }
                    AddOverlaySurfaceToTopOfStack(&pNewOverlayInfo, pTempOverlay->OffsetInVideoMemory(), bShown);
                    if (bShown)
                    {
                        numVisible++; 
                    }

                    // Retrieve surface info pointer that we just created, and copy other surf data
                    // from old pointer into new pointer
                    if (pTempInfo != NULL)
                    {
                        pOverlaySurf_t pNewInfo = GetOverlaySurfaceInfo(pNewOverlayInfo, pTempOverlay->OffsetInVideoMemory());
                        memcpy(pNewInfo, pTempInfo, sizeof(overlaySurf_t));
                        // Clear next ptr, since it is stale data
                        pNewInfo->next = NULL;
                    }
                }
                i++;
                pTempOverlay = DDGPESurf::GetDDGPESurf(pd->lpDD->pOverlays[i]);
            }

            // Now that we have created a new list, we can delete our old one
            DeleteOverlayStack(&m_pOverlaySurfaceInfo);

            // Now point to the newly created list
            m_pOverlaySurfaceInfo = pNewOverlayInfo;

            m_iNumVisibleOverlays = numVisible;
            
            if(numVisible)
                bZOrderChanged = TRUE;
        }

        // Grab pointer to the appropriate overlay surface info structure
        pOverlaySurfPtr = GetOverlaySurfaceInfo(m_pOverlaySurfaceInfo, pSrcSurf->OffsetInVideoMemory());

        // Ensure surface 
        if (!pOverlaySurfPtr)
        {
            DEBUGMSG(1, (TEXT("Current surface not in stack of overlays!\r\n")));
            pd->ddRVal = DDERR_INVALIDPARAMS;
            return DDHAL_DRIVER_HANDLED;
        }

        //************************************************
        // Handle requests to HIDE an overlay surface
        //************************************************
        if (pd->dwFlags & DDOVER_HIDE)
        {
            DEBUGMSG(GPE_ZONE_HW, (TEXT("DDIPU::OVERLAY HIDE REQUEST\r\n")));

            // Only take action if surface is currently shown
            if (pOverlaySurfPtr->bIsShown)
            {
                // Hide current surface
                pOverlaySurfPtr->bIsShown = FALSE;

                m_iNumVisibleOverlays--;

                // Are any more surfaces being shown?
                if (!IsOverlaySurfaceShown(m_pOverlaySurfaceInfo, 0))
                {
                    // Shut down overlays now that all overlays have been hidden

                    // XEC DLS : Set the Video Off Event
                    Display_SetVideoOffEvent();

                    // If it was running, stop the pre-processor.
                    EnterCriticalSection(&m_csOverlayShutdown);

                    DisableOverlay();

                    LeaveCriticalSection(&m_csOverlayShutdown);

                    // Wipe out any remaining surfaces in stack
                    DeleteOverlayStack(&m_pOverlaySurfaceInfo);
                }
                else
                {
                    EnterCriticalSection(&m_csOverlayData);
                    // We are removing an overlay, but some overlays still remain.
                    // Therefore, we must resend the overlay array to the DIL
                    // and allow it to reconfigure.  EnableOverlay will
                    // take care of this
                    EnableOverlay();
                    LeaveCriticalSection(&m_csOverlayData);
                }
            }
            else
            {
                // the overlay is not currently visible
                // nothing we need to do here
            }
            
            DEBUGMSG(GPE_ZONE_HW, (TEXT("DDIPU::OVERLAY HIDE!\r\n")));

            DumpOverlayStack(m_pOverlaySurfaceInfo);

            pd->ddRVal = DD_OK;
            return DDHAL_DRIVER_HANDLED;
        }

        if((pDstSurf->Width() > 1024) && (m_iNumVisibleOverlays > 1))
        {
            ERRORMSG(1, (TEXT("For oversized frame, only 1 overlay surface will be supported.\r\n")));
            pd->ddRVal = DDERR_INVALIDPARAMS;
            return DDHAL_DRIVER_HANDLED;        
        }
        if(m_pPrimarySurface2 && (m_iNumVisibleOverlays > 1))
        {
            ERRORMSG(1, (TEXT("Only 1 overlay surface will be supported when secondary panel is on.\r\n")));
            pd->ddRVal = DDERR_INVALIDPARAMS;
            return DDHAL_DRIVER_HANDLED;        
        }

        //************************************************
        // Handle requests to SHOW an overlay surface
        //************************************************
        if (!IsOverlaySurfaceShown(m_pOverlaySurfaceInfo, pSrcSurf->OffsetInVideoMemory()))
        {
            // Overlay is not currently being shown

            if (pd->dwFlags & DDOVER_SHOW)
            {
                DEBUGMSG(GPE_ZONE_HW, (TEXT("DDIPU::OVERLAY SHOW REQUEST\r\n")));

                // Increase the number of visible overlays by 1
                m_iNumVisibleOverlays++;

                if (m_iNumVisibleOverlays > MAX_VISIBLE_OVERLAYS)
                {
                    DEBUGMSG(GPE_ZONE_ERROR, (TEXT("Error: Exceeding max visible overlays!\r\n")));

                    pd->ddRVal = DDERR_OUTOFCAPS;
                    return DDHAL_DRIVER_HANDLED;
                }

                pOverlaySurfPtr->bIsShown = TRUE;

                bChangeToShow = TRUE;

                DEBUGMSG(GPE_ZONE_HW, (TEXT("DDIPU::OVERLAY SHOW\r\n")));
                DumpOverlayStack(m_pOverlaySurfaceInfo);
            }
            else if (bZOrderChanged)
            {
                EnterCriticalSection(&m_csOverlayData);
                // We have changed the Z-order, but not shown or hidden
                // anything.  Since we rebuilt the m_pOverlaySurfaceInfo stack,
                // we must also rebuild the m_ppActiveOverlays array.  EnableOverlay will
                // take care of this
                EnableOverlay();
                LeaveCriticalSection(&m_csOverlayData);
                DumpOverlayStack(m_pOverlaySurfaceInfo);

                pd->ddRVal = DD_OK;
                return DDHAL_DRIVER_HANDLED;
            }
            else // DDOVER_SHOW not requested
            {
                // The overlay isn't visible, and we haven't been asked to make
                // it visible, so there's nothing that we need to do except just
                // return with a successful status.

                pd->ddRVal = DD_OK;
                return DDHAL_DRIVER_HANDLED;
            }
        }
        else if (bZOrderChanged)
        {
            EnterCriticalSection(&m_csOverlayData);
            // We have changed the Z-order, but surface is already shown.  
            // Since we rebuilt the m_pOverlaySurfaceInfo stack,
            // we must also rebuild the m_ppActiveOverlays array.  EnableOverlay will
            // take care of this
            EnableOverlay();
            LeaveCriticalSection(&m_csOverlayData);
            DumpOverlayStack(m_pOverlaySurfaceInfo);

            pd->ddRVal = DD_OK;
            return DDHAL_DRIVER_HANDLED;
        }

        // Check here for the valid overlay surface formats that we can handle in
        // hardware. Return immediately if the source overlay surface is not one
        // of the hardware-supported pixel formats.
        switch (pSrcSurf->PixelFormat())
        {
            case ddgpePixelFormat_YV12: // All hardware-supported YUV pixel formats.
            case ddgpePixelFormat_UYVY:
            case ddgpePixelFormat_NV12:
                break;

            case ddgpePixelFormat_565:  // All hardware-supported RGB pixel formats.
            case ddgpePixelFormat_8880:
            case ddgpePixelFormat_8888:
                break;

            default:
                DEBUGMSG(GPE_ZONE_ERROR, (TEXT("DDIPU::UpdateOverlay: unexpected ")
                                          TEXT("pixel format %d\r\n"),
                                          pSrcSurf->PixelFormat()));

                pd->ddRVal = DDERR_UNSUPPORTED;
                return DDHAL_DRIVER_HANDLED;
        }

        //align the source and destination rectangles according to hardware limitation.
        if(!Display_OverlayParamAlign(&pd->rSrc, &pd->rDest, 
                        m_iRotate, pSrcSurf->PixelFormat(),pSrcSurf->Width(), pd->dwFlags & (DDOVER_ALPHASRC | DDOVER_ALPHADEST)))
        {
            DEBUGMSG(GPE_ZONE_ERROR, (TEXT("%s: wrong parameters!\r\n"),
                                      __WFUNCTION__));

            DumpOverlayInfo(pOverlaySurfPtr);
            DisableOverlay();

            // We are aborting, so hide the overlay
            pOverlaySurfPtr->bIsShown = FALSE;

            m_iNumVisibleOverlays--;

            pd->ddRVal = DDERR_INVALIDPARAMS;
            return DDHAL_DRIVER_HANDLED;
        }

        overlaySurfInfo.next = pOverlaySurfPtr->next;
        overlaySurfInfo.bIsShown = pOverlaySurfPtr->bIsShown;

        srcWidth  = (UINT16) (pd->rSrc.right - pd->rSrc.left);
        srcHeight = (UINT16) (pd->rSrc.bottom - pd->rSrc.top);
        dstWidth  = (UINT16) (pd->rDest.right - pd->rDest.left);
        dstHeight = (UINT16) (pd->rDest.bottom - pd->rDest.top);

        overlaySurfInfo.DstWidth_Orig  = dstWidth;
        overlaySurfInfo.DstHeight_Orig = dstHeight;

        switch(m_iRotate)
        {
            case DMDO_90:
            case DMDO_270:
                overlaySurfInfo.DstWidth  = overlaySurfInfo.DstHeight_Orig;
                overlaySurfInfo.DstHeight = overlaySurfInfo.DstWidth_Orig;
                break;

            case DMDO_0:
            case DMDO_180:
            default:
                overlaySurfInfo.DstWidth  = overlaySurfInfo.DstWidth_Orig;
                overlaySurfInfo.DstHeight = overlaySurfInfo.DstHeight_Orig;
                break;
        }

        overlaySurfInfo.SrcRect.left   = pd->rSrc.left;
        overlaySurfInfo.SrcRect.top    = pd->rSrc.top;
        overlaySurfInfo.SrcRect.right  = pd->rSrc.left + srcWidth;
        overlaySurfInfo.SrcRect.bottom = pd->rSrc.top + srcHeight;

        // Parameter validity check
        if((pd->rDest.right  > m_pMode->width)        ||
           (pd->rDest.bottom > m_pMode->height)       ||
           (overlaySurfInfo.DstWidth < DISP_MIN_WIDTH) ||
           (overlaySurfInfo.DstHeight == 0) ||
           (srcWidth == 0) ||
           (srcHeight == 0) ||
           (m_iRotate && (overlaySurfInfo.DstHeight < DISP_MIN_WIDTH))
            )
        {
            DEBUGMSG(GPE_ZONE_ERROR, (TEXT("%s: wrong parameters!\r\n"),
                                      __WFUNCTION__));

            DumpOverlayInfo(pOverlaySurfPtr);
            DisableOverlay();

            // We are aborting, so hide the overlay
            pOverlaySurfPtr->bIsShown = FALSE;

            m_iNumVisibleOverlays--;

            pd->ddRVal = DDERR_INVALIDPARAMS;
            return DDHAL_DRIVER_HANDLED;
        }

        overlaySurfInfo.SrcWidth  = (UINT16)pSrcSurf->Width();
        overlaySurfInfo.SrcHeight = (UINT16)pSrcSurf->Height();
        overlaySurfInfo.SrcLineStride= pSrcSurf->Stride();
        overlaySurfInfo.SrcPixelFormat = pSrcSurf->PixelFormat();
        overlaySurfInfo.SrcBpp = (UINT16) pSrcSurf->Bpp();
        //Only YUV data support deinterlacing.
        if(overlaySurfInfo.SrcPixelFormat > ddgpePixelFormat_8888)
            overlaySurfInfo.isInterlaced = m_bVideoIsInterlaced;
        else
            overlaySurfInfo.isInterlaced = FALSE;

        overlaySurfInfo.DstBpp = (UINT16) m_nScreenBpp;
        overlaySurfInfo.DstLineStride = (UINT16)(overlaySurfInfo.DstWidth * m_nScreenBpp / 8);

        SetupOverlayPosition(&overlaySurfInfo, pd->rDest.left, pd->rDest.top);

        overlaySurfInfo.iRotate = m_iRotate;

        overlaySurfInfo.nBufPhysicalAddr = pSrcSurf->OffsetInVideoMemory();
        // In TV mode, screen should remain unrotated.

        // Setup if overlay is flipped as upside-down
        if(IsOverlayMirrorUpDown(pd))
        {
            DEBUGMSG(GPE_ZONE_HW, (TEXT("DDIPU UpdateOverlay: upside-down overlay ")
                                   TEXT("is requested!\r\n")));

            overlaySurfInfo.isUpsideDown = TRUE;
        }
        else
        {
            overlaySurfInfo.isUpsideDown = FALSE;
        }

        // Setup color key.
        // IPU only supports one color key value and does not
        // support a range.  So, we take the low value
        // from the color key mask to use as the color key.

        // Initialize color key mask to 0xFFFFFFFF.  If no color keying
        // is specified, this 0xFFFFFFFF will indicate no color key.
        overlaySurfInfo.ColorKeyMask = 0xFFFFFFFF;

        // Default Graphics Window as Foreground.
        // GW should only be Background in case of Destination Color Keying.
        overlaySurfInfo.ColorKeyPlane = DisplayPlane_1;

        if(pd->dwFlags & DDOVER_KEYSRC)
        {
            overlaySurfInfo.ColorKeyMask  =
                pd->lpDDDestSurface->ddckCKSrcOverlay.dwColorSpaceLowValue;
            overlaySurfInfo.ColorKeyPlane = DisplayPlane_1;
        }
        else if(pd->dwFlags & DDOVER_KEYSRCOVERRIDE)
        {
            overlaySurfInfo.ColorKeyMask  =
                pd->overlayFX.dckSrcColorkey.dwColorSpaceLowValue;
            overlaySurfInfo.ColorKeyPlane = DisplayPlane_1;
        }

        if(pd->dwFlags & DDOVER_KEYDEST)
        {
            overlaySurfInfo.ColorKeyMask  =
                pd->lpDDDestSurface->ddckCKDestOverlay.dwColorSpaceLowValue;
            overlaySurfInfo.ColorKeyPlane = DisplayPlane_0;
        }
        else if(pd->dwFlags & DDOVER_KEYDESTOVERRIDE)
        {
            overlaySurfInfo.ColorKeyMask  =
                pd->overlayFX.dckDestColorkey.dwColorSpaceLowValue;
            overlaySurfInfo.ColorKeyPlane = DisplayPlane_0;
        }

        // Setup alpha blending
        overlaySurfInfo.Transparency = 0xFF;
        overlaySurfInfo.bGlobalAlpha = TRUE;

        if(pd->dwFlags & DDOVER_ALPHACONSTOVERRIDE)
        {
            // For SDC, invert transparency if we are using the overlay as the
            // graphics plane
            if (overlaySurfInfo.ColorKeyPlane == DisplayPlane_1)
            {
                overlaySurfInfo.Transparency = 0xFFFFFFFF -
                                               pd->overlayFX.dwAlphaConst;
            }
            else
            {
                overlaySurfInfo.Transparency = pd->overlayFX.dwAlphaConst;
            }
        }
        else if (pd->dwFlags & (DDOVER_ALPHASRC | DDOVER_ALPHADEST))
        {
            overlaySurfInfo.bGlobalAlpha = FALSE;
        }

        // If overlay surface data has not changed and a surface is not being
        // newly shown, skip the step to set up overlay and return.
        if( (!IsOverlaySurfEqual(pOverlaySurfPtr, &overlaySurfInfo))
            || (m_bIsOverlayWindowRunning ==FALSE) || bChangeToShow)
        {
            EnterCriticalSection(&m_csOverlayData);

            memcpy(pOverlaySurfPtr, &overlaySurfInfo, sizeof(overlaySurf_t));

            // Update the graphic window of the IPU display
            pd->ddRVal = EnableOverlay();

            LeaveCriticalSection(&m_csOverlayData);
        }
        else if ((overlaySurfInfo.XOffset != pOverlaySurfPtr->XOffset)
                ||(overlaySurfInfo.YOffset != pOverlaySurfPtr->YOffset))
        {
            UINT32 overlayIndex;
            // Get index from overlay array
            overlayIndex = GetOverlayIndex(pOverlaySurfPtr);
            if (overlayIndex == ERROR_NO_OVERLAY)
            {
                DEBUGMSG(1, (TEXT("%x: Couldn't match this overlay - Addr = 0x%x!  Dang.\r\n"), __WFUNCTION__, pOverlaySurfPtr->nBufPhysicalAddr));
                DumpOverlayStack(m_pOverlaySurfaceInfo);
                pd->ddRVal = DDERR_INVALIDPARAMS;
                return DDHAL_DRIVER_HANDLED;
            }

            pOverlaySurfPtr->XOffset = overlaySurfInfo.XOffset;
            pOverlaySurfPtr->YOffset = overlaySurfInfo.YOffset;
            Display_SetupOverlayWindowPos(overlayIndex, pOverlaySurfPtr->XOffset, pOverlaySurfPtr->YOffset);
            EnterCriticalSection(&m_csOverlayData);
            //The content maybe changed, so we need to update the surface also.
            SetVisibleSurfaceOverlay(pSrcSurf, NULL, DDFLIP_WAITNOTBUSY);
            LeaveCriticalSection(&m_csOverlayData);
            pd->ddRVal = DD_OK;
        }
        else
        {
            EnterCriticalSection(&m_csOverlayData);
            //The content maybe changed, so we need to update the surface also.
            SetVisibleSurfaceOverlay(pSrcSurf, NULL, DDFLIP_WAITNOTBUSY);
            LeaveCriticalSection(&m_csOverlayData);
            pd->ddRVal = DD_OK;
        }
    }
    return DDHAL_DRIVER_HANDLED;
}


//------------------------------------------------------------------------------
//
// Function: SetOverlayPosition
//
// This callback function changes the display coordinates of
// an overlay surface.
//
// Parameters:
//      pd
//          [in, out] Pointer to a DDHAL_SETOVERLAYPOSITIONDATA structure
//          that contains the information required to change the display
//          coordinates of an overlay surface.
//
// Returns:
//      Returns one of the following values:
//       DDHAL_DRIVER_HANDLED
//       DDHAL_DRIVER_NOTHANDLED
//
//------------------------------------------------------------------------------
DWORD DDIPU::SetOverlayPosition(LPDDHAL_SETOVERLAYPOSITIONDATA pd)
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

    pOverlaySurf_t pOverlayInfo;
    UINT32 overlayIndex;
    
    DDGPESurf* pSrcSurf = DDGPESurf::GetDDGPESurf(pd->lpDDSrcSurface);
    DDGPESurf* pDstSurf = DDGPESurf::GetDDGPESurf(pd->lpDDDestSurface);
    if((!pSrcSurf)||(!pDstSurf))
    {
        pd->ddRVal = DD_OK;
        return DDHAL_DRIVER_HANDLED;
    }
    if(pDstSurf == m_pPrimarySurface2)
    {
        if (pSrcSurf->OffsetInVideoMemory() != m_Overlay2SurfaceInfo.nBufPhysicalAddr)
        {
            pd->ddRVal = DD_OK;
            return DDHAL_DRIVER_HANDLED;
        }
        
        // Parameter validity check
        if(
            (pd->lXPos + m_Overlay2SurfaceInfo.DstWidth_Orig > m_pPrimarySurface2->Width())   ||
            (pd->lYPos + m_Overlay2SurfaceInfo.DstHeight_Orig > m_pPrimarySurface2->Height())
            )
        {
            DEBUGMSG(GPE_ZONE_ERROR, (TEXT("DDIPU SetOverlayPosition: error pd->lXPos [%d], pd->lYPos[%d]\r\n"), pd->lXPos, pd->lYPos));
            pd->ddRVal = DDERR_INVALIDPARAMS;
            return DDHAL_DRIVER_HANDLED;
        }
       
        SetupOverlayPosition(&m_Overlay2SurfaceInfo, pd->lXPos, pd->lYPos);
       
        // Go through full overlay window setup, in order to clip foreground
        // if necessary (i.e., if FG exceeds BG in the X-direction).
        Display_Middle_SetupOverlayWindowPos(0, 
            m_Overlay2SurfaceInfo.XOffset, m_Overlay2SurfaceInfo.YOffset, TRUE);
    }
    else
    {
        if(!m_bIsOverlayWindowRunning)
        {
            // The overlay window is not running
            pd->ddRVal = DD_OK;
            return DDHAL_DRIVER_HANDLED;
        }

        pOverlayInfo = GetOverlaySurfaceInfo(m_pOverlaySurfaceInfo, pSrcSurf->OffsetInVideoMemory());

        if (pOverlayInfo == NULL)
        {
            DEBUGMSG(1, (TEXT("Trying to set position for overlay that is not in the stack!\r\n")));
            pd->ddRVal = DDERR_INVALIDPARAMS;
            return DDHAL_DRIVER_HANDLED;
        }

        // Check to see if surface is currently being shown
        if (!pOverlayInfo->bIsShown)
        {
            pd->ddRVal = DD_OK;
            return DDHAL_DRIVER_HANDLED;
        }

        // Parameter validity check
        if(
            (pd->lXPos + pOverlayInfo->DstWidth_Orig > m_pMode->width)   ||
            (pd->lYPos + pOverlayInfo->DstHeight_Orig > m_pMode->height)
            )
        {
            DEBUGMSG(GPE_ZONE_ERROR, (TEXT("DDIPU SetOverlayPosition: error pd->lXPos [%d], pd->lYPos[%d]\r\n"), pd->lXPos, pd->lYPos));
            pd->ddRVal = DDERR_INVALIDPARAMS;
            return DDHAL_DRIVER_HANDLED;
        }

        // Get index from overlay array
        overlayIndex = GetOverlayIndex(pOverlayInfo);
        if (overlayIndex == ERROR_NO_OVERLAY)
        {
            DEBUGMSG(1, (TEXT("%x: Couldn't match this overlay - Addr = 0x%x!  Dang.\r\n"), __WFUNCTION__, pOverlayInfo->nBufPhysicalAddr));
            DumpOverlayStack(m_pOverlaySurfaceInfo);
            pd->ddRVal = DDERR_INVALIDPARAMS;
            return DDHAL_DRIVER_HANDLED;
        }

        SetupOverlayPosition(pOverlayInfo, pd->lXPos, pd->lYPos);

        WaitForNotBusyOverlay();

        // Go through full overlay window setup, in order to clip foreground
        // if necessary (i.e., if FG exceeds BG in the X-direction).
        Display_SetupOverlayWindowPos(overlayIndex, pOverlayInfo->XOffset, pOverlayInfo->YOffset);
    }
    pd->ddRVal = DD_OK;
    return DDHAL_DRIVER_HANDLED;
}


//------------------------------------------------------------------------------
//
// Function: SetVisibleSurfaceOverlay
//
// This function changes the overlay surface displayed by the IPU.
//
// Parameters:
//      pSurf
//          [in] Surface to be set as the visible overlay.
//
//      pSurfCurr
//          [in] Current surface as managed by Flip().  NULL if 
//          SetVisibleSurfaceOverlay not called from Flip().
//
//      bWait
//          [in] If want to return immediately, set TRUE, otherwise, set FALSE.
//
// Returns:
//      DDERR_OUTOFCAPS if pSurfCurr not currently shown
//      DDERR_WASSTILLDRAWING if overlay rendering is busy
//      DD_OK if successful
//
//------------------------------------------------------------------------------
HRESULT DDIPU::SetVisibleSurfaceOverlay(DDGPESurf * pSurf, DDGPESurf * pSurfCurr, DWORD dwFlags)
{
    pOverlaySurf_t pOverlayInfo;
    UINT32 overlayIndex;

    BOOL bWait = (dwFlags & DDFLIP_WAITNOTBUSY) ? TRUE : FALSE;
    if(pSurfCurr&&(pSurfCurr->OffsetInVideoMemory() == m_Overlay2SurfaceInfo.nBufPhysicalAddr))
    {
        m_Overlay2SurfaceInfo.nBufPhysicalAddr = pSurf->OffsetInVideoMemory();
        Display_Middle_SetOverlayBuffer(pSurf->OffsetInVideoMemory(), 0, bWait);
        return DD_OK;
    }
    else
    {
        // Bail out if the overlay window is not running
        if (!m_bIsOverlayWindowRunning)
        {
            return DD_OK;
        }

        // Only check current surface if non-NULL (called from Flip())
        if (pSurfCurr != NULL)
        {
            if (!IsOverlaySurfaceShown(m_pOverlaySurfaceInfo, pSurfCurr->OffsetInVideoMemory()))
            {
                // Current surface not being shown
                return DDERR_OUTOFCAPS;
            }

            // Since we are flipping, we need to update our stacks with the new surfaces
            SwapSurfaceInStack(m_pOverlaySurfaceInfo, pSurfCurr->OffsetInVideoMemory(), pSurf->OffsetInVideoMemory());
        }
     
        pOverlayInfo = GetOverlaySurfaceInfo(m_pOverlaySurfaceInfo, pSurf->OffsetInVideoMemory());

        // Get index from overlay array
        overlayIndex = GetOverlayIndex(pOverlayInfo);
        if (overlayIndex == ERROR_NO_OVERLAY)
        {
            DEBUGMSG(1, (TEXT("%x: Couldn't match this overlay - Addr = 0x%x!  Dang.\r\n"), __WFUNCTION__, pOverlayInfo->nBufPhysicalAddr));
            DumpOverlayStack(m_pOverlaySurfaceInfo);
            return DDERR_OUTOFCAPS;
        }

        // XEC DLS : Send the Video Frame to XEC    
        Display_SendVideoFrame(pSurf, pOverlayInfo->SrcRect);

        //For 2 overlay case, when DDFLIP_ODD/DDFLIP_EVEN is called, deinteralce can open.
        if(m_iNumVisibleOverlays == 2)
        {
            if(dwFlags & DDFLIP_ODD)
            {
                if(!m_ppActiveOverlays[overlayIndex]->isInterlaced)
                {
                    Display_Reconfigure_Interlaced(overlayIndex, TRUE);
                }
                m_ppActiveOverlays[overlayIndex]->TopField = TopFieldSelect_Odd;
            }
            else if(dwFlags & DDFLIP_EVEN)
            {
                if(!m_ppActiveOverlays[overlayIndex]->isInterlaced)
                {
                    Display_Reconfigure_Interlaced(overlayIndex, TRUE);
                }
                m_ppActiveOverlays[overlayIndex]->TopField = TopFieldSelect_Even;
            }
        }
        
        if (Display_SetOverlayBuffer(overlayIndex, bWait))
        {
            // return success
            return DD_OK;
        }
        else
        {
            // overlay processing busy
            return DDERR_WASSTILLDRAWING;
        }
    }
}


//------------------------------------------------------------------------------
//
// Function: EnableOverlay
//
// This function enables the graphics window of the IPU.
//
// Parameters:
//      pOverlaySurf
//          [in] Overlay surface.
//
//      pOverlaySurfInfo
//          [in] Overlay surface info.
//
// Returns:
//      DD_OK                successful
//      others               failed
//
//------------------------------------------------------------------------------
HRESULT DDIPU::EnableOverlay()
{
    HRESULT result = DDERR_OUTOFCAPS;
    pOverlaySurf_t pCurOverlay;
    int i, numVisible;

    if (m_bIsOverlayWindowRunning)
    {
        // We need stop IPU processing flows first.
        Display_StopOverlay();
    }

    // Only enable if we are not already running
    if (!m_bIsOverlayWindowRunning)
    {
        m_bIsOverlayWindowRunning = TRUE;
    }

    // We want to create our array of visible overlays in the reverse order that they are stored
    // in m_pOverlaySurfaceInfo, since WinCE support told us that lower index = closer to viewer.
    // DIL interprets lower index as closer to background, so we invert the array ordering here.
    // First, we get a count of the visible overlays, then we build the array from the highest
    // index going down.

    // Just get a count of the number of visible overlays first
    i = 0;
    for (pCurOverlay = m_pOverlaySurfaceInfo; pCurOverlay != NULL; pCurOverlay = pCurOverlay->next)
    {
        // Add to our array if it is currently shown
        if (pCurOverlay->bIsShown)
        {
            i++;
        }
    }

    numVisible = i;

    // Create array of overlays to send to display interface layer
    for (pCurOverlay = m_pOverlaySurfaceInfo; pCurOverlay != NULL; pCurOverlay = pCurOverlay->next)
    {
        // Add to our array if it is currently shown
        if (pCurOverlay->bIsShown)
        {
            // Count down in array indices while going up through overlay stack
            i--;
            m_ppActiveOverlays[i] = pCurOverlay;
        }
    }

    //If the local alpha surface is not the top surface in multiple overlay case, move it to the top-most.
    if((numVisible == 2)&&(m_ppActiveOverlays[0]->SrcBpp == 32)&&(m_ppActiveOverlays[1]->SrcBpp != 32))
    {
        pOverlaySurf_t pTmpOverlay;
        pTmpOverlay = m_ppActiveOverlays[1];
        m_ppActiveOverlays[1] = m_ppActiveOverlays[0];
        m_ppActiveOverlays[0] = pTmpOverlay;
        RETAILMSG(1,(TEXT("Z-order of local alpha overlay is changed automatically.\r\n")));
        RETAILMSG(1,(TEXT("You can avoid this message by adjusting overlay creation sequence.\r\n")));
    }

    if ((INT)m_iNumVisibleOverlays != numVisible)
    {
        DEBUGMSG(1, (TEXT("DDIPU number of visible overlays not matching our running count!\r\n")));
        m_iNumVisibleOverlays = numVisible;
    }

    Display_SetupOverlay(m_ppActiveOverlays, m_iNumVisibleOverlays);

    //**************************
    // Execute PrP, PP and DP tasks
    //**************************

    Display_StartOverlay();

    result = DD_OK;

    DEBUGMSG(GPE_ZONE_INIT, (TEXT("DDIPU EnableOverlay: result 0x%08x\r\n"), result));

   return result;
}

//------------------------------------------------------------------------------
//
// Function: DisableOverlay
//
// This function disables the graphics window of the IPU.
//
// Parameters:
//      None.
//
// Returns:
//      DD_OK                successful
//      DDERR_EXCEPTION      failed (for WinCE/PMC)
//      DDERR_GENERIC        failed (for Windows Mobile/PocketPC)
//
//------------------------------------------------------------------------------
HRESULT DDIPU::DisableOverlay(VOID)
{
    if (m_bIsOverlayWindowRunning)
    {

        m_bIsOverlayWindowRunning = FALSE;
        if (!Display_DisableOverlay())
        {
            return DDERR_GENERIC;
        }
    }
    else if (m_Dx == D4)
    {
        m_bOverlayDisableNotHandled = TRUE;
    }

    return DD_OK;
}

//------------------------------------------------------------------------------
//
// Function: WaitForNotBusyOverlay
//
// This function waits until the graphics window of the IPU is ready to
// update.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
VOID DDIPU::WaitForNotBusyOverlay(VOID)
{
    // TODO:
    //DisplayPlaneWaitForNotBusy();
    return;
}

//------------------------------------------------------------------------------
//
// Function: SetupOverlayPosition
//
// This function waits until the graphics window of the IPU is ready to
// update.
//
// Parameters:
//      pOverlaySurf
//          [in/out] Surface for which the X and Y offsets are to
//          be configured.
//
//      X
//          [in] Desired X offset of overlay surface in display.
//
//      Y
//          [in] Desired Y offset of overlay surface in display.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void DDIPU::SetupOverlayPosition(pOverlaySurf_t pOverlaySurf, LONG X, LONG Y)
{
    switch(m_iRotate){
        case DMDO_0:
            pOverlaySurf->XOffset = (UINT16)X;
            pOverlaySurf->YOffset = (UINT16)Y;
            break;
        case DMDO_90:
            pOverlaySurf->XOffset = (UINT16)Y;
            pOverlaySurf->YOffset = (UINT16)(m_pMode->width - (X + pOverlaySurf->DstWidth_Orig));
            break;
        case DMDO_180:
            pOverlaySurf->XOffset = (UINT16)(m_pMode->width - (X + pOverlaySurf->DstWidth_Orig));
            pOverlaySurf->YOffset = (UINT16)(m_pMode->height - (Y + pOverlaySurf->DstHeight_Orig));
            break;
        case DMDO_270:
            pOverlaySurf->XOffset = (UINT16)(m_pMode->height - (Y + pOverlaySurf->DstHeight_Orig));
            pOverlaySurf->YOffset = (UINT16)X;

            break;
    }
}


//------------------------------------------------------------------------------
//
// Function: GetOverlayIndex
//
// This function gets the overlay index from the overlay array.
//
// Parameters:
//      pOverlaySurf
//          [in] Overlay to search for.
//
// Returns:
//      The overlay index number
//      ERROR_NO_OVERLAY if the overlay is not found
//
//------------------------------------------------------------------------------
UINT32 DDIPU::GetOverlayIndex(pOverlaySurf_t pOverlaySurf)
{
    UINT32 overlayIndex = ERROR_NO_OVERLAY;

    for (UINT32 i = 0; i < m_iNumVisibleOverlays; i++)
    {
        if (m_ppActiveOverlays[i] == pOverlaySurf)
        {
            overlayIndex = i;
            break;
        }
    }

    return overlayIndex;
}

//------------------------------------------------------------------------------
//
// Function: AddOverlaySurfaceToTopOfStack
//
// This function removes a single element from an overlay stack,
// maintaining the stack order for the remaining elements.
//
// Parameters:
//      pHeader
//          [in] Header node of overlays.
//
//      iSurfAddr
//          [in] Overlay surface to remove.
//
//      bShown
//          [in] Is new element shown or hidden?
//
// Returns:
//      Number of elements in stack after addition.  Returns zero if failure.
//
//------------------------------------------------------------------------------
INT AddOverlaySurfaceToTopOfStack(pOverlaySurf_t *ppHeader, UINT32 iSurfAddr, BOOL bShown)
{
    INT dwNumElems = 1; // There will be at least one element after the addition process
    pOverlaySurf_t pNodeCur = *ppHeader;

    // Allocate new node
    pOverlaySurf_t pNewNode = (pOverlaySurf_t)LocalAlloc(LPTR, sizeof(overlaySurf_t));

    memset(pNewNode, 0x00, sizeof(overlaySurf_t));

    // Set new node overlay pointer and Shown status
    pNewNode->nBufPhysicalAddr = iSurfAddr;
    pNewNode->bIsShown = bShown;

    // This is our last node
    pNewNode->next = NULL;

    // If stack was empty, point to the new node
    if (*ppHeader == NULL)
    {
        *ppHeader = pNewNode;
    }
    else
    {
        // Otherwise, jump to the end of the stack
        while (pNodeCur->next != NULL)
        {
            pNodeCur = pNodeCur->next;
            dwNumElems++;
        }

        // Set last node to newly created node
        pNodeCur->next = pNewNode;

        // Just added new element
        dwNumElems++;
    }

    return dwNumElems;
}

//------------------------------------------------------------------------------
//
// Function: RemoveOverlaySurfaceFromStack
//
// This function removes a single element from an overlay stack,
// maintaining the stack order for the remaining elements.
//
// Parameters:
//      ppTop
//          [in] Array of overlay surface info structures.
//
//      iSurfAddr
//          [in] Overlay surface to remove.
//
// Returns:
//      TRUE if overlay found and removed; FALSE if overlay not found.
//
//------------------------------------------------------------------------------
BOOL RemoveOverlaySurfaceFromStack(pOverlaySurf_t* ppTop, UINT32 iSurfAddr)
{
    // Check list of overlays for the one that is being hidden
    pOverlaySurf_t pLastOverlay, pCurOverlay;
    BOOL bOverlayNotFound = TRUE;

    pLastOverlay = *ppTop;

    for (pCurOverlay = *ppTop; pCurOverlay != NULL; pCurOverlay = pCurOverlay->next)
    {
        // Is current overlay equal to overlay being hidden?
        if (pCurOverlay->nBufPhysicalAddr == iSurfAddr)
        {
            // If Top overlay removed
            if (pCurOverlay == *ppTop)
            {
                if (pCurOverlay->next != NULL)
                {
                    // New Top overlay
                    *ppTop = pCurOverlay->next;
                }
                else
                {
                    // No more overlays left being shown
                    *ppTop = NULL;
                }

                LocalFree(pCurOverlay);
                bOverlayNotFound = FALSE;
            }
            else
            {
                // Remove from list
                pLastOverlay->next = pCurOverlay->next;
                pCurOverlay->next = NULL;
                LocalFree(pCurOverlay);
                bOverlayNotFound = FALSE;
            }


            // Bail out of loop since we found surface 
            // in list and removed it
            break;
        }
        else
        {
            // Track the current node so we can link nodes together
            // when we find the node we want to remove
            pLastOverlay = pCurOverlay;
        }
    }

    return bOverlayNotFound;
}


//------------------------------------------------------------------------------
//
// Function: DeleteOverlayStack
//
// This function deletes all nodes in the stack.
//
// Parameters:
//      ppTop
//          [in] Header node of overlays.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void DeleteOverlayStack(pOverlaySurf_t *ppTop)
{
    pOverlaySurf_t pCurOverlay, pNextOverlay;

    for (pCurOverlay = *ppTop; pCurOverlay != NULL; pCurOverlay = pNextOverlay)
    {
        // Save next pointer since we will be deleting this node
        pNextOverlay = pCurOverlay->next;

        // Remove from list
        pCurOverlay->nBufPhysicalAddr = NULL;
        pCurOverlay->next = NULL;
        LocalFree(pCurOverlay);
    }

    *ppTop = NULL;
}

//------------------------------------------------------------------------------
//
// Function: SwapSurfaceInStack
//
// This function searches the stack of overlays for the specified address,
// and swaps it with a 2nd address.  The purpose of this is to update the
// surface address whenever a surface flip occurs.
//
// Parameters:
//      pTop
//          [in] Header node of overlays.
//
//      iSurfAddrSwapOut
//          [in] Overlay surface to search for and swap out.
//
//      iSurfAddrSwapIn
//          [in] Overlay surface to swap in.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void SwapSurfaceInStack(pOverlaySurf_t pTop, UINT32 iSurfAddrSwapOut, UINT32 iSurfAddrSwapIn)
{
    pOverlaySurf_t pCurOverlay;

    // Check list of overlays for pSurf
    for (pCurOverlay = pTop; pCurOverlay != NULL; pCurOverlay = pCurOverlay->next)
    {
        // Is current overlay equal to overlay being searched for?
        if (pCurOverlay->nBufPhysicalAddr == iSurfAddrSwapOut)
        {
            // If so, perform swap
            pCurOverlay->nBufPhysicalAddr = iSurfAddrSwapIn;
        }
    }
}

//------------------------------------------------------------------------------
//
// Function: GetOverlaySurfaceInfo
//
// This function searches the stack of overlay surfaces for the
// specified surface.
//
// Parameters:
//      pTop
//          [in] Header node of overlays.
//
//      iSurfAddr
//          [in] Overlay surface to search for.
//
// Returns:
//      pointer to overlay surface structure if it is found in the list
//      NULL if it is not found.
//
//------------------------------------------------------------------------------
pOverlaySurf_t GetOverlaySurfaceInfo(pOverlaySurf_t pTop, UINT32 iSurfAddr)
{
    pOverlaySurf_t pCurOverlay;

    // Check list of overlays for pSurf
    for (pCurOverlay = pTop; pCurOverlay != NULL; pCurOverlay = pCurOverlay->next)
    {
        // Is current overlay equal to overlay being searched for?
        if (pCurOverlay->nBufPhysicalAddr == iSurfAddr)
        {
            break;
        }
    }

    return pCurOverlay;
}

//------------------------------------------------------------------------------
//
// Function: IsOverlaySurfaceShown
//
// This function searches the stack of visible overlay surfaces for the
// specified surface and returns whether it is shown or not.
//
// Parameters:
//      pTop
//          [in] Header node of overlays.
//
//      iSurfAddr
//          [in] Overlay surface to search for.  If set to 0, function
//          will search to see if any surfaces are shown.
//
// Returns:
//      FALSE if surface not shown or if not found
//      TRUE if it is currently shown.
//
//------------------------------------------------------------------------------
BOOL IsOverlaySurfaceShown(pOverlaySurf_t pTop, UINT32 iSurfAddr)
{
    BOOL bShown = FALSE;
    pOverlaySurf_t pCurOverlay;
    BOOL bSearchAll = (iSurfAddr == 0) ? TRUE : FALSE;

    // Check list of overlays for pSurf
    for (pCurOverlay = pTop; pCurOverlay != NULL; pCurOverlay = pCurOverlay->next)
    {
        if (bSearchAll)
        {
            // Is current overlay shown?
            if (pCurOverlay->bIsShown)
            {
                return TRUE;
            }
        }
        else
        {
            // Is current overlay equal to overlay being searched for?
            if (pCurOverlay->nBufPhysicalAddr == iSurfAddr)
            {
                bShown = pCurOverlay->bIsShown;
                break;
            }
        }
    }

    return bShown;
}


//------------------------------------------------------------------------------
//
// Function: IsOverlayStackEqual
//
// This function compares the driver-managed stack of overlays against
// the array provided in UpdateOverlay call.
//
// Parameters:
//      pTop
//          [in] Header node of overlays.
//
//      pOverlays
//          [in] Overlay surface array provided through UpdateOverlay.
//
// Returns:
//      TRUE if stacks are equivalent.
//      FALSE if not.
//
//------------------------------------------------------------------------------
BOOL IsOverlayStackEqual(pOverlaySurf_t pTop, LPDDRAWI_DDRAWSURFACE_LCL* pOverlays)
{
    pOverlaySurf_t pCurOverlay;
    DDGPESurf *pNewOverlay;
    int i;

    pNewOverlay = DDGPESurf::GetDDGPESurf(pOverlays[0]);

    //The first overlay pointer in overlay chain provided by MSFT maybe not on the top. 
    //Null pointer maybe on the top.So we check first 10 overlays to confirm if the whole 
    //overlay chain is NULL.
    //Once found non-NULL overlay pointer, set it as pNewOverlay for following comparion.
    // TODO: Remove the following code once MSFT fix the bug about overlay chain.
    if(pTop == NULL)
    {
        for(i=0;i<10;i++)
            if(DDGPESurf::GetDDGPESurf(pOverlays[i])!=NULL) return FALSE;
    }
    else
    {
        for(i=0;i<10;i++)
        {
            if(pNewOverlay == NULL)
            {
                pNewOverlay = DDGPESurf::GetDDGPESurf(pOverlays[i]);
            }
            if(pNewOverlay != NULL)  break;
        }
        if(pNewOverlay == NULL)
            return FALSE;
    }

    // If one list is NULL and the other is non-NULL, return FALSE
    if (((pTop == NULL) && (pNewOverlay != NULL)) || 
        ((pTop != NULL) && (pNewOverlay == NULL)))
    {
        return FALSE;
    }

    // Check list of overlays for pSurf
    for (pCurOverlay = pTop; (pCurOverlay != NULL) && (pNewOverlay != NULL) ; pCurOverlay = pCurOverlay->next, i++)
    {
        // Is current overlay equal to overlay being serached for?
        if (pCurOverlay->nBufPhysicalAddr != pNewOverlay->OffsetInVideoMemory())
        {
            return FALSE;
        }

        // Get next overlay from input array
        pNewOverlay = DDGPESurf::GetDDGPESurf(pOverlays[i+1]);

        // Check to see if stacks have different number of elements
        if (((pCurOverlay->next == NULL) && (pNewOverlay != NULL)) ||
            ((pCurOverlay->next != NULL) && (pNewOverlay == NULL)))
        {
            return FALSE;
        }
    }

    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function: IsOverlaySurfEqual
//
// This function compares overlay surfaces to determine if they are equal.
// update.
//
// Parameters:
//      pSurf1
//          [in] Overlay surface descriptor 1.
//
//      pSurf2
//          [in] Overlay surface descriptor 2.
//
// Returns:
//      TRUE if pSurf1 and pSurf2 are equal.
//      FALSE if not.
//
//------------------------------------------------------------------------------
BOOL IsOverlaySurfEqual(pOverlaySurf_t pSurf1, pOverlaySurf_t pSurf2)
{
    if ((pSurf1->SrcWidth == pSurf2->SrcWidth) &&
        (pSurf1->SrcHeight == pSurf2->SrcHeight) &&
        (pSurf1->SrcLineStride == pSurf2->SrcLineStride) &&
        (pSurf1->SrcRect.top == pSurf2->SrcRect.top) &&
        (pSurf1->SrcRect.left == pSurf2->SrcRect.left) &&
        (pSurf1->SrcRect.bottom == pSurf2->SrcRect.bottom) &&
        (pSurf1->SrcRect.right == pSurf2->SrcRect.right) &&
        (pSurf1->SrcPixelFormat == pSurf2->SrcPixelFormat) &&
        (pSurf1->SrcBpp == pSurf2->SrcBpp) &&
        (pSurf1->DstWidth_Orig == pSurf2->DstWidth_Orig) &&
        (pSurf1->DstHeight_Orig == pSurf2->DstHeight_Orig) &&
        (pSurf1->DstWidth == pSurf2->DstWidth) &&
        (pSurf1->DstHeight == pSurf2->DstHeight) &&
        (pSurf1->DstLineStride == pSurf2->DstLineStride) &&
        (pSurf1->DstBpp == pSurf2->DstBpp) &&
        (pSurf1->Transparency == pSurf2->Transparency) &&
        (pSurf1->isUpsideDown == pSurf2->isUpsideDown) &&
        (pSurf1->ColorKeyMask == pSurf2->ColorKeyMask) &&
        (pSurf1->ColorKeyPlane == pSurf2->ColorKeyPlane) &&
        (pSurf1->bGlobalAlpha== pSurf2->bGlobalAlpha))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


void DumpOverlayInfo(pOverlaySurf_t pSurfInfo)
{
#ifndef DEBUG
    UNREFERENCED_PARAMETER(pSurfInfo);
#endif
    DEBUGMSG(GPE_ZONE_TEMP, (TEXT("Overlay OP: Overlay->nBufPhysicalAddr 0x%08x\r\n"), pSurfInfo->nBufPhysicalAddr));                                    \
    DEBUGMSG(GPE_ZONE_TEMP, (TEXT("Overlay OP: Overlay->SrcWidth            %d\r\n"), pSurfInfo->SrcWidth));                                                \
    DEBUGMSG(GPE_ZONE_TEMP, (TEXT("Overlay OP: Overlay->SrcHeight           %d\r\n"), pSurfInfo->SrcHeight));                                                \
    DEBUGMSG(GPE_ZONE_TEMP, (TEXT("Overlay OP: Overlay->DstWidth          %d\r\n"), pSurfInfo->DstWidth));                                                \
    DEBUGMSG(GPE_ZONE_TEMP, (TEXT("Overlay OP: Overlay->DstHeight         %d\r\n"), pSurfInfo->DstHeight));                                                \
    DEBUGMSG(GPE_ZONE_TEMP, (TEXT("Overlay OP: Overlay->DstLineStride       %d\r\n"), pSurfInfo->DstLineStride));                                            \
    DEBUGMSG(GPE_ZONE_TEMP, (TEXT("Overlay OP: Overlay->XOffset          %d\r\n"), pSurfInfo->XOffset));                                                \
    DEBUGMSG(GPE_ZONE_TEMP, (TEXT("Overlay OP: Overlay->YOffset          %d\r\n"), pSurfInfo->YOffset));                                                \
    DEBUGMSG(GPE_ZONE_TEMP, (TEXT("Overlay OP: Overlay->Transparency     %d\r\n"), pSurfInfo->Transparency));                                            \
    DEBUGMSG(GPE_ZONE_TEMP, (TEXT("Overlay OP: Overlay->isUpsideDown     %s\r\n"), pSurfInfo->isUpsideDown ? L"TRUE" : L"FALSE"));                    \
    DEBUGMSG(GPE_ZONE_TEMP, (TEXT("Overlay OP: Overlay->ColorKeyMask     0x%08x\r\n"), pSurfInfo->ColorKeyMask));                                        \
}

void DumpOverlayStack(pOverlaySurf_t pHeader)
{
    pOverlaySurf_t pNodeCur = pHeader;
    int i = 0;

    // Iterate through stack elements, printing surface pointer
    while (pNodeCur != NULL)
    {
        DEBUGMSG(1, (TEXT("DDIPU DumpOverlayStack: Element %d = 0x%x, Shown? = 0x%x\r\n"), i, pNodeCur->nBufPhysicalAddr, pNodeCur->bIsShown));

        pNodeCur = pNodeCur->next;
        i++;
    }

    return;
}
