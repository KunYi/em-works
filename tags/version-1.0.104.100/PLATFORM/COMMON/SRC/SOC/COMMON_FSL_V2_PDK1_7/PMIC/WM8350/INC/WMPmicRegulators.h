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
 * @file   WMPmicRegulators.h
 * @brief  Functions and definitions for regulators on Wolfson PMICs.
 *
 * @version $Id: WMPmicRegulators.h 659 2007-06-19 15:48:40Z ib $
 *
 * @Warning
 *   This software is specifically written for Wolfson devices. It may not be
 *   used with other devices.
 *
 ******************************************************************************/
#ifndef __WMPMICREGULATORS_H__
#define __WMPMICREGULATORS_H__

/*
* Include files
*/
#include <WMStatus.h>
#include <OSAbstraction.h>

/*
* Definitions
*/

/**
 * Type to select a given current sink.
 */
typedef enum WM_REGULATOR
{
    WM_REGL_NONE    = 0,
    WM_DCDC1        = 0x0001,
    WM_DCDC2        = 0x0002,
    WM_DCDC3        = 0x0004,
    WM_DCDC4        = 0x0008,
    WM_DCDC5        = 0x0010,
    WM_DCDC6        = 0x0020,
    WM_LDO1         = 0x0100,
    WM_LDO2         = 0x0200,
    WM_LDO3         = 0x0400,
    WM_LDO4         = 0x0800,
    WM_LIMIT_SW     = 0x8000
} WM_REGULATOR, WM_REGULATORS;

/**
 * Regulator operating modes
 */
typedef enum WM_REGULATOR_MODE
{
    WM_REGMODE_NORMAL,
    WM_REGMODE_LOW_POWER
} WM_REGULATOR_MODE;

/**
 * Regulator startup and shutdown slots.
 */
typedef enum WM_TSLOT
{
    WM_TSLOT_NONE = 0,
    WM_TSLOT_1,
    WM_TSLOT_2,
    WM_TSLOT_3,
    WM_TSLOT_4,
    WM_TSLOT_5,
    WM_TSLOT_6,
    WM_TSLOT_7,
    WM_TSLOT_8,
    WM_TSLOT_9,
    WM_TSLOT_10,
    WM_TSLOT_11,
    WM_TSLOT_12,
    WM_TSLOT_13,
    WM_TSLOT_14,
    WM_TSLOT_LAST,
} WM_TSLOT;

/**
 * Regulator error actions.
 */
typedef enum WM_ERRACT
{
    WM_ERRACT_NONE = 0,
    WM_ERRACT_SHUTDOWN_REG,
    WM_ERRACT_SHUTDOWN_SYS,
} WM_ERRACT;

/**
 * Regulator hibernate mode actions.
 * Use the WM_REGVAL from the appropriate regulator.
 */
typedef WM_REGVAL WM_HIB_MODE;

/**
 * Regulator Hibernate trigger hardware signal selects
 * NOTE : DCDC2/5 do not have dedicated low power registers
 * and the WM_HIB_TRIG bits can be found in the control
 * register.
 */
typedef enum WM_HIB_TRIG
{
    WM_HIB_TRIG_HIB = 0,
    WM_HIB_TRIG_L_PWR1,
    WM_HIB_TRIG_L_PWR2,
    WM_HIB_TRIG_L_PWR3
} WM_HIB_TRIG;

/*
* Function prototypes
*/
#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Function: WMPmicReglEnable                                              *//**
 *
 * @brief   Enable the given regulator(s).
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   regs            Regulator(s) to enable.
 *
 * @retval  WMS_SUCCESS          succeeded
 * @retval  WMS_HW_ERROR         error communicating with device
 ******************************************************************************/
WM_STATUS WMPmicReglEnable( WM_DEVICE_HANDLE    hDevice,
                            WM_REGULATORS       regs
                          );

/*******************************************************************************
 * Function: WMPmicReglDisable                                             *//**
 *
 * @brief   Disable the given regulator(s).
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   regs            Regulator(s) to disable.
 *
 * @retval  WMS_SUCCESS          succeeded
 * @retval  WMS_HW_ERROR         error communicating with device
 ******************************************************************************/
WM_STATUS WMPmicReglDisable( WM_DEVICE_HANDLE    hDevice,
                             WM_REGULATORS       regs
                           );

/*******************************************************************************
 * Function: WMPmicReglSetVoltage                                          *//**
 *
 * @brief   Sets the voltage level of the given regulator.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   reg                Regulator to configure.
 * @param   milliVolts        Voltage level in millivolts.
 * @param   mode            Operating mode to configure voltage for.
 *
 * @retval  WMS_SUCCESS         succeeded
 * @retval  WMS_OUT_OF_RANGE    requested voltage out of range
 * @retval  WMS_HW_ERROR        error communicating with device
 ******************************************************************************/
WM_STATUS WMPmicReglSetVoltage( WM_DEVICE_HANDLE    hDevice,
                                WM_REGULATOR        reg,
                                unsigned int        milliVolts,
                                WM_REGULATOR_MODE   mode
                              );

/*******************************************************************************
 * Function: WMPmicReglGetVoltage                                          *//**
 *
 * @brief   Returns the voltage level of the given regulator.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   reg                Regulator to query.
 * @param   pMilliVolts        Voltage level in millivolts.
 * @param   pLowMilliVolts    Low power voltage level in millivolts.
 *
 * @retval  WMS_SUCCESS          succeeded
 * @retval  WMS_HW_ERROR         error communicating with device
 ******************************************************************************/
WM_STATUS WMPmicReglGetVoltage( WM_DEVICE_HANDLE    hDevice,
                                WM_REGULATOR        reg,
                                unsigned int        *pMilliVolts,
                                unsigned int        *pLowMilliVolts
                              );

/*******************************************************************************
* Function: WMPmicReglConfigure                                            *//**
*
* @brief   Configure the given regulator.
*
* @param   hDevice          The handle to the device (from WMOpenDevice).
* @param   reg              Regulator to enable.
* @param   enSlot           Startup timeslot.
* @param   sdSlot           Shutdown timeslot.
* @param   errAct           Action to take on error.
* @param   hibMode            Hibernate Mode.
* @param   hibTrigger        Hibernate signal select.
*
* @retval  WMS_SUCCESS          succeeded
* @retval  WMS_HW_ERROR         error communicating with device
******************************************************************************/
WM_STATUS WMPmicReglConfigure( WM_DEVICE_HANDLE hDevice,
                               WM_REGULATOR     reg,
                               WM_TSLOT         enSlot,
                               WM_TSLOT         sdSlot,
                               WM_ERRACT        errAct,
                               WM_HIB_MODE         hibMode,
                               WM_HIB_TRIG      hibTrigger
                             );


#ifdef __cplusplus
}   /* extern "C" */
#endif

#endif    /* __WMPMICREGULATORS_H__ */
/*------------------------------ END OF FILE ---------------------------------*/
