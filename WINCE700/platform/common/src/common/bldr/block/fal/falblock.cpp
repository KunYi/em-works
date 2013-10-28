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
#include <falBlock.hpp>
#include <falBlockSegment.hpp>
#include <bootLog.h>
#include <bootString.h>

using namespace ceboot;

//------------------------------------------------------------------------------

#define PARTITIONS_IN_MBR           4

#define FREE_SECTOR                 0xFFFF      // Sector is free (erased)
#define DIRTY_SECTOR                0x0001      // Sector is ready to erase
#define SECTOR_WRITE_IN_PROGRESS    0x0002      // Sector write is in progress
#define SECTOR_WRITE_COMPLETED      0x0004      // Sector write completed
#define SECTOR_WRITE                0x0006

//------------------------------------------------------------------------------

extern "C"
handle_t
BootBlockFalInit(
    uint32_t phAddress,
    enum_t binRegions,
    size_t binRegionsSize[]
    )
{
    return BootBlockFal_t::BootStoreFalInit(
        phAddress, binRegions, binRegionsSize
        );
}

//------------------------------------------------------------------------------

BootBlockFal_t*
BootBlockFal_t::
BootStoreFalInit(
    uint32_t phAddress,
    enum_t binaryRegions,
    size_t binaryRegionsSize[]
    )
{
    BootBlockFal_t* pDriver = NULL;

    pDriver = new BootBlockFal_t();
    if ((pDriver != NULL) &&
        !pDriver->Init(phAddress, binaryRegions, binaryRegionsSize))
        {
        delete pDriver;
        pDriver = NULL;
        }
    return pDriver;
}

//------------------------------------------------------------------------------

BootBlockFal_t::
BootBlockFal_t(
    )
{
    m_blocks = 0; m_aBlock = NULL; 
    m_regions = 0; m_aRegion = NULL;
    m_binaries = 0; m_aBinary = NULL;
    m_reserveds = 0; m_aReserved = NULL;
    m_partitions = 0; m_aPartition = NULL;
    m_hFmd = NULL;
    m_refCount = 1;
}

//------------------------------------------------------------------------------

BootBlockFal_t::
~BootBlockFal_t(
    )
{
    Dismount();
    if (m_hFmd != NULL) FMD_Deinit(m_hFmd);
    delete [] m_aBinary;
    delete [] m_aBlock;
}

//------------------------------------------------------------------------------

enum_t
BootBlockFal_t::
AddRef(
    )
{
    return m_refCount++;
}

//------------------------------------------------------------------------------

enum_t
BootBlockFal_t::
Release(
    )
{
    if (--m_refCount == 0) delete this;
    return m_refCount;
}

//------------------------------------------------------------------------------

bool_t
BootBlockFal_t::
Init(
    uint32_t phAddress,
    enum_t binRegions,
    size_t binRegionSectors[]
    )
{
    bool rc = false;
    PCI_REG_INFO regInfo;
    FlashInfo flashInfo;


    // Open FMD to access NAND
    regInfo.MemBase.Reg[0] = phAddress;
    m_hFmd = FMD_Init(NULL, &regInfo, NULL);
    if (m_hFmd == NULL)
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootBlockFal_t::Init: "
            L"FMD_Init call failed!\r\n"
            ));
        goto cleanUp;
        }

    // Get flash info
    if (!FMD_GetInfo(&flashInfo))
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootBlockFal_t::Init: "
            L"FMD_GetInfo call failed!\r\n"
            ));
        goto cleanUp;
        }

    // Save flash geometry info
    m_sectorSize = flashInfo.wDataBytesPerSector;
    m_sectorsPerBlock = flashInfo.wSectorsPerBlock;
    m_blockSize = m_sectorSize * m_sectorsPerBlock;
    m_blocks = flashInfo.dwNumBlocks;

    // Allocate bad block bitmap
    m_aBlock = new Block_t[m_blocks];
    if (m_aBlock == NULL)
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootBlockFal_t::Init: "
            L"Block info array allocation failed!\r\n"
            ));
        goto cleanUp;
        }

    // Detect bad blocks...
    m_badBlocks = 0;
    m_baseBlock = (size_t)-1;
    for (size_t block = 0; block < m_blocks; block++)
        {
        m_aBlock[block].flags = 0;
        m_aBlock[block].sectorLow = 0;
        m_aBlock[block].sectorHigh = 0;

        // Skip bad blocks
        DWORD status = FMD_GetBlockStatus(block);
        if ((status & BLOCK_STATUS_BAD) != 0)
            {
            m_aBlock[block].flags = (flags_t)BlockBad;
            m_badBlocks++;
            continue;
            }

        if ((status & BLOCK_STATUS_RESERVED) != 0)
            {
            m_aBlock[block].flags |= BlockReserved;
            continue;
            }

        // First non-bad & non-reserved block is base one...
        if (m_baseBlock == -1) m_baseBlock = block;
        }

    // Save and copy binary regions info
    m_binaries = binRegions;
    if (m_binaries > 0)
        {
        m_aBinary = new Binary_t[m_binaries];
        if (m_aBinary == NULL)
            {
            BOOTMSG(ZONE_ERROR, (L"ERROR: BootBlockFal_t::Init: "
                L"Failed allocate memory for binary regions info!\r\n"
                ));
            goto cleanUp;
            }
        }
    size_t sector = 0;
    for (enum_t binary = 0; binary < m_binaries; binary++)
        {
        size_t sectors = binRegionSectors[binary] + m_sectorsPerBlock - 1;
        sectors = (sectors / m_sectorsPerBlock) * m_sectorsPerBlock;
        m_aBinary[binary].sector = sector;
        m_aBinary[binary].sectors = sectors;
        sector += sectors;
        }
    m_binaryBlocks = sector / m_sectorsPerBlock;
    
    // Update base block to accomodate binary regions
    size_t binaryBlocks = m_binaryBlocks;
    for (size_t block = 0; (block < m_blocks) && (binaryBlocks > 0); block++)
        {
        if ((m_aBlock[block].flags & BlockBad) != 0) continue;
        if (--binaryBlocks == 0)
            {
            if (block > m_baseBlock) m_baseBlock = block;
            }
        }
    
    // Mount storage (find MBR/Layout & parse it...)
    rc = Mount();

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootBlockFal_t::
DeInit(
    )
{
    return Release() == 0;
}

//------------------------------------------------------------------------------

bool_t
BootBlockFal_t::
IoCtl(
    enum_t code,
    void *pBuffer,
    size_t size
    )
{
    bool rc = false;

    switch (code)
        {
        case InfoIoCtl: // BOOT_BLOCK_IOCTL_INFO
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
        case FormatIoCtl: // BOOT_BLOCK_IOCTL_FORMAT
            {
            FormatInfo_t *pInfo;
            pInfo = static_cast<FormatInfo_t*>(pBuffer);
            if ((pInfo == NULL) || (size != sizeof(*pInfo))) break;
            rc = Format(pInfo);
            }
            break;
        case LockModeIoCtl: // BOOT_BLOCK_IOCTL_LOCK_MODE
            {
            LockModeParams_t *pInfo;
            pInfo = static_cast<LockModeParams_t*>(pBuffer);
            if ((pInfo == NULL) || (size != sizeof(*pInfo))) break;
            rc = LockMode(pInfo->mode);
            }
            break;
        case OpenBinaryIoCtl: // BOOT_BLOCK_IOCTL_OPEN_BINARY
            {
            OpenBinaryParams_t *pInfo;
            pInfo = static_cast<OpenBinaryParams_t*>(pBuffer);
            if ((pInfo == NULL) || (size != sizeof(*pInfo))) break;
            pInfo->hSection = OpenBinaryRegion(pInfo->index);
            rc = true;
            }
            break;
        case OpenReservedIoCtl: // BOOT_BLOCK_IOCTL_OPEN_RESERVED
            {
            OpenReservedParams_t *pInfo;
            pInfo = static_cast<OpenReservedParams_t*>(pBuffer);
            if ((pInfo == NULL) || (size != sizeof(*pInfo))) break;
            pInfo->hSection = OpenReservedRegion(pInfo->name);
            rc = true;
            }
            break;
        case OpenPartitionIoCtl: // BOOT_BLOCK_IOCTL_OPEN_PARTITION
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
        case InfoBinaryIoCtl: // BOOT_BLOCK_IOCTL_INFO_BINARY
            {
            InfoBinaryParams_t *pInfo;
            size_t sector;
            pInfo = static_cast<InfoBinaryParams_t*>(pBuffer);
            if ((pInfo == NULL) || (size != sizeof(*pInfo))) break;
            rc = QueryBinaryRegion(pInfo->index, sector, pInfo->sectors);
            }
        case InfoReservedIoCtl: // BOOT_BLOCK_IOCTL_INFO_RESERVED
            {
            InfoReservedParams_t *pInfo;
            size_t sector;
            pInfo = static_cast<InfoReservedParams_t*>(pBuffer);
            if ((pInfo == NULL) || (size != sizeof(*pInfo))) break;
            rc = QueryReservedRegionByIndex(pInfo->index, pInfo->name, sizeof(pInfo->name), sector, pInfo->sectors);
            }
            break;
        case InfoPartitionIoCtl: // BOOT_BLOCK_IOCTL_INFO_PARTITION
            {
            InfoPartitionParams_t *pInfo;
            size_t sector;
            enum_t  region;
            pInfo = static_cast<InfoPartitionParams_t*>(pBuffer);
            if ((pInfo == NULL) || (size != sizeof(*pInfo))) break;
            rc = QueryPartitionByIndex(pInfo->index, pInfo->fileSystem, region, sector, pInfo->sectors);
            }
            break;
        }

    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootBlockFal_t::
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
    sectors = (m_blocks - m_badBlocks) * m_sectorsPerBlock;
    binaryRegions = m_binaries;
    reservedRegions = m_reserveds;
    partitions = m_partitions;
    return true;
}

//------------------------------------------------------------------------------

bool_t
BootBlockFal_t::
Format(
    FormatInfo_t *pInfo
    )
{
    bool rc = false;
    uint8_t *pMbr = NULL, *pLayout = NULL;
    bool formatBinary = ((pInfo->flags & FormatBinaryRegions) != 0);


    // Check info consistency
    if (((pInfo->binaryRegions > 0) && (pInfo->pBinaryRegionInfo == NULL)) ||
        ((pInfo->reservedRegions > 0) && (pInfo->pReservedRegionInfo == NULL)) ||
        ((pInfo->partitions > 0) && (pInfo->pPartitionInfo == NULL)))
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootBlockFal_t::Format: "
            L"Invalid format info structure values!\r\n"
            ));
        goto cleanUp;
        }

    // Allocate memory for MBR/Layout sectors...
    pMbr = new uint8_t[m_sectorSize];
    pLayout = new uint8_t[m_sectorSize];
    if ((pMbr == NULL) || (pLayout == NULL))
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootBlockFal_t::Format: "
            L"MBR or Layout sector buffer allocation failed!\r\n"
            ));
        goto cleanUp;
        }
    
    // Find how many blocks for binary regions are needed
    enum_t binaries = 0;
    Binary_t *aBinary = NULL;
    size_t binaryBlocks = m_binaryBlocks;
    if (formatBinary)
        {
        binaries = pInfo->binaryRegions;
        aBinary = NULL;
        if (binaries > 0)
            {
            aBinary = new Binary_t[m_binaries];
            if (aBinary == NULL)
                {
                BOOTMSG(ZONE_ERROR, (L"ERROR: BootBlockFal_t::Format: "
                    L"Failed allocate memory for binary regions info!\r\n"
                    ));
                goto cleanUp;
                }
            }
        size_t sector = 0;
        for (enum_t binary = 0; binary < m_binaries; binary++)
            {
            size_t sectors = pInfo->pBinaryRegionInfo[binary].sectors;
            sectors += m_sectorsPerBlock - 1;
            sectors = (sectors / m_sectorsPerBlock) * m_sectorsPerBlock;
            aBinary[binary].sector = sector;
            aBinary[binary].sectors = sectors;
            sector += sectors;
            }
        binaryBlocks = sector / m_sectorsPerBlock;
        // Check if we fit to media
        if (binaryBlocks >= (m_blocks - m_badBlocks))
            {
            BOOTMSG(ZONE_ERROR, (L"ERROR: BootBlockFal_t::Format: "
                L"Run out of media space when adding binary regions!\r\n"
                ));
            goto cleanUp;
            }
        }
    
    // Find MBR/layout block
    size_t mbrBlock = m_binaryBlocks;
    for (size_t region = 0; region < pInfo->reservedRegions; region++)
        {
        size_t sectors = pInfo->pReservedRegionInfo[region].sectors;
        mbrBlock += (sectors + m_sectorsPerBlock - 1) / m_sectorsPerBlock;
        }
    if (mbrBlock >= (m_blocks - m_badBlocks))
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootBlockFal_t::Format: "
            L"Run out of media space when adding reserved regions!\r\n"
            ));
        goto cleanUp;
        }
    
    // Create MBR & layout sectors
    if (!CreateMbrAndLayout(pInfo, mbrBlock, pMbr, pLayout))
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootBlockFal_t::Format: "
            L"Run out of media space when adding partitions!\r\n"
            ));
        goto cleanUp;
        }

    // At this moment we know we fit on media, so we can update...
    if (formatBinary)
        {
        delete [] m_aBinary; 
        m_aBinary = aBinary;
        m_binaries = binaries;
        m_binaryBlocks = binaryBlocks;
        }
    if (!Dismount()) goto cleanUp;
    
    // Erase blocks
    for (size_t block = 0, badBlocks = 0; block < m_blocks; block++)
        {
        // Skip bad blocks
        if ((m_aBlock[block].flags & BlockBad) != 0)
            {
            badBlocks++;
            continue;
            }
        // Skip binary regions blocks if we don't format them
        if (!formatBinary && ((block - badBlocks) < m_binaryBlocks))
            {
            if ((m_aBlock[block].flags & BlockReserved) == 0)
                {
                UpdateOemByte(block * m_sectorsPerBlock, 0);
                m_aBlock[block].flags |= BlockReserved;
                }
            continue;
            }
        // Erase block (add block to bad if it fails...)
        if (!EraseBlock(block))
            {
            badBlocks++;
            continue;
            }

        // Set reserved block flag for binary region blocks
        if ((block - badBlocks) < m_binaryBlocks)
            {
            UpdateOemByte(block * m_sectorsPerBlock, 0);
            m_aBlock[block].flags |= BlockReserved;
            FMD_SetBlockStatus(block, BLOCK_STATUS_RESERVED);
            }

        // Is this MBR/Layout block?
        if (mbrBlock != (block - badBlocks)) continue;

        // It is, so write MBR and Layout
        size_t sector = block * m_sectorsPerBlock;
        if (!WriteSectorMultistep(sector, pMbr, 0)) goto cleanUp;
        if (!WriteSectorMultistep(sector + 1, pLayout, 1)) goto cleanUp;
        m_aBlock[block].sectorLow = 0;
        m_aBlock[block].sectorHigh = 1;
        m_aBlock[block].flags &= ~BlockEmpty;
        m_aBlock[block].flags |= BlockSectors;
        }
    
    // Re-mount store after layout change
    if (!Mount()) goto cleanUp;

    // Done
    rc = true;

cleanUp:
    delete [] pMbr;
    delete [] pLayout;
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootBlockFal_t::
LockMode(
    enum_t mode
    )
{
    UNREFERENCED_PARAMETER(mode);
    return false;
}

//------------------------------------------------------------------------------

handle_t
BootBlockFal_t::
OpenBinaryRegion(
    enum_t index
    )
{
    BootBlockSegmentFal_t *pSegment = NULL;

    pSegment = new BootBlockSegmentFal_t(this);
    if ((pSegment != NULL) && !pSegment->OpenBinaryRegion(index))
        {
        pSegment->DeInit();
        pSegment = NULL;
        }
    return pSegment;
}

//------------------------------------------------------------------------------

handle_t
BootBlockFal_t::
OpenReservedRegion(
    LPCSTR name
    )
{
    BootBlockSegmentFal_t *pSegment = NULL;

    pSegment = new BootBlockSegmentFal_t(this);
    if ((pSegment != NULL) && !pSegment->OpenReservedRegion(name))
        {
        pSegment->DeInit();
        pSegment = NULL;
        }
    return pSegment;
}

//------------------------------------------------------------------------------

handle_t
BootBlockFal_t::
OpenPartition(
    uchar fileSystem,
    enum_t index
    )
{
    BootBlockSegmentFal_t *pSegment = NULL;

    pSegment = new BootBlockSegmentFal_t(this);
    if ((pSegment != NULL) && !pSegment->OpenPartition(fileSystem, index))
        {
        pSegment->DeInit();
        pSegment = NULL;
        }
    return pSegment;
}

//------------------------------------------------------------------------------

bool_t
BootBlockFal_t::
QueryBinaryRegion(
    enum_t index,
    size_t &sector,
    size_t &sectors
    )
{
    bool_t rc = false;

    if (index >= m_binaries) goto cleanUp;
    sector = m_aBinary[index].sector;
    sectors = m_aBinary[index].sectors; 
    rc = true;

cleanUp:        
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootBlockFal_t::
QueryReservedRegion(
    const char *name,
    size_t &sector,
    size_t &sectors
    )
{
    bool_t rc = false;

    for (enum_t reserved = 0; reserved < m_reserveds; reserved++)
        {
        Reserved_t *pReserved = &m_aReserved[reserved];
        if (strcmp(name, pReserved->name) != 0) continue;
        sector = pReserved->sector;
        sectors = pReserved->sectors;
        rc = true;
        break;
        }
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootBlockFal_t::
QueryReservedRegionByIndex(
    enum_t index,
    char *pName,
    size_t nameLength,
    size_t &sector,
    size_t &sectors
    )
{
    if(index >= m_reserveds ) return false;
    if(NULL == pName) return false;

    Reserved_t *pReserved = &m_aReserved[index];
    BootStringCchCopyA(pName, nameLength, pReserved->name);
    sector = pReserved->sector;
    sectors = pReserved->sectors;
    return true;
}

//------------------------------------------------------------------------------

bool_t
BootBlockFal_t::
QueryPartition(
    uint8_t fileSystem,
    enum_t index,
    enum_t &region,
    size_t &sector,
    size_t &sectors
    )
{
    bool_t rc = false;

    for (enum_t partition = 0; partition < m_partitions; partition++)
        {
        Partition_t *pPartition = &m_aPartition[partition];
        if (pPartition->fileSystem != fileSystem) continue;
        if (index-- > 0) continue;
        region = pPartition->region;
        sector = pPartition->sector;
        sectors = pPartition->sectors;
        rc = true;
        break;
        }
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootBlockFal_t::
QueryPartitionByIndex(
    enum_t index,
    uint8_t &fileSystem,
    enum_t &region,
    size_t &sector,
    size_t &sectors
    )
{
    if(index >= m_partitions ) return false;
    Partition_t *pPartition = &m_aPartition[index];
    fileSystem = pPartition->fileSystem;
    region = pPartition->region;
    sector = pPartition->sector;
    sectors = pPartition->sectors;
    return true;
}

//------------------------------------------------------------------------------

bool_t
BootBlockFal_t::
Read(
    enum_t region,
    size_t sector,
    size_t sectors,
    void *pBuffer
    )
{
    bool rc = false;
    Region_t *pRegion;

    // Check parameters
    if (region >= m_regions) goto cleanUp;
    pRegion = &m_aRegion[region];
    if (sector > pRegion->sectors) goto cleanUp;
    if (sectors > (pRegion->sectors - sector)) goto cleanUp;

    switch (pRegion->type)
        {
        case RegionBinary:
        case RegionReserved:
        case RegionOther:
        case RegionXIP:
            rc = ReadDirect(pRegion, sector, sectors, pBuffer);
            break;
        case RegionRO:
        case RegionRW:
            rc = ReadMapped(pRegion, sector, sectors, pBuffer);
            break;
        }

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootBlockFal_t::
Write(
    enum_t region,
    size_t sector,
    size_t sectors,
    void *pBuffer
    )
{
    bool rc = false;
    Region_t *pRegion;

    // Check parameters
    if (region >= m_regions) goto cleanUp;
    pRegion = &m_aRegion[region];
    if (sector > pRegion->sectors) goto cleanUp;
    if (sectors > (pRegion->sectors - sector)) goto cleanUp;

    switch (pRegion->type)
        {
        case RegionBinary:
        case RegionReserved:
        case RegionOther:
        case RegionXIP:
            rc = EraseDirect(pRegion, sector, sectors);
            if (!rc) break;
            rc = WriteDirect(pRegion, sector, sectors, pBuffer);
            break;
        case RegionRO:
        case RegionRW:
            rc = EraseMapped(pRegion, sector, sectors);
            if (!rc) break;
            rc = WriteMapped(pRegion, sector, sectors, pBuffer);
            break;
        }

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootBlockFal_t::
Erase(
    enum_t region,
    size_t sector,
    size_t sectors
    )
{
    bool rc = false;
    Region_t *pRegion;

    // Check parameters
    if (region >= m_regions) goto cleanUp;
    pRegion = &m_aRegion[region];
    if (sector > pRegion->sectors) goto cleanUp;
    if (sectors > (pRegion->sectors - sector)) goto cleanUp;

    switch (pRegion->type)
        {
        case RegionBinary:
        case RegionReserved:
        case RegionOther:
        case RegionXIP:
            rc = EraseDirect(pRegion, sector, sectors);
            break;
        case RegionRO:
        case RegionRW:
            rc = EraseMapped(pRegion, sector, sectors);
            break;
        }

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootBlockFal_t::
Mount(
    )
{
    bool_t rc = false;
    uint8_t *pMbr = NULL, *pLayout = NULL;


    // Allocate MBR & Layout sector buffers
    pMbr = new uint8_t[m_sectorSize];
    pLayout = new uint8_t[m_sectorSize];
    if ((pMbr == NULL) || (pLayout == NULL))
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootBlockFal_t::Mount: "
            L"MBR or Layout sector buffer allocation failed!\r\n"
            ));
        goto cleanUp;
        }

    // Dismount
    Dismount();
    
    // Find MBR 
    size_t mbrBlock = (size_t)-1;
    for (
        size_t block = m_baseBlock, sector = m_baseBlock * m_sectorsPerBlock;
        block < m_blocks; block++, sector += m_sectorsPerBlock
            )
        {
        // Skip bad & reserved blocks
        if ((m_aBlock[block].flags & (BlockBad|BlockReserved)) != 0) continue;

        // Check if this block is MBR/Layout one
        if (!ReadSector(sector, pMbr, NULL))
            {
            m_aBlock[block].flags = (flags_t)BlockBad;
            continue;
            }
        if (!IsThisMbr(pMbr)) continue;
        if (!ReadSector(sector + 1, pLayout, NULL))
            {
            m_aBlock[block].flags = (flags_t)BlockBad;
            continue;
            }
        if (!IsThisLayoutSector(pLayout)) continue;

        // We find it...
        mbrBlock = block;
        break;
        }

    // Depending if we find MBR/Layout
    if (mbrBlock == -1)
        {
        // No MBR/Layout - there is only one region for binary
        m_regions = 1;
        m_aRegion = new Region_t[m_regions];
        if (m_aRegion == NULL)
            {
            BOOTMSG(ZONE_ERROR, (L"ERROR: BootBlockFal_t::Mount: "
                L"Regions info array allocation failed!\r\n"
                ));
            goto cleanUp;
            }
        m_aRegion[0].type = RegionBinary;
        m_aRegion[0].block = 0;
        m_aRegion[0].blocks = m_baseBlock;
        m_aRegion[0].sector = 0;
        m_aRegion[0].sectors = 0;
        for (size_t block = 0; block < m_baseBlock; block++)
            {
            if ((m_aBlock[block].flags & BlockBad) != 0) continue;
            m_aRegion[0].sectors += m_sectorsPerBlock;
            }
        }
    else
        {
        // Create partition info
        m_partitions = Partitions(pMbr);
        if (m_partitions > 0)
            {
            m_aPartition = new Partition_t[m_partitions];
            if (m_aPartition == NULL)
                {
                BOOTMSG(ZONE_ERROR, (L"ERROR: BootBlockFal_t::Mount: "
                    L"Partitions info array allocation failed!\r\n"
                    ));
                }
            }

        // Fill info based on MBR
        for (enum_t partition = 0; partition < m_partitions; partition++)
            {
            uint8_t fileSystem;
            size_t sector, sectors;
            PartitionInfo(pMbr, partition, fileSystem, sector, sectors);
            m_aPartition[partition].fileSystem = fileSystem;
            m_aPartition[partition].region = 2;
            m_aPartition[partition].sector = sector;
            m_aPartition[partition].sectors = sectors;
            }

        // Create region info
        m_regions = 2 + FlashRegions(pLayout);
        m_aRegion = new Region_t[m_regions];
        if (m_aRegion == NULL)
            {
            BOOTMSG(ZONE_ERROR, (L"ERROR: BootBlockFal_t::Mount: "
                L"Regions info array allocation failed!\r\n"
                ));
            goto cleanUp;
            }

        // First region contains binary regions
        m_aRegion[0].type = RegionBinary;
        m_aRegion[0].block = 0;
        m_aRegion[0].blocks = m_baseBlock;
        m_aRegion[0].sector = 0;
        m_aRegion[0].sectors = 0;
        for (size_t block = 0; block < m_baseBlock; block++)
            {
            if ((m_aBlock[block].flags & BlockBad) != 0) continue;
            m_aRegion[0].sectors += m_sectorsPerBlock;
            }
       
        // Second region before MBR/Layout block is reserved region
        m_aRegion[1].type = RegionReserved;
        m_aRegion[1].block = m_baseBlock;
        m_aRegion[1].blocks = mbrBlock - m_baseBlock;
        m_aRegion[1].sector = 0;
        m_aRegion[1].sectors = 0;
        for (size_t block = m_baseBlock; block < mbrBlock; block++)
            {
            if ((m_aBlock[block].flags & BlockBad) != 0) continue;
            m_aRegion[1].sectors += m_sectorsPerBlock;
            }

        // Create reserved regions info
        m_reserveds = Reserveds(pLayout);
        if (m_reserveds > 0)
            {
            m_aReserved = new Reserved_t[m_reserveds];
            if (m_aReserved == NULL)
                {
                BOOTMSG(ZONE_ERROR, (L"ERROR: BootBlockFal_t::Mount: "
                    L"Reserved regions info array allocation failed!\r\n"
                    ));
                goto cleanUp;
                }
            }
        for (enum_t reserved = 0; reserved < m_reserveds; reserved++)
            {
            Reserved_t *pReserved = &m_aReserved[reserved];
            
            ReservedInfo(
                pLayout, reserved, pReserved->name, sizeof(pReserved->name),
                pReserved->sector, pReserved->sectors
                );
            }
        
        // Create region map for media after MBR/Layout
        size_t sector = 0;
        block = mbrBlock;
        for (size_t region = 2; region < m_regions; region++)
            {
            enum_t type;
            size_t blocks, compactBlocks;

            // Get flash region information
            FlashRegionInfo(pLayout, region - 2, type, blocks, compactBlocks);

            // Save information
            switch (type)
                {
                case XIP:
                    m_aRegion[region].type = RegionXIP;
                    break;
                case READONLY_FILESYS:
                    m_aRegion[region].type = RegionRO;
                    break;
                case FILESYS:
                    m_aRegion[region].type = RegionRW;
                    break;
                default:
                    m_aRegion[region].type = RegionOther;
                    break;
                };
            m_aRegion[region].block = block;
            m_aRegion[region].blocks = 0;
            m_aRegion[region].sector = sector;

            // Fix partition info
            size_t sectors;
            if (blocks != -1)
                {
                sectors = blocks * m_sectorsPerBlock;
                sector += sectors;
                }
            else
                {
                sectors = (size_t)-1;
                }
            m_aRegion[region].sectors = sectors;

            for (enum_t partition = 0; partition < m_partitions; partition++)
                {
                if (m_aPartition[partition].region < region) continue;
                if (m_aPartition[partition].sector < sectors) continue;
                m_aPartition[partition].region++;
                m_aPartition[partition].sector -= sectors;
                }

            // Increase number of blocks by compact block
            if (blocks != -1) blocks += compactBlocks;

            // Fix for bad/reserved blocks
            while ((block < m_blocks) && (blocks > 0))
                {
                if ((m_aBlock[block].flags & (BlockBad|BlockReserved)) == 0)
                    {
                    m_aRegion[region].blocks++;
                    blocks--;
                    }
                block++;
                }
            }

        }

    // Done
    rc = m_mounted = true;

cleanUp:
    delete [] pMbr;
    delete [] pLayout;
    if (!rc) Dismount();
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootBlockFal_t::
Dismount(
    )
{
    // Delete previous information
    delete [] m_aRegion; m_aRegion = NULL; m_regions = 0;
    delete [] m_aReserved; m_aReserved = NULL; m_reserveds = 0;
    delete [] m_aPartition; m_aPartition = NULL; m_partitions = 0;
    m_mounted = false;
    
    return true;
}

//------------------------------------------------------------------------------

bool_t
BootBlockFal_t::
IsThisMbr(
    uint8_t *pSector
    )
{
    bool_t rc = false;

    if (*(uint16_t*)&pSector[BOOT_SIZE - 2] != BOOTSECTRAILSIGH) goto cleanUp;
    if ((pSector[0] != BS2BYTJMP) && (pSector[0] != BS3BYTJMP)) goto cleanUp;
    rc = true;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootBlockFal_t::
IsThisLayoutSector(
    uint8_t *pSector
    )
{
    bool_t rc = false;

    // Check for layout sector
    if (memcmp(pSector, FLASH_LAYOUT_SIG, FLASH_LAYOUT_SIG_SIZE) != 0)
        goto cleanUp;

    rc = true;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

enum_t
BootBlockFal_t::
FlashRegions(
    uint8_t *pSector
    )
{
    FlashLayoutSector *pLayout = reinterpret_cast<FlashLayoutSector *>(pSector);
    return pLayout->cbRegionEntries/sizeof(FlashRegion);
}

//------------------------------------------------------------------------------

void
BootBlockFal_t::
FlashRegionInfo(
    uint8_t *pSector,
    enum_t index,
    enum_t &type,
    size_t &blocks,
    size_t &compactBlocks
    )
{
    FlashLayoutSector *pLayout = reinterpret_cast<FlashLayoutSector *>(pSector);
    uint8_t *pInfo = reinterpret_cast<uint8_t*>(&pLayout[1]);
    FlashRegion *pRegion;

    pInfo += pLayout->cbReservedEntries;
    pRegion = reinterpret_cast<FlashRegion*>(pInfo);
    type = pRegion[index].regionType;
    blocks = pRegion[index].dwNumLogicalBlocks;
    compactBlocks = pRegion[index].dwCompactBlocks;
}

//------------------------------------------------------------------------------

enum_t
BootBlockFal_t::
Reserveds(
    uint8_t *pSector
    )
{
    FlashLayoutSector *pLayout = reinterpret_cast<FlashLayoutSector *>(pSector);
    return pLayout->cbReservedEntries/sizeof(ReservedEntry);
}

//------------------------------------------------------------------------------

void
BootBlockFal_t::
ReservedInfo(
    uint8_t *pSector,
    enum_t index,
    char *pName,
    size_t nameLength,
    size_t &sector,
    size_t &sectors
    )
{
    FlashLayoutSector *pLayout = reinterpret_cast<FlashLayoutSector *>(pSector);
    ReservedEntry *pEntry = reinterpret_cast<ReservedEntry *>(&pLayout[1]);

    memset(pName, 0, nameLength);
    BootStringCchCopyA(pName, nameLength, pEntry[index].szName);
    sector = pEntry[index].dwStartBlock * m_sectorsPerBlock;
    sectors = pEntry[index].dwNumBlocks * m_sectorsPerBlock;
}

//------------------------------------------------------------------------------

enum_t
BootBlockFal_t::
Partitions(
    uint8_t *pSector
    )
{
    enum_t partitions = 0;
    PARTENTRY table[PARTITIONS_IN_MBR];

    // Copy partition table to avoid alignment issue
    memcpy(table, &pSector[BOOT_SIZE - 2 - sizeof(table)], sizeof(table));
    for (enum_t ix = 0; ix < PARTITIONS_IN_MBR; ix++)
        {
        if (table[ix].Part_FileSystem != 0) partitions++;
        }

    return partitions;
}

//------------------------------------------------------------------------------

void
BootBlockFal_t::
PartitionInfo(
    uint8_t *pSector,
    enum_t index,
    uint8_t &fileSystem,
    size_t &sector,
    size_t &sectors
    )
{
   PARTENTRY table[PARTITIONS_IN_MBR];

    // Copy partition table to avoid alignment issue
    memcpy(table, &pSector[BOOT_SIZE - 2 - sizeof(table)], sizeof(table));
    for (enum_t ix = 0; ix < PARTITIONS_IN_MBR; ix++)
        {
        if (table[ix].Part_FileSystem != 0)
            {
            if (index == 0)
                {
                fileSystem = table[ix].Part_FileSystem;
                sector = table[ix].Part_StartSector;
                sectors = table[ix].Part_TotalSectors;
                break;
                }
            index--;
            }
        }
}

//------------------------------------------------------------------------------

bool_t
BootBlockFal_t::
AddPartitionByType(
    PARTENTRY *pTable,
    uchar fileSystem,
    FormatInfo_t *pInfo,
    size_t &sector,
    size_t &sectors
    )
{
    bool_t rc = false;

    // Look in format information
    for (enum_t ix1 = 0; ix1 < pInfo->partitions; ix1++)
        {
        PartitionInfo_t *pPartition = &pInfo->pPartitionInfo[ix1];
        if (pPartition->fileSystem != fileSystem) continue;
        size_t count = pPartition->sectors;
        if (count > sectors) count = sectors;
        for (enum_t ix2 = 0; ix2 < PARTITIONS_IN_MBR; ix2++)
            {
            PARTENTRY *pEntry = &pTable[ix2];
            if (pEntry->Part_FileSystem != 0) continue;
            pEntry->Part_FileSystem = fileSystem;
            pEntry->Part_StartSector = sector;
            pEntry->Part_TotalSectors = count;
            sector += count;
            sectors -= count;
            rc = true;
            break;
            }
        break;
        }

    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootBlockFal_t::
CreateMbrAndLayout(
    FormatInfo_t *pInfo,
    size_t mbrBlock,
    uint8_t *pMbrSector,
    uint8_t *pLayoutSector
    )
{
    bool rc = false;

    // Calculate size for flash regions
    size_t xipSectors = 2;
    size_t roSectors = 0;
    size_t rwSectors = 0;
    for (enum_t ix = 0; ix < pInfo->partitions; ix++)
        {
        PartitionInfo_t *pPartition = &pInfo->pPartitionInfo[ix];
        switch (pPartition->fileSystem)
            {
            case PART_BOOTSECTION:
            case PART_RAMIMAGE:
                xipSectors += pPartition->sectors;
                break;
            case PART_IMGFS:
                roSectors += pPartition->sectors;
                break;
            case PART_DOS3_FAT:
            case PART_DOS4_FAT:
                if (rwSectors != -1) rwSectors += pPartition->sectors;
                break;
            }
        }

    // We have so many blocks
    size_t blocks = m_blocks - m_badBlocks;
    if (mbrBlock > blocks) goto cleanUp;
    blocks -= mbrBlock;
    
    // Calculate XIP region blocks & check we fit
    size_t xipBlocks = (xipSectors + m_sectorsPerBlock - 1) / m_sectorsPerBlock;
    if (xipBlocks > blocks) goto cleanUp;
    blocks -= xipBlocks;
    
    // Calculate RO region blocks & check we fit
    size_t roBlocks = (roSectors + m_sectorsPerBlock - 1) / m_sectorsPerBlock;
    if ((roBlocks + 2) > blocks) goto cleanUp;
    
    // Calculate RW region blocks & check we fit
    size_t rwBlocks = (size_t)-1; 
    if (rwSectors != -1)
        {
        rwBlocks = (rwSectors + m_sectorsPerBlock - 1) / m_sectorsPerBlock;
        if ((rwBlocks + 2) > blocks) goto cleanUp;
        }
    // But even if we count number of RW region block use rest of flash
    rwBlocks = (size_t)-1;

    // Clear layout sector & set signature
    memset(pLayoutSector, 0xFF, m_sectorSize);
    FlashLayoutSector *pLayout;
    pLayout = reinterpret_cast<FlashLayoutSector *>(pLayoutSector);
    memcpy(pLayout->abFLSSig, FLASH_LAYOUT_SIG, FLASH_LAYOUT_SIG_SIZE);

    // Fill reserved region info
    size_t block = 0;
    pLayout->cbReservedEntries = 0;
    ReservedEntry *pEntry = reinterpret_cast<ReservedEntry *>(&pLayout[1]);
    for (enum_t ix = 0; ix < pInfo->reservedRegions; ix++)
        {
        ReservedRegionInfo_t *pReserved = &pInfo->pReservedRegionInfo[ix];
        memset(pEntry, 0, sizeof(ReservedEntry));
        size_t sectors = pReserved->sectors + m_sectorsPerBlock - 1;
        blocks = sectors / m_sectorsPerBlock;
        memcpy(pEntry->szName, pReserved->name, sizeof(pEntry->szName));
        pEntry->dwStartBlock = block;
        pEntry->dwNumBlocks = blocks;
        block += blocks;
        pEntry++;
        pLayout->cbReservedEntries += sizeof(ReservedEntry);
        }

    // Fill flash regions info
    pLayout->cbRegionEntries = 0;
    FlashRegion *pRegion = reinterpret_cast<FlashRegion *>(pEntry);
    if (xipBlocks > 0)
        {
        memset(pRegion, 0, sizeof(FlashRegion));
        pRegion->regionType = XIP;
        pRegion->dwNumLogicalBlocks = xipBlocks;
        pRegion->dwSectorsPerBlock = m_sectorsPerBlock;
        pRegion->dwBytesPerBlock = m_blockSize;
        pRegion->dwCompactBlocks = 0;
        pLayout->cbRegionEntries += sizeof(FlashRegion);
        pRegion++;
        }
    if (roBlocks > 0)
        {
        memset(pRegion, 0, sizeof(FlashRegion));
        pRegion->regionType = READONLY_FILESYS;
        pRegion->dwNumLogicalBlocks = roBlocks;
        pRegion->dwSectorsPerBlock = m_sectorsPerBlock;
        pRegion->dwBytesPerBlock = m_blockSize;
        pRegion->dwCompactBlocks = 2;
        pLayout->cbRegionEntries += sizeof(FlashRegion);
        pRegion++;
        }
    if (rwBlocks > 0)
        {
        memset(pRegion, 0, sizeof(FlashRegion));
        pRegion->regionType = FILESYS;
        pRegion->dwNumLogicalBlocks = rwBlocks;
        pRegion->dwSectorsPerBlock = m_sectorsPerBlock;
        pRegion->dwBytesPerBlock = m_blockSize;
        pRegion->dwCompactBlocks = 2;
        pLayout->cbRegionEntries += sizeof(FlashRegion);
        pRegion++;
        }

    // Clean partition table
    PARTENTRY table[PARTITIONS_IN_MBR];
    memset(table, 0, sizeof(table));

    // Add partitions on XIP flash region
    size_t sector = 2;
    size_t sectors = xipSectors - 2;
    AddPartitionByType(table, PART_BOOTSECTION, pInfo, sector, sectors);
    AddPartitionByType(table, PART_RAMIMAGE, pInfo, sector, sectors);

    // Add partitions on RO flash region
    sector = xipBlocks * m_sectorsPerBlock;
    sectors = roSectors;
    AddPartitionByType(table, PART_IMGFS, pInfo, sector, sectors);

    // Add partitions on RW flash region
    sector = (xipBlocks + roBlocks) * m_sectorsPerBlock;
    sectors = rwSectors;
    AddPartitionByType(table, PART_DOS3_FAT, pInfo, sector, sectors);
    AddPartitionByType(table, PART_DOS4_FAT, pInfo, sector, sectors);

    // Create MBR layout
    memset(pMbrSector, 0xFF, m_sectorSize);
    pMbrSector[0] = BS3BYTJMP;
    *(uint16_t*)&pMbrSector[BOOT_SIZE - 2] = BOOTSECTRAILSIGH;
    memcpy(&pMbrSector[BOOT_SIZE - 2 - sizeof(table)], table, sizeof(table));

    // Done
    rc = true;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootBlockFal_t::
ReadSector(
    size_t sector,
    uint8_t *pBuffer,
    SectorInfo *pInfo
    )
{
    bool_t rc = false;

    if (!FMD_ReadSector(sector, pBuffer, pInfo, 1))
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootBlockFal_t::ReadSector: "
            L"FMD_ReadSector failed for sector %d!\r\n", sector
            ));
        goto cleanUp;
        }
    rc = true;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootBlockFal_t::
WriteSector(
    size_t sector,
    uint8_t *pBuffer,
    SectorInfo *pInfo
    )
{
    bool_t rc = false;

    if (!FMD_WriteSector(sector, pBuffer, pInfo, 1))
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootBlockFal_t::WriteSector: "
            L"FMD_WriteSector failed for sector %d!\r\n", sector
            ));
        goto cleanUp;
        }
    rc = true;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootBlockFal_t::
WriteSectorMultistep(
    size_t sector,
    uint8_t *pData,
    uint32_t reserved1
    )
{
    bool_t rc = false;
    SectorInfo info;

    // Write header first
    info.dwReserved1 = reserved1;
    info.bOEMReserved = 0xFF;
    info.wReserved2 = (WORD)~SECTOR_WRITE_IN_PROGRESS;
    info.bBadBlock = 0xFF;
    if (!WriteSector(sector, NULL, &info)) goto cleanUp;

    // Now write data            
    if (!WriteSector(sector, pData, &info)) goto cleanUp;

    // And as last update
    info.wReserved2 &= ~SECTOR_WRITE_COMPLETED;
    if (!WriteSector(sector, NULL, &info)) goto cleanUp;

    // Done
    rc = true;
    
cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootBlockFal_t::
UpdateOemByte(
    size_t sector,
    uint8_t oem
    )
{
    bool_t rc = false;
    SectorInfo info;
    uint8_t newOem;

    // Read sector information
    if (!ReadSector(sector, NULL, &info)) goto cleanUp;

    newOem = info.bOEMReserved & oem;
    if (newOem != oem)
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootBlockFal_t::UpdateOemByte: "
            L"Unable update OEM byte from 0x%02x to 0x%02x (sector %d)!\r\n",
            info.bOEMReserved, oem, sector
            ));
        goto cleanUp;
        }
    info.bOEMReserved = newOem;

    // Update sector information
    if (!WriteSector(sector, NULL, &info)) goto cleanUp;

    // Done
    rc = true;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootBlockFal_t::
EraseBlock(
    size_t block
    )
{
    bool_t rc = false;

    // Don't erase bad block
    if ((m_aBlock[block].flags & BlockBad) != 0)
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootBlockFal_t::EraseBlock: "
            L"Block %d marked as bad block!\r\n", block
            ));
        goto cleanUp;
        }

    if (!FMD_EraseBlock(block))
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootBlockFal_t::EraseBlock: "
            L"FMD_EraseBlock failed for block %d!\r\n", block
            ));
        m_aBlock[block].flags = (flags_t)BlockBad;
        goto cleanUp;
        }

    m_aBlock[block].flags = (flags_t)BlockEmpty;
    m_aBlock[block].sectorLow = (size_t)-1;
    m_aBlock[block].sectorHigh = (size_t)-1;
    rc = true;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootBlockFal_t::
IsEmpty(
    SectorInfo &info
    )
{
    bool_t rc = false;

    if (info.wReserved2 != FREE_SECTOR) goto cleanUp;
    rc = true;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootBlockFal_t::
IsDirty(
    SectorInfo &info
    )
{
    bool_t rc = true;

    if ((info.wReserved2 & SECTOR_WRITE) != 0) goto cleanUp;
    if ((info.wReserved2 & DIRTY_SECTOR) == 0) goto cleanUp;
    rc = false;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootBlockFal_t::
EraseInBlock(
    size_t block,
    size_t sector,
    size_t sectors
    )
{
    bool_t rc = false;
    uint8_t *pBuffer = NULL;
    SectorInfo *pInfoBuffer = NULL;

    // We are done if block is empty
    if ((m_aBlock[block].flags & BlockEmpty) != 0)
        {
        rc = true;
        goto cleanUp;
        }

    // Allocate buffer
    pInfoBuffer = new SectorInfo[m_sectorsPerBlock];
    if (pInfoBuffer == NULL) goto cleanUp;

    size_t s = block * m_sectorsPerBlock;
    uint8_t *pData = NULL;
    SectorInfo *pInfo = pInfoBuffer;
    size_t used = 0;
    for (size_t ix = 0; ix < m_sectorsPerBlock; ix++)
        {
        if (used == 0)
            {
            if (!ReadSector(s, NULL, pInfo)) goto cleanUp;
            if (IsEmpty(*pInfo))
                {
                s++;
                pInfo++;
                continue;
                }
            pBuffer = new uint8_t[m_blockSize];
            if (pBuffer == NULL) goto cleanUp;
            pData = pBuffer + ix * m_sectorSize;
            used++;
            }
        if (!ReadSector(s, pData, pInfo)) goto cleanUp;
        s++;
        pData += m_sectorSize;
        pInfo++;
        }

    // If there isn't erase needed, we are done...
    if (used == 0)
        {
        m_aBlock[block].flags |= BlockEmpty;
        rc = true;
        goto cleanUp;
        }

    // Erase block
    if (!EraseBlock(block)) goto cleanUp;
    m_aBlock[block].flags |= BlockEmpty;
    if ((m_aBlock[block].flags & BlockReserved) != 0)
        {
        FMD_SetBlockStatus(block, BLOCK_STATUS_RESERVED);
        }

    // Write back data
    s = block * m_sectorsPerBlock;
    pData = pBuffer;
    pInfo = pInfoBuffer;
    for (size_t ix = 0; ix < m_sectorsPerBlock; ix++)
        {
        // Don't write back erased sectors and empty ones...
        if ((ix < sector) || (ix >= (sector + sectors)))
            {
            if (!IsEmpty(*pInfo))
                {
                if (!WriteSector(s, pData, pInfo)) goto cleanUp;
                m_aBlock[block].flags &= ~BlockEmpty;
                }
            }
        s++;
        pData += m_sectorSize;
        pInfo++;
        }

    // Done
    rc = true;

cleanUp:
    delete [] pInfoBuffer;
    delete [] pBuffer;
    return rc;
}

//------------------------------------------------------------------------------

size_t
BootBlockFal_t::
MapSectorDirect(
    Region_t *pRegion,
    size_t &sector,
    size_t &block,
    size_t &sectorInBlock
    )
{
    sector -= pRegion->sector;
    size_t blocks = sector / m_sectorsPerBlock;
    sectorInBlock = sector - blocks * m_sectorsPerBlock;
    size_t endBlock = pRegion->block + pRegion->blocks;

    block = pRegion->block;
    while ((blocks > 0) && (block < endBlock))
        {
        block++;
        if ((m_aBlock[block].flags & BlockBad) != 0) continue;
        blocks--;
        }

    sector = block * m_sectorsPerBlock + sectorInBlock;
    return (blocks == 0);
}

//------------------------------------------------------------------------------

bool_t
BootBlockFal_t::
NextSectorDirect(
    Region_t *pRegion,
    size_t &sector,
    size_t sectors,
    size_t &block,
    size_t &sectorInBlock
    )
{
    bool_t rc = false;

    while (sectors-- > 0)
        {
        sector++;
        sectorInBlock++;
        if (sectorInBlock >= m_sectorsPerBlock)
            {
            size_t endBlock = pRegion->block + pRegion->blocks;
            sectorInBlock = 0;
            block++;
            while ((m_aBlock[block].flags & BlockBad) != 0)
                {
                block++;
                sector += m_sectorsPerBlock;
                if (block >= endBlock) goto cleanUp;
                }
            }
        }
    
    rc = true;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootBlockFal_t::
ReadDirect(
    Region_t *pRegion,
    size_t sector,
    size_t sectors,
    void *pBuffer
    )
{
    bool rc = false;
    uint8_t *pData = reinterpret_cast<uint8_t*>(pBuffer);

    // Map sector to physical block
    size_t block, sectorInBlock;
    if (!MapSectorDirect(pRegion, sector, block, sectorInBlock)) goto cleanUp;

    // Read sectors
    while (sectors > 0)
        {
        if (!ReadSector(sector, pData, NULL))
            goto cleanUp;
        pData += m_sectorSize;
        sectors--;
        if (sectors == 0) break;
        if (!NextSectorDirect(pRegion, sector, 1, block, sectorInBlock))
            goto cleanUp;
        }

    // Done
    rc = true;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootBlockFal_t::
EraseDirect(
    Region_t *pRegion,
    size_t sector,
    size_t sectors
    )
{
    bool rc = false;

    // Map sector to physical block
    size_t block, inBlock;
    if (!MapSectorDirect(pRegion, sector, block, inBlock)) goto cleanUp;

    // Erase sector by sector
    while (sectors > 0)
        {
        size_t erase = m_sectorsPerBlock - inBlock;
        if (erase > sectors) erase = sectors;
        if (erase >= m_sectorsPerBlock)
            {
            EraseBlock(block);

            // Preserve reserved marking for binary region blocks
            if(pRegion->type == RegionBinary)
                {
                UpdateOemByte(block * m_sectorsPerBlock, 0);
                m_aBlock[block].flags = BlockReserved;
                FMD_SetBlockStatus(block, BLOCK_STATUS_RESERVED);
                }
            }
        else
            {
            if (!EraseInBlock(block, inBlock, erase)) goto cleanUp;
            }
        sectors -= erase;
        if (sectors == 0) break; 
        if (!NextSectorDirect(pRegion, sector, erase, block, inBlock))
            goto cleanUp;
        }

    // Done
    rc = true;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootBlockFal_t::
WriteDirect(
    Region_t *pRegion,
    size_t sector,
    size_t sectors,
    void *pBuffer
    )
{
    bool rc = false;
    uint8_t *pData = reinterpret_cast<uint8_t*>(pBuffer);
    uint32_t id;

    switch (pRegion->type)
        {
        case RegionXIP:
            id = pRegion->sector + sector;
            break;
        default:
            id = (uint32_t)-1;
            break;
        }

    // Map sector to physical block
    size_t block, inBlock;
    if (!MapSectorDirect(pRegion, sector, block, inBlock)) goto cleanUp;

    // Write sectors
    while (sectors > 0)
        {
        if (!WriteSectorMultistep(sector, pData, id)) goto cleanUp;
        m_aBlock[block].flags &= ~BlockEmpty;
        pData += m_sectorSize;
        sectors--;
        if (id != -1) id++;
        if (sectors == 0) break;
        if (!NextSectorDirect(pRegion, sector, 1, block, inBlock))
            goto cleanUp;
        }

    // Done
    rc = true;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootBlockFal_t::
ReadMapped(
    Region_t *pRegion,
    size_t sector,
    size_t sectors,
    void *pBuffer
    )
{
    bool rc = false;
    uint8_t *pData = reinterpret_cast<uint8_t*>(pBuffer);
    size_t blockFirst = pRegion->block;
    size_t blockLast = blockFirst + pRegion->blocks - 1;
    size_t sectorFirst = sector + pRegion->sector;
    size_t sectorLast = sectorFirst + sectors - 1;

    size_t sc = blockFirst * m_sectorsPerBlock;
    for (size_t block = blockFirst; block < blockLast; block++)
        {
        Block_t *pBlock = &m_aBlock[block];

        // Skip bad, reserved or empty blocks
        if ((pBlock->flags & (BlockBad|BlockReserved|BlockEmpty)) != 0)
            {
            sc += m_sectorsPerBlock;
            continue;
            }

        // If block mapping is valid we may skip block
        if (((pBlock->flags & BlockSectors) != 0) &&
            (sectorLast < pBlock->sectorLow) &&
            (sectorFirst > pBlock->sectorHigh))
            {
            sc += m_sectorsPerBlock;
            continue;
            }

        // Now look for sector
        size_t ix, low = (size_t)-1, high = 0, used = 0;
        for (ix = 0; (ix < m_sectorsPerBlock) && (sectors > 0); ix++, sc++)
            {
            // Read sector info
            SectorInfo info;
            if (!ReadSector(sc, NULL, &info)) goto cleanUp;

            // Check for empty sector
            if (IsEmpty(info)) continue;

            // At this moment we know sector is used
            used++;
            
            // If sector is dirty or not done, try next one...
            if (IsDirty(info)) continue;

            // Update low/high
            if (info.dwReserved1 < low) low = info.dwReserved1;
            if (info.dwReserved1 > high) high = info.dwReserved1;

            if (info.dwReserved1 < sectorFirst) continue;
            if (info.dwReserved1 > sectorLast) continue;

            // Read sector data
            size_t offset = (info.dwReserved1 - sectorFirst) * m_sectorSize;
            if (!ReadSector(sc, &pData[offset], NULL)) goto cleanUp;

            // We read one sector
            sectors--;
            }

        // Update block sector info
        if (ix >= m_sectorsPerBlock)
            {
            pBlock->sectorLow = low;
            pBlock->sectorHigh = high;
            pBlock->flags |= BlockSectors;
            if (used == 0) pBlock->flags |= BlockEmpty;
            else if (used >= m_sectorsPerBlock) pBlock->flags |= BlockFull;
            }
        
        // If we read all sectors we are done
        if (sectors == 0) break;
        }

    // Done
    rc = (sectors == 0);

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootBlockFal_t::
EraseMapped(
    Region_t *pRegion,
    size_t sector,
    size_t sectors
    )
{
    bool rc = false;
    size_t blockFirst = pRegion->block;
    size_t blockLast = blockFirst + pRegion->blocks - 1;
    size_t sectorFirst = sector + pRegion->sector;;
    size_t sectorLast = sectorFirst + sectors - 1;


    for (size_t block = blockFirst; block < blockLast; block++)
        {
        Block_t *pBlock = &m_aBlock[block];

        // Skip bad, reserved or empty blocks
        if ((pBlock->flags & (BlockBad|BlockReserved|BlockEmpty)) != 0)
            continue;

        // If block mapping is valid we may skip block
        if ((pBlock->flags & BlockSectors) != 0)
            {
            if ((sectorLast < pBlock->sectorLow) ||
                (sectorFirst > pBlock->sectorHigh))
                {
                continue;
                }
            }
        
        // Now look for sector
        size_t ix, low = (size_t)-1, high = 0, dirty = 0, empty = 0;
        size_t s = block * m_sectorsPerBlock;
        for (ix = 0; (ix < m_sectorsPerBlock) && (sectors > 0); ix++, s++)
            {
            // Read sector info
            SectorInfo info;
            if (!ReadSector(s, NULL, &info)) goto cleanUp;

            // If sector is dirty, not done or empty, try next one...
            if (IsEmpty(info))
                {
                empty++;
                continue;
                }

            if (IsDirty(info))
                {
                dirty++;
                continue;
                }
            
            // If sector shouldn't be erased only update low/high
            if ((info.dwReserved1 < sectorFirst) ||
                (info.dwReserved1 > sectorLast))
                {
                // Update low/high
                if (info.dwReserved1 < low) low = info.dwReserved1;
                if (info.dwReserved1 > high) high = info.dwReserved1;
                continue;
                }

            // Set dirty flag
            info.wReserved2 &= ~0x0001;
            if (!WriteSector(s, NULL, &info)) goto cleanUp;

            // Mapping isn't valid anymore
            pBlock->flags &= ~BlockSectors;
            dirty++;
            
            // We read one sector
            sectors--;
            }

        // Update block sector info
        if (ix >= m_sectorsPerBlock)
            {
            if ((dirty + empty) >= m_sectorsPerBlock)
                {
                if (dirty > 0) 
                    {
                    EraseBlock(block);
                    }
                else
                    {
                    pBlock->flags |= BlockEmpty;
                    }
                }
            else
                {
                pBlock->sectorLow = low;
                pBlock->sectorHigh = high;
                pBlock->flags |= BlockSectors;
                }
            }

        // If we erase all sectors we are done
        if (sectors == 0) break;
        }

    // Done
    rc = true;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootBlockFal_t::
WriteMapped(
    Region_t *pRegion,
    size_t sector,
    size_t sectors,
    void *pBuffer
    )
{
    bool_t rc = false;
    uint8_t *pData = reinterpret_cast<uint8_t*>(pBuffer);
    size_t blockFirst = pRegion->block;
    size_t blockLast = blockFirst + pRegion->blocks - 1;


    size_t sc = blockFirst * m_sectorsPerBlock;
    for (size_t block = blockFirst; block < blockLast; block++)
        {
        Block_t *pBlock = &m_aBlock[block];

        // Skip bad, reserved and full blocks
        if ((pBlock->flags & (BlockBad|BlockReserved|BlockFull)) != 0)
            {
            sc += m_sectorsPerBlock;
            continue;
            }

        // Now look for sector
        size_t ix, low = (size_t)-1, high = 0, used = 0;
        for (ix = 0; (ix < m_sectorsPerBlock) && (sectors > 0); ix++, sc++)
            {
            // Read sector info
            SectorInfo info;
            if (!ReadSector(sc, NULL, &info)) goto cleanUp;

            // If sector isn't empty try next one...
            if (!IsEmpty(info))
                {
                used++;
                // Update low/high
                if (!IsDirty(info))
                    {
                    if (info.dwReserved1 < low) low = info.dwReserved1;
                    if (info.dwReserved1 > high) high = info.dwReserved1;
                    }
                continue;
                }

            // Write sector
            size_t id = pRegion->sector + sector + ix;
            if (!WriteSectorMultistep(sc, pData, id)) goto cleanUp;

            // Update block mapping info
            if ((pBlock->flags & BlockEmpty) != 0)
                {
                pBlock->sectorLow = id;
                pBlock->sectorHigh = id;
                pBlock->flags |= BlockSectors;
                pBlock->flags &= ~BlockEmpty;
                }
            else if ((pBlock->flags & BlockSectors) != 0)
                {
                if (id < pBlock->sectorLow) pBlock->sectorLow = id;
                if (id > pBlock->sectorHigh) pBlock->sectorHigh = id;
                }
            
            // Update low/high
            if (id < low) low = id;
            if (id > high) high = id;
            
            // We write one sector
            pData += m_sectorSize;
            sectors--;
            used++;
            }

        // Update block sector info
        if (ix >= m_sectorsPerBlock)
            {
            pBlock->sectorLow = low;
            pBlock->sectorHigh = high;
            pBlock->flags |= BlockSectors;
            if (used >= m_sectorsPerBlock) pBlock->flags |= BlockFull;
            }
        
        // If we read all sectors we are done
        if (sectors == 0) break;
        }

    // Done
    rc = (sectors == 0);

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

