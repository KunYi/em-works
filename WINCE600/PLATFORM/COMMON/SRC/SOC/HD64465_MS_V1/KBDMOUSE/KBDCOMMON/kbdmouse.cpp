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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
/*

  Copyright(c) 1998,1999 Renesas Technology Corp.
  Copyright(c) 1998,1999 3RD Rail Engineering.

	Module Name:

		keybdist.cpp

	Revision History:

		26th April 1999		Released
		16th June  1999		Revised
*/

#include <windows.h>
#include <ceddk.h>
#include <dbgapi.h>
#include <ddkreg.h>

#include <laymgr.h>
#include <keybdpdd.h>

#include "ps2p465.hpp"
#include "ps2mouse.hpp"
#include "ps2keybd.hpp"


extern Ps2K465	*v_pp2k;
extern Ps2M465	*v_pp2m;
extern PVBYTE	v_pIoRegs;

static Ps2P465	*v_pp2p;

UINT            v_uiPddId;
PFN_KEYBD_EVENT v_pfnKeybdEvent;


BOOL
ReadRegDWORD(
	LPCWSTR	szKeyName,
	LPCWSTR szValueName,
    LPDWORD pdwValue
    )
{
    HKEY hKeybd;
    DWORD ValType;
    DWORD ValLen;
    DWORD status;
    BOOL fRet = FALSE;
    
    *pdwValue = 0; // Default value to 0

    //
    // Get the device key from the active device registry key
    //
    status = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                szKeyName,
                0,
                0,
                &hKeybd);
    if (status) {
        DEBUGMSG(1, (TEXT("ReadRegDWORD: RegOpenKeyEx(HLM\\%s) returned %d!!!\r\n"),
                  szKeyName, status));
        goto leave;
    }

    ValLen = sizeof(DWORD);
    status = RegQueryValueEx(       // Retrieve the value
                hKeybd,
                szValueName,
                NULL,
                &ValType,
                (PUCHAR)pdwValue,
                &ValLen);
    if (status != ERROR_SUCCESS) {
        DEBUGMSG(1, (TEXT("ReadRegDWORD: RegQueryValueEx(%s) returned %d\r\n"),
                  szValueName, status));
	        goto leave;
    }

	DEBUGMSG(ZONE_INIT, (_T("ReadRegDWORD(): %s -> %s is %d (0x%x)\r\n"), szKeyName, szValueName, *pdwValue, *pdwValue));

	fRet = TRUE;
	
leave:
	if (hKeybd) RegCloseKey(hKeybd);
	return fRet;
}



// Attempt to read I/O window configuration from both the keyboard and the mouse
// configuration keys, since either one may be missing based on sysgen settings.
// Return TRUE if we find an I/O window, FALSE otherwise.
BOOL
Read_MemWindowInfo(PDDKWINDOWINFO pdwi)
{
	HKEY hkReg;
	BOOL fStatus = FALSE;
	LPCTSTR pszKeys[] = {
		_T("HARDWARE\\DEVICEMAP\\KEYBD"),
		_T("HARDWARE\\DEVICEMAP\\MOUSE"),
		NULL
	};
	int i;

	// look for configuration data
	for(i = 0; fStatus == FALSE && pszKeys[i] != NULL; i++) {
		DWORD dwStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, pszKeys[i], 0, 0, &hkReg);
		if(dwStatus == ERROR_SUCCESS) {
			pdwi->cbSize = sizeof(*pdwi);
			if(DDKReg_GetWindowInfo(hkReg, pdwi) == ERROR_SUCCESS && pdwi->dwNumMemWindows == 1) {
				// found a window configuration
				DEBUGMSG(ZONE_INIT, (_T("Read_MemWindowInfo(): found window 0x%08x / 0x%x in '%s'\r\n"), pdwi->memWindows[0].dwBase,
					pdwi->memWindows[0].dwLen, pszKeys[i]));
				fStatus = TRUE;
			}
			RegCloseKey(hkReg);
		}
	}

	return fStatus;
}


void
WINAPI
KeybdPdd_PowerHandler(
	BOOL	bOff
	);
	
static
void
WINAPI
PS2_HD64465_PowerHandler(
    UINT uiPddId,
    BOOL fTurnOff
    )
{
    KeybdPdd_PowerHandler(fTurnOff);
}


static
void 
WINAPI
PS2_HD64465_ToggleLights(
    UINT uiPddId,
    KEY_STATE_FLAGS KeyStateFlags
    )
{
    static const KEY_STATE_FLAGS ksfLightMask = KeyShiftCapitalFlag | 
        KeyShiftNumLockFlag | KeyShiftScrollLockFlag; 
    static KEY_STATE_FLAGS ksfCurr;

    SETFNAME(_T("PS2_ToggleLights"));

    KEY_STATE_FLAGS ksfNewState = (ksfLightMask & KeyStateFlags);

    if (ksfNewState != ksfCurr) 
    {
        DEBUGMSG(ZONE_PDD, (_T("%s: PDD %u: Changing light state\r\n"), 
            pszFname, uiPddId));
        KeybdPdd_ToggleKeyNotification(ksfNewState);
        ksfCurr = ksfNewState;
    }

    return;
}


static KEYBD_PDD PS28042Pdd = {
    PS2_AT_PDD,
    _T("PS/2 HD64465"),
    PS2_HD64465_PowerHandler,
    PS2_HD64465_ToggleLights
};


BOOL
WINAPI
PS2_HD64465_Entry(
    UINT uiPddId,
    PFN_KEYBD_EVENT pfnKeybdEvent,
    PKEYBD_PDD *ppKeybdPdd
    )
{
	SETFNAME(_T("PS2_HD64465_Entry"));

	BOOL    fRet = FALSE;
	DDKWINDOWINFO dwi;

	PVBYTE				iopBase;
	ULONG				inIoSpace = 1;    

	v_uiPddId = uiPddId;
	v_pfnKeybdEvent = pfnKeybdEvent;

	DEBUGMSG(ZONE_INIT, (_T("%s: Initialize keyboard ID %u\r\n"), pszFname, 
		uiPddId));
	PREFAST_DEBUGCHK(ppKeybdPdd != NULL);

	*ppKeybdPdd = &PS28042Pdd;

	if ( v_pp2p )
	{
		fRet = TRUE;
		goto leave;
	}

	if(Read_MemWindowInfo(&dwi) == FALSE) {
		DEBUGMSG(TRUE, (_T("%s: Read_MemWindowInfo() failed\r\n"), pszFname));
		goto leave;
	}

	if ( KeybdDriverInitializeAddresses(dwi.memWindows[0].dwBase,
		dwi.memWindows[0].dwLen) )
	{
		iopBase = v_pIoRegs;
	}
	else
	{
		ERRORMSG( 1, (TEXT("Error mapping I/O Ports.\r\n")));
		goto leave;
	}
	
	v_pp2p = new Ps2P465;
	if ( (v_pp2p == NULL) || !v_pp2p -> Initialize(iopBase) )
	{
		ERRORMSG(1, (TEXT("Could not initialize ps2 port.\r\n")));
		goto leave;
	}
	
	//	We always assume that there is a keyboard.
	v_pp2k = new Ps2K465;
	if ( v_pp2k && v_pp2k -> Initialize(v_pp2p) )
	{
		v_pp2k -> IsrThreadStart();
	}
	else
	{
		ERRORMSG(1, (TEXT("Could not initialize ps2 keyboard.\r\n")));
		delete v_pp2k;
		v_pp2k = NULL;
	}
	if ( v_pp2p -> bMouseFound() )
	{
		v_pp2m = new Ps2M465;
		
		if ( v_pp2m && v_pp2m -> Initialize(v_pp2p) )
		{
			v_pp2m -> IsrThreadStart();
		}
		else
		{
			ERRORMSG(1, (TEXT("Could not initialize ps2 mouse\r\n")));
			delete v_pp2m;
			v_pp2m = NULL;
		}
	}
	
	fRet = TRUE;

leave:
	return fRet;
}
#ifdef DEBUG
// Verify function declaration against the typedef.
static PFN_KEYBD_PDD_ENTRY v_pfnKeybdEntry = PS2_HD64465_Entry;
#endif

