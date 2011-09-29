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
;;	File:   sh4_ccn.inc
;;
;;------------------------------------------------------------------------------

;
; CCN.
;

CCN_REGBASE             .equ    h'FF000000    ; CCN Register Base Address
CCN_REGSIZE             .equ    h'40

CCN_CCR_OFFSET          .equ    h'001C        ; Cache Control Register Offset
CCN_QACR0_OFFSET        .equ    h'0038        ; Queue Address Control Register 0
CCN_QACR1_OFFSET        .equ    h'003C        ; Queue Address Control Register 1

CCN_CCR                 .equ    (CCN_REGBASE + CCN_CCR_OFFSET)      ; Cache Control Register
CCN_QACR0               .equ    (CCN_REGBASE + CCN_QACR0_OFFSET)    ; Queue Address Control Register 0
CCN_QACR1               .equ    (CCN_REGBASE + CCN_QACR1_OFFSET)    ; Queue Address Control Register 1

; Cache Control Register

CCN_CCR_IIX             .equ    h'00008000    ;IC index mode
CCN_CCR_ICI             .equ    h'00000800    ;IC invalidation
CCN_CCR_ICE             .equ    h'00000100    ;IC enable
CCN_CCR_OIX             .equ    h'00000080    ;OC index mode
CCN_CCR_ORA             .equ    h'00000020    ;OC RAM enable
CCN_CCR_OCI             .equ    h'00000008    ;OC invalidation
CCN_CCR_CB              .equ    h'00000004    ;Copy-back enable 
CCN_CCR_WT              .equ    h'00000002    ;Write-through enable
CCN_CCR_OCE             .equ    h'00000001    ;OC enable

