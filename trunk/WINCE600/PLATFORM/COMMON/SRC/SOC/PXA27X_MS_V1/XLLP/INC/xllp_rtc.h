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
#ifndef __XLLP_RTC_H__
#define __XLLP_RTC_H__

/******************************************************************************
**
**  COPYRIGHT (C) 2002 Intel Corporation.
**
**  This software as well as the software described in it is furnished under
**  license and may only be used or copied in accordance with the terms of the
**  license. The information in this file is furnished for informational use
**  only, is subject to change without notice, and should not be construed as
**  a commitment by Intel Corporation. Intel Corporation assumes no
**  responsibility or liability for any errors or inaccuracies that may appear
**  in this document or any software that may be provided in association with
**  this document. 
**  Except as permitted by such license, no part of this document may be 
**  reproduced, stored in a retrieval system, or transmitted in any form or by
**  any means without the express written consent of Intel Corporation. 
**
**  FILENAME:   xllp_rtc.h
**
**  PURPOSE:    contains all XLLP RTC specific macros, typedefs, and prototypes.
**
******************************************************************************/

#include "xllp_defs.h"
#include "xllp_intc.h"


//
// RTC Register Definitions
//
typedef struct
{
    XLLP_VUINT32_T    rcnr;     // RTC count register
    XLLP_VUINT32_T    rtar;     // RTC alarm register
    XLLP_VUINT32_T    rtsr;     // RTC status register
    XLLP_VUINT32_T    rttr;     // RTC timer trim register
    XLLP_VUINT32_T    rdcr;	    // RTC day counter register
    XLLP_VUINT32_T    rycr;	    // RTC year counter register
    XLLP_VUINT32_T    rdar1;    // RTC day alarm register 1
    XLLP_VUINT32_T    ryar1;    // RTC year alarm register 1
    XLLP_VUINT32_T    rdar2;    // RTC day alarm register 2
    XLLP_VUINT32_T    ryar2;    // RTC year alarm register 2
    XLLP_VUINT32_T    swcr;	    // RTC stopwatch counter
    XLLP_VUINT32_T    swar1;    // RTC stopwatch alarm register 1
    XLLP_VUINT32_T    swar2;    // RTC stopwatch alarm register 2
    XLLP_VUINT32_T    picr;	    // RTC periodic interrupt counter register
    XLLP_VUINT32_T    piar;	    // RTC periodic interrupt alarm register

} XLLP_RTC_T, *P_XLLP_RTC_T;


//
// Device Handle required for XLLP RTC primitives
//
typedef struct 
{
    P_XLLP_RTC_T 	pRTCRegs;
    P_XLLP_INTC_T 	pINTCRegs;
} XLLP_RTC_HANDLE_T, *P_XLLP_RTC_HANDLE_T;


//
// RTC Bit definitions
//
//
// RTSR Bits
//
#define XLLP_RTSR_AL     (0x1 << 0)   // RTC Alarm detect
#define XLLP_RTSR_HZ     (0x1 << 1)   // HZ rising edge detect
#define XLLP_RTSR_ALE    (0x1 << 2)   // RTC Alarm enable
#define XLLP_RTSR_HZE    (0x1 << 3)   // HZ enable
#define XLLP_RTSR_RDAL1  (0x1 << 4)   // Wristwatch alarm 1 detect
#define XLLP_RTSR_RDALE1 (0x1 << 5)   // Wristwatch alarm 1 enable
#define XLLP_RTSR_RDAL2  (0x1 << 6)   // Wristwatch alarm 2 detect
#define XLLP_RTSR_RDALE2 (0x1 << 7)   // Wristwatch alarm 2 enable
#define XLLP_RTSR_SWAL1  (0x1 << 8)   // Stopwatch alarm 1 detect
#define XLLP_RTSR_SWALE1 (0x1 << 9)   // Stopwatch alarm 1 enable
#define XLLP_RTSR_SWAL2  (0x1 << 10)  // Stopwatch alarm 2 detect
#define XLLP_RTSR_SWALE2 (0x1 << 11)  // Stopwatch alarm 2 enable
#define XLLP_RTSR_SWCE   (0x1 << 12)  // Stopwatch counter enable
#define XLLP_RTSR_PIAL   (0x1 << 13)  // Periodic alarm detect
#define XLLP_RTSR_PIALE  (0x1 << 14)  // Periodic alarm enable
#define XLLP_RTSR_PICE   (0x1 << 15)  // Periodic interrupt count enable

#define XLLP_RTSR_RESERVED_BITS (0xFFFF0000)

// 
// RTSR has mixed bits: r/w and sticky
// Mask for use with write to clear bits
//
#define XLLP_RTSR_MASK (XLLP_RTSR_AL | XLLP_RTSR_HZ | XLLP_RTSR_RDAL1 | XLLP_RTSR_RDAL2 | XLLP_RTSR_SWAL1 | XLLP_RTSR_SWAL2 | XLLP_RTSR_PIAL)

//
// RTC XLLP Prototypes
//
void XllpRtcConfigureAlarm
    (P_XLLP_RTC_HANDLE_T pRTCHandle, XLLP_UINT32_T alarmtime);

#endif