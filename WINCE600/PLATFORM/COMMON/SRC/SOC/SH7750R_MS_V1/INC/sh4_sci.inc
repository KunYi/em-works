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
;;	File:   sh4_sci.inc
;;
;;------------------------------------------------------------------------------

;
; Serial Communication Interface (SCI) 
;

SCI_REGBASE                 .equ    h'FFE00000    ; SH-4 control regs 
SCI_REGSIZE                 .equ    h'0020    

SCI_SCSMR1_OFFSET           .equ    h'0000        ; Serial Mode Register 
SCI_SCBRR1_OFFSET           .equ    h'0004        ; Bit rate register 
SCI_SCSCR1_OFFSET           .equ    h'0008        ; Serial Control Register 
SCI_SCTDR1_OFFSET           .equ    h'000c        ; transmit data register 
SCI_SCSSR1_OFFSET           .equ    h'0010        ; Serail Status Register 
SCI_SCRDR1_OFFSET           .equ    h'0014        ; Receive Data register 
SCI_SCSPTR1_OFFSET          .equ    h'001c        ; Serial Port register 

SCI_SCSMR1                  .equ    (SCI_REGBASE + SCI_SCSMR1_OFFSET)
SCI_SCBRR1                  .equ    (SCI_REGBASE + SCI_SCBRR1_OFFSET)
SCI_SCSCR1                  .equ    (SCI_REGBASE + SCI_SCSCR1_OFFSET)
SCI_SCTDR1                  .equ    (SCI_REGBASE + SCI_SCTDR1_OFFSET)
SCI_SCSSR1                  .equ    (SCI_REGBASE + SCI_SCSSR1_OFFSET)
SCI_SCRDR1                  .equ    (SCI_REGBASE + SCI_SCRDR1_OFFSET)
SCI_SCSPTR1                 .equ    (SCI_REGBASE + SCI_SCSPTR1_OFFSET)

