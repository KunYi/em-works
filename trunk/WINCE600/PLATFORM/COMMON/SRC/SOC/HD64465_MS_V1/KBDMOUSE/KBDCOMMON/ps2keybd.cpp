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

		ps2keybd.cpp

	Revision History:

		26th April 1999		Released
		16th June  1999		Revised
*/

#include <windows.h>
#include <ceddk.h>
#include <nkintr.h>
//#include <oalintr.h>
#include <pkfuncs.h>

#include <keybddr.h>

#include "ps2p465.hpp"
#include "ps2mouse.hpp"
#include "ps2keybd.hpp"

#include <laymgr.h>
#include <keybdpdd.h>
#include <keybdist.h>


// Scan code consts
static const UINT8 scE0Extended	= 0xe0;
static const UINT8 scE1Extended	= 0xe1;
static const UINT8 scKeyUpMask	= 0x80;

extern PVBYTE	v_pIoRegs;
DWORD dwSysIntr_Keybd;

//	There is really only one physical keyboard supported by the system.
Ps2K465	*v_pp2k;


extern BOOL ReadRegDWORD( LPCWSTR szKeyName, LPCWSTR szValueName, LPDWORD pdwSysIntr );

void
WINAPI
KeybdPdd_PowerHandler(
	BOOL	bOff
	)
{
	return;
}


UINT
WINAPI
KeybdPdd_GetEventEx2(
	UINT			uiPddId,
	UINT32			rguiScanCode[16],
	BOOL			rgfKeyUp[16]
	)
{
	SETFNAME(_T("KeybdPdd_GetEventEx2"));

	UINT8			ui8ScanCode;
	UINT			cEvents = 0;

	static	UINT32	scInProgress;
	static	UINT32	scPrevious;
	static	BOOL	fKeyUp;

	v_pp2k -> m_pp2p -> KeybdDataRead(&ui8ScanCode);

	DEBUGMSG(ZONE_SCANCODES, 
		(_T("%s: scan code 0x%08x, code in progress 0x%08x, previous 0x%08x\r\n"),
		pszFname, ui8ScanCode, scInProgress, scPrevious));

	if ( ui8ScanCode == 0xf0 )
		{
		fKeyUp = TRUE;
		}
	else if ( ui8ScanCode == scE0Extended )
		{
		scInProgress = 0xe000;
		}
	else if ( ui8ScanCode == scE1Extended )
		{
		scInProgress = 0xe10000;
		}
	else if ( scInProgress == 0xe10000 )
		{
		scInProgress |= ui8ScanCode << 8;
		}
	else
		{
		scInProgress |= ui8ScanCode;

		if ( ( scInProgress == scPrevious ) && ( fKeyUp == FALSE ) )
			{
			//	mdd handles auto-repeat so ignore auto-repeats from keybd
			}
		else	// Not a repeated key.  This is the real thing.
			{
			//	The Korean keyboard has two keys which generate a single
			//	scan code when pressed.  The keys don't auto-repeat or
			//	generate a scan code on release.  The scan codes are 0xf1
			//	and 0xf2.  It doesn't look like any other driver uses
			//	the 0x71 or 0x72 scan code so it should be safe.

			//	If it is one of the Korean keys, drop the previous scan code.
			//	If we didn't, the earlier check to ignore auto-repeating keys
			//	would prevent this key from working twice in a row.  (Since the
			//	key does not generate a scan code on release.)
			if ( ( fKeyUp == TRUE ) ||
				 ( scInProgress == 0xf1 ) ||
				 ( scInProgress == 0xf2 ) )
				{
				scPrevious = 0;
				}
			else 
				{
				scPrevious = scInProgress;
				}
				
			rguiScanCode[cEvents] = scInProgress;
			rgfKeyUp[cEvents] = fKeyUp;
			++cEvents;
			}
			
		scInProgress = 0;
		fKeyUp = FALSE;
		}

	v_pp2k -> m_pp2p -> KeybdInterruptEnable();

	return cEvents;
}



void
WINAPI
KeybdPdd_ToggleKeyNotification(
	KEY_STATE_FLAGS	KeyStateFlags
	)
{
	unsigned int	fLights;

	fLights = 0;

	if ( KeyStateFlags & KeyShiftCapitalFlag )
		{
		fLights |= 0x04;
		}

	if ( KeyStateFlags & KeyShiftNumLockFlag )
		{
		fLights |= 0x02;
		}

	if ( KeyStateFlags & KeyShiftScrollLockFlag )
		{
		fLights |= 0x1;
		}

	v_pp2k -> m_pp2p -> KeyboardLights(fLights);

	return;
}


BOOL
Ps2K465::
IsrThreadProc(
	void
	)
{
	LPCTSTR szKey = TEXT("HARDWARE\\DEVICEMAP\\KEYBD");

	DWORD dwEnableWake;
	DWORD dwTransferred;
	
	m_hevInterrupt = CreateEvent(NULL, FALSE, FALSE, NULL);
	if ( m_hevInterrupt == NULL)
		{
		goto leave;
		}

	ReadRegDWORD(szKey, _T("SysIntr"), &dwSysIntr_Keybd);
	if(! dwSysIntr_Keybd) {
		goto leave;
	}


	if ( !InterruptInitialize(dwSysIntr_Keybd, m_hevInterrupt, NULL, 0) )
		{
		goto leave;
		}

	if (ReadRegDWORD(szKey, TEXT("EnableWake"), &dwEnableWake)) {
		if (dwEnableWake != 0) {
			m_pp2p->SetWake(TRUE);
		}
	}

	if (m_pp2p->WillWake()) {
		// Ask the OAL to enable our interrupt to wake the system from suspend.
		DEBUGMSG(ZONE_INIT, (TEXT("Keyboard: Enabling wake from suspend\r\n")));		 	
		BOOL fErr = KernelIoControl(IOCTL_HAL_ENABLE_WAKE, &dwSysIntr_Keybd,  
			sizeof(dwSysIntr_Keybd), NULL, 0, &dwTransferred);
	}

	m_pp2p -> KeybdInterruptEnable();

	extern UINT v_uiPddId;
	extern PFN_KEYBD_EVENT v_pfnKeybdEvent;

	KEYBD_IST keybdIst;
	keybdIst.hevInterrupt = m_hevInterrupt;
	keybdIst.dwSysIntr_Keybd = dwSysIntr_Keybd;
	keybdIst.uiPddId = v_uiPddId;
	keybdIst.pfnGetKeybdEvent = KeybdPdd_GetEventEx2;
	keybdIst.pfnKeybdEvent = v_pfnKeybdEvent;

	KeybdIstLoop(&keybdIst);



leave:
	return 0;
}



DWORD
Ps2K465IsrThread(
	Ps2K465	*pp2k
	)
{

	pp2k -> IsrThreadProc();

	return 0;
}




BOOL
Ps2K465::
IsrThreadStart(
	void
	)
{
	HANDLE	hthrd;

	hthrd = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Ps2K465IsrThread, this, 0, NULL);

	//	Since we don't need the handle, close it now.
	CloseHandle(hthrd);
	return TRUE;
}






BOOL
Ps2K465::
Initialize(
	Ps2P465	*pp2p
	)
{
	BOOL	bRet = FALSE;

	m_pp2p = pp2p;
	if ( !m_pp2p -> KeyboardInterfaceTest() )
		{
		ASSERT(0);
		goto leave;
		}

	v_pp2k -> m_pp2p -> KeyboardLights(0);

	return bRet = TRUE;

leave:
	return bRet;
}


