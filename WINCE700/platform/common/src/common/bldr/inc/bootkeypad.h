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
#ifndef __BOOT_KEYPAD_H
#define __BOOT_KEYPAD_H

#include <bootDriver.h>
#include <bootFactory.h>

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#define BOOT_KEYPAD_IOCTL(i)        BOOT_IOCTL(BOOT_DRIVER_CLASS_KEYPAD, i)
#define BOOT_KEYPAD_IOCTL_OEM(i)    BOOT_IOCTL(BOOT_DRIVER_CLASS_KEYPAD, 0x8000 + i)

enum BootKeypadIoCtl_e {
    BOOT_KEYPAD_IOCTL_ISPRESSED = BOOT_KEYPAD_IOCTL(0x0001)
};

//------------------------------------------------------------------------------

#define BootKeypadDeinit    BootDriverDeinit
#define BootKeypadIoCtl     BootDriverIoCtl

//------------------------------------------------------------------------------
//  Define VK typically used on WM

#ifndef _WINUSERM_H_

#define VK_THOME                0x5C
#define VK_TSTART               0x5B
#define VK_TBACK                0x1B
#define VK_TSOFT1               0x70
#define VK_TSOFT2               0x71
#define VK_TUP                  0x26

#define VK_T1                   L'1'
#define VK_T2                   L'2'
#define VK_T3                   L'3'
#define VK_TTALK                0x72
#define VK_TRIGHT               0x27

#define VK_T4                   L'4'
#define VK_T5                   L'5'
#define VK_T6                   L'6'
#define VK_TEND                 0x73
#define VK_TLEFT                0x25

#define VK_T7                   L'7'
#define VK_T8                   L'8'
#define VK_T9                   L'9'
#define VK_TVOLUMEDOWN          0x76
#define VK_TDOWN                0x28

#define VK_TSTAR                0x77
#define VK_T0                   L'0'
#define VK_TPOUND               0x78
#define VK_TVOLUMEUP            0x75
#define VK_TACTION              0x0D

#define VK_TPOWER               0x81
#define VK_TFLIP                0x80
#define VK_TRECORD              0x79

#endif

//------------------------------------------------------------------------------

__inline
bool_t
BootKeypadIsPressed(
    handle_t hDriver,
    UINT16 vkey
    )
{
    return BootDriverIoCtl(
        hDriver, BOOT_KEYPAD_IOCTL_ISPRESSED, &vkey, sizeof(vkey)
        );
}

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // __BOOT_KEYPAD_H
