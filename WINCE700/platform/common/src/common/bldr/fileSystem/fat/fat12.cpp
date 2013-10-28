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
#include "fat12.hpp"
#include "file.hpp"
#include "utils.hpp"
#include <bootBlockBios.h>
#include <bootLog.h>

using namespace ceboot;

//------------------------------------------------------------------------------

#pragma pack(push, 1)

typedef struct DirEntry_t {
    uint8_t  fileName[11];                  // 00
    uint8_t  attrib;                        // 0B
    uint8_t  reserved1;                     // 0C
    uint8_t  createTimeFine;                // 0D
    uint16_t createTime;                    // 0E
    uint16_t createDate;                    // 10
    uint16_t accessData;                    // 12
    uint16_t firstClusterHi;                // 14
    uint16_t modifyTime;                    // 16
    uint16_t modifyDate;                    // 18
    uint16_t firstCluster;                  // 1A
    uint32_t fileSize;                      // 1C
} DirEntry_t;

#pragma pack(pop)

//------------------------------------------------------------------------------

BootFileSystemFat12_t*
BootFileSystemFat12_t::
Create(
    BootBlockSegment_t* pSegment,
    size_t sectorSize
    )
{
    BootFileSystemFat12_t* pThis = NULL;

    pThis = new BootFileSystemFat12_t();
    if ((pThis != NULL) && !pThis->Init(pSegment, sectorSize))
        {
        delete pThis;
        pThis = NULL;
        }
    return pThis;
}

//------------------------------------------------------------------------------

BootFileSystemFat12_t::
BootFileSystemFat12_t(
    ) : m_pSegment(NULL), m_pSector(NULL)
{
}

//------------------------------------------------------------------------------

BootFileSystemFat12_t::
~BootFileSystemFat12_t(
    )
{
    Flush();
    if (m_pSegment != NULL) m_pSegment->DeInit();
    delete m_pSector;
}

//------------------------------------------------------------------------------

bool_t
BootFileSystemFat12_t::
Init(
    BootBlockSegment_t* pSegment,
    size_t sectorSize
    )
{
    bool rc = false;

    // Check & save parameters
    if ((pSegment == NULL) || (sectorSize == 0)) goto cleanUp;
    m_pSegment = pSegment;
    m_sectorSize = sectorSize;
    
    // Allocate sector buffer
    m_pSector = new uint8_t[m_sectorSize];
    if (m_pSector == NULL) goto cleanUp;
    m_sector = (size_t)-1;
    m_sectorDirty = false;

    // Read boot record containing parameter block
    if (!ReadToBuffer(0)) goto cleanUp;

    // Get FAT parameters
    BootBlockBiosParameterBlockType1_t* pBpb;
    pBpb = (BootBlockBiosParameterBlockType1_t*)&m_pSector[3];

    // Verify that this is really FAT12 flawor
    if (memcmp(
            pBpb->fatType, "FAT12   ", sizeof(pBpb->fatType)
            ) != 0) goto cleanUp;

    m_sectorsPerCluster = pBpb->sectorsPerCluster;
    m_fatSector = pBpb->reservedSectors;
    m_sectorsPerFat = pBpb->sectorsPerFat;
    if (pBpb->numberOfFats == 0)
        {
        // Restore FAT from backup copy
        m_fats = 2;
        if (!SyncFat(
             m_fatSector, m_fatSector + m_sectorsPerFat, m_sectorsPerFat
             )) goto cleanUp;
        pBpb->numberOfFats = 2;
        m_sectorDirty = true;
        }
    else
        {
        m_fats = pBpb->numberOfFats;
        if (m_fats == 2)
            {
            // Create backup FAT copy
            if (!SyncFat(
                 m_fatSector + m_sectorsPerFat, m_fatSector,
                 m_sectorsPerFat
                 )) goto cleanUp;
            }
        }
    m_rootDirSector = m_fatSector;
    m_rootDirSector += pBpb->numberOfFats * pBpb->sectorsPerFat;
    m_rootDirEntries = pBpb->numberOfRootEntries;
    m_clusterSector = m_rootDirSector + Items(
        m_rootDirEntries * sizeof(DirEntry_t), m_sectorSize
        );

    // We may update MBR so write it back...
    if (!FlushFromBuffer()) goto cleanUp;

    // Done
    rc = true;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootFileSystemFat12_t::
ReadToBuffer(
    size_t sector
    )
{
    bool_t rc = false;

    if (sector != m_sector)
        {
        if (!FlushFromBuffer()) goto cleanUp;
        if (!m_pSegment->Read(sector, 1, m_pSector))
            {
            m_sector = (size_t)-1;
            goto cleanUp;
            }
        m_sector = sector;
        }

    rc = true;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootFileSystemFat12_t::
FlushFromBuffer(
    )
{
    bool_t rc = false;

    if (m_sectorDirty)
        {
        if (!m_pSegment->Write(m_sector, 1, m_pSector)) goto cleanUp;
        m_sectorDirty = false;
        }

    rc = true;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootFileSystemFat12_t::
SyncFat(
    size_t sectorTo,
    size_t sectorFrom,
    size_t sectors
    )
{
    bool_t rc = false;
    uint8_t* pFrom = new uint8_t[m_sectorSize];
    uint8_t* pTo = new uint8_t[m_sectorSize];

    if ((pFrom == NULL) || (pTo == NULL)) goto cleanUp;

    for (size_t ix = 0; ix < sectors; ix++)
        {
        if (!m_pSegment->Read(sectorFrom + ix, 1, pFrom)) goto cleanUp;
        if (!m_pSegment->Read(sectorTo + ix, 1, pTo)) goto cleanUp;
        if (memcmp(pFrom, pTo, m_sectorSize) == 0) continue;
        if (!m_pSegment->Write(sectorTo + ix, 1, pFrom)) goto cleanUp;
        }

    rc = true;

cleanUp:
    delete pTo;
    delete pFrom;
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootFileSystemFat12_t::
NormalizeName(
    wcstring_t fileName,
    char* pDirName
    )
{
    bool_t rc = false;
    size_t ix, iy;

    if (fileName == NULL) goto cleanUp;

    memset(pDirName, ' ', 11);
    for (ix = 0, iy = 0; iy < 11; ix++, iy++)
        {
        if (fileName[ix] == L'\0') break;
        if (fileName[ix] == L'.')
            {
            while (iy < 8)
                {
                pDirName[iy++] = ' ';
                }
            iy--;
            }
        else if ((fileName[ix] >= L'a') && (fileName[ix] <= L'z'))
            {
            pDirName[iy] = static_cast<char>(fileName[ix] - L'a' + L'A');
            }
        else if ((fileName[ix] > L' ') && (fileName[ix] < 128))
            {
            pDirName[iy] = static_cast<char>(fileName[ix]);
            }
        else goto cleanUp;
        }

    // Fill rest of name with spaces
    while (iy < 11) pDirName[iy++] = ' ';

    // Done
    rc = fileName[ix] == L'\0';

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootFileSystemFat12_t::
FindOnFat(
    wcstring_t fileName,
    size_t& dirCluster,
    enum_t& dirEntry,
    size_t& fileCluster,
    size_t& fileSize,
    bool_t allocate
    )
{
    bool_t rc = false;

    // Normalize file name
    char name[11];
    if (!NormalizeName(fileName, name)) goto cleanUp;

    // Depending on
    size_t dirSector, dirEntries;
    if (dirCluster == 0)
        {
        dirSector = m_rootDirSector;
        dirEntries = m_rootDirEntries;
        }
    else
        {
        dirSector = m_clusterSector + (dirCluster - 2) * m_sectorsPerCluster;
        dirEntries = (m_sectorSize * m_sectorsPerCluster) / sizeof(DirEntry_t);
        }

    dirEntry = 0;
    size_t offset = 0;
    while (dirEntry < dirEntries)
        {
        if (!ReadToBuffer(dirSector)) goto cleanUp;
        DirEntry_t* pEntry = (DirEntry_t*)&m_pSector[offset];

        // Does name match?
        if (pEntry->fileName[0] == '\0') break;
        if (memcmp(pEntry->fileName, name, dimof(name)) == 0)
            {
            fileCluster = pEntry->firstCluster;
            fileSize = pEntry->fileSize;
            rc = true;
            break;
            }

        // Move to next entry
        offset += sizeof(DirEntry_t);
        if (offset >= m_sectorSize)
            {
            dirSector++;
            offset = 0;
            }

        dirEntry++;
        if ((dirEntry >= dirEntries) && (dirCluster != 0))
            {
            size_t nextCluster = NextCluster(dirCluster);
            if (nextCluster == -1) break;
            dirCluster = nextCluster;
            dirSector = m_clusterSector;
            dirSector += (dirCluster - 2) * m_sectorsPerCluster;
            dirEntry = 0;
            }
        }

    // When entry wasn't found and we have create new one...
    if (!rc && allocate)
        {
        // Should we allocate new cluster for directory?
        if (dirEntry >= dirEntries)
            {
            // There isn't way how allocate it on Fat12/16
            if (dirCluster == NULL) goto cleanUp;
            // Allocate new cluster
            size_t next = NextCluster(dirCluster, true);
            if (next == -1) goto cleanUp;
            dirCluster = next;
            dirSector = m_clusterSector;
            dirSector += (dirCluster - 2) * m_sectorsPerCluster;
            dirEntry = 0;
            }
        if (!ReadToBuffer(dirSector)) goto cleanUp;
        DirEntry_t* pEntry = (DirEntry_t*)&m_pSector[offset];
        memset(pEntry, 0, sizeof(DirEntry_t));
        memcpy(pEntry->fileName, name, sizeof(name));
        m_sectorDirty = true;
        fileCluster = 0;
        fileSize = 0;
        rc = true;
        }

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootFileSystemFat12_t::
DeleteDirEntry(
    size_t dirCluster,
    size_t dirEntry
    )
{
    bool_t rc = false;

    // Calculate directory sector
    size_t sector = (dirEntry * sizeof(DirEntry_t)) / m_sectorSize;
    dirEntry -= (sector * m_sectorSize) / sizeof(DirEntry_t);
    if (dirCluster == 0)
        {
        sector += m_rootDirSector;
        }
    else
        {
        sector += m_clusterSector + (dirCluster - 2) * m_sectorsPerCluster;
        }

    // Read sector data
    if (!ReadToBuffer(sector)) goto cleanUp;

    // Update directory entry
    DirEntry_t* pEntry = (DirEntry_t*)m_pSector;
    pEntry[dirEntry].fileName[0] = 0xE5;
    m_sectorDirty = true;

    // Done
    rc = true;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

size_t
BootFileSystemFat12_t::
Next(
    size_t cluster,
    bool_t remove
    )
{
    bool rc = false;
    size_t nextCluster = 0;

    size_t offset = ((cluster << 1) + cluster) >> 1;
    size_t sector = offset / m_sectorSize;
    offset -= sector * m_sectorSize;
    sector += m_fatSector;

    if (!ReadToBuffer(sector)) goto cleanUp;

    if (offset == (m_sectorSize - 1))
        {
        nextCluster = m_pSector[offset];
        if (remove)
            {
            m_pSector[offset] = 0;
            m_sectorDirty = true;
            }

        if (!ReadToBuffer(sector + 1)) goto cleanUp;

        nextCluster |= (m_pSector[0] & 0x0F) << 8;
        if (remove)
            {
            m_pSector[0] &= ~0x0F;
            m_sectorDirty = true;
            }
        }
    else if ((cluster & 0x1) != 0)
        {
        nextCluster = *((uint16_t*)&m_pSector[offset]) >> 4;
        if (remove)
            {
            m_pSector[offset] &= ~0xF0;
            m_pSector[offset + 1] = 0;
            m_sectorDirty = true;
            }
        }
    else
        {
        nextCluster = *((uint16_t*)&m_pSector[offset]) & 0x0FFF;
        if (remove)
            {
            m_pSector[offset] = 0;
            m_pSector[offset + 1] &= ~0xF0;
            m_sectorDirty = true;
            }
        }

    // Done
    rc = (cluster >= 2) && (cluster <= 0x0FEF);

cleanUp:
    return rc ? nextCluster : -1;
}

//------------------------------------------------------------------------------

size_t
BootFileSystemFat12_t::
Allocate(
    size_t cluster
    )
{
    UNREFERENCED_PARAMETER(cluster);
    return (size_t)-1;
}

//------------------------------------------------------------------------------

handle_t
BootFileSystemFat12_t::
Open(
    wcstring_t fileName,
    flags_t access
    )
{
    BootFileSystemFileFat_t *pFile = NULL;
    size_t dirCluster, dirEntry, fileCluster, fileSize;

    bool_t create = (access & BootFileSystem_t::AccessWrite) != 0;
    dirCluster = 0;
    if (!FindOnFat(
            fileName, dirCluster, dirEntry, fileCluster, fileSize, create
            )) goto cleanUp;

    // Create file object
    pFile = BootFileSystemFileFat_t::Create(
        this, access, m_sectorSize, m_sectorsPerCluster, 0, dirEntry,
        fileSize, fileCluster
        );

cleanUp:
    return reinterpret_cast<handle_t>(pFile);
}

//------------------------------------------------------------------------------

bool_t
BootFileSystemFat12_t::
Delete(
    wcstring_t fileName
    )
{
    bool_t rc = false;
    size_t dirCluster, dirEntry, fileCluster, fileSize;

    dirCluster = 0;
    if (!FindOnFat(
            fileName, dirCluster, dirEntry, fileCluster, fileSize
            )) goto cleanUp;

    // First mark entry as deleted
    if (!DeleteDirEntry(dirCluster, dirEntry)) goto cleanUp;

    // Now release all clusters in chain
    if (!ReleaseClusterLink(fileCluster)) goto cleanUp;

    // Make sure that all is written to disk
    Flush();

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootFileSystemFat12_t::
DeInit(
    )
{
    return Release() == 0;
}

//------------------------------------------------------------------------------

bool_t
BootFileSystemFat12_t::
IoCtl(
    enum_t code,
    void *pBuffer,
    size_t size
    )
{
    bool rc = false;

    switch (code)
        {
        case OpenIoCtl:
            {
            OpenParams_t *pInfo;
            pInfo = static_cast<OpenParams_t*>(pBuffer);
            if ((pInfo == NULL) || (size != sizeof(*pInfo))) break;
            pInfo->hFile = Open(pInfo->name, pInfo->access);
            rc = true;
            }
            break;
        case DeleteIoCtl:
            {
            DeleteParams_t *pInfo;
            pInfo = static_cast<DeleteParams_t*>(pBuffer);

            if ((pInfo == NULL) || (size != sizeof(*pInfo))) break;
            rc = Delete(pInfo->name);
            }
            break;
        }

    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootFileSystemFat12_t::
ReadSector(
    size_t cluster,
    size_t sector,
    uint8_t *pBuffer
    )
{
    bool_t rc = false;

    if (cluster < 2) goto cleanUp;
    sector += (cluster - 2) * m_sectorsPerCluster + m_clusterSector;
    if (!m_pSegment->Read(sector, 1, pBuffer)) goto cleanUp;

    rc = true;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootFileSystemFat12_t::
WriteSector(
    size_t cluster,
    size_t sector,
    uint8_t *pBuffer
    )
{
    bool_t rc = false;

    if (cluster < 2) goto cleanUp;
    sector += (cluster - 2) * m_sectorsPerCluster + m_clusterSector;
    if (!m_pSegment->Write(sector, 1, pBuffer)) goto cleanUp;

    rc = true;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootFileSystemFat12_t::
Flush(
    )
{
    return FlushFromBuffer();
}

//------------------------------------------------------------------------------

size_t
BootFileSystemFat12_t::
NextCluster(
    size_t cluster,
    bool_t allocate
    )
{
    size_t nextCluster = 0;

    if (cluster < 2)
        {
        if (allocate) nextCluster = Allocate(cluster);
        }
    else
        {
        nextCluster = Next(cluster, false);
        if (allocate & (nextCluster == -1))
            {
            nextCluster = Allocate(cluster);
            }
        }

    return nextCluster;
}

//------------------------------------------------------------------------------

bool_t
BootFileSystemFat12_t::
ReleaseClusterLink(
    size_t cluster
    )
{
    while (cluster != -1)
        {
        cluster = Next(cluster, true);
        }
    return true;
}

//------------------------------------------------------------------------------

bool_t
BootFileSystemFat12_t::
UpdateDirEntry(
    size_t dirCluster,
    size_t dirEntry,
    size_t fileCluster,
    size_t fileSize
    )
{
    bool_t rc = false;

    // Calculate directory sector
    size_t sector = (dirEntry * sizeof(DirEntry_t)) / m_sectorSize;
    dirEntry -= (sector * m_sectorSize) / sizeof(DirEntry_t);
    if (dirCluster == 0)
        {
        sector += m_rootDirSector;
        }
    else
        {
        sector += m_clusterSector + (dirCluster - 2) * m_sectorsPerCluster;
        }

    // Read sector data
    if (!ReadToBuffer(sector)) goto cleanUp;

    // Update directory entry
    DirEntry_t* pEntry = (DirEntry_t*)m_pSector;
    pEntry[dirEntry].firstCluster = (uint16_t)fileCluster;
    pEntry[dirEntry].firstClusterHi = (uint16_t)(fileCluster >> 16);
    pEntry[dirEntry].fileSize = (uint32_t)fileSize;
    m_sectorDirty = true;

    // Done
    rc = true;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

