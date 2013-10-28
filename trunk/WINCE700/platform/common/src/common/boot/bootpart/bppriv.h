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
#ifndef _BPPRIV_H_
#define _BPPRIV_H_

#include <fmd.h>

#define NUM_PARTS                   4
#define SIZE_END_SIG              2
#define PART_ENTRY_SIG          0xabcdabcd
#define INVALID_ADDR            0xffffffff
#define INVALID_PART             0xffffffff
#define INVALID_HANDLE         (HANDLE)-1
// end of sector - 2 bytes for signature - maximum of 4 16-byte partition records
#define PARTTABLE_OFFSET        (SECTOR_SIZE - SIZE_END_SIG - (sizeof(PARTENTRY) * NUM_PARTS))

#define SECTOR_WRITE_COMPLETED 0x0004  // Indicates data is valid for the FAL
#define MINIMUM_FLASH_BLOCKS_TO_RESERVE             2
#define PERCENTAGE_OF_MEDIA_TO_RESERVE              400     // 0.25% of the media {NOTE: 100% / 0.25% == 400}

typedef struct _PARTSTATE {
        PPARTENTRY  pPartEntry;
        DWORD         dwDataPointer;        // Pointer to where next read and write will occur
} PARTSTATE, *PPARTSTATE;


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// stores a cylinder/head/sector based ATA address
typedef struct _CHSAddr {
    WORD cylinder;
    WORD head;
    WORD sector;
} CHSAddr, *PCHSAddr;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// stores a Logical Block Address
typedef DWORD LBAAddr, *PLBAAddr;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
typedef enum { CHS, LBA } CHSLBA ;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// stores a union of LBA and CHS address
typedef struct _Addr {
    CHSLBA type;
    union {
        LBAAddr lba;
        CHSAddr chs;
    };
} Addr, *PAddr;


extern "C"
{
//
// Bootpart.cpp private helper functions
//
static Addr LBAtoCHS(FlashInfo *pFlashInfo, Addr lba);
//CHStoLBA generates C4505 warning: "unreferenced local function has been removed". 
//Keep it as comments for reference.
//static Addr CHStoLBA(FlashInfo *pFlashInfo, Addr chs);
static BOOL CreateMBR();
static BOOL IsValidMBR();
static BOOL IsValidPart (PPARTENTRY pPartEntry);
static void AddPartitionTableEntry(DWORD entry, DWORD startSector, DWORD totalSectors, BYTE fileSystem, BYTE bootInd);
static BOOL GetPartitionTableIndex (DWORD dwPartType, BOOL fActive, DWORD dwIndex, PDWORD pdwIndex);
static BOOL WriteLogicalNumbers (DWORD dwStartSector, DWORD dwNumSectors, BOOL fReadOnly);
static DWORD GetMBRSectorNum ();
static BOOL Format ();
static DWORD LastLogSector();
static DWORD Log2Phys (DWORD dwLogSector);
static DWORD FindFreeSector();
static HANDLE CreatePartition (DWORD dwStartSector, DWORD dwNumSectors, DWORD dwPartType, BOOL fActive, DWORD dwPartIndex);
static BOOL ReadBlock (DWORD dwBlock, LPBYTE pbBlock, PSectorInfo pSectorInfoTable);
static BOOL WriteBlock (DWORD dwBlock, LPBYTE pbBlock, PSectorInfo pSectorInfoTable);

BOOL EraseAllBlocks(void);
BOOL EraseBlocks(DWORD dwStartBlock, DWORD dwNumBlocks, DWORD dwFlags);
}

extern FlashInfo g_FlashInfo;
extern LPBYTE g_pbBlock;



#endif  // _BPPRIV_H_

