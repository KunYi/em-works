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
// Copyright (C) 2006, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  File:  kbd.c
//
//  Implementation of keyboard entry point
//
//------------------------------------------------------------------------------
#include <windows.h>
#include <ceddk.h>
#include <keybddbg.h>
#include <keybdpdd.h>
#include "keypad.hpp"
#include "csp.h"


//------------------------------------------------------------------------------
// External Functions
extern BOOL BSPKppPowerOn();
extern BOOL BSPKppPowerOff();

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
CEDEVICE_POWER_STATE g_KppPowerState = D0;


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
// Function: KeybdPdd_IOControl
//
// Call back function in PDD for handling the IOcontrol codes from MDD
// This function operates differently based upon the IOCTL that is passed to it.
// The following table describes the expected values associated with each
// IOCTL implemented by this function.
// dwCode                      pBufIn         pBufOut         Description
// --------------------------- -------------- --------------- -----------------
//
// Parameters:
//      dwCode
//          [in] The IOCTL requested.
//
//      pBufIn
//          [in] Input buffer.
//
//      dwLenIn
//          [in] Length of the input buffer.
//
//      pdwBufOut
//          [out] Output buffer.
//
//      dwLenOut
//          [out] The length of the output buffer.
//
//      pdwActualOut
//          [out] Size of output buffer returned to application.
//
// Returns:
//      TRUE if the IOCTL is handled. FALSE if the IOCTL was not recognized or
//      an error occurred while processing the IOCTL
//
//
//------------------------------------------------------------------------------
BOOL KeybdPdd_IOControl(
    DWORD                        dwCode,
    __in_bcount(dwLenIn) PVOID   pBufIn,
    DWORD                         dwLenIn,
    __out_bcount(dwLenOut) PDWORD pdwBufOut,
    DWORD                         dwLenOut,
    __out PDWORD                  pdwActualOut
    )
{
    BOOL fRetVal = FALSE;
    switch (dwCode)
    {
    case IOCTL_POWER_SET:
        DEBUGMSG(ZONE_FUNCTION, (L"KeybdPdd_IOControl: IOCTL_POWER_SET\r\n"));
        if(pdwBufOut != NULL && dwLenOut == sizeof(CEDEVICE_POWER_STATE))
        {
            CEDEVICE_POWER_STATE dx = *(CEDEVICE_POWER_STATE*)pdwBufOut;
            if( VALID_DX(dx) )
            {
                DEBUGMSG(ZONE_FUNCTION, (L"KeybdPdd_IOControl: IOCTL_POWER_SET = to D%u\r\n", dx));
                g_KppPowerState = dx;

                if (pdwActualOut)
                    *pdwActualOut = sizeof(CEDEVICE_POWER_STATE);

                //  Enable Keypad for D0, D1 and disable for D2,D3 and D4
                switch( dx )
                {
                    case D0:
                    case D1:
                        //  Turn ON Keypad power
                        BSPKppPowerOn();
                        break;

                    case D2:
                    case D3:
                    case D4:
                        // Turn Off Keypad power
                        BSPKppPowerOff();
                        break;

                    default:
                        break;
                }

                fRetVal = TRUE;
            }
        }
        break;

    case IOCTL_POWER_GET:
        DEBUGMSG(ZONE_FUNCTION, (L"KeybdPdd_IOControl: IOCTL_POWER_GET\r\n"));
        if(pdwBufOut != NULL && dwLenOut == sizeof(CEDEVICE_POWER_STATE))
        {
            *(PCEDEVICE_POWER_STATE) pdwBufOut = g_KppPowerState;

            if (pdwActualOut)
                *pdwActualOut = sizeof(CEDEVICE_POWER_STATE);

            fRetVal = TRUE;
        }
        break;

    // Return device specific power capabilities.
    case IOCTL_POWER_CAPABILITIES:
        DEBUGMSG(ZONE_FUNCTION, (L"KeybdPdd_IOControl: IOCTL_POWER_CAPABILITIES\r\n"));
        if (pdwBufOut != NULL && dwLenOut == sizeof(POWER_CAPABILITIES))
        {
            PPOWER_CAPABILITIES ppc = (PPOWER_CAPABILITIES) pdwBufOut;
            memset(ppc, 0, sizeof(POWER_CAPABILITIES));
            ppc->DeviceDx = DX_MASK(D0) | DX_MASK(D1) | DX_MASK(D2) | DX_MASK(D3) | DX_MASK(D4);

            if (pdwActualOut)
                *pdwActualOut = sizeof(POWER_CAPABILITIES);

            fRetVal = TRUE;
        }
        break;

    default:
        // no other control codes are supported
        break;
    }

    return fRetVal;
}


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

    //************
    KEYBD_CALLBACKS Callbacks;
    ZeroMemory(&Callbacks, sizeof(Callbacks));
    Callbacks.dwSize = sizeof Callbacks;
    Callbacks.pfnPddExits = NULL;
    Callbacks.pfnPddIoControl = KeybdPdd_IOControl;
    Callbacks.pfnPddGetPowerCaps = NULL;
    KeybdMDDRegisterCallbacks(uiPddId, &Callbacks);

    // {CBE6DDF2-F5D4-4e16-9F61-4CCC0B6695F3}
    static const GUID PS2GUID =
    {0xCBE6DDF2, 0xF5D4, 0x4e16, {0x9F, 0x61, 0x4C, 0xCC, 0x0B, 0x66, 0x95, 0xF3} };

    KeybdMDDRegisterGUID(uiPddId, &PS2GUID);
    //************


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

    BSPKppPowerOn();

    fRet = TRUE;

    KPP_FUNCTION_ENTRY();

    return fRet;
}
#ifdef DEBUG
// Verify function declaration against the typedef.
static PFN_KEYBD_PDD_ENTRY v_pfnKeybdEntry = KPP_Entry;
#endif

