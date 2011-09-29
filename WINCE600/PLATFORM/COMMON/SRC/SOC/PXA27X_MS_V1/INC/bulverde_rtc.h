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
//  Header: bulverde_rtc.h
//
//  Defines the Real Time Clock (RTC) register layout and associated 
//  types and constants.
//
#ifndef __BULVERDE_RTC_H__
#define __BULVERDE_RTC_H__

#if __cplusplus
extern "C" {
#endif

//////////////////////////////////////////////////////////////////////////////
//    Real-Time Clock (RTC) macros
//////////////////////////////////////////////////////////////////////////////

// Clear any current RTC alarm interrupt.
//
#define RT_ALARM_INT_CLR(pRTSR) (pRTSR &= (~(XLLP_RTSR_MASK | XLLP_RTSR_RESERVED_BITS) | XLLP_RTSR_AL))

// Clear any current RTC 1Hz edge detect interrupt (write to clear).
//
#define RT_HZ_INT_CLR(pRTSR)    (pRTSR &= (~(XLLP_RTSR_MASK | XLLP_RTSR_RESERVED_BITS) | XLLP_RTSR_HZ))

// Enable/Disable the RTC alarm interrupt.
//
#define RT_ALARM_INT_EN(pRTSR)  (pRTSR = (pRTSR & ~(XLLP_RTSR_MASK | XLLP_RTSR_RESERVED_BITS)) | XLLP_RTSR_ALE)
#define RT_ALARM_INT_DIS(pRTSR) (pRTSR &= (~(XLLP_RTSR_MASK | XLLP_RTSR_RESERVED_BITS | XLLP_RTSR_ALE)))

// Enable/Disable any current RTC 1Hz edge detect interrupt.
//
#define RT_HZ_INT_EN(pRTSR)     (pRTSR = (pRTSR & ~(XLLP_RTSR_MASK | XLLP_RTSR_RESERVED_BITS)) | XLLP_RTSR_HZE)
#define RT_HZ_INT_DIS(pRTSR)    (pRTSR &= (~(XLLP_RTSR_MASK | XLLP_RTSR_RESERVED_BITS | XLLP_RTSR_HZE)))


//------------------------------------------------------------------------------
//
//  Type: BULVERDE_RTC_REG    
//
//  RTC control registers.
//

#include <xllp_rtc.h>

typedef XLLP_RTC_T BULVERDE_RTC_REG;
typedef XLLP_RTC_T *PBULVERDE_RTC_REG;    

//------------------------------------------------------------------------------

#if __cplusplus
}
#endif

#endif 
