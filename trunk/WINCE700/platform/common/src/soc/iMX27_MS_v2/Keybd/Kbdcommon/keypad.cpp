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
//
//  Copyright (C) 2004, Motorola Inc. All Rights Reserved
//
//------------------------------------------------------------------------------
//
// Copyright (C) 2006, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  File:  Keypad.cpp
//
//  Implementation of CSP-specific Keypad methods for Keypad Port Driver
//
//------------------------------------------------------------------------------
#include <windows.h>
#include <winbase.h>
#include <Devload.h>
#include <types.h>
#include <nkintr.h>
#include <ceddk.h>
#include "csp.h"

#undef ZONE_INIT

#include <keybddbg.h>
#include <keybddr.h>
#include <keybdpdd.h>
#include <keybdist.h>
#include "Keypad.hpp"


//------------------------------------------------------------------------------
// External Functions
extern void BSPKppRegInit();
extern UINT BSPKPPGetScanCodes(UINT32 rguiScanCode[16], BOOL rgfKeyUp[16]);
extern BOOL BSPKppPowerOn();
extern BOOL BSPKppPowerOff();


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
PCSP_KPP_REGS g_pKPP = NULL;
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
BOOL KppIsrThreadProc()
{
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
    PHYSICAL_ADDRESS phyAddr;
    DWORD dwIrq_Keybd = IRQ_KPP;

    KPP_FUNCTION_ENTRY();

    phyAddr.QuadPart = CSP_BASE_REG_PA_KPP;

    //map KPP memory space
    g_pKPP = (PCSP_KPP_REGS) MmMapIoSpace(phyAddr, sizeof(CSP_KPP_REGS), FALSE);
    if(g_pKPP == NULL)
    {
        RETAILMSG(1, (TEXT("%s: KPP memory space mapping failed!\r\n"), __WFUNCTION__));
        return FALSE;
    }

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
        g_dwSysIntr_Keybd = SYSINTR_UNDEFINED;
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

#if 1
    DEBUGMSG(ZONE_INIT, (_T("%s: About to enable the wake source %d %d\r\n"), 
       __WFUNCTION__, g_dwSysIntr_Keybd, IRQ_KPP));
    // Ask the OAL to enable our interrupt to wake the system from suspend.
    KernelIoControl(IOCTL_HAL_ENABLE_WAKE, &g_dwSysIntr_Keybd, 
        sizeof(g_dwSysIntr_Keybd), NULL, 0, NULL);
    DEBUGMSG(ZONE_INIT, (_T("%s: Done enabling the wake source\r\n"), __WFUNCTION__));
#endif

    DEBUGMSG(ZONE_INIT, 
        (TEXT("%s: KPP virtual address: %x\r\n"), __WFUNCTION__, g_pKPP));

    BSPKppRegInit();    

    KPP_FUNCTION_EXIT();

    return TRUE;
}
