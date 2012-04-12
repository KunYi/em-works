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
 * @file   WM8350Util.h
 * @brief  Useful specific functions for the WM8350.
 *
 * This file contains functions which set up the WM8350 in various standard
 * configurations designed for different platforms.
 *
 * @version $Id: WM8350Util.h 483 2007-05-04 19:02:20Z ib $
 *
 * @Warning
 *   This software is specifically written for Wolfson devices. It may not be
 *   used with other devices.
 *
 ******************************************************************************/
#ifndef __WM8350UTIL_H__
#define __WM8350UTIL_H__

/*
 * Include files
 */
#include <WMStatus.h>

/*
 * Definitions
 */

/*
 * Function prototypes
 */
#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Function: WM8350In3ToOut1                                               *//**
 *
 * @brief   Routes IN3L/R (AUXL/R) to OUT1L/R (HPL/R) on the WM8350
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 *
 * @retval  WMS_SUCCESS          succeeded
 * @retval  WMS_HW_ERROR         error communicating with device
 ******************************************************************************/
WM_STATUS WM8350In3ToOut1( WM_DEVICE_HANDLE hDevice );

/*******************************************************************************
 * Function: WM8350PowerBoardFromUSB                                       *//**
 *
 * @brief   Sets up the eval board to power off USB.
 *
 * This sets LDO2 to power AVDD_CODEC, LDO4 to power HPVDD (both 3.0V) and
 * DCDC3 to power DBVDD and DCVDD (at 2.0V).
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 *
 * @retval  WMS_SUCCESS          succeeded
 * @retval  WMS_HW_ERROR         error communicating with device
 ******************************************************************************/
WM_STATUS WM8350PowerBoardFromUSB( WM_DEVICE_HANDLE hDevice );

/*******************************************************************************
 * Function: WM8350PowerMX32ADSBoard                                       *//**
 *
 * @brief   Sets up the i.MX31/i.MX32 ADS plugin board to power the system.
 *
 * This sets the voltages as per 1133-EV1 WM8350 DC Spec:
 *
 *   Timeslot 1:
 *      - DCDC1 @ 1.2V
 *   Timeslot 2:
 *      - DCDC2 enabled
 *      - DCDC3 @ 2.8V
 *      - DCDC4 @ 1.8V
 *      - LDO1  @ 2.8V
 *      - LDO2  @ 3.3V
 *      - LDO4  @ 2.8V
 *   Timeslot 3:
 *      - DCDC6 @ 1.8V
 *      - LDO3  @ 1.5V
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 *
 * @retval  WMS_SUCCESS          succeeded
 * @retval  WMS_HW_ERROR         error communicating with device
 ******************************************************************************/
WM_STATUS WM8350PowerMX32ADSBoard( WM_DEVICE_HANDLE hDevice );

/*******************************************************************************
 * Function: WM8350SelectInterface                                         *//**
 *
 * @brief  Selects primary or alternate interface.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   controlIF   control interface
 * 
 * @retval  WMS_SUCCESS          succeeded
 * @retval  WMS_HW_ERROR         error communicating with device
 ******************************************************************************/
WM_STATUS WM8350SelectInterface( WM_DEVICE_HANDLE   hDevice,
                                 WM_CONTROL_IF      controlIF
                               );

/*******************************************************************************
* Function: WMPmicConfigureHibernate                                       *//**
*
* @brief   Configure the PMIC for entering the Hibernate state.
*
* @param   none
* 
* @retval  WMS_SUCCESS          succeeded
* @retval  WMS_HW_ERROR         error communicating with device
******************************************************************************/
WM_STATUS WMPmicConfigureHibernate( WM_DEVICE_HANDLE hDevice );

#ifdef __cplusplus
}   /* extern "C" */
#endif

#endif    /* __WM8350UTIL_H__ */
/*------------------------------ END OF FILE ---------------------------------*/
