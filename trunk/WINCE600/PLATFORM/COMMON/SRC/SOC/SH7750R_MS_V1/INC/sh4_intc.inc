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
;;	File:   sh4_intc.inc
;;
;;------------------------------------------------------------------------------

;
; Interrupt Controller registers
;

INTC_REGBASE                .equ    h'FFD00000
INTC_REGSIZE                .equ    h'0020

INTC_ICR_OFFSET             .equ    h'0000    ; intr controll reg offset
INTC_IPRA_OFFSET            .equ    h'0004    ; intr priority level A  offset
INTC_IPRB_OFFSET            .equ    h'0008    ; intr priority level B  offset
INTC_IPRC_OFFSET            .equ    h'000C    ; intr priority level C  offset

INTC_ICR                    .equ    (INTC_REGBASE + INTC_ICR_OFFSET)
INTC_IPRA                   .equ    (INTC_REGBASE + INTC_IPRA_OFFSET)
INTC_IPRB                   .equ    (INTC_REGBASE + INTC_IPRB_OFFSET)
INTC_IPRC                   .equ    (INTC_REGBASE + INTC_IPRC_OFFSET)

; Interrupt Control Register

INTC_ICR_IRL_MASK           .equ    h'FF7F
INTC_ICR_IRL_ENCODE         .equ    h'0000
INTC_ICR_IRL_INDEPENDENT    .equ    h'0080

; Interrupt Priority level setting Register A
 
INTC_IPRA_TMU0_MASK         .equ    h'0FFF
INTC_IPRA_TMU1_MASK         .equ    h'F0FF
INTC_IPRA_TMU2_MASK         .equ    h'FF0F
INTC_IPRA_RTC_MASK          .equ    h'FFF0

; Interrupt Priority level setting Register B
 
INTC_IPRB_WDT_MASK          .equ    h'0FFF
INTC_IPRB_REF_MASK          .equ    h'F0FF
INTC_IPRB_SCI_MASK          .equ    h'FF0F

; Interrupt Priority level setting Register C
 
INTC_IPRC_DMAC_MASK         .equ    h'F0FF
INTC_IPRC_SCIF_MASK         .equ    h'FF0F
INTC_IPRC_JTAG_MASK         .equ    h'FFF0

