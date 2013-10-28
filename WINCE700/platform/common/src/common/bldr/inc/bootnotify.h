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
#ifndef __BOOT_NOTIFY_H
#define __BOOT_NOTIFY_H

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  Define:  BOOT_NOTIFY
//
//  This macro should be used to define notify codes from different
//  drivers. It uses same logic as used for IoCtl codes. Upper 16-bits
//  contains driver class and lower 16-bit notification code in driver
//  context. There isn't class with 0 id, so this codes are used for
//  notification from boot loader framework core.
//
#define BOOT_NOTIFY(c, i)           ((c << 16) | i)
//------------------------------------------------------------------------------
//
//  Define:  BOOT_NOTIFY_xxx
//

enum BootNotifyCode_e {
    BOOT_NOTIFY_INIT_FAILURE    = BOOT_NOTIFY(0, 0),
    BOOT_NOTIFY_POWERON         = BOOT_NOTIFY(0, 1),
    BOOT_NOTIFY_RUN_FAILURE     = BOOT_NOTIFY(0, 2),
    BOOT_NOTIFY_OEM             = BOOT_NOTIFY(0, 0x8000)
};    

//------------------------------------------------------------------------------
//
//  Function:  OEMBootNotify
//
//  This function is called to notify user about boot loader state and get
//  feedback if required (e.g. confirmation to delete user store).
//
void
OEMBootNotify(
    void *pContext,
    enum_t notifyCode,
    void *pNotifyInfo,
    size_t notifyInfoSize
    );

//------------------------------------------------------------------------------
//
//  Define:  MAX_OEM_UNSUPPORTED_NICS
//
//  This is used to specify the bounds of the g_dwOEMUnsupportedNICs array.
//  g_dwOEMUnsupportedNICs is an array of unit32_t containing the PCI 
//  DeviceId + VendorId of any unsupported NICs discovered during a PCI
//  bus scan.  The g_dwOEMUnsupportedNICCount variable must be incremented
//  once for every device found in the array.
//

#define MAX_OEM_UNSUPPORTED_NICS 8

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // __BOOT_NOTIFY_H
