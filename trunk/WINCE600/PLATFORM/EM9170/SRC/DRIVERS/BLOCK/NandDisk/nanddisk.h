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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:  

    nanddisk.h

Abstract:  

    This module contains the function prototypes and constant, type,
    global data and structure definitions for the Windows CE Nand disk driver.

Functions:

Notes:


--*/

#ifndef _NANDDISK_H_
#define _NANDDISK_H_

#include <windows.h>
#include <types.h>
#include <excpt.h>
#include <tchar.h>
#include <devload.h>
#include <diskio.h>
#include <storemgr.h>
#include <bootpart.h>
#include "bsp.h"
#include "common_nandfmd.h"


#ifdef __cplusplus
extern "C" {
#endif

//#define BYTES_PER_SECTOR			NAND_PAGE_SIZE

#define DEFAULT_SECTOR_SIZE	512

// Partition
#define NUM_PARTS						4
#define SIZE_END_SIG					2
#define PARTTABLE_OFFSET			(DEFAULT_SECTOR_SIZE - SIZE_END_SIG - (sizeof(PARTENTRY) * NUM_PARTS))

//
// Structure to keep track of a disk.  NUM_MEM_WINDOWS are maintained and
// remapped as needed.  One of these will remain fixed at location 0 to make
// FAT file system faster (the FAT is at the beginning of the disk).
//
typedef struct _DISK {
    struct _DISK * d_next;
    CRITICAL_SECTION d_DiskCardCrit;// guard access to global state and card    
    DISK_INFO d_DiskInfo;    // for DISK_IOCTL_GET/SETINFO
    DWORD d_StartBlock;
    DWORD d_TotalSize;
    LPWSTR d_ActivePath;    // registry path to active key for this device
} DISK, * PDISK; 


#ifdef DEBUG
//
// Debug zones
//
#define ZONE_FUNCTION   DEBUGZONE(2)
#define ZONE_INIT       DEBUGZONE(3)
#define ZONE_IO         DEBUGZONE(5)

#endif  // DEBUG

#ifdef __cplusplus
}
#endif

#endif // _NANDDISK_H_

