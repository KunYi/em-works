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
//  Copyright (C) 2007-2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
#include "sdfmd.h"
#pragma warning(push)
#pragma warning(disable: 4201)
#include <blcommon.h>
#pragma warning(pop)
#include "sdfat.h"

//-----------------------------------------------------------------------------
// External Functions
extern VOID OEMWriteDebugByte(UINT8 ch);
extern LPBYTE OEMMapMemAddr (DWORD dwImageStart, DWORD dwAddr);
extern UINT32 MMC_Send_Switch_Cmd(UINT32 switch_arg);


//-----------------------------------------------------------------------------
// External Variables
extern BOOL                     g_bSDHCExist;
extern SD_IMAGE_CFG             SDImageCfg;
extern CHAR                     extCSDBuffer[SDHC_BLK_LEN];
extern SDCARD_CARD_REGISTERS    sdCardReg;
extern DWORD                    eSDBootPartitionSize;


//-----------------------------------------------------------------------------
// Defines
#define SDHC_BOOT_SIZE          (8 * 1024)
#define SDHC_CACHE_BLK_NUM (128)
#define NUMSECTORS_REQUIRED_FOR_BOOTIMAGES ((SECTOR_ADDR)(SDImageCfg.dwSdSize / ESDHC_SECTOR_SIZE))
#define NK_UNUSED_BLOCKS     (2)
#define IMAGE_LENGTH_MAGIC_STRING   "06732754"
#define IMAGE_LENGTH_MAGIC_SIZE  8


//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables
static FlashInfo g_flashInfo;
BOOL eSDBPEnabled = FALSE;

SDH_HARDWARE_CONTEXT SDController;



//-----------------------------------------------------------------------------
// Local Variables
FILEINFO g_fnFileInfo;

BYTE bCacheBuffer[SDHC_BLK_LEN*SDHC_CACHE_BLK_NUM] = { 0 };     // Cache Buffer
DWORD   g_dwSDHC_RW_NUM = 512;

//-----------------------------------------------------------------------------
// Local Functions

//-----------------------------------------------------------------------------
//
//  Function:  InitSDHCContext
//
//  This function init the global SDHC hardware context 
// 
//  Parameters:
//
//  None
//
//  Returns:
//      None
//
//-----------------------------------------------------------------------------
VOID InitSDHCContext(VOID)
{
    memset((PVOID)&SDController, 0x0, sizeof(SDController));
}

//-----------------------------------------------------------------------------
//
//  Function:  SDHCWriteXldr
//
//  This function writes to SDHC memory the XLDR image stored 
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
BOOL SDHCWriteXldr(DWORD dwStartAddr, DWORD dwLength)
{
    FlashInfo flashInfo;
    LPBYTE pSectorBuf, pImage;
    SECTOR_ADDR sectorAddr = 0;
    DWORD dwNumSectors;

    // Check for SDHC device availability
    //
    if(!g_bSDHCExist)
    {
        OALMSG(1, (_T("WARNING: SDHC device doesn't exist - unable to store image.\r\n")));
        return FALSE;
    }

    OALMSG(1, (_T("INFO: Writing XLDR image to SDHC (please wait)...\r\n")));
    
    if(!SDMMC_GetInfo(&flashInfo))
    {
        OALMSG(1, (_T("ERROR: Unable to get SDHC flash information.\r\n")));
        return FALSE;
    }

    // Get cached image location
    pImage = OEMMapMemAddr(dwStartAddr, dwStartAddr);

    // If ROM_SIGNATURE is there, it must be xldr.bin being downloaded.
    if (*((UINT32 *)(pImage + ROM_SIGNATURE_OFFSET)) == ROM_SIGNATURE)
    {
        // ROMIMAGE adds 4K page at the beginning of the image.
        // We will flash the "real" bytes that appear after this 4K page.
        //
        pImage += 0x1000;
        dwLength -= 0x1000;
    }

    // Make sure XLDR length does not exceed size that can be supported by SDHC (8 KB)
    if(dwLength > SDHC_BOOT_SIZE)
    {
        OALMSG(1, (_T("ERROR: XLDR exceeds 8KByte\r\n")));
        return FALSE;
    }

    // Fill unused space with 0xFF
    memset(pImage + dwLength, 0xFF, (SDHC_BOOT_SIZE - dwLength));

    OALMSG(1, (_T("INFO: Using XLDR image from flash cache address 0x%x, size = %d\r\n"), pImage, dwLength));
    
    // Write XLDR to SDHC flash
    pSectorBuf = pImage;
    sectorAddr = (SECTOR_ADDR)(SDImageCfg.dwXldrOffset / flashInfo.dwBytesPerBlock);
    dwNumSectors = (SDHC_BOOT_SIZE / flashInfo.dwBytesPerBlock);
    if(!SDMMC_WriteSector(sectorAddr, pSectorBuf, NULL, dwNumSectors))
    {
        OALMSG(1, (_T("ERROR: Failed to update XLDR\r\n")));
        return FALSE;
    }

    // Read XLDR from SDHC flash to verify contents
    pSectorBuf = pImage + SDHC_BOOT_SIZE;
    if(!SDMMC_ReadSector(sectorAddr, pSectorBuf, NULL, dwNumSectors))
    {
        OALMSG(1, (_T("ERROR: Failed to verify XLDR\r\n")));
        return FALSE;
    }
        
    if(memcmp(pImage, pImage + SDHC_BOOT_SIZE, SDHC_BOOT_SIZE) != 0)
    {
        OALMSG(1, (_T("ERROR: Failed to verify XLDR\r\n")));
        return FALSE;
    }
    
    OALMSG(1, (_T("INFO: Update of XLDR completed successfully\r\n")));
 
    return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function:  SDHCWriteBoot
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
BOOL SDHCWriteBoot(DWORD dwStartAddr, DWORD dwLength)
{
    FlashInfo flashInfo;
    LPBYTE pSectorBuf, pImage;
    SECTOR_ADDR startSectorAddr;
    DWORD dwNumSectors;

    // Check for SDHC device availability
    //
    if(!g_bSDHCExist)
    {
        OALMSG(1, (_T("WARNING: SDHC device doesn't exist - unable to store image\r\n")));
        return FALSE;
    }

    OALMSG(1, (_T("INFO: Writing Boot image to SDHC (please wait)...\r\n")));

    if(!SDMMC_GetInfo(&flashInfo))
    {
        OALMSG(1, (_T("ERROR: Unable to get SDHC flash information.\r\n")));
        return FALSE;
    }

    // Make sure Boot length does not exceed reserved SDHC size
    if(dwLength > SDImageCfg.dwBootSize)
    {
        OALMSG(1, (_T("ERROR: Boot size exceeds reserved SDHC region (size = 0x%x)\r\n"), dwLength));
        return FALSE;
    }
  
    // Get cached image location
    pImage = OEMMapMemAddr(dwStartAddr, dwStartAddr);

    if(SDImageCfg.dwSocId == 50) //MX50 has no xldr.
    {
        // If ROM_SIGNATURE is there, it must be eboot.bin being downloaded.
        if (*((UINT32 *)(pImage + ROM_SIGNATURE_OFFSET)) == ROM_SIGNATURE)
        {
            // ROMIMAGE adds 4K page at the beginning of the image.
            // We will flash the "real" bytes that appear after this 4K page.
            //
            pImage += 0x1000;
            dwLength -= 0x1000;
        }
    }
    // Fill unused space with 0xFF
    memset(pImage + dwLength, 0xFF, ((DWORD)SDImageCfg.dwBootSize - dwLength));

    OALMSG(1, (_T("INFO: Programming EBOOT/SBOOT image from flash cache address 0x%x, size = %d\r\n"), pImage, dwLength));
    
    // Write EBOOT to SDHC flash
    pSectorBuf = pImage;
    startSectorAddr = (SECTOR_ADDR)(SDImageCfg.dwBootOffset / flashInfo.dwBytesPerBlock);
    dwNumSectors = (DWORD)(SDImageCfg.dwBootSize / flashInfo.dwBytesPerBlock);

    if(!SDMMC_WriteSector(startSectorAddr, pSectorBuf, NULL, dwNumSectors))
    {
        OALMSG(1, (_T("ERROR: Failed to update EBOOT/SBOOT\r\n")));
        return FALSE;
    }

    // Read EBOOT from SDHC flash to verify contents
    pSectorBuf = pImage + SDImageCfg.dwBootSize;
    if(!SDMMC_ReadSector(startSectorAddr, pSectorBuf, NULL, dwNumSectors))
    {
        OALMSG(1, (_T("ERROR: Failed to verify EBOOT/SBOOT\r\n")));
        return FALSE;
    }
    
    OALMSG(1, (_T("INFO: Verifying image\r\n")));

    if(memcmp(pImage, pImage + SDImageCfg.dwBootSize, (DWORD)SDImageCfg.dwBootSize) != 0)
    {
        OALMSG(1, (_T("ERROR: Failed to verify EBOOT/SBOOT\r\n")));
        return FALSE;
    }

    OALMSG(1, (_T("INFO: Update of EBOOT/SBOOT completed successfully\r\n")));

    return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function:  SDHCWriteRedundantBoot
//
//  This function writes to SDHC memory the Redundant Boot image stored 
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
BOOL SDHCWriteRedundantBoot(DWORD dwStartAddr, DWORD dwLength)
{
    FlashInfo flashInfo;
    LPBYTE pSectorBuf, pImage;
    SECTOR_ADDR startSectorAddr;
    DWORD dwNumSectors;
    DWORD SIT[ESDHC_SECTOR_SIZE >> 2];  // 512 bytes secondary image table.

    // Check for SDHC device availability
    //
    if(!g_bSDHCExist)
    {
        OALMSG(1, (_T("WARNING: SDHC device doesn't exist - unable to store image\r\n")));
        return FALSE;
    }

    OALMSG(1, (_T("INFO: Writing Boot image to SDHC (please wait)...\r\n")));

    if(!SDMMC_GetInfo(&flashInfo))
    {
        OALMSG(1, (_T("ERROR: Unable to get SDHC flash information.\r\n")));
        return FALSE;
    }

    // Make sure Boot length does not exceed reserved SDHC size
    if(dwLength > SDImageCfg.dwBootSize)
    {
        OALMSG(1, (_T("ERROR: Boot size exceeds reserved SDHC region (size = 0x%x)\r\n"), dwLength));
        return FALSE;
    }
  
    // Get cached image location
    pImage = OEMMapMemAddr(dwStartAddr, dwStartAddr);

    if(SDImageCfg.dwSocId == 53) //MX53 has no xldr.
    {
        // If ROM_SIGNATURE is there, it must be xldr.bin being downloaded.
        if (*((UINT32 *)(pImage + ROM_SIGNATURE_OFFSET)) == ROM_SIGNATURE)
        {
            // ROMIMAGE adds 4K page at the beginning of the image.
            // We will flash the "real" bytes that appear after this 4K page.
            //
            pImage += 0x1000;
            dwLength -= 0x1000;
        }

        // Make sure eboot length does not exceed size that can be supported by SDHC (512 KB)
        if(dwLength > SDImageCfg.dwBoot2Size)
        {
            OALMSG(1, (_T("ERROR: Redundant image exceeds %dKByte\r\n"),SDImageCfg.dwBoot2Size/1024));
            return FALSE;
        }
    }
    // Fill unused space with 0xFF
    memset(pImage + dwLength, 0xFF, ((DWORD)SDImageCfg.dwBoot2Size - dwLength));

    OALMSG(1, (_T("INFO: Programming redundant boot image from flash cache address 0x%x, size = %d\r\n"), pImage, dwLength));
    
    // Write EBOOT to SDHC flash
    pSectorBuf = pImage;
    startSectorAddr = (DWORD)(SDImageCfg.dwBoot2Offset / flashInfo.dwBytesPerBlock);
    dwNumSectors = (DWORD)(SDImageCfg.dwBoot2Size / flashInfo.dwBytesPerBlock);

    if(!SDMMC_WriteSector(startSectorAddr, pSectorBuf, NULL, dwNumSectors))
    {
        OALMSG(1, (_T("ERROR: Failed to update redundant boot image\r\n")));
        return FALSE;
    }

    // Read EBOOT from SDHC flash to verify contents
    pSectorBuf = pImage + SDImageCfg.dwBoot2Size;
    if(!SDMMC_ReadSector(startSectorAddr, pSectorBuf, NULL, dwNumSectors))
    {
        OALMSG(1, (_T("ERROR: Failed to verify redundant boot image\r\n")));
        return FALSE;
    }
    
    OALMSG(1, (_T("INFO: Verifying image\r\n")));

    if(memcmp(pImage, pImage + (DWORD)SDImageCfg.dwBoot2Size, (DWORD)SDImageCfg.dwBoot2Size) != 0)
    {
        OALMSG(1, (_T("ERROR: Failed to verify redundant boot image\r\n")));
        return FALSE;
    }

    
    // initialize MBR to 0
    memset(SIT, 0, sizeof(SIT));

    // set boot sector signature at the end of the sector
    SIT[2] = 0x00112233; //0x584D2E69; //"i.MX"
    SIT[3] = (DWORD)((SDImageCfg.dwBoot2Offset - SDImageCfg.dwXldrOffset)/flashInfo.dwBytesPerBlock);
    if(SDMMC_WriteSector(1, (LPBYTE)SIT, NULL, 1))
    {
        OALMSG(1, (L"Successfully created Secondary image table \r\n"));
    }
    else
    {
        OALMSG(1, (L"Error creating Secondary image table on the SD/MMC card.\r\n"));
        return FALSE;
    }

    OALMSG(1, (_T("INFO: Update of redundant boot image completed successfully\r\n")));

    return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function:  SDHCWriteSB
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
BOOL SDHCWriteSB(DWORD dwStartAddr, DWORD dwLength)
{
    FlashInfo flashInfo;
    LPBYTE pSectorBuf, pImage;
    SECTOR_ADDR sectorAddr, startSectorAddr, endSectorAddr, numSectors;
    LONGLONG  dwBootSize;

    // Check for SDHC device availability
    //
    if(!g_bSDHCExist)
    {
        OALMSG(1, (_T("WARNING: SDHC device doesn't exist - unable to store image\r\n")));
        return FALSE;
    }

    OALMSG(1, (_T("INFO: Writing Boot image to SDHC (please wait)...\r\n")));

    if(!SDMMC_GetInfo(&flashInfo))
    {
        OALMSG(1, (_T("ERROR: Unable to get SDHC flash information.\r\n")));
        return FALSE;
    }

    // MX233 Block count reg can only hold 8 bits data, so the block size can not large than (255+1)
    if(SDImageCfg.dwSocId == 233)
        g_dwSDHC_RW_NUM = 256; 
    else
        g_dwSDHC_RW_NUM = 512;

    dwBootSize = SDImageCfg.dwBootSize;    

    // Make sure Boot length does not exceed reserved SDHC size
    if(dwLength > dwBootSize)
    {
        OALMSG(1, (_T("ERROR: Boot size exceeds reserved SDHC region (size = 0x%x)\r\n"), dwLength));
        return FALSE;
    }

    // Get cached image location
    pImage = OEMMapMemAddr(dwStartAddr, dwStartAddr);

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

    OALMSG(1, (_T("INFO: Programming EBOOT/SBOOT image from flash cache address 0x%x, size = %d\r\n"), pImage, dwLength));
    
    // Write EBOOT to SDHC flash
    pSectorBuf = pImage;
    startSectorAddr = (SECTOR_ADDR)(SDImageCfg.dwBootOffset / flashInfo.dwBytesPerBlock);
    endSectorAddr = startSectorAddr + (SECTOR_ADDR)(dwBootSize / flashInfo.dwBytesPerBlock);

    if((endSectorAddr - startSectorAddr) < g_dwSDHC_RW_NUM)
    {    
        numSectors = endSectorAddr - startSectorAddr;
    }
    else
    {
        numSectors = g_dwSDHC_RW_NUM;
    }
    
    for(sectorAddr = startSectorAddr; sectorAddr < endSectorAddr; )
    {
        if(!SDMMC_WriteSector(sectorAddr, pSectorBuf, NULL, numSectors))
        {
        OALMSG(1, (_T("ERROR: Failed to update EBOOT\r\n")));
            return FALSE;
        }
        
        pSectorBuf += (numSectors * flashInfo.dwBytesPerBlock);

        sectorAddr += numSectors;
        if((endSectorAddr - sectorAddr) < g_dwSDHC_RW_NUM)
        {
            numSectors = endSectorAddr - sectorAddr;
        }
    }

    // Read EBOOT from SDHC flash to verify contents
    pSectorBuf = pImage + dwBootSize;
    if((endSectorAddr - startSectorAddr) < g_dwSDHC_RW_NUM)
    {    
        numSectors = endSectorAddr - startSectorAddr;
    }
    else
    {
        numSectors = g_dwSDHC_RW_NUM;
    }
    
    OALMSG(1, (_T("\r\nINFO: Verifying image\r\n")));
    for(sectorAddr = startSectorAddr; sectorAddr < endSectorAddr; )
    {
        if(!SDMMC_ReadSector(sectorAddr, pSectorBuf, NULL, numSectors))
        {
            OALMSG(1, (_T("ERROR: Failed to read EBOOT\r\n")));
            return FALSE;
        }
        if(memcmp(pImage, pSectorBuf, (numSectors * flashInfo.dwBytesPerBlock)) != 0)
        {
            OALMSG(1, (_T("ERROR: Failed to verify EBOOT\r\n")));
            return FALSE;
        }
        pImage += (numSectors * flashInfo.dwBytesPerBlock);
        sectorAddr += numSectors;
        if((endSectorAddr - sectorAddr) < g_dwSDHC_RW_NUM)
        {
            numSectors = endSectorAddr - sectorAddr;        
        }
    }
    
    OALMSG(1, (_T("INFO: Update of EBOOT completed successfully...\r\n")));

    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function:  SDHCWriteNK
//
//  This function writes to SDHC  memory the OS image stored 
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
BOOL SDHCWriteNK(DWORD dwStartAddr, DWORD dwLength)
{
    FlashInfo flashInfo;
    LPBYTE pSectorBuf, pImage; 
    SECTOR_ADDR sectorAddr, startSectorAddr, endSectorAddr, numSectors;
    UINT32 percentComplete, lastPercentComplete;
    DWORD dwNKSize;

    // Check for SDHC device availability
    //
    if(!g_bSDHCExist)
    {
        OALMSG(1, (_T("WARNING: SDHC device doesn't exist - unable to store image\r\n")));
        return FALSE;
    }

    OALMSG(1, (_T("INFO: Writing NK image to SDHC (please wait)...\r\n")));

    if(!SDMMC_GetInfo(&flashInfo))
    {
        OALMSG(1, (_T("ERROR: Unable to get SDHC flash information\r\n")));
        return FALSE;
    }

    // MX233 Block count reg can only hold 8 bits data, so the block size can not large than (255+1)
    if(SDImageCfg.dwSocId == 233)
        g_dwSDHC_RW_NUM = 256; 
    else
        g_dwSDHC_RW_NUM = 512;

    // Make sure NK length does not exceed reserved SDHC size
    if(dwLength > SDImageCfg.dwNkSize)
    {
        OALMSG(1, (_T("ERROR: NK size exceeds reserved SDHC region (size = 0x%x)\r\n"), dwLength));
        return FALSE;
    }
 
    // Get cached image location
    pImage = OEMMapMemAddr(dwStartAddr, dwStartAddr);

    if (dwLength % flashInfo.dwBytesPerBlock)
    {
        dwNKSize = (dwLength / flashInfo.dwBytesPerBlock + 1) * flashInfo.dwBytesPerBlock;
        // Fill unused space with 0xFF
        memset(pImage + dwLength, 0xFF, (dwNKSize - dwLength));
    }
    else
    {
        dwNKSize = dwLength;
    }

    // The last dword store the actual NK length
    if (SDImageCfg.dwNkSize - dwLength >= NK_UNUSED_BLOCKS *flashInfo.dwBytesPerBlock )
    {
        memset(pImage + SDImageCfg.dwNkSize-flashInfo.dwBytesPerBlock, 0xFF, flashInfo.dwBytesPerBlock - 4 - IMAGE_LENGTH_MAGIC_SIZE);
        memcpy(pImage + SDImageCfg.dwNkSize - 4 - IMAGE_LENGTH_MAGIC_SIZE ,  IMAGE_LENGTH_MAGIC_STRING, IMAGE_LENGTH_MAGIC_SIZE);
        *(DWORD*)(pImage + SDImageCfg.dwNkSize - 4) = dwLength;
    }
    OALMSG(1, (_T("INFO: Programming NK image from flash cache address 0x%x, size = %d\r\n"), pImage, dwLength));

    // Write NK to SDHC flash
    pSectorBuf = pImage;
    startSectorAddr = (SECTOR_ADDR)(SDImageCfg.dwNkOffset / flashInfo.dwBytesPerBlock);
    endSectorAddr = startSectorAddr + (dwNKSize / flashInfo.dwBytesPerBlock);

    if((endSectorAddr - startSectorAddr) < g_dwSDHC_RW_NUM)
    {    
        numSectors = endSectorAddr - startSectorAddr;
    }
    else
    {
        numSectors = g_dwSDHC_RW_NUM;
    }
    
    lastPercentComplete = 0;
    for(sectorAddr = startSectorAddr; sectorAddr < endSectorAddr; )
    {
        if(!SDMMC_WriteSector(sectorAddr, pSectorBuf, NULL, numSectors))
        {
            OALMSG(1, (_T("ERROR: Failed to update NK\r\n")));
            return FALSE;
        }
        
        pSectorBuf += (numSectors * flashInfo.dwBytesPerBlock);
        percentComplete = 100 * (sectorAddr - startSectorAddr + 1) / (endSectorAddr - startSectorAddr);

        // If percentage complete has changed, show the progress
        if(lastPercentComplete != percentComplete)
        {
            lastPercentComplete = percentComplete;
            OEMWriteDebugByte('\r');
            OALMSG(1, (_T("INFO: Program is %d%% complete"), percentComplete));
        }

        sectorAddr += numSectors;
        if((endSectorAddr - sectorAddr) < g_dwSDHC_RW_NUM)
        {
            numSectors = endSectorAddr - sectorAddr;
        }
    }

    OEMWriteDebugByte('\r');
    OALMSG(1, (_T("INFO: Program is %d%% complete"), 100));
    
    // The last dword of last sector store the actual NK length
    if (SDImageCfg.dwNkSize - dwLength >= NK_UNUSED_BLOCKS *flashInfo.dwBytesPerBlock )
    {
        sectorAddr = startSectorAddr + (SECTOR_ADDR)(SDImageCfg.dwNkSize/flashInfo.dwBytesPerBlock) - 1;
        pSectorBuf = pImage + SDImageCfg.dwNkSize -flashInfo.dwBytesPerBlock; 
    
        if(!SDMMC_WriteSector(sectorAddr, pSectorBuf, NULL, 1))
        {
            OALMSG(1, (_T("ERROR: Failed to update NK\r\n")));
            return FALSE;
        }
    }

    OALMSG(1, (_T("\r\nINFO: Reading image in SDHC for verification\r\n")));

    // Read NK from SDHC flash to verify contents
    pSectorBuf = pImage + SDImageCfg.dwNkSize;
    
    if((endSectorAddr - startSectorAddr) < g_dwSDHC_RW_NUM)
    {    
        numSectors = endSectorAddr - startSectorAddr;
    }
    else
    {
        numSectors = g_dwSDHC_RW_NUM;
    }
    
    OALMSG(1, (_T("\r\nINFO: Verifying image\r\n")));

    lastPercentComplete = 0;
    for(sectorAddr = startSectorAddr; sectorAddr < endSectorAddr; )
    {
        if(!SDMMC_ReadSector(sectorAddr, pSectorBuf, NULL, numSectors))
        {
            OALMSG(1, (_T("ERROR: Failed to update NK\r\n")));
            return FALSE;
        }
        if(memcmp(pImage, pSectorBuf, (numSectors * flashInfo.dwBytesPerBlock)) != 0)
        {
            OALMSG(1, (_T("ERROR: Failed to verify NK\r\n")));
            return FALSE;
        }
        pImage += (numSectors * flashInfo.dwBytesPerBlock);
        percentComplete = 100 * (sectorAddr - startSectorAddr + 1) / (endSectorAddr - startSectorAddr);

        // If percentage complete has changed, show the progress
        if(lastPercentComplete != percentComplete)
        {
            lastPercentComplete = percentComplete;
            OEMWriteDebugByte('\r');
            OALMSG(1, (_T("INFO: verify is %d%% complete"), percentComplete));
        }

        sectorAddr += numSectors;
        if((endSectorAddr - sectorAddr) < g_dwSDHC_RW_NUM)
        {
            numSectors = endSectorAddr - sectorAddr;        
        }
    }

    OEMWriteDebugByte('\r');
    OALMSG(1, (_T("INFO: verify is %d%% complete"), 100));
    
    OALMSG(1, (_T("\r\nINFO: Update of NK completed successfully\r\n")));
    
    return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function:  SDHCLoadNK
//
//  This function loads an OS image from SDHC memory into RAM for
//  execution.
//
//  Parameters:
//      None.     
//
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//-----------------------------------------------------------------------------
BOOL SDHCLoadNK(VOID)
{
    FlashInfo flashInfo;
    LPBYTE pSectorBuf;
    SECTOR_ADDR sectorAddr, startSectorAddr, endSectorAddr, numSectors;
    UINT32 percentComplete=0, lastPercentComplete=100;    
    DWORD dwActualLength = 0;
    CHAR *pLengthMagic;

    // Check for SDHC device availability
    //
    if(!g_bSDHCExist)
    {
        OALMSG(1, (_T("WARNING: SDHC device doesn't exist - unable to read image\r\n")));
        return FALSE;
    }

    OALMSG(1, (_T("INFO: Reading NK image to SDHC (please wait)...\r\n")));

    if(!SDMMC_GetInfo(&flashInfo))
    {
        OALMSG(1, (_T("ERROR: Unable to get SDHC flash information\r\n")));
        return FALSE;
    }

    // MX233 Block count reg can only hold 8 bits data, so the block size can not large than (255+1)
    if(SDImageCfg.dwSocId == 233)
        g_dwSDHC_RW_NUM = 256; 
    else
        g_dwSDHC_RW_NUM = 512;

    // Set image load address
    pSectorBuf = (LPBYTE) OALPAtoUA(SDImageCfg.dwNkRAMOffset);    

    // Calculate the physical block range for the NK image
    startSectorAddr = (SECTOR_ADDR)(SDImageCfg.dwNkOffset / flashInfo.dwBytesPerBlock);
    endSectorAddr = startSectorAddr + (SECTOR_ADDR)(SDImageCfg.dwNkSize / flashInfo.dwBytesPerBlock);    

    // Read actual length of NK
    sectorAddr  = endSectorAddr - 1; 

    if(!SDMMC_ReadSector(sectorAddr, pSectorBuf, NULL, 1))
    {
        OALMSG(1, (_T("ERROR: Failed to read NK\r\n")));
        return FALSE;
    }    

    dwActualLength = *(DWORD*)(pSectorBuf +  flashInfo.dwBytesPerBlock - 4);
    pLengthMagic = (CHAR*)(pSectorBuf +  flashInfo.dwBytesPerBlock - 4 - IMAGE_LENGTH_MAGIC_SIZE);

    if ((memcmp(pLengthMagic, IMAGE_LENGTH_MAGIC_STRING, IMAGE_LENGTH_MAGIC_SIZE) == 0) && 
        (dwActualLength!= 0 &&  dwActualLength <= SDImageCfg.dwNkSize))
    {
    
        OALMSG(1, (_T("INFO:  dwActualLength = [0x%x]\r\n"), dwActualLength));
    
        if (dwActualLength % flashInfo.dwBytesPerBlock)
            endSectorAddr = startSectorAddr + (dwActualLength / flashInfo.dwBytesPerBlock) + 1;  
        else
            endSectorAddr = startSectorAddr + (dwActualLength / flashInfo.dwBytesPerBlock);
    }

    OALMSG(1, (_T("INFO: Copying NK image to RAM address 0x%x\r\n"), pSectorBuf));

    lastPercentComplete = 0;
    
    if(endSectorAddr - startSectorAddr < g_dwSDHC_RW_NUM)
    {    
        numSectors = endSectorAddr - startSectorAddr;
    }
    else
    {
        numSectors = g_dwSDHC_RW_NUM;
    }
  
    for(sectorAddr = startSectorAddr; sectorAddr < endSectorAddr;)
    {
        if(!SDMMC_ReadSector(sectorAddr, pSectorBuf, NULL, numSectors))
        {
            OALMSG(1, (_T("ERROR: Failed to load NK\r\n")));
            return FALSE;
        }

        pSectorBuf += (numSectors * flashInfo.dwBytesPerBlock);
        percentComplete = 100 * (sectorAddr - startSectorAddr + 1) / (endSectorAddr - startSectorAddr);

        // If percentage complete has changed, show the progress
        if(lastPercentComplete != percentComplete)
        {
            lastPercentComplete = percentComplete;
            OEMWriteDebugByte('\r');
            OALMSG(1, (_T("INFO: Read is %d%% complete"), percentComplete));
        }

        sectorAddr += numSectors;
        if(endSectorAddr - sectorAddr < g_dwSDHC_RW_NUM)
        {
            numSectors = endSectorAddr - sectorAddr;  
        }
    }

    OEMWriteDebugByte('\r');
    OALMSG(1, (_T("INFO: Read is %d%% complete"), 100));

    OALMSG(1, (_T("\r\nINFO: Copy of NK completed successfully\r\n")));

    return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function:  SDHCFormatAll
//
//  This function formats (erases) all the SDHC regions ( full card capacity)
//
//  Parameters:
//      None.     
//
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//-----------------------------------------------------------------------------
BOOL SDHCFormatAll(void)
{
    FlashInfo flashInfo;
    BLOCK_ID BlockID, startBlockID, endBlockID;
    UINT32 percentComplete, lastPercentComplete;
    DWORD dwNumBlocks;
    BYTE bBuf[ESDHC_SECTOR_SIZE] = { 0 };

    // Check for SDHC device availability
    //
    if(!g_bSDHCExist)
    {
        OALMSG(1, (_T("WARNING: SDHC device doesn't exist - unable to read image\r\n")));
        return FALSE;
    }
    
    // Get SDHC flash data bytes per sector.
    //
    if(!SDMMC_GetInfo(&flashInfo))
    {
        OALMSG(1, (_T("ERROR: Unable to get SDHC flash information\r\n")));
        return FALSE;
    }
    
    // MX233 Block count reg can only hold 8 bits data, so the block size can not large than (255+1)
    if(SDImageCfg.dwSocId == 233)
        g_dwSDHC_RW_NUM = 256; 
    else
        g_dwSDHC_RW_NUM = 512;

    // Calculate the end physical block range for the SD/MMC card
    startBlockID = 0;
    endBlockID = SDMMC_GetCardCapacity();

    OALMSG(1, (_T("INFO: Starting format of SD/MMC card\r\n")));

    /* Erase g_dwSDHC_RW_NUM blocks at a time */
    if(endBlockID - startBlockID < g_dwSDHC_RW_NUM)
    {    
        dwNumBlocks = endBlockID- startBlockID;
    }
    else
    {
        dwNumBlocks = g_dwSDHC_RW_NUM;
    }
    
    lastPercentComplete = 0;
    
    for( BlockID = startBlockID; BlockID < endBlockID; )
    {
        if(!SDMMC_EraseBlock(BlockID, dwNumBlocks))
        {
            OALMSG(1, (_T("ERROR: Unable to erase SDHC  blocks from [0x%x]-[0x%x]\r\n"), BlockID, BlockID + dwNumBlocks));
            return FALSE;
        }

        percentComplete = 100 * (BlockID - startBlockID + 1) / (endBlockID - startBlockID);

        // If percentage complete has changed, show the progress
        if(lastPercentComplete != percentComplete)
        {
            lastPercentComplete = percentComplete;
            OEMWriteDebugByte('\r');
            OALMSG(1, (_T("INFO: Program is %d%% complete"), percentComplete));
        }

        BlockID += dwNumBlocks;
        if(endBlockID - BlockID < dwNumBlocks)
        {
            dwNumBlocks = endBlockID - BlockID;
        }
    }

    if(!SDMMC_WriteSector(0, (LPBYTE)bBuf, NULL, 1))
    {
        OALMSG(1, (_T("MBR Clear Fail\r\n")));
    }
        
    OALMSG(1, (_T("\r\nINFO: Format of SD/MMC completed successfully\r\n")));

    return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function:  SDHCCreateFileSystemPartition
//
//  This function creates an MBR at sector 0 of the card with 2 partitions: 1st is 64 MB for
//   boot images and 2nd is for the FAT32 file system in the rest of the card.
//
//  Parameters:
//      None.     
//
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//-----------------------------------------------------------------------------
BOOL SDHCCreateFileSystemPartition(void)
{
    BOOL fRet = FALSE;
    DWORD dwNumSectorsInCard = SDMMC_GetCardCapacity();
    DWORD mbr[ESDHC_SECTOR_SIZE >> 2];  // 512 bytes

    DWORD startSect1 = (DWORD)(SDImageCfg.dwXldrOffset / ESDHC_SECTOR_SIZE);
    DWORD totSect1 = NUMSECTORS_REQUIRED_FOR_BOOTIMAGES - startSect1;
    DWORD startSect2 = NUMSECTORS_REQUIRED_FOR_BOOTIMAGES;
    DWORD totSect2 = dwNumSectorsInCard - startSect2;

    // PC tool is available for i.MX233,i.MX28 to perform this same task, and that would avoid overwriting the existing boot code
    if(SDImageCfg.dwSocId == 233 || SDImageCfg.dwSocId == 28)
    {
        OALMSG(1, (L"\r\nPlease create file system partition on card on the PC using cfimager.exe tool provided with the BSP\r\n"));
        goto EXIT;

    }
    
    // if number of sectors in card are less than or equal the required num sectors for 64 MBs boot space, 
    // return FALSE because the card does not have any room left for any possible file system
    if(dwNumSectorsInCard <= NUMSECTORS_REQUIRED_FOR_BOOTIMAGES )
    {
        OALMSG(1, (L"\r\nCannot create file system parition in SD/MMC card. Size has to be bigger than 64 MBs.\r\n"));
        goto EXIT;

    }

    // initialize MBR to 0
    memset(mbr, 0, sizeof(mbr));

    // set boot sector signature at the end of the sector
    mbr[MBR_SIGNATURE_OFFSET >> 2] = (DWORD) (MBR_SIGNATURE << 16);

    //
    // Partition 1: This is the 64 MB boot partition that has no file system, just the xldr/eboot/nk images
    //
    
    // low part of start sector at offset 0x1C6
    mbr[MBR_PART1_STARTSECTOR_OFFSET >> 2] = (startSect1 & 0x0000FFFF) << 16;
    // high part of start sector at offset 0x1C8
    mbr[MBR_PART1_TOTALSECTORS_OFFSET >> 2] = (startSect1 & 0xFFFF0000) >> 16;

    // low part of total sectors at offset 0x1CA (and preserve part of start sector in low part, so OR the previous value)
    mbr[MBR_PART1_TOTALSECTORS_OFFSET >> 2] |= ((totSect1 & 0x0000FFFF) << 16);
    // high part of total sectors at offset 0x1CC
    mbr[(MBR_PART1_TOTALSECTORS_OFFSET >> 2) + 1] = (totSect1 & 0xFFFF0000) >> 16;

    //
    // Partition 2: Rest of card will be set aside for FAT32 file system
    //
    
    // low part of start sector at offset 0x1D6
    mbr[MBR_PART2_STARTSECTOR_OFFSET >> 2] = (startSect2 & 0x0000FFFF) << 16;
    // high part of start sector at offset 0x1D8
    mbr[MBR_PART2_TOTALSECTORS_OFFSET >> 2] = (startSect2 & 0xFFFF0000) >> 16;

    // low part of total sectors at offset 0x1DA
    mbr[MBR_PART2_TOTALSECTORS_OFFSET >> 2] |= (totSect2 & 0x0000FFFF) << 16;
    // high part of total sectors at offset 0x1DC
    mbr[(MBR_PART2_TOTALSECTORS_OFFSET >> 2) + 1] = (totSect2 & 0xFFFF0000) >> 16;

    // file system FAT32 at 0x1D2
    mbr[MBR_PART2_FILESYS_OFFSET >> 2] = MBR_PART2_FILESYS << 16;
    
    if(SDMMC_WriteSector(0, (LPBYTE)mbr, NULL, 1))
    {
        fRet = TRUE;
        OALMSG(1, (L"Successfully created 2 partitions \r\n"));
        OALMSG(1, (L" Boot Partition size = %d Kbytes\r\n File System Partition size = %d Kbytes\r\n", totSect1 / 2, totSect2 / 2));
    }
    else
        OALMSG(1, (L"Error creating file system partition on the SD/MMC card.\r\n"));
    
EXIT:
    
    return fRet;
}


//------------------------------------------------------------------------------
//
//  Function:  SDHCLoadBootCFG
//
//  Retrieves bootloader configuration information (menu settings, etc.) from 
//  the SDHC flash.
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
BOOL SDHCLoadBootCFG(BYTE *pBootCfg, DWORD cbBootCfgSize)
{
    FlashInfo flashInfo;
    SECTOR_ADDR startSectorAddr;
    DWORD dwNumSectors, dwSectorNum;
    PBYTE pSectorBuf;

    // Check for SDHC device availability
    //
    if(!g_bSDHCExist)
    {
        OALMSG(1, (_T("WARNING: SDHC device doesn't exist - unable to read image\r\n")));
        return FALSE;
    }
    
    if(!SDMMC_GetInfo(&flashInfo))
    {
        OALMSG(1, (_T("ERROR: Unable to get SDHC flash information\r\n")));
        return FALSE;
    }

    if(cbBootCfgSize > SDImageCfg.dwCfgSize)
    {
        cbBootCfgSize = (DWORD)SDImageCfg.dwCfgSize;
    }

    // Calculate the physical block range for the boot configuration
    startSectorAddr = (SECTOR_ADDR)(SDImageCfg.dwCfgOffset / flashInfo.dwBytesPerBlock);
    dwNumSectors = (cbBootCfgSize + flashInfo.dwBytesPerBlock - 1) / flashInfo.dwBytesPerBlock;

    // For MX233,MX28 the offset is already provided as the sector address
    if(SDImageCfg.dwSocId == 233 || SDImageCfg.dwSocId == 28)
    {
        startSectorAddr = (DWORD)SDImageCfg.dwCfgOffset;
    }
    
    OALMSG(1, (_T("INFO: Loading boot configuration from SDHC\r\n")));

    pSectorBuf = (LPBYTE)OALPAtoUA(SDImageCfg.dwNkRAMOffset);
    for(dwSectorNum = 0; dwSectorNum < dwNumSectors; dwSectorNum++, startSectorAddr++)
    {
        pSectorBuf += (dwSectorNum * flashInfo.dwBytesPerBlock);
        if(!SDMMC_ReadSector(startSectorAddr, pSectorBuf, NULL, 1))
        {
            OALMSG(1, (_T("ERROR: Failed to load boot configuration from SDHC\r\n")));
            return FALSE;
        }
    }

    memcpy(pBootCfg, pSectorBuf, cbBootCfgSize);

    OALMSG(OAL_INFO, (_T("INFO: Successfully loaded boot configuration from SDHC\r\n")));

    return TRUE;
}


//------------------------------------------------------------------------------
//
//  Function:  SDHCStoreBootCFG
//
//  Stores bootloader configuration information (menu settings, etc.) to 
//  the SDHC flash.
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
BOOL SDHCStoreBootCFG(BYTE *pBootCfg, DWORD cbBootCfgSize)
{
    FlashInfo flashInfo;
    SECTOR_ADDR startSectorAddr;
    DWORD dwNumSectors, dwSectorNum;
    PBYTE pSectorBuf;
    DWORD dwExtraSize;

    // Check for SDHC device availability
    //
    if(!g_bSDHCExist)
    {
        OALMSG(1, (_T("WARNING: SDHC device doesn't exist - unable to read image\r\n")));
        return FALSE;
    }
    
    if(!SDMMC_GetInfo(&flashInfo))
    {
        OALMSG(1, (_T("ERROR: Unable to get SDHC flash information\r\n")));
        return FALSE;
    }

    if(cbBootCfgSize > SDImageCfg.dwCfgSize)
    {
        cbBootCfgSize = (DWORD)SDImageCfg.dwCfgSize;
    }

    // Calculate the physical block range for the boot configuration  
    startSectorAddr = (DWORD)(SDImageCfg.dwCfgOffset / flashInfo.dwBytesPerBlock);
    dwNumSectors = (cbBootCfgSize + flashInfo.dwBytesPerBlock - 1) / flashInfo.dwBytesPerBlock;

    // For MX233,MX28 the offset is already provided as the sector address
    if(SDImageCfg.dwSocId == 233 || SDImageCfg.dwSocId == 28)
    {
        startSectorAddr = (DWORD)SDImageCfg.dwCfgOffset;
    }

    // For example, if the cbBootCfgSize is 128Bytes, then the dwExtraSize is (512 - 128)Bytes
    dwExtraSize = dwNumSectors * flashInfo.dwBytesPerBlock - cbBootCfgSize;

    pSectorBuf = (LPBYTE)OALPAtoUA(SDImageCfg.dwNkRAMOffset);
    memcpy(pSectorBuf, pBootCfg, cbBootCfgSize);
    memset(pSectorBuf + cbBootCfgSize, 0xFF, dwExtraSize);

    OALMSG(1, (_T("INFO: Storing boot configuration to SDHC\r\n")));

    for(dwSectorNum = 0; dwSectorNum < dwNumSectors; dwSectorNum++, startSectorAddr++)
    {
        pSectorBuf += (dwSectorNum * flashInfo.dwBytesPerBlock);
        if(!SDMMC_WriteSector(startSectorAddr, pSectorBuf, NULL, 1))
        {
            OALMSG(1, (_T("ERROR: Failed to update BOOT configuration\r\n")));
            return FALSE;
        }
    }

    OALMSG(1, (_T("INFO: Successfully stored boot configuration to SDHC\r\n")));

    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: SDHCDisableBP
//
//    This function will disable the boot partition for eSD2.1 or eMMC 4.3
//    so that when NK loads the boot partition is not overwritten
//
// Parameters:
//        none
//
// Returns:
//        none
//
//------------------------------------------------------------------------------
VOID SDHCDisableBP(VOID)
{
    // if it is eMMC, clear the BOOT_PARTITION_ACCESS bits in the EXT_CSD
    if (SDController.IsMMC)
    {
        // last 3 bits of the BOOT_CONFIG byte are the ACCESS bits, clear them if they are non-zero
        if ( (extCSDBuffer[MMC_EXT_CSD_BOOT_CONFIG] & 0x7) )
        {
            // clear the ACCESS bits, but leave the ENABLE bits intact
            MMC_Send_Switch_Cmd(EMMC_SWITCH_CLEAR_BOOT_PART | (0x7 << EMMC_SWITCH_BOOT_VALUE_SHIFT));
        }

        // update the global extCSDBuffer
        SDMMC_get_ext_csd();   
        
    }

    // if it eSD, then set usr partition to be the active partition
    else
    {
        ESDSetActivePartition(ESD_SET_USR_PARTITION0);
    }

}


//------------------------------------------------------------------------------
//
// Function: EMMCToggleBP
//
//    This function will toggle the eMMC 4.3 boot partition configuration
//
// Parameters:
//        none
//
// Returns:
//        ESDHC_STATUS_PASS for success/ESDHC_STATUS_FAILURE for failure
//
//------------------------------------------------------------------------------
UINT32 EMMCToggleBP(void)
{
    UINT32 status = ESDHC_STATUS_FAILURE;

    if (!SDController.IsMMC)
    {
        OALMSG(OAL_INFO, (L"\r\neMMC not present!\r\n"));
        return status;
    }

    switch(extCSDBuffer[MMC_EXT_CSD_BOOT_CONFIG])
    {
        default:
        case 0x0:
        case 0x8:
        case 0x48:            
            // enable boot partition 1
            MMC_Send_Switch_Cmd(EMMC_SWITCH_WRITE_BOOT_PART | (0x49 << EMMC_SWITCH_BOOT_VALUE_SHIFT));

            // if BP_SIZE is 0, ask user to creat non-zero partition size first
            if (!extCSDBuffer[226] && !extCSDBuffer[227])
                OALMSG(OAL_INFO, (L"\r\nChange boot partition size to non-zero before flashing!\r\n"));         
            break;

        case 0x9:
        case 0x49:
        case 0x50:
        case 0x10:            
            // enable boot partition 2
            MMC_Send_Switch_Cmd(EMMC_SWITCH_WRITE_BOOT_PART | (0x52 << EMMC_SWITCH_BOOT_VALUE_SHIFT));

            // if BP_SIZE is 0, ask user to creat non-zero partition size first
            if (!extCSDBuffer[226] && !extCSDBuffer[227])
                OALMSG(OAL_INFO, (L"\r\nChange boot partition size to non-zero before flashing!\r\n"));                
            break;

        case 0x52:
        case 0x12:            
            // disable boot from boot partition
            MMC_Send_Switch_Cmd(EMMC_SWITCH_WRITE_BOOT_PART | (0x0 << EMMC_SWITCH_BOOT_VALUE_SHIFT));
            break;
    }

    // update the global extCSDBuffer
    status = SDMMC_get_ext_csd();
    return status;
}


//------------------------------------------------------------------------------
//
// Function: EMMCToggleBPSize
//
//    This function will toggle the eMMC 4.3 boot partition size between 0 and 64 MBs
//
// Parameters:
//        none
//
// Returns:
//        ESDHC_STATUS_PASS for success/ESDHC_STATUS_FAILURE for failure
//
//------------------------------------------------------------------------------
UINT32 EMMCToggleBPSize(void)
{
    UINT32 status = ESDHC_STATUS_FAILURE;
    SD_BUS_REQUEST sdRequest;

    // the 3rd argument sets up a 64 MB size for boot partition
    // 4 MB * 2 * boot_arg[2] = 64 MB : boot_arg[2] = 64 MB / 8 MB = (64MB >> 23) = 8   
    DWORD dwPartSizeParameter = ( (NUMSECTORS_REQUIRED_FOR_BOOTIMAGES << 9)  >> 23);

    UINT32 boot_arg[3] = {0xEFAC62EC, 0x00CBAEA7, 0x0};
    UINT32 i = 0;
    
    
    if (!SDController.IsMMC)
    {
        OALMSG(OAL_INFO, (L"\r\neMMC not present!\r\n"));
        return status;
    }

    // Make sure this is eMMC 4.3 from Samsung
    if ((sdCardReg.CID[15] == 0x15) && (sdCardReg.CID[7] == 0x44) && (sdCardReg.CID[8] == 0x44) && (sdCardReg.CID[9] == 0x38)
        && (sdCardReg.CID[10] == 0x47) && (sdCardReg.CID[11] == 0x41) && (sdCardReg.CID[12] == 0x4D) )
    {

        // if BP_SIZE is already non-zero, then zero it out
        if (extCSDBuffer[226] || extCSDBuffer[227])
        {
            OALMSG(OAL_INFO, (L"\r\nChanging boot partition size to 0!\r\n"));
            boot_arg[2] = 0;

            // disable boot partition: including disabling BOOT_PARTITION_ENABLE bits
            MMC_Send_Switch_Cmd(EMMC_SWITCH_WRITE_BOOT_PART | (0x0 << EMMC_SWITCH_BOOT_VALUE_SHIFT));            
        }
        // size of boot partition to be created = # of 4MB sectors / 2
        else
        {
            boot_arg[2] = dwPartSizeParameter;
        }
            
        while(i < 3)
        {

            /* Configure CMD62 to change partition size */
            SDCard_command_config(&sdRequest, EMMC_CMD62,
                                boot_arg[i++], SD_COMMAND, ResponseR1b);        
            
            /* Sending the card from stand-by to transfer state
            */
            if(!SDHCSendCmdWaitResp(&sdRequest))
            {
                if (i >= 3 && !SDMMC_R1b_busy_wait())
                {
                    status = ESDHC_STATUS_PASS;
                }
            }
        
        }
    }

    else
    {
        OALMSG(OAL_INFO, (L"\r\nEither this is not an eMMC 4.3 or this manufacturer (0x%x) is not yet supported\r\n", sdCardReg.CID[15]));
    }
    
    // update the global extCSDBuffer
    status = SDMMC_get_ext_csd();
    
    return status;
}


//------------------------------------------------------------------------------
//
// Function: ESDSetActivePartition
//
//    This function will activate the specified partition on eSD2.1 device
//
// Parameters:
//        dwPartId[in] - partition Id to be activated (in the appropriate format for eSD)
//
// Returns:
//        ESDHC_STATUS_PASS for success/ESDHC_STATUS_FAILURE for failure
//
//------------------------------------------------------------------------------
UINT32 ESDSetActivePartition (DWORD dwPartId)
{
    UINT32 status = ESDHC_STATUS_PASS;
    SD_BUS_REQUEST sdRequest;
    DWORD dwPartNum = dwPartId >> 24;
    
    if (SDController.IsMMC)
    {
        OALMSG(OAL_INFO, (L"\r\neSD not present!\r\n"));
        status = ESDHC_STATUS_FAILURE;
        return status;
    }

    //Configure CMD43 for partition Id
    SDCard_command_config(&sdRequest,ESD_CMD_SET_ACTIVE_PART, dwPartId, SD_COMMAND, ResponseR1b);
    if(!SDHCSendCmdWaitResp(&sdRequest))
    {
        if (!SDMMC_R1b_busy_wait())
        {
            // usr partition is partition 0, while boot partition is partition 1
            if (dwPartNum == 0)
                eSDBPEnabled = FALSE;
            else if (dwPartNum == 1)
                eSDBPEnabled = TRUE;
            
            OALMSG(OAL_VERBOSE, (_T("Partition %d enabled\r\n"), dwPartNum));
        }
        else
        {
            OALMSG(OAL_INFO, (_T("Failed to set active partition %d\r\n"), dwPartNum));
            status = ESDHC_STATUS_FAILURE;            
        }  
    }
    else
    {
        OALMSG(OAL_VERBOSE, (_T("Either card is not eSD2.1, or boot partition is not created yet\r\n"))); 
    }

    return status;

}


//------------------------------------------------------------------------------
//
// Function: ESDQueryPartitionSize
//
//    This function will query the eSD2.1 device for list of partitions 
//
// Parameters:
//        none
//
// Returns:
//       Size of boot partition (in number of sectors)
//
//------------------------------------------------------------------------------
DWORD ESDQueryPartitionSize(void)
{
    DWORD dwBootPartSize = 0;
    SD_BUS_REQUEST sdRequest;    
    DWORD dwQueryPart[SDHC_BLK_LEN >> 2];  // 512 bytes = 128 dwords

    memset(dwQueryPart, 0, SDHC_BLK_LEN);
    
    SDCard_command_config(&sdRequest,ESD_CMD_QUERY_PARTITION, ESD_QRY_PART_QUERY_SIZES, SD_READ, ResponseR1);
    sdRequest.NumBlocks = 1;
    sdRequest.BlockSize = SDHC_BLK_LEN;

    if(SDHCSendCmdWaitResp(&sdRequest) == ESDHC_STATUS_PASS)
    {
        if (SDHCDataRead((UINT32 *)&dwQueryPart[0],SDHC_BLK_LEN) == ESDHC_STATUS_PASS)
        {
            dwBootPartSize = dwQueryPart[1];            
        }
    }

    return dwBootPartSize;
}


//------------------------------------------------------------------------------
//
// Function: ESDToggleBP
//
//    This function will enable/disable boot partition
//
// Parameters:
//        none
//
// Returns:
//        ESDHC_STATUS_PASS for success/ESDHC_STATUS_FAILURE for failure
//
//------------------------------------------------------------------------------
UINT32 ESDToggleBP(void)
{
    UINT32 status = ESDHC_STATUS_PASS;
    
    if (SDController.IsMMC)
    {
        OALMSG(OAL_INFO, (L"\r\neSD not present!\r\n"));
        return status;
    }

    if (eSDBootPartitionSize == 0)
    {
        OALMSG(OAL_INFO, (L"\r\nChange boot partition size to non-zero before flashing!\r\n"));
    }

    // if boot partition is enabled, then enable usr partition; else enable boot partition
    if (eSDBPEnabled)
        ESDSetActivePartition(ESD_SET_USR_PARTITION0);
    else
        ESDSetActivePartition(ESD_SET_BOOT_PARTITION1);

    return status;
}


//------------------------------------------------------------------------------
//
// Function: ESDToggleBPSize
//
//    This function will handle boot partition commands for eSD 2.1 cards
//     Boot partition size will be toggled between 0 and 64 MBs
//
// Parameters:
//        none
//
// Returns:
//        ESDHC_STATUS_PASS for success/ESDHC_STATUS_FAILURE for failure
//
//------------------------------------------------------------------------------
UINT32 ESDToggleBPSize (void)
{
    UINT32 status = ESDHC_STATUS_PASS;
    SD_BUS_REQUEST sdRequest;
    BYTE bMngPartBuf[SDHC_BLK_LEN];
    DWORD dwOpCode = ESD_MNG_PART_SPLIT_PARTS;
    
    if (SDController.IsMMC)
    {
        OALMSG(OAL_INFO, (L"\r\neSD not present!\r\n"));
        return status;
    }

    // first make sure user partition is active
    if(ESDSetActivePartition(ESD_SET_USR_PARTITION0) == ESDHC_STATUS_FAILURE)
    {
        status = ESDHC_STATUS_FAILURE;
        return status;
    }

    memset(bMngPartBuf, 0, SDHC_BLK_LEN);

    // if boot partition is already created, merge back to usr part; otherwise create it (split) from the usr part
    if (eSDBootPartitionSize)
        dwOpCode = ESD_MNG_PART_JOIN_PARTS;

    // if we are going to create it, then setup the boot partition size to 64 MBs
    if (dwOpCode == ESD_MNG_PART_SPLIT_PARTS)
    {
        // boot partition id must equal 1
        bMngPartBuf[0] = 1;
        //partition size = # of sectors to hold 64 MBs, in bytes 4-7
        *((PDWORD)(bMngPartBuf + 4)) = NUMSECTORS_REQUIRED_FOR_BOOTIMAGES;
        
        // Physical partition type = 3 (boot partition): bits 160-163 of partition attributes
        bMngPartBuf[ESD_PART_TYPE_OFFSET] = (BYTE)ESD_PART_TYPE_BOOT;

    }

    // send manage partitions commands with the appropriate opcode (split or join partitions) as argument
    SDCard_command_config(&sdRequest,ESD_CMD_MNG_PART, dwOpCode, SD_WRITE, ResponseR1);
    sdRequest.NumBlocks = 1;
    sdRequest.BlockSize = SDHC_BLK_LEN;
    
    if(!SDHCSendCmdWaitResp(&sdRequest))
    {
        if((status = SDHCDataWrite((UINT32 *)&bMngPartBuf[0],SDHC_BLK_LEN)) == ESDHC_STATUS_PASS)
        {
            status = SDMMC_R1b_busy_wait();
        }
    }

    // get new partition size
    eSDBootPartitionSize = ESDQueryPartitionSize();

    // if we just created a boot partition, activate it
    if (eSDBootPartitionSize)
        status = ESDSetActivePartition(ESD_SET_BOOT_PARTITION1);

    status = ESDHC_STATUS_PASS;
    return status;
}


//------------------------------------------------------------------------------
//
//  Function:  SDReadDataBlock
//
//  This function reads data from SDMMC.
//
//  Parameters:
//      pBuffer 
//          [out] Point to the buffer.
//
//      dwLength 
//          [in] length of data to be read.
//
//      dwAddress 
//          [in] address from which to read.
//
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//-----------------------------------------------------------------------------
BOOL SDReadDataBlock(PVOID pBuffer, DWORD dwLength, DWORD dwAddress)
{
    PBYTE pBuf = NULL;
    static SECTOR_ADDR SectorAddr = 0xFFFFFFFF;
    DWORD dwOffset = 0;
    DWORD dwCacheSize  = SDHC_BLK_LEN*SDHC_CACHE_BLK_NUM;
    static DWORD dwCacheStart = 0xFFFFFFFF, dwCacheEnd = 0;

    if(pBuffer == NULL)
    {
        OALMSG(1, (_T("INFO: pBuffer NULL\r\n")));
        return FALSE;
    }

    pBuf = (PBYTE)pBuffer;

    OALMSG(0, (_T("SDReadDataBlock : dwLength = %d dwAddress = %d\r\n"), dwLength, dwAddress));
    
    while (dwLength > 0)
    {
        if ((dwAddress >= dwCacheStart) && (dwAddress <= dwCacheEnd))
        {
            dwOffset = dwAddress -dwCacheStart;
            if (dwAddress+dwLength<= dwCacheEnd+1)
            { 
                memcpy(pBuf, bCacheBuffer + dwOffset, dwLength);
                return TRUE;
            }
            else
            {
                memcpy(pBuf, bCacheBuffer + dwOffset, dwCacheSize-dwOffset);
                pBuf += dwCacheSize-dwOffset;
                dwLength -= (dwCacheSize-dwOffset);
                dwAddress = dwCacheEnd+1;
            }  
        }
        else if ((dwAddress < dwCacheStart) && (dwAddress+dwLength > dwCacheStart))
        {
            dwOffset = dwCacheStart - dwAddress;
            memcpy(pBuf+dwOffset, bCacheBuffer, dwLength-dwOffset);
            dwLength = dwOffset;
    
        }
        else
        {
            SectorAddr = dwAddress / SDHC_BLK_LEN;
            if(!SDMMC_ReadSector(SectorAddr, (LPBYTE)bCacheBuffer, NULL, SDHC_CACHE_BLK_NUM))
            {
                OALMSG(OAL_ERROR, (_T("ERROR: Failed to read from SDHC\r\n")));
                return FALSE;
            }

            dwCacheStart = SectorAddr*SDHC_BLK_LEN;
            dwCacheEnd = dwCacheStart+dwCacheSize - 1;
        }
    }
    return TRUE;
}


//------------------------------------------------------------------------------
//
//  Function:  SDMMCDownload
//
//  This function prepares for the downloading.
//
//  Parameters:
//      pFileName 
//          [in] Name of the file to be read.
//
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//-----------------------------------------------------------------------------
BOOL SDMMCDownload(PCSTR pFileName)
{
    if(!FATInitDisk())
    {
        OALMSG(1, (L"SDMMCDownload : FATInitDisk Fail\r\n"));
        return FALSE;
    }

    if(!FATOpenFile(&g_fnFileInfo, pFileName))
    {
        OALMSG(OAL_ERROR, (L"ERROR: cann't open image file'%S'\r\n", pFileName));
        return FALSE;
    }

    return TRUE;
}


//------------------------------------------------------------------------------
//
//  Function:  SDMMCReadData
//
//  This function reads the image data from SDMMC.
//
//  Parameters:
//      cbData 
//          [in] Size of data to be read.
//
//      pbData
//          [out] Buffer to receive the data.
//
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//
//-----------------------------------------------------------------------------
BOOL SDMMCReadData(DWORD cbData, LPBYTE pbData)
{
    OALMSG(0, (L"SDMMCReadData : length = %d\r\n", cbData));
    OALMSG(OAL_INFO, (L">"));
    if(FATReadFile(&g_fnFileInfo, pbData, cbData) != cbData)
    {
        OALMSG(OAL_ERROR, (L"ERROR: read image data failure\r\n"));
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}


