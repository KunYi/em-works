//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2007, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  ddipu_rotate.cpp
//
//  Implementation of DDIPU screen rotation operations when
//  not in DirectDraw mode.
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
int DDIPU::GetRotateModeFromReg(VOID)
{
    HKEY hKey;
    int iRotate = DMDO_0;

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
VOID DDIPU::SetRotateParams(VOID)
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
// Function: IsRotated
//
// Return whether the display memory is currently rotated or not.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if rotated, FALSE if not rotated.
//
//------------------------------------------------------------------------------
BOOL DDIPU::IsRotated(VOID)
{
    // TODO: If below is correct, this is the same as IsRotate(). Note: is the panel was landscape-addressed, this might get more complex.
    switch (m_iRotate)
    {
    case DMDO_0: case DMDO_180:
        return FALSE;
        break;

    case DMDO_270: case DMDO_90:
        return TRUE;
        break;

    default:
        return FALSE;
    }
}

//------------------------------------------------------------------------------
//
// Function: SetRotation
//
// This function Rotates the screen.
//
// Parameters:
//      dwMode
//          [in] Current display mode.
//      dwRotation
//          [in] Current display orientation
//
// Returns:
//      DISP_CHANGE_SUCCESSFUL
//
//------------------------------------------------------------------------------
int DDIPU::SetRotation(DWORD dwMode, DWORD dwRotation)
{
    UNREFERENCED_PARAMETER(dwMode);

    // The display driver may choose not to support rotation, in which
    // case the m_bRotationSupported variable will be set to TRUE.
    if (!m_bRotationSupported)
    {
        return DISP_CHANGE_BADMODE;
    }

    // Return if no change is needed
    if (m_iRotate == (int) dwRotation)
    {
        return DISP_CHANGE_SUCCESSFUL;
    }

    // Update the rotation
    m_iRotate = dwRotation;

    CursorOff();

    SetRotateParams();

    m_nScreenWidth = m_pMode->width;
    m_nScreenHeight = m_pMode->height;

    // Update the surface
    DDIPUSurf *pSurf = (DDIPUSurf *) m_pPrimarySurface;

    // Clear the virtual memory for the primary surface
    memset((PUCHAR)(pSurf->Buffer()), 0, m_nScreenHeight * m_nScreenWidth * (m_nScreenBpp / 8));

    pSurf->SetRotation(m_nScreenWidth, m_nScreenHeight, dwRotation);

    m_rcWorkRect.left = 0;
    m_rcWorkRect.top = 0;
    m_rcWorkRect.right = m_nScreenWidth;
    m_rcWorkRect.bottom = m_nScreenHeight;

    DisplaySetScreenRotation(dwRotation);

    // Now that rotation has completed, we must update the full screen
    RECT fullRect = {0, 0, m_nScreenWidth, m_nScreenHeight};
    DisplayUpdate(&fullRect);

    CursorOn();

    return DISP_CHANGE_SUCCESSFUL;
}
