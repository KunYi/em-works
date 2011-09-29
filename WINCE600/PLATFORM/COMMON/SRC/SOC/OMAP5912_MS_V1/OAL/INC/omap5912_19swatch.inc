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
;  Header:  omap5912_19swatch.inc
;
;  This header file defines the offsets of watchdog timer as well as
;  its command sequences
;

OMAP5912_19SWATCHDOG_MODE                  EQU     0x48
OMAP5912_19SWATCHDOG_STATUS                EQU     0x34

OMAP5912_19SWATCHDOG_DISABLE_SEQ1          EQU     0xAAAA
OMAP5912_19SWATCHDOG_DISABLE_SEQ2          EQU     0x5555


;------------------------------------------------------------------------------

                        END
