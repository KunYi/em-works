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
 * @file   WMPmicLED.h
 * @brief  LED and current sink functions for Wolfson PMICs.
 *
 * @version $Id: WMPmicWatchdog.h 486 2007-05-04 21:13:38Z ib $
 *
 * @Warning
 *   This software is specifically written for Wolfson devices. It may not be
 *   used with other devices.
 *
 ******************************************************************************/
#ifndef __WMPMICWATCHDOG_H__
#define __WMPMICWATCHDOG_H__

/*
 * Include files
 */
#include "WMStatus.h"

/*
 * Definitions
 */

/**
 * Type to select the watchdog mode.
 */
typedef enum WM_WDOG_MODE
{
    WM_WDOG_DISABLED        = 0,    /* Watchdog disabled. */
    WM_WDOG_INTERRUPT       = 1,    /* Generate SYS_WDOG_TO interrupt */
    WM_WDOG_RESET           = 2,    /* Reset system and generate WKUP_WDOG_RST */
    WM_WDOG_INT_THEN_RESET  = 3     /* SYS_WDOG_TO on first timeout, reset on second */
} WM_WDOG_MODE;

/**
 * Type to select the watchdog timeout.
 */
typedef enum WM_WDOG_TIMEOUT
{
    WM_WDOG_125MS           = 0,    /* 125ms */
    WM_WDOG_250MS           = 1,    /* 250ms */
    WM_WDOG_500MS           = 2,    /* 500ms */
    WM_WDOG_1S              = 3,    /* 1s */
    WM_WDOG_2S              = 4,    /* 2s */
    WM_WDOG_4S              = 5,    /* 4s */
} WM_WDOG_TIMEOUT;

/**
 * How the watchdog should behave in HIBERNATE state.
 */
typedef enum WM_WDOG_HIB_MODE
{
    WM_WDOG_HIB_DISABLE     = 0,    /* Disabled in HIBERNATE state */
    WM_WDOG_HIB_ENABLE      = 1     /* Enabled in HIBERNATE state */
} WM_WDOG_HIB_MODE;

/*
 * Function prototypes
 */
#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Function: WMWdogEnable                                                  *//**
 *
 * @brief   Enable or disable the watchdog as specified.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   mode            Mode to set (disabled, interrupt, reset, both.
 * @param   timeout         Watchdog timeout.
 * @param   hibMode         Hibernate mode.
 *
 * @retval  WMS_SUCCESS         succeeded
 * @retval  WMS_UNSUPPORTED     current sink of regulator not supported
 * @retval  WMS_HW_ERROR        error communicating with device
 ******************************************************************************/
WM_STATUS WMWdogEnable( WM_DEVICE_HANDLE    hDevice,
                        WM_WDOG_MODE        mode,
                        WM_WDOG_TIMEOUT     timeout,
                        WM_WDOG_HIB_MODE    hibMode
                     );

/*******************************************************************************
 * Function: WMWdogQueryConfig                                             *//**
 *
 * @brief   Query the current watchdog configuration.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   pMode           Receives current mode.
 * @param   pTimeout        Receives watchdog timeout.
 * @param   pHibMode        Receives hibernate mode.
 *
 * @retval  WMS_SUCCESS         succeeded
 * @retval  WMS_UNSUPPORTED     current sink of regulator not supported
 * @retval  WMS_HW_ERROR        error communicating with device
 ******************************************************************************/
WM_STATUS WMWdogQueryConfig( WM_DEVICE_HANDLE    hDevice,
                             WM_WDOG_MODE        *pMode,
                             WM_WDOG_TIMEOUT     *pTimeout,
                             WM_WDOG_HIB_MODE    *pHibMode
                           );

/*******************************************************************************
 * Function: WMWdogDisable                                                 *//**
 *
 * @brief   Disable the watchdog.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 *
 * @retval  WMS_SUCCESS         succeeded
 * @retval  WMS_UNSUPPORTED     current sink of regulator not supported
 * @retval  WMS_HW_ERROR        error communicating with device
 ******************************************************************************/
WM_STATUS WMWdogDisable( WM_DEVICE_HANDLE hDevice );

/*******************************************************************************
 * Function: WMWdogService                                                 *//**
 *
 * @brief   Service the watchdog.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 *
 * @retval  WMS_SUCCESS         succeeded
 * @retval  WMS_UNSUPPORTED     current sink of regulator not supported
 * @retval  WMS_HW_ERROR        error communicating with device
 ******************************************************************************/
WM_STATUS WMWdogService( WM_DEVICE_HANDLE hDevice );

/*******************************************************************************
 * Function: WMWdogPause                                                   *//**
 *
 * @brief   Pauses the watchdog (for a debugger).
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 *
 * @retval  WMS_SUCCESS         succeeded
 * @retval  WMS_UNSUPPORTED     current sink of regulator not supported
 * @retval  WMS_HW_ERROR        error communicating with device
 ******************************************************************************/
WM_STATUS WMWdogPause( WM_DEVICE_HANDLE hDevice );

/*******************************************************************************
 * Function: WMWdogResume                                                  *//**
 *
 * @brief   Resumes the watchdog (for a debugger).
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 *
 * @retval  WMS_SUCCESS         succeeded
 * @retval  WMS_UNSUPPORTED     current sink of regulator not supported
 * @retval  WMS_HW_ERROR        error communicating with device
 ******************************************************************************/
WM_STATUS WMWdogResume( WM_DEVICE_HANDLE hDevice );

#ifdef __cplusplus
}   /* extern "C" */
#endif

#endif    /* __WMPMICWATCHDOG_H__ */
/*------------------------------ END OF FILE ---------------------------------*/
