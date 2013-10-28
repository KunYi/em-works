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
#include <bootBlockUtils.h>
#include <bootMemory.h>
#include <bootLog.h>

#include <pehdr.h>
#include <romLdr.h>
#include <cecompress.h>

//------------------------------------------------------------------------------

#define PARTITION_HEADER_BYTES     (ROM_TOC_OFFSET_OFFSET + sizeof(DWORD))

//------------------------------------------------------------------------------
// External variables

extern
ROMHDR*
volatile
const
pTOC;

//------------------------------------------------------------------------------

typedef struct ReadContext_t {
    handle_t hSegment;
    size_t sectorSize;
    size_t sector;
    uint8_t *pBuffer;
} ReadContext_t;

//------------------------------------------------------------------------------

static
bool_t
ReadInit(
    ReadContext_t *pContext,
    handle_t hBlock,
    handle_t hSegment
    )
{
    bool_t rc = false;

    pContext->hSegment = hSegment;
    pContext->pBuffer = NULL;
    
    pContext->sectorSize = BootBlockSectorSize(hBlock);
    if (pContext->sectorSize == 0) goto cleanUp;

    pContext->pBuffer = BootAlloc(pContext->sectorSize);
    if (pContext->pBuffer == NULL) goto cleanUp;

    pContext->sector = (size_t)-1;

    rc = true;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

static
void
ReadDeinit(
    ReadContext_t *pContext
    )
{
    BootFree(pContext->pBuffer);
}

//------------------------------------------------------------------------------

static
bool_t
Read(
    ReadContext_t *pContext,
    size_t pos,
    size_t size,
    void *pBuffer
    )
{
    bool_t rc = false;
    size_t sector = pos / pContext->sectorSize;
    size_t posInSector = pos - sector * pContext->sectorSize;
    
    while (size > 0)
        {
        if ((posInSector == 0) && (size >= pContext->sectorSize))
            {
            size_t count;
            size_t sectors = size / pContext->sectorSize;
            if (!BootBlockRead(pContext->hSegment, sector, sectors, pBuffer))
                goto cleanUp;
            sector += sectors;
            count = sectors * pContext->sectorSize;
            size -= count;
            pBuffer = (uint8_t*)pBuffer + count;
            }
        else
            {
            size_t count;
            if (sector != pContext->sector)
                {
                if (!BootBlockRead(
                        pContext->hSegment, sector, 1, pContext->pBuffer
                        )) goto cleanUp;
                pContext->sector = sector;
                }
            count = pContext->sectorSize - posInSector;
            if (count > size) count = size;
            memcpy(pBuffer, &pContext->pBuffer[posInSector], count);
            sector++;
            size -= count;
            pBuffer = (uint8_t*)pBuffer + count;
            posInSector = 0;
            }
        }

    // Done
    rc = true;
    
cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

static
VOID*
CompressAlloc(
    VOID* pContext,
    DWORD size
    )
{
    UNREFERENCED_PARAMETER(pContext);
    return BootAlloc(size);
}

//------------------------------------------------------------------------------

static
VOID
CompressFree(
    VOID* pContext,
    VOID* pMemory
    )
{
    UNREFERENCED_PARAMETER(pContext);
    BootFree(pMemory);
}

//------------------------------------------------------------------------------

static
bool_t
LoadCompressed(
    ReadContext_t *pContext,
    COMPRESSED_RAMIMAGE_HEADER *pHeader,
    uint32_t offset,
    uint32_t *pAddress
    )
{
    bool_t rc = FALSE;
    CeCompressDecodeStream decodeStream = NULL;
    uint16_t *pBlockSize = NULL;
    uint8_t *pData = NULL, *pBuffer = NULL, *pFirst = NULL;
    size_t pos, size, block, blockSize;


    // Allocate buffer & signature one
    blockSize = pHeader->dwCompressedBlockSize;
    if (blockSize < PARTITION_HEADER_BYTES) goto cleanUp;
    pBuffer = BootAlloc(blockSize);
    if (pBuffer == NULL) goto cleanUp;
    pData = pFirst = BootAlloc(blockSize);
    if (pFirst == NULL) goto cleanUp;

    // Allocate memory for block sizes
    size = sizeof(uint16_t) * pHeader->dwCompressedBlockCount;
    pBlockSize = BootAlloc(size);
    if (pBlockSize == NULL) goto cleanUp;

    // Read block sizes
    pos = FIELD_OFFSET(COMPRESSED_RAMIMAGE_HEADER, wBlockSizeTable);
    if (!Read(pContext, pos, size, pBlockSize)) goto cleanUp;

    // Initialize the decoder
    decodeStream = CeCompressDecodeCreate(NULL, CompressAlloc);
    if (decodeStream == NULL) goto cleanUp;

    // Read all blocks
    pos = pHeader->dwHeaderSize;
    for (block = 0; block < pHeader->dwCompressedBlockCount; block++)
        {
        // read the first page into our compressed data buffer
        size = pBlockSize[block];
        if (!Read(pContext, pos, size, pBuffer)) goto cleanUp;

        // if block size = max size, this block is not compressed
        if (size == blockSize)
            {
            // Just copy data over into SectorData
            memcpy(pData, pBuffer, size);
            }
        else
            {
            // Decompress data
            if (CeCompressDecode(
                    decodeStream, pData, blockSize, blockSize, pBuffer, size
                    ) != blockSize)
                goto cleanUp;
            }

        if (block == 0)
            {
            uint32_t toc, tocOffset, base, top;

            if (*(uint32_t*)(&pData[ROM_SIGNATURE_OFFSET]) != ROM_SIGNATURE)
                {
                goto cleanUp;
                }
            toc = *(uint32_t*)(&pData[ROM_TOC_POINTER_OFFSET]);
            tocOffset= *(uint32_t*)(&pData[ROM_TOC_OFFSET_OFFSET]);
            base = toc - tocOffset;
            base = (uint32_t)BootPAtoCA(BootImageVAtoPA((void*)base));
            *pAddress = base;

            base += offset;
            top = base + blockSize;

            // Check if loaded image doesn't overwritte boot loader
            if (((top > pTOC->physfirst) && (base <= pTOC->physlast)) ||
                ((top > pTOC->ulRAMStart) && (base <= pTOC->ulRAMEnd)))
                {
                BOOTMSG(ZONE_ERROR, (L"ERROR: BootBlockLoadBinFsImage: "
                    L"Image overlaps boot loader code (base 0x%08)!\r\n",
                    base
                    ));
                goto cleanUp;
                }

            pData = (uint8_t*)base;
            memcpy(pData, pFirst, blockSize);
            }

        pos += size;
        pData += blockSize;
        }

    // Done
    rc = true;

cleanUp:
    BootFree(pBlockSize);
    BootFree(pFirst);
    BootFree(pBuffer);
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootBlockLoadBinFsImage(
    handle_t hBlock,
    handle_t hSegment,
    size_t offset,
    uint32_t *pAddress
    )
{
    bool_t rc = FALSE;
    ReadContext_t context;
    uint8_t data[PARTITION_HEADER_BYTES];
    COMPRESSED_RAMIMAGE_HEADER *pHeader = NULL;


    // Initialize read context
    if (!ReadInit(&context, hBlock, hSegment)) goto cleanUp;

    // Read header
    if (!Read(&context, 0, sizeof(data), data))
        {
        BOOTMSG(ZONE_WARN, (L"BootBlockLoadBinFsImage: "
            L"Failed read signature and offsets!\r\n"
            ));
        goto cleanUp;
        }

    // Find which image is there...
    pHeader = (COMPRESSED_RAMIMAGE_HEADER*)data;
    if ((pHeader->dwSignature == RAMIMAGE_COMPRESSION_SIGNATURE) &&
        (pHeader->dwVersion == RAMIMAGE_COMPRESSION_VERSION) &&
        (pHeader->dwCompressedBlockSize == RAMIMAGE_COMPRESSION_BLOCK_SIZE))
        {
        rc = LoadCompressed(&context, pHeader, offset, pAddress);
        }
    else if (*(UINT32*)&data[ROM_SIGNATURE_OFFSET] == ROM_SIGNATURE)
        {
        ROMHDR romHdr;
        uint32_t toc, base, size, top;

        // Read ROMHDR
        toc = *(uint32_t*)(&data[ROM_TOC_OFFSET_OFFSET]);
        if (!Read(&context, toc, sizeof(romHdr), &romHdr))
            {
            BOOTMSG(ZONE_WARN, (L"BootBlockLoadBinFsImage: "
                L"Failed read ROMHDR structure!\r\n"
                ));
            goto cleanUp;
            }

        // Set image address and size
        size = romHdr.physlast - romHdr.physfirst;
        base = (uint32_t)BootPAtoCA(BootImageVAtoPA((void*)romHdr.physfirst));
        top = base + size;
        
        // Return base address back
        *pAddress = base;

        // Check if loaded image doesn't overwritte boot loader
        if (((top > pTOC->physfirst) && (base <= pTOC->physlast)) ||
            ((top > pTOC->ulRAMStart) && (base <= pTOC->ulRAMEnd)))
            {
            BOOTMSG(ZONE_ERROR, (L"ERROR: BootBlockLoadBinFsImage: "
                L"Image overlaps boot loader code (0x%08x - 0x%08x)!\r\n",
                base, top
                ));
            goto cleanUp;
            }

        // Read image
        if (!Read(&context, 0, size, (void*)(*pAddress + offset)))
            {
            BOOTMSG(ZONE_ERROR, (L"ERROR: BootBlockLoadBinFsImage: "
                L"Failed read image!\r\n"
                ));
            goto cleanUp;
            }

        // Done
        rc = true;
        }
    else
        {
        BOOTMSG(ZONE_WARN, (L"BootBlockLoadBinFsImage: "
            L"BinFS signature wasn't found!\r\n"
            ));
        goto cleanUp;
        }

cleanUp:
    ReadDeinit(&context);
    return rc;
}

//------------------------------------------------------------------------------

