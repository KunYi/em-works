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

class BootFileSystemFat32_t : public BootFileSystemFat_t {

    friend class BootFileSystemFat_t;

private:

#pragma pack(push, 1)
    struct DirEntry_t {
        uint8_t  fileName[11];                  // 00
        uint8_t  attrib;                        // 0B
        uint8_t  reserved1;                     // 0C
        uint8_t  createTimeFine;                // 0D
        uint16_t createTime;                    // 0E
        uint16_t createDate;                    // 10
        uint16_t accessData;                    // 12
        uint16_t firstClusterHi;                // 14
        uint16_t modifyTime;                    // 16
        uint16_t modifyDate;                    // 18
        uint16_t firstCluster;                  // 1A
        uint32_t fileSize;                      // 1C
        };
#pragma pack(pop)

private:

    BootBlockSegment_t*     m_pSegment;

    size_t                  m_sectorSize;
    size_t                  m_sectorsPerCluster;

    size_t                  m_fatSector;
    size_t                  m_sectorsPerFat;
    enum_t                  m_fats;

    size_t                  m_rootDirCluster;

    size_t                  m_clusterSector;
    size_t                  m_clusters;

    uint8_t*                m_pSector;
    size_t                  m_sector;
    bool_t                  m_sectorDirty;

private:

    BootFileSystemFat32_t(
        );

    virtual
    ~BootFileSystemFat32_t(
        );

    bool_t
    Init(
        BootBlockSegment_t* pSegment,
        size_t sectorSize
        );

    bool_t
    FlushBuffer(
        );

    bool_t
    ReadToBuffer(
        size_t sector
        );

    uint8_t*
    DataSector(
        size_t cluster, 
        size_t sector
        );

    void
    DataSectorDirty(
        size_t cluster, 
        size_t sector
        );
        
    bool_t
    DirEntry(
        size_t cluster,
        size_t entry,
        DirEntry_t** ppEntry
        );

    bool_t
    NextDirEntry(
        size_t& dirCluster,
        size_t& dirEntry,
        bool_t allocate
        );

    void
    DirEntryDirty(
        size_t dirCluster,
        size_t dirEntry
        );

    uint32_t*
    FatLink(
        size_t cluster 
        );

    void
    FatLinkDirty(
        size_t cluster 
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

    size_t
    FreeSpace(
        );
   
public:

    static
    BootFileSystemFat32_t*
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

