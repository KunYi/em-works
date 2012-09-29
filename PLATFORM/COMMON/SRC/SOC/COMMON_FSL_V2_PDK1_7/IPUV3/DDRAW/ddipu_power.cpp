//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  ddipu_power.cpp
//
//  Implementation of DDIPU power handling routines.
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
BOOL DDIPU::AdvertisePowerInterface(HMODULE hInst)
{
    GUID gTemp;
    BOOL fOk = FALSE;
    HKEY hk;
    DWORD dwStatus;
    HRESULT hRes;

    // assume we are advertising the default class
    //_tcscpy(m_szGuidClass, PMCLASS_DISPLAY);
    hRes = StringCchCopy(m_szGuidClass, MAX_PATH, PMCLASS_DISPLAY);
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
    if(fOk)
    {
        fOk = GetModuleFileName(hInst, m_szDevName, sizeof(m_szDevName) / sizeof(m_szDevName[0]));
        DEBUGCHK(fOk);
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
VOID DDIPU::SetDisplayPower(CEDEVICE_POWER_STATE dx)
{
    static BOOL bOverlayWindowRestore = FALSE;
  
    switch(dx)
    {
        case D0:
            if(m_Dx != dx)
            {
                DisplayRegPop();
                DisplayEnableActiveFlows(TRUE);
                DisplayEnablePanels();

                // If overlay window was running, re-enable it.
                if (bOverlayWindowRestore || m_bIsOverlayWindowRunning)
                {
                    m_bIsOverlayWindowRunning = TRUE;
                    bOverlayWindowRestore = FALSE;
                    Display_StartOverlay();
                }

                // If Overlay disable request occurred during D4, we must
                // handle it now that we are coming out of D4.
                if (m_bOverlayDisableNotHandled)
                {
                    DisableOverlay();
                    m_bOverlayDisableNotHandled = FALSE;
                }

                SetVisibleSurface(m_pActiveSurface, TRUE);
                if(m_pPrimarySurface2)
                    DisplaySetSrcBuffer2(m_pPrimarySurface2->OffsetInVideoMemory());

                m_Dx = D0;
            }
            break;

        case D1:
        case D2:
        case D3:
            break;
        case D4:
            if(m_Dx != D4)
            {
                // If overlay window was running, disable it, but
                // leave isOverlayWindowRunning variable set to TRUE.
                // We will use this to restore the overlay upon power up.
                if (m_bIsOverlayWindowRunning)
                {
                    Display_StopOverlay();
                    bOverlayWindowRestore = TRUE;
                    m_bIsOverlayWindowRunning = FALSE;
                }
                //Stop the overlay on secondary display device if it's on
                Display_Middle_StopOverlay();

                DisplayDisablePanels();
                DisplayDisableActiveFlows(TRUE);
                DisplayRegPush();
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
VIDEO_POWER_STATE DDIPU::PmToVideoPowerState(CEDEVICE_POWER_STATE dx)
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
            DEBUGMSG(GPE_ZONE_ERROR, (L"MX21DDLcdc PmToVideoPowerState: mapping unknown PM state %d to VideoPowerOn\r\n", dx));
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
CEDEVICE_POWER_STATE DDIPU::VideoToPmPowerState(VIDEO_POWER_STATE vps)
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
            DEBUGMSG(GPE_ZONE_WARNING, (L"MX21DDLcdc VideoToPmPowerState: mapping unknown video state %d to pm state %d\r\n",
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
BOOL DDIPU::ConvertStringToGuid(LPCTSTR pszGuid, GUID * pGuid)
{
    UINT Data4[8];
    int  Count;
    BOOL fOk = FALSE;
    TCHAR *pszGuidFormat = _T("{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}");

    DEBUGCHK(pGuid != NULL && pszGuid != NULL);
    PREFAST_SUPPRESS(6320, "Generic exception handler");
    __try
    {
        if (_stscanf(pszGuid, pszGuidFormat, &pGuid->Data1,
            &pGuid->Data2, &pGuid->Data3, &Data4[0], &Data4[1], &Data4[2], &Data4[3],
            &Data4[4], &Data4[5], &Data4[6], &Data4[7]) == 11)
        {
            for(Count = 0; Count < (sizeof(Data4) / sizeof(Data4[0])); Count++)
            {
                PREFAST_SUPPRESS(6385, "_stscanf is a banned API, must be guaranteed by ourselves.");
                pGuid->Data4[Count] = (UCHAR) Data4[Count];
            }
        }
        fOk = TRUE;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        fOk = FALSE;
    }

    return fOk;
}
