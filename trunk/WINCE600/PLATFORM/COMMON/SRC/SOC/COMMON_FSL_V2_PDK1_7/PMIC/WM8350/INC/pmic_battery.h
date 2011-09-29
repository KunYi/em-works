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
/// @brief  Public prototypes and types used for the PMIC battery API.
///
/// This file contains the interface for controlling the battery management via the WM8350.
///
/// @version $Id: pmic_battery.h 648 2007-06-15 22:30:24Z ib $
///
/// @Warning
///   This software is specifically written for Wolfson devices. It may not be
///   used with other devices.
///
//------------------------------------------------------------------------------

#ifndef __PMIC_BATTERY_H__
#define __PMIC_BATTERY_H__

#include "pmic_basic_types.h"

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines

#define WM8350_BATTERY_CHRGDET_VOLT_THRESHOLD_HIGH   0x0F3C  // 3900 millivolts
#define WM8350_BATTERY_CHRGDET_VOLT_THRESHOLD_LOW    0x0C1C  // 3100 millivolts

// Charger status indicator GPIO
#define BSP_PMIC_CH_IND                              WM_GPIO_10

// Trickle charge threshold
#define BSP_PMIC_CHARGE_VOLTAGE_THRESHOLD            WM_TRICKLE_THRESHOLD_3_1V

// Temp choke while trickle charging
#define BSP_PMIC_TRICKLE_TEMP_CHOKE                  FALSE

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
#ifdef BSP_PMIC_WM8350
PMIC_STATUS PmicBatteryPauseCharging(BOOL pause);
PMIC_STATUS PmicBatterySetEOCChargeVoltage(UINT8 chargevoltagelevel);
PMIC_STATUS PmicBatteryGetEOCChargeVoltage(UINT8* chargevoltagelevel);
PMIC_STATUS PmicBatteryGetChargerStatus(UINT8 *status);
PMIC_STATUS PmicBatterySetTimer(UINT8 timeout);
#endif // BSP_PMIC_WM8350
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
