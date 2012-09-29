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
/// @brief  Public prototypes and types used for the PMIC regulator API.
///
/// This file contains the interface for controlling the regulators on the WM8350.
///
/// @version $Id: pmic_regulator.h 648 2007-06-15 22:30:24Z ib $
///
/// @Warning
///   This software is specifically written for Wolfson devices. It may not be
///   used with other devices.
///
//------------------------------------------------------------------------------

#ifndef __PMIC_REGULATOR_H__
#define __PMIC_REGULATOR_H__

#include "pmic_basic_types.h"

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Defines

//------------------------------------------------------------------------------
// Types
// regulators
typedef enum _WM8350_REGULATOR
{
    // switch mode regulators
    DCDC1 = 0,
    DCDC2,
    DCDC3,
    DCDC4,
    DCDC5,
    DCDC6,
    // linear regulators
    LDO1 = 0x10,
    LDO2,
    LDO3,
    LDO4,
    // Mappings to MC13783 regulators as in WM8350 daughtercard
    SW1A = DCDC1,
    SW2A = DCDC6,
    SW2B = DCDC4,
    SW3 = DCDC2,
    VIOHI = DCDC3,
    VIOLO = DCDC4,
    VCAM = LDO1,
    VMMC1 = LDO1,
    VMMC2 = LDO1,
    VSIM = LDO2,
    VESIM = LDO2,
    VGEN = LDO3,
    VDIG = LDO3,
    VRF1 = LDO4,
    VRF2 = LDO4
} WM8350_REGULATOR;
typedef WM8350_REGULATOR PMIC_REGULATOR_SREG;
typedef WM8350_REGULATOR PMIC_REGULATOR_VREG;

/*************************************************
 * A type which can be either regulator or millivolts.
 */
typedef WM8350_REGULATOR    PMIC_REGULATOR;

/**
 * Regulator voltages.
 *
 * Voltages are specified in millivolts.
 */
typedef UINT32 PMIC_REGULATOR_VOLTAGE;
typedef enum PMIC_REGULATOR_SREG_VOLTAGE
{
    PMIC_SREG_0_85V  =  850, /* output = 0.85V  */
    PMIC_SREG_0_875V =  875, /* output = 0.875V */
    PMIC_SREG_0_9V   =  900, /* output = 0.9V   */
    PMIC_SREG_0_925V =  925, /* output = 0.925V */
    PMIC_SREG_0_95V  =  950, /* output = 0.95V  */
    PMIC_SREG_0_975V =  975, /* output = 0.975V */
    PMIC_SREG_1_0V   = 1000, /* output = 1V     */
    PMIC_SREG_1_025V = 1025, /* output = 1.025V */
    PMIC_SREG_1_05V  = 1050, /* output = 1.05V  */
    PMIC_SREG_1_075V = 1075, /* output = 1.075V */
    PMIC_SREG_1_1V   = 1100, /* output = 1.1V   */
    PMIC_SREG_1_125V = 1125, /* output = 1.125V */
    PMIC_SREG_1_15V  = 1150, /* output = 1.15V  */
    PMIC_SREG_1_175V = 1175, /* output = 1.175V */
    PMIC_SREG_1_2V   = 1200, /* output = 1.2V   */
    PMIC_SREG_1_225V = 1225, /* output = 1.225V */
    PMIC_SREG_1_25V  = 1250, /* output = 1.25V  */
    PMIC_SREG_1_275V = 1275, /* output = 1.275V */
    PMIC_SREG_1_3V   = 1300, /* output = 1.3V   */
    PMIC_SREG_1_325V = 1325, /* output = 1.325V */
    PMIC_SREG_1_35V  = 1350, /* output = 1.35V  */
    PMIC_SREG_1_375V = 1375, /* output = 1.375V */
    PMIC_SREG_1_4V   = 1400, /* output = 1.4V   */
    PMIC_SREG_1_425V = 1425, /* output = 1.425V */
    PMIC_SREG_1_45V  = 1450, /* output = 1.45V  */
    PMIC_SREG_1_475V = 1475, /* output = 1.475V */
    PMIC_SREG_1_5V   = 1500, /* output = 1.5V   */
    PMIC_SREG_1_525V = 1525, /* output = 1.525V */
    PMIC_SREG_1_55V  = 1550, /* output = 1.55V  */
    PMIC_SREG_1_575V = 1575, /* output = 1.575V */
    PMIC_SREG_1_6V   = 1600, /* output = 1.6V   */
    PMIC_SREG_1_625V = 1625, /* output = 1.625V */
    PMIC_SREG_1_65V  = 1650, /* output = 1.65V  */
    PMIC_SREG_1_675V = 1675, /* output = 1.675V */
    PMIC_SREG_1_7V   = 1700, /* output = 1.7V   */
    PMIC_SREG_1_725V = 1725, /* output = 1.725V */
    PMIC_SREG_1_75V  = 1750, /* output = 1.75V  */
    PMIC_SREG_1_775V = 1775, /* output = 1.775V */
    PMIC_SREG_1_8V   = 1800, /* output = 1.8V   */
    PMIC_SREG_1_825V = 1825, /* output = 1.825V */
    PMIC_SREG_1_85V  = 1850, /* output = 1.85V  */
    PMIC_SREG_1_875V = 1875, /* output = 1.875V */
    PMIC_SREG_1_9V   = 1900, /* output = 1.9V   */
    PMIC_SREG_1_925V = 1925, /* output = 1.925V */
    PMIC_SREG_1_95V  = 1950, /* output = 1.95V  */
    PMIC_SREG_1_975V = 1975, /* output = 1.975V */
    PMIC_SREG_2_0V   = 2000, /* output = 2V     */
    PMIC_SREG_2_025V = 2025, /* output = 2.025V */
    PMIC_SREG_2_05V  = 2050, /* output = 2.05V  */
    PMIC_SREG_2_075V = 2075, /* output = 2.075V */
    PMIC_SREG_2_1V   = 2100, /* output = 2.1V   */
    PMIC_SREG_2_125V = 2125, /* output = 2.125V */
    PMIC_SREG_2_15V  = 2150, /* output = 2.15V  */
    PMIC_SREG_2_175V = 2175, /* output = 2.175V */
    PMIC_SREG_2_2V   = 2200, /* output = 2.2V   */
    PMIC_SREG_2_225V = 2225, /* output = 2.225V */
    PMIC_SREG_2_25V  = 2250, /* output = 2.25V  */
    PMIC_SREG_2_275V = 2275, /* output = 2.275V */
    PMIC_SREG_2_3V   = 2300, /* output = 2.3V   */
    PMIC_SREG_2_325V = 2325, /* output = 2.325V */
    PMIC_SREG_2_35V  = 2350, /* output = 2.35V  */
    PMIC_SREG_2_375V = 2375, /* output = 2.375V */
    PMIC_SREG_2_4V   = 2400, /* output = 2.4V   */
    PMIC_SREG_2_425V = 2425, /* output = 2.425V */
    PMIC_SREG_2_45V  = 2450, /* output = 2.45V  */
    PMIC_SREG_2_475V = 2475, /* output = 2.475V */
    PMIC_SREG_2_5V   = 2500, /* output = 2.5V   */
    PMIC_SREG_2_525V = 2525, /* output = 2.525V */
    PMIC_SREG_2_55V  = 2550, /* output = 2.55V  */
    PMIC_SREG_2_575V = 2575, /* output = 2.575V */
    PMIC_SREG_2_6V   = 2600, /* output = 2.6V   */
    PMIC_SREG_2_625V = 2625, /* output = 2.625V */
    PMIC_SREG_2_65V  = 2650, /* output = 2.65V  */
    PMIC_SREG_2_675V = 2675, /* output = 2.675V */
    PMIC_SREG_2_7V   = 2700, /* output = 2.7V   */
    PMIC_SREG_2_725V = 2725, /* output = 2.725V */
    PMIC_SREG_2_75V  = 2750, /* output = 2.75V  */
    PMIC_SREG_2_775V = 2775, /* output = 2.775V */
    PMIC_SREG_2_8V   = 2800, /* output = 2.8V   */
    PMIC_SREG_2_825V = 2825, /* output = 2.825V */
    PMIC_SREG_2_85V  = 2850, /* output = 2.85V  */
    PMIC_SREG_2_875V = 2875, /* output = 2.875V */
    PMIC_SREG_2_9V   = 2900, /* output = 2.9V   */
    PMIC_SREG_2_925V = 2925, /* output = 2.925V */
    PMIC_SREG_2_95V  = 2950, /* output = 2.95V  */
    PMIC_SREG_2_975V = 2975, /* output = 2.975V */
    PMIC_SREG_3_0V   = 3000, /* output = 3V     */
    PMIC_SREG_3_025V = 3025, /* output = 3.025V */
    PMIC_SREG_3_05V  = 3050, /* output = 3.05V  */
    PMIC_SREG_3_075V = 3075, /* output = 3.075V */
    PMIC_SREG_3_1V   = 3100, /* output = 3.1V   */
    PMIC_SREG_3_125V = 3125, /* output = 3.125V */
    PMIC_SREG_3_15V  = 3150, /* output = 3.15V  */
    PMIC_SREG_3_175V = 3175, /* output = 3.175V */
    PMIC_SREG_3_2V   = 3200, /* output = 3.2V   */
    PMIC_SREG_3_225V = 3225, /* output = 3.225V */
    PMIC_SREG_3_25V  = 3250, /* output = 3.25V  */
    PMIC_SREG_3_275V = 3275, /* output = 3.275V */
    PMIC_SREG_3_3V   = 3300, /* output = 3.3V   */
    PMIC_SREG_3_325V = 3325, /* output = 3.325V */
    PMIC_SREG_3_35V  = 3350, /* output = 3.35V  */
    PMIC_SREG_3_375V = 3375, /* output = 3.375V */
    PMIC_SREG_3_4V   = 3400, /* output = 3.4V   */
} PMIC_REGULATOR_SREG_VOLTAGE;

/*************************************
 * Switch regulator voltage settings type:
 *
 * SW_VOLTAGE_NORMAL
 * SW_VOLTAGE_DVS
 * SW_VOLTAGE_STBY
 *
 *************************************/
typedef enum _RR_REGULATOR_SREG_VOLTAGE_TYPE{
    SW_VOLTAGE_NORMAL=0,
    SW_VOLTAGE_DVS,
    SW_VOLTAGE_STBY,
 } RR_REGULATOR_SREG_VOLTAGE_TYPE;
typedef RR_REGULATOR_SREG_VOLTAGE_TYPE PMIC_REGULATOR_SREG_VOLTAGE_TYPE;


// standby input state H/L
typedef enum _WM8350_REGULATOR_SREG_STBY{
    LOW = 0,
    HIGH,
}WM8350_REGULATOR_SREG_STBY;
typedef WM8350_REGULATOR_SREG_STBY PMIC_REGULATOR_SREG_STBY;


/*************************************************
// switch regulator modes:
//              1. OFF
//              2. PWM mode and no Pulse Skipping
//              3. PWM mode and pulse Skipping Allowed
//              4. Low Power PFM mode
**************************************************/
typedef enum _WM8350_REGULATOR_SREG_MODE{
    SW_MODE_OFF,
    SW_MODE_PWM,
    SW_MODE_PULSESKIP,
    SW_MODE_PFM,
}WM8350_REGULATOR_SREG_MODE;
typedef WM8350_REGULATOR_SREG_MODE PMIC_REGULATOR_SREG_MODE;

/****************************************************************************
// LOW_POWER
// VxMODE=1, Set Low Power no matter of VxSTBY and STANDBY pin
//
// LOW_POWER_CTL_BY_PIN
// VxMODE=0, VxSTBY=1, Low Power Mode is contorled by STANDBY pin
//
// LOW_POWER_DISABLED
// VxMODE=0, VxSTBY=0, Low Power Mode is disabled
*****************************************************************************/
typedef enum _WM8350_REGULATOR_VREG_POWER_MODE{
    LOW_POWER_DISABLED = 0,
    LOW_POWER,
    LOW_POWER_CTRL_BY_PIN,
} WM8350_REGULATOR_VREG_POWER_MODE;
typedef WM8350_REGULATOR_VREG_POWER_MODE PMIC_REGULATOR_VREG_POWER_MODE;

typedef enum _WM8350_REGULATOR_VREG_VOLTAGE
{
    PMIC_VREG_0_9V   =  900, /* output = 0.9V   */
    PMIC_VREG_0_95V  =  950, /* output = 0.95V  */
    PMIC_VREG_1_0V   = 1000, /* output = 1V     */
    PMIC_VREG_1_05V  = 1050, /* output = 1.05V  */
    PMIC_VREG_1_1V   = 1100, /* output = 1.1V   */
    PMIC_VREG_1_15V  = 1150, /* output = 1.15V  */
    PMIC_VREG_1_2V   = 1200, /* output = 1.2V   */
    PMIC_VREG_1_25V  = 1250, /* output = 1.25V  */
    PMIC_VREG_1_3V   = 1300, /* output = 1.3V   */
    PMIC_VREG_1_35V  = 1350, /* output = 1.35V  */
    PMIC_VREG_1_4V   = 1400, /* output = 1.4V   */
    PMIC_VREG_1_45V  = 1450, /* output = 1.45V  */
    PMIC_VREG_1_5V   = 1500, /* output = 1.5V   */
    PMIC_VREG_1_55V  = 1550, /* output = 1.55V  */
    PMIC_VREG_1_6V   = 1600, /* output = 1.6V   */
    PMIC_VREG_1_65V  = 1650, /* output = 1.65V  */
    PMIC_VREG_1_8V   = 1800, /* output = 1.8V   */
    PMIC_VREG_1_9V   = 1900, /* output = 1.9V   */
    PMIC_VREG_2_0V   = 2000, /* output = 2V     */
    PMIC_VREG_2_1V   = 2100, /* output = 2.1V   */
    PMIC_VREG_2_2V   = 2200, /* output = 2.2V   */
    PMIC_VREG_2_3V   = 2300, /* output = 2.3V   */
    PMIC_VREG_2_4V   = 2400, /* output = 2.4V   */
    PMIC_VREG_2_5V   = 2500, /* output = 2.5V   */
    PMIC_VREG_2_6V   = 2600, /* output = 2.6V   */
    PMIC_VREG_2_7V   = 2700, /* output = 2.7V   */
    PMIC_VREG_2_8V   = 2800, /* output = 2.8V   */
    PMIC_VREG_2_9V   = 2900, /* output = 2.9V   */
    PMIC_VREG_3_0V   = 3000, /* output = 3V     */
    PMIC_VREG_3_1V   = 3100, /* output = 3.1V   */
    PMIC_VREG_3_2V   = 3200, /* output = 3.2V   */
    PMIC_VREG_3_3V   = 3300, /* output = 3.3V   */
} WM8350_REGULATOR_VREG_VOLTAGES;

typedef enum _WM8350_REGULATOR_VREG_VOLTAGE_VIOHI{
    VIOHI_2_775 = 2800,   //output  2.775V,
 } WM8350_REGULATOR_VREG_VOLTAGE_VIOHI;

typedef enum _WM8350_REGULATOR_VREG_VOLTAGE_VIOLO{
    VIOLO_1_20V = 1200,     //output  1.20V,
    VIOLO_1_30V = 1300,     //output  1.30V,
    VIOLO_1_50V = 1500,     //output  1.50V,
    VIOLO_1_80V = 1800,     //output  1.80V,
} WM8350_REGULATOR_VREG_VOLTAGE_VIOLO;

typedef enum _WM8350_REGULATOR_VREG_VOLTAGE_VRFDIG{
    VRFDIG_1_20V = 1200,    //output  1.20V,
    VRFDIG_1_50V = 1500,    //output  1.50V,
    VRFDIG_1_80V = 1800,    //output  1.80V,
    VRFDIG_1_875V = 1900,   //output  1.875V,
} WM8350_REGULATOR_VREG_VOLTAGE_VRFDIG;

typedef enum _WM8350_REGULATOR_VREG_VOLTAGE_VDIG{
    VDIG_1_20V = 1200,      //output  1.20V,
    VDIG_1_30V = 1300,      //output  1.30V,
    VDIG_1_50V = 1500,      //output  1.50V,
    VDIG_1_80V = 1800,      //output  1.80V,
} WM8350_REGULATOR_VREG_VOLTAGE_VDIG;

typedef enum _WM8350_REGULATOR_VREG_VOLTAGE_VGEN{
    VGEN_1_20V = 1200,      //output  1.20V,
    VGEN_1_30V = 1300,      //output  1.30V,
    VGEN_1_50V = 1500,      //output  1.50V,
    VGEN_1_80V = 1800,      //output  1.80V,
} WM8350_REGULATOR_VREG_VOLTAGE_VGEN;

typedef enum _WM8350_REGULATOR_VREG_VOLTAGE_VRF{
    VRF2_1_875V = 1900,     //output  1.875V,
    VRF2_2_475V = 2500,     //output  2.475V,
    VRF2_2_700V = 2700,     //output  2.700V,
    VRF2_2_775V = 2800,     //output  2.775V,
} WM8350_REGULATOR_VREG_VOLTAGE_VRF;

typedef enum _WM8350_REGULATOR_VREG_VOLTAGE_VRFCP{
    VRFCP_2_700V = 2700,    //output  2.700V,
    VRFCP_2_775V = 2800,    //output  2.775V,
} WM8350_REGULATOR_VREG_VOLTAGE_VRFCP;

typedef enum _WM8350_REGULATOR_VREG_VOLTAGE_VRFREF{
    VRFREF_2_475V = 2500,   //output  2.475V,
    VRFREF_2_600V = 2600,   //output  2.600V,
    VRFREF_2_700V = 2700,   //output  2.700V,
    VRFREF_2_775V = 2800,   //output  2.775V,
} WM8350_REGULATOR_VREG_VOLTAGE_VRFREF;

typedef enum _WM8350_REGULATOR_VREG_VOLTAGE_CAM{
                          //        1st silicon,  2nd silicon
    VCAM_1 = 1500,        //output   1.50V,          1.5V.
    VCAM_2 = 1800,        //output   1.80V,          1.80V
    VCAM_3 = 2500,        //output   2.50V,          2.50V
    VCAM_4 = 2600,        //output   2.80V,          2.55V
    VCAM_5 = 2600,        //output    -              2.60V
    VCAM_6 = 2800,        //output    -              2.80V
    VCAM_7 = 3000,        //output    -              3.00V
    VCAM_8 = 3300,        //output    -               TBD
} WM8350_REGULATOR_VREG_VOLTAGE_CAM;

typedef enum _WM8350_REGULATOR_VREG_VOLTAGE_SIM{
    VSIM_1_8V = 1800,   //output = 1.80V
    VSIM_2_9V = 2900,   //output = 2.90V
} WM8350_REGULATOR_VREG_VOLTAGE_SIM;

typedef enum _WM8350_REGULATOR_VREG_VOLTAGE_ESIM{
    VESIM_1_8V = 1800,   //output = 1.80V
    VESIM_2_9V = 2900,   //output = 2.90V
} WM8350_REGULATOR_VREG_VOLTAGE_ESIM;


typedef enum _WM8350_REGULATOR_VREG_VOLTAGE_MMC{
                    //        1st silicon,  2nd silicon
    VMMC_1 = 1600,  //output    1.60V,           1.60V
    VMMC_2 = 1800,  //output    1.80V,           1.80V
    VMMC_3 = 2000,  //output    2.00V,           2.00V
    VMMC_4 = 2600, //output     2.20V,           2.60V
    VMMC_5 = 2700,  //output    2.40V,           2.70V
    VMMC_6 = 2800,  //output    2.60V,           2.80V
    VMMC_7 = 2900,  //output    2.80V,           2.90V
    VMMC_8 = 3000,  //output    2.90V,           3.00V
} WM8350_REGULATOR_VREG_VOLTAGE_MMC;

typedef enum _WM8350_REGULATOR_VREG_VOLTAGE_VIB{
    V_VIB_1_3V = 1300,   //output = 1.30V
    V_VIB_1_8V = 1800,   //output = 1.80V
    V_VIB_2_0V = 2000,   //output = 2.0V
    V_VIB_3_0V = 3000,   //output = 3.0V
} WM8350_REGULATOR_VREG_VOLTAGE_VIB;

typedef union {
    PMIC_REGULATOR_VOLTAGE mV;
    WM8350_REGULATOR_VREG_VOLTAGES vreg;
    WM8350_REGULATOR_VREG_VOLTAGE_VIOHI viohi;
    WM8350_REGULATOR_VREG_VOLTAGE_VIOLO violo;
    WM8350_REGULATOR_VREG_VOLTAGE_VRFDIG vrfdig;
    WM8350_REGULATOR_VREG_VOLTAGE_VDIG vdig;
    WM8350_REGULATOR_VREG_VOLTAGE_VGEN vgen;
    WM8350_REGULATOR_VREG_VOLTAGE_VRF vrf;
    WM8350_REGULATOR_VREG_VOLTAGE_VRFCP vrfcp;
    WM8350_REGULATOR_VREG_VOLTAGE_VRFREF vrfref;
    WM8350_REGULATOR_VREG_VOLTAGE_CAM vcam;
    WM8350_REGULATOR_VREG_VOLTAGE_SIM vsim;
    WM8350_REGULATOR_VREG_VOLTAGE_ESIM vesim;
    WM8350_REGULATOR_VREG_VOLTAGE_MMC vmmc;
    WM8350_REGULATOR_VREG_VOLTAGE_VIB v_vib;
} WM8350_REGULATOR_VREG_VOLTAGE;
typedef WM8350_REGULATOR_VREG_VOLTAGE PMIC_REGULATOR_VREG_VOLTAGE;

typedef enum _WM8350_REGULATOR_ENABLE{
    DISABLE = 0,
    ENABLE = 1,
} WM8350_REGULATOR_ENABLE;


//------------------------------------------------------------------------------
// Functions

//switch mode regulator
PMIC_STATUS PmicSwitchModeRegulatorOn (PMIC_REGULATOR_SREG regulator);
PMIC_STATUS PmicSwitchModeRegulatorOff (PMIC_REGULATOR_SREG regulator);
PMIC_STATUS PmicSwitchModeRegulatorSetVoltageLevel (PMIC_REGULATOR_SREG regulator, PMIC_REGULATOR_SREG_VOLTAGE_TYPE voltageType, PMIC_REGULATOR_SREG_VOLTAGE voltage);
PMIC_STATUS PmicSwitchModeRegulatorGetVoltageLevel (PMIC_REGULATOR_SREG regulator, PMIC_REGULATOR_SREG_VOLTAGE_TYPE voltageType, PMIC_REGULATOR_SREG_VOLTAGE* voltage);
PMIC_STATUS PmicSwitchModeRegulatorSetMode (PMIC_REGULATOR_SREG regulator,PMIC_REGULATOR_SREG_STBY standby,PMIC_REGULATOR_SREG_MODE mode );
PMIC_STATUS PmicSwitchModeRegulatorGetMode (PMIC_REGULATOR_SREG regulator, PMIC_REGULATOR_SREG_STBY standby,PMIC_REGULATOR_SREG_MODE* mode );
PMIC_STATUS PmicSwitchModeRegulatorEnableSTBYDVFS (PMIC_REGULATOR_SREG regulator);
PMIC_STATUS PmicSwitchModeRegulatorDisableSTBYDVFS (PMIC_REGULATOR_SREG regulator);
PMIC_STATUS PmicSwitchModeRegulatorSetDVSSpeed (PMIC_REGULATOR_SREG regulator, UINT8 dvsspeed);
PMIC_STATUS PmicSwitchModeRegulatorEnablePanicMode(PMIC_REGULATOR_SREG regulator);
PMIC_STATUS PmicSwitchModeRegulatorDisablePanicMode(PMIC_REGULATOR_SREG regulator);
PMIC_STATUS PmicSwitchModeRegulatorEnableSoftStart(PMIC_REGULATOR_SREG regulator);
PMIC_STATUS PmicSwitchModeRegulatorDisableSoftStart(PMIC_REGULATOR_SREG regulator);

// linear voltage regulator
PMIC_STATUS PmicVoltageRegulatorOn (PMIC_REGULATOR_VREG regulator);
PMIC_STATUS PmicVoltageRegulatorOff (PMIC_REGULATOR_VREG regulator);
PMIC_STATUS PmicVoltageRegulatorSetVoltageLevel (PMIC_REGULATOR_VREG regulator, PMIC_REGULATOR_VREG_VOLTAGE voltage);
PMIC_STATUS PmicVoltageRegulatorGetVoltageLevel (PMIC_REGULATOR_VREG regulator, PMIC_REGULATOR_VREG_VOLTAGE* voltage);
PMIC_STATUS PmicVoltageRegulatorSetPowerMode (PMIC_REGULATOR_VREG regulator, PMIC_REGULATOR_VREG_POWER_MODE powerMode);
PMIC_STATUS PmicVoltageRegulatorGetPowerMode (PMIC_REGULATOR_VREG regulator, PMIC_REGULATOR_VREG_POWER_MODE* powerMode);


#ifdef __cplusplus
}
#endif

#endif // __PMIC_REGULATOR_H__
