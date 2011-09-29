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
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.
Copyright (c) 1995-1998  Microsoft Corporation

	@doc	EXTERNAL DRIVERS

	@module	vkrmpsimple.cpp |

This file implements the virtual key remapping part of the platform 
independent code of the keyboard driver.  This is provided as a sample to 
platform driver writers and is expected to be able to be used without 
major modification on most hardware platforms.  

*/

#include <windows.h>
#include <keybddr.h>
#include <keybdpdd.h>


#define VK_NUMPAD_FIRST	VK_NUMPAD0
#define VK_NUMPAD_LAST	VK_DECIMAL
#define C_VK_NUMPAD		(VK_NUMPAD_LAST-VK_NUMPAD_FIRST+1)

#define InNumPadRange(x) ( ((x) >= VK_NUMPAD_FIRST) && ((x) <= VK_NUMPAD_LAST) )


//	Keep track of the last vkey actually sent for keys which have multiple 
//	virtual keys.  
static UINT32	v_NumPadVKeySent[C_VK_NUMPAD];


//	Keep track of the numlock state.  This will cause problems when
//	numlock is toggled from the software.
//static BOOL		v_bNumLockToggled;


//	Keep track of other keystates.
static BOOL		v_bNumLockDown;
static BOOL		v_bLShiftDown;
static BOOL		v_bRShiftDown;

//	Keep track of fake key events.
static BOOL		v_bShadowLShiftDown;
static BOOL		v_bShadowRShiftDown;

static UINT32	v_scLShift;
static UINT32	v_scRShift;


//	Number pad keys get remapped when numlock is off.
static UINT32	v_NumPadVKeyNumLockOff[C_VK_NUMPAD] =
	{
	VK_INSERT,		//	VK_NUMPAD0        0x60
	VK_END,			//	VK_NUMPAD1        0x61
	VK_DOWN,		//	VK_NUMPAD2        0x62
	VK_NEXT,		//  VK_NUMPAD3        0x63
	VK_LEFT,		//  VK_NUMPAD4        0x64
	0x0c,			//	VK_NUMPAD5        0x65
	VK_RIGHT,		//	VK_NUMPAD6        0x66
	VK_HOME,		//	VK_NUMPAD7        0x67
	VK_UP,			//	VK_NUMPAD8        0x68
	VK_PRIOR,		//	VK_NUMPAD9        0x69
	VK_MULTIPLY,	//	VK_MULTIPLY       0x6A
	VK_ADD,			//	VK_ADD            0x6B
	VK_SEPARATOR,	//	VK_SEPARATOR      0x6C
	VK_SUBTRACT,	//	VK_SUBTRACT       0x6D
	VK_DELETE		//	VK_DECIMAL        0x6E
	};


static
BOOL
AnyNumPadKeyDown(
	void
	)
{
	int	i;

	for ( i = 0; i < C_VK_NUMPAD; i++ )
		{
		if ( v_NumPadVKeySent[i] )
			return TRUE;
		}
	return FALSE;
}





/*++

KeybdDriverRemapVKeyDown:

On a key down, do any virtual key re-mapping on the keyboard.


Notes:

The basic pattern for each special key is to see if we are already sending 
a virtual key and auto-repeat it or check if its particular modifier is 
down and send the modified key or just send the unmodified key.  

For modified keys, we need to make it look like the modifier key went up 
and then the desired virtual key went down.  Additionally, when the 
modifier is the Alt key, we need to send a null character before the Alt 
up so that menus do not activate.  

--*/
int
KeybdDriverRemapVKeyDownEx(
	UINT32			VirtualKey,
	UINT32			ScanCode,
	KEY_STATE_FLAGS	KeyStateFlags,
	UINT32			RemapVKeyBuf[16],
	UINT32			RemapScanCodeBuf[16],
	KEY_STATE_FLAGS	RemapKeyStateFlagsBuf[16]
	)
{
	int		i = 0;
	UINT32	vkUp1 = 0;
	UINT32	vkUp2 = 0;
	UINT32	scUp1 = 0;
	UINT32	scUp2 = 0;
	UINT32	vkDown;
	UINT32	vkOnly = VirtualKey & 0xff;		//	Just the vkey
	UINT32	vkOther = VirtualKey & ~0xff;	//	Just the other stuff


	//	Normally, we just send the vkey that came in.
	vkDown = vkOnly;

	//	Track numlock toggle.  We aren't doing anything with this right now.
	if ( vkOnly == VK_NUMLOCK )
		{
		if ( !v_bNumLockDown )
			{
			v_bNumLockDown = TRUE;
			}
		}
	else if ( vkOnly == VK_LSHIFT )
		{
		v_bLShiftDown = TRUE;
		v_bShadowLShiftDown = TRUE;
		v_scLShift = ScanCode;
		}
	else if ( vkOnly == VK_RSHIFT )
		{
		v_bRShiftDown = TRUE;
		v_bShadowRShiftDown = TRUE;
		v_scRShift = ScanCode;
		}
	else if ( InNumPadRange(vkOnly) )
		{
		int	NumPadIndex = vkOnly - VK_NUMPAD_FIRST;

//		RETAILMSG(1,(TEXT("Numpad\r\n")));

		if ( v_NumPadVKeySent[NumPadIndex] ) 			// Key is auto-repeating.
			{
			vkDown = v_NumPadVKeySent[NumPadIndex];		// Send out whatever we sent last time.
			}
		else if ( v_bLShiftDown || v_bRShiftDown )
			{
			//	If numlock is toggled, shifts undo the numlock toggle.
			//	We undo the shifts by generating up events for the shift
			//	keys.  This causes the real state and the shadow state to
			//	differ.  Note that this will only be done for the first
			//	number pad key if more than one is pressed.
			if ( v_bLShiftDown && v_bShadowLShiftDown )
				{
				vkUp1 = VK_LSHIFT;
				scUp1 = v_scLShift;
				v_bShadowLShiftDown = FALSE;
				}

			if ( v_bRShiftDown && v_bShadowRShiftDown )
				{
				vkUp2 = VK_RSHIFT;
				scUp2 = v_scRShift;
				v_bShadowRShiftDown = FALSE;
				}
			}
		else
			{
			//	Numlock is toggled but no shift, use the regular numpad vkey.
			}

		//	When auto-repeating, this gets set again, but it doesn't hurt since
		//	it is the same vkey.
		v_NumPadVKeySent[NumPadIndex] = vkDown;
		}

	//	Now generate the necessary key events.
	//	If necessary, make it look like the shift key(s) went up.
	if ( vkUp1 )
		{
		RemapVKeyBuf[i] = vkUp1;
		RemapScanCodeBuf[i] = scUp1;
		RemapKeyStateFlagsBuf[i] = 0;
		i++;
		}

	if ( vkUp2 )
		{
		RemapVKeyBuf[i] = vkUp2;
		RemapScanCodeBuf[i] = scUp2;
		RemapKeyStateFlagsBuf[i] = 0;
		i++;
		}

	//	Finally!  Send the key down.
	RemapVKeyBuf[i] = vkDown | vkOther;
	RemapScanCodeBuf[i] = ScanCode;
	RemapKeyStateFlagsBuf[i] = KeyStateDownFlag;
	i++;

//	NKDbgPrintfW(TEXT("VkeyDown: vkeybuf[i]=%x scancode=%x Flags=%x\r\n"),RemapVKeyBuf[i-1],RemapScanCodeBuf[i-1],RemapKeyStateFlagsBuf[i-1]);
	return i;
}





/*++

KeybdDriverRemapKeyUp:

On a key up, undo all of the virtual key re-mapping on the keyboard.  


Notes:

When a special key is released, we send an up event for whatever virtual 
key was original sent as going down.  

If the special key's modifier key is still down, we send a down event for 
it to keep the state consistent.  (We earlier sent an up event for the modifier
when the special key went down.).

We remember if the modifier key we are resending the down for is the Alt 
key.  If it is, when it is really released, we send a null through the 
system to keep menus from activating.  Otherwise, menus would see this 
down followed directly by an Alt up and so would activate.  

--*/
int
KeybdDriverRemapVKeyUpEx(
	UINT32			VirtualKey,
	UINT32			ScanCode,
	KEY_STATE_FLAGS	KeyStateFlags,
	UINT32			RemapVKeyBuf[16],
	UINT32			RemapScanCodeBuf[16],
	KEY_STATE_FLAGS	RemapKeyStateFlagsBuf[16]
	)
{
	int		i = 0;
	UINT32	vkUp;
	BOOL	bGenUp = TRUE;
	UINT32	vkDown1 = 0;
	UINT32	scDown1 = 0;
	UINT32	vkDown2 = 0;
	UINT32	scDown2 = 0;
	UINT32	vkOnly = VirtualKey & 0xff;		// Just the vkey
	UINT32	vkOther = VirtualKey & ~0xff;	// Just the other stuff

	//	The key up we send is usually this.
	vkUp = vkOnly;

	//	Track numlock toggle.  We aren't doing anything with this right now.
	if ( vkOnly == VK_NUMLOCK )
		{
		v_bNumLockDown = FALSE;
		}
	else if ( vkOnly == VK_LSHIFT )
		{
		v_bLShiftDown = FALSE;
		//	If we faked an LShift up don't generate another.
		if ( !v_bShadowLShiftDown )
			{
			bGenUp = FALSE;
			}
		v_bShadowLShiftDown = FALSE;
		}
	else if ( vkOnly == VK_RSHIFT )
		{
		v_bRShiftDown = FALSE;
		//	If we faked an LShift up don't generate another.
		if ( !v_bShadowRShiftDown )
			{
			bGenUp = FALSE;
			}
		v_bShadowRShiftDown = FALSE;
		}
	else if ( InNumPadRange(vkOnly) )
		{
		vkUp = v_NumPadVKeySent[vkOnly - VK_NUMPAD_FIRST];		// Send out whatever we sent last time.
		v_NumPadVKeySent[vkOnly - VK_NUMPAD_FIRST] = 0;

		//	If this is the last one up, see if we need to restore the
		//	proper shift state.
		if ( !AnyNumPadKeyDown() )
			{
			//	If any of the shifts are still down and we generated a fake up,
			//	generate a key down to get back into a consistent state.
			if ( v_bLShiftDown && !v_bShadowLShiftDown )
				{
				vkDown1 = VK_LSHIFT;
				scDown1 = v_scLShift;
				v_bShadowLShiftDown = TRUE;
				}

			if ( v_bRShiftDown && !v_bShadowRShiftDown )
				{
				vkDown2 = VK_RSHIFT;
				scDown2 = v_scRShift;
				v_bShadowRShiftDown = TRUE;
				}
			}
		}

	if ( bGenUp )
		{
		//	Send the virtual key up.
		RemapVKeyBuf[i] = vkUp | vkOther;
		RemapScanCodeBuf[i] = ScanCode;
		RemapKeyStateFlagsBuf[i] = 0;
		i++;
		}

	if ( vkDown1 )
		{
		RemapVKeyBuf[i] = vkDown1;
		RemapScanCodeBuf[i] = scDown1;
		RemapKeyStateFlagsBuf[i] = KeyStateDownFlag;
		i++;
		}

	if ( vkDown2 )
		{
		RemapVKeyBuf[i] = vkDown2;
		RemapScanCodeBuf[i] = scDown2;
		RemapKeyStateFlagsBuf[i] = KeyStateDownFlag;
		i++;
		}

//	NKDbgPrintfW(TEXT("VkeyUp: vkeybuf[i]=%x scancode=%x Flags=%x\r\n"),RemapVKeyBuf[i-1],RemapScanCodeBuf[i-1],RemapKeyStateFlagsBuf[i-1]);
	return i;
}



