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
;******************************************************************************
;
; Copyright 2002 Intel Corporation. All Rights Reserved.
;
; Portions of the source code contained or described herein and all documents
; related to such source code (Material) are owned by Intel Corporation
; or its suppliers or licensors and is licensed by Microsoft Corporation for distribution.  
; Title to the Material remains with Intel Corporation or its suppliers and licensors. 
; Use of the Materials is subject to the terms of the Microsoft license agreement which accompanied the Materials.  
; No other license under any patent, copyright, trade secret or other intellectual
; property right is granted to or conferred upon you by disclosure or
; delivery of the Materials, either expressly, by implication, inducement,
; estoppel or otherwise 
; Some portion of the Materials may be copyrighted by Microsoft Corporation.
;
;  FILENAME:       pmuregs.s
;
;  PURPOSE:        Provides low level PMU primitive functions written specifically for
;                  the Bulverde/Mainstone processor/platform.  Specially design to fit  
;                  into Intel VTUNE Architecture
;                  
; 
;  LAST MODIFIED:  11/21/02
;******************************************************************************
;
; List of primitive functions in this source code include:
;
    EXPORT ReadPMUReg          
    EXPORT WritePMUReg

    AREA    |.text|, CODE, READONLY, ALIGN=5        ; Align =5 required for "ALIGN 32" to work

;
; ReadPMUReg - Read the PMU Register
;
; Description:
;   This routine reads the designated PMU register via CoProcesser 14.
;
; Input Parameters:
;       r0 - arg1, PMU register number to read.  Number between 0 to 8
;   if r0 contains:
;       0 -> PMNC,  PMU Control Register
;       1 -> CCNT,  PMU Clock Counter
;       2 -> PMN0,  PMU Count Register 0
;       3 -> PMN1,  PMU Count Register 1
;       4 -> PMN2,  PMU Count Register 2
;       5 -> PMN3,  PMU Count Register 3
;       6 -> INTEN, PMU Interupt Enable Register
;       7 -> FLAG,  PMU Overflow Flag Status Register
;       8 -> EVTSEL PMU Event Select Register           
;
; Returns:
;   r0 - 32-bit value read from CoProcessor
;    
; Registers Modified:
;   CoProcessor Register Modified: None
;   General Purpose Registers Modified: r0  
;
; NOTE:  
;   Error checking not included
;

ReadPMUReg  FUNCTION

    cmp     r0, #8
    addls   pc, pc, r0, lsl #2
    b       RRet
    b       RdPMNC
    b       RdCCNT
    b       RdPMN0
    b       RdPMN1
    b       RdPMN2
    b       RdPMN3
    b       RdINTEN
    b       RdFLAG
    b       RdEVTSEL

RdPMNC  
    mrc     p14, 0, r0, c0, c1, 0       ; Read PMNC
    b       RRet
RdCCNT  
    mrc     p14, 0, r0, c1, c1, 0       ; Read CCNT
    b       RRet
RdPMN0  
    mrc     p14, 0, r0, c0, c2, 0       ; Read PMN0
    b       RRet
RdPMN1  
    mrc     p14, 0, r0, c1, c2, 0       ; Read PMN1
    b       RRet
RdPMN2  
    mrc     p14, 0, r0, c2, c2, 0       ; Read PMN2
    b       RRet
RdPMN3  
    mrc     p14, 0, r0, c3, c2, 0       ; Read PMN3
    b       RRet
RdINTEN  
    mrc     p14, 0, r0, c4, c1, 0       ; Read INTEN
    b       RRet
RdFLAG  
    mrc     p14, 0, r0, c5, c1, 0       ; Read FLAG
    b       RRet
RdEVTSEL  
    mrc     p14, 0, r0, c8, c1, 0       ; Read EVTSEL

RRet
    IF :DEF: Interworking
      IF Interworking :LOR: Thumbing
        bx  lr
      ELSE
        mov  pc, lr          ; return
      ENDIF 
    ELSE
      mov  pc, lr          ; return
    ENDIF 
     
    ENDFUNC

;
; WritePMUReg - Writes to the PMU Register
;
; Description:
;   This routine writes to the designated PMU register via CoProcesser 14.
;
; Input Parameters:   
;       r0 - arg1 - PMU register number to write
;       r1 - arg2 - Value to write to PMU register
;
;   if r0 contains:
;       0 -> PMNC,  PMU Control Register
;       1 -> CCNT,  PMU Clock Counter
;       2 -> PMN0,  PMU Count Register 0
;       3 -> PMN1,  PMU Count Register 1
;       4 -> PMN2,  PMU Count Register 2
;       5 -> PMN3,  PMU Count Register 3
;       6 -> INTEN, PMU Interupt Enable Register
;       7 -> FLAG,  PMU Overflow Flag Status Register
;       8 -> EVTSEL PMU Event Select Register
;
; Returns:
;   None
;
; Registers Modified:
;   CoProcessor Register Modified: PMU Register
;   General Purpose Registers Modified: None
;
; NOTE   
;   Error checking not included
;

WritePMUReg  FUNCTION

    cmp     r0, #8
    addls   pc, pc, r0, lsl #2
    b       WRet
    b       WrPMNC
    b       WrCCNT
    b       WrPMN0
    b       WrPMN1
    b       WrPMN2
    b       WrPMN3
    b       WrINTEN
    b       WrFLAG
    b       WrEVTSEL

WrPMNC  
    mcr     p14, 0, r1, c0, c1, 0       ; Write PMNC
    b       WRet
WrCCNT  
    mcr     p14, 0, r1, c1, c1, 0       ; Write CCNT
    b       WRet
WrPMN0  
    mcr     p14, 0, r1, c0, c2, 0       ; Write PMN0
    b       WRet
WrPMN1  
    mcr     p14, 0, r1, c1, c2, 0       ; Write PMN1
    b       WRet
WrPMN2  
    mcr     p14, 0, r1, c2, c2, 0       ; Write PMN2
    b       WRet
WrPMN3  
    mcr     p14, 0, r1, c3, c2, 0       ; Write PMN3
    b       WRet
WrINTEN  
    mcr     p14, 0, r1, c4, c1, 0       ; Write INTEN
    b       WRet
WrFLAG  
    mcr     p14, 0, r1, c5, c1, 0       ; Write FLAG
    b       WRet
WrEVTSEL  
    mcr     p14, 0, r1, c8, c1, 0       ; Write EVTSEL
    
WRet   
    IF :DEF: Interworking
      IF Interworking :LOR: Thumbing
        bx  lr
      ELSE
        mov  pc, lr          ; return
      ENDIF 
    ELSE
      mov  pc, lr          ; return
    ENDIF
    

     
    ENDFUNC

 END 
