;
; Copyright (c) Microsoft Corporation.  All rights reserved.
;
;
; Use of this source code is subject to the terms of the Microsoft end-user
; license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
; If you did not accept the terms of the EULA, you are not authorized to use
; this source code. For a copy of the EULA, please see the LICENSE.RTF on your
; install media.
;
;------------------------------------------------------------------------------
;
;  Copyright (C) 2004-2009, Freescale Semiconductor, Inc. All Rights Reserved.
;  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
;  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
;
;------------------------------------------------------------------------------
;
;  File:  image_cfg.inc
;
;  Defines configuration parameters used to create the NK and Bootloader
;  program images.
;
;------------------------------------------------------------------------------

    IF  !:DEF: image_cfg_inc
image_cfg_inc    EQU 1


;;------------------------------------------------------------------------------
;;  RESTRICTION
;;
;;  This file is a configuration file. It should ONLY contain simple #define 
;;  directives defining constants. This file is included by other files that 
;;  only support simple substitutions.
;;
;;------------------------------------------------------------------------------

;;------------------------------------------------------------------------------
;;  NAMING CONVENTION
;;
;;  The IMAGE_ naming convention ...
;;
;;  IMAGE_<NAME>_<SECTION>_<MEMORY_DEVICE>_[OFFSET|SIZE|START|END]
;;
;;      <NAME>          - WINCE, BOOT, SHARE
;;      <SECTION>       - section name: user defined
;;      <MEMORY_DEVICE> - the memory device the block resides on
;;      OFFSET          - number of bytes from memory device start address
;;      SIZE            - maximum size of the block
;;      START           - start address of block    (device address + offset)
;;      END             - end address of block      (start address  + size - 1)
;;
;;------------------------------------------------------------------------------

;;------------------------------------------------------------------------------

;------------------------------------------------------------------------------
; Internal RAM Mapping (128 KB)
IMAGE_WINCE_IRAM_PA_START          EQU        0x00000000
IMAGE_WINCE_IRAM_SIZE              EQU        (128 * 1024)

; 4K bytes reserved for USB Transfer
IMAGE_WINCE_USB_IRAM_OFFSET        EQU        (0)
IMAGE_WINCE_USB_IRAM_PA_START      EQU        (IMAGE_WINCE_IRAM_PA_START + IMAGE_WINCE_USB_IRAM_OFFSET)
IMAGE_WINCE_USB_IRAM_SIZE          EQU        (4 * 1024)

; 4K bytes reserved for SD/MMC Transfer
IMAGE_WINCE_SDMMC_IRAM_OFFSET      EQU        (IMAGE_WINCE_USB_IRAM_OFFSET + IMAGE_WINCE_USB_IRAM_SIZE)
IMAGE_WINCE_SDMMC_IRAM_PA_START    EQU        (IMAGE_WINCE_IRAM_PA_START + IMAGE_WINCE_SDMMC_IRAM_OFFSET)
IMAGE_WINCE_SDMMC_IRAM_SIZE        EQU        (4 * 1024)


; RAM image defines
IMAGE_BOOT_RAMDEV_RAM_PA_START      EQU     (0x40000000) 
IMAGE_BOOT_RAMDEV_RAM_UA_START      EQU     (0xA0000000)
IMAGE_BOOT_RAMDEV_RAM_CA_START      EQU     (0x80000000)
IMAGE_BOOT_RAMDEV_RAM_SIZE          EQU     (128*1024*1024);; 128 MB
IMAGE_BOOT_RAMDEV_RAM_PA_END        EQU     (IMAGE_BOOT_RAMDEV_RAM_PA_START+IMAGE_BOOT_RAMDEV_RAM_SIZE-1)

;Stack space for boot procedure
IMAGE_BOOT_STACK_RAM_OFFSET         EQU     (0)
IMAGE_BOOT_STACK_RAM_SIZE           EQU     (64*1024)
; Stack grows downward.  Start must be at top of reserved region.
IMAGE_BOOT_STACK_RAM_PA_START       EQU     (IMAGE_BOOT_RAMDEV_RAM_PA_START+IMAGE_BOOT_STACK_RAM_OFFSET+IMAGE_BOOT_STACK_RAM_SIZE)
IMAGE_BOOT_STACK_RAM_CA_START       EQU     (IMAGE_BOOT_RAMDEV_RAM_CA_START+IMAGE_BOOT_STACK_RAM_OFFSET+IMAGE_BOOT_STACK_RAM_SIZE)

; RAM used during boot process
IMAGE_BOOT_SPARE_RAM_OFFSET         EQU     (IMAGE_BOOT_STACK_RAM_OFFSET+IMAGE_BOOT_STACK_RAM_SIZE)
IMAGE_BOOT_SPARE_RAM_PA_START       EQU     (IMAGE_BOOT_RAMDEV_RAM_PA_START+IMAGE_BOOT_SPARE_RAM_OFFSET)
IMAGE_BOOT_SPARE_RAM_SIZE           EQU     (256*1024)

; EBOOT image
IMAGE_BOOT_BOOTIMAGE_RAM_OFFSET     EQU     (IMAGE_BOOT_SPARE_RAM_OFFSET+IMAGE_BOOT_SPARE_RAM_SIZE)
IMAGE_BOOT_BOOTIMAGE_RAM_PA_START   EQU     (IMAGE_BOOT_RAMDEV_RAM_PA_START+IMAGE_BOOT_BOOTIMAGE_RAM_OFFSET)
IMAGE_BOOT_BOOTIMAGE_RAM_SIZE       EQU     (256*1024)
IMAGE_BOOT_BOOTIMAGE_RAM_PA_END     EQU     (IMAGE_BOOT_BOOTIMAGE_RAM_PA_START+IMAGE_BOOT_BOOTIMAGE_RAM_SIZE-1)

; NAND cache region
IMAGE_BOOT_NANDCACHE_RAM_OFFSET     EQU     (IMAGE_BOOT_BOOTIMAGE_RAM_OFFSET)
IMAGE_BOOT_NANDCACHE_RAM_START      EQU     (IMAGE_BOOT_BOOTIMAGE_RAM_PA_START)
IMAGE_BOOT_NANDCACHE_RAM_SIZE       EQU     (IMAGE_BOOT_BOOTIMAGE_RAM_SIZE)

; BOOT Page table
IMAGE_BOOT_BOOTPT_RAM_OFFSET        EQU     (IMAGE_BOOT_NANDCACHE_RAM_OFFSET+IMAGE_BOOT_NANDCACHE_RAM_SIZE)
IMAGE_BOOT_BOOTPT_RAM_PA_START      EQU     (IMAGE_BOOT_RAMDEV_RAM_PA_START+IMAGE_BOOT_BOOTPT_RAM_OFFSET)
IMAGE_BOOT_BOOTPT_RAM_SIZE          EQU     (16*1024)
IMAGE_BOOT_BOOTPT_RAM_PA_END        EQU     (IMAGE_BOOT_BOOTPT_RAM_PA_START+IMAGE_BOOT_BOOTPT_RAM_SIZE-1)

; Reserved for future use
IMAGE_BOOT_RSVD_RAM_OFFSET          EQU     (IMAGE_BOOT_BOOTPT_RAM_OFFSET+IMAGE_BOOT_BOOTPT_RAM_SIZE)
IMAGE_BOOT_RSVD_RAM_PA_START        EQU     (IMAGE_BOOT_RAMDEV_RAM_PA_START+IMAGE_BOOT_RSVD_RAM_OFFSET)
IMAGE_BOOT_RSVD_RAM_SIZE            EQU     (364*1024)

; ENC28J60 Ethernet DMA buffers
IMAGE_BOOT_ENC28J60_RAM_OFFSET      EQU     (IMAGE_BOOT_RSVD_RAM_OFFSET+IMAGE_BOOT_RSVD_RAM_SIZE)
IMAGE_BOOT_ENC28J60_RAM_PA_START    EQU     (IMAGE_BOOT_RAMDEV_RAM_PA_START+IMAGE_BOOT_ENC28J60_RAM_OFFSET)
IMAGE_BOOT_ENC28J60_RAM_SIZE        EQU     (16*1024)

; DDK CLK
IMAGE_WINCE_DDKCLK_RAM_OFFSET       EQU     (IMAGE_BOOT_ENC28J60_RAM_OFFSET+IMAGE_BOOT_ENC28J60_RAM_SIZE)
IMAGE_WINCE_DDKCLK_RAM_PA_START     EQU     (IMAGE_BOOT_RAMDEV_RAM_PA_START+IMAGE_WINCE_DDKCLK_RAM_OFFSET)
IMAGE_WINCE_DDKCLK_RAM_SIZE         EQU     (16*1024)

; Share args
IMAGE_SHARE_ARGS_RAM_OFFSET         EQU     (IMAGE_WINCE_DDKCLK_RAM_OFFSET+IMAGE_WINCE_DDKCLK_RAM_SIZE)
IMAGE_SHARE_ARGS_RAM_PA_START       EQU     (IMAGE_BOOT_RAMDEV_RAM_PA_START+IMAGE_SHARE_ARGS_RAM_OFFSET)
IMAGE_SHARE_ARGS_RAM_UA_START       EQU     (IMAGE_BOOT_RAMDEV_RAM_UA_START+IMAGE_SHARE_ARGS_RAM_OFFSET)
IMAGE_SHARE_ARGS_RAM_SIZE           EQU     (4*1024)

; USB KITL transfer
IMAGE_KITL_USB_RAM_OFFSET           EQU     (IMAGE_SHARE_ARGS_RAM_OFFSET+IMAGE_SHARE_ARGS_RAM_SIZE)
; IMAGE_KITL_USB_RAM_PA_START       EQU     (IMAGE_BOOT_RAMDEV_RAM_PA_START+IMAGE_KITL_USB_RAM_OFFSET)
IMAGE_KITL_USB_RAM_PA_START         EQU     (IMAGE_WINCE_USB_IRAM_PA_START)
IMAGE_KITL_USB_RAM_SIZE             EQU     (32*1024)

;Display Frame buffer 
IMAGE_WINCE_DISPLAY_RAM_OFFSET      EQU     (IMAGE_KITL_USB_RAM_OFFSET+IMAGE_KITL_USB_RAM_SIZE)
IMAGE_WINCE_DISPLAY_RAM_SIZE        EQU     (1024*1024)

;Run-time image memory
IMAGE_BOOT_NKIMAGE_RAM_OFFSET       EQU     (0x200000)
IMAGE_BOOT_NKIMAGE_RAM_PA_START     EQU     (IMAGE_BOOT_RAMDEV_RAM_PA_START+IMAGE_BOOT_NKIMAGE_RAM_OFFSET)
IMAGE_BOOT_NKIMAGE_RAM_SIZE         EQU     (IMAGE_BOOT_RAMDEV_RAM_SIZE-IMAGE_BOOT_NKIMAGE_RAM_OFFSET)
IMAGE_BOOT_NKIMAGE_RAM_PA_END       EQU     (IMAGE_BOOT_NKIMAGE_RAM_PA_START+IMAGE_BOOT_NKIMAGE_RAM_SIZE-1)

;;------------------------------------------------------------------------------

    ENDIF
    END
