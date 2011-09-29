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
#ifndef __XLLP_INTC_H__
#define __XLLP_INTC_H__

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
**  FILENAME:   xllp_intc.h
**
**  PURPOSE:    contains the XLLP INTC typedefs and bit definitions.
**
******************************************************************************/

#include "xllp_defs.h"


//
// INTC Register definitions
//
typedef struct
{
    // First set of registers are for controlling for interrupts 0-31
  XLLP_VUINT32_T    icip;       // INTC IRQ Pending Register 
  XLLP_VUINT32_T    icmr;       // INTC Mask Register
  XLLP_VUINT32_T    iclr;       // INTC Level Register (IRQ vs FIQ)
  XLLP_VUINT32_T    icfp;       // INTC FIQ Pending Register
  XLLP_VUINT32_T    icpr;       // INTC Pending Register
  XLLP_VUINT32_T    iccr;       // INTC Control Register
  XLLP_VUINT32_T    ichp;       // INTC Highest Priority Register
  XLLP_VUINT32_T    ipr[32];    // Interrupt Priority Registers [0-31]
    // Registers controlling interrupt signals 32+
  XLLP_VUINT32_T    icip2;      // INTC IRQ Pending Register 
  XLLP_VUINT32_T    icmr2;      // INTC Mask Register
  XLLP_VUINT32_T    iclr2;      // INTC Level Register (IRQ vs FIQ)
  XLLP_VUINT32_T    icfp2;      // INTC FIQ Pending Register
  XLLP_VUINT32_T    icpr2;      // INTC Pending Registers 32+33 valid
    // Second array of IPR registers here  because addresses for [32+]
    //  are discontinuous from IPR [0..31] 
    // Space reserved in documents for 32-39 even though 34-39 not used now.
  XLLP_VUINT32_T    ipr2[2];    // Interrupt Priority Registers 32,33 valid
  XLLP_VUINT32_T    iprRsvd[6]; // Interrupt Priority Registers 34-39, futures
} XLLP_INTC_T, *P_XLLP_INTC_T;

//
// INTC Bit definitions for interrupt signals 0..31
//
#define XLLP_INTC_RTCALARM   (0x1u << 31)
#define XLLP_INTC_RTC_TIC    (0x1u << 30)
#define XLLP_INTC_OSMR3      (0x1u << 29)
#define XLLP_INTC_OSMR2      (0x1u << 28)
#define XLLP_INTC_OSMR1      (0x1u << 27)
#define XLLP_INTC_OSMR0      (0x1u << 26)
#define XLLP_INTC_DMAC       (0x1u << 25)
#define XLLP_INTC_SSP        (0x1u << 24)
#define XLLP_INTC_MMC        (0x1u << 23)
#define XLLP_INTC_FFUART     (0x1u << 22)
#define XLLP_INTC_BTUART     (0x1u << 21)
#define XLLP_INTC_STUART     (0x1u << 20)
#define XLLP_INTC_ICP        (0x1u << 19)
#define XLLP_INTC_I2C        (0x1u << 18)
#define XLLP_INTC_LCD        (0x1u << 17)
#define XLLP_INTC_SSP2       (0x1u << 16)
#define XLLP_INTC_USIM       (0x1u << 15)
#define XLLP_INTC_AC97       (0x1u << 14)
#define XLLP_INTC_I2S        (0x1u << 13)
#define XLLP_INTC_PMU        (0x1u << 12)
#define XLLP_INTC_USBCLIENT  (0x1u << 11)
#define XLLP_INTC_GPIOXX_2   (0x1u << 10)
#define XLLP_INTC_GPIO1      (0x1u << 9)
#define XLLP_INTC_GPIO0      (0x1u << 8)
#define XLLP_INTC_OSMRXX_4   (0x1u << 7)
#define XLLP_INTC_PWRI2C     (0x1u << 6)
#define XLLP_INTC_MEMSTICK   (0x1u << 5)
#define XLLP_INTC_KEYPAD     (0x1u << 4)
#define XLLP_INTC_USBOHCI    (0x1u << 3)
#define XLLP_INTC_USBNONOHCI (0x1u << 2)
#define XLLP_INTC_BASEBAND   (0x1u << 1)
#define XLLP_INTC_SSP3       (0x1u << 0)

//
// INTC Bit definitions for interrupt signals 32..33
//

#define XLLP_INTC_CAPTURE     (0x1u << 1) // Camera Capture interface
#define XLLP_INTC_CTM        (0x1u << 0) // Trusted Platform Module (Caddo)



// Reserved, valid bit masks

#define XLLP_INTC_ICIP_RESERVED_BITS 0x00000000u
#define XLLP_INTC_ICIP_MASK (~(XLLP_INTC_ICIP_RESERVED_BITS))
#define XLLP_INTC_ICMR_RESERVED_BITS (XLLP_INTC_ICIP_RESERVED_BITS)
#define XLLP_INTC_ICMR_MASK (~(XLLP_INTC_ICMR_RESERVED_BITS))
#define XLLP_INTC_ICLR_RESERVED_BITS (XLLP_INTC_ICIP_RESERVED_BITS)
#define XLLP_INTC_ICLR_MASK (~(XLLP_INTC_ICLR_RESERVED_BITS))
#define XLLP_INTC_ICFP_RESERVED_BITS (XLLP_INTC_ICIP_RESERVED_BITS)
#define XLLP_INTC_ICFP_MASK (~(XLLP_INTC_ICFP_RESERVED_BITS))
#define XLLP_INTC_ICPR_RESERVED_BITS (XLLP_INTC_ICIP_RESERVED_BITS)
#define XLLP_INTC_ICPR_MASK (~(XLLP_INTC_ICPR_RESERVED_BITS))
#define XLLP_INTC_ICCR_RESERVED_BITS 0xFFFFFFFEu
#define XLLP_INTC_ICCR_MASK (~(XLLP_INTC_ICCR_RESERVED_BITS))
#define XLLP_INTC_ICHP_RESERVED_BITS 0x7FC07FC0u
#define XLLP_INTC_ICHP_MASK (~(XLLP_INTC_ICHP_RESERVED_BITS))
#define XLLP_INTC_IPR_RESERVED_BITS 0x7FFFFFC0u
#define XLLP_INTC_IPR_MASK (~(XLLP_INTC_IPR_RESERVED_BITS))

#define XLLP_INTC_ICIP2_RESERVED_BITS 0xFFFFFFFCu
#define XLLP_INTC_ICIP2_MASK (~(XLLP_INTC_ICIP2_RESERVED_BITS))
#define XLLP_INTC_ICMR2_RESERVED_BITS (XLLP_INTC_ICIP2_RESERVED_BITS)
#define XLLP_INTC_ICMR2_MASK (~(XLLP_INTC_ICMR2_RESERVED_BITS))
#define XLLP_INTC_ICLR2_RESERVED_BITS (XLLP_INTC_ICIP2_RESERVED_BITS)
#define XLLP_INTC_ICLR2_MASK (~(XLLP_INTC_ICLR2_RESERVED_BITS))
#define XLLP_INTC_ICFP2_RESERVED_BITS (XLLP_INTC_ICIP2_RESERVED_BITS)
#define XLLP_INTC_ICFP2_MASK (~(XLLP_INTC_ICFP2_RESERVED_BITS))
#define XLLP_INTC_IPR2_RESERVED_BITS (XLLP_INTC_IPR_RESERVED_BITS)
#define XLLP_INTC_IPR2_MASK (~(XLLP_INTC_IPR2_RESERVED_BITS))

#endif
