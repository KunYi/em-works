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

		ps2p465.hpp

	Revision History:

		26th April 1999		Released
		16th June  1999		Revised
*/

#ifndef __PS2P465_HPP_INCLUDED__
#define __PS2P465_HPP_INCLUDED__



#include <windows.h>
#include "macros.h"

//#include "mobytel.h"

/*
 * Keyboard and Mouse(AUX) port interface.
 *
 * Supports the PS/2 port in the HD64465 companion chip
 *
 * (** cea 6/3/99)
 */

 /*
 * Keyboard (HD64465) defines 
 */
//#define KB_BASE 			(HD64465_BASE + HD64465_KBC_OFFSET)
#define KB_ISR				0x0C04		/* offset from keyboard base reg */
#define KB_CSR				0x0C00		/* offset from keyboard base reg */
#define KB_INTR_MASK		0x0001
#define KB_RDRF 			0x0001
#define KB_PARITY			0x0100			
#define KB_DATA_RD			0x0200
#define KB_CLK_RD			0x0400
#define KB_DATA_DR			0x0800
#define KB_CLK_DR			0x1000
#define KB_DATA_DR_EN		0x2000
#define KB_CLK_DR_EN		0x4000
#define KB_CLK_EN			0x8000

/* 
 * Mouse (HD64465) defines
 *
 * This version has the mouse base address equal 
 * to the keyboard base address.
 */
//#define MOUSE_BASE			(HD64465_BASE + HD64465_KBC_OFFSET)
#define MOUSE_ISR			0x0C14		/* offset from keyboard base reg */
#define MOUSE_CSR			0x0C10		/* offset from keyboard base reg */
#define MOUSE_INTR_MASK 	0x0001
#define MOUSE_RDRF			0x0001
#define MOUSE_PARITY		0x0100			
#define MOUSE_DATA_RD		0x0200
#define MOUSE_CLK_RD		0x0400
#define MOUSE_DATA_DR		0x0800
#define MOUSE_CLK_DR		0x1000
#define MOUSE_DATA_DR_EN	0x2000
#define MOUSE_CLK_DR_EN 	0x4000
#define MOUSE_CLK_EN		0x8000


/*++

Ps2P465:

This is a very simple minded PS/2 interface.

The HD64465 has very limited hardware support for the PS/2.

There is an input shift register but no output shift register.

This means that when sending commands to a PS/2 device they
are sent one bit at a time.

It does not even try to emmulate i8042 chip. It is intended 
to provide only basic keyboard and mouse support.


--*/
class Ps2P465
	{
	PVBYTE				m_iopBase;
	CRITICAL_SECTION	m_csWrite;

	int					m_cEnterWrites;
	BOOL				m_bMouseFound;
	bool				m_bIntelliMouseFound;

	UINT8				m_ui8CmdByte;

	BOOL				m_fEnableWake;

	BOOL
	MouseTest(
		void
		);


public:

	BOOL
	Initialize(
		PVBYTE			iopBase
		);

	BOOL
	bMouseFound(
		void
		)
	{
	return m_bMouseFound;
	}

	BOOL
	MouseReset(
		void
		);

	BOOL
	MouseDataRead(
		UINT8	*pui8Data
		);

	BOOL
	MousePollRead(
		UINT8	*pui8
		);

	BOOL
	MouseInterruptEnable(
		void
		);

	BOOL
	MouseCommandPut(
		UINT8	ui8Cmd
		);


	BOOL
	KeyboardInterfaceTest(
		void
		);

	BOOL
	KeyboardCommandPut(
		UINT8	ui8Cmd
		);

	BOOL
	KeyboardReset(
		void
		);

	void
	KeyboardLights(
		unsigned int	fLights
		);

	BOOL
	KeybdInterruptEnable(
		void
		);

	BOOL
	KeybdDataRead(
		UINT8	*pui8Data
		);

	BOOL
	KeyboardPollRead(
		UINT8	*pui8
		);


    void SetModeIntelliMouse( void );
    UINT8 MouseId( 	void  );
    bool bIntelliMouseFound( void ) { return m_bIntelliMouseFound; }

	// Note: On a PS/2 controller, if you have both a keyboard and mouse
	// connected and one is set to wake but the other is not, there is a good
	// chance that neither will wake the system. If the non-wake source 
	// has activity, its data will fill the controller's output buffer, but it will not
	// be removed. Then, if the wake source has activity, it will wait for the
	// controller to accept its data, but this will not happen until the data in
	// the controller's output buffer is read. Thus, no activity on either device 
	// will wake the system. Moral: Have both the keyboard and mouse set as
	// wake sources or neither set as wake sources. We enforce that here.
	void
	SetWake(
		BOOL fEnable
		)
	{
	m_fEnableWake = fEnable;
	}

	BOOL
	WillWake(
		void
		)
	{
	return m_fEnableWake;
	}    

};

	BOOL KeybdDriverInitializeAddresses( 
		DWORD	dwBase,
		DWORD	dwLen
		);

#endif

