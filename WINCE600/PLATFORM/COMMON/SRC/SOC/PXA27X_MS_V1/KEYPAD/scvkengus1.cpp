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

@doc    EXTERNAL

@module scvkengus1.cpp |

Sample implementation of the keyboard platform dependent scan code to
Virtual Key mapping for keyboard driver.

Exports ScanCodeToVKey for the PDD to use to map scan codes to virtual
keys.  A version of this will be needed for every physical/virtual key
configuration.

*/

#include <windows.h>
#include <keybddr.h>

#ifdef OSV_SP
#include <winuserm.h>
#endif

#define PROFILE 1
#ifdef PROFILE
extern "C" void ProfileStart(DWORD,DWORD);
extern "C" void ProfileStop(void);
#endif

#define	ScanCodeTableLast	0xBf

#ifdef BSP_NOFASTTAPKP
#ifdef OSV_SP
const UINT8  ScanCodeToVKeyTable[ScanCodeTableLast+1] =
{
	VK_T1,			// Scan Code 0x0		//	Bulverde KeyPad	'1'
	VK_T2,			// Scan Code 0x1		//	Bulverde KeyPad	'2'
	VK_T3,			// Scan Code 0x2		//	Bulverde KeyPad	'3'	
	'2',			// Scan Code 0x3
	VK_UP,			// Scan Code 0x4		
	'4',			// Scan Code 0x5
	VK_DOWN,		// Scan Code 0x6		
	'6',			// Scan Code 0x7
	VK_LEFT,		// Scan Code 0x8		
	'8',			// Scan Code 0x9
	VK_TEND,		// Scan Code 0xA		//	Bulverde Keypad Rotary key scroll up		
	VK_TBACK,		// Scan Code 0xB		//	Bulverde keypad Rotary key scroll down
	VK_TTALK,		// Scan Code 0xC		//	Bulverde keypad Rotary key Action.
	VK_EQUAL,		// Scan Code 0xD
	'3',			// Scan Code 0xE		
	VK_TAB,			// Scan Code 0xF


	VK_T4,			// Scan Code 0x10		//	Bulverde KeyPad	'4'		
	VK_T5,			// Scan Code 0x11		//	Bulverde KeyPad	'5'
	VK_T6,			// Scan Code 0x12		//	Bulverde KeyPad	'6'		
	'R',			// Scan Code 0x13
	'T',			// Scan Code 0x14
	'Y',			// Scan Code 0x15
	'U',			// Scan Code 0x16
	'I',			// Scan Code 0x17
	'O',			// Scan Code 0x18
	'P',			// Scan Code 0x19
	VK_LBRACKET,	// Scan Code 0x1A
	VK_RBRACKET,	// Scan Code 0x1B
	VK_RETURN,		// Scan Code 0x1C
	VK_LCONTROL,	// Scan Code 0x1D
	'A',			// Scan Code 0x1E
	'S',			// Scan Code 0x1F


	VK_T7,			// Scan Code 0x20		//	Bulverde KeyPad	'7'		
	VK_T8,			// Scan Code 0x21		//	Bulverde KeyPad	'8'
	VK_T9,			// Scan Code 0x22		//	Bulverde KeyPad	'9'
	'H',			// Scan Code 0x23
	'6',			// Scan Code 0x24		
	'K',			// Scan Code 0x25
	'5',			// Scan Code 0x26		
	VK_SEMICOLON,	// Scan Code 0x27
	'4',			// Scan Code 0x28		
	VK_BACKQUOTE,	// Scan Code 0x29
	VK_F2,			// Scan Code 0x2A		
	VK_BACKSLASH,	// Scan Code 0x2B
	'C',			// Scan Code 0x2C		
	'X',			// Scan Code 0x2D
	'9',			// Scan Code 0x2E		
	'V',			// Scan Code 0x2F


	VK_TSTAR,		// Scan Code 0x30		// Bulverde KeyPad '*'		
	VK_T0,			// Scan Code 0x31		// Bulverde KeyPad '0'
	VK_TPOUND,		// Scan Code 0x32		// Bulverde KeyPad '#'
	VK_COMMA,		// Scan Code 0x33
	VK_PERIOD,		// Scan Code 0x34
	VK_SLASH,		// Scan Code 0x35
	VK_RSHIFT,		// Scan Code 0x36
	VK_MULTIPLY,	// Scan Code 0x37
	VK_LMENU,		// Scan Code 0x38
	VK_SPACE,		// Scan Code 0x39
	VK_CAPITAL,		// Scan Code 0x3A
	VK_F1,			// Scan Code 0x3B
	VK_F2,			// Scan Code 0x3C
	VK_F3,			// Scan Code 0x3D
	VK_F4,			// Scan Code 0x3E
	VK_F5,			// Scan Code 0x3F


	VK_TSOFT1,		// Scan Code 0x40		// Bvd Keypad '1' remapped to F1			
	VK_TUP,			// Scan Code 0x41		// Bvd Keypad '2' remapped to up arrow			
	VK_TSOFT2,		// Scan Code 0x42		// Bvd Keypad '3' remapped to F2			
	VK_F9,			// Scan Code 0x43										
	VK_OFF,			// Scan Code 0x44		
	VK_NUMLOCK,		// Scan Code 0x45
	'0',			// Scan Code 0x46		
	VK_NUMPAD7,		// Scan Code 0x47
	VK_MULTIPLY,	// Scan Code 0x48		
	VK_NUMPAD9,		// Scan Code 0x49
 	VK_TVOLUMEUP,	// Scan Code 0x4A		// Bvd Keypad SCROLL UP remapped to VOLUME UP			
	VK_TVOLUMEDOWN,	// Scan Code 0x4B		// Bvd Keypad SCROLL DOWN remapped to VOLUME DOWN			
	VK_TACTION,		// Scan Code 0x4C		// Bvd Keypad ACTION remapped to ACTION for now , change to POWER later			
	VK_NUMPAD6,		// Scan Code 0x4D
	VK_ADD,			// Scan Code 0x4E
	VK_NUMPAD1,		// Scan Code 0x4F


	VK_TLEFT,		// Scan Code 0x50		// Bvd Keypad '4' remapped to left arrow			
	VK_TACTION,		// Scan Code 0x51		// Bvd Keypad '5' remapped to action			
	VK_TRIGHT,		// Scan Code 0x52		// Bvd Keypad '6' remapped to right arrow		
	VK_DECIMAL,		// Scan Code 0x53
	VK_SNAPSHOT,	// Scan Code 0x54
	0,				// Scan Code 0x55
	0,				// Scan Code 0x56
	VK_F11,			// Scan Code 0x57
	VK_F12,			// Scan Code 0x58
	0,				// Scan Code 0x59
	0,				// Scan Code 0x5A
	VK_LWIN,		// Scan Code 0x5B
	VK_RWIN,		// Scan Code 0x5C
	VK_APPS,		// Scan Code 0x5D
	0,				// Scan Code 0x5E
	0,				// Scan Code 0x5F

	VK_THOME,		// Scan Code 0x60		// Bvd Keypad '7' remapped to HOME		
	VK_TDOWN,		// Scan Code 0x61		// Bvd Keypad '8' remapped to DOWN		
	VK_TBACK,		// Scan Code 0x62		// Bvd Keypad '9' remapped to BACK		
	VK_HELP,		// Scan Code 0x63
	VK_F13,			// Scan Code 0x64
	VK_F14,			// Scan Code 0x65
	VK_F15,			// Scan Code 0x66
	VK_F16,			// Scan Code 0x67
	VK_F17,			// Scan Code 0x68
	VK_F18,			// Scan Code 0x69
	VK_F19,			// Scan Code 0x6A
	VK_F20,			// Scan Code 0x6B
	VK_F21,			// Scan Code 0x6C
	VK_F22,			// Scan Code 0x6D
	VK_F23,			// Scan Code 0x6E
	0,				// Scan Code 0x6F

	VK_TTALK,		// Scan Code 0x70		// Bvd Keypad '*' remapped to TALK		
	VK_T0,			// Scan Code 0x71		// Bvd Keypad '0' remapped to HOME		
	VK_TEND,		// Scan Code 0x72		// Bvd Keypad '#' remapped to END		
	0,				// Scan Code 0x73
	0,				// Scan Code 0x74
	0,				// Scan Code 0x75
	VK_F24,			// Scan Code 0x76
	0,				// Scan Code 0x77
	0,				// Scan Code 0x78
	0,				// Scan Code 0x79
	0,				// Scan Code 0x7A
	0,				// Scan Code 0x7B
	0,				// Scan Code 0x7C
	0,				// Scan Code 0x7D
	0,				// Scan Code 0x7E
	0,				// Scan Code 0x7F
};

#else // else #ifdef OSV_SP

const UINT8  ScanCodeToVKeyTable[ScanCodeTableLast+1] =
{
	'1',			// Scan Code 0x0		//	Bulverde KeyPad	'1'
	'2',			// Scan Code 0x1		//	Bulverde KeyPad	'2'
	'3',			// Scan Code 0x2		//	Bulverde KeyPad	'3'	
	'2',			// Scan Code 0x3
	VK_UP,			// Scan Code 0x4		
	'4',			// Scan Code 0x5
	VK_DOWN,		// Scan Code 0x6		
	'6',			// Scan Code 0x7
	VK_LEFT,		// Scan Code 0x8		
	'8',			// Scan Code 0x9
	VK_UP,			// Scan Code 0xA		//	Bulverde Keypad Rotary key scroll up		
	VK_DOWN,		// Scan Code 0xB		//	Bulverde keypad Rotary key scroll down
	VK_RETURN,		// Scan Code 0xC		//	Bulverde keypad Rotary key Action.
	VK_EQUAL,		// Scan Code 0xD
	'3',			// Scan Code 0xE		
	VK_TAB,			// Scan Code 0xF


	'4',			// Scan Code 0x10		//	Bulverde KeyPad	'4'		
	'5',			// Scan Code 0x11		//	Bulverde KeyPad	'5'
	'6',			// Scan Code 0x12		//	Bulverde KeyPad	'6'		
	'R',			// Scan Code 0x13
	'T',			// Scan Code 0x14
	'Y',			// Scan Code 0x15
	'U',			// Scan Code 0x16
	'I',			// Scan Code 0x17
	'O',			// Scan Code 0x18
	'P',			// Scan Code 0x19
	VK_LBRACKET,	// Scan Code 0x1A
	VK_RBRACKET,	// Scan Code 0x1B
	VK_RETURN,		// Scan Code 0x1C
	VK_LCONTROL,	// Scan Code 0x1D
	'A',			// Scan Code 0x1E
	'S',			// Scan Code 0x1F


	'7',			// Scan Code 0x20		//	Bulverde KeyPad	'7'		
	'8',			// Scan Code 0x21		//	Bulverde KeyPad	'8'
	'9',			// Scan Code 0x22		//	Bulverde KeyPad	'9'
	'H',			// Scan Code 0x23
	'6',			// Scan Code 0x24		
	'K',			// Scan Code 0x25
	'5',			// Scan Code 0x26		
	VK_SEMICOLON,	// Scan Code 0x27
	'4',			// Scan Code 0x28		
	VK_BACKQUOTE,	// Scan Code 0x29
	VK_F2,			// Scan Code 0x2A		
	VK_BACKSLASH,	// Scan Code 0x2B
	'C',			// Scan Code 0x2C		
	'X',			// Scan Code 0x2D
	'9',			// Scan Code 0x2E		
	'V',			// Scan Code 0x2F


	VK_MULTIPLY,	// Scan Code 0x30		// Bulverde KeyPad '*'		
	'0',			// Scan Code 0x31		// Bulverde KeyPad '0'
	'A',			// Scan Code 0x32		// Bulverde KeyPad '#'
	VK_COMMA,		// Scan Code 0x33
	VK_PERIOD,		// Scan Code 0x34
	VK_SLASH,		// Scan Code 0x35
	VK_RSHIFT,		// Scan Code 0x36
	VK_MULTIPLY,	// Scan Code 0x37
	VK_LMENU,		// Scan Code 0x38
	VK_SPACE,		// Scan Code 0x39
	VK_CAPITAL,		// Scan Code 0x3A
	VK_F1,			// Scan Code 0x3B
	VK_F2,			// Scan Code 0x3C
	VK_F3,			// Scan Code 0x3D
	VK_F4,			// Scan Code 0x3E
	VK_F5,			// Scan Code 0x3F


	VK_LWIN,		// Scan Code 0x40		// For Merlin remapped to Windows key
	VK_UP,			// Scan Code 0x41		// For Merlin remapped to UP key
	VK_TAB,			// Scan Code 0x42		// For Merlin remapped to Tab key
	VK_F9,			// Scan Code 0x43										
	VK_OFF,			// Scan Code 0x44		
	VK_NUMLOCK,		// Scan Code 0x45
	'0',			// Scan Code 0x46		
	VK_NUMPAD7,		// Scan Code 0x47
	VK_MULTIPLY,	// Scan Code 0x48		
	VK_NUMPAD9,		// Scan Code 0x49
 	VK_UP,			// Scan Code 0x4A		// For Merlin remapped to UP key
	VK_DOWN,		// Scan Code 0x4B		// For Merlin remapped to DOWN key
	VK_ESCAPE,		// Scan Code 0x4C		// For Merlin remapped to Return key
	VK_NUMPAD6,		// Scan Code 0x4D
	VK_ADD,			// Scan Code 0x4E
	VK_NUMPAD1,		// Scan Code 0x4F


	VK_LEFT,		// Scan Code 0x50		// For Merlin remapped to Backspace key
	VK_RETURN,		// Scan Code 0x51		// For Merlin remapped to Enter key
	VK_RIGHT,		// Scan Code 0x52		// For Merlin remapped to Right Arrow key
	VK_DECIMAL,		// Scan Code 0x53
	VK_SNAPSHOT,	// Scan Code 0x54
	0,				// Scan Code 0x55
	0,				// Scan Code 0x56
	VK_F11,			// Scan Code 0x57
	VK_F12,			// Scan Code 0x58
	0,				// Scan Code 0x59
	0,				// Scan Code 0x5A
	VK_LWIN,		// Scan Code 0x5B
	VK_RWIN,		// Scan Code 0x5C
	VK_APPS,		// Scan Code 0x5D
	0,				// Scan Code 0x5E
	0,				// Scan Code 0x5F

	VK_BACK,		// Scan Code 0x60		// For Merlin remapped to Back space key
	VK_DOWN,		// Scan Code 0x61		// For Merlin remapped to Down Arrow key
	VK_SPACE,		// Scan Code 0x62		// For Merlin remapped to Space key
	VK_HELP,		// Scan Code 0x63
	VK_F13,			// Scan Code 0x64
	VK_F14,			// Scan Code 0x65
	VK_F15,			// Scan Code 0x66
	VK_F16,			// Scan Code 0x67
	VK_F17,			// Scan Code 0x68
	VK_F18,			// Scan Code 0x69
	VK_F19,			// Scan Code 0x6A
	VK_F20,			// Scan Code 0x6B
	VK_F21,			// Scan Code 0x6C
	VK_F22,			// Scan Code 0x6D
	VK_F23,			// Scan Code 0x6E
	0,				// Scan Code 0x6F

	VK_HOME,			// Scan Code 0x70		// For Merlin remapped to ALT key
	VK_LCONTROL,		// Scan Code 0x71		// For Merlin remapped to Control key
	VK_END,			// Scan Code 0x72		// For Merlin remapped to F4 key
	0,				// Scan Code 0x73
	0,				// Scan Code 0x74
	0,				// Scan Code 0x75
	VK_F24,			// Scan Code 0x76
	0,				// Scan Code 0x77
	0,				// Scan Code 0x78
	0,				// Scan Code 0x79
	0,				// Scan Code 0x7A
	0,				// Scan Code 0x7B
	0,				// Scan Code 0x7C
	0,				// Scan Code 0x7D
	0,				// Scan Code 0x7E
	0,				// Scan Code 0x7F
};
#endif	// end #ifdef OSV_SP

#else  //  else #ifdef BSP_NOFASTTAPKP

// fast tap key pad scan codes
#ifndef OSV_SP
const UINT8  ScanCodeToVKeyTable[ScanCodeTableLast+1] =
{
	'A',			// Scan Code 0x0		//	Bulverde KeyPad	'1'
	'G',			// Scan Code 0x1		//	Bulverde KeyPad	'2'
	'M',			// Scan Code 0x2		//	Bulverde KeyPad	'3'	
	'S',			// Scan Code 0x3
	VK_PERIOD,		// Scan Code 0x4		
	VK_HOME,		// Scan Code 0x5
	VK_UP,		   	// Scan Code 0x6		
	0,				// Scan Code 0x7
	0,				// Scan Code 0x8		
	0,				// Scan Code 0x9
	VK_UP,			// Scan Code 0xA		//	Bulverde Keypad Rotary key scroll up		
	VK_DOWN,		// Scan Code 0xB		//	Bulverde keypad Rotary key scroll down
	VK_RETURN,		// Scan Code 0xC		//	Bulverde keypad Rotary key Action.
	0,				// Scan Code 0xD
	0,				// Scan Code 0xE		
    0,				// Scan Code 0xF


	'B',			// Scan Code 0x10		//	Bulverde KeyPad	'4'		
	'H',			// Scan Code 0x11		//	Bulverde KeyPad	'5'
	'N',			// Scan Code 0x12		//	Bulverde KeyPad	'6'		
	'T',			// Scan Code 0x13
	VK_NUMPAD2,	 	// Scan Code 0x14
	VK_CAPITAL,	   	// Scan Code 0x15
	VK_DOWN,   		// Scan Code 0x16
	0,				// Scan Code 0x17
	0,				// Scan Code 0x18
	0,				// Scan Code 0x19
	0,				// Scan Code 0x1A
	0,				// Scan Code 0x1B
	0,				// Scan Code 0x1C
	0,				// Scan Code 0x1D
	0,				// Scan Code 0x1E
	0,				// Scan Code 0x1F


	'C',			// Scan Code 0x20		//	Bulverde KeyPad	'7'		
	'I',			// Scan Code 0x21		//	Bulverde KeyPad	'8'
	'O',			// Scan Code 0x22		//	Bulverde KeyPad	'9'
	'U',			// Scan Code 0x23
	'Y',			// Scan Code 0x24		
	VK_SPACE,	 	// Scan Code 0x25
	VK_LEFT,		// Scan Code 0x26		
	0,				// Scan Code 0x27
	0,				// Scan Code 0x28		
	0,				// Scan Code 0x29
	0,				// Scan Code 0x2A		
	0,				// Scan Code 0x2B
	0,				// Scan Code 0x2C		
	0,				// Scan Code 0x2D
	0,				// Scan Code 0x2E		
	0,				// Scan Code 0x2F

	'D',			// Scan Code 0x30		// Bulverde KeyPad '*'		
	'J',			// Scan Code 0x31		// Bulverde KeyPad '0'
	'P',			// Scan Code 0x32		// Bulverde KeyPad '#'
	'V',			// Scan Code 0x33
	'Z',			// Scan Code 0x34
	VK_SPACE,		// Scan Code 0x35
	VK_RIGHT,		// Scan Code 0x36
	0,				// Scan Code 0x37
	0,				// Scan Code 0x38
	0,				// Scan Code 0x39
	0,				// Scan Code 0x3A
	0,				// Scan Code 0x3B
	0,				// Scan Code 0x3C
	0,				// Scan Code 0x3D
	0,				// Scan Code 0x3E
	0,				// Scan Code 0x3F


	'E',			// Scan Code 0x40		// Bvd Keypad '1' remapped to F1			
	'K',			// Scan Code 0x41		// Bvd Keypad '2' remapped to up arrow			
	'Q',			// Scan Code 0x42		// Bvd Keypad '3' remapped to F2			
	'W',			// Scan Code 0x43										
	VK_SLASH,		// Scan Code 0x44		
	VK_CONTROL,		// Scan Code 0x45
	VK_EXECUTE,	 	// Scan Code 0x46		
	0,				// Scan Code 0x47
	0,				// Scan Code 0x48		
	0,				// Scan Code 0x49
 	0,				// Scan Code 0x4A		// Bvd Keypad SCROLL UP remapped to VOLUME UP			
	0,				// Scan Code 0x4B		// Bvd Keypad SCROLL DOWN remapped to VOLUME DOWN			
	0,				// Scan Code 0x4C		// Bvd Keypad ACTION remapped to ACTION for now , change to POWER later			
	0,				// Scan Code 0x4D
	0,				// Scan Code 0x4E
	0,				// Scan Code 0x4F


	'F',			// Scan Code 0x50		// Bvd Keypad '4' remapped to left arrow			
	'L',			// Scan Code 0x51		// Bvd Keypad '5' remapped to action			
	'R',			// Scan Code 0x52		// Bvd Keypad '6' remapped to right arrow		
	'X',			// Scan Code 0x53
	VK_BACKSLASH,	// Scan Code 0x54
	VK_BACK,   		// Scan Code 0x55
	0,				// Scan Code 0x56
	0,				// Scan Code 0x57
	0,				// Scan Code 0x58
	0,				// Scan Code 0x59
	0,				// Scan Code 0x5A
	0,				// Scan Code 0x5B
	0,				// Scan Code 0x5C
	0,				// Scan Code 0x5D
	0,				// Scan Code 0x5E
	0,				// Scan Code 0x5F

	VK_EXECUTE,		// Scan Code 0x60		// Bvd Keypad '7' remapped to HOME		
	VK_END,			// Scan Code 0x61		// Bvd Keypad '8' remapped to DOWN		
	VK_LWIN,		// Scan Code 0x62		// Bvd Keypad '9' remapped to BACK		
	VK_RWIN,		// Scan Code 0x63
	VK_EXECUTE,		// Scan Code 0x64
	0,				// Scan Code 0x65
	0,				// Scan Code 0x66
	0,				// Scan Code 0x67
	0,				// Scan Code 0x68
	0,				// Scan Code 0x69
	0,				// Scan Code 0x6A
	0,				// Scan Code 0x6B
	0,				// Scan Code 0x6C
	0,				// Scan Code 0x6D
	0,				// Scan Code 0x6E
	0,				// Scan Code 0x6F

	'1',			// Scan Code 0x70		// Bvd FTKeypad '1'	
	VK_NUMPAD1,
	'2',			// Scan Code 0x72		// Bvd FTKeypad '2'		
	VK_NUMPAD3,			// Scan Code 0x73
	'3',			// Scan Code 0x74		// Bvd FTKeypad '3'
	0,				// Scan Code 0x75
	0,				// Scan Code 0x76
	0,				// Scan Code 0x77
	0,				// Scan Code 0x78
	0,				// Scan Code 0x79
	0,				// Scan Code 0x7A
	0,				// Scan Code 0x7B
	0,				// Scan Code 0x7C
	0,				// Scan Code 0x7D
	0,				// Scan Code 0x7E
	0,				// Scan Code 0x7F

	'4',			// Scan Code 0x80		// Bvd Keypad '7' remapped to HOME		
	VK_NUMPAD4,			// Scan Code 0x81		// Bvd Keypad '8' remapped to DOWN		
	'5',			// Scan Code 0x82		// Bvd Keypad '9' remapped to BACK		
	VK_NUMPAD7,			// Scan Code 0x83
	'6',			// Scan Code 0x84
	0,				// Scan Code 0x85
	0,				// Scan Code 0x86
	0,				// Scan Code 0x87
	0,				// Scan Code 0x88
	0,				// Scan Code 0x89
	0,				// Scan Code 0x8A
	0,				// Scan Code 0x8B
	0,				// Scan Code 0x8C
	0,				// Scan Code 0x8D
	0,				// Scan Code 0x8E
	0,				// Scan Code 0x8F


	'7',		   	// Scan Code 0x90		// Bvd Keypad '*' remapped to TALK		
	VK_ADD,			// Scan Code 0x91		// Bvd Keypad '0' remapped to HOME		
	'8',	   		// Scan Code 0x92		// Bvd Keypad '#' remapped to END		
	VK_SEMICOLON,	// Scan Code 0x93
	'9',	   		// Scan Code 0x94
	0,				// Scan Code 0x95
	0,				// Scan Code 0x96
	0,				// Scan Code 0x97
	0,				// Scan Code 0x98
	0,				// Scan Code 0x99
	0,				// Scan Code 0x9A
	0,				// Scan Code 0x9B
	0,				// Scan Code 0x9C
	0,				// Scan Code 0x9D
	0,				// Scan Code 0x9E
	0,				// Scan Code 0x9F
			   
	VK_MULTIPLY,   	// Scan Code 0xA0		// Bvd Keypad '7' remapped to HOME		
	VK_APOSTROPHE,	// Scan Code 0xA1		// Bvd Keypad '8' remapped to DOWN		
	VK_NUMPAD0,		// Scan Code 0xA2		// Bvd Keypad '9' remapped to BACK		
	VK_COMMA,		// Scan Code 0xA3
	VK_NUMPAD6,		// Scan Code 0xA4
	0,				// Scan Code 0xA5
	0,				// Scan Code 0xA6
	0,				// Scan Code 0xA7
	0,				// Scan Code 0xA8
	0,				// Scan Code 0xA9
	0,				// Scan Code 0xAA
	0,				// Scan Code 0xAB
	0,				// Scan Code 0xAC
	0,				// Scan Code 0xAD
	0,				// Scan Code 0xAE
	0,				// Scan Code 0xAF

	VK_TAB,   		// Scan Code 0xB0		// Bvd Keypad '*' remapped to TALK		
	VK_LBRACKET,	// Scan Code 0xB1		// Bvd Keypad '0' remapped to HOME		
	VK_BACK,		// Scan Code 0xB2		// Bvd Keypad '#' remapped to END		
	VK_RBRACKET, 	// Scan Code 0xB3
	VK_RETURN,		// Scan Code 0xB4
	0,				// Scan Code 0xB5
	0,				// Scan Code 0xB6
	0,				// Scan Code 0xB7
	0,				// Scan Code 0xB8
	0,				// Scan Code 0xB9
	0,				// Scan Code 0xBA
	0,				// Scan Code 0xBB
	0,				// Scan Code 0xBC
	0,				// Scan Code 0xBD
	0,				// Scan Code 0xBE
	0,				// Scan Code 0xBF
};

#else  // else #ifdef OSV_SP  

const UINT8  ScanCodeToVKeyTable[ScanCodeTableLast+1] =
{
	'A',			// Scan Code 0x0		//	Bulverde KeyPad	'1'
	'G',			// Scan Code 0x1		//	Bulverde KeyPad	'2'
	'M',			// Scan Code 0x2		//	Bulverde KeyPad	'3'	
	'S',			// Scan Code 0x3
	VK_PERIOD,		// Scan Code 0x4		
	VK_THOME,		// Scan Code 0x5
	VK_TUP,		   	// Scan Code 0x6		
	0,				// Scan Code 0x7
	0,				// Scan Code 0x8		
	0,				// Scan Code 0x9
	VK_TVOLUMEUP,	// Scan Code 0xA		//	Bulverde Keypad Rotary key scroll up		
	VK_TVOLUMEDOWN,	// Scan Code 0xB		//	Bulverde keypad Rotary key scroll down
	VK_TACTION,		// Scan Code 0xC		//	Bulverde keypad Rotary key Action.
	0,				// Scan Code 0xD
	0,				// Scan Code 0xE		
    0,				// Scan Code 0xF

	'B',			// Scan Code 0x10		//	Bulverde KeyPad	'4'		
	'H',			// Scan Code 0x11		//	Bulverde KeyPad	'5'
	'N',			// Scan Code 0x12		//	Bulverde KeyPad	'6'		
	'T',			// Scan Code 0x13
	VK_NUMPAD2,		// Scan Code 0x14
	VK_CAPITAL,	   	// Scan Code 0x15
	VK_TDOWN,   	// Scan Code 0x16
	0,				// Scan Code 0x17
	0,				// Scan Code 0x18
	0,				// Scan Code 0x19
	0,				// Scan Code 0x1A
	0,				// Scan Code 0x1B
	0,				// Scan Code 0x1C
	0,				// Scan Code 0x1D
	0,				// Scan Code 0x1E
	0,				// Scan Code 0x1F

	'C',			// Scan Code 0x20		//	Bulverde KeyPad	'7'		
	'I',			// Scan Code 0x21		//	Bulverde KeyPad	'8'
	'O',			// Scan Code 0x22		//	Bulverde KeyPad	'9'
	'U',			// Scan Code 0x23
	'Y',			// Scan Code 0x24		
	VK_SPACE,	 	// Scan Code 0x25
	VK_TLEFT,		// Scan Code 0x26		
	0,				// Scan Code 0x27
	0,				// Scan Code 0x28		
	0,				// Scan Code 0x29
	0,				// Scan Code 0x2A		
	0,				// Scan Code 0x2B
	0,				// Scan Code 0x2C		
	0,				// Scan Code 0x2D
	0,				// Scan Code 0x2E		
	0,				// Scan Code 0x2F

	'D',			// Scan Code 0x30		// Bulverde KeyPad '*'		
	'J',			// Scan Code 0x31		// Bulverde KeyPad '0'
	'P',			// Scan Code 0x32		// Bulverde KeyPad '#'
	'V',			// Scan Code 0x33
	'Z',			// Scan Code 0x34
	VK_SPACE,		// Scan Code 0x35
	VK_TRIGHT,		// Scan Code 0x36
	0,				// Scan Code 0x37
	0,				// Scan Code 0x38
	0,				// Scan Code 0x39
	0,				// Scan Code 0x3A
	0,				// Scan Code 0x3B
	0,				// Scan Code 0x3C
	0,				// Scan Code 0x3D
	0,				// Scan Code 0x3E
	0,				// Scan Code 0x3F


	'E',			// Scan Code 0x40		// Bvd Keypad '1' remapped to F1			
	'K',			// Scan Code 0x41		// Bvd Keypad '2' remapped to up arrow			
	'Q',			// Scan Code 0x42		// Bvd Keypad '3' remapped to F2			
	'W',			// Scan Code 0x43										
	VK_SLASH,		// Scan Code 0x44		
	VK_CONTROL,		// Scan Code 0x45
	VK_TACTION,	 	// Scan Code 0x46		
	0,				// Scan Code 0x47
	0,				// Scan Code 0x48		
	0,				// Scan Code 0x49
 	0,				// Scan Code 0x4A		// Bvd Keypad SCROLL UP remapped to VOLUME UP			
	0,				// Scan Code 0x4B		// Bvd Keypad SCROLL DOWN remapped to VOLUME DOWN			
	0,				// Scan Code 0x4C		// Bvd Keypad ACTION remapped to ACTION for now , change to POWER later			
	0,				// Scan Code 0x4D
	0,				// Scan Code 0x4E
	0,				// Scan Code 0x4F


	'F',			// Scan Code 0x50		// Bvd Keypad '4' remapped to left arrow			
	'L',			// Scan Code 0x51		// Bvd Keypad '5' remapped to action			
	'R',			// Scan Code 0x52		// Bvd Keypad '6' remapped to right arrow		
	'X',			// Scan Code 0x53
	VK_BACKSLASH,	// Scan Code 0x54
	VK_TBACK, 		// Scan Code 0x55
	0,				// Scan Code 0x56
	0,				// Scan Code 0x57
	0,				// Scan Code 0x58
	0,				// Scan Code 0x59
	0,				// Scan Code 0x5A
	0,				// Scan Code 0x5B
	0,				// Scan Code 0x5C
	0,				// Scan Code 0x5D
	0,				// Scan Code 0x5E
	0,				// Scan Code 0x5F

	VK_TTALK,		// Scan Code 0x60		// Bvd Keypad '7' remapped to HOME		
	VK_TEND,	 	// Scan Code 0x61		// Bvd Keypad '8' remapped to DOWN		
	VK_TSOFT1,		// Scan Code 0x62		// Bvd Keypad '9' remapped to BACK		
	VK_TSOFT2,		// Scan Code 0x63
	VK_TACTION,		// Scan Code 0x64
	0,				// Scan Code 0x65
	0,				// Scan Code 0x66
	0,				// Scan Code 0x67
	0,				// Scan Code 0x68
	0,				// Scan Code 0x69
	0,				// Scan Code 0x6A
	0,				// Scan Code 0x6B
	0,				// Scan Code 0x6C
	0,				// Scan Code 0x6D
	0,				// Scan Code 0x6E
	0,				// Scan Code 0x6F

	VK_T1,			// Scan Code 0x70		// Bvd FTKeypad '1'	
	VK_NUMPAD1,		// Scan Code 0x71
	VK_T2,			// Scan Code 0x72		// Bvd FTKeypad '2'		
	VK_NUMPAD3,	   	// Scan Code 0x73
	VK_T3,			// Scan Code 0x74		// Bvd FTKeypad '3'
	0,				// Scan Code 0x75
	0,				// Scan Code 0x76
	0,				// Scan Code 0x77
	0,				// Scan Code 0x78
	0,				// Scan Code 0x79
	0,				// Scan Code 0x7A
	0,				// Scan Code 0x7B
	0,				// Scan Code 0x7C
	0,				// Scan Code 0x7D
	0,				// Scan Code 0x7E
	0,				// Scan Code 0x7F

	VK_T4,			// Scan Code 0x80		// Bvd Keypad '7' remapped to HOME		
	VK_NUMPAD4,		// Scan Code 0x81		// Bvd Keypad '8' remapped to DOWN		
	VK_T5,			// Scan Code 0x82		// Bvd Keypad '9' remapped to BACK		
	VK_NUMPAD7,		// Scan Code 0x83
	VK_T6,			// Scan Code 0x84
	0,				// Scan Code 0x85
	0,				// Scan Code 0x86
	0,				// Scan Code 0x87
	0,				// Scan Code 0x88
	0,				// Scan Code 0x89
	0,				// Scan Code 0x8A
	0,				// Scan Code 0x8B
	0,				// Scan Code 0x8C
	0,				// Scan Code 0x8D
	0,				// Scan Code 0x8E
	0,				// Scan Code 0x8F


	VK_T7,			// Scan Code 0x90		// Bvd Keypad '*' remapped to TALK		
	VK_ADD,			// Scan Code 0x91		// Bvd Keypad '0' remapped to HOME		
	VK_T8,			// Scan Code 0x92		// Bvd Keypad '#' remapped to END		
	VK_SEMICOLON,	// Scan Code 0x93
	VK_T9,			// Scan Code 0x94
	0,				// Scan Code 0x95
	0,				// Scan Code 0x96
	0,				// Scan Code 0x97
	0,				// Scan Code 0x98
	0,				// Scan Code 0x99
	0,				// Scan Code 0x9A
	0,				// Scan Code 0x9B
	0,				// Scan Code 0x9C
	0,				// Scan Code 0x9D
	0,				// Scan Code 0x9E
	0,				// Scan Code 0x9F
			   
	VK_TSTAR,   	// Scan Code 0xA0		// Bvd Keypad '7' remapped to HOME		
	VK_APOSTROPHE,	// Scan Code 0xA1		// Bvd Keypad '8' remapped to DOWN		
	VK_T0,			// Scan Code 0xA2		// Bvd Keypad '9' remapped to BACK		
	VK_COMMA,		// Scan Code 0xA3
	VK_TPOUND,		// Scan Code 0xA4
	0,				// Scan Code 0xA5
	0,				// Scan Code 0xA6
	0,				// Scan Code 0xA7
	0,				// Scan Code 0xA8
	0,				// Scan Code 0xA9
	0,				// Scan Code 0xAA
	0,				// Scan Code 0xAB
	0,				// Scan Code 0xAC
	0,				// Scan Code 0xAD
	0,				// Scan Code 0xAE
	0,				// Scan Code 0xAF

	VK_NEXT,		// Scan Code 0xB0		// Bvd Keypad '*' remapped to TALK		
	VK_LBRACKET,	// Scan Code 0xB1		// Bvd Keypad '0' remapped to HOME		
	VK_BACK,		// Scan Code 0xB2		// Bvd Keypad '#' remapped to END		
	VK_RBRACKET,	// Scan Code 0xB3
	VK_TACTION,		// Scan Code 0xB4
	0,				// Scan Code 0xB5
	0,				// Scan Code 0xB6
	0,				// Scan Code 0xB7
	0,				// Scan Code 0xB8
	0,				// Scan Code 0xB9
	0,				// Scan Code 0xBA
	0,				// Scan Code 0xBB
	0,				// Scan Code 0xBC
	0,				// Scan Code 0xBD
	0,				// Scan Code 0xBE
	0,				// Scan Code 0xBF

};




#endif	// end #ifdef OSV_SP

#endif	//  else #ifdef BSP_NOFASTTAPKP

#define E0ScanCodeTableFirst	0xe035
#define E0ScanCodeTableLast		0xe05d

const UINT8 ScanCodeE0ToVKeyTable[] =
{
	VK_DIVIDE,		// Scan Code 0xE035
	0,				// Scan Code 0xE036
	VK_SNAPSHOT,	// Scan Code 0xE037	
	VK_RMENU,		// Scan Code 0xE038
	0,				// Scan Code 0xE039	
	0,				// Scan Code 0xE03A	
	0,				// Scan Code 0xE03B	
	0,				// Scan Code 0xE03C	
	0,				// Scan Code 0xE03D	
	0,				// Scan Code 0xE03E
	0,				// Scan Code 0xE03F	
	0,				// Scan Code 0xE040
	0,				// Scan Code 0xE041
	0,				// Scan Code 0xE042
	0,				// Scan Code 0xE043
	0,				// Scan Code 0xE044
	0,				// Scan Code 0xE045
	0,				// Scan Code 0xE046
	VK_HOME,		// Scan Code 0xE047
	VK_UP,			// Scan Code 0xE048
	VK_PRIOR,		// Scan Code 0xE049
	0,				// Scan Code 0xE04A
	VK_LEFT,		// Scan Code 0xE04B
	0,				// Scan Code 0xE04C
	VK_RIGHT,		// Scan Code 0xE04D
	0,				// Scan Code 0xE04E
	VK_END,			// Scan Code 0xE04F
	VK_DOWN,		// Scan Code 0xE050
	VK_NEXT,		// Scan Code 0xE051
	VK_INSERT,		// Scan Code 0xE052
	VK_DELETE,		// Scan Code 0xE053
	0,				// Scan Code 0xE054
	0,				// Scan Code 0xE055
	0,				// Scan Code 0xE056
	0,				// Scan Code 0xE057
	0,				// Scan Code 0xE058
	0,				// Scan Code 0xE059
	0,				// Scan Code 0xE05A
	VK_LWIN,		// Scan Code 0xE05B
	VK_RWIN,		// Scan Code 0xE05C
	VK_APPS,		// Scan Code 0xE05D
};


/*++

ScanCodeToVKeyEx:

Map a scan code to virtual key(s).

--*/
UINT32
ScanCodeToVKeyEx(
	UINT32			ScanCode,
	KEY_STATE_FLAGS	KeyStateFlags,
	UINT32			VKeyBuf[16],
	UINT32			ScanCodeBuf[16],
	KEY_STATE_FLAGS	KeyStateFlagsBuf[16]
	)
{
	int	cVKeys = 0;

#ifdef PROFILE
	if (ScanCode == /* 7A VK_F8 */ 0x42 && KeyStateFlags == KeyStateDownFlag)
				ProfileStart(200,8);
	if (ScanCode == /* 7A VK_F9 */ 0x43 && KeyStateFlags == KeyStateDownFlag)
				ProfileStart(200,4);
	if (ScanCode == /* 7A VK_F10 */ 0x44 && KeyStateFlags == KeyStateDownFlag)
				ProfileStart(200,2);
	if (ScanCode == /* 7A VK_F11 */ 0x57 && KeyStateFlags == KeyStateDownFlag)
				ProfileStart(200,0);
	if (ScanCode == /* 7B VK_F12 */ 0x58 && KeyStateFlags == KeyStateDownFlag)
				ProfileStop();
#endif
 //	RETAILMSG(1,(TEXT("ScanCodeToVKeyEx: ui8ScanCode: %08x KeyStateFlags %08x\r\n"), ScanCode, KeyStateFlags));

	if ( ScanCode == 0x45 )
		{
		//	The desktop shows that this is an extended
		//	scan code.  To make it look the same, we
		//	pass back the scan code with a e0 tacked
		//	on to it.
		VKeyBuf[cVKeys] = VK_NUMLOCK;
		ScanCodeBuf[cVKeys] = 0xe045;
		KeyStateFlagsBuf[cVKeys] = KeyStateFlags;
		cVKeys++;
		}
	else if ( ScanCode == 0xe01c )
		{
		VKeyBuf[cVKeys] = VK_RETURN;
		ScanCodeBuf[cVKeys] = ScanCode;
		KeyStateFlagsBuf[cVKeys] = KeyStateFlags;
		cVKeys++;
		}
	else if ( ScanCode == 0xe01d )
		{
		VKeyBuf[cVKeys] = VK_RCONTROL;
		ScanCodeBuf[cVKeys] = ScanCode;
		KeyStateFlagsBuf[cVKeys] = KeyStateFlags;
		cVKeys++;
		}
#if USE_ALT_GR
	//	Turn this on if you need AltGr on a keyboard.
	else if ( ScanCode == 0xe038 )
		{
		VKeyBuf[cVKeys] = VK_RMENU;
		ScanCodeBuf[cVKeys] = ScanCode;
		KeyStateFlagsBuf[cVKeys] = KeyStateFlags;
		cVKeys++;
		VKeyBuf[cVKeys] = VK_RCONTROL;
		ScanCodeBuf[cVKeys] = ScanCode;
		KeyStateFlagsBuf[cVKeys] = KeyStateFlags;
		cVKeys++;
		}
#endif
	else if ( ScanCode == 0xe11d45 )
		{
		VKeyBuf[cVKeys] = VK_PAUSE;
		ScanCodeBuf[cVKeys] = 0x45;
		KeyStateFlagsBuf[cVKeys] = KeyStateFlags;
		cVKeys++;
		}
	else if ( ( ScanCode >= E0ScanCodeTableFirst ) &&
			  ( ScanCode <= E0ScanCodeTableLast ) )
		{
	//	NKDbgPrintfW(TEXT("Between first and last\r\n"));
		VKeyBuf[cVKeys] = ScanCodeE0ToVKeyTable[ScanCode - E0ScanCodeTableFirst];
		if ( VKeyBuf[cVKeys] )
			{
			ScanCodeBuf[cVKeys] = ScanCode;
			KeyStateFlagsBuf[cVKeys] = KeyStateFlags;
			cVKeys++;
			}
		}
	else if ( ScanCode <= ScanCodeTableLast )
		{
//		NKDbgPrintfW(TEXT("< scan code table last\r\n"));
//	RETAILMSG(1,(TEXT("cVKeys %08x ScanCode %08x KeyStateFlags %08x\r\n"), cVKeys, ScanCode, KeyStateFlags));
		VKeyBuf[cVKeys] = ScanCodeToVKeyTable[ScanCode];
	   	if ( VKeyBuf[cVKeys] )
			{
			ScanCodeBuf[cVKeys] = ScanCode;
			KeyStateFlagsBuf[cVKeys] = KeyStateFlags;
			cVKeys++;
			}
		}
	//	For some reason, the print screen key has an extra 0xe02a scan code
	//	in it.  It doesn't do anything so we won't complain about it.
/*
	if ( ( cVKeys == 0 ) &&
		 ( ScanCode != 0xe02a ) )
		{
		RETAILMSG(1,(TEXT("Unknown scan code %x\r\n"), ScanCode));
		}
*/

//	RETAILMSG(1,(TEXT("ScanCodeToVKeyEx:  cVKeys: %08x\r\n"),  cVKeys));
	return cVKeys;

}



static const UINT16  VKeyToScanCodeTable[COUNT_VKEYS] =
	{
/* 00 */				0,
/* 01 VK_LBUTTON	*/	0,
/* 02 VK_RBUTTON	*/	0,
/* 03 VK_CANCEL		*/	0,
/* 04 VK_MBUTTON	*/	0,
/* 05 */				0,
/* 06 */				0,
/* 07 */				0,
/* 08 VK_BACK		*/	0x0e,
/* 09 VK_TAB		*/	0x0f,
/* 0A */				0,
/* 0B */				0,
/* 0C VK_CLEAR		*/	0x4c,
/* 0D VK_RETURN		*/	0x1c,
/* 0E */				0,
/* 0F */				0,


/* 10 VK_SHIFT		*/	0x2a,
/* 11 VK_CONTROL	*/	0x1d,
/* 12 VK_MENU		*/	0x38,
/* 13 VK_PAUSE		*/	0,
/* 14 VK_CAPITAL	*/	0x3a,
/* 15 */				0,
/* 16 */				0,
/* 17 */				0,
/* 18 */				0,
/* 19 */				0,
/* 1A */				0,
/* 1B VK_ESCAPE		*/	0x01,
/* 1C */				0,
/* 1D */				0,
/* 1E */				0,
/* 1F */				0,


/* 20 VK_SPACE		*/	0x39,
/* 21 VK_PRIOR		*/	0xe049,
/* 22 VK_NEXT		*/	0xe051,
/* 23 VK_END		*/	0xe04f,
/* 24 VK_HOME		*/	0xe047,
/* 25 VK_LEFT		*/	0xe04b,
/* 26 VK_UP			*/	0xe048,
/* 27 VK_RIGHT		*/	0xe04d,
/* 28 VK_DOWN		*/	0xe050,
/* 29 VK_SELECT		*/	0,
/* 2A VK_PRINT		*/	0,
/* 2B VK_EXECUTE	*/	0,
/* 2C VK_SNAPSHOT	*/	0x54,
/* 2D VK_INSERT		*/	0xe052,
/* 2E VK_DELETE		*/	0xe053,
/* 2F VK_HELP		*/	0,




/* 30 */				0x0b,
/* 31 */				0x02,
/* 32 */				0x03,
/* 33 */				0x04,
/* 34 */				0x05,
/* 35 */				0x06,
/* 36 */				0x07,
/* 37 */				0x08,
/* 38 */				0x09,
/* 39 */				0x0a,
/* 3A */				0,
/* 3B */				0,
/* 3C */				0,
/* 3D */				0,
/* 3E */				0,
/* 3F */				0,


/* 40 */				0,
/* 41 */				0x1e,
/* 42 */				0x30,
/* 43 */				0x2e,
/* 44 */				0x20,
/* 45 */				0x12,
/* 46 */				0x21,
/* 47 */				0x22,
/* 48 */				0x23,
/* 49 */				0x17,
/* 4A */				0x24,
/* 4B */				0x25,
/* 4C */				0x26,
/* 4D */				0x32,
/* 4E */				0x31,
/* 4F */				0x18,


/* 50 */				0x19,
/* 51 */				0x10,
/* 52 */				0x13,
/* 53 */				0x1f,
/* 54 */				0x14,
/* 55 */				0x16,
/* 56 */				0x2f,
/* 57 */				0x11,
/* 58 */				0x2d,
/* 59 */				0x15,
/* 5A */				0x2c,
/* 5B VK_LWIN		*/	0x5b,
/* 5C VK_RWIN		*/	0x5c,
/* 5D VK_APPS		*/	0x5d,
/* 5E */				0,
/* 5F */				0,


/* 60 VK_NUMPAD0	*/	0x52,
/* 61 VK_NUMPAD1	*/	0x4f,
/* 62 VK_NUMPAD2	*/	0x50,
/* 63 VK_NUMPAD3	*/	0x51,
/* 64 VK_NUMPAD4	*/	0x4b,
/* 65 VK_NUMPAD5	*/	0x4c,
/* 66 VK_NUMPAD6	*/	0x4d,
/* 67 VK_NUMPAD7	*/	0x47,
/* 68 VK_NUMPAD8	*/	0x48,
/* 69 VK_NUMPAD9	*/	0x49,
/* 6A VK_MULTIPLY	*/	0xe037,
/* 6B VK_ADD		*/	0x4e,
/* 6C VK_SEPARATOR	*/	0,
/* 6D VK_SUBTRACT	*/	0x4a,
/* 6E VK_DECIMAL	*/	0x53,
/* 6F VK_DIVIDE		*/	0xe035,


/* 70 VK_F1			*/	0x3b,
/* 71 VK_F2			*/	0x3c,
/* 72 VK_F3			*/	0x3d,
/* 73 VK_F4			*/	0x3e,
/* 74 VK_F5			*/	0x3f,
/* 75 VK_F6			*/	0x40,
/* 76 VK_F7			*/	0x41,
/* 77 VK_F8			*/	0x42,
/* 78 VK_F9			*/	0x43,
/* 79 VK_F10		*/	0x44,
/* 7A VK_F11		*/	0x57,
/* 7B VK_F12		*/	0x58,
/* 7C VK_F13		*/	0x64,
/* 7D VK_F14		*/	0x65,
/* 7E VK_F15		*/	0x66,
/* 7F VK_F16		*/	0x67,


/* 80 VK_F17		*/	0x68,
/* 81 VK_F18		*/	0x69,
/* 82 VK_F19		*/	0x6a,
/* 83 VK_F20		*/	0x6b,
/* 84 VK_F21		*/	0x6c,
/* 85 VK_F22		*/	0x6d,
/* 86 VK_F23		*/	0x6e,
/* 87 VK_F24		*/	0x76,
/* 88 */				0,
/* 89 */				0,
/* 8A */				0,
/* 8B */				0,
/* 8C */				0,
/* 8D */				0,
/* 8E */				0,
/* 8F */				0,


/* 90 VK_NUMLOCK	*/	0xe045,
/* 91 VK_SCROLL		*/	0x46,
/* 92 */				0,
/* 93 */				0,
/* 94 */				0,
/* 95 */				0,
/* 96 */				0,
/* 97 */				0,
/* 98 */				0,
/* 99 */				0,
/* 9A */				0,
/* 9B */				0,
/* 9C */				0,
/* 9D */				0,
/* 9E */				0,
/* 9F */				0,


/* A0 VK_LSHIFT		*/	0x2a,
/* A1 VK_RSHIFT		*/	0x36,
/* A2 VK_LCONTROL	*/	0x1d,
/* A3 VK_RCONTROL	*/	0xe01d,
/* A4 VK_LMENU		*/	0x38,
/* A5 VK_RMENU		*/	0xe038,
/* A6 */				0,
/* A7 */				0,
/* A8 */				0,
/* A9 */				0,
/* AA */				0,
/* AB */				0,
/* AC */				0,
/* AD */				0,
/* AE */				0,
/* AF */				0,

/* B0 */				0,
/* B1 */				0,
/* B2 */				0,
/* B3 */				0,
/* B4 */				0,
/* B5 */				0,
/* B6 */				0,
/* B7 */				0,
/* B8 */				0,
/* B9 */				0,
/* BA VK_SEMICOLON	*/	0x27,
/* BB VK_EQUAL		*/	0x0d,
/* BC VK_COMMA		*/	0x33,
/* BD VK_HYPHEN		*/	0x0c,
/* BE VK_PERIOD		*/	0x34,
/* BF VK_SLASH		*/	0x35,


/* C0 VK_BACKQUOTE	*/	0x29,
/* C1 */				0,
/* C2 */				0,
/* C3 */				0,
/* C4 */				0,
/* C5 */				0,
/* C6 */				0,
/* C7 */				0,
/* C8 */				0,
/* C9 */				0,
/* CA */				0,
/* CB */				0,
/* CC */				0,
/* CD */				0,
/* CE */				0,
/* CF */				0,


/* D0 */				0,
/* D1 */				0,
/* D2 */				0,
/* D3 */				0,
/* D4 */				0,
/* D5 */				0,
/* D6 */				0,
/* D7 */				0,
/* D8 */				0,
/* D9 */				0,
/* DA */				0,
/* DB VK_LBRACKET	*/	0x1a,
/* DC VK_BACKSLASH	*/	0x2b,
/* DD VK_RBRACKET	*/	0x1b,
/* DE VK_APOSTROPHE	*/	0x28,
/* DF VK_OFF		*/	0,


/* E0 */				0,
/* E1 */				0,
/* E2 */				0,
/* E3 */				0,
/* E4 */				0,
/* E5 */				0,
/* E6 */				0,
/* E7 */				0,
/* E8 */				0,
/* E9 */				0,
/* EA */				0,
/* EB */				0,
/* EC */				0,
/* ED */				0,
/* EE */				0,
/* EF */				0,


/* F0 */				0,
/* F1 */				0,
/* F2 */				0,
/* F3 */				0,
/* F4 */				0,
/* F5 */				0,
/* F6 */				0,
/* F7 */				0,
/* F8 */				0,
/* F9 */				0,
/* FA */				0,
/* FB */				0,
/* FC */				0,
/* FD */				0,
/* FE */				0,
/* FF */				0


};




/*++

Maps a virtual key to a scan code.  The LR specific virtual keys are mapped to LR specific scan
codes.

--*/
UINT32
VKeyToScanCode(
	UINT32	vkey
	)
{
	if ( vkey >= COUNT_VKEYS )
		{
		return 0;
		}

//	NKDbgPrintfW(TEXT("vkey is %x ad scan code is %x \r\r"),vkey,VKeyToScanCodeTable[vkey]);
	return VKeyToScanCodeTable[vkey];
}




