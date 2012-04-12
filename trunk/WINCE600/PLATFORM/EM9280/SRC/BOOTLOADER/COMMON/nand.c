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
#include "common_nandfmd.h"
#pragma warning(pop)
//-----------------------------------------------------------------------------
// External Functions


//-----------------------------------------------------------------------------
// External Variables
extern BOOL g_bNandExist;
extern FlashInfoExt g_CSPfi;

//-----------------------------------------------------------------------------
// Defines
//#define NANDFC_BOOT_SIZE (4096)
#define MAX_GPMI_CLK_FREQUENCY_kHZ (120000)


//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables


//-----------------------------------------------------------------------------
// Local Variables
BYTE sectorBuf[NANDFC_BOOT_SIZE];

//-----------------------------------------------------------------------------
// Local Functions
//-----------------------------------------------------------------------------
//
//  Function:  NFC_SetClock
//
//  This enables/disable clocks for the NANDFC.
//
//  Parameters:
//     bEnabled
//          [in] - enable/disable clock.  
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------
BOOL NFC_SetClock(BOOL bEnabled)
{
    BOOL rc = TRUE;
    UINT32 frequency , rootfreq, u32Div;
    static BOOL bInit = FALSE;

    if(!bInit){
        // Bump GPMI_CLK frequency up to the maximum.
        frequency = MAX_GPMI_CLK_FREQUENCY_kHZ;
        //status = DDKClockSetGpmiClk(&frequency, TRUE);

        DDKClockGetFreq(DDK_CLOCK_SIGNAL_REF_GPMI, &rootfreq);

        u32Div = rootfreq / (frequency*1000) + 1;
        if(u32Div != 0)
            rc = DDKClockConfigBaud(DDK_CLOCK_SIGNAL_GPMI, DDK_CLOCK_BAUD_SOURCE_REF_GPMI, u32Div );
        if (rc != TRUE)
        {
            return rc;
        }    
        bInit = TRUE;
    }
    
    if (bEnabled)
    {
        rc = DDKClockSetGatingMode(DDK_CLOCK_GATE_GPMI_CLK, FALSE);
    }
    else
    {
        rc = DDKClockSetGatingMode(DDK_CLOCK_GATE_GPMI_CLK, TRUE);
    }

    return rc;
}

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
        EdbgOutputDebugString("ERROR: Unable to get NAND flash information.\r\n");
        return;
    }
    
    pNANDImageCfg->dwXldrOffset = 0xFFFFFFFF;
    pNANDImageCfg->dwXldrSize = 0;

    pNANDImageCfg->dwBootOffset = IMAGE_BOOT_BOOTIMAGE_NAND_OFFSET * flashInfo.dwBytesPerBlock;
    pNANDImageCfg->dwBootSize = IMAGE_BOOT_BOOTIMAGE_NAND_BLOCKS * flashInfo.dwBytesPerBlock;

    pNANDImageCfg->dwIplOffset = 0xFFFFFFFF;
    pNANDImageCfg->dwIplSize = 0;
    
    pNANDImageCfg->dwNkOffset = IMAGE_BOOT_NKIMAGE_NAND_OFFSET * flashInfo.dwBytesPerBlock;		// IMAGE_BOOT_NKIMAGE_NAND_OFFSET = 0x20C
    pNANDImageCfg->dwNkSize = IMAGE_BOOT_NKIMAGE_NAND_SIZE;										// IMAGE_BOOT_NKIMAGE_NAND_SIZE = 0x2000000 => 32MB

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
	pNANDImageCfg->dwMBRSize   = IMAGE_BOOT_MBR_NAND_BLOCKS * flashInfo.dwBytesPerBlock;

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

static BOOL GetGoodPhyBlock(DWORD LogBlockAddr, DWORD *pGoodBlkAddr)
{
    DWORD StartPhyBlockAddr = 0;
    DWORD i;
    FlashInfo flashInfo;
    
    if (!FMD_GetInfo(&flashInfo))
    {
        EdbgOutputDebugString("ERROR: Unable to get NAND flash information.\r\n");
        return(FALSE);
    }
    
    if(LogBlockAddr >= flashInfo.dwNumBlocks)
    {
        EdbgOutputDebugString("ERROR: LogBlockAddr exceed flashInfo.dwNumBlocks.\r\n");
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
            //EdbgOutputDebugString(" skip bad block 0x%x\r\n",StartPhyBlockAddr);
        }
        StartPhyBlockAddr++;
    }

    return FALSE;
}

static BOOL GetNextGoodBlock(DWORD StartPhyBlockAddr, DWORD *pNxtGoodBlkAddr)
{
    DWORD		BlockStatus;
    FlashInfo	flashInfo;
    
    if (!FMD_GetInfo(&flashInfo))
    {
        EdbgOutputDebugString("ERROR: Unable to get NAND flash information.\r\n");
        return(FALSE);
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

////////////////////////////////////////////////////////////////////////////////
//!
//! \brief Implements BCB checksum algorithm,
//!
//! \param[in] pointer to BCB buffer.
//! \param[in] size of buffer
//!
//! \retval returns calculated checksum on pBuffer of u32Size.
//!
////////////////////////////////////////////////////////////////////////////////
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

BOOL NANDWriteNCB(PFlashInfoExt pFlashInfo)
{
    BootBlockStruct_t NCB;
    DWORD i, j;
    SectorInfo si;
    BYTE NCBNum = 0;
    DWORD startBlk1, startBlk2, endBlk2, bbtBlk;
    
    //write NCB to block 0/2
    memset(&NCB, 0, sizeof(NCB));
    memset(sectorBuf, 0xff, NANDFC_BOOT_SIZE);

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

	//
	// CS&ZHL DEC-13-2011: check StartBlockAddr of Eboot1, Eboot2, EbootCFG, and Bad Block Table used by rom
	//
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
        if(!FMD_EraseBlock(i))
        {
            ERRORMSG(TRUE, (_T("erase block %d failed, set to bad\r\n"),i));
            FMD_SetBlockStatus(i, BLOCK_STATUS_BAD);
        }
        else
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
    
    return NCBNum >= 2;
}

BOOL NANDWriteDBBT(PFlashInfoExt pFlashInfo)
{
    BootBlockStruct_t NCB;
    DWORD badBlockNum = 0;
    WORD  badBlockBuf[CHIP_BAD_BLOCK_RANGE], i;			// CHIP_BAD_BLOCK_RANGE = 425
    SectorInfo si;
    DWORD bbtBlk;
    
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
//-----------------------------------------------------------------------------
//
//  Function:  NANDWriteSB
//
//  This function writes to NAND flash memory the Boot image stored 
//  in the RAM file cache area.
//
//  Parameters:
//      dwStartAddr 
//          [in] Address in flash memory where the start of the downloaded 
//          Boot image is to be written.
//
//      dwLength 
//          [in] Length of the Boot image, in bytes, to be written to flash
//          memory.            
//
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//-----------------------------------------------------------------------------
BOOL NANDWriteSB(DWORD dwStartAddr, DWORD dwLength)
{
    LPBYTE pSectorBuf, pImage,pVerSectorBuf;
    SectorInfo sectorInfo,VersectorInfo;
    SECTOR_ADDR sectorAddr, startSectorAddr, endSectorAddr;
    DWORD StartLogBlkAddr,EndLogBlkAddr,PhyBlockAddr,ValidEndLogBlkAddr;
    DWORD dwValidImageLength;
    FlashInfo flashInfo;
    
    EdbgOutputDebugString("->NANDWriteSB\r\n");

    // Check for NAND device availability
    if(!g_bNandExist)
    {
        EdbgOutputDebugString("WARNING: NAND device doesn't exist - unable to store image.\r\n");
        return FALSE;
    }
    
    EdbgOutputDebugString("INFO: Writing Boot image to NAND (please wait)...\r\n");
    
    if(!FMD_GetInfo(&flashInfo))
    {
        EdbgOutputDebugString("ERROR: Unable to get NAND flash information.\r\n");
        return FALSE;
    }
    
    // Make sure Boot length does not exceed reserved NAND size
    if(dwLength > IMAGE_BOOT_BOOTIMAGE_NAND_BLOCKS * flashInfo.dwBytesPerBlock / 2)
    {
        EdbgOutputDebugString("ERROR: Boot size exceeds reserved NAND region (size = 0x%x)\r\n", dwLength);
        return FALSE;
    }

    // Get cached image location
    pImage = OEMMapMemAddr(dwStartAddr, dwStartAddr);
    
    EdbgOutputDebugString("INFO: Programming Firmware.sb image from flash cache address 0x%x, size = %d\r\n", pImage, dwLength);

    //To reduce unnecessary programming
    dwValidImageLength = ((dwLength + flashInfo.dwBytesPerBlock - 1) / flashInfo.dwBytesPerBlock)*flashInfo.dwBytesPerBlock;
    
    // Write sb to NAND flash
    pSectorBuf = pImage;    
    pVerSectorBuf = pImage + dwValidImageLength;
    
    // Fill unused space with 0xFF
    memset(pSectorBuf + dwLength, 0xFF, (dwValidImageLength) - dwLength);

    // Fill the sectorInfo
    memset(&sectorInfo, 0xFF, sizeof(sectorInfo));

    // Set Reserved and image flag
    sectorInfo.bOEMReserved &= ~OEM_BLOCK_RESERVED;   
        
    // Calculate block range for the EBOOT image
    StartLogBlkAddr = IMAGE_BOOT_BOOTIMAGE_NAND_OFFSET;     
    ValidEndLogBlkAddr   = StartLogBlkAddr + dwValidImageLength / flashInfo.dwBytesPerBlock;
    EndLogBlkAddr = StartLogBlkAddr + IMAGE_BOOT_BOOTIMAGE_NAND_BLOCKS / 2;
	//
	// CS&ZHL DEC-13-2011: get the good physical block of StartLogBlkAddr;
	//
    if(!GetGoodPhyBlock(StartLogBlkAddr, &PhyBlockAddr))
    {
		ERRORMSG(TRUE, (_T("NANDWriteSB::GetGoodPhyBlock failed: 0x%x, exit!\r\n"),StartLogBlkAddr));   
        return FALSE;
    }

begin_program:  
    // Write sb to NAND flash
    pSectorBuf = pImage;    
    pVerSectorBuf = pImage + dwValidImageLength;
    //EdbgOutputDebugString("INFO: Programming NAND flash blocks [0x%x - 0x%x].0x%x\r\n", StartLogBlkAddr, EndLogBlkAddr-1,ValidEndLogBlkAddr);
    EdbgOutputDebugString("INFO: Programming NAND flash blocks [0x%x - 0x%x] within block[0x%x - 0x%x]\r\n", StartLogBlkAddr, ValidEndLogBlkAddr, StartLogBlkAddr, EndLogBlkAddr-1);
    do
    {
        //PhyBlockAddr = StartLogBlkAddr;
        //if(!GetGoodPhyBlock(StartLogBlkAddr, &PhyBlockAddr))
        //{
        //    ERRORMSG(TRUE, (_T("GetGoodPhyBlock failed: 0x%x\r\n"),StartLogBlkAddr));   
        //}
        
        // Erase the block...
        if (!FMD_EraseBlock(PhyBlockAddr))
        {
            FMD_SetBlockStatus(PhyBlockAddr, BLOCK_STATUS_BAD);
            if(!(FMD_GetBlockStatus(PhyBlockAddr) & BLOCK_STATUS_BAD))
            {
                EdbgOutputDebugString("ERROR: Unable to erase NAND flash block [0x%x].\r\n", PhyBlockAddr);
                return FALSE;
            }
            else
            {
                continue;
            }
        }
        
        // Compute sector address based on current physical block
        startSectorAddr = PhyBlockAddr * flashInfo.wSectorsPerBlock;
        endSectorAddr = startSectorAddr + flashInfo.wSectorsPerBlock;
        
        for (sectorAddr = startSectorAddr; sectorAddr < endSectorAddr; sectorAddr++)
        {
            if (!FMD_WriteSector(sectorAddr, pSectorBuf, &sectorInfo, 1))
            {
                EdbgOutputDebugString("ERROR: Failed to update EBOOT/SBOOT.\r\n");
                return FALSE;
            }

            //Verify...
            if (!FMD_ReadSector(sectorAddr, pVerSectorBuf, &VersectorInfo, 1))
            {
                EdbgOutputDebugString("ERROR: Failed to verify image.\r\n");
                return FALSE;
            }

            if (memcmp(pSectorBuf, pVerSectorBuf, flashInfo.wDataBytesPerSector) != 0)
            {
                EdbgOutputDebugString("ERROR: Failed to verify image.\r\n");
                return FALSE;
            }
            
            pSectorBuf += flashInfo.wDataBytesPerSector;            
        }

		//
		// CS&ZHL DEC-13-2011: get next physical good block
		//
        if(!GetNextGoodBlock(PhyBlockAddr, &PhyBlockAddr))
        {
            EdbgOutputDebugString("Error: No good block found - unable to store image!\r\n");
            return FALSE;
        }

        StartLogBlkAddr++;
    }while(StartLogBlkAddr < ValidEndLogBlkAddr);   
    
    //Mark all blocks as reserved.  
    while(StartLogBlkAddr < EndLogBlkAddr)
    {
        //PhyBlockAddr = StartLogBlkAddr;
        //if(!GetGoodPhyBlock(StartLogBlkAddr, &PhyBlockAddr))
        //{
        //    ERRORMSG(TRUE, (_T("GetGoodPhyBlock failed: 0x%x\r\n"),StartLogBlkAddr));   
        //}
        
        if (!FMD_EraseBlock(PhyBlockAddr))
        {
            FMD_SetBlockStatus(PhyBlockAddr, BLOCK_STATUS_BAD);
            if(!(FMD_GetBlockStatus(PhyBlockAddr) & BLOCK_STATUS_BAD))
            {
                EdbgOutputDebugString("ERROR: Unable to erase NAND flash block [0x%x].\r\n", PhyBlockAddr);
                return FALSE;
            }
            else
            {
                continue;
            }
        }   
        
        FMD_SetBlockStatus((BLOCK_ID)PhyBlockAddr, BLOCK_STATUS_RESERVED);  
        
        if(!(FMD_GetBlockStatus((BLOCK_ID)PhyBlockAddr) & BLOCK_STATUS_RESERVED))
        {
            EdbgOutputDebugString("NAND flash block [0x%x] isn't reserved!!\r\n", PhyBlockAddr);

        }

		//
		// CS&ZHL DEC-13-2011: get next physical good block
		//
        if(!GetNextGoodBlock(PhyBlockAddr, &PhyBlockAddr))
        {
            EdbgOutputDebugString("Error: No good block found - unable to store image!\r\n");
            return FALSE;
        }

		StartLogBlkAddr++;
    }
    
    if(EndLogBlkAddr != (IMAGE_BOOT_BOOTIMAGE_NAND_OFFSET + IMAGE_BOOT_BOOTIMAGE_NAND_BLOCKS))
    {	// CS&ZHL JAN-11-2012: comments -> prepare to write the second Eboot area 
        StartLogBlkAddr = EndLogBlkAddr;
        ValidEndLogBlkAddr = StartLogBlkAddr + dwValidImageLength/flashInfo.dwBytesPerBlock;
        EndLogBlkAddr = IMAGE_BOOT_BOOTIMAGE_NAND_OFFSET + IMAGE_BOOT_BOOTIMAGE_NAND_BLOCKS;
        goto begin_program;
    }
   
    
    EdbgOutputDebugString("INFO: Write NCB and DBBT.\r\n");    
    if(!NANDWriteNCB(&g_CSPfi) || !NANDWriteDBBT(&g_CSPfi))
    {
        ERRORMSG(TRUE, (_T("write NCB&LDLB failed\r\n")));
        return FALSE;
    }

    EdbgOutputDebugString("INFO: Verifying image succeed.\r\n");    
    EdbgOutputDebugString("INFO: Updating of SB image completed successfully.\r\n");
    EdbgOutputDebugString("<-NANDWriteSB\r\n");
    return(TRUE);
}

BOOL BSP_OEMIoControl(DWORD dwIoControlCode, PBYTE pInBuf, DWORD nInBufSize, 
    PBYTE pOutBuf, DWORD nOutBufSize, PDWORD pBytesReturned)
{
    BOOL rc = FALSE;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(dwIoControlCode);
    UNREFERENCED_PARAMETER(pInBuf);
    UNREFERENCED_PARAMETER(nInBufSize);
    UNREFERENCED_PARAMETER(pOutBuf);
    UNREFERENCED_PARAMETER(nOutBufSize);
    UNREFERENCED_PARAMETER(pBytesReturned);
    return(rc);
}
