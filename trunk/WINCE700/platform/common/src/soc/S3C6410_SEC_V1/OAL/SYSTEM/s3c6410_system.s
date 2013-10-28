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
; THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
;
;
;------------------------------------------------------------------------------
;
;   File:  startup.s
;
;   Kernel startup routine for Samsung SMDK6410 board. Hardware is
;   initialized in boot loader - so there isn't much code at all.
;
;------------------------------------------------------------------------------

        INCLUDE kxarm.h
        INCLUDE armmacros.s
        INCLUDE s3c6410.inc

        TEXTAREA

        EXPORT    System_EnableVIC
        EXPORT    System_DisableVIC
        EXPORT    System_EnableIRQ
        EXPORT    System_DisableIRQ
        EXPORT    System_EnableFIQ
        EXPORT    System_DisableFIQ
        EXPORT    System_EnableBP
        EXPORT    System_DisableBP
        EXPORT    System_EnableICache
        EXPORT    System_CheckSyncMode
        EXPORT    System_SetSyncMode
        EXPORT    System_SetAsyncMode
        EXPORT    System_WaitForInterrupt
        EXPORT    ChangeAsyncToSync
        EXPORT    ChangeSyncToAsync
        EXPORT    ChangeDivider
        EXPORT    ChangeToL1
        EXPORT    ChangeToL0


    ;--------------------
    ;    Enable VIC
    ;--------------------
    LEAF_ENTRY System_EnableVIC
        mrc        p15,0,r0,c1,c0,0
        orr        r0,r0,#R1_VE
        mcr        p15,0,r0,c1,c0,0
        mov        pc, lr
        ENTRY_END


    ;--------------------
    ;    Disable VIC
    ;--------------------
    LEAF_ENTRY System_DisableVIC
        mrc        p15,0,r0,c1,c0,0
        bic        r0,r0,#R1_VE
        mcr        p15,0,r0,c1,c0,0
        mov        pc, lr
        ENTRY_END


    ;--------------------
    ;    Enable IRQ
    ;--------------------
    LEAF_ENTRY System_EnableIRQ
        mrs        r0,cpsr
        bic        r0,r0,#I_Bit
        msr        cpsr_cxsf,r0
        mov        pc, lr
        ENTRY_END


    ;--------------------
    ;    Disable IRQ
    ;--------------------
    LEAF_ENTRY System_DisableIRQ
        mrs        r0,cpsr
        orr        r0,r0,#I_Bit
        msr        cpsr_cxsf,r0
        mov        pc, lr
        ENTRY_END


    ;--------------------
    ;    Enable FIQ
    ;--------------------
    LEAF_ENTRY System_EnableFIQ
        mrs        r0,cpsr
        bic        r0,r0,#F_Bit
        msr        cpsr_cxsf,r0
        mov        pc, lr
        ENTRY_END


    ;--------------------
    ;    Disable FIQ
    ;--------------------
    LEAF_ENTRY System_DisableFIQ
        mrs        r0,cpsr
        orr        r0,r0,#F_Bit
        msr        cpsr_cxsf,r0
        mov        pc, lr
        ENTRY_END


    ;-------------------------
    ;    Enable Branch Prediction
    ;-------------------------
    LEAF_ENTRY System_EnableBP
        mrc        p15,0,r0,c1,c0,0
        orr        r0,r0,#R1_BP
        mcr        p15,0,r0,c1,c0,0
        mov        pc, lr
        ENTRY_END


    ;-------------------------
    ;    Disable Branch Prediction
    ;-------------------------
    LEAF_ENTRY System_DisableBP
        mrc        p15,0,r0,c1,c0,0
        bic        r0,r0,#R1_BP
        mcr        p15,0,r0,c1,c0,0
        mov        pc, lr
        ENTRY_END


    ;-------------------------
    ;    Enable ICache
    ;-------------------------
    LEAF_ENTRY System_EnableICache
        mrc        p15, 0, r0, c1, c0, 0
        orr        r0, r0, #R1_I
        mcr        p15, 0, r0, c1, c0, 0
        mov        pc, lr
        ENTRY_END

    ;---------------------------
    ;    Set to Synchronous Mode
    ;---------------------------
    LEAF_ENTRY System_CheckSyncMode

        ldr        r0, =OTHERS
        ldr        r1, [r0]                 ; Read OTHERS
        and        r1, r1, #0xC0            ; SyncMUXSel OTHERS[7] = Sync, SyncReq OTHERS[6] = Sync
        mov         r0, #0x01
        cmp        r1, #0xC0
        movne       r0, #0x00

        mov         pc, lr

        ENTRY_END

    ;---------------------------
    ;    Set to Synchronous Mode
    ;---------------------------
    LEAF_ENTRY System_SetSyncMode

        ldr        r0, =OTHERS
        ldr        r1, [r0]
        orr        r1, r1, #0x40            ; SyncMUX = Sync
        str        r1, [r0]

        nop
        nop
        nop
        nop
        nop

        ldr        r1, [r0]
        orr        r1, r1, #0x80            ; SyncReq = Sync
        str        r1, [r0]

WaitForSync
        ldr        r1, [r0]                ; Read OTHERS
        and        r1, r1, #0xF00            ; Wait SYNCMODEACK = 0xF
        cmp        r1, #0xF00
        bne        WaitForSync

        mov        pc, lr

        ENTRY_END


    ;---------------------------
    ;    Set to Asynchronous Mode
    ;---------------------------
    LEAF_ENTRY System_SetAsyncMode

        ldr        r0, =OTHERS
        ldr        r1, [r0]
        bic        r1, r1, #0xC0
        orr        r1, r1, #0x40            ; SyncReq = Async, SyncMUX = Sync
        str        r1, [r0]

WaitForAsync
        ldr        r1, [r0]                ; Read OTHERS
        and        r1, r1, #0xF00            ; Wait SYNCMODEACK = 0x0
        cmp        r1, #0x0
        bne        WaitForAsync

        ldr        r0, =OTHERS
        ldr        r1, [r0]
        bic        r1, r1, #0x40            ; SyncMUX = Async
        str        r1, [r0]

        nop
        nop
        nop
        nop
        nop

        mov        pc, lr

        ENTRY_END


    ;---------------------------
    ;    Set WaitForInterrupt
    ;---------------------------
    LEAF_ENTRY System_WaitForInterrupt

        mov        r0, #0

        mcr        p15,0,r0,c7,c10,4        ; Data Synchronization Barrier (TRM said...)
        mcr        p15,0,r0,c7,c0,4        ; Wait For Interrupt

        mov        pc, lr

        ENTRY_END


    ;---------------------------
    ;   Set ChangeAsyncToSync
    ;---------------------------

    LEAF_ENTRY ChangeAsyncToSync
        stmfd   sp!, {r0-r5}
        ldr r1,=0xB2A0F900
        ldr r0, [r1]
        mov r3, #0
loop1
        add r3, r3, #1
        mov r4, #0
        mcr p15, 0, r2, c7, c10, 4 ; data synchronization barrier instruction
        mcr p15, 0, r2, c7, c10, 5 ; data memory barrier operation                      
        cmp r3, #2
        orreq r0, r0, #0x1<<6      ; Change SYNCMUX
        cmp r3, #2
        streq r0, [r1]

        nop
        nop
        nop
        nop             

        cmp r3, #2
        orreq r0, r0, #0x1<<7      ; Change SYNCMODE
        cmp r3, #2
        streq r0, [r1]
loop2
        add r4, r4, #1
        cmp r4, #0x64
        bne loop2
        cmp r3, #2
        bne loop1   

        ldmfd   sp!, {r0-r5}
        mov     pc,lr

        ENTRY_END

    ;---------------------------
    ;   Set ChangeSyncToAsync
    ;---------------------------

    LEAF_ENTRY ChangeSyncToAsync

        stmfd   sp!, {r0-r5}
        ldr r1,=0xB2A0F900
        ldr r0, [r1]
        mov r3, #0
loop3   add r3, r3, #1
        mov r4, #0
        mcr p15, 0, r2, c7, c10, 4 ; data synchronization barrier instruction
        mcr p15, 0, r2, c7, c10, 5 ; data memory barrier operation                      
        cmp r3, #2
        biceq r0, r0, #0x1<<7       ; Change ASYNCMODE
        cmp r3, #2
        streq r0, [r1]
        mov r5, #0
loopasync
        add r5, r5, #1                      
        cmp r5, #0x100
        bne loopasync
        cmp r3, #2
        biceq r0, r0, #0x1<<6           ; Change SYNCMUX
        cmp r3, #2
        streq r0, [r1]
loop4
        add r4, r4, #1
        cmp r4, #0x64
        bne loop4
        cmp r3, #2
        bne loop3   
        ldmfd   sp!, {r0-r5}

        mov     pc,lr

    ENTRY_END

    ;---------------------------
    ;   Set ChangeSyncToAsync
    ;---------------------------

    LEAF_ENTRY ChangeDivider

        stmfd sp!, {r0-r5}
        ldr r1,=0xB2A0F020
        ldr r2, [r1]
        mov r3, #0
loopcd
        add r3, r3, #1
        mov r4, #0
        mcr p15, 0, r2, c7, c10, 4 ; data synchronization barrier instruction
        mcr p15, 0, r2, c7, c10, 5 ; data memory barrier operation
        cmp r3, #2
        streq r0, [r1]
loop1000
        add r4, r4, #1
        cmp r4, #0x100
        bne loop1000
        cmp r3, #2
        bne loopcd                      

        ldmfd sp!, {r0-r5}
        mov pc,lr

    ENTRY_END


    LEAF_ENTRY ChangeToL1
    
        stmfd sp!, {r0-r7}                                                   

        ldr r2,=0xb2a0f900          ; OTHERS
        ldr r3,=0xb2a0f00c          ; APLLCON
        ldr r4,=0xb2a0f020          ; CLKDIV0
        ldr r7,=0x814d0301          ; 667 MPS Value

        ldr r1, [r3]                ; Caching for D-Cache
        ldr r1, [r4]                ; Caching for D-Cache
        mov r1, r7                  ; Caching for D-Cache
        ldr r1, [r2]                ; r0=OTHERS
                        
        mov r5, #0
loopcache0
        add r5, r5, #1

        mcr p15, 0, r2, c7, c10, 4  ; data synchronization barrier instruction
        mcr p15, 0, r2, c7, c10, 5  ; data memory barrier operation                     

        cmp r5, #2
        biceq r1, r1, #0x1<<7       ; Change ASYNCMODE
        cmp r5, #2
        streq r1, [r2]

        mov r6, #0
loopwait0
        add r6, r6, #1                      
        cmp r6, #0x100
        bne loopwait0

        cmp r5, #2
        biceq r1, r1, #0x1<<6       ; Change SYNCMUX
        cmp r5, #2
        streq r1, [r2]

        mov r6, #0
loopwait1
        add r6, r6, #1                      
        cmp r6, #0x10
        bne loopwait1

        cmp r5, #2
        streq r7, [r3]              ; Set MPS to APLLCON

        mov r6, #0
loopwait2
        add r6, r6, #1                      
        cmp r6, #0x10
        bne loopwait2

        cmp r5, #2
        streq r0, [r4]              ; Set Divider to CLKDIV0

        mov r6, #0
loopwait3
        add r6, r6, #1                      
        cmp r6, #0x100
        bne loopwait3                           

        cmp r5, #2
        bne loopcache0  

        ldmfd sp!, {r0-r7}
        mov pc,lr

    ENTRY_END                                               


    LEAF_ENTRY ChangeToL0

        stmfd   sp!, {r0-r7}                                                     

        ldr r2,=0xb2a0f900          ; OTHERS
        ldr r3,=0xb2a0f00c          ; APLLCON
        ldr r4,=0xb2a0f020          ; CLKDIV0
        ldr r7,=0x814d0300          ; 667 MPS Value

        ldr r1, [r3]                ; Caching for D-Cache
        ldr r1, [r4]                ; Caching for D-Cache
        mov r1, r7                  ; Caching for D-Cache
        ldr r1, [r2]                ; r0=OTHERS 

        mov r5, #0
loopcache1
        add r5, r5, #1

        mcr p15, 0, r2, c7, c10, 4  ; data synchronization barrier instruction
        mcr p15, 0, r2, c7, c10, 5  ; data memory barrier operation                     

        cmp r5, #2
        streq r0, [r4]              ; Set Divider to CLKDIV0

        mov r6, #0
loopwait4
        add r6, r6, #1                      
        cmp r6, #0x10
        bne loopwait4

        cmp r5, #2
        streq r7, [r3]              ; Set MPS to APLLCON

        mov r6, #0
loopwait5
        add r6, r6, #1                      
        cmp r6, #0x10
        bne loopwait5                       

        cmp r5, #2
        orreq r1, r1, #0x1<<6       ; Change SYNCMUX
        cmp r5, #2
        streq r1, [r2]

        mov r6, #0
loopwait6
        add r6, r6, #1
        cmp r6, #0x10
        bne loopwait6

        cmp r5, #2              
        orreq r1, r1, #0x1<<7       ; Change SYNCMODE
        cmp r5, #2
        streq r1, [r2]

        mov r6, #0
loopwait7
        add r6, r6, #1                      
        cmp r6, #0x100
        bne loopwait7               

        cmp r5, #2
        bne loopcache1  

        ldmfd sp!, {r0-r7}
        mov pc,lr

    ENTRY_END


        END

;------------------------------------------------------------------------------

