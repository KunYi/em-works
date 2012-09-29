//------------------------------------------------------------------------------
//
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  sdboot.cpp
//
//  Implements the SD/MMC boot functions.
//
//-----------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable:  6287 6262 4201 4512 4100 4115 4214 6001)
#include <windows.h>
#pragma warning(pop)
#include <Storemgr.h>
#include <stdio.h>
#include <diskio.h>
#include "sdboot.h"

#define SECTOR_SIZE             (512)
#define SDHC_RW_NUM             (512)
#define MAX_NUM_REGIONS         ((SECTOR_SIZE-16)/12)
#define REGION_BOOTIMAGE        (0)
#define SD_BOOT_RESERVED_SECTOR (33 * 1024 * 1024/SECTOR_SIZE)

// definitions used to discover the Config Block Signature
#define CONFIG_BLOCK_SIGNATURE                      0x00112233
#define CONFIG_BLOCK_SIGNATURE_VERSION              0x00000001
#define CONFIG_BLOCK_SIGNATURE_WORD_POS             0
#define CONFIG_BLOCK_SIGNATURE_VERSION_WORD_POS     1

// definitions used to discover the MBR signature
#define MBR_BLOCK_POSITION                          0
// the boot image is at a fixed sector offset within the i.mx partition
#define MBR_BOOT_OFFSET                             (4)

#define NUM_RETRIES_FOR_STATUS                      (20)

#define BOOTIMAGE_TAG                               (0x50)

typedef struct {
    DWORD        eDriveType;
    DWORD        Tag;
    DWORD        NumBlksInDrive;
} MediaRegion, *PMediaRegion;
 
typedef struct {
    DWORD        Signature;
    DWORD        Version;
    DWORD        NumRegions;
    MediaRegion  Region[MAX_NUM_REGIONS];
} MediaConfigBlock, *PMediaConfigBlock;


DWORD gStartSectorAddr;
HANDLE ghDsk;

BOOL SDMMC_GETCapacity(DWORD *pNumofSectors)
{
    DISK_INFO DiskInfo;
    DWORD dwBytesReturned;
    
    // Write the buffer to the device
    BOOL fRet = DeviceIoControl( ghDsk,       // handle to device
        DISK_IOCTL_GETINFO,              // control code
        & DiskInfo,                        // address of DISK_INFO structure
        sizeof(DISK_INFO),                  // size of DISK_INFO structure
        NULL,                            // output buffer (not used)
        NULL,                            // output buffer size (not used)
        & dwBytesReturned,               // address of DWORD containing number of bytes returned
        NULL);                           // pointer to overlapped structure (not used)

    // Was the write successful?
    if ( !fRet ) {
        RETAILMSG(TRUE, (L"DISK_IOCTL_GETINFO FAILED. GetLastError()=%lu.\r\n",GetLastError()));
        return FALSE;
    }    
    
    *pNumofSectors = DiskInfo.di_total_sectors;
    RETAILMSG(TRUE, (L"Total sectors of SD disk is 0x%x.\r\n",*pNumofSectors));
    
    return TRUE;
}

BOOL SDMMC_WriteSector(DWORD startSectorAddr, LPBYTE pSectorBuff, DWORD dwNumSectors)
{
    BOOL fRet;
    DWORD dwBytesReturned;
    SG_REQ sSGReq;

    // Populate scatter/gather structures for read and write ioctls
    sSGReq.sr_start = startSectorAddr;  // starting sector number
    sSGReq.sr_num_sec = dwNumSectors;   // number of sectors
    sSGReq.sr_num_sg = 1;                   // number of scatter/gather buffers
    sSGReq.sr_callback = NULL;              // request completion callback function
    sSGReq.sr_sglist[0].sb_len = dwNumSectors * SECTOR_SIZE;// length of buffer
    sSGReq.sr_sglist[0].sb_buf = PUCHAR(pSectorBuff);
    
    // Write the buffer to the device
    fRet = DeviceIoControl( ghDsk,       // handle to device
        DISK_IOCTL_WRITE,                   // control code
        & sSGReq,                           // address of DISK_INFO structure
        sizeof(SG_REQ),                 // size of DISK_INFO structure
        NULL,                                   // output buffer (not used)
        NULL,                                   // output buffer size (not used)
        & dwBytesReturned,              // address of DWORD containing number of bytes returned
        NULL);                             // pointer to overlapped structure (not used)

    // Was the write successful?
    if ( !fRet ) {
        RETAILMSG(TRUE, (L"WriteSector FAILED. DISK_IOCTL_WRITE was unable to write to sectors %d through %d. GetLastError()=%lu. SG_REQ.sr_status=%d.",
            startSectorAddr, (startSectorAddr + dwNumSectors - 1), GetLastError(), sSGReq.sr_status));
    }

    return fRet;
}

BOOL SDMMC_ReadSector(DWORD startSectorAddr, LPBYTE pSectorBuff, DWORD dwNumSectors)
{
    BOOL fRet;
    DWORD dwBytesReturned;
    SG_REQ sSGReq;

    // Populate scatter/gather structures for read and write ioctls
    sSGReq.sr_start = startSectorAddr;  // starting sector number
    sSGReq.sr_num_sec = dwNumSectors;   // number of sectors
    sSGReq.sr_num_sg = 1;                   // number of scatter/gather buffers
    sSGReq.sr_callback = NULL;              // request completion callback function
    sSGReq.sr_sglist[0].sb_len = dwNumSectors * SECTOR_SIZE;// length of buffer
    sSGReq.sr_sglist[0].sb_buf = PUCHAR(pSectorBuff);
    
    // Write the buffer to the device
    fRet = DeviceIoControl( ghDsk,       // handle to device
        DISK_IOCTL_READ,                   // control code
        & sSGReq,                           // address of DISK_INFO structure
        sizeof(SG_REQ),                 // size of DISK_INFO structure
        NULL,                                   // output buffer (not used)
        NULL,                                   // output buffer size (not used)
        & dwBytesReturned,              // address of DWORD containing number of bytes returned
        NULL);                             // pointer to overlapped structure (not used)

    // Was the write successful?
    if ( !fRet ) {
        RETAILMSG(TRUE, (L"WriteSector FAILED. DISK_IOCTL_WRITE was unable to write to sectors %d through %d. GetLastError()=%lu. SG_REQ.sr_status=%d.",
            startSectorAddr, (startSectorAddr + dwNumSectors - 1), GetLastError(), sSGReq.sr_status));
    }

    return fRet;
}

BOOL VerifyBCB(DWORD LastSectorAddr)
{
    LPBYTE pBCB;
    DWORD * pu32BCB;
    DWORD u32NumDrives, u32StartBlk, u32Index;  
    
    pBCB = (LPBYTE)LocalAlloc(LPTR, SECTOR_SIZE);
    if(pBCB == NULL)
    {
        RETAILMSG(1, (_T("ERROR: Failed to allocate buffer for boot configuration block.\r\n")));
        goto EXIT;
    } 
    
    memset(pBCB, 0x00, SECTOR_SIZE);
    
    if(!SDMMC_ReadSector(LastSectorAddr, pBCB,1))
    {
        RETAILMSG(1, (_T("ERROR: Failed to read boot configuration block. \r\n")));
        goto EXIT;
    }
    
    // SD device last block is set to a valig config block
    pu32BCB = (DWORD *)pBCB;
 
    // We have a good block, now lets test for config block
    if((pu32BCB[CONFIG_BLOCK_SIGNATURE_WORD_POS] != CONFIG_BLOCK_SIGNATURE) ||
       (pu32BCB[CONFIG_BLOCK_SIGNATURE_VERSION_WORD_POS] != CONFIG_BLOCK_SIGNATURE_VERSION))
    {
        RETAILMSG(1, (_T("ERROR: SD device last block is not a valig config block!\r\n")));
        goto EXIT;
    }
    // set up pointer to point to the number of drives field.
    pu32BCB = &pu32BCB[CONFIG_BLOCK_SIGNATURE_VERSION_WORD_POS + 1];
    
    // Current location in the config block will tell us number of regions.
    u32NumDrives = *pu32BCB;
    DEBUGMSG(1, (_T("INFO: The number of regions is 0x%x. \r\n"),u32NumDrives));
    
    // move pointer past num drives field, to the start of the first
    // region (drive) info structure.  
    pu32BCB++;
    
    // Init start block to 0.
    u32StartBlk = 0;
    
    // Search for the bootmanager drive tag
    for(u32Index = 0; u32Index < u32NumDrives; u32Index++)
    {
        if(pu32BCB[1] == BOOTIMAGE_TAG)
        {
            if(u32Index != REGION_BOOTIMAGE+1)
            {
                RETAILMSG(1, (_T("ERROR: SD device last block is not a valig config block!\r\n")));
                goto EXIT;        
            }            
            DEBUGMSG(1, (_T("INFO: The region stores image address is found : Index = 0x%x! \r\n"),u32Index-1));
            break;
        }
        
        // increment the start block by the number of blocks in this region.
        u32StartBlk += pu32BCB[2];
    
        pu32BCB += 3;
    }
    
    DEBUGMSG(1, (_T("INFO: Image address is 0x%x! \r\n"),u32StartBlk));
    if(u32StartBlk != LastSectorAddr - SD_BOOT_RESERVED_SECTOR)
    {
        RETAILMSG(1, (_T("ERROR: SD device last block is not a valig config block!\r\n")));
        goto EXIT;        
    }    
      
    if(pBCB)
    {
        LocalFree(pBCB);
    }
    
    RETAILMSG(1, (_T("INFO: Verifying boot configuration block succeeds.\r\n")));
    return TRUE;
    
EXIT:
    if(pBCB)
    {
        LocalFree(pBCB);
    }
    return FALSE;
}

BOOL WriteBCB(DWORD LastSectorAddr)
{
    PMediaConfigBlock pMediaConfigBlock;
    LPBYTE pBCB;
    
    RETAILMSG(1, (_T("INFO: Writing boot configuration block...\r\n")));
    if(LastSectorAddr < SD_BOOT_RESERVED_SECTOR)
    {
        RETAILMSG(1, (_T("ERROR: There is not enough disk space to store image.\r\n")));
        return FALSE;
    }
    
    //Create and initialize a buffer for BCB
    pBCB = (LPBYTE)LocalAlloc(LPTR, SECTOR_SIZE);
    if(pBCB == NULL)
    {
        RETAILMSG(1, (_T("ERROR: Failed to allocate buffer for BCB.\r\n")));
        return FALSE;
    } 
    
    memset(pBCB, 0x00, SECTOR_SIZE);
    
    //Initialize BCB
    pMediaConfigBlock = (PMediaConfigBlock)pBCB;
    
    //Set signature and version.
    pMediaConfigBlock->Signature = CONFIG_BLOCK_SIGNATURE;
    pMediaConfigBlock->Version   = CONFIG_BLOCK_SIGNATURE_VERSION;
    
    //Set the number of regions
    pMediaConfigBlock->NumRegions       = MAX_NUM_REGIONS;    
    DEBUGMSG(1, (_T("INFO: The number of regions is 0x%x.\r\n"),pMediaConfigBlock->NumRegions));
    
    //Set the start block address of image stored in SD
    pMediaConfigBlock->Region[REGION_BOOTIMAGE].NumBlksInDrive    = LastSectorAddr - SD_BOOT_RESERVED_SECTOR;
    DEBUGMSG(1, (_T("INFO: Image address is 0x%x.\r\n"),pMediaConfigBlock->Region[REGION_BOOTIMAGE].NumBlksInDrive));

    //We choose second region as a flag to indicate image address is stored in previous region
    pMediaConfigBlock->Region[REGION_BOOTIMAGE+1].Tag    = BOOTIMAGE_TAG;
   
    if(!SDMMC_WriteSector(LastSectorAddr, pBCB,1))
    {
        RETAILMSG(1, (_T("ERROR: Failed to write BCB. \r\n")));
        goto EXIT;
    }    
    
    if(pBCB)
    {
        LocalFree(pBCB);
    }
    
    if(!VerifyBCB(LastSectorAddr))
    {
        RETAILMSG(1, (_T("ERROR: Verification of boot configuration block fails!\r\n")));
        return FALSE;
    }
    
    return TRUE;
    
EXIT:
    if(pBCB)
    {
        LocalFree(pBCB);
    }
    return FALSE;
}

BOOL SDWriteImage(HANDLE hDsk, LPBYTE pImage, DWORD dwLength)
{
    LPBYTE pSectorBuf, pVerifyBuf; 
    DWORD sectorAddr, startSectorAddr, endSectorAddr, dwNumSectors, dwTotalSectors,LastSectorAddr;
    DWORD percentComplete, lastPercentComplete; 

    RETAILMSG(1, (_T("INFO: Writing NK image to SD/MMC media, please wait...\r\n")));

    ghDsk = hDsk;
    
    //Get the last sector address.
    if(!SDMMC_GETCapacity(&LastSectorAddr))
    {
        return FALSE;
    }
    //SD device last block is stored with a valig config block
    LastSectorAddr--;
  
    //first, write boot configuration block.    
    if(!WriteBCB(LastSectorAddr))
    {
        RETAILMSG(1, (_T("ERROR: Writing boot configuration block failed!\r\n")));
        return FALSE;
    }
    
    // Write NK to SDHC flash
    pSectorBuf = pImage;
    pVerifyBuf = (LPBYTE) LocalAlloc(LPTR, SDHC_RW_NUM*SECTOR_SIZE); 
        
    startSectorAddr = LastSectorAddr - SD_BOOT_RESERVED_SECTOR;
    dwTotalSectors = (dwLength+SECTOR_SIZE-1)/SECTOR_SIZE;
    endSectorAddr = startSectorAddr + dwTotalSectors;  

    RETAILMSG(1, (_T("INFO: Totally %d sectors needs to be written.\r\n"), dwTotalSectors));
    
    lastPercentComplete = 0;
    RETAILMSG(1, (_T("\rINFO: Program is %d%% complete"), lastPercentComplete));
    for(sectorAddr = startSectorAddr; sectorAddr < endSectorAddr; )
    {
        // 256KB at a time(512 sectors at a time) 
        if((endSectorAddr - sectorAddr) < SDHC_RW_NUM)
        {    
            dwNumSectors = endSectorAddr - sectorAddr;
        }
        else
        {
            dwNumSectors = SDHC_RW_NUM;
        }  
              
        if(!SDMMC_WriteSector(sectorAddr, pSectorBuf,dwNumSectors))
        {
            RETAILMSG(1, (_T("ERROR: Failed to update NK\r\n")));
            goto EXIT;
        }

        //Verify data
        if(!SDMMC_ReadSector(sectorAddr, pVerifyBuf, dwNumSectors))
        {
            RETAILMSG(1, (_T("ERROR: Failed to update NK\r\n")));
            goto EXIT;
        }

        if (memcmp(pSectorBuf, pVerifyBuf, dwNumSectors*SECTOR_SIZE) != 0)
        {
            RETAILMSG(TRUE, (_T("ERROR: Failed to verify image.\r\n")));
            goto EXIT;
        }
        
        pSectorBuf += (dwNumSectors * SECTOR_SIZE);
        percentComplete = 100 * (sectorAddr - startSectorAddr + 1) / dwTotalSectors;

        // If percentage complete has changed, show the progress
        if(lastPercentComplete != percentComplete)
        {
            lastPercentComplete = percentComplete;
            RETAILMSG(1, (_T("\rINFO: Program is %d%% complete"), percentComplete));
        }

        sectorAddr += dwNumSectors;
    }

    LocalFree(pVerifyBuf);
    RETAILMSG(1, (_T("\rINFO: Program is 100%% complete.\r\n")));
    RETAILMSG(TRUE, (_T("INFO: Verifying image succeeds.\r\n")));    
    RETAILMSG(TRUE, (_T("INFO: Updating of image completed successfully.\r\n")));
    return TRUE;

EXIT:
    LocalFree(pVerifyBuf);
    RETAILMSG(TRUE, (_T("\r\nINFO: Updating image failed.\r\n")));  
    return FALSE;
}

BOOL SDPrePartialWriteImage(HANDLE hDsk, DWORD dwLength)
{
    DWORD LastSectorAddr,dwTotalSectors;

    ghDsk = hDsk;
    
    //Get the last sector address.
    if(!SDMMC_GETCapacity(&LastSectorAddr))
    {
        return FALSE;
    }
    LastSectorAddr--;
  
    //first, write boot configuration block.    
    if(!WriteBCB(LastSectorAddr))
    {
        RETAILMSG(1, (_T("ERROR: Writing boot configuration block failed!\r\n")));
        return FALSE;
    }
    
    gStartSectorAddr = LastSectorAddr - SD_BOOT_RESERVED_SECTOR;
    dwTotalSectors = (dwLength+SECTOR_SIZE-1)/SECTOR_SIZE;
    
    RETAILMSG(1, (_T("INFO: Totally %d sectors needs to be written.\r\n"), dwTotalSectors));

    return TRUE;   
}

BOOL SDPartialWriteImage(LPBYTE pImage, DWORD dwLength)
{
    DWORD sectorAddr, endSectorAddr, dwNumSectors, dwTotalSectors;    
    LPBYTE pSectorBuf, pVerifyBuf;    
    
    dwTotalSectors = (dwLength+SECTOR_SIZE-1)/SECTOR_SIZE;
    endSectorAddr = gStartSectorAddr + dwTotalSectors;  
    
    pSectorBuf = pImage;
    pVerifyBuf = (LPBYTE) LocalAlloc(LPTR, SDHC_RW_NUM*SECTOR_SIZE);  
    
    for(sectorAddr = gStartSectorAddr; sectorAddr < endSectorAddr; )
    {
        // 256KB at a time(512 sectors at a time) 
        if((endSectorAddr - sectorAddr) < SDHC_RW_NUM)
        {    
            dwNumSectors = endSectorAddr - sectorAddr;
        }
        else
        {
            dwNumSectors = SDHC_RW_NUM;
        }              
    
        if(!SDMMC_WriteSector(sectorAddr, pSectorBuf,dwNumSectors))
        {
            RETAILMSG(1, (_T("ERROR: Failed to update NK\r\n")));
            goto EXIT;
        }

        //Verify data
        if(!SDMMC_ReadSector(sectorAddr, pVerifyBuf, dwNumSectors))
        {
            RETAILMSG(1, (_T("ERROR: Failed to update NK\r\n")));
            goto EXIT;
        }

        if (memcmp(pSectorBuf, pVerifyBuf, dwNumSectors*SECTOR_SIZE) != 0)
        {
            RETAILMSG(TRUE, (_T("ERROR: Failed to verify image.\r\n")));
            goto EXIT;
        }
        
        pSectorBuf += (dwNumSectors * SECTOR_SIZE);
        sectorAddr += dwNumSectors;
    }   
    LocalFree(pVerifyBuf);
    gStartSectorAddr = endSectorAddr; 
    return TRUE;

EXIT:
    LocalFree(pVerifyBuf);
    RETAILMSG(TRUE, (_T("\r\nINFO: Updating image failed.\r\n")));  
    return FALSE;    
}

BOOL SDEndPartialWriteImage()
{
    RETAILMSG(TRUE, (_T("INFO: Verifying image succeeds.\r\n")));    
    RETAILMSG(TRUE, (_T("INFO: Updating of image completed successfully.\r\n")));
    return TRUE;      
}




