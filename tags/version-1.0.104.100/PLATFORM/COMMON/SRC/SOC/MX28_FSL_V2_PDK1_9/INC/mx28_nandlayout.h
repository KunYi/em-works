//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Header:  MX28_nandlayout.h
//
//  Provides definitions for NAND layout on Freescale MX28 SoC.
//
//------------------------------------------------------------------------------
#ifndef __MX28_NANDLAYOUT_H
#define __MX28_NANDLAYOUT_H


#if __cplusplus
extern "C" {
#endif

//the layout is for 128 pages flash
//for 64 pages SLC flash, you need to enlarge the value by two
#define CHIP_BCB_SECTOR_STRIDE		(64)
#define CHIP_BCB_BLOCK_LIMIT		(2)

#define CHIP_NCB_NAND_OFFSET        (0)
#define CHIP_NCB_NAND_BLOCKS        (2 * CHIP_BCB_BLOCK_LIMIT)

#define CHIP_NCB_SEARCH_RANGE       (CHIP_NCB_NAND_OFFSET + CHIP_NCB_NAND_BLOCKS)		// = 4

#define CHIP_BAD_BLOCK_RANGE        (425)
#define CHIP_BBT_SECTOR_OFFSET      (4)

#ifdef __cplusplus
}
#endif

#endif // __MX28_NANDLAYOUT_H
