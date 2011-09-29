;
; Copyright (c) Microsoft Corporation.  All rights reserved.
;
;
; Use of this source code is subject to the terms of the Microsoft end-user
; license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
; If you did not accept the terms of the EULA, you are not authorized to use
; this source code. For a copy of the EULA, please see the LICENSE.RTF on your
; install media.
;
;TEST_STUB_PWR_OFF EQU 1
;******************************************************************************
;
;  COPYRIGHT (C) 2002 Intel Corporation.
;
;  This software as well as the software described in it is furnished under 
;  license and may only be used or copied in accordance with the terms of the 
;  license. The information in this file is furnished for informational use 
;  only, is subject to change without notice, and should not be construed as 
;  a commitment by Intel Corporation. Intel Corporation assumes no 
;  responsibility or liability for any errors or inaccuracies that may appear 
;  in this document or any software that may be provided in association with 
;  this document.
; 
;  Except as permitted by such license, no part of this document may be 
;  reproduced, stored in a retrieval system, or transmitted in any form or by 
;  any means without the express written consent of Intel Corporation. 
;
;  FILENAME:       xllp_Pm_SleepContextA.s
;
;  PURPOSE:        Provides assembly level code to support context saving, 
;                  processor sleep and context restoration after sleep
;                  for the Bulverde/Mainstone processor/platform.
;                  
;
;  LAST MODIFIED:  8/27/02
;******************************************************************************
;
; The functions in this source code are called via a branch with link instruction.
;
; NOTES:
;
; These functions assume that the MMU is enabled, except for any which receive all addresses
; as parameters.
;
        INCLUDE  xlli_Bulverde_defs.inc                 ; Bulverde specific include file
        INCLUDE  xlli_Mainstone_defs.inc                ; Mainstone specific include file
        INCLUDE  xllp_Pm_SleepContext.inc               ; Definitions for this file
 IF 1=0
        INCLUDE  mainstone.mac                          ; Definitions for this file
 ENDIF ;0 : development LED markers
;
        AREA    |.text|, CODE, READONLY, ALIGN=5        ; Align =5 required for "ALIGN 32" to work
;
; Functions made public by this file:
;
        EXPORT XllpPmEnterSleep
        EXPORT XllpPmChecksumSleepDataVi
        EXPORT XllpPmRestoreAfterSleep                   ; Not called by name elsewhere.

;
; External functions required for this file:
;
        IMPORT XllpPmSleepCLevelProcessing
        IMPORT XllpPmWakeCLevelProcessing

;**************************************************************************************************
;
; XllpPmChecksumSleepDataVi
;
; Does not assume a stack or MMU enable/disable state.
; 
; Inputs: 
;       R0: Base address of area to checksum
;       R1: Number of 4-byte words in area
;       Assumes return address in link register
;
; Returns:
;       R0: Checksum
;
; Uses: R0, R1, R2, R3
;


XllpPmChecksumSleepDataVi  FUNCTION

    ; do Checksum on the buffer

    ldr     r2, =(0x5A72)                  ; Pick a non-zero seed.
XllpPmCheckSumSleepDataVi_Lab1
    ldr     r3, [r0], #4                   ; get value and increment pointer
    add     r2, r2,   r3
    mov     r2, r2,   ROR #31              ; Rotate left by one bit position
    subs    r1, r1,   #1                   ; Count down for entire buffer size
    bne     XllpPmCheckSumSleepDataVi_Lab1
    mov     r0, r2

    IF :DEF: Interworking
        IF Interworking :LOR: Thumbing
         bx  lr
        ELSE
         mov  pc, lr          ; return
        ENDIF ; ELSE of IF Interworking :LOR: Thumbing
    ELSE ; IF :DEF: Interworking
         mov  pc, lr          ; return
    ENDIF ; ELSE of IF :DEF: Interworking

    ENDFUNC

;; End of XllpPmChecksumSleepDataVi()
;**************************************************************************************************


;******************************************************************************
;
;
;       ********************
;       *                  * 
;       * XllpPmEnterSleep *
;       *                  *
;       ********************
;
;
; NOTES: Written for the Bulverde Processor on the Mainstone Development Platform.
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;     
;    XllpPmEnterSleep() is an assembly-coded function that:
;    ·	Saves processor and some coprocessor register context.
;    ·	Performs a checksum and stores it in the saved data area.
;    ·	Performs some final platform-level shutdown.
;    ·	Configures the processor Power Manager sleep state registers.
;    ·	Places the physical address of the saved data area in the PSPR.
;    ·	Puts the processor to sleep. 
; Expects RO to have the parameter structure address
; Prototype:
;    XllpPmEnterSleep(P_XLLP_PM_ENTER_SLEEP_PARAMS_T)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;     

XllpPmEnterSleep   FUNCTION

    ; (This assumes only some trusted processor mode and the existence of stacks for all modes)
    ; 9.2.1:    Save ARM registers
    ; Save r0-r12 + lr (r14) onto current stack (sp=r13)
    stmdb   sp!, {r0-r12,lr}                        ; Push entry mode state onto our stack

    ; Move parameter pointer to r4 for protection from C functions.
    mov     r4, r0 
    ldr     r5, [r4, #SleepParams_SleepDataAreaVA]  ;Get sleep state save area addr

    ;
    ; Protect R4+R5 until further notice
    ; For processor register storage sequence, use r3 only to set up CPSR.
    ;

    ; Store CPSR, sp and (if not SYS mode) SPSR
    mrs     r3, cpsr
    str     r3, [r5, #SleepState_ENTRY_CPSR]
    str     sp, [r5, #SleepState_ENTRY_SP]

    ldr     r2, =xlli_CPSR_Mode_SYS             ; Mbit pattern for SYS mode, which has no SPSR
    and     r1, r3, r2                          ; Isolate "M" bits from CPSR
    cmp     r1, r2                              ; identical? if so, skip nonexistent SPSR
    mrsne   r2, spsr                            ; undefined operation if SPSR doesn't exist.
    strne   r2, [r5, #SleepState_ENTRY_SPSR]

    ;    
    ;    Enter SYS mode and save r8-r12 + lr (r14) onto stack
    ;    Store sp (no SPSR)
    bic     r3, r3, #(xlli_CPSR_Mode_MASK) ; cpsr still in r3
    orr     r3, r3, #(xlli_CPSR_Mode_SYS:OR:xlli_CPSR_I_Bit:OR:xlli_CPSR_F_Bit) ; Enter SYS mode, no interrupts
    msr     cpsr_c, r3                      ; Only interested in control fields
                                            ; Push SYS state onto our stack
    stmdb   sp!, {r8-r12,lr}                ; Save R8 and up, in case entry mode was FIQ
    str     sp, [r5, #SleepState_SYS_SP]
    ;    
    ; Enter each of these modes and save their mode-unique registers (in parentheses)
    ; onto the stack, then store SP and SPSR for eacn.
    ;    
    ;FIQ (r8-r12, r14, SPSR on stack; store SP)
    ; (For FIQ, put Saved PSR into r2 and save r2,r8..12, sp and lr)
    ;  r3 still has CPSR template in it.
    bic     r3, r3, #(xlli_CPSR_Mode_MASK)
    orr     r3, r3, #(xlli_CPSR_Mode_FIQ:OR:xlli_CPSR_I_Bit:OR:xlli_CPSR_F_Bit); Enter FIQ mode, no interrupts
    msr     cpsr_c, r3          ; Only interested in control fields

    mrs     r2, spsr 
    stmdb   sp!, {r2, r8-r12,lr}            ; Save SPSR, LR and R8-R12,
    str     sp, [r5, #SleepState_FIQ_SP]    ; Store SP

    ; IRQ (r14, SPSR on stack; store SP)
    ;  r3 still has CPSR template in it.
    bic     r3, r3, #(xlli_CPSR_Mode_MASK)
    orr     r3, r3, #(xlli_CPSR_Mode_IRQ:OR:xlli_CPSR_I_Bit:OR:xlli_CPSR_F_Bit) ; Enter IRQ mode, no interrupts
    msr     cpsr_c, r3 ; Only interested in control fields

    mrs     r2, spsr 
    stmdb   sp!, {r2, lr}                   ; Save SPSR and LR
    str     sp, [r5, #SleepState_IRQ_SP]    ; Store SP

    ; ABT (r14, SPSR on stack; store SP)
    ; r3 still has CPSR template in it.
    bic     r3, r3, #(xlli_CPSR_Mode_MASK)
    orr     r3, r3, #(xlli_CPSR_Mode_ABT:OR:xlli_CPSR_I_Bit:OR:xlli_CPSR_F_Bit) ; Enter ABT mode, no interrupts
    msr     cpsr_c, r3 ; Only interested in control fields

    mrs     r2, spsr 
    stmdb   sp!, {r2, lr}                   ; Save SPSR and LR
    str     sp, [r5, #SleepState_ABT_SP]    ; Store SP

    ; UND (r14, SPSR on stack; store SP)
    ; r3 still has CPSR template in it.
    bic     r3, r3, #(xlli_CPSR_Mode_MASK)
    orr     r3, r3, #(xlli_CPSR_Mode_UND:OR:xlli_CPSR_I_Bit:OR:xlli_CPSR_F_Bit) ; Enter UND mode, no interrupts
    msr     cpsr_c, r3 ; Only interested in control fields

    mrs     r2, spsr 
    stmdb   sp!, {r2, lr}                   ; Save SPSR and LR
    str     sp, [r5, #SleepState_UND_SP]    ; Store SP

    ; SVC (r14, SPSR on stack; store SP)
    ; r3 still has CPSR template in it.
    bic     r3, r3, #(xlli_CPSR_Mode_MASK)
    orr     r3, r3, #(xlli_CPSR_Mode_SVC:OR:xlli_CPSR_I_Bit:OR:xlli_CPSR_F_Bit) ; Enter SVC mode, no interrupts
    msr     cpsr_c, r3 ; Only interested in control fields

    mrs     r2, spsr 
    stmdb   sp!, {r2, lr}                   ; Save SPSR and LR
    str     sp, [r5, #SleepState_SVC_SP]    ; Store SP


    ; (Don't explicitly save USR context; there are no unique USR mode registers; 
    ;       any registers not shared with other modes are shared with SYS mode.)

    ; Continue from here using SVC mode's stack, now that all  processor registers are saved.

    ;
    ; End r3 reserved sequence; continue to protect r4, r5
    ;
    ; r5 is pointer to sleep save data area

    ; Cp15_ACR_MMU
    mrc     p15, 0, r0, c1, c0, 0               ; load r0 with MMU Control
    ldr     r1, =XLLP_CP15_ACR_MMU_VLD_MSK      ; mask off the undefined bits
    and     r0, r0, r1
    str     r0, [r5, #SleepState_Cp15_ACR_MMU]  ; store MMU Control data

    ; Cp15_AUXCR_MMU;   // cp15 Reg1:1: assume restored elsewhere.
    mrc     p15, 0, r0, c1, c1, 0               ; load r0 with MMU Aux Control
    ldr     r1, =XLLP_CP15_AUXCR_MMU_VLD_MSK    ; mask off the undefined bits
    and     r0, r0, r1
    str     r0, [r5, #SleepState_Cp15_AUXCR_MMU] ; store MMU Aux Control data

    ; Cp15_TTBR_MMU;    // cp15 Reg2:0
    mrc     p15, 0, r0, c2, c0, 0               ; load r0 with TTB address.
    ldr     r1, =XLLP_CP15_TTBR_MMU_VLD_MSK     ; mask off the undefined bits
    and     r0, r0, r1
    str     r0, [r5, #SleepState_Cp15_TTBR_MMU] ; store TTB address

    ; Cp15_DACR_MMU;    // cp15 Reg3:0, all bits valid
    mrc     p15, 0, r0, c3, c0, 0               ; load r0 with domain access control.
    str     r0, [r5, #SleepState_Cp15_DACR_MMU] ; store domain access control

    ; Cp15_PID_MMU;              // cp15 Reg13; Assume set by OS if used.
    mrc     p15, 0, r0, c13, c0, 0              ; load r0 with PID.
    ldr     r1, =XLLP_CP15_PID_MMU_VLD_MSK      ; mask off the undefined bits
    and     r0, r0, r1
    str     r0, [r5, #SleepState_Cp15_PID_MMU]  ; store PID
    ;
    ; Save Coprocessor Access Register value before modifying to get CP0
    ;  Cp15_CPAR;      // cp15 Reg15
    mrc     p15, 0, r0, c15, c1, 0              ; load r0 with Coprocessor Access Register.
    ldr     r1, =XLLP_CP15_CPAR_VLD_MSK         ; mask off the undefined bits
    and     r0, r0, r1
    str     r0, [r5, #SleepState_Cp15_CPAR]     ; store Coprocessor Access Register
    ; Now enable access to all valid coprocessors
    mcr     p15, 0, r1, c15, c1, 0       
    ;    CPWAIT  $R0
    
    mrc     p15, 0, r0, c2, c0, 0               ; arbitrary read of CP15
    mov     r0, r0                              ; wait for it (foward dependency)
    sub     pc, pc, #4                          ; branch to next instruction

    ;; Moving to C-level code for everything except the final sleep command.

    mov     r0, r4  ; C function expects the parameter pointer we got.

    stmdb   sp!, {lr}
    bl      XllpPmSleepCLevelProcessing 
    ldmia   sp!, {lr}

    nop
    nop
    nop
    nop

    ;    r0: contains start address
    ;    r2: contains count
    ;    uses: r2,r3,r4,r5,r6

 
    ;  Enter specified power mode
    ldr     r3, [r4, #SleepParams_PWRMODE]

    b   XllpPmEnterSleep_Lab1     ; Purely precautionary, in case we needed to clear pipeline
    ALIGN  32
XllpPmEnterSleep_Lab1
    mcr     p14, 0, r3, c7, c0, 0                   ; Enter sleep

    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop

    ;------------------------------------------------------------------------
    ; We are now down (and should never get here ...)

; Skip for now.
;    IF BSP_MAINSTONE = "1"
;
;        ldr     r1, =(FPGA_REGS_BASE_U_VIRTUAL)
;        mov     r0, #0
;        ldr     r0, [r1, #BLANKLED_OFFSET]   ; Enable all LEDs
;        ldr     r0, =(0x88880666)            ; Failure code for non-sleeping
;        str     r0, [r1, #HEXLED_OFFSET]
;
;    ENDIF; :DEF: BSP_MAINSTONE

XllpPmEnterSleep_Lab2
    b       XllpPmEnterSleep_Lab2                            ; don't let it fall through

    IF :DEF: Interworking
        IF Interworking :LOR: Thumbing
         bx  lr
        ELSE
         mov  pc, lr          ; return
        ENDIF ; ELSE of IF Interworking :LOR: Thumbing
    ELSE ; IF :DEF: Interworking
         mov  pc, lr          ; return
    ENDIF ; ELSE OF IF :DEF: Interworking

    ENDFUNC


;; End of XllpPmEnterSleep()
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; XllpPmRestoreAfterSleep()
;
; Expects:
;    R0: Virtual address of of the saved data area (XllpPmSleepDataArea).  
;         - Physical address was  obtained from the PSPR at wakeup.
;    R1: RCSR (bits 15:0 in 15:0 of R1) + PSSR (bits 15:0 in 31:16 of R1)
;
;    Assumes virtual mode and that all MMU registers are already restored.

    LTORG
XllpPmRestoreAfterSleep   FUNCTION

; Move sleep data area virtual address in to r5, same as before sleep
; Save wakeup status data from r1 into r6

    mov     r5, r0
    mov     r6, r1
    ldr     r4, [r5, #SleepState_SleepParamVA]    ; Get parameter struct addr

;   Protect r4, r5 and r6 until further notice.


;Enter and restore the register contexts for these processor modes:

;   Use r3 for CPSR template until further notice.

;Enter SYS mode and restore r8-r12 + lr (r14) from stack
;     (no SPSR)
;    
    mrs     r3, cpsr
    bic     r3, r3, #(xlli_CPSR_Mode_MASK)
    orr     r3, r3, #(xlli_CPSR_Mode_SYS:OR:xlli_CPSR_I_Bit:OR:xlli_CPSR_F_Bit) ; Enter SYS mode, no interrupts
    msr     cpsr_c, r3                      ; Only interested in control fields
    ldr     sp, [r5, #SleepState_SYS_SP]    ; Get saved  sp
                                            ; Pop SYS state from our stack
    ldmia   sp!, {r8-r12,lr}                ; Restore R8 and up - no SPSR


;   FIQ (r8-r12, r14, SPSR on stack; store SP)
    ;    // For FIQ, get Saved PSR into r2 and restore ,r8..12, sp and lr)
    ;  r3 still has CPSR template in it.
    bic     r3, r3, #(xlli_CPSR_Mode_MASK)
    orr     r3, r3, #(xlli_CPSR_Mode_FIQ:OR:xlli_CPSR_I_Bit:OR:xlli_CPSR_F_Bit) ; Enter FIQ mode, no interrupts
    msr     cpsr_c, r3 ; Only interested in control fields
    ldr     sp, [r5, #SleepState_FIQ_SP]    ; Get saved  sp
    ldmia   sp!, {r2, r8-r12,lr}            ; Saved SPSR, LR and R8-R12,
    msr     spsr_cxsf, r2  

;  IRQ (r14, SPSR on stack; stored SP)
    ;  r3 still has CPSR template in it.
    bic     r3, r3, #(xlli_CPSR_Mode_MASK)
    orr     r3, r3, #(xlli_CPSR_Mode_IRQ:OR:xlli_CPSR_I_Bit:OR:xlli_CPSR_F_Bit) ; Enter IRQ mode, no interrupts
    msr     cpsr_c, r3 ; Only interested in control fields
    ldr     sp, [r5, #SleepState_IRQ_SP]    ; Stored SP
    ldmia   sp!, {r2, lr}                   ; Saved SPSR and LR
    msr     spsr_cxsf, r2  

;   ABT (r14, SPSR on stack; stored SP)
    ;  r3 still has CPSR template in it.
    bic     r3, r3, #(xlli_CPSR_Mode_MASK)
    orr     r3, r3, #(xlli_CPSR_Mode_ABT:OR:xlli_CPSR_I_Bit:OR:xlli_CPSR_F_Bit) ; Enter ABT mode, no interrupts
    msr     cpsr_c, r3 ; Only interested in control fields
    ldr     sp, [r5, #SleepState_ABT_SP]    ; Stored SP
    ldmia   sp!, {r2, lr}                   ; Saved SPSR and LR
    msr     spsr_cxsf, r2  

;   UND (r14, SPSR on stack; stored SP)
    ;  r3 still has CPSR template in it.
    bic     r3, r3, #(xlli_CPSR_Mode_MASK)
    orr     r3, r3, #(xlli_CPSR_Mode_UND:OR:xlli_CPSR_I_Bit:OR:xlli_CPSR_F_Bit) ; Enter UND mode, no interrupts
    msr     cpsr_c, r3 ; Only interested in control fields
    ldr     sp, [r5, #SleepState_UND_SP]    ; Stored SP
    ldmia   sp!, {r2, lr}                   ; Saved SPSR and LR
    msr     spsr_cxsf, r2  


; Enter SVC mode and restore only its saved sp.

;   SVC (r14, SPSR on stack; store SP)
    ;  r3 still has CPSR template in it.
    bic     r3, r3, #(xlli_CPSR_Mode_MASK)
    orr     r3, r3, #(xlli_CPSR_Mode_SVC:OR:xlli_CPSR_I_Bit:OR:xlli_CPSR_F_Bit) ; Enter SVC mode, no interrupts
    msr     cpsr_c, r3 ; Only interested in control fields
    ldr     sp, [r5, #SleepState_SVC_SP]    ; Stored SP

; (Don't explicitly restore USR context; there are no unique USR mode registers; 
;       any registers not shared with other modes are shared with SYS mode.)

;   ==============
;   Leaving SVC mode context unrestored, use its stack for
;   general restoration processing
;   End protection of r3
;   ==============

    ;; Moving to C-level code for everything except CPAR restoration, SVC
    ;    restoration and original mode restoration

    ; Enable access to all valid coprocessors  (needed for restore CP0+1 in XllpPmWakeCLevelProcessing()
    ldr     r1, =XLLP_CP15_CPAR_VLD_MSK     ; mask off the undefined bits
    mcr     p15, 0, r1, c15, c1, 0      
    ;    CPWAIT  $R0

    mrc     p15, 0, r0, c2, c0, 0           ; arbitrary read of CP15
    mov     r0, r0                          ; wait for it (foward dependency)
    sub     pc, pc, #4                      ; branch to next instruction

    mov     r0, r4  ; C function expects the parameter pointer
    mov     r1, r5  ; and pointer to saved data area
    mov     r2, r6  ; and Wakeup info from startup
    bl      XllpPmWakeCLevelProcessing 

    ldr     r0, [r5, #SleepState_Cp15_CPAR] ; Get saved Coproc Access Reg
    mcr     p15, 0, r0, c15, c1, 0          ; restore Coprocessor Access Register.


;   Still in SVC mode
;   Restore unique SVC register context
    ldmia   sp!, {r2, lr}                   ; Saved SPSR and LR
    msr     spsr_cxsf, r2  

;  Re-establish whatever mode was in use at the time XllpPmEnterSleep() was
;  invoked and restore complete register context.  Before restoring the SPSR, 
;  make sure that the entry mode was not SYS mode, which has no SPSR.

;   Load CPSR, sp and (if not SYS mode) SPSR
    ldr     r3, [r5, #SleepState_ENTRY_CPSR]
    msr     cpsr_c, r3 ; Only interested in control fields for now, will trash flags
    ldr     r2, =xlli_CPSR_Mode_SYS         ; Mbit pattern for SYS mode, which has no SPSR
    and     r3, r3, r2                      ; Isolate "M" bits from CPSR
    cmp     r3, r2                          ; identical? if so, skip nonexistent SPSR
    ldrne   r2, [r5, #SleepState_ENTRY_SPSR]
    msrne   spsr_cxsf, r2  
    ldr     r3, [r5, #SleepState_ENTRY_CPSR] 
    msr     cpsr_cxsf, r3                   ; Restore original mode status and flags
    ldr     sp, [r5, #SleepState_ENTRY_SP]

    ldmia   sp!, {r0-r12,lr}                ; Pop entry mode state from stack

;  Branch to lr location (Caller sees this as return from XllpPmEnterSleep()).

    IF :DEF: Interworking
        IF Interworking :LOR: Thumbing
         bx  lr
        ELSE
         mov  pc, lr          ; return
        ENDIF ; ELSE of IF Interworking :LOR: Thumbing
    ELSE ; IF :DEF: Interworking
         mov  pc, lr          ; return
    ENDIF ; ELSE of IF :DEF: Interworking

    ENDFUNC

;; End of XllpPmRestoreAfterSleep()
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    IF 0=1

;**************************************************************************************************

XllpPmDumpMemLEDs_2   FUNCTION
;    r0: contains start address
;    r2: contains count
;    uses: r2,r3,r4,r5,r6

    mov r4, r0
    ldr r3, =(FPGA_REGS_BASE_U_VIRTUAL)

XllpPmDumpMemLEDs_LabOuter

    ldr r5, [r4]
    setHexLED   r3, r5
    ldr     r6, =0x18000000;
XllpPmDumpMemLEDs_LabInner

    subs    r6, r6, #1
    bne     XllpPmDumpMemLEDs_LabInner

; add marker in case of repeated identical data (like 0)
;; cjw.  Add separator in case of duplicate data.

    ldr r5, =(0x77777777)
    setHexLED   r3, r5
    ldr     r6, =0x400000;
XllpPmDumpMemLEDs_LabInner2

    subs    r6, r6, #1
    bne     XllpPmDumpMemLEDs_LabInner2


    add     r4, r4, #4
    subs    r2, r2, #1
    bne     XllpPmDumpMemLEDs_LabOuter

    IF :DEF: Interworking
        IF Interworking :LOR: Thumbing
         bx  lr
        ELSE
         mov  pc, lr          ; return
        ENDIF ; ELSE of IF Interworking :LOR: Thumbing
    ELSE ; IF :DEF: Interworking
         mov  pc, lr          ; return
    ENDIF ; ELSE OF IF :DEF: Interworking

    ENDFUNC

;; end of XllpPmDumpMemLEDs_2
;**************************************************************************************************


    ENDIF ; 0=1


    END
  
