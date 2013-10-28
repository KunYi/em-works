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
#ifndef __BOOT_DOWNLOAD_NOTIFY_H
#define __BOOT_DOWNLOAD_NOTIFY_H

#include <bootNotify.h>

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#define BOOT_DOWNLOAD_NOTIFY(i)     BOOT_NOTIFY(BOOT_DRIVER_CLASS_DOWNLOAD, i)
#define BOOT_DOWNLOAD_NOTIFY_OEM(i) BOOT_NOTIFY(BOOT_DRIVER_CLASS_DOWNLOAD, 0x8000 + i)

enum BootNotifyDownload_e {
    BOOT_NOTIFY_DOWNLOAD_START = BOOT_DOWNLOAD_NOTIFY(0x0001),
    BOOT_NOTIFY_DOWNLOAD_CONTINUE = BOOT_DOWNLOAD_NOTIFY(0x0002),
    BOOT_NOTIFY_DOWNLOAD_DONE = BOOT_DOWNLOAD_NOTIFY(0x0003)
};

//------------------------------------------------------------------------------

typedef struct BootNotifyDownloadStart_t {
    enum_t imageType;
    size_t imageSize;
} BootNotifyDownloadStart_t;

typedef struct BootNotifyDownloadContinue_t {
    uint32_t shareDone;
} BootNotifyDownloadContinue_t;

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // __BOOT_DOWNLOAD_NOTIFY_H
