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
 * @file   WMInterrupts.h
 * @brief  Interrupt functions for Wolfson devices.
 *
 * @version $Id: WMInterrupts.h 451 2007-05-02 10:09:22Z ib $
 *
 * @Warning
 *   This software is specifically written for Wolfson devices. It may not be
 *   used with other devices.
 *
 ******************************************************************************/
#ifndef __WMINTERRUPTS_H__
#define __WMINTERRUPTS_H__

/*
* Include files
*/
#include <WMStatus.h>

/*
* Definitions
*/

/**
 * Type to select the interrupt.  Each device will have its own
 * table of supported interrupts in the corresponding device header file.
 */
typedef int WM_INTERRUPT;

/**
 * A distinctive value for an invalid interrupt.
 */
#define WM_INT_INVALID          0xFFFF

/**
 * Type to specify a table of interrupts.  Each device will have its own
 * table of supported interrupts in the corresponding device header file.
 */
typedef WM_REGVAL WM_INTERRUPT_BANK;
typedef WM_INTERRUPT_BANK *WM_INTERRUPT_SET;

/*
 * Function prototypes
 */
#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Function: WMIntRegister                                                 *//**
 *
 * @brief   Registers to receive notification of the given interrupt.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   interrupt       Interrupt to register for.
 * @param   event           Event to set
 *
 * @return  The bitmask of supported GPIOs
 ******************************************************************************/
WM_STATUS WMIntRegister( WM_DEVICE_HANDLE   hDevice,
                         WM_INTERRUPT       interrupt,
                         HANDLE             hEvent
                       );

/*******************************************************************************
 * Function: WMIntDeregister                                               *//**
 *
 * @brief   Stop receiving notification of the given interrupt.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   interrupt       Interrupt to register for.
 *
 * @retval  WMS_SUCCESS          succeeded
 * @retval  WMS_HW_ERROR         error communicating with device
 ******************************************************************************/
WM_STATUS WMIntDeregister( WM_DEVICE_HANDLE hDevice, WM_INTERRUPT interrupt );

/*******************************************************************************
 * Function: WMIntEnable                                                   *//**
 *
 * @brief   Enables the interrupt.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   interrupt       Interrupt to enable.
 *
 * @retval  WMS_SUCCESS          succeeded
 * @retval  WMS_HW_ERROR         error communicating with device
 ******************************************************************************/
WM_STATUS WMIntEnable( WM_DEVICE_HANDLE hDevice, WM_INTERRUPT interrupt );

/*******************************************************************************
 * Function: WMIntDisable                                                  *//**
 *
 * @brief   Disables the interrupt.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   interrupt       Interrupt to disable.
 *
 * @retval  WMS_SUCCESS          succeeded
 * @retval  WMS_HW_ERROR         error communicating with device
 ******************************************************************************/
WM_STATUS WMIntDisable( WM_DEVICE_HANDLE hDevice, WM_INTERRUPT interrupt );

/*******************************************************************************
 * Function: WMIntIsEnabled                                                *//**
 *
 * @brief   Receives whether the interrupt is enabled or disabled.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   interrupt       Interrupt to query.
 * @param   pEnabled        TRUE = enabled, FALSE = disabled.
 *
 * @retval  WMS_SUCCESS          succeeded
 * @retval  WMS_HW_ERROR         error communicating with device
 ******************************************************************************/
WM_STATUS WMIntIsEnabled( WM_DEVICE_HANDLE  hDevice,
                          WM_INTERRUPT      interrupt,
                          BOOL              *pEnabled
                        );

/*******************************************************************************
 * Function: WMIntEnableSet                                                *//**
 *
 * @brief   Enables the interrupts in the set
 *
 * Note each device will define its own interrupt structure.  It will also
 * define a union for converting to and from a WM_INTERRUPT_SET.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   interrupts      Interrupts to enable.
 *
 * @retval  WMS_SUCCESS          succeeded
 * @retval  WMS_HW_ERROR         error communicating with device
 ******************************************************************************/
WM_STATUS WMIntEnableSet( WM_DEVICE_HANDLE          hDevice,
                          const WM_INTERRUPT_SET    pInterrupts
                        );

/*******************************************************************************
 * Function: WMIntDisableSet                                                *//**
 *
 * @brief   Disables the interrupts in the set
 *
 * Note each device will define its own interrupt structure.  It will also
 * define a union for converting to and from a WM_INTERRUPT_SET.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   interrupts      Interrupts to disable.
 *
 * @retval  WMS_SUCCESS          succeeded
 * @retval  WMS_HW_ERROR         error communicating with device
 ******************************************************************************/
WM_STATUS WMIntDisableSet( WM_DEVICE_HANDLE         hDevice,
                           const WM_INTERRUPT_SET   pInterrupts
                         );

/*******************************************************************************
 * Function: WMIntGetEnableSet                                             *//**
 *
 * @brief   Receives the set of enabled and disabled interrupts.
 *
 * Note each device will define its own interrupt structure.  It will also
 * define a union for converting to and from a WM_INTERRUPT_SET.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   pInterrupts     Receives the set of enabled interrupts.
 *
 * @retval  WMS_SUCCESS          succeeded
 * @retval  WMS_HW_ERROR         error communicating with device
 ******************************************************************************/
WM_STATUS WMIntGetEnableSet( WM_DEVICE_HANDLE hDevice,
                             WM_INTERRUPT_SET pInterrupts
                           );

/*******************************************************************************
 * Function: WMIntProcessInterrupts                                        *//**
 *
 * @brief   Processes the interrupts and sets corresponding events.
 *
 * This function runs through all the registered events.  For each flagged
 * interrupt which has an associated event, the function will set the event
 * and disable the interrupt.
 *
 * To receive subsequent interrupts, the client must call WMIntHandled for the
 * given interrupt.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   pInterrupts     interrupt set returned from WMIntGetStatus.
 *
 * @retval  WMS_SUCCESS         succeeded
 * @retval  WMS_HW_ERROR        error communicating with device
 ******************************************************************************/
WM_STATUS WMIntProcessInterrupts( WM_DEVICE_HANDLE hDevice,
                                  WM_INTERRUPT_SET pInterrupts
                                );

/*******************************************************************************
 * Function: WMIntGetAndProcessInterrupts                                  *//**
 *
 * @brief   Processes the interrupts and sets corresponding events.
 *
 * This function gets the current interrupt status and runs through all the
 * registered events.  For each flagged interrupt which has an associated event,
 * the function will set the event and disable the interrupt.
 *
 * To receive subsequent interrupts, the client must call WMIntHandled for the
 * given interrupt.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 *
 * @retval  WMS_SUCCESS          succeeded
 * @retval  WMS_HW_ERROR         error communicating with device
 ******************************************************************************/
WM_STATUS WMIntGetAndProcessInterrupts( WM_DEVICE_HANDLE hDevice );

/*******************************************************************************
 * Function: WMIntHandled                                                  *//**
 *
 * @brief   Indicates the interrupt has been handled and can be re-enabled.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   interrupt       Interrupt to re-enable.
 *
 * @retval  WMS_SUCCESS          succeeded
 * @retval  WMS_HW_ERROR         error communicating with device
 ******************************************************************************/
WM_STATUS WMIntHandled( WM_DEVICE_HANDLE    hDevice,
                        WM_INTERRUPT        interrupt
                      );

/*******************************************************************************
 * Function: WMIntGetStatus                                                *//**
 *
 * @brief   Reads the current status of all interrupts.
 *
 * Note this will also clear the interrupts.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   pInterrupts     Receives the interrupt set.  Can be NULL.
 *
 * @retval  WMS_SUCCESS          succeeded
 * @retval  WMS_HW_ERROR         error communicating with device
 ******************************************************************************/
WM_STATUS WMIntGetStatus( WM_DEVICE_HANDLE hDevice,
                          WM_INTERRUPT_SET pInterrupts
                        );

/*******************************************************************************
 * Function: WMIntGetStatusAndMask                                         *//**
 *
 * @brief   Reads the current status of all interrupts.
 *
 * Note this will also clear and disable the interrupts.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 * @param   pInterrupts     Receives the interrupt set.  Can be NULL.
 *
 * @retval  WMS_SUCCESS          succeeded
 * @retval  WMS_HW_ERROR         error communicating with device
 ******************************************************************************/
WM_STATUS WMIntGetStatusAndMask( WM_DEVICE_HANDLE hDevice,
                                 WM_INTERRUPT_SET pInterrupts
                               );

/*******************************************************************************
 * Function: WMIntClearAll                                                 *//**
 *
 * @brief   Clears all interrupts.
 *
 * @param   hDevice         The handle to the device (from WMOpenDevice).
 *
 * @retval  WMS_SUCCESS          succeeded
 * @retval  WMS_HW_ERROR         error communicating with device
 ******************************************************************************/
WM_STATUS WMIntClearAll( WM_DEVICE_HANDLE hDevice );

#ifdef __cplusplus
}   /* extern "C" */
#endif

#endif    /* __WMINTERRUPTS_H__ */
/*------------------------------ END OF FILE ---------------------------------*/
