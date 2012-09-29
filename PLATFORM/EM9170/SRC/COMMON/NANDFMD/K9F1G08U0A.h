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
//  CS&ZHL APR-23-2011: supporting 256MB Nandflash
//  File:  K9F2G08U0A.h
//
//  Contains definitions for FMD impletation of the Samsung K9F2G08U0A NAND
//  flash memory device.
//
//------------------------------------------------------------------------------
#ifndef __K9F2G08U0A_H__
#define __K9F2G08U0A_H__


// NAND Flash Chip CMD
#define CMD_READID              (0x90)      // Read ID
#define CMD_READ                 (0x00)      // Read data 1st cycle
#define CMD_READ2               (0x30)      // Read data 2nd cycle
#define CMD_RESET                (0xFF)      // Reset
#define CMD_ERASE                (0x60)      // Erase setup
#define CMD_ERASE2              (0xD0)      // Erase 
#define CMD_WRITE                (0x80)      // Sequential data input
#define CMD_WRITE2              (0x10)      // Program
#define CMD_STATUS              (0x70)      // Read status

// NAND Flash Chip Size
#define NAND_BLOCK_CNT			1024			// 1024 blocks / device
#define NAND_PAGE_CNT				64				// Each Block has 64 Pages(Sectors)
#define NAND_PAGE_SIZE				2048			// Each Page(Sector) has 2048 Bytes
#define NAND_SPARE_SIZE			64				// Each Page(Sector) has 64 Bytes of spare area
#define NAND_BUS_WIDTH			8					// 8-bit bus

// NAND Flash Chip
#define NAND_NUM_OF_CS          (1)

// NAND Flash Chip ID
#define NAND_MAKER_CODE			0xEC				// Samsung
#define NAND_DEVICE_CODE		0xF1				// -> K9F1G08U0A
#define NAND_ID_CODE				((NAND_DEVICE_CODE << 8) | NAND_MAKER_CODE)

// NAND Flash Chip Operation Status
#define NAND_STATUS_ERROR_BIT		(0)         // Status Bit0 indicates error
#define NAND_STATUS_BUSY_BIT			(6)         // Status Bit6 indicates busy

// SWAP BBI
#define BBI_MAIN_ADDR						(464)       // Bad block info address offset -> 2048 - ((512 + 16) ¡Á 3) = 464.     
#define BBI_NUM									(1)
BYTE    BBMarkPage[1] = {63};							// the last page(sector) number of a block

#endif    // __K9F2G08U0A_H__

