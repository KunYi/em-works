//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  ddipu_cursor.cpp
//
//  Implementation of DDIPU cursor operation functions.
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
// Function: SetPointerShape
//
// This method sets the shape of the pointer, the hot spot of the pointer,
// and the colors to use for the cursor if the cursor is multicolored. The
// graphics device interface (GDI) calls GPE::MovePointer separately to move
// or hide the cursor.
//
// Parameters:
//      pMask
//          [in] Pointer to a mask containing the cursor shape.
//
//      pColorSurf
//          [in] Pointer to a surface specifying the colors to use for the cursor.
//
//      xHot
//          [in] Horizontal location of the cursor's hot spot.
//
//      yHot
//          [in] Vertical location of the cursor's hot spot.
//
//      cx
//          [in] Width of the cursor.
//
//      cy
//          [in] Height of the cursor.
//
// Returns:
//      S_OK            successful
//      others          failed
//
//------------------------------------------------------------------------------
SCODE DDIPU::SetPointerShape(GPESurf * pMask,
                                 GPESurf * pColorSurf,
                                 int xHot,
                                 int yHot,
                                 int cx,
                                 int cy)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pColorSurf);

#ifndef PLAT_PMC    // PMC doesn't support cursors

    UCHAR    *andPtr;        // input pointer
    UCHAR    *xorPtr;        // input pointer
    UCHAR    *andLine;       // output pointer
    UCHAR    *xorLine;       // output pointer
    char      bAnd;
    char      bXor;
    int       row;
    int       col;
    int       i;
    int       bitMask;

    // turn current cursor off
    CursorOff();

    // release memory associated with old cursor
    if (!pMask)                            // do we have a new cursor shape
    {
        m_CursorDisabled = TRUE;        // no, so tag as disabled
    }
    else
    {
        m_CursorDisabled = FALSE;        // yes, so tag as not disabled

        // store size and hotspot for new cursor
        m_CursorSize.x = cx;
        m_CursorSize.y = cy;
        m_CursorHotspot.x = xHot;
        m_CursorHotspot.y = yHot;

        andPtr = (UCHAR*)pMask->Buffer();
        xorPtr = (UCHAR*)pMask->Buffer() + (cy * pMask->Stride());

        // store OR and AND mask for new cursor
        for (row = 0; row < cy; row++)
        {
            andLine = &m_CursorAndShape[cx * row];
            xorLine = &m_CursorXorShape[cx * row];

            for (col = 0; col < cx / 8; col++)
            {
                bAnd = andPtr[row * pMask->Stride() + col];
                bXor = xorPtr[row * pMask->Stride() + col];

                for (bitMask = 0x0080, i = 0; i < 8; bitMask >>= 1, i++)
                {
                    andLine[(col * 8) + i] = bAnd & bitMask ? 0xFF : 0x00;
                    xorLine[(col * 8) + i] = bXor & bitMask ? 0xFF : 0x00;
                }
            }
        }
    }

#endif // PLAT_PMC

    return    S_OK;
}


//------------------------------------------------------------------------------
//
// Function: MovePointer
//
// This method executes from applications either to move the hot spot of
// the cursor to a specific screen location or to hide the cursor.
//
// Parameters:
//      x
//          [in] Horizontal screen location to move the cursor to.
//          Applications can pass a value of -1 to hide the cursor.
//
//      y
//          [in] Vertical screen location to move the cursor to.
//
// Returns:
//      S_OK            successful
//      others          failed
//
//------------------------------------------------------------------------------
SCODE DDIPU::MovePointer(int x, int y)
{
    CursorOff();

    if (x != -1 || y != -1)
    {
        // compute new cursor rect
        m_CursorRect.left = x - m_CursorHotspot.x;
        m_CursorRect.right = m_CursorRect.left + m_CursorSize.x;
        m_CursorRect.top = y - m_CursorHotspot.y;
        m_CursorRect.bottom = m_CursorRect.top + m_CursorSize.y;

        CursorOn();
    }

    return    S_OK;
}


//------------------------------------------------------------------------------
//
// Function: CursorOn
//
// Turns on the hardware cursor.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
VOID DDIPU::CursorOn(VOID)
{
    UCHAR    *ptrScreen = (UCHAR*)m_pActiveSurface->Buffer();
    UCHAR    *ptrLine;
    UCHAR    *cbsLine;
    int        x, y;
    
    //Check if there is locked area in which cursor can't be shown.
    if(m_CursorAreaLock)
    {
        if((m_LockRect.bottom ==0)&&(m_LockRect.right == 0))
        {
            //For Valid Rect, top <= bottom, and left <= right, and all value should be positive.
            //So checking bottom and right is enough.
            //full screen lock
            return;
        }
        else if(m_CursorRect.top <= m_LockRect.bottom && 
                m_CursorRect.bottom >= m_LockRect.top &&
                m_CursorRect.left <= m_LockRect.right && 
                m_CursorRect.right >= m_LockRect.left)
        {
            return;
        }
    }
    if (!m_CursorForcedOff && !m_CursorDisabled && !m_CursorVisible)
    {
        RECTL cursorRectSave = m_CursorRect;
        int   iRotate;
        RotateRectl(&m_CursorRect);
        for (y = m_CursorRect.top; y < m_CursorRect.bottom; y++)
        {
            if (y < 0)
            {
                continue;
            }
            if (y >= m_nScreenHeightSave)
            {
                break;
            }

            ptrLine = &ptrScreen[y * m_pActiveSurface->Stride()];
            cbsLine = &m_CursorBackingStore[(y - m_CursorRect.top) * (m_CursorSize.x * (m_pMode->Bpp >> 3))];

            for (x = m_CursorRect.left; x < m_CursorRect.right; x++)
            {
                if (x < 0)
                {
                    continue;
                }
                if (x >= m_nScreenWidthSave)
                {
                    break;
                }

                // x' = x - m_CursorRect.left; y' = y - m_CursorRect.top;
                // Width = m_CursorSize.x;   Height = m_CursorSize.y;
                switch (m_iRotate)
                {
                    case DMDO_0:
                        iRotate = (y - m_CursorRect.top)*m_CursorSize.x + x - m_CursorRect.left;
                        break;
                    case DMDO_90:
                        iRotate = (x - m_CursorRect.left)*m_CursorSize.x + m_CursorSize.y - 1 - (y - m_CursorRect.top);
                        break;
                    case DMDO_180:
                        iRotate = (m_CursorSize.y - 1 - (y - m_CursorRect.top))*m_CursorSize.x + m_CursorSize.x - 1 - (x - m_CursorRect.left);
                        break;
                    case DMDO_270:
                        iRotate = (m_CursorSize.x -1 - (x - m_CursorRect.left))*m_CursorSize.x + y - m_CursorRect.top;
                        break;
                    default:
                        iRotate = (y - m_CursorRect.top)*m_CursorSize.x + x - m_CursorRect.left;
                        break;
                }
                cbsLine[(x - m_CursorRect.left) * (m_pMode->Bpp >> 3)] = ptrLine[x * (m_pMode->Bpp >> 3)];
                ptrLine[x * (m_pMode->Bpp >> 3)] &= m_CursorAndShape[iRotate];
                ptrLine[x * (m_pMode->Bpp >> 3)] ^= m_CursorXorShape[iRotate];
                if (m_pMode->Bpp > 8)
                {
                    cbsLine[(x - m_CursorRect.left) * (m_pMode->Bpp >> 3) + 1] = ptrLine[x * (m_pMode->Bpp >> 3) + 1];
                    ptrLine[x * (m_pMode->Bpp >> 3) + 1] &= m_CursorAndShape[iRotate];
                    ptrLine[x * (m_pMode->Bpp >> 3) + 1] ^= m_CursorXorShape[iRotate];
                    if (m_pMode->Bpp > 16)
                    {
                        cbsLine[(x - m_CursorRect.left) * (m_pMode->Bpp >> 3) + 2] = ptrLine[x * (m_pMode->Bpp >> 3) + 2];
                        ptrLine[x * (m_pMode->Bpp >> 3) + 2] &= m_CursorAndShape[iRotate];
                        ptrLine[x * (m_pMode->Bpp >> 3) + 2] ^= m_CursorXorShape[iRotate];
                    }
                }
            }
        }
        m_CursorRect = cursorRectSave;
        m_CursorVisible = TRUE;
    }
}


//------------------------------------------------------------------------------
//
// Function: CursorOff
//
// Turns on the hardware cursor.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
VOID DDIPU::CursorOff (void)
{
    UCHAR    *ptrScreen = (UCHAR*)m_pActiveSurface->Buffer();
    UCHAR    *ptrLine;
    UCHAR    *cbsLine;
    int        x, y;

    if (!m_CursorForcedOff && !m_CursorDisabled && m_CursorVisible)
    {
        RECTL rSave = m_CursorRect;
        RotateRectl(&m_CursorRect);
        for (y = m_CursorRect.top; y < m_CursorRect.bottom; y++)
        {
            // clip to displayable screen area (top/bottom)
            if (y < 0)
            {
                continue;
            }
            if (y >= m_nScreenHeightSave)
            {
                break;
            }

            ptrLine = &ptrScreen[y * m_pActiveSurface->Stride()];
            cbsLine = &m_CursorBackingStore[(y - m_CursorRect.top) * (m_CursorSize.x * (m_pMode->Bpp >> 3))];

            for (x = m_CursorRect.left; x < m_CursorRect.right; x++)
            {
                // clip to displayable screen area (left/right)
                if (x < 0)
                {
                    continue;
                }
                if (x >= m_nScreenWidthSave)
                {
                    break;
                }

                ptrLine[x * (m_pMode->Bpp >> 3)] = cbsLine[(x - m_CursorRect.left) * (m_pMode->Bpp >> 3)];
                if (m_pMode->Bpp > 8)
                {
                    ptrLine[x * (m_pMode->Bpp >> 3) + 1] = cbsLine[(x - m_CursorRect.left) * (m_pMode->Bpp >> 3) + 1];
                    if (m_pMode->Bpp > 16)
                    {
                        ptrLine[x * (m_pMode->Bpp >> 3) + 2] = cbsLine[(x - m_CursorRect.left) * (m_pMode->Bpp >> 3) + 2];
                    }
                }
            }
        }
        m_CursorRect = rSave;
        m_CursorVisible = FALSE;
    }
}


//------------------------------------------------------------------------------
//
// Function: PointerBltPrepare
//
// Prepare cursor for blit operation.
//
// Parameters:
//      pBltParms
//          [in] Blit parameter structure.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
VOID DDIPU::PointerBltPrepare(GPEBltParms * pBltParms)
{
    RECTL    rectl;

    if (pBltParms->pDst == m_pActiveSurface)    // only care if dest is main display surface
    {
        if (m_CursorVisible && !m_CursorDisabled)
        {
            if (pBltParms->prclDst != NULL)        // make sure there is a valid prclDst
            {
                rectl = *pBltParms->prclDst;    // if so, use it
            }
            else
            {
                rectl = m_CursorRect;            // if not, use the Cursor rect - this forces the cursor to be turned off in this case
            }

            if (m_CursorRect.top <= rectl.bottom && m_CursorRect.bottom >= rectl.top &&
                m_CursorRect.left <= rectl.right && m_CursorRect.right >= rectl.left)
            {
                CursorOff();
                m_CursorForcedOff = TRUE;
            }
        }
    }

    // check for source overlap with cursor and turn off cursor if overlaps
    if (pBltParms->pSrc == m_pActiveSurface)    // only care if source is main display surface
    {
        if (m_CursorVisible && !m_CursorDisabled)
        {
            if (pBltParms->prclSrc != NULL)        // make sure there is a valid prclSrc
            {
                rectl = *pBltParms->prclSrc;    // if so, use it
            }
            else
            {
                rectl = m_CursorRect;            // if not, use the CUrsor rect - this forces the cursor to be turned off in this case
            }
            if (m_CursorRect.top <= rectl.bottom && m_CursorRect.bottom >= rectl.top &&
                m_CursorRect.left <= rectl.right && m_CursorRect.right >= rectl.left)
            {
                CursorOff();
                m_CursorForcedOff = TRUE;
            }
        }
    }
}
//------------------------------------------------------------------------------
//
// Function: AreaLock
//
// Register an area locked by directdraw interface in which cursor shouldn't be drawn.
//
// Parameters:
//      pSurf
//          [in] The surface been locked.
//
//      pRect
//          [in] the locked area.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
VOID DDIPU::AreaLock(DDIPUSurf * pSurf, RECT* pRect)
{
    if(pSurf == m_pActiveSurface)
    {
        if(pRect)
        {
            //If cursor is in the locked area, hide cursor.
            if (m_CursorRect.top <= pRect->bottom && 
                m_CursorRect.bottom >= pRect->top &&
                m_CursorRect.left <= pRect->right && 
                m_CursorRect.right >= pRect->left)
            {
                CursorOff();
            }
            //Record the locked area for further check when cursor moves
            m_LockRect.bottom = pRect->bottom;
            m_LockRect.top = pRect->top;
            m_LockRect.right = pRect->right;
            m_LockRect.left = pRect->left;
        }
        else
        {
            //If pRect equals to NULL, it means whole screen is locked. 
            //Set the lock area to 0 for further check.
            memset(&m_LockRect,0,sizeof(RECTL)); 
            CursorOff();
        }
        m_CursorAreaLock = TRUE;
    }
}
//------------------------------------------------------------------------------
//
// Function: AreaUnLock
//
// Release the area locked by directdraw interface in which cursor shouldn't be drawn.
//
// Parameters:
//      pSurf
//          [in] The surface been locked.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
VOID DDIPU::AreaUnLock(DDIPUSurf * pSurf)
{
    if(pSurf == m_pActiveSurface)
    {
        m_CursorAreaLock = FALSE;
        //Restore the cursor.
        if((m_LockRect.bottom ==0)&&(m_LockRect.right == 0))
        {
            //For Valid Rect, top <= bottom, and left <= right, and all value should be positive.
            //So checking bottom and right is enough.
            //Locked area is 0, original full screen is locked.
            CursorOn();
        }
        else if(m_CursorRect.top <= m_LockRect.bottom && 
                m_CursorRect.bottom >= m_LockRect.top &&
                m_CursorRect.left <= m_LockRect.right && 
                m_CursorRect.right >= m_LockRect.left)
        {
            //If cursor is in the locked area, restore cursor.
            CursorOn();
        }
    }

}


