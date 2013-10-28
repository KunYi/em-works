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
#include <pcireg.h>
#include <bootpart.h>
#include <fmd.h>

namespace ceboot {

//------------------------------------------------------------------------------

class BootBlockFal_t : public BootBlock_t {

private:

    enum BlockFlags_e {
        BlockBad = (1 << 31),
        BlockReserved = (1 << 30),
        BlockEmpty = (1 << 29),
        BlockFull = (1 << 28),
        BlockSectors = (1 << 27)
        };
    
    struct Block_t {
        flags_t flags;
        size_t sectorLow;
        size_t sectorHigh;
        };

    enum RegionType_e {
        RegionXIP = 0,
        RegionRO,
        RegionRW,
        RegionBinary = 0x100,
        RegionReserved,
        RegionOther
        };

    struct Region_t {
        RegionType_e type;
        size_t block;
        size_t blocks;
        size_t sector;
        size_t sectors;
        };

    struct Binary_t {
        size_t sector;
        size_t sectors;
        };
    
    struct Reserved_t {
        char name[8];
        size_t sector;
        size_t sectors;
        };
    
    struct Partition_t {
        uint8_t fileSystem;
        enum_t region;
        size_t sector;
        size_t sectors;
        };
    
private:

    HANDLE          m_hFmd;

    enum_t         m_refCount;
    
    size_t          m_sectorSize;
    size_t          m_sectorsPerBlock;
    size_t          m_blockSize;
    size_t          m_blocks;
    Block_t*        m_aBlock;

    size_t          m_badBlocks;
    size_t          m_baseBlock;

    bool_t          m_mounted;
    
    enum_t         m_regions;
    Region_t*       m_aRegion;

    enum_t         m_binaries;
    Binary_t*       m_aBinary;
    size_t          m_binaryBlocks;
    
    enum_t         m_reserveds;
    Reserved_t*     m_aReserved;
    
    enum_t         m_partitions;
    Partition_t*    m_aPartition;

private:

    BootBlockFal_t(
        );

    ~BootBlockFal_t(
        );

    bool_t
    Init(
        uint32_t phAddress,
        enum_t binaryRegions,
        size_t binaryRegionsSize[]
        );

    bool_t
    Info(
        flags_t &flags,
        size_t  &sectorSize,
        size_t  &sectors,
        enum_t &binaryRegions,
        enum_t &reservedRegions,
        enum_t &partitions
        );

    bool_t
    Format(
        FormatInfo_t *pInfo
        );

    bool_t
    LockMode(
        enum_t mode
        );

    handle_t
    OpenBinaryRegion(
        enum_t index
        );

    handle_t
    OpenReservedRegion(
        LPCSTR name
        );

    handle_t
    OpenPartition(
        uchar fileSystem,
        enum_t index
        );

    bool_t
    Mount(
        );

    bool_t
    Dismount(
        );

    bool_t
    IsThisMbr(
        uint8_t *pSector
        );

    bool_t
    IsThisLayoutSector(
        uint8_t *pSector
        );

    enum_t
    FlashRegions(
        uint8_t *pSector
        );

    void
    FlashRegionInfo(
        uint8_t *pSector,
        enum_t index,
        enum_t &type,
        size_t &blocks,
        size_t &compactBlocks
        );

    enum_t
    Reserveds(
        uint8_t *pSector
        );

    void
    ReservedInfo(
        uint8_t *pSector,
        enum_t index,
        char *pName,
        size_t nameLength,
        size_t &sector,
        size_t &sectors
        );

    enum_t
    Partitions(
        uint8_t *pSector
        );

    void
    PartitionInfo(
        uint8_t *pSector,
        enum_t index,
        uint8_t &fileSystem,
        size_t &sector,
        size_t &sectors
        );

    bool_t
    AddPartitionByType(
        PARTENTRY *pTable,
        uchar fileSystem,
        FormatInfo_t *pInfo,
        size_t &sector,
        size_t &sectors
        );

    bool_t
    CreateMbrAndLayout(
        FormatInfo_t *pInfo,
        size_t mbrBlock,
        uint8_t *pMbrSector,
        uint8_t *pLayoutSector
        );
    
    bool_t
    ReadSector(
        size_t sector,
        uint8_t *pBuffer,
        SectorInfo *pInfo
        );

    bool_t
    WriteSector(
        size_t sector,
        uint8_t *pBuffer,
        SectorInfo *pInfo
        );

    bool_t
    WriteSectorMultistep(
        size_t sector,
        uint8_t *pData,
        uint32_t reserved1
        );

    bool_t
    UpdateOemByte(
        size_t sector,
        uint8_t oem
        );

    bool_t
    EraseBlock(
        size_t block
        );

    bool_t
    IsEmpty(
        SectorInfo &info
        );

    bool_t
    IsDirty(
        SectorInfo &info
        );

    bool_t
    EraseInBlock(
        size_t block,
        size_t sector,
        size_t sectors
        );

    size_t
    MapSectorDirect(
        Region_t *pRegion,
        size_t &sector,
        size_t &block,
        size_t &sectorInBlock
        );

    bool_t
    NextSectorDirect(
        Region_t *pRegion,
        size_t &sector,
        size_t sectors,
        size_t &block,
        size_t &sectorInBlock
        );

    bool_t
    ReadDirect(
        Region_t *pRegion,
        size_t sector,
        size_t sectors,
        void *pBuffer
        );

    bool_t
    ReadMapped(
        Region_t *pRegion,
        size_t sector,
        size_t sectors,
        void *pBuffer
        );

    bool_t
    WriteDirect(
        Region_t *pRegion,
        size_t sector,
        size_t sectors,
        void *pBuffer
        );

    bool_t
    EraseDirect(
        Region_t *pRegion,
        size_t sector,
        size_t sectors
        );


    bool_t
    WriteMapped(
        Region_t *pRegion,
        size_t sector,
        size_t sectors,
        void *pBuffer
        );

    bool_t
    EraseMapped(
        Region_t *pRegion,
        size_t sector,
        size_t sectors
        );

public:

    static 
    BootBlockFal_t*        
    BootStoreFalInit(
        uint32_t phAddress,
        enum_t binaryRegions,
        size_t binaryRegionsSize[]
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
        const char *name,
        size_t &sector,
        size_t &sectors
        );

    bool_t
    QueryReservedRegionByIndex(
        enum_t index,
        char *pName,
        size_t nameLength,
        size_t &sector,
        size_t &sectors
        );

    bool_t
    QueryPartition(
        uint8_t fileSystem,
        enum_t index,
        enum_t &region,
        size_t &sector,
        size_t &sectors
        );

    bool_t
    QueryPartitionByIndex(
        enum_t index,
        uint8_t &fileSystem,
        enum_t &region,
        size_t &sector,
        size_t &sectors
        );
    
    bool_t
    Read(
        enum_t region,
        size_t sector,
        size_t sectors,
        void *pBuffer
        );

    bool_t
    Write(
        enum_t region,
        size_t sector,
        size_t sectors,
        void *pBuffer
        );

    bool_t
    Erase(
        enum_t region,
        size_t sector,
        size_t sectors
        );

};

//------------------------------------------------------------------------------

}; // ceboot
