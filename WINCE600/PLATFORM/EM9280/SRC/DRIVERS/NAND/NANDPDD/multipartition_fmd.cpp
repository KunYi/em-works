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
//  Copyright (C) 2005-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//#include <windows.h>
//#include <ceddk.h>
//#include <nkintr.h>
//#include "csp.h"
#include "bsp.h"
#include "FmdWrapperPdd.h"
#include "common_nandfmd.h"


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

//-----------------------------------------------------------------------------
//
//  Function: OEMFMD_Init
//
//  This function initializes the flash memory of a device.
//
//  Parameters:
//      lpActiveReg
//          [in] Pointer to the active registry string used to find device 
//          information from the registry. Set to NULL if not needed. 
//
//      pRegIn
//          [in] Pointer to a PCI_REG_INFO structure. Used to find flash 
//          hardware on PCI hardware. Set to NULL if not needed.
//
//      pRegOut
//          [in/out] Pointer to a PCI_REG_INFO structure. Used to return 
//          flash information. Set to NULL if not needed.
//
//  Returns:
//      A handle that can be used in a call to FMD_Deinit. It is the 
//      responsibility of the specific flash media driver (FMD) implementation 
//      to determine what this value represents. A value of zero (0) 
//      represents failure.
//
//-----------------------------------------------------------------------------
PVOID OEMFMD_Init(LPCTSTR lpActiveReg, PPCI_REG_INFO pRegIn, PPCI_REG_INFO pRegOut)
{
	FmdAccessInfo fmdInfo;

	UNREFERENCED_PARAMETER(lpActiveReg);
	UNREFERENCED_PARAMETER(pRegIn);
	UNREFERENCED_PARAMETER(pRegOut);

	//RETAILMSG(1, (TEXT("->OEMFMD_Init\r\n")));

	fmdInfo.dwAccessCode = FMD_ACCESS_CODE_HWINIT;
	
	if(KernelIoControl(IOCTL_HAL_NANDFMD_ACCESS, (PVOID)(&fmdInfo), sizeof(FmdAccessInfo), NULL, 0, NULL) == FALSE)
	{
		RETAILMSG(1, (TEXT("OEMFMD_Init::IOCTL_HAL_NANDFMD_ACCESS = 0x%08x fail!\r\n"), IOCTL_HAL_NANDFMD_ACCESS));
		return NULL;
	}

	return (PVOID)0x12345678;
}


//-----------------------------------------------------------------------------
//
//  Function: FMD_Deinit
//
//  This function de-initializes the flash chip.
//
//  Parameters:
//      hFMD 
//          [in] The handle returned from FMD_Init. 
//
//  Returns:  
//      Returns TRUE on success. Returns FALSE on failure.
//
//-----------------------------------------------------------------------------
BOOL OEMFMD_Deinit(PVOID pContext)
{
	UNREFERENCED_PARAMETER(pContext);

	return(TRUE);
}


//-----------------------------------------------------------------------------
//
//  Function: FMD_ReadSector
//
//  This function reads the requested sector data and metadata from the 
//  flash media.
//
//  Parameters:
//      startSectorAddr 
//          [in] The starting physical sector address to read.
//
//      pSectorBuff 
//          [out] Pointer to the buffer that contains the sector data read 
//          from flash memory. Set to NULL if this data is not needed.
//
//      pSectorInfoBuff 
//          [out] Buffer for an array of sector information structures. There 
//          is one sector information entry for every sector that is to be read. 
//          Set to NULL if this data is not needed. 
//
//      dwNumSectors 
//          [in] Number of sectors to read.
//
//  Returns:  
//      Returns TRUE on success. Returns FALSE on failure.
//
//-----------------------------------------------------------------------------
BOOL OEMFMD_ReadSector(PVOID pContext, SECTOR_ADDR startSectorAddr, LPBYTE pSectorBuff, PSectorInfo pSectorInfoBuff, DWORD dwNumSectors, BOOL bWithStartBlock)
{
	FmdWrapperPdd * pFlashWrapper = (FmdWrapperPdd *)pContext;
	FmdAccessInfo fmdInfo;
	SECTOR_ADDR SectorAddr = startSectorAddr;
	BOOL bRc = FALSE;

//	RETAILMSG(1, (TEXT("OEMFMD_ReadSector() start.\r\n")));
//	RETAILMSG(1, (TEXT("SectorAddr = %d.\r\n"), SectorAddr));

	if(!pFlashWrapper)
	{
		return FALSE;
	}

	if(!pFlashWrapper->m_bInitialized)
	{
		return FALSE;
	}

	//
	// CS&ZHL MAY-14-2011: MUST get FlashInfo before reading sector
	//
	if(!pFlashWrapper->m_dwFlashInfoSize)
	{
		return FALSE;
	}

	if(bWithStartBlock)
	{
		//SectorAddr += BLOCK_TO_SECTOR(pFlashWrapper->m_dwStartBlock);
		SectorAddr += (pFlashWrapper->m_dwStartBlock * pFlashWrapper->m_FlashInfo.wSectorsPerBlock);
	}

	fmdInfo.dwAccessCode = FMD_ACCESS_CODE_READSECTOR;
	fmdInfo.dwStartSector = SectorAddr;
	fmdInfo.dwSectorNum = dwNumSectors;
	fmdInfo.pMData = (VOID *)pSectorBuff;
	fmdInfo.pSData = (VOID *)pSectorInfoBuff;

	bRc = KernelIoControl(IOCTL_HAL_NANDFMD_ACCESS, (PVOID)(&fmdInfo), sizeof(FmdAccessInfo), NULL, 0, NULL);

//	RETAILMSG(1, (TEXT("OEMFMD_ReadSector() end.\r\n")));

	return bRc;
}


//-----------------------------------------------------------------------------
//
//  Function: FMD_WriteSector
//
//  This function writes the requested sector data and metadata to the 
//  flash media.
//
//  Parameters:
//      startSectorAddr 
//          [in] The starting physical sector address to write to.
//
//      pSectorBuff 
//          [in] Pointer to the buffer that contains the sector data to write. 
//          Set to NULL if no data is to be written.
//
//      pSectorInfoBuff 
//          [in] Buffer for an array of sector information structures. There 
//          must be one sector information entry for each sector that is to be 
//          written. Set to NULL if this data is not written.
//
//      dwNumSectors 
//          [in] Number of sectors to write.
//
//  Returns:  
//      Returns TRUE on success. Returns FALSE on failure.
//
//-----------------------------------------------------------------------------
BOOL OEMFMD_WriteSector(PVOID pContext, SECTOR_ADDR startSectorAddr, LPBYTE pSectorBuff, PSectorInfo pSectorInfoBuff, DWORD dwNumSectors, BOOL bWithStartBlock)
{
	FmdWrapperPdd * pFlashWrapper = (FmdWrapperPdd *)pContext;
	FmdAccessInfo fmdInfo;
	SECTOR_ADDR SectorAddr = startSectorAddr;
	BOOL bRc = FALSE;

//	RETAILMSG(1, (TEXT("OEMFMD_WriteSector() start.\r\n")));

	if(!pFlashWrapper)
	{
		return FALSE;
	}

	if(!pFlashWrapper->m_bInitialized)
	{
		return FALSE;
	}

	//
	// CS&ZHL MAY-14-2011: MUST get FlashInfo before writing sector
	//
	if(!pFlashWrapper->m_dwFlashInfoSize)
	{
		return FALSE;
	}

	if(bWithStartBlock)
	{
		//SectorAddr += BLOCK_TO_SECTOR(pFlashWrapper->m_dwStartBlock);
		SectorAddr += (pFlashWrapper->m_dwStartBlock * pFlashWrapper->m_FlashInfo.wSectorsPerBlock);
	}

//	RETAILMSG(TRUE, (_T("Write Sector Start = 0x%x, number = 0x%x\r\n"), startSectorAddr, dwNumSectors));

	fmdInfo.dwAccessCode = FMD_ACCESS_CODE_WRITESECTOR;
	fmdInfo.dwStartSector = SectorAddr;
	fmdInfo.dwSectorNum = dwNumSectors;
	fmdInfo.pMData = (VOID *)pSectorBuff;
	fmdInfo.pSData = (VOID *)pSectorInfoBuff;

	bRc = KernelIoControl(IOCTL_HAL_NANDFMD_ACCESS, (PVOID)(&fmdInfo), sizeof(FmdAccessInfo), NULL, 0, NULL);

//	RETAILMSG(1, (TEXT("OEMFMD_WriteSector() end.\r\n")));

	return bRc;
}


//-----------------------------------------------------------------------------
//
//  Function: FMD_EraseBlock
//
//  This function erases the specified flash block.
//
//  Parameters:
//      blockID 
//          [in] The block number to erase.
//
//  Returns:  
//      Returns TRUE on success. Returns FALSE on failure.
//
//-----------------------------------------------------------------------------
BOOL OEMFMD_EraseBlock(PVOID pContext, BLOCK_ID blockID, BOOL bWithStartBlock)
{
	FmdWrapperPdd * pFlashWrapper = (FmdWrapperPdd *)pContext;
	FmdAccessInfo fmdInfo;
	//SECTOR_ADDR SectorAddr = BLOCK_TO_SECTOR(blockID);
	SECTOR_ADDR SectorAddr;
	BOOL bRc = FALSE;

//	RETAILMSG(1, (TEXT("OEMFMD_EraseBlock() start.\r\n")));

	if(!pFlashWrapper)
	{
		return FALSE;
	}

	if(!pFlashWrapper->m_bInitialized)
	{
		return FALSE;
	}

	//
	// CS&ZHL MAY-14-2011: MUST get FlashInfo before erasing block
	//
	if(!pFlashWrapper->m_dwFlashInfoSize)
	{
		return FALSE;
	}

	SectorAddr = blockID * pFlashWrapper->m_FlashInfo.wSectorsPerBlock;
	if(bWithStartBlock)
	{
		//SectorAddr += BLOCK_TO_SECTOR(pFlashWrapper->m_dwStartBlock);
		SectorAddr += (pFlashWrapper->m_dwStartBlock * pFlashWrapper->m_FlashInfo.wSectorsPerBlock);
	}

	fmdInfo.dwAccessCode = FMD_ACCESS_CODE_ERASEBLOCK;
	fmdInfo.dwStartSector = SectorAddr;

	bRc = KernelIoControl(IOCTL_HAL_NANDFMD_ACCESS, (PVOID)(&fmdInfo), sizeof(FmdAccessInfo), NULL, 0, NULL);

//	RETAILMSG(1, (TEXT("OEMFMD_EraseBlock() end.\r\n")));

	return bRc;
}


//-----------------------------------------------------------------------------
//
//  Function: FMD_PowerUp
//
//  This function restores power to the flash memory device, if applicable.
//
//  Parameters:
//      None
//
//  Returns:  
//      None
//
//-----------------------------------------------------------------------------
VOID OEMFMD_PowerUp(PVOID pContext)
{
	UNREFERENCED_PARAMETER(pContext);
}


//-----------------------------------------------------------------------------
//
//  Function: FMD_PowerDown
//
//  This function suspends power to the flash memory device, if applicable.
//
//  Parameters:
//      None
//
//  Returns:  
//      None
//
//-----------------------------------------------------------------------------
VOID OEMFMD_PowerDown(PVOID pContext)
{
	UNREFERENCED_PARAMETER(pContext);
}


//-----------------------------------------------------------------------------
//
//  Function: FMD_OEMIoControl
//
//  This function implements user-defined commands for the flash memory device. 
//
//  Parameters:
//      dwIoControlCode 
//          [in] The control code specifying the command to execute.
//
//      pInBuf
//          [in] Long pointer to a buffer that contains the data required to 
//          perform the operation. Set to NULL if the dwIoControlCode parameter 
//          specifies an operation that does not require input data.
//                        
//      nInBufSize
//          [in] Size, in bytes, of the buffer pointed to by pInBuf. 
//
//      pOutBuf
//          [out] Long pointer to a buffer that receives the output data for 
//          the operation. Set to NULL if the dwIoControlCode parameter 
//          specifies an operation that does not produce output data.
//
//      nOutBufSize
//          [in] Size, in bytes, of the buffer pointed to by pOutBuf.
//
//      pBytesReturned
//          [out] Long pointer to a variable that receives the size, in bytes, 
//          of the data stored into the buffer pointed to by pOutBuf. Even 
//          when an operation produces no output data and pOutBuf is set to 
//          NULL, the DeviceIoControl function uses the variable pointed to 
//          by pBytesReturned. After such an operation, the value of the 
//          variable has no meaning. 
//
//  Returns:  
//      Returns TRUE on success. Returns FALSE on failure.
//
//-----------------------------------------------------------------------------
BOOL OEMFMD_OemIoControl(PVOID pContext, DWORD dwIoControlCode, PBYTE pInBuf, DWORD nInBufSize, PBYTE pOutBuf, DWORD nOutBufSize, PDWORD pBytesReturned)
{
	UNREFERENCED_PARAMETER(pContext);
	UNREFERENCED_PARAMETER(dwIoControlCode);
	UNREFERENCED_PARAMETER(pInBuf);
	UNREFERENCED_PARAMETER(nInBufSize);
	UNREFERENCED_PARAMETER(pOutBuf);
	UNREFERENCED_PARAMETER(nOutBufSize);
	UNREFERENCED_PARAMETER(pBytesReturned);

//	RETAILMSG(1, (TEXT("OEMFMD_OemIoControl() start.\r\n")));
//	RETAILMSG(1, (TEXT("dwIoControlCode =  0x%x.\r\n"), dwIoControlCode));

//	RETAILMSG(1, (TEXT("OEMFMD_OemIoControl() end.\r\n")));

	return FALSE;
}


//-----------------------------------------------------------------------------
//
//  Function: FMD_GetInfo
//
//  This function determines the size characteristics for the flash memory 
//  device.
//
//  Parameters:
//      pFlashInfo 
//          [out] A pointer to a structure that contains the size 
//          characteristics for the flash memory device.
//
//  Returns:  
//      Returns TRUE on success. Returns FALSE on failure.
//
//-----------------------------------------------------------------------------
BOOL OEMFMD_GetInfo(PVOID pContext, PFlashInfo pFlashInfo)
{
	FmdWrapperPdd * pFlashWrapper = (FmdWrapperPdd *)pContext;
	DWORD dwTargetBlock, dwSkipBlocks;
	DWORD dwStatus;

	//	RETAILMSG(1, (TEXT("OEMFMD_GetInfo() start.\r\n")));

	if(!pFlashWrapper || !pFlashInfo)
	{
		return FALSE;
	}

	if(!pFlashWrapper->m_bInitialized)
	{
		return FALSE;
	}

	//
	// CS&ZHL MAY-14-2011: MUST get FlashInfo before doing anything else
	//
	if(!pFlashWrapper->m_dwFlashInfoSize)
	{
		FmdAccessInfo fmdInfo;
		DWORD				dwSize = sizeof(FlashInfo);
		BOOL				bRc;

		fmdInfo.dwAccessCode = FMD_ACCESS_CODE_GETINFO;
		fmdInfo.pMData = (VOID *)&pFlashWrapper->m_FlashInfo;
		fmdInfo.pSData = (VOID *)&dwSize;

		bRc = KernelIoControl(IOCTL_HAL_NANDFMD_ACCESS, (PVOID)(&fmdInfo), sizeof(FmdAccessInfo), NULL, 0, NULL);
		if(!bRc)
		{
			RETAILMSG(1, (TEXT("OEMFMD_GetInfo: Get NandFlash Info Failed\r\n")));
			return FALSE;
		}
		pFlashWrapper->m_dwFlashInfoSize = sizeof(FlashInfo);	//set the flag as a valid FlashInfo gotten
	}

	dwSkipBlocks = IMAGE_BOOT_NANDDEV_RESERVED_SIZE / pFlashWrapper->m_FlashInfo.dwBytesPerBlock;
	if(pFlashWrapper->m_dwRegionNumber == 1)
	{
		// Skip BOOT Config
		//pFlashWrapper->m_dwStartBlock = NAND_BLOCK_CNT - SMALL_PARTITION_BLOCKS;
		//pFlashInfo->dwNumBlocks = SMALL_PARTITION_BLOCKS - (IMAGE_BOOT_BOOTCFG_NAND_SIZE / NAND_BLOCK_SIZE);
		pFlashWrapper->m_dwStartBlock = dwSkipBlocks;
		pFlashInfo->dwNumBlocks = SMALL_PARTITION_BLOCKS;
	}
	else if(pFlashWrapper->m_dwRegionNumber == 2)
	{
		// Skip XLDR, EBoot, IPL and NK
		//dwSkipBlocks = (IMAGE_BOOT_XLDRIMAGE_NAND_SIZE + IMAGE_BOOT_EBOOTIMAGE_NAND_SIZE + IMAGE_BOOT_IPLIMAGE_NAND_SIZE + IMAGE_BOOT_NKIMAGE_NAND_SIZE) / NAND_BLOCK_SIZE;
		//pFlashWrapper->m_dwStartBlock = dwSkipBlocks;
		//pFlashInfo->dwNumBlocks = NAND_BLOCK_CNT - SMALL_PARTITION_BLOCKS - dwSkipBlocks;
		pFlashWrapper->m_dwStartBlock = dwSkipBlocks + SMALL_PARTITION_BLOCKS;
		pFlashInfo->dwNumBlocks = pFlashWrapper->m_FlashInfo.dwNumBlocks - pFlashWrapper->m_dwStartBlock;
	}
	else
	{
		// Skip XLDR, EBoot, IPL, NK and BOOT Config
		//dwSkipBlocks = (IMAGE_BOOT_XLDRIMAGE_NAND_SIZE + IMAGE_BOOT_EBOOTIMAGE_NAND_SIZE + IMAGE_BOOT_IPLIMAGE_NAND_SIZE + IMAGE_BOOT_NKIMAGE_NAND_SIZE) / NAND_BLOCK_SIZE;
		//pFlashWrapper->m_dwStartBlock = dwSkipBlocks;
		//pFlashInfo->dwNumBlocks = NAND_BLOCK_CNT - dwSkipBlocks - (IMAGE_BOOT_BOOTCFG_NAND_SIZE / NAND_BLOCK_SIZE);
		pFlashWrapper->m_dwStartBlock = dwSkipBlocks;
		pFlashInfo->dwNumBlocks = pFlashWrapper->m_FlashInfo.dwNumBlocks - pFlashWrapper->m_dwStartBlock;
	}

	// The first block for partition can't be bad, reserved or read only.
	for(dwTargetBlock = pFlashWrapper->m_dwStartBlock; dwTargetBlock < pFlashWrapper->m_dwStartBlock + pFlashInfo->dwNumBlocks; dwTargetBlock ++)
	{
		dwStatus = OEMFMD_GetBlockStatus((PVOID)pFlashWrapper, dwTargetBlock, FALSE);
		if(dwStatus == 0)
			break;
	}
	dwSkipBlocks = dwTargetBlock - pFlashWrapper->m_dwStartBlock;
	pFlashWrapper->m_dwStartBlock += dwSkipBlocks;
	pFlashInfo->dwNumBlocks -= dwSkipBlocks;

	pFlashWrapper->m_dwBlockCounts = pFlashInfo->dwNumBlocks;

	pFlashInfo->flashType = NAND;
	pFlashInfo->wDataBytesPerSector = pFlashWrapper->m_FlashInfo.wDataBytesPerSector;
	pFlashInfo->wSectorsPerBlock = pFlashWrapper->m_FlashInfo.wSectorsPerBlock;
	pFlashInfo->dwBytesPerBlock = (pFlashInfo->wSectorsPerBlock * pFlashInfo->wDataBytesPerSector);

	//RETAILMSG(1, (TEXT("FlashInfo Region%d: Start block = %d, Total blocks = %d.\r\n"), 
	//					pFlashWrapper->m_dwRegionNumber, pFlashWrapper->m_dwStartBlock, pFlashWrapper->m_dwBlockCounts));

	//	RETAILMSG(1, (TEXT("OEMFMD_GetInfo() end.\r\n")));

	return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function: FMD_GetBlockStatus
//					CS&ZHL MAY-14-2011: re-write the routine
//
//  This function returns the status of a block.
//
//  Parameters:
//      blockID 
//          [in] The block number used to check status.
//
//  Returns:  
//      Flags to describe the status of the block.
//
//-----------------------------------------------------------------------------
DWORD OEMFMD_GetBlockStatus(PVOID pContext, BLOCK_ID blockID, BOOL bWithStartBlock)
{
	FmdWrapperPdd * pFlashWrapper = (FmdWrapperPdd *)pContext;
	FmdAccessInfo		fmdInfo;
	SECTOR_ADDR		SectorAddr;
	DWORD					dwResult = 0;
	BOOL					bRc;

	//	RETAILMSG(1, (TEXT("OEMFMD_GetBlockStatus() start.\r\n")));

	if(!pFlashWrapper)
	{
		goto cleanUp;
	}

	if(!pFlashWrapper->m_bInitialized)
	{
		goto cleanUp;
	}

	if(!pFlashWrapper->m_dwFlashInfoSize)
	{
		goto cleanUp;
	}

	SectorAddr = blockID * pFlashWrapper->m_FlashInfo.wSectorsPerBlock;
	if(bWithStartBlock)
	{
		SectorAddr += (pFlashWrapper->m_dwStartBlock * pFlashWrapper->m_FlashInfo.wSectorsPerBlock);
	}

	fmdInfo.dwAccessCode = FMD_ACCESS_CODE_GETSTATUS;
	fmdInfo.dwStartSector = SectorAddr;
	fmdInfo.pMData = (VOID *)&dwResult;

	bRc = KernelIoControl(IOCTL_HAL_NANDFMD_ACCESS, (PVOID)(&fmdInfo), sizeof(FmdAccessInfo), NULL, 0, NULL);
	if(!bRc)
	{
		dwResult = BLOCK_STATUS_BAD;
		goto cleanUp;
	}


cleanUp:

	if(BLOCK_STATUS_BAD == dwResult)
	{
		if(bWithStartBlock)
			RETAILMSG(TRUE, (_T("Block 0x%x status is 0x%x.\r\n"), blockID + pFlashWrapper->m_dwStartBlock, dwResult));
		else
			RETAILMSG(TRUE, (_T("Block 0x%x status is 0x%x.\r\n"), blockID, dwResult));
	}

	//	RETAILMSG(1, (TEXT("OEMFMD_GetBlockStatus() end.\r\n")));
	return dwResult;
}


//-----------------------------------------------------------------------------
//
//  Function: FMD_SetBlockStatus
//					CS&ZHL MAY-14-2011: re-write the routine
//
//  This function sets the status of a block.
//
//  Parameters:
//      blockID 
//          [in] The block number used to set status. 
//
//      dwStatus
//          [in] The status value to set.
//
//  Returns:  
//      Returns TRUE on success. Returns FALSE on failure.
//
//-----------------------------------------------------------------------------
BOOL OEMFMD_SetBlockStatus(PVOID pContext, BLOCK_ID blockID, DWORD dwStatus, BOOL bWithStartBlock)
{
	FmdWrapperPdd * pFlashWrapper = (FmdWrapperPdd *)pContext;
	FmdAccessInfo		fmdInfo;
	SECTOR_ADDR		SectorAddr;
	BOOL					bRc;

	//	RETAILMSG(1, (TEXT("OEMFMD_SetBlockStatus() start.\r\n")));

	if(!pFlashWrapper)
	{
		return FALSE;
	}

	if(!pFlashWrapper->m_bInitialized)
	{
		return FALSE;
	}

	if(!pFlashWrapper->m_dwFlashInfoSize)
	{
		return FALSE;
	}

	SectorAddr = blockID * pFlashWrapper->m_FlashInfo.wSectorsPerBlock;
	if(bWithStartBlock)
	{
		SectorAddr += (pFlashWrapper->m_dwStartBlock * pFlashWrapper->m_FlashInfo.wSectorsPerBlock);
	}

	fmdInfo.dwAccessCode = FMD_ACCESS_CODE_SETSTATUS;
	fmdInfo.dwStartSector = SectorAddr;
	fmdInfo.pMData = (VOID *)&dwStatus;

	bRc = KernelIoControl(IOCTL_HAL_NANDFMD_ACCESS, (PVOID)(&fmdInfo), sizeof(FmdAccessInfo), NULL, 0, NULL);
	if(!bRc)
	{
		return FALSE;
	}

	//	RETAILMSG(1, (TEXT("OEMFMD_SetBlockStatus() end.\r\n")));
	return TRUE;
}

