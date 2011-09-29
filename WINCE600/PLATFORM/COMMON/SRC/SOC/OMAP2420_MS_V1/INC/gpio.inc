;
; Copyright (c) Microsoft Corporation.  All rights reserved.
;
;
; Use of this sample source code is subject to the terms of the Microsoft
; license agreement under which you licensed this sample source code. If
; you did not accept the terms of the license agreement, you are not
; authorized to use this sample source code. For the terms of the license,
; please see the license agreement between you and Microsoft or, if applicable,
; see the LICENSE.RTF on your install media or the root of your tools installation.
; THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
;
;//
;//
;//
;// Use of this source code is subject to the terms of the Microsoft end-user
;// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
;// If you did not accept the terms of the EULA, you are not authorized to use
;// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
;// install media.
;//
;//------------------------------------------------------------------------------
;//
;//  File:  gpio.h
;//
;//  This header defines interface for GPIO device driver. This driver control
;//  GPIO pins on hardware. It allows abstract GPIO interface and break up
;//  physicall and logical pins. To avoid overhead involved the driver exposes
;//  interface which allows obtain funtion pointers to base set/clr/get etc.
;//  functions.
;//
;//------------------------------------------------------------------------------
;//
;//  Define:  GPIO_xxx
;//
;//  Following defines logical GPIO pins. Mapping to physical GPIO pin is
;//  defined in bsp_gpio_cfg.h file.
;//
GPIO_OTG_TRANS                  EQU 0
GPIO_BATTERY_CHARGING           EQU 1
GPIO_KEYPAD_BACKLIGHT           EQU 2
GPIO_DISPLAY_BACKLIGHT          EQU 3
GPIO_HEADSET_DETECT             EQU 4
GPIO_NAND_READY                 EQU 5
GPIO_ROLLER_TRIG                EQU 6
GPIO_ROLLER_DIR                 EQU 7
GPIO_BATTERY_CHG_EN             EQU 8
GPIO_BATTERY_CHG_HI             EQU 9
GPIO_VIBRATOR_EN		EQU 10
GPIO_PNDTBUS_DATA		EQU 11
GPIO_PNDTBUS_CLK		EQU 12

GPIO_COUNT                      EQU 13

;//------------------------------------------------------------------------------
;//
;//  Global:  g_gpioMap
;//
;//  Global array mapping logical to physical GPIO.
;//

;//------------------------------------------------------------------------------
;//
;//  Define:  GPIO_DIR_xxx/GPIO_INT_xxx
;//
;//  Following defines config values for GPIO pins.
;//
GPIO_DIR_INPUT          EQU 0x01
GPIO_DIR_OUTPUT         EQU 0x00
GPIO_INT_LOW_HIGH       EQU 0x02
GPIO_INT_HIGH_LOW       EQU 0x00

;//------------------------------------------------------------------------------
;//
;//  Define:  GPIO_DEVICE_NAME
;//
GPIO_DEVICE_NAME        EQU L"GIO1:"

;//------------------------------------------------------------------------------
;//
;//  GUID:  DEVICE_IFC_GPIO_GUID
;//

;//------------------------------------------------------------------------------
;//
;//  Type:  DEVICE_IFC_GPIO
;//
;//  This structure is used to obtain GPIO interface funtion pointers used for
;//  in-process calls via IOCTL_DDK_GET_DRIVER_IFC.
;//

;//------------------------------------------------------------------------------
;//
;//  Type:  DEVICE_CONTEXT_GPIO
;//
;//  This structure is used to store GPIO device context.
;//

;//------------------------------------------------------------------------------

;IOCTL_GPIO_SETBIT       EQU \

;IOCTL_GPIO_CLRBIT       EQU \

;IOCTL_GPIO_UPDATEBIT    EQU \

;IOCTL_GPIO_GETBIT       EQU \

;IOCTL_GPIO_SETMODE      EQU \

;IOCTL_GPIO_GETMODE      EQU \

;IOCTL_GPIO_GETIRQ       EQU \

;//------------------------------------------------------------------------------



;// Allocate memory for our handler...

;// Get function pointers, fail when IOCTL isn't supported...

;// Save device handle










;//------------------------------------------------------------------------------

	END
