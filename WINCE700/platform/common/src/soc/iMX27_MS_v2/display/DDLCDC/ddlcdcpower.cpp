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
// File:        /drivers/display/DDLCDC/DDLcdcPower.cpp
// Purpose:     Power handling routines
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
// Function: AdvertisePowerInterface
//
// This routine notifies the OS that we support the
// Power Manager IOCTLs (through ExtEscape(), which
// calls DrvEscape()).
//
// Parameters:
//      hInst
//          [in] handle to module DLL
//
// Returns:
//      TRUE            successful
//      FALSE           failed
//
//------------------------------------------------------------------------------
BOOL MX27DDLcdc::AdvertisePowerInterface(HMODULE hInst)
{
    GUID gTemp;
    BOOL fOk = FALSE;
    HKEY hk;
    DWORD dwStatus;

    // assume we are advertising the default class
    _tcscpy_s(m_szGuidClass, _countof(m_szGuidClass), PMCLASS_DISPLAY);
    m_szGuidClass[MAX_PATH-1] = 0;
    fOk = ConvertStringToGuid(m_szGuidClass, &gTemp);
    DEBUGCHK(fOk);

    // check for an override in the registry
    dwStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("System\\GDI\\Drivers"), 0, 0, &hk);
    if(dwStatus == ERROR_SUCCESS)
    {
        DWORD dwType, dwSize;
        dwSize = sizeof(m_szGuidClass);
        dwStatus = RegQueryValueEx(hk, _T("DisplayPowerClass"), NULL, &dwType, (LPBYTE)m_szGuidClass, &dwSize);
        if(dwStatus == ERROR_SUCCESS && dwType == REG_SZ)
        {
            // got a guid string, convert it to a guid
            fOk = ConvertStringToGuid(m_szGuidClass, &gTemp);
            DEBUGCHK(fOk);
        }

        // release the registry key
        RegCloseKey(hk);
    }

    // figure out what device name to advertise
    TCHAR szModuleName[MAX_PATH];
    if(fOk)
    {
        fOk = GetModuleFileName(hInst, szModuleName, _countof(szModuleName));
        DEBUGCHK(fOk);
    }

    // Obtain just the file name
    if(fOk)
    {
        const TCHAR* pszFileName = _tcsrchr(szModuleName, _T('\\'));
        if (pszFileName)
        {
            pszFileName++;
        }
        else
        {
            pszFileName = szModuleName;
        }

        if (FAILED(StringCchCopy(m_szDevName, _countof(m_szDevName), pszFileName)))
        {
            fOk = FALSE;
        }
    }

    // now advertise the interface
    if(fOk)
    {
        fOk = AdvertiseInterface(&gTemp, m_szDevName, TRUE);
        DEBUGCHK(fOk);
    }

    return fOk;
}



//------------------------------------------------------------------------------
//
// Function: SetDisplayPower
//
// This function sets the display hardware according
// to the desired video power state. Also updates
// current device power state variable.
//
// Parameters:
//      dx
//          [in] Desired video power state.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
VOID MX27DDLcdc::SetDisplayPower(CEDEVICE_POWER_STATE dx)
{
    switch(dx)
    {
        case D0:
            if(m_Dx != dx)
            {
                // Restore to primary surface.
                SetVisibleSurface(m_pPrimarySurface, TRUE);

                if(m_pOverlaySurfaceOp->isGraphicWindowRunning)
                    INSREG32BF(&m_pLcdcReg->GWCR, LCDC_GWCR_GWE, 1);
#ifdef ENABLE_TV_ON
                if(m_bTVModeActive)
                    BSPPowerOnTVOut();
                else
#endif
                    BSPTurnOnLCD();

                m_Dx = D0;
            }
            break;

        case D1:
        case D2:
        case D3:
        case D4:
            if(m_Dx != D4)
            {
                if(m_pOverlaySurfaceOp->isGraphicWindowRunning)
                    INSREG32BF(&m_pLcdcReg->GWCR, LCDC_GWCR_GWE, 0);

                // Show a white screen when suspension.
                SetVisibleSurface(m_pPrimarySurface, TRUE);
                Sleep(200); // IMPORTANT: Wait at least one frame time!

#ifdef ENABLE_TV_ON
                if(m_bTVModeActive)
                    BSPPowerOffTVOut();
                else
#endif
                    BSPTurnOffLCD();

                m_Dx = D4;
            }
            break;

        default:
            break;
    }
}


//------------------------------------------------------------------------------
//
// Function: PmToVideoPowerState
//
// This function maps between power manager power
// states and video ioctl power states.
//
// Parameters:
//      dx
//          [in] Desired power manager device power state.
//
// Returns:
//      Corresponding video ioctl power state.
//
//------------------------------------------------------------------------------
VIDEO_POWER_STATE MX27DDLcdc::PmToVideoPowerState(CEDEVICE_POWER_STATE dx)
{
    VIDEO_POWER_STATE vps;

    switch(dx)
    {
        // turn the display on
        case D0:
            vps = VideoPowerOn;
            break;

        // if asked for a state we don't support, go to the next lower one
        case D1:
        case D2:
        case D3:
        case D4:
            vps = VideoPowerOff;
            break;

        default:
            DEBUGMSG(GPE_ZONE_ERROR, (L"MX27DDLcdc PmToVideoPowerState: mapping unknown PM state %d to VideoPowerOn\r\n", dx));
            vps = VideoPowerOn;
            break;
    }
    return vps;
}


//------------------------------------------------------------------------------
//
// Function: VideoToPmPowerState
//
// This function maps between video power states
// to PM power states.
//
// Parameters:
//      vps
//          [in] Desired video power state.
//
// Returns:
//      Corresponding PM device power state.
//
//------------------------------------------------------------------------------
CEDEVICE_POWER_STATE MX27DDLcdc::VideoToPmPowerState(VIDEO_POWER_STATE vps)
{
    CEDEVICE_POWER_STATE dx;

    switch(vps)
    {
        case VideoPowerOn:
            dx = D0;
            break;

        case VideoPowerStandBy:
        case VideoPowerSuspend:
        case VideoPowerOff:
            dx = D4;
            break;

        default:
            dx = D0;
            DEBUGMSG(GPE_ZONE_WARNING, (L"MX27DDLcdc VideoToPmPowerState: mapping unknown video state %d to pm state %d\r\n",
                     vps, dx));
            break;
    }

    return dx;
}


//------------------------------------------------------------------------------
//
// Function: ConvertStringToGuid
//
// This converts a string into a GUID.
//
// Parameters:
//      pszGuid
//          [in] Pointer to GUID in string format.
//
//      pGuid
//          [out] Pointer to GUID struct for output
//
// Returns:
//      TRUE            successful
//      FALSE           failed
//
//------------------------------------------------------------------------------
BOOL MX27DDLcdc::ConvertStringToGuid(LPCTSTR pszGuid, GUID * pGuid)
{
    UINT Data4[8];
    int  Count;
    BOOL fOk = FALSE;
    TCHAR *pszGuidFormat = _T("{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}");

    DEBUGCHK(pGuid != NULL && pszGuid != NULL);
    __try
    {
        if (_stscanf_s(pszGuid, pszGuidFormat, &pGuid->Data1,
            &pGuid->Data2, &pGuid->Data3, &Data4[0], &Data4[1], &Data4[2], &Data4[3],
            &Data4[4], &Data4[5], &Data4[6], &Data4[7]) == 11)
        {
            for(Count = 0; Count < (sizeof(Data4) / sizeof(Data4[0])); Count++)
            {
                        pGuid->Data4[Count] = (UCHAR) Data4[Count];
            }
        }
        fOk = TRUE;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
    }

    return fOk;
}
