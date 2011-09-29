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
;------------------------------------------------------------------------------
;
;  File:  omap5912_gpio.inc
;
;  This file defines offset addresses for GPIO registers.
;
OMAP5912_GPIO_REVISION_REGS_OA          EQU     0x0000
OMAP5912_GPIO_SYSCONFIG_REGS_OA         EQU     0x0010
OMAP5912_GPIO_SYSSTATUS_REGS_OA         EQU     0x0014
OMAP5912_GPIO_IRQSTATUS1_REGS_OA        EQU     0x0018
OMAP5912_GPIO_IRQENABLE1_REGS_OA        EQU     0x001C
OMAP5912_GPIO_IRQSTATUS2_REGS_OA        EQU     0x0020
OMAP5912_GPIO_IRQENABLE2_REGS_OA        EQU     0x0024
OMAP5912_GPIO_WAKEUPENABLE_REGS_OA      EQU     0x0028
OMAP5912_GPIO_DATAIN_REGS_OA            EQU     0x002C
OMAP5912_GPIO_DATAOUT_REGS_OA           EQU     0x0030
OMAP5912_GPIO_DIRECTION_REGS_OA         EQU     0x0034
OMAP5912_GPIO_EDGE_CTRL1_REGS_OA        EQU     0x0038
OMAP5912_GPIO_EDGE_CTRL2_REGS_OA        EQU     0x003C
OMAP5912_GPIO_CLEAR_IRQENABLE1_REGS_OA  EQU     0x009C
OMAP5912_GPIO_CLEAR_IRQENABLE2_REGS_OA  EQU     0x00A4
OMAP5912_GPIO_CLEAR_WAKEUPENA_REGS_OA   EQU     0x00A8
OMAP5912_GPIO_CLEAR_DATAOUT_REGS_OA     EQU     0x00B0
OMAP5912_GPIO_SET_IRQENABLE1_REGS_OA    EQU     0x00DC
OMAP5912_GPIO_SET_IRQENABLE2_REGS_OA    EQU     0x00E4
OMAP5912_GPIO_SET_WAKEUPENA_REGS_OA     EQU     0x00E8
OMAP5912_GPIO_SET_DATAOUT_REGS_OA       EQU     0x00F0

;------------------------------------------------------------------------------

                                        END
