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
#pragma once
#include <bootDriver.hpp>
#include <bootDriverClasses.h>

namespace ceboot {

//------------------------------------------------------------------------------

#define BOOT_FILESYSTEM_IOCTL(i)    BOOT_IOCTL(BOOT_DRIVER_CLASS_FILESYSTEM, i)

//------------------------------------------------------------------------------

class BootFileSystemFile_t;

class BootFileSystem_t : public BootDriver_t {

protected:

    enum IoCtl_e {
        OpenIoCtl           = BOOT_FILESYSTEM_IOCTL(0x0001),
        DeleteIoCtl         = BOOT_FILESYSTEM_IOCTL(0x0002),
        FreeSpaceIoCtl      = BOOT_FILESYSTEM_IOCTL(0x0003),
        };

    struct OpenParams_t {
        wcstring_t name;
        flags_t access;
        flags_t attributes;
        handle_t hFile;
        };

    struct DeleteParams_t {
        wcstring_t name;
        };

    struct FreeSpaceParams_t {
        size_t freeSpace;
        };

public:

    enum AccessFlags_e {
        AccessRead      = (1 << 0),
        AccessWrite     = (1 << 1),
        CreateNew       = (1 << 2),
        CreateAlways    = (1 << 3),
        OpenExisting    = (1 << 4),
        OpenAlways      = (1 << 5),
        Truncate        = (1 << 6)
        };

    enum AttributeFlags_e {
        ReadOnly        = (1 << 0),
        Hidden          = (1 << 1),
        System          = (1 << 2),
        Directory       = (1 << 4),
        Archive         = (1 << 5),
        Normal          = (1 << 7)
        };
    
    virtual
    bool_t
    __cdecl
    DeInit(
        ) = 0;

    virtual
    bool_t
    __cdecl
    IoCtl(
        enum_t code,
        void *pBuffer,
        size_t size
        ) = 0;

    handle_t
    Open(
        wcstring_t name,
        flags_t access,
        flags_t attributes
        ) {
        handle_t hFile = NULL;
        OpenParams_t params;

        params.name = name;
        params.access = access;
        params.attributes = attributes;
        if (IoCtl(OpenIoCtl, &params, sizeof(params)))
            {
            hFile = params.hFile;
            }
        return hFile;
        };

    bool_t
    Delete(
        wcstring_t name
        ) {
        DeleteParams_t params;

        params.name = name;
        return IoCtl(DeleteIoCtl, &params, sizeof(params));
        };

    size_t
    FreeSpace(
        ) {
        size_t freeSpace = (size_t)-1;
        FreeSpaceParams_t params;

        if (IoCtl(FreeSpaceIoCtl, &params, sizeof(params)))
            {
            freeSpace = params.freeSpace;
            }
        return freeSpace;
        };
        
};

//------------------------------------------------------------------------------

class BootFileSystemFile_t : public BootDriver_t {

protected:

    enum IoCtl_e {
        ReadIoCtl       = BOOT_FILESYSTEM_IOCTL(0x0101),
        WriteIoCtl      = BOOT_FILESYSTEM_IOCTL(0x0102),
        SeekIoCtl       = BOOT_FILESYSTEM_IOCTL(0x0103),
        GetSizeIoCtl    = BOOT_FILESYSTEM_IOCTL(0x0104),
        FlushIoCtl      = BOOT_FILESYSTEM_IOCTL(0x0105)
        };

    enum SeekMethod_e {
        SeekBegin       = 0,
        SeekCurrent,
        SeekEnd
        };    

    struct ReadParams_t {
        void *pBuffer;
        size_t size;
        };

    struct WriteParams_t {
        void *pBuffer;
        size_t size;
        };

    struct SeekParams_t {
        enum_t method;
        size_t pos;
        };

    struct GetSizeParams_t {
        size_t size;
        };

public:

    virtual
    bool_t
    __cdecl
    DeInit(
        ) = 0;

    virtual
    bool_t
    __cdecl
    IoCtl(
        enum_t code,
        void *pBuffer,
        size_t size
        ) = 0;

    size_t
    Read(
        void *pBuffer,
        size_t size
        ) {
        size_t rc = (size_t)-1;
        ReadParams_t params;

        params.pBuffer = pBuffer;
        params.size = size;
        if (IoCtl(ReadIoCtl, &params, sizeof(params)))
            {
            rc = params.size;
            }
        return rc;
        };

    size_t
    Write(
        void *pBuffer,
        size_t size
        ) {
        size_t rc = (size_t)-1;
        ReadParams_t params;

        params.pBuffer = pBuffer;
        params.size = size;
        if (IoCtl(WriteIoCtl, &params, sizeof(params)))
            {
            rc = params.size;
            }
        return rc;
        };

    bool_t
    Seek(
        enum_t method,
        size_t pos
        ) {
        SeekParams_t params;

        params.method = method;
        params.pos = pos;
        return IoCtl(SeekIoCtl, &params, sizeof(params));
        };

    size_t
    GetSize(
        ) {
        size_t size = (size_t)-1;
        GetSizeParams_t params;

        if (IoCtl(GetSizeIoCtl, &params, sizeof(params))) size = params.size;
        return size;
        };

    bool_t
    Flush(
        ) {
        return IoCtl(FlushIoCtl, NULL, 0);
        }
    
    bool_t
    Close(
        ) {
        return DeInit();
        };
};

//------------------------------------------------------------------------------

}; // ceboot
