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
#pragma once
#include <bootBlock.hpp>

extern "C"
handle_t
BootBlockFalInit(
    uint32_t phAddress,
    enum_t binaryRegions,
    size_t binaryRegionsSize[]
    );

namespace ceboot {

//------------------------------------------------------------------------------

class BootBlockFal_t : public BootBlock_t {

protected:

    enum IoCtl_e {
        FlashInfoIoCtl      = BOOT_BLOCK_IOCTL(0x8001), // BOOT_BLOCK_FAL_IOCTL_FLASH_INFO
        EraseIoCtl          = BOOT_BLOCK_IOCTL(0x8002), // BOOT_BLOCK_FAL_IOCTL_ERASE
        RawReadIoCtl        = BOOT_BLOCK_IOCTL(0x8003)  // BOOT_BLOCK_FAL_IOCTL_RAW_READ
        };

    struct FlashInfoParams_t {
        size_t blockSize;
        size_t blocks;
        size_t badBlocks;
        size_t sectorsPerBlock;
        };

public:

    virtual
    bool_t
    __cdecl
    DeInit(
        ) = 0;

    virtual
    bool_t
    __cdecl
    IoCtl(
        enum_t code,
        void *pBuffer,
        size_t size
        ) = 0;

};
    
//------------------------------------------------------------------------------

}; // ceboot

