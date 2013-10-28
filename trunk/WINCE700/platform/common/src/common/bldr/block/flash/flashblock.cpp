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
#include <flashBlock.hpp>
#include <flashBlockSegment.hpp>
#include <bootLog.h>
#include <bootpart.h>

using namespace ceboot;

//------------------------------------------------------------------------------

extern "C"
handle_t
BootBlockFlashInit(
    uint32_t context,
    enum_t binRegions,
    size_t binRegionSectors[]
    )
{
    return BootBlockFlash_t::BootBlockFlashInit(
        context, binRegions, binRegionSectors
        );
}

//------------------------------------------------------------------------------

__forceinline
static
size_t
DivUp(
    size_t total,
    size_t item
    )
{
    return (total + item - 1) / item;
}

//------------------------------------------------------------------------------

BootBlockFlash_t*
BootBlockFlash_t::
BootBlockFlashInit(
    uint32_t context,
    enum_t binRegions,
    size_t binRegionSectors[]
    )
{
    BootBlockFlash_t* pDriver = NULL;

    pDriver = new BootBlockFlash_t();
    if ((pDriver != NULL) && !pDriver->Init(
            context, binRegions, binRegionSectors
            ))
        {
        delete pDriver;
        pDriver = NULL;
        }
    return pDriver;
}

//------------------------------------------------------------------------------

BootBlockFlash_t::
BootBlockFlash_t(
    ) : m_refCount(1)
{
}

//------------------------------------------------------------------------------

BootBlockFlash_t::
~BootBlockFlash_t(
    )
{
    delete[] m_pPartitionTable;
    delete m_pFal;
}

//------------------------------------------------------------------------------

enum_t
BootBlockFlash_t::
AddRef(
    )
{
    return m_refCount++;
}

//------------------------------------------------------------------------------

enum_t
BootBlockFlash_t::
Release(
    )
{
    if (--m_refCount == 0) delete this;
    return m_refCount;
}

//------------------------------------------------------------------------------

bool_t
BootBlockFlash_t::
Init(
    uint32_t context,
    enum_t binRegions,
    size_t binRegionSectors[]
    )
{
    bool rc = false;
    LRESULT code;
    FLASH_REGION_INFO regionInfo;

    m_pFal = FlashMddInterface::Create(context);

    if (m_pFal == NULL) goto cleanUp;

    code = m_pFal->GetRegionInfoTable(1, &regionInfo);
    if (code != ERROR_SUCCESS)
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootBlockFlash_t::Info: "
            L"FAL_GetRegionInfoTable failed with code %d!\r\n", code
            ));
        goto cleanUp;
        }

    m_sectorSize = regionInfo.DataBytesPerSector;
    m_sectorsPerBlock = regionInfo.SectorsPerBlock;
    m_wearBlocksPerCent = regionInfo.BadBlockHundredthPercent;
    m_blockCount = (size_t)regionInfo.BlockCount;
    m_pBinaryList = NULL;
    m_binaryRegions = 0;
    
    // Tell to flash that we expect some binary regions
    if (!BinaryInfo(binRegions, (DWORD*)binRegionSectors))
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootBlockFlash_t::Info: "
            L"BinaryInfo failed.\r\n"
            ));
        goto cleanUp;
        }

    m_pPartitionTable = NULL;
    m_partitionCount = 0;
    GetPartitionTable();
    
    // Done
    rc = true;
    
cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootBlockFlash_t::
DeInit(
    )
{
    return Release() == 0;
}

//------------------------------------------------------------------------------

bool_t
BootBlockFlash_t::
IoCtl(
    enum_t code,
    void *pBuffer,
    size_t size
    )
{
    bool rc = false;

    switch (code)
        {
        case InfoIoCtl:
            {
            InfoParams_t *pInfo;
            pInfo = static_cast<InfoParams_t*>(pBuffer);
            if ((pInfo == NULL) || (size != sizeof(*pInfo))) break;
            rc = Info(
                    pInfo->flags, pInfo->sectorSize, pInfo->sectors,
                    pInfo->binaryRegions, pInfo->reservedRegions,
                    pInfo->partitions
                    );
            }
            break;
        case FormatIoCtl:
            {
            FormatInfo_t *pInfo;
            pInfo = static_cast<FormatInfo_t*>(pBuffer);

            if ((pInfo == NULL) || (size != sizeof(*pInfo))) break;
            rc = Format(pInfo);
            }
            break;
        case LockModeIoCtl:
            {
            LockModeParams_t *pInfo;
            pInfo = static_cast<LockModeParams_t*>(pBuffer);
            if ((pInfo == NULL) || (size != sizeof(*pInfo))) break;
            rc = LockMode(pInfo->mode);
            }
            break;
        case OpenBinaryIoCtl:
            {
            OpenBinaryParams_t *pInfo;
            pInfo = static_cast<OpenBinaryParams_t*>(pBuffer);
            if ((pInfo == NULL) || (size != sizeof(*pInfo))) break;
            pInfo->hSection = OpenBinaryRegion(pInfo->index);
            rc = true;
            }
            break;
        case OpenReservedIoCtl:
            {
            OpenReservedParams_t *pInfo;
            pInfo = static_cast<OpenReservedParams_t*>(pBuffer);
            if ((pInfo == NULL) || (size != sizeof(*pInfo))) break;
            pInfo->hSection = OpenReservedRegion(pInfo->name);
            rc = true;
            }
            break;
        case OpenPartitionIoCtl:
            {
            OpenPartitionParams_t *pInfo;
            pInfo = static_cast<OpenPartitionParams_t*>(pBuffer);
            if ((pInfo == NULL) || (size != sizeof(*pInfo))) break;
            pInfo->hSection = OpenPartition(
                pInfo->fileSystem, pInfo->fileSystemIndex
                );
            rc = true;
            }
            break;
        case InfoBinaryIoCtl:
            {
            InfoBinaryParams_t *pInfo;
            size_t sector;
            pInfo = static_cast<InfoBinaryParams_t*>(pBuffer);
            if ((pInfo == NULL) || (size != sizeof(*pInfo))) break;
            rc = QueryBinaryRegionByIndex(pInfo->index, sector, pInfo->sectors);
            }
            break;
        case InfoReservedIoCtl:
            {
            InfoReservedParams_t *pInfo;
            size_t sector;
            pInfo = static_cast<InfoReservedParams_t*>(pBuffer);
            if ((pInfo == NULL) || (size != sizeof(*pInfo))) break;
            rc = QueryReservedRegionByIndex(pInfo->index, pInfo->name, sizeof(pInfo->name), sector, pInfo->sectors);
            }
            break;
        case InfoPartitionIoCtl:
            {
            InfoPartitionParams_t *pInfo;
            size_t sector;
            enum_t  region;
            pInfo = static_cast<InfoPartitionParams_t*>(pBuffer);
            if ((pInfo == NULL) || (size != sizeof(*pInfo))) break;
            rc = QueryPartitionByIndex(pInfo->index, pInfo->fileSystem, region, sector, pInfo->sectors);
            }
            break;    
        case PartitionDataIoCtl:
            {
            PartitionDataParams_t *pInfo;
            FLASH_PARTITION_INFO partitionInfo;
            DWORD count;
            pInfo = static_cast<PartitionDataParams_t*>(pBuffer);
            if ((pInfo == NULL) || (size != sizeof(*pInfo))) break;

            rc = PartitionData(pInfo->index, partitionInfo, count);
            if (rc)
                {
                pInfo->partitionCount = count;
                pInfo->partitionType = partitionInfo.PartitionType;
                pInfo->pPartitionName = partitionInfo.PartitionName;
                pInfo->startPhysicalBlock = partitionInfo.StartPhysicalBlock;
                pInfo->physicalBlockCount = partitionInfo.PhysicalBlockCount;
                pInfo->logicalBlockCount = partitionInfo.LogicalBlockCount;
                pInfo->partitionFlags = partitionInfo.PartitionFlags;
                }
            }
            break;
        }

    return rc;
}

bool_t
BootBlockFlash_t::
GetPartitionTable()
{
    bool_t rc = false;

    if (m_pPartitionTable)
    {
        delete[] m_pPartitionTable;
        m_pPartitionTable = NULL;
    }
    m_partitionCount = 0;

    DWORD count = 0;
    LRESULT code = m_pFal->GetPartitionCount(&count);
    if (code != ERROR_SUCCESS)
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootBlockFlash_t::GetPartitionTable: "
            L"FAL_GetPartitionCount failed with code %d!\r\n", code
            ));
        goto cleanUp;
        }

    if (count > 0)
        {
        m_pPartitionTable = new FLASH_PARTITION_INFO[count];
        if (m_pPartitionTable == NULL)
            {
            BOOTMSG(ZONE_ERROR, (L"ERROR: BootBlockFlash_t::Info: "
                L"Failed to allocate %d bytes!\r\n", count * sizeof(FLASH_PARTITION_INFO)
                ));
            goto cleanUp;
            }

        code = m_pFal->GetPartitionTable(count, m_pPartitionTable);
        if (code != ERROR_SUCCESS)
            {
            BOOTMSG(ZONE_ERROR, (L"ERROR: BootBlockFlash_t::Info: "
                L"FAL_GetPartitionTable failed with code %d!\r\n", code
                ));
            goto cleanUp;
            }
        }

    m_partitionCount = count;
    
    rc = true;
    
cleanUp:    
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootBlockFlash_t::
Info(
    flags_t &flags,
    size_t  &sectorSize,
    size_t  &sectors,
    enum_t &binaryRegions,
    enum_t &reservedRegions,
    enum_t &partitions
    )
{
    flags = SupportBinaryRegions | SupportReservedRegions;
    sectorSize = m_sectorSize;
    sectors = m_blockCount * m_sectorsPerBlock;
    binaryRegions = m_binaryRegions;
    reservedRegions = 0;
    partitions = 0;

    for (DWORD i = 0; i < m_partitionCount; i++)
        {
        if (m_pPartitionTable[i].PartitionFlags & FLASH_PARTITION_FLAG_RESERVED)
            {
            reservedRegions++;
            }
        else
            {
            partitions++;
            }
        }

    return true;
}


//------------------------------------------------------------------------------

ULONG 
BootBlockFlash_t::
PartitionTypeToFlags(
    ULONG PartitionType
    )
{
    DWORD PartitionFlags = 0;

    switch(PartitionType)
    {
        case PART_BOOTSECTION:
            __fallthrough;
        case PART_BINFS:
            __fallthrough;
        case PART_ROMIMAGE:
            __fallthrough;
        case PART_RAMIMAGE:
            PartitionFlags |= (FLASH_PARTITION_FLAG_DIRECT_MAP | FLASH_PARTITION_FLAG_READ_ONLY);
            break;
        case PART_IMGFS:
            PartitionFlags |= FLASH_PARTITION_FLAG_READ_ONLY;
            break;
        case RESERVED_PARTITION_TYPE:
            PartitionFlags |= (FLASH_PARTITION_FLAG_DIRECT_MAP | FLASH_PARTITION_FLAG_RESERVED);
            break;
        default:
            break;
    }

    return PartitionFlags;
}

//------------------------------------------------------------------------------

bool_t
BootBlockFlash_t::
Format(
    FormatInfo_t *pInfo
    )
{
    bool_t rc = false;
    LRESULT code;
    size_t size;


    // Check info consistency
    if (((pInfo->binaryRegions > 0)&&(pInfo->pBinaryRegionInfo == NULL)) ||
        ((pInfo->reservedRegions > 0)&&(pInfo->pReservedRegionInfo == NULL)) ||
        ((pInfo->partitions > 0)&&(pInfo->pPartitionInfo == NULL)))
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootBlockFlash_t::Format: "
            L"Invalid format info structure values!\r\n"
            ));
        goto cleanUp;
        }

    size = 0;

    for (enum_t ix = 0; ix < pInfo->binaryRegions; ix++)
        {
        BinaryRegionInfo_t*  pBinaryInfo = &pInfo->pBinaryRegionInfo[ix];
        size_t blocks = DivUp(pBinaryInfo->sectors, m_sectorsPerBlock);
        size += blocks;
        }
    
    for (enum_t ix = 0; ix < pInfo->reservedRegions; ix++)
        {
        ReservedRegionInfo_t* pRegionInfo = &pInfo->pReservedRegionInfo[ix];
        size_t blocks = DivUp(pRegionInfo->sectors, m_sectorsPerBlock);
        blocks += DivUp(blocks * m_wearBlocksPerCent, 10000);
        size += blocks;
        }

    for (enum_t ix = 0; ix < pInfo->partitions; ix++)
        {
        PartitionInfo_t *pPartitionInfo = &pInfo->pPartitionInfo[ix];
        size_t blocks;

        if (pPartitionInfo->sectors == END_OF_FLASH)
            {
            if (ix != (pInfo->partitions - 1))
                {
                BOOTMSG(ZONE_ERROR, (L"ERROR: BootBlockFlash_t::Format: "
                    L"Partition with END_OF_FLASH (-1) size must be last!\r\n"
                    ));
                goto cleanUp;
                }
                
            break;                
            }
        else
            {                        
            blocks = DivUp(pPartitionInfo->sectors, m_sectorsPerBlock);
            blocks += DivUp(blocks * m_wearBlocksPerCent, 10000);
            size += blocks;
   
            if (size > m_blockCount)
                {
                if (ix == (pInfo->partitions - 1))
                    {
                    BootLog(L"INFO: Partition 0x%02x won't fit into remaining flash. Setting its size to -1 (END_OF_FLASH).\r\n", 
                        pPartitionInfo->fileSystem
                        );            
                        
                    // occupy the remaining flash space                    
                    pPartitionInfo->sectors = END_OF_FLASH;   
                    break;
                    }
                else
                    {
                    BOOTMSG(ZONE_ERROR, (L"ERROR: BootBlockFlash_t::Format: "
                        L"Available Flash space too small for partitions! Required = %d.  Available = %d.\r\n",
                        size, m_blockCount
                        ));
                    goto cleanUp;          
                    }
                }
            }
        }

    // Start with binary regions
    if (pInfo->binaryRegions > 0)
        {
        size_t regions = pInfo->binaryRegions;
        m_binaryRegions = regions;
        rc = BinaryInfo(pInfo->binaryRegions, (DWORD*)pInfo->pBinaryRegionInfo);

        if (!rc)
            {
            BOOTMSG(ZONE_ERROR, (L"ERROR: BootBlockFlash_t::Format: "
                L"BinaryInfo failed!\r\n"
                ));
            goto cleanUp;
            }
        }
    
    // Now format store, this will destroy all data on storage
    code = m_pFal->FormatStore(0, TRUE);
    if (code != ERROR_SUCCESS)
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootBlockFlash_t::Format: "
            L"FAL_FormatStore Failed with code %d!\r\n", code
            ));
        goto cleanUp;
        }

    // Create all reserved regions    
    for (enum_t ix = 0 ; ix < pInfo->reservedRegions; ix++)
        {
        ReservedRegionInfo_t* pRegionInfo = &pInfo->pReservedRegionInfo[ix];
        FLASH_PARTITION_CREATE_INFO createInfo;
        FLASH_PARTITION_INFO info;

        memset(&createInfo, 0, sizeof(createInfo));
        createInfo.RegionIndex = 0;
        BootStringUtf8toUnicode(
            createInfo.PartitionName, _countof(createInfo.PartitionName), pRegionInfo->name
            );
        createInfo.PartitionType = 0x0100;
        createInfo.PartitionFlags = PartitionTypeToFlags(createInfo.PartitionType);
        
        createInfo.LogicalBlockCount = DivUp(
            pRegionInfo->sectors, m_sectorsPerBlock
            );

        code = m_pFal->CreatePartition(&createInfo, &info);
        if (code != ERROR_SUCCESS)
            {
            BOOTMSG(ZONE_ERROR, (L"ERROR: BootBlockFlash_t::Format: "
                L"FAL_CreatePartition Failed with code %d!\r\n", code
                ));
            }
        else
            {
            BootLog(
                L"Reserved region %s - %d sectors, %d blocks\r\n", 
                createInfo.PartitionName, 
                pRegionInfo->sectors, 
                createInfo.LogicalBlockCount
                );
            }
        }


    // Ordinary regions
    for (enum_t ix = 0; ix < pInfo->partitions; ix++)
        {
        PartitionInfo_t *pPartitionInfo = &pInfo->pPartitionInfo[ix];
        FLASH_PARTITION_CREATE_INFO createInfo;
        FLASH_PARTITION_INFO info;
        ULONGLONG blocks;

        if (ix == (pInfo->partitions - 1) && pPartitionInfo->sectors == END_OF_FLASH)
            {
            blocks = END_OF_FLASH;
            }
        else            
            {
            blocks = DivUp(pPartitionInfo->sectors, m_sectorsPerBlock);
            }

        memset(&createInfo, 0, sizeof(createInfo));
        createInfo.RegionIndex = 0;
        createInfo.PartitionType = pPartitionInfo->fileSystem;
        createInfo.PartitionFlags = PartitionTypeToFlags(pPartitionInfo->fileSystem);

        createInfo.LogicalBlockCount = blocks;
        BootLogSPrintf(
            createInfo.PartitionName, _countof(createInfo.PartitionName),
            L"PART%02X", ix
            );

        code = m_pFal->CreatePartition(&createInfo, &info);
        if (code != ERROR_SUCCESS)
            {
            BOOTMSG(ZONE_ERROR, (L"ERROR: BootBlockFlash_t::Format: "
                L"FAL_CreatePartition Failed with code %d!\r\n", code
                ));
            }
        else
            {
            if (pPartitionInfo->sectors == END_OF_FLASH)
                {
                BootLog(
                    L"Partition %02x - END_OF_FLASH\r\n", 
                    createInfo.PartitionType
                    );            
                }
            else
                {
                BootLog(
                    L"Partition %02x - %d sectors, %d blocks\r\n", 
                    createInfo.PartitionType, 
                    pPartitionInfo->sectors, 
                    createInfo.LogicalBlockCount
                    );
                }
            }
        }

    // Refresh the partition table
    GetPartitionTable();

    // Done
    rc = true;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootBlockFlash_t::
LockMode(
    enum_t mode
    )
{
    bool_t rc = false;
    LRESULT code;
    uint8_t fileSystem = 0;
    PARTITION_ID *pPartitionIdList = NULL;
    DWORD lockCount = 0;

    // Assumed order of partitions on Flash:
    // PART_BOOTSECTION (ULDR)
    // PART_RAMIMAGE (NK)
    // PART_IMGFS (IMAGE FILESYS)
    // PART_DOS3_FAT (T/EXFAT)
    switch (mode)
        {
        case LockModeOS: // lock up to start of TFAT  
            fileSystem = PART_DOS3_FAT;
            break;
        case LockModeULDR: // lock up to start of PART_RAMIMAGE (NK)
            fileSystem = PART_RAMIMAGE;
            break;
        case LockModeHwMon: // unlock the whole flash
            fileSystem = PART_BOOTSECTION;
            break;
        default:
            goto cleanUp;
        }

    // build the partition id list
    pPartitionIdList = new PARTITION_ID[m_partitionCount];
    if (pPartitionIdList == NULL)
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootBlockFlash_t::LockMode: "
            L"Failed to allocate %d bytes!\r\n", m_partitionCount * sizeof(PARTITION_ID)
            ));
        goto cleanUp;
        }

    while (m_pPartitionTable[lockCount].PartitionType != fileSystem && 
           lockCount < m_partitionCount)
        {
        pPartitionIdList[lockCount] = m_pPartitionTable[lockCount].PartitionId;
        lockCount++;
        }

    code = m_pFal->LockPartitions(pPartitionIdList, lockCount);
    if (code != ERROR_SUCCESS)
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootBlockFlash_t::LockMode: "
            L"FAL_LockPartitions Failed with code %d!\r\n", code
            ));
        }

    // Done
    rc = true;

cleanUp:
    if (pPartitionIdList) delete [] pPartitionIdList;
    return rc;
}

//------------------------------------------------------------------------------

handle_t
BootBlockFlash_t::
OpenBinaryRegion(
    enum_t index
    )
{
    BootBlockSegmentFlash_t *pSegment = NULL;

    pSegment = new BootBlockSegmentFlash_t(this);
    if ((pSegment != NULL) && !pSegment->OpenBinaryRegion(index))
        {
        pSegment->DeInit();
        pSegment = NULL;
        }
    return pSegment;
}

//------------------------------------------------------------------------------

handle_t
BootBlockFlash_t::
OpenReservedRegion(
    cstring_t name
    )
{
    BootBlockSegmentFlash_t *pSegment = NULL;

    pSegment = new BootBlockSegmentFlash_t(this);
    if ((pSegment != NULL) && !pSegment->OpenReservedRegion(name))
        {
        pSegment->DeInit();
        pSegment = NULL;
        }
    return pSegment;
}

//------------------------------------------------------------------------------

handle_t
BootBlockFlash_t::
OpenPartition(
    uchar fileSystem,
    enum_t index
    )
{
    BootBlockSegmentFlash_t *pSegment = NULL;

    pSegment = new BootBlockSegmentFlash_t(this);
    if ((pSegment != NULL) && !pSegment->OpenPartition(fileSystem, index))
        {
        pSegment->DeInit();
        pSegment = NULL;
        }
    return pSegment;
}

//------------------------------------------------------------------------------

bool_t
BootBlockFlash_t::
QueryBinaryRegion(
    enum_t index,
    PARTITION_ID& partitionId
    )
{
    bool_t rc = false;

    if (index >= m_binaryRegions) goto cleanUp;

    partitionId.RegionIndex = (WORD)-1;
    partitionId.PartitionIndex = (WORD)index;
    
    rc = true;
    
cleanUp:    
    return rc;
}


//------------------------------------------------------------------------------

bool_t
BootBlockFlash_t::
QueryBinaryRegionByIndex(
    enum_t index,
    size_t &sector,
    size_t &sectors
    )
{
    bool_t rc = false;

    if (index >= m_binaryRegions) goto cleanUp;
    sector = m_pBinaryList[index].block * m_sectorsPerBlock;
    sectors = m_pBinaryList[index].sectors; 
    rc = true;

cleanUp:        
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootBlockFlash_t::
QueryReservedRegion(
    cstring_t name,
    PARTITION_ID& partitionId
    )
{
    bool_t rc = false;
    wchar_t partitionName[16];
    
    BootStringUtf8toUnicode(partitionName, _countof(partitionName), name);
    for (enum_t ix = 0; ix < m_partitionCount; ix++)
        {
        if (m_pPartitionTable[ix].PartitionType != 0x0100) continue;                
        if (!BootStringEqual(partitionName, m_pPartitionTable[ix].PartitionName)) continue;
        partitionId = m_pPartitionTable[ix].PartitionId;
        rc = true;
        break;
        }

    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootBlockFlash_t::
QueryReservedRegionByIndex(
    enum_t index,
    char *pName,
    size_t nameLength,
    size_t &sector,
    size_t &sectors
    )
{
    if(index >= m_partitionCount ) return false;
    if(NULL == pName) return false;

    DWORD reservedIndex = 0;
    for (DWORD i = 0; i < m_partitionCount; i++)
        {
        if (m_pPartitionTable[i].PartitionFlags & FLASH_PARTITION_FLAG_RESERVED)
            {
            if (reservedIndex == index)
                {
                BootStringUnicodetoUtf8(pName, nameLength, m_pPartitionTable[i].PartitionName);
                sector = (DWORD)m_pPartitionTable[i].StartPhysicalBlock * m_sectorsPerBlock;
                sectors = (DWORD)m_pPartitionTable[i].PhysicalBlockCount * m_sectorsPerBlock;
                return true;                    
                }
            reservedIndex++;
            }
        }

    // The reserved index was not found.
    return false;
}

//------------------------------------------------------------------------------

bool_t
BootBlockFlash_t::
QueryPartition(
    uint8_t fileSystem,
    enum_t index,
    PARTITION_ID& partitionId
    )
{
    bool_t rc = false;
    for (enum_t ix = 0; ix < m_partitionCount; ix++)
        {
        if (m_pPartitionTable[ix].PartitionType != fileSystem) continue;
        if (index == 0)
            {
            partitionId = m_pPartitionTable[ix].PartitionId;
            rc = true;
            break;
            }
        index--;
        }

    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootBlockFlash_t::
QueryPartitionByIndex(
    enum_t index,
    uint8_t &fileSystem,
    enum_t &region,
    size_t &sector,
    size_t &sectors
    )
{
    if(index >= m_partitionCount ) return false;

    DWORD partitionIndex = 0;
    for (DWORD i = 0; i < m_partitionCount; i++)
        {
        if (!(m_pPartitionTable[i].PartitionFlags & FLASH_PARTITION_FLAG_RESERVED))
            {
            if (partitionIndex == index)
                {
                fileSystem = (uint8_t)m_pPartitionTable[i].PartitionType;
                region = m_pPartitionTable[i].PartitionId.RegionIndex;
                sector = (DWORD)m_pPartitionTable[i].StartPhysicalBlock * m_sectorsPerBlock;
                sectors = (DWORD)m_pPartitionTable[i].PhysicalBlockCount * m_sectorsPerBlock;
                return true;                    
                }
            partitionIndex++;
            }
        }

    // The partition index was not found.
    return false;
}

//------------------------------------------------------------------------------

bool_t
BootBlockFlash_t::
Read(
    FLASH_TRANSFER_REQUEST* pRequest    
    )
{
    bool rc = false;
    LRESULT code;

    if (pRequest->PartitionId.RegionIndex == BINARY_REGION_INDEX)
        {
        rc = BinaryReadWrite(
            TRUE,
            pRequest->PartitionId.PartitionIndex,
            (DWORD)pRequest->TransferList[0].SectorRun.StartSector,
            (ULONG)pRequest->TransferList[0].SectorRun.SectorCount,
            pRequest->TransferList[0].pBuffer
            );            
        }
    else
        {
        code = m_pFal->ReadLogicalSectors(pRequest);
        if (code != ERROR_SUCCESS)
            {
            BOOTMSG(ZONE_ERROR, (L"ERROR: BootBlockFlash_t::Read: "
                L"FAL_ReadLogicalSectors Failed on sectors (%d, %d) with code %d!\r\n", 
                (DWORD)pRequest->TransferList[0].SectorRun.StartSector,
                pRequest->TransferList[0].SectorRun.SectorCount,
                code
                ));
            goto cleanUp;
            }
        
        // Done
        rc = true;
        }
    

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootBlockFlash_t::
Write(
    FLASH_TRANSFER_REQUEST* pRequest    
    )
{
    bool rc = false;
    LRESULT code;

    if (pRequest->PartitionId.RegionIndex == BINARY_REGION_INDEX)
        {
        rc = BinaryReadWrite(
            FALSE,
            pRequest->PartitionId.PartitionIndex,
            (DWORD)pRequest->TransferList[0].SectorRun.StartSector,
            (ULONG)pRequest->TransferList[0].SectorRun.SectorCount,
            pRequest->TransferList[0].pBuffer
            );                                    
        }
    else
        {
        code = m_pFal->WriteLogicalSectors(pRequest);
        if (code != ERROR_SUCCESS)
            {
            BOOTMSG(ZONE_ERROR, (L"ERROR: BootBlockFlash_t::Write: "
                L"FAL_WriteLogicalSectors Failed with code %d!\r\n", code
                ));
            goto cleanUp;
            }
        // Done
        rc = true;
        }
    

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootBlockFlash_t::
Erase(
    PARTITION_ID partitionId
    )
{
    bool rc = false;
    LRESULT code;

    if (partitionId.RegionIndex == BINARY_REGION_INDEX)
        {
        rc = BinaryErase(partitionId.PartitionIndex);            
        }
    else
        {
        code = m_pFal->FormatPartition(partitionId);
        if (code != ERROR_SUCCESS)
            {
            BOOTMSG(ZONE_ERROR, (L"ERROR: BootBlockFlash_t::Erase: "
                L"FAL_FormatPartition Failed with code %d!\r\n", code
                ));
            goto cleanUp;
            }
        // Done
        rc = true;
        }

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

//
// Read partition table only once and return cached info afterwards
//
bool_t
BootBlockFlash_t::
PartitionData(
    enum_t index,
    FLASH_PARTITION_INFO &info,
    DWORD &count
    )
{
    count = m_partitionCount;
    
    if (index < count) 
        {
        info = m_pPartitionTable[index];
        }

    return true;
}

//------------------------------------------------------------------------------

bool_t
BootBlockFlash_t::
BinaryInfo(
    IN ULONG binaryCount,
    IN DWORD *pSectorList
    )
{
    bool_t rc = FALSE;
    FlashPddInterface* pFlashPdd = m_pFal->GetFlashPddInterface();
    
    // Just to be sure...
    delete [] m_pBinaryList;
    m_pBinaryList = NULL;
    m_binaryRegions = binaryCount;

    // If there are some binary regions...        
    if (binaryCount > 0)
        {
        ULONG block = 0;
       
        m_pBinaryList = new BinaryRegion_t[binaryCount];
        if (m_pBinaryList == NULL) goto cleanUp;
        
        for (ULONG ix = 0; ix < binaryCount; ix++)
            {
            ULONG blocks = DivUp(pSectorList[ix], m_sectorsPerBlock);
            
            m_pBinaryList[ix].block = block;
            m_pBinaryList[ix].blocks = blocks;
            m_pBinaryList[ix].sectors = pSectorList[ix];
            while (blocks > 0)
                {     
                BOOTMSG(ZONE_INFO, (L"BootBlockFlash_t::BinaryInfo: "
                    L"Checking reserved on block %d.\r\n", 
                    block           
                    ));
                    
                BLOCK_RUN blockRun = {0, block, 1};
                DWORD status = 0;
                LRESULT code = pFlashPdd->GetBlockStatus(blockRun, FALSE, &status);
                if (code != ERROR_SUCCESS) goto cleanUp;
                
                if (status & FLASH_BLOCK_STATUS_BAD) 
                    {
                    block++;
                    continue;
                    }
                if ((status & FLASH_BLOCK_STATUS_RESERVED) == 0)
                    {
                    BOOTMSG(ZONE_INFO, (L"BootBlockFlash_t::BinaryInfo: "
                        L"SetBlockStatus on block %d.\r\n", 
                        block           
                        ));

                    pFlashPdd->SetBlockStatus(blockRun, FLASH_BLOCK_STATUS_RESERVED);    
                    }
                block++;
                blocks--;
                }
            }
        }

    // Done
    rc = TRUE;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootBlockFlash_t::
BinaryReadWrite(
    IN BOOL read,
    IN DWORD binaryIndex,
    IN DWORD sector,
    IN ULONG sectors,
    OUT UCHAR *pData
    )
{
    bool_t rc = FALSE;
    BinaryRegion_t *pBinary;
    ULONG block;
    LRESULT code;
    FlashPddInterface* pFlashPdd = m_pFal->GetFlashPddInterface();

    // Check index
    if (binaryIndex >= m_binaryRegions) goto cleanUp;
    pBinary = &m_pBinaryList[binaryIndex];

    // Check size
    if ((sector >= pBinary->sectors) || 
        ((pBinary->sectors - sector) < sectors))
        {
        goto cleanUp;
        }

    block = pBinary->block;

    // Find the starting block to read/write to
    ULONG skipBlocks = sector / m_sectorsPerBlock;
    while (skipBlocks)
        {
        BLOCK_RUN blockRun = {0, block, 1};
        DWORD status = 0;
        code = pFlashPdd->GetBlockStatus(blockRun, FALSE, &status);
        if (code != ERROR_SUCCESS) goto cleanUp;

        block++;
        if ((status & FLASH_BLOCK_STATUS_BAD) != 0) continue;
        skipBlocks--;
        }

    // Read or write the sectors
    while (sectors > 0)
        {
        // Skip bad blocks
        BLOCK_RUN blockRun = {0, block, 1};
        DWORD status = 0;
        code = pFlashPdd->GetBlockStatus(blockRun, FALSE, &status);
        if (code != ERROR_SUCCESS) goto cleanUp;

        if ((status & FLASH_BLOCK_STATUS_BAD) != 0) 
            {
            block++;
            continue;
            }
        
        // Read or write sector
        ULONG sectorOffset = sector % m_sectorsPerBlock;
        ULONG sectorCount = min (sectors, m_sectorsPerBlock - sectorOffset);
        DWORD requestStatus = 0;
        FLASH_PDD_TRANSFER request = {0};
        request.SectorRun.StartSector = block * m_sectorsPerBlock + sectorOffset;
        request.SectorRun.SectorCount = sectorCount;
        request.pData = pData;

        if (read)
            {
            code = pFlashPdd->ReadPhysicalSectors(1, &request, &requestStatus);
            }
        else
            {
            code = pFlashPdd->WritePhysicalSectors(1, &request, &requestStatus);
            }           

        if (code != ERROR_SUCCESS)
            {
            BOOTMSG(ZONE_ERROR, (L"ERROR: BootBlockFlash_t::BinaryReadWrite: "
                L"%s failed on sector %d with code %d!\r\n", 
                read ? L"ReadPhysicalSectors" : L"WritePhysicalSectors",
                (DWORD)request.SectorRun.StartSector,
                code                
                ));
            goto cleanUp;
            }
       
        // Move to next block
        block++;
        pData += sectorCount * m_sectorSize;
        sector += sectorCount;
        sectors -= sectorCount;
        }

    // Done
    rc = TRUE;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootBlockFlash_t::
BinaryErase(
    IN DWORD binaryIndex
    )
{
    bool_t rc = FALSE;
    BinaryRegion_t *pBinary;
    ULONG block;
    ULONG blocks;
    FlashPddInterface* pFlashPdd = m_pFal->GetFlashPddInterface();

    // Check index
    if (binaryIndex >= m_binaryRegions) goto cleanUp;
    pBinary = &m_pBinaryList[binaryIndex];

    // Erase blocks
    block = pBinary->block;
    blocks = pBinary->blocks;
    while (blocks > 0)
        {
        BLOCK_RUN blockRun = {0, block, 1};
        DWORD status = 0;
        LRESULT code = pFlashPdd->GetBlockStatus(blockRun, FALSE, &status);
        if (code != ERROR_SUCCESS) goto cleanUp;

        if ((status & FLASH_BLOCK_STATUS_BAD) != 0) 
            {
            block++;
            continue;
            }

        code = pFlashPdd->EraseBlocks(1, &blockRun);
        if (code != ERROR_SUCCESS)
            {
            BOOTMSG(ZONE_ERROR, (L"ERROR: BootBlockFlash_t::BinaryErase: "
                L"Erase failed on block %d with code %d!\r\n", 
                block,
                code             
                ));
            goto cleanUp;
            }

        pFlashPdd->SetBlockStatus(blockRun, FLASH_BLOCK_STATUS_RESERVED);    

        block++;
        blocks--;
        }
    
    // Done
    rc = TRUE;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------


