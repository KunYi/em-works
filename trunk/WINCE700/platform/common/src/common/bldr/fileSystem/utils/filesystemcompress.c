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
#include <bootFileSystem.h>
#include <bootMemory.h>
#include <bootLog.h>
#include <cecompress.h>

//------------------------------------------------------------------------------

#define BINCOMPRESS_SIGNATURE       "XPRS"
#define NO_COMPRESSION_BIT          (1 << 31)
#define PARTIAL_BLOCK_BIT           (1 << 30)

//------------------------------------------------------------------------------
// Data buffers

static
UINT8
s_readBuffer[CECOMPRESS_MAX_BLOCK];

//------------------------------------------------------------------------------

static
VOID*
BootFileSystemCompressAlloc(
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
BootFileSystemCompressFree(
    VOID* pContext, 
    VOID* pMemory
    )
{
    UNREFERENCED_PARAMETER(pContext);
    BootFree(pMemory);
}

//------------------------------------------------------------------------------

size_t
BootFileSystemReadCompressed(
    handle_t hFile,
    VOID *pBuffer,
    size_t bufferSize
    )
{
    uint32_t rc = 0;
    CeCompressDecodeStream decodeStream = NULL;
    uint8_t sign[dimof(BINCOMPRESS_SIGNATURE) - 1];
    uint32_t totalLength;
    uint32_t header;
    uint32_t compressedLength, decompressedLength;
    bool_t storedBlock, partialBlock;
    

    // Initialize the decoder
    decodeStream = CeCompressDecodeCreate(NULL, BootFileSystemCompressAlloc); 
    if (decodeStream == NULL) goto cleanUp;
    
    // Read signature
    if (sizeof(sign) != BootFileSystemRead(hFile, sign, sizeof(sign)))
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootFileSystemReadCompressed: "
            L"Unexpected stream read error!\r\n"
            ));
        goto cleanUp;
        }
    
    // Check signature value
    if (memcmp(sign, BINCOMPRESS_SIGNATURE, sizeof(sign)) != 0)
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootFileSystemReadCompressed: "
            L"BinCompress signature XPRS doesn't found!\r\n"
            ));
        goto cleanUp;
        }

    // Read decompressed file length
    if (sizeof(totalLength) != BootFileSystemRead(
            hFile, &totalLength, sizeof(totalLength)
            ))
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootFileSystemReadCompressed: "
            L"Unexpected stream read error!"
            ));
        goto cleanUp;
        }
    
    // Decode input blocks into CECOMPRESS_MAX_BLOCK-sized blocks
    while (sizeof(header) == BootFileSystemRead(
            hFile, &header, sizeof(header)
            ))
        {
        storedBlock = header & NO_COMPRESSION_BIT;
        partialBlock = header & PARTIAL_BLOCK_BIT;
        compressedLength = header & ~(NO_COMPRESSION_BIT|PARTIAL_BLOCK_BIT);

        // Read decompressed block length if it's a partial block
        if (partialBlock)
            {
            if (sizeof(decompressedLength) != BootFileSystemRead(
                    hFile, &decompressedLength, sizeof(decompressedLength)
                    ))
                {
                BOOTMSG(ZONE_ERROR, (L"ERROR: BootFileSystemReadCompressed: "
                    L"Unexpected stream read error!\r\n"
                    ));
                goto cleanUp;
                }
            }
        else
            {
            decompressedLength = CECOMPRESS_MAX_BLOCK;
            }

        // Check if decompressed block fits in buffer
        if (decompressedLength > bufferSize) break;

        // Read compressed data
        if (compressedLength != BootFileSystemRead(
                hFile, s_readBuffer, compressedLength
                ))
            {
            BOOTMSG(ZONE_ERROR, (L"ERROR: BootFileSystemReadCompressed: "
                L"Unexpected boot store stream read error!\r\n"
                ));
            goto cleanUp;
            }

        // Compressed or just stored?
        if (storedBlock)
            {
            // Uncompressed block - just copy
            memcpy(pBuffer, s_readBuffer, compressedLength);
            }
        else
            {
            // Decode the data
            if (CeCompressDecode(
                    decodeStream, pBuffer, decompressedLength, 
                    decompressedLength, s_readBuffer, compressedLength
                    ) == -1)
                {
                BOOTMSG(ZONE_ERROR, (L"ERROR: BootFileSystemReadCompressed: "
                    L"Decompression failed!\r\n"
                    ));
                goto cleanUp;
                }
            }

        // Move forward in buffer
        pBuffer = (UINT8*)pBuffer + decompressedLength;
        bufferSize -= decompressedLength;
        rc += decompressedLength;
        }
   
cleanUp:
    // Clean things up
    if (decodeStream != NULL) 
        {
        CeCompressDecodeClose(decodeStream, NULL, BootFileSystemCompressFree);
        }
    return rc;
}

//------------------------------------------------------------------------------

