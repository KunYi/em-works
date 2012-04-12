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
; Copyright (C) 2007-2010, Freescale Semiconductor, Inc. All Rights Reserved.
; THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
; AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
;
;-------------------------------------------------------------------------------
;
;  File:  cleandclines.s
;
;  This file implements the OALCleanDCacheLines function.
;
    INCLUDE kxarm.h
    INCLUDE armmacros.s
    INCLUDE oal_cache.inc

    IMPORT g_oalCacheInfo

    TEXTAREA

;-------------------------------------------------------------------------------
;
;  Function:  OALCleanDCacheLines
;
    LEAF_ENTRY OALCleanDCacheLines

    ldr     r2, =g_oalCacheInfo
    ldr     r3, [r2, #L1DLineSize]

    ; Clean L1 lines to PoU (L2 cache)
10  mcr     p15, 0, r0, c7, c11, 1          ; clean entry to PoU
    add     r0, r0, r3                      ; move to next entry
    subs    r1, r1, r3
    bgt     %b10                            ; loop while > 0 bytes left

    RETURN

    END
