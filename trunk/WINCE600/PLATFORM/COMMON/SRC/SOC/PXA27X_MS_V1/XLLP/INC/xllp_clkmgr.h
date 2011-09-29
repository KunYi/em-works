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
#ifndef __XLLP_CLKMGR_H__
#define __XLLP_CLKMGR_H__

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
**  FILENAME:   xllp_clkmgr.h
**
**  PURPOSE:    contains all XLLP CLKMGR typedefs and bit definitions..
**              Valid for processor codenamed Bulverde, stepping B0
**
******************************************************************************/

#include "xllp_defs.h"

//
// Current to Bulverde EAS 3.0
//

//
// Clock Manager (CLKMGR) Register Bank
//
typedef struct
{
    XLLP_VUINT32_T    cccr;         	// Core Clock Configuration register
    XLLP_VUINT32_T    cken;         	// Clock Enable register
    XLLP_VUINT32_T    oscc;         	// Oscillator Configuration register
    XLLP_VUINT32_T    ccsr;         	// Core Clock Status register
 } XLLP_CLKMGR_T, *P_XLLP_CLKMGR_T;  

//
// Clock Enable Register (CLKEN) Bits
//
#define XLLP_CLKEN_PWM0_2	    (0x1u << 0)
#define XLLP_CLKEN_PWM1_3	    (0x1u << 1)
#define XLLP_CLKEN_AC97		    (0x1u << 2)
#define XLLP_CLKEN_SSP2		    (0x1u << 3)
#define XLLP_CLKEN_SSP3		    (0x1u << 4)
#define XLLP_CLKEN_STUART	    (0x1u << 5)
#define XLLP_CLKEN_FFUART	    (0x1u << 6)
#define XLLP_CLKEN_BTUART	    (0x1u << 7)
#define XLLP_CLKEN_I2S		    (0x1u << 8)
#define XLLP_CLKEN_OST		    (0x1u << 9)
#define XLLP_CLKEN_USBHOST	    (0x1u << 10)
#define XLLP_CLKEN_USBCLIENT    (0x1u << 11)
#define XLLP_CLKEN_MMC		    (0x1u << 12)
#define XLLP_CLKEN_ICP		    (0x1u << 13)
#define XLLP_CLKEN_I2C		    (0x1u << 14)
#define XLLP_CLKEN_PWRI2C	    (0x1u << 15)
#define XLLP_CLKEN_LCD		    (0x1u << 16)
#define XLLP_CLKEN_BASEBAND     (0x1u << 17)
#define XLLP_CLKEN_USIM		    (0x1u << 18)
#define XLLP_CLKEN_KEYPAD	    (0x1u << 19)
#define XLLP_CLKEN_MEMCLOCK     (0x1u << 20)
#define XLLP_CLKEN_MEMSTICK	    (0x1u << 21)
#define XLLP_CLKEN_MEMC		    (0x1u << 22)
#define XLLP_CLKEN_SSP1		    (0x1u << 23)
#define XLLP_CLKEN_CAMERA	    (0x1u << 24) // Camera Capture interface
#define XLLP_CLKEN_TPM		    (0x1u << 25) // Trusted Platform Module (Caddo)


//
// CLKEN register reserved and valid bits
//
#define XLLP_CLKEN_RESERVED_BITS  0xfc000000u
#define XLLP_CLKEN_MASK (~(XLLP_CLKEN_RESERVED_BITS))


//
// Oscillator Configuration Register (OSCC) Bits
//
#define XLLP_OSCC_OOK       (0x1u << 0)
#define XLLP_OSCC_OON       (0x1u << 1)
#define XLLP_OSCC_TOUT_EN   (0x1u << 2)
#define XLLP_OSCC_PIO_EN    (0x1u << 3)
#define XLLP_OSCC_CRI       (0x1u << 4)
#define XLLP_OSCC_OSD       (0x3u << 5)

//
// OSCC register reserved and valid bits
//
#define XLLP_OSCC_RESERVED_BITS  0xffffff80u
#define XLLP_OSCC_MASK (~(XLLP_OSCC_RESERVED_BITS))

//
// Core Clock Configuration Register (CCCR) Bits
//
#define XLLP_CCCR_L             (0x1fu << 0)
#define XLLP_CCCR_2N            (0x0fu << 7)
#define XLLP_CCCR_A             (0x1u << 25)
#define XLLP_CCCR_PLL_EARLY_EN  (0x1u << 26)
#define XLLP_CCCR_LCD_26        (0x1u << 27)
#define XLLP_CCCR_PPDIS         (0x1u << 30)
#define XLLP_CCCR_CPDIS         (0x1u << 31)

//
// CCCR register reserved and valid bits
//
#define XLLP_CCCR_RESERVED_BITS  0x31fff860u
#define XLLP_CCCR_MASK (~(XLLP_CCCR_RESERVED_BITS))

//
// Core Clock Status Register (CCSR) Bits
//
#define XLLP_CCSR_L_S       (0x1fu << 0)
#define XLLP_CCSR_2N_S      (0x0fu << 7)
#define XLLP_CCSR_PPLCK     (0x1u << 28)
#define XLLP_CCSR_CPLCK     (0x1u << 29)
#define XLLP_CCSR_PPDIS_S   (0x1u << 30)
#define XLLP_CCSR_CPDIS_S   (0x1u << 31)

//
// CCCR register reserved and valid bits
//
#define XLLP_CCSR_RESERVED_BITS  0x0ffff860u
#define XLLP_CCSR_MASK (~(XLLP_CCSR_RESERVED_BITS))

#endif