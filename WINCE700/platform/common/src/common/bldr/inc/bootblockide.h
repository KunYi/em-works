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
#ifndef __BOOT_BLOCK_IDE_H
#define __BOOT_BLOCK_IDE_H

#include <bootBlock.h>

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

enum BootBlockBiosIoCtl_e {
    BOOT_BLOCK_IOCTL_DATA_SECTORS = BOOT_BLOCK_IOCTL(0x8001) // DataSectorsIoCtl
};

typedef struct BootBlockDataSectorParams_t {
    size_t sector;
    size_t sectors;
} BootBlockDataSectorsParams_t;

//------------------------------------------------------------------------------

handle_t
BootBlockIdeInit(
    void *pBaseRegs,
    void *pAltRegs,
    enum_t device
    );

//------------------------------------------------------------------------------

__inline
bool_t
BootBlockDataSectors(
    handle_t hSection,
    size_t *pSector,
    size_t *pSectors
    )
{
    bool_t rc = false;
    BootBlockDataSectorsParams_t params;

    if (BootDriverIoCtl(
            hSection, BOOT_BLOCK_IOCTL_DATA_SECTORS, &params, sizeof(params)
            ))
        {
        if (pSector != NULL) *pSector = params.sector;
        if (pSectors != NULL) *pSectors = params.sectors;
        rc = true;
        }
    return rc;
}

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // __BOOT_BLOCK_IDE_H

