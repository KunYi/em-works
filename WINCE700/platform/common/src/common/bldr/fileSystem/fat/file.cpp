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
#include "file.hpp"
#include <bootLog.h>

using namespace ceboot;

//------------------------------------------------------------------------------

BootFileSystemFileFat_t*
BootFileSystemFileFat_t::
Create(
    BootFileSystemFat_t *pFileSystem,
    flags_t access,
    size_t sectorSize,
    size_t sectorsPerCluster,
    size_t dirCluster,
    size_t dirEntry,
    size_t fileSize,
    size_t fileCluster
    )
{
    BootFileSystemFileFat_t* pThis = NULL;

    pThis = new BootFileSystemFileFat_t(pFileSystem, access);
    if ((pThis != NULL) && !pThis->Init(
        sectorSize, sectorsPerCluster, dirCluster, dirEntry,
        fileSize, fileCluster
        ))
        {
        delete pThis;
        pThis = NULL;
        }
    return pThis;
}

//------------------------------------------------------------------------------

BootFileSystemFileFat_t::
BootFileSystemFileFat_t(
    BootFileSystemFat_t* pFileSystem,
    flags_t access
    ) : m_pFileSystem(pFileSystem), m_access(access), m_pBuffer(NULL),
        m_bufferDirty(false), m_bufferCluster(0), m_bufferSector(0)
{
    pFileSystem->AddRef();
}

//------------------------------------------------------------------------------

bool_t
BootFileSystemFileFat_t::
Init(
    size_t sectorSize,
    size_t sectorsPerCluster,
    size_t dirCluster,
    size_t dirEntry,
    size_t fileSize,
    size_t fileCluster
    )
{
    bool_t rc = false;

    m_pBuffer = new uint8_t[sectorSize];
    if (m_pBuffer == NULL) goto cleanUp;
    m_bufferDirty = false;
    m_bufferCluster = 0;
    m_bufferSector = 0;

    m_sectorSize = sectorSize;
    m_sectorsPerCluster = sectorsPerCluster;

    m_dirCluster = dirCluster;
    m_dirEntry = dirEntry;
    m_fileCluster = fileCluster;
    m_fileSize = fileSize;

    m_fileSizeChanged = false;

    m_pos = 0;
    m_posCluster = fileCluster;
    m_posSector = (fileCluster > 1) ? 0 : m_sectorsPerCluster - 1;
    m_posOffset = (fileCluster > 1) ? 0 : m_sectorSize;

    rc = true;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

BootFileSystemFileFat_t::
~BootFileSystemFileFat_t(
    )
{
    Flush();
    delete m_pBuffer;
    m_pFileSystem->Release();
}

//------------------------------------------------------------------------------

bool_t
BootFileSystemFileFat_t::
DeInit(
    )
{
    delete this;
    return true;
}

//------------------------------------------------------------------------------

bool_t
BootFileSystemFileFat_t::
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
            ReadParams_t* pInfo = (ReadParams_t *)pBuffer;
            if ((pInfo == NULL) || (size != sizeof(*pInfo))) break;
            pInfo->size = Read(pInfo->pBuffer, pInfo->size);
            rc = true;
            }
            break;
        case WriteIoCtl:
            {
            WriteParams_t* pInfo = (WriteParams_t *)pBuffer;
            if ((pInfo == NULL) || (size != sizeof(*pInfo))) break;
            pInfo->size = Write(pInfo->pBuffer, pInfo->size);
            rc = true;
            }
            break;
        case SeekIoCtl:
            {
            SeekParams_t* pInfo = (SeekParams_t *)pBuffer;
            if ((pInfo == NULL) || (size != sizeof(*pInfo))) break;
            rc = Seek(pInfo->method, pInfo->pos);
            }
            break;
        case GetSizeIoCtl:
            {
            GetSizeParams_t* pInfo = (GetSizeParams_t *)pBuffer;
            if ((pInfo == NULL) || (size != sizeof(*pInfo))) break;
            pInfo->size = GetSize();
            rc = true;
            }
            break;
        case FlushIoCtl:
            if ((pBuffer != NULL) || (size != 0)) break;
            rc = Flush();
            break;
        }

    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootFileSystemFileFat_t::
FlushSectorBuffer(
    )
{
    bool_t rc = false;

    if (m_bufferDirty && !m_pFileSystem->WriteSector(
            m_bufferCluster, m_bufferSector, m_pBuffer
            )) goto cleanUp;

    m_bufferDirty = false;
    rc = true;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootFileSystemFileFat_t::
CopyFromSectorBuffer(
    void** ppBuffer,
    size_t cluster,
    size_t sector,
    size_t* pOffset,
    size_t size
    )
{
    bool_t rc = false;
    uint8_t* pBuffer = (uint8_t*)*ppBuffer;

    // If we don't own correct cluster?
    if ((cluster != m_bufferCluster) || (sector != m_bufferSector))
        {
        if (!FlushSectorBuffer()) goto cleanUp;
        if (!m_pFileSystem->ReadSector(cluster, sector, m_pBuffer))
            {
            m_bufferCluster = 0;
            m_bufferSector = 0;
            goto cleanUp;
            }
        m_bufferCluster = cluster;
        m_bufferSector = sector;
        }

    memcpy(pBuffer, &m_pBuffer[*pOffset], size);
    pBuffer += size;
    *pOffset += size;

    // Done
    *ppBuffer = pBuffer;
    rc = true;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootFileSystemFileFat_t::
CopyToSectorBuffer(
    size_t cluster,
    size_t sector,
    size_t* pOffset,
    void** ppBuffer,
    size_t size
    )
{
    bool_t rc = false;
    uint8_t* pBuffer = (uint8_t*)*ppBuffer;

    if ((cluster != m_bufferCluster) || (sector != m_bufferSector))
        {
        if (!FlushSectorBuffer()) goto cleanUp;
        if (!m_pFileSystem->ReadSector(cluster, sector, m_pBuffer))
            {
            m_bufferCluster = 0;
            m_bufferSector = 0;
            goto cleanUp;
            }
        m_bufferCluster = cluster;
        m_bufferSector = sector;
        }

    if (*pOffset > m_sectorSize) DebugBreak();
    if ((*pOffset + size) > m_sectorSize) DebugBreak();

    memcpy(&m_pBuffer[*pOffset], pBuffer, size);
    *pOffset += size;
    pBuffer += size;
    m_bufferDirty = true;

    // Done
    *ppBuffer = pBuffer;
    rc = true;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

size_t
BootFileSystemFileFat_t::
Read(
    void *pBuffer,
    size_t size
    )
{
    size_t rc = 0;

    // Modify size so we don't read behind end of file
    if ((m_fileSize - m_pos) < size) size = m_fileSize - m_pos;

    // Read data to buffer
    while (size > 0)
        {
        // Amount of data in buffer
        size_t count = m_sectorSize - m_posOffset;
        if (count > size) count = size;

        // Copy data
        if ((count > 0) && !CopyFromSectorBuffer(
                &pBuffer, m_posCluster, m_posSector, &m_posOffset, count
                )) break;

        // Update number of bytes we read and need to read
        m_pos += count;
        size -= count;
        rc += count;

        // When we are on end of sector move forward
        if ((m_posOffset >= m_sectorSize) && (size > 0))
            {
            if ((m_posSector + 1) >= m_sectorsPerCluster)
                {
                size_t next = m_pFileSystem->NextCluster(m_posCluster, false);
                if (next == -1) break;
                m_posCluster = next;
                m_posSector = 0;
                m_posOffset = 0;
                }
            else
                {
                m_posSector++;
                m_posOffset = 0;
                }
            }
        }

    return rc;
}

//------------------------------------------------------------------------------

size_t
BootFileSystemFileFat_t::
Write(
    void *pBuffer,
    size_t size
    )
{
    size_t rc = 0;

    while (size > 0)
        {
        size_t count = m_sectorSize - m_posOffset;
        if (count > size) count = size;

        // If there is room in current cluster
        if ((count > 0) && !CopyToSectorBuffer(
                m_posCluster, m_posSector, &m_posOffset, &pBuffer, count
                )) break;

        m_pos += count;
        if (m_pos > m_fileSize)
            {
            m_fileSize = m_pos;
            m_fileSizeChanged = true;
            }
        size -= count;
        rc += count;

        // When we are on end of sector move forward
        if ((m_posOffset >= m_sectorSize) && (size > 0))
            {
            if ((m_posSector + 1) >= m_sectorsPerCluster)
                {
                size_t next = m_pFileSystem->NextCluster(m_posCluster, true);
                if (next == -1) break;
                if (m_fileCluster < 2) m_fileCluster = next;
                m_posCluster = next;
                m_posSector = 0;
                m_posOffset = 0;
                }
            else
                {
                m_posSector++;
                m_posOffset = 0;
                }
            }
        }

    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootFileSystemFileFat_t::
Seek(
    enum_t method,
    size_t pos
    )
{
    bool_t rc = false;
    
    switch (method)
        {
        case SeekBegin:
            if (pos > m_fileSize) goto cleanUp;
            break;
        case SeekCurrent:
            pos += m_pos;
            if (pos > m_fileSize) goto cleanUp;
            break;
        case SeekEnd:
            pos += m_fileSize;
            if (pos > m_fileSize) goto cleanUp;
            break;
        default:
            goto cleanUp;
        }

    size_t cluster = m_fileCluster;
    size_t filePos = pos;
    size_t clusterSize = m_sectorSize * m_sectorsPerCluster;
    while (filePos > clusterSize)
        {
        size_t next = m_pFileSystem->NextCluster(cluster);
        if (next == -1) break;
        cluster = next;
        filePos -= clusterSize;
        }

    size_t sector = 0;
    while (filePos > m_sectorSize)
        {
        sector++;
        filePos -= m_sectorSize;
        }

    if (cluster >= 2)
        {
        m_posCluster = cluster;
        m_posSector = sector;
        m_posOffset = filePos;
        }
    else
        {
        m_posCluster = 0;
        m_posSector = m_sectorsPerCluster - 1;
        m_posOffset = m_sectorSize;
        }
    
    m_pos = pos;
    rc = true;
        
cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

size_t
BootFileSystemFileFat_t::
GetSize(
    )
{
    return m_fileSize;
}

//------------------------------------------------------------------------------

bool_t
BootFileSystemFileFat_t::
Flush(
    )
{
    bool_t rc = false;

    // Write back cluster if it is dirty
    if (!FlushSectorBuffer()) goto cleanUp;

    // And update directory entry if needed
    if (m_fileSizeChanged && !m_pFileSystem->UpdateDirEntry(
            m_dirCluster, m_dirEntry, m_fileCluster, m_fileSize
            )) goto cleanUp;

    // Make sure that file system is also flushed
    if (!m_pFileSystem->Flush()) goto cleanUp;

    // Done
    rc = true;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

