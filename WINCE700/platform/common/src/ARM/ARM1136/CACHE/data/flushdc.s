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
;
;======================================================================
;  File:  flushdc.s                              
;

        INCLUDE kxarm.h
        INCLUDE armmacros.s

        TEXTAREA

;======================================================================
;FUNCTION  OALFlushDCache 
;
;DESCRIPTION
;   Flush the ARM11 data cache
;
;DEPENDENCIES
;
;RETURN VALUE
;   NONE
;
;SIDE EFFECTS
;  Data Cache flushed  
;
;===================================================================
;
    
        LEAF_ENTRY OALFlushDCache

        mcr     p15, 0, r15, c7, c14, 0
        mov     r0, #0
        mcr     p15, 0, r0, c7, c10, 4

        RETURN

        END
