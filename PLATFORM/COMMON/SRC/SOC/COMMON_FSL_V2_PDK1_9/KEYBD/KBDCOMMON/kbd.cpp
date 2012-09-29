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
//  File:  kbd.c
//
//  Implementation of keyboard entry point
//
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <ceddk.h>
#include <keybddbg.h>
#include <keybdpdd.h>
#pragma warning(pop)

#include "common_macros.h"
#include "keypad.hpp"


//------------------------------------------------------------------------------
// External Functions
extern BOOL BSPKppPowerOn();

//------------------------------------------------------------------------------
// External Variables
extern "C" DBGPARAM dpCurSettings;


//------------------------------------------------------------------------------
// Defines
#define ZONEID_PDD         7
#define ZONEID_FUNCTION    13
#define ZONEMASK_PDD       (1 << ZONEID_PDD)
#define ZONEMASK_FUNCTION  (1 << ZONEID_FUNCTION)


//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Global Variables
UINT v_uiPddId;
PFN_KEYBD_EVENT v_pfnKeybdEvent;


//------------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
// Local Functions

void WINAPI KeybdPdd_PowerHandler(BOOL bOff);


//------------------------------------------------------------------------------
//
// Function: KPP_PowerHandler
//
// Called from layout manager MDD to power up or down the keypad.
//
// Parameters:
//      uiPddId -
//          [in] PDD id value.
//
//      fTurnOff -
//          [in] If TRUE, request to turn power off.
//               If FALSE, request to turn power on.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
static void WINAPI KPP_PowerHandler(UINT uiPddId, BOOL fTurnOff)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(uiPddId);
    
    KeybdPdd_PowerHandler(fTurnOff);
}


static KEYBD_PDD KPPPdd = {
    KPP_PDD,
    _T("KPPLayout"),
    KPP_PowerHandler,
    NULL
};


//------------------------------------------------------------------------------
//
// Function: KPP_Entry
//
// Entry function for the keypad PDD.
//
// Parameters:
//      uiPddId -
//          [in] PDD id value.
//
//      pfnKeybdEvent -
//          [in] When the PDD's IST executes, the IST sends the current
//          keyboard event to the keyboard Layout Manager through the
//          pfnKeybdEvent callback function that the Layout Manager
//          identifies to the PDD's entry function.
//
//      ppKeybdPdd -
//          [out] Set to KEYBD_PDD structure for KPP.
//
//
// Returns:
//      TRUE is success, FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL WINAPI KPP_Entry(UINT uiPddId, PFN_KEYBD_EVENT pfnKeybdEvent,
                      PKEYBD_PDD *ppKeybdPdd)
{
    SETFNAME(_T("KPP_Entry"));

    BOOL fRet = FALSE;

    KPP_FUNCTION_ENTRY();

    v_uiPddId = uiPddId;
    v_pfnKeybdEvent = pfnKeybdEvent;

    // dpCurSettings.ulZoneMask |= ZONEMASK_PDD;

    DEBUGMSG(ZONE_INIT, (_T("%s: Initialize KPP ID %u\r\n"), pszFname,
        uiPddId));
    DEBUGCHK(ppKeybdPdd != NULL);

    *ppKeybdPdd = &KPPPdd;

    // We always assume that there is a keyboard.
    if (KppInitialize())
    {
        KppIsrThreadStart();
    }
    else
    {
        ERRORMSG(1,(TEXT("Could not initialize keypad.\r\n")));
    }

    fRet = TRUE;

    KPP_FUNCTION_ENTRY();

    return fRet;
}
#ifdef DEBUG
// Verify function declaration against the typedef.
static PFN_KEYBD_PDD_ENTRY v_pfnKeybdEntry = KPP_Entry;
#endif

