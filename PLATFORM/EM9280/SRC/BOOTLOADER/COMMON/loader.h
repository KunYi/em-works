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
//-----------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File: loader.h
//
//  This file contains definitions specific to the bootloader.
//
//------------------------------------------------------------------------------
#ifndef _LOADER_H_
#define _LOADER_H_

#pragma warning(push)
#pragma warning(disable: 4201)
#include <blcommon.h>
#include <halether.h>
#include <image_cfg.h>
#include <oal_kitl.h>
#include "menu.h"
#pragma warning(pop)

//------------------------------------------------------------------------------
// Defines
//
#define EBOOT_VERSION_MAJOR                 1
#define EBOOT_VERSION_MINOR                 0
#define EBOOT_CFG_MAGIC_NUMBER              0x01020304

#define SBOOT_VERSION_MAJOR                 1
#define SBOOT_VERSION_MINOR                 0
#define SBOOT_CFG_MAGIC_NUMBER              0x04030201

#define BOOT_TOTAL_RAM_SIZE                0x200000  // see eboot.bib/sboot.bib to compute size
#define BOOT_FLASHBLOCK_CACHE_START        ((UINT32) OALPAtoVA(IMAGE_BOOT_RAMDEV_RAM_PA_START, FALSE) + BOOT_TOTAL_RAM_SIZE)


#define XLDR_NB0_FILE                       "xldr.nb0"
#define XLDR_NB0_FILE_LEN                   8   // 8 chars in name
#define EBOOT_NB0_FILE                      "eboot.nb0"
#define EBOOT_NB0_FILE_LEN                  9   // 9 chars in name
#define EBOOT_MSB_FILE                      "eboot.msb"
#define EBOOT_MSB_FILE_LEN                  9   // 9 chars in name
#define EBOOT_IVT_MSB_FILE                  "eboot_ivt.msb"
#define EBOOT_IVT_MSB_FILE_LEN              13  // 13 chars in name
#define SBOOT_NB0_FILE                      "sboot.nb0"
#define SBOOT_NB0_FILE_LEN                  9   // 9 chars in name
#define NK_NB0_FILE                         "nk.nb0"
#define NK_NB0_FILE_LEN                     6   // 6 chars in name
#define NK_MSB_FILE                         "nk.msb"
#define NK_MSB_FILE_LEN                     6   // 6 chars in name


#define BOOT_CFG_AUTODOWNLOAD_NONE          0
#define BOOT_CFG_AUTODOWNLOAD_NK_NOR        1
#define BOOT_CFG_AUTODOWNLOAD_NK_NAND       2
#define BOOT_CFG_AUTODOWNLOAD_IPL_NAND      3
#define BOOT_CFG_AUTODOWNLOAD_NK_SD         4

#define SBOOT_PARITY_EVEN                   0
#define SBOOT_PARITY_ODD                    1
#define SBOOT_PARITY_NONE                   2

#define SBOOT_FLOWCTRL_OFF                  0
#define SBOOT_FLOWCTRL_ON                   1

#define SBOOT_DATABITS_7                    0
#define SBOOT_DATABITS_8                    1

#define SBOOT_STOPBITS_1                    0
#define SBOOT_STOPBITS_2                    1

#define BOOT_CFG_MODE_ETHERNET              0
#define BOOT_CFG_MODE_SERIAL                1

#define SBOOT_BAUDRATE_9600                 9600
#define SBOOT_BAUDRATE_19200                19200
#define SBOOT_BAUDRATE_38400                38400
#define SBOOT_BAUDRATE_57600                57600
#define SBOOT_BAUDRATE_115200               115200


#define DEFAULT_SBOOT_CHANNEL               3
#define DEFAULT_SBOOT_BAUDRATE              SBOOT_BAUDRATE_115200
#define DEFAULT_SBOOT_BASE_REG              CSP_BASE_REG_PA_UART3

#define FLASH_UUID_STR_SIZE                 (100 + 4)       // 'UUID' + Stored UUID string 
#define UUID_SIZE                           4               // 'UUID' Block recognizing string

//------------------------------------------------------------------------------
// Structure definitions.
//
//------------------------------------------------------------------------------
// Structure definitions.
//
// The BOOT_CFG structure holds a variety of configuration parameters.
// When adding new parameters, make sure that the size of the structure in bytes is
// an integral number of WORDS.  Pad the structure if necessary.
// Add new members at the end. Also remember to increment the EBOOT version
// numbers.
#define EBOOT_CFG_MAGIC_NUMBER              0x01020304
#define EBOOT_CFG_AUTODOWNLOAD_NONE         0
#define EBOOT_CFG_AUTODOWNLOAD_NK_NOR       1
#define EBOOT_CFG_AUTODOWNLOAD_NK_NAND      2
#define EBOOT_CFG_AUTODOWNLOAD_IPL_NAND     3

#define CONFIG_FLAGS_KITL_INT                   (1 << 0)
#define CONFIG_FLAGS_KITL_PASSIVE               (1 << 1)
#define CONFIG_FLAGS_USBKITL                    (1 << 2)

#define SDBG_USB_SERIAL   101

typedef struct
{
    UINT32 recordOffset;
    UINT8  *pReadBuffer;
    UINT32 readSize;
} BOOT_BINDIO_CONTEXT;

//------------------------------------------------------------------------------
// Enumerated type definitions.
//

typedef enum
{
    IMAGE_TYPE_NK,
    IMAGE_TYPE_BOOT,
    IMAGE_TYPE_IPL,
    IMAGE_TYPE_BINDIO,
    IMAGE_TYPE_XLDR
} IMAGE_TYPE, *PIMAGE_TYPE;

typedef enum
{
    IMAGE_MEMORY_RAM,
    IMAGE_MEMORY_NAND,
    IMAGE_MEMORY_NOR,
    IMAGE_MEMORY_SD
} IMAGE_MEMORY, *PIMAGE_MEMORY;


//------------------------------------------------------------------------------
// External functions
//
extern void OEMBootInit();
extern BOOL GetPreDownloadInfo(PBOOT_CFG p_bootCfg);
extern void GetLaunchInfo(void);
extern void SpinForever(void);
extern void OEMRepeatDebugByte(UINT8 ch, UINT32 repeatCnt);
extern DWORD OEMEthGetSecs(void);
extern void BootTimerInit(void);
extern UINT32 InitSpecifiedEthDevice(OAL_KITL_ARGS *pKITLArgs, UINT32 EthDevice);
extern BOOL FlashLoadBootCFG(BYTE *pBootCfg, DWORD cbBootCfgSize);
extern BOOL FlashStoreBootCFG(BYTE *pBootCfg, DWORD cbBootCfgSize);

//#define IMAGE_BOOT_BOOTCFG_NAND_SIZE  FLASH_ARGS_LENGTH

#define SGTL_FLASH_PHYS_BASE_ADDRESS    0//0x60000000
#define SGTL_FLASH_SIZE                 0x4000000

// BCB region:
#define FLASH_BCB_ADDRESS               (SGTL_FLASH_PHYS_BASE_ADDRESS)
#define FLASH_BCB_LENGTH                0x00300000

// EBOOT Args region:
#define FLASH_ARGS_ADDRESS              (FLASH_BCB_ADDRESS + FLASH_BCB_LENGTH)
#define FLASH_ARGS_LENGTH               0x00040000

// EBOOT region:
#define FLASH_EBOOT_ADDRESS             (FLASH_ARGS_ADDRESS + FLASH_ARGS_LENGTH) 
#define FLASH_EBOOT_LENGTH              0x00100000 

// This should match address in FMD driver 0x3C0000
#define FLASH_UUID_ADDRESS              (FLASH_EBOOT_ADDRESS + FLASH_EBOOT_LENGTH) 
#define FLASH_UUID_SIZE                 0x00040000

// BMP region:
#define FLASH_BMP_IMAGE_ADDRESS         (FLASH_UUID_ADDRESS + FLASH_UUID_SIZE) 
#define FLASH_BMP_IMAGE_SIZE            0x00080000 

// Spare region:
#define FLASH_EXTRA_SPACE               (FLASH_BMP_IMAGE_ADDRESS + FLASH_BMP_IMAGE_SIZE)
#define FLASH_EXTRA_SPACE_LENGTH        0x00040000*8 // Leave 10 block 

// OS (NK.BIN) region:
#define FLASH_OS_ADDRESS                (FLASH_EXTRA_SPACE + FLASH_EXTRA_SPACE_LENGTH)
#define FLASH_OS_LENGTH                 0x03000000




// OEM reserved area bitfield.
#define OEM_BLOCK_RESERVED              0x01
#define OEM_BLOCK_READONLY              0x02

#define NAND_SEARCH_STRIDE              64
#define NAND_BB_SEARCH_NUMBER           20
#define NAND0                           0

#define NCB2_SECTOR_ADDRESS             (0*64)
#define LDLB2_SECTOR_ADDRESS            (4*64)

#define DBBT1_SECTOR_ADDRESS            (8*64)
#define DBBT2_SECTOR_ADDRESS            (10*64)

//for 64 secotr multiply by 2
#define NCB2_BLOCK_ADDRESS              0
#define LDLB2_BLOCK_ADDRESS             4

#define DBBT1_BLOCK_ADDRESS             8
#define DBBT2_BLOCK_ADDRESS             10


#define FLASH_EBOOT_IMAGE_SIZE          0x80

#define FLASH_EBOOT1_SECTOR_ADDRESS     (FLASH_EBOOT_ADDRESS / 2048 )
#define FLASH_EBOOT2_SECTOR_ADDRESS     (FLASH_EBOOT1_SECTOR_ADDRESS + FLASH_EBOOT_IMAGE_SIZE)
//#define FLASH_EBOOT2_SECTOR_ADDRESS   (FLASH_OS_ADDRESS + FLASH_OS_LENGTH) / 2048 )

#define NAND_128_SECTOR_STRIDE          0x80

//Bad blocks & DBBT related defines
#define DBBT_DATA_START_PAGE_OFFSET     4
#define BB_START_NO                     0x02
#define BB_SECTOR_SEARCH_MARGIN         0x03
// SectorsPerBlock(64) - beginning(3 Already searched) - end(Need to search 3) 
#define BB_SECTOR_END_MARGIN            58 


#define RAM_BUFFER_DBBT_2K_PAGE         0x40020000
#define DBBT_MAX_2K_PAGES               (2048*10)

// Chip information definations
#define NAND_1K_PAGE                    1024
#define NAND_2K_PAGE                    (NAND_1K_PAGE*2)
#define NAND_4K_PAGE                    (NAND_1K_PAGE*4)

#define NAND_64K_BLK                    (64*NAND_1K_PAGE)
#define NAND_128K_BLK                   (128*NAND_1K_PAGE)
#define NAND_256K_BLK                   (256*NAND_1K_PAGE)
#define NAND_512K_BLK                   (512*NAND_1K_PAGE)

#define NAND_64_SECTOR_BLK              64
#define NAND_128_SECTOR_BLK             128
#define NAND_256_SECTOR_BLK             256

#define FLASH_SECTOR_BUFFER_SIZE        (NAND_1K_PAGE*4)
#define FLASH_SECTOR_INFO_BUFFER_SIZE   256


extern BOOLEAN g_SerialUSBDownload;
extern BOOLEAN g_StorageSDDownload;


#endif  // _LOADER_H_
