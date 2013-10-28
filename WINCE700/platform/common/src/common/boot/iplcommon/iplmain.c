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
#include <windows.h>
#include <pehdr.h>
#include <romldr.h>
#include <bootpart.h>
#include <iplcommon.h>
#include <fmd.h>
#include <cecompress.h>
#include <fallite.h>

// the minimum data we need from the first page of a partition
#define PARTITION_HEADER_BYTES     ROM_TOC_OFFSET_OFFSET + sizeof(DWORD)

// Uldr update mode (from updateapp.h)
#define ULDR_UPDATE_MODE            0x00000002
#define UPDATE_MODE                 0x00000001

// Globals.
//
// The IPL's table of contents pointer.
ROMHDR * volatile const pTOC = (ROMHDR *)-1;
// Message handler callback function pointer.
PFN_MessageHandler g_pfnMessageHandler = NULL;
// Compressed Partition flag
BOOL g_fCompressed = FALSE;
// Compression Decoder Stream
CeCompressDecodeStream g_DecodeStream = NULL;
// Compression Header
PCOMPRESSED_RAMIMAGE_HEADER g_CompressionHeader = NULL;
// buffer for reading compression data
BYTE CompressedData[RAMIMAGE_COMPRESSION_BLOCK_SIZE];

// External variable definitions.
//
extern UINT32 g_ulFlashBase;          // Flash start and length.
extern UINT32 g_ulFlashLengthBytes;

extern UINT32 g_ulBPartBase;          // BootPart work buffer start and length.
extern UINT32 g_ulBPartLengthBytes;

// Functions used for compression
BOOL InitDecompressor(HANDLE hPartition, DWORD dwHeaderSize);
VOID DeInitDecompressor();
LPVOID MyAlloc(LPVOID pIgnored, DWORD dwAllocSize);
VOID MyDeAlloc(LPVOID pIgnored, LPVOID pAddress);
BOOL CopyToRAM(HANDLE hPartition, DWORD dwPartitionBase);

// Local variables.
//
static UINT16 g_wSectorSize;
static void *g_pMessageTable[] = 
{
    TEXT("ERROR: Unable to find IPL table of contents.\0"),              // IPL_ERROR_BAD_IPLTOC
    TEXT("ERROR: Failed to initialize bootpart library.\0"),             // IPL_ERROR_BPINIT_FAILED
    TEXT("ERROR: Failed to open storage partition.\0"),                  // IPL_ERROR_OPENPARTITION_FAILED
    TEXT("ERROR: Failed to get storage type information.\0"),            // IPL_ERROR_GETSTRORAGETYPE_FAILED
    TEXT("ERROR: Failed to retrieve stored image information.\0"),       // IPL_ERROR_GETIMAGEINFO_FAILED
    TEXT("ERROR: Failed to translate image address.\0"),                 // IPL_ERROR_TRANSLATEADDR_FAILED
    TEXT("ERROR: Failed to read image data from storage partition.\0"),  // IPL_ERROR_READDATA_FAILED
    TEXT("ERROR: Failed to jump to loaded image.\0"),                    // IPL_ERROR_JUMP_FAILED
    TEXT("INFO: Loading image ...\r\n\0"),                               // IPL_INFO_LOADING_IMAGE
    TEXT("INFO: Jumping to image...\r\n\0"),                             // IPL_INFO_JUMPING_IMAGE
    TEXT("INFO: Loading backup ULDR ...\r\n\0")                          // IPL_INFO_LOADING_BACKUP_ULDR
};


/*
    @func   void | SpinForever | Halts execution.
    @rdesc  None.
    @comm    
    @xref   
*/
static void SpinForever(void)
{
    for(;;)
    {
        // Do nothing.
    }
}


/*
    @func   void | MessageDispatch | Dispatches debug messages to the OEM message handler.
    @rdesc  None.
    @comm    
    @xref   
*/
static void MessageDispatch(IPL_MESSAGE_CODE MessageCode)
{
    // Dispatch the message to the OEM-provided message handler (if there is one).
    //
    if (g_pfnMessageHandler)
    {
        // Make sure we don't overstep the array boundary.
        //
        if (MessageCode >= (_countof(g_pMessageTable)))
        {
            return;
        }

        // Call the OEM's messange handler routine.
        //
        g_pfnMessageHandler(MessageCode, g_pMessageTable[MessageCode]);
    }

}


/*
    @func   BOOLEAN | SetupCopySection | Copies the IPL image's copy section data (initialized globals) to the correct fix-up location.  Once completed, the IPLs initialized globals are valid.
    @rdesc  TRUE == Success and FALSE == Failure.
    @comm    
    @xref   
*/
static BOOLEAN SetupCopySection(ROMHDR *const pTOC)
{
    UINT32 ulLoop;
    COPYentry *pCopyEntry;

    if (pTOC == (ROMHDR *const) -1)
    {
        return(FALSE);
    }

    // This is where the data sections become valid... don't read globals until after this
    //
    for (ulLoop = 0; ulLoop < pTOC->ulCopyEntries; ulLoop++)
    {
        pCopyEntry = (COPYentry *)(pTOC->ulCopyOffset + ulLoop*sizeof(COPYentry));
        if (pCopyEntry->ulCopyLen)
        {
            memcpy((LPVOID)pCopyEntry->ulDest, (LPVOID)pCopyEntry->ulSource, pCopyEntry->ulCopyLen);
        }
        if (pCopyEntry->ulCopyLen < pCopyEntry->ulDestLen)
        {
            memset((LPVOID)(pCopyEntry->ulDest+pCopyEntry->ulCopyLen), 0, pCopyEntry->ulDestLen-pCopyEntry->ulCopyLen);
        }
    }

    return(TRUE);

}


/*
    @func   BOOLEAN | ReadTOC | Locates the load image's table of contents (TOC) and reads the TOC into the caller-provided buffer.
    @rdesc  TRUE == Success and FALSE == Failure.
    @comm    
    @xref   
*/
static BOOLEAN ReadTOC(HANDLE hPartition, ROMHDR *pTOC)
{
    // TODO: statically allocate some big sector buffer size on the stack...
    BYTE SectorData[PARTITION_HEADER_BYTES];
    UINT32 TOCoffset = 0; 

    if (hPartition == INVALID_HANDLE_VALUE || pTOC == NULL)
    {
        return(FALSE);
    }

    // Restore the file pointer to the start of the partition.
    //
    if (!BP_SetDataPointer(hPartition, 0))
    {
        return(FALSE);
    }

    // Read the first sector - this contains the address of the table of contents.
    //
    if (!BP_ReadData(hPartition, SectorData, PARTITION_HEADER_BYTES))
    {
        return(FALSE);
    }

    // Valid table of contents address signature?
    //
    if (*(UINT32 *)(&SectorData[ROM_SIGNATURE_OFFSET]) != ROM_SIGNATURE)
    {
        return(FALSE);
    }
    TOCoffset = (*(UINT32 *)(&SectorData[ROM_TOC_OFFSET_OFFSET]));

    // Move the file pointer to the correct offset.
    //
    if (!BP_SetDataPointer(hPartition, TOCoffset))
    {
        return(FALSE);
    }
    
    // Read the table of contents.
    //
    if (!BP_ReadData(hPartition, (BYTE *)pTOC, sizeof(ROMHDR)))
    {
        return(FALSE);
    }

    // Restore the file pointer to the start of the partition.
    //
    if (!BP_SetDataPointer(hPartition, 0))
    {
        return(FALSE);
    }
    
    return(TRUE);

}


/*
    @func   BOOLEAN | GetImageInfo | Uses the image's table of contents (TOC) to locate the image start address, length, and jump address.
    @rdesc  TRUE == Success and FALSE == Failure.
    @comm    
    @xref   
*/
static BOOLEAN GetImageInfo(HANDLE hPartition, UINT32 *ulImageStartAddr)
{
    BYTE SectorData[PARTITION_HEADER_BYTES];
    PCOMPRESSED_RAMIMAGE_HEADER pCompHeader = NULL;
    DWORD dwTocLoc = 0, dwTocOffset = 0, dwBytesRead = 0;
    WORD wBlockSize = 0;

    if (hPartition == INVALID_HANDLE_VALUE || ulImageStartAddr == NULL)
    {
        return(FALSE);
    }

    // set the pointer to the beginning of the partition
    if (!BP_SetDataPointer(hPartition, 0))
    {
        return(FALSE);
    }

    // read the first page
    if (!BP_ReadData(hPartition, SectorData, PARTITION_HEADER_BYTES))
    {
        return(FALSE);
    }
    
    // is the partition compressed?
    pCompHeader = (PCOMPRESSED_RAMIMAGE_HEADER) SectorData;
    if ((pCompHeader->dwSignature == RAMIMAGE_COMPRESSION_SIGNATURE) &&
        (pCompHeader->dwVersion == RAMIMAGE_COMPRESSION_VERSION) &&
        (pCompHeader->dwCompressedBlockSize == RAMIMAGE_COMPRESSION_BLOCK_SIZE))
    {
        // initialize the compressor
        if (!InitDecompressor(hPartition, pCompHeader->dwHeaderSize))
        {
            return(FALSE);
        }

        // update the global "partition is compressed" flag
        g_fCompressed = TRUE;
        
        // set the data pointer to the first byte after the compressed header
        if (!BP_SetDataPointer(hPartition, g_CompressionHeader->dwHeaderSize))
        {
            return(FALSE);
        }

        // the compressed size of the first block is stored in pCompHeader
        wBlockSize = g_CompressionHeader->wBlockSizeTable[0];

        // read the first page into our compressed data buffer
        if (!BP_ReadData(hPartition, CompressedData, wBlockSize))
        {
            return(FALSE);
        }

        // if block size = max size, this block is not compressed
        if (wBlockSize == RAMIMAGE_COMPRESSION_BLOCK_SIZE)
        {
            // just copy data over into SectorData
            memcpy(SectorData, CompressedData, PARTITION_HEADER_BYTES);
        }
        else
        {
            // decompress the first page back into SectorData
            dwBytesRead = CeCompressDecode(g_DecodeStream, SectorData, RAMIMAGE_COMPRESSION_BLOCK_SIZE, PARTITION_HEADER_BYTES, CompressedData, wBlockSize);
            if (dwBytesRead != PARTITION_HEADER_BYTES)
            {
                return(FALSE);
            }
        }
    }

    // Valid table of contents address signature?
    if (*(UINT32 *)(&SectorData[ROM_SIGNATURE_OFFSET]) != ROM_SIGNATURE)
    {
        return(FALSE);
    }

    // Extract the start address, length, and jump address from the table of contents.
    //
    dwTocLoc = *(UINT32 *)(&SectorData[ROM_TOC_POINTER_OFFSET]);
    dwTocOffset= *(UINT32 *)(&SectorData[ROM_TOC_OFFSET_OFFSET]);        
    *ulImageStartAddr = dwTocLoc - dwTocOffset;

    return(TRUE);

}


/*
    @func   void | IPLmain | Main IPLcommon entry point.
    @rdesc  None.
    @comm    
    @xref   
*/
void IPLmain(void)
{
    BOOLEAN bROMimage         = FALSE;
    UINT32 ulPartType         = PART_ROMIMAGE;
    PCI_REG_INFO RegInfo;
    FlashInfo StorageInfo;
    HANDLE hPartition         = INVALID_HANDLE_VALUE;
    UINT32 ulImageStartAddr   = 0;
    UINT32 ulPartIndex        = 0;
    BOOLEAN bUpdateMode       = FALSE;

    // Set up the copy section data.
    //
    if (!SetupCopySection(pTOC))
    {
        //MessageDispatch(IPL_ERROR_BAD_IPLTOC);
        SpinForever();
    }
  
    // Perform OEM-specific initialization
    //
    if (!OEMIPLInit())
    {
        SpinForever();
    }

    memset(&RegInfo, 0, sizeof(PCI_REG_INFO));
    RegInfo.MemBase.Num    = 1;
    RegInfo.MemLen.Num     = 1;
    RegInfo.MemBase.Reg[0] = g_ulFlashBase;
    RegInfo.MemLen.Reg[0]  = g_ulFlashLengthBytes;

    // Initialize the boot partition library.
    //
    if (!BP_Init((LPBYTE)g_ulBPartBase, g_ulBPartLengthBytes, NULL, &RegInfo, NULL))
    {
        MessageDispatch(IPL_ERROR_BPINIT_FAILED);
        SpinForever();
    }

    // Determine whether we should enter update mode or not.
    //
    bUpdateMode = OEMGetUpdateMode();
    if (bUpdateMode & UPDATE_MODE)
    {
        ulPartType = PART_BOOTSECTION;
        if (bUpdateMode & ULDR_UPDATE_MODE)
        {
            MessageDispatch(IPL_INFO_LOADING_BACKUP_ULDR);
            ulPartIndex = 1;
        }
    }

    // Open a handle to the requested partition.
    //
    hPartition = BP_OpenPartitionEx(NEXT_FREE_LOC, USE_REMAINING_SPACE, ulPartType, FALSE, PART_OPEN_EXISTING, ulPartIndex);

    // there are two types of XIP partitions - if PART_ROMIMAGE failed, look for PART_RAMIMAGE
    if (hPartition == INVALID_HANDLE_VALUE && ulPartType == PART_ROMIMAGE)
    {
        ulPartType = PART_RAMIMAGE;
        hPartition = BP_OpenPartition(NEXT_FREE_LOC, USE_REMAINING_SPACE, ulPartType, FALSE, PART_OPEN_EXISTING);

        // if we fail to open any XIP partition, try to find the update loader (as a fail-safe)
        if (hPartition == INVALID_HANDLE_VALUE)
        {
            ulPartType = PART_BOOTSECTION;
            hPartition = BP_OpenPartition(NEXT_FREE_LOC, USE_REMAINING_SPACE, ulPartType, FALSE, PART_OPEN_EXISTING);
        }
    }

    // if we still can't find a valid partition, fail
    if (hPartition == INVALID_HANDLE_VALUE)
    {
        MessageDispatch(IPL_ERROR_OPENPARTITION_FAILED);
        SpinForever();
    }

    // Call the FMD to get the flash's sector size.
    //
    if (!FMD_GetInfo(&StorageInfo))
    {
        MessageDispatch(IPL_ERROR_GETSTORAGETYPE_FAILED);
        SpinForever();
    }
    g_wSectorSize = StorageInfo.wDataBytesPerSector;

    // Lock the flash part if we're not loading the Update Loader (added flash protection).
    //
    if ((ulPartType == PART_ROMIMAGE) || (ulPartType == PART_RAMIMAGE))
    {
        FAL_LockFlashRegion(XIP);
        FAL_LockFlashRegion(READONLY_FILESYS);
    }
    else if (ulPartType == PART_BOOTSECTION)
    {
        FAL_UnlockFlashRegion(XIP);
        FAL_UnlockFlashRegion(READONLY_FILESYS);
    }

    // Always unlock the userstore (in case the bootloader didn't do it)
    //
    FAL_UnlockFlashRegion(FILESYS);
    
    // Get image attributes from the table of contents.
    //
    if (!GetImageInfo(hPartition, &ulImageStartAddr))
    {
        MessageDispatch(IPL_ERROR_GETIMAGEINFO_FAILED);
        SpinForever();
    }

    // Convert the returned addressed into IPL-compatible addresses.
    //
    if (!OEMTranslateBaseAddress(ulPartType, ulImageStartAddr, &ulImageStartAddr))
    {
        MessageDispatch(IPL_ERROR_TRANSLATEADDR_FAILED);
        SpinForever();
    }

    // If the start address is in RAM, then copy the image to RAM.
    //
    bROMimage = ((ulImageStartAddr >= g_ulFlashBase) && (ulImageStartAddr < (g_ulFlashBase + g_ulFlashLengthBytes)));
    if (!bROMimage)
    {
        // Let the user know we're about the load the image.
        //
        MessageDispatch(IPL_INFO_LOADING_IMAGE);
        if (!CopyToRAM(hPartition, ulImageStartAddr))
        {
            MessageDispatch(IPL_ERROR_READDATA_FAILED);
            SpinForever();
        }
    }

    // Let the user know we're about to jump to the image.
    //
    MessageDispatch(IPL_INFO_JUMPING_IMAGE);

    // Call OEM code to launch the loaded image.
    //
    OEMLaunchImage(ulImageStartAddr);

    // We should never get here.
    //
    MessageDispatch(IPL_ERROR_JUMP_FAILED);
    SpinForever();

}


// Stub functions.
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD SetKMode(DWORD dwMode)
{
    UNREFERENCED_PARAMETER(dwMode);
    return((DWORD)TRUE);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL InitDecompressor(HANDLE hPartition, DWORD dwHeaderSize)
{
    // Allocate space for the compression header
    g_CompressionHeader = (PCOMPRESSED_RAMIMAGE_HEADER) MyAlloc(NULL, dwHeaderSize);
    if (g_CompressionHeader == NULL)
    {
        return(FALSE);
    }
    
    // read the entire compression header into memory
    if (!BP_SetDataPointer(hPartition, 0))
    {
        return(FALSE);
    }
    
    if (!BP_ReadData(hPartition, (LPVOID) g_CompressionHeader, dwHeaderSize))
    {
        return(FALSE);
    }

    // now create a decode stream
    g_DecodeStream = CeCompressDecodeCreate(NULL, MyAlloc);
    if (g_DecodeStream == NULL)
    {
        return(FALSE);
    }

    return(TRUE);
}

VOID DeInitDecompressor()
{
    CeCompressDecodeClose(g_DecodeStream, NULL, MyDeAlloc);
    g_DecodeStream = NULL;
}

LPVOID MyAlloc(LPVOID pIgnored, DWORD dwAllocSize)
{
    static DWORD dwCurRAMBase = 0;
    LPVOID pAllocBase = NULL;

    UNREFERENCED_PARAMETER(pIgnored);

    // initialize the current allocation base if necessary
    if (dwCurRAMBase == 0)
    {
        dwCurRAMBase = pTOC->ulRAMFree;
    }

    // if there is enough free RAM, perform the allocation
    if ((pTOC->ulRAMEnd - dwCurRAMBase) > dwAllocSize)
    {
        pAllocBase = (LPVOID) dwCurRAMBase;
        dwCurRAMBase += dwAllocSize;
    }

    return pAllocBase;
}

VOID MyDeAlloc(LPVOID pIgnored, LPVOID pAddress)
{
    UNREFERENCED_PARAMETER(pIgnored);
    UNREFERENCED_PARAMETER(pAddress);
    // we're not actually managing memory, so this function is a no-op
}

BOOL CopyToRAM(HANDLE hPartition, DWORD dwPartitionBase)
{
    ROMHDR TOC;
    PWORD pwCompBlockSize = NULL;
    DWORD dwBytesRead = 0, dwCurRAMLoc = 0, i;
    
    if (g_fCompressed)
    {
        // set the data pointer to the first byte after the compressed header
        if (!BP_SetDataPointer(hPartition, g_CompressionHeader->dwHeaderSize))
        {
            return(FALSE);
        }

        // point our block size pointer to the first entry in the table
        pwCompBlockSize = &g_CompressionHeader->wBlockSizeTable[0];

        // start at the beginning of RAM
        dwCurRAMLoc = dwPartitionBase;

        // iterate through all compressed blocks, decompressing straight into RAM
        for (i = 0; i < g_CompressionHeader->dwCompressedBlockCount; i++)
        {
            // read the compressed data into our temporary buffer
            if (!BP_ReadData(hPartition, CompressedData, *pwCompBlockSize))
            {
                return(FALSE);
            }

            // determine if decompression is necessary
            if (*pwCompBlockSize == RAMIMAGE_COMPRESSION_BLOCK_SIZE)
            {
                // just do a memcpy
                memcpy((LPVOID) dwCurRAMLoc, CompressedData, RAMIMAGE_COMPRESSION_BLOCK_SIZE);
            }
            else
            {
                // decompress the data to RAM
                dwBytesRead = CeCompressDecode(g_DecodeStream, (LPVOID) dwCurRAMLoc, RAMIMAGE_COMPRESSION_BLOCK_SIZE, RAMIMAGE_COMPRESSION_BLOCK_SIZE, CompressedData, *pwCompBlockSize);
                if (dwBytesRead != RAMIMAGE_COMPRESSION_BLOCK_SIZE)
                {
                    return(FALSE);
                }
            }

            // increment current RAM location and block number
            ++pwCompBlockSize;
            dwCurRAMLoc += RAMIMAGE_COMPRESSION_BLOCK_SIZE;
        }

    }
    else
    {
        // if the partition is not compressed, we need to read the TOC 
        // to figure out how much we need to copy to RAM
        if (!ReadTOC(hPartition, &TOC))
        {
            return(FALSE);
        }

        // simply copy the data to RAM
        if (!BP_ReadData(hPartition, (LPBYTE) dwPartitionBase, (TOC.physlast - TOC.physfirst)))
        {
            return(FALSE);
        }
    }
    
    return(TRUE);
}
