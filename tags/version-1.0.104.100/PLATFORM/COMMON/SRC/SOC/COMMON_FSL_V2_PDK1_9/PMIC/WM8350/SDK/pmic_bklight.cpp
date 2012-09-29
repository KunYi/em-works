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
/// @file   pmic_bklight.c
/// @brief  Backlight functions for the WM8350 PMIC.
///
//  This file contains the PMIC Backlight SDK interface that is used by
//  applications and other drivers to access registers of the PMIC.
///
/// @version $Id: pmic_bklight.cpp 453 2007-05-02 11:33:48Z ib $
///
/// @Warning
///   This software is specifically written for Wolfson devices. It may not be
///   used with other devices.
///
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4201)   // Non-standard extension
#include <windows.h>
#pragma warning(pop)
#include <Devload.h>
#include <ceddk.h>
#include "common_macros.h"
#include "pmic_ioctl.h"
#include "pmic_basic_types.h"
#include "pmic_bklight.h"
#include "WMPmic.h"

//-----------------------------------------------------------------------------
// External Functions

//-----------------------------------------------------------------------------
// External Variables

//-----------------------------------------------------------------------------
// Defines

#define WINDOWS_TO_WM8350_LEVEL(_level) PMICLog2PhyBacklightLevel(_level)
#define WM8350_TO_WINDOWS_LEVEL(_level) PMICPhy2LogBacklightLevel(_level)

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

// The current levels of the two regulators
static WM_REGVAL    s_level_A = WINDOWS_TO_WM8350_LEVEL(BKL_LEVEL_DEFAULT);
static WM_REGVAL    s_level_B = WINDOWS_TO_WM8350_LEVEL(BKL_LEVEL_DEFAULT);

//-----------------------------------------------------------------------------
// Local Functions

//------------------------------------------------------------------------------
//
// Function: PmicBacklightMasterEnable
//
// This function sets the master enable bit of the PMIC backlight and tri-color led controller.
//
// Parameters:
//      None.
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBacklightMasterEnable()
{
    // Not required by the WM8350.
    return PMIC_SUCCESS;
}


//------------------------------------------------------------------------------
//
// Function: PmicBacklightMasterDisable
//
// This function clears the master enable bit of the PMIC backlight and tri-color led controller.
//
// Parameters:
//      None.
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBacklightMasterDisable()
{
    // Not required by the WM8350.
    return PMIC_SUCCESS;
}


//------------------------------------------------------------------------------
//
// Function: PmicBacklightEnable
//
// This function enables the given PMIC backlight with the given regulator.
//
// Parameters:
//      None.
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBacklightEnable(BACKLIGHT_CHANNEL channel, BACKLIGHT_REGULATOR regl)
{
    PMIC_STATUS     retval;
    WM_STATUS       status;
    WM_ISINK        iSink;
    WM_REGULATOR    reg;
    WM_REGVAL       wmLevel;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    switch (channel)
    {
        case BACKLIGHT_ISINK_A:
            iSink = WM_ISINK_A;
            wmLevel = s_level_A;
            break;

        case BACKLIGHT_ISINK_B:
            iSink = WM_ISINK_B;
            wmLevel = s_level_B;
            break;

        default:
            retval = PMIC_NOT_SUPPORTED;
            goto done;
            break;
    }

    switch (regl)
    {
        case BACKLIGHT_REGL_DCDC2:
            reg = WM_DCDC2;
            break;

        case BACKLIGHT_REGL_DCDC5:
            reg = WM_DCDC5;
            break;

        default:
            retval = PMIC_NOT_SUPPORTED;
            goto done;
            break;
    }

    status = WMPmicLEDEnable( g_hWMDevice, iSink, reg, wmLevel );
    if ( !WM_SUCCESS( status ) )
    {
        retval = PMIC_ERROR;
        goto done;
    }

    retval = PMIC_SUCCESS;

done:
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return retval;
}


//------------------------------------------------------------------------------
//
// Function: PmicBacklightDisable
//
// This function disables the given PMIC backlight and the given regulator.
//
// Parameters:
//      None.
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBacklightDisable(BACKLIGHT_CHANNEL channel, BACKLIGHT_REGULATOR regl)
{
    PMIC_STATUS     retval;
    WM_STATUS       status;
    WM_ISINK        iSink;
    WM_REGULATOR    reg;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    switch (channel)
    {
        case BACKLIGHT_ISINK_A:
            iSink = WM_ISINK_A;
            break;

        case BACKLIGHT_ISINK_B:
            iSink = WM_ISINK_B;
            break;

        default:
            retval = PMIC_NOT_SUPPORTED;
            goto done;
            break;
    }

    switch (regl)
    {
        case BACKLIGHT_REGL_DCDC2:
            reg = WM_DCDC2;
            break;

        case BACKLIGHT_REGL_DCDC5:
            reg = WM_DCDC5;
            break;

        default:
            retval = PMIC_NOT_SUPPORTED;
            goto done;
            break;
    }

    status = WMPmicLEDDisable( g_hWMDevice, iSink, reg );
    if ( !WM_SUCCESS( status ) )
    {
        retval = PMIC_ERROR;
        goto done;
    }

    retval = PMIC_SUCCESS;

done:
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return retval;
}


//------------------------------------------------------------------------------
//
// Function: PmicBacklightRampUp
//
// This function ramps up the PMIC backlight channel.
//
// Parameters:
//      channel [IN]    backlight channel
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBacklightRampUp(BACKLIGHT_CHANNEL channel)
{
    UNREFERENCED_PARAMETER( channel );

    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}


//------------------------------------------------------------------------------
//
// Function: PmicBacklightRampDown
//
// This function ramps down the PMIC backlight channel.
//
// Parameters:
//      channel[IN]    backlight channel
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBacklightRampDown(BACKLIGHT_CHANNEL channel)
{
    UNREFERENCED_PARAMETER( channel );

    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
//
// Function: PmicBacklightRampUpOff
//
// This function stops ramping of the PMIC backlight channel.
//
// Parameters:
//      channel [IN]    backlight channel
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBacklightRampOff(BACKLIGHT_CHANNEL channel)
{
    UNREFERENCED_PARAMETER( channel );

    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}


//------------------------------------------------------------------------------
//
// Function: PmicBacklightSetMode
//
// This function sets the operation mode for PMIC backlight channel.
//
// Parameters:
//      channel[IN]    backlight channel
//      mode[IN]    backlight operation mode
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBacklightSetMode(BACKLIGHT_CHANNEL channel, BACKLIGHT_MODE mode)
{
    PMIC_STATUS    retval;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    if ( BACKLIGHT_CURRENT_CTRL_MODE != mode )
    {
        retval = PMIC_NOT_SUPPORTED;
        goto done;
    }

    switch (channel)
    {
        case BACKLIGHT_ISINK_A:
        case BACKLIGHT_ISINK_B:
            retval = PMIC_SUCCESS;
            break;

        default:
            retval = PMIC_NOT_SUPPORTED;
            goto done;
            break;
    }

done:
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return retval;
}

//------------------------------------------------------------------------------
//
// Function: PmicBacklightGetMode
//
// This function sets the operation mode for PMIC backlight channel.
//
// Parameters:
//      channel[IN]    backlight channel
//      mode[OUT]    pointer to backlight operation mode
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBacklightGetMode(BACKLIGHT_CHANNEL channel, BACKLIGHT_MODE *mode)
{
    PMIC_STATUS    retval;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    switch (channel)
    {
        case BACKLIGHT_ISINK_A:
        case BACKLIGHT_ISINK_B:
            retval = PMIC_SUCCESS;
            *mode = BACKLIGHT_CURRENT_CTRL_MODE;
            break;

        default:
            retval = PMIC_NOT_SUPPORTED;
            goto done;
            break;
    }

done:
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return retval;
}


//------------------------------------------------------------------------------
//
//  Function: PmicBacklightSetCurrentLevel
//
//  This function program the current level for backlight channel.
//
//  Parameters:
//      channel[IN]    backlight channel
//      level[IN]    current level
//
//  Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBacklightSetCurrentLevel(BACKLIGHT_CHANNEL channel, UINT8 level)
{
    PMIC_STATUS     retval;
    WM_STATUS       status;
    WM_REGVAL       wmLevel = WINDOWS_TO_WM8350_LEVEL( level );

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    if ( wmLevel > WM8350_LED_MAX_BACKLIGHT_CURRENT_LEVEL )
    {
        ERRORMSG( 1, (_T("PmicBacklightSetCurrentLevel Invalid Parameter\r\n")));
        retval = PMIC_PARAMETER_ERROR;
        goto done;
    }

    switch ( channel )
    {
        case BACKLIGHT_ISINK_A:
            status = WMPmicLEDSetLevel( g_hWMDevice, WM_ISINK_A, wmLevel );
            s_level_A = wmLevel;
            break;

        case BACKLIGHT_ISINK_B:
            status = WMPmicLEDSetLevel( g_hWMDevice, WM_ISINK_B, wmLevel );
            s_level_B = wmLevel;
            break;

        default:
            retval = PMIC_NOT_SUPPORTED;
            goto done;
            break;
    }

    if ( !WM_SUCCESS( status ) )
    {
        retval = PMIC_ERROR;
        goto done;
    }

    retval = PMIC_SUCCESS;

done:
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return retval;
}


//------------------------------------------------------------------------------
//
//  Function: PmicBacklightGetCurrentLevel
//
//  This function returns the current level for backlight channel.
//
//  Parameters:
//      channel[IN]    backlight channel
//      level[OUT]    pointer to current level
//
//  Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBacklightGetCurrentLevel(BACKLIGHT_CHANNEL channel, UINT8* pLevel)
{
    PMIC_STATUS     retval;
    WM_STATUS       status;
    WM_REGVAL       wmLevel;

    DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__));

    switch ( channel )
    {
        case BACKLIGHT_ISINK_A:
            status = WMPmicLEDGetLevel( g_hWMDevice, WM_ISINK_A, &wmLevel );
            break;

        case BACKLIGHT_ISINK_B:
            status = WMPmicLEDGetLevel( g_hWMDevice, WM_ISINK_B, &wmLevel );
            break;

        default:
            retval = PMIC_NOT_SUPPORTED;
            goto done;
            break;
    }

    if ( !WM_SUCCESS( status ) )
    {
        retval = PMIC_ERROR;
        goto done;
    }

    *pLevel = WM8350_TO_WINDOWS_LEVEL((UINT8)wmLevel );

    retval = PMIC_SUCCESS;

done:
    DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__));

    return retval;
}


//------------------------------------------------------------------------------
//
//  Function: PmicBacklightSetDutyCycle
//
//  This function program the duty cycle for backlight channel.
//
//  Parameters:
//      channel[IN]    backlight channel
//      cycle[IN]    duty cycle
//
//  Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBacklightSetDutyCycle(BACKLIGHT_CHANNEL channel, UINT8 cycle)
{
    UNREFERENCED_PARAMETER( channel );
    UNREFERENCED_PARAMETER( cycle );

    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}


//------------------------------------------------------------------------------
//
//  Function: PmicBacklightGetDutyCycle
//
//  This function returns the duty cycle for backlight channel.
//
//  Parameters:
//      channel[IN]    backlight channel
//      cycle[OUT]    duty cycle
//
//  Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBacklightGetDutyCycle(BACKLIGHT_CHANNEL channel, UINT8* cycle)
{
    UNREFERENCED_PARAMETER( channel );
    UNREFERENCED_PARAMETER( cycle );

    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}


//------------------------------------------------------------------------------
//
//  Function: PmicBacklightSetCycleTime
//
//  This function program the cycle period for backlight controller.
//
//  Parameters:
//      period[IN]    cycle time.
//
//  Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBacklightSetCycleTime(UINT8 period)
{
    UNREFERENCED_PARAMETER( period );

    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}


//------------------------------------------------------------------------------
//
//  Function: PmicBacklightGetCycleTime
//
//  This function returns the cycle period for backlight controller.
//
//  Parameters:
//      period[OUT]    pointer to the cycle time.
//
//  Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBacklightGetCycleTime(UINT8* period)
{
    UNREFERENCED_PARAMETER( period );

    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}


//------------------------------------------------------------------------------
//
// Function: PmicBacklightEnableEdgeSlow
//
// This function enables the edge slowing for backlight.
//
// Parameters:
//      None.
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBacklightEnableEdgeSlow()
{
    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
//
// Function: PmicBacklightDisableEdgeSlow
//
// This function disables the edge slowing for backlight.
//
// Parameters:
//      None.
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBacklightDisableEdgeSlow()
{
    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
//
// Function: PmicBacklightGetEdgeSlow
//
// This function gets the edge slowing status for backlight.
//
// Parameters:
//      edge[OUT]    pointer to status of edge slowing for backlight.
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBacklightGetEdgeSlow(BOOL *edge)
{
    UNREFERENCED_PARAMETER( edge );

    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}


//------------------------------------------------------------------------------
//
// Function: PmicBacklightEnableBoostMode
//
// This function enables boost mode for backlight. (Only availabe on MC13783.
//
// Parameters:
//      none
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBacklightEnableBoostMode()
{
    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}


//------------------------------------------------------------------------------
//
// Function: PmicBacklightDisableBoostMode
//
// This function disables boost mode for backlight. (Only availabe on MC13783.
//
// Parameters:
//      none
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBacklightDisableBoostMode()
{
    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}


//------------------------------------------------------------------------------
//
// Function: PmicBacklightSetBoostMode
//
// This function gets the edge slowing status for backlight.
//
// Parameters:
//      edge[OUT]    pointer to status of edge slowing for backlight.
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicBacklightSetBoostMode(UINT32 abms, UINT32 abr)
{
    UNREFERENCED_PARAMETER( abms );
    UNREFERENCED_PARAMETER( abr );

    //
    // ### TBD
    //
    return PMIC_NOT_SUPPORTED;
}


// Logical backlight level to physical backlight level
UINT8 PMICLog2PhyBacklightLevel(UINT8 BacklightLevel)
{
    DWORD dwCurrent;
    DWORD dwMaxCurrent;
    UINT8 PhysicalValue;

    UINT8 minValue = WM8350_LED_MIN_BACKLIGHT_CURRENT_LEVEL;
    UINT8 maxValue = WM8350_LED_MAX_BACKLIGHT_CURRENT_LEVEL;

    dwMaxCurrent = PmicToCurrent[WM8350_LED_MAX_BACKLIGHT_CURRENT_LEVEL];
    
    dwCurrent = (dwMaxCurrent * ((DWORD)BacklightLevel + 1)) >> 8; 
   
    for(PhysicalValue = maxValue; PhysicalValue >= minValue; PhysicalValue--)
    {
        if(dwCurrent == PmicToCurrent[PhysicalValue])
        {
            break;
        }
        else if(dwCurrent > PmicToCurrent[PhysicalValue])
        {
            if((PmicToCurrent[PhysicalValue + 1] - dwCurrent) < (dwCurrent - PmicToCurrent[PhysicalValue]))
            {
                PhysicalValue++;
            }

            break;
        }
    }

    return PhysicalValue;
}


// Physical backlight level to logical backlight level
UINT8 PMICPhy2LogBacklightLevel(UINT8 BacklightLevel)
{
    DWORD dwCurrent;
    DWORD dwMaxCurrent;
    DWORD dwLogicalValue;

    dwMaxCurrent = PmicToCurrent[WM8350_LED_MAX_BACKLIGHT_CURRENT_LEVEL];
    dwCurrent = PmicToCurrent[BacklightLevel];

    dwLogicalValue = ((((dwCurrent << 8) + dwMaxCurrent - 1) / dwMaxCurrent) - 1);

    if(dwLogicalValue > BKL_LEVEL_MAX)
    {
        dwLogicalValue = BKL_LEVEL_MAX;
    }

    if(dwLogicalValue < BKL_LEVEL_MIN)
    {
        dwLogicalValue = BKL_LEVEL_MIN;
    }
    
    return (UINT8)dwLogicalValue;
}

