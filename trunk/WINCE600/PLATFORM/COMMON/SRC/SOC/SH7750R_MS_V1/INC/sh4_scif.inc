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
;;	File:   sh4_scif.inc
;;
;;------------------------------------------------------------------------------

;
; Serial Communication Interface with FIFO (SCIF) 
;

SCIF_REGBASE                .equ    h'FFE80000    ; SH-4 control regs 
SCIF_REGSIZE                .equ    h'0028    

SCIF_SCSMR2_OFFSET          .equ    h'0000        ; Serial Mode Register 
SCIF_SCBRR2_OFFSET          .equ    h'0004        ; Bit rate register 
SCIF_SCSCR2_OFFSET          .equ    h'0008        ; Serial Control Register 
SCIF_SCFTDR2_OFFSET         .equ    h'000c        ; transmit FIFO data register 
SCIF_SCFSR2_OFFSET          .equ    h'0010        ; Serail Status Register 
SCIF_SCFRDR2_OFFSET         .equ    h'0014        ; Receive Data FIFO register 
SCIF_SCFCR2_OFFSET          .equ    h'0018        ; FIFO Control Register 
SCIF_SCFDR2_OFFSET          .equ    h'001c        ; FIFO Data Count set register 
SCIF_SCSPTR2_OFFSET         .equ    h'0020        ; FIFO Data Count set register 
SCIF_SCLSR2_OFFSET          .equ    h'0024        ; FIFO Data Count set register 

SCIF_SCSMR2                 .equ    (SCIF_REGBASE + SCIF_SCSMR2_OFFSET)
SCIF_SCBRR2                 .equ    (SCIF_REGBASE + SCIF_SCBRR2_OFFSET)
SCIF_SCSCR2                 .equ    (SCIF_REGBASE + SCIF_SCSCR2_OFFSET)
SCIF_SCFTDR2                .equ    (SCIF_REGBASE + SCIF_SCFTDR2_OFFSET)
SCIF_SCFSR2                 .equ    (SCIF_REGBASE + SCIF_SCFSR2_OFFSET)
SCIF_SCFRDR2                .equ    (SCIF_REGBASE + SCIF_SCFRDR2_OFFSET)
SCIF_SCFCR2                 .equ    (SCIF_REGBASE + SCIF_SCFCR2_OFFSET)
SCIF_SCFDR2                 .equ    (SCIF_REGBASE + SCIF_SCFDR2_OFFSET)
SCIF_SCSPTR2                .equ    (SCIF_REGBASE + SCIF_SCSPTR2_OFFSET)
SCIF_SCLSR2                 .equ    (SCIF_REGBASE + SCIF_SCLSR2_OFFSET)

