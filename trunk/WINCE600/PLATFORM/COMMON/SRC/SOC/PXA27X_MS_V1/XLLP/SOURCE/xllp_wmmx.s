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
;******************************************************************************
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
;*********************************************************************************
;
;  FILENAME:       xllp_wmmx.s
;
;  PURPOSE:        Provides assembly level code to support thread level
;                  context saving and restoration for the
;                  Bulverde processor's WMMX registers.
;
;  LAST MODIFIED:  26-Jul-2005 AAG - fix the LDC/STC - need to use p1 instead of p0
;                  22-Nov-2004 AAG  - changed load/store to wldr/wstr equivalent, but used LDC/STC to make
;              code assemble one more tool chains.  also change tmcr/tmrc to generic
;              mcr/mrc to allow assembly on more tool chains.
;
;******************************************************************************
;
; NOTES:
;
;    None.
;
;******************************************************************************

        AREA    |.text|, CODE, READONLY, ALIGN=5        ; Align =5 required for "ALIGN 32" to work
;
; List of Low Level Init functions in this source code include:
;
        EXPORT Xllp_Read_CoProc_Access
        EXPORT Xllp_Set_CoProc_Access
        EXPORT Xllp_Save_WMMX_Regs
        EXPORT Xllp_Restore_WMMX_Regs
        EXPORT Xllp_Store_All_WMMX_Regs
        EXPORT Xllp_Restore_All_WMMX_Regs


;******************************************************************************
;
;       ***************************
;       *                         *
;       * Xllp_Read_CoProc_Access *
;       *                         *
;       ***************************
;
; This routine is used to return the coprocessor access bits located in
; coprocessor 15, register 15.  It returns it to the location specified in
; r0
;
;******************************************************************************
Xllp_Read_CoProc_Access  FUNCTION

        stmdb   sp!, {r0 - r2, r14}

        mrc     p15, 0, r1, c15, c1, 0    ;Get Reg15 of CP15 for Access to CP0

        ;CPWAIT  r2                       ;Now 'stall' so the value can have time to be read
        MRC     P15, 0, r2, C2, C0, 0     ; arbitrary read of CP15
        MOV     r2, r2                    ; wait for it (foward dependency)
        SUB     PC, PC, #4                ; branch to next instruction

        str  r1, [r0]
        ldmia   sp!, {r0 - r2, r14}

        IF Interworking :LOR: Thumbing
            bx  lr
        ELSE
            mov  pc, lr          ; return
        ENDIF ; IF Interworking :LOR: Thumbing

        ENDFUNC


;******************************************************************************
;
;       ***************************
;       *                         *
;       * Xllp_Set_CoProc_Access  *
;       *                         *
;       ***************************
;
;
; This routine enables the access bits to Coprocessors 0 & 1
;
; Uses R0, R1. R2
;
;******************************************************************************
Xllp_Set_CoProc_Access  FUNCTION

        stmdb   sp!, {r0 - r2, r14}

        mrc     p15, 0, r0, c15, c1, 0    ;Get Reg15 of CP15 for Access to CP0
        mov     r1, #0x3                  ;Load R2 with mask for setting lowest bit
        orr     r2, r0, r1                ;OR current value with 1 to set the lowest bit
        mcr     p15, 0, r2, c15, c1, 0    ;Now set the value back into R15 of CP15

        ;CPWAIT  r0                       ;Now 'stall' so the value can have time to be written
        MRC     P15, 0, r0, C2, C0, 0     ; arbitrary read of CP15
        MOV     r0, r0                    ; wait for it (foward dependency)
        SUB     PC, PC, #4                ; branch to next instruction

        ldmia   sp!, {r0 - r2, r14}

        IF Interworking :LOR: Thumbing
            bx  lr
        ELSE
            mov  pc, lr          ; return
        ENDIF ; IF Interworking :LOR: Thumbing

        ENDFUNC


;******************************************************************************
;
;       ***************************
;       *                         *
;       * Xllp_Save_WMMX_Regs     *
;       *                         *
;       ***************************
;
;
; This routine is expected to be the entry point for the "SaveAll".  It checks
; the CUP and MUP bits in the control register to see if we need to save, if it
; is unnecessary, then we simply dump out of the routine.  If it is necessary,
; it sets a flag at the start of the save area and calls "SaveAll" to save all
; the WMMX registers.
;
; Uses r0, - R0 contains a pointer to the alloced memory buffer of 8 bytes long
;
;******************************************************************************
Xllp_Save_WMMX_Regs   FUNCTION

        stmdb   sp!, {r0 - r4, r14}         ;Store registers so we don't stomp anything (INC LR)

        mov     r4, r0                    ;Save R0 to R4, so pointer value doesn't get killed by CPWAIT

        mrc     p1, 0, r1, c1, c0, 0      ;Grab the CUP & MUP bits in CP1, Reg1

        ;CPWAIT  r0                       ;Now 'stall' so the value can have time to be written
        MRC     P15, 0, r0, C2, C0, 0     ; arbitrary read of CP15
        MOV     r0, r0                    ; wait for it (foward dependency)
        SUB     PC, PC, #4                ; branch to next instruction

        ands    r1, r1, #0x3              ;We only are concerned with the lowest 2 bits

        ;if flag == 0 then no change since last save, skip save functionality
        beq  Finish_Save

        mov     r0, #0x1                  ;Set a flag in the memory area indicating we have actually SAVED
        str     r0, [r4], #4              ;Save & increment pointer

        ; Now branch to the 'save' function, R0 = pointer to save area
        mov     r0, r4
        bl      Xllp_Store_All_WMMX_Regs

Finish_Save
        ldmia   sp!, {r0 - r4, r14}         ;Now restore the regsiters we stacked3

        IF Interworking :LOR: Thumbing
            bx  lr
        ELSE
            mov  pc, lr          ; return
        ENDIF ; IF Interworking :LOR: Thumbing


        ENDFUNC

;******************************************************************************
;
;       ***************************
;       *                         *
;       * Xllp_Restore_WMMX_Regs  *
;       *                         *
;       ***************************
;
; This function is used to restore the WMMX registers.  It checks the flags saved in
; the beginning of the saved area to see if a restore is necessary.  If it is unnecessary
; the routine dumps out, otherwise it calls the "RestoreAll" function.
;
;       Uses r0 - pointer to the memory buffer containing the WMMX saved area
;
;******************************************************************************
Xllp_Restore_WMMX_Regs   FUNCTION

        stmdb   sp!, {r0 - r4, r14}         ;stack the registers used, so don't munge anything

        ldr     r1, [r0], #4              ;Load the first 4 bytes containing 'if saving' flag
        cmp     r1, #0
        beq     Skip_Restore

        ;Branch to the 'Restore' Function, R0= pointer to save/restore memory area
        bl      Xllp_Restore_All_WMMX_Regs

Skip_Restore
        ldmia     sp!, {r0 - r4, r14}

        IF Interworking :LOR: Thumbing
            bx  lr
        ELSE
            mov  pc, lr          ; return
        ENDIF ; IF Interworking :LOR: Thumbing


        ENDFUNC


;******************************************************************************
;
;       *******************************
;       *                             *
;       * Xllp_Store_All_WMMX_Regs    *
;       *                             *
;       *******************************
;
;
;
; Stores all WMMX Registers as listed below
; Saving:
;  CP0, R0 - 15
;  CP1, R2, R3, R8 - 11
;
;  NOTE:  This routine was written to optimize both the read performance and
;  the register usage.  It was designed against the EAS stated instruction
;  latencies for tmrc and tmrrc instructions in hopes of keeping the
;  connection between the coprocessor and core maximized with respect to bandwitdh.
;  As such, the registes are not saved in a completely linear fashion, they are
;  interleaved.  As a result, the restore must treat these like a stack and pop
;  off the registers in a similar order.
;
;******************************************************************************
Xllp_Store_All_WMMX_Regs FUNCTION

    ;; trick - if addr is not double word aligned, save a word to align it
    mov r1,r0
    ands r2,r0,#7
    beq sskip1
    stc2 p1, c2, [r1],#4    ;; this instruction is skipped if word aligned
sskip1
    stcl p1, c0,   [r1],#8
    stcl p1, c1,   [r1],#8
    stcl p1, c2,   [r1],#8
    stcl p1, c3,   [r1],#8
    stcl p1, c4,   [r1],#8
    stcl p1, c5,   [r1],#8
    stcl p1, c6,   [r1],#8
    stcl p1, c7,   [r1],#8
    stcl p1, c8,   [r1],#8
    stcl p1, c9,   [r1],#8
    stcl p1, c10,  [r1],#8
    stcl p1, c11,  [r1],#8
    stcl p1, c12,  [r1],#8
    stcl p1, c13,  [r1],#8
    stcl p1, c14,  [r1],#8
    stcl p1, c15,  [r1],#8
    bne sskip2             ;; this instruction depend on condition
                       ;; set by ANDs above
    stc2 p1, c2, [r1],#4   ;; this instruction is executed if word aligned
sskip2
    stc2 p1, c3, [r1],#4
    stc2 p1, c8, [r1],#4
    stc2 p1, c9, [r1],#4
    stc2 p1, c10, [r1],#4
    stc2 p1, c11, [r1],#4

        ; Now clear the control MUP & CUP bits (Control Update Bits)
        ; These are WRITE 1 TO CLEAR!
        mov     r3, #0x3                  ;Set the 2 lowest bits == 1
        mcr     p1, 0, r3, c1, c0, 0      ;Now Clear the CUP & MUP bits

        ;CPWAIT  r0
        MRC     P15, 0, r0, C2, C0, 0     ; arbitrary read of CP15
        MOV     r2, r0                    ; wait for it (foward dependency)
        SUB     PC, PC, #4                ; branch to next instruction

        IF Interworking :LOR: Thumbing
            bx  lr
        ELSE
            mov  pc, lr          ; return
        ENDIF ; IF Interworking :LOR: Thumbing

        ENDFUNC

;******************************************************************************
;
;       *******************************
;       *                             *
;       * Xllp_Restore_All_WMMX_Regs  *
;       *                             *
;       *******************************
;
;
;
; Restores all WMMX Registers saved by the above Store function
; Restoring:
;  CP0, R0 - 15
;  CP1, R2, R3, R8 - 11
;
;  NOTE:  This routine was written to optimize both the read performance and
;  the register usage.  It was designed against the EAS stated instruction
;  latencies for tmcr and tmcrr instructions in hopes of keeping the
;  connection between the coprocessor and core maximized with respect to bandwitdh.
;  As such, the registes are not saved in a completely linear fashion, they are
;  interleaved.  Due to the order saved, the restore order is also not sequential.
;
;******************************************************************************
Xllp_Restore_All_WMMX_Regs FUNCTION

    ;; trick - if addr is not double word aligned, save a word to align it
    mov r1,r0
    ands r2,r0,#7
    beq rskip1
    ldc2 p1, c2, [r1],#4    ;; this instruction is skipped if word aligned
rskip1
    ldcl p1, c0,   [r1],#8
    ldcl p1, c1,   [r1],#8
    ldcl p1, c2,   [r1],#8
    ldcl p1, c3,   [r1],#8
    ldcl p1, c4,   [r1],#8
    ldcl p1, c5,   [r1],#8
    ldcl p1, c6,   [r1],#8
    ldcl p1, c7,   [r1],#8
    ldcl p1, c8,   [r1],#8
    ldcl p1, c9,   [r1],#8
    ldcl p1, c10,  [r1],#8
    ldcl p1, c11,  [r1],#8
    ldcl p1, c12,  [r1],#8
    ldcl p1, c13,  [r1],#8
    ldcl p1, c14,  [r1],#8
    ldcl p1, c15,  [r1],#8
    bne rskip2             ;; this instruction depend on condition
                       ;; set by ANDs above
    ldc2 p1, c2, [r1],#4   ;; this instruction is executed if word aligned
rskip2
    ldc2 p1, c3, [r1],#4
    ldc2 p1, c8, [r1],#4
    ldc2 p1, c9, [r1],#4
    ldc2 p1, c10, [r1],#4
    ldc2 p1, c11, [r1],#4

        ; Now clear the control MUP & CUP bits (Control Update Bits)
        ; These are WRITE 1 TO CLEAR!
        mov     r1, #0x3                  ;Set the 2 lowest bits == 1
        mcr     p1, 0, r1, c1, c0, 0      ;Now Clear the CUP & MUP bits

        ;CPWAIT  r2
        MRC     P15, 0, r2, C2, C0, 0     ; arbitrary read of CP15
        MOV     r2, r2                    ; wait for it (foward dependency)
        SUB     PC, PC, #4                ; branch to next instruction


        IF Interworking :LOR: Thumbing
            bx  lr
        ELSE
            mov  pc, lr          ; return
        ENDIF ; IF Interworking :LOR: Thumbing


        ENDFUNC


        END
