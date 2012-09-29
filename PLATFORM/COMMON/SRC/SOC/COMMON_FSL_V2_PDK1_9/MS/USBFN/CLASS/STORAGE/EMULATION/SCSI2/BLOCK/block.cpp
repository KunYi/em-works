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

/*++

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:

        block.cpp

Abstract:

        Block-based SCSI-2 direct-access device (hard disk drive) emulator.

--*/

//------------------------------------------------------------------------------
//
// Copyright (C) 2005-2010, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
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

#include "bufferpool.h"

BufferPool* g_pReadBufferPool = NULL;
BufferPool* g_pWriteBufferPool = NULL;

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
static const UCHAR g_bSenseKey = 0;
static const UCHAR g_bASC = 0;
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

#define RETRY_INTERVAL         500
#define MAX_RETRY_COUNT        3    // Storage may not be mounted yet in cold boot, so we use a delay sleep mechanism for retrying

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

extern DWORD  g_cbDataBuffer;

//global variable
BOOL   g_bAsyncTransfer = TRUE;

// ----------------------------------------------------------------------------
// Function: BytesPerSector
//     Return number of bytes per sector on exposed device
//
// Parameters:
//     None
// ----------------------------------------------------------------------------

//static
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
#ifdef DEBUG // Remove-W4: Warning C4189 workaround
    SETFNAME(_T("ProcessScsiModeSense6"));
#endif

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(ptcCommand);
    PBYTE pbData = (PBYTE) ptdData->DataBlock;
    DWORD dwResult = EXECUTE_FAIL;

    // don't test if LUN is valid; LUNs are deprecated

    // test if data block is large enough
    if (ptdData->RequestLength < DATASIZE_MODE_SENSE6) {
        DEBUGMSG(ZONE_ERROR, (_T(
            "%s host requesting less than required; ptdData->RequestLength < DATASIZE_MODE_SENSE6\r\n"
            ), pszFname));
#ifdef QFE_MERGE /*080430*/ /*CE6QFE*/
#else
        goto EXIT;
#endif
    }    

    ZeroMemory(pbData, ptdData->RequestLength);
    ptdData->TransferLength = ptdData->RequestLength;

    // mode data length
    pbData[0] = 0; // msb
    pbData[1] = 6; // lsb
    // medium type, 9.3.3
    pbData[2] = 0x00;  // default medium type
    // device-specific parameter, 9.3.3
    pbData[3] = 0x00;  // bit 7 = WP (write protected)

    dwResult = EXECUTE_PASS;

#ifdef QFE_MERGE /*080430*/ /*CE6QFE*/
#else
EXIT:;
#endif
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
    pSenseData->SenseKey = g_bSenseKey;
    pSenseData->AdditionalSenseLength = bAdditionalSenseLength;
    pSenseData->AdditionalSenseCode = g_bASC;
    pSenseData->AdditionalSenseCodeQualifier = g_bASCQ;

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
    dwResult = EXECUTE_PASS;
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

    PBYTE pbCommand = (PBYTE) ptcCommand->CommandBlock;
    PBYTE pbData = (PBYTE) ptdData->DataBlock;
    DWORD dwLogicalBlockAddress;
    DWORD dwTransferLength;
    DWORD dwResult = EXECUTE_FAIL;
     
    SG_REQ sgSgReq;
    SG_BUF sgSgBuf;
    DWORD dwBytesReturned;
    BOOL fResult = FALSE;

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

    if(!g_bAsyncTransfer)
    {
        ZeroMemory(pbData, (dwTransferLength * BytesPerSector()));
    }
    
    ptdData->TransferLength = dwTransferLength * BytesPerSector();

    // is the host reading the virtual disk's MBR?
    if ((dwLogicalBlockAddress == 0) && (g_lpbMBR != NULL)) {
        // we can't handle a request to access {virtual sector 0, physical
        // sector 1, physical sector2, ...)
        DEBUGCHK(dwTransferLength == 1);
        DEBUGMSG(ZONE_COMMENT, (_T(
            "%s read request targeting virtual disk's MBR\r\n"
            ), pszFname));
        // return virtual disk's MBR
        ptdData->DataBlock = g_lpbMBR;
        dwResult = EXECUTE_PASS;
        goto EXIT;
    }

    if(g_bAsyncTransfer)
    {
        ptdData->DataBlock = g_pReadBufferPool->GetBuffer(dwLogicalBlockAddress, dwTransferLength);
    
        if(ptdData->DataBlock != NULL)
        {
            //RETAILMSG(1,(TEXT("g_pReadBufferPool->GetBuffer successful\r\n")));
        }
        else
        {
            RETAILMSG(1,(TEXT("g_pReadBufferPool->GetBuffer failed")));
            goto EXIT;
        }
    }
    else
    {
        // prepare scatter/gather buffer
        sgSgBuf.sb_buf = pbData;
        sgSgBuf.sb_len = ptdData->TransferLength;
    
        // prepare scatter/gather request
        sgSgReq.sr_start = dwLogicalBlockAddress;
        sgSgReq.sr_num_sec = dwTransferLength;
        sgSgReq.sr_status = 0;
        sgSgReq.sr_callback = NULL;
        sgSgReq.sr_num_sg = 1;
        sgSgReq.sr_sglist[0] = sgSgBuf;
    
        // read from device
        DWORD dwIoControlCode = (g_fLegacyBlockDriver) ? DISK_IOCTL_READ : IOCTL_DISK_READ;
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
                "%s IOCTL_DISK_READ passed; %u bytes read\r\n"
                ), pszFname, dwBytesReturned));
        }
        else {
#ifdef DEBUG // Remove-W4: Warning C4189 workaround
            DWORD dwError = GetLastError();
#endif
            DEBUGMSG(ZONE_ERROR, (_T(
                "%s IOCTL_DISK_READ failed; error = %u\r\n"
                ), pszFname, dwError));
            goto EXIT;
        }
    }

    dwResult = EXECUTE_PASS;

EXIT:;
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

    // don't test if LUN is valid; LUNs are deprecated

    // test if data block is large enough
    if (ptdData->RequestLength < sizeof(READ_CAPACITY_DATA)) {
        DEBUGMSG(ZONE_ERROR, (_T(
            "%s host requesting less than required; ptdData->RequestLength < %u\r\n"
            ), pszFname, sizeof(READ_CAPACITY_DATA)));
        goto EXIT;
    }

    ZeroMemory(pbData, ptdData->RequestLength);
    ptdData->TransferLength = ptdData->RequestLength;

    // pack logical block address
    PREAD_CAPACITY_DATA pReadCapacityData;
    pReadCapacityData = (PREAD_CAPACITY_DATA) pbData;
    // -1 to return last addressable
    pReadCapacityData->LastLogicalBlockAddress = ByteSwapUlong(g_diDiskInfo.di_total_sectors - 1);

    // pack block length in bytes
    pReadCapacityData->BlockLength = ByteSwapUlong(BytesPerSector());

    dwResult = EXECUTE_PASS;

EXIT:;
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
    PBYTE pbCommand = (PBYTE) ptcCommand->CommandBlock;
    PBYTE pbData = (PBYTE) ptdData->DataBlock;
    DWORD dwResult = EXECUTE_FAIL;

    if(pbCommand){} //L4 Warning removal

    // don't test if LUN is valid; LUNs are deprecated

    // test if data block is large enough
    if (ptdData->RequestLength < sizeof(READ_FORMAT_CAPACITY_DATA)) {
        DEBUGMSG(ZONE_ERROR, (_T(
            "%s host requesting less than required; ptdData->RequestLength < %u\r\n"
            ), pszFname, sizeof(READ_FORMAT_CAPACITY_DATA)));
        goto EXIT;
    }

    ZeroMemory(pbData, ptdData->RequestLength);
    ptdData->TransferLength = ptdData->RequestLength;

    // pack logical block address
    PREAD_FORMAT_CAPACITY_DATA pReadCapacityData;
    pReadCapacityData = (PREAD_FORMAT_CAPACITY_DATA) pbData;
    // -1 to return last addressable
    pReadCapacityData->LastLogicalBlockAddress = ByteSwapUlong(g_diDiskInfo.di_total_sectors - 1);

    // pack block length in bytes
    pReadCapacityData->BlockLength = ByteSwapUlong(BytesPerSector());

    dwResult = EXECUTE_PASS;

EXIT:;
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


    if(g_bAsyncTransfer)
    {
        g_pWriteBufferPool->ReturnBuffer(dwLogicalBlockAddress, dwTransferLength, pbData);
    }
    else
    {

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

void STORE_ReturnReadBuffer(PVOID pbData, DWORD dwTransferLength)
{
    g_pReadBufferPool->ReturnBuffer(0,dwTransferLength,pbData);
}

PVOID STORE_GetWriteBuffer(DWORD dwLength)
{
    return g_pWriteBufferPool->GetBuffer(0,dwLength);
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
#ifdef DEBUG // Remove-W4: Warning C4189 workaround
    SETFNAME(_T("STORE_Init"));
#endif

    HKEY hKey = NULL;
    DWORD cbData;
    DWORD dwError;
    DWORD dwType;
    BOOL fResult;
    TCHAR szSpecifiedDev[30] = {0};
    BOOL bUseCodedPriority = FALSE;   // if there is pre-defined DSK name in registry, use that
                                      // otherwise use pre-coded priority to find proper store
                                      // ATA > SD (MMC) > USB > NAND
    DWORD dwRetryCnt;
    BOOL bOK;
    DWORD dwBytesReturned;
    STOREINFO storeInfo;
    TCHAR compString[30];
    
    HANDLE hFindPartition = INVALID_HANDLE_VALUE;
    PARTINFO partInfo = {0};

#ifdef MSCDSK
    BOOL bSDDisk = FALSE, bUSBDisk = FALSE;
    HANDLE hSDDisk = NULL, hUSBDisk = NULL;
    BOOL bHandleCloseSD = FALSE, bHandleCloseUSB = FALSE;

    hSDDisk = CreateEvent(NULL, FALSE, FALSE, L"MSC_SDDISK");
    if (GetLastError() == ERROR_ALREADY_EXISTS) bHandleCloseUSB = TRUE;
    hUSBDisk = CreateEvent(NULL, FALSE, FALSE, L"MSC_USBDISK");
    if (GetLastError() == ERROR_ALREADY_EXISTS) bHandleCloseSD = TRUE;

    ResetEvent(hSDDisk);
    ResetEvent(hUSBDisk);
#endif

    FUNCTION_ENTER_MSG();

#ifdef ASYNC_TRANSFER
    g_bAsyncTransfer = TRUE;
#else
    g_bAsyncTransfer = FALSE;
#endif
    
    // mark self as uninitialized
    g_fInitialized = FALSE;

    // open the client driver key
    hKey = OpenDeviceKey(pszActiveKey);
    if (hKey == NULL) {
        dwError = GetLastError();
        DEBUGMSG(ZONE_ERROR, (_T(
            "%s Failed to open device key for \"%s\"; Error = %u\r\n"
            ), pszFname, pszActiveKey, dwError));
        goto EXIT;
    }

    // read name of store to expose
    cbData = sizeof(szSpecifiedDev);
    dwError = RegQueryValueEx(hKey, PRX_DEVICE_NAME_VAL, NULL, &dwType, (PBYTE)szSpecifiedDev, &cbData);
    if ((dwError != ERROR_SUCCESS) || (dwType != REG_SZ)) {
        DEBUGMSG(ZONE_ERROR, (_T(
            "%s Failed to read %s; error = %u\r\n"
            ), pszFname, PRX_DEVICE_NAME_VAL, dwError));
        // goto EXIT;
        bUseCodedPriority = TRUE;
    }

    // read removable media flag; default is true if flag not present
    cbData = sizeof(g_dwRemovable);
    dwError = RegQueryValueEx(hKey, PRX_RMB_VAL, NULL, &dwType, (PBYTE) &g_dwRemovable, &cbData);
    if ( (dwError != ERROR_SUCCESS) || (dwType != REG_DWORD) ) {
        DEBUGMSG(ZONE_WARNING, (_T(
            "%s %s not present; default is true\r\n"
            ), pszFname, PRX_RMB_VAL, dwError));
    }
    DEBUGMSG(ZONE_INIT, (_T(
        "%s %s = %u\r\n"
        ), pszFname, PRX_RMB_VAL, g_dwRemovable));

    if (bUseCodedPriority)
    {
        dwRetryCnt = 0;
        for (;;)
        {
            //enumerate all disk
            storeInfo.cbSize = sizeof(STOREINFO);
            
            //first find ata
            StringCchPrintf(compString, sizeof(compString)/sizeof(compString[0]), (L"ATA HARD DISK"));
            g_hStore = FindFirstStore(&storeInfo);
            if (!_stricmp((const char*)storeInfo.szStoreName, (const char*)compString))
            {
                goto FINDSTORE;
            }
            for (;;)
            {
                bOK = FindNextStore(g_hStore, &storeInfo);
                if (!bOK)
                {
                    break;
                }
                else
                {
                    if (!_stricmp((const char*)storeInfo.szStoreName, (const char*)compString))
                    {
                        goto FINDSTORE;
                    }
                }
            }
#ifdef MSCDSK
            bSDDisk = TRUE;
#endif
            StringCchPrintf(compString, sizeof(compString)/sizeof(compString[0]), (L"SDMEMORY CARD"));
            g_hStore = FindFirstStore(&storeInfo);
            if (!_stricmp((const char*)storeInfo.szStoreName, (const char*)compString))
            {
                goto FINDSTORE;
            }
            for (;;)
            {
                bOK = FindNextStore(g_hStore, &storeInfo);
                if (!bOK)
                {
                    break;
                }
                else
                {
                    if (!_stricmp((const char*)storeInfo.szStoreName, (const char*)compString))
                    {
                        goto FINDSTORE;
                    }
                }
            }

            StringCchPrintf(compString, sizeof(compString)/sizeof(compString[0]), (L"MMC CARD"));
            g_hStore = FindFirstStore(&storeInfo);
            if (!_stricmp((const char*)storeInfo.szStoreName, (const char*)compString))
            {
                goto FINDSTORE;
            }
            for (;;)
            {
                bOK = FindNextStore(g_hStore, &storeInfo);
                if (!bOK)
                {
                    break;
                }
                else
                {
                    if (!_stricmp((const char*)storeInfo.szStoreName, (const char*)compString))
                    {
                        goto FINDSTORE;
                    }
                }
            }
#ifdef MSCDSK
            bSDDisk = FALSE;
#endif

#ifdef MSCDSK
            bUSBDisk = TRUE;
#endif
            StringCchPrintf(compString, sizeof(compString)/sizeof(compString[0]), (L"USB HARD DISK"));
            g_hStore = FindFirstStore(&storeInfo);
            if (!_stricmp((const char*)storeInfo.szStoreName, (const char*)compString))
            {
                goto FINDSTORE;
            }
            for (;;)
            {
                bOK = FindNextStore(g_hStore, &storeInfo);
                if (!bOK)
                {
                    break;
                }
                else
                {
                    if (!_stricmp((const char*)storeInfo.szStoreName, (const char*)compString))
                    {
                        goto FINDSTORE;
                    }
                }
            }
#ifdef MSCDSK
            bUSBDisk = FALSE;
#endif

            StringCchPrintf(compString, sizeof(compString)/sizeof(compString[0]), (L"NAND FLASH"));
            g_hStore = FindFirstStore(&storeInfo);
            if (!_stricmp((const char*)storeInfo.szStoreName, (const char*)compString))
            {
                goto FINDSTORE;
            }
            for (;;)
            {
                bOK = FindNextStore(g_hStore, &storeInfo);
                if (!bOK)
                {
                    break;
                }
                else
                {
                    if (!_stricmp((const char*)storeInfo.szStoreName, (const char*)compString))
                    {
                        goto FINDSTORE;
                    }
                }
            }

            // don't find store in this round, sleep a while and retry
            Sleep(RETRY_INTERVAL);
            if (dwRetryCnt++ > MAX_RETRY_COUNT) break;
        }

        RETAILMSG(1, (L"No available mass storage medium\r\n"));
        goto EXIT;
    }
    else
    {
        dwRetryCnt = 0;
        for (;;)
        {
            // Use what is specified in Registry
            storeInfo.cbSize = sizeof(STOREINFO);
#ifdef MSCDSK
            bUSBDisk = FALSE;
            bSDDisk = FALSE;
#endif

            StringCchPrintf(compString, sizeof(compString)/sizeof(compString[0]), szSpecifiedDev);
            g_hStore = FindFirstStore(&storeInfo);
            if (!_stricmp((const char*)storeInfo.szStoreName, (const char*)compString))
            {
#ifdef MSCDSK
                if (!_stricmp((const char*)szSpecifiedDev, "USB HARD DISK"))
                {
                    bUSBDisk = TRUE;
                }
                if (!_stricmp((const char*)szSpecifiedDev, "SDMEMORY CARD") || _stricmp((const char*)szSpecifiedDev, "MMC CARD"))
                {
                    bSDDisk = TRUE;
                }
#endif
                goto FINDSTORE;
            }
            for (;;)
            {
                bOK = FindNextStore(g_hStore, &storeInfo);
                if (!bOK)
                {
                    break;
                }
                else
                {
                    if (!_stricmp((const char*)storeInfo.szStoreName, (const char*)compString))
                    {
#ifdef MSCDSK
                        if (!_stricmp((const char*)szSpecifiedDev, "USB HARD DISK"))
                        {
                            bUSBDisk = TRUE;
                        }
                        if (!_stricmp((const char*)szSpecifiedDev, "SDMEMORY CARD") || _stricmp((const char*)szSpecifiedDev, "MMC CARD"))
                        {
                            bSDDisk = TRUE;
                        }
#endif
                        goto FINDSTORE;
                    }
                }
            }

            // don't find store in this round, sleep a while and retry
            Sleep(RETRY_INTERVAL);
            if (dwRetryCnt++ > MAX_RETRY_COUNT) break;
        }

        RETAILMSG(1, (L"specified mass storage medium not available\r\n"));
        goto EXIT;
    }

    // open store
FINDSTORE:
    RETAILMSG(1, (L"%s as mass storage medium\r\n", storeInfo.szStoreName));
    StringCchPrintf(compString, sizeof(compString)/sizeof(compString[0]), (L"NAND FLASH"));
    if (!_stricmp((const char*)storeInfo.szStoreName, (const char*)compString))
    {
        RETAILMSG(1,(TEXT("Nand Flash act as mass storage, performance optimization can't be applied\r\n")));
        g_bAsyncTransfer = FALSE;
    }
    g_hStore = OpenStore(storeInfo.szDeviceName);
    StringCchCopy(g_szDeviceName, dim(g_szDeviceName), storeInfo.szDeviceName);
    g_szDeviceName[dim(g_szDeviceName) - 1] = 0;

    if ((g_hStore == NULL) || (g_hStore == INVALID_HANDLE_VALUE)) {
        dwError = GetLastError();
        DEBUGMSG(ZONE_ERROR, (_T(
            "%s failed to open store %s; error = %u\r\n"
            ), pszFname, g_szDeviceName, dwError));
        goto EXIT;
    }
    DEBUGMSG(ZONE_COMMENT, (_T("%s opened store %s\r\n"), pszFname, g_szDeviceName));

    
    
    GetStoreInfo(g_hStore, &storeInfo);

    StringCchPrintf(compString, sizeof(compString)/sizeof(compString[0]), (L"NAND FLASH"));
    if (!_stricmp((const char*)storeInfo.szStoreName, (const char*)compString))
    {
        partInfo.cbSize = sizeof(PARTINFO);
        
        // Find the first partition in the store
        hFindPartition = FindFirstPartition(g_hStore, &partInfo);
        if (hFindPartition == INVALID_HANDLE_VALUE) {
            dwError = GetLastError();
            DEBUGMSG(ZONE_ERROR, (_T(
                "%s failed to FindFirstPartition in store %s; error = %u\r\n"
                        ), pszFname, g_szDeviceName, dwError));
            goto EXIT;        
        }
        
        FindClosePartition(hFindPartition);

        g_hStore1 = g_hStore;
        g_hStore = OpenPartition(g_hStore1, partInfo.szPartitionName);
        if (g_hStore == INVALID_HANDLE_VALUE) {
            dwError = GetLastError();
            DEBUGMSG(ZONE_ERROR, (_T(
                "%s failed to OpenPartition %s in store %s; error = %u\r\n"
                ), pszFname, partInfo.szPartitionName, g_szDeviceName, dwError));
            goto EXIT;        
        }
        
        DEBUGMSG(ZONE_COMMENT, (_T("%s opened partition \"%s\" in store \"%s\"\r\n"), 
            pszFname, partInfo.szPartitionName, g_szDeviceName));

    }

    // read disk information
    fResult = DeviceIoControl(
        g_hStore,
        IOCTL_DISK_GETINFO,
        NULL,
        0,
        &g_diDiskInfo,
        sizeof(g_diDiskInfo),
        &dwBytesReturned,
        NULL);
    
    if (fResult) {
        DEBUGMSG(ZONE_INIT, (_T("%s IOCTL_DISK_GETINFO passed\r\n"), pszFname));
        DUMP_DISKINFO(g_diDiskInfo);
    }
    else {

        dwError = GetLastError();
        DEBUGMSG(ZONE_ERROR, (_T(
            "%s IOCTL_DISK_GETINFO failed; error = %u\r\n"
            ), pszFname, dwError));

        // is this a legacy block driver?
        fResult = DeviceIoControl(
            g_hStore,
            DISK_IOCTL_GETINFO,
            &g_diDiskInfo,
            sizeof(g_diDiskInfo),
            NULL,
            0,
            &dwBytesReturned,
            NULL);
        if (fResult) {
            DEBUGMSG(ZONE_INIT, (_T("%s DISK_IOCTL_GETINFO passed\r\n"), pszFname));
            DUMP_DISKINFO(g_diDiskInfo);
            g_fLegacyBlockDriver = TRUE; // legacy block driver
        }
        else {
            dwError = GetLastError();
            DEBUGMSG(ZONE_ERROR, (_T(
                "%s DISK_IOCTL_GETINFO failed; error = %u\r\n"
                ), pszFname, dwError));
            goto EXIT;
        }
    }

    g_pReadBufferPool = new ReadBufferPool(g_cbDataBuffer, g_fLegacyBlockDriver, g_hStore);
    if(g_pReadBufferPool == NULL)
    {
        RETAILMSG(1,(TEXT("g_pReadBufferPool NULL error\r\n")));
        goto EXIT;
    }
    g_pWriteBufferPool = new WriteBufferPool(g_cbDataBuffer, g_fLegacyBlockDriver, g_hStore);
    if(g_pWriteBufferPool == NULL)
    {
        RETAILMSG(1,(TEXT("g_pWriteBufferPool NULL error\r\n")));
        goto EXIT;
    }


    // selective partition exposure is not supported; default is to expose all partitions
#ifdef MSCDSK
    if (bSDDisk)
    {
        SetEvent(hSDDisk);
    }
    else if (bUSBDisk)
    {
        SetEvent(hUSBDisk);
    }
#endif

    // dismount entire store (all partitions)
    if (!DismountStore(g_hStore)) {
        dwError = GetLastError();
        goto EXIT;
    }

    g_bmPartitions = 0x0F;
    g_fInitialized = TRUE;
    dwError = ERROR_SUCCESS;

EXIT:;
#ifdef MSCDSK
    if (hSDDisk && bHandleCloseSD) CloseHandle(hSDDisk);
    if (hUSBDisk && bHandleCloseUSB) CloseHandle(hUSBDisk);
#endif
    if (!g_fInitialized) {
        STORE_Close();
    }
    CLOSE_KEY(hKey);
    FUNCTION_LEAVE_MSG();
    return dwError;
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
#ifdef DEBUG // Remove-W4: Warning C4189 workaround
    SETFNAME(_T("STORE_Close"));
#endif
    FUNCTION_ENTER_MSG();

    DWORD dwRet = ERROR_SUCCESS;
    TCHAR szFullDeviceName[32] = {
        '\\', 'S', 't', 'o', 'r', 'e', 'M', 'g', 'r', '\\'
        };
    ULONG ulFullDeviceNameIndex = 10;

#ifdef MSCDSK
    HANDLE hMscDisconnectEvent = NULL;
    BOOL bHandleCloseDetach = FALSE;
    hMscDisconnectEvent = CreateEvent(NULL, TRUE, FALSE, L"MSC_DETACH_EVENT");
    if (GetLastError() == ERROR_ALREADY_EXISTS) bHandleCloseDetach = TRUE;

    DWORD dwErr = 0;
    dwErr = WaitForSingleObject(hMscDisconnectEvent, 0);
    if (dwErr != WAIT_TIMEOUT)
    {
        ResetEvent(hMscDisconnectEvent);
        goto MEDIUM_REMOVED;
    }
#endif

    if ((g_hStore != NULL) && (g_hStore != INVALID_HANDLE_VALUE)) {

        // a virtual MBR does not exist
        
        if ((g_hStore1 != NULL) && (g_hStore1 != INVALID_HANDLE_VALUE)) {
            DismountStore(g_hStore1);
            CLOSE_HANDLE(g_hStore1);
            g_hStore1 = INVALID_HANDLE_VALUE;
            
            CLOSE_HANDLE(g_hStore);
            g_hStore = INVALID_HANDLE_VALUE;
        }
        else {
            // dismount store; this isn't strictly necessary, as the store should
            // already be dismounted
            DismountStore(g_hStore);
            CLOSE_HANDLE(g_hStore);
            g_hStore = NULL;
        }

        
    // append g_szDeviceName to "\\StoreMgr\\" to get full device name
        ULONG ulDeviceNameChars = _tcslen(g_szDeviceName);
        DEBUGCHK(ulDeviceNameChars <= PRX_STREAM_NAME_LEN);
        for (ULONG ul = 0; ul < ulDeviceNameChars; ul += 1) {
            szFullDeviceName[ulFullDeviceNameIndex + ul] = g_szDeviceName[ul];
        }

        // force re-examination of disk's organization
        DEBUGMSG(ZONE_COMMENT, (_T(
            "%s forcing storage manager to re-examine %s\r\n"
            ), pszFname, szFullDeviceName));
        if (!MoveFile(szFullDeviceName, szFullDeviceName)) {
            DEBUGMSG(ZONE_COMMENT, (_T(
                "%s failed to force storage manager to re-examine %s\r\n"
                ), pszFname, szFullDeviceName));
        }

    }

#ifdef MSCDSK
MEDIUM_REMOVED:
    {   
        // Reset MSC_SDDISK and MSC_USBDISK
        HANDLE hSDDisk = NULL, hUSBDisk = NULL;
        BOOL bHandleCloseSD = FALSE, bHandleCloseUSB = FALSE;

        hSDDisk = CreateEvent(NULL, FALSE, FALSE, L"MSC_SDDISK");
        if (GetLastError() == ERROR_ALREADY_EXISTS) bHandleCloseUSB = TRUE;
        hUSBDisk = CreateEvent(NULL, FALSE, FALSE, L"MSC_USBDISK");
        if (GetLastError() == ERROR_ALREADY_EXISTS) bHandleCloseSD = TRUE;

        ResetEvent(hSDDisk);
        ResetEvent(hUSBDisk);
        if (hSDDisk && bHandleCloseSD) CloseHandle(hSDDisk);
        if (hUSBDisk && bHandleCloseUSB) CloseHandle(hUSBDisk);
    }
    if (hMscDisconnectEvent && bHandleCloseDetach) CloseHandle(hMscDisconnectEvent);
#endif
    // clean up emulator

    // deallocate list of partition names
    LOCAL_FREE(g_ptcPartitions);
    g_ptcPartitions = NULL;

    // deallocate virtual disk's MBR
    LOCAL_FREE(g_lpbMBR);
    g_lpbMBR = NULL;

    // deallocate copy of actual disk's MBR
    LOCAL_FREE(g_lpbPhysMBR);
    g_lpbPhysMBR = NULL;

    g_bmPartitions = 0;
    g_fInitialized = FALSE;
    g_fLegacyBlockDriver = FALSE;

    if(g_pReadBufferPool != NULL)
    {
        delete g_pReadBufferPool;
        g_pReadBufferPool = NULL;
    }

    if(g_pWriteBufferPool != NULL)
    {
        delete g_pWriteBufferPool;
        g_pWriteBufferPool = NULL;
    }

    FUNCTION_LEAVE_MSG();

    return dwRet;
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

    if ((g_hStore == NULL) || (!g_fInitialized)) {
        DEBUGMSG(ZONE_ERROR, (_T(
            "%s emulator not open; commands not accepted\r\n"
            ), pszFname));
        fResult = FALSE;
        goto EXIT;
    }

    *pfDataStageRequired = FALSE;
    *pdwDirection = 0;
    *pdwDataSize = 0;
    
    // command/op code is byte 0
    bOpCode = *((PBYTE) ptcCommand->CommandBlock);

    PUFI_CB pUfiCb = (PUFI_CB) ptcCommand->CommandBlock;

    DEBUGMSG(ZONE_COMMENT, (_T("%s command 0x%x\r\n"), pszFname, bOpCode));

    switch (bOpCode) {
        case SCSI_INQUIRY:
            DEBUGMSG(ZONE_COMMENT, (_T("%s INQUIRY\r\n"), pszFname));
            *pfDataStageRequired = TRUE;
            *pdwDirection = DATA_IN;
            *pdwDataSize = DATASIZE_INQUIRY;
            break;
        case SCSI_MODE_SENSE6:
            DEBUGMSG(ZONE_COMMENT, (_T("%s MODE SENSE (6)\r\n"), pszFname));
            *pfDataStageRequired = TRUE;
            *pdwDirection = DATA_IN;
            *pdwDataSize = DATASIZE_MODE_SENSE6;
            break;
        case SCSI_MODE_SENSE10:
            DEBUGMSG(ZONE_COMMENT, (_T("%s MODE SENSE (10)\r\n"), pszFname));
            *pfDataStageRequired = TRUE;
            *pdwDirection = DATA_IN;
            *pdwDataSize = DATASIZE_MODE_SENSE10;
            break;
        case SCSI_REQUEST_SENSE:
            DEBUGMSG(ZONE_COMMENT, (_T("%s REQUEST SENSE\r\n"), pszFname));
            *pfDataStageRequired = TRUE;
            *pdwDirection = DATA_IN;
            *pdwDataSize = DATASIZE_REQUEST_SENSE;
            break;
        case SCSI_SEND_DIAGNOSTIC:
            DEBUGMSG(ZONE_COMMENT, (_T("%s SEND DIAGNOSTIC\r\n"), pszFname));
            break;
        case SCSI_TEST_UNIT_READY:
            DEBUGMSG(ZONE_COMMENT, (_T("%s TEST UNIT READY\r\n"), pszFname));
            break;
        case SCSI_READ10:
            DEBUGMSG(ZONE_COMMENT, (_T("%s READ (10)\r\n"), pszFname));
            *pfDataStageRequired = TRUE;
            *pdwDirection = DATA_IN;
            // transfer length is byte 7 + 8
            *pdwDataSize  = ByteSwapUshort(pUfiCb->wTransferLength);
            *pdwDataSize *= BytesPerSector();
            break;
        case SCSI_READ_CAPACITY:
            DEBUGMSG(ZONE_COMMENT, (_T("%s READ CAPACITY\r\n"), pszFname));
            *pfDataStageRequired = TRUE;
            *pdwDirection = DATA_IN;
            *pdwDataSize = sizeof(READ_CAPACITY_DATA);
            break;
        case SCSI_READ_FORMAT_CAPACITY:
            DEBUGMSG(ZONE_COMMENT, (_T("%s READ FORMAT CAPACITY\r\n"), pszFname));
            *pfDataStageRequired = TRUE;
            *pdwDirection = DATA_IN;
            *pdwDataSize = sizeof(READ_FORMAT_CAPACITY_DATA);// to read the value of read format capacity
            break;
        case SCSI_START_STOP:
            DEBUGMSG(ZONE_COMMENT, (_T("%s START STOP\r\n"), pszFname));
            break;
        case SCSI_WRITE10:
            DEBUGMSG(ZONE_COMMENT, (_T("%s WRITE (10)\r\n"), pszFname));
            *pfDataStageRequired = TRUE;
            *pdwDirection = DATA_OUT;
            *pdwDataSize  = ByteSwapUshort(pUfiCb->wTransferLength);
            *pdwDataSize *= BytesPerSector();
            break;
        case SCSI_PREVENT_ALLOW_MEDIUM_REMOVAL:
            DEBUGMSG(ZONE_COMMENT, (_T("%s PREVENT ALLOW MEDIUM REMOVAL\r\n"), pszFname));
            break;
        case SCSI_VERIFY:
            DEBUGMSG(ZONE_COMMENT, (_T("%s VERIFY\r\n"), pszFname));
            break;

#ifdef QFE_MERGE /*080430*/ /*CE6QFE*/
        // List of Recognized commands not supported by the USB Mass Function WinCE driver
        case SCSI_MODE_SELECT6:
            fResult = FALSE;
            *pdwDirection = DATA_IN;
            break;
#endif

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

EXIT:;
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

    if ((g_hStore == NULL) || (!g_fInitialized)) {
        DEBUGMSG(ZONE_ERROR, (_T(
            "%s emulator not open; commands not accepted\r\n"
            ), pszFname));
        dwResult = EXECUTE_FAIL;
        goto EXIT;
    }

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
        default:
            DEBUGCHK(FALSE);
            break;
    }

    if (dwResult != EXECUTE_PASS) {
        DEBUGMSG(ZONE_ERROR, (_T(
            "%s failed to execute command 0x%02x\r\n"
            ), pszFname, bOpCode));
    }


EXIT:;

    FUNCTION_LEAVE_MSG();

    return dwResult;
}

#pragma optimize ( "", on )
