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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
//-----------------------------------------------------------------------------
//
//  Copyright (C) 2004, Motorola Inc. All Rights Reserved
//
//------------------------------------------------------------------------------
/*---------------------------------------------------------------------------
* Copyright (C) 2006, Freescale Semiconductor, Inc. All Rights Reserved.
* THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
* AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
*--------------------------------------------------------------------------*/
//------------------------------------------------------------------------------
//
// File: pmic_bklight.h
//
//  Defines the public prototypes and types used for the PMIC backlight API.
//
//------------------------------------------------------------------------------

#ifndef __PMIC_BKLIGHT_H__
#define __PMIC_BKLIGHT_H__

#include "pmic_basic_types.h"

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines
#define MC13783_LED_MAX_BACKLIGHT_CURRENT_LEVEL   7
#define MC13783_LED_MIN_BACKLIGHT_CURRENT_LEVEL   0
#define MC13783_LED_DEFAULT_BACKLIGHT_CURRENT_LEVEL       ((MC13783_LED_MAX_BACKLIGHT_CURRENT_LEVEL - \
                                MC13783_LED_MIN_BACKLIGHT_CURRENT_LEVEL) / 2)


#define MC13783_LED_MAX_BACKLIGHT_DUTY_CYCLE      0xF
#define MC13783_LED_DEFAULT_BACKLIGHT_DUTY_CYCLE  MC13783_LED_MAX_BACKLIGHT_DUTY_CYCLE / 2

#define MC13783_LED_MAX_BACKLIGHT_PERIOD          3
#define MC13783_LED_MIN_BACKLIGHT_PERIOD          0

#define MC13783_LED_MAX_BACKLIGHT_BOOST_ABMS          7
#define MC13783_LED_MAX_BACKLIGHT_BOOST_ABR             3

//------------------------------------------------------------------------------
// Types
typedef enum _BACKLIGHT_MODE {
    BACKLIGHT_CURRENT_CTRL_MODE,
    BACKLIGHT_TRIODE_MODE
} BACKLIGHT_MODE;

typedef enum _BACKLIGHT_CHANNEL {
    BACKLIGHT_MAIN_DISPLAY,
    BACKLIGHT_AUX_DISPLAY,
    BACKLIGHT_KEYPAD
} BACKLIGHT_CHANNEL;

typedef enum _BACKLIGHT_STROBE_MODE {
    BACKLIGHT_STROBE_NONE
} BACKLIGHT_STROBE_MODE;


//------------------------------------------------------------------------------
// Functions
PMIC_STATUS PmicBacklightMasterEnable(void);
PMIC_STATUS PmicBacklightMasterDisable(void);

PMIC_STATUS PmicBacklightRampUp(BACKLIGHT_CHANNEL channel);
PMIC_STATUS PmicBacklightRampDown(BACKLIGHT_CHANNEL channel);
PMIC_STATUS PmicBacklightRampOff(BACKLIGHT_CHANNEL channel);

PMIC_STATUS PmicBacklightSetMode(BACKLIGHT_CHANNEL channel, BACKLIGHT_MODE mode);
PMIC_STATUS PmicBacklightGetMode(BACKLIGHT_CHANNEL channel, BACKLIGHT_MODE *mode);

PMIC_STATUS PmicBacklightSetCurrentLevel(BACKLIGHT_CHANNEL channel, UINT8 level);
PMIC_STATUS PmicBacklightGetCurrentLevel(BACKLIGHT_CHANNEL channel, UINT8* level);

PMIC_STATUS PmicBacklightSetDutyCycle(BACKLIGHT_CHANNEL channel, UINT8 cycle);
PMIC_STATUS PmicBacklightGetDutyCycle(BACKLIGHT_CHANNEL channel, UINT8* cycle);

PMIC_STATUS PmicBacklightSetCycleTime(UINT8 period);
PMIC_STATUS PmicBacklightGetCycleTime(UINT8* period);

PMIC_STATUS PmicBacklightEnableEdgeSlow(void);
PMIC_STATUS PmicBacklightDisableEdgeSlow(void);
PMIC_STATUS PmicBacklightGetEdgeSlow(BOOL *edge);

PMIC_STATUS PmicBacklightEnableBoostMode();
PMIC_STATUS PmicBacklightDisableBoostMode();
PMIC_STATUS PmicBacklightSetBoostMode(UINT32 abms, UINT32 abr);

#ifdef __cplusplus
}
#endif

#endif // __PMIC_BKLIGHT_H__
