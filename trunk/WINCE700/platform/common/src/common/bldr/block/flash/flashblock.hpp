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
#include <flashMddInterface.h>
#include <flashPddInterface.h>

namespace ceboot {

//------------------------------------------------------------------------------

static const DWORD BINARY_REGION_INDEX = 0xffff;
static const DWORD RESERVED_PARTITION_TYPE = 0x100;

//------------------------------------------------------------------------------

class BootBlockFlash_t : public BootBlock_t {

private:
    enum IoCtl_e {
        PartitionDataIoCtl  = BOOT_BLOCK_IOCTL(0x8001)
        };

    enum BootBlockLockMode_e {
        LockModeULDR       = 1,
        LockModeOS         = 2,
        LockModeHwMon      = 3
        };

    struct BinaryRegion_t {
        ULONG block;
        ULONG blocks;
        ULONG sectors;
        };

    struct PartitionDataParams_t {
        enum_t index;
        DWORD partitionCount;
        ULONG partitionType;    
        WCHAR *pPartitionName;  
        ULONGLONG startPhysicalBlock;
        ULONGLONG physicalBlockCount;   
        ULONGLONG logicalBlockCount;          
        ULONG partitionFlags;
    };

    enum_t                  m_refCount;
    FlashMddInterface*      m_pFal;

    size_t                  m_sectorSize;
    size_t                  m_sectorsPerBlock;
    size_t                  m_wearBlocksPerCent;
    size_t                  m_blockCount;

    size_t                  m_binaryRegions;
    BinaryRegion_t*         m_pBinaryList;

    size_t                  m_partitionCount;
    FLASH_PARTITION_INFO*   m_pPartitionTable;
    
private:

    BootBlockFlash_t(
        );

    ~BootBlockFlash_t(
        );

    bool_t
    Init(
        uint32_t context,
        enum_t binRegions,
        size_t binRegionSectors[]
        );

    bool_t
    GetPartitionTable();

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

    bool_t
    PartitionData(
        enum_t index,
        FLASH_PARTITION_INFO &info,
        DWORD &count
        );

    ULONG
    PartitionTypeToFlags(
        ULONG PartitionType
        );

    bool_t
    BinaryInfo(
        IN ULONG binaryCount,
        IN DWORD *pSectors
        );

    bool_t
    BinaryReadWrite(
        IN BOOL read,
        IN DWORD binaryIndex,
        IN DWORD sector,
        IN ULONG sectors,
        OUT UCHAR *pData
        );

    bool_t
    BinaryErase(
        IN DWORD binaryIndex
        );


public:

    static
    BootBlockFlash_t*
    BootBlockFlashInit(
        uint32_t context,
        enum_t binRegions,
        size_t binRegionSectors[]
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
        PARTITION_ID& partitionId
        );

    bool_t
    QueryBinaryRegionByIndex(
        enum_t index,
        size_t &sector,
        size_t &sectors
        );

    bool_t
    QueryReservedRegion(
        cstring_t name,
        PARTITION_ID& partitionId
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
        PARTITION_ID& partitionId
        );

    bool_t
    QueryPartitionByIndex(
        enum_t index,
        uint8_t &fileSystem,
        enum_t &region,
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
        FLASH_TRANSFER_REQUEST* pRequest    
        );

    bool_t
    Write(
        FLASH_TRANSFER_REQUEST* pRequest    
        );

    bool_t
    Erase(
        PARTITION_ID partitionId
        );
};

//------------------------------------------------------------------------------

}; // ceboot
