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
//  Copyright (C) 2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  sdmmc.c
//
//  Contains SDMMC BOOT support functions.
//
//-----------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include "bsp.h"
#include "loader.h"
#include "sdmmc.h"
#include "sdfmd.h"
#pragma warning(pop)
//-----------------------------------------------------------------------------
// External Functions
UINT32 MMC_Send_Switch_Cmd(UINT32 switch_arg);
//-----------------------------------------------------------------------------
// External Variables
extern SD_IMAGE_CFG            SDImageCfg;
extern BOOL                    g_bSDHCExist;
extern CHAR                    extCSDBuffer[SDHC_BLK_LEN];
//-----------------------------------------------------------------------------
// Defines

//-----------------------------------------------------------------------------
// Types

//-----------------------------------------------------------------------------
// Global Variables

//-----------------------------------------------------------------------------
// Local Variables

BOOL SDHCWriteSB2BP(DWORD dwStartAddr, DWORD dwLength)
{
    FlashInfo flashInfo;
    LPBYTE pSectorBuf, pImage;
    SECTOR_ADDR sectorAddr;
    DWORD len;
    
    if(!g_bSDHCExist)
    {
        OALMSG(1, (_T("WARNING: SDHC device doesn't exist - unable to store image\r\n")));
        return FALSE;
    }
    if(!SDMMC_GetInfo(&flashInfo))
    {
        OALMSG(1, (_T("ERROR: Unable to get SDHC flash information.\r\n")));
        return FALSE;
    }
    OALMSG(1, (_T("SDHCWriteSB2BP\r\n")));
    
    // Get cached image location
    pSectorBuf = pImage = OEMMapMemAddr(dwStartAddr, dwStartAddr);
    
    sectorAddr = 0;
    
    dwLength = (dwLength + flashInfo.dwBytesPerBlock - 1) & (~(flashInfo.dwBytesPerBlock - 1));
    len = dwLength;
    
    while(dwLength)
    {
        if(!SDMMC_WriteSector(sectorAddr, pSectorBuf, NULL, 1))
        {
            OALMSG(1, (_T("ERROR: Failed to update EBOOT\r\n")));
            return FALSE;
        }
        
        pSectorBuf += flashInfo.dwBytesPerBlock;
        sectorAddr++;
        dwLength -= flashInfo.dwBytesPerBlock;
    }
    
    OALMSG(1, (_T("\r\nINFO: Verifying image\r\n")));
    dwLength = len;
    sectorAddr = 0;
    while(dwLength)
    {
        // Read EBOOT from SDHC flash to verify contents
        pSectorBuf = pImage + dwLength;

        if(!SDMMC_ReadSector(sectorAddr, pSectorBuf, NULL, 1))
        {
            OALMSG(1, (_T("ERROR: Failed to read EBOOT\r\n")));
            return FALSE;
        }
        if(memcmp(pImage, pSectorBuf, flashInfo.dwBytesPerBlock) != 0)
        {
            OALMSG(1, (_T("ERROR: Failed to verify EBOOT\r\n")));
            return FALSE;
        }
        
        pImage += flashInfo.dwBytesPerBlock;
        sectorAddr++;
        dwLength -= flashInfo.dwBytesPerBlock;
    }
    
    OALMSG(1, (_T("INFO: Update of EBOOT completed successfully...\r\n")));

    return TRUE;
}
//-----------------------------------------------------------------------------
//
//  Function:  SDHCWriteRedundantSB
//
//  This function writes to SDHC memory the Boot image stored 
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
BOOL SDHCWriteRedundantSB(DWORD dwStartAddr, DWORD dwLength)
{
    FlashInfo flashInfo;
    LPBYTE pSectorBuf, pImage, pTemp;
    SECTOR_ADDR sectorAddr, startSectorAddr, endSectorAddr, numSectors;
    LONGLONG  dwBootSize, dwBlankSize;
    DWORD dwSDHC_RW_NUM;
    DWORD count = 0;
    
    // Check for SDHC device availability
    //
    if(!g_bSDHCExist)
    {
        OALMSG(1, (_T("WARNING: SDHC device doesn't exist - unable to store image\r\n")));
        return FALSE;
    }
    
    BSP_GetSDImageCfg(&SDImageCfg);
    
    OALMSG(1, (_T("INFO: Writing Boot image to SDHC (please wait)...\r\n")));

    if(!SDMMC_GetInfo(&flashInfo))
    {
        OALMSG(1, (_T("ERROR: Unable to get SDHC flash information.\r\n")));
        return FALSE;
    }

    dwSDHC_RW_NUM = 512;

    dwBootSize = SDImageCfg.dwBootSize / 2;    

    // Make sure Boot length does not exceed reserved SDHC size
    if(dwLength > dwBootSize)
    {
        OALMSG(1, (_T("ERROR: Boot size exceeds reserved SDHC region (size = 0x%x)\r\n"), dwLength));
        return FALSE;
    }

    // Get cached image location
    pTemp = pImage = OEMMapMemAddr(dwStartAddr, dwStartAddr);

    if (dwLength % flashInfo.dwBytesPerBlock)
    {
        dwBootSize = (dwLength / flashInfo.dwBytesPerBlock + 1) * flashInfo.dwBytesPerBlock;
        // Fill unused space with 0xFF
        memset(pImage + dwLength, 0xFF, (DWORD)(dwBootSize - dwLength));
    }
    else
    {
        dwBootSize = dwLength;
    }
    
    dwBlankSize = SDImageCfg.dwBootSize / 2 - dwBootSize;
    
    OALMSG(1, (_T("INFO: Programming EBOOT/SBOOT image from flash cache address 0x%x, size = %d\r\n"), pImage, dwLength));

retry:    

    // Write EBOOT to SDHC flash
    pSectorBuf = pImage = pTemp;
    
    if(count == 0)
    {
        startSectorAddr = (SECTOR_ADDR)(SDImageCfg.dwBootOffset / flashInfo.dwBytesPerBlock);
        endSectorAddr = startSectorAddr + (SECTOR_ADDR)(dwBootSize / flashInfo.dwBytesPerBlock);
    }
    else
    {
        startSectorAddr = (SECTOR_ADDR)(SDImageCfg.dwBootOffset / flashInfo.dwBytesPerBlock) \
                        + (SECTOR_ADDR)(SDImageCfg.dwBootSize / 2 / flashInfo.dwBytesPerBlock);
        endSectorAddr = startSectorAddr + (SECTOR_ADDR)(dwBootSize / flashInfo.dwBytesPerBlock);
    }
    
    //OALMSG(1, (_T("INFO: startSectorAddr 0x%x, endSectorAddr 0x%x\r\n"), startSectorAddr, endSectorAddr));
    
    if((endSectorAddr - startSectorAddr) < dwSDHC_RW_NUM)
    {    
        numSectors = endSectorAddr - startSectorAddr;
    }
    else
    {
        numSectors = dwSDHC_RW_NUM;
    }
    
    for(sectorAddr = startSectorAddr; sectorAddr < endSectorAddr; )
    {
        while(numSectors)
        {
            if(!SDMMC_WriteSector(sectorAddr, pSectorBuf, NULL, 1))
            {
                OALMSG(1, (_T("ERROR: Failed to update EBOOT\r\n")));
                return FALSE;
            }
            
            numSectors--;
            pSectorBuf += flashInfo.dwBytesPerBlock;
            sectorAddr++;
        }
        
        if((endSectorAddr - sectorAddr) < dwSDHC_RW_NUM)
        {
            numSectors = endSectorAddr - sectorAddr;
        }
        else
        {
            numSectors = dwSDHC_RW_NUM;
        }
    }
    
    if (!SDController.IsMMC)
    {
        if(!SDMMC_EraseBlock(sectorAddr, (DWORD)(dwBlankSize / flashInfo.dwBytesPerBlock)))
        {
            OALMSG(1, (_T("ERROR: Failed to erase block\r\n")));
            return FALSE;
        }
    }
    
    OALMSG(1, (_T("\r\nINFO: Verifying image\r\n")));
    
    for(sectorAddr = startSectorAddr; sectorAddr < endSectorAddr; )
    {
        // Read EBOOT from SDHC flash to verify contents
        pSectorBuf = pTemp + dwBootSize;
        
        if(!SDMMC_ReadSector(sectorAddr, pSectorBuf, NULL, 1))
        {
            OALMSG(1, (_T("ERROR: Failed to read EBOOT\r\n")));
            return FALSE;
        }
        if(memcmp(pImage, pSectorBuf, flashInfo.dwBytesPerBlock) != 0)
        {
            OALMSG(1, (_T("ERROR: Failed to verify EBOOT\r\n")));
            return FALSE;
        }
        pImage += flashInfo.dwBytesPerBlock;
        sectorAddr++;
    }
    
    if(count == 0)
    {
        count++;
        goto retry;   
    }
    
    OALMSG(1, (_T("INFO: Update of EBOOT completed successfully...\r\n")));

    return TRUE;
}
//-----------------------------------------------------------------------------
// Local Functions
//-----------------------------------------------------------------------------
void FormatMBR(PMASTERBOOT_RECORD pMBR, DWORD dwNumOfSectors)
{
    BYTE i;
    
    memset(pMBR, 0x0, sizeof(MASTERBOOT_RECORD));
    
    pMBR->Signature = 0xAA55;
    
    // three partitions
    // first one is FAT 
    pMBR->Partition[0].BootIndicator = PART_BOOTABLE;
    pMBR->Partition[0].SystemId = PART_DOS32;
    pMBR->Partition[0].RelativeSector = 1;
    pMBR->Partition[0].TotalSector = dwNumOfSectors - (IMAGE_BOOT_BOOTIMAGE_SD_SIZE + IMAGE_BOOT_NKIMAGE_SD_SIZE) / DEFAULT_SECTOR_SIZE - 1;
    
    // second one is boot
    pMBR->Partition[1].BootIndicator = PART_READONLY;
    pMBR->Partition[1].SystemId = 'S';
    pMBR->Partition[1].RelativeSector = pMBR->Partition[0].RelativeSector + pMBR->Partition[0].TotalSector;
    pMBR->Partition[1].TotalSector = IMAGE_BOOT_BOOTIMAGE_SD_SIZE / DEFAULT_SECTOR_SIZE;
    
    // third one is raw binary
    pMBR->Partition[2].BootIndicator = PART_READONLY;
    pMBR->Partition[2].SystemId = 0x10;
    pMBR->Partition[2].RelativeSector = pMBR->Partition[1].RelativeSector + pMBR->Partition[1].TotalSector;
    pMBR->Partition[2].TotalSector = IMAGE_BOOT_NKIMAGE_SD_SIZE / DEFAULT_SECTOR_SIZE;
        
    for(i = 0; i < 3; i++)
    {
        pMBR->Partition[i].StartingSector = (BYTE)(pMBR->Partition[i].RelativeSector + 1) & 0x3F;
        pMBR->Partition[i].EndingSector = (BYTE)(pMBR->Partition[i].RelativeSector + pMBR->Partition[i].TotalSector) & 0x3F;
    }
}

void DUMP_MBR(PMASTERBOOT_RECORD pMBR)
{
    int i;
    
    if(!pMBR)
        return;
        
    //dump MBR contents
    DEBUGMSG(TRUE, (_T("Signature=0x%x\r\n"), pMBR->Signature));
    
    for(i = 0; i < MAX_PARTTABLE_ENTRIES; i++)
    {
        DEBUGMSG(TRUE, (_T("Partition[%d].BootIndicator=0x%x\r\n"),i, pMBR->Partition[i].BootIndicator));
        DEBUGMSG(TRUE, (_T("Partition[%d].StartingHead=0x%x\r\n"),i, pMBR->Partition[i].StartingHead));
        DEBUGMSG(TRUE, (_T("Partition[%d].StartingSector=0x%x\r\n"),i, pMBR->Partition[i].StartingSector));
        DEBUGMSG(TRUE, (_T("Partition[%d].StartingCylinder=0x%x\r\n"),i, pMBR->Partition[i].StartingCylinder));
        DEBUGMSG(TRUE, (_T("Partition[%d].SystemId=0x%x\r\n"),i, pMBR->Partition[i].SystemId));
        DEBUGMSG(TRUE, (_T("Partition[%d].EndingHead=0x%x\r\n"),i, pMBR->Partition[i].EndingHead));
        DEBUGMSG(TRUE, (_T("Partition[%d].EndingSector=0x%x\r\n"),i, pMBR->Partition[i].EndingSector));
        DEBUGMSG(TRUE, (_T("Partition[%d].EndingCylinder=0x%x\r\n"),i, pMBR->Partition[i].EndingCylinder));
        DEBUGMSG(TRUE, (_T("Partition[%d].RelativeSector=0x%x\r\n"),i, pMBR->Partition[i].RelativeSector));
        DEBUGMSG(TRUE, (_T("Partition[%d].TotalSector=0x%x\r\n"),i, pMBR->Partition[i].TotalSector));
    }
}

//------------------------------------------------------------------------------
//
// Function: BSPEMMCCheckBP
//
//    This function will check the eMMC 4.3/4.4 boot partition configuration
//
// Parameters:
//        none
//
// Returns:
//        TRUE for boot partition
//        FALSE for user partition
//
//------------------------------------------------------------------------------
BOOL BSPEMMCCheckBP(DWORD length)
{
    BOOL status = FALSE;

    if (!SDController.IsMMC)
    {
        OALMSG(OAL_INFO, (L"\r\neMMC not present!\r\n"));
        return status;
    }
    
    if(ESDHC_STATUS_PASS != SDMMC_get_ext_csd())
    {
        return status;
    } 
     
    switch(extCSDBuffer[MMC_EXT_CSD_BOOT_CONFIG])
    {
        case 0x49:
        case 0x52:
            {
                if((DWORD)extCSDBuffer[226] * 128 * 1024 > length)
                {
                    RETAILMSG(1, (_T("boot partition size(0x%x) large enough\r\n"),extCSDBuffer[226] * 128 * 1024));
                    //MMC_Send_Switch_Cmd(0x03b10000 | (0x1 << EMMC_SWITCH_BOOT_VALUE_SHIFT));
                    status = TRUE;
                }
                else
                {
                    RETAILMSG(1, (_T("boot partition size(0x%x) too small, switch to user partition\r\n"),extCSDBuffer[226] * 128 * 1024));
                    MMC_Send_Switch_Cmd(EMMC_SWITCH_WRITE_BOOT_PART | (0x0 << EMMC_SWITCH_BOOT_VALUE_SHIFT));
                    status = FALSE;
                }
            }
            break;
        default:
            break;
    }
    
    // update the global extCSDBuffer
    SDMMC_get_ext_csd();
    RETAILMSG(1, (_T("extCSDBuffer[177]=0x%x\r\n"),extCSDBuffer[177]));
    return status;
}

//-----------------------------------------------------------------------------
//
//  Function:  MX28_SDHCWriteSB
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
BOOL MX28_SDHCWriteSB(DWORD dwStartAddr, DWORD dwLength)
{
    int i;
    int key;
    char buf[512] = {0};
    BOOL bNeedFormat = FALSE;
    int entry = 0xff;
    DWORD dwConfigSector;
    DWORD dwTotalNumOfSectors = SDMMC_GetCardCapacity();
    PMASTERBOOT_RECORD pMBR = (PMASTERBOOT_RECORD)buf;
    ConfigBlock_t *config;
    
    //0. check boot partition size
    if(BSPEMMCCheckBP(dwLength))
    {
        //its target is boot partition   
        //we'll only write .sb file to it
        return SDHCWriteSB2BP(dwStartAddr, dwLength);
    }
    
    
    //1. read MBR contents
    if(!SDMMC_ReadSector(0, (LPBYTE)buf, NULL, 1))
    {
        RETAILMSG(1, (_T("ERROR: Failed to read MBR from SDHC\r\n")));
        return FALSE;
    }
   
    DUMP_MBR(pMBR);
    
    //2. check MBR contents
    if(pMBR->Signature == 0xAA55)
    {
        for(i = 0; i < MAX_PARTTABLE_ENTRIES; i++)
        {
            if(pMBR->Partition[i].SystemId == 'S')       //soc spec   
            {
                //it's boot partition
                break;
            }
        }
        
        if(i == MAX_PARTTABLE_ENTRIES)
        {
            bNeedFormat = TRUE;
        }
        else
        {
            if((pMBR->Partition[i].RelativeSector + pMBR->Partition[i].TotalSector < dwTotalNumOfSectors) &&
                pMBR->Partition[i].TotalSector * ESDHC_SECTOR_SIZE >= dwLength /*IMAGE_BOOT_BOOTIMAGE_SD_SIZE*/ )
            {
                bNeedFormat = FALSE;
                entry = i;
                dwConfigSector = pMBR->Partition[entry].RelativeSector;
            }
            else
            {                
                bNeedFormat = TRUE;
            }
        }
        
    }
    else
    {
        bNeedFormat = TRUE;
    }
    
    //3. Format storage if needed
    if(bNeedFormat)
    {
        KITLOutputDebugString("We must format storage before writing image to it, do you want to continue? [Y/N]");
        do {
            key = OEMReadDebugByte();
        } while ((key != 'Y') && (key != 'N') && (key != 'y') && (key != 'n'));
        KITLOutputDebugString("\r\n");
        switch (key)
        {
            case 'y':
            case 'Y':
                break;
    
            default:
                return FALSE;
        }
        //format storage
        if(dwTotalNumOfSectors <= (IMAGE_BOOT_BOOTIMAGE_SD_SIZE + IMAGE_BOOT_NKIMAGE_SD_SIZE) / DEFAULT_SECTOR_SIZE)
        {
            RETAILMSG(1, (_T("card capacity not enough, please change another card!\r\n")));
            return FALSE;
        }
        FormatMBR(pMBR, dwTotalNumOfSectors);
            
        if(!SDMMC_WriteSector(0, (LPBYTE)pMBR, NULL, 1))
        {
            RETAILMSG(1, (_T("ERROR: Failed to Write MBR to SDHC\r\n")));
            return FALSE;
        }

        entry = 1;
        BSP_GetSDImageCfg(&SDImageCfg);
    }
    
    dwConfigSector = pMBR->Partition[entry].RelativeSector;
    
    //4. write BCB data
    config = (ConfigBlock_t*)buf;
    memset(buf, 0x0, sizeof(buf));
    
    config->u32Signature = FIRMWARE_CONFIG_BLOCK_SIGNATURE;
    config->u32PrimaryBootTag = PRIMARY_TAG;
    config->u32SecondaryBootTag = SECONDARY_TAG;
    config->u32NumCopies = 2;
    
    config->aDriveInfo[0].u32Tag = PRIMARY_TAG;
    config->aDriveInfo[0].u32FirstSectorNumber = (UINT32)(SDImageCfg.dwBootOffset / ESDHC_SECTOR_SIZE);
    config->aDriveInfo[0].u32SectorCount = (UINT32)(SDImageCfg.dwBootSize / 2 / ESDHC_SECTOR_SIZE);
    
    config->aDriveInfo[1].u32Tag = SECONDARY_TAG;
    config->aDriveInfo[1].u32FirstSectorNumber = config->aDriveInfo[0].u32FirstSectorNumber + config->aDriveInfo[0].u32SectorCount;
    config->aDriveInfo[1].u32SectorCount = config->aDriveInfo[0].u32SectorCount;
    
    if(!SDMMC_WriteSector(dwConfigSector, (LPBYTE)buf, NULL, 1))
    {
        RETAILMSG(1, (_T("ERROR: Failed to Write BCB to SDHC\r\n")));
        return FALSE;
    }
    
    //5. write sb to storage
    return SDHCWriteRedundantSB(dwStartAddr, dwLength);
}
