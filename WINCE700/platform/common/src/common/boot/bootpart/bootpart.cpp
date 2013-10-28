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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
#include <bootpart.h>
#include "bppriv.h"


LPBYTE g_pbMBRSector = NULL;
LPBYTE g_pbBlock = NULL;
DWORD g_dwMBRSectorNum = INVALID_ADDR;
FlashInfo g_FlashInfo;
PARTSTATE g_partStateTable[NUM_PARTS];
PSectorInfo g_pSectorInfoBuf;
DWORD g_dwLastLogSector;          // Stores the last valid logical sector
DWORD g_dwDataBytesPerBlock;
DWORD g_dwLastWrittenLoc;  // Stores the byte address of the last physical flash address written to

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static Addr LBAtoCHS(FlashInfo *pFlashInfo, Addr lba)
{
    if(lba.type == CHS)
        return lba;

    Addr chs;
    DWORD tmp = pFlashInfo->dwNumBlocks * pFlashInfo->wSectorsPerBlock;

    chs.type = CHS;
    chs.chs.cylinder = (WORD)(lba.lba / tmp);
    tmp = lba.lba % tmp;
    chs.chs.head = (WORD)(tmp / pFlashInfo->wSectorsPerBlock);
    chs.chs.sector = (WORD)((tmp % pFlashInfo->wSectorsPerBlock) + 1);

    return chs;
}

/* 
CHStoLBA generates C4505 warning: "unreferenced local function has been removed". 
Keep it as comments for possible future reference.
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static Addr CHStoLBA(FlashInfo *pFlashInfo, Addr chs)
{
    Addr lba;

    if(chs.type == LBA)
        return chs;

    lba.type = LBA;
    lba.lba = ((chs.chs.cylinder * pFlashInfo->dwNumBlocks + chs.chs.head)
        * pFlashInfo->wSectorsPerBlock)+ chs.chs.sector - 1;

    return lba;
}
*/

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static DWORD GetMBRSectorNum ()
{
    DWORD dwBlockNum = 0;

    while (dwBlockNum < g_FlashInfo.dwNumBlocks) {

        if (!IS_BLOCK_UNUSABLE (dwBlockNum)) {
            return (dwBlockNum * g_FlashInfo.wSectorsPerBlock);
        }

        dwBlockNum++;
    }

    return INVALID_ADDR;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static BOOL WriteMBR()
{
    DWORD dwMBRBlockNum = g_dwMBRSectorNum / g_FlashInfo.wSectorsPerBlock;

    RETAILMSG(1, (TEXT("WriteMBR: MBR block = 0x%x.\r\n"), dwMBRBlockNum));

    memset (g_pbBlock, 0xff, g_dwDataBytesPerBlock);
    memset (g_pSectorInfoBuf, 0xff, sizeof(SectorInfo) * g_FlashInfo.wSectorsPerBlock);

    // No need to check return, since a failed read means data hasn't been written yet.
    ReadBlock (dwMBRBlockNum, g_pbBlock, g_pSectorInfoBuf);

    if (!FMD_EraseBlock (dwMBRBlockNum)) {
        RETAILMSG (1, (TEXT("CreatePartition: error erasing block 0x%x\r\n"), dwMBRBlockNum));
        return FALSE;
    }

    memcpy (g_pbBlock + (g_dwMBRSectorNum % g_FlashInfo.wSectorsPerBlock) * g_FlashInfo.wDataBytesPerSector, g_pbMBRSector, g_FlashInfo.wDataBytesPerSector);
    g_pSectorInfoBuf->bOEMReserved &= ~OEM_BLOCK_READONLY;
    g_pSectorInfoBuf->wReserved2 &= ~SECTOR_WRITE_COMPLETED;
    g_pSectorInfoBuf->dwReserved1 = 0;

    if (!WriteBlock (dwMBRBlockNum, g_pbBlock, g_pSectorInfoBuf)) {
        RETAILMSG (1, (TEXT("CreatePartition: could not write to block 0x%x\r\n"), dwMBRBlockNum));
        return FALSE;
    }

    return TRUE;

}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static BOOL CreateMBR()
{
    // This, plus a valid partition table, is all the CE partition manager needs to recognize
    // the MBR as valid. It does not contain boot code.

    memset (g_pbMBRSector, 0xff, g_FlashInfo.wDataBytesPerSector);
    g_pbMBRSector[0] = 0xE9;
    g_pbMBRSector[1] = 0xfd;
    g_pbMBRSector[2] = 0xff;
    g_pbMBRSector[SECTOR_SIZE-2] = 0x55;
    g_pbMBRSector[SECTOR_SIZE-1] = 0xAA;

    // Zero out partition table so that mspart treats entries as empty.
    memset (g_pbMBRSector+PARTTABLE_OFFSET, 0, sizeof(PARTENTRY) * NUM_PARTS);

    return WriteMBR();

}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static BOOL IsValidMBR()
{
    BLOCK_ID  curBlock;

    for (curBlock = 0; curBlock < g_FlashInfo.dwNumBlocks; curBlock ++)
    {
        if (IS_BLOCK_UNUSABLE(curBlock)) {
            // skip bad or reserved block
            continue;
        }

        // calculate the first sector of the block
        g_dwMBRSectorNum = curBlock * g_FlashInfo.wSectorsPerBlock;

        if (!FMD_ReadSector (g_dwMBRSectorNum, g_pbMBRSector, NULL, 1)) {
            // failed to read the sector, something is wrong
            return FALSE;
        }

        // check for MBR signature
        if ((g_pbMBRSector[0] == 0xE9) &&
            (g_pbMBRSector[1] == 0xfd) &&
            (g_pbMBRSector[2] == 0xff) &&
            (g_pbMBRSector[SECTOR_SIZE-2] == 0x55) &&
            (g_pbMBRSector[SECTOR_SIZE-1] == 0xAA)) {

            // found a valid MBR
            RETAILMSG (1, (TEXT("IsValidMBR: MBR sector = 0x%x (valid MBR)\r\n"), g_dwMBRSectorNum));
            return TRUE;
        }
    }

    // found no MBR, so MBR sector should be the first sector of the first usable block
    g_dwMBRSectorNum = GetMBRSectorNum();
    RETAILMSG (1, (TEXT("IsValidMBR: MBR sector = 0x%x (invalid MBR)\r\n"), g_dwMBRSectorNum));

    return FALSE;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static BOOL IsValidPart (PPARTENTRY pPartEntry)
{
    return (pPartEntry->Part_FileSystem != 0xff) && (pPartEntry->Part_FileSystem != 0);
}


/*  AddPartitionTableEntry
 *
 *  Generates the partition entry for the partition table and copies the entry
 *  into the MBR that is stored in memory.
 *
 *
 *  ENTRY
 *      entry - index into partition table
 *      startSector - starting logical sector
 *      totalSectors - total logical sectors
 *      fileSystem - type of partition
 *      bootInd - byte in partition entry that stores various flags such as
 *          active and read-only status.
 *
 *  EXIT
 */

static void AddPartitionTableEntry(DWORD entry, DWORD startSector, DWORD totalSectors, BYTE fileSystem, BYTE bootInd)
{
    PARTENTRY partentry = {0};
    Addr startAddr;
    Addr endAddr;

    ASSERT(entry < 4);

    // no checking with disk info and start/total sectors because we allow
    // bogus partitions for testing purposes

    // initially known partition table entry
    partentry.Part_BootInd = bootInd;
    partentry.Part_FileSystem = fileSystem;
    partentry.Part_StartSector = startSector;
    partentry.Part_TotalSectors = totalSectors;

    // logical block addresses for the first and final sector (start on the second head)
    startAddr.type = LBA;
    startAddr.lba = partentry.Part_StartSector;
    endAddr.type = LBA;
    endAddr.lba = partentry.Part_StartSector + partentry.Part_TotalSectors-1;

    // translate the LBA addresses to CHS addresses
    startAddr = LBAtoCHS(&g_FlashInfo, startAddr);
    endAddr = LBAtoCHS(&g_FlashInfo, endAddr);

    // starting address
    partentry.Part_FirstTrack = (BYTE)(startAddr.chs.cylinder & 0xFF);
    partentry.Part_FirstHead = (BYTE)(startAddr.chs.head & 0xFF);
    // lower 6-bits == sector, upper 2-bits = cylinder upper 2-bits of 10-bit cylinder #
    partentry.Part_FirstSector = (BYTE)((startAddr.chs.sector & 0x3F) | ((startAddr.chs.cylinder & 0x0300) >> 2));

    // ending address:
    partentry.Part_LastTrack = (BYTE)(endAddr.chs.cylinder & 0xFF);
    partentry.Part_LastHead = (BYTE)(endAddr.chs.head & 0xFF);
    // lower 6-bits == sector, upper 2-bits = cylinder upper 2-bits of 10-bit cylinder #
    partentry.Part_LastSector = (BYTE)((endAddr.chs.sector & 0x3F) | ((endAddr.chs.cylinder & 0x0300) >> 2));

    memcpy(g_pbMBRSector+PARTTABLE_OFFSET+(sizeof(PARTENTRY)*entry), &partentry, sizeof(PARTENTRY));
}


/*  GetPartitionTableIndex
 *
 *  Get the partition index for a particular partition type and active status.
 *  If partition is not found, then the index of the next free partition in the
 *  partition table of the MBR is returned.
 *
 *
 *  ENTRY
 *      dwPartType - type of partition
 *      fActive - TRUE indicates the active partition.  FALSE indicates inactive.
 *      dwIndex - used to disambiguate which instance of dwPartType requested
 *
 *  EXIT
 *      pdwIndex - Contains the index of the partition if found.  If not found,
 *          contains the index of the next free partition
 *      returns TRUE if partition found. FALSE if not found.
 */

static BOOL GetPartitionTableIndex (DWORD dwPartType, BOOL fActive, DWORD dwIndex, PDWORD pdwIndex)
{
    PPARTENTRY pPartEntry = (PPARTENTRY)(g_pbMBRSector + PARTTABLE_OFFSET);
    DWORD iEntry = 0;

    for (iEntry = 0; iEntry < NUM_PARTS; iEntry++, pPartEntry++) {
        if ((pPartEntry->Part_FileSystem == dwPartType) && (dwIndex > 0)) {
            // found a partition of the correct type, but not the one we're looking for
            dwIndex--;
            continue;
        }

        if ((pPartEntry->Part_FileSystem == dwPartType) && (((pPartEntry->Part_BootInd & PART_IND_ACTIVE) != 0) == fActive)) {
            *pdwIndex = iEntry;
            return TRUE;
        }

        if (!IsValidPart (pPartEntry) && (dwIndex == 0)) {
            // only set pdwIndex if we've already skipped past all requested partitions 
            // of type dwPartType - otherwise fall through and fail below
            *pdwIndex = iEntry;
            return FALSE;
        }

        if (!IsValidPart (pPartEntry))
        {
            break;
        }
    }

    // partition not found and either dwIndex > 0 or we hit the max number of partitions
    *pdwIndex = NUM_PARTS;
    return FALSE;
}

/* WriteLogicalNumbers
 *
 *  Writes a range of logical sector numbers
 *
 *  ENTRY
 *      dwStartSector - starting logical sector
 *      dwNumSectors - number of logical sectors to mark
 *      fReadOnly - TRUE indicates to mark read-only.  FALSE to mark not read-only
 *
 *  EXIT
 *      TRUE on success
 */


static BOOL WriteLogicalNumbers (DWORD dwStartSector, DWORD dwNumSectors, BOOL fReadOnly)
{
    DWORD dwNumSectorsWritten = 0;

    DWORD dwPhysSector = Log2Phys (dwStartSector);
    DWORD dwBlockNum = dwPhysSector / g_FlashInfo.wSectorsPerBlock;
    DWORD dwOffset = dwPhysSector % g_FlashInfo.wSectorsPerBlock;

    while (dwNumSectorsWritten < dwNumSectors) {

        // If bad block, move to the next block
        if (IS_BLOCK_UNUSABLE (dwBlockNum)) {
            dwBlockNum++;
            continue;
        }

        memset (g_pbBlock, 0xff, g_dwDataBytesPerBlock);
        memset (g_pSectorInfoBuf, 0xff, sizeof(SectorInfo) * g_FlashInfo.wSectorsPerBlock);
        // No need to check return, since a failed read means data hasn't been written yet.
        ReadBlock (dwBlockNum, g_pbBlock, g_pSectorInfoBuf);
        if (!FMD_EraseBlock (dwBlockNum)) {
            return FALSE;
        }

        DWORD dwSectorsToWrite = g_FlashInfo.wSectorsPerBlock - dwOffset;
        PSectorInfo pSectorInfo = g_pSectorInfoBuf + dwOffset;

        // If this is the last block, then calculate sectors to write if there isn't a full block to update
        if ((dwSectorsToWrite + dwNumSectorsWritten) > dwNumSectors)
            dwSectorsToWrite = dwNumSectors - dwNumSectorsWritten;

        for (DWORD iSector = 0; iSector < dwSectorsToWrite; iSector++, pSectorInfo++, dwNumSectorsWritten++) {
            // Assert read only by setting bit to 0 to prevent wear-leveling by FAL
            if (fReadOnly)
                pSectorInfo->bOEMReserved &= ~OEM_BLOCK_READONLY;
            // Set to write completed so FAL can map the sector
            pSectorInfo->wReserved2 &= ~SECTOR_WRITE_COMPLETED;
            // Write the logical sector number
            pSectorInfo->dwReserved1 = dwStartSector + dwNumSectorsWritten;
        }

        if (!WriteBlock (dwBlockNum, g_pbBlock, g_pSectorInfoBuf))
            return FALSE;

        dwOffset = 0;
        dwBlockNum++;
    }
    return TRUE;
}



/*  LastLogSector
 *
 *  Returns the last logical sector on the flash
 *
 *  ENTRY
 *
 *  EXIT
 *      Returns the last logical sector on the flash
 */

static DWORD LastLogSector()
{
    if (g_dwLastLogSector) {
       return g_dwLastLogSector;
    }

    DWORD dwMBRBlock = g_dwMBRSectorNum / g_FlashInfo.wSectorsPerBlock;
    DWORD dwUnusableBlocks = dwMBRBlock;

    for (DWORD i = dwMBRBlock; i < g_FlashInfo.dwNumBlocks; i++) {
        if (IS_BLOCK_UNUSABLE (i))
            dwUnusableBlocks++;
    }

    g_dwLastLogSector = (g_FlashInfo.dwNumBlocks - dwUnusableBlocks) * g_FlashInfo.wSectorsPerBlock - 1;

    RETAILMSG(1, (TEXT("LastLogSector: Last log sector is: 0x%x.\r\n"), g_dwLastLogSector));

    return g_dwLastLogSector;
}

/*  Log2Phys
 *
 *  Converts a logical sector into a physical sector
 *
 *  ENTRY
 *      dwLogSector - logical sector
 *
 *  EXIT
 *      Returns the physical sector.  INVALID_ADDR if logical sector does not
 *      exist.
 */

static DWORD Log2Phys (DWORD dwLogSector)
{
    // Determine logical block number
    DWORD dwLogBlock = dwLogSector / g_FlashInfo.wSectorsPerBlock;

    // Start searching at the MBR block
    if (g_dwMBRSectorNum == INVALID_ADDR) {
        RETAILMSG(1, (TEXT("Log2Phys: MBR sector number is invalid.\r\n")));
        return INVALID_ADDR;
    }
    DWORD dwPhysBlock = g_dwMBRSectorNum / g_FlashInfo.wSectorsPerBlock;

    if (dwLogBlock >= g_FlashInfo.dwNumBlocks)
        return INVALID_ADDR;

    // The physical block will be the number of logical blocks plus the number of bad blocks
    // starting from the MBR block.
    while (dwLogBlock--) {
        dwPhysBlock++;
        while (IS_BLOCK_UNUSABLE (dwPhysBlock) && dwPhysBlock < g_FlashInfo.dwNumBlocks) {
            dwPhysBlock++;
        }
        if (dwPhysBlock >= g_FlashInfo.dwNumBlocks)
            return INVALID_ADDR;
    }

    RETAILMSG(FALSE, (TEXT("Log2Phys: Logical 0x%x -> Physical 0x%x\r\n"), dwLogSector, dwPhysBlock * g_FlashInfo.wSectorsPerBlock + (dwLogSector % g_FlashInfo.wSectorsPerBlock)));

    return dwPhysBlock * g_FlashInfo.wSectorsPerBlock + (dwLogSector % g_FlashInfo.wSectorsPerBlock);
}


/*  FindFreeSectors
 *
 *  Finds the last free logical sector.
 *
 *  ENTRY
 *
 *  EXIT
 *      Last free logical sector.  INVALID_ADDR if none are free.
 */

static DWORD FindFreeSector()
{
    DWORD dwFreeSector = 1;
    PPARTENTRY pPartEntry = (PPARTENTRY)(g_pbMBRSector + PARTTABLE_OFFSET);

    for (int i = 0; i < NUM_PARTS; i++) {
        if (!IsValidPart (pPartEntry))
            break;
        if ((pPartEntry->Part_StartSector + pPartEntry->Part_TotalSectors) > dwFreeSector)
            dwFreeSector = pPartEntry->Part_StartSector + pPartEntry->Part_TotalSectors;
        RETAILMSG(1, (TEXT("FindFreeSector: FreeSector is: 0x%x after processing part 0x%x.\r\n"), dwFreeSector, pPartEntry->Part_FileSystem));
        pPartEntry++;
    }

    DWORD dwLastLogSector = LastLogSector();
    if (dwLastLogSector == INVALID_ADDR || dwFreeSector >= dwLastLogSector)
        dwFreeSector = INVALID_ADDR;

    return dwFreeSector;
}

/*  AreSectorsFree
 *
 *  Determines if sectors are available for a new partition
 *
 *  ENTRY
 *      dwStartSector - Logical sector to start the partition.
 *      dwNumSectors - Number of logical sectors of the partition.
 *
 *  EXIT
 *      TRUE if sectors are free.  FALSE otherwise.
 */

static BOOL AreSectorsFree (DWORD dwStartSector, DWORD dwNumSectors)
{
    PPARTENTRY pPartEntry = (PPARTENTRY)(g_pbMBRSector + PARTTABLE_OFFSET);
    DWORD dwEndSector = dwStartSector + dwNumSectors - 1;
    DWORD dwEndReservedSector;

    DWORD dwLastLogSector = LastLogSector();
    if ((dwLastLogSector == INVALID_ADDR) || (dwNumSectors > dwLastLogSector+1) || (dwEndSector > dwLastLogSector) ||
        (dwNumSectors == 0) || (dwStartSector > dwEndSector))
        return FALSE;

    for (int i = 0; i < NUM_PARTS; i++) {
        if (!IsValidPart (pPartEntry))
            break;
        dwEndReservedSector = pPartEntry->Part_StartSector + pPartEntry->Part_TotalSectors - 1;

        if (!(dwStartSector > dwEndReservedSector || dwEndSector < pPartEntry->Part_StartSector))
            return FALSE;

        pPartEntry++;
    }
    return TRUE;
}

/*  CreatePartition
 *
 *  Creates a new partition.  If it is a boot section partition, then it formats
 *  flash.
 *
 *  ENTRY
 *      dwStartSector - Logical sector to start the partition.  NEXT_FREE_LOC if
 *          none specified.
 *      dwNumSectors - Number of logical sectors of the partition.  USE_REMAINING_SPACE
 *          to indicate to take up the rest of the space on the flash for that partition.
 *      dwPartType - Type of partition to create.
 *      fActive - TRUE indicates to create the active partition.  FALSE for
 *          inactive.
 *      dwPartIndex - Index of the partition entry on the MBR
 *
 *  EXIT
 *      Handle to the partition on success.  INVALID_HANDLE_VALUE on error.
 */

static HANDLE CreatePartition (DWORD dwStartSector, DWORD dwNumSectors, DWORD dwPartType, BOOL fActive, DWORD dwPartIndex)
{
    DWORD dwBootInd = 0;

    RETAILMSG(1, (TEXT("CreatePartition: Enter CreatePartition for 0x%x.\r\n"), dwPartType));

    if (fActive)
        dwBootInd |= PART_IND_ACTIVE;
    if (dwPartType == PART_BOOTSECTION || dwPartType == PART_BINFS || dwPartType == PART_XIP)
        dwBootInd |= PART_IND_READ_ONLY;

     // If start sector is invalid, it means find next free sector
    if (dwStartSector == NEXT_FREE_LOC) {

        dwStartSector = FindFreeSector();
        if (dwStartSector == INVALID_ADDR) {
            RETAILMSG(1, (TEXT("CreatePartition: can't find free sector.\r\n")));
            return INVALID_HANDLE_VALUE;
        }

        // Start partitions on the next block if they are currently on the wrong block type.
        if (dwStartSector % g_FlashInfo.wSectorsPerBlock) {
            DWORD dwBlock = dwStartSector / g_FlashInfo.wSectorsPerBlock;
            if (IS_PART_READONLY(dwBootInd) != IS_BLOCK_READONLY(dwBlock)) {
                dwStartSector = (dwBlock+1) * g_FlashInfo.wSectorsPerBlock;
            }
        }
    }

    if (IS_PART_READONLY(dwBootInd)) {

        // Allow read-only partitions to go to the end of disk, if requested.
        if (dwNumSectors == USE_REMAINING_SPACE) {

            DWORD dwLastLogSector = LastLogSector();
            if (dwLastLogSector == INVALID_ADDR)
                return INVALID_HANDLE_VALUE;

            dwNumSectors = dwLastLogSector - dwStartSector + 1;
        }
    }
    else {

        DWORD dwLastLogSector = LastLogSector();
        if (dwLastLogSector == INVALID_ADDR)
            return INVALID_HANDLE_VALUE;

        // Determine the number of blocks to reserve for the FAL compaction when creating an extended partition.
        DWORD dwReservedBlocks = g_FlashInfo.dwNumBlocks / PERCENTAGE_OF_MEDIA_TO_RESERVE;
        if((dwReservedBlocks = g_FlashInfo.dwNumBlocks / PERCENTAGE_OF_MEDIA_TO_RESERVE) < MINIMUM_FLASH_BLOCKS_TO_RESERVE) {
            dwReservedBlocks = MINIMUM_FLASH_BLOCKS_TO_RESERVE;
        }

        DWORD dwNumMaxSectors = dwLastLogSector - dwStartSector + 1 - dwReservedBlocks * g_FlashInfo.wSectorsPerBlock;

        // If dwNumSectors was provided, validate it isn't past the max.
        // If dwNumSectors is USE_REMAINING_SPACE, fill disk with max sectors.
        if ((dwNumSectors == USE_REMAINING_SPACE)  || (dwNumMaxSectors <  dwNumSectors)) {
            RETAILMSG(1, (TEXT("CreatePartition: Num sectors set to 0x%x to allow for compaction blocks.\r\n"), dwNumMaxSectors));
            dwNumSectors = dwNumMaxSectors ;
        }
    }


    if (!AreSectorsFree (dwStartSector, dwNumSectors)){
        RETAILMSG (1, (TEXT("CreatePartition: sectors [0x%x, 0x%x] requested are out of range or taken by another partition\r\n"), dwStartSector, dwNumSectors));
        return INVALID_HANDLE_VALUE;
    }

    RETAILMSG(1, (TEXT("CreatePartition: Start = 0x%x, Num = 0x%x.\r\n"), dwStartSector, dwNumSectors));

    AddPartitionTableEntry (dwPartIndex, dwStartSector, dwNumSectors, (BYTE)dwPartType, (BYTE)dwBootInd);

    if (IS_PART_READONLY(dwBootInd)) {
        if (!WriteLogicalNumbers (dwStartSector, dwNumSectors, TRUE)) {
            RETAILMSG(1, (TEXT("CreatePartition: can't mark sector info.\r\n")));
            return INVALID_HANDLE_VALUE;
        }
    }

    if (!WriteMBR())
        return INVALID_HANDLE_VALUE;

    g_partStateTable[dwPartIndex].pPartEntry = (PPARTENTRY)(g_pbMBRSector + PARTTABLE_OFFSET + sizeof(PARTENTRY)*dwPartIndex);
    g_partStateTable[dwPartIndex].dwDataPointer = 0;

    return (HANDLE)&g_partStateTable[dwPartIndex];
}


/*  ReadBlock
 *
 *  Reads one block of data
 *
 *  ENTRY
 *      dwBlock - Physical block number to read from
 *      pbBlock - Buffer to read into
 *      pSectorInfoTable - Pointer to table of sector infos
 *
 *  EXIT
 *      TRUE on success
 */

static BOOL ReadBlock (DWORD dwBlock, LPBYTE pbBlock, PSectorInfo pSectorInfoTable)
{
    for (int iSector = 0; iSector < g_FlashInfo.wSectorsPerBlock; iSector++) {
        if (!FMD_ReadSector(dwBlock * g_FlashInfo.wSectorsPerBlock + iSector, pbBlock, pSectorInfoTable, 1))
            return FALSE;
        if (pbBlock)
            pbBlock += g_FlashInfo.wDataBytesPerSector;
        if (pSectorInfoTable)
            pSectorInfoTable++;
    }
    return TRUE;
}

/*  WriteBlock
 *
 *  Writes one block of data
 *
 *  ENTRY
 *      dwBlock - Physical block number to write to
 *      pbBlock - Pointer to data to write
 *      pSectorInfoTable - Pointer to table of sector infos to write
 *
 *  EXIT
 *      TRUE on success
 */

static BOOL WriteBlock (DWORD dwBlock, LPBYTE pbBlock, PSectorInfo pSectorInfoTable)
{
    for (int iSector = 0; iSector < g_FlashInfo.wSectorsPerBlock; iSector++) {
        if (!FMD_WriteSector(dwBlock * g_FlashInfo.wSectorsPerBlock + iSector, pbBlock, pSectorInfoTable, 1))
            return FALSE;
        if (pbBlock)
            pbBlock += g_FlashInfo.wDataBytesPerSector;
        if (pSectorInfoTable)
            pSectorInfoTable++;
    }
    return TRUE;
}

/*  BP_ReadData
 *
 *  Reads data from the partition starting at the data pointer.  Call fails
 *  if length of the buffer is too long (i.e. trying to read past the end
 *  of the partition)
 *
 *  ENTRY
 *      hPartition - handle to the partition
 *      pbBuffer - pointer to buffer of data to read
 *      dwLength - number of bytes to read
 *
 *  EXIT
 *      TRUE on success
 */

BOOL BP_ReadData(HANDLE hPartition, LPBYTE pbBuffer, DWORD dwLength)
{
    if (hPartition == INVALID_HANDLE_VALUE)
        return FALSE;

    DWORD dwNumSects, dwBlockAddress;
    static LPBYTE pbSector = g_pbBlock;
    PPARTSTATE pPartState = (PPARTSTATE) hPartition;
    DWORD dwNextPtrValue = pPartState->dwDataPointer + dwLength;

    if (!pbBuffer || !pbSector || dwLength == 0)
        return(FALSE);

    RETAILMSG (FALSE, (TEXT("ReadData: Start = 0x%x, Length = 0x%x.\r\n"), pPartState->dwDataPointer, dwLength));

    // Check to make sure buffer size is within limits of partition
    if (((dwNextPtrValue - 1) / g_FlashInfo.wDataBytesPerSector) >= pPartState->pPartEntry->Part_TotalSectors) {
        RETAILMSG (1, (TEXT("ReadData: trying to read past end of partition.\r\n")));
        return FALSE;
    }

    // Get the starting physical sector
    DWORD dwSectorAddr = Log2Phys (pPartState->dwDataPointer / g_FlashInfo.wDataBytesPerSector + pPartState->pPartEntry->Part_StartSector);

    // If current pointer is not on a sector boundary, copy bytes up to the first sector boundary
    DWORD dwOffsetSector = pPartState->dwDataPointer % g_FlashInfo.wDataBytesPerSector;
    if (dwOffsetSector)
    {
        if (!FMD_ReadSector(dwSectorAddr, pbSector, NULL, 1))
        {
            RETAILMSG (1, (TEXT("ReadData: failed to read sector (0x%x).\r\n"), dwSectorAddr));
            return(FALSE);
        }

        DWORD dwNumBytesRead = g_FlashInfo.wDataBytesPerSector - dwOffsetSector;
        if (dwNumBytesRead > dwLength)
            dwNumBytesRead = dwLength;

        memcpy(pbBuffer, pbSector + dwOffsetSector, dwNumBytesRead);
        dwLength -= dwNumBytesRead;
        pbBuffer += dwNumBytesRead;
        dwSectorAddr++;
    }

    // Compute sector length.
    dwNumSects = (dwLength / g_FlashInfo.wDataBytesPerSector);

    // NAND FMD only supports single-sector reads at the moment.
    while (dwNumSects--)
    {
        dwBlockAddress = (dwSectorAddr / g_FlashInfo.wSectorsPerBlock);

        // If the block is marked bad, skip to next block.  Note that the assumption in our error checking
        // is that any truely bad block will be marked either by the factory during production or will be marked
        // during the erase and write verification phases.  If anything other than a bad block fails ECC correction
        // in this routine, it's fatal.
        if (IS_BLOCK_UNUSABLE(dwBlockAddress))
        {
            dwSectorAddr += g_FlashInfo.wSectorsPerBlock;
            ++dwNumSects;        // Compensate for fact that we didn't write any sector data.
            continue;
        }

        // Read the sector - if this fails ECC correction, we fail the whole read operation.
        // Note - only single sector reads supported at the moment.
        if (!FMD_ReadSector(dwSectorAddr, pbBuffer, NULL, 1))
        {
            RETAILMSG (1, (TEXT("ReadData: failed to read sector (0x%x).\r\n"), dwSectorAddr));
            return(FALSE);
        }

        ++dwSectorAddr;
        pbBuffer += g_FlashInfo.wDataBytesPerSector;
    }

    DWORD dwNumExtraBytes = (dwLength % g_FlashInfo.wDataBytesPerSector);
    if (dwNumExtraBytes)
    {
        // Skip bad blocks
        while (IS_BLOCK_UNUSABLE(dwSectorAddr / g_FlashInfo.wSectorsPerBlock))
        {
            dwSectorAddr += g_FlashInfo.wSectorsPerBlock;
            if ((dwSectorAddr / g_FlashInfo.wSectorsPerBlock) >= g_FlashInfo.dwNumBlocks)
            {
                // This should never happen since partition has already been created
                RETAILMSG (1, (TEXT("ReadData: corrupt partition.  Reformat flash.\r\n")));
                return FALSE;
            }
        }

        if (!FMD_ReadSector(dwSectorAddr, pbSector, NULL, 1))
        {
            RETAILMSG (1, (TEXT("ReadData: failed to read sector (0x%x).\r\n"), dwSectorAddr));
            return(FALSE);
        }
        memcpy(pbBuffer, pbSector, dwNumExtraBytes);
    }

    pPartState->dwDataPointer = dwNextPtrValue;
    return(TRUE);
}

/*  BP_WriteData
 *
 *  Writes data to the partition starting at the data pointer.  Call fails
 *  if length of the buffer is too long (i.e. trying to write past the end
 *  of the partition)
 *
 *  ENTRY
 *      hPartition - handle to the partition
 *      pbBuffer - pointer to buffer of data to write
 *      dwLength - length in bytes of the buffer
 *
 *  EXIT
 *      TRUE on success
 */

BOOL BP_WriteData(HANDLE hPartition, LPBYTE pbBuffer, DWORD dwLength)
{
    if (hPartition == INVALID_HANDLE_VALUE)
        return FALSE;

    DWORD dwNumBlocks;
    PPARTSTATE pPartState = (PPARTSTATE) hPartition;
    DWORD dwNextPtrValue = pPartState->dwDataPointer + dwLength;

    RETAILMSG (1, (TEXT("WriteData: Start = 0x%x, Length = 0x%x.\r\n"), pPartState->dwDataPointer, dwLength));

    if (!pbBuffer || !g_pbBlock || dwLength == 0) {
        RETAILMSG(1,(TEXT("BP_WriteData Fails.  pbBuffer = 0x%x, g_pbBlock = 0x%x, dwLength = 0x%x\r\n"), pbBuffer, g_pbBlock, dwLength));
        return(FALSE);
    }

    // Check to make sure buffer size is within limits of partition
    if (((dwNextPtrValue - 1) / g_FlashInfo.wDataBytesPerSector) >= pPartState->pPartEntry->Part_TotalSectors) {
        RETAILMSG (1, (TEXT("WriteData: trying to write past end of partition.\r\n")));
        return FALSE;
    }

    // Get the starting physical block
    DWORD dwBlock = Log2Phys (pPartState->dwDataPointer / g_FlashInfo.wDataBytesPerSector + pPartState->pPartEntry->Part_StartSector) / g_FlashInfo.wSectorsPerBlock;

    DWORD dwOffsetBlock = (pPartState->dwDataPointer + pPartState->pPartEntry->Part_StartSector * g_FlashInfo.wDataBytesPerSector) % g_dwDataBytesPerBlock;

    // Update the global indicating last written physical address.  Global variable is used by the caller.
    g_dwLastWrittenLoc = dwBlock * g_dwDataBytesPerBlock + dwOffsetBlock;

    // If current pointer is not on a block boundary, copy bytes up to the first block boundary
    if (dwOffsetBlock)
    {
        if (!ReadBlock(dwBlock, g_pbBlock, g_pSectorInfoBuf)) {
            RETAILMSG (1, (TEXT("WriteData: failed to read block (0x%x).\r\n"), dwBlock));
            return(FALSE);
        }

        DWORD dwNumBytesWrite = g_dwDataBytesPerBlock - dwOffsetBlock;
        if (dwNumBytesWrite > dwLength)
            dwNumBytesWrite = dwLength;

        memcpy(g_pbBlock + dwOffsetBlock, pbBuffer, dwNumBytesWrite);

        if (!FMD_EraseBlock(dwBlock)) {
            RETAILMSG (1, (TEXT("WriteData: failed to erase block (0x%x).\r\n"), dwBlock));
            return FALSE;
        }

        if (!WriteBlock(dwBlock, g_pbBlock, g_pSectorInfoBuf)) {
            RETAILMSG (1, (TEXT("WriteData: failed to write block (0x%x).\r\n"), dwBlock));
            return(FALSE);
        }

        dwLength -= dwNumBytesWrite;
        pbBuffer += dwNumBytesWrite;
        dwBlock++;
    }

    // Compute number of blocks.
    dwNumBlocks = (dwLength / g_dwDataBytesPerBlock);

    while (dwNumBlocks--)
    {
        // If the block is marked bad, skip to next block.  Note that the assumption in our error checking
        // is that any truely bad block will be marked either by the factory during production or will be marked
        // during the erase and write verification phases.  If anything other than a bad block fails ECC correction
        // in this routine, it's fatal.
        if (IS_BLOCK_UNUSABLE(dwBlock))
        {
            ++dwBlock;
            ++dwNumBlocks;        // Compensate for fact that we didn't write any blocks.
            continue;
        }

        if (!ReadBlock(dwBlock, NULL, g_pSectorInfoBuf)) {
            RETAILMSG (1, (TEXT("WriteData: failed to read block (0x%x).\r\n"), dwBlock));
            return(FALSE);
        }

        if (!FMD_EraseBlock(dwBlock)) {
            RETAILMSG (1, (TEXT("WriteData: failed to erase block (0x%x).\r\n"), dwBlock));
            return FALSE;
        }

        if (!WriteBlock(dwBlock, pbBuffer, g_pSectorInfoBuf)) {
            RETAILMSG (1, (TEXT("WriteData: failed to write block (0x%x).\r\n"), dwBlock));
            return(FALSE);
        }

        ++dwBlock;
        pbBuffer += g_dwDataBytesPerBlock;
    }

    DWORD dwNumExtraBytes = (dwLength % g_dwDataBytesPerBlock);
    if (dwNumExtraBytes)
    {
        // Skip bad blocks
        while (IS_BLOCK_UNUSABLE(dwBlock))
        {
            dwBlock++;
            if (dwBlock >= g_FlashInfo.dwNumBlocks)
            {
                // This should never happen since partition has already been created
                RETAILMSG (1, (TEXT("WriteData: corrupt partition.  Reformat flash.\r\n")));
                return FALSE;
            }

        }

        if (!ReadBlock(dwBlock, g_pbBlock, g_pSectorInfoBuf)) {
            RETAILMSG (1, (TEXT("WriteData: failed to read block (0x%x).\r\n"), dwBlock));
            return(FALSE);
        }

        memcpy(g_pbBlock, pbBuffer, dwNumExtraBytes);

        if (!FMD_EraseBlock(dwBlock)) {
            RETAILMSG (1, (TEXT("WriteData: failed to erase block (0x%x).\r\n"), dwBlock));
            return FALSE;
        }

        if (!WriteBlock(dwBlock, g_pbBlock, g_pSectorInfoBuf)) {
            RETAILMSG (1, (TEXT("WriteData: failed to write block (0x%x).\r\n"), dwBlock));
            return(FALSE);
        }

    }

    pPartState->dwDataPointer = dwNextPtrValue;
    return(TRUE);
}

/*  BP_OpenPartition(Ex)
 *
 *  Opens/creates a partition depending on the creation flags.  If it is opening
 *  and the partition has already been opened, then it returns a handle to the
 *  opened partition.  Otherwise, it loads the state information of that partition
 *  into memory and returns a handle.
 *
 *  ENTRY
 *      dwStartSector - Logical sector to start the partition.  NEXT_FREE_LOC if none
 *          specified.  Ignored if opening existing partition.
 *      dwNumSectors - Number of logical sectors of the partition.  USE_REMAINING_SPACE
 *          to indicate to take up the rest of the space on the flash for that partition (should
 *          only be used when creating extended partitions).  This parameter is ignored
 *          if opening existing partition.
 *      dwPartType - Type of partition to create/open.
 *      fActive - TRUE indicates to create/open the active partition.  FALSE for
 *          inactive.
 *      dwCreationFlags - PART_CREATE_NEW to create only.  Fail if it already
 *          exists.  PART_OPEN_EXISTING to open only.  Fail if it doesn't exist.
 *          PART_OPEN_ALWAYS creates if it does not exist and opens if it
 *          does exist.
 *      dwIndex - Used to designate nth partition of a given type.  Fail if it can't
 *          be created or opened (depending on dwCreationFlags).  Zero-based.
 *
 *  EXIT
 *      Handle to the partition on success.  INVALID_HANDLE_VALUE on error.
 */

HANDLE BP_OpenPartition(DWORD dwStartSector, DWORD dwNumSectors, DWORD dwPartType, BOOL fActive, DWORD dwCreationFlags)
{
    // Back-compat - just manipulate the first partition of type dwPartType
    return BP_OpenPartitionEx(dwStartSector, dwNumSectors, dwPartType, fActive, dwCreationFlags, 0);
}

HANDLE BP_OpenPartitionEx(DWORD dwStartSector, DWORD dwNumSectors, DWORD dwPartType, BOOL fActive, DWORD dwCreationFlags, DWORD dwIndex)
{
        DWORD dwPartIndex;
        BOOL fExists;

        ASSERT (g_pbMBRSector);

        if (!IsValidMBR()) {
            DWORD dwFlags = 0;

            if ((dwCreationFlags == PART_OPEN_EXISTING) || (dwIndex != 0)){
                RETAILMSG(1, (TEXT("OpenPartition: Invalid MBR.  Cannot open existing partition 0x%x.\r\n"), dwPartType));
                return INVALID_HANDLE_VALUE;
            }

            RETAILMSG(1, (TEXT("OpenPartition: Invalid MBR.  Formatting flash.\r\n")));
            if (g_FlashInfo.flashType == NOR) {
                dwFlags |= FORMAT_SKIP_BLOCK_CHECK;
            }
            BP_LowLevelFormat (0, g_FlashInfo.dwNumBlocks, dwFlags);
            dwPartIndex = 0;
            fExists = FALSE;
        }
        else {
            fExists = GetPartitionTableIndex(dwPartType, fActive, dwIndex, &dwPartIndex);
        }

        RETAILMSG(1, (TEXT("OpenPartition: Partition Exists=0x%x for part 0x%x.\r\n"), fExists, dwPartType));
        if (fExists) {

            // Partition was found.
            if (dwCreationFlags == PART_CREATE_NEW)
                return INVALID_HANDLE_VALUE;

            if (g_partStateTable[dwPartIndex].pPartEntry == NULL) {
                // Open partition.  If this is the boot section partition, then file pointer starts after MBR
                g_partStateTable[dwPartIndex].pPartEntry = (PPARTENTRY)(g_pbMBRSector + PARTTABLE_OFFSET + sizeof(PARTENTRY)*dwPartIndex);
                g_partStateTable[dwPartIndex].dwDataPointer = 0;
            }
            return (HANDLE)&g_partStateTable[dwPartIndex];
        }
        else {

            // If there are already 4 partitions, or creation flag specified OPEN_EXISTING, fail.
            if ((dwPartIndex == NUM_PARTS) || (dwCreationFlags == PART_OPEN_EXISTING))
                return INVALID_HANDLE_VALUE;

            // Create new partition
            return CreatePartition (dwStartSector, dwNumSectors, dwPartType, fActive, dwPartIndex);
        }
}


/*  BP_SetDataPointer
 *
 *  Sets the data pointer of a particular partition.  Data pointer stores the logical
 *  byte address where the next read or write will occur.
 *
 *  ENTRY
 *      hPartition - handle to the partition
 *      dwAddress - Address to set data pointer to
 *
 *  EXIT
 *      TRUE on success
 */

BOOL BP_SetDataPointer (HANDLE hPartition, DWORD dwAddress)
{
    if (hPartition == INVALID_HANDLE_VALUE)
        return FALSE;

    RETAILMSG(1,(TEXT("BP_SetDataPointer at 0x%x\r\n"), dwAddress));

    PPARTSTATE pPartState = (PPARTSTATE) hPartition;

    if (dwAddress >= pPartState->pPartEntry->Part_TotalSectors * g_FlashInfo.wDataBytesPerSector)
        return FALSE;

    pPartState->dwDataPointer = dwAddress;
    return TRUE;

}

/*  BP_SetPhysDataPointer
 *
 *  Sets the data pointer of a particular partition.  This function compensates
 *  for bad and reserved blocks, and accounts for them in the supplied address.
 *  This behavior is useful on NOR flash when locating data based on statically
 *  mapped virtual addresses that do not account for reserved ranges.
 *
 *  ENTRY
 *      hPartition - handle to the partition
 *      dwAddress - Physical address to set data pointer to, zero-based from the
 *          partition start.
 *
 *  EXIT
 *      TRUE on success
 */

BOOL BP_SetPhysDataPointer (HANDLE hPartition, DWORD dwPhysAddress)
{
    if (hPartition == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    RETAILMSG(1,(TEXT("BP_SetPhysDataPointer at 0x%x\r\n"), dwPhysAddress));

    DWORD dwCurBlock, dwEndBlock;
    PPARTSTATE pPartState = (PPARTSTATE) hPartition;

    // Compute partition's starting block number. Round up to the next block because a
    // partition can't start in the middle of a reserved/bad block
    dwCurBlock = (Log2Phys(pPartState->pPartEntry->Part_StartSector) + g_FlashInfo.wSectorsPerBlock - 1) / g_FlashInfo.wSectorsPerBlock;

    // Compute physical block where the specified dwPhysAddress lies.
    dwEndBlock = (Log2Phys(pPartState->pPartEntry->Part_StartSector) * g_FlashInfo.wDataBytesPerSector + dwPhysAddress) /
        g_FlashInfo.wDataBytesPerSector / g_FlashInfo.wSectorsPerBlock;

    if (IS_BLOCK_UNUSABLE (dwEndBlock)) {
        // fail because this physical address ends in a bad/reserved block
        RETAILMSG(1,(TEXT("BP_SetPhysDataPointer address 0x%x is in an unusable block\r\n"), dwPhysAddress));
        return FALSE;
    }

    // compensate for the total number of unusable blocks before the requested block
    // by subtracting the block size from the supplied physical address
    // logical address = dwPhysAddres - ((unusable blocks < logical block) * block size)
    while(dwCurBlock < dwEndBlock) {
        if (IS_BLOCK_UNUSABLE (dwCurBlock)) {
            dwPhysAddress -= (g_FlashInfo.wDataBytesPerSector * g_FlashInfo.wSectorsPerBlock);
        }
        dwCurBlock++;
    }

    return BP_SetDataPointer(hPartition, dwPhysAddress);
}


/*  BP_GetPartitionInfo
 *
 *  Get the partition entry for an open partition.
 *
 *  ENTRY
 *      hPartition - handle to the partition
 *
 *  EXIT
 *      The partition entry
 */

PPARTENTRY BP_GetPartitionInfo (HANDLE hPartition)
{
    if (!hPartition)
        return NULL;

    return ((PPARTSTATE)hPartition)->pPartEntry;
}


/*  BP_Init
 *
 *  Sets up locations for various objects in memory provided by caller
 *
 *  ENTRY
 *      pMemory - pointer to memory for storing objects
 *      dwSize - size of the memory
 *      lpActiveReg - used by FMD_Init. NULL if not needed.
 *      pRegIn - used by FMD_Init. NULL if not needed.
 *      pRegOut - used by FMD_Init. NULL if not needed.
 *
 *  EXIT
 *      TRUE on success
 */

BOOL BP_Init (LPBYTE pMemory, DWORD dwSize, LPCTSTR lpActiveReg, PPCI_REG_INFO pRegIn, PPCI_REG_INFO pRegOut)
{
    DWORD dwBufferSize;

    if (!pMemory) {
        RETAILMSG(1,(TEXT("BP_Init Fails No memory fails!!!\r\n")));
        return FALSE;
    }

    if (!FMD_Init (lpActiveReg, pRegIn, pRegOut))
        return FALSE;

    if (!FMD_GetInfo (&g_FlashInfo)) {
        RETAILMSG(1,(TEXT("BP_Init Fails FMD_GetInfo fails!!!\r\n")));
        return FALSE;
    }

    // Check to make sure size is enough for one sector, one block, and sectorinfo buffer for one block
    g_dwDataBytesPerBlock = g_FlashInfo.wDataBytesPerSector * g_FlashInfo.wSectorsPerBlock;
    dwBufferSize = g_FlashInfo.wDataBytesPerSector + g_dwDataBytesPerBlock;
    if (dwBufferSize < g_dwDataBytesPerBlock) {
        return FALSE; // integer overflow
    }
    dwBufferSize += (g_FlashInfo.wSectorsPerBlock * sizeof(SectorInfo));
    if (dwBufferSize < (g_FlashInfo.wSectorsPerBlock * sizeof(SectorInfo))) {
        return FALSE; // integer overflow
    }
    if (dwSize < dwBufferSize) {
        RETAILMSG(1,(TEXT("BP_Init Fails buffer size = %x < required = %x!!!\r\n"),dwSize,dwBufferSize));
        return FALSE;
    }

    for (int i = 0; i < NUM_PARTS; i++) {
        g_partStateTable[i].pPartEntry= NULL;
        g_partStateTable[i].dwDataPointer = 0;
    }

    g_pbMBRSector = pMemory;  //size = g_FlashInfo.wDataBytesPerSector;
    g_pbBlock = pMemory + g_FlashInfo.wDataBytesPerSector;  //size = g_dwDataBytesPerBlock;
    g_pSectorInfoBuf = (PSectorInfo)(g_pbBlock + g_dwDataBytesPerBlock);  //size = g_FlashInfo.wSectorsPerBlock * sizeof(SectorInfo);
    g_dwLastLogSector = 0;

    return TRUE;
}



/*  BP_LowLevelFormat
 *
 *  Called when preparing flash for a multiple-BIN download.
 *  Erases, verifies, and writes logical sector numbers in the range to be written.
 *
 *  ENTRY
 *      dwStartBlock - starting physical block for format
 *      dwNumBlocks - number of physical blocks to format
 *      dwFlags - Flags used in formatting.
 *
 *  EXIT
 *      TRUE returned on success and FALSE on failure.
 */

BOOL BP_LowLevelFormat(DWORD dwStartBlock, DWORD dwNumBlocks, DWORD dwFlags)
{
    dwNumBlocks = min (dwNumBlocks, g_FlashInfo.dwNumBlocks);

    RETAILMSG(1,(TEXT("Enter LowLevelFormat [0x%x, 0x%x].\r\n"), dwStartBlock, dwStartBlock + dwNumBlocks - 1));

    // Erase all the flash blocks.
    if (!EraseBlocks(dwStartBlock, dwNumBlocks, dwFlags))
        return(FALSE);

    // Determine first good starting block
    while (IS_BLOCK_UNUSABLE (dwStartBlock) && dwStartBlock < g_FlashInfo.dwNumBlocks) {
        dwStartBlock++;
    }

    if (dwStartBlock >= g_FlashInfo.dwNumBlocks) {
        RETAILMSG(1,(TEXT("BP_LowLevelFormat: no good blocks\r\n")));
        return FALSE;
    }

    // MBR goes in the first sector of the starting block.  This will be logical sector 0.
    g_dwMBRSectorNum = dwStartBlock * g_FlashInfo.wSectorsPerBlock;

    // Create an MBR.
    CreateMBR();

    RETAILMSG (1, (TEXT("Done.\r\n\r\n")));
    return(TRUE);
}

