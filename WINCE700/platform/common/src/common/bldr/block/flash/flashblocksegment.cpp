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

using namespace ceboot;

//------------------------------------------------------------------------------

BootBlockSegmentFlash_t::
BootBlockSegmentFlash_t(
    BootBlockFlash_t *pBlock
    )
{
    m_pBlock = pBlock;
    pBlock->AddRef();
}

//------------------------------------------------------------------------------

BootBlockSegmentFlash_t::
~BootBlockSegmentFlash_t(
    )
{
}

//------------------------------------------------------------------------------

bool_t
BootBlockSegmentFlash_t::
DeInit(
    )
{
    m_pBlock->Release();
    delete this;
    return true;
}

//------------------------------------------------------------------------------

bool_t
BootBlockSegmentFlash_t::
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
BootBlockSegmentFlash_t::
OpenBinaryRegion(
    enum_t index
    )
{
    return m_pBlock->QueryBinaryRegion(index, m_partitionId);
}

//------------------------------------------------------------------------------

bool_t
BootBlockSegmentFlash_t::
OpenReservedRegion(
    const char *name
    )
{
    return m_pBlock->QueryReservedRegion(name, m_partitionId);
}

//------------------------------------------------------------------------------

bool_t
BootBlockSegmentFlash_t::
OpenPartition(
    uint8_t fileSystem,
    enum_t index
    )
{
    return m_pBlock->QueryPartition(fileSystem, index, m_partitionId);
}

//------------------------------------------------------------------------------

bool_t
BootBlockSegmentFlash_t::
Read(
    size_t sector,
    size_t sectors,
    uint8_t *pBuffer
    )
{
    FLASH_TRANSFER_REQUEST request;

    request.PartitionId = m_partitionId;
    request.AssociationId = 0;
    request.RequestFlags = 0;
    request.TransferCount = 1;
    request.AssociationId = 0;
    request.TransferList[0].pBuffer = pBuffer;
    request.TransferList[0].pPhysicalAddress = NULL;
    request.TransferList[0].SectorRun.StartSector = sector;
    request.TransferList[0].SectorRun.SectorCount = sectors;

    return m_pBlock->Read(&request);
}

//------------------------------------------------------------------------------

bool_t
BootBlockSegmentFlash_t::
Write(
    size_t sector,
    size_t sectors,
    uint8_t *pBuffer
    )
{
    FLASH_TRANSFER_REQUEST request;

    request.PartitionId = m_partitionId;
    request.AssociationId = 0;
    request.RequestFlags = 0;
    request.TransferCount = 1;
    request.AssociationId = 0;
    request.TransferList[0].pBuffer = pBuffer;
    request.TransferList[0].pPhysicalAddress = NULL;
    request.TransferList[0].SectorRun.StartSector = sector;
    request.TransferList[0].SectorRun.SectorCount = sectors;

    return m_pBlock->Write(&request);
}

//------------------------------------------------------------------------------

bool_t
BootBlockSegmentFlash_t::
Erase(
    )
{
    return m_pBlock->Erase(m_partitionId);
}

//------------------------------------------------------------------------------

