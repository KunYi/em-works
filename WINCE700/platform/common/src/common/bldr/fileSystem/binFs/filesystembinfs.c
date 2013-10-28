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
#include <bootFileSystemBinFs.h>
#include <bootCore.h>
#include <bootMemory.h>
#include <bootLog.h>

#include <pehdr.h>
#include <romldr.h>

//------------------------------------------------------------------------------
//  Global variables

extern
ROMHDR*
volatile
const
pTOC;

//------------------------------------------------------------------------------

typedef struct FILESYSTEM  {
    BootDriverVTable_t *pVTable;
} FILESYSTEM;

typedef struct FILEITEM {
    BootDriverVTable_t *pVTable;
    FILESentry *pFileEntry;
    UINT8 *pBase;
    size_t size;
    size_t pos;
} FILEITEM;

//------------------------------------------------------------------------------
//  Local functions

bool_t
BootFileSystemBinFsDeinit(
    void *pContext
    );

bool_t
BootFileSystemBinFsIoCtl(
    void *pContext,
    enum_t code,
    void *pBuffer,
    size_t size
    );

bool_t
BootFileBinFsDeinit(
    void *pContext
    );

bool_t
BootFileBinFsIoCtl(
    void *pContext,
    enum_t code,
    void *pBuffer,
    size_t size
    );

static
handle_t
Open(
    FILESYSTEM *pContext,
    LPCWSTR name,
    uint32_t access
    );

static
size_t
Read(
    FILEITEM *pFile,
    VOID *pBuffer,
    size_t size
    );

static
bool_t
Seek(
    FILEITEM *pFile,
    enum_t method,
    int32_t offset
    );

static
bool_t
GetPos(
    FILEITEM *pFile,
    size_t *pPosition
    );

static
bool_t
GetSize(
    FILEITEM *pFile,
    size_t *pSize
    );

//------------------------------------------------------------------------------

static
BootDriverVTable_t
s_fileSystemVTable = {
    BootFileSystemBinFsDeinit,
    BootFileSystemBinFsIoCtl
};

static
BootDriverVTable_t
s_fileVTable = {
    BootFileBinFsDeinit,
    BootFileBinFsIoCtl
};

//------------------------------------------------------------------------------

static
FILESYSTEM
s_fileSystem;

//------------------------------------------------------------------------------

VOID*
BootFileSystemBinFsInit(
    )
{
    VOID *pContext = NULL;


    // This driver shouldn't be instantied twice...
    if (s_fileSystem.pVTable == &s_fileSystemVTable)
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootFileSystemBinFsInit: "
            L"Driver already initialized!\r\n"
            ));
        goto cleanUp;
        }

    memset(&s_fileSystem, 0, sizeof(s_fileSystem));
    s_fileSystem.pVTable = &s_fileSystemVTable;

    // Done
    pContext = &s_fileSystem;

cleanUp:
    return pContext;
}

//------------------------------------------------------------------------------

bool_t
BootFileSystemBinFsDeinit(
    void *pContext
    )
{
    bool_t rc = FALSE;
    FILESYSTEM *pFileSystem = pContext;


    // Check driver handle
    if (pFileSystem != &s_fileSystem)
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootFileSystemBinFsDeinit: "
            L"Invalid driver handle!\r\n"
            ));
        goto cleanUp;
        }

    // Clear all info
    memset(&s_fileSystem, 0, sizeof(s_fileSystem));

    // Done
    rc = TRUE;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootFileSystemBinFsIoCtl(
    void *pContext,
    enum_t code,
    void *pBuffer,
    size_t size
    )
{
    bool_t rc = FALSE;
    FILESYSTEM *pFileSystem = pContext;

    
    // Check driver handle
    if (pFileSystem != &s_fileSystem)
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootFileSystemBinFsIoCtl: "
            L"Invalid driver handle!\r\n"
            ));
        goto cleanUp;
        }

    switch (code)
        {
        case BOOT_FILESYSTEM_IOCTL_OPEN:
            {
            BootFileSystemOpenParams_t *pParams = pBuffer;
            if ((pParams == NULL) || (size != sizeof(*pParams)))
                {
                BOOTMSG(ZONE_ERROR, (L"ERROR: BootFileSystemBinFsIoCtl: "
                    L"Invalid BOOT_FILESYSTEM_IOCTL_OPEN parameter!\r\n"
                    ));
                break;
                }
            pParams->hFile = Open(pFileSystem, pParams->name, pParams->options);
            rc = TRUE;
            }
            break;
        }

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootFileBinFsDeinit(
    VOID *pContext
    )
{
    bool_t rc = FALSE;
    FILEITEM *pFileItem = pContext;


    // Check driver handle
    if ((pFileItem == NULL) || (pFileItem->pVTable != &s_fileVTable))
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootFileBinFsDeinit: "
            L"Invalid driver handle!\r\n"
            ));
        goto cleanUp;
        }

    // Clear all info & free memory
    memset(pFileItem, 0, sizeof(*pFileItem));
    BootFree(pFileItem);

    // Done
    rc = TRUE;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootFileBinFsIoCtl(
    void *pContext,
    enum_t code,
    void *pBuffer,
    size_t size
    )
{
    bool_t rc = FALSE;
    FILEITEM *pFileItem = pContext;


    // Check driver handle
    if ((pFileItem == NULL) || (pFileItem->pVTable != &s_fileVTable))
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootFileBinFsIoCtl: "
            L"Invalid driver handle!\r\n"
            ));
        goto cleanUp;
        }

    switch (code)
        {
        case BOOT_FILESYSTEM_IOCTL_READ:
            {
            BootFileSystemReadParams_t *pParams = pBuffer;
            if ((pParams == NULL) || (size != sizeof(*pParams)))
                {
                BOOTMSG(ZONE_ERROR, (L"ERROR: BootFileBinFsIoCtl: "
                    L"Invalid BOOT_FILESYSTEM_IOCTL_READ parameter!\r\n"
                    ));
                break;
                }
            pParams->size = Read(pFileItem, pParams->pBuffer, pParams->size);
            rc = TRUE;
            }
            break;
        case BOOT_FILESYSTEM_IOCTL_SEEK:
            {
            BootFileSystemSeekParams_t *pParams = pBuffer;
            if ((pParams == NULL) || (size != sizeof(*pParams)))
                {
                BOOTMSG(ZONE_ERROR, (L"ERROR: BootFileBinFsIoCtl: "
                    L"Invalid BOOT_FILESYSTEM_IOCTL_SEEK parameter!\r\n"
                    ));
                break;
                }
            rc = Seek(pFileItem, pParams->method, pParams->offset );
            }
            break;
        case BOOT_FILESYSTEM_IOCTL_GET_POSITION:
            {
            BootFileSystemGetPosParams_t *pParams = pBuffer;
            if ((pParams == NULL) || (size != sizeof(*pParams)))
                {
                BOOTMSG(ZONE_ERROR, (L"ERROR: BootFileBinFsIoCtl: "
                    L"Invalid BOOT_FILESYSTEM_IOCTL_GETPOS parameter!\r\n"
                    ));
                break;
                }
            rc = GetPos(pFileItem, pParams->pPosition );
            }
            break;
        case BOOT_FILESYSTEM_IOCTL_GET_SIZE:
            {
            BootFileSystemGetSizeParams_t *pParams = pBuffer;
            if ((pParams == NULL) || (size != sizeof(*pParams)))
                {
                BOOTMSG(ZONE_ERROR, (L"ERROR: BootFileBinFsIoCtl: "
                    L"Invalid BOOT_FILESYSTEM_IOCTL_GETSIZE parameter!\r\n"
                    ));
                break;
                }
            rc = GetSize(pFileItem, &pParams->size );
            }
            break;

        default:
            BOOTMSG(ZONE_WARN, (L"WARN: BootFileBinFsIoCtl: "
                L"Unsupported IoCtl code %08x!\r\n", code
                ));
            break;
        }

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

static
bool_t
AreNameEqual(
    LPCWSTR wname,
    LPCSTR name
    )
{
    bool_t rc = FALSE;

    while (wname[0] == name[0])
        {
        if (name[0] == '\0')
            {
            rc = TRUE;
            break;
            }
        wname++;
        name++;
        }

    return rc;
}

//------------------------------------------------------------------------------

static
handle_t
Open(
    FILESYSTEM *pContext,
    LPCWSTR name,
    uint32_t access
    )
{
    FILEITEM *pFile = NULL;
    ULONG ix;
    TOCentry *pModuleEntry;
    FILESentry *pFileEntry;

    UNREFERENCED_PARAMETER(pContext);

    // We support only read access on BinFs
    if ((access & BOOT_FILESYSTEM_ACCESS_WRITE) != 0) goto cleanUp;
    if ((access & BOOT_FILESYSTEM_ACCESS_READ) == 0) goto cleanUp;

    // Get module & files entries
    pModuleEntry = (TOCentry*)&pTOC[1];
    pFileEntry = (FILESentry*)(&pModuleEntry[pTOC->nummods]);

    // Look for file
    ix = 0;
    while (ix < pTOC->numfiles)
        {
        if (AreNameEqual(name, pFileEntry->lpszFileName)) break;
        pFileEntry++;
        ix++;
        }
    if (ix >= pTOC->numfiles) goto cleanUp;

    
    // Allocate file structure
    pFile = BootAlloc(sizeof(FILEITEM));
    if (pFile == NULL) goto cleanUp;

    pFile->pVTable = &s_fileVTable;
    pFile->pFileEntry = pFileEntry;
    pFile->pBase = (UINT8*)pFileEntry->ulLoadOffset;
    pFile->pos = 0;
    if (pFileEntry->nCompFileSize == 0)
        {
        pFile->size = pFileEntry->nRealFileSize;
        }
    else
        {
        pFile->size = pFileEntry->nCompFileSize;
        }

cleanUp:
    return pFile;
}

//------------------------------------------------------------------------------

static
size_t
Read(
    FILEITEM *pFile,
    VOID *pBuffer,
    size_t size
    )
{
    size_t rc;

    // Get data size & copy
    rc = pFile->size - pFile->pos;
    if (size < rc) rc = size;
    memcpy(pBuffer, &pFile->pBase[pFile->pos], rc);
    pFile->pos += rc;
    
    return rc;
}

//------------------------------------------------------------------------------

static
bool_t
Seek(
    FILEITEM *pFile,
    enum_t method,
    int32_t offset
    )
{
    bool_t rc = false;

    switch(method)
        {
        case BOOT_FILESYSTEM_SEEK_FILE_BEGIN:
            if (offset < 0) goto cleanUp;
            if ((uint32_t)offset > pFile->size) goto cleanUp;
            pFile->pos = offset;
            break;
        case BOOT_FILESYSTEM_SEEK_FILE_CURRENT:
            if (pFile->pos + offset > pFile->size) goto cleanUp;
            if (pFile->pos + offset < 0) goto cleanUp;
            pFile->pos += offset;
            break;
        case BOOT_FILESYSTEM_SEEK_FILE_END:
            if (offset > 0) goto cleanUp;
            offset *= -1;
            if ((uint32_t)offset > pFile->size) goto cleanUp;
            pFile->pos = pFile->size - offset;
            break;
        default:
            goto cleanUp;
        }

    rc = true;

cleanUp:    
    return rc;
}

//------------------------------------------------------------------------------

static
bool_t
GetPos(
    FILEITEM *pFile,
    size_t *pPosition
    )
{
    if( NULL == pPosition ) return false;
    *pPosition = pFile->pos;
    return true;
}

//------------------------------------------------------------------------------

static
bool_t
GetSize(
    FILEITEM *pFile,
    size_t *pSize
    )
{
    if( NULL == pSize ) return false;
    *pSize = pFile->size;
    return true;
}
