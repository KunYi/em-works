//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header:  MX51_ata.h
//
//  Provides definitions for ATA module based on Freescale MX51 SoC.
//
//------------------------------------------------------------------------------

#ifndef __MX51_ATA_H__
#define __MX51_ATA_H__

//-----------------------------------------------------------------------------
// Defines
#define ATA_SECTOR_SIZE         512
#define MAX_SECT_PER_SDMA       127 // should not exceed disk command and SDMA limit
#define ATA_MAX_SDMATRANSFER    (MAX_SECT_PER_SDMA * ATA_SECTOR_SIZE)
#define ATA_MAX_SDMADESC_COUNT  ((ATA_MAX_SDMATRANSFER / 512)*2)

#define CLOCK_PERIOD            15 // 66.5MHZ 15ns 15000ps
#define ATA_REG_PATH            TEXT("Drivers\\BuiltIn\\ATA")
#define REG_DMA_MODE            TEXT("DMAMode")

#define CACHE_LINE_SIZE_MASK    (32 - 1)    // L1 & L2 cache lines are 32 bytes
#define ATA_DMA_WATERMARK       32  // ATA DMA watermark in bytes

#endif // __MX51_ATA_H__
