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
//------------------------------------------------------------------------------
//
//  Header: bulverde_ost.h
//
//  Defines the OS timer register layout and associated types and constants.
//
#ifndef __BULVERDE_OST_H__
#define __BULVERDE_OST_H__

#if __cplusplus
extern "C" {
#endif

//////////////////////////////////////////////////////////////////////////////
//    OS timer macros
//////////////////////////////////////////////////////////////////////////////

// OSSR bits are all sticky (write one to clear)
// Optimize macro with assignment, instead of ANDing)
//
#define TIMER_M0_INT_CLR(pOSSR)  (pOSSR = XLLP_OSSR_M0)
#define TIMER_M1_INT_CLR(pOSSR)  (pOSSR = XLLP_OSSR_M1)
#define TIMER_M2_INT_CLR(pOSSR)  (pOSSR = XLLP_OSSR_M2)
#define TIMER_M3_INT_CLR(pOSSR)  (pOSSR = XLLP_OSSR_M3)
#define TIMER_M4_INT_CLR(pOSSR)  (pOSSR = XLLP_OSSR_M4)
#define TIMER_M5_INT_CLR(pOSSR)  (pOSSR = XLLP_OSSR_M5)
#define TIMER_M6_INT_CLR(pOSSR)  (pOSSR = XLLP_OSSR_M6)
#define TIMER_M7_INT_CLR(pOSSR)  (pOSSR = XLLP_OSSR_M7)
#define TIMER_M8_INT_CLR(pOSSR)  (pOSSR = XLLP_OSSR_M8)
#define TIMER_M9_INT_CLR(pOSSR)  (pOSSR = XLLP_OSSR_M9)
#define TIMER_M10_INT_CLR(pOSSR) (pOSSR = XLLP_OSSR_M10)
#define TIMER_M11_INT_CLR(pOSSR) (pOSSR = XLLP_OSSR_M11)

// OIER macros for Match Registers (0-7, 10-11)
// (8-9 are being reserved for possible xllp timer delay use)
//
#define TIMER_M0_INT_EN(pOIER)   (pOIER |= XLLP_OIER_E0)
#define TIMER_M0_INT_DIS(pOIER)  (pOIER &= ~(XLLP_OIER_RESERVED_BITS | XLLP_OIER_E0))

#define TIMER_M1_INT_EN(pOIER)   (pOIER |= XLLP_OIER_E1)
#define TIMER_M1_INT_DIS(pOIER)  (pOIER &= ~(XLLP_OIER_RESERVED_BITS | XLLP_OIER_E1))

#define TIMER_M2_INT_EN(pOIER)   (pOIER |= XLLP_OIER_E2)
#define TIMER_M2_INT_DIS(pOIER)  (pOIER &= ~(XLLP_OIER_RESERVED_BITS | XLLP_OIER_E2))

#define TIMER_M3_INT_EN(pOIER)   (pOIER |= XLLP_OIER_E3)
#define TIMER_M3_INT_DIS(pOIER)  (pOIER &= ~(XLLP_OIER_RESERVED_BITS | XLLP_OIER_E3))

#define TIMER_M4_INT_EN(pOIER)   (pOIER |= XLLP_OIER_E4)
#define TIMER_M4_INT_DIS(pOIER)  (pOIER &= ~(XLLP_OIER_RESERVED_BITS | XLLP_OIER_E4))

#define TIMER_M5_INT_EN(pOIER)   (pOIER |= XLLP_OIER_E5)
#define TIMER_M5_INT_DIS(pOIER)  (pOIER &= ~(XLLP_OIER_RESERVED_BITS | XLLP_OIER_E5))

#define TIMER_M6_INT_EN(pOIER)   (pOIER |= XLLP_OIER_E6)
#define TIMER_M6_INT_DIS(pOIER)  (pOIER &= ~(XLLP_OIER_RESERVED_BITS | XLLP_OIER_E6))

#define TIMER_M7_INT_EN(pOIER)   (pOIER |= XLLP_OIER_E7)
#define TIMER_M7_INT_DIS(pOIER)  (pOIER &= ~(XLLP_OIER_RESERVED_BITS | XLLP_OIER_E7))

#define TIMER_M10_INT_EN(pOIER)  (pOIER |= XLLP_OIER_E10)
#define TIMER_M10_INT_DIS(pOIER) (pOIER &= ~(XLLP_OIER_RESERVED_BITS | XLLP_OIER_E10))

#define TIMER_M11_INT_EN(pOIER)  (pOIER |= XLLP_OIER_E11)
#define TIMER_M11_INT_DIS(pOIER) (pOIER &= ~(XLLP_OIER_RESERVED_BITS | XLLP_OIER_E11))


//------------------------------------------------------------------------------
//
//  Type: BULVERDE_OST_REG    
//
//  OST control registers.
//

#include <xllp_ost.h>

typedef XLLP_OST_T BULVERDE_OST_REG;
typedef XLLP_OST_T *PBULVERDE_OST_REG;    

//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif 
