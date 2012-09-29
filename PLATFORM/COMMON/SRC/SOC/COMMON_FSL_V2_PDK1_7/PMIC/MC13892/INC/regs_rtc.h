//------------------------------------------------------------------------------
//
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header:  regs_rtc.h
//
//  This header file defines the Linear Regulator Register of MC13892 PMIC
//
//------------------------------------------------------------------------------

#ifndef __REGS_RTC_H__
#define __REGS_RTC_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// REGISTER BIT FIELD POSITIONS (LEFT SHIFT)
//------------------------------------------------------------------------------


#define MC13892_RTC_TM_RTCCAL_LSH                 17
#define MC13892_RTC_TM_RTCCALMODE_LSH             22

#define MC13892_RTC_TM_RTCCAL_WID                  5
#define MC13892_RTC_TM_RTCCALMODE_WID              0


#ifdef __cplusplus
}
#endif

#endif // __REGS_RTC_H__
