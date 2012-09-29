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
#include "uce_media.h"
#include "sdboot.h"

#pragma pack(push, 1)

#define SECTOR_SIZE             (512)
#define SDHC_RW_NUM             (512)
#define MAX_NUM_REGIONS         ((SECTOR_SIZE-16)/12)
#define REGION_BOOTIMAGE        (0)
#define SD_BOOT_BOOTIMAGE_SECTOR 64*1024*1024/SECTOR_SIZE//for *.sb(eboot.sb,nk.sb) file
#define SD_BOOT_NKIMAGE_SECTOR   32*1024*1024/SECTOR_SIZE//for NK
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

//--------------------------------------------------------------------------------
#define PART_BOOTABLE           0x80    // bootable partition
#define PART_NON_BOOTABLE       0x0     // non-bootable partition
#define PART_READONLY           0x02

// Part_FileSystem
#define PART_UNKNOWN            0
#define PART_DOS2_FAT           0x01    // legit DOS partition
#define PART_DOS3_FAT           0x04    // legit DOS partition(FAT16)
#define PART_EXTENDED           0x05    // legit DOS partition
#define PART_DOS4_FAT           0x06    // legit DOS partition
#define PART_DOS32              0x0B    // legit DOS partition (FAT32)

#define PARTTABLE_ENTRIES_NUM   4
#define PARTTABLE_ENTRIES_SIZE  16
#define MBR_SIGNATURE_SIZE      2

#define CODEAREA_SIZE        (SECTOR_SIZE - MBR_SIGNATURE_SIZE - (PARTTABLE_ENTRIES_NUM * PARTTABLE_ENTRIES_SIZE))

typedef __unaligned struct _PARTITION_TABLE
{
    UINT32  BootIndicator:8;
    UINT32  StartingHead:8;
    UINT32 StartingSector:6;
    UINT32 StartingCylinder:10;
    UINT32  SystemId:8;
    UINT32  EndingHead:8;
    UINT32 EndingSector:6;
    UINT32 EndingCylinder:10;
    UINT32 RelativeSector;
    UINT32 TotalSector;
}PARTITION_TABLE, *PPARTITION_TABLE;

typedef __unaligned struct _MASTER_BOOT_RECORD
{
    UINT8           CodeArea[CODEAREA_SIZE];
    PARTITION_TABLE Partition[PARTTABLE_ENTRIES_NUM];
    UINT16          Signature;
}MASTER_BOOT_RECORD, *PMASTER_BOOT_RECORD;

#define FIRMWARE_CONFIG_BLOCK_SIGNATURE     (0x00112233) 
#define PRIMARY_TAG                         (0x484C494E)
#define SECONDARY_TAG                       (0x4E494C48)

typedef struct _DriveInfo_t
{
    UINT32    u32ChipNum;             //!< Chip Select, ROM does not use it
    UINT32    u32DriveType;           //!< Always system drive, ROM does not use it
    UINT32    u32Tag;                 //!< Drive Tag
    UINT32    u32FirstSectorNumber;   //!< For BA-NAND devices, this number should be divisible by 4, 
                                        //!< Protocol is set to 4 sectors of 512 bytes. 
                                        //!< Firmware can start at sectors 4, 8, 12, 16,....
    UINT32    u32SectorCount;         //!< Not used by ROM
} DriveInfo_t;

typedef struct _BOOT_CONFIGBLOCK
{
    UINT32    u32Signature;           //!< Signature 0x00112233
    UINT32    u32PrimaryBootTag;      //!< Primary boot drive identified by this tag
    UINT32    u32SecondaryBootTag;    //!< Secondary boot drive identified by this tag
    UINT32    u32NumCopies;           //!< Num elements in aFWSizeLoc array
    DriveInfo_t aDriveInfo[2];           //!< Let array aDriveInfo be last in this data 
                                        //!< structure to be able to add more drives in future without changing ROM code
} BOOT_CONFIGBLOCK;
//--------------------------------------------------------------------------------
DWORD gStartSectorAddr;
DWORD g2ndStartSectorAddr;
HANDLE ghDsk;

void SDGetHandle(HANDLE hDsk)
{
    ghDsk = hDsk;
}

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

    PREFAST_SUPPRESS(6001,"This Warning can be skipped!")        
    *pNumofSectors = DiskInfo.di_total_sectors;   
    RETAILMSG(TRUE, (L"Total sectors of SD disk is 0x%x.\r\n",*pNumofSectors));
    
    return TRUE;
}

BOOL SDMMC_WriteSector(DWORD startSectorAddr, LPBYTE pSectorBuff, DWORD dwNumSectors)
{
    BOOL fRet;
    DWORD dwBytesReturned;
    SG_REQ sSGReq;

    DEBUGMSG(1,(_T("SDMMC_WriteSector begin:startSectorAddr=0x%x, pSectorBuff=0x%x, dwNumSectors=0x%x\r\n"),
            startSectorAddr,pSectorBuff,dwNumSectors));
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

    DEBUGMSG(1,(_T("SDMMC_ReadSector begin:startSectorAddr=0x%x, pSectorBuff=0x%x, dwNumSectors=0x%x\r\n"),
            startSectorAddr,pSectorBuff,dwNumSectors));
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
        RETAILMSG(TRUE, (L"ReadSector FAILED. DISK_IOCTL_READ was unable to read sectors %d through %d. GetLastError()=%lu. SG_REQ.sr_status=%d.\r\n",
            startSectorAddr, (startSectorAddr + dwNumSectors - 1), GetLastError(), sSGReq.sr_status));
    }

    return fRet;
}

//Init handle and start address for SD memory
BOOL SDPreSet(DWORD startAddr, DWORD dwValidDataLength)
{   
    DWORD TotalNumofScts;
    
    //Get the last sector address.
    if(!SDMMC_GETCapacity(&TotalNumofScts))
    {
        return FALSE;
    }

    if((startAddr + dwValidDataLength) >= TotalNumofScts*SECTOR_SIZE){
        RETAILMSG(1, (_T("The required memory space 0x%x exceeds max capacity 0x%x!\r\n") \
            , dwValidDataLength, TotalNumofScts));
        return FALSE;    
    }
    
    gStartSectorAddr = startAddr/SECTOR_SIZE;

    return TRUE;
}

//Write raw data
BOOL SDWriteRawData(PBYTE pbData, DWORD dwValidDataLength)
{   
    DEBUGMSG(1, (_T("WriteSector: start address = 0x%x, sector number = 0x%x., Buffer addr = 0x%x.\r\n") \
        ,gStartSectorAddr, dwValidDataLength/SECTOR_SIZE,pbData));

    if(!SDMMC_WriteSector(gStartSectorAddr, pbData, dwValidDataLength/SECTOR_SIZE))
    {
        RETAILMSG(1, (_T("ERROR: Failed to write raw data to SDMMC media. \r\n")));
        return FALSE;
    }  

    RETAILMSG(1, (_T("Succeeded to write raw data to SDMMC media. \r\n")));
    gStartSectorAddr += dwValidDataLength/SECTOR_SIZE;

    return TRUE;
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

BOOL SDWriteImage(LPBYTE pImage, DWORD dwLength)
{
    LPBYTE pSectorBuf, pVerifyBuf; 
    DWORD sectorAddr, startSectorAddr, endSectorAddr, dwNumSectors, dwTotalSectors,LastSectorAddr;
    DWORD percentComplete, lastPercentComplete; 

    RETAILMSG(1, (_T("INFO: Writing NK image to SD/MMC media, please wait...\r\n")));
    
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
    RETAILMSG(1, (_T("\rINFO: Program is %d%% complete\r\n"), lastPercentComplete));
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

BOOL SDPrePartialWriteImage(DWORD dwLength)
{
    DWORD LastSectorAddr,dwTotalSectors;
    
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

void DUMP_MBR(INT iPartition,PMASTER_BOOT_RECORD pMBR)
{
    int i;
    
    if(!pMBR)
        return;

    DEBUGMSG(1,(_T("\r\n")));
    //dump MBR contents
    DEBUGMSG(TRUE, (_T("MBR Signature = 0x%x\r\n"), pMBR->Signature));
    
    for(i = 0; i < iPartition; i++)
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
    RETAILMSG(1,(_T("\r\n")));
}

void DUMP_MBRModeBCB(BOOT_CONFIGBLOCK * pBCB)
{
    int i;

    if(!pBCB)
        return;

    for ( i=0 ; i<2 ; i++ )
    {
        DEBUGMSG(1, (_T("pBCB->aDriveInfo[%d].u32ChipNum=0x%x\r\n"),i,pBCB->aDriveInfo[i].u32ChipNum));
        DEBUGMSG(1, (_T("pBCB->aDriveInfo[%d].u32DriveType=0x%x\r\n"),i,pBCB->aDriveInfo[i].u32DriveType));
        DEBUGMSG(1, (_T("pBCB->aDriveInfo[%d].u32FirstSectorNumber=0x%x\r\n"),i,pBCB->aDriveInfo[i].u32FirstSectorNumber));
        DEBUGMSG(1, (_T("pBCB->aDriveInfo[%d].u32SectorCount=0x%x\r\n"),i,pBCB->aDriveInfo[i].u32SectorCount));
        DEBUGMSG(1, (_T("pBCB->aDriveInfo[%d].u32Tag=0x%x\r\n"),i,pBCB->aDriveInfo[i].u32Tag));
    }
    DEBUGMSG(1, (_T("pBCB->u32NumCopies=0x%x\r\n"), pBCB->u32NumCopies));
    DEBUGMSG(1, (_T("pBCB-->u32PrimaryBootTag=0x%x\r\n"), pBCB->u32PrimaryBootTag));
    DEBUGMSG(1, (_T("pBCB->u32SecondaryBootTag=0x%x\r\n"), pBCB->u32SecondaryBootTag));
    DEBUGMSG(1, (_T("pBCB->u32Signature=0x%x\r\n"), pBCB->u32Signature));
}

void GetSDStorereInfo(HANDLE hStore,STOREINFO *pStoreInfo)
{    
    if( !GetStoreInfo(hStore, pStoreInfo) )
    {
        RETAILMSG(1,(_T("ERR: Could not get information for disk \"%s\".\r\n"), pStoreInfo->szDeviceName));
    }
    DEBUGMSG(1,(_T("GetStoreInfo: szDeviceName is %s\r\n"),pStoreInfo->szDeviceName));
    DEBUGMSG(1,(_T("              szStoreName is %s\r\n"),pStoreInfo->szStoreName));
    DEBUGMSG(1,(_T("              snNumSectors is 0x%x\r\n"),pStoreInfo->snNumSectors ));
    DEBUGMSG(1,(_T("              dwBytesPerSector is 0x%x\r\n"),pStoreInfo->dwBytesPerSector ));
    DEBUGMSG(1,(_T("              snFreeSectors is 0x%x\r\n"),pStoreInfo->snFreeSectors ));
    DEBUGMSG(1,(_T("              snBiggestPartCreatable is 0x%x\r\n"),pStoreInfo->snBiggestPartCreatable));
    DEBUGMSG(1,(_T("              dwPartitionCount is 0x%x\r\n"),pStoreInfo->dwPartitionCount));
    DEBUGMSG(1,(_T("              dwMountCount is 0x%x\r\n"),pStoreInfo->dwMountCount ));
    DEBUGMSG(1,(_T("\r\n"))); 
}

void GetSDPartInfo(HANDLE hPart)
{
    PARTINFO PartInfo;
    PartInfo.cbSize = sizeof(PartInfo);
    if(!GetPartitionInfo(hPart,&PartInfo))
    {
        RETAILMSG(1,(_T("ERR: Could not get information for partition \"%s\".\r\n"), PartInfo.szPartitionName));
    }
    DEBUGMSG(1,(_T("GetPartitionInfo: szPartitionName is %s\r\n"),PartInfo.szPartitionName ));
    DEBUGMSG(1,(_T("                  szFileSys is %s\r\n"),PartInfo.szFileSys ));
    DEBUGMSG(1,(_T("                  szVolumeName is %s\r\n"),PartInfo.szVolumeName ));
    DEBUGMSG(1,(_T("                  snNumSectors  is 0x%x\r\n"),PartInfo.snNumSectors  ));
    DEBUGMSG(1,(_T("                  bPartType  is 0x%x\r\n"),PartInfo.bPartType )); 
    DEBUGMSG(1,(_T("                  dwAttributes   is 0x%x\r\n"),PartInfo.dwAttributes  )); 
    DEBUGMSG(1,(_T("\r\n")));
}

BOOL GetPartitionList(HANDLE hDsk)
{
    HANDLE hFind = INVALID_HANDLE_VALUE;
    PARTINFO partInfo = {0};
    TCHAR szPartName[15];
    partInfo.cbSize = sizeof (PARTINFO);

    hFind = FindFirstPartition(hDsk, &partInfo);
    if(INVALID_HANDLE_VALUE != hFind)
    {
        do
        {           
            ZeroMemory(szPartName,15);
            // append a * to the name if the partition is mounted
            if(PARTITION_ATTRIBUTE_MOUNTED & partInfo.dwAttributes)
            {
                StringCbPrintf(szPartName, sizeof(szPartName), _T("%s *"), partInfo.szPartitionName);
            }
            else
            {
                StringCbPrintf(szPartName, sizeof(szPartName), _T("%s  "), partInfo.szPartitionName);
            }
            szPartName[15-1] = 0;

            RETAILMSG(1,(_T("\r\n")));
            RETAILMSG(1,(_T("Find Partition name is %s\r\n"),szPartName));
            RETAILMSG(1,(_T("GetPartitionList: szPartitionName is %s\r\n"),partInfo.szPartitionName ));
            RETAILMSG(1,(_T("                  szFileSys is %s\r\n"),partInfo.szFileSys ));
            RETAILMSG(1,(_T("                  szVolumeName is %s\r\n"),partInfo.szVolumeName ));
            RETAILMSG(1,(_T("                  snNumSectors  is 0x%x\r\n"),partInfo.snNumSectors  ));
            RETAILMSG(1,(_T("                  bPartType  is 0x%x\r\n"),partInfo.bPartType ));
        }
        while(FindNextPartition(hFind, &partInfo));

        FindClosePartition (hFind);
    }
    else
    {
        RETAILMSG(1,(_T("INFO: Can not find partitions.\r\n")));
        return FALSE;
    }

    return TRUE;
}    

BOOL CreatePartitions(HANDLE hDsk,INT iPartition,PARTITION_CONT partCont)
{
    INT i=0;
    TCHAR szTemp[10];
    TCHAR szPartName[15];
    BOOL bRet;
    HANDLE hPart;
    SECTORNUM partNum[4]={0};
    STOREINFO StoreInfo;
    StoreInfo.cbSize = sizeof(StoreInfo);   

    DismountStore(hDsk);

    FormatStore(hDsk);

    //Get the sector number for each partition
    GetSDStorereInfo(hDsk,&StoreInfo);
    
    //Get the sector number for each partition for ST
    if(iPartition == 3)//for ST
    {
        for(i =0; i<iPartition;i++)
        { 
            //Part0-File,Part1-Eboot,Part2-NK
            if( wcsncmp(partCont.PartName[i],_T("EBOOT"),wcslen(_T("EBOOT")))==0 )//Partition-1 is for eboot used
            {
                partNum[1] = partCont.iPartSize[0]*1024*1024/StoreInfo.dwBytesPerSector;
            }
            else if ( wcsncmp(partCont.PartName[i],_T("NK"),wcslen(_T("NK")))==0 )//Partition-2 is for nk used
            {
                partNum[2] = partCont.iPartSize[1]*1024*1024/StoreInfo.dwBytesPerSector;
            }
            else if ( wcsncmp(partCont.PartName[i],_T("File"),wcslen(_T("File")))==0 )//Partition-0 is for FAT
            {
                partNum[0] = StoreInfo.snBiggestPartCreatable - (partNum[1]+1) - (partNum[2]+1);
            }
            else
            {
    			RETAILMSG(1,(_T("Error: There is no partition name.\r\n")));
                return FALSE;
            }
        }

        for(i =0; i<iPartition;i++)
        {
            partCont.iPartSize[i] = (INT)partNum[i]*StoreInfo.dwBytesPerSector/(1024*1024);
            ZeroMemory(partCont.PartName[i],15);
            if(i == 0)
                wcsncpy(partCont.PartName[i],_T("File"),wcslen(_T("File")));
            else if(i == 1)
                wcsncpy(partCont.PartName[i],_T("EBOOT"),wcslen(_T("EBOOT")));
            else
                wcsncpy(partCont.PartName[i],_T("NK"),wcslen(_T("NK"))); 
            RETAILMSG(1, (_T("%s Partition %d:%dMB done!\r\n"),partCont.PartName[i],i,partCont.iPartSize[i]));
        }
    }

    //Create the partition
    for(i =0; i<iPartition;i++)
    { 
        memset(szTemp,0,10);
        memset(szPartName,0,15);
        wcscpy(szPartName,_T("Part0"));
        _itow(i,szTemp,10);
        wcscat(szPartName,szTemp);          

        if(iPartition == 2)//for MX
        {
             //Part0-Firmware,Part1-File
            if( wcsncmp(partCont.PartName[i],_T("Firmware"),wcslen(_T("Firmware")))==0 )
            {
                partNum[0] = partCont.iPartSize[i]*1024*1024/StoreInfo.dwBytesPerSector;
            }
            else if( wcsncmp(partCont.PartName[i],_T("File"),wcslen(_T("File")))==0 )
            {
                GetSDStorereInfo(hDsk,&StoreInfo);
                partNum[1] = StoreInfo.snBiggestPartCreatable;
            } 
            else
            {
                RETAILMSG(1,(_T("Error: There is no partition name.\r\n")));
                return FALSE;
            }
        }
        
        RETAILMSG(1,(_T("Create %s Partition.Partition sector number is %dM\r\n"),szPartName,partNum[i]*512/(1024*1024))); 
        bRet = CreatePartitionEx(hDsk,szPartName,PART_UNKNOWN,partNum[i]);
        if(!bRet)
        {
            RETAILMSG(1,(_T("Create %d Partition %s fail.Get Err code is %d\r\n"),i,szPartName,GetLastError()));
            return FALSE;
        }

        if( wcsncmp(partCont.PartName[i],_T("File"),wcslen(_T("File")))==0 )
        {   
            hPart = OpenPartition(hDsk, szPartName);
            if(hPart == INVALID_HANDLE_VALUE)
            {
                RETAILMSG(1,(_T("ERROR: OpenPartition %s fail.\r\n"),szPartName));
                return FALSE;
            }
        
    		DismountPartition(hPart);

            if(!FormatPartitionEx(hPart,0x0B,FALSE))
            {
                RETAILMSG(1,(_T("ERROR: FormatPartitionEx fail.\r\n")));
                return FALSE;
            }

            if(!MountPartition(hPart))
            {
                RETAILMSG(1,(_T("ERROR: MountPartition fail.\r\n")));
                return FALSE;   
            }

            CloseHandle(hPart);
        }
    }

    return TRUE;
}

BOOL SDCreatePartitions(HANDLE hDsk,INT iPartition,PARTITION_CONT partCont)
{ 
	DWORD dwUsedSectorNum = 0;
    STOREINFO StoreInfo;
    StoreInfo.cbSize = sizeof(StoreInfo);   

    //Check if storage is enough
    GetSDStorereInfo(hDsk,&StoreInfo);
    for(INT i=0;i<iPartition;i++)
        dwUsedSectorNum += partCont.iPartSize[i]*1024*1024/SECTOR_SIZE; 
    if(StoreInfo.snNumSectors < dwUsedSectorNum)
    {
        RETAILMSG(1, (_T("card capacity not enough, please change another card!\r\n")));
        return FALSE;
    }

    //For Mx,two partitions:Part0-Firmware,Part1-File
    //For ST,two partitions:Part0-File,Part1-Eboot,Part2-NK
    if(!CreatePartitions(hDsk,iPartition,partCont))
    {
        RETAILMSG(1, (_T("ERROR: Create partition fail!\r\n")));
        return FALSE;
    }

    return TRUE;
}

BOOL CreateOnePartition(HANDLE hDsk,INT iPartition,PARTITION_CONT partCont)
{
    STOREINFO StoreInfo;
    StoreInfo.cbSize = sizeof(StoreInfo); 
	BOOL bRet;
    SECTORNUM snNum; 

    DismountStore(hDsk);

    FormatStore(hDsk);

    //Part0-File,Part1-Eboot,Part2-NK
    //Get every partition sector num
    GetSDStorereInfo(hDsk,&StoreInfo);
    for(INT i=0;i<iPartition;i++)
    {
        snNum = StoreInfo.snBiggestPartCreatable - partCont.iPartSize[i]*1024*1024/StoreInfo.dwBytesPerSector;
    }
    bRet = CreatePartitionEx(hDsk,_T("Part00"),PART_DOS32,StoreInfo.snBiggestPartCreatable);
    if(!bRet)
    {
        RETAILMSG(1,(_T("CreatePartition %s fail.Get Err code is %d\r\n"),_T("Part00"),GetLastError()));
        return FALSE;
    }

    return TRUE;
}


//Calculate MBR value and Write MBR to first sector directly.
//But if the SD does't format,and it  may not write file.
//So add one function CreateOnePartition before write MBR.It will can write files.
BOOL SDWriteMBR(HANDLE hDsk,INT iPartition,PARTITION_CONT partCont)
{
    char *buf = NULL;
    char *pVerifyBuf = NULL;
        
    buf = (char*) LocalAlloc(LPTR, 512);
    if(buf == NULL)
        goto EXIT;
    ZeroMemory(buf,512);
    PMASTER_BOOT_RECORD pMBR = (PMASTER_BOOT_RECORD)buf;
    pVerifyBuf = (char*) LocalAlloc(LPTR, 512);
    if(pVerifyBuf == NULL)
        goto EXIT;
    ZeroMemory(pVerifyBuf,512);
    PMASTER_BOOT_RECORD pVerifyMBR = (PMASTER_BOOT_RECORD)pVerifyBuf;
    DWORD dwTotalSectorNum = 0,dwUsedSectorNum = 0;
    STOREINFO StoreInfo;
    StoreInfo.cbSize = sizeof(StoreInfo);   
    
    //Check if storage is enough
    GetSDStorereInfo(hDsk,&StoreInfo);
    dwTotalSectorNum = (DWORD)StoreInfo.snNumSectors;
    for(INT i=0;i<iPartition;i++)
        dwUsedSectorNum += partCont.iPartSize[i]*1024*1024/SECTOR_SIZE; 
    if(StoreInfo.snNumSectors < dwUsedSectorNum)
    {
        RETAILMSG(1, (_T("card capacity not enough, please change another card!\r\n")));
        goto EXIT;
    }

    //Write MBR will not create partitions at first time.So firstly create&format one partition before writing MBR.
    CreateOnePartition(hDsk,iPartition,partCont);
    
    //Config MBR parameters
    pMBR->Signature = 0xAA55;

    // For MX,two partitions
    if(iPartition == 2)
    {
        // first one is Firmware 
        pMBR->Partition[0].BootIndicator = PART_READONLY;
        pMBR->Partition[0].SystemId = 0x0;
        pMBR->Partition[0].RelativeSector = 0x2;
        pMBR->Partition[0].TotalSector = partCont.iPartSize[0]*1024*1024/SECTOR_SIZE;
        
        // second one is for FAT
        pMBR->Partition[1].BootIndicator = PART_BOOTABLE;
        pMBR->Partition[1].SystemId = PART_DOS32;
        pMBR->Partition[1].RelativeSector = pMBR->Partition[0].RelativeSector + pMBR->Partition[0].TotalSector;
        pMBR->Partition[1].TotalSector = dwTotalSectorNum - pMBR->Partition[1].RelativeSector;
    }
    
    // For ST,three partitions
    if(iPartition == 3)
    {
        // first one is FAT 
        pMBR->Partition[0].BootIndicator = PART_BOOTABLE;
        pMBR->Partition[0].SystemId = PART_DOS32;
        pMBR->Partition[0].RelativeSector = 1;//0 Sector is record MBR
        pMBR->Partition[0].TotalSector = dwTotalSectorNum - (partCont.iPartSize[0] + partCont.iPartSize[1])*1024*1024/SECTOR_SIZE -1;
        
        // second one is boot
        pMBR->Partition[1].BootIndicator = PART_READONLY;
        pMBR->Partition[1].SystemId = 'S';
        pMBR->Partition[1].RelativeSector = pMBR->Partition[0].RelativeSector + pMBR->Partition[0].TotalSector;
        pMBR->Partition[1].TotalSector = partCont.iPartSize[0]*1024*1024/SECTOR_SIZE;
        
        // third one is raw binary
        pMBR->Partition[2].BootIndicator = PART_READONLY;
        pMBR->Partition[2].SystemId = 0x10;
        pMBR->Partition[2].RelativeSector = pMBR->Partition[1].RelativeSector + pMBR->Partition[1].TotalSector;
        pMBR->Partition[2].TotalSector = partCont.iPartSize[1]*1024*1024/SECTOR_SIZE;
    }
        
    for(i = 0; i < iPartition; i++)
    {
        pMBR->Partition[i].StartingSector = (BYTE)(pMBR->Partition[i].RelativeSector + 1) & 0x3F;
        pMBR->Partition[i].EndingSector = (BYTE)(pMBR->Partition[i].RelativeSector + pMBR->Partition[i].TotalSector) & 0x3F;
    }

    //Write MBR to first sector
    if(!SDMMC_WriteSector(0, (LPBYTE)pMBR, 1))
    {
        RETAILMSG(1, (_T("ERROR: Failed to Write MBR to SDHC\r\n")));
        goto EXIT;
    }

    //Verify MBR
    memset(pVerifyMBR, 0x0, sizeof(MASTER_BOOT_RECORD));
    //Read MBR to first sector
    if(!SDMMC_ReadSector(0, (LPBYTE)pVerifyMBR, 1))
    {
        RETAILMSG(1, (_T("ERROR: Failed to Read MBR from SDHC\r\n")));
        goto EXIT;
    }
    DUMP_MBR(iPartition,pVerifyMBR);

    if(memcmp(pMBR,pVerifyMBR,SECTOR_SIZE) != 0)
    {
        RETAILMSG(1,(_T("Fail to verify MBR!\r\n")));
        goto EXIT;
    }

    DEBUGMSG(1, (_T("WriteMBR: Write MBR to SDHC Finished!\r\n")));

    LocalFree(buf);
    LocalFree(pVerifyBuf);
    return TRUE;

EXIT:
    if(buf != NULL)
        LocalFree(buf);
    if(pVerifyBuf != NULL)
        LocalFree(pVerifyBuf);
    return FALSE;
}


// First wirte BCB before write image(eb/nk)
BOOL WriteMBRModeBCB(PMASTER_BOOT_RECORD pMBR,BOOT_CONFIGBLOCK *pBCB)
{
    DWORD dwBcbStarAddr;
    char VerifyBuf[512] = {0};
    BOOT_CONFIGBLOCK *pVerifyBCB;
    pVerifyBCB = (BOOT_CONFIGBLOCK *)VerifyBuf;
    memset(pVerifyBCB, 0x0, 512);

    //Read MBR
    if(!SDMMC_ReadSector(0, (LPBYTE)pMBR, 1))
    {
        RETAILMSG(1, (_T("ERROR: Failed to Read MBR from SDHC\r\n")));
        return FALSE;
    }
    
    //Partition0-FAT,Partition1-Eboot,Partition2-NK
    dwBcbStarAddr = pMBR->Partition[1].RelativeSector;
    pBCB->u32Signature = FIRMWARE_CONFIG_BLOCK_SIGNATURE;
    pBCB->u32PrimaryBootTag = PRIMARY_TAG;
    pBCB->u32SecondaryBootTag = SECONDARY_TAG;
    pBCB->u32NumCopies = 2;

    //boot Partition will be divided into two parts, if the first part fail will bring up from second part
    pBCB->aDriveInfo[0].u32Tag = PRIMARY_TAG;
    pBCB->aDriveInfo[0].u32FirstSectorNumber = (pMBR->Partition[1].RelativeSector + 4 + 3) & ~3;//bootoffset
    pBCB->aDriveInfo[0].u32SectorCount = (pMBR->Partition[1].TotalSector - \
                      (pBCB->aDriveInfo[0].u32FirstSectorNumber - pMBR->Partition[1].RelativeSector))/2 ;//bootsize

    pBCB->aDriveInfo[1].u32Tag = SECONDARY_TAG;
    pBCB->aDriveInfo[1].u32FirstSectorNumber = pBCB->aDriveInfo[0].u32FirstSectorNumber + pBCB->aDriveInfo[0].u32SectorCount;
    pBCB->aDriveInfo[1].u32SectorCount = pBCB->aDriveInfo[0].u32SectorCount;

    if(!SDMMC_WriteSector(dwBcbStarAddr, (LPBYTE)pBCB, 1))
    {
      RETAILMSG(1, (_T("ERROR: Failed to Write BCB to SDHC\r\n")));
      return FALSE;
    }
    DUMP_MBRModeBCB(pBCB);

    //Verify BCB
    memset(pVerifyBCB, 0x0, 512);
    //Read BCB 
    if(!SDMMC_ReadSector(dwBcbStarAddr, (LPBYTE)pVerifyBCB, 1))
    {
      RETAILMSG(1, (_T("ERROR: Failed to Read BCB from SDHC\r\n")));
      return FALSE;
    }
    DUMP_MBRModeBCB(pVerifyBCB);

    if(memcmp(pBCB,pVerifyBCB,SECTOR_SIZE) != 0)
    {
      RETAILMSG(1,(_T("ERROR: Fail to verify BCB!\r\n")));
      return FALSE;
    }

    //Read BCB 
    if(!SDMMC_ReadSector(pMBR->Partition[1].RelativeSector, (LPBYTE)pBCB, 1))
    {
        RETAILMSG(1, (_T("ERROR: Failed to Read BCB from SDHC\r\n")));
        return FALSE;
    }

    return TRUE;
}


//write image for trivial file not need partial write
BOOL SDWriteMBRModeImage(LPBYTE pImage, DWORD startSectorAddr, DWORD dwLength)
{
    LPBYTE pSectorBuf = NULL, pVerifyBuf = NULL; 
    DWORD sectorAddr, endSectorAddr, dwNumSectors, dwTotalSectors;
    DWORD percentComplete, lastPercentComplete; 
    
    // Write NK to SDHC flash
    pSectorBuf = pImage;
    pVerifyBuf = (LPBYTE) LocalAlloc(LPTR, SDHC_RW_NUM*SECTOR_SIZE); 

    dwTotalSectors = (dwLength + SECTOR_SIZE - 1)/SECTOR_SIZE;
    endSectorAddr = startSectorAddr + dwTotalSectors;  

    RETAILMSG(1, (_T("INFO: Totally %d sectors needs to be written.\r\n"), dwTotalSectors));
    
    lastPercentComplete = 0;
    RETAILMSG(1, (_T("\rINFO: Program is %d%% complete\r"), lastPercentComplete));
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
        DEBUGMSG(1,(_T("sectorAddr is : 0x%x. endSectorAddr is : 0x%x\r\n"),sectorAddr,endSectorAddr));
              
        if(!SDMMC_WriteSector(sectorAddr, pSectorBuf,dwNumSectors))
        {
            RETAILMSG(1, (_T("ERROR: Failed to write sector\r\n")));
            goto EXIT;
        }

        //Verify data
        if(!SDMMC_ReadSector(sectorAddr, pVerifyBuf, dwNumSectors))
        {
            RETAILMSG(1, (_T("ERROR: Failed to read sector\r\n")));
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
            RETAILMSG(1, (_T("\rINFO: Program is %d%% complete.\r"), percentComplete));
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
    return FALSE;
        
}

// Write eboot
BOOL SDWriteMBRModeEboot(LPBYTE pImage, DWORD dwLength)
{
    char *buf = NULL;
    char *bufBCB = NULL;
    
    buf = (char*) LocalAlloc(LPTR, 512);
    if(buf == NULL)
        goto EXIT;
    ZeroMemory(buf,512);
    PMASTER_BOOT_RECORD pMBR = (PMASTER_BOOT_RECORD)buf;
    bufBCB = (char*) LocalAlloc(LPTR, 512);
    if(bufBCB == NULL)
        goto EXIT;
    ZeroMemory(bufBCB,512);
    BOOT_CONFIGBLOCK *pBCB = (BOOT_CONFIGBLOCK *)bufBCB;
    DWORD startSectorAddr;
    INT iWriteCount = 0;

    RETAILMSG(1, (_T("INFO: Writing EBOOT image to SD/MMC media, please wait...\r\n")));

    //Write BCB
    if(!WriteMBRModeBCB(pMBR,pBCB))
    {
        RETAILMSG(1, (_T("ERROR: Failed to write BCB.\r\n")));
        goto EXIT;
    }

retry:    
    
    if(iWriteCount == 0)
    {
        startSectorAddr = pBCB->aDriveInfo[0].u32FirstSectorNumber;
    }
    else
    {
        startSectorAddr = pBCB->aDriveInfo[1].u32FirstSectorNumber;;
    }
    DEBUGMSG(1, (_T("Eboot startSectorAddr is 0x%x.\r\n"),startSectorAddr));

    if(!SDWriteMBRModeImage(pImage,startSectorAddr,dwLength))
    {
        RETAILMSG(1, (_T("ERROR: Failed to write EBOOT.\r\n")));
        goto EXIT;
    }

    if(iWriteCount == 0)
    {
        iWriteCount++;
        goto retry;   
    }

    LocalFree(buf);
    LocalFree(bufBCB);
    return TRUE;

EXIT:
    RETAILMSG(TRUE, (_T("\r\nINFO: Updating image failed.\r\n"))); 
    if( buf != NULL)
        LocalFree(buf);
    if(bufBCB != NULL)
        LocalFree(bufBCB);
    return FALSE;
}

// Write nk
BOOL SDWriteMBRModeNK(LPBYTE pImage, DWORD dwLength)
{
    char *buf = NULL;
    char *bufBCB = NULL;
    DWORD startSectorAddr;
    
    buf = (char*) LocalAlloc(LPTR, 512);
    if(buf == NULL)
        goto EXIT;
    ZeroMemory(buf,512);
    PMASTER_BOOT_RECORD pMBR = (PMASTER_BOOT_RECORD)buf;
    bufBCB = (char*) LocalAlloc(LPTR, 512);
    if(bufBCB == NULL)
        goto EXIT;
    ZeroMemory(bufBCB,512);
    BOOT_CONFIGBLOCK *pBCB = (BOOT_CONFIGBLOCK *)bufBCB;

    RETAILMSG(1, (_T("INFO: Writing NK image to SD/MMC media, please wait...\r\n")));

    //Write BCB
    if(!WriteMBRModeBCB(pMBR,pBCB))
    {
        RETAILMSG(1, (_T("ERROR: Failed to BCB.\r\n")));
        goto EXIT;
    } 

    startSectorAddr = pMBR->Partition[2].RelativeSector;

    if(!SDWriteMBRModeImage(pImage,startSectorAddr,dwLength))
    {
        RETAILMSG(1, (_T("ERROR: Failed to write EBOOT.\r\n")));
        goto EXIT;
    }

    LocalFree(buf);
    LocalFree(bufBCB);
    return TRUE;

EXIT:
    RETAILMSG(TRUE, (_T("\r\nINFO: Updating image failed.\r\n")));
    if(buf != NULL)
        LocalFree(buf);
    if(bufBCB != NULL)
        LocalFree(bufBCB);
    return FALSE;
}


// Prepare for write NK
BOOL SDPrePartialWriteMBRModeImage(fwType type, DWORD dwLength)
{
    char *buf = NULL;
    char *bufBCB = NULL;
    DWORD dwTotalSectors;
    
    buf = (char*) LocalAlloc(LPTR, 512);
    if(buf == NULL)
        goto EXIT;
    ZeroMemory(buf,512);
    PMASTER_BOOT_RECORD pMBR = (PMASTER_BOOT_RECORD)buf;
    bufBCB = (char*) LocalAlloc(LPTR, 512);
    if(bufBCB == NULL)
        goto EXIT;
    ZeroMemory(bufBCB,512);
    BOOT_CONFIGBLOCK *pBCB = (BOOT_CONFIGBLOCK *)bufBCB;

    if(!WriteMBRModeBCB(pMBR,pBCB))
    {
        RETAILMSG(1, (_T("ERROR: Failed to write MBR and BCB.\r\n")));
        return FALSE;
    }

    switch(type)
    {
        case fwType_NK_NB:
            gStartSectorAddr = pMBR->Partition[2].RelativeSector;
            break;
        case fwType_NK_SB:
            gStartSectorAddr = pBCB->aDriveInfo[0].u32FirstSectorNumber;
            g2ndStartSectorAddr = pBCB->aDriveInfo[1].u32FirstSectorNumber;
            DEBUGMSG(1, (_T("SDPrePartialWriteMBRModeImage: g2ndStartSectorAddr is 0x%x.\r\n"), g2ndStartSectorAddr));
            break;
        default:
            RETAILMSG(1, (_T("INFO: Firmware type is wrong.\r\n")));
            return FALSE;
    }
    DEBUGMSG(1, (_T("SDPrePartialWriteMBRModeImage: gStartSectorAddr is 0x%x.\r\n"), gStartSectorAddr));

    dwTotalSectors = (dwLength+SECTOR_SIZE-1)/SECTOR_SIZE;
    RETAILMSG(1, (_T("INFO: Totally %d sectors needs to be written.\r\n"), dwTotalSectors));
    
    LocalFree(buf);
    LocalFree(bufBCB);
    return TRUE;  

EXIT:
    RETAILMSG(TRUE, (_T("\r\nINFO: Updating image failed.\r\n")));
    if(buf != NULL)
        LocalFree(buf);
    if(bufBCB != NULL)
        LocalFree(bufBCB);
    return FALSE;   
}

//write nk
BOOL SDPartialWriteImage(LPBYTE pImage, DWORD dwLength)
{
    DWORD sectorAddr, endSectorAddr, dwNumSectors, dwTotalSectors;    
    LPBYTE pSectorBuf, pVerifyBuf;    
    
    dwTotalSectors = (dwLength+SECTOR_SIZE-1)/SECTOR_SIZE;
    endSectorAddr = gStartSectorAddr + dwTotalSectors;  
    DEBUGMSG(1,(_T("SDPartialWriteImage:gStartSectorAddr is 0x%x\r\n"),gStartSectorAddr));
    DEBUGMSG(1, (_T("SDPartialWriteImage: Totally %d sectors needs to be written.\r\n"), dwTotalSectors));
    
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

//write nk.sb
BOOL SDPartialWriteRedundantImage(LPBYTE pImage, DWORD dwLength)
{
    DWORD sectorAddr;

    if(!SDPartialWriteImage(pImage, dwLength))
        return FALSE;
    sectorAddr = gStartSectorAddr;
    gStartSectorAddr = g2ndStartSectorAddr;
    if(!SDPartialWriteImage(pImage, dwLength))
        return FALSE;
    g2ndStartSectorAddr = gStartSectorAddr;
    gStartSectorAddr = sectorAddr;

    return TRUE;
}

BOOL SDEndPartialWriteImage()
{
    RETAILMSG(TRUE, (_T("INFO: Verifying image succeeds.\r\n")));    
    RETAILMSG(TRUE, (_T("INFO: Updating of image completed successfully.\r\n")));
    return TRUE;      
}
