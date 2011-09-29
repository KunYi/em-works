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
;// Portions Copyright (c) Texas Instruments.  All rights reserved.
;//
;//------------------------------------------------------------------------------
;//
;//  File:  omap2420_gpio.h
;//
;//  This header file is comprised of GPIO module register details defined as
;//  structures and macros for configuring and controlling GPIO module.

;//------------------------------------------------------------------------------


;// All the GPIO modules(1-4) have a common structure GPIOREGS for
;// the various registers control.

OMAP2420_GPIO_REVISION_OA				EQU 0x00
OMAP2420_GPIO_SYSCONFIG					EQU 0x10
OMAP2420_GPIO_SYSSTAUS					EQU 0x14
OMAP2420_GPIO_IRQSTATUS1				EQU 0x18
OMAP2420_GPIO_IRQENABLE1				EQU 0x1C
OMAP2420_GPIO_WAKEUPENABLE				EQU 0x20
OMAP2420_GPIO_IRQSTATUS2				EQU 0x28
OMAP2420_GPIO_IRQENABLE2				EQU 0x2C
OMAP2420_GPIO_CTRL						EQU 0x30
OMAP2420_GPIO_OE						EQU 0x34
OMAP2420_GPIO_DATAIN					EQU 0x38
OMAP2420_GPIO_DATAOUT					EQU 0x3C
OMAP2420_GPIO_LEVELDETECT0				EQU 0x40
OMAP2420_GPIO_LEVELDETECT1				EQU 0x44
OMAP2420_GPIO_RISINGDETECT				EQU 0x48
OMAP2420_GPIO_FALLINGDETECT				EQU 0x4C
OMAP2420_GPIO_DEBOUNCENABLE				EQU 0x50
OMAP2420_GPIO_DEBOUNCINGTIME			EQU 0x54
OMAP2420_GPIO_CLEARIRQENABLE1			EQU 0x60
OMAP2420_GPIO_SETIRQENABLE1				EQU 0x64
OMAP2420_GPIO_CLEARIRQENABLE2			EQU 0x70
OMAP2420_GPIO_SETIRQENABLE2				EQU 0x74
OMAP2420_GPIO_CLEARWKUENA				EQU 0x80
OMAP2420_GPIO_SETWKUENA					EQU 0x84
OMAP2420_GPIO_CLEARDATAOUT				EQU 0x90
OMAP2420_GPIO_SETDATAOUT				EQU 0x94


    END
