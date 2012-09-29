//-----------------------------------------------------------------------------
//
//  Use of this source code is subject to the terms of the Microsoft end-user
//  license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
//  If you did not accept the terms of the EULA, you are not authorized to use
//  this source code. For a copy of the EULA, please see the LICENSE.RTF on
//  your install media.
//
//-----------------------------------------------------------------------------
//
//  Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  nand.c
//
//  Contains BOOT NAND flash support functions.
//
//-----------------------------------------------------------------------------
#include "bsp.h"
#include "nandboot.h"

//-----------------------------------------------------------------------------
// External Functions


//-----------------------------------------------------------------------------
// External Variables


//-----------------------------------------------------------------------------
// Defines


//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables


//-----------------------------------------------------------------------------
// Local Variables


//-----------------------------------------------------------------------------
// Local Functions


//------------------------------------------------------------------------------
//
// Function: BSP_GetNANDImageCfg
//
//    Get the image parameters
//
// Parameters:
//        pNANDImageCfg[out] - image parameters
//
// Returns:
//        the image parameters
//
//------------------------------------------------------------------------------
void BSP_GetNANDImageCfg(PNAND_IMAGE_CFG pNANDImageCfg)
{
    pNANDImageCfg->dwXldrOffset = IMAGE_BOOT_XLDRIMAGE_NAND_OFFSET;
    pNANDImageCfg->dwXldrSize = IMAGE_BOOT_XLDRIMAGE_NAND_SIZE;

    pNANDImageCfg->dwBootOffset = IMAGE_BOOT_BOOTIMAGE_NAND_OFFSET;
    pNANDImageCfg->dwBootSize = IMAGE_BOOT_BOOTIMAGE_NAND_SIZE;

  
    pNANDImageCfg->dwNkOffset = IMAGE_BOOT_NKIMAGE_NAND_OFFSET;
    pNANDImageCfg->dwNkSize = IMAGE_BOOT_NKIMAGE_NAND_SIZE;

    pNANDImageCfg->dwDioOffset = IMAGE_BOOT_NKIMAGE_NAND_OFFSET;
    pNANDImageCfg->dwDioSize = IMAGE_BOOT_NKIMAGE_NAND_SIZE;
    
    pNANDImageCfg->dwCfgOffset = IMAGE_BOOT_BOOTCFG_NAND_OFFSET;
    pNANDImageCfg->dwCfgSize = IMAGE_BOOT_BOOTCFG_NAND_SIZE;

    pNANDImageCfg->dwNkRamStart = IMAGE_BOOT_NKIMAGE_RAM_PA_START;
    pNANDImageCfg->dwNandSize = IMAGE_BOOT_NANDDEV_RESERVED_SIZE;			//36MB -> 40MB

	// CS&ZHL MAY-21-2011: for BinFS
	pNANDImageCfg->dwNandDevPhyAddr = IMAGE_BOOT_NANDDEV_NAND_PA_START;

	// MBR
	pNANDImageCfg->dwMBROffset = IMAGE_BOOT_MBR_NAND_OFFSET;
	pNANDImageCfg->dwMBRSize = IMAGE_BOOT_MBR_NAND_SIZE;

	// Bin
	pNANDImageCfg->dwBinOffset = 0;
	pNANDImageCfg->dwBinSize = 0;
}
