/*******************************************************************************
 *
 * Copyright (c) 2007 Wolfson Microelectronics plc.  All rights reserved.
 *
 * This software as well as any related documentation may only be used or
 * copied in accordance with the terms of the Wolfson Microelectronics plc
 * agreement(s) (e.g. licence or non-disclosure agreement (NDA)).
 *
 * The information in this file is furnished for informational use only,
 * is subject to change without notice, and should not be construed as a
 * commitment by Wolfson Microelectronics plc.  Wolfson Microelectronics plc
 * assumes no responsibility or liability for any errors or inaccuracies that
 * may appear in the software or any related documention.
 *
 * Except as permitted by the agreement(s), no part of the software or any
 * related documention may be reproduced, stored in a retrieval system, or
 * transmitted in any form or by any means without the express written
 * consent of Wolfson Microelectronics plc.
 *                                                                         *//**
 * $Id: WMDevice.h 640 2007-06-15 22:01:17Z ib $
 *
 * This file contains platform-independent routines for communicating with
 * Wolfson codecs.
 *
 * Warning:
 *  This driver is specifically written for Wolfson Codecs. It is not a
 *  general CODEC device driver.
 *
 *---------------------------------------------------------------------------*/
#ifndef __WMDEVICE_H__
#define __WMDEVICE_H__

/*
 * Include files
 */
#include "WMTypes.h"

/*
 * Definitions
 */

/*
 * Function prototypes
 */
#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------------
 * Function:    WMOpenDevice
 *
 * Initialises the use of the Wolfson device by a given driver.
 *
 * Parameters:
 *        driverId            The driver ID (e.g. WM_DRIVER_TOUCH)
 *      phDevice            A variable to receive the handle to the device
 *
 * Returns:     WMSTATUS
 *        See WMStatus.h.
 *---------------------------------------------------------------------------*/
WM_STATUS WMOpenDevice( WM_DRIVER_ID       driverID,
                        WM_DEVICE_HANDLE   *phDevice
                      );

/*-----------------------------------------------------------------------------
 * Function:    WMAudioOpenDevice
 *
 * Initialises the use of the Wolfson device by a given audio driver.
 *
 * Parameters:
 *      phDevice            A variable to receive the handle to the device
 *
 * Returns:     WMSTATUS
 *      See WMStatus.h.
 *---------------------------------------------------------------------------*/
#define WMAudioOpenDevice( phDevice )                            \
                WMOpenDevice( WM_DRIVER_AUDIO, phDevice )

/*-----------------------------------------------------------------------------
 * Function:    WMTouchOpenDevice
 *
 * Initialises the use of the Wolfson device by a given touch driver.
 *
 * Parameters:
 *      phDevice            A variable to receive the handle to the device
 *
 * Returns:     WMSTATUS
 *      See WMStatus.h.
 *---------------------------------------------------------------------------*/
#define WMTouchOpenDevice( phDevice )                            \
                WMOpenDevice( WM_DRIVER_TOUCH, phDevice )

/*-----------------------------------------------------------------------------
 * Function:    WMBatteryOpenDevice
 *
 * Initialises the use of the Wolfson device by a given battery driver.
 *
 * Parameters:
 *      phDevice            A variable to receive the handle to the device
 *
 * Returns:     WMSTATUS
 *      See WMStatus.h.
 *---------------------------------------------------------------------------*/
#define WMBatteryOpenDevice( phDevice )                            \
                WMOpenDevice( WM_DRIVER_BATTERY, phDevice )

/*-----------------------------------------------------------------------------
 * Function:    WMAuxADCOpenDevice
 *
 * Initialises the use of the Wolfson device by a given Auxiliary ADC driver.
 *
 * Parameters:
 *      phDevice            A variable to receive the handle to the device
 *
 * Returns:     WMSTATUS
 *      See WMStatus.h.
 *---------------------------------------------------------------------------*/
#define WMAuxADCOpenDevice( phDevice )                            \
                WMOpenDevice( WM_DRIVER_AUXADC, phDevice )
/*-----------------------------------------------------------------------------
 * Function:    WMPmicOpenDevice
 *
 * Initialises the use of the Wolfson device by a given PMIC driver.
 *
 * Parameters:
 *      phDevice            A variable to receive the handle to the device
 *
 * Returns:     WMSTATUS
 *      See WMStatus.h.
 *---------------------------------------------------------------------------*/
#define WMPmicOpenDevice( phDevice )                            \
                WMOpenDevice( WM_DRIVER_PMIC, phDevice )

/*-----------------------------------------------------------------------------
 * Function:    WMCloseDevice
 *
 * Marks the end of use of the library by this driver.
 *
 * Parameters:
 *      hDevice             handle to the device (from WMOpenDevice)
 *      driverId            The driver ID (e.g. WM_DRIVER_TOUCH)
 *
 * Returns:     void
 *---------------------------------------------------------------------------*/
void WMCloseDevice( WM_DEVICE_HANDLE hDevice, WM_DRIVER_ID driverID );

/*-----------------------------------------------------------------------------
 * Function:    WMAudioCloseDevice
 *
 * Marks the end of use of the library by the given audio driver.
 *
 * Parameters:
 *      hDevice             handle to the device (from WMOpenDevice)
 *
 * Returns:     WMSTATUS
 *      See WMStatus.h.
 *---------------------------------------------------------------------------*/
#define WMAudioCloseDevice( hDevice )                            \
                WMCloseDevice( hDevice, WM_DRIVER_AUDIO )

/*-----------------------------------------------------------------------------
 * Function:    WMPmicCloseDevice
 *
 * Marks the end of use of the library by the given PMIC driver.
 *
 * Parameters:
 *      hDevice             handle to the device (from WMOpenDevice)
 *
 * Returns:     WMSTATUS
 *      See WMStatus.h.
 *---------------------------------------------------------------------------*/
#define WMPmicCloseDevice( hDevice )                            \
                WMCloseDevice( hDevice, WM_DRIVER_PMIC )

/*-----------------------------------------------------------------------------
 * Function:    WMInitDeviceId
 *
 * Initialise our device type information.
 *
 * Parameters:
 *      hDevice     handle to the device (from WMOpenDevice)
 *
 * Returns:     WMSTATUS
 *      See WMStatus.h
 *---------------------------------------------------------------------------*/
WM_STATUS WMInitDeviceId( WM_DEVICE_HANDLE hDevice );

/*******************************************************************************
 * Function: WMGetDeviceType                                               *//**
 *
 * @brief   Returns the chip type.
 *
 * @param   hDevice     handle to the device (from WMOpenDevice)
 * 
 * @return  The device type, or WM_CHIP_UNKNOWN if unknown.
 ******************************************************************************/
WM_CHIPTYPE WMGetDeviceType( WM_DEVICE_HANDLE  hDevice );

/*******************************************************************************
 * Function: WMGetDeviceRev                                                *//**
 *
 * @brief   Returns the chip revision.
 *
 * @param   hDevice     handle to the device (from WMOpenDevice)
 * 
 * @return  The device revision, or WM_REV_UNKNOWN if unknown.
 ******************************************************************************/
WM_CHIPREV WMGetDeviceRev( WM_DEVICE_HANDLE  hDevice );

/*-----------------------------------------------------------------------------
 * Function:    WMEnterSleep
 *
 * Enters sleep mode - powers down the device and does any state preservation
 * needed for restoring later.
 *
 * Parameters:
 *      hDevice     handle to the device (from WMOpenDevice)
 *
 * Returns:     WMSTATUS
 *      See WMStatus.h.
 *---------------------------------------------------------------------------*/
WM_STATUS WMEnterSleep( WM_DEVICE_HANDLE hDevice );

/*-----------------------------------------------------------------------------
 * Function:    WMLeaveSleep
 *
 * Leaves sleep mode - powers the device back on again and restores any
 * settings which couldn't be preserved.
 *
 * Parameters:
 *      hDevice     handle to the device (from WMOpenDevice)
 *
 * Returns:     WMSTATUS
 *      See WMStatus.h.
 *---------------------------------------------------------------------------*/
WM_STATUS WMLeaveSleep( WM_DEVICE_HANDLE hDevice );

/*-----------------------------------------------------------------------------
 * Function:    WMRewriteShadowRegisters
 *
 * Writes the current shadow register values back to those registers.
 *
 * Parameters:
 *      hDevice             handle to the device (from WMOpenDevice)
 *
 * Returns:     WMSTATUS
 *        See WMStatus.h.
 *---------------------------------------------------------------------------*/
//WMSTATUS WMRewriteShadowRegisters( WM_DEVICE_HANDLE hDevice );

/*-----------------------------------------------------------------------------
 * Function:    WMPreventSystemCalls
 *
 * Tells this module whether we can allow system calls.
 * Note this is re-entrant - you must allow as many times as you prevent.
 * Note this assumes that it is called under the power manager and that the
 * power manager is single-threaded. Therefore calls to WMPreventSystemCalls
 * and WMSystemCallsAllowed will not interfere with themselves/each other.
 *
 * Parameters:
 *      hDevice     handle to the device (from WMOpenDevice)
 *      prevent     Whether to prevent or allow.
 *
 * Returns:     void
 *---------------------------------------------------------------------------*/
void WMPreventSystemCalls( WM_DEVICE_HANDLE hDevice, BOOL prevent );

/*-----------------------------------------------------------------------------
 * Function:    WMSystemCallsAllowed
 *
 * Tells this module whether we can make system calls.
 * Note this assumes that it is called under the power manager and that the
 * power manager is single-threaded. Therefore calls to WMPreventSystemCalls
 * and WMSystemCallsAllowed will not interfere with themselves/each other.
 *
 * Parameters:
 *      hDevice             handle to the device (from WMOpenDevice)
 *
 * Returns:     WM_BOOL
 *      Whether the driver has been told to allow system calls.
 *---------------------------------------------------------------------------*/
BOOL WMSystemCallsAllowed( WM_DEVICE_HANDLE hDevice );

/*-----------------------------------------------------------------------------
 * Function:    WMBeNice
 *
 * Allows other tasks to continue in a way which is safe for the power handler.
 *
 * Parameters:
 *      hDevice             handle to the device (from WMOpenDevice)
 *
 * Returns:     void
 *---------------------------------------------------------------------------*/
void WMBeNice( WM_DEVICE_HANDLE hDevice );

#ifdef __cplusplus
}   /* extern "C" */
#endif

#endif    /* __WMDEVICE_H__ */
/*------------------------------ END OF FILE ---------------------------------*/
