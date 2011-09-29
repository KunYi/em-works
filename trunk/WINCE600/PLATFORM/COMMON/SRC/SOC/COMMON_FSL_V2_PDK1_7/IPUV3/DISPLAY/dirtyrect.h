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
//  File:  DirtyRect.h
//
//  Header for Dirty Rectangle driver used for UI updating in TV Out mode.
//
//------------------------------------------------------------------------------


#pragma once

class DirtyRect
{
public:
    DirtyRect(
        DWORD dwWidth,
        DWORD dwHeight,
        DWORD dwRegionWidth,
        DWORD dwRegionHeight
        );

    virtual ~DirtyRect();

    void SetDirtyRegion(LPRECT prect);
    UINT32 GetNumDirtyRegions();
    void GetDirtyRegions(LPRECT prects, UINT32 numDirtyRegions);
    void GetFullDirtyRect(LPRECT prects, UINT32 numDirtyRegions, LPRECT prect_full);
    void Reset();

private:

    // source information
    DWORD m_dwWidth;
    DWORD m_dwHeight;

    // size in pixels of each copy region
    DWORD m_dwRegionWidth;
    DWORD m_dwRegionHeight;

    // total number of regions
    DWORD m_dwRegionCount;

    // array of dirty regions
    BOOL *m_pDirty;

    // array of rectangles for each region
    RECT *m_pRects;

    // current number of dirty regions
    DWORD m_dwNumDirtyRegions;
};
