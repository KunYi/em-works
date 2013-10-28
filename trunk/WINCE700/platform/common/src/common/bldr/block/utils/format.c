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
#include <bootBlockUtils.h>
#include <bootDownload.h>
#include <bootMemory.h>
#include <bootLog.h>

//------------------------------------------------------------------------------

bool_t
BootBlockFormatForStoreDownload(
    handle_t hBlock,
    handle_t hDownload
    )
{
    bool_t rc = FALSE;
    BootDownloadStoreInfo_t imageInfo;
    BootBlockFormatInfo_t formatInfo;
    BootBlockBinaryRegionInfo_t* pBinary;
    BootBlockReservedRegionInfo_t *pRegion;
    BootBlockPartitionInfo_t *pPartition;
    BootDownloadStoreSegmentInfo_t *pSegment;
    enum_t binaries = 0, regions = 0, partitions = 0;
    enum_t ix;
    size_t size;
    

    // Get image information
    if (!BootDownloadStoreImageInfo(hDownload, &imageInfo)) goto cleanUp;

    // Create format information
    memset(&formatInfo, 0, sizeof(formatInfo));

    // Find number of reserved regions and partitions
    pSegment = imageInfo.pSegment;
    for (ix = 0; ix < imageInfo.segments; ix++)
        {
        switch (pSegment->type)
            {
            case BOOT_DOWNLOAD_STORE_SEGMENT_TYPE_BINARY:
                binaries++;
                break;
            case BOOT_DOWNLOAD_STORE_SEGMENT_TYPE_REGION:
                regions++;
                break;
            case BOOT_DOWNLOAD_STORE_SEGMENT_TYPE_PARTITION:
                partitions++;
                break;
            }
            pSegment++;
        }

    // Allocate structures
    if (binaries > 0)
        {
        size = binaries * sizeof(BootBlockBinaryRegionInfo_t);
        formatInfo.pBinaryRegionInfo = BootAlloc(size);
        if (formatInfo.pBinaryRegionInfo == NULL) goto cleanUp;
        formatInfo.binaryRegions = regions;
        }

    if (regions > 0)
        {
        size = regions * sizeof(BootBlockReservedRegionInfo_t);
        formatInfo.pReservedRegionInfo = BootAlloc(size);
        if (formatInfo.pReservedRegionInfo == NULL) goto cleanUp;
        formatInfo.reservedRegions = regions;
        }

    if (partitions > 0)
        {
        size = partitions * sizeof(BootBlockPartitionInfo_t);
        formatInfo.pPartitionInfo = BootAlloc(size);
        if (formatInfo.pPartitionInfo == NULL) goto cleanUp;
        formatInfo.partitions = partitions;
        }

    // Fill them
    pBinary = formatInfo.pBinaryRegionInfo;
    pRegion = formatInfo.pReservedRegionInfo;
    pPartition = formatInfo.pPartitionInfo;
    pSegment = imageInfo.pSegment;
    for (ix = 0; ix < imageInfo.segments; ix++)
        {
        switch (pSegment->type)
            {
            case BOOT_DOWNLOAD_STORE_SEGMENT_TYPE_BINARY:
                pBinary->sectors = pSegment->sectors;
                pBinary++;
                break;
            case BOOT_DOWNLOAD_STORE_SEGMENT_TYPE_REGION:
                memcpy(pRegion->name, pSegment->name, sizeof(pRegion->name));
                pRegion->sectors = pSegment->sectors;
                pRegion++;
                break;
            case BOOT_DOWNLOAD_STORE_SEGMENT_TYPE_PARTITION:
                pPartition->fileSystem = pSegment->fileSystem;
                pPartition->sectors = pSegment->sectors;
                pPartition++;
                break;
            }
            pSegment++;
        }

    // Format media
    if (!BootBlockFormat(hBlock, &formatInfo)) goto cleanUp;

    // Done
    rc = TRUE;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

