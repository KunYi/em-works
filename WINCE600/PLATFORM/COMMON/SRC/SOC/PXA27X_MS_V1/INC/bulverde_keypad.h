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
//------------------------------------------------------------------------------
//
//  Header: bulverde_keypad.h
//
//  Defines the keypad controller register layout and associated types 
//  and constants.
//
#ifndef __BULVERDE_KEYPAD_H
#define __BULVERDE_KEYPAD_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  Type:  BULVERDE_KEYPAD_REG
//
//  Defines the keypad control register layout.
//

#include <xllp_keypad.h>

typedef XLLP_KEYPAD_REGS BULVERDE_KEYPAD_REG;
typedef XLLP_KEYPAD_REGS *PBULVERDE_KEYPAD_REG;

//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif 
