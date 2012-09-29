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
// Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
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

//#define BOOT_TOTAL_RAM_SIZE                0x100000  // see eboot.bib/sboot.bib to compute size
//
// CS&ZHL MAY-7-2011: add 512KB to EBoot's RAM, so...
//
#define BOOT_TOTAL_RAM_SIZE						0x200000  // see eboot.bib/sboot.bib to compute size
#define BOOT_FLASHBLOCK_CACHE_START		((UINT32) OALPAtoVA(IMAGE_BOOT_RAMDEV_RAM_PA_START, FALSE) + BOOT_TOTAL_RAM_SIZE)

#define XLDR_NB0_FILE                       "xldr.nb0"
#define XLDR_NB0_FILE_LEN                   8   // 8 chars in name
#define EBOOT_NB0_FILE                      "eboot.nb0"
#define EBOOT_NB0_FILE_LEN                  9   // 9 chars in name
#define SBOOT_NB0_FILE                      "sboot.nb0"
#define SBOOT_NB0_FILE_LEN                  9   // 9 chars in name

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

#define SDBG_USB_SERIAL   101

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
    IMAGE_MEMORY_SD,
    IMAGE_MEMORY_I2C,
    IMAGE_MEMORY_SPI
} IMAGE_MEMORY, *PIMAGE_MEMORY;


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


typedef struct
{
    UINT32 recordOffset;
    UINT8  *pReadBuffer;
    UINT32 readSize;
} BOOT_BINDIO_CONTEXT;

//------------------------------------------------------------------------------
// User config flags selected in bootloader
//------------------------------------------------------------------------------

//#define CONFIG_FLAGS_DEFAULT \
//( \
//    CONFIG_FLAGS_KITL_INT | CONFIG_FLAGS_KITL_ENABLE \
//)
// CS&ZHL MAY-24-2011: try...
#define CONFIG_FLAGS_DEFAULT		CONFIG_FLAGS_KITL_INT

//------------------------------------------------------------------------------
// External functions
//
extern void OEMBootInit();
extern BOOL GetPreDownloadInfo (PBOOT_CFG p_bootCfg);
extern void GetLaunchInfo (void);
extern void SpinForever(void);
extern void BootTimerInit(void);
extern BOOL CPLDInit(void);
extern UINT32 InitSpecifiedEthDevice(OAL_KITL_ARGS *pKITLArgs, UINT32 EthDevice);
extern BOOL FlashLoadBootCFG(BYTE *pBootCfg, DWORD cbBootCfgSize);
extern BOOL FlashStoreBootCFG(BYTE *pBootCfg, DWORD cbBootCfgSize);

extern BOOLEAN g_SerialUSBDownload;

#endif  // _LOADER_H_
