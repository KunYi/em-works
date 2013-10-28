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
#include <bootDriver.hpp>
#include <bootDriverClasses.h>
#include <bootString.h>

namespace ceboot {

//------------------------------------------------------------------------------

#define BOOT_BLOCK_IOCTL(i)             BOOT_IOCTL(BOOT_DRIVER_CLASS_BLOCK, i)

//------------------------------------------------------------------------------

class BootBlockSegment_t;

class BootBlock_t : public BootDriver_t {

protected:

    enum IoCtl_e {
        InfoIoCtl           = BOOT_BLOCK_IOCTL(0x0001), // BOOT_BLOCK_IOCTL_INFO
        FormatIoCtl         = BOOT_BLOCK_IOCTL(0x0002), // BOOT_BLOCK_IOCTL_FORMAT
        LockModeIoCtl       = BOOT_BLOCK_IOCTL(0x0003), // BOOT_BLOCK_IOCTL_LOCK_MODE
        InfoBinaryIoCtl     = BOOT_BLOCK_IOCTL(0x0004), // BOOT_BLOCK_IOCTL_INFO_BINARY
        InfoReservedIoCtl   = BOOT_BLOCK_IOCTL(0x0005), // BOOT_BLOCK_IOCTL_INFO_RESERVED
        InfoPartitionIoCtl  = BOOT_BLOCK_IOCTL(0x0006), // BOOT_BLOCK_IOCTL_INFO_PARTITION
        OpenBinaryIoCtl     = BOOT_BLOCK_IOCTL(0x0101), // BOOT_BLOCK_IOCTL_OPEN_BINARY
        OpenReservedIoCtl   = BOOT_BLOCK_IOCTL(0x0102), // BOOT_BLOCK_IOCTL_OPEN_RESERVED
        OpenPartitionIoCtl  = BOOT_BLOCK_IOCTL(0x0103)  // BOOT_BLOCK_IOCTL_OPEN_PARTITION
        };

    struct InfoParams_t {
        flags_t flags;
        size_t sectorSize;
        size_t sectors;
        enum_t binaryRegions;
        enum_t reservedRegions;
        enum_t partitions;
        };

    struct InfoBinaryParams_t {
        enum_t index;
        size_t sectors;
        };

    struct InfoReservedParams_t {
        enum_t index;
        char name[8];
        size_t sectors;
        };

    struct InfoPartitionParams_t {
        enum_t index;
        uchar fileSystem;
        enum_t fileSystemIndex;
        size_t sectors;
        };

    struct OpenBinaryParams_t {
        enum_t index;
        handle_t hSection;
        };

    struct OpenReservedParams_t {
        char name[8];
        handle_t hSection;
        };

    struct OpenPartitionParams_t {
        uchar fileSystem;
        enum_t fileSystemIndex;
        handle_t hSection;
        };

    struct LockModeParams_t {
        enum_t mode;
        };

public:

    enum InfoFlags_e {
        SupportBinaryRegions    = (1 << 0),
        SupportReservedRegions  = (1 << 1)
        };

    typedef struct BinaryRegionInfo_t {
        size_t sectors;
    } BinaryRegionInfo_t;

    typedef struct ReservedRegionInfo_t {
        char name[8];
        size_t sectors;
    } ReservedRegionInfo_t;

    typedef struct PartitionInfo_t {
        char fileSystem;
        size_t sectors;
    } PartitionInfo_t;

    enum FormatFlags_e {
        FormatBinaryRegions = (1 << 0)
    };

    typedef struct FormatInfo_t {
        flags_t flags;
        size_t binaryRegions;
        BinaryRegionInfo_t *pBinaryRegionInfo;
        size_t reservedRegions;
        ReservedRegionInfo_t *pReservedRegionInfo;
        size_t partitions;
        PartitionInfo_t *pPartitionInfo;
    } FormatInfo_t;

    enum LockMode_e {
        ULDR       = 1,
        OS         = 2,
        HWMON      = 3
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

    bool_t
    Info(
        flags_t &flags,
        size_t &sectorSize,
        size_t &sectors,
        enum_t &binaryRegions,
        enum_t &reservedRegions,
        enum_t &partitions
        ) {
        bool_t rc = false;
        struct InfoParams_t params;

        if (IoCtl(InfoIoCtl, &params, sizeof(params)))
            {
            flags = params.flags;
            sectorSize = params.sectorSize;
            sectors = params.sectors;
            binaryRegions = params.binaryRegions;
            reservedRegions = params.reservedRegions;
            partitions = params.partitions;
            rc = true;
            }
        return rc;
        };

    bool_t
    Format(
        FormatInfo_t *pInfo
        ) {
        return IoCtl(FormatIoCtl, pInfo, sizeof(*pInfo));
        };

    bool_t
    LockMode(
        enum_t mode
        ) {
        LockModeParams_t params;

        params.mode = mode;
        return IoCtl(LockModeIoCtl, &params, sizeof(params));
        };

    bool_t
    BinaryRegionInfo(
        enum_t index,
        size_t &sectors
        ) {
        bool_t rc = false;
        InfoBinaryParams_t params;

        params.index = index;
        if (IoCtl(InfoBinaryIoCtl, &params, sizeof(params)))
            {
            sectors = params.sectors;
            rc = true;
            }
        return rc;
        };

    bool_t
    ReservedRegionInfo(
        enum_t index,
        char name[8],
        size_t &sectors
        ) {
        bool_t rc = false;
        InfoReservedParams_t params;

        params.index = index;
        if (IoCtl(InfoReservedIoCtl, &params, sizeof(params)))
            {
            StringCbCopyA(name, sizeof(name), params.name);
            sectors = params.sectors;
            rc = true;
            }
        return rc;
        };

    bool_t
    PartitionInfo(
        enum_t index,
        uchar &fileSystem,
        enum_t &fileSystemIndex,
        size_t &sectors
        ) {
        bool_t rc = false;
        InfoPartitionParams_t params;

        params.index = index;
        if (IoCtl(InfoPartitionIoCtl, &params, sizeof(params)))
            {
            fileSystem = params.fileSystem;
            fileSystemIndex = params.fileSystemIndex;
            sectors = params.sectors;
            rc = true;
            }
        return rc;
        };

    BootBlockSegment_t*
    OpenBinaryRegion(
        enum_t index
        ) {
        BootBlockSegment_t* hSection = NULL;
        OpenBinaryParams_t params;

        params.index = index;
        if (IoCtl(OpenBinaryIoCtl, &params, sizeof(params)))
            {
            hSection = reinterpret_cast<BootBlockSegment_t*>(params.hSection);
            }
        return hSection;
        };

    BootBlockSegment_t*
    OpenReservedRegion(
        const char *name
        ) {
        BootBlockSegment_t* hSection = NULL;
        OpenReservedParams_t params;

        StringCbCopyA(params.name, sizeof(params.name), name);
        if (IoCtl(OpenReservedIoCtl, &params, sizeof(params)))
            {
            hSection = reinterpret_cast<BootBlockSegment_t*>(params.hSection);
            }
        return hSection;
        };

    BootBlockSegment_t*
    OpenPartition(
        uchar fileSystem,
        enum_t fileSystemIndex
        ) {
        BootBlockSegment_t* hSection = NULL;
        OpenPartitionParams_t params;

        params.fileSystem = fileSystem;
        params.fileSystemIndex = fileSystemIndex;
        if (IoCtl(OpenPartitionIoCtl, &params, sizeof(params)))
            {
            hSection = reinterpret_cast<BootBlockSegment_t*>(params.hSection);
            }
        return hSection;
        };

    size_t
    SectorSize(
        ) {
        size_t sectorSize = 0;
        struct InfoParams_t params;

        if (IoCtl(InfoIoCtl, &params, sizeof(params)))
            {
            sectorSize = params.sectorSize;
            }
        return sectorSize;
        };

};

//------------------------------------------------------------------------------

class BootBlockSegment_t : public BootDriver_t {

protected:

    enum IoCtl_e {
        ReadIoCtl           = BOOT_BLOCK_IOCTL(0x0201), // BOOT_BLOCK_IOCTL_READ
        WriteIoCtl          = BOOT_BLOCK_IOCTL(0x0202), // BOOT_BLOCK_IOCTL_WRITE
        EraseIoCtl          = BOOT_BLOCK_IOCTL(0x0203)  // BOOT_BLOCK_IOCTL_ERASE
        };

    struct ReadParams_t {
        size_t sector;
        size_t sectors;
        uint8_t *pBuffer;
        };

    struct WriteParams_t {
        size_t sector;
        size_t sectors;
        uint8_t *pBuffer;
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

    bool_t
    Read(
        size_t sector,
        size_t sectors,
        uint8_t *pBuffer
        ) {
        ReadParams_t params;

        params.sector = sector;
        params.sectors = sectors;
        params.pBuffer = pBuffer;
        return IoCtl(ReadIoCtl, &params, sizeof(params));
        };

    bool_t
    Write(
        size_t sector,
        size_t sectors,
        uint8_t *pBuffer
        ) {
        WriteParams_t params;

        params.sector = sector;
        params.sectors = sectors;
        params.pBuffer = pBuffer;
        return IoCtl(WriteIoCtl, &params, sizeof(params));
        };

    bool_t
    Erase(
        ) {
        return IoCtl(EraseIoCtl, NULL, 0);
        };

    bool_t
    Close(
        ) {
        return DeInit();
        };
};

//------------------------------------------------------------------------------

}; // ceboot
