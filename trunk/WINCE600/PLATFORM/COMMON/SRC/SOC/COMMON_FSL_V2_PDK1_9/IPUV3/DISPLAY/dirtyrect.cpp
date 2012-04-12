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
//  Copyright (C) 2006-2007, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  DirtyRect.cpp
//
//  Dirty Rectangle driver used for UI updating in TV Out mode.
//
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4201)

#include <windows.h>
#include <types.h>
#include <winddi.h>
#include <emul.h>
#include <ceddk.h>

#include <ddkmacro.h>
#include <ddkreg.h>

#include <ddrawi.h>
#include <ddgpe.h>
#include <ddhfuncs.h>

#include <strsafe.h>

#pragma warning(pop)

#include "dirtyrect.h"

DirtyRect::DirtyRect(DWORD dwWidth, DWORD dwHeight, DWORD dwRegionWidth, DWORD dwRegionHeight) :
m_dwWidth(dwWidth),
m_dwHeight(dwHeight),
m_dwRegionWidth(dwRegionWidth),
m_dwRegionHeight(dwRegionHeight)
{
    // width and height must be evenly divisible by region size
    DEBUGCHK(dwWidth % dwRegionWidth == 0);
    DEBUGCHK(dwHeight % dwRegionHeight == 0);

    m_dwRegionCount = (m_dwWidth / m_dwRegionWidth) * (m_dwHeight / m_dwRegionHeight);
    m_pDirty = new BOOL[m_dwRegionCount];
    m_dwNumDirtyRegions = 0;

    // Build table of rectangles
    m_pRects = new RECT[m_dwRegionCount];
    DEBUGCHK(m_pRects != NULL);

    int currentRow, currentCol;
    DWORD index;
    int rows = m_dwHeight / m_dwRegionHeight;
    int cols = m_dwWidth / m_dwRegionWidth;

    // for each row
    for(currentRow = 0; currentRow < rows; currentRow++)
    {
        for(currentCol = 0; currentCol < cols; currentCol++)
        {
            index = currentRow*cols + currentCol;
            DEBUGCHK(index < m_dwRegionCount);
            if ((index <  m_dwRegionCount) && (m_pRects != NULL))
            {
                // Create rectangle
                m_pRects[index].top = currentRow * m_dwRegionHeight;
                m_pRects[index].bottom = (currentRow + 1) * m_dwRegionHeight - 1;
                m_pRects[index].left = currentCol * m_dwRegionWidth;
                m_pRects[index].right = (currentCol + 1) * m_dwRegionWidth - 1;
            }
        }
    }


    DEBUGCHK(m_pDirty != NULL);

}


DirtyRect::~DirtyRect()
{

    if(m_pDirty != NULL)
        delete[] m_pDirty;
    
    if(m_pRects != NULL)
        delete[] m_pRects;
}

void DirtyRect::SetDirtyRegion(LPRECT prect)
{

    int row,col;
    DWORD index;
    // mark updated regions
    int startrow = prect->top / m_dwRegionHeight;
    int startcol = prect->left / m_dwRegionWidth;
    int endrow = (prect->bottom - 1) / m_dwRegionHeight;
    int endcol = (prect->right - 1)  / m_dwRegionWidth;

    if (prect->bottom == 0) 
        endrow = 0;

    if (prect->right  == 0)
        endcol = 0;

    // for each row
    for(row = startrow; row <= endrow; row++)
    {
        for(col = startcol; col <= endcol; col++)
        {
            index = row*(m_dwWidth / m_dwRegionWidth) + col;
            //DEBUGCHK(index < m_dwRegionCount);
            if (index < m_dwRegionCount && m_pDirty[index] != TRUE)
            {
                m_pDirty[index] = TRUE;
                m_dwNumDirtyRegions++;
            }
        }
    }
}

UINT32 DirtyRect::GetNumDirtyRegions()
{
    return m_dwNumDirtyRegions;
}

void DirtyRect::GetDirtyRegions(LPRECT prects, UINT32 numDirtyRegions)
{
    UINT32 regionsAdded = 0;
    UINT32 i;

    if (numDirtyRegions < 1)
    {
        return;
    }

    // for each row
    for(i = 0; i < m_dwRegionCount; i++)
    {
        if (m_pDirty[i] == TRUE)
        {
            prects[regionsAdded] = m_pRects[i];
            regionsAdded++;
            // Now that we have grabbed the dirty region, reset
            // the dirty flag
            m_pDirty[i] = FALSE;
            if (regionsAdded == numDirtyRegions)
            {
                // We have grabbed all of the regions requested
                m_dwNumDirtyRegions = 0;
                return;
            }
        }
    }
}


// This function returns the smallest rectangle that encompasses all
// of the current dirty regions.
void DirtyRect::GetFullDirtyRect(LPRECT prects, UINT32 numDirtyRegions, LPRECT prect_full)
{
    RECT fullRect = {10000, 10000, 0, 0};
    UINT32 i;

    if (numDirtyRegions < 1)
    {
        return;
    }

    // for each row
    for(i = 0; i < numDirtyRegions; i++)
    {
        if (prects[i].top < fullRect.top)
        {
            fullRect.top = prects[i].top;
        }
        if (prects[i].bottom > fullRect.bottom)
        {
            fullRect.bottom = prects[i].bottom;
        }
        if (prects[i].left < fullRect.left)
        {
            fullRect.left = prects[i].left;
        }
        if (prects[i].right > fullRect.right)
        {
            fullRect.right = prects[i].right;
        }
    }

    // We have completed creation of our full rectangle, now copy to output
    prect_full->top = fullRect.top;
    prect_full->bottom = fullRect.bottom;
    prect_full->left = fullRect.left;
    prect_full->right = fullRect.right;
}

void DirtyRect::Reset()
{
    m_dwNumDirtyRegions = 0;
}

