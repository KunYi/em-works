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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//

//
// This module defines the platform power management customizations.
//

#ifndef _PM_PLATFORM_H_
#define _PM_PLATFORM_H_

#include <nkintr.h>

//--------------------------------------------------------------------------------------

//
// Power Controllable IP List (Block Power will be turn off, when all IPs in the power domain is turned off)
//
typedef enum
{
    PWR_IP_IROM = 0,
    PWR_IP_ETM,
    PWR_IP_SDMA0,        // Domain S
    PWR_IP_SDMA1,
    PWR_IP_SECURITY,
    PWR_IP_ROTATOR,    // Domain F
    PWR_IP_POST,
    PWR_IP_DISPCON,
    PWR_IP_2D,            // Domain P
    PWR_IP_TVENC,
    PWR_IP_TVSC,
    PWR_IP_JPEG,        // Domain I
    PWR_IP_CAMIF,
    PWR_IP_MFC,        // Domain V
    PWR_IP_MAX
} PWR_IP_LIST;

//
// Power Control Driver I/O Control
//
#define FILE_DEVICE_PWRCON        (FILE_DEVICE_CONTROLLER)

#define IOCTL_PWRCON_SET_POWER_ON    \
    CTL_CODE(FILE_DEVICE_PWRCON, 0, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_PWRCON_SET_POWER_OFF    \
    CTL_CODE(FILE_DEVICE_PWRCON, 1, METHOD_BUFFERED, FILE_ANY_ACCESS)
    
#define IOCTL_PWRCON_SET_SYSTEM_LEVEL   \
    CTL_CODE(FILE_DEVICE_PWRCON, 2, METHOD_BUFFERED, FILE_ANY_ACCESS)    

#define IOCTL_PWRCON_QUERY_SYSTEM_LEVEL   \
    CTL_CODE(FILE_DEVICE_PWRCON, 3, METHOD_BUFFERED, FILE_ANY_ACCESS)    

#define IOCTL_PWRCON_PROFILE_DVS   \
    CTL_CODE(FILE_DEVICE_PWRCON, 4, METHOD_BUFFERED, FILE_ANY_ACCESS)    

//--------------------------------------------------------------------------------------

//
// S3C6410 Wakeup Source Definition
//
typedef enum
{
    OEMWAKE_EINT0 = SYSWAKE_OEMBASE, // in "nkintr.h"
    OEMWAKE_EINT1,
    OEMWAKE_EINT2,
    OEMWAKE_EINT3,
    OEMWAKE_EINT4,
    OEMWAKE_EINT5,
    OEMWAKE_EINT6,
    OEMWAKE_EINT7,
    OEMWAKE_EINT8,
    OEMWAKE_EINT9,
    OEMWAKE_EINT10,
    OEMWAKE_EINT11,
    OEMWAKE_EINT12,
    OEMWAKE_EINT13,
    OEMWAKE_EINT14,
    OEMWAKE_EINT15,
    OEMWAKE_EINT16,
    OEMWAKE_EINT17,
    OEMWAKE_EINT18,
    OEMWAKE_EINT19,
    OEMWAKE_EINT20,
    OEMWAKE_EINT21,
    OEMWAKE_EINT22,
    OEMWAKE_EINT23,
    OEMWAKE_EINT24,
    OEMWAKE_EINT25,
    OEMWAKE_EINT26,
    OEMWAKE_EINT27,
    OEMWAKE_RTC_ALARM,
    OEMWAKE_RTC_TICK,
    OEMWAKE_KEYPAD,
    OEMWAKE_MSM,
    OEMWAKE_BATTERY_FAULT,
    OEMWAKE_WARM_RESET,
    OEMWAKE_HSI,
    OEMWAKE_TOUCH,    // Stop mode only
    OEMWAKE_MMC0,    // Stop mode only
    OEMWAKE_MMC1,    // Stop mode only
    OEMWAKE_MMC2        // Stop mode only
} OEMWAKE_SOURCE;

//--------------------------------------------------------------------------------------

//
// System Active Level for Dynamic Voltage and Frequency Scaling
//
typedef enum
{
    SYS_L0 = 0,    // Full Speed
    SYS_L1,
    SYS_L2,
    SYS_L3,
    SYS_L4,
    SYS_L5,
    SYS_LEVEL_MAX
} SYSTEM_ACTIVE_LEVEL;

//
// Control Function of Dynamic Voltage and Frequency Scaling
//
void InitializeDVS(void);
void SetCurrentIdleTime(DWORD dwIdleTime);
void UpdateDVS(void);
void ChangeDVSLevel(SYSTEM_ACTIVE_LEVEL NewLevel);
void ProfileDVSOnOff(BOOL bOnOff);        // only for testing
void ChangeClockDivider(SYSTEM_ACTIVE_LEVEL NewLevel);

//  For Reset Device
//
void OEMSWReset(void);

//--------------------------------------------------------------------------------------

//#define POWER_CAP_PARENT            0x00000001      // defined in pm.h
//
// These Flags define the meaning of POWER_CAPABILITIES.Flags field.
// They define the unit of measure and prefix reported in the Power field.
//
#define POWER_CAP_UNIT_WATTS        0x00000010
#define POWER_CAP_UNIT_AMPS        0x00000020

#define POWER_CAP_PREFIX_MILLI    0x00000040
#define POWER_CAP_PREFIX_MICRO    0x00000080
#define POWER_CAP_PREFIX_NAN0        0x00000100

//--------------------------------------------------------------------------------------

#endif    // _PM_PLATFORM_H_

