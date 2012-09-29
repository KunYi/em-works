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
//  Copyright (C) 2007-2008 Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  K9LBG08U0M.h
//
//  Contains definitions for FMD impletation of the Samsung K9LBG08U0M NAND
//  flash memory device.
//
//------------------------------------------------------------------------------
#ifndef __K9LBG08U0M_H__
#define __K9LBG08U0M_H__


// NAND Flash Chip CMD
#define CMD_READID              (0x90)      // Read ID
#define CMD_READ                (0x00)      // Read data 1st cycle
#define CMD_READ2               (0x30)      // Read data 2nd cycle
#define CMD_RESET               (0xFF)      // Reset
#define CMD_ERASE               (0x60)      // Erase setup
#define CMD_ERASE2              (0xD0)      // Erase 
#define CMD_WRITE               (0x80)      // Sequential data input
#define CMD_WRITE2              (0x10)      // Program
#define CMD_STATUS              (0x70)      // Read status

// NAND Flash Chip Size
#define NAND_BLOCK_CNT          (8192)      // 8192 blocks
#define NAND_PAGE_CNT           (128)       // Each Block has 128 Pages
#define NAND_PAGE_SIZE          (4096)      // Each Page has 4096 Bytes
#define NAND_SPARE_SIZE         (128)       // Each Page has 128 Bytes spare space
#define NAND_BUS_WIDTH          (8)         // 8-bit bus

// NAND Flash Chip
#define NAND_NUM_OF_CS          (1)

// NAND Flash Chip ID
#define NAND_MAKER_CODE         (0xEC)
#define NAND_DEVICE_CODE        (0xD7)
#define NAND_ID_CODE            ((NAND_DEVICE_CODE << 8) | NAND_MAKER_CODE)

// NAND Flash Chip Operation Status
#define NAND_STATUS_ERROR_BIT   (0)         // Status Bit0 indicates error
#define NAND_STATUS_BUSY_BIT    (6)         // Status Bit6 indicates busy

// SWAP BBI
#define BBI_MAIN_ADDR           (400)       // Bad block info address offset    
#define BBI_NUM                 (1)
BYTE    BBMarkPage[1] = {127};


#endif    // __K9LBG08U0M_H__

