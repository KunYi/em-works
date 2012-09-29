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
//-----------------------------------------------------------------------------
//
// Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  cspi_flash.c
//
//  Contains BOOT SPI flash support functions.
//
//-----------------------------------------------------------------------------
#include "bsp.h"
#include "loader.h"
#include "cspifmd.h"

//-----------------------------------------------------------------------------
// External Functions


//-----------------------------------------------------------------------------
// External Variables
extern BOOL g_bSpiExist;


//-----------------------------------------------------------------------------
// Defines
#define     SPI_BOOT_SIZE        (4096)


//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables
BYTE sectorBuf[SPI_BOOT_SIZE];


//-----------------------------------------------------------------------------
// Local Variables


//-----------------------------------------------------------------------------
// Local Functions


//-----------------------------------------------------------------------------
//
//  Function:  SPIWriteXldr
//
//  This function writes to SPI flash memory the XLDR image stored 
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
BOOL SPIWriteXldr(DWORD dwStartAddr, DWORD dwLength)
{
    FlashInfo flashInfo;
    LPBYTE pSectorBuf, pImage;

    // Check for SPI Flash device availability
    //
    if (!g_bSpiExist)
    {
        EdbgOutputDebugString("WARNING: SPI Flash device doesn't exist - unable to store image.\r\n");
        return(FALSE);
    }

    EdbgOutputDebugString("INFO: Writing XLDR image to SPI Flash (please wait)...\r\n");

    if (!SPIFMD_GetInfo(&flashInfo))
    {
        EdbgOutputDebugString("ERROR: Unable to get SPI flash information.\r\n");
        return(FALSE);
    }

    // Get cached image location
    pImage = OEMMapMemAddr(dwStartAddr, dwStartAddr);

    // If image is larger than 4K, this must be xldr.bin produced by
    // ROMIMAGE
    if (dwLength > 0x1000)
    {
        // ROMIMAGE adds 4K page at the beginning of the image.
        // We will flash the first 2K bytes that appear after this 4K page.
        //
        pImage += 0x1000;
        dwLength -= 0x1000;
    }

    // Make sure XLDR length does not exceed size that can be supported by SPI Flash (4KB)
    if (dwLength > flashInfo.wDataBytesPerSector)
    {
        EdbgOutputDebugString("ERROR: XLDR exceeds allocated size\r\n");
        return(FALSE);
    }

    // Fill unused space with 0xFF
    memset(pImage + dwLength, 0xFF, (flashInfo.wDataBytesPerSector) - dwLength);

    EdbgOutputDebugString("INFO: Using XLDR image from flash cache address 0x%x, size = %d\r\n", pImage, dwLength);

    // Write XLDR to SPI flash
    // First Block
    if (!SPIFMD_EraseBlock(0))
    {
        EdbgOutputDebugString("WARNING: Unable to erase SPI flash block #0\r\n");
        return(FALSE);
    }

    pSectorBuf = pImage;
    if (!SPIFMD_WriteSector(0, pSectorBuf, NULL, 1))
    {
        EdbgOutputDebugString("WARNING: Failed to write XLDR in Block #0 .\r\n");
        return(FALSE);
    }

    // Read XLDR from SPI flash to verify contents
    pSectorBuf = pImage + flashInfo.wDataBytesPerSector;
    if (!SPIFMD_ReadSector(0, pSectorBuf, NULL, 1))
    {
        EdbgOutputDebugString("WARNING: Failed to read XLDR in the 1st Block.\r\n");
        return(FALSE);
    }
    
    if (memcmp(pImage, pImage + flashInfo.wDataBytesPerSector, flashInfo.wDataBytesPerSector) != 0)
    {
        EdbgOutputDebugString("WARNING: Failed to verify XLDR.\r\n");
        return(FALSE);
    }


    EdbgOutputDebugString("INFO: Update of XLDR completed successfully.\r\n");

    return(TRUE);

}


//-----------------------------------------------------------------------------
//
//  Function:  SPIWriteBoot
//
//  This function writes to SPI flash memory the Boot image stored 
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
BOOL SPIWriteBoot(DWORD dwStartAddr, DWORD dwLength)
{
    FlashInfo flashInfo;
    LPBYTE pSectorBuf, pImage;
    BLOCK_ID blockID, startBlockID, endBlockID;
    SECTOR_ADDR sectorAddr, startSectorAddr, endSectorAddr;

    // Check for SPI Flash device availability
    //
    if (!g_bSpiExist)
    {
        EdbgOutputDebugString("WARNING: SPI device doesn't exist - unable to store image.\r\n");
        return(FALSE);
    }

    EdbgOutputDebugString("INFO: Writing Boot image to SPI Flash (please wait)...\r\n");

    if (!SPIFMD_GetInfo(&flashInfo))
    {
        EdbgOutputDebugString("ERROR: Unable to get SPI flash information.\r\n");
        return(FALSE);
    }

    // Make sure Boot length does not exceed reserved FLASH size
    if (dwLength > IMAGE_BOOT_BOOTIMAGE_SPI_SIZE)
    {
        EdbgOutputDebugString("ERROR: Boot size exceeds reserved SPI region (size = 0x%x)\r\n", dwLength);
        return(FALSE);
    }

    // Calculate the physical block range for the EBOOT image
    startBlockID = IMAGE_BOOT_BOOTIMAGE_SPI_OFFSET / flashInfo.dwBytesPerBlock;
    endBlockID = startBlockID + (IMAGE_BOOT_BOOTIMAGE_SPI_SIZE / flashInfo.dwBytesPerBlock);
    
    EdbgOutputDebugString("INFO: Erasing SPI flash blocks [0x%x - 0x%x].\r\n", startBlockID, endBlockID);
    
    // Erase range of FLASH blocks reserved for EBOOT
    for (blockID = startBlockID; blockID < endBlockID; blockID++)
    {
        // Skip bad blocks (currently not supported in SPI Flash)
        if (SPIFMD_GetBlockStatus(blockID) == BLOCK_STATUS_BAD)
        {
            EdbgOutputDebugString("INFO: Found bad SPI flash block [0x%x].\r\n", blockID);
            continue;
        }

        // Erase the block...
        if (!SPIFMD_EraseBlock(blockID))
        {
            EdbgOutputDebugString("ERROR: Unable to erase SPI flash block [0x%x].\r\n", blockID);
            return(FALSE);
        }
    }

    // Get cached image location
    pImage = OEMMapMemAddr(dwStartAddr, dwStartAddr);

    // Fill unused space with 0xFF
    memset(pImage + dwLength, 0xFF, (IMAGE_BOOT_BOOTIMAGE_SPI_SIZE) - dwLength);
    memset(pImage + IMAGE_BOOT_BOOTIMAGE_SPI_SIZE, 0xFF, IMAGE_BOOT_BOOTIMAGE_SPI_SIZE);

    EdbgOutputDebugString("INFO: Programming EBOOT/SBOOT image from flash cache address 0x%x, size = %d\r\n", pImage, dwLength);

     // Write EBOOT to SPI flash
    pSectorBuf = pImage;

    for (blockID = startBlockID; blockID < endBlockID; blockID++)
    {        
        // Skip bad blocks (currently not supported in SPI Flash)
        if (SPIFMD_GetBlockStatus(blockID) == BLOCK_STATUS_BAD)
        {
            EdbgOutputDebugString("INFO: Found bad SPI flash block [0x%x].\r\n", blockID);
            continue;
        }

        // Compute sector address based on current physical block
        startSectorAddr = blockID * flashInfo.wSectorsPerBlock;
        endSectorAddr = startSectorAddr + flashInfo.wSectorsPerBlock;
        
        for (sectorAddr = startSectorAddr; sectorAddr < endSectorAddr; sectorAddr++)
        {
            if (!SPIFMD_WriteSector(sectorAddr, pSectorBuf, NULL, 1))
            {
                EdbgOutputDebugString("ERROR: Failed to update EBOOT/SBOOT.\r\n");
                return(FALSE);
            }

            pSectorBuf += flashInfo.wDataBytesPerSector;
        }
    }
    
    // Read EBOOT from SPI flash to verify contents
    pSectorBuf = pImage + IMAGE_BOOT_BOOTIMAGE_SPI_SIZE;

    for (blockID = startBlockID; blockID < endBlockID ; blockID++)
    {        
        // Skip bad blocks (currently not supported in SPI Flash)
        if (SPIFMD_GetBlockStatus(blockID) == BLOCK_STATUS_BAD)
        {
            EdbgOutputDebugString("INFO: Found bad SPI flash block [0x%x].\r\n", blockID);
            continue;
        }

        // Compute sector address based on current physical block
        startSectorAddr = blockID * flashInfo.wSectorsPerBlock;
        endSectorAddr = startSectorAddr + flashInfo.wSectorsPerBlock;
        
        for (sectorAddr = startSectorAddr; sectorAddr < endSectorAddr; sectorAddr++)
        {
            if (!SPIFMD_ReadSector(sectorAddr, pSectorBuf, NULL, 1))
            {
                EdbgOutputDebugString("ERROR: Failed to update EBOOT/SBOOT.\r\n");
                return(FALSE);
            }

            pSectorBuf += flashInfo.wDataBytesPerSector;
        }
    }

    EdbgOutputDebugString("INFO: Verifying image.\r\n");

    if (memcmp(pImage, pImage + IMAGE_BOOT_BOOTIMAGE_SPI_SIZE, IMAGE_BOOT_BOOTIMAGE_SPI_SIZE) != 0)
    {
        EdbgOutputDebugString("ERROR: Failed to verify EBOOT/SBOOT.\r\n");
    }

    EdbgOutputDebugString("INFO: Update of EBOOT/SBOOT completed successfully.\r\n");

    
    return(TRUE);

}

//------------------------------------------------------------------------------
//
//  Function:  SPILoadBootCFG
//
//  Retrieves bootloader configuration information (menu settings, etc.) from 
//  the SPI flash.
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
BOOL SPILoadBootCFG(BYTE *pBootCfg, DWORD cbBootCfgSize)
{
    BOOL rc = FALSE;
    FlashInfo flashInfo;
    BLOCK_ID blockID, startBlockID, endBlockID;
    SECTOR_ADDR sectorAddr;

    if (!SPIFMD_GetInfo(&flashInfo))
    {
        EdbgOutputDebugString("ERROR: Unable to get SPI flash information.\r\n");
        return(FALSE);
    }

    // Calculate the physical block range for the boot configuration
    startBlockID = IMAGE_BOOT_BOOTCFG_SPI_OFFSET / flashInfo.dwBytesPerBlock;
    endBlockID = startBlockID + (IMAGE_BOOT_BOOTCFG_SPI_SIZE / flashInfo.dwBytesPerBlock);
    
    EdbgOutputDebugString("INFO: Loading boot configuration from SPI\r\n");
    
    // Find a good block and load the boot configuration
    for (blockID = startBlockID; (blockID < endBlockID) && (rc == FALSE) ; blockID++)
    {        
        // Skip bad blocks (currently not supported in SPI Flash)
        if (SPIFMD_GetBlockStatus(blockID) == BLOCK_STATUS_BAD)
        {
            EdbgOutputDebugString("INFO: Found bad SPI flash block [0x%x].\r\n", blockID);
            continue;
        }

        // Compute sector address based on current physical block
        sectorAddr = blockID * flashInfo.wSectorsPerBlock;

        rc = SPIFMD_ReadSector(sectorAddr, sectorBuf, NULL, 1);
    }

    if (rc)
    {
        if (cbBootCfgSize > flashInfo.wDataBytesPerSector)
            cbBootCfgSize = flashInfo.wDataBytesPerSector;
        memcpy(pBootCfg, sectorBuf, cbBootCfgSize);
    }
    else
    {
        EdbgOutputDebugString("ERROR: Failed to load boot configuration from SPI\r\n");
    }

    return rc;
}


//------------------------------------------------------------------------------
//
//  Function:  SPIStoreBootCFG
//
//  Stores bootloader configuration information (menu settings, etc.) to 
//  the SPI flash.
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
BOOL SPIStoreBootCFG(BYTE *pBootCfg, DWORD cbBootCfgSize)
{

    FlashInfo flashInfo;
    BLOCK_ID blockID, startBlockID, endBlockID;
    SECTOR_ADDR sectorAddr;

    if (!SPIFMD_GetInfo(&flashInfo))
    {
        EdbgOutputDebugString("ERROR: Unable to get SPI flash information.\r\n");
        return FALSE;
    }

    if (cbBootCfgSize > flashInfo.wDataBytesPerSector)
        cbBootCfgSize = flashInfo.wDataBytesPerSector;
    memcpy(sectorBuf, pBootCfg, cbBootCfgSize);
    memset(sectorBuf + cbBootCfgSize, 0xFF, flashInfo.wDataBytesPerSector - cbBootCfgSize);

    // Calculate the physical block range for the boot configuration
    startBlockID = IMAGE_BOOT_BOOTCFG_SPI_OFFSET / flashInfo.dwBytesPerBlock;
    endBlockID = startBlockID + (IMAGE_BOOT_BOOTCFG_SPI_SIZE / flashInfo.dwBytesPerBlock);

    EdbgOutputDebugString("INFO: Storing boot configuration to SPI\r\n");
    
    // Erase range of SPI blocks reserved for boot configuration
    for (blockID = startBlockID; blockID < endBlockID; blockID++)
    {
        // Skip bad blocks (currently not supported in SPI Flash)
        if (SPIFMD_GetBlockStatus(blockID) == BLOCK_STATUS_BAD)
        {
            EdbgOutputDebugString("INFO: Found bad SPI flash block [0x%x].\r\n", blockID);
            continue;
        }

        // Erase the block...
        if (!SPIFMD_EraseBlock(blockID))
        {
            EdbgOutputDebugString("ERROR: Unable to erase SPI flash block [0x%x].\r\n", blockID);
            return FALSE;
        }
    }
    
    // Find a good block and store the boot configuration
    for (blockID = startBlockID; blockID < endBlockID ; blockID++)
    {        
        // Skip bad blocks (currently not supported in SPI Flash)
        if (SPIFMD_GetBlockStatus(blockID) == BLOCK_STATUS_BAD)
        {
            EdbgOutputDebugString("INFO: Found bad SPI flash block [0x%x].\r\n", blockID);
            continue;
        }

        // Compute sector address based on current physical block
        sectorAddr = blockID * flashInfo.wSectorsPerBlock;
        
        // Write out the boot configuration to the first sector of the block
        if (!SPIFMD_WriteSector(sectorAddr, sectorBuf, NULL, 1))
        {
            EdbgOutputDebugString("ERROR: Failed to update EBOOT.\r\n");
            return FALSE;
        }        
    }

    EdbgOutputDebugString("INFO: Successfully stored boot configuration to SPI\r\n");

    return TRUE;
}

