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
/// @version $Id: pmic_tcled.cpp 419 2007-04-24 09:25:37Z ib $
///
/// @Warning
///   This software is specifically written for Wolfson devices. It may not be
///   used with other devices.
///
//-----------------------------------------------------------------------------
//
//  File:  pmic_tcled.cpp
//
//  This file contains the PMIC TCLED SDK interface that is used by applications
//  and other drivers to access registers of the PMIC.
//
//-----------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4201)   // Non-standard extension
#include <windows.h>
#pragma warning(pop)
#include <Devload.h>
#include <ceddk.h>
#include "common_macros.h"
#include "pmic_ioctl.h"
#include "pmic_tcled.h"

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
// Function: PmicTCLEDEnable
//
// This function enables the Tri-Colour LED operation mode.
//
// Parameters:
//      mode [in] : operational mode for Tri-Color LED
//      bank[in]  : Selected tri-color bank
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicTCLEDEnable(TCLED_MODE mode, FUNLIGHT_BANK bank)
{
    UNREFERENCED_PARAMETER( mode );
    UNREFERENCED_PARAMETER( bank );

    //
    // Not supported by WM8350
    //

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_NOT_SUPPORTED;
}


//-----------------------------------------------------------------------------
//
// Function: PmicTCLEDDisable
//
// This function disables the Tri-Color LED operation mode.
//
// Parameters:
//      bank[in]  : Selected tri-color bank
//
// Returns:
//      PMIC_STATUS.
//
//-----------------------------------------------------------------------------
PMIC_STATUS PmicTCLEDDisable(FUNLIGHT_BANK bank)
{
    UNREFERENCED_PARAMETER( bank );

    //
    // Not supported by WM8350
    //

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_NOT_SUPPORTED;
}

//-----------------------------------------------------------------------------
//
// Function: PmicTCLEDGetMode
//
// This function retrieves the Tri-Color LED operation mode.
//
// Parameters:
//      mode [out] : pointer to operational mode for Tri-Color LED
//      bank[in]  : Selected tri-color bank
//
// Returns:
//      PMIC_STATUS.
//
//-----------------------------------------------------------------------------
PMIC_STATUS PmicTCLEDGetMode(TCLED_MODE *mode, FUNLIGHT_BANK bank)
{
    UNREFERENCED_PARAMETER( mode );
    UNREFERENCED_PARAMETER( bank );

    //
    // Not supported by WM8350
    //

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_NOT_SUPPORTED;
}

//-----------------------------------------------------------------------------
//
// Function: PmicTCLEDFunLightSetCurrentLevel
//
// This function sets the current level when Tri-Color LED is operating in fun light mode.
//
// Parameters:
//      bank
//           [in] bank.
//      channel
//           [in] channel.
//      level
//           [in] current level.
//
// Returns:
//      PMIC_STATUS.
//
//-----------------------------------------------------------------------------
PMIC_STATUS PmicTCLEDFunLightSetCurrentLevel(FUNLIGHT_BANK bank, FUNLIGHT_CHANNEL channel, TCLED_CUR_LEVEL level)
{
    UNREFERENCED_PARAMETER( bank );
    UNREFERENCED_PARAMETER( channel );
    UNREFERENCED_PARAMETER( level );

    //
    // Not supported by WM8350
    //

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_NOT_SUPPORTED;
}


//-----------------------------------------------------------------------------
//
// Function: PmicTCLEDFunLightGetCurrentLevel
//
// This function retrieves the current level when Tri-Color LED is operating in fun light mode.
//
// Parameters:
//      bank
//           [in] bank.
//      channel
//           [in] channel.
//      level
//           [out] variable to receive current level.
//
// Returns:
//      PMIC_STATUS.
//
//-----------------------------------------------------------------------------
PMIC_STATUS PmicTCLEDFunLightGetCurrentLevel(FUNLIGHT_BANK bank, FUNLIGHT_CHANNEL channel, TCLED_CUR_LEVEL* level)
{
    UNREFERENCED_PARAMETER( bank );
    UNREFERENCED_PARAMETER( channel );
    UNREFERENCED_PARAMETER( level );

    //
    // Not supported by WM8350
    //

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_NOT_SUPPORTED;
}


//-----------------------------------------------------------------------------
//
// Function: PmicTCLEDFunLightSetCycleTime
//
// This function sets the cycle time when Tri-Color LED is operating in fun light mode.
//
// Parameters:
//      bank
//           [in] bank.
//      ct
//           [in] cycle time.
//
// Returns:
//      PMIC_STATUS.
//
//-----------------------------------------------------------------------------
PMIC_STATUS PmicTCLEDFunLightSetCycleTime(FUNLIGHT_BANK bank, TCLED_FUN_CYCLE_TIME ct)
{
    UNREFERENCED_PARAMETER( bank );
    UNREFERENCED_PARAMETER( ct );

    //
    // Not supported by WM8350
    //

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_NOT_SUPPORTED;
}


//-----------------------------------------------------------------------------
//
// Function: PmicTCLEDFunLightGetCycleTime
//
// This function retrieves the cycle time when Tri-Color LED is operating in fun light mode.
//
// Parameters:
//      bank
//           [in] bank.
//      ct
//           [out] variable to receive cycle time.
//
// Returns:
//      PMIC_STATUS.
//
//-----------------------------------------------------------------------------
PMIC_STATUS PmicTCLEDFunLightGetCycleTime(FUNLIGHT_BANK bank, TCLED_FUN_CYCLE_TIME* ct)
{
    UNREFERENCED_PARAMETER( bank );
    UNREFERENCED_PARAMETER( ct );

    //
    // Not supported by WM8350
    //

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_NOT_SUPPORTED;
}


//-----------------------------------------------------------------------------
//
// Function: PmicTCLEDFunLightSetDutyCycle
//
// This function sets the duty cycle when Tri-Color LED is operating in fun light
// mode. The valid duty cycle settings are integers from 0 to 31.
//
// Parameters:
//      bank
//           [in] bank.
//      channel
//           [in] channel.
//      dc
//           [in] duty cycle.
//
// Returns:
//      PMIC_STATUS.
//
//-----------------------------------------------------------------------------
PMIC_STATUS PmicTCLEDFunLightSetDutyCycle(FUNLIGHT_BANK bank, FUNLIGHT_CHANNEL channel, unsigned char dc)
{
    UNREFERENCED_PARAMETER( bank );
    UNREFERENCED_PARAMETER( channel );
    UNREFERENCED_PARAMETER( dc );

    //
    // Not supported by WM8350
    //

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_NOT_SUPPORTED;
}


//-----------------------------------------------------------------------------
//
// Function: PmicTCLEDFunLightGetDutyCycle
//
// This function retrieves the duty cycle when Tri-Color LED is operating in fun light mode.
//
// Parameters:
//      bank
//           [in] bank.
//      channel
//           [in] channel.
//      dc
//           [out] variable to receive duty cycle.
//
// Returns:
//      PMIC_STATUS.
//
//-----------------------------------------------------------------------------
PMIC_STATUS PmicTCLEDFunLightGetDutyCycle(FUNLIGHT_BANK bank, FUNLIGHT_CHANNEL channel, unsigned char* dc)
{
    UNREFERENCED_PARAMETER( bank );
    UNREFERENCED_PARAMETER( channel );
    UNREFERENCED_PARAMETER( dc );

    //
    // Not supported by WM8350
    //

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_NOT_SUPPORTED;
}


//-----------------------------------------------------------------------------
//
// Function: PmicTCLEDFunLightBlendedRamps
//
// This function sets the fun light in Blended Ramps pattern.
// The calling program has to disable the bank before changing to a new pattern, if the bank is enabled.
//
//
// Parameters:
//      bank
//           [in] bank.
//      speed
//           [in] speed.
//
// Returns:
//      PMIC_STATUS.
//
//-----------------------------------------------------------------------------
PMIC_STATUS PmicTCLEDFunLightBlendedRamps(FUNLIGHT_BANK bank, TCLED_FUN_SPEED speed)
{
    UNREFERENCED_PARAMETER( bank );
    UNREFERENCED_PARAMETER( speed );

    //
    // Not supported by WM8350
    //

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_NOT_SUPPORTED;
}


//-----------------------------------------------------------------------------
//
// Function: PmicTCLEDFunLightSawRamps
//
// This function sets the fun light in Saw Ramps pattern.
// The calling program has to disable the bank before changing to a new pattern, if the bank is enabled.
//
// Parameters:
//      bank
//           [in] bank.
//      speed
//           [in] speed.
//
// Returns:
//      PMIC_STATUS.
//
//-----------------------------------------------------------------------------
PMIC_STATUS PmicTCLEDFunLightSawRamps(FUNLIGHT_BANK bank, TCLED_FUN_SPEED speed)
{
    UNREFERENCED_PARAMETER( bank );
    UNREFERENCED_PARAMETER( speed );

    //
    // Not supported by WM8350
    //

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_NOT_SUPPORTED;
}


//-----------------------------------------------------------------------------
//
// Function: PmicTCLEDFunLightBlendedBowtie
//
// This function sets the fun light in Blended Bowtie pattern.
// The calling program has to disable the bank before changing to a new pattern, if the bank is enabled.
//
// Parameters:
//      bank
//           [in] bank.
//      speed
//           [in] speed.
//
// Returns:
//      PMIC_STATUS.
//
//-----------------------------------------------------------------------------
PMIC_STATUS PmicTCLEDFunLightBlendedBowtie(FUNLIGHT_BANK bank, TCLED_FUN_SPEED speed)
{
    UNREFERENCED_PARAMETER( bank );
    UNREFERENCED_PARAMETER( speed );

    //
    // Not supported by WM8350
    //

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_NOT_SUPPORTED;
}

//-----------------------------------------------------------------------------
//
// Function: PmicTCLEDFunLightChasingLightsPattern
//
// This function sets the fun light in Chasing Lights RGB pattern.
//
// Parameters:
//      bank
//           [in] bank.
//      pattern
//           [in] pattern.
//      speed
//           [in] speed.
//
// Returns:
//      PMIC_STATUS.
//
//-----------------------------------------------------------------------------
PMIC_STATUS PmicTCLEDFunLightChasingLightsPattern(FUNLIGHT_BANK bank, CHASELIGHT_PATTERN pattern, TCLED_FUN_SPEED speed)
{
    UNREFERENCED_PARAMETER( bank );
    UNREFERENCED_PARAMETER( pattern );
    UNREFERENCED_PARAMETER( speed );

    //
    // Not supported by WM8350
    //

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_NOT_SUPPORTED;
}

//-----------------------------------------------------------------------------
//
// Function: PmicTCLEDFunLightRampUp
//
// This function starts LEDs Brightness Ramp Up function. Ramp time is fixed at 1 second.
//
// Parameters:
//      bank
//           [in] bank.
//      channel
//           [in] the channel to ramp up.
//
// Returns:
//      PMIC_STATUS.
//
//-----------------------------------------------------------------------------
PMIC_STATUS PmicTCLEDFunLightRampUp(FUNLIGHT_BANK bank, FUNLIGHT_CHANNEL channel)
{
    UNREFERENCED_PARAMETER( bank );
    UNREFERENCED_PARAMETER( channel );

    //
    // Not supported by WM8350
    //

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_NOT_SUPPORTED;
}


//-----------------------------------------------------------------------------
//
// Function: PmicTCLEDFunLightRampDown
//
// This function starts LEDs Brightness Ramp Down function. Ramp time is fixed at 1 second.
//
// Parameters:
//      bank
//           [in] bank.
//      channel
//           [in] the channel to ramp down.
//
// Returns:
//      PMIC_STATUS.
//
//-----------------------------------------------------------------------------
PMIC_STATUS PmicTCLEDFunLightRampDown(FUNLIGHT_BANK bank, FUNLIGHT_CHANNEL channel)
{
    UNREFERENCED_PARAMETER( bank );
    UNREFERENCED_PARAMETER( channel );

    //
    // Not supported by WM8350
    //

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_NOT_SUPPORTED;
}

//-----------------------------------------------------------------------------
//
// Function: PmicTCLEDFunLightRampOff
//
// This function stop LEDs Brightness Ramp function.
//
// Parameters:
//      bank
//           [in] bank.
//      channel
//           [in] the channel to stop ramp
//
// Returns:
//      PMIC_STATUS.
//
//-----------------------------------------------------------------------------
PMIC_STATUS PmicTCLEDFunLightRampOff(FUNLIGHT_BANK bank, FUNLIGHT_CHANNEL channel)
{
    UNREFERENCED_PARAMETER( bank );
    UNREFERENCED_PARAMETER( channel );

    //
    // Not supported by WM8350
    //

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_NOT_SUPPORTED;
}

//-----------------------------------------------------------------------------
//
// Function: PmicTCLEDFunLightTriodeOn
//
// This function enable triode mode of the channel.
//
// Parameters:
//      bank
//           [in] bank.
//      channel
//           [in] the channel to enable triode mode.
//
// Returns:
//      PMIC_STATUS.
//
//-----------------------------------------------------------------------------
PMIC_STATUS PmicTCLEDFunLightTriodeOn(FUNLIGHT_BANK bank, FUNLIGHT_CHANNEL channel)
{
    UNREFERENCED_PARAMETER( bank );
    UNREFERENCED_PARAMETER( channel );

    //
    // Not supported by WM8350
    //

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_NOT_SUPPORTED;
}

//-----------------------------------------------------------------------------
//
// Function: PmicTCLEDFunLightTriodeOff
//
// This function disable triode mode of the channel.
//
// Parameters:
//      bank
//           [in] bank.
//      channel
//           [in] the channel to disable triode mode.
//
// Returns:
//      PMIC_STATUS.
//
//-----------------------------------------------------------------------------
PMIC_STATUS PmicTCLEDFunLightTriodeOff(FUNLIGHT_BANK bank, FUNLIGHT_CHANNEL channel)
{
    UNREFERENCED_PARAMETER( bank );
    UNREFERENCED_PARAMETER( channel );

    //
    // Not supported by WM8350
    //

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_NOT_SUPPORTED;
}

//-----------------------------------------------------------------------------
//
// Function: PmicTCLEDEnableEdgeSlow
//
// This function enable Analog Edge Slowing.
//
// Parameters:
//      None.
//
// Returns:
//      PMIC_STATUS.
//
//-----------------------------------------------------------------------------
PMIC_STATUS PmicTCLEDEnableEdgeSlow(void)
{
    //
    // Not supported by WM8350
    //

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_NOT_SUPPORTED;
}

//-----------------------------------------------------------------------------
//
// Function: PmicTCLEDDisableEdgeSlow
//
// This function disable Analog Edge Slowing.
//
// Parameters:
//      None.
//
// Returns:
//      PMIC_STATUS.
//
//-----------------------------------------------------------------------------
PMIC_STATUS PmicTCLEDDisableEdgeSlow(void)
{
    //
    // Not supported by WM8350
    //

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_NOT_SUPPORTED;
}

//-----------------------------------------------------------------------------
//
// Function: PmicTCLEDEnableHalfCurrent
//
// This function enable Half Current Mode for Tri-color 1 Driver channel.
//
// Parameters:
//      None.
//
// Returns:
//      PMIC_STATUS.
//
//-----------------------------------------------------------------------------
PMIC_STATUS PmicTCLEDEnableHalfCurrent(void)
{
    //
    // Not supported by WM8350
    //

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_NOT_SUPPORTED;
}

//-----------------------------------------------------------------------------
//
// Function: PmicTCLEDDisableHalfCurrent
//
// This function disable Half Current Mode for Tri-color 1 Driver channel.
//
// Parameters:
//      None.
//
// Returns:
//      PMIC_STATUS.
//
//-----------------------------------------------------------------------------
PMIC_STATUS PmicTCLEDDisableHalfCurrent(void)
{
    //
    // Not supported by WM8350
    //

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return PMIC_NOT_SUPPORTED;
}


PMIC_STATUS PmicTCLEDIndicatorSetCurrentLevel(IND_CHANNEL channel, TCLED_CUR_LEVEL level)
{
    UNREFERENCED_PARAMETER( channel );
    UNREFERENCED_PARAMETER( level );

    return PMIC_NOT_SUPPORTED;
};
PMIC_STATUS PmicTCLEDIndicatorGetCurrentLevel(IND_CHANNEL channel, TCLED_CUR_LEVEL* level)
{
    UNREFERENCED_PARAMETER( channel );
    UNREFERENCED_PARAMETER( level );

    return PMIC_NOT_SUPPORTED;
};
PMIC_STATUS PmicTCLEDIndicatorSetBlinkPattern(IND_CHANNEL channel, TCLED_IND_BLINK_MODE mode, BOOL skip)
{
    UNREFERENCED_PARAMETER( channel );
    UNREFERENCED_PARAMETER( mode );
    UNREFERENCED_PARAMETER( skip );

    return PMIC_NOT_SUPPORTED;
};
PMIC_STATUS PmicTCLEDIndicatorGetBlinkPattern(IND_CHANNEL channel, TCLED_IND_BLINK_MODE* mode, BOOL* skip)
{
    UNREFERENCED_PARAMETER( channel );
    UNREFERENCED_PARAMETER( mode );
    UNREFERENCED_PARAMETER( skip );

    return PMIC_NOT_SUPPORTED;
};
PMIC_STATUS PmicTCLEDFunLightStrobe(FUNLIGHT_CHANNEL channel, TCLED_FUN_STROBE_SPEED speed)
{
    UNREFERENCED_PARAMETER( channel );
    UNREFERENCED_PARAMETER( speed );

    return PMIC_NOT_SUPPORTED;
};

