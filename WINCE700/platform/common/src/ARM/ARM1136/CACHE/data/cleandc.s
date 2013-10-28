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
; THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
;

;======================================================================
;
;   File: cleandc.s                              

        INCLUDE kxarm.h
        INCLUDE armmacros.s

        TEXTAREA

;FUNCTION  OALCleanDCache 
;
;DESCRIPTION
;   Clean the ARM11 data cache
;
;DEPENDENCIES
;
;RETURN VALUE
;   NONE
;
;SIDE EFFECTS
;  Data Cache cleaned  
;
;===================================================================
;
        LEAF_ENTRY OALCleanDCache

        mcr     p15, 0, r5, c7, c10, 0

        mov     r0, #0
        mcr     p15, 0, r0, c7, c10, 4
    
        RETURN

        END
