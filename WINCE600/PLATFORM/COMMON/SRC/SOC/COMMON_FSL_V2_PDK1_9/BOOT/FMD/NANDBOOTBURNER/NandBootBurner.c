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
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  nandbootburner.c
//
//  Implement image burning to NAND flash.
//
//-----------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include "nandboot.h"
#include "common_nandfmd.h"
#pragma warning(pop)
//-----------------------------------------------------------------------------
// External Functions

//-----------------------------------------------------------------------------
// External Variables

//-----------------------------------------------------------------------------
// Defines
#define NANDFC_BOOT_SIZE (4096*8)

//-----------------------------------------------------------------------------
// Types

//-----------------------------------------------------------------------------
// Global Variables
NAND_IMAGE_CFG NANDImageCfg;
FlashInfo flashInfo;
static BYTE pVeriSectorBuf[NANDFC_BOOT_SIZE];

//-----------------------------------------------------------------------------
// Local Functions

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
    
    if(!FMD_GetInfo(&flashInfo))
    {
        RETAILMSG(TRUE, (_T("ERROR: Unable to get NAND flash information.\r\n")));
        return FALSE;
    }    
    
    BSP_GetNANDImageCfg(&NANDImageCfg);

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
    
    // Boot Configuration and Buffer
    if(!NANDBootInit())
    {
        RETAILMSG(TRUE, (_T("WARNING: NANDBootInit failed.\r\n")));
        return ;
    }
    
    startBlockID = 0;
    endBlockID = (DWORD)(NANDImageCfg.dwNandSize + flashInfo.dwBytesPerBlock - 1) / (DWORD)flashInfo.dwBytesPerBlock;
    RETAILMSG(TRUE, (_T("INFO: Set NAND flash blocks [0x%x ~ 0x%x] as reserved.\r\n"), startBlockID, endBlockID-1));        

    for(blockID = startBlockID; blockID < endBlockID; blockID++)
    {
        dwResult = FMD_GetBlockStatus(blockID);
        
        // Skip bad blocks
        if(dwResult & BLOCK_STATUS_BAD)
        {
            //RETAILMSG(TRUE, (_T("INFO: Found bad NAND flash block [0x%x].\r\n", blockID);
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
            //RETAILMSG(TRUE, (_T("ERROR: Unable to erase NAND flash block [0x%x].\r\n", blockID);
            FMD_SetBlockStatus(blockID, BLOCK_STATUS_BAD);
            continue;
        }
        
        //RETAILMSG(TRUE, (_T("Set Block #%d to be reserved.\r\n"), blockID));
        if(!FMD_SetBlockStatus(blockID, BLOCK_STATUS_RESERVED))
        {
            //RETAILMSG(TRUE, (_T("ERROR: Unable to set block status [0x%x].\r\n", blockID);
            continue;
        }
    }

}

//-----------------------------------------------------------------------------
//
//  Function:  GetNextGoodBlock
//
//  This function searches the good block mapped to specified logical block.
//
//  Parameters:
//      LogBlockAddr
//          [in] The address of first physical block,the next physical block address will be a start point to search a good block.   
//
//      pGoodBlkAddr 
//          [out] a pointer to a good block which is mapped to the specified logical block.          
//
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//-----------------------------------------------------------------------------
static BOOL GetGoodPhyBlock(DWORD LogBlockAddr, DWORD *pGoodBlkAddr)
{
    DWORD StartPhyBlockAddr = 0;
    DWORD i;

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

//-----------------------------------------------------------------------------
//
//  Function:  GetNextGoodBlock
//
//  This function searches next good block from the address of specified physical block.
//
//  Parameters:
//      StartPhyBlockAddr
//          [in] The address of first physical block,the next physical block address will be a start point to search a good block.   
//
//      pNxtGoodBlkAddr 
//          [out] a pointer to a good block which is the nearest good block from the specified physical block.          
//
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//-----------------------------------------------------------------------------
BOOL GetNextGoodBlock(DWORD StartPhyBlockAddr, DWORD *pNxtGoodBlkAddr)
{
    DWORD BlockStatus;

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

//-----------------------------------------------------------------------------
//
//  Function:  GetPhyBlkAddr
//
//  This function searches the good block from specified logical address on.
//
//  Parameters:
//      LogicalBlkAddr
//          [in] The address of a logical block.
//
//      pNxtGoodBlkAddr 
//          [out] a pointer to a good physical block which is mapped to the next logical block from the specified logical block.          
//
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//-----------------------------------------------------------------------------
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

BOOL WriteSingleBlock(DWORD BlockID, LPBYTE pSectorBuf)
{
    SectorInfo sectorInfo;
    SECTOR_ADDR sectorAddr, startSectorAddr, endSectorAddr;
    
    FMD_EraseBlock(BlockID);
    
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
BOOL NANDWriteImage(PNANDWrtImgInfo pNANDWrtImgInfo, LPBYTE pImage, DWORD dwLength)
{
    DWORD StartLogBlkAddr,PhyBlockAddr,dwStartOffset;
                
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
    else
    {
        dwStartOffset = NANDImageCfg.dwXldrOffset;
        if(!pNANDWrtImgInfo->dwIndex)
        {
            RETAILMSG(TRUE, (_T("INFO: Writing XLDR image to NAND (please wait)...\r\n")));
        }        
    }
      
    StartLogBlkAddr = dwStartOffset / flashInfo.dwBytesPerBlock + pNANDWrtImgInfo->dwIndex; 

    //RETAILMSG(TRUE, (_T("INFO: Programming NAND flash blocks 0x%x\r\n"), StartLogBlkAddr));
    
    if(!GetGoodPhyBlock(StartLogBlkAddr, &PhyBlockAddr))
    {
        ERRORMSG(TRUE, (_T("GetGoodPhyBlock failed: 0x%x\r\n"),StartLogBlkAddr));  
        return FALSE; 
    }

    if(!WriteSingleBlock(PhyBlockAddr,pImage))
    {
         RETAILMSG(TRUE, (_T("Error: Writing image failed!\r\n")));
         return FALSE;
    } 
    
    return(TRUE);
}

//-----------------------------------------------------------------------------
//
//  Function:  NANDEndWritingImage
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
BOOL NANDEndWritingImage(PNANDWrtImgInfo pNANDWrtImgInfo)
{
    
    NANDBootReserved();    

    if(pNANDWrtImgInfo->dwImgType == IMAGE_NK)
    {
        RETAILMSG(TRUE, (_T("INFO: Writing NK image to NAND completed successfully.\r\n")));
    }
    else if(pNANDWrtImgInfo->dwImgType == IMAGE_EBOOT)
    {
        RETAILMSG(TRUE, (_T("INFO: Writing EBOOT image to NAND completed successfully.\r\n")));        
    }                
    else
    {
        RETAILMSG(TRUE, (_T("INFO: Writing XLDR image to NAND completed successfully.\r\n")));        
    }
      
    return(TRUE);
}



//-----------------------------------------------------------------------------
//
//  Function: BSP_OEMIoControl
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
    PNANDWrtImgInfo pNANDWrtImgInfo;
    
    UNREFERENCED_PARAMETER(pBytesReturned);
    
    // Boot Configuration and Buffer
    if(!NANDBootInit())
    {
         RETAILMSG(TRUE, (_T("WARNING: NANDBoot failed to initialize!\r\n")));
         return FALSE;
    }
            
    switch (dwIoControlCode){
        case IOCTL_DISK_VENDOR_GET_IMGINFO:
            {
                DEBUGMSG(TRUE, (L"IOCTL_DISK_VENDOR_GET_IMGINFO received. \r\n"));   
                
                if(!pOutBuf || nOutBufSize < sizeof(NANDWrtImgInfo))
                {
                    ERRORMSG(TRUE, (_T("invalid parameters\r\n")));
                    break;
                }
                
                pNANDWrtImgInfo = (PNANDWrtImgInfo)pOutBuf;

                pNANDWrtImgInfo->dwImgSizeUnit = flashInfo.dwBytesPerBlock;

                rc = TRUE;
            }
            break;
        case IOCTL_DISK_VENDOR_WRITE_IMAGE:
            {
                DEBUGMSG(TRUE, (L"IOCTL_DISK_VENDOR_WRITE_IMAGE received. \r\n")); 
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

                rc = NANDWriteImage(pNANDWrtImgInfo, pOutBuf, nOutBufSize);

                BSPNAND_SetClock(FALSE);
                break;
            }            
        case IOCTL_DISK_VENDOR_END_WRITE_IMAGE:
            {
                DEBUGMSG(TRUE, (L"IOCTL_DISK_VENDOR_END_WRITE_IMAGE received. \r\n")); 
                if(!pInBuf || nInBufSize < sizeof(NANDWrtImgInfo))
                {
                    ERRORMSG(TRUE, (_T("invalid parameters\r\n")));
                    break;
                }             
                pNANDWrtImgInfo = (PNANDWrtImgInfo)pInBuf;
        
                BSPNAND_SetClock(TRUE);
        
                NANDEndWritingImage(pNANDWrtImgInfo);

                BSPNAND_SetClock(FALSE);
        
                break;    
            }            
            
        default:
            break;
    }

    return(rc);
}
