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

#include <fallite.h>
#include <bootpart.h>
#include <bppriv.h>

typedef struct
{
    GUID guidReserved;
    DWORD dwReserved1;
    DWORD dwReserved2;
    DWORD dwReserved3;
    DWORD dwReserved4;
    DWORD dwReserved5;
    DWORD dwReserved6;
    DWORD dwReserved7;
    DWORD dwReserved8;
    DWORD dwReserved9;
    DWORD dwReserved10;
    DWORD dwUpdateModeFlag;

} IMGFS_BOOT_SECTOR, *PIMGFS_BOOT_SECTOR;


// --------------------------------------------------------------------
// --------------------------------------------------------------------
BOOL BP_ReadLogicalSector(HANDLE hPartition, DWORD dwSector, LPBYTE lpBuffer)
{
    static PVOID hFAL = NULL;
    PPARTSTATE pPartState = (PPARTSTATE) hPartition;   
    if (INVALID_HANDLE_VALUE == hPartition) {
        return FALSE;
    }

    if (!hFAL) {
        // initialize the FAL (flash abstraction layer) for logical to
        // physical sector mapping
        hFAL = FAL_Init(NULL);
        if (NULL == hFAL) {
            return FALSE;
        }
    }

    return FAL_ReadSectors(hFAL, dwSector + pPartState->pPartEntry->Part_StartSector, lpBuffer, 1);
}

// --------------------------------------------------------------------
// --------------------------------------------------------------------
BOOL BP_GetUpdateModeFlag(BOOL *pfUpdateMode)
{
    // sectors will be 4KB at the most
    static BYTE sector[0x1000];

    // track whether or not the IMGFS partition is an extended partition
    BOOL fExtendedPartition = FALSE;
    DWORD dwLogicalBootSector = 0;

    if (NULL == pfUpdateMode) {
        return FALSE;
    }

    // open the imgfs partition
    HANDLE hPartition = BP_OpenPartition(0, 0, PART_IMGFS, FALSE, PART_OPEN_EXISTING);
    if (INVALID_HANDLE_VALUE == hPartition) {
        // try again with PART_IMGFS, fActive = TRUE
        hPartition = BP_OpenPartition(0, 0, PART_IMGFS, TRUE, PART_OPEN_EXISTING);
        if (INVALID_HANDLE_VALUE == hPartition) {
            // try again with PART_EXTENDED, fActive = FALSE
            fExtendedPartition = TRUE;
            dwLogicalBootSector = 1;
            hPartition = BP_OpenPartition(0, 0, PART_EXTENDED, FALSE, PART_OPEN_EXISTING);
            if (INVALID_HANDLE_VALUE == hPartition) {
                // try again with PART_EXTENDED, fActive = TRUE
                hPartition = BP_OpenPartition(0, 0, PART_EXTENDED, TRUE, PART_OPEN_EXISTING);
            }
        }
    }

    if (INVALID_HANDLE_VALUE == hPartition) {
        // there is probably no IMGFS partition
        RETAILMSG(1, (TEXT("BP_GetUpdateModeFlag: failed to open IMGFS partition\r\n")));
        return FALSE;
    }

    // We assume that there is only one IMGFS partition and that if it is
    // in the extended partition, it is the first partition present.  If this assumption
    // is not true for your platform, modify the code below to iterate through
    // PBR entries until it finds the IMGFS partition of interest.
    // read logical sector zero of the imgfs partition
    if (!BP_ReadLogicalSector(hPartition, dwLogicalBootSector, sector)) {
        RETAILMSG(1, (TEXT("BP_GetUpdateModeFlag: failed to read bootsector of IMGFS partition\r\n")));
        return FALSE;
    }

    *pfUpdateMode = ((PIMGFS_BOOT_SECTOR)sector)->dwUpdateModeFlag;
    return TRUE;
}


