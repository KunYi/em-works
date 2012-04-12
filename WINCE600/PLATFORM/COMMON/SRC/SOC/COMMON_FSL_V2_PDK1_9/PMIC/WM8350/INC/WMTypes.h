/*******************************************************************************
 *
 * Copyright (c) 2003-2007 Wolfson Microelectronics plc.  All rights reserved.
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
 * @file   WMTypes.h
 * @brief  Type definitions
 *
 * @version $Id: WMTypes.h 640 2007-06-15 22:01:17Z ib $
 *
 * @Warning
 *   This software is specifically written for Wolfson devices. It may not be
 *   used with other devices.
 *
 ******************************************************************************/
#ifndef __WMTYPES_H__
#define __WMTYPES_H__

/*
 * Definitions
 */

/******************************************************************************
 * Macro: ARRAY_SIZE                                                    *//**
 *
 * @brief  Return the size of an array.
 *
 * @param _array          The array.
 *
 * @return int            Size of the array.
 *****************************************************************************/
#define ARRAY_SIZE(_array)   (sizeof(_array)/sizeof(_array[0]))


/******************************************************************************
* Macro: RANGE_CHECK                                                      *//**
*
* @brief  Return TRUE if within range, FALSE if out of range.
*
* @param _min            The minimum value of the range.
* @param _max            The maximum value of the range.
* @param _value          The value to check.
*
* @return BOOL           TRUE if within range, FALSE if out of range.
*****************************************************************************/
#define RANGE_CHECK( _min, _max, _value )   (BOOL) ( ( _value >= _min ) && \
                                                     ( _value <= _max )    \
                                                   )
/******************************************************************************/
/**
 ** Wolfson handle types and defines.
 **
 ******************************************************************************/
typedef void *WM_HANDLE;
typedef WM_HANDLE WM_DEVICE_HANDLE;
#define WM_HANDLE_INVALID   ((WM_DEVICE_HANDLE) 0)

/******************************************************************************/
/**
 ** Wolfson device register types and defines.
 **
 ******************************************************************************/

/**
 * @typedef WM_REGTYPE
 * Register typedef.
 */
typedef unsigned short          WM_REG;
typedef WM_REG                  WM_REGTYPE;

/**
 * @typedef WM_REGVAL
 * register value typedef.
 */
typedef unsigned short          WM_REGVAL;

/**
 * @typedef WM_REGFIELD
 * A structure to represent a field within a register.
 */
typedef struct WM_REGFIELD
{
    WM_REG          reg;    /**< register */
    WM_REGVAL       mask;   /**< bitmask of field */
    unsigned int    shift;  /**< number of bits to shift to get field value */
} WM_REGFIELD;

/**
 * @def WM_REG_INVALID
 * Invalid register define.
 */
#define WM_REG_INVALID          ((WM_REGTYPE)-1)

/**
 * @def WM_REG_INVALID_NAME
 * Invalid register name.
 */
#define WM_REG_INVALID_NAME     TEXT("INVALID REGISTER")

/* Chip IDs */
typedef unsigned short WM_CHIPTYPE;
#define WOLFSON_MICRO               0x574D  /* 'WM' */
#define WM_CHIP_UNSUPPORTED         0
#define WM_CHIP_UNKNOWN             0
#define WM_CHIP_WM9703              0x4C03
#define WM_CHIP_WM9704              0x4C04
#define WM_CHIP_WM9705              0x4C05
#define WM_CHIP_WM9707              WM_CHIP_WM9703
#define WM_CHIP_WM9708              WM_CHIP_WM9703
#define WM_CHIP_WM9709              0x4C09
#define WM_CHIP_WM9710              WM_CHIP_WM9705
#define WM_CHIP_WM9711              WM_CHIP_WM9712
#define WM_CHIP_WM9712              0x4C12
#define WM_CHIP_WM9713              0x4C13
#define WM_CHIP_WM9714              WM_CHIP_WM9713
#define WM_CHIP_WM9717              WM_CHIP_WM9703
#define WM_CHIP_WM8350              0x8350
#define WM_CHIP_WM8711              0x8711
#define WM_CHIP_WM8721              WM_CHIP_WM8711
#define WM_CHIP_WM8731              0x8731
#define WM_CHIP_WM8734              WM_CHIP_WM8731
#define WM_CHIP_WM8753              0x8753

#define WM_CHIP_STRING( _ID )   (                                              \
    ( WM_CHIP_WM9713 == (_ID) )             ? "WM9713/WM9714"                : \
    ( WM_CHIP_WM9712 == (_ID) )             ? "WM9711/WM9712"                : \
    ( WM_CHIP_WM9705 == (_ID) )             ? "WM9705/WM9710"                : \
    ( WM_CHIP_WM9703 == (_ID) )             ? "WM9703/WM9707/WM9708/WM9717"  : \
    ( WM_CHIP_WM8753 == (_ID) )             ? "WM8753"                       : \
    ( WM_CHIP_WM8731 == (_ID) )             ? "WM8731/WM8734"                : \
    ( WM_CHIP_WM8711 == (_ID) )             ? "WM8711/WM8721"                : \
    ( WM_CHIP_WMI2S_HW_ONLY == (_ID) )      ? "Wolfson I2S HW control"       : \
    ( WM_CHIP_WMI2S_7BIT_ADDR == (_ID) )    ? "Wolfson I2S 7-bit addr"       : \
    ( WM_CHIP_WMI2S_8BIT_ADDR == (_ID) )    ? "Wolfson I2S 8-bit addr"       : \
                                              "unknown device" )

#define WM_CHIP_IS_AC97( _ID )      ( 0x4C00 == ( (_ID) & 0xFF00 ) || 0x9700 == (_ID) )
#define WM_CHIP_HAS_READBACK( _ID ) ( WM_CHIP_IS_AC97( _ID ) || WM_DEV_I2S == (_ID) )
#define WM_CHIP_IS_HW_ONLY( _ID )   ( WM_CHIP_WMI2S_HW_ONLY == (_ID) )

/* Chip revisions */
typedef unsigned short WM_CHIPREV;
#define WM_REV_UNKNOWN          0
#define WM_REV_A                'A'
#define WM_REV_B                'B'
#define WM_REV_C                'C'
#define WM_REV_D                'D'
#define WM_REV_E                'E'


/******************************************************************************/
/**
 ** Wolfson driver ID types and defines.
 **
 ******************************************************************************/

/**
 * @typedef WM_DRIVER_ID
 * Driver ID typedef.
 */
typedef unsigned int WM_DRIVER_ID;

/**
 * @def WM_DRIVER_AUDIO
 * Audio driver ID
 */
#define WM_DRIVER_AUDIO                         0
/**
 * @def WM_DRIVER_TOUCH
 * Touch driver ID
 */
#define WM_DRIVER_TOUCH                         1
/**
 * @def WM_DRIVER_AUXADC
 * AuxADC driver ID
 */
#define WM_DRIVER_AUXADC                        2
/**
 * @def WM_DRIVER_BATTERY
 * Battery driver ID
 */
#define WM_DRIVER_BATTERY                       3
/**
 * @def WM_DRIVER_ACLINK
 * AC Link driver ID
 */
#define WM_DRIVER_ACLINK                        4
/**
 * @def WM_DRIVER_I2C
 * I2C driver ID
 */
#define WM_DRIVER_I2C                           5
/**
 * @def WM_DRIVER_DEVICE_GPIO
 * Device GPIO driver ID
 */
#define WM_DRIVER_DEVICE_GPIO                   6
/**
 * @def WM_DRIVER_DEVICE_DMA
 * DMA driver ID
 */
#define WM_DRIVER_DEVICE_DMA                    7
/**
 * @def WM_DRIVER_PMIC
 * PMIC driver ID
 */
#define WM_DRIVER_PMIC                          8
/**
 * @def WM_DRIVER_DEVICE_UNKNOWN
 * Unknown driver ID
 */
#define WM_DRIVER_DEVICE_UNKNOWN                0xFFFF

                                                
/**
 * @def WM_DRIVER_MASK
 * driver ID bit mask macro.
 * Takes the driver ID as it's parameter
 */
#define WM_DRIVER_MASK(_driver)                 (1U<<(_driver))

/******************************************************************************/
/**
 ** Wolfson device ID types and defines.
 **
 ******************************************************************************/

/**
 * @typedef WM_DEVICE_ID
 * Device ID typedef.
 */
typedef unsigned int WM_DEVICE_ID;

/**
 * @def  WM_DEV_AC97_PRIMARY
 * AC'97 primary device.
 */
#define WM_DEV_AC97_PRIMARY                 0
/**
 * @def  WM_DEV_AC97_SECONDARY
 * AC'97 secondary device.
 */
#define WM_DEV_AC97_SECONDARY               1
/**
 * @def  WM_DEV_I2S_PRIMARY
 * I2S primary device.
 */
#define WM_DEV_I2S_PRIMARY                  2
/**
 * @def  WM_DEV_I2S_SECONDARY
 * I2S secondary device.
 */
#define WM_DEV_I2S_SECONDARY                3
/**
 * @def  WM_DEV_UNKNOWN_DEVICE
 * Unknown device.
 */
#define WM_DEV_UNKNOWN_DEVICE                0xFFFF

/* 2 Wire device Address */
typedef unsigned short WM_2WIRE_ADDR;

#endif  /* __WMTYPES_H__ */
/*------------------------------ END OF FILE ---------------------------------*/
