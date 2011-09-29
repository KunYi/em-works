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
	
	EXPORT	EmulatedBltFill16ASM

	AREA	EBFILL16OPT,	CODE,	READONLY

;EmulatedBltFill16ASM(WORD* pDst, WORD color, int width, int height, int step)
;step is counted by byte
;r0		pDst
;r1		color
;r2		width
;r3		height
;r5		temp
;r6		step

STACKDEPTH	EQU		16
STEPPOS		EQU		STACKDEPTH

EmulatedBltFill16ASM

	stmdb	sp!,	{r4-r6,r14}
	
	ldr		r6,		[sp,	#STEPPOS]
	orr		r4,		r1,		r1,		LSL #16

	cmp		r2,		#0
	beq		EXIT_POS

	cmp		r3,		#0
	beq		EXIT_POS

	ands	r5,		r0,		#3		;WORD aligned?

	beq		START_0	

START_2

	sub		r6,		r6,		r2,	LSL #1		; r6 = step - width
	sub		r2,		r2,		#1

LOOP_HEIGHT_2

	strh	r1,		[r0],	#2
	cmp		r2,		#0				;r2	-> (width - 1)
	beq		LOOP_2_UPDATE

	movs	r5,		r2,		LSR #5
	beq		WIDTH_2_REMAIN



LOOP_WIDTH_2
	str		r4,		[r0],	#4
	str		r4,		[r0],	#4
	str		r4,		[r0],	#4
	str		r4,		[r0],	#4

	str		r4,		[r0],	#4
	str		r4,		[r0],	#4
	str		r4,		[r0],	#4
	str		r4,		[r0],	#4

	str		r4,		[r0],	#4
	str		r4,		[r0],	#4
	str		r4,		[r0],	#4
	str		r4,		[r0],	#4

	str		r4,		[r0],	#4
	str		r4,		[r0],	#4
	str		r4,		[r0],	#4
	str		r4,		[r0],	#4

	subs	r5,		r5,		#1		;Fill 32 units(16bits WORD) in each loop here
	bne		LOOP_WIDTH_2



WIDTH_2_REMAIN

	ands	r5,		r2,		#31
	beq		LOOP_2_UPDATE

	cmp		r5,		#16
	blt		REMAIN_2_1

	str		r4,		[r0],	#4
	str		r4,		[r0],	#4
	str		r4,		[r0],	#4
	str		r4,		[r0],	#4

	str		r4,		[r0],	#4
	str		r4,		[r0],	#4
	str		r4,		[r0],	#4
	str		r4,		[r0],	#4

	subs	r5,		r5,		#16
	beq		LOOP_2_UPDATE	

REMAIN_2_1
	
	cmp		r5,		#8
	blt		REMAIN_2_2

	str		r4,		[r0],	#4
	str		r4,		[r0],	#4
	str		r4,		[r0],	#4
	str		r4,		[r0],	#4

	subs	r5,		r5,		#8
	beq		LOOP_2_UPDATE

REMAIN_2_2

	cmp		r5,		#4
	
	blt		REMAIN_2_3

	str		r4,		[r0],	#4
	str		r4,		[r0],	#4

	subs	r5,		r5,		#4

	beq		LOOP_2_UPDATE

REMAIN_2_3

	strh	r1,		[r0],	#2
	subs	r5,		r5,		#1

	bne		REMAIN_2_3


LOOP_2_UPDATE

	add		r0,		r0,		r6			;r6 = step - width
	
	subs	r3,		r3,		#1
	bne		LOOP_HEIGHT_2

	b		EXIT_POS


START_0

	sub		r6,		r6,		r2,	LSL #1		; r6 = step - width	

LOOP_HEIGHT_0
	movs	r5,		r2,		LSR #5
	beq		WIDTH_0_REMAIN



LOOP_WIDTH_0
	str		r4,		[r0],	#4
	str		r4,		[r0],	#4
	str		r4,		[r0],	#4
	str		r4,		[r0],	#4

	str		r4,		[r0],	#4
	str		r4,		[r0],	#4
	str		r4,		[r0],	#4
	str		r4,		[r0],	#4

	str		r4,		[r0],	#4
	str		r4,		[r0],	#4
	str		r4,		[r0],	#4
	str		r4,		[r0],	#4

	str		r4,		[r0],	#4
	str		r4,		[r0],	#4
	str		r4,		[r0],	#4
	str		r4,		[r0],	#4

	subs	r5,		r5,		#1		;Fill 32 units(16bits WORD) in each loop here
	bne		LOOP_WIDTH_0


WIDTH_0_REMAIN

	ands	r5,		r2,		#31
	beq		LOOP_0_UPDATE
	
	cmp		r5,		#16
	blt		REMAIN_0_1

	str		r4,		[r0],	#4
	str		r4,		[r0],	#4
	str		r4,		[r0],	#4
	str		r4,		[r0],	#4

	str		r4,		[r0],	#4
	str		r4,		[r0],	#4
	str		r4,		[r0],	#4
	str		r4,		[r0],	#4

	subs	r5,		r5,		#16
	beq		LOOP_0_UPDATE	

REMAIN_0_1
	
	cmp		r5,		#8
	blt		REMAIN_0_2

	str		r4,		[r0],	#4
	str		r4,		[r0],	#4
	str		r4,		[r0],	#4
	str		r4,		[r0],	#4

	subs	r5,		r5,		#8
	beq		LOOP_0_UPDATE

REMAIN_0_2

	cmp		r5,		#4
	
	blt		REMAIN_0_3

	str		r4,		[r0],	#4
	str		r4,		[r0],	#4

	subs	r5,		r5,		#4

	beq		LOOP_0_UPDATE

REMAIN_0_3

	strh	r1,		[r0],	#2
	subs	r5,		r5,		#1

	bne		REMAIN_0_3
	
LOOP_0_UPDATE

	add		r0,		r0,		r6			;r6 = step - width
	
	subs	r3,		r3,		#1
	bne		LOOP_HEIGHT_0

EXIT_POS
	mov		r0,		#0
	LDMIA	sp!,	{r4-r6, pc}



	end




