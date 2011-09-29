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
;  File:  omap5912_wdog.inc
;
;  This file contains offset addresses for WatchDog registers.
;

OMAP5912_WDOG_CNTL_TIMER	EQU     0x0000
OMAP5912_WDOG_LOAD_TIMER 	EQU	0x0004
OMAP5912_WDOG_READ_TIMER 	EQU	0x0004
OMAP5912_WDOG_TIMER_MODE 	EQU	0x0008

;------------------------------------------------------------------------------

OMAP5912_WDOG_DISABLE_SEQ1	EQU	0xF5
OMAP5912_WDOG_DISABLE_SEQ2	EQU	0xA0

;------------------------------------------------------------------------------

				END
