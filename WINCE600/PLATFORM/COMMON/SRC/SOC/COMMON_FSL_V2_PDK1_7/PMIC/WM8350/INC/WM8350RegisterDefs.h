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
 * may appear in the software or any related documention.
 *
 * Except as permitted by the agreement(s), no part of the software or any
 * related documention may be reproduced, stored in a retrieval system, or
 * transmitted in any form or by any means without the express written
 * consent of Wolfson Microelectronics plc.
 *                                                                         *//**
 * @file   WM8350RegisterDefs.h
 * @brief  Definitions for registers and fields on the Wolfson WM8350 Rev E.
 *
 * @version $Id: WM8350RegisterDefs.h 702 2007-07-06 09:33:30Z fb $
 *
 * @Warning
 *   This software is specifically written for Wolfson devices. It may not be
 *   used with other devices.
 *
 ******************************************************************************/
#ifndef __WM8350REGISTERDEFS_H__
#define __WM8350REGISTERDEFS_H__

/*
 * Include the public definitions.
 */
#include "WM8350.h"

/*
 * Register values.
 */
#define WM8350_RESET_ID                         0x00
#define WM8350_ID                               0x01
#define WM8350_SYSTEM_CONTROL_1                 0x03
#define WM8350_SYSTEM_CONTROL_2                 0x04
#define WM8350_SYSTEM_HIBERNATE                 0x05
#define WM8350_INTERFACE_CONTROL                0x06
#define WM8350_POWER_MGMT_1                     0x08
#define WM8350_POWER_MGMT_2                     0x09
#define WM8350_POWER_MGMT_3                     0x0A
#define WM8350_POWER_MGMT_4                     0x0B
#define WM8350_POWER_MGMT_5                     0x0C
#define WM8350_POWER_MGMT_6                     0x0D
#define WM8350_POWER_MGMT_7                     0x0E
#define WM8350_RTC_SECONDS_MINUTES              0x10
#define WM8350_RTC_HOURS_DAY                    0x11
#define WM8350_RTC_DATE_MONTH                   0x12
#define WM8350_RTC_YEAR                         0x13
#define WM8350_ALARM_SECONDS_MINUTES            0x14
#define WM8350_ALARM_HOURS_DAY                  0x15
#define WM8350_ALARM_DATE_MONTH                 0x16
#define WM8350_RTC_TIME_CONTROL                 0x17
#define WM8350_SYSTEM_INTERRUPTS                0x18
#define WM8350_INTERRUPT_STATUS_1               0x19
#define WM8350_INTERRUPT_STATUS_2               0x1A
#define WM8350_UNDER_VOLTAGE_INTERRUPT_STATUS   0x1C
#define WM8350_OVER_CURRENT_INTERRUPT_STATUS    0x1D
#define WM8350_GPIO_INTERRUPT_STATUS            0x1E
#define WM8350_COMPARATOR_INTERRUPT_STATUS      0x1F
#define WM8350_SYSTEM_INTERRUPTS_MASK           0x20
#define WM8350_INTERRUPT_STATUS_1_MASK          0x21
#define WM8350_INTERRUPT_STATUS_2_MASK          0x22
#define WM8350_UNDER_VOLTAGE_INTERRUPT_STATUS_MASK 0x24
#define WM8350_OVER_CURRENT_INTERRUPT_STATUS_MASK 0x25
#define WM8350_GPIO_INTERRUPT_STATUS_MASK       0x26
#define WM8350_COMPARATOR_INTERRUPT_STATUS_MASK 0x27
#define WM8350_CLOCK_CONTROL_1                  0x28
#define WM8350_CLOCK_CONTROL_2                  0x29
#define WM8350_FLL_CONTROL_1                    0x2A
#define WM8350_FLL_CONTROL_2                    0x2B
#define WM8350_FLL_CONTROL_3                    0x2C
#define WM8350_FLL_CONTROL_4                    0x2D
#define WM8350_DAC_CONTROL                      0x30
#define WM8350_DAC_DIGITAL_VOLUME_L             0x32
#define WM8350_DAC_DIGITAL_VOLUME_R             0x33
#define WM8350_DAC_LR_RATE                      0x35
#define WM8350_DAC_CLOCK_CONTROL                0x36
#define WM8350_DAC_MUTE                         0x3A
#define WM8350_DAC_MUTE_VOLUME                  0x3B
#define WM8350_DAC_SIDE                         0x3C
#define WM8350_ADC_CONTROL                      0x40
#define WM8350_ADC_DIGITAL_VOLUME_L             0x42
#define WM8350_ADC_DIGITAL_VOLUME_R             0x43
#define WM8350_ADC_DIVIDER                      0x44
#define WM8350_ADC_LR_RATE                      0x46
#define WM8350_INPUT_CONTROL                    0x48
#define WM8350_IN3_INPUT_CONTROL                0x49
#define WM8350_MIC_BIAS_CONTROL                 0x4A
#define WM8350_OUTPUT_CONTROL                   0x4C
#define WM8350_JACK_DETECT                      0x4D
#define WM8350_ANTI_POP_CONTROL                 0x4E
#define WM8350_LEFT_INPUT_VOLUME                0x50
#define WM8350_RIGHT_INPUT_VOLUME               0x51
#define WM8350_LEFT_MIXER_CONTROL               0x58
#define WM8350_RIGHT_MIXER_CONTROL              0x59
#define WM8350_OUT3_MIXER_CONTROL               0x5C
#define WM8350_OUT4_MIXER_CONTROL               0x5D
#define WM8350_OUTPUT_LEFT_MIXER_VOLUME         0x60
#define WM8350_OUTPUT_RIGHT_MIXER_VOLUME        0x61
#define WM8350_INPUT_MIXER_VOLUME_L             0x62
#define WM8350_INPUT_MIXER_VOLUME_R             0x63
#define WM8350_INPUT_MIXER_VOLUME               0x64
#define WM8350_OUT1L_VOLUME                     0x68
#define WM8350_OUT1R_VOLUME                     0x69
#define WM8350_OUT2L_VOLUME                     0x6A
#define WM8350_OUT2R_VOLUME                     0x6B
#define WM8350_BEEP_VOLUME                      0x6F
#define WM8350_AI_FORMATTING                    0x70
#define WM8350_ADC_DAC_COMP                     0x71
#define WM8350_AI_ADC_CONTROL                   0x72
#define WM8350_AI_DAC_CONTROL                   0x73
#define WM8350_GPIO_DEBOUNCE                    0x80
#define WM8350_GPIO_PIN_PULL_UP_CONTROL         0x81
#define WM8350_GPIO_PULL_DOWN_CONTROL           0x82
#define WM8350_GPIO_INTERRUPT_MODE              0x83
#define WM8350_GPIO_CONTROL                     0x85
#define WM8350_GPIO_CONFIGURATION_I_O           0x86
#define WM8350_GPIO_PIN_POLARITY_TYPE           0x87
#define WM8350_GPIO_FUNCTION_SELECT_1           0x8C
#define WM8350_GPIO_FUNCTION_SELECT_2           0x8D
#define WM8350_GPIO_FUNCTION_SELECT_3           0x8E
#define WM8350_GPIO_FUNCTION_SELECT_4           0x8F
#define WM8350_DIGITISER_CONTROL_1              0x90
#define WM8350_DIGITISER_CONTROL_2              0x91
#define WM8350_AUX1_READBACK                    0x98
#define WM8350_AUX2_READBACK                    0x99
#define WM8350_AUX3_READBACK                    0x9A
#define WM8350_AUX4_READBACK                    0x9B
#define WM8350_USB_VOLTAGE_READBACK             0x9C
#define WM8350_LINE_VOLTAGE_READBACK            0x9D
#define WM8350_BATT_VOLTAGE_READBACK            0x9E
#define WM8350_CHIP_TEMP_READBACK               0x9F
#define WM8350_GENERIC_COMPARATOR_CONTROL       0xA3
#define WM8350_GENERIC_COMPARATOR_1             0xA4
#define WM8350_GENERIC_COMPARATOR_2             0xA5
#define WM8350_GENERIC_COMPARATOR_3             0xA6
#define WM8350_GENERIC_COMPARATOR_4             0xA7
#define WM8350_BATTERY_CHARGER_CONTROL_1        0xA8
#define WM8350_BATTERY_CHARGER_CONTROL_2        0xA9
#define WM8350_BATTERY_CHARGER_CONTROL_3        0xAA
#define WM8350_CURRENT_SINK_DRIVER_A            0xAC
#define WM8350_CSA_FLASH_CONTROL                0xAD
#define WM8350_CURRENT_SINK_DRIVER_B            0xAE
#define WM8350_CSB_FLASH_CONTROL                0xAF
#define WM8350_DCDC_LDO_REQUESTED               0xB0
#define WM8350_DCDC_ACTIVE_OPTIONS              0xB1
#define WM8350_DCDC_SLEEP_OPTIONS               0xB2
#define WM8350_POWER_CHECK_COMPARATOR           0xB3
#define WM8350_DCDC1_CONTROL                    0xB4
#define WM8350_DCDC1_TIMEOUTS                   0xB5
#define WM8350_DCDC1_LOW_POWER                  0xB6
#define WM8350_DCDC2_CONTROL                    0xB7
#define WM8350_DCDC2_TIMEOUTS                   0xB8
#define WM8350_DCDC3_CONTROL                    0xBA
#define WM8350_DCDC3_TIMEOUTS                   0xBB
#define WM8350_DCDC3_LOW_POWER                  0xBC
#define WM8350_DCDC4_CONTROL                    0xBD
#define WM8350_DCDC4_TIMEOUTS                   0xBE
#define WM8350_DCDC4_LOW_POWER                  0xBF
#define WM8350_DCDC5_CONTROL                    0xC0
#define WM8350_DCDC5_TIMEOUTS                   0xC1
#define WM8350_DCDC6_CONTROL                    0xC3
#define WM8350_DCDC6_TIMEOUTS                   0xC4
#define WM8350_DCDC6_LOW_POWER                  0xC5
#define WM8350_LIMIT_SWITCH_CONTROL             0xC7
#define WM8350_LDO1_CONTROL                     0xC8
#define WM8350_LDO1_TIMEOUTS                    0xC9
#define WM8350_LDO1_LOW_POWER                   0xCA
#define WM8350_LDO2_CONTROL                     0xCB
#define WM8350_LDO2_TIMEOUTS                    0xCC
#define WM8350_LDO2_LOW_POWER                   0xCD
#define WM8350_LDO3_CONTROL                     0xCE
#define WM8350_LDO3_TIMEOUTS                    0xCF
#define WM8350_LDO3_LOW_POWER                   0xD0
#define WM8350_LDO4_CONTROL                     0xD1
#define WM8350_LDO4_TIMEOUTS                    0xD2
#define WM8350_LDO4_LOW_POWER                   0xD3
#define WM8350_VCC_FAULT_MASKS                  0xD7
#define WM8350_MAIN_BANDGAP_CONTROL             0xD8
#define WM8350_OSC_CONTROL                      0xD9
#define WM8350_RTC_TICK_CONTROL                 0xDA
#define WM8350_SECURITY                         0xDB
#define WM8350_SIGNAL_OVERRIDES                 0xE0
#define WM8350_DCDC_LDO_STATUS                  0xE1
#define WM8350_CHARGER_OVERIDES_STATUS          0xE2
#define WM8350_MISC_OVERRIDES                   0xE3
#define WM8350_SUPPLY_OVERRIDES_STATUS_1        0xE4
#define WM8350_SUPPLY_OVERRIDES_STATUS_2        0xE5
#define WM8350_GPIO_PIN_STATUS                  0xE6
#define WM8350_COMPARATOR_OVERRIDES             0xE7
#define WM8350_STATE_MACHINE_STATUS             0xE9
#define WM8350_DCDC1_TEST_CONTROLS              0xF8
#define WM8350_DCDC3_TEST_CONTROLS              0xFA
#define WM8350_DCDC4_TEST_CONTROLS              0xFB
#define WM8350_DCDC6_TEST_CONTROLS              0xFD

#define WM8350_REGISTER_COUNT                   164
#define WM8350_MAX_REGISTER                     0xFD
/*
 * Field Definitions.
 */
#ifndef _WMACCESSORS_DEFINED_

typedef unsigned short WM_REGVAL;

/*******************************************************************************
 * Macro:  WM_FIELD_VAL                                                    *//**
 *
 * @brief  Extracts the value of the field from the register.
 *
 * This is the absolute field value after right-shifting to bring it down to
 * a zero-base.  I.e. if the regval is 0x45C1, and the field WM1234_FOO is [10:8]
 * WM_FIELD_VAL( regval, WM1234_FOO ) would return 5.
 *
 * @param _regval   Register value.
 * @param _field    Field base name.
 *
 * @return The zero-based value of the field.
 ******************************************************************************/
#   define WM_FIELD_VAL( _field, _regval )      (((_regval) & _field##_MASK) >> _field##_SHIFT)

/*******************************************************************************
 * Macro:  WM_FIELD_REGVAL                                                 *//**
 *
 * @brief  Generates the register value for a field value.
 *
 * This is the field value which can be used in building up a register value.
 * I.e. if the fieldval is 5, and the field WM1234_FOO is [10:8]
 * WM_FIELD_REGVAL( field, WM1234_FOO ) would return 0x0500.
 *
 * @param _fieldval Field value (zero-based).
 * @param _field    Field base name.
 *
 * @return The value of the field in the register.
 ******************************************************************************/
#   define WM_FIELD_REGVAL( _field, _fieldval ) (((_fieldval) << _field##_SHIFT) & _field##_MASK )

/*******************************************************************************
 * Macro:  WM_FIELD_CLEAR                                                  *//**
 *
 * @brief  Sets the given field to zero in the register.
 *
 * @param _regval   Register value.
 * @param _field    Field base name.
 *
 * @return void.
 ******************************************************************************/
#   define WM_FIELD_CLEAR( _regval, _field )    ( (_regval) &= ~_field##_MASK )

/*******************************************************************************
 * Macro:  WM_FIELD_SET                                                    *//**
 *
 * @brief  Sets the given field to the given value.
 *
 * This is the absolute field value after right-shifting to bring it down to
 * a zero-base.  I.e. if the regval is 0x45C1, and the field WM1234_FOO is [10:8]
 * WM_FIELD_VAL( regval, WM1234_FOO ) would return 5.
 *
 * @param _regval   Register value.
 * @param _field    Field base name.
 * @param _val      Field value (zero-based).
 *
 * @return The new register value.
 ******************************************************************************/
#   define WM_FIELD_SET( _regval, _field, _val )( (_regval) = ((_regval) & ~_field##_MASK) | WM_FIELD_REGVAL(_val, _field) )
#endif /* _WMACCESSORS_DEFINED_ */

/*
 * R0 (0x00) - Reset/ID
 */
#define WM8350_SW_RESET_CHIP_ID_MASK            0xFFFF  /* SW_RESET/CHIP_ID - [15:0] */

/*
 * R1 (0x01) - ID
 */
#define WM8350_CHIP_REV_MASK                    0x7000  /* CHIP_REV - [14:12] */
#define WM8350_CHIP_REV_SHIFT                       12  /* CHIP_REV - [14:12] */
#define WM8350_CONF_STS_MASK                    0x0C00  /* CONF_STS - [11:10] */
#define WM8350_CONF_STS_SHIFT                       10  /* CONF_STS - [11:10] */
#define WM8350_CUST_ID_MASK                     0x00FF  /* CUST_ID - [7:0] */
#define WM8350_CUST_ID_SHIFT                         0  /* CUST_ID - [7:0] */

/*
 * R3 (0x03) - System Control 1
 */
#define WM8350_CHIP_ON                          0x8000  /* CHIP_ON */
#define WM8350_SYS_RST                          0x4000  /* SYS_RST */
#define WM8350_POWERCYCLE                       0x2000  /* POWERCYCLE */
#define WM8350_VCC_FAULT_OV                     0x1000  /* VCC_FAULT_OV */
#define WM8350_RSTB_TO_MASK                     0x0C00  /* RSTB_TO - [11:10] */
#define WM8350_BG_SLEEP                         0x0200  /* BG_SLEEP */
#define WM8350_WDOG_DEBUG                       0x0080  /* WDOG_DEBUG */
#define WM8350_WDOG_DEBUG_MASK                  0x0080  /* WDOG_DEBUG */
#define WM8350_WDOG_DEBUG_SHIFT                      7  /* WDOG_DEBUG */
#define WM8350_MEM_VALID                        0x0020  /* MEM_VALID */
#define WM8350_CHIP_SET_UP                      0x0010  /* CHIP_SET_UP */
#define WM8350_ON_DEB_T                         0x0008  /* ON_DEB_T */
#define WM8350_ON_POL                           0x0002  /* ON_POL */
#define WM8350_IRQ_POL                          0x0001  /* IRQ_POL */

/*
 * R4 (0x04) - System Control 2
 */
#define WM8350_USB_SUSPEND_8MA                  0x8000  /* USB_SUSPEND_8MA */
#define WM8350_USB_SUSPEND_8MA_MASK             0x8000  /* USB_SUSPEND_8MA */
#define WM8350_USB_SUSPEND_8MA_SHIFT                15  /* USB_SUSPEND_8MA */
#define WM8350_USB_SUSPEND                      0x4000  /* USB_SUSPEND */
#define WM8350_USB_SUSPEND_MASK                 0x4000  /* USB_SUSPEND */
#define WM8350_USB_SUSPEND_SHIFT                    14  /* USB_SUSPEND */
#define WM8350_USB_MSTR                         0x2000  /* USB_MSTR */
#define WM8350_USB_MSTR_MASK                    0x2000  /* USB_MSTR */
#define WM8350_USB_MSTR_SHIFT                       13  /* USB_MSTR */
#define WM8350_USB_MSTR_SRC                     0x1000  /* USB_MSTR_SRC */
#define WM8350_USB_MSTR_SRC_MASK                0x1000  /* USB_MSTR_SRC */
#define WM8350_USB_MSTR_SRC_SHIFT                   12  /* USB_MSTR_SRC */
#define WM8350_USB_MSTR_500MA                   0x0800  /* USB_MSTR_500MA */
#define WM8350_USB_MSTR_500MA_MASK              0x0800  /* USB_MSTR_500MA */
#define WM8350_USB_MSTR_500MA_SHIFT                 11  /* USB_MSTR_500MA */
#define WM8350_USB_NOLIM                        0x0400  /* USB_NOLIM */
#define WM8350_USB_NOLIM_MASK                   0x0400  /* USB_NOLIM */
#define WM8350_USB_NOLIM_SHIFT                      10  /* USB_NOLIM */
#define WM8350_USB_SLV_500MA                    0x0200  /* USB_SLV_500MA */
#define WM8350_USB_SLV_500MA_MASK               0x0200  /* USB_SLV_500MA */
#define WM8350_USB_SLV_500MA_SHIFT                   9  /* USB_SLV_500MA */
#define WM8350_WDOG_HIB_MODE                    0x0080  /* WDOG_HIB_MODE */
#define WM8350_WDOG_HIB_MODE_MASK               0x0080  /* WDOG_HIB_MODE */
#define WM8350_WDOG_HIB_MODE_SHIFT                   7  /* WDOG_HIB_MODE */
#define WM8350_WDOG_DEBUG_REVB                  0x0040  /* WDOG_DEBUG */
#define WM8350_WDOG_DEBUG_REVB_MASK             0x0040  /* WDOG_DEBUG */
#define WM8350_WDOG_DEBUG_REVB_SHIFT                 6  /* WDOG_DEBUG */
#define WM8350_WDOG_MODE_MASK                   0x0030  /* WDOG_MODE - [5:4] */
#define WM8350_WDOG_MODE_SHIFT                       4  /* WDOG_MODE - [5:4] */
#define WM8350_WDOG_TO_MASK                     0x0007  /* WDOG_TO - [2:0] */
#define WM8350_WDOG_TO_SHIFT                         0  /* WDOG_TO - [2:0] */

/* Bit definitions for R4 (0x$) */
#define WM8350_WDOG_HIB_MODE_DISABLED                0  /* Disabled in HIBERNATE state */
#define WM8350_WDOG_HIB_MODE_ENABLED                 1  /* Enabled in HIBERNATE state */

#define WM8350_WDOG_DEBUG_RUN                        0  /* Watchdog running */
#define WM8350_WDOG_DEBUG_PAUSE                      1  /* Watchdog paused for debugging */

#define WM8350_WDOG_MODE_DISABLED                    0  /* Disabled */
#define WM8350_WDOG_MODE_INTERRUPT                   1  /* Generate interrupt on timeout */
#define WM8350_WDOG_MODE_RESET                       2  /* Reset on timeout */
#define WM8350_WDOG_MODE_INT_THEN_RESET              3  /* Interrupt on first timeout, reset on second */

#define WM8350_WDOG_TO_125MS                         0  /* 125ms timeout */
#define WM8350_WDOG_TO_250MS                         1  /* 250ms timeout */
#define WM8350_WDOG_TO_500MS                         2  /* 500ms timeout */
#define WM8350_WDOG_TO_1S                            3  /* 1s timeout */
#define WM8350_WDOG_TO_2S                            4  /* 2s timeout */
#define WM8350_WDOG_TO_4S                            5  /* 4s timeout */

/*
 * R5 (0x05) - System Hibernate
 */
#define WM8350_HIBERNATE                        0x8000  /* HIBERNATE */
#define WM8350_WDOG_HIB_MODE                    0x0080  /* WDOG_HIB_MODE */
#define WM8350_HIB_STARTUP_SEQ                  0x0040  /* HIB_STARTUP_SEQ */
#define WM8350_REG_RESET_HIB_MODE               0x0020  /* REG_RESET_HIB_MODE */
#define WM8350_RST_HIB_MODE                     0x0010  /* RST_HIB_MODE */
#define WM8350_IRQ_HIB_MODE                     0x0008  /* IRQ_HIB_MODE */
#define WM8350_MEMRST_HIB_MODE                  0x0004  /* MEMRST_HIB_MODE */
#define WM8350_PCCOMP_HIB_MODE                  0x0002  /* PCCOMP_HIB_MODE */
#define WM8350_TEMPMON_HIB_MODE                 0x0001  /* TEMPMON_HIB_MODE */

/*
 * R6 (0x06) - Interface Control
 */
#define WM8350_USE_DEV_PINS                     0x8000  /* USE_DEV_PINS */
#define WM8350_USE_DEV_PINS_MASK                0x8000  /* USE_DEV_PINS */
#define WM8350_USE_DEV_PINS_SHIFT                   15  /* USE_DEV_PINS */
#define WM8350_DEV_ADDR_MASK                    0x6000  /* DEV_ADDR - [14:13] */
#define WM8350_DEV_ADDR_SHIFT                       13  /* DEV_ADDR - [14:13] */
#define WM8350_CONFIG_DONE                      0x1000  /* CONFIG_DONE */
#define WM8350_CONFIG_DONE_MASK                 0x1000  /* CONFIG_DONE */
#define WM8350_CONFIG_DONE_SHIFT                    12  /* CONFIG_DONE */
#define WM8350_RECONFIG_AT_ON                   0x0800  /* RECONFIG_AT_ON */
#define WM8350_RECONFIG_AT_ON_MASK              0x0800  /* RECONFIG_AT_ON */
#define WM8350_RECONFIG_AT_ON_SHIFT                 11  /* RECONFIG_AT_ON */
#define WM8350_AUTOINC                          0x0200  /* AUTOINC */
#define WM8350_AUTOINC_MASK                     0x0200  /* AUTOINC */
#define WM8350_AUTOINC_SHIFT                         9  /* AUTOINC */
#define WM8350_ARA                              0x0100  /* ARA */
#define WM8350_ARA_MASK                         0x0100  /* ARA */
#define WM8350_ARA_SHIFT                             8  /* ARA */
#define WM8350_SPI_CFG                          0x0008  /* SPI_CFG */
#define WM8350_SPI_CFG_MASK                     0x0008  /* SPI_CFG */
#define WM8350_SPI_CFG_SHIFT                         3  /* SPI_CFG */
#define WM8350_SPI_4WIRE                        0x0004  /* SPI_4WIRE */
#define WM8350_SPI_4WIRE_MASK                   0x0004  /* SPI_4WIRE */
#define WM8350_SPI_4WIRE_SHIFT                       2  /* SPI_4WIRE */
#define WM8350_SPI_3WIRE                        0x0002  /* SPI_3WIRE */
#define WM8350_SPI_3WIRE_MASK                   0x0002  /* SPI_3WIRE */
#define WM8350_SPI_3WIRE_SHIFT                       1  /* SPI_3WIRE */

/* Bit values for R06 (0x06) */
#define WM8350_USE_DEV_PINS_PRIMARY                  0  /* Primary control interface */
#define WM8350_USE_DEV_PINS_DEV                      1  /* Secondary control interface */

#define WM8350_DEV_ADDR_34                           0  /* 2-wire address 0x1A/0x34 */
#define WM8350_DEV_ADDR_36                           1  /* 2-wire address 0x1B/0x36 */
#define WM8350_DEV_ADDR_3C                           2  /* 2-wire address 0x1E/0x3C */
#define WM8350_DEV_ADDR_3E                           3  /* 2-wire address 0x1F/0x3E */

#define WM8350_CONFIG_DONE_OFF                       0  /* Not programmed */
#define WM8350_CONFIG_DONE_DONE                      1  /* Programming complete */

#define WM8350_RECONFIG_AT_ON_OFF                    0  /* Don't reset registers on ON-event */
#define WM8350_RECONFIG_AT_ON_ON                     1  /* Reset registers on ON-event */

#define WM8350_AUTOINC_OFF                           0  /* No register auto-increment */
#define WM8350_AUTOINC_ON                            1  /* Enable register auto-increment */

#define WM8350_ARA_OFF                               0  /* Alert response address disabled */
#define WM8350_ARA_ON                                1  /* Alert response address enabled */

#define WM8350_SPI_CFG_CMOS                          0  /* SDOUT is CMOS */
#define WM8350_SPI_CFG_OD                            1  /* SDOUT is open-drain */

#define WM8350_SPI_4WIRE_OFF                         0  /* 3-wire SPI or I2C depending on SPI_3WIRE */
#define WM8350_SPI_4WIRE_ON                          1  /* 4-wire SPI */

#define WM8350_SPI_3WIRE_I2C                         0  /* Select 2-wire interface */
#define WM8350_SPI_3WIRE_SPI                         1  /* Select 3-wire interface */

/*
 * R8 (0x08) - Power mgmt (1)
 */
#define WM8350_CODEC_ISEL_MASK                  0xC000  /* CODEC_ISEL - [15:14] */
#define WM8350_CODEC_ISEL_SHIFT                     14  /* CODEC_ISEL - [15:14] */
#define WM8350_VBUF_ENA_R8                      0x2000  /* VBUFEN */
#define WM8350_OUTPUT_DRAIN_ENA_R8              0x0400  /* OUTPUT_DRAIN_EN */
#define WM8350_MIC_DET_ENA_R8                   0x0100  /* MIC_DET_ENA */
#define WM8350_BIAS_ENA_R8                      0x0020  /* BIASEN */
#define WM8350_MICB_ENA_R8                      0x0010  /* MICBEN */
#define WM8350_VMID_ENA_R8                      0x0004  /* VMIDEN */
#define WM8350_VMID_MASK                        0x0003  /* VMID - [1:0] */
#define WM8350_VMID_SHIFT                            0  /* VMID - [1:0] */

/* Bit values for R08 (0x08) */
#define WM8350_CODEC_ISEL_1_5                        0  /* 0 = x1.5 */
#define WM8350_CODEC_ISEL_1_0                        1  /* 1 = x1.0 */
#define WM8350_CODEC_ISEL_0_75                       2  /* 2 = x0.75 */
#define WM8350_CODEC_ISEL_0_5                        3  /* 3 = x0.5 */

#define WM8350_VMID_OFF                              0  /* 0 = off */
#define WM8350_VMID_500K                             1  /* 1 = 500k resistor string */
#define WM8350_VMID_100K                             2  /* 2 = 100k resistor string */
#define WM8350_VMID_10K                              3  /* 3 = 10k resistor string */

/*
 * R9 (0x09) - Power mgmt (2)
 */
#define WM8350_IN3R_ENA_R9                      0x0800  /* IN3R_ENA */
#define WM8350_IN3L_ENA_R9                      0x0400  /* IN3L_ENA */
#define WM8350_INR_ENA_R9                       0x0200  /* INR_ENA */
#define WM8350_INL_ENA_R9                       0x0100  /* INL_ENA */
#define WM8350_MIXINR_ENA_R9                    0x0080  /* MIXINR_ENA */
#define WM8350_MIXINL_ENA_R9                    0x0040  /* MIXINL_ENA */
#define WM8350_OUT4_ENA_R9                      0x0020  /* OUT4_ENA */
#define WM8350_OUT3_ENA_R9                      0x0010  /* OUT3_ENA */
#define WM8350_MIXOUTR_ENA_R9                   0x0002  /* MIXOUTR_ENA */
#define WM8350_MIXOUTL_ENA_R9                   0x0001  /* MIXOUTL_ENA */

/*
 * R10 (0x0A) - Power mgmt (3)
 */
#define WM8350_IN3R_TO_OUT2R                    0x0080  /* IN3R_TO_OUT2R */
#define WM8350_OUT2R_ENA_R10                    0x0008  /* OUT2R_ENA */
#define WM8350_OUT2L_ENA_R10                    0x0004  /* OUT2L_ENA */
#define WM8350_OUT1R_ENA_R10                    0x0002  /* OUT1R_ENA */
#define WM8350_OUT1L_ENA_R10                    0x0001  /* OUT1L_ENA */

/*
 * R11 (0x0B) - Power mgmt (4)
 */
#define WM8350_SYSCLK_ENA_R11                   0x4000  /* SYSCLK_ENA */
#define WM8350_ADC_HPF_ENA_R11                  0x2000  /* ADC_HPF_ENA */
#define WM8350_FLL_ENA_R11                      0x0800  /* FLL_ENA */
#define WM8350_FLL_OSC_ENA_R11                  0x0400  /* FLL_OSC_ENA */
#define WM8350_TOCLK_ENA_R11                    0x0100  /* TOCLK_ENA */
#define WM8350_DACR_ENA_R11                     0x0020  /* DACR_ENA */
#define WM8350_DACL_ENA_R11                     0x0010  /* DACL_ENA */
#define WM8350_ADCR_ENA_R11                     0x0008  /* ADCR_ENA */
#define WM8350_ADCL_ENA_R11                     0x0004  /* ADCL_ENA */

/*
 * R12 (0x0C) - Power mgmt (5)
 */
#define WM8350_CODEC_ENA_R12                    0x1000  /* CODEC_ENA */
#define WM8350_RTC_TICK_ENA_R12                 0x0800  /* RTC_TICK_ENA */
#define WM8350_OSC32K_ENA_R12                   0x0400  /* OSC32K_ENA */
#define WM8350_CHG_ENA_R12                      0x0200  /* CHG_ENA */
#define WM8350_SW_VRTC_ENA_R12                  0x0100  /* SW_VRTC_ENA */
#define WM8350_AUXADC_ENA_R12                   0x0080  /* AUXADC_ENA */
#define WM8350_DCMP4_ENA_R12                    0x0008  /* DCMP4_ENA */
#define WM8350_DCMP3_ENA_R12                    0x0004  /* DCMP3_ENA */
#define WM8350_DCMP2_ENA_R12                    0x0002  /* DCMP2_ENA */
#define WM8350_DCMP1_ENA_R12                    0x0001  /* DCMP1_ENA */

/*
 * R13 (0x0D) - Power mgmt (6)
 */
#define WM8350_LS_ENA_R13                       0x8000  /* LS_ENA */
#define WM8350_LDO4_ENA_R13                     0x0800  /* LDO4_ENA */
#define WM8350_LDO3_ENA_R13                     0x0400  /* LDO3_ENA */
#define WM8350_LDO2_ENA_R13                     0x0200  /* LDO2_ENA */
#define WM8350_LDO1_ENA_R13                     0x0100  /* LDO1_ENA */
#define WM8350_DC6_ENA_R13                      0x0020  /* DC6_ENA */
#define WM8350_DC5_ENA_R13                      0x0010  /* DC5_ENA */
#define WM8350_DC4_ENA_R13                      0x0008  /* DC4_ENA */
#define WM8350_DC3_ENA_R13                      0x0004  /* DC3_ENA */
#define WM8350_DC2_ENA_R13                      0x0002  /* DC2_ENA */
#define WM8350_DC1_ENA_R13                      0x0001  /* DC1_ENA */

/*
 * R14 (0x0E) - Power mgmt (7)
 */
#define WM8350_CS2_ENA_R14                      0x0002  /* CS2_ENA */
#define WM8350_CS1_ENA_R14                      0x0001  /* CS1_ENA */

/*
 * R16 (0x10) - RTC Seconds/Minutes
 */
#define WM8350_RTC_MINS_MASK                    0x7F00  /* RTC_MINS - [14:8] */
#define WM8350_RTC_MINS_SHIFT                        8  /* RTC_MINS - [14:8] */
#define WM8350_RTC_SECS_MASK                    0x007F  /* RTC_SECS - [6:0] */
#define WM8350_RTC_SECS_SHIFT                        0  /* RTC_SECS - [6:0] */

/*
 * R17 (0x11) - RTC Hours/Day
 */
#define WM8350_RTC_DAY_MASK                     0x0700  /* RTC_DAY - [10:8] */
#define WM8350_RTC_DAY_SHIFT                         8  /* RTC_DAY - [10:8] */
#define WM8350_RTC_HPM_MASK                     0x0020  /* RTC_HPM */
#define WM8350_RTC_HPM_SHIFT                         5  /* RTC_HPM */
#define WM8350_RTC_HRS_MASK                     0x001F  /* RTC_HRS - [4:0] */
#define WM8350_RTC_HRS_SHIFT                         0  /* RTC_HRS - [4:0] */

/* Bit values for R21 (0x15) */
#define WM8350_RTC_DAY_SUN                           1  /* 1 = Sunday */
#define WM8350_RTC_DAY_MON                           2  /* 2 = Monday */
#define WM8350_RTC_DAY_TUE                           3  /* 3 = Tuesday */
#define WM8350_RTC_DAY_WED                           4  /* 4 = Wednesday */
#define WM8350_RTC_DAY_THU                           5  /* 5 = Thursday */
#define WM8350_RTC_DAY_FRI                           6  /* 6 = Friday */
#define WM8350_RTC_DAY_SAT                           7  /* 7 = Saturday */

#define WM8350_RTC_HPM_AM                            0  /* 0 = AM */
#define WM8350_RTC_HPM_PM                            1  /* 1 = PM */


/*
 * R18 (0x12) - RTC Date/Month
 */
#define WM8350_RTC_MTH_MASK                     0x1F00  /* RTC_MTH - [12:8] */
#define WM8350_RTC_MTH_SHIFT                         8  /* RTC_MTH - [12:8] */
#define WM8350_RTC_DATE_MASK                    0x003F  /* RTC_DATE - [5:0] */
#define WM8350_RTC_DATE_SHIFT                        0  /* RTC_DATE - [5:0] */

/* Bit values for R22 (0x16) */
#define WM8350_RTC_MTH_JAN                           1  /* 1 = January */
#define WM8350_RTC_MTH_FEB                           2  /* 2 = February */
#define WM8350_RTC_MTH_MAR                           3  /* 3 = March */
#define WM8350_RTC_MTH_APR                           4  /* 4 = April */
#define WM8350_RTC_MTH_MAY                           5  /* 5 = May */
#define WM8350_RTC_MTH_JUN                           6  /* 6 = June */
#define WM8350_RTC_MTH_JUL                           7  /* 7 = July */
#define WM8350_RTC_MTH_AUG                           8  /* 8 = August */
#define WM8350_RTC_MTH_SEP                           9  /* 9 = September */
#define WM8350_RTC_MTH_OCT                          10  /* 10 = October */
#define WM8350_RTC_MTH_NOV                          11  /* 11 = November */
#define WM8350_RTC_MTH_DEC                          12  /* 12 = December */
#define WM8350_RTC_MTH_JAN_BCD                    0x01  /* 1 = January */
#define WM8350_RTC_MTH_FEB_BCD                    0x02  /* 2 = February */
#define WM8350_RTC_MTH_MAR_BCD                    0x03  /* 3 = March */
#define WM8350_RTC_MTH_APR_BCD                    0x04  /* 4 = April */
#define WM8350_RTC_MTH_MAY_BCD                    0x05  /* 5 = May */
#define WM8350_RTC_MTH_JUN_BCD                    0x06  /* 6 = June */
#define WM8350_RTC_MTH_JUL_BCD                    0x07  /* 7 = July */
#define WM8350_RTC_MTH_AUG_BCD                    0x08  /* 8 = August */
#define WM8350_RTC_MTH_SEP_BCD                    0x09  /* 9 = September */
#define WM8350_RTC_MTH_OCT_BCD                    0x10  /* 10 = October */
#define WM8350_RTC_MTH_NOV_BCD                    0x11  /* 11 = November */
#define WM8350_RTC_MTH_DEC_BCD                    0x12  /* 12 = December */

/*
 * R19 (0x13) - RTC Year
 */
#define WM8350_RTC_YHUNDREDS_MASK               0x3F00  /* RTC_YHUNDREDS - [13:8] */
#define WM8350_RTC_YHUNDREDS_SHIFT                   8  /* RTC_YHUNDREDS - [13:8] */
#define WM8350_RTC_YUNITS_MASK                  0x00FF  /* RTC_YUNITS - [7:0] */
#define WM8350_RTC_YUNITS_SHIFT                      0  /* RTC_YUNITS - [7:0] */

/*
 * R20 (0x14) - Alarm Seconds/Minutes
 */
#define WM8350_RTC_ALMMINS_MASK                 0x7F00  /* RTC_ALMMINS - [14:8] */
#define WM8350_RTC_ALMMINS_SHIFT                     8  /* RTC_ALMMINS - [14:8] */
#define WM8350_RTC_ALMSECS_MASK                 0x007F  /* RTC_ALMSECS - [6:0] */
#define WM8350_RTC_ALMSECS_SHIFT                     0  /* RTC_ALMSECS - [6:0] */

/* Bit values for R20 (0x14) */
#define WM8350_RTC_ALMMINS_DONT_CARE              0x7F  /* -1 = don't care */

#define WM8350_RTC_ALMSECS_DONT_CARE              0x7F  /* -1 = don't care */

/*
 * R21 (0x15) - Alarm Hours/Day
 */
#define WM8350_RTC_ALMDAY_MASK                  0x0F00  /* RTC_ALMDAY - [11:8] */
#define WM8350_RTC_ALMDAY_SHIFT                      8  /* RTC_ALMDAY - [11:8] */
#define WM8350_RTC_ALMHPM_MASK                  0x0020  /* RTC_ALMHPM */
#define WM8350_RTC_ALMHPM_SHIFT                      5  /* RTC_ALMHPM */
#define WM8350_RTC_ALMHRS_MASK                  0x001F  /* RTC_ALMHRS - [4:0] */
#define WM8350_RTC_ALMHRS_SHIFT                      0  /* RTC_ALMHRS - [4:0] */

/* Bit values for R21 (0x15) */
#define WM8350_RTC_ALMDAY_DONT_CARE                0xF  /* -1 = don't care */
#define WM8350_RTC_ALMDAY_SUN                        1  /* 1 = Sunday */
#define WM8350_RTC_ALMDAY_MON                        2  /* 2 = Monday */
#define WM8350_RTC_ALMDAY_TUE                        3  /* 3 = Tuesday */
#define WM8350_RTC_ALMDAY_WED                        4  /* 4 = Wednesday */
#define WM8350_RTC_ALMDAY_THU                        5  /* 5 = Thursday */
#define WM8350_RTC_ALMDAY_FRI                        6  /* 6 = Friday */
#define WM8350_RTC_ALMDAY_SAT                        7  /* 7 = Saturday */

#define WM8350_RTC_ALMHPM_AM                         0  /* 0 = AM */
#define WM8350_RTC_ALMHPM_PM                         1  /* 1 = PM */

#define WM8350_RTC_ALMHRS_DONT_CARE               0x1F  /* -1 = don't care */

/*
 * R22 (0x16) - Alarm Date/Month
 */
#define WM8350_RTC_ALMMTH_MASK                  0x1F00  /* RTC_ALMMTH - [12:8] */
#define WM8350_RTC_ALMMTH_SHIFT                      8  /* RTC_ALMMTH - [12:8] */
#define WM8350_RTC_ALMDATE_MASK                 0x003F  /* RTC_ALMDATE - [5:0] */
#define WM8350_RTC_ALMDATE_SHIFT                     0  /* RTC_ALMDATE - [5:0] */

/* Bit values for R22 (0x16) */
#define WM8350_RTC_ALMDATE_DONT_CARE              0x3F  /* -1 = don't care */

#define WM8350_RTC_ALMMTH_DONT_CARE               0x1F  /* -1 = don't care */
#define WM8350_RTC_ALMMTH_JAN                        1  /* 1 = January */
#define WM8350_RTC_ALMMTH_FEB                        2  /* 2 = February */
#define WM8350_RTC_ALMMTH_MAR                        3  /* 3 = March */
#define WM8350_RTC_ALMMTH_APR                        4  /* 4 = April */
#define WM8350_RTC_ALMMTH_MAY                        5  /* 5 = May */
#define WM8350_RTC_ALMMTH_JUN                        6  /* 6 = June */
#define WM8350_RTC_ALMMTH_JUL                        7  /* 7 = July */
#define WM8350_RTC_ALMMTH_AUG                        8  /* 8 = August */
#define WM8350_RTC_ALMMTH_SEP                        9  /* 9 = September */
#define WM8350_RTC_ALMMTH_OCT                       10  /* 10 = October */
#define WM8350_RTC_ALMMTH_NOV                       11  /* 11 = November */
#define WM8350_RTC_ALMMTH_DEC                       12  /* 12 = December */
#define WM8350_RTC_ALMMTH_JAN_BCD                 0x01  /* 1 = January */
#define WM8350_RTC_ALMMTH_FEB_BCD                 0x02  /* 2 = February */
#define WM8350_RTC_ALMMTH_MAR_BCD                 0x03  /* 3 = March */
#define WM8350_RTC_ALMMTH_APR_BCD                 0x04  /* 4 = April */
#define WM8350_RTC_ALMMTH_MAY_BCD                 0x05  /* 5 = May */
#define WM8350_RTC_ALMMTH_JUN_BCD                 0x06  /* 6 = June */
#define WM8350_RTC_ALMMTH_JUL_BCD                 0x07  /* 7 = July */
#define WM8350_RTC_ALMMTH_AUG_BCD                 0x08  /* 8 = August */
#define WM8350_RTC_ALMMTH_SEP_BCD                 0x09  /* 9 = September */
#define WM8350_RTC_ALMMTH_OCT_BCD                 0x10  /* 10 = October */
#define WM8350_RTC_ALMMTH_NOV_BCD                 0x11  /* 11 = November */
#define WM8350_RTC_ALMMTH_DEC_BCD                 0x12  /* 12 = December */

/*
 * R23 (0x17) - RTC Time Control
 */
#define WM8350_RTC_BCD                          0x8000  /* RTC_BCD */
#define WM8350_RTC_BCD_MASK                     0x8000  /* RTC_BCD */
#define WM8350_RTC_BCD_SHIFT                        15  /* RTC_BCD */
#define WM8350_RTC_12HR                         0x4000  /* RTC_12HR */
#define WM8350_RTC_12HR_MASK                    0x4000  /* RTC_12HR */
#define WM8350_RTC_12HR_SHIFT                       14  /* RTC_12HR */
#define WM8350_RTC_DST                          0x2000  /* RTC_DST */
#define WM8350_RTC_DST_MASK                     0x2000  /* RTC_DST */
#define WM8350_RTC_DST_SHIFT                        13  /* RTC_DST */
#define WM8350_RTC_SET                          0x0800  /* RTC_SET */
#define WM8350_RTC_SET_MASK                     0x0800  /* RTC_SET */
#define WM8350_RTC_SET_SHIFT                        11  /* RTC_SET */
#define WM8350_RTC_STS                          0x0400  /* RTC_STS */
#define WM8350_RTC_STS_MASK                     0x0400  /* RTC_STS */
#define WM8350_RTC_STS_SHIFT                        10  /* RTC_STS */
#define WM8350_RTC_ALMSET                       0x0200  /* RTC_ALMSET */
#define WM8350_RTC_ALMSET_MASK                  0x0200  /* RTC_ALMSET */
#define WM8350_RTC_ALMSET_SHIFT                      9  /* RTC_ALMSET */
#define WM8350_RTC_ALMSTS                       0x0100  /* RTC_ALMSTS */
#define WM8350_RTC_ALMSTS_MASK                  0x0100  /* RTC_ALMSTS */
#define WM8350_RTC_ALMSTS_SHIFT                      8  /* RTC_ALMSTS */
#define WM8350_RTC_PINT                         0x0070  /* RTC_PINT - [6:4] */
#define WM8350_RTC_PINT_MASK                    0x0070  /* RTC_PINT - [6:4] */
#define WM8350_RTC_PINT_SHIFT                        4  /* RTC_PINT - [6:4] */
#define WM8350_RTC_DSW                          0x000F  /* RTC_DSW - [3:0] */
#define WM8350_RTC_DSW_MASK                     0x000F  /* RTC_DSW - [3:0] */
#define WM8350_RTC_DSW_SHIFT                         0  /* RTC_DSW - [3:0] */

/* Bit values for R23 (0x17) */
#define WM8350_RTC_BCD_BINARY                        0  /* 0 = binary */
#define WM8350_RTC_BCD_BCD                           1  /* 1 = BCD */

#define WM8350_RTC_12HR_24HR                         0  /* 0 = 24-hour */
#define WM8350_RTC_12HR_12HR                         1  /* 1 = 12-hour */

#define WM8350_RTC_DST_DISABLED                      0  /* 0 = disabled */
#define WM8350_RTC_DST_ENABLED                       1  /* 1 = enabled */

#define WM8350_RTC_SET_RUN                           0  /* 0 = let RTC run */
#define WM8350_RTC_SET_SET                           1  /* 1 = request RTC set */

#define WM8350_RTC_STS_RUNNING                       0  /* 0 = RTC running */
#define WM8350_RTC_STS_STOPPED                       1  /* 1 = RTC stopped */

#define WM8350_RTC_ALMSET_RUN                        0  /* 0 = let RTC alarm run */
#define WM8350_RTC_ALMSET_SET                        1  /* 1 = request RTC alarm set */

#define WM8350_RTC_ALMSTS_RUNNING                    0  /* 0 = RTC alarm running */
#define WM8350_RTC_ALMSTS_STOPPED                    1  /* 1 = RTC alarm stopped */

#define WM8350_RTC_PINT_DISABLED                     0  /* 0 = disabled */
#define WM8350_RTC_PINT_SECS                         1  /* 1 = seconds rollover */
#define WM8350_RTC_PINT_MINS                         2  /* 2 = minutes rollover */
#define WM8350_RTC_PINT_HRS                          3  /* 3 = hours rollover */
#define WM8350_RTC_PINT_DAYS                         4  /* 4 = days rollover */
#define WM8350_RTC_PINT_MTHS                         5  /* 5 = months rollover */

#define WM8350_RTC_DSW_DISABLED                      0  /* 0 = disabled */
#define WM8350_RTC_DSW_1HZ                           1  /* 1 = 1Hz */
#define WM8350_RTC_DSW_2HZ                           2  /* 2 = 2Hz */
#define WM8350_RTC_DSW_4HZ                           3  /* 3 = 4Hz */
#define WM8350_RTC_DSW_8HZ                           4  /* 4 = 8Hz */
#define WM8350_RTC_DSW_16HZ                          5  /* 5 = 16Hz */
#define WM8350_RTC_DSW_32HZ                          6  /* 6 = 32Hz */
#define WM8350_RTC_DSW_64HZ                          7  /* 7 = 64Hz */
#define WM8350_RTC_DSW_128HZ                         8  /* 8 = 128Hz */
#define WM8350_RTC_DSW_256HZ                         9  /* 9 = 256Hz */
#define WM8350_RTC_DSW_512HZ                        10  /* 10 = 512Hz */
#define WM8350_RTC_DSW_1024HZ                       11  /* 11 = 1024Hz */

/*
 * R24 (0x18) - System Interrupts
 */
#define WM8350_OC_INT                           0x2000  /* OC_INT */
#define WM8350_UV_INT                           0x1000  /* UV_INT */
#define WM8350_CS_INT                           0x0200  /* CS_INT */
#define WM8350_EXT_INT                          0x0100  /* EXT_INT */
#define WM8350_CODEC_INT                        0x0080  /* CODEC_INT */
#define WM8350_GP_INT                           0x0040  /* GP_INT */
#define WM8350_AUXADC_INT                       0x0020  /* AUXADC_INT */
#define WM8350_RTC_INT                          0x0010  /* RTC_INT */
#define WM8350_SYS_INT                          0x0008  /* SYS_INT */
#define WM8350_CHG_INT                          0x0004  /* CHG_INT */
#define WM8350_USB_INT                          0x0002  /* USB_INT */
#define WM8350_WKUP_INT                         0x0001  /* WKUP_INT */

/*
 * R25 (0x19) - Interrupt Status 1
 */
#define WM8350_CHG_BATT_HOT_EINT                0x8000  /* CHG_BATT_HOT_EINT */
#define WM8350_CHG_BATT_COLD_EINT               0x4000  /* CHG_BATT_COLD_EINT */
#define WM8350_CHG_BATT_FAIL_EINT               0x2000  /* CHG_BATT_FAIL_EINT */
#define WM8350_CHG_TO_EINT                      0x1000  /* CHG_TO_EINT */
#define WM8350_CHG_END_EINT                     0x0800  /* CHG_END_EINT */
#define WM8350_CHG_START_EINT                   0x0400  /* CHG_START_EINT */
#define WM8350_CHG_FAST_RDY_EINT                0x0200  /* CHG_FAST_RDY_EINT */
#define WM8350_RTC_PER_EINT                     0x0080  /* RTC_PER_EINT */
#define WM8350_RTC_SEC_EINT                     0x0040  /* RTC_SEC_EINT */
#define WM8350_RTC_ALM_EINT                     0x0020  /* RTC_ALM_EINT */
#define WM8350_CHG_VBATT_LT_3P9_EINT            0x0004  /* CHG_VBATT_LT_3P9_EINT */
#define WM8350_CHG_VBATT_LT_3P1_EINT            0x0002  /* CHG_VBATT_LT_3P1_EINT */
#define WM8350_CHG_VBATT_LT_2P85_EINT           0x0001  /* CHG_VBATT_LT_2P85_EINT */

/*
 * R26 (0x1A) - Interrupt Status 2
 */
#define WM8350_CS1_EINT                         0x2000  /* CS1_EINT */
#define WM8350_CS2_EINT                         0x1000  /* CS2_EINT */
#define WM8350_USB_LIMIT_EINT                   0x0400  /* USB_LIMIT_EINT */
#define WM8350_AUXADC_DATARDY_EINT              0x0100  /* AUXADC_DATARDY_EINT */
#define WM8350_AUXADC_DCOMP4_EINT               0x0080  /* AUXADC_DCOMP4_EINT */
#define WM8350_AUXADC_DCOMP3_EINT               0x0040  /* AUXADC_DCOMP3_EINT */
#define WM8350_AUXADC_DCOMP2_EINT               0x0020  /* AUXADC_DCOMP2_EINT */
#define WM8350_AUXADC_DCOMP1_EINT               0x0010  /* AUXADC_DCOMP1_EINT */
#define WM8350_SYS_HYST_COMP_FAIL_EINT          0x0008  /* SYS_HYST_COMP_FAIL_EINT */
#define WM8350_SYS_CHIP_GT115_EINT              0x0004  /* SYS_CHIP_GT115_EINT */
#define WM8350_SYS_CHIP_GT140_EINT              0x0002  /* SYS_CHIP_GT140_EINT */
#define WM8350_SYS_WDOG_TO_EINT                 0x0001  /* SYS_WDOG_TO_EINT */

/*
 * R28 (0x1C) - Under Voltage Interrupt status
 */
#define WM8350_UV_LDO4_EINT                     0x0800  /* UV_LDO4_EINT */
#define WM8350_UV_LDO3_EINT                     0x0400  /* UV_LDO3_EINT */
#define WM8350_UV_LDO2_EINT                     0x0200  /* UV_LDO2_EINT */
#define WM8350_UV_LDO1_EINT                     0x0100  /* UV_LDO1_EINT */
#define WM8350_UV_DC6_EINT                      0x0020  /* UV_DC6_EINT */
#define WM8350_UV_DC5_EINT                      0x0010  /* UV_DC5_EINT */
#define WM8350_UV_DC4_EINT                      0x0008  /* UV_DC4_EINT */
#define WM8350_UV_DC3_EINT                      0x0004  /* UV_DC3_EINT */
#define WM8350_UV_DC2_EINT                      0x0002  /* UV_DC2_EINT */
#define WM8350_UV_DC1_EINT                      0x0001  /* UV_DC1_EINT */

/*
 * R29 (0x1D) - Over Current Interrupt status
 */
#define WM8350_OC_LS_EINT                       0x8000  /* OC_LS_EINT */

/*
 * R30 (0x1E) - GPIO Interrupt Status
 */
#define WM8350_GP12_EINT                        0x1000  /* GP12_EINT */
#define WM8350_GP11_EINT                        0x0800  /* GP11_EINT */
#define WM8350_GP10_EINT                        0x0400  /* GP10_EINT */
#define WM8350_GP9_EINT                         0x0200  /* GP9_EINT */
#define WM8350_GP8_EINT                         0x0100  /* GP8_EINT */
#define WM8350_GP7_EINT                         0x0080  /* GP7_EINT */
#define WM8350_GP6_EINT                         0x0040  /* GP6_EINT */
#define WM8350_GP5_EINT                         0x0020  /* GP5_EINT */
#define WM8350_GP4_EINT                         0x0010  /* GP4_EINT */
#define WM8350_GP3_EINT                         0x0008  /* GP3_EINT */
#define WM8350_GP2_EINT                         0x0004  /* GP2_EINT */
#define WM8350_GP1_EINT                         0x0002  /* GP1_EINT */
#define WM8350_GP0_EINT                         0x0001  /* GP0_EINT */

/*
 * R31 (0x1F) - Comparator Interrupt Status
 */
#define WM8350_EXT_USB_FB_EINT                  0x8000  /* EXT_USB_FB_EINT */
#define WM8350_EXT_WALL_FB_EINT                 0x4000  /* EXT_WALL_FB_EINT */
#define WM8350_EXT_BATT_FB_EINT                 0x2000  /* EXT_BATT_FB_EINT */
#define WM8350_CODEC_JCK_DET_L_EINT             0x0800  /* CODEC_JCK_DET_L_EINT */
#define WM8350_CODEC_JCK_DET_R_EINT             0x0400  /* CODEC_JCK_DET_R_EINT */
#define WM8350_CODEC_MICSCD_EINT                0x0200  /* CODEC_MICSCD_EINT */
#define WM8350_CODEC_MICD_EINT                  0x0100  /* CODEC_MICD_EINT */
#define WM8350_WKUP_OFF_STATE_EINT              0x0040  /* WKUP_OFF_STATE_EINT */
#define WM8350_WKUP_HIB_STATE_EINT              0x0020  /* WKUP_HIB_STATE_EINT */
#define WM8350_WKUP_CONV_FAULT_EINT             0x0010  /* WKUP_CONV_FAULT_EINT */
#define WM8350_WKUP_WDOG_RST_EINT               0x0008  /* WKUP_WDOG_RST_EINT */
#define WM8350_WKUP_GP_PWR_ON_EINT              0x0004  /* WKUP_GP_PWR_ON_EINT */
#define WM8350_WKUP_ONKEY_EINT                  0x0002  /* WKUP_ONKEY_EINT */
#define WM8350_WKUP_GP_WAKEUP_EINT              0x0001  /* WKUP_GP_WAKEUP_EINT */

/*
 * R32 (0x20) - System Interrupts Mask
 */
#define WM8350_IM_OC_INT                        0x2000  /* IM_OC_INT */
#define WM8350_IM_UV_INT                        0x1000  /* IM_UV_INT */
#define WM8350_IM_CS_INT                        0x0200  /* IM_CS_INT */
#define WM8350_IM_EXT_INT                       0x0100  /* IM_EXT_INT */
#define WM8350_IM_CODEC_INT                     0x0080  /* IM_CODEC_INT */
#define WM8350_IM_GP_INT                        0x0040  /* IM_GP_INT */
#define WM8350_IM_AUXADC_INT                    0x0020  /* IM_AUXADC_INT */
#define WM8350_IM_RTC_INT                       0x0010  /* IM_RTC_INT */
#define WM8350_IM_SYS_INT                       0x0008  /* IM_SYS_INT */
#define WM8350_IM_CHG_INT                       0x0004  /* IM_CHG_INT */
#define WM8350_IM_USB_INT                       0x0002  /* IM_USB_INT */
#define WM8350_IM_WKUP_INT                      0x0001  /* IM_WKUP_INT */

/*
 * R33 (0x21) - Interrupt Status 1 Mask
 */
#define WM8350_IM_CHG_BATT_HOT_EINT             0x8000  /* IM_CHG_BATT_HOT_EINT */
#define WM8350_IM_CHG_BATT_COLD_EINT            0x4000  /* IM_CHG_BATT_COLD_EINT */
#define WM8350_IM_CHG_BATT_FAIL_EINT            0x2000  /* IM_CHG_BATT_FAIL_EINT */
#define WM8350_IM_CHG_TO_EINT                   0x1000  /* IM_CHG_TO_EINT */
#define WM8350_IM_CHG_END_EINT                  0x0800  /* IM_CHG_END_EINT */
#define WM8350_IM_CHG_START_EINT                0x0400  /* IM_CHG_START_EINT */
#define WM8350_IM_CHG_FAST_RDY_EINT             0x0200  /* IM_CHG_FAST_RDY_EINT */
#define WM8350_IM_RTC_PER_EINT                  0x0080  /* IM_RTC_PER_EINT */
#define WM8350_IM_RTC_SEC_EINT                  0x0040  /* IM_RTC_SEC_EINT */
#define WM8350_IM_RTC_ALM_EINT                  0x0020  /* IM_RTC_ALM_EINT */
#define WM8350_IM_CHG_VBATT_LT_3P9_EINT         0x0004  /* IM_CHG_VBATT_LT_3P9_EINT */
#define WM8350_IM_CHG_VBATT_LT_3P1_EINT         0x0002  /* IM_CHG_VBATT_LT_3P1_EINT */
#define WM8350_IM_CHG_VBATT_LT_2P85_EINT        0x0001  /* IM_CHG_VBATT_LT_2P85_EINT */

/*
 * R34 (0x22) - Interrupt Status 2 Mask
 */
#define WM8350_IM_CS1_EINT                      0x2000  /* IM_CS1_EINT */
#define WM8350_IM_CS2_EINT                      0x1000  /* IM_CS2_EINT */
#define WM8350_IM_USB_LIMIT_EINT                0x0400  /* IM_USB_LIMIT_EINT */
#define WM8350_IM_AUXADC_DATARDY_EINT           0x0100  /* IM_AUXADC_DATARDY_EINT */
#define WM8350_IM_AUXADC_DCOMP4_EINT            0x0080  /* IM_AUXADC_DCOMP4_EINT */
#define WM8350_IM_AUXADC_DCOMP3_EINT            0x0040  /* IM_AUXADC_DCOMP3_EINT */
#define WM8350_IM_AUXADC_DCOMP2_EINT            0x0020  /* IM_AUXADC_DCOMP2_EINT */
#define WM8350_IM_AUXADC_DCOMP1_EINT            0x0010  /* IM_AUXADC_DCOMP1_EINT */
#define WM8350_IM_SYS_HYST_COMP_FAIL_EINT       0x0008  /* IM_SYS_HYST_COMP_FAIL_EINT */
#define WM8350_IM_SYS_CHIP_GT115_EINT           0x0004  /* IM_SYS_CHIP_GT115_EINT */
#define WM8350_IM_SYS_CHIP_GT140_EINT           0x0002  /* IM_SYS_CHIP_GT140_EINT */
#define WM8350_IM_SYS_WDOG_TO_EINT              0x0001  /* IM_SYS_WDOG_TO_EINT */

/*
 * R36 (0x24) - Under Voltage Interrupt status Mask
 */
#define WM8350_IM_UV_LDO4_EINT                  0x0800  /* IM_UV_LDO4_EINT */
#define WM8350_IM_UV_LDO3_EINT                  0x0400  /* IM_UV_LDO3_EINT */
#define WM8350_IM_UV_LDO2_EINT                  0x0200  /* IM_UV_LDO2_EINT */
#define WM8350_IM_UV_LDO1_EINT                  0x0100  /* IM_UV_LDO1_EINT */
#define WM8350_IM_UV_DC6_EINT                   0x0020  /* IM_UV_DC6_EINT */
#define WM8350_IM_UV_DC5_EINT                   0x0010  /* IM_UV_DC5_EINT */
#define WM8350_IM_UV_DC4_EINT                   0x0008  /* IM_UV_DC4_EINT */
#define WM8350_IM_UV_DC3_EINT                   0x0004  /* IM_UV_DC3_EINT */
#define WM8350_IM_UV_DC2_EINT                   0x0002  /* IM_UV_DC2_EINT */
#define WM8350_IM_UV_DC1_EINT                   0x0001  /* IM_UV_DC1_EINT */

/*
 * R37 (0x25) - Over Current Interrupt status Mask
 */
#define WM8350_IM_OC_LS_EINT                    0x8000  /* IM_OC_LS_EINT */

/*
 * R38 (0x26) - GPIO Interrupt Status Mask
 */
#define WM8350_IM_GP12_EINT                     0x1000  /* IM_GP12_EINT */
#define WM8350_IM_GP11_EINT                     0x0800  /* IM_GP11_EINT */
#define WM8350_IM_GP10_EINT                     0x0400  /* IM_GP10_EINT */
#define WM8350_IM_GP9_EINT                      0x0200  /* IM_GP9_EINT */
#define WM8350_IM_GP8_EINT                      0x0100  /* IM_GP8_EINT */
#define WM8350_IM_GP7_EINT                      0x0080  /* IM_GP7_EINT */
#define WM8350_IM_GP6_EINT                      0x0040  /* IM_GP6_EINT */
#define WM8350_IM_GP5_EINT                      0x0020  /* IM_GP5_EINT */
#define WM8350_IM_GP4_EINT                      0x0010  /* IM_GP4_EINT */
#define WM8350_IM_GP3_EINT                      0x0008  /* IM_GP3_EINT */
#define WM8350_IM_GP2_EINT                      0x0004  /* IM_GP2_EINT */
#define WM8350_IM_GP1_EINT                      0x0002  /* IM_GP1_EINT */
#define WM8350_IM_GP0_EINT                      0x0001  /* IM_GP0_EINT */

/*
 * R39 (0x27) - Comparator Interrupt Status Mask
 */
#define WM8350_IM_EXT_USB_FB_EINT               0x8000  /* IM_EXT_USB_FB_EINT */
#define WM8350_IM_EXT_WALL_FB_EINT              0x4000  /* IM_EXT_WALL_FB_EINT */
#define WM8350_IM_EXT_BATT_FB_EINT              0x2000  /* IM_EXT_BATT_FB_EINT */
#define WM8350_IM_CODEC_JCK_DET_L_EINT          0x0800  /* IM_CODEC_JCK_DET_L_EINT */
#define WM8350_IM_CODEC_JCK_DET_R_EINT          0x0400  /* IM_CODEC_JCK_DET_R_EINT */
#define WM8350_IM_CODEC_MICSCD_EINT             0x0200  /* IM_CODEC_MICSCD_EINT */
#define WM8350_IM_CODEC_MICD_EINT               0x0100  /* IM_CODEC_MICD_EINT */
#define WM8350_IM_WKUP_OFF_STATE_EINT           0x0040  /* IM_WKUP_OFF_STATE_EINT */
#define WM8350_IM_WKUP_HIB_STATE_EINT           0x0020  /* IM_WKUP_HIB_STATE_EINT */
#define WM8350_IM_WKUP_CONV_FAULT_EINT          0x0010  /* IM_WKUP_CONV_FAULT_EINT */
#define WM8350_IM_WKUP_WDOG_RST_EINT            0x0008  /* IM_WKUP_WDOG_RST_EINT */
#define WM8350_IM_WKUP_GP_PWR_ON_EINT           0x0004  /* IM_WKUP_GP_PWR_ON_EINT */
#define WM8350_IM_WKUP_ONKEY_EINT               0x0002  /* IM_WKUP_ONKEY_EINT */
#define WM8350_IM_WKUP_GP_WAKEUP_EINT           0x0001  /* IM_WKUP_GP_WAKEUP_EINT */

/*
 * R40 (0x28) - Clock Control 1
 */
#define WM8350_TOCLK_ENA_R40                    0x8000  /* TOCLK_ENA */
#define WM8350_TOCLK_ENA_R40_MASK               0x8000  /* TOCLK_ENA */
#define WM8350_TOCLK_ENA_R40_SHIFT                  15  /* TOCLK_ENA */
#define WM8350_TOCLK_RATE                       0x4000  /* TOCLK_RATE */
#define WM8350_TOCLK_RATE_MASK                  0x4000  /* TOCLK_RATE */
#define WM8350_TOCLK_RATE_SHIFT                     14  /* TOCLK_RATE */
#define WM8350_MCLK_SEL                         0x0800  /* MCLK_SEL */
#define WM8350_MCLK_SEL_MASK                    0x0800  /* MCLK_SEL */
#define WM8350_MCLK_SEL_SHIFT                       11  /* MCLK_SEL */
#define WM8350_MCLK_DIV                         0x0100  /* MCLK_DIV */
#define WM8350_MCLK_DIV_MASK                    0x0100  /* MCLK_DIV */
#define WM8350_MCLK_DIV_SHIFT                        8  /* MCLK_DIV */
#define WM8350_BCLK_DIV_MASK                    0x00F0  /* BCLK_DIV - [7:4] */
#define WM8350_BCLK_DIV_SHIFT                        4  /* BCLK_DIV - [7:4] */
#define WM8350_OPCLK_DIV_MASK                   0x0007  /* OPCLK_DIV - [2:0] */
#define WM8350_OPCLK_DIV_SHIFT                       0  /* OPCLK_DIV - [2:0] */

/* Bit definitions for R40 (0x28) */
#define WM8350_TOCLK_OFF                             0
#define WM8350_TOCLK_ENA                             1

#define WM8350_TOCLK_RATE_SLOW                       0  /* Slow TOCLK (SYSCLK/2^21) */
#define WM8350_TOCLK_RATE_FAST                       1  /* Fast TOCLK (SYSCLK/2^19) */

#define WM8350_MCLK_SEL_MCLK                         0  /* Take SYSCLK from MCLK input (bypass FLL) */
#define WM8350_MCLK_SEL_FLL                          1  /* Take SYSCLK from FLL */

#define WM8350_MCLK_DIV_1                            0  /* Use MCLK/1 in slave mode */
#define WM8350_MCLK_DIV_2                            1  /* Use MCLK/2 in slave mode */

#define WM8350_BCLK_DIV_1                            0  /* BITCLK = SYSCLK/1 */
#define WM8350_BCLK_DIV_1_5                          1  /* BITCLK = SYSCLK/1.5 */
#define WM8350_BCLK_DIV_2                            2  /* BITCLK = SYSCLK/2 */
#define WM8350_BCLK_DIV_3                            3  /* BITCLK = SYSCLK/3 */
#define WM8350_BCLK_DIV_4                            4  /* BITCLK = SYSCLK/4 */
#define WM8350_BCLK_DIV_5_5                          5  /* BITCLK = SYSCLK/5.5 */
#define WM8350_BCLK_DIV_6                            6  /* BITCLK = SYSCLK/6 */
#define WM8350_BCLK_DIV_8                            7  /* BITCLK = SYSCLK/8 */
#define WM8350_BCLK_DIV_11                           8  /* BITCLK = SYSCLK/11 */
#define WM8350_BCLK_DIV_12                           9  /* BITCLK = SYSCLK/12 */
#define WM8350_BCLK_DIV_16                          10  /* BITCLK = SYSCLK/16 */
#define WM8350_BCLK_DIV_22                          11  /* BITCLK = SYSCLK/22 */
#define WM8350_BCLK_DIV_24                          12  /* BITCLK = SYSCLK/24 */
#define WM8350_BCLK_DIV_32                          13  /* BITCLK = SYSCLK/32 */
#define WM8350_BCLK_DIV_44                          14  /* BITCLK = SYSCLK/44 */
#define WM8350_BCLK_DIV_48                          15  /* BITCLK = SYSCLK/48 */

#define WM8350_OPCLK_DIV_1                           0  /* OPCLK = SYSCLK/1 */
#define WM8350_OPCLK_DIV_1_5                         1  /* OPCLK = SYSCLK/1.5 */
#define WM8350_OPCLK_DIV_2                           2  /* OPCLK = SYSCLK/2 */
#define WM8350_OPCLK_DIV_3                           3  /* OPCLK = SYSCLK/3 */
#define WM8350_OPCLK_DIV_4                           4  /* OPCLK = SYSCLK/4 */
#define WM8350_OPCLK_DIV_5_5                         5  /* OPCLK = SYSCLK/5.5 */
#define WM8350_OPCLK_DIV_6                           6  /* OPCLK = SYSCLK/6 */

/*
 * R41 (0x29) - Clock Control 2
 */
#define WM8350_LRC_ADC_SEL                      0x8000  /* LRC_ADC_SEL */
#define WM8350_LRC_ADC_SEL_MASK                 0x8000  /* LRC_ADC_SEL */
#define WM8350_LRC_ADC_SEL_SHIFT                    15  /* LRC_ADC_SEL */
#define WM8350_MCLK_DIR                         0x0001  /* MCLK_DIR */
#define WM8350_MCLK_DIR_MASK                    0x0001  /* MCLK_DIR */
#define WM8350_MCLK_DIR_SHIFT                        0  /* MCLK_DIR */

/*
 * R42 (0x2A) - FLL Control 1
 */
#define WM8350_FLL_ENA_R42                      0x8000  /* FLL_ENA */
#define WM8350_FLL_ENA_R42_MASK                 0x8000  /* FLL_ENA */
#define WM8350_FLL_ENA_R42_SHIFT                    15  /* FLL_ENA */
#define WM8350_FLL_OSC_ENA_R42                  0x4000  /* FLL_OSC_ENA */
#define WM8350_FLL_OSC_ENA_R42_MASK             0x4000  /* FLL_OSC_ENA */
#define WM8350_FLL_OSC_ENA_R42_SHIFT                14  /* FLL_OSC_ENA */
#define WM8350_FLL_DITHER_WIDTH_MASK            0x3000  /* FLL_DITHER_WIDTH - [13:12] */
#define WM8350_FLL_DITHER_WIDTH_SHIFT               12  /* FLL_DITHER_WIDTH - [13:12] */
#define WM8350_FLL_DITHER_HP                    0x0800  /* FLL_DITHER_HP */
#define WM8350_FLL_DITHER_HP_MASK               0x0800  /* FLL_DITHER_HP */
#define WM8350_FLL_DITHER_HP_SHIFT                  11  /* FLL_DITHER_HP */
#define WM8350_FLL_OUTDIV_MASK                  0x0700  /* FLL_OUTDIV - [10:8] */
#define WM8350_FLL_OUTDIV_SHIFT                      8  /* FLL_OUTDIV - [10:8] */
#define WM8350_FLL_RSP_RATE_MASK                0x00F0  /* FLL_RSP_RATE - [7:4] */
#define WM8350_FLL_RSP_RATE_SHIFT                    4  /* FLL_RSP_RATE - [7:4] */
#define WM8350_FLL_RATE_MASK                    0x0007  /* FLL_RATE - [2:0] */
#define WM8350_FLL_RATE_SHIFT                        0  /* FLL_RATE - [2:0] */

/* Bit definitions for R42 (0x2A) */
#define WM8350_FLL_OFF                               0
#define WM8350_FLL_ENA                               1

#define WM8350_FLL_OSC_OFF                           0
#define WM8350_FLL_OSC_ENA                           1

#define WM8350_FLL_OUTDIV_4                          1
#define WM8350_FLL_OUTDIV_8                          2
#define WM8350_FLL_OUTDIV_16                         3
#define WM8350_FLL_OUTDIV_32                         4

/*
 * R43 (0x2B) - FLL Control 2
 */
#define WM8350_FLL_FRATIO_MASK                  0xF800  /* FLL_RATIO - [15:11] */
#define WM8350_FLL_FRATIO_SHIFT                     11  /* FLL_RATIO - [15:11] */
#define WM8350_FLL_N_MASK                       0x03FF  /* FLL_N - [9:0] */
#define WM8350_FLL_N_SHIFT                           0  /* FLL_N - [9:0] */

/* Bit definitions for R43 (0x2B) */
#define WM8350_FLL_FRATIO_1                          1
#define WM8350_FLL_FRATIO_8                          8

/*
 * R44 (0x2C) - FLL Control 3
 */
#define WM8350_FLL_K_MASK                       0xFFFF  /* FLL_K - [15:0] */
#define WM8350_FLL_K_SHIFT                           0  /* FLL_K - [15:0] */

/*
 * R45 (0x2D) - FLL Control 4
 */
#define WM8350_FLL_FRAC                         0x0020  /* FLL_FRAC */
#define WM8350_FLL_FRAC_MASK                    0x0020  /* FLL_FRAC */
#define WM8350_FLL_FRAC_SHIFT                        5  /* FLL_FRAC */
#define WM8350_FLL_SLOW_LOCK_REF                0x0010  /* FLL_SLOW_LOCK_REF */
#define WM8350_FLL_SLOW_LOCK_REF_MASK           0x0010  /* FLL_SLOW_LOCK_REF */
#define WM8350_FLL_SLOW_LOCK_REF_SHIFT               4  /* FLL_SLOW_LOCK_REF */
#define WM8350_FLL_CLK_SRC_MASK                 0x0003  /* FLL_CLK_SRC - [1:0] */
#define WM8350_FLL_CLK_SRC_SHIFT                     0  /* FLL_CLK_SRC - [1:0] */

/* Bit definitions for R45 (0x2D) */
#define WM8350_FLL_FRAC_OFF                          0
#define WM8350_FLL_FRAC_ENA                          1

#define WM8350_FLL_SLOW_LOCK_REF_OFF                 0
#define WM8350_FLL_SLOW_LOCK_REF_ENA                 1

#define WM8350_FLL_CLK_SRC_MCLK                      0
#define WM8350_FLL_CLK_SRC_DACLRCLK                  1
#define WM8350_FLL_CLK_SRC_ADCLRCLK                  2
#define WM8350_FLL_CLK_SRC_32K_REF                   3
#define WM8350_FLL_CLK_SRC_32KREF                    WM8350_FLL_CLK_SRC_32K_REF

/*
 * R48 (0x30) - DAC Control
 */
#define WM8350_DAC_MONO                         0x2000  /* DAC_MONO */
#define WM8350_AIF_LRCLKRATE                    0x1000  /* AIF_LRCLKRATE */
#define WM8350_DEEMP_MASK                       0x0030  /* DEEMP - [5:4] */
#define WM8350_DAC_SDMCLK_RATE                  0x0008  /* DAC_SDMCLK_RATE */
#define WM8350_DACL_DATINV                      0x0002  /* DACL_DATINV */
#define WM8350_DACR_DATINV                      0x0001  /* DACR_DATINV */

/*
 * R50 (0x32) - DAC Digital Volume L
 */
#define WM8350_DACL_ENA_R50                     0x8000  /* DACL_ENA */
#define WM8350_DACL_ENA_R50_MASK                0x8000  /* DACL_ENA */
#define WM8350_DACL_ENA_R50_SHIFT                   15  /* DACL_ENA */
#define WM8350_DAC_VU                           0x0100  /* DAC_VU */
#define WM8350_DAC_VU_MASK                      0x0100  /* DAC_VU */
#define WM8350_DAC_VU_SHIFT                          8  /* DAC_VU */
#define WM8350_DACL_VOL_MASK                    0x00FF  /* DACL_VOL - [7:0] */
#define WM8350_DACL_VOL_SHIFT                        0  /* DACL_VOL - [7:0] */

/* Bit definitions for R50 (0x32) */
#define WM8350_DACL_VOL_MIN                       0x01  /* Minimum volume value */
#define WM8350_DACL_VOL_0DB                       0xC0  /* 0dB volume value */
#define WM8350_DACL_VOL_MAX                       0xFF  /* Maximum volume value */

/*
 * R51 (0x33) - DAC Digital Volume R
 */
#define WM8350_DACR_ENA_R51                     0x8000  /* DACR_ENA */
#define WM8350_DACR_ENA_R51_MASK                0x8000  /* DACR_ENA */
#define WM8350_DACR_ENA_R51_SHIFT                   15  /* DACR_ENA */
#define WM8350_DAC_VU                           0x0100  /* DAC_VU */
#define WM8350_DAC_VU_MASK                      0x0100  /* DAC_VU */
#define WM8350_DAC_VU_SHIFT                          8  /* DAC_VU */
#define WM8350_DACR_VOL_MASK                    0x00FF  /* DACR_VOL - [7:0] */
#define WM8350_DACR_VOL_SHIFT                        0  /* DACR_VOL - [7:0] */

/* Bit definitions for R51 (0x33) */
#define WM8350_DACR_VOL_MIN                       0x01  /* Minimum volume value */
#define WM8350_DACR_VOL_0DB                       0xC0  /* 0dB volume value */
#define WM8350_DACR_VOL_MAX                       0xFF  /* Maximum volume value */

/*
 * R53 (0x35) - DAC LR Rate
 */
#define WM8350_DACLRC_ENA_R53                   0x0800  /* DACLRC_ENA */
#define WM8350_DACLRC_RATE_MASK                 0x07FF  /* DACLRC_RATE - [10:0] */
#define WM8350_DACLRC_RATE_SHIFT                     0  /* DACLRC_RATE - [10:0] */

/*
 * R54 (0x36) - DAC Clock Control
 */
#define WM8350_DACCLK_POL                       0x0010  /* DACCLK_POL */
#define WM8350_DACCLK_POL_MASK                  0x0010  /* DACCLK_POL */
#define WM8350_DACCLK_POL_SHIFT                      4  /* DACCLK_POL */
#define WM8350_DAC_CLKDIV_MASK                  0x0007  /* DAC_CLKDIV - [2:0] */
#define WM8350_DAC_CLKDIV_SHIFT                      0  /* DAC_CLKDIV - [2:0] */

/* Bit definitions for R54 (0x36) */

#define WM8350_DAC_CLKDIV_1                          0  /* SYSCLK/1 */
#define WM8350_DAC_CLKDIV_1_5                        1  /* SYSCLK/1.5 */
#define WM8350_DAC_CLKDIV_2                          2  /* SYSCLK/2 */
#define WM8350_DAC_CLKDIV_3                          3  /* SYSCLK/3 */
#define WM8350_DAC_CLKDIV_4                          4  /* SYSCLK/4 */
#define WM8350_DAC_CLKDIV_5_5                        5  /* SYSCLK/5.5 */
#define WM8350_DAC_CLKDIV_6                          6  /* SYSCLK/6 */

/*
 * R58 (0x3A) - DAC Mute
 */
#define WM8350_DAC_MUTE_R58                     0x4000  /* DAC_MUTE */

/*
 * R59 (0x3B) - DAC Mute Volume
 */
#define WM8350_DAC_MUTEMODE                     0x4000  /* DAC_MUTEMODE */
#define WM8350_DAC_MUTEMODE_MASK                0x4000  /* DAC_MUTEMODE */
#define WM8350_DAC_MUTEMODE_SHIFT                   14  /* DAC_MUTEMODE */
#define WM8350_DAC_MUTERATE                     0x2000  /* DAC_MUTERATE */
#define WM8350_DAC_MUTERATE_MASK                0x2000  /* DAC_MUTERATE */
#define WM8350_DAC_MUTERATE_SHIFT                   13  /* DAC_MUTERATE */
#define WM8350_DAC_SB_FILT                      0x1000  /* DAC_SB_FILT */
#define WM8350_DAC_SB_FILT_MASK                 0x1000  /* DAC_SB_FILT */
#define WM8350_DAC_SB_FILT_SHIFT                    12  /* DAC_SB_FILT */

/* Bit definitions for R59 (0x3B) */
#define WM8350_DAC_MUTEMODE_IMMEDIATE                0
#define WM8350_DAC_MUTEMODE_RAMP                     1

#define WM8350_DAC_MUTERATE_FAST                     0
#define WM8350_DAC_MUTERATE_SLOW                     1

#define WM8350_DAC_SB_FILT_NORMAL                    0  /* Normal stopband filter */
#define WM8350_DAC_SB_FILT_SLOPING                   1  /* Sloping stopband filter */

/*
 * R60 (0x3C) - DAC Side
 */
#define WM8350_ADC_TO_DACL_MASK                 0x3000  /* ADC_TO_DACL - [13:12] */
#define WM8350_ADC_TO_DACR_MASK                 0x0C00  /* ADC_TO_DACR - [11:10] */

/*
 * R64 (0x40) - ADC Control
 */
#define WM8350_ADC_HPF_ENA_R64                  0x8000  /* ADC_HPF_ENA */
#define WM8350_ADC_HPF_CUT_MASK                 0x0300  /* ADC_HPF_CUT - [9:8] */
#define WM8350_ADCL_DATINV                      0x0002  /* ADCL_DATINV */
#define WM8350_ADCR_DATINV                      0x0001  /* ADCR_DATINV */

/*
 * R66 (0x42) - ADC Digital Volume L
 */
#define WM8350_ADCL_ENA_R66                     0x8000  /* ADCL_ENA */
#define WM8350_ADCL_ENA_R66_MASK                0x8000  /* ADCL_ENA */
#define WM8350_ADCL_ENA_R66_SHIFT                   15  /* ADCL_ENA */
#define WM8350_ADC_VU                           0x0100  /* ADC_VU */
#define WM8350_ADC_VU_MASK                      0x0100  /* ADC_VU */
#define WM8350_ADC_VU_SHIFT                          8  /* ADC_VU */
#define WM8350_ADCL_VOL_MASK                    0x00FF  /* ADCL_VOL - [7:0] */
#define WM8350_ADCL_VOL_SHIFT                        0  /* ADCL_VOL - [7:0] */

/* Bit definitions for R66 (0x42) */
#define WM8350_ADCL_VOL_MIN                       0x01  /* Minimum volume value */
#define WM8350_ADCL_VOL_0DB                       0xC0  /* 0dB volume value */
#define WM8350_ADCL_VOL_MAX                       0xEF  /* Maximum volume value */

/*
 * R67 (0x43) - ADC Digital Volume R
 */
#define WM8350_ADCR_ENA_R66                     0x8000  /* ADCR_ENA */
#define WM8350_ADCR_ENA_R66_MASK                0x8000  /* ADCR_ENA */
#define WM8350_ADCR_ENA_R66_SHIFT                   15  /* ADCR_ENA */
#define WM8350_ADC_VU                           0x0100  /* ADC_VU */
#define WM8350_ADC_VU_MASK                      0x0100  /* ADC_VU */
#define WM8350_ADC_VU_SHIFT                          8  /* ADC_VU */
#define WM8350_ADCR_VOL_MASK                    0x00FF  /* ADCR_VOL - [7:0] */
#define WM8350_ADCR_VOL_SHIFT                        0  /* ADCR_VOL - [7:0] */

/* Bit definitions for R67 (0x43) */
#define WM8350_ADCR_VOL_MIN                       0x01  /* Minimum volume value */
#define WM8350_ADCR_VOL_0DB                       0xC0  /* 0dB volume value */
#define WM8350_ADCR_VOL_MAX                       0xEF  /* Maximum volume value */

/*
 * R68 (0x44) - ADC Divider
 */
#define WM8350_ADCL_DAC_SVOL_MASK               0x0F00  /* ADCL_DAC_SVOL - [11:8] */
#define WM8350_ADCL_DAC_SVOL_SHIFT                   8  /* ADCL_DAC_SVOL - [11:8] */
#define WM8350_ADCR_DAC_SVOL_MASK               0x00F0  /* ADCR_DAC_SVOL - [7:4] */
#define WM8350_ADCR_DAC_SVOL_SHIFT                   4  /* ADCR_DAC_SVOL - [7:4] */
#define WM8350_ADCCLK_POL                       0x0008  /* ADCCLK_POL */
#define WM8350_ADCCLK_POL_MASK                  0x0008  /* ADCCLK_POL */
#define WM8350_ADCCLK_POL_SHIFT                      3  /* ADCCLK_POL */
#define WM8350_ADC_CLKDIV_MASK                  0x0007  /* ADC_CLKDIV - [2:0] */
#define WM8350_ADC_CLKDIV_SHIFT                      0  /* ADC_CLKDIV - [2:0] */

/* Bit definitions for R68 (0x44) */

#define WM8350_ADC_CLKDIV_1                          0  /* SYSCLK/1 */
#define WM8350_ADC_CLKDIV_1_5                        1  /* SYSCLK/1.5 */
#define WM8350_ADC_CLKDIV_2                          2  /* SYSCLK/2 */
#define WM8350_ADC_CLKDIV_3                          3  /* SYSCLK/3 */
#define WM8350_ADC_CLKDIV_4                          4  /* SYSCLK/4 */
#define WM8350_ADC_CLKDIV_5_5                        5  /* SYSCLK/5.5 */
#define WM8350_ADC_CLKDIV_6                          6  /* SYSCLK/6 */

/*
 * R70 (0x46) - ADC LR Rate
 */
#define WM8350_ADCLRC_ENA_R70                   0x0800  /* ADCLRC_ENA */
#define WM8350_ADCLRC_RATE_MASK                 0x07FF  /* ADCLRC_RATE - [10:0] */

/*
 * R72 (0x48) - Input Control
 */
#define WM8350_IN2R_ENA_R72                     0x0400  /* IN2R_ENA */
#define WM8350_IN1RN_ENA_R72                    0x0200  /* IN1RN_ENA */
#define WM8350_IN1RP_ENA_R72                    0x0100  /* IN1RP_ENA */
#define WM8350_IN2L_ENA_R72                     0x0004  /* IN2L_ENA */
#define WM8350_IN1LN_ENA_R72                    0x0002  /* IN1LN_ENA */
#define WM8350_IN1LP_ENA_R72                    0x0001  /* IN1LP_ENA */

/*
 * R73 (0x49) - IN3 Input Control
 */
#define WM8350_IN3R_ENA_R73                     0x8000  /* IN3R_ENA */
#define WM8350_IN3R_SHORT                       0x4000  /* IN3R_SHORT */
#define WM8350_IN3L_ENA_R73                     0x0080  /* IN3L_ENA */
#define WM8350_IN3L_SHORT                       0x0040  /* IN3L_SHORT */

/*
 * R74 (0x4A) - Mic Bias Control
 */
#define WM8350_MICB_ENA_R74                     0x8000  /* MICB_ENA */
#define WM8350_MICB_SEL                         0x4000  /* MICB_SEL */
#define WM8350_MIC_DET_ENA_R74                  0x0080  /* MIC_DET_ENA */
#define WM8350_MCDTHR_MASK                      0x001C  /* MCDTHR - [4:2] */
#define WM8350_MCDSCTHR_MASK                    0x0003  /* MCDSCTHR - [1:0] */

/*
 * R76 (0x4C) - Output Control
 */
#define WM8350_OUT4_VROI                        0x0800  /* OUT4_VROI */
#define WM8350_OUT4_VROI_MASK                   0x0800  /* OUT4_VROI */
#define WM8350_OUT4_VROI_SHIFT                      11  /* OUT4_VROI */
#define WM8350_OUT3_VROI                        0x0400  /* OUT3_VROI */
#define WM8350_OUT3_VROI_MASK                   0x0400  /* OUT3_VROI */
#define WM8350_OUT3_VROI_SHIFT                      10  /* OUT3_VROI */
#define WM8350_OUT2_VROI                        0x0200  /* OUT2_VROI */
#define WM8350_OUT2_VROI_MASK                   0x0200  /* OUT2_VROI */
#define WM8350_OUT2_VROI_SHIFT                       9  /* OUT2_VROI */
#define WM8350_OUT1_VROI                        0x0100  /* OUT1_VROI */
#define WM8350_OUT1_VROI_MASK                   0x0100  /* OUT1_VROI */
#define WM8350_OUT1_VROI_SHIFT                       8  /* OUT1_VROI */
#define WM8350_OUTPUT_DRAIN_ENA                 0x0010  /* OUTPUT_DRAIN_ENA */
#define WM8350_OUT2_FB                          0x0004  /* OUT2_FB */
#define WM8350_OUT1_FB                          0x0001  /* OUT1_FB */

/* R76 (0x4C) regsiter bits */
#define WM8350_OUT4_VROI_500                         0  /* 500 ohm resistor string */
#define WM8350_OUT4_VROI_30K                         1  /* 30 Kohm resistor string */

#define WM8350_OUT3_VROI_500                         0  /* 500 ohm resistor string */
#define WM8350_OUT3_VROI_30K                         1  /* 30 Kohm resistor string */

#define WM8350_OUT2_VROI_500                         0  /* 500 ohm resistor string */
#define WM8350_OUT2_VROI_30K                         1  /* 30 Kohm resistor string */

#define WM8350_OUT1_VROI_500                         0  /* 500 ohm resistor string */
#define WM8350_OUT1_VROI_30K                         1  /* 30 Kohm resistor string */

/*
 * R77 (0x4D) - Jack Detect
 */
#define WM8350_JDL_ENA_R77                      0x8000  /* JDL_ENA */
#define WM8350_JDR_ENA_R77                      0x4000  /* JDR_ENA */

/*
 * R78 (0x4E) - Anti Pop Control
 */
#define WM8350_ANTI_POP_MASK                    0x0300  /* ANTI_POP - [9:8] */
#define WM8350_ANTI_POP_SHIFT                        8  /* ANTI_POP - [9:8] */
#define WM8350_DIS_OP_OUT4_MASK                 0x00C0  /* DIS_OP_OUT4 - [7:6] */
#define WM8350_DIS_OP_OUT4_SHIFT                     6  /* DIS_OP_OUT4 - [7:6] */
#define WM8350_DIS_OP_OUT3_MASK                 0x0030  /* DIS_OP_OUT3 - [5:4] */
#define WM8350_DIS_OP_OUT3_SHIFT                     4  /* DIS_OP_OUT3 - [5:4] */
#define WM8350_DIS_OP_OUT2_MASK                 0x000C  /* DIS_OP_OUT2 - [3:2] */
#define WM8350_DIS_OP_OUT2_SHIFT                     2  /* DIS_OP_OUT2 - [3:2] */
#define WM8350_DIS_OP_OUT1_MASK                 0x0003  /* DIS_OP_OUT1 - [1:0] */
#define WM8350_DIS_OP_OUT1_SHIFT                     0  /* DIS_OP_OUT1 - [1:0] */

/* Bit definitions for R78 (0x4E) */
#define WM8350_ANTI_POP_OFF                          0  /* 0 = anti-pop off */
#define WM8350_ANTI_POP_FAST_S                       1  /* 1 = fastest S-curve */
#define WM8350_ANTI_POP_MEDIUM_S                     2  /* 2 = medium S-curve */
#define WM8350_ANTI_POP_SLOW_S                       3  /* 3 = slowest S-curve */

#define WM8350_DIS_OP_OUT4_OFF                       0  /* 0 = OUT4 discharge path OFF */
#define WM8350_DIS_OP_OUT4_FAST                      1  /* 1 = OUT4 fastest discharge */
#define WM8350_DIS_OP_OUT4_MEDIUM                    2  /* 2 = OUT4 medium discharge */
#define WM8350_DIS_OP_OUT4_SLOW                      3  /* 3 = OUT4 slowest discharge */

#define WM8350_DIS_OP_OUT3_OFF                       0  /* 0 = OUT3 discharge path OFF */
#define WM8350_DIS_OP_OUT3_FAST                      1  /* 1 = OUT3 fastest discharge */
#define WM8350_DIS_OP_OUT3_MEDIUM                    2  /* 2 = OUT3 medium discharge */
#define WM8350_DIS_OP_OUT3_SLOW                      3  /* 3 = OUT3 slowest discharge */

#define WM8350_DIS_OP_OUT2_OFF                       0  /* 0 = OUT2 discharge path OFF */
#define WM8350_DIS_OP_OUT2_FAST                      1  /* 1 = OUT2 fastest discharge */
#define WM8350_DIS_OP_OUT2_MEDIUM                    2  /* 2 = OUT2 medium discharge */
#define WM8350_DIS_OP_OUT2_SLOW                      3  /* 3 = OUT2 slowest discharge */

#define WM8350_DIS_OP_OUT1_OFF                       0  /* 0 = OUT1 discharge path OFF */
#define WM8350_DIS_OP_OUT1_FAST                      1  /* 1 = OUT1 fastest discharge */
#define WM8350_DIS_OP_OUT1_MEDIUM                    2  /* 2 = OUT1 medium discharge */
#define WM8350_DIS_OP_OUT1_SLOW                      3  /* 3 = OUT1 slowest discharge */

/*
 * R80 (0x50) - Left Input Volume
 */
#define WM8350_INL_ENA_R80                      0x8000  /* INL_ENA */
#define WM8350_INL_ENA_R80_MASK                 0x8000  /* INL_ENA */
#define WM8350_INL_ENA_R80_SHIFT                    15  /* INL_ENA */
#define WM8350_INL_MUTE                         0x4000  /* INL_MUTE */
#define WM8350_INL_MUTE_MASK                    0x4000  /* INL_MUTE */
#define WM8350_INL_MUTE_SHIFT                       14  /* INL_MUTE */
#define WM8350_INL_ZC                           0x2000  /* INL_ZC */
#define WM8350_INL_ZC_MASK                      0x2000  /* INL_ZC */
#define WM8350_INL_ZC_SHIFT                         13  /* INL_ZC */
#define WM8350_IN_VU                            0x0100  /* IN_VU */
#define WM8350_IN_VU_MASK                       0x0100  /* IN_VU */
#define WM8350_IN_VU_SHIFT                           8  /* IN_VU */
#define WM8350_INL_VOL_MASK                     0x00FC  /* INL_VOL - [7:2] */
#define WM8350_INL_VOL_SHIFT                         2  /* INL_VOL - [7:2] */

/* Bit definitions for R80 (0x50) */
#define WM8350_INL_VOL_MIN                        0x00  /* minimum volume value */
#define WM8350_INL_VOL_0DB                        0x10  /* 0dB volume value */
#define WM8350_INL_VOL_MAX                        0x3F  /* maximum volume value */
#define WM8350_INL_VOL( _dB )                     ((_dB) + WM8350_INL_VOL_0DB)    /* 0 = -57dB, 1dB steps */
#define WM8350_INL_DB( _vol )                     ((_vol) - WM8350_INL_VOL_0DB)   /* 0 = -57dB, 1dB steps */

/*
 * R81 (0x51) - Right Input Volume
 */
#define WM8350_INR_ENA_R81                      0x8000  /* INR_ENA */
#define WM8350_INR_ENA_R81_MASK                 0x8000  /* INR_ENA */
#define WM8350_INR_ENA_R81_SHIFT                    15  /* INR_ENA */
#define WM8350_INR_MUTE                         0x4000  /* INR_MUTE */
#define WM8350_INR_MUTE_MASK                    0x4000  /* INR_MUTE */
#define WM8350_INR_MUTE_SHIFT                       14  /* INR_MUTE */
#define WM8350_INR_ZC                           0x2000  /* INR_ZC */
#define WM8350_INR_ZC_MASK                      0x2000  /* INR_ZC */
#define WM8350_INR_ZC_SHIFT                         13  /* INR_ZC */
#define WM8350_IN_VU                            0x0100  /* IN_VU */
#define WM8350_IN_VU_MASK                       0x0100  /* IN_VU */
#define WM8350_IN_VU_SHIFT                           8  /* IN_VU */
#define WM8350_INR_VOL_MASK                     0x00FC  /* INR_VOL - [7:2] */
#define WM8350_INR_VOL_SHIFT                         2  /* INR_VOL - [7:2] */

/* Bit definitions for R81 (0x51) */
#define WM8350_INR_VOL_MIN                        0x00  /* minimum volume value */
#define WM8350_INR_VOL_0DB                        0x10  /* 0dB volume value */
#define WM8350_INR_VOL_MAX                        0x3F  /* maximum volume value */

/*
 * R88 (0x58) - Left Mixer Control
 */
#define WM8350_MIXOUTL_ENA_R88                  0x8000  /* MIXOUTL_ENA */
#define WM8350_DACR_TO_MIXOUTL                  0x1000  /* DACR_TO_MIXOUTL */
#define WM8350_DACL_TO_MIXOUTL                  0x0800  /* DACL_TO_MIXOUTL */
#define WM8350_IN3L_TO_MIXOUTL                  0x0004  /* IN3L_TO_MIXOUTL */
#define WM8350_INR_TO_MIXOUTL                   0x0002  /* INR_TO_MIXOUTL */
#define WM8350_INL_TO_MIXOUTL                   0x0001  /* INL_TO_MIXOUTL */

/*
 * R89 (0x59) - Right Mixer Control
 */
#define WM8350_MIXOUTR_ENA_R89                  0x8000  /* MIXOUTR_ENA */
#define WM8350_DACR_TO_MIXOUTR                  0x1000  /* DACR_TO_MIXOUTR */
#define WM8350_DACL_TO_MIXOUTR                  0x0800  /* DACL_TO_MIXOUTR */
#define WM8350_IN3R_TO_MIXOUTR                  0x0008  /* IN3R_TO_MIXOUTR */
#define WM8350_INR_TO_MIXOUTR                   0x0002  /* INR_TO_MIXOUTR */
#define WM8350_INL_TO_MIXOUTR                   0x0001  /* INL_TO_MIXOUTR */

/*
 * R92 (0x5C) - OUT3 Mixer Control
 */
#define WM8350_OUT3_ENA_R92                     0x8000  /* OUT3_ENA */
#define WM8350_DACL_TO_OUT3                     0x0800  /* DACL_TO_OUT3 */
#define WM8350_MIXINL_TO_OUT3                   0x0100  /* MIXINL_TO_OUT3 */
#define WM8350_OUT4_TO_OUT3                     0x0008  /* OUT4_TO_OUT3 */
#define WM8350_MIXOUTL_TO_OUT3                  0x0001  /* MIXOUTL_TO_OUT3 */

/*
 * R93 (0x5D) - OUT4 Mixer Control
 */
#define WM8350_OUT4_ENA_R93                     0x8000  /* OUT4_ENA */
#define WM8350_DACR_TO_OUT4                     0x1000  /* DACR_TO_OUT4 */
#define WM8350_DACL_TO_OUT4                     0x0800  /* DACL_TO_OUT4 */
#define WM8350_OUT4_ATTN                        0x0400  /* OUT4_ATTN */
#define WM8350_MIXINR_TO_OUT4                   0x0200  /* MIXINR_TO_OUT4 */
#define WM8350_OUT3_TO_OUT4                     0x0004  /* OUT3_TO_OUT4 */
#define WM8350_MIXOUTR_TO_OUT4                  0x0002  /* MIXOUTR_TO_OUT4 */
#define WM8350_MIXOUTL_TO_OUT4                  0x0001  /* MIXOUTL_TO_OUT4 */

/*
 * R96 (0x60) - Output Left Mixer Volume
 */
#define WM8350_IN3L_MIXOUTL_VOL_MASK            0x0E00  /* IN3L_MIXOUTL_VOL - [11:9] */
#define WM8350_IN3L_MIXOUTL_VOL_SHIFT                9  /* IN3L_MIXOUTL_VOL - [11:9] */
#define WM8350_INR_MIXOUTL_VOL_MASK             0x00E0  /* INR_MIXOUTL_VOL - [7:5] */
#define WM8350_INR_MIXOUTL_VOL_SHIFT                 5  /* INR_MIXOUTL_VOL - [7:5] */
#define WM8350_INL_MIXOUTL_VOL_MASK             0x000E  /* INL_MIXOUTL_VOL - [3:1] */
#define WM8350_INL_MIXOUTL_VOL_SHIFT                 1  /* INL_MIXOUTL_VOL - [3:1] */

/* Bit values for R96 (0x60) */
#define WM8350_IN3L_MIXOUTL_VOL_OFF                  0  /* 0 = disabled */
#define WM8350_IN3L_MIXOUTL_VOL_M12DB                1  /* 1 = -12dBFS */
#define WM8350_IN3L_MIXOUTL_VOL_M9DB                 2  /* 2 = -9dBFS */
#define WM8350_IN3L_MIXOUTL_VOL_M6DB                 3  /* 3 = -6dBFS */
#define WM8350_IN3L_MIXOUTL_VOL_M3DB                 4  /* 4 = -3dBFS */
#define WM8350_IN3L_MIXOUTL_VOL_0DB                  5  /* 5 = 0dBFS */
#define WM8350_IN3L_MIXOUTL_VOL_3DB                  6  /* 6 = 3dBFS */
#define WM8350_IN3L_MIXOUTL_VOL_6DB                  7  /* 7 = 6dBFS */

#define WM8350_INR_MIXOUTL_VOL_OFF                   0  /* 0 = disabled */
#define WM8350_INR_MIXOUTL_VOL_M12DB                  1  /* 1 = -12dBFS */
#define WM8350_INR_MIXOUTL_VOL_M9DB                  2  /* 2 = -9dBFS */
#define WM8350_INR_MIXOUTL_VOL_M6DB                  3  /* 3 = -6dBFS */
#define WM8350_INR_MIXOUTL_VOL_M3DB                  4  /* 4 = -3dBFS */
#define WM8350_INR_MIXOUTL_VOL_0DB                   5  /* 5 = 0dBFS */
#define WM8350_INR_MIXOUTL_VOL_3DB                   6  /* 6 = 3dBFS */
#define WM8350_INR_MIXOUTL_VOL_6DB                   7  /* 7 = 6dBFS */

#define WM8350_INL_MIXOUTL_VOL_OFF                   0  /* 0 = disabled */
#define WM8350_INL_MIXOUTL_VOL_M12DB                 1  /* 1 = -12dBFS */
#define WM8350_INL_MIXOUTL_VOL_M9DB                  2  /* 2 = -9dBFS */
#define WM8350_INL_MIXOUTL_VOL_M6DB                  3  /* 3 = -6dBFS */
#define WM8350_INL_MIXOUTL_VOL_M3DB                  4  /* 4 = -3dBFS */
#define WM8350_INL_MIXOUTL_VOL_0DB                   5  /* 5 = 0dBFS */
#define WM8350_INL_MIXOUTL_VOL_3DB                   6  /* 6 = 3dBFS */
#define WM8350_INL_MIXOUTL_VOL_6DB                   7  /* 7 = 6dBFS */

/*
 * R97 (0x61) - Output Right Mixer Volume
 */
#define WM8350_IN3R_MIXOUTR_VOL_MASK            0xE000  /* IN3R_MIXOUTR_VOL - [15:13] */
#define WM8350_IN3R_MIXOUTR_VOL_SHIFT               13  /* IN3R_MIXOUTR_VOL - [11:9] */
#define WM8350_INR_MIXOUTR_VOL_MASK             0x00E0  /* INR_MIXOUTR_VOL - [7:5] */
#define WM8350_INR_MIXOUTR_VOL_SHIFT                 5  /* INR_MIXOUTR_VOL - [7:5] */
#define WM8350_INL_MIXOUTR_VOL_MASK             0x000E  /* INL_MIXOUTR_VOL - [3:1] */
#define WM8350_INL_MIXOUTR_VOL_SHIFT                 1  /* INL_MIXOUTR_VOL - [3:1] */

/* Bit values for R96 (0x60) */
#define WM8350_IN3R_MIXOUTR_VOL_OFF                  0  /* 0 = disabled */
#define WM8350_IN3R_MIXOUTR_VOL_M12DB                1  /* 1 = -12dBFS */
#define WM8350_IN3R_MIXOUTR_VOL_M9DB                 2  /* 2 = -9dBFS */
#define WM8350_IN3R_MIXOUTR_VOL_M6DB                 3  /* 3 = -6dBFS */
#define WM8350_IN3R_MIXOUTR_VOL_M3DB                 4  /* 4 = -3dBFS */
#define WM8350_IN3R_MIXOUTR_VOL_0DB                  5  /* 5 = 0dBFS */
#define WM8350_IN3R_MIXOUTR_VOL_3DB                  6  /* 6 = 3dBFS */
#define WM8350_IN3R_MIXOUTR_VOL_6DB                  7  /* 7 = 6dBFS */

#define WM8350_INR_MIXOUTR_VOL_OFF                   0  /* 0 = disabled */
#define WM8350_INR_MIXOUTR_VOL_M12DB                  1  /* 1 = -12dBFS */
#define WM8350_INR_MIXOUTR_VOL_M9DB                  2  /* 2 = -9dBFS */
#define WM8350_INR_MIXOUTR_VOL_M6DB                  3  /* 3 = -6dBFS */
#define WM8350_INR_MIXOUTR_VOL_M3DB                  4  /* 4 = -3dBFS */
#define WM8350_INR_MIXOUTR_VOL_0DB                   5  /* 5 = 0dBFS */
#define WM8350_INR_MIXOUTR_VOL_3DB                   6  /* 6 = 3dBFS */
#define WM8350_INR_MIXOUTR_VOL_6DB                   7  /* 7 = 6dBFS */

#define WM8350_INL_MIXOUTR_VOL_OFF                   0  /* 0 = disabled */
#define WM8350_INL_MIXOUTR_VOL_M12DB                 1  /* 1 = -12dBFS */
#define WM8350_INL_MIXOUTR_VOL_M9DB                  2  /* 2 = -9dBFS */
#define WM8350_INL_MIXOUTR_VOL_M6DB                  3  /* 3 = -6dBFS */
#define WM8350_INL_MIXOUTR_VOL_M3DB                  4  /* 4 = -3dBFS */
#define WM8350_INL_MIXOUTR_VOL_0DB                   5  /* 5 = 0dBFS */
#define WM8350_INL_MIXOUTR_VOL_3DB                   6  /* 6 = 3dBFS */
#define WM8350_INL_MIXOUTR_VOL_6DB                   7  /* 7 = 6dBFS */

/*
 * R98 (0x62) - Input Mixer Volume L
 */
#define WM8350_IN3L_MIXINL_VOL_MASK             0x0E00  /* IN3L_MIXINL_VOL - [11:9] */
#define WM8350_IN2L_MIXINL_VOL_MASK             0x000E  /* IN2L_MIXINL_VOL - [3:1] */
#define WM8350_INL_MIXINL_VOL                   0x0001  /* INL_MIXINL_VOL */

/*
 * R99 (0x63) - Input Mixer Volume R
 */
#define WM8350_IN3R_MIXINR_VOL_MASK             0xE000  /* IN3R_MIXINR_VOL - [15:13] */
#define WM8350_IN2R_MIXINR_VOL_MASK             0x00E0  /* IN2R_MIXINR_VOL - [7:5] */
#define WM8350_INR_MIXINR_VOL                   0x0001  /* INR_MIXINR_VOL */

/*
 * R100 (0x64) - Input Mixer Volume
 */
#define WM8350_OUT4_MIXIN_DST                   0x8000  /* OUT4_MIXIN_DST */
#define WM8350_OUT4_MIXIN_VOL_MASK              0x000E  /* OUT4_MIXIN_VOL - [3:1] */

/*
 * R104 (0x68) - OUT1L Volume
 */
#define WM8350_OUT1L_ENA_R104                   0x8000  /* OUT1L_ENA */
#define WM8350_OUT1L_ENA_R104_MASK              0x8000  /* OUT1L_ENA */
#define WM8350_OUT1L_ENA_R104_SHIFT                 15  /* OUT1L_ENA */
#define WM8350_OUT1L_MUTE                       0x4000  /* OUT1L_MUTE */
#define WM8350_OUT1L_MUTE_MASK                  0x4000  /* OUT1L_MUTE */
#define WM8350_OUT1L_MUTE_SHIFT                     14  /* OUT1L_MUTE */
#define WM8350_OUT1L_ZC                         0x2000  /* OUT1L_ZC */
#define WM8350_OUT1L_ZC_MASK                    0x2000  /* OUT1L_ZC */
#define WM8350_OUT1L_ZC_SHIFT                       13  /* OUT1L_ZC */
#define WM8350_OUT1_VU                          0x0100  /* OUT1_VU */
#define WM8350_OUT1_VU_MASK                     0x0100  /* OUT1_VU */
#define WM8350_OUT1_VU_SHIFT                         8  /* OUT1_VU */
#define WM8350_OUT1L_VOL_MASK                   0x00FC  /* OUT1L_VOL - [7:2] */
#define WM8350_OUT1L_VOL_SHIFT                       2  /* OUT1L_VOL - [7:2] */

/* Bit values for R104 (0x68) */
#define WM8350_OUT1L_VOL_MIN                      0x00  /* minimum volume value */
#define WM8350_OUT1L_VOL_0DB                      0x39  /* 0dB volume value */
#define WM8350_OUT1L_VOL_MAX                      0x3F  /* maximum volume value */
#define WM8350_OUT1L_VOL( _dB )                   ((_dB) + WM8350_OUT1L_VOL_0DB)    /* 0 = -57dB, 1dB steps */
#define WM8350_OUT1L_DB( _vol )                   ((_vol) - WM8350_OUT1L_VOL_0DB)   /* 0 = -57dB, 1dB steps */

/*
 * R105 (0x69) - OUT1R Volume
 */
#define WM8350_OUT1R_ENA_R105                   0x8000  /* OUT1R_ENA */
#define WM8350_OUT1R_ENA_R105_MASK              0x8000  /* OUT1R_ENA */
#define WM8350_OUT1R_ENA_R105_SHIFT                 15  /* OUT1R_ENA */
#define WM8350_OUT1R_MUTE                       0x4000  /* OUT1R_MUTE */
#define WM8350_OUT1R_MUTE_MASK                  0x4000  /* OUT1R_MUTE */
#define WM8350_OUT1R_MUTE_SHIFT                     14  /* OUT1R_MUTE */
#define WM8350_OUT1R_ZC                         0x2000  /* OUT1R_ZC */
#define WM8350_OUT1R_ZC_MASK                    0x2000  /* OUT1R_ZC */
#define WM8350_OUT1R_ZC_SHIFT                       13  /* OUT1R_ZC */
#define WM8350_OUT1_VU                          0x0100  /* OUT1_VU */
#define WM8350_OUT1_VU_MASK                     0x0100  /* OUT1_VU */
#define WM8350_OUT1_VU_SHIFT                         8  /* OUT1_VU */
#define WM8350_OUT1R_VOL_MASK                   0x00FC  /* OUT1R_VOL - [7:2] */
#define WM8350_OUT1R_VOL_SHIFT                       2  /* OUT1R_VOL - [7:2] */

/* Bit values for R105 (0x69) */
#define WM8350_OUT1R_VOL_MIN                      0x00  /* minimum volume value */
#define WM8350_OUT1R_VOL_0DB                      0x39  /* 0dB volume value */
#define WM8350_OUT1R_VOL_MAX                      0x3F  /* maximum volume value */
#define WM8350_OUT1R_VOL( _dB )                   ((_dB) + WM8350_OUT1R_VOL_0DB)    /* 0 = -57dB, 1dB steps */
#define WM8350_OUT1R_DB( _vol )                   ((_vol) - WM8350_OUT1R_VOL_0DB)   /* 0 = -57dB, 1dB steps */

/*
 * R106 (0x6A) - OUT2L Volume
 */
#define WM8350_OUT2L_ENA_R106                   0x8000  /* OUT2L_ENA */
#define WM8350_OUT2L_ENA_R106_MASK              0x8000  /* OUT2L_ENA */
#define WM8350_OUT2L_ENA_R106_SHIFT                 15  /* OUT2L_ENA */
#define WM8350_OUT2L_MUTE                       0x4000  /* OUT2L_MUTE */
#define WM8350_OUT2L_MUTE_MASK                  0x4000  /* OUT2L_MUTE */
#define WM8350_OUT2L_MUTE_SHIFT                     14  /* OUT2L_MUTE */
#define WM8350_OUT2L_ZC                         0x2000  /* OUT2L_ZC */
#define WM8350_OUT2L_ZC_MASK                    0x2000  /* OUT2L_ZC */
#define WM8350_OUT2L_ZC_SHIFT                       13  /* OUT2L_ZC */
#define WM8350_OUT1_VU                          0x0100  /* OUT1_VU */
#define WM8350_OUT1_VU_MASK                     0x0100  /* OUT1_VU */
#define WM8350_OUT1_VU_SHIFT                         8  /* OUT1_VU */
#define WM8350_OUT2L_VOL_MASK                   0x00FC  /* OUT2L_VOL - [7:2] */
#define WM8350_OUT2L_VOL_SHIFT                       2  /* OUT2L_VOL - [7:2] */

/* Bit values for R104 (0x6A) */
#define WM8350_OUT2L_VOL_MIN                      0x00  /* minimum volume value */
#define WM8350_OUT2L_VOL_0DB                      0x39  /* 0dB volume value */
#define WM8350_OUT2L_VOL_MAX                      0x3F  /* maximum volume value */
#define WM8350_OUT2L_VOL( _dB )                   ((_dB) + WM8350_OUT2L_VOL_0DB)    /* 0 = -57dB, 1dB steps */
#define WM8350_OUT2L_DB( _vol )                   ((_vol) - WM8350_OUT2L_VOL_0DB)   /* 0 = -57dB, 1dB steps */

/*
 * R107 (0x6B) - OUT2R Volume
 */
#define WM8350_OUT2R_ENA_R107                   0x8000  /* OUT2R_ENA */
#define WM8350_OUT2R_ENA_R107_MASK              0x8000  /* OUT2R_ENA */
#define WM8350_OUT2R_ENA_R107_SHIFT                 15  /* OUT2R_ENA */
#define WM8350_OUT2R_MUTE                       0x4000  /* OUT2R_MUTE */
#define WM8350_OUT2R_MUTE_MASK                  0x4000  /* OUT2R_MUTE */
#define WM8350_OUT2R_MUTE_SHIFT                     14  /* OUT2R_MUTE */
#define WM8350_OUT2R_ZC                         0x2000  /* OUT2R_ZC */
#define WM8350_OUT2R_ZC_MASK                    0x2000  /* OUT2R_ZC */
#define WM8350_OUT2R_ZC_SHIFT                       13  /* OUT2R_ZC */
#define WM8350_OUT2R_INV                        0x0400  /* OUT2R_INV */
#define WM8350_OUT2R_INV_MASK                   0x0400  /* OUT2R_INV */
#define WM8350_OUT2R_INV_SHIFT                      10  /* OUT2R_INV */
#define WM8350_OUT2R_INV_MUTE                   0x0200  /* OUT2R_INV_MUTE */
#define WM8350_OUT2R_INV_MUTE_MASK              0x0200  /* OUT2R_INV_MUTE */
#define WM8350_OUT2R_INV_MUTE_SHIFT                  9  /* OUT2R_INV_MUTE */
#define WM8350_OUT2_VU                          0x0100  /* OUT2_VU */
#define WM8350_OUT2_VU_MASK                     0x0100  /* OUT2_VU */
#define WM8350_OUT2_VU_SHIFT                         8  /* OUT2_VU */
#define WM8350_OUT2R_VOL_MASK                   0x00FC  /* OUT2R_VOL - [7:2] */
#define WM8350_OUT2R_VOL_SHIFT                       2  /* OUT2R_VOL - [7:2] */

/* Bit values for R107 (0x6B) */
#define WM8350_OUT2R_VOL_MIN                      0x00  /* minimum volume value */
#define WM8350_OUT2R_VOL_0DB                      0x39  /* 0dB volume value */
#define WM8350_OUT2R_VOL_MAX                      0x3F  /* maximum volume value */
#define WM8350_OUT2R_VOL( _dB )                   ((_dB) + WM8350_OUT2R_VOL_0DB)    /* 0 = -57dB, 1dB steps */
#define WM8350_OUT2R_DB( _vol )                   ((_vol) - WM8350_OUT2R_VOL_0DB)   /* 0 = -57dB, 1dB steps */

/*
 * R111 (0x6F) - BEEP Volume
 */
#define WM8350_IN3R_TO_OUT2R_R111               0x8000  /* IN3R_TO_OUT2R */
#define WM8350_IN3R_OUT2R_VOL_MASK              0x00E0  /* IN3R_OUT2R_VOL - [7:5] */

/*
 * R112 (0x70) - AI Formating
 */
#define WM8350_AIF_BCLK_INV                     0x8000  /* AIF_BCLK_INV */
#define WM8350_AIF_BCLK_INV_MASK                0x8000  /* AIF_BCLK_INV */
#define WM8350_AIF_BCLK_INV_SHIFT                   15  /* AIF_BCLK_INV */
#define WM8350_AIF_TRI                          0x2000  /* AIF_TRI */
#define WM8350_AIF_TRI_MASK                     0x2000  /* AIF_TRI */
#define WM8350_AIF_TRI_SHIFT                        13  /* AIF_TRI */
#define WM8350_AIF_LRCLK_INV                    0x1000  /* AIF_LRCLK_INV */
#define WM8350_AIF_LRCLK_INV_MASK               0x1000  /* AIF_LRCLK_INV */
#define WM8350_AIF_LRCLK_INV_SHIFT                  12  /* AIF_LRCLK_INV */
#define WM8350_AIF_WL_MASK                      0x0C00  /* AIF_WL - [11:10] */
#define WM8350_AIF_WL_SHIFT                         10  /* AIF_WL - [11:10] */
#define WM8350_AIF_FMT_MASK                     0x0300  /* AIF_FMT - [9:8] */
#define WM8350_AIF_FMT_SHIFT                         8  /* AIF_FMT - [9:8] */

/* Bit values for R112 (0x70) */
#define WM8350_AIF_BCLK_INV_OFF                      0
#define WM8350_AIF_BCLK_INV_ON                       1

#define WM8350_AIF_TRI_ENABLED                       0  /* No tristating - interface enabled */
#define WM8350_AIF_TRI_TRISTATE                      1  /* Interface tristated */

#define WM8350_AIF_LRCLK_INV_OFF                     0
#define WM8350_AIF_LRCLK_INV_ON                      1
#define WM8350_AIF_DSP_MODE_LATE                     0
#define WM8350_AIF_DSP_MODE_EARLY                    1

#define WM8350_AIF_WL_16BIT                          0  /* 16-bit word length */
#define WM8350_AIF_WL_20BIT                          1  /* 20-bit word length */
#define WM8350_AIF_WL_24BIT                          2  /* 24-bit word length */
#define WM8350_AIF_WL_32BIT                          3  /* 32-bit word length */

#define WM8350_AIF_FMT_RIGHT_JUSTIFIED               0  /* Right-justified mode */
#define WM8350_AIF_FMT_LEFT_JUSTIFIED                1  /* Left-justified mode */
#define WM8350_AIF_FMT_I2S                           2  /* I2S */
#define WM8350_AIF_FMT_DSP                           3  /* DSP/PCM mode */

/*
 * R113 (0x71) - ADC DAC COMP
 */
#define WM8350_DAC_COMP                         0x0080  /* DAC_COMP */
#define WM8350_DAC_COMPMODE                     0x0040  /* DAC_COMPMODE */
#define WM8350_ADC_COMP                         0x0020  /* ADC_COMP */
#define WM8350_ADC_COMPMODE                     0x0010  /* ADC_COMPMODE */
#define WM8350_LOOPBACK                         0x0001  /* LOOPBACK */

/*
 * R114 (0x72) - AI ADC Control
 */
#define WM8350_AIFADC_PD                        0x0080  /* AIFADC_PD */
#define WM8350_AIFADCL_SRC                      0x0040  /* AIFADCL_SRC */
#define WM8350_AIFADCR_SRC                      0x0020  /* AIFADCR_SRC */
#define WM8350_AIFADC_TDM_CHAN                  0x0010  /* AIFADC_TDM_CHAN */
#define WM8350_AIFADC_TDM                       0x0008  /* AIFADC_TDM */

/*
 * R115 (0x73) - AI DAC Control
 */
#define WM8350_BCLK_MSTR                        0x4000  /* BCLK_MSTR */
#define WM8350_BCLK_MSTR_MASK                   0x4000  /* BCLK_MSTR */
#define WM8350_BCLK_MSTR_SHIFT                      14  /* BCLK_MSTR */
#define WM8350_AIFDAC_PD                        0x0080  /* AIFDAC_PD */
#define WM8350_AIFDAC_PD_MASK                   0x0080  /* AIFDAC_PD */
#define WM8350_AIFDAC_PD_SHIFT                       7  /* AIFDAC_PD */
#define WM8350_DACL_SRC                         0x0040  /* DACL_SRC */
#define WM8350_DACL_SRC_MASK                    0x0040  /* DACL_SRC */
#define WM8350_DACL_SRC_SHIFT                        6  /* DACL_SRC */
#define WM8350_DACR_SRC                         0x0020  /* DACR_SRC */
#define WM8350_DACR_SRC_MASK                    0x0020  /* DACR_SRC */
#define WM8350_DACR_SRC_SHIFT                        5  /* DACR_SRC */
#define WM8350_AIFDAC_TDM_CHAN                  0x0010  /* AIFDAC_TDM_CHAN */
#define WM8350_AIFDAC_TDM_CHAN_MASK             0x0010  /* AIFDAC_TDM_CHAN */
#define WM8350_AIFDAC_TDM_CHAN_SHIFT                 4  /* AIFDAC_TDM_CHAN */
#define WM8350_AIFDAC_TDM                       0x0008  /* AIFDAC_TDM */
#define WM8350_AIFDAC_TDM_MASK                  0x0008  /* AIFDAC_TDM */
#define WM8350_AIFDAC_TDM_SHIFT                      3  /* AIFDAC_TDM */
#define WM8350_DAC_BOOST_MASK                   0x0003  /* DAC_BOOST - [1:0] */
#define WM8350_DAC_BOOST_SHIFT                       0  /* DAC_BOOST - [1:0] */

/* Bit definitions for R115 (0x73) */
#define WM8350_BCLK_MSTR_SLAVE                       0
#define WM8350_BCLK_MSTR_MASTER                      1

#define WM8350_AIFDAC_PD_OFF                         0
#define WM8350_AIFDAC_PD_ENA                         1

#define WM8350_DACL_SRC_LEFT                         0
#define WM8350_DACL_SRC_RIGHT                        1

#define WM8350_DACR_SRC_LEFT                         0
#define WM8350_DACR_SRC_RIGHT                        1

#define WM8350_AIFDAC_TDM_CHAN_0                     0
#define WM8350_AIFDAC_TDM_CHAN_1                     1

#define WM8350_AIFDAC_TDM_OFF                        0
#define WM8350_AIFDAC_TDM_ENA                        1

#define WM8350_DAC_BOOST_0DB                         0
#define WM8350_DAC_BOOST_6DB                         1
#define WM8350_DAC_BOOST_12DB                        2

/*
 * R116 (0x74) - AIF Test
 */
#define WM8350_CODEC_BYP                        0x4000  /* CODEC_BYP */
#define WM8350_AIFADC_WR_TST                    0x2000  /* AIFADC_WR_TST */
#define WM8350_AIFADC_RD_TST                    0x1000  /* AIFADC_RD_TST */
#define WM8350_AIFDAC_WR_TST                    0x0800  /* AIFDAC_WR_TST */
#define WM8350_AIFDAC_RD_TST                    0x0400  /* AIFDAC_RD_TST */
#define WM8350_AIFADC_ASYN                      0x0020  /* AIFADC_ASYN */
#define WM8350_AIFDAC_ASYN                      0x0010  /* AIFDAC_ASYN */

/*
 * R128 (0x80) - GPIO Debounce
 */
#define WM8350_GP12_DB                          0x1000  /* GP12_DB */
#define WM8350_GP11_DB                          0x0800  /* GP11_DB */
#define WM8350_GP10_DB                          0x0400  /* GP10_DB */
#define WM8350_GP9_DB                           0x0200  /* GP9_DB */
#define WM8350_GP8_DB                           0x0100  /* GP8_DB */
#define WM8350_GP7_DB                           0x0080  /* GP7_DB */
#define WM8350_GP6_DB                           0x0040  /* GP6_DB */
#define WM8350_GP5_DB                           0x0020  /* GP5_DB */
#define WM8350_GP4_DB                           0x0010  /* GP4_DB */
#define WM8350_GP3_DB                           0x0008  /* GP3_DB */
#define WM8350_GP2_DB                           0x0004  /* GP2_DB */
#define WM8350_GP1_DB                           0x0002  /* GP1_DB */
#define WM8350_GP0_DB                           0x0001  /* GP0_DB */

/*
 * R129 (0x81) - GPIO Pin pull up Control
 */
#define WM8350_GP12_PU                          0x1000  /* GP12_PU */
#define WM8350_GP11_PU                          0x0800  /* GP11_PU */
#define WM8350_GP10_PU                          0x0400  /* GP10_PU */
#define WM8350_GP9_PU                           0x0200  /* GP9_PU */
#define WM8350_GP8_PU                           0x0100  /* GP8_PU */
#define WM8350_GP7_PU                           0x0080  /* GP7_PU */
#define WM8350_GP6_PU                           0x0040  /* GP6_PU */
#define WM8350_GP5_PU                           0x0020  /* GP5_PU */
#define WM8350_GP4_PU                           0x0010  /* GP4_PU */
#define WM8350_GP3_PU                           0x0008  /* GP3_PU */
#define WM8350_GP2_PU                           0x0004  /* GP2_PU */
#define WM8350_GP1_PU                           0x0002  /* GP1_PU */
#define WM8350_GP0_PU                           0x0001  /* GP0_PU */

/*
 * R130 (0x82) - GPIO Pull down Control
 */
#define WM8350_GP12_PD                          0x1000  /* GP12_PD */
#define WM8350_GP11_PD                          0x0800  /* GP11_PD */
#define WM8350_GP10_PD                          0x0400  /* GP10_PD */
#define WM8350_GP9_PD                           0x0200  /* GP9_PD */
#define WM8350_GP8_PD                           0x0100  /* GP8_PD */
#define WM8350_GP7_PD                           0x0080  /* GP7_PD */
#define WM8350_GP6_PD                           0x0040  /* GP6_PD */
#define WM8350_GP5_PD                           0x0020  /* GP5_PD */
#define WM8350_GP4_PD                           0x0010  /* GP4_PD */
#define WM8350_GP3_PD                           0x0008  /* GP3_PD */
#define WM8350_GP2_PD                           0x0004  /* GP2_PD */
#define WM8350_GP1_PD                           0x0002  /* GP1_PD */
#define WM8350_GP0_PD                           0x0001  /* GP0_PD */

/*
 * R131 (0x83) - GPIO Interrupt Mode
 */
#define WM8350_GP12_INTMODE                     0x1000  /* GP12_INTMODE */
#define WM8350_GP11_INTMODE                     0x0800  /* GP11_INTMODE */
#define WM8350_GP10_INTMODE                     0x0400  /* GP10_INTMODE */
#define WM8350_GP9_INTMODE                      0x0200  /* GP9_INTMODE */
#define WM8350_GP8_INTMODE                      0x0100  /* GP8_INTMODE */
#define WM8350_GP7_INTMODE                      0x0080  /* GP7_INTMODE */
#define WM8350_GP6_INTMODE                      0x0040  /* GP6_INTMODE */
#define WM8350_GP5_INTMODE                      0x0020  /* GP5_INTMODE */
#define WM8350_GP4_INTMODE                      0x0010  /* GP4_INTMODE */
#define WM8350_GP3_INTMODE                      0x0008  /* GP3_INTMODE */
#define WM8350_GP2_INTMODE                      0x0004  /* GP2_INTMODE */
#define WM8350_GP1_INTMODE                      0x0002  /* GP1_INTMODE */
#define WM8350_GP0_INTMODE                      0x0001  /* GP0_INTMODE */

/*
 * R133 (0x85) - GPIO Control
 */
#define WM8350_GP_DBTIME_MASK                   0x00C0  /* GP_DBTIME - [7:6] */

/*
 * R134 (0x86) - GPIO Configuration (i/o)
 */
#define WM8350_GP12_DIR_MASK                    0x1000  /* GP12_DIR */
#define WM8350_GP12_DIR_SHIFT                       12  /* GP12_DIR */
#define WM8350_GP11_DIR_MASK                    0x0800  /* GP11_DIR */
#define WM8350_GP11_DIR_SHIFT                       11  /* GP11_DIR */
#define WM8350_GP10_DIR_MASK                    0x0400  /* GP10_DIR */
#define WM8350_GP10_DIR_SHIFT                       10  /* GP10_DIR */
#define WM8350_GP9_DIR_MASK                     0x0200  /* GP9_DIR */
#define WM8350_GP9_DIR_SHIFT                         9  /* GP9_DIR */
#define WM8350_GP8_DIR_MASK                     0x0100  /* GP8_DIR */
#define WM8350_GP8_DIR_SHIFT                         8  /* GP8_DIR */
#define WM8350_GP7_DIR_MASK                     0x0080  /* GP7_DIR */
#define WM8350_GP7_DIR_SHIFT                         7  /* GP7_DIR */
#define WM8350_GP6_DIR_MASK                     0x0040  /* GP6_DIR */
#define WM8350_GP6_DIR_SHIFT                         6  /* GP6_DIR */
#define WM8350_GP5_DIR_MASK                     0x0020  /* GP5_DIR */
#define WM8350_GP5_DIR_SHIFT                         5  /* GP5_DIR */
#define WM8350_GP4_DIR_MASK                     0x0010  /* GP4_DIR */
#define WM8350_GP4_DIR_SHIFT                         4  /* GP4_DIR */
#define WM8350_GP3_DIR_MASK                     0x0008  /* GP3_DIR */
#define WM8350_GP3_DIR_SHIFT                         3  /* GP3_DIR */
#define WM8350_GP2_DIR_MASK                     0x0004  /* GP2_DIR */
#define WM8350_GP2_DIR_SHIFT                         2  /* GP2_DIR */
#define WM8350_GP1_DIR_MASK                     0x0002  /* GP1_DIR */
#define WM8350_GP1_DIR_SHIFT                         1  /* GP1_DIR */
#define WM8350_GP0_DIR_MASK                     0x0001  /* GP0_DIR */
#define WM8350_GP0_DIR_SHIFT                         0  /* GP0_DIR */

/* Bit definitions for R134 (0x86) */
#define WM8350_GP12_DIR_OUT                          0  /* 0 = output */
#define WM8350_GP12_DIR_IN                           1  /* 1 = input */

#define WM8350_GP11_DIR_OUT                          0  /* 0 = output */
#define WM8350_GP11_DIR_IN                           1  /* 1 = input */

#define WM8350_GP10_DIR_OUT                          0  /* 0 = output */
#define WM8350_GP10_DIR_IN                           1  /* 1 = input */

#define WM8350_GP9_DIR_OUT                           0  /* 0 = output */
#define WM8350_GP9_DIR_IN                            1  /* 1 = input */

#define WM8350_GP8_DIR_OUT                           0  /* 0 = output */
#define WM8350_GP8_DIR_IN                            1  /* 1 = input */

#define WM8350_GP7_DIR_OUT                           0  /* 0 = output */
#define WM8350_GP7_DIR_IN                            1  /* 1 = input */

#define WM8350_GP6_DIR_OUT                           0  /* 0 = output */
#define WM8350_GP6_DIR_IN                            1  /* 1 = input */

#define WM8350_GP5_DIR_OUT                           0  /* 0 = output */
#define WM8350_GP5_DIR_IN                            1  /* 1 = input */

#define WM8350_GP4_DIR_OUT                           0  /* 0 = output */
#define WM8350_GP4_DIR_IN                            1  /* 1 = input */

#define WM8350_GP3_DIR_OUT                           0  /* 0 = output */
#define WM8350_GP3_DIR_IN                            1  /* 1 = input */

#define WM8350_GP2_DIR_OUT                           0  /* 0 = output */
#define WM8350_GP2_DIR_IN                            1  /* 1 = input */

#define WM8350_GP1_DIR_OUT                           0  /* 0 = output */
#define WM8350_GP1_DIR_IN                            1  /* 1 = input */

#define WM8350_GP0_DIR_OUT                           0  /* 0 = output */
#define WM8350_GP0_DIR_IN                            1  /* 1 = input */

/*
 * R135 (0x87) - GPIO Pin Polarity / Type
 */
#define WM8350_GP12_CFG                         0x1000  /* GP12_CFG */
#define WM8350_GP11_CFG                         0x0800  /* GP11_CFG */
#define WM8350_GP10_CFG                         0x0400  /* GP10_CFG */
#define WM8350_GP9_CFG                          0x0200  /* GP9_CFG */
#define WM8350_GP8_CFG                          0x0100  /* GP8_CFG */
#define WM8350_GP7_CFG                          0x0080  /* GP7_CFG */
#define WM8350_GP6_CFG                          0x0040  /* GP6_CFG */
#define WM8350_GP5_CFG                          0x0020  /* GP5_CFG */
#define WM8350_GP4_CFG                          0x0010  /* GP4_CFG */
#define WM8350_GP3_CFG                          0x0008  /* GP3_CFG */
#define WM8350_GP2_CFG                          0x0004  /* GP2_CFG */
#define WM8350_GP1_CFG                          0x0002  /* GP1_CFG */
#define WM8350_GP0_CFG                          0x0001  /* GP0_CFG */

/*
 * R140 (0x8C) - GPIO Function Select 1
 */
#define WM8350_GP3_FN_MASK                      0xF000  /* GP3_FN - [15:12] */
#define WM8350_GP3_FN_SHIFT                         12  /* GP3_FN - [15:12] */
#define WM8350_GP2_FN_MASK                      0x0F00  /* GP2_FN - [11:8] */
#define WM8350_GP2_FN_SHIFT                          8  /* GP2_FN - [11:8] */
#define WM8350_GP1_FN_MASK                      0x00F0  /* GP1_FN - [7:4] */
#define WM8350_GP1_FN_SHIFT                          4  /* GP1_FN - [7:4] */
#define WM8350_GP0_FN_MASK                      0x000F  /* GP0_FN - [3:0] */
#define WM8350_GP0_FN_SHIFT                          0  /* GP0_FN - [3:0] */

/* Bit definitions for R140 (0x8C) */
/** Alternate functions for GPIO 0 - inputs */
#define WM8350_GP0_FN_GPIO                           0
#define WM8350_GP0_FN_PWR_ON                         1
#define WM8350_GP0_FN_LDO_EN                         2
#define WM8350_GP0_FN_LPWR1                          3
#define WM8350_GP0_FN_PWR_OFF                        4

/** Alternate functions for GPIO 0 - outputs */
#define WM8350_GP0_FN_GPIO                           0
#define WM8350_GP0_FN_PWR_ON                         1
#define WM8350_GP0_FN_VRTC                           2
#define WM8350_GP0_FN_POR_B                          3
#define WM8350_GP0_FN_RST                            4

/** Alternate functions for GPIO 1 - inputs */
#define WM8350_GP1_FN_GPIO                           0
#define WM8350_GP1_FN_PWR_ON                         1
#define WM8350_GP1_FN_LDO_EN                         2
#define WM8350_GP1_FN_LPWR2                          3
#define WM8350_GP1_FN_WAKEUP                         4

/** Alternate functions for GPIO 1 - outputs */
#define WM8350_GP1_FN_GPIO                           0
#define WM8350_GP1_FN_DO_CONF                        1
#define WM8350_GP1_FN_RST                            2
#define WM8350_GP1_FN_MEMRST                         3
#define WM8350_GP1_FN_32KHZ                          4

/** Alternate functions for GPIO 2 - inputs */
#define WM8350_GP2_FN_GPIO                           0
#define WM8350_GP2_FN_PWR_ON                         1
#define WM8350_GP2_FN_WAKEUP                         2
#define WM8350_GP2_FN_32KHZ                          3
#define WM8350_GP2_FN_L_PWR3                         4

/** Alternate functions for GPIO 2 - outputs */
#define WM8350_GP2_FN_GPIO                           0
#define WM8350_GP2_FN_PWR_ON                         1
#define WM8350_GP2_FN_VRTC                           2
#define WM8350_GP2_FN_32KHZ                          3
#define WM8350_GP2_FN_RST                            4

/** Alternate functions for GPIO 3 - inputs */
#define WM8350_GP3_FN_GPIO                           0
#define WM8350_GP3_FN_PWR_ON                         1
#define WM8350_GP3_FN_LDO_EN                         2
#define WM8350_GP3_FN_PWR_OFF                        3
#define WM8350_GP3_FN_FLASH                          4

/** Alternate functions for GPIO 3 - outputs */
#define WM8350_GP3_FN_GPIO                           0
#define WM8350_GP3_FN_P_CLK                          1
#define WM8350_GP3_FN_VRTC                           2
#define WM8350_GP3_FN_32KHZ                          3
#define WM8350_GP3_FN_MEMRST                         4

/*
 * R141 (0x8D) - GPIO Function Select 2
 */
#define WM8350_GP7_FN_MASK                      0xF000  /* GP7_FN - [15:12] */
#define WM8350_GP7_FN_SHIFT                         12  /* GP7_FN - [15:12] */
#define WM8350_GP6_FN_MASK                      0x0F00  /* GP6_FN - [11:8] */
#define WM8350_GP6_FN_SHIFT                          8  /* GP6_FN - [11:8] */
#define WM8350_GP5_FN_MASK                      0x00F0  /* GP5_FN - [7:4] */
#define WM8350_GP5_FN_SHIFT                          4  /* GP5_FN - [7:4] */
#define WM8350_GP4_FN_MASK                      0x000F  /* GP4_FN - [3:0] */
#define WM8350_GP4_FN_SHIFT                          0  /* GP4_FN - [3:0] */

/* Bit definitions for R141 (0x8D) */
/** Alternate functions for GPIO 4 - inputs */
#define WM8350_GP4_FN_GPIO                           0
#define WM8350_GP4_FN_MR                             1
#define WM8350_GP4_FN_FLASH                          2
#define WM8350_GP4_FN_HIBERNATE                      3
#define WM8350_GP4_FN_MASK_IN                        4

/** Alternate functions for GPIO 4 - outputs */
#define WM8350_GP4_FN_GPIO                           0
#define WM8350_GP4_FN_MEMRST                         1
#define WM8350_GP4_FN_ADA                            2
#define WM8350_GP4_FN_FLASH_OUT                      3
#define WM8350_GP4_FN_VCC_FAULT                      4
#define WM8350_GP4_FN_MICSHT                         5
#define WM8350_GP4_FN_MICDET                        10

/** Alternate functions for GPIO 5 - inputs */
#define WM8350_GP5_FN_GPIO                           0
#define WM8350_GP5_FN_LPWR1                          1
#define WM8350_GP5_FN_ADCLRCLK                       2
#define WM8350_GP5_FN_HIBERNATE                      3
#define WM8350_GP5_FN_PWR_OFF                        4

/** Alternate functions for GPIO 5 - outputs */
#define WM8350_GP5_FN_GPIO                           0
#define WM8350_GP5_FN_P_CLK                          1
#define WM8350_GP5_FN_ADCLRCLK                       2
#define WM8350_GP5_FN_OSC32KHZ                       3
#define WM8350_GP5_FN_BATT_FAULT                     4
#define WM8350_GP5_FN_MICSHT                         5
#define WM8350_GP5_FN_ADA                            6
#define WM8350_GP5_FN_CODEC_OPCLK                    7
#define WM8350_GP5_FN_MICDET                        10

/** Alternate functions for GPIO 6 - inputs */
#define WM8350_GP6_FN_GPIO                           0
#define WM8350_GP6_FN_LPWR2                          1
#define WM8350_GP6_FN_FLASH                          2
#define WM8350_GP6_FN_HIBERNATE                      3

/** Alternate functions for GPIO 6 - outputs */
#define WM8350_GP6_FN_GPIO                           0
#define WM8350_GP6_FN_MEMRST                         1
#define WM8350_GP6_FN_ADA                            2
#define WM8350_GP6_FN_RTC_DSW                        3
#define WM8350_GP6_FN_MICDET                         4
#define WM8350_GP6_FN_MICSHT                         5
#define WM8350_GP6_FN_ADCLRCLKB                      6

/** Alternate functions for GPIO 7 - inputs */
#define WM8350_GP7_FN_GPIO                           0
#define WM8350_GP7_FN_LPWR3                          1
#define WM8350_GP7_FN_MASK_IN                        2
#define WM8350_GP7_FN_HIBERNATE                      3

/** Alternate functions for GPIO 7 - outputs */
#define WM8350_GP7_FN_GPIO                           0
#define WM8350_GP7_FN_P_CLK                          1
#define WM8350_GP7_FN_VCC_FAULT                      2
#define WM8350_GP7_FN_BATT_FAULT                     3
#define WM8350_GP7_FN_MICDET                         4
#define WM8350_GP7_FN_MICSHT                         5
#define WM8350_GP7_FN_ADA                            6
#define WM8350_GP7_FN_FLL_CLK                       12

/*
 * R142 (0x8E) - GPIO Function Select 3
 */
#define WM8350_GP11_FN_MASK                     0xF000  /* GP11_FN - [15:12] */
#define WM8350_GP11_FN_SHIFT                        12  /* GP11_FN - [15:12] */
#define WM8350_GP10_FN_MASK                     0x0F00  /* GP10_FN - [11:8] */
#define WM8350_GP10_FN_SHIFT                         8  /* GP10_FN - [11:8] */
#define WM8350_GP9_FN_MASK                      0x00F0  /* GP9_FN - [7:4] */
#define WM8350_GP9_FN_SHIFT                          4  /* GP9_FN - [7:4] */
#define WM8350_GP8_FN_MASK                      0x000F  /* GP8_FN - [3:0] */
#define WM8350_GP8_FN_SHIFT                          0  /* GP8_FN - [3:0] */

/* Bit definitions for R142 (0x8E) */
/** Alternate functions for GPIO 8 - inputs */
#define WM8350_GP8_FN_GPIO                           0
#define WM8350_GP8_FN_MR                             1
#define WM8350_GP8_FN_ADCBCLK                        2
#define WM8350_GP8_FN_PWR_OFF                        3
#define WM8350_GP8_FN_HIBERNATE                      4

/** Alternate functions for GPIO 8 - outputs */
#define WM8350_GP8_FN_GPIO                           0
#define WM8350_GP8_FN_VCC_FAULT                      1
#define WM8350_GP8_FN_ADCBCLK                        2
#define WM8350_GP8_FN_BATT_FAULT                     3
#define WM8350_GP8_FN_RST                            4

/** Alternate functions for GPIO 9 - inputs */
#define WM8350_GP9_FN_GPIO                           0
#define WM8350_GP9_FN_HEARTBEAT                      1
#define WM8350_GP9_FN_MASK_IN                        2
#define WM8350_GP9_FN_PWR_OFF                        3
#define WM8350_GP9_FN_HIBERNATE                      4

/** Alternate functions for GPIO 9 - outputs */
#define WM8350_GP9_FN_GPIO                           0
#define WM8350_GP9_FN_VCC_FAULT                      1
#define WM8350_GP9_FN_LINE_GT_BATT                   2
#define WM8350_GP9_FN_BATT_FAULT                     3
#define WM8350_GP9_FN_MEMRST                         4

/** Alternate functions for GPIO 10 - inputs */
#define WM8350_GP10_FN_GPIO                          0
#define WM8350_GP10_FN_PWR_OFF                       3

/** Alternate functions for GPIO 10 - outputs */
#define WM8350_GP10_FN_GPIO                          0
#define WM8350_GP10_FN_ISINKC                        1
#define WM8350_GP10_FN_LINE_GT_BATT                  2
#define WM8350_GP10_FN_CH_IND                        3

/** Alternate functions for GPIO 11 - inputs */
#define WM8350_GP11_FN_GPIO                          0
#define WM8350_GP11_FN_WAKEUP                        2

/** Alternate functions for GPIO 11 - outputs */
#define WM8350_GP11_FN_GPIO                          0
#define WM8350_GP11_FN_ISINKD                        1
#define WM8350_GP11_FN_LINE_GT_BATT                  2
#define WM8350_GP11_FN_CH_IND                        3

/*
 * R143 (0x8F) - GPIO Function Select 4
 */
#define WM8350_GP12_FN_MASK                     0x000F  /* GP12_FN - [3:0] */
#define WM8350_GP12_FN_SHIFT                         0  /* GP12_FN - [3:0] */

/* Bit definitions for R141 (0x8D) */
/** Alternate functions for GPIO 12 - inputs */
#define WM8350_GP12_FN_GPIO                          0

/** Alternate functions for GPIO 12 - outputs */
#define WM8350_GP12_FN_GPIO                          0
#define WM8350_GP12_FN_ISINKE                        1
#define WM8350_GP12_FN_LINE_GT_BATT                  2
#define WM8350_GP12_FN_LIN_EN                        3
#define WM8350_GP12_FN_32KHZ                         4

/*
 * R144 (0x90) - Digitiser Control (1)
 */
#define WM8350_AUXADC_ENA_R144                  0x8000  /* AUXADC_ENA */
#define WM8350_AUXADC_CTC                       0x4000  /* AUXADC_CTC */
#define WM8350_AUXADC_POLL                      0x2000  /* AUXADC_POLL */
#define WM8350_AUXADC_HIB_MODE                  0x1000  /* AUXADC_HIB_MODE */
#define WM8350_AUXADC_SEL8                      0x0080  /* AUXADC_SEL8 */
#define WM8350_AUXADC_SEL7                      0x0040  /* AUXADC_SEL7 */
#define WM8350_AUXADC_SEL6                      0x0020  /* AUXADC_SEL6 */
#define WM8350_AUXADC_SEL5                      0x0010  /* AUXADC_SEL5 */
#define WM8350_AUXADC_SEL4                      0x0008  /* AUXADC_SEL4 */
#define WM8350_AUXADC_SEL3                      0x0004  /* AUXADC_SEL3 */
#define WM8350_AUXADC_SEL2                      0x0002  /* AUXADC_SEL2 */
#define WM8350_AUXADC_SEL1                      0x0001  /* AUXADC_SEL1 */

/*
 * R145 (0x91) - Digitiser Control (2)
 */
#define WM8350_AUXADC_MASKMODE_MASK             0x3000  /* AUXADC_MASKMODE - [13:12] */
#define WM8350_AUXADC_CRATE_MASK                0x0700  /* AUXADC_CRATE - [10:8] */
#define WM8350_AUXADC_CAL                       0x0004  /* AUXADC_CAL */
#define WM8350_AUXADC_RBMODE                    0x0002  /* AUXADC_RBMODE */
#define WM8350_AUXADC_WAIT                      0x0001  /* AUXADC_WAIT */

/*
 * R152 (0x98) - AUX1 Readback
 */
#define WM8350_AUXADC_SCALE1_MASK               0x6000  /* AUXADC_SCALE1 - [14:13] */
#define WM8350_AUXADC_REF1                      0x1000  /* AUXADC_REF1 */
#define WM8350_AUXADC_DATA1_MASK                0x0FFF  /* AUXADC_DATA1 - [11:0] */

/*
 * R153 (0x99) - AUX2 Readback
 */
#define WM8350_AUXADC_SCALE2_MASK               0x6000  /* AUXADC_SCALE2 - [14:13] */
#define WM8350_AUXADC_REF2                      0x1000  /* AUXADC_REF2 */
#define WM8350_AUXADC_DATA2_MASK                0x0FFF  /* AUXADC_DATA2 - [11:0] */

/*
 * R154 (0x9A) - AUX3 Readback
 */
#define WM8350_AUXADC_SCALE3_MASK               0x6000  /* AUXADC_SCALE3 - [14:13] */
#define WM8350_AUXADC_REF3                      0x1000  /* AUXADC_REF3 */
#define WM8350_AUXADC_DATA3_MASK                0x0FFF  /* AUXADC_DATA3 - [11:0] */

/*
 * R155 (0x9B) - AUX4 Readback
 */
#define WM8350_AUXADC_SCALE4_MASK               0x6000  /* AUXADC_SCALE4 - [14:13] */
#define WM8350_AUXADC_REF4                      0x1000  /* AUXADC_REF4 */
#define WM8350_AUXADC_DATA4_MASK                0x0FFF  /* AUXADC_DATA4 - [11:0] */

/*
 * R156 (0x9C) - USB Voltage Readback
 */
#define WM8350_AUXADC_DATA_USB_MASK             0x0FFF  /* AUXADC_DATA_USB - [11:0] */

/*
 * R157 (0x9D) - LINE Voltage Readback
 */
#define WM8350_AUXADC_DATA_LINE_MASK            0x0FFF  /* AUXADC_DATA_LINE - [11:0] */

/*
 * R158 (0x9E) - BATT Voltage Readback
 */
#define WM8350_AUXADC_DATA_BATT_MASK            0x0FFF  /* AUXADC_DATA_BATT - [11:0] */

/*
 * R159 (0x9F) - Chip Temp Readback
 */
#define WM8350_AUXADC_DATA_CHIPTEMP_MASK        0x0FFF  /* AUXADC_DATA_CHIPTEMP - [11:0] */

/*
 * R163 (0xA3) - Generic Comparator Control
 */
#define WM8350_DCMP4_ENA_R163                   0x0008  /* DCMP4_ENA */
#define WM8350_DCMP3_ENA_R163                   0x0004  /* DCMP3_ENA */
#define WM8350_DCMP2_ENA_R163                   0x0002  /* DCMP2_ENA */
#define WM8350_DCMP1_ENA_R163                   0x0001  /* DCMP1_ENA */

/*
 * R164 (0xA4) - Generic comparator 1
 */
#define WM8350_DCMP1_SRCSEL_MASK                0xE000  /* DCMP1_SRCSEL - [15:13] */
#define WM8350_DCMP1_GT                         0x1000  /* DCMP1_GT */
#define WM8350_DCMP1_THR_MASK                   0x0FFF  /* DCMP1_THR - [11:0] */

/*
 * R165 (0xA5) - Generic comparator 2
 */
#define WM8350_DCMP2_SRCSEL_MASK                0xE000  /* DCMP2_SRCSEL - [15:13] */
#define WM8350_DCMP2_GT                         0x1000  /* DCMP2_GT */
#define WM8350_DCMP2_THR_MASK                   0x0FFF  /* DCMP2_THR - [11:0] */

/*
 * R166 (0xA6) - Generic comparator 3
 */
#define WM8350_DCMP3_SRCSEL_MASK                0xE000  /* DCMP3_SRCSEL - [15:13] */
#define WM8350_DCMP3_GT                         0x1000  /* DCMP3_GT */
#define WM8350_DCMP3_THR_MASK                   0x0FFF  /* DCMP3_THR - [11:0] */

/*
 * R167 (0xA7) - Generic comparator 4
 */
#define WM8350_DCMP4_SRCSEL_MASK                0xE000  /* DCMP4_SRCSEL - [15:13] */
#define WM8350_DCMP4_GT                         0x1000  /* DCMP4_GT */
#define WM8350_DCMP4_THR_MASK                   0x0FFF  /* DCMP4_THR - [11:0] */

/*
 * R168 (0xA8) - Battery Charger Control 1
 */
#define WM8350_CHG_ENA_R168                     0x8000  /* CHG_ENA */
#define WM8350_CHG_ENA_R168_MASK                0x8000  /* CHG_ENA */
#define WM8350_CHG_ENA_R168_SHIFT                   15  /* CHG_ENA */
#define WM8350_CHG_THR                          0x2000  /* CHG_THR */
#define WM8350_CHG_THR_MASK                     0x2000  /* CHG_THR */
#define WM8350_CHG_THR_SHIFT                        13  /* CHG_THR */
#define WM8350_CHG_EOC_SEL                      0x1C00  /* CHG_EOC_SEL - [12:10] */
#define WM8350_CHG_EOC_SEL_MASK                 0x1C00  /* CHG_EOC_SEL - [12:10] */
#define WM8350_CHG_EOC_SEL_SHIFT                    10  /* CHG_EOC_SEL - [12:10] */
#define WM8350_CHG_TRICKLE_TEMP_CHOKE           0x0200  /* CHG_TRICKLE_TEMP_CHOKE */
#define WM8350_CHG_TRICKLE_TEMP_CHOKE_MASK      0x0200  /* CHG_TRICKLE_TEMP_CHOKE */
#define WM8350_CHG_TRICKLE_TEMP_CHOKE_SHIFT          9  /* CHG_TRICKLE_TEMP_CHOKE */
#define WM8350_CHG_TRICKLE_USB_CHOKE            0x0100  /* CHG_TRICKLE_USB_CHOKE */
#define WM8350_CHG_TRICKLE_USB_MASK             0x0100  /* CHG_TRICKLE_USB_CHOKE */
#define WM8350_CHG_TRICKLE_USB_SHIFT                 8  /* CHG_TRICKLE_USB_CHOKE */
#define WM8350_CHG_RECOVER_T                    0x0080  /* CHG_RECOVER_T */
#define WM8350_CHG_RECOVER_T_MASK               0x0080  /* CHG_RECOVER_T */
#define WM8350_CHG_RECOVER_T_SHIFT                   7  /* CHG_RECOVER_T */
#define WM8350_CHG_END_ACT                      0x0040  /* CHG_END_ACT */
#define WM8350_CHG_END_ACT_MASK                 0x0040  /* CHG_END_ACT */
#define WM8350_CHG_END_ACT_SHIFT                     6  /* CHG_END_ACT */
#define WM8350_CHG_FAST                         0x0020  /* CHG_FAST */
#define WM8350_CHG_FAST_MASK                    0x0020  /* CHG_FAST */
#define WM8350_CHG_FAST_SHIFT                        5  /* CHG_FAST */
#define WM8350_CHG_FAST_USB_THROTTLE            0x0010  /* CHG_FAST_USB_THROTTLE */
#define WM8350_CHG_FAST_USB_THROTTLE_MASK       0x0010  /* CHG_FAST_USB_THROTTLE */
#define WM8350_CHG_FAST_USB_THROTTLE_SHIFT           4  /* CHG_FAST_USB_THROTTLE */
#define WM8350_CHG_NTC_MON                      0x0008  /* CHG_NTC_MON */
#define WM8350_CHG_NTC_MON_MASK                 0x0008  /* CHG_NTC_MON */
#define WM8350_CHG_NTC_MON_SHIFT                     3  /* CHG_NTC_MON */
#define WM8350_CHG_BATT_HOT_MON                 0x0004  /* CHG_BATT_HOT_MON */
#define WM8350_CHG_BATT_HOT_MON_MASK            0x0004  /* CHG_BATT_HOT_MON */
#define WM8350_CHG_BATT_HOT_MON_SHIFT                2  /* CHG_BATT_HOT_MON */
#define WM8350_CHG_BATT_COLD_MON                0x0002  /* CHG_BATT_COLD_MON */
#define WM8350_CHG_BATT_COLD_MON_MASK           0x0002  /* CHG_BATT_COLD_MON */
#define WM8350_CHG_BATT_COLD_MON_SHIFT               1  /* CHG_BATT_COLD_MON */
#define WM8350_CHG_CHIP_TEMP_MON                0x0001  /* CHG_CHIP_TEMP_MON */
#define WM8350_CHG_CHIP_TEMP_MON_MASK           0x0001  /* CHG_CHIP_TEMP_MON */
#define WM8350_CHG_CHIP_TEMP_MON_SHIFT               0  /* CHG_CHIP_TEMP_MON */

/* Bit values for R168 (0xA8) */
#define WM8350_CHG_ENA_R168_ENABLE              1
#define WM8350_CHG_ENA_R168_DISABLE             0
#define WM8350_EOC_SEL( _Ma )                   (((_Ma)-20)/10)
#define WM8350_ECO_SEL_V( _val )                (((_val)*10)+20)

/*
 * R169 (0xA9) - Battery Charger Control 2
 */
#define WM8350_CHG_ACTIVE                       0x8000  /* CHG_ACTIVE */
#define WM8350_CHG_ACTIVE_MASK                  0x8000  /* CHG_ACTIVE */
#define WM8350_CHG_ACTIVE_SHIFT                     15  /* CHG_ACTIVE */
#define WM8350_CHG_PAUSE                        0x4000  /* CHG_PAUSE */
#define WM8350_CHG_PAUSE_MASK                   0x4000  /* CHG_PAUSE */
#define WM8350_CHG_PAUSE_SHIFT                      14  /* CHG_PAUSE */
#define WM8350_CHG_STS                          0x3000  /* CHG_STS - [13:12] */
#define WM8350_CHG_STS_MASK                     0x3000  /* CHG_STS - [13:12] */
#define WM8350_CHG_STS_SHIFT                        12  /* CHG_STS - [13:12] */
#define WM8350_CHG_TIME                         0x0F00  /* CHG_TIME - [11:8] */
#define WM8350_CHG_TIME_MASK                    0x0F00  /* CHG_TIME - [11:8] */
#define WM8350_CHG_TIME_SHIFT                        8  /* CHG_TIME - [11:8] */
#define WM8350_CHG_MASK_WALL_FB                 0x0080  /* CHG_MASK_WALL_FB */
#define WM8350_CHG_MASK_WALL_FB_MASK            0x0080  /* CHG_MASK_WALL_FB */
#define WM8350_CHG_MASK_WALL_FB_SHIFT           0x0080  /* CHG_MASK_WALL_FB */
#define WM8350_CHG_TRICKLE_SEL                  0x0040  /* CHG_TRICKLE_SEL */
#define WM8350_CHG_TRICKLE_SEL_MASK             0x0040  /* CHG_TRICKLE_SEL */
#define WM8350_CHG_TRICKLE_SEL_SHIFT                 6  /* CHG_TRICKLE_SEL */
#define WM8350_CHG_VSEL                         0x0030  /* CHG_VSEL - [5:4] */
#define WM8350_CHG_VSEL_MASK                    0x0030  /* CHG_VSEL - [5:4] */
#define WM8350_CHG_VSEL_SHIFT                        4  /* CHG_VSEL - [5:4] */
#define WM8350_CHG_ISEL                         0x000F  /* CHG_ISEL - [3:0] */
#define WM8350_CHG_ISEL_MASK                    0x000F  /* CHG_ISEL - [3:0] */
#define WM8350_CHG_ISEL_SHIFT                        0  /* CHG_ISEL - [3:0] */

/* Bit values for R168 (0xA8) */
#define WM8350_CHARGER_PAUSE                         1
#define WM8350_CHARGER_RESUME                         0
#define WM8350_CHG_STS_CUR_ZERO                      0
#define WM8350_CHG_STS_TRICKLE                       1
#define WM8350_CHG_STS_FAST                          2
#define WM8350_CHG_STS_RESERVED                      3
#define WM8350_TIME( _min )         (((_min)*60)/1024)
#define WM8350_TIME_V( _val )         ((_val*1024)/60)
#define WM8350_CHG_VSEL_4_05                         0
#define WM8350_CHG_VSEL_4_1                          1
#define WM8350_CHG_VSEL_4_15                         2
#define WM8350_CHG_VSEL_4_2                          3
#define WM8350_ISEL( _Ma )            ((_Ma)/(750/15))
#define WM8350_ISEL_V( _val )        ((_val)*(750/15))

/*
 * R170 (0xAA) - Battery Charger Control 3
 */
#define WM8350_CHG_THROTTLE_T                   0x0060  /* CHG_THROTTLE_T - [6:5] */
#define WM8350_CHG_THROTTLE_T_MASK              0x0060  /* CHG_THROTTLE_T - [6:5] */
#define WM8350_CHG_THROTTLE_T_SHIFT                  5  /* CHG_THROTTLE_T - [6:5] */
#define WM8350_CHG_TIMER_ADJT                   0x000F  /* CHG_TIMER_ADJT - [3:0] */
#define WM8350_CHG_TIMER_ADJT_MASK              0x000F  /* CHG_TIMER_ADJT - [3:0] */
#define WM8350_CHG_TIMER_ADJT_SHIFT                  0  /* CHG_TIMER_ADJT - [3:0] */

#define WM8350_CHG_THROTTLE_T_8_US                     0
#define WM8350_CHG_THROTTLE_T_16_US                     1
#define WM8350_CHG_THROTTLE_T_32_US                     2
#define WM8350_CHG_THROTTLE_T_64_US                     3

/*
 * R172 (0xAC) - Current Sink Driver A
 */
#define WM8350_CS1_ENA_R172                     0x8000  /* CS1_ENA */
#define WM8350_CS1_HIB_MODE                     0x1000  /* CS1_HIB_MODE */
#define WM8350_CS1_HIB_MODE_MASK                0x1000  /* CS1_HIB_MODE */
#define WM8350_CS1_HIB_MODE_SHIFT                   12  /* CS1_HIB_MODE */
#define WM8350_CS1_ISEL_MASK                    0x003F  /* CS1_ISEL - [5:0] */
#define WM8350_CS1_ISEL_SHIFT                        0  /* CS1_ISEL - [5:0] */

/* Bit values for R172 (0xAC) */
#define WM8350_CS1_HIB_MODE_DISABLE                  0  /* Disable current sink in hibernate */
#define WM8350_CS1_HIB_MODE_LEAVE                    1  /* Leave current sink as-is in hibernate */

#define WM8350_CS1_ISEL_4UA                       0x00  /* 4.05uA */
#define WM8350_CS1_ISEL_5UA                       0x01  /* 4.85uA */
#define WM8350_CS1_ISEL_6UA                       0x02  /* 5.64uA */
#define WM8350_CS1_ISEL_7UA                       0x03  /* 6.83uA */
#define WM8350_CS1_ISEL_8UA                       0x04  /* 8.02uA */
#define WM8350_CS1_ISEL_10UA                      0x05  /* 9.6uA */
#define WM8350_CS1_ISEL_11UA                      0x06  /* 11.2uA */
#define WM8350_CS1_ISEL_13UA                      0x07  /* 13.5uA */
#define WM8350_CS1_ISEL_16UA                      0x08  /* 16.1uA */
#define WM8350_CS1_ISEL_19UA                      0x09  /* 19.3uA */
#define WM8350_CS1_ISEL_22UA                      0x0A  /* 22.4uA */
#define WM8350_CS1_ISEL_27UA                      0x0B  /* 27.2uA */
#define WM8350_CS1_ISEL_32UA                      0x0C  /* 32uA */
#define WM8350_CS1_ISEL_38UA                      0x0D  /* 38.3uA */
#define WM8350_CS1_ISEL_45UA                      0x0E  /* 44.7uA */
#define WM8350_CS1_ISEL_54UA                      0x0F  /* 54.1uA */
#define WM8350_CS1_ISEL_64UA                      0x10  /* 64.1uA */
#define WM8350_CS1_ISEL_77UA                      0x11  /* 76.8uA */
#define WM8350_CS1_ISEL_90UA                      0x12  /* 89.5uA */
#define WM8350_CS1_ISEL_109UA                     0x13  /* 109uA */
#define WM8350_CS1_ISEL_128UA                     0x14  /* 128uA */
#define WM8350_CS1_ISEL_153UA                     0x15  /* 153uA */
#define WM8350_CS1_ISEL_178UA                     0x16  /* 178uA */
#define WM8350_CS1_ISEL_216UA                     0x17  /* 216uA */
#define WM8350_CS1_ISEL_256UA                     0x18  /* 256uA */
#define WM8350_CS1_ISEL_307UA                     0x19  /* 307uA */
#define WM8350_CS1_ISEL_358UA                     0x1A  /* 358uA */
#define WM8350_CS1_ISEL_434UA                     0x1B  /* 434uA */
#define WM8350_CS1_ISEL_510UA                     0x1C  /* 510uA */
#define WM8350_CS1_ISEL_612UA                     0x1D  /* 612uA */
#define WM8350_CS1_ISEL_713UA                     0x1E  /* 713uA */
#define WM8350_CS1_ISEL_865UA                     0x1F  /* 865uA */
#define WM8350_CS1_ISEL_1MA                       0x20  /* 1.02mA */
#define WM8350_CS1_ISEL_1_2MA                     0x21  /* 1.22mA */
#define WM8350_CS1_ISEL_1_4MA                     0x22  /* 1.42mA */
#define WM8350_CS1_ISEL_1_7MA                     0x23  /* 1.73mA */
#define WM8350_CS1_ISEL_2MA                       0x24  /* 2.03mA */
#define WM8350_CS1_ISEL_2_4MA                     0x25  /* 2.43mA */
#define WM8350_CS1_ISEL_2_8MA                     0x26  /* 2.83mA */
#define WM8350_CS1_ISEL_3_4MA                     0x27  /* 3.43mA */
#define WM8350_CS1_ISEL_4MA                       0x28  /* 4.08mA */
#define WM8350_CS1_ISEL_5MA                       0x29  /* 4.89mA */
#define WM8350_CS1_ISEL_6MA                       0x2A  /* 5.7mA */
#define WM8350_CS1_ISEL_7MA                       0x2B  /* 6.91mA */
#define WM8350_CS1_ISEL_8MA                       0x2C  /* 8.13mA */
#define WM8350_CS1_ISEL_10MA                      0x2D  /* 9.74mA */
#define WM8350_CS1_ISEL_11MA                      0x2E  /* 11.3mA */
#define WM8350_CS1_ISEL_14MA                      0x2F  /* 13.7mA */
#define WM8350_CS1_ISEL_16MA                      0x30  /* 16.3mA */
#define WM8350_CS1_ISEL_20MA                      0x31  /* 19.6mA */
#define WM8350_CS1_ISEL_23MA                      0x32  /* 22.8mA */
#define WM8350_CS1_ISEL_28MA                      0x33  /* 27.6mA */
#define WM8350_CS1_ISEL_32MA                      0x34  /* 32.5mA */
#define WM8350_CS1_ISEL_39MA                      0x35  /* 39mA */
#define WM8350_CS1_ISEL_45MA                      0x36  /* 45.4mA */
#define WM8350_CS1_ISEL_55MA                      0x37  /* 54.9mA */
#define WM8350_CS1_ISEL_65MA                      0x38  /* 65.3mA */
#define WM8350_CS1_ISEL_78MA                      0x39  /* 78.2mA */
#define WM8350_CS1_ISEL_91MA                      0x3A  /* 91.2mA */
#define WM8350_CS1_ISEL_111MA                     0x3B  /* 111mA */
#define WM8350_CS1_ISEL_130MA                     0x3C  /* 130mA */
#define WM8350_CS1_ISEL_156MA                     0x3D  /* 156mA */
#define WM8350_CS1_ISEL_181MA                     0x3E  /* 181mA */
#define WM8350_CS1_ISEL_220M                      0x3F  /* 220mA */

/*
 * R173 (0xAD) - CSA Flash control
 */
#define WM8350_CS1_FLASH_MODE                   0x8000  /* CS1_FLASH_MODE */
#define WM8350_CS1_FLASH_MODE_MASK              0x8000  /* CS1_FLASH_MODE */
#define WM8350_CS1_FLASH_MODE_SHIFT                 15  /* CS1_FLASH_MODE */
#define WM8350_CS1_TRIGSRC                      0x4000  /* CS1_TRIGSRC */
#define WM8350_CS1_TRIGSRC_MASK                 0x4000  /* CS1_TRIGSRC */
#define WM8350_CS1_TRIGSRC_SHIFT                    14  /* CS1_TRIGSRC */
#define WM8350_CS1_DRIVE                        0x2000  /* CS1_DRIVE */
#define WM8350_CS1_DRIVE_MASK                   0x2000  /* CS1_DRIVE */
#define WM8350_CS1_DRIVE_SHIFT                      13  /* CS1_DRIVE */
#define WM8350_CS1_FLASH_DUR_MASK               0x0300  /* CS1_FLASH_DUR - [9:8] */
#define WM8350_CS1_FLASH_DUR_SHIFT                   8  /* CS1_FLASH_DUR - [9:8] */
#define WM8350_CS1_OFF_RAMP_MASK                0x0030  /* CS1_OFF_RAMP - [5:4] */
#define WM8350_CS1_OFF_RAMP_SHIFT                    4  /* CS1_OFF_RAMP - [5:4] */
#define WM8350_CS1_ON_RAMP_MASK                 0x0003  /* CS1_ON_RAMP - [1:0] */
#define WM8350_CS1_ON_RAMP_SHIFT                     0  /* CS1_ON_RAMP - [1:0] */

/*
 * R174 (0xAE) - Current Sink Driver B
 */
#define WM8350_CS2_ENA_R174                     0x8000  /* CS2_ENA */
#define WM8350_CS2_HIB_MODE                     0x1000  /* CS2_HIB_MODE */
#define WM8350_CS2_ISEL_MASK                    0x003F  /* CS2_ISEL - [5:0] */

/* Bit values for R174 (0xAE) */
#define WM8350_CS2_HIB_MODE_DISABLE                  0  /* Disable current sink in hibernate */
#define WM8350_CS2_HIB_MODE_LEAVE                    1  /* Leave current sink as-is in hibernate */

#define WM8350_CS2_ISEL_4UA                       0x00  /* 4.05uA */
#define WM8350_CS2_ISEL_5UA                       0x01  /* 4.85uA */
#define WM8350_CS2_ISEL_6UA                       0x02  /* 5.64uA */
#define WM8350_CS2_ISEL_7UA                       0x03  /* 6.83uA */
#define WM8350_CS2_ISEL_8UA                       0x04  /* 8.02uA */
#define WM8350_CS2_ISEL_10UA                      0x05  /* 9.6uA */
#define WM8350_CS2_ISEL_11UA                      0x06  /* 11.2uA */
#define WM8350_CS2_ISEL_13UA                      0x07  /* 13.5uA */
#define WM8350_CS2_ISEL_16UA                      0x08  /* 16.1uA */
#define WM8350_CS2_ISEL_19UA                      0x09  /* 19.3uA */
#define WM8350_CS2_ISEL_22UA                      0x0A  /* 22.4uA */
#define WM8350_CS2_ISEL_27UA                      0x0B  /* 27.2uA */
#define WM8350_CS2_ISEL_32UA                      0x0C  /* 32uA */
#define WM8350_CS2_ISEL_38UA                      0x0D  /* 38.3uA */
#define WM8350_CS2_ISEL_45UA                      0x0E  /* 44.7uA */
#define WM8350_CS2_ISEL_54UA                      0x0F  /* 54.1uA */
#define WM8350_CS2_ISEL_64UA                      0x10  /* 64.1uA */
#define WM8350_CS2_ISEL_77UA                      0x11  /* 76.8uA */
#define WM8350_CS2_ISEL_90UA                      0x12  /* 89.5uA */
#define WM8350_CS2_ISEL_109UA                     0x13  /* 109uA */
#define WM8350_CS2_ISEL_128UA                     0x14  /* 128uA */
#define WM8350_CS2_ISEL_153UA                     0x15  /* 153uA */
#define WM8350_CS2_ISEL_178UA                     0x16  /* 178uA */
#define WM8350_CS2_ISEL_216UA                     0x17  /* 216uA */
#define WM8350_CS2_ISEL_256UA                     0x18  /* 256uA */
#define WM8350_CS2_ISEL_307UA                     0x19  /* 307uA */
#define WM8350_CS2_ISEL_358UA                     0x1A  /* 358uA */
#define WM8350_CS2_ISEL_434UA                     0x1B  /* 434uA */
#define WM8350_CS2_ISEL_510UA                     0x1C  /* 510uA */
#define WM8350_CS2_ISEL_612UA                     0x1D  /* 612uA */
#define WM8350_CS2_ISEL_713UA                     0x1E  /* 713uA */
#define WM8350_CS2_ISEL_865UA                     0x1F  /* 865uA */
#define WM8350_CS2_ISEL_1MA                       0x20  /* 1.02mA */
#define WM8350_CS2_ISEL_1_2MA                     0x21  /* 1.22mA */
#define WM8350_CS2_ISEL_1_4MA                     0x22  /* 1.42mA */
#define WM8350_CS2_ISEL_1_7MA                     0x23  /* 1.73mA */
#define WM8350_CS2_ISEL_2MA                       0x24  /* 2.03mA */
#define WM8350_CS2_ISEL_2_4MA                     0x25  /* 2.43mA */
#define WM8350_CS2_ISEL_2_8MA                     0x26  /* 2.83mA */
#define WM8350_CS2_ISEL_3_4MA                     0x27  /* 3.43mA */
#define WM8350_CS2_ISEL_4MA                       0x28  /* 4.08mA */
#define WM8350_CS2_ISEL_5MA                       0x29  /* 4.89mA */
#define WM8350_CS2_ISEL_6MA                       0x2A  /* 5.7mA */
#define WM8350_CS2_ISEL_7MA                       0x2B  /* 6.91mA */
#define WM8350_CS2_ISEL_8MA                       0x2C  /* 8.13mA */
#define WM8350_CS2_ISEL_10MA                      0x2D  /* 9.74mA */
#define WM8350_CS2_ISEL_11MA                      0x2E  /* 11.3mA */
#define WM8350_CS2_ISEL_14MA                      0x2F  /* 13.7mA */
#define WM8350_CS2_ISEL_16MA                      0x30  /* 16.3mA */
#define WM8350_CS2_ISEL_20MA                      0x31  /* 19.6mA */
#define WM8350_CS2_ISEL_23MA                      0x32  /* 22.8mA */
#define WM8350_CS2_ISEL_28MA                      0x33  /* 27.6mA */
#define WM8350_CS2_ISEL_32MA                      0x34  /* 32.5mA */
#define WM8350_CS2_ISEL_39MA                      0x35  /* 39mA */
#define WM8350_CS2_ISEL_45MA                      0x36  /* 45.4mA */
#define WM8350_CS2_ISEL_55MA                      0x37  /* 54.9mA */
#define WM8350_CS2_ISEL_65MA                      0x38  /* 65.3mA */
#define WM8350_CS2_ISEL_78MA                      0x39  /* 78.2mA */
#define WM8350_CS2_ISEL_91MA                      0x3A  /* 91.2mA */
#define WM8350_CS2_ISEL_111MA                     0x3B  /* 111mA */
#define WM8350_CS2_ISEL_130MA                     0x3C  /* 130mA */
#define WM8350_CS2_ISEL_156MA                     0x3D  /* 156mA */
#define WM8350_CS2_ISEL_181MA                     0x3E  /* 181mA */
#define WM8350_CS2_ISEL_220M                      0x3F  /* 220mA */

/*
 * R175 (0xAF) - CSB Flash control
 */
#define WM8350_CS2_FLASH_MODE                   0x8000  /* CS2_FLASH_MODE */
#define WM8350_CS2_TRIGSRC                      0x4000  /* CS2_TRIGSRC */
#define WM8350_CS2_DRIVE                        0x2000  /* CS2_DRIVE */
#define WM8350_CS2_FLASH_DUR_MASK               0x0300  /* CS2_FLASH_DUR - [9:8] */
#define WM8350_CS2_OFF_RAMP_MASK                0x0030  /* CS2_OFF_RAMP - [5:4] */
#define WM8350_CS2_ON_RAMP_MASK                 0x0003  /* CS2_ON_RAMP - [1:0] */

/*
 * R176 (0xB0) - DCDC/LDO requested
 */
#define WM8350_LS_ENA_R176                      0x8000  /* LS_ENA */
#define WM8350_LDO4_ENA_R176                    0x0800  /* LDO4_ENA */
#define WM8350_LDO3_ENA_R176                    0x0400  /* LDO3_ENA */
#define WM8350_LDO2_ENA_R176                    0x0200  /* LDO2_ENA */
#define WM8350_LDO1_ENA_R176                    0x0100  /* LDO1_ENA */
#define WM8350_DC6_ENA_R176                     0x0020  /* DC6_ENA */
#define WM8350_DC5_ENA_R176                     0x0010  /* DC5_ENA */
#define WM8350_DC4_ENA_R176                     0x0008  /* DC4_ENA */
#define WM8350_DC3_ENA_R176                     0x0004  /* DC3_ENA */
#define WM8350_DC2_ENA_R176                     0x0002  /* DC2_ENA */
#define WM8350_DC1_ENA_R176                     0x0001  /* DC1_ENA */

/*
 * R177 (0xB1) - DCDC Active options
 */
#define WM8350_DCDC_DISCLKS                     0x8000  /* DCDC_DISCLKS */
#define WM8350_PUTO_MASK                        0x3000  /* PUTO - [13:12] */
#define WM8350_PWRUP_DELAY_MASK                 0x0300  /* PWRUP_DELAY - [9:8] */
#define WM8350_DC6_ACTIVE                       0x0020  /* DC6_ACTIVE */
#define WM8350_DC4_ACTIVE                       0x0008  /* DC4_ACTIVE */
#define WM8350_DC3_ACTIVE                       0x0004  /* DC3_ACTIVE */
#define WM8350_DC1_ACTIVE                       0x0001  /* DC1_ACTIVE */

/*
 * R178 (0xB2) - DCDC Sleep options
 */
#define WM8350_DC6_SLEEP                        0x0020  /* DC6_SLEEP */
#define WM8350_DC4_SLEEP                        0x0008  /* DC4_SLEEP */
#define WM8350_DC3_SLEEP                        0x0004  /* DC3_SLEEP */
#define WM8350_DC1_SLEEP                        0x0001  /* DC1_SLEEP */

/*
 * R179 (0xB3) - Power-check comparator
 */
#define WM8350_PCCMP_ERRACT                     0x4000  /* PCCMP_ERRACT */
#define WM8350_PCCOMP_HIB_MODE_R179             0x1000  /* PCCOMP_HIB_MODE */
#define WM8350_PCCMP_OFF_THR_MASK               0x0070  /* PCCMP_OFF_THR - [6:4] */
#define WM8350_PCCMP_ON_THR_MASK                0x0007  /* PCCMP_ON_THR - [2:0] */

/*
 * R180 (0xB4) - DCDC1 Control
 */
#define WM8350_DC1_DISOVP                       0x0800  /* DC1_DISOVP */
#define WM8350_DC1_OPFLT                        0x0400  /* DC1_OPFLT */
#define WM8350_DC1_VSEL_MASK                    0x007F  /* DC1_VSEL - [6:0] */
#define WM8350_DC1_VSEL_SHIFT                        0  /* DC1_VSEL - [6:0] */

/* Bit values for R180 (0xB4) */
#define WM8350_DC1_VSEL( _mV )                  (((_mV)-850)/25)
#define WM8350_DC1_VSEL_V( _val )               (((_val)*25)+850)

/*
 * R181 (0xB5) - DCDC1 Timeouts
 */
#define WM8350_DC1_ERRACT_MASK                  0xC000  /* DC1_ERRACT - [15:14] */
#define WM8350_DC1_ERRACT_SHIFT                     14  /* DC1_ERRACT - [15:14] */
#define WM8350_DC1_ENSLOT_MASK                  0x3C00  /* DC1_ENSLOT - [13:10] */
#define WM8350_DC1_ENSLOT_SHIFT                     10  /* DC1_ENSLOT - [13:10] */
#define WM8350_DC1_SDSLOT_MASK                  0x03C0  /* DC1_SDSLOT - [9:6] */
#define WM8350_DC1_SDSLOT_SHIFT                      6  /* DC1_SDSLOT - [9:6] */

/* Bit values for R181 (0xB5) */
#define WM8350_DC1_ERRACT_NONE                       0
#define WM8350_DC1_ERRACT_SHUTDOWN_CONV              1
#define WM8350_DC1_ERRACT_SHUTDOWN_SYS               2

#define WM8350_DC1_ENSLOT_OFF                        0
#define WM8350_DC1_ENSLOT_1                          1
#define WM8350_DC1_ENSLOT_2                          2
#define WM8350_DC1_ENSLOT_3                          3
#define WM8350_DC1_ENSLOT_4                          4
#define WM8350_DC1_ENSLOT_5                          5
#define WM8350_DC1_ENSLOT_6                          6
#define WM8350_DC1_ENSLOT_7                          7
#define WM8350_DC1_ENSLOT_8                          8
#define WM8350_DC1_ENSLOT_9                          9
#define WM8350_DC1_ENSLOT_10                         10
#define WM8350_DC1_ENSLOT_11                         11
#define WM8350_DC1_ENSLOT_12                         12
#define WM8350_DC1_ENSLOT_13                         13
#define WM8350_DC1_ENSLOT_14                         14
#define WM8350_DC1_ENSLOT_END                        15

#define WM8350_DC1_SDSLOT_OFF                        0
#define WM8350_DC1_SDSLOT_1                          1
#define WM8350_DC1_SDSLOT_2                          2
#define WM8350_DC1_SDSLOT_3                          3
#define WM8350_DC1_SDSLOT_4                          4
#define WM8350_DC1_SDSLOT_5                          5
#define WM8350_DC1_SDSLOT_6                          6
#define WM8350_DC1_SDSLOT_7                          7
#define WM8350_DC1_SDSLOT_8                          8
#define WM8350_DC1_SDSLOT_9                          9
#define WM8350_DC1_SDSLOT_10                         10
#define WM8350_DC1_SDSLOT_11                         11
#define WM8350_DC1_SDSLOT_12                         12
#define WM8350_DC1_SDSLOT_13                         13
#define WM8350_DC1_SDSLOT_14                         14

/*
 * R182 (0xB6) - DCDC1 Low Power
 */
#define WM8350_DC1_HIB_MODE                     0x7000  /* DC1_HIB_MODE */
#define WM8350_DC1_HIB_MODE_MASK                0x7000  /* DC1_HIB_MODE - [14:12] */
#define WM8350_DC1_HIB_MODE_SHIFT                   12  /* DC1_HIB_MODE */
#define WM8350_DC1_HIB_TRIG                     0x0300  /* DC1_HIB_TRIG */
#define WM8350_DC1_HIB_TRIG_MASK                0x0300  /* DC1_HIB_TRIG - [9:8] */
#define WM8350_DC1_HIB_TRIG_SHIFT                    8  /* DC1_HIB_TRIG */
#define WM8350_DC1_VIMG_MASK                    0x007F  /* DC1_VIMG - [6:0] */

/* Bit values for R182 (0xB6) */

#define WM8350_DC1_HIB_MODE_CURRENT                    0
#define WM8350_DC1_HIB_MODE_V_IMAGE                    1
#define WM8350_DC1_HIB_MODE_STANDBY_V_IMAGE            2
#define WM8350_DC1_HIB_MODE_LDO                        4
#define WM8350_DC1_HIB_MODE_LDO_V_IMAGE                5
#define WM8350_DC1_HIB_MODE_DISABLE                    7

#define WM8350_DC1_HIB_TRIG_HIBERNATE                0
#define WM8350_DC1_HIB_TRIG_L_PWR1                    1
#define WM8350_DC1_HIB_TRIG_L_PWR2                    2
#define WM8350_DC1_HIB_TRIG_L_PWR3                    3

/*
 * R183 (0xB7) - DCDC2 Control
 */
#define WM8350_DC2_MODE                         0x4000  /* DC2_MODE */
#define WM8350_DC2_MODE_MASK                    0x4000  /* DC2_MODE */
#define WM8350_DC2_MODE_SHIFT                       14  /* DC2_MODE */
#define WM8350_DC2_HIB_MODE                     0x1000  /* DC2_HIB_MODE */
#define WM8350_DC2_HIB_MODE_MASK                0x1000  /* DC2_HIB_MODE */
#define WM8350_DC2_HIB_MODE_SHIFT                   12  /* DC2_HIB_MODE */
#define WM8350_DC2_HIB_TRIG                     0x0300  /* DC2_HIB_TRIG */
#define WM8350_DC2_HIB_TRIG_MASK                0x0300  /* DC2_HIB_TRIG - [9:8] */
#define WM8350_DC2_HIB_TRIG_SHIFT                    8  /* DC2_HIB_TRIG - [9:8] */
#define WM8350_DC2_ILIM                         0x0040  /* DC2_ILIM */
#define WM8350_DC2_ILIM_MASK                    0x0040  /* DC2_ILIM */
#define WM8350_DC2_ILIM_SHIFT                        6  /* DC2_ILIM */
#define WM8350_DC2_RMP_MASK                     0x0018  /* DC2_RMP - [4:3] */
#define WM8350_DC2_RMP_SHIFT                         3  /* DC2_RMP */
#define WM8350_DC2_FBSRC_MASK                   0x0003  /* DC2_FBSRC - [1:0] */
#define WM8350_DC2_FBSRC_SHIFT                       0  /* DC2_FBSRC - [1:0] */

/* Bit values for R183 (0xB7) */
#define WM8350_DC2_MODE_BOOST                        0
#define WM8350_DC2_MODE_SWITCH                       1

#define WM8350_DC2_HIB_MODE_ACTIVE                   0
#define WM8350_DC2_HIB_MODE_DISABLE                  1

#define WM8350_DC2_HIB_TRIG_NONE                     0
#define WM8350_DC2_HIB_TRIG_LPWR1                    1
#define WM8350_DC2_HIB_TRIG_LPWR2                    2
#define WM8350_DC2_HIB_TRIG_LPWR3                    3

#define WM8350_DC2_ILIM_HIGH                         0
#define WM8350_DC2_ILIM_LOW                          1

#define WM8350_DC2_RMP_30V                           0
#define WM8350_DC2_RMP_20V                           1
#define WM8350_DC2_RMP_10V                           2
#define WM8350_DC2_RMP_5V                            3

#define WM8350_DC2_FBSRC_FB                          0
#define WM8350_DC2_FBSRC_ISINKA                      1
#define WM8350_DC2_FBSRC_ISINKB                      2
#define WM8350_DC2_FBSRC_USB                         3

/*
 * R184 (0xB8) - DCDC2 Timeouts
 */
#define WM8350_DC2_ERRACT_MASK                  0xC000  /* DC2_ERRACT - [15:14] */
#define WM8350_DC2_ERRACT_SHIFT                     14  /* DC2_ERRACT - [15:14] */
#define WM8350_DC2_ENSLOT_MASK                  0x3C00  /* DC2_ENSLOT - [13:10] */
#define WM8350_DC2_ENSLOT_SHIFT                     10  /* DC2_ENSLOT - [13:10] */
#define WM8350_DC2_SDSLOT_MASK                  0x03C0  /* DC2_SDSLOT - [9:6] */
#define WM8350_DC2_SDSLOT_SHIFT                      6  /* DC2_SDSLOT - [9:6] */

/* Bit values for R184 (0xB8) */
#define WM8350_DC2_ERRACT_NONE                       0
#define WM8350_DC2_ERRACT_SHUTDOWN_CONV              1
#define WM8350_DC2_ERRACT_SHUTDOWN_SYS               2

#define WM8350_DC2_ENSLOT_OFF                        0
#define WM8350_DC2_ENSLOT_1                          1
#define WM8350_DC2_ENSLOT_2                          2
#define WM8350_DC2_ENSLOT_3                          3
#define WM8350_DC2_ENSLOT_4                          4
#define WM8350_DC2_ENSLOT_5                          5
#define WM8350_DC2_ENSLOT_6                          6
#define WM8350_DC2_ENSLOT_7                          7
#define WM8350_DC2_ENSLOT_8                          8
#define WM8350_DC2_ENSLOT_9                          9
#define WM8350_DC2_ENSLOT_10                         10
#define WM8350_DC2_ENSLOT_11                         11
#define WM8350_DC2_ENSLOT_12                         12
#define WM8350_DC2_ENSLOT_13                         13
#define WM8350_DC2_ENSLOT_14                         14
#define WM8350_DC2_ENSLOT_END                        15

#define WM8350_DC2_SDSLOT_OFF                        0
#define WM8350_DC2_SDSLOT_1                          1
#define WM8350_DC2_SDSLOT_2                          2
#define WM8350_DC2_SDSLOT_3                          3
#define WM8350_DC2_SDSLOT_4                          4
#define WM8350_DC2_SDSLOT_5                          5
#define WM8350_DC2_SDSLOT_6                          6
#define WM8350_DC2_SDSLOT_7                          7
#define WM8350_DC2_SDSLOT_8                          8
#define WM8350_DC2_SDSLOT_9                          9
#define WM8350_DC2_SDSLOT_10                         10
#define WM8350_DC2_SDSLOT_11                         11
#define WM8350_DC2_SDSLOT_12                         12
#define WM8350_DC2_SDSLOT_13                         13
#define WM8350_DC2_SDSLOT_14                         14

/*
 * R186 (0xBA) - DCDC3 Control
 */
#define WM8350_DC3_DISOVP                       0x0800  /* DC3_DISOVP */
#define WM8350_DC3_OPFLT                        0x0400  /* DC3_OPFLT */
#define WM8350_DC3_VSEL_MASK                    0x007F  /* DC3_VSEL - [6:0] */
#define WM8350_DC3_VSEL_SHIFT                        0  /* DC3_VSEL - [6:0] */

/* Bit values for R186 (0xBA) */
#define WM8350_DC3_VSEL( _mV )                  (((_mV)-850)/25)
#define WM8350_DC3_VSEL_V( _val )               (((_val)*25)+850)

/*
 * R187 (0xBB) - DCDC3 Timeouts
 */
#define WM8350_DC3_ERRACT_MASK                  0xC000  /* DC3_ERRACT - [15:14] */
#define WM8350_DC3_ERRACT_SHIFT                     14  /* DC3_ERRACT - [15:14] */
#define WM8350_DC3_ENSLOT_MASK                  0x3C00  /* DC3_ENSLOT - [13:10] */
#define WM8350_DC3_ENSLOT_SHIFT                     10  /* DC3_ENSLOT - [13:10] */
#define WM8350_DC3_SDSLOT_MASK                  0x03C0  /* DC3_SDSLOT - [9:6] */
#define WM8350_DC3_SDSLOT_SHIFT                      6  /* DC3_SDSLOT - [9:6] */

/* Bit values for R187 (0xBB) */
#define WM8350_DC3_ERRACT_NONE                       0
#define WM8350_DC3_ERRACT_SHUTDOWN_CONV              1
#define WM8350_DC3_ERRACT_SHUTDOWN_SYS               2

#define WM8350_DC3_ENSLOT_OFF                        0
#define WM8350_DC3_ENSLOT_1                          1
#define WM8350_DC3_ENSLOT_2                          2
#define WM8350_DC3_ENSLOT_3                          3
#define WM8350_DC3_ENSLOT_4                          4
#define WM8350_DC3_ENSLOT_5                          5
#define WM8350_DC3_ENSLOT_6                          6
#define WM8350_DC3_ENSLOT_7                          7
#define WM8350_DC3_ENSLOT_8                          8
#define WM8350_DC3_ENSLOT_9                          9
#define WM8350_DC3_ENSLOT_10                         10
#define WM8350_DC3_ENSLOT_11                         11
#define WM8350_DC3_ENSLOT_12                         12
#define WM8350_DC3_ENSLOT_13                         13
#define WM8350_DC3_ENSLOT_14                         14
#define WM8350_DC3_ENSLOT_END                        15

#define WM8350_DC3_SDSLOT_OFF                        0
#define WM8350_DC3_SDSLOT_1                          1
#define WM8350_DC3_SDSLOT_2                          2
#define WM8350_DC3_SDSLOT_3                          3
#define WM8350_DC3_SDSLOT_4                          4
#define WM8350_DC3_SDSLOT_5                          5
#define WM8350_DC3_SDSLOT_6                          6
#define WM8350_DC3_SDSLOT_7                          7
#define WM8350_DC3_SDSLOT_8                          8
#define WM8350_DC3_SDSLOT_9                          9
#define WM8350_DC3_SDSLOT_10                         10
#define WM8350_DC3_SDSLOT_11                         11
#define WM8350_DC3_SDSLOT_12                         12
#define WM8350_DC3_SDSLOT_13                         13
#define WM8350_DC3_SDSLOT_14                         14

/*
* R188 (0xBC) - DCDC3 Low Power
*/
#define WM8350_DC3_HIB_MODE                     0x7000  /* DC3_HIB_MODE */
#define WM8350_DC3_HIB_MODE_MASK                0x7000  /* DC3_HIB_MODE - [14:12] */
#define WM8350_DC3_HIB_MODE_SHIFT                   12  /* DC3_HIB_MODE */
#define WM8350_DC3_HIB_TRIG                     0x0300  /* DC3_HIB_TRIG */
#define WM8350_DC3_HIB_TRIG_MASK                0x0300  /* DC3_HIB_TRIG - [9:8] */
#define WM8350_DC3_HIB_TRIG_SHIFT                    8  /* DC3_HIB_TRIG */
#define WM8350_DC3_VIMG_MASK                    0x007F  /* DC3_VIMG - [6:0] */

/* Bit values for R188 (0xBC) */

#define WM8350_DC3_HIB_MODE_CURRENT                    0
#define WM8350_DC3_HIB_MODE_V_IMAGE                    1
#define WM8350_DC3_HIB_MODE_STANDBY_V_IMAGE            2
#define WM8350_DC3_HIB_MODE_LDO                        4
#define WM8350_DC3_HIB_MODE_LDO_V_IMAGE                5
#define WM8350_DC3_HIB_MODE_DISABLE                    7

#define WM8350_DC3_HIB_TRIG_HIBERNATE                0
#define WM8350_DC3_HIB_TRIG_L_PWR1                    1
#define WM8350_DC3_HIB_TRIG_L_PWR2                    2
#define WM8350_DC3_HIB_TRIG_L_PWR3                    3

/*
 * R189 (0xBD) - DCDC4 Control
 */
#define WM8350_DC4_DISOVP                       0x0800  /* DC4_DISOVP */
#define WM8350_DC4_OPFLT                        0x0400  /* DC4_OPFLT */
#define WM8350_DC4_VSEL_MASK                    0x007F  /* DC4_VSEL - [6:0] */
#define WM8350_DC4_VSEL_SHIFT                        0  /* DC4_VSEL - [6:0] */

/* Bit values for R189 (0xBD) */
#define WM8350_DC4_VSEL( _mV )                  (((_mV)-850)/25)
#define WM8350_DC4_VSEL_V( _val )               (((_val)*25)+850)

/*
 * R190 (0xBE) - DCDC4 Timeouts
 */
#define WM8350_DC4_ERRACT_MASK                  0xC000  /* DC4_ERRACT - [15:14] */
#define WM8350_DC4_ERRACT_SHIFT                     14  /* DC4_ERRACT - [15:14] */
#define WM8350_DC4_ENSLOT_MASK                  0x3C00  /* DC4_ENSLOT - [13:10] */
#define WM8350_DC4_ENSLOT_SHIFT                     10  /* DC4_ENSLOT - [13:10] */
#define WM8350_DC4_SDSLOT_MASK                  0x03C0  /* DC4_SDSLOT - [9:6] */
#define WM8350_DC4_SDSLOT_SHIFT                      6  /* DC4_SDSLOT - [9:6] */

/* Bit values for R190 (0xBE) */
#define WM8350_DC4_ERRACT_NONE                       0
#define WM8350_DC4_ERRACT_SHUTDOWN_CONV              1
#define WM8350_DC4_ERRACT_SHUTDOWN_SYS               2

#define WM8350_DC4_ENSLOT_OFF                        0
#define WM8350_DC4_ENSLOT_1                          1
#define WM8350_DC4_ENSLOT_2                          2
#define WM8350_DC4_ENSLOT_3                          3
#define WM8350_DC4_ENSLOT_4                          4
#define WM8350_DC4_ENSLOT_5                          5
#define WM8350_DC4_ENSLOT_6                          6
#define WM8350_DC4_ENSLOT_7                          7
#define WM8350_DC4_ENSLOT_8                          8
#define WM8350_DC4_ENSLOT_9                          9
#define WM8350_DC4_ENSLOT_10                         10
#define WM8350_DC4_ENSLOT_11                         11
#define WM8350_DC4_ENSLOT_12                         12
#define WM8350_DC4_ENSLOT_13                         13
#define WM8350_DC4_ENSLOT_14                         14
#define WM8350_DC4_ENSLOT_END                        15

#define WM8350_DC4_SDSLOT_OFF                        0
#define WM8350_DC4_SDSLOT_1                          1
#define WM8350_DC4_SDSLOT_2                          2
#define WM8350_DC4_SDSLOT_3                          3
#define WM8350_DC4_SDSLOT_4                          4
#define WM8350_DC4_SDSLOT_5                          5
#define WM8350_DC4_SDSLOT_6                          6
#define WM8350_DC4_SDSLOT_7                          7
#define WM8350_DC4_SDSLOT_8                          8
#define WM8350_DC4_SDSLOT_9                          9
#define WM8350_DC4_SDSLOT_10                         10
#define WM8350_DC4_SDSLOT_11                         11
#define WM8350_DC4_SDSLOT_12                         12
#define WM8350_DC4_SDSLOT_13                         13
#define WM8350_DC4_SDSLOT_14                         14

/*
 * R191 (0xBF) - DCDC4 Low Power
 */
#define WM8350_DC4_HIB_MODE                     0x7000  /* DC4_HIB_MODE */
#define WM8350_DC4_HIB_MODE_MASK                0x7000  /* DC4_HIB_MODE - [14:12] */
#define WM8350_DC4_HIB_MODE_SHIFT                   12  /* DC4_HIB_MODE */
#define WM8350_DC4_HIB_TRIG                     0x0300  /* DC4_HIB_TRIG */
#define WM8350_DC4_HIB_TRIG_MASK                0x0300  /* DC4_HIB_TRIG - [9:8] */
#define WM8350_DC4_HIB_TRIG_SHIFT                    8  /* DC4_HIB_TRIG */
#define WM8350_DC4_VIMG_MASK                    0x007F  /* DC4_VIMG - [6:0] */

/* Bit values for R191 (0xBF) */

#define WM8350_DC4_HIB_MODE_CURRENT                    0
#define WM8350_DC4_HIB_MODE_V_IMAGE                    1
#define WM8350_DC4_HIB_MODE_STANDBY_V_IMAGE            2
#define WM8350_DC4_HIB_MODE_LDO                        4
#define WM8350_DC4_HIB_MODE_LDO_V_IMAGE                5
#define WM8350_DC4_HIB_MODE_DISABLE                    7

#define WM8350_DC4_HIB_TRIG_HIBERNATE                0
#define WM8350_DC4_HIB_TRIG_L_PWR1                    1
#define WM8350_DC4_HIB_TRIG_L_PWR2                    2
#define WM8350_DC4_HIB_TRIG_L_PWR3                    3
/*
 * R192 (0xC0) - DCDC5 Control
 */
#define WM8350_DC5_MODE                         0x4000  /* DC5_MODE */
#define WM8350_DC5_MODE_MASK                    0x4000  /* DC5_MODE */
#define WM8350_DC5_MODE_SHIFT                       14  /* DC5_MODE */
#define WM8350_DC5_HIB_MODE                     0x1000  /* DC5_HIB_MODE */
#define WM8350_DC5_HIB_MODE_MASK                0x1000  /* DC5_HIB_MODE */
#define WM8350_DC5_HIB_MODE_SHIFT                   12  /* DC5_HIB_MODE */
#define WM8350_DC5_HIB_TRIG_MASK                0x0300  /* DC5_HIB_TRIG - [9:8] */
#define WM8350_DC5_HIB_TRIG_SHIFT                    8  /* DC5_HIB_TRIG - [9:8] */
#define WM8350_DC5_ILIM                         0x0040  /* DC5_ILIM */
#define WM8350_DC5_ILIM_MASK                    0x0040  /* DC5_ILIM */
#define WM8350_DC5_ILIM_SHIFT                        6  /* DC5_ILIM */
#define WM8350_DC5_RMP_MASK                     0x0018  /* DC5_RMP - [4:3] */
#define WM8350_DC5_RMP_SHIFT                         3  /* DC5_RMP */
#define WM8350_DC5_FBSRC_MASK                   0x0003  /* DC5_FBSRC - [1:0] */
#define WM8350_DC5_FBSRC_SHIFT                       0  /* DC5_FBSRC - [1:0] */

/* Bit values for R192 (0xC0) */
#define WM8350_DC5_MODE_BOOST                        0
#define WM8350_DC5_MODE_SWITCH                       1

#define WM8350_DC5_HIB_MODE_ACTIVE                   0
#define WM8350_DC5_HIB_MODE_DISABLE                  1

#define WM8350_DC5_HIB_TRIG_NONE                     0
#define WM8350_DC5_HIB_TRIG_LPWR1                    1
#define WM8350_DC5_HIB_TRIG_LPWR2                    2
#define WM8350_DC5_HIB_TRIG_LPWR3                    3

#define WM8350_DC5_ILIM_HIGH                         0
#define WM8350_DC5_ILIM_LOW                          1

#define WM8350_DC5_RMP_30V                           0
#define WM8350_DC5_RMP_20V                           1
#define WM8350_DC5_RMP_10V                           2
#define WM8350_DC5_RMP_5V                            3

#define WM8350_DC5_FBSRC_FB                          0
#define WM8350_DC5_FBSRC_ISINKA                      1
#define WM8350_DC5_FBSRC_ISINKB                      2
#define WM8350_DC5_FBSRC_USB                         3


/*
 * R193 (0xC1) - DCDC5 Timeouts
 */
#define WM8350_DC5_ERRACT_MASK                  0xC000  /* DC5_ERRACT - [15:14] */
#define WM8350_DC5_ERRACT_SHIFT                     14  /* DC5_ERRACT - [15:14] */
#define WM8350_DC5_ENSLOT_MASK                  0x3C00  /* DC5_ENSLOT - [13:10] */
#define WM8350_DC5_ENSLOT_SHIFT                     10  /* DC5_ENSLOT - [13:10] */
#define WM8350_DC5_SDSLOT_MASK                  0x03C0  /* DC5_SDSLOT - [9:6] */
#define WM8350_DC5_SDSLOT_SHIFT                      6  /* DC5_SDSLOT - [9:6] */

/* Bit values for R193 (0xC1) */
#define WM8350_DC5_ERRACT_NONE                       0
#define WM8350_DC5_ERRACT_SHUTDOWN_CONV              1
#define WM8350_DC5_ERRACT_SHUTDOWN_SYS               2

#define WM8350_DC5_ENSLOT_OFF                        0
#define WM8350_DC5_ENSLOT_1                          1
#define WM8350_DC5_ENSLOT_2                          2
#define WM8350_DC5_ENSLOT_3                          3
#define WM8350_DC5_ENSLOT_4                          4
#define WM8350_DC5_ENSLOT_5                          5
#define WM8350_DC5_ENSLOT_6                          6
#define WM8350_DC5_ENSLOT_7                          7
#define WM8350_DC5_ENSLOT_8                          8
#define WM8350_DC5_ENSLOT_9                          9
#define WM8350_DC5_ENSLOT_10                         10
#define WM8350_DC5_ENSLOT_11                         11
#define WM8350_DC5_ENSLOT_12                         12
#define WM8350_DC5_ENSLOT_13                         13
#define WM8350_DC5_ENSLOT_14                         14
#define WM8350_DC5_ENSLOT_END                        15

#define WM8350_DC5_SDSLOT_OFF                        0
#define WM8350_DC5_SDSLOT_1                          1
#define WM8350_DC5_SDSLOT_2                          2
#define WM8350_DC5_SDSLOT_3                          3
#define WM8350_DC5_SDSLOT_4                          4
#define WM8350_DC5_SDSLOT_5                          5
#define WM8350_DC5_SDSLOT_6                          6
#define WM8350_DC5_SDSLOT_7                          7
#define WM8350_DC5_SDSLOT_8                          8
#define WM8350_DC5_SDSLOT_9                          9
#define WM8350_DC5_SDSLOT_10                         10
#define WM8350_DC5_SDSLOT_11                         11
#define WM8350_DC5_SDSLOT_12                         12
#define WM8350_DC5_SDSLOT_13                         13
#define WM8350_DC5_SDSLOT_14                         14

/*
 * R195 (0xC3) - DCDC6 Control
 */
#define WM8350_DC6_DISOVP                       0x0800  /* DC6_DISOVP */
#define WM8350_DC6_OPFLT                        0x0400  /* DC6_OPFLT */
#define WM8350_DC6_VSEL_MASK                    0x007F  /* DC6_VSEL - [6:0] */
#define WM8350_DC6_VSEL_SHIFT                        0  /* DC6_VSEL - [6:0] */

/* Bit values for R195 (0xC3) */
#define WM8350_DC6_VSEL( _mV )                  (((_mV)-850)/25)
#define WM8350_DC6_VSEL_V( _val )               (((_val)*25)+850)

/*
 * R196 (0xC4) - DCDC6 Timeouts
 */
#define WM8350_DC6_ERRACT_MASK                  0xC000  /* DC6_ERRACT - [15:14] */
#define WM8350_DC6_ERRACT_SHIFT                     14  /* DC6_ERRACT - [15:14] */
#define WM8350_DC6_ENSLOT_MASK                  0x3C00  /* DC6_ENSLOT - [13:10] */
#define WM8350_DC6_ENSLOT_SHIFT                     10  /* DC6_ENSLOT - [13:10] */
#define WM8350_DC6_SDSLOT_MASK                  0x03C0  /* DC6_SDSLOT - [9:6] */
#define WM8350_DC6_SDSLOT_SHIFT                      6  /* DC6_SDSLOT - [9:6] */

/* Bit values for R196 (0xC4) */
#define WM8350_DC6_ERRACT_NONE                       0
#define WM8350_DC6_ERRACT_SHUTDOWN_CONV              1
#define WM8350_DC6_ERRACT_SHUTDOWN_SYS               2

#define WM8350_DC6_ENSLOT_OFF                        0
#define WM8350_DC6_ENSLOT_1                          1
#define WM8350_DC6_ENSLOT_2                          2
#define WM8350_DC6_ENSLOT_3                          3
#define WM8350_DC6_ENSLOT_4                          4
#define WM8350_DC6_ENSLOT_5                          5
#define WM8350_DC6_ENSLOT_6                          6
#define WM8350_DC6_ENSLOT_7                          7
#define WM8350_DC6_ENSLOT_8                          8
#define WM8350_DC6_ENSLOT_9                          9
#define WM8350_DC6_ENSLOT_10                         10
#define WM8350_DC6_ENSLOT_11                         11
#define WM8350_DC6_ENSLOT_12                         12
#define WM8350_DC6_ENSLOT_13                         13
#define WM8350_DC6_ENSLOT_14                         14
#define WM8350_DC6_ENSLOT_END                        15

#define WM8350_DC6_SDSLOT_OFF                        0
#define WM8350_DC6_SDSLOT_1                          1
#define WM8350_DC6_SDSLOT_2                          2
#define WM8350_DC6_SDSLOT_3                          3
#define WM8350_DC6_SDSLOT_4                          4
#define WM8350_DC6_SDSLOT_5                          5
#define WM8350_DC6_SDSLOT_6                          6
#define WM8350_DC6_SDSLOT_7                          7
#define WM8350_DC6_SDSLOT_8                          8
#define WM8350_DC6_SDSLOT_9                          9
#define WM8350_DC6_SDSLOT_10                         10
#define WM8350_DC6_SDSLOT_11                         11
#define WM8350_DC6_SDSLOT_12                         12
#define WM8350_DC6_SDSLOT_13                         13
#define WM8350_DC6_SDSLOT_14                         14

/*
 * R197 (0xC5) - DCDC6 Low Power
 */
#define WM8350_DC6_HIB_MODE                     0x7000  /* DC6_HIB_MODE */
#define WM8350_DC6_HIB_MODE_MASK                0x7000  /* DC6_HIB_MODE - [14:12] */
#define WM8350_DC6_HIB_MODE_SHIFT                   12  /* DC6_HIB_MODE */
#define WM8350_DC6_HIB_TRIG                     0x0300  /* DC6_HIB_TRIG */
#define WM8350_DC6_HIB_TRIG_MASK                0x0300  /* DC6_HIB_TRIG - [9:8] */
#define WM8350_DC6_HIB_TRIG_SHIFT                    8  /* DC6_HIB_TRIG */
#define WM8350_DC6_VIMG_MASK                    0x007F  /* DC6_VIMG - [6:0] */

/* Bit values for R197 (0xC5) */

#define WM8350_DC6_HIB_MODE_CURRENT                    0
#define WM8350_DC6_HIB_MODE_V_IMAGE                    1
#define WM8350_DC6_HIB_MODE_STANDBY_V_IMAGE            2
#define WM8350_DC6_HIB_MODE_LDO                        4
#define WM8350_DC6_HIB_MODE_LDO_V_IMAGE                5
#define WM8350_DC6_HIB_MODE_DISABLE                    7

#define WM8350_DC6_HIB_TRIG_HIBERNATE                0
#define WM8350_DC6_HIB_TRIG_L_PWR1                    1
#define WM8350_DC6_HIB_TRIG_L_PWR2                    2
#define WM8350_DC6_HIB_TRIG_L_PWR3                    3

/*
 * R199 (0xC7) - Limit Switch Control
 */
#define WM8350_LS_ERRACT_MASK                   0xC000  /* LS_ERRACT - [15:14] */
#define WM8350_LS_ERRACT_SHIFT                      14  /* LS_ERRACT - [15:14] */
#define WM8350_LS_ENSLOT_MASK                   0x3C00  /* LS_ENSLOT - [13:10] */
#define WM8350_LS_ENSLOT_SHIFT                      10  /* LS_ENSLOT - [13:10] */
#define WM8350_LS_SDSLOT_MASK                   0x03C0  /* LS_SDSLOT - [9:6] */
#define WM8350_LS_SDSLOT_SHIFT                       6  /* LS_SDSLOT - [9:6] */
#define WM8350_LS_HIB_MODE                      0x0010  /* LS_HIB_MODE */
#define WM8350_LS_HIB_MODE_MASK                 0x0010  /* LS_HIB_MODE */
#define WM8350_LS_HIB_MODE_SHIFT                     4  /* LS_HIB_MODE */
#define WM8350_LS_HIB_PROT                      0x0002  /* LS_HIB_PROT */
#define WM8350_LS_HIB_PROT_MASK                 0x0002  /* LS_HIB_PROT */
#define WM8350_LS_HIB_PROT_SHIFT                     1  /* LS_HIB_PROT */
#define WM8350_LS_PROT                          0x0001  /* LS_PROT */
#define WM8350_LS_PROT_MASK                     0x0001  /* LS_PROT */
#define WM8350_LS_PROT_SHIFT                         0  /* LS_PROT */

/* Bit values for R199 (0xC7) */
#define WM8350_LS_ERRACT_NONE                       0
#define WM8350_LS_ERRACT_SHUTDOWN_CONV              1
#define WM8350_LS_ERRACT_SHUTDOWN_SYS               2

#define WM8350_LS_ENSLOT_OFF                        0
#define WM8350_LS_ENSLOT_1                          1
#define WM8350_LS_ENSLOT_2                          2
#define WM8350_LS_ENSLOT_3                          3
#define WM8350_LS_ENSLOT_4                          4
#define WM8350_LS_ENSLOT_5                          5
#define WM8350_LS_ENSLOT_6                          6
#define WM8350_LS_ENSLOT_7                          7
#define WM8350_LS_ENSLOT_8                          8
#define WM8350_LS_ENSLOT_9                          9
#define WM8350_LS_ENSLOT_10                         10
#define WM8350_LS_ENSLOT_11                         11
#define WM8350_LS_ENSLOT_12                         12
#define WM8350_LS_ENSLOT_13                         13
#define WM8350_LS_ENSLOT_14                         14
#define WM8350_LS_ENSLOT_END                        15

#define WM8350_LS_SDSLOT_OFF                        0
#define WM8350_LS_SDSLOT_1                          1
#define WM8350_LS_SDSLOT_2                          2
#define WM8350_LS_SDSLOT_3                          3
#define WM8350_LS_SDSLOT_4                          4
#define WM8350_LS_SDSLOT_5                          5
#define WM8350_LS_SDSLOT_6                          6
#define WM8350_LS_SDSLOT_7                          7
#define WM8350_LS_SDSLOT_8                          8
#define WM8350_LS_SDSLOT_9                          9
#define WM8350_LS_SDSLOT_10                         10
#define WM8350_LS_SDSLOT_11                         11
#define WM8350_LS_SDSLOT_12                         12
#define WM8350_LS_SDSLOT_13                         13
#define WM8350_LS_SDSLOT_14                         14

#define WM8350_LS_HIB_MODE_DISABLE                   0
#define WM8350_LS_HIB_MODE_ACTIVE                    1

#define WM8350_LS_HIB_PROT_DISABLE                   0
#define WM8350_LS_HIB_PROT_ENABLE                    1

#define WM8350_LS_PROT_DISABLE                       0
#define WM8350_LS_PROT_ENABLE                        1

/*
 * R200 (0xC8) - LDO1 Control
 */
#define WM8350_LDO1_SWI                         0x4000  /* LDO1_SWI */
#define WM8350_LDO1_OPFLT                       0x0400  /* LDO1_OPFLT */
#define WM8350_LDO1_VSEL_MASK                   0x001F  /* LDO1_VSEL - [4:0] */
#define WM8350_LDO1_VSEL_SHIFT                       0  /* LDO1_VSEL - [4:0] */

/* Bit values for R200 (0xC8) */
#define WM8350_LDO1_VSEL( _mV )                 ((WM_REGVAL)( ((_mV)<1800) ? (((_mV)-900)/50)  : (((_mV)-1800)/100 + 16) ))
#define WM8350_LDO1_VSEL_V( _val )              ( ((_val)<16)  ? (((_val)*50)+900) : ((((_val)-16)*100)+1800) )

/*
 * R201 (0xC9) - LDO1 Timeouts
 */
#define WM8350_LDO1_ERRACT_MASK                 0xC000  /* LDO1_ERRACT - [15:14] */
#define WM8350_LDO1_ERRACT_SHIFT                    14  /* LDO1_ERRACT - [15:14] */
#define WM8350_LDO1_ENSLOT_MASK                 0x3C00  /* LDO1_ENSLOT - [13:10] */
#define WM8350_LDO1_ENSLOT_SHIFT                    10  /* LDO1_ENSLOT - [13:10] */
#define WM8350_LDO1_SDSLOT_MASK                 0x03C0  /* LDO1_SDSLOT - [9:6] */
#define WM8350_LDO1_SDSLOT_SHIFT                     6  /* LDO1_SDSLOT - [9:6] */

/* Bit values for R201 (0xC9) */
#define WM8350_LDO1_ERRACT_NONE                       0
#define WM8350_LDO1_ERRACT_SHUTDOWN_CONV              1
#define WM8350_LDO1_ERRACT_SHUTDOWN_SYS               2

#define WM8350_LDO1_ENSLOT_OFF                        0
#define WM8350_LDO1_ENSLOT_1                          1
#define WM8350_LDO1_ENSLOT_2                          2
#define WM8350_LDO1_ENSLOT_3                          3
#define WM8350_LDO1_ENSLOT_4                          4
#define WM8350_LDO1_ENSLOT_5                          5
#define WM8350_LDO1_ENSLOT_6                          6
#define WM8350_LDO1_ENSLOT_7                          7
#define WM8350_LDO1_ENSLOT_8                          8
#define WM8350_LDO1_ENSLOT_9                          9
#define WM8350_LDO1_ENSLOT_10                         10
#define WM8350_LDO1_ENSLOT_11                         11
#define WM8350_LDO1_ENSLOT_12                         12
#define WM8350_LDO1_ENSLOT_13                         13
#define WM8350_LDO1_ENSLOT_14                         14
#define WM8350_LDO1_ENSLOT_END                        15

#define WM8350_LDO1_SDSLOT_OFF                        0
#define WM8350_LDO1_SDSLOT_1                          1
#define WM8350_LDO1_SDSLOT_2                          2
#define WM8350_LDO1_SDSLOT_3                          3
#define WM8350_LDO1_SDSLOT_4                          4
#define WM8350_LDO1_SDSLOT_5                          5
#define WM8350_LDO1_SDSLOT_6                          6
#define WM8350_LDO1_SDSLOT_7                          7
#define WM8350_LDO1_SDSLOT_8                          8
#define WM8350_LDO1_SDSLOT_9                          9
#define WM8350_LDO1_SDSLOT_10                         10
#define WM8350_LDO1_SDSLOT_11                         11
#define WM8350_LDO1_SDSLOT_12                         12
#define WM8350_LDO1_SDSLOT_13                         13
#define WM8350_LDO1_SDSLOT_14                         14

/*
 * R202 (0xCA) - LDO1 Low Power
 */
#define WM8350_LDO1_HIB_MODE                    0x3000  /* LDO1_HIB_MODE */
#define WM8350_LDO1_HIB_MODE_MASK               0x3000  /* LDO1_HIB_MODE - [14:12] */
#define WM8350_LDO1_HIB_MODE_SHIFT                  12  /* LDO1_HIB_MODE */
#define WM8350_LDO1_HIB_TRIG                    0x0300  /* LDO1_HIB_TRIG */
#define WM8350_LDO1_HIB_TRIG_MASK               0x0300  /* LDO1_HIB_TRIG - [9:8] */
#define WM8350_LDO1_HIB_TRIG_SHIFT                   8  /* LDO1_HIB_TRIG */
#define WM8350_LDO1_VIMG_MASK                   0x001F  /* LDO1_VIMG - [6:0] */

/* Bit values for R202 (0xCA) */

#define WM8350_LDO1_HIB_MODE_V_IMAGE                 0
#define WM8350_LDO1_HIB_MODE_DISABLE                 1

#define WM8350_LDO1_HIB_TRIG_HIBERNATE               0
#define WM8350_LDO1_HIB_TRIG_L_PWR1                  1
#define WM8350_LDO1_HIB_TRIG_L_PWR2                  2
#define WM8350_LDO1_HIB_TRIG_L_PWR3                  3
/*
 * R203 (0xCB) - LDO2 Control
 */
#define WM8350_LDO2_SWI                         0x4000  /* LDO2_SWI */
#define WM8350_LDO2_OPFLT                       0x0400  /* LDO2_OPFLT */
#define WM8350_LDO2_VSEL_MASK                   0x001F  /* LDO2_VSEL - [4:0] */
#define WM8350_LDO2_VSEL_SHIFT                       0  /* LDO2_VSEL - [4:0] */

/* Bit values for R203 (0xCB) */
#define WM8350_LDO2_VSEL( _mV )                 ((WM_REGVAL)( ((_mV)<1800) ? (((_mV)-900)/50)  : (((_mV)-1800)/100 + 16) ))
#define WM8350_LDO2_VSEL_V( _val )              ( ((_val)<16)  ? (((_val)*50)+900) : ((((_val)-16)*100)+1800) )

/*
 * R204 (0xCC) - LDO2 Timeouts
 */
#define WM8350_LDO2_ERRACT_MASK                 0xC000  /* LDO2_ERRACT - [15:14] */
#define WM8350_LDO2_ERRACT_SHIFT                    14  /* LDO2_ERRACT - [15:14] */
#define WM8350_LDO2_ENSLOT_MASK                 0x3C00  /* LDO2_ENSLOT - [13:10] */
#define WM8350_LDO2_ENSLOT_SHIFT                    10  /* LDO2_ENSLOT - [13:10] */
#define WM8350_LDO2_SDSLOT_MASK                 0x03C0  /* LDO2_SDSLOT - [9:6] */
#define WM8350_LDO2_SDSLOT_SHIFT                     6  /* LDO2_SDSLOT - [9:6] */

/* Bit values for R204 (0xCC) */
#define WM8350_LDO2_ERRACT_NONE                       0
#define WM8350_LDO2_ERRACT_SHUTDOWN_CONV              1
#define WM8350_LDO2_ERRACT_SHUTDOWN_SYS               2

#define WM8350_LDO2_ENSLOT_OFF                        0
#define WM8350_LDO2_ENSLOT_1                          1
#define WM8350_LDO2_ENSLOT_2                          2
#define WM8350_LDO2_ENSLOT_3                          3
#define WM8350_LDO2_ENSLOT_4                          4
#define WM8350_LDO2_ENSLOT_5                          5
#define WM8350_LDO2_ENSLOT_6                          6
#define WM8350_LDO2_ENSLOT_7                          7
#define WM8350_LDO2_ENSLOT_8                          8
#define WM8350_LDO2_ENSLOT_9                          9
#define WM8350_LDO2_ENSLOT_10                         10
#define WM8350_LDO2_ENSLOT_11                         11
#define WM8350_LDO2_ENSLOT_12                         12
#define WM8350_LDO2_ENSLOT_13                         13
#define WM8350_LDO2_ENSLOT_14                         14
#define WM8350_LDO2_ENSLOT_END                        15

#define WM8350_LDO2_SDSLOT_OFF                        0
#define WM8350_LDO2_SDSLOT_1                          1
#define WM8350_LDO2_SDSLOT_2                          2
#define WM8350_LDO2_SDSLOT_3                          3
#define WM8350_LDO2_SDSLOT_4                          4
#define WM8350_LDO2_SDSLOT_5                          5
#define WM8350_LDO2_SDSLOT_6                          6
#define WM8350_LDO2_SDSLOT_7                          7
#define WM8350_LDO2_SDSLOT_8                          8
#define WM8350_LDO2_SDSLOT_9                          9
#define WM8350_LDO2_SDSLOT_10                         10
#define WM8350_LDO2_SDSLOT_11                         11
#define WM8350_LDO2_SDSLOT_12                         12
#define WM8350_LDO2_SDSLOT_13                         13
#define WM8350_LDO2_SDSLOT_14                         14
/*
 * R205 (0xCD) - LDO2 Low Power
 */
#define WM8350_LDO2_HIB_MODE                    0x3000  /* LDO2_HIB_MODE */
#define WM8350_LDO2_HIB_MODE_MASK               0x3000  /* LDO2_HIB_MODE - [14:12] */
#define WM8350_LDO2_HIB_MODE_SHIFT                  12  /* LDO2_HIB_MODE */
#define WM8350_LDO2_HIB_TRIG                    0x0300  /* LDO2_HIB_TRIG */
#define WM8350_LDO2_HIB_TRIG_MASK               0x0300  /* LDO2_HIB_TRIG - [9:8] */
#define WM8350_LDO2_HIB_TRIG_SHIFT                   8  /* LDO2_HIB_TRIG */
#define WM8350_LDO2_VIMG_MASK                   0x001F  /* LDO2_VIMG - [6:0] */

/* Bit values for R205 (0xCD) */

#define WM8350_LDO2_HIB_MODE_V_IMAGE                 0
#define WM8350_LDO2_HIB_MODE_DISABLE                 1

#define WM8350_LDO2_HIB_TRIG_HIBERNATE               0
#define WM8350_LDO2_HIB_TRIG_L_PWR1                  1
#define WM8350_LDO2_HIB_TRIG_L_PWR2                  2
#define WM8350_LDO2_HIB_TRIG_L_PWR3                  3

/*
 * R206 (0xCE) - LDO3 Control
 */
#define WM8350_LDO3_SWI                         0x4000  /* LDO3_SWI */
#define WM8350_LDO3_OPFLT                       0x0400  /* LDO3_OPFLT */
#define WM8350_LDO3_VSEL_MASK                   0x001F  /* LDO3_VSEL - [4:0] */
#define WM8350_LDO3_VSEL_SHIFT                       0  /* LDO3_VSEL - [4:0] */

/* Bit values for R206 (0xCE) */
#define WM8350_LDO3_VSEL( _mV )                 ((WM_REGVAL)( ((_mV)<1800) ? (((_mV)-900)/50)  : (((_mV)-1800)/100 + 16) ))
#define WM8350_LDO3_VSEL_V( _val )              ( ((_val)<16)  ? (((_val)*50)+900) : ((((_val)-16)*100)+1800) )

/*
 * R207 (0xCF) - LDO3 Timeouts
 */
#define WM8350_LDO3_ERRACT_MASK                 0xC000  /* LDO3_ERRACT - [15:14] */
#define WM8350_LDO3_ERRACT_SHIFT                    14  /* LDO3_ERRACT - [15:14] */
#define WM8350_LDO3_ENSLOT_MASK                 0x3C00  /* LDO3_ENSLOT - [13:10] */
#define WM8350_LDO3_ENSLOT_SHIFT                    10  /* LDO3_ENSLOT - [13:10] */
#define WM8350_LDO3_SDSLOT_MASK                 0x03C0  /* LDO3_SDSLOT - [9:6] */
#define WM8350_LDO3_SDSLOT_SHIFT                     6  /* LDO3_SDSLOT - [9:6] */

/* Bit values for R207 (0xCF) */
#define WM8350_LDO3_ERRACT_NONE                       0
#define WM8350_LDO3_ERRACT_SHUTDOWN_CONV              1
#define WM8350_LDO3_ERRACT_SHUTDOWN_SYS               2

#define WM8350_LDO3_ENSLOT_OFF                        0
#define WM8350_LDO3_ENSLOT_1                          1
#define WM8350_LDO3_ENSLOT_2                          2
#define WM8350_LDO3_ENSLOT_3                          3
#define WM8350_LDO3_ENSLOT_4                          4
#define WM8350_LDO3_ENSLOT_5                          5
#define WM8350_LDO3_ENSLOT_6                          6
#define WM8350_LDO3_ENSLOT_7                          7
#define WM8350_LDO3_ENSLOT_8                          8
#define WM8350_LDO3_ENSLOT_9                          9
#define WM8350_LDO3_ENSLOT_10                         10
#define WM8350_LDO3_ENSLOT_11                         11
#define WM8350_LDO3_ENSLOT_12                         12
#define WM8350_LDO3_ENSLOT_13                         13
#define WM8350_LDO3_ENSLOT_14                         14
#define WM8350_LDO3_ENSLOT_END                        15

#define WM8350_LDO3_SDSLOT_OFF                        0
#define WM8350_LDO3_SDSLOT_1                          1
#define WM8350_LDO3_SDSLOT_2                          2
#define WM8350_LDO3_SDSLOT_3                          3
#define WM8350_LDO3_SDSLOT_4                          4
#define WM8350_LDO3_SDSLOT_5                          5
#define WM8350_LDO3_SDSLOT_6                          6
#define WM8350_LDO3_SDSLOT_7                          7
#define WM8350_LDO3_SDSLOT_8                          8
#define WM8350_LDO3_SDSLOT_9                          9
#define WM8350_LDO3_SDSLOT_10                         10
#define WM8350_LDO3_SDSLOT_11                         11
#define WM8350_LDO3_SDSLOT_12                         12
#define WM8350_LDO3_SDSLOT_13                         13
#define WM8350_LDO3_SDSLOT_14                         14

/*
 * R208 (0xD0) - LDO3 Low Power
 */
#define WM8350_LDO3_HIB_MODE                    0x3000  /* LDO3_HIB_MODE */
#define WM8350_LDO3_HIB_MODE_MASK               0x3000  /* LDO3_HIB_MODE - [14:12] */
#define WM8350_LDO3_HIB_MODE_SHIFT                  12  /* LDO3_HIB_MODE */
#define WM8350_LDO3_HIB_TRIG                    0x0300  /* LDO3_HIB_TRIG */
#define WM8350_LDO3_HIB_TRIG_MASK               0x0300  /* LDO3_HIB_TRIG - [9:8] */
#define WM8350_LDO3_HIB_TRIG_SHIFT                   8  /* LDO3_HIB_TRIG */
#define WM8350_LDO3_VIMG_MASK                   0x001F  /* LDO3_VIMG - [6:0] */

/* Bit values for R208 (0xD0) */

#define WM8350_LDO3_HIB_MODE_V_IMAGE                 0
#define WM8350_LDO3_HIB_MODE_DISABLE                 1

#define WM8350_LDO3_HIB_TRIG_HIBERNATE               0
#define WM8350_LDO3_HIB_TRIG_L_PWR1                  1
#define WM8350_LDO3_HIB_TRIG_L_PWR2                  2
#define WM8350_LDO3_HIB_TRIG_L_PWR3                  3

/*
 * R209 (0xD1) - LDO4 Control
 */
#define WM8350_LDO4_SWI                         0x4000  /* LDO4_SWI */
#define WM8350_LDO4_OPFLT                       0x0400  /* LDO4_OPFLT */
#define WM8350_LDO4_VSEL_MASK                   0x001F  /* LDO4_VSEL - [4:0] */
#define WM8350_LDO4_VSEL_SHIFT                       0  /* LDO4_VSEL - [4:0] */

/* Bit values for R209 (0xD1) */
#define WM8350_LDO4_VSEL( _mV )                 ((WM_REGVAL)( ((_mV)<1800) ? (((_mV)-900)/50)  : (((_mV)-1800)/100 + 16) ))
#define WM8350_LDO4_VSEL_V( _val )              ( ((_val)<16)  ? (((_val)*50)+900) : ((((_val)-16)*100)+1800) )

/*
 * R210 (0xD2) - LDO4 Timeouts
 */
#define WM8350_LDO4_ERRACT_MASK                 0xC000  /* LDO4_ERRACT - [15:14] */
#define WM8350_LDO4_ERRACT_SHIFT                    14  /* LDO4_ERRACT - [15:14] */
#define WM8350_LDO4_ENSLOT_MASK                 0x3C00  /* LDO4_ENSLOT - [13:10] */
#define WM8350_LDO4_ENSLOT_SHIFT                    10  /* LDO4_ENSLOT - [13:10] */
#define WM8350_LDO4_SDSLOT_MASK                 0x03C0  /* LDO4_SDSLOT - [9:6] */
#define WM8350_LDO4_SDSLOT_SHIFT                     6  /* LDO4_SDSLOT - [9:6] */

/* Bit values for R210 (0xD2) */
#define WM8350_LDO4_ERRACT_NONE                      0
#define WM8350_LDO4_ERRACT_SHUTDOWN_CONV             1
#define WM8350_LDO4_ERRACT_SHUTDOWN_SYS              2

#define WM8350_LDO4_ENSLOT_OFF                       0
#define WM8350_LDO4_ENSLOT_1                         1
#define WM8350_LDO4_ENSLOT_2                         2
#define WM8350_LDO4_ENSLOT_3                         3
#define WM8350_LDO4_ENSLOT_4                         4
#define WM8350_LDO4_ENSLOT_5                         5
#define WM8350_LDO4_ENSLOT_6                         6
#define WM8350_LDO4_ENSLOT_7                         7
#define WM8350_LDO4_ENSLOT_8                         8
#define WM8350_LDO4_ENSLOT_9                         9
#define WM8350_LDO4_ENSLOT_10                        10
#define WM8350_LDO4_ENSLOT_11                        11
#define WM8350_LDO4_ENSLOT_12                        12
#define WM8350_LDO4_ENSLOT_13                        13
#define WM8350_LDO4_ENSLOT_14                        14
#define WM8350_LDO4_ENSLOT_END                       15

#define WM8350_LDO4_SDSLOT_OFF                       0
#define WM8350_LDO4_SDSLOT_1                         1
#define WM8350_LDO4_SDSLOT_2                         2
#define WM8350_LDO4_SDSLOT_3                         3
#define WM8350_LDO4_SDSLOT_4                         4
#define WM8350_LDO4_SDSLOT_5                         5
#define WM8350_LDO4_SDSLOT_6                         6
#define WM8350_LDO4_SDSLOT_7                         7
#define WM8350_LDO4_SDSLOT_8                         8
#define WM8350_LDO4_SDSLOT_9                         9
#define WM8350_LDO4_SDSLOT_10                        10
#define WM8350_LDO4_SDSLOT_11                        11
#define WM8350_LDO4_SDSLOT_12                        12
#define WM8350_LDO4_SDSLOT_13                        13
#define WM8350_LDO4_SDSLOT_14                        14

/*
 * R211 (0xD3) - LDO4 Low Power
 */
#define WM8350_LDO4_HIB_MODE                    0x3000  /* LDO4_HIB_MODE */
#define WM8350_LDO4_HIB_MODE_MASK               0x3000  /* LDO4_HIB_MODE - [13:12] */
#define WM8350_LDO4_HIB_MODE_SHIFT                  12  /* LDO4_HIB_MODE */
#define WM8350_LDO4_HIB_TRIG                    0x0300  /* LDO4_HIB_TRIG */
#define WM8350_LDO4_HIB_TRIG_MASK               0x0300  /* LDO4_HIB_TRIG - [9:8] */
#define WM8350_LDO4_HIB_TRIG_SHIFT                   8  /* LDO4_HIB_TRIG */
#define WM8350_LDO4_VIMG_MASK                   0x001F  /* LDO4_VIMG - [5:0] */

/* Bit values for R208 (0xD0) */

#define WM8350_LDO4_HIB_MODE_V_IMAGE                 0
#define WM8350_LDO4_HIB_MODE_DISABLE                 1

#define WM8350_LDO4_HIB_TRIG_HIBERNATE                 0
#define WM8350_LDO4_HIB_TRIG_L_PWR1                     1
#define WM8350_LDO4_HIB_TRIG_L_PWR2                     2
#define WM8350_LDO4_HIB_TRIG_L_PWR3                     3

/*
 * R215 (0xD7) - VCC_FAULT Masks
 */
#define WM8350_LS_FAULT                         0x8000  /* LS_FAULT */
#define WM8350_LDO4_FAULT                       0x0800  /* LDO4_FAULT */
#define WM8350_LDO3_FAULT                       0x0400  /* LDO3_FAULT */
#define WM8350_LDO2_FAULT                       0x0200  /* LDO2_FAULT */
#define WM8350_LDO1_FAULT                       0x0100  /* LDO1_FAULT */
#define WM8350_DC6_FAULT                        0x0020  /* DC6_FAULT */
#define WM8350_DC5_FAULT                        0x0010  /* DC5_FAULT */
#define WM8350_DC4_FAULT                        0x0008  /* DC4_FAULT */
#define WM8350_DC3_FAULT                        0x0004  /* DC3_FAULT */
#define WM8350_DC2_FAULT                        0x0002  /* DC2_FAULT */
#define WM8350_DC1_FAULT                        0x0001  /* DC1_FAULT */

/*
 * R216 (0xD8) - Main Bandgap Control
 */
#define WM8350_MBG_LOAD_FUSES                   0x8000  /* MBG_LOAD_FUSES */

/*
 * R217 (0xD9) - OSC Control
 */
#define WM8350_OSC_LOAD_FUSES                   0x8000  /* OSC_LOAD_FUSES */

/*
 * R218 (0xDA) - RTC Tick Control
 */
#define WM8350_RTC_TICK_ENA_R218                0x8000  /* RTC_TICK_ENA */
#define WM8350_RTC_TICKSTS                      0x4000  /* RTC_TICKSTS */
#define WM8350_RTC_CLKSRC                       0x2000  /* RTC_CLKSRC */
#define WM8350_OSC32K_ENA_R218                  0x1000  /* OSC32K_ENA */
#define WM8350_RTC_TRIM_MASK                    0x03FF  /* RTC_TRIM - [9:0] */

/*
 * R219 (0xDB) - Security
 */
#define WM8350_SECURITY_MASK                    0xFFFF
#define WM8350_SECURITY_SHIFT                        0

/* Bit definitions for R219 (0xDB) */
#define WM8350_SECURITY_UNLOCK                  0x0013  /* Key to unlock protected registers */
#define WM8350_SECURITY_LOCK                    0x0000  /* Any other value will lock it */

/*
 * R224 (0xE0) - Signal overrides
 */
#define WM8350_WALL_FB_GT_BATT_OVRDE            0x0800  /* WALL_FB_GT_BATT_OVRDE */
#define WM8350_USB_FB_GT_BATT_OVRDE             0x0400  /* USB_FB_GT_BATT_OVRDE */
#define WM8350_FLL_OK_OVRDE                     0x0200  /* FLL_OK_OVRDE */
#define WM8350_DEB_TICK_OVRDE                   0x0100  /* DEB_TICK_OVRDE */
#define WM8350_UVLO_B_OVRDE                     0x0080  /* UVLO_B_OVRDE */
#define WM8350_RTC_ALARM_OVRDE                  0x0040  /* RTC_ALARM_OVRDE */
#define WM8350_LINE_GT_BATT_OVRDE               0x0008  /* LINE_GT_BATT_OVRDE */
#define WM8350_LINE_GT_VRTC_OVRDE               0x0004  /* LINE_GT_VRTC_OVRDE */
#define WM8350_USB_GT_LINE_OVRDE                0x0002  /* USB_GT_LINE_OVRDE */
#define WM8350_BATT_GT_USB_OVRDE                0x0001  /* BATT_GT_USB_OVRDE */

/*
 * R225 (0xE1) - DCDC/LDO status
 */
#define WM8350_LS_STS                           0x8000  /* LS_STS */
#define WM8350_LDO4_STS                         0x0800  /* LDO4_STS */
#define WM8350_LDO3_STS                         0x0400  /* LDO3_STS */
#define WM8350_LDO2_STS                         0x0200  /* LDO2_STS */
#define WM8350_LDO1_STS                         0x0100  /* LDO1_STS */
#define WM8350_DC6_STS                          0x0020  /* DC6_STS */
#define WM8350_DC5_STS                          0x0010  /* DC5_STS */
#define WM8350_DC4_STS                          0x0008  /* DC4_STS */
#define WM8350_DC3_STS                          0x0004  /* DC3_STS */
#define WM8350_DC2_STS                          0x0002  /* DC2_STS */
#define WM8350_DC1_STS                          0x0001  /* DC1_STS */

/*
 * R226 (0xE2) - Charger Overides/status
 */
#define WM8350_CHG_BATT_HOT_OVRDE               0x8000  /* CHG_BATT_HOT_OVRDE */
#define WM8350_CHG_BATT_COLD_OVRDE              0x4000  /* CHG_BATT_COLD_OVRDE */
#define WM8350_CHG_END_OVRDE                    0x0800  /* CHG_END_OVRDE */
#define WM8350_CHG_BATT_LT_3P9_OVRDE            0x0004  /* CHG_BATT_LT_3P9_OVRDE */
#define WM8350_CHG_BATT_LT_3P1_OVRDE            0x0002  /* CHG_BATT_LT_3P1_OVRDE */
#define WM8350_CHG_BATT_LT_2P85_OVRDE           0x0001  /* CHG_BATT_LT_2P85_OVRDE */

/*
 * R227 (0xE3) - misc overrides
 */
#define WM8350_CS2_NOT_REG_OVRDE                0x2000  /* CS2_NOT_REG_OVRDE */
#define WM8350_CS1_NOT_REG_OVRDE                0x1000  /* CS1_NOT_REG_OVRDE */
#define WM8350_USB_LIMIT_OVRDE                  0x0400  /* USB_LIMIT_OVRDE */
#define WM8350_AUX_DCOMP4_OVRDE                 0x0080  /* AUX_DCOMP4_OVRDE */
#define WM8350_AUX_DCOMP3_OVRDE                 0x0040  /* AUX_DCOMP3_OVRDE */
#define WM8350_AUX_DCOMP2_OVRDE                 0x0020  /* AUX_DCOMP2_OVRDE */
#define WM8350_AUX_DCOMP1_OVRDE                 0x0010  /* AUX_DCOMP1_OVRDE */
#define WM8350_HYST_UVLO_OK_OVRDE               0x0008  /* HYST_UVLO_OK_OVRDE */
#define WM8350_CHIP_GT115_OVRDE                 0x0004  /* CHIP_GT115_OVRDE */
#define WM8350_CHIP_GT140_OVRDE                 0x0002  /* CHIP_GT140_OVRDE */

/*
 * R228 (0xE4) - Supply overrides/status 1
 */
#define WM8350_OVRV_DC6_OVRDE                   0x0020  /* OVRV_DC6_OVRDE */
#define WM8350_OVRV_DC4_OVRDE                   0x0008  /* OVRV_DC4_OVRDE */
#define WM8350_OVRV_DC3_OVRDE                   0x0004  /* OVRV_DC3_OVRDE */
#define WM8350_OVRV_DC1_OVRDE                   0x0001  /* OVRV_DC1_OVRDE */

/*
 * R229 (0xE5) - Supply overrides/status 2
 */
#define WM8350_OVCR_LS_OVRDE                    0x8000  /* OVCR_LS_OVRDE */
#define WM8350_UNDV_LDO4_OVRDE                  0x0800  /* UNDV_LDO4_OVRDE */
#define WM8350_UNDV_LDO3_OVRDE                  0x0400  /* UNDV_LDO3_OVRDE */
#define WM8350_UNDV_LDO2_OVRDE                  0x0200  /* UNDV_LDO2_OVRDE */
#define WM8350_UNDV_LDO1_OVRDE                  0x0100  /* UNDV_LDO1_OVRDE */
#define WM8350_UNDV_DC6_OVRDE                   0x0020  /* UNDV_DC6_OVRDE */
#define WM8350_UNDV_DC5_OVRDE                   0x0010  /* UNDV_DC5_OVRDE */
#define WM8350_UNDV_DC4_OVRDE                   0x0008  /* UNDV_DC4_OVRDE */
#define WM8350_UNDV_DC3_OVRDE                   0x0004  /* UNDV_DC3_OVRDE */
#define WM8350_UNDV_DC2_OVRDE                   0x0002  /* UNDV_DC2_OVRDE */
#define WM8350_UNDV_DC1_OVRDE                   0x0001  /* UNDV_DC1_OVRDE */

/*
 * R230 (0xE6) - GPIO Pin Status
 */
#define WM8350_GP12_LVL                         0x1000  /* GP12_LVL */
#define WM8350_GP11_LVL                         0x0800  /* GP11_LVL */
#define WM8350_GP10_LVL                         0x0400  /* GP10_LVL */
#define WM8350_GP9_LVL                          0x0200  /* GP9_LVL */
#define WM8350_GP8_LVL                          0x0100  /* GP8_LVL */
#define WM8350_GP7_LVL                          0x0080  /* GP7_LVL */
#define WM8350_GP6_LVL                          0x0040  /* GP6_LVL */
#define WM8350_GP5_LVL                          0x0020  /* GP5_LVL */
#define WM8350_GP4_LVL                          0x0010  /* GP4_LVL */
#define WM8350_GP3_LVL                          0x0008  /* GP3_LVL */
#define WM8350_GP2_LVL                          0x0004  /* GP2_LVL */
#define WM8350_GP1_LVL                          0x0002  /* GP1_LVL */
#define WM8350_GP0_LVL                          0x0001  /* GP0_LVL */

/*
 * R231 (0xE7) - comparator overrides
 */
#define WM8350_USB_FB_OVRDE                     0x8000  /* USB_FB_OVRDE */
#define WM8350_WALL_FB_OVRDE                    0x4000  /* WALL_FB_OVRDE */
#define WM8350_BATT_FB_OVRDE                    0x2000  /* BATT_FB_OVRDE */
#define WM8350_CODEC_JCK_DET_L_OVRDE            0x0800  /* CODEC_JCK_DET_L_OVRDE */
#define WM8350_CODEC_JCK_DET_R_OVRDE            0x0400  /* CODEC_JCK_DET_R_OVRDE */
#define WM8350_CODEC_MICSCD_OVRDE               0x0200  /* CODEC_MICSCD_OVRDE */
#define WM8350_CODEC_MICD_OVRDE                 0x0100  /* CODEC_MICD_OVRDE */

/*
 * R233 (0xE9) - State Machine status
 */
#define WM8350_USB_SM_MASK                      0x0700  /* USB_SM - [10:8] */
#define WM8350_CHG_SM_MASK                      0x0070  /* CHG_SM - [6:4] */
#define WM8350_MAIN_SM_MASK                     0x000F  /* MAIN_SM - [3:0] */

/*
 * R248 (0xF8) - DCDC1 Test Controls
 */
#define WM8350_DC1_FORCE_PWM                    0x0010  /* DC1_FORCE_PWM */

/*
 * R250 (0xFA) - DCDC3 Test Controls
 */
#define WM8350_DC3_FORCE_PWM                    0x0010  /* DC3_FORCE_PWM */

/*
 * R251 (0xFB) - DCDC4 Test Controls
 */
#define WM8350_DC4_FORCE_PWM                    0x0010  /* DC4_FORCE_PWM */

/*
 * R253 (0xFD) - DCDC6 Test Controls
 */
#define WM8350_DC6_FORCE_PWM                    0x0010  /* DC6_FORCE_PWM */

/*
 * Default values.
 */
#define WM8350_CONFIG_BANKS                     4

/* Bank 0 */
#define WM8350_REGISTER_DEFAULTS_0 \
{ \
    0x6143,     /* R0   - Reset/ID */ \
    0x3000,     /* R1   - ID */ \
    0x0000,     /* R2 */ \
    0x1002,     /* R3   - System Control 1 */ \
    0x0004,     /* R4   - System Control 2 */ \
    0x0000,     /* R5   - System Hibernate */ \
    0x8A00,     /* R6   - Interface Control */ \
    0x0000,     /* R7 */ \
    0x8000,     /* R8   - Power mgmt (1) */ \
    0x0000,     /* R9   - Power mgmt (2) */ \
    0x0000,     /* R10  - Power mgmt (3) */ \
    0x2000,     /* R11  - Power mgmt (4) */ \
    0x0E00,     /* R12  - Power mgmt (5) */ \
    0x0000,     /* R13  - Power mgmt (6) */ \
    0x0000,     /* R14  - Power mgmt (7) */ \
    0x0000,     /* R15 */ \
    0x0000,     /* R16  - RTC Seconds/Minutes */ \
    0x0100,     /* R17  - RTC Hours/Day */ \
    0x0101,     /* R18  - RTC Date/Month */ \
    0x1400,     /* R19  - RTC Year */ \
    0x0000,     /* R20  - Alarm Seconds/Minutes */ \
    0x0000,     /* R21  - Alarm Hours/Day */ \
    0x0000,     /* R22  - Alarm Date/Month */ \
    0x0320,     /* R23  - RTC Time Control */ \
    0x0000,     /* R24  - System Interrupts */ \
    0x0000,     /* R25  - Interrupt Status 1 */ \
    0x0000,     /* R26  - Interrupt Status 2 */ \
    0x0000,     /* R27 */ \
    0x0000,     /* R28  - Under Voltage Interrupt status */ \
    0x0000,     /* R29  - Over Current Interrupt status */ \
    0x0000,     /* R30  - GPIO Interrupt Status */ \
    0x0000,     /* R31  - Comparator Interrupt Status */ \
    0x3FFF,     /* R32  - System Interrupts Mask */ \
    0x0000,     /* R33  - Interrupt Status 1 Mask */ \
    0x0000,     /* R34  - Interrupt Status 2 Mask */ \
    0x0000,     /* R35 */ \
    0x0000,     /* R36  - Under Voltage Interrupt status Mask */ \
    0x0000,     /* R37  - Over Current Interrupt status Mask */ \
    0x0000,     /* R38  - GPIO Interrupt Status Mask */ \
    0x0000,     /* R39  - Comparator Interrupt Status Mask */ \
    0x0040,     /* R40  - Clock Control 1 */ \
    0x0000,     /* R41  - Clock Control 2 */ \
    0x3B00,     /* R42  - FLL Control 1 */ \
    0x7086,     /* R43  - FLL Control 2 */ \
    0xC226,     /* R44  - FLL Control 3 */ \
    0x0000,     /* R45  - FLL Control 4 */ \
    0x0000,     /* R46 */ \
    0x0000,     /* R47 */ \
    0x0000,     /* R48  - DAC Control */ \
    0x0000,     /* R49 */ \
    0x00C0,     /* R50  - DAC Digital Volume L */ \
    0x00C0,     /* R51  - DAC Digital Volume R */ \
    0x0000,     /* R52 */ \
    0x0040,     /* R53  - DAC LR Rate */ \
    0x0000,     /* R54  - DAC Clock Control */ \
    0x0000,     /* R55 */ \
    0x0000,     /* R56 */ \
    0x0000,     /* R57 */ \
    0x4000,     /* R58  - DAC Mute */ \
    0x0000,     /* R59  - DAC Mute Volume */ \
    0x0000,     /* R60  - DAC Side */ \
    0x0000,     /* R61 */ \
    0x0000,     /* R62 */ \
    0x0000,     /* R63 */ \
    0x8000,     /* R64  - ADC Control */ \
    0x0000,     /* R65 */ \
    0x00C0,     /* R66  - ADC Digital Volume L */ \
    0x00C0,     /* R67  - ADC Digital Volume R */ \
    0x0000,     /* R68  - ADC Divider */ \
    0x0000,     /* R69 */ \
    0x0040,     /* R70  - ADC LR Rate */ \
    0x0000,     /* R71 */ \
    0x0303,     /* R72  - Input Control */ \
    0x0000,     /* R73  - IN3 Input Control */ \
    0x0000,     /* R74  - Mic Bias Control */ \
    0x0000,     /* R75 */ \
    0x0000,     /* R76  - Output Control */ \
    0x0000,     /* R77  - Jack Detect */ \
    0x0000,     /* R78  - Anti Pop Control */ \
    0x0000,     /* R79 */ \
    0x0040,     /* R80  - Left Input Volume */ \
    0x0040,     /* R81  - Right Input Volume */ \
    0x0000,     /* R82 */ \
    0x0000,     /* R83 */ \
    0x0000,     /* R84 */ \
    0x0000,     /* R85 */ \
    0x0000,     /* R86 */ \
    0x0000,     /* R87 */ \
    0x0800,     /* R88  - Left Mixer Control */ \
    0x1000,     /* R89  - Right Mixer Control */ \
    0x0000,     /* R90 */ \
    0x0000,     /* R91 */ \
    0x0000,     /* R92  - OUT3 Mixer Control */ \
    0x0000,     /* R93  - OUT4 Mixer Control */ \
    0x0000,     /* R94 */ \
    0x0000,     /* R95 */ \
    0x0000,     /* R96  - Output Left Mixer Volume */ \
    0x0000,     /* R97  - Output Right Mixer Volume */ \
    0x0000,     /* R98  - Input Mixer Volume L */ \
    0x0000,     /* R99  - Input Mixer Volume R */ \
    0x0000,     /* R100 - Input Mixer Volume */ \
    0x0000,     /* R101 */ \
    0x0000,     /* R102 */ \
    0x0000,     /* R103 */ \
    0x00E4,     /* R104 - OUT1L Volume */ \
    0x00E4,     /* R105 - OUT1R Volume */ \
    0x00E4,     /* R106 - OUT2L Volume */ \
    0x02E4,     /* R107 - OUT2R Volume */ \
    0x0000,     /* R108 */ \
    0x0000,     /* R109 */ \
    0x0000,     /* R110 */ \
    0x0000,     /* R111 - BEEP Volume */ \
    0x0A00,     /* R112 - AI Formating */ \
    0x0000,     /* R113 - ADC DAC COMP */ \
    0x0020,     /* R114 - AI ADC Control */ \
    0x0020,     /* R115 - AI DAC Control */ \
    0x0000,     /* R116 */ \
    0x0000,     /* R117 */ \
    0x0000,     /* R118 */ \
    0x0000,     /* R119 */ \
    0x0000,     /* R120 */ \
    0x0000,     /* R121 */ \
    0x0000,     /* R122 */ \
    0x0000,     /* R123 */ \
    0x0000,     /* R124 */ \
    0x0000,     /* R125 */ \
    0x0000,     /* R126 */ \
    0x0000,     /* R127 */ \
    0x0812,     /* R128 - GPIO Debounce */ \
    0x0000,     /* R129 - GPIO Pin pull up Control */ \
    0x0110,     /* R130 - GPIO Pull down Control */ \
    0x0000,     /* R131 - GPIO Interrupt Mode */ \
    0x0000,     /* R132 */ \
    0x0000,     /* R133 - GPIO Control */ \
    0x0FFC,     /* R134 - GPIO Configuration (i/o) */ \
    0x0FFC,     /* R135 - GPIO Pin Polarity / Type */ \
    0x0000,     /* R136 */ \
    0x0000,     /* R137 */ \
    0x0000,     /* R138 */ \
    0x0000,     /* R139 */ \
    0x0013,     /* R140 - GPIO Function Select 1 */ \
    0x0000,     /* R141 - GPIO Function Select 2 */ \
    0x0000,     /* R142 - GPIO Function Select 3 */ \
    0x0003,     /* R143 - GPIO Function Select 4 */ \
    0x0000,     /* R144 - Digitiser Control (1) */ \
    0x0002,     /* R145 - Digitiser Control (2) */ \
    0x0000,     /* R146 */ \
    0x0000,     /* R147 */ \
    0x0000,     /* R148 */ \
    0x0000,     /* R149 */ \
    0x0000,     /* R150 */ \
    0x0000,     /* R151 */ \
    0x7000,     /* R152 - AUX1 Readback */ \
    0x7000,     /* R153 - AUX2 Readback */ \
    0x7000,     /* R154 - AUX3 Readback */ \
    0x7000,     /* R155 - AUX4 Readback */ \
    0x0000,     /* R156 - USB Voltage Readback */ \
    0x0000,     /* R157 - LINE Voltage Readback */ \
    0x0000,     /* R158 - BATT Voltage Readback */ \
    0x0000,     /* R159 - Chip Temp Readback */ \
    0x0000,     /* R160 */ \
    0x0000,     /* R161 */ \
    0x0000,     /* R162 */ \
    0x0000,     /* R163 - Generic Comparator Control */ \
    0x0000,     /* R164 - Generic comparator 1 */ \
    0x0000,     /* R165 - Generic comparator 2 */ \
    0x0000,     /* R166 - Generic comparator 3 */ \
    0x0000,     /* R167 - Generic comparator 4 */ \
    0xA00F,     /* R168 - Battery Charger Control 1 */ \
    0x0B06,     /* R169 - Battery Charger Control 2 */ \
    0x0000,     /* R170 - Battery Charger Control 3 */ \
    0x0000,     /* R171 */ \
    0x0000,     /* R172 - Current Sink Driver A */ \
    0x0000,     /* R173 - CSA Flash control */ \
    0x0000,     /* R174 - Current Sink Driver B */ \
    0x0000,     /* R175 - CSB Flash control */ \
    0x0000,     /* R176 - DCDC/LDO requested */ \
    0x002D,     /* R177 - DCDC Active options */ \
    0x0000,     /* R178 - DCDC Sleep options */ \
    0x0025,     /* R179 - Power-check comparator */ \
    0x000E,     /* R180 - DCDC1 Control */ \
    0x0000,     /* R181 - DCDC1 Timeouts */ \
    0x1006,     /* R182 - DCDC1 Low Power */ \
    0x0018,     /* R183 - DCDC2 Control */ \
    0x0000,     /* R184 - DCDC2 Timeouts */ \
    0x0000,     /* R185 */ \
    0x0000,     /* R186 - DCDC3 Control */ \
    0x0000,     /* R187 - DCDC3 Timeouts */ \
    0x0006,     /* R188 - DCDC3 Low Power */ \
    0x0000,     /* R189 - DCDC4 Control */ \
    0x0000,     /* R190 - DCDC4 Timeouts */ \
    0x0006,     /* R191 - DCDC4 Low Power */ \
    0x0008,     /* R192 - DCDC5 Control */ \
    0x0000,     /* R193 - DCDC5 Timeouts */ \
    0x0000,     /* R194 */ \
    0x0000,     /* R195 - DCDC6 Control */ \
    0x0000,     /* R196 - DCDC6 Timeouts */ \
    0x0006,     /* R197 - DCDC6 Low Power */ \
    0x0000,     /* R198 */ \
    0x0003,     /* R199 - Limit Switch Control */ \
    0x001C,     /* R200 - LDO1 Control */ \
    0x0000,     /* R201 - LDO1 Timeouts */ \
    0x001C,     /* R202 - LDO1 Low Power */ \
    0x001B,     /* R203 - LDO2 Control */ \
    0x0000,     /* R204 - LDO2 Timeouts */ \
    0x001C,     /* R205 - LDO2 Low Power */ \
    0x001B,     /* R206 - LDO3 Control */ \
    0x0000,     /* R207 - LDO3 Timeouts */ \
    0x001C,     /* R208 - LDO3 Low Power */ \
    0x001B,     /* R209 - LDO4 Control */ \
    0x0000,     /* R210 - LDO4 Timeouts */ \
    0x001C,     /* R211 - LDO4 Low Power */ \
    0x0000,     /* R212 */ \
    0x0000,     /* R213 */ \
    0x0000,     /* R214 */ \
    0x0000,     /* R215 - VCC_FAULT Masks */ \
    0x001F,     /* R216 - Main Bandgap Control */ \
    0x0000,     /* R217 - OSC Control */ \
    0x9000,     /* R218 - RTC Tick Control */ \
    0x0000,     /* R219 - Security */ \
    0x0000,     /* R220 */ \
    0x0000,     /* R221 */ \
    0x0000,     /* R222 */ \
    0x0000,     /* R223 */ \
    0x0000,     /* R224 - Signal overrides */ \
    0x0000,     /* R225 - DCDC/LDO status */ \
    0x0000,     /* R226 - Charger Overides/status */ \
    0x0000,     /* R227 - misc overrides */ \
    0x0000,     /* R228 - Supply overrides/status 1 */ \
    0x0000,     /* R229 - Supply overrides/status 2 */ \
    0xE000,     /* R230 - GPIO Pin Status */ \
    0x0000,     /* R231 - comparator overrides */ \
    0x0000,     /* R232 */ \
    0x0000,     /* R233 - State Machine status */ \
    0x0000,     /* R234 */ \
    0x0000,     /* R235 */ \
    0x0000,     /* R236 */ \
    0x0000,     /* R237 */ \
    0x0000,     /* R238 */ \
    0x0000,     /* R239 */ \
    0x0000,     /* R240 */ \
    0x0000,     /* R241 */ \
    0x0000,     /* R242 */ \
    0x0000,     /* R243 */ \
    0x0000,     /* R244 */ \
    0x0000,     /* R245 */ \
    0x0000,     /* R246 */ \
    0x0000,     /* R247 */ \
    0x1000,     /* R248 - DCDC1 Test Controls */ \
    0x0000,     /* R249 */ \
    0x1000,     /* R250 - DCDC3 Test Controls */ \
    0x1000,     /* R251 - DCDC4 Test Controls */ \
    0x0000,     /* R252 */ \
    0x1000,     /* R253 - DCDC6 Test Controls */ \
}

/* Bank 1 */
#define WM8350_REGISTER_DEFAULTS_1 \
{ \
    0x6143,     /* R0   - Reset/ID */ \
    0x3000,     /* R1   - ID */ \
    0x0000,     /* R2 */ \
    0x1002,     /* R3   - System Control 1 */ \
    0x0014,     /* R4   - System Control 2 */ \
    0x0000,     /* R5   - System Hibernate */ \
    0x8A00,     /* R6   - Interface Control */ \
    0x0000,     /* R7 */ \
    0x8000,     /* R8   - Power mgmt (1) */ \
    0x0000,     /* R9   - Power mgmt (2) */ \
    0x0000,     /* R10  - Power mgmt (3) */ \
    0x2000,     /* R11  - Power mgmt (4) */ \
    0x0E00,     /* R12  - Power mgmt (5) */ \
    0x0000,     /* R13  - Power mgmt (6) */ \
    0x0000,     /* R14  - Power mgmt (7) */ \
    0x0000,     /* R15 */ \
    0x0000,     /* R16  - RTC Seconds/Minutes */ \
    0x0100,     /* R17  - RTC Hours/Day */ \
    0x0101,     /* R18  - RTC Date/Month */ \
    0x1400,     /* R19  - RTC Year */ \
    0x0000,     /* R20  - Alarm Seconds/Minutes */ \
    0x0000,     /* R21  - Alarm Hours/Day */ \
    0x0000,     /* R22  - Alarm Date/Month */ \
    0x0320,     /* R23  - RTC Time Control */ \
    0x0000,     /* R24  - System Interrupts */ \
    0x0000,     /* R25  - Interrupt Status 1 */ \
    0x0000,     /* R26  - Interrupt Status 2 */ \
    0x0000,     /* R27 */ \
    0x0000,     /* R28  - Under Voltage Interrupt status */ \
    0x0000,     /* R29  - Over Current Interrupt status */ \
    0x0000,     /* R30  - GPIO Interrupt Status */ \
    0x0000,     /* R31  - Comparator Interrupt Status */ \
    0x3FFF,     /* R32  - System Interrupts Mask */ \
    0x0000,     /* R33  - Interrupt Status 1 Mask */ \
    0x0000,     /* R34  - Interrupt Status 2 Mask */ \
    0x0000,     /* R35 */ \
    0x0000,     /* R36  - Under Voltage Interrupt status Mask */ \
    0x0000,     /* R37  - Over Current Interrupt status Mask */ \
    0x0000,     /* R38  - GPIO Interrupt Status Mask */ \
    0x0000,     /* R39  - Comparator Interrupt Status Mask */ \
    0x0040,     /* R40  - Clock Control 1 */ \
    0x0000,     /* R41  - Clock Control 2 */ \
    0x3B00,     /* R42  - FLL Control 1 */ \
    0x7086,     /* R43  - FLL Control 2 */ \
    0xC226,     /* R44  - FLL Control 3 */ \
    0x0000,     /* R45  - FLL Control 4 */ \
    0x0000,     /* R46 */ \
    0x0000,     /* R47 */ \
    0x0000,     /* R48  - DAC Control */ \
    0x0000,     /* R49 */ \
    0x00C0,     /* R50  - DAC Digital Volume L */ \
    0x00C0,     /* R51  - DAC Digital Volume R */ \
    0x0000,     /* R52 */ \
    0x0040,     /* R53  - DAC LR Rate */ \
    0x0000,     /* R54  - DAC Clock Control */ \
    0x0000,     /* R55 */ \
    0x0000,     /* R56 */ \
    0x0000,     /* R57 */ \
    0x4000,     /* R58  - DAC Mute */ \
    0x0000,     /* R59  - DAC Mute Volume */ \
    0x0000,     /* R60  - DAC Side */ \
    0x0000,     /* R61 */ \
    0x0000,     /* R62 */ \
    0x0000,     /* R63 */ \
    0x8000,     /* R64  - ADC Control */ \
    0x0000,     /* R65 */ \
    0x00C0,     /* R66  - ADC Digital Volume L */ \
    0x00C0,     /* R67  - ADC Digital Volume R */ \
    0x0000,     /* R68  - ADC Divider */ \
    0x0000,     /* R69 */ \
    0x0040,     /* R70  - ADC LR Rate */ \
    0x0000,     /* R71 */ \
    0x0303,     /* R72  - Input Control */ \
    0x0000,     /* R73  - IN3 Input Control */ \
    0x0000,     /* R74  - Mic Bias Control */ \
    0x0000,     /* R75 */ \
    0x0000,     /* R76  - Output Control */ \
    0x0000,     /* R77  - Jack Detect */ \
    0x0000,     /* R78  - Anti Pop Control */ \
    0x0000,     /* R79 */ \
    0x0040,     /* R80  - Left Input Volume */ \
    0x0040,     /* R81  - Right Input Volume */ \
    0x0000,     /* R82 */ \
    0x0000,     /* R83 */ \
    0x0000,     /* R84 */ \
    0x0000,     /* R85 */ \
    0x0000,     /* R86 */ \
    0x0000,     /* R87 */ \
    0x0800,     /* R88  - Left Mixer Control */ \
    0x1000,     /* R89  - Right Mixer Control */ \
    0x0000,     /* R90 */ \
    0x0000,     /* R91 */ \
    0x0000,     /* R92  - OUT3 Mixer Control */ \
    0x0000,     /* R93  - OUT4 Mixer Control */ \
    0x0000,     /* R94 */ \
    0x0000,     /* R95 */ \
    0x0000,     /* R96  - Output Left Mixer Volume */ \
    0x0000,     /* R97  - Output Right Mixer Volume */ \
    0x0000,     /* R98  - Input Mixer Volume L */ \
    0x0000,     /* R99  - Input Mixer Volume R */ \
    0x0000,     /* R100 - Input Mixer Volume */ \
    0x0000,     /* R101 */ \
    0x0000,     /* R102 */ \
    0x0000,     /* R103 */ \
    0x00E4,     /* R104 - OUT1L Volume */ \
    0x00E4,     /* R105 - OUT1R Volume */ \
    0x00E4,     /* R106 - OUT2L Volume */ \
    0x02E4,     /* R107 - OUT2R Volume */ \
    0x0000,     /* R108 */ \
    0x0000,     /* R109 */ \
    0x0000,     /* R110 */ \
    0x0000,     /* R111 - BEEP Volume */ \
    0x0A00,     /* R112 - AI Formating */ \
    0x0000,     /* R113 - ADC DAC COMP */ \
    0x0020,     /* R114 - AI ADC Control */ \
    0x0020,     /* R115 - AI DAC Control */ \
    0x0000,     /* R116 */ \
    0x0000,     /* R117 */ \
    0x0000,     /* R118 */ \
    0x0000,     /* R119 */ \
    0x0000,     /* R120 */ \
    0x0000,     /* R121 */ \
    0x0000,     /* R122 */ \
    0x0000,     /* R123 */ \
    0x0000,     /* R124 */ \
    0x0000,     /* R125 */ \
    0x0000,     /* R126 */ \
    0x0000,     /* R127 */ \
    0x0812,     /* R128 - GPIO Debounce */ \
    0x0000,     /* R129 - GPIO Pin pull up Control */ \
    0x0110,     /* R130 - GPIO Pull down Control */ \
    0x0000,     /* R131 - GPIO Interrupt Mode */ \
    0x0000,     /* R132 */ \
    0x0000,     /* R133 - GPIO Control */ \
    0x00FB,     /* R134 - GPIO Configuration (i/o) */ \
    0x04FE,     /* R135 - GPIO Pin Polarity / Type */ \
    0x0000,     /* R136 */ \
    0x0000,     /* R137 */ \
    0x0000,     /* R138 */ \
    0x0000,     /* R139 */ \
    0x0312,     /* R140 - GPIO Function Select 1 */ \
    0x1003,     /* R141 - GPIO Function Select 2 */ \
    0x1331,     /* R142 - GPIO Function Select 3 */ \
    0x0003,     /* R143 - GPIO Function Select 4 */ \
    0x0000,     /* R144 - Digitiser Control (1) */ \
    0x0002,     /* R145 - Digitiser Control (2) */ \
    0x0000,     /* R146 */ \
    0x0000,     /* R147 */ \
    0x0000,     /* R148 */ \
    0x0000,     /* R149 */ \
    0x0000,     /* R150 */ \
    0x0000,     /* R151 */ \
    0x7000,     /* R152 - AUX1 Readback */ \
    0x7000,     /* R153 - AUX2 Readback */ \
    0x7000,     /* R154 - AUX3 Readback */ \
    0x7000,     /* R155 - AUX4 Readback */ \
    0x0000,     /* R156 - USB Voltage Readback */ \
    0x0000,     /* R157 - LINE Voltage Readback */ \
    0x0000,     /* R158 - BATT Voltage Readback */ \
    0x0000,     /* R159 - Chip Temp Readback */ \
    0x0000,     /* R160 */ \
    0x0000,     /* R161 */ \
    0x0000,     /* R162 */ \
    0x0000,     /* R163 - Generic Comparator Control */ \
    0x0000,     /* R164 - Generic comparator 1 */ \
    0x0000,     /* R165 - Generic comparator 2 */ \
    0x0000,     /* R166 - Generic comparator 3 */ \
    0x0000,     /* R167 - Generic comparator 4 */ \
    0xA00F,     /* R168 - Battery Charger Control 1 */ \
    0x0B06,     /* R169 - Battery Charger Control 2 */ \
    0x0000,     /* R170 - Battery Charger Control 3 */ \
    0x0000,     /* R171 */ \
    0x0000,     /* R172 - Current Sink Driver A */ \
    0x0000,     /* R173 - CSA Flash control */ \
    0x0000,     /* R174 - Current Sink Driver B */ \
    0x0000,     /* R175 - CSB Flash control */ \
    0x0000,     /* R176 - DCDC/LDO requested */ \
    0x002D,     /* R177 - DCDC Active options */ \
    0x0000,     /* R178 - DCDC Sleep options */ \
    0x0025,     /* R179 - Power-check comparator */ \
    0x0062,     /* R180 - DCDC1 Control */ \
    0x0400,     /* R181 - DCDC1 Timeouts */ \
    0x1006,     /* R182 - DCDC1 Low Power */ \
    0x0018,     /* R183 - DCDC2 Control */ \
    0x0000,     /* R184 - DCDC2 Timeouts */ \
    0x0000,     /* R185 */ \
    0x0026,     /* R186 - DCDC3 Control */ \
    0x0400,     /* R187 - DCDC3 Timeouts */ \
    0x0006,     /* R188 - DCDC3 Low Power */ \
    0x0062,     /* R189 - DCDC4 Control */ \
    0x0400,     /* R190 - DCDC4 Timeouts */ \
    0x0006,     /* R191 - DCDC4 Low Power */ \
    0x0008,     /* R192 - DCDC5 Control */ \
    0x0000,     /* R193 - DCDC5 Timeouts */ \
    0x0000,     /* R194 */ \
    0x0026,     /* R195 - DCDC6 Control */ \
    0x0800,     /* R196 - DCDC6 Timeouts */ \
    0x0006,     /* R197 - DCDC6 Low Power */ \
    0x0000,     /* R198 */ \
    0x0003,     /* R199 - Limit Switch Control */ \
    0x0006,     /* R200 - LDO1 Control */ \
    0x0400,     /* R201 - LDO1 Timeouts */ \
    0x001C,     /* R202 - LDO1 Low Power */ \
    0x0006,     /* R203 - LDO2 Control */ \
    0x0400,     /* R204 - LDO2 Timeouts */ \
    0x001C,     /* R205 - LDO2 Low Power */ \
    0x001B,     /* R206 - LDO3 Control */ \
    0x0000,     /* R207 - LDO3 Timeouts */ \
    0x001C,     /* R208 - LDO3 Low Power */ \
    0x001B,     /* R209 - LDO4 Control */ \
    0x0000,     /* R210 - LDO4 Timeouts */ \
    0x001C,     /* R211 - LDO4 Low Power */ \
    0x0000,     /* R212 */ \
    0x0000,     /* R213 */ \
    0x0000,     /* R214 */ \
    0x0000,     /* R215 - VCC_FAULT Masks */ \
    0x001F,     /* R216 - Main Bandgap Control */ \
    0x0000,     /* R217 - OSC Control */ \
    0x9000,     /* R218 - RTC Tick Control */ \
    0x0000,     /* R219 - Security */ \
    0x0000,     /* R220 */ \
    0x0000,     /* R221 */ \
    0x0000,     /* R222 */ \
    0x0000,     /* R223 */ \
    0x0000,     /* R224 - Signal overrides */ \
    0x0000,     /* R225 - DCDC/LDO status */ \
    0x0000,     /* R226 - Charger Overides/status */ \
    0x0000,     /* R227 - misc overrides */ \
    0x0000,     /* R228 - Supply overrides/status 1 */ \
    0x0000,     /* R229 - Supply overrides/status 2 */ \
    0xE000,     /* R230 - GPIO Pin Status */ \
    0x0000,     /* R231 - comparator overrides */ \
    0x0000,     /* R232 */ \
    0x0000,     /* R233 - State Machine status */ \
    0x0000,     /* R234 */ \
    0x0000,     /* R235 */ \
    0x0000,     /* R236 */ \
    0x0000,     /* R237 */ \
    0x0000,     /* R238 */ \
    0x0000,     /* R239 */ \
    0x0000,     /* R240 */ \
    0x0000,     /* R241 */ \
    0x0000,     /* R242 */ \
    0x0000,     /* R243 */ \
    0x0000,     /* R244 */ \
    0x0000,     /* R245 */ \
    0x0000,     /* R246 */ \
    0x0000,     /* R247 */ \
    0x1000,     /* R248 - DCDC1 Test Controls */ \
    0x0000,     /* R249 */ \
    0x1000,     /* R250 - DCDC3 Test Controls */ \
    0x1000,     /* R251 - DCDC4 Test Controls */ \
    0x0000,     /* R252 */ \
    0x1000,     /* R253 - DCDC6 Test Controls */ \
}

/* Bank 2 */
#define WM8350_REGISTER_DEFAULTS_2 \
{ \
    0x6143,     /* R0   - Reset/ID */ \
    0x3000,     /* R1   - ID */ \
    0x0000,     /* R2 */ \
    0x1002,     /* R3   - System Control 1 */ \
    0x0214,     /* R4   - System Control 2 */ \
    0x0000,     /* R5   - System Hibernate */ \
    0x8A00,     /* R6   - Interface Control */ \
    0x0000,     /* R7 */ \
    0x8000,     /* R8   - Power mgmt (1) */ \
    0x0000,     /* R9   - Power mgmt (2) */ \
    0x0000,     /* R10  - Power mgmt (3) */ \
    0x2000,     /* R11  - Power mgmt (4) */ \
    0x0E00,     /* R12  - Power mgmt (5) */ \
    0x0000,     /* R13  - Power mgmt (6) */ \
    0x0000,     /* R14  - Power mgmt (7) */ \
    0x0000,     /* R15 */ \
    0x0000,     /* R16  - RTC Seconds/Minutes */ \
    0x0100,     /* R17  - RTC Hours/Day */ \
    0x0101,     /* R18  - RTC Date/Month */ \
    0x1400,     /* R19  - RTC Year */ \
    0x0000,     /* R20  - Alarm Seconds/Minutes */ \
    0x0000,     /* R21  - Alarm Hours/Day */ \
    0x0000,     /* R22  - Alarm Date/Month */ \
    0x0320,     /* R23  - RTC Time Control */ \
    0x0000,     /* R24  - System Interrupts */ \
    0x0000,     /* R25  - Interrupt Status 1 */ \
    0x0000,     /* R26  - Interrupt Status 2 */ \
    0x0000,     /* R27 */ \
    0x0000,     /* R28  - Under Voltage Interrupt status */ \
    0x0000,     /* R29  - Over Current Interrupt status */ \
    0x0000,     /* R30  - GPIO Interrupt Status */ \
    0x0000,     /* R31  - Comparator Interrupt Status */ \
    0x3FFF,     /* R32  - System Interrupts Mask */ \
    0x0000,     /* R33  - Interrupt Status 1 Mask */ \
    0x0000,     /* R34  - Interrupt Status 2 Mask */ \
    0x0000,     /* R35 */ \
    0x0000,     /* R36  - Under Voltage Interrupt status Mask */ \
    0x0000,     /* R37  - Over Current Interrupt status Mask */ \
    0x0000,     /* R38  - GPIO Interrupt Status Mask */ \
    0x0000,     /* R39  - Comparator Interrupt Status Mask */ \
    0x0040,     /* R40  - Clock Control 1 */ \
    0x0000,     /* R41  - Clock Control 2 */ \
    0x3B00,     /* R42  - FLL Control 1 */ \
    0x7086,     /* R43  - FLL Control 2 */ \
    0xC226,     /* R44  - FLL Control 3 */ \
    0x0000,     /* R45  - FLL Control 4 */ \
    0x0000,     /* R46 */ \
    0x0000,     /* R47 */ \
    0x0000,     /* R48  - DAC Control */ \
    0x0000,     /* R49 */ \
    0x00C0,     /* R50  - DAC Digital Volume L */ \
    0x00C0,     /* R51  - DAC Digital Volume R */ \
    0x0000,     /* R52 */ \
    0x0040,     /* R53  - DAC LR Rate */ \
    0x0000,     /* R54  - DAC Clock Control */ \
    0x0000,     /* R55 */ \
    0x0000,     /* R56 */ \
    0x0000,     /* R57 */ \
    0x4000,     /* R58  - DAC Mute */ \
    0x0000,     /* R59  - DAC Mute Volume */ \
    0x0000,     /* R60  - DAC Side */ \
    0x0000,     /* R61 */ \
    0x0000,     /* R62 */ \
    0x0000,     /* R63 */ \
    0x8000,     /* R64  - ADC Control */ \
    0x0000,     /* R65 */ \
    0x00C0,     /* R66  - ADC Digital Volume L */ \
    0x00C0,     /* R67  - ADC Digital Volume R */ \
    0x0000,     /* R68  - ADC Divider */ \
    0x0000,     /* R69 */ \
    0x0040,     /* R70  - ADC LR Rate */ \
    0x0000,     /* R71 */ \
    0x0303,     /* R72  - Input Control */ \
    0x0000,     /* R73  - IN3 Input Control */ \
    0x0000,     /* R74  - Mic Bias Control */ \
    0x0000,     /* R75 */ \
    0x0000,     /* R76  - Output Control */ \
    0x0000,     /* R77  - Jack Detect */ \
    0x0000,     /* R78  - Anti Pop Control */ \
    0x0000,     /* R79 */ \
    0x0040,     /* R80  - Left Input Volume */ \
    0x0040,     /* R81  - Right Input Volume */ \
    0x0000,     /* R82 */ \
    0x0000,     /* R83 */ \
    0x0000,     /* R84 */ \
    0x0000,     /* R85 */ \
    0x0000,     /* R86 */ \
    0x0000,     /* R87 */ \
    0x0800,     /* R88  - Left Mixer Control */ \
    0x1000,     /* R89  - Right Mixer Control */ \
    0x0000,     /* R90 */ \
    0x0000,     /* R91 */ \
    0x0000,     /* R92  - OUT3 Mixer Control */ \
    0x0000,     /* R93  - OUT4 Mixer Control */ \
    0x0000,     /* R94 */ \
    0x0000,     /* R95 */ \
    0x0000,     /* R96  - Output Left Mixer Volume */ \
    0x0000,     /* R97  - Output Right Mixer Volume */ \
    0x0000,     /* R98  - Input Mixer Volume L */ \
    0x0000,     /* R99  - Input Mixer Volume R */ \
    0x0000,     /* R100 - Input Mixer Volume */ \
    0x0000,     /* R101 */ \
    0x0000,     /* R102 */ \
    0x0000,     /* R103 */ \
    0x00E4,     /* R104 - OUT1L Volume */ \
    0x00E4,     /* R105 - OUT1R Volume */ \
    0x00E4,     /* R106 - OUT2L Volume */ \
    0x02E4,     /* R107 - OUT2R Volume */ \
    0x0000,     /* R108 */ \
    0x0000,     /* R109 */ \
    0x0000,     /* R110 */ \
    0x0000,     /* R111 - BEEP Volume */ \
    0x0A00,     /* R112 - AI Formating */ \
    0x0000,     /* R113 - ADC DAC COMP */ \
    0x0020,     /* R114 - AI ADC Control */ \
    0x0020,     /* R115 - AI DAC Control */ \
    0x0000,     /* R116 */ \
    0x0000,     /* R117 */ \
    0x0000,     /* R118 */ \
    0x0000,     /* R119 */ \
    0x0000,     /* R120 */ \
    0x0000,     /* R121 */ \
    0x0000,     /* R122 */ \
    0x0000,     /* R123 */ \
    0x0000,     /* R124 */ \
    0x0000,     /* R125 */ \
    0x0000,     /* R126 */ \
    0x0000,     /* R127 */ \
    0x0812,     /* R128 - GPIO Debounce */ \
    0x0000,     /* R129 - GPIO Pin pull up Control */ \
    0x0110,     /* R130 - GPIO Pull down Control */ \
    0x0000,     /* R131 - GPIO Interrupt Mode */ \
    0x0000,     /* R132 */ \
    0x0000,     /* R133 - GPIO Control */ \
    0x09F2,     /* R134 - GPIO Configuration (i/o) */ \
    0x0DF6,     /* R135 - GPIO Pin Polarity / Type */ \
    0x0000,     /* R136 */ \
    0x0000,     /* R137 */ \
    0x0000,     /* R138 */ \
    0x0000,     /* R139 */ \
    0x0310,     /* R140 - GPIO Function Select 1 */ \
    0x0003,     /* R141 - GPIO Function Select 2 */ \
    0x2000,     /* R142 - GPIO Function Select 3 */ \
    0x0000,     /* R143 - GPIO Function Select 4 */ \
    0x0000,     /* R144 - Digitiser Control (1) */ \
    0x0002,     /* R145 - Digitiser Control (2) */ \
    0x0000,     /* R146 */ \
    0x0000,     /* R147 */ \
    0x0000,     /* R148 */ \
    0x0000,     /* R149 */ \
    0x0000,     /* R150 */ \
    0x0000,     /* R151 */ \
    0x7000,     /* R152 - AUX1 Readback */ \
    0x7000,     /* R153 - AUX2 Readback */ \
    0x7000,     /* R154 - AUX3 Readback */ \
    0x7000,     /* R155 - AUX4 Readback */ \
    0x0000,     /* R156 - USB Voltage Readback */ \
    0x0000,     /* R157 - LINE Voltage Readback */ \
    0x0000,     /* R158 - BATT Voltage Readback */ \
    0x0000,     /* R159 - Chip Temp Readback */ \
    0x0000,     /* R160 */ \
    0x0000,     /* R161 */ \
    0x0000,     /* R162 */ \
    0x0000,     /* R163 - Generic Comparator Control */ \
    0x0000,     /* R164 - Generic comparator 1 */ \
    0x0000,     /* R165 - Generic comparator 2 */ \
    0x0000,     /* R166 - Generic comparator 3 */ \
    0x0000,     /* R167 - Generic comparator 4 */ \
    0xA00F,     /* R168 - Battery Charger Control 1 */ \
    0x0B06,     /* R169 - Battery Charger Control 2 */ \
    0x0000,     /* R170 - Battery Charger Control 3 */ \
    0x0000,     /* R171 */ \
    0x0000,     /* R172 - Current Sink Driver A */ \
    0x0000,     /* R173 - CSA Flash control */ \
    0x0000,     /* R174 - Current Sink Driver B */ \
    0x0000,     /* R175 - CSB Flash control */ \
    0x0000,     /* R176 - DCDC/LDO requested */ \
    0x002D,     /* R177 - DCDC Active options */ \
    0x0000,     /* R178 - DCDC Sleep options */ \
    0x0025,     /* R179 - Power-check comparator */ \
    0x000E,     /* R180 - DCDC1 Control */ \
    0x0400,     /* R181 - DCDC1 Timeouts */ \
    0x1006,     /* R182 - DCDC1 Low Power */ \
    0x0018,     /* R183 - DCDC2 Control */ \
    0x0000,     /* R184 - DCDC2 Timeouts */ \
    0x0000,     /* R185 */ \
    0x002E,     /* R186 - DCDC3 Control */ \
    0x0000,     /* R187 - DCDC3 Timeouts */ \
    0x0006,     /* R188 - DCDC3 Low Power */ \
    0x000E,     /* R189 - DCDC4 Control */ \
    0x0000,     /* R190 - DCDC4 Timeouts */ \
    0x0006,     /* R191 - DCDC4 Low Power */ \
    0x0008,     /* R192 - DCDC5 Control */ \
    0x0000,     /* R193 - DCDC5 Timeouts */ \
    0x0000,     /* R194 */ \
    0x0026,     /* R195 - DCDC6 Control */ \
    0x0C00,     /* R196 - DCDC6 Timeouts */ \
    0x0006,     /* R197 - DCDC6 Low Power */ \
    0x0000,     /* R198 */ \
    0x0003,     /* R199 - Limit Switch Control */ \
    0x001C,     /* R200 - LDO1 Control */ \
    0x0000,     /* R201 - LDO1 Timeouts */ \
    0x001C,     /* R202 - LDO1 Low Power */ \
    0x0010,     /* R203 - LDO2 Control */ \
    0x0800,     /* R204 - LDO2 Timeouts */ \
    0x001C,     /* R205 - LDO2 Low Power */ \
    0x000A,     /* R206 - LDO3 Control */ \
    0x0C00,     /* R207 - LDO3 Timeouts */ \
    0x001C,     /* R208 - LDO3 Low Power */ \
    0x001A,     /* R209 - LDO4 Control */ \
    0x0000,     /* R210 - LDO4 Timeouts */ \
    0x001C,     /* R211 - LDO4 Low Power */ \
    0x0000,     /* R212 */ \
    0x0000,     /* R213 */ \
    0x0000,     /* R214 */ \
    0x0000,     /* R215 - VCC_FAULT Masks */ \
    0x001F,     /* R216 - Main Bandgap Control */ \
    0x0000,     /* R217 - OSC Control */ \
    0x9000,     /* R218 - RTC Tick Control */ \
    0x0000,     /* R219 - Security */ \
    0x0000,     /* R220 */ \
    0x0000,     /* R221 */ \
    0x0000,     /* R222 */ \
    0x0000,     /* R223 */ \
    0x0000,     /* R224 - Signal overrides */ \
    0x0000,     /* R225 - DCDC/LDO status */ \
    0x0000,     /* R226 - Charger Overides/status */ \
    0x0000,     /* R227 - misc overrides */ \
    0x0000,     /* R228 - Supply overrides/status 1 */ \
    0x0000,     /* R229 - Supply overrides/status 2 */ \
    0xE000,     /* R230 - GPIO Pin Status */ \
    0x0000,     /* R231 - comparator overrides */ \
    0x0000,     /* R232 */ \
    0x0000,     /* R233 - State Machine status */ \
    0x0000,     /* R234 */ \
    0x0000,     /* R235 */ \
    0x0000,     /* R236 */ \
    0x0000,     /* R237 */ \
    0x0000,     /* R238 */ \
    0x0000,     /* R239 */ \
    0x0000,     /* R240 */ \
    0x0000,     /* R241 */ \
    0x0000,     /* R242 */ \
    0x0000,     /* R243 */ \
    0x0000,     /* R244 */ \
    0x0000,     /* R245 */ \
    0x0000,     /* R246 */ \
    0x0000,     /* R247 */ \
    0x1000,     /* R248 - DCDC1 Test Controls */ \
    0x0000,     /* R249 */ \
    0x1000,     /* R250 - DCDC3 Test Controls */ \
    0x1000,     /* R251 - DCDC4 Test Controls */ \
    0x0000,     /* R252 */ \
    0x1000,     /* R253 - DCDC6 Test Controls */ \
}

/* Bank 3 */
#define WM8350_REGISTER_DEFAULTS_3 \
{ \
    0x6143,     /* R0   - Reset/ID */ \
    0x3000,     /* R1   - ID */ \
    0x0000,     /* R2 */ \
    0x1002,     /* R3   - System Control 1 */ \
    0x0214,     /* R4   - System Control 2 */ \
    0x0000,     /* R5   - System Hibernate */ \
    0x8A00,     /* R6   - Interface Control */ \
    0x0000,     /* R7 */ \
    0x8000,     /* R8   - Power mgmt (1) */ \
    0x0000,     /* R9   - Power mgmt (2) */ \
    0x0000,     /* R10  - Power mgmt (3) */ \
    0x2000,     /* R11  - Power mgmt (4) */ \
    0x0E00,     /* R12  - Power mgmt (5) */ \
    0x0000,     /* R13  - Power mgmt (6) */ \
    0x0000,     /* R14  - Power mgmt (7) */ \
    0x0000,     /* R15 */ \
    0x0000,     /* R16  - RTC Seconds/Minutes */ \
    0x0100,     /* R17  - RTC Hours/Day */ \
    0x0101,     /* R18  - RTC Date/Month */ \
    0x1400,     /* R19  - RTC Year */ \
    0x0000,     /* R20  - Alarm Seconds/Minutes */ \
    0x0000,     /* R21  - Alarm Hours/Day */ \
    0x0000,     /* R22  - Alarm Date/Month */ \
    0x0320,     /* R23  - RTC Time Control */ \
    0x0000,     /* R24  - System Interrupts */ \
    0x0000,     /* R25  - Interrupt Status 1 */ \
    0x0000,     /* R26  - Interrupt Status 2 */ \
    0x0000,     /* R27 */ \
    0x0000,     /* R28  - Under Voltage Interrupt status */ \
    0x0000,     /* R29  - Over Current Interrupt status */ \
    0x0000,     /* R30  - GPIO Interrupt Status */ \
    0x0000,     /* R31  - Comparator Interrupt Status */ \
    0x3FFF,     /* R32  - System Interrupts Mask */ \
    0x0000,     /* R33  - Interrupt Status 1 Mask */ \
    0x0000,     /* R34  - Interrupt Status 2 Mask */ \
    0x0000,     /* R35 */ \
    0x0000,     /* R36  - Under Voltage Interrupt status Mask */ \
    0x0000,     /* R37  - Over Current Interrupt status Mask */ \
    0x0000,     /* R38  - GPIO Interrupt Status Mask */ \
    0x0000,     /* R39  - Comparator Interrupt Status Mask */ \
    0x0040,     /* R40  - Clock Control 1 */ \
    0x0000,     /* R41  - Clock Control 2 */ \
    0x3B00,     /* R42  - FLL Control 1 */ \
    0x7086,     /* R43  - FLL Control 2 */ \
    0xC226,     /* R44  - FLL Control 3 */ \
    0x0000,     /* R45  - FLL Control 4 */ \
    0x0000,     /* R46 */ \
    0x0000,     /* R47 */ \
    0x0000,     /* R48  - DAC Control */ \
    0x0000,     /* R49 */ \
    0x00C0,     /* R50  - DAC Digital Volume L */ \
    0x00C0,     /* R51  - DAC Digital Volume R */ \
    0x0000,     /* R52 */ \
    0x0040,     /* R53  - DAC LR Rate */ \
    0x0000,     /* R54  - DAC Clock Control */ \
    0x0000,     /* R55 */ \
    0x0000,     /* R56 */ \
    0x0000,     /* R57 */ \
    0x4000,     /* R58  - DAC Mute */ \
    0x0000,     /* R59  - DAC Mute Volume */ \
    0x0000,     /* R60  - DAC Side */ \
    0x0000,     /* R61 */ \
    0x0000,     /* R62 */ \
    0x0000,     /* R63 */ \
    0x8000,     /* R64  - ADC Control */ \
    0x0000,     /* R65 */ \
    0x00C0,     /* R66  - ADC Digital Volume L */ \
    0x00C0,     /* R67  - ADC Digital Volume R */ \
    0x0000,     /* R68  - ADC Divider */ \
    0x0000,     /* R69 */ \
    0x0040,     /* R70  - ADC LR Rate */ \
    0x0000,     /* R71 */ \
    0x0303,     /* R72  - Input Control */ \
    0x0000,     /* R73  - IN3 Input Control */ \
    0x0000,     /* R74  - Mic Bias Control */ \
    0x0000,     /* R75 */ \
    0x0000,     /* R76  - Output Control */ \
    0x0000,     /* R77  - Jack Detect */ \
    0x0000,     /* R78  - Anti Pop Control */ \
    0x0000,     /* R79 */ \
    0x0040,     /* R80  - Left Input Volume */ \
    0x0040,     /* R81  - Right Input Volume */ \
    0x0000,     /* R82 */ \
    0x0000,     /* R83 */ \
    0x0000,     /* R84 */ \
    0x0000,     /* R85 */ \
    0x0000,     /* R86 */ \
    0x0000,     /* R87 */ \
    0x0800,     /* R88  - Left Mixer Control */ \
    0x1000,     /* R89  - Right Mixer Control */ \
    0x0000,     /* R90 */ \
    0x0000,     /* R91 */ \
    0x0000,     /* R92  - OUT3 Mixer Control */ \
    0x0000,     /* R93  - OUT4 Mixer Control */ \
    0x0000,     /* R94 */ \
    0x0000,     /* R95 */ \
    0x0000,     /* R96  - Output Left Mixer Volume */ \
    0x0000,     /* R97  - Output Right Mixer Volume */ \
    0x0000,     /* R98  - Input Mixer Volume L */ \
    0x0000,     /* R99  - Input Mixer Volume R */ \
    0x0000,     /* R100 - Input Mixer Volume */ \
    0x0000,     /* R101 */ \
    0x0000,     /* R102 */ \
    0x0000,     /* R103 */ \
    0x00E4,     /* R104 - OUT1L Volume */ \
    0x00E4,     /* R105 - OUT1R Volume */ \
    0x00E4,     /* R106 - OUT2L Volume */ \
    0x02E4,     /* R107 - OUT2R Volume */ \
    0x0000,     /* R108 */ \
    0x0000,     /* R109 */ \
    0x0000,     /* R110 */ \
    0x0000,     /* R111 - BEEP Volume */ \
    0x0A00,     /* R112 - AI Formating */ \
    0x0000,     /* R113 - ADC DAC COMP */ \
    0x0020,     /* R114 - AI ADC Control */ \
    0x0020,     /* R115 - AI DAC Control */ \
    0x0000,     /* R116 */ \
    0x0000,     /* R117 */ \
    0x0000,     /* R118 */ \
    0x0000,     /* R119 */ \
    0x0000,     /* R120 */ \
    0x0000,     /* R121 */ \
    0x0000,     /* R122 */ \
    0x0000,     /* R123 */ \
    0x0000,     /* R124 */ \
    0x0000,     /* R125 */ \
    0x0000,     /* R126 */ \
    0x0000,     /* R127 */ \
    0x0812,     /* R128 - GPIO Debounce */ \
    0x0000,     /* R129 - GPIO Pin pull up Control */ \
    0x0110,     /* R130 - GPIO Pull down Control */ \
    0x0000,     /* R131 - GPIO Interrupt Mode */ \
    0x0000,     /* R132 */ \
    0x0000,     /* R133 - GPIO Control */ \
    0x09F2,     /* R134 - GPIO Configuration (i/o) */ \
    0x0DF6,     /* R135 - GPIO Pin Polarity / Type */ \
    0x0000,     /* R136 */ \
    0x0000,     /* R137 */ \
    0x0000,     /* R138 */ \
    0x0000,     /* R139 */ \
    0x0310,     /* R140 - GPIO Function Select 1 */ \
    0x0003,     /* R141 - GPIO Function Select 2 */ \
    0x2000,     /* R142 - GPIO Function Select 3 */ \
    0x0000,     /* R143 - GPIO Function Select 4 */ \
    0x0000,     /* R144 - Digitiser Control (1) */ \
    0x0002,     /* R145 - Digitiser Control (2) */ \
    0x0000,     /* R146 */ \
    0x0000,     /* R147 */ \
    0x0000,     /* R148 */ \
    0x0000,     /* R149 */ \
    0x0000,     /* R150 */ \
    0x0000,     /* R151 */ \
    0x7000,     /* R152 - AUX1 Readback */ \
    0x7000,     /* R153 - AUX2 Readback */ \
    0x7000,     /* R154 - AUX3 Readback */ \
    0x7000,     /* R155 - AUX4 Readback */ \
    0x0000,     /* R156 - USB Voltage Readback */ \
    0x0000,     /* R157 - LINE Voltage Readback */ \
    0x0000,     /* R158 - BATT Voltage Readback */ \
    0x0000,     /* R159 - Chip Temp Readback */ \
    0x0000,     /* R160 */ \
    0x0000,     /* R161 */ \
    0x0000,     /* R162 */ \
    0x0000,     /* R163 - Generic Comparator Control */ \
    0x0000,     /* R164 - Generic comparator 1 */ \
    0x0000,     /* R165 - Generic comparator 2 */ \
    0x0000,     /* R166 - Generic comparator 3 */ \
    0x0000,     /* R167 - Generic comparator 4 */ \
    0xA00F,     /* R168 - Battery Charger Control 1 */ \
    0x0B06,     /* R169 - Battery Charger Control 2 */ \
    0x0000,     /* R170 - Battery Charger Control 3 */ \
    0x0000,     /* R171 */ \
    0x0000,     /* R172 - Current Sink Driver A */ \
    0x0000,     /* R173 - CSA Flash control */ \
    0x0000,     /* R174 - Current Sink Driver B */ \
    0x0000,     /* R175 - CSB Flash control */ \
    0x0000,     /* R176 - DCDC/LDO requested */ \
    0x002D,     /* R177 - DCDC Active options */ \
    0x0000,     /* R178 - DCDC Sleep options */ \
    0x0025,     /* R179 - Power-check comparator */ \
    0x000E,     /* R180 - DCDC1 Control */ \
    0x0400,     /* R181 - DCDC1 Timeouts */ \
    0x1006,     /* R182 - DCDC1 Low Power */ \
    0x0018,     /* R183 - DCDC2 Control */ \
    0x0000,     /* R184 - DCDC2 Timeouts */ \
    0x0000,     /* R185 */ \
    0x002E,     /* R186 - DCDC3 Control */ \
    0x0000,     /* R187 - DCDC3 Timeouts */ \
    0x0006,     /* R188 - DCDC3 Low Power */ \
    0x000E,     /* R189 - DCDC4 Control */ \
    0x0000,     /* R190 - DCDC4 Timeouts */ \
    0x0006,     /* R191 - DCDC4 Low Power */ \
    0x0008,     /* R192 - DCDC5 Control */ \
    0x0000,     /* R193 - DCDC5 Timeouts */ \
    0x0000,     /* R194 */ \
    0x0026,     /* R195 - DCDC6 Control */ \
    0x0C00,     /* R196 - DCDC6 Timeouts */ \
    0x0006,     /* R197 - DCDC6 Low Power */ \
    0x0000,     /* R198 */ \
    0x0003,     /* R199 - Limit Switch Control */ \
    0x001C,     /* R200 - LDO1 Control */ \
    0x0000,     /* R201 - LDO1 Timeouts */ \
    0x001C,     /* R202 - LDO1 Low Power */ \
    0x0010,     /* R203 - LDO2 Control */ \
    0x0800,     /* R204 - LDO2 Timeouts */ \
    0x001C,     /* R205 - LDO2 Low Power */ \
    0x000A,     /* R206 - LDO3 Control */ \
    0x0C00,     /* R207 - LDO3 Timeouts */ \
    0x001C,     /* R208 - LDO3 Low Power */ \
    0x001A,     /* R209 - LDO4 Control */ \
    0x0000,     /* R210 - LDO4 Timeouts */ \
    0x001C,     /* R211 - LDO4 Low Power */ \
    0x0000,     /* R212 */ \
    0x0000,     /* R213 */ \
    0x0000,     /* R214 */ \
    0x0000,     /* R215 - VCC_FAULT Masks */ \
    0x001F,     /* R216 - Main Bandgap Control */ \
    0x0000,     /* R217 - OSC Control */ \
    0x9000,     /* R218 - RTC Tick Control */ \
    0x0000,     /* R219 - Security */ \
    0x0000,     /* R220 */ \
    0x0000,     /* R221 */ \
    0x0000,     /* R222 */ \
    0x0000,     /* R223 */ \
    0x0000,     /* R224 - Signal overrides */ \
    0x0000,     /* R225 - DCDC/LDO status */ \
    0x0000,     /* R226 - Charger Overides/status */ \
    0x0000,     /* R227 - misc overrides */ \
    0x0000,     /* R228 - Supply overrides/status 1 */ \
    0x0000,     /* R229 - Supply overrides/status 2 */ \
    0xE000,     /* R230 - GPIO Pin Status */ \
    0x0000,     /* R231 - comparator overrides */ \
    0x0000,     /* R232 */ \
    0x0000,     /* R233 - State Machine status */ \
    0x0000,     /* R234 */ \
    0x0000,     /* R235 */ \
    0x0000,     /* R236 */ \
    0x0000,     /* R237 */ \
    0x0000,     /* R238 */ \
    0x0000,     /* R239 */ \
    0x0000,     /* R240 */ \
    0x0000,     /* R241 */ \
    0x0000,     /* R242 */ \
    0x0000,     /* R243 */ \
    0x0000,     /* R244 */ \
    0x0000,     /* R245 */ \
    0x0000,     /* R246 */ \
    0x0000,     /* R247 */ \
    0x1000,     /* R248 - DCDC1 Test Controls */ \
    0x0000,     /* R249 */ \
    0x1000,     /* R250 - DCDC3 Test Controls */ \
    0x1000,     /* R251 - DCDC4 Test Controls */ \
    0x0000,     /* R252 */ \
    0x1000,     /* R253 - DCDC6 Test Controls */ \
}

/*
 * Access masks.
 */
#ifndef _WMREGACCESS_DEFINED_
#define _WMREGACCESS_DEFINED_
typedef struct WMRegAccess
{
    unsigned short  readable;   /* Mask of readable bits */
    unsigned short  writable;   /* Mask of writable bits */
    unsigned short  vol;        /* Mask of volatile bits */
} WMRegAccess;
#endif

#define WM8350_ACCESS \
{  /*  read    write volatile */ \
    { 0xFFFF, 0xFFFF, 0x0000 }, /* R0   - Reset/ID */ \
    { 0x7CFF, 0x0000, 0x0000 }, /* R1   - ID */ \
    { 0x0000, 0x0000, 0x0000 }, /* R2 */ \
    { 0xFEBB, 0xFEBB, 0xC000 }, /* R3   - System Control 1 */ \
    { 0xFEB7, 0xFEB7, 0xF800 }, /* R4   - System Control 2 */ \
    { 0x80FF, 0x80FF, 0x8000 }, /* R5   - System Hibernate */ \
    { 0xFA0E, 0xFA0E, 0x0000 }, /* R6   - Interface Control */ \
    { 0x0000, 0x0000, 0x0000 }, /* R7 */ \
    { 0xE537, 0xE537, 0x0000 }, /* R8   - Power mgmt (1) */ \
    { 0x0FF3, 0x0FF3, 0x0000 }, /* R9   - Power mgmt (2) */ \
    { 0x008F, 0x008F, 0x0000 }, /* R10  - Power mgmt (3) */ \
    { 0x6D3C, 0x6D3C, 0x0000 }, /* R11  - Power mgmt (4) */ \
    { 0x1F8F, 0x1F8F, 0x0000 }, /* R12  - Power mgmt (5) */ \
    { 0x8F3F, 0x8F3F, 0x8F3F }, /* R13  - Power mgmt (6) */ \
    { 0x0003, 0x0003, 0x0000 }, /* R14  - Power mgmt (7) */ \
    { 0x0000, 0x0000, 0x0000 }, /* R15 */ \
    { 0x7F7F, 0x7F7F, 0x0000 }, /* R16  - RTC Seconds/Minutes */ \
    { 0x073F, 0x073F, 0x0000 }, /* R17  - RTC Hours/Day */ \
    { 0x1F3F, 0x1F3F, 0x0000 }, /* R18  - RTC Date/Month */ \
    { 0x3FFF, 0x00FF, 0x0000 }, /* R19  - RTC Year */ \
    { 0x7F7F, 0x7F7F, 0x0000 }, /* R20  - Alarm Seconds/Minutes */ \
    { 0x0F3F, 0x0F3F, 0x0000 }, /* R21  - Alarm Hours/Day */ \
    { 0x1F3F, 0x1F3F, 0x0000 }, /* R22  - Alarm Date/Month */ \
    { 0xCF7F, 0xCA7F, 0x0000 }, /* R23  - RTC Time Control */ \
    { 0x33FF, 0x0000, 0x0000 }, /* R24  - System Interrupts */ \
    { 0xFEE7, 0x0000, 0x0000 }, /* R25  - Interrupt Status 1 */ \
    { 0x35FF, 0x0000, 0x0000 }, /* R26  - Interrupt Status 2 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R27 */ \
    { 0x0F3F, 0x0000, 0x0000 }, /* R28  - Under Voltage Interrupt status */ \
    { 0x8000, 0x0000, 0x0000 }, /* R29  - Over Current Interrupt status */ \
    { 0x1FFF, 0x0000, 0x0000 }, /* R30  - GPIO Interrupt Status */ \
    { 0xEF7F, 0x0000, 0x0000 }, /* R31  - Comparator Interrupt Status */ \
    { 0x33FF, 0x33FF, 0x0000 }, /* R32  - System Interrupts Mask */ \
    { 0xFEE7, 0xFEE7, 0x0000 }, /* R33  - Interrupt Status 1 Mask */ \
    { 0x35FF, 0x35FF, 0x0000 }, /* R34  - Interrupt Status 2 Mask */ \
    { 0x0000, 0x0000, 0x0000 }, /* R35 */ \
    { 0x0F3F, 0x0F3F, 0x0000 }, /* R36  - Under Voltage Interrupt status Mask */ \
    { 0x8000, 0x8000, 0x0000 }, /* R37  - Over Current Interrupt status Mask */ \
    { 0x1FFF, 0x1FFF, 0x0000 }, /* R38  - GPIO Interrupt Status Mask */ \
    { 0xEF7F, 0xEF7F, 0x0000 }, /* R39  - Comparator Interrupt Status Mask */ \
    { 0xC9F7, 0xC9F7, 0x0000 }, /* R40  - Clock Control 1 */ \
    { 0x8001, 0x8001, 0x0000 }, /* R41  - Clock Control 2 */ \
    { 0xFFF7, 0xFFF7, 0x0000 }, /* R42  - FLL Control 1 */ \
    { 0xFBFF, 0xFBFF, 0x0000 }, /* R43  - FLL Control 2 */ \
    { 0xFFFF, 0xFFFF, 0x0000 }, /* R44  - FLL Control 3 */ \
    { 0x0033, 0x0033, 0x0000 }, /* R45  - FLL Control 4 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R46 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R47 */ \
    { 0x303B, 0x303B, 0x0000 }, /* R48  - DAC Control */ \
    { 0x0000, 0x0000, 0x0000 }, /* R49 */ \
    { 0x81FF, 0x81FF, 0x0000 }, /* R50  - DAC Digital Volume L */ \
    { 0x81FF, 0x81FF, 0x0000 }, /* R51  - DAC Digital Volume R */ \
    { 0x0000, 0x0000, 0x0000 }, /* R52 */ \
    { 0x0FFF, 0x0FFF, 0x0000 }, /* R53  - DAC LR Rate */ \
    { 0x0017, 0x0017, 0x0000 }, /* R54  - DAC Clock Control */ \
    { 0x0000, 0x0000, 0x0000 }, /* R55 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R56 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R57 */ \
    { 0x4000, 0x4000, 0x0000 }, /* R58  - DAC Mute */ \
    { 0x7000, 0x7000, 0x0000 }, /* R59  - DAC Mute Volume */ \
    { 0x3C00, 0x3C00, 0x0000 }, /* R60  - DAC Side */ \
    { 0x0000, 0x0000, 0x0000 }, /* R61 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R62 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R63 */ \
    { 0x8303, 0x8303, 0x0000 }, /* R64  - ADC Control */ \
    { 0x0000, 0x0000, 0x0000 }, /* R65 */ \
    { 0x81FF, 0x81FF, 0x0000 }, /* R66  - ADC Digital Volume L */ \
    { 0x81FF, 0x81FF, 0x0000 }, /* R67  - ADC Digital Volume R */ \
    { 0x0FFF, 0x0FFF, 0x0000 }, /* R68  - ADC Divider */ \
    { 0x0000, 0x0000, 0x0000 }, /* R69 */ \
    { 0x0FFF, 0x0FFF, 0x0000 }, /* R70  - ADC LR Rate */ \
    { 0x0000, 0x0000, 0x0000 }, /* R71 */ \
    { 0x0707, 0x0707, 0x0000 }, /* R72  - Input Control */ \
    { 0xC0C0, 0xC0C0, 0x0000 }, /* R73  - IN3 Input Control */ \
    { 0xC09F, 0xC09F, 0x0000 }, /* R74  - Mic Bias Control */ \
    { 0x0000, 0x0000, 0x0000 }, /* R75 */ \
    { 0x0F15, 0x0F15, 0x0000 }, /* R76  - Output Control */ \
    { 0xC000, 0xC000, 0x0000 }, /* R77  - Jack Detect */ \
    { 0x03FF, 0x03FF, 0x0000 }, /* R78  - Anti Pop Control */ \
    { 0x0000, 0x0000, 0x0000 }, /* R79 */ \
    { 0xE1FC, 0xE1FC, 0x0000 }, /* R80  - Left Input Volume */ \
    { 0xE1FC, 0xE1FC, 0x0000 }, /* R81  - Right Input Volume */ \
    { 0x0000, 0x0000, 0x0000 }, /* R82 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R83 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R84 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R85 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R86 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R87 */ \
    { 0x9807, 0x9807, 0x0000 }, /* R88  - Left Mixer Control */ \
    { 0x980B, 0x980B, 0x0000 }, /* R89  - Right Mixer Control */ \
    { 0x0000, 0x0000, 0x0000 }, /* R90 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R91 */ \
    { 0x8909, 0x8909, 0x0000 }, /* R92  - OUT3 Mixer Control */ \
    { 0x9E07, 0x9E07, 0x0000 }, /* R93  - OUT4 Mixer Control */ \
    { 0x0000, 0x0000, 0x0000 }, /* R94 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R95 */ \
    { 0x0EEE, 0x0EEE, 0x0000 }, /* R96  - Output Left Mixer Volume */ \
    { 0xE0EE, 0xE0EE, 0x0000 }, /* R97  - Output Right Mixer Volume */ \
    { 0x0E0F, 0x0E0F, 0x0000 }, /* R98  - Input Mixer Volume L */ \
    { 0xE0E1, 0xE0E1, 0x0000 }, /* R99  - Input Mixer Volume R */ \
    { 0x800E, 0x800E, 0x0000 }, /* R100 - Input Mixer Volume */ \
    { 0x0000, 0x0000, 0x0000 }, /* R101 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R102 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R103 */ \
    { 0xE1FC, 0xE1FC, 0x0000 }, /* R104 - OUT1L Volume */ \
    { 0xE1FC, 0xE1FC, 0x0000 }, /* R105 - OUT1R Volume */ \
    { 0xE1FC, 0xE1FC, 0x0000 }, /* R106 - OUT2L Volume */ \
    { 0xE7FC, 0xE7FC, 0x0000 }, /* R107 - OUT2R Volume */ \
    { 0x0000, 0x0000, 0x0000 }, /* R108 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R109 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R110 */ \
    { 0x80E0, 0x80E0, 0x0000 }, /* R111 - BEEP Volume */ \
    { 0xBF00, 0xBF00, 0x0000 }, /* R112 - AI Formating */ \
    { 0x00F1, 0x00F1, 0x0000 }, /* R113 - ADC DAC COMP */ \
    { 0x00F8, 0x00F8, 0x0000 }, /* R114 - AI ADC Control */ \
    { 0x40FB, 0x40FB, 0x0000 }, /* R115 - AI DAC Control */ \
    { 0x0000, 0x0000, 0x0000 }, /* R116 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R117 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R118 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R119 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R120 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R121 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R122 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R123 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R124 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R125 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R126 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R127 */ \
    { 0x1FFF, 0x1FFF, 0x0000 }, /* R128 - GPIO Debounce */ \
    { 0x1FFF, 0x1FFF, 0x0000 }, /* R129 - GPIO Pin pull up Control */ \
    { 0x1FFF, 0x1FFF, 0x0000 }, /* R130 - GPIO Pull down Control */ \
    { 0x1FFF, 0x1FFF, 0x0000 }, /* R131 - GPIO Interrupt Mode */ \
    { 0x0000, 0x0000, 0x0000 }, /* R132 */ \
    { 0x00C0, 0x00C0, 0x0000 }, /* R133 - GPIO Control */ \
    { 0x1FFF, 0x1FFF, 0x0000 }, /* R134 - GPIO Configuration (i/o) */ \
    { 0x1FFF, 0x1FFF, 0x0000 }, /* R135 - GPIO Pin Polarity / Type */ \
    { 0x0000, 0x0000, 0x0000 }, /* R136 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R137 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R138 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R139 */ \
    { 0xFFFF, 0xFFFF, 0x0000 }, /* R140 - GPIO Function Select 1 */ \
    { 0xFFFF, 0xFFFF, 0x0000 }, /* R141 - GPIO Function Select 2 */ \
    { 0xFFFF, 0xFFFF, 0x0000 }, /* R142 - GPIO Function Select 3 */ \
    { 0x000F, 0x000F, 0x0000 }, /* R143 - GPIO Function Select 4 */ \
    { 0xD0FF, 0xD0FF, 0x2000 }, /* R144 - Digitiser Control (1) */ \
    { 0x3707, 0x3707, 0x0000 }, /* R145 - Digitiser Control (2) */ \
    { 0x0000, 0x0000, 0x0000 }, /* R146 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R147 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R148 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R149 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R150 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R151 */ \
    { 0x7FFF, 0x7000, 0x0000 }, /* R152 - AUX1 Readback */ \
    { 0x7FFF, 0x7000, 0x0000 }, /* R153 - AUX2 Readback */ \
    { 0x7FFF, 0x7000, 0x0000 }, /* R154 - AUX3 Readback */ \
    { 0x7FFF, 0x7000, 0x0000 }, /* R155 - AUX4 Readback */ \
    { 0x0FFF, 0x0000, 0x0000 }, /* R156 - USB Voltage Readback */ \
    { 0x0FFF, 0x0000, 0x0000 }, /* R157 - LINE Voltage Readback */ \
    { 0x0FFF, 0x0000, 0x0000 }, /* R158 - BATT Voltage Readback */ \
    { 0x0FFF, 0x0000, 0x0000 }, /* R159 - Chip Temp Readback */ \
    { 0x0000, 0x0000, 0x0000 }, /* R160 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R161 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R162 */ \
    { 0x000F, 0x000F, 0x0000 }, /* R163 - Generic Comparator Control */ \
    { 0xFFFF, 0xFFFF, 0x0000 }, /* R164 - Generic comparator 1 */ \
    { 0xFFFF, 0xFFFF, 0x0000 }, /* R165 - Generic comparator 2 */ \
    { 0xFFFF, 0xFFFF, 0x0000 }, /* R166 - Generic comparator 3 */ \
    { 0xFFFF, 0xFFFF, 0x0000 }, /* R167 - Generic comparator 4 */ \
    { 0xBFFF, 0xBFFF, 0x0020 }, /* R168 - Battery Charger Control 1 */ \
    { 0xFFFF, 0x4FFF, 0xB000 }, /* R169 - Battery Charger Control 2 */ \
    { 0x006F, 0x006F, 0x0000 }, /* R170 - Battery Charger Control 3 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R171 */ \
    { 0x903F, 0x903F, 0x0000 }, /* R172 - Current Sink Driver A */ \
    { 0xE333, 0xE333, 0x2000 }, /* R173 - CSA Flash control */ \
    { 0x903F, 0x903F, 0x0000 }, /* R174 - Current Sink Driver B */ \
    { 0xE333, 0xE333, 0x2000 }, /* R175 - CSB Flash control */ \
    { 0x8F3F, 0x8F3F, 0x8F3F }, /* R176 - DCDC/LDO requested */ \
    { 0xB32D, 0xB32D, 0x0000 }, /* R177 - DCDC Active options */ \
    { 0x002D, 0x002D, 0x0000 }, /* R178 - DCDC Sleep options */ \
    { 0x5077, 0x5077, 0x0000 }, /* R179 - Power-check comparator */ \
    { 0x0C7F, 0x0C7F, 0x0000 }, /* R180 - DCDC1 Control */ \
    { 0xFFC0, 0xFFC0, 0x0000 }, /* R181 - DCDC1 Timeouts */ \
    { 0x737F, 0x737F, 0x0000 }, /* R182 - DCDC1 Low Power */ \
    { 0x535B, 0x535B, 0x0000 }, /* R183 - DCDC2 Control */ \
    { 0xFFC0, 0xFFC0, 0x0000 }, /* R184 - DCDC2 Timeouts */ \
    { 0x0000, 0x0000, 0x0000 }, /* R185 */ \
    { 0x0C7F, 0x0C7F, 0x0000 }, /* R186 - DCDC3 Control */ \
    { 0xFFC0, 0xFFC0, 0x0000 }, /* R187 - DCDC3 Timeouts */ \
    { 0x737F, 0x737F, 0x0000 }, /* R188 - DCDC3 Low Power */ \
    { 0x0C7F, 0x0C7F, 0x0000 }, /* R189 - DCDC4 Control */ \
    { 0xFFC0, 0xFFC0, 0x0000 }, /* R190 - DCDC4 Timeouts */ \
    { 0x737F, 0x737F, 0x0000 }, /* R191 - DCDC4 Low Power */ \
    { 0x535B, 0x535B, 0x0000 }, /* R192 - DCDC5 Control */ \
    { 0xFFC0, 0xFFC0, 0x0000 }, /* R193 - DCDC5 Timeouts */ \
    { 0x0000, 0x0000, 0x0000 }, /* R194 */ \
    { 0x0C7F, 0x0C7F, 0x0000 }, /* R195 - DCDC6 Control */ \
    { 0xFFC0, 0xFFC0, 0x0000 }, /* R196 - DCDC6 Timeouts */ \
    { 0x737F, 0x737F, 0x0000 }, /* R197 - DCDC6 Low Power */ \
    { 0x0000, 0x0000, 0x0000 }, /* R198 */ \
    { 0xFFD3, 0xFFD3, 0x0000 }, /* R199 - Limit Switch Control */ \
    { 0x441F, 0x441F, 0x0000 }, /* R200 - LDO1 Control */ \
    { 0xFFC0, 0xFFC0, 0x0000 }, /* R201 - LDO1 Timeouts */ \
    { 0x331F, 0x331F, 0x0000 }, /* R202 - LDO1 Low Power */ \
    { 0x441F, 0x441F, 0x0000 }, /* R203 - LDO2 Control */ \
    { 0xFFC0, 0xFFC0, 0x0000 }, /* R204 - LDO2 Timeouts */ \
    { 0x331F, 0x331F, 0x0000 }, /* R205 - LDO2 Low Power */ \
    { 0x441F, 0x441F, 0x0000 }, /* R206 - LDO3 Control */ \
    { 0xFFC0, 0xFFC0, 0x0000 }, /* R207 - LDO3 Timeouts */ \
    { 0x331F, 0x331F, 0x0000 }, /* R208 - LDO3 Low Power */ \
    { 0x441F, 0x441F, 0x0000 }, /* R209 - LDO4 Control */ \
    { 0xFFC0, 0xFFC0, 0x0000 }, /* R210 - LDO4 Timeouts */ \
    { 0x331F, 0x331F, 0x0000 }, /* R211 - LDO4 Low Power */ \
    { 0x0000, 0x0000, 0x0000 }, /* R212 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R213 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R214 */ \
    { 0x8F3F, 0x8F3F, 0x0000 }, /* R215 - VCC_FAULT Masks */ \
    { 0x8000, 0x8000, 0x0000 }, /* R216 - Main Bandgap Control */ \
    { 0x8000, 0x8000, 0x0000 }, /* R217 - OSC Control */ \
    { 0xF3FF, 0xB3FF, 0x0000 }, /* R218 - RTC Tick Control */ \
    { 0x0000, 0xFFFF, 0x0000 }, /* R219 - Security */ \
    { 0x0000, 0x0000, 0x0000 }, /* R220 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R221 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R222 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R223 */ \
    { 0x0FCF, 0x0FCF, 0x0000 }, /* R224 - Signal overrides */ \
    { 0x8F3F, 0x0000, 0x0000 }, /* R225 - DCDC/LDO status */ \
    { 0xC807, 0xC807, 0x0000 }, /* R226 - Charger Overides/status */ \
    { 0x34FE, 0x34FE, 0x0000 }, /* R227 - misc overrides */ \
    { 0x002D, 0x002D, 0x0000 }, /* R228 - Supply overrides/status 1 */ \
    { 0x8F3F, 0x8F3F, 0x0000 }, /* R229 - Supply overrides/status 2 */ \
    { 0xFFFF, 0x1FFF, 0x0000 }, /* R230 - GPIO Pin Status */ \
    { 0xEF00, 0xEF00, 0x0000 }, /* R231 - comparator overrides */ \
    { 0x0000, 0x0000, 0x0000 }, /* R232 */ \
    { 0x077F, 0x0000, 0x0000 }, /* R233 - State Machine status */ \
    { 0x0000, 0x0000, 0x0000 }, /* R234 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R235 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R236 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R237 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R238 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R239 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R240 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R241 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R242 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R243 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R244 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R245 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R246 */ \
    { 0x0000, 0x0000, 0x0000 }, /* R247 */ \
    { 0x0010, 0x0010, 0x0000 }, /* R248 - DCDC1 Test Controls */ \
    { 0x0000, 0x0000, 0x0000 }, /* R249 */ \
    { 0x0010, 0x0010, 0x0000 }, /* R250 - DCDC3 Test Controls */ \
    { 0x0010, 0x0010, 0x0000 }, /* R251 - DCDC4 Test Controls */ \
    { 0x0000, 0x0000, 0x0000 }, /* R252 */ \
    { 0x0010, 0x0010, 0x0000 }, /* R253 - DCDC6 Test Controls */ \
}

#endif    /* __WM8350REGISTERDEFS_H__ */
/*------------------------------ END OF FILE ---------------------------------*/
