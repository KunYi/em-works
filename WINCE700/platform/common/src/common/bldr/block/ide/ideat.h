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

#include <bootTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 1)

//------------------------------------------------------------------------------

typedef struct {
    uint8_t data;
    union {
        uint8_t feature;
        uint8_t error;
        };
    uint8_t sectorCount;
    uint8_t sector;
    uint8_t cylinderLo;
    uint8_t cylinderHi;
    uint8_t head;
    union {
        uint8_t command;
        uint8_t status;
        };
} IdeBaseRegs_t;

typedef struct {
    uint8_t unused[6];
    union {
        uint8_t status;
        uint8_t ctrl;
        };
    uint8_t address;
} IdeAltRegs_t;

//------------------------------------------------------------------------------

#define IDE_STATUS_BSY          (1 << 7)
#define IDE_STATUS_DRDY         (1 << 6)
#define IDE_STATUS_DWF          (1 << 5)
#define IDE_STATUS_DSC          (1 << 4)
#define IDE_STATUS_DRQ          (1 << 3)
#define IDE_STATUS_CORR         (1 << 2)
#define IDE_STATUS_IDX          (1 << 1)
#define IDE_STATUS_ERR          (1 << 0)

#define IDE_CTRL_SRST           (1 << 2)
#define IDE_CTRL_NIEN           (1 << 1)

#define IDE_HEAD_LBA            (1 << 6)
#define IDE_HEAD_DEVICE_1       (1 << 4)

//------------------------------------------------------------------------------

#define ATA_CMD_RECALIBRATE             0x10
#define ATA_CMD_READ                    0x20
#define ATA_CMD_WRITE                   0x30
#define ATA_CMD_MULTIPLE_READ           0xC4
#define ATA_CMD_MULTIPLE_WRITE          0xC5
#define ATA_CMD_SET_MULTIPLE            0xC6
#define ATA_CMD_SEEK                    0x70
#define ATA_CMD_SET_DRIVE_PARMS         0x91
#define ATA_CMD_IDLE                    0x97
#define ATA_CMD_FLUSH_CACHE             0xE7
#define ATA_CMD_IDENTIFY                0xEC
#define ATA_CMD_ACKMEDIACHANGE          0xDB
#define ATA_CMD_READ_DMA                0xC8
#define ATA_CMD_WRITE_DMA               0xCA
#define ATA_CMD_STANDBY_IMMEDIATE       0xE0
#define ATA_CMD_IDLE_IMMEDIATE          0xE1
#define ATA_CMD_STANDBY                 0xE2
#define ATA_CMD_NEW_IDLE                0xE3
#define ATA_CMD_CHECK_POWER_MODE        0xE5
#define ATA_CMD_SLEEP                   0xE6

//------------------------------------------------------------------------------

typedef struct {
    uint16_t generalConfiguration;            // 00
    uint16_t numberOfCylinders;               // 01
    uint16_t reserved1;                       // 02
    uint16_t numberOfHeads;                   // 03
    uint16_t unformattedBytesPerTrack;        // 04
    uint16_t unformattedBytesPerSector;       // 05
    uint16_t sectorsPerTrack;                 // 06
    uint16_t vendorUnique1[3];                // 07-09
    uint16_t serialNumber[10];                // 10
    uint16_t bufferType;                      // 20
    uint16_t bufferSectorSize;                // 21
    uint16_t numberOfEccBytes;                // 22
    uint16_t firmwareRevision[4];             // 23
    uint16_t modelNumber[20];                 // 27
    uint8_t  maximumBlockTransfer;            // 47
    uint8_t  vendorUnique2;                   // 47
    uint16_t doubleWordIo;                    // 48
    uint16_t capabilities;                    // 49
    uint16_t reserved2;                       // 50
    uint8_t  vendorUnique3;                   // 51
    uint8_t  pioCycleTimingMode;              // 51
    uint8_t  vendorUnique4;                   // 52
    uint8_t  dmaCycleTimingMode;              // 52
    uint16_t translationFieldsValid;          // 53
    uint16_t numberOfCurrentCylinders;        // 54
    uint16_t numberOfCurrentHeads;            // 55
    uint16_t currentSectorsPerTrack;          // 56
    uint32_t currentSectorCapacity;           // 57 & 58
    uint8_t  multiSectorCount;                // 59
    uint8_t  multiSectorSettingValid;         // 59
    uint32_t totalUserAddressableSectors;     // 60
    uint8_t  singleDmaModesSupported;         // 62
    uint8_t  singleDmaTransferActive;         // 62
    uint8_t  multiDmaModesSupported;          // 63
    uint8_t  multiDmaTransferActive;          // 63
    uint8_t  advancedPIOxferreserved;         // 64
    uint8_t  advancedPIOxfer;                 // 64
    uint16_t minimumMultiwordDMATime;         // 65
    uint16_t manuRecomendedDMATime;           // 66
    uint16_t minimumPIOxferTimeWOFlow;        // 67
    uint16_t minimumPIOxferTimeIORDYFlow;     // 68
    uint16_t reservedADVPIOSupport[2];        // 69
    uint16_t typicalProcTimeForOverlay;       // 71
    uint16_t typicalRelTimeForOverlay;        // 72
    uint16_t majorRevisionNumber;             // 73
    uint16_t minorRevisionNumber;             // 74
    uint16_t queueDepth;                      // 75
    uint16_t reserved6[4];                    // 76-79
    uint16_t majorVersionNumber;              // 80
    uint16_t minorVersionNumber;              // 81
    uint16_t commandSetSupported1;            // 82
    uint16_t commandSetSupported2;            // 83
    uint16_t commandSetFeaturesSupported;     // 84
    uint16_t commandSetFeatureEnabled1;       // 85
    uint16_t commandSetFeatureEnabled2;       // 86
    uint16_t commandSetFeatureDefault;        // 87
    uint8_t  ultraDMASupport;                 // 88
    uint8_t  ultraDMAActive;                  // 88
    uint16_t timeRequiredForSecurityErase;    // 89
    uint16_t timeReuiregForEnhancedSecurtity; // 90
    uint16_t currentAdvancePowerMng;          // 91
    uint16_t masterPasswordRevisionCode;      // 92
    uint16_t hardwareResetResult;             // 93
    uint16_t reserved7[33];                   // 94-126
    uint16_t mediaStatusNotification;         // 127
    uint16_t securityStatus;                  // 128
    uint16_t reserved8[31];                   // 129-159
    uint16_t cfaPowerMode1;                   // 160
    uint16_t reserved9[94];                   // 161-254
    uint16_t integrityWord;                   // 255 Checksum & Signature    
    
}AtaIdentifyData_t;

//------------------------------------------------------------------------------

#pragma pack(pop)

#ifdef __cplusplus
}
#endif

