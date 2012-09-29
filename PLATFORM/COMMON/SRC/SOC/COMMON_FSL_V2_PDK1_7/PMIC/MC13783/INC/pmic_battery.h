/*---------------------------------------------------------------------------
* Copyright (C) 2006, Freescale Semiconductor, Inc. All Rights Reserved.
* THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
* AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
*--------------------------------------------------------------------------*/
//------------------------------------------------------------------------------
//
// File: pmic_battery.h
//
//  Defines the public prototypes and types used for the PMIC battery API 
//  (MC13783).
//
//------------------------------------------------------------------------------

#ifndef __PMIC_BATTERY_H__
#define __PMIC_BATTERY_H__

#include "pmic_basic_types.h"

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines
#define MC13783_BATTERY_MAX_CHARGE_VOLTAGE_LEVEL          0x07        
#define MC13783_BATTERY_MAX_CHARGE_CURRENT_LEVEL          0x0F
#define MC13783_BATTERY_MAX_TRICKLE_CURRENT_LEVEL         0x07
#define MC13783_BATTERY_MAX_OVERVOLTAGE_THRESHOLD_LEVEL   0x03        
#define MC13783_BATTERY_MAX_COINCELL_VOLTAGE_LEVEL        0x06
#define MC13783_BATTERY_MAX_FETCONTROL_INPUT_LEVEL        0x03

#define MC13783_BATTERY_CHRGDET_VOLT_THRESHOLD_HIGH   0x0ED3  // 3795 millivolts
#define MC13783_BATTERY_CHRGDET_VOLT_THRESHOLD_LOW    0x0DAC  // 3500 millivolts

#define CHRGISNS_CHNL        4        // CHRGRAW charger current when AD_SEL = 0


//------------------------------------------------------------------------------
// Types

//
// This enumeration defines battery charagers.
// 
typedef enum {
        BATT_MAIN_CHGR = 0,         // Main battery charger
        BATT_CELL_CHGR,             // CoinCell battery charger
        BATT_TRCKLE_CHGR            // Trickle charger
} BATT_CHARGER;

typedef enum {
    DUAL_PATH = 0,
    SINGLE_PATH,
    SERIAL_PATH,
    DUAL_INPUT_SINGLE_PATH,
    DUAL_INPUT_SERIAL_PATH,
    DUAL_INPUT_DUAL_PATH,
    INVALID_CHARGER_MODE
}CHARGER_MODE;

typedef enum {
    LLOW = 0,  // GND
    OPEN,      // HI Z
    HHIGH      // VPMIC
}CHARGERMODE_PIN;    

//------------------------------------------------------------------------------
// Functions

PMIC_STATUS PmicBatteryEnableEolComparator (void);
PMIC_STATUS PmicBatteryDisableEolComparator (void);
PMIC_STATUS PmicBatteryEnableCoincellCharger (void);
PMIC_STATUS PmicBatteryDisableCoincellCharger (void);
PMIC_STATUS PmicBatterySetCoincellVoltage (UINT8 coincellvoltagelevel);
PMIC_STATUS PmicBatteryGetCoincellVoltage (UINT8* coincellvoltagelevel);
PMIC_STATUS PmicBatterySetChargeVoltage(UINT8 chargevoltagelevel);
PMIC_STATUS PmicBatteryGetChargeVoltage(UINT8* chargevoltagelevel);
PMIC_STATUS PmicBatterySetChargeCurrent (UINT8 chargecurrentlevel);
PMIC_STATUS PmicBatteryGetChargeCurrent (UINT8* chargecurrentlevel);
PMIC_STATUS PmicBatterySetTrickleCurrent(UINT8 tricklecurrentlevel);
PMIC_STATUS PmicBatteryGetTrickleCurrent(UINT8* tricklecurrentlevel);
PMIC_STATUS PmicBatteryFETControl(UINT8 fetcontrol);
PMIC_STATUS PmicBatteryReverseDisable(void);
PMIC_STATUS PmicBatteryReverseEnable(void);
PMIC_STATUS PmicBatterySetOvervoltageThreshold(UINT8 ovthresholdlevel);
PMIC_STATUS PmicBatteryGetOvervoltageThreshold(UINT8* ovthresholdlevel);
PMIC_STATUS PmicBatteryUnregulatedChargeDisable(void);
PMIC_STATUS PmicBatteryUnregulatedChargeEnable(void);
PMIC_STATUS PmicBatteryChargeLedDisable(void);
PMIC_STATUS PmicBatteryChargeLedEnable(void);
PMIC_STATUS PmicBatteryEnablePulldown(void);
PMIC_STATUS PmicBatteryDisablePulldown(void);
PMIC_STATUS PmicBatteryGetChargerMode(CHARGER_MODE *mode);

PMIC_STATUS PmicBatteryEnableAdChannel5 (void);
PMIC_STATUS PmicBatteryDisableAdChannel5 (void);
PMIC_STATUS PmicBatterySetCoincellCurrentlimit (UINT8 coincellcurrentlevel);
PMIC_STATUS PmicBatteryGetCoincellCurrentlimit (UINT8* coincellcurrentlevel);
PMIC_STATUS PmicBatterySetEolTrip (UINT8 eoltriplevel);
PMIC_STATUS PmicBatteryGetEolTrip (UINT8* eoltriplevel);

// these APIs are an attempt to standardize on an interface for PMIC
PMIC_STATUS PmicBatterEnableCharger(BATT_CHARGER chgr, UINT8 c_voltage, UINT8 c_current);
PMIC_STATUS PmicBatterDisableCharger(BATT_CHARGER chgr);
PMIC_STATUS PmicBatterSetCharger(BATT_CHARGER chgr, UINT8 c_voltage, UINT8 c_current);
PMIC_STATUS PmicBatterGetChargerSetting(BATT_CHARGER chgr, UINT8* c_voltage, UINT8* c_current);
PMIC_STATUS PmicBatterGetChargeCurrent(UINT16* c_current);
PMIC_STATUS PmicBatterEnableEol(void);
PMIC_STATUS PmicBatterDisableEol(void);
PMIC_STATUS PmicBatterLedControl(BOOL on);
PMIC_STATUS PmicBatterSetReverseSupply(BOOL enable);
PMIC_STATUS PmicBatterSetUnregulated(BOOL enable);


#ifdef __cplusplus
}
#endif

#endif // __PMIC_BATTERY_H__
