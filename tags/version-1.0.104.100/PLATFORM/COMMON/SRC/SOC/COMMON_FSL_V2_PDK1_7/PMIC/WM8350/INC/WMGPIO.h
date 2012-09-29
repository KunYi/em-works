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
 * @file   WMGPIO.h
 * @brief  GPIO functions for Wolfson devices.
 *
 * @version $Id: WMGPIO.h 644 2007-06-15 22:16:53Z ib $
 *
 * @Warning
 *   This software is specifically written for Wolfson devices. It may not be
 *   used with other devices.
 *
 ******************************************************************************/
#ifndef __WMGPIO_H__
#define __WMGPIO_H__

/*
* Include files
*/
#include <WMStatus.h>
#include <WMTypes.h>

/*
* Definitions
*/

/**
 * Type to select a given GPIO or set of GPIOs.
 */
typedef enum WM_GPIO
{
    WM_GPIO_0   = 0x0001,
    WM_GPIO_1   = 0x0002,
    WM_GPIO_2   = 0x0004,
    WM_GPIO_3   = 0x0008,
    WM_GPIO_4   = 0x0010,
    WM_GPIO_5   = 0x0020,
    WM_GPIO_6   = 0x0040,
    WM_GPIO_7   = 0x0080,
    WM_GPIO_8   = 0x0100,
    WM_GPIO_9   = 0x0200,
    WM_GPIO_10  = 0x0400,
    WM_GPIO_11  = 0x0800,
    WM_GPIO_12  = 0x1000,
    WM_GPIO_13  = 0x2000,
    WM_GPIO_14  = 0x4000,
    WM_GPIO_15  = 0x8000,
} WM_GPIO, WM_GPIOS;

/**
 * Type to indicate the GPIO direction.
 */
typedef enum WM_GPIO_DIR
{
    WM_GPIO_OUTPUT = 0, /**< GPIO configured as output */
    WM_GPIO_INPUT = 1,  /**< GPIO configured as input */
} WM_GPIO_DIR;

/**
 * Type to indicate the GPIO level.
 */
typedef enum WM_GPIO_LEVEL
{
    WM_GPIO_LOW = 0,  /**< GPIO is low */
    WM_GPIO_HIGH = 1, /**< GPIO is high */
} WM_GPIO_LEVEL;

/**
 * Type to indicate the GPIO polarity for input GPIOs.
 */
typedef enum WM_GPIO_POLARITY
{
    WM_GPIO_ACTIVE_LOW = 0,  /**< GPIO is active-low */
    WM_GPIO_ACTIVE_HIGH = 1, /**< GPIO is active-high */
} WM_GPIO_POLARITY;

/**
 * Type to indicate the GPIO pin type for output GPIOs.
 */
typedef enum WM_GPIO_PIN_TYPE
{
    WM_GPIO_CMOS = 0,  /**< GPIO is CMOS */
    WM_GPIO_OPEN_DRAIN = 1, /**< GPIO is open-drain */
} WM_GPIO_PIN_TYPE;

/**
 * Type to indicate the GPIO alternate function.
 *
 * This should be passed in as a register field value (i.e. the absolute
 * value of the field before it is shifted to put it into a register).
 *
 * For example, GPIO3 alternate function might be bits 15:12 of the register
 * and hence have values from 0x1000 to 0xF000.  These should be passed in as
 * 0x0001 to 0x000F.  The value will be shifted appropriately before it is
 * written to the device.
 */
typedef WM_REGVAL WM_GPIO_ALTFN;

/*
 * GPIO setup
 */
typedef struct WM_GPIO_CONFIG
{
    WM_GPIO          GPIO;
    WM_GPIO_DIR      dir;
    WM_GPIO_ALTFN    function;
    union {
        WM_GPIO_POLARITY polarity;
        WM_GPIO_PIN_TYPE pinType;
    } POL_PIN;
    BOOL             pullUp;
    BOOL             pullDown;
} WM_GPIO_CONFIG;
/*
 * Function prototypes
 */
#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Function: WMGPGetValidGPIOs                                             *//**
 *
 * @brief   Returns the bitmask of GPIOs supported on the device.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 *
 * @return  The bitmask of supported GPIOs
 ******************************************************************************/
WM_GPIOS WMGPGetValidGPIOs( WM_DEVICE_HANDLE hDevice );

/*******************************************************************************
 * Function: WMGPSetLevels                                                 *//**
 *
 * @brief   Sets the level of the given GPIOs.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   GPIOs           Bitmask of GPIOs to configure.
 * @param   levelMask       GPIO levels to set - 1 = high and 0 = low.
 *
 * @retval  WMS_SUCCESS          succeeded
 * @retval  WMS_HW_ERROR         error communicating with device
 ******************************************************************************/
WM_STATUS WMGPSetLevels( WM_DEVICE_HANDLE hDevice, WM_GPIOS GPIOs, WM_GPIOS levelMask );

/*******************************************************************************
 * Function: WMGPGetLevels                                                 *//**
 *
 * @brief   Returns the current GPIO levels.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   pLevels         Receives the GPIO levels as a bitmask. For each bit,
 *                          set corresponds to a high level and clear corresponds
 *                          to low.
 *
 * @retval  WMS_SUCCESS          succeeded
 * @retval  WMS_HW_ERROR         error communicating with device
 ******************************************************************************/
WM_STATUS WMGPGetLevels( WM_DEVICE_HANDLE hDevice, WM_GPIOS *pHigh );

/*******************************************************************************
 * Function: WMGPClearInt                                                  *//**
 *
 * @brief   Clears interrupts on the given input GPIOs.
 *
 * Note: if any of the GPIOs passed in are outputs, this function will set their
 * level to low.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   GPIOs           Bitmask of GPIOs to configure.
 *
 * @retval  WMS_SUCCESS          succeeded
 * @retval  WMS_HW_ERROR         error communicating with device
 ******************************************************************************/
WM_STATUS WMGPClearInt( WM_DEVICE_HANDLE hDevice, WM_GPIOS GPIOs );

/*******************************************************************************
 * Function: WMGPSetDir                                                    *//**
 *
 * @brief   Sets up the given GPIOs as inputs or outputs.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   GPIOs           Bitmask of GPIOs to configure.
 * @param   dir             GPIO direction to set.
 *
 * @retval  WMS_SUCCESS          succeeded
 * @retval  WMS_HW_ERROR         error communicating with device
 ******************************************************************************/
WM_STATUS WMGPSetDir( WM_DEVICE_HANDLE hDevice, WM_GPIOS GPIOs, WM_GPIO_DIR dir );

/*******************************************************************************
 * Function: WMGPGetInputs                                                 *//**
 *
 * @brief   Returns the set of GPIOs which are configured as inputs.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   pInputs         Receives the GPIOs configured as inputs as a bitmask.
 *
 * @retval  WMS_SUCCESS          succeeded
 * @retval  WMS_HW_ERROR         error communicating with device
 ******************************************************************************/
WM_STATUS WMGPGetInputs( WM_DEVICE_HANDLE hDevice, WM_GPIOS *pInputs );

/*******************************************************************************
 * Function: WMGPGetOutputs                                                *//**
 *
 * @brief   Returns the set of GPIOs which are configured as outputs.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   pOutputs        Receives the GPIOs configured as outputs as a bitmask.
 *
 * @retval  WMS_SUCCESS          succeeded
 * @retval  WMS_HW_ERROR         error communicating with device
 ******************************************************************************/
WM_STATUS WMGPGetOutputs( WM_DEVICE_HANDLE hDevice, WM_GPIOS *pOutputs );

/*******************************************************************************
 * Function: WMGPSetPolarity                                               *//**
 *
 * @brief   Configures the polarity of the given GPIOs.
 *
 * This only makes sense for GPIOs configured as inputs.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   GPIOs           Bitmask of GPIOs to configure.
 * @param   polarity        Polarity to set.
 *
 * @retval  WMS_SUCCESS          succeeded
 * @retval  WMS_HW_ERROR         error communicating with device
 ******************************************************************************/
WM_STATUS WMGPSetPolarity( WM_DEVICE_HANDLE hDevice,
                           WM_GPIOS         GPIOs,
                           WM_GPIO_POLARITY polarity
                         );

/*******************************************************************************
 * Function: WMGPGetPolarity                                               *//**
 *
 * @brief   Returns the polarity of the GPIOs.
 *
 * This function returns a bitmask.  1 corresponds to active high and 0
 * corresponds to active low.  This only makes sense for input GPIOs.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   pActiveHigh     Receives the GPIOs configured as active high as a bitmask.
 *
 * @retval  WMS_SUCCESS          succeeded
 * @retval  WMS_HW_ERROR         error communicating with device
 ******************************************************************************/
WM_STATUS WMGPGetPolarity( WM_DEVICE_HANDLE hDevice, WM_GPIOS *pActiveHigh );

/*******************************************************************************
 * Function: WMGPSetAutoInvert                                             *//**
 *
 * @brief   Configures the auto-invert of the given GPIOs.
 *
 * GPIOs configured with auto-invert automatically flip polarity when an
 * interrupt is triggered to detect press and release.
 *
 * This only makes sense for GPIOs configured as inputs.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   GPIOs           Bitmask of GPIOs to configure.
 * @param   debounce        Whether to add or remove debounce.
 *
 * @retval  WMS_SUCCESS          succeeded
 * @retval  WMS_HW_ERROR         error communicating with device
 ******************************************************************************/
WM_STATUS WMGPSetAutoInvert( WM_DEVICE_HANDLE hDevice,
                             WM_GPIOS         GPIOs,
                             BOOL             autoInvert
                           );

/*******************************************************************************
 * Function: WMGPGetAutoInvert                                             *//**
 *
 * @brief   Returns the set of GPIOs which are configured with auto-invert.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   pAutoInvert     Receives the GPIOs configured with auto-invert as a bitmask.
 *
 * @retval  WMS_SUCCESS          succeeded
 * @retval  WMS_HW_ERROR         error communicating with device
 ******************************************************************************/
WM_STATUS WMGPGetAutoInvert( WM_DEVICE_HANDLE hDevice, WM_GPIOS *pAutoInvert );

/*******************************************************************************
 * Function: WMGPSetPinType                                                *//**
 *
 * @brief   Configures the pin type of the given GPIOs.
 *
 * This only makes sense for GPIOs configured as outputs.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   GPIOs           Bitmask of GPIOs to configure.
 * @param   pinType         Pin type.
 *
 * @retval  WMS_SUCCESS          succeeded
 * @retval  WMS_HW_ERROR         error communicating with device
 ******************************************************************************/
WM_STATUS WMGPSetPinType( WM_DEVICE_HANDLE  hDevice,
                          WM_GPIOS          GPIOs,
                          WM_GPIO_PIN_TYPE  pinType
                        );

/*******************************************************************************
 * Function: WMGPGetPinType                                                *//**
 *
 * @brief   Returns the pin types of the GPIOs.
 *
 * This function returns a bitmask.  1 corresponds to open-drain and 0
 * corresponds to CMOS.  This only makes sense for output GPIOs.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   pOpenDrain      Receives the GPIOs configured as open drain as a bitmask.
 *
 * @retval  WMS_SUCCESS          succeeded
 * @retval  WMS_HW_ERROR         error communicating with device
 ******************************************************************************/
WM_STATUS WMGPGetPinType( WM_DEVICE_HANDLE hDevice, WM_GPIOS *pOpenDrain );

/*******************************************************************************
 * Function: WMGPSetPullUp                                                 *//**
 *
 * @brief   This function sets or removes pull-ups on the given GPIOs.
 *
 * This will remove pull-downs from GPIOs configured with pull-ups.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   GPIOs           Bitmask of GPIOs to configure.
 * @param   pullUp          Whether to enable (TRUE) or disable (FALSE) pull-up.
 *
 * @retval  WMS_SUCCESS          succeeded
 * @retval  WMS_HW_ERROR         error communicating with device
 ******************************************************************************/
WM_STATUS WMGPSetPullUp( WM_DEVICE_HANDLE hDevice, WM_GPIOS GPIOs, BOOL pullUp );

/*******************************************************************************
 * Function: WMGPGetPullUp                                                 *//**
 *
 * @brief   Returns the set of GPIOs which are configured with pull-ups.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   pPullUps        Receives the GPIOs configured with pull-ups as a bitmask.
 *
 * @retval  WMS_SUCCESS          succeeded
 * @retval  WMS_HW_ERROR         error communicating with device
 ******************************************************************************/
WM_STATUS WMGPGetPullUp( WM_DEVICE_HANDLE hDevice, WM_GPIOS *pPullUps );

/*******************************************************************************
 * Function: WMGPSetPullDown                                               *//**
 *
 * @brief   Sets up the given GPIOs with pull-downs.
 *
 * This will remove pull-ups from GPIOs configured with pull-down.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   GPIOs           Bitmask of GPIOs to configure.
 * @param   pullDown        Whether to enable (TRUE) or disable (FALSE) pull-down.
 *
 * @retval  WMS_SUCCESS          succeeded
 * @retval  WMS_HW_ERROR         error communicating with device
 ******************************************************************************/
WM_STATUS WMGPSetPullDown( WM_DEVICE_HANDLE hDevice,
                           WM_GPIOS         GPIOs,
                           BOOL             pullDown
                         );

/*******************************************************************************
 * Function: WMGPGetPullDown                                               *//**
 *
 * @brief   Returns the set of GPIOs which are configured with pull-downs.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   pPullDowns      Receives the GPIOs configured with pull-downs as a bitmask.
 *
 * @retval  WMS_SUCCESS          succeeded
 * @retval  WMS_HW_ERROR         error communicating with device
 ******************************************************************************/
WM_STATUS WMGPGetPullDown( WM_DEVICE_HANDLE hDevice, WM_GPIOS *pPullDowns );

/*******************************************************************************
 * Function: WMGPSetDebounce                                               *//**
 *
 * @brief   Configures the debouncing of the given GPIOs.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   GPIOs           Bitmask of GPIOs to configure.
 * @param   debounce        Whether to add or remove debounce.
 *
 * @retval  WMS_SUCCESS          succeeded
 * @retval  WMS_HW_ERROR         error communicating with device
 ******************************************************************************/
WM_STATUS WMGPSetDebounce( WM_DEVICE_HANDLE hDevice,
                           WM_GPIOS         GPIOs,
                           BOOL             debounce
                         );

/*******************************************************************************
 * Function: WMGPGetDebounce                                               *//**
 *
 * @brief   Returns the set of GPIOs which are configured with debounce.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   pDebounced      Receives the GPIOs configured with debounce as a bitmask.
 *
 * @retval  WMS_SUCCESS          succeeded
 * @retval  WMS_HW_ERROR         error communicating with device
 ******************************************************************************/
WM_STATUS WMGPGetDebounce( WM_DEVICE_HANDLE hDevice, WM_GPIOS *pDebounced );

/*******************************************************************************
 * Function: WMGPSetFunction                                               *//**
 *
 * @brief   Sets the function of the given GPIO.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   GPIO            GPIO to configure.
 * @param   function        GPIO function to set.
 *
 * @retval  WMS_SUCCESS             succeeded
 * @retval  WMS_HW_ERROR            error communicating with device
 * @retval  WMS_INVALID_PARAMETER   not a valid GPIO.
 ******************************************************************************/
WM_STATUS WMGPSetFunction( WM_DEVICE_HANDLE hDevice,
                           WM_GPIO          GPIO,
                           WM_GPIO_ALTFN    function
                         );

/*******************************************************************************
 * Function: WMGPGetFunction                                               *//**
 *
 * @brief   Returns the current GPIO alternate function.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   GPIO            GPIO to query.
 * @param   pFunction       Receives the GPIO function.
 *
 * @retval  WMS_SUCCESS             succeeded
 * @retval  WMS_HW_ERROR            error communicating with device
 * @retval  WMS_INVALID_PARAMETER   not a valid GPIO.
 ******************************************************************************/
WM_STATUS WMGPGetFunction( WM_DEVICE_HANDLE hDevice,
                           WM_GPIO          GPIO,
                           WM_GPIO_ALTFN    *pFunction
                         );


/*******************************************************************************
* Function: WMGPConfigFunction                                             *//**
*
* @brief   Configures the function of the given GPIO.
*
* @param   hDevice         The handle to the device (from WMOpenDevice).
* @param   GPIOConfig      The GPIO configuration.
*
* @retval  WMS_SUCCESS             succeeded
* @retval  WMS_HW_ERROR            error communicating with device
* @retval  WMS_INVALID_PARAMETER   not a valid GPIO.
******************************************************************************/
WM_STATUS WMGPConfigFunction( WM_DEVICE_HANDLE hDevice,
                              WM_GPIO_CONFIG   GPIOConfig
                            );
#ifdef __cplusplus
}   /* extern "C" */
#endif

#endif    /* __WMGPIO_H__ */
/*------------------------------ END OF FILE ---------------------------------*/
