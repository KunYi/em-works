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
;;	File:   sh4_cpg.inc
;;
;;------------------------------------------------------------------------------

;
; Clock Pulse Generator registers.
;

CPG_REGBASE             .equ    h'FFC00000    ; Frequency control register
CPG_REGSIZE             .equ    h'0014

CPG_FRQCR_OFFSET        .equ    h'0000        ; Frequency control register
CPG_STBCR_OFFSET        .equ    h'0004        ; Stanby control register
CPG_WTCNT_OFFSET        .equ    h'0008        ; Watchdog timer counter
CPG_WTCSR_OFFSET        .equ    h'000C        ; Watchdog timer control/status register
CPG_STBCR2_OFFSET       .equ    h'0010        ; Stanby control register 2

CPG_FRQCR               .equ    (CPG_REGBASE + CPG_FRQCR_OFFSET)
CPG_STBCR               .equ    (CPG_REGBASE + CPG_STBCR_OFFSET)
CPG_WTCNT               .equ    (CPG_REGBASE + CPG_WTCNT_OFFSET)
CPG_WTCSR               .equ    (CPG_REGBASE + CPG_WTCSR_OFFSET)
CPG_STBCR2              .equ    (CPG_REGBASE + CPG_STBCR2_OFFSET)

; Frequency control register.

CPG_FRQCR_CKOEN:        .equ    h'0800    ; Clock enable
CPG_FRQCR_PLL1EN:       .equ    h'0400    ; PLL circuit 1 enable
CPG_FRQCR_PLL2EN:       .equ    h'0200    ; PLL circuit 2 enable

CPG_FRQCR_IFC:          .equ    h'01c0    ; Internal clock freq divider mask
CPG_FRQCR_IFC_1:        .equ    h'0000    ; /1
CPG_FRQCR_IFC_2:        .equ    h'0040    ; /2
CPG_FRQCR_IFC_3:        .equ    h'0080    ; /3
CPG_FRQCR_IFC_4:        .equ    h'00c0    ; /4
CPG_FRQCR_IFC_6:        .equ    h'0100    ; /6
CPG_FRQCR_IFC_8:        .equ    h'0140    ; /8

CPG_FRQCR_BFC:          .equ    h'0038    ; Bus clock frequency division ratio mask
CPG_FRQCR_BFC_1:        .equ    h'0000    ; /1
CPG_FRQCR_BFC_2:        .equ    h'0008    ; /2
CPG_FRQCR_BFC_3:        .equ    h'0010    ; /3
CPG_FRQCR_BFC_4:        .equ    h'0018    ; /4
CPG_FRQCR_BFC_6:        .equ    h'0020    ; /6
CPG_FRQCR_BFC_8:        .equ    h'0028    ; /8

CPG_FRQCR_PFC:          .equ    h'0007    ; Peripheral clock freq divider 2 rate mask
CPG_FRQCR_PFC_2:        .equ    h'0000    ; /2
CPG_FRQCR_PFC_3:        .equ    h'0001    ; /3
CPG_FRQCR_PFC_4:        .equ    h'0002    ; /4
CPG_FRQCR_PFC_6:        .equ    h'0003    ; /6
CPG_FRQCR_PFC_8:        .equ    h'0004    ; /8    

; Stanby control register

CPG_STBCR_STBY:         .equ    h'80    ; Standby
CPG_STBCR_PHZ:          .equ    h'40    ; Peripheral module pin high impedance
CPG_STBCR_PPU:          .equ    h'20    ; Peripheral moduke pin pull up
CPG_STBCR_MSTP4:        .equ    h'10    ; Module standby 4 (DMAC clock on/off)
CPG_STBCR_MSTP3:        .equ    h'08    ; Module standby 3 (SCIF clock on/off)
CPG_STBCR_MSTP2:        .equ    h'04    ; Module standby 2 (TMU clock on/off)
CPG_STBCR_MSTP1:        .equ    h'02    ; Module standby 1 (RTC clock on/off)
CPG_STBCR_MSTP0:        .equ    h'01    ; Module standby 0 (SCI clock on/off)

; Watchdog timer counter.

CPG_WTCNT_COOKIE:       .equ    h'5a00    ; OR with new value before writing
CPG_WTCNT_COUNT:        .equ    h'00ff    ; Watchdog timer counter mask

; Watchdog timer control/status register.

CPG_WTCSR_COOKIE:       .equ    h'a500    ; OR with new value before writing
CPG_WTCSR_TME:          .equ    h'0080    ; Timer enable
CPG_WTCSR_WT:           .equ    h'0040    ; Timer mode select, 1 <=> watchdog
CPG_WTCSR_RSTS:         .equ    h'0020    ; Reset select
CPG_WTCSR_WOVF:         .equ    h'0010    ; Watchdog timer overflow
CPG_WTCSR_IOVF:         .equ    h'0008    ; Interval timer overflow

CPG_WTCSR_CKS:          .equ    h'0007    ; Clock select, periph clock div ratio
CPG_WTCSR_CKS_32:       .equ    h'0000    ; /32
CPG_WTCSR_CKS_64:       .equ    h'0001    ; /64
CPG_WTCSR_CKS_128:      .equ    h'0002    ; /128
CPG_WTCSR_CKS_256:      .equ    h'0003    ; /256
CPG_WTCSR_CKS_512:      .equ    h'0004    ; /512
CPG_WTCSR_CKS_1024:     .equ    h'0005    ; /1024
CPG_WTCSR_CKS_2048:     .equ    h'0006    ; /2048
CPG_WTCSR_CKS_4096:     .equ    h'0007    ; /4096

; Stanby control register 2

CPG_STBCR2_DSLP:        .equ    h'80    ; Deep sleep mode

