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
#include <bootBlock.hpp>

namespace ceboot {

//------------------------------------------------------------------------------

class BootBlockBios_t : public BootBlock_t {

private:

    struct Reserved_t {
        char name[8];
        size_t sector;
        size_t sectors;
        };

    struct Partition_t {
        uint8_t fileSystem;
        bool_t active;
        size_t baseSector;
        size_t sector;
        size_t sectors;
        };

    enum_t          m_refCount;

    uint8_t         m_eddBiosMajor;
    uint8_t         m_eddBiosMinor;
    uint16_t        m_eddBiosIfcs;

    uint8_t         m_driveId;
    bool_t          m_useEdd;
    
    size_t          m_sectorSize;
    size_t          m_sectors;

    size_t          m_cylinders;
    size_t          m_headsPerCylinder;
    size_t          m_sectorsPerHead;

    bool_t          m_mounted;

    enum_t          m_partitions;
    Partition_t*    m_aPartition;

    enum_t          m_reserveds;
    Reserved_t*     m_aReserved;
    enum_t          m_reservedPartion;

private:

    BootBlockBios_t(
        );

    ~BootBlockBios_t(
        );

    bool_t
    Init(
        enum_t driveId
        );

    bool_t
    Mount(
        );

    void
    Dismount(
        );

    bool_t
    Info(
        flags_t &flags,
        size_t  &sectorSize,
        size_t  &sectors,
        enum_t  &binaryRegions,
        enum_t  &reservedRegions,
        enum_t  &partitions
        );

    bool_t
    Format(
        FormatInfo_t *pInfo
        );

    bool_t
    LockMode(
        enum_t mode
        );

    bool_t
    Lba2Chs(
        size_t sector,
        uint8_t &c,
        uint8_t &h,
        uint8_t &s
        );

    bool_t
    UpdateLayout(
        enum_t partitions,
        Partition_t *aPartitions
        );

public:

    static
    BootBlockBios_t*
    BootBlockBiosInit(
        enum_t diskId
        );

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

    enum_t
    AddRef(
        );

    enum_t
    Release(
        );

    bool_t
    QueryBinaryRegion(
        enum_t index,
        size_t &sector,
        size_t &sectors
        );

    bool_t
    QueryReservedRegion(
        cstring_t name,
        size_t &sector,
        size_t &sectors
        );

    bool_t
    QueryPartition(
        uint8_t fileSystem,
        enum_t index,
        size_t &sector,
        size_t &sectors
        );

    handle_t
    OpenBinaryRegion(
        enum_t index
        );

    handle_t
    OpenReservedRegion(
        cstring_t name
        );

    handle_t
    OpenPartition(
        uint8_t fileSystem,
        enum_t fileSystemIndex
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
        size_t sector,
        size_t sectors
        );

};

//------------------------------------------------------------------------------

}; // ceboot
