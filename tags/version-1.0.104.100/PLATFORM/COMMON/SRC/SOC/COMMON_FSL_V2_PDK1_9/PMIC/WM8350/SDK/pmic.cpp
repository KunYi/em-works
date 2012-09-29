//-----------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
///
/// Copyright (c) 2007 Wolfson Microelectronics plc.  All rights reserved.
///
/// This software as well as any related documentation may only be used or
/// copied in accordance with the terms of the Wolfson Microelectronic plc
/// agreement(s) (e.g. licence or non-disclosure agreement (NDA)).
///
/// The information in this file is furnished for informational use only,
/// is subject to change without notice, and should not be construed as a
/// commitment by Wolfson Microelectronics plc.  Wolfson Microelectronics plc
/// assumes no responsibility or liability for any errors or inaccuracies that
/// may appear in the software or any related documention.
///
/// Except as permitted by the agreement(s), no part of the software or any
/// related documention may be reproduced, stored in a retrieval system, or
/// transmitted in any form or by any means without the express written
/// consent of Wolfson Microelectronics plc.
///
/// @file   pmicpdk.c
/// @brief  Platform-specific WM8350 PMIC functions.
///
/// This file contains the PMIC platform-specific functions that provide control
/// over the Power Management IC.
///
/// @version $Id: pmic.cpp 453 2007-05-02 11:33:48Z ib $
///
/// @Warning
///   This software is specifically written for Wolfson devices. It may not be
///   used with other devices.
///
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4201)   // Non-standard extension
#include <windows.h>
#pragma warning(pop)
#include "ceddk.h"

#include "pmic_adc.h"
#include "pmic_audio.h"
#include "WMPmic.h"

//-----------------------------------------------------------------------------
// Global Variables
extern "C" HANDLE               hPMI = WM_HANDLE_INVALID;           // Our global handle to the PMIC driver
extern "C" WM_DEVICE_HANDLE     g_hWMDevice = WM_HANDLE_INVALID;    // Our global Wolfson device handle

#ifdef DEBUG

// Debug zone bit positions
#define ZONEID_ERROR           0
#define ZONEID_WARN            1
#define ZONEID_INIT            2
#define ZONEID_FUNC            3
#define ZONEID_INFO            4

// Debug zone masks
#define ZONEMASK_ERROR         (1 << ZONEID_ERROR)
#define ZONEMASK_WARN          (1 << ZONEID_WARN)
#define ZONEMASK_INIT          (1 << ZONEID_INIT)
#define ZONEMASK_FUNC          (1 << ZONEID_FUNC)
#define ZONEMASK_INFO          (1 << ZONEID_INFO)

// Debug zone args to DEBUGMSG
#define ZONE_ERROR             DEBUGZONE(ZONEID_ERROR)
#define ZONE_WARN              DEBUGZONE(ZONEID_WARN)
#define ZONE_INIT              DEBUGZONE(ZONEID_INIT)
#define ZONE_FUNC              DEBUGZONE(ZONEID_FUNC)
#define ZONE_INFO              DEBUGZONE(ZONEID_INFO)

DBGPARAM dpCurSettings = {
    _T("PMIC"),
    {
        TEXT("Errors"), TEXT("Warnings"), TEXT("Init"), TEXT("Func"),
        TEXT("Info"), TEXT(""), TEXT(""), TEXT(""),
        TEXT(""),TEXT(""),TEXT(""),TEXT(""),
        TEXT(""),TEXT(""),TEXT(""),TEXT("")
    },
    ZONEMASK_ERROR | ZONEMASK_WARN | ZONEMASK_INIT | ZONEMASK_FUNC | ZONEMASK_INFO // ulZoneMask
};

#endif  // DEBUG

extern PMIC_STATUS PmicConvityInit(void);
extern void PmicConvityDeinit(void);

//------------------------------------------------------------------------------
//
// Function: InitPMICSys
//
// This function initializes PMIC subsystems.
//
// Parameters:
//          none.
// Returns:
//              none.
//------------------------------------------------------------------------------
void InitPMICSys(void)
{
    WM_STATUS   status;

    status = WMPmicOpenDevice( &g_hWMDevice );
    if ( !WM_SUCCESS( status ) )
    {
        ERRORMSG(1, (_T("InitPMICSys: WMPmicOpenDevice failed: 0x%X.\r\n"), status ));
    }

    if (PmicADCInit()!=PMIC_SUCCESS)
    {
        ERRORMSG(1, (_T("InitPMICSys: PmicADCInit failed.\r\n")));
    }

    if (PmicAudioDriverInit() != PMIC_SUCCESS)
    {
        ERRORMSG(1, (_T("InitPMICSys: PmicAudioDriverInit failed.\r\n")));
    }
    PmicConvityInit();
}


//------------------------------------------------------------------------------
//
// Function: DeinitPMICSys
//
// This function deinitializes PMIC subsystems.
//
// Parameters:
//          none.
// Returns:
//              none.
//------------------------------------------------------------------------------
void DeinitPMICSys(void)
{
    PmicADCDeinit();

    PmicAudioDriverDeinit();
    PmicConvityDeinit();

    WMPmicCloseDevice( g_hWMDevice );
}

//-----------------------------------------------------------------------------
//
// Function: DllEntry
//
// This is the entry and exit point for the PMIC DDK module.  This
// function is called when processed and threads attach and detach from this
// module.
//
// Parameters:
//      hInstDll
//           [in] The handle to this module.
//
//      dwReason
//           [in] Specifies a flag indicating why the DLL entry-point function
//           is being called.
//
//      lpvReserved
//           [in] Specifies further aspects of DLL initialization and cleanup.
//
// Returns:
//      TRUE if the PMIC is initialized; FALSE if an error occurred during
//      initialization.
//
//-----------------------------------------------------------------------------
BOOL WINAPI
DllEntry(HANDLE hInstDll, DWORD dwReason, LPVOID lpvReserved)
{
    UNREFERENCED_PARAMETER( lpvReserved );

    switch (dwReason) {
    case DLL_PROCESS_ATTACH:
        DEBUGREGISTER((HMODULE)hInstDll);
        DEBUGMSG(ZONE_INIT, (_T("***** DLL PROCESS ATTACH TO PMIC DLL *****\r\n")));
        DisableThreadLibraryCalls((HMODULE) hInstDll);

        hPMI = CreateFile(TEXT("PMI1:"), GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
            FILE_FLAG_RANDOM_ACCESS, NULL);
        if ((hPMI == NULL) || (hPMI == INVALID_HANDLE_VALUE))
        {
            ERRORMSG(1, (_T("Failed in createFile()\r\n")));
        }
        InitPMICSys();
        break;

    case DLL_PROCESS_DETACH:
        if ((hPMI != NULL) && (hPMI != INVALID_HANDLE_VALUE))
        {
            CloseHandle(hPMI);
        }
        DeinitPMICSys();
        break;
    }

    // return TRUE for success
    return TRUE;
}
