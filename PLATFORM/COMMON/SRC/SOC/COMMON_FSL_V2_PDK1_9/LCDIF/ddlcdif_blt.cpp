//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
// File:        ddlcdif_blt.cpp
//
// bitblt/rectangle for LCDIF
//
//------------------------------------------------------------------------------

#include "precomp.h"
#include "dispperf.h"

//------------------------------------------------------------------------------
// External Functions


//------------------------------------------------------------------------------
// External Variables
extern HANDLE m_hCombSurfNeedUdEvent;

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
SCODE DDLcdif::BltPrepare(GPEBltParms * pBltParms)
{
    DispPerfStart(pBltParms->rop4);

    // default to base EmulatedBlt routine
    pBltParms->pBlt = &GPE::EmulatedBlt;

    // see if we need to deal with cursor
    PointerBltPrepare(pBltParms);

    if (m_iRotate && ((pBltParms->pSrc == m_pBackgroundSurface) || (pBltParms->pDst == m_pBackgroundSurface)))
        pBltParms->pBlt = (SCODE (GPE::*)(GPEBltParms *))&GPE::EmulatedBltRotate;

#ifdef USE_DCP
    if ((pBltParms->pSrc && !pBltParms->pSrc->InVideoMemory()) ||
        (pBltParms->pDst && !pBltParms->pDst->InVideoMemory()))
        return S_OK;

    if (m_iRotate) 
        return S_OK;

    if ((pBltParms->pLookup) ||
        (pBltParms->pConvert) ||
        (pBltParms->bltFlags & (BLT_STRETCH |
        BLT_ALPHABLEND |
        BLT_TRANSPARENT)))
    {
        return S_OK;
    }

    DDLcdifSurf* dst_surf = (DDLcdifSurf*) pBltParms->pDst;
    DDLcdifSurf* src_surf = (DDLcdifSurf*) pBltParms->pSrc;

    switch (pBltParms->rop4) {
    case 0x0000: // Fill Black
        pBltParms->solidColor = 0;
        pBltParms->pBlt = (SCODE (GPE::*)(struct GPEBltParms*)) &DDLcdif::AcceleratedFillRect;
        break;
    case 0xFFFF: // Fill White
        pBltParms->solidColor = 0xFFFFFFFF;
        pBltParms->pBlt = (SCODE (GPE::*)(struct GPEBltParms*)) &DDLcdif::AcceleratedFillRect;
        break;       
    case 0xF0F0:
        if(!dst_surf) break;    //Just to remove prefast warning
        if (pBltParms->solidColor == -1) break;
        switch (dst_surf->Format()) {
        case gpe16Bpp:
            break;
        case gpe32Bpp:
            break;
        default:
            return S_OK;
        }//switch
        pBltParms->pBlt = (SCODE (GPE::*)(struct GPEBltParms*)) &DDLcdif::AcceleratedFillRect;
        break;

    case 0xCCCC:
        if(!(dst_surf && src_surf)) break;      //Just to remove prefast warning
        if (dst_surf->Format() != src_surf->Format()) break;
        pBltParms->pBlt = (SCODE (GPE::*)(struct GPEBltParms*)) &DDLcdif::AcceleratedSrcCopy;

        break;

    default:
        break;
    }
#endif

    return S_OK;
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
SCODE DDLcdif::BltComplete(GPEBltParms * pBltParms)
{
    UNREFERENCED_PARAMETER(pBltParms);

#ifdef USE_DCP
    dcp_WaitForComplete(blit_handle, INFINITE);
#endif

    DispPerfEnd(0);

    // see if cursor was forced off because of overlap with source or destination and turn back on
    if (m_CursorForcedOff)
    {
        m_CursorForcedOff = FALSE;
        CursorOn();
    }

    if(m_bManualCombine && m_pCombinedSurface &&(m_hPXP != NULL)&&(m_hPXP != INVALID_HANDLE_VALUE))
    {
        SetEvent(m_hCombSurfNeedUdEvent);
    }

    return S_OK;
}

#ifdef USE_DCP
//------------------------------------------------------------------------------
//
// Function: AcceleratedFillRect
//
// This method executes to accelerate
// solid fill using DCP
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
SCODE DDLcdif::AcceleratedFillRect(struct GPEBltParms* BltParams)
{
    DDLcdifSurf* dst_surf = (DDLcdifSurf*) BltParams->pDst;

    int stride = dst_surf->Stride();

    int width = BltParams->prclDst->right - BltParams->prclDst->left;
    int height = BltParams->prclDst->bottom - BltParams->prclDst->top;

    int offset_bytes;

    RETAILMSG(0, (L"addr [0x%08X], Bpp [%d], color [0x%08X], x [%d], y [%d], width [%d], height [%d], stride [%d]\r\n",
        dst_surf->Buffer(),
        dst_surf->BytesPerPixel(),
        BltParams->solidColor,
        BltParams->prclDst->left, BltParams->prclDst->top,
        width, height, stride));

    /* \todo WBB: Add negative x/y handling for dst */

    // DCP expects width to be in bytes!
    width *= dst_surf->BytesPerPixel();

    if(width * height < 51200)  //DCP just has better performance in big block operation than software processing.
        return (m_iRotate) ? EmulatedBltRotate(BltParams) : EmulatedBlt(BltParams);

    if(0xF0F0 == BltParams->rop4)
    {
        if (gpe16Bpp == dst_surf->Format() && width % 4)
        { 
            return (m_iRotate) ? EmulatedBltRotate(BltParams) : EmulatedBlt(BltParams);            
        }

        if(gpe16Bpp == dst_surf->Format())
            BltParams->solidColor |= (BltParams->solidColor << 16);

    }

    offset_bytes = (stride * BltParams->prclDst->top);
    offset_bytes += BltParams->prclDst->left * dst_surf->BytesPerPixel();

    dcp_WaitForComplete(blit_handle, INFINITE);    
    dcp_bltfill(BltParams->solidColor,
        (void*) (m_nLAWPhysical + dst_surf->OffsetInVideoMemory() + offset_bytes),
        (void*) ((UINT32) dst_surf->Buffer() + offset_bytes),
        width, height, stride,
        NULL, NULL, &blit_handle);


    return S_OK;
}//AcceleratedFillRect

//------------------------------------------------------------------------------
//
// Function: AcceleratedSrcCopy
//
// This method executes to accelerate
// source copy using DCP
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
SCODE DDLcdif::AcceleratedSrcCopy(struct GPEBltParms* BltParams)
{
    DDLcdifSurf* dst_surf = (DDLcdifSurf*) BltParams->pDst;
    DDLcdifSurf* src_surf = (DDLcdifSurf*) BltParams->pSrc;

    int dst_stride = dst_surf->Stride();
    int src_stride = src_surf->Stride();

    int width = BltParams->prclSrc->right - BltParams->prclSrc->left;
    int height = BltParams->prclSrc->bottom - BltParams->prclSrc->top;

    int src_offset_bytes, dst_offset_bytes;

    RETAILMSG(0, (L"src addr [0x%08X], x [%d], y [%d], width [%d], height [%d], stride [%d]\r\n",
        src_surf->Buffer(),
        BltParams->prclSrc->left, BltParams->prclSrc->top,
        width, height, src_stride));
    RETAILMSG(0, (L"dst addr [0x%08X], x [%d], y [%d], width [%d], height [%d], stride [%d]\r\n",
        dst_surf->Buffer(),
        BltParams->prclDst->left, BltParams->prclDst->top,
        BltParams->prclDst->right - BltParams->prclDst->left,
        BltParams->prclDst->bottom - BltParams->prclDst->top,
        dst_stride));

    /* \todo WBB: Add negative x/y handling for src/dst */

    // DCP expects width to be in bytes!
    width *= src_surf->BytesPerPixel();

    if(width * height < 115200) //DCP just has better performance in big block operation than software processing.
        return (m_iRotate) ? EmulatedBltRotate(BltParams) : EmulatedBlt(BltParams);

    /* The DCP cannot handle a non-contiguous source buffer (i.e. the
    * width must be equal to the stride).  This check should really be
    * made in the BltPrepare routine however WinCE seems to lie to that
    * function.  It tells BltPrepare that the width is equal to the stride,
    * but then calls the actual srccopy routine with a different width
    * height.  This may seem somewhat logical (though not pretty) 
    * in that it calls the srccopy routine multiple times with the 
    * same buffer and only one BltPrepare call.
    */

    if (width != src_stride)
        return (m_iRotate) ? EmulatedBltRotate(BltParams) : EmulatedBlt(BltParams);

    src_offset_bytes  = (src_stride * BltParams->prclSrc->top);
    src_offset_bytes += (BltParams->prclSrc->left * src_surf->BytesPerPixel());

    dst_offset_bytes  = (dst_stride * BltParams->prclDst->top);
    dst_offset_bytes += (BltParams->prclDst->left * dst_surf->BytesPerPixel());

    dcp_WaitForComplete(blit_handle, INFINITE);
    dcp_blt((void*) (m_nLAWPhysical + src_surf->OffsetInVideoMemory() + src_offset_bytes),
        (void*) (m_nLAWPhysical + dst_surf->OffsetInVideoMemory() + dst_offset_bytes),
        width, height, dst_stride,
        NULL, NULL, &blit_handle);
    

    return S_OK;
}//AcceleratedSrcCopy
#endif
