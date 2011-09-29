/*---------------------------------------------------------------------------
* Copyright (C) 2008, Freescale Semiconductor, Inc. All Rights Reserved.
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
#define MC13892_LED_MAX_BACKLIGHT_CURRENT_LEVEL   7
#define MC13892_LED_MIN_BACKLIGHT_CURRENT_LEVEL   0
#define MC13892_LED_DEFAULT_BACKLIGHT_CURRENT_LEVEL       ((MC13892_LED_MAX_BACKLIGHT_CURRENT_LEVEL - \
                                MC13892_LED_MIN_BACKLIGHT_CURRENT_LEVEL) / 2)


#define MC13892_LED_MAX_BACKLIGHT_DUTY_CYCLE      0x1F
#define MC13892_LED_DEFAULT_BACKLIGHT_DUTY_CYCLE  MC13892_LED_MAX_BACKLIGHT_DUTY_CYCLE / 2

//------------------------------------------------------------------------------
// Types

typedef enum _BACKLIGHT_CHANNEL {
    BACKLIGHT_MAIN_DISPLAY,
    BACKLIGHT_AUX_DISPLAY,
    BACKLIGHT_KEYPAD
} BACKLIGHT_CHANNEL;


//------------------------------------------------------------------------------
// Functions

PMIC_STATUS PmicBacklightEnableHIMode(BACKLIGHT_CHANNEL channel);
PMIC_STATUS PmicBacklightDisableHIMode(BACKLIGHT_CHANNEL channel);

PMIC_STATUS PmicBacklightEnableRamp(BACKLIGHT_CHANNEL channel);
PMIC_STATUS PmicBacklightDisableRamp(BACKLIGHT_CHANNEL channel);

PMIC_STATUS PmicBacklightSetCurrentLevel(BACKLIGHT_CHANNEL channel, UINT8 level);
PMIC_STATUS PmicBacklightGetCurrentLevel(BACKLIGHT_CHANNEL channel, UINT8* level);

PMIC_STATUS PmicBacklightSetDutyCycle(BACKLIGHT_CHANNEL channel, UINT8 cycle);
PMIC_STATUS PmicBacklightGetDutyCycle(BACKLIGHT_CHANNEL channel, UINT8* cycle);


#ifdef __cplusplus
}
#endif

#endif // __PMIC_BKLIGHT_H__
