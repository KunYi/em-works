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
	
	EXPORT	EmulatedBltSrcCopy1616ASM

	AREA	EBCOPY16OPT,	CODE,	READONLY

;EmulatedBltSrcCopy1616ASM(WORD* pwScanLineSrc, WORD *pwScanLineDst, int width, int height,
;							UINT iScanStrideSrc, UINT iScanStrideDst, int xPositive)
;step is counted by byte
;r0		pwScanLineSrc
;r1		pwScanLineDst
;r2		width
;r3		height
;;
;r4		iScanStrideSrc->sp
;r5		iScanStrideDst->sp+4
;r6		XPOSITIVE		

PWSCANLINESRC				EQU		0
PWSCANLINEDST				EQU		4
WIDTH						EQU		8
HEIGHT						EQU		12

STACKDEPTH					EQU		(13*4)
ISCANSTRIDESRC				EQU		STACKDEPTH
ISCANSTRIDEDST				EQU		(STACKDEPTH+4)
XPOSITIVE					EQU		(STACKDEPTH+8)

EmulatedBltSrcCopy1616ASM

	stmdb	sp!,	{r0-r11,r14}
	

	cmp		r2,		#0
	ble		EXIT_POS

	cmp		r3,		#0
	ble		EXIT_POS

	ldr		r4,		[sp,	#ISCANSTRIDESRC]
	ldr		r5,		[sp,	#ISCANSTRIDEDST]
	ldr		r6,		[sp,	#XPOSITIVE]

	cmp		r6,		#0
	movgt	r6,		#2
	movle	r6,		#-2
	mov		r7,		r6,	lsl #1; r7 = 4/-4

	eor		r14,		r0,		r1
	ands	r14,		r14,	#0x3
	bne		COPY_IN_16BIT_CASE
;;;;;;;;;;;;; copy in 32bit case
COPY_IN_32BIT_CASE
	; set pwPixelDst & pwPixelSrc first 
	cmp		r6,		#0
	ble		COPY_IN_32BIT_RIGHT_TO_LEFT


COPY_IN_32BIT_LEFT_TO_RIGHT
	; Copy from left to right xPositive > 0

;; in the row loop
COPY_IN_32BIT_ROW_LOOP_INC
	str		r0,		[sp,	#PWSCANLINESRC]
	str		r1,		[sp,	#PWSCANLINEDST]
	
COPY_IN_32BIT_INCREASE	
	tst		r0,		#0x2			; r0->pScanSrc
	beq		COPY_IN_32BIT_A4_INC
COPY_IN_32BIT_A2_INC
	; pwScanLineSrc and pwScanLineDst are both 2-byte aligned: 0x2, 0x6
	ldrh	r10,		[r0],	r6
	strh	r10,		[r1],	r6
	sub		r2,			r2,		#1
COPY_IN_32BIT_A4_INC
	; pwScanLineSrc and pwScanLineDst are both 4-byte aligned
	; loop unrooling at 4 now
	movs	r14,		r2,		lsr #2 ; r2	--> width
	beq		COPY_IN_32BIT_HEIGHT_LOOP_INC_B4_EXIT
COPY_IN_32BIT_HEIGHT_LOOP_INC_B4	
	ldr		r10,		[r0],	r7
	ldr		r11,		[r0],	r7
	str		r10,		[r1],	r7
	str		r11,		[r1],	r7
	subs	r14,		r14,	#1
	bgt		COPY_IN_32BIT_HEIGHT_LOOP_INC_B4	
	
COPY_IN_32BIT_HEIGHT_LOOP_INC_B4_EXIT	
	ands	r14,		r2,		#3
	beq		COPY_IN_32BIT_HEIGHT_LOOP_INC_EXIT

COPY_IN_32BIT_HEIGHT_LOOP_INC_B1	
	ldrh	r10,		[r0],	r6
	strh	r10,		[r1],	r6
	subs	r14,		r14,	#1
	bgt		COPY_IN_32BIT_HEIGHT_LOOP_INC_B1	

COPY_IN_32BIT_HEIGHT_LOOP_INC_EXIT

;;update the pointers etc for row loop
	ldr		r2,			[sp,	#WIDTH]
	ldr		r0,			[sp,	#PWSCANLINESRC]
	ldr		r1,			[sp,	#PWSCANLINEDST]
	add		r0,			r0,		r4,		lsl #1
	add		r1,			r1,		r5,		lsl #1
	subs	r3,			r3,		#1;  r3->height
	bne		COPY_IN_32BIT_ROW_LOOP_INC
	b		EXIT_POS
	
	; Copy from right to left xPositive < 0
COPY_IN_32BIT_RIGHT_TO_LEFT
 ; r0->pwScanLineSrc + cCols - 1
 ; r1->pwScanLineSrc + cCols - 1
	add		r0,		r0,		r2,	lsl #1
	sub		r0,		r0,		#2
	add		r1,		r1,		r2,	lsl #1
	sub		r1,		r1,		#2
	
COPY_IN_32BIT_DECREASE

COPY_IN_32BIT_ROW_LOOP_DEC
	str		r0,		[sp,	#PWSCANLINESRC]
	str		r1,		[sp,	#PWSCANLINEDST]

	tst		r0,		#0x2
	bne		COPY_IN_32BIT_DEC_A2
COPY_IN_32BIT_DEC_A4
	; pwScanLineSrc and pwScanLineDst are both 2-byte aligned: 0x2, 0x6
	ldrh	r10,		[r0],	r6
	strh	r10,		[r1],	r6
	sub		r2,			r2,		#1	
COPY_IN_32BIT_DEC_A2
	; now r0, r1 is at 2-byte align, such as 0x2, 0x4
	; make sure that there are more than 2 short pixels need to be copied
	cmp		r2,			#1
	ldreqh	r10,		[r0],	r6
	streqh	r10,		[r1],	r6
	subeqs	r2,			r2,		#1
	beq		COPY_IN_32BIT_HEIGHT_LOOP_DEC_EXIT
	
	add		r0,			r0,		r6  ; let r0, r1 4-byte aligned
	add		r1,			r1,		r6
	
	; pwScanLineSrc and pwScanLineDst are both 4-byte aligned
	; loop unrooling at 4 now
	movs	r14,		r2,		lsr #2 ; r2	--> width
	beq		COPY_IN_32BIT_HEIGHT_LOOP_DEC_B4_EXIT
COPY_IN_32BIT_HEIGHT_LOOP_DEC_B4	
	ldr		r10,		[r0],	r7
	ldr		r11,		[r0],	r7
	str		r10,		[r1],	r7
	str		r11,		[r1],	r7
	subs	r14,		r14,	#1
	bgt		COPY_IN_32BIT_HEIGHT_LOOP_DEC_B4	
	
COPY_IN_32BIT_HEIGHT_LOOP_DEC_B4_EXIT	
	ands	r14,		r2,		#3
	beq		COPY_IN_32BIT_HEIGHT_LOOP_DEC_EXIT
	
	sub		r0,			r0,		r6 ; switch to 2-byte operation again
	sub		r1,			r1,		r6
		
COPY_IN_32BIT_HEIGHT_LOOP_DEC_B1	
	ldrh	r10,		[r0],	r6
	strh	r10,		[r1],	r6
	subs	r14,		r14,	#1
	bgt		COPY_IN_32BIT_HEIGHT_LOOP_DEC_B1	

COPY_IN_32BIT_HEIGHT_LOOP_DEC_EXIT

;;update the pointers etc for row loop
	ldr		r2,			[sp,	#WIDTH]
	ldr		r0,			[sp,	#PWSCANLINESRC]
	ldr		r1,			[sp,	#PWSCANLINEDST]
	add		r0,			r0,		r4,		lsl #1
	add		r1,			r1,		r5,		lsl #1
	subs	r3,			r3,		#1;  r3->height
	bne		COPY_IN_32BIT_ROW_LOOP_DEC
	b		EXIT_POS	
;;;;;;;;;;;;; end of copy in 32bit case ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

COPY_IN_16BIT_CASE
	; set pwPixelDst & pwPixelSrc first 
	cmp		r6,		#0
	bne		COPY_IN_16BIT_LEFT_TO_RIGHT
	; Copy from right to left xPositive < 0
	
COPY_IN_16BIT_RIGHT_TO_LEFT
 ; r0->pwScanLineSrc + cCols - 1
 ; r1->pwScanLineSrc + cCols - 1
	add		r0,		r0,		r2,	lsl #1
	sub		r0,		r0,		#2
	add		r1,		r1,		r2,	lsl #1
	sub		r1,		r1,		#2
COPY_IN_16BIT_LEFT_TO_RIGHT	
	; Copy from left to right xPositive > 0

;; in the row loop
COPY_IN_16BIT_ROW_LOOP
	str		r0,		[sp,	#PWSCANLINESRC]
	str		r1,		[sp,	#PWSCANLINEDST]
	
	; loop unrooling at 4 now
	movs	r14,		r2,		lsr #2 ; r2	--> width
	beq		COPY_IN_16BIT_HEIGHT_LOOP_B4_EXIT
COPY_IN_16BIT_HEIGHT_LOOP_B4	
	ldrh	r10,		[r0],	r6
	ldrh	r11,		[r0],	r6
	strh	r10,		[r1],	r6
	strh	r11,		[r1],	r6
	
	ldrh	r10,		[r0],	r6
	ldrh	r11,		[r0],	r6
	strh	r10,		[r1],	r6
	strh	r11,		[r1],	r6

	subs	r14,		r14,	#1
	bgt		COPY_IN_16BIT_HEIGHT_LOOP_B4	
	
COPY_IN_16BIT_HEIGHT_LOOP_B4_EXIT	
	ands	r14,		r2,		#3
	beq		COPY_IN_16BIT_HEIGHT_LOOP_EXIT

COPY_IN_16BIT_HEIGHT_LOOP_B1	
	ldrh	r10,		[r0],	r6
	strh	r10,		[r1],	r6
	subs	r14,		r14,	#1
	bgt		COPY_IN_16BIT_HEIGHT_LOOP_B1	

COPY_IN_16BIT_HEIGHT_LOOP_EXIT

;;update the pointers etc for row loop
	ldr		r2,			[sp,	#WIDTH]
	ldr		r0,			[sp,	#PWSCANLINESRC]
	ldr		r1,			[sp,	#PWSCANLINEDST]
	add		r0,			r0,		r4,		lsl #1
	add		r1,			r1,		r5,		lsl #1
	subs	r3,			r3,		#1;  r3->height
	bne		COPY_IN_16BIT_ROW_LOOP
	
	
	
EXIT_POS
	mov		r0,		#0
	LDMIA	sp!,	{r0-r11,pc}

	end




