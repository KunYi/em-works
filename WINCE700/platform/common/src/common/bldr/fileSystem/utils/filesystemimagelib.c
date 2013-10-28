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
#include <bootFileSystemUtils.h>
#include <bootLog.h>

//------------------------------------------------------------------------------

bool_t
BootFileSystemReadFromImageLib(
    handle_t hFileSystem,
    const wstring_t name,
    uint32_t index,
    RECT *pRect,
    void *pBuffer,
    size_t bufferSize
    )
{
    bool_t rc = false;
    handle_t hFile = NULL;
    size_t pos = 0, size;
    size_t decompressedImageSize = 0;


    hFile = BootFileSystemOpen(
        hFileSystem, name, BOOT_FILESYSTEM_ACCESS_READ, 
        BOOT_FILESYSTEM_ATTRIBUTE_NORMAL
        );
    if (hFile == NULL)
        {
        BOOTMSG(ZONE_ERROR, (L"BootFileSystemReadFromImageLib: "
            L"BootFileSystem(..., %s, ...) failed!\r\n", name
            ));
        goto cleanUp;
        }
    
    // Seek to record beginning
    while (index-- > 0)
        {
        // Read record size
        if (BootFileSystemRead(hFile, &size, sizeof(size)) != sizeof(size))
            {
            BOOTMSG(ZONE_ERROR, (L"BootFileSystemReadFromImageLib: "
                L"BootFileSystemRead(%s, ..., %d) failed!\r\n", name, size
                ));
            goto cleanUp;
            }
        // Get next record position
        pos += size + sizeof(size);
        // Skip it
        if (!BootFileSystemSeek(hFile, BOOT_FILESYSTEM_SEEK_FILE_BEGIN, pos))
            {
            BOOTMSG(ZONE_ERROR, (L"BootFileSystemReadFromImageLib: "
                L"BootFileSystemSeek(%s, %d) failed!\r\n", name, pos
                ));
            goto cleanUp;
            }
        }

    // Read record size
    if (BootFileSystemRead(hFile, &size, sizeof(size)) != sizeof(size))
        {
        BOOTMSG(ZONE_ERROR, (L"BootFileSystemReadFromImageLib: "
            L"BootFileSystemRead(%s, ..., %d) failed!\r\n", name, size
            ));
        goto cleanUp;
        }

    // Now read image position
    if (BootFileSystemRead(hFile, pRect, sizeof(*pRect)) != sizeof(*pRect))
        {
        BOOTMSG(ZONE_ERROR, (L"BootFileSystemReadFromImageLib: "
            L"BootFileSystemRead(%s, ..., %d) failed!\r\n", name, size
            ));
        goto cleanUp;
        }

    // decompressed image size is width * height * bpp
    // bpp is hardcoded to 2 (16 bits, RGB565) since imglib tool converts bitmaps to RGB565 format
    decompressedImageSize = (pRect->right - pRect->left) * 
                            (pRect->bottom - pRect->top) * 2;

    if (bufferSize < decompressedImageSize) 
        {
        BOOTMSG(ZONE_ERROR, (L"BootFileSystemReadFromImageLib: "
            L"Decompressed image size (%d) larger than buffer size (%d)!\r\n", 
                decompressedImageSize , bufferSize
            ));
        goto cleanUp;
        }

    // Read binary image
    if (BootFileSystemReadCompressed(hFile, pBuffer, decompressedImageSize) != decompressedImageSize)
        {
        BOOTMSG(ZONE_ERROR, (L"BootFileSystemReadFromImageLib: "
            L"BootFileSystemRead(%s, ..., %d) failed!\r\n", name, decompressedImageSize
            ));
        goto cleanUp;
        }

    // Done
    rc = true;
    
cleanUp:
    BootFileSystemClose(hFile);
    return rc;
}

//------------------------------------------------------------------------------

