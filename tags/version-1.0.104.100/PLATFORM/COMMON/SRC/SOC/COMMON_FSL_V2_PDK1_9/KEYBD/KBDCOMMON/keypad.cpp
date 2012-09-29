//------------------------------------------------------------------------------
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
//  File:  Keypad.cpp
//
//  Implementation of CSP-specific Keypad methods for Keypad Port Driver
//
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <winbase.h>
#include <Devload.h>
#include <types.h>
#include <nkintr.h>
#include <ceddk.h>
#pragma warning(pop)

#include "common_macros.h"

#undef ZONE_INIT

#include <keybddbg.h>
#include <keybddr.h>
#include <keybdpdd.h>
#include <keybdist.h>
#include "Keypad.hpp"


//------------------------------------------------------------------------------
// External Functions
extern UINT32 CSPKppGetIrq(VOID);;
extern BOOL BSPKppRegInit();
extern UINT BSPKPPGetScanCodes(UINT32 rguiScanCode[16], BOOL rgfKeyUp[16]);
extern BOOL BSPKppPowerOn();
extern BOOL BSPKppPowerOff();
extern BOOL BSPKppIsWakeUpSource();


//------------------------------------------------------------------------------
// External Variables
extern UINT v_uiPddId;
extern PFN_KEYBD_EVENT v_pfnKeybdEvent;


//------------------------------------------------------------------------------
// Defines


//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Global Variables
HANDLE g_hevInterrupt = NULL;
DWORD g_dwSysIntr_Keybd;


//------------------------------------------------------------------------------
// Local Variables
static HANDLE g_hIntrThread = NULL;


//------------------------------------------------------------------------------
// Local Functions


//------------------------------------------------------------------------------
//
// Function: KeybdPdd_PowerHandler
//
// Power on or off the keyboard.
//
// Parameters:
//      bOff -
//          [in] If TRUE, request to power down.
//               If FALSE, request to power up.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void WINAPI KeybdPdd_PowerHandler(BOOL bOff)
{
    KPP_FUNCTION_ENTRY();

    if (!bOff)
       BSPKppPowerOn();
    else
       BSPKppPowerOff();

    KPP_FUNCTION_EXIT();
}


//------------------------------------------------------------------------------
//
// Function: KeybdPdd_GetEventEx2
//
// Keyboard event entry point.  Determines scan codes for key events.
//
// Parameters:
//      uiPddId -
//          [in] PDD id value.
//
//      rguiScanCode[16] -
//          [out] An array of scan codes for each key event detected.
//
//      rgfKeyUp[16] -
//          [out] An array of BOOLs indicating, for each key event,
//          whether the key has gone up or down.
//
//
// Returns:
//      The number of keypad events.
//
//------------------------------------------------------------------------------
static UINT KeybdPdd_GetEventEx2(UINT uiPddId, UINT32 rguiScanCode[16],
                                     BOOL rgfKeyUp[16])
{
    UINT            cEvents = 0;
    UINT            iEvent;

    KPP_FUNCTION_ENTRY();

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(uiPddId);

    SETFNAME(_T("KeybdPdd_GetEventEx2"));

    DEBUGCHK(rguiScanCode != NULL);
    DEBUGCHK(rgfKeyUp != NULL);

    // Here we must read and somehow get scan codes from the KPP
    cEvents = BSPKPPGetScanCodes(rguiScanCode, rgfKeyUp);

    iEvent = 0;
    // Print out scan codes and key up status for each key
    while (iEvent < cEvents)
    {
        DEBUGMSG(ZONE_PDD, (_T("%s: event 0x%08x: ScanCode 0x%08x, %c\r\n"),
            pszFname, iEvent, rguiScanCode[iEvent],
            rgfKeyUp[iEvent] ? 'u' : 'd'));
        iEvent++;
    }

    DEBUGMSG(ZONE_PDD,
        (_T("%s: keypad events 0x%08x\r\n"), pszFname, cEvents));

    KPP_FUNCTION_EXIT();

    return cEvents;
}


//------------------------------------------------------------------------------
//
// Function: KppIsrThreadProc
//
// ISR Thread Process that initializes Keypad interrupt and launches IST loop.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if success, FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL KppIsrThreadProc(
    PVOID Reserved  //@parm Reserved, not used.
    )
{
    UNREFERENCED_PARAMETER(Reserved);

    KEYBD_IST keybdIst;

    KPP_FUNCTION_ENTRY();

    keybdIst.hevInterrupt = g_hevInterrupt;
    keybdIst.dwSysIntr_Keybd = g_dwSysIntr_Keybd;
    keybdIst.uiPddId = v_uiPddId;
    keybdIst.pfnGetKeybdEvent = KeybdPdd_GetEventEx2;
    keybdIst.pfnKeybdEvent = v_pfnKeybdEvent;

    KeybdIstLoop(&keybdIst);

    KPP_FUNCTION_EXIT();

    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: KppIsrThreadStart
//
// Creates thread to run keypad ISR.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if success, FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL KppIsrThreadStart()
{
    HANDLE hthrd;

    KPP_FUNCTION_ENTRY();

    hthrd = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)KppIsrThreadProc,
                         0, 0, NULL);
    if (!hthrd)
    {
        DEBUGMSG(ZONE_ERROR,
                 (TEXT("KppIsrThreadStart: CreateThread failed\r\n")));
        return FALSE;
    }
    // Since we don't need the handle, close it now.
    CloseHandle(hthrd);

    KPP_FUNCTION_EXIT();

    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: KppInitialize
//
// Initializes the keypad and allocates memory for the global keypad pointer.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if success, FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL KppInitialize()
{
    DWORD dwIrq_Keybd = CSPKppGetIrq();

    KPP_FUNCTION_ENTRY();

    // Create Keypad interrupt event
    g_hevInterrupt = CreateEvent(NULL,FALSE,FALSE,NULL);
    if (g_hevInterrupt == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("IsrThreadProc: InterruptInitialize\r\n")));
        return FALSE;
    }

    // Call the OAL to translate the IRQ into a SysIntr value.
    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &dwIrq_Keybd,
        sizeof(DWORD), &g_dwSysIntr_Keybd, sizeof(DWORD), NULL))
    {
        RETAILMSG(1, (TEXT("ERROR: Failed to obtain sysintr value for keyboard interrupt.\r\n")));
        g_dwSysIntr_Keybd = (DWORD)SYSINTR_UNDEFINED;
        return FALSE;
    }

    // Initialize KPP interrupt
    if (!InterruptInitialize(g_dwSysIntr_Keybd, g_hevInterrupt, NULL, 0))
    {
        CloseHandle(g_hevInterrupt);
        g_hevInterrupt = NULL;
        DEBUGMSG(ZONE_ERROR, (TEXT("IsrThreadProc: KeybdInterruptEnable failed\r\n")));
        return FALSE;
    }

    // Set the Keypad as a wake up source if defined by the BSP
    if (BSPKppIsWakeUpSource())
    {
        DEBUGMSG(ZONE_INIT, (_T("%s: About to enable the wake source %d %d\r\n"),
                             __WFUNCTION__, g_dwSysIntr_Keybd, dwIrq_Keybd));
        // Ask the OAL to enable our interrupt to wake the system from suspend.
        KernelIoControl(IOCTL_HAL_ENABLE_WAKE, &g_dwSysIntr_Keybd,
            sizeof(g_dwSysIntr_Keybd), NULL, 0, NULL);
        DEBUGMSG(ZONE_INIT, (_T("%s: Done enabling the wake source\r\n"),
                             __WFUNCTION__));
        DEBUGMSG(ZONE_INIT,
                 (TEXT("[KPP]%s:  irq=0x%x, sysIrq=0x%x\r\n"),
                 __WFUNCTION__, dwIrq_Keybd, g_dwSysIntr_Keybd));
    }

    if (!(BSPKppRegInit()))
    {
        RETAILMSG(1, (TEXT("[KPP] %s: fail to BSPKppRegInit!\r\n"),
                  __WFUNCTION__));
        return FALSE;
    }

    KPP_FUNCTION_EXIT();

    return TRUE;
}
