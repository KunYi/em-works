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
// File:        /drivers/display/DDLCDC/DDLcdcRotate.cpp
// Purpose:     Screen rotation operations when not in DirectDraw Mode
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
// EXPORTED FUNCTIONS

//------------------------------------------------------------------------------
//
// Function: GetRotateModeFromReg
//
// This function is used to read the registry to get the initial
// rotation angle.
//
// Parameters:
//      None.
//
// Returns:
//      returns default rotation angle.
//
//------------------------------------------------------------------------------
int MX27DDLcdc::GetRotateModeFromReg(VOID)
{
    HKEY hKey;
    int iRotate;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SYSTEM\\GDI\\ROTATION"), 0, 0, &hKey))
    {
        DWORD dwSize, dwAngle, dwType = REG_DWORD;
        dwSize = sizeof(DWORD);
        if (ERROR_SUCCESS == RegQueryValueEx(hKey,
                                             TEXT("ANGLE"),
                                             NULL,
                                             &dwType,
                                             (LPBYTE)&dwAngle,
                                             &dwSize))
        {
            switch (dwAngle)
            {
                case 0:
                    iRotate = DMDO_0;
                    break;

                case 90:
                    iRotate = DMDO_90;
                    break;

                case 180:
                    iRotate = DMDO_180;
                    break;

                case 270:
                    iRotate = DMDO_270;
                    break;

                default:
                    iRotate = DMDO_0;
                    break;
            }
        }

        RegCloseKey(hKey);
    }
    else
    {
        iRotate = DMDO_0;
    }

    return iRotate;
}


//------------------------------------------------------------------------------
//
// Function: SetRotateParms
//
// This function is used to set up the screen width and height
// based on the current rotation angle.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
VOID MX27DDLcdc::SetRotateParams(VOID)
{
    switch(m_iRotate)
    {
        case DMDO_90:
        case DMDO_270:
            m_pMode->height = m_nScreenWidthSave;
            m_pMode->width = m_nScreenHeightSave;
            break;
    
        case DMDO_0:
        case DMDO_180:
        default:
            m_pMode->width = m_nScreenWidthSave;
            m_pMode->height = m_nScreenHeightSave;
            break;
    }

    return;
}


//------------------------------------------------------------------------------
//
//  FUNCTION:       DynRotate
//
//  DESCRIPTION:    This function Rotates the screen when DrvEscape gets a 
//                  DRVESC_SETSCREENROTATION message.
//
//  PARAMETERS:     
//                  int angle - angle to rotate
//
//  RETURNS:        
//                  DISP_CHANGE_SUCCESSFUL
//
//------------------------------------------------------------------------------
LONG MX27DDLcdc::DynRotate(int angle)
{
    // Return if no change is needed
    if (angle == m_iRotate)
    {
        return DISP_CHANGE_SUCCESSFUL;
    }

    // Update the rotation
    m_iRotate = angle;

    CursorOff();

    SetRotateParams();

    m_nScreenWidth = m_pMode->width;
    m_nScreenHeight = m_pMode->height;
    m_pPrimarySurface->SetRotation(m_nScreenWidth, m_nScreenHeight, angle);

    CursorOn();

    return DISP_CHANGE_SUCCESSFUL;
}
