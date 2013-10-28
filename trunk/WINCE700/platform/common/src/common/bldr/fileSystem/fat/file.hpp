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
#include "fat.hpp"

namespace ceboot {

//------------------------------------------------------------------------------

class BootFileSystemFileFat_t : public BootFileSystemFile_t {

    friend class BootFileSystemFat_t;

private:

    BootFileSystemFat_t*    m_pFileSystem;
    flags_t                 m_access;

    size_t                  m_dirCluster;
    size_t                  m_dirEntry;
    size_t                  m_fileCluster;
    size_t                  m_fileSize;

    size_t                  m_sectorSize;
    size_t                  m_sectorsPerCluster;

    bool_t                  m_fileSizeChanged;

    size_t                  m_pos;
    size_t                  m_posCluster;
    size_t                  m_posSector;
    size_t                  m_posOffset;
    
    uint8_t*                m_pBuffer;
    size_t                  m_bufferCluster;
    size_t                  m_bufferSector;
    bool_t                  m_bufferDirty;
    
private:

    BootFileSystemFileFat_t(
        BootFileSystemFat_t *pFileSystem,
        flags_t access
        );

    ~BootFileSystemFileFat_t(
        );

    bool_t
    Init(
        size_t sectorSize,
        size_t sectorsPerCluster,
        size_t dirCluster,
        size_t dirEntry,
        size_t fileSize,
        size_t fileCluster
        );

    bool_t
    FlushSectorBuffer(
        );

    bool_t
    CopyFromSectorBuffer(
        void** ppBuffer,
        size_t cluster,
        size_t sector,
        size_t* pOffset,
        size_t size
        );

    bool_t
    CopyToSectorBuffer(
        size_t cluster,
        size_t sector,
        size_t* pOffset,
        void** ppBuffer,
        size_t size
        );

    size_t
    Read(
        void *pBuffer,
        size_t size
        );

    size_t
    Write(
        void *pBuffer,
        size_t size
        );

    bool_t
    Seek(
        enum_t method,
        size_t pos
        );

    size_t
    GetSize(
        );
    
    bool_t
    Flush(
        );

public:

    static
    BootFileSystemFileFat_t*
    Create(
        BootFileSystemFat_t *pFileSystem,
        flags_t access,
        size_t sectorSize,
        size_t sectorsPerCluster,
        size_t dirCluster,
        size_t dirEntry,
        size_t fileSize,
        size_t fileCluster
        );
    
    virtual
    bool_t
    __cdecl
    DeInit(
        );

    virtual
    bool_t
    __cdecl
    IoCtl(
        enum_t code,
        void *pBuffer,
        size_t size
        );
    
};

//------------------------------------------------------------------------------

}; // ceboot
