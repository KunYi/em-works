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
; Module Name:
;
;    bvd1macros.inc
;
; Abstract:
;
;    Intel Bulverde helper macros.
;
; Environment:
;
; Revision History:
;
;--

    IF  !:DEF: _bvd1macros_inc_
_bvd1macros_inc_    EQU 1


;
; **** Macro DisableInts *********************************************
; * Params: $gp1 (scratch reg)
; *
; * Returns: nothing
; *
; *  This macro disables both IRQ and FIQ nondestructively
; ********************************************************************
;
    MACRO
    DisableInts $gp1
        MRS $gp1, cpsr                     ; Get value of CPSR
        ORR $gp1, $gp1, #NoIntsMask        ; Set IRQ and FIQ-disabling bits
        MSR cpsr_c, $gp1                   ; Disable the IRQ/FIQ
    MEND


;
; **** Macro EnableInts *********************************************
; * Params: $gp1 (scratch reg)
; *
; * Returns: nothing
; *
; *  This macro enables both IRQ and FIQ nondestructively
; ********************************************************************
;
    MACRO
    EnableInts $gp1
        MRS $gp1, cpsr                     ; Get value of CPSR
        AND $gp1, $gp1, #IrqFiqEnable      ; Set enable/disable bits
        MSR cpsr_c, $gp1                   ; Control the IRQ/FIQ
    MEND


;
; **** Macro Zero_SDRAM *********************************************
; * Params: $StartAdx: 32-bit aligned start address.
; *         $NumMB   : Number of MB to clear.
; *         $gp1,2   : general purpose scratch
; *
; * Returns: nothing
; *
; * Effects: corrupts $gp1,2
; *
; *  This macro simply zeros out RAM, ending at $StartAdx, and
; *  starting at ($StartAdx + ($NumMB*0x100000)), inclusive.  It is the resposibility of the
; *  caller to ensure that the addresses are valid!  Ensure $StartAdx
; *  is 32-bit aligned, or the math will be wrong.
; *
; Ex:
;    ldr r0, =0xA0000000     ; start Adx
;    mov r1, #64             ; #MB
;
;    Zero_SDRAM r0, r1, r2, r3
;
; ********************************************************************
;
    MACRO
    Zero_SDRAM $StartAdx, $NumMB, $gp1, $gp2
    ;
    ; Determine ending address: endAdx = (($NumMB * 0x0010 0000) + $StartAdx)
    ;
    mov     $gp1, #0x100000
    mla     $gp2, $NumMB, $gp1, $StartAdx

    ldr     $gp1, =0x00000000           ; source data
10
    sub     $gp2, $gp2, #4              ; word pre-decrement

    str     $gp1, [$gp2]                ; 32-bit zero fill
    cmp     $gp2, $StartAdx
    bne     %BT10

    MEND


;
; **** Macro Zero_SDRAM_Tuned_64MB ***********************************
; * Params: $StartAdx: 32-bit aligned start address.
; *         $gp1,2   : general purpose scratch
; *
; * Returns: nothing
; *
; * Effects: corrupts $gp1-6
; *
; *  This macro simply zeros out 64MB of RAM starting at $StartAdx.
; *  It is the resposibility of the caller to ensure that the addresses 
; *  are valid!  Ensure $StartAdx is 32-bit aligned, or the math will be
; *  wrong.
; *
; Ex:
;    ldr r0, =0xA0000000     ; start Adx
;
;    Zero_SDRAM_Tuned_64MB r0, r1, r2, r3, r4, r5, r6
;
; ********************************************************************
;
    MACRO
    SCRUB_SDRAM_TUNED_64 $StartAdx, $gp1, $gp2, $gp3, $gp4, $gp5, $gp6

    mov    $gp6, #0                ; data
    mov    $gp1, $StartAdx
    add    $gp2, $StartAdx, #4
    add    $gp3, $StartAdx, #8
    add    $gp4, $StartAdx, #C

    mov    $gp5, #0x04000000       ; loop counter = 64 MB

10
    str    $gp6, [$gp1], #+16
    str    $gp6, [$gp2], #+16
    str    $gp6, [$gp3], #+16
    str    $gp6, [$gp4], #+16

    subs   $gp5, $gp5, #16
    bne    %BT10

    str    $gp6, [$gp1]            ; get last word

    MEND


;
; **** CPWAIT ********************************************************
; * Params: $Rd      : temporary read register
; *
; * Returns: nothing
; *
; * Effects: corrupts $Rd
; *
; *  This macro is used to wait for coprocessor operations to complete.
; *
; Ex:
;
;    CPWAIT r0
;
; ********************************************************************
;
    MACRO
    CPWAIT  $Rd
    
    mrc     p15, 0, $Rd, c2, c0, 0       ; arbitrary read of CP15
    mov     $Rd, $Rd                     ; wait for it (foward dependency)
    sub     pc, pc, #4                   ; branch to next instruction
    
    MEND


;
; **** Macro InitFFUART *********************************************
; * Params: $FBA (FFUart Base Address )$gp2, $gp3 (scratch regs)
; *
; * Returns: nothing
; *
; * Registers:  PReserves $FBA, corrupts the rest
; *
; *  This macro inits the FFUART in non-polled, non-FIFO mode at 38400 baud.
; ********************************************************************
;
    MACRO
    InitFFUART $FBA, $gp2, $gp3

    ; Disable UART and disable interrupts
    ldr $gp2, =0x0
    str $gp2, [$FBA, #0x0c] ; (DLAB OFF)
    str $gp2, [$FBA, #0x04] ; IER_DLH = 0x0

    ; Set baud rate divisor (38400 baud)
    ldr $gp2, =0x80         
    str $gp2, [$FBA, #0x0c] ; (DLAB ON)
    ldr $gp2, =0x18         
    str $gp2, [$FBA]        ; THR_RBR_DLL = 0x18
    ldr $gp2, =0x0
    str $gp2, [$FBA, #0x04] ; IER_DLH = 0x0

    ; Set communications parameters to 8,N,1
    ldr $gp2, =0x0
    str $gp2, [$FBA, #0x0c] ; (DLAB OFF)
    ldr $gp2, =0x3
    str $gp2, [$FBA, #0x0c] ; LCR = 0x3

    ; Clear and enable fifos
    ldr $gp2, =0x7
    str $gp2, [$FBA, #0x08] ; IIR_FCR = 0x8

    ; Set polled mode
    ldr $gp2, =0x0
    str $gp2, [$FBA, #0x04] ; IER_DLH = 0x0

    ; Set normal UART mode
    ldr $gp2, =0x0
    str $gp2, [$FBA, #0x10] ; MCR = 0


    ; Enable UART
    ldr $gp2, [$FBA, #0x04] ; $gp2 = IER_DLH
    orr $gp2, $gp2, #0x40   ; Set the enable uart bit
    str $gp2, [$FBA, #0x04] ;

    MEND


;
; **** Macro PrintStr *********************************************
; * Params: $FBA (FFUART Base Address), $pStr (pointer to string), $gp1
; *
; * Returns: nothing
; *
; * Effects: Corrupts $pStr & $gp1, preserves $FBA
; *
; *  This macro writes the string pointed to by $pSTr until a '0' is reached.
; *
; ********************************************************************
;
    MACRO
    PrintStr $FBA, $pStr, $gp1

10
    ldrb    $gp1, [$pStr]        ; load the first byte
    cmp     $gp1, #0             ; is it NULL?
    beq     %FT20                ; if so, let's end now (search forward, this macro only)

    IsTBE   $FBA, $gp1           ; ensure TBE
    ldrb    $gp1, [$pStr]        ; load the first byte (agin, for now... really need another register)

    strb    $gp1, [$FBA]         ; transmit a byte
    add     $pStr, $pStr, #1     ;  and increment the byte pointer
    b       %BT10                ; otherwise, keep looping (search backwards, this macro only)
20

    MEND


;
; **** Macro PrintReg *********************************************
; * Params: $FBA (FFUART Base Address), $Reg (register to dump), rest gp
; *
; * Returns: nothing
; *
; * Prints a 32-bit register to HTERM via FFUART.  Does ASCII conversion.
; *
; ********************************************************************
;
    MACRO
    PrintReg $FBA, $Reg, $gp1, $gp2, $gp3

    ; First, must convert register to ASCII
    ;
    mov     $gp1, #28                       ; n = 28
99
    mov     $gp2, $Reg LSR $gp1             ; $gp2 = $Reg >> n
    and     $gp2, $gp2, #0xF                ; mask off irrelevant bits
    cmp     $gp2, #0x0000000A               ; if r1 < 0xA
    addlt   $gp3, $gp2, #0x30               ; $gp3 = ($gp2 + 0x30)  {0x0 -> 0x9}
    addge   $gp3, $gp2, #0x37               ; $gp3 = ($gp2 + 0x37)  {0xA -> 0xF)

    ; Now just dump the char i just converted
    ;
    IsTBE   $FBA, $gp2         ; ensure TBE
    strb    $gp3, [$FBA]       ; transmit

    subs    $gp1, $gp1, #4                  ; n=n-4
    bne     %BT99

    ; 0th Iteration
    ;
    mov     $gp2, $Reg                      ; $gp2 = $reg (no need to shift for the LSN)
    and         $gp2, $gp2, #0xF                ; mask off irrelevant bits
    cmp     $gp2, #0x0000000A               ; if r1 < 0xA
    addlt   $gp3, $gp2, #0x30               ; $gp3 = ($gp2 + 0x30)  {0x0 -> 0x9}
    addge   $gp3, $gp2, #0x37               ; $gp3 = ($gp2 + 0x37)  [0xA -> 0xF)

    ; dump the char i just converted
    ;
    IsTBE   $FBA, $gp2         ; ensure TBE
    strb    $gp3, [$FBA]       ; transmit

    ; Add cr/lf
    ;
    mov     $gp1, #0x0A
    IsTBE   $FBA, $gp2         ; ensure TBE
    strb    $gp1, [$FBA]       ; transmit  (LF)

    MEND


;
; **** Macro IsTBE ***************************************************
; * Params: $FBA (FFUART Base Address), $gp1 (scratch reg)
; *
; * Returns: nothing
; *
; * Effects: Corrupts $gp1, preserves $FBA
; *
; *  This macro spins until FFUART.LSR.TEMT gets set, indicating it
; *    is ready for data.
; ********************************************************************
;
    MACRO
    IsTBE $FBA, $gp1

10
    ldr  $gp1, [$FBA, #FF_LSR_OFFSET]
    ands $gp1, $gp1, #0x40    ; mask all but bit 6, and set Z if result=0 (i.e. if bit not set)
    beq  %BT10

    MEND


;
; **** Macro GET_CCSR_L **********************************************
; * Params: $retval (return value register), $base (base address), and
; *         $gp1 (scratch reg).
; *
; * Returns: L value in CCSR.
; *
; * Effects: Corrupts $gp1.
; *
; *  This macro will read the CCSR and return the L value in $retval
; *  $Base should contain the base address of the CCSR (virtual or phy)
; *
; ********************************************************************
;
    MACRO
    GET_CCSR_L $retval, $Base, $gp1

    ; read CCCR's value
    ldr     $retval, [$Base, #CCSR_OFFSET]

    ; mask out irrelevant bits
    ldr     $gp1, =0x1F
    and     $retval, $retval, $gp1

    MEND


;
; **** Macro GET_CCSR_2N *********************************************
; * Params: $retval (return value register), $base (base address), and
; *         $gp1 (scratch reg).
; *
; * Returns: 2N value in CCSR.
; *
; * Effects: Corrupts $gp1.
; *
; *  This macro will read the CCSR and return the 2N value in $retval
; *  $Base should contain the base address of the CCSR (virtual or phy)
; *
; ********************************************************************
;
    MACRO
    GET_CCSR_2N $retval, $Base, $gp1

    ; read CCCR's value
    ldr     $retval, [$Base, #CCSR_OFFSET]

    ; mask out irrelevant bits
    ldr     $gp1, =0x780
    and     $retval, $retval, $gp1
    mov     $retval, $retval LSR #7

    MEND


;
; **** Macro GET_CLKCFG *********************************************
; * Params: $retval (return value register)
; *
; * Returns: Reads the clock configuration register (cp16 reg 6).
; *
; * Effects: Corrupts $gp1.
; *
; *  This macro will read the CCSR and return the 2N value in $retval
; *  $Base should contain the base address of the CCSR (virtual or phy)
; *
; ********************************************************************
;
    MACRO
    GET_CLKCFG $retval

    ; read CLKCFG's value
    mrc p14, 0, $retval, c6, c0, 0

    ; mask out irrelevant bits
    and     $retval, $retval, #0xF

    MEND


    ENDIF ; !:DEF: _bvd1macros_inc_
    
    END 
