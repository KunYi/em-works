//------------------------------------------------------------------------------
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
//  Copyright (C) 2007-2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  image_cfg.h
//
//  Defines configuration parameters used to create the NK and Bootloader
//  program images.
//
//------------------------------------------------------------------------------
#ifndef __IMAGE_CFG_H
#define __IMAGE_CFG_H

#include "mx28_nandlayout.h"

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//  RESTRICTION
//
//  This file is a configuration file. It should ONLY contain simple #define
//  directives defining constants. This file is included by other files that
//  only support simple substitutions.
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//  NAMING CONVENTION
//
//  The IMAGE_ naming convention ...
//
//  IMAGE_<NAME>_<SECTION>_<MEMORY_DEVICE>_[OFFSET|SIZE|START|END]
//
//      <NAME>          - WINCE, BOOT, SHARE
//      <SECTION>       - section name: user defined
//      <MEMORY_DEVICE> - the memory device the block resides on
//      OFFSET          - number of bytes from memory device start address
//      SIZE            - maximum size of the block
//      START           - start address of block    (device address + offset)
//      END             - end address of block      (start address  + size - 1)
//
//------------------------------------------------------------------------------
#define IMAGE_BOOT_RAM_PA_START         CSP_BASE_MEM_PA_SDRAM


#define IMAGE_WINCE_CODE_RAM_CA_START   ((DWORD)OALPAtoVA(IMAGE_BOOT_RAM_PA_START, TRUE) + 0x00200000)


//------------------------------------------------------------------------------
//// Internal RAM Mapping (128 KB)
#define IMAGE_WINCE_IRAM_PA_START			CSP_BASE_MEM_PA_IRAM
#define IMAGE_WINCE_IRAM_SIZE				(128 * 1024)

// 4K bytes reserved for USB Transfer
#define IMAGE_WINCE_USB_IRAM_OFFSET			(0)
#define IMAGE_WINCE_USB_IRAM_PA_START		(IMAGE_WINCE_IRAM_PA_START+IMAGE_WINCE_USB_IRAM_OFFSET)
#define IMAGE_WINCE_USB_IRAM_SIZE			(4 * 1024)

// 4K bytes reserved for SD/MMC Transfer
#define IMAGE_WINCE_SDMMC_IRAM_OFFSET		(IMAGE_WINCE_USB_IRAM_OFFSET + IMAGE_WINCE_USB_IRAM_SIZE)
#define IMAGE_WINCE_SDMMC_IRAM_PA_START		(IMAGE_WINCE_IRAM_PA_START + IMAGE_WINCE_SDMMC_IRAM_OFFSET)
#define IMAGE_WINCE_SDMMC_IRAM_SIZE			(4 * 1024)

// 12K bytes reserved for NAND Transfer
#define IMAGE_WINCE_NAND_IRAM_OFFSET		(IMAGE_WINCE_SDMMC_IRAM_OFFSET + IMAGE_WINCE_SDMMC_IRAM_SIZE)
#define IMAGE_WINCE_NAND_IRAM_PA_START		(IMAGE_WINCE_IRAM_PA_START + IMAGE_WINCE_NAND_IRAM_OFFSET)
#define IMAGE_WINCE_NAND_IRAM_SIZE			(12 * 1024)

// 1K bytes reserved for POWER 
#define IMAGE_WINCE_POWEROFF_IRAM_OFFSET    (IMAGE_WINCE_NAND_IRAM_OFFSET + IMAGE_WINCE_NAND_IRAM_SIZE)
#define IMAGE_WINCE_POWEROFF_IRAM_PA_START  (IMAGE_WINCE_IRAM_PA_START + IMAGE_WINCE_POWEROFF_IRAM_OFFSET)
#define IMAGE_WINCE_POWEROFF_IRAM_SIZE      (1 * 1024)

// 1K bytes reserved for AUDIO DMA Descriptor
#define IMAGE_WINCE_AUDIO_DESCRIPTOR_IRAM_OFFSET    (IMAGE_WINCE_POWEROFF_IRAM_OFFSET + IMAGE_WINCE_POWEROFF_IRAM_SIZE)
#define IMAGE_WINCE_AUDIO_DESCRIPTOR_IRAM_PA_START  (IMAGE_WINCE_IRAM_PA_START + IMAGE_WINCE_AUDIO_DESCRIPTOR_IRAM_OFFSET)
#define IMAGE_WINCE_AUDIO_DESCRIPTOR_IRAM_SIZE      (1 * 1024)

// 8K bytes reserved for AUDIO DMA Buffer
#define IMAGE_WINCE_AUDIO_IRAM_OFFSET		(IMAGE_WINCE_AUDIO_DESCRIPTOR_IRAM_OFFSET + IMAGE_WINCE_AUDIO_DESCRIPTOR_IRAM_SIZE)
#define IMAGE_WINCE_AUDIO_IRAM_PA_START		(IMAGE_WINCE_IRAM_PA_START + IMAGE_WINCE_AUDIO_IRAM_OFFSET)
#define IMAGE_WINCE_AUDIO_IRAM_SIZE			(8 * 1024)

// 4 K bytes reserved for DVFC 
#define IMAGE_WINCE_DVFC_IRAM_OFFSET		(IMAGE_WINCE_AUDIO_IRAM_OFFSET + IMAGE_WINCE_AUDIO_IRAM_SIZE)
#define IMAGE_WINCE_DVFC_IRAM_PA_START		(IMAGE_WINCE_IRAM_PA_START + IMAGE_WINCE_DVFC_IRAM_OFFSET)
#define IMAGE_WINCE_DVFC_IRAM_DATA_SIZE		(2*1024)
#define IMAGE_WINCE_DVFC_IRAM_FUNC_SIZE		(2*1024)
#define IMAGE_WINCE_DVFC_IRAM_SIZE			(IMAGE_WINCE_DVFC_IRAM_FUNC_SIZE + IMAGE_WINCE_DVFC_IRAM_DATA_SIZE)

// 1 K bytes reserved for Debug 
#define IMAGE_WINCE_DEBUG_IRAM_OFFSET		(IMAGE_WINCE_DVFC_IRAM_OFFSET + IMAGE_WINCE_DVFC_IRAM_SIZE)
#define IMAGE_WINCE_DEBUG_IRAM_PA_START		(IMAGE_WINCE_IRAM_PA_START + IMAGE_WINCE_DEBUG_IRAM_OFFSET)
#define IMAGE_WINCE_DEBUG_IRAM_SIZE			(1 * 1024)



//------------------------------------------------------------------------------
// RAM image defines
#define IMAGE_BOOT_RAMDEV_RAM_PA_START      (IMAGE_BOOT_RAM_PA_START)
#define IMAGE_BOOT_RAMDEV_RAM_SIZE          (128*1024*1024) // 128 MB
#define IMAGE_BOOT_RAMDEV_RAM_PA_END        (IMAGE_BOOT_RAMDEV_RAM_PA_START+IMAGE_BOOT_RAMDEV_RAM_SIZE-1)

// Stack space for boot procedure
#define IMAGE_BOOT_STACK_RAM_OFFSET         (0)
#define IMAGE_BOOT_STACK_RAM_START          (IMAGE_BOOT_RAMDEV_RAM_PA_START+IMAGE_BOOT_STACK_RAM_OFFSET)
#define IMAGE_BOOT_STACK_RAM_SIZE           (64*1024)

// RAM used during boot process
#define IMAGE_BOOT_SPARE_RAM_OFFSET         (IMAGE_BOOT_STACK_RAM_OFFSET+IMAGE_BOOT_STACK_RAM_SIZE)
#define IMAGE_BOOT_SPARE_RAM_PA_START       (IMAGE_BOOT_RAMDEV_RAM_PA_START+IMAGE_BOOT_SPARE_RAM_OFFSET)
#define IMAGE_BOOT_SPARE_RAM_SIZE           (256*1024)

// EBOOT image
#define IMAGE_BOOT_BOOTIMAGE_RAM_OFFSET     (IMAGE_BOOT_SPARE_RAM_OFFSET+IMAGE_BOOT_SPARE_RAM_SIZE)
#define IMAGE_BOOT_BOOTIMAGE_RAM_PA_START   (IMAGE_BOOT_RAMDEV_RAM_PA_START+IMAGE_BOOT_BOOTIMAGE_RAM_OFFSET)
#define IMAGE_BOOT_BOOTIMAGE_RAM_SIZE       (256*1024)
#define IMAGE_BOOT_BOOTIMAGE_RAM_PA_END     (IMAGE_BOOT_BOOTIMAGE_RAM_PA_START+IMAGE_BOOT_BOOTIMAGE_RAM_SIZE-1)

// NAND cache region
#define IMAGE_BOOT_NANDCACHE_RAM_OFFSET     (IMAGE_BOOT_BOOTIMAGE_RAM_OFFSET)
#define IMAGE_BOOT_NANDCACHE_RAM_START      (IMAGE_BOOT_BOOTIMAGE_RAM_PA_START)
#define IMAGE_BOOT_NANDCACHE_RAM_SIZE       (IMAGE_BOOT_BOOTIMAGE_RAM_SIZE)

// BOOT Page table
#define IMAGE_BOOT_BOOTPT_RAM_OFFSET        (IMAGE_BOOT_NANDCACHE_RAM_OFFSET+IMAGE_BOOT_NANDCACHE_RAM_SIZE)
#define IMAGE_BOOT_BOOTPT_RAM_PA_START      (IMAGE_BOOT_RAMDEV_RAM_PA_START+IMAGE_BOOT_BOOTPT_RAM_OFFSET)
#define IMAGE_BOOT_BOOTPT_RAM_SIZE          (16*1024)
#define IMAGE_BOOT_BOOTPT_RAM_PA_END        (IMAGE_BOOT_BOOTPT_RAM_PA_START+IMAGE_BOOT_BOOTPT_RAM_SIZE-1)

// Reserved for future use
#define IMAGE_BOOT_RSVD_RAM_OFFSET          (IMAGE_BOOT_BOOTPT_RAM_OFFSET+IMAGE_BOOT_BOOTPT_RAM_SIZE)
#define IMAGE_BOOT_RSVD_RAM_PA_START        (IMAGE_BOOT_RAMDEV_RAM_PA_START+IMAGE_BOOT_RSVD_RAM_OFFSET)
#define IMAGE_BOOT_RSVD_RAM_SIZE            (364*1024)

// ENC28J60 DMA buffer
#define IMAGE_BOOT_ENC28J60_RAM_OFFSET      (IMAGE_BOOT_RSVD_RAM_OFFSET+IMAGE_BOOT_RSVD_RAM_SIZE)
#define IMAGE_BOOT_ENC28J60RAM_PA_START     (IMAGE_BOOT_RAMDEV_RAM_PA_START+IMAGE_BOOT_ENC28J60_RAM_OFFSET)
#define IMAGE_BOOT_ENC28J60_RAM_SIZE        (16*1024)

// DDK CLK
#define IMAGE_WINCE_DDKCLK_RAM_OFFSET       (IMAGE_BOOT_ENC28J60_RAM_OFFSET+IMAGE_BOOT_ENC28J60_RAM_SIZE)
#define IMAGE_WINCE_DDKCLK_RAM_PA_START     (IMAGE_BOOT_RAMDEV_RAM_PA_START+IMAGE_WINCE_DDKCLK_RAM_OFFSET)
#define IMAGE_WINCE_DDKCLK_RAM_UA_START     (OALPAtoVA(IMAGE_WINCE_DDKCLK_RAM_PA_START, FALSE))
#define IMAGE_WINCE_DDKCLK_RAM_SIZE         (16*1024)

// Share args
#define IMAGE_SHARE_ARGS_RAM_OFFSET         (IMAGE_WINCE_DDKCLK_RAM_OFFSET+IMAGE_WINCE_DDKCLK_RAM_SIZE)
#define IMAGE_SHARE_ARGS_RAM_PA_START       (IMAGE_BOOT_RAMDEV_RAM_PA_START+IMAGE_SHARE_ARGS_RAM_OFFSET)
#define IMAGE_SHARE_ARGS_UA_START           (OALPAtoVA(IMAGE_SHARE_ARGS_RAM_PA_START, FALSE))
#define IMAGE_SHARE_ARGS_RAM_SIZE           (4*1024)

// USB KITL transfer
#define IMAGE_USB_KITL_RAM_OFFSET           (IMAGE_SHARE_ARGS_RAM_OFFSET+IMAGE_SHARE_ARGS_RAM_SIZE)
//#define IMAGE_USB_KITL_RAM_PA_START        (IMAGE_BOOT_RAMDEV_RAM_PA_START+IMAGE_USB_KITL_RAM_OFFSET)
#define IMAGE_USB_KITL_RAM_PA_START         (IMAGE_WINCE_USB_IRAM_PA_START)
#define IMAGE_USB_KITL_RAM_SIZE             (32*1024)

// Display Frame buffer
#define IMAGE_WINCE_DISPLAY_RAM_OFFSET      (IMAGE_USB_KITL_RAM_OFFSET+IMAGE_USB_KITL_RAM_SIZE)
#define IMAGE_WINCE_DISPLAY_RAM_PA_START    (IMAGE_BOOT_RAMDEV_RAM_PA_START+IMAGE_WINCE_DISPLAY_RAM_OFFSET)
#define IMAGE_WINCE_DISPLAY_RAM_SIZE        (1008*1024)

#define IMAGE_SHARE_ENET_RAM_PA_START       (IMAGE_WINCE_DISPLAY_RAM_PA_START+IMAGE_WINCE_DISPLAY_RAM_SIZE)
#define IMAGE_SHARE_ENET_RAM_SIZE           (16*1024)

// Run-time image memory
#define IMAGE_BOOT_NKIMAGE_RAM_OFFSET       (0x200000)
#define IMAGE_BOOT_NKIMAGE_RAM_PA_START     (IMAGE_BOOT_RAMDEV_RAM_PA_START+IMAGE_BOOT_NKIMAGE_RAM_OFFSET)
#define IMAGE_BOOT_NKIMAGE_RAM_SIZE         (IMAGE_BOOT_RAMDEV_RAM_SIZE-IMAGE_BOOT_NKIMAGE_RAM_OFFSET)
#define IMAGE_BOOT_NKIMAGE_RAM_PA_END       (IMAGE_BOOT_NKIMAGE_RAM_PA_START+IMAGE_BOOT_NKIMAGE_RAM_SIZE-1)

//
//
//------------------------------------------------------------------------------
//// NAND flash image defines
//
//// NOTE:  EBOOT assumes these NAND regions are block-aligned
//
//// Define a special unused SOC address range that can be used to detect when an
//// image is destined for NAND flash : NandFlash image的起始物理地址：0x4020_0000
#define IMAGE_BOOT_NANDDEV_NAND_PA_START	(IMAGE_BOOT_RAMDEV_RAM_PA_START+IMAGE_BOOT_NKIMAGE_RAM_OFFSET)
//
// CS&ZHL FEB-23-2012: adjust size of OS images = (6MB + 48MB) = 54MB in EM9280
// CS&ZHL APR-01-2012: adjust size of OS images = (4MB + 48MB) = 52MB in EM9280
//
#define IMAGE_BOOT_NANDDEV_RESERVED_SIZE	(DWORD)(52 * 1024 * 1024)
#define IMAGE_BOOT_NANDDEV_NAND_PA_END		(IMAGE_BOOT_NANDDEV_NAND_PA_START+IMAGE_BOOT_NANDDEV_RESERVED_SIZE-1)

#define PLATFORM_BCB_SECTOR_SEARCH_RANGE	(256)
// Boot Control Block
#define IMAGE_BOOT_NCB_NAND_OFFSET          (CHIP_NCB_NAND_OFFSET)													// = 0x000
#define IMAGE_BOOT_NCB_NAND_BLOCKS          (CHIP_NCB_NAND_BLOCKS)													// = 4

// BAD block table
#define IMAGE_BBT_NAND_OFFSET               (IMAGE_BOOT_NCB_NAND_OFFSET + IMAGE_BOOT_NCB_NAND_BLOCKS)				// = 0x004
#define IMAGE_BBT_NAND_BLOCKS               (4)

// Bad block table used by rom
#define IMAGE_BOOT_DBBT_NAND_OFFSET         (IMAGE_BBT_NAND_OFFSET + IMAGE_BBT_NAND_BLOCKS)							// = 0x008
#define IMAGE_BOOT_DBBT_NAND_BLOCKS         (1)

// EBOOT image
#define IMAGE_BOOT_BOOTIMAGE_NAND_OFFSET    (IMAGE_BOOT_DBBT_NAND_OFFSET + IMAGE_BOOT_DBBT_NAND_BLOCKS)				// = 0x009
//#define IMAGE_BOOT_BOOTIMAGE_NAND_BLOCKS    (512)
//
// CS&ZHL JAN-5-2012: reduce EBoot space to 12 blocks, Eboot size < 3 blocks (384KB)
//
#define IMAGE_BOOT_BOOTIMAGE_NAND_BLOCKS    (6)																		// (12)

// Boot configuration
#define IMAGE_BOOT_BOOTCFG_NAND_OFFSET      (IMAGE_BOOT_BOOTIMAGE_NAND_OFFSET + IMAGE_BOOT_BOOTIMAGE_NAND_BLOCKS)	// = 0x00F // = 0x016, //= 0x20A
#define IMAGE_BOOT_BOOTCFG_NAND_BLOCKS      (1)
//#define IMAGE_BOOT_BOOTCFG_NAND_BLOCKS      (2)			// CS&ZHL JAN-5-2012: add a reserved block, (1)

// UUID
#define IMAGE_BOOT_UUID_NAND_OFFSET         (IMAGE_BOOT_BOOTCFG_NAND_OFFSET+IMAGE_BOOT_BOOTCFG_NAND_BLOCKS)			// = 0x010 // = 0x018, //= 0x20B
#define IMAGE_BOOT_UUID_NAND_BLOCKS         (1)
//#define IMAGE_BOOT_UUID_NAND_BLOCKS         (2)			// CS&ZHL JAN-5-2012: add a reserved block, (1)

//
// CS&ZHL JAN-5-2012: allocate 6 blocks for splash screen
//
#define IMAGE_BOOT_SPLASH_NAND_OFFSET       (IMAGE_BOOT_UUID_NAND_OFFSET+IMAGE_BOOT_UUID_NAND_BLOCKS)				// = 0x011 // = 0x01A
#define IMAGE_BOOT_SPLASH_NAND_BLOCKS       (4)			

//
// CS&ZHL FEB-24-2012: allocate 4 blocks for vendor ID cryption
//                     allocate 4 blocks for user ID cryption 
//                     allocate 8 blocks for future purpose
//					   allocate 48MB for NK image
//                     allocate 73MB for FAT file system
//
//
#define IMAGE_BOOT_VID_NAND_OFFSET			(IMAGE_BOOT_SPLASH_NAND_OFFSET+IMAGE_BOOT_SPLASH_NAND_BLOCKS)			// = 0x015
#define IMAGE_BOOT_VID_NAND_BLOCKS			(1)			
#define IMAGE_BOOT_UID_NAND_OFFSET			(IMAGE_BOOT_VID_NAND_OFFSET+IMAGE_BOOT_VID_NAND_BLOCKS)					// = 0x016
#define IMAGE_BOOT_UID_NAND_BLOCKS			(1)			
#define IMAGE_BOOT_RFU_NAND_OFFSET			(IMAGE_BOOT_UID_NAND_OFFSET+IMAGE_BOOT_UID_NAND_BLOCKS)					// = 0x017: Reserve for Future Use
#define IMAGE_BOOT_RFU_NAND_BLOCKS			(8)			                                                            // -> 1MB

//
// CS&ZHL MAR-07-2012: allocate 2 blocks for MBR 
//
#define IMAGE_BOOT_MBR_NAND_OFFSET			(IMAGE_BOOT_RFU_NAND_OFFSET+IMAGE_BOOT_RFU_NAND_BLOCKS)					// = 0x01F
#define IMAGE_BOOT_MBR_NAND_BLOCKS			(1)			

#define IMAGE_BOOT_NKIMAGE_NAND_OFFSET      (IMAGE_BOOT_MBR_NAND_OFFSET+IMAGE_BOOT_MBR_NAND_BLOCKS)				    // = 0x20 -> 0x400000 -> 4MB
#define IMAGE_BOOT_NKIMAGE_NAND_BLOCKS		(384)																	// 384 => 0x180
#define IMAGE_BOOT_NKIMAGE_NAND_SIZE        (48 * 1024 * 1024)														// 0x180 * 128KB => 48MB
#define IMAGE_FILE_SYSTEM_NAND_OFFSET		(IMAGE_BOOT_NKIMAGE_NAND_OFFSET + IMAGE_BOOT_NKIMAGE_NAND_BLOCKS)		// = 0x1A0 => FATFS_SIZE = 76MB(K9F1G08)

// IMG, the address is only for download memory verification
// this configuration should be aligned with g_oalAddressTable
// or OALVATOPA will fail to get the virtual address from physical address

// first 1M is reserved for RAM image
#define IMAGE_BOOT_BOOTIMAGE_NAND_PA_START  (IMAGE_BOOT_NANDDEV_NAND_PA_START + 1 * 1024 * 1024)
#define IMAGE_BOOT_BOOTIMAGE_NAND_PA_END    (IMAGE_BOOT_BOOTIMAGE_NAND_PA_START + 63 * 1024 * 1024 -1U)
//
// CS&ZHL DEC-8-2011: comments -> IMAGE_BOOT_NANDDEV_NAND_PA_START = 0x40200000
//                             -> IMAGE_BOOT_BOOTIMAGE_NAND_PA_START = 0x40200000 + 0x100000 = 0x40300000
//                             -> IMAGE_BOOT_NKIMAGE_NAND_PA_START = 0x40300000 + 0x3F00000 = 0x44200000
// MAR-29-2012: 此项定义为FLASH_ADDR（专门针对已Nand类型Flash地址：-->0x84200000）
#define IMAGE_BOOT_NKIMAGE_NAND_PA_START    (IMAGE_BOOT_BOOTIMAGE_NAND_PA_END + 1)
#define IMAGE_BOOT_NKIMAGE_NAND_PA_END      (IMAGE_BOOT_NKIMAGE_NAND_PA_START + 64 * 1024 * 1024 - 1U)

// 
// CS&ZHL MAR-08-2012: 增加对于FLASH_ADDR的CA、UA地址的定义。-->0x84200000
//
#define IMAGE_BOOT_NKIMAGE_NAND_UA_START	((DWORD)OALPAtoUA(IMAGE_BOOT_NKIMAGE_NAND_PA_START))		
#define IMAGE_BOOT_NKIMAGE_NAND_CA_START	((DWORD)OALPAtoCA(IMAGE_BOOT_NKIMAGE_NAND_PA_START))	

// NOR Flash BOOT reserved
#define IMAGE_BOOT_BOOTIMAGE_NOR_PA_START   (0x60000000)
#define IMAGE_BOOT_BOOTIMAGE_NOR_SIZE       (1 * 1024 * 1024)
#define IMAGE_BOOT_BOOTIMAGE_NOR_PA_END     (IMAGE_BOOT_BOOTIMAGE_NOR_PA_START + IMAGE_BOOT_BOOTIMAGE_NOR_SIZE - 1U)
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// SDHC flash image defines

// Define a special unused SOC address range that can be used to detect when an
// image is destined for SD/MMC Memory
#define IMAGE_BOOT_BOOTIMAGE_SD_PA_START    (0x80000000)
#define IMAGE_BOOT_BOOTIMAGE_SD_SIZE        (64*1024*1024)
#define IMAGE_BOOT_BOOTIMAGE_SD_PA_END      (IMAGE_BOOT_BOOTIMAGE_SD_PA_START+IMAGE_BOOT_BOOTIMAGE_SD_SIZE-1)

#define IMAGE_BOOT_NKIMAGE_SD_OFFSET		(125*1024*1024)			// -> 0x7D00000
// CS&ZHL DEC-8-2011: comment -> IMAGE_BOOT_NKIMAGE_SD_PA_START = 0x80000000 + 0x00200000 + 0x7D00000 = 0x87F00000
#define IMAGE_BOOT_NKIMAGE_SD_PA_START		(IMAGE_BOOT_RAMDEV_RAM_PA_START+IMAGE_BOOT_NKIMAGE_RAM_OFFSET+IMAGE_BOOT_NKIMAGE_SD_OFFSET)
#define IMAGE_BOOT_NKIMAGE_SD_SIZE			(32*1024*1024)
#define IMAGE_BOOT_NKIMAGE_SD_PA_END        (IMAGE_BOOT_NKIMAGE_SD_PA_START+IMAGE_BOOT_NKIMAGE_SD_SIZE-1)

#if __cplusplus
}
#endif

#endif
