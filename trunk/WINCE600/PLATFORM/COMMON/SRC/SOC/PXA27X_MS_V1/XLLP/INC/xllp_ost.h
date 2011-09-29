//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
#ifndef __XLLP_OST_H__
#define __XLLP_OST_H__

/******************************************************************************
**
** Copyright 2000-2003 Intel Corporation All Rights Reserved.
**
** Portions of the source code contained or described herein and all documents
** related to such source code (Material) are owned by Intel Corporation
** or its suppliers or licensors and is licensed by Microsoft Corporation for distribution.  
** Title to the Material remains with Intel Corporation or its suppliers and licensors. 
** Use of the Materials is subject to the terms of the Microsoft license agreement which accompanied the Materials.  
** No other license under any patent, copyright, trade secret or other intellectual
** property right is granted to or conferred upon you by disclosure or
** delivery of the Materials, either expressly, by implication, inducement,
** estoppel or otherwise 
** Some portion of the Materials may be copyrighted by Microsoft Corporation.
**
**  FILENAME:   xllp_ost.h
**
**  PURPOSE:    contains all XLLP OST specific macros, typedefs, and prototypes.
**
******************************************************************************/

#include "xllp_defs.h"
#include "xllp_intc.h"

//
// OST Register Definitions
//
typedef struct
{
    XLLP_VUINT32_T    osmr0;         	//OS timer match register 0
    XLLP_VUINT32_T    osmr1;         	//OS timer match register 1
    XLLP_VUINT32_T    osmr2;         	//OS timer match register 2
    XLLP_VUINT32_T    osmr3;          	//OS timer match register 3
    XLLP_VUINT32_T    oscr0;            //OS timer counter register 0(compatible)
    XLLP_VUINT32_T    ossr;             //OS timer status register
    XLLP_VUINT32_T    ower;          	//OS timer watchdog enable register
    XLLP_VUINT32_T    oier;           	//OS timer interrupt enable register
    XLLP_VUINT32_T    osnr;           	//OS timer snapshot register
    XLLP_VUINT32_T    reserved1[7];
    XLLP_VUINT32_T    oscr4;		//OS timer counter register 4
    XLLP_VUINT32_T    oscr5;		//OS timer counter register  5
    XLLP_VUINT32_T    oscr6;		//OS timer counter register  6
    XLLP_VUINT32_T    oscr7;		//OS timer counter register  7
    XLLP_VUINT32_T    oscr8;		//OS timer counter register  8
    XLLP_VUINT32_T    oscr9;		//OS timer counter register  9
    XLLP_VUINT32_T    oscr10;		//OS timer counter register  10
    XLLP_VUINT32_T    oscr11;		//OS timer counter register  11
    XLLP_VUINT32_T    reserved2[8];
    XLLP_VUINT32_T    osmr4;		//OS timer match register 4
    XLLP_VUINT32_T    osmr5;		//OS timer match register 5
    XLLP_VUINT32_T    osmr6;		//OS timer match register 6
    XLLP_VUINT32_T    osmr7;		//OS timer match register 7
    XLLP_VUINT32_T    osmr8;		//OS timer match register 8
    XLLP_VUINT32_T    osmr9;		//OS timer match register 9
    XLLP_VUINT32_T    osmr10;		//OS timer match register 10
    XLLP_VUINT32_T    osmr11;		//OS timer match register 11
    XLLP_VUINT32_T    reserved3[8];
    XLLP_VUINT32_T    omcr4;		//OS timer match control register 4
    XLLP_VUINT32_T    omcr5;		//OS timer match control register 5
    XLLP_VUINT32_T    omcr6;		//OS timer match control register 6
    XLLP_VUINT32_T    omcr7;		//OS timer match control register 7
    XLLP_VUINT32_T    omcr8;		//OS timer match control register 8
    XLLP_VUINT32_T    omcr9;		//OS timer match control register 9
    XLLP_VUINT32_T    omcr10;		//OS timer match control register 10
    XLLP_VUINT32_T    omcr11;		//OS timer match control register 11
} XLLP_OST_T, *P_XLLP_OST_T;


//
// Device Handle required for XLLP OST primitives
//
typedef struct 
{
    P_XLLP_OST_T 	pOSTRegs;
    P_XLLP_INTC_T  	pINTCRegs;
} XLLP_OST_HANDLE_T, *P_XLLP_OST_HANDLE_T;

//
// Enumeration for compatible OST match registers
//
typedef enum 
{
    MatchReg0 = 0,
    MatchReg1,
    MatchReg2,
    MatchReg3,
    MatchReg4,
    MatchReg5,
    MatchReg6,
    MatchReg7,
    MatchReg8,
    MatchReg9,
    MatchReg10,
    MatchReg11
}XLLP_OST_MATCHREG;

//
// OST Bit Definitions
//

//
// OST Tick constants
//
#define XLLP_OST_TICKS_MS    3250          // 1ms in ticks (3.25x10^6tick/sec * 1/1000sec/msec)
#define XLLP_OST_TICKS_US    3             // 1usec in ticks (3.25x10^6tick/sec * 1/1000000sec/usec)

//
// OSSR Bits
//
#define XLLP_OSSR_M0			(0x1 << 0)
#define XLLP_OSSR_M1			(0x1 << 1)
#define XLLP_OSSR_M2			(0x1 << 2)
#define XLLP_OSSR_M3			(0x1 << 3)
#define XLLP_OSSR_M4			(0x1 << 4)
#define XLLP_OSSR_M5			(0x1 << 5)
#define XLLP_OSSR_M6			(0x1 << 6)
#define XLLP_OSSR_M7			(0x1 << 7)
#define XLLP_OSSR_M8			(0x1 << 8)
#define XLLP_OSSR_M9			(0x1 << 9)
#define XLLP_OSSR_M10			(0x1 << 10)
#define XLLP_OSSR_M11			(0x1 << 11)

#define XLLP_OSSR_RESERVED_BITS (0xFFFFF000)

//
// OIER Bits
//
#define XLLP_OIER_E0			(0x1 << 0)
#define XLLP_OIER_E1			(0x1 << 1)
#define XLLP_OIER_E2			(0x1 << 2)
#define XLLP_OIER_E3			(0x1 << 3)
#define XLLP_OIER_E4			(0x1 << 4)
#define XLLP_OIER_E5			(0x1 << 5)
#define XLLP_OIER_E6			(0x1 << 6)
#define XLLP_OIER_E7			(0x1 << 7)
#define XLLP_OIER_E8			(0x1 << 8)
#define XLLP_OIER_E9			(0x1 << 9)
#define XLLP_OIER_E10			(0x1 << 10)
#define XLLP_OIER_E11			(0x1 << 11)

#define XLLP_OIER_RESERVED_BITS (0xFFFFF000)

//
// OMCR 4-11
//
#define XLLP_OMCR_CRES_REG			(0x1)
#define XLLP_OMCR_CRES_1MSEC		(0x2)
#define XLLP_OMCR_CRES_1USEC		(0x4)
#define XLLP_OMCR_CRES_1SEC			(0x3)

#define XLLP_OMCR_R					(0x1 << 3)
#define XLLP_OMCR_S					(0x3 << 4)
#define XLLP_OMCR_P					(0x1 << 6)
#define XLLP_OMCR_C					(0x1 << 7)

#define XLLP_OMCR_RESERVED_BITS		(0xFFFFFF00)

//
// XLLP OST Prototypes
//
void XllpOstConfigureTimer 
  (P_XLLP_OST_HANDLE_T pOSTHandle, XLLP_OST_MATCHREG matchreg, XLLP_UINT32_T matchvalue);

void XllpOstConfigureMatchReg
  (P_XLLP_OST_HANDLE_T pOSTHandle, XLLP_OST_MATCHREG matchreg, XLLP_UINT32_T matchincrement);

void XllpOstDelayMicroSeconds 
     (P_XLLP_OST_T pOstRegs, XLLP_UINT32_T microseconds);

void XllpOstDelayMilliSeconds 
     (P_XLLP_OST_T pOstRegs, XLLP_UINT32_T milliseconds);



#endif