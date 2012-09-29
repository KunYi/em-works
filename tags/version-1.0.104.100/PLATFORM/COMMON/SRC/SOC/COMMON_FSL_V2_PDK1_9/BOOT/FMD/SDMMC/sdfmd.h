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
//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009 Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
#ifndef __SDFMD_H__
#define __SDFMD_H__

#pragma warning(push)
#pragma warning(disable: 4115 4127 4201 4214)
#include "fmd.h"
#include "sdcard.h"
#include "sdcardddk.h"
#include <oal.h>
#pragma warning(pop)

// hardware specific context
typedef struct _SDH_HARDWARE_CONTEXT {
    BOOL            SendInitClocks;                 // flag to indicate that we need to send the init clock
    BOOL            IsMMC;                          // indicates if the card is MMC or SD
    BOOL            DevicePresent;                  // device is present in the slot
    DWORD           BusWidthSetting;                // 1 bit mode or 4 bit mode
    DWORD           Words_in_fifo ;                 // number of 4 byte words in FIFO buffer
    DWORD           Bytes_in_fifo;                  // FIFO size 
    BOOL            fAppCommandSent;                // Flag if CMD55 sent  
    DWORD           dwClockRate;                    // Current card clock rate
    ULONG           ulReadTimeout;                  // ReadTimeout
    UINT32          dwVddSettingMask ;              // Slot Vdd voltage
    UINT32          dwVoltageWindowMask ;           // Slot voltage window mask
    UINT32          dwOptVoltageMask ;              // Slot optimum voltage 
    UINT32          dwPowerUpDelay ;                // Slot power up delay
    UINT32          dwBlockLen;                     // Block Length

    DWORD           dwNumBytesToTransfer;           // Number of bytes (remaining) to transfer
    DWORD           dwNumWordsToTransfer;           // Number of words (remaining) to transfer
    DWORD           dwMisalignedBytesToTransfer;    // last few bytes which are less than a word
    PUCHAR          pBuffer;                        // Pointer to user data buffer
    SD_BUS_REQUEST  sdRequest;
}SDH_HARDWARE_CONTEXT, *PSDH_HARDWARE_CONTEXT;

extern SDH_HARDWARE_CONTEXT SDController;

typedef struct SDCARD_CARD_REGISTERS {
    UCHAR   OCR[SD_OCR_REGISTER_SIZE];              // SD OCR
    UCHAR   IO_OCR[SD_IO_OCR_REGISTER_SIZE];        // IO OCR
    UCHAR   CID[SD_CID_REGISTER_SIZE];              // CID
    UCHAR   CSD[SD_CSD_REGISTER_SIZE];              // CSD
    UCHAR   SCR[SD_SCR_REGISTER_SIZE];              // SCR
} SDCARD_CARD_REGISTERS;

typedef struct _SD_IMAGE_CFG {
    // XLDR
    LONGLONG dwXldrOffset;
    LONGLONG dwXldrSize;

    // BOOT
    LONGLONG dwBootOffset;
    LONGLONG dwBootSize;

    // NK
    LONGLONG dwNkOffset;
    LONGLONG dwNkSize;

    //Redundant boot
    LONGLONG dwBoot2Offset;
    LONGLONG dwBoot2Size;
	
    // CFG
    LONGLONG dwCfgOffset;
    LONGLONG dwCfgSize;

    LONGLONG dwSdSize;
    
    DWORD dwNkRAMOffset;
    DWORD dwSocId;
}SD_IMAGE_CFG, *PSD_IMAGE_CFG;

enum esdhc_status
{
    ESDHC_STATUS_PASS = 0,
    ESDHC_STATUS_FAILURE = 1
};

// DEFINES

#define SDMMC                                   2       // Flash Type
#define ESDHC_INIT_CLOCK_RATE                   150000  // 150 kHz during card initialization
#define ESDHC_SECTOR_SIZE                       512     // Sector Size
#define ESDHC_BLOCK_CNT                         65536   // Max Number of Blocks
#define ESDHC_SECTOR_CNT                        1       // Number of sectors to constitute a Block
#define ESDHC_DELAY_TIMEOUT                     10      // Timeout in Number of seconds

#define MMC_CMD_ERASE_WR_BLK_START              35
#define MMC_CMD_ERASE_WR_BLK_END                36
#define MMC_OCR_VALUE                           0x40FF8000
#define MMC_OCR_VALUE_MASK                      0x00FF8000
#define MMC_OCR_HC_RES                          0xC0FF8000
#define MMC_OCR_LC_RES                          0x80FF8000
#define CARD_BUSY_BIT                           0x80000000
#define SDHC_BLK_LEN                            ESDHC_SECTOR_SIZE
#define RCA_SHIFT                               16
#define DEFAULT_BUS_REQUEST_RETRY_COUNT         3
#define SD_DEFAULT_BUSY_REQUEST_RETRY_COUNT     10

#define MMC_SPEC_VER_1_0_TO_1_2                 0
#define MMC_SPEC_VER_1_4                        1
#define MMC_SPEC_VER_2_0_TO_2_2                 2
#define MMC_SPEC_VER_3_1_TO_3_3                 3
#define MMC_SPEC_VER_4_0                        4

#define MBR_SIGNATURE                           0xAA55
#define MBR_SIGNATURE_OFFSET                    0x1FE
#define MBR_PART1_STARTSECTOR_OFFSET            0x1C6
#define MBR_PART1_TOTALSECTORS_OFFSET           0x1CA
#define MBR_PART2_STARTSECTOR_OFFSET            0x1D6
#define MBR_PART2_TOTALSECTORS_OFFSET           0x1DA
#define MBR_PART2_FILESYS                       0xB         // For FAT32
#define MBR_PART2_FILESYS_OFFSET                0x1D2

/* eMMC4.3 macros */
#define EMMC_SWITCH_SET_BOOT_PART               ((UINT32)0x01B30000)
#define EMMC_SWITCH_CLEAR_BOOT_PART             ((UINT32)0x02B30000)
#define EMMC_SWITCH_WRITE_BOOT_PART             ((UINT32)0x03B30000)
#define EMMC_SWITCH_BOOT_VALUE_SHIFT            (8)
#define EMMC_CMD62                              (62)

/* MMC EXT_CSD macros */
#define MMC_EXT_CSD_BOOT_CONFIG                 (179)

// eSD 2.1 macros 
#define ESD_CMD_SET_ACTIVE_PART                 (43)
#define ESD_SET_BOOT_PARTITION1                 ((UINT32)0x01000000)
#define ESD_SET_USR_PARTITION0                  ((UINT32)0x00000000)
#define ESD_CMD_MNG_PART                        (44)
#define ESD_MNG_PART_JOIN_PARTS                 ((UINT32)0x21000000)
#define ESD_MNG_PART_SPLIT_PARTS                ((UINT32)0x22000000)
#define ESD_CMD_QUERY_PARTITION                 (45)
#define ESD_QRY_PART_QUERY_SIZES                ((UINT32)0xA1000000)
#define ESD_PART_TYPE_OFFSET                    (256 + 20)
#define ESD_PART_TYPE_BOOT                      (3)


// PROTOTYPES

BOOL SDMMC_Init(void);
BOOL SDMMC_Deinit (void);
UINT32 SDMMC_get_cid (void);
UINT32 SDMMC_get_csd (void);
UINT32 SDMMC_get_ext_csd (void);
UINT32 SDMMC_set_blocklen (UINT32 BlockLen);
UINT32 SDMMC_set_data_transfer_mode (void);
UINT32 SDMMC_card_software_reset(void);
UINT32 SDMMC_card_send_appcmd (void);
UINT32 SDMMC_send_rca (BOOL isMMC);
void SDCard_command_config (PSD_BUS_REQUEST pReq,UCHAR CommandCode, DWORD argument,
                            SD_TRANSFER_CLASS transfer,SD_RESPONSE_TYPE respType);
UINT32 SDMMC_R1b_busy_wait (void);
BOOL SDHC_IsCardPresent(void);
BOOL SDHC_IsCardWriteProtected(void);
BOOL SDMMC_GetInfo(PFlashInfo pFlashInfo);
BOOL SDMMC_EraseBlock(BLOCK_ID blockID, UINT32 num_blocks);
DWORD SDMMC_GetCardCapacity (void);

void SDConfigPins (void);
BOOL SDInterface_Init (void);
void SetClockRate(PDWORD pdwRate);
UINT32 SDHCSendCmdWaitResp (PSD_BUS_REQUEST pReq);
UINT32 SDHCReadResponse (PSD_COMMAND_RESPONSE pResp);
BOOL SDMMC_ReadSector(SECTOR_ADDR startSectorAddr, LPBYTE pSectorBuff,
                      PSectorInfo pSectorInfoBuff, DWORD dwNumSectors);
BOOL SDMMC_WriteSector(SECTOR_ADDR startSectorAddr, LPBYTE pSectorBuff,
                       PSectorInfo pSectorInfoBuff, DWORD dwNumSectors);

UINT32 SDHCDataRead (UINT32* dest_ptr, UINT32 blk_len);
UINT32 SDHCDataWrite (UINT32* dest_ptr, UINT32 blk_len);

UINT32 SD_Init(void);
UINT32 SD_VoltageValidation (void);

UINT32 MMC_Init(void);

UINT32 EMMCToggleBP(void);
UINT32 EMMCToggleBPSize(void);
UINT32 ESDSetActivePartition (DWORD dwPartId);
DWORD ESDQueryPartitionSize(void);
UINT32 ESDToggleBPSize(void);

extern void BSP_GetSDImageCfg(PSD_IMAGE_CFG pSDImageCfg);
extern BOOL BSP_MMC4BitSupported();
#endif      // __SDFMD_H__

