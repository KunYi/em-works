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
//------------------------------------------------------------------------------
/*
 * Copyright (C) 2003-2004, MOTOROLA, INC. All Rights Reserved
 *
/*---------------------------------------------------------------------------
 * Copyright (C) 2006-2007, Freescale Semiconductor, Inc. All Rights Reserved.
 * THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
 * AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
 *--------------------------------------------------------------------------*/
/*
 *  File:       APPS/ADVBACKLIGHT/advbacklight.cpp
 *  Purpose:    Customised "Advanced..." button under Display Properties\Backlight tab
 *
 *  Notes:      This is for WinCE only
 *
 *  Author:     
 *  Date:       
 *
 *  Modifications:
 *  MM/DD/YYYY      Initials        Change description 
 *  11/28/2003      SYF             Modified to support runtime backlight levels
 *                                  
 */

#include "AdvBacklight.h"
#include "backlight.h"

HINSTANCE g_hInst = NULL;
PROPSHEETPAGE *rgPropPages;
PROPSHEETHEADER PropHdr;

static const TCHAR szRegKey[]       = TEXT("ControlPanel\\BackLight");
static const TCHAR szRegValue1[]    = TEXT("BatPowerOn");
static const TCHAR szRegValue2[]    = TEXT("ExtPowerOn");
static const TCHAR szUseBattery[]   = TEXT("UseBattery");
static const TCHAR szUseExt[]       = TEXT("UseExt");

static const TCHAR szRegBatteryLevel[]  = TEXT("BattBacklightLevel");
static const TCHAR szRegExtLevel[]      = TEXT("ACBacklightLevel");

#define BKL_MIN_SETTING     BKL_LEVEL_MIN
#define BKL_MAX_SETTING     BKL_LEVEL_MAX
#define BKL_NUM_TICKS       10
#define BKL_TICK_INTERVAL   ((BKL_MAX_SETTING - BKL_MIN_SETTING) / BKL_NUM_TICKS)


////////////////////////////////////////////////////////////////////////////////
//
//  BOOL BacklightAdvApplet(HWND hDlg)
//
//  Function name is based on the definition under cplmain\regcpl.h:
//  
//  This is the entry point we look for in the CPL pointed to by RV_ADVANCEDCPL
//
////////////////////////////////////////////////////////////////////////////////
BOOL BacklightAdvApplet(HWND hDlg)
{
    BOOL fRet = FALSE;
    int iData = 0;  //To track the OK|Cancel choice.
    
    // Allocate the page array (one page in this sample code).
    rgPropPages = (PROPSHEETPAGE*)LocalAlloc(LPTR, sizeof(PROPSHEETPAGE));

    if (NULL == rgPropPages) {
        return FALSE;
    }

    rgPropPages[0].dwSize       = sizeof(PROPSHEETPAGE);
    rgPropPages[0].dwFlags      = PSP_USETITLE;
    rgPropPages[0].hInstance    = g_hInst;
    rgPropPages[0].pszTemplate  = MAKEINTRESOURCE(IDD_DLG); 
    rgPropPages[0].pszIcon      = NULL;
    rgPropPages[0].pfnDlgProc   = AdvancedDlgProc;  //Pointer to the dialog box procedure for the page. 
                                                    //The dlg proc must not call the Win32 EndDialog function. 
    rgPropPages[0].pszTitle     = MAKEINTRESOURCE(IDS_PRSHDIALOG);
    rgPropPages[0].pfnCallback  = NULL;
    rgPropPages[0].lParam       = (LPARAM)&iData;

    //Create a property sheet and add the page defined in the property sheet header structure. 
    PropHdr.dwSize     = sizeof(PROPSHEETHEADER);
    PropHdr.dwFlags    = PSH_PROPSHEETPAGE;                                             
    PropHdr.hwndParent = hDlg; 
    PropHdr.hInstance  = g_hInst;
    PropHdr.pszIcon    = NULL;                      
    PropHdr.pszCaption = MAKEINTRESOURCE(IDS_PRSHTITLE);    
    PropHdr.nPages     = 1; 
    PropHdr.nStartPage = 0;
    PropHdr.ppsp       = rgPropPages;
    
    //PropertySheet creates a modal dialog. Return -1 if it fails.
    fRet = PropertySheet(&PropHdr);     
    
    if (-1 != fRet && iData) {
        fRet = TRUE;
    } else {
        fRet = FALSE;
    }
    
    if(rgPropPages) LocalFree(rgPropPages);

    return fRet;
}
////////////////////////////////////////////////////////////////////////////////
//
//  BOOL WINAPI DllEntry(HANDLE hInstance, DWORD fdwReason, LPVOID lpvReserved)
//
//  Win 32 Initialization DLL
//
////////////////////////////////////////////////////////////////////////////////
BOOL WINAPI DllEntry(HANDLE hInstance, DWORD fdwReason, LPVOID lpvReserved)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(lpvReserved);

    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            // Remember the instance
            g_hInst = (HINSTANCE)hInstance;
            break;

        case DLL_PROCESS_DETACH:
            g_hInst = NULL;
            break;
    }
    return TRUE;
}
////////////////////////////////////////////////////////////////////////////////
//
//  BOOL APIENTRY AdvancedDlgProc (HWND hwndPage, UINT message, WPARAM wParam, LPARAM lParam)
//
//  Settings tab dialog proc: DlgProc for the tab.
//
//////////////////////////////////////////////////////////////////////////////// 
BOOL CALLBACK AdvancedDlgProc(HWND hwndPage, UINT message, WPARAM wParam, LPARAM lParam)
{       
    PROPSHEETPAGE *psp;
    int *piData;
    static DWORD dwBattLevel;
    static DWORD dwExtLevel;
    
    switch (message)     
    {
        case WM_INITDIALOG:
            // Get application-defined information (lParam field).
            psp = (PROPSHEETPAGE *)lParam;          
            SetWindowLong(hwndPage, GWL_USERDATA, psp->lParam); 
            piData = (int *)psp->lParam;

            // Sets new extra information that is private to the application
            SetWindowLong(hwndPage, DWL_USER, (LONG)piData);        

            // get backlight driver info from registry & init controls
            GetFromRegistry(&dwBattLevel, &dwExtLevel, szRegKey, szRegBatteryLevel, szRegExtLevel);
            InitSliders(hwndPage, dwBattLevel, dwExtLevel);

            return TRUE;

        case WM_COMMAND:
            switch(LOWORD(wParam)) 
            {
                case IDOK:
                    UpdateBklSettings(hwndPage);
                    return TRUE;
                    
                case IDCANCEL:  
                    // Restore settings
                    RestoreBklSettings(dwBattLevel, dwExtLevel);
                    return TRUE;
            }
            break;

        case WM_SYSCOLORCHANGE:
            // fwd the syscolorchange to our trackbars
            SendMessage(GetDlgItem(hwndPage, IDC_TRACKBAR_BATT), message, wParam, lParam);
            SendMessage(GetDlgItem(hwndPage, IDC_TRACKBAR_AC), message, wParam, lParam);
            break; // return FALSE anyway so DefDlgProc handles it for ourselves

        case WM_HSCROLL:            // track bar message
            switch LOWORD(wParam)
            {
                case TB_BOTTOM:
                case TB_THUMBPOSITION:
                case TB_LINEUP:
                case TB_LINEDOWN:
                case TB_PAGEUP:
                case TB_PAGEDOWN:
                case TB_TOP:
                    UpdateBklSettings(hwndPage);
                    return TRUE;
            }
            break;
        case WM_NOTIFY:
        {
            switch(((NMHDR*)lParam)->code)
            {
                case PSN_APPLY:     //Notifies a page that the OK button is chosen.                                 
                    piData = (int *)GetWindowLong(hwndPage, DWL_USER);  
                    *piData = 1;

                    // update backlight levels
                    UpdateBklSettings(hwndPage);

                    //Accept the changes and allow the property sheet to be destroyed. 
                    SetWindowLong(hwndPage, DWL_MSGRESULT, PSNRET_NOERROR);                 
                    return TRUE; 

                case PSN_RESET:     //Notifies a page that the Cancel button is chosen
                                    //and the property sheet is about to be destroyed.                  
                    piData = (int *)GetWindowLong (hwndPage, DWL_USER); 
                    *piData = 0;    //Cancel is chosen.

                    // restore backlight levels
                    RestoreBklSettings(dwBattLevel, dwExtLevel);

                    SetWindowLong(hwndPage, DWL_MSGRESULT, FALSE);
                    return TRUE;
            }
            break;
        }
    }
    return FALSE;
}
////////////////////////////////////////////////////////////////////////////////
//
//  void GetFromRegistry(DWORD *dwState1, DWORD *dwState2, LPCTSTR lpszRegKey, 
//                      LPCTSTR lpszUseBat, LPCTSTR lpszUseExt)    
//
//  Get values from the registry. Set values to 1 in case of query errors.
//
////////////////////////////////////////////////////////////////////////////////
void GetFromRegistry(DWORD *dwState1, DWORD *dwState2, LPCTSTR lpszRegKey, LPCTSTR lpszUseBat, LPCTSTR lpszUseExt) 
{
    HKEY    hKey;
    DWORD   dwSize, dwType;

    if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, lpszRegKey, 0, 0, &hKey))
    {
        dwSize = sizeof(DWORD); 
        // Query 'UseBattery' value to set the state of the "While on battery power" check box.
        RegQueryValueEx(hKey, lpszUseBat, 0, &dwType, (LPBYTE)dwState1, &dwSize);
        // Query 'UseExt' value to set the state of the "While on external power" check box.
        RegQueryValueEx(hKey, lpszUseExt, 0, &dwType, (LPBYTE)dwState2, &dwSize);
            
        RegCloseKey(hKey);      
    }          
}
////////////////////////////////////////////////////////////////////////////////
//
//  void SetToRegistry(DWORD *dwState1, DWORD *dwState2, LPCTSTR lpszRegKey, 
//                      LPCTSTR lpszRegValue1, LPCTSTR lpszRegValue2) 
//
//  Set values to the regsitry. 
//
//////////////////////////////////////////////////////////////////////////////// 
void SetToRegistry(DWORD *dwState1, DWORD *dwState2, LPCTSTR lpszRegKey, 
                   LPCTSTR lpszRegValue1, LPCTSTR lpszRegValue2)    
{
    HKEY    hKey;
    
    if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, lpszRegKey, 0, 0, &hKey))
    {       
        RegSetValueEx(hKey, lpszRegValue1, 0, REG_DWORD, (LPBYTE)dwState1, sizeof(DWORD));
        RegSetValueEx(hKey, lpszRegValue2, 0, REG_DWORD, (LPBYTE)dwState2, sizeof(DWORD));                                  
        RegCloseKey(hKey);
    }   
}

////////////////////////////////////////////////////////////////////////////////
//
//  void InitSliders(HWND hDlg, DWORD dwBattBacklightLevel, 
//                  DWORD dwExtBacklightLevel)
//
//  Initialises trackbar controls 
//
//////////////////////////////////////////////////////////////////////////////// 
void InitSliders(HWND hDlg, DWORD dwBattBacklightLevel, DWORD dwExtBacklightLevel)
{
    HWND hwndSlider1 = GetDlgItem(hDlg, IDC_TRACKBAR_BATT);
    HWND hwndSlider2= GetDlgItem(hDlg, IDC_TRACKBAR_AC);

    SendMessage(hwndSlider1, TBM_SETRANGE, TRUE, MAKELONG(BKL_MIN_SETTING/BKL_TICK_INTERVAL, BKL_MAX_SETTING/BKL_TICK_INTERVAL));
    SendMessage(hwndSlider1, TBM_SETTICFREQ, 1, 0L);    // 15 tick marks
    SendMessage(hwndSlider1, TBM_SETPAGESIZE, 0L, 3 );

    SendMessage(hwndSlider2, TBM_SETRANGE, TRUE, MAKELONG(BKL_MIN_SETTING/BKL_TICK_INTERVAL, BKL_MAX_SETTING/BKL_TICK_INTERVAL));
    SendMessage(hwndSlider2, TBM_SETTICFREQ, 1, 0L);
    SendMessage(hwndSlider2, TBM_SETPAGESIZE, 0L, 3);

    SendMessage(hwndSlider1, TBM_SETPOS, TRUE, (dwBattBacklightLevel / BKL_TICK_INTERVAL));
    SendMessage(hwndSlider2, TBM_SETPOS, TRUE, (dwExtBacklightLevel / BKL_TICK_INTERVAL));
}

////////////////////////////////////////////////////////////////////////////////
//
//  void UpdateBklSettings(HWND hDlg) 
//
//  Updates Backlight settings to the registry from trackbar control & signal 
//  change event. 
//
//////////////////////////////////////////////////////////////////////////////// 
void UpdateBklSettings(HWND hDlg)
{
    HANDLE hEvent;
    DWORD dwBattLevel;
    DWORD dwExtLevel;
    
    HWND hwndSlider1 = GetDlgItem(hDlg, IDC_TRACKBAR_BATT);
    HWND hwndSlider2 = GetDlgItem(hDlg, IDC_TRACKBAR_AC);

    dwBattLevel = BKL_TICK_INTERVAL * SendMessage(hwndSlider1, TBM_GETPOS, 0, 0) ;
    dwExtLevel = BKL_TICK_INTERVAL * SendMessage(hwndSlider2, TBM_GETPOS, 0, 0);
    
    if(dwBattLevel < BKL_MIN_SETTING)
        dwBattLevel = BKL_MIN_SETTING;
    else if(dwBattLevel > BKL_MAX_SETTING)
        dwBattLevel = BKL_MAX_SETTING;
                    
    if(dwExtLevel < BKL_MIN_SETTING)
        dwExtLevel = BKL_MIN_SETTING;
    else if(dwExtLevel > BKL_MAX_SETTING)
        dwExtLevel = BKL_MAX_SETTING;

    DEBUGMSG(1, (L"Read TrackBar: batt=%d Ext=%d \r\n", dwBattLevel, dwExtLevel));
    
    SetToRegistry(&dwBattLevel, &dwExtLevel, szRegKey, szRegBatteryLevel, szRegExtLevel);  

    // Signal backlight driver to update
    hEvent = CreateEvent(NULL, FALSE, FALSE, EVENTNAME_BACKLIGHTLEVELCHANGEEVENT);
    if(hEvent != NULL)
    {
        SetEvent(hEvent);
        CloseHandle(hEvent);
        DEBUGMSG(1, (L"Change event triggered\r\n"));
    }
}

////////////////////////////////////////////////////////////////////////////////
//
//  void RestoreBklSettings(DWORD dwBattLevel, DWORD dwExtLevel)
//
//  Updates Backlight settings to the registry & signal change event. 
//
//////////////////////////////////////////////////////////////////////////////// 
void RestoreBklSettings(DWORD dwBattLevel, DWORD dwExtLevel)
{
    HANDLE hEvent;
    
    DEBUGMSG(1, (L"Restore: batt=%d Ext=%d \r\n", dwBattLevel, dwExtLevel));
    
    SetToRegistry(&dwBattLevel, &dwExtLevel, szRegKey, szRegBatteryLevel, szRegExtLevel);  

    // Signal backlight driver to update
    hEvent = CreateEvent(NULL, FALSE, FALSE, EVENTNAME_BACKLIGHTLEVELCHANGEEVENT);
    if(hEvent != NULL)
    {
        SetEvent(hEvent);
        CloseHandle(hEvent);
        DEBUGMSG(1, (L"Change event triggered\r\n"));
    }
}
