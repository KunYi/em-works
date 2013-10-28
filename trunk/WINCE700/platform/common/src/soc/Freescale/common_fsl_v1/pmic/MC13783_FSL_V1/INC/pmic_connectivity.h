//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
/*---------------------------------------------------------------------------
* Copyright (C) 2006, Freescale Semiconductor, Inc. All Rights Reserved.
* THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
* AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT 
*--------------------------------------------------------------------------*/
//
//  File:  pmic_connectivity.h
//
//  This file contains the PMIC Connectivity SDK interface that is used by 
//  applications and other drivers to access registers of the PMIC.
//
//-----------------------------------------------------------------------------

#ifndef __PMIC_CONNECTIVITY_H__
#define __PMIC_CONNECTIVITY_H__

#include "pmic_basic_types.h"

#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------------------------
// External Functions

//-----------------------------------------------------------------------------
// External Variables

//-----------------------------------------------------------------------------
// Types

// General data types

// handle for PMIC access
typedef long PMIC_CONVITY_HANDLE;

// Callback function prototype
typedef void (*PMIC_CONVITY_CALLBACK)(const PMIC_CONVITY_EVENTS);

// Operating modes of connectivity interface
// assigned values based on the MC13783 spec, they are important
typedef enum {
    USB = 0,
    RS232 = 1,
    CEA936_MONO = 4,
    CEA936_STEREO = 5,
    CEA936_TEST_RIGHT = 6,
    CEA936_TEST_LEFT = 7
} PMIC_CONVITY_MODE;

// Connectivity interface events. Bitwise or-able
// note that all sense bits are dual, except except CKDETS which is low-to-high
typedef enum {
    USB_DETECT_4V4_RISE         = 1,    // USB4V4S 0 -> 1 transition
    USB_DETECT_4V4_FALL         = 2,    // USB4V4S 1 -> 0 transition
    USB_DETECT_2V0_RISE         = 4,    // USB2V0S 0 -> 1 transition
    USB_DETECT_2V0_FALL         = 8,    // USB2V0S 1 -> 0 transition
    USB_DETECT_0V8_RISE         = 16,   // USB0V8S 0 -> 1 transition
    USB_DETECT_0V8_FALL         = 32,   // USB0V8S 1 -> 0 transition
    USB_DETECT_MINI_A           = 64,   // IDFLOATS -> 0, IDGNDS -> 1
    USB_DETECT_MINI_B           = 128,  // IDFLOATS -> 1, IDGNDS -> 0
    USB_DETECT_NON_USB_ACCESORY = 256,  // IDFLOATS -> 0, IDGNDS -> 0
    USB_DETECT_FACTORY_MODE     = 512,  // IDFLOATS -> 1, IDGNDS -> 1
    USB_DETECT_SE1_RISE         = 1024, // SE1S 0 -> 1 transition
    USB_DETECT_SE1_FALL         = 2048, // SE1S 1 -> 0 transition
    USB_DETECT_CKDETECT         = 4096  // car kit detection (CKDETS 0 -> 1 transition)
} PMIC_CONVITY_EVENTS;


// USB mode specific data types
// USB transceiver speed
typedef enum {
    USB_LOW_SPEED,
    USB_FULL_SPEED,
    USB_HIGH_SPEED
} PMIC_CONVITY_USB_SPEED;

// Modes as USB transceiver
typedef enum {
    USB_HOST,
    USB_PERIPHERAL
} PMIC_CONVITY_USB_MODE;

// input source for USB transceiver's power regulator
typedef enum {
    USB_POWER_VBUS = 1,
    USB_POWER_INTERNAL = 2,
    USB_POWER_INTERNAL_BOOST = 0
} PMIC_CONVITY_USB_POWER_IN;

// output voltage of USB transceiver's power regulator
typedef enum {
    USB_POWER_2V775 = 0,
    USB_POWER_3V3
} PMIC_CONVITY_USB_POWER_OUT;

// USB transceiver operating mode
typedef enum {
    USB_TRANSCEIVER_OFF,
    USB_SINGLE_ENDED_UNIDIR_TX,
    USB_SINGLE_ENDED_UNIDIR_RX,
    USB_SINGLE_ENDED_BIDIR_TX,
    USB_SINGLE_ENDED_BIDIR_RX,
    USB_SINGLE_ENDED_LOW,
    USB_DIFFERENTIAL_UNIDIR_TX,
    USB_DIFFERENTIAL_UNIDIR_RX,
    USB_DIFFERENTIAL_BIDIR_TX,
    USB_DIFFERENTIAL_BIDIR_RX,
    USB_SUSPEND_ON,
    USB_SUSPEND_OFF,
    USB_OTG_SRP_DLP_START,
    USB_OTG_SRP_DLP_STOP
} PMIC_CONVITY_USB_TRANSCEIVER_MODE;

// USB on-the-go configuration. Bitwise or-able
typedef enum {
    USB_OTG_SE0CONN                 = 1,
    USB_OTG_DLP_SRP                 = 2,
    USB_PULL_OVERRIDE               = 4,
    USB_VBUS_CURRENT_LIMIT_HIGH     = 8,
    USB_VBUS_CURRENT_LIMIT_LOW      = 16,
    USB_VBUS_CURRENT_LIMIT_LOW_10MS = 32,
    USB_VBUS_CURRENT_LIMIT_LOW_20MS = 64,
    USB_VBUS_CURRENT_LIMIT_LOW_30MS = 128,
    USB_VBUS_CURRENT_LIMIT_LOW_40MS = 256,
    USB_VBUS_CURRENT_LIMIT_LOW_50MS = 512,
    USB_VBUS_CURRENT_LIMIT_LOW_60MS = 1024,
    USB_VBUS_PULLDOWN               = 2048
} PMIC_CONVITY_USB_OTG_CONFIG;

// USB device type, matching the connector connected to the receptacle.
typedef enum {
    USB_A_DEVICE,
    USB_B_DEVICE
} PMIC_CONVITY_USB_DEVICE_TYPE;

// RS-232 mode specific data types
// RS-232 transceiver external connections
typedef enum {
    RS232_TX_UDM_RX_UDP,
    RS232_TX_UDP_RX_UDM,
    RS232_TX_RX_EXTERNAL_DEFAULT
} PMIC_CONVITY_RS232_EXTERNAL;

// RS-232 transceiver internal connections
typedef enum {
    RS232_TX_USE0VM_RX_UDATVP,
    RS232_TX_UDATVP_RX_URXVM,
    RS232_TX_UTXDI_RX_URXDO,
    RS232_TX_RX_INTERNAL_DEFAULT
} PMIC_CONVITY_RS232_INTERNAL;

// CEA-936 mode specific data types
// Accessory detection config. Bitwise or-able
typedef enum {
    ACCESSORY_ID_ID100KPU = 1,
    ACCESSORY_ID_IDPUCNTRL = 2,
    ACCESSORY_ID_DP150KPU = 4
} PMIC_CONVITY_CEA936_DETECTION_CONFIG;

// CEA-936 mode exit signals
typedef enum {
    CEA936_UID_NO_PULLDOWN,
    CEA936_UID_PULLDOWN_6MS,
    CEA936_UID_PULLDOWN,
    CEA936_UDMPULSE
} PMIC_CONVITY_CEA936_EXIT_SIGNAL;


// Function Prototypes

// open and gain exclusive access to the PMIC connectivity hardware
PMIC_STATUS PmicConvityOpen (PMIC_CONVITY_HANDLE *const handle, 
                             const PMIC_CONVITY_MODE mode);

// Terminates further access to PMIC connectivity hardware
PMIC_STATUS PmicConvityClose (const PMIC_CONVITY_HANDLE handle);

// Change the current operating mode of the PMIC Connectivity hardware. 
// The available connectivity operating modes are hardware dependent. 
PMIC_STATUS PmicConvitySetMode (const PMIC_CONVITY_HANDLE handle, 
                                const PMIC_CONVITY_MODE mode);

// This function gets the PMIC connectivity hardware's current operating mode.
PMIC_STATUS PmicConvityGetMode (const PMIC_CONVITY_HANDLE handle, 
                                PMIC_CONVITY_MODE *const mode);

// This function resets all the hardware registers to the initial power-on/reset state.
PMIC_STATUS PmicConvityReset (const PMIC_CONVITY_HANDLE handle);

// This function is used to register callback for receiving notifications 
// related to connectivity interface related events.  
PMIC_STATUS PmicConvitySetCallback (const PMIC_CONVITY_HANDLE handle, 
                                    const PMIC_CONVITY_CALLBACK func, 
                                    const PMIC_CONVITY_EVENTS eventMask);

// This function deregisters the callback that was previously registered using PmicConvitySetCallback().
PMIC_STATUS PmicConvityClearCallback (const PMIC_CONVITY_HANDLE handle);

// This function returns the callback function and event mask.
PMIC_STATUS PmicConvityGetCallback (const PMIC_CONVITY_HANDLE handle, 
                                    PMIC_CONVITY_CALLBACK *const func, 
                                    PMIC_CONVITY_EVENTS *const eventMask);

// This function get the Event satus
PMIC_STATUS PmicConvityGetEventStatus(const PMIC_CONVITY_HANDLE handle,
                                      PMIC_CONVITY_EVENTS *const eventSet);

// This function sets the USB transceiver speed.
PMIC_STATUS PmicConvityUsbSetSpeed (const PMIC_CONVITY_HANDLE handle, 
                                    const PMIC_CONVITY_USB_SPEED speed, 
                                    const PMIC_CONVITY_USB_MODE  mode);

// This function gets the USB transceiver speed.
PMIC_STATUS PmicConvityUsbGetSpeed (const PMIC_CONVITY_HANDLE handle, 
                                    PMIC_CONVITY_USB_SPEED *const speed, 
                                    PMIC_CONVITY_USB_MODE *const mode);

// This function provides settings for the USB transceiver's power supply.
PMIC_STATUS PmicConvityUsbSetPowerSource (const PMIC_CONVITY_HANDLE handle, 
                                          const PMIC_CONVITY_USB_POWER_IN pwrin, 
                                          const PMIC_CONVITY_USB_POWER_OUT pwrout);

// This function gets settings for the USB transceiver's power supply.
PMIC_STATUS PmicConvityUsbGetPowerSource (const PMIC_CONVITY_HANDLE handle, 
                                          PMIC_CONVITY_USB_POWER_IN *const pwrin, 
                                          PMIC_CONVITY_USB_POWER_OUT *const pwrout);

// This function sets the USB transceiver's operating mode.
PMIC_STATUS PmicConvityUsbSetXcvr (const PMIC_CONVITY_HANDLE handle, 
                                   const PMIC_CONVITY_USB_TRANSCEIVER_MODE mode);

// This function gets the USB transceiver's operating mode.
PMIC_STATUS PmicConvityUsbGetXcvr (const PMIC_CONVITY_HANDLE handle, 
                                   PMIC_CONVITY_USB_TRANSCEIVER_MODE *const mode);

// This function sets the data line pulsing duration for USB OTG session request protocol.
PMIC_STATUS PmicConvityUsbOtgSetDlpDuration (const PMIC_CONVITY_HANDLE handle, 
                                             const unsigned int duration);

// This function gets the data line pulsing duration for USB OTG session request protocol. 
PMIC_STATUS PmicConvityUsbOtgGetDlpDuration (const PMIC_CONVITY_HANDLE handle, 
                                             unsigned int *const duration);

// Explicitly start HNP by setting up D+ pull-up on A device and disconnecting
// all pull-up and pull-down on B device.
PMIC_STATUS PmicConvityUsbOtgBeginHnp(const PMIC_CONVITY_HANDLE handle, 
                                      const PMIC_CONVITY_USB_DEVICE_TYPE type);

// Explicitly end the HNP by disconnecting pull-up on A device and enabling pull-up
// on B device.
PMIC_STATUS PmicConvityUsbOtgEndHnp(const PMIC_CONVITY_HANDLE handle, 
                                    const PMIC_CONVITY_USB_DEVICE_TYPE type);

// This function sets the USB On-The-Go configuration. Multiple configuration 
// settings may be OR-ed together. Selecting conflicting settings (e.g., 
// multiple VBUS current limits) will result in undefined behavior.
PMIC_STATUS PmicConvityUsbOtgSetConfig (const PMIC_CONVITY_HANDLE handle, 
                                        const PMIC_CONVITY_USB_OTG_CONFIG cfg);

// This function clears the USB On-The-Go configuration. Multiple configuration 
// settings may be OR-ed together. Selecting conflicting settings (e.g., 
// multiple VBUS current limits) will result in undefined behavior.
PMIC_STATUS PmicConvityUsbOtgClearConfig (const PMIC_CONVITY_HANDLE handle, 
                                          const PMIC_CONVITY_USB_OTG_CONFIG cfg);

// This function gets the USB On-The-Go configuration.
PMIC_STATUS PmicConvityUsbOtgGetConfig (const PMIC_CONVITY_HANDLE handle, 
                                        PMIC_CONVITY_USB_OTG_CONFIG *const cfg);

// This function sets the connectivity interface to the selected RS-232 operating mode. 
PMIC_STATUS PmicConvityRs232SetConfig (const PMIC_CONVITY_HANDLE handle, 
                                       const PMIC_CONVITY_RS232_INTERNAL cfgInternal, 
                                       const PMIC_CONVITY_RS232_EXTERNAL cfgExternal,
                                       const BOOL txTristated);

// Get the connectivity interface's current RS-232 operating mode.
PMIC_STATUS PmicConvityRs232GetConfig (const PMIC_CONVITY_HANDLE handle, 
                                       PMIC_CONVITY_RS232_INTERNAL *const cfgInternal, 
                                       PMIC_CONVITY_RS232_EXTERNAL *const cfgExternal,
                                       BOOL *const txTristated);

// This function sets the circuitry for accessory type identification.
PMIC_STATUS PmicConvityCea936SetDetectionConfig (const PMIC_CONVITY_HANDLE handle, 
                                                 const PMIC_CONVITY_CEA936_DETECTION_CONFIG cfg);

// This function gets the current circuitry set-up for accessory type identification.
PMIC_STATUS PmicConvityCea936GetDetectionConfig (const PMIC_CONVITY_HANDLE handle,
                                                 PMIC_CONVITY_CEA936_DETECTION_CONFIG *const cfg);

// This function is used to signal the accessory to exit the audio mode.
PMIC_STATUS PmicConvityCea936ExitSignal (const PMIC_CONVITY_HANDLE handle, 
                                         const PMIC_CONVITY_CEA936_EXIT_SIGNAL signal);

#ifdef __cplusplus
}
#endif

#endif // __PMIC_CONNECTIVITY_H__
