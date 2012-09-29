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
/// @version $Id: pmic_pwrctrl.cpp 419 2007-04-24 09:25:37Z ib $
///
/// @Warning
///   This software is specifically written for Wolfson devices. It may not be
///   used with other devices.
///
//-----------------------------------------------------------------------------
//
//  File:  pmic_pwrctrl.cpp
//
//  This file contains the PMIC power control and power cut SDK interface that is used by applications
//  and other drivers to access registers of the PMIC.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4201)   // Non-standard extension
#include <windows.h>
#pragma warning(pop)
#include "common_macros.h"
#include "pmic_ioctl.h"
#include "pmic_pwrctrl.h"

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

//-----------------------------------------------------------------------------
// Local Functions

//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlSetPowerCutTimer
//
// This function is used to set the power cut timer duration.
//
// Parameters:
//             duration [in]             The value to set to power cut timer register, it's from 0 to 255.
//                                            The timer will be set to a duration of 0 to 31.875 seconds,
//                                            in 125 ms increments.
//
// Returns:
//          status
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlSetPowerCutTimer (UINT8 duration)
{
    UNREFERENCED_PARAMETER( duration );

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_NOT_SUPPORTED;
}


//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlGetPowerCutTimer
//
// This function is used to get the power cut timer duration.
//
// Parameters:
//             duration [out]             the duration to set to power cut timer
//
// Returns:
//          status
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlGetPowerCutTimer (UINT8* duration)
{
    UNREFERENCED_PARAMETER( duration );

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    *duration = 0;
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_NOT_SUPPORTED;
}



//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlEnablePowerCut
//
// This function is used to enable the power cut.
//
// Parameters:
//             none
//
// Returns:
//          status
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlEnablePowerCut (void)
{
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_NOT_SUPPORTED;
}


//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlDisablePowerCut
//
// This function is used to disable the power cut.
//
// Parameters:
//             none
//
// Returns:
//          status
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlDisablePowerCut (void)
{
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlSetPowerCutCounter
//
// This function is used to set the power cut counter.
//
// Parameters:
//             counter [in]            The counter number value to be set to the register. It's value
//                                          from 0 to 15.
//
//                                          The power cut counter is a 4 bit counter that keeps track of
//                                          the number of rising edges of the UV_TIMER (power cut
//                                          events) that have occurred since the counter was last initialized.
//
// Returns:
//          status
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlSetPowerCutCounter (UINT8 counter)
{
    UNREFERENCED_PARAMETER( counter );

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_NOT_SUPPORTED;
}


//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlGetPowerCutCounter
//
// This function is used to get the power cut counter.
//
// Parameters:
//             counter [out]            to get the counter number
//
// Returns:
//          status
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlGetPowerCutCounter (UINT8* counter)
{
    UNREFERENCED_PARAMETER( counter );

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    *counter = 0;
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_NOT_SUPPORTED;
}


//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlSetPowerCutMaxCounter
//
// This function is used to set the maxium number of power cut counter.
//
// Parameters:
//             counter [in]            maxium counter number to set. It's value from 0 to 15.
//
//                                          The power cut register provides a method for disabling
//                                          power cuts if this situation manifests itself.
//                                          If PC_COUNT >= PC_MAX_COUNT, then the number of
//                                          resets that have occurred since the power cut counter was
//                                          last initialized exceeds the established limit, and power cuts
//                                          will be disabled.
//
// Returns:
//          status
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlSetPowerCutMaxCounter (UINT8 counter)
{
    UNREFERENCED_PARAMETER( counter );

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_NOT_SUPPORTED;
}



//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlgetPowerCutMaxCounter
//
// This function is used to get the setting of maxium power cut counter.
//
// Parameters:
//             counter [out]            to get the maxium counter number
//
// Returns:
//          status
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlGetPowerCutMaxCounter (UINT8* counter)
{
    UNREFERENCED_PARAMETER( counter );

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    *counter = 0;
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_NOT_SUPPORTED;
}



//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlEnableCounter
//
// The power cut register provides a method for disabling power cuts if this situation
// manifests itself. If PC_COUNT >= PC_MAX_COUNT, then the number of resets that
// have occurred since the power cut counter was last initialized exceeds the established
// limit, and power cuts will be disabled.
// This function can be disabled by setting PC_COUNT_EN=0. In this case, each power cut
// event will increment the power cut counter, but power cut coverage will not be disabled,
// even if PC_COUNT exceeds PC_MAX_COUNT.
//
// This PmicPwrctrlEnableCounter function will set PC_COUNT_EN=1.
//
// Parameters:
//             none
//
// Returns:
//          status
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlEnableCounter(void)
{
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_NOT_SUPPORTED;
}



//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlDisableCounter
//
// The power cut register provides a method for disabling power cuts if this situation
// manifests itself. If PC_COUNT >= PC_MAX_COUNT, then the number of resets that
// have occurred since the power cut counter was last initialized exceeds the established
// limit, and power cuts will be disabled.
// This function can be disabled by setting PC_COUNT_EN=0. In this case, each power cut
// event will increment the power cut counter, but power cut coverage will not be disabled,
// even if PC_COUNT exceeds PC_MAX_COUNT.
//
// This PmicPwrctrlEnableCounter function will set PC_COUNT_EN=0.
//
// Parameters:
//             none
//
// Returns:
//          status
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlDisableCounter (void)
{
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_NOT_SUPPORTED;
}



//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlSetMemHoldTimer
//
// This function is used to set the duration of memory hold timer.
//
// Parameters:
//             duration [in]             The value to set to memory hold timer register. It's from
//                                            0 to 15.
//                                            The resolution of the memory hold timer is 32 seconds
//                                            for a maximum duration of 512 seconds.
//
// Returns:
//          status
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlSetMemHoldTimer (UINT8 duration)
{
    UNREFERENCED_PARAMETER( duration );

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlGetMemHoldTimer
//
// This function is used to get the setting of memory hold timer
//
// Parameters:
//             duration [out]            to get the duration of the timer
//
// Returns:
//          status
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlGetMemHoldTimer (UINT8* duration)
{
    UNREFERENCED_PARAMETER( duration );

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    *duration = 0;
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_NOT_SUPPORTED;
}


//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlSetMemHoldTimerAllOn
//
// This function is used to set the duration of the memory hold timer to infinity
//
// Parameters:
//             none
//
// Returns:
//          status
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlSetMemHoldTimerAllOn (void)
{
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_NOT_SUPPORTED;
}


//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlClearMemHoldTimerAllOn
//
// This function is used to clear the infinity duration of the memory hold timer
//
// Parameters:
//             none
//
// Returns:
//          status
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlClearMemHoldTimerAllOn (void)
{
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_NOT_SUPPORTED;
}


//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlEnableClk32kMCU
//
// This function is used to enable the CLK32KMCU
//
// Parameters:
//             none
//
// Returns:
//          status
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlEnableClk32kMCU (void)
{
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlDisableClk32kMCU
//
// This function is used to disable the CLK32KMCU
//
// Parameters:
//             none
//
// Returns:
//          status
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlDisableClk32kMCU (void)
{
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_NOT_SUPPORTED;
}



//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlEnableUserOffModeWhenDelay
//
// This function is used to place the phone in User Off Mode after a delay.
//
// Parameters:
//             none
//
// Returns:
//          status
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlEnableUserOffModeWhenDelay (void)
{
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_NOT_SUPPORTED;
}


//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlDisableUserOffModeWhenDelay
//
// This function is used to set not to place the phone in User Off Mode after a delay.
//
// Parameters:
//             none
//
// Returns:
//          status
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlDisableUserOffModeWhenDelay (void)
{
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_NOT_SUPPORTED;
}


//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlSetVBKUPRegulator
//
// This function is used to set the VBKUP regulator
//
// Parameters:
//             reg [in]           the backup regulator to set
//             mode [in]        the mode to set to backup regulator
//
//                                   VBKUP_MODE1 - VBKUPxEN = 0, VBKUPxAUTO = 0
//                                   Backup Regulator Off in Non Power Cut Modes and Off in Power Cut Modes
//
//                                   VBKUP_MODE2 - VBKUPxEN = 0, VBKUPxAUTO = 1
//                                   Backup Regulator Off in Non Power Cut Modes and On in Power Cut Modes
//
//                                   VBKUP_MODE3 - VBKUPxEN = 1, VBKUPxAUTO = 0
//                                   Backup Regulator On in Non Power Cut Modes and Off in Power Cut Modes
//
//                                   VBKUP_MODE4 - VBKUPxEN = 1, VBKUPxAUTO = 1
//                                   Backup Regulator On in Non Power Cut Modes and On in Power Cut Modes
//
// Returns:
//          status
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlSetVBKUPRegulator (MC13783_PWRCTRL_REG_VBKUP reg, MC13783_PWRCTRL_VBKUP_MODE mode)
{
    UNREFERENCED_PARAMETER( reg );
    UNREFERENCED_PARAMETER( mode );

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    //
    // ### TBD - is this HIBERNATE mode on the WM8350?
    //
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_NOT_SUPPORTED;
}


//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlSetVBKUPRegulatorVoltage
//
// This function is used to set the VBKUP regulator voltage
//
// Parameters:
//             reg [in]           the backup regulator to set
//             volt [in]        the voltage to set to backup regulator
//                                0 - 1.0v
//                                1 - 1.2v
//                                2 - 1.5v
//                                3 - 1.8v
//
// Returns:
//          status
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlSetVBKUPRegulatorVoltage (MC13783_PWRCTRL_REG_VBKUP reg, UINT8 volt)
{
    UNREFERENCED_PARAMETER( reg );
    UNREFERENCED_PARAMETER( volt );

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    //
    // ### TBD - is this HIBERNATE mode on the WM8350?
    //
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlEnableWarmStart
//
// This function is used to set the phone to transit from the ON state to the User Off state
// when either the USER_OFF pin is pulled high or the USER_OFF_SPI bit is set (after an 8ms
// delay in the Memwait state).
//
// Parameters:
//             none
//
// Returns:
//          status
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlEnableWarmStart (void)
{
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    //
    // ### TBD - is this HIBERNATE mode on the WM8350?
    //
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_NOT_SUPPORTED;
}



//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlDisableWarmStart
//
// This function is used to disable the warm start and set the phone to transit from the
// ON state to the MEMHOLD ONLY state when either the USER_OFF pin is pulled high or
// the USER_OFF_SPI bit is set (after an 8ms delay in the Memwait state).
//
// Parameters:
//             none
//
// Returns:
//          status
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlDisableWarmStart (void)
{
    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    //
    // ### TBD - is this HIBERNATE mode on the WM8350?
    //
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlEnableRegenAssig
//
// This function enables the REGEN pin of slected voltage regulator. The
// REGEN function can be used in two ways. It can be used as a regulator enable pin like with SIMEN
// where the SPI programming is static and the REGEN pin is dynamic. It can also be used in a static
// fashion where REGEN is maintained high while the regulators get enabled and disabled dynamically
// via SPI. In that case REGEN functions as a master enable..
//
// Parameters:
//             t_regulator regu
//
// Returns:
//          status
//                  PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlEnableRegenAssig (t_regulator regu)
{
    UNREFERENCED_PARAMETER( regu );

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    //
    // ### TBD - is this PWR_1/PWR_2/PWR_3 HIBERNATE mode on the WM8350?
    //
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_NOT_SUPPORTED;
}
//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlDisableRegenAssig
//
// This function Disbale the REGEN pin of slected voltage regulator.
//
// Parameters:
//             t_regulator regu
//
// Returns:
//          status
// PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlDisableRegenAssig (t_regulator regu)
{
    UNREFERENCED_PARAMETER( regu );

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    //
    // ### TBD - is this HIBERNATE mode on the WM8350?
    //
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_NOT_SUPPORTED;
}
//------------------------------------------------------------------------------
//
// Function: PmicPwrctrlGetRegenAssin
//
// This function reads the REGEN pin value for said voltage regulator.
//
// Parameters:
//             t_regulator regu , value
//
// Returns:
//          status
// PMIC_SUCCESS for success and PMIC_ERROR for failure
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicPwrctrlGetRegenAssig (t_regulator regu , UINT8* value)
{
    UNREFERENCED_PARAMETER( regu );
    UNREFERENCED_PARAMETER( value );

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    //
    // ### TBD - is this HIBERNATE mode on the WM8350?
    //
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_NOT_SUPPORTED;
}

// end of file
