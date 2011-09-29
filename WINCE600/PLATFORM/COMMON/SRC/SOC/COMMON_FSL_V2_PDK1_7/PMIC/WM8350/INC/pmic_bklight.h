//-----------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
///
/// Copyright (c) 2007 Wolfson Microelectronics plc.  All rights reserved.
///
/// This software as well as any related documentation may only be used or
/// copied in accordance with the terms of the Wolfson Microelectronic plc
/// agreement(s) (e.g. licence or non-disclosure agreement (NDA)).
///
/// The information in this file is furnished for informational use only,
/// is subject to change without notice, and should not be construed as a
/// commitment by Wolfson Microelectronics plc.  Wolfson Microelectronics plc
/// assumes no responsibility or liability for any errors or inaccuracies that
/// may appear in the software or any related documention.
///
/// Except as permitted by the agreement(s), no part of the software or any
/// related documention may be reproduced, stored in a retrieval system, or
/// transmitted in any form or by any means without the express written
/// consent of Wolfson Microelectronics plc.
///
/// @file   pmic_gpio.h
/// @brief  Public prototypes and types used for the PMIC backlight API.
///
/// This file contains the interface for controlling the backlight via the WM8350.
///
/// @version $Id: pmic_bklight.h 648 2007-06-15 22:30:24Z ib $
///
/// @Warning
///   This software is specifically written for Wolfson devices. It may not be
///   used with other devices.
///
//------------------------------------------------------------------------------

#ifndef __PMIC_BKLIGHT_H__
#define __PMIC_BKLIGHT_H__

#include "pmic_basic_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// The current for 64 steps of WM8350
static const DWORD PmicToCurrent[] = {
    511,       // 0
    596,       // 1
    660,       // 2
    809,       // 3
    9369,      // 4
    1106,      // 5
    1277,      // 6
    1532,      // 7
    1745,      // 8
    2085,      // 9
    2404,      // 10
    2872,      // 11
    3404,      // 12
    4085,      // 13
    4723,      // 14
    5681,      // 15
    6660,      // 16
    7936,      // 17
    9213,      // 18
    11128,     // 19
    13191,     // 20
    15766,     // 21
    18340,     // 22
    22191,     // 23
    26383,     // 24
    31532,     // 25
    36660,     // 26
    44383,     // 27
    52574,     // 28
    62894,     // 29
    73213,     // 30
    88745,     // 31
    105532,    // 32    
    126234,    // 33    
    146787,    // 34
    177851,    // 35
    210447,    // 36
    252000,    // 37
    293340,    // 38
    355702,    // 39
    416468,    // 40
    498617,    // 41
    580404,    // 42
    704000,    // 43
    833681,    // 44
    999064,    // 45
    1163574,   // 46
    1412234,   // 47
    1668213,   // 48
    1997596,   // 49
    2324894,   // 50
    2819787,   // 51
    3338511,   // 52
    3994894,   // 53 
    4063404,
    4067021,
    4079574,
    4079574,
    4079574,
    4079574,
    4079574,
    4081064,
    4090000,
};

//------------------------------------------------------------------------------
// Defines
#define BKL_LEVEL_MIN           (0)                                     // off
#define BKL_LEVEL_MAX           (255)                                   // full on
#define BKL_LEVEL_DEFAULT       ((BKL_LEVEL_MAX - BKL_LEVEL_MIN) / 2)


// Physical Range
#define WM8350_LED_MAX_BACKLIGHT_CURRENT_LEVEL      (0x34)              // Max LED current for Dumb LCD
#define WM8350_LED_MIN_BACKLIGHT_CURRENT_LEVEL      (0)                 // 0x16

#define WM8350_LED_BACKLIGHT_RANGE                  (WM8350_LED_MAX_BACKLIGHT_CURRENT_LEVEL - WM8350_LED_MIN_BACKLIGHT_CURRENT_LEVEL + 1)


//------------------------------------------------------------------------------
// Types
typedef enum _BACKLIGHT_MODE {
    BACKLIGHT_CURRENT_CTRL_MODE,
    BACKLIGHT_TRIODE_MODE
} BACKLIGHT_MODE;

typedef enum _BACKLIGHT_CHANNEL {
    BACKLIGHT_ISINK_A,
    BACKLIGHT_ISINK_B,
} BACKLIGHT_CHANNEL;

typedef enum _BACKLIGHT_REGULATOR {
    BACKLIGHT_REGL_DCDC2,
    BACKLIGHT_REGL_DCDC5,
} BACKLIGHT_REGULATOR;

//------------------------------------------------------------------------------
// Functions
PMIC_STATUS PmicBacklightMasterEnable(void);
PMIC_STATUS PmicBacklightMasterDisable(void);

PMIC_STATUS PmicBacklightEnable(BACKLIGHT_CHANNEL channel, BACKLIGHT_REGULATOR regl);
PMIC_STATUS PmicBacklightDisable(BACKLIGHT_CHANNEL channel, BACKLIGHT_REGULATOR regl);

PMIC_STATUS PmicBacklightRampUp(BACKLIGHT_CHANNEL channel);
PMIC_STATUS PmicBacklightRampDown(BACKLIGHT_CHANNEL channel);
PMIC_STATUS PmicBacklightRampOff(BACKLIGHT_CHANNEL channel);

PMIC_STATUS PmicBacklightSetMode(BACKLIGHT_CHANNEL channel, BACKLIGHT_MODE mode);
PMIC_STATUS PmicBacklightGetMode(BACKLIGHT_CHANNEL channel, BACKLIGHT_MODE *mode);

PMIC_STATUS PmicBacklightSetCurrentLevel(BACKLIGHT_CHANNEL channel, UINT8 level);
PMIC_STATUS PmicBacklightGetCurrentLevel(BACKLIGHT_CHANNEL channel, UINT8* level);

UINT8 PMICLog2PhyBacklightLevel(UINT8 BacklightLevel);
UINT8 PMICPhy2LogBacklightLevel(UINT8 BacklightLevel);

#ifdef __cplusplus
}
#endif

#endif // __PMIC_BKLIGHT_H__
