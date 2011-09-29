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
;;------------------------------------------------------------------------------
;;
;;  Copyright(c) 1998,1999 Renesas Technology Corp.
;;
;;------------------------------------------------------------------------------
;;
;;	File:   sh4_tmu.inc
;;
;;------------------------------------------------------------------------------

;
; Timer Unit registers
;

TMU_REGBASE                 .equ    h'FFD80000
TMU_REGSIZE                 .equ    h'0040

TMU_TOCR_OFFSET             .equ    h'0000                ;common
TMU_TSTR_OFFSET             .equ    h'0004

TMU_TCOR0_OFFSET            .equ    h'0008                ;TMU0
TMU_TCNT0_OFFSET            .equ    h'000C
TMU_TCR0_OFFSET             .equ    h'0010

TMU_TCOR1_OFFSET            .equ    h'0014                ;TMU1
TMU_TCNT1_OFFSET            .equ    h'0018
TMU_TCR1_OFFSET             .equ    h'001C

TMU_TCOR2_OFFSET            .equ    h'0020                ;TMU2
TMU_TCNT2_OFFSET            .equ    h'0024
TMU_TCR2_OFFSET             .equ    h'0028

TMU_TCPR2_OFFSET            .equ    h'002C

TMU_TOCR                    .equ    (TMU_REGBASE + TMU_TOCR_OFFSET)
TMU_TSTR                    .equ    (TMU_REGBASE + TMU_TSTR_OFFSET)

TMU_TCOR0                   .equ    (TMU_REGBASE + TMU_TCOR0_OFFSET)
TMU_TCNT0                   .equ    (TMU_REGBASE + TMU_TCNT0_OFFSET)
TMU_TCR0                    .equ    (TMU_REGBASE + TMU_TCR0_OFFSET)

TMU_TCOR1                   .equ    (TMU_REGBASE + TMU_TCOR1_OFFSET)
TMU_TCNT1                   .equ    (TMU_REGBASE + TMU_TCNT1_OFFSET)
TMU_TCR1                    .equ    (TMU_REGBASE + TMU_TCR1_OFFSET)

TMU_TCOR2                   .equ    (TMU_REGBASE + TMU_TCOR2_OFFSET)
TMU_TCNT2                   .equ    (TMU_REGBASE + TMU_TCNT2_OFFSET)
TMU_TCR2                    .equ    (TMU_REGBASE + TMU_TCR2_OFFSET)

TMU_TCPR2                   .equ    (TMU_REGBASE + TMU_TCPR2_OFFSET)

; Timer Start Register

TMU_TSTR_STR0               .equ    h'01
TMU_TSTR_STR1               .equ    h'02
TMU_TSTR_STR2               .equ    h'04

; Timer Control Register

TMU_TCR_UNF                 .equ    h'100    ; counter underflowed
TMU_TCR_UNIE                .equ    h'20    ; underflow interrupt enable

TMU_TCR_RISE                .equ    h'00    ; count on rising edge of clock
TMU_TCR_FALL                .equ    h'08    ; count on falling edge of clock
TMU_TCR_BOTH                .equ    h'10    ; count on both edges of clock

TMU_TCR_D4                  .equ    h'00    ; PERIPHERAL clock / 4
TMU_TCR_D16                 .equ    h'01    ; PERIPHERAL clock / 16
TMU_TCR_D64                 .equ    h'02    ; PERIPHERAL clock / 64
TMU_TCR_D256                .equ    h'03    ; PERIPHERAL clock / 256
TMU_TCR_D1024               .equ    h'04    ; PERIPHERAL clock / 1024
TMU_TCR_RTC                 .equ    h'06    ; real time clock output (16 kHz)
TMU_TCR_EXT                 .equ    h'07    ; external clock input

