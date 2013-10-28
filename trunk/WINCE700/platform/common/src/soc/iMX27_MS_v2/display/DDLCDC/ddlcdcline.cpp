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
// File:        /drivers/display/DDLCDC/DDLcdcLine.cpp
// Purpose:     Draw lines
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
SCODE MX27DDLcdc::Line(GPELineParms * pLineParms, EGPEPhase phase)
{
    // LCDC of i.MX27 doesn't have any hardware acceleration for line
    // drawing, so just use emulated one provided by Microsoft
    pLineParms->pLine = &GPE::EmulatedLine;

    return S_OK;
}
