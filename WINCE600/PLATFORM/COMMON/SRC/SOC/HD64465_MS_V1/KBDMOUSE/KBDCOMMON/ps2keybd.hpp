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

		ps2keybd.hpp

	Revision History:

		26th April 1999		Released
		16th June  1999		Revised
*/

#ifndef __PS2KEYBD_HPP_INCLUDED__
#define __PS2KEYBD_HPP_INCLUDED__


#include <windows.h>
#include <keybd.h>

class Ps2P465;

class Ps2K465
	{
	Ps2P465	*m_pp2p;
	HANDLE	m_hevInterrupt;


public:
	BOOL
	Initialize(
		Ps2P465	*pp2p
		);

	BOOL
	IsrThreadStart(
		void
		);


	BOOL
	IsrThreadProc(
		void
		);

friend
	UINT
	WINAPI
	KeybdPdd_GetEventEx2(
		UINT			uiPddId,
		UINT32			rguiScanCode[16],
		BOOL			rgfKeyUp[16]
		);

friend
	void
	WINAPI
	KeybdPdd_ToggleKeyNotification(
		KEY_STATE_FLAGS	KeyStateFlags
		);


	};













#endif





