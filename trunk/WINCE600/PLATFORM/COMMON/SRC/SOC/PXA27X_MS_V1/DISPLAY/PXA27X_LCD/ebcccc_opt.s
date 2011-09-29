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
;/* 
;** Copyright 2000-2003 Intel Corporation All Rights Reserved.
;**
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
;*/
	INCLUDE memcopy_macro.inc
	EXPORT	Memmove1616_ASM_LE16
	EXPORT	Memmove1616_ASM_GT16
	EXPORT	EmulatedBltSrcCopy1616ASM
	
	AREA	EBCOPY16OPT,	CODE,	READONLY
	
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;	
	; void Memmove1616_ASM_LE16(WORD* pSrc, WORD *pDst, int len) 	;
	; 																;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;	
Memmove1616_ASM_LE16

	; r0->pSrc, r1->pDst, r2->len
	; if ( pSrc > pDst ), use increase mode
	cmp		r2,		#0

    IF Thumbing :LOR: Interworking
        bxle  lr
    ELSE
        movle  pc, lr          ; return
    ENDIF
	
	cmp 	r0, 	r1
	ble		Memmove1616_ASM_LE16_DEC
Memmove1616_ASM_LE16_INC

Memmove1616_ASM_LE16_INC_LABEL MEMCOPYMBTS_LT16 r0, r1, r2, #2, #2, r3, r12, ldrh, strh, ldrneh, strneh
    IF Thumbing :LOR: Interworking
        bx  lr
    ELSE
        mov  pc, lr          ; return
    ENDIF

; (pSrc <= pDst ) use decrease mode
Memmove1616_ASM_LE16_DEC
	add		r0,		r0,		r2, lsl #1
	add		r1,		r1,		r2, lsl #1
	sub		r0,		r0,		#2
	sub		r1,		r1,		#2
Memmove1616_ASM_LE16_DEC_LABEL MEMCOPYMBTS_LT16 r0, r1, r2, #-2, #-2, r3, r12, ldrh, strh, ldrneh, strneh
    IF Thumbing :LOR: Interworking
        bx  lr
    ELSE
        mov  pc, lr          ; return
    ENDIF

	
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;	
	; void Memmove1616_ASM_GT16(WORD* pSrc, WORD *pDst, int len) 	;
	; 																;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;	
Memmove1616_ASM_GT16

	cmp		r2,		#0			; len >= 0?
    IF Thumbing :LOR: Interworking
        bxle  lr
    ELSE
        movle  pc, lr          ; return
    ENDIF

	stmdb	sp!,	{r4-r6,r14}


	subs	r3,		r0,		r1
	rsblt	r3,		r3,		#0
	
	cmp		r3,		r2,		lsl #1
	cmple	r0,		r1
	ble		Memmove1616_ASM_GT16_DEC
Memmove1616_ASM_GT16_INC
	eor		r3,		r0,		r1
	tst		r3,		#2
	bne		Memmove1616_ASM_GT16_INC_2B
Memmove1616_ASM_GT16_INC_4B
	tst		r0,		#0x2
	ldrneh	r12,	[r0], #2
	subne	r2,		r2,	  #1
	mov		r14,	r2, 	lsr	#1 ; r14->number in 4-byte
	strneh	r12,	[r1], #2
	
	
Memmove1616_ASM_GT16_INC_4B_LABLE   MEMCOPYMBTS_LG16 r0, r1, r14, #4, #4, r3, r12, r4, r5, r6, ldr, str
	 
	tst		r2,		#0x1
	ldrneh	r12,	[r0], #2
	strneh	r12,	[r1], #2
 	b		Memmove1616_ASM_GT16_EXIT
 	
			
Memmove1616_ASM_GT16_INC_2B	
Memmove1616_ASM_GT16_INC_2B_LABLE	MEMCOPYMBTS_LG16 r0, r1, r2, #2, #2, r3, r12, r4, r5, r6, ldrh, strh
 	b		Memmove1616_ASM_GT16_EXIT

	
Memmove1616_ASM_GT16_DEC
	add		r0,		r0,		r2, lsl #1
	add		r1,		r1,		r2, lsl #1
	sub		r0,		r0,		#2
	sub		r1,		r1,		#2
	eor		r3,		r0,		r1
	tst		r3,		#2
	bne		Memmove1616_ASM_GT16_DEC_2B
	
Memmove1616_ASM_GT16_DEC_4B
	tst		r0,		#0x2
	ldreqh	r12,	[r0], #-2
	subeq	r2,		r2,	  #1
	mov		r14,	r2,   lsr	#1
	streqh	r12,	[r1], #-2
	
	sub		r0,		r0,		#2
	sub		r1,		r1,		#2
	
Memmove1616_ASM_GT16_DEC_4B_LABLE 	MEMCOPYMBTS_LG16 r0, r1, r14, #-4, #-4, r3, r12, r4, r5, r6, ldr, str

	add		r0,		r0,		#2
	add		r1,		r1,		#2
	 
	tst		r2,		#0x1
	ldrneh	r12,	[r0], #-2
	strneh	r12,	[r1], #-2
 	b		Memmove1616_ASM_GT16_EXIT
 	
			
Memmove1616_ASM_GT16_DEC_2B	
Memmove1616_ASM_GT16_DEC_2B_LABLE	MEMCOPYMBTS_LG16 r0, r1, r2, #-2, #-2, r3, r12, r4, r5, r6, ldrh, strh

	
Memmove1616_ASM_GT16_EXIT	
	ldmia	sp!,	{r4-r6,pc} 
	
	
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;	
	; void EmulatedBltSrcCopy1616ASM(WORD* pSrc, WORD *pDst, int len, int cRows,			;
	;								UINT32 iScanStrideSrc,									; 
	;								UINT32 iScanStrideDst)									;
	; 																						;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;	

STACKDEPTH					EQU		(4*4)
	
ISCANSTRIDESRC				EQU		(STACKDEPTH+0)
ISCANSTRIDEDST				EQU		(STACKDEPTH+4)
EmulatedBltSrcCopy1616ASM	
	cmp		r2,		#0
	cmpgt	r3,		#0

    IF Thumbing :LOR: Interworking
        bxle  lr
    ELSE
        movle  pc, lr          ; return
    ENDIF

	
	str		r4,		[sp,	#-4]!
	str		r5,		[sp,	#-4]!
	str		r6,		[sp,	#-4]!
	str		r7,		[sp,	#-4]!
	cmp		r2,		#16
	
	ldr		r4,		[sp,	#ISCANSTRIDESRC]
	ldr		r5,		[sp,	#ISCANSTRIDEDST]	
	mov		r7,		r2			; keep len in one line
	
	bgt		SrcCopy1616_ASM_GT16
	
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;	
SrcCopy1616_ASM_LE16	
SrcCopy1616_ASM_LE16_LOOP	
	cmp 	r0, 	r1
	ble		SrcCopy1616_ASM_LE16_DEC
		
SrcCopy1616_ASM_LE16_INC
SrcCopy1616_ASM_LE16_INC_LABEL MEMCOPYMBTS_LT16 r0, r1, r2, #2, #2, r6, r12, ldrh, strh, ldrneh, strneh
	subs	r3,		r3,		#1
	add		r0,		r0,		r4, lsl #1
	add		r1,		r1,		r5, lsl #1
	mov		r2,		r7
	sub		r0,		r0,		r2, lsl #1
	sub		r1,		r1,		r2, lsl #1
	
	bgt		SrcCopy1616_ASM_LE16_LOOP
	; return from the function	
	ldr		r7,		[sp],	#4
	ldr		r6,		[sp],	#4
	ldr		r5,		[sp],	#4
	ldr		r4,		[sp],	#4
    IF Thumbing :LOR: Interworking
        bx  lr
    ELSE
        mov  pc, lr          ; return
    ENDIF

	
; (pSrc <= pDst ) use decrease mode
SrcCopy1616_ASM_LE16_DEC
	add		r0,		r0,		r2, lsl #1
	add		r1,		r1,		r2, lsl #1
	sub		r0,		r0,		#2
	sub		r1,		r1,		#2
SrcCopy1616_ASM_LE16_DEC_LABEL MEMCOPYMBTS_LT16 r0, r1, r2, #-2, #-2, r6, r12, ldrh, strh, ldrneh, strneh
	subs	r3,		r3,		#1
	add		r0,		r0,		#2  ;; important
	add		r1,		r1,		#2
	add		r0,		r0,		r4, lsl #1
	add		r1,		r1,		r5, lsl #1
	mov		r2,		r7
	bgt		SrcCopy1616_ASM_LE16_LOOP
	; return from the function
	ldr		r7,		[sp],	#4
	ldr		r6,		[sp],	#4
	ldr		r5,		[sp],	#4
	ldr		r4,		[sp],	#4
    IF Thumbing
        bx  lr
    ELSE
        mov  pc, lr          ; return
    ENDIF

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	
SrcCopy1616_ASM_GT16
	str		r8,		[sp,	#-4]!
	str		r9,		[sp,	#-4]!
	str		r10,	[sp,	#-4]!
	str		r11,	[sp,	#-4]!
	
SrcCopy1616_ASM_GT16_LOOP	
	subs	r12,	r0,		r1
	rsblt	r12,	r12,	#0
	
	cmp		r12,	r2,		lsl #1
	cmplt	r0,		r1
	blt		SrcCopy1616_ASM_GT16_DEC
	
SrcCopy1616_ASM_GT16_INC
	eor		r12,	r0,		r1
	tst		r12,	#2
	bne		SrcCopy1616_ASM_GT16_INC_2B
SrcCopy1616_ASM_GT16_INC_4B
	tst		r0,		#0x2
	ldrneh	r12,	[r0], #2
	subne	r2,		r2,	  #1
	mov		r11,	r2, 	lsr	#1 ; r11->number in 4-byte
	strneh	r12,	[r1], #2
	
	
SrcCopy1616_ASM_GT16_INC_4B_LABLE   MEMCOPYMBTS_LG16 r0, r1, r11, #4, #4, r6, r12, r8, r9, r10, ldr, str
	 
	tst		r2,		#0x1
	ldrneh	r12,	[r0], #2
	mov		r2,		r7
	strneh	r12,	[r1], #2
	
	sub		r0,		r0,		r2, lsl #1
	sub		r1,		r1,		r2, lsl #1
	add		r0,		r0,		r4, lsl #1
	add		r1,		r1,		r5, lsl #1
	subs	r3,		r3,		#1
	bgt		SrcCopy1616_ASM_GT16_LOOP

	;return from the function
	ldr		r11,	[sp],	#4
	ldr		r10,	[sp],	#4
	ldr		r9,		[sp],	#4
	ldr		r8,		[sp],	#4
	ldr		r7,		[sp],	#4
	ldr		r6,		[sp],	#4
	ldr		r5,		[sp],	#4
	ldr		r4,		[sp],	#4
    IF Thumbing :LOR: Interworking
        bx  lr
    ELSE
        mov  pc, lr          ; return
    ENDIF

 	
			
SrcCopy1616_ASM_GT16_INC_2B	
SrcCopy1616_ASM_GT16_INC_2B_LABLE	MEMCOPYMBTS_LG16 r0, r1, r2, #2, #2, r6, r12, r8, r9, r10, ldrh, strh
	mov		r2,		r7
	subs	r3,		r3,		#1
	add		r0,		r0,		r4, lsl #1
	add		r1,		r1,		r5, lsl #1
	sub		r0,		r0,		r2, lsl #1
	sub		r1,		r1,		r2, lsl #1

	bgt		SrcCopy1616_ASM_GT16_LOOP

	;return from the function
	ldr		r11,	[sp],	#4
	ldr		r10,	[sp],	#4
	ldr		r9,		[sp],	#4
	ldr		r8,		[sp],	#4
	ldr		r7,		[sp],	#4
	ldr		r6,		[sp],	#4
	ldr		r5,		[sp],	#4
	ldr		r4,		[sp],	#4
    IF Thumbing :LOR: Interworking
        bx  lr
    ELSE
        mov  pc, lr          ; return
    ENDIF



	
SrcCopy1616_ASM_GT16_DEC
	add		r0,		r0,		r2, lsl #1
	add		r1,		r1,		r2, lsl #1
	sub		r0,		r0,		#2
	sub		r1,		r1,		#2
	eor		r12,	r0,		r1
	tst		r12,	#2
	bne		SrcCopy1616_ASM_GT16_DEC_2B
	
SrcCopy1616_ASM_GT16_DEC_4B
	tst		r0,		#0x2
	ldreqh	r12,	[r0], #-2
	subeq	r2,		r2,	  #1
	mov		r11,	r2,   lsr	#1
	streqh	r12,	[r1], #-2
	
	sub		r0,		r0,		#2
	sub		r1,		r1,		#2
	
SrcCopy1616_ASM_GT16_DEC_4B_LABLE 	MEMCOPYMBTS_LG16 r0, r1, r11, #-4, #-4, r6, r12, r8, r9, r10, ldr, str

	add		r0,		r0,		#2
	add		r1,		r1,		#2
	 
	tst		r2,		#0x1
	ldrneh	r12,	[r0], #-2
	strneh	r12,	[r1], #-2
	
	add		r0,		r0,		#2  ;; important
	add		r1,		r1,		#2

	subs	r3,		r3,		#1
	add		r0,		r0,		r4, lsl #1
	add		r1,		r1,		r5, lsl #1
	mov		r2,		r7
	bgt		SrcCopy1616_ASM_GT16_LOOP

	;return from the function
	ldr		r11,	[sp],	#4
	ldr		r10,	[sp],	#4
	ldr		r9,		[sp],	#4
	ldr		r8,		[sp],	#4
	ldr		r7,		[sp],	#4
	ldr		r6,		[sp],	#4
	ldr		r5,		[sp],	#4
	ldr		r4,		[sp],	#4
    IF Thumbing :LOR: Interworking
        bx  lr
    ELSE
        mov  pc, lr          ; return
    ENDIF


 	
			
SrcCopy1616_ASM_GT16_DEC_2B	
SrcCopy1616_ASM_GT16_DEC_2B_LABLE	MEMCOPYMBTS_LG16 r0, r1, r2, #-2, #-2, r6, r12, r8, r9, r10, ldrh, strh

	add		r0,		r0,		#2  ;; important
	add		r1,		r1,		#2

	subs	r3,		r3,		#1
	add		r0,		r0,		r4, lsl #1
	add		r1,		r1,		r5, lsl #1
	mov		r2,		r7
	bgt		SrcCopy1616_ASM_GT16_LOOP

	;return from the function
	ldr		r11,	[sp],	#4
	ldr		r10,	[sp],	#4
	ldr		r9,		[sp],	#4
	ldr		r8,		[sp],	#4
	ldr		r7,		[sp],	#4
	ldr		r6,		[sp],	#4
	ldr		r5,		[sp],	#4
	ldr		r4,		[sp],	#4
    IF Thumbing :LOR: Interworking
        bx  lr
    ELSE
        mov  pc, lr          ; return
    ENDIF


	
	END
	
