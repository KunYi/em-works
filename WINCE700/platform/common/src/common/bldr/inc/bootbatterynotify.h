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
#ifndef __BOOT_BATTERY_NOTIFY_H
#define __BOOT_BATTERY_NOTIFY_H

#include <bootNotify.h>

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#define BOOT_BATTERY_NOTIFY(i)      BOOT_NOTIFY(BOOT_DRIVER_CLASS_BATTERY, i)
#define BOOT_BATTERY_NOTIFY_OEM(i)  BOOT_BATTERY_NOTIFY(0x8000 + i)

enum BootNotifyBattery_e {
    BOOT_NOTIFY_BATTERY_CHARGE_START = BOOT_BATTERY_NOTIFY(0x0001),
    BOOT_NOTIFY_BATTERY_CHARGE_CONTINUE = BOOT_BATTERY_NOTIFY(0x0002),
    BOOT_NOTIFY_BATTERY_CHARGE_STOP = BOOT_BATTERY_NOTIFY(0x0003)
};

//------------------------------------------------------------------------------

typedef struct BootNotifyBatteryChangeStart_t {
    enum_t source;
    int current;
} BootNotifyBatteryChangeStart_t;

typedef struct BootNotifyBatteryChangeContinue_t {
    bool_t charge;
    uint32_t capacity;
} BootNotifyBatteryChangeContinue_t;

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // __BOOT_BATTERY_NOTIFY_H
