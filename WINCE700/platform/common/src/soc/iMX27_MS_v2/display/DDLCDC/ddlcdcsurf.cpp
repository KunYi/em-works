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
// File:        DDLcdcSurf.cpp
// Purpose:     surface allocation/manipulation/free routines
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
// CLASS MEMBER FUNCTIONS

//------------------------------------------------------------------------------
//
// Function: SetVisibleSurface
//
// This function is used to update the display to 
// a surface other than current one. 
//
// Parameters:
//      pSurf 
//          [in] A pointer to the surface that will be displayed. 
//
//      bWaitForVBlank 
//          [in] Indicates whether the flip should wait to 
//          synchronize with a vertical blank. 
//
// Returns:
//      TRUE: successful
//      FALSE: failed
//
//------------------------------------------------------------------------------
VOID MX27DDLcdc::SetVisibleSurface(GPESurf * pSurf, BOOL bWaitForVBlank)
{
    if(bWaitForVBlank)
        WaitForNotBusy();

    EnterCriticalSection(&m_CriticalSection);
    m_pLcdcReg->SSAR = m_nLAWPhysical + pSurf->OffsetInVideoMemory();

    // Set the flip in progress flag here, it will be cleared in IST
    // when the EOF arrives.
    m_bFlipInProgress = TRUE;
    // Enable the end of frame interrupt here, we disable it in the IST when
    // the interrupt is fired.
    OUTREG32(&m_pLcdcReg->IER, CSP_BITFVAL(LCDC_IER_EOF_EN, LCDC_IER_EOF_EN_ENABLE));    

    // Update the pre-int status with the latest interrupt status.
    m_nPreIntrStatus = m_pLcdcReg->ISR;
    LeaveCriticalSection(&m_CriticalSection);

    return;
}


//------------------------------------------------------------------------------
//
// Function: AllocSurface
//
// This method executes when the driver 
// must allocate storage for surface pixels.
//
// Parameters:
//      ppSurf
//          [in] A pointer to a new DDGPESurf object. 
//
//      width 
//          [in] The desired width of the surface. 
//
//      height 
//          [in] The desired height of the surface. 
//
//      format 
//          [in] The desired format of the surface. 
//
//      pixelFormat
//          [in] The desired pixel format of the surface. 
//
//      surfaceFlags 
//          [in] GPE surface flags for the surface. 
//
// Returns:
//      S_OK            successful
//      others          failed
//
//------------------------------------------------------------------------------
SCODE MX27DDLcdc::AllocSurface(GPESurf **ppSurf, int width, int height, EGPEFormat format, int surfaceFlags)
{
    return AllocSurface((DDGPESurf**)ppSurf, 
                        width, 
                        height,
                        format, 
                        EGPEFormatToEDDGPEPixelFormat[format],
                        surfaceFlags);
}

SCODE MX27DDLcdc::AllocSurface(DDGPESurf **ppSurf, int width, int height,
    EGPEFormat format, EDDGPEPixelFormat pixelFormat, int surfaceFlags)
{
    DWORD bpp  = EGPEFormatToBpp[format];
    
    DWORD alignedWidth = ((width + m_nSurfacePixelAlign - 1) & (~(m_nSurfacePixelAlign - 1)));
    DWORD nSurfaceSize = (bpp * (alignedWidth * height)) / 8;
    DWORD stride = ((alignedWidth * bpp) / 8);

    if ((surfaceFlags & GPE_REQUIRE_VIDEO_MEMORY) || 
        ((surfaceFlags & GPE_PREFER_VIDEO_MEMORY) && (format == m_pMode->format) ||
        (surfaceFlags & GPE_BACK_BUFFER))
        )
    {
        if (!(format == m_pMode->format))
            return E_INVALIDARG;

        RETAILMSG(0, (TEXT("Video memory before after surface allocation: %x"),
             m_pVideoMemoryHeap->Available()));

        // Attempt to allocate from video memory
        SurfaceHeap *pStack = m_pVideoMemoryHeap->Alloc(nSurfaceSize);
        if (pStack)
        {

            ULONG offset = pStack->Address();
            *ppSurf = new MX27DDLcdcSurf(width, height, offset, m_pLAW + offset, stride,
                                      format, pixelFormat, pStack);

            RETAILMSG(0, (TEXT("Video memory remaining after surface allocation: %x"),
                 m_pVideoMemoryHeap->Available()));

            if (!(*ppSurf))
            {
                pStack->Free();
                return E_OUTOFMEMORY;
            }
            
            return S_OK;
        }

        if (surfaceFlags & GPE_REQUIRE_VIDEO_MEMORY)
        {
            *ppSurf = NULL;
            return E_OUTOFMEMORY;
        }
    }

    // Allocate from system memory
    DEBUGMSG(GPE_ZONE_CREATE, (TEXT("Creating a DDGPESurf in system memory - ")
         TEXT("EGPEFormat = %d, DDGPEFormat = %d\r\n"),
         (int) format, (int) pixelFormat));

    *ppSurf = new DDGPESurf(width, height, stride, format, pixelFormat);

    if (*ppSurf)
    {
        // check we allocated bits succesfully
        if (!((*ppSurf)->Buffer()))
        {
            delete *ppSurf;
            return E_OUTOFMEMORY;
        }
    }

    return S_OK;
}


//------------------------------------------------------------------------------
//
// Function: Flip
//
// This callback function associates the surface memory for the back 
// buffer with the surface memory for the front buffer. 
//
// Parameters:
//      pd 
//          [in, out] Pointer to a DDHAL_FLIPDATA structure that 
//          contains information necessary to perform a flip. 
//
// Returns:
//      Returns one of the following values: 
//          DDHAL_DRIVER_HANDLED 
//          DDHAL_DRIVER_NOTHANDLED 
//          DDERR_CURRENTLYNOTAVAIL 
//
//------------------------------------------------------------------------------
DWORD MX27DDLcdc::Flip(LPDDHAL_FLIPDATA pd)
{
    if (pd->dwFlags & DDFLIP_WAITNOTBUSY)
    {
        WaitForNotBusy();
    }
    else if (IsBusy())
    {
        DEBUGMSG(GPE_ZONE_FLIP,(TEXT("Graphics engine busy\r\n")));

        pd->ddRVal = DDERR_WASSTILLDRAWING;
        return DDHAL_DRIVER_HANDLED;
    }

    DDGPESurf* surfTarg = DDGPESurf::GetDDGPESurf(pd->lpSurfTarg);

    if (pd->lpSurfCurr->ddsCaps.dwCaps & DDSCAPS_OVERLAY)
    {
        DDGPESurf* surfCurr = DDGPESurf::GetDDGPESurf(pd->lpSurfCurr);
        if (surfCurr == m_pVisibleOverlay)
        {
            SetVisibleSurfaceOverlay(surfTarg);

            m_pVisibleOverlay = surfTarg;
        }
        else
        {
            pd->ddRVal = DDERR_OUTOFCAPS;
            return DDHAL_DRIVER_HANDLED;
        }
    }
    else
    {
        SetVisibleSurface((GPESurf *)surfTarg);
    }

    pd->ddRVal = DD_OK;
    
    return DDHAL_DRIVER_HANDLED;
}

//------------------------------------------------------------------------------
//
// Function: GetFlipStatus
//
// This callback function checks the status of flip operation, based on the
// EOF interrupt
//
// Parameters:
//      pd 
//          [in, out] Pointer to a LPDDHAL_GETFLIPSTATUSDATA structure that 
//          contains information necessary.
//
// Returns:
//      Returns one of the following values: 
//          DDHAL_DRIVER_HANDLED 
//
//------------------------------------------------------------------------------
DWORD  MX27DDLcdc::GetFlipStatus(LPDDHAL_GETFLIPSTATUSDATA pd)
{
    if (IsBusy())
    {
        pd->ddRVal = DDERR_WASSTILLDRAWING;
    }
    else
    {
        pd->ddRVal = DD_OK;
    }

    return DDHAL_DRIVER_HANDLED;
}
//------------------------------------------------------------------------------
//
//  FUNCTION:     MX27DDLcdcSurf
//
//  DESCRIPTION:  Constructor of MX27DDLcdcSurf
//
// Parameters:
//      width 
//          [in] The requested width for the surface, in pixels. 
//
//      height 
//          [in] The requested height for the surface, in pixels. 
//
//      offset
//          [in] Offset in video memory for surface.
//
//      pBits 
//          [in] A pointer allocated for the surface's graphic data. 
//          If the pBits parameter is not passed into the constructor, 
//          the DDGPESurf object allocates system memory for that 
//          surface's data. 
//
//      stride 
//          [in] The requested stride for the surface. 
//
//      format 
//          [in] The requested format for the surface. 
//
//      pixelFormat 
//          [in] The requested pixel format for the surface. 
//
//      pHeap
//          [in] Pointer to surface heap allocated for the surface.
//
// Returns:
//      New MX27DDLcdcSurf object.
//
//------------------------------------------------------------------------------
MX27DDLcdcSurf::MX27DDLcdcSurf(int width, int height, ULONG offset, PVOID pBits, 
                 int stride, EGPEFormat format, EDDGPEPixelFormat pixelFormat,
                 SurfaceHeap *pHeap)
                : DDGPESurf(width, height, pBits, stride, format, pixelFormat)
{
    m_pHeap = pHeap;
    m_fInVideoMemory = TRUE;
    m_nOffsetInVideoMemory = offset;
}


//------------------------------------------------------------------------------
//
//  FUNCTION:     ~MX27DDLcdcSurf
//
//  DESCRIPTION:  Destructor of ~MX27DDLcdcSurf
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
MX27DDLcdcSurf::~MX27DDLcdcSurf(VOID)
{
    m_pHeap->Free();
}
