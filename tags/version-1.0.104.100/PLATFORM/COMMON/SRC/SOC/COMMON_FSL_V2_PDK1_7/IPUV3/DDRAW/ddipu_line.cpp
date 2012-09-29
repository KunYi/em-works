//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  ddipu_line.cpp
//
//  Implementation of DDIPU functions to draw lines.
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
// Function: Line
//
// This method executes before and after a sequence of line segments,
// which are drawn as a path.
//
// Parameters:
//      pLineParms
//          [in] A pointer to a DDGPELineParms structure.
//
//      phase
//          [in] Set to one of the values in the following table.
//
// Returns:
//      S_OK            successful
//      others          failed
//
//------------------------------------------------------------------------------
SCODE DDIPU::Line(GPELineParms * pLineParms, EGPEPhase phase)
{
    EnterCriticalSection(&m_csDrawLock);

    if (phase == gpeSingle || phase == gpePrepare)
    {
        DispPerfStart(ROP_LINE);

        if ((pLineParms->pDst != m_pActiveSurface))
        {
            pLineParms->pLine = &DDIPU::EmulatedLine;
        }
        else
        {
            pLineParms->pLine = (SCODE (GPE::*)(struct GPELineParms *))(&DDIPU::WrappedEmulatedLine);
        }

#if defined(USE_C2D_ROUTINES)
    
        if ((m_c2dFlag & 0x8) != 0)//registry flag is to enable acceleration
        // solid line
        if (pLineParms->pDst->InVideoMemory() && 
            IsFormatSupported(pLineParms->pDst->Format()) && 
            (pLineParms->mix == 0x0D0D))
        {
            pLineParms->pLine = (GPELineFn)&DDIPU::SolidLine;
            DispPerfType(DISPPERF_ACCEL_HARDWARE);
        }  
#endif
    }    
    else if (phase == gpeComplete)
    {
        DispPerfEnd (0);
    }

    LeaveCriticalSection(&m_csDrawLock);
    return S_OK;
}


//------------------------------------------------------------------------------
//
// Function: WrappedEmulatedLine
//
// This method executes the emulated line routine and modified the
// dirty rectangle if we are in TV mode.
//
// Parameters:
//      pBltParms
//          [in] A pointer to a DDGPELineParms structure.
//
// Returns:
//      S_OK            successful
//      others          failed
//
//------------------------------------------------------------------------------
SCODE DDIPU::WrappedEmulatedLine(GPELineParms *pLineParms)
{
    SCODE retval;

    // Update dirty rectangle with new dirty regions.

    // Now, calculate the dirty-rect to refresh to the actual hardware
    RECT bounds;
    int N_plus_1;           // Minor length of bounding rect + 1

    if( pLineParms->dN) // The line has a diagonal component (we'll refresh the bounding rect)
        N_plus_1 = 2 + ((pLineParms->cPels * pLineParms->dN) / pLineParms->dM);
    else
        N_plus_1 = 1;

    switch(pLineParms->iDir) {
    case 0:
        bounds.left = pLineParms->xStart;
        bounds.top = pLineParms->yStart;
        bounds.right = pLineParms->xStart + pLineParms->cPels;
        bounds.bottom = bounds.top + N_plus_1;
        break;
    case 1:
        bounds.left = pLineParms->xStart;
        bounds.top = pLineParms->yStart;
        bounds.bottom = pLineParms->yStart + pLineParms->cPels;
        bounds.right = bounds.left + N_plus_1;
        break;
    case 2:
        bounds.right = pLineParms->xStart + 1;
        bounds.top = pLineParms->yStart;
        bounds.bottom = pLineParms->yStart + pLineParms->cPels;
        bounds.left = bounds.right - N_plus_1;
        break;
    case 3:
        bounds.right = pLineParms->xStart + 1;
        bounds.top = pLineParms->yStart;
        bounds.left = pLineParms->xStart - pLineParms->cPels;
        bounds.bottom = bounds.top + N_plus_1;
        break;
    case 4:
        bounds.right = pLineParms->xStart + 1;
        bounds.bottom = pLineParms->yStart + 1;
        bounds.left = pLineParms->xStart - pLineParms->cPels;
        bounds.top = bounds.bottom - N_plus_1;
        break;
    case 5:
        bounds.right = pLineParms->xStart + 1;
        bounds.bottom = pLineParms->yStart + 1;
        bounds.top = pLineParms->yStart - pLineParms->cPels;
        bounds.left = bounds.right - N_plus_1;
        break;
    case 6:
        bounds.left = pLineParms->xStart;
        bounds.bottom = pLineParms->yStart + 1;
        bounds.top = pLineParms->yStart - pLineParms->cPels;
        bounds.right = bounds.left + N_plus_1;
        break;
    case 7:
        bounds.left = pLineParms->xStart;
        bounds.bottom = pLineParms->yStart + 1;
        bounds.right = pLineParms->xStart + pLineParms->cPels;
        bounds.top = bounds.bottom - N_plus_1;
        break;
    default:
        DEBUGMSG(GPE_ZONE_ERROR,(TEXT("Invalid direction: %d\r\n"),pLineParms->iDir));
        return E_INVALIDARG;
    }

    // If line overlap cursor, turn if off
    RECTL cursorRect = m_CursorRect;
    // Convert from rectangle defined in rotated perspective to
    // rectangle from display panel perspective, as the line bounds uses panel perspective
    RotateRectl (&cursorRect);
    // If cursor is on check for line overlap
    if (m_CursorVisible && !m_CursorDisabled)
    {
        if (cursorRect.top <= bounds.bottom && cursorRect.bottom >= bounds.top &&
            cursorRect.left <= bounds.right && cursorRect.right >= bounds.left)
        {
            CursorOff();
            m_CursorForcedOff = TRUE;
        }
    }

    // do emulated line
    retval = EmulatedLine (pLineParms);

    // If cursor was forced off turn it back on
    if (m_CursorForcedOff) {
        m_CursorForcedOff = FALSE;
        CursorOn();
    }

    if (FAILED(retval))
        return retval;

    //for rotate the display panel coordinate to rotated coordinate, 
    //as following operation is according to rotated coordinate
    LONG temp1,temp2;
    switch (m_iRotate)
    {
        case DMDO_0:
            // No changes necessary...already in panel perspective
            break;
        case DMDO_90:
            temp1 = bounds.top;
            temp2 = bounds.bottom;
            bounds.top = bounds.left;
            bounds.bottom = bounds.right;
            bounds.left = m_nScreenWidth - 1  - temp2;
            bounds.right = m_nScreenWidth - 1 - temp1;
            break;
        case DMDO_270:
            temp1 = bounds.left;
            temp2 = bounds.right;
            bounds.left = bounds.top;
            bounds.right = bounds.bottom;
            bounds.top = m_nScreenHeight - 1  - temp2;
            bounds.bottom = m_nScreenHeight - 1 - temp1;
            break;
        case DMDO_180:
            temp1 = bounds.top;
            bounds.top = m_nScreenHeight - 1 - bounds.bottom;
            bounds.bottom = m_nScreenHeight - 1 - temp1;
            temp2 = bounds.left;
            bounds.left = m_nScreenWidth - 1 - bounds.right;
            bounds.right = m_nScreenWidth - 1 - temp2;
            break;
        default:
            break;
    }

    DisplayUpdate((LPRECT)&bounds);

    return S_OK;
}

#if defined(USE_C2D_ROUTINES)
SCODE DDIPU::SolidLine(GPELineParms* lineParms)
{
    SCODE retval;

    RECT bounds = {0};
    int N_plus_1;           // Minor length of bounding rect + 1

    if (lineParms->pDst == m_pActiveSurface)
    {
        // Update dirty rectangle with new dirty regions.

        // Now, calculate the dirty-rect to refresh to the actual hardware
        if( lineParms->dN) // The line has a diagonal component (we'll refresh the bounding rect)
            N_plus_1 = 2 + ((lineParms->cPels * lineParms->dN) / lineParms->dM);
        else
            N_plus_1 = 1;

        switch(lineParms->iDir) {
        case 0:
            bounds.left = lineParms->xStart;
            bounds.top = lineParms->yStart;
            bounds.right = lineParms->xStart + lineParms->cPels;
            bounds.bottom = bounds.top + N_plus_1;
            break;
        case 1:
            bounds.left = lineParms->xStart;
            bounds.top = lineParms->yStart;
            bounds.bottom = lineParms->yStart + lineParms->cPels;
            bounds.right = bounds.left + N_plus_1;
            break;
        case 2:
            bounds.right = lineParms->xStart + 1;
            bounds.top = lineParms->yStart;
            bounds.bottom = lineParms->yStart + lineParms->cPels;
            bounds.left = bounds.right - N_plus_1;
            break;
        case 3:
            bounds.right = lineParms->xStart + 1;
            bounds.top = lineParms->yStart;
            bounds.left = lineParms->xStart - lineParms->cPels;
            bounds.bottom = bounds.top + N_plus_1;
            break;
        case 4:
            bounds.right = lineParms->xStart + 1;
            bounds.bottom = lineParms->yStart + 1;
            bounds.left = lineParms->xStart - lineParms->cPels;
            bounds.top = bounds.bottom - N_plus_1;
            break;
        case 5:
            bounds.right = lineParms->xStart + 1;
            bounds.bottom = lineParms->yStart + 1;
            bounds.top = lineParms->yStart - lineParms->cPels;
            bounds.left = bounds.right - N_plus_1;
            break;
        case 6:
            bounds.left = lineParms->xStart;
            bounds.bottom = lineParms->yStart + 1;
            bounds.top = lineParms->yStart - lineParms->cPels;
            bounds.right = bounds.left + N_plus_1;
            break;
        case 7:
            bounds.left = lineParms->xStart;
            bounds.bottom = lineParms->yStart + 1;
            bounds.right = lineParms->xStart + lineParms->cPels;
            bounds.top = bounds.bottom - N_plus_1;
            break;
        default:
            DEBUGMSG(GPE_ZONE_ERROR,(TEXT("Invalid direction: %d\r\n"),lineParms->iDir));
            return E_INVALIDARG;
        }

        // If line overlap cursor, turn if off
        RECTL cursorRect = m_CursorRect;
        // Convert from rectangle defined in rotated perspective to
        // rectangle from display panel perspective, as the line bounds uses panel perspective
        RotateRectl (&cursorRect);
        // If cursor is on check for line overlap
        if (m_CursorVisible && !m_CursorDisabled)
        {
            if (cursorRect.top <= bounds.bottom && cursorRect.bottom >= bounds.top &&
                cursorRect.left <= bounds.right && cursorRect.right >= bounds.left)
            {
                CursorOff();
                m_CursorForcedOff = TRUE;
            }
        }

    }
    
    // These functions don't consider rotation, it's handled in 
    // DrvStrokePath.
    if (lineParms->dN != 0)    
    {
        if(lineParms->llGamma == 0)  //z160_c2d hardware always assume llGamma =0
            retval = DrawLineDiag(lineParms);
        else
            retval = EmulatedLine(lineParms);
    }
    else
    {
        retval = DrawLineHorzVert(lineParms);
    }
    
    if (lineParms->pDst == m_pActiveSurface)
    {

        // If cursor was forced off turn it back on
        if (m_CursorForcedOff) {
            m_CursorForcedOff = FALSE;
            CursorOn();
        }

        if (FAILED(retval))
            return retval;

        //for rotate the display panel coordinate to rotated coordinate, 
        //as following operation is according to rotated coordinate
        LONG temp1,temp2;
        switch (m_iRotate)
        {
            case DMDO_0:
                // No changes necessary...already in panel perspective
                break;
            case DMDO_90:
                temp1 = bounds.top;
                temp2 = bounds.bottom;
                bounds.top = bounds.left;
                bounds.bottom = bounds.right;
                bounds.left = m_nScreenWidth - 1  - temp2;
                bounds.right = m_nScreenWidth - 1 - temp1;
                break;
            case DMDO_270:
                temp1 = bounds.left;
                temp2 = bounds.right;
                bounds.left = bounds.top;
                bounds.right = bounds.bottom;
                bounds.top = m_nScreenHeight - 1  - temp2;
                bounds.bottom = m_nScreenHeight - 1 - temp1;
                break;
            case DMDO_180:
                temp1 = bounds.top;
                bounds.top = m_nScreenHeight - 1 - bounds.bottom;
                bounds.bottom = m_nScreenHeight - 1 - temp1;
                temp2 = bounds.left;
                bounds.left = m_nScreenWidth - 1 - bounds.right;
                bounds.right = m_nScreenWidth - 1 - temp2;
                break;
            default:
                break;
        }

        DisplayUpdate((LPRECT)&bounds);
    }
    
    return retval;
}

SCODE DDIPU::DrawLineDiag(GPELineParms* lineParms)
{
    
    C2D_POINT startPoint = {lineParms->xStart, lineParms->yStart};
    C2D_POINT endPoint;

    int nPixels = (lineParms->cPels * lineParms->dN +lineParms->llGamma)/ lineParms->dM;
    int mPixels = lineParms->cPels;

    switch (lineParms->iDir & 0x07)
    {
    case 0:        
        endPoint.x      = startPoint.x + mPixels;
        endPoint.y      = startPoint.y + nPixels;

        break;
    case 1:
        endPoint.x      = startPoint.x + nPixels;
        endPoint.y      = startPoint.y + mPixels;
        break;        
    case 2:
        endPoint.x      = startPoint.x - nPixels;
        endPoint.y      = startPoint.y + mPixels;
        break;
    case 3:
        endPoint.x      = startPoint.x - mPixels;
        endPoint.y      = startPoint.y + nPixels;
        break;
    case 4:
        endPoint.x      = startPoint.x - mPixels;
        endPoint.y      = startPoint.y - nPixels;

        break;
    case 5:
        endPoint.x      = startPoint.x - nPixels;
        endPoint.y      = startPoint.y - mPixels;
        break;
    case 6:
        endPoint.x      = startPoint.x + nPixels;
        endPoint.y      = startPoint.y - mPixels;
        break;
    case 7:
        endPoint.x      = startPoint.x + mPixels;
        endPoint.y      = startPoint.y - nPixels;

        break;
    default:
        return E_FAIL;

    }
    return DrawLine(lineParms->pDst, lineParms->prclClip, lineParms->solidColor, &startPoint, &endPoint, FALSE);
//    return DrawLine(lineParms, &startPoint, &endPoint, &lineRect);
}


SCODE DDIPU::DrawLineHorzVert(GPELineParms* lineParms)
{
    C2D_POINT startPoint = {lineParms->xStart, lineParms->yStart};    
    C2D_POINT endPoint;
    int mPixels = lineParms->cPels;


    switch (lineParms->iDir & 0x07)
    {
        case 3:
        case 4:
            endPoint.x      = startPoint.x - mPixels ;       
            endPoint.y      = startPoint.y ;
            break;
        case 7: 
        case 0: 
            endPoint.x      = startPoint.x + mPixels ;
            endPoint.y      = startPoint.y ;
            break;
        case 5:
        case 6:
            endPoint.y      = startPoint.y - mPixels ;
            endPoint.x      = startPoint.x ;
            break;
        case 1:
        case 2:
            endPoint.y      = startPoint.y + mPixels ;
            endPoint.x      = startPoint.x ;
            break;

    default:
        return E_FAIL;
    
    }
    return DrawLine(lineParms->pDst, lineParms->prclClip, lineParms->solidColor, &startPoint, &endPoint, FALSE);
//    return DrawLine(lineParms, &startPoint, &endPoint, &lineRect);
}

SCODE DDIPU::DrawLine(  GPESurf*   pDst,
                        RECTL*     prclClip,
                        COLOR      solidColor,
                        C2D_POINT* p0, 
                        C2D_POINT* p1,
                        BOOL       bRotate)
{
    C2D_RECT clipRect;
    C2D_STATUS status;

    C2D_ENTER;

    ON_ERROR_EXIT_AND_LOG(c2dSetDstSurface(m_c2dCtx, ((DDIPUSurf *)pDst)->GetC2DSurface()),  L"DDIPU::DrawLine - Failed to set line destination.\r\n");

    if (prclClip)
    {
        RectlToC2DRect(prclClip, clipRect);
        ON_ERROR_EXIT_AND_LOG(c2dSetDstClipRect(m_c2dCtx, &clipRect), L"DDIPU::DrawLine - Failed to set clip rectangle.\r\n");
    }
    else
    {
        c2dSetDstClipRect(m_c2dCtx, 0);                              
    }

    if(!bRotate)
    {
        ON_ERROR_EXIT_AND_LOG(c2dSetDstRotate(m_c2dCtx, 0), L"DDIPU::DrawLine - Failed to zero rotation.\r\n");
    }

    ON_ERROR_EXIT_AND_LOG(c2dSetFgColor(m_c2dCtx, solidColor), L"DDIPU::DrawLine - Failed to set foreground color.\r\n");

    status = c2dDrawLine(m_c2dCtx, p0, p1, 0);

    if(status == C2D_STATUS_OK)
    {
        status = c2dFinish(m_c2dCtx);
    }
    C2D_EXIT;

    if (status != C2D_STATUS_OK)
    {
        ERRORMSG(1, (L"DDIPU::DrawLine - C2D line failed.\r\n"));
    }
    return C2DStatusToScode(status);
}

SCODE DDIPU::DrawLine(GPELineParms* lineParms, 
                       C2D_POINT*   startPoint, 
                       C2D_POINT*   endPoint,
                       C2D_RECT*    lineRect)
{
    (lineRect);
    return DrawLine(lineParms->pDst, lineParms->prclClip, lineParms->solidColor, startPoint, endPoint, FALSE);
}

extern PFN_DrvStrokePath g_DefaultStrokePath;
extern DDGPE* gGPE;

#define CLIP_LIMIT 50
typedef struct _CLIPENUM {
    LONG    c;
    RECTL   arcl[CLIP_LIMIT];   // Space for enumerating complex clipping
}CLIPENUM;                      // ce, pce
#endif //defined(USE_C2D_ROUTINES)
