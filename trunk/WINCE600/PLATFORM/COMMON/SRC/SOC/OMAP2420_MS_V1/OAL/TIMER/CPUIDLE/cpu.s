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
; Portions Copyright (c) Texas Instruments.  All rights reserved. 
;
; Use of this source code is subject to the terms of the Microsoft end-user
; license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
; If you did not accept the terms of the EULA, you are not authorized to use
; this source code. For a copy of the EULA, please see the LICENSE.RTF on your
; install media.
;
;------------------------------------------------------------------------------

        INCLUDE kxarm.h


        EXPORT OALCPUIdle
        EXPORT OALCPUIdle_sz
        
        TEXTAREA

;-------------------------------------------------------------------------------
;
;  Function:  OALCPUIdle
;
;
;  This routine is called by the kernel when there are no threads ready to
;  run. The CPU should be put into a reduced power mode and halted. It is
;  important to be able to resume execution quickly upon receiving an interrupt.

; BSP_CPLD_REGS_PA 0x08000000 = VA 0x88400000 = UA 0xA8400000
; LED offset 0x16 = UA 0xA8400016
;Debug LED can be written to as follows
;ldr   r0, =0xA8400016
;ldr   r1, =0xF1 ;LED_REG_VAL
;strh  r1, [r0]

;Context is saved at the beginning of SRAM
ARM_CONTEXT_SAVE_ADDRESS            EQU 0xA8200000
OMAP2420_PRCM_GENERAL_PURPOSE14     EQU 0xA86080E4
DCACHE_MASK                         EQU 0x00000004
IRQ_MASK                            EQU 0x80        ; IRQ mask value
FIQ_MASK                            EQU 0x40        ; FIQ mask value
MODE_MASK                           EQU 0x1F        ; Processor Mode Mask
TBIT_MASK                           EQU 0x20        ; Thumb mode bit mask (bit 5 of CPSR/SPSR)
SUP_MODE                            EQU 0x13        ; Supervisor Mode
FIQ_MODE                            EQU 0x11        ; Fast Interrupt Mode (FIQ)
IRQ_MODE                            EQU 0x12        ; Interrupt Mode (IRQ)
ABORT_MODE                          EQU 0x17        ; Abort Mode
USR_MODE                            EQU 0x10        ; User Mode

 
    ALIGN   1024                    ; Align on 1KB boundary

    LEAF_ENTRY OALCPUIdle

    ;save registers on stack
    stmfd	sp!, {r0 - r12, lr}

; This Wait for Interrupt should be called when the context is to be saved and restored with the
; CORE domain going into Retention during idle.
WaitForInterruptCRr
    B       SaveContext
SaveContext
    LDR     R3,  =ARM_CONTEXT_SAVE_ADDRESS     ; load SDRAM store location into R3
    
    LDR     R4,  =OMAP2420_PRCM_GENERAL_PURPOSE14      ; load the GPR14 register address into R4
    ADR     R5,  _Restore             ; load the _Restore address into R5
    STR     R5,  [R4]                 ; store the _Restore address into GPR14
            
    ADR     R4,  _Restore             ; load _Restore address into R4
    MRS     R5,  CPSR                 ; move CPSR value into R5
    MOV     R6,  SP                   ; move SP value into R6
    MOV     R7,  LR                   ; move LR value into R7
    MRS     R8,  SPSR                 ; move SPSR value into R8
    STMIA   R3!, {R4-R8}              ; store into SDRAM
    MRC     p15, 0, R4, c1, c0, 1  ; Auxiliary Control Register
    MRC     p15, 0, R5, c1, c0, 2  ; Coprocessor access Control Register
    MRC     p15, 0, R6, c2, c0, 0  ; TTBR0 
    MRC     p15, 0, R7, c2, c0, 1  ; TTBR1  
    MRC     p15, 0, R8, c2, c0, 2  ; TTBR Control
    MRC     p15, 0, R9, c3, c0, 0  ; Domain Access  
    MRC     p15, 0, R10,c5, c0, 0  ; DFSR 
    MRC     p15, 0, R11,c5, c0, 1  ; IFSR
    STMIA   R3!, {R4-R11}             ; store into SDRAM
    MRC     p15, 0, R4, c6, c0, 0  ; DFAR 
    MRC     p15, 0, R5, c6, c0, 1  ; IFAR  
    MRC     p15, 0, R6, c9, c0, 0  ; Data Cache Lockdown 
    MRC     p15, 0, R7, c9, c0, 1  ; ICache Lockdown 
    MRC     p15, 0, R8, c10,c0, 0  ; TLB Lockdown
    MRC     p15, 0, R9, c13,c0, 0  ; FCSE PID Register
    MRC     p15, 0, R10,c13,c0, 1  ; Context ID Register
    MRC     p15, 0, R11,c15,c2, 4  ; Peripheral Port remap register   
    STMIA   R3!, {R4-R11}          ; store into SDRAM
    MRC     p15, 0, R4, c1, c0, 0  ; Co-processor control register
    STR     R4,  [R3], #4          ; store into SDRAM
    
    ; check if the d-cache is ON, if so then clean it
    ; note: this should not be necessary when Cache memories are retained
    
    TST     R4,  #DCACHE_MASK
    MOVNE   R4,  #0
    MCRNE   p15, 0, R4, c7, c10, 0
    
    ; store the SP and LR for FIQ, IRQ, ABORT, and SUP modes  
                               
    MOV     R4,   #0xD1                             ;#FIQ_MODE | IRQ_MASK | FIQ_MASK
    MSR     CPSR, R4                                ; go into FIQ mode
    MOV     R4,   SP                                ; move SP into R4
    MOV     R5,   LR                                ; move LR into R5
    STMIA   R3!,  {R4-R5}                           ; store into SDRAM
    MOV     R4,   #0xD2                             ;#IRQ_MODE | IRQ_MASK | FIQ_MASK
    MSR     CPSR, R4                                ; go into IRQ mode
    MOV     R4,   SP                                ; move SP into R4
    MOV     R5,   LR                                ; move LR into R5
    STMIA   R3!,  {R4-R5}                           ; store into SDRAM
    MOV     R4,   #0xD7                             ;#ABORT_MODE | IRQ_MASK | FIQ_MASK
    MSR     CPSR, R4                                ; go into ABORT mode
    MOV     R4,   SP                                ; move SP into R4
    MOV     R5,   LR                                ; move LR into R5
    STMIA   R3!,  {R4-R5}                           ; store into SDRAM
    MOV     R4,   #0xD3                             ;#SUP_MODE | IRQ_MASK | FIQ_MASK
    MSR     CPSR, R4                                ; go back into supervisor mode
    B       GoToStandby

GoToStandby
    MOV     R4,  #0
    MCR     p15, 0, R4, c7, c0, 4  ; wait for interrupt (WFI)
    B       _Restore

_Restore
    LDR     R3,   =ARM_CONTEXT_SAVE_ADDRESS   ; load SDRAM store location into R3
    LDMIA   R3!,  {R4-R8}                ; load data from SDRAM
    MOV     SP,   R6                     ; update the SP value
    MOV     LR,   R7                     ; update the LR value
    MSR     SPSR, R8                     ; update the SPSR value
    LDMIA   R3!,  {R4-R11}               ; load from SDRAM
    MCR     p15,  0, R4,  c1,  c0, 1  ; Auxiliary Control Register
    MCR     p15,  0, R5,  c1,  c0, 2  ; Coprcessor access Control Register
    MCR     p15,  0, R6,  c2,  c0, 0  ; TTBR0 
    MCR     p15,  0, R7,  c2,  c0, 1  ; TTBR1  
    MCR     p15,  0, R8,  c2,  c0, 2  ; TTBR Control
    MCR     p15,  0, R9,  c3,  c0, 0  ; Domain Access  
    MCR     p15,  0, R10, c5,  c0, 0  ; DFSR 
    MCR     p15,  0, R11, c5,  c0, 1  ; IFSR  
    LDMIA   R3!,  {R4-R11}               ; load from SDRAM
    MCR     p15,  0, R4,  c6,  c0, 0  ; DFAR 
    MCR     p15,  0, R5,  c6,  c0, 1  ; IFAR  
    MCR     p15,  0, R6,  c9,  c0, 0  ; Data Cache Lockdown 
    MCR     p15,  0, R7,  c9,  c0, 1  ; ICache Lockdown
    MCR     p15,  0, R8,  c10, c0, 0  ; TLB Lockdown
    MCR     p15,  0, R9,  c13, c0, 0  ; FCSE PID Register
    MCR     p15,  0, R10, c13, c0, 1  ; Context ID Register
    MCR     p15,  0, R11, c15, c2, 4  ; Peripheral Port remap register

    LDR     R4,   [R3], #4             ; load from SDRAM
    MCR     p15,  0, R4, c1, c0, 0     ; Co-processor control register
    
    ; enter into FIQ, IRQ, and ABORT mode and store the
    ; SP and LR values and then go back into supervisor mode
    
    MOV     R4,   #0xD1                            ;#FIQ_MODE | IRQ_MASK | FIQ_MASK
    MSR     CPSR, R4                               ; go into FIQ mode
    LDMIA   R3!,  {R4-R5}                          ; load the SP and LR from SDRAM
    MOV     SP,   R4                               ; update the SP
    MOV     LR,   R5                               ; update the LR
    MOV     R4,   #0xD2                            ;#IRQ_MODE | IRQ_MASK | FIQ_MASK
    MSR     CPSR, R4                               ; go into IRQ mode
    LDMIA   R3!,  {R4-R5}                          ; load the SP and LR from SDRAM
    MOV     SP,   R4                               ; update the SP
    MOV     LR,   R5                               ; update the LR
    MOV     R4,   #0xD7                            ;#ABORT_MODE | IRQ_MASK | FIQ_MASK
    MSR     CPSR, R4                               ; go into ABORT mode
    LDMIA   R3!,  {R4-R5}                          ; load the SP and LR from SDRAM
    MOV     SP,   R4                               ; update the SP
    MOV     LR,   R5                               ; update the LR
    MOV     R4,   #0x93                            ;#SUP_MODE | IRQ_MASK (NOTE: CE does not set FIQ_MASK on entry)
    MSR     CPSR, R4                               ; go back into supervisor mode       

    ; restore regs and return
    ldmfd   sp!, {r0 - r12, pc}

    ENTRY_END

    LEAF_ENTRY OALCPUIdle_sz
    ;Entry used to calculate the size of the function
    nop
    nop
    ENTRY_END


;------------------------------------------------------------------------------
        END
