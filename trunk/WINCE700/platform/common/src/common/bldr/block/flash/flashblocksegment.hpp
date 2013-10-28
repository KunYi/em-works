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
#include <bootBlock.hpp>

namespace ceboot {

//------------------------------------------------------------------------------

class BootBlockSegmentFlash_t : public BootBlockSegment_t {

private:

    BootBlockFlash_t*   m_pBlock;
    PARTITION_ID        m_partitionId;

private:

    ~BootBlockSegmentFlash_t(
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

    BootBlockSegmentFlash_t(
        BootBlockFlash_t *pBlock
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
        uint8_t *pBuffer
        );

    bool_t
    Write(
        size_t sector,
        size_t sectors,
        uint8_t *pBuffer
        );

    bool_t
    Erase(
        );

};

//------------------------------------------------------------------------------

}; // ceboot
