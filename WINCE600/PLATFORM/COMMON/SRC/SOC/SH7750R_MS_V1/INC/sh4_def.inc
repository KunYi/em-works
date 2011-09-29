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
;;	File:   sh4_def.inc
;;
;;------------------------------------------------------------------------------

;
; Clock Pulse Generator (CPG)
;

ZERO_TME                    .equ    CPG_WTCSR_COOKIE        | h'00
SET_CKS                     .equ    CPG_WTCSR_COOKIE        | CPG_WTCSR_CKS_1024
COUNT_INIT                  .equ    CPG_WTCNT_COOKIE        | h'00

CPG_FRQCR_CLOCK             .equ    CPG_FRQCR_CKOEN         | CPG_FRQCR_PLL1EN  | 
+                                   CPG_FRQCR_PLL2EN        | CPG_FRQCR_IFC_1   | 
+                                   CPG_FRQCR_BFC_3         | CPG_FRQCR_PFC_6

CPG_FRQCR_CLOCK2            .equ    CPG_FRQCR_CKOEN         | CPG_FRQCR_PLL1EN  | 
+                                   CPG_FRQCR_PLL2EN        | CPG_FRQCR_IFC_2   | 
+                                   CPG_FRQCR_BFC_3         | CPG_FRQCR_PFC_6

;
; Bus State Control Modes
;

;BSC_BCR1_MEMORY_MAP_EDO    .equ    BSC_BCR1_DRAM_A2N3D     | BSC_BCR1_A56PCM   
           
BSC_BCR1_MEMORY_MAP_SDRAM   .equ    BSC_BCR1_DRAM_A2N3S     | BSC_BCR1_A56PCM   |
+                                   BSC_BCR1_A1MBC          | BSC_BCR1_A4MBC

BSC_BCR2_A1_BUS_WIDTH       .equ    BSC_BCR2_A1SZ_32
BSC_BCR2_A2_BUS_WIDTH       .equ    BSC_BCR2_A2SZ_32
BSC_BCR2_A3_BUS_WIDTH       .equ    BSC_BCR2_A3SZ_64
BSC_BCR2_A4_BUS_WIDTH       .equ    BSC_BCR2_A4SZ_32
BSC_BCR2_A5_BUS_WIDTH       .equ    BSC_BCR2_A5SZ_8
BSC_BCR2_A6_BUS_WIDTH       .equ    BSC_BCR2_A6SZ_8

BSC_BCR2_BUS_WIDTHS         .equ    BSC_BCR2_A1_BUS_WIDTH   |
+                                   BSC_BCR2_A2_BUS_WIDTH   |
+                                   BSC_BCR2_A3_BUS_WIDTH   | 
+                                   BSC_BCR2_A4_BUS_WIDTH   |
+                                   BSC_BCR2_A5_BUS_WIDTH   |
+                                   BSC_BCR2_A6_BUS_WIDTH

BSC_WCR1_IDLE_STATES        .equ    BSC_WCR1_A6IW_15        |
+                                   BSC_WCR1_A5IW_15        |
+                                   BSC_WCR1_A4IW_3         |
+                                   BSC_WCR1_A3IW_2         |
+                                   BSC_WCR1_A2IW_12        |
+                                   BSC_WCR1_A1IW_3         |
+                                   BSC_WCR1_A0IW_2         |
+                                   BSC_WCR1_DMAIW_15

BSC_WCR2_WAIT_STATES        .equ    BSC_WCR2_A6W_15         | BSC_WCR2_A6B_7    |
+                                   BSC_WCR2_A5W_15         | BSC_WCR2_A5B_7    |
+                                   BSC_WCR2_A4W_15         |
+                                   BSC_WCR2_A3W_DRAM_3     |
+                                   BSC_WCR2_A2W_NORMAL_15  |
+                                   BSC_WCR2_A1W_3          |
+                                   BSC_WCR2_A0W_15         | BSC_WCR2_A0B_7

BSC_WCR3_WAIT_INSERTED      .equ    BSC_WCR3_A6S_0          | BSC_WCR3_A6H_0    |
+                                   BSC_WCR3_A5S_0          | BSC_WCR3_A5H_0    |
+                                   BSC_WCR3_A4S_0          | BSC_WCR3_A4H_0    |
+                                   BSC_WCR3_A3S_0          | BSC_WCR3_A3H_0    |
+                                   BSC_WCR3_A2S_0          | BSC_WCR3_A2H_0    |
+                                   BSC_WCR3_A1S_0          | BSC_WCR3_A1H_0    |
+                                   BSC_WCR3_A0S_0          | BSC_WCR3_A0H_0

BSC_MCR_SDRAM               .equ    BSC_MCR_TRC_0           | BSC_MCR_TPC_1     |
+                                   BSC_MCR_RCD_4           | BSC_MCR_TRWL_1    |
+                                   BSC_MCR_TRAS_2          | BSC_MCR_SZ_64     |
+                                   BSC_MCR_AMX_11          | BSC_MCR_RFSH

BSC_MCR_SDRAM_MRSET         .equ    BSC_MCR_MRSET           | BSC_MCR_SDRAM

BSC_SDMR3                   .equ    BSC_SDMR3_COOKIE        | BSC_SDMR3_110

BSC_PCR_WAIT_INSERTED       .equ    BSC_PCR_A5PCW_0         | BSC_PCR_A6PCW_0   |
+                                   BSC_PCR_A5TED_6         | BSC_PCR_A6TED_6   |
+                                   BSC_PCR_A5TEH_2         | BSC_PCR_A6TEH_2

BSC_RTCSR_SETTINGS          .equ    BSC_RTCSR_COOKIE        |
+                                   BSC_RTCSR_CKS_4         |
+                                   BSC_RTCSR_LMTS

BSC_RTCNT_START             .equ    BSC_RTCNT_COOKIE        | 0  

BSC_RTCOR_MAX_COUNT         .equ    BSC_RTCOR_COOKIE        | h'37

;
; Cache Control Register
;

CCN_CCR_MODE                .equ    CCN_CCR_WT
CCN_CCR_DISABLE             .equ    0x00

CCN_CCR_FLUSH:              .equ    CCN_CCR_ICI | CCN_CCR_OCI
CCN_CCR_ENABLE:             .equ    CCN_CCR_ICE | CCN_CCR_OCE | CCN_CCR_WT

;
; The SR reg, puts us in priviledged mode, and blocks all interrupts
;

TM_SR						.equ	h'70000000
