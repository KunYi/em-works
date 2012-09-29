//-----------------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//  Use of this source code is subject to the terms of the Microsoft end-user
//  license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
//  If you did not accept the terms of the EULA, you are not authorized to use
//  this source code. For a copy of the EULA, please see the LICENSE.RTF on
//  your install media.
//
//-----------------------------------------------------------------------------
//
//  Copyright (C) 2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  sdmmc.h
//
//
//-----------------------------------------------------------------------------


#ifndef _SDMMC_BOOT_H
#define _SDMMC_BOOT_H

#pragma pack(push, 1)
#define DEFAULT_SECTOR_SIZE     512

// PARTITION defines

// Part_BootInd

#define PART_BOOTABLE           0x80    // bootable partition
#define PART_ACTIVE             PART_BOOTABLE
#define BOOT_IND_ACTIVE         PART_BOOTABLE
#define PART_NON_BOOTABLE       0       // non-bootable partition
#define BOOT_IND_INACTIVE       PART_NON_BOOTABLE

#define PART_READONLY   0x02

// Part_FileSystem

#define PART_UNKNOWN            0
#define PART_DOS2_FAT           0x01    // legit DOS partition
#define PART_DOS3_FAT           0x04    // legit DOS partition
#define PART_EXTENDED           0x05    // legit DOS partition
#define PART_DOS4_FAT           0x06    // legit DOS partition
#define PART_DOS32              0x0B    // legit DOS partition (FAT32)
#define PART_DOS32X13           0x0C    // Same as 0x0B only "use LBA"
#define PART_DOSX13             0x0E    // Same as 0x06 only "use LBA"
#define PART_DOSX13X            0x0F    // Same as 0x05 only "use LBA"

// CE-only definitions for Part_FileSystem

#define PART_CE_HIDDEN          0x18
#define PART_BOOTSECTION        0x20
#define PART_BINFS              0x21    // BINFS file system
#define PART_XIP                0x22    // XIP ROM Image
#define PART_ROMIMAGE           0x22    // XIP ROM Image (same as PART_XIP)
#define PART_RAMIMAGE           0x23    // XIP RAM Image
#define PART_IMGFS              0x25    // IMGFS file system
#define PART_BINARY             0x26    // Raw Binary Data

#define MAX_PARTTABLE_ENTRIES   4
#define SIZE_PARTTABLE_ENTRIES  16

#define PARTTABLE_OFFSET        (DEFAULT_SECTOR_SIZE - 2 - (SIZE_PARTTABLE_ENTRIES * MAX_PARTTABLE_ENTRIES))

typedef __unaligned struct _PARTITION_TABLE{
    UINT32  BootIndicator:8;
    UINT32  StartingHead:8;
    UINT32 StartingSector:6;
    UINT32 StartingCylinder:10;
    UINT32  SystemId:8;
    UINT32  EndingHead:8;
    UINT32 EndingSector:6;
    UINT32 EndingCylinder:10;
    UINT32 RelativeSector;
    UINT32 TotalSector;
}PARTITION_TABLE, *PPARTITION_TABLE;

typedef __unaligned struct _MASTERBOOT_RECORD{
    UINT8           BootCode[PARTTABLE_OFFSET];
    PARTITION_TABLE Partition[MAX_PARTTABLE_ENTRIES];
    UINT16          Signature;
}MASTERBOOT_RECORD, *PMASTERBOOT_RECORD;


//below is copied from rom code

#define FIRMWARE_CONFIG_BLOCK_SIGNATURE     (0x00112233) 
#define PRIMARY_TAG                         (0x484C494E)
#define SECONDARY_TAG                       (0x4E494C48)

typedef struct _DriveInfo_t
{
    UINT32    u32ChipNum;             //!< Chip Select, ROM does not use it
    UINT32    u32DriveType;           //!< Always system drive, ROM does not use it
    UINT32    u32Tag;                 //!< Drive Tag
    UINT32    u32FirstSectorNumber;   //!< For BA-NAND devices, this number should be divisible by 4, 
                                        //!< Protocol is set to 4 sectors of 512 bytes. 
                                        //!< Firmware can start at sectors 4, 8, 12, 16,....
    UINT32    u32SectorCount;         //!< Not used by ROM
} DriveInfo_t;

typedef struct _ConfigBlock_t 
{
    UINT32    u32Signature;           //!< Signature 0x00112233
    UINT32    u32PrimaryBootTag;      //!< Primary boot drive identified by this tag
    UINT32    u32SecondaryBootTag;    //!< Secondary boot drive identified by this tag
    UINT32    u32NumCopies;           //!< Num elements in aFWSizeLoc array
    DriveInfo_t aDriveInfo[2];           //!< Let array aDriveInfo be last in this data 
                                        //!< structure to be able to add more drives in future without changing ROM code
} ConfigBlock_t;

#pragma pack(pop)

#endif
