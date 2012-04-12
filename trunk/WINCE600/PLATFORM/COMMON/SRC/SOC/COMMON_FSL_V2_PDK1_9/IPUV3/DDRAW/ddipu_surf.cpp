//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  ddipu_surf.cpp
//
//  Implementation of DDIPU surface allocation/manipulation/free routines.
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
VOID DDIPU::SetVisibleSurface(GPESurf * pSurf, BOOL bWaitForVBlank)
{
    if (bWaitForVBlank)
        WaitForNotBusy();

    // Change primary surface pointer to new surface
    // Wrap with CS to avoid changing primary surface
    // while functions are using it.
    EnterCriticalSection(&m_csDrawLock);
    //Active surface is changed
    if(pSurf != m_pActiveSurface)
        m_CursorAreaLock = FALSE;
    m_pActiveSurface = (DDIPUSurf *)pSurf;

    LeaveCriticalSection(&m_csDrawLock);
    DisplaySetSrcBuffer(pSurf->OffsetInVideoMemory());
    return;
}


#if defined(USE_C2D_ROUTINES)
SCODE DDIPU::AllocSurface(GPESurf           **ppSurf,
                          int               width,
                          int               height,
                          int               stride,
                          unsigned int      virtAddr,
                          unsigned int      physAddr,
                          EGPEFormat        format,
                          C2D_COLORFORMAT   c2dFormat)
{
    ppSurf;
    width;
    height;
    stride;
    virtAddr;
    physAddr;
    format;
    c2dFormat;

    return S_OK;
}
#endif 

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
SCODE DDIPU::AllocSurface(GPESurf **ppSurf,
                          int width,
                          int height,
                          EGPEFormat format,
                          int surfaceFlags)
{
    return AllocSurface((DDGPESurf**)ppSurf,
                        width,
                        height,
                        format,
                        EGPEFormatToEDDGPEPixelFormat[format],
                        surfaceFlags);
}

SCODE DDIPU::AllocSurface(DDGPESurf **ppSurf,
                          int width,
                          int height,
                          EGPEFormat format,
                          EDDGPEPixelFormat pixelFormat,
                          int surfaceFlags)
{
    DWORD bpp  = EGPEFormatToBpp[format];
    // YV12 planar is a weird case.  Set up surface here.
    if ((pixelFormat == ddgpePixelFormat_YV12)
        ||(pixelFormat == ddgpePixelFormat_NV12))
    {
        // YV12 has 12 bits per pixel.
        bpp = 12;
    }

     DWORD alignedWidth = ((width + m_nSurfacePixelAlign - 1) &
                           (~(m_nSurfacePixelAlign - 1)));
     DWORD nSurfaceSize = (bpp * (alignedWidth * height)) / 8;
     DWORD stride = ((alignedWidth * bpp) / 8);

    if ((pixelFormat == ddgpePixelFormat_YV12)
        ||(pixelFormat == ddgpePixelFormat_NV12))
    {
        // Y plane has 8 bits per pixel, and this is how we measure
        // stride for YV12 surfaces.
        stride = ((alignedWidth * 8) / 8);
    }
    else
    {
        //enlarge this alignment to fit c2d 
        alignedWidth = ((width + 32 - 1) &
                           (~(32 - 1)));   //32-pixel aligned
        DWORD alignedHeight = ((height + 32 - 1) &
                           (~(32 - 1)));   //32-pixel aligned,  z430 required.                 
        stride = ((alignedWidth * bpp) / 8);
        nSurfaceSize = (bpp * (alignedWidth * alignedHeight)) / 8;
        nSurfaceSize = ((nSurfaceSize + 4096 - 1) &
                           (~(4096 - 1)));   // 4K aligned offset
    }    

    if ((surfaceFlags & (GPE_REQUIRE_VIDEO_MEMORY | GPE_BACK_BUFFER)) ||
        ((surfaceFlags & GPE_PREFER_VIDEO_MEMORY) &&
         (format == m_pMode->format)))
    {

        // Attempt to allocate from video memory
        PREFAST_SUPPRESS(28197, "pIpuBuffer will be freed in DisplayDeallocateBuffer.");
        IpuBuffer * pIpuBuffer = new IpuBuffer(nSurfaceSize, MEM_TYPE_VIDEOMEMORY);
        DisplayAllocateBuffer(nSurfaceSize, pIpuBuffer);
        if (pIpuBuffer)
        {
            if(!pIpuBuffer->IpuBufferHeap())
            {
                delete pIpuBuffer;
                return E_OUTOFMEMORY;
            }
            //It's a callback function, it may be run in kernel mode or user mode.
            //CeOpenCallerBuffer maybe return E_ACCESSDENIED when it runs in user mode.
            //For getting memory from kernel process, we needn't do marshaling in this case.
            *ppSurf = new DDIPUSurf(width, height, (ULONG)pIpuBuffer->PhysAddr(), pIpuBuffer->VirtAddr(),
                                    stride, format, pixelFormat, pIpuBuffer);

            if (!(*ppSurf))
            {
                DisplayDeallocateBuffer(pIpuBuffer);
                return E_OUTOFMEMORY;
            }

#if defined(USE_C2D_ROUTINES)

            //&&&&&&&: wrap a c2d surface here
            //
            //  For every DDIPUSurf object, create a C2DGPE wrapper.  This way we can use our C2D routines a little more transparently.
            //
            C2D_SURFACE     c2dSurf;
            C2D_SURFACE_DEF surfDef;
            
            if (pixelFormat == ddgpePixelFormat_CustomFormat )
                surfDef.format  = EDDGPEPixelFormatToC2DFormat[1]; //C2D_UNKNOWNFORMAT, avoid prefast warning
            else
                surfDef.format  = EDDGPEPixelFormatToC2DFormat[pixelFormat];
            surfDef.width   = width;
            surfDef.height  = height;
            surfDef.buffer  = (void*)pIpuBuffer->PhysAddr();
            surfDef.host    = (void*)pIpuBuffer->VirtAddr();
            surfDef.stride  = stride;
            surfDef.flags   = C2D_SURFACE_NO_BUFFER_ALLOC; //do not allocate real buffer, just a wrapper

            C2D_STATUS alloc_status = c2dSurfAlloc(m_c2dCtx, &c2dSurf, &surfDef);
            if(alloc_status == C2D_STATUS_OK)
            {
                ((DDIPUSurf*)*ppSurf)->m_c2dCtx = m_c2dCtx;
                ((DDIPUSurf*)*ppSurf)->m_c2dSurf = c2dSurf;
            } else
            {
                return E_OUTOFMEMORY;
            }
#endif

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
// Function: DetectPixelFormat
//
// This method is for handling NV12 format which is a customized pixel format.
// For other pixel format it will call DDGPE::DetectPixelFormat() to handle it.
//
// Parameters:
//      dwCaps
//          [in] The flags for capabilities of the surface.
//
//      pDDPF
//          [in] Explicit pixel format or current mode
//
//      pFormat
//          [in] The desired format of the surface.
//
//      pPixelFormat
//          [in] The desired pixel format of the surface.
//
// Returns:
//      DD_OK          successful
//      others          failed
//
//------------------------------------------------------------------------------
SCODE DDIPU::DetectPixelFormat(
                DWORD                dwCaps,            
                DDPIXELFORMAT*        pDDPF,            
                EGPEFormat*            pFormat,
                EDDGPEPixelFormat*    pPixelFormat
                )
{
    if( pDDPF->dwFlags & DDPF_FOURCC )
    {
        if( pDDPF->dwFourCC == FOURCC_NV12)
        {
            pDDPF->dwYUVBitCount    = 12;
            pDDPF->dwYBitMask       = (DWORD) -1;
            pDDPF->dwUBitMask       = (DWORD) -1;
            pDDPF->dwVBitMask       = (DWORD) -1;
            *pPixelFormat = (EDDGPEPixelFormat)ddgpePixelFormat_NV12;
        }
        else
        {
            return DDGPE::DetectPixelFormat(dwCaps, pDDPF, pFormat, pPixelFormat);
        }
    }
    else
    {
        return DDGPE::DetectPixelFormat(dwCaps, pDDPF, pFormat, pPixelFormat);
    }
    *pFormat = gpe16Bpp;

    DEBUGMSG(GPE_ZONE_CREATE, (TEXT("Detected surface pixelFormat, format = %d, %d\r\n"), *pPixelFormat, *pFormat));

    return DD_OK;
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
DWORD DDIPU::Flip(LPDDHAL_FLIPDATA pd)
{
    DDGPESurf* surfTarg = DDGPESurf::GetDDGPESurf(pd->lpSurfTarg);
    DWORD retVal;

    if (pd->lpSurfCurr->ddsCaps.dwCaps & DDSCAPS_OVERLAY)
    {
        // Synchronicity is handled by double buffering in the ASync
        // ...flipping is thus automatically synchronized with
        // end-of-frame for the ASync channel.
        //if (pd->dwFlags & DDFLIP_WAIT)
        //{
        //    WaitForNotBusy(DisplayPlane_1);
        //}

        DDGPESurf* surfCurr = DDGPESurf::GetDDGPESurf(pd->lpSurfCurr);
        
        //Pass down the dwFlags
        EnterCriticalSection(&m_csOverlayData);
        retVal = SetVisibleSurfaceOverlay(surfTarg, surfCurr, pd->dwFlags);
        LeaveCriticalSection(&m_csOverlayData);
        if ((retVal == DDERR_WASSTILLDRAWING) || (retVal == DDERR_OUTOFCAPS))
        {
            // If set visible surface didn't go smoothly, 
            // set return value appropriately
            pd->ddRVal = retVal;
            return DDHAL_DRIVER_HANDLED;
        }

        pd->ddRVal = DD_OK;
    }
    else if((pd->lpSurfCurr->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE2)
        == DDSCAPS_PRIMARYSURFACE2)
    {
        m_pPrimarySurface2 = (DDIPUSurf *)surfTarg;
        DisplaySetSrcBuffer2(m_pPrimarySurface2->OffsetInVideoMemory());
    }
    else
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

        SetVisibleSurface((GPESurf *)surfTarg);
    }

    pd->ddRVal = DD_OK;

    return DDHAL_DRIVER_HANDLED;
}

DDIPUSurf * DDIPU::PrimarySurface2()
{
    return m_pPrimarySurface2;
}


//------------------------------------------------------------------------------
//
// Function: DDIPUSurf
//
// Constructor of DDIPUSurf.
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
//      pIpuBuffer
//          [in] Pointer to IPU Buffer object created for the surface.
//
// Returns:
//      New DDIPUSurf object.
//
//------------------------------------------------------------------------------
DDIPUSurf::DDIPUSurf(int width, int height, ULONG offset, PVOID pBits,
                 int stride, EGPEFormat format, EDDGPEPixelFormat pixelFormat,
                 IpuBuffer *pIpuBuffer)
                : DDGPESurf(width, height, pBits, stride, format, pixelFormat)
{
    m_pIpuBuffer = pIpuBuffer;
    m_fInVideoMemory = TRUE;
    m_nOffsetInVideoMemory = offset;

    if ((pixelFormat == ddgpePixelFormat_YV12)
        ||(pixelFormat == ddgpePixelFormat_NV12))
    {
        // Correct the m_BytesPixel for special format. Otherwise lxpitch will be incorrect.
        // YV12 original value is 2. NV12 is 0.
        m_BytesPixel = 1;
        // Correct the surface size for YV12 and NV12.
        m_dwSurfaceSize = m_dwSurfaceSize /2 *3;
    }


    m_nHandle = NULL;
}


//------------------------------------------------------------------------------
//
// Function: ~DDIPUSurf
//
// Destructor of ~DDIPUSurf.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
DDIPUSurf::~DDIPUSurf(VOID)
{
#if defined(USE_C2D_ROUTINES)
    if (m_c2dSurf)
    {
        c2dSurfFree(m_c2dCtx, m_c2dSurf);
    }
#endif
    DisplayDeallocateBuffer(m_pIpuBuffer);

}

#if defined(USE_C2D_ROUTINES)
// Copies data from given surface to this instance. 
bool DDIPUSurf::AssignSurfData(GPESurf* source)
{
    if (source)
    {
        //RETAILMSG(1,(TEXT("DDIPUSurf::AssignSurfData - Assigning data to %ix%i surface\n"), m_nWidth, m_nHeight));        
        void* ptr;
        C2D_STATUS status = c2dSurfLock(m_c2dCtx, m_c2dSurf, &ptr);
        if (status == C2D_STATUS_OK)
        {
            void* srcPtr = source->Buffer();
            //For 1bpp mask data the bytesperpixel will be zero.
            if(source->Format() == gpe1Bpp)
            {
                for (int y = 0; y < source->Height(); y++)
                {                                    
                    memcpy(ptr, srcPtr, (source->Width()+7)/8);
                    srcPtr = (unsigned char*)srcPtr + source->Stride();
                    ptr    = (unsigned char*)ptr    + m_nStrideBytes;
                }  
            }
            else
            {
                for (int y = 0; y < source->Height(); y++)
                {                                    
                    memcpy(ptr, srcPtr, source->Width() * source->BytesPerPixel());
                    srcPtr = (unsigned char*)srcPtr + source->Stride();
                    ptr    = (unsigned char*)ptr    + m_nStrideBytes;
                }
            }
            c2dSurfUnlock(m_c2dCtx, m_c2dSurf);
            return true;
        }
    }
    return false;
}
#endif
