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
#ifndef __BOOT_FILESYSTEM_H
#define __BOOT_FILESYSTEM_H

#include <bootDriver.h>
#include <bootFactory.h>

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#define BOOT_FILESYSTEM_IOCTL(i)    BOOT_IOCTL(BOOT_DRIVER_CLASS_FILESYSTEM, i)

enum BootFileSystemIoCtl_e {
    BOOT_FILESYSTEM_IOCTL_OPEN          = BOOT_FILESYSTEM_IOCTL(0x0001),
    BOOT_FILESYSTEM_IOCTL_DELETE        = BOOT_FILESYSTEM_IOCTL(0x0002),
    BOOT_FILESYSTEM_IOCTL_FREE_SPACE    = BOOT_FILESYSTEM_IOCTL(0x0003),
    BOOT_FILESYSTEM_IOCTL_READ          = BOOT_FILESYSTEM_IOCTL(0x0101),
    BOOT_FILESYSTEM_IOCTL_WRITE         = BOOT_FILESYSTEM_IOCTL(0x0102),
    BOOT_FILESYSTEM_IOCTL_SEEK          = BOOT_FILESYSTEM_IOCTL(0x0103),
    BOOT_FILESYSTEM_IOCTL_GET_SIZE      = BOOT_FILESYSTEM_IOCTL(0x0104),
    BOOT_FILESYSTEM_IOCTL_FLUSH         = BOOT_FILESYSTEM_IOCTL(0x0105),
    BOOT_FILESYSTEM_IOCTL_GET_POSITION  = BOOT_FILESYSTEM_IOCTL(0x0106),
};

enum BootFileSystemAccess_e {
    BOOT_FILESYSTEM_ACCESS_READ         = (1 << 0),
    BOOT_FILESYSTEM_ACCESS_WRITE        = (1 << 1),
    BOOT_FILESYSTEM_CREATE_NEW          = (1 << 2),
    BOOT_FILESYSTEM_CREATE_ALWAYS       = (1 << 3),
    BOOT_FILESYSTEM_OPEN_EXISTING       = (1 << 4),
    BOOT_FILESYSTEM_OPEN_ALWAYS         = (1 << 5),
    BOOT_FILESYSTEM_TRUNCATE            = (1 << 6)
};    

enum BootFileSystemAttributes_e {
    BOOT_FILESYSTEM_ATTRIBUTE_READONLY  = (1 << 0),
    BOOT_FILESYSTEM_ATTRIBUTE_HIDDEN    = (1 << 1),
    BOOT_FILESYSTEM_ATTRIBUTE_SYSTEM    = (1 << 2),
    BOOT_FILESYSTEM_ATTRIBUTE_DIRECTORY = (1 << 4),
    BOOT_FILESYSTEM_ATTRIBUTE_ARCHIVE   = (1 << 5),
    BOOT_FILESYSTEM_ATTRIBUTE_NORMAL    = (1 << 7)
};

typedef struct BootFileSystemOpenParams_t {
    wcstring_t name;
    flags_t options;
    flags_t attributes;
    handle_t hFile;
} BootFileSystemOpenParams_t;

typedef struct BootFileSystemDeleteParams_t {
    wcstring_t name;
    flags_t options;
    handle_t hFile;
} BootFileSystemDeleteParams_t;

typedef struct BootFileSystemFreeSpaceParams_t {
    size_t freeSpace;
} BootFileSystemFreeSpaceParams_t;

typedef struct BootFileSystemFindFirstParams_t {
    wstring_t name;
} BootFileSystemFindFirstParams_t;

typedef struct BootFileSystemFindNextParams_t {
    wstring_t name;
} BootFileSystemFindNextParams_t;

typedef struct BootFileSystemReadParams_t {
    void *pBuffer;
    size_t size;
} BootFileSystemReadParams_t;

typedef struct BootFileSystemWriteParams_t {
    void *pBuffer;
    size_t size;
} BootFileSystemWriteParams_t;

enum BootFileSystemSeekMethod_e {
    BOOT_FILESYSTEM_SEEK_FILE_BEGIN = 0,
    BOOT_FILESYSTEM_SEEK_FILE_CURRENT,
    BOOT_FILESYSTEM_SEEK_FILE_END
};    
        
typedef struct BootFileSystemSeekParams_t {
    enum_t method;
    int32_t offset;
} BootFileSystemSeekParams_t;

typedef struct BootFileSystemGetPosParams_t {
    size_t *pPosition;
} BootFileSystemGetPosParams_t;

typedef struct BootFileSystemGetSizeParams_t {
    size_t size;
} BootFileSystemGetSizeParams_t;

//------------------------------------------------------------------------------

#define BootFileSystemDeinit    BootDriverDeinit
#define BootFileSystemIoCtl     BootDriverIoCtl
#define BootFileSystemClose     BootDriverDeinit

//------------------------------------------------------------------------------
//
//  Function:  BootFileSystemOpen
//
__inline
handle_t
BootFileSystemOpen(
    handle_t hDriver,
    wcstring_t name,
    flags_t options,
    flags_t attributes
    )
{
    handle_t hFile = NULL;
    BootFileSystemOpenParams_t params;

    params.name = name;
    params.options = options;
    params.attributes = attributes;
    if (BootDriverIoCtl(
            hDriver, BOOT_FILESYSTEM_IOCTL_OPEN, &params, sizeof(params)
            ))
        {
        hFile = params.hFile;
        }
    return hFile;
}

//------------------------------------------------------------------------------
//
//  Function:  BootFileSystemOpen
//
__inline
bool_t
BootFileSystemDelete(
    handle_t hDriver,
    wcstring_t name
    )
{
    BootFileSystemDeleteParams_t params;

    params.name = name;
    return BootDriverIoCtl(
        hDriver, BOOT_FILESYSTEM_IOCTL_DELETE, &params, sizeof(params)
        );
}

//------------------------------------------------------------------------------
//
//  Function:  BootFileSystemRead
//
__inline
size_t
BootFileSystemRead(
    handle_t hDriver,
    void *pBuffer,
    size_t size
    )
{
    size_t rc = (size_t)-1;
    BootFileSystemReadParams_t params;

    params.pBuffer = pBuffer;
    params.size = size;
    if (BootDriverIoCtl(
            hDriver, BOOT_FILESYSTEM_IOCTL_READ, &params, sizeof(params)
            ))
        {
        rc = params.size;
        }
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  BootFileSystemFreeSpace
//
__inline
size_t
BootFileSystemFreeSpace(
    handle_t hDriver
    )
{
    size_t rc = (size_t)-1;
    BootFileSystemFreeSpaceParams_t params;

    if (BootDriverIoCtl(
            hDriver, BOOT_FILESYSTEM_IOCTL_FREE_SPACE, &params, sizeof(params)
            ))
        {
        rc = params.freeSpace;
        }
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  BootFileSystemRead
//
__inline
size_t
BootFileSystemWrite(
    handle_t hDriver,
    void *pBuffer,
    size_t size
    )
{
    size_t rc = (size_t)-1;
    BootFileSystemReadParams_t params;

    params.pBuffer = pBuffer;
    params.size = size;
    if (BootDriverIoCtl(
            hDriver, BOOT_FILESYSTEM_IOCTL_WRITE, &params, sizeof(params)
            ))
        {
        rc = params.size;
        }
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  BootFileSystemSeek
//
__inline
size_t
BootFileSystemSeek(
    handle_t hDriver,
    enum_t method,
    int32_t fOffset
    )
{
    BootFileSystemSeekParams_t params;

    params.method = method;
    params.offset = fOffset;
    return BootDriverIoCtl(
        hDriver, BOOT_FILESYSTEM_IOCTL_SEEK, &params, sizeof(params)
        );
}

//------------------------------------------------------------------------------
//
//  Function:  BootFileSystemGetPos
//
__inline
size_t
BootFileSystemGetPos(
    handle_t hDriver,
    size_t *pPosition
    )
{
    BootFileSystemGetPosParams_t params;

    params.pPosition = pPosition;
    return BootDriverIoCtl(
        hDriver, BOOT_FILESYSTEM_IOCTL_GET_POSITION, &params, sizeof(params)
        );
}

//------------------------------------------------------------------------------
//
//  Function:  BootFileSystemGetSize
//
__inline
size_t
BootFileSystemGetSize(
    handle_t hDriver,
    size_t *pSize
    )
{
    BootFileSystemGetSizeParams_t params;
    size_t rc;

    if ((rc = BootDriverIoCtl(
        hDriver, BOOT_FILESYSTEM_IOCTL_GET_SIZE, &params, sizeof(params)
        )))
        *pSize = params.size;

    return rc;
}
//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // __BOOT_FILESYSTEM_H
