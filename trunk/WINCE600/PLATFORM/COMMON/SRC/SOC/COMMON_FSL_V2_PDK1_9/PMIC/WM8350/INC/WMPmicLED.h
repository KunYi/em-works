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
 * @version $Id: WMPmicLED.h 659 2007-06-19 15:48:40Z ib $
 *
 * @Warning
 *   This software is specifically written for Wolfson devices. It may not be
 *   used with other devices.
 *
 ******************************************************************************/
#ifndef __WMPMICLED_H__
#define __WMPMICLED_H__

/*
* Include files
*/
#include "WMStatus.h"
#include "WMPmicRegulators.h"

/*
* Definitions
*/

/**
 * Type to select a given current sink.
 */
typedef enum WM_ISINK
{
    WM_ISINK_NONE = 0,
    WM_ISINK_A = 1,
    WM_ISINK_B,
    WM_ISINK_C,
    WM_ISINK_D,
    WM_ISINK_E,
} WM_ISINK;

/**
 * The LED ramp settings in LED mode.
 */
typedef enum WM_LED_RAMP_RATE
{
    WM_LED_RAMP_NONE = 0,
    WM_LED_RAMP_250MS,
    WM_LED_RAMP_500MS,
    WM_LED_RAMP_1S,
    WM_LED_FLASH_RAMP_2MS = WM_LED_RAMP_250MS,
    WM_LED_FLASH_RAMP_4MS = WM_LED_RAMP_500MS,
    WM_LED_FLASH_RAMP_8MS = WM_LED_RAMP_1S
} WM_LED_RAMP_RATE;

/**
 * Which ramp to control.
 */
typedef enum WM_LED_RAMP
{
    WM_LED_ON_RAMP = 0x1,
    WM_LED_OFF_RAMP = 0x2,
    WM_LED_ON_OFF_RAMP = (WM_LED_ON_RAMP|WM_LED_OFF_RAMP)
} WM_LED_RAMP;

/*
 * Function prototypes
 */
#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Function: WMPmicLEDEnable                                               *//**
 *
 * @brief   Enable or disable the given current sink.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   iSink           Current sink to configure.
 * @param   regl            Current source (regulator).
 * @param   iSel            Current level (field value).
 *
 * @retval  WMS_SUCCESS         succeeded
 * @retval  WMS_UNSUPPORTED     current sink of regulator not supported
 * @retval  WMS_HW_ERROR        error communicating with device
 ******************************************************************************/
WM_STATUS WMPmicLEDEnable( WM_DEVICE_HANDLE hDevice,
                           WM_ISINK         iSink,
                           WM_REGULATOR     regl,
                           WM_REGVAL        iSel
                         );

/*******************************************************************************
 * Function: WMPmicLEDDisable                                              *//**
 *
 * @brief   Disable the given current sink.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   iSink           Current sink to configure.
 * @param   regl            Current source (regulator).
 *
 * @retval  WMS_SUCCESS         succeeded
 * @retval  WMS_UNSUPPORTED     current sink of regulator not supported
 * @retval  WMS_HW_ERROR        error communicating with device
 ******************************************************************************/
WM_STATUS WMPmicLEDDisable( WM_DEVICE_HANDLE    hDevice,
                            WM_ISINK            iSink,
                            WM_REGULATOR        regl
                          );

/*******************************************************************************
 * Function: WMPmicLEDIsEnabled                                            *//**
 *
 * @brief   Returns whether the given current sink is enabled.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   iSink           Current sink to query.
 * @param   pEnabled        Receives whether the sink is enabled.
 *
 * @retval  WMS_SUCCESS         succeeded
 * @retval  WMS_UNSUPPORTED     current sink of regulator not supported
 * @retval  WMS_HW_ERROR        error communicating with device
 ******************************************************************************/
WM_STATUS WMPmicLEDIsEnabled( WM_DEVICE_HANDLE    hDevice,
                              WM_ISINK            iSink,
                              BOOL                *pEnabled
                            );

/*******************************************************************************
 * Function: WMPmicLEDSetLevel                                             *//**
 *
 * @brief  Sets the current level on the given current sink.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   iSink           Current sink to configure.
 * @param   iSel            Current level (field value).
 *
 * @retval  WMS_SUCCESS         succeeded
 * @retval  WMS_UNSUPPORTED     current sink not supported
 * @retval  WMS_OUT_OF_RANGE    requested current too large
 * @retval  WMS_HW_ERROR        error communicating with device
 ******************************************************************************/
WM_STATUS WMPmicLEDSetLevel( WM_DEVICE_HANDLE   hDevice,
                             WM_ISINK           iSink,
                             WM_REGVAL          iSel
                           );

/*******************************************************************************
 * Function: WMPmicLEDGetLevel                                             *//**
 *
 * @brief  Returns the current level on the given current sink.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   iSink           Current sink to configure.
 * @param   pLevel          Receives current level (field value).
 *
 * @retval  WMS_SUCCESS         succeeded
 * @retval  WMS_UNSUPPORTED     current sink not supported
 * @retval  WMS_HW_ERROR        error communicating with device
 ******************************************************************************/
WM_STATUS WMPmicLEDGetLevel( WM_DEVICE_HANDLE   hDevice,
                             WM_ISINK           iSink,
                             WM_REGVAL          *pLevel
                           );

/*******************************************************************************
 * Function: WMPmicLEDSetRamp                                              *//**
 *
 * @brief  Sets the ramp speed on the given current sink.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   iSink           Current sink to configure.
 * @param   ramp            Ramp(s) to configure.
 * @param   rate            Ramp rate.
 *
 * @retval  WMS_SUCCESS         succeeded
 * @retval  WMS_UNSUPPORTED     current sink not supported
 * @retval  WMS_HW_ERROR        error communicating with device
 ******************************************************************************/
WM_STATUS WMPmicLEDSetRamp( WM_DEVICE_HANDLE    hDevice,
                            WM_ISINK            iSink,
                            WM_LED_RAMP         ramp,
                            WM_LED_RAMP_RATE    rate
                          );

/*******************************************************************************
 * Function: WMPmicLEDGetRamp                                              *//**
 *
 * @brief  Returns the ramp speed on the given current sink.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   iSink           Current sink to configure.
 * @param   ramp            Ramp to query.
 * @param   pRate           Receives ramp rate.
 *
 * @retval  WMS_SUCCESS         succeeded
 * @retval  WMS_UNSUPPORTED     current sink not supported
 * @retval  WMS_HW_ERROR        error communicating with device
 ******************************************************************************/
WM_STATUS WMPmicLEDGetRamp( WM_DEVICE_HANDLE    hDevice,
                            WM_ISINK            iSink,
                            WM_LED_RAMP         ramp,
                            WM_LED_RAMP_RATE    *pRate
                          );

#ifdef __cplusplus
}   /* extern "C" */
#endif

#endif    /* __WMPMICLED_H__ */
/*------------------------------ END OF FILE ---------------------------------*/
