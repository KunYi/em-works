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
//  Copyright (C) 2005-2006, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//---------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
// File:        /drivers/display/DDLCDC/DDLcdcBlt.cpp
// Purpose:     bitblt/rectangle for LCDC
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
SCODE MX27DDLcdc::BltPrepare(GPEBltParms * pBltParms)
{
    // default to base EmulatedBlt routine
    pBltParms->pBlt = &GPE::EmulatedBlt;
    
    // see if we need to deal with cursor
    PointerBltPrepare(pBltParms);

    if (m_iRotate && ((pBltParms->pSrc == m_pPrimarySurface) || (pBltParms->pDst == m_pPrimarySurface)))
        pBltParms->pBlt = (SCODE (GPE::*)(GPEBltParms *))&GPE::EmulatedBltRotate;

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
SCODE MX27DDLcdc::BltComplete(GPEBltParms * pBltParms)
{
    // see if cursor was forced off because of overlap with source or destination and turn back on
    if (m_CursorForcedOff)
    {
        m_CursorForcedOff = FALSE;
        CursorOn();
    }
    
    return S_OK;
}

