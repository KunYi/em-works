//-----------------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//  Use of this source code is subject to the terms of the Microsoft end-user
//  license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
//  If you did not accept the terms of the EULA, you are not authorized to use
//  this source code. For a copy of the EULA, please see the LICENSE.RTF on
//  your install media.
//
//-----------------------------------------------------------------------------
//
//  Copyright (C) 2007-2010, Freescale Semiconductor, Inc. All Rights Reserved.
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
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include "bsp.h"
#include "loader.h"
#include "nandboot.h"
#include "bcb.h"
#include "otp.h"
#include "security.h"		// CS&ZHL APR-12-2012: support security fucntion
#pragma warning(pop)
//-----------------------------------------------------------------------------
// External Functions

//-----------------------------------------------------------------------------
// External Variables
extern FlashInfoExt g_CSPfi;

static NANDImgInfo g_ImgInfo;

//-----------------------------------------------------------------------------
// Defines
#define NANDFC_BOOT_SIZE (4096*8)

//-----------------------------------------------------------------------------
// Types

//-----------------------------------------------------------------------------
// Global Variables
NAND_IMAGE_CFG	NANDImageCfg;
SECTOR_BUFFER	SectorBuffer;
//-----------------------------------------------------------------------------
// Local Variables
BYTE sectorBuf[NANDFC_BOOT_SIZE];

// CS&ZHL APR-13-2012: setup flag to control SyncVendorInfo
#ifdef	EM9280_SYNC_VENDOR_INFO
BOOL	g_bAbortVendorInfoSync = FALSE;
#else
BOOL	g_bAbortVendorInfoSync = TRUE;
#endif	//EM9280_SYNC_VENDOR_INFO

// CS&ZHL MAY-29-2012: buffer for security info from nandflash
VENDOR_SECURITY_INFO	g_SecurityInfo;

//-----------------------------------------------------------------------------
// Local Functions
VOID NANDGetImgInfo(PNANDImgInfo pInfo);

// CS&ZHL APR-11-2012: routines for NandFlash Security
BOOL OTPSyncEbootCfg(PBYTE pBuf, DWORD dwBufSize);
BOOL OTPSyncVendorInfo(PBYTE pBuf, DWORD dwBufSize);
BOOL NANDVendorAuthentication(void);
BOOL NANDUserAuthentication(PBYTE pBuf, DWORD dwBufSize);

// get bad block layout info
// return = number of blocks in the nandflash chip
DWORD GetBadBlockInfo(PBYTE pBuf, DWORD dwBufSize);

//------------------------------------------------------------------------------
//
// Function: BSP_GetNANDBufferPointer
//
//    Get the Sector Buffer pointer
//
// Parameters:
//        pSectorBuffer[out] - buffer parameters
//
// Returns:
//        the buffer parameters
//
//------------------------------------------------------------------------------
void BSP_GetNANDBufferPointer(PSECTOR_BUFFER pSectorBuffer)
{   
    pSectorBuffer->pSectorBuf = sectorBuf;
    pSectorBuffer->dwSectorBufSize = NANDFC_BOOT_SIZE;
}

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
    FlashInfo flashInfo;
    
    if (!FMD_GetInfo(&flashInfo))
    {
        RETAILMSG(TRUE, (_T("ERROR: Unable to get NAND flash information.\r\n")));
        return;
    }
    
    pNANDImageCfg->dwXldrOffset = 0xFFFFFFFF;
    pNANDImageCfg->dwXldrSize = 0;

    pNANDImageCfg->dwBootOffset = IMAGE_BOOT_BOOTIMAGE_NAND_OFFSET * flashInfo.dwBytesPerBlock;
    pNANDImageCfg->dwBootSize = IMAGE_BOOT_BOOTIMAGE_NAND_BLOCKS * flashInfo.dwBytesPerBlock;

    pNANDImageCfg->dwIplOffset = 0xFFFFFFFF;
    pNANDImageCfg->dwIplSize = 0;
    
    pNANDImageCfg->dwNkOffset = IMAGE_BOOT_NKIMAGE_NAND_OFFSET * flashInfo.dwBytesPerBlock;
    pNANDImageCfg->dwNkSize = IMAGE_BOOT_NKIMAGE_NAND_SIZE;

    pNANDImageCfg->dwDioOffset = 0xFFFFFFFF;
    pNANDImageCfg->dwDioSize = 0;
    
    pNANDImageCfg->dwCfgOffset = IMAGE_BOOT_BOOTCFG_NAND_OFFSET * flashInfo.dwBytesPerBlock;
    pNANDImageCfg->dwCfgSize = IMAGE_BOOT_BOOTCFG_NAND_BLOCKS * flashInfo.dwBytesPerBlock;

    pNANDImageCfg->dwIplRamStart = 0xFFFFFFFF;
    pNANDImageCfg->dwNkRamStart = IMAGE_BOOT_NKIMAGE_RAM_PA_START;
    pNANDImageCfg->dwNandSize = IMAGE_BOOT_NANDDEV_RESERVED_SIZE;

	// CS&ZHL JAN-9-2012: add splash screen configuration message
	pNANDImageCfg->dwSplashOffset = IMAGE_BOOT_SPLASH_NAND_OFFSET * flashInfo.dwBytesPerBlock;

	// CS&ZHL DEC-13-2011: add FAT configuration message
	pNANDImageCfg->dwFATFSOffset = IMAGE_FILE_SYSTEM_NAND_OFFSET * flashInfo.dwBytesPerBlock;

	// CS&ZHL MAR-28-2012: for BinFS
	pNANDImageCfg->dwNandDevPhyAddr = IMAGE_BOOT_NKIMAGE_NAND_PA_START;						// --> 0x44200000

	// CS&ZHL MAR-28-2012: MBR
	pNANDImageCfg->dwMBROffset = IMAGE_BOOT_MBR_NAND_OFFSET * flashInfo.dwBytesPerBlock;	// 0x2E
	pNANDImageCfg->dwMBRSize = IMAGE_BOOT_MBR_NAND_BLOCKS * flashInfo.dwBytesPerBlock;

	// CS&ZHL APR-9-2012: allocate blocks for security of EM9280
	pNANDImageCfg->dwVIDOffset = IMAGE_BOOT_VID_NAND_OFFSET * flashInfo.dwBytesPerBlock;
	pNANDImageCfg->dwVIDSize   = IMAGE_BOOT_VID_NAND_BLOCKS * flashInfo.dwBytesPerBlock;
	pNANDImageCfg->dwUIDOffset = IMAGE_BOOT_UID_NAND_OFFSET * flashInfo.dwBytesPerBlock;
	pNANDImageCfg->dwUIDSize   = IMAGE_BOOT_UID_NAND_BLOCKS * flashInfo.dwBytesPerBlock;
	pNANDImageCfg->dwRFUOffset = IMAGE_BOOT_RFU_NAND_OFFSET * flashInfo.dwBytesPerBlock;
	pNANDImageCfg->dwRFUSize   = IMAGE_BOOT_RFU_NAND_BLOCKS * flashInfo.dwBytesPerBlock;

	// CS&ZHL MAR-28-2012: Bin
	pNANDImageCfg->dwBinOffset = 0;
	pNANDImageCfg->dwBinSize = 0;
}

//
// CS&ZHL DEC-13-2011: return NandFlash configuration info
//
DWORD BSP_NANDImageFileSystemOffset()
{
	return NANDImageCfg.dwFATFSOffset;
}

//-----------------------------------------------------------------------------
//
//  Function:  NANDBootInit
//
//      This function reads the image configuration and sector buffer.
//
//  Parameters:
//      None.        
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
static BOOL NANDBootInit()
{ 
    // Get the Image Cfg
    BSP_GetNANDImageCfg(&NANDImageCfg);
    // Get the Sector Buffer
    BSP_GetNANDBufferPointer(&SectorBuffer);
    if(SectorBuffer.pSectorBuf == NULL)
    {
        return FALSE;
    }

    if(SectorBuffer.dwSectorBufSize == 0)
    {
        return FALSE;
    }
    
    memset(SectorBuffer.pSectorBuf, 0xFF, SectorBuffer.dwSectorBufSize);
    NANDGetImgInfo(&g_ImgInfo);
    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function:  NANDBootReserved
//
//      This function reserves the blocks of the image area.
//
//  Parameters:
//      None.        
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
void NANDBootReserved()
{
    DWORD			dwResult;
    FlashInfo		flashInfo;
    BLOCK_ID		blockID, startBlockID, endBlockID;
	static DWORD	dwNANDBootReservedCount = 0;
    
    //RETAILMSG(TRUE, (_T("->NANDBootReserved %d\r\n"), dwNANDBootReservedCount));
	//if(!dwNANDBootReservedCount)	// for test
	//{
	//	// turn off GPMI_CLK
	//	RETAILMSG(TRUE, (_T("NANDBootReserved: turn off GPMI_CLK\r\n")));
	//  DDKClockSetGatingMode(DDK_CLOCK_GATE_GPMI_CLK, TRUE);
	//	RETAILMSG(TRUE, (_T("NANDBootReserved: sleep 100ms\r\n")));
	//	Sleep(100);
	//	// turn on GPMI_CLK again
	//	DDKClockSetGatingMode(DDK_CLOCK_GATE_GPMI_CLK, FALSE);
	//	RETAILMSG(TRUE, (_T("NANDBootReserved: turn on GPMI_CLK again\r\n")));
	//}
	dwNANDBootReservedCount++;

    // Boot Configuration and Buffer
    if(!NANDBootInit())
    {
        RETAILMSG(TRUE, (_T("WARNING: NANDBootInit failed.\r\n")));
        return ;
    }
    
    if(!FMD_GetInfo(&flashInfo))
    {
        RETAILMSG(TRUE, (_T("ERROR: Unable to get NAND flash information.\r\n")));
        return;
    }
   
    startBlockID = 0;
    endBlockID = (DWORD)(NANDImageCfg.dwNandSize + flashInfo.dwBytesPerBlock - 1) / (DWORD)flashInfo.dwBytesPerBlock;
    //RETAILMSG(TRUE, (_T("INFO: Set NAND flash blocks [0x%x ~ 0x%x] as reserved.\r\n"), startBlockID, endBlockID-1));        

    //for(blockID = startBlockID; blockID < endBlockID; blockID++)
    for(blockID = startBlockID; startBlockID < endBlockID; blockID++)	// CS&ZHL APR-2-2012: counting valid block number only!		
	{
        dwResult = FMD_GetBlockStatus(blockID);
		//if( dwResult != 0 )
		//{
		//	RETAILMSG(TRUE, (_T("Result:0x%x blockID:0x%x startBlockID:0x%x\r\n"), dwResult, blockID, startBlockID));        
		//}

        // Skip bad blocks
        if(dwResult & BLOCK_STATUS_BAD)
        {
			RETAILMSG(TRUE, (_T("INFO: Found bad NAND flash block [0x%x].\r\n"), blockID));  
            continue;
        }

        // Skip reserved blocks
        if(dwResult & BLOCK_STATUS_RESERVED)
        {
			// CS&ZHL APR-2-2012: count valid block only!
			startBlockID++;
            continue;
        }
        
        // Erase the block...
        if(!FMD_EraseBlock(blockID))
        {
			RETAILMSG(TRUE, (_T("ERROR: Unable to erase NAND block [0x%x]. Mark as bad block!\r\n"), blockID));        
            FMD_SetBlockStatus(blockID, BLOCK_STATUS_BAD);
            continue;
        }
        
        // Set reserved flag
        if(!FMD_SetBlockStatus(blockID, BLOCK_STATUS_RESERVED))
        {
			RETAILMSG(TRUE, (_T("ERROR: Unable to set flag to NAND block [0x%x]. Mark as bad block!\r\n"), blockID));        
            FMD_SetBlockStatus(blockID, BLOCK_STATUS_BAD);
            continue;
        }

		// CS&ZHL APR-2-2012: count valid block only!
		startBlockID++;
    }

	//RETAILMSG(TRUE, (_T("<-NANDBootReserved\r\n")));
}

/*
	if(!dwNANDBootReservedCount)
	{
	    SectorInfo	sectorInfo;
		LPBYTE		pSectorBuf;

		RETAILMSG(TRUE, (_T("NANDBootReserved: do dummy read\r\n")));

		// setup dummy buffer
		pSectorBuf = sectorBuf;
		memset(&sectorInfo, 0xFF, sizeof(sectorInfo));		// Fill the sectorInfo
		sectorInfo.bOEMReserved &= ~OEM_BLOCK_RESERVED;		// Set Reserved flag

		if (!FMD_ReadSector(0, pSectorBuf, &sectorInfo, 1))
        {
            RETAILMSG(TRUE, (_T("NANDBootReserved: dummy read failed\r\n")));
        }
	}
*/

//-----------------------------------------------------------------------------
//
//  Function:  NANDLowLevelFormat
//
//  This function formats (erases) the entire NAND flash memory without
//  checking the NAND flash block status.
//
//  Parameters:
//      None.     
//
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//-----------------------------------------------------------------------------
BOOL NANDLowLevelFormat(void)
{
    UINT32 blockID, startBlockID, endBlockID;
    UINT32 percentComplete, lastPercentComplete;
    FlashInfo flashInfo;

    if(!FMD_GetInfo(&flashInfo))
    {
        RETAILMSG(TRUE, (_T("ERROR: Unable to get NAND flash information.\r\n")));
        return FALSE;
    }

    // Calculate the physical block range for the enrire NAND device
    startBlockID = 0;
    endBlockID = flashInfo.dwNumBlocks;

    RETAILMSG(TRUE, (_T("INFO: Start erasing whole NAND space! 0 - %d\r\n"), endBlockID ));

    lastPercentComplete = 0;

    for (blockID = startBlockID; blockID < endBlockID ; blockID++)
    {
        // Erase the block...
        //
        if (!FMD_EraseBlock(blockID))
        {
            RETAILMSG(TRUE, (_T("ERROR: Unable to erase NAND flash block 0x%x.\r\n"), blockID));
			//FMD_SetBlockStatus(blockID, BLOCK_STATUS_BAD);
            continue;
        }
        else
        {
            percentComplete = 100 * (blockID - startBlockID + 1) / (endBlockID - startBlockID);

            // If percentage complete has changed, show the progress
            if (lastPercentComplete != percentComplete)
            {
                lastPercentComplete = percentComplete;
                RETAILMSG(TRUE, (_T("\rINFO: Erasing is %d%% complete."), percentComplete));
            }
        }
    }
    
    RETAILMSG(TRUE, (_T("\r\nINFO: Erasing whole NAND space completed successfully.\r\n"))); 
    RETAILMSG(TRUE, (_T("INFO: Please power off the board.\r\n")));  
    RETAILMSG(TRUE, (_T("INFO: Spin Forever...\r\n"))); 
    //Spin forever
    while(endBlockID!=0)
    {
        endBlockID = lastPercentComplete;         
    }

    return(TRUE);
}

//----------------------------------------------------------------------------------------
//
//  Function:  NANDLowLevelFormat
// 
//	This function gets bad block layout info
//
//  Parameters:
//      pBuf      - bad block flag table. 1'b1 -> bad block, 1'b0 -> good block
//      dwBufSize - pBuf Size in byte
//
//  Returns:
//      number of blocks statisticesed in pBuf
//
//----------------------------------------------------------------------------------------
DWORD GetBadBlockInfo(PBYTE pBuf, DWORD dwBufSize)
{
    DWORD			dwResult;
    FlashInfo		flashInfo;
    BLOCK_ID		blockID;
	DWORD			dwNumBlocks;
	DWORD			dwByteIndex, dwBitIndex;
    
	//make reserved first
	//NANDBootReserved();

    if(!FMD_GetInfo(&flashInfo))
    {
        RETAILMSG(TRUE, (_T("ERROR: Unable to get NAND flash information.\r\n")));
        return 0;
    }
    
	// clear the buffer
	memset(pBuf, 0, dwBufSize);

	dwNumBlocks = 8 * dwBufSize;						// max 4096 blocks
	if(dwNumBlocks > flashInfo.dwNumBlocks)
	{
		dwNumBlocks = flashInfo.dwNumBlocks;
	}

	for(blockID = 0; blockID < dwNumBlocks; blockID++ ) 
	{
        dwResult = FMD_GetBlockStatus(blockID);
        
        if(dwResult & BLOCK_STATUS_BAD)
        {
			// set flags for each bad blocks
			dwByteIndex = blockID / 8;
			dwBitIndex  = blockID % 8;
			pBuf[dwByteIndex] |= (1 << dwBitIndex);
			//RETAILMSG(TRUE, (_T("0x%x - %d: 0x%x\r\n"), blockID, dwByteIndex, pBuf[dwByteIndex] ));
        }
	}

	return dwNumBlocks;
}


//----------------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------------
static BOOL GetGoodPhyBlock(DWORD LogBlockAddr, DWORD *pGoodBlkAddr)
{
    DWORD StartPhyBlockAddr = 0;
    DWORD i;
    FlashInfo flashInfo;
    
    if (!FMD_GetInfo(&flashInfo))
    {
        DEBUGMSG(TRUE, (_T("ERROR: Unable to get NAND flash information.\r\n")));
        return(FALSE);
    }
    
    if(LogBlockAddr >= flashInfo.dwNumBlocks)
    {
        DEBUGMSG(TRUE, (_T("ERROR: LogBlockAddr exceed flashInfo.dwNumBlocks.\r\n")));
        return(FALSE);
    }
    
    for(i = 0; i < LogBlockAddr + 1;)
    {
        if(StartPhyBlockAddr >= flashInfo.dwNumBlocks)
        {
            return FALSE;   
        }
        //check block status
        if(FMD_GetBlockStatus(StartPhyBlockAddr) != BLOCK_STATUS_BAD)
        {
            i++;
            if(i == LogBlockAddr + 1)
            {
                *pGoodBlkAddr = StartPhyBlockAddr;
                return TRUE;
            }
        }
        else
        {
            DEBUGMSG(TRUE, (_T(" skip bad block 0x%x\r\n"),StartPhyBlockAddr));
        }
        StartPhyBlockAddr++;
    }

    return FALSE;
}


BOOL GetNextGoodBlock(DWORD StartPhyBlockAddr, DWORD *pNxtGoodBlkAddr)
{
    DWORD BlockStatus;
    FlashInfo flashInfo;
    
    if (!FMD_GetInfo(&flashInfo))
    {
        RETAILMSG(TRUE, (_T("ERROR: Unable to get NAND flash information.\r\n")));
        return FALSE;
    }
    
    do
    {
        //We start with next block of StartPhyBlockAddr
        StartPhyBlockAddr++;

        //check border validation
        if(StartPhyBlockAddr >= flashInfo.dwNumBlocks)
        return FALSE;

        //check block status
        BlockStatus = FMD_GetBlockStatus(StartPhyBlockAddr);

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
            if(FMD_GetBlockStatus(*pNxtGoodBlkAddr) != BLOCK_STATUS_BAD)
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

void CalculateParity_New(BYTE d, BYTE * p)
{
    BYTE Bit0  = (d & (1 << 0))  ? 1 : 0; 
    BYTE Bit1  = (d & (1 << 1))  ? 1 : 0;
    BYTE Bit2  = (d & (1 << 2))  ? 1 : 0;
    BYTE Bit3  = (d & (1 << 3))  ? 1 : 0;
    BYTE Bit4  = (d & (1 << 4))  ? 1 : 0;
    BYTE Bit5  = (d & (1 << 5))  ? 1 : 0;
    BYTE Bit6  = (d & (1 << 6))  ? 1 : 0;
    BYTE Bit7  = (d & (1 << 7))  ? 1 : 0;

    *p = 0;

    *p |= ((Bit6 ^ Bit5 ^ Bit3 ^ Bit2)               << 0);
    *p |= ((Bit7 ^ Bit5 ^ Bit4 ^ Bit2 ^ Bit1)        << 1);
    *p |= ((Bit7 ^ Bit6 ^ Bit5 ^ Bit1 ^ Bit0)        << 2);
    *p |= ((Bit7 ^ Bit4 ^ Bit3 ^ Bit0)               << 3);
    *p |= ((Bit6 ^ Bit4 ^ Bit3 ^ Bit2 ^ Bit1 ^ Bit0) << 4);
}

void CalculateHammingForNCB_New(unsigned char* pSector, BYTE *pOutBuffer, DWORD dwLen)
{
    DWORD i;
    BYTE *pui8SectorData = (BYTE*)pSector;
 
    for(i=0; i<dwLen; i++ )
    {
        BYTE NP=0;
 
        CalculateParity_New(pui8SectorData[i], &NP);
 
        pOutBuffer[i] = (NP & 0x1F);
    }
}

VOID NANDClearNCB(SBPosition Pos)
{
    DWORD i;
    
    UNREFERENCED_PARAMETER(Pos);
    
    for(i = CHIP_NCB_NAND_OFFSET; i < CHIP_NCB_SEARCH_RANGE; i++)
    {
        //FMD_EraseBlock(i);
		// CS&ZHL JUN-2-2012: set bad mark for
        if(!FMD_EraseBlock(i))
        {
			RETAILMSG(TRUE, (_T("ERROR: Unable to erase NAND block [0x%x]. Mark as bad block!\r\n"), i));        
            FMD_SetBlockStatus(i, BLOCK_STATUS_BAD);
            continue;
        }
        
        // Set reserved flag
        if(!FMD_SetBlockStatus(i, BLOCK_STATUS_RESERVED))
        {
			RETAILMSG(TRUE, (_T("ERROR: Unable to set flag to NAND block [0x%x]. Mark as bad block!\r\n"), i));        
            FMD_SetBlockStatus(i, BLOCK_STATUS_BAD);
        }
    }
}

UINT32 GetBCBChecksum(void * pBuffer, int u32Size)
{
    BYTE *pu8Buf = (BYTE *)pBuffer;
    UINT32 crc=0;
    int i;
    
    for(i=0;i < u32Size;i++)
    {
        crc += pu8Buf[i];
    }
    return (crc ^ 0xFFFFFFFF);
}

BOOL NANDWriteNCB(PFlashInfoExt pFlashInfo, SBPosition Pos)
{
    BootBlockStruct_t NCB;
    DWORD i, j;
    SectorInfo si;
    BYTE NCBNum = 0;
    DWORD startBlk1, startBlk2, endBlk2, bbtBlk;
    
    UNREFERENCED_PARAMETER(Pos);
    
    NCB.m_u32FingerPrint = 0x20424346;
    NCB.m_u32Version = 0x01000000;
    NCB.FCB_Block.m_NANDTiming.initializer = 0x60f0f0f;
    NCB.FCB_Block.m_u32DataPageSize = pFlashInfo->fi.wDataBytesPerSector;
    NCB.FCB_Block.m_u32TotalPageSize = pFlashInfo->fi.wDataBytesPerSector + pFlashInfo->SpareDataLength;
    NCB.FCB_Block.m_u32SectorsPerBlock = pFlashInfo->fi.wSectorsPerBlock;
    
    NCB.FCB_Block.m_u32NumberOfNANDs = pFlashInfo->NumberOfChip;
    NCB.FCB_Block.m_u32TotalInternalDie = 1;
    NCB.FCB_Block.m_u32CellType = 1;
    
    if(pFlashInfo->fi.wDataBytesPerSector == 2048)
    {
        NCB.FCB_Block.m_u32EccBlockNEccType = ECCN_2K64_PAGE;
    }
    else if(pFlashInfo->fi.wDataBytesPerSector == 4096 && pFlashInfo->SpareDataLength == 128)
    {
        NCB.FCB_Block.m_u32EccBlockNEccType = ECCN_4K128_PAGE;
    }
    else if(pFlashInfo->fi.wDataBytesPerSector == 4096 && pFlashInfo->SpareDataLength == 218)
    {
        NCB.FCB_Block.m_u32EccBlockNEccType = ECCN_4K218_PAGE;
    }
    else
    {
        ERRORMSG(1, (_T("Fatal Error!\r\n")));
        return FALSE;
    }
    NCB.FCB_Block.m_u32EccBlock0EccType = ECC0_2K4K_PAGE;     //!< Ecc level for Block 0 - BCH
    
    NCB.FCB_Block.m_u32EccBlock0Size = 0;         //!< Number of bytes for Block0 - BCH
    NCB.FCB_Block.m_u32EccBlockNSize = 512;         //!< Block size in bytes for all blocks other than Block0 - BCH
    NCB.FCB_Block.m_u32NumEccBlocksPerPage = pFlashInfo->fi.wDataBytesPerSector / 512;   //!< Number of blocks per page - BCH
    NCB.FCB_Block.m_u32MetadataBytes = METADATA_SIZE;         //!< Metadata size - BCH
    NCB.FCB_Block.m_u32EraseThreshold = NCB.FCB_Block.m_u32EccBlockNEccType;

    if(!GetGoodPhyBlock(IMAGE_BOOT_BOOTIMAGE_NAND_OFFSET, &startBlk1) || \
    !GetGoodPhyBlock(IMAGE_BOOT_BOOTIMAGE_NAND_OFFSET + IMAGE_BOOT_BOOTIMAGE_NAND_BLOCKS / 2, &startBlk2) || \
    !GetGoodPhyBlock(IMAGE_BOOT_BOOTIMAGE_NAND_OFFSET + IMAGE_BOOT_BOOTIMAGE_NAND_BLOCKS, &endBlk2) || \
    !GetGoodPhyBlock(IMAGE_BOOT_DBBT_NAND_OFFSET, &bbtBlk))
    {
        ERRORMSG(TRUE, (_T(" get good block failed\r\n")));
        return FALSE;
    }

    NCB.FCB_Block.m_u32Firmware1_startingSector = startBlk1 * pFlashInfo->fi.wSectorsPerBlock;   //!< Firmware image starts on this sector.
    NCB.FCB_Block.m_u32SectorsInFirmware1 = (startBlk2 - startBlk1) * pFlashInfo->fi.wSectorsPerBlock; //!< Number of sectors in firmware image.
    NCB.FCB_Block.m_u32Firmware2_startingSector = startBlk2 * pFlashInfo->fi.wSectorsPerBlock;  //!< Secondary FW Image starting Sector.
    NCB.FCB_Block.m_u32SectorsInFirmware2 = (endBlk2 - startBlk2) * pFlashInfo->fi.wSectorsPerBlock; //!< Number of sector in secondary FW image.
    NCB.FCB_Block.m_u32DBBTSearchAreaStartAddress = bbtBlk * pFlashInfo->fi.wSectorsPerBlock;
    
    if(pFlashInfo->fi.wDataBytesPerSector == 2048)
    {
        NCB.FCB_Block.m_u32BadBlockMarkerByte = BBMarkByteOffsetInPageData_2K64;
        NCB.FCB_Block.m_u32BadBlockMarkerStartBit = BBMarkBitOffset_2K64;
    }
    else if(pFlashInfo->fi.wDataBytesPerSector == 4096 && pFlashInfo->SpareDataLength == 128)
    {
        NCB.FCB_Block.m_u32BadBlockMarkerByte = BBMarkByteOffsetInPageData_4K128;
        NCB.FCB_Block.m_u32BadBlockMarkerStartBit = BBMarkBitOffset_4K128;
    }
    else if(pFlashInfo->fi.wDataBytesPerSector == 4096 && pFlashInfo->SpareDataLength == 218)
    {
        NCB.FCB_Block.m_u32BadBlockMarkerByte = BBMarkByteOffsetInPageData_4K218;
        NCB.FCB_Block.m_u32BadBlockMarkerStartBit = BBMarkBitOffset_4K218;
    }
    
    NCB.FCB_Block.m_u32BBMarkerPhysicalOffset = pFlashInfo->fi.wDataBytesPerSector;
    
    memcpy(sectorBuf + NAND_HC_ECC_OFFSET_DATA_COPY, &NCB, sizeof(NCB));
    NCB.m_u32Checksum = GetBCBChecksum(sectorBuf + NAND_HC_ECC_OFFSET_DATA_COPY + 4, 512 - 4);
    memcpy(sectorBuf + NAND_HC_ECC_OFFSET_DATA_COPY, &NCB, sizeof(NCB));
    
    CalculateHammingForNCB_New((unsigned char *)sectorBuf + NAND_HC_ECC_OFFSET_DATA_COPY, sectorBuf + NAND_HC_ECC_OFFSET_PARITY_COPY, NAND_HC_ECC_SIZEOF_DATA_BLOCK_IN_BYTES);
    
    memset(&si, 0xff, sizeof(SectorInfo));
    si.bOEMReserved = (BYTE)(~OEM_BLOCK_RESERVED);
    
	//-----------------------------------------------------------------
	// mx28_nandlayout.h:
	// CHIP_NCB_NAND_OFFSET = 0, 
	// CHIP_NCB_SEARCH_RANGE = 4
	// CHIP_BCB_SECTOR_STRIDE = 64
	//-----------------------------------------------------------------
    for(i = CHIP_NCB_NAND_OFFSET; i < CHIP_NCB_SEARCH_RANGE; i++)
    {
        //if(!FMD_EraseBlock(i))
        //{
        //    ERRORMSG(TRUE, (_T("erase block %d failed, set to bad\r\n"),i));
        //    FMD_SetBlockStatus(i, BLOCK_STATUS_BAD);
        //}
        //else
		//
        // CS&ZHL JUN-2-2012: Skip bad blocks
		//
        if(FMD_GetBlockStatus(i) & BLOCK_STATUS_RESERVED)
        {
            for(j = 0; j < pFlashInfo->fi.wSectorsPerBlock; j += CHIP_BCB_SECTOR_STRIDE)
            {
                if(!FMD_WriteSector(i * pFlashInfo->fi.wSectorsPerBlock + j, sectorBuf, &si, 1))
                {
                    ERRORMSG(TRUE, (_T("failed write sector 0x%x\r\n"), i * pFlashInfo->fi.wSectorsPerBlock));   
                }
                else
                {
                    NCBNum++;
                }
            }
        }
    }
    
    return NCBNum > 0;
}

BOOL NANDWriteDBBT(PFlashInfoExt pFlashInfo, SBPosition Pos)
{
    BootBlockStruct_t NCB;
    DWORD badBlockNum = 0;
    WORD  badBlockBuf[CHIP_BAD_BLOCK_RANGE], i;
    SectorInfo si;
    DWORD bbtBlk;
    
    UNREFERENCED_PARAMETER(Pos);
    
    memset(&NCB, 0, sizeof(NCB));
    memset(badBlockBuf, 0xff, sizeof(badBlockBuf));
    
    for(i = 0; i < CHIP_BAD_BLOCK_RANGE; i++)
    {
        if(FMD_GetBlockStatus(i) & BLOCK_STATUS_BAD)
        {
            badBlockBuf[badBlockNum] = i;
            badBlockNum++;
        }
    }
    
    NCB.m_u32FingerPrint = 0x54424244;
    NCB.m_u32Version = 0x01000000;
    
    NCB.DBBT_Block.m_u32NumberBB = badBlockNum;
    NCB.DBBT_Block.m_u32Number2KPagesBB = 1;
    
    if(!GetGoodPhyBlock(IMAGE_BOOT_DBBT_NAND_OFFSET, &bbtBlk))
    {
        ERRORMSG(TRUE, (_T(" get good block failed\r\n")));
        return FALSE;
    }
    
    memset(sectorBuf, 0, NANDFC_BOOT_SIZE);
    memcpy(sectorBuf, &NCB, sizeof(NCB));
    memset(&si, 0xff, sizeof(SectorInfo));
    si.bOEMReserved = (BYTE)~OEM_BLOCK_RESERVED;
    
    if(!FMD_EraseBlock(bbtBlk))
    {
        ERRORMSG(TRUE, (_T("erase block %d failed, set to bad\r\n"),i));
        FMD_SetBlockStatus(bbtBlk, BLOCK_STATUS_BAD);
        return FALSE;
    }
    else
    {
        if(!FMD_WriteSector(bbtBlk * pFlashInfo->fi.wSectorsPerBlock, sectorBuf, &si, 1))
        {
            ERRORMSG(TRUE, (_T("failed write sector 0x%x\r\n"), i * pFlashInfo->fi.wSectorsPerBlock));  
            FMD_EraseBlock(bbtBlk);
            FMD_SetBlockStatus(bbtBlk, BLOCK_STATUS_BAD);
            return FALSE; 
        }
    }
    memset(sectorBuf, 0xff, NANDFC_BOOT_SIZE);
    
    *(DWORD*)sectorBuf = 0;
    *((DWORD*)sectorBuf + 1) = badBlockNum;  
        
    memcpy((DWORD*)sectorBuf + 2, badBlockBuf, sizeof(badBlockBuf));
    if(!FMD_WriteSector(bbtBlk * pFlashInfo->fi.wSectorsPerBlock + CHIP_BBT_SECTOR_OFFSET, sectorBuf, &si, 1))
    {
        ERRORMSG(TRUE, (_T("failed write sector 0x%x\r\n"), i * pFlashInfo->fi.wSectorsPerBlock));  
        FMD_EraseBlock(bbtBlk);
        FMD_SetBlockStatus(bbtBlk, BLOCK_STATUS_BAD);
        return FALSE; 
    }
    
    return TRUE;
}

VOID NANDGetImgInfo(PNANDImgInfo pInfo)
{
    FlashInfo flashInfo;

    FMD_GetInfo(&flashInfo);
    
    pInfo->dwSBLen = IMAGE_BOOT_BOOTIMAGE_NAND_BLOCKS * flashInfo.dwBytesPerBlock / 2;
    pInfo->dwSBLenUnit = flashInfo.dwBytesPerBlock;
    pInfo->dwNB0Len = NANDImageCfg.dwNkSize;
    pInfo->dwNB0LenUnit = flashInfo.dwBytesPerBlock;
}

//-----------------------------------------------------------------------------
//
//  Function:  NANDReadSB
//
//  This function reads the Boot image stored in NAND flash memory to RAM
//
//  Parameters:
//      pRequest 
//          [in] transfer infomation, including SB position and index
//
//      dwLength 
//          [in] Length of the RAM, in bytes, to be read from flash
//          memory.            
//      Pos
//          [in] specify which boot stream caller wants to read.
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//-----------------------------------------------------------------------------
BOOL NANDReadSB(PNANDIORequest pRequest, LPBYTE pImage, DWORD dwLength)
{
    FlashInfo flashInfo;
    SectorInfo sectorInfo;
    SECTOR_ADDR sectorAddr, startSectorAddr, endSectorAddr;
    DWORD StartLogBlkAddr,EndLogBlkAddr,PhyBlockAddr;
    PBYTE pSectorBuf, pBuf;
    DWORD dwReadSectorLen;
    
    if(!FMD_GetInfo(&flashInfo))
    {
        RETAILMSG(TRUE, (_T("ERROR: Unable to get NAND flash information.\r\n")));
        return FALSE;
    }
    dwReadSectorLen = flashInfo.wDataBytesPerSector;
    pSectorBuf = (PBYTE)LocalAlloc(LPTR, flashInfo.wDataBytesPerSector);
	// CS&ZHL MAR-21-2012: use local buffer to avoid using functions of coredll
	//pSectorBuf = g_pSectorBuffer;
    if(!pSectorBuf)
    {
        ERRORMSG(TRUE, (_T("alloc memory for pSectorBuf failed\r\n")));   
    }
    
    // Check parameters
    if(dwLength < g_ImgInfo.dwSBLenUnit)
    {
        ERRORMSG(TRUE, (_T("length size(0x%x) is smaller than expected (0x%x)\r\n"), dwLength, g_ImgInfo.dwSBLenUnit));
        return FALSE;
    }
    if(pRequest->dwIndex > g_ImgInfo.dwSBLen / g_ImgInfo.dwSBLenUnit)
    {
        ERRORMSG(TRUE, (_T("dwIndex (0x%x) exceeds expected (0x%x)\r\n"), pRequest->dwIndex, g_ImgInfo.dwSBLen / g_ImgInfo.dwSBLenUnit));
        return FALSE;
    }
    
    // Fill the sectorInfo
    memset(&sectorInfo, 0xFF, sizeof(sectorInfo));

    // Calculate block range for the EBOOT image
    StartLogBlkAddr = IMAGE_BOOT_BOOTIMAGE_NAND_OFFSET;     
    EndLogBlkAddr = IMAGE_BOOT_BOOTIMAGE_NAND_OFFSET + IMAGE_BOOT_BOOTIMAGE_NAND_BLOCKS / 2;

    if(pRequest->SBPos == SecondBoot)
    {
        StartLogBlkAddr = IMAGE_BOOT_BOOTIMAGE_NAND_OFFSET + IMAGE_BOOT_BOOTIMAGE_NAND_BLOCKS / 2;
        EndLogBlkAddr = IMAGE_BOOT_BOOTIMAGE_NAND_OFFSET + IMAGE_BOOT_BOOTIMAGE_NAND_BLOCKS;
    }
    StartLogBlkAddr += pRequest->dwIndex;
    
    DEBUGMSG(TRUE, (_T("INFO: reading NAND flash blocks 0x%x\r\n"), StartLogBlkAddr));
    
    if(!GetGoodPhyBlock(StartLogBlkAddr, &PhyBlockAddr))
    {
        ERRORMSG(TRUE, (_T("GetGoodPhyBlock failed: 0x%x\r\n"),StartLogBlkAddr));  
        return FALSE; 
    }
    // Compute sector address based on current physical block
    startSectorAddr = PhyBlockAddr * flashInfo.wSectorsPerBlock;
    endSectorAddr = startSectorAddr + flashInfo.wSectorsPerBlock;
    
    for (sectorAddr = startSectorAddr; sectorAddr < endSectorAddr; sectorAddr++)
    {
        if(dwLength < dwReadSectorLen)
        {
            pBuf = pSectorBuf;
        }
        else
        {
            pBuf = pImage;
        }
    
        if (!FMD_ReadSector(sectorAddr, pBuf, &sectorInfo, 1))
        {
            RETAILMSG(TRUE, (_T("ERROR: Failed to verify image.\r\n")));
            return FALSE;
        }
        
        if(dwLength < dwReadSectorLen)
        {
            memcpy(pImage, pSectorBuf, dwLength);
        }
        pImage += flashInfo.wDataBytesPerSector;      
        dwLength -= flashInfo.wDataBytesPerSector;
    }
    
    LocalFree(pSectorBuf);
    
    return(TRUE);
}
//-----------------------------------------------------------------------------
//
//  Function:  NANDWriteSB
//
//  This function writes to NAND flash memory the Boot image stored 
//  in the RAM file cache area.
//
//  Parameters:
//      pRequest 
//          [in] transfer infomation, including SB position and index
//
//      dwLength 
//          [in] Length of the Boot image, in bytes, to be written to flash
//          memory.            
//      Pos
//          [in] specify which boot stream caller wants to write.
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//-----------------------------------------------------------------------------
BOOL NANDWriteSB(PNANDIORequest pRequest, LPBYTE pImage, DWORD dwLength)
{
    FlashInfo flashInfo;
    LPBYTE pBuf, pVerSectorBuf,pExtraBuf;
    SectorInfo sectorInfo,VersectorInfo;
    SECTOR_ADDR sectorAddr, startSectorAddr, endSectorAddr;
    DWORD StartLogBlkAddr,PhyBlockAddr;
    DWORD dwPhySectorSize;
    LPBYTE pOriginalImage = pImage;
    DWORD dwOriginalLength = dwLength;

    //RETAILMSG(TRUE, (_T("->NANDWriteSB\r\n")));
    if(!FMD_GetInfo(&flashInfo))
    {
        RETAILMSG(TRUE, (_T("ERROR: Unable to get NAND flash information.\r\n")));
        return FALSE;
    }
    
    dwPhySectorSize = flashInfo.wDataBytesPerSector;
    pExtraBuf = (PBYTE) LocalAlloc(LPTR, dwPhySectorSize); 
    if(pExtraBuf == NULL){
        ERRORMSG(TRUE, (_T("Allocate memory buffer with size of 0x%x failed!\r\n"),flashInfo.dwBytesPerBlock+dwPhySectorSize));
        return FALSE;
    }
        
    // Check parameters
    if(dwLength < g_ImgInfo.dwSBLenUnit)
    {
        ERRORMSG(TRUE, (_T("length size(0x%x) is smaller than expected (0x%x)\r\n"), dwLength, g_ImgInfo.dwSBLenUnit));
        return FALSE;
    }
    if(pRequest->dwIndex > g_ImgInfo.dwSBLen / g_ImgInfo.dwSBLenUnit)
    {
        ERRORMSG(TRUE, (_T("dwIndex (0x%x) exceeds expected (0x%x)\r\n"), pRequest->dwIndex, g_ImgInfo.dwSBLen / g_ImgInfo.dwSBLenUnit));
        return FALSE;
    }
    
    memset(pExtraBuf, 0xFF, dwPhySectorSize);    

    // Write sb to NAND flash
    pVerSectorBuf = (PBYTE) LocalAlloc(LPTR, dwPhySectorSize);
    if (pVerSectorBuf == NULL) {
        ERRORMSG(1, (_T("Failed to alloc enough memory space! Memory required: 0x%x bytes.\r\n"), flashInfo.wDataBytesPerSector));    
        return FALSE;
    }    
    
    // Fill the sectorInfo
    memset(&sectorInfo, 0xFF, sizeof(sectorInfo));

    // Set Reserved and image flag
    sectorInfo.bOEMReserved &= ~OEM_BLOCK_RESERVED;   
        
    StartLogBlkAddr = IMAGE_BOOT_BOOTIMAGE_NAND_OFFSET;     
    
    if(pRequest->SBPos == SecondBoot)
    {
        StartLogBlkAddr = IMAGE_BOOT_BOOTIMAGE_NAND_OFFSET + IMAGE_BOOT_BOOTIMAGE_NAND_BLOCKS / 2;
    }

	// CS&ZHL JUN-2-2012: should clear NCB before programming SB image
    if(pRequest->dwIndex == 0)
    {
		RETAILMSG(TRUE, (_T("NANDWriteSB: clear NCB before programming SB image\r\n")));
        NANDClearNCB(pRequest->SBPos);   
    }

begin_program:    
    StartLogBlkAddr += pRequest->dwIndex;
    
    //RETAILMSG(TRUE, (_T("NANDWriteSB: Programming logical blocks[0x%x]"), StartLogBlkAddr));
    
    //if(pRequest->dwIndex == 0)
    //{
    //    NANDClearNCB(pRequest->SBPos);   
    //}
retry:
    if(!GetGoodPhyBlock(StartLogBlkAddr, &PhyBlockAddr))
    {
        ERRORMSG(TRUE, (_T("\r\nGetGoodPhyBlock failed: 0x%x\r\n"),StartLogBlkAddr));  
        return FALSE; 
    }
    
    // Erase the block...
    if (!FMD_EraseBlock(PhyBlockAddr))
    {
        FMD_SetBlockStatus(PhyBlockAddr, BLOCK_STATUS_BAD);
        if(!(FMD_GetBlockStatus(PhyBlockAddr) & BLOCK_STATUS_BAD))
        {
            RETAILMSG(TRUE, (_T("\r\nNANDWriteSB: erase block[0x%x] failed, exit!!\r\n"), PhyBlockAddr));
            return FALSE;
        }
        else
        {
            goto retry;
        }
    }
    //RETAILMSG(TRUE, (_T(" -> physical blocks[0x%x]\r\n"), PhyBlockAddr));

    // Compute sector address based on current physical block
    startSectorAddr = PhyBlockAddr * flashInfo.wSectorsPerBlock;
    endSectorAddr = startSectorAddr + flashInfo.wSectorsPerBlock;

    for (sectorAddr = startSectorAddr; sectorAddr < endSectorAddr; sectorAddr++)
    {
        if(dwLength < dwPhySectorSize)
        {
            pBuf = pExtraBuf;
            memcpy(pBuf, pImage, dwLength);
        }
        else
        {
            pBuf = pImage;
        }
        
        if (!FMD_WriteSector(sectorAddr, pBuf, &sectorInfo, 1))
        {
            RETAILMSG(TRUE, (_T("ERROR: Failed to update EBOOT/SBOOT.\r\n")));
            return FALSE;
        }

        if (!FMD_ReadSector(sectorAddr, pVerSectorBuf, &VersectorInfo, 1))
        {
            RETAILMSG(TRUE, (_T("ERROR: Failed to verify image.\r\n")));
            return FALSE;
        }

        if (memcmp(pBuf, pVerSectorBuf, flashInfo.wDataBytesPerSector) != 0)
        {
            RETAILMSG(TRUE, (_T("ERROR: Failed to verify image.\r\n")));
            return FALSE;
        }
        
        pImage += flashInfo.wDataBytesPerSector; 
        dwLength -= flashInfo.wDataBytesPerSector;
    }
    
    if(pRequest->SBPos == BothBoot && StartLogBlkAddr < IMAGE_BOOT_BOOTIMAGE_NAND_OFFSET + IMAGE_BOOT_BOOTIMAGE_NAND_BLOCKS / 2)
    {
        StartLogBlkAddr = IMAGE_BOOT_BOOTIMAGE_NAND_OFFSET + IMAGE_BOOT_BOOTIMAGE_NAND_BLOCKS / 2;
        pImage = pOriginalImage;
        dwLength = dwOriginalLength;
        goto begin_program;   
    }
    
    if(pExtraBuf)
    {
        LocalFree(pExtraBuf);
    }   
    if(pVerSectorBuf)
    { 
        LocalFree(pVerSectorBuf);
    }
    //RETAILMSG(TRUE, (_T("<-NANDWriteSB\r\n")));
    return(TRUE);
}

BOOL NANDEndWriteSB(PNANDIORequest pRequest)
{
    if(!NANDWriteNCB(&g_CSPfi, pRequest->SBPos) || !NANDWriteDBBT(&g_CSPfi, pRequest->SBPos))
    {
        ERRORMSG(TRUE, (_T("write NCB&LDLB failed\r\n")));
        return FALSE;
    }
   
    NANDBootReserved();
    return(TRUE);
}
//-----------------------------------------------------------------------------
//
//  Function:  NANDWriteNK
//
//  This function writes to NAND flash memory the OS image stored 
//  in the RAM file cache area.
//
//  Parameters:
//      dwStartAddr 
//          [in] Address in flash memory where the start of the downloaded 
//          OS image is to be written.
//
//      dwLength 
//          [in] Length of the OS image, in bytes, to be written to flash
//          memory.            
//
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//-----------------------------------------------------------------------------
BOOL NANDWriteNK(DWORD dwIndex, LPBYTE pImage, DWORD dwLength)
{
    FlashInfo flashInfo;
    LPBYTE pSectorBuf, pVerSectorBuf, pVerifyBuf;
    SectorInfo sectorInfo,VersectorInfo;
    SECTOR_ADDR sectorAddr, startSectorAddr, endSectorAddr;
    DWORD StartLogBlkAddr,PhyBlockAddr;
    
    // Boot Configuration and Buffer
    if(!NANDBootInit())
    {
        RETAILMSG(TRUE, (_T("WARNING: NANDBootInit fail\r\n")));
        return FALSE;
    }

    if(!FMD_GetInfo(&flashInfo))
    {
        RETAILMSG(TRUE, (_T("ERROR: Unable to get NAND flash information.\r\n")));
        return FALSE;
    }

    // Check parameters
    if(dwLength < g_ImgInfo.dwNB0LenUnit)
    {
        ERRORMSG(TRUE, (_T("length size(0x%x) is smaller than expected (0x%x)\r\n"), dwLength, g_ImgInfo.dwNB0LenUnit));
        return FALSE;
    }
    if(dwIndex > g_ImgInfo.dwNB0Len / g_ImgInfo.dwNB0LenUnit)
    {
        ERRORMSG(TRUE, (_T("dwIndex (0x%x) exceeds expected (0x%x)\r\n"), dwIndex, g_ImgInfo.dwNB0Len / g_ImgInfo.dwNB0LenUnit));
        return FALSE;
    }
    
    // Write EBOOT to NAND flash
    pSectorBuf = pImage;    
    pVerSectorBuf = pVerifyBuf = (PBYTE) LocalAlloc(LPTR, flashInfo.dwBytesPerBlock);
    if (!pVerifyBuf) {
        ERRORMSG(1, (_T("Failed to alloc enough memory space!\r\n"))); 
        goto exit;   
    }

    // Fill the sectorInfo
    memset(&sectorInfo, 0xFF, sizeof(sectorInfo));

    // Set Reserved and image flag
    sectorInfo.bOEMReserved &= ~OEM_BLOCK_RESERVED;   
    sectorInfo.bOEMReserved &= ~OEM_BLOCK_READONLY;

    // Calculate block range for the NK image
    StartLogBlkAddr = NANDImageCfg.dwNkOffset / flashInfo.dwBytesPerBlock + dwIndex;
    
    DEBUGMSG(TRUE, (_T("INFO: Programming NAND flash blocks [0x%x].\r\n"), StartLogBlkAddr));        

retry:
    if(!GetPhyBlkAddr(StartLogBlkAddr, &PhyBlockAddr))
    {
        RETAILMSG(TRUE, (_T("Error: No good block found - unable to store image!\r\n")));
        goto exit;
    }

    // Erase the block...
    if (!FMD_EraseBlock(PhyBlockAddr))
    {
        FMD_SetBlockStatus(PhyBlockAddr, BLOCK_STATUS_BAD);
        if(!(FMD_GetBlockStatus(PhyBlockAddr) & BLOCK_STATUS_BAD))
        {
            RETAILMSG(TRUE, (_T("ERROR: Unable to erase NAND flash block [0x%x].\r\n"), PhyBlockAddr));
            goto exit;
        }
        else
        {
            goto retry;
        }
    }
    
    // Compute sector address based on current physical block
    startSectorAddr = PhyBlockAddr * flashInfo.wSectorsPerBlock;
    endSectorAddr = startSectorAddr + flashInfo.wSectorsPerBlock;
    
    for (sectorAddr = startSectorAddr; sectorAddr < endSectorAddr; sectorAddr++)
    {
        if (!FMD_WriteSector(sectorAddr, pSectorBuf, &sectorInfo, 1))
        {
            RETAILMSG(TRUE, (_T("ERROR: Failed to update image.\r\n")));
            goto exit;
        }

        pSectorBuf += flashInfo.wDataBytesPerSector;
    }
    
    // Compute sector address based on current physical block
    startSectorAddr = PhyBlockAddr * flashInfo.wSectorsPerBlock;
    endSectorAddr = startSectorAddr + flashInfo.wSectorsPerBlock;    
    for (sectorAddr = startSectorAddr; sectorAddr < endSectorAddr; sectorAddr++)
    {
        if (!FMD_ReadSector(sectorAddr, pVerSectorBuf, &VersectorInfo, 1))
        {
            RETAILMSG(TRUE, (_T("ERROR: Failed to verify image.\r\n")));
            goto exit;
        }
        pVerSectorBuf += flashInfo.wDataBytesPerSector;
    }
    pVerSectorBuf = pVerifyBuf;

    if (memcmp(pSectorBuf - flashInfo.dwBytesPerBlock, pVerSectorBuf, flashInfo.dwBytesPerBlock) != 0)
    {
        RETAILMSG(TRUE, (_T("ERROR: Failed to verify image.\r\n")));
        goto exit;
    }
    
exit:
    if(pVerifyBuf)
    {  
        LocalFree(pVerifyBuf);
    }
    return(TRUE);

}


//-----------------------------------------------------------------------------
//
//  Function:  NANDWriteImage
//
//  This function writes to NAND flash memory the Boot image .
//
//  Parameters:
//      pNANDWrtImgInfo 
//          [in] Image information contains image type, nand address to be written, etc.
//      pImage
//          [in] buffer which contains image to be written.          
//      dwLength 
//          [in] Length of the Boot image, in bytes, to be written to flash
//          memory.            
//
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//-----------------------------------------------------------------------------
BOOL NANDWriteImage(PNANDWrtImgInfo pNANDWrtImgInfo, LPBYTE pImage, DWORD dwLength)
{
    FlashInfo	flashInfo;
    SectorInfo	sectorInfo;
    SECTOR_ADDR sectorAddr, startSectorAddr, endSectorAddr;
    DWORD		StartLogBlkAddr, PhyBlockAddr, dwStartOffset;
    LPBYTE		pSectorBuf, pVerifySectorBuf;
                
    // Boot Configuration and Buffer
    if(!NANDBootInit())
    {
        RETAILMSG(TRUE, (_T("WARNING: NANDBootInit fail\r\n")));
        return FALSE;
    }

    if(!FMD_GetInfo(&flashInfo))
    {
        RETAILMSG(TRUE, (_T("ERROR: Unable to get NAND flash information.\r\n")));
        return FALSE;
    }

    // Check parameters
    if(dwLength > flashInfo.dwBytesPerBlock)
    {
        ERRORMSG(TRUE, (_T("length size(0x%x) is differen with expected (0x%x)\r\n"), dwLength, flashInfo.dwBytesPerBlock));
        return FALSE;
    }

    if(pNANDWrtImgInfo->dwImgType == IMAGE_NK)
    {
        dwStartOffset = NANDImageCfg.dwNkOffset;
        if(!pNANDWrtImgInfo->dwIndex)
        {
            RETAILMSG(TRUE, (_T("INFO: Writing NK image to NAND (please wait)...\r\n")));
        }
    }
    else if(pNANDWrtImgInfo->dwImgType == IMAGE_EBOOT)
    {
        dwStartOffset = NANDImageCfg.dwBootOffset;
        if(!pNANDWrtImgInfo->dwIndex)
        {
            RETAILMSG(TRUE, (_T("INFO: Writing EBOOT image to NAND (please wait)...\r\n")));
        }        
    }                
    else if(pNANDWrtImgInfo->dwImgType == IMAGE_EBOOTCFG)
    {
        dwStartOffset = NANDImageCfg.dwCfgOffset;
        if(!pNANDWrtImgInfo->dwIndex)
        {
            //RETAILMSG(TRUE, (_T("INFO: Writing EbootCFG to NAND 0x%x (please wait)...\r\n"), dwStartOffset));
            RETAILMSG(TRUE, (_T("INFO: Writing EbootCFG to NAND (please wait)...\r\n") ));
        }        
    }                
    else if(pNANDWrtImgInfo->dwImgType == IMAGE_SPLASH)
    {
        dwStartOffset = NANDImageCfg.dwSplashOffset;
        if(!pNANDWrtImgInfo->dwIndex)
        {
            RETAILMSG(TRUE, (_T("INFO: Writing SplashScreen image to NAND (please wait)...\r\n")));
        }        
    }                
	//
	// CS&ZHL MAR-30-2012: add Image Type for MBR
	//
	else if(pNANDWrtImgInfo->dwImgType == IMAGE_MBR)
    {
        dwStartOffset = NANDImageCfg.dwMBROffset;
        if(!pNANDWrtImgInfo->dwIndex)
        {
            RETAILMSG(TRUE, (_T("INFO: Writing MBR image to NAND (please wait)...\r\n") ));
            //RETAILMSG(TRUE, (_T("INFO: MBR 0x%x 0x%x 0x%x 0x%x 0x%x\r\n"), pImage[0], pImage[1], pImage[2], pImage[510], pImage[511]));
        }        
    }                
	//
	// CS&ZHL APR-12-2012: add Image Type for both VID and UID
	//
	else if(pNANDWrtImgInfo->dwImgType == IMAGE_VID)
    {
        dwStartOffset = NANDImageCfg.dwVIDOffset;
        if(!pNANDWrtImgInfo->dwIndex)
        {
            RETAILMSG(TRUE, (_T("INFO: Writing VendorID image to NAND (please wait)...\r\n") ));
        }        
    }                
	else if(pNANDWrtImgInfo->dwImgType == IMAGE_UID)
    {
        dwStartOffset = NANDImageCfg.dwUIDOffset;
        if(!pNANDWrtImgInfo->dwIndex)
        {
            RETAILMSG(TRUE, (_T("INFO: Writing UserID image to NAND (please wait)...\r\n") ));
        }        
    }                
    else
    {
        dwStartOffset = NANDImageCfg.dwXldrOffset;
        if(!pNANDWrtImgInfo->dwIndex)
        {
            RETAILMSG(TRUE, (_T("INFO: Writing XLDR image to NAND (please wait)...\r\n")));
        }        
    }

	// CS&ZHL JAN-9-2012: setup init parameters
    pSectorBuf = pImage;    
    pVerifySectorBuf = sectorBuf;

	// Fill the sectorInfo
    memset(&sectorInfo, 0xFF, sizeof(sectorInfo));

    // Set Reserved and image flag
    sectorInfo.bOEMReserved &= ~OEM_BLOCK_RESERVED;   
    sectorInfo.bOEMReserved &= ~OEM_BLOCK_READONLY;

    StartLogBlkAddr = dwStartOffset / flashInfo.dwBytesPerBlock + pNANDWrtImgInfo->dwIndex; 

    
	//RETAILMSG(TRUE, (_T("NANDWriteImage: Programming logical blocks[0x%x]"), StartLogBlkAddr));
retry:
    if(!GetPhyBlkAddr(StartLogBlkAddr, &PhyBlockAddr))
    {
        RETAILMSG(TRUE, (_T("\r\nNANDWriteImage: No good block found, exit!\r\n")));
        return FALSE; 
    }

    // Erase the block...
    if (!FMD_EraseBlock(PhyBlockAddr))
    {
        FMD_SetBlockStatus(PhyBlockAddr, BLOCK_STATUS_BAD);
        if(!(FMD_GetBlockStatus(PhyBlockAddr) & BLOCK_STATUS_BAD))
        {
            RETAILMSG(TRUE, (_T("\r\nNANDWriteImage: erase physical block[0x%x] failed, exit!\r\n"), PhyBlockAddr));
			return FALSE; 
        }
        else
        {
            goto retry;
        }
    }
    
	// show physical block index
	//switch(pNANDWrtImgInfo->dwImgType)
	//{
	//case IMAGE_EBOOTCFG:
 //       RETAILMSG(TRUE, (_T("-> IMAGE_EBOOTCFG -> PhyBlock[0x%x]\r\n"), PhyBlockAddr));
	//	break;

	//case IMAGE_SPLASH:
 //       RETAILMSG(TRUE, (_T("-> IMAGE_SPLASH %d -> PhyBlock[0x%x]\r\n"), pNANDWrtImgInfo->dwIndex, PhyBlockAddr));
	//	break;

	//case IMAGE_MBR:
 //       RETAILMSG(TRUE, (_T("-> IMAGE_MBR -> PhyBlock[0x%x]\r\n"), PhyBlockAddr));
	//	break;

	//case IMAGE_VID:
 //       RETAILMSG(TRUE, (_T("-> IMAGE_VID -> PhyBlock[0x%x]\r\n"), PhyBlockAddr));
	//	break;

	//case IMAGE_UID:
 //       RETAILMSG(TRUE, (_T("-> IMAGE_UID -> PhyBlock[0x%x]\r\n"), PhyBlockAddr));
	//	break;
	//}

    // Compute sector address based on current physical block
    startSectorAddr = PhyBlockAddr * flashInfo.wSectorsPerBlock;
    endSectorAddr = startSectorAddr + (dwLength / flashInfo.wDataBytesPerSector);		//flashInfo.wSectorsPerBlock;
	if(dwLength % flashInfo.wDataBytesPerSector)
	{
		endSectorAddr++;
	}
    
    for (sectorAddr = startSectorAddr; sectorAddr < endSectorAddr; sectorAddr++)
    {
        if (!FMD_WriteSector(sectorAddr, pSectorBuf, &sectorInfo, 1))
        {
            RETAILMSG(TRUE, (_T("ERROR: Failed to write @0x%x into the sectorAddr 0x%x.\r\n"), pSectorBuf, sectorAddr));
			return FALSE; 
        }

        if(!FMD_ReadSector(sectorAddr, pVerifySectorBuf, &sectorInfo, 1))
        {
            RETAILMSG(TRUE, (_T("ERROR: Failed to read @0x%x from the sectorAddr 0x%x.\r\n"), pVerifySectorBuf, sectorAddr));
            return FALSE;
        }
        
        if(memcmp(pVerifySectorBuf, pSectorBuf, flashInfo.wDataBytesPerSector) != 0)
        {
            RETAILMSG(TRUE, (_T("ERROR: Failed to verify IMAGE_TYPE = %d.\r\n"), pNANDWrtImgInfo->dwImgType));
            return FALSE;
        } 

		pSectorBuf += flashInfo.wDataBytesPerSector;
    }

    return(TRUE);
}

//----------------------------------------------------------------------------------------
//
//  CS&ZHL APR-10-2012: -> NANDReadImage
//
//  This function reads some boot images from NAND flash memory.
//
//  Parameters:
//      pNANDWrtImgInfo 
//          [in] Image information contains image type, nand address to be written, etc.
//      pImage
//          [in] buffer which contains image to be read.
//      dwLength 
//          [in] Length of the Boot image, in bytes, to be read from flash
//          memory.            
//
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//----------------------------------------------------------------------------------------
BOOL NANDReadImage(PNANDWrtImgInfo pNANDWrtImgInfo, LPBYTE pImage, DWORD dwLength)
{
    FlashInfo	flashInfo;
    SECTOR_ADDR sectorAddr, startSectorAddr, endSectorAddr;
    DWORD		StartLogBlkAddr, PhyBlockAddr, dwStartOffset;
	LPBYTE		pDestSectorBuf;		// destination buffer
    LPBYTE		pSectorBuf;			// temp bffer
                
    // Boot Configuration and Buffer
    if(!NANDBootInit())
    {
        RETAILMSG(TRUE, (_T("WARNING: NANDBootInit fail\r\n")));
        return FALSE;
    }

    if(!FMD_GetInfo(&flashInfo))
    {
        RETAILMSG(TRUE, (_T("ERROR: Unable to get NAND flash information.\r\n")));
        return FALSE;
    }

    // Check parameters
    if(dwLength > flashInfo.dwBytesPerBlock)
    {
        ERRORMSG(TRUE, (_T("length size(0x%x) is differen with expected (0x%x)\r\n"), dwLength, flashInfo.dwBytesPerBlock));
        return FALSE;
    }

	switch(pNANDWrtImgInfo->dwImgType)
	{
	case IMAGE_EBOOTCFG:
        dwStartOffset = NANDImageCfg.dwCfgOffset;
		break;

	case IMAGE_VID:
        dwStartOffset = NANDImageCfg.dwVIDOffset;
        RETAILMSG(TRUE, (_T("INFO: reading vendor info (please wait)\r\n")));
		break;

	case IMAGE_UID:
        dwStartOffset = NANDImageCfg.dwUIDOffset;
        RETAILMSG(TRUE, (_T("INFO: reading user info (please wait)\r\n")));
		break;

	default:
        RETAILMSG(TRUE, (_T("ERROR: NOT supported image type.\r\n")));
        return FALSE;
	}

	// CS&ZHL JAN-9-2012: setup init parameters
    pDestSectorBuf = pImage;    
    pSectorBuf = sectorBuf;

	// compute logical block number
    StartLogBlkAddr = dwStartOffset / flashInfo.dwBytesPerBlock + pNANDWrtImgInfo->dwIndex; 
    if(!GetPhyBlkAddr(StartLogBlkAddr, &PhyBlockAddr))
    {
        RETAILMSG(TRUE, (_T("Error: No good block found - unable to store image!\r\n")));
        return FALSE; 
    }

    // Compute sector address based on current physical block
    startSectorAddr = PhyBlockAddr * flashInfo.wSectorsPerBlock;
    endSectorAddr = startSectorAddr + (dwLength / flashInfo.wDataBytesPerSector);		//flashInfo.wSectorsPerBlock;
	if(dwLength % flashInfo.wDataBytesPerSector)
	{
		endSectorAddr++;
	}
    
    for (sectorAddr = startSectorAddr; sectorAddr < endSectorAddr; sectorAddr++)
    {
        if (!FMD_ReadSector(sectorAddr, pSectorBuf, NULL, 1))
        {
            RETAILMSG(TRUE, (_T("ERROR: read sectorAddr 0x%x failed.\r\n"), sectorAddr));
			return FALSE; 
        }

		if(dwLength > flashInfo.wDataBytesPerSector)
		{
			memcpy(pDestSectorBuf, pSectorBuf, flashInfo.wDataBytesPerSector);
			dwLength -= flashInfo.wDataBytesPerSector;
			pDestSectorBuf += flashInfo.wDataBytesPerSector;
		}
		else
		{
			memcpy(pDestSectorBuf, pSectorBuf, dwLength);
			dwLength = 0;
			// this is the last part of data, so break the loop
			break;
		}
    }

    return(TRUE);
}



BOOL NANDCopyBack(SBPosition SrcPos, SBPosition DestPos)
{
    FlashInfo flashInfo;
    SectorInfo sectorInfo;
    PBYTE pBuf, pBuf2;
    SECTOR_ADDR sectorAddr, startSectorAddr, endSectorAddr;
    DWORD rdStartLogBlkAddr,rdEndLogBlkAddr,rdPhyBlockAddr;
    DWORD wtStartLogBlkAddr,wtEndLogBlkAddr,wtPhyBlockAddr;
    
    if(SrcPos == DestPos)
    {
        return FALSE;   
    }
    if(SrcPos != FirstBoot && SrcPos != SecondBoot)
    {
        return FALSE;   
    }
    if(DestPos != FirstBoot && DestPos != SecondBoot)
    {
        return FALSE;   
    }
    
    if(!FMD_GetInfo(&flashInfo))
    {
        RETAILMSG(TRUE, (_T("ERROR: Unable to get NAND flash information.\r\n")));
        return FALSE;
    }
    pBuf2 = pBuf = (PBYTE)LocalAlloc(LPTR, flashInfo.dwBytesPerBlock);
    if(!pBuf)
    {
        ERRORMSG(TRUE, (_T("Alloc memory fails\r\n")));
        return FALSE;   
    }
    
    // Fill the sectorInfo
    memset(&sectorInfo, 0xFF, sizeof(sectorInfo));

    // Calculate block range for the EBOOT image
    rdStartLogBlkAddr = IMAGE_BOOT_BOOTIMAGE_NAND_OFFSET;     
    rdEndLogBlkAddr = IMAGE_BOOT_BOOTIMAGE_NAND_OFFSET + IMAGE_BOOT_BOOTIMAGE_NAND_BLOCKS / 2;
    
    wtStartLogBlkAddr = IMAGE_BOOT_BOOTIMAGE_NAND_OFFSET + IMAGE_BOOT_BOOTIMAGE_NAND_BLOCKS / 2; 
    wtEndLogBlkAddr = IMAGE_BOOT_BOOTIMAGE_NAND_OFFSET + IMAGE_BOOT_BOOTIMAGE_NAND_BLOCKS;
    
    if(SrcPos == SecondBoot)
    {
        rdStartLogBlkAddr = IMAGE_BOOT_BOOTIMAGE_NAND_OFFSET + IMAGE_BOOT_BOOTIMAGE_NAND_BLOCKS / 2;
        rdEndLogBlkAddr = IMAGE_BOOT_BOOTIMAGE_NAND_OFFSET + IMAGE_BOOT_BOOTIMAGE_NAND_BLOCKS;
        
        wtStartLogBlkAddr = IMAGE_BOOT_BOOTIMAGE_NAND_OFFSET;
        wtEndLogBlkAddr = IMAGE_BOOT_BOOTIMAGE_NAND_OFFSET + IMAGE_BOOT_BOOTIMAGE_NAND_BLOCKS / 2;
    }
    if(!GetGoodPhyBlock(rdStartLogBlkAddr, &rdPhyBlockAddr))
    {
        ERRORMSG(TRUE, (_T("GetGoodPhyBlock failed: 0x%x\r\n"),rdStartLogBlkAddr));  
        return FALSE; 
    }
    if(!GetGoodPhyBlock(wtStartLogBlkAddr, &wtPhyBlockAddr))
    {
        ERRORMSG(TRUE, (_T("GetGoodPhyBlock failed: 0x%x\r\n"),wtStartLogBlkAddr));  
        return FALSE; 
    }
        
    do
    {
        // read
        pBuf = pBuf2;
        startSectorAddr = rdPhyBlockAddr * flashInfo.wSectorsPerBlock;
        endSectorAddr = startSectorAddr + flashInfo.wSectorsPerBlock;
        
        for (sectorAddr = startSectorAddr; sectorAddr < endSectorAddr; sectorAddr++)
        {
            if (!FMD_ReadSector(sectorAddr, pBuf, &sectorInfo, 1))
            {
                RETAILMSG(TRUE, (_T("ERROR: Failed to read image.\r\n")));
                return FALSE;
            }

            pBuf += flashInfo.wDataBytesPerSector;            
        }
        // write
        
        if (!FMD_EraseBlock(wtPhyBlockAddr))
        {
            FMD_SetBlockStatus(wtPhyBlockAddr, BLOCK_STATUS_BAD);
            RETAILMSG(TRUE, (_T("ERROR: Failed to erase block.\r\n")));
            return FALSE;
        }
        
        pBuf = pBuf2;
        startSectorAddr = wtPhyBlockAddr * flashInfo.wSectorsPerBlock;
        endSectorAddr = startSectorAddr + flashInfo.wSectorsPerBlock;
        
        for (sectorAddr = startSectorAddr; sectorAddr < endSectorAddr; sectorAddr++)
        {
            if (!FMD_WriteSector(sectorAddr, pBuf, &sectorInfo, 1))
            {
                RETAILMSG(TRUE, (_T("ERROR: Failed to write image.\r\n")));
                return FALSE;
            }

            pBuf += flashInfo.wDataBytesPerSector;            
        }
        
        
        rdStartLogBlkAddr++;
        wtStartLogBlkAddr++;
        
        GetNextGoodBlock(rdPhyBlockAddr, &rdPhyBlockAddr);
        GetNextGoodBlock(wtPhyBlockAddr, &wtPhyBlockAddr);
        
    }while(rdStartLogBlkAddr < rdEndLogBlkAddr);   

    if(!NANDWriteNCB(&g_CSPfi, DestPos) || !NANDWriteDBBT(&g_CSPfi, DestPos))
    {
        ERRORMSG(TRUE, (_T("write NCB&LDLB failed\r\n")));
        return FALSE;
    }   
    
    return TRUE;
}

//-----------------------------------------------------------------------------
// CS&ZHL APR-11-2012: routines of OTP
//-----------------------------------------------------------------------------
BOOL OTP_CUST_Read(DWORD* pdwCUST, DWORD dwLen)
{
	OtpProgram	Otp;
	DWORD		dwCUSTRegNum[4];
	DWORD		i1;

	//RETAILMSG(TRUE, (_T("-->OTP_CUST_READ\r\n")));

	InitOTP();
	dwCUSTRegNum[0] = OCOTP_CUST0_REG_NUM;
	dwCUSTRegNum[1] = OCOTP_CUST1_REG_NUM;
	dwCUSTRegNum[2] = OCOTP_CUST2_REG_NUM;
	dwCUSTRegNum[3] = OCOTP_CUST3_REG_NUM;

	//RETAILMSG(TRUE, (_T("Reg: 0x%x 0x%x 0x%x 0x%x\r\n"), dwCUSTRegNum[0], dwCUSTRegNum[1], dwCUSTRegNum[2], dwCUSTRegNum[3]));

	// read register CUST0 - CUST3
	if(dwLen > 4)
	{
		dwLen = 4;
	}

	for(i1 = 0; i1 < dwLen; i1++)
	{
		Otp.OtpAddr = OCOTP_ADDR_OFFSET(dwCUSTRegNum[i1]);
		if(!OTPRead(&Otp))
		{
			RETAILMSG(TRUE, (_T("read HW_OCOTP_CUST[%d] failed\r\n"), i1));
			return FALSE;
		}
		pdwCUST[i1] = Otp.OtpData;
	}

	return TRUE;
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
BOOL BSP_OEMIoControl(DWORD dwIoControlCode, PBYTE pInBuf, DWORD nInBufSize, 
    PBYTE pOutBuf, DWORD nOutBufSize, PDWORD pBytesReturned)
{
    BOOL rc = FALSE;
	DWORD  dwCUST[2];
    
    switch (dwIoControlCode)
	{
    case IOCTL_DISK_COPYBACK:
        if(!pOutBuf || nOutBufSize < sizeof(SBPosition))
        {
            ERRORMSG(TRUE, (L"para error in IOCTL_DISK_COPYBACK. \r\n"));
            break;
        }
        if(!pInBuf || nInBufSize < sizeof(SBPosition))
        {
            ERRORMSG(TRUE, (L"para error in IOCTL_DISK_COPYBACK. \r\n"));
            break;
        }
        BSPNAND_SetClock(TRUE);
        rc = NANDCopyBack(*(PSBPosition)pInBuf, *(PSBPosition)pOutBuf);
        BSPNAND_SetClock(FALSE);
        break;

	case IOCTL_DISK_VENDOR_GET_UPDATE_SIG:
    case IOCTL_DISK_VENDOR_SET_UPDATE_SIG:
    case IOCTL_DISK_GET_NANDBOOT_MODE:
    case IOCTL_DISK_SET_NANDBOOT_MODE:
        //SetLastError(ERROR_NOT_SUPPORTED);
		// CS&ZHL MAR-21-2012: SetLastError() is a founction in coredll
        ERRORMSG(TRUE, (_T("not supported\r\n")));
        break;

	case IOCTL_DISK_VENDOR_GET_SBIMGINFO:
        {
            DEBUGMSG(TRUE, (L"IOCTL_DISK_VENDOR_GET_IMGLENTH received. \r\n"));    
            if(!pOutBuf || nOutBufSize < sizeof(NANDImgInfo))
            {
                break;
            }
            memcpy(pOutBuf, &g_ImgInfo, sizeof(NANDImgInfo));
            //NANDGetImgInfo((PNANDImgInfo)pOutBuf);
            rc = TRUE;
            break;       
        }
        
    case IOCTL_DISK_VENDOR_GET_IMGINFO:
        {
            PNANDWrtImgInfo pNANDWrtImgInfo;

            DEBUGMSG(TRUE, (L"IOCTL_DISK_VENDOR_GET_IMGLENTH received. \r\n"));   
        
            if(!pOutBuf || nOutBufSize < sizeof(NANDWrtImgInfo))
            {
                ERRORMSG(TRUE, (_T("invalid parameters\r\n")));
                break;
            }
            
            pNANDWrtImgInfo = (PNANDWrtImgInfo)pOutBuf;
              
            if(pNANDWrtImgInfo->dwImgType == IMAGE_NK)
            {
                pNANDWrtImgInfo->dwImgSizeUnit = g_ImgInfo.dwNB0LenUnit;
            }
            else
            {
                pNANDWrtImgInfo->dwImgSizeUnit = g_ImgInfo.dwSBLenUnit;                  
            }
            rc = TRUE;
            break;       
        }

    case IOCTL_DISK_VENDOR_READ_EBOOT:    
        DEBUGMSG(TRUE, (L"IOCTL_DISK_VENDOR_READ_EBOOT received. \r\n"));
        if(!pOutBuf || nOutBufSize < g_ImgInfo.dwSBLenUnit)
        {
            ERRORMSG(TRUE, (_T("invalid parameters\r\n")));
            break;
        }
        if(!pInBuf || nInBufSize < sizeof(NANDIORequest))
        {
            ERRORMSG(TRUE, (_T("invalid parameters\r\n")));
            break;
        }
        if(((PNANDIORequest)pInBuf)->SBPos != FirstBoot && ((PNANDIORequest)pInBuf)->SBPos != SecondBoot)
        {
            ERRORMSG(TRUE, (_T("invalid parameters\r\n")));
            break;
        }
        BSPNAND_SetClock(TRUE);
        rc = NANDReadSB((PNANDIORequest)pInBuf, pOutBuf, nOutBufSize);
        BSPNAND_SetClock(FALSE);
        break;
          
	case IOCTL_DISK_VENDOR_WRITE_SB:
		DEBUGMSG(TRUE, (L"IOCTL_FLASH_VENDOR_WRITE_SB received. \r\n"));
		if(!pOutBuf || nOutBufSize < g_ImgInfo.dwSBLenUnit)
		{
		  ERRORMSG(TRUE, (_T("invalid parameters\r\n")));
		  break;
		}
		if(!pInBuf || nInBufSize < sizeof(NANDIORequest))
		{
		  ERRORMSG(TRUE, (_T("invalid parameters\r\n")));
		  break;
		}
		BSPNAND_SetClock(TRUE);
		rc = NANDWriteSB((PNANDIORequest)pInBuf, pOutBuf, nOutBufSize);
		BSPNAND_SetClock(FALSE);
		break;
              
	case IOCTL_DISK_VENDOR_END_WRITE_SB:
		DEBUGMSG(TRUE, (L"IOCTL_DISK_VENDOR_END_WRITE_SB received. \r\n"));
		if(!pInBuf || nInBufSize < sizeof(NANDIORequest))
		{
		  ERRORMSG(TRUE, (_T("invalid parameters\r\n")));
		  break;
		}
		BSPNAND_SetClock(TRUE);
		rc = NANDEndWriteSB((PNANDIORequest)pInBuf);
		BSPNAND_SetClock(FALSE);
		break; 

	case IOCTL_DISK_VENDOR_WRITE_IMAGE:
		{
			PNANDWrtImgInfo pNANDWrtImgInfo;

			if(!pInBuf || nInBufSize < sizeof(NANDWrtImgInfo))
			{
				ERRORMSG(TRUE, (_T("invalid parameters\r\n")));
				break;
			}                
			if(!pOutBuf || nOutBufSize < sizeof(DWORD))
			{
				ERRORMSG(TRUE, (_T("invalid parameters\r\n")));
				break;
			}
	        
			BSPNAND_SetClock(TRUE);
			pNANDWrtImgInfo = (PNANDWrtImgInfo)pInBuf;
	        
			//if(pNANDWrtImgInfo->dwImgType == IMAGE_NK)
			//{
			//	rc = NANDWriteNK(pNANDWrtImgInfo->dwIndex, pOutBuf,nOutBufSize);
			//}
			//else if((pNANDWrtImgInfo->dwImgType == IMAGE_EBOOTCFG) 
			//	 || (pNANDWrtImgInfo->dwImgType == IMAGE_SPLASH) 
			//	 || (pNANDWrtImgInfo->dwImgType == IMAGE_MBR))					// CS&ZHL APR-2-2012: add MBR boot mode
			//{
			//	rc = NANDWriteImage(pNANDWrtImgInfo, pOutBuf, nOutBufSize);
			//}
			//else
			//{
			//	NANDIORequest NANDIORequest;  
			//	NANDIORequest.SBPos = BothBoot;
			//	NANDIORequest.dwIndex = pNANDWrtImgInfo->dwIndex;
			//	rc = NANDWriteSB(&NANDIORequest, pOutBuf, nOutBufSize);                    
			//}
			//
			// CS&ZHL APR-11-2012: add more image types
			//
			switch(pNANDWrtImgInfo->dwImgType)
			{
			case IMAGE_NK:
				rc = NANDWriteNK(pNANDWrtImgInfo->dwIndex, pOutBuf,nOutBufSize);
				break;

			case IMAGE_EBOOTCFG:
				// use the MAC stored in OTP if possible
				OTPSyncEbootCfg(pOutBuf, nOutBufSize);
				pNANDWrtImgInfo->dwIndex = 0;	// force 0
				rc = NANDWriteImage(pNANDWrtImgInfo, pOutBuf, nOutBufSize);
				break;

			case IMAGE_SPLASH:
				rc = NANDWriteImage(pNANDWrtImgInfo, pOutBuf, nOutBufSize);
				break;

			case IMAGE_MBR:
				pNANDWrtImgInfo->dwIndex = 0;	// force 0
				rc = NANDWriteImage(pNANDWrtImgInfo, pOutBuf, nOutBufSize);
				break;

			case IMAGE_VID:
				if(!OTPSyncVendorInfo(pOutBuf, nOutBufSize))
				{
					RETAILMSG(TRUE, (L"OTP Sync Vendor Info failed.\r\n"));    
					break;
				}
				pNANDWrtImgInfo->dwIndex = 0;	// force 0
				rc = NANDWriteImage(pNANDWrtImgInfo, pOutBuf, nOutBufSize);
				break;

			case IMAGE_UID:
				pNANDWrtImgInfo->dwIndex = 0;	// force 0
				rc = NANDWriteImage(pNANDWrtImgInfo, pOutBuf, nOutBufSize);
				break;

			default:
				{
					NANDIORequest NANDIORequest;  
					NANDIORequest.SBPos = BothBoot;
					NANDIORequest.dwIndex = pNANDWrtImgInfo->dwIndex;
					rc = NANDWriteSB(&NANDIORequest, pOutBuf, nOutBufSize);  
				}
			}

			BSPNAND_SetClock(FALSE);
			break;
		}            
        
	case IOCTL_DISK_VENDOR_END_WRITE_IMAGE:
        {
            PNANDWrtImgInfo pNANDWrtImgInfo;

            if(!pInBuf || nInBufSize < sizeof(NANDWrtImgInfo))
            {
                ERRORMSG(TRUE, (_T("invalid parameters\r\n")));
                break;
            }             
            pNANDWrtImgInfo = (PNANDWrtImgInfo)pInBuf;

            BSPNAND_SetClock(TRUE);
            
            if(pNANDWrtImgInfo->dwImgType == IMAGE_NK)
            {
                NANDBootReserved();
            }
            else
            {
                NANDIORequest NANDIORequest;            

                NANDIORequest.SBPos = BothBoot;           
                rc = NANDEndWriteSB(&NANDIORequest);                  
            }       

            BSPNAND_SetClock(FALSE);

            break;    
        }
        
	case IOCTL_DISK_AUTHENTICATION:
		RETAILMSG(TRUE, (_T("INFO: DISK_VENDOR_AUTHENTICATION\r\n")));
        if(!pInBuf || (nInBufSize == 0))
		{
			ERRORMSG(TRUE, (_T("IOCTL_DISK_VENDOR_AUTHENTICATION: invalid parameters\r\n")));
		}
		if(!pOutBuf || (nOutBufSize != sizeof(DWORD)))
		{
			ERRORMSG(TRUE, (_T("IOCTL_DISK_VENDOR_AUTHENTICATION: invalid parameters\r\n")));
            break;
		}

#ifdef NAND_PDD
		{
			DWORD	dwType = *((DWORD*)pInBuf);

			if((dwType == 0) && (nInBufSize == sizeof(DWORD)))
			{
				// do vendor authentication
				rc = NANDVendorAuthentication();
			}
			else
			{
				// do user authentication
				rc = NANDUserAuthentication(pInBuf, nInBufSize);
			}

			if(!rc)
			{
				RETAILMSG(TRUE, (_T("IOCTL_DISK_VENDOR_AUTHENTICATION: failed\r\n")));
				*((PDWORD)pOutBuf) = 0;
			}
			else
			{
				// vendor authentication passed
				*((PDWORD)pOutBuf) = 1;
			}

			if(pBytesReturned)
			{
				*pBytesReturned = sizeof(DWORD);
			}
			rc = TRUE;
		}
#else
		// vendor authentication passed
		*((PDWORD)pOutBuf) = 1;
		if(pBytesReturned)
		{
			*pBytesReturned = sizeof(DWORD);
		}
		rc = TRUE;
#endif    // NAND_PDD
		break;
	//
	// CS&ZHL JUN-1-2012: code for EM9280 uce to check OTP mac
	//
	case IOCTL_DISK_GET_OTP_MAC:

		//RETAILMSG(TRUE, (_T("INFO: DISK_GET_OTP_MAC\r\n")));
		if( !pOutBuf || (nOutBufSize != 2*sizeof(DWORD)))
		{
			ERRORMSG(TRUE, (_T("IOCTL_DISK_GET_OTP_MAC: invalid parameters\r\n")));
            break;
		}
		
		if(!OTP_CUST_Read( dwCUST, 2))
		{
			RETAILMSG(TRUE, (_T("Read OTP failed\r\n")));
			dwCUST[0] = 0;
			dwCUST[1] = 0;
		}
		//RETAILMSG(TRUE, (_T("Read OTP 0x%08x 0x%08x\r\n"),dwCUST[0], dwCUST[1] ));

		memcpy( pOutBuf, (PBYTE)dwCUST, 8 );
		if(pBytesReturned)
		{
			*pBytesReturned = 2*sizeof(DWORD);
		}
		rc = TRUE;
		break;
	//
	// CS&ZHL AUG-13-2012: code for EM9280 uce to format NandFlash
	//
	case IOCTL_DISK_FORMAT:
		BSPNAND_SetClock(TRUE);
		NANDLowLevelFormat( );
        BSPNAND_SetClock(FALSE);
		rc = TRUE;
		break;
	default:
		break;
    }

    return(rc);
}


BYTE GetSum(PBYTE pBuf, DWORD dwLen)
{
	BYTE ub1 = 0;

	while(dwLen)
	{
		//ub1 += *pBuf;
		ub1 = ub1 + (*pBuf);
		pBuf++;
		dwLen--;
	}

	return ub1;
}

BOOL OTP_CUST_Program(DWORD* pdwCUST, DWORD dwLen)
{
	OtpProgram	Otp;
	DWORD		dwCUSTRegNum[4];
	DWORD		i1;
	DWORD		dwLockBits;
	BOOL		bRet=TRUE;
	DWORD		dwReadCUST[4];

	RETAILMSG(TRUE, (_T("-->OTP_CUST_Program\r\n")));

	InitOTP();
	dwCUSTRegNum[0] = OCOTP_CUST0_REG_NUM;
	dwCUSTRegNum[1] = OCOTP_CUST1_REG_NUM;
	dwCUSTRegNum[2] = OCOTP_CUST2_REG_NUM;
	dwCUSTRegNum[3] = OCOTP_CUST3_REG_NUM;

	// read register CUST0 - CUST3
	if(dwLen > 4)
	{
		dwLen = 4;
	}

	dwLockBits = 0;
	for(i1 = 0; i1 < dwLen; i1++)
	{
		dwLockBits |= (1 << i1);
		Otp.OtpAddr = OCOTP_ADDR_OFFSET(dwCUSTRegNum[i1]);
		Otp.OtpData = pdwCUST[i1];
		if(!OTPProgram(&Otp))
		{
			RETAILMSG(TRUE, (_T("program HW_OCOTP_CUST[%d] failed\r\n"), i1));
			bRet = FALSE;
			//return FALSE;
		}
	}

	if( !bRet )
		return FALSE;

	//
	// CS&ZHL JUN01-2012: check the chusum of pdwCUST[0] pdwCUST[1]
	//
	// read back
	for(i1 = 0; i1 < dwLen; i1++)
	{
		Otp.OtpAddr = OCOTP_ADDR_OFFSET(dwCUSTRegNum[i1]);
		if(!OTPRead(&Otp))
		{
			RETAILMSG(TRUE, (_T("read HW_OCOTP_CUST[%d] failed\r\n"), i1));
			return FALSE;
		}
		dwReadCUST[i1] = Otp.OtpData;
	}
	
	if( GetSum((PBYTE)dwReadCUST, 8)==0 )                        // lock  
	{
		// read OTP lock register
		Otp.OtpAddr = OCOTP_ADDR_OFFSET(OCOTP_LOCK_REG_NUM);
		if(!OTPRead(&Otp))
		{
			RETAILMSG(TRUE, (_T("read HW_OCOTP_LOCK failed\r\n")));
			return FALSE;
		}

		//program lock bits if required
		if(((DWORD)Otp.OtpData & dwLockBits) != dwLockBits)
		{
			Otp.OtpAddr  = OCOTP_ADDR_OFFSET(OCOTP_LOCK_REG_NUM);
			Otp.OtpData |= dwLockBits;
			if(!OTPProgram(&Otp))
			{
				RETAILMSG(TRUE, (_T("program HW_OCOTP_LOCK failed\r\n")));
				return FALSE;
			}
		}
	}

	return TRUE;
}


BOOL SecurityInfo2OTPCUST(PVENDOR_SECURITY_INFO pSecurityInfo, PDWORD pdwCUST)
{
	DWORD	dwTmp;
	BYTE	ub1;

	// setp 1: get MAC ID
	dwTmp = pSecurityInfo->mac[1] & 0xFF00;							// retrieve MAC_ADDR3
	pdwCUST[0] = dwTmp << 16;										// make MAC_ADDR3
	dwTmp = pSecurityInfo->mac[2] & 0x00FF;							// retrieve MAC_ADDR4
	pdwCUST[0] = pdwCUST[0] | (dwTmp << 16);						// make MAC_ADDR4
	dwTmp = pSecurityInfo->mac[2] & 0xFF00;							// retrieve MAC_ADDR5
	pdwCUST[0] = pdwCUST[0] | dwTmp;								// make MAC_ADDR5
	dwTmp = pSecurityInfo->dwNumOfPort & 0xFF;						// retrieve number of ethernet ports
	pdwCUST[0] = pdwCUST[0] | dwTmp;								// make MAC_ADDR5
	// setp 2: get date info
	dwTmp = pSecurityInfo->dwDay & 0xFF;							// retrieve DAY
	pdwCUST[1]  = dwTmp;											// make DAY
	dwTmp = pSecurityInfo->dwMonth & 0xFF;							// retrieve MONTH
	pdwCUST[1] |= (dwTmp << 8);										// make Month
	if(pSecurityInfo->dwYear >= 2000)
	{
		dwTmp = (pSecurityInfo->dwYear - 2000) & 0xFF;				// retrieve YEAR
	}
	else
	{
		dwTmp = pSecurityInfo->dwYear & 0xFF;						// retrieve YEAR
	}
	pdwCUST[1] |= (dwTmp << 16);									// make Year

	// step 3: make checksum
	ub1 = GetSum((PBYTE)pdwCUST, 7);
	//RETAILMSG(TRUE, (_T("SUM7 = 0x%02X\r\n"), ub1));
	ub1 = ~ub1 + 1;
	pdwCUST[1] |= ((DWORD)ub1) << 24;								// put into chusum

	// step4: dump message for debug
	RETAILMSG(TRUE, (_T("MAC = %02X-%02X-%02X-%02X-%02X-%02X\r\n"), 
						(pSecurityInfo->mac[0] & 0xFF),
						((pSecurityInfo->mac[0] >> 8) & 0xFF),
						(pSecurityInfo->mac[1] & 0xFF),
						((pSecurityInfo->mac[1] >> 8) & 0xFF),
						(pSecurityInfo->mac[2] & 0xFF),
						((pSecurityInfo->mac[2] >> 8) & 0xFF)));
	RETAILMSG(TRUE, (_T("Date = %d-%d-%d\r\n"), pSecurityInfo->dwYear, pSecurityInfo->dwMonth, pSecurityInfo->dwDay));
	//RETAILMSG(TRUE, (_T("CUST0 = 0x%08X, CUST1 = 0x%08X\r\n"), pdwCUST[0], pdwCUST[1]));

	return TRUE;
}

//----------------------------------------------------------------------------------------
//
// do security processing
//
// pBuf      - data buffer pointer
// dwBufSize - data buffer size in byte
// bLockFlag = TRUE: lock to data in buffer
//           = FALSE: unlock to data in buffer
//
//----------------------------------------------------------------------------------------
BOOL DoSecurityProcessing(PBYTE pBuf, DWORD dwBufSize, BOOL bLockFlag)
{
    UNREFERENCED_PARAMETER(pBuf);
    UNREFERENCED_PARAMETER(dwBufSize);
    UNREFERENCED_PARAMETER(bLockFlag);

	return TRUE;
}


//--------------------------------------------------------------------------------
// CS&ZHL APR-11-2012: routines for NandFlash Security
//--------------------------------------------------------------------------------
BOOL OTPSyncEbootCfg(PBYTE pBuf, DWORD dwBufSize)
{
	PBOOT_CFG				pBootCFG = (PBOOT_CFG)pBuf;
	BOOL					bRet = TRUE;
	DWORD					dwCUST[2];						// for HW_OCOTP_CUST0 - HW_OCOTP_CUST1
	WORD					wTemp;
	DWORD					dwSecurityInfoSize;
    NANDWrtImgInfo			NANDSecurityImgInfo;

    UNREFERENCED_PARAMETER(dwBufSize);

	if(!OTP_CUST_Read(dwCUST, 2))
	{
		RETAILMSG(TRUE, (_T("Read OTP failed\r\n")));
		goto read_vid;
	}

	// check HW_OCOTP_CUST0
	if((dwCUST[0] == 0) || (GetSum((PBYTE)dwCUST, 8) != 0))
	{
		RETAILMSG(TRUE, (_T("Invalid OTP_CUST\r\n")));
		goto read_vid;
	}

	// sync MAC ID to BootCFG
    pBootCFG->mac[0] = 0x9BD0;						// OUI => D0-9B-05
	wTemp = (WORD)(dwCUST[0] >> 16) & 0xFF00;		// get MAC_ADDR3
    pBootCFG->mac[1] = wTemp | 0x0005;
	wTemp = (WORD)(dwCUST[0] >> 16) & 0x00FF;		// get MAC_ADDR4
    pBootCFG->mac[2] = wTemp;
	wTemp = (WORD)(dwCUST[0] >> 0) & 0xFF00;		// get MAC_ADDR5
    pBootCFG->mac[2] = pBootCFG->mac[2] | wTemp;
	goto end_otp_sync;

read_vid:
	dwSecurityInfoSize = sizeof(VENDOR_SECURITY_INFO);
	// read use ID data 
	NANDSecurityImgInfo.dwImgType = IMAGE_VID;
	NANDSecurityImgInfo.dwIndex = 0;
	NANDSecurityImgInfo.dwImgSizeUnit = 0x20000;			// 128KB for large sector
	if(!NANDReadImage(&NANDSecurityImgInfo, (PBYTE)&g_SecurityInfo, dwSecurityInfoSize))
	{
        ERRORMSG(TRUE, (_T("read IMAGE_VID failed!\r\n")));
		bRet = FALSE;
		return bRet;
	}
	// copy mac info
	memcpy( pBootCFG->mac, g_SecurityInfo.mac, 6 );

end_otp_sync:
	RETAILMSG(TRUE, (_T("EbootCFG MAC = %02X-%02X-%02X-%02X-%02X-%02X\r\n"), 
						(pBootCFG->mac[0] & 0xFF),
						((pBootCFG->mac[0] >> 8) & 0xFF),
						(pBootCFG->mac[1] & 0xFF),
						((pBootCFG->mac[1] >> 8) & 0xFF),
						(pBootCFG->mac[2] & 0xFF),
						((pBootCFG->mac[2] >> 8) & 0xFF)));
	return bRet;
}

//--------------------------------------------------------------------------------
// if OTP is bank, program MAC & Date info into OTP.
// otherwise, sync OTP to Vendor Info
//--------------------------------------------------------------------------------
BOOL OTPSyncVendorInfo(PBYTE pBuf, DWORD dwBufSize)
{
	PVENDOR_SECURITY_INFO	pSecurityInfo;
	DWORD					dwCUST[2];			// for HW_OCOTP_CUST0 - HW_OCOTP_CUST1
	WORD					wTemp;
	BYTE					ub1;

	if( g_bAbortVendorInfoSync )
	{
		RETAILMSG(TRUE, (_T("Abort Vendor Info Sync\r\n")));
		return FALSE;
	}

	if(!pBuf || (dwBufSize != sizeof(VENDOR_SECURITY_INFO)))
	{
		RETAILMSG(TRUE, (_T("OTPSyncVendorInfo::invalid input parameter\r\n")));
		return FALSE;
	}
	pSecurityInfo = (PVENDOR_SECURITY_INFO)pBuf;

	if(!OTP_CUST_Read(dwCUST, 2))
	{
		RETAILMSG(TRUE, (_T("Read OTP failed\r\n")));
		return FALSE;
	}
	
	//RETAILMSG(TRUE, (_T("Read OTP 0x%x 0x%x\r\n"),dwCUST[0], dwCUST[1] ));

	// OTP is blank, program it!
	if((dwCUST[0] == 0) && (dwCUST[1] == 0))
	{
		RETAILMSG(TRUE, (_T("Program OTP...\r\n")));
		// convert security info into OTP_CUST format
		SecurityInfo2OTPCUST(pSecurityInfo, dwCUST);

		// then program OTP 
		if(!OTP_CUST_Program(dwCUST, 2))
		{
			RETAILMSG(TRUE, (_T("Program OTP failed\r\n")));
			return FALSE;
		}
	}
	else
	{	
		RETAILMSG(TRUE, (_T("Convert OTP...\r\n")));
		// OTP is burned already, sync it to SecurityInfo
		pSecurityInfo->mac[0] = 0x9BD0;								// OUI => D0-9B-05
		wTemp = (WORD)(dwCUST[0] >> 16) & 0xFF00;					// get MAC_ADDR3
		pSecurityInfo->mac[1] = wTemp | 0x0005;
		wTemp = (WORD)(dwCUST[0] >> 16) & 0x00FF;					// get MAC_ADDR4
		pSecurityInfo->mac[2] = wTemp;
		wTemp = (WORD)(dwCUST[0] >> 0) & 0xFF00;					// get MAC_ADDR5
		pSecurityInfo->mac[2] = pSecurityInfo->mac[2] | wTemp;

		// sync date info to SecurityInfo
		pSecurityInfo->dwDay   = (dwCUST[1] >>  0) & 0xFF;
		pSecurityInfo->dwMonth = (dwCUST[1] >>  8) & 0xFF;
		pSecurityInfo->dwYear  = ((dwCUST[1] >> 16) & 0xFF) + 2000;

		// dump message for debug
		RETAILMSG(TRUE, (_T("Read MAC = %02X-%02X-%02X-%02X-%02X-%02X\r\n"), 
							(pSecurityInfo->mac[0] & 0xFF),
							((pSecurityInfo->mac[0] >> 8) & 0xFF),
							(pSecurityInfo->mac[1] & 0xFF),
							((pSecurityInfo->mac[1] >> 8) & 0xFF),
							(pSecurityInfo->mac[2] & 0xFF),
							((pSecurityInfo->mac[2] >> 8) & 0xFF)));
		RETAILMSG(TRUE, (_T("Read Date = %d-%d-%d\r\n"), pSecurityInfo->dwYear, pSecurityInfo->dwMonth, pSecurityInfo->dwDay));
	}

	// get bad block layout info
	pSecurityInfo->dwNumOfBlocks = GetBadBlockInfo(pSecurityInfo->BadBlockTable, sizeof(pSecurityInfo->BadBlockTable));

	// do security processing -> lock the info
	DoSecurityProcessing(pBuf, dwBufSize, TRUE);

	// finally, make checksum of SecurityInfo
	ub1 = GetSum((PBYTE)pSecurityInfo, sizeof(VENDOR_SECURITY_INFO) - 1);
	ub1 = ~ub1 + 1;
	pSecurityInfo->ucCheckSum = ub1;

	return TRUE;
}

//--------------------------------------------------------------------------------
// CS&ZHL MAY-29-2012: routines for NandFlash Bad Block Table
//--------------------------------------------------------------------------------
BOOL NandBadBlockTableIsValid( PBYTE pTab1, PBYTE pTab2, DWORD dwSize )
{
    FlashInfo		flashInfo;
	DWORD			dwByteIndex;

    
    if(!FMD_GetInfo(&flashInfo))
    {
        RETAILMSG(TRUE, (_T("ERROR: Unable to get NAND flash information.\r\n")));
        return 0;
    }
    
	/* get system for reserved blockID/8 */
	dwByteIndex = IMAGE_BOOT_NANDDEV_RESERVED_SIZE / flashInfo.dwBytesPerBlock / 8;

	if( ( dwByteIndex > dwSize )||( memcmp( pTab1, pTab2, dwByteIndex )!=0 ) )
	{
		return FALSE;
	}

	for( ; dwByteIndex < dwSize; dwByteIndex++ ) 
	{
		if( pTab2[dwByteIndex]!=0 )
		{
			if( (pTab1[dwByteIndex] & pTab2[dwByteIndex]) != pTab2[dwByteIndex] )
				return FALSE;
		}
	}

	return TRUE;
}

//--------------------------------------------------------------------------------
// CS&ZHL APR-11-2012: routines for NandFlash Security
//--------------------------------------------------------------------------------
BOOL NANDVendorAuthentication(void)
{
	BOOL					bRet = TRUE;
	DWORD					dwOTP_CUST[2];			// HW_OCOTP_CUST0 - HW_OCOTP_CUST1 in OTP registers
	PVENDOR_SECURITY_INFO	pSecurityInfo;
	DWORD					dwSecurityInfoSize;
    NANDWrtImgInfo			NANDSecurityImgInfo;
	DWORD					dwNAND_CUST[2];			// HW_OCOTP_CUST0 - HW_OCOTP_CUST1 in IMAGE_VID
	BYTE					pBadBlockTable[512];
	DWORD					dwNumOfBlocks;

	// read OTP info
	if(!OTP_CUST_Read(dwOTP_CUST, 2))
	{
		ERRORMSG(TRUE, (_T("Read OTP failed\r\n")));
		bRet = FALSE;
		goto cleanup;
	}
	//RETAILMSG(TRUE, (_T("OTP 0x%x 0x%x\r\n"), dwOTP_CUST[0], dwOTP_CUST[1]));

	if((dwOTP_CUST[0] == 0) && (dwOTP_CUST[1] == 0))
	{
		RETAILMSG(TRUE, (_T(" Vendor Authentication Failed!\r\n")));
		bRet = FALSE;
		goto cleanup;
	}


	dwSecurityInfoSize = sizeof(VENDOR_SECURITY_INFO);
	pSecurityInfo = (PVENDOR_SECURITY_INFO)&g_SecurityInfo;
	
	// read use ID data 
	NANDSecurityImgInfo.dwImgType = IMAGE_VID;
	NANDSecurityImgInfo.dwIndex = 0;
	NANDSecurityImgInfo.dwImgSizeUnit = 0x20000;			// 128KB for large sector
	if(!NANDReadImage(&NANDSecurityImgInfo, (PBYTE)pSecurityInfo, dwSecurityInfoSize))
	{
        ERRORMSG(TRUE, (_T("read IMAGE_VID failed!\r\n")));
		bRet = FALSE;
		goto cleanup;
	}
	

	// verify checksum
	if(GetSum((PBYTE)pSecurityInfo, dwSecurityInfoSize) != 0)
	{
        RETAILMSG(TRUE, (_T("IMAGE_VID checksum failed!\r\n")));
		bRet = FALSE;
		goto cleanup;
	}
    //RETAILMSG(TRUE, (_T("IMAGE_VID checksum OK!\r\n")));

	// do security processing -> unlock the info
	DoSecurityProcessing((PBYTE)pSecurityInfo, dwSecurityInfoSize, FALSE);

	// convert security info into OTP_CUST format with chksum
	SecurityInfo2OTPCUST(pSecurityInfo, dwNAND_CUST);
	//RETAILMSG(TRUE, (_T("Nand 0x%x 0x%x\r\n"), dwNAND_CUST[0], dwNAND_CUST[1]));

	// check OTP info
	if(memcmp(dwOTP_CUST, dwNAND_CUST, 8) != 0)
	{
        RETAILMSG(TRUE, (_T("OTP is NOT matched!\r\n")));
		bRet = FALSE;
		goto cleanup;
	}

	// get bad block layout info
	dwNumOfBlocks = GetBadBlockInfo(pBadBlockTable, sizeof(pSecurityInfo->BadBlockTable));
	if(dwNumOfBlocks != pSecurityInfo->dwNumOfBlocks)
	{
        RETAILMSG(TRUE, (_T("Bad Block Table Size is NOT matched!\r\n")));
		bRet = FALSE;
		goto cleanup;
	}

	// compare the bad block table
	//if(memcmp(pBadBlockTable, pSecurityInfo->BadBlockTable, sizeof(pSecurityInfo->BadBlockTable)) != 0)
	if( NandBadBlockTableIsValid( pBadBlockTable, pSecurityInfo->BadBlockTable, sizeof(pSecurityInfo->BadBlockTable )) == FALSE )
	{
        RETAILMSG(TRUE, (_T("Bad Block Table is NOT matched!\r\n")));
		bRet = FALSE;
		goto cleanup;
	}

cleanup:
	return bRet;
}


BOOL NANDUserAuthentication(PBYTE pBuf, DWORD dwBufSize)
{
    NANDWrtImgInfo  NANDSecurityImgInfo;
	PBYTE			pSrcBuf;
	BOOL			bRet = TRUE;

	// allocate source data buffer
    pSrcBuf = (PBYTE) LocalAlloc(LPTR, dwBufSize); 
	if(pSrcBuf == NULL)
	{
        ERRORMSG(TRUE, (_T("Allocate memory with size of 0x%x failed!\r\n"), dwBufSize));
        return FALSE;
	}

	// read use ID data 
	NANDSecurityImgInfo.dwImgType = IMAGE_UID;
	NANDSecurityImgInfo.dwIndex = 0;
	NANDSecurityImgInfo.dwImgSizeUnit = 0x20000;			// 128KB for large sector
	if(!NANDReadImage(&NANDSecurityImgInfo, pSrcBuf, dwBufSize))
	{
        ERRORMSG(TRUE, (_T("read IMAGE_UID failed!\r\n")));
		bRet = FALSE;
		goto cleanup;
	}

	// compare the data
	if(memcmp(pBuf, pSrcBuf, dwBufSize) != 0)
	{
        RETAILMSG(TRUE, (_T("User Authentication Failed!\r\n")));
		bRet = FALSE;
	}

cleanup:
	LocalFree(pSrcBuf); 

	return bRet;
}
