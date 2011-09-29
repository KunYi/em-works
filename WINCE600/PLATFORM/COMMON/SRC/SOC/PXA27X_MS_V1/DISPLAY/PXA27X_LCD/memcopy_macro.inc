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
	IF 		:LNOT:	:DEF:	__MEMCOPY_MACRO_INC_
	GBLL	__MEMCOPY_MACRO_INC_
	; len >= 16, use this method to avoid much pipeline stalls
	MACRO
$mmbtl	MEMCOPYMBTS_LG16 $pSrc, $pDst, $len, $inc0, $inc1, $tReg, $dReg0, $dReg1, $dReg2, $dReg3, $ldrop, $strop 
	; loop unrooling at 8 now
	movs	$tReg,		$len,		lsr #3 ; $len	--> width
	beq		$mmbtl.COPY_IN_16BIT_HEIGHT_LOOP_B8_EXIT
$mmbtl.COPY_IN_16BIT_HEIGHT_LOOP_B8	
	$ldrop	$dReg0,		[$pSrc],	$inc0
	$ldrop	$dReg1,		[$pSrc],	$inc0
	$ldrop	$dReg2,		[$pSrc],	$inc0
	$ldrop	$dReg3,		[$pSrc],	$inc0
	subs	$tReg,		$tReg,		#1

	$strop	$dReg0,		[$pDst],	$inc1
	$strop	$dReg1,		[$pDst],	$inc1
	$strop	$dReg2,		[$pDst],	$inc1
	$strop	$dReg3,		[$pDst],	$inc1

	$ldrop	$dReg0,		[$pSrc],	$inc0
	$ldrop	$dReg1,		[$pSrc],	$inc0
	$ldrop	$dReg2,		[$pSrc],	$inc0
	$ldrop	$dReg3,		[$pSrc],	$inc0

	$strop	$dReg0,		[$pDst],	$inc1
	$strop	$dReg1,		[$pDst],	$inc1
	$strop	$dReg2,		[$pDst],	$inc1
	$strop	$dReg3,		[$pDst],	$inc1

	bgt		$mmbtl.COPY_IN_16BIT_HEIGHT_LOOP_B8	
	
$mmbtl.COPY_IN_16BIT_HEIGHT_LOOP_B8_EXIT	
	ands	$tReg,		$len,		#7
	beq		$mmbtl.COPY_IN_16BIT_HEIGHT_LOOP_EXIT

$mmbtl.COPY_IN_16BIT_HEIGHT_LOOP_B1	
	$ldrop	$dReg0,		[$pSrc],	$inc0
	subs	$tReg,		$tReg,		#1
	$strop	$dReg0,		[$pDst],	$inc1
	bgt		$mmbtl.COPY_IN_16BIT_HEIGHT_LOOP_B1	

$mmbtl.COPY_IN_16BIT_HEIGHT_LOOP_EXIT

	MEND
	
	; len < 16, use this method to avoid much overheader: less register required, less branch
	; input:
	; $pSrc  ->reg pointer to the source buffer
	; $pDst  ->reg pointer to the destination buffer
	; $len   ->reg buffer length
	; $inc0  ->reg or immediate data(8bit?), address increasement offset for pSrc
	; $inc1  ->reg or immediate data(8bit?), address increasement offset for pDst
	; $ldrop ->load operation, can be ldr, ldrh, ldrb
	; $strop ->store operation, can be str, strh, strb
	; work registers
	; $dReg0 -> data reg0
	; $dReg1 -> data reg1, must use different register from $dReg0  
	; use 5 registers
	MACRO
$mmbts	MEMCOPYMBTS_LT16 $pSrc, $pDst, $len, $inc0, $inc1, $dReg0, $dReg1, $ldrop, $strop, $ldrop2, $strop2 
	; loop unrooling at 8 now
	cmp			$len,		#0
	ble			$mmbts.COPY_IN_16BIT_HEIGHT_LOOP_B2_EXIT
	
	tst			$len,		#1
	$ldrop2		$dReg0,		[$pSrc],	$inc0
	$strop2		$dReg0,		[$pDst],	$inc1

	movs	$len,		$len,		lsr #1 ; $len	--> width
	beq		$mmbts.COPY_IN_16BIT_HEIGHT_LOOP_B2_EXIT
$mmbts.COPY_IN_16BIT_HEIGHT_LOOP_B2	
	$ldrop	$dReg0,		[$pSrc],	$inc0
	$ldrop	$dReg1,		[$pSrc],	$inc0
	subs	$len,		$len,		#1
	$strop	$dReg0,		[$pDst],	$inc1
	$strop	$dReg1,		[$pDst],	$inc1
	bgt		$mmbts.COPY_IN_16BIT_HEIGHT_LOOP_B2	
$mmbts.COPY_IN_16BIT_HEIGHT_LOOP_B2_EXIT	

	MEND
	
	
	ENDIF	
	
	END
	

	
