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

class BootFileSystemFat12_t : public BootFileSystemFat_t {

    friend class BootFileSystemFat_t;
    
private:

    BootBlockSegment_t*     m_pSegment;

    size_t                  m_sectorSize;
    size_t                  m_sectorsPerCluster;

    size_t                  m_fatSector;
    size_t                  m_sectorsPerFat;
    enum_t                  m_fats;

    size_t                  m_rootDirSector;
    size_t                  m_rootDirEntries;

    size_t                  m_clusterSector;
    size_t                  m_clusters;

    uint8_t*                m_pSector;
    size_t                  m_sector;
    bool_t                  m_sectorDirty;

private:

    BootFileSystemFat12_t(
        );

    virtual
    ~BootFileSystemFat12_t(
        );

    bool_t
    Init(
        BootBlockSegment_t* pSegment,
        size_t sectorSize
        );

    bool_t
    ReadToBuffer(
        size_t sector
        );

    bool_t
    FlushFromBuffer(
        );
        
    bool_t
    SyncFat(
        size_t sectorTo,
        size_t sectorFrom,
        size_t sectors
        );
    
    bool_t
    NormalizeName(
        wcstring_t fileName,
        char* pDirName
        );

    bool_t
    FindOnFat(
        wcstring_t fileName,
        size_t& dirCluster,
        size_t& dirEntry,
        size_t& fileCluster,
        size_t& fileSize,
        bool_t allocate = false
        );

    bool_t
    DeleteDirEntry(
        size_t cluster,
        size_t pos
        );

    size_t
    Next(
        size_t cluster,
        bool_t remove
        );

    size_t
    Allocate(
        size_t cluster
        );

    handle_t
    Open(
        wcstring_t name,
        flags_t access
        );

    bool_t
    Delete(
        wcstring_t name
        );

   
public:

    static
    BootFileSystemFat12_t*
    Create(
        BootBlockSegment_t* pSegment,
        size_t sectorSize
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

    virtual
    bool_t
    __cdecl
    ReadSector(
        size_t cluster,
        size_t sector,
        uint8_t *pBuffer
        );

    virtual
    bool_t
    __cdecl
    WriteSector(
        size_t cluster,
        size_t sector,
        uint8_t *pBuffer
        );

    virtual
    bool_t
    __cdecl
    Flush(
        );
    
    virtual
    size_t
    __cdecl
    NextCluster(
        size_t cluster,
        bool_t allocate = false
        );

    virtual
    bool_t
    __cdecl
    ReleaseClusterLink(
        size_t cluster
        );
    
    virtual
    bool_t
    __cdecl
    UpdateDirEntry(
        size_t dirCluster,
        size_t dirEntry,
        size_t fileCluster,
        size_t fileSize
        );

};

//------------------------------------------------------------------------------

}; // ceboot

