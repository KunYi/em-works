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
 * @file   WMPmic.h
 * @brief  Wolfson power-management interface
 *
 * @version $Id: WMPmic.h 667 2007-06-21 14:48:36Z ib $
 *
 * @Warning
 *   This software is specifically written for Wolfson devices. It may not be
 *   used with other devices.
 *
 ******************************************************************************/
#ifndef __WMPMIC_H__
#define __WMPMIC_H__

/*
 * Include files
 */
#include "WMStatus.h"
#include "WMTypes.h"
#include "WMControlLink.h"
#include "WMAudio.h"
#include "WMDevice.h"
#include "WMGPIO.h"
#include "WMInterrupts.h"
#include "WMPmicLED.h"
#include "WMPmicRegulators.h"
#include "WMPmicRTC.h"
#include "WMPmicWatchdog.h"
#include "WMPmicCharger.h"

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
 * Function: WMPmicInit                                                    *//**
 *
 * @brief  Initialises the WMPmic library
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * 
 * @retval WMS_SUCCESS          succeeded
 * @retval WMS_RESOURCE_FAIL    couldn't allocate memory
 * @retval WMS_HW_ERROR         error communicating with device
 ******************************************************************************/
WM_STATUS WMPmicInit( WM_DEVICE_HANDLE hDevice );

/*******************************************************************************
 * Function: WMPmicShutdown                                                *//**
 *
 * @brief  Cleans up the WMPmic library
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * 
 * @return void
 ******************************************************************************/
void WMPmicShutdown( WM_DEVICE_HANDLE hDevice );

#ifdef __cplusplus
}   /* extern "C" */
#endif

#endif  // __WMPMIC_H__
/*------------------------------ END OF FILE ---------------------------------*/
