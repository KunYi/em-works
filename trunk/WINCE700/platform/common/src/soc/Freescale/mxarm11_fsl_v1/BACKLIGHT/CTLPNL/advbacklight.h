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
 *Copyright (C) 2003-2004, MOTOROLA, INC. All Rights Reserved
 *
/*---------------------------------------------------------------------------
 * Copyright (C) 2004-2007, Freescale Semiconductor, Inc. All Rights Reserved.
 * THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
 * AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
 *--------------------------------------------------------------------------*/
/*
 *  File:       APPS/ADVBACKLIGHT/advbacklight.h
 *  Purpose:    Function prototypes for code in customised
 *              "Advanced..." button under Display Properties\Backlight tab
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
#ifndef _ADVBACKLIGHT_H_
#define _ADVBACKLIGHT_H_

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <string.h>
#include <Winuser.h>
#include <prsht.h>
#pragma warning(pop)

#include "resource.h"

/*********************************************************************
 MACRO DEFINITIONS
*********************************************************************/
#define EVENTNAME_BACKLIGHTLEVELCHANGEEVENT     L"BackLightLevelChangeEvent"

/*********************************************************************
 ENUMERATIONS AND STRUCTURES
*********************************************************************/
/*********************************************************************
 FUNCTION PROTOTYPES
*********************************************************************/

void GetFromRegistry(DWORD *dwState1, DWORD *dwState2, LPCTSTR lpszRegKey,
                     LPCTSTR lpszUseBat, LPCTSTR lpszUseExt);
void SetToRegistry(DWORD *dwState1, DWORD *dwState2, LPCTSTR lpszRegKey,
                   LPCTSTR lpszRegValue1, LPCTSTR lpszRegValue2);
BOOL APIENTRY AdvancedDlgProc (HWND hDlg, UINT message, UINT wParam, LONG lParam);
extern "C" BOOL BacklightAdvApplet(HWND hDlg);
extern "C" BOOL DllEntry(HANDLE hInstance, DWORD fdwReason, LPVOID lpvReserved);

void InitSliders(HWND hDlg, DWORD dwBattBacklightLevel, DWORD dwExtBacklightLevel);
void UpdateBklSettings(HWND hDlg);
void RestoreBklSettings(DWORD dwBattLevel, DWORD dwExtLevel);

/*********************************************************************
 EXTERN DECLARATIONS
*********************************************************************/
/*********************************************************************
 CLASS DEFINITIONS
*********************************************************************/
#endif  // _ADVBACKLIGHT_H_

/*********************************************************************
 END OF FILE
*********************************************************************/
