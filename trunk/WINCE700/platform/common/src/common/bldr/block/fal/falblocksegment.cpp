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

using namespace ceboot;

//------------------------------------------------------------------------------

BootBlockSegmentFal_t::
BootBlockSegmentFal_t(
    BootBlockFal_t *pBlock
    )
{
    m_pBlock = pBlock;
    pBlock->AddRef();
}

//------------------------------------------------------------------------------

BootBlockSegmentFal_t::
~BootBlockSegmentFal_t(
    )
{
}

//------------------------------------------------------------------------------

bool_t
BootBlockSegmentFal_t::
DeInit(
    )
{
    m_pBlock->Release();
    delete this;
    return true;
}

//------------------------------------------------------------------------------

bool_t
BootBlockSegmentFal_t::
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
        }

    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootBlockSegmentFal_t::
OpenBinaryRegion(
    enum_t index
    )
{
    m_region = 0;
    return m_pBlock->QueryBinaryRegion(index, m_sector, m_sectors);
}

//------------------------------------------------------------------------------

bool_t
BootBlockSegmentFal_t::
OpenReservedRegion(
    const char *name
    )
{
    m_region = 1;
    return m_pBlock->QueryReservedRegion(name, m_sector, m_sectors);
}

//------------------------------------------------------------------------------

bool_t
BootBlockSegmentFal_t::
OpenPartition(
    uchar fileSystem,
    enum_t index
    )
{
    return m_pBlock->QueryPartition(
        fileSystem, index, m_region, m_sector, m_sectors
        );
}

//------------------------------------------------------------------------------

bool_t
BootBlockSegmentFal_t::
Read(
    size_t sector,
    size_t sectors,
    void *pBuffer
    )
{
    bool_t rc = false;

    if (sector > m_sectors) goto cleanUp;
    if (sectors > (m_sectors - sector)) goto cleanUp;

    sector += m_sector;        
    rc = m_pBlock->Read(m_region, sector, sectors, pBuffer);

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootBlockSegmentFal_t::
Write(
    size_t sector,
    size_t sectors,
    void *pBuffer
    )
{
    bool_t rc = false;

    if (sector > m_sectors) goto cleanUp;
    if (sectors > (m_sectors - sector)) goto cleanUp;

    sector += m_sector;        
    rc = m_pBlock->Write(m_region, sector, sectors, pBuffer);

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootBlockSegmentFal_t::
Erase(
    )
{
    return m_pBlock->Erase(m_region, m_sector, m_sectors);
}

//------------------------------------------------------------------------------

