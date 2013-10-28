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
#include "fat32.hpp"
#include "file.hpp"
#include "utils.hpp"
#include <bootBlockBios.h>
#include <bootLog.h>

using namespace ceboot;

//------------------------------------------------------------------------------

BootFileSystemFat32_t*
BootFileSystemFat32_t::
Create(
    BootBlockSegment_t* pSegment,
    size_t sectorSize
    )
{
    BootFileSystemFat32_t* pThis = NULL;

    pThis = new BootFileSystemFat32_t();
    if ((pThis != NULL) && !pThis->Init(pSegment, sectorSize))
        {
        delete pThis;
        pThis = NULL;
        }
    return pThis;
}

//------------------------------------------------------------------------------

BootFileSystemFat32_t::
BootFileSystemFat32_t(
    ) : m_pSegment(NULL), m_pSector(NULL)
{
}

//------------------------------------------------------------------------------

BootFileSystemFat32_t::
~BootFileSystemFat32_t(
    )
{
    Flush();
    if (m_pSegment != NULL) m_pSegment->DeInit();
    delete m_pSector;
}

//------------------------------------------------------------------------------

bool_t
BootFileSystemFat32_t::
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
    BootBlockBiosParameterBlockType2_t* pBpb;
    pBpb = (BootBlockBiosParameterBlockType2_t*)&m_pSector[3];

    // Verify that this is really FAT32 flawor
    if (memcmp(
            pBpb->fatType, "FAT32   ", sizeof(pBpb->fatType)
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

    m_rootDirCluster = pBpb->rootDirectoryCluster;
    m_clusterSector = m_fatSector + m_fats * m_sectorsPerFat;

    // We may update MBR so write it back...
    if (!FlushBuffer()) goto cleanUp;

    // Done
    rc = true;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootFileSystemFat32_t::
FlushBuffer(
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
BootFileSystemFat32_t::
ReadToBuffer(
    size_t sector
    )
{
    bool_t rc = false;

    if (sector != m_sector)
        {
        if (!FlushBuffer()) goto cleanUp;
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

uint32_t*
BootFileSystemFat32_t::
FatLink(
    size_t cluster 
    )
{
    uint32_t* pLink = NULL;

    size_t offset = cluster << 2;
    size_t sector = offset / m_sectorSize;
    offset -= sector * m_sectorSize;
    sector += m_fatSector;
    if (!ReadToBuffer(sector)) goto cleanUp;
    pLink = (uint32_t*)&m_pSector[offset];

cleanUp:    
    return pLink;
}

//------------------------------------------------------------------------------

void
BootFileSystemFat32_t::
FatLinkDirty(
    size_t cluster 
    )
{
    UNREFERENCED_PARAMETER(cluster);
    m_sectorDirty = true;
}

//------------------------------------------------------------------------------

uint8_t*
BootFileSystemFat32_t::
DataSector(
    size_t cluster, 
    size_t sector
    )
{
    sector += m_clusterSector + ((cluster - 2) * m_sectorsPerCluster);
    bool_t rc = ReadToBuffer(sector);
    return rc ? m_pSector : NULL;
}

//------------------------------------------------------------------------------

void
BootFileSystemFat32_t::
DataSectorDirty(
    size_t cluster, 
    size_t sector
    )
{
    UNREFERENCED_PARAMETER(cluster);
    UNREFERENCED_PARAMETER(sector);
    m_sectorDirty = true;
}

//------------------------------------------------------------------------------

bool_t
BootFileSystemFat32_t::
DirEntry(
    size_t dirCluster,
    size_t dirEntry,
    DirEntry_t** ppEntry
    )
{
    bool_t rc = false;

    size_t offset = dirEntry << 5;
    size_t sector = offset / m_sectorSize;
    offset -= sector * m_sectorSize;
    sector += (dirCluster - 2) * m_sectorsPerCluster + m_clusterSector;

    if (!ReadToBuffer(sector)) goto cleanUp;

    *ppEntry = (DirEntry_t*)&m_pSector[offset];

    rc = true;
    
cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

void
BootFileSystemFat32_t::
DirEntryDirty(
    size_t dirCluster,
    size_t dirEntry
    )
{
    UNREFERENCED_PARAMETER(dirCluster);
    UNREFERENCED_PARAMETER(dirEntry);
    m_sectorDirty = true;
}

//------------------------------------------------------------------------------

bool_t
BootFileSystemFat32_t::
NextDirEntry(
    size_t& dirCluster,
    size_t& dirEntry,
    bool_t allocate
    )
{
    bool_t rc = false;
    size_t nextEntry = dirEntry + 1;

    size_t offset = nextEntry << 5;
    size_t sector = offset / m_sectorSize;
    
    if (sector >= m_sectorsPerCluster)
        {
        size_t nextCluster = Next(dirCluster, false);
        if ((nextCluster == -1) && !allocate) goto cleanUp;
        nextCluster = Allocate(dirCluster);
        if (nextCluster == -1) goto cleanUp;
        for (size_t sector = 0; sector < m_sectorsPerCluster; sector++)
            {
            uint8_t* pSector = DataSector(nextCluster, sector);
            if (pSector == NULL) goto cleanUp;
            memset(pSector, 0, m_sectorSize);
            DataSectorDirty(nextCluster, sector);
            }
        nextEntry = 0;
        dirCluster = nextCluster;
        }
    dirEntry = nextEntry;
    rc = true;
    
cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootFileSystemFat32_t::
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
BootFileSystemFat32_t::
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
BootFileSystemFat32_t::
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
    size_t cluster = dirCluster;
    size_t entry = 0;
    do
        {
        DirEntry_t* pEntry;
        if (!DirEntry(cluster, entry, &pEntry)) goto cleanUp;

        // Does name match?
        if (pEntry->fileName[0] == '\0') break;
        if (memcmp(pEntry->fileName, name, dimof(name)) == 0)
            {
            dirCluster = cluster;
            dirEntry = entry;
            fileCluster = pEntry->firstCluster;
            fileSize = pEntry->fileSize;
            rc = true;
            break;
            }
        }
    while (NextDirEntry(cluster, entry, false));

    // When entry wasn't found and we have create new one...
    if (!rc && allocate)
        {
        cluster = dirCluster;
        entry = 0;
        
        DirEntry_t* pEntry;
        do
            {
            if (!DirEntry(cluster, entry, &pEntry)) goto cleanUp;

            // Does name match?
            if ((pEntry->fileName[0] == '\0') || (pEntry->fileName[0] == 0xE5))
                {
                rc = true;
                break;
                }
            }
        while (NextDirEntry(cluster, entry, true));
        if (!rc) goto cleanUp;
        
        memset(pEntry, 0, sizeof(DirEntry_t));
        memcpy(pEntry->fileName, name, sizeof(name));
        DirEntryDirty(cluster, entry);
        fileCluster = 0;
        fileSize = 0;
        rc = true;
        }

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootFileSystemFat32_t::
DeleteDirEntry(
    size_t dirCluster,
    size_t dirEntry
    )
{
    bool_t rc = false;
    DirEntry_t* pEntry;

    if (!DirEntry(dirCluster, dirEntry, &pEntry)) goto cleanUp;
    pEntry[dirEntry].fileName[0] = 0xE5;
    DirEntryDirty(dirCluster, dirEntry);

    // Done
    rc = true;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

size_t
BootFileSystemFat32_t::
Next(
    size_t cluster,
    bool_t remove
    )
{
    bool rc = false;

    UNREFERENCED_PARAMETER(remove);

    uint32_t* pLink = FatLink(cluster);
    if (pLink == NULL) goto cleanUp;

    cluster = *pLink;
    rc = (cluster >= 2) && (cluster <= 0x0FFFFFEF);

cleanUp:
    return rc ? cluster : -1;
}

//------------------------------------------------------------------------------

size_t
BootFileSystemFat32_t::
Allocate(
    size_t cluster
    )
{
    bool rc = false;
    size_t sector, offset = 0, freeCluster;

    freeCluster = 2;
    for (sector = 0; sector < m_sectorsPerFat; sector++)
        {
        if (!ReadToBuffer(m_fatSector + sector)) goto cleanUp;
        offset = (sector == 0) ? 8 : 0;
        while (offset < m_sectorSize)
            {
            if (*(uint32_t*)&m_pSector[offset] == 0) break;
            offset += sizeof(uint32_t);
            freeCluster++;
            }
        if (offset < m_sectorSize) break;
        }

    // If we didn't find empty cluster there isn't room for new data
    if (sector >= m_sectorsPerFat) goto cleanUp;

    // Mark cluster as last in file
    *(uint32_t*)&m_pSector[offset] = 0x0FFFFFFF;
    m_sectorDirty = true;

    // Update previous cluster link
    if (cluster > 1)
        {
        offset = cluster << 2;
        sector = offset / m_sectorSize;
        offset -= sector * m_sectorSize;
        if (!ReadToBuffer(m_fatSector + sector)) goto cleanUp;
        *(uint32_t*)&m_pSector[offset] = (uint16_t)freeCluster;
        m_sectorDirty = true;
        }

    // Done
    rc = true;

cleanUp:
    return rc ? freeCluster : -1;
}

//------------------------------------------------------------------------------

handle_t
BootFileSystemFat32_t::
Open(
    wcstring_t fileName,
    flags_t access
    )
{
    BootFileSystemFileFat_t *pFile = NULL;
    bool_t create = (access & BootFileSystem_t::AccessWrite) != 0;
    size_t dirCluster = m_rootDirCluster;
    size_t dirEntry = 0;
    size_t fileCluster, fileSize;

    // Find or create file...
    if (!FindOnFat(
            fileName, dirCluster, dirEntry, fileCluster, fileSize, create
            )) goto cleanUp;

    // Create file object
    pFile = BootFileSystemFileFat_t::Create(
        this, access, m_sectorSize, m_sectorsPerCluster, dirCluster, dirEntry,
        fileSize, fileCluster
        );

cleanUp:
    return reinterpret_cast<handle_t>(pFile);
}

//------------------------------------------------------------------------------

bool_t
BootFileSystemFat32_t::
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

size_t
BootFileSystemFat32_t::
FreeSpace(
    )
{
    size_t freeSpace = (size_t)-1;


    size_t clusters = 0;
    for (size_t sector = 0; sector < m_sectorsPerFat; sector++)
        {
        if (!ReadToBuffer(m_fatSector + sector)) goto cleanUp;
        size_t offset = (sector == 0) ? 8 : 0;
        while (offset < m_sectorSize)
            {
            if (*(uint32_t*)&m_pSector[offset] == 0) clusters++;
            offset += sizeof(uint32_t);
            }
        }

    freeSpace = clusters * m_sectorsPerCluster * m_sectorSize;
    
cleanUp:
    return freeSpace;
}

//------------------------------------------------------------------------------

bool_t
BootFileSystemFat32_t::
DeInit(
    )
{
    return Release() == 0;
}

//------------------------------------------------------------------------------

bool_t
BootFileSystemFat32_t::
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
            OpenParams_t* pInfo = (OpenParams_t *)pBuffer;
            if ((pInfo == NULL) || (size != sizeof(*pInfo))) break;
            pInfo->hFile = Open(pInfo->name, pInfo->access);
            rc = true;
            }
            break;
        case DeleteIoCtl:
            {
            DeleteParams_t* pInfo = (DeleteParams_t *)pBuffer;
            if ((pInfo == NULL) || (size != sizeof(*pInfo))) break;
            rc = Delete(pInfo->name);
            }
            break;
        case FreeSpaceIoCtl:
            {
            FreeSpaceParams_t* pInfo = (FreeSpaceParams_t *)pBuffer;
            if ((pInfo == NULL) || (size != sizeof(*pInfo))) break;
            pInfo->freeSpace = FreeSpace();
            rc = true;
            }
            break;
        }

    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootFileSystemFat32_t::
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
BootFileSystemFat32_t::
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
BootFileSystemFat32_t::
Flush(
    )
{
    return FlushBuffer();
}

//------------------------------------------------------------------------------

size_t
BootFileSystemFat32_t::
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
BootFileSystemFat32_t::
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
BootFileSystemFat32_t::
UpdateDirEntry(
    size_t dirCluster,
    size_t dirEntry,
    size_t fileCluster,
    size_t fileSize
    )
{
    bool_t rc = false;
    DirEntry_t* pEntry;

    if (!DirEntry(dirCluster, dirEntry, &pEntry)) goto cleanUp;
    pEntry->firstCluster = (uint16_t)fileCluster;
    pEntry->firstClusterHi = (uint16_t)(fileCluster >> 16);
    pEntry->fileSize = (uint32_t)fileSize;
    DirEntryDirty(dirCluster, dirEntry);

    // Done
    rc = true;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

