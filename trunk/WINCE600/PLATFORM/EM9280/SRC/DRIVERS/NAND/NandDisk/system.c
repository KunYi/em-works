//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:  

    system.c

Abstract:  

    Windows CE Nand disk driver.

--*/

#pragma warning(push)
#pragma warning(disable: 4100 4115 4214)
#include <nanddisk.h>
#pragma warning(pop)


#ifdef DEBUG
//
// These defines must match the ZONE_* defines
//
#define DBG_ERROR			1
#define DBG_WARNING			2
#define DBG_FUNCTION		4
#define DBG_INIT			8
#define DBG_PCMCIA			16
#define DBG_IO				32

DBGPARAM dpCurSettings = {
    TEXT("NAND Disk"), {
    TEXT("Errors"),TEXT("Warnings"),TEXT("Functions"),TEXT("Initialization"),
    TEXT("PCMCIA"),TEXT("Disk I/O"),TEXT("Misc"),TEXT("Undefined"),
    TEXT("Undefined"),TEXT("Undefined"),TEXT("Undefined"),TEXT("Undefined"),
    TEXT("Undefined"),TEXT("Undefined"),TEXT("Undefined"),TEXT("Undefined") },
    0x1
};
#endif


#define REG_PROFILE_KEY     TEXT("Profile")
#define DEFAULT_PROFILE     TEXT("Default")

//#define MAX_BADBLOCK_COUNT		(NAND_BLOCK_CNT >> 5)	// 1/32 blocks
#define MAX_BADBLOCK_COUNT			(1024 >> 5)				// 1/32 blocks -> 32 bad blocks in case of K9F1G08U0A(128MB)


//
// Global Variables
//
CRITICAL_SECTION	v_DiskCrit;
PDISK				v_DiskList;                // initialized to 0 in bss

//static DWORD g_dwBBTSize = MAX_BADBLOCK_COUNT;
//static DWORD g_dwBad[MAX_BADBLOCK_COUNT];
//static DWORD g_dwBadBlockNumber = 0;
//
// CS&ZHL APR-5-2012: save status of all block of reserved nand area
//
static DWORD g_dwBlockStatus[2048];
static DWORD g_dwActualNumBlock = 0;		// actual number of blocks for reserved nand drive

//
// CS&ZHL MAY-20-2011: Flash Info comes from low level
//
static FlashInfo	*g_pFlashInfo = NULL;		// Info about NandFlash chip Block/Sector
static BYTE			g_SectorBuffer[2048];		// we support nandflash chip with the sector size = 2048 only!
//-----------------------------------------------------------------------------
//
//  Function: NAND_ReadSector
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
static BOOL NAND_ReadSector(SECTOR_ADDR startSectorAddr, LPBYTE pSectorBuff, PSectorInfo pSectorInfoBuff, DWORD dwNumSectors)
{
	FmdAccessInfo fmdInfo;
	SECTOR_ADDR SectorAddr = startSectorAddr;
	BOOL bRc = FALSE;

	DEBUGMSG(ZONE_FUNCTION, (TEXT("NAND_ReadSector() start.\r\n")));
	DEBUGMSG(ZONE_FUNCTION, (TEXT("SectorAddr = %d; dwNumSectors = %d.\r\n"), SectorAddr, dwNumSectors));

	fmdInfo.dwAccessCode = FMD_ACCESS_CODE_READSECTOR;
	fmdInfo.dwStartSector = SectorAddr;
	fmdInfo.dwSectorNum = dwNumSectors;
	fmdInfo.pMData = (VOID *)pSectorBuff;
	fmdInfo.pSData = (VOID *)pSectorInfoBuff;

	bRc = KernelIoControl(IOCTL_HAL_NANDFMD_ACCESS, (PVOID)(&fmdInfo), sizeof(FmdAccessInfo), NULL, 0, NULL);

	DEBUGMSG(ZONE_FUNCTION, (TEXT("NAND_ReadSector() end.\r\n")));

	return bRc;
}


//-----------------------------------------------------------------------------
//
//  Function: NAND_GetBlockStatus
//
//  This function returns the status of a block.
//
//  Parameters:
//      dwBlockID 
//          [in] The block number used to check status.
//
//  Returns:  
//      Flags to describe the status of the block.
//
//-----------------------------------------------------------------------------
static DWORD NAND_GetBlockStatus(DWORD dwBlockID)
{
	FmdAccessInfo	fmdInfo;
	DWORD				dwResult = 0;

	DEBUGMSG(ZONE_FUNCTION, (TEXT("NAND_GetBlockStatus() start.\r\n")));

	//
	// CS&ZHL MAY-14-2011: MUST get FlashInfo before reading sector
	//
	if(g_pFlashInfo == NULL)
	{
		RETAILMSG(1, (TEXT("NAND_GetBlockStatus()::g_pFlashInfo == NULL!\r\n")));
		goto cleanUp;
	}

	fmdInfo.dwAccessCode = FMD_ACCESS_CODE_GETSTATUS;
	fmdInfo.dwStartSector = dwBlockID * g_pFlashInfo->wSectorsPerBlock;		// = BLOCK_TO_SECTOR(dwBlockID);
	fmdInfo.pMData = (VOID *)&dwResult;

	if(KernelIoControl(IOCTL_HAL_NANDFMD_ACCESS, (PVOID)(&fmdInfo), sizeof(FmdAccessInfo), NULL, 0, NULL) == FALSE)
	{
		dwResult = BLOCK_STATUS_BAD;
		goto cleanUp;
	}

cleanUp:

	if(BLOCK_STATUS_BAD == dwResult)
	{
		RETAILMSG(TRUE, (_T("Block 0x%x status is 0x%x.\r\n"), dwBlockID, dwResult));
	}

	DEBUGMSG(ZONE_FUNCTION, (TEXT("NAND_GetBlockStatus() end.\r\n")));

	return dwResult;
}

/*
static void InitBadBlockTable(PDISK pDisk)
{
	DWORD		dwDiskBlockCount = pDisk->d_TotalSize / g_pFlashInfo->dwBytesPerBlock;		//NAND_BLOCK_SIZE;
	DWORD		dwTagetBlock;
	DWORD		dwNKStartBlock;

	// CS&ZHL NOV-8-2011: count bad block from start block of NK image
	dwNKStartBlock = IMAGE_BOOT_NKIMAGE_NAND_OFFSET / g_pFlashInfo->dwBytesPerBlock;

	// CS&ZHL MAY-20-2011: allocate bad block table
	//if(!g_dwBBTSize)
	//{
	//	g_dwBBTSize = g_pFlashInfo->dwNumBlocks >> 5;										// 1/32
	//	g_dwBad = (DWORD *)LocalAlloc(LPTR, sizeof(DWORD) * g_dwBBTSize);		//assume always OK!
	//}

	g_dwBadBlockNumber = 0;
	//RETAILMSG(1, (TEXT("NANDDisk: Registry bad block from 0x%x to 0x%x\r\n"), 
	//					pDisk->d_StartBlock, pDisk->d_StartBlock + dwDiskBlockCount));
	for(dwTagetBlock = pDisk->d_StartBlock; dwTagetBlock < pDisk->d_StartBlock + dwDiskBlockCount; dwTagetBlock ++)
	{
		// CS&ZHL NOV-8-2011: skip the blocks of MBR
		if(dwTagetBlock < dwNKStartBlock)
		{
			continue;
		}

		if(NAND_GetBlockStatus(dwTagetBlock) == BLOCK_STATUS_BAD)
		{
			g_dwBad[g_dwBadBlockNumber] = dwTagetBlock;
			g_dwBadBlockNumber ++;

			if(g_dwBadBlockNumber >= g_dwBBTSize)			//MAX_BADBLOCK_COUNT
			{
				RETAILMSG(1, (TEXT("NANDDisk: So many bad blocks, it's impossible.\r\n")));
				break;
			}
		}
	}
	//RETAILMSG(1, (TEXT("NANDDisk: g_dwBadBlockNumber = %d\r\n"), g_dwBadBlockNumber));
}
*/

static void InitBadBlockTable(PDISK pDisk)
{
	DWORD	dwNandReservedBlockID = 0;
	DWORD	dwNandReservedBlockCount = IMAGE_BOOT_NANDDEV_RESERVED_SIZE / g_pFlashInfo->dwBytesPerBlock;

	UNREFERENCED_PARAMETER(pDisk);

	RETAILMSG(1, (TEXT("NANDDisk: Registry block status in Nand Reserved Partition\r\n")));
	for(g_dwActualNumBlock = 0; dwNandReservedBlockID < dwNandReservedBlockCount; g_dwActualNumBlock++)
	{
		g_dwBlockStatus[g_dwActualNumBlock] = NAND_GetBlockStatus(g_dwActualNumBlock);
		if(g_dwBlockStatus[g_dwActualNumBlock] == BLOCK_STATUS_BAD)
		{
			continue;
		}
		// count useable block
		dwNandReservedBlockID++;
	}
	RETAILMSG(1, (TEXT("NANDDisk: g_dwActualNumBlock = %d\r\n"), g_dwActualNumBlock));
}


BOOL GetNextGoodBlock(DWORD StartPhyBlockAddr, DWORD *pNxtGoodBlkAddr)
{
    DWORD BlockStatus;
    
    do
    {
        //We start with next block of StartPhyBlockAddr
        StartPhyBlockAddr++;

        //check border validation
        if(StartPhyBlockAddr >= g_dwActualNumBlock)
		{
			return FALSE;
		}

        //check block status
        BlockStatus = g_dwBlockStatus[StartPhyBlockAddr];

    }while(BlockStatus == BLOCK_STATUS_BAD);

    //return good block address
    *pNxtGoodBlkAddr = StartPhyBlockAddr;

    return TRUE;
}


BOOL GetPhyBlkAddr(DWORD LogicalBlkAddr, DWORD *pNxtGoodBlkAddr)
{
    DWORD i;

    *pNxtGoodBlkAddr = 0;
    for(i=0; i < LogicalBlkAddr+1; i++)
    {
        if(i==0)
        {
            if(g_dwBlockStatus[*pNxtGoodBlkAddr] != BLOCK_STATUS_BAD)
            {
                continue;
            }
        }
        
        if(!GetNextGoodBlock(*pNxtGoodBlkAddr, pNxtGoodBlkAddr))
        {
            return FALSE;
        }
    }
    
    return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function:  GetRealBlockAddress
//
//  This function find the real nand flash block address from the input logical
//  address, skip bad blocks. If there's no bad block, the read block address is
//  same as the logical address.
//
//  Parameters:
//      dwBlockLogAddress 
//          [in] Nand block logical address in flash memory. 
//
//  Returns:
//      If success, return the real block address, skipped bad blocks.
//      If failure, return INVALID_BLOCK_ID.
//
//-----------------------------------------------------------------------------
/*
static DWORD GetRealBlockAddress(DWORD dwBlockLogAddress)
{
	DWORD i;
	DWORD dwBadCount = 0;
	DWORD dwReturnBlock;

	for(i = 0; i < g_dwBadBlockNumber; i ++)
	{
		if(dwBlockLogAddress >= g_dwBad[i])
		{
			dwBadCount ++;
		}
	}
	dwReturnBlock = dwBlockLogAddress + dwBadCount;

	// CS&ZHL NOV-28-2011: the second search is required!
	dwBadCount = 0;
	for(i = 0; i < g_dwBadBlockNumber; i ++)
	{
		if(dwReturnBlock >= g_dwBad[i])
		{
			dwBadCount ++;
		}
	}
	dwReturnBlock = dwBlockLogAddress + dwBadCount;

	for(i = 0; i < g_dwBadBlockNumber; i ++)
	{
		if(dwReturnBlock == g_dwBad[i])
		{
			dwReturnBlock ++;
		}
	}

	return dwReturnBlock;
}
*/


static BOOL IsValidMbr(DWORD dwBlockID, DWORD * pdwDiskSize)
{
	FmdAccessInfo	fmdInfo;
	BOOL			bRc = FALSE;
	PARTENTRY		sPart = {0};
	PPARTENTRY		pPart = &sPart;
	DWORD			i, dwDiskSectors, dwTemp = 0;

	fmdInfo.dwAccessCode = FMD_ACCESS_CODE_READSECTOR;
	fmdInfo.dwStartSector = dwBlockID * g_pFlashInfo->wSectorsPerBlock;		//BLOCK_TO_SECTOR(dwBlockID);
	fmdInfo.dwSectorNum = 1;
	fmdInfo.pMData = (VOID *)g_SectorBuffer;
	fmdInfo.pSData = NULL;

	bRc = KernelIoControl(IOCTL_HAL_NANDFMD_ACCESS, (PVOID)(&fmdInfo), sizeof(FmdAccessInfo), NULL, 0, NULL);
	if(bRc == TRUE)
	{
		if((g_SectorBuffer[0] == 0xE9) && (g_SectorBuffer[1] == 0xfd) && (g_SectorBuffer[2] == 0xff) && 
			(g_SectorBuffer[DEFAULT_SECTOR_SIZE - 2] == 0x55) && (g_SectorBuffer[DEFAULT_SECTOR_SIZE - 1] == 0xAA))
		{
			bRc = TRUE;

			// Check disk size
			dwDiskSectors = 0;
			for(i = 0; i < NUM_PARTS; i ++)
			{
				// retrieve partition info from sector buffer
				memcpy((void *)pPart, (void *)(&g_SectorBuffer[PARTTABLE_OFFSET + sizeof(PARTENTRY) * i]), sizeof(PARTENTRY));
				//RETAILMSG(1, (TEXT("Partition %d, File system 0x%x, StartSector 0x%x, TotalSectors 0x%x.\r\n"), 
				//			i, pPart->Part_FileSystem, pPart->Part_StartSector, pPart->Part_TotalSectors));

				if((pPart->Part_TotalSectors != 0xFFFFFFFF) && (pPart->Part_TotalSectors != 0))
				{
					dwTemp = pPart->Part_StartSector + pPart->Part_TotalSectors;
					if(dwDiskSectors < dwTemp)
						dwDiskSectors = dwTemp;
				}
			}

			if(pdwDiskSize)
			{
				*pdwDiskSize = dwDiskSectors * g_pFlashInfo->wDataBytesPerSector;			//BYTES_PER_SECTOR;
			}
		}
		else
		{
			bRc = FALSE;
		}
	}

	return bRc;
}


static BOOL GetNandDiskInfo(PDISK pDisk)
{
	DWORD dwTargetBlock;
	DWORD dwDiskStartBlock, dwDiskEndBlock;
	DWORD dwDiskSize = 0;
	DWORD dwStatus;

	//
	// CS&ZHL APR-6-2012: search MBR from speicfied area of NandFlash
	//
	dwDiskStartBlock = IMAGE_BOOT_MBR_NAND_OFFSET;
	dwDiskEndBlock = dwDiskStartBlock + 32;			// search 32 blocks
	if(dwDiskEndBlock > g_pFlashInfo->dwNumBlocks)
	{
		dwDiskEndBlock = g_pFlashInfo->dwNumBlocks;
	}

	for(dwTargetBlock = dwDiskStartBlock; dwTargetBlock < dwDiskEndBlock; dwTargetBlock ++)
	{
		dwStatus = NAND_GetBlockStatus(dwTargetBlock);
		if(dwStatus != BLOCK_STATUS_BAD)
		{
			if(IsValidMbr(dwTargetBlock, &dwDiskSize))
			{
				break;
			}
		}
	}

	if(dwTargetBlock >= g_pFlashInfo->dwNumBlocks)					//NAND_BLOCK_CNT
	{
		RETAILMSG(1, (TEXT("NANDDisk: Failed to find MBR.\r\n")));
		return FALSE;
	}

	//
	// CS&ZHL APR-5-2012: always use MBR logical start block
	//
	pDisk->d_StartBlock = dwDiskStartBlock;
	pDisk->d_TotalSize  = dwDiskSize;
	RETAILMSG(1, (TEXT("NANDDisk::MBR => d_StartBlock = 0x%x, d_TotalSize = 0x%x bytes, found at Block[0x%x].\r\n"), 
				pDisk->d_StartBlock, pDisk->d_TotalSize, dwTargetBlock));

	return TRUE;
}



//------------------------------------------------------------------------------
//
// CreateDiskObject - create a DISK structure, init some fields and link it.
//
//------------------------------------------------------------------------------
static PDISK CreateDiskObject(VOID)
{
	PDISK pDisk;

	DEBUGMSG(ZONE_FUNCTION, (TEXT("+CreateDiskObject\r\n")));

	pDisk = LocalAlloc(LPTR, sizeof(DISK));
	if(pDisk != NULL)
	{
		pDisk->d_ActivePath = NULL;
		InitializeCriticalSection(&(pDisk->d_DiskCardCrit));
		EnterCriticalSection(&v_DiskCrit);
		pDisk->d_next = v_DiskList;
		pDisk->d_StartBlock = 0;
		v_DiskList = pDisk;
		LeaveCriticalSection(&v_DiskCrit);
	}
    
	DEBUGMSG(ZONE_FUNCTION, (TEXT("-CreateDiskObject\r\n")));
	return pDisk;
}


//------------------------------------------------------------------------------
//
// IsValidDisk - verify that pDisk points to something in our list
//
// Return TRUE if pDisk is valid, FALSE if not.
//
//------------------------------------------------------------------------------
static BOOL IsValidDisk(PDISK pDisk)
{
	PDISK pd;
	BOOL ret = FALSE;

	EnterCriticalSection(&v_DiskCrit);
	pd = v_DiskList;
	while(pd)
	{
		if(pd == pDisk)
		{
			ret = TRUE;
			break;
		}
		pd = pd->d_next;
	}
	LeaveCriticalSection(&v_DiskCrit);
	return ret;
}


//------------------------------------------------------------------------------
//
// Function to open the driver key specified by the active key
//
// The caller is responsible for closing the returned HKEY
//
//------------------------------------------------------------------------------
static HKEY OpenDriverKey(LPTSTR ActiveKey)
{
	TCHAR DevKey[256];
	HKEY hDevKey;
	HKEY hActive;
	DWORD ValType;
	DWORD ValLen;
	DWORD status;

	//
	// Get the device key from active device registry key
	//
	status = RegOpenKeyEx(
				HKEY_LOCAL_MACHINE, 
				ActiveKey, 
				0, 
				0, 
				&hActive);
	if(status)
	{
		DEBUGMSG(ZONE_INIT | ZONE_ERROR, (TEXT("NandDisk:OpenDriverKey RegOpenKeyEx(HLM\\%s) returned %d!!!\r\n"), ActiveKey, status));
		return NULL;
	}

	hDevKey = NULL;

	ValLen = sizeof(DevKey);
	status = RegQueryValueEx(
				hActive, 
				DEVLOAD_DEVKEY_VALNAME, 
				NULL, 
				&ValType, 
				(PUCHAR)DevKey, 
				&ValLen);
	if(status != ERROR_SUCCESS)
	{
		DEBUGMSG(ZONE_INIT | ZONE_ERROR, (TEXT("NandDisk:OpenDriverKey - RegQueryValueEx(%s) returned %d\r\n"), DEVLOAD_DEVKEY_VALNAME, status));
		goto odk_fail;
	}

	//
	// Get the geometry values from the device key
	//
	status = RegOpenKeyEx(
				HKEY_LOCAL_MACHINE, 
				DevKey, 
				0, 
				0, 
				&hDevKey);
	if(status)
	{
		hDevKey = NULL;
		DEBUGMSG(ZONE_INIT | ZONE_ERROR, (TEXT("NandDisk:OpenDriverKey RegOpenKeyEx - DevKey(HLM\\%s) returned %d!!!\r\n"), DevKey, status));
	}

odk_fail:
	RegCloseKey(hActive);
	return hDevKey;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
BOOL GetDeviceInfo(PDISK pDisk, PSTORAGEDEVICEINFO pInfo)
{
	BOOL fRet = FALSE;
	HKEY hKeyDriver;
	DWORD dwBytes;
	DWORD dwStatus;
	DWORD dwValType;

	if(pDisk && pInfo)
	{
		_tcsncpy(pInfo->szProfile, DEFAULT_PROFILE, PROFILENAMESIZE);
		__try
		{
			hKeyDriver = OpenDriverKey(pDisk->d_ActivePath);
			if(hKeyDriver)
			{
				// read the profile string from the active reg key if it exists
				dwBytes = sizeof(pInfo->szProfile);
				dwStatus = RegQueryValueEx(hKeyDriver, REG_PROFILE_KEY, NULL, &dwValType, (PBYTE)&pInfo->szProfile, &dwBytes);
				RegCloseKey (hKeyDriver);

				// if this fails, szProfile will contain the default string
			}

			// set our class, type, and flags
			pInfo->dwDeviceClass = STORAGE_DEVICE_CLASS_BLOCK;
			pInfo->dwDeviceType = STORAGE_DEVICE_TYPE_UNKNOWN;
			// CS&ZHL NOV-26-2011: try this type -> it's same as the above
			//pInfo->dwDeviceType = STORAGE_DEVICE_TYPE_FLASH;
			//pInfo->dwDeviceFlags = STORAGE_DEVICE_FLAG_READWRITE;
			pInfo->dwDeviceFlags = STORAGE_DEVICE_FLAG_READONLY;
			// CS&ZHL NOV-26-2011: add size return 
			pInfo->cbSize = sizeof(STORAGEDEVICEINFO);
			fRet = TRUE;
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			SetLastError(ERROR_INVALID_PARAMETER);
			fRet = FALSE;
		}
	}
	return fRet;
}


//------------------------------------------------------------------------------
//
// Function to retrieve the folder name value from the driver key. The folder name is
// used by FATFS to name this disk volume.
//
//------------------------------------------------------------------------------
static BOOL GetFolderName(PDISK pDisk, LPWSTR FolderName, DWORD cBytes, DWORD * pcBytes)
{
	HKEY DriverKey;
	DWORD ValType;
	DWORD status;

	DriverKey = OpenDriverKey(pDisk->d_ActivePath);
	if(DriverKey)
	{
		*pcBytes = cBytes;
		status = RegQueryValueEx(
					DriverKey, 
					TEXT("Folder"), 
					NULL, 
					&ValType, 
					(PUCHAR)FolderName, 
					pcBytes);
		if(status != ERROR_SUCCESS)
		{
			DEBUGMSG(ZONE_INIT | ZONE_ERROR, (TEXT("NAND:GetFolderName - RegQueryValueEx(Folder) returned %d\r\n"), status));
			*pcBytes = 0;
		}
		else
		{
			DEBUGMSG(ZONE_INIT | ZONE_ERROR, (TEXT("NAND:GetFolderName - FolderName = %s, length = %d\r\n"), FolderName, *pcBytes));
			*pcBytes += sizeof(WCHAR);  // account for terminating 0.
		}
		RegCloseKey(DriverKey);
		if(status || (*pcBytes == 0))
		{
			// Use default
			return FALSE; 
		}
		return TRUE;
	}

	return FALSE;
}


//------------------------------------------------------------------------------
//
// CloseDisk - free all resources associated with the specified disk
//
//------------------------------------------------------------------------------
static VOID CloseDisk(PDISK pDisk)
{
	PDISK pd;

	DEBUGMSG(ZONE_IO, (TEXT("NANDDISK:CloseDisk closing 0x%x\r\n"), pDisk));

	//
	// Remove it from the global list of disks
	//
	EnterCriticalSection(&v_DiskCrit);
	if(pDisk == v_DiskList)
	{
		v_DiskList = pDisk->d_next;
	}
	else
	{
		pd = v_DiskList;
		while(pd->d_next != NULL)
		{
			if(pd->d_next == pDisk)
			{
				pd->d_next = pDisk->d_next;
				break;
			}
			pd = pd->d_next;
		}
	}
	LeaveCriticalSection(&v_DiskCrit);

	DEBUGMSG(ZONE_IO, (TEXT("NANDDISK:CloseDisk - freeing resources\r\n")));

	//
	// Try to ensure this is the only thread holding the disk crit sec
	//
	Sleep(50);
	EnterCriticalSection(&(pDisk->d_DiskCardCrit));
	LeaveCriticalSection(&(pDisk->d_DiskCardCrit));
	DeleteCriticalSection(&(pDisk->d_DiskCardCrit));
	if(pDisk->d_ActivePath)
	{
		LocalFree(pDisk->d_ActivePath);
	}
	LocalFree(pDisk);
	DEBUGMSG(ZONE_IO, (TEXT("NANDDISK:CloseDisk done with 0x%x\r\n"), pDisk));
}


//------------------------------------------------------------------------------
//
// DoDiskIO - Perform requested I/O.
//            This function is called from DSK_IOControl.
//
// Requests are serialized using the disk's critical section.
//
//------------------------------------------------------------------------------
static DWORD DoDiskIO(PDISK pDisk, DWORD Opcode, PSG_REQ pSgr)
{
	DWORD status = ERROR_SUCCESS;
	DWORD num_sg;
	DWORD curr_byte;
	DWORD bytes_this_sg;
	PSG_BUF pSg = NULL;
	PUCHAR pBuf;
	PUCHAR saveoldptrs[MAX_SG_BUF];
	DWORD num_mapped = 0, i;
	DWORD dwLogBlockID, dwPhyBlockID;

	pSgr->sr_status = ERROR_IO_PENDING;

	if(pSgr->sr_num_sg > MAX_SG_BUF)
	{
		status = ERROR_INVALID_PARAMETER;
		goto ddi_exit;
	}

	//
	// Make sure request doesn't exceed the disk
	//
	if((pSgr->sr_start + pSgr->sr_num_sec - 1) >= pDisk->d_DiskInfo.di_total_sectors)
	{
		status = ERROR_SECTOR_NOT_FOUND;
		goto ddi_exit;
	}
	status = ERROR_SUCCESS;
	curr_byte = pSgr->sr_start * pDisk->d_DiskInfo.di_bytes_per_sect;

	num_sg = pSgr->sr_num_sg;
	pSg = &(pSgr->sr_sglist[0]);
	bytes_this_sg = pSg->sb_len;

	for(num_mapped = 0; num_mapped < num_sg; num_mapped ++)
	{
		saveoldptrs[num_mapped] = pSg[num_mapped].sb_buf;
		if(FAILED(CeOpenCallerBuffer(
					&pSg[num_mapped].sb_buf, 
					saveoldptrs[num_mapped], 
					pSg[num_mapped].sb_len, 
					((Opcode == DISK_IOCTL_READ || Opcode == IOCTL_DISK_READ) ? ARG_O_PTR : ARG_I_PTR), 
					FALSE)))
		{
			status = ERROR_INVALID_PARAMETER;
			goto ddi_exit;
		}
	}

	pBuf = pSg->sb_buf;
	EnterCriticalSection(&(pDisk->d_DiskCardCrit));

	__try
	{
		//
		// Read or write sectors from/to disk.
		// NOTE: Multiple sectors may go in one scatter/gather buffer or one
		// sector may fit in multiple scatter/gather buffers.
		//
		PSG_BUF pCurSg = pSg;
		DEBUGMSG(ZONE_IO, (TEXT("NANDDISK:DoDiskIO - Number of scatter/gather descriptors %d\r\n"), num_sg));
		while(num_sg)
		{
			DEBUGMSG(ZONE_IO, (TEXT("NANDDISK:DoDiskIO - Bytes left for this sg %d\r\n"), bytes_this_sg));
			DEBUGMSG(ZONE_IO, (TEXT("NANDDISK:DoDiskIO - curr_byte %d\r\n"), curr_byte));
			//dwLogBlockID = curr_byte / g_pFlashInfo->dwBytesPerBlock;		//SECTOR_TO_BLOCK(curr_byte / BYTES_PER_SECTOR);
			//dwPhyBlockID = GetRealBlockAddress(dwLogBlockID + pDisk->d_StartBlock);
			if(Opcode == DISK_IOCTL_READ || Opcode == IOCTL_DISK_READ)
			{
				// CS&ZHL MAY-20-2011: add local variable for easy-understanding
				SECTOR_ADDR	startSectorAddr;
				DWORD		dwNumSectors;
				DWORD		dwByteAddr;

				DEBUGMSG(ZONE_IO, (TEXT("NANDDISK:DoDiskIO - reading %d bytes at sector %d\r\n"), bytes_this_sg, curr_byte / pDisk->d_DiskInfo.di_bytes_per_sect));
				//dwLogBlockID = curr_byte / g_pFlashInfo->dwBytesPerBlock;							//SECTOR_TO_BLOCK(curr_byte / BYTES_PER_SECTOR);
				//dwPhyBlockID = GetRealBlockAddress(dwLogBlockID + pDisk->d_StartBlock);
				
				//if(NAND_ReadSector((curr_byte / BYTES_PER_SECTOR) + BLOCK_TO_SECTOR(dwPhyBlockID - dwLogBlockID), pBuf, NULL, bytes_this_sg / BYTES_PER_SECTOR) == FALSE)
				//startSectorAddr = (curr_byte / g_pFlashInfo->wDataBytesPerSector) + ((dwPhyBlockID - dwLogBlockID) * g_pFlashInfo->wSectorsPerBlock);
				dwNumSectors = bytes_this_sg / g_pFlashInfo->wDataBytesPerSector;

				//if(NAND_ReadSector(startSectorAddr, pBuf, NULL, dwNumSectors) == FALSE)
				//{
				//	status = ERROR_GEN_FAILURE;
				//	break;
				//}
				//
				// CS&ZHL NOV-25-2011: read data once a sector to prevent reading accross block border
				//
				dwByteAddr = curr_byte;
				for(i = 0; i < dwNumSectors; i++)
				{
					//dwLogBlockID = dwByteAddr / g_pFlashInfo->dwBytesPerBlock;
					//dwPhyBlockID = GetRealBlockAddress(dwLogBlockID + pDisk->d_StartBlock);
					//startSectorAddr = (dwByteAddr / g_pFlashInfo->wDataBytesPerSector) + ((dwPhyBlockID - dwLogBlockID) * g_pFlashInfo->wSectorsPerBlock);
					//
					// CS&ZHL APR-5-2012: use block0 based bab block avoid algorithm
					//
					dwLogBlockID = (dwByteAddr / g_pFlashInfo->dwBytesPerBlock) + pDisk->d_StartBlock;							
					if(!GetPhyBlkAddr(dwLogBlockID, &dwPhyBlockID))
					{
						RETAILMSG(1, (TEXT("NANDDISK:DoDiskIO - can not find good physical block\r\n")));
						status = ERROR_GEN_FAILURE;
						break;
					}
					startSectorAddr = ((dwByteAddr / g_pFlashInfo->wDataBytesPerSector) % g_pFlashInfo->wSectorsPerBlock) 
						            + (dwPhyBlockID * g_pFlashInfo->wSectorsPerBlock);
					if(NAND_ReadSector(startSectorAddr, pBuf, NULL, 1) == FALSE)
					{
						//RETAILMSG(1, (TEXT("NANDDISK:DoDiskIO - read LogBlock 0x%x - PhyBlock 0x%x SectorAddr 0x%x failed!!!\r\n"), 
						//								(dwLogBlockID + pDisk->d_StartBlock), dwPhyBlockID, startSectorAddr));
						status = ERROR_GEN_FAILURE;
						break;
					}

					/*
					startSectorAddr++;
					if((startSectorAddr % g_pFlashInfo->wSectorsPerBlock) == 0)
					{
						RETAILMSG(1, (TEXT("NANDDISK:DoDiskIO - LogBlock = 0x%x -> PhyBlock 0x%x, goto next block...\r\n"), (dwLogBlockID + pDisk->d_StartBlock), dwPhyBlockID));
						// cross to next block
						dwPhyBlockID++;
						while(NAND_GetBlockStatus(dwPhyBlockID) == BLOCK_STATUS_BAD)
						{
							RETAILMSG(1, (TEXT("NANDDISK:DoDiskIO - PhyBlock 0x%x is a bad block, "), dwPhyBlockID));
							dwPhyBlockID++;
							startSectorAddr += g_pFlashInfo->wSectorsPerBlock;
							RETAILMSG(1, (TEXT("move to next PhyBlock 0x%x!!!\r\n"), dwPhyBlockID));
						}
					}
					*/
					dwByteAddr += g_pFlashInfo->wDataBytesPerSector;
					pBuf += g_pFlashInfo->wDataBytesPerSector;
				}

				// error check
				if(status == ERROR_GEN_FAILURE)
				{
					break;
				}
			}
			else
			{
				//DEBUGMSG(ZONE_IO, (TEXT("NANDDISK:DoDiskIO - writing %d bytes at sector %d\r\n"), bytes_this_sg, curr_byte / pDisk->d_DiskInfo.di_bytes_per_sect));
				//RETAILMSG(1, (TEXT("NANDDISK:DoDiskIO - writing %d bytes at sector %d\r\n"), bytes_this_sg, curr_byte / pDisk->d_DiskInfo.di_bytes_per_sect));
				// Not support
			}

			//
			// Use the next scatter/gather buffer
			//
			num_sg --;
			if(num_sg == 0)
			{
				break;
			}
			pCurSg ++;

			pBuf = pCurSg->sb_buf;
			curr_byte += bytes_this_sg;
			bytes_this_sg = pCurSg->sb_len;
		}  // while sg
        
		//LeaveCriticalSection(&(pDisk->d_DiskCardCrit));
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		if(num_sg)
		{
			//LeaveCriticalSection(&(pDisk->d_DiskCardCrit));
		}
		status = ERROR_INVALID_PARAMETER;
	}
			
	LeaveCriticalSection(&(pDisk->d_DiskCardCrit));

ddi_exit:

	for(i = 0; i < num_mapped; i ++)
	{
		if(FAILED(CeCloseCallerBuffer(
					pSg[i].sb_buf, 
					saveoldptrs[i], 
					pSg[i].sb_len, 
					((Opcode == DISK_IOCTL_READ || Opcode == IOCTL_DISK_READ) ? ARG_O_PTR : ARG_I_PTR))))
		{
			status = ERROR_GEN_FAILURE;
			break;
		}
	}

	pSgr->sr_status = status;
	//RETAILMSG(1, (TEXT("NANDDISK:DoDiskIO - done, status =  0x%x\r\n"), pSgr->sr_status));
	return status;
}


//------------------------------------------------------------------------------
//
// GetDiskInfo - return disk info in response to DISK_IOCTL_GETINFO
//
//------------------------------------------------------------------------------
static DWORD GetDiskInfo(PDISK pDisk, PDISK_INFO pInfo)
{
	//*pInfo = pDisk->d_DiskInfo;
	memcpy(pInfo, &pDisk->d_DiskInfo, sizeof(DISK_INFO));
	pInfo->di_flags |= DISK_INFO_FLAG_PAGEABLE;
	pInfo->di_flags &= ~DISK_INFO_FLAG_UNFORMATTED;
	return ERROR_SUCCESS;
}


//------------------------------------------------------------------------------
//
// SetDiskInfo - store disk info in response to DISK_IOCTL_SETINFO
//
//------------------------------------------------------------------------------
static DWORD SetDiskInfo(PDISK pDisk, PDISK_INFO pInfo)
{
	pDisk->d_DiskInfo = *pInfo;
	return ERROR_SUCCESS;
}


static DWORD GetSectorAddr(PDISK pDisk, DWORD dwSector)
{
	if(dwSector >= pDisk->d_DiskInfo.di_total_sectors)
	{
		return (DWORD)-1;
	}
	else
	{
		return (DWORD)(dwSector * pDisk->d_DiskInfo.di_bytes_per_sect);
	}
}


//------------------------------------------------------------------------------
//
// NANDDISK.DLL entry
//
//------------------------------------------------------------------------------
BOOL WINAPI DllEntry(HINSTANCE DllInstance, DWORD Reason, LPVOID Reserved)
{
	UNREFERENCED_PARAMETER(Reserved);

	switch(Reason)
	{
		case DLL_PROCESS_ATTACH:
			DEBUGMSG(ZONE_INIT, (TEXT("NANDDISK: DLL_PROCESS_ATTACH\r\n")));
			DEBUGREGISTER(DllInstance);
			DisableThreadLibraryCalls((HMODULE) DllInstance);
			break;
    
		case DLL_PROCESS_DETACH:
			DEBUGMSG(ZONE_INIT, (TEXT("NANDDISK: DLL_PROCESS_DETACH\r\n")));
			DeleteCriticalSection(&v_DiskCrit);
			break;
	}
	return TRUE;
}


//------------------------------------------------------------------------------
//
// Returns context data for this Init instance
//
// Arguments:
//      dwContext - registry path for this device's active key
//
//------------------------------------------------------------------------------
DWORD DSK_Init(DWORD dwContext)
{
	PDISK		  pDisk;
	LPWSTR		  ActivePath = (LPWSTR)dwContext;
	FmdAccessInfo fmdInfo;
	DWORD		  dwSize = sizeof(FlashInfo);

	//DEBUGMSG(ZONE_INIT, (TEXT("NANDDISK: DSK_Init entered\r\n")));
	RETAILMSG(1, (TEXT("NANDDISK: DSK_Init entered\r\n")));

	fmdInfo.dwAccessCode = FMD_ACCESS_CODE_HWINIT;

	if(KernelIoControl(IOCTL_HAL_NANDFMD_ACCESS, (PVOID)(&fmdInfo), sizeof(FmdAccessInfo), NULL, 0, NULL) == FALSE)
	{
		RETAILMSG(1,(TEXT("NANDDISK:IOCTL_HAL_NANDFMD_ACCESS(FMD_ACCESS_CODE_HWINIT) failed %d\r\n"), GetLastError()));
		return 0;
	}

	//
	// CS&ZHL MAY-20-2011: get info of current nandflash chip
	//
	if(g_pFlashInfo == NULL)
	{
		g_pFlashInfo = LocalAlloc(LPTR, sizeof(FlashInfo));
		if(g_pFlashInfo == NULL)
		{
			RETAILMSG(1,(TEXT("NANDDISK:LocalAlloc(FlashInfo) Failed! %d\r\n"), GetLastError()));
			return 0;
		}

		//get flash info
		dwSize = sizeof(FlashInfo);
		fmdInfo.dwAccessCode = FMD_ACCESS_CODE_GETINFO;
		fmdInfo.pMData = (VOID *)g_pFlashInfo;
		fmdInfo.pSData = (VOID *)&dwSize;

		if(KernelIoControl(IOCTL_HAL_NANDFMD_ACCESS, (PVOID)(&fmdInfo), sizeof(FmdAccessInfo), NULL, 0, NULL) == FALSE)
		{
			RETAILMSG(1, (TEXT("NANDDISK:Get NandFlash Info Failed %d\r\n"), GetLastError()));
			return 0;
		}
	}

	if(v_DiskList == NULL)
	{
		InitializeCriticalSection(&v_DiskCrit);
	}

	pDisk = CreateDiskObject();
	if(pDisk == NULL)
	{
		RETAILMSG(1,(TEXT("NANDDISK: LocalAlloc(PDISK) failed %d\r\n"), GetLastError()));
		LocalFree(g_pFlashInfo);			// CS&ZHL MAY-20-2011: remember free!
		return 0;
	}

	if(ActivePath)
	{
		DEBUGMSG(ZONE_INIT, (TEXT("NANDDISK : ActiveKey = %s\r\n"), ActivePath));
		//if(pDisk->d_ActivePath = LocalAlloc(LPTR, wcslen(ActivePath) * sizeof(WCHAR) + sizeof(WCHAR)))
		pDisk->d_ActivePath = LocalAlloc(LPTR, wcslen(ActivePath) * sizeof(WCHAR) + sizeof(WCHAR));
		if(pDisk->d_ActivePath)
		{
			wcscpy(pDisk->d_ActivePath, ActivePath);
		}
		DEBUGMSG(ZONE_INIT, (TEXT("NANDDISK : ActiveKey (copy) = %s (@ 0x%08X)\r\n"), pDisk->d_ActivePath, pDisk->d_ActivePath));
	}

	if(GetNandDiskInfo(pDisk) == FALSE)
	{
		RETAILMSG(1,(TEXT("NANDDISK: GetNandDiskInfo(pDisk) failed %d\r\n"), GetLastError()));
		if(pDisk->d_ActivePath)
		{
			LocalFree(pDisk->d_ActivePath);
		}
		LocalFree(pDisk);
		LocalFree(g_pFlashInfo);			// CS&ZHL MAY-20-2011: remember free!
		return 0;
	}    

	//pDisk->d_DiskInfo.di_total_sectors = pDisk->d_TotalSize / BYTES_PER_SECTOR;
	//pDisk->d_DiskInfo.di_bytes_per_sect = BYTES_PER_SECTOR;
	//
	// CS&ZHL MAY-20-2011: use FlashInfo instead of constant
	//
	pDisk->d_DiskInfo.di_total_sectors = pDisk->d_TotalSize / g_pFlashInfo->wDataBytesPerSector;		//BYTES_PER_SECTOR
	pDisk->d_DiskInfo.di_bytes_per_sect = g_pFlashInfo->wDataBytesPerSector;							//BYTES_PER_SECTOR
	pDisk->d_DiskInfo.di_cylinders = 0;
	pDisk->d_DiskInfo.di_heads = 0;
	pDisk->d_DiskInfo.di_sectors = pDisk->d_DiskInfo.di_total_sectors / (16 * 2);
	pDisk->d_DiskInfo.di_flags = DISK_INFO_FLAG_PAGEABLE;												// CS&ZHL NOV-28-2011: why not DISK_INFO_FLAG_MBR?
	//
	// CS&ZHL NOV-28-2011: try to setup a proper CHS addressing 
	//
	//pDisk->d_DiskInfo.di_flags = DISK_INFO_FLAG_PAGEABLE | DISK_INFO_FLAG_CHS_UNCERTAIN;
	//pDisk->d_DiskInfo.di_sectors = 32;
	//pDisk->d_DiskInfo.di_heads = 64;
	//pDisk->d_DiskInfo.di_cylinders = pDisk->d_DiskInfo.di_total_sectors / (pDisk->d_DiskInfo.di_sectors * pDisk->d_DiskInfo.di_sectors);
	//if(pDisk->d_DiskInfo.di_total_sectors % (pDisk->d_DiskInfo.di_sectors * pDisk->d_DiskInfo.di_sectors))
	//{
	//	pDisk->d_DiskInfo.di_cylinders++;
	//}
	//pDisk->d_DiskInfo.di_total_sectors = pDisk->d_DiskInfo.di_cylinders * pDisk->d_DiskInfo.di_sectors * pDisk->d_DiskInfo.di_sectors;
	
	RETAILMSG(1, (TEXT("NANDDISK: di_total_sectors = 0x%x, di_bytes_per_sect = 0x%x\r\n"), pDisk->d_DiskInfo.di_total_sectors, pDisk->d_DiskInfo.di_bytes_per_sect));

	InitBadBlockTable(pDisk);

	DEBUGMSG(ZONE_INIT, (TEXT("NANDDISK: sectors = %d\r\n"), pDisk->d_DiskInfo.di_total_sectors));
    
	//DEBUGMSG(ZONE_INIT, (TEXT("NANDDISK: DSK_Init returning 0x%x\r\n"), pDisk));
	RETAILMSG(1, (TEXT("NANDDISK: DSK_Init returning 0x%x\r\n"), pDisk));

	return (DWORD)pDisk;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
BOOL DSK_Close(DWORD Handle)
{
	PDISK pDisk = (PDISK)Handle;
	//BOOL bClose = FALSE;

	DEBUGMSG(ZONE_IO, (TEXT("NANDDISK: DSK_Close entered\r\n")));

	if(!IsValidDisk(pDisk))
	{
		return FALSE;
	}

	DEBUGMSG(ZONE_IO, (TEXT("NANDDISK: DSK_Close done\r\n")));
	return TRUE;
}


//------------------------------------------------------------------------------
//
// Device deinit - devices are expected to close down.
// The device manager does not check the return code.
//
//------------------------------------------------------------------------------
BOOL DSK_Deinit(DWORD dwContext)
{
	DEBUGMSG(ZONE_INIT, (TEXT("NANDDISK: DSK_Deinit entered\r\n")));
	DSK_Close(dwContext);
	CloseDisk((PDISK)dwContext);
	//
	// CS&ZHL MAY-20-2011: free ...
	//
	if(g_pFlashInfo)
	{
		LocalFree(g_pFlashInfo);
		g_pFlashInfo = NULL;
	}

	/*if(g_dwBBTSize)
	{
		LocalFree(g_dwBad);
		g_dwBad = NULL;
		g_dwBBTSize = 0;
		g_dwBadBlockNumber = 0;
	}*/

	DEBUGMSG(ZONE_INIT, (TEXT("NANDDISK: DSK_Deinit done\r\n")));
	return TRUE;
}


//------------------------------------------------------------------------------
//
// Returns handle value for the open instance.
//
//------------------------------------------------------------------------------
DWORD DSK_Open(DWORD dwData, DWORD dwAccess, DWORD dwShareMode)
{
	PDISK pDisk = (PDISK)dwData;

	UNREFERENCED_PARAMETER(dwAccess);
	UNREFERENCED_PARAMETER(dwShareMode);

	//RETAILMSG(1, (TEXT("NANDDISK: DSK_Open enter pDisk = 0x%x\r\n"), (DWORD)pDisk));

	if(IsValidDisk(pDisk) == FALSE)
	{
		RETAILMSG(1, (TEXT("NANDDISK: DSK_Open - Passed invalid disk handle\r\n")));
		return 0;
	}

	//RETAILMSG(1, (TEXT("NANDDISK: DSK_Open returning 0x%x\r\n"), dwData));
	return dwData;
}


//------------------------------------------------------------------------------
//
// I/O Control function - responds to info, read and write control codes.
// The read and write take a scatter/gather list in pInBuf
//
//------------------------------------------------------------------------------
BOOL DSK_IOControl(DWORD Handle, DWORD dwIoControlCode, PBYTE pInBuf, DWORD nInBufSize, PBYTE pOutBuf, DWORD nOutBufSize, PDWORD pBytesReturned)
{
	PSG_REQ pSG;
	PDISK pDisk = (PDISK)Handle;

	DEBUGMSG(ZONE_FUNCTION, (TEXT("+DSK_IOControl (%d) \r\n"), dwIoControlCode));
	//RETAILMSG(1, (TEXT("+DSK_IOControl (%d) \r\n"), dwIoControlCode));

	if(IsValidDisk(pDisk) == FALSE)
	{
		SetLastError(ERROR_INVALID_HANDLE);
		DEBUGMSG(ZONE_FUNCTION, (TEXT("-DSK_IOControl (invalid disk) \r\n")));
		return FALSE;
	}

	//
	// Check parameters
	//
	switch(dwIoControlCode)
	{
		case DISK_IOCTL_READ:
		case IOCTL_DISK_READ:        
		case DISK_IOCTL_WRITE:
		case IOCTL_DISK_WRITE:        
		case DISK_IOCTL_INITIALIZED:
			if(pInBuf == NULL)
			{
				SetLastError(ERROR_INVALID_PARAMETER);
				return FALSE;
			}
			break;

		case DISK_IOCTL_GETNAME:
		case IOCTL_DISK_GETNAME:        
			if(pOutBuf == NULL)
			{
				SetLastError(ERROR_INVALID_PARAMETER);
				return FALSE;
			}
			break;

		case IOCTL_DISK_GETINFO:
			if(pOutBuf == NULL || nOutBufSize != sizeof(DISK_INFO))
			{
				SetLastError(ERROR_INVALID_PARAMETER);
				return FALSE;
			}
			break;

		case DISK_IOCTL_GETINFO:
		case DISK_IOCTL_SETINFO:
		case IOCTL_DISK_SETINFO:
			if(pInBuf == NULL || nInBufSize != sizeof(DISK_INFO)) 
			{
				SetLastError(ERROR_INVALID_PARAMETER);
				return FALSE;
			}
			break;

		case IOCTL_DISK_DEVICE_INFO:
			if(!pInBuf || nInBufSize != sizeof(STORAGEDEVICEINFO))
			{
				SetLastError(ERROR_INVALID_PARAMETER);
				return FALSE;
			}
			break;
        
		case IOCTL_DISK_GET_SECTOR_ADDR:
			if(pInBuf == NULL || pOutBuf == NULL || nInBufSize != nOutBufSize)
			{
				SetLastError(ERROR_INVALID_PARAMETER);
				return FALSE;
			}
			break;

		case DISK_IOCTL_FORMAT_MEDIA:
		case IOCTL_DISK_FORMAT_MEDIA:
			SetLastError(ERROR_SUCCESS);
			return TRUE;
    
		default:
			SetLastError(ERROR_INVALID_PARAMETER);
			return FALSE;
	}

	//
	// Execute dwIoControlCode
	//
	switch(dwIoControlCode)
	{
		case DISK_IOCTL_READ:
		case IOCTL_DISK_READ:
		case DISK_IOCTL_WRITE:
		case IOCTL_DISK_WRITE:
			//RETAILMSG(1, (TEXT("-DSK_IOControl (DoDiskIO) \r\n")));					//debug only
			pSG = (PSG_REQ)pInBuf;
			if(!(pSG && nInBufSize >= (sizeof(SG_REQ) + sizeof(SG_BUF) * (pSG->sr_num_sg - 1))))
			{
				SetLastError(ERROR_INVALID_PARAMETER);
				return FALSE;
			}
			DoDiskIO(pDisk, dwIoControlCode, pSG);
			if(pSG->sr_status)
			{
				SetLastError(pSG->sr_status);
				return FALSE;
			}
			return TRUE;

		case DISK_IOCTL_GETINFO:
		case IOCTL_DISK_GETINFO:
			//RETAILMSG(1, (TEXT("NandDisk->DSK_IOControl (GetDiskInfo) \r\n")));			//debug only
			SetLastError(GetDiskInfo(pDisk, (PDISK_INFO)(dwIoControlCode == IOCTL_DISK_GETINFO ? pOutBuf : pInBuf)));
			return TRUE;

		case DISK_IOCTL_SETINFO:
		case IOCTL_DISK_SETINFO:
			//RETAILMSG(1, (TEXT("NandDisk->DSK_IOControl (SetDiskInfo) \r\n")));				//debug only
			SetLastError(SetDiskInfo(pDisk, (PDISK_INFO)pInBuf));
			return TRUE;

		case DISK_IOCTL_INITIALIZED:
			return TRUE;

		case DISK_IOCTL_GETNAME:
		case IOCTL_DISK_GETNAME:        
			DEBUGMSG(ZONE_FUNCTION, (TEXT("-DSK_IOControl (name) \r\n")));
			//RETAILMSG(1, (TEXT("NandDisk->DSK_IOControl (GetFolderName) \r\n")));		//debug only
			return GetFolderName(pDisk, (LPWSTR)pOutBuf, nOutBufSize, pBytesReturned);

		case IOCTL_DISK_DEVICE_INFO: // new ioctl for disk info
			DEBUGMSG(ZONE_FUNCTION, (TEXT("-DSK_IOControl (device info) \r\n")));
			//RETAILMSG(1, (TEXT("NandDisk->DSK_IOControl (GetDeviceInfo) \r\n")));			//debug only
			return GetDeviceInfo(pDisk, (PSTORAGEDEVICEINFO)pInBuf);

		case IOCTL_DISK_GET_SECTOR_ADDR:
			{
				LPDWORD pLogicalSectors = (LPDWORD)pInBuf;
				LPDWORD pAddresses = (LPDWORD)pOutBuf;
				DWORD dwNumSectors = nInBufSize / sizeof(DWORD);
				DWORD i;
        
				DEBUGMSG(ZONE_FUNCTION, (TEXT("DSK_IOControl(IOCTL_DISK_GET_SECTOR_ADDR)\r\n")));

				RETAILMSG(1, (TEXT("NandDisk->DSK_IOControl (GetSectorAddr) \r\n")));
				for(i = 0; i < dwNumSectors; i ++)
				{
					pAddresses[i] = GetSectorAddr(pDisk, pLogicalSectors[i]);
				}
				return TRUE;
			}

		default:
			DEBUGMSG(ZONE_FUNCTION, (TEXT("-DSK_IOControl (default) \r\n")));
			SetLastError(ERROR_INVALID_PARAMETER);
			return FALSE;
	}
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
DWORD DSK_Read(DWORD Handle, LPVOID pBuffer, DWORD dwNumBytes)
{
	UNREFERENCED_PARAMETER(Handle);
	UNREFERENCED_PARAMETER(pBuffer);
	UNREFERENCED_PARAMETER(dwNumBytes);

	return 0;
}

DWORD DSK_Write(DWORD Handle, LPCVOID pBuffer, DWORD dwNumBytes)
{
	UNREFERENCED_PARAMETER(Handle);
	UNREFERENCED_PARAMETER(pBuffer);
	UNREFERENCED_PARAMETER(dwNumBytes);

	return 0;
}

DWORD DSK_Seek(DWORD Handle, long lDistance, DWORD dwMoveMethod)
{
	UNREFERENCED_PARAMETER(Handle);
	UNREFERENCED_PARAMETER(lDistance);
	UNREFERENCED_PARAMETER(dwMoveMethod);

	return 0;
}

void DSK_PowerUp(void){}
void DSK_PowerDown(void){}


