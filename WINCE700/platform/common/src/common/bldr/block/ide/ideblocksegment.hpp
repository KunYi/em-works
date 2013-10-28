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
#include "ideBlock.hpp"

namespace ceboot {

//------------------------------------------------------------------------------

class BootBlockSegmentIde_t : public BootBlockSegment_t {

private:
    
    enum IoCtl_e {
        DataSectorsIoCtl  = BOOT_BLOCK_IOCTL(0x8001) // BOOT_BLOCK_IOCTL_DATA_SECTORS
        };

    struct DataSectorsParams_t {
        size_t sector;
        size_t sectors;
        };

private:

    BootBlockIde_t*    m_pBlock;
    enum_t             m_sector;
    enum_t             m_sectors;


private:

    ~BootBlockSegmentIde_t(
        );

public:

    virtual
    bool_t
    __cdecl
    DeInit(
        );

    virtual
    bool_t
    __cdecl
    IoCtl(
        enum_t code,
        void *pBuffer,
        size_t size
        );

    BootBlockSegmentIde_t(
        BootBlockIde_t *pBlock
        );

    bool_t
    OpenBinaryRegion(
        enum_t index
        );

    bool_t
    OpenReservedRegion(
        const char *name
        );

    bool_t
    OpenPartition(
        uchar fileSystem,
        enum_t index
        );

    bool_t
    Read(
        size_t sector,
        size_t sectors,
        uint8_t *pBuffer
        );

    bool_t
    Write(
        size_t sector,
        size_t sectors,
        uint8_t *pBuffer
        );

    bool_t
    Erase(
        );

    bool_t
    DataSectors(
        size_t *pSector,
        size_t *pSectors
        );
};

//------------------------------------------------------------------------------

}; // ceboot
