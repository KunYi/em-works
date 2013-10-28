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
#ifndef __BOOT_BLOCK_FAL_H
#define __BOOT_BLOCK_FAL_H

#include <bootBlock.h>

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

enum BootBlockIoFalCtl_e {
    BOOT_BLOCK_FAL_IOCTL_FLASH_INFO = BOOT_BLOCK_IOCTL(0x8001), // FlashInfoIoCtl
    BOOT_BLOCK_FAL_IOCTL_ERASE      = BOOT_BLOCK_IOCTL(0x8002), // EraseIoCtl
    BOOT_BLOCK_FAL_IOCTL_RAW_READ   = BOOT_BLOCK_IOCTL(0x8003)  // RawReadIoCtl
};

//------------------------------------------------------------------------------

handle_t
BootBlockFalInit(
    uint32_t phAddress,
    enum_t binaryRegions,
    size_t binaryRegionsSize[]
    );

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // __BOOT_BLOCK_FAL_H
