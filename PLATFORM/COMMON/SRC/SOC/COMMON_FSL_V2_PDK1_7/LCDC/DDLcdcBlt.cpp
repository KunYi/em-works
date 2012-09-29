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
// File:        /drivers/display/DDLCDC/DDLcdcBlt.cpp
// 
// bitblt/rectangle for LCDC
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
SCODE MXDDLcdc::BltPrepare(GPEBltParms * pBltParms)
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
SCODE MXDDLcdc::BltComplete(GPEBltParms * pBltParms)
{
    UNREFERENCED_PARAMETER(pBltParms);

    // see if cursor was forced off because of overlap with source or destination and turn back on
    if (m_CursorForcedOff)
    {
        m_CursorForcedOff = FALSE;
        CursorOn();
    }
    
    return S_OK;
}

