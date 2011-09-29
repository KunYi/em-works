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
 * may appear in the software or any related documentation.
 *
 * Except as permitted by the agreement(s), no part of the software or any
 * related documentation may be reproduced, stored in a retrieval system, or
 * transmitted in any form or by any means without the express written
 * consent of Wolfson Microelectronics plc.
 *                                                                         *//**
 * @file   WMControlLink.h
 * @brief  Interface between the DSP and the Wolfson device.
 *
 * @version $Id: WMControlLink.h 667 2007-06-21 14:48:36Z ib $
 *
 * @Warning
 *   This software is specifically written for Wolfson devices. It may not be
 *   used with other devices.
 *
 ******************************************************************************/
#ifndef __WMCONTROLLINK_H__
#define __WMCONTROLLINK_H__

/*
 * Include files
 */
#include "WMStatus.h"
#include "WMTypes.h"

/*
 * Definitions
 */

/**
 * Interfaces which can be selected.
 */
typedef enum WM_CONTROL_IF
{
    WM_CTRL_2WIRE,
    WM_CTRL_ALT_2WIRE,
    WM_CTRL_3WIRE,
    WM_CTRL_4WIRE
} WM_CONTROL_IF;

/*
 * Debugging
 */
/* Dummy for now - need to implement properly at some point */
#ifdef DEBUG
#   define WM_TRACE( _hDevice, args )  do                                      \
    {                                                                          \
        if ( WMSystemCallsAllowed( _hDevice ) )                                \
            WMTrace args;                                                      \
    } while ( 0 )
#   define WM_ASSERT( _hDevice, _cond ) if (!(_cond))                          \
    {                                                                          \
        WMDebugAssert( _hDevice, _T(__FILE__), __LINE__, _T(#_cond) );         \
    }
#else /* of DEBUG */
#   define WM_TRACE( hDevice, args ) 
#   define WM_ASSERT( hDevice, _cond )
#endif

/*
 * Function prototypes
 */
#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Function: WMLinkInit                                                    *//**
 *
 * @brief   Initialises the Wolfson device comms.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 *
 * @retval  WMS_SUCCESS          succeeded
 * @retval  WMS_RESOURCE_FAIL    couldn't allocate memory
 * @retval  WMS_HW_ERROR         error communicating with device
 ******************************************************************************/
WM_STATUS WMLinkInit( WM_DEVICE_HANDLE hDevice );

/*******************************************************************************
 * Function: WMLinkShutdown                                                *//**
 *
 * @brief   Cleans up the Wolfson device comms.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 *
 * @return void
 ******************************************************************************/
void WMLinkShutdown( WM_DEVICE_HANDLE hDevice );

/*******************************************************************************
 * Function:  WMRead                                                       *//**
 *
 * @brief   Reads the value from a register on the device.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   reg       Register index to read.
 * @param   pValue    Variable to receive the value.
 *
 * @retval  WMS_SUCCESS       succeeded
 * @retval  WMS_HW_ERROR      error communicating with device
 ******************************************************************************/
WM_STATUS WMRead( WM_DEVICE_HANDLE hDevice, WM_REG reg, WM_REGVAL *pValue );

/*******************************************************************************
 * Function:  WMWrite                                                      *//**
 *
 * @brief   Reads the value from a register on the device.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   reg       Register index to read.
 * @param   value     Value to write to the register.
 *
 * @retval  WMS_SUCCESS       succeeded
 * @retval  WMS_HW_ERROR      error communicating with device
 ******************************************************************************/
WM_STATUS WMWrite( WM_DEVICE_HANDLE hDevice, WM_REG reg, WM_REGVAL value );

/*******************************************************************************
 * Function:  WMSetField                                                   *//**
 *
 * @brief   Masked write of a value to a field of a register on the device.
 *
 * This function writes the given value to a field, doing a read-modify-write
 * cycle.  The value passed in should be the value in the register (all shifting
 * should already be done).
 *
 * For example, to set the field [10:8] of WM1234_FOO to 5, make the call:
 *
 *      WMSetField( hDevice, WM1234_FOO, 0x0500, 0x0700 );
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   reg       Register index to read.
 * @param   value     Value to write to the register.
 * @param   mask      Field bitmask.
 *
 * @retval  WMS_SUCCESS       succeeded
 * @retval  WMS_HW_ERROR      error communicating with device
 ******************************************************************************/
WM_STATUS WMSetField( WM_DEVICE_HANDLE  hDevice,
                      WM_REG            reg,
                      WM_REGVAL         value,
                      WM_REGVAL         mask
                    );

/*******************************************************************************
 * Function:  WMClearField                                                 *//**
 *
 * @brief   Clears the field of a register on the device.
 *
 * This function writes 0 to a field, doing a read-modify-write cycle.
 *
 * For example, to set the field [10:8] of WM1234_FOO to 0, make the call:
 *
 *      WMClearField( hDevice, WM1234_FOO, 0x0700 );
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   reg       Register index to read.
 * @param   mask      Field bitmask.
 *
 * @retval  WMS_SUCCESS       succeeded
 * @retval  WMS_HW_ERROR      error communicating with device
 ******************************************************************************/
WM_STATUS WMClearField( WM_DEVICE_HANDLE hDevice, WM_REG reg, WM_REGVAL mask );

/*******************************************************************************
 * Function: WMUnprotectRegs                                               *//**
 *
 * @brief  Unlocks the protected registers on the device.
 *
 * (no parameters)
 * 
 * @retval WMS_SUCCESS          succeeded
 * @retval WMS_HW_ERROR         error communicating with device
 ******************************************************************************/
WM_STATUS WMUnprotectRegs( WM_DEVICE_HANDLE hDevice );

/*******************************************************************************
 * Function: WMProtectRegs                                                 *//**
 *
 * @brief  Re-locks the protected registers on the device.
 *
 * (no parameters)
 * 
 * @retval WMS_SUCCESS          succeeded
 * @retval WMS_HW_ERROR         error communicating with device
 ******************************************************************************/
WM_STATUS WMProtectRegs( WM_DEVICE_HANDLE hDevice );

/*******************************************************************************
 * Function:  WMDumpRegs                                                   *//**
 *
 * @brief   Dumps the current values of all registers on the device.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 *
 * @retval  WMS_SUCCESS       succeeded
 * @retval  WMS_HW_ERROR      error communicating with device
 ******************************************************************************/
WM_STATUS WMDumpRegs( WM_DEVICE_HANDLE hDevice );

/*******************************************************************************
 * Function: WMPrint                                                       *//**
 *
 * @brief  Prints out a debug message.
 *
 * This function is implemented within the platform layer to do the platform-
 * specific output.  It is used by the other output functions such as WMTrace.
 *
 * @param  message  message to print
 *
 * @return void
 ******************************************************************************/
void WMPrint( const TCHAR *message );

/*******************************************************************************
 * Function: WMTrace                                                       *//**
 *
 * @brief  Prints out a debug message.
 *
 * @param  format   printf format string
 * @param  ...      printf args
 *
 * @return void
 ******************************************************************************/
void WMTrace( const TCHAR *format, ... );

/*******************************************************************************
 * Function: MicroSleep                                                    *//**
 *
 * @brief   Sleeps for the given number of microseconds, yielding if possible.
 *
 * @param   hDevice         handle to the device (from WMOpenDevice)
 * @param   microSeconds    The number of microseconds to sleep.
 * 
 * @return  void
 ******************************************************************************/
void MicroSleep( WM_DEVICE_HANDLE hDevice, unsigned microSeconds );

/*******************************************************************************
 * Function: MilliSleep                                                    *//**
 *
 * @brief   Sleeps for the given number of milliseconds, yielding if possible.
 *
 * @param   hDevice         handle to the device (from WMOpenDevice)
 * @param   milliSeconds    The number of microseconds to sleep.
 * 
 * @return  void
 ******************************************************************************/
void MilliSleep( WM_DEVICE_HANDLE hDevice, unsigned milliSeconds );

#if defined(DEBUG) || WM_TESTING
/*******************************************************************************
 * Function: WMDebugAssert                                                 *//**
 *
 * @brief   Outputs error message and breaks.
 *
 * @param   hDevice handle to the device (from WMOpenDevice), or NULL if
 *              not available.
 * @param   file    Source file of assertion (from __FILE__)
 * @param   line    Source line of assertion (from __LINE__)
 * @param   text    The textual form of the assertion.
 * 
 * @return  void
 ******************************************************************************/
void WMDebugAssert( WM_DEVICE_HANDLE    hDevice,
                    const TCHAR         *file,
                    int                 line,
                    const TCHAR         *text
                  );

/*******************************************************************************
 * Function: WMDebugBreakpoint                                             *//**
 *
 * @brief  Puts a breakpoint into the code.
 *
 * (no parameters)
 * 
 * @return  void
 ******************************************************************************/
void WMDebugBreakpoint();

#endif /* DEBUG || WM_TESTING */

#ifdef __cplusplus
}   /* extern "C" */
#endif

#endif    // __WMCONTROLLINK_H__
/*------------------------------ END OF FILE ---------------------------------*/
