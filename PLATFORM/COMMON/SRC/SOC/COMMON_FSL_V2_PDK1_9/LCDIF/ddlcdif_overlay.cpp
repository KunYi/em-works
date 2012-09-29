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
// File:       ddlcdif_overlay.cpp
//
//  Implementation of Overlay-related functions for DDLcdif class, the
//  DirectDraw display driver.
//
//-----------------------------------------------------------------------------

#include "precomp.h"

//#undef DEBUGMSG
//#define DEBUGMSG(cond, msg) RETAILMSG(1,msg)


#define IsOverlayMirrorUpDown(pd) (pd->dwFlags & DDOVER_MIRRORUPDOWN)

#define IsOverlayMirrorLeftRight(pd) (pd->dwFlags & DDOVER_MIRRORLEFTRIGHT)

#define DUMP_OVERLAY_OP()   {\
    DEBUGMSG(1, (TEXT("Overlay OP: Overlay->nBufPhysicalAddr 0x%08x\r\n"), m_pOverlaySurfaceOp->nBufPhysicalAddr));                                 \
    DEBUGMSG(1, (TEXT("Overlay OP: Overlay->DstRecWidth      %d\r\n"), m_pOverlaySurfaceOp->DstRecWidth));                                          \
    DEBUGMSG(1, (TEXT("Overlay OP: Overlay->DstRecHeight     %d\r\n"), m_pOverlaySurfaceOp->DstRecHeight));                                         \
    DEBUGMSG(1, (TEXT("Overlay OP: Overlay->DstRecWidthHw    %d\r\n"), m_pOverlaySurfaceOp->DstRecWidthHw));                                        \
    DEBUGMSG(1, (TEXT("Overlay OP: Overlay->DstRecHeightHw   %d\r\n"), m_pOverlaySurfaceOp->DstRecHeightHw));                                       \
    DEBUGMSG(1, (TEXT("Overlay OP: Overlay->XOffset          %d\r\n"), m_pOverlaySurfaceOp->XOffset));                                              \
    DEBUGMSG(1, (TEXT("Overlay OP: Overlay->YOffset          %d\r\n"), m_pOverlaySurfaceOp->YOffset));                                              \
    DEBUGMSG(1, (TEXT("Overlay OP: Overlay->SrcXOffset       %d\r\n"), m_pOverlaySurfaceOp->SrcXOffset));                                           \
    DEBUGMSG(1, (TEXT("Overlay OP: Overlay->SrcYOffset       %d\r\n"), m_pOverlaySurfaceOp->SrcYOffset));                                           \
    DEBUGMSG(1, (TEXT("Overlay OP: Overlay->SrcRecWidth      %d\r\n"), m_pOverlaySurfaceOp->SrcRecWidth));                                          \
    DEBUGMSG(1, (TEXT("Overlay OP: Overlay->SrcRecHeight     %d\r\n"), m_pOverlaySurfaceOp->SrcRecHeight));                                         \
    DEBUGMSG(1, (TEXT("Overlay OP: Overlay->SrcSurWidth      %d\r\n"), m_pOverlaySurfaceOp->SrcSurWidth));                                          \
    DEBUGMSG(1, (TEXT("Overlay OP: Overlay->SrcSurHeight     %d\r\n"), m_pOverlaySurfaceOp->SrcSurHeight));                                         \
    DEBUGMSG(1, (TEXT("Overlay OP: Overlay->Transparency     %d\r\n"), m_pOverlaySurfaceOp->Transparency));                                         \
    DEBUGMSG(1, (TEXT("Overlay OP: Overlay->isUpsideDown     %s\r\n"), m_pOverlaySurfaceOp->isUpsideDown ? L"TRUE" : L"FALSE"));                    \
    DEBUGMSG(1, (TEXT("Overlay OP: Overlay->isLeftsideRight  %s\r\n"), m_pOverlaySurfaceOp->isLeftsideRight ? L"TRUE" : L"FALSE"));                 \
    DEBUGMSG(1, (TEXT("Overlay OP: Overlay->SrcColorKeyLow   0x%0x\r\n"), m_pOverlaySurfaceOp->SrcColorKeyLow));                                    \
    DEBUGMSG(1, (TEXT("Overlay OP: Overlay->SrcColorKeyHigh  0x%0x\r\n"), m_pOverlaySurfaceOp->SrcColorKeyHigh));                                   \
    DEBUGMSG(1, (TEXT("Overlay OP: Overlay->DestColorKeyLow  0x%0x\r\n"), m_pOverlaySurfaceOp->DestColorKeyLow));                                   \
    DEBUGMSG(1, (TEXT("Overlay OP: Overlay->DestColorKeyHigh 0x%0x\r\n"), m_pOverlaySurfaceOp->DestColorKeyHigh));                                  \
    DEBUGMSG(1, (TEXT("Overlay OP: Overlay->OverlayPara      0x%0x\r\n"), m_pOverlaySurfaceOp->OverlayPara));                                       \
    DEBUGMSG(1, (TEXT("Overlay OP: Overlay->SrcPixelFormat   %d\r\n"), m_pOverlaySurfaceOp->SrcPixelFormat));                                       \
    DEBUGMSG(1, (TEXT("Overlay OP: Overlay->DestPixelFormat  %d\r\n"), m_pOverlaySurfaceOp->DestPixelFormat));                                      \
}

#define SETUP_OVERLAY_POSITION(X, Y) switch(m_iRotate){ \
        case DMDO_0:                                                                                            \
            m_pOverlaySurfaceOp->XOffset = (UINT16)X;                                                           \
            m_pOverlaySurfaceOp->YOffset = (UINT16)Y;                                                           \
            break;                                                                                              \
        case DMDO_90:                                                                                           \
            m_pOverlaySurfaceOp->XOffset = (UINT16)Y;                                                           \
            m_pOverlaySurfaceOp->YOffset = (UINT16)(m_pMode->width - (X + m_pOverlaySurfaceOp->DstRecWidth));   \
            break;                                                                                              \
        case DMDO_180:                                                                                          \
            m_pOverlaySurfaceOp->XOffset = (UINT16)(m_pMode->width - (X + m_pOverlaySurfaceOp->DstRecWidth));   \
            m_pOverlaySurfaceOp->YOffset = (UINT16)(m_pMode->height - (Y + m_pOverlaySurfaceOp->DstRecHeight)); \
            break;                                                                                              \
        case DMDO_270:                                                                                          \
            m_pOverlaySurfaceOp->XOffset = (UINT16)(m_pMode->height - (Y + m_pOverlaySurfaceOp->DstRecHeight)); \
            m_pOverlaySurfaceOp->YOffset = (UINT16)X;                                                           \
            break;                                                                                              \
        default:                                                                                                \
            break;                                                                                              \
}

#define GRAPH_WIND_CTRL_SDC_KEY_COLOR_R_LSH 16
#define GRAPH_WIND_CTRL_SDC_KEY_COLOR_G_LSH 8
#define GRAPH_WIND_CTRL_SDC_KEY_COLOR_B_LSH 0

//-----------------------------------------------------------------------------
// External Variables
extern HANDLE m_hCombSurfUpdatedEvent;
extern bool CombineSurface_thread_running;
extern HANDLE CombineSurface_thread;
//-----------------------------------------------------------------------------
// External Functions

//-----------------------------------------------------------------------------
// Local Functions
BOOL IsOverlaySurfEqual(pOverlaySurf_t pSurf1, pOverlaySurf_t pSurf2);

//------------------------------------------------------------------------------
// CLASS MEMBER FUNCTIONS
//------------------------------------------------------------------------------
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
DWORD DDLcdif::UpdateOverlay(LPDDHAL_UPDATEOVERLAYDATA pd)
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

    DDGPESurf* pSrcSurf = DDGPESurf::GetDDGPESurf(pd->lpDDSrcSurface);
    DDGPESurf* pDstSurf = DDGPESurf::GetDDGPESurf(pd->lpDDDestSurface);

    DEBUGMSG(1, (TEXT("pd->dwFlags = 0x%0x\r\n"),pd->dwFlags));
    if (pd->dwFlags & DDOVER_HIDE)
    {
        DEBUGMSG(GPE_ZONE_HW, (TEXT("DDLcdif::OVERLAY HIDE REQUEST\r\n")));
        if (pSrcSurf == m_pVisibleOverlay)
        {
            // hide the overlay
            //WaitForVSync();
            DisableOverlay();
            // reset visible overlay
            m_pVisibleOverlay = NULL;
            DEBUGMSG(GPE_ZONE_HW, (TEXT("DDLcdif::OVERLAY HIDE!\r\n")));
        }
        else
        {
            // the overlay is not currently visible
            // nothing we need to do here
        }

        pd->ddRVal = DD_OK;
        goto UpdateOverlay_Done;
    }

    if (pSrcSurf != m_pVisibleOverlay)
    {
        if (pd->dwFlags & DDOVER_SHOW)
        {
            DEBUGMSG(GPE_ZONE_HW, (TEXT("DDLcdif::OVERLAY SHOW REQUEST\r\n")));
            if (m_pVisibleOverlay != NULL)
            {
                // some other overlay is already visible
                DEBUGMSG(GPE_ZONE_ERROR,
                    (TEXT("Error: Other overlay already visible!\r\n")));
                pd->ddRVal = DDERR_OUTOFCAPS;
                goto UpdateOverlay_Done;
            }
            else
            {
                // we are going to make the overlay visible
                // so mark it as such:
                m_pVisibleOverlay = pSrcSurf;
                DEBUGMSG(GPE_ZONE_HW, (TEXT("DDLcdif::OVERLAY SHOW\r\n")));
            }
        }
        else // DDOVER_SHOW not requested
        {
            // the overlay isn't visible, and we haven't been
            // asked to make it visible, so nothing we need to do
            pd->ddRVal = DD_OK;
            goto UpdateOverlay_Done;
        }
    }

    switch (pSrcSurf->PixelFormat())
    {
    case ddgpePixelFormat_YV12: // All hardware-supported YUV pixel formats.
        break;

    case ddgpePixelFormat_565:  // All hardware-supported RGB pixel formats.
    case ddgpePixelFormat_5550:
    case ddgpePixelFormat_5551:        
    case ddgpePixelFormat_8888:
        break;

    default:
        RETAILMSG(1, (TEXT("PXP::UpdateOverlay: unexpected ")
            TEXT("Src surface pixel format %d\r\n"),
            pSrcSurf->PixelFormat()));

        pd->ddRVal = DDERR_UNSUPPORTED;
        m_pVisibleOverlay = NULL;
        goto UpdateOverlay_Done;
    }

    switch (pDstSurf->PixelFormat())
    {
    case ddgpePixelFormat_565:  // All hardware-supported RGB pixel formats.
    case ddgpePixelFormat_5550:
    case ddgpePixelFormat_8888:
    case ddgpePixelFormat_5551:
        break;

    default:
        RETAILMSG(1, (TEXT("PXP::UpdateOverlay: unexpected ")
            TEXT("Dest surface pixel format %d\r\n"),
            pDstSurf->PixelFormat()));

        pd->ddRVal = DDERR_UNSUPPORTED;
        m_pVisibleOverlay = NULL;
        goto UpdateOverlay_Done;
    }

    m_pOverlaySurfaceOp->DstRecWidth = overlaySurfInfo.DstRecWidth   = (UINT16)(pd->rDest.right - pd->rDest.left);
    m_pOverlaySurfaceOp->DstRecHeight = overlaySurfInfo.DstRecHeight  = (UINT16)(pd->rDest.bottom - pd->rDest.top);

    switch(m_iRotate)
    {
    case DMDO_0:
    case DMDO_180:
        overlaySurfInfo.DstRecWidthHw = overlaySurfInfo.DstRecWidth;
        overlaySurfInfo.DstRecHeightHw = overlaySurfInfo.DstRecHeight;
        break;

    case DMDO_90:
    case DMDO_270:
        overlaySurfInfo.DstRecWidthHw = overlaySurfInfo.DstRecHeight;
        overlaySurfInfo.DstRecHeightHw = overlaySurfInfo.DstRecWidth;
        break;
    default:
        overlaySurfInfo.DstRecWidthHw = overlaySurfInfo.DstRecWidth;
        overlaySurfInfo.DstRecHeightHw = overlaySurfInfo.DstRecHeight;
        break;
    }

    // Parameter validity check
    if(
        (pd->rDest.right  > m_pMode->width)  
        ||(pd->rDest.bottom > m_pMode->height) 
//       || (overlaySurfInfo.DstRecWidthHw & 0x8)
        )
    {
        RETAILMSG(1, (TEXT("DDLcdif UpdateOverlay: wrong parameters.!\r\n")));
        DUMP_OVERLAY_OP();
        pd->ddRVal = DDERR_INVALIDPARAMS;
        m_pVisibleOverlay = NULL;
        goto UpdateOverlay_Done;
    }

    //DEBUGMSG(1,(_T("pd->rDest.right-pd->rDest.left %d,pd->rDest.bottom-pd->rDest.top %d,pDstSurf->Width %d,pDstSurf->Height %d"),pd->rDest.right-pd->rDest.left,pd->rDest.bottom-pd->rDest.top,pDstSurf->Width(),pDstSurf->Height()));

    SETUP_OVERLAY_POSITION(pd->rDest.left, pd->rDest.top);
    overlaySurfInfo.iRotate = m_iRotate;

    DEBUGMSG(1,(_T("m_iRotate=%d,m_pOverlaySurfaceOp->XOffset=%d,m_pOverlaySurfaceOp->YOffset=%d\r\n"),m_iRotate,m_pOverlaySurfaceOp->XOffset,m_pOverlaySurfaceOp->YOffset));

    overlaySurfInfo.SrcXOffset = (UINT16)pd->rSrc.left;
    overlaySurfInfo.SrcYOffset = (UINT16)pd->rSrc.top;
    overlaySurfInfo.SrcRecWidth = (UINT16)(pd->rSrc.right-pd->rSrc.left);
    overlaySurfInfo.SrcRecHeight = (UINT16)(pd->rSrc.bottom-pd->rSrc.top);
    overlaySurfInfo.SrcSurWidth  = (UINT16)pSrcSurf->Width();
    overlaySurfInfo.SrcSurHeight = (UINT16)pSrcSurf->Height();
    DEBUGMSG(1,(_T("pd->rSrc.right-pd->rSrc.left %d,pd->rSrc.bottom-pd->rSrc.top %d,pSrcSurf->Width %d,pSrcSurf->Height %d\r\n"),pd->rSrc.right-pd->rSrc.left,pd->rSrc.bottom-pd->rSrc.top,pSrcSurf->Width(),pSrcSurf->Height()));
    DEBUGMSG(1,(_T("pd->rSrc.left %d, pd->rSrc.top %d\r\n"),pd->rSrc.left,pd->rSrc.top));
    overlaySurfInfo.nBufPhysicalAddr = m_nLAWPhysical + pSrcSurf->OffsetInVideoMemory();
    overlaySurfInfo.pBufVirtualAddr = (PBYTE)VirtualMemAddr + pSrcSurf->OffsetInVideoMemory();
    DEBUGMSG(1, (TEXT("overlaySurfInfo.nBufPhysicalAddr %0x\r\n"), overlaySurfInfo.nBufPhysicalAddr));
    overlaySurfInfo.SrcPixelFormat = pSrcSurf->PixelFormat();
    DEBUGMSG(1, (TEXT("overlaySurfInfo.SrcPixelFormat %d\r\n"), overlaySurfInfo.SrcPixelFormat));

    overlaySurfInfo.DestPixelFormat = pDstSurf->PixelFormat();
    //DEBUGMSG(1, (TEXT("pDstSurf->PixelFormat()=%d,EGPEFormatToEDDGPEPixelFormat[m_pMode->format]=%d\r\n"), pDstSurf->PixelFormat(),EGPEFormatToEDDGPEPixelFormat[m_pMode->format]));

    if((((overlaySurfInfo.DstRecWidth&m_nAlignMask) != (overlaySurfInfo.SrcRecWidth&m_nAlignMask))||((overlaySurfInfo.DstRecHeight&m_nAlignMask) != (overlaySurfInfo.SrcRecHeight&m_nAlignMask))) && (overlaySurfInfo.SrcPixelFormat != ddgpePixelFormat_YV12))
    {
        RETAILMSG(1,(TEXT("Error: PXP doesn't support RGB resize!\r\n")));
        pd->ddRVal = DDERR_INVALIDPARAMS;
        m_pVisibleOverlay = NULL;
        goto UpdateOverlay_Done;
    }

    if(((overlaySurfInfo.DstRecWidth << m_nDownScaleExp) < overlaySurfInfo.SrcRecWidth)||((overlaySurfInfo.DstRecHeight << m_nDownScaleExp) < overlaySurfInfo.SrcRecHeight))
    {
        RETAILMSG(1,(TEXT("Error: The maximum down scaling factor of PXP is 1/%d!\r\n"),1<<m_nDownScaleExp));
        pd->ddRVal = DDERR_INVALIDPARAMS;
        m_pVisibleOverlay = NULL;
        goto UpdateOverlay_Done;
    }

    if(((overlaySurfInfo.DstRecWidth >> 12) > overlaySurfInfo.SrcRecWidth)||((overlaySurfInfo.DstRecHeight >> 12) > overlaySurfInfo.SrcRecHeight))
    {
        RETAILMSG(1,(TEXT("Error: The maximum up scaling factor of PXP is 4096!\r\n")));
        pd->ddRVal = DDERR_INVALIDPARAMS;
        m_pVisibleOverlay = NULL;
        goto UpdateOverlay_Done;
    }

    // Setup if overlay is flipped as upside-down
    if(IsOverlayMirrorUpDown(pd))
    {
        DEBUGMSG(GPE_ZONE_HW, (TEXT("DDLcdif UpdateOverlay: upside-down overlay is requested!\r\n")));
        overlaySurfInfo.isUpsideDown = TRUE;
    }
    else
    {
        overlaySurfInfo.isUpsideDown = FALSE;
    }

    // Setup if overlay is flipped as leftside-right
    if(IsOverlayMirrorLeftRight(pd))
    {
        DEBUGMSG(GPE_ZONE_HW, (TEXT("DDLcdif UpdateOverlay: leftside-right overlay is requested!\r\n")));
        overlaySurfInfo.isLeftsideRight = TRUE;
    }
    else
    {
        overlaySurfInfo.isLeftsideRight = FALSE;
    }

    overlaySurfInfo.OverlayPara.U = 0;

    if(pd->dwFlags & DDOVER_KEYSRC)
    {
        overlaySurfInfo.SrcColorKeyLow = pd->lpDDSrcSurface->ddckCKSrcOverlay.dwColorSpaceLowValue;
        overlaySurfInfo.SrcColorKeyHigh = pd->lpDDSrcSurface->ddckCKSrcOverlay.dwColorSpaceHighValue;
        overlaySurfInfo.OverlayPara.B.KEYSRC_EN = TRUE; 
        DEBUGMSG(1, (TEXT("DDOVER_KEYSRC: SrcColorKeyLow 0x%x SrcColorKeyHigh 0x%x,\r\n"),overlaySurfInfo.SrcColorKeyLow,overlaySurfInfo.SrcColorKeyHigh));
    }
    else if(pd->dwFlags & DDOVER_KEYSRCOVERRIDE)
    {
        overlaySurfInfo.SrcColorKeyLow = pd->overlayFX.dckSrcColorkey.dwColorSpaceLowValue;
        overlaySurfInfo.SrcColorKeyHigh = pd->overlayFX.dckSrcColorkey.dwColorSpaceHighValue;
        overlaySurfInfo.OverlayPara.B.KEYSRC_EN = TRUE;
        DEBUGMSG(1, (TEXT("DDOVER_KEYSRCOVERRIDE: SrcColorKeyLow 0x%x SrcColorKeyHigh 0x%x,\r\n"),overlaySurfInfo.SrcColorKeyLow,overlaySurfInfo.SrcColorKeyHigh));
    }

    if(pd->dwFlags & DDOVER_KEYDEST)
    {
        overlaySurfInfo.DestColorKeyLow = pd->lpDDDestSurface->ddckCKDestOverlay.dwColorSpaceLowValue;
        overlaySurfInfo.DestColorKeyHigh = pd->lpDDDestSurface->ddckCKDestOverlay.dwColorSpaceHighValue;
        overlaySurfInfo.OverlayPara.B.KEYDEST_EN = TRUE;
        DEBUGMSG(1, (TEXT("DDOVER_KEYDEST: DestColorKeyLow 0x%x DestColorKeyHigh 0x%x,\r\n"),overlaySurfInfo.DestColorKeyLow,overlaySurfInfo.DestColorKeyHigh));
    }
    else if(pd->dwFlags & DDOVER_KEYDESTOVERRIDE)
    {
        overlaySurfInfo.DestColorKeyLow = pd->overlayFX.dckDestColorkey.dwColorSpaceLowValue;
        overlaySurfInfo.DestColorKeyHigh = pd->overlayFX.dckDestColorkey.dwColorSpaceHighValue;
        overlaySurfInfo.OverlayPara.B.KEYDEST_EN = TRUE;
        DEBUGMSG(1, (TEXT("DDOVER_KEYDESTOVERRIDE: DestColorKeyLow 0x%x DestColorKeyHigh 0x%x,\r\n"),overlaySurfInfo.DestColorKeyLow,overlaySurfInfo.DestColorKeyHigh));

        /*
        // This is a hack required because of incompatibilities between DDraw
        // and rotation.  The wrong color key is provided in rotation mode if
        // this is not included.  For Windows Media Player, this should always
        // be the color key: 0x841
        if (m_iRotate)
        {
        //DEBUGMSG(1, (TEXT("UpdateOverlay: Color Key hack - set to 0x841 for CE player overlay.\r\n")));
        overlaySurfInfo.ColorKeyMask = 0x841;
        }
        */
    }

    // Setup alpha blending
    overlaySurfInfo.Transparency = 0x00;

    if(pd->dwFlags & DDOVER_ALPHASRC)
    {
        overlaySurfInfo.OverlayPara.B.ALPHASRC_EN = TRUE;
        DEBUGMSG(1, (TEXT("DDOVER_ALPHASRC\r\n")));
    }
    if(pd->dwFlags & DDOVER_ALPHASRCNEG)
    {
        RETAILMSG(1,
          (TEXT("Error: can't handle src neg alpha!\r\n")));
        pd->ddRVal = DDERR_INVALIDPARAMS;
        m_pVisibleOverlay = NULL;
        goto UpdateOverlay_Done;
    }

    if (pd->dwFlags & DDOVER_ALPHACONSTOVERRIDE)
    {
        overlaySurfInfo.Transparency = (UINT8)(pd->overlayFX.dwAlphaConst);
        overlaySurfInfo.OverlayPara.B.ALPHACONSTANT_EN = TRUE;
        DEBUGMSG(1, (TEXT("DDOVER_ALPHACONSTOVERRIDE: pd->overlayFX.dwAlphaConst %d\r\n"),pd->overlayFX.dwAlphaConst));
    }

    if(pd->dwFlags & DDOVER_ALPHADEST)
    {
        RETAILMSG(1,
            (TEXT("Error: can't handle dest alpha!\r\n")));
        pd->ddRVal = DDERR_INVALIDPARAMS;
        m_pVisibleOverlay = NULL;
        goto UpdateOverlay_Done;
    }
    if(pd->dwFlags & DDOVER_ALPHADESTNEG)
    {
        RETAILMSG(1,
            (TEXT("Error: can't handle dest alpha!\r\n")));
        pd->ddRVal = DDERR_INVALIDPARAMS;
        m_pVisibleOverlay = NULL;
        goto UpdateOverlay_Done;
    }

    if((overlaySurfInfo.OverlayPara.B.ALPHASRC_EN||overlaySurfInfo.OverlayPara.B.ALPHACONSTANT_EN)&& (overlaySurfInfo.SrcXOffset||overlaySurfInfo.SrcYOffset))
    {
        RETAILMSG(1,(TEXT("Error: Alpha blending doesn't support source surface cropping!\r\n")));
        pd->ddRVal = DDERR_INVALIDPARAMS;
        m_pVisibleOverlay = NULL;
        goto UpdateOverlay_Done;
    }

    // If overlay surface data has not changed, skip
    // step to set up overlay and return.
    if (!IsOverlaySurfEqual(m_pOverlaySurfaceOp, &overlaySurfInfo))
    {
        overlaySurfInfo.XOffset = m_pOverlaySurfaceOp->XOffset;
        overlaySurfInfo.YOffset = m_pOverlaySurfaceOp->YOffset;
        memcpy(m_pOverlaySurfaceOp, &overlaySurfInfo, sizeof(overlaySurf_t));

        pd->ddRVal = EnableOverlay();
    }
    else
    {
        SetVisibleSurfaceOverlay(pSrcSurf);
        pd->ddRVal = DD_OK;
    }

UpdateOverlay_Done:
    DEBUGMSG(GPE_ZONE_HW, (TEXT("DDLcdif UpdateOverlay: %s!\r\n"), (pd->ddRVal == DD_OK) ? L"successful" : L"failed"));

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
DWORD DDLcdif::SetOverlayPosition(LPDDHAL_SETOVERLAYPOSITIONDATA pd)
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


    DDGPESurf* pSrcSurf = DDGPESurf::GetDDGPESurf(pd->lpDDSrcSurface);

    if (pSrcSurf != m_pVisibleOverlay)
    {
        pd->ddRVal = DD_OK;
        DEBUGMSG(1,(_T("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\r\n")));
        return DDHAL_DRIVER_HANDLED;
    }

    // Parameter validity check
    if(
        (pd->lXPos + m_pOverlaySurfaceOp->DstRecWidth > m_pMode->width) ||
        (pd->lYPos + m_pOverlaySurfaceOp->DstRecHeight > m_pMode->height)                 
        )
    {
        ERRORMSG(1, (TEXT("DDLcdif SetOverlayPosition: error pd->lXPos [%d], pd->lYPos[%d]\r\n"), pd->lXPos, pd->lYPos));
        DUMP_OVERLAY_OP();
        pd->ddRVal = DDERR_INVALIDPARAMS;
        return DDHAL_DRIVER_HANDLED;
    }

    SETUP_OVERLAY_POSITION(pd->lXPos, pd->lYPos);

    if(m_pOverlaySurfaceOp->OverlayPara.B.ALPHASRC_EN||m_pOverlaySurfaceOp->OverlayPara.B.ALPHACONSTANT_EN) //Overlay surface with alpha channel
    {
        pxpOverlayBuffersPos pxpOverlayPos;
        pxpOverlayPos.iOverlayBufNum = 0;
        pxpOverlayPos.rOverlayBufRect.left = m_pOverlaySurfaceOp->XOffset;
        pxpOverlayPos.rOverlayBufRect.right = m_pOverlaySurfaceOp->XOffset + m_pOverlaySurfaceOp->DstRecWidthHw;
        pxpOverlayPos.rOverlayBufRect.top = m_pOverlaySurfaceOp->YOffset;
        pxpOverlayPos.rOverlayBufRect.bottom = m_pOverlaySurfaceOp->YOffset + m_pOverlaySurfaceOp->DstRecHeightHw;
        EnterCriticalSection(&m_PxpOperateCS);
        if(!m_hPXP)
        {
            LeaveCriticalSection(&m_PxpOperateCS);
            pd->ddRVal = DDERR_UNSUPPORTED;
            return DDHAL_DRIVER_HANDLED;
        }
        
        PXPSetOverlayBuffersPos(m_hPXP, &pxpOverlayPos);
    }
    else    //Overlay surface without alpha channel    
    {
        pxpCoordinate pxpS0Coor;
        pxpS0Coor.iXBase = m_pOverlaySurfaceOp->XOffset;
        pxpS0Coor.iYBase = m_pOverlaySurfaceOp->YOffset;
        EnterCriticalSection(&m_PxpOperateCS);
        if(!m_hPXP)
        {
            LeaveCriticalSection(&m_PxpOperateCS);
            pd->ddRVal = DDERR_UNSUPPORTED;
            return DDHAL_DRIVER_HANDLED;
        }

        PXPSetS0BufferOffsetInOutput(m_hPXP, &pxpS0Coor);
    }
       
    PXPStartProcess(m_hPXP, FALSE);
    SetEvent(m_hCombSurfUpdatedEvent);

    LeaveCriticalSection(&m_PxpOperateCS);

    pd->ddRVal = DD_OK;
    DEBUGMSG(1, (TEXT("DDLcdif SetOverlayPosition:\r\n")));

    return DDHAL_DRIVER_HANDLED;
}


//------------------------------------------------------------------------------
//
// Function: SetVisibleSurfaceOverlay
//
// This function changes the overlay surface displayed by the LCDIF.
//
// Parameters:
//      pSurf
//          [in] Surface to be set as the visible overlay.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
VOID DDLcdif::SetVisibleSurfaceOverlay(DDGPESurf * pSurf)
{

    m_pOverlaySurfaceOp->nBufPhysicalAddr = m_nLAWPhysical + pSurf->OffsetInVideoMemory();

    if(m_pOverlaySurfaceOp->OverlayPara.B.ALPHASRC_EN||m_pOverlaySurfaceOp->OverlayPara.B.ALPHACONSTANT_EN) //Overlay surface with alpha channel
    {
        if(m_pOverlaySurfaceOp->iRotate||m_pOverlaySurfaceOp->isUpsideDown||m_pOverlaySurfaceOp->isLeftsideRight)   //Overlay surface needs rotation
        {
            pxpOverlayBuffersAddr pxpOverlayAddr;
            pxpOverlayAddr.iOverlayBufNum = 0;
            pxpOverlayAddr.iOverlayBufAddress = m_pOverlaySurfaceOp->nBufPhysicalAddr;
            
            pxpOverlayProperty pxpOverlayProp;
            memset(&pxpOverlayProp, 0 , sizeof(pxpOverlayProp));
            pxpOverlayProp.bEnableOverlay = TRUE;

            if(m_pOverlaySurfaceOp->OverlayPara.B.ALPHASRC_EN)
            {
                pxpOverlayProp.iAlphaOverlay = 0xFF;
                switch(m_pOverlaySurfaceOp->SrcPixelFormat)
                {
                case ddgpePixelFormat_8888:
                    pxpOverlayProp.eAlphaCntl = pxpAlphaControl_EMBEDDED;
                    break;
                case ddgpePixelFormat_5551:
                    pxpOverlayProp.iAlphaOverlay = 0xFF;
                    pxpOverlayProp.eAlphaCntl = pxpAlphaControl_MULTIPLE;           
                    break;
                default:
                    pxpOverlayProp.eAlphaCntl = pxpAlphaControl_EMBEDDED;
                    break;
                }
            }

            if(m_pOverlaySurfaceOp->OverlayPara.B.ALPHACONSTANT_EN)
            {
                pxpOverlayProp.iAlphaOverlay = m_pOverlaySurfaceOp->Transparency;
                pxpOverlayProp.eAlphaCntl = pxpAlphaControl_OVERRIDE;                
            }

            switch(m_pOverlaySurfaceOp->SrcPixelFormat)
            {
            case ddgpePixelFormat_8888:
                pxpOverlayProp.eOverlayFormat = pxpOverlayFormat_ARGB8888;
                break;
            case ddgpePixelFormat_5551:
                pxpOverlayProp.eOverlayFormat = pxpOverlayFormat_ARGB1555;
                break;
            case ddgpePixelFormat_565:
                pxpOverlayProp.eOverlayFormat = pxpOverlayFormat_RGB565;
                break;
            case ddgpePixelFormat_5550:
                pxpOverlayProp.eOverlayFormat = pxpOverlayFormat_RGB555;
                break;
            default:
                RETAILMSG(1,(_T("PXP doesn't support alpha blending source surface pixel format!\r\n")));
            }
            
            pxpOverlayProp.iOverlayBufNum = 0;                

            pxpOverlayBuffersPos pxpOverlayPos;
            pxpOverlayPos.iOverlayBufNum = 0;
            pxpOverlayPos.rOverlayBufRect.left = 0;
            pxpOverlayPos.rOverlayBufRect.right = m_pOverlaySurfaceOp->SrcSurWidth;
            pxpOverlayPos.rOverlayBufRect.top = 0;
            pxpOverlayPos.rOverlayBufRect.bottom = m_pOverlaySurfaceOp->SrcSurHeight;

            pxpColorKey pxpS0ColorKey;  //S0 surface will be transparent
            pxpS0ColorKey.iColorKeyHigh = 0xFFFFFF;
            pxpS0ColorKey.iColorKeyLow = 0;

            pxpOutProperty pxpOutProp;
            memset(&pxpOutProp, 0 , sizeof(pxpOutProp));

            switch(m_pOverlaySurfaceOp->SrcPixelFormat)
            {
            case ddgpePixelFormat_8888:
                pxpOutProp.epxpOutputFmt = pxpOutputFormat_ARGB8888;
                break;
            case ddgpePixelFormat_5551:
                pxpOutProp.epxpOutputFmt = pxpOutputFormat_ARGB1555;
                break;
            case ddgpePixelFormat_565:
                pxpOutProp.epxpOutputFmt = pxpOutputFormat_RGB565;
                break;
            case ddgpePixelFormat_5550:
                pxpOutProp.epxpOutputFmt = pxpOutputFormat_RGB555;
                break;
            default:
                RETAILMSG(1,(_T("PXP doesn't support Alpha blending output format for Rotation!\r\n")));
                return;
            }

            pxpOutProp.iOutputWidth = m_pOverlaySurfaceOp->DstRecWidth & m_nAlignMask;
            pxpOutProp.iOutputHeight = m_pOverlaySurfaceOp->DstRecHeight & m_nAlignMask;
            
            switch(m_pOverlaySurfaceOp->iRotate)
            {
            case DMDO_90:
                pxpOutProp.epxpOutputRot = pxpOutputROT_270;
                break;
            case DMDO_180:
                pxpOutProp.epxpOutputRot = pxpOutputROT_180;   
                break;
            case DMDO_270:
                pxpOutProp.epxpOutputRot = pxpOutputROT_90;
                break;
            default:
                return;
            }//Rotation direction for display screen is counterclockwise, but PXP rotation direction is clockwise, thus conversion is need. 
            if(m_pOverlaySurfaceOp->isUpsideDown)
                pxpOutProp.bVFlip = TRUE;
            if(m_pOverlaySurfaceOp->isLeftsideRight)
                pxpOutProp.bHFlip = TRUE;

            EnterCriticalSection(&m_PxpOperateCS);
            if(!m_hPXP)
            {
                LeaveCriticalSection(&m_PxpOperateCS);
                return;
            }
            
            PXPSetOverlayBuffersAddr(m_hPXP, &pxpOverlayAddr);
            PXPSetOutputProperty(m_hPXP, &pxpOutProp);
            PXPSetOverlayBufsProperty(m_hPXP, &pxpOverlayProp);            
            PXPSetOverlayBuffersPos(m_hPXP, &pxpOverlayPos);
            PXPSetS0BufferColorKey(m_hPXP, &pxpS0ColorKey);            
            PXPSetOutputBuffer1Addr(m_hPXP, m_nLAWPhysical+m_pRotatedSurface->OffsetInVideoMemory());
            PXPStartProcess(m_hPXP, FALSE);   //Rotate overlay surface first           

            pxpOverlayPos.iOverlayBufNum = 0;
            pxpOverlayPos.rOverlayBufRect.left = m_pOverlaySurfaceOp->XOffset;
            pxpOverlayPos.rOverlayBufRect.right = m_pOverlaySurfaceOp->XOffset + m_pOverlaySurfaceOp->DstRecWidthHw;
            pxpOverlayPos.rOverlayBufRect.top = m_pOverlaySurfaceOp->YOffset;
            pxpOverlayPos.rOverlayBufRect.bottom = m_pOverlaySurfaceOp->YOffset + m_pOverlaySurfaceOp->DstRecHeightHw;
               
            pxpOverlayAddr.iOverlayBufNum = 0;
            pxpOverlayAddr.iOverlayBufAddress = m_nLAWPhysical+m_pRotatedSurface->OffsetInVideoMemory();

            if(m_pOverlaySurfaceOp->OverlayPara.B.KEYDEST_EN)
                pxpS0ColorKey = m_pOverlaySurfaceOp->AlphaDestColKeyForRotation;
            else
            {
                pxpS0ColorKey.iColorKeyHigh = 0;
                pxpS0ColorKey.iColorKeyLow = 0xFFFFFF;
            }

            if(m_pOverlaySurfaceOp->OverlayPara.B.KEYSRC_EN)
            {
                pxpOverlayProp.bColorKey = TRUE;
            }
            else if(m_pOverlaySurfaceOp->OverlayPara.B.KEYDEST_EN)
            {
                pxpOverlayProp.bColorKey = FALSE;
            }
        
            memset(&pxpOutProp, 0 , sizeof(pxpOutProp));
        
            switch(m_pOverlaySurfaceOp->DestPixelFormat)
            {
            case ddgpePixelFormat_8888:
                pxpOutProp.epxpOutputFmt = pxpOutputFormat_ARGB8888;
                break;
            case ddgpePixelFormat_5551:
                pxpOutProp.epxpOutputFmt = pxpOutputFormat_ARGB1555;
                break;
            case ddgpePixelFormat_565:
                pxpOutProp.epxpOutputFmt = pxpOutputFormat_RGB565;
                break;
            case ddgpePixelFormat_5550:
                pxpOutProp.epxpOutputFmt = pxpOutputFormat_RGB555;
                break;
            default:
                RETAILMSG(1,(_T("PXP doesn't support alpha blending output format!\r\n")));
                LeaveCriticalSection(&m_PxpOperateCS);
                return;
            }

            pxpOutProp.iOutputWidth = (UINT16)m_nScreenWidthSave;
            pxpOutProp.iOutputHeight = (UINT16)m_nScreenHeightSave;

            PXPSetOverlayBufsProperty(m_hPXP, &pxpOverlayProp);            
            PXPSetS0BufferColorKey(m_hPXP, &pxpS0ColorKey);            
            PXPSetOverlayBuffersAddr(m_hPXP, &pxpOverlayAddr);
            PXPSetOverlayBuffersPos(m_hPXP, &pxpOverlayPos);            
            PXPSetOutputProperty(m_hPXP, &pxpOutProp);
            PXPSetOutputBuffer1Addr(m_hPXP, m_nLAWPhysical+m_pCombinedSurface->OffsetInVideoMemory());
        }
        else    //Overlay surface doesn't need rotation
        {        
            pxpOverlayBuffersAddr pxpOverlayAddr;
            pxpOverlayAddr.iOverlayBufNum = 0;
            pxpOverlayAddr.iOverlayBufAddress = m_pOverlaySurfaceOp->nBufPhysicalAddr;

            EnterCriticalSection(&m_PxpOperateCS);
            if(!m_hPXP)
            {
                LeaveCriticalSection(&m_PxpOperateCS);
                return;
            }

            PXPSetOverlayBuffersAddr(m_hPXP, &pxpOverlayAddr);
        }
    }
    else    //Overlay surface without alpha channel
    {
        if(m_pOverlaySurfaceOp->iRotate||m_pOverlaySurfaceOp->isUpsideDown||m_pOverlaySurfaceOp->isLeftsideRight)   //Overlay surface needs rotation
        {
            pxpS0BufferAddrGroup pxpS0AddrGroup;
            pxpS0AddrGroup.iRGBorYBufAddr = m_pOverlaySurfaceOp->nBufPhysicalAddr;

            if(ddgpePixelFormat_YV12 == m_pOverlaySurfaceOp->SrcPixelFormat)
            {
                UINT32 UBufTempAddr,VBufTempAddr;
                UINT32  YBufSize = m_pOverlaySurfaceOp->SrcSurWidth * m_pOverlaySurfaceOp->SrcSurHeight;
                VBufTempAddr = m_pOverlaySurfaceOp->nBufPhysicalAddr + YBufSize;
                UINT32  UVBufSize = m_pOverlaySurfaceOp->SrcSurWidth * m_pOverlaySurfaceOp->SrcSurHeight/4;
                UBufTempAddr = VBufTempAddr + UVBufSize;

                if(VBufTempAddr%4)  //The address MUST be word-aligned for proper PXP operation. Maybe we can substitute using memory DMA for memcpy.
                {
                    RETAILMSG(1,(_T("Need memcpy VBuf\r\n")));
                    if(!m_pVBufAdjVirtAddr)
                        m_pVBufAdjVirtAddr = (UINT32 *) AllocPhysMem(UVBufSize, PAGE_EXECUTE_READWRITE, 0, 0, (ULONG *) &m_nVBufAdjPhysAddr);

                    if (!m_pVBufAdjVirtAddr)
                        return;   

                    PBYTE pVBufOrgVirtAddr = m_pOverlaySurfaceOp->pBufVirtualAddr + YBufSize;
                    memcpy(m_pVBufAdjVirtAddr,pVBufOrgVirtAddr,UVBufSize);
                    VBufTempAddr = m_nVBufAdjPhysAddr;
                }

                if(UBufTempAddr%4)
                {
                    RETAILMSG(1,(_T("Need memcpy VBuf\r\n")));
                    if(!m_pUBufAdjVirtAddr)
                        m_pUBufAdjVirtAddr = (UINT32 *) AllocPhysMem(UVBufSize, PAGE_EXECUTE_READWRITE, 0, 0, (ULONG *) &m_nUBufAdjPhysAddr);

                    if (!m_pUBufAdjVirtAddr)
                        return;   

                    PBYTE pUBufOrgVirtAddr = m_pOverlaySurfaceOp->pBufVirtualAddr + YBufSize + UVBufSize;
                    memcpy(m_pUBufAdjVirtAddr,pUBufOrgVirtAddr,UVBufSize);
                    UBufTempAddr = m_nUBufAdjPhysAddr;
                }

                pxpS0AddrGroup.iVorCrBufAddr = VBufTempAddr;
                pxpS0AddrGroup.iUorCbBufAddr = UBufTempAddr;
            }
        
            pxpS0Property pxpS0Prop;
            memset(&pxpS0Prop, 0 , sizeof(pxpS0Prop));

            switch(m_pOverlaySurfaceOp->SrcPixelFormat)
            {
            case ddgpePixelFormat_8888:
                pxpS0Prop.eS0BufferFormat = pxpS0BufferFormat_RGB888;
                break;
            case ddgpePixelFormat_565:
                pxpS0Prop.eS0BufferFormat = pxpS0BufferFormat_RGB565;
                break;
            case ddgpePixelFormat_5550:
                pxpS0Prop.eS0BufferFormat = pxpS0BufferFormat_RGB555;
                break;
            case ddgpePixelFormat_YV12:
                pxpS0Prop.eS0BufferFormat = pxpS0BufferFormat_YUV420;
                pxpS0Prop.bYCbCrCsc = TRUE;
                //TBD
                break;
            default:
                RETAILMSG(1,(_T("PXP S0 buffer unsupported format!\r\n")));
            }

            if((m_pOverlaySurfaceOp->SrcXOffset)||(m_pOverlaySurfaceOp->SrcYOffset)||(m_pOverlaySurfaceOp->SrcSurWidth != m_pOverlaySurfaceOp->DstRecWidth)||(m_pOverlaySurfaceOp->SrcSurHeight != m_pOverlaySurfaceOp->DstRecHeight))
            {
                pxpS0Prop.bCrop = TRUE;
                pxpS0Prop.rSOCropRect.left = m_pOverlaySurfaceOp->SrcXOffset;
                pxpS0Prop.rSOCropRect.top = m_pOverlaySurfaceOp->SrcYOffset;
                pxpS0Prop.rSOCropRect.right = m_pOverlaySurfaceOp->SrcXOffset + m_pOverlaySurfaceOp->DstRecWidth;
                pxpS0Prop.rSOCropRect.bottom = m_pOverlaySurfaceOp->SrcYOffset + m_pOverlaySurfaceOp->DstRecHeight;            
            }
                        
            if(((m_pOverlaySurfaceOp->SrcRecWidth != m_pOverlaySurfaceOp->DstRecWidth)||(m_pOverlaySurfaceOp->SrcRecHeight != m_pOverlaySurfaceOp->DstRecHeight))&& (m_pOverlaySurfaceOp->SrcPixelFormat == ddgpePixelFormat_YV12))
            {
                pxpS0Prop.bCrop = TRUE;
                pxpS0Prop.rSOCropRect.left = m_pOverlaySurfaceOp->SrcXOffset;
                pxpS0Prop.rSOCropRect.top = m_pOverlaySurfaceOp->SrcYOffset;
                pxpS0Prop.rSOCropRect.right = m_pOverlaySurfaceOp->SrcXOffset + m_pOverlaySurfaceOp->DstRecWidth;
                pxpS0Prop.rSOCropRect.bottom = m_pOverlaySurfaceOp->SrcYOffset + m_pOverlaySurfaceOp->DstRecHeight;
                pxpS0Prop.bScale = TRUE;
                pxpS0Prop.fXScale = (float)m_pOverlaySurfaceOp->DstRecWidth / (float)(m_pOverlaySurfaceOp->SrcRecWidth-2);          //Workaround For PXP resize
                pxpS0Prop.fYScale = (float)(m_pOverlaySurfaceOp->DstRecHeight-1) / (float)(m_pOverlaySurfaceOp->SrcRecHeight-1);    //Workaround For PXP resize
                pxpS0Prop.fXScaleOffset = 0;
                pxpS0Prop.fYScaleOffset = 0;            
            }            

            pxpCoordinate pxpS0Coor;
            pxpS0Coor.iXBase = 0;
            pxpS0Coor.iYBase = 0;       
            
            pxpRectSize pxpS0RectSize;
            pxpS0RectSize.iWidth = m_pOverlaySurfaceOp->SrcSurWidth;
            pxpS0RectSize.iHeight = m_pOverlaySurfaceOp->SrcSurHeight;       
            
            pxpOverlayProperty pxpOverlayProp;
            memset(&pxpOverlayProp, 0 , sizeof(pxpOverlayProp));
            
            pxpOutProperty pxpOutProp;
            memset(&pxpOutProp, 0 , sizeof(pxpOutProp));
            pxpOutProp.iOutputAlpha = 0xFF;
            pxpOutProp.iOutputWidth =  m_pOverlaySurfaceOp->DstRecWidth & m_nAlignMask;
            pxpOutProp.iOutputHeight = m_pOverlaySurfaceOp->DstRecHeight & m_nAlignMask;
            switch(m_pOverlaySurfaceOp->DestPixelFormat)
            {
            case ddgpePixelFormat_8888:
                pxpOutProp.epxpOutputFmt = pxpOutputFormat_ARGB8888;
                break;
            case ddgpePixelFormat_5551:
                pxpOutProp.epxpOutputFmt = pxpOutputFormat_ARGB1555;
                break;
            case ddgpePixelFormat_565:
                pxpOutProp.epxpOutputFmt = pxpOutputFormat_RGB565;
                break;
            case ddgpePixelFormat_5550:
                pxpOutProp.epxpOutputFmt = pxpOutputFormat_RGB555;
                break;
            default:
                RETAILMSG(1,(_T("PXP output buffer unsupported format!\r\n")));
                return;
            }

            switch(m_pOverlaySurfaceOp->iRotate)
            {
            case DMDO_90:
                pxpOutProp.epxpOutputRot = pxpOutputROT_270;
                break;
            case DMDO_180:
                pxpOutProp.epxpOutputRot = pxpOutputROT_180;   
                break;
            case DMDO_270:
                pxpOutProp.epxpOutputRot = pxpOutputROT_90;
                break;
            default:
                return;
            }//Rotation direction for display screen is counterclockwise, but PXP rotation direction is clockwise, thus conversion is need. 
            if(m_pOverlaySurfaceOp->isUpsideDown)
                pxpOutProp.bVFlip = TRUE;
            if(m_pOverlaySurfaceOp->isLeftsideRight)
                pxpOutProp.bHFlip = TRUE;

            EnterCriticalSection(&m_PxpOperateCS);
            if(!m_hPXP)
            {
                LeaveCriticalSection(&m_PxpOperateCS);
                return;
            }

            PXPSetS0BufProperty(m_hPXP, &pxpS0Prop);
            PXPSetS0BufferOffsetInOutput(m_hPXP, &pxpS0Coor);
            PXPSetS0BufferAddrGroup(m_hPXP, &pxpS0AddrGroup);
            PXPSetOutputProperty(m_hPXP, &pxpOutProp);
            PXPSetS0BufferSize(m_hPXP, &pxpS0RectSize);
            PXPSetOverlayBufsProperty(m_hPXP, &pxpOverlayProp);            
            PXPSetOutputBuffer1Addr(m_hPXP, m_nLAWPhysical+m_pRotatedSurface->OffsetInVideoMemory());
            PXPStartProcess(m_hPXP, FALSE);   //Rotate overlay surface first
            
            pxpS0RectSize.iWidth = m_pOverlaySurfaceOp->DstRecWidthHw;
            pxpS0RectSize.iHeight = m_pOverlaySurfaceOp->DstRecHeightHw; 
            
            memset(&pxpS0Prop, 0 , sizeof(pxpS0Prop));
            switch(m_pOverlaySurfaceOp->DestPixelFormat)
            {
            case ddgpePixelFormat_8888:
                pxpS0Prop.eS0BufferFormat = pxpS0BufferFormat_RGB888;
                break;
            case ddgpePixelFormat_565:
                pxpS0Prop.eS0BufferFormat = pxpS0BufferFormat_RGB565;
                break;
            case ddgpePixelFormat_5550:
                pxpS0Prop.eS0BufferFormat = pxpS0BufferFormat_RGB555;
                break;
            case ddgpePixelFormat_YV12:
                pxpS0Prop.eS0BufferFormat = pxpS0BufferFormat_YUV420;
                pxpS0Prop.bYCbCrCsc = TRUE;
                //TBD
                break;
            default:
                RETAILMSG(1,(_T("PXP S0 buffer unsupported format!\r\n")));
                LeaveCriticalSection(&m_PxpOperateCS);
                return;        
            }

            if(m_pOverlaySurfaceOp->OverlayPara.B.KEYSRC_EN)
                pxpS0Prop.iS0BKColor = m_pOverlaySurfaceOp->SrcColKeyForRotation;
            
            pxpS0AddrGroup.iRGBorYBufAddr = m_nLAWPhysical+m_pRotatedSurface->OffsetInVideoMemory();

            pxpS0Coor.iXBase = m_pOverlaySurfaceOp->XOffset;
            pxpS0Coor.iYBase = m_pOverlaySurfaceOp->YOffset;

            memset(&pxpOverlayProp, 0 , sizeof(pxpOverlayProp));
            if(m_pOverlaySurfaceOp->OverlayPara.B.KEYSRC_EN)
            {
                pxpOverlayProp.bColorKey = FALSE;
                pxpOverlayProp.iAlphaOverlay = 0;
            }
            else if(m_pOverlaySurfaceOp->OverlayPara.B.KEYDEST_EN)
            {
                pxpOverlayProp.bColorKey = TRUE;
                pxpOverlayProp.iAlphaOverlay = 0xFF;
            }

            pxpOverlayProp.bEnableOverlay = TRUE;

            pxpOverlayProp.eAlphaCntl = pxpAlphaControl_OVERRIDE;
            switch(m_pOverlaySurfaceOp->DestPixelFormat)
            {
            case ddgpePixelFormat_8888:
                pxpOverlayProp.eOverlayFormat = pxpOverlayFormat_ARGB8888;
                break;
            case ddgpePixelFormat_5551:
                pxpOverlayProp.eOverlayFormat = pxpOverlayFormat_ARGB1555;
                break;
            case ddgpePixelFormat_565:
                pxpOverlayProp.eOverlayFormat = pxpOverlayFormat_RGB565;
                break;
            case ddgpePixelFormat_5550:
                pxpOverlayProp.eOverlayFormat = pxpOverlayFormat_RGB555;
                break;
            default:
                RETAILMSG(1,(_T("PXP overlay buffer unsupported format!\r\n")));
            }

            pxpOverlayProp.iOverlayBufNum = 0;

            memset(&pxpOutProp, 0 , sizeof(pxpOutProp));
            pxpOutProp.iOutputAlpha = 0xFF;
            pxpOutProp.iOutputWidth =  (UINT16)m_nScreenWidthSave;
            pxpOutProp.iOutputHeight = (UINT16)m_nScreenHeightSave;

            switch(m_pOverlaySurfaceOp->DestPixelFormat)
            {
            case ddgpePixelFormat_8888:
                pxpOutProp.epxpOutputFmt = pxpOutputFormat_ARGB8888;
                break;
            case ddgpePixelFormat_5551:
                pxpOutProp.epxpOutputFmt = pxpOutputFormat_ARGB1555;
                break;
            case ddgpePixelFormat_565:
                pxpOutProp.epxpOutputFmt = pxpOutputFormat_RGB565;
                break;
            case ddgpePixelFormat_5550:
                pxpOutProp.epxpOutputFmt = pxpOutputFormat_RGB555;
                break;
            default:
                RETAILMSG(1,(_T("PXP overlay buffer unsupported format!\r\n")));
                LeaveCriticalSection(&m_PxpOperateCS);
                return;
            }

            pxpOverlayBuffersPos pxpOverlayPos;
            pxpOverlayPos.iOverlayBufNum = 0;
            pxpOverlayPos.rOverlayBufRect.left = 0;
            pxpOverlayPos.rOverlayBufRect.right = m_nScreenWidthSave;
            pxpOverlayPos.rOverlayBufRect.top = 0;
            pxpOverlayPos.rOverlayBufRect.bottom = m_nScreenHeightSave;

            pxpOverlayBuffersAddr pxpOverlayAddr;
            pxpOverlayAddr.iOverlayBufNum = 0;
            pxpOverlayAddr.iOverlayBufAddress = (UINT32) m_nLAWPhysical+m_pBackgroundSurface->OffsetInVideoMemory();
  

            PXPSetS0BufferSize(m_hPXP, &pxpS0RectSize); //Combine overlay surface secondly
            PXPSetOverlayBuffersAddr(m_hPXP, &pxpOverlayAddr);
            PXPSetS0BufProperty(m_hPXP, &pxpS0Prop);
            PXPSetS0BufferAddrGroup(m_hPXP, &pxpS0AddrGroup);
            PXPSetS0BufferOffsetInOutput(m_hPXP, &pxpS0Coor);            
            PXPSetOverlayBufsProperty(m_hPXP, &pxpOverlayProp);
            PXPSetOutputProperty(m_hPXP, &pxpOutProp);
            PXPSetOverlayBuffersPos(m_hPXP, &pxpOverlayPos);
            PXPSetOutputBuffer1Addr(m_hPXP, m_nLAWPhysical+m_pCombinedSurface->OffsetInVideoMemory());            

        }
        else    //Overlay surface doesn't need rotation
        {
            pxpS0BufferAddrGroup pxpS0AddrGroup;
            pxpS0AddrGroup.iRGBorYBufAddr = m_pOverlaySurfaceOp->nBufPhysicalAddr;

            if(ddgpePixelFormat_YV12 == m_pOverlaySurfaceOp->SrcPixelFormat)
            {
                UINT32 UBufTempAddr,VBufTempAddr;
                UINT32  YBufSize = m_pOverlaySurfaceOp->SrcSurWidth * m_pOverlaySurfaceOp->SrcSurHeight;
                VBufTempAddr = m_pOverlaySurfaceOp->nBufPhysicalAddr + YBufSize;
                UINT32  UVBufSize = m_pOverlaySurfaceOp->SrcSurWidth * m_pOverlaySurfaceOp->SrcSurHeight/4;
                UBufTempAddr = VBufTempAddr + UVBufSize;

                if(VBufTempAddr%4)  //The address MUST be word-aligned for proper PXP operation. Maybe we can substitute using memory DMA for memcpy.
                {
                    RETAILMSG(1,(_T("Need memcpy VBuf\r\n")));
                    if(!m_pVBufAdjVirtAddr)
                        m_pVBufAdjVirtAddr = (UINT32 *) AllocPhysMem(UVBufSize, PAGE_EXECUTE_READWRITE, 0, 0, (ULONG *) &m_nVBufAdjPhysAddr);

                    if (!m_pVBufAdjVirtAddr)
                        return;   

                    PBYTE pVBufOrgVirtAddr = m_pOverlaySurfaceOp->pBufVirtualAddr + YBufSize;
                    memcpy(m_pVBufAdjVirtAddr,pVBufOrgVirtAddr,UVBufSize);
                    VBufTempAddr = m_nVBufAdjPhysAddr;
                }

                if(UBufTempAddr%4)
                {
                    RETAILMSG(1,(_T("Need memcpy VBuf\r\n")));
                    if(!m_pUBufAdjVirtAddr)
                        m_pUBufAdjVirtAddr = (UINT32 *) AllocPhysMem(UVBufSize, PAGE_EXECUTE_READWRITE, 0, 0, (ULONG *) &m_nUBufAdjPhysAddr);

                    if (!m_pUBufAdjVirtAddr)
                        return;   

                    PBYTE pUBufOrgVirtAddr = m_pOverlaySurfaceOp->pBufVirtualAddr + YBufSize + UVBufSize;
                    memcpy(m_pUBufAdjVirtAddr,pUBufOrgVirtAddr,UVBufSize);
                    UBufTempAddr = m_nUBufAdjPhysAddr;
                }

                pxpS0AddrGroup.iVorCrBufAddr = VBufTempAddr;
                pxpS0AddrGroup.iUorCbBufAddr = UBufTempAddr;

                EnterCriticalSection(&m_PxpOperateCS);
                if(!m_hPXP)
                {
                    LeaveCriticalSection(&m_PxpOperateCS);
                    return;
                }
                                
                PXPSetS0BufferAddrGroup(m_hPXP, &pxpS0AddrGroup);
            }
            else
            {
                EnterCriticalSection(&m_PxpOperateCS);
                if(!m_hPXP)
                {
                    LeaveCriticalSection(&m_PxpOperateCS);
                    return;
                }

                PXPSetS0BufferAddrGroup(m_hPXP, &pxpS0AddrGroup);
            }
        }
    }
  
    PXPStartProcess(m_hPXP, FALSE);
    SetEvent(m_hCombSurfUpdatedEvent);   
    LeaveCriticalSection(&m_PxpOperateCS);

    //DEBUGMSG(HAL_ZONE_INIT, (TEXT("DDLcdif SetVisibleSurfaceOverlay:\r\n")));    
    
    return;
}


//------------------------------------------------------------------------------
//
// Function: EnableOverlay
//
//  DESCRIPTION:   This function enable the graphic window of LCDIF of 
//                 i.MX processor for overlay window.
//
// Parameters:
//      None.
//
// Returns:
//      DD_OK                successful
//      others               failed
//
//------------------------------------------------------------------------------
HRESULT DDLcdif::EnableOverlay(VOID)
{
    HRESULT result = DDERR_OUTOFCAPS;

    UINT32 tempColorKeyLow = 0;
    UINT32 tempColorKeyHigh = 0;
    UINT32 RGBColorKeyLow = 0;
    UINT32 RGBColorKeyHigh = 0;

    
    pxpColorKey NoColorKey, SetColorKey, pxpS0ColorKey, pxpOverlayColorKey;    
    NoColorKey.iColorKeyHigh = 0; //First disable colorkey first
    NoColorKey.iColorKeyLow = 0xFFFFFF;
    SetColorKey = pxpS0ColorKey = pxpOverlayColorKey = NoColorKey;    

    if(m_pOverlaySurfaceOp->OverlayPara.B.KEYDEST_EN)
    {
        tempColorKeyLow = m_pOverlaySurfaceOp->DestColorKeyLow;
        tempColorKeyHigh = m_pOverlaySurfaceOp->DestColorKeyHigh;
        switch(m_pMode->Bpp)
        {
        case 18:
            // TODO: NEED CHECK
            RGBColorKeyLow =
                (((tempColorKeyLow & 0x3F000) << (GRAPH_WIND_CTRL_SDC_KEY_COLOR_R_LSH-12+2)) |
                ((tempColorKeyLow & 0x00FC0) << (GRAPH_WIND_CTRL_SDC_KEY_COLOR_G_LSH-6+2)) |
                ((tempColorKeyHigh & 0x0003F) << (GRAPH_WIND_CTRL_SDC_KEY_COLOR_B_LSH+2)));
            RGBColorKeyHigh =
                (((tempColorKeyHigh & 0x3F000) << (GRAPH_WIND_CTRL_SDC_KEY_COLOR_R_LSH-12+2)) |
                ((tempColorKeyHigh & 0x00FC0) << (GRAPH_WIND_CTRL_SDC_KEY_COLOR_G_LSH-6+2)) |
                ((tempColorKeyHigh & 0x0003F) << (GRAPH_WIND_CTRL_SDC_KEY_COLOR_B_LSH+2)));
            break;
        case 16:
            // We add 3 to R, 2 to G, and 3 to B, because PXP internal format is 888.
            // Thus, we must expand from 565 color key values to 888.
            RGBColorKeyLow =
                (((tempColorKeyLow & 0xF800) << (GRAPH_WIND_CTRL_SDC_KEY_COLOR_R_LSH-11+3)) |
                ((tempColorKeyLow & 0x07E0) << (GRAPH_WIND_CTRL_SDC_KEY_COLOR_G_LSH-5+2)) |
                ((tempColorKeyLow & 0x001F) << (GRAPH_WIND_CTRL_SDC_KEY_COLOR_B_LSH+3)));
            RGBColorKeyHigh =
                (((tempColorKeyHigh & 0xF800) << (GRAPH_WIND_CTRL_SDC_KEY_COLOR_R_LSH-11+3)) |
                ((tempColorKeyHigh & 0x07E0) << (GRAPH_WIND_CTRL_SDC_KEY_COLOR_G_LSH-5+2)) |
                ((tempColorKeyHigh & 0x001F) << (GRAPH_WIND_CTRL_SDC_KEY_COLOR_B_LSH+3)));
            break;        
        case 24:
        case 32:
            RGBColorKeyLow = tempColorKeyLow;
            RGBColorKeyHigh = tempColorKeyHigh;
            break;
        default:
            RGBColorKeyLow = 0xFFFFFF;
            RGBColorKeyHigh = 0;
            break;
        }

        SetColorKey.iColorKeyHigh = RGBColorKeyHigh;
        SetColorKey.iColorKeyLow = RGBColorKeyLow;

        if(m_pOverlaySurfaceOp->OverlayPara.B.ALPHASRC_EN||m_pOverlaySurfaceOp->OverlayPara.B.ALPHACONSTANT_EN)
        {
            m_pOverlaySurfaceOp->AlphaDestColKeyForRotation = SetColorKey;
        }

    }

    if(m_pOverlaySurfaceOp->OverlayPara.B.KEYSRC_EN)
    {
        tempColorKeyLow = m_pOverlaySurfaceOp->SrcColorKeyLow;
        tempColorKeyHigh = m_pOverlaySurfaceOp->SrcColorKeyHigh;
        switch(m_pMode->Bpp)
        {
        case 18:
            // TODO: NEED CHECK
            RGBColorKeyLow =
                (((tempColorKeyLow & 0x3F000) << (GRAPH_WIND_CTRL_SDC_KEY_COLOR_R_LSH-12+2)) |
                ((tempColorKeyLow & 0x00FC0) << (GRAPH_WIND_CTRL_SDC_KEY_COLOR_G_LSH-6+2)) |
                ((tempColorKeyHigh & 0x0003F) << (GRAPH_WIND_CTRL_SDC_KEY_COLOR_B_LSH+2)));
            RGBColorKeyHigh =
                (((tempColorKeyHigh & 0x3F000) << (GRAPH_WIND_CTRL_SDC_KEY_COLOR_R_LSH-12+2)) |
                ((tempColorKeyHigh & 0x00FC0) << (GRAPH_WIND_CTRL_SDC_KEY_COLOR_G_LSH-6+2)) |
                ((tempColorKeyHigh & 0x0003F) << (GRAPH_WIND_CTRL_SDC_KEY_COLOR_B_LSH+2)));
            break;
        case 16:
            // We add 3 to R, 2 to G, and 3 to B, because PXP internal format is 888.
            // Thus, we must expand from 565 color key values to 888.
            RGBColorKeyLow =
                (((tempColorKeyLow & 0xF800) << (GRAPH_WIND_CTRL_SDC_KEY_COLOR_R_LSH-11+3)) |
                ((tempColorKeyLow & 0x07E0) << (GRAPH_WIND_CTRL_SDC_KEY_COLOR_G_LSH-5+2)) |
                ((tempColorKeyLow & 0x001F) << (GRAPH_WIND_CTRL_SDC_KEY_COLOR_B_LSH+3)));
            RGBColorKeyHigh =
                (((tempColorKeyHigh & 0xF800) << (GRAPH_WIND_CTRL_SDC_KEY_COLOR_R_LSH-11+3)) |
                ((tempColorKeyHigh & 0x07E0) << (GRAPH_WIND_CTRL_SDC_KEY_COLOR_G_LSH-5+2)) |
                ((tempColorKeyHigh & 0x001F) << (GRAPH_WIND_CTRL_SDC_KEY_COLOR_B_LSH+3)));
            break;        
        case 24:
        case 32:
            RGBColorKeyLow = tempColorKeyLow;
            RGBColorKeyHigh = tempColorKeyHigh;
            break;
        default:
            RGBColorKeyLow = 0xFFFFFF;
            RGBColorKeyHigh = 0;
            break;
        }

        m_pOverlaySurfaceOp->SrcColKeyForRotation = RGBColorKeyLow;

        SetColorKey.iColorKeyHigh = RGBColorKeyHigh;
        SetColorKey.iColorKeyLow = RGBColorKeyLow;

    }

    if(!m_pCombinedSurface)
    {
        if(FAILED(AllocSurface((GPESurf **)&m_pCombinedSurface,
            m_nScreenWidthSave,
            m_nScreenHeightSave,
            EDDGPEPixelFormatToEGPEFormat[m_pOverlaySurfaceOp->DestPixelFormat],
            GPE_REQUIRE_VIDEO_MEMORY))) {
                RETAILMSG (1, (L"Couldn't allocate combine surface\r\n"));
                m_pVisibleOverlay = NULL;
                return  DDERR_OUTOFMEMORY;
        }
    }

    if(m_pOverlaySurfaceOp->OverlayPara.B.ALPHASRC_EN||m_pOverlaySurfaceOp->OverlayPara.B.ALPHACONSTANT_EN) //Overlay surface with alpha channel
    {
        if(m_pOverlaySurfaceOp->iRotate||m_pOverlaySurfaceOp->isUpsideDown||m_pOverlaySurfaceOp->isLeftsideRight)   //Overlay surface needs rotation
        {
            pxpCoordinate pxpS0Coor;
            pxpS0Coor.iXBase = 0;
            pxpS0Coor.iYBase = 0;
            
            pxpS0BufferAddrGroup pxpS0AddrGroup;
            pxpS0AddrGroup.iRGBorYBufAddr = m_nLAWPhysical+m_pBackgroundSurface->OffsetInVideoMemory();

            pxpRectSize pxpS0RectSize;
            pxpS0RectSize.iWidth = (UINT16)m_nScreenWidthSave;
            pxpS0RectSize.iHeight = (UINT16)m_nScreenHeightSave;

            pxpS0Property pxpS0Prop;
            memset(&pxpS0Prop, 0 , sizeof(pxpS0Prop));

            switch(m_pOverlaySurfaceOp->DestPixelFormat)
            {
            case ddgpePixelFormat_8888:
                pxpS0Prop.eS0BufferFormat = pxpS0BufferFormat_RGB888;
                break;
            case ddgpePixelFormat_565:
                pxpS0Prop.eS0BufferFormat = pxpS0BufferFormat_RGB565;
                break;
            case ddgpePixelFormat_5551:            
            case ddgpePixelFormat_5550:
                pxpS0Prop.eS0BufferFormat = pxpS0BufferFormat_RGB555;
                break;
            default:
                RETAILMSG(1,(_T("PXP doesn't support alpha blending destination format!\r\n")));
                return DDERR_UNSUPPORTED;
            }
            
            pxpOverlayBuffersAddr pxpOverlayAddr;
            pxpOverlayAddr.iOverlayBufNum = 0;
            pxpOverlayAddr.iOverlayBufAddress = m_pOverlaySurfaceOp->nBufPhysicalAddr;

            pxpOverlayProperty pxpOverlayProp;
            memset(&pxpOverlayProp, 0 , sizeof(pxpOverlayProp));
            pxpOverlayProp.bEnableOverlay = TRUE;

            if(m_pOverlaySurfaceOp->OverlayPara.B.ALPHASRC_EN)
            {
                switch(m_pOverlaySurfaceOp->SrcPixelFormat)
                {
                case ddgpePixelFormat_8888:
                    pxpOverlayProp.eAlphaCntl = pxpAlphaControl_EMBEDDED;
                    break;
                case ddgpePixelFormat_5551:
                    pxpOverlayProp.iAlphaOverlay = 0xFF;
                    pxpOverlayProp.eAlphaCntl = pxpAlphaControl_MULTIPLE;           
                    break;
                default:
                    pxpOverlayProp.eAlphaCntl = pxpAlphaControl_EMBEDDED;
                    break;
                }
            }

            if(m_pOverlaySurfaceOp->OverlayPara.B.ALPHACONSTANT_EN)
            {
                pxpOverlayProp.iAlphaOverlay = m_pOverlaySurfaceOp->Transparency;
                pxpOverlayProp.eAlphaCntl = pxpAlphaControl_OVERRIDE;                
            }
                
            switch(m_pOverlaySurfaceOp->SrcPixelFormat)
            {
            case ddgpePixelFormat_8888:
                pxpOverlayProp.eOverlayFormat = pxpOverlayFormat_ARGB8888;
                break;
            case ddgpePixelFormat_5551:
                pxpOverlayProp.eOverlayFormat = pxpOverlayFormat_ARGB1555;
                break;
            case ddgpePixelFormat_565:
                pxpOverlayProp.eOverlayFormat = pxpOverlayFormat_RGB565;
                break;
            case ddgpePixelFormat_5550:
                pxpOverlayProp.eOverlayFormat = pxpOverlayFormat_RGB555;
                break;
            default:
                RETAILMSG(1,(_T("PXP doesn't support Alpha blending source surface pixel format!\r\n")));
            }

            pxpOverlayProp.iOverlayBufNum = 0;       
        
            pxpOverlayBuffersPos pxpOverlayPos;
            pxpOverlayPos.iOverlayBufNum = 0;
            pxpOverlayPos.rOverlayBufRect.left = 0;
            pxpOverlayPos.rOverlayBufRect.right = m_pOverlaySurfaceOp->SrcSurWidth;
            pxpOverlayPos.rOverlayBufRect.top = 0;
            pxpOverlayPos.rOverlayBufRect.bottom = m_pOverlaySurfaceOp->SrcSurHeight;

            pxpOutProperty pxpOutProp;
            memset(&pxpOutProp, 0 , sizeof(pxpOutProp));
            
            switch(m_pOverlaySurfaceOp->SrcPixelFormat)
            {
            case ddgpePixelFormat_8888:
                pxpOutProp.epxpOutputFmt = pxpOutputFormat_ARGB8888;
                break;
            case ddgpePixelFormat_5551:
                pxpOutProp.epxpOutputFmt = pxpOutputFormat_ARGB1555;
                break;
            case ddgpePixelFormat_565:
                pxpOutProp.epxpOutputFmt = pxpOutputFormat_RGB565;
                break;
            case ddgpePixelFormat_5550:
                pxpOutProp.epxpOutputFmt = pxpOutputFormat_RGB555;
                break;
            default:
                RETAILMSG(1,(_T("PXP doesn't support alpha blending output format for Rotation!\r\n")));
                return DDERR_UNSUPPORTED;
            }

            pxpOutProp.iOutputWidth = m_pOverlaySurfaceOp->DstRecWidth & m_nAlignMask;
            pxpOutProp.iOutputHeight = m_pOverlaySurfaceOp->DstRecHeight & m_nAlignMask;
            
            switch(m_pOverlaySurfaceOp->iRotate)
            {
            case DMDO_90:
                pxpOutProp.epxpOutputRot = pxpOutputROT_270;
                break;
            case DMDO_180:
                pxpOutProp.epxpOutputRot = pxpOutputROT_180;   
                break;
            case DMDO_270:
                pxpOutProp.epxpOutputRot = pxpOutputROT_90;
                break;
            default:
                return DDERR_UNSUPPORTED;
            }//Rotation direction for display screen is counterclockwise, but PXP rotation direction is clockwise, thus conversion is need. 
            if(m_pOverlaySurfaceOp->isUpsideDown)
                pxpOutProp.bVFlip = TRUE;
            if(m_pOverlaySurfaceOp->isLeftsideRight)
                pxpOutProp.bHFlip = TRUE;
            
            if((!m_pRotatedSurface)||(m_iOrgRotSurfaceWidth!=(m_pOverlaySurfaceOp->DstRecWidth & m_nAlignMask))||(m_iOrgRotSurfaceHeight!=(m_pOverlaySurfaceOp->DstRecHeight & m_nAlignMask)))
            {
                if(m_pRotatedSurface)
                    delete m_pRotatedSurface;

                if(FAILED(AllocSurface(&m_pRotatedSurface,
                    m_pOverlaySurfaceOp->DstRecWidthHw & m_nAlignMask,
                    m_pOverlaySurfaceOp->DstRecHeightHw & m_nAlignMask,
                    EDDGPEPixelFormatToEGPEFormat[m_pOverlaySurfaceOp->SrcPixelFormat],
                    m_pOverlaySurfaceOp->SrcPixelFormat,
                    GPE_REQUIRE_VIDEO_MEMORY))) {
                        RETAILMSG (1, (L"Couldn't allocate rotation surface\n"));
                        return  DDERR_OUTOFMEMORY;
                    }

                m_iOrgRotSurfaceWidth = m_pOverlaySurfaceOp->DstRecWidth & m_nAlignMask;
                m_iOrgRotSurfaceHeight = m_pOverlaySurfaceOp->DstRecHeight & m_nAlignMask;            
            }

            EnterCriticalSection(&m_PxpOperateCS);
            if (!m_hPXP)
            {
                m_hPXP = PXPOpenHandle();
                DEBUGMSG(1,(_T("m_hPXP = %0x\r\n"),m_hPXP));
                // Bail if no PXP handle
                if ((m_hPXP == NULL)||(m_hPXP == INVALID_HANDLE_VALUE))
                {
                    RETAILMSG(1,(_T("No Pixel Pipeline handle available! \r\n")));
                    LeaveCriticalSection(&m_PxpOperateCS);
                    return DDERR_UNSUPPORTED;
                }

                pxpParaConfig pxpPara;
                memset(&pxpPara, 0 , sizeof(pxpPara));
                pxpPara.iBlockSize = m_nOverlayAlign; 
                PXPConfigureGeneral(m_hPXP, &pxpPara);
                PXPInterruptEnable(m_hPXP);
            }

                                      
            PXPSetS0BufferOffsetInOutput(m_hPXP, &pxpS0Coor);
            PXPSetS0BufferAddrGroup(m_hPXP, &pxpS0AddrGroup);
            PXPSetS0BufferSize(m_hPXP, &pxpS0RectSize);
            PXPSetS0BufProperty(m_hPXP, &pxpS0Prop);
            PXPSetOverlayBuffersAddr(m_hPXP, &pxpOverlayAddr);
            PXPSetOverlayBufsProperty(m_hPXP, &pxpOverlayProp);
            PXPSetOverlayBuffersPos(m_hPXP, &pxpOverlayPos);
            PXPSetS0BufferColorKey(m_hPXP, &NoColorKey);
            PXPSetOverlayColorKey(m_hPXP, &NoColorKey);            
            PXPSetOutputProperty(m_hPXP, &pxpOutProp);            
            PXPSetOutputBuffer1Addr(m_hPXP, m_nLAWPhysical+m_pRotatedSurface->OffsetInVideoMemory());
            PXPStartProcess(m_hPXP, FALSE);   //Rotate overlay surface first

            pxpOverlayPos.iOverlayBufNum = 0;
            pxpOverlayPos.rOverlayBufRect.left = m_pOverlaySurfaceOp->XOffset;
            pxpOverlayPos.rOverlayBufRect.right = m_pOverlaySurfaceOp->XOffset + m_pOverlaySurfaceOp->DstRecWidthHw;
            pxpOverlayPos.rOverlayBufRect.top = m_pOverlaySurfaceOp->YOffset;
            pxpOverlayPos.rOverlayBufRect.bottom = m_pOverlaySurfaceOp->YOffset + m_pOverlaySurfaceOp->DstRecHeightHw;
               
            pxpOverlayAddr.iOverlayBufNum = 0;
            pxpOverlayAddr.iOverlayBufAddress = m_nLAWPhysical+m_pRotatedSurface->OffsetInVideoMemory();

            memset(&pxpOutProp, 0 , sizeof(pxpOutProp));
            
            switch(m_pOverlaySurfaceOp->DestPixelFormat)
            {
            case ddgpePixelFormat_8888:
                pxpOutProp.epxpOutputFmt = pxpOutputFormat_ARGB8888;
                break;
            case ddgpePixelFormat_5551:
                pxpOutProp.epxpOutputFmt = pxpOutputFormat_ARGB1555;
                break;
            case ddgpePixelFormat_565:
                pxpOutProp.epxpOutputFmt = pxpOutputFormat_RGB565;
                break;
            case ddgpePixelFormat_5550:
                pxpOutProp.epxpOutputFmt = pxpOutputFormat_RGB555;
                break;
            default:
                RETAILMSG(1,(_T("PXP doesn't support alpha blending output format!\r\n")));
                LeaveCriticalSection(&m_PxpOperateCS);
                return DDERR_UNSUPPORTED;
            }

            pxpOutProp.iOutputWidth = (UINT16)m_nScreenWidthSave;
            pxpOutProp.iOutputHeight = (UINT16)m_nScreenHeightSave;
            

            if(m_pOverlaySurfaceOp->OverlayPara.B.KEYDEST_EN)   //Combine overlay surface secondly
            {
                pxpOverlayProp.bColorKey = FALSE;
                PXPSetS0BufferColorKey(m_hPXP, &SetColorKey);
            }
            if(m_pOverlaySurfaceOp->OverlayPara.B.KEYSRC_EN)
            {
                pxpOverlayProp.bColorKey = TRUE;
                PXPSetOverlayColorKey(m_hPXP, &SetColorKey);
            }

            PXPSetOutputBuffer1Addr(m_hPXP, m_nLAWPhysical+m_pCombinedSurface->OffsetInVideoMemory());
            PXPSetOutputProperty(m_hPXP, &pxpOutProp);            
            PXPSetOverlayBufsProperty(m_hPXP, &pxpOverlayProp);
            PXPSetOverlayBuffersAddr(m_hPXP, &pxpOverlayAddr);            
            PXPSetOverlayBuffersPos(m_hPXP, &pxpOverlayPos);            
         
        }
        else    //Overlay surface doesn't need rotation
        {
            pxpCoordinate pxpS0Coor;
            pxpS0Coor.iXBase = 0;
            pxpS0Coor.iYBase = 0;
            
            pxpS0BufferAddrGroup pxpS0AddrGroup;
            pxpS0AddrGroup.iRGBorYBufAddr = m_nLAWPhysical+m_pBackgroundSurface->OffsetInVideoMemory();        
            
            pxpRectSize pxpS0RectSize;
            pxpS0RectSize.iWidth = (UINT16)m_nScreenWidthSave;
            pxpS0RectSize.iHeight = (UINT16)m_nScreenHeightSave; 
            
            pxpS0Property pxpS0Prop;
            memset(&pxpS0Prop, 0 , sizeof(pxpS0Prop));
            
            switch(m_pOverlaySurfaceOp->DestPixelFormat)
            {
            case ddgpePixelFormat_8888:
                pxpS0Prop.eS0BufferFormat = pxpS0BufferFormat_RGB888;
                break;
            case ddgpePixelFormat_565:
                pxpS0Prop.eS0BufferFormat = pxpS0BufferFormat_RGB565;
                break;
            case ddgpePixelFormat_5551:            
            case ddgpePixelFormat_5550:
                pxpS0Prop.eS0BufferFormat = pxpS0BufferFormat_RGB555;
                break;
            default:
                RETAILMSG(1,(_T("PXP doesn't support alpha blending destination format!\r\n")));
                return DDERR_UNSUPPORTED;
            }
                        
            pxpOverlayBuffersAddr pxpOverlayAddr;
            pxpOverlayAddr.iOverlayBufNum = 0;
            pxpOverlayAddr.iOverlayBufAddress = m_pOverlaySurfaceOp->nBufPhysicalAddr;
            
            pxpOverlayProperty pxpOverlayProp;
            memset(&pxpOverlayProp, 0 , sizeof(pxpOverlayProp));
            pxpOverlayProp.bEnableOverlay = TRUE;
            
            if(m_pOverlaySurfaceOp->OverlayPara.B.ALPHASRC_EN)
            {
                switch(m_pOverlaySurfaceOp->SrcPixelFormat)
                {
                case ddgpePixelFormat_8888:
                    pxpOverlayProp.eAlphaCntl = pxpAlphaControl_EMBEDDED;
                    break;
                case ddgpePixelFormat_5551:
                    pxpOverlayProp.iAlphaOverlay = 0xFF;
                    pxpOverlayProp.eAlphaCntl = pxpAlphaControl_MULTIPLE;           
                    break;
                default:
                    pxpOverlayProp.eAlphaCntl = pxpAlphaControl_EMBEDDED;
                    break;
                }
            }
            
            if(m_pOverlaySurfaceOp->OverlayPara.B.ALPHACONSTANT_EN)
            {
                pxpOverlayProp.iAlphaOverlay = m_pOverlaySurfaceOp->Transparency;
                pxpOverlayProp.eAlphaCntl = pxpAlphaControl_OVERRIDE;                
            }
                
            switch(m_pOverlaySurfaceOp->SrcPixelFormat)
            {
            case ddgpePixelFormat_8888:
                pxpOverlayProp.eOverlayFormat = pxpOverlayFormat_ARGB8888;
                break;
            case ddgpePixelFormat_5551:
                pxpOverlayProp.eOverlayFormat = pxpOverlayFormat_ARGB1555;
                break;
            case ddgpePixelFormat_565:
                pxpOverlayProp.eOverlayFormat = pxpOverlayFormat_RGB565;
                break;
            case ddgpePixelFormat_5550:
                pxpOverlayProp.eOverlayFormat = pxpOverlayFormat_RGB555;
                break;
            default:
                RETAILMSG(1,(_T("PXP doesn't support Alpha blending source surface pixel format!\r\n")));
            }
            if(m_pOverlaySurfaceOp->OverlayPara.B.KEYSRC_EN)
            {
                pxpOverlayProp.bColorKey = TRUE;
                pxpOverlayColorKey = SetColorKey;
            }
            else if(m_pOverlaySurfaceOp->OverlayPara.B.KEYDEST_EN)
            {
                pxpOverlayProp.bColorKey = FALSE;
                pxpS0ColorKey = SetColorKey;
            }
        
            pxpOverlayProp.iOverlayBufNum = 0;
        
            pxpOverlayBuffersPos pxpOverlayPos;
            pxpOverlayPos.iOverlayBufNum = 0;
            pxpOverlayPos.rOverlayBufRect.left = m_pOverlaySurfaceOp->XOffset;
            pxpOverlayPos.rOverlayBufRect.right = m_pOverlaySurfaceOp->XOffset + m_pOverlaySurfaceOp->SrcSurWidth;
            pxpOverlayPos.rOverlayBufRect.top = m_pOverlaySurfaceOp->YOffset;
            pxpOverlayPos.rOverlayBufRect.bottom = m_pOverlaySurfaceOp->YOffset + m_pOverlaySurfaceOp->SrcSurHeight;

            pxpOutProperty pxpOutProp;
            memset(&pxpOutProp, 0 , sizeof(pxpOutProp));
            
            switch(m_pOverlaySurfaceOp->DestPixelFormat)
            {
            case ddgpePixelFormat_8888:
                pxpOutProp.epxpOutputFmt = pxpOutputFormat_ARGB8888;
                break;
            case ddgpePixelFormat_5551:
                pxpOutProp.epxpOutputFmt = pxpOutputFormat_ARGB1555;
                break;
            case ddgpePixelFormat_565:
                pxpOutProp.epxpOutputFmt = pxpOutputFormat_RGB565;
                break;
            case ddgpePixelFormat_5550:
                pxpOutProp.epxpOutputFmt = pxpOutputFormat_RGB555;
                break;
            default:
                RETAILMSG(1,(_T("PXP doesn't support alpha blending output format!\r\n")));
                return DDERR_UNSUPPORTED;
            }
            
            pxpOutProp.iOutputWidth = (UINT16)m_nScreenWidthSave;
            pxpOutProp.iOutputHeight = (UINT16)m_nScreenHeightSave;
           
            EnterCriticalSection(&m_PxpOperateCS);
            if (!m_hPXP)
            {
                m_hPXP = PXPOpenHandle();
                DEBUGMSG(1,(_T("m_hPXP = %0x\r\n"),m_hPXP));
                // Bail if no PXP handle
                if ((m_hPXP == NULL)||(m_hPXP == INVALID_HANDLE_VALUE))
                {
                    RETAILMSG(1,(_T("No Pixel Pipeline handle available! \r\n")));
                    LeaveCriticalSection(&m_PxpOperateCS);
                    return DDERR_UNSUPPORTED;
                }

                pxpParaConfig pxpPara;
                memset(&pxpPara, 0 , sizeof(pxpPara));
                pxpPara.iBlockSize = m_nOverlayAlign;                
                PXPConfigureGeneral(m_hPXP, &pxpPara);
                PXPInterruptEnable(m_hPXP);
            }


            PXPSetOutputProperty(m_hPXP, &pxpOutProp);
            PXPSetS0BufferOffsetInOutput(m_hPXP, &pxpS0Coor);
            PXPSetS0BufferAddrGroup(m_hPXP, &pxpS0AddrGroup);            
            PXPSetS0BufferColorKey(m_hPXP, &pxpS0ColorKey);
            PXPSetOverlayColorKey(m_hPXP, &pxpOverlayColorKey);
            PXPSetS0BufferSize(m_hPXP, &pxpS0RectSize);            
            PXPSetOverlayBuffersPos(m_hPXP, &pxpOverlayPos);
            PXPSetS0BufProperty(m_hPXP, &pxpS0Prop);
            PXPSetOverlayBufsProperty(m_hPXP, &pxpOverlayProp);            
            PXPSetOverlayBuffersAddr(m_hPXP, &pxpOverlayAddr);
            PXPSetOutputBuffer1Addr(m_hPXP, m_nLAWPhysical+m_pCombinedSurface->OffsetInVideoMemory());
            
        }
                     
    }
    else    //Overlay surface without alpha channel
    {
        if(m_pOverlaySurfaceOp->iRotate||m_pOverlaySurfaceOp->isUpsideDown||m_pOverlaySurfaceOp->isLeftsideRight)   //Overlay surface needs rotation
        {
            pxpS0BufferAddrGroup pxpS0AddrGroup;
            pxpS0AddrGroup.iRGBorYBufAddr = m_pOverlaySurfaceOp->nBufPhysicalAddr;
            
            if(ddgpePixelFormat_YV12 == m_pOverlaySurfaceOp->SrcPixelFormat)
            {
                UINT32 UBufTempAddr,VBufTempAddr;
                UINT32  YBufSize = m_pOverlaySurfaceOp->SrcSurWidth * m_pOverlaySurfaceOp->SrcSurHeight;
                VBufTempAddr = m_pOverlaySurfaceOp->nBufPhysicalAddr + YBufSize;
                UINT32  UVBufSize = m_pOverlaySurfaceOp->SrcSurWidth * m_pOverlaySurfaceOp->SrcSurHeight/4;
                UBufTempAddr = VBufTempAddr + UVBufSize;
                if(VBufTempAddr%4)  //The address MUST be word-aligned for proper PXP operation. Maybe we can substitute using memory DMA for memcpy.
                {
                    RETAILMSG(1,(_T("Need memcpy VBuf\r\n")));
                    if(!m_pVBufAdjVirtAddr)
                        m_pVBufAdjVirtAddr = (UINT32 *) AllocPhysMem(UVBufSize, PAGE_EXECUTE_READWRITE, 0, 0, (ULONG *) &m_nVBufAdjPhysAddr);
            
                    if (!m_pVBufAdjVirtAddr)
                        return DDERR_UNSUPPORTED;
            
                    PBYTE pVBufOrgVirtAddr = m_pOverlaySurfaceOp->pBufVirtualAddr + YBufSize;
                    memcpy(m_pVBufAdjVirtAddr,pVBufOrgVirtAddr,UVBufSize);
                    VBufTempAddr = m_nVBufAdjPhysAddr;
                }
            
                if(UBufTempAddr%4)
                {
                    RETAILMSG(1,(_T("Need memcpy VBuf\r\n")));
                    if(!m_pUBufAdjVirtAddr)
                        m_pUBufAdjVirtAddr = (UINT32 *) AllocPhysMem(UVBufSize, PAGE_EXECUTE_READWRITE, 0, 0, (ULONG *) &m_nUBufAdjPhysAddr);
            
                    if (!m_pUBufAdjVirtAddr)
                        return DDERR_UNSUPPORTED;
            
                    PBYTE pUBufOrgVirtAddr = m_pOverlaySurfaceOp->pBufVirtualAddr + YBufSize + UVBufSize;
                    memcpy(m_pUBufAdjVirtAddr,pUBufOrgVirtAddr,UVBufSize);
                    UBufTempAddr = m_nUBufAdjPhysAddr;
                }
            
                pxpS0AddrGroup.iVorCrBufAddr = VBufTempAddr;
                pxpS0AddrGroup.iUorCbBufAddr = UBufTempAddr;
            }
            
                        
            pxpS0Property pxpS0Prop;
            memset(&pxpS0Prop, 0 , sizeof(pxpS0Prop));
            
            switch(m_pOverlaySurfaceOp->SrcPixelFormat)
            {
            case ddgpePixelFormat_8888:
                pxpS0Prop.eS0BufferFormat = pxpS0BufferFormat_RGB888;
                break;
            case ddgpePixelFormat_565:
                pxpS0Prop.eS0BufferFormat = pxpS0BufferFormat_RGB565;
                break;
            case ddgpePixelFormat_5550:
                pxpS0Prop.eS0BufferFormat = pxpS0BufferFormat_RGB555;
                break;
            case ddgpePixelFormat_YV12:
                pxpS0Prop.eS0BufferFormat = pxpS0BufferFormat_YUV420;
                pxpS0Prop.bYCbCrCsc = TRUE;
                //TBD
                break;
            default:
                RETAILMSG(1,(_T("PXP S0 buffer unsupported format!\r\n")));
                return DDERR_UNSUPPORTED;        
            }
            
            if(m_pOverlaySurfaceOp->OverlayPara.B.KEYSRC_EN)
                pxpS0Prop.iS0BKColor = RGBColorKeyLow;
            
            if((m_pOverlaySurfaceOp->SrcXOffset)||(m_pOverlaySurfaceOp->SrcYOffset)||(m_pOverlaySurfaceOp->SrcSurWidth != m_pOverlaySurfaceOp->DstRecWidth)||(m_pOverlaySurfaceOp->SrcSurHeight != m_pOverlaySurfaceOp->DstRecHeight))
            {
                pxpS0Prop.bCrop = TRUE;
                pxpS0Prop.rSOCropRect.left = m_pOverlaySurfaceOp->SrcXOffset;
                pxpS0Prop.rSOCropRect.top = m_pOverlaySurfaceOp->SrcYOffset;
                pxpS0Prop.rSOCropRect.right = m_pOverlaySurfaceOp->SrcXOffset + m_pOverlaySurfaceOp->DstRecWidth;
                pxpS0Prop.rSOCropRect.bottom = m_pOverlaySurfaceOp->SrcYOffset + m_pOverlaySurfaceOp->DstRecHeight;            
            }
                        
            if(((m_pOverlaySurfaceOp->SrcRecWidth != m_pOverlaySurfaceOp->DstRecWidth)||(m_pOverlaySurfaceOp->SrcRecHeight != m_pOverlaySurfaceOp->DstRecHeight))&& (m_pOverlaySurfaceOp->SrcPixelFormat == ddgpePixelFormat_YV12))
            {
                pxpS0Prop.bCrop = TRUE;
                pxpS0Prop.rSOCropRect.left = m_pOverlaySurfaceOp->SrcXOffset;
                pxpS0Prop.rSOCropRect.top = m_pOverlaySurfaceOp->SrcYOffset;
                pxpS0Prop.rSOCropRect.right = m_pOverlaySurfaceOp->SrcXOffset + m_pOverlaySurfaceOp->DstRecWidth;
                pxpS0Prop.rSOCropRect.bottom = m_pOverlaySurfaceOp->SrcYOffset + m_pOverlaySurfaceOp->DstRecHeight;
                pxpS0Prop.bScale = TRUE;
                pxpS0Prop.fXScale = (float)m_pOverlaySurfaceOp->DstRecWidth / (float)(m_pOverlaySurfaceOp->SrcRecWidth-2);          //Workaround For PXP resize
                pxpS0Prop.fYScale = (float)(m_pOverlaySurfaceOp->DstRecHeight-1) / (float)(m_pOverlaySurfaceOp->SrcRecHeight-1);    //Workaround For PXP resize
                pxpS0Prop.fXScaleOffset = 0;
                pxpS0Prop.fYScaleOffset = 0;            
            }
            
            pxpCoordinate pxpS0Coor;
            pxpS0Coor.iXBase = 0;
            pxpS0Coor.iYBase = 0;       

            pxpRectSize pxpS0RectSize;
            pxpS0RectSize.iWidth = m_pOverlaySurfaceOp->SrcSurWidth;
            pxpS0RectSize.iHeight = m_pOverlaySurfaceOp->SrcSurHeight;       

            pxpOverlayProperty pxpOverlayProp;
            memset(&pxpOverlayProp, 0 , sizeof(pxpOverlayProp));

            pxpOutProperty pxpOutProp;
            memset(&pxpOutProp, 0 , sizeof(pxpOutProp));
            pxpOutProp.iOutputAlpha = 0xFF;
            pxpOutProp.iOutputWidth =  m_pOverlaySurfaceOp->DstRecWidth & m_nAlignMask;
            pxpOutProp.iOutputHeight = m_pOverlaySurfaceOp->DstRecHeight & m_nAlignMask;

            switch(m_pOverlaySurfaceOp->DestPixelFormat)
            {
            case ddgpePixelFormat_8888:
                pxpOutProp.epxpOutputFmt = pxpOutputFormat_ARGB8888;
                break;
            case ddgpePixelFormat_5551:
                pxpOutProp.epxpOutputFmt = pxpOutputFormat_ARGB1555;
                break;
            case ddgpePixelFormat_565:
                pxpOutProp.epxpOutputFmt = pxpOutputFormat_RGB565;
                break;
            case ddgpePixelFormat_5550:
                pxpOutProp.epxpOutputFmt = pxpOutputFormat_RGB555;
                break;
            default:
                RETAILMSG(1,(_T("PXP output buffer unsupported format!\r\n")));                
                return DDERR_UNSUPPORTED;
            }
        
            switch(m_pOverlaySurfaceOp->iRotate)
            {
            case DMDO_90:
                pxpOutProp.epxpOutputRot = pxpOutputROT_270;
                break;
            case DMDO_180:
                pxpOutProp.epxpOutputRot = pxpOutputROT_180;   
                break;
            case DMDO_270:
                pxpOutProp.epxpOutputRot = pxpOutputROT_90;
                break;
            default:               
                return DDERR_UNSUPPORTED;
            }//Rotation direction for display screen is counterclockwise, but PXP rotation direction is clockwise, thus conversion is need. 
            if(m_pOverlaySurfaceOp->isUpsideDown)
                pxpOutProp.bVFlip = TRUE;
            if(m_pOverlaySurfaceOp->isLeftsideRight)
                pxpOutProp.bHFlip = TRUE;

            if((!m_pRotatedSurface)||(m_iOrgRotSurfaceWidth!=(m_pOverlaySurfaceOp->DstRecWidth & m_nAlignMask))||(m_iOrgRotSurfaceHeight!=(m_pOverlaySurfaceOp->DstRecHeight & m_nAlignMask)))
            {
                if(m_pRotatedSurface)
                    delete m_pRotatedSurface;

                if(FAILED(AllocSurface(&m_pRotatedSurface,
                    m_pOverlaySurfaceOp->DstRecWidthHw & m_nAlignMask,
                    m_pOverlaySurfaceOp->DstRecHeightHw & m_nAlignMask,
                    EDDGPEPixelFormatToEGPEFormat[m_pOverlaySurfaceOp->DestPixelFormat],
                    m_pOverlaySurfaceOp->DestPixelFormat,
                    GPE_REQUIRE_VIDEO_MEMORY))) {
                        RETAILMSG (1, (L"Couldn't allocate rotation surface\r\n"));
                        return  DDERR_OUTOFMEMORY;
                    }

                m_iOrgRotSurfaceWidth = m_pOverlaySurfaceOp->DstRecWidth & m_nAlignMask;
                m_iOrgRotSurfaceHeight = m_pOverlaySurfaceOp->DstRecHeight & m_nAlignMask;            
            }

            EnterCriticalSection(&m_PxpOperateCS);
            if (!m_hPXP)
            {
                m_hPXP = PXPOpenHandle();
                DEBUGMSG(1,(_T("m_hPXP = %0x\r\n"),m_hPXP));
                // Bail if no PXP handle
                if ((m_hPXP == NULL)||(m_hPXP == INVALID_HANDLE_VALUE))
                {
                    RETAILMSG(1,(_T("No Pixel Pipeline handle available! \r\n")));
                    LeaveCriticalSection(&m_PxpOperateCS);
                    return DDERR_UNSUPPORTED;
                }

                pxpParaConfig pxpPara;
                memset(&pxpPara, 0 , sizeof(pxpPara));
                pxpPara.iBlockSize = m_nOverlayAlign;                
                PXPConfigureGeneral(m_hPXP, &pxpPara);
                PXPInterruptEnable(m_hPXP);
            }


            PXPSetS0BufferColorKey(m_hPXP, &NoColorKey);
            PXPSetOverlayColorKey(m_hPXP, &NoColorKey);             
            PXPSetS0BufferAddrGroup(m_hPXP, &pxpS0AddrGroup);
            PXPSetS0BufProperty(m_hPXP, &pxpS0Prop);
            PXPSetS0BufferOffsetInOutput(m_hPXP, &pxpS0Coor);
            PXPSetS0BufferSize(m_hPXP, &pxpS0RectSize);
            PXPSetOverlayBufsProperty(m_hPXP, &pxpOverlayProp);
            PXPSetOutputProperty(m_hPXP, &pxpOutProp);            
            PXPSetOutputBuffer1Addr(m_hPXP, m_nLAWPhysical+m_pRotatedSurface->OffsetInVideoMemory());

            PXPStartProcess(m_hPXP, FALSE);   //Rotate overlay surface first

            pxpS0RectSize.iWidth = m_pOverlaySurfaceOp->DstRecWidthHw;
            pxpS0RectSize.iHeight = m_pOverlaySurfaceOp->DstRecHeightHw;
        
            memset(&pxpS0Prop, 0 , sizeof(pxpS0Prop));
            switch(m_pOverlaySurfaceOp->DestPixelFormat)
            {
            case ddgpePixelFormat_8888:
                pxpS0Prop.eS0BufferFormat = pxpS0BufferFormat_RGB888;
                break;
            case ddgpePixelFormat_565:
                pxpS0Prop.eS0BufferFormat = pxpS0BufferFormat_RGB565;
                break;
            case ddgpePixelFormat_5550:
                pxpS0Prop.eS0BufferFormat = pxpS0BufferFormat_RGB555;
                break;
            case ddgpePixelFormat_YV12:
                pxpS0Prop.eS0BufferFormat = pxpS0BufferFormat_YUV420;
                pxpS0Prop.bYCbCrCsc = TRUE;
                //TBD
                break;
            default:
                RETAILMSG(1,(_T("PXP S0 buffer unsupported format!\r\n")));
                LeaveCriticalSection(&m_PxpOperateCS);                
                return DDERR_UNSUPPORTED;        
            }

            if(m_pOverlaySurfaceOp->OverlayPara.B.KEYSRC_EN)
                pxpS0Prop.iS0BKColor = RGBColorKeyLow;
            
            pxpS0AddrGroup.iRGBorYBufAddr = m_nLAWPhysical+m_pRotatedSurface->OffsetInVideoMemory();

            pxpS0Coor.iXBase = m_pOverlaySurfaceOp->XOffset;
            pxpS0Coor.iYBase = m_pOverlaySurfaceOp->YOffset;
                                    
            memset(&pxpOverlayProp, 0 , sizeof(pxpOverlayProp));
            if(m_pOverlaySurfaceOp->OverlayPara.B.KEYSRC_EN)
            {
                pxpOverlayProp.bColorKey = FALSE;
                pxpOverlayProp.iAlphaOverlay = 0;
            }
            else if(m_pOverlaySurfaceOp->OverlayPara.B.KEYDEST_EN)
            {
                pxpOverlayProp.bColorKey = TRUE;
                pxpOverlayProp.iAlphaOverlay = 0xFF;
            }
            
            pxpOverlayProp.bEnableOverlay = TRUE;
            
            pxpOverlayProp.eAlphaCntl = pxpAlphaControl_OVERRIDE;
            switch(m_pOverlaySurfaceOp->DestPixelFormat)
            {
            case ddgpePixelFormat_8888:
                pxpOverlayProp.eOverlayFormat = pxpOverlayFormat_ARGB8888;
                break;
            case ddgpePixelFormat_5551:
                pxpOverlayProp.eOverlayFormat = pxpOverlayFormat_ARGB1555;
                break;
            case ddgpePixelFormat_565:
                pxpOverlayProp.eOverlayFormat = pxpOverlayFormat_RGB565;
                break;
            case ddgpePixelFormat_5550:
                pxpOverlayProp.eOverlayFormat = pxpOverlayFormat_RGB555;
                break;
            default:
                RETAILMSG(1,(_T("PXP overlay buffer unsupported format!\r\n")));
                LeaveCriticalSection(&m_PxpOperateCS);                
                return DDERR_UNSUPPORTED; 
            }
            
            pxpOverlayProp.iOverlayBufNum = 0;
           
            memset(&pxpOutProp, 0 , sizeof(pxpOutProp));
            pxpOutProp.iOutputAlpha = 0xFF;
            pxpOutProp.iOutputWidth =  (UINT16)m_nScreenWidthSave;
            pxpOutProp.iOutputHeight = (UINT16)m_nScreenHeightSave;
            
            switch(m_pOverlaySurfaceOp->DestPixelFormat)
            {
            case ddgpePixelFormat_8888:
                pxpOutProp.epxpOutputFmt = pxpOutputFormat_ARGB8888;
                break;
            case ddgpePixelFormat_5551:
                pxpOutProp.epxpOutputFmt = pxpOutputFormat_ARGB1555;
                break;
            case ddgpePixelFormat_565:
                pxpOutProp.epxpOutputFmt = pxpOutputFormat_RGB565;
                break;
            case ddgpePixelFormat_5550:
                pxpOutProp.epxpOutputFmt = pxpOutputFormat_RGB555;
                break;
            default:
                RETAILMSG(1,(_T("PXP output buffer unsupported format!\r\n")));
                LeaveCriticalSection(&m_PxpOperateCS);            
                return DDERR_UNSUPPORTED;
            }
                        
            pxpOverlayBuffersPos pxpOverlayPos;
            pxpOverlayPos.iOverlayBufNum = 0;
            pxpOverlayPos.rOverlayBufRect.left = 0;
            pxpOverlayPos.rOverlayBufRect.right = m_nScreenWidthSave;
            pxpOverlayPos.rOverlayBufRect.top = 0;
            pxpOverlayPos.rOverlayBufRect.bottom = m_nScreenHeightSave;
                        
            pxpOverlayBuffersAddr pxpOverlayAddr;
            pxpOverlayAddr.iOverlayBufNum = 0;
            pxpOverlayAddr.iOverlayBufAddress = (UINT32) m_nLAWPhysical+m_pBackgroundSurface->OffsetInVideoMemory();
             


            if(m_pOverlaySurfaceOp->OverlayPara.B.KEYSRC_EN)    //Combine overlay surface secondly
            {
                PXPSetS0BufferColorKey(m_hPXP, &SetColorKey);
            }
            else if(m_pOverlaySurfaceOp->OverlayPara.B.KEYDEST_EN)
            {
                PXPSetOverlayColorKey(m_hPXP, &SetColorKey);
            }

            PXPSetS0BufferOffsetInOutput(m_hPXP, &pxpS0Coor);            
            PXPSetS0BufProperty(m_hPXP, &pxpS0Prop);
            PXPSetS0BufferSize(m_hPXP, &pxpS0RectSize); 
            PXPSetS0BufferAddrGroup(m_hPXP, &pxpS0AddrGroup);
            PXPSetOverlayBufsProperty(m_hPXP, &pxpOverlayProp);
            PXPSetOutputProperty(m_hPXP, &pxpOutProp);
            PXPSetOverlayBuffersPos(m_hPXP, &pxpOverlayPos);
            PXPSetOverlayBuffersAddr(m_hPXP, &pxpOverlayAddr);
            PXPSetOutputBuffer1Addr(m_hPXP, m_nLAWPhysical+m_pCombinedSurface->OffsetInVideoMemory());
            
        }
        else    //Overlay surface doesn't need rotation
        {
            pxpS0BufferAddrGroup pxpS0AddrGroup;
            pxpS0AddrGroup.iRGBorYBufAddr = m_pOverlaySurfaceOp->nBufPhysicalAddr;
            
            if(ddgpePixelFormat_YV12 == m_pOverlaySurfaceOp->SrcPixelFormat)
            {
                UINT32 UBufTempAddr,VBufTempAddr;
                UINT32  YBufSize = m_pOverlaySurfaceOp->SrcSurWidth * m_pOverlaySurfaceOp->SrcSurHeight;
                VBufTempAddr = m_pOverlaySurfaceOp->nBufPhysicalAddr + YBufSize;
                UINT32  UVBufSize = m_pOverlaySurfaceOp->SrcSurWidth * m_pOverlaySurfaceOp->SrcSurHeight/4;
                UBufTempAddr = VBufTempAddr + UVBufSize;
                if(VBufTempAddr%4)  //The address MUST be word-aligned for proper PXP operation. Maybe we can substitute using memory DMA for memcpy.
                {
                    RETAILMSG(1,(_T("Need memcpy VBuf\r\n")));
                    if(!m_pVBufAdjVirtAddr)
                        m_pVBufAdjVirtAddr = (UINT32 *) AllocPhysMem(UVBufSize, PAGE_EXECUTE_READWRITE, 0, 0, (ULONG *) &m_nVBufAdjPhysAddr);
            
                    if (!m_pVBufAdjVirtAddr)
                        return DDERR_UNSUPPORTED;
            
                    PBYTE pVBufOrgVirtAddr = m_pOverlaySurfaceOp->pBufVirtualAddr + YBufSize;
                    memcpy(m_pVBufAdjVirtAddr,pVBufOrgVirtAddr,UVBufSize);
                    VBufTempAddr = m_nVBufAdjPhysAddr;
                }
            
                if(UBufTempAddr%4)
                {
                    RETAILMSG(1,(_T("Need memcpy VBuf\r\n")));
                    if(!m_pUBufAdjVirtAddr)
                        m_pUBufAdjVirtAddr = (UINT32 *) AllocPhysMem(UVBufSize, PAGE_EXECUTE_READWRITE, 0, 0, (ULONG *) &m_nUBufAdjPhysAddr);
            
                    if (!m_pUBufAdjVirtAddr)
                        return DDERR_UNSUPPORTED;
            
                    PBYTE pUBufOrgVirtAddr = m_pOverlaySurfaceOp->pBufVirtualAddr + YBufSize + UVBufSize;
                    memcpy(m_pUBufAdjVirtAddr,pUBufOrgVirtAddr,UVBufSize);
                    UBufTempAddr = m_nUBufAdjPhysAddr;
                }
            
                pxpS0AddrGroup.iVorCrBufAddr = VBufTempAddr;
                pxpS0AddrGroup.iUorCbBufAddr = UBufTempAddr;
            }
                                    
            pxpS0Property pxpS0Prop;
            memset(&pxpS0Prop, 0 , sizeof(pxpS0Prop));
            
            switch(m_pOverlaySurfaceOp->SrcPixelFormat)
            {
            case ddgpePixelFormat_8888:
                pxpS0Prop.eS0BufferFormat = pxpS0BufferFormat_RGB888;
                break;
            case ddgpePixelFormat_565:
                pxpS0Prop.eS0BufferFormat = pxpS0BufferFormat_RGB565;
                break;
            case ddgpePixelFormat_5550:
                pxpS0Prop.eS0BufferFormat = pxpS0BufferFormat_RGB555;
                break;
            case ddgpePixelFormat_YV12:
                pxpS0Prop.eS0BufferFormat = pxpS0BufferFormat_YUV420;
                pxpS0Prop.bYCbCrCsc = TRUE;
                //TBD
                break;
            default:
                RETAILMSG(1,(_T("PXP S0 buffer unsupported format!\r\n")));
                return DDERR_UNSUPPORTED;        
            }
            
            if(m_pOverlaySurfaceOp->OverlayPara.B.KEYSRC_EN)
                pxpS0Prop.iS0BKColor = RGBColorKeyLow;
            
            if((m_pOverlaySurfaceOp->SrcXOffset)||(m_pOverlaySurfaceOp->SrcYOffset)||(m_pOverlaySurfaceOp->SrcSurWidth != m_pOverlaySurfaceOp->DstRecWidth)||(m_pOverlaySurfaceOp->SrcSurHeight != m_pOverlaySurfaceOp->DstRecHeight))
            {
                pxpS0Prop.bCrop = TRUE;
                pxpS0Prop.rSOCropRect.left = m_pOverlaySurfaceOp->SrcXOffset;
                pxpS0Prop.rSOCropRect.top = m_pOverlaySurfaceOp->SrcYOffset;
                pxpS0Prop.rSOCropRect.right = m_pOverlaySurfaceOp->SrcXOffset + m_pOverlaySurfaceOp->DstRecWidth;
                pxpS0Prop.rSOCropRect.bottom = m_pOverlaySurfaceOp->SrcYOffset + m_pOverlaySurfaceOp->DstRecHeight;            
            }
            
            if(((m_pOverlaySurfaceOp->SrcRecWidth != m_pOverlaySurfaceOp->DstRecWidth)||(m_pOverlaySurfaceOp->SrcRecHeight != m_pOverlaySurfaceOp->DstRecHeight))&& (m_pOverlaySurfaceOp->SrcPixelFormat == ddgpePixelFormat_YV12))
            {
                pxpS0Prop.bCrop = TRUE;
                pxpS0Prop.rSOCropRect.left = m_pOverlaySurfaceOp->SrcXOffset;
                pxpS0Prop.rSOCropRect.top = m_pOverlaySurfaceOp->SrcYOffset;
                pxpS0Prop.rSOCropRect.right = m_pOverlaySurfaceOp->SrcXOffset + m_pOverlaySurfaceOp->DstRecWidth;
                pxpS0Prop.rSOCropRect.bottom = m_pOverlaySurfaceOp->SrcYOffset + m_pOverlaySurfaceOp->DstRecHeight;
                pxpS0Prop.bScale = TRUE;
                pxpS0Prop.fXScale = (float)m_pOverlaySurfaceOp->DstRecWidth / (float)(m_pOverlaySurfaceOp->SrcRecWidth-2);          //Workaround For PXP resize
                pxpS0Prop.fYScale = (float)(m_pOverlaySurfaceOp->DstRecHeight-1) / (float)(m_pOverlaySurfaceOp->SrcRecHeight-1);    //Workaround For PXP resize
                pxpS0Prop.fXScaleOffset = 0;
                pxpS0Prop.fYScaleOffset = 0;            
            }            
                    
            pxpRectSize pxpS0RectSize;
            pxpS0RectSize.iWidth = m_pOverlaySurfaceOp->SrcSurWidth;
            pxpS0RectSize.iHeight = m_pOverlaySurfaceOp->SrcSurHeight;
            
            pxpCoordinate pxpS0Coor;
            pxpS0Coor.iXBase = m_pOverlaySurfaceOp->XOffset;
            pxpS0Coor.iYBase = m_pOverlaySurfaceOp->YOffset;

            pxpOverlayProperty pxpOverlayProp;
            memset(&pxpOverlayProp, 0 , sizeof(pxpOverlayProp));
            if(m_pOverlaySurfaceOp->OverlayPara.B.KEYSRC_EN)
            {
                pxpOverlayProp.bColorKey = FALSE;
                pxpOverlayProp.iAlphaOverlay = 0;
            }
            else if(m_pOverlaySurfaceOp->OverlayPara.B.KEYDEST_EN)
            {
                pxpOverlayProp.bColorKey = TRUE;
                pxpOverlayProp.iAlphaOverlay = 0xFF;
            }

            pxpOverlayProp.bEnableOverlay = TRUE;

            pxpOverlayProp.eAlphaCntl = pxpAlphaControl_OVERRIDE;
            switch(m_pOverlaySurfaceOp->DestPixelFormat)
            {
            case ddgpePixelFormat_8888:
                pxpOverlayProp.eOverlayFormat = pxpOverlayFormat_ARGB8888;
                break;
            case ddgpePixelFormat_5551:
                pxpOverlayProp.eOverlayFormat = pxpOverlayFormat_ARGB1555;
                break;
            case ddgpePixelFormat_565:
                pxpOverlayProp.eOverlayFormat = pxpOverlayFormat_RGB565;
                break;
            case ddgpePixelFormat_5550:
                pxpOverlayProp.eOverlayFormat = pxpOverlayFormat_RGB555;
                break;
            default:
                RETAILMSG(1,(_T("PXP overlay buffer unsupported format!\r\n")));
                return DDERR_UNSUPPORTED; 
            }

            pxpOverlayProp.iOverlayBufNum = 0;

            pxpOutProperty pxpOutProp;
            memset(&pxpOutProp, 0 , sizeof(pxpOutProp));
            pxpOutProp.iOutputAlpha = 0xFF;
            pxpOutProp.iOutputWidth =  (UINT16)m_nScreenWidthSave;
            pxpOutProp.iOutputHeight = (UINT16)m_nScreenHeightSave;

            switch(m_pOverlaySurfaceOp->DestPixelFormat)
            {
            case ddgpePixelFormat_8888:
                pxpOutProp.epxpOutputFmt = pxpOutputFormat_ARGB8888;
                break;
            case ddgpePixelFormat_5551:
                pxpOutProp.epxpOutputFmt = pxpOutputFormat_ARGB1555;
                break;
            case ddgpePixelFormat_565:
                pxpOutProp.epxpOutputFmt = pxpOutputFormat_RGB565;
                break;
            case ddgpePixelFormat_5550:
                pxpOutProp.epxpOutputFmt = pxpOutputFormat_RGB555;
                break;
            default:
                RETAILMSG(1,(_T("PXP output buffer unsupported format!\r\n")));            
                return DDERR_UNSUPPORTED;
            }

            pxpOverlayBuffersPos pxpOverlayPos;
            pxpOverlayPos.iOverlayBufNum = 0;
            pxpOverlayPos.rOverlayBufRect.left = 0;
            pxpOverlayPos.rOverlayBufRect.right = m_nScreenWidthSave;
            pxpOverlayPos.rOverlayBufRect.top = 0;
            pxpOverlayPos.rOverlayBufRect.bottom = m_nScreenHeightSave;

            pxpOverlayBuffersAddr pxpOverlayAddr;
            pxpOverlayAddr.iOverlayBufNum = 0;
            pxpOverlayAddr.iOverlayBufAddress = (UINT32) m_nLAWPhysical+m_pBackgroundSurface->OffsetInVideoMemory();

            if(m_pOverlaySurfaceOp->OverlayPara.B.KEYDEST_EN)
                pxpOverlayColorKey = SetColorKey;
            if(m_pOverlaySurfaceOp->OverlayPara.B.KEYSRC_EN)
                pxpS0ColorKey = SetColorKey;

            EnterCriticalSection(&m_PxpOperateCS);
            if (!m_hPXP)
            {
                m_hPXP = PXPOpenHandle();
                DEBUGMSG(1,(_T("m_hPXP = %0x\r\n"),m_hPXP));
                // Bail if no PXP handle
                if ((m_hPXP == NULL)||(m_hPXP == INVALID_HANDLE_VALUE))
                {
                    RETAILMSG(1,(_T("No Pixel Pipeline handle available! \r\n")));
                    LeaveCriticalSection(&m_PxpOperateCS);
                    return DDERR_UNSUPPORTED;
                }

                pxpParaConfig pxpPara;
                memset(&pxpPara, 0 , sizeof(pxpPara));
                pxpPara.iBlockSize = m_nOverlayAlign;                
                PXPConfigureGeneral(m_hPXP, &pxpPara);
                PXPInterruptEnable(m_hPXP);
            }


            PXPSetS0BufferAddrGroup(m_hPXP, &pxpS0AddrGroup);
            PXPSetS0BufProperty(m_hPXP, &pxpS0Prop);
            PXPSetS0BufferSize(m_hPXP, &pxpS0RectSize);        
            PXPSetOverlayBuffersAddr(m_hPXP, &pxpOverlayAddr);
            PXPSetS0BufferOffsetInOutput(m_hPXP, &pxpS0Coor);
            PXPSetOutputProperty(m_hPXP, &pxpOutProp);
            PXPSetOverlayBuffersPos(m_hPXP, &pxpOverlayPos);
            PXPSetS0BufferColorKey(m_hPXP, &pxpS0ColorKey);
            PXPSetOverlayColorKey(m_hPXP, &pxpOverlayColorKey);
            PXPSetS0BufferColorKey(m_hPXP, &pxpS0ColorKey);
            PXPSetOverlayColorKey(m_hPXP, &pxpOverlayColorKey);            
            PXPSetOverlayBufsProperty(m_hPXP, &pxpOverlayProp);
            PXPSetOutputBuffer1Addr(m_hPXP, m_nLAWPhysical+m_pCombinedSurface->OffsetInVideoMemory());
        
        }        
    }

    if(m_bManualCombine)
    {
        PXPStartProcess(m_hPXP, FALSE);
    }
    else
    { 
        PXPStartProcess(m_hPXP, TRUE);
        PXPWaitForCompletion(m_hPXP);   //We have to wait till a frame is updated on CombineSurface.
        m_bManualCombine = TRUE;
        SetVisibleSurface((GPESurf *) m_pCombinedSurface);
    }


    SetEvent(m_hCombSurfUpdatedEvent);

    LeaveCriticalSection(&m_PxpOperateCS);

    result = DD_OK;


    DEBUGMSG(1, (TEXT("DDLcdif EnableOverlay: result 0x%08x\r\n"), result));

    return result;
}


//------------------------------------------------------------------------------
//
// Function: DisableOverlay
//
//  DESCRIPTION:   This function disable the graphic window of LCDIF of 
//                 for overlay window.
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
HRESULT DDLcdif::DisableOverlay(VOID)
{

    SetVisibleSurface((GPESurf *) m_pBackgroundSurface);

    memset(m_pOverlaySurfaceOp, 0x00, sizeof(overlaySurf_t));


    if(m_pCombinedSurface)
    {
        Sleep(17);  //refresh rate is nearly 60Hz
        delete m_pCombinedSurface;
        m_pCombinedSurface = NULL;
    }
    
    EnterCriticalSection(&m_PxpOperateCS);
    m_bManualCombine = FALSE;

    if ((NULL != m_hPXP)&&(INVALID_HANDLE_VALUE != m_hPXP))
    {
        PXPResetDriverStatus(m_hPXP);   //In case PXP module runs out of order.
        PXPCloseHandle(m_hPXP);
        m_hPXP = NULL;
    }

    LeaveCriticalSection(&m_PxpOperateCS);

    if(m_pVBufAdjVirtAddr)
    {
        FreePhysMem( m_pVBufAdjVirtAddr );
        m_pVBufAdjVirtAddr = NULL;
    }

    if(m_pUBufAdjVirtAddr)
    {
        FreePhysMem( m_pUBufAdjVirtAddr );
        m_pUBufAdjVirtAddr = NULL;
    }

    if(m_pRotatedSurface)
    {
        delete m_pRotatedSurface;
        m_pRotatedSurface = NULL;
    }

    DEBUGMSG(1, (TEXT("DDLcdif DisableOverlay:\r\n")));    
    return DD_OK;
}


//------------------------------------------------------------------------------
//
//  FUNCTION:     WaitForNotBusyOverlay
//
//  DESCRIPTION:  This function is to wait until the graphic window of 
//                the LCDIF available to update.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
VOID DDLcdif::WaitForNotBusyOverlay(VOID)
{
    // TODO: We can't use the interrupt for graphic window because it
    //       will mess up the syncronyzation of the main frame buffer.

    return;
}

//------------------------------------------------------------------------------
//
// Function: SetColorKey
//
// This function sets the surface's colorkey. 
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
DWORD DDLcdif::SetColorKey(LPDDHAL_SETCOLORKEYDATA pd)
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

        DEBUGMSG(1,(_T("pd->ckNew.dwColorSpaceLowValue = 0x%0x,pd->ckNew.dwColorSpaceHighValue = 0x%0x\r\n"),pd->ckNew.dwColorSpaceLowValue,pd->ckNew.dwColorSpaceHighValue));

        if(pd->dwFlags &DDCKEY_DESTOVERLAY)
        {
            pxpColorKey pxpOverlayColorKey;
            pxpOverlayColorKey.iColorKeyHigh = 0xFFFFFF;
            pxpOverlayColorKey.iColorKeyLow = 0;
            PXPSetOverlayColorKey(m_hPXP, &pxpOverlayColorKey);
        }

    }
    else
    {
        pd->ddRVal = DDERR_INVALIDOBJECT;
    }

    return DDHAL_DRIVER_HANDLED;

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
    if ((pSurf1->DstRecWidth == pSurf2->DstRecWidth) &&
        (pSurf1->DstRecHeight == pSurf2->DstRecHeight) &&
        (pSurf1->DstRecWidthHw == pSurf2->DstRecWidthHw) &&
        (pSurf1->DstRecHeightHw == pSurf2->DstRecHeightHw) &&
        (pSurf1->XOffset == pSurf2->XOffset) &&
        (pSurf1->YOffset == pSurf2->YOffset) &&
        (pSurf1->SrcXOffset == pSurf2->SrcXOffset) &&
        (pSurf1->SrcYOffset == pSurf2->SrcYOffset) &&
        (pSurf1->SrcXOffset == pSurf2->SrcRecWidth) &&
        (pSurf1->SrcYOffset == pSurf2->SrcRecHeight) &&
        (pSurf1->SrcSurWidth == pSurf2->SrcSurWidth) &&
        (pSurf1->SrcSurHeight == pSurf2->SrcSurHeight) &&
        (pSurf1->Transparency == pSurf2->Transparency) &&
        (pSurf1->isUpsideDown == pSurf2->isUpsideDown) &&
        (pSurf1->isLeftsideRight == pSurf2->isLeftsideRight) &&
        (pSurf1->SrcColorKeyLow == pSurf2->SrcColorKeyLow) &&
        (pSurf1->SrcColorKeyHigh == pSurf2->SrcColorKeyHigh) &&
        (pSurf1->DestColorKeyLow == pSurf2->DestColorKeyLow) &&
        (pSurf1->DestColorKeyHigh == pSurf2->DestColorKeyHigh) &&
        (pSurf1->OverlayPara.U == pSurf2->OverlayPara.U) &&
        (pSurf1->SrcPixelFormat == pSurf2->SrcPixelFormat) &&
        (pSurf1->DestPixelFormat == pSurf2->DestPixelFormat))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

