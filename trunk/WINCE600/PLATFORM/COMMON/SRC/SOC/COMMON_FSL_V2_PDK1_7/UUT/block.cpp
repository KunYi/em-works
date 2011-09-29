//------------------------------------------------------------------------------
//
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//  block.cpp
//  Block-based SCSI-2 direct-access device (hard disk drive) emulator.
//------------------------------------------------------------------------------ 

#pragma warning(push)
#pragma warning(disable: 4201)
#include <windows.h>
#pragma warning(pop)

#include <diskio.h>
#include <storemgr.h>
#include <devload.h>
#include "proxy.h"
#include "scsi2.h"
#include "transporttypes.h"
#include "usbmsfndbg.h"
#include "utp.h"


#include <pshpack1.h>

typedef struct _UFI_CB {
    BYTE  bOpCode;                // 0
    BYTE  bReserved1:5;           // 1
    BYTE  bLogicalUnitNumber : 3;
    DWORD dwLogicalBlockAddress;  // 2-5
    BYTE  bReserved2;             // 6
    WORD  wTransferLength;        // 7-8
    BYTE  bReserved3[3];          // 9-11
} UFI_CB, *PUFI_CB;

#include <poppack.h>

#if DEBUG
#define DUMP_DISKINFO(di) { \
    DEBUGMSG(1, (_T("%s bytes per sector = %u\r\n"), pszFname, di.di_bytes_per_sect)); \
    DEBUGMSG(1, (_T("%s cylinders = %u\r\n"), pszFname, di.di_cylinders)); \
    DEBUGMSG(1, (_T("%s flags = 0x%x\r\n"), pszFname, di.di_flags)); \
    DEBUGMSG(1, (_T("%s heads = %u\r\n"), pszFname, di.di_heads)); \
    DEBUGMSG(1, (_T("%s sectors = %u\r\n"), pszFname, di.di_sectors)); \
    DEBUGMSG(1, (_T("%s total sectors = %u\r\n"), pszFname, di.di_total_sectors)); \
    }
#define DUMP_PARTENTRY(ppe) { \
    DEBUGMSG(1, (_T("%s bootind=0x%x \r\n"), pszFname, ppe->Part_BootInd)); \
    DEBUGMSG(1, (_T("%s filesystem=0x%x \r\n"), pszFname, ppe->Part_FileSystem)); \
    DEBUGMSG(1, (_T("%s firsthead=%u\r\n"), pszFname, ppe->Part_FirstHead)); \
    DEBUGMSG(1, (_T("%s firstsector=%u\r\n"), pszFname, ppe->Part_FirstSector)); \
    DEBUGMSG(1, (_T("%s firsttrack=%u\r\n"), pszFname, ppe->Part_FirstTrack)); \
    DEBUGMSG(1, (_T("%s lasthead=%u\r\n"), pszFname, ppe->Part_LastHead)); \
    DEBUGMSG(1, (_T("%s lastsector=%u\r\n"), pszFname, ppe->Part_LastSector)); \
    DEBUGMSG(1, (_T("%s lasttrack=%u\r\n"), pszFname, ppe->Part_LastTrack)); \
    DEBUGMSG(1, (_T("%s startsector=%u\r\n"), pszFname, ppe->Part_StartSector)); \
    DEBUGMSG(1, (_T("%s totalsectors=%u\r\n"), pszFname, ppe->Part_TotalSectors)); \
    }
#else
#define DUMP_DISKINFO(di)
#define DUMP_PARTENTRY(pe)
#endif // DEBUG

// Helpful macros
#define CLOSE_KEY(x)    if (x != NULL) RegCloseKey(x)
#define LOCAL_FREE(x)   if (x != NULL) LocalFree(x)
#define CLOSE_HANDLE(x) if (x != NULL) CloseHandle(x)

// SCSI-2 command data size definitions
#define DATASIZE_INQUIRY       36
#define DATASIZE_MODE_SENSE6   8
#define DATASIZE_MODE_SENSE10  512
#define DATASIZE_REQUEST_SENSE 18

// SCSI-2 direct-access device state
static const UCHAR g_bSenseKey = SENSE_NOT_READY;
static const UCHAR g_bASC = ASC_MEDIUM_NOT_PRESENT;
static const UCHAR g_bASCQ = 0;
static BOOL        g_fInitialized = FALSE;

// Properties of exposed device
static HANDLE    g_hStore;
static HANDLE    g_hStore1 = INVALID_HANDLE_VALUE;
static DISK_INFO g_diDiskInfo;
static TCHAR     g_szDeviceName[PRX_DEVICE_NAME_LEN];
static BOOL      g_fLegacyBlockDriver = FALSE; // IOCTL_DISK_Xxx or DISK_IOCTL_Xxx

// Whether to report to USB host that exposed device is removable
static DWORD g_dwRemovable = 1;

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

#define PART_PRIMARY  0x00   // Part_BootInd stores the partition type; see PARTENTRY
#define PART_EXTENDED 0x05   // Part_FileSystem stores the file system type; see PARTENTRY

// Partition table entry
typedef struct _PARTENTRY {
    BYTE  Part_BootInd;      // Boot index (80h = boot partition)
    BYTE  Part_FirstHead;    // Partition starting head based 0
    BYTE  Part_FirstSector;  // Partition starting sector based 1
    BYTE  Part_FirstTrack;   // Partition starting track based 0
    BYTE  Part_FileSystem;   // Partition type signature field
    BYTE  Part_LastHead;     // Partition ending head based 0
    BYTE  Part_LastSector;   // Partition ending sector based 1
    BYTE  Part_LastTrack;    // Partition ending track based 0
    DWORD Part_StartSector;  // Physical starting sector based 0
    DWORD Part_TotalSectors; // Total physical sectors in partition
} PARTENTRY;
typedef PARTENTRY UNALIGNED *PPARTENTRY;

#define DEFAULT_SECTOR_SIZE    512
#define MAX_PARTTABLE_ENTRIES  4
#define SIZE_PARTTABLE_ENTRIES 16
#define PARTTABLE_OFFSET       (DEFAULT_SECTOR_SIZE - 2 - (SIZE_PARTTABLE_ENTRIES * MAX_PARTTABLE_ENTRIES))

typedef struct _PARTTABLE {
    PARTENTRY PartEntry[MAX_PARTTABLE_ENTRIES];
} PARTTABLE;
typedef PARTTABLE UNALIGNED *PPARTTABLE;

// List of partition names
static PTCHAR g_ptcPartitions = NULL;

// Copy of physical disk's MBR (to facilitate write-protection); it is possible
// for a host to attempt to reorganize a disk while only a subset of its
// partitions are exposed; unexposed partitions appear as unallocated space;
// we need a copy of the actual MBR to determine the sector range of an
// unexposed partition, to protect unexposed partitions from modification
static LPBYTE g_lpbPhysMBR = NULL;

// Virtual disk's MBR; this is the modified copy of the MBR that we advertise to
// the USB host when the disk is only exposing a subset of its partitions; the
// modified MBR contains "holes" where unexposed partitions exist
static LPBYTE g_lpbMBR = NULL;

// Bit mask of primary partitions selected for exposure; bit 0 = primary 1
// selected, bit 1 = primary 2 selected, etc.
static BYTE g_bmPartitions = 0;

extern BOOL g_bUTPMsgReplyInfoNeeded;
// ----------------------------------------------------------------------------
// Function: BytesPerSector
//     Return number of bytes per sector on exposed device
//
// Parameters:
//     None
// ----------------------------------------------------------------------------

static
DWORD
inline
BytesPerSector(
    )
{
    DEBUGCHK(g_fInitialized);
    return g_diDiskInfo.di_bytes_per_sect;
}

// ----------------------------------------------------------------------------
// Function: ByteSwapUlong
//     Same as _byteswap_ulong
//
// Parameters:
//     ULONG to swap
// ----------------------------------------------------------------------------

static
inline
ULONG
ByteSwapUlong(
    ULONG ul
    )
{
    PBYTE pb = (PBYTE)&ul;
    ULONG ulRet = pb[3];
    ulRet |= pb[2] << 8;
    ulRet |= pb[1] << 16;
    ulRet |= pb[0] << 24;
    return ulRet;
}

// ----------------------------------------------------------------------------
// Function: ByteSwapUshort
//     Same as _byteswap_ushort
//
// Parameters:
//     USHORT to swap
// ----------------------------------------------------------------------------

static
inline
USHORT 
ByteSwapUshort(
    USHORT us
    )
{
    PBYTE pb = (PBYTE)&us;
    USHORT usRet = pb[1];
    usRet |= pb[0] << 8;
    return usRet;
}

// ----------------------------------------------------------------------------
// Function: ProcessScsiInquiry
//     Process a SCSI-2 INQURY command
//
// Parameters:
//     ptcCommand - command block wrapper
//     ptdData - data block wrapper
// ----------------------------------------------------------------------------

static
DWORD
ProcessScsiInquiry(
    PTRANSPORT_COMMAND ptcCommand,
    PTRANSPORT_DATA ptdData
    )
{
#ifdef DEBUG // Remove-W4: Warning C4189 workaround
    SETFNAME(_T("ProcessScsiInquiry"));
#endif

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(ptcCommand);
    PBYTE pbData = (PBYTE) ptdData->DataBlock;
    DWORD dwResult = EXECUTE_FAIL;

    // don't test if LUN is valid; LUNs are deprecated

    // test if data block is large enough
    if (ptdData->RequestLength < DATASIZE_INQUIRY) {
        DEBUGMSG(ZONE_ERROR, (_T(
            "%s host requesting less than required; ptdData->RequestLength < DATASIZE_INQUIRY\r\n"
            ), pszFname));
#ifdef QFE_MERGE /*080430*/ /*CE6QFE*/
#else
        goto EXIT;
#endif
    }

    ZeroMemory(pbData, ptdData->RequestLength);
    ptdData->TransferLength = ptdData->RequestLength;

    // peripherial qualifier    = 0x0
    // peripherial device type  = 0x0 (SCSI_DEVICE_DIRECT_ACCESS)
    // RMB                      = 0x0 (non-removable medium)

    pbData[1] = ((g_dwRemovable ? 1 : 0) << 7); // RMB = 1; removable medium
    pbData[4] = 0x1F;//Additional length    
    pbData[8] = 'F';
    pbData[9] = 'S';
    pbData[10] = 'L';	
    pbData[16] = 'U';
    pbData[17] = 'U';
    pbData[18] = 'C';                   
    pbData[32] = '1';
    pbData[33] = '.';
    pbData[34] = '0';     

    dwResult = EXECUTE_PASS;

#ifdef QFE_MERGE /*080430*/ /*CE6QFE*/
#else
EXIT:;
#endif
    return dwResult;
}

// ----------------------------------------------------------------------------
// Function: ProcessScsiModeSense6
//     Process a SCSI-2 MODE SENSE (6) command
//
// Parameters:
//     ptcCommand - command block wrapper
//     ptdData - data block wrapper
// ----------------------------------------------------------------------------

static
DWORD
ProcessScsiModeSense6(
    PTRANSPORT_COMMAND ptcCommand,
    PTRANSPORT_DATA ptdData
    )
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(ptcCommand);
	PBYTE pbData = (PBYTE) ptdData->DataBlock;	
    DWORD dwResult = EXECUTE_FAIL;

    ZeroMemory(pbData, ptdData->RequestLength);
	
    ptdData->TransferLength = ptdData->RequestLength;

	pbData[0] = 0x0F;
	pbData[4] = 0x08;	
	pbData[5] = 0x0A;
	pbData[6] = 0x04;
	pbData[8] = 0xFF;
	pbData[9] = 0xFF;
	pbData[12] = 0xFF;
	pbData[13] = 0xFF;
	pbData[14] = 0xFF;
	pbData[15] = 0xFF;

    return dwResult;

}

// ----------------------------------------------------------------------------
// Function: ProcessScsiModeSelect6
//     Process a SCSI-2 MODE SELECT (6) command
//
// Parameters:
//     ptcCommand - command block wrapper
//     ptdData - data block wrapper
// ----------------------------------------------------------------------------

static
DWORD
ProcessScsiModeSelect6(
    PTRANSPORT_COMMAND ptcCommand,
    PTRANSPORT_DATA ptdData
    )
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(ptcCommand);
	PBYTE pbData = (PBYTE) ptdData->DataBlock;	
    DWORD dwResult = EXECUTE_FAIL;

    ZeroMemory(pbData, ptdData->RequestLength);
	
    ptdData->TransferLength = ptdData->RequestLength;

	pbData[3] = 0x08;
	pbData[11] = 0x02;	
	pbData[13] = 0x08;	
	pbData[14] = 0x0A;
	pbData[16] = 0xFF;
	pbData[17] = 0xFF;
	pbData[20] = 0xFF;
	pbData[21] = 0xFF;
	pbData[22] = 0xFF;
	pbData[23] = 0xFF;

    return dwResult;

}


// ----------------------------------------------------------------------------
// Function: ProcessScsiModeSense10
//     Process a SCSI-2 MODE SENSE (10) command
//
// Parameters:
//     ptcCommand - command block wrapper
//     ptdData - data block wrapper
// ----------------------------------------------------------------------------

static
DWORD
ProcessScsiModeSense10(
    PTRANSPORT_COMMAND ptcCommand,
    PTRANSPORT_DATA ptdData
    )
{
#ifdef DEBUG // Remove-W4: Warning C4189 workaround
    SETFNAME(_T("ProcessScsiModeSense10"));
#endif
    PBYTE pbCommand = (PBYTE) ptcCommand->CommandBlock;
    PBYTE pbData = (PBYTE) ptdData->DataBlock;
    DWORD dwResult = EXECUTE_FAIL;

    // don't test if LUN is valid; LUNs are deprecated

    // test if data block is large enough
    if (ptdData->RequestLength < DATASIZE_MODE_SENSE10) {
        DEBUGMSG(ZONE_ERROR, (_T(
            "%s host requesting less than required; ptdData->RequestLength < DATASIZE_MODE_SENSE10\r\n"
            ), pszFname));
#ifdef QFE_MERGE /*080430*/ /*CE6QFE*/
#else
        goto EXIT;
#endif
    }

    // test if DBD (disable block descriptors) bit is high
    if (!(pbCommand[1] & 0x08)) {
        DEBUGMSG(ZONE_ERROR, (_T("%s DBD is not enabled\r\n"), pszFname));
        goto EXIT;
    }

    // test if PC is 0
    if ((pbCommand[2] & 0xC0) != 0) {
        DEBUGMSG(ZONE_ERROR, (_T("%s PC is not 0\r\n"), pszFname));
        goto EXIT;
    }

    // test if page code is 0
    if ((pbCommand[2] & 0x3F) != 0) {
        DEBUGMSG(ZONE_ERROR, (_T("%s page code is not 0\r\n"), pszFname));
        goto EXIT;
    }

    ZeroMemory(pbData, ptdData->RequestLength);
    ptdData->TransferLength = ptdData->RequestLength;

    // only return the header

    // mode data length
    pbData[0] = 0; // msb
    pbData[1] = 6; // lsb
    // medium type, 9.3.3
    pbData[2] = 0x00; // default medium type
    // device-specific parameter, 9.3.3
    pbData[3] = 0x00; // bit 7 = WP (write protected); read from registry

    dwResult = EXECUTE_PASS;

EXIT:;
    return dwResult;
}

// ----------------------------------------------------------------------------
// Function: ProcessScsiRequestSense
//     Process a SCSI-2 REQUEST SENSE command
//
// Parameters:
//     ptcCommand - command block wrapper
//     ptdData - data block wrapper
// ----------------------------------------------------------------------------

static
DWORD
ProcessScsiRequestSense(
    PTRANSPORT_COMMAND ptcCommand,
    PTRANSPORT_DATA ptdData
    )
{
#ifdef DEBUG // Remove-W4: Warning C4189 workaround
    SETFNAME(_T("ProcessScsiRequestSense"));
#endif

    const BYTE bErrorCode = 0x70;
    const BYTE bAdditionalSenseLength = 10;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(ptcCommand);
    PBYTE pbData = (PBYTE) ptdData->DataBlock;
    DWORD dwResult = EXECUTE_FAIL;

    // don't test if LUN is valid; LUNs are deprecated

    // test if data block is large enough
    if (ptdData->RequestLength < sizeof(SENSE_DATA)) {
        DEBUGMSG(ZONE_ERROR, (_T(
            "%s host requesting less than required; ptdData->RequestLength < %u\r\n"
            ), pszFname, sizeof(SENSE_DATA)));
#ifdef QFE_MERGE /*080430*/ /*CE6QFE*/
#else
        goto EXIT;
#endif
    }

    ZeroMemory(pbData, ptdData->RequestLength);
    ptdData->TransferLength = ptdData->RequestLength;

    PSENSE_DATA pSenseData;
    pSenseData = (PSENSE_DATA) pbData;

    pSenseData->ErrorCode = bErrorCode;	

	/*pSenseData->Valid= 0xFF;
	pSenseData->CommandSpecificInformation[0]=0x00;
	pSenseData->CommandSpecificInformation[1]=0xAA;
	pSenseData->CommandSpecificInformation[2]=0x55;
	pSenseData->CommandSpecificInformation[3]=0x20;*/	

	pSenseData->AdditionalSenseLength = bAdditionalSenseLength;


	if(g_bUTPMsgReplyInfoNeeded){
		UTPCmdResponse(pSenseData);
		g_bUTPMsgReplyInfoNeeded = FALSE;
	}
	else{
	    pSenseData->SenseKey = g_bSenseKey;
	    pSenseData->AdditionalSenseCode = g_bASC;
	    pSenseData->AdditionalSenseCodeQualifier = g_bASCQ;		
	}

    dwResult = EXECUTE_PASS;

#ifdef QFE_MERGE /*080430*/ /*CE6QFE*/
#else
EXIT:;
#endif
    return dwResult;
}

// ----------------------------------------------------------------------------
// Function: ProcessScsiSendDiagnostic
//     Process a SCSI-2 SEND DIAGNOSTIC command
//
// Parameters:
//     ptcCommand - command block wrapper
// ----------------------------------------------------------------------------

static
DWORD
ProcessScsiSendDiagnostic(
    PTRANSPORT_COMMAND ptcCommand
    )
{
#ifdef DEBUG // Remove-W4: Warning C4189 workaround
    SETFNAME(_T("ProcessScsiSendDiagnostic"));
#endif

    const BYTE bSelfTest = 0x04;

    PBYTE pbCommand = (PBYTE) ptcCommand->CommandBlock;
    DWORD dwResult = EXECUTE_FAIL;

    // don't test if LUN is valid; LUNs are deprecated

    // test if self-test bit is high
    if (!(pbCommand[1] & bSelfTest)) {
        DEBUGMSG(ZONE_ERROR, (_T("%s only self-test is supported\r\n"), pszFname));
        goto EXIT;
    }

    // nothing to check
    dwResult = EXECUTE_PASS;

EXIT:;
    return dwResult;
}

// ----------------------------------------------------------------------------
// Function: ProcessScsiTestUnitReady
//     Process a SCSI-2 TEST UNIT READY command
//
// Parameters:
//     ptcCommand - command block wrapper
// ----------------------------------------------------------------------------

static
DWORD
ProcessScsiTestUnitReady(
    PTRANSPORT_COMMAND ptcCommand
    )
{
#if 0 // Remove-W4: Warning C4189 workaround
    SETFNAME(_T("ProcessScsiTestUnitReady"));
#endif

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(ptcCommand);
    DWORD dwResult = EXECUTE_FAIL;

    // don't test if LUN is valid; LUNs are deprecated

    // nothing to check
    //dwResult = EXECUTE_PASS;
    return dwResult;
}

// ----------------------------------------------------------------------------
// Function: ProcessScsiRead10
//     Process a SCSI-2 READ (10) command
//
// Parameters:
//     ptcCommand - command block wrapper
//     ptdData - data block wrapper
// ----------------------------------------------------------------------------

static
DWORD
ProcessScsiRead10(
    PTRANSPORT_COMMAND ptcCommand,
    PTRANSPORT_DATA ptdData
    )
{
#ifdef DEBUG // Remove-W4: Warning C4189 workaround
    SETFNAME(_T("ProcessScsiRead10"));
#endif

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(ptcCommand);
	PBYTE pbData = (PBYTE) ptdData->DataBlock;
    DWORD dwResult = EXECUTE_FAIL;

    ZeroMemory(pbData, ptdData->RequestLength);
    ptdData->TransferLength = ptdData->RequestLength;

    return dwResult;

}

// ----------------------------------------------------------------------------
// Function: ProcessScsiReadCapacity
//     Process a SCSI-2 READ CAPACITY command
//
// Parameters:
//     ptcCommand - command block wrapper
//     ptdData - data block wrapper
// ----------------------------------------------------------------------------

static
DWORD
ProcessScsiReadCapacity(
    PTRANSPORT_COMMAND ptcCommand,
    PTRANSPORT_DATA ptdData
    )
{
#ifdef DEBUG // Remove-W4: Warning C4189 workaround
    SETFNAME(_T("ProcessScsiReadCapacity"));
#endif

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(ptcCommand);
	PBYTE pbData = (PBYTE) ptdData->DataBlock;
    DWORD dwResult = EXECUTE_FAIL;


    ZeroMemory(pbData, ptdData->RequestLength);
    ptdData->TransferLength = ptdData->RequestLength;

    return dwResult;
}

// ----------------------------------------------------------------------------
// Function: ProcessScsiReadForamtCapacity
//     Process a SCSI-2 READ FORMAT CAPACITY command
//
// Parameters:
//     ptcCommand - command block wrapper
//     ptdData - data block wrapper
// ----------------------------------------------------------------------------

static
DWORD
ProcessScsiReadFormatCapacity(
    PTRANSPORT_COMMAND ptcCommand,
    PTRANSPORT_DATA ptdData
    )
{
#ifdef DEBUG // Remove-W4: Warning C4189 workaround
    SETFNAME(_T("ProcessScsiReadForamtCapacity"));
#endif
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(ptcCommand);
	PBYTE pbData = (PBYTE) ptdData->DataBlock;
    DWORD dwResult = EXECUTE_FAIL;


    ZeroMemory(pbData, ptdData->RequestLength);
    ptdData->TransferLength = ptdData->RequestLength;

    return dwResult;


   
}

// ----------------------------------------------------------------------------
// Function: ProcessScsiStartStop
//     Process a SCSI-2 START STOP command
//
// Parameters:
//     ptcCommand - command block wrapper
// ----------------------------------------------------------------------------

static
DWORD
ProcessScsiStartStop(
    PTRANSPORT_COMMAND ptcCommand
    )
{
#ifdef DEBUG // Remove-W4: Warning C4189 workaround
    SETFNAME(_T("ProcessScsiStartStop"));
#endif

    PBYTE pbCommand = (PBYTE) ptcCommand->CommandBlock;
    DWORD dwResult = EXECUTE_FAIL;

    // don't test if LUN is valid; LUNs are deprecated

    if (pbCommand[4] & 0x02) {
        // LoEj = 1
        if (pbCommand[4] & 0x01) {
            // Start = 1
            // load medium
            DEBUGMSG(ZONE_COMMENT, (_T("%s load medium\r\n"), pszFname));
        }
        else {
            // Start = 0
            // unload medium
            DEBUGMSG(ZONE_COMMENT, (_T("%s unload medium\r\n"), pszFname));
        }
    }
    else {
        // LoEj = 0
        if (pbCommand[4] & 0x01) {
            // Start = 1
            // start medium
            DEBUGMSG(ZONE_COMMENT, (_T("%s start medium\r\n"), pszFname));
        }
        else {
            // Start = 0
            // stop medium
            DEBUGMSG(ZONE_COMMENT, (_T("%s stop medium\r\n"), pszFname));
        }
    }

    dwResult = EXECUTE_PASS;
    return dwResult;
}

// ----------------------------------------------------------------------------
// Function: ProcessScsiWrite10
//     Process a SCSI-2 WRITE (10) command
//
// Parameters:
//     ptcCommand - command block wrapper
//     ptdData - data block wrapper
// ----------------------------------------------------------------------------

static
DWORD
ProcessScsiWrite10(
    PTRANSPORT_COMMAND ptcCommand,
    PTRANSPORT_DATA ptdData
    )
{
#ifdef DEBUG // Remove-W4: Warning C4189 workaround
    SETFNAME(_T("ProcessScsiWrite10"));
#endif

    PBYTE pbCommand = (PBYTE) ptcCommand->CommandBlock;
    PBYTE pbData = (PBYTE) ptdData->DataBlock;
    DWORD dwLogicalBlockAddress;
    DWORD dwTransferLength;
    DWORD dwResult = EXECUTE_FAIL;

    SG_REQ sgSgReq;
    SG_BUF sgSgBuf;
    DWORD dwBytesReturned;
    BOOL fResult = FALSE;

    DWORD dwIndex;

    PUFI_CB pUfiCb = (PUFI_CB) pbCommand;
    DEBUGCHK(pUfiCb->bReserved1 == 0);

    // don't test if LUN is valid; LUNs are deprecated

    // test if logical block address is valid
    dwLogicalBlockAddress = ByteSwapUlong(pUfiCb->dwLogicalBlockAddress);
    // test if transfer length is valid
    dwTransferLength = ByteSwapUshort(pUfiCb->wTransferLength);

    DEBUGMSG(ZONE_COMMENT, (_T(
        "%s starting LBA/sector = %u, transfer length = %u (sectors)\r\n"
        ), pszFname, dwLogicalBlockAddress, dwTransferLength));

    ptdData->TransferLength = dwTransferLength * BytesPerSector();

    // is the host attempting to overwrite the virtual disk's MBR?
    if ((dwLogicalBlockAddress == 0) && (g_lpbMBR != NULL)) {
        DEBUGMSG(ZONE_COMMENT, (_T(
            "%s write request targeting virtual disk's MBR; virtual disk's MBR is write-protected\r\n"
            ), pszFname));
        dwResult = EXECUTE_FAIL;
        goto EXIT;
    }

    // if partitions being exposed selectively, then an unexposed partition
    // appears as unallocated space to the host; we want to prevent access to
    // unexposed partitions; ensure write request does not target a sector in
    // an unexposed partition

    // are partitions being exposed selectively?
    if (g_lpbMBR != NULL) {

        // cycle through bit mast of partitions selected for exposure; (0
        // denotes an unexposed partition)
        for (dwIndex = 0; dwIndex < MAX_PARTTABLE_ENTRIES; dwIndex += 1) {

            if ((g_bmPartitions & (1 << dwIndex)) == 0) {

                // this partition is unexposed

                // fetch corresponding partition table entry
                PPARTENTRY pPartEntry = (PPARTENTRY) ((g_lpbPhysMBR + PARTTABLE_OFFSET) + (dwIndex * sizeof(PARTENTRY)));

                // determine whether the current write request lies in the
                // range of the unexposed partition
                if ((dwLogicalBlockAddress >= pPartEntry->Part_StartSector) &&
                    (dwLogicalBlockAddress <= (pPartEntry->Part_StartSector + pPartEntry->Part_TotalSectors))
                ) {
                    DEBUGMSG(ZONE_COMMENT, (_T(
                        "%s write request targets unexposed partition\r\n"
                        ), pszFname));
                    dwResult = EXECUTE_FAIL;
                    goto EXIT;
                }
            }
        }
    }

    // prepare scatter/gather buffer
    sgSgBuf.sb_buf = (PBYTE) pbData;
    sgSgBuf.sb_len = ptdData->TransferLength;

    // prepare scatter/gather request
    sgSgReq.sr_start = dwLogicalBlockAddress;
    sgSgReq.sr_num_sec = dwTransferLength;
    sgSgReq.sr_status = 0;
    sgSgReq.sr_callback = NULL;
    sgSgReq.sr_num_sg = 1;
    sgSgReq.sr_sglist[0] = sgSgBuf;
    
    DWORD dwIoControlCode = (g_fLegacyBlockDriver) ? DISK_IOCTL_WRITE : IOCTL_DISK_WRITE;
    fResult = DeviceIoControl(
        g_hStore,
        dwIoControlCode,
        &sgSgReq,
        sizeof(sgSgReq),
        NULL,
        0,
        &dwBytesReturned,
        NULL);
    if (fResult) {
        DEBUGMSG(ZONE_COMMENT, (_T(
            "%s IOCTL_DISK_WRITE passed; %u bytes read\r\n"
            ), pszFname, dwBytesReturned));
    }
    else {
        DWORD dwError;
		dwError = GetLastError();
        DEBUGMSG(ZONE_ERROR, (_T("%s IOCTL_DISK_WRITE failed; error = %u\r\n"
        ), pszFname, dwError));
        goto EXIT;
    }

    dwResult = EXECUTE_PASS;
    
EXIT:;
    return dwResult;
}

// ----------------------------------------------------------------------------
// Function: ProcessScsiPreventAllowMediumRemoval
//     Process a SCSI-2 PREVENT ALLOW MEDIUM REMOVAL command
//
// Parameters:
//     ptcCommand - command block wrapper
// ----------------------------------------------------------------------------

static
DWORD
ProcessScsiPreventAllowMediumRemoval(
    PTRANSPORT_COMMAND ptcCommand
    )
{
#ifdef DEBUG // Remove-W4: Warning C4189 workaround
    SETFNAME(_T("ProcessScsiPreventAllowMediumRemoval"));
#endif

    const BYTE bPreventBit = 0x01;
    PBYTE pbCommand = (PBYTE) ptcCommand->CommandBlock;
    DWORD dwResult = EXECUTE_FAIL;

    // don't test if LUN is valid; LUNs are deprecated

    // test if prevent bit high
    if (pbCommand[4] & bPreventBit) {
        DEBUGMSG(ZONE_COMMENT, (_T("%s prevent enabled\r\n"), pszFname));
    }
    else {
        DEBUGMSG(ZONE_COMMENT, (_T("%s prevent disabled\r\n"), pszFname));
    }

    dwResult = EXECUTE_PASS;
    return dwResult;
}

// ----------------------------------------------------------------------------
// Function: ProcessScsiVerify
//     Process a SCSI-2 VERIFY command
//
// Parameters:
//     ptcCommand - command block wrapper
// ----------------------------------------------------------------------------

static
DWORD
ProcessScsiVerify(
    PTRANSPORT_COMMAND ptcCommand
    )
{
#ifdef DEBUG // Remove-W4: Warning C4189 workaround
    SETFNAME(_T("ProcessScsiVerify"));
#endif

#ifdef DEBUG // Remove-W4: Warning C4189 workaround
    PBYTE pbCommand = (PBYTE) ptcCommand->CommandBlock;
#else
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(ptcCommand);
#endif
    DWORD dwResult = EXECUTE_FAIL;

    // don't test if LUN is valid; LUNs are deprecated

    DEBUGMSG(ZONE_COMMENT, (_T(
        "%s DPO = %u; BytChk = %u; RelAdr = %u\r\n"),
        pszFname,
        pbCommand[1] & (1 << 4), // DPO bit
        pbCommand[1] & (1 << 1), // BytChk bit
        pbCommand[1] & (1 << 0)  // RelAdr
        ));

    // fake the verification
    dwResult = EXECUTE_PASS;

    return dwResult;
}

// ----------------------------------------------------------------------------
// Function: STORE_Init
//     Initialize SCSI-2 direct-access device emulator
//
// Parameters:
//     pszContext - driver's configuration key
// ----------------------------------------------------------------------------

DWORD
STORE_Init(
    LPCTSTR pszActiveKey
    )
{
	UNREFERENCED_PARAMETER(pszActiveKey);
	return ERROR_SUCCESS;
}

// ----------------------------------------------------------------------------
// Function: STORE_Close
//     Close SCSI-2 direct-access device emulator; unitialize, close associated
//     store, and remount exposed partitions
//
// Parameters:
//     None
// ----------------------------------------------------------------------------

DWORD
STORE_Close(
    )
{
	return ERROR_SUCCESS;
}

// ----------------------------------------------------------------------------
// Function: STORE_IsCommandSupported
//     Determine whether a SCSI-2 command is supported by the SCSI-2
//     direct-access device emulator
//
// Parameters:
//     ptcCommand - command block wrapper
//     pfDataStageRequired - return whether a data stage is required, if
//                           command is supported
//     pdwDirection - return the direction of the command
//     pdwDataSize - return the size of the data stage
// ----------------------------------------------------------------------------

// Determine whether a SCSI-2 command is supported
BOOL
STORE_IsCommandSupported(
    PTRANSPORT_COMMAND ptcCommand,
    PBOOL pfDataStageRequired,
    PDWORD pdwDirection,
    PDWORD pdwDataSize
    )
{
#ifdef DEBUG // Remove-W4: Warning C4189 workaround
    SETFNAME(_T("STORE_IsCommandSupported"));
#endif
    FUNCTION_ENTER_MSG();

    BYTE bOpCode = 0;
    BOOL fResult = TRUE;

    PREFAST_DEBUGCHK(ptcCommand);
    DEBUGCHK(ptcCommand->CommandBlock);
    DEBUGCHK(pfDataStageRequired);
    DEBUGCHK(pdwDirection);
    DEBUGCHK(pdwDataSize);

    *pfDataStageRequired = FALSE;
    *pdwDirection = 0;
    *pdwDataSize = 0;
    
    // command/op code is byte 0
    bOpCode = *((PBYTE) ptcCommand->CommandBlock);

    //PUFI_CB pUfiCb = (PUFI_CB) ptcCommand->CommandBlock;

    DEBUGMSG(ZONE_COMMENT, (_T("%s command 0x%x\r\n"), pszFname, bOpCode));

    switch (bOpCode) {
        case SCSI_INQUIRY:
            DEBUGMSG(ZONE_COMMENT, (_T("%s INQUIRY\r\n"), pszFname));
            *pfDataStageRequired = TRUE;
            *pdwDirection = DATA_IN;
            *pdwDataSize = DATASIZE_INQUIRY;
            break;
        case SCSI_REQUEST_SENSE:
            DEBUGMSG(ZONE_COMMENT, (_T("%s REQUEST SENSE\r\n"), pszFname));
            *pfDataStageRequired = TRUE;
            *pdwDirection = DATA_IN;
            *pdwDataSize = DATASIZE_REQUEST_SENSE;
            break;	
        case SCSI_TEST_UNIT_READY:
            DEBUGMSG(ZONE_COMMENT, (_T("%s TEST UNIT READY\r\n"), pszFname));
            break;		
        case SCSI_PREVENT_ALLOW_MEDIUM_REMOVAL:
            DEBUGMSG(ZONE_COMMENT, (_T("%s PREVENT ALLOW MEDIUM REMOVAL\r\n"), pszFname));
            break;			
        case SCSI_MODE_SENSE6:
            DEBUGMSG(ZONE_COMMENT, (_T("%s MODE SENSE (6)\r\n"), pszFname));
            *pfDataStageRequired = TRUE;
            *pdwDirection = DATA_IN;
            //*pdwDataSize = DATASIZE_MODE_SENSE6;
            break;
       case SCSI_MODE_SELECT6:
            DEBUGMSG(ZONE_COMMENT, (_T("%s MODE SENSE (6)\r\n"), pszFname));
            *pfDataStageRequired = TRUE;
            *pdwDirection = DATA_OUT;
            break;			
        case SCSI_MODE_SENSE10:
            DEBUGMSG(ZONE_COMMENT, (_T("%s MODE SENSE (10)\r\n"), pszFname));
            *pfDataStageRequired = TRUE;
            *pdwDirection = DATA_IN;
            //*pdwDataSize = DATASIZE_MODE_SENSE10;
            break;
        case SCSI_READ10:
            DEBUGMSG(ZONE_COMMENT, (_T("%s READ (10)\r\n"), pszFname));
            *pfDataStageRequired = TRUE;
            *pdwDirection = DATA_IN;
            // transfer length is byte 7 + 8
            //*pdwDataSize  = 512;//ByteSwapUshort(pUfiCb->wTransferLength);
            //*pdwDataSize *= BytesPerSector();
            break;
        case SCSI_READ_CAPACITY:
            DEBUGMSG(ZONE_COMMENT, (_T("%s READ CAPACITY\r\n"), pszFname));
            *pfDataStageRequired = TRUE;
            *pdwDirection = DATA_IN;
            //*pdwDataSize = sizeof(READ_CAPACITY_DATA);
            break;
        case SCSI_READ_FORMAT_CAPACITY:
            DEBUGMSG(ZONE_COMMENT, (_T("%s READ FORMAT CAPACITY\r\n"), pszFname));
            *pfDataStageRequired = TRUE;
            *pdwDirection = DATA_IN;
            //*pdwDataSize = sizeof(READ_FORMAT_CAPACITY_DATA);// to read the value of read format capacity
            break;
        case SCSI_UTP: 
            DEBUGMSG(ZONE_COMMENT, (_T("%s SCSI_UTP\r\n"), pszFname));
			UtpMessagePreProc(ptcCommand, pfDataStageRequired, pdwDirection, pdwDataSize);
            break;			
        /*case SCSI_START_STOP:
            DEBUGMSG(ZONE_COMMENT, (_T("%s START STOP\r\n"), pszFname));
            break;
        case SCSI_WRITE10:
            DEBUGMSG(ZONE_COMMENT, (_T("%s WRITE (10)\r\n"), pszFname));
            *pfDataStageRequired = TRUE;
            *pdwDirection = DATA_OUT;
            *pdwDataSize  = ByteSwapUshort(pUfiCb->wTransferLength);
            *pdwDataSize *= BytesPerSector();
            break;

        case SCSI_VERIFY:
            DEBUGMSG(ZONE_COMMENT, (_T("%s VERIFY\r\n"), pszFname));
            break;*/

			// Unrecognized commands
        default:
            DEBUGMSG(ZONE_WARNING, (_T("%s unsupported command 0x%x\r\n"),
                pszFname, bOpCode));
            fResult = FALSE;
#ifdef QFE_MERGE /*080430*/ /*CE6QFE*/
            *pdwDirection = DATA_UNKNOWN;
#endif
            break;
    }

//EXIT:;
    FUNCTION_LEAVE_MSG();
    return fResult;
}

// ----------------------------------------------------------------------------
// Function: STORE_ExecuteCommand
//     Execute and/or emulate the specified command
//
// Parameters:
//     ptcCommand - command block wrapper
//     ptdData - data block wrapper
// ----------------------------------------------------------------------------

DWORD
STORE_ExecuteCommand(
    PTRANSPORT_COMMAND ptcCommand,
    PTRANSPORT_DATA ptdData
    )
{
#ifdef DEBUG // Remove-W4: Warning C4189 workaround
    SETFNAME(_T("STORE_ExecuteCommand"));
#endif
    FUNCTION_ENTER_MSG();

    DWORD dwResult = EXECUTE_FAIL;

    DEBUGCHK(ptcCommand);
    DEBUGCHK(ptcCommand->CommandBlock);

#ifdef DEBUG
    {
        BOOL  fDataStageRequired;
        DWORD dwDirection;
        DWORD dwDataSize;
        BOOL fSupported = STORE_IsCommandSupported(ptcCommand, &fDataStageRequired, &dwDirection, &dwDataSize);
        DEBUGCHK(fSupported);
        DEBUGCHK(!fDataStageRequired || (ptdData && ptdData->DataBlock));
    }
#endif

    // command/op code is byte 0
    BYTE bOpCode = ((PBYTE) ptcCommand->CommandBlock)[0];

    DEBUGMSG(ZONE_COMMENT, (_T("%s command 0x%x\r\n"), pszFname, bOpCode));

    switch (bOpCode) {
        case SCSI_INQUIRY:
            DEBUGMSG(ZONE_COMMENT, (_T("%s SCSI INQUIRY\r\n"), pszFname));
            dwResult = ProcessScsiInquiry(ptcCommand, ptdData);
            break;
        case SCSI_MODE_SENSE6:
            DEBUGMSG(ZONE_COMMENT, (_T("%s MODE SENSE (6)\r\n"), pszFname));
            dwResult = ProcessScsiModeSense6(ptcCommand, ptdData);
            break;
        case SCSI_MODE_SELECT6:
            DEBUGMSG(ZONE_COMMENT, (_T("%s MODE SENSE (6)\r\n"), pszFname));
            dwResult = ProcessScsiModeSelect6(ptcCommand, ptdData);
            break;			
        case SCSI_MODE_SENSE10:
            DEBUGMSG(ZONE_COMMENT, (_T("%s MODE SENSE (10)\r\n"), pszFname));
            dwResult = ProcessScsiModeSense10(ptcCommand, ptdData);
            break;
        case SCSI_REQUEST_SENSE:
            DEBUGMSG(ZONE_COMMENT, (_T("%s REQUEST SENSE\r\n"), pszFname));
            dwResult = ProcessScsiRequestSense(ptcCommand, ptdData);
            break;
        case SCSI_SEND_DIAGNOSTIC:
            DEBUGMSG(ZONE_COMMENT, (_T("%s SEND DIAGNOSTIC\r\n"), pszFname));
            dwResult = ProcessScsiSendDiagnostic(ptcCommand);
            break;
        case SCSI_TEST_UNIT_READY:
            DEBUGMSG(ZONE_COMMENT, (_T("%s TEST UNIT READY\r\n"), pszFname));
            dwResult = ProcessScsiTestUnitReady(ptcCommand);
            break;
        case SCSI_READ10:
            DEBUGMSG(ZONE_COMMENT, (_T("%s READ (10)\r\n"), pszFname));
            dwResult = ProcessScsiRead10(ptcCommand, ptdData);
            break;
        case SCSI_READ_CAPACITY:
            DEBUGMSG(ZONE_COMMENT, (_T("%s READ CAPACITY\r\n"), pszFname));
            dwResult = ProcessScsiReadCapacity(ptcCommand, ptdData);
            break;
        case SCSI_READ_FORMAT_CAPACITY:
            DEBUGMSG(ZONE_COMMENT, (_T("%s READ FORMAT CAPACITY\r\n"), pszFname));
            dwResult = ProcessScsiReadFormatCapacity(ptcCommand, ptdData);
            break;
        case SCSI_START_STOP:
            DEBUGMSG(ZONE_COMMENT, (_T("%s START STOP\r\n"), pszFname));
            dwResult = ProcessScsiStartStop(ptcCommand);
            break;
        case SCSI_WRITE10:
            DEBUGMSG(ZONE_COMMENT, (_T("%s WRITE (10)\r\n"), pszFname));
            dwResult = ProcessScsiWrite10(ptcCommand, ptdData);
            break;
        case SCSI_PREVENT_ALLOW_MEDIUM_REMOVAL:
            DEBUGMSG(ZONE_COMMENT, (_T("%s PREVENT ALLOW MEDIUM REMOVAL\r\n"), pszFname));
            dwResult = ProcessScsiPreventAllowMediumRemoval(ptcCommand);
            break;
        case SCSI_VERIFY:
            DEBUGMSG(ZONE_COMMENT, (_T("%s VERIFY\r\n"), pszFname));
            dwResult = ProcessScsiVerify(ptcCommand);
            break;
        case SCSI_UTP:
            DEBUGMSG(ZONE_COMMENT, (_T("%s SCSI_UTP\r\n"), pszFname));
            dwResult = UtpMessageHandler(ptcCommand, ptdData);
            break;			
        default:
            DEBUGCHK(FALSE);
            break;
    }

    if (dwResult != EXECUTE_PASS) {
        DEBUGMSG(ZONE_ERROR, (_T(
            "%s failed to execute command 0x%02x\r\n"
            ), pszFname, bOpCode));
    }


//EXIT:;

    FUNCTION_LEAVE_MSG();

    return dwResult;
}

#pragma optimize ( "", on )
