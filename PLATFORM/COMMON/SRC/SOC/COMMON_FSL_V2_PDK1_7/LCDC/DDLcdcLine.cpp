//-----------------------------------------------------------------------------
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
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
// File:        /drivers/display/DDLCDC/DDLcdcLine.cpp
// Draw lines
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
SCODE MXDDLcdc::Line(GPELineParms * pLineParms, EGPEPhase phase)
{
    if (phase == gpeSingle || phase == gpePrepare)
    {
            pLineParms->pLine = (SCODE (GPE::*)(struct GPELineParms *))(&MXDDLcdc::WrappedEmulatedLine);
    }

    return S_OK;
}

//------------------------------------------------------------------------------
//
// Function: WrappedEmulatedLine
//
// This method executes the emulated line routine.
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
SCODE MXDDLcdc::WrappedEmulatedLine(GPELineParms *pLineParms)
{
    SCODE retval;

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
//            bounds.right = pLineParms->xStart + pLineParms->cPels + 1;
            bounds.right = pLineParms->xStart + pLineParms->cPels;
            bounds.bottom = bounds.top + N_plus_1;
            break;
        case 1:
            bounds.left = pLineParms->xStart;
            bounds.top = pLineParms->yStart;
//            bounds.bottom = pLineParms->yStart + pLineParms->cPels + 1;
            bounds.bottom = pLineParms->yStart + pLineParms->cPels;
            bounds.right = bounds.left + N_plus_1;
            break;
        case 2:
            bounds.right = pLineParms->xStart + 1;
            bounds.top = pLineParms->yStart;
//            bounds.bottom = pLineParms->yStart + pLineParms->cPels + 1;
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
//            bounds.right = pLineParms->xStart + pLineParms->cPels + 1;
            bounds.right = pLineParms->xStart + pLineParms->cPels;
            bounds.top = bounds.bottom - N_plus_1;
            break;
        default:
            DEBUGMSG(GPE_ZONE_ERROR,(TEXT("Invalid direction: %d\r\n"),pLineParms->iDir));
            return E_INVALIDARG;
        } // switch

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

    return S_OK;
}
