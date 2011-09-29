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
;------------------------------------------------------------------------------
;
;   Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
;   THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
;   AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
;
;------------------------------------------------------------------------------
;
;   File: nfc.s
;
;   This file implements low-level NAND Flash Controller access routines.
;
;------------------------------------------------------------------------------
    INCLUDE kxarm.h
        
    TEXTAREA

    IMPORT g_pNfcAxi
    
    MACRO 
    LDR4STR1 $src, $tmp1, $tmp2    
        ldrb    $tmp1, [$src], #+1
        ldrb    $tmp2, [$src], #+1
        orr     $tmp1, $tmp1, $tmp2, lsl #8
        ldrb    $tmp2, [$src], #+1
        orr     $tmp1, $tmp1, $tmp2, lsl #16
        ldrb    $tmp2, [$src], #+1
        orr     $tmp1, $tmp1, $tmp2, lsl #24
    MEND

    MACRO 
    STR4LDR1 $tgt, $src
        strb    $src, [$tgt], #+1
        mov     $src, $src, lsr #8
        strb    $src, [$tgt], #+1
        mov     $src, $src, lsr #8
        strb    $src, [$tgt], #+1
        mov     $src, $src, lsr #8
        strb    $src, [$tgt], #+1
    MEND

    TEXTAREA

;------------------------------------------------------------------------------
;
;   Function: RdPageAlign4
;
;   This function reads from the NFC main buffer and stores to the word-aligned
;   user buffer.
;
;   Parameters:
;       r0 - Sector buffer to be filled (must be 32-bit word aligned). 
;       r1 - # of bytes to be filled. 
;
;   Returns:
;------------------------------------------------------------------------------
    LEAF_ENTRY  RdPageAlign4
        stmfd   sp!, {r1 - r11}

        ldr     r2, =g_pNfcAxi
        ldr     r2, [r2]

rd_align4
        ldmia   r2!, {r4 - r11}
        stmia   r0!, {r4 - r11}
        subs    r1, r1, #32
        bne     rd_align4

        ldmfd   sp!, {r1 - r11}

        IF Interworking :LOR: Thumbing
          bx    lr
        ELSE
          mov   pc, lr
        ENDIF

;------------------------------------------------------------------------------
;
;   Function: RdPageAlign1
;
;   This function reads from the NFC main buffer and stores to the byte-aligned
;   user buffer (aligned on BYTE[1] of 32-bit word).
;
;   Parameters:
;       r0 - Sector buffer to be filled (aligned on BYTE[1] of 32-bit word). 
;       r1 - # of bytes to be filled. 
;
;   Returns:
;------------------------------------------------------------------------------
    LEAF_ENTRY  RdPageAlign1
        stmfd   sp!, {r1 - r11}

        ldr     r2, =g_pNfcAxi
        ldr     r2, [r2]

rd_align1
        ; Read BYTE[0]-BYTE[31]
        ldmia   r2!, {r4 - r11}         ; r4  = B03  B02  B01  B00
                                        ; r5  = B07  B06  B05  B04
                                        ; r6  = B11  B10  B09  B08
                                        ; r7  = B15  B14  B13  B12
                                        ; r8  = B19  B18  B17  B16
                                        ; r9  = B23  B22  B21  B20
                                        ; r10 = B27  B26  B25  B24
                                        ; r11 = B31  B30  B29  B28

        ; Write BYTE[0] (byte alignment)
        strb    r4, [r0], #+1     

        ; Write BYTE[1]-BYTE[2] (half-word alignment)
        mov     r4, r4, lsr #8          ; r4  =  00  B03  B02  B01
        strh    r4, [r0], #+2

        ; Shift data for multi-word write
        mov     r4, r4, lsr #16     
        orr     r4, r4, r5, lsl #8      ; r4  = B06  B05  B04  B03
        mov     r5, r5, lsr #24     
        orr     r5, r5, r6, lsl #8      ; r5  = B10  B09  B08  B07
        mov     r6, r6, lsr #24     
        orr     r6, r6, r7, lsl #8      ; r6  = B14  B13  B12  B11
        mov     r7, r7, lsr #24     
        orr     r7, r7, r8, lsl #8      ; r7  = B18  B17  B16  B15
        mov     r8, r8, lsr #24     
        orr     r8, r8, r9, lsl #8      ; r8  = B22  B21  B20  B19
        mov     r9, r9, lsr #24     
        orr     r9, r9, r10, lsl #8     ; r9  = B26  B25  B24  B23
        mov     r10, r10, lsr #24     
        orr     r10, r10, r11, lsl #8   ; r10 = B30  B29  B28  B27
        
        ; Write BYTE[3]-BYTE[30] (double word alignment)        
        stmia   r0!, {r4 - r10}

        ; Write BYTE[31] (byte alignment)
        mov     r11, r11, lsr #24       ; r11 =  00   00   00  B31
        strb    r11, [r0], #+1

        subs    r1, r1, #32
        bne     rd_align1

        ldmfd   sp!, {r1 - r11}

        IF Interworking :LOR: Thumbing
          bx    lr
        ELSE
          mov   pc, lr
        ENDIF


;------------------------------------------------------------------------------
;
;   Function: RdPageAlign2
;
;   This function reads from the NFC main buffer and stores to the 
;   half-word aligned user buffer (aligned on BYTE[2] of 32-bit word).
;
;   Parameters:
;       r0 - Sector buffer to be filled (aligned on BYTE[2] of 32-bit word). 
;       r1 - # of bytes to be filled. 
;
;   Returns:
;------------------------------------------------------------------------------
    LEAF_ENTRY  RdPageAlign2
        stmfd   sp!, {r1 - r11}

        ldr     r2, =g_pNfcAxi
        ldr     r2, [r2]

rd_align2
        ; Read BYTE[0]-BYTE[31]
        ldmia   r2!, {r4 - r11}         ; r4  = B03  B02  B01  B00
                                        ; r5  = B07  B06  B05  B04
                                        ; r6  = B11  B10  B09  B08
                                        ; r7  = B15  B14  B13  B12
                                        ; r8  = B19  B18  B17  B16
                                        ; r9  = B23  B22  B21  B20
                                        ; r10 = B27  B26  B25  B24
                                        ; r11 = B31  B30  B29  B28

        ; Write BYTE[0]-BYTE[1] (half-word alignment)
        strh    r4, [r0], #+2

        ; Shift data for multi-word write
        mov     r4, r4, lsr #16
        orr     r4, r4, r5, lsl #16     ; r4  = B05  B04  B03  B02
        mov     r5, r5, lsr #16     
        orr     r5, r5, r6, lsl #16     ; r5  = B09  B08  B07  B06
        mov     r6, r6, lsr #16     
        orr     r6, r6, r7, lsl #16     ; r6  = B13  B12  B11  B10
        mov     r7, r7, lsr #16     
        orr     r7, r7, r8, lsl #16     ; r7  = B17  B16  B15  B14
        mov     r8, r8, lsr #16     
        orr     r8, r8, r9, lsl #16     ; r8  = B21  B20  B19  B18
        mov     r9, r9, lsr #16    
        orr     r9, r9, r10, lsl #16    ; r9  = B25  B24  B23  B22
        mov     r10, r10, lsr #16    
        orr     r10, r10, r11, lsl #16  ; r10 = B29  B28  B27  B25
        
        ; Write BYTE[2]-BYTE[29] (double word alignment)        
        stmia   r0!, {r4 - r10}

        ; Write BYTE[30]-BYTE[31] (half-word alignment)
        mov     r11, r11, lsr #16       ; r11  =  00  00  B31  B30
        strh    r11, [r0], #+2

        subs    r1, r1, #32
        bne     rd_align2

        ldmfd   sp!, {r1 - r11}

        IF Interworking :LOR: Thumbing
          bx    lr
        ELSE
          mov   pc, lr
        ENDIF

        
;------------------------------------------------------------------------------
;
;   Function: RdPageAlign3
;
;   This function reads from the NFC main buffer and stores to the byte-aligned
;   user buffer (aligned on BYTE[3] of 32-bit word).
;
;   Parameters:
;       r0 - Sector buffer to be filled (aligned on BYTE[3] of 32-bit word). 
;       r1 - # of bytes to be filled. 
;
;   Returns:
;------------------------------------------------------------------------------
    LEAF_ENTRY  RdPageAlign3
        stmfd   sp!, {r1 - r11}

        ldr     r2, =g_pNfcAxi
        ldr     r2, [r2]

rd_align3
        ; Read BYTE[0]-BYTE[31]
        ldmia   r2!, {r4 - r11}         ; r4  = B03  B02  B01  B00
                                        ; r5  = B07  B06  B05  B04
                                        ; r6  = B11  B10  B09  B08
                                        ; r7  = B15  B14  B13  B12
                                        ; r8  = B19  B18  B17  B16
                                        ; r9  = B23  B22  B21  B20
                                        ; r10 = B27  B26  B25  B24
                                        ; r11 = B31  B30  B29  B28

        ; Write BYTE[0] (byte alignment)
        strb    r4, [r0], #+1     

        ; Shift data for multi-word write
        mov     r4, r4, lsr #8     
        orr     r4, r4, r5, lsl #24     ; r4  = B04  B03  B02  B01
        mov     r5, r5, lsr #8     
        orr     r5, r5, r6, lsl #24     ; r5  = B08  B07  B06  B05
        mov     r6, r6, lsr #8     
        orr     r6, r6, r7, lsl #24     ; r6  = B12  B11  B10  B09
        mov     r7, r7, lsr #8     
        orr     r7, r7, r8, lsl #24     ; r7  = B16  B15  B14  B13
        mov     r8, r8, lsr #8     
        orr     r8, r8, r9, lsl #24     ; r8  = B20  B19  B18  B17
        mov     r9, r9, lsr #8     
        orr     r9, r9, r10, lsl #24    ; r9  = B24  B23  B22  B21
        mov     r10, r10, lsr #8     
        orr     r10, r10, r11, lsl #24  ; r10 = B28  B27  B26  B25
        
        ; Write BYTE[1]-BYTE[28] (double word alignment)        
        stmia   r0!, {r4 - r10}

        ; Write BYTE[29]-BYTE[30] (half-word alignment)
        mov     r11, r11, lsr #8        ; r11  =  00  B31  B30  B29
        strh    r11, [r0], #+2

        ; Write BYTE[31] (byte alignment)
        mov     r11, r11, lsr #16       ; r11 =  00   00   00  B31
        strb    r11, [r0], #+1

        subs    r1, r1, #32
        bne     rd_align3

        ldmfd   sp!, {r1 - r11}

        IF Interworking :LOR: Thumbing
          bx    lr
        ELSE
          mov   pc, lr
        ENDIF


;------------------------------------------------------------------------------
;
;   Function: WrPageAlign4
;
;   This function reads from the word-aligned user buffer and stores to the
;   NFC main buffer.
;
;   Parameters:
;       r0 - Sector buffer to be written (must be 32-bit word aligned). 
;       r1 - # of bytes to be written. 
;
;   Returns:
;------------------------------------------------------------------------------
    LEAF_ENTRY  WrPageAlign4
        stmfd   sp!,{r1 - r11}

        ldr     r2, =g_pNfcAxi
        ldr     r2, [r2]

wr_align4
        ldmia   r0!, {r4 - r11}
        stmia   r2!, {r4 - r11}
        subs    r1, r1, #32
        bne     wr_align4

        ldmfd   sp!, {r1 - r11}

        IF Interworking :LOR: Thumbing
          bx    lr
        ELSE
          mov   pc, lr
        ENDIF


;------------------------------------------------------------------------------
;
;   Function: WrPageAlign1
;
;   This function reads from the byte-aligned user buffer (aligned on BYTE[1] 
;   of 32-bit word) and stores to the NFC main buffer.
;
;   Parameters:
;       r0 - Sector buffer to be written (aligned on BYTE[1] of 32-bit word). 
;       r1 - # of bytes to be written. 
;
;   Returns:
;------------------------------------------------------------------------------
    LEAF_ENTRY  WrPageAlign1
        stmfd   sp!, {r1 - r11}

        ldr     r2, =g_pNfcAxi
        ldr     r2, [r2]

wr_align1
        ; Read BYTE[0] (byte alignment)
        ldrb    r4, [r0], #+1           ; r4  =  00   00   00  B00

        ; Read BYTE[1]-BYTE[2] (half-word alignment)
        ldrh    r3, [r0], #+2           ; r3  =  00   00  B02  B01
        orr     r4, r4, r3, lsl #8      ; r4  =  00  B02  B01  B00

        ; Read BYTE[3]-BYTE[30]
        ldmia   r0!, {r5 - r11}         ; r5  = B06  B05  B04  B03
                                        ; r6  = B10  B09  B08  B07
                                        ; r7  = B14  B13  B12  B11
                                        ; r8  = B18  B17  B16  B15
                                        ; r9  = B22  B21  B20  B19
                                        ; r10 = B26  B25  B24  B23
                                        ; r11 = B30  B29  B28  B27

        ; Shift data for multi-word write
        orr     r4, r4, r5, lsl #24     ; r4  = B03  B02  B01  B00
        mov     r5, r5, lsr #8     
        orr     r5, r5, r6, lsl #24     ; r5  = B07  B06  B05  B04
        mov     r6, r6, lsr #8     
        orr     r6, r6, r7, lsl #24     ; r6  = B11  B10  B09  B08
        mov     r7, r7, lsr #8     
        orr     r7, r7, r8, lsl #24     ; r7  = B15  B14  B13  B12
        mov     r8, r8, lsr #8     
        orr     r8, r8, r9, lsl #24     ; r8  = B19  B18  B17  B16
        mov     r9, r9, lsr #8     
        orr     r9, r9, r10, lsl #24    ; r9  = B23  B22  B21  B20
        mov     r10, r10, lsr #8     
        orr     r10, r10, r11, lsl #24  ; r10 = B27  B26  B25  B24
        
        ; Read BYTE[31] (byte alignment)
        ldrb    r3, [r0], #+1           ; r3  =  00   00   00  B31
        mov     r11, r11, lsr #8     
        orr     r11, r11, r3, lsl #24   ; r11 = B31  B30  B29  B28

        ; Write BYTE[0]-BYTE[31]
        stmia   r2!, {r4 - r11}

        subs    r1, r1, #32
        bne     wr_align1

        ldmfd   sp!, {r1 - r11}

        IF Interworking :LOR: Thumbing
          bx    lr
        ELSE
          mov   pc, lr
        ENDIF


;------------------------------------------------------------------------------
;
;   Function: WrPageAlign2
;
;   This function reads from the half-word aligned user buffer (aligned on 
;   BYTE[2] of 16-bit word) and stores to the NFC main buffer.
;
;   Parameters:
;       r0 - Sector buffer to be written (aligned on BYTE[2] of 32-bit word). 
;       r1 - # of bytes to be written.  
;
;   Returns:
;------------------------------------------------------------------------------
    LEAF_ENTRY  WrPageAlign2
        stmfd   sp!, {r1 - r11}

        ldr     r2, =g_pNfcAxi
        ldr     r2, [r2]

wr_align2
        ; Read BYTE[0]-BYTE[1] (half-word alignment)
        ldrh    r4, [r0], #+2           ; r4  =  00   00  B01  B00

        ; Read BYTE[2]-BYTE[29]
        ldmia   r0!, {r5 - r11}         ; r5  = B05  B04  B03  B02
                                        ; r6  = B09  B08  B07  B06
                                        ; r7  = B13  B12  B11  B10
                                        ; r8  = B17  B16  B15  B14
                                        ; r9  = B21  B20  B19  B18
                                        ; r10 = B25  B24  B23  B22
                                        ; r11 = B29  B28  B27  B26

        ; Shift data for multi-word write
        orr     r4, r4, r5, lsl #16     ; r4  = B03  B02  B01  B00
        mov     r5, r5, lsr #16     
        orr     r5, r5, r6, lsl #16     ; r5  = B07  B06  B05  B04
        mov     r6, r6, lsr #16     
        orr     r6, r6, r7, lsl #16     ; r6  = B11  B10  B09  B08
        mov     r7, r7, lsr #16     
        orr     r7, r7, r8, lsl #16     ; r7  = B15  B14  B13  B12
        mov     r8, r8, lsr #16     
        orr     r8, r8, r9, lsl #16     ; r8  = B19  B18  B17  B16
        mov     r9, r9, lsr #16     
        orr     r9, r9, r10, lsl #16    ; r9  = B23  B22  B21  B20
        mov     r10, r10, lsr #16     
        orr     r10, r10, r11, lsl #16  ; r10 = B27  B26  B25  B24
        
        ; Read BYTE[30]-BYTE[31] (half-word alignment)
        ldrh    r3, [r0], #+2           ; r3  =  00   00  B31  B30
        mov     r11, r11, lsr #16        
        orr     r11, r11, r3, lsl #16   ; r11 = B31  B30  B29  B28

        ; Write BYTE[0]-BYTE[31]
        stmia   r2!, {r4 - r11}

        subs    r1, r1, #32
        bne     wr_align2

        ldmfd   sp!, {r1 - r11}

        IF Interworking :LOR: Thumbing
          bx    lr
        ELSE
          mov   pc, lr
        ENDIF


;------------------------------------------------------------------------------
;
;   Function: WrPageAlign3
;
;   This function reads from the byte-aligned user buffer (aligned on BYTE[3] 
;   of 32-bit word) and stores to the NFC main buffer.
;
;   Parameters:
;       r0 - Sector buffer to be written (aligned on BYTE[3] of 32-bit word). 
;       r1 - # of bytes to be written. 
;
;   Returns:
;------------------------------------------------------------------------------
    LEAF_ENTRY  WrPageAlign3
        stmfd   sp!, {r1 - r11}

        ldr     r2, =g_pNfcAxi
        ldr     r2, [r2]

wr_align3
        ; Read BYTE[0] (byte alignment)
        ldrb    r4, [r0], #+1           ; r4  =  00   00   00  B00

        ; Read BYTE[1]-BYTE[28]
        ldmia   r0!, {r5 - r11}         ; r5  = B04  B03  B02  B01
                                        ; r6  = B08  B07  B06  B05
                                        ; r7  = B12  B11  B10  B09
                                        ; r8  = B16  B15  B14  B13
                                        ; r9  = B20  B19  B18  B17
                                        ; r10 = B24  B23  B22  B21
                                        ; r11 = B28  B27  B26  B25

        ; Shift data for multi-word write
        orr     r4, r4, r5, lsl #8      ; r4  = B03  B02  B01  B00
        mov     r5, r5, lsr #24     
        orr     r5, r5, r6, lsl #8      ; r5  = B07  B06  B05  B04
        mov     r6, r6, lsr #24     
        orr     r6, r6, r7, lsl #8      ; r6  = B11  B10  B09  B08
        mov     r7, r7, lsr #24     
        orr     r7, r7, r8, lsl #8      ; r7  = B15  B14  B13  B12
        mov     r8, r8, lsr #24     
        orr     r8, r8, r9, lsl #8      ; r8  = B19  B18  B17  B16
        mov     r9, r9, lsr #24     
        orr     r9, r9, r10, lsl #8     ; r9  = B23  B22  B21  B20
        mov     r10, r10, lsr #24     
        orr     r10, r10, r11, lsl #8   ; r10 = B27  B26  B25  B24
        
        ; Read BYTE[29]-BYTE[30] (half-word alignment)
        ldrh    r3, [r0], #+2           ; r3  =  00   00  B30  B29
        mov     r11, r11, lsr #24        
        orr     r11, r11, r3, lsl #8    ; r11 =  00  B30  B29  B28

        ; Read BYTE[31] (byte alignment)
        ldrb    r3, [r0], #+1           ; r3  =  00   00   00  B31
        orr     r11, r11, r3, lsl #24   ; r11 = B31  B30  B29  B28

        ; Write BYTE[0]-BYTE[31]
        stmia   r2!, {r4 - r11}

        subs    r1, r1, #32
        bne     wr_align3

        ldmfd   sp!, {r1 - r11}

        IF Interworking :LOR: Thumbing
          bx    lr
        ELSE
          mov   pc, lr
        ENDIF
    END
