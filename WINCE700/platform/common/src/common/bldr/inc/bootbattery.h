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
#ifndef __BOOT_BATTERY_H
#define __BOOT_BATTERY_H

#include <bootDriver.h>
#include <bootDriverClasses.h>

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#define BOOT_BATTERY_IOCTL(i)       BOOT_IOCTL(BOOT_DRIVER_CLASS_BATTERY, i)

enum BootBatteryIoCtl_e {
    BOOT_BATTERY_IOCTL_STATE_OK = BOOT_BATTERY_IOCTL(0x0001),
    BOOT_BATTERY_IOCTL_CHARGE_START = BOOT_BATTERY_IOCTL(0x0002),
    BOOT_BATTERY_IOCTL_CHARGE_CONTINUE = BOOT_BATTERY_IOCTL(0x0003),
    BOOT_BATTERY_IOCTL_CHARGE_STOP = BOOT_BATTERY_IOCTL(0x0004)
};

typedef struct BootBatteryChargeStartParams_t {
    enum_t source;
    uint32_t current;
} BootBatteryChargeStartParams_t;

//------------------------------------------------------------------------------

#define BootBatteryDeinit   BootDriverDeinit
#define BootBatteryIoCtl    BootDriverIoCtl

//------------------------------------------------------------------------------

__inline
bool_t
BootBatteryStateOk(
    handle_t hDriver
    )
{
    return BootDriverIoCtl(hDriver, BOOT_BATTERY_IOCTL_STATE_OK, NULL, 0);
}

//------------------------------------------------------------------------------

__inline
bool_t
BootBatteryChargeStart(
    handle_t hDriver,
    enum_t source,
    uint32_t current
    )
{
    BootBatteryChargeStartParams_t params;

    params.source = source;
    params.current = current;
    return BootDriverIoCtl(
        hDriver, BOOT_BATTERY_IOCTL_CHARGE_START, &params, sizeof(params)
        );
}

//------------------------------------------------------------------------------

__inline
bool_t
BootBatteryChargeContinue(
    handle_t hDriver
    )
{
    return BootDriverIoCtl(
        hDriver, BOOT_BATTERY_IOCTL_CHARGE_CONTINUE, NULL, 0
        );
}

//------------------------------------------------------------------------------

__inline
bool_t
BootBatteryChargeStop(
    handle_t hDriver
    )
{
    return BootDriverIoCtl(hDriver, BOOT_BATTERY_IOCTL_CHARGE_STOP, NULL, 0);
}

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // __BOOT_BATTERY_H
