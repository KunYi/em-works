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
/// @file   pmic_connectivity.c
/// @brief  Connectivity functions for the WM8350 PMIC.
///
/// This file contains the SDK interface that is used by applications and other
/// drivers to access connectivity feature of the PMIC.
///
/// Note: these functions are not supported by the WM8350, so these are all
/// stub functions.
///
/// @version $Id: pmic_connectivity.cpp 363 2007-04-20 15:47:55Z ib $
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
#include "pmic_status.h"
#include "pmic_ioctl.h"
#include "pmic_connectivity.h"

#ifdef DEBUG

// Debug zone bit positions
#define ZONEID_ERROR        0
#define ZONEID_WARN            1
#define ZONEID_INIT            2
#define ZONEID_FUNC            3
#define ZONEID_INFO            4

// Debug zone masks
#define ZONEMASK_ERROR         (1 << ZONEID_ERROR)
#define ZONEMASK_WARN        (1 << ZONEID_WARN)
#define ZONEMASK_INIT        (1 << ZONEID_INIT)
#define ZONEMASK_FUNC        (1 << ZONEID_FUNC)
#define ZONEMASK_INFO        (1 << ZONEID_INFO)

// Debug zone args to DEBUGMSG
#define ZONE_ERROR             DEBUGZONE(ZONEID_ERROR)
#define ZONE_WARN            DEBUGZONE(ZONEID_WARN)
#define ZONE_INIT            DEBUGZONE(ZONEID_INIT)
#define ZONE_FUNC            DEBUGZONE(ZONEID_FUNC)
#define ZONE_INFO            DEBUGZONE(ZONEID_INFO)

extern DBGPARAM dpCurSettings;

#endif// DEBUG


//-----------------------------------------------------------------------------
// External Functions

//-----------------------------------------------------------------------------
// External Variables
extern "C" HANDLE               hPMI;           // Our global handle to the PMIC driver

//-----------------------------------------------------------------------------
// Defines

#define ENTRY_MSG        DEBUGMSG(ZONE_FUNC, (TEXT("+%s()\r\n"), __WFUNCTION__))
#define EXIT_MSG        DEBUGMSG(ZONE_FUNC, (TEXT("-%s()\r\n"), __WFUNCTION__))

//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables


//-----------------------------------------------------------------------------
// Local Variables

//-----------------------------------------------------------------------------
// Local Functions


PMIC_STATUS PmicConvityInit(void)
{
    ENTRY_MSG;
    EXIT_MSG;
    return PMIC_NOT_SUPPORTED;
}

void PmicConvityDeinit(void)
{
    ENTRY_MSG;
    EXIT_MSG;
}


//------------------------------------------------------------------------------
// Function: PmicConvityOpen
//
// Attempt to open and gain exclusive access to the PMIC connectivity
// hardware. An initial operating mode must also be specified.
//
// If the open request is successful, then a numeric handle is returned
// and this handle must be used in all subsequent function calls. The
// same handle must also be used in the pmic_convity_close() call when use
// of the PMIC connectivity hardware is no longer required.
//
// Parameters:
//            handle          device handle from open() call
//            mode            initial connectivity operating mode
//
// Returns:
//           PMIC_SUCCESS    if the open request was successful
//------------------------------------------------------------------------------
PMIC_STATUS PmicConvityOpen(PMIC_CONVITY_HANDLE *const handle,
                            const PMIC_CONVITY_MODE    mode)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( mode );

    ENTRY_MSG;
    EXIT_MSG;
    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
// Function: PmicConvityClose
//
// Terminate further access to the PMIC connectivity hardware. Also allows
// another process to call PmicConvityOpen() to gain access.
//
// Parameters:
//            handle          device handle from PmicConvityOpen() call
// Returns:
//           PMIC_SUCCESS    if the open request was successful
//------------------------------------------------------------------------------
PMIC_STATUS PmicConvityClose(const PMIC_CONVITY_HANDLE handle)
{
    UNREFERENCED_PARAMETER( handle );

    ENTRY_MSG;
    EXIT_MSG;
    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
// Function: PmicConvitySetMode
//
// Change the current operating mode of the PMIC connectivity hardware.
// The available connectivity operating modes is hardware dependent and
// consists of one or more of the following: USB (including USB On-the-Go),
// RS-232, and CEA-936. Requesting an operating mode that is not supported
// by the PMIC hardware will return PMIC_NOT_SUPPORTED.
//
// Parameters:
//            handle          device handle from PmicConvityOpen() call
//            mode            desired operating mode
// Returns:
//           PMIC_SUCCESS    if the requested mode was successfully set
//------------------------------------------------------------------------------
PMIC_STATUS PmicConvitySetMode(const PMIC_CONVITY_HANDLE handle,
                               const PMIC_CONVITY_MODE   mode)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( mode );

    ENTRY_MSG;
    EXIT_MSG;
    return PMIC_NOT_SUPPORTED;
}
//------------------------------------------------------------------------------
// Function: PmicConvityGetMode
//
//  Get the current operating mode for the PMIC connectivity hardware.
//
// Parameters:
//            handle          device handle from PmicConvityOpen() call
//            mode            the current PMIC connectivity operating mode
// Returns:
//           PMIC_SUCCESS    if the operation was successful
//------------------------------------------------------------------------------
PMIC_STATUS PmicConvityGetMode(const PMIC_CONVITY_HANDLE handle,
                               PMIC_CONVITY_MODE *const  mode)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( mode );

    ENTRY_MSG;
    EXIT_MSG;
    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
// Function: PmicConvityReset
//
//  Restore all registers to the initial power-on/reset state.
//
// Parameters:
//            handle          device handle from PmicConvityOpen() call
// Returns:
//           PMIC_SUCCESS    if the reset was successful
//------------------------------------------------------------------------------
PMIC_STATUS PmicConvityReset(const PMIC_CONVITY_HANDLE handle)
{
    UNREFERENCED_PARAMETER( handle );

    ENTRY_MSG;
    EXIT_MSG;
    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
// Function: PmicConvitySetCallback
//
// Register a callback function that will be used to signal PMIC connectivity
// events. For example, the USB subsystem should register a callback function
// in order to be notified of device connect/disconnect events. Note, however,
// that non-USB events may also be signalled depending upon the PMIC hardware
// capabilities. Therefore, the callback function must be able to properly
// handle all of the possible events if support for non-USB peripherals is
// also to be included.
//
// Parameters:
//            handle          device handle from PmicConvityOpen() call
//            func            a pointer to the callback function
//            eventMask       a mask selecting events to be notified
// Returns:
//           PMIC_SUCCESS    if the callback was successful registered
//------------------------------------------------------------------------------
PMIC_STATUS PmicConvitySetCallback(const PMIC_CONVITY_HANDLE   handle,
                                   const PMIC_CONVITY_CALLBACK func,
                                   const PMIC_CONVITY_EVENTS   eventMask)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( func );
    UNREFERENCED_PARAMETER( eventMask );

    ENTRY_MSG;
    EXIT_MSG;
    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
// Function: PmicConvityClearCallback
//
// Clears the current callback function. If this function returns successfully
// then all future Connectivity events will only be handled by the default
// handler within the Connectivity driver.
//
// Parameters:
//            handle          device handle from PmicConvityOpen() call
// Returns:
//           PMIC_SUCCESS    if the callback was successful cleared
//------------------------------------------------------------------------------
PMIC_STATUS PmicConvityClearCallback(const PMIC_CONVITY_HANDLE handle)
{
    UNREFERENCED_PARAMETER( handle );

    ENTRY_MSG;
    EXIT_MSG;
    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
// Function: PmicConvityGetCallback
//
// Get the current callback function and event mask.
//
// Parameters:
//            handle          device handle from PmicConvityOpen() call
//            func            the current callback function
//            eventMask       the current event selection mask
// Returns:
//           PMIC_SUCCESS    if the callback information was successful
//                           retrieved
//------------------------------------------------------------------------------
PMIC_STATUS PmicConvityGetCallback(const PMIC_CONVITY_HANDLE        handle,
                                   PMIC_CONVITY_CALLBACK     *const func,
                                   PMIC_CONVITY_EVENTS       *const eventMask)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( func );
    UNREFERENCED_PARAMETER( eventMask );

    ENTRY_MSG;
    EXIT_MSG;
    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
// Function: PmicConvityGetEventStatus
//
// Get the connectivity Event status
//
//  Parameters:
//            handle          device handle from PmicConvityOpen() call
//            events          the currently signalled events
//
//  Returns:
//           PMIC_SUCCESS    if the transceiver speed was successfully set
//------------------------------------------------------------------------------

PMIC_STATUS PmicConvityGetEventStatus(const PMIC_CONVITY_HANDLE handle,
                                    PMIC_CONVITY_EVENTS *const events)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( events );

    ENTRY_MSG;
    EXIT_MSG;
    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
// Set the USB transceiver speed.
//
// Parameters:
//            handle          device handle from PmicConvityOpen() call
//            speed           the desired USB transceiver speed
//            mode            the USB transceiver mode
// Returns:
//           PMIC_SUCCESS    if the transceiver speed was successfully set
//------------------------------------------------------------------------------
PMIC_STATUS PmicConvityUsbSetSpeed(const PMIC_CONVITY_HANDLE    handle,
                                   const PMIC_CONVITY_USB_SPEED speed,
                                   const PMIC_CONVITY_USB_MODE  mode)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( speed );
    UNREFERENCED_PARAMETER( mode );

    ENTRY_MSG;
    EXIT_MSG;
    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
// Function: PmicConvityUsbGetSpeed
//
// Get the USB transceiver speed.
//
// Parameters:
//            handle          device handle from PmicConvityOpen() call
//            speed           the current USB transceiver speed
//            mode            the current USB transceiver mode
// Returns:
//           PMIC_SUCCESS    if the transceiver speed was successfully obtained
//------------------------------------------------------------------------------
PMIC_STATUS PmicConvityUsbGetSpeed(const PMIC_CONVITY_HANDLE     handle,
                                   PMIC_CONVITY_USB_SPEED *const speed,
                                   PMIC_CONVITY_USB_MODE *const  mode)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( speed );
    UNREFERENCED_PARAMETER( mode );

    ENTRY_MSG;
    EXIT_MSG;
    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
// Function: PmicConvityUsbSetPowerSource
//
// Set the USB transceiver's power supply configuration.
//
// Parameters:
//            handle          device handle from PmicConvityOpen() call
//            pwrin           USB transceiver regulator input power source
//            pwrout          USB transceiver regulator output power level
// Returns:
//           PMIC_SUCCESS    if the USB transceiver's power supply
//                           configuration was successfully set
//------------------------------------------------------------------------------
PMIC_STATUS PmicConvityUsbSetPowerSource(const PMIC_CONVITY_HANDLE        handle,
                                         const PMIC_CONVITY_USB_POWER_IN  pwrin,
                                         const PMIC_CONVITY_USB_POWER_OUT pwrout)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( pwrin );
    UNREFERENCED_PARAMETER( pwrout );

    ENTRY_MSG;
    EXIT_MSG;
    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
// Function: PmicConvityUsbGetPowerSource
//
// Get the USB transceiver's current power supply configuration.
//
// Parameters:
//            handle          device handle from PmicConvityOpen() call
//            pwrin           USB transceiver regulator input power source
//            pwrout          USB transceiver regulator output power level
// Returns:
//           PMIC_SUCCESS    if the USB transceiver's power supply
//                           configuration was successfully retrieved
//------------------------------------------------------------------------------
PMIC_STATUS PmicConvityUsbGetPowerSource(const PMIC_CONVITY_HANDLE         handle,
                                         PMIC_CONVITY_USB_POWER_IN  *const pwrin,
                                         PMIC_CONVITY_USB_POWER_OUT *const pwrout)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( pwrin );
    UNREFERENCED_PARAMETER( pwrout );

    ENTRY_MSG;
    EXIT_MSG;
    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
// Function: PmicConvityUsbSetXcvr
//
// Set the USB transceiver's operating mode.
//
// Parameters:
//            handle          device handle from PmicConvityOpen() call
//            mode            desired operating mode
// Returns:
//           PMIC_SUCCESS    if the USB transceiver's operating mode
//                           was successfully configured
//------------------------------------------------------------------------------
PMIC_STATUS PmicConvityUsbSetXcvr(const PMIC_CONVITY_HANDLE             handle,
                                  const PMIC_CONVITY_USB_TRANSCEIVER_MODE mode)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( mode );

    ENTRY_MSG;
    EXIT_MSG;
    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
// Function: PmicConvityUsbGetXcvr
//
// Get the USB transceiver's current operating mode.
//
// Parameters:
//            handle          device handle from PmicConvityOpen() call
//            mode            current operating mode
// Returns:
//           PMIC_SUCCESS    if the USB transceiver's operating mode
//                           was successfully retrieved
//------------------------------------------------------------------------------
PMIC_STATUS PmicConvityUsbGetXcvr(const PMIC_CONVITY_HANDLE              handle,
                                  PMIC_CONVITY_USB_TRANSCEIVER_MODE *const mode)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( mode );

    ENTRY_MSG;
    EXIT_MSG;
    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
// Function: PmicConvityUsbOtgSetDlpDuration
//
// Set the Data Line Pulse duration (in milliseconds) for the USB OTG
// Session Request Protocol.
// For MC13783, the get/set DLP duration APIs are not supported.
// Parameters:
//            handle          device handle from PmicConvityOpen() call
//            duration        the data line pulse duration (ms)
// Returns:
//           PMIC_SUCCESS    if the pulse duration was successfully set
//------------------------------------------------------------------------------
PMIC_STATUS PmicConvityUsbOtgSetDlpDuration(const PMIC_CONVITY_HANDLE handle,
                                            const unsigned int      duration)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( duration );

    ENTRY_MSG;
    EXIT_MSG;
    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
// Function: PmicConvityUsbOtgGetDlpDuration
//
// Get the current Data Line Pulse duration (in milliseconds) for the USB
// OTG Session Request Protocol.
// Parameters:
//            handle          device handle from PmicConvityOpen() call
//            duration        the data line pulse duration (ms)
// Returns:
//           PMIC_SUCCESS    if the pulse duration was successfully obtained
//------------------------------------------------------------------------------
PMIC_STATUS PmicConvityUsbOtgGetDlpDuration(const PMIC_CONVITY_HANDLE handle,
                                            unsigned int *const     duration)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( duration );

    ENTRY_MSG;
    EXIT_MSG;
    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
// Function: PmicConvityUsbOtgBeginHnp
//
// Explicitly start the USB OTG Host Negotiation Protocol (HNP) process.
// This simply involves pulling the D+ line high for the "A" device
// and disconnecting all pull-up and pull-down resistors for the "B"
// device.
// Note that the pmic_convity_usb_otg_end_hnp() function must be called
// after a suitable time has elapsed to complete the HNP process.
// Parameters:
//            handle          device handle from PmicConvityOpen() call
//            deviceType      the USB device type (either A or B)
// Returns:
//           PMIC_SUCCESS    if the HNP was successfully started
//------------------------------------------------------------------------------
PMIC_STATUS PmicConvityUsbOtgBeginHnp(const PMIC_CONVITY_HANDLE          handle,
                                      const PMIC_CONVITY_USB_DEVICE_TYPE deviceType)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( deviceType );

    ENTRY_MSG;
    EXIT_MSG;
    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
// Function: PmicConvityUsbOtgEndHnp
//
// Explicitly complete the USB OTG Host Negotiation Protocol (HNP) process.
// This just involves disconnecting the pull-up resistor on D+ for the "A"
// device and turning on the pull-up resistor on D+ for the "B" device.
//
// Note that this function should only be called after a suitable time has
// elapsed after calling pmic_convity_usb_otg_begin_hnp().
// Parameters:
//            handle          device handle from PmicConvityOpen() call
//            deviceType      the USB device type (either A or B)
// Returns:
//           PMIC_SUCCESS    if the HNP was successfully ended
//------------------------------------------------------------------------------
PMIC_STATUS PmicConvityUsbOtgEndHnp(const PMIC_CONVITY_HANDLE          handle,
                                    const PMIC_CONVITY_USB_DEVICE_TYPE deviceType)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( deviceType );

    ENTRY_MSG;
    EXIT_MSG;
    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
// Function: PmicConvityUsbOtgSetConfig
//
// Set the USB On-The-Go (OTG) configuration.
// Parameters:
//            handle          device handle from PmicConvityOpen() call
//            cfg             desired USB OTG configuration
// Returns:
//           PMIC_SUCCESS    if the OTG configuration was successfully set
//------------------------------------------------------------------------------
PMIC_STATUS PmicConvityUsbOtgSetConfig(const PMIC_CONVITY_HANDLE      handle,
                                       const PMIC_CONVITY_USB_OTG_CONFIG cfg)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( cfg );

    ENTRY_MSG;
    EXIT_MSG;
    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
// Function: PmicConvityUsbOtgClearConfig
//
// Clears the USB On-The-Go (OTG) configuration. Multiple configuration settings
// may be OR'd together in a single call. However, selecting conflicting
// settings (e.g., multiple VBUS current limits) will result in undefined
// behavior.
// Parameters:
//            handle          device handle from PmicConvityOpen() call
//            cfg             USB OTG configuration settings to be cleared.
// Returns:
//           PMIC_SUCCESS         If the OTG configuration was successfully
//                                cleared.
//           PMIC_ERROR           If the handle is invalid.
//           PMIC_NOT_SUPPORTED   If the desired USB OTG configuration is
//                                not supported by the PMIC hardware.
//------------------------------------------------------------------------------
PMIC_STATUS PmicConvityUsbOtgClearConfig(const PMIC_CONVITY_HANDLE handle,
                                         const PMIC_CONVITY_USB_OTG_CONFIG cfg)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( cfg );

    ENTRY_MSG;
    EXIT_MSG;
    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
// Function: PmicConvityUsbOtgGetConfig
//
// Get the current USB On-The-Go (OTG) configuration.
// Parameters:
//            handle          device handle from PmicConvityOpen() call
//            cfg             the current USB OTG configuration
// Returns:
//           PMIC_SUCCESS         if the OTG configuration was successfully
//                                retrieved
//------------------------------------------------------------------------------
PMIC_STATUS PmicConvityUsbOtgGetConfig(const PMIC_CONVITY_HANDLE       handle,
                                       PMIC_CONVITY_USB_OTG_CONFIG *const cfg)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( cfg );

    ENTRY_MSG;
    EXIT_MSG;
    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
// Function: PmicConvityRs232SetConfig
//
// Set the connectivity interface to the selected RS-232 operating
// configuration.
// Parameters:
//            handle          device handle from PmicConvityOpen() call
//            cfgInternal     RS-232 transceiver internal connections
//            cfgExternal     RS-232 transceiver external connections
//            txTristated     RS-232 transceiver TX state
// Returns:
//           PMIC_SUCCESS     if the requested mode was set
//------------------------------------------------------------------------------
PMIC_STATUS PmicConvityRs232SetConfig(const PMIC_CONVITY_HANDLE         handle,
                                      const PMIC_CONVITY_RS232_INTERNAL cfgInternal,
                                      const PMIC_CONVITY_RS232_EXTERNAL cfgExternal,
                                      const BOOL                        txTristated)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( cfgInternal );
    UNREFERENCED_PARAMETER( cfgExternal );
    UNREFERENCED_PARAMETER( txTristated );

    ENTRY_MSG;
    EXIT_MSG;
    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
// Function: PmicConvityRs232GetConfig
//
// Get the connectivity interface's current RS-232 operating configuration.
// Parameters:
//            handle          device handle from PmicConvityOpen() call
//            cfgInternal     RS-232 transceiver internal connections
//            cfgExternal     RS-232 transceiver external connections
//            txTristated     RS-232 transceiver TX state
// Returns:
//           PMIC_SUCCESS     if the requested mode was retrieved
//------------------------------------------------------------------------------
PMIC_STATUS PmicConvityRs232GetConfig(const PMIC_CONVITY_HANDLE          handle,
                                      PMIC_CONVITY_RS232_INTERNAL *const cfgInternal,
                                      PMIC_CONVITY_RS232_EXTERNAL *const cfgExternal,
                                      BOOL                        *const txTristated)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( cfgInternal );
    UNREFERENCED_PARAMETER( cfgExternal );
    UNREFERENCED_PARAMETER( txTristated );

    ENTRY_MSG;
    EXIT_MSG;
    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
// Function: PmicConvityCea936SetDetectionConfig
//
// Set-up the circuitry helping in accessory type identification.
// Parameters:
//            handle          device handle from PmicConvityOpen() call
//            cfg             detection related circuitry set-up,
//                            bit-wise OR allowed.
// Returns:
//           PMIC_SUCCESS     if the requested set-up was done successfully.
//------------------------------------------------------------------------------
PMIC_STATUS PmicConvityCea936SetDetectionConfig(const PMIC_CONVITY_HANDLE handle,
                                 const PMIC_CONVITY_CEA936_DETECTION_CONFIG  cfg)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( cfg );

    ENTRY_MSG;
    EXIT_MSG;
    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
// Function: PmicConvityCea936GetDetectionConfig
//
// Get the current set-up of circuitry helping in accessory type identification.
// Parameters:
//            handle          device handle from PmicConvityOpen() call
//            cfg             detection related circuitry set-up,
//                            bit-wise OR allowed.
// Returns:
//           PMIC_SUCCESS     if the requested set-up was successfully retrieved.
//------------------------------------------------------------------------------
PMIC_STATUS PmicConvityCea936GetDetectionConfig(const PMIC_CONVITY_HANDLE handle,
                                 PMIC_CONVITY_CEA936_DETECTION_CONFIG *const cfg)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( cfg );

    ENTRY_MSG;
    EXIT_MSG;
    return PMIC_NOT_SUPPORTED;
}

//------------------------------------------------------------------------------
// Function: PmicConvityCea936ExitSignal
//
// Signal the attached device to exit the current CEA-936 operating mode.
// Returns an error if the current operating mode is not CEA-936.
// Parameters:
//            handle          device handle from PmicConvityOpen() call
//            signal          type of exit signal to be sent
// Returns:
//           PMIC_SUCCESS     if exit signal was sent
//------------------------------------------------------------------------------
PMIC_STATUS PmicConvityCea936ExitSignal(const PMIC_CONVITY_HANDLE        handle,
                                   const PMIC_CONVITY_CEA936_EXIT_SIGNAL signal)
{
    UNREFERENCED_PARAMETER( handle );
    UNREFERENCED_PARAMETER( signal );

    ENTRY_MSG;
    EXIT_MSG;
    return PMIC_NOT_SUPPORTED;
}

