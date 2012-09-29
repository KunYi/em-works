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
/// @file   pmic_gpio.c
/// @brief  GPIO functions for the WM8350 PMIC.
///
/// This file contains the SDK interface that is used by applications and other
/// drivers to control GPIOs on the PMIC.
///
/// @version $Id: pmic_gpio.cpp 650 2007-06-15 22:31:22Z ib $
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
#include "pmic_basic_types.h"
#include "pmic_gpio.h"
#include "pmic_lla.h"
#include "WMPmic.h"

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
// Function: PmicGpioSetLevels
//
// This function sets the level of the given GPIOs.
//
// Parameters:
//      GPIOs           Bitmask of GPIOs to configure.
//      levelMask       GPIO levels to set - 1 = high and 0 = low.
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicGpioSetLevels(WM_GPIOS GPIOs, WM_GPIOS levelMask)
{
    PMIC_STATUS     retval;
    WM_STATUS       status;

    status = WMGPSetLevels(g_hWMDevice, GPIOs, levelMask);

    retval = WMStatusToPmicStatus(status);

    return retval;
}


//------------------------------------------------------------------------------
//
// Function: PmicGpioGetLevels
//
// This function returns the current GPIO levels.
//
// Parameters:
//      pLevels         Receives the GPIO levels as a bitmask. For each bit,
//                      set corresponds to a high level and clear corresponds
//                      to low.
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicGpioGetLevels(WM_GPIOS *pHigh)
{
    PMIC_STATUS     retval;
    WM_STATUS       status;

    status = WMGPGetLevels(g_hWMDevice, pHigh);

    retval = WMStatusToPmicStatus(status);

    return retval;
}


//------------------------------------------------------------------------------
//
// Function: PmicGpioClearInt
//
// This function clears interrupts on the given input GPIOs.
//
// Parameters:
//      GPIOs           Bitmask of GPIOs to configure.
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicGpioClearInt(WM_GPIOS GPIOs)
{
    PMIC_STATUS     retval;
    WM_STATUS       status;

    status = WMGPClearInt(g_hWMDevice, GPIOs);

    retval = WMStatusToPmicStatus(status);

    return retval;
}


//------------------------------------------------------------------------------
//
// Function: PmicGpioSetDir
//
// This function sets up the given GPIOs as inputs or outputs.
//
// Parameters:
//      GPIOs           Bitmask of GPIOs to configure.
//      dir             GPIO direction to set.
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicGpioSetDir(WM_GPIOS GPIOs, WM_GPIO_DIR dir)
{
    PMIC_STATUS     retval;
    WM_STATUS       status;

    status = WMGPSetDir(g_hWMDevice, GPIOs, dir);

    retval = WMStatusToPmicStatus(status);

    return retval;
}


//------------------------------------------------------------------------------
//
// Function: PmicGpioGetInputs
//
// This function returns the set of GPIOs which are configured as inputs.
//
// Parameters:
//      pInputs         Receives the GPIOs configured as inputs as a bitmask.
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicGpioGetInputs(WM_GPIOS *pInputs)
{
    PMIC_STATUS     retval;
    WM_STATUS       status;

    status = WMGPGetInputs(g_hWMDevice, pInputs);

    retval = WMStatusToPmicStatus(status);

    return retval;
}


//------------------------------------------------------------------------------
//
// Function: PmicGpioGetOutputs
//
// This function returns the set of GPIOs which are configured as outputs.
//
// Parameters:
//      pOutputs        Receives the GPIOs configured as outputs as a bitmask.
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicGpioGetOutputs(WM_GPIOS *pOutputs)
{
    PMIC_STATUS     retval;
    WM_STATUS       status;

    status = WMGPGetOutputs(g_hWMDevice, pOutputs);

    retval = WMStatusToPmicStatus(status);

    return retval;
}


//------------------------------------------------------------------------------
//
// Function: PmicGpioSetPolarity
//
// This function configures the polarity of the given GPIOs.
//
// This only makes sense for GPIOs configured as inputs.
//
// Parameters:
//      GPIOs           Bitmask of GPIOs to configure.
//      polarity        Polarity to set.
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicGpioSetPolarity(WM_GPIOS GPIOs, WM_GPIO_POLARITY polarity)
{
    PMIC_STATUS     retval;
    WM_STATUS       status;

    status = WMGPSetPolarity(g_hWMDevice, GPIOs, polarity);

    retval = WMStatusToPmicStatus(status);

    return retval;
}


//------------------------------------------------------------------------------
//
// Function: PmicGpioGetPolarity
//
// This function returns the polarity of the GPIOs.
//
// This function returns a bitmask.  1 corresponds to active high and 0
// corresponds to active low.  This only makes sense for input GPIOs.
//
// Parameters:
//      pActiveHigh     Receives the GPIOs configured as active high as a bitmask.
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicGpioGetPolarity(WM_GPIOS *pActiveHigh)
{
    PMIC_STATUS     retval;
    WM_STATUS       status;

    status = WMGPGetPolarity(g_hWMDevice, pActiveHigh);

    retval = WMStatusToPmicStatus(status);

    return retval;
}


//------------------------------------------------------------------------------
//
// Function: PmicGpioSetAutoInvert
//
// This function configures the auto-invert of the given GPIOs.
//
// GPIOs configured with auto-invert automatically flip polarity when an
// interrupt is triggered to detect press and release.
//
// This only makes sense for GPIOs configured as inputs.
//
// Parameters:
//      GPIOs           Bitmask of GPIOs to configure.
//      polarity        Polarity to set.
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicGpioSetAutoInvert(WM_GPIOS GPIOs, WM_GPIO_POLARITY autoInvert)
{
    PMIC_STATUS     retval;
    WM_STATUS       status;

    status = WMGPSetAutoInvert(g_hWMDevice, GPIOs, autoInvert);

    retval = WMStatusToPmicStatus(status);

    return retval;
}


//------------------------------------------------------------------------------
//
// Function: PmicGpioGetAutoInvert
//
// This function returns the set of GPIOs which are configured with auto-invert.
//
// Parameters:
//      pAutoInvert     Receives the GPIOs configured with auto-invert as a bitmask.
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicGpioGetAutoInvert(WM_GPIOS *pAutoInvert)
{
    PMIC_STATUS     retval;
    WM_STATUS       status;

    status = WMGPGetAutoInvert(g_hWMDevice, pAutoInvert);

    retval = WMStatusToPmicStatus(status);

    return retval;
}


//------------------------------------------------------------------------------
//
// Function: PmicGpioSetPinType
//
// This function configures the pin type of the given GPIOs.
//
// This only makes sense for GPIOs configured as outputs.
//
// Parameters:
//      GPIOs           Bitmask of GPIOs to configure.
//      levelMask       GPIO levels to set - 1 = high and 0 = low.
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicGpioSetPinType(WM_GPIOS GPIOs, WM_GPIO_PIN_TYPE pinType)
{
    PMIC_STATUS     retval;
    WM_STATUS       status;

    status = WMGPSetPinType(g_hWMDevice, GPIOs, pinType);

    retval = WMStatusToPmicStatus(status);

    return retval;
}


//------------------------------------------------------------------------------
//
// Function: PmicGpioGetPinType
//
// This function returns the pin types of the GPIOs.
//
// This function returns a bitmask.  1 corresponds to open-drain and 0
// corresponds to CMOS.  This only makes sense for output GPIOs.
//
// Parameters:
//      pOpenDrain      Receives the GPIOs configured as open drain as a bitmask.
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicGpioGetPinType(WM_GPIOS *pOpenDrain)
{
    PMIC_STATUS     retval;
    WM_STATUS       status;

    status = WMGPGetPinType(g_hWMDevice, pOpenDrain);

    retval = WMStatusToPmicStatus(status);

    return retval;
}


//------------------------------------------------------------------------------
//
// Function: PmicGpioSetPullUp
//
// This function sets or removes pull-ups on the given GPIOs.
//
// This will remove pull-downs from GPIOs configured with pull-ups.
//
// Parameters:
//      GPIOs           Bitmask of GPIOs to configure.
//      pullUp          Whether to enable (TRUE) or disable (FALSE) pull-up.
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicGpioSetPullUp(WM_GPIOS GPIOs, BOOL pullUp)
{
    PMIC_STATUS     retval;
    WM_STATUS       status;

    status = WMGPSetPullUp(g_hWMDevice, GPIOs, pullUp);

    retval = WMStatusToPmicStatus(status);

    return retval;
}


//------------------------------------------------------------------------------
//
// Function: PmicGpioGetPullUp
//
// This function returns the the set of GPIOs which are configured with pull-ups.
//
// Parameters:
//      pPullUps        Receives the GPIOs configured with pull-ups as a bitmask.
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicGpioGetPullUp(WM_GPIOS *pPullUps)
{
    PMIC_STATUS     retval;
    WM_STATUS       status;

    status = WMGPGetPullUp(g_hWMDevice, pPullUps);

    retval = WMStatusToPmicStatus(status);

    return retval;
}


//------------------------------------------------------------------------------
//
// Function: PmicGpioSetPullDown
//
// This function sets or removes pull-Downs on the given GPIOs.
//
// This will remove pull-downs from GPIOs configured with pull-Downs.
//
// Parameters:
//      GPIOs           Bitmask of GPIOs to configure.
//      pullDown          Whether to enable (TRUE) or disable (FALSE) pull-Down.
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicGpioSetPullDown(WM_GPIOS GPIOs, BOOL pullDown)
{
    PMIC_STATUS     retval;
    WM_STATUS       status;

    status = WMGPSetPullDown(g_hWMDevice, GPIOs, pullDown);

    retval = WMStatusToPmicStatus(status);

    return retval;
}


//------------------------------------------------------------------------------
//
// Function: PmicGpioGetPullDown
//
// This function returns the the set of GPIOs which are configured with pull-downs.
//
// Parameters:
//      pPullDowns        Receives the GPIOs configured with pull-downs as a bitmask.
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicGpioGetPullDown(WM_GPIOS *pPullDowns)
{
    PMIC_STATUS     retval;
    WM_STATUS       status;

    status = WMGPGetPullDown(g_hWMDevice, pPullDowns);

    retval = WMStatusToPmicStatus(status);

    return retval;
}


//------------------------------------------------------------------------------
//
// Function: PmicGpioSetDebounce
//
// This function configures the debouncing of the given GPIOs.
//
// Parameters:
//      GPIOs           Bitmask of GPIOs to configure.
//      debounce        Whether to add or remove debounce.
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicGpioSetDebounce(WM_GPIOS GPIOs, BOOL debounce)
{
    PMIC_STATUS     retval;
    WM_STATUS       status;

    status = WMGPSetDebounce(g_hWMDevice, GPIOs, debounce);

    retval = WMStatusToPmicStatus(status);

    return retval;
}


//------------------------------------------------------------------------------
//
// Function: PmicGpioGetDebounce
//
// This function returns the set of GPIOs which are configured with debounce.
//
// Parameters:
//      GPIOs           Bitmask of GPIOs to configure.
//      pDebounced      Receives the GPIOs configured with debounce as a bitmask.
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicGpioGetDebounce(WM_GPIOS *pDebounced)
{
    PMIC_STATUS     retval;
    WM_STATUS       status;

    status = WMGPGetDebounce(g_hWMDevice, pDebounced);

    retval = WMStatusToPmicStatus(status);

    return retval;
}


//------------------------------------------------------------------------------
//
// Function: PmicGpioSetFunction
//
// This function sets the function of the given GPIO.
//
// Parameters:
//      GPIOs           Bitmask of GPIOs to configure.
//      function        GPIO function to set.
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicGpioSetFunction(WM_GPIO GPIO, WM_GPIO_ALTFN function)
{
    PMIC_STATUS     retval;
    WM_STATUS       status;

    status = WMGPSetFunction(g_hWMDevice, GPIO, function);

    retval = WMStatusToPmicStatus(status);

    return retval;
}


//------------------------------------------------------------------------------
//
// Function: PmicGpioGetFunction
//
// This function returns the current GPIO alternate function.
//
// Parameters:
//      GPIO            GPIO to query.
//      pFunction       Receives the GPIO function.
//
// Returns:
//      PMIC_STATUS.
//
//------------------------------------------------------------------------------
PMIC_STATUS PmicGpioGetFunction(WM_GPIO GPIO, WM_GPIO_ALTFN *pFunction)
{
    PMIC_STATUS     retval;
    WM_STATUS       status;

    status = WMGPGetFunction(g_hWMDevice, GPIO, pFunction);

    retval = WMStatusToPmicStatus(status);

    return retval;
}


