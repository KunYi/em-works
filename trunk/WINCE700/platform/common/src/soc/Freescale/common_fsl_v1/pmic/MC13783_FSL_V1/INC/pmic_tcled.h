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
//  Copyright (C) 2005, Motorola Inc. All Rights Reserved
//
//-----------------------------------------------------------------------------
/*---------------------------------------------------------------------------
* Copyright (C) 2006, Freescale Semiconductor, Inc. All Rights Reserved.
* THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
* AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
*--------------------------------------------------------------------------*/

//------------------------------------------------------------------------------
//
// File: pmic_tcled.h
//
//  Defines the public prototypes and types used for the PMIC Tri-Color LED API.
//
//------------------------------------------------------------------------------

#ifndef __PMIC_TCLED_H__
#define __PMIC_TCLED_H__

#include "pmic_basic_types.h"

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines
#define MC13783_MAX_FUNLIGHT_CURRENT_LEVEL   TCLED_CUR_LEVEL_4
#define MC13783_MAX_FUNLIGHT_CYCLE_TIME      TC_CYCLE_TIME_4
#define MC13783_MAX_FUNLIGHT_DUTY_CYCLE      31
#define MC13783_BLENDED_RAMPS_SLOW           0x0
#define MC13783_BLENDED_RAMPS_FAST           0x1
#define MC13783_SAW_RAMPS_SLOW               0x2
#define MC13783_SAW_RAMPS_FAST               0x3
#define MC13783_BLENDED_BOWTIE_RAMPS_SLOW    0x4
#define MC13783_BLENDED_BOWTIE_RAMPS_FAST    0x5
#define MC13783_CHASING_LIGHTS_RGB_SLOW      0x6
#define MC13783_CHASING_LIGHTS_RGB_FAST      0x7
#define MC13783_CHASING_LIGHTS_BGR_SLOW      0x8
#define MC13783_CHASING_LIGHTS_BGR_FAST      0x9

//------------------------------------------------------------------------------
// Types

typedef enum _FUNLIGHT_BANK {
    TCLED_FUN_BANK1 = 0,
    TCLED_FUN_BANK2,
    TCLED_FUN_BANK3
} FUNLIGHT_BANK;

typedef enum _TCLED_MODE {
    TCLED_IND_MODE = 0,
    TCLED_FUN_MODE
} TCLED_MODE;

typedef enum _IND_CHANNEL {
    TCLED_IND_RED = 0 ,
    TCLED_IND_GREEN
} IND_CHANNEL;

typedef enum _FUNLIGHT_CHANNEL {
    TCLED_FUN_CHANNEL1 = 0,
    TCLED_FUN_CHANNEL2,
    TCLED_FUN_CHANNEL3
} FUNLIGHT_CHANNEL;


typedef enum _IND_BLINK_MODE {
    TCLED_IND_OFF = 0,
    TCLED_IND_BLINK_1,
    TCLED_IND_BLINK_2,
    TCLED_IND_BLINK_3,
    TCLED_IND_BLINK_4,
    TCLED_IND_BLINK_5,
    TCLED_IND_BLINK_6,
    TCLED_IND_BLINK_7,
    TCLED_IND_BLINK_8,
    TCLED_IND_BLINK_9,
    TCLED_IND_BLINK_10,
    TCLED_IND_BLINK_11,
    TCLED_IND_ON
} TCLED_IND_BLINK_MODE;


typedef enum _TCLED_CUR_LEVEL {
    TCLED_CUR_LEVEL_1 = 0,
    TCLED_CUR_LEVEL_2,
    TCLED_CUR_LEVEL_3,
    TCLED_CUR_LEVEL_4
} TCLED_CUR_LEVEL;


typedef enum _TCLED_FUN_CYCLE_TIME {
    TC_CYCLE_TIME_1 = 0,
    TC_CYCLE_TIME_2,
    TC_CYCLE_TIME_3,
    TC_CYCLE_TIME_4
} TCLED_FUN_CYCLE_TIME;


typedef enum _TCLED_FUN_SPEED {
    TC_OFF = 0,
    TC_SLOW,
    TC_FAST
} TCLED_FUN_SPEED;


typedef enum _TCLED_FUN_STROBE_SPEED {
    TC_STROBE_OFF = 0,
    TC_STROBE_SLOW,
    TC_STROBE_FAST
} TCLED_FUN_STROBE_SPEED;

typedef enum _CHASELIGHT_PATTERN{ 
    RGB = 0, 
    BGR 
} CHASELIGHT_PATTERN;

//------------------------------------------------------------------------------
// Functions

PMIC_STATUS PmicTCLEDEnable(TCLED_MODE mode, FUNLIGHT_BANK bank);
PMIC_STATUS PmicTCLEDDisable(FUNLIGHT_BANK bank);
PMIC_STATUS PmicTCLEDGetMode(TCLED_MODE* mode, FUNLIGHT_BANK bank);
PMIC_STATUS PmicTCLEDIndicatorSetCurrentLevel(IND_CHANNEL channel, TCLED_CUR_LEVEL level);
PMIC_STATUS PmicTCLEDIndicatorGetCurrentLevel(IND_CHANNEL channel, TCLED_CUR_LEVEL* level);
PMIC_STATUS PmicTCLEDIndicatorSetBlinkPattern(IND_CHANNEL channel, TCLED_IND_BLINK_MODE mode, BOOL skip);
PMIC_STATUS PmicTCLEDIndicatorGetBlinkPattern(IND_CHANNEL channel, TCLED_IND_BLINK_MODE* mode, BOOL* skip);
PMIC_STATUS PmicTCLEDFunLightSetCurrentLevel(FUNLIGHT_BANK bank, FUNLIGHT_CHANNEL channel, TCLED_CUR_LEVEL level);
PMIC_STATUS PmicTCLEDFunLightGetCurrentLevel(FUNLIGHT_BANK bank, FUNLIGHT_CHANNEL channel, TCLED_CUR_LEVEL* level);
PMIC_STATUS PmicTCLEDFunLightSetCycleTime(FUNLIGHT_BANK bank, TCLED_FUN_CYCLE_TIME ct);
PMIC_STATUS PmicTCLEDFunLightGetCycleTime(FUNLIGHT_BANK bank, TCLED_FUN_CYCLE_TIME* ct);
PMIC_STATUS PmicTCLEDFunLightSetDutyCycle(FUNLIGHT_BANK bank, FUNLIGHT_CHANNEL channel, unsigned char dc);
PMIC_STATUS PmicTCLEDFunLightGetDutyCycle(FUNLIGHT_BANK bank, FUNLIGHT_CHANNEL channel, unsigned char* dc);
PMIC_STATUS PmicTCLEDFunLightBlendedRamps(FUNLIGHT_BANK bank, TCLED_FUN_SPEED speed);
PMIC_STATUS PmicTCLEDFunLightSawRamps(FUNLIGHT_BANK bank, TCLED_FUN_SPEED speed);
PMIC_STATUS PmicTCLEDFunLightBlendedBowtie(FUNLIGHT_BANK bank, TCLED_FUN_SPEED speed);
PMIC_STATUS PmicTCLEDFunLightChasingLightsPattern(FUNLIGHT_BANK bank, CHASELIGHT_PATTERN pattern, TCLED_FUN_SPEED speed);
PMIC_STATUS PmicTCLEDFunLightStrobe(FUNLIGHT_CHANNEL channel, TCLED_FUN_STROBE_SPEED speed);
PMIC_STATUS PmicTCLEDFunLightRampUp(FUNLIGHT_BANK bank, FUNLIGHT_CHANNEL channel);
PMIC_STATUS PmicTCLEDFunLightRampDown(FUNLIGHT_BANK bank, FUNLIGHT_CHANNEL channel);
PMIC_STATUS PmicTCLEDFunLightRampOff(FUNLIGHT_BANK bank, FUNLIGHT_CHANNEL channel);
PMIC_STATUS PmicTCLEDFunLightTriodeOn(FUNLIGHT_BANK bank, FUNLIGHT_CHANNEL channel);
PMIC_STATUS PmicTCLEDFunLightTriodeOff(FUNLIGHT_BANK bank, FUNLIGHT_CHANNEL channel);
PMIC_STATUS PmicTCLEDEnableEdgeSlow(void);
PMIC_STATUS PmicTCLEDDisableEdgeSlow(void);
PMIC_STATUS PmicTCLEDEnableHalfCurrent(void);
PMIC_STATUS PmicTCLEDDisableHalfCurrent(void);


#ifdef __cplusplus
}
#endif

#endif // __PMIC_TCLED_H__
