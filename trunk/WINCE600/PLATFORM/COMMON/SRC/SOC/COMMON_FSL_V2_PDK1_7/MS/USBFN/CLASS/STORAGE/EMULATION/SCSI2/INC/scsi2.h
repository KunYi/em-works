//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
/*++

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name: 

        SCSI2.H

Abstract:

        SCSI-2 Definitions.
        
--*/

//------------------------------------------------------------------------------
//
// Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

#ifndef _SCSI2_H_
#define _SCSI2_H_

//
// S C S I - 2   D E V I C E   T Y P E S
//

#define SCSI_DEVICE_DIRECT_ACCESS       0x00    // e.g. disk
#define SCSI_DEVICE_SEQUENTIAL_ACCESS   0x01
#define SCSI_DEVICE_PRINTER             0x02
#define SCSI_DEVICE_PROCESSOR           0x03
#define SCSI_DEVICE_WRITE_ONCE          0x04
#define SCSI_DEVICE_CDROM               0x05
#define SCSI_DEVICE_SCANNER             0x06
#define SCSI_DEVICE_OPTICAL             0x07
#define SCSI_DEVICE_MEDIUM_CHANGER      0x08
#define SCSI_DEVICE_COMM                0x09
#define SCSI_DEVICE_UNKNOWN             0x1F

//
// S C S I - 2   M E D I U M   T Y P E S
//

#define SCSI_MEDIUM_UNKNOWN             0xFF

//
// C D - R O M   M E D I U M   T Y P E S
//

#define MEDIUM_CD_ROM_UNKNOWN   0x00
#define MEDIUM_CD_ROM_120       0x01
#define MEDIUM_CD_DA_120        0x02
#define MEDIUM_CD_MIXED_120     0x03
#define MEDIUM_CD_HYBRID_120    0x04
#define MEDIUM_CD_ROM_80        0x05
#define MEDIUM_CD_DA_80         0x06
#define MEDIUM_CD_MIXED_80      0x07
#define MEDIUM_CD_HYBRID_80     0x08
#define MEDIUM_CDR_ROM_UNKNOWN  0x10
#define MEDIUM_CDR_ROM_120      0x11
#define MEDIUM_CDR_DA_120       0x12
#define MEDIUM_CDR_MIXED_120    0x13
#define MEDIUM_CDR_HYBRID_120   0x14
#define MEDIUM_CDR_ROM_80       0x15
#define MEDIUM_CDR_DA_80        0x16
#define MEDIUM_CDR_MIXED_80     0x17

//
// M A N D A T O R Y   S C S I - 2   C O M M A N D S
//

#define SCSI_TEST_UNIT_READY    0x00
#define SCSI_REQUEST_SENSE      0x03
#define SCSI_INQUIRY            0x12
#define SCSI_SEND_DIAGNOSTIC    0x1D

//
// M A N D A T O R Y   D E V I C E   S P E C I F I C   C O M M A N D S
//

#define SCSI_READ10             0x28
#define SCSI_READ_CAPACITY      0x25
#define SCSI_READ_FORMAT_CAPACITY   0x23  // Read format capacity

//
// O P T I O N A L   D E V I C E   S P E C I F I C   C O M M A N D S
//

#define SCSI_MODE_SELECT6       0x15
#define SCSI_MODE_SENSE6        0x1A
#define SCSI_START_STOP         0x1B
#define SCSI_WRITE10            0x2A
#define SCSI_MODE_SELECT10      0x55
#define SCSI_MODE_SENSE10       0x5A

// XP
#define SCSI_PREVENT_ALLOW_MEDIUM_REMOVAL 0x1E
#define SCSI_VERIFY             0x2F

//
// ( A T A P I )   C D - R O M   C O M M A N D S
//

#define SCSI_CD_READ_TOC        0x43
#define SCSI_CD_PLAY10          0x45
#define SCSI_CD_PLAY_MSF        0x47
#define SCSI_CD_PAUSE_RESUME    0x4B
#define SCSI_CD_STOP            0x4E

//
// M O D E  P A G E S
//

#define MODE_PAGE_FLEXIBLE_DISK 0x05
#define MODE_PAGE_CDROM         0x0D
#define MODE_PAGE_CDROM_AUDIO   0x0E
#define MODE_PAGE_CDROM_CAPS    0x2A

//
// S C S I - 2   S E N S E   K E Y S
//

#define SENSE_NONE              0x00
#define SENSE_RECOVERED_ERROR   0x01
#define SENSE_NOT_READY         0x02
#define SENSE_MEDIUM_ERROR      0x03
#define SENSE_HARDWARE_ERROR    0x04
#define SENSE_ILLEGAL_REQUEST   0x05
#define SENSE_UNIT_ATTENTION    0x06
#define SENSE_DATA_PROTECT      0x07
#define SENSE_BLANK_CHECK       0x08

//
// S C S I - 2   A S C
//

#define ASC_LUN                     0x04
#define ASC_INVALID_COMMAND_FIELD   0x24
#define ASC_MEDIA_CHANGED           0x28
#define ASC_RESET                   0x29
#define ASC_COMMANDS_CLEARED        0x2F
#define ASC_MEDIUM_NOT_PRESENT      0x3A

//
// M A X  L U N
//

#define MAX_LUN      0x7

#define SCSI_CDB_6    6
#define SCSI_CDB_10  10
#define UFI_CDB      12
#define ATAPI_CDB    12
#define MAX_CDB      UFI_CDB



#include <pshpack1.h>

typedef struct _READ_CAPACITY_DATA {
    DWORD LastLogicalBlockAddress;
    DWORD BlockLength;
} READ_CAPACITY_DATA, *PREAD_CAPACITY_DATA;

typedef struct format_capacity_descriptor {
          DWORD LastLogicalBlockAddress;
          DWORD BlockLength;
}READ_FORMAT_CAPACITY_DATA,*PREAD_FORMAT_CAPACITY_DATA;

typedef struct _SENSE_DATA {
    UCHAR ErrorCode:7;
    UCHAR Valid:1;
    UCHAR SegmentNumber;
    UCHAR SenseKey:4;
    UCHAR Reserved:1;
    UCHAR IncorrectLength:1;
    UCHAR EndOfMedia:1;
    UCHAR FileMark:1;
    UCHAR Information[4];
    UCHAR AdditionalSenseLength;
    UCHAR CommandSpecificInformation[4];
    UCHAR AdditionalSenseCode;
    UCHAR AdditionalSenseCodeQualifier;
    UCHAR FieldReplaceableUnitCode;
    UCHAR SenseKeySpecific[3];
} SENSE_DATA, *PSENSE_DATA;

#include <poppack.h>


#endif // _SCSI2_H_

