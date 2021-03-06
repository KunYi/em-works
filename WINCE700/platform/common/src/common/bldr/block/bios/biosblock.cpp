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
#include <biosBlock.hpp>
#include <biosBlockSegment.hpp>
#include <bootLog.h>
#include <bootBios.h>

using namespace ceboot;

//------------------------------------------------------------------------------

#pragma pack(push, 1)

typedef struct PartitionEntry_t {
    uint8_t  bootInd;           // If 80h means this is boot partition
    uint8_t  firstHead;         // Partition starting head based 0
    uint8_t  firstSector;       // Partition starting sector based 1
    uint8_t  firstTrack;        // Partition starting track based 0
    uint8_t  fileSystem;        // Partition type signature field
    uint8_t  lastHead;          // Partition ending head based 0
    uint8_t  lastSector;        // Partition ending sector based 1
    uint8_t  lastTrack;         // Partition ending track based 0
    uint32_t startSector;       // Logical starting sector based 0
    uint32_t totalSectors;      // Total logical sectors in partition
} PartitionEntry_t;

typedef PartitionEntry_t PartitionTable_t[4];

#pragma pack(pop)

enum FILE_SYSTEM {
    FILE_SYSTEM_EXTENDED = 0x05,
    FILE_SYSTEM_EXTENDED2 = 0x0F,
    FILE_SYSTEM_BINARY = 0x18,
    FILE_SYSTEM_ULDR = 0x20,
    FILE_SYSTEM_BINFS = 0x21,
    FILE_SYSTEM_IMGFS = 0x25,
    FILE_SYSTEM_RESERVED = 0x26
};

typedef struct BiosDeviceAddressPacket_t {
    uint16_t size;
    uint16_t blocks;
    uint16_t bufferPtr[2];
    uint32_t blockLo;
    uint32_t blockHi;
} BiosDeviceAddressPacket_t;

typedef struct BiosDriveParameters_t {
    uint16_t size;
    uint16_t flags;
    uint32_t physicalCylinders;
    uint32_t physicalHeads;
    uint32_t physicalSectors;
    uint32_t sectorsLo;
    uint32_t sectorsHi;
    uint16_t sectorSize;
} BiosDriveParameters_t;

//------------------------------------------------------------------------------

#define PARTITIONS_IN_MBR           4
#define BOOT_SIZE                   512

//------------------------------------------------------------------------------

__inline
size_t
RoundUp(
    size_t size,
    size_t itemSize
    )
{
    return ((size + itemSize - 1) / itemSize) * itemSize;
}

//------------------------------------------------------------------------------

extern "C"
handle_t
BootBlockBiosInit(
    enum_t diskId
    )
{
    return BootBlockBios_t::BootBlockBiosInit(diskId);
}

//------------------------------------------------------------------------------

BootBlockBios_t*
BootBlockBios_t::
BootBlockBiosInit(
    enum_t driveId
    )
{
    BootBlockBios_t* pDriver = NULL;

    pDriver = new BootBlockBios_t();
    if ((pDriver != NULL) && !pDriver->Init(driveId))
        {
        delete pDriver;
        pDriver = NULL;
        }
    return pDriver;
}

//------------------------------------------------------------------------------

BootBlockBios_t::
BootBlockBios_t(
    ) : m_aReserved(NULL), m_aPartition(NULL),
        m_reserveds(0), m_partitions(0),
        m_refCount(1)
{
}

//------------------------------------------------------------------------------

BootBlockBios_t::
~BootBlockBios_t(
    )
{
    delete [] m_aReserved;
    delete [] m_aPartition;
}

//------------------------------------------------------------------------------

enum_t
BootBlockBios_t::
AddRef(
    )
{
    return m_refCount++;
}

//------------------------------------------------------------------------------

enum_t
BootBlockBios_t::
Release(
    )
{
    if (--m_refCount == 0) delete this;
    return m_refCount;
}

//------------------------------------------------------------------------------

bool_t
BootBlockBios_t::
Init(
    enum_t driveId
    )
{
    bool rc = false;
    BiosDriveParameters_t *pInfo;
    uint32_t eax, ebx, ecx, edx, esi, edi;

    // Find if there is BIOS Enhanced Disk Drive Services support
    eax = 0x4100;
    ebx = 0x55AA;
    edx = driveId;
    BootBiosInt13(&eax, &ebx, &ecx, &edx, &esi, &edi);
    if ((ebx & 0xFFFF) == 0xAA55 && (ecx & 1))
        {
        m_useEdd = true;
        m_eddBiosMajor = (uint8_t)((eax & 0xF000) >> 12);
        m_eddBiosMinor = (uint8_t)((eax & 0x0F00) >> 8);
        m_eddBiosIfcs  = (uint16_t)ecx;
        }
    else
        {
        m_useEdd = false;
        m_eddBiosMajor = 0;
        m_eddBiosMinor = 0;
        m_eddBiosIfcs  = 0;
        }
    
    // Get logical geometry
    eax = 0x0800;
    edx = driveId;
    BootBiosInt13(&eax, &ebx, &ecx, &edx, &esi, &edi);
    if ((eax & 0xFF00) != 0) goto cleanUp;

    m_cylinders = (((ecx & 0xFF00) >> 8)|((ecx & 0x00C0) << 2)) + 1;
    m_headsPerCylinder = ((edx & 0xFF00) >> 8) + 1;
    m_sectorsPerHead = (ecx & 0x003F);

    m_driveId = (uint8_t)driveId;

    // Get additional info
    if (m_useEdd)
        {
        pInfo = reinterpret_cast<BiosDriveParameters_t *>(BootBiosBuffer());
        eax = 0x4800;
        edx = driveId;
        esi = (uint16_t)pInfo;
        memset(pInfo, 0, sizeof(*pInfo));
        pInfo->size = sizeof(*pInfo);
        BootBiosInt13(&eax, &ebx, &ecx, &edx, &esi, &edi);

        m_sectorSize = pInfo->sectorSize;
        m_sectors = pInfo->sectorsLo; // | (pInfo->sectorsHi << 32);
        }
    else
        {
        m_sectorSize = 512;
        m_sectors = m_cylinders * m_headsPerCylinder * m_sectorsPerHead;
        }

    if ((m_driveId & 0x80) != 0)
        {
        rc = Mount();
        }
    else
        {
        m_aPartition = new Partition_t[1];
        if (m_aPartition == NULL)
            {
            BOOTMSG(ZONE_ERROR, (L"ERROR: BootBlockBios_t::Init: "
                L"Memory allocation for partition info failed!\r\n"
                ));
            goto cleanUp;
            }
        m_partitions = 1;
        m_aPartition[0].active = true;
        m_aPartition[0].baseSector = 0;
        m_aPartition[0].sector = 0;
        m_aPartition[0].sectors = m_sectors;
        rc = true;
        }

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootBlockBios_t::
DeInit(
    )
{
    return Release() == 0;
}

//------------------------------------------------------------------------------

bool_t
BootBlockBios_t::
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
        }

    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootBlockBios_t::
Info(
    flags_t &flags,
    size_t  &sectorSize,
    size_t  &sectors,
    enum_t  &binaryRegions,
    enum_t  &reservedRegions,
    enum_t  &partitions
    )
{
    flags = 0;
    sectorSize = m_sectorSize;
    sectors = m_sectors;
    binaryRegions = 0;
    reservedRegions = 0;
    partitions = 0;
    return true;
}

//------------------------------------------------------------------------------

bool_t
BootBlockBios_t::
Format(
    FormatInfo_t *pInfo
    )
{
    bool_t rc = false;
    Partition_t *aPartitions = NULL;

    // Check info consistency
    if (((pInfo->binaryRegions > 0)&&(pInfo->pBinaryRegionInfo == NULL)) ||
        ((pInfo->reservedRegions > 0)&&(pInfo->pReservedRegionInfo == NULL)) ||
        ((pInfo->partitions > 0)&&(pInfo->pPartitionInfo == NULL)))
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootBlockBios_t::Format: "
            L"Invalid format info structure values!\r\n"
            ));
        goto cleanUp;
        }

    // Special check for media with only one FAT12/16/32 partition which
    // should be formated for image update. Now we will do it with this
    // trivial check...
    if ((m_partitions == 1) && ((pInfo->flags & FormatBinaryRegions) == 0))
        {
        m_aPartition[0].fileSystem = FILE_SYSTEM_BINARY;
        }

    // Find how many partition new layout needs
    enum_t partitions;
    enum_t firstPatitionToErase;
    if ((pInfo->flags & FormatBinaryRegions) != 0)
        {
        partitions = pInfo->binaryRegions;
        firstPatitionToErase = 0;
        }
    else
        {
        enum_t partition = 0;
        while (partition < m_partitions)
            {
            if (m_aPartition[partition].fileSystem != FILE_SYSTEM_BINARY) break;
            partition++;
            }
        partitions = partition;
        firstPatitionToErase = partition;
        }

    partitions += pInfo->partitions;
    if (pInfo->reservedRegions > 0) partitions++;

    // Allocate table for new format
    aPartitions = new Partition_t[partitions];
    if (aPartitions == NULL)
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootBlockBios_t::Format: "
            L"Memory allocation for partition table failed!\r\n"
            ));
        goto cleanUp;
        }

    // Start create new partition table
    enum_t partition = 0;
    size_t sector = m_sectorsPerHead;

    // Binary region
    if ((pInfo->flags & FormatBinaryRegions) == 0)
        {
        // Leave original binary partitions
        for (enum_t ix = 0; ix < m_partitions; ix++)
            {
            if (m_aPartition[ix].fileSystem != FILE_SYSTEM_BINARY) break;
            aPartitions[partition].fileSystem = m_aPartition[ix].fileSystem;
            aPartitions[partition].baseSector = m_aPartition[ix].baseSector;
            aPartitions[partition].sector = m_aPartition[ix].sector;
            aPartitions[partition].sectors = m_aPartition[ix].sectors;
            sector = m_aPartition[ix].sector + m_aPartition[ix].sectors;
            sector = RoundUp(sector, m_sectorsPerHead);
            partition++;
            }
        }
    else
        {
        // There will be new binary region
        for (enum_t ix = 0; ix < pInfo->binaryRegions; ix++)
            {
            BinaryRegionInfo_t *pBinaryInfo = &pInfo->pBinaryRegionInfo[ix];
            aPartitions[partition].fileSystem = FILE_SYSTEM_BINARY;
            if (partition < PARTITIONS_IN_MBR - 1)
                {
                aPartitions[partition].baseSector = 0;
                aPartitions[partition].sector = sector;
                }
            else
                {
                aPartitions[partition].baseSector = sector;
                sector += m_sectorsPerHead;
                aPartitions[partition].sector  = sector;
                }
            aPartitions[partition].sectors = pBinaryInfo->sectors;
            sector += pBinaryInfo->sectors;
            sector = RoundUp(sector, m_sectorsPerHead);
            partition++;
            }
        }

    // Reserved regions partition
    if (pInfo->reservedRegions > 0)
        {
        size_t size = 1;
        for (enum_t ix = 0; ix < pInfo->reservedRegions; ix++)
            {
            size += pInfo->pReservedRegionInfo[ix].sectors;
            }
        aPartitions[partition].fileSystem = FILE_SYSTEM_RESERVED;
        if (partition < PARTITIONS_IN_MBR - 1)
            {
            aPartitions[partition].baseSector = 0;
            aPartitions[partition].sector = sector;
            }
        else
            {
            aPartitions[partition].baseSector = sector;
            sector += m_sectorsPerHead;
            aPartitions[partition].sector  = sector;
            }
        aPartitions[partition].sectors = size;
        sector += size;
        sector = RoundUp(sector, m_sectorsPerHead);
        partition++;
        }

    // Ordinary regions
    for (enum_t ix = 0; ix < pInfo->partitions; ix++)
        {
        PartitionInfo_t *pPartitionInfo = &pInfo->pPartitionInfo[ix];

        aPartitions[partition].fileSystem = pPartitionInfo->fileSystem;
        if (partition < PARTITIONS_IN_MBR - 1)
            {
            aPartitions[partition].baseSector = 0;
            aPartitions[partition].sector = sector;
            }
        else
            {
            aPartitions[partition].baseSector = sector;
            sector += m_sectorsPerHead;
            aPartitions[partition].sector  = sector;
            }
        // Partition with size -1 should occupy rest of media
        if (pPartitionInfo->sectors == -1)
            {
            size_t sectors = m_sectors - sector;
            aPartitions[partition].sectors = sectors;
            sector += sectors;
            }
        else
            {
            aPartitions[partition].sectors = pPartitionInfo->sectors;
            sector += pPartitionInfo->sectors;
            }

            if (ix + 1 < pInfo->partitions)
                {
                // don't round up for the last partition
                sector = RoundUp(sector, m_sectorsPerHead);
                }
        partition++;
        if (sector >= m_sectors) break;
        }

    // Check if there is space for required layout on block device
    if ((sector > m_sectors) || (partition < partitions))
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootBlockBios_t::Format: "
            L"Required format doesn't fit on this device!\r\n"
            ));
        goto cleanUp;
        }

    // Dismount disk
    Dismount();

    // Update MBR and EBR on disk
    if (!UpdateLayout(partitions, aPartitions)) goto cleanUp;

    // Mount disk with new layout
    if (!Mount()) goto cleanUp;

    // Erase first sector from new partitions
    for (enum_t ix = firstPatitionToErase; ix < pInfo->partitions; ix++)
        {
        Erase(m_aPartition[ix].sector, 1);
        }

    // Done
    rc = true;

cleanUp:
    delete [] aPartitions;
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootBlockBios_t::
LockMode(
    enum_t mode
    )
{
    UNREFERENCED_PARAMETER(mode);
    return false;
}

//------------------------------------------------------------------------------

handle_t
BootBlockBios_t::
OpenBinaryRegion(
    enum_t index
    )
{
    UNREFERENCED_PARAMETER(index);
    return NULL;
}

//------------------------------------------------------------------------------

handle_t
BootBlockBios_t::
OpenReservedRegion(
    cstring_t name
    )
{
    UNREFERENCED_PARAMETER(name);
    return NULL;
}

//------------------------------------------------------------------------------

handle_t
BootBlockBios_t::
OpenPartition(
    uchar fileSystem,
    enum_t index
    )
{
    BootBlockSegmentBios_t *pSegment = NULL;

    pSegment = new BootBlockSegmentBios_t(this);
    if ((pSegment != NULL) && !pSegment->OpenPartition(fileSystem, index))
        {
        pSegment->DeInit();
        pSegment = NULL;
        }
    return pSegment;
}

//------------------------------------------------------------------------------

bool_t
BootBlockBios_t::
QueryBinaryRegion(
    enum_t index,
    size_t &sector,
    size_t &sectors
    )
{
    return QueryPartition(FILE_SYSTEM_BINARY, index, sector, sectors);
}

//------------------------------------------------------------------------------

bool_t
BootBlockBios_t::
QueryReservedRegion(
    cstring_t name,
    size_t &sector,
    size_t &sectors
    )
{
    UNREFERENCED_PARAMETER(name);
    UNREFERENCED_PARAMETER(sector);
    UNREFERENCED_PARAMETER(sectors);
    return false;
}

//------------------------------------------------------------------------------

bool_t
BootBlockBios_t::
QueryPartition(
    uint8_t fileSystem,
    enum_t index,
    size_t &sector,
    size_t &sectors
    )
{
    bool_t rc = false;

    for (enum_t partition = 0; partition < m_partitions; partition++)
        {
        Partition_t *pPartition = &m_aPartition[partition];
        if ((fileSystem != 0) || (index > 0))
            {
            if (pPartition->fileSystem != fileSystem) continue;
            if (index-- > 0) continue;
            }
        else
            {
            if (!pPartition->active) continue;
            }
        sector = pPartition->sector;
        sectors = pPartition->sectors;
        rc = true;
        break;
        }
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootBlockBios_t::
Read(
    size_t sector,
    size_t sectors,
    uint8_t *pBuffer
    )
{
    bool rc = false;
    BiosDeviceAddressPacket_t *pPacket;
    uint32_t eax, ebx, ecx, edx, esi, edi;

    pPacket = static_cast<BiosDeviceAddressPacket_t *>(BootBiosBuffer());
    
    while (sectors > 0)
        {
        if (m_useEdd)
            {
            // Prepare address packet
            pPacket->size = sizeof(*pPacket);
            pPacket->blocks = (uint16_t)(sectors > 8 ? 8 : sectors);
            pPacket->bufferPtr[0] = (uint16_t)&pPacket[1];
            pPacket->bufferPtr[1] = 0;
            pPacket->blockLo = sector;
            pPacket->blockHi = 0;

            // Call Bios EDD Services
            eax = 0x4200;
            edx = m_driveId;
            esi = (uint16_t)pPacket;
            BootBiosInt13(&eax, &ebx, &ecx, &edx, &esi, &edi);
            if ((eax & 0xFF00) != 0) goto cleanUp;

            // Copy data
            size_t size = pPacket->blocks * m_sectorSize;
            memcpy(pBuffer, &pPacket[1], size);
            pBuffer += size;
            sector += pPacket->blocks;
            sectors -= pPacket->blocks;
            
            }
        else
            {
            uint8_t c, h, s;

            // Translate LBA to CHS
            if (!Lba2Chs(sector, c, h, s)) goto cleanUp;

            // Call normal read
            eax = 0x0201;
            ebx = (uint16_t)&pPacket[1];
            ecx = s | (c << 8);
            edx = m_driveId | (h << 8);

            BootBiosInt13(&eax, &ebx, &ecx, &edx, &esi, &edi);
            if ((eax & 0xFF00) != 0) goto cleanUp;

            // Copy data
            memcpy(pBuffer, &pPacket[1], m_sectorSize);
            pBuffer += m_sectorSize;
            sector++;
            sectors--;
            }
        }
    
    // Done
    rc = true;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootBlockBios_t::
Write(
    size_t sector,
    size_t sectors,
    uint8_t *pBuffer
    )
{
    bool rc = false;
    BiosDeviceAddressPacket_t *pPacket;
    uint32_t eax, ebx, ecx, edx, esi, edi;

    pPacket = static_cast<BiosDeviceAddressPacket_t *>(BootBiosBuffer());

    while (sectors > 0)
        {
        if (m_useEdd)
            {
            // Prepare address packet
            pPacket->size = sizeof(*pPacket);
            pPacket->blocks = (uint16_t)(sectors > 8 ? 8 : sectors);
            pPacket->bufferPtr[0] = (uint16_t)&pPacket[1];
            pPacket->bufferPtr[1] = 0;
            pPacket->blockLo = sector;
            pPacket->blockHi = 0;

            // Copy data
            size_t size = pPacket->blocks * m_sectorSize;
            memcpy(&pPacket[1], pBuffer, size);

            // Call Bios EDD Services
            eax = 0x4300;
            edx = m_driveId;
            esi = (uint32_t)pPacket;
            BootBiosInt13(&eax, &ebx, &ecx, &edx, &esi, &edi);
            if ((eax & 0xFF00) != 0) goto cleanUp;

            pBuffer += size;
            sector += pPacket->blocks;
            sectors -= pPacket->blocks;
            
            }
        else
            {
            uint8_t c, h, s;

            // Translate LBA to CHS
            if (!Lba2Chs(sector, c, h, s)) goto cleanUp;

            // Copy data
            memcpy(&pPacket[1], pBuffer, m_sectorSize);

            // Call normal write
            eax = 0x0301;
            ebx = (uint16_t)&pPacket[1];
            ecx = s | (c << 8);
            edx = m_driveId | (h << 8);
            BootBiosInt13(&eax, &ebx, &ecx, &edx, &esi, &edi);
            if ((eax & 0xFF00) != 0) goto cleanUp;

            pBuffer += m_sectorSize;
            sector++;
            sectors--;
            }
        }
    
    // Done
    rc = true;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootBlockBios_t::
Erase(
    size_t sector,
    size_t sectors
    )
{
    bool rc = false;
    BiosDeviceAddressPacket_t *pPacket;
    uint32_t eax, ebx, ecx, edx, esi, edi;

    pPacket = static_cast<BiosDeviceAddressPacket_t *>(BootBiosBuffer());

    while (sectors > 0)
        {
        if (m_useEdd)
            {
            // Prepare address packet
            pPacket->size = sizeof(*pPacket);
            pPacket->blocks = (uint16_t)(sectors > 8 ? 8 : sectors);
            pPacket->bufferPtr[0] = (uint16_t)&pPacket[1];
            pPacket->bufferPtr[1] = 0;
            pPacket->blockLo = sector;
            pPacket->blockHi = 0;

            // Erase buffer
            size_t size = pPacket->blocks * m_sectorSize;
            memset(&pPacket[1], 0, size);

            // Call Bios EDD Services
            eax = 0x4300;
            edx = m_driveId;
            esi = (uint32_t)pPacket;
            BootBiosInt13(&eax, &ebx, &ecx, &edx, &esi, &edi);
            if ((eax & 0xFF00) != 0) goto cleanUp;

            sector += pPacket->blocks;
            sectors -= pPacket->blocks;
            
            }
        else
            {
            uint8_t c, h, s;

            // Translate LBA to CHS
            if (!Lba2Chs(sector, c, h, s)) goto cleanUp;

            // EraseBuffer
            memcpy(&pPacket[1], 0, m_sectorSize);

            // Call normal write
            eax = 0x0301;
            ebx = (uint16_t)&pPacket[1];
            ecx = s | (c << 8);
            edx = m_driveId | (h << 8);
            BootBiosInt13(&eax, &ebx, &ecx, &edx, &esi, &edi);
            if ((eax & 0xFF00) != 0) goto cleanUp;

            sector++;
            sectors--;
            }
        }
    
    // Done
    rc = true;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootBlockBios_t::
Mount(
    )
{
    bool_t rc = false;
    size_t sector;
    uint8_t *pSector = NULL;
    enum_t partition;


    // Allocate sector buffer
    pSector = new uint8_t[m_sectorSize];
    if (pSector == NULL)
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootBlockBios_t::Mount: "
            L"MBR sector buffer allocation failed!\r\n"
            ));
        goto cleanUp;
        }

    // Dismount
    Dismount();

    // Read partition tables
    partition = 0;
    sector = 0;
    for (;;)
        {
        size_t startSector = 0;

        // Read sector where MBR/Ext should be...
        if (!Read(sector, 1, pSector))
            {
            BOOTMSG(ZONE_ERROR, (L"ERROR: BootBlockBios_t::Mount: "
                L"MBR/Ext sector %d read failed!\r\n", sector
                ));
            goto cleanUp;
            }

        // This shouldn't happen, but who knows...
        if (pSector[BOOT_SIZE - 2] != 0x55) break;
        if (pSector[BOOT_SIZE - 1] != 0xAA) break;

        // Copy partition table to avoid alignment issue
        PartitionEntry_t table[PARTITIONS_IN_MBR];
        memcpy(table, &pSector[BOOT_SIZE - 2 - sizeof(table)], sizeof(table));

        for (enum_t ix = 0; ix < PARTITIONS_IN_MBR; ix++)
            {
            switch (table[ix].fileSystem)
                {
                case 0:
                    break;
                case FILE_SYSTEM_EXTENDED:
                case FILE_SYSTEM_EXTENDED2:
                    startSector = table[ix].startSector;
                    break;
                default:
                    partition++;
                    break;
                }
            }

        // If there isn't extension partition we are done
        if (startSector == 0) break;

        // This is new sector with partition table
        sector += startSector;
        }

    // Allocate partition info
    m_aPartition = new Partition_t[partition];
    if (m_aPartition == NULL)
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootBlockBios_t::Mount: "
            L"Memory allocation for partition info failed!\r\n"
            ));
        goto cleanUp;
        }
    m_partitions = partition;

    // Parse partition table again and save info
    partition = 0;
    sector = 0;
    for (;;)
        {
        size_t startSector = 0;

        // Read sector where MBR/Ext should be...
        if (!Read(sector, 1, pSector))
            {
            BOOTMSG(ZONE_ERROR, (L"ERROR: BootBlockBios_t::Mount: "
                L"MBR/Ext sector %d read failed!\r\n", sector
                ));
            goto cleanUp;
            }

        // This shouldn't happen, but who knows...
        if (pSector[BOOT_SIZE - 2] != 0x55) break;
        if (pSector[BOOT_SIZE - 1] != 0xAA) break;

        // Copy partition table to avoid alignment issue
        PartitionEntry_t table[PARTITIONS_IN_MBR];
        memcpy(table, &pSector[BOOT_SIZE - 2 - sizeof(table)], sizeof(table));
        for (enum_t ix = 0; ix < PARTITIONS_IN_MBR; ix++)
            {
            switch (table[ix].fileSystem)
                {
                case 0:
                    break;
                case FILE_SYSTEM_EXTENDED:
                case FILE_SYSTEM_EXTENDED2:
                    startSector = table[ix].startSector;
                    break;
                default:
                    {
                    Partition_t *pPartition = &m_aPartition[partition];
                    pPartition->fileSystem = table[ix].fileSystem;
                    pPartition->active = table[ix].bootInd == 0x80;
                    pPartition->baseSector = sector;
                    pPartition->sector = sector + table[ix].startSector;
                    pPartition->sectors = table[ix].totalSectors;
                    partition++;
                    }
                    break;
                }
            }

        // If there isn't extension partition we are done
        if (startSector == 0) break;

        // This is new sector with partition table
        sector += startSector;
        }

    // Done
    rc = true;

cleanUp:
    delete [] pSector;
    if (!rc) Dismount();
    return rc;
}

//------------------------------------------------------------------------------

void
BootBlockBios_t::
Dismount(
    )
{
    delete [] m_aReserved;
    m_aReserved = NULL;
    m_reserveds = 0;
    delete [] m_aPartition;
    m_aPartition = NULL;
    m_partitions = 0;
}

//------------------------------------------------------------------------------

bool_t
BootBlockBios_t::
Lba2Chs(
    size_t sector,
    uint8_t &c,
    uint8_t &h,
    uint8_t &s
    )
{
    bool_t rc = false;
    uint32_t sectorsPerCylinder, cylinder;

    c = h = s = 0xFF;
    uint32_t sectors = m_sectorsPerHead * m_headsPerCylinder * m_cylinders;
    if (sector > sectors) goto cleanUp;

    sectorsPerCylinder = m_sectorsPerHead * m_headsPerCylinder;
    cylinder = sector / sectorsPerCylinder;
    sector -= cylinder * sectorsPerCylinder;
    h = (uint8_t)(sector / m_sectorsPerHead);
    sector -= h * m_sectorsPerHead;
    s = (uint8_t)((sector + 1) | ((cylinder >> 2) & 0xC0));
    c = (uint8_t)(cylinder & 0xFF);

    // Done
    rc = true;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootBlockBios_t::
UpdateLayout(
    enum_t partitions,
    Partition_t *aPartitions
    )
{
    bool_t rc = false;
    uint8_t *pSector = NULL;
    enum_t partition;
    size_t sector;
    PartitionEntry_t table[PARTITIONS_IN_MBR];


    // Allocate sector buffer
    pSector = new uint8_t[m_sectorSize];
    if (pSector == NULL)
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootBlockBios_t::UpdateLayout: "
            L"Sector buffer allocation failed!\r\n"
            ));
        goto cleanUp;
        }

    memset(table, 0, sizeof(table));
    if (!Read(0, 1, pSector))
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootBlockBios_t::UpdateLayout: "
            L"Master Boot Record read from drive 0x%02x failed!\r\n",
            m_driveId
            ));
        goto cleanUp;
        }

    partition = 0;
    sector = 0;
    for (enum_t ix = 0; ix < dimof(table) - 1; ix++)
        {
        table[ix].fileSystem = aPartitions[partition].fileSystem;
        table[ix].startSector = aPartitions[partition].sector;
        table[ix].totalSectors = aPartitions[partition].sectors;
        Lba2Chs(
            aPartitions[partition].sector,
            table[ix].firstTrack, table[ix].firstHead, table[ix].firstSector
            );
        Lba2Chs(
            aPartitions[partition].sector + aPartitions[partition].sectors - 1,
            table[ix].lastTrack, table[ix].lastHead, table[ix].lastSector
            );
        if (++partition >= partitions) break;
        }

    if (partition >= dimof(table) - 1)
        {
        enum_t ix = dimof(table) - 1;
        table[ix].fileSystem = FILE_SYSTEM_EXTENDED;
        table[ix].startSector = aPartitions[partition].baseSector;
        for (enum_t iy = partition; iy < partitions; iy++)
            {
            table[ix].totalSectors += m_sectorsPerHead;
            table[ix].totalSectors += aPartitions[iy].sectors;
            }
        Lba2Chs(
            table[ix].startSector,
            table[ix].firstTrack, table[ix].firstHead, table[ix].firstSector
            );
        Lba2Chs(
            table[ix].startSector + table[ix].totalSectors - 1,
            table[ix].lastTrack, table[ix].lastHead, table[ix].lastSector
            );
        }

    // First partition is active
    table[0].bootInd = 0x80;

    // Update partition table in boot sector
    memcpy(&pSector[BOOT_SIZE - 2 - sizeof(table)], table, sizeof(table));
    if (!Write(sector, 1, pSector))
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootBlockBios_t::UpdateLayout: "
            L"Master Boot Record write to drive 0x%02x failed!\r\n",
            m_driveId
            ));
        goto cleanUp;
        }

    // Write extended partitions tables if needed...
    while (partition < partitions)
        {
        sector = aPartitions[partition].baseSector;
        memset(table, 0, sizeof(table));
        table[0].fileSystem = aPartitions[partition].fileSystem;
        table[0].startSector = m_sectorsPerHead;
        table[0].totalSectors = aPartitions[partition].sectors;
        Lba2Chs(
            table[0].startSector + sector,
            table[0].firstTrack, table[0].firstHead, table[0].firstSector
            );
        Lba2Chs(
            table[0].startSector + table[0].totalSectors + sector - 1,
            table[0].lastTrack, table[0].lastHead, table[0].lastSector
            );
        partition++;
        if (partition < partitions)
            {
            table[1].fileSystem = FILE_SYSTEM_EXTENDED;
            table[1].startSector = aPartitions[partition].baseSector - sector;
            for (enum_t iy = partition; iy < partitions; iy++)
                {
                table[1].totalSectors += m_sectorsPerHead;
                table[1].totalSectors += aPartitions[iy].sectors;
                }
            Lba2Chs(
                table[1].startSector + sector,
                table[1].firstTrack, table[1].firstHead, table[1].firstSector
                );
            Lba2Chs(
                table[1].startSector + table[1].totalSectors + sector - 1,
                table[1].lastTrack, table[1].lastHead, table[1].lastSector
                );
            }

        // Copy new table to EBR
        memset(pSector, 0, m_sectorSize);
        memcpy(&pSector[BOOT_SIZE - 2 - sizeof(table)], table, sizeof(table));
        pSector[BOOT_SIZE - 2] = 0x55;
        pSector[BOOT_SIZE - 1] = 0xAA;
        if (!Write(sector, 1, pSector))
            {
            BOOTMSG(ZONE_ERROR, (L"ERROR: BootBlockBios_t::UpdateLayout: "
                L"EBR sector %d write to drive 0x%02x failed!\r\n", sector,
                m_driveId
                ));
            goto cleanUp;
            }
        }

    // Done
    rc = true;

cleanUp:
    delete [] pSector;
    return rc;
}

//------------------------------------------------------------------------------

