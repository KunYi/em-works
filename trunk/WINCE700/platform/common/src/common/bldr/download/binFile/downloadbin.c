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
#include <bootDownloadBin.h>
#include <bootDownloadBinFormat.h>
#include <bootDownloadNotify.h>
#include <bootMemory.h>
#include <bootLog.h>
#include <bootTransport.h>


//------------------------------------------------------------------------------

typedef struct RamImage_t {
    uint32_t base;
    uint32_t start;
    uint32_t offset;
    size_t size;
    size_t downloaded;
} RamImage_t;

typedef struct StoreImage_t {

    size_t sectorSize;
    size_t sectorsPerRecord;
    size_t sectors;

    size_t segments;
    BootDownloadStoreSegmentInfo_t *pSegment;

    BootDownloadStoreRecordInfo_t recordInfo;
    size_t recordSize;
    uint8_t *pRecord;

    uint32_t hashType;
    uint32_t hashSize;
    uint8_t *pHash;

    uint32_t signType;
    uint32_t signSize;
    uint8_t *pSign;

    uint32_t hashInfoSize;
    uint8_t *pHashInfo;

    size_t downloaded;

} StoreImage_t;

#pragma warning(push)
//Under Microsoft extensions (/Ze), you can specify a structure without 
//a declarator as members of another structure or union. These structures 
//generate W4 warning C4201. 
#pragma warning(disable:4201) 
typedef struct Download_t {
    BootDriverVTable_t *pVTable;    // Driver virtual function table
    void *pContext;                 // Notify context
    handle_t hTransport;            // Transport handle (to get data from)
    enum_t type;                    // Image type
    union {                         // Information depending on image type
        RamImage_t ram;
        StoreImage_t store;
        };
} Download_t;
#pragma warning(pop)

//------------------------------------------------------------------------------
//  Local functions

bool_t
BootDownloadBinDeinit(
    void *pContext
    );

bool_t
BootDownloadBinIoCtl(
    void *pContext,
    enum_t code,
    void *pBuffer,
    size_t size
    );

static
bool_t
ImageType(
    Download_t *pContext,
    enum_t *pType
    );

static
bool_t
GetData(
    Download_t *pDownload,
    void **pInfo,
    size_t *pInfoSize,
    void **ppData,
    size_t *pDataSize
    );

static
bool_t
RamInfo(
    Download_t *pDownload,
    BootDownloadRamImageInfo_t *pInfo
    );

bool_t
RamOffset(
    Download_t *pDownload,
    BootDownloadRamOffsetParams_t *pInfo
    );

static
bool_t
StoreInfo(
    Download_t *pDownload,
    BootDownloadStoreInfo_t *pInfo
    );

bool_t
GetOSInfo(
    Download_t *pDownload,
    BootDownloadGetOsInfoParams_t *pInfo
    );

//------------------------------------------------------------------------------

static
BootDriverVTable_t
s_downloadVTable = {
    BootDownloadBinDeinit,
    BootDownloadBinIoCtl
};

//------------------------------------------------------------------------------

static
Download_t
s_download;

//------------------------------------------------------------------------------


handle_t
BootDownloadBinInit(
    void *pContext,
    handle_t hTransport
    )
{
    handle_t hDriver = NULL;


    // This driver shouldn't be initialized twice...
    if (s_download.pVTable != NULL)
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootDownloadBinInit: "
            L"Driver already initialized!\r\n"
            ));
        goto cleanUp;
        }

    if (hTransport == NULL)
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootDownloadBinInit: "
            L"Invalid (NULL) transport driver handle!\r\n"
            ));
        goto cleanUp;
        }

    s_download.pVTable = &s_downloadVTable;
    s_download.pContext = pContext;
    s_download.hTransport = hTransport;
    s_download.type = BOOT_DOWNLOAD_IMAGE_UNKNOWN;

    // Done
    hDriver = &s_download;

cleanUp:
    return hDriver;
}

//------------------------------------------------------------------------------

bool_t
BootDownloadBinDeinit(
    void *pContext
    )
{
    bool_t rc = false;
    Download_t *pDownload = pContext;


    // Check display handle
    if (pDownload != &s_download)
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootDownloadBinDeinit: "
            L"Invalid driver handle!\r\n"
            ));
        goto cleanUp;
        }

    // Release transport
    BootTransportDeinit(pDownload->hTransport);

    // Clear context
    memset(pDownload, 0, sizeof(*pDownload));

    // Done
    rc = true;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootDownloadBinIoCtl(
    void *pContext,
    enum_t code,
    void *pBuffer,
    size_t size
    )
{
    bool_t rc = false;
    Download_t *pDownload = pContext;


    // Check display handle
    if (pDownload != &s_download)
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootDownloadBinIoCtl: "
            L"Invalid driver handle!\r\n"
            ));
        goto cleanUp;
        }

    switch (code)
        {
        case BOOT_DOWNLOAD_IOCTL_IMAGE_TYPE:
            {
            BootDownloadImageTypeParams_t *pParams = pBuffer;
            if ((pParams == NULL) || (size != sizeof(*pParams)))
                {
                BOOTMSG(ZONE_ERROR, (L"ERROR: BootDownloadBinIoCtl: "
                    L"Invalid BOOT_DOWNLOAD_IOCTL_IMAGE_TYPE parameter!\r\n"
                    ));
                break;
                }
            rc = ImageType(pDownload, &pParams->type);
            }
            break;
        case BOOT_DOWNLOAD_IOCTL_GET_DATA:
            {
            BootDownloadGetDataParams_t *pParams = pBuffer;
            if ((pParams == NULL) || (size != sizeof(*pParams)))
                {
                BOOTMSG(ZONE_ERROR, (L"ERROR: BootDownloadBinIoCtl: "
                    L"Invalid BOOT_DOWNLOAD_IOCTL_GET_DATA parameter!\r\n"
                    ));
                break;
                }
            rc = GetData(
                    pDownload, &pParams->pInfo, &pParams->infoSize,
                    &pParams->pData, &pParams->dataSize
                    );
            }
            break;
        case BOOT_DOWNLOAD_IOCTL_RAM_INFO:
            {
            BootDownloadRamImageInfo_t *pInfo = pBuffer;
            if ((pInfo == NULL) || (size != sizeof(*pInfo)))
                {
                BOOTMSG(ZONE_ERROR, (L"ERROR: BootDownloadBinIoCtl: "
                    L"Invalid BOOT_DOWNLOAD_IOCTL_RAM_INFO parameter!\r\n"
                    ));
                break;
                }
            rc = RamInfo(pDownload, pInfo);
            }
            break;
        case BOOT_DOWNLOAD_IOCTL_RAM_OFFSET:
            {
            BootDownloadRamOffsetParams_t *pInfo = pBuffer;
            if ((pInfo == NULL) || (size != sizeof(*pInfo)))
                {
                BOOTMSG(ZONE_ERROR, (L"ERROR: BootDownloadBinIoCtl: "
                    L"Invalid BOOT_DOWNLOAD_IOCTL_RAM_OFFSET parameter!\r\n"
                    ));
                break;
                }
            rc = RamOffset(pDownload, pInfo);
            }
            break;
        case BOOT_DOWNLOAD_IOCTL_STORE_INFO:
            {
            BootDownloadStoreInfo_t *pInfo = pBuffer;
            if ((pInfo == NULL) || (size != sizeof(*pInfo)))
                {
                BOOTMSG(ZONE_ERROR, (L"ERROR: BootDownloadBinIoCtl: "
                    L"Invalid BOOT_DOWNLOAD_IOCTL_STORE_INFO parameter!\r\n"
                    ));
                break;
                }
            rc = StoreInfo(pDownload, pInfo);
            }
            break;
        case BOOT_DOWNLOAD_IOCTL_GET_OS_INFO:
            {
            BootDownloadGetOsInfoParams_t *pInfo = pBuffer;
            if ((pInfo == NULL) || (size != sizeof(*pInfo)))
                {
                BOOTMSG(ZONE_ERROR, (L"ERROR: BootDownloadBinIoCtl: "
                    L"Invalid BOOT_DOWNLOAD_IOCTL_GET_OS_INFO parameter!\r\n"
                    ));
                break;
                }
            rc = GetOSInfo(pDownload, pInfo);
            }
            break;
        default:
            BOOTMSG(ZONE_ERROR, (L"ERROR: BootDownloadBinIoCtl: "
                L"Unsupported code!\r\n"
                ));
            break;
        }

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
ImageTypeRam(
    Download_t *pDownload
    )
{
    bool_t rc = false;
    RamImage_t *pRam = &pDownload->ram;
    BootBinFormatRamHeader_t header;


    // It is RAM image...
    pDownload->type = BOOT_DOWNLOAD_IMAGE_RAM;
    memset(pRam, 0, sizeof(*pRam));
    // Read image header
    if (!BootTransportRead(pDownload->hTransport, &header, sizeof(header)))
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootDownloadBin!ImageTypeRam: "
            L"BootTransportRead Failed!\r\n"
            ));
        goto cleanUp;
        }

    // Store header info
    pDownload->ram.base = header.start;
    pDownload->ram.size = header.size;

    // Done
    rc = true;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
ImageTypeStore(
    Download_t *pDownload
    )
{
    bool_t rc = false;
    StoreImage_t *pStore = &pDownload->store;
    BootBinFormatStoreHeader_t header;
    uint32_t size, ix;

    BOOTMSG(ZONE_FUNC, (L"+BootDownloadBin!ImageTypeStore\r\n"));

    // It is storage image...
    pDownload->type = BOOT_DOWNLOAD_IMAGE_STORE;
    memset(pStore, 0, sizeof(*pStore));

    // Read image header
    if (!BootTransportRead(
            pDownload->hTransport, &header, sizeof(header)
            ))
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootDownloadBin!ImageTypeStore: "
            L"Store Image header read from transport failed!\r\n"
            ));
        goto cleanUp;
        }

    // Save header info
    pStore->sectorSize = header.sectorSize;
    pStore->sectors = header.sectors;
    pStore->segments = header.segments;
    pStore->hashType = header.hashType;
    pStore->hashSize = header.hashSize;
    pStore->hashInfoSize = header.hashInfoSize;

    // Allocate segment descriptors
    size = pStore->segments * sizeof(BootDownloadStoreSegmentInfo_t);
    pStore->pSegment = BootAlloc(size);
    if (pStore->pSegment == NULL)
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootDownloadBin!ImageTypeStore: "
            L"Store Image Segment buffer allocation failed (%d bytes)!\r\n",
            size
            ));
        goto cleanUp;
        }

    // Allocate hash buffer if needed
    if (pStore->hashSize > 0)
        {
        pStore->pHash = BootAlloc(pStore->hashSize);
        if (pStore->pHash == NULL)
            {
            BOOTMSG(ZONE_ERROR, (L"ERROR: BootDownloadBin!ImageTypeStore: "
                L"Store Image hash buffer allocation failed (%d bytes)!\r\n",
                pStore->hashSize
                ));
            goto cleanUp;
            }
        }

    // read hashInfo
    if (pStore->hashInfoSize > 0)
        {
        pStore->pHashInfo = BootAlloc(pStore->hashInfoSize);
        if (pStore->pHashInfo == NULL)
            {
            BOOTMSG(ZONE_ERROR, (L"ERROR: BootDownloadBin!ImageTypeStore: "
                L"Hash Info buffer allocation failed (%d bytes)!\r\n",
                pStore->hashInfoSize
                ));
            goto cleanUp;
            }

        // HashInfo will not be included in the hash of the header to avoid any
        // recursive dependencies
        if (!BootTransportRead(
                pDownload->hTransport, pStore->pHashInfo, pStore->hashInfoSize
                ))
            {
            BOOTMSG(ZONE_ERROR, (L"ERROR: BootDownloadBin!ImageTypeStore: "
                L"Store Image Hash Info read from transport failed!\r\n"
                ));
            goto cleanUp;
            }
        }

    BOOTMSG(ZONE_INFO, (
        L"Download STORE image with %d segments (sector %d bytes, hash type %d)\r\n",
        pStore->segments, pStore->sectorSize, pStore->hashType
        ));

    // Initialize hash if needed...
    switch (pStore->hashType)
        {
        case BOOT_BIN_FORMAT_HASH_SUM:
            break;
        default:
            BOOTMSG(ZONE_ERROR, (L"ERROR: BootDownloadBin!ImageTypeStore: "
                L"Unsupported hash type %d!\r\n", pStore->hashType
                ));
            goto cleanUp;
        }

    // Read segments info
    for (ix = 0; ix < pStore->segments; ix++)
        {
        BootBinFormatStoreSegmentHeader_t segmentHeader;
        BootDownloadStoreSegmentInfo_t *pSegment = &pStore->pSegment[ix];

        // Read segment info from transport
        if (!BootTransportRead(
                pDownload->hTransport, &segmentHeader, sizeof(segmentHeader)
                ))
            {
            BOOTMSG(ZONE_ERROR, (L"ERROR: BootDownloadBin!ImageTypeStore: "
                L"Store Image Segment Header read from transport failed!\r\n"
                ));
            goto cleanUp;
            }


        // Depending on segment type read additional info
        switch (segmentHeader.type)
            {
            case BOOT_BIN_FORMAT_STORE_SEGMENT_BINARY:
                {
                BootBinFormatStoreSegmentBinaryInfo_t info;
                pSegment->type = BOOT_DOWNLOAD_STORE_SEGMENT_TYPE_BINARY;
                pSegment->sectors = segmentHeader.sectors;
                if (segmentHeader.infoSize != sizeof(info))
                    {
                    BOOTMSG(ZONE_ERROR, (L"ERROR: BootDownloadBin!ImageType: "
                        L"Store Image unexpected segment info size (%d)!\r\n",
                        segmentHeader.infoSize
                        ));
                    goto cleanUp;
                    }
                if (!BootTransportRead(
                        pDownload->hTransport, &info, sizeof(info)
                        ))
                    {
                    BOOTMSG(ZONE_ERROR, (L"ERROR: BootDownloadBin!ImageType: "
                        L"Store Image Segment Info read from transport failed!\r\n"
                        ));
                    goto cleanUp;
                    }
                pSegment->index = info.index;
                BOOTMSG(ZONE_INFO, (
                    L"Segment %d has size %5d sectors, binary 0x%02x\r\n",
                    ix, pSegment->sectors, pSegment->index
                    ));
                }
                break;
            case BOOT_BIN_FORMAT_STORE_SEGMENT_RESERVED:
                {
                BootBinFormatStoreSegmentReservedInfo_t info;
                pSegment->type = BOOT_DOWNLOAD_STORE_SEGMENT_TYPE_REGION;
                pSegment->sectors = segmentHeader.sectors;
                if (segmentHeader.infoSize != sizeof(info))
                    {
                    BOOTMSG(ZONE_ERROR, (L"ERROR: BootDownloadBin!ImageTypeStore: "
                        L"Store Image unexpected segment info size (%d)!\r\n",
                        segmentHeader.infoSize
                        ));
                    goto cleanUp;
                    }
                if (!BootTransportRead(
                        pDownload->hTransport, &info, sizeof(info)
                        ))
                    {
                    BOOTMSG(ZONE_ERROR, (L"ERROR: BootDownloadBin!ImageType: "
                        L"Store Image Segment Info read from transport failed!\r\n"
                        ));
                    goto cleanUp;
                    }
                memcpy(pSegment->name, info.name, sizeof(pSegment->name));
                BOOTMSG(ZONE_INFO, (
                    L"Segment %d has size %5d sectors, reserved region %hs\r\n",
                    ix, pSegment->sectors, pSegment->name
                    ));
                }
                break;
            case BOOT_BIN_FORMAT_STORE_SEGMENT_PARTITION:
                {
                BootBinFormatStoreSegmentPartitionInfo_t info;
                pSegment->type = BOOT_DOWNLOAD_STORE_SEGMENT_TYPE_PARTITION;
                pSegment->sectors = segmentHeader.sectors;
                if (segmentHeader.infoSize != sizeof(info))
                    {
                    BOOTMSG(ZONE_ERROR, (L"ERROR: BootDownloadBin!ImageTypeStore: "
                        L"Store Image unexpected segment info size (%d)!\r\n",
                        segmentHeader.infoSize
                        ));
                    goto cleanUp;
                    }
                if (!BootTransportRead(
                        pDownload->hTransport, &info, sizeof(info)
                        ))
                    {
                    BOOTMSG(ZONE_ERROR, (L"ERROR: BootDownloadBin!ImageTypeStore: "
                        L"Store Image segment info read from transport failed!\r\n"
                        ));
                    goto cleanUp;
                    }
                pSegment->fileSystem = info.fileSystem;
                BOOTMSG(ZONE_INFO, (
                    L"Segment %d has size %5d sectors, partition 0x%02x\r\n",
                    ix, pSegment->sectors, pSegment->fileSystem
                    ));
                }
                break;
            default:
                BOOTMSG(ZONE_ERROR, (L"ERROR: BootDownloadBin!ImageTypeStore: "
                    L"Store Image unsupported segment type (%d)!\r\n",
                    pSegment->type
                    ));
                goto cleanUp;
            }
        }

    // Check header sum
    switch (pStore->hashType)
        {
        case BOOT_BIN_FORMAT_HASH_SUM:
            {
            // Read hash from transport
            if (!BootTransportRead(
                    pDownload->hTransport, pStore->pHash, pStore->hashSize
                    ))
                {
                BOOTMSG(ZONE_ERROR, (L"ERROR: BootDownloadBin!ImageTypeStore: "
                    L"Failed read image header hash from transport failed!\r\n"
                    ));
                goto cleanUp;
                }
            }
            break;
        default:
            BOOTMSG(ZONE_ERROR, (L"ERROR: BootDownloadBin!ImageTypeStore: "
                L"Unsupported hash type %d!\r\n", pStore->hashType
                ));
            goto cleanUp;
        }

    // Done
    rc = true;

cleanUp:
    BOOTMSG(ZONE_FUNC, (L"-BootDownloadBin!ImageTypeStore(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------

bool_t
ImageType(
    Download_t *pDownload,
    enum_t *pType
    )
{
    bool_t rc = false;
    uint8_t header[7];
    BootNotifyDownloadStart_t info;


    BOOTMSG(ZONE_FUNC, (L"+BootDownloadBin!ImageType\r\n"));

    // At this moment image type is unknown
    pDownload->type = BOOT_DOWNLOAD_IMAGE_UNKNOWN;

    // Read header
    if (!BootTransportRead(pDownload->hTransport, header, sizeof(header)))
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootDownloadBin!ImageType: "
            L"BootTransportRead Failed!\r\n"
            ));
        goto cleanUp;
        }

    // Check if it is supported image
    if (memcmp(header, BOOT_BIN_SIGNATURE_JUMP, sizeof(header)) == 0)
        {
        pDownload->type = BOOT_DOWNLOAD_IMAGE_JUMP;
        info.imageType = BOOT_DOWNLOAD_IMAGE_JUMP;
        info.imageSize = 0;
        OEMBootNotify(
            pDownload->pContext, BOOT_NOTIFY_DOWNLOAD_START, &info, sizeof(info)
            );
        }
    else if (memcmp(header, BOOT_BIN_SIGNATURE_RAM, sizeof(header)) == 0)
        {
        if (!ImageTypeRam(pDownload)) goto cleanUp;
        info.imageType = pDownload->type;
        info.imageSize = pDownload->ram.size;
        OEMBootNotify(
            pDownload->pContext, BOOT_NOTIFY_DOWNLOAD_START, &info, sizeof(info)
            );
        }
    else if (memcmp(header, BOOT_BIN_SIGNATURE_STORE, sizeof(header)) == 0)
        {
        if (!ImageTypeStore(pDownload)) goto cleanUp;
        info.imageType = pDownload->type;
        info.imageSize = pDownload->store.sectorSize * pDownload->store.sectors;
        OEMBootNotify(
            pDownload->pContext, BOOT_NOTIFY_DOWNLOAD_START, &info, sizeof(info)
            );
        }
    else
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootDownloadBin!ImageType: "
            L"Unsupported binary image header!\r\n"
            ));
        goto cleanUp;
        }

    // Done
    rc = true;
    
cleanUp:
    *pType = pDownload->type;
    BOOTMSG(ZONE_FUNC, (L"-BootDownloadBin!ImageType(rc = %d)\r\n", rc));
    return rc;
}



//------------------------------------------------------------------------------

bool_t
GetDataRam(
    Download_t *pDownload,
    void** ppInfo,
    size_t* pInfoSize,
    void** ppData,
    size_t* pDataSize
    )
{
    bool_t rc = false;
    void *pBuffer;
    BootBinFormatRamRecordHeader_t header;
    uint32_t sum, ix;


    // Read record header
    if (!BootTransportRead(pDownload->hTransport, &header, sizeof(header)))
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootDownloadBin!GetDataRam: "
            L"Failed read record header from transport!\r\n"
            ));
        goto cleanUp;
        }

    // Record with start address zero is last one
    if (header.address == 0)
        {
        pBuffer = NULL;
        
        // Notify
        OEMBootNotify(pDownload->pContext, BOOT_NOTIFY_DOWNLOAD_DONE, NULL, 0);
        }
    else
        {
        uint32_t top = pDownload->ram.base + pDownload->ram.size;
        BootNotifyDownloadContinue_t info;

        // Check if record is in image span
        if ((header.address < pDownload->ram.base) ||
            ((header.address + header.length) > top))
            {
            BOOTMSG(ZONE_ERROR, (L"ERROR: BootDownloadBin!GetDataRam: "
                L"Record is located outside image location!\r\n"
                ));
            goto cleanUp;
            }

        // Read record to memory
        pBuffer = (void*)(header.address + pDownload->ram.offset);
        pBuffer = BootPAtoCA(BootImageVAtoPA(pBuffer));
        if (!BootTransportRead(pDownload->hTransport, pBuffer, header.length))
            {
            BOOTMSG(ZONE_ERROR, (L"ERROR: BootDownloadBin!GetDataRam: "
                L"Failed read record from transport driver!\r\n"
                ));
            goto cleanUp;
            }

        // Verify checksum
        sum = 0;
        for (ix = 0; ix < header.length; ix++) sum += ((uint8_t*)pBuffer)[ix];
        if (sum != header.checksum)
            {
            BOOTMSG(ZONE_ERROR, (L"ERROR: BootDownloadBin!GetDataRam: "
                L"Check Sum Error (record address 0x%08x, length 0x%08x)!\r\n",
                header.address, header.length
                ));
            goto cleanUp;
            }

        // Update amount we already downloaded
        pDownload->ram.downloaded += header.length;

        // Notify
        info.shareDone  = (uint32_t)(
            (pDownload->ram.downloaded * (uint64_t)100) / pDownload->ram.size
            );
        OEMBootNotify(
            pDownload->pContext, BOOT_NOTIFY_DOWNLOAD_CONTINUE, 
            &info, sizeof(info)
            );
        }

     // Return information about record
    *ppInfo = NULL;
    *pInfoSize = 0;
    *ppData = pBuffer;
    *pDataSize = header.length;

    // Done
    rc = true;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
GetDataStore(
    Download_t *pDownload,
    void **ppInfo,
    size_t *pInfoSize,
    void **ppData,
    size_t *pDataSize
    )
{
    bool_t rc = false;
    StoreImage_t *pStore = &pDownload->store;
    BootBinFormatStoreRecordHeader_t header;
    BootDownloadStoreSegmentInfo_t *pSegment;
    BootDownloadStoreRecordInfo_t *pInfo = &pDownload->store.recordInfo;
    size_t recordSize;


    BOOTMSG(ZONE_FUNC, (L"+BootDownloadBin!GetDataStore\r\n"));

    // Read record header
    if (!BootTransportRead(pDownload->hTransport, &header, sizeof(header)))
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootDownloadBin!GetDataStore: "
            L"Failed read record header from transport!\r\n"
            ));
        goto cleanUp;
        }

    // Check record segment
    if (header.segment >= pStore->segments)
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootDownloadBin!GetDataStore: "
            L"Invalid segment number in record header!\r\n"
            ));
        goto cleanUp;
        }

    // Check record sector and sectors
    pSegment = &pStore->pSegment[header.segment];
    if ((header.sector >= pSegment->sectors) ||
        (header.sectors > (pSegment->sectors - header.sector)))
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootDownloadBin!GetDataStore: "
            L"Invalid base sector or number of sectors in record header!\r\n"
            ));
        goto cleanUp;
        }

    // Check record size & reallocate buffer if needed
    recordSize = header.sectors * pStore->sectorSize;
    if (recordSize > pStore->recordSize)
        {
        BootFree(pStore->pRecord);
        pStore->recordSize = 0;
        pStore->pRecord = BootAlloc(recordSize);
        if (pStore->pRecord == NULL)
            {
            BOOTMSG(ZONE_ERROR, (L"ERROR: BootDownloadBin!GetDataStore: "
                L"Failed allocate memory for record (size %d)!\r\n", recordSize
                ));
            goto cleanUp;
            }
        pStore->recordSize = recordSize;
        }

    // Read record data
    if (!BootTransportRead(pDownload->hTransport, pStore->pRecord, recordSize))
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootDownloadBin!GetDataStore: "
            L"Failed read record data from transport!\r\n"
            ));
        goto cleanUp;
        }
    
    // Read record hash
    if (!BootTransportRead(
            pDownload->hTransport, pStore->pHash, pStore->hashSize
            ))
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootDownloadBin!GetDataStore: "
            L"Failed read record hash from transport!\r\n"
            ));
        goto cleanUp;
        }

    // Verify hash/checksum
    switch (pStore->hashType)
        {
        case BOOT_BIN_FORMAT_HASH_SUM:
            {
            uint32_t sum1 = 0, sum2;
            size_t ix;

            for (ix = 0; ix < sizeof(header); ix++)
                sum1 += ((uint8_t*)&header)[ix];
            for (ix = 0; ix < recordSize; ix++)
                sum1 += pStore->pRecord[ix];
            memcpy(&sum2, pStore->pHash, sizeof(sum2));
            if (sum1 != sum2)
                {
                BOOTMSG(ZONE_ERROR, (L"ERROR: BootDownloadBin!GetDataStore: "
                    L"Check Sum Error (segment %d, sector %d, sectors %d)!\r\n",
                    header.segment, header.sector, header.sectors
                    ));
                goto cleanUp;
                }
            }
            break;
        default:
            BOOTMSG(ZONE_ERROR, (L"ERROR: BootDownloadBin!GetDataStore: "
                L"Unsupported checksum/hash algorithm!\r\n"
                ));
            goto cleanUp;
        }

    // Fill record info
    pInfo->segment = header.segment;
    pInfo->sector  = header.sector;
    pInfo->sectors = header.sectors;

     // Return information about record
    *ppInfo = pInfo;
    *pInfoSize = sizeof(*pInfo);
    *ppData = pStore->pRecord;
    *pDataSize = recordSize;

    // Update info about downloaded sectors
    pDownload->store.downloaded += header.sectors;

    // Notify about download state
    if (pDownload->store.downloaded < pDownload->store.sectors)
        {
        BootNotifyDownloadContinue_t info;
        info.shareDone  = pDownload->store.downloaded * 100;
        info.shareDone /= pDownload->store.sectors;
        OEMBootNotify(
            pDownload->pContext, BOOT_NOTIFY_DOWNLOAD_CONTINUE, 
            &info, sizeof(info)
            );
        }
    else
        {
        OEMBootNotify(pDownload->pContext, BOOT_NOTIFY_DOWNLOAD_DONE, NULL, 0);
        }
    

    // Done
    rc = true;

cleanUp:
    BOOTMSG(ZONE_FUNC, (L"-BootDownloadBin!GetDataStore = %d\r\n", rc));
    return rc;
}


//------------------------------------------------------------------------------

bool_t
GetData(
    Download_t *pDownload,
    void** ppInfo,
    size_t* pInfoSize,
    void** ppData,
    size_t* pDataSize
    )
{
    bool_t rc = false;

    switch (pDownload->type)
        {
        case BOOT_DOWNLOAD_IMAGE_RAM:
            rc = GetDataRam(pDownload, ppInfo, pInfoSize, ppData, pDataSize);
            break;
        case BOOT_DOWNLOAD_IMAGE_STORE:
            rc = GetDataStore(pDownload, ppInfo, pInfoSize, ppData, pDataSize);
            break;
        }

    return rc;
}

//------------------------------------------------------------------------------

bool_t
RamInfo(
    Download_t *pDownload,
    BootDownloadRamImageInfo_t *pInfo
    )
{
    bool_t rc = false;

    if (pDownload->type != BOOT_DOWNLOAD_IMAGE_RAM) goto cleanUp;

    pInfo->address = pDownload->ram.base;
    pInfo->size = pDownload->ram.size;
    pInfo->start = pDownload->ram.start;
    pInfo->offset = pDownload->ram.offset;

    // Done
    rc = true;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
RamOffset(
    Download_t *pDownload,
    BootDownloadRamOffsetParams_t *pInfo
    )
{
    bool_t rc = false;

    if (pDownload->type != BOOT_DOWNLOAD_IMAGE_RAM) goto cleanUp;

    // Store offset
    pDownload->ram.offset = pInfo->offset;

    // Done
    rc = true;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
StoreInfo(
    Download_t *pDownload,
    BootDownloadStoreInfo_t *pInfo
    )
{
    bool_t rc = false;

    if (pDownload->type != BOOT_DOWNLOAD_IMAGE_STORE) goto cleanUp;

    pInfo->sectorSize = pDownload->store.sectorSize;
    pInfo->sectors = pDownload->store.sectors;
    pInfo->segments = pDownload->store.segments;
    pInfo->pSegment = pDownload->store.pSegment;

    // Done
    rc = true;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

// Pass the request for OS information down to the transport
bool_t
GetOSInfo(
    Download_t *pDownload,
    BootDownloadGetOsInfoParams_t *pInfo
    )
{
    bool_t rc = false;
    enum_t type = 0;

    // Map the Download driver constant to the Tranport layer constant 
    // When they match the compiler should change this to a nop
    switch(pInfo->type)
        {
        case BOOT_DOWNLOAD_GET_OS_INFO_TYPE_PASSIVE_KITL:
            type = BOOT_TRANSPORT_READ_OS_CONFIG_TYPE_PASSIVE_KITL;
            break;
        case BOOT_DOWNLOAD_GET_OS_INFO_TYPE_KITL_TRANSPORT:
            type = BOOT_TRANSPORT_READ_OS_CONFIG_TYPE_KITL_TRANSPORT;
            break;
        default:
            goto cleanUp;
        }
                
    // Pass the call down to the transport which has the OS Info from the download
    rc = BootTransportGetOSConfigInfo(pDownload->hTransport, type, pInfo->pBuffer,
            pInfo->BufferSize);

cleanUp:    
    return rc;
}

//------------------------------------------------------------------------------


