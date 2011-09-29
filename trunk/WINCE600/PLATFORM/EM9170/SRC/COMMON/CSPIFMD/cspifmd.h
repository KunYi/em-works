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
// Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
// FILE:    cspifmd.h
//
//  This file contains structure definitions required by CSPI FMD
//
//------------------------------------------------------------------------------

#ifndef __SPIFMD_H__
#define __SPIFMD_H__

#include "bsp.h"

#pragma warning(push)
#pragma warning(disable: 4115 4127 4201 4214)
#include "fmd.h"
#pragma warning(pop)

// DEFINES

#define SPIFMD                                    3         // Flash Type
#define SPIFMD_SECTOR_SIZE                        4096      // Sector Size
#define SPIFMD_BLOCK_CNT                          32        // Max Number of Blocks
#define SPIFMD_SECTOR_CNT                         16        // Number of sectors to constitute a Block
#define SPIFMD_PAGE_SIZE                          256       // Page size in bytes

#define WREN_CMD            0x06
#define WRDI_CMD            0x04
#define RDID_CMD            0x9F
#define RDSR_CMD            0x05
#define WRSR_CMD            0x01
#define BE_CMD              0xD8
#define PP_CMD              0x02
#define READ_CMD            0x03

#define WRSR_CYCLE_TIME             100*1000            //Write Status Register Cycle Time (in microseconds)
#define PP_CYCLE_TIME               5*1000              //Page Program Cycle Time
#define BE_CYCLE_TIME               2*1000*1000         //Block Erase Cycle Time

//Manufacturer and Device ID
#define MANUFACTURER_ID             0xC2

//Status Register
#define WEL                         (1 << 1)            //Write Enable Bit
#define WIP                         (1)                 //Write In Progress Bit

#define WIP_TIMEOUT                 1000                //number of attempts to read WIP bit
#define ONE_MS                      1000                //1 MS in units of microseconds

// PROTOTYPES

BOOL SPIFMD_Init(void);
BOOL SPIFMD_Deinit (void);

BOOL SPIFMD_GetInfo(PFlashInfo pFlashInfo);
BOOL SPIFMD_EraseBlock(BLOCK_ID blockID);
DWORD SPIFMD_GetBlockStatus(BLOCK_ID blockID);
BOOL SPIFMD_SetBlockStatus(BLOCK_ID blockID, DWORD dwStatus);

BOOL SPIFMD_ReadSector(SECTOR_ADDR startSectorAddr, LPBYTE pSectorBuff,
                      PSectorInfo pSectorInfoBuff, DWORD dwNumSectors);
BOOL SPIFMD_WriteSector(SECTOR_ADDR startSectorAddr, LPBYTE pSectorBuff,
                       PSectorInfo pSectorInfoBuff, DWORD dwNumSectors);

#endif      // __SPIFMD_H__

