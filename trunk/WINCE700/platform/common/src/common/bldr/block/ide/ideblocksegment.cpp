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
#include "ideBlock.hpp"
#include "ideBlockSegment.hpp"
#include <bootLog.h>

using namespace ceboot;

//------------------------------------------------------------------------------

#define BOOT_SIZE                   512

//------------------------------------------------------------------------------

#pragma pack(push, 1)

typedef struct BiosParameterBlock_t {
    char     versionId[8];
    uint16_t bytesPerSector;
    uint8_t  sectorsPerCluster;
    uint16_t reservedSectors;
    uint8_t  numberOfFats;
    uint16_t numberOfRootEntries;
    uint16_t sectorsPerPartition;
    uint8_t  mediaDescriptor;
    uint16_t sectorsPerFat;
    uint16_t sectorsPerTrack;
    uint16_t numberOfHeads;
    uint32_t hiddenSectors;
    uint32_t totalSectors;
    uint8_t  driveId;
    uint8_t  tempValue;
    uint8_t  extRecordSig;
    uint32_t volumeSerialNumber;
    char     volumeLabel[11];
    char     fatType[8];
} BiosParameterBlock_t;

#pragma pack(pop)

//------------------------------------------------------------------------------

BootBlockSegmentIde_t::
BootBlockSegmentIde_t(
    BootBlockIde_t *pBlock
    )
{
    m_pBlock = pBlock;
    pBlock->AddRef();
}

//------------------------------------------------------------------------------

BootBlockSegmentIde_t::
~BootBlockSegmentIde_t(
    )
{
}

//------------------------------------------------------------------------------

bool_t
BootBlockSegmentIde_t::
DeInit(
    )
{
    m_pBlock->Release();
    delete this;
    return true;
}

//------------------------------------------------------------------------------

bool_t
BootBlockSegmentIde_t::
IoCtl(
    enum_t code,
    void *pBuffer,
    size_t size
    )
{
    bool rc = false;

    switch (code)
        {
        case ReadIoCtl:
            {
            ReadParams_t *pInfo;
            pInfo = static_cast<ReadParams_t*>(pBuffer);
            if ((pInfo == NULL) || (size != sizeof(*pInfo))) break;
            rc = Read(pInfo->sector, pInfo->sectors, pInfo->pBuffer);
            }
            break;
        case WriteIoCtl:
            {
            WriteParams_t *pInfo;
            pInfo = static_cast<WriteParams_t*>(pBuffer);
            if ((pInfo == NULL) || (size != sizeof(*pInfo))) break;
            rc = Write(pInfo->sector, pInfo->sectors, pInfo->pBuffer);
            }
            break;
        case EraseIoCtl:
            if (pBuffer != NULL) break;
            rc = Erase();
            break;
        case DataSectorsIoCtl:
            {
            DataSectorsParams_t *pInfo;
            pInfo = static_cast<DataSectorsParams_t*>(pBuffer);
            if ((pInfo == NULL) || (size != sizeof(*pInfo))) break;
            rc = DataSectors(&pInfo->sector, &pInfo->sectors);
            }
            break;
        }

    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootBlockSegmentIde_t::
OpenBinaryRegion(
    enum_t index
    )
{
    return m_pBlock->QueryBinaryRegion(index, m_sector, m_sectors);
}

//------------------------------------------------------------------------------

bool_t
BootBlockSegmentIde_t::
OpenReservedRegion(
    const char *name
    )
{
    return m_pBlock->QueryReservedRegion(name, m_sector, m_sectors);
}

//------------------------------------------------------------------------------

bool_t
BootBlockSegmentIde_t::
OpenPartition(
    uint8_t fileSystem,
    enum_t index
    )
{
    return m_pBlock->QueryPartition(fileSystem, index, m_sector, m_sectors);
}

//------------------------------------------------------------------------------

bool_t
BootBlockSegmentIde_t::
Read(
    size_t sector,
    size_t sectors,
    uint8_t *pBuffer
    )
{
    bool_t rc = false;

    if (sector > m_sectors) goto cleanUp;
    if (sectors > (m_sectors - sector)) goto cleanUp;

    sector += m_sector;    
    rc = m_pBlock->Read(sector, sectors, pBuffer);

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootBlockSegmentIde_t::
Write(
    size_t sector,
    size_t sectors,
    uint8_t *pBuffer
    )
{
    bool_t rc = false;

    if (sector > m_sectors) goto cleanUp;
    if (sectors > (m_sectors - sector)) goto cleanUp;

    sector += m_sector;        
    rc = m_pBlock->Write(sector, sectors, pBuffer);

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootBlockSegmentIde_t::
Erase(
    )
{
    return m_pBlock->Erase(m_sector, m_sectors);
}

//------------------------------------------------------------------------------

bool_t
BootBlockSegmentIde_t::
DataSectors(
    size_t *pSector,
    size_t *pSectors
    )
{
    bool_t rc = false;
    uint8_t *pBuffer = NULL;
    BiosParameterBlock_t *pBPB;

    size_t sectorSize = m_pBlock->SectorSize();
    if (sectorSize == 0) goto cleanUp;

    pBuffer = new uint8_t[sectorSize];
    if (pBuffer == NULL) goto cleanUp;

    if (!m_pBlock->Read(m_sector, 1, pBuffer)) goto cleanUp;

    // This shouldn't happen, but who knows...
    if (pBuffer[BOOT_SIZE - 2] != 0x55) goto cleanUp;
    if (pBuffer[BOOT_SIZE - 1] != 0xAA) goto cleanUp;

    // Verify BPB
    pBPB = reinterpret_cast<BiosParameterBlock_t *>(&pBuffer[3]);

#if 0
    BootLog(L"bytesPerSector: %04x\r\n", pBPB->bytesPerSector);
    BootLog(L"sectorsPerCluster: %02x\r\n", pBPB->sectorsPerCluster);
    BootLog(L"reservedSectors: %04x\r\n", pBPB->reservedSectors);
    BootLog(L"numberOfFats: %02x\r\n", pBPB->numberOfFats);
    BootLog(L"numberOfRootEntries: %04x\r\n", pBPB->numberOfRootEntries);
    BootLog(L"sectorsPerPartition: %04x\r\n", pBPB->sectorsPerPartition);
    BootLog(L"mediaDescriptor: %02x\r\n", pBPB->mediaDescriptor);
    BootLog(L"sectorsPerFat: %04x\r\n", pBPB->sectorsPerFat);
    BootLog(L"sectorsPerTrack: %04x\r\n", pBPB->sectorsPerTrack);
    BootLog(L"numberOfHeads: %04x\r\n", pBPB->numberOfHeads);
    BootLog(L"hiddenSectors: %08x\r\n", pBPB->hiddenSectors);
    BootLog(L"totalSectors: %08x\r\n", pBPB->totalSectors);
    BootLog(L"driveId: %02x\r\n", pBPB->driveId);
#endif
    
    if (pBPB->bytesPerSector != sectorSize) goto cleanUp;
    if (pBPB->hiddenSectors != m_sector) goto cleanUp;

    size_t sector = pBPB->reservedSectors;
    sector += pBPB->sectorsPerFat * pBPB->numberOfFats;
    sector += (pBPB->numberOfRootEntries * 0x20) / sectorSize;
    
    *pSector = sector;
    *pSectors = pBPB->totalSectors - sector;

    rc = true;
    
cleanUp:
    delete [] pBuffer;
    return rc;
}

//------------------------------------------------------------------------------

