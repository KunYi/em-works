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
//  Copyright (C) 2005-2006, 2007, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//---------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
// File:        DDLcdcOverlay.cpp
//
//  Implementation of Overlay-related functions for DDLCDC class, the
//  DirectDraw display driver.
//
//-----------------------------------------------------------------------------

#include "precomp.h"


//-----------------------------------------------------------------------------
// External Functions


//-----------------------------------------------------------------------------
// External Variables

//-----------------------------------------------------------------------------
// Local Functions
BOOL IsOverlaySurfEqual(pOverlaySurf_t pSurf1, pOverlaySurf_t pSurf2);

//-----------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------
// Convert Microsoft color key mask into the one of the LCDC of i.MX27 for graphic window
// Microsoft : r bit 0~7; g bit 8~15; b bit 16~23
// i.MX27:     r bit 12~17; g bit 6~11, b bit 0~5
#define CONFIGURE_COLORKEY(colorKey32)   colorKey32 = ( \
        ((colorKey32 << 12) & bmLCDC_GWCR_GWCKR) |      \
        ((colorKey32 >>  2) & bmLCDC_GWCR_GWCKG) |      \
        ((colorKey32 >> 16) & bmLCDC_GWCR_GWCKB)        \
        )

#if (UNDER_CE >= 600)
#define IsOverlayMirrorUpDown(pd) (pd->dwFlags & DDOVER_MIRRORUPDOWN)
#else
#define IsOverlayMirrorUpDown(pd) ((pd->dwFlags & DDOVER_DDFX) && (pd->overlayFX.dwDDFX & DDOVERFX_MIRRORUPDOWN))
#endif


#define DUMP_OVERLAY_OP()   {\
    DEBUGMSG(1, (TEXT("Overlay OP: Overlay->isGraphicWindowRunning %s\r\n"), m_pOverlaySurfaceOp->isGraphicWindowRunning ? L"TRUE" : L"FALSE"));    \
    DEBUGMSG(1, (TEXT("Overlay OP: Overlay->nBufPhysicalAddr 0x%08x\r\n"), m_pOverlaySurfaceOp->nBufPhysicalAddr));                                 \
    DEBUGMSG(1, (TEXT("Overlay OP: Overlay->Width            %d\r\n"), m_pOverlaySurfaceOp->Width));                                                \
    DEBUGMSG(1, (TEXT("Overlay OP: Overlay->Height           %d\r\n"), m_pOverlaySurfaceOp->Height));                                               \
    DEBUGMSG(1, (TEXT("Overlay OP: Overlay->WidthHw          %d\r\n"), m_pOverlaySurfaceOp->WidthHw));                                              \
    DEBUGMSG(1, (TEXT("Overlay OP: Overlay->HeightHw         %d\r\n"), m_pOverlaySurfaceOp->HeightHw));                                             \
    DEBUGMSG(1, (TEXT("Overlay OP: Overlay->LineStride       %d\r\n"), m_pOverlaySurfaceOp->LineStride));                                           \
    DEBUGMSG(1, (TEXT("Overlay OP: Overlay->XOffset          %d\r\n"), m_pOverlaySurfaceOp->XOffset));                                              \
    DEBUGMSG(1, (TEXT("Overlay OP: Overlay->YOffset          %d\r\n"), m_pOverlaySurfaceOp->YOffset));                                              \
    DEBUGMSG(1, (TEXT("Overlay OP: Overlay->Transparency     %d\r\n"), m_pOverlaySurfaceOp->Transparency));                                         \
    DEBUGMSG(1, (TEXT("Overlay OP: Overlay->isUpsideDown     %s\r\n"), m_pOverlaySurfaceOp->isUpsideDown ? L"TRUE" : L"FALSE"));                    \
    DEBUGMSG(1, (TEXT("Overlay OP: Overlay->ColorKeyMask     0x%08x\r\n"), m_pOverlaySurfaceOp->ColorKeyMask));                                     \
}

#define SETUP_OVERLAY_POSITION(X, Y) switch(m_iRotate){ \
        case DMDO_0:                                                                                        \
            m_pOverlaySurfaceOp->XOffset = (UINT16)X;                                                       \
            m_pOverlaySurfaceOp->YOffset = (UINT16)Y;                                                       \
            break;                                                                                          \
        case DMDO_90:                                                                                       \
            m_pOverlaySurfaceOp->XOffset = (UINT16)Y;                                                       \
            m_pOverlaySurfaceOp->YOffset = (UINT16)(m_pMode->width - (X + m_pOverlaySurfaceOp->Width));     \
            break;                                                                                          \
        case DMDO_180:                                                                                      \
            m_pOverlaySurfaceOp->XOffset = (UINT16)(m_pMode->width - (X + m_pOverlaySurfaceOp->Width));     \
            m_pOverlaySurfaceOp->YOffset = (UINT16)(m_pMode->height - (Y + m_pOverlaySurfaceOp->Height));   \
            break;                                                                                          \
        case DMDO_270:                                                                                      \
            m_pOverlaySurfaceOp->XOffset = (UINT16)(m_pMode->height - (Y + m_pOverlaySurfaceOp->Height));   \
            m_pOverlaySurfaceOp->YOffset = (UINT16)X;                                                       \
            break;                                                                                          \
        default:                                                                                            \
            break;                                                                                          \
}


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
DWORD MX27DDLcdc::UpdateOverlay(LPDDHAL_UPDATEOVERLAYDATA pd)
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

    if (pd->dwFlags & DDOVER_HIDE)
    {
        DEBUGMSG(GPE_ZONE_HW, (TEXT("MX27DDLcdc::OVERLAY HIDE REQUEST\r\n")));
        if (pSrcSurf == m_pVisibleOverlay)
        {
            // hide the overlay
            //WaitForVSync();
            DisableOverlay();
            // reset visible overlay
            m_pVisibleOverlay = NULL;
            DEBUGMSG(GPE_ZONE_HW, (TEXT("MX27DDLcdc::OVERLAY HIDE!\r\n")));
        }
        else
        {
            // the overlay is not currently visible
            // nothing we need to do here
        }

        pd->ddRVal = DD_OK;
        goto _done;
    }

    DDGPESurf* pDstSurf = DDGPESurf::GetDDGPESurf(pd->lpDDDestSurface);

    if (pSrcSurf != m_pVisibleOverlay)
    {
        if (pd->dwFlags & DDOVER_SHOW)
        {
            DEBUGMSG(GPE_ZONE_HW, (TEXT("MX27DDLcdc::OVERLAY SHOW REQUEST\r\n")));
            if (m_pVisibleOverlay != NULL)
            {
                // some other overlay is already visible
                DEBUGMSG(GPE_ZONE_ERROR,
                          (TEXT("Error: Other overlay already visible!\r\n")));
                pd->ddRVal = DDERR_OUTOFCAPS;
                goto _done;
            }
            else
            {
                // we are going to make the overlay visible
                // so mark it as such:
                m_pVisibleOverlay = pSrcSurf;
                DEBUGMSG(GPE_ZONE_HW, (TEXT("MX27DDLcdc::OVERLAY SHOW\r\n")));
            }
        }
        else // DDOVER_SHOW not requested
        {
            // the overlay isn't visible, and we haven't been
            // asked to make it visible, so nothing we need to do
            pd->ddRVal = DD_OK;
            goto _done;
        }
    }

    overlaySurfInfo.Width   = (UINT16)(pd->rDest.right - pd->rDest.left);
    overlaySurfInfo.Height  = (UINT16)(pd->rDest.bottom - pd->rDest.top);

    switch(m_iRotate)
    {
        case DMDO_0:
        case DMDO_180:
            overlaySurfInfo.WidthHw = overlaySurfInfo.Width;
            overlaySurfInfo.HeightHw = overlaySurfInfo.Height;
            break;

        case DMDO_90:
        case DMDO_270:
            overlaySurfInfo.WidthHw = overlaySurfInfo.Height;
            overlaySurfInfo.HeightHw = overlaySurfInfo.Width;
            break;
        default:
            break;
    }

    // Parameter validity check
    if(
        (pd->rDest.right  > m_pMode->width)  ||
        (pd->rDest.bottom > m_pMode->height) ||
        (overlaySurfInfo.WidthHw & 0xF)   ||
        (overlaySurfInfo.WidthHw < LCDC_MIN_GRAPHICWINDOW_WIDTH)
        )
    {
        DEBUGMSG(GPE_ZONE_ERROR, (TEXT("MX27DDLcdc UpdateOverlay: wrong parameters!\r\n")));
        DUMP_OVERLAY_OP();
        pd->ddRVal = DDERR_INVALIDPARAMS;
        goto _done;
    }

    overlaySurfInfo.LineStride = overlaySurfInfo.WidthHw * m_pMode->Bpp / 8;

    SETUP_OVERLAY_POSITION(pd->rDest.left, pd->rDest.top);

    overlaySurfInfo.nBufPhysicalAddr = m_nLAWPhysical + pSrcSurf->OffsetInVideoMemory();

    // Setup if overlay is flipped as upside-down
    if(IsOverlayMirrorUpDown(pd))
    {
        DEBUGMSG(GPE_ZONE_HW, (TEXT("MX27DDLcdc UpdateOverlay: upside-down overlay is requested!\r\n")));
        overlaySurfInfo.isUpsideDown = TRUE;
    }
    else
    {
        overlaySurfInfo.isUpsideDown = FALSE;
    }

    // Setup color key.
    // LCDC only supports one color key value and does not
    // support a range.  So, we take the low value
    // from the color key mask to use as the color key.

    // Initialize color key mask to 0xFFFFFFFF.  If no color keying
    // is specified, this 0xFFFFFFFF will indicate no color key.
    overlaySurfInfo.ColorKeyMask = 0xFFFFFFFF;

    if(pd->dwFlags & DDOVER_KEYSRC)
    {
        overlaySurfInfo.ColorKeyMask = pd->lpDDSrcSurface->ddckCKSrcOverlay.dwColorSpaceLowValue;
    }
    else if(pd->dwFlags & DDOVER_KEYSRCOVERRIDE)
    {
        overlaySurfInfo.ColorKeyMask = pd->overlayFX.dckSrcColorkey.dwColorSpaceLowValue;
    }

    if(pd->dwFlags & DDOVER_KEYDEST)
    {
        DEBUGMSG(1,
          (TEXT("Error: can't handle dest color keying!\r\n")));
        pd->ddRVal = DDERR_INVALIDPARAMS;
        return (DDHAL_DRIVER_HANDLED);

        // overlaySurfInfo.ColorKeyMask = pd->lpDDDestSurface->ddckCKDestOverlay.dwColorSpaceLowValue;

    }
    else if(pd->dwFlags & DDOVER_KEYDESTOVERRIDE)
    {
        DEBUGMSG(1,
          (TEXT("Error: can't handle dest color keying!\r\n")));
        pd->ddRVal = DDERR_INVALIDPARAMS;
        return (DDHAL_DRIVER_HANDLED);

        // overlaySurfInfo.ColorKeyMask = pd->overlayFX.dckDestColorkey.dwColorSpaceLowValue;
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
        overlaySurfInfo.Transparency = (UINT16)(pd->lpDDSrcSurface->ddOverlayFX.dwAlphaConst);
    }
    if(pd->dwFlags & DDOVER_ALPHASRCNEG)
    {
        overlaySurfInfo.Transparency = 0xff-overlaySurfInfo.Transparency;
    }
    if(pd->dwFlags & DDOVER_ALPHADEST)
    {
        DEBUGMSG(1,
          (TEXT("Error: can't handle dest alpha!\r\n")));
        pd->ddRVal = DDERR_INVALIDPARAMS;
        return (DDHAL_DRIVER_HANDLED);
    }
    if(pd->dwFlags & DDOVER_ALPHADESTNEG)
    {
        DEBUGMSG(1,
          (TEXT("Error: can't handle dest alpha!\r\n")));
        pd->ddRVal = DDERR_INVALIDPARAMS;
        return (DDHAL_DRIVER_HANDLED);
    }

    // If overlay surface data has not changed, skip
    // step to set up overlay and return.
    if (!IsOverlaySurfEqual(m_pOverlaySurfaceOp, &overlaySurfInfo))
    {
        overlaySurfInfo.XOffset = m_pOverlaySurfaceOp->XOffset;
        overlaySurfInfo.YOffset = m_pOverlaySurfaceOp->YOffset;
        overlaySurfInfo.isGraphicWindowRunning = m_pOverlaySurfaceOp->isGraphicWindowRunning;
        memcpy(m_pOverlaySurfaceOp, &overlaySurfInfo, sizeof(overlaySurf_t));

        // Update the graphic window of LCDC of i.MX27 processor
        pd->ddRVal = EnableOverlay();
    }
    else
    {
        pd->ddRVal = DD_OK;
    }

_done:
    DEBUGMSG(GPE_ZONE_HW, (TEXT("MX27DDLcdc UpdateOverlay: %s!\r\n"), (pd->ddRVal == DD_OK) ? L"successful" : L"failed"));
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
DWORD MX27DDLcdc::SetOverlayPosition(LPDDHAL_SETOVERLAYPOSITIONDATA pd)
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

    if(!m_pOverlaySurfaceOp->isGraphicWindowRunning)
    {
        // The graphic window is not running
        pd->ddRVal = DD_OK;
        return DDHAL_DRIVER_HANDLED;
    }

    DDGPESurf* pSrcSurf = DDGPESurf::GetDDGPESurf(pd->lpDDSrcSurface);

    if (pSrcSurf != m_pVisibleOverlay)
    {
        pd->ddRVal = DD_OK;
        return DDHAL_DRIVER_HANDLED;
    }

    // Parameter validity check
    if(
        (pd->lXPos + m_pOverlaySurfaceOp->Width > m_pMode->width) ||
        (pd->lYPos + m_pOverlaySurfaceOp->Height > m_pMode->height)                 
        )
    {
        DEBUGMSG(GPE_ZONE_ERROR, (TEXT("MX27DDLcdc SetOverlayPosition: error pd->lXPos [%d], pd->lYPos[%d]\r\n"), pd->lXPos, pd->lYPos));
        DUMP_OVERLAY_OP();
        pd->ddRVal = DDERR_INVALIDPARAMS;
        return DDHAL_DRIVER_HANDLED;
    }

    SETUP_OVERLAY_POSITION(pd->lXPos, pd->lYPos);

    WaitForNotBusyOverlay();

    m_pLcdcReg->GWPR = \
            CSP_BITFVAL(LCDC_GWPR_GWXP, m_pOverlaySurfaceOp->XOffset) |
            CSP_BITFVAL(LCDC_GWPR_GWYP, m_pOverlaySurfaceOp->YOffset) ;
    
    pd->ddRVal = DD_OK;
    return DDHAL_DRIVER_HANDLED;
}


//------------------------------------------------------------------------------
//
// Function: SetVisibleSurfaceOverlay
//
// This function changes the overlay surface displayed by the LCDC.
//
// Parameters:
//      pSurf
//          [in] Surface to be set as the visible overlay.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
VOID MX27DDLcdc::SetVisibleSurfaceOverlay(DDGPESurf * pSurf)
{
    if(!m_pOverlaySurfaceOp->isGraphicWindowRunning)
    {
        return;
    }

    if(m_pOverlaySurfaceOp->isUpsideDown)
    {
        m_pOverlaySurfaceOp->nBufPhysicalAddr = m_nLAWPhysical + pSurf->OffsetInVideoMemory() + m_pOverlaySurfaceOp->LineStride * (m_pOverlaySurfaceOp->HeightHw - 1);
    }
    else
    {
        m_pOverlaySurfaceOp->nBufPhysicalAddr = m_nLAWPhysical + pSurf->OffsetInVideoMemory();
    }

    WaitForNotBusyOverlay();

    m_pLcdcReg->GWSAR = m_pOverlaySurfaceOp->nBufPhysicalAddr;

    return;
}


//------------------------------------------------------------------------------
//
// Function: EnableOverlay
//
//  DESCRIPTION:   This function enable the graphic window of LCDC of 
//                 i.MX27 processor for overlay window.
//
// Parameters:
//      None.
//
// Returns:
//      DD_OK                successful
//      others               failed
//
//------------------------------------------------------------------------------
HRESULT MX27DDLcdc::EnableOverlay(VOID)
{
    UINT32  tempGWCR = CSP_BITFMASK(LCDC_GWCR_GWE);
    HRESULT result = DDERR_OUTOFCAPS;
    UINT32  RGBColorKey;


    if(m_pOverlaySurfaceOp->isGraphicWindowRunning)
    {
        WaitForNotBusyOverlay();
    }

    INSREG32BF(&m_pLcdcReg->GWSR, LCDC_GWSR_GWH, m_pOverlaySurfaceOp->HeightHw);
    INSREG32BF(&m_pLcdcReg->GWSR, LCDC_GWSR_GWW, LCDC_GW_SIZE_X(m_pOverlaySurfaceOp->WidthHw));

    INSREG32BF(&m_pLcdcReg->GWVPWR, LCDC_GWVPWR_GWVPW, LCDC_GW_SIZE_STRIDE(m_pOverlaySurfaceOp->LineStride));

    INSREG32BF(&m_pLcdcReg->GWPR, LCDC_GWPR_GWXP, m_pOverlaySurfaceOp->XOffset);
    INSREG32BF(&m_pLcdcReg->GWPR, LCDC_GWPR_GWYP, m_pOverlaySurfaceOp->YOffset);
    
    CSP_BITFINS(tempGWCR, LCDC_GWCR_GWAV, LCDC_GW_TRANSPARENCY(m_pOverlaySurfaceOp->Transparency));

    if(m_pOverlaySurfaceOp->ColorKeyMask != 0xFFFFFFFF)
    {
        // Convert Microsoft color key mask into the one of the IPU
        // for graphic window.
        // Microsoft    : r bit 0~4; g bit 5~10; b bit 11~15        565
        // GW           : r bit 12~17; g bit 6~11, b bit 0~5        666

        switch (m_pMode->Bpp)
        {
            case 16:
                RGBColorKey =
                    ((((m_pOverlaySurfaceOp->ColorKeyMask & 0x0000FF) >> 3) << 12) | 
                    (((m_pOverlaySurfaceOp->ColorKeyMask & 0x00FF00) >> (8+2)) << 6) | 
                    ((m_pOverlaySurfaceOp->ColorKeyMask & 0xFF0000) >> (16+3))); 
                break;
            default:
                RGBColorKey = 0;
                break;
        }
        tempGWCR |= ((RGBColorKey) | CSP_BITFMASK(LCDC_GWCR_GWCKE));
    }
        
    if(m_pOverlaySurfaceOp->isUpsideDown)
    {
        m_pLcdcReg->GWSAR = (UINT32)m_pOverlaySurfaceOp->nBufPhysicalAddr + m_pOverlaySurfaceOp->LineStride * (m_pOverlaySurfaceOp->HeightHw - 1);
        CSP_BITFINS(tempGWCR, LCDC_GWCR_GW_RVS, 1);
    }
    else
    {
        m_pLcdcReg->GWSAR = (UINT32)m_pOverlaySurfaceOp->nBufPhysicalAddr;
    }

    if(m_nMask != 0) // for flicker
    {
        m_pOverlaySurfaceOp->isGraphicWindowRunning = TRUE;
        m_pLcdcReg->GWCR = tempGWCR;
    }
    else
    {
        //DWORD dwTransferred;
        BOOL initState;

        m_nMask = 1;
        m_pOverlaySurfaceOp->isGraphicWindowRunning = TRUE;

        // Disable LCDC hw clocks
        initState = DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX_LCDC, DDK_CLOCK_GATE_MODE_DISABLE);
        if(!initState)
        {
            DEBUGMSG(GPE_ZONE_ERROR, (TEXT("MX27DDLcdc SetModeHardware: can not shutdown LCDC clock, error[%d]!\r\n"), GetLastError()));        
            goto _done;
        }
    
        m_pLcdcReg->GWCR = tempGWCR;
        
        // Re-enable LCDC hw clocks
        initState = DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX_LCDC, DDK_CLOCK_GATE_MODE_ENABLE);
        if(initState != TRUE)
        {
            DEBUGMSG(GPE_ZONE_ERROR, (TEXT("MX27DDLcdc EnableOverlay: LCDC hw clock enable failed!\r\n")));
            goto _done;
        }
    }
    
    result = DD_OK;

_done:

    DEBUGMSG(HAL_ZONE_INIT, (TEXT("MX27DDLcdc EnableOverlay: result 0x%08x\r\n"), result));
    
    return result;
}


//------------------------------------------------------------------------------
//
// Function: DisableOverlay
//
//  DESCRIPTION:   This function disable the graphic window of LCDC of 
//                 i.MX27 processor for overlay window.
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
HRESULT MX27DDLcdc::DisableOverlay(VOID)
{
    if(m_pOverlaySurfaceOp->isGraphicWindowRunning)
    {
        // we should set the alpha to 0, and reduce 
        // the GW size first, then set GWE to 0
        INSREG32BF(&m_pLcdcReg->GWCR, LCDC_GWCR_GWAV, 0);
        // INSREG32BF(&m_pLcdcReg->GWSR, LCDC_GWSR_GWH, 16);
        // INSREG32BF(&m_pLcdcReg->GWSR, LCDC_GWSR_GWW, 16);
        Sleep(10);
        INSREG32BF(&m_pLcdcReg->GWCR, LCDC_GWCR_GWE, 0);
        //ClearGraphicWindowRegs(m_pLcdcReg);
        memset(m_pOverlaySurfaceOp, 0x00, sizeof(overlaySurf_t));
    }
    
    return DD_OK;
}


//------------------------------------------------------------------------------
//
// These three function is for furture expansion (alpha blending and 
// more efficient invert overlay)
//
//------------------------------------------------------------------------------
#if 0
//------------------------------------------------------------------------------
//
//  FUNCTION:      InverOverlay
//
//  DESCRIPTION:   This function is to invert the Graphic Window to the 
//                 opposite of the current one.
//
//  PARAMETERS:     
//
//  RETURNS:        
//                 DD_OK             successful
//                 others            failed
//
//------------------------------------------------------------------------------
HRESULT MX27DDLcdc::InverOverlay(VOID)
{
    if(m_pOverlaySurfaceOp->isUpsideDown)
    {
        INSREG32BF(&m_pLcdcReg->GWCR, LCDC_GWCR_GW_RVS, 0);
        INSREG32BF(&m_pLcdcReg->GWSAR, LCDC_GWSAR_GWSA, \
                (UINT32)m_pOverlaySurfaceOp->nBufPhysicalAddr);
        m_pOverlaySurfaceOp->isUpsideDown = FALSE;
    }
    else
    {
        INSREG32BF(&m_pLcdcReg->GWCR, LCDC_GWCR_GW_RVS, 1);
        INSREG32BF(&m_pLcdcReg->GWSAR, LCDC_GWSAR_GWSA, \
                (UINT32)m_pOverlaySurfaceOp->nBufPhysicalAddr + m_pOverlaySurfaceOp->LineStride * (m_pOverlaySurfaceOp->HeightHw - 1));
        m_pOverlaySurfaceOp->isUpsideDown = TRUE;
    }

    return DD_OK;
}


//------------------------------------------------------------------------------
//
//  FUNCTION:     GetOverlayAlphaValue
//
//  DESCRIPTION:  This function is to get current transparency of the 
//                graphic window.
//
//  PARAMETERS:     
//
//  RETURNS:        
//                DD_OK             successful
//                others            failed
//
//------------------------------------------------------------------------------
HRESULT MX27DDLcdc::GetOverlayAlphaValue(VOID)
{
    m_pOverlaySurfaceOp->Transparency = \
        LCDC_GW_TRANSPARENCY(EXTREG32BF(&m_pLcdcReg->GWCR, LCDC_GWCR_GWAV));

    return DD_OK;
}


//------------------------------------------------------------------------------
//
//  FUNCTION:     SetOverlayAlphaValue
//
//  DESCRIPTION:  This function is to set current transparency of the 
//                graphic window.
//
//  PARAMETERS:     
//
//  RETURNS:        
//                DD_OK             successful
//                others            failed
//
//------------------------------------------------------------------------------
HRESULT MX27DDLcdc::SetOverlayAlphaValue(VOID)
{
    if((m_pOverlaySurfaceOp->Transparency >= 0) &&
        (m_pOverlaySurfaceOp->Transparency <= 0xFF))
    {
        INSREG32BF(&m_pLcdcReg->GWCR, LCDC_GWCR_GWAV, \
            LCDC_GW_TRANSPARENCY(m_pOverlaySurfaceOp->Transparency));

        return DD_OK;
    }
    else
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return DDERR_INVALIDPARAMS;
    }
}
#endif //0  


//------------------------------------------------------------------------------
//
//  FUNCTION:     WaitForNotBusyOverlay
//
//  DESCRIPTION:  This function is to wait until the graphic window of 
//                the LCDC of i.MX27 processor available to update.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
VOID MX27DDLcdc::WaitForNotBusyOverlay(VOID)
{
    // TODO: We can't use the interrupt for graphic window because it
    //       will mess up the syncronyzation of the main frame buffer.

    return;
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
    if ((pSurf1->nBufPhysicalAddr == pSurf2->nBufPhysicalAddr) &&
        (pSurf1->Width == pSurf2->Width) &&
        (pSurf1->Height == pSurf2->Height) &&
        (pSurf1->WidthHw == pSurf2->WidthHw) &&
        (pSurf1->HeightHw == pSurf2->HeightHw) &&
        (pSurf1->LineStride == pSurf2->LineStride) &&
        (pSurf1->XOffset == pSurf2->XOffset) &&
        (pSurf1->YOffset == pSurf2->YOffset) &&
        (pSurf1->Transparency == pSurf2->Transparency) &&
        (pSurf1->isUpsideDown == pSurf2->isUpsideDown) &&
        (pSurf1->ColorKeyMask == pSurf2->ColorKeyMask))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
