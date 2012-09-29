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
 * @file   WMPmicRTC.h
 * @brief  Real-time clock functions for Wolfson PMICs.
 *
 * @version $Id: WMPmicRTC.h 483 2007-05-04 19:02:20Z ib $
 *
 * @Warning
 *   This software is specifically written for Wolfson devices. It may not be
 *   used with other devices.
 *
 ******************************************************************************/
#ifndef __WMPMICRTC_H__
#define __WMPMICRTC_H__

/*
 * Include files
 */
#include <WMStatus.h>

/*
 * Definitions
 */
#define WM_ALARM_DONT_CARE              ((WORD)-1)

/*
 * Function prototypes
 */
#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Function: WMPmicGetTime                                                 *//**
 *
 * @brief   Returns the current time.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   pTime           variable to receive the time
 * 
 * @retval  WMS_SUCCESS          succeeded
 * @retval  WMS_HW_ERROR         error communicating with device
 ******************************************************************************/
WM_STATUS WMPmicGetTime( WM_DEVICE_HANDLE hDevice, SYSTEMTIME *pTime );

/*******************************************************************************
 * Function: WMPmicGetAlarm                                                *//**
 *
 * @brief   Returns the current alarm time.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   pTime           variable to receive the time
 *
 * @retval  WMS_SUCCESS          succeeded
 * @retval  WMS_HW_ERROR         error communicating with device
 ******************************************************************************/
WM_STATUS WMPmicGetAlarm( WM_DEVICE_HANDLE hDevice, SYSTEMTIME *pTime );

/*******************************************************************************
 * Function: WMPmicSetTime                                                 *//**
 *
 * @brief   Sets the time to the given time.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   pTime           The time to set to.
 * 
 * @retval  WMS_SUCCESS          succeeded
 * @retval  WMS_HW_ERROR         error communicating with device
 ******************************************************************************/
WM_STATUS WMPmicSetTime( WM_DEVICE_HANDLE hDevice, const SYSTEMTIME *pTime );

/*******************************************************************************
 * Function: WMPmicSetAlarm                                                *//**
 *
 * @brief   Sets the alarm to the given time.
 *
 * Time fields set to -1 indicate "don't care".  Years are not supported.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   pTime           The time to set to.  Fields set to -1 indicate "don't care.
 *
 * @retval  WMS_SUCCESS          succeeded
 * @retval  WMS_HW_ERROR         error communicating with device
 ******************************************************************************/
WM_STATUS WMPmicSetAlarm( WM_DEVICE_HANDLE hDevice, const SYSTEMTIME *pTime );

/*******************************************************************************
 * Function: WMPmicSetDelayAlarm                                           *//**
 *
 * @brief   Sets the alarm to go off after a delay.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   seconds         The number of seconds delay to use.
 *
 * @retval  WMS_SUCCESS          succeeded
 * @retval  WMS_HW_ERROR         error communicating with device
 ******************************************************************************/
WM_STATUS WMPmicSetDelayAlarm( WM_DEVICE_HANDLE hDevice, unsigned int seconds );

#ifdef __cplusplus
}   /* extern "C" */
#endif

#endif  /* __WMPMICRTC_H__ */
/*------------------------------ END OF FILE ---------------------------------*/
