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
//  File:  Keypad.hpp
//
//  Definitions for Keypad Driver
//
//------------------------------------------------------------------------------

#ifndef __KEYPAD_HPP__
#define __KEYPAD_HPP__

#include <windows.h>

//------------------------------------------------------------------------------
// Defines
#define KPP_PDD 8

#define KPP_FUNCTION_ENTRY() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("++%s\r\n"), __WFUNCTION__))
#define KPP_FUNCTION_EXIT() \
    DEBUGMSG(ZONE_FUNCTION, (TEXT("--%s\r\n"), __WFUNCTION__))

//------------------------------------------------------------------------------
// Types

//------------------------------------------------------------------------------
// Functions
BOOL KppInitialize(void);
BOOL KppIsrThreadStart(void);
void KeybdPdd_PowerHandler(BOOL bOff);
void KppRegInit(void);

#endif // __KEYPAD_HPP__
