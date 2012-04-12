//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  pmic_adc.h
//
//  This file contains the PMIC ADC (A/D Converter, including Touch Screen) SDK 
//  interface that is used by applications and other drivers to access registers 
//  of the PMIC.
//
//-----------------------------------------------------------------------------

#ifndef __PMIC_ADC_H__
#define __PMIC_ADC_H__

#include "pmic_basic_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ADC_CHARGER_MAX_VOLTAGE    20000    // 20V
#define ADC_CHARGER_MIN_VOLTAGE    0    // 0V

#define ADC_CHARGER_SNS_RESISTOR    100    // 100mOhm

// charger current (voltage difference) is ranging from -500mV to 500mV
// the actual current is Vdiff/Rcs, Rcs = 100mOhm
// charger current would range from -5Amp to 5Amp
#define ADC_CHARGER_MAX_CURRENT    500*1000/ADC_CHARGER_SNS_RESISTOR // 5 Amp, -5Amp depends on charge/discharge
#define ADC_CHARGER_MIN_CURRENT    0    // 0 mAmp

#define ADC_BATT_MAX_VOLTAGE    4650    // 4.65V
#define ADC_BATT_MIN_VOLTAGE    2500    // 2.5V

#define ADC_BATT_SNS_RESISTOR    15    // 15 mOhm

// battery current (battery difference) is ranging from -50mV to 50mV
// the actual current is Vdiff/Rsns, Rsns = 15mOhm
// charger current would range from -3.3Amp to 3.3Amp
#define ADC_BATT_MAX_CURRENT    50*1000/ADC_BATT_SNS_RESISTOR // 3.3 Amp, -3.3Amp depends on charge/discharge
#define ADC_BATT_MIN_CURRENT    0    // 0 mAmp

#define ADC_SAMPLE_MAX_VALUE    1023    // 0X3FF

#define MC13892_ADC_MAX_COMPARATOR_LEVEL    0x3FF

//-----------------------------------------------------------------------------
// External Functions

//-----------------------------------------------------------------------------
// External Variables

//-----------------------------------------------------------------------------
// Types

typedef enum _MC13892_TOUCH_MODE {
    TM_INACTIVE = 0,
    TM_INTERRUPT,
    TM_TOUCHSCREEM
} MC13892_TOUCH_MODE;
typedef MC13892_TOUCH_MODE PMIC_TOUCH_MODE;

typedef enum _PMIC_ADC_CONVERTOR_MODE
{
    ADC_8CHAN_1X = 0, // RAND = 0, 8 channels, 1 sample
    ADC_1CHAN_8X      // RAND = 1, 1 channel, reads 8 sequential values
} PMIC_ADC_CONVERTOR_MODE;


// Function Prototypes
PMIC_STATUS PmicADCInit(void);
void        PmicADCDeinit(void);
PMIC_STATUS PmicADCGetSingleChannelOneSample(UINT16 channel, UINT16 * pResult);
PMIC_STATUS PmicADCGetSingleChannelEightSamples(UINT16 channel, UINT16 * pResult);
PMIC_STATUS PmicADCGetMultipleChannelsSamples(UINT16 channels, UINT16 * pResult);
PMIC_STATUS PmicADCGetHandsetCurrent(PMIC_ADC_CONVERTOR_MODE mode, UINT16 *pResult);
PMIC_STATUS PmicADCTouchRead(UINT16* x, UINT16* y);
PMIC_STATUS PmicADCTouchStandby(BOOL intEna);

#ifdef __cplusplus
}
#endif

#endif // __PMIC_ADC_H__
