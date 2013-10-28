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
//#include <bootBlockBios.h>
#include <bootLog.h>
#include "exFat.hpp"
#include "file.hpp"

using namespace ceboot;

//------------------------------------------------------------------------------

BootFileSystemExFat_t*
BootFileSystemExFat_t::
Create(
    BootBlockSegment_t* pSegment,
    size_t sectorSize
    )
{
    BootFileSystemExFat_t* pThis = NULL;

    pThis = new BootFileSystemExFat_t();
    if ((pThis != NULL) && !pThis->Init(pSegment, sectorSize))
        {
        delete pThis;
        pThis = NULL;
        }
    return pThis;
}

//------------------------------------------------------------------------------

BootFileSystemExFat_t::
BootFileSystemExFat_t(
    ) : m_pSegment(NULL), m_pSector(NULL), m_bitmapCluster(0), m_bitmapSize(0),
        m_upCaseTableCluster(0), m_upCaseTableSize(0)
{
}

//------------------------------------------------------------------------------

BootFileSystemExFat_t::
~BootFileSystemExFat_t(
    )
{
    Flush();
    if (m_pSegment != NULL) m_pSegment->DeInit();
    delete m_pSector;
}

//------------------------------------------------------------------------------

bool_t
BootFileSystemExFat_t::
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

    // Get BPB parameters
    BootBlockBiosParameterBlockTypeEx_t* pBpb;
    pBpb = (BootBlockBiosParameterBlockTypeEx_t*)&m_pSector[3];

    // Verify that this is really exFAT flawor
    if (memcmp(
            pBpb->versionId, "EXFAT   ", sizeof(pBpb->versionId)
            ) != 0) goto cleanUp;

    if ((pBpb->fileSystemVersion & 0xFF00) != 0x0100) goto cleanUp;
    if (m_sectorSize != (size_t)(1 << pBpb->bytesPerSectorLog2)) goto cleanUp;
    m_sectorSizeLog2 = pBpb->bytesPerSectorLog2;
    m_sectorsPerCluster = 1 << pBpb->sectorsPerClusterLog2;
    m_sectorsPerClusterLog2 = pBpb->sectorsPerClusterLog2;

    m_fatSector = pBpb->fatOffset;
    m_sectorsPerFat = pBpb->fatLength;
    if (pBpb->numberOfFats == 2)
        {
        // ToDo: Check consistency
        }
    else if (pBpb->numberOfFats != 1)
        {
        goto cleanUp;
        }

    m_fats = pBpb->numberOfFats;
    m_rootDirCluster = pBpb->firstRootDirCluster;
    m_clusterSector = pBpb->clusterHeapOffset;
    m_clusters = pBpb->clusterCount;

    // Walk root directory for required entries    
    size_t cluster = m_rootDirCluster;
    size_t entry = 0;
    do
        {
        DirEntry_t* pEntry;
        if (!DirEntry(cluster, entry, &pEntry)) goto cleanUp;
        switch (pEntry->type)
            {
            case 0x81:      // Allocation bitmap
                // There shouldn't be another bitmap
                if (m_bitmapCluster != 0) goto cleanUp;
                // We don't support TexFAT yet...
                if ((pEntry->x81.flags & 0x01) != 0) goto cleanUp;
                // Bitmap bigger than 4GB isn't for us either
                if (pEntry->x81.sizeHi != 0) goto cleanUp;
                // Save cluster and size
                m_bitmapCluster = pEntry->x81.cluster;
                m_bitmapSize = pEntry->x81.sizeLo;
                break;
            case 0x82:      // Up-case table
                // There should be only one up case table
                if (m_upCaseTableCluster != 0) goto cleanUp;
                // Up case table bigger than 4GB isn't for us
                if (pEntry->x82.sizeHi != 0) goto cleanUp;
                // Save cluster/size/checksum
                m_upCaseTableCluster = pEntry->x82.cluster;
                m_upCaseTableSize = pEntry->x82.sizeLo;
                m_upCaseTableSum = pEntry->x82.checksum;
                break;
            }                        
        }
    while (NextDirEntry(cluster, entry, false));
    
    // At this moment we have to know them...
    if ((m_bitmapCluster == 0) || (m_upCaseTableCluster == 0)) goto cleanUp;
    
    // Done
    rc = true;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootFileSystemExFat_t::
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
BootFileSystemExFat_t::
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

uint8_t*
BootFileSystemExFat_t::
DataSector(
    size_t cluster, 
    size_t sector
    )
{
    sector += m_clusterSector + ((cluster - 2) << m_sectorsPerClusterLog2);
    bool_t rc = ReadToBuffer(sector);
    return rc ? m_pSector : NULL;
}

//------------------------------------------------------------------------------

void
BootFileSystemExFat_t::
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
BootFileSystemExFat_t::
NextDataSector(
    size_t& cluster,
    size_t& sector,
    bool_t allocate
    )
{
    bool_t rc = false;

    size_t nextSector = sector + 1;
    if (nextSector >= m_sectorsPerCluster)
        {
        size_t nextCluster = NextCluster(cluster);
        if ((nextCluster == -1) && !allocate) goto cleanUp;
        nextCluster = Allocate(cluster);
        if (nextCluster == -1) goto cleanUp;
        nextSector = 0;
        }
    sector = nextSector;
    rc = true;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootFileSystemExFat_t::
NextDirEntry(
    size_t& dirCluster,
    size_t& dirEntry,
    bool_t allocate
    )
{
    bool_t rc = false;
    size_t nextEntry = dirEntry + 1;
    if ((nextEntry >> (m_sectorsPerClusterLog2 + m_sectorSizeLog2 - 5)) > 0)
        {
        size_t nextCluster = Next(dirCluster);
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
BootFileSystemExFat_t::
DirEntry(
    size_t dirCluster,
    size_t dirEntry,
    DirEntry_t** ppEntry
    )
{
    bool_t rc = false;

    size_t sector = (dirCluster - 2) << m_sectorsPerClusterLog2;
    sector += m_clusterSector;
    sector += dirEntry >> (m_sectorSizeLog2 - 5);
    if (!ReadToBuffer(sector)) goto cleanUp;
    *ppEntry = (DirEntry_t*)&m_pSector[(dirEntry << 5) & (m_sectorSize - 1)];

    rc = true;
    
cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

void
BootFileSystemExFat_t::
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

uint32_t*
BootFileSystemExFat_t::
FatLink(
    size_t cluster 
    )
{
    uint32_t* pLink = NULL;

    size_t offset = (cluster - 2) << 2;
    size_t sector = (offset >> m_sectorSizeLog2) + m_fatSector;
    offset &= m_sectorSize - 1;
    if (!ReadToBuffer(sector)) goto cleanUp;
    pLink = (uint32_t*)&m_pSector[offset];

cleanUp:    
    return pLink;
}

//------------------------------------------------------------------------------

void
BootFileSystemExFat_t::
FatLinkDirty(
    size_t cluster 
    )
{
    UNREFERENCED_PARAMETER(cluster);
    m_sectorDirty = true;
}

//------------------------------------------------------------------------------

bool_t
BootFileSystemExFat_t::
ToUpper(
    wstring_t string
    )
{
    bool_t rc = false;
    size_t items = m_upCaseTableSize >> 1;
    size_t itemsPerSector = m_sectorSize >> 1;
    uint16_t* pTable;
    size_t cluster = m_upCaseTableCluster;
    size_t sector = 0;
    size_t offset = 0;


    pTable = (uint16_t*)DataSector(cluster, sector);    
    if (pTable == NULL) goto cleanUp;

    wchar_t ch = 0;
    while (!rc && (items > 0))
        {
        if (pTable[offset] == 0xFFFF)
            {
            items--;
            if (items == 0) break;
            if (++offset >= itemsPerSector)
                {
                // Read next sector
                if (!NextDataSector(cluster, sector, false)) goto cleanUp;
                pTable = (uint16_t*)DataSector(cluster, sector);    
                if (pTable == NULL) goto cleanUp;
                offset = 0;
                }
            ch += pTable[offset];
            }
        else
            {
            wchar_t* pos = string;
            rc = true;
            while (*pos != L'\0')
                {
                if (*pos == ch)
                    {
                    *pos = pTable[offset];
                    }
                else if (*pos > ch)
                    {
                    rc = false;
                    }
                pos++;
                }                    
            if (rc) break;
            ch++;
            }
        items--;
        if (++offset >= itemsPerSector)
            {
            if (!NextDataSector(cluster, sector, false)) goto cleanUp;
            pTable = (uint16_t*)DataSector(cluster, sector);    
            if (pTable == NULL) goto cleanUp;
            offset = 0;
            }
        }

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

uint16_t
BootFileSystemExFat_t::
NameHash(
    wcstring_t name
    )
{
    uint16_t hash = 0;
    uint8_t *pData = (uint8_t*)name;
    while (*name != L'\0')
        {
        hash = ((hash & 1) ? 0x8000 : 0) + (hash >> 1) + *pData++;
        hash = ((hash & 1) ? 0x8000 : 0) + (hash >> 1) + *pData++;
        name++;
        }
    return hash;
}

//------------------------------------------------------------------------------

uint16_t
BootFileSystemExFat_t::
DirEntryChecksum(
    uint16_t checksum,
    DirEntry_t* pEntry,
    bool_t primary
    )
{
    uint8_t* pData = (uint8_t*)pEntry;
    for (size_t ix = 0; ix < sizeof(DirEntry_t); ix++)
        {
        if (primary && ((ix == 2) || (ix == 3))) continue;
        checksum = ((checksum & 1) ? 0x8000 : 0) + (checksum >> 1) + pData[ix];
        }
    return checksum;
}

//------------------------------------------------------------------------------

bool_t
BootFileSystemExFat_t::
NextFileDirEntry(
    size_t& dirCluster,
    size_t& dirEntry,
    FileDirEntryEx_t* pFileEntry
    )
{
    bool_t rc = false;
    DirEntry_t* pEntry;

    for (;;)
        {

        // First entry must be directory one
        if (!DirEntry(dirCluster, dirEntry, &pEntry)) goto cleanUp;
        if (pEntry->type != 0x85)
            {
            if (!NextDirEntry(dirCluster, dirEntry, false)) goto cleanUp;
            continue;
            }
        
        // We find possible file directory entry
        pFileEntry->dirCluster = dirCluster;
        pFileEntry->dirEntry = dirEntry;
        pFileEntry->dirEntries = pEntry->x85.follows + 1;

        // Save all we will need from entry
        size_t entries = pEntry->x85.follows;
        uint16_t entryChecksum = pEntry->x85.checksum;

        // We quite don't expect entry without at least one follower
        if (entries == 0)
            {
            if (!NextDirEntry(dirCluster, dirEntry, false)) goto cleanUp;
            continue;
            }
        
        // Calculate checksum from this entry
        uint16_t checksum = DirEntryChecksum(0, pEntry, true);

        // Move to next entry
        if (!NextDirEntry(dirCluster, dirEntry, false)) goto cleanUp;
        if (!DirEntry(dirCluster, dirEntry, &pEntry)) goto cleanUp;
        
        // We have skip all dummy & benign secondary entries 
        while ((pEntry->type == 0xA1) || (pEntry->type >= 0xE0))
            {
            if (entries == 0) break;
            // But count benign secondary to checksum
            if (pEntry->type >= 0xE0)
                {
                checksum = DirEntryChecksum(checksum, pEntry, false);    
                }
            entries--;
            if (!NextDirEntry(dirCluster, dirEntry, false)) goto cleanUp;
            if (!DirEntry(dirCluster, dirEntry, &pEntry)) goto cleanUp;
            }
        
        // We are expecting only 0xC0 entry there
        if ((entries == 0) || (pEntry->type != 0xC0)) continue;

        // Include 0xC0 entry to checksum
        checksum = DirEntryChecksum(checksum, pEntry, false);    
        entries--;

        // Save all we need from 0xC0 entry
        pFileEntry->nameChars = pEntry->xC0.nameChars;
        pFileEntry->cluster = pEntry->xC0.cluster;
        pFileEntry->nameHash = pEntry->xC0.nameHash;
        pFileEntry->size = pEntry->xC0.sizeLo;

        // Move to next entry
        if (!NextDirEntry(dirCluster, dirEntry, false)) goto cleanUp;
        
        // If there are no directory entries try next one...
        if (entries == 0) continue;

        
        // Now we expect name, dummy or benign secondary entries
        size_t nameOffset = 0;
        if (!DirEntry(dirCluster, dirEntry, &pEntry)) goto cleanUp;
        while ((pEntry->type == 0xC1) || (pEntry->type == 0xA1) ||
                (pEntry->type >= 0xE0))
            {
            if (pEntry->type == 0xC1)
                {
                size_t count = pFileEntry->nameChars - nameOffset;
                if (count > 15) count = 15;
                memcpy(
                    &pFileEntry->name[nameOffset], pEntry->xC1.name, count << 1
                    );
                nameOffset += count;
                pFileEntry->name[nameOffset] = L'\0';
                }
            if (pEntry->type != 0xA1)
                {
                checksum = DirEntryChecksum(checksum, pEntry, false);    
                }
            // Move to next entry
            entries--;
            if (entries == 0) break;
            if (!NextDirEntry(dirCluster, dirEntry, false)) goto cleanUp;
            if (!DirEntry(dirCluster, dirEntry, &pEntry)) goto cleanUp;
            }

        // If we didn't get all entries simply skip file entries
        if (entries > 0) continue;        

        // Checksum make sure we get correct entry
        if (checksum == entryChecksum) break;

        // Move to next entry
        if (!NextDirEntry(dirCluster, dirEntry, false)) goto cleanUp;
        }
    
    // Done
    rc = true;
    
cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable:4701)
bool_t
BootFileSystemExFat_t::
AllocateFileDirEntry(
    wcstring_t fileName,
    size_t& dirCluster,
    size_t& dirEntry
    )
{
    bool_t rc = false;
    wchar_t name[256];

    // Make name local copy as we need upcase it...
    size_t nameChars = BootStringCchCopy(name, dimof(name), fileName);
    if (!ToUpper(name)) goto cleanUp;
    uint16_t nameHash = NameHash(name);

    // We will need two entries plus entries for file name 
    size_t dirEntries = 2 + (nameChars + 14) / 15;

    // Look for empty entries
    size_t cluster = dirCluster;
    size_t entry = 0;
    size_t clusterEmpty, entryEmpty, entriesEmpty;
    DirEntry_t* pEntry;
    while (DirEntry(cluster, entry, &pEntry))
        {
        if ((pEntry->type & 0x80) != 0)
            {
            entriesEmpty = 0;
            }
        else if (entriesEmpty == 0)
            {
            clusterEmpty = cluster;
            entryEmpty = entry;
            entriesEmpty = 1;
            }
        else
            {
            entriesEmpty++;
            }
        // We are done when we find enough empty entries
        if (entriesEmpty >= dirEntries) break;
        // Move to next
        if (!NextDirEntry(cluster, entry, true)) goto cleanUp;
        }

    cluster = clusterEmpty;
    entry = entryEmpty;

    // Get first entry
    if (!DirEntry(cluster, entry, &pEntry)) goto cleanUp;
    memset(pEntry, 0, sizeof(*pEntry));
    pEntry->type = 0x85;
    pEntry->x85.follows = (uint8_t)(dirEntries - 1);
    pEntry->x85.attrib = 0;
    pEntry->x85.createTime.year = 20;
    pEntry->x85.modifyTime.year = 20;
    pEntry->x85.accessTime.year = 20;
    DirEntryDirty(cluster, entry);
    uint16_t checksum = DirEntryChecksum(0, pEntry, true);

    // Now get next position for stream entry
    if (!NextDirEntry(cluster, entry, false)) goto cleanUp;
    if (!DirEntry(cluster, entry, &pEntry)) goto cleanUp;
    memset(pEntry, 0, sizeof(*pEntry));
    pEntry->type = 0xC0;
    pEntry->xC0.nameChars = (uint8_t)nameChars;
    pEntry->xC0.nameHash = nameHash;
    DirEntryDirty(cluster, entry);
    checksum = DirEntryChecksum(checksum, pEntry, false);

    // Finally name entries
    size_t offset = 0;
    while (offset < nameChars)
        {
        if (!NextDirEntry(cluster, entry, false)) goto cleanUp;
        if (!DirEntry(cluster, entry, &pEntry)) goto cleanUp;
        memset(pEntry, 0, sizeof(*pEntry));
        pEntry->type = 0xC1;
        size_t count = nameChars - offset;
        if (count > 15) count = 15;
        memcpy(pEntry->xC1.name, &fileName[offset], count << 1);
        offset += count;
        DirEntryDirty(cluster, entry);
        checksum = DirEntryChecksum(checksum, pEntry, false);
        }

    // Now go back and update first entry with checksum
    if (!DirEntry(clusterEmpty, entryEmpty, &pEntry)) goto cleanUp;
    pEntry->x85.checksum = checksum;
    DirEntryDirty(clusterEmpty, entryEmpty);

    // Done
    dirCluster = clusterEmpty;
    dirEntry = entryEmpty;
    rc = true;
    
cleanUp:
    return rc;
}
#pragma warning(pop)

//------------------------------------------------------------------------------

bool_t
BootFileSystemExFat_t::
FindFileDirEntry(
    wcstring_t fileName,
    size_t &dirCluster,
    size_t &dirEntry,
    size_t &fileCluster,
    size_t &fileSize,
    bool_t allocate
    )
{
    bool_t rc = false;
    wchar_t name[256];
    FileDirEntryEx_t entry;

    // Create name local copy & upper case it and calculate hash
    size_t nameChars = BootStringCchCopy(name, dimof(name), fileName);
    if (!ToUpper(name)) goto cleanUp;
    uint16_t nameHash = NameHash(name);
    dirEntry = 0;

    do
        {
        if (!NextFileDirEntry(dirCluster, dirEntry, &entry)) break;
        if (nameHash != entry.nameHash) continue;
        if (nameChars != entry.nameChars) continue;
        if (!ToUpper(entry.name)) goto cleanUp;
        if (memcmp(name, entry.name, nameChars << 1) != 0) continue;
        dirCluster = entry.dirCluster;
        dirEntry = entry.dirEntry;
        fileCluster = entry.cluster;
        fileSize = entry.size;
        rc = true;
        break;
        }
    while (NextDirEntry(dirCluster, dirEntry, false));

    if (!rc && allocate)
        {
        rc = AllocateFileDirEntry(fileName, dirCluster, dirEntry);
        fileCluster = 0;
        fileSize = 0;
        }
    
cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootFileSystemExFat_t::
DeleteDirEntry(
    size_t dirCluster,
    size_t dirEntry
    )
{
    bool_t rc = false;
    DirEntry_t* pEntry;

    if (!DirEntry(dirCluster, dirEntry, &pEntry)) goto cleanUp;
    if (pEntry->type != 0x85) goto cleanUp;
    size_t followEntries = pEntry->x85.follows;

    // Mark entry as deleted
    pEntry->type &= ~0x80;
    DirEntryDirty(dirCluster, dirEntry);
    
    // And now all subsequent entries    
    while (followEntries > 0)
        {
        if (!NextDirEntry(dirCluster, dirEntry, false)) goto cleanUp;
        if (!DirEntry(dirCluster, dirEntry, &pEntry)) goto cleanUp;
        if (pEntry->type == 0xA1) continue;
        // Don't delete what we don't understand...
        if ((pEntry->type != 0xC0) && (pEntry->type != 0xC1) &&
            (pEntry->type < 0xE0)) goto cleanUp;
        pEntry->type &= ~0x80;
        DirEntryDirty(dirCluster, dirEntry);
        followEntries--;    
        }

    // Done
    rc = true;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootFileSystemExFat_t::
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
    if (pEntry->type != 0x85) goto cleanUp;

    size_t cluster = dirCluster;
    size_t entry = dirEntry;
    size_t entries = pEntry->x85.follows;

    // There should be at least one entry
    if (entries == 0) goto cleanUp;
    
    // Calculate checksum from this entry
    uint16_t checksum = DirEntryChecksum(0, pEntry, true);

    // Move to next entry
    if (!NextDirEntry(cluster, entry, false)) goto cleanUp;
    if (!DirEntry(cluster, entry, &pEntry)) goto cleanUp;
        
    // We have skip all dummy & benign secondary entries 
    while ((pEntry->type == 0xA1) || (pEntry->type >= 0xE0))
        {
        if (entries == 0) break;
        // But count benign secondary to checksum
        if (pEntry->type >= 0xE0)
            {
            checksum = DirEntryChecksum(checksum, pEntry, false);    
            }
        entries--;
        if (!NextDirEntry(cluster, entry, false)) goto cleanUp;
        if (!DirEntry(cluster, entry, &pEntry)) goto cleanUp;
        }
       
    // We are expecting only 0xC0 entry there
    if ((entries == 0) || (pEntry->type != 0xC0)) goto cleanUp;
    entries--;
    
    // Update size
    pEntry->xC0.sizeLo = fileSize;
    pEntry->xC0.sizeHi = 0;
    pEntry->xC0.lengthLo = fileSize;
    pEntry->xC0.lengthHi = 0;
    pEntry->xC0.cluster = fileCluster;
    if (fileCluster != 0) pEntry->xC0.flags |= 0x01;
    DirEntryDirty(cluster, entry);

    // Include updated entry to checksum
    checksum = DirEntryChecksum(checksum, pEntry, false);

    // Move to next
    if (entries == 0) goto cleanUp;
    if (!NextDirEntry(cluster, entry, false)) goto cleanUp;
    if (!DirEntry(cluster, entry, &pEntry)) goto cleanUp;
    entries--;

    // Now we expect name, dummy or benign secondary entries
    while ((pEntry->type == 0xC1) || (pEntry->type == 0xA1) ||
            (pEntry->type >= 0xE0))
        {
        if (pEntry->type != 0xA1)
            {
            checksum = DirEntryChecksum(checksum, pEntry, false);    
            }
        // Move to next entry if there is more
        if (entries == 0) break;
        entries--;
        if (!NextDirEntry(cluster, entry, false)) goto cleanUp;
        if (!DirEntry(cluster, entry, &pEntry)) goto cleanUp;
        }

    // If we didn't get all entries simply skip file entries
    if (entries > 0) goto cleanUp;
    
    // Read back first entry
    if (!DirEntry(dirCluster, dirEntry, &pEntry)) goto cleanUp;
    if (pEntry->type != 0x85) goto cleanUp;

    // Update checksum
    pEntry->x85.checksum = checksum;
    DirEntryDirty(dirCluster, dirEntry);

    // Done
    rc = true;
    
cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

size_t
BootFileSystemExFat_t::
Next(
    size_t cluster
    )
{
    bool rc = false;

    uint32_t* pLink = FatLink(cluster);
    if (pLink == NULL) goto cleanUp;

    cluster = *pLink;
    rc = (cluster >= 2) && (cluster <= 0xFFFFFFF6);

cleanUp:
    return rc ? cluster : -1;
}

//------------------------------------------------------------------------------

size_t
BootFileSystemExFat_t::
Allocate(
    size_t cluster
    )
{
    bool rc = false;

    size_t bitmapCluster = m_bitmapCluster;
    size_t bitmapSector = 0;
    size_t bitmapSize = m_bitmapSize;
    uint8_t* pBitmap = NULL;

    size_t offset = 0;
    size_t freeCluster = 2;
    while (bitmapSize > 0)
        {
        // Read bitmap sector
        pBitmap = DataSector(bitmapCluster, bitmapSector);
        if (pBitmap == NULL) goto cleanUp;
        // Look for empty cluster
        offset = 0;
        while (offset < m_sectorSize)
            {
            if (pBitmap[offset] != 0xFF) break;
            offset++;
            freeCluster += 8;
            if (--bitmapSize == 0) goto cleanUp;
            }
        if (offset < m_sectorSize) break;
        if (!NextDataSector(bitmapCluster, bitmapSector, false)) goto cleanUp;
        }

    if (pBitmap == NULL) goto cleanUp;
    
    // Which cluster
    uint8_t mask = 0x01;
    while ((mask & pBitmap[offset]) != 0)
        {
        mask <<= 1;
        freeCluster++;
        }

    // Allocate cluster
    pBitmap[offset] |= mask;
    DataSectorDirty(bitmapCluster, bitmapSector);

    // Set last cluster mark for new cluster
    uint32_t* pLink = FatLink(freeCluster);
    if (pLink == NULL) goto cleanUp;
    *pLink = 0xFFFFFFFF;
    FatLinkDirty(freeCluster);

    // And link it with previous one (if there is one)
    if (cluster >= 2)
        {
        pLink = FatLink(cluster);
        if (pLink == NULL) goto cleanUp;
        *pLink = freeCluster;
        FatLinkDirty(cluster);
        }
    
    // Done
    rc = true;

cleanUp:
    return rc ? freeCluster : -1;
}

//------------------------------------------------------------------------------

bool_t
BootFileSystemExFat_t::
Free(
    size_t cluster
    )
{
    bool rc = false;

    size_t bitmapCluster = m_bitmapCluster;
    size_t bitmapSector = 0;
    uint8_t* pBitmap;

    size_t sector = (cluster - 2) >> (3 + m_sectorSizeLog2);
    size_t offset = (cluster - 2) & (m_sectorSize - 1);
    while (sector > 0)
        {
        if (!NextDataSector(bitmapCluster, bitmapSector, false)) goto cleanUp;
        sector--;
        }

    // Read bitmap sector
    pBitmap = DataSector(bitmapCluster, bitmapSector);
    if (pBitmap == NULL) goto cleanUp;

    // Mark cluster as free
    pBitmap[offset >> 3] &= ~(0x01 << (offset & 0x03));
    DataSectorDirty(bitmapCluster, bitmapSector);

    // Done
    rc = true;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

handle_t
BootFileSystemExFat_t::
Open(
    wcstring_t fileName,
    flags_t access
    )
{
    BootFileSystemFileFat_t *pFile = NULL;
    size_t dirCluster, dirEntry, fileCluster, fileSize;

    bool_t create = (access & BootFileSystem_t::AccessWrite) != 0;
    dirCluster = m_rootDirCluster;
    if (!FindFileDirEntry(
        fileName, dirCluster, dirEntry, fileCluster, fileSize, create
        )) goto cleanUp;

    // Create file object
    pFile = BootFileSystemFileFat_t::Create(
        this, access, m_sectorSize, m_sectorsPerCluster, dirCluster, dirEntry,
        fileSize, fileCluster
        );

cleanUp:
    return (handle_t)pFile;
}

//------------------------------------------------------------------------------

bool_t
BootFileSystemExFat_t::
Delete(
    wcstring_t fileName
    )
{
    bool_t rc = false;
    size_t dirCluster, dirEntry, fileCluster, fileSize;

    dirCluster = 0;
    if (!FindFileDirEntry(
            fileName, dirCluster, dirEntry, fileCluster, fileSize, false
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
BootFileSystemExFat_t::
DeInit(
    )
{
    return Release() == 0;
}

//------------------------------------------------------------------------------

bool_t
BootFileSystemExFat_t::
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
            OpenParams_t *pInfo = (OpenParams_t *)pBuffer;
            if ((pInfo == NULL) || (size != sizeof(*pInfo))) break;
            pInfo->hFile = Open(pInfo->name, pInfo->access);
            rc = true;
            }
            break;
        case DeleteIoCtl:
            {
            DeleteParams_t *pInfo = (DeleteParams_t *)pBuffer;
            if ((pInfo == NULL) || (size != sizeof(*pInfo))) break;
            rc = Delete(pInfo->name);
            }
            break;
        }

    return rc;
}


//------------------------------------------------------------------------------

bool_t
BootFileSystemExFat_t::
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
BootFileSystemExFat_t::
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
BootFileSystemExFat_t::
Flush(
    )
{
    return FlushBuffer();
}

//------------------------------------------------------------------------------

size_t
BootFileSystemExFat_t::
NextCluster(
    size_t cluster,
    bool_t allocate
    )
{
    size_t nextCluster = (size_t)-1;

    if (cluster < 2)
        {
        if (allocate) nextCluster = Allocate(cluster);
        }
    else
        {
        nextCluster = Next(cluster);
        if (allocate & (nextCluster == -1))
            {
            nextCluster = Allocate(cluster);
            }
        }
    return nextCluster;
}

//------------------------------------------------------------------------------

bool_t
BootFileSystemExFat_t::
ReleaseClusterLink(
    size_t cluster
    )
{
    bool_t rc = false;

    // Until there isn't linked cluster
    while (cluster != -1)
        {
        size_t nextCluster = *FatLink(cluster);
        if (!Free(cluster)) goto cleanUp;
        cluster = nextCluster;
        }

    // Done
    rc = true;

cleanUp:    
    return rc;
}

//------------------------------------------------------------------------------

