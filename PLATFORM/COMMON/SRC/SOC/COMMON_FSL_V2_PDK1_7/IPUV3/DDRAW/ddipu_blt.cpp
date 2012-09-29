//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  ddipu_blt.cpp
//
//  Implementation of DDIPU bitblt operation functions.
//
//------------------------------------------------------------------------------

#include "precomp.h"
#pragma warning(push)
#pragma warning(disable: 4201 4245 4100)
#include "dispperf.h"
#pragma warning(pop)

//------------------------------------------------------------------------------
// External Functions


//------------------------------------------------------------------------------
// External Variables


//------------------------------------------------------------------------------
// Defines
#undef SWAP
#define SWAP(a,b,type) { type tmp=a; a=b; b=tmp; }

#ifndef BLT_WAITVSYNC
#define BLT_WAITVSYNC 2048
#endif
//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Global Variables


//------------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
// Local Functions
#if defined(USE_C2D_ROUTINES)

const unsigned int GPEToC2DRotation[] = 
{ 
    0,   // DMDO_0,
    90,  // DMDO_90  
    180, // DMDO_180 
    0,   // Invalid
    270, // DMDO_270
};

//=========================================================================
// Video memory to video memory BLT through C2D
//=========================================================================
SCODE DDIPU::VidmemToVidmemBlt(GPEBltParms* bltParms)
{
    C2D_STATUS status;    

    C2D_ENTER;
    // Destination
    ON_ERROR_EXIT_AND_LOG(SetBltDestination(bltParms->pDst, bltParms->prclDst, bltParms->prclClip),
                          L"DDIPU::VidmemToVidmemBlt - Failed to set BLT destination.\r\n");

    // Source
    ON_ERROR_EXIT_AND_LOG(SetBltSource(bltParms->pSrc, bltParms->prclSrc),
                          L"DDIPU::VidmemToVidmemBlt - Failed to set BLT source.\r\n");

    // Blt parameters
    ON_ERROR_EXIT_AND_LOG(SetBltParms(bltParms),
                          L"DDIPU::VidmemToVidmemBlt - Failed to set C2D blt parameters.\r\n");

    //RETAILMSG(1,(TEXT("DDIPU::VidmemToVidmemBlt - Performing accelerated blt operation.\n")));

    // Do the actual blit
    status = c2dDrawBlit(m_c2dCtx);
    if (status == C2D_STATUS_OK)
    {
        if(bltParms->bltFlags & BLT_WAITVSYNC)
        {
            if(bltParms->pDst == m_pActiveSurface)
                DisplayWaitForVSync(FALSE);
            else if(bltParms->pDst == m_pPrimarySurface2)
                DisplayWaitForVSync(TRUE);
        }

        status = c2dFinish(m_c2dCtx);
    }
    C2D_EXIT;

    if (status != C2D_STATUS_OK)
    {
        ERRORMSG(1, (L"DDIPU::VidmemToVidmemBlt - C2D blt failed.\r\n"));
    }

    // Update dirty rectangle with new dirty regions.
    // This will be used if we have an ASync driver (to trigger a partial refresh)
    // or if we are using TV out.
    if((bltParms->pDst == m_pActiveSurface) ||    // only care if dest is main display surface
         (bltParms->pSrc == m_pActiveSurface))    // only care if src is main display surface
    {
        // Now, calculate the dirty-rect to refresh to the actual hardware
        RECT bounds;

        bounds.left     = bltParms->prclDst->left;
        bounds.top      = bltParms->prclDst->top;
        bounds.right    = bltParms->prclDst->right;
        bounds.bottom   = bltParms->prclDst->bottom;

        if(bounds.left > bounds.right) {
            SWAP(bounds.left,bounds.right, int)
        }
        if( bounds.top > bounds.bottom) {
            SWAP(bounds.top,bounds.bottom, int)
        }

        // If boundaries are screwy, just make boundaries as large as possible to be safe
        if((bounds.left < 0) || (bounds.left > (m_rcWorkRect.right - m_rcWorkRect.left)))
            bounds.left = 0;

        if((bounds.right < 0) || (bounds.right > (m_rcWorkRect.right - m_rcWorkRect.left)))
            bounds.right = (m_rcWorkRect.right - m_rcWorkRect.left);

        if((bounds.top < 0) || (bounds.top > (m_rcWorkRect.bottom - m_rcWorkRect.top)))
            bounds.top = 0;

        if((bounds.bottom < 0) || (bounds.bottom > (m_rcWorkRect.bottom - m_rcWorkRect.top)))
            bounds.bottom = (m_rcWorkRect.bottom - m_rcWorkRect.top);

        DisplayUpdate((LPRECT)&bounds);
    }


    return C2DStatusToScode(status);
}

//=========================================================================
// System memory to video memory BLT through C2D
//=========================================================================
SCODE DDIPU::SysmemToVidmemBlt(GPEBltParms* bltParms)
{
    GPESurf* src = bltParms->pSrc;
    DDIPUSurf* tempSurface;

    //RETAILMSG(1,(TEXT("DDIPU::SysmemToVidmemBlt - Allocating %ix%i temporary surface.\n"), src->Width(), src->Height()));
    // Copy source surface data to video memory    
    if (CopyToVideoMemory(src, &tempSurface) != S_OK)
    {
        //RETAILMSG(1,(TEXT("DDIPU::SysmemToVidmemBlt - Falling back to EmulatedBlt.\n")));
        return EmulatedBlt(bltParms);
    }
    
    bltParms->pSrc = tempSurface;
    SCODE status = VidmemToVidmemBlt(bltParms);   
    

    delete tempSurface;
    bltParms->pSrc = src;
    return status;
}

//=========================================================================
// Solid color fill through C2D
//=========================================================================
SCODE DDIPU::SolidColorFill(GPEBltParms* bltParms)
{
    C2D_STATUS status;
    C2D_ENTER;

    // Destination
    ON_ERROR_EXIT_AND_LOG(SetBltDestination(bltParms->pDst, bltParms->prclDst, bltParms->prclClip),
                          L"DDIPU::SolidColorFill - Failed to set BLT destination.\r\n");

        
    unsigned int solidColor;
    // Raster operation
    ROP4 rop = bltParms->rop4;    
    // Change rop for whiteness and blackness
    if (rop == 0x0000 || rop == 0xFFFF)
    {
        solidColor = (rop == 0) ? 0x000000 : 0xFFFFFF;
        // Pattern copy
        rop = 0xF0F0;
    }
    else
    {
        solidColor = bltParms->solidColor;
    }

    // Solid color
    ON_ERROR_EXIT_AND_LOG(c2dSetFgColor(m_c2dCtx, solidColor),
                          L"DDIPU::SolidColorFill - Failed to set foreground color.\r\n");

    ON_ERROR_EXIT_AND_LOG(c2dSetRop(m_c2dCtx, rop),
                          L"DDIPU::SolidColorFill - Failed to set rop.\r\n");

    // Do the actual blit
    //RETAILMSG(1,(TEXT("DDIPU::SolidColorFill - Performing solid color fill operation.\n")));    

    status = c2dDrawRect(m_c2dCtx, C2D_PARAM_FILL_BIT);
    if (status == C2D_STATUS_OK)
    {
        if(bltParms->bltFlags & BLT_WAITVSYNC)
        {
            if(bltParms->pDst == m_pActiveSurface)
                DisplayWaitForVSync(FALSE);
            else if(bltParms->pDst == m_pPrimarySurface2)
                DisplayWaitForVSync(TRUE);
        }

        status = c2dFinish(m_c2dCtx);
    }

    C2D_EXIT;

    if (status != C2D_STATUS_OK)
    {
        ERRORMSG(1, (L"DDIPU::SolidColorFill - C2D blt failed.\r\n"));
    }

    // Update dirty rectangle with new dirty regions.
    // This will be used if we have an ASync driver (to trigger a partial refresh)
    // or if we are using TV out.
    if((bltParms->pDst == m_pActiveSurface) ||    // only care if dest is main display surface
         (bltParms->pSrc == m_pActiveSurface))    // only care if src is main display surface
    {
        // Now, calculate the dirty-rect to refresh to the actual hardware
        RECT bounds;

        bounds.left     = bltParms->prclDst->left;
        bounds.top      = bltParms->prclDst->top;
        bounds.right    = bltParms->prclDst->right;
        bounds.bottom   = bltParms->prclDst->bottom;

        if(bounds.left > bounds.right) {
            SWAP(bounds.left,bounds.right, int)
        }
        if( bounds.top > bounds.bottom) {
            SWAP(bounds.top,bounds.bottom, int)
        }

        // If boundaries are screwy, just make boundaries as large as possible to be safe
        if((bounds.left < 0) || (bounds.left > (m_rcWorkRect.right - m_rcWorkRect.left)))
            bounds.left = 0;

        if((bounds.right < 0) || (bounds.right > (m_rcWorkRect.right - m_rcWorkRect.left)))
            bounds.right = (m_rcWorkRect.right - m_rcWorkRect.left);

        if((bounds.top < 0) || (bounds.top > (m_rcWorkRect.bottom - m_rcWorkRect.top)))
            bounds.top = 0;

        if((bounds.bottom < 0) || (bounds.bottom > (m_rcWorkRect.bottom - m_rcWorkRect.top)))
            bounds.bottom = (m_rcWorkRect.bottom - m_rcWorkRect.top);

        DisplayUpdate((LPRECT)&bounds);
    }

    return C2DStatusToScode(status);
}

//=========================================================================
// Pattern fill through C2D
//=========================================================================
SCODE DDIPU::PatternFill(GPEBltParms* bltParms)
{
    C2D_STATUS status;
    C2D_ENTER;

    // Destination
    ON_ERROR_EXIT_AND_LOG(SetBltDestination(bltParms->pDst, bltParms->prclDst, bltParms->prclClip),
                                            L"DDIPU::PatternFill - Failed to set BLT destination.\r\n");
    
    // Raster operation
    ON_ERROR_EXIT_AND_LOG(c2dSetRop(m_c2dCtx, bltParms->rop4),
                          L"DDIPU::PatternFill - Failed to set rop.\r\n");
    // Brush
    DDIPUSurf* brush;         
    // If brush is in system memory, we need to copy it to video memory.
    if (!bltParms->pBrush->InVideoMemory())
    {
        if (CopyToVideoMemory(bltParms->pBrush, &brush) != S_OK)
        {
            C2D_EXIT;
            //RETAILMSG(1,(TEXT("DDIPU::PatternFill - Falling back to EmulatedBlt.\n")));    
            return EmulatedBlt(bltParms);
        }
        // Delete temporary surface before using
        if (m_TempSurf)
        {
            delete m_TempSurf;
        }
        m_TempSurf = brush;
    }
    else
    {
        brush = (DDIPUSurf*)bltParms->pBrush;
    }

    // Brush origin
    int x = (bltParms->pptlBrush) ? (int)bltParms->pptlBrush->x : 0;
    int y = (bltParms->pptlBrush) ? (int)bltParms->pptlBrush->y : 0;
    GetBrushOffset(bltParms, x, y);
    
    C2D_POINT brushOffset = {x, y};

    
    // Set brush as source.
    ON_ERROR_EXIT_AND_LOG(c2dSetBrushSurface(m_c2dCtx, brush->GetC2DSurface(), &brushOffset),
                          L"DDIPU::PatternFill - Failed to set brush.\r\n");
    
    ON_ERROR_EXIT_AND_LOG(SetSurfRotation(brush, bltParms->pDst),
                          L"DDIPU::PatternFill - Failed to set rotation.\r\n");  

            
    // Do the actual blit
    //RETAILMSG(1,(TEXT("DDIPU::PatternFill - Performing pattern fill operation, pattern offset (%i, %i).\n"), x, y));    

    status = c2dDrawRect(m_c2dCtx, (C2D_PARAMETERS)(C2D_PARAM_PATTERN_BIT | C2D_PARAM_TILING_BIT));
    if (status == C2D_STATUS_OK)
    {
        if(bltParms->bltFlags & BLT_WAITVSYNC)
        {
            if(bltParms->pDst == m_pActiveSurface)
                DisplayWaitForVSync(FALSE);
            else if(bltParms->pDst == m_pPrimarySurface2)
                DisplayWaitForVSync(TRUE);
        }

        status = c2dFinish(m_c2dCtx);
    }

    C2D_EXIT;

    if (status != C2D_STATUS_OK)
    {
        ERRORMSG(1, (L"DDIPU::PatternFill - C2D blt failed.\r\n"));
    }

    // Update dirty rectangle with new dirty regions.
    // This will be used if we have an ASync driver (to trigger a partial refresh)
    // or if we are using TV out.
    if((bltParms->pDst == m_pActiveSurface) ||    // only care if dest is main display surface
         (bltParms->pSrc == m_pActiveSurface))    // only care if src is main display surface
    {
        // Now, calculate the dirty-rect to refresh to the actual hardware
        RECT bounds;

        bounds.left     = bltParms->prclDst->left;
        bounds.top      = bltParms->prclDst->top;
        bounds.right    = bltParms->prclDst->right;
        bounds.bottom   = bltParms->prclDst->bottom;

        if(bounds.left > bounds.right) {
            SWAP(bounds.left,bounds.right, int)
        }
        if( bounds.top > bounds.bottom) {
            SWAP(bounds.top,bounds.bottom, int)
        }

        // If boundaries are screwy, just make boundaries as large as possible to be safe
        if((bounds.left < 0) || (bounds.left > (m_rcWorkRect.right - m_rcWorkRect.left)))
            bounds.left = 0;

        if((bounds.right < 0) || (bounds.right > (m_rcWorkRect.right - m_rcWorkRect.left)))
            bounds.right = (m_rcWorkRect.right - m_rcWorkRect.left);

        if((bounds.top < 0) || (bounds.top > (m_rcWorkRect.bottom - m_rcWorkRect.top)))
            bounds.top = 0;

        if((bounds.bottom < 0) || (bounds.bottom > (m_rcWorkRect.bottom - m_rcWorkRect.top)))
            bounds.bottom = (m_rcWorkRect.bottom - m_rcWorkRect.top);

        DisplayUpdate((LPRECT)&bounds);
    }

    return C2DStatusToScode(status);

}


SCODE DDIPU::CopyToVideoMemory(GPESurf* src, DDIPUSurf** dst)
{
    SCODE result = AllocSurface((GPESurf**)dst, src->Width(), src->Height(), src->Format(), GPE_REQUIRE_VIDEO_MEMORY);
    if (result != S_OK)
    {
        ERRORMSG(1, (L"DDIPU::CopyToVideoMemory - Failed to allocate temp surface from video memory.\r\n"));
        return E_FAIL;
    }

    if (!(*dst)->AssignSurfData(src))
    {
        delete dst;
        ERRORMSG(1, (L"DDIPU::CopyToVideoMemory - Failed to copy data to video memory.\r\n"));
        return E_FAIL;
    }
    return S_OK;
}

C2D_STATUS DDIPU::SetBltDestination(GPESurf* dst, PRECTL dstRect, PRECTL clipRect)
{
    if (!(dst && dst->InVideoMemory()))
    {
        ERRORMSG(1, (L"DDIPU::SetBltDestination - Destination surface not in video memory.\r\n"));
        return C2D_STATUS_INVALID_PARAM;
    }

    // Destination surface always in video memory in accelerated blts, cast should be safe.
    DDIPUSurf   *temp = (DDIPUSurf*)dst;
    C2D_STATUS  status;

    C2D_CALL(c2dSetDstSurface(m_c2dCtx, temp->GetC2DSurface()))
    
    C2D_RECT dstSurfRect;
    RECTL rotatedRect = *dstRect;
    dst->RotateRectl(&rotatedRect);
    RectlToC2DRect(&rotatedRect, dstSurfRect);

    C2D_CALL(c2dSetDstRectangle(m_c2dCtx, &dstSurfRect));

    if (clipRect)
    {        
        // Clipping rectangle
        C2D_RECT c2dClipRect;
        rotatedRect = *clipRect;
        dst->RotateRectl(&rotatedRect);
        RectlToC2DRect(&rotatedRect, c2dClipRect);
        C2D_CALL(c2dSetDstClipRect(m_c2dCtx, &c2dClipRect))
    }
    else
    {
        C2D_CALL(c2dSetDstClipRect(m_c2dCtx, 0))
    }    
    return C2D_STATUS_OK;
}

C2D_STATUS DDIPU::SetBltSource(GPESurf* src, PRECTL srcRect)
{
    if (!(src && src->InVideoMemory()))
    {
        ERRORMSG(1, (L"DDIPU::SetBltSource - Source surface not in video memory.\r\n"));
        return C2D_STATUS_INVALID_PARAM;
    }

    C2D_STATUS status;
    
    DDIPUSurf   *temp = (DDIPUSurf*)src;
    C2D_CALL(c2dSetSrcSurface(m_c2dCtx, temp->GetC2DSurface()));

    C2D_RECT srcSurfRect;
    RECTL rotatedRect = *srcRect;        
    src->RotateRectl(&rotatedRect);
    RectlToC2DRect(&rotatedRect, srcSurfRect);    
    C2D_CALL(c2dSetSrcRectangle(m_c2dCtx, &srcSurfRect));
    
    return C2D_STATUS_OK;
}

// Retrieves the actual color of a given color index from a lookup table
// and converts it. If no lookup table nor conversion method exist the index
// is considered as a color and returned with no change.
COLOR DDIPU::GetConvertedColor(GPEBltParms* bltParms, COLOR index)
{
  COLOR lColor = index;

  if( bltParms->pLookup )
  {
    lColor = (bltParms->pLookup)[lColor];
  }

  if( bltParms->pConvert )
  {
    lColor = (bltParms->pColorConverter->*(bltParms->pConvert))(lColor);
  }
  return lColor;
}

void DDIPU::GetBrushOffset(GPEBltParms* bltParms, int& x, int& y)
{    
    int tmp;
    switch (bltParms->pDst->Rotate())
    {
    case DMDO_270:
        tmp = y;
        y = x;
        //x = bltParms->pDst->Width() - tmp - bltParms->pBrush->Height();        
        x = bltParms->pDst->Height() - tmp - bltParms->pBrush->Height();        
        break;
    case DMDO_180:
        y = bltParms->pDst->Height() - y - bltParms->pBrush->Height();
        x = bltParms->pDst->Width()  - x - bltParms->pBrush->Width();             
        break;
    case DMDO_90:
        tmp = y;
        //y = bltParms->pDst->Height() - x - bltParms->pBrush->Width();
        y = bltParms->pDst->Width() - x - bltParms->pBrush->Width();
        x = tmp;
        break;
    }

    // C2D uses offset from destination rectangle's upper left corner.
    RECTL rotatedRect = *(bltParms->prclDst);
    bltParms->pDst->RotateRectl(&rotatedRect);
    x -= rotatedRect.left;
    y -= rotatedRect.top;
}    

C2D_STATUS DDIPU::SetBltParms(GPEBltParms* bltParms)
{
    C2D_STATUS status;        

    // ROP
    C2D_CALL(c2dSetRop(m_c2dCtx, bltParms->rop4));
    // Rotation
    C2D_CALL(SetSurfRotation(bltParms->pSrc, bltParms->pDst));

    // Stretch mode
    if (bltParms->bltFlags & BLT_STRETCH)
    {
        C2D_STRETCH_MODE mode;       
        if (bltParms->iMode == BILINEAR)
        {
            mode = C2D_STRETCH_BILINEAR_SAMPLING;            
        }
        else
        {
            mode = C2D_STRETCH_POINT_SAMPLING;                        
        }
        C2D_CALL(c2dSetStretchMode(m_c2dCtx, mode));
    }

    // Alpha blend
    int globalAlpha = 255;
    C2D_ALPHA_BLEND_MODE blendMode = C2D_ALPHA_BLEND_NONE;
    if (bltParms->bltFlags & BLT_ALPHABLEND)
    {
        globalAlpha = bltParms->blendFunction.SourceConstantAlpha;
        if (bltParms->blendFunction.AlphaFormat == AC_SRC_ALPHA && bltParms->blendFunction.BlendOp == AC_SRC_OVER)
        {
            blendMode = C2D_ALPHA_BLEND_SRCOVER;
        }
    }
    C2D_CALL(c2dSetGlobalAlpha(m_c2dCtx, globalAlpha));
    C2D_CALL(c2dSetBlendMode(m_c2dCtx, blendMode));

    // Mask
    if (bltParms->pMask)
    {
        DDIPUSurf* mask;
        
        // If mask is in system memory, we need to copy it to video memory.
        if (!bltParms->pMask->InVideoMemory())
        {
            if (CopyToVideoMemory(bltParms->pMask, &mask) != S_OK)
            {
                return C2D_STATUS_FAILURE;
            }
            // Delete temporary surface before using, v2vblt maybe called more than one time before bltcomplete
            if (m_TempSurf)
            {
                delete m_TempSurf;
            }
            m_TempSurf = mask;
        }
        else
        {
            mask = (DDIPUSurf*)bltParms->pMask;
        }        
        
        if (bltParms->prclMask)
        {
            
            RECTL rotatedRect = *(bltParms->prclMask);
            bltParms->pMask->RotateRectl(&rotatedRect);        
            C2D_POINT maskPoint = {rotatedRect.left, rotatedRect.top};
            C2D_CALL(c2dSetMaskSurface(m_c2dCtx, mask->GetC2DSurface(), &maskPoint));
        }
        else
        {
            C2D_CALL(c2dSetMaskSurface(m_c2dCtx, mask->GetC2DSurface(), NULL));
        }
        
    }
    else
    {
        c2dSetMaskSurface(m_c2dCtx, NULL, NULL);
    }
    // Transparency
    if (bltParms->bltFlags & BLT_TRANSPARENT)
    {
        // The DrvTransparentBlt & AnyBlt we use sets the solidColor member as the 
        // transparent color.
        COLOR colorKey = GetConvertedColor(bltParms, bltParms->solidColor);
        C2D_CALL(c2dSetSrcColorkey(m_c2dCtx, static_cast<unsigned int>(colorKey), 1))
    }
    else
    {
        C2D_CALL(c2dSetSrcColorkey(m_c2dCtx, 0, 0));
    }

    return C2D_STATUS_OK;
}

C2D_STATUS DDIPU::SetSurfRotation(GPESurf* src, GPESurf* dst)
{
    C2D_STATUS status;
    if (src)
    {
        C2D_CALL(c2dSetSrcRotate(m_c2dCtx, GPEToC2DRotation[src->Rotate()]));
    }
    if (dst)
    {
        C2D_CALL(c2dSetDstRotate(m_c2dCtx, GPEToC2DRotation[dst->Rotate()]));
    }
    return C2D_STATUS_OK;
}

// Copies data from RECTL to C2D_RECT.
void DDIPU::RectlToC2DRect(RECTL* rect, C2D_RECT& c2dRect)
{ 
    c2dRect.height = rect->bottom - rect->top;
    c2dRect.width = rect->right - rect->left;
    c2dRect.x  = rect->left;
    c2dRect.y  = rect->top;        
}

// Returns true if there is acceleration support for BLT defined
// in bltParms. 
// TODO: Update this method when C2D supports more features.
bool DDIPU::IsBltSupported(GPEBltParms* bltParms)
{    
    if((!(bltParms->bltFlags & BLT_STRETCH )) 
        && (bltParms->pSrc->Rotate() == bltParms->pDst->Rotate())
        && ((m_c2dFlag & 0x4)== 0))
    {
        //When the 4th bit of m_c2dFlag is not set, simple srccopy will be disabled.
        return FALSE;
    }
    if((bltParms->bltFlags & BLT_STRETCH ) 
        &&((m_c2dFlag & 0x20)== 0))
    {
        return FALSE;
    }

    //For ddraw stretch blt, the iMode will be 0. Replace it to bilinear stretch blt.
    if(bltParms->iMode == 0)
    {
        bltParms->iMode = BILINEAR;
    }

    if ((bltParms->bltFlags & BLT_STRETCH) && !(bltParms->iMode == COLORONCOLOR || bltParms->iMode == BILINEAR))
    {
        return FALSE;
    }
    if (bltParms->pSrc)
    {
        if(!IsFormatSupported(bltParms->pSrc->Format()))
            return FALSE;

        // trying 24bpp gdi acceleration via z430 c2d
        // ONLY when both src & dest are 24bpp
        if((bltParms->pSrc->Format() == gpe24Bpp) && (bltParms->pDst->Format() != gpe24Bpp))
            return FALSE;

        if(bltParms->pSrc->InVideoMemory())
        {
            //All video memory surface is ddipu surface.
            //We don't support YV12 and NV12 surface blt.
            if(((DDIPUSurf *)bltParms->pSrc)->PixelFormat() > ddgpePixelFormat_YUY2)
                return FALSE;
        }
    }
    
    if (!(bltParms->xPositive > 0 && bltParms->yPositive > 0))
    {
        return false;
    }

    // Rotation of mask and source should be same (0).
    // MaskBlt documentation in MSDN:
    //    If a rotation or shear transformation is in effect for the source 
    //    device context when this function is called, an error occurs. 
    //    However, other types of transformation are allowed.
    if (bltParms->pMask)
    {
//&&&&&&&: enable mask path
//#if 1
#if 0    
        // TODO: enable mask after C2D mask blt is fixed
        return false;
#else
        if ((bltParms->pMask->Format() != gpe1Bpp) || 
            (bltParms->pSrc && (bltParms->pMask->Rotate() != bltParms->pSrc->Rotate())))
        {
            return false;
        }
#endif        
    }

    // The DrvTransparentBlt & AnyBlt we use never sets BLT_DSTTRANSPARENT flag.
    if (bltParms->bltFlags & BLT_DSTTRANSPARENT)
    {
        return false;
    }
    
    else if (IsAMirroredRect(bltParms->prclDst))
    {
        // TODO: Improperly ordered prclDst resulting from stretch Blt (mirror?) 
        // currently not accelerated (only prclDst is ever improperly ordered?)
        return false;
    }
    
    // No color palette nor conversion
    else if(bltParms->pLookup != 0 || bltParms->pConvert != 0)
    {
        // Color conversion currently not supported by C2D
        return false;
    }        
    return true;
}

// Returns true if pRect is mirrored (left > right or top > bottom)
bool DDIPU::IsAMirroredRect(RECTL* rect)
{
  if (rect)
  {
    if ((rect->left > rect->right) || (rect->top > rect->bottom))
    {
        return true;
    }
    return false;
  }
  return true;
}


bool DDIPU::IsSimpleCopyBlt(GPEBltParms* bltParms)
{

    bool standard = ((bltParms->xPositive == 1) && 
                     (bltParms->yPositive == 1));    
    
    bool rotate = (bltParms->pDst->IsRotate() || 
                   (bltParms->pSrc && bltParms->pSrc->IsRotate())); 
    
    //Exclude bltflag BLT_WAITVSYNC and BLT_WAITNOTBUSY            
    if (bltParms->rop4 != 0xCCCC || (bltParms->bltFlags & 0x3ff) != 0 || 
        !standard || rotate)
    {
        return false;
    }    
    return true;
}

void DDIPU::AdvancedSourceCheck(GPEBltParms * pBltParms)
{
    GPESurf* pSrc = pBltParms->pSrc; 
    C2D_COLORFORMAT format = C2D_COLOR_DUMMY;
    GPEFormat       *formatPtr = NULL;
    unsigned int    nOffsetInVideoMemory = 0;

    //
    //  Before we give up entirely, try to determine if the source is something that we can handle.   If it is, 
    //  wrap a DDIPUSurf around it and continue.
    //
    //  Check our stride to be sure this is something we can handle.
    //
    if ((int)pSrc->Stride() < 0)
        return;
    //
    //  If the source format is not 16 or 32 bpp, then we're not going to try handling this.
    //
    if ((pSrc->Format() != gpe16Bpp) && (pSrc->Format() != gpe32Bpp))
        return;

    //
    //  Check the format.  We're attempting to convert a GPE generic and mask to a C2D format.
    //
    formatPtr = pSrc->FormatPtr();
    if (formatPtr)
    {
        if (formatPtr->m_pPalette)
        {
            switch(formatPtr->m_pPalette[0])
            {
                case    0x000000FF:
                    format = C2D_COLOR_8888_ABGR;
                    break;
                case    0x0000000F:
                    format = C2D_COLOR_4444_RGBA;
                    break;
                case    0x0000F000:
                    format = C2D_COLOR_4444;
                    break;
                case    0x00007C00:
                    format = C2D_COLOR_1555;
                    break;
                case    0x0000F800:
                    format = C2D_COLOR_0565;
                    if (formatPtr->m_pPalette[1] == 0x000007C0)
                        format = C2D_COLOR_1555;
                    break;
                case    0x00FF0000:
                    format = C2D_COLOR_8888_RGBA;
                    //
                    //  Special case for D3DM surfaces.
                    //
                    if (formatPtr->m_PaletteEntries == 4)
                        format = C2D_COLOR_8888;
                    break;
            }
            //
            //  If we still don't have something that we recognize, break down to our fallback path.
            //
            if (format == C2D_COLOR_DUMMY)
                return;
        }
        else
            return;
    }
    else
        return;

    nOffsetInVideoMemory = pSrc->OffsetInVideoMemory();
    //
    //  If a video memory offset is supplied, it must be 4K aligned.
    //
    if (nOffsetInVideoMemory & 0xFFF)
        return;
    //
    //  If a video memory offset is not supplied, get one, then check it for alignment.
    //
    if (!nOffsetInVideoMemory)
    {
        c2dTranslatePhysaddr(pSrc->Buffer(), &nOffsetInVideoMemory);
        if (nOffsetInVideoMemory & 0xFFF)
            return;
    }

    //
    //  At this point we should have enough info to perform a vidmem to vidmem blt.  We'll modify the
    //  bltParms to contain a properly modified DDIPUSurf.
    //
    pBltParms->bltFlags |= 0x80000000;       //  Tweak the flags so we can know that we have an allocated surface that needs freeing in BltComplete.

    GPESurf *newSurface;
    AllocSurface(&newSurface, pSrc->Width(), pSrc->Height(), pSrc->Stride(), (unsigned int) pSrc->Buffer(), nOffsetInVideoMemory, pSrc->Format(), format);
    pBltParms->pBlt = (GPEBltFn)&DDIPU::VidmemToVidmemBlt;
    pBltParms->pSrc = newSurface;
}


#endif

//------------------------------------------------------------------------------
//
// Function: BltPrepare
//
// This method identifies the appropriate functions
// needed to perform individual blits.
//
// Parameters:
//      pBltParms
//          [in] A pointer to a DDGPEBltParms structure.
//
// Returns:
//      S_OK            successful
//      others          failed
//
//------------------------------------------------------------------------------
SCODE DDIPU::BltPrepare(GPEBltParms * pBltParms)
{
    EnterCriticalSection(&m_csDrawLock);

    DispPerfStart(pBltParms->rop4);

    // default to base EmulatedBlt routine
    pBltParms->pBlt = (SCODE (GPE::*)(GPEBltParms *))(&DDIPU::WrapEmulatedBlt);

    // see if we need to deal with cursor
    PointerBltPrepare(pBltParms);

#if defined(USE_C2D_ROUTINES)
    GPESurf* pDst = pBltParms->pDst;
    GPESurf* pSrc = pBltParms->pSrc; 

    if(pDst->InVideoMemory() && IsFormatSupported(pDst->Format()))
    {
        if ((m_c2dFlag & 0x100)== 0) // registry flag is to enable acceleration
        {
            if (m_iRotate)
                return S_OK;   // &&&&&&&: accleration is NOT ready when rotated.
        }
        switch (pBltParms->rop4)
        {
            case 0x0000:    // Blackness
                            // intentional fall
            case 0xFFFF:    // Whiteness
                if ((m_c2dFlag & 0x1)!= 0) // registry flag is to enable acceleration            
                    pBltParms->pBlt = (GPEBltFn)&DDIPU::SolidColorFill;
                break;
            case 0xF0F0:    // Pattern copy
                if (pBltParms->solidColor != -1)
                {
                    if ((m_c2dFlag & 0x1)!= 0) // registry flag is to enable acceleration
                        pBltParms->pBlt = (GPEBltFn)&DDIPU::SolidColorFill;
                }
                else if (pBltParms->pBrush && IsFormatSupported(pBltParms->pBrush->Format()) && !pBltParms->pBrush->IsRotate())
                {
                    if ((m_c2dFlag & 0x2)!= 0) // registry flag is to enable acceleration
                        pBltParms->pBlt = (GPEBltFn)&DDIPU::PatternFill;
                }
                break;

            case 0xAACC:    // Source copy with mask
               if (IsBltSupported(pBltParms) && ((m_c2dFlag & 0x10)!= 0))
                {
                    // Source surface in system memory. 
                    if(pSrc && !(pSrc->InVideoMemory()))
                    {
                        // Use sw in simple sysmem to vidmem srccopy blt.
                        if (!IsSimpleCopyBlt(pBltParms))
                        {                
                            pBltParms->pBlt = (GPEBltFn)&DDIPU::SysmemToVidmemBlt;
                        }                        
                    }           
                    // Source surface in video memory
                    else if (pSrc)
                    {                   
                        pBltParms->pBlt = (GPEBltFn)&DDIPU::VidmemToVidmemBlt;
                    }
                }
                break;            
                
            case 0xCCCC:    // Source copy
               if (IsBltSupported(pBltParms) && ((m_c2dFlag & 0x24)!= 0))
                {
                    // Source surface in system memory. 
                    if(pSrc && !(pSrc->InVideoMemory()))
                    {
                        // Use sw in simple sysmem to vidmem srccopy blt.
                        if (!IsSimpleCopyBlt(pBltParms))
                        {                
                            pBltParms->pBlt = (GPEBltFn)&DDIPU::SysmemToVidmemBlt;
                            //
                            //  Try to setup the blt to avoid the copy
                            //
                //&&&&&&&: comment this path
  //                          AdvancedSourceCheck(pBltParms);
                        }                        
                    }           
                    // Source surface in video memory
                    else if (pSrc)
                    {                   
                        pBltParms->pBlt = (GPEBltFn)&DDIPU::VidmemToVidmemBlt;
                    }
                }
                else if((m_c2dFlag & 0x24)!= 0)
                {
                //&&&&&&&: comment this path
//                    AdvancedSourceCheck(pBltParms);
                }
                break;
            default:
                int i;    // just a debug breakpoint placeholder.
                i = 0;
                break;
        }
    }
#endif

    // Performance Logging Type
    if (pBltParms->pBlt != (SCODE (GPE::*)(GPEBltParms *))(&DDIPU::WrapEmulatedBlt))
    {
        DispPerfType(DISPPERF_ACCEL_HARDWARE);
    }

    return S_OK;
}

SCODE DDIPU::WrapEmulatedBlt(GPEBltParms *blitParameters)
{
    SCODE ret = S_OK;

    if(blitParameters->bltFlags & BLT_WAITVSYNC)
    {
        if(blitParameters->pDst == m_pActiveSurface)
            DisplayWaitForVSync(FALSE);
        else if(blitParameters->pDst == m_pPrimarySurface2)
            DisplayWaitForVSync(TRUE);
    }
    // fall back on compatible blt
    if (m_iRotate && ((blitParameters->pDst == m_pActiveSurface) ||    // only care if dest is main display surface
         (blitParameters->pSrc == m_pActiveSurface)))    // only care if src is main display surface
    {
        ret = EmulatedBltRotate(blitParameters);
    }
    else
    {
        ret = EmulatedBlt(blitParameters);
    }

    // Update dirty rectangle with new dirty regions.
    // This will be used if we have an ASync driver (to trigger a partial refresh)
    // or if we are using TV out.
    if((blitParameters->pDst == m_pActiveSurface) ||    // only care if dest is main display surface
         (blitParameters->pSrc == m_pActiveSurface))    // only care if src is main display surface
    {
        // Now, calculate the dirty-rect to refresh to the actual hardware
        RECT bounds;

        bounds.left     = blitParameters->prclDst->left;
        bounds.top      = blitParameters->prclDst->top;
        bounds.right    = blitParameters->prclDst->right;
        bounds.bottom   = blitParameters->prclDst->bottom;

        if(bounds.left > bounds.right) {
            SWAP(bounds.left,bounds.right, int)
        }
        if( bounds.top > bounds.bottom) {
            SWAP(bounds.top,bounds.bottom, int)
        }

        // If boundaries are screwy, just make boundaries as large as possible to be safe
        if((bounds.left < 0) || (bounds.left > (m_rcWorkRect.right - m_rcWorkRect.left)))
            bounds.left = 0;

        if((bounds.right < 0) || (bounds.right > (m_rcWorkRect.right - m_rcWorkRect.left)))
            bounds.right = (m_rcWorkRect.right - m_rcWorkRect.left);

        if((bounds.top < 0) || (bounds.top > (m_rcWorkRect.bottom - m_rcWorkRect.top)))
            bounds.top = 0;

        if((bounds.bottom < 0) || (bounds.bottom > (m_rcWorkRect.bottom - m_rcWorkRect.top)))
            bounds.bottom = (m_rcWorkRect.bottom - m_rcWorkRect.top);

        DisplayUpdate((LPRECT)&bounds);
    }

    return ret;
}

//------------------------------------------------------------------------------
//
// Function: BltComplete
//
// This method executes to complete
// a blit sequence initiated by GPE::BltPrepare
//
// Parameters:
//      pBltParms
//          [in] A pointer to a DDGPEBltParms structure.
//
// Returns:
//      S_OK            successful
//      others          failed
//
//------------------------------------------------------------------------------
SCODE DDIPU::BltComplete(GPEBltParms * pBltParms)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pBltParms);
#if defined(USE_C2D_ROUTINES)
    //
    //  We had to hack to get an accelerated blt.  Delete the allocated surface.
    //
    if (pBltParms->bltFlags & 0x80000000)
    {
        delete pBltParms->pSrc;
    }
    // Delete temporary surface
    if (m_TempSurf)
    {
        delete m_TempSurf;
        m_TempSurf = NULL;
    }

#endif

    // see if cursor was forced off because of overlap with source or destination and turn back on
    if (m_CursorForcedOff)
    {
        m_CursorForcedOff = FALSE;
        CursorOn();
    }

    DispPerfEnd(0);

    LeaveCriticalSection(&m_csDrawLock);

    return S_OK;
}
