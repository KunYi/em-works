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
 * @file   WMPmicCharger.h
 * @brief  Functions and definitions for battery chargers on Wolfson PMICs.
 *
 * @version $Id: WMPmicCharger.h 667 2007-06-21 14:48:36Z ib $
 *
 * @Warning
 *   This software is specifically written for Wolfson devices. It may not be
 *   used with other devices.
 ******************************************************************************/
#ifndef __WMPMICCHARGER_H__
#define __WMPMICCHARGER_H__

/*
 * Include files
 */
#include <WMPmic.h>
#include <OSAbstraction.h>

/*
 * Definitions
 */

/**
* Trickle charge current values.
*/
typedef enum WM_TRICKLE_CURRENT
{
    WM_TRICKLE_CURRENT_50mA = 0,
    WM_TRICKLE_CURRENT_100mA,
} WM_TRICKLE_CURRENT;

/**
* Trickle charge threshold values.
*/
typedef enum WM_TRICKLE_THRESHOLD
{
    WM_TRICKLE_THRESHOLD_3_1V = 0,
    WM_TRICKLE_THRESHOLD_3_9V,
} WM_TRICKLE_THRESHOLD;

/**
* Trickle battery voltage values.
*/
typedef enum WM_BATT_VOLTAGE
{
    WM_BATT_VOLTAGE_4_05V   = 0,
    WM_BATT_VOLTAGE_4_1V,
    WM_BATT_VOLTAGE_4_15V,
    WM_BATT_VOLTAGE_4_2V,
} WM_BATT_VOLTAGE;

/**
* Trickle charger status values.
*/
typedef enum WM_CHARGER_STATUS
{
    WM_CHARGER_STATUS_CURRENT_ZERO = 0,
    WM_CHARGER_STATUS_TRICKLE,
    WM_CHARGER_STATUS_FAST,
} WM_CHARGER_STATUS;

/**
* Trickle charge configuration structure.
*/
typedef struct WM_TRICKLE_CONFIG
{
    WM_TRICKLE_CURRENT      current;
    WM_TRICKLE_THRESHOLD    threshold;
    BOOL                    tempChoke;
} WM_TRICKLE_CONFIG;

#define WM_EOC_MIN              20 /* Minimum End Of Charge current in mA */
#define WM_EOC_MAX              90 /* Maximum End Of Charge current in mA */
#define WM_EOC_TRICKLE_LIMIT    50 /* If trickle set to 50mA EOC should be below this value */

#define WM_FAST_CHARGE_CURRENT_OFF  0
#define WM_FAST_CHARGE_CURRENT_MIN  50
#define WM_FAST_CHARGE_CURRENT_MAX  750

#define WM_CHARGING_TIME_OUT_MIN    17
#define WM_CHARGING_TIME_OUT_MAX    255
/*
* Function prototypes
*/
#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Function: WMPmicChargerEnable                                           *//**
 *
 * @brief   Enable the battery charger.
 *
 * Note that the charger is always enabled to protect the battery from being 
 * charged in an uncontrolled fashion.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * 
 * @retval  WMS_SUCCESS          succeeded
 * @retval  WMS_HW_ERROR         error communicating with device
 ******************************************************************************/
WM_STATUS WMPmicChargerEnable( WM_DEVICE_HANDLE hDevice );

/*******************************************************************************
 * Function: WMPmicChargerDisable                                          *//**
 *
 * @brief   Disable the battery charger.
 *
 * Note that the charger is always enabled to protect the battery from being
 * charged in an uncontrolled fashion. 
 * This function will set the current to zero.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * 
 * @retval  WMS_SUCCESS          succeeded
 * @retval  WMS_HW_ERROR         error communicating with device
 ******************************************************************************/
WM_STATUS WMPmicChargerDisable( WM_DEVICE_HANDLE hDevice );

/*******************************************************************************
 * Function: WMPmicChargerIsEnabled                                        *//**
 *
 * @brief   Checks to see if the battery charger is enabled or disabled.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   pEnabled        TRUE = enabled, FALSE = disabled.
 * 
 * @retval  WMS_SUCCESS          succeeded
 * @retval  WMS_HW_ERROR         error communicating with device
 ******************************************************************************/
WM_STATUS WMPmicChargerIsEnabled( WM_DEVICE_HANDLE hDevice, BOOL *pEnabled );

/*******************************************************************************
 * Function: WMPmicChargerPause                                            *//**
 *
 * @brief   Pause the battery charger.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   pause           TRUE to pause, FALSE to continue charging.
 * 
 * @retval  WMS_SUCCESS          succeeded
 * @retval  WMS_HW_ERROR         error communicating with device
 ******************************************************************************/
WM_STATUS WMPmicChargerPause( WM_DEVICE_HANDLE hDevice );

/*******************************************************************************
 * Function: WMPmicChargerResume                                           *//**
 *
 * @brief   Resume the battery charger.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   pause           TRUE to pause, FALSE to continue charging.
 * 
 * @retval  WMS_SUCCESS          succeeded
 * @retval  WMS_HW_ERROR         error communicating with device
 ******************************************************************************/
WM_STATUS WMPmicChargerResume( WM_DEVICE_HANDLE hDevice );

/*******************************************************************************
 * Function: WMPmicChargerIsPaused                                         *//**
 *
 * @brief   Check if battery charging is paused.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   pPaused         TRUE if paused, FALSE of not paused.
 * 
 * @retval  WMS_SUCCESS          succeeded
 * @retval  WMS_HW_ERROR         error communicating with device
******************************************************************************/
WM_STATUS WMPmicChargerIsPaused( WM_DEVICE_HANDLE hDevice, BOOL *pPaused );

/*******************************************************************************
 * Function: WMPmicChargerSetEOCCurrent                                    *//**
 *
 * @brief   Set the end of charge current for the battery charger.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   eocCurrent      The end of charge current in milliamps.
 * 
 * @retval  WMS_SUCCESS           succeeded
 * @retval  WMS_HW_ERROR          error communicating with device
 * @retval  WMS_OUT_OF_RANGE      EOC current value out of range
 ******************************************************************************/
WM_STATUS WMPmicChargerSetEOCCurrent( WM_DEVICE_HANDLE hDevice, 
                                      int eocCurrent 
                                    );

/*******************************************************************************
 * Function: WMPmicChargerGetEOCCurrent                                    *//**
 *
 * @brief   Get the end of charge current for the battery charger.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   pEocCurrent     The end of charge current in milliamps.
 * 
 * @retval  WMS_SUCCESS          succeeded
 * @retval  WMS_HW_ERROR         error communicating with device
 ******************************************************************************/
WM_STATUS WMPmicChargerGetEOCCurrent( WM_DEVICE_HANDLE hDevice, 
                                      int *pEocCurrent 
                                    );

/*******************************************************************************
 * Function: WMPmicChargerTrickleConfig                                    *//**
 *
 * @brief   Set up the charger for trickle charge operation.
 *
 * @param   hDevice          The handle to the device (from WMOpenDevice).
 * @param   trickleConfig    The trickle charge configuration.
 * 
 * @retval  WMS_SUCCESS           succeeded
 * @retval  WMS_HW_ERROR          error communicating with device
 * @retval  WMS_INVALID_PARAMETER invalid trickle charge value.
 ******************************************************************************/
WM_STATUS WMPmicChargerSetTrickleConfig( WM_DEVICE_HANDLE       hDevice, 
                                         WM_TRICKLE_CONFIG      trickleConfig
                                       );

/*******************************************************************************
 * Function: WMPmicChargerGetTrickleConfig                                 *//**
 *
 * @brief   Get the trickle charge current for the battery charger.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   pTrickleConfig  The trickle charge configuration.
 * 
 * @retval  WMS_SUCCESS           succeeded
 * @retval  WMS_HW_ERROR          error communicating with device
 ******************************************************************************/
WM_STATUS WMPmicChargerGetTrickleConfig( WM_DEVICE_HANDLE   hDevice, 
                                         WM_TRICKLE_CONFIG  *pTrickleConfig
                                       );

/*******************************************************************************
 * Function: WMPmicChargerSetBattVoltage                                   *//**
 *
 * @brief   Set the target battery voltage for charging.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   voltage         The target voltage for charging.
 * 
 * @retval  WMS_SUCCESS           succeeded
 * @retval  WMS_HW_ERROR          error communicating with device
 ******************************************************************************/
WM_STATUS WMPmicChargerSetBattVoltage( WM_DEVICE_HANDLE hDevice, 
                                       WM_BATT_VOLTAGE voltage 
                                     );

/*******************************************************************************
 * Function: WMPmicChargerGetBattVoltage                                   *//**
 *
 * @brief   Get the target battery voltage for charging.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   pVoltage        The target voltage for charging.
 * 
 * @retval  WMS_SUCCESS           succeeded
 * @retval  WMS_HW_ERROR          error communicating with device
 ******************************************************************************/
WM_STATUS WMPmicChargerGetBattVoltage( WM_DEVICE_HANDLE hDevice, 
                                       WM_BATT_VOLTAGE *pVoltage 
                                     );

/*******************************************************************************
 * Function: WMPmicChargerGetStatus                                        *//**
 *
 * @brief   Get the charging status.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   pStatus         The charging status.
 * 
 * @retval  WMS_SUCCESS           succeeded
 * @retval  WMS_HW_ERROR          error communicating with device
 ******************************************************************************/
WM_STATUS WMPmicChargerGetStatus( WM_DEVICE_HANDLE hDevice, 
                                  WM_CHARGER_STATUS *pStatus 
                                );

/*******************************************************************************
 * Function: WMPmicChargerConfigStatusLED                                  *//**
 *
 * @brief   Configure GPIO pin as CH_IND to show the charger status.
 *          LED off                     - Charger current set to zero.
 *          LED blinking slowly (0.5Hz) - Trickle charging.
 *          LED blinking quickly (1Hz)  - Fast charging.
 *          LED continuously on         - Charging complete.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   enable          TRUE enables the LED status, FALSE disables it.
 * @param   gpio               The GPIO to configure as CH_IND
 * 
 * @retval  WMS_SUCCESS             succeeded
 * @retval  WMS_HW_ERROR            error communicating with device
 * @retval  WMS_INVALID_PARAMETER   not a valid GPIO.
 ******************************************************************************/
WM_STATUS WMPmicChargerConfigStatusLED( WM_DEVICE_HANDLE hDevice,
                                        BOOL enable,
                                        WM_GPIO gpio
                                      );

/*******************************************************************************
 * Function: WMPmicChargerEnableFastCharge                                 *//**
 *
 * @brief   Set up the charger for fast charge operation.
 *
 * Note if current is set to zero then fast charging will be disabled.
 *
 * @param   hDevice          The handle to the device (from WMOpenDevice).
 * @param   enable           TRUE enable fast charge, FALSE disable fast charge.
 * @param   current         charge current.
 * 
 * @retval  WMS_SUCCESS           succeeded
 * @retval  WMS_HW_ERROR          error communicating with device
 * @retval  WMS_OUT_OF_RANGE      Fast charge current out of range
 ******************************************************************************/
WM_STATUS WMPmicChargerEnableFastCharge( WM_DEVICE_HANDLE hDevice, 
                                         BOOL enable,
                                         int current
                                       );

/*******************************************************************************
 * Function: WMPmicChargerGetFastChargeConfig                              *//**
 *
 * @brief   Get the fast charge configuration.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   pEnabled        TRUE fast charge enabled, FALSE fast charge disabled.
 * @param   current        charge current.
 * 
 * @retval  WMS_SUCCESS           succeeded
 * @retval  WMS_HW_ERROR          error communicating with device
 ******************************************************************************/
WM_STATUS WMPmicChargerGetFastChargeConfig( WM_DEVICE_HANDLE hDevice, 
                                            BOOL *pEnabled, 
                                            int *pCurrent
                                          );

/*******************************************************************************
 * Function: WMPmicChargerSetTimer                                         *//**
 *
 * @brief   Sets the charging time-out.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   mins            Charging time out in mins.
 * 
 * @retval  WMS_SUCCESS           succeeded
 * @retval  WMS_HW_ERROR          error communicating with device
 * @retval  WMS_OUT_OF_RANGE      Charging time-out value is out of range
 ******************************************************************************/
WM_STATUS WMPmicChargerSetTimer( WM_DEVICE_HANDLE hDevice, 
                                 int mins
                               );

#ifdef __cplusplus
}   /* extern "C" */
#endif

#endif  /* __WMPMICCHARGER_H__ */
/*------------------------------ END OF FILE ---------------------------------*/
