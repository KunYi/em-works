/*---------------------------------------------------------------------------
* Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
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
#define MC13892_MAX_FUNLIGHT_CURRENT_LEVEL   TCLED_CUR_LEVEL_4
#define MC13892_MAX_FUNLIGHT_CYCLE_TIME      TC_CYCLE_TIME_4
#define MC13892_MAX_FUNLIGHT_DUTY_CYCLE      31
#define MC13892_BLENDED_RAMPS_SLOW           0x0
#define MC13892_BLENDED_RAMPS_FAST           0x1
#define MC13892_SAW_RAMPS_SLOW               0x2
#define MC13892_SAW_RAMPS_FAST               0x3
#define MC13892_BLENDED_BOWTIE_RAMPS_SLOW    0x4
#define MC13892_BLENDED_BOWTIE_RAMPS_FAST    0x5
#define MC13892_CHASING_LIGHTS_RGB_SLOW      0x6
#define MC13892_CHASING_LIGHTS_RGB_FAST      0x7
#define MC13892_CHASING_LIGHTS_BGR_SLOW      0x8
#define MC13892_CHASING_LIGHTS_BGR_FAST      0x9

#define MC13892_LED_MAX_LED_CURRENT_LEVEL   7
#define MC13892_LED_MIN_LED_CURRENT_LEVEL   0
#define MC13892_LED_DEFAULT_LED_CURRENT_LEVEL       ((MC13892_LED_MAX_LED_CURRENT_LEVEL - \
                                MC13892_LED_MIN_LED_CURRENT_LEVEL) / 2)

#define MC13892_LED_MAX_LED_BLINK_PERIOD    3
#define MC13892_LED_MAX_LED_DUTY_CYCLE      0x20

//------------------------------------------------------------------------------
// Types

typedef enum _LED_CHANNEL {
    TCLED_RED ,
    TCLED_GREEN,
    TCLED_BLUE
} LED_CHANNEL;


//------------------------------------------------------------------------------
// Functions


PMIC_STATUS PmicLEDIndicatorEnableRamp(LED_CHANNEL channel);
PMIC_STATUS PmicLEDIndicatorDisableRamp(LED_CHANNEL channel);

PMIC_STATUS PmicLEDIndicatorSetCurrentLevel(LED_CHANNEL channel, unsigned char level);
PMIC_STATUS PmicLEDIndicatorGetCurrentLevel(LED_CHANNEL channel, unsigned char* level);

PMIC_STATUS PmicLEDIndicatorSetDutyCycle(LED_CHANNEL channel, unsigned char dc);
PMIC_STATUS PmicLEDIndicatorGetDutyCycle(LED_CHANNEL channel,  unsigned char* dc);

PMIC_STATUS PmicLEDIndicatorSetBlinkPeriod(LED_CHANNEL channel, unsigned char bp);
PMIC_STATUS PmicLEDIndicatorGetBlinkPeriod(LED_CHANNEL channel,  unsigned char* bp);

PMIC_STATUS PmicLEDIndicatorEnableSWBST();
PMIC_STATUS PmicLEDIndicatorDisableSWBST();


#ifdef __cplusplus
}
#endif

#endif // __PMIC_TCLED_H__
