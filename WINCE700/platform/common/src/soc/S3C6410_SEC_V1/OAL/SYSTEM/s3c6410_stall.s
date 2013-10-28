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
;------------------------------------------------------------------------------
;
;  File:  s3c6410_stall.s

    INCLUDE kxarm.h
    INCLUDE s3c6410.inc

    TEXTAREA

    EXPORT    OALStall_us
    EXPORT    OALStall_ms
    
MICROSEC_STALL_VALUE    EQU     (ARM_CLK/1000000)

MICROSEC_STALL_COUNT    EQU        ( MICROSEC_STALL_VALUE + ( 20 - (MICROSEC_STALL_VALUE:MOD:20) ) )        ; This value must be multiple of 20
MILLISEC_STALL_COUNT    EQU        (MICROSEC_STALL_COUNT*1000)        ; This value must be multiple of 20

;-------------------------------------------------------------------------------
;
;  Function:  OALStall_us
;
;  This function implements busy stall loop. On entry r0 contains number
;  of microseconds to stall.
;  We assume that system is in full-speed. I-Cache and Branch-Prediction is enabled.
;  Stall function may take more time with DVS
;

        LEAF_ENTRY OALStall_us

10        ldr        r1, =MICROSEC_STALL_COUNT    ; This value must be multiple of 20
20        sub        r1, r1, #1
        sub        r1, r1, #1
        sub        r1, r1, #1
        sub        r1, r1, #1
        sub        r1, r1, #1    ; -5
        sub        r1, r1, #1
        sub        r1, r1, #1
        sub        r1, r1, #1
        sub        r1, r1, #1
        sub        r1, r1, #1    ; -10
        sub        r1, r1, #1
        sub        r1, r1, #1
        sub        r1, r1, #1
        sub        r1, r1, #1
        sub        r1, r1, #1    ; -15
        sub        r1, r1, #1
        sub        r1, r1, #1
        sub        r1, r1, #1
        sub        r1, r1, #1
        subs        r1, r1, #1    ; -20
        bne        %B20
        subs        r0, r0, #1
        bne        %B10
        bx        lr

        ENTRY_END OALStall_us

;-------------------------------------------------------------------------------
;
;  Function:  OALStall_ms
;
;  This function implements busy stall loop. On entry r0 contains number
;  of microseconds to stall.
;  We assume that system is in full-speed. I-Cache and Branch-Prediction is enabled.
;  Stall function may take more time with DVS
;

        LEAF_ENTRY OALStall_ms

10        ldr        r1, =MILLISEC_STALL_COUNT    ; This value must be multiple of 20
20        sub        r1, r1, #1
        sub        r1, r1, #1
        sub        r1, r1, #1
        sub        r1, r1, #1
        sub        r1, r1, #1    ; -5
        sub        r1, r1, #1
        sub        r1, r1, #1
        sub        r1, r1, #1
        sub        r1, r1, #1
        sub        r1, r1, #1    ; -10
        sub        r1, r1, #1
        sub        r1, r1, #1
        sub        r1, r1, #1
        sub        r1, r1, #1
        sub        r1, r1, #1    ; -15
        sub        r1, r1, #1
        sub        r1, r1, #1
        sub        r1, r1, #1
        sub        r1, r1, #1
        subs        r1, r1, #1    ; -20
        bne        %B20
        subs        r0, r0, #1
        bne        %B10
        bx        lr

        ENTRY_END OALStall_ms

;------------------------------------------------------------------------------

        END

