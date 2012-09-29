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
/*++
   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
   PARTICULAR PURPOSE.

   Module Name:    surface allocation/manipulation/free routines

   Abstract:

   Functions:


   Notes:


   --*/
//------------------------------------------------------------------------------
//
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
// File:        ddlcdif_surf.cpp
//
// Implementation of DDLcdif surface allocation/manipulation/free routines.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include "precomp.h"
#include "dispperf.h"
#pragma warning(pop)


//#undef DEBUGMSG
//#define DEBUGMSG(cond, msg) RETAILMSG(1,msg)

#define ADDRESSALIGN 4 //The address MUST be word-aligned for proper PXP operation.

CRITICAL_SECTION AllocCS;

//-----------------------------------------------------------------------------
// External Variables
extern HANDLE m_hCombSurfUpdatedEvent;
//-----------------------------------------------------------------------------

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
//      surfaceFlags
//          [in] GPE surface flags for the surface.
//
// Returns:
//      S_OK            successful
//      others          failed
//
//------------------------------------------------------------------------------
SCODE DDLcdif::AllocSurface(GPESurf** ppSurf,
                                  int width,
                                  int height,
                                  EGPEFormat format,
                                  int surfaceFlags)
{
//    DEBUGMSG (1,(TEXT("AllocSurface-1(width=%d(0x%x) height=%d(0x%x))\r\n"),width,width,height,height));
    return AllocSurface((DDGPESurf**) ppSurf,
                        width,
                        height,
                        format,
                        EGPEFormatToEDDGPEPixelFormat[format],
                        surfaceFlags);
} //AllocSurface

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
SCODE DDLcdif::AllocSurface(DDGPESurf** ppSurf,
                                  int width,
                                  int height,
                                  EGPEFormat format,
                                  EDDGPEPixelFormat pixelFormat,
                                  int surfaceFlags)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(surfaceFlags);

    DEBUGMSG (1,(TEXT("AllocSurface-2(width=%d(0x%x) height=%d(0x%x))\r\n"),width,width,height,height));

    DWORD bpp = EGPEFormatToBpp[format];

    if (pixelFormat == ddgpePixelFormat_YV12)
    {
        // YV12 has 12 bits per pixel.
        bpp = 12;
    }

    DWORD size = (width * height * bpp / 8 + ADDRESSALIGN -1) & ~(ADDRESSALIGN -1);//The address MUST be word-aligned for proper PXP operation.
    DWORD stride = ((width * (bpp / 8) + 3) >> 2) << 2;//Change to this to get rid of data abort in TVOut mode while using mediaplayer.

    if (pixelFormat == ddgpePixelFormat_YV12)
    {
        // Y plane has 8 bits per pixel, and this is how we measure
        // stride for YV12 surfaces.
        stride = width * 8 >> 3;
    }

    if ((surfaceFlags & (GPE_REQUIRE_VIDEO_MEMORY | GPE_BACK_BUFFER)) ||
        (surfaceFlags & GPE_PREFER_VIDEO_MEMORY))
    {
        switch(pixelFormat)
        {
        case ddgpePixelFormat_565:
            DEBUGMSG (1,(TEXT("AllocSurface-2 ddgpePixelFormat_565\r\n")));
            break;
        case ddgpePixelFormat_5550:
            DEBUGMSG (1,(TEXT("AllocSurface-2 ddgpePixelFormat_5550\r\n")));
            break;
        case ddgpePixelFormat_8888:
            DEBUGMSG (1,(TEXT("AllocSurface-2 ddgpePixelFormat_8888\r\n")));
            break;
        case ddgpePixelFormat_YV12:
            DEBUGMSG (1,(TEXT("AllocSurface-2 ddgpePixelFormat_YV12\r\n")));
            break;
        case ddgpePixelFormat_8880:
            DEBUGMSG (1,(TEXT("AllocSurface-2 ddgpePixelFormat_8880\r\n")));
            break;
        case ddgpePixelFormat_5551:
            DEBUGMSG (1,(TEXT("AllocSurface-2 ddgpePixelFormat_5551\r\n")));
            break;              
        default:
            DEBUGMSG (1,(TEXT("AllocSurface-2 other type %d\r\n"),pixelFormat));
            goto AllocFromSys;
        }

        EnterCriticalSection (&AllocCS);

        SurfaceHeap* heap = FrameBufferHeap->Alloc(size);
        if (!heap) {
            DEBUGMSG (1, (L"No mem for heap\r\n"));
            LeaveCriticalSection(&AllocCS);

            if (surfaceFlags & GPE_REQUIRE_VIDEO_MEMORY)
            {
                *ppSurf = NULL;
                return DDERR_OUTOFVIDEOMEMORY;
            }
            else 
                goto AllocFromSys;
        } //if

        ULONG offset = heap->Address() - ((ULONG) VirtualMemAddr);
        DEBUGMSG (1,(TEXT("AllocSurface-2(bpp=%d size=0x%x stride=%d offset=0x%x))\r\n"),bpp,size,stride,offset));

        *ppSurf = new DDLcdifSurf(width, height,
                                        offset,
                                        (void*)(((ULONG) VirtualMemAddr) + offset),
                                        stride,
                                        format, pixelFormat,
                                        heap);

        if (!*ppSurf) {
            DEBUGMSG (1, (L"Bad *ppSurf\r\n"));
            heap->Free();
            LeaveCriticalSection(&AllocCS);
            return DDERR_OUTOFVIDEOMEMORY;
        } //if

        LeaveCriticalSection(&AllocCS);

        return S_OK;
    }
AllocFromSys:
    DEBUGMSG(1, (TEXT("Creating a DDGPESurf in system memory - ")
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
} //AllocSurface

//------------------------------------------------------------------------------
//
// Function: SetVisibleSurface
//
// This function is used to update the display to 
// a surface other than current one. 
//
// Parameters:
//      pTempSurf 
//          [in] A pointer to the surface that will be displayed. 
//
//      bWaitForVBlank 
//          [in] Indicates whether the flip should wait to 
//          synchronize with a vertical blank. 
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void DDLcdif::SetVisibleSurface(GPESurf* pTempSurf, BOOL bWaitForVBlank)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pTempSurf);

    DEBUGMSG (1, (L"setvisiblesurface enter\r\n"));

    WaitForNotBusy();

    if (bWaitForVBlank) {
        WaitForVBlank();

        // \todo WBB: Wait for VSYNC IRQ and Flip.
    } //if

    LCDIFDisplayFrameBuffer((const void*) (m_nLAWPhysical+pTempSurf->OffsetInVideoMemory()));
    DEBUGMSG (1, (L"setvisiblesurface exit\r\n"));
} //SetVisibleSurface

//------------------------------------------------------------------------------
//
//  FUNCTION:     DDLcdifSurf
//
//  DESCRIPTION:  Constructor of DDLcdifSurf
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
//      New DDLcdifSurf object.
//
//------------------------------------------------------------------------------
DDLcdifSurf::DDLcdifSurf(int width, int height, ULONG offset,
                                     void* pBits, int stride,
                                     EGPEFormat format,
                                     EDDGPEPixelFormat pixelFormat,
                                     SurfaceHeap* pHeap) :
    DDGPESurf (width, height, pBits, stride, format, pixelFormat)
{
    this->pHeap = pHeap;
    this->size = width * height * (EGPEFormatToBpp[format] >> 3);

    DEBUGMSG (1,(TEXT("DDLcdifSurf-const pHeap=0x%x offset=0x%x\r\n"),pHeap, offset));

    /* GPESurf Global Variables */
    m_fInVideoMemory = TRUE;
    m_nOffsetInVideoMemory = offset;
    m_nHandle = NULL;
} //DDLcdifSurf

//------------------------------------------------------------------------------
//
// Function:     ~DDLcdifSurf
//
//  Destructor of ~DDLcdifSurf
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
DDLcdifSurf::~DDLcdifSurf()
{
    DEBUGMSG (1,(TEXT("DDLcdifSurf-del pHeap=0x%x\r\n"),pHeap));
    pHeap->Free();
} //~DDLcdifSurf


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
DWORD DDLcdif::Flip(LPDDHAL_FLIPDATA pd)
{

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
            DEBUGMSG(1,(_T(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>")));
            return DDHAL_DRIVER_HANDLED;
        }
    }
    else
    {
        SetVisibleSurfacePrimary(surfTarg);
    }

    pd->ddRVal = DD_OK;

    return DDHAL_DRIVER_HANDLED;
}

//------------------------------------------------------------------------------
//
// Function: SetVisibleSurfacePrimary
//
// This function associates the surface memory for the back buffer
// with the surface memory for the front buffer in primary surface. 
//
// Parameters:
//      pSurf 
//          [in] Pointer to a DDGPESurf structure that 
//          contains information about surface to perform a flip. 
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void  DDLcdif::SetVisibleSurfacePrimary(DDGPESurf* pSurf)
{

    m_pBackgroundSurface = pSurf;
    
    if(m_pCombinedSurface)  //Overlay surface is enabled, need to call PXP for combination
    {
        EnterCriticalSection(&m_PxpOperateCS);
        if(m_pOverlaySurfaceOp->OverlayPara.B.ALPHASRC_EN||m_pOverlaySurfaceOp->OverlayPara.B.ALPHACONSTANT_EN) //Overlay surface with alpha channel
        {
            if(!m_hPXP)
            {
                LeaveCriticalSection(&m_PxpOperateCS);
                return;
            }

            pxpS0BufferAddrGroup pxpS0AddrGroup;
            pxpS0AddrGroup.iRGBorYBufAddr = m_nLAWPhysical+m_pBackgroundSurface->OffsetInVideoMemory();
            PXPSetS0BufferAddrGroup(m_hPXP, &pxpS0AddrGroup);
        }
        else    //Overlay surface without alpha channel
        {
            if(!m_hPXP)
            {
                LeaveCriticalSection(&m_PxpOperateCS);
                return;
            }

            pxpOverlayBuffersAddr pxpOverlayAddr;
            pxpOverlayAddr.iOverlayBufNum = 0;
            pxpOverlayAddr.iOverlayBufAddress = (UINT32) m_nLAWPhysical+m_pBackgroundSurface->OffsetInVideoMemory();
            PXPSetOverlayBuffersAddr(m_hPXP, &pxpOverlayAddr);
        }
        PXPStartProcess(m_hPXP, FALSE);
        SetEvent(m_hCombSurfUpdatedEvent);   
        LeaveCriticalSection(&m_PxpOperateCS);
    }
    else    //There is no overlay surface
    {
        SetVisibleSurface((GPESurf *)pSurf);
    }

}

