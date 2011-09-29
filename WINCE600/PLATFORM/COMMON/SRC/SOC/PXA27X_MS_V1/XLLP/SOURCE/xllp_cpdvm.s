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
;
; Copyright 2002-2003 Intel Corporation All Rights Reserved.
;
;** Portions of the source code contained or described herein and all documents
;** related to such source code (Material) are owned by Intel Corporation
;** or its suppliers or licensors and is licensed by Microsoft Corporation for distribution.  
;** Title to the Material remains with Intel Corporation or its suppliers and licensors. 
;** Use of the Materials is subject to the terms of the Microsoft license agreement which accompanied the Materials.  
;** No other license under any patent, copyright, trade secret or other intellectual
;** property right is granted to or conferred upon you by disclosure or
;** delivery of the Materials, either expressly, by implication, inducement,
;** estoppel or otherwise 
;** Some portion of the Materials may be copyrighted by Microsoft Corporation.
;

    INCLUDE xlli_Bulverde_defs.inc  
    EXPORT  XllpXSC1EnterTurbo  
    EXPORT  XllpXSC1ExitTurbo
    EXPORT  XllpXSC1ReadCLKCFG
    EXPORT  XllpXSC1WriteCLKCFG
    EXPORT  XllpXSC1ChangeVoltage
    EXPORT  XllpXSC1FreqChange
    EXPORT  XSC1GetCPUId
    EXPORT  XSC1GetCPSR
    EXPORT  XSC1GetSPSR

    AREA    |.text|, CODE, READONLY, ALIGN=5        ; Align =5 required for "ALIGN 32" to work

IRQ_MODE    EQU 2_10010
SVC_MODE    EQU 2_10011

;
; XllpXSC1EnterTurbo - Enters Turbo Mode
;
;   Uses r0 - contains value for writing to PWRMODE coprocessor register
;   
;   Preserve the Fast Bus Mode, B bit.
;
XllpXSC1EnterTurbo   FUNCTION

    mrc     p14, 0, r0, c6, c0, 0           ; read c6
    and     r0,  r0, #0xd                   ; clear F bit
    orr     r0, r0, #1                      ; or in Turbo bit
    mcr     p14, 0, r0, c6, c0, 0           ; enter Turbo mode
      IF Interworking :LOR: Thumbing
         bx  lr
      ELSE
         mov  pc, lr          ; return
      ENDIF

    ENDFUNC

;
; XllpXSC1ExitTurbo - Exits Turbo Mode
;
;   Uses r0 - contains value for writing to PWRMODE coprocessor register   
;
;   Preserve the Fast Bus Mode, B bit.
;
XllpXSC1ExitTurbo   FUNCTION

    mrc     p14, 0, r0, c6, c0, 0       ; read c6
    and     r0, r0, #0xe                ; clear Turbo bit to
    mcr     p14, 0, r0, c6, c0, 0       ; exit Turbo mode
      IF Interworking :LOR: Thumbing
         bx  lr
      ELSE
         mov  pc, lr          ; return
      ENDIF

    ENDFUNC

;
;XllpXSC1ReadCLKCFG - Reads CCLKCFG register
;
;   Uses r0 - contains return value -> CCLKCFG register contents
;
XllpXSC1ReadCLKCFG   FUNCTION

    mrc     p14, 0, r0, c6, c0, 0       ; Read CCLKCFG

    IF Interworking :LOR: Thumbing
      bx  lr
    ELSE
      mov  pc, lr          ; return
    ENDIF

    ENDFUNC


;----------------------------------------------------------------------------------------
; XSC1GetCPUId - Get the CPU ID from CP15 R0 Register
;
; This routine reads R0 from CoProcesser 15 to get the CPU ID
;
;       Uses r0 - return value of CPU ID
;----------------------------------------------------------------------------------------
;        LEAF_ENTRY XSC1GetCPUId
XSC1GetCPUId FUNCTION

    mrc     p15, 0, r0, c0, c0, 0

    IF Interworking :LOR: Thumbing
        bx  lr
    ELSE
        mov  pc, lr          ; return
    ENDIF

    ENDFUNC


;----------------------------------------------------------------------------------------
; XSC1GetCPSR - Returns the Current Program Status Register
;
;       Uses r0 returns CPSR
;----------------------------------------------------------------------------------------
XSC1GetCPSR FUNCTION

    mrs     r0, CPSR

    IF Interworking :LOR: Thumbing
     bx  lr
    ELSE
     mov  pc, lr          ; return
    ENDIF

    ENDFUNC


;----------------------------------------------------------------------------------------
; XSC1GetSPSR - Returns the Saved Program Status Register
;
; This routine is called by the IRQ interrupt handler when
; PMU is active and capturing data
;
;       Uses    r0 returns SPSR
;----------------------------------------------------------------------------------------
XSC1GetSPSR FUNCTION

    mrs     r0, SPSR
      IF Interworking :LOR: Thumbing
         bx  lr
      ELSE
         mov  pc, lr          ; return
      ENDIF

    ENDFUNC


;
; XllpXSC1ChangeVoltage - change voltage
;
;   Uses r0 - contains value for writing to PWRMODE coprocessor register   
;
;
;
XllpXSC1ChangeVoltage   FUNCTION

    mrc     p14, 0, r0, c7, c0, 0           ; read c7
    orr     r0, r0, #0x8                    ; Voltage change sequence begins
    mcr     p14, 0, r0, c7, c0, 0           ; exit Turbo mode
      IF Interworking :LOR: Thumbing
         bx  lr
      ELSE
         mov  pc, lr          ; return
      ENDIF

    ENDFUNC

XllpXSC1FreqChange  FUNCTION
;
; XSC1FreqChange - Do a Frequency Change
;
;       Uses    
;            r0 - arg1 - mask to set the CLKCFG register
;                
    orr     r0, r0, #2                      ; set F=1               
    mcr     p14, 0, r0, c6, c0, 0           ; do Freq Change
    
FreqRet
     
    IF Interworking :LOR: Thumbing
        bx  lr
    ELSE
        mov  pc, lr          ; return
    ENDIF

    ENDFUNC
    
XllpXSC1WriteCLKCFG FUNCTION
;
; XSC1FreqChange - Do a Frequency Change
;
;       Uses    
;            r0 - arg1 - mask to set the CLKCFG register
;                
    mcr     p14, 0, r0, c6, c0, 0           ; do Freq Change
    IF Interworking :LOR: Thumbing
        bx  lr
    ELSE
        mov  pc, lr          ; return
    ENDIF

    ENDFUNC
    
 END

