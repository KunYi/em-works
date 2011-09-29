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
;  File:  omap2420_wdog.inc
;
;  This file contains offset addresses for WatchDog registers.
;

OMAP2420_WDOG_WIDR_OA          EQU   0x0000
OMAP2420_WDOG_WD_SYSCONFIG_OA  EQU   0x0010
OMAP2420_WDOG_WD_SYSSTATUS_OA  EQU   0x0014
OMAP2420_WDOG_WISR_OA          EQU   0x0018
OMAP2420_WDOG_WIER_OA          EQU   0x001C
OMAP2420_WDOG_WCLR_OA          EQU   0x0024
OMAP2420_WDOG_WCRR_OA          EQU   0x0028
OMAP2420_WDOG_WLDR_OA          EQU   0x002C
OMAP2420_WDOG_WTGR_OA          EQU   0x0030
OMAP2420_WDOG_WWPS_OA          EQU   0x0034
OMAP2420_WDOG_WSPR_OA          EQU   0x0048

;------------------------------------------------------------------------------

OMAP2420_WDOG_DISABLE_SEQ1	EQU	0xF5
OMAP2420_WDOG_DISABLE_SEQ2	EQU	0xA0

;------------------------------------------------------------------------------

				END
