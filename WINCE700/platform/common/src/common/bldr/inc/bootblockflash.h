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
#ifndef __BOOT_BLOCK_FLASH_H
#define __BOOT_BLOCK_FLASH_H

#include <bootBlock.h>

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

enum BootBlockFlashIoCtl_e {
    BOOT_BLOCK_IOCTL_PARTITIONDATA  = BOOT_BLOCK_IOCTL(0x8001)
};

typedef struct BootBlockPartitionDataParams_t {
    enum_t index;
    DWORD partitionCount;
    ULONG partitionType;    
    WCHAR *pPartitionName;  
    ULONGLONG startPhysicalBlock;
    ULONGLONG physicalBlockCount;   
    ULONGLONG logicalBlockCount;          
    ULONG partitionFlags;
} BootBlockPartitionDataParams_t;

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------

handle_t
BootBlockFlashInit(
    uint32_t context,
    enum_t binRegions,
    size_t binRegionSectors[]
    );

//------------------------------------------------------------------------------

__inline
bool_t
BootBlockPartitionData(
    handle_t hDriver,
    enum_t index,     
    DWORD *pPartitionCount,
    ULONG *pPartitionType,              // The partition type used to determine which file system to mount.
    WCHAR *pPartitionName,              // Unique name of the partition.
    ULONGLONG *pStartPhysicalBlock,     // Starting physical block of this partition    
    ULONGLONG *pPhysicalBlockCount,     // Number of physical blocks in this partition.
    ULONGLONG *pLogicalBlockCount,      // Number of logical blocks in this partition.
    ULONG *pPartitionFlags              // Bitmask describing properties of this partition.
                                        // FLASH_PARTITION_FLAG_DIRECT_MAP = 0x00000001,
                                        // FLASH_PARTITION_FLAG_READ_ONLY = 0x00000002,  
                                        // FLASH_PARTITION_FLAG_RESERVED = 0x00000004,
    )
{
    bool_t rc = false;
    BootBlockPartitionDataParams_t params;

    params.index = index;
    if (BootDriverIoCtl(
            hDriver, BOOT_BLOCK_IOCTL_PARTITIONDATA, &params, sizeof(params)
            ))
        {
        if (pPartitionCount != NULL) *pPartitionCount = params.partitionCount;
        if (pPartitionType != NULL) *pPartitionType = params.partitionType;
        if (pPartitionName != NULL) BootStringCchCopy(pPartitionName, 16, params.pPartitionName);
        if (pStartPhysicalBlock != NULL) *pStartPhysicalBlock = params.startPhysicalBlock;
        if (pPhysicalBlockCount != NULL) *pPhysicalBlockCount = params.physicalBlockCount;
        if (pLogicalBlockCount != NULL) *pLogicalBlockCount = params.logicalBlockCount;
        if (pPartitionFlags != NULL) *pPartitionFlags = params.partitionFlags;
        rc = true;        
        }
    return rc;
}

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // __BOOT_BLOCK_FLASH_H
