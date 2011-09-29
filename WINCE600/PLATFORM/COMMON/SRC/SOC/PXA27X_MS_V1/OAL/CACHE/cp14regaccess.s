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

;      TITLE("CoProcessor 14 Read / Write")
;++
;
; Copyright (c) 2001  Intel Corporation
;
;--

    OPT     2                               ; disable listing

    INCLUDE kxarm.h

    OPT     1                               ; reenable listing

    TEXTAREA

;
; ReadPMURegister - Read the PMU Register
;
; This routine reads the designated PMU register via CoProcesser 14.
;
;       Uses r0 - arg1 - PMU register number to read (0-3)
;            r0 - integer result - value read in register
;    
;

    LEAF_ENTRY ReadPMURegister

    cmp     r0, #3
    bhi     RRet
    cmp     r0, #0
    bne     RReg1
    mrc     p14, 0, r0, c0, c0, 0
    b       RRet
RReg1 
    cmp     r0, #1
    bne     RReg2
    mrc     p14, 0, r0, c1, c0, 0
    b       RRet
RReg2
    cmp     r0, #2
    bne     RReg3
    mrc     p14, 0, r0, c2, c0, 0
    b       RRet
RReg3
    cmp     r0, #3
    bne     RRet
    mrc     p14, 0,  r0, c3, c0, 0
RRet
      IF Interworking :LOR: Thumbing
         bx  lr
      ELSE
         mov  pc, lr          ; return
      ENDIF


;
; WritePMURegister - Writes to the PMU Register
;
; This routine writes to the designated PMU register via CoProcesser 14.
;
;       Uses    
;            r0 - arg1 - PMU register number to write (0-3)
;            r1 - arg2 - Value to write to PMU register
;

    LEAF_ENTRY WritePMURegister

    stmfd   sp!, {r1} 

    cmp     r0, #3
    bhi     WRet
    cmp     r0, #0
    bne     WReg1
    mcr     p14, 0, r1, c0, c0, 0
    b       WRet
WReg1 
    cmp     r0, #1
    bne     WReg2
    mcr     p14, 0, r1, c1, c0, 0
    b       WRet
WReg2
    cmp     r0, #2
    bne     WReg3
    mcr     p14, 0, r1, c2, c0, 0
    b       WRet
WReg3
    cmp     r0, #3
    bne     WRet
    mcr     p14, 0,  r1, c3, c0, 0
WRet
    ldmfd   sp!, {r1} 
   
      IF Interworking :LOR: Thumbing
         bx  lr
      ELSE
         mov  pc, lr          ; return
      ENDIF

    END
