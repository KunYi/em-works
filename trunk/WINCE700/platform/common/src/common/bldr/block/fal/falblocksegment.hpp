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
#include <bootBlock.hpp>

namespace ceboot {

//------------------------------------------------------------------------------

class BootBlockSegmentFal_t : public BootBlockSegment_t {

private:

    BootBlockFal_t*     m_pBlock;

    enum_t             m_region;
    enum_t             m_sector;
    enum_t             m_sectors;

private:

    ~BootBlockSegmentFal_t(
        );

public:

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

    BootBlockSegmentFal_t(
        BootBlockFal_t *pBlock
        );

    bool_t
    OpenBinaryRegion(
        enum_t index
        );

    bool_t
    OpenReservedRegion(
        const char *name
        );

    bool_t
    OpenPartition(
        uchar fileSystem,
        enum_t index
        );

    bool_t
    Read(
        size_t sector,
        size_t sectors,
        void *pBuffer
        );

    bool_t
    Write(
        size_t sector,
        size_t sectors,
        void *pBuffer
        );

    bool_t
    Erase(
        );

};

//------------------------------------------------------------------------------

}; // ceboot
