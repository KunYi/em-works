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
#ifndef __NANDBOOT_H__
#define __NANDBOOT_H__

#pragma warning(push)
#pragma warning(disable: 4115 4127 4201 4214)
#include <fmd.h>
#include <oal.h>
#pragma warning(pop)

#define NANDFC_BOOT_SIZE								(4096*8)
#define IMAGE_LENGTH_MAGIC_STRING				"06732754"
#define IMAGE_LENGTH_MAGIC_STRING_SIZE		8
#define IMAGE_LENGTH_SIZE								4


typedef struct _NAND_IMAGE_CFG {
    // XLDR
    DWORD dwXldrOffset;
    DWORD dwXldrSize;

    // BOOT
    DWORD dwBootOffset;
    DWORD dwBootSize;

    // IPL
    DWORD dwIplOffset;
    DWORD dwIplSize;
    
    // NK
    DWORD dwNkOffset;
    DWORD dwNkSize;

    // DIO
    DWORD dwDioOffset;
    DWORD dwDioSize;
    
    // CFG
    DWORD dwCfgOffset;
    DWORD dwCfgSize;

    DWORD dwIplRamStart;    // IMAGE_BOOT_IPLIMAGE_RAM_START
    DWORD dwNkRamStart;     // IMAGE_BOOT_NKIMAGE_RAM_PA_START
    DWORD dwNandSize;       // IMAGE_BOOT_NANDDEV_RESERVED_SIZE
    
	// CS&ZHL MAY-21-2011: for BinFS
	DWORD dwNandDevPhyAddr;		// = IMAGE_BOOT_NANDDEV_NAND_PA_START;

	// MBR
	DWORD dwMBROffset;					// = IMAGE_BOOT_MBR_NAND_OFFSET;
	DWORD dwMBRSize;						// = IMAGE_BOOT_MBR_NAND_SIZE;

	// Bin
	DWORD dwBinOffset;					// = 0;
	DWORD dwBinSize;						// = 0;

}NAND_IMAGE_CFG, *PNAND_IMAGE_CFG;


typedef struct _NAND_SECTOR_BUFFER
{
    BYTE* pSectorBuf;                   // Secotr Buffer
    DWORD dwSectorBufSize;              // Size of the Secotr Buffer
}SECTOR_BUFFER, *PSECTOR_BUFFER;

//
// CS&ZHL MAY-19-2011: add data types for BinFS
//
typedef struct _CHSAddr {
    WORD cylinder;
    WORD head;
    WORD sector;
} CHSAddr, *PCHSAddr;

typedef DWORD LBAAddr, *PLBAAddr;


//-----------------------------------------------------------------------------
// External Functions
extern VOID OEMWriteDebugByte(UINT8 ch);
extern LPBYTE OEMMapMemAddr (DWORD dwImageStart, DWORD dwAddr);
extern void BSP_GetNANDImageCfg(PNAND_IMAGE_CFG pNANDImageCfg);
extern void BSP_GetNANDBufferPointer(PSECTOR_BUFFER pSectorBuffer);
#endif      // __NANDBOOT_H__


