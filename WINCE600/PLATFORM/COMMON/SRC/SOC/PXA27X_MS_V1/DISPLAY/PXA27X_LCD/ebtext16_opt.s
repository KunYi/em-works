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
	
	EXPORT	EmulatedBltText16ASM

	AREA	EBTEXT16OPT,	CODE,	READONLY

;EmulatedBltText16ASM(BYTE* pSrcMask, WORD* pDstPixel, WORD color, 
;int width, int height, int stepMask, int stepDst, int OffsetSrc)

;stepMask and stepDst are all counted by byte




;r0		pSrcMask
;r1		pDstPixel
;r2		color
;r3		width (heer part width laterly)
;r4		height
;r5		stepMask
;r6		stepDst
;r8		OffsetSrc


;	 |<-   r12   ->|<-               r3                     |
;    |Head residue |<-    body   part         ->|heer part  |
;| O O O X X X X X |X X X X X X X X|X X...   X X|X X X X X X O O|

STACKDEPTH		EQU		40
HEIGHTPOS		EQU		STACKDEPTH
STEPMASKPOS		EQU		STACKDEPTH + 4
STEPDSTPOS		EQU		STACKDEPTH + 8
OFFSETSRCPOS	EQU		STACKDEPTH + 12

EmulatedBltText16ASM

	stmdb	sp!,	{r4-r12,r14}
	
	
	ldr		r6,		[sp,	#STEPDSTPOS]
	ldr		r8,		[sp,	#OFFSETSRCPOS]
	ldr		r5,		[sp,	#STEPMASKPOS]
	ldr		r4,		[sp,	#HEIGHTPOS]

;	ands	r14,	r1,		#1
;	bne		EXIT_POS
	
;	cmp		r6,		#0
;	ble		EXIT_POS
	
;	ands	r14,		r6,		#3
;	bne		EXIT_POS
	
;	cmp		r3,		#0
;	ble		EXIT_POS
	
;	cmp		r4,		#0
;	ble		EXIT_POS
	
;	cmp		r8,		#0
;	blt		EXIT_POS
	
;	cmp		r8,		#7
;	bgt		EXIT_POS
	
	mov		r2,		r2,		LSL	#16
	mov		r2,		r2,		LSR #16
	orr		r11,	r2,		r2,		LSL #16
	
	
	sub		r5,		r5,		#1			; remove Head residue
	
	add		r14,	r3,		r8
	subs	r14,	r14,	#8	

	bge		NORMAL_INIT
	
	mov		r12,	r3					;r12 the starting residue part number
	sub		r6,		r6,		r3,	LSL #1

;	cmp		r8,		#0

;	moveq	r3,		#8
;	moveq	r12,	#0
	
;	movne	r3,		#0					;r3 = body part + heer part = 0
	mov		r3,		#0
	
	b		LOOP_HEIGHT
	
NORMAL_INIT

	sub		r5,		r5,		r14,	LSR #3	;body part, note r14 is positive here
	
	ands	r12,	r14,	#7
	
	subne	r5,		r5,		#1			; heer residue
										; r5 = stepMask - width
										
	sub		r6,		r6,		r3,	LSL #1	;r6 = stepDst - width
	
	
	sub		r12,	r3,	r14
	mov		r3,		r14					;r3 = body part + heer part
	
	
	cmp		r8,		#0
	addeq	r3,		r3,		#8
	moveq	r12,	#0
	

LOOP_HEIGHT


	movs	r14,	r12	

	beq		ALIGNED_START
	
	ldrb	r9,		[r0],	#1

	mov		r7,		#0x80
	mov		r7,		r7,		LSR r8		;r7 = Mask = 0x80 >> OffsetSrc


START_REMAINS

	ands	r10,	r9,		r7
	
	strneh	r2,		[r1]
	add		r1,		r1,		#2

	mov		r7,		r7,		LSR #1
	
	subs	r14,	r14,	#1
	bne		START_REMAINS

ALIGNED_START

	movs	r14,	r3,	LSR #3
	beq		WIDTH_REMAIN

	ldrb	r9,		[r0],	#1

LOOP_WIDTH						;Each Loop deal with 8 pixels
	
	cmp		r9,		#0
	beq		case_0

	cmp		r9,		#0xff
	beq		case_f

case_others

	ands	r10,	r9,		#0x80
	strneh	r2,		[r1]
	
	ands	r10,	r9,		#0x40
	strneh	r2,		[r1, #2]

	ands	r10,	r9,		#0x20
	strneh	r2,		[r1, #4]
	
	ands	r10,	r9,		#0x10
	strneh	r2,		[r1, #6]

	ands	r10,	r9,		#0x08
	strneh	r2,		[r1, #8]

	ands	r10,	r9,		#0x04
	strneh	r2,		[r1, #10]

	ands	r10,	r9,		#0x02
	strneh	r2,		[r1, #12]

	ands	r10,	r9,		#0x01
	strneh	r2,		[r1, #14]

	add		r1,		r1,		#16
	subs	r14,	r14,	#1
	
	ldrneb	r9,		[r0],	#1
	bne		LOOP_WIDTH

	b		WIDTH_REMAIN
	
case_f

	;str		r11,	[r1],	#4
	;str		r11,	[r1],	#4
	;str		r11,	[r1],	#4
	;str		r11,	[r1],	#4
	
	strh		r2,		[r1],	#2
	strh		r2,		[r1],	#2

	strh		r2,		[r1],	#2
	strh		r2,		[r1],	#2		
	
	strh		r2,		[r1],	#2
	strh		r2,		[r1],	#2
	
	strh		r2,		[r1],	#2
	strh		r2,		[r1],	#2
	
	subs	r14,	r14,	#1
	ldrneb	r9,		[r0],	#1
	bne		LOOP_WIDTH

	b		WIDTH_REMAIN
	

case_0

	add		r1,		r1,		#16
	subs	r14,	r14,	#1
	ldrneb	r9,		[r0],	#1
	bne		LOOP_WIDTH


WIDTH_REMAIN

	ands	r14,	r3,		#7
	ldrneb	r9,		[r0],	#1
	beq		LOOP_UPDATE
	
	mov		r7,		#0x80

LOOP_REMAIN

	ands	r10,	r9,		r7
	strneh	r2,		[r1]
	add		r1,		r1,		#2

	mov		r7,		r7,		LSR #1
	subs	r14,	r14,	#1
	bne		LOOP_REMAIN

LOOP_UPDATE

	subs	r4,		r4,		#1
	addgt	r1,		r1,		r6	;pDstPixel	+= (stepDst - width)
	addgt	r0,		r0,		r5	;pSrcMask	+= (stepMask - width)
	bgt		LOOP_HEIGHT

EXIT_POS
	
	mov		r0,		#0
	ldmia	sp!,	{r4-r12, pc}
	

	END

