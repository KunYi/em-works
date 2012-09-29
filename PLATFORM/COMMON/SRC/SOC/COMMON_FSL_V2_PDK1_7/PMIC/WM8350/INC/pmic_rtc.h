//-----------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
///
/// Copyright (c) 2007 Wolfson Microelectronics plc.  All rights reserved.
///
/// This software as well as any related documentation may only be used or
/// copied in accordance with the terms of the Wolfson Microelectronic plc
/// agreement(s) (e.g. licence or non-disclosure agreement (NDA)).
///
/// The information in this file is furnished for informational use only,
/// is subject to change without notice, and should not be construed as a
/// commitment by Wolfson Microelectronics plc.  Wolfson Microelectronics plc
/// assumes no responsibility or liability for any errors or inaccuracies that
/// may appear in the software or any related documention.
///
/// Except as permitted by the agreement(s), no part of the software or any
/// related documention may be reproduced, stored in a retrieval system, or
/// transmitted in any form or by any means without the express written
/// consent of Wolfson Microelectronics plc.
///
/// @file   pmic_rtc.h
/// @brief  Public prototypes and types used for the PMIC RTC API.
///
/// This file contains the interface for controlling the RTC on the WM8350.
///
/// @version $Id: pmic_rtc.h 648 2007-06-15 22:30:24Z ib $
///
/// @Warning
///   This software is specifically written for Wolfson devices. It may not be
///   used with other devices.
///
//------------------------------------------------------------------------------

#ifndef __REGS_RTC_H__
#define __REGS_RTC_H__
#include "pmic_basic_types.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef void (FAR PASCAL *RTC_ALARM_CB)(void);
static BOOL bTODalarmTerminate;
// Function Prototypes
PMIC_STATUS pmicRTCSetSystemTime(SYSTEMTIME* lpSystemTime);
PMIC_STATUS pmicRTCGetSystemTime(SYSTEMTIME* lpSystemTime);
PMIC_STATUS pmicRTCSetAlarmTime(SYSTEMTIME* lpAlarmTime);
PMIC_STATUS pmicRTCGetAlarmTime(SYSTEMTIME* lpAlarmTime);
PMIC_STATUS pmicRTCRegisterAlarmCallback(RTC_ALARM_CB alarmCB);
PMIC_STATUS pmicRTCCancelAlarm();

#ifdef __cplusplus
}
#endif

#endif // __REGS_REGULATOR_H__
