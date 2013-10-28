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
#include <bootFileSystem.hpp>
#include <bootBlock.hpp>
#include <bootBlockBios.h>

namespace ceboot {

//------------------------------------------------------------------------------

class BootFileSystemFat_t : public BootFileSystem_t {

private:

    enum_t                  m_refCount;

protected:

    BootFileSystemFat_t(
        );
    
    virtual
    ~BootFileSystemFat_t(
        );

public:

    static
    BootFileSystemFat_t*
    BootFileSystemFatInit(
        handle_t hBlock,
        enum_t index
        );

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

    enum_t
    AddRef(
        );

    enum_t
    Release(
        );

    virtual
    bool_t
    __cdecl
    ReadSector(
        size_t cluster,
        size_t sector,
        uint8_t *pBuffer
        ) = 0;

    virtual
    bool_t
    __cdecl
    WriteSector(
        size_t cluster,
        size_t sector,
        uint8_t *pBuffer
        ) = 0;

    virtual
    bool_t
    __cdecl
    Flush(
        ) = 0;
    
    virtual
    size_t
    __cdecl
    NextCluster(
        size_t cluster,
        bool_t allocate = false
        ) = 0;

    virtual
    bool_t
    __cdecl
    ReleaseClusterLink(
        size_t cluster
        ) = 0;
    
    virtual
    bool_t
    __cdecl
    UpdateDirEntry(
        size_t dirCluster,
        size_t dirEntry,
        size_t fileCluster,
        size_t fileSize
        ) = 0;

};

//------------------------------------------------------------------------------

}; // ceboot

