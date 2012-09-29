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
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  nandboot.c
//
//  Contains BOOT NAND flash support functions.
//
//-----------------------------------------------------------------------------
#pragma warning(disable: 4100 4115 4127 4201 4204 4214)
#include <windows.h>
#include <blcommon.h>
#include <bootpart.h>
#include "nandboot.h"
#include "common_nandfmd.h"

//-----------------------------------------------------------------------------
// External Functions


//-----------------------------------------------------------------------------
// External Variables
extern BOOL g_bNandExist;

//
// CS&ZHL MAY-19-2011: variables for BinFS
//
extern DWORD g_dwTotalBinNum;			// CS&ZHL MAY-21-2011: get from OEMMultiBINNotify()
extern DWORD g_dwCurrentBinNum;
extern DWORD g_dwBinAddress[];		// CS&ZHL MAY-21-2011: Flash Physical Addresses
extern DWORD g_dwBinLength[];			// Now we support 16 bin files for MultiBIN


//-----------------------------------------------------------------------------
// Defines
// CS&ZHL MAY-21-2011: Partition for BinFS
#define DEFAULT_SECTOR_SIZE	512
#define MBR_SIZE							DEFAULT_SECTOR_SIZE
#define NUM_PARTS						4
#define SIZE_END_SIG					2


//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables


//-----------------------------------------------------------------------------
// Local Variables
static NAND_IMAGE_CFG NANDImageCfg;
static SECTOR_BUFFER SectorBuffer;
static FlashInfo flashInfo;


//-----------------------------------------------------------------------------
// Local Functions
//
// CS&ZHL MAY-19-2011: routines for BinFS
//
static CHSAddr LBAtoCHS(FlashInfo *pFlashInfo, LBAAddr lba)
{
    CHSAddr chs;
    DWORD tmp = pFlashInfo->dwNumBlocks * pFlashInfo->wSectorsPerBlock;

    chs.cylinder = (WORD)(lba / tmp);
    tmp = lba % tmp;
    chs.head = (WORD)(tmp / pFlashInfo->wSectorsPerBlock);
    chs.sector = (WORD)((tmp % pFlashInfo->wSectorsPerBlock) + 1);

    return chs;
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

    return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function:  NANDCheckImageAddress
//
//  This function checks the image address if it is block aligned. 
//
//  Parameters:
//      dwPhyAddr 
//          [in] Physical address for nand image.            
//
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//-----------------------------------------------------------------------------
BOOL NANDCheckImageAddress(DWORD dwPhyAddr)
{
    // Get NAND flash data bytes per sector.
    if(!g_bNandExist)
    {
        EdbgOutputDebugString("WARNING: NAND device doesn't exist - unable to read image.\r\n");
        return FALSE;
    }
    
    // Boot Configuration and Buffer
    if(!NANDBootInit())
    {
        EdbgOutputDebugString("WARNING: NANDBootInit fail\r\n");
        return FALSE;
    }

    // Get NAND flash data bytes per sector.
    //
    if (!FMD_GetInfo(&flashInfo))
    {
        EdbgOutputDebugString("ERROR: Unable to get NAND flash information.\r\n");
        return(FALSE);
    }

	//if(((dwPhyAddr - IMAGE_BOOT_NANDDEV_NAND_PA_START) % flashInfo.dwBytesPerBlock) != 0)
	if(((dwPhyAddr - NANDImageCfg.dwNandDevPhyAddr) % flashInfo.dwBytesPerBlock) != 0)
		return FALSE;
	else
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
    DWORD dwResult;
    BLOCK_ID blockID, startBlockID, endBlockID;
    
    if(!FMD_GetInfo(&flashInfo))
    {
        EdbgOutputDebugString("ERROR: Unable to get NAND flash information.\r\n");
        return;
    }

    // Boot Configuration and Buffer
    BSP_GetNANDImageCfg(&NANDImageCfg);

    startBlockID = 0;
    endBlockID = (DWORD)(NANDImageCfg.dwNandSize + flashInfo.dwBytesPerBlock - 1) / (DWORD)flashInfo.dwBytesPerBlock;
    for(blockID = startBlockID; blockID < endBlockID; blockID++)
    {
        dwResult = FMD_GetBlockStatus(blockID);
        
        // Skip bad blocks
        if(dwResult & BLOCK_STATUS_BAD)
        {
            EdbgOutputDebugString("INFO: Found bad NAND flash block [0x%x].\r\n", blockID);
            continue;
        }

        // Skip reserved blocks
        if(dwResult & BLOCK_STATUS_RESERVED)
        {
            continue;
        }
        
        // Erase the block...
        if(!FMD_EraseBlock(blockID))
        {
            EdbgOutputDebugString("ERROR: Unable to erase NAND flash block [0x%x].\r\n", blockID);
            FMD_SetBlockStatus(blockID, BLOCK_STATUS_BAD);
            continue;
        }

        if(!FMD_SetBlockStatus(blockID, BLOCK_STATUS_RESERVED))
        {
            EdbgOutputDebugString("ERROR: Unable to set block status [0x%x].\r\n", blockID);
            continue;
        }
    }
}


BOOL GetNextGoodBlock(UINT32 StartPhyBlockAddr, UINT32 *pNxtGoodBlkAddr)
{
    UINT32 BlockStatus;

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


BOOL GetPhyBlkAddr(UINT32 LogicalBlkAddr, UINT32 *pNxtGoodBlkAddr)
{
    UINT32 i;

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

BOOL WriteSingleBlock(DWORD BlockID, LPBYTE pSectorBuf)
{
    BYTE *pVeriSectorBuf = SectorBuffer.pSectorBuf;
    SectorInfo sectorInfo;
    SECTOR_ADDR sectorAddr, startSectorAddr, endSectorAddr;
    
    FMD_EraseBlock(BlockID);
    DEBUGMSG(TRUE, (_T("INFO: Programming NAND flash blocks [0x%x].\r\n"), BlockID));
        
    
    startSectorAddr = BlockID * flashInfo.wSectorsPerBlock;
    endSectorAddr = startSectorAddr + flashInfo.wSectorsPerBlock;
    
    // Fill the sectorInfo
    memset(&sectorInfo, 0xFF, sizeof(sectorInfo));
    sectorInfo.bOEMReserved &= ~OEM_BLOCK_RESERVED;
    sectorInfo.bOEMReserved &= ~OEM_BLOCK_READONLY;

    // Write the image
    for(sectorAddr = startSectorAddr; sectorAddr < endSectorAddr; sectorAddr++)
    {
        if(!FMD_WriteSector(sectorAddr, pSectorBuf, &sectorInfo, 1))
        {
            RETAILMSG(TRUE, (_T("WARNING: Failed to write XLDR in the 1st Block.\r\n")));
            return FALSE;
        }
        
        if(!FMD_ReadSector(sectorAddr, pVeriSectorBuf, &sectorInfo, 1))
        {
            RETAILMSG(TRUE, (_T("WARNING: Failed to write XLDR in the 1st Block.\r\n")));
            return FALSE;
        }
        
        if(memcmp(pVeriSectorBuf, pSectorBuf, flashInfo.wDataBytesPerSector) != 0)
        {
            RETAILMSG(TRUE, (_T("ERROR: Failed to verify XLDR.\r\n")));
            return FALSE;
        }        

        pSectorBuf += flashInfo.wDataBytesPerSector;
    }    
    return TRUE;
    
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
//BOOL NANDWriteImage(PNANDWrtImgInfo pNANDWrtImgInfo, LPBYTE pImage, DWORD dwLength)
//
// CS&ZHL MAY-23-2011: change to:
//			dwImageStart:		[in] virtual address of image in NandFlash       
//			dwImageLength:	[in] byte length of image in NandFlash       
//
BOOL NANDWriteImage(PNANDWrtImgInfo pNANDWrtImgInfo, DWORD dwImageStart, DWORD dwImageLength)
{
    UINT32 StartLogBlkAddr,EndLogBlkAddr,PhyBlockAddr;
    UINT32 dwStartOffset,dwImageSize, dwValidImageBlock;
    UINT32 percentComplete, lastPercentComplete=0;    
	LPBYTE pImage;

    // Check for NAND device availability
    if (!g_bNandExist)
    {
        EdbgOutputDebugString("WARNING: NAND device doesn't exist - unable to store image.\r\n");
        return FALSE;
    }

    // Boot Configuration and Buffer
    if(!NANDBootInit())
    {
        EdbgOutputDebugString("WARNING: NANDBootInit fail\r\n");
        return FALSE;
    }

    if (!FMD_GetInfo(&flashInfo))
    {
        EdbgOutputDebugString("ERROR: Unable to get NAND flash information.\r\n");
        return FALSE;
    }                

	//
	// CS&ZHL MAY-23-2011: the image is buffered in ram, get ram address of image (Get cached image location)
	//
    pImage = OEMMapMemAddr(dwImageStart, dwImageStart);

	if(pNANDWrtImgInfo->dwImgType == IMAGE_NK)
    {
        //dwStartOffset = NANDImageCfg.dwNkOffset;
		//
		// CS&ZHL MAY-23-2011: get nand offset for image written
		//
		dwStartOffset = (DWORD)OALVAtoPA((void *)dwImageStart) - NANDImageCfg.dwNandDevPhyAddr;
        dwImageSize = NANDImageCfg.dwNkSize;         
        RETAILMSG(TRUE, (_T("INFO: Writing NK image from RAM [0x%x] to NAND Offset [0x%x], Length [0x%x]\r\n"),
										(DWORD)pImage, dwStartOffset, dwImageLength));
    }
    else if(pNANDWrtImgInfo->dwImgType == IMAGE_EBOOT)
    {
        dwStartOffset = NANDImageCfg.dwBootOffset;
        dwImageSize = NANDImageCfg.dwBootSize;   
        RETAILMSG(TRUE, (_T("INFO: Writing EBOOT image to NAND (please wait)...\r\n")));            
    }                
    else
    {
        // If image is larger than 4K, this must be xldr.bin produced by ROMIMAGE
        if(dwImageLength > 0x1000)
        {
            pImage += 0x1000;
            dwImageLength -= 0x1000;
        }        
        dwStartOffset = NANDImageCfg.dwXldrOffset;
        dwImageSize = NANDImageCfg.dwXldrSize;
        RETAILMSG(TRUE, (_T("INFO: Writing XLDR image to NAND (please wait)...\r\n")));        
    }

    // Check parameters
    if(dwImageLength > dwImageSize)
    {
        ERRORMSG(TRUE, (_T("length size(0x%x) is differen with expected (0x%x)\r\n"), dwImageLength, dwImageSize));
        return FALSE;
    }    
    
    //To reduce unnecessary programming
    dwValidImageBlock = (dwImageLength + flashInfo.dwBytesPerBlock - 1) / flashInfo.dwBytesPerBlock; 

    // Fill unused space with 0xFF
    memset(pImage + dwImageLength, 0xFF, (dwValidImageBlock * flashInfo.dwBytesPerBlock) - dwImageLength);
    
    // Calculate block range for the image
    StartLogBlkAddr = dwStartOffset / flashInfo.dwBytesPerBlock; 
    EndLogBlkAddr = StartLogBlkAddr + dwValidImageBlock - 1;

    RETAILMSG(TRUE, (_T("INFO: Programming NAND flash blocks [0x%x - 0x%x].\r\n"), StartLogBlkAddr, EndLogBlkAddr));
    
    if(!GetPhyBlkAddr(StartLogBlkAddr, &PhyBlockAddr))
    {
        ERRORMSG(TRUE, (_T("GetGoodPhyBlock failed: 0x%x\r\n"),StartLogBlkAddr)); 
        return FALSE;
    }
    DEBUGMSG(TRUE, (_T("INFO: NAND flash block [0x%x] is a good block.\r\n"), PhyBlockAddr));
 
    do
    {
        if(!WriteSingleBlock(PhyBlockAddr,pImage))
        {
             RETAILMSG(TRUE, (_T("Error: Writing image failed!\r\n")));
             return FALSE;
        }
        pImage += flashInfo.dwBytesPerBlock;

        if(!GetNextGoodBlock(PhyBlockAddr, &PhyBlockAddr))
        {
            EdbgOutputDebugString("Error: No good block found - unable to store image!\r\n");
            return FALSE;
        }

        percentComplete =  100 * (dwValidImageBlock + StartLogBlkAddr - EndLogBlkAddr) / dwValidImageBlock;

        // If percentage complete has changed, show the progress
        if (lastPercentComplete != percentComplete)
        {
            lastPercentComplete = percentComplete;
            //OEMWriteDebugByte('\r');
            //EdbgOutputDebugString("INFO: Programming and Verifying image are %d%% completed.", percentComplete);
			OEMWriteDebugByte('#');
        }
        StartLogBlkAddr++;        
    }while(StartLogBlkAddr <= EndLogBlkAddr);
    EdbgOutputDebugString("\r\n");    

	//
	// CS&ZHL MAY-23-2011: we will save the valid image length into BootCFG, so mask the following code
	//
	/*
    //If ValidEndLogBlkAddr is same with EndLogBlkAddr, then it doesn't make sense to mark valid image length to NAND flash.  
    if(EndLogBlkAddr > ValidEndLogBlkAddr && pNANDWrtImgInfo->dwImgType == IMAGE_NK)
    {
        //Mark valid image length into the end of the image.
        //Fill unused space with 0xFF
        memset(pImage, 0xFF, flashInfo.dwBytesPerBlock);

        // The last dword store the actual image length
        memcpy((char *)(pImage + flashInfo.dwBytesPerBlock - IMAGE_LENGTH_SIZE - IMAGE_LENGTH_MAGIC_STRING_SIZE)\
        ,IMAGE_LENGTH_MAGIC_STRING, IMAGE_LENGTH_MAGIC_STRING_SIZE);
        *(DWORD*)(pImage + flashInfo.dwBytesPerBlock - IMAGE_LENGTH_SIZE) = dwLength;
        
        if(!GetPhyBlkAddr(EndLogBlkAddr, &PhyBlockAddr))
        {
            ERRORMSG(TRUE, (_T("GetGoodPhyBlock failed: 0x%x\r\n"),EndLogBlkAddr)); 
            return FALSE;
        } 
        
        if(!WriteSingleBlock(PhyBlockAddr,pImage))
        {
             RETAILMSG(TRUE, (_T("Error: Writing image length mark failed!\r\n")));
             return FALSE;
        }
    }
	*/
    
    NANDBootReserved();

    EdbgOutputDebugString("INFO: Updating of image completed successfully.\r\n");
    return(TRUE);
}

//-----------------------------------------------------------------------------
//
//  Function:  NANDWriteXldr
//
//  This function writes to NAND flash memory the XLDR image stored 
//  in the RAM file cache area.
//
//  Parameters:
//      dwStartAddr 
//          [in] Address in flash memory where the start of the downloaded 
//          XLDR image is to be written.
//
//      dwLength 
//          [in] Length of the XLDR image, in bytes, to be written to flash
//          memory.            
//
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//-----------------------------------------------------------------------------
BOOL NANDWriteXldr(DWORD dwStartAddr, DWORD dwLength)
{
    //LPBYTE  pImage;  
    NANDWrtImgInfo NANDWrtImgInfo;
    
    NANDWrtImgInfo.dwImgType = IMAGE_XLDR;

    // Get cached image location
    //pImage = OEMMapMemAddr(dwStartAddr, dwStartAddr);

    //return NANDWriteImage(&NANDWrtImgInfo,pImage, dwLength);
    return NANDWriteImage(&NANDWrtImgInfo, dwStartAddr, dwLength);
}


//-----------------------------------------------------------------------------
//
//  Function:  NANDWriteBoot
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
BOOL NANDWriteBoot(DWORD dwStartAddr, DWORD dwLength)
{
    //LPBYTE  pImage;  
    NANDWrtImgInfo NANDWrtImgInfo;
    
    NANDWrtImgInfo.dwImgType = IMAGE_EBOOT;

    // Get cached image location
    //pImage = OEMMapMemAddr(dwStartAddr, dwStartAddr);

    //return NANDWriteImage(&NANDWrtImgInfo, pImage, dwLength);
    return NANDWriteImage(&NANDWrtImgInfo, dwStartAddr, dwLength);
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
BOOL NANDWriteNK(DWORD dwStartAddr, DWORD dwLength)
{
    //LPBYTE  pImage;  
    NANDWrtImgInfo NANDWrtImgInfo;
    
    NANDWrtImgInfo.dwImgType = IMAGE_NK;

    // Get cached image location
    //pImage = OEMMapMemAddr(dwStartAddr, dwStartAddr);
	//EdbgOutputDebugString("NANDWriteNK::Write RAM data [0x%x] to NAND [0x%x], Length = 0x%x\r\n", 
	//										(DWORD)pImage, dwStartAddr, dwLength);

    //return NANDWriteImage(&NANDWrtImgInfo, pImage, dwLength);
    return NANDWriteImage(&NANDWrtImgInfo, dwStartAddr, dwLength);
}


//-----------------------------------------------------------------------------
//
//  Function:  NANDLoadNK
//
//  This function loads an OS image from NAND flash memory into RAM for
//  execution.
//
//  Parameters:
//      dwImageStart:		[in] virtual start address of OS image in NandFlash
//      dwImageLength:	[in] byte length  of OS image
//
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//-----------------------------------------------------------------------------
//BOOL NANDLoadNK(VOID)
BOOL NANDLoadNK(DWORD dwImageStart, DWORD dwImageLength)
{
    LPBYTE				pSectorBuf;
    SectorInfo			sectorInfo;
    SECTOR_ADDR	sectorAddr, startSectorAddr, endSectorAddr;
    UINT32				StartLogBlkAddr = 0;
	UINT32				EndLogBlkAddr = 0;
	UINT32				PhyBlockAddr;
    UINT32				percentComplete, lastPercentComplete=0;    
	UINT32				dwValidImageBlock = 0;
	DWORD				dwMBRStartSector;			// -> BinFS		
	DWORD				dwParttableOffset;			// -> BinFS
	PARTENTRY		sPart = {0};						// -> BinFS
	PPARTENTRY		pPart = &sPart;					// -> BinFS
	DWORD				i;										// -> BinFS
    //UINT32 dwActualLength = 0; 
    //char *pLengthMagic;

    // Check for NAND device availability
    if(!g_bNandExist)
    {
        EdbgOutputDebugString("WARNING: NAND device doesn't exist - unable to read image.\r\n");
        return FALSE;
    }

    // Boot Configuration and Buffer
    if(!NANDBootInit())
    {
        EdbgOutputDebugString("WARNING: NANDBootInit fail\r\n");
        return FALSE;
    }

    EdbgOutputDebugString("INFO: Reading NK image from NAND (please wait)...\r\n");

    if(!FMD_GetInfo(&flashInfo))
    {
        EdbgOutputDebugString("ERROR: Unable to get NAND flash information.\r\n");
        return FALSE;
    }

    // Fill the sectorInfo
    memset(&sectorInfo, 0xFF, sizeof(sectorInfo));

    // Calculate block range for the NK image    
    //StartLogBlkAddr = NANDImageCfg.dwNkOffset / flashInfo.dwBytesPerBlock;
    //EndLogBlkAddr = StartLogBlkAddr + (NANDImageCfg.dwNkSize + flashInfo.dwBytesPerBlock - 1) / flashInfo.dwBytesPerBlock -1;
	//
	// CS&ZHL MAY-24-2011: consider case of BinFS
	//
	EdbgOutputDebugString("INFO: BinFS -> find MBR...\r\n");

	// CS&ZHL MAY-21-2011: use temp buffer
	pSectorBuf = SectorBuffer.pSectorBuf;
	EdbgOutputDebugString("INFO: use RAM buffer at 0x%x as MBR buffer\r\n", (DWORD)pSectorBuf);

	dwMBRStartSector = NANDImageCfg.dwMBROffset / flashInfo.wDataBytesPerSector;
	EdbgOutputDebugString("INFO: Find MBR start from NAND flash sector [0x%x].\r\n", dwMBRStartSector);

	//get a good block to write
	for( ; ; )
	{
		// dwMBRStartSector is block align, Calculate the current block number
		StartLogBlkAddr = dwMBRStartSector / flashInfo.wSectorsPerBlock;

		// Skip bad blocks
		if (FMD_GetBlockStatus(StartLogBlkAddr) == BLOCK_STATUS_BAD)
		{
			EdbgOutputDebugString("INFO: Found bad NAND flash block [0x%x].\r\n", StartLogBlkAddr);
			dwMBRStartSector += flashInfo.wSectorsPerBlock;
			continue;
		}

		// Finally, we find a good block to write
		break;
	}
	EdbgOutputDebugString("INFO: MBR is at NAND flash block [0x%x].\r\n", StartLogBlkAddr);

	// read a sector only!
	if (!FMD_ReadSector(dwMBRStartSector, pSectorBuf, &sectorInfo, 1))
	{
		EdbgOutputDebugString("ERROR: Read sector: [0x%x] failed.\r\n", dwMBRStartSector);
		return FALSE;
	}

	// check the MBR
	if((pSectorBuf[0] == 0xE9) && (pSectorBuf[1] == 0xFD) && (pSectorBuf[2] == 0xFF) && 
		(pSectorBuf[MBR_SIZE - 2] == 0x55) && (pSectorBuf[MBR_SIZE - 1] == 0xAA))
	{
		dwParttableOffset = (MBR_SIZE - SIZE_END_SIG - (sizeof(PARTENTRY) * NUM_PARTS));
		for(i = 0; i < NUM_PARTS; i ++)
		{
			memcpy((void *)pPart, (void *)(&pSectorBuf[dwParttableOffset + sizeof(PARTENTRY) * i]), sizeof(PARTENTRY));
			EdbgOutputDebugString("INFO: Partition %d, File system 0x%x, StartSector 0x%x, TotalSectors 0x%x.\r\n", i, pPart->Part_FileSystem, pPart->Part_StartSector, pPart->Part_TotalSectors);

			if(pPart->Part_FileSystem == PART_RAMIMAGE)
			{
				StartLogBlkAddr += (pPart->Part_StartSector / flashInfo.wSectorsPerBlock);
				dwValidImageBlock = (pPart->Part_TotalSectors / flashInfo.wSectorsPerBlock);
				EndLogBlkAddr = StartLogBlkAddr +  dwValidImageBlock -1;
				break;
			}
		}
	}

	if(dwValidImageBlock == 0)
	{
		EdbgOutputDebugString("INFO: Not found RAMIMAGE info from MBR, use input parameters\r\n");
		//
		// CS&ZHL MAY-23-2011: use input parameters
		//
		StartLogBlkAddr = ((DWORD)OALVAtoPA((void *)dwImageStart) - NANDImageCfg.dwNandDevPhyAddr) / flashInfo.dwBytesPerBlock;
		dwValidImageBlock = (dwImageLength + flashInfo.dwBytesPerBlock - 1) / flashInfo.dwBytesPerBlock;
		EndLogBlkAddr = StartLogBlkAddr +  dwValidImageBlock -1;
	}
    EdbgOutputDebugString("INFO: StartLogBlkAddr=0x%x,  EndLogBlkAddr=0x%x\r\n", StartLogBlkAddr, EndLogBlkAddr);

    // CS&ZHL MAY-23-2011: Set image load address, this is a fixed address = dwLaunchAddr
    pSectorBuf = (LPBYTE) OALPAtoUA(NANDImageCfg.dwNkRamStart);
    EdbgOutputDebugString("INFO: dwNkRamStart=0x%x,  dwImageStart=0x%x\r\n", pSectorBuf, dwImageStart);

	//
	// CS&ZHL MAY-23-2011: we don't use the following way to get the length of OS image anymore
	//
	/*
    if(!GetPhyBlkAddr(EndLogBlkAddr, &PhyBlockAddr))
    {
        EdbgOutputDebugString("Error: No good block found - unable to store image!\r\n");
        return FALSE;
    }

    if (!FMD_ReadSector((PhyBlockAddr+1)*flashInfo.wSectorsPerBlock-1, pSectorBuf, NULL, 1))
    {
        EdbgOutputDebugString("ERROR: Read sector: [0x%x] failed.\r\n", (PhyBlockAddr+1)*flashInfo.wSectorsPerBlock-1);
        return FALSE;
    }    

    dwActualLength = *(DWORD*)(pSectorBuf +  flashInfo.wDataBytesPerSector - IMAGE_LENGTH_SIZE);
    pLengthMagic = (CHAR*)(pSectorBuf +  flashInfo.wDataBytesPerSector - IMAGE_LENGTH_SIZE - IMAGE_LENGTH_MAGIC_STRING_SIZE);
    *(pLengthMagic + IMAGE_LENGTH_MAGIC_STRING_SIZE) = '\0';
    dwValidImageBlock = (dwActualLength + flashInfo.dwBytesPerBlock - 1) / flashInfo.dwBytesPerBlock;

    if ((memcmp(pLengthMagic, IMAGE_LENGTH_MAGIC_STRING, IMAGE_LENGTH_MAGIC_STRING_SIZE) == 0) && 
        (dwActualLength!= 0 &&  dwActualLength <= NANDImageCfg.dwNkSize))
    {
        RETAILMSG(1, (_T("INFO: Valid image length is [0x%x]\r\n"), dwActualLength));
        EndLogBlkAddr = StartLogBlkAddr + (dwActualLength + flashInfo.dwBytesPerBlock - 1) / flashInfo.dwBytesPerBlock;
    }
	*/
        
    if(!GetPhyBlkAddr(StartLogBlkAddr, &PhyBlockAddr))
    {
        EdbgOutputDebugString("Error: No good block found - unable to store image!\r\n");
        return FALSE;
    }

    // Copy NK from NAND flash to RAM
    do
    {
        //EdbgOutputDebugString("Info: reading block [0x%x].\r\n", PhyBlockAddr);

        // Compute sector address based on current physical block
        startSectorAddr = PhyBlockAddr * flashInfo.wSectorsPerBlock;
        endSectorAddr = startSectorAddr + flashInfo.wSectorsPerBlock;
        
        for (sectorAddr = startSectorAddr; sectorAddr < endSectorAddr; sectorAddr++)
        {
            if (!FMD_ReadSector(sectorAddr, pSectorBuf, &sectorInfo, 1))
            {
                EdbgOutputDebugString("ERROR: Read sector: [0x%x] failed.\r\n", sectorAddr);
                return FALSE;
            }

            pSectorBuf += flashInfo.wDataBytesPerSector;
        }

        if(!GetNextGoodBlock(PhyBlockAddr, &PhyBlockAddr))
        {
            EdbgOutputDebugString("Error: No good block found - unable to store image!\r\n");
            return FALSE;
        }

        percentComplete =  100 * (dwValidImageBlock + StartLogBlkAddr - EndLogBlkAddr) / dwValidImageBlock;

        // If percentage complete has changed, show the progress
        if (lastPercentComplete != percentComplete)
        {
            lastPercentComplete = percentComplete;
            //OEMWriteDebugByte('\r');
            //EdbgOutputDebugString("INFO: Loading image is %d%% completed.", percentComplete);
			OEMWriteDebugByte('#');
        }
        
        StartLogBlkAddr++;        
    }while(StartLogBlkAddr <= EndLogBlkAddr);

    EdbgOutputDebugString("\r\nINFO: Loading of NK completed successfully.\r\n");
    
    return(TRUE);

}


//-----------------------------------------------------------------------------
//
//  Function:  BOOL EBOOT_ReadFlash
//
//  CS&ZHL MAY-5-2011: This function read data from NAND flash memory 
//                                     into specified RAM area
//
//  Parameters:
//      dwRAMAddressDst:  [in] pointer of RAM area
//      dwNANDAddressSrc:	[in] nand flash offset
//      dwLen:						[in] the length in byte which need to read
//
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//-----------------------------------------------------------------------------
BOOL EBOOT_ReadFlash(DWORD dwRAMAddressDst, DWORD dwNANDAddressSrc, DWORD dwLen)
{
    LPBYTE			pSectorBuf;
    SectorInfo		sectorInfo;
    UINT32			PhyBlockAddr;
	DWORD			dwCurrentSector;
	DWORD			dwSectorsInBlock;
	DWORD			dwNumSectorsPerChip;

    // Check for NAND device availability
    if(!g_bNandExist)
    {
        EdbgOutputDebugString("WARNING: NAND device doesn't exist - unable to read image.\r\n");
        return FALSE;
    }

    // Boot Configuration and Buffer
    if(!NANDBootInit())
    {
        EdbgOutputDebugString("WARNING: NANDBootInit fail\r\n");
        return FALSE;
    }

    if(!FMD_GetInfo(&flashInfo))
    {
        EdbgOutputDebugString("ERROR: Unable to get NAND flash information.\r\n");
        return FALSE;
    }

	// check sector alignment
	if (dwNANDAddressSrc % flashInfo.wDataBytesPerSector)
	{
		EdbgOutputDebugString("EBOOT_ReadFlash::ERROR: dwNANDAddressSrc must be sector aligned !!\r\n");
		return FALSE;
	}

    EdbgOutputDebugString("EBOOT_ReadFlash: Reading %d bytes  from NAND...\r\n", dwLen);

	// compute start sector and sectors remaining in the first read block
    EdbgOutputDebugString("NandFlashChip: wDataBytesPerSector = 0x%x bytes,  wSectorsPerBlock = 0x%x\r\n", 
										flashInfo.wDataBytesPerSector, flashInfo.wSectorsPerBlock);

	dwCurrentSector = dwNANDAddressSrc / flashInfo.wDataBytesPerSector;
	dwSectorsInBlock = flashInfo.wSectorsPerBlock - (dwCurrentSector % flashInfo.wSectorsPerBlock);
	dwNumSectorsPerChip = (DWORD)flashInfo.dwNumBlocks * flashInfo.wSectorsPerBlock;
    EdbgOutputDebugString("dwStartSector = 0x%x, dwNumSectorsPerChip = 0x%x\r\n", dwCurrentSector, dwNumSectorsPerChip);

    // Fill the sectorInfo
    memset(&sectorInfo, 0xFF, sizeof(sectorInfo));

    // Set data load address
    pSectorBuf = (LPBYTE) dwRAMAddressDst;

	//start to read....
	for( ; ; )				//do {
	{
		// check that we are not out of our device
		if (dwCurrentSector >= dwNumSectorsPerChip)
		{
			return FALSE;
		}

		// Calculate the current block number
		PhyBlockAddr = dwCurrentSector / flashInfo.wSectorsPerBlock;
		if(dwCurrentSector % flashInfo.wSectorsPerBlock)
		{
			PhyBlockAddr++;
		}

		// if block is not valid, move at the begining of the next one
		if(!GetPhyBlkAddr(PhyBlockAddr, &PhyBlockAddr))
		{
			dwCurrentSector += dwSectorsInBlock;
			EdbgOutputDebugString("WARNING: ->Bad Block, dwCurrentSector => [0x%x]\r\n", dwCurrentSector);
		}
		else				// else read it
		{
			while (dwSectorsInBlock && (dwLen >= flashInfo.wDataBytesPerSector))
			{
				if (!FMD_ReadSector(dwCurrentSector, pSectorBuf, &sectorInfo, 1))
				{
					EdbgOutputDebugString("ERROR: Read sector: [0x%x] failed.\r\n", dwCurrentSector);
					return FALSE;
				}

				pSectorBuf += flashInfo.wDataBytesPerSector;
				dwLen -= flashInfo.wDataBytesPerSector;
				dwCurrentSector++;
				dwSectorsInBlock--;
			}
		}

		if (dwLen < flashInfo.wDataBytesPerSector)
		{
			break;
		}
		dwSectorsInBlock = flashInfo.wSectorsPerBlock;
	}																						//while (1);

	if (dwLen > 0)
	{
		EdbgOutputDebugString("Read remaining %d bytes...\r\n", dwLen);
		// less than a sector size wanted, if unlikely we are at the begining of a block
		if (dwSectorsInBlock == 0)
		{
			for(; ; )		//while (TRUE)
			{
				if (dwCurrentSector >= dwNumSectorsPerChip)
				{
					return FALSE;
				}

				// Calculate the current block number
				PhyBlockAddr = dwCurrentSector / flashInfo.wSectorsPerBlock;
				if(dwCurrentSector % flashInfo.wSectorsPerBlock)
				{
					PhyBlockAddr++;
				}

				// if block is not valid, move at the begining of the next one
				if(GetPhyBlkAddr(PhyBlockAddr, &PhyBlockAddr))
				{
					break;
				}
				dwCurrentSector += flashInfo.wSectorsPerBlock;
			}
		}

		// read rest of data which is less than the number of byte of a sector
		if (!FMD_ReadSector(dwCurrentSector, SectorBuffer.pSectorBuf, &sectorInfo, 1))
		{
			EdbgOutputDebugString("ERROR: Read sector: [0x%x] failed.\r\n", dwCurrentSector);
			return FALSE;
		}
		memcpy(pSectorBuf, SectorBuffer.pSectorBuf, dwLen);
	}

    EdbgOutputDebugString("EBOOT_ReadFlash: reading completed successfully.\r\n");
    return(TRUE);
}

//-----------------------------------------------------------------------------
//
//  Function:  BOOL EBOOT_WriteFlash
//
//  CS&ZHL MAY-22-2011: This function write data in RAM to NAND flash
//
//  Parameters:
//      dwRAMAddressSrc:		[in] pointer of RAM area
//      dwNANDAddressDst:	[in] nand flash offset
//      dwLen:							[in] the length in byte which need to write
//
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//-----------------------------------------------------------------------------
BOOL EBOOT_WriteFlash(DWORD dwRAMAddressSrc, DWORD dwNANDAddressDst, DWORD dwLen)
{
    LPBYTE			pSectorBuf;
    SectorInfo		sectorInfo;
    UINT32			PhyBlockAddr;
	DWORD			dwCurrentSector;
	DWORD			dwSectorsInBlock;
	DWORD			dwNumSectorsPerChip;

    // Check for NAND device availability
    if(!g_bNandExist)
    {
        EdbgOutputDebugString("WARNING: NAND device doesn't exist - unable to read image.\r\n");
        return FALSE;
    }

    // Boot Configuration and Buffer
    if(!NANDBootInit())
    {
        EdbgOutputDebugString("WARNING: NANDBootInit fail\r\n");
        return FALSE;
    }

    if(!FMD_GetInfo(&flashInfo))
    {
        EdbgOutputDebugString("ERROR: Unable to get NAND flash information.\r\n");
        return FALSE;
    }

	// check sector alignment
	if (dwNANDAddressDst % flashInfo.wDataBytesPerSector)
	{
		EdbgOutputDebugString("EBOOT_WriteFlash::ERROR: dwNANDAddressDst must be sector aligned !!\r\n");
		return FALSE;
	}

    EdbgOutputDebugString("EBOOT_WriteFlash: Writing %d bytes  to NAND...\r\n", dwLen);

	// compute start sector and sectors remaining in the first read block
    EdbgOutputDebugString("NandFlashChip: wDataBytesPerSector = 0x%x bytes,  wSectorsPerBlock = 0x%x\r\n", 
										flashInfo.wDataBytesPerSector, flashInfo.wSectorsPerBlock);

	dwCurrentSector = dwNANDAddressDst / flashInfo.wDataBytesPerSector;
	dwSectorsInBlock = flashInfo.wSectorsPerBlock - (dwCurrentSector % flashInfo.wSectorsPerBlock);
	dwNumSectorsPerChip = (DWORD)flashInfo.dwNumBlocks * flashInfo.wSectorsPerBlock;
    EdbgOutputDebugString("dwStartSector = 0x%x, dwNumSectorsPerChip = 0x%x\r\n", dwCurrentSector, dwNumSectorsPerChip);

    // Fill the sectorInfo
    memset(&sectorInfo, 0xFF, sizeof(sectorInfo));
    sectorInfo.bOEMReserved &= ~OEM_BLOCK_RESERVED;
    sectorInfo.bOEMReserved &= ~OEM_BLOCK_READONLY;

    // Set data load address
    pSectorBuf = (LPBYTE) dwRAMAddressSrc;

	//start to read....
	for( ; ; )				//do {
	{
		// check that we are not out of our device
		if (dwCurrentSector >= dwNumSectorsPerChip)
		{
			return FALSE;
		}

		// Calculate the current block number
		PhyBlockAddr = dwCurrentSector / flashInfo.wSectorsPerBlock;
		if(dwCurrentSector % flashInfo.wSectorsPerBlock)
		{
			PhyBlockAddr++;
		}

		// if block is not valid, move at the begining of the next one
		if(!GetPhyBlkAddr(PhyBlockAddr, &PhyBlockAddr))
		{
			dwCurrentSector += dwSectorsInBlock;
			EdbgOutputDebugString("WARNING: ->Bad Block, dwCurrentSector => [0x%x]\r\n", dwCurrentSector);
		}
		else				// else read it
		{
			while (dwSectorsInBlock && (dwLen >= flashInfo.wDataBytesPerSector))
			{
				if (!FMD_WriteSector(dwCurrentSector, pSectorBuf, &sectorInfo, 1))
				{
					EdbgOutputDebugString("ERROR: Write sector: [0x%x] failed.\r\n", dwCurrentSector);
					return FALSE;
				}

				pSectorBuf += flashInfo.wDataBytesPerSector;
				dwLen -= flashInfo.wDataBytesPerSector;
				dwCurrentSector++;
				dwSectorsInBlock--;
			}
		}

		if (dwLen < flashInfo.wDataBytesPerSector)
		{
			break;
		}
		dwSectorsInBlock = flashInfo.wSectorsPerBlock;
	}																						//while (1);

	if (dwLen > 0)
	{
		EdbgOutputDebugString("Read remaining %d bytes...\r\n", dwLen);
		// less than a sector size wanted, if unlikely we are at the begining of a block
		if (dwSectorsInBlock == 0)
		{
			for(; ; )		//while (TRUE)
			{
				if (dwCurrentSector >= dwNumSectorsPerChip)
				{
					return FALSE;
				}

				// Calculate the current block number
				PhyBlockAddr = dwCurrentSector / flashInfo.wSectorsPerBlock;
				if(dwCurrentSector % flashInfo.wSectorsPerBlock)
				{
					PhyBlockAddr++;
				}

				// if block is not valid, move at the begining of the next one
				if(GetPhyBlkAddr(PhyBlockAddr, &PhyBlockAddr))
				{
					break;
				}
				dwCurrentSector += flashInfo.wSectorsPerBlock;
			}
		}

		// write rest of data which is less than the number of byte of a sector
		memset(SectorBuffer.pSectorBuf, 0xFF, flashInfo.wDataBytesPerSector);
		memcpy(SectorBuffer.pSectorBuf, pSectorBuf, dwLen);
		if (!FMD_WriteSector(dwCurrentSector, SectorBuffer.pSectorBuf, &sectorInfo, 1))
		{
			EdbgOutputDebugString("ERROR: Write sector: [0x%x] failed.\r\n", dwCurrentSector);
			return FALSE;
		}
	}

    EdbgOutputDebugString("EBOOT_WriteFlash: reading completed successfully.\r\n");
    return(TRUE);
}


//-----------------------------------------------------------------------------
//
//  Function:  BOOL EBOOT_EraseFlash
//
//  CS&ZHL MAY-22-2011: This function erase the specified NAND flash region
//
//  Parameters:
//      dwNANDAddress:	[in] nand flash offset which must be block alignment
//      dwLen:					[in] the length in byte which need to erase
//
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//-----------------------------------------------------------------------------
BOOL EBOOT_EraseFlash(DWORD dwNANDAddress, DWORD dwLen)
{
	DWORD			dwStartBlock;
	DWORD			dwEndBlock;
	DWORD			dwBlockID;

    // Check for NAND device availability
    if(!g_bNandExist)
    {
        EdbgOutputDebugString("WARNING: NAND device doesn't exist - unable to read image.\r\n");
        return FALSE;
    }

    // Boot Configuration and Buffer
    if(!NANDBootInit())
    {
        EdbgOutputDebugString("WARNING: NANDBootInit fail\r\n");
        return FALSE;
    }

    if(!FMD_GetInfo(&flashInfo))
    {
        EdbgOutputDebugString("ERROR: Unable to get NAND flash information.\r\n");
        return FALSE;
    }

	// check block alignment
	if (dwNANDAddress % flashInfo.dwBytesPerBlock)
	{
		EdbgOutputDebugString("EBOOT_EraseFlash::ERROR: dwNANDAddress must be block aligned !!\r\n");
		return FALSE;
	}

	dwStartBlock = dwNANDAddress / flashInfo.dwBytesPerBlock;
	dwEndBlock = dwStartBlock + (dwLen / flashInfo.dwBytesPerBlock) - 1;
	if(dwLen % flashInfo.dwBytesPerBlock)
	{
		dwEndBlock++;
	}
    EdbgOutputDebugString("EBOOT_EraseFlash: Erasing from startBlock [0x%x] to endBlock [0x%x]\r\n", dwStartBlock, dwEndBlock);

    for (dwBlockID = dwStartBlock; dwBlockID <= dwEndBlock; dwBlockID++)
    {
        // Skip bad blocks
        if (FMD_GetBlockStatus(dwBlockID) == BLOCK_STATUS_BAD)
        {
            EdbgOutputDebugString("INFO: Found bad NAND flash block [0x%x].\r\n", dwBlockID);
			dwEndBlock++;
            continue;
        }

        // Erase the block...
        if (!FMD_EraseBlock(dwBlockID))
        {
            EdbgOutputDebugString("ERROR: Unable to erase NAND flash block [0x%x].\r\n", dwBlockID);
            return(FALSE);
        }
    }

    EdbgOutputDebugString("EBOOT_EraseFlash: erasing completed successfully.\r\n");
    return(TRUE);
}



//------------------------------------------------------------------------------
//
//  Function:  NANDMakeMBR
//
//  CS&ZHL MAY-21-2011: save start_offset and length of multi-bin images for BinFS
//
//	Conditions:
//		g_dwCurrentBinNum = 1;
//		g_dwTotalBinNum >= 1;
//
//  Parameters:
//      None.
//
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//-----------------------------------------------------------------------------
BOOL NANDMakeMBR(void)
{
    LPBYTE			pSectorBuf;					//, pImage;
	DWORD			dwMBRStartSector;
    BLOCK_ID		blockID;
    SectorInfo		sectorInfo;
	DWORD			dwRealDataSize;
	PARTENTRY	sPart = {0};
	DWORD			dwParttableOffset;
	CHSAddr		chsStartAddr;
	CHSAddr		chsEndAddr;
	LBAAddr		lbaStartAddr;
	LBAAddr		lbaEndAddr;

    // Check for NAND device availability
    //
    if (!g_bNandExist)
    {
        EdbgOutputDebugString("WARNING: NAND device doesn't exist - unable to store image.\r\n");
        return(FALSE);
    }

    // Boot Configuration and Buffer
    if(!NANDBootInit())
    {
        EdbgOutputDebugString("WARNING: NANDBootInit fail\r\n");
        return FALSE;
    }

    EdbgOutputDebugString("INFO: Writing MBR image to NAND (please wait)...\r\n");

    if (!FMD_GetInfo(&flashInfo))
    {
        EdbgOutputDebugString("ERROR: Unable to get NAND flash information.\r\n");
        return(FALSE);
    }

	dwMBRStartSector = NANDImageCfg.dwMBROffset / flashInfo.wDataBytesPerSector;
    EdbgOutputDebugString("INFO: Write MBR to NAND flash secttor [0x%x].\r\n", dwMBRStartSector);

	// CS&ZHL MAY-21-2011: use temp buffer
	pSectorBuf = SectorBuffer.pSectorBuf;
    EdbgOutputDebugString("INFO: use RAM buffer at 0x%x to make MBR\r\n", (DWORD)pSectorBuf);

	// Create MBR
	memset(pSectorBuf, 0xff, flashInfo.wDataBytesPerSector);
	pSectorBuf[0] = 0xE9;
	pSectorBuf[1] = 0xFD;
	pSectorBuf[2] = 0xFF;
	pSectorBuf[MBR_SIZE - 2] = 0x55;
	pSectorBuf[MBR_SIZE - 1] = 0xAA;

	// Partition table
	dwParttableOffset = (MBR_SIZE - SIZE_END_SIG - (sizeof(PARTENTRY) * NUM_PARTS));
	memset(&(pSectorBuf[dwParttableOffset]), 0, sizeof(PARTENTRY) * NUM_PARTS);

	// First partition
	sPart.Part_BootInd = (PART_IND_ACTIVE | PART_IND_READ_ONLY);
	sPart.Part_FileSystem = PART_RAMIMAGE;
	sPart.Part_StartSector = flashInfo.wSectorsPerBlock * (NANDImageCfg.dwMBRSize / flashInfo.dwBytesPerBlock);		//????

	// Get data size
	if(g_dwTotalBinNum > 1)
	{
		// Should include chain.bin
		dwRealDataSize = g_dwBinAddress[1] - g_dwBinAddress[0];
	}
	else
	{
		dwRealDataSize = g_dwBinLength[g_dwCurrentBinNum - 1];			//g_dwCurrentBinNum = 1
	}
	// Align real data size in block.
	if((dwRealDataSize % flashInfo.dwBytesPerBlock) != 0)
	{
		dwRealDataSize += (flashInfo.dwBytesPerBlock - (dwRealDataSize % flashInfo.dwBytesPerBlock));
	}

	sPart.Part_TotalSectors = dwRealDataSize / flashInfo.wDataBytesPerSector;

	// logical block addresses for the first and final sector (start on the second head)
	lbaStartAddr = sPart.Part_StartSector;
	lbaEndAddr = sPart.Part_StartSector + sPart.Part_TotalSectors - 1;

	// translate the LBA addresses to CHS addresses
	chsStartAddr = LBAtoCHS(&flashInfo, lbaStartAddr);
	chsEndAddr = LBAtoCHS(&flashInfo, lbaEndAddr);

	// starting address
	sPart.Part_FirstTrack = (BYTE)(chsStartAddr.cylinder & 0xFF);
	sPart.Part_FirstHead = (BYTE)(chsStartAddr.head & 0xFF);
	// lower 6-bits == sector, upper 2-bits = cylinder upper 2-bits of 10-bit cylinder #
	sPart.Part_FirstSector = (BYTE)((chsStartAddr.sector & 0x3F) | ((chsStartAddr.cylinder & 0x0300) >> 2));

	// ending address:
	sPart.Part_LastTrack = (BYTE)(chsEndAddr.cylinder & 0xFF);
	sPart.Part_LastHead = (BYTE)(chsEndAddr.head & 0xFF);
	// lower 6-bits == sector, upper 2-bits = cylinder upper 2-bits of 10-bit cylinder #
	sPart.Part_LastSector = (BYTE)((chsEndAddr.sector & 0x3F) | ((chsEndAddr.cylinder & 0x0300) >> 2));

    EdbgOutputDebugString("INFO: Created partition: \r\n");
    EdbgOutputDebugString("INFO: Part_BootInd = 0x%x.\r\n", sPart.Part_BootInd);
    EdbgOutputDebugString("INFO: Part_FileSystem = 0x%x.\r\n", sPart.Part_FileSystem);
    EdbgOutputDebugString("INFO: Part_StartSector = 0x%x.\r\n", sPart.Part_StartSector);
    EdbgOutputDebugString("INFO: Part_TotalSectors = 0x%x.\r\n", sPart.Part_TotalSectors);

	memcpy(&(pSectorBuf[dwParttableOffset]), &sPart, sizeof(PARTENTRY));
	
	if(g_dwTotalBinNum > 1)
	{
		// For Partition 2
		sPart.Part_BootInd = PART_IND_READ_ONLY;
		sPart.Part_FileSystem = PART_BINFS;
		sPart.Part_StartSector += sPart.Part_TotalSectors;

		dwRealDataSize = g_dwBinAddress[g_dwTotalBinNum - 2] - g_dwBinAddress[1] + g_dwBinLength[g_dwTotalBinNum - 2];
		if((dwRealDataSize % flashInfo.dwBytesPerBlock) != 0)
			dwRealDataSize += (flashInfo.dwBytesPerBlock - (dwRealDataSize % flashInfo.dwBytesPerBlock));

		sPart.Part_TotalSectors = dwRealDataSize / flashInfo.wDataBytesPerSector;

		// logical block addresses for the first and final sector (start on the second head)
		lbaStartAddr = sPart.Part_StartSector;
		lbaEndAddr = sPart.Part_StartSector + sPart.Part_TotalSectors - 1;

		// translate the LBA addresses to CHS addresses
		chsStartAddr = LBAtoCHS(&flashInfo, lbaStartAddr);
		chsEndAddr = LBAtoCHS(&flashInfo, lbaEndAddr);

		// starting address
		sPart.Part_FirstTrack = (BYTE)(chsStartAddr.cylinder & 0xFF);
		sPart.Part_FirstHead = (BYTE)(chsStartAddr.head & 0xFF);
		// lower 6-bits == sector, upper 2-bits = cylinder upper 2-bits of 10-bit cylinder #
		sPart.Part_FirstSector = (BYTE)((chsStartAddr.sector & 0x3F) | ((chsStartAddr.cylinder & 0x0300) >> 2));

		// ending address:
		sPart.Part_LastTrack = (BYTE)(chsEndAddr.cylinder & 0xFF);
		sPart.Part_LastHead = (BYTE)(chsEndAddr.head & 0xFF);
		// lower 6-bits == sector, upper 2-bits = cylinder upper 2-bits of 10-bit cylinder #
		sPart.Part_LastSector = (BYTE)((chsEndAddr.sector & 0x3F) | ((chsEndAddr.cylinder & 0x0300) >> 2));

	    EdbgOutputDebugString("INFO: Created partition: \r\n");
	    EdbgOutputDebugString("INFO: Part_BootInd = 0x%x.\r\n", sPart.Part_BootInd);
	    EdbgOutputDebugString("INFO: Part_FileSystem = 0x%x.\r\n", sPart.Part_FileSystem);
	    EdbgOutputDebugString("INFO: Part_StartSector = 0x%x.\r\n", sPart.Part_StartSector);
	    EdbgOutputDebugString("INFO: Part_TotalSectors = 0x%x.\r\n", sPart.Part_TotalSectors);

		memcpy(&(pSectorBuf[dwParttableOffset + sizeof(PARTENTRY)]), &sPart, sizeof(PARTENTRY));
	}

    memset(&sectorInfo, 0xFF, sizeof(sectorInfo));
    sectorInfo.bOEMReserved &= ~OEM_BLOCK_RESERVED;
    sectorInfo.bOEMReserved &= ~OEM_BLOCK_READONLY;

	//get a good block to write
	for( ; ; )
	{
		// dwMBRStartSector is block align, Calculate the current block number
		blockID = dwMBRStartSector / flashInfo.wSectorsPerBlock;

        // Skip bad blocks
        if (FMD_GetBlockStatus(blockID) == BLOCK_STATUS_BAD)
        {
            EdbgOutputDebugString("INFO: Found bad NAND flash block [0x%x].\r\n", blockID);
			dwMBRStartSector += flashInfo.wSectorsPerBlock;
            continue;
        }

        // Erase the block...
        if (!FMD_EraseBlock(blockID))
        {
            EdbgOutputDebugString("ERROR: Unable to erase NAND flash block [0x%x].\r\n", blockID);
            return(FALSE);
        }

		// Finally, we find a good block to write
		break;
	}
    EdbgOutputDebugString("INFO: Write MBR to NAND flash block [0x%x].\r\n", blockID);

	// write a sector only!
	if (!FMD_WriteSector(dwMBRStartSector, pSectorBuf, &sectorInfo, 1))
	{
		EdbgOutputDebugString("ERROR: Write sector: [0x%x] failed.\r\n", dwMBRStartSector);
		return FALSE;
	}

    EdbgOutputDebugString("INFO: Update of MBR completed successfully.\r\n");
    return(TRUE);
}

//------------------------------------------------------------------------------
//
//  Function:  NANDLoadBootCFG
//
//  Retrieves bootloader configuration information (menu settings, etc.) from 
//  the NAND flash.
//
//  Parameters:
//      eBootCFG 
//          [out] Points to bootloader configuration that will be filled with
//          loaded data. 
//
//      cbBootCfgSize
//          [in] Size in bytes of the bootloader configuration.
//
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//-----------------------------------------------------------------------------
BOOL NANDLoadBootCFG(BYTE *pBootCfg, DWORD cbBootCfgSize)
{
    SectorInfo sectorInfo;
    UINT32 PhyBlockAddr;

    if(!g_bNandExist)
    {
        EdbgOutputDebugString("WARNING: NAND device doesn't exist - unable to read image.\r\n");
        return FALSE;
    }
    
    // Boot Configuration and Buffer
    if(!NANDBootInit())
    {
        EdbgOutputDebugString("WARNING: NANDBootInit fail\r\n");
        return FALSE;
    }
    
    if(!FMD_GetInfo(&flashInfo))
    {
        EdbgOutputDebugString("ERROR: Unable to get NAND flash information.\r\n");
        return FALSE;
    }

    // Calculate the physical block range for the boot configuration
    EdbgOutputDebugString("INFO: Loading boot configuration from NAND\r\n");  

    if(!GetPhyBlkAddr(NANDImageCfg.dwCfgOffset / flashInfo.dwBytesPerBlock, &PhyBlockAddr))
    {
        EdbgOutputDebugString("Error: No good block found!\r\n");
        return FALSE;
    }   
    
    if (!FMD_ReadSector(PhyBlockAddr*flashInfo.wSectorsPerBlock, SectorBuffer.pSectorBuf, &sectorInfo, 1))
    {
        EdbgOutputDebugString("WARNING: Failed to Load configuration data.\r\n");
        return FALSE;
    }

    if(cbBootCfgSize > flashInfo.wDataBytesPerSector)
    {
        cbBootCfgSize = flashInfo.wDataBytesPerSector;
    }
    memcpy(pBootCfg, SectorBuffer.pSectorBuf, cbBootCfgSize);   

    return TRUE;
}


//------------------------------------------------------------------------------
//
//  Function:  NANDStoreBootCFG
//
//  Stores bootloader configuration information (menu settings, etc.) to 
//  the NAND flash.
//
//  Parameters:
//      eBootCFG 
//          [out] Points to bootloader configuration that will be stored.
//
//      cbBootCfgSize
//          [in] Size in bytes of the bootloader configuration.
//
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//-----------------------------------------------------------------------------
BOOL NANDStoreBootCFG(BYTE *pBootCfg, DWORD cbBootCfgSize)
{
    SectorInfo sectorInfo;
    UINT32 PhyBlockAddr;

    // Check for NAND device availability
    if (!g_bNandExist)
    {
        EdbgOutputDebugString("WARNING: NAND device doesn't exist - unable to store image.\r\n");
        return FALSE;
    }

    // Boot Configuration and Buffer
    if(!NANDBootInit())
    {
        EdbgOutputDebugString("WARNING: NANDBootInit fail\r\n");
        return FALSE;
    }

    if (!FMD_GetInfo(&flashInfo))
    {
        EdbgOutputDebugString("ERROR: Unable to get NAND flash information.\r\n");
        return FALSE;
    }

    if(cbBootCfgSize > flashInfo.wDataBytesPerSector)
    {
        cbBootCfgSize = flashInfo.wDataBytesPerSector;
    }

    memcpy(SectorBuffer.pSectorBuf, pBootCfg, cbBootCfgSize);
    memset(SectorBuffer.pSectorBuf + cbBootCfgSize, 0xFF, flashInfo.wDataBytesPerSector - cbBootCfgSize);

    // Fill the sectorInfo
    memset(&sectorInfo, 0xFF, sizeof(sectorInfo));

    // Set Reserved
    sectorInfo.bOEMReserved &= ~OEM_BLOCK_RESERVED;   

    if(!GetPhyBlkAddr(NANDImageCfg.dwCfgOffset / flashInfo.dwBytesPerBlock, &PhyBlockAddr))
    {
        EdbgOutputDebugString("Error: No good block found - unable to store boot configuration!\r\n");
        return FALSE;
    }   

    EdbgOutputDebugString("INFO: Storing boot configuration to NAND block [0x%x].\r\n", NANDImageCfg.dwCfgOffset / flashInfo.dwBytesPerBlock);  

    if (!FMD_EraseBlock(PhyBlockAddr))
    {
        FMD_SetBlockStatus(PhyBlockAddr, BLOCK_STATUS_BAD);
        EdbgOutputDebugString("ERROR: Unable to erase NAND flash block [0x%x].\r\n", PhyBlockAddr);
        return FALSE;
    }
    
    if (!FMD_WriteSector(PhyBlockAddr*flashInfo.wSectorsPerBlock, SectorBuffer.pSectorBuf, &sectorInfo, 1))
    {
        EdbgOutputDebugString("WARNING: Failed to write boot configuration data.\r\n");
        return FALSE;
    }

    EdbgOutputDebugString("INFO: Successfully store boot configuration to NAND!\r\n");
 
    return(TRUE);
}


//-----------------------------------------------------------------------------
//
//  Function:  NANDFormatNK
//
//  This function formats (erases) the NAND flash region reserved for OS 
//  images.
//
//  Parameters:
//      None.     
//
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//-----------------------------------------------------------------------------
BOOL NANDFormatNK(void)
{
    UINT32 StartLogBlkAddr,EndLogBlkAddr,PhyBlockAddr;

    // Get NAND flash data bytes per sector.
    if(!g_bNandExist)
    {
        EdbgOutputDebugString("WARNING: NAND device doesn't exist - unable to read image.\r\n");
        return FALSE;
    }
    
    // Boot Configuration and Buffer
    if(!NANDBootInit())
    {
        EdbgOutputDebugString("WARNING: NANDBootInit fail\r\n");
        return FALSE;
    }
    
    // Get NAND flash data bytes per sector.
    if(!FMD_GetInfo(&flashInfo))
    {
        EdbgOutputDebugString("ERROR: Unable to get NAND flash information.\r\n");
        return FALSE;
    }

    EdbgOutputDebugString("INFO: Starting format of NAND NK region.\r\n");

    // Calculate block range for the NK image
    StartLogBlkAddr = NANDImageCfg.dwNkOffset / flashInfo.dwBytesPerBlock;
    EndLogBlkAddr   = StartLogBlkAddr + (NANDImageCfg.dwNkSize + flashInfo.dwBytesPerBlock - 1) / flashInfo.dwBytesPerBlock;

    while(StartLogBlkAddr < EndLogBlkAddr)
    {
        if(!GetPhyBlkAddr(StartLogBlkAddr, &PhyBlockAddr))
        {
            EdbgOutputDebugString("Error: No good block found!\r\n");
            return FALSE;
        }    
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
        StartLogBlkAddr++;
    }

    EdbgOutputDebugString("\r\nINFO: Format of NAND NK region completed successfully.\r\n");

    
    return(TRUE);
}


//------------------------------------------------------------------------------
//
//  Function:  NANDWriteIPL
//
//  N/A
//
//  Parameters:
//      None.
//
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//-----------------------------------------------------------------------------
BOOL NANDWriteIPL(DWORD dwStartAddr, DWORD dwLength)
{
    LPBYTE pSectorBuf, pImage,pVerSectorBuf;
    SectorInfo sectorInfo,VersectorInfo;
    SECTOR_ADDR sectorAddr, startSectorAddr, endSectorAddr;
    UINT32 StartLogBlkAddr,EndLogBlkAddr,PhyBlockAddr,ValidEndLogBlkAddr;
    UINT32 dwValidImageLength;
    UINT32 i=0;

    // Check for NAND device availability
    //
    if(!g_bNandExist)
    {
        EdbgOutputDebugString("WARNING: NAND device doesn't exist - unable to store image.\r\n");
        return FALSE;
    }

    // Boot Configuration and Buffer
    if(!NANDBootInit())
    {
        EdbgOutputDebugString("WARNING: NANDBootInit fail\r\n");
        return FALSE;
    }

    EdbgOutputDebugString("INFO: Writing IPL image to NAND (please wait)...\r\n");

    if(!FMD_GetInfo(&flashInfo))
    {
        EdbgOutputDebugString("ERROR: Unable to get NAND flash information.\r\n");
        return FALSE;
    }

    // Make sure IPL length does not exceed reserved NAND size
    if(dwLength > NANDImageCfg.dwIplSize * flashInfo.dwBytesPerBlock)
    {
        EdbgOutputDebugString("ERROR: IPL size exceeds reserved NAND region (size = 0x%x)\r\n", dwLength);
        return FALSE;
    }

    // Get cached image location
    pImage = OEMMapMemAddr(dwStartAddr, dwStartAddr);

    EdbgOutputDebugString("INFO: Programming IPL image from flash cache address 0x%x, size = %d\r\n", pImage, dwLength);

    //To reduce unnecessary programming
    dwValidImageLength = ((dwLength + flashInfo.dwBytesPerBlock - 1) / flashInfo.dwBytesPerBlock)*flashInfo.dwBytesPerBlock;

    // Write EBOOT to NAND flash
    pSectorBuf = pImage;    
    pVerSectorBuf = pImage + dwValidImageLength;

    // Fill unused space with 0xFF
    memset(pSectorBuf + dwLength, 0xFF, (dwValidImageLength) - dwLength);

    // Fill the sectorInfo
    memset(&sectorInfo, 0xFF, sizeof(sectorInfo));

    // Set Reserved and image flag
    sectorInfo.bOEMReserved &= ~OEM_BLOCK_RESERVED;   

    // Calculate block range for the  image
    StartLogBlkAddr = NANDImageCfg.dwIplOffset / flashInfo.dwBytesPerBlock; 
    ValidEndLogBlkAddr   = StartLogBlkAddr + dwValidImageLength/flashInfo.dwBytesPerBlock;
    EndLogBlkAddr = StartLogBlkAddr + NANDImageCfg.dwIplSize / flashInfo.dwBytesPerBlock;

    if(!GetPhyBlkAddr(StartLogBlkAddr, &PhyBlockAddr))
    {
        EdbgOutputDebugString("Error: No good block found - unable to store image!\r\n");
        return FALSE;
    }
    
    do
    {
        //EdbgOutputDebugString("Info: block [0x%x] is chosen to store image.\r\n", PhyBlockAddr);
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
                EdbgOutputDebugString("ERROR: Failed to update image.\r\n");
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

        if(!GetNextGoodBlock(PhyBlockAddr, &PhyBlockAddr))
        {
            EdbgOutputDebugString("Error: No good block found - unable to store image!\r\n");
            return FALSE;
        }
        
        StartLogBlkAddr++;
    }while(StartLogBlkAddr < ValidEndLogBlkAddr);

    //Mark all blocks as reserved.
    while(i < EndLogBlkAddr-ValidEndLogBlkAddr)
    {
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

        if(!GetNextGoodBlock(PhyBlockAddr, &PhyBlockAddr))
        {
            EdbgOutputDebugString("Error: No good block found - unable to store image!\r\n");
            return FALSE;
        }
        i++;
    }

    EdbgOutputDebugString("INFO: Verifying image.\r\n");

    if(memcmp(pImage, pImage + (NANDImageCfg.dwIplSize * flashInfo.dwBytesPerBlock), (NANDImageCfg.dwIplSize * flashInfo.dwBytesPerBlock)) != 0)
    {
        EdbgOutputDebugString("ERROR: Failed to verify IPL.\r\n");
    }
    
    EdbgOutputDebugString("INFO: Update of IPL completed successfully.\r\n");

    return TRUE;
}


//------------------------------------------------------------------------------
//
//  Function:  IsSectorEmpty
//
//  N/A
//
//  Parameters:
//      None.
//
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//-----------------------------------------------------------------------------
BOOL IsSectorEmpty(UCHAR *pData, ULONG sectorSize, SectorInfo *pSectorInfo)
{
    BOOL rc = FALSE;
    ULONG idx;

    if (pSectorInfo->dwReserved1 != 0xFFFFFFFF) goto cleanUp;
    if (pSectorInfo->wReserved2 != 0xFFFF) goto cleanUp;
    if (pSectorInfo->bOEMReserved != 0xFF) goto cleanUp;

    for (idx = 0; idx < sectorSize; idx++)
    {
        if (pData[idx] != 0xFF) 
            goto cleanUp;
    }      

    rc = TRUE;

cleanUp:
    return rc;
}


//------------------------------------------------------------------------------
//
//  Function:  NANDStartWriteBinDIO
//
//  N/A
//
//  Parameters:
//      None.
//
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//-----------------------------------------------------------------------------
BOOL NANDStartWriteBinDIO(DWORD dwStartAddr, DWORD dwLength)
{
    UINT32 blockID, startBlockID, endBlockID,PhyBlockAddr;
    UINT32 percentComplete, lastPercentComplete = 0;
    
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(dwStartAddr);

    // Check for NAND device availability
    //
    if (!g_bNandExist)
    {
        EdbgOutputDebugString("WARNING: NAND device doesn't exist - unable to store image.\r\n");
        return FALSE;
    }

    // Boot Configuration and Buffer
    if(!NANDBootInit())
    {
        EdbgOutputDebugString("WARNING: NANDBootInit fail\r\n");
        return FALSE;
    }

    if (!FMD_GetInfo(&flashInfo))
    {
        EdbgOutputDebugString("ERROR: Unable to get NAND flash information.\r\n");
        return FALSE;
    }

    // Make sure DIO image length does not exceed reserved NAND size
    if (dwLength > NANDImageCfg.dwDioSize)
    {
        EdbgOutputDebugString("ERROR: DIO image size exceeds reserved NAND region (size = 0x%x)\r\n", dwLength);
        return FALSE;
    }

    // Force all blocks in XLDR(128KB), EBOOT(256KB) and IPL(256KB) to be reserved
    startBlockID = 0;
    endBlockID = (NANDImageCfg.dwDioOffset / flashInfo.dwBytesPerBlock) - 1;

    // Calculate the physical block range for the DIO image
    startBlockID = (NANDImageCfg.dwDioOffset / flashInfo.dwBytesPerBlock);
    endBlockID = startBlockID + (NANDImageCfg.dwDioSize + flashInfo.dwBytesPerBlock - 1) / flashInfo.dwBytesPerBlock;

    EdbgOutputDebugString("INFO: Erasing NAND flash blocks [0x%x - 0x%x].\r\n", startBlockID, endBlockID);
    
    // Erase range of NAND blocks reserved for DIO image
    for (blockID = startBlockID; blockID < endBlockID; blockID++)
    {
        // Skip bad blocks
        if(!GetPhyBlkAddr(blockID, &PhyBlockAddr))
        {
            EdbgOutputDebugString("Error: No good block found - unable to store image!\r\n");
            return FALSE;
        }

        // Erase the block...
        if (!FMD_EraseBlock(PhyBlockAddr))
        {
            FMD_SetBlockStatus(PhyBlockAddr, BLOCK_STATUS_BAD);
            if(!(FMD_GetBlockStatus(PhyBlockAddr) & BLOCK_STATUS_BAD))
            {
                EdbgOutputDebugString("ERROR: Unable to erase NAND flash block [0x%x].\r\n", blockID);
                return FALSE;
            }
            else
            {
                continue;
            }
        }

        percentComplete = 100 * (blockID - startBlockID + 1) / (endBlockID - startBlockID);
        // If percentage complete has changed, show the progress
        if (lastPercentComplete != percentComplete)
        {
            lastPercentComplete = percentComplete;
            OEMWriteDebugByte('\r');
            EdbgOutputDebugString("INFO: Erase is %d%% complete.", percentComplete);
        }

    }

    EdbgOutputDebugString("\r\nINFO: Ready to write DIO image ... \r\n");
    
    return(TRUE);
}


//------------------------------------------------------------------------------
//
//  Function:  NANDContinueWriteBinDIO
//
//  N/A
//
//  Parameters:
//      None.
//
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//-----------------------------------------------------------------------------
BOOL NANDContinueWriteBinDIO(DWORD dwAddress, BYTE *pbData, DWORD dwSize)
{
    BOOL bOK;
    UINT32 blockID,PhyBlockAddr;
    SECTOR_ADDR sectorAddr;
    SectorInfo *pSectorInfo;
    UINT32 blockSize, sectorSize;
    UINT32 dwCount;
    static UINT32 nTimes = 0, dwBadBlock = 0;

    // Check for NAND device availability
    //
    if(!g_bNandExist)
    {
        EdbgOutputDebugString("WARNING: NAND device doesn't exist - unable to store image.\r\n");
        return FALSE;
    }

    // Boot Configuration and Buffer
    if(!NANDBootInit())
    {
        EdbgOutputDebugString("WARNING: NANDBootInit fail\r\n");
        return FALSE;
    }

    if(!FMD_GetInfo(&flashInfo))
    {
        EdbgOutputDebugString("ERROR: Unable to get NAND flash information.\r\n");
        return FALSE;
    }

    // First we need to calculate position where to write
    sectorSize = flashInfo.wDataBytesPerSector + sizeof(SectorInfo);
    blockSize = flashInfo.wSectorsPerBlock * sectorSize;
    blockID = dwAddress / blockSize;
    sectorAddr = (dwAddress - blockID * blockSize) / sectorSize;

    // Shift block by DIO image region base
    blockID += (NANDImageCfg.dwDioOffset / flashInfo.dwBytesPerBlock);

    // Skip the number of bad blocks
    blockID += dwBadBlock;

    // Write record
    dwCount = 0;
    while (dwCount < dwSize && blockID < flashInfo.dwNumBlocks) 
    {
        // Skip bad blocks
        if(!GetPhyBlkAddr(blockID, &PhyBlockAddr))
        {
            EdbgOutputDebugString("Error: No good block found - unable to store image!\r\n");
            return FALSE;
        }
  
        // Write sectors
        bOK = TRUE;
        while (sectorAddr < flashInfo.wSectorsPerBlock && dwCount < dwSize) 
        {
            // First we have to check for empty sectors
            pSectorInfo = (SectorInfo *)(pbData + dwCount + flashInfo.wDataBytesPerSector);

            // Don't write empty sector
            if (IsSectorEmpty(pbData + dwCount, flashInfo.wDataBytesPerSector, pSectorInfo))
            {
                // EdbgOutputDebugString("INFO: Skipping empty sector [0x%x].\r\n", sectorAddr);
                
                // Move to next sector
                dwCount += sectorSize;
                sectorAddr++;
                continue;
            }

            // Clear reserved flag if set
            pSectorInfo->bOEMReserved |= OEM_BLOCK_RESERVED;

            // Never ever write sector with bad block
            if (pSectorInfo->bBadBlock != 0xFF)
            {
                EdbgOutputDebugString("ERROR: Incorrect or corrupted DIO BIN file - bad block flag set.\r\n");
                return FALSE;
            }

            // Write sector
            if ((bOK = FMD_WriteSector(PhyBlockAddr * flashInfo.wSectorsPerBlock + sectorAddr, 
                pbData + dwCount, pSectorInfo, 1)) == 0)
            {
                EdbgOutputDebugString("ERROR: Writing sector [0x%x] failed.\r\n", sectorAddr);
                break;
            }

            // Move to next sector
            dwCount += sectorSize;
            sectorAddr++;
        }

        // When sector write failed, mark block as bad and move back
        if (!bOK) 
        {
            EdbgOutputDebugString("WARN: Block [0x%x] / sector [0x%x] write failed, mark block as bad.\r\n", blockID, sectorAddr);

            // First move back
            dwCount -= sectorAddr * sectorSize;
            // Mark block as bad
            FMD_SetBlockStatus(blockID, BLOCK_STATUS_BAD);
            blockID++;
            continue;
        }

        // We are done with block
        sectorAddr = 0;
        blockID++;
    }

    // Progress
    OEMWriteDebugByte('\r');
    switch (nTimes++ % 4)
    {
        case 0:
            OEMWriteDebugByte('\\');
            break;
                
        case 1:
            OEMWriteDebugByte('|');
            break;
                
        case 2:
            OEMWriteDebugByte('/');
            break;

        case 3:
            OEMWriteDebugByte('-');
    }

    // Before we leave erase buffer
    memset(pbData, 0xFF, dwSize);

    // If we wrote all, we are succesfull
    return (dwCount >= dwSize);
}


//------------------------------------------------------------------------------
//
//  Function:  NANDFinishWriteBinDIO
//
//  N/A
//
//  Parameters:
//      None.
//
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//-----------------------------------------------------------------------------
BOOL NANDFinishWriteBinDIO()
{
    if (!g_bNandExist)
    {
        EdbgOutputDebugString("\r\nINFO: No NAND present!\r\n");
        return FALSE;
    }
    
    // Nothing to do...
    EdbgOutputDebugString("\r\nINFO: Update of DIO image completed successfully.\r\n");

    return TRUE;
}


//------------------------------------------------------------------------------
//
//  Function:  NANDLoadIPL
//
//  N/A
//
//  Parameters:
//      None.
//
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//-----------------------------------------------------------------------------
BOOL NANDLoadIPL(VOID)
{
    LPBYTE pSectorBuf;
    SectorInfo sectorInfo;
    SECTOR_ADDR sectorAddr, startSectorAddr, endSectorAddr;
    UINT32 StartLogBlkAddr,EndLogBlkAddr,PhyBlockAddr;

    // Check for NAND device availability
    //
    if(!g_bNandExist)
    {
        EdbgOutputDebugString("WARNING: NAND device doesn't exist - unable to read image.\r\n");
        return FALSE;
    }

    // Boot Configuration and Buffer
    if(!NANDBootInit())
    {
        EdbgOutputDebugString("WARNING: NANDBootInit fail\r\n");
        return FALSE;
    }

    EdbgOutputDebugString("INFO: Reading IPL image from NAND (please wait)...\r\n");

    if(!FMD_GetInfo(&flashInfo))
    {
        EdbgOutputDebugString("ERROR: Unable to get NAND flash information.\r\n");
        return FALSE;
    }
  
    // Set image load address
    pSectorBuf = (LPBYTE)OALPAtoUA(NANDImageCfg.dwIplRamStart);

    // Calculate block range for the IPL image
    StartLogBlkAddr = NANDImageCfg.dwIplOffset;
    EndLogBlkAddr = StartLogBlkAddr + NANDImageCfg.dwIplSize;

    EdbgOutputDebugString("INFO: Copying IPL image to RAM address 0x%x\r\n", pSectorBuf);

    if(!GetPhyBlkAddr(StartLogBlkAddr, &PhyBlockAddr))
    {
        EdbgOutputDebugString("Error: No good block found - unable to store image!\r\n");
        return FALSE;
    }

    // Copy IPL from NAND flash to RAM
    do
    {
        // Compute sector address based on current physical block
        startSectorAddr = PhyBlockAddr * flashInfo.wSectorsPerBlock;
        endSectorAddr = startSectorAddr + flashInfo.wSectorsPerBlock;
        
        for (sectorAddr = startSectorAddr; sectorAddr < endSectorAddr; sectorAddr++)
        {
            if (!FMD_ReadSector(sectorAddr, pSectorBuf, &sectorInfo, 1))
            {
                EdbgOutputDebugString("ERROR: Read sector: [0x%x] failed.\r\n", sectorAddr);
                return FALSE;
            }

            pSectorBuf += flashInfo.wDataBytesPerSector;
        }

        if(!GetNextGoodBlock(PhyBlockAddr, &PhyBlockAddr))
        {
            EdbgOutputDebugString("Error: No good block found - unable to store image!\r\n");
            return FALSE;
        }
        
        StartLogBlkAddr++;

    }while(StartLogBlkAddr < EndLogBlkAddr);


    EdbgOutputDebugString("INFO: Copy of IPL completed successfully.\r\n");

    return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function:  NANDFormatAll
//
//  This function formats (erases) the entire NAND flash memory.
//
//  Parameters:
//      None.     
//
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//-----------------------------------------------------------------------------
BOOL NANDFormatAll(void)
{
    UINT32 blockID, startBlockID, endBlockID;
    UINT32 percentComplete, lastPercentComplete;

    // Get NAND flash data bytes per sector.
    //
    if(!g_bNandExist)
    {
        EdbgOutputDebugString("WARNING: NAND device doesn't exist - unable to read image.\r\n");
        return FALSE;
    }
    
    // Get NAND flash data bytes per sector.
    //
    if(!FMD_GetInfo(&flashInfo))
    {
        EdbgOutputDebugString("ERROR: Unable to get NAND flash information.\r\n");
        return FALSE;
    }

    // Calculate the physical block range for the enrire NAND device
    startBlockID = 0;
    endBlockID = flashInfo.dwNumBlocks;

    EdbgOutputDebugString("INFO: Starting format of all NAND regions.\r\n");

    lastPercentComplete = 0;

    for (blockID = startBlockID; blockID < endBlockID ; blockID++)
    {
        // Is the block bad?
        //
        if (FMD_GetBlockStatus(blockID) == BLOCK_STATUS_BAD)
        {
            EdbgOutputDebugString("INFO: Found bad NAND flash block [0x%x].\r\n", blockID);
            continue;
        }

        // Erase the block...
        //
        if (!FMD_EraseBlock(blockID))
        {
            EdbgOutputDebugString("ERROR: Unable to erase NAND flash block 0x%x.\r\n", blockID);
            continue;
        }
        else
        {
            percentComplete = 100 * (blockID - startBlockID + 1) / (endBlockID - startBlockID);

            // If percentage complete has changed, show the progress
            if (lastPercentComplete != percentComplete)
            {
                lastPercentComplete = percentComplete;
                OEMWriteDebugByte('\r');
                EdbgOutputDebugString("INFO: Format is %d%% complete.", percentComplete);
            }
        }
    }
    
    EdbgOutputDebugString("\r\nINFO: Format of all NAND regions completed successfully.\r\n");

    return(TRUE);
}


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

    // Get NAND flash data bytes per sector.
    //
    if (!FMD_GetInfo(&flashInfo))
    {
        EdbgOutputDebugString("ERROR: Unable to get NAND flash information.\r\n");
        return FALSE;
    }

    // Calculate the physical block range for the enrire NAND device
    startBlockID = 0;
    endBlockID = flashInfo.dwNumBlocks;

    EdbgOutputDebugString("INFO: Starting format of all NAND regions.\r\n");

    lastPercentComplete = 0;

    for (blockID = startBlockID; blockID < endBlockID ; blockID++)
    {
        // Erase the block...
        //
        if (!FMD_EraseBlock(blockID))
        {
            EdbgOutputDebugString("ERROR: Unable to erase NAND flash block 0x%x.\r\n", blockID);
            continue;
        }
        else
        {
            percentComplete = 100 * (blockID - startBlockID + 1) / (endBlockID - startBlockID);

            // If percentage complete has changed, show the progress
            if (lastPercentComplete != percentComplete)
            {
                lastPercentComplete = percentComplete;
                OEMWriteDebugByte('\r');
                EdbgOutputDebugString("INFO: Format is %d%% complete.", percentComplete);
            }
        }
    }
    
    EdbgOutputDebugString("\r\nINFO: Format of all NAND regions completed successfully.\r\n");

    return(TRUE);
}


