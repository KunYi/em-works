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
#include <bootFileSystemUtils.h>
#include <bootMemory.h>
#include <bootLog.h>
#include <bootDownloadBinFormat.h>

#include <pehdr.h>
#include <romLdr.h>

//------------------------------------------------------------------------------
// External variables

extern
ROMHDR*
volatile
const
pTOC;

//------------------------------------------------------------------------------

bool_t
BootFileSystemReadBinFile(
    handle_t hFile,
    size_t offset,
    uint32_t *pAddress
    )
{
    bool_t rc = false;
    uint8_t sign[7];
    BootBinFormatRamHeader_t imageHeader;
    uint32_t imageBase, imageTop;


    if (!BootFileSystemSeek(hFile, BOOT_FILESYSTEM_SEEK_FILE_BEGIN, 0))
        {
        BOOTMSG(ZONE_WARN, (L"BootFileSystemReadBinFile: "
            L"Failed seek to file start!\r\n"
            ));
        goto cleanUp;
        }

    // Read signature
    if ((BootFileSystemRead(hFile, sign, sizeof(sign)) != sizeof(sign)) || 
        (memcmp(sign, BOOT_BIN_SIGNATURE_RAM, sizeof(sign) != 0)))
        {
        BOOTMSG(ZONE_WARN, (L"BootFileSystemReadBinFile: "
            L"Invalid file signature!\r\n"
            ));
        goto cleanUp;
        }

    // Read image header
    if (BootFileSystemRead(
            hFile, &imageHeader, sizeof(imageHeader)
            ) != sizeof(imageHeader))
        {
        BOOTMSG(ZONE_WARN, (L"BootFileSystemReadBinFile: "
            L"Failed read image header from file!\r\n"
            ));
        goto cleanUp;
        }

    // Check image location
    imageBase = imageHeader.start + offset;
    imageTop = imageBase + imageHeader.size;
    if (imageTop < imageBase)
        {
        BOOTMSG(ZONE_WARN, (L"BootFileSystemReadBinFile: "
            L"Image overlap 32-bit address window!\r\n"
            ));
        goto cleanUp;
        }
    
    // Check if loaded image doesn't overwritte boot loader
    if (((imageTop > pTOC->physfirst) && (imageBase <= pTOC->physlast)) ||
        ((imageTop > pTOC->ulRAMStart) && (imageBase <= pTOC->ulRAMEnd)))
        {
        BOOTMSG(ZONE_WARN, (L"BootFileSystemReadBinFile: "
            L"Image overlaps boot loader code (0x%08 - 0x%08x)!\r\n",
            imageBase, imageTop
            ));
        goto cleanUp;
        }

    // Read all records
    for (;;)
        {
        BootBinFormatRamRecordHeader_t header;
        uint32_t base, top, sum, ix;
        uint8_t* pData;

        // Read record header
        if (BootFileSystemRead(
                hFile, &header, sizeof(header)
                ) != sizeof(header))
            {
            BOOTMSG(ZONE_WARN, (L"BootFileSystemReadBinFile: "
                L"Failed read record header from file!\r\n"
                ));
            goto cleanUp;
            }

        // Check for last record
        if ((header.address == 0) && (header.checksum == 0))
            {
            *pAddress = BootImageVAtoPA((void*)header.length);
            break;
            }

        // Check if record fit in declated image location
        base = header.address + offset;
        top = base + header.length;
        if ((top < base) || (base < imageBase) || (top > imageTop))
            {
            BOOTMSG(ZONE_WARN, (L"BootFileSystemReadBinFile: "
                L"Invalid record header (0x%08x, 0x%08x, 0x%08x)!\r\n", 
                header.address, header.length, offset
                ));
            goto cleanUp;
            }

        // Calculate boot loader address
        pData = BootPAtoCA(BootImageVAtoPA((void*)base));
        
        // Read record to memory
        if (BootFileSystemRead(hFile, pData, header.length) != header.length)
            {
            BOOTMSG(ZONE_WARN, (L"BootFileSystemReadBinFile: "
                L"Failed read record data from file!\r\n"
                ));
            goto cleanUp;
            }

        // Calculate check sum
        for (ix = 0, sum = 0; ix < header.length; ix++) sum += pData[ix];
        if (sum != header.checksum)
            {
            BOOTMSG(ZONE_WARN, (L"BootFileSystemReadBinFile: "
                L"Calculated record checksum doesn't match header!\r\n"
                ));
            goto cleanUp;
            }
        
        }

    // Done
    rc = true;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

