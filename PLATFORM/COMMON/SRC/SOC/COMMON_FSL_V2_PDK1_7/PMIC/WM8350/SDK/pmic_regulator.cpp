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
/// @file   pmicpdk.c
/// @brief  Platform-specific WM8350 PMIC functions.
///
/// This file contains the PMIC platform-specific functions that provide control
/// over the Power Management IC.
///
/// @version $Id: pmic_regulator.cpp 660 2007-06-19 16:13:29Z ib $
///
/// @Warning
///   This software is specifically written for Wolfson devices. It may not be
///   used with other devices.
///
//-----------------------------------------------------------------------------
//
//  File:  pmic_regulator.cpp
//
//  This file contains the PMIC regulator SDK interface that is used by applications
//  and other drivers to access registers of the PMIC.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4201)   // Non-standard extension
#include <windows.h>
#pragma warning(pop)
#include "common_macros.h"
#include "pmic_ioctl.h"
#include "pmic_lla.h"
#include "pmic_regulator.h"
#include "WMPmic.h"

//-----------------------------------------------------------------------------
// External Functions

//-----------------------------------------------------------------------------
// External Variables

//-----------------------------------------------------------------------------
// Defines

//
// Works out the number of elements in an array.
//
#define ARRAY_SIZE(_a)  (sizeof(_a)/sizeof(_a[0]))

//-----------------------------------------------------------------------------
// Types

//-----------------------------------------------------------------------------
// Global Variables
extern "C" HANDLE               hPMI;           // Our global handle to the PMIC driver
extern "C" WM_DEVICE_HANDLE     g_hWMDevice;    // Our global Wolfson device handle

#ifdef DEBUG

// Debug zone bit positions
#define ZONEID_ERROR            0
#define ZONEID_WARN             1
#define ZONEID_INIT             2
#define ZONEID_FUNC             3
#define ZONEID_INFO             4

// Debug zone masks
#define ZONEMASK_ERROR      (1 << ZONEID_ERROR)
#define ZONEMASK_WARN           (1 << ZONEID_WARN)
#define ZONEMASK_INIT           (1 << ZONEID_INIT)
#define ZONEMASK_FUNC       (1 << ZONEID_FUNC)
#define ZONEMASK_INFO           (1 << ZONEID_INFO)

// Debug zone args to DEBUGMSG
#define ZONE_ERROR              DEBUGZONE(ZONEID_ERROR)
#define ZONE_WARN               DEBUGZONE(ZONEID_WARN)
#define ZONE_INIT               DEBUGZONE(ZONEID_INIT)
#define ZONE_FUNC               DEBUGZONE(ZONEID_FUNC)
#define ZONE_INFO               DEBUGZONE(ZONEID_INFO)

extern DBGPARAM dpCurSettings;

#endif  // DEBUG

//-----------------------------------------------------------------------------
// Local Variables

#define UNKNOWN_REGULATOR   ((WM_REGULATOR) 0)

//
// The SREG -> WM_REGULATOR translation table
//
static WM_REGULATOR s_SREGS[] =
{
    WM_DCDC1,
    WM_DCDC2,
    WM_DCDC3,
    WM_DCDC4,
    WM_DCDC5,
    WM_DCDC6
};
#define TRANSLATE_SREG(_reg) (((_reg) < ARRAY_SIZE(s_SREGS))?s_SREGS[_reg]:UNKNOWN_REGULATOR)

//
// The VREG -> WM_REGULATOR translation table
//
static WM_REGULATOR s_VREGS[] =
{
    WM_LDO1,
    WM_LDO2,
    WM_LDO3,
    WM_LDO4,
};
#define TRANSLATE_VREG(_reg) (((_reg-LDO1) < ARRAY_SIZE(s_VREGS))?s_VREGS[_reg-LDO1]:UNKNOWN_REGULATOR)

#define TRANSLATE_REG(_reg)  ( (_reg)< LDO1 ? TRANSLATE_SREG(_reg) : TRANSLATE_VREG(_reg) )
//-----------------------------------------------------------------------------
// Local Functions

//------------------------------------------------------------------------------
//
// Function: PmicRegulatorOn
//
// This function is used to turn on the regulator.
//
// Parameters:
//      regulator [in]      which regulator to turn on
//
// Returns:
//      status
//          PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicRegulatorOn (PMIC_REGULATOR regulator)
{
    PMIC_STATUS     retval;
    WM_REGULATOR    wmReg;
    WM_STATUS       status;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    //
    // Work out which regulator we want.
    //
    wmReg = TRANSLATE_REG( regulator );
    if ( UNKNOWN_REGULATOR == wmReg )
    {
        DEBUGMSG( ZONE_ERROR, (
                  _T("%s: unknown regulator %d\r\n"),
                  _T(__FUNCTION__),
                  regulator
                ));
        retval = PMIC_NOT_SUPPORTED;
        goto done;
    }

    //
    // Now call the library to turn it on.
    //
    status = WMPmicReglEnable( g_hWMDevice, wmReg );
    if ( !WM_SUCCESS( status ) )
    {
        DEBUGMSG( ZONE_ERROR, (
                  _T("%s: WMPmicReglEnable (%d) failed: 0x%X\r\n"),
                  _T(__FUNCTION__),
                  regulator,
                  status
                ));
    }

    retval = WMStatusToPmicStatus( status );

done:
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return retval;
}

//------------------------------------------------------------------------------
//
// Function: PmicRegulatorOff
//
// This function is used to turn off the regulator
//
// Parameters:
//      regulator [in]      which regulator to turn off
//
// Returns:
//      status
//          PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicRegulatorOff (PMIC_REGULATOR regulator)
{
    PMIC_STATUS     retval;
    WM_REGULATOR    wmReg;
    WM_STATUS       status;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    //
    // Work out which regulator we want.
    //
    wmReg = TRANSLATE_REG( regulator );
    if ( UNKNOWN_REGULATOR == wmReg )
    {
        DEBUGMSG( ZONE_ERROR, (
                  _T("%s: unknown regulator %d\r\n"),
                  _T(__FUNCTION__),
                  regulator
                ));
        retval = PMIC_NOT_SUPPORTED;
        goto done;
    }

    //
    // Now call the library to turn it off.
    //
    status = WMPmicReglDisable( g_hWMDevice, wmReg );
    if ( !WM_SUCCESS( status ) )
    {
        DEBUGMSG( ZONE_ERROR, (
                  _T("%s: WMPmicReglEnable (%d) failed: 0x%X\r\n"),
                  _T(__FUNCTION__),
                  regulator,
                  status
                ));
    }

    retval = WMStatusToPmicStatus( status );

done:
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return retval;
}


//------------------------------------------------------------------------------
//
// Function: PmicRegulatorSetVoltageMilliVolts
//
// This function is to set the voltage level for the regulator.
//
// Parameters:
//          regulator [in]       which switch mode regulator to be set
//
//          voltageType [in]
//                  SW_VOLTAGE_NORMAL/SW_VOLTAGE_DVS/SW_VOLTAGE_STBY
//
//                  SWxy offers support for Dynamic Voltage-Frequency scaling. If this feature is
//                  activated, then assertion of the STANDBY input will automatically configure
//                  SWxy to output the voltage defined by the 6-bit field SWxy_STBY.
//                  If STANDBY=LOW, then assertion of the DVS input will automatically configure
//                  SWxy to output the voltage defined by the 6-bit field SWxy_DVS. These
//                  alternative bit fields would normally be programmed to a voltage lower than
//                  that encoded in the SWxy bit field. When STANDBY and DVS are both
//                  de-asserted, the output voltage will revert the that encoded by the SWxy field.
//
//          voltage [in]        voltage to be set to the regulator in mV.
//
// Returns:
//          status
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicRegulatorSetVoltageMilliVolts( PMIC_REGULATOR regulator,
                                               PMIC_REGULATOR_SREG_VOLTAGE_TYPE voltageType,
                                               PMIC_REGULATOR_VOLTAGE voltage )
{
    PMIC_STATUS         retval;
    WM_REGULATOR        wmReg;
    WM_REGULATOR_MODE   wmMode;
    WM_STATUS           status;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    //
    // Work out which regulator we want.
    //
    wmReg = TRANSLATE_REG( regulator );
    if ( UNKNOWN_REGULATOR == wmReg )
    {
        DEBUGMSG( ZONE_ERROR, (
                  _T("%s: unknown regulator %d\r\n"),
                  _T(__FUNCTION__),
                  regulator
                ));
        retval = PMIC_NOT_SUPPORTED;
        goto done;
    }

    //
    // Work out which mode.
    //
    switch ( voltageType )
    {
    case SW_VOLTAGE_NORMAL:
        wmMode = WM_REGMODE_NORMAL;
        break;
    case SW_VOLTAGE_STBY:
        wmMode = WM_REGMODE_LOW_POWER;
        break;
    default:
        retval = PMIC_NOT_SUPPORTED;
        goto done;
    }

    //
    // Now call the library to control the voltage.
    //
    status = WMPmicReglSetVoltage( g_hWMDevice, wmReg, voltage, wmMode );
    if ( !WM_SUCCESS( status ) )
    {
        DEBUGMSG( ZONE_ERROR, (
                  _T("%s: WMPmicReglSetVoltage (%d) failed: 0x%X\r\n"),
                  _T(__FUNCTION__),
                  regulator,
                  status
                ));
    }

    retval = WMStatusToPmicStatus( status );

done:
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return retval;
}


//------------------------------------------------------------------------------
//
// Function: PmicRegulatorGetVoltageMilliVolts
//
// This function is to get the voltage settings.
// Parameters:
//          regulator [in]             which regulator to get the voltage value
//
//          voltageType [in]
//                                          SW_VOLTAGE_NORMAL
//                                          SW_VOLTAGE_DVS
//                                          SW_VOLTAGE_STBY
//
//          pVoltage [out]             the pointer to store the return value
//
// Returns:
//          status
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------

PMIC_STATUS PmicRegulatorGetVoltageMilliVolts (
                                       PMIC_REGULATOR regulator,
                                       PMIC_REGULATOR_SREG_VOLTAGE_TYPE voltageType,
                                       PMIC_REGULATOR_VOLTAGE *pVoltage)
{
    PMIC_STATUS         retval;
    WM_REGULATOR        wmReg;
    unsigned int        mVNormal, mVLowPower;
    WM_STATUS           status;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    //
    // Work out which regulator we want.
    //
    wmReg = TRANSLATE_REG( regulator );
    if ( UNKNOWN_REGULATOR == wmReg )
    {
        DEBUGMSG( ZONE_ERROR, (
                  _T("%s: unknown regulator %d\r\n"),
                  _T(__FUNCTION__),
                  regulator
                ));
        retval = PMIC_NOT_SUPPORTED;
        goto done;
    }

    //
    // Now call the library to control the voltage.
    //
    status = WMPmicReglGetVoltage( g_hWMDevice, wmReg, &mVNormal, &mVLowPower );
    if ( !WM_SUCCESS( status ) )
    {
        DEBUGMSG( ZONE_ERROR, (
                  _T("%s: WMPmicReglGetVoltage (%d) failed: 0x%X\r\n"),
                  _T(__FUNCTION__),
                  regulator,
                  status
                ));
    }

    //
    // Work out which mode.
    //
    switch ( voltageType )
    {
    case SW_VOLTAGE_NORMAL:
        *pVoltage = mVNormal;
        break;
    case SW_VOLTAGE_STBY:
        *pVoltage = mVLowPower;
        break;
    default:
        retval = PMIC_NOT_SUPPORTED;
        goto done;
    }

    retval = WMStatusToPmicStatus( status );

done:
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return retval;
}


//------------------------------------------------------------------------------
//
// Function: PmicSwitchModeRegulatorOn
//
// This function is used to turn on the switch mode regulator.
//
// Parameters:
//             regulator [in]             which switch mode regulator to turn on
//
// Returns:
//          status
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicSwitchModeRegulatorOn (PMIC_REGULATOR_SREG regulator)
{
    PMIC_STATUS     retval;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    retval = PmicRegulatorOn( regulator );

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return retval;
}

//------------------------------------------------------------------------------
//
// Function: PmicSwitchModeRegulatorOff
//
// This function is used to turn off the switch regulator
//
// Parameters:
//             regulator [in]
//                     which switch mode regulator to turn off
//
// Returns:
//          status
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicSwitchModeRegulatorOff (PMIC_REGULATOR_SREG regulator)
{
    PMIC_STATUS     retval;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    retval = PmicRegulatorOff( regulator );

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return retval;
}


//------------------------------------------------------------------------------
//
// Function: PmicSwitchModeRegulatorSetVoltageLevel
//
// This function is to set the voltage level for the regulator.
//
// Parameters:
//          regulator [in]       which switch mode regulator to be set
//
//          voltageType [in]
//                  SW_VOLTAGE_NORMAL/SW_VOLTAGE_DVS/SW_VOLTAGE_STBY
//
//                  SWxy offers support for Dynamic Voltage-Frequency scaling. If this feature is
//                  activated, then assertion of the STANDBY input will automatically configure
//                  SWxy to output the voltage defined by the 6-bit field SWxy_STBY.
//                  If STANDBY=LOW, then assertion of the DVS input will automatically configure
//                  SWxy to output the voltage defined by the 6-bit field SWxy_DVS. These
//                  alternative bit fields would normally be programmed to a voltage lower than
//                  that encoded in the SWxy bit field. When STANDBY and DVS are both
//                  de-asserted, the output voltage will revert the that encoded by the SWxy field.
//
//          voltage [in]        voltage to be set to the regulator.
//                                  0-0x3F (0.900v-2.200v)
//
// Returns:
//          status
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicSwitchModeRegulatorSetVoltageLevel( PMIC_REGULATOR_SREG regulator,
                                                    PMIC_REGULATOR_SREG_VOLTAGE_TYPE voltageType,
                                                    PMIC_REGULATOR_SREG_VOLTAGE voltage )
{
    PMIC_STATUS     retval;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    retval = PmicRegulatorSetVoltageMilliVolts( regulator, voltageType, voltage );

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return retval;
}


//------------------------------------------------------------------------------
//
// Function: PmicSwitchModeRegulatorGetVoltageLevel
//
// This function is to get the voltage settings.
// Parameters:
//          regulator [in]             which regulator to get the voltage value
//
//          voltageType [in]
//                                          SW_VOLTAGE_NORMAL
//                                          SW_VOLTAGE_DVS
//                                          SW_VOLTAGE_STBY
//
//          pVoltage [out]             the pointer to store the return value
//
// Returns:
//          status
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------

PMIC_STATUS PmicSwitchModeRegulatorGetVoltageLevel (
                                       PMIC_REGULATOR_SREG regulator,
                                       PMIC_REGULATOR_SREG_VOLTAGE_TYPE voltageType,
                                       PMIC_REGULATOR_SREG_VOLTAGE *pVoltage)
{
    PMIC_STATUS             retval;
    PMIC_REGULATOR_VOLTAGE  voltage;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    retval = PmicRegulatorGetVoltageMilliVolts( regulator, voltageType, &voltage );
    *pVoltage = (PMIC_REGULATOR_SREG_VOLTAGE) voltage;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return retval;
}


//------------------------------------------------------------------------------
//
// Function: PmicSwitchModeRegulatorSetMode
//
// This function is to set the switch mode regulator into different mode for both standby
// and normal case.
//
// Parameters:
//          regulator [in]        the regulator to be set
//
//          standby [i]          standby = LOW,  the mode will be set in SWxMODE
//                                   standby = HIGH,  the mode will be set in SWxSTBYMODE
//
//          mode [in]          the mode to be use
//              1. OFF
//              2. PWM mode and no Pulse Skipping
//              3. PWM mode and pulse Skipping Allowed
//              4. Low Power PFM mode
//
// Returns:
//          status
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicSwitchModeRegulatorSetMode (
                                                       PMIC_REGULATOR_SREG regulator,
                                                       PMIC_REGULATOR_SREG_STBY standby,
                                                       PMIC_REGULATOR_SREG_MODE mode )
{
    UNREFERENCED_PARAMETER( regulator );
    UNREFERENCED_PARAMETER( standby );
    UNREFERENCED_PARAMETER( mode );

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    //
    // ### TBD
    //
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
//
// Function: PmicSwitchModeRegulatorGetMode
//
// This function is to get the switch regulator mode settings.
//
// Parameters:
//          regulator [in]        the regulator to get the settings from.
//
//          standby [i]          standby = LOW,  the mode will be set in SWxMODE
//                                   standby = HIGH,  the mode will be set in SWxSTBYMODE
//
//          mode [out]          the pointer to get the mode settings
//
// Returns:
//          status
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicSwitchModeRegulatorGetMode (
                                                       PMIC_REGULATOR_SREG regulator,
                                                       PMIC_REGULATOR_SREG_STBY standby,
                                                       PMIC_REGULATOR_SREG_MODE* mode )
{
    UNREFERENCED_PARAMETER( regulator );
    UNREFERENCED_PARAMETER( standby );
    UNREFERENCED_PARAMETER( mode );

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    //
    // ### TBD
    //
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
//
// Function: PmicSwitchModeRegulatorSetDVSSpeed
//
// This function is to set the DVS speed the regulator.
//
// Parameters:
//             regulator [in]         the regulator to be set
//             dvsspeed [in]
//                         0 - Transition speed is dictated by the current
//                               limit and input -output conditions
//                         1 - 25mV step each 4us
//                         2 - 25mV step each 8us
//                         3 - 25mV step each 16us
//
// Returns:
//          status
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicSwitchModeRegulatorSetDVSSpeed (
                                                       PMIC_REGULATOR_SREG regulator,
                                                       UINT8 dvsspeed)
{
    UNREFERENCED_PARAMETER( regulator );
    UNREFERENCED_PARAMETER( dvsspeed );

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    //
    // ### TBD
    //
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_NOT_SUPPORTED;
}



//------------------------------------------------------------------------------
//
// Function: PmicSwitchModeRegulatorEnablePanicMode
//
// This function is used to enable the panic mode.
//
// Parameters:
//           regulator [in]          the regulator to be set
//
// Returns:
//          status
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------

PMIC_STATUS PmicSwitchModeRegulatorEnablePanicMode(PMIC_REGULATOR_SREG regulator)
{
    UNREFERENCED_PARAMETER( regulator );

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    //
    // ### TBD
    //
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_NOT_SUPPORTED;
}


//------------------------------------------------------------------------------
//
// Function: PmicSwitchModeRegulatorDisablePanicMode
//
// This function is used to disable the panic mode.
//
// Parameters:
//           regulator [in]             the regulator to be set
//
// Returns:
//          status
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------

PMIC_STATUS PmicSwitchModeRegulatorDisablePanicMode(PMIC_REGULATOR_SREG regulator)
{
    UNREFERENCED_PARAMETER( regulator );

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    //
    // ### TBD
    //
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_NOT_SUPPORTED;
}


//------------------------------------------------------------------------------
//
// Function: PmicSwitchModeRegulatorEnableSoftStart
//
// This function is used to enable soft start.
//
// Parameters:
//           regulator [in]        the regulator to be set
//
// Returns:
//          status
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------

PMIC_STATUS PmicSwitchModeRegulatorEnableSoftStart(PMIC_REGULATOR_SREG regulator)
{
    UNREFERENCED_PARAMETER( regulator );

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    //
    // The WM8350 regulators always soft-start.
    //
    return PMIC_SUCCESS;
}


//------------------------------------------------------------------------------
//
// Function: PmicSwitchModeRegulatorDisableSoftStart
//
// This function is used to disable soft start.
//
// Parameters:
//           regulator [in]       the regulator to be set
//
// Returns:
//          status
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------

PMIC_STATUS PmicSwitchModeRegulatorDisableSoftStart(PMIC_REGULATOR_SREG regulator)
{
    UNREFERENCED_PARAMETER( regulator );

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    //
    // The WM8350 regulators always soft-start.
    //
    return PMIC_NOT_SUPPORTED;
}


//------------------------------------------------------------------------------
//
// Function: PmicVoltageRegulatorOn
//
// This function is used to turn on the voltage regulator
//
// Parameters:
//             regulator [in]         which voltage regulator to turn on
//
// Returns:
//          status
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicVoltageRegulatorOn (PMIC_REGULATOR_VREG regulator)
{
    PMIC_STATUS     retval;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    retval = PmicRegulatorOn( regulator );

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return retval;
}

//------------------------------------------------------------------------------
//
// Function: PmicVoltageRegulatorOff
//
// This function is used to turn off the regulator
//
// Parameters:
//             regulator [in]              which voltage regulator to turn off
//
// Returns:
//          status
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicVoltageRegulatorOff (PMIC_REGULATOR_VREG regulator)
{
    PMIC_STATUS     retval;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    retval = PmicRegulatorOff( regulator );

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return retval;
}


//------------------------------------------------------------------------------
//
// Function: PmicVoltageRegulatorSetVoltageLevel
//
// This function is used to set voltage level for the voltage regulator.
//
// Parameters:
//          regulator [in]     which voltage regulator to be set
//          voltage [in]       the voltage level to set
//
// Returns:
//          status
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicVoltageRegulatorSetVoltageLevel( PMIC_REGULATOR_VREG regulator,
                                                 PMIC_REGULATOR_VREG_VOLTAGE voltage )
{
    PMIC_STATUS             retval;
    PMIC_REGULATOR_VOLTAGE  mV;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    mV = voltage.mV;
    retval = PmicRegulatorSetVoltageMilliVolts( regulator, SW_VOLTAGE_NORMAL, mV );

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return retval;
}

//------------------------------------------------------------------------------
//
// Function: PmicVoltageRegulatorGetVoltageLevel
//
// This function is to get the current voltage settings of the regulator.
//
// Parameters:
//          regulator [in]           which voltage regulator to get from
//          voltage [out]           get voltage value
//
// Returns:
//          status
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicVoltageRegulatorGetVoltageLevel (
                                             PMIC_REGULATOR_VREG regulator,
                                             PMIC_REGULATOR_VREG_VOLTAGE *pVoltage)
{
    PMIC_STATUS             retval;
    PMIC_REGULATOR_VOLTAGE  mV;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    retval = PmicRegulatorGetVoltageMilliVolts( regulator, SW_VOLTAGE_NORMAL, &mV );
    pVoltage->mV = mV;

    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return retval;
}

//------------------------------------------------------------------------------
//
// Function: PmicVolatageRegulatorSetPowerMode
//
// This function is used to set low power mode for the regulator and whether to enter
// low power mode during STANDBY assertion or not.
//
// VxMODE=1, Set Low Power no matter of VxSTBY and STANDBY pin
// VxMODE=0, VxSTBY=1, Low Power Mode is contorled by STANDBY pin
// VxMODE=0, VxSTBY=0, Low Power Mode is disabled
//
// Parameters:
//          regulator [in]                which voltage regulator to be set
//          powerMode[in]              the power mode to be set
//
// Returns:
//          status
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicVoltageRegulatorSetPowerMode (
                                                            PMIC_REGULATOR_VREG regulator,
                                                            PMIC_REGULATOR_VREG_POWER_MODE powerMode)
{
    UNREFERENCED_PARAMETER( regulator );
    UNREFERENCED_PARAMETER( powerMode );

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    //
    // ### TBD
    //
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_NOT_SUPPORTED;
}


//------------------------------------------------------------------------------
//
// Function: PmicVolatageRegulatorGetPowerMode
//
// This function is to get the current power mode for the regulator
//
// Parameters:
//          regulator [in]                      to get which regulator setting
//          powerMode [out]                 the power mode to get
//
// Returns:
//          status
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicVoltageRegulatorGetPowerMode (
                                             PMIC_REGULATOR_VREG regulator,
                                             PMIC_REGULATOR_VREG_POWER_MODE* powerMode)
{
    UNREFERENCED_PARAMETER( regulator );
    UNREFERENCED_PARAMETER( powerMode );

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    //
    // ### TBD
    //
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
//
// Function: PmicSwitchModeRegulatorEnableSTBYDVFS
//
// This function is used to enable the standby or Dynamic Voltage-Frequency scaling.
//
// Parameters:
//             regulator [in]        the regulator to be set
//
// Returns:
//          status
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
// Remarks:
// This function is only applicable to Roadrunner, it is a stub function here.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicSwitchModeRegulatorEnableSTBYDVFS (PMIC_REGULATOR_SREG regulator)
{
    UNREFERENCED_PARAMETER( regulator );

    return PMIC_SUCCESS;
}

//------------------------------------------------------------------------------
//
// Function: PmicSwitchModeRegulatorDisableSTBYDVFS
//
// This function is used to disable the standby or Dynamic Voltage-Frequency scaling.
//
// Parameters:
//             regulator [in]        the regulator to be set
//
// Returns:
//          status
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
// Remarks:
// This function is only applicable to Roadrunner, it is a stub function here.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicSwitchModeRegulatorDisableSTBYDVFS (PMIC_REGULATOR_SREG regulator)
{
    UNREFERENCED_PARAMETER( regulator );

    return PMIC_SUCCESS;
}


// end of file
