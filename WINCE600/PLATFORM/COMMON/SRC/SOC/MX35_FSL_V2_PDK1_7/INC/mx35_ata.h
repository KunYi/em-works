//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header:  mx35_ata.h
//
//  Provides definitions for ATA module based on Freescale MX35 SoC.
//
//------------------------------------------------------------------------------

#ifndef __MX35_ATA_H__
#define __MX35_ATA_H__

//-----------------------------------------------------------------------------
// Defines
#define ATA_SECTOR_SIZE         512
#define MAX_SECT_PER_SDMA       127 // should not exceed disk command and SDMA limit
#define ATA_MAX_SDMATRANSFER    (MAX_SECT_PER_SDMA * ATA_SECTOR_SIZE)
#define ATA_MAX_SDMADESC_COUNT  ((ATA_MAX_SDMATRANSFER / 512)*2)

#define MAX_SECT_PER_ADMA       120 // should not exceed disk command and ADMA limit
#define ATA_MAX_ADMATRANSFER    (MAX_SECT_PER_ADMA * ATA_SECTOR_SIZE)
#define ATA_MAX_ADMADESC_COUNT  ((ATA_MAX_ADMATRANSFER / 512)*2)

#define CLOCK_PERIOD            7 // 133MHZ 7ns 7000ps
#define ATA_REG_PATH            TEXT("Drivers\\BuiltIn\\ATA")
#define REG_DMA_MODE            TEXT("DMAMode")

#define CACHE_LINE_SIZE_MASK    (32 - 1)    // L1 & L2 cache lines are 32 bytes
#define ATA_DMA_WATERMARK       32  // ATA DMA watermark in bytes

// Bit field positions (left shift)
#define ADMA_ACT_NOP                0x00
#define ADMA_ACT_SET                0x10
#define ADMA_ACT_TRAN               0x20
#define ADMA_ACT_LINK               0x30

#define ADMA_MODE_VALID_LSH         0
#define ADMA_MODE_END_LSH           1
#define ADMA_MODE_INT_LSH           2

typedef enum 
{
    DDK_ADMA_FLAGS_VALID = (1 << ADMA_MODE_VALID_LSH),
    DDK_ADMA_FLAGS_END = (1 << ADMA_MODE_END_LSH),
    DDK_ADMA_FLAGS_INT = (1 << ADMA_MODE_INT_LSH)
} DDK_ADMA_FLAGS;

#endif // __MX35_ATA_H__
