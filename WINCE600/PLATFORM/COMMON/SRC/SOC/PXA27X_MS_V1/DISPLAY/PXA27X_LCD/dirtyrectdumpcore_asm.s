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


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;  void dirtyRectDump_core_ASM(WORD *pwSrc, WORD *pwDst,int rowLen, DWORD srcWidthB,	  ;
; 						  DWORD bytesPerRow, DWORD srcMarginWidth, DWORD dstMarginWidth)  ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	INCLUDE memcopy_macro.inc
	EXPORT	dirtyRectDump_core_ASM
	
	AREA	EBCOPY16OPT,	CODE,	READONLY
	
dirtyRectDump_core_ASM
	; r0-> pwSrc
	; r1-> pwDst
	; r2-> rowLen
	; r3-> srcWidthB
	; r4-> bytesPerRow
	; r5-> srcMarginWidth
	; r6-> dstMarginWidth
	
STACKDEPTH					EQU		(5*4)
	
BYTESPERROW					EQU		(STACKDEPTH+0)
SRCMARGINWIDTH				EQU		(STACKDEPTH+4)
DSTMARGINWIDTH				EQU		(STACKDEPTH+8)
		
	stmdb	sp!,	{r4-r7,r14}
	
	ldr		r4,		[sp,	#BYTESPERROW]
	ldr		r5,		[sp,	#SRCMARGINWIDTH]
	ldr		r6,		[sp,	#DSTMARGINWIDTH]
	
	cmp		r2,		#0
	ble		EXIT_POS

	cmp		r3,		#0
	ble		EXIT_POS

	mov		r7,		r2
	rsb		r4,		r4,		#0	; r4-> -bytesPerRow
	add		r5,		r5,		#2  ; r5->srcMarginWidth+2
	
	cmp		r2,		#16
	bgt		ROW_LEN_GT16


IWIDTH_LOOP_LE16	
	
ROW_LEN_LE16_MMCP_LABEL MEMCOPYMBTS_LT16 r0, r1, r2, r4, #2, r12, r14, ldrh, strh, ldrneh, strneh
	add		r0,		r0,		r5
	add		r1,		r1,		r6
	subs	r3,		r3,		#1
	mov		r2,		r7
	bgt		IWIDTH_LOOP_LE16
	ldmia	sp!,	{r4-r7,pc}
	
ROW_LEN_GT16	
	str		r8,		[sp,	#-4]!
	str		r9,		[sp,	#-4]!
	str		r10,	[sp,	#-4]!
	
IWIDTH_LOOP_GT16	

ROW_LEN_GT16_MMCP_LABEL MEMCOPYMBTS_LG16 r0, r1, r2, r4, #2, r10, r12, r14, r8, r9, ldrh, strh
	;;pwDst +=dstMarginWidth;
	add		r0,		r0,		r5
	add		r1,		r1,		r6
	subs	r3,		r3,		#1
	mov		r2,		r7
	bgt		IWIDTH_LOOP_GT16

	ldr		r10,	[sp],	#4
	ldr		r9,		[sp],	#4
	ldr		r8,		[sp],	#4

EXIT_POS	
	ldmia	sp!,	{r4-r7,pc}


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;  void dirtyRectDump_core_wop_ASM(WORD *pwSrc, WORD *pwDst,int rowLen, DWORD srcWidthB,	  ;
; 						  DWORD bytesPerRow, DWORD srcMarginWidth, DWORD dstMarginWidth)  ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	INCLUDE memcopy_macro.inc
	EXPORT	dirtyRectDump_core_wop_ASM
	
	AREA	DISPLAYDRV,	CODE,	READONLY
	
dirtyRectDump_core_wop_ASM
	; r0-> pwSrc
	; r1-> pwDst
	; r2-> rowLen
	; r3-> srcWidthB
	; r4-> bytesPerRow
	; r5-> srcMarginWidth
	; r6-> dstMarginWidth
	
STACKDEPTH_WOP				EQU		(9*4)
	
BYTESPERROW_WOP				EQU		(STACKDEPTH_WOP+0)
SRCMARGINWIDTH_WOP			EQU		(STACKDEPTH_WOP+4)
DSTMARGINWIDTH_WOP			EQU		(STACKDEPTH_WOP+8)
		
	stmdb	sp!,	{r4-r11,r14}
	
	ldr		r4,		[sp,	#BYTESPERROW_WOP]
	ldr		r5,		[sp,	#SRCMARGINWIDTH_WOP]
	ldr		r6,		[sp,	#DSTMARGINWIDTH_WOP]
	
	cmp		r2,		#0
	ble		WOP_EXIT_POS

	cmp		r3,		#0
	ble		WOP_EXIT_POS

	rsb		r4,		r4,		#0	; r4-> -bytesPerRow
	mov		r7,		r2
	add		r5,		r5,		#2  ; r5->srcMarginWidth+2

	tst		r0,		#2
	beq		WOP_4B_ALIGN
WOP_MMCP_LABEL MEMCOPYMBTS_LG16 r0, r1, r2, r4, #2, r10, r12, r14, r8, r9, ldrh, strh
	sub		r3,		r3,		#1  ; r3->srcWidthB --
	add		r0,		r0,		r5  ; pwSrc +=srcMarginWidth+1
	add		r1,		r1,		r6  ; pwDst +=dstMarginWidth;
	;************************************
	;pld 		[r0] ; This will save sometime for sure.//Check the syntax
	;************************************
	DCD   0xf5d0f000
WOP_4B_ALIGN	
	; now pwSrc is 4-byte aligned
	movs   r8,		r3,		lsr #1 ; r8->srcWidthB>>1
	beq	   WOP_4B_WIDTH_BLOCK_EXIT ; jump to the last colume in source
	
	; prepare for 0xffff
	mov		r9,		#0xff00
	add		r9,		r9,		#0xff	; r9->0xffff	
	add		r10,	r6,		r7, lsl #1; r10->pwDst2, next line in destination
	add		r10,	r10,	r1
	
WOP_4B_WIDTH_BLOCK_LOOP
	stmdb	sp!,	{r5-r8}		   ; push and keep r5-r7 on stack
	mov		r2,		r7			   ; restore r2->rowLen
	tst		r1,		#2
	beq		WOP_DST_4B_ALGIN
	ldr		r12,	[r0],	r4
	;************************************
	;pld 		[r0] ; This will save sometime for sure.//Check the syntax
	;pld 		[r0,r4] ; This will save sometime for sure.//Check the syntax
	;pld 		[r0,2*r4] ; This will save sometime for sure.//Check the syntax
	;pld 		[r0,3*r4] ; This will save sometime for sure.//Check the syntax
	;************************************
	DCD     0xf5d0f000
	DCD     0xf7d0f004
	DCD     0xf7d0f0a4
	
	sub		r2,		r2,		#1
	strh	r12,	[r1],	#2			
	and		r14,	r9,		r12,	lsr #16	; r14->(tmp>>16) & 0xffff
	strh	r14,	[r10],  #2
	
WOP_DST_4B_ALGIN
	; now pwDst is 4-byte aligned	
	movs	r14,	r2,		lsr	#2 ; r14->rowLen>>2
	beq		WOP_4B_WIDTH_SQUARE2
	;need to keep r5, r6, r7, r8
	
WOP_4B_WIDTH_SQUARE_LOOP
	
	ldr		r5,		[r0],	r4
	ldr		r11,	[r0],	r4
	ldr		r8,		[r0],	r4
	ldr		r12,	[r0],	r4
	
	
	
	and		r6,		r5,		r9				; r6->tmp = tmp0 & 0xffff;
	orr		r6,		r6,		r11, lsl #16	; r6->tmp |= tmp1 << 16;
	DCD     0xf5d0f000	
	and		r11,	r11,	r9,  lsl #16	; r11->tmp1 &= 0xffff0000;
	and		r7,		r8,		r9				; r6->tmp = tmp0 & 0xffff;
	orr		r11,	r11,	r5,  lsr #16	; r11->tmp1 |= tmp0 >> 16;
	
	
	orr		r7,		r7,		r12, lsl #16	; r6->tmp |= tmp1 << 16;
	str		r6,		[r1],	#4
	str		r7,		[r1],	#4
	
	and		r12,	r12,	r9,  lsl #16	; r11->tmp1 &= 0xffff0000;
	orr		r12,	r12,	r8,  lsr #16	; r11->tmp1 |= tmp0 >> 16;
	str		r11,	[r10],	#4
	str		r12,	[r10],	#4
	
			
	subs	r14,	r14,	#1
	bgt		WOP_4B_WIDTH_SQUARE_LOOP
	
	
	
			
WOP_4B_WIDTH_SQUARE2	
	tst		r2,		#2
	beq		WOP_4B_WIDTH_SQUARE2_NEXT
	
	ldr		r5,		[r0],	r4
	ldr		r11,	[r0],	r4
	
	and		r6,		r5,		r9				; r6->tmp = tmp0 & 0xffff;
	orr		r6,		r6,		r11, lsl #16	; r6->tmp |= tmp1 << 16;
	DCD     0xf7d0f004
	str		r6,		[r1],	#4
	
	
	and		r11,	r11,	r9,  lsl #16	; r11->tmp1 &= 0xffff0000;
	orr		r11,	r11,	r5,  lsr #16	; r11->tmp1 |= tmp0 >> 16;
	str		r11,	[r10],	#4
	
WOP_4B_WIDTH_SQUARE2_NEXT

	
	tst		r2,		#1
	beq		WOP_4B_WIDTH_ROWLEN1_NEXT
	ldr		r12,	[r0],	r4
	and		r14,	r9,		r12,	lsr #16	; r14->(tmp>>16) & 0xffff
	strh	r12,	[r1],	#2	
	strh	r14,	[r10],  #2

WOP_4B_WIDTH_ROWLEN1_NEXT


	ldmia	sp!,	{r5-r8}		   ; pop up r5-r8 from stack

	;update r0->pSrc, r1->pDst, r10->pDst2
	
	
	add		r1,		r1,		r6,		lsl #1
	add		r0,		r0,		#2
	add		r1,		r1,		r7,		lsl #1
	add		r10,	r10,	r6,		lsl #1
	add		r0,		r0,		r5
	add		r10,	r10,	r7,		lsl #1
	subs   r8,		r8,		#1
	
	bgt	   WOP_4B_WIDTH_BLOCK_LOOP
	
WOP_4B_WIDTH_BLOCK_EXIT
;last colume in source
	tst		r3,		#1
	beq		WOP_4B_WIDTH1_EXIT
	mov		r2,		r7
WOP_4B_WIDTH1_LABEL MEMCOPYMBTS_LG16 r0, r1, r2, r4, #2, r10, r12, r14, r8, r9, ldrh, strh

WOP_4B_WIDTH1_EXIT
WOP_EXIT_POS	
	ldmia	sp!,	{r4-r11,pc}

	EXPORT ellipse_core_ASM

ellipse_core_ASM
;	r0 --> src Color;
;	r1 --> line margin
;	r2 --> width;
;   r3 --> pDst Buffer

	stmdb	sp!,	{r4-r11,r14}	;
	add r4, r3, r1			; r4 = pDst + margin
	add r5, r3, r1, LSL #1	; r5 = pDst + 2*margin
	add r6, r4, r1, LSL #1	; r6 = pDst + margin + 2* margin
	add r7, r3, r1, LSL #2	; r7 = pDst + 4*margin
	add r8, r4, r1, LSL #2	; r8 = pDst + margin + 4*margin
	add r9, r5, r1, LSL #2	; r9 = pDst + 2*margin + 4*margin
	add r10,r6, r1, LSL #2	; r10 = pDst + margin + 2*margin + 4*margin


	cmp r2, #0x7;
	mov r11, r2,LSR #3	;
	ble  ellipse_end	;
	
	mov r1, r1, LSL #3	;
	
ellipse_loop
	subs r11, r11, #1	;
	strh r0, [r3], r1	;
	strh r0, [r4], r1	;
	strh r0, [r5], r1	;
	strh r0, [r6], r1	;
	strh r0, [r7], r1	;
	strh r0, [r8], r1	;
	strh r0, [r9], r1	;
	strh r0, [r10], r1	;
	bne ellipse_loop	;
	
	and r2, r2, #0x7	;
	mov r1, r1, LSR #3	;  

ellipse_end
	cmp r2, #0			;
	beq ellipse_exit	;

ellipse_loop2	
	subs r2, r2, #1
	strh r0, [r3], r1	;
	bne ellipse_loop2		;

ellipse_exit
	ldmia	sp!,	{r4-r11,pc}	


	END
	

