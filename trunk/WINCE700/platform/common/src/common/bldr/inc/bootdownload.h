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
#ifndef __BOOT_DOWNLOAD_H
#define __BOOT_DOWNLOAD_H

#include <bootDriver.h>
#include <bootFactory.h>

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#define BOOT_DOWNLOAD_IOCTL(i)      BOOT_IOCTL(BOOT_DRIVER_CLASS_DOWNLOAD, i)

enum BootDownloadIoctl_e {
    BOOT_DOWNLOAD_IOCTL_IMAGE_TYPE      = BOOT_DOWNLOAD_IOCTL(0x0001),
    BOOT_DOWNLOAD_IOCTL_GET_DATA        = BOOT_DOWNLOAD_IOCTL(0x0002),
    BOOT_DOWNLOAD_IOCTL_RAM_INFO        = BOOT_DOWNLOAD_IOCTL(0x0101),
    BOOT_DOWNLOAD_IOCTL_RAM_OFFSET      = BOOT_DOWNLOAD_IOCTL(0x0102),
    BOOT_DOWNLOAD_IOCTL_STORE_INFO      = BOOT_DOWNLOAD_IOCTL(0x0103),
    BOOT_DOWNLOAD_IOCTL_GET_OS_INFO     = BOOT_DOWNLOAD_IOCTL(0x0104)
};

enum BootDownloadImage_e {
    BOOT_DOWNLOAD_IMAGE_UNKNOWN     =  0,
    BOOT_DOWNLOAD_IMAGE_JUMP        =  1,
    BOOT_DOWNLOAD_IMAGE_RAM         =  2,
    BOOT_DOWNLOAD_IMAGE_STORE       =  3
};

typedef struct BootDownloadImageTypeParams_t {
    enum_t type;
} BootDownloadImageTypeParams_t;

typedef struct BootDownloadGetDataParams_t {
    VOID *pInfo;
    size_t infoSize;
    VOID *pData;
    size_t dataSize;
} BootDownloadGetDataParams_t;

//------------------------------------------------------------------------------

typedef struct BootDownloadRamImageInfo_t {
    uint32_t address;
    uint32_t size;
    uint32_t start;
    uint32_t offset;
} BootDownloadRamImageInfo_t;

typedef struct BootDownloadRamOffsetParams_t {
    uint32_t offset;
} BootDownloadRamOffsetParams_t;

//------------------------------------------------------------------------------

enum BootDownloadStoreSegmentType_e {
    BOOT_DOWNLOAD_STORE_SEGMENT_TYPE_BINARY = 0,
    BOOT_DOWNLOAD_STORE_SEGMENT_TYPE_REGION = 1,
    BOOT_DOWNLOAD_STORE_SEGMENT_TYPE_PARTITION = 2
};

#pragma warning(push)
//Under Microsoft extensions (/Ze), you can specify a structure without 
//a declarator as members of another structure or union. These structures 
//generate W4 warning C4201. 
#pragma warning(disable:4201) 
typedef struct BootDownloadStoreSegmentInfo_t {
    enum_t type;
    size_t sectors;
    union {
        uchar index;
        char name[8];
        uchar fileSystem;
    };        
} BootDownloadStoreSegmentInfo_t;
#pragma warning(pop)

typedef struct BootDownloadStoreInfo_t {
    size_t sectorSize;
    size_t sectors;
    size_t segments;
    BootDownloadStoreSegmentInfo_t *pSegment;
} BootDownloadStoreInfo_t;

typedef struct BootDownloadStoreRecordInfo_t {
    size_t segment;
    size_t sector;
    size_t sectors;
} BootDownloadStoreRecordInfo_t;

enum BootDownloadGetOsInfoType_e {
    BOOT_DOWNLOAD_GET_OS_INFO_TYPE_PASSIVE_KITL = 0,   // boot_t returned
    BOOT_DOWNLOAD_GET_OS_INFO_TYPE_KITL_TRANSPORT = 1  // uint32_t returned
};

typedef struct BootDownloadGetOsInfoParams_t {
    enum_t  type;
    void    *pBuffer;
    size_t  BufferSize;
} BootDownloadGetOsInfoParams_t;

//------------------------------------------------------------------------------

#define BootDownloadDeinit      BootDriverDeinit
#define BootDownloadIoCtl       BootDriverIoCtl

//------------------------------------------------------------------------------
//
//  Function:  BootDownloadImageType
//
__inline
enum_t
BootDownloadImageType(
    handle_t hDriver
    )
{
    ULONG rc = BOOT_DOWNLOAD_IMAGE_UNKNOWN;
    BootDownloadImageTypeParams_t params;

    if (BootDriverIoCtl(
            hDriver, BOOT_DOWNLOAD_IOCTL_IMAGE_TYPE, &params, sizeof(params)
            ))
        {
        rc = params.type;
        }
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  BootDownloadGetData
//
__inline
bool_t
BootDownloadGetData(
    handle_t hDriver,
    VOID **ppInfo,
    size_t *pInfoSize,
    VOID **ppData,
    size_t *pDataSize
    )
{
    bool_t rc;
    BootDownloadGetDataParams_t params;

    rc = BootDriverIoCtl(
            hDriver, BOOT_DOWNLOAD_IOCTL_GET_DATA, &params, sizeof(params)
            );
    if (rc)
        {
        *ppInfo = params.pInfo;
        *pInfoSize = params.infoSize;
        *ppData = params.pData;
        *pDataSize = params.dataSize;
        }
    return rc;
}

//------------------------------------------------------------------------------

__inline
bool_t
BootDownloadRamImageInfo(
    handle_t hDriver,
    BootDownloadRamImageInfo_t *pInfo
    )
{
    return BootDriverIoCtl(
        hDriver, BOOT_DOWNLOAD_IOCTL_RAM_INFO, pInfo, sizeof(*pInfo)
        );
}

//------------------------------------------------------------------------------

__inline
bool_t
BootDownloadRamImageOffset(
    handle_t hDriver,
    uint32_t offset
    )
{
    BootDownloadRamOffsetParams_t info;

    info.offset = offset;
    return BootDriverIoCtl(
        hDriver, BOOT_DOWNLOAD_IOCTL_RAM_OFFSET, &info, sizeof(info)
        );
}
//------------------------------------------------------------------------------

__inline
bool_t
BootDownloadStoreImageInfo(
    handle_t hDriver,
    BootDownloadStoreInfo_t *pInfo
    )
{
    return BootDriverIoCtl(
        hDriver, BOOT_DOWNLOAD_IOCTL_STORE_INFO, pInfo, sizeof(*pInfo)
        );
}

//------------------------------------------------------------------------------

__inline
bool_t
BootDownloadGetOSInfo(
    handle_t hDriver,
    enum_t   type,
    void     *pBuffer,
    size_t   BufferSize
    )
{
    BootDownloadGetOsInfoParams_t params;

    params.type = type;
    params.pBuffer = pBuffer;
    params.BufferSize = BufferSize;
   
    
    return BootDriverIoCtl(
            hDriver, BOOT_DOWNLOAD_IOCTL_GET_OS_INFO, &params, sizeof(params)
            );

}


//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // BOOT_DOWNLOAD
