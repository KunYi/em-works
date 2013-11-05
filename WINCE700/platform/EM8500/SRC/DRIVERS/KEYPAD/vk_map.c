// All rights reserved ADENEO EMBEDDED 2010
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
//  File: keypad_drv.c
//
#include "bsp.h"
#include <winuserm.h>
#include "gpio_keypad.h"

#ifndef dimof
#define dimof(x)            (sizeof(x)/sizeof(x[0]))
#endif

#define NB_KEYS 8


//------------------------------------------------------------------------------

const GPIO_KEY g_keypadVK[NB_KEYS] = { 
    {1,  VK_DOWN  },   
    {2,  VK_UP    },
    {3,  VK_LEFT  },
    {4,  VK_RIGHT },
    {5,  L'1'     },
    {6,  L'2'     },
    {7,  L'3'     },
    {8,  VK_RETURN},
        
};

const int g_nbKeys = NB_KEYS;

//------------------------------------------------------------------------------

const UCHAR g_wakeupVKeys[]     = { VK_PERIOD };
const int g_nbWakeupVKeys = dimof(g_wakeupVKeys);


//------------------------------------------------------------------------------

static const UCHAR off[]     = { VK_PERIOD };
static const KEYPAD_REMAP_ITEM remapItems[] = {
    { VK_OFF, dimof(off),     3000, off     },
};

const KEYPAD_REMAP g_keypadRemap = { 0, /*dimof(remapItems),*/ remapItems };

//------------------------------------------------------------------------------

static const UCHAR softkeys[] = { VK_CONTROL };

static const KEYPAD_REPEAT_BLOCK softkeyBlock = { dimof(softkeys), softkeys };

static const KEYPAD_REPEAT_ITEM repeatItems[] = {
    {VK_DOWN,             500, 500, TRUE,  &softkeyBlock},
    {VK_UP,               500, 500, TRUE,  &softkeyBlock },
    {VK_LEFT,             500, 500, TRUE,  &softkeyBlock },
    {VK_RIGHT,            500, 500, TRUE,  &softkeyBlock },
    {L'1',                500, 500, TRUE,  NULL },
    {L'2',                500, 500, TRUE,  NULL },
    {L'3',                500, 500, TRUE,  NULL },
    {VK_RETURN,			  500, 500, TRUE,  NULL },
};

const KEYPAD_REPEAT g_keypadRepeat = { dimof(repeatItems), repeatItems };

