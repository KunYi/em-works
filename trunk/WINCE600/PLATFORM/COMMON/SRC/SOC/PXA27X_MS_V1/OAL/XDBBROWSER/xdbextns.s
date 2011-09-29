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
;++
;
; Copyright 2002-2003 Intel Corp. All Rights Reserved.
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
;++
;
;      TITLE("XSC Debug Browser Extenstions")
;
;--
;
; Module Name:
;
;    xdbextns.s
;
; Abstract:
;
;
;
;--
    OPT    2   ; disable listing
    INCLUDE kxarm.h
    OPT     1  ; reenable listing

 

   IMPORT       XSCBwrHandleTraceBufferException
   IMPORT       DataAbortHandler
   IMPORT       NKSetDataAbortHandler

    ;; code piece in data segment. The first dword is patched
    ;; by the bsp to a CP read/write instruction.
    AREA |.data|, DATA
asm_patch_value
    DCD  0x00000000         ;  patch field
    DCD  0xE12FFF1E         ;  BX LR

    TEXTAREA

;------------------------------------------------------------------------
; This macro issues a CPWAIT by using a specific register
;
    MACRO 
    CPWAIT_RX $tag

    mrc p15, 0, $tag, c2, c0, 0
    mov $tag, $tag
    sub pc, pc, #4

    MEND
;------------------------------------------------------------------------
; This macro will Clean a given Data Cache line dictated by bits [31:5] of $tag
;        
    MACRO
    CLEAN_DCACHE_LINE $tag
        
    ; Issue clean command
        ;
        mcr p15, 0, $tag, c7, c10, 1

    MEND

;------------------------------------------------------------------------
; This macro will Drain the Wite & Fill Buffers dictated by bits [31:5] of $tag
;        
    MACRO
    DRAIN_BUFFER $tag
        
    ; Issue buffer drain command
        ;
        mcr p15, 0, $tag, c7, c10, 4

    MEND

;------------------------------------------------------------------------
; This macro will Invalidate a given Data Cache line dictated by bits [31:5] of $tag
;        
    MACRO
    INVALIDATE_D_CACHE_LINE $tag
        
    ; Issue invalidate command
        ;
        mcr p15, 0, $tag, c7, c6, 1

    MEND

;------------------------------------------------------------------------
; This macro will Invalidate a given I Cache line dictated by bits [31:5] of $tag
;        
    MACRO
    INVALIDATE_I_CACHE_LINE $tag
        
    ; Issue invalidate command
        ;
        mcr p15, 0, $tag, c7, c5, 1

    MEND


    LEAF_ENTRY INVALIDATE_DRAIN_PIPELINE	
    
     ;Stack all registers
    stmdb r13!,{r0-r12,r14}
    
    ;  unlock data cache - DON'T need to do this right now - have to research
    ; into our future locking plans, we may need to return error here 
    ; in the future 
    ;mcr p15,0,r0,c9,c2,1 
    
    ;  Clean the data cache line
    CLEAN_DCACHE_LINE r0  ;  R0: address of the memory region
    CPWAIT_RX r2
    ;   drain: clean the write memory fifo
    DRAIN_BUFFER r0
    CPWAIT_RX r2
    
    ;  unlock the instruction cache:only unlocked lines can be invalidated - 
    ; AGAIN, DON't DO this now, need to research into how we plan on locking
    ; things into the cache which could break this
    ;mcr p15,0,r0,c9,c1,1
    
    ;  invalid the instruction cache
    INVALIDATE_I_CACHE_LINE r0
    CPWAIT_RX r2

    ; Invalidate the BTB - Use r0, as you've just invalidated that instruction
    mcr p15,0, r0, c7,c5,6

    ; At this point the piplene is empty
    ;

    ; Invalide DCACHE line
    ;
    mcr p15,0,R0,c7,c6,1
    CPWAIT_RX r2

    ldmia r13!,{r0-r12,r14}
     
    IF Interworking :LOR: Thumbing
       bx  lr
    ELSE
       mov  pc, lr          ; return
    ENDIF
    
    
    
;
;   XSCBwrExecuteCoProcCode- Execute a given "Write 64-bit register" 
;    OpCode, passed in by R0 for the XDB Browser support
;
;  Assumptions:  R0 contains the Instruction/Opcode
;        R1 contains the 1st arg lower 32-bits
;        R2 contains the 2nd arg upper 32-bits 
;  Uses
;
    LEAF_ENTRY  XSCBwrExecuteCoProcCode

    STMDB sp!,{r0-r12,r14}

    ;Move the return value pointers up out of the way
    mov r3,r2 ;Should contain the upper 32-bit address
    mov r2,r1 ;Should contain the lowest 32-bit address
    
    ; read_dsp_asm_patch must contain the opcode
    ; so perform the patch and sychnonize the cache
    ldr r4, =asm_patch_value
    str r0,[r4]
    mov r0,r4
    bl  INVALIDATE_DRAIN_PIPELINE

    ; load the values from the addresses
    ; If this is a 'read', these won't matter, in addition
    ; if it's a 32-bit write, the upper address (R1) is set to a 
    ; dummy variable, so it won't harm the system
    ldr r0,[r2]  
    ldr r1,[r3]

    ; jump into opcode patch field
     mov lr, pc
     mov pc, r4

    ; Need to throw a CPWAIT in here as data on read won't be valid
    ; yet - takes 3 to 4 cycles for data to be 'read' into dest. regs.
    ; Make sure not to use r0 - r2 (already in use), so use r4
    CPWAIT_RX  r4
    ; store the result of the operation
    ; Again, if it was a load, then these are essentially No Ops
    ; If it's a 32-bit, then R1 is pointing to a dummy variable passed
    ; in, so it's a valid address that contains 0
    str r0,[r2]
    str r1,[r3]
    
    ldmia     sp!, {r0-r12,r14}
    
    IF Interworking :LOR: Thumbing
       bx  lr
    ELSE
       mov  pc, lr          ; return
    ENDIF

; 
;  XSCBwrEnableTrace
;  activate the trace buffer
;
;  Assumptions:  none 
; 
    LEAF_ENTRY XSCBwrEnableTrace
    mov  r1, #0x80000001
    mrc  p14, 0, R0, c10, c0, 0		; read DCSR
    orr  r1, r0, r1			; set bit 0 (== trace on)  
    mcr p14, 0, R1, c10, c0, 0		; write DCSR

    IF Interworking :LOR: Thumbing
       bx  lr
    ELSE
       mov  pc, lr          ; return
    ENDIF

; 
;  XSCBwrDisableTrace
;  Disable the trace buffer
;
;  Assumptions:  none 
; 
    LEAF_ENTRY XSCBwrDisableTrace
    mvn  r1, #1
    mrc  p14, 0, R0, c10, c0, 0         ; read DCSR
    and  r1, r0, r1                     ; clear bit 0 (== trace off)  
    mcr p14, 0, R1, c10, c0, 0          ; write DCSR

    IF Interworking :LOR: Thumbing
       bx  lr
    ELSE
       mov  pc, lr          ; return
    ENDIF

    ; 
;  XSCBwrTraceSetFillOnce
;  Enable the trace buffer full exception (fill once mode)
;
;  Assumptions:  none 
; 
    LEAF_ENTRY XSCBwrTraceSetFillOnce
    mov  r1, #02
    mrc  p14, 0, R0, c10, c0, 0     ; read DCSR
    orr  r1, r0, r1                 ; set bit 1 (== trace fill-once mode)  
    mcr p14, 0, R1, c10, c0, 0      ; write DCSR

    IF Interworking :LOR: Thumbing
       bx  lr
    ELSE
       mov  pc, lr          ; return
    ENDIF
    
; 
;  XSCBwrReadTraceByte
;  Read a single trace buffer byte int r0
;
;  Assumptions:  none 
; 
    LEAF_ENTRY XSCBwrReadTraceByte
    mrc p14, 0, r0, c11, c0,  0     ; read TBREG

    IF Interworking :LOR: Thumbing
       bx  lr
    ELSE
       mov  pc, lr          ; return
    ENDIF
        
;
;  XSCBwrSaveTrace
;  Store the trace buffer and the checkpoint registers into a user 
; supplied buffer
;
;  Assumptions:  R0 contains the address of the user buffer that must 
;                be 264 bytes large
; 
    LEAF_ENTRY  XSCBwrSaveTrace

    stmdb sp!,{r0-r4,r14}

    mrc p14, 0, R1, c10, c0, 0          ;  read DCSR into R1
    mvn r2, #1                          ;  disable trace by clearing bit 0
    and r2, r1, r2
    mcr p14, 0, R2, c10, c0, 0          ;  write DCSR back, old value is in R1 
    CPWAIT_RX  r4
        
    mrc p14, 0, R2, c12, c0, 0          ;  save CHKPT0 into buffer
    str r2, [r0],#4

    mrc p14, 0, R2, c13, c0, 0          ;  save CHKPT1 into buffer
    str r2, [r0],#4

    ldr r3, =256                        ; load the 256 byte trace buffer
trace_loop
    mrc p14, 0, r2, c11, c0,  0         ; read TBREG
    strb r2, [r0], #1
    adds r3, r3, #-1
    bne trace_loop

    mcr p14, 0, r1, c10, c0, 0          ; restore DCSR
    CPWAIT_RX r4
 
    ldmia     sp!, {r0-r4,r14}

    IF Interworking :LOR: Thumbing
       bx  lr
    ELSE
       mov  pc, lr          ; return
    ENDIF

;
;  XSCBwrDebugAbortHandler
;  Process exceptions from the debug unit that are signaled via data abort
;
;  Assumptions:  CPU is in abort mode. Label is called directly from
;                exception entry
;

   LEAF_ENTRY  XSCBwrDebugAbortHandler
    stmdb sp!,{r0-r12, r14}             ; need to retain all registers

    mrc   p14, 0, r0, c10, c0, 0        ; test MOE bit in DCSR 
    and   r0, r0, #0x1c
    cmp   r0, #0x18                     ; trace buffer full ?
    bne   unknown_debug_exception       ; unkown exception -> OS 
  
    bl    XSCBwrHandleTraceBufferException
    bl    XSCBwrEnableTrace

    ldmia sp!,{r0-r12, r14}             ; restore all registers
    subs  pc, lr, #4                    ; return from abort mode

unknown_debug_exception
    ldmia sp!,{r0-r12, r14}             ; restore all registers
    b     DataAbortHandler              ; let OS handle the exception


;
;  XSCBwrTraceDataAbortHandler
;  Trace aware data abort handler that vectors to the os data abort
;  handler if the DataAbort was not cause by the debug unit.
;  If the debug unit caused the excecption, capture the trace and
;  continue as if no Abort happend.
; 
;  Assumptions:  CPU is in abort mode. Label is called directly from
;                exception entry
;
  LEAF_ENTRY XSCBwrTraceDataAbortHandler
    stmdb sp!,{r0} ; temp. save of R0
    mrc p15, 0, r0, c5, c0, 0 ; read FAULT_STATUS into R0
    ands r0, r0, #0x200 ; test debug exception bit
    ldmia sp!, {r0} ; restore r0
    beq DataAbortHandler ; Jump to the main data abort handler
    b XSCBwrDebugAbortHandler ; call debug handler on debug abo
 
    END
