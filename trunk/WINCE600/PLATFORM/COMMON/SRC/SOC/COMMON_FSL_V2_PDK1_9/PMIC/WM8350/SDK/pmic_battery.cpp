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
/// @file   pmic_battery.cpp
/// @brief  Platform-specific WM8350 PMIC functions.
///
/// This file contains the PMIC platform-specific functions that provide control
/// over the Power Management IC.
///
/// @version $Id: pmic_battery.cpp 562 2007-05-18 21:05:20Z fb $
///
/// @Warning
///   This software is specifically written for Wolfson devices. It may not be
///   used with other devices.
///
//-----------------------------------------------------------------------------
//
//  File:  pmic_battery.cpp
//
//  This file contains the PMIC Battery SDK interface that is used by
//  applications and other drivers to access registers of the PMIC (MC13783).
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4201)   // Non-standard extension
#include <windows.h>
#pragma warning(pop)
#include <Devload.h>
#include "common_macros.h"
#include "pmic_battery.h"
#include "pmic_lla.h"

//-----------------------------------------------------------------------------
// External Functions

//-----------------------------------------------------------------------------
// External Variables

//-----------------------------------------------------------------------------
// Defines

//-----------------------------------------------------------------------------
// Types

//-----------------------------------------------------------------------------
// Global Variables

extern "C" HANDLE               hPMI;           // Our global handle to the PMIC driver
extern "C" WM_DEVICE_HANDLE     g_hWMDevice;    // Our global Wolfson device handle

#ifdef DEBUG

// Debug zone bit positions
#define ZONEID_ERROR           0
#define ZONEID_WARN            1
#define ZONEID_INIT            2
#define ZONEID_FUNC            3
#define ZONEID_INFO            4

// Debug zone masks
#define ZONEMASK_ERROR         (1 << ZONEID_ERROR)
#define ZONEMASK_WARN          (1 << ZONEID_WARN)
#define ZONEMASK_INIT          (1 << ZONEID_INIT)
#define ZONEMASK_FUNC          (1 << ZONEID_FUNC)
#define ZONEMASK_INFO          (1 << ZONEID_INFO)

// Debug zone args to DEBUGMSG
#define ZONE_ERROR             DEBUGZONE(ZONEID_ERROR)
#define ZONE_WARN              DEBUGZONE(ZONEID_WARN)
#define ZONE_INIT              DEBUGZONE(ZONEID_INIT)
#define ZONE_FUNC              DEBUGZONE(ZONEID_FUNC)
#define ZONE_INFO              DEBUGZONE(ZONEID_INFO)

extern DBGPARAM dpCurSettings;

#endif

//-----------------------------------------------------------------------------
// Local Variables

//-----------------------------------------------------------------------------
// Local Functions

//------------------------------------------------------------------------------
//
// Function: PmicBatterySetChargeVoltage
//
// This function programs the output voltage of charge regulator
//
// Parameters:
//      chargevoltagelevel [IN] voltage level
//      level 0 = 4.05V
//            1 = 4.10V
//            2 = 4.15V
//            3 = 4.20V
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatterySetChargeVoltage(UINT8 chargevoltagelevel)
{
    PMIC_STATUS retval;
    WM_STATUS status;
    WM_BATT_VOLTAGE voltage;

    switch ( chargevoltagelevel )
    {
        case 0:
            voltage = WM_BATT_VOLTAGE_4_05V;
            break;
        case 1:
            voltage = WM_BATT_VOLTAGE_4_1V;
            break;
        case 2:
            voltage = WM_BATT_VOLTAGE_4_15V;
            break;
        case 3:
            voltage = WM_BATT_VOLTAGE_4_2V;
            break;
        default:
            retval = PMIC_PARAMETER_ERROR;
            goto error;
    }
    status = WMPmicChargerSetBattVoltage( g_hWMDevice, voltage );

    retval = WMStatusToPmicStatus( status );

    if( WMS_SUCCESS != status )
        goto error;

    return PMIC_SUCCESS;

error:
    return retval;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryGetChargeVoltage
//
// This function returns the output voltage of charge regulator
//
// Parameters:
//      chargevoltagelevel [OUT] pointer to voltage level
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryGetChargeVoltage(UINT8* chargevoltagelevel)
{
    PMIC_STATUS retval;
    WM_STATUS status;
    WM_BATT_VOLTAGE voltage;

    status = WMPmicChargerGetBattVoltage( g_hWMDevice, &voltage );

    retval = WMStatusToPmicStatus( status );

    if ( WMS_SUCCESS != status )
        goto error;

    switch ( voltage )
    {
    case WM_BATT_VOLTAGE_4_05V:
            *chargevoltagelevel = 0;
            break;
        case WM_BATT_VOLTAGE_4_1V:
            *chargevoltagelevel = 1;
            break;
        case WM_BATT_VOLTAGE_4_15V:
            *chargevoltagelevel = 2;
            break;
        case WM_BATT_VOLTAGE_4_2V:
            *chargevoltagelevel = 3;
            break;
        default:
            retval = PMIC_PARAMETER_ERROR;
            goto error;
    }

    return PMIC_SUCCESS;

error:
    return retval;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatterySetChargeCurrent
//
// This function programs the charge current in the constant-current phase to
// the main battery
//
// Parameters:
//      chargecurrentlevel [IN] current level
//      level 0 = OFF
//            1 = 1 x Imax/15
//            2 = 2 x Imax/15
//              ...        (step size is Imax/15)
//            15 = 15 x Imax/15
//
//            Note: Imax = 750Ma
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatterySetChargeCurrent (UINT8 chargecurrentlevel)
{
    PMIC_STATUS retval;
    WM_STATUS status;
    int current = chargecurrentlevel * ( 750 / 15 );

    if ( 0 == chargecurrentlevel )
    {
        status = WMPmicChargerEnableFastCharge( g_hWMDevice, FALSE, current );

        retval = WMStatusToPmicStatus( status );
    }
    else
    {
        status = WMPmicChargerEnableFastCharge( g_hWMDevice, TRUE, current );

        retval = WMStatusToPmicStatus( status );
    }

    return retval;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryGetChargeCurrent
//
// This function returns the charge current setting of the main battery
//
// Parameters:
//      chargecurrentlevel [OUT] pointer to current level
//      level 0 = OFF
//            1 = 1 x Imax/15
//            2 = 2 x Imax/15
//              ...        (step size is Imax/15)
//            15 = 15 x Imax/15
//
//            Note: Imax = 750Ma
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryGetChargeCurrent (UINT8* chargecurrentlevel)
{
    PMIC_STATUS retval;
    WM_STATUS status;
    BOOL enabled;
    int current;

    status = WMPmicChargerGetFastChargeConfig( g_hWMDevice, &enabled, &current );

    retval = WMStatusToPmicStatus( status );

    if( WMS_SUCCESS != status )
        goto error;

    *chargecurrentlevel = (UINT8)(( current * 15 ) / 750);

    return PMIC_SUCCESS;

error:
    return retval;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatterySetTrickleCurrent
//
// This function programs the current of the trickle charger
//
// Parameters:
//      tricklecurrentlevel [IN] trickle current level
//      level 0 = 50 mA
//            1 = 100 mA
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatterySetTrickleCurrent(UINT8 tricklecurrentlevel)
{
    PMIC_STATUS retval;
    WM_STATUS status;
    WM_TRICKLE_CONFIG     trickleConfig;
    WM_TRICKLE_CURRENT   trickleCurrent;

    trickleConfig.threshold = BSP_PMIC_CHARGE_VOLTAGE_THRESHOLD;
    trickleConfig.tempChoke = BSP_PMIC_TRICKLE_TEMP_CHOKE;

    switch( tricklecurrentlevel )
    {
        case 0:
            trickleCurrent = WM_TRICKLE_CURRENT_50mA;
            break;
        case 1:
            trickleCurrent = WM_TRICKLE_CURRENT_100mA;
            break;
        default:
            retval = PMIC_PARAMETER_ERROR;
            goto error;
    }

    status = WMPmicChargerGetTrickleConfig( g_hWMDevice, &trickleConfig );

    retval = WMStatusToPmicStatus( status );

    if( WMS_SUCCESS != status )
        goto error;

    trickleConfig.current = trickleCurrent;

    status = WMPmicChargerSetTrickleConfig( g_hWMDevice, trickleConfig );

    retval = WMStatusToPmicStatus( status );

    if( WMS_SUCCESS != status )
        goto error;

    return PMIC_SUCCESS;

error:
    return retval;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryGetTrickleCurrent
//
// This function returns the current of the trickle charger
//
// Parameters:
//      tricklecurrentlevel [OUT] pointer to trickle current level
//
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryGetTrickleCurrent (UINT8* tricklecurrentlevel)
{
    PMIC_STATUS retval;
    WM_STATUS status;
    WM_TRICKLE_CONFIG    trickleConfig;


    status = WMPmicChargerGetTrickleConfig( g_hWMDevice, &trickleConfig );

    retval = WMStatusToPmicStatus( status );

    if( WMS_SUCCESS != status )
        goto error;

    switch( trickleConfig.current )
    {
        case WM_TRICKLE_CURRENT_50mA:
            *tricklecurrentlevel = 0;
            break;
        case WM_TRICKLE_CURRENT_100mA:
            *tricklecurrentlevel = 1;
            break;
        default:
            retval = PMIC_PARAMETER_ERROR;
            goto error;
    }

    return PMIC_SUCCESS;

error:
    return retval;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryFETControl
//
// This function programs the control mode and setting of BPFET and FETOVRD
// BATTFET and BPFET to be controlled by FETCTRL bit or hardware
//
// Parameters:
//      fetcontrol [IN] BPFET and FETOVRD control mode and setting
//
//      input = 0 (BATTFET and BPFET outputs are controlled by hardware)
//            = 1 (BATTFET and BPFET outputs are controlled by hardware)
//            = 2 (BATTFET low and BATTFET high, controlled by FETCTRL)
//            = 3 (BATTFET high and BATTFET low, controlled by FETCTRL)
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryFETControl(UINT8 fetcontrol)
{
    UNREFERENCED_PARAMETER( fetcontrol );

    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryReverseDisable
//
// This function disables the reverse mode
//
// Parameters:
//      None.
//
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryReverseDisable()
{
    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryReverseEnable
//
// This function enables the reverse mode
//
// Parameters:
//      None.
//
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryReverseEnable()
{
    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatterySetOvervoltageThreshold
//
// This function programs the overvoltage threshold value
//
// Parameters:
//      ovthresholdlevel [IN] overvoltage threshold level
//      High to low, Low to High  (5.35V)
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatterySetOvervoltageThreshold(UINT8 ovthresholdlevel)
{
    UNREFERENCED_PARAMETER( ovthresholdlevel );

    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryGetOvervoltageThreshold
//
// This function returns the overvoltage threshold value
//
// Parameters:
//      ovthresholdlevel [OUT] pointer to overvoltage threshold level
//
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryGetOvervoltageThreshold (UINT8* ovthresholdlevel)
{
    UNREFERENCED_PARAMETER( ovthresholdlevel );

    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryUnregulatedChargeDisable
//
// This function disables the unregulated charge path. The voltage and current
// limits will be controlled by the charge path regulator.
//
// Parameters:
//      None.
//
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryUnregulatedChargeDisable()
{
    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryUnregulatedChargeEnable
//
// This function enables the unregulated charge path. The settings of the charge
// path regulator (voltage and current limits) will be overruled.
//
// Parameters:
//      None.
//
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryUnregulatedChargeEnable()
{

    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryChargeLedDisable
//
// This function disables the charging LED
//
// Parameters:
//      None.
//
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryChargeLedDisable()
{
    PMIC_STATUS retval;
    WM_STATUS status;
    WM_GPIO gpio = WM_GPIO_10;

    //TBD - Create entry in bsp_cfg.h and function to get the value here.

    status = WMPmicChargerConfigStatusLED( g_hWMDevice, FALSE, gpio );

    retval = WMStatusToPmicStatus( status );

    return retval;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryChargeLedEnable
//
// This function enables the charging LED
//
// Parameters:
//      None.
//
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryChargeLedEnable()
{
    PMIC_STATUS retval;
    WM_STATUS status;
    WM_GPIO gpio = BSP_PMIC_CH_IND;

    status = WMPmicChargerConfigStatusLED( g_hWMDevice, TRUE, gpio );

    retval = WMStatusToPmicStatus( status );

    return retval;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryEnablePulldown
//
// This function enables the 5k pulldown resistor used in the dual path charging
//
// Parameters:
//      None.
//
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryEnablePulldown()
{
    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryDisablePulldown
//
// This function disables the 5k pulldown resistor used in dual path charging
//
// Parameters:
//      None.
//
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryDisablePulldown()
{
    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryEnableCoincellCharger
//
// This function enables the coincell charger
//
// Parameters:
//      None.
//
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryEnableCoincellCharger()
{
    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}

//-----------------------------------------------------------------------------
//
// Function: PmicBatteryDisableCoincellCharger
//
// This function disables the coincell charger
//
// Parameters:
//      None.
//
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryDisableCoincellCharger()
{
    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatterySetCoincellVoltage
//
// This function programs the output voltage level of coincell charger
//
// Parameters:
//      votlagelevel [IN] voltage level
//      level 0 = 2.7V
//            1 = 2.8V
//            2 = 2.9V
//            ... (in 100mV increment)
//            6 = 3.3V
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatterySetCoincellVoltage (UINT8 coincellvoltagelevel)
{
    UNREFERENCED_PARAMETER( coincellvoltagelevel );

    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryGetCoincellVoltage
//
// This function returns the output voltage level of coincell charger
//
// Parameters:
//      voltagelevel [OUT] pointer to voltage level
//
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryGetCoincellVoltage (UINT8* coincellvoltagelevel)
{
    UNREFERENCED_PARAMETER( coincellvoltagelevel );

    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryEnableEolComparator
//
// This function enables the end-of-life function instead of the LOBAT
//
// Parameters:
//      None.
//
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryEnableEolComparator()
{
    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryDisableEolComparator
//
// This function disables the end-of-life comparator function
//
// Parameters:
//      None.
//
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryDisableEolComparator()
{
    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
// Function: PmicBatteryGetChargerMode
//
//      This function returns the charger mode (ie. Dual Path, Single Path,
//  Serial Path, Dual Input Single Path and Dual Input Serial Path).
//
//      The way the bits are mapped to the actual pin definitions is:
//              LLOW --> GND
//              OPEN --> HI Z
//              HHIGH--> VPMIC
//
// Parameters:
//       mode [OUT] pointer to charger mode.
//
// Returns:
//      PMIC_STATUS.
//
// Remarks:
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryGetChargerMode(CHARGER_MODE *mode)
{
    UNREFERENCED_PARAMETER( mode );

    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}

#ifdef BSP_PMIC_WM8350
//------------------------------------------------------------------------------
//
// Function: PmicBatteryPauseCharging
//
// This function pauses or restarts charging of the main battery.
//
// Parameters:
//      pause [IN]         TRUE pauses charging FALSE restarts charging
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryPauseCharging(BOOL pause)
{
    PMIC_STATUS retval;
    WM_STATUS status;

    status = WMPmicChargerPause( g_hWMDevice, pause );

    retval = WMStatusToPmicStatus( status );

    return retval;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatterySetEOCChargeCurrent
//
// This function programs the end of charge current setting to the main battery.
//
// Parameters:
//      chargecurrentlevel [IN] current level
//      level 0 = 20 mA    (max value)
//            1 = 30 mA    (max value)
//              ...        (in increment of 10 mA)
//            6 = 80 mA    (max value)
//            7 = 90 mA    (max value)
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatterySetEOCChargeCurrent(UINT8 chargecurrentlevel)
{
    PMIC_STATUS retval;
    WM_STATUS status;
    int current = ( chargecurrentlevel * 10 ) + 20;

    status = WMPmicChargerSetEOCCurrent( g_hWMDevice, current );

    retval = WMStatusToPmicStatus( status );

    return retval;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryGetEOCChargeCurrent
//
// This function returns the end of charge current setting of the main battery
//
// Parameters:
//      chargecurrentlevel [OUT] pointer to current level
//      level 0 = 20 mA    (max value)
//            1 = 30 mA    (max value)
//              ...        (in increment of 10 mA)
//            6 = 80 mA    (max value)
//            7 = 90 mA    (max value)
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryGetEOCChargeCurrent(UINT8* chargecurrentlevel)
{
    PMIC_STATUS retval;
    WM_STATUS status;
    int current;

    status = WMPmicChargerGetEOCCurrent( g_hWMDevice, &current );

    retval = WMStatusToPmicStatus( status );

    if( WMS_SUCCESS != status )
        goto error;

    *chargecurrentlevel = UINT8( ( current - 20 ) / 10 );

    return PMIC_SUCCESS;

error:
    return retval;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryGetChargerStatus
//
// This function returns the current status of the charger
//
// Parameters:
//      status [OUT] pointer to charger status
//      level 0 = Zero current
//            1 = Trickle charge
//            2 = Fast charge
//
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryGetChargerStatus(UINT8 *chargeStatus)
{
    PMIC_STATUS retval;
    WM_STATUS status;
    WM_CHARGER_STATUS chargeStat;

    status = WMPmicChargerGetStatus( g_hWMDevice, &chargeStat );

    retval = WMStatusToPmicStatus( status );

    if( WMS_SUCCESS != status )
        goto error;

    switch( chargeStat )
    {
        case WM_CHARGER_STATUS_CURRENT_ZERO:
            *chargeStatus = 0;
            break;
        case WM_CHARGER_STATUS_TRICKLE:
            *chargeStatus = 1;
            break;
        case WM_CHARGER_STATUS_FAST:
            *chargeStatus = 2;
            break;
        default:
            retval = PMIC_PARAMETER_ERROR;
            goto error;
               break;
    }

    return PMIC_SUCCESS;

error:
    return retval;

}

//------------------------------------------------------------------------------
//
// Function: PmicBatterySetTimer
//
// This function programs the charging timer for the main battery.
//
// Parameters:
//      chargecurrentlevel [IN] current level
//      level 0 = 30 mins
//            1 = 45 mins
//              ...        (in increment of 15 mins)
//            15 = 255 mins
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatterySetTimer(UINT8 timeout)
{
    PMIC_STATUS retval;
    WM_STATUS status;
    UINT8 mins = ( timeout * 15 ) + 30;

    status = WMPmicChargerSetTimer( g_hWMDevice, mins );

    retval = WMStatusToPmicStatus( status );

    return retval;
}
#endif // BSP_PMIC_WM8350

//------------------------------------------------------------------------------
// Function: PmicBatteryEnableAdChannel5
//
// This function enables use of AD channel 5 to read charge current
//
// Parameters:
//      None.
//
// Returns:
//      PMIC_STATUS.
//
// Remarks:
//      This function is only applicable to Roadrunner, it is a stub function
//      in MC13783
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryEnableAdChannel5()
{
    return PMIC_SUCCESS;
}

//-----------------------------------------------------------------------------
//
// Function: PmicBatteryDisableAdChannel5
//
// This function disables use of AD channel 5 to read charge current
//
// Parameters:
//      None.
//
// Returns:
//      PMIC_STATUS.
//
// Remarks:
//      This function is only applicable to Roadrunner, it is a stub function
//      in MC13783
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryDisableAdChannel5()
{
    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatterySetCoincellCurrentlimit
//
// This function limits the output current level of coincell charger
//
// Parameters:
//      coincellcurrentlevel [IN] coincell current level
//
// Returns:
//      PMIC_STATUS.
//
// Remarks:
//      This function is only applicable to Roadrunner, it is a stub function
//      in MC13783
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatterySetCoincellCurrentlimit (UINT8 coincellcurrentlevel)
{
    UNREFERENCED_PARAMETER( coincellcurrentlevel );

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryGetCoincellCurrentlimit
//
// This function returns the output current limit of coincell charger
//
// Parameters:
//      coincellcurrentlevel [OUT] pointer to coincell current level
//
//
// Returns:
//      PMIC_STATUS.
//
// Remarks:
//      This function is only applicable to Roadrunner, it is a stub function
//      in MC13783
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryGetCoincellCurrentlimit (UINT8* coincellcurrentlevel)
{
    UNREFERENCED_PARAMETER( coincellcurrentlevel );

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatterySetEolTrip
//
// This function sets the end-of-life threshold
//
// Parameters:
//      eoltriplevel [IN] eol trip level
//
// Returns:
//      PMIC_STATUS.
//
// Remarks:
//      This function is only applicable to Roadrunner, it is a stub function
//      in MC13783
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatterySetEolTrip (UINT8 eoltriplevel)
{
    UNREFERENCED_PARAMETER( eoltriplevel );

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatteryGetEolTrip
//
// This function returns the end-of-life threshold
//
// Parameters:
//      eoltriplevel [OUT] pointer to eol trip level
//
// Returns:
//      PMIC_STATUS.
//
// Remarks:
//      This function is only applicable to Roadrunner, it is a stub function
//      in MC13783
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatteryGetEolTrip (UINT8* eoltriplevel)
{
    UNREFERENCED_PARAMETER( eoltriplevel );

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatterEnableCharger
//
// This function is used to start charging a battery. For different charger,
// different voltage and current range are supported.
//
// Parameters:
//      chgr
//          [in]  Charger as defined in BATT_CHARGER.
//      c_voltage
//          [in]  Charging voltage.
//      c_current
//          [in]  Charging current.
//
// Returns:
//      This function returns PMIC_SUCCESS if successful.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatterEnableCharger(BATT_CHARGER chgr, UINT8 c_voltage,
                                        UINT8 c_current)
{
    PMIC_STATUS retval;
    WM_STATUS status;

    switch (chgr)
    {
    case BATT_MAIN_CHGR:
        status = WMPmicChargerEnable( g_hWMDevice );

        retval = WMStatusToPmicStatus( status );

        if( WMS_SUCCESS != status )
            goto error;

        retval = PmicBatterySetChargeVoltage(c_voltage);
        if( PMIC_SUCCESS != retval )
            goto error;

        retval = PmicBatterySetChargeCurrent (c_current);
        if( PMIC_SUCCESS != retval )
            goto error;

        break;

    case BATT_CELL_CHGR:
        retval = PMIC_NOT_SUPPORTED;
        goto error;
        break;

    case BATT_TRCKLE_CHGR:
        status = WMPmicChargerEnable( g_hWMDevice );

        retval = WMStatusToPmicStatus( status );

        if( WMS_SUCCESS != status )
            goto error;

        retval = PmicBatterySetTrickleCurrent(c_current);
        if( PMIC_SUCCESS != retval )
            goto error;
        break;

    default:
        retval = PMIC_PARAMETER_ERROR;
        goto error;
    }

    return PMIC_SUCCESS;

error:
    return retval;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatterDisableCharger
//
// This function turns off the selected charger.
//
// Parameters:
//      chgr
//          [in]  Charger as defined in BATT_CHARGER.
//
// Returns:
//      This function returns PMIC_SUCCESS if successful.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatterDisableCharger(BATT_CHARGER chgr)
{
    PMIC_STATUS retval;
    WM_STATUS status;

    switch (chgr)
    {
    case BATT_MAIN_CHGR:
    case BATT_TRCKLE_CHGR:
        status = WMPmicChargerDisable( g_hWMDevice );

        retval = WMStatusToPmicStatus( status );

        if( WMS_SUCCESS != status )
            goto error;
        break;

    case BATT_CELL_CHGR:
        retval = PMIC_NOT_SUPPORTED;
        goto error;

    default:
        return PMIC_PARAMETER_ERROR;
        goto error;
    }

    return PMIC_SUCCESS;

error:

    return retval;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatterSetCharger
//
// This function is used to change the charger setting.
//
// Parameters:
//      chgr
//          [in]  Charger as defined in BATT_CHARGER.
//      c_voltage
//          [in]  Charging voltage.
//      c_current
//          [in]  Charging current.
//
// Returns:
//      This function returns PMIC_SUCCESS if successful.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatterSetCharger(BATT_CHARGER chgr, UINT8 c_voltage,
                                        UINT8 c_current)
{
    PMIC_STATUS retval;

    switch (chgr)
    {
    case BATT_MAIN_CHGR:
        retval = PmicBatterySetChargeVoltage(c_voltage);
        if( PMIC_SUCCESS != retval )
            goto error;

        retval = PmicBatterySetChargeCurrent (c_current);
        if( PMIC_SUCCESS != retval )
            goto error;

        break;

    case BATT_CELL_CHGR:
        retval = PMIC_NOT_SUPPORTED;
        goto error;
        break;

    case BATT_TRCKLE_CHGR:
        retval = PmicBatterySetTrickleCurrent(c_current);
        if( PMIC_SUCCESS != retval )
            goto error;

        break;

    default:
        return PMIC_PARAMETER_ERROR;
        break;
    }

    return PMIC_SUCCESS;

error:
    return retval;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatterGetChargerSetting
//
// This function is used to retrive the charger setting.
//
// Parameters:
//      chgr
//          [in]  Charger as defined in BATT_CHARGER.
//      c_voltage
//          [out] a pointer to what the charging voltage is set to.
//      c_current
//          [out] a pointer to what the charging current is set to.
//
// Returns:
//      This function returns PMIC_SUCCESS if successful.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatterGetChargerSetting(BATT_CHARGER chgr, UINT8* c_voltage,
                                        UINT8* c_current)
{
    PMIC_STATUS retval;

    switch (chgr)
    {
    case BATT_MAIN_CHGR:
        retval = PmicBatteryGetChargeVoltage(c_voltage);
        if( PMIC_SUCCESS != retval )
            goto error;

        retval = PmicBatteryGetChargeCurrent (c_current);
        if( PMIC_SUCCESS != retval )
            goto error;

        break;

    case BATT_CELL_CHGR:
        retval = PMIC_NOT_SUPPORTED;
        goto error;

        break;

    case BATT_TRCKLE_CHGR:
        retval = PmicBatteryGetTrickleCurrent(c_current);
        if( PMIC_SUCCESS != retval )
            goto error;

        break;

    default:
        retval = PMIC_PARAMETER_ERROR;
        goto error;

        break;
    }

    return PMIC_SUCCESS;

error:

    return retval;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatterGetChargeCurrent
//
// This function retrives the main charger current.
//
// Parameters:
//      c_current
//          [out] a pointer to what the charging current measures
//
// Returns:
//      This function returns PMIC_SUCCESS if successful.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatterGetChargeCurrent(UINT16* c_current)
{
    UNREFERENCED_PARAMETER( c_current );

    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
//
// Function: PmicBatterEnableEol
//
// This function enables End-of-Life comparator.
//
// Parameters:  -- none
//
// Returns:
//      This function returns PMIC_SUCCESS if successful.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatterEnableEol(void)
{
    return PmicBatteryEnableEolComparator();
}

//------------------------------------------------------------------------------
//
// Function: PmicBatterDisableEol
//
// This function disables End-of-Life comparator.
//
// Parameters:  -- none
//
// Returns:
//      This function returns PMIC_SUCCESS if successful.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatterDisableEol(void)
{
    return PmicBatteryDisableEolComparator();
}

//------------------------------------------------------------------------------
//
// Function: PmicBatterLedControl
//
// This function controls charge LED.
//
// Parameters:
//      on
//          [in]  If on is ture, LED will be turned on, or otherwise the
//                LED will be turned off.
//
// Returns:
//      This function returns PMIC_SUCCESS if successful.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatterLedControl(BOOL on)
{
    if (on)
        return PmicBatteryChargeLedEnable();
    else
        return PmicBatteryChargeLedDisable();
}

//------------------------------------------------------------------------------
//
// Function: PmicBatterSetReverseSupply
//
// This function sets reverse supply mode.
//
// Parameters:
//      enable
//          [i]]  If enable is ture, reverse supply mode is enable or otherwise
//                the reverse supply mode is disabled.
//
// Returns:
//      This function returns PMIC_SUCCESS if successful.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatterSetReverseSupply(BOOL enable)
{
    if(enable)
        return PmicBatteryReverseEnable();
    else
        return PmicBatteryReverseDisable();
}

//------------------------------------------------------------------------------
//
// Function: PmicBatterSetUnregulated
//
// This function sets unregulatored charging mode on main battery.
//
// Parameters:
//      enable
//          [in]  If enable is ture, unregulated charging mode is enabled
//                otherwise it is disabled.
//
// Returns:
//      This function returns PMIC_SUCCESS if successful.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBatterSetUnregulated(BOOL enable)
{
    if(enable)
        return PmicBatteryUnregulatedChargeEnable();
    else
        return PmicBatteryUnregulatedChargeDisable();
}
