;
; Copyright (c) Microsoft Corporation.  All rights reserved.
;
;
; Use of this source code is subject to the terms of the Microsoft end-user
; license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
; If you did not accept the terms of the EULA, you are not authorized to use
; this source code. For a copy of the EULA, please see the LICENSE.RTF on your
; install media.
;
;-------------------------------------------------------------------------------
;
; Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
; THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
; AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
;
;-------------------------------------------------------------------------------
;
;  File:  flushdclines.s
;
;
    INCLUDE kxarm.h
    INCLUDE armmacros.s
    INCLUDE oal_cache.inc

    IMPORT g_oalCacheInfo

    TEXTAREA

;-------------------------------------------------------------------------------
;
;  Function:  OALFlushDCacheLines
;
    LEAF_ENTRY OALFlushDCacheLines

    ldr     r2, =g_oalCacheInfo
    ldr     r3, [r2, #L1DLineSize]

10  mcr     p15, 0, r0, c7, c14, 1          ; Clean and invalidate entry
    add     r0, r0, r3                      ; Move to next
    subs    r1, r1, r3
    bgt     %b10                            ; Loop while > 0 bytes left

    mov     r0, #0
    mcr     p15, 0, r0, c7, c5, 6           ; Flush the BTAC

    RETURN

    END
