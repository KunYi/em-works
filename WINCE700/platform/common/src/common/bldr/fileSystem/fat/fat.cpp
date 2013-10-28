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
#include "fat.hpp"
#include "fat12.hpp"
#include "fat16.hpp"
#include "fat32.hpp"
#include "exfat.hpp"
#include "file.hpp"
#include <bootBlockBios.h>
#include <bootLog.h>

using namespace ceboot;

//------------------------------------------------------------------------------

extern "C"
handle_t
BootFileSystemFatInit(
    handle_t hBlock,
    enum_t index
    )
{
    return BootFileSystemFat_t::BootFileSystemFatInit(hBlock, index);
}

//------------------------------------------------------------------------------

BootFileSystemFat_t*
BootFileSystemFat_t::
BootFileSystemFatInit(
    handle_t hBlock,
    enum_t index
    )
{
    uint8_t* pSector = NULL;
    BootFileSystemFat_t *pDriver = NULL;
    BootBlock_t* pBlock = (BootBlock_t *)hBlock;
    if (pBlock == NULL) goto cleanUp;

    // Get sector size
    size_t sectorSize = pBlock->SectorSize();
    if (sectorSize == 0) goto cleanUp;

    // Open FAT partition
    BootBlockSegment_t* pSegment = pBlock->OpenPartition(0x04, index);
    if (pSegment == NULL) pSegment = pBlock->OpenPartition(0x06, index);
    if (pSegment == NULL) pSegment = pBlock->OpenPartition(0x07, index);
    if (pSegment == NULL) pSegment = pBlock->OpenPartition(0x0B, index);
    if (pSegment == NULL) pSegment = pBlock->OpenPartition(0x0C, index);
    if (pSegment == NULL) pSegment = pBlock->OpenPartition(0x0E, index);
    if (pSegment == NULL) pSegment = pBlock->OpenPartition(0x0F, index);

    // There isn't any FAT partition on block device
    if (pSegment == NULL) goto cleanUp;

    // Allocate buffer for sector
    pSector = new uint8_t[sectorSize];
    if (pSector == NULL) goto cleanUp;

    // Read sector zero
    if (!pSegment->Read(0, 1, pSector))
        {
        pSegment->DeInit();
        goto cleanUp;
        }

    // Get FAT parameters
    BootBlockBiosParameterBlockType1_t* pBpb1;
    pBpb1 = (BootBlockBiosParameterBlockType1_t*)&pSector[3];
    BootBlockBiosParameterBlockType2_t* pBpb2;
    pBpb2 = (BootBlockBiosParameterBlockType2_t*)&pSector[3];
    BootBlockBiosParameterBlockTypeEx_t* pBpbEx;
    pBpbEx = (BootBlockBiosParameterBlockTypeEx_t*)&pSector[3];

    // Find FAT file system flavor
    if (memcmp(pBpb1->fatType, "FAT12   ", sizeof(pBpb1->fatType)) == 0)
        {
        pDriver = BootFileSystemFat12_t::Create(pSegment, sectorSize);
        }
    else if (memcmp(pBpb1->fatType, "FAT16   ", sizeof(pBpb1->fatType)) == 0)
        {
        pDriver = BootFileSystemFat16_t::Create(pSegment, sectorSize);
        }
    else if (memcmp(pBpb2->fatType, "FAT32   ", sizeof(pBpb2->fatType)) == 0)
        {
        pDriver = BootFileSystemFat32_t::Create(pSegment, sectorSize);
        }
    else if (memcmp(pBpbEx->versionId, "EXFAT   ", sizeof(pBpbEx->versionId)) == 0)
        {
        pDriver = BootFileSystemExFat_t::Create(pSegment, sectorSize);
        }
    else
        {
        pSegment->DeInit();
        }
   
cleanUp:    
    if (pSector)
        {
        delete [] pSector;
        }
    return pDriver;
}

//------------------------------------------------------------------------------

BootFileSystemFat_t::
BootFileSystemFat_t(
    ) : m_refCount(1)
{
}

//------------------------------------------------------------------------------

BootFileSystemFat_t::
~BootFileSystemFat_t(
    )
{
}

//------------------------------------------------------------------------------

enum_t
BootFileSystemFat_t::
AddRef(
    )
{
    return m_refCount++;
}

//------------------------------------------------------------------------------

enum_t
BootFileSystemFat_t::
Release(
    )
{
    if (--m_refCount == 0) delete this;
    return m_refCount;
}

//------------------------------------------------------------------------------
