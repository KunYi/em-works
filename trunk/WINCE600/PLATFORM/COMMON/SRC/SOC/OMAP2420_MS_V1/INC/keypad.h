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
//  File:  keypad.h
//
#ifndef __KEYPAD_H
#define __KEYPAD_H

//------------------------------------------------------------------------------
//
//  Define: KEYPAD_ROWS/KEYPAD_COLUMNS
//
#define KEYPAD_ROWS         5
#define KEYPAD_COLUMNS      6

//------------------------------------------------------------------------------
//
//  Global:  g_keypadVK
//
//  Global array mapping physical keypad location to virtual key
//
extern const UCHAR g_keypadVK[KEYPAD_ROWS * KEYPAD_COLUMNS];

//------------------------------------------------------------------------------

typedef struct {
    const UCHAR  vkey;                  // virtual key to be result of remapping
    const UCHAR  keys;                  // number of keys to map
    const USHORT delay;                 // debounce delay in ms 
    const UCHAR  *pVKeys;               // virtual keys to map from
} KEYPAD_REMAP_ITEM;

typedef struct {
    const USHORT count;
    const KEYPAD_REMAP_ITEM *pItem;
} KEYPAD_REMAP;

extern const KEYPAD_REMAP g_keypadRemap;

//------------------------------------------------------------------------------

typedef struct {
    const UCHAR  count;                 // number of keys to block repeat
    const UCHAR  *pVKey;                // virtual keys blocking repeat
} KEYPAD_REPEAT_BLOCK;

typedef struct {
    const UCHAR  vkey;                  // virtual key to be repeated
    const USHORT firstDelay;            // delay before first repeat
    const USHORT nextDelay;             // delay before next repeat
    const BOOL   silent;                // repeat should be without key click
    const KEYPAD_REPEAT_BLOCK *pBlock;  // virtual keys blocking repeat
} KEYPAD_REPEAT_ITEM;

typedef struct {
    const USHORT count;
    const KEYPAD_REPEAT_ITEM *pItem;
} KEYPAD_REPEAT;

extern const KEYPAD_REPEAT g_keypadRepeat;

//------------------------------------------------------------------------------

#endif // __KEYPAD_H
