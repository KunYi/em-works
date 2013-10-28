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

class BootFileSystemExFat_t : public BootFileSystemFat_t {


private:


#pragma warning(push)
//Under Microsoft extensions (/Ze), you can specify a structure without 
//a declarator as members of another structure or union. These structures 
//generate W4 warning C4201.
#pragma warning(disable:4201) 

#pragma pack(push, 1)

    struct DirEntry_t {
        uint8_t  type;                          // 00
        union {
            struct {
                uint8_t  flags;                 // 01
                uint8_t  reserved[18];          // 02
                uint32_t cluster;               // 16
                union {                         // 18
                    uint64_t size;
                    struct {
                        uint32_t sizeLo;
                        uint32_t sizeHi;
                        };
                    };
                } x81;
            struct {
                uint8_t  reserved1[3];          // 01
                uint32_t checksum;              // 04
                uint8_t  reserved2[12];         // 08
                uint32_t cluster;               // 14
                union {                         // 18
                    uint64_t size;
                    struct {
                        uint32_t sizeLo;
                        uint32_t sizeHi;
                        };
                    };
                } x82;
            struct {
                uint8_t  follows;               // 01
                uint16_t checksum;              // 02
                uint16_t attrib;                // 04
                uint16_t reserved1;             // 06
                union {                         // 08
                    struct {
                        uint32_t doubleSecs:5;
                        uint32_t minute:6;
                        uint32_t hour:5;
                        uint32_t day:5;
                        uint32_t month:4;
                        uint32_t year:7;
                        };
                    uint32_t raw;
                } createTime;
                union {                         // 0C
                    struct {
                        uint32_t doubleSecs:5;
                        uint32_t minute:6;
                        uint32_t hour:5;
                        uint32_t day:5;
                        uint32_t month:4;
                        uint32_t year:7;
                        };
                    uint32_t raw;
                } modifyTime;
                union {                         // 10
                    struct {
                        uint32_t doubleSecs:5;
                        uint32_t minute:6;
                        uint32_t hour:5;
                        uint32_t day:5;
                        uint32_t month:4;
                        uint32_t year:7;
                        };
                    uint32_t raw;
                } accessTime;
                uint8_t  createMs;              // 14
                uint8_t  modifyMs;              // 15
                uint8_t  createZone;            // 16
                uint8_t  modifyZone;            // 17
                uint8_t  accessZone;            // 18
                uint8_t  reserved2[7];          // 19
                } x85;
            struct {
                uint8_t  flags;                 // 01
                uint8_t  reserved1;             // 02
                uint8_t  nameChars;             // 03
                uint16_t nameHash;              // 04
                uint16_t reserved2;             // 06
                union {                         // 08
                    uint64_t length;
                    struct {
                        uint32_t lengthLo;
                        uint32_t lengthHi;
                        };
                    };
                uint32_t reserved3;             // 10
                uint32_t cluster;               // 14
                union {                         // 18
                    uint64_t size;
                    struct {
                        uint32_t sizeLo;
                        uint32_t sizeHi;
                        };
                    };
                } xC0;
            struct {
                uint8_t  reserved1;             // 01
                uint16_t name[15];              // 02
                } xC1;
            };
        } ;
#pragma pack(pop)
#pragma warning(pop)

    struct FileDirEntryEx_t {
        size_t   dirCluster;
        size_t   dirEntry;
        size_t   dirEntries;
        uint16_t nameHash;
        size_t   nameChars;
        wchar_t  name[256];
        size_t   cluster;
        size_t   size;
        };

private:

    BootBlockSegment_t*     m_pSegment;

    size_t                  m_sectorSize;
    size_t                  m_sectorSizeLog2;
    size_t                  m_sectorsPerCluster;
    size_t                  m_sectorsPerClusterLog2;

    size_t                  m_fatSector;
    size_t                  m_sectorsPerFat;
    enum_t                  m_fats;

    size_t                  m_rootDirCluster;

    size_t                  m_clusterSector;
    size_t                  m_clusters;

    size_t                  m_bitmapCluster;
    size_t                  m_bitmapSize;

    size_t                  m_upCaseTableCluster;
    uint32_t                m_upCaseTableSum;
    size_t                  m_upCaseTableSize;

    uint8_t*                m_pSector;
    size_t                  m_sector;
    bool_t                  m_sectorDirty;
    
private:

    BootFileSystemExFat_t(
        );

    ~BootFileSystemExFat_t(
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
    NextDataSector(
        size_t& cluster,
        size_t& sector,
        bool_t allocate
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
    ToUpper(
        wstring_t name
        );

    uint16_t
    NameHash(
        wcstring_t name
        );

    uint16_t
    DirEntryChecksum(
        uint16_t checksum,
        DirEntry_t* pEntry,
        bool_t primary
        );

    bool_t
    NextFileDirEntry(
        size_t& dirCluster,
        size_t& dirEntry,
        FileDirEntryEx_t* pEntry
        );

    bool_t
    AllocateFileDirEntry(
        wcstring_t fileName,
        size_t& dirCluster,
        size_t& dirEntry
        );

    bool_t
    FindFileDirEntry(
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
        size_t cluster
        );

    size_t
    Allocate(
        size_t cluster
        );

    bool_t
    Free(
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
    BootFileSystemExFat_t*
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

